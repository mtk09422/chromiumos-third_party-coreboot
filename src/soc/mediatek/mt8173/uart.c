/*
 * This file is part of the coreboot project.
 *
 * Copyright 2015 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <uart.h>
#include <arch/io.h>
#include <console/console.h>	/* for __console definition */
#include <stdint.h>
#include <uart8250.h>

#include <soc/mt8173.h>
#include <soc/addressmap.h>

struct mtk_uart {
	union {
		uint32_t thr;	/* Transmit holding register. */
		uint32_t rbr;	/* Receive buffer register. */
		uint32_t dll;	/* Divisor latch lsb. */
	};
	union {
		uint32_t ier;	/* Interrupt enable register. */
		uint32_t dlm;	/* Divisor latch msb. */
	};
	union {
		uint32_t iir;	/* Interrupt identification register. */
		uint32_t fcr;	/* FIFO control register. */
		uint32_t efr;	/* Enhanced feature register. */
	};
	uint32_t lcr;		/* Line control register. */
	union {
		uint32_t mcr;	/* Modem control register. */
		uint32_t xn1;	/* XON1 */
	};
	union {
		uint32_t lsr;	/* Line status register. */
		uint32_t xn2;	/* XON2 */
	};
	union {
		uint32_t msr;	/* Modem status register. */
		uint32_t xf1;	/* XOFF1 */
	};
	union {
		uint32_t scr;	/* Scratch register. */
		uint32_t xf2;	/* XOFF2 */
	};
	uint32_t autobaud_en;	/* Enable auto baudrate. */
	uint32_t highspeed;	/* High speed UART. */
} __attribute__ ((packed));

/* Peripheral Reset and Power Down registers */
struct mtk_peri_globalcon {
	uint32_t rst0;
	uint32_t rst1;
	uint32_t pdn0_set;
	uint32_t pdn1_set;
	uint32_t pdn0_clr;
	uint32_t pdn1_clr;
	uint32_t pdn0_sta;
	uint32_t pdn1_sta;
	uint32_t pdn_md1_set;
	uint32_t pdn_md2_set;
	uint32_t pdn_md1_clr;
	uint32_t pdn_md2_clr;
	uint32_t pdn_md1_sta;
	uint32_t pdn_md2_sta;
	uint32_t pdn_md_mask;
} __attribute__ ((packed));

static struct mtk_uart *const uart_ptr = (void *)UART0_BASE;

static void mtk_uart_tx_flush(void);
static int mtk_uart_tst_byte(void);

static void mtk_uart_init(void)
{
	/* Use a hardcoded divisor for now. */
	const unsigned uartclk = 26000000;
	const unsigned baudrate = CFG_META_BAUDRATE;
	const uint8_t line_config = UART8250_LCR_WLS_8;	/* 8n1 */
	unsigned highspeed, quot, divisor, remainder;

	if (baudrate <= 115200) {
		highspeed = 0;
		quot = 16;
	} else {
		highspeed = 2;
		quot = 4;
	}

	/* Set divisor DLL and DLH  */
	divisor = uartclk / (quot * baudrate);
	remainder = uartclk % (quot * baudrate);

	if (remainder >= (quot / 2) * baudrate)
		divisor += 1;

	mtk_uart_tx_flush();

	/* Disable interrupts. */
	write8(&uart_ptr->ier, 0);
	/* Force DTR and RTS to high. */
	write8(&uart_ptr->mcr, UART8250_MCR_DTR | UART8250_MCR_RTS);
	/* Set High speed UART register. */
	write8(&uart_ptr->highspeed, highspeed);
	/* Set line configuration, access divisor latches. */
	write8(&uart_ptr->lcr, UART8250_LCR_DLAB | line_config);
	/* Set the divisor. */
	write8(&uart_ptr->dll, divisor & 0xff);
	write8(&uart_ptr->dlm, (divisor >> 8) & 0xff);
	/* Hide the divisor latches. */
	write8(&uart_ptr->lcr, line_config);
	/* Enable FIFOs, and clear receive and transmit. */
	write8(&uart_ptr->fcr,
	       UART8250_FCR_FIFO_EN | UART8250_FCR_CLEAR_RCVR |
               UART8250_FCR_CLEAR_XMIT);

}

static void mtk_uart_tx_byte(unsigned char data)
{
	while (!(read8(&uart_ptr->lsr) & UART8250_LSR_THRE));
	write8(&uart_ptr->thr, data);
}

static void mtk_uart_tx_flush(void)
{
	while (!(read8(&uart_ptr->lsr) & UART8250_LSR_TEMT));
}

static unsigned char mtk_uart_rx_byte(void)
{
	if (!mtk_uart_tst_byte())
		return 0;
	return read8(&uart_ptr->rbr);
}

static int mtk_uart_tst_byte(void)
{
	return (read8(&uart_ptr->lsr) & UART8250_LSR_DR) == UART8250_LSR_DR;
}

#if !defined(__PRE_RAM__)

static const struct console_driver mtk_uart_console __console = {
	.init = mtk_uart_init,
	.tx_byte = mtk_uart_tx_byte,
	.tx_flush = mtk_uart_tx_flush,
	.rx_byte = mtk_uart_rx_byte,
	.tst_byte = mtk_uart_tst_byte,
};

uint32_t uartmem_getbaseaddr(void)
{
	return (uintptr_t) uart_ptr;
}

#else

void uart_init(void)
{
	mtk_uart_init();
}

void uart_tx_byte(unsigned char data)
{
	mtk_uart_tx_byte(data);
}

void uart_tx_flush(void)
{
	mtk_uart_tx_flush();
}

unsigned char uart_rx_byte(void)
{
	return mtk_uart_rx_byte();
}

#endif
