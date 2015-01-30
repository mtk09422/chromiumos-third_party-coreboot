/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 MediaTek Inc.
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

#ifndef SOC_MEDIATEK_MT8135_MT8135_H
#define SOC_MEDIATEK_MT8135_MT8135_H

enum {
	IO_PHYS = 0x10000000,
	IO_SIZE = 0x00100000,
};

/*=======================================================================*/
/* Register Bases                                                        */
/*=======================================================================*/
enum {
	RGU_BASE = 0x10000000,
	CONFIG_BASE = 0x10001000,
	SRAMROM_BASE = 0x10002000,
	PERI_CON_BASE = 0x10003000,	/* CHECKME & FIXME */
	DRAMC0_BASE = 0x10004000,
	GPIO_BASE = 0x10005000,
	PLL_BASE = 0x10007000,
	GPT_BASE = 0x10008000,
	DDRPHY_BASE = 0x10011000,

	DEM_BASE = 0x1011A000,

	EMI_BASE = 0x10203000,
	GPIO1_BASE = 0x1020C000,
	DRAMC_NAO_BASE = 0x1020F000,

	NFI_BASE = 0x11001000,
	NFIECC_BASE = 0x11002000,

	UART0_BASE = 0x11006000,
	UART1_BASE = 0x11007000,
	UART2_BASE = 0x11008000,
	UART3_BASE = 0x11009000,

	I2C0_BASE = 0x1100D000,
	I2C1_BASE = 0x1100E000,
	KPD_BASE = 0x1100E000,	/* CHECKME & FIXME new design, */
	I2C2_BASE = 0x1100F000,
	I2C3_BASE = 0x11010000,
	I2C4_BASE = 0x11011000,
	I2C5_BASE = 0x11012000,
	I2C6_BASE = 0x11013000,

	SPI_BASE = 0x11016000,

	PWRAP_BRIDGE_BASE = 0x11021000,

	USB_BASE = 0x11200000,
	USB0_BASE = 0x11200000,
	USB1_BASE = 0x11210000,
	USBSIF_BASE = 0x11220000,

	MSDC0_BASE = 0x11230000,
	MSDC1_BASE = 0x11240000,
	MSDC2_BASE = 0x11250000,
	MSDC3_BASE = 0x11260000,
	MSDC4_BASE = 0x11270000,

	PMIC_WRAP_BASE = 0x1000F000,
	PERI_PWRAP_BRIDGE_BASE = 0x11017000
};

enum {
	VER_BASE = 0x08000000,
	APHW_CODE = VER_BASE,
	APHW_SUBCODE = VER_BASE + 0x04,
	APHW_VER = VER_BASE + 0x08,
	APSW_VER = VER_BASE + 0x0C,
};

enum {
	AP_PERI_GLOBALCON_RST0 = PERI_CON_BASE + 0x0,
	AP_PERI_GLOBALCON_PDN0 = PERI_CON_BASE + 0x10,
};

/*=======================================================================*/
/* UART download control                                                 */
/*=======================================================================*/

enum {
	UART0_SW_RST = 1 << 0,
	UART1_SW_RST = 1 << 1,
	UART2_SW_RST = 1 << 2,
	UART3_SW_RST = 1 << 3,
};

enum {
	UART0_PDN = 1 << 24,
	UART1_PDN = 1 << 25,
	UART2_PDN = 1 << 26,
	UART3_PDN = 1 << 27,
};

/*=======================================================================*/
/* USB download control                                                  */
/*=======================================================================*/
#define SRAMROM_USBDL       (SRAMROM_BASE + 0x0050)

#define USBDL_BIT_EN        (0x00000001)	/* 1: download bit enabled */
#define USBDL_PL            (0x00000002)	/* 0: usbdl by brom; 1: usbdl by bootloader */
#define USBDL_TIMEOUT_MASK  (0x0000FFFC)	/* 14-bit timeout: 0x0000~0x3FFE: second; 0x3FFFF: no timeout */
#define USBDL_TIMEOUT_MAX   (USBDL_TIMEOUT_MASK >> 2)	/* maximum timeout indicates no timeout */

#define MTK_EMMC_SUPPORT	    1
#define BOOTDEV_SDMMC		    (1)

#define CFG_BOOT_DEV                (BOOTDEV_SDMMC)
#define CFG_BATTERY_DETECT          (0)

#define CFG_UART_TOOL_HANDSHAKE     (1)
#define CFG_USB_TOOL_HANDSHAKE      (1)
#define CFG_USB_DOWNLOAD            (1)
#define CFG_PMT_SUPPORT             (0)

#define CFG_LOG_BAUDRATE            (921600)
#define CFG_META_BAUDRATE           (115200)
#define CFG_UART_LOG                (UART4)
#define CFG_UART_META               (UART1)

#define CFG_EMERGENCY_DL_SUPPORT    (1)
#define CFG_EMERGENCY_DL_TIMEOUT_MS (1000 * 60 * 5)	/* 5 mins */

#define FEATURE_MMC_ADDR_TRANS
#define WORKAROUND_GPU_VRF18_2
#undef EVB_PLATFORM

#define CFG_HW_WATCHDOG                 (1)
#define MT8135			(1)

#define LPDDR	2		/* TODO: VERIFY */

#define UNUSED	0
#define NOT_IMPLEMENTED	0

#endif /* SOC_MEDIATEK_MT8135_MT8135_H */
