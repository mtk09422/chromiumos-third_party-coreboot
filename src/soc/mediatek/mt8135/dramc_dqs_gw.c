#include <arch/io.h>
#include <console/console.h>
#include <string.h>

#include <soc/mt8135.h>
#include <soc/dramc.h>
#include <soc/emi_hw.h>
#include <soc/emi.h>
#include <soc/stdlib.h>

#if CONFIG_DEBUG_MEMORY_DRAM
#define DEBUG_DRAMC_DQS_GW(level, x...)	printk(level, "DQS GW: " x)
#else
#define DEBUG_DRAMC_DQS_GW(level, x...)
#endif /* CONFIG_DEBUG_MEMORY_DRAM */

#define DRAM_START (0x80000000)
#define RANK_SIZE (0x40000000)

const char *opt_gw_coarse_value;
const char *opt_gw_fine_value;
const char *opt_gw_coarse_value0;
const char *opt_gw_fine_value0;
const char *opt_gw_coarse_value1;
const char *opt_gw_fine_value1;

unsigned int score;
unsigned int high_score;

int ett_recursive_factor_tuning(unsigned int n, tuning_factor *tuning_factors,
				print_callbacks *cbs)
{
	unsigned int i;

	cbs->ett_print_banner(n);

	if (n == 0)
		cbs->ett_print_before_start_loop_zero();

	for (i = 0; tuning_factors[n].factor_tbl[i] != NULL; i++) {
		/* adjust factor steps */
		tuning_factors[n].factor_handler(tuning_factors[n].
						factor_tbl[i]);
		/* set the current factor steps */
		tuning_factors[n].curr_val = tuning_factors[n].factor_tbl[i];
		if (n == 0) { /* The most inner loop */
			cbs->ett_print_before_each_round_of_loop_zero();
			/* run test code */
			score += cbs->ett_print_result();
			cbs->ett_print_after_each_round_of_loop_zero();
		} else {  /* Other loops. Call this function recursively. */
			ett_recursive_factor_tuning(n - 1, tuning_factors, cbs);
		}
	}

	cbs->ett_calc_opt_value(n, &score, &high_score);

	cbs->ett_print_after_finish_loop_n(n);
	return 0;
}

/* DQS gating window (coarse) */
const char *dqsi_gw_dly_coarse_tbl[] = {
	"0", "1", "2", "3", "4", "5", "6", "7",
	"8", "9", "10", "11", "12", "13", "14", "15",
	"16", "17", "18", "19", "20", "21", "22", "23",
	"24", "25", "26", "27", "28", "29", "30", "31",
	NULL,
};

void dqsi_gw_dly_coarse_factor_handler_rank1(const char *factor_value)
{
	uint32_t curr_val = (uint32_t)atoi(factor_value);
	uint32_t data = 0;

	/*
	 * Reserved original value for DRAMC_DQSGCTL
	 * R1DQSG_COARSE_DLY_COM0: DRAMC_DQSGCTL[3:2], 2 bits
	 * R1DQSG_COARSE_DLY_COM1: DRAMC_DQSGCTL[7:6], 2 bits
	 */
	data = (DRAMC_READ_REG(DRAMC_DQSGCTL) & 0xFFFFFF33) |
	       ((curr_val & 0x3) << 2) |
	       ((curr_val & 0x3) << 6);
	DRAMC_WRITE_REG(data, DRAMC_DQSGCTL);
}

