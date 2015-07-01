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
#include <delay.h>
#include <soc/pmic_wrap_init.h>
#include <soc/pmic.h>
#include <timer.h>
#include <console/console.h>

#if CONFIG_DEBUG_PMIC
#define DEBUG_PMIC(level, x...)		printk(level, x)
#else
#define DEBUG_PMIC(level, x...)
#endif

u32 mt6391_read(u32 reg, u32 mask, u32 shift)
{
	u32 rdata;

	pwrap_wacs2(0, reg, 0, &rdata, 1);
	rdata &= (mask << shift);
	rdata = (rdata >> shift);

	DEBUG_PMIC(BIOS_INFO, "[%s] rdata = %#x\n", __func__, rdata);
	return rdata;
}

void mt6391_write(u32 reg, u32 val, u32 mask, u32 shift)
{
	u32 rdata;

	pwrap_wacs2(0, reg, 0, &rdata, 1);

	DEBUG_PMIC(BIOS_INFO, "[%s] Reg[%x]=%#x\n", __func__, reg, rdata);
	rdata &= ~(mask << shift);
	rdata |= (val << shift);

	pwrap_wacs2(1, reg, rdata, &rdata, 1);
	DEBUG_PMIC(BIOS_INFO, "[%s] write Reg[%x]=%#x\n",
	           __func__, reg, rdata);

	return;
}

static u32 get_pmic6397_chip_version(void)
{
	u32 eco_version = 0;

	eco_version = mt6391_read((u32)(PMIC_RG_CID), (u32)(PMIC_RG_CID_MASK),
				  (u32)(PMIC_RG_CID_SHIFT));

	return eco_version;
}

