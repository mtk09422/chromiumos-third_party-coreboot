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

#ifndef ___MTK_UART_H__
#define ___MTK_UART_H__

#include <libpayload-config.h>

#define WITH_STABLE_SPEED_CONSOLE    1

typedef enum {
	UART1 = (CONFIG_LP_MTK_SERIAL_BASE),
	UART2 = (CONFIG_LP_MTK_SERIAL_BASE + 0x1000),
	UART3 = (CONFIG_LP_MTK_SERIAL_BASE + 0x2000),
	UART4 = (CONFIG_LP_MTK_SERIAL_BASE + 0x2000)
} MTK_UART;

/*uart info*/
enum {
	PERI_CON_BASE = 0x11003000,
	UART_FIFO_SIZE = 16,
	IO_OFFSET = 0
};

/* IER */
enum {
	UART_IER_ERBFI = (1 << 0),      /* RX buffer conatins data int. */
	UART_IER_ETBEI = (1 << 1),      /* TX FIFO threshold trigger int. */
	UART_IER_ELSI = (1 << 2),       /* BE, FE, PE, or OE int. */
	UART_IER_EDSSI = (1 << 3),      /* CTS change (DCTS) int. */
	UART_IER_XOFFI = (1 << 5),
	UART_IER_RTSI = (1 << 6),
	UART_IER_CTSI = (1 << 7),
	UART_IER_ALL_INTS = (UART_IER_ERBFI | UART_IER_ETBEI | UART_IER_ELSI | \
			     UART_IER_EDSSI | UART_IER_XOFFI | UART_IER_RTSI | \
			     UART_IER_CTSI),
	UART_IER_HW_NORMALINTS = (UART_IER_ERBFI | UART_IER_ELSI | UART_IER_EDSSI),
	UART_IER_HW_ALLINTS = (UART_IER_ERBFI | UART_IER_ETBEI | \
			       UART_IER_ELSI | UART_IER_EDSSI)
};

/* FCR */
enum {
	UART_FCR_FIFOE = (1 << 0),
	UART_FCR_CLRR = (1 << 1),
	UART_FCR_CLRT = (1 << 2),
	UART_FCR_DMA1 = (1 << 3),
	UART_FCR_RXFIFO_1B_TRI = (0 << 6),
	UART_FCR_RXFIFO_6B_TRI = (1 << 6),
	UART_FCR_RXFIFO_12B_TRI = (2 << 6),
	UART_FCR_RXFIFO_RX_TRI = (3 << 6),
	UART_FCR_TXFIFO_1B_TRI = (0 << 4),
	UART_FCR_TXFIFO_4B_TRI = (1 << 4),
	UART_FCR_TXFIFO_8B_TRI = (2 << 4),
	UART_FCR_TXFIFO_14B_TRI = (3 << 4),
	UART_FCR_FIFO_INIT = (UART_FCR_FIFOE | UART_FCR_CLRR | UART_FCR_CLRT),
	UART_FCR_NORMAL = (UART_FCR_FIFO_INIT |	\
			   UART_FCR_TXFIFO_4B_TRI | \
			   UART_FCR_RXFIFO_12B_TRI)
};

/* LCR */
enum {
	UART_LCR_BREAK = (1 << 6),
	UART_LCR_DLAB = (1 << 7),
	UART_WLS_5 = (0 << 0),
	UART_WLS_6 = (1 << 0),
	UART_WLS_7 = (2 << 0),
	UART_WLS_8 = (3 << 0),
	UART_WLS_MASK = (3 << 0),
	UART_1_STOP = (0 << 2),
	UART_2_STOP = (1 << 2),
	UART_1_5_STOP = (1 << 2),    /* Only when WLS=5 */
	UART_STOP_MASK = (1 << 2),
	UART_NONE_PARITY = (0 << 3),
	UART_ODD_PARITY = (0x1 << 3),
	UART_EVEN_PARITY = (0x3 << 3),
	UART_MARK_PARITY = (0x5 << 3),
	UART_SPACE_PARITY = (0x7 << 3),
	UART_PARITY_MASK = (0x7 << 3)
};

/* MCR */
enum {
	UART_MCR_DTR = (1 << 0),
	UART_MCR_RTS = (1 << 1),
	UART_MCR_OUT1 = (1 << 2),
	UART_MCR_OUT2 = (1 << 3),
	UART_MCR_LOOP = (1 << 4),
	UART_MCR_XOFF = (1 << 7),    /* read only */
	UART_MCR_NORMAL = (UART_MCR_DTR | UART_MCR_RTS)
};

/* LSR */
enum {
	UART_LSR_DR = (1 << 0),
	UART_LSR_OE = (1 << 1),
	UART_LSR_PE = (1 << 2),
	UART_LSR_FE = (1 << 3),
	UART_LSR_BI = (1 << 4),
	UART_LSR_THRE = (1 << 5),
	UART_LSR_TEMT = (1 << 6),
	UART_LSR_FIFOERR = (1 << 7)
};

