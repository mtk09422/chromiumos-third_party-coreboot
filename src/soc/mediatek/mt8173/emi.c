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

#include <arch/io.h>
#include <assert.h>
#include <delay.h>
#include <string.h>
#include <console/console.h>

#include <soc/mt8173.h>
#include <soc/addressmap.h>
#include <soc/emi.h>
#include <soc/emi_hw.h>
#include <soc/dramc_common.h>
#include <soc/dramc_register.h>
#include <soc/dramc_pi_api.h>
#include <soc/pll.h>

extern u32 seclib_get_devinfo_with_index(u32 index);

DRAMC_CTX_T *psCurrDramCtx;

DRAMC_CTX_T DramCtx_LPDDR3 = {

	CHANNEL_A,		/* DRAM_CHANNEL */
	TYPE_LPDDR3,	        /* DRAM_DRAM_TYPE_T */
	PACKAGE_SBS,	        /* DRAM_PACKAGE_T */
	DATA_WIDTH_32BIT,	/* DRAM_DATA_WIDTH_T */
	DEFAULT_TEST2_1_CAL,	/* test2_1; */
	DEFAULT_TEST2_2_CAL,	/* test2_2; */
	TEST_XTALK_PATTERN,	/* test_pattern; */
	896,			/* dram frequency */
	533,			/* frequency_low; */
	DISABLE,		/* fglow_freq_write_en; */
	DISABLE,		/* ssc_en; */
	DISABLE			/* en_4bitMux; */
};

DRAMC_CTX_T DramCtx_PCDDR3 = {

	CHANNEL_A,		/* DRAM_CHANNEL */
	TYPE_PCDDR3,		/* DRAM_DRAM_TYPE_T */
	PACKAGE_SBS,		/* DRAM_PACKAGE_T */
	DATA_WIDTH_32BIT,	/* DRAM_DATA_WIDTH_T */
	DEFAULT_TEST2_1_CAL,	/* test2_1; */
	DEFAULT_TEST2_2_CAL,	/* test2_2; */
	TEST_XTALK_PATTERN,	/* test_pattern; Audio or Xtalk. */
	900,			/* frequency; */
	533,			/* frequency_low; */
	ENABLE,			/* fglow_freq_write_en; */
	DISABLE,		/* ssc_en; */
	DISABLE			/* en_4bitMux; */
};

static int enable_combo_dis;

extern int num_of_emi_records;
extern EMI_SETTINGS emi_settings[];

unsigned int g_ddr_reserve_enable;
unsigned int g_ddr_reserve_success;

typedef struct {
	u32 pll_setting_num;
	u32 freq_setting_num;
	u32 low_freq_pll_setting_addr;
	u32 low_freq_cha_setting_addr;
	u32 low_freq_chb_setting_addr;
	u32 high_freq_pll_setting_addr;
	u32 high_freq_cha_setting_addr;
	u32 high_freq_chb_setting_addr;
} vcore_dvfs_info_t;

#define PMIC_6397_VCORE_VMEM
extern u32 pmic_config_interface(u32 RegNum, u32 val, u32 MASK, u32 SHIFT);

/* Options used in HQA stress test */
enum {
	VcHV_VmHV = 0,
	VcLV_VmLV = 0,
	VcNV_VmNV = 1,
	VcLV_VmNV = 0
};

/* Vcore voltage adjustment */
enum {
	Vcore_HHV_LPPDR3 = 0x60,   /* 1.300V */
	Vcore_HV_LPPDR3  = 0x48,   /* 1.150V */
	Vcore_NV_LPPDR3  = 0x44,   /* 1.125V */
	Vcore_LV_LPPDR3  = 0x34,   /* 1.025V */
	Vcore_LLV_LPPDR3 = 0x25    /* 0.931V */
};

/* Vmem voltage adjustment*/
enum {
	Vmem_HV_LPDDR3 = 0x50,   /* 1.300V */
	Vmem_NV_LPDDR3 = 0x44,   /* 1.225V */
	Vmem_LV_LPDDR3 = 0x36    /* 1.138V */
};

