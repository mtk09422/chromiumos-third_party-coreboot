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
#include <soc/pmic_wrap_init.h>
#include <soc/pmic.h>
#include <soc/da9212.h>
#include <timer.h>
#include <console/console.h>

u32 pmic_read_interface(u32 RegNum, u32 *val, u32 MASK, u32 SHIFT);
u32 get_pmic6397_chip_version(void);
u32 pmic_IsUsbCableIn(void);
int pmic_detect_homekey(void);
void pmic_init_setting_v1(void);

#if CONFIG_DEBUG_PMIC
#define DEBUG_PMIC(level, x...)		printk(level, x)
#else
#define DEBUG_PMIC(level, x...)
#endif

/* flag to indicate ca15 related power is ready */
volatile int g_ca15_ready __attribute__ ((section(".data"))) = 0;

u32 pmic_read_interface(u32 RegNum, u32 *val, u32 MASK, u32 SHIFT)
{
	u32 return_value = 0;
	u32 pmic_reg = 0;
	u32 rdata;

	return_value = pwrap_wacs2(0, (RegNum), 0, &rdata);
	pmic_reg = rdata;
	if (return_value != 0) {
		printk(BIOS_INFO, "[%s] Reg[%x]= pmic_wrap read data fail\n",
		       __func__, RegNum);
		return return_value;
	}
	DEBUG_PMIC(BIOS_INFO, "[%s] Reg[%x]=%#x\n", __func__, RegNum, pmic_reg);
	pmic_reg &= (MASK << SHIFT);
	*val = (pmic_reg >> SHIFT);

	DEBUG_PMIC(BIOS_INFO, "[%s] val=%#x\n", __func__, *val);
	return return_value;
}

u32 pmic_config_interface(u32 RegNum, u32 val, u32 MASK, u32 SHIFT)
{
	u32 return_value = 0;
	u32 pmic_reg = 0;
	u32 rdata;

	/* 1. mt_read_byte(RegNum, &pmic_reg); */
	return_value = pwrap_wacs2(0, (RegNum), 0, &rdata);
	pmic_reg = rdata;
	if (return_value != 0) {
		printk(BIOS_INFO, "[%s] Reg[%x]= pmic_wrap read data fail\n",
		       __func__, RegNum);
		return return_value;
	}
	DEBUG_PMIC(BIOS_INFO, "[%s] Reg[%x]=%#x\n", __func__, RegNum, pmic_reg);
	pmic_reg &= ~(MASK << SHIFT);
	pmic_reg |= (val << SHIFT);

	/* 2. mt_write_byte(RegNum, pmic_reg); */
	return_value = pwrap_wacs2(1, (RegNum), pmic_reg, &rdata);
	if (return_value != 0) {
		printk(BIOS_INFO, "[%s] Reg[%x]= pmic_wrap read data fail\n",
		       __func__, RegNum);
		return return_value;
	}
	DEBUG_PMIC(BIOS_INFO, "[%s] write Reg[%x]=%#x\n",
	           __func__, RegNum, pmic_reg);

	return return_value;
}

u32 get_pmic6397_chip_version(void)
{
	u32 ret = 0;
	u32 eco_version = 0;

	ret = pmic_read_interface((u32)(CID),
				  (&eco_version),(u32)(PMIC_CID_MASK),
				  (u32)(PMIC_CID_SHIFT));

	return eco_version;
}

u32 pmic_IsUsbCableIn(void)
{
	u32 ret = 0;
	u32 val = 0;

	ret = pmic_read_interface((u32)(CHR_CON0),
				  (&val),(u32)(PMIC_RGS_CHRDET_MASK),
				  (u32)(PMIC_RGS_CHRDET_SHIFT));


	if (val)
		return PMIC_CHRDET_EXIST;
	else
		return PMIC_CHRDET_NOT_EXIST;
}

int pmic_detect_powerkey(void)
{
	u32 ret = 0;
	u32 val = 0;

	ret = pmic_read_interface((u32)(CHRSTATUS),
				  (&val),(u32)(PMIC_PWRKEY_DEB_MASK),
				  (u32)(PMIC_PWRKEY_DEB_SHIFT));
	/* pmic detect powerkey release or press and return different value to keypad */
	val = !val;
	printk(BIOS_INFO, "[%s] %s\n", __func__, val ? "Press" : "Release");
	return val;
}

int pmic_detect_homekey(void)
{
	u32 ret = 0;
	u32 val = 0;

	ret = pmic_read_interface((u32)(OCSTATUS2),
				  (&val),(u32)(PMIC_HOMEKEY_DEB_MASK),
				  (u32)(PMIC_HOMEKEY_DEB_SHIFT));

	return val;
}

