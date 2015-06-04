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

#ifndef _A60808_REGISTER_H_
#define _A60808_REGISTER_H_

#define DRIVING_DS2_0	    7	/* DS[2:0] 7->6 */
#define CATRAINING_STEP	    0

#define CH_INFO 30

#if 0
#define CHA_DRAMCAO_BASE	((uintptr_t)0x10004000)
#define CHA_DDRPHY_BASE		((uintptr_t)0x1000F000)
#define CHA_DRAMCNAO_BASE	((uintptr_t)0x1020E000)
#define CHB_DRAMCAO_BASE	((uintptr_t)0x10011000)
#define CHB_DDRPHY_BASE		((uintptr_t)0x10012000)
#define CHB_DRAMCNAO_BASE	((uintptr_t)0x10213000)
#define DDR_BASE		((uintptr_t)0x40000000)
#define EMI_APB_BASE		((uintptr_t)0x10203000)
#endif

#if 1
#define CHA_DRAMCAO_BASE	(0x10004000)
#define CHA_DDRPHY_BASE		(0x1000F000)
#define CHA_DRAMCNAO_BASE	(0x1020E000)
#define CHB_DRAMCAO_BASE	(0x10011000)
#define CHB_DDRPHY_BASE		(0x10012000)
#define CHB_DRAMCNAO_BASE	(0x10213000)
#define DDR_BASE		(0x40000000)
#define EMI_APB_BASE		(0x10203000)
#endif