enum {
	VCORE_CON9 = 0x0278,
	VCORE_CON10 = 0x027A,
	VDRM_CON9 = 0x038A,
	VDRM_CON10 = 0x038C
};

static void DRAM_Vcore_adjust(int nAdjust)
{
	switch (nAdjust) {
	case 1:
		pmic_config_interface(VCORE_CON9, Vcore_HV_LPPDR3, 0x7F, 0);
		pmic_config_interface(VCORE_CON10, Vcore_HV_LPPDR3, 0x7F, 0);
		break;
	case 2:
		pmic_config_interface(VCORE_CON9, Vcore_NV_LPPDR3, 0x7F, 0);
		pmic_config_interface(VCORE_CON10, Vcore_NV_LPPDR3, 0x7F, 0);
		break;
	case 3:
		pmic_config_interface(VCORE_CON9, Vcore_LV_LPPDR3, 0x7F, 0);
		pmic_config_interface(VCORE_CON10, Vcore_LV_LPPDR3, 0x7F, 0);
		break;
	default:
		pmic_config_interface(VCORE_CON9, Vcore_NV_LPPDR3, 0x7F, 0);
		pmic_config_interface(VCORE_CON10, Vcore_NV_LPPDR3, 0x7F, 0);
		break;
	}
}

static void DRAM_Vmem_adjust(int nAdjust)
{
	switch (nAdjust) {
	case 1:
		pmic_config_interface(VDRM_CON9, Vmem_HV_LPDDR3, 0x7F, 0);
		pmic_config_interface(VDRM_CON10, Vmem_HV_LPDDR3, 0x7F, 0);
		break;
	case 2:
		pmic_config_interface(VDRM_CON9, Vmem_NV_LPDDR3, 0x7F, 0);
		pmic_config_interface(VDRM_CON10, Vmem_NV_LPDDR3, 0x7F, 0);
		break;
	case 3:
		pmic_config_interface(VDRM_CON9, Vmem_LV_LPDDR3, 0x7F, 0);
		pmic_config_interface(VDRM_CON10, Vmem_LV_LPDDR3, 0x7F, 0);
		break;
	default:
		pmic_config_interface(VDRM_CON9, Vmem_NV_LPDDR3, 0x7F, 0);
		pmic_config_interface(VDRM_CON10, Vmem_NV_LPDDR3, 0x7F, 0);
		break;
	}
}

static void EMI_Init(DRAMC_CTX_T *p, EMI_SETTINGS *emi_set)
{
	/* ----------------EMI Setting-------------------- */
	*(volatile unsigned *)(EMI_APB_BASE+0x00000028) = 0x08420000;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000060) = 0x40000500;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000140) = 0x20406188;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000144) = 0x20406188;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000100) = 0x7f077a49;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000110) = 0xa0a070dd;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000118) = 0x07007046;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000120) = 0x40407046;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000128) = 0xa0a070c6;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000130) = 0xffff7047;

	*(volatile unsigned *)(EMI_APB_BASE+0x00000148) = 0x9719595e;
	*(volatile unsigned *)(EMI_APB_BASE+0x0000014c) = 0x9719595e;

	*(volatile unsigned *)(EMI_APB_BASE+0x00000000) = emi_set->EMI_CONA_VAL;
	*(volatile unsigned *)(EMI_APB_BASE+0x000000f8) = 0x00000000;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000400) = 0x00ff0001;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000008) = 0x17283544;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000010) = 0x0a1a0b1a;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000018) = 0x00000000;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000020) = 0xFFFF0848;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000030) = 0x2b2b2a38;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000038) = emi_set->EMI_CONH_VAL;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000158) = 0x00010800;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000078) = 0x80030303;
	*(volatile unsigned *)(EMI_APB_BASE+0x0000015c) = 0x80030303;

	*(volatile unsigned *)(EMI_APB_BASE+0x00000150) = 0x64f3fc79;
	*(volatile unsigned *)(EMI_APB_BASE+0x00000154) = 0x64f3fc79;

	/* ==============Scramble address========================== */
	/* Defer WR threthold */
	*(volatile unsigned *)(EMI_APB_BASE+0x000000f0) = 0x38470000;
	/* Reserve bufer */
	/* SBR not defined */
	/* MDMCU don't always ULTRA, but small age */
	*(volatile unsigned *)(EMI_APB_BASE+0x00000078) = 0x34220c3f;
	*(volatile unsigned *)(EMI_APB_BASE+0x000000e8) = 0x00060124;
	/* Turn on M1 Ultra and all port DRAMC hi enable */
	*(volatile unsigned *)(EMI_APB_BASE+0x00000158) = 0xff03ff00;
	/* RFF)_PBC_MASK; [9] decrease noSBR push to DRAMC */

	/* Page hit is high */
	*(volatile unsigned *)(EMI_APB_BASE+0x00000060) = 0x400005ff;
	*(volatile unsigned *)(EMI_APB_BASE+0x000000d0) = 0xCCCCCCCC;
	*(volatile unsigned *)(EMI_APB_BASE+0x000000d8) = 0xcccccccc;
}

