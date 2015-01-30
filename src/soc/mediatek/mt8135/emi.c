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
#include <string.h>
#include <console/console.h>

#include <soc/mt8135.h>
#include <soc/emi.h>
#include <soc/emi_hw.h>
#include <soc/dramc.h>
#include <soc/wdt.h>

#if CONFIG_DEBUG_MEMORY_DRAM
#define DEBUG_EMI(level, x...)	printk(level, "EMI: " x)
#else
#define DEBUG_EMI(level, x...)
#endif	/* CONFIG_DEBUG_MEMORY_DRAM */

#define emi_write32(val, addr)		write32(val, (void *)addr)

extern int num_of_emi_records;
extern EMI_SETTINGS emi_settings[];

#define EMI_BASE	0x10203000
#define DRAMC0_BASE	0x10004000
#define DDRPHY_BASE	0x10011000
#define DRAMC_NAO_BASE	0x1020F000

/* configuration:
 *   MEMPLL_CLK_666
 *   SINGLE_RANK_MODE
 *   !DDR3_16B
 *   !DDR3_SCRAMBLE_SUPPORT
 *   fcDELAY_CLK
 *   !DDR3_NO_PINMUX
 *   !DRAMC_ASYNC
 *   fcDELAY_CLK
 *   fcENABLE_DUAL_SCHEDULER
 *   !DDR3_AUTOREF_SUPPORT
 *   !DDR3_ZQCS_SUPPORT
 */

static inline void dramc0_phy_write32(uint32_t val, uint32_t offset)
{
	write32(val, (void *)(DRAMC0_BASE + offset));
	write32(val, (void *)(DDRPHY_BASE + offset));
}