void dqsi_gw_dly_coarse_factor_handler(const char *factor_value)
{
	uint32_t curr_val = (uint32_t)atoi(factor_value);
	uint32_t data = 0;

	/*
	 * Reserved original values for DRAMC_DQSCTL1
	 * DQSINCTL: DRAMC_DQSCTL1[26:24], 3 bits
	 */
	data = (DRAMC_READ_REG(DRAMC_DQSCTL1) & 0xF8FFFFFF) |
	       (((curr_val >> 2) & 0x7) << 24);
	DRAMC_WRITE_REG(data, DRAMC_DQSCTL1);

	/*
	 * Reserved original values for DRAMC_DQSGCTL
	 * R0DQSG_COARSE_DLY_COM0: DRAMC_DQSGCTL[1:0], 2 bits
	 * R0DQSG_COARSE_DLY_COM1: DRAMC_DQSGCTL[5:4], 2 bits
	 */
	data = (DRAMC_READ_REG(DRAMC_DQSGCTL) & 0xFFFFFFCC) |
	       ((curr_val & 0x3) << 0) |
	       ((curr_val & 0x3) << 4);
	DRAMC_WRITE_REG(data, DRAMC_DQSGCTL);
}

/* DQS gating window (fine) */
const char *dqsi_gw_dly_fine_tbl[] = {
	"0", /* "1", "2", "3", "4", "5", "6", "7", */ "8",
	/* "9", "10", "11", "12", "13", "14", "15", */
	"16", /* "17", "18", "19", "20", "21", "22", "23", */ "24",
	/* "25", "26", "27", "28", "29", "30", "31", */
	"32", /* "33", "34", "35", "36", "37", "38", "39", */ "40",
	/* "41", "42", "43", "44", "45", "46", "47", */
	"48", /* "49", "50", "51", "52", "53", "54", "55", */ "56",
	/* "57", "58", "59", "60", "61", "62", "63", */
	"64", /* "65", "66", "67", "68", "69", "70", "71", */ "72",
	/* "73", "74", "75", "76", "77", "78", "79", */
	"80", /* "81", "82", "83", "84", "85", "86", "87", */ "88",
	/* "89", "90", "91", "92", "93", "94", "95", */
	"96", /* "97", "98", "99", "100", "101", "102", "103", */ "104",
	/* "105", "106", "107", "108", "109", "110", "111", */
	"112", /* "113", "114", "115", "116", "117", "118", "119", */ "120",
	/* "121", "122", "123", "124", "125", "126", "127", */
	NULL,
};

void dqsi_gw_dly_fine_factor_handler_rank1(const char *factor_value)
{
	uint32_t curr_val = (uint32_t)atoi(factor_value);
	uint32_t data = 0;

	/*
	 * Reserved original values for DRAMC_DQSIEN[31]
	 * DQS0IEN: DRAMC_DQSIEN[0:6], 7 bits
	 * DQS1IEN: DRAMC_DQSIEN[8:14], 7 bits
	 * DQS2IEN: DRAMC_DQSIEN[16:22], 7 bits
	 * DQS3IEN: DRAMC_DQSIEN[24:30], 7 bits
	 */
	data = (DRAMC_READ_REG(DRAMC_R1DQSIEN) & 0x80000000) |
	       ((curr_val & 0x7F) << 0) |
	       ((curr_val & 0x7F) << 8) |
	       ((curr_val & 0x7F) << 16) |
	       ((curr_val & 0x7F) << 24);
	DRAMC_WRITE_REG(data, DRAMC_R1DQSIEN);
}

void dqsi_gw_dly_fine_factor_handler(const char *factor_value)
{
	uint32_t curr_val = (uint32_t)atoi(factor_value);
	uint32_t data = 0;

	/*
	 * Reserved original values for DRAMC_DQSIEN[31]
	 * DQS0IEN: DRAMC_DQSIEN[0:6], 7 bits
	 * DQS1IEN: DRAMC_DQSIEN[8:14], 7 bits
	 * DQS2IEN: DRAMC_DQSIEN[16:22], 7 bits
	 * DQS3IEN: DRAMC_DQSIEN[24:30], 7 bits
	 */
	data = (DRAMC_READ_REG(DRAMC_R0DQSIEN) & 0x80000000) |
	       ((curr_val & 0x7F) << 0) |
	       ((curr_val & 0x7F) << 8) |
	       ((curr_val & 0x7F) << 16) |
	       ((curr_val & 0x7F) << 24);
	DRAMC_WRITE_REG(data, DRAMC_R0DQSIEN);
}