static void do_calib(DRAMC_CTX_T *p, EMI_SETTINGS *emi_set, int skip_dual_freq_k)
{
	u8 ucstatus = 0;
	u32 u4value;

	/* DDR_CHANNEL_INIT: */
	p->channel = CHANNEL_A;
	DramcSwImpedanceCal((DRAMC_CTX_T *)p, 1);

	/* Run again here for different voltage. */
	/* For preloader, if the following code is executed */
	/* after voltage change, no need. */
	/* SPM_CONTROL_AFTERK */
	TransferToRegControl();

	MemPllPreInit(p);
	MemPllInit(p);
	udelay(1);
	mt_mempll_cali(p);
	DramcDiv2PhaseSync((DRAMC_CTX_T *)p);

	{
		/* Calibration */
		/* CA_WR_ENABLE defined */
		p->channel = CHANNEL_A;
		DramcCATraining((DRAMC_CTX_T *)p);
		p->channel = CHANNEL_B;
		DramcCATraining((DRAMC_CTX_T *)p);

		/* COMBO_MCP defined */
		p->channel = CHANNEL_A;
		DramcWriteLeveling((DRAMC_CTX_T *)p, emi_set);
		p->channel = CHANNEL_B;
		DramcWriteLeveling((DRAMC_CTX_T *)p, emi_set);

		/* DUAL_RANKS defined */
		if (uiDualRank) {
			p->channel = CHANNEL_A;
			DualRankDramcRxdqsGatingCal((DRAMC_CTX_T *)p);
			p->channel = CHANNEL_B;
			DualRankDramcRxdqsGatingCal((DRAMC_CTX_T *)p);
		} else {
			p->channel = CHANNEL_A;
			DramcRxdqsGatingCal((DRAMC_CTX_T *)p);
			p->channel = CHANNEL_B;
			DramcRxdqsGatingCal((DRAMC_CTX_T *)p);
		}

		if (((DRAMC_CTX_T *)p)->fglow_freq_write_en == ENABLE) {

			mcSHOW_DBG_MSG2("**********************");
			mcSHOW_DBG_MSG2("NOTICE*************************\n");
			mcSHOW_DBG_MSG2("Low speed write and high speed ");
			mcSHOW_DBG_MSG2("read calibration...\n");
			mcSHOW_DBG_MSG2("********************\n");
			mcSHOW_DBG_MSG2("*********************************\n");

			/* change low frequency and use test engine2 to write */
			/* data, after write, recover back to the */
			/* original frequency */

			/* do channel A & B low frequency write simultaneously */
			CurrentRank = 0;
			DramcLowFreqWrite((DRAMC_CTX_T *)p);
			/* DUAL_RANKS */
			if (uiDualRank) {
				CurrentRank = 1;
				/* Swap CS0 and CS1. */
				u32 addr = mcSET_DRAMC_REG_ADDR(p, 0x110);

				ucstatus |= ucDram_Register_Read(addr, &u4value);
				u4value = u4value | 0x08;
				ucstatus |= ucDram_Register_Write(addr, u4value);

				/* do channel A & B low frequency write */
				/* simultaneously */
				DramcLowFreqWrite((DRAMC_CTX_T *)p);

				/* Swap CS back. */
				addr = mcSET_DRAMC_REG_ADDR(p, 0x110);
				ucstatus |= ucDram_Register_Read(addr, &u4value);
				u4value = u4value & (~0x08);
				ucstatus |= ucDram_Register_Write(addr, u4value);
				CurrentRank = 0;
			}
		}
		/* DUAL_RANKS */
		if (uiDualRank) {
			p->channel = CHANNEL_A;
			DramcDualRankRxdatlatCal((DRAMC_CTX_T *)p);
			p->channel = CHANNEL_B;
			DramcDualRankRxdatlatCal((DRAMC_CTX_T *)p);
		} else {
			p->channel = CHANNEL_A;
			DramcRxdatlatCal((DRAMC_CTX_T *)p);
			p->channel = CHANNEL_B;
			DramcRxdatlatCal((DRAMC_CTX_T *)p);
		}

		/* RX_DUTY_CALIBRATION defined */
		p->channel = CHANNEL_A;
		DramcClkDutyCal(p);
		p->channel = CHANNEL_B;
		DramcClkDutyCal(p);

		p->channel = CHANNEL_A;
		DramcRxWindowPerbitCal((DRAMC_CTX_T *)p);
		p->channel = CHANNEL_B;
		DramcRxWindowPerbitCal((DRAMC_CTX_T *)p);

		p->channel = CHANNEL_A;
		DramcTxWindowPerbitCal((DRAMC_CTX_T *)p);
		p->channel = CHANNEL_B;
		DramcTxWindowPerbitCal((DRAMC_CTX_T *)p);

		/* Set here in order to save for frequency jump. */
		p->channel = CHANNEL_A;
		DramcRANKINCTLConfig(p);
		p->channel = CHANNEL_B;
		DramcRANKINCTLConfig(p);
	}

	p->channel = CHANNEL_A;
	DramcRunTimeConfig(p);
	p->channel = CHANNEL_B;
	DramcRunTimeConfig(p);

	/* SPM_CONTROL_AFTERK */
	TransferToSPMControl();
}