static void pmic_init_setting_v1(void)
{
	u32 chip_version = 0;

	/* put init setting from DE/SA */
	chip_version = get_pmic6397_chip_version();
	switch (chip_version) {
	/* [7:4]: RG_VCDT_HV_VTH; 7V OVP */
	case PMIC6391_E1_CID_CODE:
	case PMIC6391_E2_CID_CODE:
		mt6391_write(PMIC_RG_CHR_CON1, 0xC, 0xF, 4);
		break;
	case PMIC6397_E2_CID_CODE:
	case PMIC6397_E3_CID_CODE:
	case PMIC6397_E4_CID_CODE:
		mt6391_write(PMIC_RG_CHR_CON1, 0xB, 0xF, 4);
		break;
	default:
		printk(BIOS_INFO, "[Power/PMIC] Error chip ID %d\n",
		       chip_version);
		mt6391_write(PMIC_RG_CHR_CON1, 0xB, 0xF, 4);
		break;
	}
	mt6391_write(PMIC_RG_TOP_CKPDN, 0x0, 0x1, 3);        /* [3:3]: RG_PWMOC_CK_PDN; For OC protection */
	mt6391_write(PMIC_RG_TOP_CKCON1, 0x1, 0x1, 9);        /* [9:9]: RG_SRCVOLT_HW_AUTO_EN; */
	mt6391_write(PMIC_RG_TOP_CKCON1, 0x1, 0x1, 8);        /* [8:8]: RG_OSC_SEL_AUTO; */
	mt6391_write(PMIC_RG_TOP_CKCON1, 0x1, 0x1, 6);        /* [6:6]: RG_SMPS_DIV2_SRC_AUTOFF_DIS; */
	mt6391_write(PMIC_RG_TOP_CKCON1, 0x1, 0x1, 5);        /* [5:5]: RG_SMPS_AUTOFF_DIS; */
	mt6391_write(PMIC_RG_OC_DEG_EN, 0x1, 0x1, 7);        /* [7:7]: VDRM_DEG_EN; */
	mt6391_write(PMIC_RG_OC_DEG_EN, 0x1, 0x1, 6);        /* [6:6]: VSRMCA7_DEG_EN; */
	mt6391_write(PMIC_RG_OC_DEG_EN, 0x1, 0x1, 5);        /* [5:5]: VPCA7_DEG_EN; */
	mt6391_write(PMIC_RG_OC_DEG_EN, 0x1, 0x1, 4);        /* [4:4]: VIO18_DEG_EN; */
	mt6391_write(PMIC_RG_OC_DEG_EN, 0x1, 0x1, 3);        /* [3:3]: VGPU_DEG_EN; For OC protection */
	mt6391_write(PMIC_RG_OC_DEG_EN, 0x1, 0x1, 2);        /* [2:2]: VCORE_DEG_EN; */
	mt6391_write(PMIC_RG_OC_DEG_EN, 0x1, 0x1, 1);        /* [1:1]: VSRMCA15_DEG_EN; */
	mt6391_write(PMIC_RG_OC_DEG_EN, 0x1, 0x1, 0);        /* [0:0]: VCA15_DEG_EN; */
	mt6391_write(PMIC_RG_INT_CON0, 0x1, 0x1, 11);       /* [11:11]: RG_INT_EN_THR_H; */
	mt6391_write(PMIC_RG_INT_CON0, 0x1, 0x1, 10);       /* [10:10]: RG_INT_EN_THR_L; */
	mt6391_write(PMIC_RG_INT_CON0, 0x1, 0x1, 4);        /* [4:4]: RG_INT_EN_BAT_L; */
	mt6391_write(PMIC_RG_INT_CON1, 0x1, 0x1, 11);       /* [11:11]: RG_INT_EN_VGPU; OC protection */
	mt6391_write(PMIC_RG_INT_CON1, 0x1, 0x1, 8);        /* [8:8]: RG_INT_EN_VCA15; OC protection */
	mt6391_write(PMIC_RG_BUCK_CON3, 0x600, 0x0FFF, 0);   /* [12:0]: BUCK_RSV; for OC protection */
	mt6391_write(PMIC_RG_BUCK_CON8, 0x1, 0x3, 10);       /* [11:10]: QI_VCORE_VSLEEP; sleep mode only (0.7V) */
	mt6391_write(PMIC_RG_BUCK_CON8, 0x0, 0x3, 6);        /* [7:6]: QI_VSRMCA7_VSLEEP; sleep mode only (0.85V) */
	mt6391_write(PMIC_RG_BUCK_CON8, 0x1, 0x3, 4);        /* [5:4]: QI_VSRMCA15_VSLEEP; sleep mode only (0.7V) */
	mt6391_write(PMIC_RG_BUCK_CON8, 0x0, 0x3, 2);        /* [3:2]: QI_VPCA7_VSLEEP; sleep mode only (0.85V) */
	mt6391_write(PMIC_RG_BUCK_CON8, 0x1, 0x3, 0);        /* [1:0]: QI_VCA15_VSLEEP; sleep mode only (0.7V) */
	mt6391_write(PMIC_RG_VCA15_CON1, 0x0, 0x3, 12);       /* [13:12]: RG_VCA15_CSL2; for OC protection */
	mt6391_write(PMIC_RG_VCA15_CON1, 0x0, 0x3, 10);       /* [11:10]: RG_VCA15_CSL1; for OC protection */
	mt6391_write(PMIC_RG_VCA15_CON8, 0x1, 0x1, 15);       /* [15:15]: VCA15_SFCHG_REN; soft change rising enable */
	mt6391_write(PMIC_RG_VCA15_CON8, 0x5, 0x7F, 8);       /* [14:8]: VCA15_SFCHG_RRATE; soft change rising step=0.5 */
	mt6391_write(PMIC_RG_VCA15_CON8, 0x1, 0x1, 7);        /* [7:7]: VCA15_SFCHG_FEN; soft change falling enable */
	mt6391_write(PMIC_RG_VCA15_CON8, 0x17, 0x7F, 0);      /* [6:0]: VCA15_SFCHG_FRATE; soft change falling step=2us */
	mt6391_write(PMIC_RG_VCA15_CON11, 0x0, 0x7F, 0);       /* [6:0]: VCA15_VOSEL_SLEEP; sleep mode only (0.7V) */
	mt6391_write(PMIC_RG_VCA15_CON18, 0x1, 0x1, 8);        /* [8:8]: VCA15_VSLEEP_EN; set sleep mode reference volt */
	mt6391_write(PMIC_RG_VCA15_CON18, 0x3, 0x3, 4);        /* [5:4]: VCA15_VOSEL_TRANS_EN; rising & falling enable */
	mt6391_write(PMIC_RG_VSRMCA15_CON5, 0x1, 0x1, 5);        /* [5:5]: VSRMCA15_TRACK_SLEEP_CTRL; */
	mt6391_write(PMIC_RG_VSRMCA15_CON6, 0x0, 0x3, 4);        /* [5:4]: VSRMCA15_VOSEL_SEL; */
	mt6391_write(PMIC_RG_VSRMCA15_CON8, 0x1, 0x1, 15);       /* [15:15]: VSRMCA15_SFCHG_REN; */
	mt6391_write(PMIC_RG_VSRMCA15_CON8, 0x5, 0x7F, 8);       /* [14:8]: VSRMCA15_SFCHG_RRATE; */
	mt6391_write(PMIC_RG_VSRMCA15_CON8, 0x1, 0x1, 7);        /* [7:7]: VSRMCA15_SFCHG_FEN; */
	mt6391_write(PMIC_RG_VSRMCA15_CON8, 0x17, 0x7F, 0);      /* [6:0]: VSRMCA15_SFCHG_FRATE; */
	mt6391_write(PMIC_RG_VSRMCA15_CON11, 0x00, 0x7F, 0);      /* [6:0]: VSRMCA15_VOSEL_SLEEP; Sleep mode setting on */
	mt6391_write(PMIC_RG_VSRMCA15_CON18, 0x1, 0x1, 8);        /* [8:8]: VSRMCA15_VSLEEP_EN; set sleep mode referenc */
	mt6391_write(PMIC_RG_VSRMCA15_CON18, 0x3, 0x3, 4);        /* [5:4]: VSRMCA15_VOSEL_TRANS_EN; rising & falling e */
	mt6391_write(PMIC_RG_VCORE_CON5, 0x1, 0x1, 1);        /* [1:1]: VCORE_VOSEL_CTRL; sleep mode voltage control fo */
	mt6391_write(PMIC_RG_VCORE_CON6, 0x0, 0x3, 4);        /* [5:4]: VCORE_VOSEL_SEL; */
	mt6391_write(PMIC_RG_VCORE_CON8, 0x1, 0x1, 15);       /* [15:15]: VCORE_SFCHG_REN; */
	mt6391_write(PMIC_RG_VCORE_CON8, 0x5, 0x7F, 8);       /* [14:8]: VCORE_SFCHG_RRATE; */
	mt6391_write(PMIC_RG_VCORE_CON8, 0x17, 0x7F, 0);      /* [6:0]: VCORE_SFCHG_FRATE; */
	mt6391_write(PMIC_RG_VCORE_CON11, 0x0, 0x7F, 0);       /* [6:0]: VCORE_VOSEL_SLEEP; Sleep mode setting only (0. */
	mt6391_write(PMIC_RG_VCORE_CON18, 0x1, 0x1, 8);        /* [8:8]: VCORE_VSLEEP_EN; Sleep mode HW control  R2R to */
	mt6391_write(PMIC_RG_VCORE_CON18, 0x0, 0x3, 4);        /* [5:4]: VCORE_VOSEL_TRANS_EN; Follows MT6320 VCORE set */
	mt6391_write(PMIC_RG_VCORE_CON18, 0x3, 0x3, 0);        /* [1:0]: VCORE_TRANSTD; */
	mt6391_write(PMIC_RG_VGPU_CON1, 0x1, 0x3, 8);        /* [9:8]: RG_VGPU_CSL; for OC protection */
	mt6391_write(PMIC_RG_VGPU_CON8, 0x1, 0x1, 15);       /* [15:15]: VGPU_SFCHG_REN; */
	mt6391_write(PMIC_RG_VGPU_CON8, 0x5, 0x7F, 8);       /* [14:8]: VGPU_SFCHG_RRATE; */
	mt6391_write(PMIC_RG_VGPU_CON8, 0x17, 0x7F, 0);      /* [6:0]: VGPU_SFCHG_FRATE; */
	mt6391_write(PMIC_RG_VGPU_CON18, 0x0, 0x3, 4);        /* [5:4]: VGPU_VOSEL_TRANS_EN; */
	mt6391_write(PMIC_RG_VGPU_CON18, 0x3, 0x3, 0);        /* [1:0]: VGPU_TRANSTD; */
	mt6391_write(PMIC_RG_VPCA7_CON6, 0x0, 0x3, 4);        /* [5:4]: VPCA7_VOSEL_SEL; */
	mt6391_write(PMIC_RG_VPCA7_CON8, 0x1, 0x1, 15);       /* [15:15]: VPCA7_SFCHG_REN; */
	mt6391_write(PMIC_RG_VPCA7_CON8, 0x5, 0x7F, 8);       /* [14:8]: VPCA7_SFCHG_RRATE; */
	mt6391_write(PMIC_RG_VPCA7_CON8, 0x1, 0x1, 7);        /* [7:7]: VPCA7_SFCHG_FEN; */
	mt6391_write(PMIC_RG_VPCA7_CON8, 0x17, 0x7F, 0);      /* [6:0]: VPCA7_SFCHG_FRATE; */
	mt6391_write(PMIC_RG_VPCA7_CON11, 0x18, 0x7F, 0);      /* [6:0]: VPCA7_VOSEL_SLEEP; */
	mt6391_write(PMIC_RG_VPCA7_CON18, 0x0, 0x1, 8);        /* [8:8]: VPCA7_VSLEEP_EN; */
	mt6391_write(PMIC_RG_VPCA7_CON18, 0x3, 0x3, 4);        /* [5:4]: VPCA7_VOSEL_TRANS_EN; */
	mt6391_write(PMIC_RG_VSRMCA7_CON5, 0x0, 0x1, 5);        /* [5:5]: VSRMCA7_TRACK_SLEEP_CTRL; */
	mt6391_write(PMIC_RG_VSRMCA7_CON6, 0x0, 0x3, 4);        /* [5:4]: VSRMCA7_VOSEL_SEL; */
	mt6391_write(PMIC_RG_VSRMCA7_CON8, 0x1, 0x1, 15);       /* [15:15]: VSRMCA7_SFCHG_REN; */
	mt6391_write(PMIC_RG_VSRMCA7_CON8, 0x5, 0x7F, 8);       /* [14:8]: VSRMCA7_SFCHG_RRATE; */
	mt6391_write(PMIC_RG_VSRMCA7_CON8, 0x1, 0x1, 7);        /* [7:7]: VSRMCA7_SFCHG_FEN; */
	mt6391_write(PMIC_RG_VSRMCA7_CON8, 0x17, 0x7F, 0);      /* [6:0]: VSRMCA7_SFCHG_FRATE; */
	mt6391_write(PMIC_RG_VSRMCA7_CON11, 0x18, 0x7F, 0);      /* [6:0]: VSRMCA7_VOSEL_SLEEP; */
	mt6391_write(PMIC_RG_VSRMCA7_CON18, 0x1, 0x1, 8);        /* [8:8]: VSRMCA7_VSLEEP_EN; */
	mt6391_write(PMIC_RG_VSRMCA7_CON18, 0x3, 0x3, 4);        /* [5:4]: VSRMCA7_VOSEL_TRANS_EN; */
	mt6391_write(PMIC_RG_VDRM_CON18, 0x1, 0x1, 8);        /* [8:8]: VDRM_VSLEEP_EN; */
	mt6391_write(PMIC_RG_DIGLDO_CON24, 0x1, 0x1, 2);        /* [2:2]: VIBR_THER_SHEN_EN; */
	mt6391_write(PMIC_RG_STRUP_CON0, 0x1, 0x1, 5);        /* [5:5]: THR_HWPDN_EN; */
	mt6391_write(PMIC_RG_STRUP_CON2, 0x1, 0x1, 3);        /* [3:3]: RG_RST_DRVSEL; */
	mt6391_write(PMIC_RG_STRUP_CON2, 0x1, 0x1, 2);        /* [2:2]: RG_EN_DRVSEL; */
	mt6391_write(PMIC_RG_STRUP_CON5, 0x1, 0x1, 1);        /* [1:1]: PWRBB_DEB_EN; */
	mt6391_write(PMIC_RG_STRUP_CON7, 0x1, 0x1, 12);       /* [12:12]: VSRMCA15_PG_H2L_EN; */
	mt6391_write(PMIC_RG_STRUP_CON7, 0x1, 0x1, 11);       /* [11:11]: VPCA15_PG_H2L_EN; */
	mt6391_write(PMIC_RG_STRUP_CON7, 0x1, 0x1, 10);       /* [10:10]: VCORE_PG_H2L_EN; */
	mt6391_write(PMIC_RG_STRUP_CON7, 0x1, 0x1, 9);        /* [9:9]: VSRMCA7_PG_H2L_EN; */
	mt6391_write(PMIC_RG_STRUP_CON7, 0x1, 0x1, 8);        /* [8:8]: VPCA7_PG_H2L_EN; */
	mt6391_write(PMIC_RG_STRUP_CON10, 0x1, 0x1, 1);        /* [1:1]: STRUP_PWROFF_PREOFF_EN; */
	mt6391_write(PMIC_RG_STRUP_CON10, 0x1, 0x1, 0);        /* [0:0]: STRUP_PWROFF_SEQ_EN; */
	mt6391_write(PMIC_RG_AUXADC_CON14, 0xFC, 0xFF, 8);      /* [15:8]: RG_ADC_TRIM_CH_SEL; */
	mt6391_write(PMIC_RG_FLASH_CON0, 0x1, 0x1, 1);        /* [1:1]: FLASH_THER_SHDN_EN; */
	mt6391_write(PMIC_RG_KPLED_CON0, 0x1, 0x1, 1);        /* [1:1]: KPLED_THER_SHDN_EN; */
	mt6391_write(PMIC_RG_VSRMCA15_CON19, 0x10, 0x7F, 8);      /* [14:8]: VSRMCA15_VOSEL_OFFSET; set offset=100mV */
	mt6391_write(PMIC_RG_VSRMCA15_CON19, 0x0, 0x7F, 0);       /* [6:0]: VSRMCA15_VOSEL_DELTA; set delta=0mV */
	mt6391_write(PMIC_RG_VSRMCA15_CON20, 0x48, 0x7F, 8);      /* [14:8]: VSRMCA15_VOSEL_ON_HB; set HB=1.15V */
	mt6391_write(PMIC_RG_VSRMCA15_CON20, 0x0, 0x7F, 0);       /* [6:0]: VSRMCA15_VOSEL_ON_LB; set LB=0.7V */
	mt6391_write(PMIC_RG_VSRMCA15_CON21, 0x0, 0x7F, 0);       /* [6:0]: VSRMCA15_VOSEL_SLEEP_LB; set sleep LB=0.7V */
	mt6391_write(PMIC_RG_VSRMCA7_CON19, 0x4, 0x7F, 8);       /* [14:8]: VSRMCA7_VOSEL_OFFSET; set offset=25mV */
	mt6391_write(PMIC_RG_VSRMCA7_CON19, 0x0, 0x7F, 0);       /* [6:0]: VSRMCA7_VOSEL_DELTA; set delta=0mV */
	mt6391_write(PMIC_RG_VSRMCA7_CON20, 0x5C, 0x7F, 8);      /* [14:8]: VSRMCA7_VOSEL_ON_HB; set HB=1.275V */
	mt6391_write(PMIC_RG_VSRMCA7_CON20, 0x38, 0x7F, 0);      /* [6:0]: VSRMCA7_VOSEL_ON_LB; set LB=1.05000V */
	mt6391_write(PMIC_RG_VSRMCA7_CON21, 0x18, 0x7F, 0);      /* [6:0]: VSRMCA7_VOSEL_SLEEP_LB; set sleep LB=0.85000 */
	mt6391_write(PMIC_RG_VCA15_CON5, 0x3, 0x3, 0);        /* [1:1]: VCA15_VOSEL_CTRL, VCA15_EN_CTRL; DVS HW control */
	mt6391_write(PMIC_RG_VSRMCA15_CON5, 0x3, 0x3, 0);        /* [1:1]: VSRMCA15_VOSEL_CTRL, VSRAM15_EN_CTRL; */
	mt6391_write(PMIC_RG_VPCA7_CON5, 0x0, 0x1, 1);        /* [1:1]: VPCA7_VOSEL_CTRL; */
	mt6391_write(PMIC_RG_VSRMCA7_CON5, 0x0, 0x1, 1);        /* [1:1]: VSRMCA7_VOSEL_CTRL; */
	mt6391_write(PMIC_RG_VCA15_CON5, 0x1, 0x1, 4);        /* [4:4]: VCA15_TRACK_ON_CTRL; DVFS tracking enable */
	mt6391_write(PMIC_RG_VSRMCA15_CON5, 0x1, 0x1, 4);        /* [4:4]: VSRMCA15_TRACK_ON_CTRL; */
	mt6391_write(PMIC_RG_VPCA7_CON5, 0x0, 0x1, 4);        /* [4:4]: VPCA7_TRACK_ON_CTRL; */
	mt6391_write(PMIC_RG_VSRMCA7_CON5, 0x0, 0x1, 4);        /* [4:4]: VSRMCA7_TRACK_ON_CTRL; */
	mt6391_write(PMIC_RG_OC_CTL1, 0x3, 0x3, 14);       /* [15:14]: VGPU OC; */
	mt6391_write(PMIC_RG_OC_CTL1, 0x3, 0x3, 2);        /* [3:2]: VCA15 OC; */
}

