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

#ifndef SOC_MEDIATEK_MT8135_DRAMC_H
#define SOC_MEDIATEK_MT8135_DRAMC_H

/* declare in dramc_dqs_gw.c*/
extern const char *opt_gw_coarse_value;
extern const char *opt_gw_fine_value;
extern const char *opt_gw_coarse_value0;
extern const char *opt_gw_fine_value0;
extern const char *opt_gw_coarse_value1;
extern const char *opt_gw_fine_value1;

/* declare in dramc_dle_cal.c*/
extern const char *opt_dle_value;

typedef struct {
	const char *name;
	char **factor_tbl;
	char *curr_val;
	char *opt_val;
	void (*factor_handler)(const char *);
} tuning_factor;

typedef struct {
	void (*ett_print_banner)(unsigned int);
	void (*ett_print_before_start_loop_zero)(void);
	void (*ett_print_before_each_round_of_loop_zero)(void);
	unsigned int (*ett_print_result)(void);
	void (*ett_print_after_each_round_of_loop_zero)(void);
	void (*ett_calc_opt_value)(unsigned int, unsigned int *,
				   unsigned int *);
	void (*ett_print_after_finish_loop_n)(unsigned int);
} print_callbacks;

#define ETT_TUNING_FACTOR_NUMS(x)	(sizeof(x)/sizeof(tuning_factor))

typedef struct {
	int (*test_case)(unsigned int, unsigned int, void *);
	unsigned int start;
	unsigned int range;
	void *ext_arg;
} test_case;

#if defined(MT8135)
#define DRAMC_WRITE_REG(val, offset)	do { \
		write32((uint32_t)val, (void *)(DRAMC0_BASE + (offset))); \
		write32((uint32_t)val, (void *)(DDRPHY_BASE + (offset))); \
		write32((uint32_t)val, (void *)(DRAMC_NAO_BASE + (offset))); \
	} while (0)

#define DRAMC_WRITE_REG_W(val, offset)	do { \
		write32((uint32_t)val, (void *)(DRAMC0_BASE + (offset))); \
		write32((uint32_t)val, (void *)(DDRPHY_BASE + (offset))); \
		write32((uint32_t)val, (void *)(DRAMC_NAO_BASE + (offset))); \
	} while (0)

#define DRAMC_WRITE_REG_H(val, offset)	do { \
		write16((uint16_t)val, (void *)(DRAMC0_BASE + (offset))); \
		write32((uint16_t)val, (void *)(DDRPHY_BASE + (offset))); \
		write32((uint16_t)val, (void *)(DRAMC_NAO_BASE + (offset))); \
	} while (0)

#define DRAMC_WRITE_REG_B(val, offset)	do { \
		write16((uint8_t)val, (void *)(DRAMC0_BASE + (offset))); \
		write32((uint8_t)val, (void *)(DDRPHY_BASE + (offset))); \
		write32((uint8_t)val, (void *)(DRAMC_NAO_BASE + (offset))); \
	} while (0)

#define DRAMC_READ_REG(offset) \
	( \
		read32((void *)(DRAMC0_BASE + (offset))) | \
		read32((void *)(DDRPHY_BASE + (offset))) | \
		read32((void *)(DRAMC_NAO_BASE + (offset))) \
	)

#define DRAMC_WRITE_SET(val, offset)	do { \
		uint32_t DRAMC0_ADDR = DRAMC0_BASE + offset; \
		uint32_t DDRPHY_ADDR = DDRPHY_BASE + offset; \
		uint32_t DRAMC_NAO_ADDR = DRAMC_NAO_BASE + offset; \
		unsigned int uval = (unsigned int)(val); \
		(*(volatile unsigned int *)(DRAMC0_ADDR)) |= uval; \
		(*(volatile unsigned int *)(DDRPHY_ADDR)) |= uval; \
		(*(volatile unsigned int *)(DRAMC_NAO_ADDR)) |= uval; \
	} while (0)

