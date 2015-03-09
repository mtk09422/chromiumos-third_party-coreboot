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

#include "dramc_register.h"
#include "emi.h"

enum {
	/* MEMPLL config */
	Fin = 52,

	PREDIV = 1,   /* pre-divider */
	POSDIV = 1    /* post-divider */
};

/* SSC / SYSPLL */
/* ====== Config ====== */
enum {
	SSC_SYSPLL_N = 56,
	DMSS_DIV = 0x7
};

/* init */
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
	/* gold pattern for dqsien compare */
	DEFAULT_GOLD_DQSIEN = 0x20202020
};

enum {
	DEFAULT_MR1_VALUE_DDR3 = 0x00002000,
	DEFAULT_MR2_VALUE_LP3 = 0x001c0002
};

/* timeout for TE2: (CMP_CPT_POLLING_PERIOD X MAX_CMP_CPT_WAIT_LOOP) */
/* complete flag */
enum {
	CMP_CPT_POLLING_PERIOD = 10,
	MAX_CMP_CPT_WAIT_LOOP = 10000   /* max loop */
};

enum {
	/* jitter meter for PLL phase calibration */
	fcJMETER_COUNT = 1024,
	/* 10us for more margin */
	fcJMETER_WAIT_DONE_US = (fcJMETER_COUNT/Fin+10)
};

enum {
	DQS_GW_COARSE_START = 10,
	DQS_GW_COARSE_END = 40,
	DQS_GW_COARSE_STEP = 1,

	DQS_GW_COARSE_MAX =
		((DQS_GW_COARSE_END-DQS_GW_COARSE_START)/DQS_GW_COARSE_STEP+1),

	DQS_GW_FINE_START = 0,
	DQS_GW_FINE_END = 127,
	DQS_GW_FINE_STEP = 8,
	DQS_GW_FINE_MAX =
		((DQS_GW_FINE_END-DQS_GW_FINE_START)/DQS_GW_FINE_STEP+1),
};

/* common */
/* define max support bus width in the system (to allocate array size) */
enum {
	DQ_DATA_WIDTH = 32,
	DQS_BIT_NUMBER = 8,
	DQS_NUMBER = (DQ_DATA_WIDTH / DQS_BIT_NUMBER)
};

/* RX DQ / DQS */
enum {
	MAX_RX_DQSDLY_TAPS = 64,     /* 0x018, May set back to 64 if no need. */
	MAX_RX_DQDLY_TAPS = 16       /* 0x210~0x22c, 0~15 delay tap */
};

/* TX DQ/DQS */
enum {
	FIRST_TX_DQ_DELAY = 0,    /* first step to DQ delay */
	FIRST_TX_DQS_DELAY = 0,   /* first step to DQS delay */
	MAX_TX_DQDLY_TAPS = 16,   /* max DQ TAP number */
	MAX_TX_DQSDLY_TAPS = 16   /* max DQS TAP number */
};

/* DRAM Driving */
enum {
	/* LPDDR2 driving setting steps */
	MAX_DRAM_DRIV_SET_LPDDR2 = 6,
	/* DDR3 driving setting steps */
	MAX_DRAM_DRIV_SET_DDR3 = 2,
	/* LPDDR2 MR3 default value */
	DEFAULT_LPDDR2_DRIV_MR3_VALUE = 0x00020003,
	/* DDR3 MR1 default value */
	DEFAULT_DDR3_DRIV_MR1_VALUE = 0x00002000
};

/*
 * Defines
 */
enum {
	ENABLE = 1,
	DISABLE = 0
};

typedef enum {
	DRAM_OK = 0, /* OK */
	DRAM_FAIL    /* FAIL */
} DRAM_STATUS_T; /* DRAM status type */

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

/* for A60808 DDR3 */
typedef enum {
	PCB_LOC_ASIDE = 0,
	PCB_LOC_BSIDE
} DRAM_PCB_LOC_T;

typedef enum {
	MODE_1X = 0,
	MODE_2X
} DRAM_DRAM_MODE_T;

typedef enum {
	PACKAGE_SBS = 0,
	PACKAGE_POP
} DRAM_PACKAGE_T;

typedef enum {
	TE_OP_WRITE_READ_CHECK = 0,
	TE_OP_READ_CHECK
} DRAM_TE_OP_T;

typedef enum {
	TEST_ISI_PATTERN = 0,
	TEST_AUDIO_PATTERN,
	TEST_TA1_SIMPLE,
	TEST_TESTPAT4,
	TEST_TESTPAT4_3,
	TEST_XTALK_PATTERN,
	TEST_MIX_PATTERN
} DRAM_TEST_PATTERN_T;

typedef enum {
	BL_TYPE_4 = 0,
	BL_TYPE_8
} DRAM_BL_TYPE_T;

typedef enum {
	DLINE_0 = 0,
	DLINE_1,
	DLINE_TOGGLE
} PLL_PHASE_CAL_STATUS_T;

typedef enum {
	TA43_OP_STOP,
	TA43_OP_CLEAR,
	TA43_OP_RUN,
	TA43_OP_RUNQUIET,
	TA43_OP_UNKNOWN,
} DRAM_TA43_OP_TYPE_T;

/* used for record last test pattern in TA */
typedef enum {
	TA_PATTERN_IDLE,
	TA_PATTERN_TA43,
	TA_PATTERN_TA4,
	TA_PATTERN_UNKNOWM,
} DRAM_TA_PATTERN_T;

