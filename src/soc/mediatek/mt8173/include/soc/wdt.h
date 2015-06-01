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

#ifndef SOC_MEDIATEK_MT8173_WDT_H
#define SOC_MEDIATEK_MT8173_WDT_H

#include <stdint.h>
#include <soc/addressmap.h>

struct mt8173_wdt_regs {
	u32 wdt_mode;
	u32 wdt_length;
	u32 wdt_restart;
	u32 wdt_status;
	u32 wdt_interval;
	u32 wdt_swrst;
	u32 wdt_swsysrst;
	u32 reserved[9];
	u32 wdt_debug_ctrl;
};

/*WDT_MODE*/
enum {
	MTK_WDT_MODE_KEYMASK = 0xff00,
	MTK_WDT_MODE_KEY = 0x22000000,

	MTK_WDT_MODE_DDR_RESERVE = 0x0080,

	MTK_WDT_MODE_DUAL_MODE = 0x0040,
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
	MTK_WDT_STATUS_SPMWDT_RST = 0x0002,
	MTK_WDT_STATUS_SPM_THERMAL_RST = 0x0001,
	MTK_WDT_STATUS_THERMAL_DIRECT_RST = 1<<18,
	MTK_WDT_STATUS_SECURITY_RST = 1<<28
};

/*WDT_SWRST*/
enum {
	MTK_WDT_SWRST_KEY = 0x1209
};

/*WDT_SWSYSRST*/
enum {
	MTK_WDT_SWSYS_RST_KEY = 0x88000000,
	MTK_WDT_SWSYS_RST_MD_RST = 0x0080,
};

/* WDT_NONRST_REG */
enum {
	MTK_WDT_NONRST_DL = 0x00008000
};

/* MTK_WDT_DEBUG_CTL */
enum {
	MTK_DEBUG_CTL_KEY = 0x59000000,
	MTK_RG_MCU_LATH_EN = 0x00002,
};

/* Reboot reason */
enum {
	RE_BOOT_REASON_UNKNOW = 0x00,
	RE_BOOT_BY_WDT_HW = 0x01,
	RE_BOOT_BY_WDT_SW = 0x02,
	RE_BOOT_WITH_INTTERUPT = 0x04,
	RE_BOOT_BY_SPM_THERMAL = 0x08,
	RE_BOOT_BY_SPM = 0x10,
	RE_BOOT_BY_THERMAL_DIRECT = 0x20,
	RE_BOOT_BY_DEBUG = 0x40,
	RE_BOOT_BY_SECURITY = 0x80,

	RE_BOOT_ABNORMAL = 0xF0
};

/* Reboot from which stage */
enum {
	WDT_NOT_WDT_REBOOT = 0x00,
	WDT_NORMAL_REBOOT = 0x01
};

void mtk_wdt_init(void);
int rgu_dram_reserved(int enable);
void rgu_swsys_reset(void);

#endif /* SOC_MEDIATEK_MT8173_WDT_H */