#define DRAMC_WRITE_CLEAR(val, offset)	do { \
		uint32_t DRAMC0_ADDR = DRAMC0_BASE + offset; \
		uint32_t DDRPHY_ADDR = DDRPHY_BASE + offset; \
		uint32_t DRAMC_NAO_ADDR = DRAMC_NAO_BASE + offset; \
		unsigned int uval = (unsigned int)(val); \
		(*(volatile unsigned int *)(DRAMC0_ADDR)) &= ~(uval); \
		(*(volatile unsigned int *)(DDRPHY_ADDR)) &= ~(uval); \
		(*(volatile unsigned int *)(DRAMC_NAO_ADDR)) &= ~(uval); \
	} while (0)

#define DDRPHY_WRITE_REG(val, offset) \
		write32(val, (void *)(DDRPHY_BASE + (offset)))

#define DRAMC0_WRITE_REG(val, offset) \
		write32(val, (void *)(DRAMC0_BASE + (offset)))

#define DRAMC_NAO_WRITE_REG(val, offset) \
		write32(val, (void *)(DRAMC_NAO_BASE + (offset)))

#endif

#define ETT_TEST_CASE_NUMS(x)	(sizeof(x)/sizeof(test_case))

#define GRAY_ENCODED(a) (a)

#ifndef NULL
#define NULL    0
#endif

#define delay_a_while(count) \
	do { \
		register unsigned int delay = 0;	\
		asm volatile ("dsb" :  :  : "memory");	\
		asm volatile ("mov %0, %1\n\t"	\
				"1:\n\t"	\
				"subs %0, %0, #1\n\t"	\
				"bne 1b\n\t"	\
				: "+r" (delay)	\
				: "r" (count)	\
				: "cc");	\
	} while (0)

#define DDR_PHY_RESET() do {	\
		uint32_t val;	\
		val = (DRAMC_READ_REG(DRAMC_PHYCTL1)) | (1 << 28);	\
		DRAMC_WRITE_REG(val, DRAMC_PHYCTL1);	\
		val = (DRAMC_READ_REG(DRAMC_GDDR3CTL1)) | (1 << 25);	\
		DRAMC_WRITE_REG(val, DRAMC_GDDR3CTL1);	\
		delay_a_while(1000);	\
		val = (DRAMC_READ_REG(DRAMC_PHYCTL1)) & (~(1 << 28));	\
		DRAMC_WRITE_REG(val, DRAMC_PHYCTL1);	\
		val = (DRAMC_READ_REG(DRAMC_GDDR3CTL1)) & (~(1 << 25));	\
		DRAMC_WRITE_REG(val, DRAMC_GDDR3CTL1);	\
	} while (0)

#define DDR_PHY_RESET_NEW() do { \
		uint32_t val;	\
		val  = (DRAMC_READ_REG(DRAMC_PHYCTL1)) | (1 << 28);	\
		DRAMC_WRITE_REG(val, DRAMC_PHYCTL1);	\
		val = (DRAMC_READ_REG(DRAMC_GDDR3CTL1)) | (1 << 25);	\
		DRAMC_WRITE_REG(val, DRAMC_GDDR3CTL1);	\
		delay_a_while(1000); \
		val = (DRAMC_READ_REG(DRAMC_PHYCTL1)) & (~(1 << 28));	\
		DRAMC_WRITE_REG(val, DRAMC_PHYCTL1);	\
		val = (DRAMC_READ_REG(DRAMC_GDDR3CTL1)) & (~(1 << 25));	\
		DRAMC_WRITE_REG(val, DRAMC_GDDR3CTL1);	\
	} while (0)

#define UNUSED_VAR(x) ((void)x)

/* define supported DRAM types */
enum {
	TYPE_LPDDR2 = 0,
	TYPE_DDR3,
	TYPE_LPDDR3,
	TYPE_mDDR,
};

int dramc_calib(void);
int do_dqs_gw_calib_1(void);
int do_dle_calib(void);
int ett_recursive_factor_tuning(unsigned int n, tuning_factor *tuning_factors,
				print_callbacks *cbs);
void dqsi_gw_dly_coarse_factor_handler(const char *factor_value);
void dqsi_gw_dly_coarse_factor_handler_rank1(const char *factor_value);
void dqsi_gw_dly_fine_factor_handler(const char *factor_value);
void dqsi_gw_dly_fine_factor_handler_rank1(const char *factor_value);
#endif /* !SOC_MEDIATEK_MT8135_DRAMC_H */