void pl_hw_ulc_det(void)
{
	u32 ret_val = 0;

	ret_val = pmic_config_interface(CHR_CON23, 0x01, PMIC_RG_ULC_DET_EN_MASK, PMIC_RG_ULC_DET_EN_SHIFT);
	ret_val = pmic_config_interface(CHR_CON22, 0x01, PMIC_RG_LOW_ICH_DB_MASK, PMIC_RG_LOW_ICH_DB_SHIFT);
}

void pmic_init_setting_v1(void)
{
	u32 ret = 0;
	u32 chip_version = 0;

	/* put init setting from DE/SA */
	chip_version = get_pmic6397_chip_version();
	switch (chip_version) {
	/* [7:4]: RG_VCDT_HV_VTH; 7V OVP */
	case PMIC6391_E1_CID_CODE:
	case PMIC6391_E2_CID_CODE:
		ret = pmic_config_interface(0x2, 0xC, 0xF, 4);
		break;
	case PMIC6397_E2_CID_CODE:
	case PMIC6397_E3_CID_CODE:
	case PMIC6397_E4_CID_CODE:
		ret = pmic_config_interface(0x2, 0xB, 0xF, 4);
		break;
	default:
		printk(BIOS_INFO, "[Power/PMIC] Error chip ID %d\n", chip_version);
		ret = pmic_config_interface(0x2, 0xB, 0xF, 4);
		break;
	}
	ret = pmic_config_interface(0xC, 0x1, 0x7, 1);          /* [3:1]: RG_VBAT_OV_VTH; VBAT_OV=4.3V */
	ret = pmic_config_interface(0x1A, 0x3, 0xF, 0);         /* [3:0]: RG_CHRWDT_TD; align to 6250's */
	ret = pmic_config_interface(0x24, 0x1, 0x1, 1);         /* [1:1]: RG_BC11_RST; */
	ret = pmic_config_interface(0x2A, 0x0, 0x7, 4);         /* [6:4]: RG_CSDAC_STP; align to 6250's setting */
	ret = pmic_config_interface(0x2E, 0x1, 0x1, 7);         /* [7:7]: RG_ULC_DET_EN; */
	ret = pmic_config_interface(0x2E, 0x1, 0x1, 6);         /* [6:6]: RG_HWCV_EN; */
	ret = pmic_config_interface(0x2E, 0x1, 0x1, 2);         /* [2:2]: RG_CSDAC_MODE; */
	ret = pmic_config_interface(0x102, 0x0, 0x1, 3);        /* [3:3]: RG_PWMOC_CK_PDN; For OC protection */
	ret = pmic_config_interface(0x128, 0x1, 0x1, 9);        /* [9:9]: RG_SRCVOLT_HW_AUTO_EN; */
	ret = pmic_config_interface(0x128, 0x1, 0x1, 8);        /* [8:8]: RG_OSC_SEL_AUTO; */
	ret = pmic_config_interface(0x128, 0x1, 0x1, 6);        /* [6:6]: RG_SMPS_DIV2_SRC_AUTOFF_DIS; */
	ret = pmic_config_interface(0x128, 0x1, 0x1, 5);        /* [5:5]: RG_SMPS_AUTOFF_DIS; */
	ret = pmic_config_interface(0x130, 0x1, 0x1, 7);        /* [7:7]: VDRM_DEG_EN; */
	ret = pmic_config_interface(0x130, 0x1, 0x1, 6);        /* [6:6]: VSRMCA7_DEG_EN; */
	ret = pmic_config_interface(0x130, 0x1, 0x1, 5);        /* [5:5]: VPCA7_DEG_EN; */
	ret = pmic_config_interface(0x130, 0x1, 0x1, 4);        /* [4:4]: VIO18_DEG_EN; */
	ret = pmic_config_interface(0x130, 0x1, 0x1, 3);        /* [3:3]: VGPU_DEG_EN; For OC protection */
	ret = pmic_config_interface(0x130, 0x1, 0x1, 2);        /* [2:2]: VCORE_DEG_EN; */
	ret = pmic_config_interface(0x130, 0x1, 0x1, 1);        /* [1:1]: VSRMCA15_DEG_EN; */
	ret = pmic_config_interface(0x130, 0x1, 0x1, 0);        /* [0:0]: VCA15_DEG_EN; */
	ret = pmic_config_interface(0x178, 0x1, 0x1, 11);       /* [11:11]: RG_INT_EN_THR_H; */
	ret = pmic_config_interface(0x178, 0x1, 0x1, 10);       /* [10:10]: RG_INT_EN_THR_L; */
	ret = pmic_config_interface(0x178, 0x1, 0x1, 4);        /* [4:4]: RG_INT_EN_BAT_L; */
	ret = pmic_config_interface(0x17E, 0x1, 0x1, 11);       /* [11:11]: RG_INT_EN_VGPU; OC protection */
	ret = pmic_config_interface(0x17E, 0x1, 0x1, 8);        /* [8:8]: RG_INT_EN_VCA15; OC protection */
	ret = pmic_config_interface(0x206, 0x600, 0x0FFF, 0);   /* [12:0]: BUCK_RSV; for OC protection */
	ret = pmic_config_interface(0x210, 0x1, 0x3, 10);       /* [11:10]: QI_VCORE_VSLEEP; sleep mode only (0.7V) */
	ret = pmic_config_interface(0x210, 0x0, 0x3, 6);        /* [7:6]: QI_VSRMCA7_VSLEEP; sleep mode only (0.85V) */
	ret = pmic_config_interface(0x210, 0x1, 0x3, 4);        /* [5:4]: QI_VSRMCA15_VSLEEP; sleep mode only (0.7V) */
	ret = pmic_config_interface(0x210, 0x0, 0x3, 2);        /* [3:2]: QI_VPCA7_VSLEEP; sleep mode only (0.85V) */
	ret = pmic_config_interface(0x210, 0x1, 0x3, 0);        /* [1:0]: QI_VCA15_VSLEEP; sleep mode only (0.7V) */
	ret = pmic_config_interface(0x216, 0x0, 0x3, 12);       /* [13:12]: RG_VCA15_CSL2; for OC protection */
	ret = pmic_config_interface(0x216, 0x0, 0x3, 10);       /* [11:10]: RG_VCA15_CSL1; for OC protection */
	ret = pmic_config_interface(0x224, 0x1, 0x1, 15);       /* [15:15]: VCA15_SFCHG_REN; soft change rising enable */
	ret = pmic_config_interface(0x224, 0x5, 0x7F, 8);       /* [14:8]: VCA15_SFCHG_RRATE; soft change rising step=0.5us */
	ret = pmic_config_interface(0x224, 0x1, 0x1, 7);        /* [7:7]: VCA15_SFCHG_FEN; soft change falling enable */
	ret = pmic_config_interface(0x224, 0x17, 0x7F, 0);      /* [6:0]: VCA15_SFCHG_FRATE; soft change falling step=2us */
	ret = pmic_config_interface(0x22A, 0x0, 0x7F, 0);       /* [6:0]: VCA15_VOSEL_SLEEP; sleep mode only (0.7V) */
	ret = pmic_config_interface(0x238, 0x1, 0x1, 8);        /* [8:8]: VCA15_VSLEEP_EN; set sleep mode reference voltage from R2R to V2V */
	ret = pmic_config_interface(0x238, 0x3, 0x3, 4);        /* [5:4]: VCA15_VOSEL_TRANS_EN; rising & falling enable */
	ret = pmic_config_interface(0x244, 0x1, 0x1, 5);        /* [5:5]: VSRMCA15_TRACK_SLEEP_CTRL; */
	ret = pmic_config_interface(0x246, 0x0, 0x3, 4);        /* [5:4]: VSRMCA15_VOSEL_SEL; */
	ret = pmic_config_interface(0x24A, 0x1, 0x1, 15);       /* [15:15]: VSRMCA15_SFCHG_REN; */
	ret = pmic_config_interface(0x24A, 0x5, 0x7F, 8);       /* [14:8]: VSRMCA15_SFCHG_RRATE; */
	ret = pmic_config_interface(0x24A, 0x1, 0x1, 7);        /* [7:7]: VSRMCA15_SFCHG_FEN; */
	ret = pmic_config_interface(0x24A, 0x17, 0x7F, 0);      /* [6:0]: VSRMCA15_SFCHG_FRATE; */
	ret = pmic_config_interface(0x250, 0x00, 0x7F, 0);      /* [6:0]: VSRMCA15_VOSEL_SLEEP; Sleep mode setting only (0.7V) */
	ret = pmic_config_interface(0x25E, 0x1, 0x1, 8);        /* [8:8]: VSRMCA15_VSLEEP_EN; set sleep mode reference voltage from R2R to V2V */
	ret = pmic_config_interface(0x25E, 0x3, 0x3, 4);        /* [5:4]: VSRMCA15_VOSEL_TRANS_EN; rising & falling enable */
	ret = pmic_config_interface(0x270, 0x1, 0x1, 1);        /* [1:1]: VCORE_VOSEL_CTRL; sleep mode voltage control follow SRCLKEN */
	ret = pmic_config_interface(0x272, 0x0, 0x3, 4);        /* [5:4]: VCORE_VOSEL_SEL; */
	ret = pmic_config_interface(0x276, 0x1, 0x1, 15);       /* [15:15]: VCORE_SFCHG_REN; */
	ret = pmic_config_interface(0x276, 0x5, 0x7F, 8);       /* [14:8]: VCORE_SFCHG_RRATE; */
	ret = pmic_config_interface(0x276, 0x17, 0x7F, 0);      /* [6:0]: VCORE_SFCHG_FRATE; */
	ret = pmic_config_interface(0x27C, 0x0, 0x7F, 0);       /* [6:0]: VCORE_VOSEL_SLEEP; Sleep mode setting only (0.7V) */
	ret = pmic_config_interface(0x28A, 0x1, 0x1, 8);        /* [8:8]: VCORE_VSLEEP_EN; Sleep mode HW control  R2R to VtoV */
	ret = pmic_config_interface(0x28A, 0x0, 0x3, 4);        /* [5:4]: VCORE_VOSEL_TRANS_EN; Follows MT6320 VCORE setting. */
	ret = pmic_config_interface(0x28A, 0x3, 0x3, 0);        /* [1:0]: VCORE_TRANSTD; */
	ret = pmic_config_interface(0x28E, 0x1, 0x3, 8);        /* [9:8]: RG_VGPU_CSL; for OC protection */
	ret = pmic_config_interface(0x29C, 0x1, 0x1, 15);       /* [15:15]: VGPU_SFCHG_REN; */
	ret = pmic_config_interface(0x29C, 0x5, 0x7F, 8);       /* [14:8]: VGPU_SFCHG_RRATE; */
	ret = pmic_config_interface(0x29C, 0x17, 0x7F, 0);      /* [6:0]: VGPU_SFCHG_FRATE; */
	ret = pmic_config_interface(0x2B0, 0x0, 0x3, 4);        /* [5:4]: VGPU_VOSEL_TRANS_EN; */
	ret = pmic_config_interface(0x2B0, 0x3, 0x3, 0);        /* [1:0]: VGPU_TRANSTD; */
	ret = pmic_config_interface(0x332, 0x0, 0x3, 4);        /* [5:4]: VPCA7_VOSEL_SEL; */
	ret = pmic_config_interface(0x336, 0x1, 0x1, 15);       /* [15:15]: VPCA7_SFCHG_REN; */
	ret = pmic_config_interface(0x336, 0x5, 0x7F, 8);       /* [14:8]: VPCA7_SFCHG_RRATE; */
	ret = pmic_config_interface(0x336, 0x1, 0x1, 7);        /* [7:7]: VPCA7_SFCHG_FEN; */
	ret = pmic_config_interface(0x336, 0x17, 0x7F, 0);      /* [6:0]: VPCA7_SFCHG_FRATE; */
	ret = pmic_config_interface(0x33C, 0x18, 0x7F, 0);      /* [6:0]: VPCA7_VOSEL_SLEEP; */
	ret = pmic_config_interface(0x34A, 0x1, 0x1, 8);        /* [8:8]: VPCA7_VSLEEP_EN; */
	ret = pmic_config_interface(0x34A, 0x3, 0x3, 4);        /* [5:4]: VPCA7_VOSEL_TRANS_EN; */
	ret = pmic_config_interface(0x356, 0x0, 0x1, 5);        /* [5:5]: VSRMCA7_TRACK_SLEEP_CTRL; */
	ret = pmic_config_interface(0x358, 0x0, 0x3, 4);        /* [5:4]: VSRMCA7_VOSEL_SEL; */
	ret = pmic_config_interface(0x35C, 0x1, 0x1, 15);       /* [15:15]: VSRMCA7_SFCHG_REN; */
	ret = pmic_config_interface(0x35C, 0x5, 0x7F, 8);       /* [14:8]: VSRMCA7_SFCHG_RRATE; */
	ret = pmic_config_interface(0x35C, 0x1, 0x1, 7);        /* [7:7]: VSRMCA7_SFCHG_FEN; */
	ret = pmic_config_interface(0x35C, 0x17, 0x7F, 0);      /* [6:0]: VSRMCA7_SFCHG_FRATE; */
	ret = pmic_config_interface(0x362, 0x18, 0x7F, 0);      /* [6:0]: VSRMCA7_VOSEL_SLEEP; */
	ret = pmic_config_interface(0x370, 0x1, 0x1, 8);        /* [8:8]: VSRMCA7_VSLEEP_EN; */
	ret = pmic_config_interface(0x370, 0x3, 0x3, 4);        /* [5:4]: VSRMCA7_VOSEL_TRANS_EN; */
	ret = pmic_config_interface(0x39C, 0x1, 0x1, 8);        /* [8:8]: VDRM_VSLEEP_EN; */
	ret = pmic_config_interface(0x440, 0x1, 0x1, 2);        /* [2:2]: VIBR_THER_SHEN_EN; */
	ret = pmic_config_interface(0x500, 0x1, 0x1, 5);        /* [5:5]: THR_HWPDN_EN; */
	ret = pmic_config_interface(0x502, 0x1, 0x1, 3);        /* [3:3]: RG_RST_DRVSEL; */
	ret = pmic_config_interface(0x502, 0x1, 0x1, 2);        /* [2:2]: RG_EN_DRVSEL; */
	ret = pmic_config_interface(0x508, 0x1, 0x1, 1);        /* [1:1]: PWRBB_DEB_EN; */
	ret = pmic_config_interface(0x50C, 0x1, 0x1, 12);       /* [12:12]: VSRMCA15_PG_H2L_EN; */
	ret = pmic_config_interface(0x50C, 0x1, 0x1, 11);       /* [11:11]: VPCA15_PG_H2L_EN; */
	ret = pmic_config_interface(0x50C, 0x1, 0x1, 10);       /* [10:10]: VCORE_PG_H2L_EN; */
	ret = pmic_config_interface(0x50C, 0x1, 0x1, 9);        /* [9:9]: VSRMCA7_PG_H2L_EN; */
	ret = pmic_config_interface(0x50C, 0x1, 0x1, 8);        /* [8:8]: VPCA7_PG_H2L_EN; */
	ret = pmic_config_interface(0x512, 0x1, 0x1, 1);        /* [1:1]: STRUP_PWROFF_PREOFF_EN; */
	ret = pmic_config_interface(0x512, 0x1, 0x1, 0);        /* [0:0]: STRUP_PWROFF_SEQ_EN; */
	ret = pmic_config_interface(0x55E, 0xFC, 0xFF, 8);      /* [15:8]: RG_ADC_TRIM_CH_SEL; */
	ret = pmic_config_interface(0x560, 0x1, 0x1, 1);        /* [1:1]: FLASH_THER_SHDN_EN; */
	ret = pmic_config_interface(0x566, 0x1, 0x1, 1);        /* [1:1]: KPLED_THER_SHDN_EN; */
	ret = pmic_config_interface(0x600, 0x1, 0x1, 9);        /* [9:9]: SPK_THER_SHDN_L_EN; */
	ret = pmic_config_interface(0x604, 0x1, 0x1, 0);        /* [0:0]: RG_SPK_INTG_RST_L; */
	ret = pmic_config_interface(0x606, 0x1, 0x1, 9);        /* [9:9]: SPK_THER_SHDN_R_EN; */
	ret = pmic_config_interface(0x60A, 0x1, 0xF, 11);       /* [14:11]: RG_SPKPGA_GAINR; */
	ret = pmic_config_interface(0x612, 0x1, 0xF, 8);        /* [11:8]: RG_SPKPGA_GAINL; */
	ret = pmic_config_interface(0x632, 0x1, 0x1, 8);        /* [8:8]: FG_SLP_EN; */
	ret = pmic_config_interface(0x638, 0xFFC2, 0xFFFF, 0);  /* [15:0]: FG_SLP_CUR_TH; */
	ret = pmic_config_interface(0x63A, 0x14, 0xFF, 0);      /* [7:0]: FG_SLP_TIME; */
	ret = pmic_config_interface(0x63C, 0xFF, 0xFF, 8);      /* [15:8]: FG_DET_TIME; */
	ret = pmic_config_interface(0x714, 0x1, 0x1, 7);        /* [7:7]: RG_LCLDO_ENC_REMOTE_SENSE_VA28; */
	ret = pmic_config_interface(0x714, 0x1, 0x1, 4);        /* [4:4]: RG_LCLDO_REMOTE_SENSE_VA33; */
	ret = pmic_config_interface(0x714, 0x1, 0x1, 1);        /* [1:1]: RG_HCLDO_REMOTE_SENSE_VA33; */
	ret = pmic_config_interface(0x71A, 0x1, 0x1, 15);       /* [15:15]: RG_NCP_REMOTE_SENSE_VA18; */
	ret = pmic_config_interface(0x260, 0x10, 0x7F, 8);      /* [14:8]: VSRMCA15_VOSEL_OFFSET; set offset=100mV */
	ret = pmic_config_interface(0x260, 0x0, 0x7F, 0);       /* [6:0]: VSRMCA15_VOSEL_DELTA; set delta=0mV */
	ret = pmic_config_interface(0x262, 0x48, 0x7F, 8);      /* [14:8]: VSRMCA15_VOSEL_ON_HB; set HB=1.15V */
	ret = pmic_config_interface(0x262, 0x0, 0x7F, 0);       /* [6:0]: VSRMCA15_VOSEL_ON_LB; set LB=0.7V */
	ret = pmic_config_interface(0x264, 0x0, 0x7F, 0);       /* [6:0]: VSRMCA15_VOSEL_SLEEP_LB; set sleep LB=0.7V   */
	ret = pmic_config_interface(0x372, 0x4, 0x7F, 8);       /* [14:8]: VSRMCA7_VOSEL_OFFSET; set offset=25mV */
	ret = pmic_config_interface(0x372, 0x0, 0x7F, 0);       /* [6:0]: VSRMCA7_VOSEL_DELTA; set delta=0mV */
	ret = pmic_config_interface(0x374, 0x5C, 0x7F, 8);      /* [14:8]: VSRMCA7_VOSEL_ON_HB; set HB=1.275V */
	ret = pmic_config_interface(0x374, 0x38, 0x7F, 0);      /* [6:0]: VSRMCA7_VOSEL_ON_LB; set LB=1.05000V */
	ret = pmic_config_interface(0x376, 0x18, 0x7F, 0);      /* [6:0]: VSRMCA7_VOSEL_SLEEP_LB; set sleep LB=0.85000V */
	ret = pmic_config_interface(0x21E, 0x3, 0x3, 0);        /* [1:1]: VCA15_VOSEL_CTRL, VCA15_EN_CTRL; DVS HW control by SRCLKEN */
	ret = pmic_config_interface(0x244, 0x3, 0x3, 0);        /* [1:1]: VSRMCA15_VOSEL_CTRL, VSRAM15_EN_CTRL; */
	ret = pmic_config_interface(0x330, 0x0, 0x1, 1);        /* [1:1]: VPCA7_VOSEL_CTRL; */
	ret = pmic_config_interface(0x356, 0x0, 0x1, 1);        /* [1:1]: VSRMCA7_VOSEL_CTRL; */
	ret = pmic_config_interface(0x21E, 0x1, 0x1, 4);        /* [4:4]: VCA15_TRACK_ON_CTRL; DVFS tracking enable */
	ret = pmic_config_interface(0x244, 0x1, 0x1, 4);        /* [4:4]: VSRMCA15_TRACK_ON_CTRL; */
	ret = pmic_config_interface(0x330, 0x0, 0x1, 4);        /* [4:4]: VPCA7_TRACK_ON_CTRL; */
	ret = pmic_config_interface(0x356, 0x0, 0x1, 4);        /* [4:4]: VSRMCA7_TRACK_ON_CTRL; */
	ret = pmic_config_interface(0x134, 0x3, 0x3, 14);       /* [15:14]: VGPU OC; */
	ret = pmic_config_interface(0x134, 0x3, 0x3, 2);        /* [3:2]: VCA15 OC; */
}