static void pmic_default_buck_voltage(void)
{
	u32 reg_val = 0;
	u32 buck_val = 0;
	/* There are two kinds of PMIC used for MT8173 : MT6397s/MT6391.
	 * MT6397s: the default voltage of register was not suitable for
	 *          MT8173, needs to apply the setting of eFuse.
	 * VPCA15/VPCA7/VSRMCA15/VSRMCA7: 1.15V
	 * VCORE: 1.05V
	 *
	 * MT6391: the default voltage of register was matched for MT8173.
	 * VPAC15/VCORE/VGPU: 1.0V
	 * VPCA7: 1.2V
	 * VSRMCA15/VSRMCA7: 1.0125V
	 */
	reg_val = mt6391_read(PMIC_RG_EFUSE_DOUT_288_303, 0xFFFF, 0);
	if ((reg_val & 0x01) == 0x01) {
		/* VCORE */
		reg_val = mt6391_read(PMIC_RG_EFUSE_DOUT_256_271, 0xF, 12);
		buck_val = mt6391_read(PMIC_RG_VCORE_CON9,
				       PMIC_RG_VCORE_VOSEL_MASK,
				       PMIC_RG_VCORE_VOSEL_SHIFT);

		/* VCORE_VOSEL[3:6] => eFuse bit 268-271 */
		buck_val = (buck_val & 0x07) | (reg_val << 3);
		mt6391_write(PMIC_RG_VCORE_CON9, buck_val,
			     PMIC_RG_VCORE_VOSEL_MASK,
			     PMIC_RG_VCORE_VOSEL_SHIFT);
		mt6391_write(PMIC_RG_VCORE_CON10, buck_val,
			     PMIC_RG_VCORE_VOSEL_ON_MASK,
			     PMIC_RG_VCORE_VOSEL_ON_SHIFT);

		reg_val = mt6391_read(PMIC_RG_EFUSE_DOUT_272_287, 0xFFFF, 0);
		/* VCA15 */
		buck_val = 0;
		buck_val = mt6391_read(PMIC_RG_VCA15_CON9,
				       PMIC_RG_VCA15_VOSEL_MASK,
				       PMIC_RG_VCA15_VOSEL_SHIFT);
		buck_val = (buck_val & 0x07) | ((reg_val & 0x0F) << 3);
		mt6391_write(PMIC_RG_VCA15_CON9, buck_val,
			     PMIC_RG_VCA15_VOSEL_MASK,
			     PMIC_RG_VCA15_VOSEL_SHIFT);
		mt6391_write(PMIC_RG_VCA15_CON10, buck_val,
			     PMIC_RG_VCA15_VOSEL_ON_MASK,
			     PMIC_RG_VCA15_VOSEL_ON_SHIFT);

		/* VSAMRCA15 */
		buck_val = 0;
		buck_val = mt6391_read(PMIC_RG_VSRMCA15_CON9,
				       PMIC_RG_VSRMCA15_VOSEL_MASK,
				       PMIC_RG_VSRMCA15_VOSEL_SHIFT);
		buck_val = (buck_val & 0x07) | ((reg_val & 0xF0) >> 1);
		mt6391_write(PMIC_RG_VSRMCA15_CON9, buck_val,
			     PMIC_RG_VSRMCA15_VOSEL_MASK,
			     PMIC_RG_VSRMCA15_VOSEL_SHIFT);
		mt6391_write(PMIC_RG_VSRMCA15_CON10, buck_val,
			     PMIC_RG_VSRMCA15_VOSEL_ON_MASK,
			     PMIC_RG_VSRMCA15_VOSEL_ON_SHIFT);

		/* VCA7 */
		buck_val = 0;
		buck_val = mt6391_read(PMIC_RG_VPCA7_CON9,
				       PMIC_RG_VPCA7_VOSEL_MASK,
				       PMIC_RG_VPCA7_VOSEL_SHIFT);
		buck_val = (buck_val & 0x07) | ((reg_val & 0xF00) >> 5);
		mt6391_write(PMIC_RG_VPCA7_CON9, buck_val,
			     PMIC_RG_VPCA7_VOSEL_MASK,
			     PMIC_RG_VPCA7_VOSEL_SHIFT);
		mt6391_write(PMIC_RG_VPCA7_CON10, buck_val,
			     PMIC_RG_VPCA7_VOSEL_ON_MASK,
			     PMIC_RG_VPCA7_VOSEL_ON_SHIFT);

		/* VSAMRCA7 */
		buck_val = 0;
		buck_val = mt6391_read(PMIC_RG_VSRMCA7_CON9,
				       PMIC_RG_VPCA7_VOSEL_MASK,
				       PMIC_RG_VPCA7_VOSEL_SHIFT);
		buck_val = (buck_val & 0x07) | ((reg_val & 0xF000) >> 9);
		mt6391_write(PMIC_RG_VSRMCA7_CON9, buck_val,
			     PMIC_RG_VSRMCA7_VOSEL_MASK,
			     PMIC_RG_VSRMCA7_VOSEL_SHIFT);
		mt6391_write(PMIC_RG_VSRMCA7_CON10, buck_val,
			     PMIC_RG_VSRMCA7_VOSEL_ON_MASK,
			     PMIC_RG_VSRMCA7_VOSEL_ON_SHIFT);

		/* set the power control by register(use original) */
		mt6391_write(PMIC_RG_BUCK_CON3, 0x1, 0x1, 12);
	}
}