static void Init_DRAM(DRAMC_CTX_T *p, EMI_SETTINGS *emi_set)
{
	/* COMBO_MCP defined */
	EMI_Init(p, emi_set);

	p->channel = CHANNEL_A;
	DramcPreInit((DRAMC_CTX_T *)p, emi_set);

	p->channel = CHANNEL_B;
	DramcPreInit((DRAMC_CTX_T *)p, emi_set);

	DramcDiv2PhaseSync((DRAMC_CTX_T *)p);

	p->channel = CHANNEL_A;
	DramcInit((DRAMC_CTX_T *)p, emi_set);
	p->channel = CHANNEL_B;
	DramcInit((DRAMC_CTX_T *)p, emi_set);
}

extern const u32 uiLPDDR_PHY_Mapping_POP_CHA[32];
extern const u32 uiLPDDR_PHY_Mapping_POP_CHB[32];

static unsigned int DRAM_MRR(int MRR_num)
{
	unsigned int MRR_value = 0x0;
	unsigned int ucstatus, u4value;
	DRAMC_CTX_T *p = psCurrDramCtx;
	u32 addr;

	if ((p->dram_type == TYPE_LPDDR3) || (p->dram_type == TYPE_LPDDR2)) {
		/* set DQ bit 0, 1, 2, 3, 4, 5, 6, 7 pinmux */
		if (p->channel == CHANNEL_A) {
			if (p->dram_type == TYPE_LPDDR3) {
				/* refer to CA training pinmux array */
				addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_RRRATE_CTL);
				ucstatus |= ucDram_Register_Read(addr, &u4value);
				mcSET_FIELD(&u4value, uiLPDDR_PHY_Mapping_POP_CHA[0],
					    MASK_RRRATE_CTL_BIT0_SEL,
							POS_RRRATE_CTL_BIT0_SEL);
				mcSET_FIELD(&u4value, uiLPDDR_PHY_Mapping_POP_CHA[1],
					    MASK_RRRATE_CTL_BIT1_SEL,
							POS_RRRATE_CTL_BIT1_SEL);
				mcSET_FIELD(&u4value, uiLPDDR_PHY_Mapping_POP_CHA[2],
					    MASK_RRRATE_CTL_BIT2_SEL,
							POS_RRRATE_CTL_BIT2_SEL);
				mcSET_FIELD(&u4value, uiLPDDR_PHY_Mapping_POP_CHA[3],
					    MASK_RRRATE_CTL_BIT3_SEL,
							POS_RRRATE_CTL_BIT3_SEL);
				ucstatus |= ucDram_Register_Write(addr, u4value);

				addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_MRR_CTL);
				ucstatus |= ucDram_Register_Read(addr, &u4value);
				mcSET_FIELD(&u4value, uiLPDDR_PHY_Mapping_POP_CHA[4],
					    MASK_MRR_CTL_BIT4_SEL,
							POS_MRR_CTL_BIT4_SEL);
				mcSET_FIELD(&u4value, uiLPDDR_PHY_Mapping_POP_CHA[5],
					    MASK_MRR_CTL_BIT5_SEL,
							POS_MRR_CTL_BIT5_SEL);
				mcSET_FIELD(&u4value, uiLPDDR_PHY_Mapping_POP_CHA[6],
					    MASK_MRR_CTL_BIT6_SEL,
							POS_MRR_CTL_BIT6_SEL);
				mcSET_FIELD(&u4value, uiLPDDR_PHY_Mapping_POP_CHA[7],
					    MASK_MRR_CTL_BIT7_SEL,
							POS_MRR_CTL_BIT7_SEL);
				ucstatus |= ucDram_Register_Write(addr, u4value);
			} else { /* LPDDR2 */
				/* TBD */
			}
		} else {
			if (p->dram_type == TYPE_LPDDR3) {
				/* refer to CA training pinmux array */
				addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_RRRATE_CTL);
				ucstatus |= ucDram_Register_Read(addr, &u4value);
				mcSET_FIELD(&u4value, uiLPDDR_PHY_Mapping_POP_CHB[0],
					    MASK_RRRATE_CTL_BIT0_SEL,
							POS_RRRATE_CTL_BIT0_SEL);
				mcSET_FIELD(&u4value, uiLPDDR_PHY_Mapping_POP_CHB[1],
					    MASK_RRRATE_CTL_BIT1_SEL,
							POS_RRRATE_CTL_BIT1_SEL);
				mcSET_FIELD(&u4value, uiLPDDR_PHY_Mapping_POP_CHB[2],
					    MASK_RRRATE_CTL_BIT2_SEL,
							POS_RRRATE_CTL_BIT2_SEL);
				mcSET_FIELD(&u4value, uiLPDDR_PHY_Mapping_POP_CHB[3],
					    MASK_RRRATE_CTL_BIT3_SEL,
							POS_RRRATE_CTL_BIT3_SEL);
				ucstatus |= ucDram_Register_Write(addr, u4value);

				addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_MRR_CTL);
				ucstatus |= ucDram_Register_Read(addr, &u4value);
				mcSET_FIELD(&u4value, uiLPDDR_PHY_Mapping_POP_CHB[4],
					    MASK_MRR_CTL_BIT4_SEL,
							POS_MRR_CTL_BIT4_SEL);
				mcSET_FIELD(&u4value, uiLPDDR_PHY_Mapping_POP_CHB[5],
					    MASK_MRR_CTL_BIT5_SEL,
							POS_MRR_CTL_BIT5_SEL);
				mcSET_FIELD(&u4value, uiLPDDR_PHY_Mapping_POP_CHB[6],
					    MASK_MRR_CTL_BIT6_SEL,
							POS_MRR_CTL_BIT6_SEL);
				mcSET_FIELD(&u4value, uiLPDDR_PHY_Mapping_POP_CHB[7],
					    MASK_MRR_CTL_BIT7_SEL,
							POS_MRR_CTL_BIT7_SEL);
				ucstatus |= ucDram_Register_Write(addr, u4value);
			} else { /* LPDDR2 */
				/* TBD */
			}
		}

		addr = mcSET_DRAMC_REG_ADDR(p, 0x088);
		ucstatus |= ucDram_Register_Write(addr, MRR_num);
		addr = mcSET_DRAMC_REG_ADDR(p, 0x1e4);
		ucstatus |= ucDram_Register_Write(addr, 0x00000002);
		udelay(1);
		addr = mcSET_DRAMC_REG_ADDR(p, 0x1e4);
		ucstatus |= ucDram_Register_Write(addr, 0x00000000);
		addr = mcSET_DRAMC_REG_ADDR(p, 0x03B8);
		ucstatus |= ucDram_Register_Read(addr, &u4value);
		MRR_value = (u4value >> 20) & 0xFF;
	}

	return MRR_value;
}

