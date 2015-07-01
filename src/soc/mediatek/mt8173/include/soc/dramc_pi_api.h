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

#ifndef _DRAMC_PI_API_H
#define _DRAMC_PI_API_H

#include "emi.h"
#include "dramc_register.h"

enum {
	MAX_CLKO_DELAY = 15,
	CATRAINING_NUM = 10
};

enum {
	/* MEMPLL config */
	Fin = 52,
	/* jitter meter for PLL phase calibration */
	fcJMETER_COUNT = 1024,
	/* 10us for more margin */
	fcJMETER_WAIT_DONE_US = (fcJMETER_COUNT/Fin+10)
};

enum {
	DQ_DATA_WIDTH = 32,
	DQS_BIT_NUMBER = 8,
	PARAM_NUMBER = 2,
	DQS_NUMBER = (DQ_DATA_WIDTH / DQS_BIT_NUMBER)
	/* max support bus width of the system (array size use) */
};

/* TX DQ/DQS */
enum {
	MAX_TX_DQDLY_TAPS = 16,   /* max DQ TAP number */
	MAX_TX_DQSDLY_TAPS = 16   /* max DQS TAP number */
};

enum {
	ENABLE = 1,
	DISABLE = 0
};

typedef enum {
	DRAM_OK = 0, /* OK */
	DRAM_FAIL    /* FAIL */
} DRAM_STATUS_T; /* DRAM status */

typedef enum {
	CHANNEL_A = 0,
	CHANNEL_B,
} DRAM_CHANNEL_T;

typedef enum {
	TYPE_mDDR = 1,
	TYPE_LPDDR2,
	TYPE_LPDDR3,
	TYPE_PCDDR3
} DRAM_DRAM_TYPE_T;

typedef enum {
	DATA_WIDTH_16BIT = 16,
	DATA_WIDTH_32BIT = 32
} DRAM_DATA_WIDTH_T;

typedef enum {
	PACKAGE_SBS = 0,
	PACKAGE_POP
} DRAM_PACKAGE_T;

typedef enum {
	TE_OP_WRITE_READ_CHECK = 0,
	TE_OP_READ_CHECK
} DRAM_TE_OP_T;

enum {
	/* pattern0 and base address for test engine when we do calibration */
	DEFAULT_TEST2_1_CAL = 0x55000000,
	/* for testing, to separate TA4-3 address for running simultaneously */
	/* pattern1 and offset address for test engine when we  do calibraion */
	DEFAULT_TEST2_2_CAL = 0xaa000400,
	/* pattern0 and base addr. for test engine when doing dqs GW */
	DEFAULT_TEST2_1_DQSIEN = 0x55000000,
	/* pattern1 and offset addr. for test engine when doing dqs GW */
	DEFAULT_TEST2_2_DQSIEN = 0xaa000010,
	/* gold pattern */
	DEFAULT_GOLD_DQSIEN = 0x20202020
};

typedef enum {
	TEST_ISI_PATTERN = 0,
	TEST_AUDIO_PATTERN,
	TEST_TA1_SIMPLE,
	TEST_TESTPAT4,
	TEST_TESTPAT4_3,
	TEST_XTALK_PATTERN,
	TEST_MIX_PATTERN
} DRAM_TEST_PATTERN_T;

typedef struct _DRAMC_CTX_T {
	DRAM_CHANNEL_T channel;
	DRAM_DRAM_TYPE_T dram_type;
	DRAM_PACKAGE_T package;
	DRAM_DATA_WIDTH_T data_width;
	u32 test2_1;
	u32 test2_2;
	DRAM_TEST_PATTERN_T test_pattern;
	u16 frequency;
	u16 frequency_low;
	u8 fglow_freq_write_en;
	u8 ssc_en;
	u8 en_4bitMux;
} DRAMC_CTX_T;

typedef struct _RXDQS_PERBIT_DLY_T {
	s8 first_dqdly_pass;
	s8 last_dqdly_pass;
	s8 first_dqsdly_pass;
	s8 last_dqsdly_pass;
	s8 best_first_dqdly_pass;
	s8 best_last_dqdly_pass;
	s8 best_first_dqsdly_pass;
	s8 best_last_dqsdly_pass;
	u8 best_dqdly;
	u8 best_dqsdly;
} RXDQS_PERBIT_DLY_T;

