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

#ifndef SOC_MEDIATEK_MT8173_EMI_HW_H
#define SOC_MEDIATEK_MT8173_EMI_HW_H

enum {
	/* Address mapping */
	EMI_CONA      = (EMI_BASE + 0x000),
	/* Data transfer overhead for specific read data size */
	EMI_CONB      = (EMI_BASE + 0x008),
	/* Data transfer overhead for specific read data size */
	EMI_CONC      = (EMI_BASE + 0x010),
	/* Data transfer overhead for specific write data size */
	EMI_COND      = (EMI_BASE + 0x018),
	/* Data transfer overhead for specific write data size */
	EMI_CONE      = (EMI_BASE + 0x020),
	/* Address scramble setting */
	EMI_CONF      = (EMI_BASE + 0x028),
	/* Data transfer overhead for non-32bytes alignment read data size */
	EMI_CONG      = (EMI_BASE + 0x030),
	/* Data transfer overhead for non-32bytes alignment write data size */
	EMI_CONH      = (EMI_BASE + 0x038),
	/* MISC */
	EMI_CONM      = (EMI_BASE + 0x060),
	/* Test mode B. */
	EMI_TESTB     = (EMI_BASE + 0x0E8),
	/* Test mode C. */
	EMI_TESTC     = (EMI_BASE + 0x0F0),
	/* Test mode D. */
	EMI_TESTD     = (EMI_BASE + 0x0F8),
	/* EMI bandwidth filter control APMCU */
	EMI_ARBA      = (EMI_BASE + 0x100),
	/* EMI bandwidth filter control ARM9 */
	EMI_ARBC      = (EMI_BASE + 0x110),
	/* EMI bandwidth filter control MDMCU */
	EMI_ARBD      = (EMI_BASE + 0x118),
	/* EMI bandwidth filter control Modem Hardware */
	EMI_ARBE      = (EMI_BASE + 0x120),
	/* EMI bandwidth filter control MM 0 */
	EMI_ARBF      = (EMI_BASE + 0x128),
	/* EMI bandwidth filter control MM 1 */
	EMI_ARBG      = (EMI_BASE + 0x130),
	/* Filter priority encode */
	EMI_ARBI      = (EMI_BASE + 0x140),
	/* Filter priority encode for MD requirement */
	EMI_ARBI_2ND  = (EMI_BASE + 0x144),
	/* Turn around command number */
	EMI_ARBJ      = (EMI_BASE + 0x148),
	/* Turn around command number for MD requirement */
	EMI_ARBJ_2ND  = (EMI_BASE + 0x14C),
	/* Page miss control */
	EMI_ARBK      = (EMI_BASE + 0x150),
	/* Page miss control */
	EMI_ARBK_2ND  = (EMI_BASE + 0x154),
	/* EMI slave control registers */
	EMI_SLCT      = (EMI_BASE + 0x158),
	/* Memory protection unit control registers A */
	EMI_MPUA      = (EMI_BASE + 0x160),
	/* Memory protection unit control registers B */
	EMI_MPUB      = (EMI_BASE + 0x168),
	/* Memory protection unit control registers C */
	EMI_MPUC      = (EMI_BASE + 0x170),
	/* Memory protection unit control registers D */
	EMI_MPUD      = (EMI_BASE + 0x178),
	/* Memory protection unit control registers E */
	EMI_MPUE      = (EMI_BASE + 0x180),
	/* Memory protection unit control registers F */
	EMI_MPUF      = (EMI_BASE + 0x188),
	/* Memory protection unit control registers G */
	EMI_MPUG      = (EMI_BASE + 0x190),
	/* Memory protection unit control registers H */
	EMI_MPUH      = (EMI_BASE + 0x198),
	/* Memory protection unit control registers I */
	EMI_MPUI      = (EMI_BASE + 0x1A0),
	/* Memory protection unit control registers J */
	EMI_MPUJ      = (EMI_BASE + 0x1A8),
	/* Memory protection unit control registers K */
	EMI_MPUK      = (EMI_BASE + 0x1B0),
	/* Memory protection unit control registers L */
	EMI_MPUL      = (EMI_BASE + 0x1B8),
	/* Memory protection unit control registers M */
	EMI_MPUM      = (EMI_BASE + 0x1C0),
	/* Memory protection unit control registers N */
	EMI_MPUN      = (EMI_BASE + 0x1C8),
	/* Memory protection unit control registers O */
	EMI_MPUO      = (EMI_BASE + 0x1D0),
	/* Memory protection unit control registers P */
	EMI_MPUP      = (EMI_BASE + 0x1D8),
	/* Memory protection unit control registers Q */
	EMI_MPUQ      = (EMI_BASE + 0x1E0),
	/* Memory protection unit control registers R */
	EMI_MPUR      = (EMI_BASE + 0x1E8),
	/* Memory protection unit control registers S */
	EMI_MPUS      = (EMI_BASE + 0x1F0),
	/* Memory protection unit control registers T */
	EMI_MPUT      = (EMI_BASE + 0x1F8),
	/* Memory protection unit control registers U */
	EMI_MPUU      = (EMI_BASE + 0x200),
	/* Memory protection unit control registers Y */
	EMI_MPUY      = (EMI_BASE + 0x220),
	/* EMI bus monitor control registers */
	EMI_BMEN      = (EMI_BASE + 0x400),
	/* EMI bus cycle counters */
	EMI_BCNT      = (EMI_BASE + 0x408),
	/* EMI total transaction counters */
	EMI_TACT      = (EMI_BASE + 0x410),
	/* EMI 1st transaction counters */
	EMI_TSCT      = (EMI_BASE + 0x418),
	/* EMI total word counters */
	EMI_WACT      = (EMI_BASE + 0x420),
	/* EMI 1st word counters */
	EMI_WSCT      = (EMI_BASE + 0x428),
	/* EMI bandwidth word counters */
	EMI_BACT      = (EMI_BASE + 0x430),
	/* EMI overhead word counters */
	EMI_BSCT      = (EMI_BASE + 0x438),
	/* EMI master selection for 2nd and 3rd counters */
	EMI_MSEL      = (EMI_BASE + 0x440),
	/* EMI 2nd transaction counters of selected masters */
	EMI_TSCT2     = (EMI_BASE + 0x448),
	/* EMI 3rd transaction counters of selected masters */
	EMI_TSCT3     = (EMI_BASE + 0x450),
	/* EMI 2nd double-word counter of selected masters. */
	EMI_WSCT2     = (EMI_BASE + 0x458),
	/* EMI 3rd double-word counter of selected masters. */
	EMI_WSCT3     = (EMI_BASE + 0x460),
	/* EMI 4th double-word counter of selected masters. */
	EMI_WSCT4     = (EMI_BASE + 0x464),
	/* EMI master selection for 4th and 5th counters */
	EMI_MSEL2     = (EMI_BASE + 0x468),
	/* EMI master selection for 6th and 7th counters */
	EMI_MSEL3     = (EMI_BASE + 0x470),
	/* EMI master selection for 8th and 9th counters */
	EMI_MSEL4     = (EMI_BASE + 0x478),
	/* EMI master selection for 10th and 11th counters */
	EMI_MSEL5     = (EMI_BASE + 0x480),
	/* EMI master selection for 12th and 13th counters */
	EMI_MSEL6     = (EMI_BASE + 0x488),
	/* EMI master selection for 14th and 15th counters */
	EMI_MSEL7     = (EMI_BASE + 0x490),
	/* EMI master selection for 16th and 17th counters */
	EMI_MSEL8     = (EMI_BASE + 0x498),
	/* EMI master selection for 18th and 19th counters */
	EMI_MSEL9     = (EMI_BASE + 0x4A0),
	/* EMI master selection for 20th and 21th counters */
	EMI_MSEL10    = (EMI_BASE + 0x4A8),
	/* EMI ID selection for 1- 4 transaction type counters */
	EMI_BMID0     = (EMI_BASE + 0x4B0),
	/* EMI ID selection for 5- 8 transaction type counters */
	EMI_BMID1     = (EMI_BASE + 0x4B8),
	/* EMI ID selection for 9- 12 transaction type counters */
	EMI_BMID2     = (EMI_BASE + 0x4C0),
	/* EMI ID selection for 13- 16 transaction type counters */
	EMI_BMID3     = (EMI_BASE + 0x4C8),
	/* EMI ID selection for 17- 20 transaction type counters */
	EMI_BMID4     = (EMI_BASE + 0x4D0),
	/* EMI ID selection for 21  transaction type counters */
	EMI_BMID5     = (EMI_BASE + 0x4D8),
	/* High priority Filter */
	EMI_BMEN1     = (EMI_BASE + 0x4E0),
	/* EMI ID selection enabling */
	EMI_BMEN2     = (EMI_BASE + 0x4E8),
	/* EMI 1st transaction type counter of selected masters */
	EMI_TTYPE1    = (EMI_BASE + 0x500),
	/* EMI 2nd transaction type counter of selected masters */
	EMI_TTYPE2    = (EMI_BASE + 0x508),
	/* EMI 3rd transaction type counter of selected masters */
	EMI_TTYPE3    = (EMI_BASE + 0x510),
	/* EMI 4th transaction type counter of selected masters */
	EMI_TTYPE4    = (EMI_BASE + 0x518),
	/* EMI 5th transaction type counter of selected masters */
	EMI_TTYPE5    = (EMI_BASE + 0x520),
	/* EMI 6th transaction type counter of selected masters */
	EMI_TTYPE6    = (EMI_BASE + 0x528),
	/* EMI 7th transaction type counter of selected masters */
	EMI_TTYPE7    = (EMI_BASE + 0x530),
	/* EMI 8th transaction type counter of selected masters */
	EMI_TTYPE8    = (EMI_BASE + 0x538),
	/* EMI 9th transaction type counter of selected masters */
	EMI_TTYPE9    = (EMI_BASE + 0x540),
	/* EMI 10th transaction type counter of selected masters */
	EMI_TTYPE10   = (EMI_BASE + 0x548),
	/* EMI 11th transaction type counter of selected masters */
	EMI_TTYPE11   = (EMI_BASE + 0x550),
	/* EMI 12th transaction type counter of selected masters */
	EMI_TTYPE12   = (EMI_BASE + 0x558),
	/* EMI 13th transaction type counter of selected masters */
	EMI_TTYPE13   = (EMI_BASE + 0x560),
	/* EMI 14th transaction type counter of selected masters */
	EMI_TTYPE14   = (EMI_BASE + 0x568),
	/* EMI 15th transaction type counter of selected masters */
	EMI_TTYPE15   = (EMI_BASE + 0x570),
	/* EMI 16th transaction type counter of selected masters */
	EMI_TTYPE16   = (EMI_BASE + 0x578),
	/* EMI 17th transaction type counter of selected masters */
	EMI_TTYPE17   = (EMI_BASE + 0x580),
	/* EMI 18th transaction type counter of selected masters */
	EMI_TTYPE18   = (EMI_BASE + 0x588),
	/* EMI 19th transaction type counter of selected masters */
	EMI_TTYPE19   = (EMI_BASE + 0x590),
	/* EMI 20th transaction type counter of selected masters */
	EMI_TTYPE20   = (EMI_BASE + 0x598),
	/* EMI 21th transaction type counter of selected masters */
	EMI_TTYPE21   = (EMI_BASE + 0x5A0)
};


