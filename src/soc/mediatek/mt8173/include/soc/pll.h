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

#ifndef SOC_MEDIATEK_MT8173_PLL_H
#include <soc/addressmap.h>
#include <soc/dramc_pi_api.h>
#include "custom_emi.h"
#define SOC_MEDIATEK_MT8173_PLL_H

enum {
	APMIXED_BASE           = 0x10209000,
	CKSYS_BASE             = 0x10000000,
	MMSYS_CONFIG_BASE      = 0x14000000,
	MCUCFG_BASE            = 0x10200000,
};

/* APMIXEDSYS Register */
enum {
	AP_PLL_CON0            = APMIXED_BASE + 0x00,
	AP_PLL_CON1            = APMIXED_BASE + 0x04,
	AP_PLL_CON2            = APMIXED_BASE + 0x08,
	AP_PLL_CON3            = APMIXED_BASE + 0x0C,
	AP_PLL_CON4            = APMIXED_BASE + 0x10,
	AP_PLL_CON5            = APMIXED_BASE + 0x14,
	AP_PLL_CON6            = APMIXED_BASE + 0x18,
	AP_PLL_CON7            = APMIXED_BASE + 0x1C,
	CLKSQ_STB_CON0         = APMIXED_BASE + 0x20,
	PLL_PWR_CON0           = APMIXED_BASE + 0x24,
	PLL_PWR_CON1           = APMIXED_BASE + 0x28,
	PLL_ISO_CON0           = APMIXED_BASE + 0x2C,
	PLL_ISO_CON1           = APMIXED_BASE + 0x30,
	PLL_STB_CON0           = APMIXED_BASE + 0x34,
	DIV_STB_CON0           = APMIXED_BASE + 0x38,
	PLL_CHG_CON0           = APMIXED_BASE + 0x3C,
	PLL_TEST_CON0          = APMIXED_BASE + 0x40,
};

enum {
	ARMCA15PLL_CON0        = APMIXED_BASE + 0x200,
	ARMCA15PLL_CON1        = APMIXED_BASE + 0x204,
	ARMCA15PLL_CON2        = APMIXED_BASE + 0x208,
	ARMCA15PLL_PWR_CON0    = APMIXED_BASE + 0x20C,
};

enum {
	ARMCA7PLL_CON0         = APMIXED_BASE + 0x210,
	ARMCA7PLL_CON1         = APMIXED_BASE + 0x214,
	ARMCA7PLL_CON2         = APMIXED_BASE + 0x218,
	ARMCA7PLL_PWR_CON0     = APMIXED_BASE + 0x21C,
};

enum {
	MAINPLL_CON0           = APMIXED_BASE + 0x220,
	MAINPLL_CON1           = APMIXED_BASE + 0x224,
	MAINPLL_PWR_CON0       = APMIXED_BASE + 0x22C,
};

enum {
	UNIVPLL_CON0           = APMIXED_BASE + 0x230,
	UNIVPLL_CON1           = APMIXED_BASE + 0x234,
	UNIVPLL_PWR_CON0       = APMIXED_BASE + 0x23C,
};

enum {
	MMPLL_CON0             = APMIXED_BASE + 0x240,
	MMPLL_CON1             = APMIXED_BASE + 0x244,
	MMPLL_CON2             = APMIXED_BASE + 0x248,
	MMPLL_PWR_CON0         = APMIXED_BASE + 0x24C,
};

enum {
	MSDCPLL_CON0           = APMIXED_BASE + 0x250,
	MSDCPLL_CON1           = APMIXED_BASE + 0x254,
	MSDCPLL_PWR_CON0       = APMIXED_BASE + 0x25C,
};

enum {
	VENCPLL_CON0           = APMIXED_BASE + 0x260,
	VENCPLL_CON1           = APMIXED_BASE + 0x264,
	VENCPLL_PWR_CON0       = APMIXED_BASE + 0x26C,
};

enum {
	TVDPLL_CON0            = APMIXED_BASE + 0x270,
	TVDPLL_CON1            = APMIXED_BASE + 0x274,
	TVDPLL_PWR_CON0        = APMIXED_BASE + 0x27C,
};

enum {
	MPLL_CON0              = APMIXED_BASE + 0x280,
	MPLL_CON1              = APMIXED_BASE + 0x284,
	MPLL_PWR_CON0          = APMIXED_BASE + 0x28C,
};

enum {
	VCODECPLL_CON0         = APMIXED_BASE + 0x290,
	VCODECPLL_CON1         = APMIXED_BASE + 0x294,
	VCODECPLL_PWR_CON0     = APMIXED_BASE + 0x29C,
};