struct dqs_gw_pass_win {
	int coarse_end;
	int fine_end;
	int size;		/* gating window's size in this range */
};

static void Sequence_Read(unsigned int start, unsigned int len, void *ext_arg)
{
	int i;

	volatile int rval;
	/* DQS gating window counter reset */
	DRAMC_WRITE_SET((1 << 9), DRAMC_SPCMD);
	DRAMC_WRITE_CLEAR((1 << 9), DRAMC_SPCMD);

	DDR_PHY_RESET_NEW();

	for (i = 0; i < len; i++)
		rval = *(unsigned int *)(start);
}

enum {
	CPU_GW_GOLD_COUNTER = 0x02020202
};

static int Do_Read_Test_DDR3(unsigned int start, unsigned int len,
			     void *ext_arg)
{
	int err = 0;
	unsigned int DQSCounter = DRAMC_DQSGNWCNT0;

	if ((read32((void *)0x10004004) & 0x01) == 0)
		DQSCounter = DRAMC_DQSGNWCNT0;

	Sequence_Read(start, len, ext_arg);

	if (DRAMC_READ_REG(DRAMC_DQSGNWCNT0) != 0x0) {
		DEBUG_DRAMC_DQS_GW(BIOS_INFO,
				   "[1]DRAMC_DQSGNWCNT0:%x,DRAMC_DQSGNWCNT1:%x\n",
				DRAMC_READ_REG(DRAMC_DQSGNWCNT0),
				DRAMC_READ_REG(DRAMC_DQSGNWCNT1));
	}
	if (DRAMC_READ_REG(DQSCounter) == CPU_GW_GOLD_COUNTER) {
		Sequence_Read(start, len, ext_arg);
		if (DRAMC_READ_REG(DQSCounter) == CPU_GW_GOLD_COUNTER)
			err = 0;
		else
			err = -1;
	} else {
		err = -1;
	}

	return err;
}

static int Do_Read_Test_DDR2(unsigned int start, unsigned int len,
			     void *ext_arg)
{
	int err = 0;

	Sequence_Read(start, len, ext_arg);

	if (DRAMC_READ_REG(DRAMC_DQSGNWCNT0) == 0x04040404) {
		Sequence_Read(start, len, ext_arg);
		if (DRAMC_READ_REG(DRAMC_DQSGNWCNT0) == 0x04040404)
			err = 0;
		else
			err = -1;
	} else {
		err = -1;
	}

	return err;
}

/* Define how many steps we have in coarse tune, fine tune
 * check the number of  dqsi_gw_dly_fine_tbl and dqsi_gw_dly_coarse_tbl
 * To-be-porting
 */
enum {
	DQS_GW_COARSE_MAX = 32,
	DQS_GW_FINE_MAX = 16,
	DQS_GW_FINE_CHK_RANGE = 4
};

static const int HW_DQS_GW_COUNTER = 0xFCFCFCFC;
static unsigned int dqs_gw[DQS_GW_COARSE_MAX];
static int dqs_gw_coarse, dqs_gw_fine;
static struct dqs_gw_pass_win cur_pwin, max_pwin;

static int cpu_read_test(unsigned int start, unsigned int len, void *ext_arg)
{
	int err = 0;

	/* cpu read test */
	if (mt_get_dram_type() == 2)
		err = Do_Read_Test_DDR2(start, 1, ext_arg);
	else if (mt_get_dram_type() == 3)
		err = Do_Read_Test_DDR3(start, 1, ext_arg);
	else
		err = Do_Read_Test_DDR2(start, 1, ext_arg);

	DRAMC_WRITE_SET((1 << 9), DRAMC_SPCMD);
	DRAMC_WRITE_CLEAR((1 << 9), DRAMC_SPCMD);

	DDR_PHY_RESET_NEW();

	return err;
}

/*
 * dramc_ta2: Run DRAMC test agent 2.
 * @start: test start address
 * @len: test length
 * @ext_arg: extend argument (0: don't check read/write results; 1: check)
 * Return error code.
 */
