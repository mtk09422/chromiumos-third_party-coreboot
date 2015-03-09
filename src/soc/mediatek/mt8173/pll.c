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
#include <timer.h>
#include <soc/spm.h>

/* after pmic_init */
void mt_pll_post_init(void)
{
	unsigned int reg_value;

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
}