enum {
	APLL1_CON0             = APMIXED_BASE + 0x2A0,
	APLL1_CON1             = APMIXED_BASE + 0x2A4,
	APLL1_CON2             = APMIXED_BASE + 0x2A8,
	APLL1_CON3             = APMIXED_BASE + 0x2AC,
	APLL1_PWR_CON0         = APMIXED_BASE + 0x2B0,
};

enum {
	APLL2_CON0             = APMIXED_BASE + 0x2B4,
	APLL2_CON1             = APMIXED_BASE + 0x2B8,
	APLL2_CON2             = APMIXED_BASE + 0x2BC,
	APLL2_CON3             = APMIXED_BASE + 0x2C0,
	APLL2_PWR_CON0         = APMIXED_BASE + 0x2C4,
};

enum {
	LVDSPLL_CON0           = APMIXED_BASE + 0x02D0,
	LVDSPLL_CON1           = APMIXED_BASE + 0x02D4,
	LVDSPLL_CON2           = APMIXED_BASE + 0x02D8,
	LVDSPLL_PWR_CON0       = APMIXED_BASE + 0x02DC,
};

enum {
	MSDCPLL2_CON0          = APMIXED_BASE + 0x02F0,
	MSDCPLL2_CON1          = APMIXED_BASE + 0x02F4,
	MSDCPLL2_CON2          = APMIXED_BASE + 0x02F8,
	MSDCPLL2_PWR_CON0      = APMIXED_BASE + 0x02FC,
};

enum {
	AP_AUXADC_CON0         = APMIXED_BASE + 0x400,
	AP_AUXADC_CON1         = APMIXED_BASE + 0x404,
	TS_CON0                = APMIXED_BASE + 0x600,
	TS_CON1                = APMIXED_BASE + 0x604,
	AP_ABIST_MON_CON0      = APMIXED_BASE + 0x800,
	AP_ABIST_MON_CON1      = APMIXED_BASE + 0x804,
	AP_ABIST_MON_CON2      = APMIXED_BASE + 0x808,
	AP_ABIST_MON_CON3      = APMIXED_BASE + 0x80C,
	OCCSCAN_CON            = APMIXED_BASE + 0x810,
	CLKDIV_CON0            = APMIXED_BASE + 0x814,
};

/* TOPCKGEN Register */
enum {
	CLK_MODE               = CKSYS_BASE + 0x000,
	DCM_CFG                = CKSYS_BASE + 0x004,
	TST_SEL_0              = CKSYS_BASE + 0x020,
	TST_SEL_1              = CKSYS_BASE + 0x024,
	TST_SEL_2              = CKSYS_BASE + 0x028,
	CLK_CFG_0              = CKSYS_BASE + 0x040,
	CLK_CFG_1              = CKSYS_BASE + 0x050,
	CLK_CFG_2              = CKSYS_BASE + 0x060,
	CLK_CFG_3              = CKSYS_BASE + 0x070,
	CLK_CFG_4              = CKSYS_BASE + 0x080,
	CLK_CFG_5              = CKSYS_BASE + 0x090,
	CLK_CFG_6              = CKSYS_BASE + 0x0A0,
	CLK_CFG_7              = CKSYS_BASE + 0x0B0,
	CLK_CFG_8              = CKSYS_BASE + 0x100,
	CLK_CFG_9              = CKSYS_BASE + 0x104,
	CLK_CFG_10             = CKSYS_BASE + 0x108,
	CLK_CFG_11             = CKSYS_BASE + 0x10C,
	CLK_CFG_12             = CKSYS_BASE + 0x0C0,
	CLK_CFG_13             = CKSYS_BASE + 0x0D0,
	CLK_SCP_CFG_0          = CKSYS_BASE + 0x200,
	CLK_SCP_CFG_1          = CKSYS_BASE + 0x204,
	CLK_MISC_CFG_0         = CKSYS_BASE + 0x210,
	CLK_MISC_CFG_1         = CKSYS_BASE + 0x214,
	CLK_MISC_CFG_2         = CKSYS_BASE + 0x218,
	CLK26CALI_0            = CKSYS_BASE + 0x220,
	CLK26CALI_1            = CKSYS_BASE + 0x224,
	CLK26CALI_2            = CKSYS_BASE + 0x228,
	CKSTA_REG              = CKSYS_BASE + 0x22C,
	TEST_MODE_CFG          = CKSYS_BASE + 0x230,
	MBIST_CFG_0            = CKSYS_BASE + 0x308,
	MBIST_CFG_1            = CKSYS_BASE + 0x30C,
	RESET_DEGLITCH_KEY     = CKSYS_BASE + 0x310,
	MBIST_CFG_3            = CKSYS_BASE + 0x314,
};