static int dramc_ta2(unsigned int start, unsigned int len, void *ext_arg)
{
	int err = 0;
	int check_result = (int)ext_arg;
	unsigned int data;

	/* set test patern length */
	data = DRAMC_READ_REG(0x40);
	DRAMC_WRITE_REG((data & 0xFF000000) | 0x3FF, 0x40);

	DEBUG_DRAMC_DQS_GW(BIOS_INFO, "dramc_ta2 enable\n");
	/* Test Agent2 write enabling, Test Agent 2 read enabling */
	DRAMC_WRITE_SET((1 << 30) | (1 << 31), DRAMC_CONF2);

	while (!(DRAMC_READ_REG(DRAMC_TESTRPT) & (1 << 10)));

	/*
	 * NoteXXX: Need to wait for at least 400 ns
	 *   After checking the simulation result, there will be a delay on
	 *   DLE_CNT_OK/DM_CMP_ERR updates after getting DM_CMP_CPT. i.e.,
	 *   After getting the complete status, need to wait for a while
	 *   before reading DLE_CNT_OK/DM_CMP_ERR in the TESTRPT register.
	 */
	delay_a_while(400);
	printk(BIOS_INFO, "dramc_ta2 done\n");
	if (check_result) {

		if (DRAMC_READ_REG(DRAMC_TESTRPT) & (1 << 14))
			err = -1;
		else if (!(DRAMC_READ_REG(DRAMC_TESTRPT) & (1 << 18)))
			err = -1;
	}

	/* disable test agent2 r/w */
	DRAMC_WRITE_CLEAR(((1 << 30) | (1 << 31)), DRAMC_CONF2);

	if ((int)ext_arg == 0xFFFFFFFF)
		return err;

	DEBUG_DRAMC_DQS_GW(BIOS_INFO,
			   "[2]DRAMC_DQSGNWCNT0:%x,DRAMC_DQSGNWCNT1,%x\n",
				DRAMC_READ_REG(DRAMC_DQSGNWCNT0),
				DRAMC_READ_REG(DRAMC_DQSGNWCNT1));
				DEBUG_DRAMC_DQS_GW(BIOS_INFO,
						   "HW_DQS_GW_COUNTER need to be make sure\n");
	DDR_PHY_RESET();
	if (!err) {
		if ((DRAMC_READ_REG(DRAMC_DQSGNWCNT0) == HW_DQS_GW_COUNTER) &&
		    (DRAMC_READ_REG(DRAMC_DQSGNWCNT1) == HW_DQS_GW_COUNTER))
			err = 0;
		else
			err = -1;
	}
	/* DQS gating window counter reset */
	DRAMC_WRITE_SET((1 << 9), DRAMC_SPCMD);
	DRAMC_WRITE_CLEAR((1 << 9), DRAMC_SPCMD);

	return err;
}

tuning_factor dqs_gw_tuning_factors[] = {
	{
		.name = "DQS Gating Window Delay (Fine Scale)",
		.factor_tbl = (char **)dqsi_gw_dly_fine_tbl,
		.curr_val = NULL,
		.opt_val = NULL,
		.factor_handler = dqsi_gw_dly_fine_factor_handler,
	},
	{
		.name = "DQS Gating Window Delay (Coarse Scale)",
		.factor_tbl = (char **)dqsi_gw_dly_coarse_tbl,
		.curr_val = NULL,
		.opt_val = NULL,
		.factor_handler = dqsi_gw_dly_coarse_factor_handler,
	},
};

test_case dqs_gw_test_cases_1[] = {
	{
		.test_case = cpu_read_test,	/* use dramc test agent */
		.start = DRAM_START,
		.range = 0xA,
		/* check_result, 1st run, no need to check the r/w value */
		.ext_arg = (void *)0,
	},
};

test_case dqs_gw_test_cases_2[] = {
	{
		.test_case = dramc_ta2,	/* use dramc test agent */
		.start = DRAM_START,
		.range = 0xA,
		/* check_result, 2nd run, we need to check the r/w value */
		.ext_arg = (void *)1,
	},
};

