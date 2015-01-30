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

#define BOOTROM_PWR_CTRL        ((volatile uint32_t *)0x10002020)
#define BOOTROM_PWR_ADDR        ((volatile uint32_t *)0x10002030)

#define POWERON_CONFIG_SET      ((volatile uint32_t *)0x10006000)
#define SPM_CA15_CX0_PWR_CON    ((volatile uint32_t *)0x1000629C)
#define SPM_CA15_CX1_PWR_CON    ((volatile uint32_t *)0x100062A0)
#define SPM_CA15_CPU0_PWR_CON   ((volatile uint32_t *)0x100062A4)
#define SPM_CA15_CPU1_PWR_CON   ((volatile uint32_t *)0x100062A8)
#define SPM_CA15_CPUTOP_PWR_CON ((volatile uint32_t *)0x100062AC)
#define SPM_CA15_L1_PWR_CON     ((volatile uint32_t *)0x100062B0)
#define SPM_CA15_L2_PWR_CON     ((volatile uint32_t *)0x100062B4)
#define SPM_MD1_PWR_CON         ((volatile uint32_t *)0x10006284)
#define SPM_PWR_STATUS          ((volatile uint32_t *)0x1000660C)
#define SPM_PWR_STATUS_S        ((volatile uint32_t *)0x10006610)
#define SPM_SLEEP_TIMER_STA     ((volatile uint32_t *)0x10006720)

#define CA15_CA7_CONNECT    ((volatile uint32_t *)0x10200018)
#define CA15_MISC_DBG       ((volatile uint32_t *)0x1020020C)
#define CA15_CCI400_DVM_EN  ((volatile uint32_t *)0x10324000)
#define CA7_CCI400_DVM_EN   ((volatile uint32_t *)0x10325000)
#define CONFIG_RES	        ((volatile uint32_t *)0x10200268)
#define RGUCFG              ((volatile uint32_t *)0x10200254)
#define CA15_RST_CTL        ((volatile uint32_t *)0x10200244)

#define SRAM_ISOINT_B   (1U << 6)
#define SRAM_CKISO      (1U << 5)
#define PWR_CLK_DIS     (1U << 4)
#define PWR_ON_S        (1U << 3)
#define PWR_ON          (1U << 2)
#define PWR_ISO         (1U << 1)
#define PWR_RST_B       (1U << 0)

#define CA15_CX0        (1U << 17)
#define CA15_CX1        (1U << 18)
#define CA15_CPU0       (1U << 19)
#define CA15_CPU1       (1U << 20)
#define CA15_CPUTOP     (1U << 21)

/* SPM_CA15 */
#define CPU0_L1_PDN_ACK (1U << 8)
#define CPU0_L1_PDN     (1U << 0)
#define CPU0_L1_PDN_ISO (1U << 4)
#define CPU1_L1_PDN_ACK (1U << 9)
#define CPU1_L1_PDN     (1U << 1)
#define CPU1_L1_PDN_ISO (1U << 5)

#define CA15_L2_PDN_ACK (1U << 8)
#define CA15_L2_PDN     (1U << 0)
#define CA15_L2_ISO     (1U << 4)

#define CA15_L2RSTDISABLE (1U << 10)
#define EXT_CA15_OFF      (1U << 1)

extern volatile int g_ca15_ready;

#define barrier() __asm__ __volatile__("" : : : "memory")

/* Slave CPU */
#define BOOT_SHARE_BASE 0x80002000
#define NS_SLAVE_JUMP_REG (BOOT_SHARE_BASE+1020)
#define NS_SLAVE_MAGIC_REG (BOOT_SHARE_BASE+1016)
#define NS_SLAVE_BOOT_ADDR (BOOT_SHARE_BASE+1012)

#define SLAVE1_MAGIC_NUM 0x534C4131
#define SLAVE2_MAGIC_NUM 0x4C415332
#define SLAVE3_MAGIC_NUM 0x41534C33

extern unsigned char _sram_m4u[];
extern unsigned char _esram_m4u[];
#define _sram_m4u_size (_esram_m4u - _sram_m4u)

#endif /* SOC_MEDIATEK_MT8135_MT8135_H */
