#include <arch/io.h>
#include <console/console.h>

#include <soc/mt8135.h>
#include <soc/dramc.h>
#include <soc/emi_hw.h>
#include <soc/stdlib.h>

#if CONFIG_DEBUG_MEMORY_DRAM
#define DEBUG_DRAMC_DLE_CAL(level, x...) printk(level, "DLE CAL: " x)
#else
#define DEBUG_DRAMC_DLE_CAL(level, x...)
#endif /* CONFIG_DEBUG_MEMORY_DRAM */

#define DLE_MAX 16
int dle_result[DLE_MAX];
const char *opt_dle_value = "0";
static int global_dle_value;

#define TEST_PASS (0)

enum {
	DRAM_START = 0x80000000,
	RANK_SIZE = 0x20000000
};

static int dramc_dma_test(unsigned int start, unsigned int len, void *ext_arg)
{
	int err = TEST_PASS;
	int i;

	unsigned int *src_buffp1;
	unsigned int *dst_buffp1;

	src_buffp1 = (unsigned int *)0x80000000;
	dst_buffp1 = (unsigned int *)0x80001000;

	for (i = 0; i < (len / sizeof(unsigned int)); i++)
		*((unsigned int *)dst_buffp1 + i) = 0;

	for (i = 0; i < (len / sizeof(unsigned int)); i++)
		*((unsigned int *)src_buffp1 + i) = i;

	write32(0x00070000, (void *)0x11000098); /* BURST_LEN: 7-8, R/W */
	write32((uint32_t)src_buffp1, (void *)0x1100009C);
	write32((uint32_t)dst_buffp1, (void *)0x110000A0);
	write32(len, (void *)0x110000A4);
	write32(0x1, (void *)0x11000088); /* start dma */

	while (read32((void *)0x11000088));

	for (i = 0; i < (len / sizeof(unsigned int)); i++) {
		if (*((unsigned int *)dst_buffp1 + i) != i) {
			err = -1;
			break;
		}
	}

	return err;
}

/* [FIXME] Change the data type from char to integer type */
const char *dram_driving_tbl[] = {
	/* DRAM I/O Driving */
	/* "1",  // 34.3 ohm:    0001 */
	/* "2",  // 40   ohm:    0010 */
	/* "3",  // 48   ohm:    0011 */
	"4",	 /* 60   ohm:    0100 */
	/* "5",  // 68.6 ohm:    0101 */
	/* "6",  // 80   ohm:    0110 */
	/* "7",  // 120  ohm:    0111 */
	NULL,
};

/* [FIXME] Change the data type from char to integer type */
const char *dle_tbl[] = {
	/* DLE: 0x0~0xf */
	"0", "1", "2", "3", "4",
	"5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15",
	NULL,
};

static void dle_factor_handler(const char *factor_value)
{
	uint32_t curr_val = (uint32_t)atoi(factor_value);
	uint32_t data = 0;

	DEBUG_DRAMC_DLE_CAL(BIOS_INFO, "dle=%d\n", curr_val);

	data = (DRAMC_READ_REG(DRAMC_DDR2CTL) & 0xFFFFFF8F) |
	       ((curr_val & 0x7) << 4);
	DRAMC_WRITE_REG(data, DRAMC_DDR2CTL);

	data = (DRAMC_READ_REG(DRAMC_PADCTL4) & 0xFFFFFFEF) |
	       (((curr_val >> 3) & 0x1) << 4);
	DRAMC_WRITE_REG(data, DRAMC_PADCTL4);
}

tuning_factor rx_tuning_factors[] = {
	{"DLE", (char **)dle_tbl, NULL, NULL, dle_factor_handler},
};

/* [FIXME] Change the data type from char to integer type */
const char *driving_tbl[] = {
	/* Tx I/O Driving */
	"8", "9", "10", "11", "12", /* LPDDR == 2 */
	NULL,
};