u32 pmic_init(void)
{
	u32 ret_code = PMIC_TEST_PASS;
	int ret_val = 0;
	u32 reg_val = 0;

	/* Adjust default BUCK voltage from eFuse */
	pmic_default_buck_voltage();

	/* Enable PMIC RST function (depends on main chip RST function) */
	/*
	 * state1: RG_SYSRSTB_EN = 1, RG_STRUP_MAN_RST_EN=1, RG_RST_PART_SEL=1
	 * state2: RG_SYSRSTB_EN = 1, RG_STRUP_MAN_RST_EN=0, RG_RST_PART_SEL=1
	 * state3: RG_SYSRSTB_EN = 1, RG_STRUP_MAN_RST_EN=x, RG_RST_PART_SEL=0
	 */
	mt6391_write(PMIC_RG_TOP_RST_MISC, 0x1,
		     PMIC_RG_SYSRSTB_EN_MASK,
		     PMIC_RG_SYSRSTB_EN_SHIFT);
	mt6391_write(PMIC_RG_TOP_RST_MISC, 0x0,
		     PMIC_RG_STRUP_MAN_RST_EN_MASK,
		     PMIC_RG_STRUP_MAN_RST_EN_SHIFT);
	mt6391_write(PMIC_RG_TOP_RST_MISC, 0x1,
		     PMIC_RG_RST_PART_SEL_MASK,
		     PMIC_RG_RST_PART_SEL_SHIFT);

	/*  Enable AP watchdog reset */
	mt6391_write(PMIC_RG_TOP_RST_MISC, 0x0,
		     PMIC_RG_AP_RST_DIS_MASK,
		     PMIC_RG_AP_RST_DIS_SHIFT);
	ret_val = mt6391_read(PMIC_RG_TOP_RST_MISC, 0xFFFF, 0);

	/* Enable CA15 by default for different PMIC behavior */
	mt6391_write(PMIC_RG_VCA15_CON7, 0x1, PMIC_RG_VCA15_EN_MASK,
		     PMIC_RG_VCA15_EN_SHIFT);
	mt6391_write(PMIC_RG_VSRMCA15_CON7, 0x1, PMIC_RG_VSRMCA15_EN_MASK,
		     PMIC_RG_VSRMCA15_EN_SHIFT);
	mt6391_write(PMIC_RG_VPCA7_CON7, 0x0, PMIC_RG_VPCA7_EN_MASK,
		     PMIC_RG_VPCA7_EN_SHIFT);
	udelay(200); /* delay for Buck ready */

	ret_val = mt6391_read(PMIC_RG_VCA15_CON7, 0xFFFF, 0);
	ret_val = mt6391_read(PMIC_RG_VSRMCA15_CON7, 0xFFFF, 0);

	/* pmic initial setting */
	pmic_init_setting_v1();

	/* 26M clock amplitute adjust */
	mt6391_write(PMIC_RG_DCXO_ANALOG_CON1, 0x0, PMIC_RG_DCXO_LDO_BB_V_MASK,
		     PMIC_RG_DCXO_LDO_BB_V_SHIFT);
	mt6391_write(PMIC_RG_DCXO_ANALOG_CON1, 0x1, PMIC_RG_DCXO_ATTEN_BB_MASK,
		     PMIC_RG_DCXO_ATTEN_BB_SHIFT);

	reg_val = mt6391_read(PMIC_RG_EFUSE_DOUT_304_319, 0xFFFF, 0);
	if ((reg_val & 0x8000) == 0) {
		mt6391_write(PMIC_RG_BUCK_K_CON0, 0x0041, 0xFFFF, 0);
		mt6391_write(PMIC_RG_BUCK_K_CON0, 0x0040, 0xFFFF, 0);
		mt6391_write(PMIC_RG_BUCK_K_CON0, 0x0050, 0xFFFF, 0);
	}

	mt6391_write(PMIC_RG_CHR_CON13, 0x00, PMIC_RG_CHRWDT_EN_MASK,
		     PMIC_RG_CHRWDT_EN_SHIFT);

	/* Set VDRM to 1.21875V only for 8173 */
	mt6391_write(PMIC_RG_VDRM_CON9, 0x43, 0x7F, 0);
	mt6391_write(PMIC_RG_VDRM_CON10, 0x43, 0x7F, 0);

	mt6391_write(PMIC_RG_VSRMCA7_CON9, 0x40, PMIC_RG_VSRMCA7_VOSEL_MASK,
		     PMIC_RG_VSRMCA7_VOSEL_SHIFT);
	mt6391_write(PMIC_RG_VSRMCA7_CON10, 0x40,
		     PMIC_RG_VSRMCA7_VOSEL_ON_MASK,
		     PMIC_RG_VSRMCA7_VOSEL_ON_SHIFT);

	return ret_code;
}