static void ett_print_dqs_gw_banner(unsigned int n)
{
	return;
}

static void ett_print_dqs_gw_before_start_loop_zero(void)
{
	return;
}

static void ett_print_dqs_gw_before_each_round_of_loop_zero(void)
{
	/* DQS gating window counter reset */
	DRAMC_WRITE_SET((1 << 9), DRAMC_SPCMD);
	DRAMC_WRITE_CLEAR((1 << 9), DRAMC_SPCMD);
	DDR_PHY_RESET();
}

static unsigned int __ett_print_dqs_gw_result(test_case *test_cases, int nr_ts)
{
	unsigned int i;
	unsigned int _score = 1;
	int ret = 0;

	for (i = 0; i < nr_ts; i++) {

		/* fixup the test start_address due to dual rank */
		test_cases[i].start = DRAM_START;

		ret = test_cases[i].test_case(test_cases[i].start,
						test_cases[i].range,
						test_cases[i].ext_arg);
		if (ret < 0) {
			_score = 0;
			break;
		}
	}
	/*
	 * DQS GW calibration rule 1: Identify a pass-window with the max gw.
	 */
	if (_score != 0) {
		/* attension:dqs_gw_fine steps must less then 32 */
		dqs_gw[dqs_gw_coarse] |= (1 << dqs_gw_fine);
	}

	dqs_gw_fine++;
	if (dqs_gw_fine >= DQS_GW_FINE_MAX) {	/* dqs_gw_fine >= 16 */
		dqs_gw_coarse++;
		dqs_gw_fine &= (DQS_GW_FINE_MAX - 1);
	}
	if (dqs_gw_coarse > DQS_GW_COARSE_MAX) {
		/* critical error */
		DEBUG_DRAMC_DQS_GW(BIOS_INFO,
				   "Critical error!! "
				"dqs_gw_coarse > DQS_GW_COARSE_MAX\n");
	}

	return _score;
}

static unsigned int ett_print_dqs_gw_result_1(void)
{
	int testcase_num = ETT_TEST_CASE_NUMS(dqs_gw_test_cases_1);

	return __ett_print_dqs_gw_result(dqs_gw_test_cases_1, testcase_num);
}

static void ett_print_dqs_gw_after_each_round_of_loop_zero(void)
{
	return;
}

static void ett_calc_dqs_gw_opt_value(unsigned int n, unsigned int *_score,
				      unsigned int *_high_score)
{
	return;
}

static void ett_print_dqs_gw_fine_after_finish_loop_zero(unsigned int n)
{
	return;
}

/*
 * nr_bit_set: Get the number of bits set in the given value.
 * @val: the gieven value
 * Return the number of bits set.
 */
static int nr_bit_set(unsigned int val)
{
	int i, cnt;

	for (cnt = 0, i = 0; i < (8 * sizeof(unsigned int)); i++) {
		if (val & (1 << i))
			cnt++;
	}

	return cnt;
}

/*
 * first_bit_set: Get the first bit set in the given value.
 * @val: the gieven value
 * Return the first bit set.
 */
static int first_bit_set(unsigned int val)
{
	int i;

	for (i = 0; i < (8 * sizeof(unsigned int)); i++) {
		if (val & (1 << i))
			return i;
	}

	return -1;
}

/*
 * __do_dqs_gw_calib: do DQS gating window calibration.
 * @cbs: pointer to the print_callbacks structure.
 * Return error code.
 */
