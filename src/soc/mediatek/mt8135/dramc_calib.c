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
#include <console/console.h>

#include <soc/mt8135.h>
#include <soc/dramc.h>
#include <soc/emi_hw.h>
#include <soc/emi.h>

#if CONFIG_DEBUG_MEMORY_DRAM
#define DEBUG_DRAM_CALIB(x...)		printk(BIOS_INFO, "DRAM_CALIB: " x)
#else
#define DEBUG_DRAM_CALIB(x...)
#endif	/* CONFIG_DEBUG_MEMORY_DRAM */

/*
 * configuration:
 * fcTX_PER_BIT_DESKEW
 */

unsigned int opt_rx_dqs0;
unsigned int opt_rx_dqs1;
unsigned int opt_tx_dq[4];
unsigned int opt_tx_dqs;
unsigned int opt_tx_dqm;

typedef struct _RXDQS_PERBIT_DLY_T {
	s8 min_cur;
	s8 max_cur;
	s8 min_best;
	s8 max_best;
	u8 center;
	u8 dq_dly_last;
} RXDQS_PERBIT_DLY_T;

#define DQ_DATA_WIDTH 32
#define MAX_RX_DQSDLY_TAPS 128
#define MAX_RX_DQDLY_TAPS 16
#define DQS_NUMBER 4
#define DQS_BIT_NUMBER (DQ_DATA_WIDTH/DQS_NUMBER)

/* Description
 *	RX DQ/DQS per bit calibration.
 * Registers
 *	- DQIDLY[1:8] : one register had 4 settings (4bits: 0~15, unit 20ps)
 *	      with corresponding DQx
 *	- R0DELDLY : 4 settings for rank 0 DQS0~DQS3. 7 bits (0~127) with unit
 *	      30ps.
 *	- R1DELDLY : 4 settings for rank 1 DQS0~DQS3. 7 bits (0~127) with unit
 *	      30ps.
 * Algorithm
 *	- Set DQS/DQ input delay to 0.
 *	- Delay all DQs from 0 to 15 until all failed.
 *	- Delay DQSs from 0 to 127 to find the pass range (min & max) of each
 *	  DQ. Further find the largest pass range.
 *	- For each DQS, find the largest middle value of corresponding DQ byte.
 *	  Then use this value to set each DQS input delay.
 *	- For each DQ, find the difference between original middle DQS delay
 *	  and max DQS delay per byte. Then delay the difference more to align
 *	  the middle of DQS per byte.
 */