tuning_factor dle_tuning_factors[] = {
	{
		.name = "DLE",
		.factor_tbl = (char **)dle_tbl,
		.curr_val = NULL,
		.opt_val = NULL,
		.factor_handler = dle_factor_handler,
	},
};

test_case dle_test_cases[] = {
	{ dramc_dma_test, 0x0, 0x80, (void *)0xFFFFFFFF }
};

static void ett_print_dle_banner(unsigned int n)
{
	return;
}

static unsigned int ett_print_dle_result(void)
{
	unsigned int i;
	unsigned int score = 1;
	int ret = 0;

	for (i = 0; i < ETT_TEST_CASE_NUMS(dle_test_cases); i++) {

		/* fixup the test start_address */
		dle_test_cases[i].start = DRAM_START;

		ret = dle_test_cases[i].test_case(dle_test_cases[i].start,
						dle_test_cases[i].range,
						dle_test_cases[i].ext_arg);
		if (ret == TEST_PASS) {

			dle_result[global_dle_value++] = score;
		} else {

			score = 0;
			dle_result[global_dle_value++] = score;
			break;
		}

	}

	return score;
}

static void ett_print_dle_before_each_round_of_loop_zero(void)
{
	DDR_PHY_RESET();
}

static void ett_print_dle_after_each_round_of_loop_zero(void)
{
	delay_a_while(200);
}

static void ett_print_dle_before_start_loop_zero(void)
{
	return;
}

static void ett_print_dle_after_finish_loop_n(unsigned int n)
{
	return;
}

static void ett_calc_dle_opt_value(unsigned int n, unsigned int *score,
				   unsigned int *high_score)
{
	return;
}

/*
 * Related Registers (Latency between DRAMC and PHY)
 *   - PADCTL4  bit4     DATLAT3
 *   - DDR2CTL  bit 6~4  DATLAT
 * Algorithm
 *   - Set DLE from 4 to 14 and check if data is correct.
 */
int do_dle_calib(void)
{
	int ix;

	global_dle_value = 0;
	print_callbacks cbs = {
		.ett_print_banner = ett_print_dle_banner,
		.ett_print_before_start_loop_zero =
		ett_print_dle_before_start_loop_zero,
		.ett_print_before_each_round_of_loop_zero =
		ett_print_dle_before_each_round_of_loop_zero,
		.ett_print_result = ett_print_dle_result,
		.ett_print_after_each_round_of_loop_zero =
		ett_print_dle_after_each_round_of_loop_zero,
		.ett_calc_opt_value = ett_calc_dle_opt_value,
		.ett_print_after_finish_loop_n =
		ett_print_dle_after_finish_loop_n,
	};

	if (ETT_TUNING_FACTOR_NUMS(dle_tuning_factors) > 0) {
		ett_recursive_factor_tuning(ETT_TUNING_FACTOR_NUMS
						(dle_tuning_factors) - 1,
						dle_tuning_factors, &cbs);
	}

	for (ix = 0; (ix < DLE_MAX) && (dle_tbl[ix + 1] != NULL); ix++) {
		if (dle_result[ix] == 1 && dle_result[ix + 1] == 1) {
			/* [TODO] Verify dle_tbl[ix+1] or *dle_tbl[ix+1] */
			if (*(dle_tbl[ix + 1]) > *opt_dle_value)
				opt_dle_value = dle_tbl[ix + 1];
			DEBUG_DRAMC_DLE_CAL(BIOS_INFO,
					    "opt_dle value:%c\n", *opt_dle_value);
			break;
		}
	}

	/* setup the opt dle value according to calibration result */
	if (*opt_dle_value != 0x30) {	/* 0x30 == "0" */
		dle_factor_handler(opt_dle_value);
		return 0;
	} else {
		DEBUG_DRAMC_DLE_CAL(BIOS_INFO, "cannot find opt_dle value\n");
		return -1;
	}
}