EMI_SETTINGS emi_setting_default_lpddr3 = {

	/* default */
	0x0,			/* sub_version */
	0x0003,			/* TYPE */
	0,			/* EMMC ID/FW ID checking length */
	0,			/* FW length */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
		0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},	/* NAND_EMMC_ID */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},	/* FW_ID */
	0x20102016,		/* EMI_CONA_VAL */
	0x00000000,		/* EMI_CONH_VAL */
	0x77FD474B,		/* DRAMC_ACTIM_VAL */
	0x11000000,		/* DRAMC_GDDR3CTL1_VAL */
	0x00048403,		/* DRAMC_CONF1_VAL */
	0x000053B1,		/* DRAMC_DDR2CTL_VAL */
	0xBFC40401,		/* DRAMC_TEST2_3_VAL */
	0x0000006C,		/* DRAMC_CONF2_VAL */
	0xD1646142,		/* DRAMC_PD_CTRL_VAL */
	0x91001E59,		/* DRAMC_ACTIM1_VAL*/
	0x17000000,		/* DRAMC_MISCTL0_VAL*/
	0x000004F1,		/* DRAMC_ACTIM05T_VAL*/
	0x002145C1,		/* DRAM_CRKCFG_VAL*/
	0x2701110D,		/* DRAMC_TEST2_4_VAL*/
	{0x40000000, 0, 0, 0},	/* DRAM RANK SIZE */
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* reserved 10 */
	{   {
			0x00830001,		/* LPDDR3_MODE_REG1 */
			0x001C0002,		/* LPDDR3_MODE_REG2 */
			0x00020003,		/* LPDDR3_MODE_REG3 */
			0x00000006,		/* LPDDR3_MODE_REG5 */
			0x00FF000A,		/* LPDDR3_MODE_REG10 */
			0x0000003F
		}
	},		/* LPDDR3_MODE_REG63 */
};