/* MT8173 EMI default values */
enum {
	EMI_CONA_DEFAULT   = 0x000003F2,
	EMI_CONB_DEFAULT   = 0x00000000,
	EMI_CONC_DEFAULT   = 0x00000000,
	EMI_COND_DEFAULT   = 0x00000000,
	EMI_CONE_DEFAULT   = 0x00000000,
	EMI_CONF_DEFAULT   = 0x00000000,
	EMI_CONG_DEFAULT   = 0x00000000,
	EMI_CONH_DEFAULT   = 0x00000000,
	EMI_CONM_DEFAULT   = 0x00000000,
	EMI_TESTA_DEFAULT  = 0x43334312,
	EMI_TESTB_DEFAULT  = 0x00000000,
	EMI_TESTC_DEFAULT  = 0x00000000,
	EMI_TESTD_DEFAULT  = 0x00000000,
	EMI_ARBA_DEFAULT   = 0x00004000,
	EMI_ARBC_DEFAULT   = 0x00004800,
	EMI_ARBD_DEFAULT   = 0x00004000,
	EMI_ARBE_DEFAULT   = 0x00004000,
	EMI_ARBF_DEFAULT   = 0x00004000,
	EMI_ARBG_DEFAULT   = 0x00004000,
	EMI_ARBI_DEFAULT   = 0x00000000,
	EMI_ARBI_2ND_DEFAULT  = 0x00000000,
	EMI_ARBJ_DEFAULT      = 0x66684848,
	EMI_ARBJ_2ND_DEFAULT  = 0x66684848,
	EMI_ARBK_DEFAULT      = 0x00000C7C,
	EMI_ARBK_2ND_DEFAULT  = 0x00000C7C,
	EMI_SLCT_DEFAULT   = 0x00000000,
	EMI_MPUA_DEFAULT   = 0x00000000,
	EMI_MPUB_DEFAULT   = 0x00000000,
	EMI_MPUC_DEFAULT   = 0x00000000,
	EMI_MPUD_DEFAULT   = 0x00000000,
	EMI_MPUE_DEFAULT   = 0x00000000,
	EMI_MPUF_DEFAULT   = 0x00000000,
	EMI_MPUG_DEFAULT   = 0x00000000,
	EMI_MPUH_DEFAULT   = 0x00000000,
	EMI_MPUI_DEFAULT   = 0x00000000,
	EMI_MPUJ_DEFAULT   = 0x00000000,
	EMI_MPUK_DEFAULT   = 0x00000000,
	EMI_MPUL_DEFAULT   = 0x00000000,
	EMI_MPUM_DEFAULT   = 0x00000000,
	EMI_MPUN_DEFAULT   = 0x00000000,
	EMI_MPUO_DEFAULT   = 0x00000000,
	EMI_MPUP_DEFAULT   = 0x00000000,
	EMI_MPUQ_DEFAULT   = 0x00000000,
	EMI_MPUR_DEFAULT   = 0x00000000,
	EMI_MPUS_DEFAULT   = 0x00000000,
	EMI_MPUT_DEFAULT   = 0x00000000,
	EMI_MPUU_DEFAULT   = 0x00000000,
	EMI_MPUY_DEFAULT   = 0x00000000,
	EMI_BMEN_DEFAULT   = 0x00000000,
	EMI_BCNT_DEFAULT   = 0x00000000,
	EMI_TACT_DEFAULT   = 0x00000000,
	EMI_TSCT_DEFAULT   = 0x00000000,
	EMI_WACT_DEFAULT   = 0x00000000,
	EMI_WSCT_DEFAULT   = 0x00000000,
	EMI_BACT_DEFAULT   = 0x00000000,
	EMI_BSCT_DEFAULT   = 0x00000000,
	EMI_MSEL_DEFAULT   = 0x00000000,
	EMI_TSCT2_DEFAULT  = 0x00000000,
	EMI_TSCT3_DEFAULT  = 0x00000000,
	EMI_WSCT2_DEFAULT  = 0x00000000,
	EMI_WSCT3_DEFAULT  = 0x00000000,
	EMI_WSCT4_DEFAULT  = 0x00000000,
	EMI_MSEL2_DEFAULT  = 0x00000000,
	EMI_MSEL3_DEFAULT  = 0x00000000,
	EMI_MSEL4_DEFAULT  = 0x00000000,
	EMI_MSEL5_DEFAULT  = 0x00000000,
	EMI_MSEL6_DEFAULT  = 0x00000000,
	EMI_MSEL7_DEFAULT  = 0x00000000,
	EMI_MSEL8_DEFAULT  = 0x00000000,
	EMI_MSEL9_DEFAULT  = 0x00000000,
	EMI_MSEL10_DEFAULT = 0x00000000,
	EMI_BMID0_DEFAULT  = 0x00000000,
	EMI_BMID1_DEFAULT  = 0x00000000,
	EMI_BMID2_DEFAULT  = 0x00000000,
	EMI_BMID3_DEFAULT  = 0x00000000,
	EMI_BMID4_DEFAULT  = 0x00000000,
	EMI_BMID5_DEFAULT  = 0x00000000,
	EMI_BMEN1_DEFAULT  = 0x00000000,
	EMI_BMEN2_DEFAULT  = 0x00000000,
	EMI_BMID6_DEFAULT  = 0x00000000,
	EMI_BMID7_DEFAULT  = 0x00000000,
	EMI_TTYPE1_DEFAULT   = 0x00000000,
	EMI_TTYPE2_DEFAULT   = 0x00000000,
	EMI_TTYPE3_DEFAULT   = 0x00000000,
	EMI_TTYPE4_DEFAULT   = 0x00000000,
	EMI_TTYPE5_DEFAULT   = 0x00000000,
	EMI_TTYPE6_DEFAULT   = 0x00000000,
	EMI_TTYPE7_DEFAULT   = 0x00000000,
	EMI_TTYPE8_DEFAULT   = 0x00000000,
	EMI_TTYPE9_DEFAULT   = 0x00000000,
	EMI_TTYPE10_DEFAULT  = 0x00000000,
	EMI_TTYPE11_DEFAULT  = 0x00000000,
	EMI_TTYPE12_DEFAULT  = 0x00000000,
	EMI_TTYPE13_DEFAULT  = 0x00000000,
	EMI_TTYPE14_DEFAULT  = 0x00000000,
	EMI_TTYPE15_DEFAULT  = 0x00000000,
	EMI_TTYPE16_DEFAULT  = 0x00000000,
	EMI_TTYPE17_DEFAULT  = 0x00000000,
	EMI_TTYPE18_DEFAULT  = 0x00000000,
	EMI_TTYPE19_DEFAULT  = 0x00000000,
	EMI_TTYPE20_DEFAULT  = 0x00000000,
	EMI_TTYPE21_DEFAULT  = 0x00000000
};