static int __do_dqs_gw_calib(print_callbacks *cbs)
{
	int err;
	int i, c = 0, f, cnt, max = 0;	/* AP initialize c = 0; */
	unsigned int tf_num; /* ETT tuning factor number */

	err = -1;

	dqs_gw_coarse = 0;	/* from begin of coarse tune, reset to 0 */
	dqs_gw_fine = 0;	/* from begin of fine tune, reset to 0 */

	for (i = 0; i < DQS_GW_COARSE_MAX; i++)
		dqs_gw[i] = 0;

	memset((void *)&cur_pwin, 0, sizeof(struct dqs_gw_pass_win));
	memset((void *)&max_pwin, 0, sizeof(struct dqs_gw_pass_win));

	/* 1.enable burst mode for gating window */
	/*   enable DQS gating window counter */
	DRAMC_WRITE_SET((1 << 28), DRAMC_DQSCTL1);
	DRAMC_WRITE_SET((1 << 8), DRAMC_SPCMD);

	if (ETT_TUNING_FACTOR_NUMS(dqs_gw_tuning_factors) > 0) {

		/* tf_num stands for ETT tuning factor number */
		tf_num = ETT_TUNING_FACTOR_NUMS(dqs_gw_tuning_factors) - 1;
		ett_recursive_factor_tuning(tf_num, dqs_gw_tuning_factors, cbs);
	}
	if ((DRAMC_READ_REG(DRAMC_MCKDLY) & 0x80000000) == 0x0) {  /* x16 */
		for (i = 0; i < DQS_GW_COARSE_MAX; i++) {
			/* find the max passed window */
			cnt = nr_bit_set(dqs_gw[i]);
			if (cnt >= max) {
				max = cnt;
				c = i;
			}
		}
	} else {		/* x8 */
		for (i = 0; i < DQS_GW_COARSE_MAX; i++) {
			/* find the first passed window */
			cnt = nr_bit_set(dqs_gw[i]);
			if (cnt >= 2) {
				c = i + 2;
				break;
			}
		}
	}
	cnt = nr_bit_set(dqs_gw[c]);
	/* for x8 PCB */
	if ((DRAMC_READ_REG(DRAMC_MCKDLY) & 0x80000000) != 0x0) {
		if (cnt >= 10)
			cnt = 10;
	}
	if (cnt) {
		f = first_bit_set(dqs_gw[c]) + cnt / 2;

		opt_gw_coarse_value = dqsi_gw_dly_coarse_tbl[c];
		opt_gw_fine_value = dqsi_gw_dly_fine_tbl[f];

		/* setup the opt coarse value and fine value according to
		 * calibration result
		 */
		DEBUG_DRAMC_DQS_GW(BIOS_INFO, "%s\n", opt_gw_coarse_value);
		DEBUG_DRAMC_DQS_GW(BIOS_INFO, "%s\n", opt_gw_fine_value);

		dqsi_gw_dly_coarse_factor_handler(dqsi_gw_dly_coarse_tbl[c]);
		dqsi_gw_dly_fine_factor_handler(dqsi_gw_dly_fine_tbl[f]);

		err = 0;
	} else {
		DEBUG_DRAMC_DQS_GW(BIOS_INFO, "Cannot find any pass-window\n");
	}

	/* disable DQS gating window counter */
	DRAMC_WRITE_CLEAR((1 << 8), DRAMC_SPCMD);

	return err;
}

/*
 * do_dqs_gw_calib_1: do DQS gating window calibration (phase 1).
 * Return error code.
 */
int do_dqs_gw_calib_1(void)
{
	print_callbacks cbs = {
		.ett_print_banner = ett_print_dqs_gw_banner,
		.ett_print_before_start_loop_zero =
		ett_print_dqs_gw_before_start_loop_zero,
		.ett_print_before_each_round_of_loop_zero =
		ett_print_dqs_gw_before_each_round_of_loop_zero,
		/* difference between do_dqs_gw_calib_2 */
		.ett_print_result = ett_print_dqs_gw_result_1,
		.ett_print_after_each_round_of_loop_zero =
		ett_print_dqs_gw_after_each_round_of_loop_zero,
		.ett_calc_opt_value = ett_calc_dqs_gw_opt_value,
		.ett_print_after_finish_loop_n =
		ett_print_dqs_gw_fine_after_finish_loop_zero,
	};

	return __do_dqs_gw_calib(&cbs);
}