static int mt_get_dram_type_for_dis(void)
{
	int i;
	int type = 2;

	type = (emi_settings[0].type & 0xF);
	for (i = 0 ; i < num_of_emi_records; i++) {
		if (type != (emi_settings[i].type & 0xF)) {
			printk(BIOS_DEBUG, "It's not allow to combine ");
			printk(BIOS_DEBUG, "two type dram when combo ");
			printk(BIOS_DEBUG, "discrete dram enable\n");
			ASSERT(0);
			break;
		}
	}
	return type;
}

u8 R0_gw_coarse_valid[2], R1_gw_coarse_valid[2];
int mdl_number = -1;
u8 MDL_first_search_done = 0;

static int mdl_get_rank_number(void)
{
	if ((R0_gw_coarse_valid[0] != 0) &&
	    (R0_gw_coarse_valid[1] != 0) &&
	    (R1_gw_coarse_valid[0] != 0) &&
	    (R1_gw_coarse_valid[1] != 0)) {
		/* rank 0 and rank 1 calibration pass */
		return 2;
	} else if (
	    (R0_gw_coarse_valid[0] != 0) &&
	    (R0_gw_coarse_valid[1] != 0) &&
	    (R1_gw_coarse_valid[0] == 0) &&
	    (R1_gw_coarse_valid[1] == 0)) {
		/* rank 0 calibration pass, rank 1 calibration fail */
		return 1;
	} else
		return -1;  /* error */
}

