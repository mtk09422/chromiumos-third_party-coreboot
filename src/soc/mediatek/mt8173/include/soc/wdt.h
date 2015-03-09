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

enum {
	MTK_WDT_BASE = RGU_BASE,

	MTK_WDT_MODE = MTK_WDT_BASE + 0x0000,
	MTK_WDT_LENGTH = MTK_WDT_BASE + 0x0004,
	MTK_WDT_RESTART = MTK_WDT_BASE + 0x0008,
	MTK_WDT_STATUS = MTK_WDT_BASE + 0x000C,
	MTK_WDT_INTERVAL = MTK_WDT_BASE + 0x0010,
	MTK_WDT_SWRST = MTK_WDT_BASE + 0x0014,
	MTK_WDT_SWSYSRST = MTK_WDT_BASE + 0x0018,
	MTK_WDT_NONRST_REG = MTK_WDT_BASE + 0x0020,
	MTK_WDT_NONRST_REG2 = MTK_WDT_BASE + 0x0024,
	MTK_WDT_REQ_MODE = MTK_WDT_BASE + 0x0030,
	MTK_WDT_REQ_IRQ_EN = MTK_WDT_BASE + 0x0034,
	MTK_WDT_DEBUG_CTL = MTK_WDT_BASE + 0x0040
};

/*WDT_MODE*/
enum {
	MTK_WDT_MODE_KEYMASK = 0xff00,
	MTK_WDT_MODE_KEY = 0x22000000,

	MTK_WDT_MODE_DDR_RESERVE = 0x0080,
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
	MTK_WDT_STATUS_SPMWDT_RST = 0x0002,
	MTK_WDT_STATUS_SPM_THERMAL_RST = 0x0001,
	MTK_WDT_STATUS_THERMAL_DIRECT_RST = 1<<18,
	MTK_WDT_STATUS_SECURITY_RST = 1<<28
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

/* MTK_WDT_DEBUG_CTL */
enum {
	MTK_DEBUG_CTL_KEY = 0x59000000,
	MTK_RG_DDR_PROTECT_EN = 0x00001,
	MTK_RG_MCU_LATH_EN = 0x00002,
	MTK_RG_DRAMC_SREF = 0x00100,
	MTK_RG_DRAMC_ISO = 0x00200,
	MTK_RG_CONF_ISO = 0x00400,
	MTK_DDR_RESERVE_RTA = 0x10000,
	MTK_DDR_SREF_STA = 0x20000
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
	RE_BOOT_FROM_UNKNOW = 0x00,
	RE_BOOT_FROM_PRE_LOADER = 0x01,
	RE_BOOT_FROM_U_BOOT = 0x02,
	RE_BOOT_FROM_KERNEL = 0x03,
	RE_BOOT_FROM_POWER_ON = 0x04,

	WDT_NORMAL_REBOOT = 0x01,
	WDT_BY_PASS_PWK_REBOOT = 0x02,
	WDT_NOT_WDT_REBOOT = 0x00
};

typedef enum wd_swsys_reset_type {
	WD_MD_RST,
} WD_SYS_RST_TYPE;

void mtk_wdt_init(void);
uint8_t mtk_is_rgu_trigger_reset(void);
void mtk_arch_reset(uint8_t mode);
int mtk_wdt_boot_check(void);
uint32_t get_g_rgu_status(void);
int rgu_dram_reserved(int enable);
int rgu_is_reserve_ddr_enabled(void);
int rgu_is_dram_slf(void);

void rgu_release_rg_dramc_conf_iso(void);
void rgu_release_rg_dramc_iso(void);
void rgu_release_rg_dramc_sref(void);
int rgu_is_reserve_ddr_mode_success(void);
void rgu_swsys_reset(WD_SYS_RST_TYPE reset_type);

#endif /* SOC_MEDIATEK_MT8173_WDT_H */
