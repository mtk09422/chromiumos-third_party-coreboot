/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2013 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <console/console.h>
#include <uart.h>

#define FOUNDATION_UART0_IO_ADDRESS	(0x1C090000)

#define PL011_UARTCR_OFFSET		(0x030)

static void pl011_init_dev(void)
{
	static volatile uint32_t *uartcr_address =
		(uint32_t *)(FOUNDATION_UART0_IO_ADDRESS + PL011_UARTCR_OFFSET);

	/* Enable TX and UART */
	*uartcr_address = *uartcr_address | (1 << 8) | (1 << 0);
}

static void pl011_uart_tx_byte(uint8_t data)
{
	static volatile uint32_t *uart0_address =
		(uint32_t *)FOUNDATION_UART0_IO_ADDRESS;

	*uart0_address = (uint32_t)data;
}

static void pl011_uart_tx_flush(void)
{
}

#if !defined(__PRE_RAM__)

static const struct console_driver pl011_uart_console __console = {
	.init     = pl011_init_dev,
	.tx_byte  = pl011_uart_tx_byte,
	.tx_flush = pl011_uart_tx_flush,
};

uint32_t uartmem_getbaseaddr(void)
{
	return FOUNDATION_UART0_IO_ADDRESS;
}

#else
void uart_init(void)
{
	pl011_init_dev();
}

void uart_tx_byte(unsigned char data)
{
	pl011_uart_tx_byte(data);
}

void uart_tx_flush(void)
{
	pl011_uart_tx_flush();
}
#endif