/* DRAMC registers */
enum {
	DRAMC_REG_ACTIM0 = 0x000,
	POS_ACTIM0_BL2 = 15,
	DRAMC_REG_CONF1 = 0x004,
	POS_CONF1_BL4 = 10,
	POS_CONF1_SELFREF = 26,
	POS_CONF1_TESTLP = 27,
	DRAMC_REG_CONF2 = 0x008,
	POS_CONF2_TEST1 = 29,
	MASK_CONF2_TE12_ENABLE = 0xe0000000,
	DRAMC_REG_PADCTL1 = 0x00c,
	DRAMC_REG_PADCTL2 = 0x010,
	DRAMC_REG_PADCTL3 = 0x014,
	DRAMC_REG_DELDLY1 = 0x018,
	DRAMC_REG_01C	  = 0x01c,
	DRAMC_REG_DLLCONF = 0x028,
	DRAMC_REG_TEST2_0 = 0x038,   /* new on MT5399 */
	DRAMC_REG_TEST2_1 = 0x03C,
	DRAMC_REG_TEST2_2 = 0x040,
	DRAMC_REG_TEST2_3 = 0x044,
	POS_TEST2_3_ADVREFEN = 30,
	POS_TEST2_3_TESTAUDPAT = 7,
	/* bit0~3 */
	MASK_TEST2_3_TESTCNT = 0x0000000f,
	POS_TEST2_3_TESTCNT = 0,
	DRAMC_REG_TEST2_4 = 0x048,
	/* bit0~4 */
	MASK_TEST2_4_TESTAUDINC = 0x0000001f,
	POS_TEST2_4_TESTAUDINC = 0,
	/* bit8~12 */
	MASK_TEST2_4_TESTAUDINIT = 0x00001f00,
	POS_TEST2_4_TESTAUDINIT = 8,
	/* bit 14, 15 */
	POS_TEST2_4_TESTAUDBITINV = 14,
	POS_TEST2_4_TESTAUDMODE = 15,
	/* bit 16 */
	POS_TEST2_4_TESTXTALKPAT = 16,
	DRAMC_REG_DDR2CTL = 0x07c,
	MASK_DDR2CTL_DATLAT = 0x00000070,
	POS_DDR2CTL_DTALAT = 4,
	POS_DDR2CTL_WOEN = 3,
	DRAMC_REG_084 = 0x084,
	DRAMC_REG_MRS = 0x088,
	DRAMC_REG_CLK1DELAY = 0x08c,
	POS_CLK1DELAY_DQCMD = 3,
	DRAMC_REG_IOCTL = 0x090,
	DRAMC_REG_DQSIEN = 0x094,
	MASK_DQSIEN_R0DQS0IEN = 0x0000007f,
	MASK_DQSIEN_R0DQS1IEN = 0x00007f00,
	MASK_DQSIEN_R0DQS2IEN = 0x007f0000,
	MASK_DQSIEN_R0DQS3IEN = 0x7f000000,
	POS_DQSIEN_R0DQS0IEN = 0,
	POS_DQSIEN_R0DQS1IEN = 8,
	POS_DQSIEN_R0DQS2IEN = 16,
	POS_DQSIEN_R0DQS3IEN = 24,
	DRAMC_REG_R1DQSIEN   = 0x098,
	MASK_DQSIEN_R1DQS0IEN = 0x0000007f,
	MASK_DQSIEN_R1DQS1IEN = 0x00007f00,
	MASK_DQSIEN_R1DQS2IEN = 0x007f0000,
	MASK_DQSIEN_R1DQS3IEN = 0x7f000000,
	POS_DQSIEN_R1DQS0IEN = 0,
	POS_DQSIEN_R1DQS1IEN = 8,
	POS_DQSIEN_R1DQS2IEN = 16,
	POS_DQSIEN_R1DQS3IEN = 24,
	DRAMC_REG_IODRV4 = 0x0b0,
	MASK_IODRV4_DQDRVN_2 = 0x00000f00,
	MASK_IODRV4_DQDRVP_2 = 0x0000f000,
	MASK_IODRV4_CMDDRVN_2 = 0x0f000000,
	MASK_IODRV4_CMDDRVP_2 = 0xf0000000,
	POS_IODRV4_DQDRVN_2 = 8,
	POS_IODRV4_DQDRVP_2 = 12,
	POS_IODRV4_CMDDRVN_2 = 24,
	POS_IODRV4_CMDDRVP_2 = 28,
	DRAMC_REG_IODRV5 = 0x0b4,
	MASK_IODRV5_DQCDRVN = 0x00000f00,
	MASK_IODRV5_DQCDRVP = 0x0000f000,
	MASK_IODRV5_DQCDRVN_2 = 0x0f000000,
	MASK_IODRV5_DQCDRVP_2 = 0xf0000000,
	POS_IODRV5_DQCDRVN = 8,
	POS_IODRV5_DQCDRVP = 12,
	POS_IODRV5_DQCDRVN_2 = 24,
	POS_IODRV5_DQCDRVP_2 = 28,
	DRAMC_REG_DRVCTL0 = 0x0b8,
	MASK_DRVCTL0_DQDRVN = 0x00000f00,
	MASK_DRVCTL0_DQDRVP = 0x0000f000,
	MASK_DRVCTL0_DQSDRVN = 0x0f000000,
	MASK_DRVCTL0_DQSDRVP = 0xf0000000,
	POS_DRVCTL0_DQDRVN = 8,
	POS_DRVCTL0_DQDRVP = 12,
	POS_DRVCTL0_DQSDRVN = 24,
	POS_DRVCTL0_DQSDRVP = 28,
	DRAMC_REG_DRVCTL1 = 0x0bc,
	MASK_DRVCTL1_CMDDRVN = 0x00000f00,
	MASK_DRVCTL1_CMDDRVP = 0x0000f000,
	MASK_DRVCTL1_CLKDRVN = 0x0f000000,
	MASK_DRVCTL1_CLKDRVP = 0xf0000000,
	POS_DRVCTL1_CMDDRVN = 8,
	POS_DRVCTL1_CMDDRVP = 12,
	POS_DRVCTL1_CLKDRVN = 24,
	POS_DRVCTL1_CLKDRVP = 28,
	DRAMC_REG_MCKDLY = 0x0d8,
	/* bit 23 */
	POS_MCKDLY_FIXODT = 23,
	/* bit 12~15 */
	POS_MCKDLY_FIXDQIEN = 12,
	MASK_MCKDLY_FIXDQIEN = 0x0000f000,
	/* bit 10,11 */
	POS_MCKDLY_DQIENQKEND = 10,
	MASK_MCKDLY_DQIENQKEND = 0x00000c00,
	DRAMC_REG_DQSCTL0 = 0x0dc,
	DRAMC_REG_DQSCTL1 = 0x0e0,
	POS_DQSCTL1_DQSIENMODE = 28,
	MASK_DQSCTL1_DQSINCTL = 0x0f000000, /* for A60808, 4 bits totally */
	POS_DQSCTL1_DQSINCTL = 24,
	POS_DQSCTL1_DQS2CTL = 0,      /* [3:0] A60808: HW run-time DATLAT */
	MASK_DQSCTL1_DQS2CTL = 0x0000000f,
	DRAMC_REG_PADCTL4 = 0x0e4,
	MASK_PADCTL4_DATLAT3 = 0x00000010,
	POS_PADCTL4_DATLAT3 = 4,
	POS_PADCTL4_CKEFIXON = 2,
	DRAMC_REG_PHYCTL1 = 0x0f0,
	MASK_PHYCTL1_DATLAT4 = 0x02000000,
	POS_PHYCTL1_DATLAT4 = 25,
	POS_PHYCTL1_PHYRST = 28,
	DRAMC_REG_GDDR3CTL1 = 0x0f4,
	POS_GDDR3CTL1_DQMSWAP = 31,
	POS_GDDR3CTL1_RDATRST = 25,
	POS_GDDR3CTL1_BKSWAP = 20,
	DRAMC_REG_PADCTL7 = 0x0f8,
	DRAMC_REG_MISCTL0 = 0x0fc,
	DRAMC_REG_OCDK = 0x100,
	POS_OCDK_DRVREF = 24,
	DRAMC_REG_RKCFG = 0x110,
	POS_RKCFG_PBREF_DISBYRATE = 6,
	/* 32-bit channel it is 0, for 16-bit or asym DRAM it is 1 */
	POS_RKCFG_WDATKEY64 = 29,
	DRAMC_REG_CKPHDET = 0x114,
	DRAMC_REG_DQSCTL2 = 0x118,
	MASK_DQSCTL2_DQSINCTL = 0x0000000f,
	POS_DQSCTL2_DQSINCTL = 0,
	DRAMC_REG_DQ_DQS01_SEL = 0x120,
	DRAMC_REG_DQSGCTL = 0x124,
	MASK_DQSGCTL_R0DQSG_COARSE_DLY_COM0 = 0x00000003,
	MASK_DQSGCTL_R1DQSG_COARSE_DLY_COM0 = 0x0000000C,
	MASK_DQSGCTL_R0DQSG_COARSE_DLY_COM1 = 0x00000030,
	MASK_DQSGCTL_R1DQSG_COARSE_DLY_COM1 = 0x000000C0,
	POS_DQSGCTL_R0DQSG_COARSE_DLY_COM0 = 0,
	POS_DQSGCTL_R1DQSG_COARSE_DLY_COM0 = 2,
	POS_DQSGCTL_R0DQSG_COARSE_DLY_COM1 = 4,
	POS_DQSGCTL_R1DQSG_COARSE_DLY_COM1 = 6,
	POS_DQSGCTL_DQSGDUALP = 30,
	DRAMC_REG_CLKENCTL = 0x130,
	DRAMC_REG_WRLEV = 0x13c,
	/* bit 12 */
	POS_WRLEV_DQS_SEL = 12,
	MASK_WRLEV_DQS_SEL = 0x0000f000,
	/* bit 8 */
	POS_WRLEV_DQS_WLEV = 8,
	/* bit 1~4 */
	POS_WRLEV_DQS_Bx_G = 1,
	MASK_WRLEV_DQS_Bx_G = 0x0000001e,
	/* bit 0 */
	POS_WRLEV_WRITE_LEVEL_EN = 0,
	DRAMC_REG_PHYCLKDUTY = 0x148,
	POS_PHYCLKDUTY_CMDCLKP0DUTYSEL = 28,
	POS_PHYCLKDUTY_CMDCLKP0DUTYP = 18,
	MASK_PHYCLKDUTY_CMDCLKP0DUTYP = 0x000c0000,
	POS_PHYCLKDUTY_CMDCLKP0DUTYN = 16,
	MASK_PHYCLKDUTY_CMDCLKP0DUTYN = 0x00030000,
	DRAMC_REG_158 = 0x158,
	DRAMC_REG_ARBCTL0 = 0x168,
	DRAMC_REG_CMDDLY0 = 0x1a8,
	DRAMC_REG_CMDDLY1 = 0x1ac,
	DRAMC_REG_CMDDLY2 = 0x1b0,
	DRAMC_REG_CMDDLY3 = 0x1b4,
	DRAMC_REG_CMDDLY4 = 0x1b8,
	DRAMC_REG_DQSCAL0 = 0x1c0,
	POS_DQSCAL0_STBCALEN = 31,
	DRAMC_REG_IMPCAL = 0x1c8,
	POS_IMPCAL_IMPCALI_EN = 0,
	POS_IMPCAL_IMPCAL_HW = 1,
	POS_IMPCAL_IMP_CALI_ENN = 4,
	POS_IMPCAL_IMP_CALI_ENP = 5,
	POS_IMPCAL_IMPPDN = 6,
	POS_IMPCAL_IMPPDP = 7,
	POS_IMPCAL_IMPDRVP = 8,
	MASK_IMPCAL_IMPDRVP = 0x00000f00,
	POS_IMPCAL_IMPDRVN = 12,
	MASK_IMPCAL_IMPDRVN = 0x0000f000,
	DRAMC_REG_DMMonitor = 0x1d8,
	DRAMC_REG_DRAMC_PD_CTRL = 0x1dc,
	MASK_DRAMC_PD_CTRL_REFCNT_FR_CLK = 0x00ff0000,
	POS_DRAMC_PD_CTRL_REFCNT_FR_CLK = 16,
	POS_DRAMC_PD_CTRL_MIOCKCTRLOFF = 26,
	DRAMC_REG_LPDDR2 = 0x1e0,
	POS_LPDDR2_ADRDECEN = 31,
	POS_LPDDR2_SELO1ASO = 30,
	DRAMC_REG_SPCMD = 0x1e4,
	POS_SPCMD_MRWEN = 0,
	POS_SPCMD_DQSGCNTEN = 8,
	POS_SPCMD_DQSGCNTRST = 9,
	DRAMC_REG_ACTIM1 = 0x1e8,
	MASK_ACTIM1_REFRCNT = 0x00ff0000,
	POS_ACTIM1_REFRCNT = 16,
	DRAMC_REG_PERFCTL0 = 0x1ec,
	DRAMC_REG_RRRATE_CTL = 0x1f4,
	MASK_RRRATE_CTL_BIT0_SEL = 0x0000001f,
	MASK_RRRATE_CTL_BIT1_SEL = 0x00001f00,
	MASK_RRRATE_CTL_BIT2_SEL = 0x001f0000,
	MASK_RRRATE_CTL_BIT3_SEL = 0x1f000000,
	POS_RRRATE_CTL_BIT0_SEL = 0,
	POS_RRRATE_CTL_BIT1_SEL = 8,
	POS_RRRATE_CTL_BIT2_SEL = 16,
	POS_RRRATE_CTL_BIT3_SEL = 24,
	DRAMC_REG_LPDDR2_4 = 0x1f8,
	DRAMC_REG_MRR_CTL = 0x1fc,
	MASK_MRR_CTL_BIT4_SEL = 0x0000001f,
	MASK_MRR_CTL_BIT5_SEL = 0x00001f00,
	MASK_MRR_CTL_BIT6_SEL = 0x001f0000,
	MASK_MRR_CTL_BIT7_SEL = 0x1f000000,
	POS_MRR_CTL_BIT4_SEL = 0,
	POS_MRR_CTL_BIT5_SEL = 8,
	POS_MRR_CTL_BIT6_SEL = 16,
	POS_MRR_CTL_BIT7_SEL = 24,
	DRAMC_REG_DQODLY1 = 0x200,
	DRAMC_REG_DQODLY2 = 0x204,
	DRAMC_REG_DQODLY3 = 0x208,
	DRAMC_REG_DQODLY4 = 0x20c,
	DRAMC_REG_DQIDLY1 = 0x210,
	DRAMC_REG_DQIDLY2 = 0x214,
	DRAMC_REG_DQIDLY3 = 0x218,
	DRAMC_REG_DQIDLY4 = 0x21c,
	DRAMC_REG_DQIDLY5 = 0x220,
	DRAMC_REG_DQIDLY6 = 0x224,
	DRAMC_REG_DQIDLY7 = 0x228,
	DRAMC_REG_DQIDLY8 = 0x22c,
	DRAMC_REG_DMACON = 0x234,
	POS_DMACON_DMAEN = 0,
	POS_DMACON_DMAPUREWRITEEN = 1,
	POS_DMACON_DMAPUREREADEN = 2,
	POS_DMACON_DMABURSTLEN = 4,
	MASK_DMACON_DMABURSTLEN = 0x00000070,
	DRAMC_REG_DMASRCADDR = 0x238,
	DRAMC_REG_DMADSTADDR = 0x23c,
	DRAMC_REG_DMATRANSLEN = 0x240,
	POS_DMATRANSLEN_DMATRANSLEN = 0,
	MASK_DMATRANSLEN_DMATRANSLEN = 0x000fffff,
	DRAMC_REG_DMAPAT = 0x244,
	DRAMC_REG_DMAMON = 0x254,
	POS_DMAMON_DMASTATUS = 0,
	DRAMC_REG_DMACMPERR = 0x258,
	DRAMC_REG_R2R_page_hit_counter = 0x280,
	DRAMC_REG_STBENERR_R = 0x320,
	DRAMC_REG_STBENERR_F = 0x324,
	DRAMC_REG_TOGGLE_CNT = 0x32c,
	DRAMC_REG_DQS_ERR_CNT = 0x330,
	DRAMC_REG_DQ_ERR_CNT = 0x334,
	DRAMC_REG_SELPH = 0x344,
	DRAMC_REG_DCBLN = 0x348,
	POS_DCBLN_RSTBCNT_LATCH_EN = 11,
	POS_DCBLN_RX_MIOCK_JIT_EN = 2,
	POS_DCBLN_RX_EYE_SCAN_EN = 1,
	POS_DCBLN_REG_SW_RST = 0,
	DRAMC_REG_TOGGLE_CNT_2 = 0x360,
	DRAMC_REG_DQS_ERR_CNT_2 = 0x364,
	DRAMC_REG_DQ_ERR_CNT_2 = 0x368,
	DRAMC_REG_CMP_ERR = 0x370,
	DRAMC_REG_DQSDLY0 = 0x3a0,
	DRAMC_REG_SPCMDRESP = 0x3b8,
	POS_SPCMDRESP_SREF_STATE = 16,
	DRAMC_REG_DQSGNWCNT0 = 0x3c0,
	DRAMC_REG_DQSGNWCNT1 = 0x3c4,
	DRAMC_REG_DLLSTATUS0 = 0x3dc,
	POS_DLLSTATUS0_CMPOT = 31,
	POS_DLLSTATUS0_CMPOUTN = 30,
	DRAMC_REG_TESTRPT = 0x3fc,
	POS_TESTRPT_DM_CMP_CPT = 10,
	POS_TESTRPT_DM_CMP_ERR = 14,
	DRAMC_REG_SELPH1 = 0x400,
	DRAMC_REG_SELPH2 = 0x404,
	MASK_SELPH2_TXDLY_DQSGATE = 0x00007000,
	POS_SELPH2_TXDLY_DQSGATE = 12,
	MASK_SELPH2_TXDLY_DQSGATE_P1 = 0x00700000,
	POS_SELPH2_TXDLY_DQSGATE_P1 = 20,
	DRAMC_REG_SELPH3 = 0x408,
	DRAMC_REG_SELPH4 = 0x40C,
	DRAMC_REG_SELPH5 = 0x410,
	MASK_SELPH5_dly_DQSGATE = 0x00c00000,
	POS_SELPH5_dly_DQSGATE = 22,
	MASK_SELPH5_dly_DQSGATE_P1 = 0x03000000,
	POS_SELPH5_dly_DQSGATE_P1 = 24,
	DRAMC_REG_SELPH6 = 0x414,
	DRAMC_REG_SELPH6_1 = 0x418,
	MASK_SELPH6_1_TXDLY_R1DQSGATE = 0x00000070,
	POS_SELPH6_1_TXDLY_R1DQSGATE = 4,
	MASK_SELPH6_1_TXDLY_R1DQSGATE_P1 = 0x00000700,
	POS_SELPH6_1_TXDLY_R1DQSGATE_P1 = 8,
	MASK_SELPH6_1_dly_R1DQSGATE = 0x00000003,
	POS_SELPH6_1_dly_R1DQSGATE = 0,
	MASK_SELPH6_1_dly_R1DQSGATE_P1 = 0x0000000c,
	POS_SELPH6_1_dly_R1DQSGATE_P1 = 2,
	DRAMC_REG_SELPH7 = 0x41c,
	DRAMC_REG_SELPH8 = 0x420,
	DRAMC_REG_SELPH9 = 0x424,
	DRAMC_REG_SELPH10 = 0x428,
	DRAMC_REG_SELPH11 = 0x42c,
	DRAMC_REG_SELPH12 = 0x430,
	DRAMC_REG_SELPH13 = 0x434,
	DRAMC_REG_SELPH14 = 0x438,
	DRAMC_REG_SELPH15 = 0x43c,
	DRAMC_REG_SELPH16 = 0x440,
	DRAMC_REG_COM0_DLY_SEL_DGTED = 0x478,
	DRAMC_REG_COM0_DLY_SEL_DQS0 = 0x47C,
	DRAMC_REG_COM0_DLY_SEL_DQS1 = 0x480,
	DRAMC_REG_COM0_DLY_SEL_DQM0 = 0x484,
	DRAMC_REG_COM0_DLY_SEL_DQ2 = 0x494,
	DRAMC_REG_COM0_DLY_SEL_DQ3 = 0x498,
	DRAMC_REG_COM0_DLY_SEL_DQ4 = 0x49c,
	DRAMC_REG_COM0_DLY_SEL_DQ5 = 0x4a0,
	DRAMC_REG_COM0_DLY_SEL_DQ6 = 0x4a4,
	DRAMC_REG_COM0_DLY_SEL_DQ7 = 0x4a8,
	DRAMC_REG_COM1_DLY_SEL_DGTED = 0x4CC,
	DRAMC_REG_COM1_DLY_SEL_DQS0 = 0x4D0,
	DRAMC_REG_COM1_DLY_SEL_DQS1 = 0x4D4,
	DRAMC_REG_COM1_DLY_SEL_DQM0 = 0x4D8,
	DRAMC_REG_COM1_DLY_SEL_DQ0 = 0x4e0,
	DRAMC_REG_COM1_DLY_SEL_DQ2 = 0x4e8,
	DRAMC_REG_COM1_DLY_SEL_DQ3 = 0x4ec,
	DRAMC_REG_COM1_DLY_SEL_DQ4 = 0x4f0,
	DRAMC_REG_COM1_DLY_SEL_DQ5 = 0x4f4,
	DRAMC_REG_COM1_DLY_SEL_DQ6 = 0x4f8,
	DRAMC_REG_COM1_DLY_SEL_DQ7 = 0x4fc,
	DRAMC_REG_PH_EN = 0x520,
	DRAMC_REG_DLY_SEL_MCK_ANA = 0x524,
	DRAMC_REG_DLY_SEL_CLKGEN = 0x528,
	DRAMC_REG_DLY_SEL_CLKGEN2 = 0x52C
};