enum {
	Vcore_HV = 0x38 + 0x11,  /* 1.15 */
	Vcore_NV = 0x38 + 0x00,  /* 1.05 */
	Vcore_LV = 0x38 - 0x11,  /* 0.95  , +8 =>1.00V */
	Vmem_HV = 0x56 + 0x09,
	Vmem_NV = 0x56 + 0x00,
	Vmem_LV = 0x56 - 0x09
};

static void pmic_default_buck_voltage(void)
{
	u32 reg_val = 0;
	u32 buck_val = 0;

	pmic_read_interface(EFUSE_DOUT_288_303, &reg_val, 0xFFFF, 0);
	if ((reg_val & 0x01) == 0x01) {
		printk(BIOS_INFO, "[EFUSE_DOUT_288_303] FUSE 288=%#x\n", reg_val);

		/* VCORE */
		pmic_read_interface(EFUSE_DOUT_256_271, &reg_val, 0xF, 12);
		pmic_read_interface(VCORE_CON9, &buck_val, PMIC_VCORE_VOSEL_MASK, PMIC_VCORE_VOSEL_SHIFT);
		buck_val = (buck_val & 0x07) | (reg_val << 3);
		pmic_config_interface(VCORE_CON9, buck_val, PMIC_VCORE_VOSEL_MASK, PMIC_VCORE_VOSEL_SHIFT);
		pmic_config_interface(VCORE_CON10, buck_val, PMIC_VCORE_VOSEL_ON_MASK, PMIC_VCORE_VOSEL_ON_SHIFT);

		pmic_read_interface(EFUSE_DOUT_272_287, &reg_val, 0xFFFF, 0);
		/* VCA15 */
		buck_val = 0;
		pmic_read_interface(VCA15_CON9, &buck_val, PMIC_VCA15_VOSEL_MASK, PMIC_VCA15_VOSEL_SHIFT);
		buck_val = (buck_val & 0x07) | ((reg_val & 0x0F) << 3);
		pmic_config_interface(VCA15_CON9, buck_val, PMIC_VCA15_VOSEL_MASK, PMIC_VCA15_VOSEL_SHIFT);
		pmic_config_interface(VCA15_CON10, buck_val, PMIC_VCA15_VOSEL_ON_MASK, PMIC_VCA15_VOSEL_ON_SHIFT);

		/* VSAMRCA15 */
		buck_val = 0;
		pmic_read_interface(VSRMCA15_CON9, &buck_val, PMIC_VSRMCA15_VOSEL_MASK, PMIC_VSRMCA15_VOSEL_SHIFT);
		buck_val = (buck_val & 0x07) | ((reg_val & 0xF0) >> 1);
		pmic_config_interface(VSRMCA15_CON9, buck_val, PMIC_VSRMCA15_VOSEL_MASK, PMIC_VSRMCA15_VOSEL_SHIFT);
		pmic_config_interface(VSRMCA15_CON10, buck_val, PMIC_VSRMCA15_VOSEL_ON_MASK, PMIC_VSRMCA15_VOSEL_ON_SHIFT);

		/* VCA7 */
		buck_val = 0;
		pmic_read_interface(VPCA7_CON9, &buck_val, PMIC_VPCA7_VOSEL_MASK, PMIC_VPCA7_VOSEL_SHIFT);
		buck_val = (buck_val & 0x07) | ((reg_val & 0xF00) >> 5);
		pmic_config_interface(VPCA7_CON9, buck_val, PMIC_VPCA7_VOSEL_MASK, PMIC_VPCA7_VOSEL_SHIFT);
		pmic_config_interface(VPCA7_CON10, buck_val, PMIC_VPCA7_VOSEL_ON_MASK, PMIC_VPCA7_VOSEL_ON_SHIFT);

		/* VSAMRCA7 */
		buck_val = 0;
		pmic_read_interface(VSRMCA7_CON9, &buck_val, PMIC_VPCA7_VOSEL_MASK, PMIC_VPCA7_VOSEL_SHIFT);
		buck_val = (buck_val & 0x07) | ((reg_val & 0xF000) >> 9);
		pmic_config_interface(VSRMCA7_CON9, buck_val, PMIC_VSRMCA7_VOSEL_MASK, PMIC_VSRMCA7_VOSEL_SHIFT);
		pmic_config_interface(VSRMCA7_CON10, buck_val, PMIC_VSRMCA7_VOSEL_ON_MASK, PMIC_VSRMCA7_VOSEL_ON_SHIFT);

		/* set the power control by register(use original) */
		pmic_config_interface(BUCK_CON3, 0x1, 0x1, 12);
	}

	/* Set VPCA7 to 1.2V */
	pmic_config_interface(VPCA7_CON9, 0x50, PMIC_VPCA7_VOSEL_MASK,
		PMIC_VPCA7_VOSEL_SHIFT);
	pmic_config_interface(VPCA7_CON10, 0x50, PMIC_VPCA7_VOSEL_ON_MASK,
		PMIC_VPCA7_VOSEL_ON_SHIFT);
	/* Enable VGP6 */
	pmic_config_interface(DIGLDO_CON10, 0x1, 0x1, 15);
	/* Set VGP6 to 3.3V */
	pmic_config_interface(DIGLDO_CON33, 0x07, 0x07, 5);

}