enum {
	/* DRAM AC TIMING SETTING 0 */
	DRAMC_ACTIM0 = 0x0,
	/* DRAM CONFIGURATION 1 */
	DRAMC_CONF1 = 0x4,
	/* DRAM CONFIGURATION 2 */
	DRAMC_CONF2 = 0x8,
	/* DRAM PAD CONTROL 1 */
	DRAMC_PADCTL1 = 0xc,
	/* DRAM PAD CONTROL 2 */
	DRAMC_PADCTL2 = 0x10,
	/* DRAM PAD CONTROL 3 */
	DRAMC_PADCTL3 = 0x14,
	/* RANK0 DQS INPUT DELAY CHAIN SETTING */
	DRAMC_R0DELDLY = 0x18,
	/* RANK1 DQS INPUT DELAY CHAIN SETTING */
	DRAMC_R1DELDLY = 0x1c,
	/* RANK0 DQS INPUT DELAY CHAIN OFFSET SETTING */
	DRAMC_R0DIFDLY = 0x20,
	/* RANK1 DQS INPUT DELAY CHAIN OFFSET SETTING */
	DRAMC_R1DIFDLY = 0x24,
	/* DLL CONFIGURATION */
	DRAMC_DLLCONF = 0x28,
	/* TEST MODE CONFIGURATION 1 */
	DRAMC_TESTMODE = 0x2c,
	/* TEST AGENT 2 CONFIGURATION 1 */
	DRAMC_TEST2_1	= 0x3c,
	/* TEST AGENT 2 CONFIGURATION 2 */
	DRAMC_TEST2_2	= 0x40,
	/* TEST AGENT 2 CONFIGURATION 3 */
	DRAMC_TEST2_3	= 0x44,
	/* TEST AGENT 2 CONFIGURATION 4 */
	DRAMC_TEST2_4	= 0x48,
	/* DDR2 CONTROL REGISTER */
	DRAMC_DDR2CTL	= 0x7c,
	/* MRS value setting */
	DRAMC_MRS	= 0x88,
	/* Clock 1 output delay CONTROL */
	DRAMC_CLK1DELAY = 0x8c,
	/* IO CONTROL */
	DRAMC_IOCTL	= 0x90,
	/* RANK0 DQS INPUT RANGE FINE TUNER */
	DRAMC_R0DQSIEN = 0x94,
	/* RANK1 DQS INPUT RANGE FINE TUNER */
	DRAMC_R1DQSIEN = 0x98,
	/* PAD DRIVING CONTROL SETTING 00 */
	DRAMC_DRVCTL00 = 0xb4,
	/* PAD DRIVING CONTROL SETTING 0 */
	DRAMC_DRVCTL0 = 0xb8,
	/* PAD DRIVING CONTROL SETTING 1 */
	DRAMC_DRVCTL1	= 0xbc,
	/* DLL SELECTION SETTING */
	DRAMC_DLLSEL = 0xc0,
	/* IO OUTPUT DUTY CONTROL 0 */
	DRAMC_TDSEL0 = 0xcc,
	/* IO OUTPUT DUTY CONTROL 1 */
	DRAMC_TDSEL1 = 0xd0,
	/* MEMORY CLOCK DELAY CHAIN SETTING */
	DRAMC_MCKDLY = 0xd8,
	/* DQS INPUT RANGE CONTROL 0 */
	DRAMC_DQSCTL0	= 0xdc,
	/* DQS INPUT RANGE CONTROL 1 */
	DRAMC_DQSCTL1	= 0xe0,
	/* PAD CONTROL 1 */
	DRAMC_PADCTL4	= 0xe4,
	/* PAD CONTROL 2 */
	DRAMC_PADCTL5	= 0xe8,
	/* PAD CONTROL 3 */
	DRAMC_PADCTL6	= 0xec,
	/* DDR PHY CONTROL 1 */
	DRAMC_PHYCTL1	= 0xf0,
	/* GDDR3 CONTROL 1 */
	DRAMC_GDDR3CTL1	= 0xf4,
	/* PAD CONTROL 4 */
	DRAMC_PADCTL7	= 0xf8,
	/* MISC CONTROL 0 */
	DRAMC_MISCTL0	= 0xfc,
	/* OCD CALIBRATION CONTROL */
	DRAMC_OCDK = 0x100,
	/* LOOP BACK DATA 0 */
	DRAMC_LBWDAT0	= 0x104,
	/* LOOP BACK DATA 1 */
	DRAMC_LBWDAT1	= 0x108,
	/* LOOP BACK DATA 2 */
	DRAMC_LBWDAT2	= 0x10c,
	/* RANK CONFIGURATION */
	DRAMC_RKCFG	= 0x110,
	/* CLOCK PHASE DETECTION SETTING */
	DRAMC_CKPHDET	= 0x114,
	/* INPUT DQS GATING CONTROL */
	DRAMC_DQSGCTL	= 0x124,
	/* DRAM CLOCK ENABLE CONTROL */
	DRAMC_CLKENCTL = 0x130,
	/* DQS gating delay control 1 */
	DRAMC_DQSGCTL1 = 0x140,
	/* DQS gating delay control 2 */
	DRAMC_DQSGCTL2 = 0x144,
	/* ARBITRATION CONTROL 0 */
	DRAMC_ARBCTL0 = 0x168,
	/* Command Delay CTL0 */
	DRAMC_CMDDLY0	= 0x1a8,
	/* Command Delay CTL1 */
	DRAMC_CMDDLY1	= 0x1ac,
	/* Command Delay CTL2 */
	DRAMC_CMDDLY2	= 0x1b0,
	/* Command Delay CTL3 */
	DRAMC_CMDDLY3	= 0x1b4,
	/* Command Delay CTL4 */
	DRAMC_CMDDLY4	= 0x1b8,
	/* Command Delay CTL5 */
	DRAMC_CMDDLY5	= 0x1bc,
	/* DQS CAL CONTROL 0 */
	DRAMC_DQSCAL0	= 0x1c0,
	/* DQS CAL CONTROL 1 */
	DRAMC_DQSCAL1	= 0x1c4,
	/* Monitor parameter */
	DRAMC_DMMonitor	= 0x1d8,
	/* PD mode parameter */
	DRAMC_DRAMC_PD_CTRL = 0x1dc,
	/* LPDDR2 setting */
	DRAMC_LPDDR2 = 0x1e0,
	/* Special command mode */
	DRAMC_SPCMD	= 0x1e4,
	/* DRAM AC TIMING SETTING 1 */
	DRAMC_ACTIM1 = 0x1e8,
	/* PERFORMANCE CONTROL 0 */
	DRAMC_PERFCTL0 = 0x1ec,
	/* AC TIME DERATING CONTROL */
	DRAMC_AC_DERATING = 0x1f0,
	/* REFRESH RATE CONTROL */
	DRAMC_RRRATE_CTL = 0x1f4,
	/* DQ output DELAY1 CHAIN setting */
	DRAMC_DQODLY1	= 0x200,
	/* DQ output DELAY2 CHAIN setting */
	DRAMC_DQODLY2	= 0x204,
	/* DQ output DELAY3 CHAIN setting */
	DRAMC_DQODLY3	= 0x208,
	/* DQ output DELAY4 CHAIN setting */
	DRAMC_DQODLY4	= 0x20c,
	/* DQ input DELAY1 CHAIN setting */
	DRAMC_DQIDLY1	= 0x210,
	/* DQ input DELAY2 CHAIN setting */
	DRAMC_DQIDLY2	= 0x214,
	/* DQ input DELAY3 CHAIN setting */
	DRAMC_DQIDLY3	= 0x218,
	/* DQ input DELAY4 CHAIN setting */
	DRAMC_DQIDLY4	= 0x21c,
	/* DQ input DELAY5 CHAIN setting */
	DRAMC_DQIDLY5	= 0x220,
	/* DQ input DELAY6 CHAIN setting */
	DRAMC_DQIDLY6	= 0x224,
	/* DQ input DELAY7 CHAIN setting */
	DRAMC_DQIDLY7	= 0x228,
	/* DQ input DELAY8 CHAIN setting */
	DRAMC_DQIDLY8	= 0x22c,
	/* R2R_page_hit_counter */
	DRAMC_R2R_page_hit_counter = 0x280,
	/* R2R_page_miss_counter */
	DRAMC_R2R_page_miss_counter = 0x284,
	/* R2R_interbank_counter */
	DRAMC_R2R_interbank_counter = 0x288,
	/* R2W_page_hit_counter */
	DRAMC_R2W_page_hit_counter  = 0x28c,
	/* R2W_page_miss_counter */
	DRAMC_R2W_page_miss_counter = 0x290,
	/* R2W_interbank_counter */
	DRAMC_R2W_interbank_counter = 0x294,
	/* W2R_page_hit_counter */
	DRAMC_W2R_page_hit_counter = 0x298,
	/* W2R_page_miss_counter */
	DRAMC_W2R_page_miss_counter = 0x29c,
	/* W2R_interbank_counter */
	DRAMC_W2R_interbank_counter = 0x2a0,
	/* W2W_page_hit_counter */
	DRAMC_W2W_page_hit_counter = 0x2a4,
	/* W2W_page_miss_counter */
	DRAMC_W2W_page_miss_counter = 0x2a8,
	/* W2W_interbank_counter */
	DRAMC_W2W_interbank_counter = 0x2ac,
	/* dramc_idle_counter */
	DRAMC_dramc_idle_counter = 0x2b0,
	/* freerun_26m_counter */
	DRAMC_freerun_26m_counter = 0x2b4,
	/* refresh_pop_counter */
	DRAMC_refresh_pop_counter = 0x2b8,
	/* Jitter Meter Status */
	DRAMC_JMETER_ST = 0x2bc,
	/* DQ INPUT CALIBRATION per bit 3-0 */
	DRAMC_DQ_CAL_MAX_0 = 0x2c0,
	/* DQ INPUT CALIBRATION per bit 7-4 */
	DRAMC_DQ_CAL_MAX_1 = 0x2c4,
	/* DQ INPUT CALIBRATION per bit 11-8 */
	DRAMC_DQ_CAL_MAX_2 = 0x2c8,
	/* DQ INPUT CALIBRATION per bit 15-12 */
	DRAMC_DQ_CAL_MAX_3 = 0x2cc,
	/* DQ INPUT CALIBRATION per bit 19-16 */
	DRAMC_DQ_CAL_MAX_4 = 0x2d0,
	/* DQ INPUT CALIBRATION per bit 23-20 */
	DRAMC_DQ_CAL_MAX_5 = 0x2d4,
	/* DQ INPUT CALIBRATION per bit 27-34 */
	DRAMC_DQ_CAL_MAX_6 = 0x2d8,
	/* DQ INPUT CALIBRATION per bit 31-28 */
	DRAMC_DQ_CAL_MAX_7 = 0x2dc,
	/* DQS INPUT CALIBRATION per bit 3-0 */
	DRAMC_DQS_CAL_MIN_0 = 0x2e0,
	/* DQS INPUT CALIBRATION per bit 7-4 */
	DRAMC_DQS_CAL_MIN_1 = 0x2e4,
	/* DQS INPUT CALIBRATION per bit 11-8 */
	DRAMC_DQS_CAL_MIN_2 = 0x2e8,
	/* DQS INPUT CALIBRATION per bit 15-12 */
	DRAMC_DQS_CAL_MIN_3 = 0x2ec,
	/* DQS INPUT CALIBRATION per bit 19-16 */
	DRAMC_DQS_CAL_MIN_4 = 0x2f0,
	/* DQS INPUT CALIBRATION per bit 23-20 */
	DRAMC_DQS_CAL_MIN_5 = 0x2f4,
	/* DQS INPUT CALIBRATION per bit 27-34 */
	DRAMC_DQS_CAL_MIN_6 = 0x2f8,
	/* DQS INPUT CALIBRATION per bit 31-28 */
	DRAMC_DQS_CAL_MIN_7 = 0x2fc,
	/* DQS INPUT CALIBRATION per bit 3-0 */
	DRAMC_DQS_CAL_MAX_0 = 0x300,
	/* DQS INPUT CALIBRATION per bit 7-4 */
	DRAMC_DQS_CAL_MAX_1 = 0x304,
	/* DQS INPUT CALIBRATION per bit 11-8 */
	DRAMC_DQS_CAL_MAX_2 = 0x308,
	/* DQS INPUT CALIBRATION per bit 15-12 */
	DRAMC_DQS_CAL_MAX_3 = 0x30c,
	/* DQS INPUT CALIBRATION per bit 19-16 */
	DRAMC_DQS_CAL_MAX_4 = 0x310,
	/* DQS INPUT CALIBRATION per bit 23-20 */
	DRAMC_DQS_CAL_MAX_5 = 0x314,
	/* DQS INPUT CALIBRATION per bit 27-34 */
	DRAMC_DQS_CAL_MAX_6 = 0x318,
	/* DQS INPUT CALIBRATION per bit 31-28 */
	DRAMC_DQS_CAL_MAX_7 = 0x31c,
	/* DQS INPUT CALIBRATION 0 */
	DRAMC_DQICAL0	= 0x350,
	/* DQS INPUT CALIBRATION 1 */
	DRAMC_DQICAL1	= 0x354,
	/* DQS INPUT CALIBRATION 2 */
	DRAMC_DQICAL2	= 0x358,
	/* DQS INPUT CALIBRATION 3 */
	DRAMC_DQICAL3	= 0x35c,
	/* CMP ERROR */
	DRAMC_CMP_ERR	= 0x370,
	/* RANK0 DQS INPUT GATING DELAY VALUE */
	DRAMC_R0DQSIENDLY = 0x374,
	/* RANK1 DQS INPUT GATING DELAY VALUE */
	DRAMC_R1DQSIENDLY = 0x378,
	/* DQS RING COUNTER 0 */
	DRAMC_STBEN0 = 0x38c,
	/* DQS RING COUNTER 1 */
	DRAMC_STBEN1 = 0x390,
	/* DQS RING COUNTER 2 */
	DRAMC_STBEN2 = 0x394,
	/* DQS RING COUNTER 3 */
	DRAMC_STBEN3 = 0x398,
	/* DQS INPUT DELAY SETTING 0 */
	DRAMC_DQSDLY0	= 0x3a0,
	/* SPECIAL COMMAND RESPONSE */
	DRAMC_SPCMDRESP = 0x3b8,
	/* IO RING COUNTER */
	DRAMC_IORGCNT	= 0x3bc,
	/* DQS GATING WINODW COUNTER 0 */
	DRAMC_DQSGNWCNT0 = 0x3c0,
	/* DQS GATING WINODW COUNTER 1 */
	DRAMC_DQSGNWCNT1 = 0x3c4,
	/* DQS GATING WINODW COUNTER 2 */
	DRAMC_DQSGNWCNT2 = 0x3c8,
	/* DQS GATING WINODW COUNTER 3 */
	DRAMC_DQSGNWCNT3 = 0x3cc,
	/* DQS GATING WINODW COUNTER 4 */
	DRAMC_DQSGNWCNT4 = 0x3d0,
	/* DQS GATING WINODW COUNTER 5 */
	DRAMC_DQSGNWCNT5 = 0x3d4,
	/* DQS SAMPLE VALUE */
	DRAMC_DQSSAMPLEV = 0x3d8,
	/* DLL STATUS 0 */
	DRAMC_DLLCNT0	= 0x3dc,
	/* CLOCK PHASE DETECTION RESULT */
	DRAMC_CKPHCNT	= 0x3e8,
	/* TEST AGENT STATUS */
	DRAMC_TESTRPT	= 0x3fc,
	/* MEMPLL REGISTER SETTING 0 */
	DRAMC_MEMPLL0	= 0x600,
	/* MEMPLL REGISTER SETTING 1 */
	DRAMC_MEMPLL1	= 0x604,
	/* MEMPLL REGISTER SETTING 2 */
	DRAMC_MEMPLL2	= 0x608,
	/* MEMPLL REGISTER SETTING 3 */
	DRAMC_MEMPLL3	= 0x60c,
	/* MEMPLL REGISTER SETTING 4 */
	DRAMC_MEMPLL4	= 0x610,
	/* MEMPLL REGISTER SETTING 5 */
	DRAMC_MEMPLL5	= 0x614,
	/* MEMPLL REGISTER SETTING 6 */
	DRAMC_MEMPLL6	= 0x618,
	/* MEMPLL REGISTER SETTING 7 */
	DRAMC_MEMPLL7	= 0x61c,
	/* MEMPLL REGISTER SETTING 8 */
	DRAMC_MEMPLL8	= 0x620,
	/* MEMPLL REGISTER SETTING 9 */
	DRAMC_MEMPLL9	= 0x624,
	/* MEMPLL REGISTER SETTING 10 */
	DRAMC_MEMPLL10 = 0x628,
	/* MEMPLL REGISTER SETTING 11 */
	DRAMC_MEMPLL11 = 0x62c,
	/* MEMPLL REGISTER SETTING 12 */
	DRAMC_MEMPLL12 = 0x630,
	/* MEMPLL DIVIDER REGISTER CONTROL */
	DRAMC_MEMPLL_DIVIDER = 0x640,
	/* VREF REGISTER SETTING 0 */
	DRAMC_VREFCTL0 = 0x644
};

