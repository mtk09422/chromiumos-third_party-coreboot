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
#include <soc/spm.h>

void mt_pll_init(void)
{
	unsigned int reg_value;

	DRV_WriteReg32(CLKSQ_STB_CON0, 0x05010501); /* reduce CLKSQ disable time */

	DRV_WriteReg32(PLL_ISO_CON0, 0x00080008); /* extend PWR/ISO control timing to 1us */

	DRV_WriteReg32(AP_PLL_CON6, 0x00000000); /*  */

	/*************
	* xPLL PWR ON
	**************/
	reg_value = DRV_ReadReg32(ARMCA15PLL_PWR_CON0);
	DRV_WriteReg32(ARMCA15PLL_PWR_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(ARMCA7PLL_PWR_CON0);
	DRV_WriteReg32(ARMCA7PLL_PWR_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(MAINPLL_PWR_CON0);
	DRV_WriteReg32(MAINPLL_PWR_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(UNIVPLL_PWR_CON0);
	DRV_WriteReg32(UNIVPLL_PWR_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(MMPLL_PWR_CON0);
	DRV_WriteReg32(MMPLL_PWR_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(MSDCPLL_PWR_CON0);
	DRV_WriteReg32(MSDCPLL_PWR_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(VENCPLL_PWR_CON0);
	DRV_WriteReg32(VENCPLL_PWR_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(TVDPLL_PWR_CON0);
	DRV_WriteReg32(TVDPLL_PWR_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(MPLL_PWR_CON0);
	DRV_WriteReg32(MPLL_PWR_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(VCODECPLL_PWR_CON0);
	DRV_WriteReg32(VCODECPLL_PWR_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(APLL1_PWR_CON0);
	DRV_WriteReg32(APLL1_PWR_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(APLL2_PWR_CON0);
	DRV_WriteReg32(APLL2_PWR_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(MSDCPLL2_PWR_CON0);
	DRV_WriteReg32(MSDCPLL2_PWR_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(LVDSPLL_PWR_CON0);
	DRV_WriteReg32(LVDSPLL_PWR_CON0, reg_value | 0x1);

	udelay(5); /* wait for xPLL_PWR_ON ready (min delay is 1us) */

	/******************
	* xPLL ISO Disable
	*******************/
	reg_value = DRV_ReadReg32(ARMCA15PLL_PWR_CON0);
	DRV_WriteReg32(ARMCA15PLL_PWR_CON0, reg_value & 0xFFFFFFFD);

	reg_value = DRV_ReadReg32(ARMCA7PLL_PWR_CON0);
	DRV_WriteReg32(ARMCA7PLL_PWR_CON0, reg_value & 0xFFFFFFFD);

	reg_value = DRV_ReadReg32(MAINPLL_PWR_CON0);
	DRV_WriteReg32(MAINPLL_PWR_CON0, reg_value & 0xFFFFFFFD);

	reg_value = DRV_ReadReg32(UNIVPLL_PWR_CON0);
	DRV_WriteReg32(UNIVPLL_PWR_CON0, reg_value & 0xFFFFFFFD);

	reg_value = DRV_ReadReg32(MMPLL_PWR_CON0);
	DRV_WriteReg32(MMPLL_PWR_CON0, reg_value & 0xFFFFFFFD);

	reg_value = DRV_ReadReg32(MSDCPLL_PWR_CON0);
	DRV_WriteReg32(MSDCPLL_PWR_CON0, reg_value & 0xFFFFFFFD);

	reg_value = DRV_ReadReg32(VENCPLL_PWR_CON0);
	DRV_WriteReg32(VENCPLL_PWR_CON0, reg_value & 0xFFFFFFFD);

	reg_value = DRV_ReadReg32(TVDPLL_PWR_CON0);
	DRV_WriteReg32(TVDPLL_PWR_CON0, reg_value & 0xFFFFFFFD);

	reg_value = DRV_ReadReg32(MPLL_PWR_CON0);
	DRV_WriteReg32(MPLL_PWR_CON0, reg_value & 0xFFFFFFFD);

	reg_value = DRV_ReadReg32(VCODECPLL_PWR_CON0);
	DRV_WriteReg32(VCODECPLL_PWR_CON0, reg_value & 0xFFFFFFFD);

	reg_value = DRV_ReadReg32(APLL1_PWR_CON0);
	DRV_WriteReg32(APLL1_PWR_CON0, reg_value & 0xFFFFFFFD);

	reg_value = DRV_ReadReg32(APLL2_PWR_CON0);
	DRV_WriteReg32(APLL2_PWR_CON0, reg_value & 0xFFFFFFFD);

	reg_value = DRV_ReadReg32(MSDCPLL2_PWR_CON0);
	DRV_WriteReg32(MSDCPLL2_PWR_CON0, reg_value & 0xFFFFFFFD);

	reg_value = DRV_ReadReg32(LVDSPLL_PWR_CON0);
	DRV_WriteReg32(LVDSPLL_PWR_CON0, reg_value & 0xFFFFFFFD);

	/********************
	* xPLL Frequency Set
	*********************/
	DRV_WriteReg32(ARMCA15PLL_CON1, 0x81106000); /* 851.5MHz */

	DRV_WriteReg32(ARMCA7PLL_CON1, 0x800AA000); /* 1105MHz */


	DRV_WriteReg32(MAINPLL_CON1, 0x800A8000); /* 1092MHz */

	DRV_WriteReg32(MMPLL_CON1, 0x82118000); /* 455MHz */

	DRV_WriteReg32(MSDCPLL_CON1, 0x800F6276); /* 800MHz */

	DRV_WriteReg32(VENCPLL_CON1, 0x800CB13B); /* 660MHz */

	DRV_WriteReg32(TVDPLL_CON1, 0x80112276); /* 445.5MHz */

	DRV_WriteReg32(MPLL_CON1, 0x801C0000);
	DRV_WriteReg32(MPLL_CON0, 0x00010110); /* 52MHz */

	DRV_WriteReg32(VCODECPLL_CON1, 0x800A9D89); /* 368 MHz */

	/*
	 * APLL1 and APLL2 use the default setting
	 * MSDCPLL2 use the default setting: 800MHz (0x800F6276)
	 * LVDSPLL use the default setting: 150MHz (0x800B89D9)
	 */
	/***********************
	* xPLL Frequency Enable
	************************/
	reg_value = DRV_ReadReg32(ARMCA15PLL_CON0);
	DRV_WriteReg32(ARMCA15PLL_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(ARMCA7PLL_CON0);
	DRV_WriteReg32(ARMCA7PLL_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(MAINPLL_CON0) & (~0x70);
	DRV_WriteReg32(MAINPLL_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(UNIVPLL_CON0);
	DRV_WriteReg32(UNIVPLL_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(MMPLL_CON0);
	DRV_WriteReg32(MMPLL_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(MSDCPLL_CON0);
	DRV_WriteReg32(MSDCPLL_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(VENCPLL_CON0);
	DRV_WriteReg32(VENCPLL_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(TVDPLL_CON0);
	DRV_WriteReg32(TVDPLL_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(MPLL_CON0);
	DRV_WriteReg32(MPLL_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(VCODECPLL_CON0);
	DRV_WriteReg32(VCODECPLL_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(APLL1_CON0);
	DRV_WriteReg32(APLL1_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(APLL2_CON0);
	DRV_WriteReg32(APLL2_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(MSDCPLL2_CON0);
	DRV_WriteReg32(MSDCPLL2_CON0, reg_value | 0x1);

	reg_value = DRV_ReadReg32(LVDSPLL_CON0);
	DRV_WriteReg32(LVDSPLL_CON0, reg_value | 0x1);

	udelay(40); /* wait for PLL stable (min delay is 20us) */

	/***************
	* xPLL DIV RSTB
	****************/
	reg_value = DRV_ReadReg32(ARMCA7PLL_CON0);
	DRV_WriteReg32(ARMCA7PLL_CON0, reg_value | 0x01000000);

	reg_value = DRV_ReadReg32(MAINPLL_CON0);
	DRV_WriteReg32(MAINPLL_CON0, reg_value | 0x01000000);

	reg_value = DRV_ReadReg32(UNIVPLL_CON0);
	DRV_WriteReg32(UNIVPLL_CON0, reg_value | 0x01000000);

	/**************
	* INFRA CLKMUX
	***************/

	reg_value = DRV_ReadReg32(TOP_DCMCTL);
	DRV_WriteReg32(TOP_DCMCTL, reg_value | 0x1); /* enable infrasys DCM */

	DRV_WriteReg32(CLK_MODE, 0x1);
	DRV_WriteReg32(CLK_MODE, 0x0); /* enable TOPCKGEN */

	/************
	* TOP CLKMUX
	*************/

	DRV_WriteReg32(CLK_CFG_0, 0x01000005); /* ddrphycfg_ck = 26MHz, not set mem_clk */

	DRV_WriteReg32(CLK_CFG_1, 0x01010100); /* pwm = 26Mhz */

	DRV_WriteReg32(CLK_CFG_2, 0x01010000); /* uart = camtg = 26Mhz */

	DRV_WriteReg32(CLK_CFG_4, 0x01000502); /* audio = 26M */

	DRV_WriteReg32(CLK_CFG_5, 0x06000100); /* bit16~23 is reserved, pmicspi use 26MHz */

	DRV_WriteReg32(CLK_CFG_6, 0x01050101);/* cci400 use univpll_d2, 624MHz */

	DRV_WriteReg32(CLK_CFG_7, 0x01010101);

	DRV_WriteReg32(CLK_CFG_12, 0x01010100); /* spinfi_bclk = 26M */
	DRV_WriteReg32(CLK_CFG_13, 0x01020202);

	/* TOP CKGEN issue on MT8173E1, below is the workaround solution
	 * ATTENTION: CLK_CFG_13(bit0~2) MUST BE SET BEFORE CLK_CFG_3
	 * OTHERWISE, WHOLE SYSTEM WILL HANG UP
	 * PLEASE DO NOT CHANGE THE REGISTER SETTING SEQUENCE!!!
	 */
	DRV_WriteReg32(CLK_CFG_3, 0x02060201);

	DRV_WriteReg32(CLK_SCP_CFG_0, 0x7FF); /* enable scpsys clock off control */
	DRV_WriteReg32(CLK_SCP_CFG_1, 0x15); /* enable scpsys clock off control */

	/*for MTCMOS*/
	spm_mtcmos_ctrl_disp(STA_POWER_ON);
}

int spm_mtcmos_ctrl_disp(int state)
{
	int err = 0;

	volatile unsigned int val;

	spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

	if (state == STA_POWER_DOWN) {

		spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | SRAM_PDN);
		while ((spm_read(SPM_DIS_PWR_CON) & DIS_SRAM_ACK) != DIS_SRAM_ACK) {
		}
		spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ISO);

		val = spm_read(SPM_DIS_PWR_CON);
		val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
		spm_write(SPM_DIS_PWR_CON, val);

		spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~(PWR_ON | PWR_ON_S));

		while ((spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK)
		       || (spm_read(SPM_PWR_STATUS_2ND) & DIS_PWR_STA_MASK)) {
		}
	} else {    /* STA_POWER_ON */
		spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ON);
		spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ON_S);

		while (!(spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK)
		       || !(spm_read(SPM_PWR_STATUS_2ND) & DIS_PWR_STA_MASK)) {
		}

		spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_CLK_DIS);
		spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_ISO);
		spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_RST_B);

		spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~SRAM_PDN);

		while ((spm_read(SPM_DIS_PWR_CON) & DIS_SRAM_ACK)) {
		}
	}

	return err;
}