typedef struct {
	uint32_t actim0;		/* 0x0 */
	uint32_t conf1;			/* 0x4 */
	uint32_t conf2;			/* 0x8 */
	uint32_t rsvd_ao1[3];		/* 0xc */
	uint32_t r0deldly;		/* 0x18 */
	uint32_t r1deldly;		/* 0x1c */
	uint32_t r0difdly;		/* 0x20 */
	uint32_t r1difdly;		/* 0x24 */
	uint32_t dllconf;		/* 0x28 */
	uint32_t rsvd_ao2[6];		/* 0x2c */
	uint32_t test2_3;		/* 0x44 */
	uint32_t test2_4;		/* 0x48 */
	uint32_t catraining;		/* 0x4c */
	uint32_t catraining2;		/* 0x50 */
	uint32_t wodt;			/* 0x54 */
	uint32_t rsvd_ao3[9];		/* 0x58 */
	uint32_t ddr2ctl;		/* 0x7c */
	uint32_t misc;			/* 0x80 */
	uint32_t zqcs;			/* 0x84 */
	uint32_t mrs;			/* 0x88 */
	uint32_t clk1delay;		/* 0x8c */
	uint32_t rsvd_ao4[1];		/* 0x90 */
	uint32_t r0dqsien;		/* 0x94 */
	uint32_t r1dqsien;		/* 0x98 */
	uint32_t rsvd_ao5[2];		/* 0x9c */
	uint32_t iodrv[6];		/* 0xa4 */
	uint32_t rsvd_ao6[1];		/* 0xbc */
	uint32_t dllsel;		/* 0xc0 */
	uint32_t rsvd_ao7[5];		/* 0xc4 */
	uint32_t mckdly;		/* 0xd8 */
	uint32_t rsvd_ao8[1];		/* 0xdc */
	uint32_t dqsctl1;		/* 0xe0 */
	uint32_t padctl4;		/* 0xe4 */
	uint32_t rsvd_ao9[2];		/* 0xe8 */
	uint32_t phyctl1;		/* 0xf0 */
	uint32_t gddr3ctl1;		/* 0xf4 */
	uint32_t padctl7;		/* 0xf8 */
	uint32_t misctl0;		/* 0xfc */
	uint32_t ocdk;			/* 0x100 */
	uint32_t rsvd_ao10[3];		/* 0x104 */
	uint32_t rkcfg;			/* 0x110 */
	uint32_t ckphdet;		/* 0x114 */
	uint32_t dqsctl2;		/* 0x118 */
	uint32_t rsvd_ao11[5];		/* 0x11c */
	uint32_t clkctl;		/* 0x130 */
	uint32_t rsvd_ao12[1];		/* 0x134 */
	uint32_t dummy;			/* 0x138 */
	uint32_t write_leveling;	/* 0x13c */
	uint32_t rsvd_ao13[10];		/* 0x140 */
	uint32_t arbctl0;		/* 0x168 */
	uint32_t rsvd_ao14[21];		/* 0x16c */
	uint32_t dqscal0;		/* 0x1c0 */
	uint32_t dqscal1;		/* 0x1c4 */
	uint32_t impcal;		/* 0x1c8 */
	uint32_t rsvd_ao15[4];		/* 0x1cc */
	uint32_t dramc_pd_ctrl;		/* 0x1dc */
	uint32_t lpddr2_3;		/* 0x1e0 */
	uint32_t spcmd;			/* 0x1e4 */
	uint32_t actim1;		/* 0x1e8 */
	uint32_t perfctl0;		/* 0x1ec */
	uint32_t ac_derating;		/* 0x1f0 */
	uint32_t rrrate_ctl;		/* 0x1f4 */
	uint32_t ac_time_05t;		/* 0x1f8 */
	uint32_t mrr_ctl;		/* 0x1fc */
	uint32_t rsvd_ao16[4];		/* 0x200 */
	uint32_t dqidly[9];		/* 0x210 */
	uint32_t rsvd_ao17[115];	/* 0x234 */
	uint32_t selph[12];		/* 0x400 */
} dramc_ao_regs;