typedef enum {
	DMA_OP_PURE_READ,
	DMA_OP_PURE_WRITE,
	DMA_OP_READ_WRITE,
} DRAM_DMA_OP_T;

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

/* dramc address */
static inline u32 mcSET_DRAMC_REG_ADDR(DRAMC_CTX_T *p, u32 offset)
{

	return ((p->channel << CH_INFO) | (offset));
}

/* #define mcSET_DRAMC_REG_ADDR(offset) ((p->channel << CH_INFO) | (offset)) */

/* basic function */
DRAM_STATUS_T DramcPreInit(DRAMC_CTX_T *p, EMI_SETTINGS *emi_set);
DRAM_STATUS_T DramcInit(DRAMC_CTX_T *p, EMI_SETTINGS *emi_set);
DRAMC_CTX_T *DramcCtxCreate(void);
void vDramcCtxDestroy(DRAMC_CTX_T *p);
void vDramcCtxInit(DRAMC_CTX_T *p);
DRAM_STATUS_T MemPllPreInit(DRAMC_CTX_T *p);
DRAM_STATUS_T MemPllInit(DRAMC_CTX_T *p);

u32 DramcEngine2(DRAMC_CTX_T *p, DRAM_TE_OP_T wr, u32 test2_1, u32 test2_2,
		 u8 testaudpat, s16 loopforever, u8 period,
		u8 log2loopcount);

u32 DramcGetMR2ByFreq(int dram_freq);
void DramcEnterSelfRefresh(DRAMC_CTX_T *p, u8 op);
void DramcLowFreqWrite(DRAMC_CTX_T *p);
void DramcPhyReset(DRAMC_CTX_T *p);
void DramcDiv2PhaseSync(DRAMC_CTX_T *p);
void DramcRunTimeConfig(DRAMC_CTX_T *p);
void DramcTestPat4_3(DRAMC_CTX_T *p, DRAM_TA43_OP_TYPE_T type);

void DramcWorstPat_mem(DRAMC_CTX_T *p, u32 src_addr);

u32 DramcWorstPat_dma(DRAMC_CTX_T *p, u32 src_addr, u32 dst_addr, u32 loop,
		      u8 check_result);

u32 DramcDmaEngineNoWait(DRAMC_CTX_T *p, DRAM_DMA_OP_T op, u32 src_addr,
			 u32 dst_addr, u32 trans_len, u8 burst_len);

u32 DramcDmaWaitCompare(DRAMC_CTX_T *p, u32 src_addr, u32 dst_addr,
			u32 trans_len, u8 burst_len);

void TxDelayForWriteLeveling(DRAMC_CTX_T *p,
			     TXDQS_PERBIT_DLY_T *dqdqs_perbit_dly,
				u8 *ucave_dqdly_byte, u8 *ucmax_dqsdly_byte);

void DramcRANKINCTLConfig(DRAMC_CTX_T *p);

/* mandatory calibration function */
DRAM_STATUS_T DramcPllPhaseCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramCPllGroupsCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcSwImpedanceCal(DRAMC_CTX_T *p, u8 apply);
DRAM_STATUS_T DramcHwImpedanceCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcWriteLeveling(DRAMC_CTX_T *p, EMI_SETTINGS *emi_set);
DRAM_STATUS_T DramcRxdqsGatingCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DualRankDramcRxdqsGatingCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcRxWindowPerbitCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcDualRankRxdatlatCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcRxdatlatCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcTxWindowPerbitCal(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcCATraining(DRAMC_CTX_T *p);
void DramcClkDutyCal(DRAMC_CTX_T *p);
void TransferToSPMControl(void);
void TransferToRegControl(void);

/* reference function */
DRAM_STATUS_T DramcRxEyeScan(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcTxEyeScan(DRAMC_CTX_T *p);

extern unsigned int uiDualRank;
extern u8 CurrentRank;
extern u8 RXPERBIT_LOG_PRINT;
extern u8 GATINGDQS_LOG_PRINT;

/* Testing */
void Dump_Registers(DRAMC_CTX_T *p);
DRAM_STATUS_T DramcRegDump(DRAMC_CTX_T *p);

/* API prototypes. */
inline u8 ucDram_Register_Read(u32 u4reg_addr, u32 *pu4reg_value);
inline u8 ucDram_Register_Write(u32 u4reg_addr, u32 u4reg_value);
inline u8 ucDramC_Register_Read(u32 u4reg_addr, u32 *pu4reg_value);
inline u8 ucDramC_Register_Write(u32 u4reg_addr, u32 u4reg_value);
void DramcDmaEngine_Config(DRAMC_CTX_T *p);
void DramcGeneralPat_mem(DRAMC_CTX_T *p, u32 src_addr);

inline u8 RegisterWriteVal(DRAMC_CTX_T *p, u32 addr, u32 val);
inline u8 RegisterSetField(DRAMC_CTX_T *p, u32 addr, u32 value,
				u32 mask, u32 pos);
inline u8 RegisterSetBit(DRAMC_CTX_T *p, u32 addr, u32 bit);
inline u8 RegisterClrBit(DRAMC_CTX_T *p, u32 addr, u32 bit);
#endif /* _PI_API_H */