/* MSR */
enum {
	UART_MSR_DCTS = (1 << 0),
	UART_MSR_DDSR = (1 << 1),
	UART_MSR_TERI = (1 << 2),
	UART_MSR_DDCD = (1 << 3),
	UART_MSR_CTS = (1 << 4),
	UART_MSR_DSR = (1 << 5),
	UART_MSR_RI = (1 << 6),
	UART_MSR_DCD = (1 << 7)
};

/* EFR */
enum {
	UART_EFR_EN = (1 << 4),
	UART_EFR_AUTO_RTS = (1 << 6),
	UART_EFR_AUTO_CTS = (1 << 7),
	UART_EFR_SW_CTRL_MASK = (0xf << 0),
	UART_EFR_NO_SW_CTRL = 0,
	UART_EFR_NO_FLOW_CTRL = 0,
	UART_EFR_AUTO_RTSCTS = (UART_EFR_AUTO_RTS | UART_EFR_AUTO_CTS),
	UART_EFR_XON1_XOFF1 = 0xa,      /* TX/RX XON1/XOFF1 flow control */
	UART_EFR_XON2_XOFF2 = 0x5,      /* TX/RX XON2/XOFF2 flow control */
	UART_EFR_XON12_XOFF12 = 0xf,    /* TX/RX XON1,2/XOFF1,2 flow control */
	UART_EFR_XON1_XOFF1_MASK = 0xa,
	UART_EFR_XON2_XOFF2_MASK = 0x5
};

/* IIR (Read Only) */
enum {
	UART_IIR_NO_INT_PENDING = 0x01,
	UART_IIR_RLS = 0x06,            /* Receiver Line Status */
	UART_IIR_RDA = 0x04,            /* Receive Data Available */
	UART_IIR_CTI = 0x0C,            /* Character Timeout Indicator */
	UART_IIR_THRE = 0x02,           /* Transmit Holding Register Empty*/
	UART_IIR_MS = 0x00,             /* Check Modem Status Register */
	UART_IIR_SW_FLOW_CTRL = 0x10,   /* Receive XOFF characters */
	UART_IIR_HW_FLOW_CTRL = 0x20,   /* CTS or RTS Rising Edge */
	UART_IIR_FIFO_EN = 0xc0,
	UART_IIR_INT_MASK = 0x1f
};

/* RateFix */
enum {
	UART_RATE_FIX = (1 << 0),
	UART_FREQ_SEL = (1 << 1),
	UART_RATE_FIX_13M = (1 << 0), /* means UARTclk = APBclk / 4 */
	UART_AUTORATE_FIX_13M = (1 << 1),
	UART_FREQ_SEL_13M = (1 << 2),
	UART_RATE_FIX_ALL_13M = (UART_RATE_FIX_13M | UART_AUTORATE_FIX_13M | \
				 UART_FREQ_SEL_13M),
	UART_RATE_FIX_26M = (0 << 0), /* means UARTclk = APBclk / 2 */
	UART_AUTORATE_FIX_26M = (0 << 1),
	UART_FREQ_SEL_26M = (0 << 2),
	UART_RATE_FIX_ALL_26M = (UART_RATE_FIX_26M | UART_AUTORATE_FIX_26M | \
				 UART_FREQ_SEL_26M),
	UART_RATE_FIX_32M5 = (0 << 0),    /* means UARTclk = APBclk / 2 */
	UART_FREQ_SEL_32M5 = (0 << 1),
	UART_RATE_FIX_ALL_32M5 = (UART_RATE_FIX_32M5 | UART_FREQ_SEL_32M5),
	UART_RATE_FIX_16M25 = (0 << 0),    /* means UARTclk = APBclk / 4 */
	UART_FREQ_SEL_16M25 = (0 << 1),
	UART_RATE_FIX_ALL_16M25 = (UART_RATE_FIX_16M25 | UART_FREQ_SEL_16M25)
};

/* Autobaud sample */
enum {
	UART_AUTOBADUSAM_13M = 7,
	UART_AUTOBADUSAM_26M = 15,
	UART_AUTOBADUSAM_52M = 31,
	UART_AUTOBAUDSAM_58_5M = 31  /* 31 or 32 ? */
};

typedef enum {
	NORMAL_BOOT = 0,
	META_BOOT = 1,
	RECOVERY_BOOT = 2,
	SW_REBOOT = 3,
	FACTORY_BOOT = 4,
	ADVMETA_BOOT = 5,
	ATE_FACTORY_BOOT = 6,
	ALARM_BOOT = 7,
	FASTBOOT = 99,
	DOWNLOAD_BOOT = 100,
	UNKNOWN_BOOT
} BOOTMODE;

#define VER_BASE        (0x08000000)
#define APHW_CODE       (VER_BASE)

void mt_reg_sync_writel(unsigned int v, unsigned long a);
void mt_reg_sync_writew(unsigned short v, unsigned long a);
void mt_reg_sync_writeb(unsigned char v, unsigned long a);
void mtk_set_current_uart(MTK_UART uart_base);

void uart_init(void);
void uart_init_early(void);
void uart_setbrg(void);
void serial_puts(unsigned int *s);
void custom_port_in_kernel(BOOTMODE boot_mode, char *command);
#endif /* !__MT65XX_UART_H__ */
