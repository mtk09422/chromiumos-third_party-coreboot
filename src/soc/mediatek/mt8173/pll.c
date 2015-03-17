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

#include <delay.h>
#include <arch/io.h>
#include <soc/addressmap.h>
#include <soc/pll.h>
#include <soc/pmic.h>
#include <timer.h>
#include <soc/spm.h>

#include <soc/dramc_pi_api.h>
#include <soc/dramc_common.h>
#include <soc/dramc_register.h>

extern DRAMC_CTX_T DramCtx_LPDDR3;

static void mt_mempll_init(DRAMC_CTX_T *p)
{
	MemPllPreInit((DRAMC_CTX_T *) p);
	MemPllInit((DRAMC_CTX_T *) p);
	return;
}

void mt_mempll_cali(DRAMC_CTX_T *p)
{
	/* called after chA and chB init done */
	/* MEMPLL05 registers, some are located @ chA and others are @ chB */
	p->channel = CHANNEL_A;
	DramcPllPhaseCal(p);
	p->channel = CHANNEL_B;
	DramcPllPhaseCal(p);

	/* Should only be called after channel A/B MEMPLL phase calibration had been done. */
	/* DramCPllGroupsCal(p);   //no need for MT8173 , open would let channel B fail */
	return;
}

static void mt_mempll_pre(void)
{
	DRAMC_CTX_T *psDramCtx;

	psDramCtx = &DramCtx_LPDDR3;
	printk(BIOS_DEBUG, "[PLL] mempll_init\n");
	mt_mempll_init(psDramCtx);
	return;
}

static void mt_mempll_post(void)
{
	DRAMC_CTX_T *psDramCtx;

	psDramCtx = &DramCtx_LPDDR3;
	printk(BIOS_DEBUG, "[PLL] mempll_cali\n");
	mt_mempll_cali(psDramCtx);
	return;
}

static void set_freq(int mode, unsigned long clks)
{
	int wait_pll = 0;

	if (clks & FCLK_VENCPLL) {
		if (mode == FREQ_SP) {
			/* 660 MHz */
			DRV_WriteReg32(VENCPLL_CON1, 0x800CB13B);
		} else if (mode == FREQ_HP) {
			/* 800 MHz */
			DRV_WriteReg32(VENCPLL_CON1, 0x800F6276);
		}
		wait_pll = 1;
	}

	if (clks & FCLK_VCODECPLL) {
		if (mode == FREQ_SP) {
			/* 384 MHz (1152 / 3) */
			DRV_WriteReg32(VCODECPLL_CON1, 0x800B13B1);
		} else if (mode == FREQ_HP) {
			/* 494 MHz (1482 / 3) */
			DRV_WriteReg32(VCODECPLL_CON1, 0x800E4000);
		}
		wait_pll = 1;
	}

	if (clks & FCLK_MMPLL) {
		if (mode == FREQ_SP) {
			/* 455 MHz */
			DRV_WriteReg32(MMPLL_CON1, 0x82118000);
		} else if (mode == FREQ_HP) {
			/* 600 MHz */
			DRV_WriteReg32(MMPLL_CON1, 0x821713B1);
		}
		wait_pll = 1;
	}

	if (clks & FCLK_AXI_CK) {
		if (mode == FREQ_SP) {
			/* axi_ck = 208 MHz (UNIVPLL2_D2) */
			DRV_WriteReg32(CLK_CFG_0, DRV_ReadReg32(CLK_CFG_0) &
				       (0xFFFFFF00 | 0x00000005));
		} else if (mode == FREQ_HP) {
			/* axi_ck = 273 MHz (SYSPLL1_D2) */
			DRV_WriteReg32(CLK_CFG_0, DRV_ReadReg32(CLK_CFG_0) &
				       (0xFFFFFF00 | 0x00000001));
		}
	}

	if (clks & FCLK_VENCLT_CK) {
		if (mode == FREQ_SP) {
			/* venclite_ck = 312 MHz (UNIVPLL1_D2) */
			DRV_WriteReg32(CLK_CFG_5, DRV_ReadReg32(CLK_CFG_5) &
				       (0x00FFFFFF | 0x06000000));
		} else if (mode == FREQ_HP) {
			/* venclite_ck = 370.5 MHz (VCODECPLL_370P5_CK) */
			DRV_WriteReg32(CLK_CFG_5, DRV_ReadReg32(CLK_CFG_5) &
				       (0x00FFFFFF | 0x0A000000));
		}
	}

	if (clks & FCLK_CCI400_CK) {
		if (mode == FREQ_SP) {
			/* cci400_ck = 546 MHz (SYSPLL_D2) */
			DRV_WriteReg32(CLK_CFG_6, DRV_ReadReg32(CLK_CFG_6) &
				       (0xFF00FFFF | 0x00050100));
		} else if (mode == FREQ_HP) {
			/* cci400_ck = 624 MHz (UNIVPLL_D2) */
			DRV_WriteReg32(CLK_CFG_6, DRV_ReadReg32(CLK_CFG_6) &
				       (0xFF00FFFF | 0x00040100));
		}
	}

	if (wait_pll)
		udelay(5);

}

void mt_vcore_pll_adjust(void)
{
	/* Vcore 1.125v */
	pmic_config_interface(VCORE_CON9, 0x44, 0x7F, 0);
	pmic_config_interface(VCORE_CON10, 0x44, 0x7F, 0);

	set_freq(FREQ_HP, FCLK_MMPLL);
	set_freq(FREQ_HP, FCLK_AXI_CK);
	set_freq(FREQ_HP, FCLK_CCI400_CK);
	set_freq(FREQ_HP, FCLK_VENCPLL);
	set_freq(FREQ_HP, FCLK_VCODECPLL);
	set_freq(FREQ_HP, FCLK_VENCLT_CK);
}

/* after pmic_init */
void mt_pll_post_init(void)
{
	unsigned int reg_value;

	mt_mempll_pre();
	mt_mempll_post();

	/* set mem_clk */
	DRV_WriteReg32(CLK_CFG_0, 0x01000105); /* ddrphycfg_ck = 26MHz */

	/* step 48 Only UNIVPLL SW Control */
	reg_value = DRV_ReadReg32(AP_PLL_CON3);
	DRV_WriteReg32(AP_PLL_CON3, reg_value & 0xFFF44440);

	/* step 49Only UNIVPLL SW Control */
	reg_value = DRV_ReadReg32(AP_PLL_CON4);
	DRV_WriteReg32(AP_PLL_CON4, reg_value & 0xFFFFFFF4);
}

/* after pmic_init */
void mt_arm_pll_sel(void)
{
	unsigned int reg_value;

	reg_value = DRV_ReadReg32(TOP_CKDIV1);
	DRV_WriteReg32(TOP_CKDIV1, reg_value & 0xFFFFFC00); /* CPU clock divide by 1 */

	DRV_WriteReg32(TOP_CKMUXSEL, 0x0245); /* select ARMPLL */

	printk(BIOS_DEBUG, "[PLL] %s done\n", __func__);
}