typedef struct {
	uint32_t rsvd_nao1[11];			/* 0x0 */
	uint32_t test_mode;			/* 0x2c */
	uint32_t rsvd_nao2[3];			/* 0x30 */
	uint32_t test2_1;			/* 0x3c */
	uint32_t test2_2;			/* 0x40 */
	uint32_t rsvd_nao3[48];			/* 0x44 */
	uint32_t lbwdat0;			/* 0x104 */
	uint32_t lbwdat1;			/* 0x108 */
	uint32_t lbwdat2;			/* 0x10c */
	uint32_t rsvd_nao4[1];			/* 0x110 */
	uint32_t ckphdet;			/* 0x114 */
	uint32_t rsvd_nao5[48];			/* 0x118 */
	uint32_t dmmonitor;			/* 0x1d8 */
	uint32_t rsvd_nao6[41];			/* 0x1dc */
	uint32_t r2r_page_hit_counter;		/* 0x280 */
	uint32_t r2r_page_miss_counter;		/* 0x284 */
	uint32_t r2r_interbank_counter;		/* 0x288 */
	uint32_t r2w_page_hit_counter;		/* 0x28c */
	uint32_t r2w_page_miss_counter;		/* 0x290 */
	uint32_t r2w_interbank_counter;		/* 0x294 */
	uint32_t w2r_page_hit_counter;		/* 0x298 */
	uint32_t w2r_page_miss_counter;		/* 0x29c */
	uint32_t w2r_page_interbank_counter;	/* 0x2a0 */
	uint32_t w2w_page_hit_counter;		/* 0x2a4 */
	uint32_t w2w_page_miss_counter;		/* 0x2a8 */
	uint32_t w2w_page_interbank_counter;	/* 0x2ac */
	uint32_t dramc_idle_counter;		/* 0x2b0 */
	uint32_t freerun_26m_counter;		/* 0x2b4 */
	uint32_t refresh_pop_counter;		/* 0x2b8 */
	uint32_t jmeter_st;			/* 0x2bc */
	uint32_t dq_cal_max[8];			/* 0x2c0 */
	uint32_t dqs_cal_min[8];		/* 0x2e0 */
	uint32_t dqs_cal_max[8];		/* 0x300 */
	uint32_t rsvd_nao7[4];			/* 0x320 */
	uint32_t read_bytes_counter;		/* 0x330 */
	uint32_t write_bytes_counter;		/* 0x334 */
	uint32_t rsvd_nao8[6];			/* 0x338 */
	uint32_t dqical[4];			/* 0x350 */
	uint32_t rsvd_nao9[4];			/* 0x360 */
	uint32_t cmp_err;			/* 0x370 */
	uint32_t r0dqsiendly;			/* 0x374 */
	uint32_t r1dqsiendly;			/* 0x378 */
	uint32_t rsvd_nao10[9];			/* 0x37c */
	uint32_t dqsdly0;			/* 0x3a0 */
	uint32_t rsvd_nao11[4];			/* 0x3a4 */
	uint32_t mrrdata;			/* 0x3b4 */
	uint32_t spcmdresp;			/* 0x3b8 */
	uint32_t iorgcnt;			/* 0x3bc */
	uint32_t dqsgnwcnt[6];			/* 0x3c0 */
	uint32_t rsvd_nao12[4];			/* 0x3d8 */
	uint32_t ckphcnt;			/* 0x3e8 */
	uint32_t rsvd_nao13[4];			/* 0x3ec */
	uint32_t testrpt;			/* 0x3fc */
} dramc_nao_regs;