static int mt_get_mdl_number(void)
{
	unsigned int dram_rank_nr;
	unsigned int dram_type;

	/* DISCRETE_DDR_MDL_SEARCH defined */
	DRAMC_CTX_T *p;
	EMI_SETTINGS *emi_set;
	uintptr_t addr;

	if (MDL_first_search_done == 0) {
		/* first time search */
		dram_type = mt_get_dram_type_for_dis();

		if (TYPE_LPDDR3 == dram_type) {
			printk(BIOS_DEBUG, "[EMI] LPDDR3 MDL search init\r\n");

		} else if (TYPE_PCDDR3 == dram_type) {
			printk(BIOS_DEBUG, "this chip no support PCDDR3 !!!\n");
			ASSERT(0);
		}

		emi_set = &emi_setting_default_lpddr3;
		p = psCurrDramCtx =  &DramCtx_LPDDR3;

		Init_DRAM(psCurrDramCtx, emi_set);

		if (num_of_emi_records >= 2) {
			int val = emi_settings[1].EMI_CONA_VAL;

			addr = EMI_APB_BASE+0x00000000;
			*(volatile unsigned *)(addr) = val;
		} else {
			int val = emi_settings[0].EMI_CONA_VAL;

			addr = EMI_APB_BASE+0x00000000;
			*(volatile unsigned *)(addr) = val;
		}

		printk(BIOS_DEBUG, "[EMI] MDL search dram init\r\n");

		p->channel = CHANNEL_A;
		DualRankDramcRxdqsGatingCal((DRAMC_CTX_T *)p);
		p->channel = CHANNEL_B;
		DualRankDramcRxdqsGatingCal((DRAMC_CTX_T *)p);

		dram_rank_nr = mdl_get_rank_number();

		printk(BIOS_DEBUG, "CH_A Rank0 value %d, ",
		       R0_gw_coarse_valid[0]);
		printk(BIOS_DEBUG, "CH_B Rank0 value %d\n",
		       R0_gw_coarse_valid[1]);
		printk(BIOS_DEBUG, "CH_A Rank1 value %d, ",
		       R1_gw_coarse_valid[0]);
		printk(BIOS_DEBUG, "CH_B Rank1 value %d\n",
		       R1_gw_coarse_valid[1]);

		/* read back value is not "0" means rank is there */
		printk(BIOS_DEBUG, "[EMI] get_rank_number:%d\n",
		       dram_rank_nr);

		MDL_first_search_done = 1;

		if ((dram_rank_nr == 2) && (num_of_emi_records >= 2))
			mdl_number = 1;  /* record search result */
		else
			mdl_number = 0;
	} else {
		if (mdl_number ==  -1)
			mdl_number = 0;
	}

	return mdl_number;
}

int get_dram_rank_nr(void)
{
	int emi_cona;
	int index;

	index = mt_get_mdl_number();
	if (index < 0 || index >=  num_of_emi_records) {
		return -1;
	}

	emi_cona = emi_settings[index].EMI_CONA_VAL;

	if ((emi_cona & (1 << 17)) != 0 || /* for channel 0 */
	    (emi_cona & (1 << 16)) != 0)  /* for channel 1 */
		return 2;
	else
		return 1;

}

int mt_get_dram_type(void)
{
	int n;

	/* if combo discrete is enabled, */
	/* the dram_type is LPDDR2 or LPDDR4, */
	/* depend on the emi_setting list */
	if (1 == enable_combo_dis)
		return mt_get_dram_type_for_dis();

	n = mt_get_mdl_number();

	if (n < 0  || n >= num_of_emi_records) {
		return 0; /* invalid */
	}

	return (emi_settings[n].type & 0xF);
}

void get_dram_rank_size(unsigned int dram_rank_size[])
{
	int index, rank_nr, i;

	index = mt_get_mdl_number();

	if (index < 0 || index >= num_of_emi_records) {
		return;
	}

	rank_nr = get_dram_rank_nr();

	for (i = 0; i < rank_nr; i++) {
		dram_rank_size[i] = emi_settings[index].DRAM_RANK_SIZE[i];

		printk(BIOS_DEBUG, "%d:dram_rank_size:%x\n",
		       i, dram_rank_size[i]);
	}

	return;
}