/* INFRASYS Register */
enum {
	TOP_CKMUXSEL           = INFRACFG_AO_BASE + 0x00,
	TOP_CKDIV1             = INFRACFG_AO_BASE + 0x08,
	TOP_DCMCTL             = INFRACFG_AO_BASE + 0x10,

	TOPAXI_PROT_EN         = INFRACFG_AO_BASE + 0x0220,
	TOPAXI_PROT_STA1       = INFRACFG_AO_BASE + 0x0228,

	TOPAXI_PROT_EN1        = INFRACFG_AO_BASE + 0x0250,
	TOPAXI_PROT_STA3       = INFRACFG_AO_BASE + 0x0258,
};

/* MCUSYS Register */
enum {
	IR_ROSC_CTL            = MCUCFG_BASE + 0x030,
	CA15L_MON_SEL          = CA15L_CONFIG_BASE + 0x01C,
};

/* MMSYS Register*/
enum {
	DISP_CG_CON0           = MMSYS_CONFIG_BASE + 0x100,
	DISP_CG_SET0           = MMSYS_CONFIG_BASE + 0x104,
	DISP_CG_CLR0           = MMSYS_CONFIG_BASE + 0x108,
	DISP_CG_CON1           = MMSYS_CONFIG_BASE + 0x110,
	DISP_CG_SET1           = MMSYS_CONFIG_BASE + 0x114,
	DISP_CG_CLR1           = MMSYS_CONFIG_BASE + 0x118,
};

enum {
	SMI_LARB0_BASE         = 0x14010000,
	SMI_LARB0_STAT         = SMI_LARB0_BASE+0x000,
	SMI_LARB0_OSTD_CTRL_EN = SMI_LARB0_BASE+0x064,
};

enum {
	ARMCA15PLL = 0,
	ARMCA7PLL  = 1,
	MAINPLL    = 2,
	MSDCPLL    = 3,
	UNIVPLL    = 4,
	MMPLL      = 5,
	VENCPLL    = 6,
	TVDPLL     = 7,
	MPLL       = 8,
	VCODECPLL  = 9,
	APLL1      = 10,
	APLL2      = 11,
	NR_PLLS    = 12,
};

/* for MTCMOS */
enum {
	STA_POWER_DOWN	= 0,
	STA_POWER_ON	= 1,
};

enum {
	DIS_PWR_STA_MASK   = 0x1 << 3,
	AUD_PWR_STA_MASK   = 0x1 << 24,
};


enum {
	PWR_RST_B          = 0x1 << 0,
	PWR_ISO            = 0x1 << 1,
	PWR_ON             = 0x1 << 2,
	PWR_ON_S           = 0x1 << 3,
	PWR_CLK_DIS        = 0x1 << 4,
};

enum {
	SRAM_PDN           = 0xf << 8,
	DIS_SRAM_ACK       = 0x1 << 12,
	AUD_SRAM_ACK       = 0xf << 12,
};

enum {
	MD1_PROT_MASK      = 0x04B8, /* bit 3,4,5,7,10 */
	MD_SRAM_PDN        = 0x1 << 8,
	MD1_PWR_STA_MASK   = 0x1 << 0,
};

enum {
	FREQ_SP,
	FREQ_HP
};

enum {
	FCLK_VENCPLL	= 0x1 << 0,
	FCLK_VCODECPLL	= 0x1 << 1,
	FCLK_MMPLL	= 0x1 << 2,
	FCLK_AXI_CK	= 0x1 << 3,
	FCLK_VENCLT_CK	= 0x1 << 4,
	FCLK_CCI400_CK	= 0x1 << 5
};

int spm_mtcmos_ctrl_disp(int state);
int spm_mtcmos_ctrl_audio(int state);
void mt_pll_post_init(void);
void mt_pll_init(void);
void mt_mempll_cali(DRAMC_CTX_T *p);
void mt_arm_pll_sel(void);
void mt_vcore_pll_adjust(void);

#define DRV_WriteReg32(addr, val)	write32( (void *)(uintptr_t)(addr), (val))
#define DRV_ReadReg32(addr)		read32((void *)(uintptr_t)(addr))

#endif /* SOC_MEDIATEK_MT8173_PLL_H */