static void init_dram3(EMI_SETTINGS *emi_setting)
{
	uint32_t tmp = 0;

	DEBUG_EMI(BIOS_INFO, "[mt_pll_init] = 666Mhz\n");
	DEBUG_EMI(BIOS_DEBUG, "init ddr3 x32\n");

	/* [FIXME]: is this delay really necessary? */
	delay_a_while(2000000);	/* test 20121015 */

	/*
	 * dw32_en=1, row 15 bit, bank 3bit,
	 * column 10bit: 4Gb x 16 + 4Gb x 16
	 */
	emi_write32(emi_setting->EMI_CONA_VAL, EMI_CONA);

	if (emi_setting->EMI_CONA_VAL == 0x0002F3AE)
		emi_write32(0xE0B4, EMI_TESTB); /* For 2GB */
	else
		emi_write32(0xE034, EMI_TESTB);

	emi_write32(0x00429000, EMI_CONF); /* Enable EMI address scrambling */
	emi_write32(0x500, EMI_CONM);      /* Enable EMI/DRAMC access */
	emi_write32(0, EMI_TESTD);

	/* default cmd output delay, determine by measurement */
	emi_write32(0, DDRPHY_BASE + DRAMC_CMDDLY0);
	emi_write32(0, DDRPHY_BASE + DRAMC_CMDDLY1);
	emi_write32(0, DDRPHY_BASE + DRAMC_CMDDLY2);
	emi_write32(0, DDRPHY_BASE + DRAMC_CMDDLY3);
	emi_write32(0, DDRPHY_BASE + DRAMC_CMDDLY4);
	emi_write32(0, DDRPHY_BASE + DRAMC_CMDDLY5);
	emi_write32(0, DDRPHY_BASE + DRAMC_DQSCAL0);

	if ((emi_setting->DRAMC_MCKDLY_VAL & 0x80000000) == 0x0)
		dramc0_phy_write32(0x0B000000, 0xC);
	else
		dramc0_phy_write32(0x00000FF0, 0xC);

	dramc0_phy_write32(0x1E00D10D, 0x48);

	dramc0_phy_write32(emi_setting->DRAMC_MCKDLY_VAL, 0xD8);

	if ((emi_setting->DRAMC_MCKDLY_VAL & 0x80000000) == 0x0)
		dramc0_phy_write32(0, 0xF0);
	else
		dramc0_phy_write32(0x80000000, 0xF0);

	dramc0_phy_write32(0x000000A3, 0xE4); /* CKEBYCTL & reset on */
	__asm__ __volatile__("dsb" :  :  : "memory");
	delay_a_while(60000);	/* 600.us, DDR3-1066 */

	dramc0_phy_write32(0, 0x90);
	dramc0_phy_write32(0x80000000, 0x94);
	dramc0_phy_write32(0x83000000, 0xDC);
	dramc0_phy_write32(0x12000000, 0xE0);
	dramc0_phy_write32(emi_setting->DRAMC_GDDR3CTL1_VAL, 0xF4);
	dramc0_phy_write32(emi_setting->DRAMC_MISCTL0_VAL, 0xFC);
	dramc0_phy_write32(0x00000010, 0x168);
	dramc0_phy_write32(0x30000000, 0x130); /* clock enable */
	__asm__ __volatile__("dsb" :  :  : "memory");
	delay_a_while(1000);

	dramc0_phy_write32(emi_setting->DRAMC_MCKDLY_VAL | 0x00200000, 0xD8);
	dramc0_phy_write32(emi_setting->DRAMC_CONF1_VAL, 0x4); /* DM64BIT = 1 */
	dramc0_phy_write32(0x00001611, 0x1EC);

	if ((emi_setting->DRAMC_MCKDLY_VAL & 0x80000000) == 0x0)
		dramc0_phy_write32(0x000B0000, 0x8C);
	else
		dramc0_phy_write32(0, 0x8C);

	dramc0_phy_write32(0x80000033, 0x124);
	dramc0_phy_write32(0x40404040, 0x94);
	dramc0_phy_write32(0x00000000, 0x1C0);
	dramc0_phy_write32(emi_setting->DRAMC_DDR2CTL_VAL, 0x7C);

	if ((emi_setting->DRAMC_MCKDLY_VAL & 0x80000000) == 0x0)
		dramc0_phy_write32(0xF1200F01, 0x28);
	else
		dramc0_phy_write32(0xF1200000, 0x28);

	dramc0_phy_write32(0, 0x158);
	dramc0_phy_write32(0x08000000, 0x1E0);
	dramc0_phy_write32(0x000000A7, 0xE4);
	__asm__ __volatile__("dsb" :  :  : "memory");
	delay_a_while(2000);

	dramc0_phy_write32(emi_setting->DRAMC_DRVCTL0_VAL, 0xB4);
	dramc0_phy_write32(emi_setting->DRAMC_DRVCTL0_VAL, 0xB8);

	if ((emi_setting->DRAMC_MCKDLY_VAL & 0x80000000) == 0x0)
		dramc0_phy_write32(0xBB00BB00, 0xBC);
	else
		dramc0_phy_write32(0xFF00FF00, 0xBC);

	dramc0_phy_write32(0, 0xC0);
	dramc0_phy_write32(0, 0x644);
	dramc0_phy_write32(0x0B051100, 0x110);
	__asm__ __volatile__("dsb" :  :  : "memory");

	/* MR2 */
	dramc0_phy_write32(emi_setting->DDR3_MODE_REG2, 0x88);
	dramc0_phy_write32(0x1, 0x1E4);
	__asm__ __volatile__("dsb" :  :  : "memory");
	delay_a_while(1000);

	/* MR3 */
	dramc0_phy_write32(0, 0x1E4);
	dramc0_phy_write32(emi_setting->DDR3_MODE_REG3, 0x88);
	dramc0_phy_write32(0x1, 0x1E4);
	__asm__ __volatile__("dsb" :  :  : "memory");
	delay_a_while(1000);

	/* MR1 */
	dramc0_phy_write32(0, 0x1E4);
	dramc0_phy_write32(emi_setting->DDR3_MODE_REG1, 0x88);
	dramc0_phy_write32(0x1, 0x1E4);
	__asm__ __volatile__("dsb" :  :  : "memory");
	delay_a_while(1000);

	/* MR0 */
	dramc0_phy_write32(0, 0x1E4);
	dramc0_phy_write32(emi_setting->DDR3_MODE_REG0, 0x88);
	dramc0_phy_write32(0x1, 0x1E4);
	__asm__ __volatile__("dsb" :  :  : "memory");
	delay_a_while(1000);

	dramc0_phy_write32(0, 0x1E4);
	dramc0_phy_write32(0x400, 0x88);
	dramc0_phy_write32(0x10, 0x1E4);
	__asm__ __volatile__("dsb" :  :  : "memory");
	delay_a_while(1000);

	dramc0_phy_write32(0, 0x1E4);
	__asm__ __volatile__("dsb" :  :  : "memory");
	delay_a_while(1000);

	dramc0_phy_write32(0x1100, 0x1E4);
	dramc0_phy_write32(0xA3, 0xE4);

	if (emi_setting->EMI_CONA_VAL == 0x0002F3AE)
		dramc0_phy_write32(emi_setting->DRAMC_PD_CTRL_VAL | 0x00000080,
				   0x1DC);
	else
		dramc0_phy_write32(emi_setting->DRAMC_PD_CTRL_VAL, 0x1DC);

	dramc0_phy_write32(emi_setting->DRAMC_ACTIM_VAL, 0);
	dramc0_phy_write32(emi_setting->DRAMC_TEST2_3_VAL, 0x44);
	dramc0_phy_write32(emi_setting->DRAMC_ACTIM1_VAL, 0x1E8);
	dramc0_phy_write32(0, 0x10);
	dramc0_phy_write32(0xEDCB000F, 0xF8);
	dramc0_phy_write32(0x00C80008, 0x1D8);
	dramc0_phy_write32(emi_setting->DRAMC_CONF2_VAL, 0x8);
	emi_write32(0x13000000, DRAMC0_BASE + 0xE0);
	emi_write32(0, DRAMC0_BASE + 0x124);
	emi_write32(0x40404040, DRAMC0_BASE + 0x94);
	tmp = read32((void *)(DRAMC0_BASE + 0x1E4));
	emi_write32(tmp | (1 << 8), DRAMC0_BASE + 0x1E4);

	/* RX DQS, DQ, DM delay setting */
	if ((emi_setting->DRAMC_MCKDLY_VAL & 0x80000000) == 0x0) {
		emi_write32(0x05070105, DRAMC0_BASE + 0x210); /* DQ0 ~ 3 */
		emi_write32(0x02060505, DRAMC0_BASE + 0x214); /* DQ4 ~ 7 */
		emi_write32(0x06080608, DRAMC0_BASE + 0x218); /* DQ8 ~ 11 */
		emi_write32(0x03050004, DRAMC0_BASE + 0x21C); /* DQ12 ~ 15 */
		emi_write32(0x05030102, DRAMC0_BASE + 0x220); /* DQ16 ~ 19 */
		emi_write32(0x02050504, DRAMC0_BASE + 0x224); /* DQ20 ~ 23 */
		emi_write32(0x03050406, DRAMC0_BASE + 0x228); /* DQ24 ~ 27 */
		emi_write32(0x03050002, DRAMC0_BASE + 0x22C); /* DQ28 ~ 31 */
		dramc0_phy_write32(0x1A1A1A1A, 0x18); /* postsim DQS0 ~ 3 */
	} else {
		emi_write32(0x0D0C0608, DRAMC0_BASE + 0x210);
		emi_write32(0x0707080C, DRAMC0_BASE + 0x214);
		emi_write32(0x0F0F0F0F, DRAMC0_BASE + 0x218);
		emi_write32(0x0B0C080D, DRAMC0_BASE + 0x21C);
		emi_write32(0x0F0F0C0F, DRAMC0_BASE + 0x220);
		emi_write32(0x070A060A, DRAMC0_BASE + 0x224);
		emi_write32(0x0F0F0F0F, DRAMC0_BASE + 0x228);
		emi_write32(0x0F0F0F0F, DRAMC0_BASE + 0x22C);
		dramc0_phy_write32(0x14141411, 0x18);
	}

	/* TX DQS, DQ, DM delay setting */
	if ((emi_setting->DRAMC_MCKDLY_VAL & 0x80000000) == 0x0)
		emi_write32(emi_setting->DRAMC_PADCTL3_VAL, DDRPHY_BASE + 0x10);
	else
		emi_write32(0x0FF0DDDD, DDRPHY_BASE + 0x10);

	emi_write32(emi_setting->DRAMC_PADCTL3_VAL & 0x0000FFFF,
		    DDRPHY_BASE + 0x14);
	/* DQ0 ~ 7 */
	emi_write32(emi_setting->DRAMC_DQODLY_VAL, DDRPHY_BASE + 0x200);
	/* DQ8 ~ 15 */
	emi_write32(emi_setting->DRAMC_DQODLY_VAL, DDRPHY_BASE + 0x204);
	/* DQ16 ~ 23 */
	emi_write32(emi_setting->DRAMC_DQODLY_VAL, DDRPHY_BASE + 0x208);
	/* DQ24 ~ 31 */
	emi_write32(emi_setting->DRAMC_DQODLY_VAL, DDRPHY_BASE + 0x20C);
}