enum {
	DRAMC_ACTIM0_DEFAULT	     = 0x22564154,
	DRAMC_CONF1_DEFAULT	     = 0x00000000,
	DRAMC_CONF2_DEFAULT	     = 0x00000000,
	DRAMC_PADCTL1_DEFAULT	     = 0x00000000,
	DRAMC_PADCTL2_DEFAULT	     = 0x00000000,
	DRAMC_PADCTL3_DEFAULT	     = 0x00000000,
	DRAMC_R0DELDLY_DEFAULT	     = 0x00000000,
	DRAMC_R1DELDLY_DEFAULT	     = 0x00000000,
	DRAMC_R0DIFDLY_DEFAULT	     = 0x00000000,
	DRAMC_R1DIFDLY_DEFAULT	     = 0x00000000,
	DRAMC_DLLCONF_DEFAULT	     = 0x8000FF01,
	DRAMC_TESTMODE_DEFAULT	     = 0x55010000,
	DRAMC_TEST2_1_DEFAULT	     = 0x01200000,
	DRAMC_TEST2_2_DEFAULT	     = 0x00010000,
	DRAMC_TEST2_3_DEFAULT	     = 0x00000000,
	DRAMC_TEST2_4_DEFAULT	     = 0x0000110D,
	DRAMC_DDR2CTL_DEFAULT	     = 0x00000000,
	DRAMC_MRS_DEFAULT	     = 0x00000000,
	DRAMC_CLK1DELAY_DEFAULT	     = 0x00000000,
	DRAMC_IOCTL_DEFAULT	     = 0x00000000,
	DRAMC_R0DQSIEN_DEFAULT	     = 0x00000000,
	DRAMC_R1DQSIEN_DEFAULT	     = 0x00000000,
	DRAMC_DRVCTL00_DEFAULT	     = 0xAA22AA22,
	DRAMC_DRVCTL0_DEFAULT	     = 0xAA22AA22,
	DRAMC_DRVCTL1_DEFAULT	     = 0xAA22AA22,
	DRAMC_DLLSEL_DEFAULT	     = 0x00000000,
	DRAMC_TDSEL0_DEFAULT	     = 0x00000000,
	DRAMC_TDSEL1_DEFAULT	     = 0x00000000,
	DRAMC_MCKDLY_DEFAULT	     = 0x00000900,
	DRAMC_DQSCTL0_DEFAULT	     = 0x00000000,
	DRAMC_DQSCTL1_DEFAULT	     = 0x00000000,
	DRAMC_PADCTL4_DEFAULT	     = 0x00000000,
	DRAMC_PADCTL5_DEFAULT	     = 0x00000000,
	DRAMC_PADCTL6_DEFAULT	     = 0x00000000,
	DRAMC_PHYCTL1_DEFAULT	     = 0x00000000,
	DRAMC_GDDR3CTL1_DEFAULT	     = 0x00000000,
	DRAMC_PADCTL7_DEFAULT	     = 0xEDCB0000,
	DRAMC_MISCTL0_DEFAULT	     = 0x00000000,
	DRAMC_OCDK_DEFAULT	     = 0x00000000,
	DRAMC_LBWDAT0_DEFAULT	     = 0x00000000,
	DRAMC_LBWDAT1_DEFAULT	     = 0x00000000,
	DRAMC_LBWDAT2_DEFAULT	     = 0x00000000,
	DRAMC_RKCFG_DEFAULT	     = 0x0B051100,
	DRAMC_CKPHDET_DEFAULT	     = 0x00000000,
	DRAMC_DQSGCTL_DEFAULT	     = 0xAA080088,
	DRAMC_CLKENCTL_DEFAULT	     = 0x50000000,
	DRAMC_DQSGCTL1_DEFAULT	     = 0x00000000,
	DRAMC_DQSGCTL2_DEFAULT	     = 0x00000000,
	DRAMC_ARBCTL0_DEFAULT	     = 0x00000000,
	DRAMC_CMDDLY0_DEFAULT	     = 0x00000000,
	DRAMC_CMDDLY1_DEFAULT	     = 0x00000000,
	DRAMC_CMDDLY2_DEFAULT	     = 0x00000000,
	DRAMC_CMDDLY3_DEFAULT	     = 0x00000000,
	DRAMC_CMDDLY4_DEFAULT	     = 0x00000000,
	DRAMC_CMDDLY5_DEFAULT	     = 0x00000000,
	DRAMC_DQSCAL0_DEFAULT	     = 0x00000000,
	DRAMC_DQSCAL1_DEFAULT	     = 0x00000000,
	DRAMC_DMMonitor_DEFAULT	     = 0x00C80000,
	DRAMC_DRAMC_PD_CTRL_DEFAULT  = 0x10622842,
	DRAMC_LPDDR2_DEFAULT	     = 0x00000000,
	DRAMC_SPCMD_DEFAULT	     = 0x00000000,
	DRAMC_ACTIM1_DEFAULT	     = 0x00000000,
	DRAMC_PERFCTL0_DEFAULT	     = 0x00000000,
	DRAMC_AC_DERATING_DEFAULT    = 0x00000000,
	DRAMC_RRRATE_CTL_DEFAULT     = 0x00020100,
	DRAMC_DQODLY1_DEFAULT	     = 0x00000000,
	DRAMC_DQODLY2_DEFAULT	     = 0x00000000,
	DRAMC_DQODLY3_DEFAULT	     = 0x00000000,
	DRAMC_DQODLY4_DEFAULT	     = 0x00000000,
	DRAMC_DQIDLY1_DEFAULT	     = 0x00000000,
	DRAMC_DQIDLY2_DEFAULT	     = 0x00000000,
	DRAMC_DQIDLY3_DEFAULT	     = 0x00000000,
	DRAMC_DQIDLY4_DEFAULT	     = 0x00000000,
	DRAMC_DQIDLY5_DEFAULT	     = 0x00000000,
	DRAMC_DQIDLY6_DEFAULT	     = 0x00000000,
	DRAMC_DQIDLY7_DEFAULT	     = 0x00000000,
	DRAMC_DQIDLY8_DEFAULT	     = 0x00000000,
	DRAMC_R2R_page_hit_counter_DEFAULT    = 0x00000000,
	DRAMC_R2R_page_miss_counter_DEFAULT   = 0x00000000,
	DRAMC_R2R_interbank_counter_DEFAULT   = 0x00000000,
	DRAMC_R2W_page_hit_counter_DEFAULT    = 0x00000000,
	DRAMC_R2W_page_miss_counter_DEFAULT   = 0x00000000,
	DRAMC_R2W_interbank_counter_DEFAULT   = 0x00000000,
	DRAMC_W2R_page_hit_counter_DEFAULT    = 0x00000000,
	DRAMC_W2R_page_miss_counter_DEFAULT   = 0x00000000,
	DRAMC_W2R_interbank_counter_DEFAULT   = 0x00000000,
	DRAMC_W2W_page_hit_counter_DEFAULT    = 0x00000000,
	DRAMC_W2W_page_miss_counter_DEFAULT   = 0x00000000,
	DRAMC_W2W_interbank_counter_DEFAULT   = 0x00000000,
	DRAMC_dramc_idle_counter_DEFAULT      = 0x00000000,
	DRAMC_freerun_26m_counter_DEFAULT     = 0x00000000,
	DRAMC_refresh_pop_counter_DEFAULT     = 0x00000000,
	DRAMC_JMETER_ST_DEFAULT		      = 0x00000000,
	DRAMC_DQ_CAL_MAX_0_DEFAULT	      = 0x00000000,
	DRAMC_DQ_CAL_MAX_1_DEFAULT	      = 0x00000000,
	DRAMC_DQ_CAL_MAX_2_DEFAULT	      = 0x00000000,
	DRAMC_DQ_CAL_MAX_3_DEFAULT	      = 0x00000000,
	DRAMC_DQ_CAL_MAX_4_DEFAULT	      = 0x00000000,
	DRAMC_DQ_CAL_MAX_5_DEFAULT	      = 0x00000000,
	DRAMC_DQ_CAL_MAX_6_DEFAULT	      = 0x00000000,
	DRAMC_DQ_CAL_MAX_7_DEFAULT	      = 0x00000000,
	DRAMC_DQS_CAL_MIN_0_DEFAULT	      = 0x00000000,
	DRAMC_DQS_CAL_MIN_1_DEFAULT	      = 0x00000000,
	DRAMC_DQS_CAL_MIN_2_DEFAULT	      = 0x00000000,
	DRAMC_DQS_CAL_MIN_3_DEFAULT	      = 0x00000000,
	DRAMC_DQS_CAL_MIN_4_DEFAULT	      = 0x00000000,
	DRAMC_DQS_CAL_MIN_5_DEFAULT	      = 0x00000000,
	DRAMC_DQS_CAL_MIN_6_DEFAULT	      = 0x00000000,
	DRAMC_DQS_CAL_MIN_7_DEFAULT	      = 0x00000000,
	DRAMC_DQS_CAL_MAX_0_DEFAULT	      = 0x00000000,
	DRAMC_DQS_CAL_MAX_1_DEFAULT	      = 0x00000000,
	DRAMC_DQS_CAL_MAX_2_DEFAULT	      = 0x00000000,
	DRAMC_DQS_CAL_MAX_3_DEFAULT	      = 0x00000000,
	DRAMC_DQS_CAL_MAX_4_DEFAULT	      = 0x00000000,
	DRAMC_DQS_CAL_MAX_5_DEFAULT	      = 0x00000000,
	DRAMC_DQS_CAL_MAX_6_DEFAULT	      = 0x00000000,
	DRAMC_DQS_CAL_MAX_7_DEFAULT	      = 0x00000000,
	DRAMC_DQICAL0_DEFAULT		      = 0x00000000,
	DRAMC_DQICAL1_DEFAULT		      = 0x00000000,
	DRAMC_DQICAL2_DEFAULT		      = 0x00000000,
	DRAMC_DQICAL3_DEFAULT		      = 0x00000000,
	DRAMC_CMP_ERR_DEFAULT		      = 0x00000000,
	DRAMC_R0DQSIENDLY_DEFAULT	      = 0x00000000,
	DRAMC_R1DQSIENDLY_DEFAULT	      = 0x00000000,
	DRAMC_STBEN0_DEFAULT		      = 0x00000003,
	DRAMC_STBEN1_DEFAULT		      = 0x00000003,
	DRAMC_STBEN2_DEFAULT		      = 0x00000003,
	DRAMC_STBEN3_DEFAULT		      = 0x00000003,
	DRAMC_DQSDLY0_DEFAULT		      = 0x0F0F0F0F,
	DRAMC_SPCMDRESP_DEFAULT		      = 0x00010300,
	DRAMC_IORGCNT_DEFAULT		      = 0x00000000,
	DRAMC_DQSGNWCNT0_DEFAULT	      = 0x00000000,
	DRAMC_DQSGNWCNT1_DEFAULT	      = 0x00000000,
	DRAMC_DQSGNWCNT2_DEFAULT	      = 0x00000000,
	DRAMC_DQSGNWCNT3_DEFAULT	      = 0x00000000,
	DRAMC_DQSGNWCNT4_DEFAULT	      = 0x00000000,
	DRAMC_DQSGNWCNT5_DEFAULT	      = 0x00000000,
	DRAMC_DQSSAMPLEV_DEFAULT	      = 0x00000000,
	DRAMC_DLLCNT0_DEFAULT		      = 0x00000000,
	DRAMC_CKPHCNT_DEFAULT		      = 0x00000000,
	DRAMC_TESTRPT_DEFAULT		      = 0x00000000,
	DRAMC_MEMPLL0_DEFAULT		      = 0x044C6000,
	DRAMC_MEMPLL1_DEFAULT		      = 0x00002000,
	DRAMC_MEMPLL2_DEFAULT		      = 0x0000044C,
	DRAMC_MEMPLL3_DEFAULT		      = 0x60000000,
	DRAMC_MEMPLL4_DEFAULT		      = 0x0000044C,
	DRAMC_MEMPLL5_DEFAULT		      = 0x60000000,
	DRAMC_MEMPLL6_DEFAULT		      = 0x0000044C,
	DRAMC_MEMPLL7_DEFAULT		      = 0x60000000,
	DRAMC_MEMPLL8_DEFAULT		      = 0x00000000,
	DRAMC_MEMPLL9_DEFAULT		      = 0x00000000,
	DRAMC_MEMPLL10_DEFAULT		      = 0x00000000,
	DRAMC_MEMPLL11_DEFAULT		      = 0x00000000,
	DRAMC_MEMPLL12_DEFAULT		      = 0x00000000,
	DRAMC_MEMPLL_DIVIDER_DEFAULT	      = 0x00000003,
	DRAMC_VREFCTL0_DEFAULT		      = 0x00000000
};

#endif				/* SOC_MEDIATEK_MT8173_EMI_HW_H */