u32 pmic_init(void)
{
	u32 ret_code = PMIC_TEST_PASS;
	int ret_val = 0;
	u32 reg_val = 0;

	printk(BIOS_INFO, "[%s] Start..................\n", __func__);

	/* Adjust default BUCK voltage */
	pmic_default_buck_voltage();

	/* Enable PMIC RST function (depends on main chip RST function) */
	/*
	 *      state1: RG_SYSRSTB_EN = 1, RG_STRUP_MAN_RST_EN=1, RG_RST_PART_SEL=1
	 *      state2: RG_SYSRSTB_EN = 1, RG_STRUP_MAN_RST_EN=0, RG_RST_PART_SEL=1
	 *      state3: RG_SYSRSTB_EN = 1, RG_STRUP_MAN_RST_EN=x, RG_RST_PART_SEL=0
	 */
	ret_val = pmic_config_interface(TOP_RST_MISC, 0x1, PMIC_RG_SYSRSTB_EN_MASK, PMIC_RG_SYSRSTB_EN_SHIFT);
	ret_val = pmic_config_interface(TOP_RST_MISC, 0x0, PMIC_RG_STRUP_MAN_RST_EN_MASK, PMIC_RG_STRUP_MAN_RST_EN_SHIFT);
	ret_val = pmic_config_interface(TOP_RST_MISC, 0x1, PMIC_RG_RST_PART_SEL_MASK, PMIC_RG_RST_PART_SEL_SHIFT);

	/*  Enable AP watchdog reset */
	ret_val = pmic_config_interface(TOP_RST_MISC, 0x0, PMIC_RG_AP_RST_DIS_MASK, PMIC_RG_AP_RST_DIS_SHIFT);
	ret_val = pmic_read_interface(TOP_RST_MISC, &reg_val, 0xFFFF, 0);
	printk(BIOS_INFO, "[%s] Enable PMIC RST function "
	       "(depends on main chip RST function) Reg[%#x]=%#x\n",
	       __func__, TOP_RST_MISC, reg_val);

	/* Enable CA15 by default for different PMIC behavior */
	pmic_config_interface(VCA15_CON7, 0x1, PMIC_VCA15_EN_MASK, PMIC_VCA15_EN_SHIFT);
	pmic_config_interface(VSRMCA15_CON7, 0x1, PMIC_VSRMCA15_EN_MASK, PMIC_VSRMCA15_EN_SHIFT);
	pmic_config_interface(VPCA7_CON7, 0x1, PMIC_VPCA7_EN_MASK, PMIC_VPCA7_EN_SHIFT);
	udelay(200); /* delay for Buck ready */
	g_ca15_ready = 1;

	ret_val = pmic_read_interface(VCA15_CON7, &reg_val, 0xFFFF, 0);
	printk(BIOS_INFO, "Reg[%#x]=%#x\n", VCA15_CON7, reg_val);
	ret_val = pmic_read_interface(VSRMCA15_CON7, &reg_val, 0xFFFF, 0);
	printk(BIOS_INFO, "Reg[%#x]=%#x\n", VSRMCA15_CON7, reg_val);

	/* pmic initial setting */
	pmic_init_setting_v1();
	printk(BIOS_INFO, "[pmic_init_setting_v1] Done\n");

	/* 26M clock amplitute adjust */
	pmic_config_interface(RG_DCXO_ANALOG_CON1, 0x0, PMIC_RG_DCXO_LDO_BB_V_MASK, PMIC_RG_DCXO_LDO_BB_V_SHIFT);
	pmic_config_interface(RG_DCXO_ANALOG_CON1, 0x1, PMIC_RG_DCXO_ATTEN_BB_MASK, PMIC_RG_DCXO_ATTEN_BB_SHIFT);

	pmic_read_interface(EFUSE_DOUT_304_319, &reg_val, 0xFFFF, 0);
	if ((reg_val & 0x8000) == 0) {
		pmic_config_interface(BUCK_K_CON0, 0x0041, 0xFFFF, 0);
		pmic_config_interface(BUCK_K_CON0, 0x0040, 0xFFFF, 0);
		pmic_config_interface(BUCK_K_CON0, 0x0050, 0xFFFF, 0);
	}

	pmic_config_interface(CHR_CON13, 0x00, PMIC_RG_CHRWDT_EN_MASK, PMIC_RG_CHRWDT_EN_SHIFT);

	/* Set VDRM to 1.21875V only for 8173 */
	pmic_config_interface(VDRM_CON9, 0x43, 0x7F, 0);
	pmic_config_interface(VDRM_CON10, 0x43, 0x7F, 0);

	pmic_config_interface(VSRMCA7_CON9, 0x40, PMIC_VSRMCA7_VOSEL_MASK, PMIC_VSRMCA7_VOSEL_SHIFT);
	pmic_config_interface(VSRMCA7_CON10, 0x40, PMIC_VSRMCA7_VOSEL_ON_MASK, PMIC_VSRMCA7_VOSEL_ON_SHIFT);
	da9212_driver_probe();
	printk(BIOS_INFO, "[%s] Done...................\n", __func__);

	return ret_code;
}