typedef struct _TXDQS_PERBIT_DLY_T {
	s8 first_dqdly_pass;
	s8 last_dqdly_pass;
	s8 first_dqsdly_pass;
	s8 last_dqsdly_pass;
	s8 best_first_dqdly_pass;
	s8 best_last_dqdly_pass;
	s8 best_first_dqsdly_pass;
	s8 best_last_dqsdly_pass;
	u8 best_dqdly;
	u8 best_dqsdly;
} TXDQS_PERBIT_DLY_T;

static inline u32 mcSET_DRAMC_REG_ADDR(DRAMC_CTX_T *p, u32 offset)
{

	return ((p->channel << CH_INFO) | (offset));
}

extern u8 RXPERBIT_LOG_PRINT;
extern u8 GATINGDQS_LOG_PRINT;

extern u8 RAM_ID;
extern u8 CurrentRank;
extern unsigned int uiDualRank;

extern const u32 uiLPDDR_PHY_Mapping_POP_CHA[32];
extern const u32 uiLPDDR_PHY_Mapping_POP_CHB[32];
extern u8 opt_gw_coarse_value_R0[2], opt_gw_fine_value_R0[2];
extern u8 opt_gw_coarse_value_R1[2], opt_gw_fine_value_R1[2];

/* Testing */
void Dump_Registers(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcRegDump(DRAMC_CTX_T *p);
u32 DramcGetMR2ByFreq(int dram_freq);

void TransferToSPMControl(void);
void TransferToRegControl(void);
void DramcPhyReset(DRAMC_CTX_T *p);
void DramcClkDutyCal(DRAMC_CTX_T *p);
void DramcLowFreqWrite(DRAMC_CTX_T *p);
void DramcDiv2PhaseSync(DRAMC_CTX_T *p);
void DramcRunTimeConfig(DRAMC_CTX_T *p);
void DramcRANKINCTLConfig(DRAMC_CTX_T *p);
void DramcEnterSelfRefresh(DRAMC_CTX_T *p, u8 op);

/* Register API prototypes */
inline u8 ucDram_Register_Read(u32 u4reg_addr, u32 *pu4reg_value);
inline u8 ucDram_Register_Write(u32 u4reg_addr, u32 u4reg_value);
inline u8 ucDramC_Register_Read(u32 u4reg_addr, u32 *pu4reg_value);
inline u8 ucDramC_Register_Write(u32 u4reg_addr, u32 u4reg_value);

inline u8 dramc_setbit(DRAMC_CTX_T *p, u32 addr, u32 bit);
inline u8 dramc_clrbit(DRAMC_CTX_T *p, u32 addr, u32 bit);
inline u8 dramc_write32(DRAMC_CTX_T *p, u32 addr, u32 val);
inline u8 dramc_setfield(DRAMC_CTX_T *p, u32 addr, u32 value, u32 mask, u32 pos);

/* dramc init prototypes */
DRAM_STATUS_T MemPllInit(DRAMC_CTX_T *p);
DRAM_STATUS_T MemPllPreInit(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcInit(DRAMC_CTX_T *p, EMI_SETTINGS *emi_set);
DRAM_STATUS_T DramcPreInit(DRAMC_CTX_T *p, EMI_SETTINGS *emi_set);

/* mandatory calibration function prototypes */
DRAM_STATUS_T DramcCATraining(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcRxdatlatCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcPllPhaseCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramCPllGroupsCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcRxdqsGatingCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcTxWindowPerbitCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcRxWindowPerbitCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcDualRankRxdatlatCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DualRankDramcRxdqsGatingCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcSwImpedanceCal(DRAMC_CTX_T *p, u8 apply);
DRAM_STATUS_T DramcWriteLeveling(DRAMC_CTX_T *p, EMI_SETTINGS *emi_set);

u32 DramcEngine2(DRAMC_CTX_T *p, DRAM_TE_OP_T wr, u32 test2_1, u32 test2_2,
		u8 testaudpat, s16 loopforever, u8 period,
		u8 log2loopcount);

u32 DramcDmaWaitCompare(DRAMC_CTX_T *p, u32 src_addr, u32 dst_addr,
			u32 trans_len, u8 burst_len);

void TxDelayForWriteLeveling(DRAMC_CTX_T *p,
			TXDQS_PERBIT_DLY_T *dqdqs_perbit_dly,
			u8 *ucave_dqdly_byte, u8 *ucmax_dqsdly_byte);

#endif /* _PI_API_H */
