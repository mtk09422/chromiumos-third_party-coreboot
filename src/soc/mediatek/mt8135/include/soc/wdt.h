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

#ifndef SOC_MEDIATEK_MT8135_WDT_H
#define SOC_MEDIATEK_MT8135_WDT_H

#include <stdint.h>

enum {
	MTK_WDT_BASE = RGU_BASE,

	MTK_WDT_MODE = MTK_WDT_BASE + 0x0000,
	MTK_WDT_LENGTH = MTK_WDT_BASE + 0x0004,
	MTK_WDT_RESTART	= MTK_WDT_BASE + 0x0008,
	MTK_WDT_STATUS = MTK_WDT_BASE + 0x000C,
	MTK_WDT_INTERVAL = MTK_WDT_BASE + 0x0010,
	MTK_WDT_SWRST = MTK_WDT_BASE + 0x0014,
	MTK_WDT_SWSYSRST = MTK_WDT_BASE + 0x0018,
	MTK_WDT_NONRST_REG = MTK_WDT_BASE + 0x0020,
	MTK_WDT_NONRST_REG2 = MTK_WDT_BASE + 0x0024,
	MTK_WDT_REQ_MODE = MTK_WDT_BASE + 0x0030,
	MTK_WDT_REQ_IRQ_EN = MTK_WDT_BASE + 0x0034,
	MTK_WDT_DRAMC_CTL = MTK_WDT_BASE + 0x0040
};

/*WDT_MODE*/
enum {
	MTK_WDT_MODE_KEYMASK = 0xff00,
	MTK_WDT_MODE_KEY = 0x22000000,

	MTK_WDT_MODE_DUAL_MODE = 0x0040,
	MTK_WDT_MODE_IN_DIS = 0x0020,	/* Reserved */
	MTK_WDT_MODE_AUTO_RESTART = 0x0010,	/* Reserved */
	MTK_WDT_MODE_IRQ = 0x0008,
	MTK_WDT_MODE_EXTEN = 0x0004,
	MTK_WDT_MODE_EXT_POL = 0x0002,
	MTK_WDT_MODE_ENABLE = 0x0001
};

/*WDT_LENGTH*/
enum {
	MTK_WDT_LENGTH_TIME_OUT = 0xffe0,
	MTK_WDT_LENGTH_KEYMASK = 0x001f,
	MTK_WDT_LENGTH_KEY = 0x0008
};

/*WDT_RESTART*/
enum {
	MTK_WDT_RESTART_KEY = 0x1971
};

/*WDT_STATUS*/
enum {
	MTK_WDT_STATUS_HWWDT_RST = 0x80000000,
	MTK_WDT_STATUS_SWWDT_RST = 0x40000000,
	MTK_WDT_STATUS_IRQWDT_RST = 0x20000000,
	MTK_WDT_STATUS_DEBUGWDT_RST = 0x00080000,
	MTK_WDT_STATUS_SPMWDT_RST = 0x0001
};

/*WDT_INTERVAL*/
enum {
	MTK_WDT_INTERVAL_MASK = 0x0fff
};

/*WDT_SWRST*/
enum {
	MTK_WDT_SWRST_KEY = 0x1209
};

/*WDT_SWSYSRST*/
enum {
	MTK_WDT_SWSYS_RST_KEY = 0x88000000,
	MTK_WDT_SWSYS_RST_PWRAP_SPI_CTL_RST = 0x0800,
	MTK_WDT_SWSYS_RST_APMIXED_RST = 0x0400,
	MTK_WDT_SWSYS_RST_MD_LITE_RST = 0x0200,
	MTK_WDT_SWSYS_RST_INFRA_AO_RST = 0x0100,
	MTK_WDT_SWSYS_RST_MD_RST = 0x0080,
	MTK_WDT_SWSYS_RST_DDRPHY_RST = 0x0040,
	MTK_WDT_SWSYS_RST_IMG_RST = 0x0020,
	MTK_WDT_SWSYS_RST_VDEC_RST = 0x0010,
	MTK_WDT_SWSYS_RST_VENC_RST = 0x0008,
	MTK_WDT_SWSYS_RST_MFG_RST = 0x0004,
	MTK_WDT_SWSYS_RST_DISP_RST = 0x0002,
	MTK_WDT_SWSYS_RST_INFRA_RST = 0x0001
};

/* WDT_NONRST_REG */
enum {
	MTK_WDT_NONRST_DL = 0x00008000
};

/* Reboot reason */
enum {
	RE_BOOT_REASON_UNKNOWN = 0x00,
	RE_BOOT_BY_WDT_HW = 0x01,
	RE_BOOT_BY_WDT_SW = 0x02,
	RE_BOOT_WITH_INTTERUPT = 0x04,
	RE_BOOT_BY_SPM_THERMAL = 0x08,

	RE_BOOT_ABNORMAL = 0xF0
};

/* Reboot from which stage */
enum {
	RE_BOOT_FROM_UNKNOWN = 0x00,
	RE_BOOT_FROM_PRE_LOADER = 0x01,
	RE_BOOT_FROM_U_BOOT = 0x02,
	RE_BOOT_FROM_KERNEL = 0x03,
	RE_BOOT_FROM_POWER_ON = 0x04,

	WDT_NORMAL_REBOOT = 0x01,
	WDT_BY_PASS_PWK_REBOOT = 0x02,
	WDT_NOT_WDT_REBOOT = 0x00
};

extern unsigned int g_rgu_status;

void mtk_wdt_init(void);
uint8_t mtk_is_rgu_trigger_reset(void);
void mtk_arch_reset(uint8_t mode);
int mtk_wdt_boot_check(void);
void mtk_wdt_restart(void);

#endif /* SOC_MEDIATEK_MT8135_WDT_H */