int mt_get_dram_freq_setting(void)
{
	unsigned int freq;

	freq = 896;

	printk(BIOS_DEBUG, "mt_get_dram_freq_setting = %d\n", freq);
	return freq;
}

void mt_set_emi(void)
{
	u32 ii, u4err_value;
	DRAMC_CTX_T *p;

	if (VcHV_VmHV) {
		DRAM_Vcore_adjust(1);
		DRAM_Vmem_adjust(1);
		printk(BIOS_DEBUG, "[EMI] ");
		printk(BIOS_DEBUG, "LPDDR3 HV : Vcore = 1.25V Vmem = 1.30V \r\n");

	} else if (VcLV_VmLV) {
		DRAM_Vcore_adjust(3);
		DRAM_Vmem_adjust(3);
		printk(BIOS_DEBUG, "[EMI] ");
		printk(BIOS_DEBUG, "LPDDR3 LV : Vcore = 1.025V Vmem = 1.138V \r\n");

	} else if (VcLV_VmNV) {
		DRAM_Vcore_adjust(3);
		DRAM_Vmem_adjust(2);
		printk(BIOS_DEBUG, "[EMI] ");
		printk(BIOS_DEBUG, "LPDDR3 LV : Vcore = 1.025V Vmem = 1.225V \r\n");

	} else {

		DRAM_Vcore_adjust(2);
		DRAM_Vmem_adjust(2);
		printk(BIOS_DEBUG, "[EMI] ");
		printk(BIOS_DEBUG, "LPDDR3 NV : Vcore = 1.125V Vmem = 1.225V \r\n");
	}

	int index = 0;
	EMI_SETTINGS *emi_set;

	index = mt_get_mdl_number();
	printk(BIOS_DEBUG, "[Check]mt_get_mdl_number %#x\n", index);

	if (index < 0 || index >=  num_of_emi_records) {
		printk(BIOS_DEBUG, "[EMI] setting failed %#x\r\n", index);
		ASSERT(0);
	}

	printk(BIOS_DEBUG, "[EMI] MDL number = %d\r\n", index);
	emi_set = &emi_settings[index];

	if ((emi_set->type & 0xF) == TYPE_LPDDR3) {
		p = psCurrDramCtx = &DramCtx_LPDDR3;
		/* !defined(DUAL_FREQ_K) */
		p->frequency = mt_get_dram_freq_setting();
		Init_DRAM(p, emi_set);
	} else if ((emi_set->type & 0xF) == TYPE_PCDDR3) {
		p = psCurrDramCtx = &DramCtx_PCDDR3;
		/* !defined(DUAL_FREQ_K) */
		p->frequency = mt_get_dram_freq_setting();

		Init_DRAM(p, emi_set);
	} else {
		printk(BIOS_DEBUG, "The DRAM type is not supported");
		ASSERT(0);
	}

	do_calib(p, emi_set, 0);

	{
		int i;
		unsigned int dram_rank_size[4] = {0, 0, 0, 0};
		u64 total_dram_size = 0;

		get_dram_rank_size(dram_rank_size);
		for (i = 0; i < 4; i++) {
			total_dram_size += dram_rank_size[i];
		}
		if (total_dram_size > 0xC0000000ULL) {
			printk(BIOS_DEBUG, "[Enable 4GB Support] ");
			printk(BIOS_DEBUG, "Total_dram_size = 0x%llx\n",
			       total_dram_size);
			*(volatile unsigned int *)(0x10003208) |= 1 << 15;
			*(volatile unsigned int *)(0x10001f00) |= 1 << 13;
		}
	}

	/* Single rank test. */
	for (ii = 0; ii < 2; ii++) {
		u4err_value = DramcEngine2((DRAMC_CTX_T *)p,
					   TE_OP_WRITE_READ_CHECK, 0x55000000,
					0xaa010000, 2, 0, 0, (u8)ii);

		mcSHOW_DBG_MSG("[A60808_MISC_CMD_TA2_XTALK-%d] ", ii);
		mcSHOW_DBG_MSG("err_value=0x%8x\n", u4err_value);
	}

	printk(BIOS_DEBUG, "[EMI]DRAM Vendor_ID : %#x\n", DRAM_MRR(0x5));
}