static int do_sw_rx_dq_dqs_calib(void)
{
	int result;
	unsigned int data;
	unsigned int test_len = 0x100;
	unsigned int i, j;
	unsigned int dqs_input_delay;
	unsigned int cmp_err;
	unsigned int max;

	unsigned int dq_dly_max;
	char dqs_delay[DQS_NUMBER];
	char dq_delay_per_bit[DQ_DATA_WIDTH];
	unsigned int dqidly[DQ_DATA_WIDTH / DQS_NUMBER];
	unsigned int dq_tap;
	unsigned int dq_delay_done[DQ_DATA_WIDTH];
	RXDQS_PERBIT_DLY_T dqs_perbit_dly[DQ_DATA_WIDTH];

	result = 0;

	DEBUG_DRAM_CALIB("in %s\n", __func__);
	DEBUG_DRAM_CALIB("*DQIDLY1 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY1));
	DEBUG_DRAM_CALIB("*DQIDLY2 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY2));
	DEBUG_DRAM_CALIB("*DQIDLY3 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY3));
	DEBUG_DRAM_CALIB("*DQIDLY4 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY4));
	DEBUG_DRAM_CALIB("*DQIDLY5 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY5));
	DEBUG_DRAM_CALIB("*DQIDLY6 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY6));
	DEBUG_DRAM_CALIB("*DQIDLY7 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY7));
	DEBUG_DRAM_CALIB("*DQIDLY8 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY8));
	DEBUG_DRAM_CALIB("*DRAMC_R0DELDLY = 0x%x\n",
			 DRAMC_READ_REG(DRAMC_R0DELDLY));

	/* 1. set DQS delay to 0 first */
	DRAMC_WRITE_REG(0x0, DRAMC_R0DELDLY);

	/* set DQ delay to 0x0. */
	for (i = 0; i < 8; i++)
		DRAMC_WRITE_REG(0x0, DRAMC_DQIDLY1 + 4 * i);

	/* set DQ delay structure to 0x0. */
	for (i = 0; i < DQ_DATA_WIDTH; i++) {
		dq_delay_per_bit[i] = 0x0;
		dq_delay_done[i] = 0x0;
	}

	/* delay DQ to find all failed */
	for (dq_tap = 0; dq_tap < MAX_RX_DQDLY_TAPS; dq_tap++) {
		/* set test patern length */
		DRAMC_WRITE_REG(0x55000000, 0x3C);

		data = DRAMC_READ_REG(0x40);
		DRAMC_WRITE_REG((data & 0x00ffffff) | 0xAA000000 | test_len,
				0x40);
		/* Test Agent 2 write enabling, Test Agent 2 read enabling */
		if (dq_tap == 0)
			DEBUG_DRAM_CALIB("%s Test Agent 2 enable\n", __func__);

		DRAMC_WRITE_SET((1 << 30) | (1 << 31), DRAMC_CONF2);

		while (!(DRAMC_READ_REG(DRAMC_TESTRPT) & (1 << 10)));

		delay_a_while(400);

		if (dq_tap == 0)
			DEBUG_DRAM_CALIB("%s Test Agent 2 Done\n", __func__);

		cmp_err = DRAMC_READ_REG(DRAMC_CMP_ERR);
		DEBUG_DRAM_CALIB("cmp_err:%x\n", cmp_err);
		/* disable test agent2 r/w */
		DRAMC_WRITE_CLEAR(((1 << 30) | (1 << 31)), DRAMC_CONF2);
		if (cmp_err == 0xFFFFFFFF)
			break;

		/* Bit i compare result
		 *     - Compare success & never fail before, record the delay
		 *       value. (dq_delay_per_bit[i] = delay value)
		 *     - Compare fail. Record fail. (dq_delay_done[i] = 1)
		 */

		for (i = 0; i < DQ_DATA_WIDTH; i++) {
			if (!(cmp_err & (0x1 << i)) && dq_delay_done[i] == 0)
				dq_delay_per_bit[i] = dq_tap;
			else
				dq_delay_done[i] = 1;

			DEBUG_DRAM_CALIB("%d)0x%x\n", i, dq_delay_per_bit[i]);
		}

		DEBUG_DRAM_CALIB("\n");

		for (i = 0; i < DQ_DATA_WIDTH; i += 4) {
			dqidly[i / 4] =
			    (dq_delay_per_bit[i]) +
			    (dq_delay_per_bit[i + 1] << 8) +
			    (dq_delay_per_bit[i + 2] << 16) +
			    (dq_delay_per_bit[i + 3] << 24);

			DEBUG_DRAM_CALIB("dqidly[%d]=0x%x\n", i / 4,
					 dqidly[i / 4]);
		}

		for (i = 0; i < 8; i++)
			DRAMC_WRITE_REG(dqidly[i], DRAMC_DQIDLY1 + 4 * i);

		DEBUG_DRAM_CALIB("*DQIDLY1 = 0x%x\n",
				 DRAMC_READ_REG(DRAMC_DQIDLY1));
		DEBUG_DRAM_CALIB("*DQIDLY2 = 0x%x\n",
				 DRAMC_READ_REG(DRAMC_DQIDLY2));
		DEBUG_DRAM_CALIB("*DQIDLY3 = 0x%x\n",
				 DRAMC_READ_REG(DRAMC_DQIDLY3));
		DEBUG_DRAM_CALIB("*DQIDLY4 = 0x%x\n",
				 DRAMC_READ_REG(DRAMC_DQIDLY4));
		DEBUG_DRAM_CALIB("*DQIDLY5 = 0x%x\n",
				 DRAMC_READ_REG(DRAMC_DQIDLY5));
		DEBUG_DRAM_CALIB("*DQIDLY6 = 0x%x\n",
				 DRAMC_READ_REG(DRAMC_DQIDLY6));
		DEBUG_DRAM_CALIB("*DQIDLY7 = 0x%x\n",
				 DRAMC_READ_REG(DRAMC_DQIDLY7));
		DEBUG_DRAM_CALIB("*DQIDLY8 = 0x%x\n",
				 DRAMC_READ_REG(DRAMC_DQIDLY8));

	}

	/* After loop, dq_delay_per_bit[0:31] value non-zero mean the last
	 * valid settings when DQS input delay is 0. dq_delay_per_bit[0:31]
	 * value 0 means it is already  failed when DQS input delay is 0.
	 * Also, current DQIDLY[1:8] settings is the setting of
	 * dq_delay_per_bit[0:31].
	 * We got the dq input delay in dq_delay_per_bit[i]
	 */

	/* 2. initialize parameters */
	for (i = 0; i < DQ_DATA_WIDTH; i++) {
		dqs_perbit_dly[i].min_cur = -1;
		dqs_perbit_dly[i].max_cur = -1;
		dqs_perbit_dly[i].min_best = -1;
		dqs_perbit_dly[i].max_best = -1;
		dqs_perbit_dly[i].center = 0;
		dqs_perbit_dly[i].dq_dly_last = dq_delay_per_bit[i];
	}
	/* find the minimum and maximum DQS input delay */
	for (i = 0; i < MAX_RX_DQSDLY_TAPS; i++) {
		dqs_input_delay = (i) + (i << 8) + (i << 16) + (i << 24);
		DRAMC_WRITE_REG(dqs_input_delay, DRAMC_R0DELDLY);

		/* set test patern length */
		data = DRAMC_READ_REG(0x40);
		DRAMC_WRITE_REG((data & 0xFF000000) | test_len, 0x40);
		/* Test Agent 2 write enabling, Test Agent 2 read enabling */
		DRAMC_WRITE_SET((1 << 30) | (1 << 31), DRAMC_CONF2);

		while (!(DRAMC_READ_REG(DRAMC_TESTRPT) & (1 << 10)));

		delay_a_while(400);

		cmp_err = DRAMC_READ_REG(DRAMC_CMP_ERR);
		/* disable test agent2 r/w */
		DRAMC_WRITE_CLEAR(((1 << 30) | (1 << 31)), DRAMC_CONF2);

		/* If bit x test pass the first time, record to min input delay
		 *   (dqs_per_bit[x].min_cur = delay value.)
		 * If bit x already had min value and no max value and pass
		 *   fail => max value is this delay-1. (dqs_per_bit[x].max_cur
		 *   = delay value-1)
		 * If bit x already had min value and no max value and pass and
		 *   delay value = 127 => max value = 127
		 *   (dqs_per_bit[x].max_cur = 127)
		 */

		for (j = 0; j < DQ_DATA_WIDTH; j++) {
			if ((dqs_perbit_dly[j].min_cur == -1)
			    && ((cmp_err & ((u32)1 << j)) == 0x0))
				/* min pass delay */
				dqs_perbit_dly[j].min_cur = i;

			if ((dqs_perbit_dly[j].min_cur != -1)
			    && (dqs_perbit_dly[j].max_cur == -1)
			    && (((cmp_err & ((u32)1 << j)) != 0x0)
				|| (i == (MAX_RX_DQSDLY_TAPS - 1)))) {
				/* we get the dqs_perbit_dly pass max */
				if ((i == (MAX_RX_DQSDLY_TAPS - 1))
				    && ((cmp_err & ((u32)1 << j)) == 0x0))
					dqs_perbit_dly[j].max_cur =
					    MAX_RX_DQSDLY_TAPS - 1;
				else
					dqs_perbit_dly[j].max_cur = i - 1;

				/* there may be more than 1 pass range, find
				 * the max range
				 * ex: x00xxxxxx00000000000000xx...
				 * (get the second one)
				 */
				if ((dqs_perbit_dly[j].max_cur -
				     dqs_perbit_dly[j].min_cur) >
				    (dqs_perbit_dly[j].max_best -
				     dqs_perbit_dly[j].min_best)) {
					dqs_perbit_dly[j].max_best =
					    dqs_perbit_dly[j].max_cur;
					dqs_perbit_dly[j].min_best =
					    dqs_perbit_dly[j].min_cur;
				}
				/* clear to find the next pass range if any */
				dqs_perbit_dly[j].max_cur = -1;
				dqs_perbit_dly[j].min_cur = -1;
			}

		}
	}

	/* 3. get dqs delay center per bit */
	for (j = 0; j < DQ_DATA_WIDTH; j++) {
		if ((dqs_perbit_dly[j].max_best != -1)
		    && (dqs_perbit_dly[j].min_best != -1)) {
			dqs_perbit_dly[j].center =
			    (dqs_perbit_dly[j].max_best +
			     dqs_perbit_dly[j].min_best) / 2;
			DEBUG_DRAM_CALIB("dqs_perbit_dly[%d].center=0x%x\n",
					 j, dqs_perbit_dly[j].center);
		}
	}

	/* we get the delay value of the 4 DQS (min of center) */
	for (i = 0; i < DQS_NUMBER; i++) {
		max = 0;
		/* find the max of center */
		for (j = 0; j < DQS_BIT_NUMBER; j++) {
			uint32_t index = i * DQS_BIT_NUMBER + j;

			if (dqs_perbit_dly[index].center > max)
				max = dqs_perbit_dly[index].center;
		}
		/* save dqs delay */
		dqs_delay[i] = max;
		DEBUG_DRAM_CALIB("dqs_delay[%d]=0x%x\n", i, max);
	}
	data = ((u32)dqs_delay[0]) +
	       (((u32)dqs_delay[1]) << 8) +
	       (((u32)dqs_delay[2]) << 16) +
	       (((u32)dqs_delay[3]) << 24);
	/* set dqs input delay */
	DRAMC_WRITE_REG(data, DRAMC_R0DELDLY);
	DRAMC_WRITE_REG(data, DRAMC_R1DELDLY);

	/* delay DQ ,let dqsdly_ok_center == DQS_delay */
	for (i = 0; i < DQ_DATA_WIDTH; i = i + 4) {
		/* every 4-bit dq have the same delay register address */
		/* dq_dly_max: taps for dq delay to be add */
		for (j = 0; j < 4; j++) {
			uint32_t index = i / DQS_BIT_NUMBER;

			dq_dly_max = dqs_delay[index] -
					dqs_perbit_dly[i + j].center;

			DEBUG_DRAM_CALIB(
			    "1.bit:%d)dq_per_bit_dly:0x%x,dq_dly:0x%x\n",
			    i + j, dqs_perbit_dly[i + j].dq_dly_last,
			    dq_dly_max);

			data = dqs_perbit_dly[i + j].dq_dly_last + dq_dly_max;

			data = ((data > (MAX_RX_DQDLY_TAPS - 1)) ?
				(MAX_RX_DQDLY_TAPS - 1) : data);

			dqs_perbit_dly[i + j].dq_dly_last = data;

			DEBUG_DRAM_CALIB("2.bit:%d)dq_per_bit_dly:0x%x\n",
					 i + j,
					dqs_perbit_dly[i + j].dq_dly_last);
		}

		data = ((u32)dqs_perbit_dly[i].dq_dly_last) +
		       (((u32)dqs_perbit_dly[i + 1].dq_dly_last) << 8) +
		       (((u32)dqs_perbit_dly[i + 2].dq_dly_last) << 16) +
		       (((u32)dqs_perbit_dly[i + 3].dq_dly_last) << 24);

		DRAMC_WRITE_REG(data, DRAMC_DQIDLY1 + i);
	}

	for (j = 0; j < DQ_DATA_WIDTH; j++)
		DEBUG_DRAM_CALIB("%d)min:0x%x,max:0x%x\n",
				 j, dqs_perbit_dly[j].min_best,
					dqs_perbit_dly[j].max_best);

	DEBUG_DRAM_CALIB("*DQIDLY1 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY1));
	DEBUG_DRAM_CALIB("*DQIDLY2 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY2));
	DEBUG_DRAM_CALIB("*DQIDLY3 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY3));
	DEBUG_DRAM_CALIB("*DQIDLY4 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY4));
	DEBUG_DRAM_CALIB("*DQIDLY5 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY5));
	DEBUG_DRAM_CALIB("*DQIDLY6 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY6));
	DEBUG_DRAM_CALIB("*DQIDLY7 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY7));
	DEBUG_DRAM_CALIB("*DQIDLY8 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY8));
	DEBUG_DRAM_CALIB("*DRAMC_R0DELDLY = 0x%x\n",
			 DRAMC_READ_REG(DRAMC_R0DELDLY));

	DEBUG_DRAM_CALIB("*DQIDLY1 = 0x%x\n", DRAMC_READ_REG(DRAMC_DQIDLY1));

	/* finish we can put result now . */

	DEBUG_DRAM_CALIB("=================================");
	DEBUG_DRAM_CALIB("=================================\n");

	DEBUG_DRAM_CALIB("	RX	DQS perbit ");
	DEBUG_DRAM_CALIB("delay software calibration\n");

	DEBUG_DRAM_CALIB("=================================");
	DEBUG_DRAM_CALIB("=================================\n");

	DEBUG_DRAM_CALIB("1.0-31 bit dq delay value\n");

	DEBUG_DRAM_CALIB("=================================");
	DEBUG_DRAM_CALIB("=================================\n");

	DEBUG_DRAM_CALIB("bit|     0  1  2  3  4  5  6  7  8  9\n");

	DEBUG_DRAM_CALIB("--------------------------------------");

	for (i = 0; i < DQ_DATA_WIDTH; i++) {
		j = i / 10;
		if (i == (j * 10)) {
			DEBUG_DRAM_CALIB("\n");
			DEBUG_DRAM_CALIB("%d |    ", i);
		}
		DEBUG_DRAM_CALIB("%d ", dq_delay_per_bit[i]);
	}

	DEBUG_DRAM_CALIB("\n--------------------------------------\n\n");
	DEBUG_DRAM_CALIB("=================================");
	DEBUG_DRAM_CALIB("=================================\n");

	DEBUG_DRAM_CALIB("2.dqs window\n");
	DEBUG_DRAM_CALIB("x=pass dqs delay value (min~max)center\n");
	DEBUG_DRAM_CALIB("y=0-7bit DQ of every group\n");

	DEBUG_DRAM_CALIB("input delay:DQS0 =%d DQS1 = %d DQS2 =%d DQS3 = %d\n",
			 dqs_delay[0], dqs_delay[1], dqs_delay[2], dqs_delay[3]);

	DEBUG_DRAM_CALIB("=================================");
	DEBUG_DRAM_CALIB("=================================\n");
	DEBUG_DRAM_CALIB("bit  DQS0     bit      DQS1     ");
	DEBUG_DRAM_CALIB("bit     DQS2     bit     DQS3\n");

	for (i = 0; i < DQS_BIT_NUMBER; i++) {
		DEBUG_DRAM_CALIB(
		"%d  (%d~%d)%d  %d  (%d~%d)%d  %d  (%d~%d)%d  %d  (%d~%d)%d\n",
			i, dqs_perbit_dly[i].min_best,
			dqs_perbit_dly[i].max_best, dqs_perbit_dly[i].center,
			i + 8, dqs_perbit_dly[i + 8].min_best,
			dqs_perbit_dly[i + 8].max_best,
			dqs_perbit_dly[i + 8].center, i + 16,
			dqs_perbit_dly[i + 16].min_best,
			dqs_perbit_dly[i + 16].max_best,
			dqs_perbit_dly[i + 16].center, i + 24,
			dqs_perbit_dly[i + 24].min_best,
			dqs_perbit_dly[i + 24].max_best,
			dqs_perbit_dly[i + 24].center);
	}

	DEBUG_DRAM_CALIB("=================================");
	DEBUG_DRAM_CALIB("=================================\n");

	DEBUG_DRAM_CALIB("3.dq delay value last\n");

	DEBUG_DRAM_CALIB("=================================");
	DEBUG_DRAM_CALIB("=================================\n");

	DEBUG_DRAM_CALIB("bit|    0  1  2  3  4  5  6  7  8   9\n");

	DEBUG_DRAM_CALIB("--------------------------------------");

	for (i = 0; i < DQ_DATA_WIDTH; i++) {
		j = i / 10;
		if (i == (j * 10)) {
			DEBUG_DRAM_CALIB("\n");
			DEBUG_DRAM_CALIB("%d |    ", i);
		}
		DEBUG_DRAM_CALIB("%d ", dqs_perbit_dly[i].dq_dly_last);
	}

	DEBUG_DRAM_CALIB("=================================");
	DEBUG_DRAM_CALIB("=================================\n");

	return 0;
}

typedef struct _TXDQS_PERBYTE_DLY_T {
	s8 first_dqdly_pass;
	s8 last_dqdly_pass;
	s8 total_dqdly_pass;
	s8 first_dqsdly_pass;
	s8 last_dqsdly_pass;
	s8 total_dqsdly_pass;
	u8 best_dqdly;
	u8 best_dqsdly;
	u8 dq;
	u8 dqs;
} TXDQS_PERBYTE_DLY_T;

enum {
	MAX_TX_DQDLY_TAPS = 16,
	MAX_TX_DQSDLY_TAPS = 16
};

/*
 * For 3PLL. This value is obtained by measurement of waveform for the
 * relationship between DQS and CLK.
 */
enum {
	TX_STEPS = 0xd,
	TX_STEPS_DQ = 0xb
};

static void set_dqs_dq_dly(TXDQS_PERBYTE_DLY_T *dqs_perbit_dly, int j,
			   int dqs_num)
{
	s8 dqdqs_pass_diff = dqs_perbit_dly[j].total_dqdly_pass -
				dqs_perbit_dly[j].total_dqsdly_pass;

	if ((DRAMC_READ_REG(DRAMC_MCKDLY & 0x80000000)) == 0x0) {
		dqs_perbit_dly[j].best_dqsdly = 0 + TX_STEPS;
		dqs_perbit_dly[j].best_dqdly += (dqdqs_pass_diff) / 2
						+ TX_STEPS_DQ;
	} else {
		if ((dqs_num == 1) || (dqs_num == 2)) {

			/* TX_STEPS = 0x0A for x8 board */
			/* TX_STEPS_DQ = 0x0A for x8 board */
			dqs_perbit_dly[j].best_dqsdly = 0 + 0x0A;
			dqs_perbit_dly[j].best_dqdly += (dqdqs_pass_diff) / 2
							+ 0x0A;
		} else {
			/* TX_STEPS = 0x0A for x8 board */
			/* TX_STEPS_DQ = 0x10 for x8 board */
			dqs_perbit_dly[j].best_dqsdly = 0 + 0x0C;
			dqs_perbit_dly[j].best_dqdly += (dqdqs_pass_diff) / 2
							+ 0x0F;
		}

		if (dqs_perbit_dly[j].best_dqdly > 0xf) {

			DEBUG_DRAM_CALIB("warning 3:bit %d,best_dqdly:%x\n",
					 j, dqs_perbit_dly[j].best_dqdly);
			DEBUG_DRAM_CALIB("warning , please use ECO IC\n");
		}
	}
}

/* [FIXME] need to rewrite this function */
static int do_sw_tx_dq_dqs_calib(void)
{
	TXDQS_PERBYTE_DLY_T dqs_perbit_dly[DQ_DATA_WIDTH];
	unsigned int data;

	unsigned int mask;
	unsigned int DQM_data;
	unsigned int DQM_mask;
	unsigned int test_len = 0x3FF;
	unsigned int cmp_err_1;
	unsigned int fail_bit;

	unsigned int finish_count;
	unsigned int bit_first;
	unsigned int bit_last;
	unsigned int max_dqsdly_byte[DQS_NUMBER];
	unsigned int ave_dqdly_byte[DQS_NUMBER];
	int i;
	int j;
	int k;
	int m;

	DEBUG_DRAM_CALIB("in %s\n", __func__);

	for (i = 0; i < DQ_DATA_WIDTH; i++) {
		dqs_perbit_dly[i].first_dqdly_pass = 0;
		dqs_perbit_dly[i].last_dqdly_pass = -1;
		dqs_perbit_dly[i].first_dqsdly_pass = 0;
		dqs_perbit_dly[i].last_dqsdly_pass = -1;
		dqs_perbit_dly[i].best_dqdly = 0;
		dqs_perbit_dly[i].best_dqsdly = 0;
		dqs_perbit_dly[i].total_dqdly_pass = 0;
		dqs_perbit_dly[i].total_dqsdly_pass = 0;
	}
	/* used for early break */
	finish_count = 0;

	/* 1. Tx DQS/DQ all zero */
	/* 1.1 DQM */
	DQM_data = DRAMC_READ_REG(DRAMC_PADCTL2);
	DQM_mask = (0xFFFF0000);
	DQM_mask = DQM_data & DQM_mask;
	DRAMC_WRITE_REG(DQM_mask, DRAMC_PADCTL2);

	/* 1.2 DQ */
	for (i = 0; i < 4; i++)
		DRAMC_WRITE_REG(0x0, (DRAMC_DQODLY1 + 4 * i));

	/* 2. fix DQ delay = 0, delay DQS to find the pass range  */
	for (i = MAX_TX_DQSDLY_TAPS - 1; i >= 0; i--) {
		data = DRAMC_READ_REG(DRAMC_PADCTL3);
		mask = 0xFFFF0000;
		data = (data & mask) |
		       ((i << 0) | (i << 4) | (i << 8) | (i << 12));
		DRAMC_WRITE_REG(data, DRAMC_PADCTL3);

		/* 2.2 use test agent to find the pass range */
		/* set test patern length */
		DRAMC_WRITE_REG(0x55000000, 0x3C);
		data = DRAMC_READ_REG(0x40);
		DRAMC_WRITE_REG((data & 0x00ffffff) | 0xAA000000 | test_len,
				0x40);

		/* Test Agent 2 write enabling */
		/* enable write */
		DRAMC_WRITE_SET((1 << 31), DRAMC_CONF2);
		/* enable read */
		DRAMC_WRITE_SET((1 << 30) | (1 << 31), DRAMC_CONF2);
		while (!(DRAMC_READ_REG(DRAMC_TESTRPT) & (1 << 10)));
		delay_a_while(400);
		cmp_err_1 = DRAMC_READ_REG(DRAMC_CMP_ERR);
		/* disable test agent2 r/w */
		DRAMC_WRITE_CLEAR(((1 << 30) | (1 << 31)), DRAMC_CONF2);
		DEBUG_DRAM_CALIB("DQS loop = %d, cmp_err_1 = %x\n",
				 i, cmp_err_1);

		for (j = 0; j < DQ_DATA_WIDTH; j++) {
			fail_bit = cmp_err_1 & ((unsigned int)1 << j);

			if (dqs_perbit_dly[j].last_dqsdly_pass == -1) {
				if (fail_bit == 0) {
					dqs_perbit_dly[j].last_dqsdly_pass = i;
					finish_count++;
					DEBUG_DRAM_CALIB("dqs_perbit_dly.last_dqsdly_pass[%d]=%d, "
							"finish count=%d\n",
							j,
							dqs_perbit_dly[j].last_dqsdly_pass,
							finish_count);
				}
			}
		}

		if (finish_count == DQ_DATA_WIDTH)
			break;
	}

	/* 3. fix DQS delay = 0, delay DQ to find the pass range  */
	DRAMC_WRITE_CLEAR(0xFFFF, DRAMC_PADCTL3);

	finish_count = 0;

	for (i = MAX_TX_DQDLY_TAPS - 1; i >= 0; i--) {
		/* 3.1 delay DQ output delay */
		/*
		 * DQM0DLY: DRAMC_PADCTL2[0:3], 4 bits
		 * DQM1DLY: DRAMC_PADCTL2[4:7], 4 bits
		 * DQM2DLY: DRAMC_PADCTL2[8:11], 4 bits
		 */
		data = ((i & 0xF) << 0)
		       | ((i & 0xF) << 4)
		       | ((i & 0xF) << 8)
		       | ((i & 0xF) << 12)
		       | (DQM_mask);
		DRAMC_WRITE_REG(data, DRAMC_PADCTL2 /* 0x10 */);
		/*
		 * DQ0DLY: DRAMC_DQODLY1[0:3], 4 bits
		 * DQ1DLY: DRAMC_DQODLY1[4:7], 4 bits
		 * DQ2DLY: DRAMC_DQODLY1[8:11], 4 bits
		 * DQ3DLY: DRAMC_DQODLY1[12:15], 4 bits
		 * DQ4DLY: DRAMC_DQODLY1[16:19], 4 bits
		 * DQ5DLY: DRAMC_DQODLY1[20:23], 4 bits
		 * DQ6DLY: DRAMC_DQODLY1[24:27], 4 bits
		 */
		data = ((i & 0xF) << 0)
		       | ((i & 0xF) << 4)
		       | ((i & 0xF) << 8)
		       | ((i & 0xF) << 12)
		       | ((i & 0xF) << 16)
		       | ((i & 0xF) << 20)
		       | ((i & 0xF) << 24)
		       | ((i & 0xF) << 28);
		DRAMC_WRITE_REG(data, DRAMC_DQODLY1);
		DRAMC_WRITE_REG(data, DRAMC_DQODLY2);
		DRAMC_WRITE_REG(data, DRAMC_DQODLY3);
		DRAMC_WRITE_REG(data, DRAMC_DQODLY4);

		/* 3.2 use test agent to find the pass range */
		/* set test patern length */
		DRAMC_WRITE_REG(0x55000000, 0x3C);
		data = DRAMC_READ_REG(0x40);
		DRAMC_WRITE_REG((data & 0x00ffffff) | 0xAA000000 | test_len,
				0x40);
		/* Test Agent 2 write enabling */
		/* enable write */
		DRAMC_WRITE_SET((1 << 31), DRAMC_CONF2);
		/* enable read */
		DRAMC_WRITE_SET((1 << 30) | (1 << 31), DRAMC_CONF2);
		while (!(DRAMC_READ_REG(DRAMC_TESTRPT) & (1 << 10)));
		delay_a_while(400);
		cmp_err_1 = DRAMC_READ_REG(DRAMC_CMP_ERR);
		DEBUG_DRAM_CALIB("DQ loop=%d, cmp_err_1 = %x\n", i, cmp_err_1);
		/* disable test agent2 r/w */
		DRAMC_WRITE_CLEAR(((1 << 30) | (1 << 31)), DRAMC_CONF2);

		for (j = 0; j < DQ_DATA_WIDTH; j++) {
			fail_bit = cmp_err_1 & ((unsigned int)1 << j);

			if (dqs_perbit_dly[j].last_dqdly_pass == -1) {
				if (fail_bit == 0) {
					dqs_perbit_dly[j].last_dqdly_pass = i;
					finish_count++;

					const char *str1 = "dqs_perbit_dly";
					const char *str2 = "last_dqdly_pass";
					const char *str3 = "finish count";

					DEBUG_DRAM_CALIB("%s.%s[%d]=%d,  ",
							 str1, str2, j,
							dqs_perbit_dly[j]
							.last_dqdly_pass);

					DEBUG_DRAM_CALIB("%s=%d\n", str3,
							 finish_count);

					UNUSED_VAR(str1);
					UNUSED_VAR(str2);
					UNUSED_VAR(str3);
				}
			}
		}

		if (finish_count == DQ_DATA_WIDTH)
			break;
	}
	if ((DRAMC_READ_REG(DRAMC_MCKDLY) & 0x80000000) != 0x0) {
		DEBUG_DRAM_CALIB("x8  TX 4-bit swap\n");
		for (j = 0; j < 4; j++) {
			k = dqs_perbit_dly[4 + j].last_dqsdly_pass;
			m = dqs_perbit_dly[4 + j].last_dqdly_pass;
			dqs_perbit_dly[4 + j].last_dqsdly_pass =
			    dqs_perbit_dly[8 + j].last_dqsdly_pass;
			dqs_perbit_dly[4 + j].last_dqdly_pass =
			    dqs_perbit_dly[8 + j].last_dqdly_pass;
			dqs_perbit_dly[8 + j].last_dqsdly_pass = k;
			dqs_perbit_dly[8 + j].last_dqdly_pass = m;

			DEBUG_DRAM_CALIB("dqs_perbit_dly.last_dqsdly_pass[%d]=%d , "
					"dqs_perbit_dly.last_dqdly_pass =%d\n",
					4 + j,
					dqs_perbit_dly[4 + j].last_dqsdly_pass,
					dqs_perbit_dly[4 + j].last_dqdly_pass);

			DEBUG_DRAM_CALIB("dqs_perbit_dly.last_dqsdly_pass[%d]=%d , "
					"dqs_perbit_dly.lastdqdly_pass =%d\n",
					8 + j,
					dqs_perbit_dly[8 + j].last_dqsdly_pass,
					dqs_perbit_dly[8 + j].last_dqdly_pass);
		}
		for (j = 0; j < 4; j++) {
			k = dqs_perbit_dly[20 + j].last_dqsdly_pass;
			m = dqs_perbit_dly[20 + j].last_dqdly_pass;
			dqs_perbit_dly[20 + j].last_dqsdly_pass =
			    dqs_perbit_dly[24 + j].last_dqsdly_pass;
			dqs_perbit_dly[20 + j].last_dqdly_pass =
			    dqs_perbit_dly[24 + j].last_dqdly_pass;
			dqs_perbit_dly[24 + j].last_dqsdly_pass = k;
			dqs_perbit_dly[24 + j].last_dqdly_pass = m;

			DEBUG_DRAM_CALIB("dqs_perbit_dly.last_dqsdly_pass[%d]=%d , "
					"dqs_perbit_dly.last_dqdly_pass =%d\n",
					20 + j,
					dqs_perbit_dly[20 + j].last_dqsdly_pass,
					dqs_perbit_dly[20 + j].last_dqdly_pass);

			DEBUG_DRAM_CALIB("dqs_perbit_dly.last_dqsdly_pass[%d]=%d , "
					"dqs_perbit_dly.last_dqdly_pass =%d\n",
					24 + j,
					dqs_perbit_dly[24 + j].last_dqsdly_pass,
					dqs_perbit_dly[24 + j].last_dqdly_pass);
		}
	}

	for (i = 0; i < DQS_NUMBER; i++) {
		bit_first = DQS_BIT_NUMBER * i;
		bit_last = DQS_BIT_NUMBER * i + (DQS_BIT_NUMBER - 1);
		max_dqsdly_byte[i] = TX_STEPS;
		/* init accumulation variable to 0 */
		ave_dqdly_byte[i] = 0;	/* for DQM */

		for (j = bit_first; j <= bit_last; j++) {
			dqs_perbit_dly[j].total_dqsdly_pass =
			    dqs_perbit_dly[j].last_dqsdly_pass -
			    dqs_perbit_dly[j].first_dqsdly_pass + 1;
			dqs_perbit_dly[j].total_dqdly_pass =
			    dqs_perbit_dly[j].last_dqdly_pass -
			    dqs_perbit_dly[j].first_dqdly_pass + 1;

			/*
			 * 4. find the middle of the pass range of DQ and DQS
			 */

			/* 5. if the middle of the pass range is in the DQ,
			 *    the delay of (DQS,DQ) is (0,DQ), if the middle of
			 *    the pass range is in the DQS the delay of (DQS,DQ)
			 *    is (DQS,0)
			 */
			u8 dq_dqs_pass_diff = dqs_perbit_dly[j].total_dqdly_pass
					- dqs_perbit_dly[j].total_dqsdly_pass;

			if (dq_dqs_pass_diff == 0) {
				dqs_perbit_dly[j].best_dqsdly = TX_STEPS;
				dqs_perbit_dly[j].best_dqdly = TX_STEPS_DQ;

			} else if (dq_dqs_pass_diff > 0) {

				/* sub-routine to set dqs & dq dly */
				set_dqs_dq_dly(dqs_perbit_dly, j, i);

			} else {
				dqs_perbit_dly[j].best_dqsdly +=
				    (dqs_perbit_dly[j].total_dqsdly_pass -
				     dqs_perbit_dly[j].total_dqdly_pass) / 2 +
				    TX_STEPS;

				if (dqs_perbit_dly[j].best_dqsdly > 0xf) {
					DEBUG_DRAM_CALIB(
					    "warning 4:bit %d,dqs:%x", j,
					    dqs_perbit_dly[j].best_dqsdly);
					dqs_perbit_dly[j].best_dqsdly = 0xf;
				}

				dqs_perbit_dly[j].best_dqdly = 0 + TX_STEPS_DQ;

				if (dqs_perbit_dly[j].best_dqsdly >
				    max_dqsdly_byte[i]) {
					max_dqsdly_byte[i] =
					    dqs_perbit_dly[j].best_dqsdly;
				}
			}

			DEBUG_DRAM_CALIB("bit:%d, (DQS,DQ)=(%x,%x)\n",
					 j, dqs_perbit_dly[j].best_dqsdly,
					dqs_perbit_dly[j].best_dqdly);
		}

		/*
		 * we delay DQ or DQS to let DQS sample the middle
		 * of tx pass window for all the 8 bits,
		 */
		for (j = bit_first; j <= bit_last; j++) {

			/* set DQS to max for 8-bit */
			if (dqs_perbit_dly[j].best_dqsdly
						< max_dqsdly_byte[i]) {
				/* delay DQ to compensate extra DQS delay */
				dqs_perbit_dly[j].best_dqdly =
				    dqs_perbit_dly[j].best_dqdly +
				    (max_dqsdly_byte[i] -
				     dqs_perbit_dly[j].best_dqsdly);
				/* max limit to 15 */
				dqs_perbit_dly[j].best_dqdly =
				    ((dqs_perbit_dly[j].best_dqdly > 31) ?
				     31 : dqs_perbit_dly[j].best_dqdly);

				DEBUG_DRAM_CALIB("bit:%d, DQ=%x\n", j,
						 dqs_perbit_dly[j].best_dqdly);
			}

			ave_dqdly_byte[i] += dqs_perbit_dly[j].best_dqdly;
		}
		/* take the average of DQ for DQM */
		ave_dqdly_byte[i] = ave_dqdly_byte[i] / DQS_BIT_NUMBER;
		DEBUG_DRAM_CALIB("ave_dqdly_byte[ %d ]:0x%x\n", i,
				 ave_dqdly_byte[i]);
	}

	/* DQM */
	/*
	 * DQM0DLY: DRAMC_PADCTL2[0:3], 4 bits
	 * DQM1DLY: DRAMC_PADCTL2[4:7], 4 bits
	 * DQM2DLY: DRAMC_PADCTL2[8:11], 4 bits
	 * DQM3DLY: DRAMC_PADCTL2[15:12], 4 bits
	 */
	data = ((ave_dqdly_byte[0] & 0xF) << 0) |
	       ((ave_dqdly_byte[1] & 0xF) << 4) |
	       ((ave_dqdly_byte[2] & 0xF) << 8) |
	       ((ave_dqdly_byte[3] & 0xF) << 12) |
	       (DQM_mask);
	DRAMC_WRITE_REG(data, DRAMC_PADCTL2);
	opt_tx_dqm = data;
	DEBUG_DRAM_CALIB("TX DQM-0x%x, data:0x%x\n", DRAMC_PADCTL2, data);

	/* DQ */
	for (i = 0; i < DQS_NUMBER; i++) {
		j = i * DQS_BIT_NUMBER;
		/*
		 * DQ0DLY: DRAMC_DQODLY1[0:3], 4 bits
		 * DQ1DLY: DRAMC_DQODLY1[4:7], 4 bits
		 * DQ2DLY: DRAMC_DQODLY1[8:11], 4 bits
		 * DQ3DLY: DRAMC_DQODLY1[12:15], 4 bits
		 * DQ4DLY: DRAMC_DQODLY1[16:19], 4 bits
		 * DQ5DLY: DRAMC_DQODLY1[20:23], 4 bits
		 * DQ6DLY: DRAMC_DQODLY1[24:27], 4 bits
		 * DQ7DLY: DRAMC_DQODLY1[31:28], 4 bits
		 */
		data = ((dqs_perbit_dly[j].best_dqdly & 0xF) << 0) |
		       ((dqs_perbit_dly[j + 1].best_dqdly & 0xF) << 4) |
		       ((dqs_perbit_dly[j + 2].best_dqdly & 0xF) << 8) |
		       ((dqs_perbit_dly[j + 3].best_dqdly & 0xF) << 12) |
		       ((dqs_perbit_dly[j + 4].best_dqdly & 0xF) << 16) |
		       ((dqs_perbit_dly[j + 5].best_dqdly & 0xF) << 20) |
		       ((dqs_perbit_dly[j + 6].best_dqdly & 0xF) << 24) |
		       ((dqs_perbit_dly[j + 7].best_dqdly & 0xF) << 28);

		DRAMC_WRITE_REG(data, DRAMC_DQODLY1 + (4 * i));
		opt_tx_dq[i] = data;

		DEBUG_DRAM_CALIB("TX DQ[%d~%d]-0x%x, data:0x%x\n",
				 j, j + DQS_BIT_NUMBER - 1,
				DRAMC_DQODLY1 + (4 * i), data);
	}

	/* DQS */
	data = max_dqsdly_byte[0] |
	       (max_dqsdly_byte[1] << 4) |
	       (max_dqsdly_byte[2] << 8) |
	       (max_dqsdly_byte[3] << 12);

	DRAMC_WRITE_REG(data, DRAMC_PADCTL3);
	opt_tx_dqs = data;

	DEBUG_DRAM_CALIB("TX DQS-0x%x, data:0x%x\n", DRAMC_PADCTL3, data);

	/* extend DQM */
	for (i = 0; i < DQS_NUMBER; i++) {
		if ((ave_dqdly_byte[i] & 0x000000F0) != 0x0) {
			uint32_t val = DRAMC_READ_REG(DRAMC_PADCTL3) |
							(0x1 << (16 + i));
			DRAMC_WRITE_REG(val, DRAMC_PADCTL3);
		}
	}
	DEBUG_DRAM_CALIB("TX DQM extend-0x%x\n", DRAMC_READ_REG(DRAMC_PADCTL3));

	/* extend DQ */
	data = 0;
	for (i = 0; i < (DQ_DATA_WIDTH - 1); i++) {
		if ((dqs_perbit_dly[i].best_dqdly & 0x000000F0) != 0x0)
			data = data | (0x1 << i);
	}

	DRAMC_WRITE_REG(data | (DRAMC_READ_REG(DRAMC_IOCTL)), DRAMC_IOCTL);
	DEBUG_DRAM_CALIB("TX DQ extend 0x%x\n", data);

	if ((dqs_perbit_dly[31].best_dqdly & 0x000000F0) != 0x0) {
		DRAMC_WRITE_REG(DRAMC_READ_REG(DRAMC_PADCTL3) | (0x1 << 24),
				DRAMC_PADCTL3);
	}

	DEBUG_DRAM_CALIB("TX DQ 0x%x, 0x%x ,0x%x,0x%x\n",
			 DRAMC_READ_REG(DRAMC_DQODLY1),
			DRAMC_READ_REG(DRAMC_DQODLY2),
			DRAMC_READ_REG(DRAMC_DQODLY3),
			DRAMC_READ_REG(DRAMC_DQODLY4));
	return 0;
}

/*
 * dramc_calib: Do DRAMC calibration.
 * Return error code;
 */
int dramc_calib(void)
{
	int err;
	int rank0_col;
	int bak_cona;
	int bak_conf1;
	int tmp;

	/* modify MA type */
	bak_cona = read32((void *)EMI_CONA);
	bak_conf1 = read32((void *)DRAMC0_BASE + 0x04);

	rank0_col = (read32((void *)EMI_CONA) & (0x3 << 4)) >> 4;
	tmp = (read32((void *)DRAMC0_BASE + 0x04) & (~(0x3 << 8))) |
	      (rank0_col << 8);
	write32(tmp, (void *)(DRAMC0_BASE + 0x04));

	/* do calibration for DQS gating window (phase 1) for rank 0 */
	err = do_dqs_gw_calib_1();
	if (err < 0)
		goto dramc_calib_exit;

	opt_gw_coarse_value0 = opt_gw_coarse_value;
	opt_gw_fine_value0 = opt_gw_fine_value;
	DEBUG_DRAM_CALIB("rank 0 coarse = %s\n", opt_gw_coarse_value0);
	DEBUG_DRAM_CALIB("rank 0 fine = %s\n", opt_gw_fine_value0);

	/* do DLE calibration */
	err = do_dle_calib();

	/* if cal DLE fail, change RX setting and re-try */
	if (err != 0) {
		DEBUG_DRAM_CALIB("Corner Case Setting\n");

		/* change RX DQS, DQ, DM delay setting , then re-try */
		/* DQ0 ~ 3 */
		write32(0x03010302, (void *)(DRAMC0_BASE + 0x210));
		/* DQ4 ~ 7 */
		write32(0x02010202, (void *)(DRAMC0_BASE + 0x214));
		/* DQ8 ~ 11 */
		write32(0x0406060A, (void *)(DRAMC0_BASE + 0x218));
		/* DQ12 ~ 15 */
		write32(0x05040202, (void *)(DRAMC0_BASE + 0x21C));
		/* DQ16 ~ 19 */
		write32(0x05070303, (void *)(DRAMC0_BASE + 0x220));
		/* DQ20 ~ 23 */
		write32(0x01050106, (void *)(DRAMC0_BASE + 0x224));
		/* DQ24 ~ 27 */
		write32(0x05070708, (void *)(DRAMC0_BASE + 0x228));
		/* DQ28 ~ 31 */
		write32(0x05050005, (void *)(DRAMC0_BASE + 0x22C));

		/* postsim DQS0 ~ 3 */
		write32(0x100A100A, (void *)(DDRPHY_BASE + 0x18));

		err = do_dle_calib();
	}

	if (err < 0)
		goto dramc_calib_exit;

	/* do SW RX calibration for DQ/DQS input delay */
	err = do_sw_rx_dq_dqs_calib();
	if (err < 0)
		goto dramc_calib_exit;

	opt_rx_dqs0 = DRAMC_READ_REG(DRAMC_R0DELDLY);

	write32(bak_cona, (void *)EMI_CONA);
	write32(bak_conf1, (void *)(DRAMC0_BASE + 0x04));
	dqsi_gw_dly_coarse_factor_handler(opt_gw_coarse_value0);
	dqsi_gw_dly_fine_factor_handler(opt_gw_fine_value0);
	DRAMC_WRITE_REG(opt_rx_dqs0, DRAMC_R0DELDLY);

	DEBUG_DRAM_CALIB("[MEM]CONA%x,conf1:%x\n",
			 read32((void *)EMI_CONA),
			read32((void *)(DRAMC0_BASE + 0x04)));

	/* do SW TX calibration for DQ/DQS output delay */
	err = do_sw_tx_dq_dqs_calib();
	if (err < 0)
		goto dramc_calib_exit;

dramc_calib_exit:

	return err;
}