typedef struct {
	uint32_t rsvd_phy1[3];		/* 0x0 */
	uint32_t padctl[3];		/* 0xc */
	uint32_t rsvd_phy2[25];		/* 0x18 */
	uint32_t ddr2ctl;		/* 0x7c */
	uint32_t rsvd_phy3[3];		/* 0x80 */
	uint32_t clk1delay;		/* 0x8c */
	uint32_t ioctl;			/* 0x90 */
	uint32_t rsvd_phy4[9];		/* 0x94 */
	uint32_t iodrv6;		/* 0xb8 */
	uint32_t drvctl1;		/* 0xbc */
	uint32_t dllsel;		/* 0xc0 */
	uint32_t rsvd_phy5[2];		/* 0xc4 */
	uint32_t tdsel[3];		/* 0xcc */
	uint32_t mckdly;		/* 0xd8 */
	uint32_t dqsctl0;		/* 0xdc */
	uint32_t dqsctl1;		/* 0xe0 */
	uint32_t dqsctl4;		/* 0xe4 */
	uint32_t dqsctl5;		/* 0xe8 */
	uint32_t dqsctl6;		/* 0xec */
	uint32_t phyctl1;		/* 0xf0 */
	uint32_t gddr3ctl1;		/* 0xf4 */
	uint32_t rsvd_phy6[1];		/* 0xf8 */
	uint32_t misctl0;		/* 0xfc */
	uint32_t ocdk;			/* 0x100 */
	uint32_t rsvd_phy7[8];		/* 0x104 */
	uint32_t dqsgctl;		/* 0x124 */
	uint32_t rsvd_phy8[6];		/* 0x128 */
	uint32_t ddrphydqsgctl;		/* 0x140 */
	uint32_t dqsgct2;		/* 0x144 */
	uint32_t phyclkduty;		/* 0x148 */
	uint32_t rsvd_phy9[3];		/* 0x14c */
	uint32_t dqsisel;		/* 0x158 */
	uint32_t dqmdqs_sel;		/* 0x15c */
	uint32_t rsvd_phy10[10];	/* 0x160 */
	uint32_t jmeterpop[4];		/* 0x188 */
	uint32_t rsvd_0x198[4];		/* 0x198 */
	uint32_t cmddly[6];		/* 0x1a8 */
	uint32_t dqscal0;		/* 0x1c0 */
	uint32_t rsvd_phy11[2];		/* 0x1c4 */
	uint32_t jmeter2;		/* 0x1cc */
	uint32_t jmeter3;		/* 0x1d0 */
	uint32_t jmeter4;		/* 0x1d4 */
	uint32_t rsvd_phy12[2];		/* 0x1d8 */
	uint32_t lpddr2_3;		/* 0x1e0 */
	uint32_t spcmd;			/* 0x1e4 */
	uint32_t rsvd_phy13[6];		/* 0x1e8 */
	uint32_t dqodly[4];		/* 0x200 */
	uint32_t rsvd_phy14[11];	/* 0x210 */
	uint32_t lpddr2_4;		/* 0x23c */
	uint32_t rsvd_phy15[56];	/* 0x240 */
	uint32_t jmeter_pll2_st;	/* 0x320 */
	uint32_t jmeter_pll3_st;	/* 0x324 */
	uint32_t jmeter_pll4_st;	/* 0x328 */
	uint32_t jmeter_done_st;	/* 0x32c */
	uint32_t rsvd_0x330[2];		/* 0x330 */
	uint32_t jmeter_pll1_st;	/* 0x338 */
	uint32_t jmeter_pop_pll2_st;	/* 0x33c */
	uint32_t jmeter_pop_pll3_st;	/* 0x340 */
	uint32_t jmeter_pop_pll4_st;	/* 0x344 */
	uint32_t jmeter_pop_pll1_st;	/* 0x348 */
	uint32_t rsvd_phy16[13];	/* 0x34c */
	uint32_t dq_o1;			/* 0x380 */
	uint32_t rsvd_phy17[2];		/* 0x384 */
	uint32_t stben[4];		/* 0x38c */
	uint32_t rsvd_phy18[16];	/* 0x39c */
	uint32_t dllcnt0;		/* 0x3dc */
	uint32_t pllautok;		/* 0x3e0 */
	uint32_t poppllautok;		/* 0x3e4 */
	uint32_t rsvd_phy19[18];	/* 0x3e8 */
	uint32_t selph12;		/* 0x430 */
	uint32_t selph13;		/* 0x434 */
	uint32_t selph14;		/* 0x438 */
	uint32_t selph15;		/* 0x43c */
	uint32_t selph16;		/* 0x440 */
	uint32_t selph17;		/* 0x444 */
	uint32_t selph18;		/* 0x448 */
	uint32_t selph19;		/* 0x44c */
	uint32_t selph20;		/* 0x450 */
	uint32_t rsvd_phy20[91];	/* 0x454 */
	uint32_t peri[4];		/* 0x5c0 */
	uint32_t rsvd_phy21[12];	/* 0x5d0 */
	uint32_t mempll[15];		/* 0x600 */
	uint32_t ddrphy_cg_ctrl;	/* 0x63c */
	uint32_t mempll_divider;	/* 0x640 */
	uint32_t vrefctl0;		/* 0x644 */
	uint32_t rsvd_phy22[18];	/* 0x648 */
	uint32_t mempll05_divider;	/* 0x690 */
} dramc_ddrphy_regs;

#endif /* _MT5133_REGISTER_H_ */