static int mt_get_mdl_number(void)
{
	return 0;
}

int mt_get_dram_type(void)
{
	int n;

	n = mt_get_mdl_number();

	if (n < 0 || n >= num_of_emi_records)
		return 0; /* invalid */

	return (emi_settings[n].type & 0xF);
}

void mt_set_emi(void)
{
	int index = 0;
	unsigned int val1;
	EMI_SETTINGS *emi_set;

	DEBUG_EMI(BIOS_INFO, "DDR%d\r\n", mt_get_dram_type());

	index = mt_get_mdl_number();

	if (index < 0 || index >= num_of_emi_records) {
		DEBUG_EMI(BIOS_INFO, "setting failed 0x%x\r\n", index);
		return;
	}

	DEBUG_EMI(BIOS_INFO, "MDL number = %d\r\n", index);

	emi_set = &emi_settings[index];

	if (((emi_set->type & 0xF) == 1) || (emi_set->type & 0xF) == 2) {
		DEBUG_EMI("Not support DDR1 or DDR2!\n");
		while (1);
	} else {
		init_dram3(emi_set);
	}

	if (dramc_calib() < 0) {
		DEBUG_EMI(BIOS_INFO, "[EMI] DRAMC calibration failed\n\r");
		mtk_arch_reset(1);
	} else {
		DEBUG_EMI(BIOS_INFO, "[EMI] DRAMC calibration passed\n\r");
	}

	/* Enable Gating windown HW calibration function */
	val1 = (DRAMC_READ_REG(0x1C0) & 0x7FFFFFFF) | (0x1 << 31);
	DRAMC_WRITE_REG(val1, 0x1C0);

	/* Enable out of order function */
	val1 = (DRAMC_READ_REG(0x1EC) & 0xFFFFF8EF) |
	       (0x1 << 4) |
	       (0x1 << 10) |
	       (0x1 << 8) |
	       (0x1 << 9);
	DRAMC_WRITE_REG(val1, 0x1EC);

	/* disable CMPPD  */
	val1 = (DRAMC_READ_REG(0x1E4) | (0x1 << 13));
	DRAMC_WRITE_REG(val1, 0x1E4);
}
