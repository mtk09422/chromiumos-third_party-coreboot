/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 MediaTek Inc.
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

#include <delay.h>
#include <arch/io.h>
#include <soc/mt8135.h>
#include <soc/pll.h>

#define DRV_Reg32(addr)             read32((void *)(addr))
#define DRV_WriteReg32(addr, data)  write32(data, (void *)(addr))

#define ACD_TEST_MODE

#define DRAMC_WRITE_REG(val, offset) \
	do { \
		(*(volatile unsigned int *)(DRAMC0_BASE + (offset))) = \
			(unsigned int)(val); \
		(*(volatile unsigned int *)(DDRPHY_BASE + (offset))) =	\
			(unsigned int)(val); \
	} while (0)

/***********************
* MEMPLL Configuration
***********************/

#define r_bias_en_stb_time            (0x00000000 << 24)
#define r_bias_lpf_en_stb_time        (0x00000000 << 16)
#define r_mempll_en_stb_time          (0x00000000 << 8)
#define r_dmall_ck_en_stb_time        (0x00000000 << 0)

#define r_dds_en_stb_time             (0x00000000 << 24)
#define r_div_en_stb_time             (0x00000000 << 16)
#define r_dmpll2_ck_en_stb_time       (0x00000000 << 8)
#define r_iso_en_stb_time             (0x00000000 << 0)

#define r_bias_en_stb_dis             (0x00000001 << 28)
#define r_bias_en_src_sel             (0x00000001 << 24)
#define r_bias_lpf_en_stb_dis         (0x00000001 << 20)
#define r_bias_lpf_en_src_sel         (0x00000001 << 16)
#define r_mempll4_en_stb_dis          (0x00000001 << 15)
#define r_mempll3_en_stb_dis          (0x00000001 << 14)
#define r_mempll2_en_stb_dis          (0x00000001 << 13)
#define r_mempll_en_stb_dis           (0x00000001 << 12)
#define r_mempll4_en_src_sel          (0x00000001 << 11)
#define r_mempll3_en_src_sel          (0x00000001 << 10)
#define r_mempll2_en_src_sel          (0x00000001 << 9)
#define r_mempll_en_src_sel           (0x00000001 << 8)
#define r_dmall_ck_en_stb_dis         (0x00000001 << 4)
#define r_dmall_ck_en_src_sel         (0x00000001 << 0)

#define r_dds_en_stb_dis              (0x00000001 << 28)
#define r_dds_en_src_sel              (0x00000001 << 24)
#define r_div_en_stb_dis              (0x00000001 << 20)
#define r_div_en_src_sel              (0x00000001 << 16)
#define r_dmpll2_ck_en_stb_dis        (0x00000001 << 12)
#define r_dmpll2_ck_en_src_sel        (0x00000001 << 8)
#define r_iso_en_stb_dis              (0x00000001 << 4)
#define r_iso_en_src_sel              (0x00000001 << 0)

#define r_dmbyp_pll4                  (0x00000001 << 0)
#define r_dmbyp_pll3                  (0x00000001 << 1)
#define r_dm1pll_sync_mode            (0x00000001 << 2)
#define r_dmall_ck_en                 (0x00000001 << 4)
#define r_dmpll2_clk_en               (0x00000001 << 5)
/* common setting for 0x180<<2 */
unsigned int pllc1_prediv_1_0_n1 = 0x00000000 << 16;    /* 180[17:16] */
unsigned int pllc1_postdiv_1_0_n5 = 0x00000000 << 14;   /* 180[15:14] */
unsigned int pllc1_blp = 0x00000001 << 12;              /* 180[12] */

/* end of common setting */
unsigned int pllc1_fbksel_1_0_n3 = 0x00000000 << 26;
unsigned int pllc1_dmss_pcw_ncpo_30_24_n4 = 0x00000050 << 25;
unsigned int pllc1_dmss_pcw_ncpo_23_0_n4 = 0x00d8fc50 << 1;

#define pllc1_dmss_pcw_ncpo_chg       (0x00000001 << 0)
#define pllc1_mempll_div_en           (0x00000001 << 24)
#define pllc1_mempll_div_6_0          (0x00000052 << 25)
#ifdef BYPASS_MEMPLL1
#define pllc1_mempll_refck_en     (0x00000000 << 13)
#else
#define pllc1_mempll_refck_en     (0x00000001 << 13)
#endif
#define pllc1_mempll_top_reserve_2_0  (0x00000000 << 16)        /* 182[18:16] */
#define pllc1_mempll_top_reserve_3_3  (0x00000000 << 19)        /* 182[19] */
#define pllc1_mempll_bias_en          (0x00000001 << 14)
#define pllc1_mempll_bias_lpf_en      (0x00000001 << 15)
#define pllc1_mempll_en               (0x00000001 << 2)
#define pllc1_dmss_ncpo_en            (0x00000001 << 4)
#define pllc1_dmss_fifo_start_man     (0x00000001 << 11)
#define pllc1_mempll_dds_en           (0x00000001 << 25)
#define mempll2_prediv_1_0            (0x00000000 << 0)
#define mempll2_vco_div_sel           (0x00000000 << 29)

unsigned int mempll2_m4pdiv_1_0 = (0x00000000 << 10);
unsigned int mempll2_fbdiv_6_0 = (0x0000000c << 2);
unsigned int mempll2_fb_mck_sel = (0x00000001 << 9);

#define mempll2_fbksel_1_0     (0x00000000 << 10)
#define mempll2_posdiv_1_0     (0x00000000 << 30)
#define mempll2_ref_dl_4_0     (0x00000000 << 27)
#define mempll2_fb_dl_4_0      (0x00000000 << 22)
#define mempll2_en             (0x00000001 << 18)
#define mempll3_prediv_1_0     (0x00000000 << 0)
#define mempll3_vco_div_sel    (0x00000000 << 29)

unsigned int mempll3_m4pdiv_1_0 = (0x00000000 << 10);
unsigned int mempll3_fbdiv_6_0 = (0x0000000c << 2);
unsigned int mempll3_fb_mck_sel = (0x00000001 << 9);

#define mempll3_fbksel_1_0   (0x00000000 << 10)
#define mempll3_posdiv_1_0   (0x00000000 << 30)
#define mempll3_ref_dl_4_0   (0x00000000 << 27)
#define mempll3_fb_dl_4_0    (0x00000000 << 22)
#define mempll3_en           (0x00000001 << 18)
#define mempll4_prediv_1_0   (0x00000000 << 0)
#define mempll4_vco_div_sel  (0x00000000 << 29) /*  */

unsigned int mempll4_m4pdiv_1_0 = (0x00000000 << 10);
unsigned int mempll4_fbdiv_6_0 = (0x0000000c << 2);
unsigned int mempll4_fb_mck_sel = (0x00000001 << 9);

#define mempll4_fbksel_1_0  (0x00000000 << 10)
#define mempll4_posdiv_1_0  (0x00000000 << 30)
#define mempll4_ref_dl_4_0  (0x00000000 << 27)
#define mempll4_fb_dl_4_0   (0x00000000 << 22)
#define mempll4_en          (0x00000001 << 18)

/* new add */
#define mempll2_bp   (0x00000001 << 27) /* 183[27] */
#define mempll2_br   (0x00000001 << 26) /* 183[26] */

#define mempll3_bp   (0x00000001 << 27) /* 185[27] */
#define mempll3_br   (0x00000001 << 26) /* 185[26] */

#define mempll4_bp   (0x00000001 << 27) /* 187[27] */
#define mempll4_br   (0x00000001 << 26) /* 187[26] */

#define DRAMC_READ_REG(offset)  (*(volatile unsigned int *)(DDRPHY_BASE + (offset)))

static void mt_mempll_init(void)
{
	unsigned int temp;

	/*
	 * (1) Setup DDRPHY operation mode
	 */

	temp = 0x00000001;      /* set div2 mode  before mempll setting */
	DRAMC_WRITE_REG(temp, DRAMC0_BASE + (0x001f << 2));
	DRAMC_WRITE_REG(temp, DDRPHY_BASE + (0x001f << 2));

	temp = (0x00000010);    /* 3PLL sync mode */
	DRAMC_WRITE_REG(temp, (0x0190 << 2));

	/*
	 * (2) Setup MEMPLL operation case & frequency, May set according to
	 * dram type & frequency
	 */

	temp = r_bias_en_stb_time |
	       r_bias_lpf_en_stb_time |
	       r_mempll_en_stb_time |
	       r_dmall_ck_en_stb_time;
	DRAMC_WRITE_REG(temp, (0x0170 << 2));

	temp = r_dds_en_stb_time |
	       r_div_en_stb_time |
	       r_dmpll2_ck_en_stb_time |
	       r_iso_en_stb_time;
	DRAMC_WRITE_REG(temp, (0x0171 << 2));

	temp = r_bias_en_stb_dis | r_bias_en_src_sel |
	       r_bias_lpf_en_stb_dis | r_bias_lpf_en_src_sel |
	       r_mempll4_en_stb_dis | r_mempll3_en_stb_dis |
	       r_mempll2_en_stb_dis | r_mempll_en_stb_dis |
	       r_mempll4_en_src_sel | r_mempll3_en_src_sel |
	       r_mempll2_en_src_sel | r_mempll_en_src_sel |
	       r_dmall_ck_en_stb_dis | r_dmall_ck_en_src_sel;
	DRAMC_WRITE_REG(temp, (0x0172 << 2));

	temp = r_dds_en_stb_dis | r_dds_en_src_sel |
	       r_div_en_stb_dis | r_div_en_src_sel |
	       r_dmpll2_ck_en_stb_dis | r_dmpll2_ck_en_src_sel |
	       r_iso_en_stb_dis | r_iso_en_src_sel |
	       r_dmbyp_pll4 | r_dmbyp_pll3 |
	       r_dm1pll_sync_mode | r_dmall_ck_en | r_dmpll2_clk_en;
	DRAMC_WRITE_REG(temp, (0x0173 << 2));

	temp = pllc1_prediv_1_0_n1 | pllc1_blp |
	       pllc1_fbksel_1_0_n3 | pllc1_postdiv_1_0_n5;
	DRAMC_WRITE_REG(temp, (0x0180 << 2));

	temp = pllc1_mempll_div_6_0 | pllc1_mempll_refck_en;
	DRAMC_WRITE_REG(temp, (0x0181 << 2));

	temp = mempll2_vco_div_sel | mempll2_m4pdiv_1_0 | mempll2_fb_mck_sel |
	       mempll2_posdiv_1_0 | mempll2_bp | mempll2_br;
	DRAMC_WRITE_REG(temp, (0x0183 << 2));

	temp = pllc1_mempll_top_reserve_2_0 | mempll2_prediv_1_0 |
	       mempll2_fbdiv_6_0 | mempll2_fbksel_1_0 |
	       pllc1_mempll_top_reserve_3_3;
	temp = temp & (0xFFF7FFFF);     /* For 8135 MEMPLL top_reserve_3 bit19=0 */
	DRAMC_WRITE_REG(temp, (0x0182 << 2));

	temp = mempll3_vco_div_sel | mempll3_m4pdiv_1_0 | mempll3_fb_mck_sel |
	       mempll3_posdiv_1_0 | mempll3_bp | mempll3_br;
	DRAMC_WRITE_REG(temp, (0x0185 << 2));

	temp = mempll2_ref_dl_4_0 | mempll2_fb_dl_4_0 | mempll3_prediv_1_0 |
	       mempll3_fbdiv_6_0 | mempll3_fbksel_1_0;
	DRAMC_WRITE_REG(temp, (0x0184 << 2));

	temp = mempll4_vco_div_sel | mempll4_m4pdiv_1_0 | mempll4_fb_mck_sel |
	       mempll4_posdiv_1_0 | mempll4_bp | mempll4_br;
	DRAMC_WRITE_REG(temp, (0x0187 << 2));

	temp = mempll3_fb_dl_4_0 | mempll3_ref_dl_4_0 | mempll4_prediv_1_0 |
	       mempll4_fbdiv_6_0 | mempll4_fbksel_1_0;
	DRAMC_WRITE_REG(temp, (0x0186 << 2));

	temp = mempll4_ref_dl_4_0 | mempll4_fb_dl_4_0;
	DRAMC_WRITE_REG(temp, (0x0188 << 2));

	temp = pllc1_dmss_pcw_ncpo_23_0_n4 | pllc1_dmss_pcw_ncpo_30_24_n4 |
	       pllc1_dmss_pcw_ncpo_chg;
	DRAMC_WRITE_REG(temp, (0x0189 << 2));

	/* SDM PLL tie high default value */
	DRAMC_WRITE_REG(0x003F0000, (0x018C << 2));

	/*
	 * (3) Setup MEMPLL power on sequence
	 */
	udelay(5);              /* min delay is 2us */

	temp = pllc1_mempll_div_6_0 |
	       pllc1_mempll_refck_en |
	       pllc1_mempll_bias_en;
	DRAMC_WRITE_REG(temp, (0x0181 << 2));

	udelay(5);              /* min delay is 1us */

	temp = pllc1_mempll_div_6_0 |
	       pllc1_mempll_refck_en |
	       (pllc1_mempll_bias_en | pllc1_mempll_bias_lpf_en);
	DRAMC_WRITE_REG(temp, (0x0181 << 2));

	udelay(5000);           /* min delay is 1ms */

	temp = pllc1_prediv_1_0_n1 | pllc1_blp | pllc1_fbksel_1_0_n3 |
	       pllc1_postdiv_1_0_n5 | (pllc1_mempll_en);
	DRAMC_WRITE_REG(temp, (0x0180 << 2));

	udelay(50);             /* min delay is 20us */

	temp = 0x10000000;
	DRAMC_WRITE_REG(temp, (0x018a << 2));

	udelay(5);              /* min delay is 2us */

	temp = 0x18000000;
	DRAMC_WRITE_REG(temp, (0x018a << 2));

	temp = mempll4_ref_dl_4_0 |
	       mempll4_fb_dl_4_0 |
	       (pllc1_dmss_ncpo_en | pllc1_dmss_fifo_start_man);
	DRAMC_WRITE_REG(temp, (0x0188 << 2));

	udelay(5);              /* min delay is 2us */

	temp = pllc1_mempll_dds_en |
	       (pllc1_prediv_1_0_n1 | pllc1_blp | pllc1_fbksel_1_0_n3 |
		pllc1_postdiv_1_0_n5 | pllc1_mempll_en);
	DRAMC_WRITE_REG(temp, (0x0180 << 2));

	temp = pllc1_mempll_div_en |
	       (pllc1_mempll_div_6_0 | pllc1_mempll_refck_en |
		(pllc1_mempll_bias_en | pllc1_mempll_bias_lpf_en));
	DRAMC_WRITE_REG(temp, (0x0181 << 2));

	udelay(50);             /* min delay is 23us */

	temp = mempll2_en | mempll2_vco_div_sel | mempll2_m4pdiv_1_0 |
	       mempll2_fb_mck_sel | mempll2_posdiv_1_0 |
	       mempll2_bp | mempll2_br;
	DRAMC_WRITE_REG(temp, (0x0183 << 2));

	temp = mempll3_en | mempll3_vco_div_sel | mempll3_m4pdiv_1_0 |
	       mempll3_fb_mck_sel | mempll3_posdiv_1_0 |
	       mempll3_bp | mempll3_br;
	DRAMC_WRITE_REG(temp, (0x0185 << 2));

	temp = mempll4_en | mempll4_vco_div_sel | mempll4_m4pdiv_1_0 |
	       mempll4_fb_mck_sel | mempll4_posdiv_1_0 |
	       mempll4_bp | mempll4_br;
	DRAMC_WRITE_REG(temp, (0x0187 << 2));

	udelay(30);             /* min delay is 23us */

	/* 3PLL sync mode */
	temp = 0x00003f20 | r_dmpll2_clk_en | r_dmall_ck_en;
	DRAMC_WRITE_REG(temp, (0x0190 << 2));

	/*
	 * MEMPLL control switch to SPM for ISO_EN zero delay
	 */

	/*
	 * [0]ISO_EN_SRC=0,
	 * [22]DIV_EN_SC_SRC=0 (pll2off),
	 * [20]DIV_EN_DLY=1 (no delay),
	 * [16]DIV_EN_SRC=0,
	 * [8]PLL2_CK_EN_SRC=1(1pll),
	 * [8]PLL2_CK_EN_SRC=0(3pll)
	 */
	temp = 0x00101010;
	DRAMC_WRITE_REG(temp, (0x0173 << 2));

	/*
	 * Setting for MEMPLL_EN control by sc_mempll3_off
	 */

	/*
	 * [24]BIAS_EN_SRC=0,
	 * [16]BIAS_LPF_EN_SRC=0,
	 * [8]MEMPLL_EN,
	 * [9][10][11]MEMPLL2,3,4_EN_SRC,
	 * [0]ALL_CK_EN_SRC=0
	 */
	temp = 0x0000F010;
	DRAMC_WRITE_REG(temp, (0x0172 << 2));

	temp = 0x00021B96; /* Setting for delay time, BIAS_LPF_EN delay time=2T */
	DRAMC_WRITE_REG(temp, (0x0170 << 2));

#ifdef ACD_TEST_MODE
	temp = DRAMC_READ_REG(0x0183 << 2);
	temp = (temp & 0xE3FFFFFF);
	DRAMC_WRITE_REG(temp, (0x0183 << 2));

	temp = DRAMC_READ_REG(0x0181 << 2);
	temp = (temp & 0x01FFFFFF) | 0x52000000;
	DRAMC_WRITE_REG(temp, (0x0181 << 2));

	temp = DRAMC_READ_REG(0x0182 << 2);
	temp = (temp & 0xFFFFFE03) | 0x00000018;
	DRAMC_WRITE_REG(temp, (0x0182 << 2));

	temp = DRAMC_READ_REG(0x0189 << 2);
	temp = (temp & 0x00000001) | 0x96254654;
	DRAMC_WRITE_REG(temp, (0x0189 << 2));

	temp = DRAMC_READ_REG(0x0185 << 2);
	temp = (temp & 0xE3FFFFFF);
	DRAMC_WRITE_REG(temp, (0x0185 << 2));

	temp = DRAMC_READ_REG(0x0184 << 2);
	temp = (temp & 0xFFFFFE03) | 0x00000018;
	DRAMC_WRITE_REG(temp, (0x0184 << 2));

	temp = DRAMC_READ_REG(0x0187 << 2);
	temp = (temp & 0xE3FFFFFF);
	DRAMC_WRITE_REG(temp, (0x0187 << 2));

	temp = DRAMC_READ_REG(0x0186 << 2);
	temp = (temp & 0xFFFFFE03) | 0x00000018;
	DRAMC_WRITE_REG(temp, (0x0186 << 2));

	temp = DRAMC_READ_REG(0x0189 << 2);
	temp = (temp | 0x00000001);
	DRAMC_WRITE_REG(temp, (0x0189 << 2));

	temp = DRAMC_READ_REG(0x0189 << 2);
	temp = (temp & 0xFFFFFFFE);
	DRAMC_WRITE_REG(temp, (0x0189 << 2));
#endif
}

static int mt_mempll_cali(void)
{
	int one_count = 0, zero_count = 0;
	int pll2_done = 0, pll3_done = 0, pll4_done = 0, ret = 0;

	unsigned int temp = 0, pll2_dl = 0, pll3_dl = 0, pll4_dl = 0;

	/*
	 * 1. Set jitter meter clock to internal FB path
	 */

	/*
	 * 2. Set jitter meter count number
	 */

	/* 0x100111CC[31:16] PLL2 0x400 = 1024 count */
	temp = DRV_Reg32(0x100111CC) & 0x0000FFFF;
	DRV_WriteReg32(0x100111CC, (temp | 0x04000000));

	/* 0x100111D0[31:16] PLL3 0x400 = 1024 count */
	temp = DRV_Reg32(0x100111D0) & 0x0000FFFF;
	DRV_WriteReg32(0x100111D0, (temp | 0x04000000));

	/* 0x100111D4[31:16] PLL4 0x400 = 1024 count */
	temp = DRV_Reg32(0x100111D4) & 0x0000FFFF;
	DRV_WriteReg32(0x100111D4, (temp | 0x04000000));

	while (1) {
		/*
		 * 3. Adjust delay chain tap number
		 */

		if (!pll2_done) {
			/* 0x10011610[31:27] PLL2 REF 0x10 fix */
			temp = DRV_Reg32(0x10011610) & ~0xF8000000;
			DRV_WriteReg32(0x10011610, (temp | (0x10 << 27)));

			/* 0x10011610[26:22] PLL2 FB inc 1 */
			temp = DRV_Reg32(0x10011610) & ~0x07C00000;
			DRV_WriteReg32(0x10011610, (temp | (pll2_dl << 22)));
		}

		if (!pll3_done) {
			/* 0x10011618[31:27] PLL3 REF 0x10 fix */
			temp = DRV_Reg32(0x10011618) & ~0xF8000000;
			DRV_WriteReg32(0x10011618, (temp | (0x10 << 27)));

			/* 0x10011618[26:22] PLL3 FB inc 1 */
			temp = DRV_Reg32(0x10011618) & ~0x07C00000;
			DRV_WriteReg32(0x10011618, (temp | (pll3_dl << 22)));
		}

		if (!pll4_done) {
			/* 0x10011620[31:27] PLL4 REF 0x10 fix */
			temp = DRV_Reg32(0x10011620) & ~0xF8000000;
			DRV_WriteReg32(0x10011620, (temp | (0x10 << 27)));

			/* 0x10011620[26:22] PLL4 FB inc 1 */
			temp = DRV_Reg32(0x10011620) & ~0x07C00000;
			DRV_WriteReg32(0x10011620, (temp | (pll4_dl << 22)));
		}

		/*
		 * 4. Enable jitter meter
		 */

		if (!pll2_done) {
			/* 0x100111CC[0]=1 PLL2 */
			temp = DRV_Reg32(0x100111CC);
			DRV_WriteReg32(0x100111CC, temp | 0x1);
		}

		if (!pll3_done) {
			/* 0x100111D0[0]=1 PLL3 */
			temp = DRV_Reg32(0x100111D0);
			DRV_WriteReg32(0x100111D0, temp | 0x1);
		}

		if (!pll4_done) {
			/* 0x100111D4[0]=1 PLL4 */
			temp = DRV_Reg32(0x100111D4);
			DRV_WriteReg32(0x100111D4, temp | 0x1);
		}

		udelay(40);     /* wait for jitter meter complete */

		/*
		 * 5. Check jitter meter counter value
		 */

		if (!pll2_done) {
			/* 0x10011320[31:16] PLL2 one count */
			/* 0x10011320[15:0] PLL2 zero count */
			one_count = DRV_Reg32(0x10011320) >> 16;
			zero_count = DRV_Reg32(0x10011320) & 0x0000FFFF;

			if (zero_count > 512)
				pll2_done = 1;
		}

		if (!pll3_done) {
			/* 0x10011324[31:16] PLL3 one count */
			/* 0x10011324[15:0] PLL3 zero count */
			one_count = DRV_Reg32(0x10011324) >> 16;
			zero_count = DRV_Reg32(0x10011324) & 0x0000FFFF;

			if (zero_count > 512)
				pll3_done = 1;
		}

		if (!pll4_done) {
			/* 0x10011328[31:16] PLL4 one count */
			/* 0x10011328[15:0] PLL4 zero count */
			one_count = DRV_Reg32(0x10011328) >> 16;
			zero_count = DRV_Reg32(0x10011328) & 0x0000FFFF;

			if (zero_count > 512)
				pll4_done = 1;
		}

		/*
		 * 6. Reset jitter meter value
		 */

		if (!pll2_done) {
			pll2_dl++;

			/* 0x100111CC[0]=0 PLL2 */
			temp = DRV_Reg32(0x100111CC);
			DRV_WriteReg32(0x100111CC, temp & ~0x1);
		}

		if (!pll3_done) {
			pll3_dl++;

			/* 0x100111D0[0]=0 PLL3 */
			temp = DRV_Reg32(0x100111D0);
			DRV_WriteReg32(0x100111D0, temp & ~0x1);
		}

		if (!pll4_done) {
			pll4_dl++;

			/* 0x100111D4[0]=0 PLL4 */
			temp = DRV_Reg32(0x100111D4);
			DRV_WriteReg32(0x100111D4, temp & ~0x1);
		}

		/*
		 * Then return to step 1 to adjust next delay chain tap value.
		 * Until we have ~ 50% of one or zero count on jitter meter
		 */

		if (pll2_done && pll3_done && pll4_done) {
			ret = 0;
			break;
		}

		if (pll2_dl >= 32 || pll3_dl >= 32 || pll4_dl >= 32) {
			ret = -1;
			break;
		}
	}

	/*
	 * 7. Set jitter meter clock to external FB path
	 */

	temp = DRV_Reg32(0x1001160C);
	DRV_WriteReg32(0x1001160C, temp | 0x200); /* 0x1001160C[9] = 1 PLL2 */

	temp = DRV_Reg32(0x10011614);
	DRV_WriteReg32(0x10011614, temp | 0x200); /* 0x10011614[9] = 1 PLL3 */

	temp = DRV_Reg32(0x1001161C);
	DRV_WriteReg32(0x1001161C, temp | 0x200); /* 0x1001161C[9] = 1 PLL4 */

	return ret;
}

static void init_mainpll(void)
{
	unsigned int temp;
	int already_on;

	/* MAINPLL */
	temp = DRV_Reg32(MAINPLL_CON0);
	already_on = temp & 0x1;

	if (already_on) {
		return;
	} else {
		temp = DRV_Reg32(MAINPLL_PWR_CON0);
		DRV_WriteReg32(MAINPLL_PWR_CON0, temp | 0x1);

		udelay(10);     /* wait for PWR ready (min delay is 2us) */

		temp = DRV_Reg32(MAINPLL_PWR_CON0);
		DRV_WriteReg32(MAINPLL_PWR_CON0, temp & ~0x2);
	}

	DRV_WriteReg32(MAINPLL_CON1, 0x800F8000);

	if (!already_on) {
		temp = DRV_Reg32(MAINPLL_CON0);
		DRV_WriteReg32(MAINPLL_CON0, temp | 0x1);
	}

	udelay(40);             /* wait for PLL stable (min delay is 20us) */

	temp = DRV_Reg32(MAINPLL_CON0);
	DRV_WriteReg32(MAINPLL_CON0, temp | 0x08000000); /* MAINPLL_DIV_RSTB = 1 */
}

static void init_msdcpll(void)
{
	unsigned int temp;
	int already_on;

	temp = DRV_Reg32(MSDCPLL_CON0);
	already_on = temp & 0x1;

	if (already_on) {
		return;
	} else {
		temp = DRV_Reg32(MSDCPLL_PWR_CON0);
		DRV_WriteReg32(MSDCPLL_PWR_CON0, temp | 0x1);

		udelay(10);     /* wait for PWR ready (min delay is 2us) */

		temp = DRV_Reg32(MSDCPLL_PWR_CON0);
		DRV_WriteReg32(MSDCPLL_PWR_CON0, temp & ~0x2);
	}

	/* MSDCPLL@208MHz: MSDCPLL_CON1 = 0x80100000 */
	DRV_WriteReg32(MSDCPLL_CON1, 0x80100000);

	if (!already_on) {
		temp = DRV_Reg32(MSDCPLL_CON0);
		DRV_WriteReg32(MSDCPLL_CON0, temp | 0x1);
	}

	udelay(40);             /* wait for PLL stable (min delay is 20us) */
}

static void init_univpll(void)
{
	unsigned int temp;
	int already_on;

	/* UNIVPLL */
	temp = DRV_Reg32(UNIVPLL_CON0);
	already_on = temp & 0x1;

	if (already_on) {
		return;
	} else {
		/* UNIVPLL@624MHz, 416MHz, 249.6MHz, 178.3MHz, 48MHz */
		DRV_WriteReg32(UNIVPLL_CON0, 0xF3006001);
		udelay(40);     /* wait for PLL stable (min delay is 20us) */

		/* UNIVPLL_DIV_RSTB = 1 */
		DRV_WriteReg32(UNIVPLL_CON0, 0xFB006001);
	}
}

static void init_mmpll(void)
{
	unsigned int temp;
	int already_on;

	/* MMPLL */
	temp = DRV_Reg32(MMPLL_CON0);
	already_on = temp & 0x1;

	if (already_on) {
		return;
	} else {
		temp = DRV_Reg32(MMPLL_PWR_CON0);
		DRV_WriteReg32(MMPLL_PWR_CON0, temp | 0x1);

		udelay(10);     /* wait for PWR ready (min delay is 2us) */

		temp = DRV_Reg32(MMPLL_PWR_CON0);
		DRV_WriteReg32(MMPLL_PWR_CON0, temp & ~0x2);
	}

	/* MMPLL@1185MHz: MMPLL_CON1 = 0xF00B64EC */
	DRV_WriteReg32(MMPLL_CON1, 0xF00B64EC);

	if (!already_on)
		DRV_WriteReg32(MMPLL_CON0, 0xF0006E01);

	udelay(40);                             /* wait for PLL stable (min delay is 20us) */
	DRV_WriteReg32(MMPLL_CON0, 0xF8006E01); /* MMPLL_DIV_RSTB = 1 */
}

static void init_vdecpll(void)
{
	unsigned int temp;
	int already_on;

	/* VDECPLL */
	temp = DRV_Reg32(VDECPLL_CON0);
	already_on = temp & 0x1;

	if (already_on) {
		return;
	} else {
		temp = DRV_Reg32(VDECPLL_PWR_CON0);
		DRV_WriteReg32(VDECPLL_PWR_CON0, temp | 0x1);

		udelay(10);     /* wait for PWR ready (min delay is 2us) */

		temp = DRV_Reg32(VDECPLL_PWR_CON0);
		DRV_WriteReg32(VDECPLL_PWR_CON0, temp & ~0x2);
	}

	/* VDECPLL@326MHz: VDECPLL_CON1 = 0x800C89D8 */
	DRV_WriteReg32(VDECPLL_CON1, 0x800C89D8);

	if (!already_on) {
		temp = DRV_Reg32(VDECPLL_CON0);
		DRV_WriteReg32(VDECPLL_CON0, temp | 0x1);
	}

	udelay(40);             /* wait for PLL stable (min delay is 20us) */
}

static void init_audpll(void)
{
	unsigned int temp;
	int already_on;

	/* AUDPLL */
	temp = DRV_Reg32(AUDPLL_CON0);
	already_on = temp & 0x1;

	if (already_on) {
		return;
	} else {
		temp = DRV_Reg32(AUDPLL_PWR_CON0);
		DRV_WriteReg32(AUDPLL_PWR_CON0, temp | 0x1);

		udelay(10);     /* wait for PWR ready (min delay is 2us) */

		temp = DRV_Reg32(AUDPLL_PWR_CON0);
		DRV_WriteReg32(AUDPLL_PWR_CON0, temp & ~0x2);
	}

	/* AUDPLL@98.304MHz: AUDPLL_CON1 = 0xBC7EA932 */
	DRV_WriteReg32(AUDPLL_CON1, 0xBC7EA932);

	if (!already_on) {
		temp = DRV_Reg32(AUDPLL_CON0);
		DRV_WriteReg32(AUDPLL_CON0, temp | 0x1);
	}

	udelay(40);             /* wait for PLL stable (min delay is 20us) */
}

static void init_armpll(void)
{
	unsigned int temp;
	int already_on;

	/* ARMPLL */
	temp = DRV_Reg32(ARMPLL_CON0);
	already_on = temp & 0x1;

	if (already_on) {
		return;
	} else {
		temp = DRV_Reg32(ARMPLL_PWR_CON0);
		DRV_WriteReg32(ARMPLL_PWR_CON0, temp | 0x1);

		udelay(10);     /* wait for PWR ready (min delay is 2us) */

		temp = DRV_Reg32(ARMPLL_PWR_CON0);
		DRV_WriteReg32(ARMPLL_PWR_CON0, temp & ~0x2);
	}

	/*
	 * ARMPLL1@1118MHz: ARMPLL_CON1 = 0x800AC000
	 * ARMPLL1@1092MHz: ARMPLL_CON1 = 0x800A8000
	 * ARMPLL1@546MHz:  ARMPLL_CON1 = 0x810A8000
	 * ARMPLL1@273MHz:  ARMPLL_CON1 = 0x820A8000
	 */
	DRV_WriteReg32(ARMPLL_CON1, 0x800AC000);

	if (!already_on) {
		temp = DRV_Reg32(ARMPLL_CON0);
		DRV_WriteReg32(ARMPLL_CON0, temp | 0x1);
	}

	udelay(40);             /* wait for ARMPLL stable (min delay is 20us) */
}

static void init_arm2pll(void)
{
	unsigned int temp;
	int already_on;

	/* ARM2PLL */
	temp = DRV_Reg32(ARMPLL2_CON0);
	already_on = temp & 0x1;

	if (already_on) {
		return;
	} else {
		temp = DRV_Reg32(ARMPLL2_PWR_CON0);
		DRV_WriteReg32(ARMPLL2_PWR_CON0, temp | 0x1);

		udelay(10);     /* wait for PWR ready (min delay is 2us) */

		temp = DRV_Reg32(ARMPLL2_PWR_CON0);
		DRV_WriteReg32(ARMPLL2_PWR_CON0, temp & ~0x2);
	}

	/*
	 * ARMPLL2@988MHz: ARMPLL2_CON1 = 0x81130000
	 * ARMPLL2@916MHz: ARMPLL2_CON1 = 0x8111A000
	 * ARMPLL2@458MHz: ARMPLL2_CON1 = 0x8211A000
	 * ARMPLL2@229MHz: ARMPLL2_CON1 = 0x8311A000
	 */
	DRV_WriteReg32(ARMPLL2_CON1, 0x81130000);

	if (!already_on) {
		temp = DRV_Reg32(ARMPLL2_CON0);
		DRV_WriteReg32(ARMPLL2_CON0, temp | 0x1);
	}

	udelay(40);             /* wait for PLL stable (min delay is 20us) */
}

static void init_tvdpll(void)
{
	unsigned int temp;
	int already_on;

	/* TVDPLL */
	temp = DRV_Reg32(TVDPLL_CON0);
	already_on = temp & 0x1;

	if (already_on) {
		return;
	} else {
		temp = DRV_Reg32(TVDPLL_PWR_CON0);
		DRV_WriteReg32(TVDPLL_PWR_CON0, temp | 0x1);

		udelay(10);     /* wait for PWR ready (min delay is 2us) */

		temp = DRV_Reg32(TVDPLL_PWR_CON0);
		DRV_WriteReg32(TVDPLL_PWR_CON0, temp & ~0x2);
	}

	/* TVDPLL@148.5MHz: TVDPLL_CON1 = 0xADB13B14 */
	DRV_WriteReg32(TVDPLL_CON1, 0xADB13B14);

	if (!already_on) {
		temp = DRV_Reg32(TVDPLL_CON0);
		DRV_WriteReg32(TVDPLL_CON0, temp | 0x1);
	}

	udelay(40);             /* wait for PLL stable (min delay is 20us) */
}

static void init_lvdspll(void)
{
	unsigned int temp;
	int already_on;

	/* TVDPLL */
	temp = DRV_Reg32(LVDSPLL_CON0);
	already_on = temp & 0x1;

	if (already_on) {
		return;
	} else {
		temp = DRV_Reg32(LVDSPLL_PWR_CON0);
		DRV_WriteReg32(LVDSPLL_PWR_CON0, temp | 0x1);

		udelay(10);     /* wait for PWR ready (min delay is 2us) */

		temp = DRV_Reg32(LVDSPLL_PWR_CON0);
		DRV_WriteReg32(LVDSPLL_PWR_CON0, temp & ~0x2);
	}

	/* LVDSPLL@75MHz: LVDSPLL_CON1 = 0x800B89D9 */
	DRV_WriteReg32(LVDSPLL_CON1, 0x800B89D9);

	if (!already_on)
		DRV_WriteReg32(LVDSPLL_CON0, 0x80000101);

	udelay(40);             /* wait for PLL stable (min delay is 20us) */
}

static void enable_deglitch(void)
{
	const unsigned long CLK_BIAS_CON2 = 0x100005E4;
	const unsigned long deglitch_magic_key = 0x67D2A357;

	DRV_WriteReg32(CLK_BIAS_CON2, deglitch_magic_key);
}

static unsigned int get_chip_sw_ver_code(void)
{
	return DRV_Reg32(APSW_VER);
}

void mt_pll_post_init(void)
{
	DRV_WriteReg32(INFRA_PDN_SET, 0x00008040);
	DRV_WriteReg32(PERI_PDN0_SET, 0x002C1BFC);
	DRV_WriteReg32(PERI_PDN1_SET, 0x00000008);
	DRV_WriteReg32(PERI_PDN1_CLR, 0x00000008);
}

int mt_pll_init(void)
{
	int ret = 0;
	unsigned int ver = get_chip_sw_ver_code();
	unsigned int temp;

	enable_deglitch();

	/* CLKSQ1, CLKSQ2 */
	DRV_WriteReg32(AP_PLL_CON0, 0x00001133); /* enable CLKSQ1 low pass filter */

	init_arm2pll();
	init_armpll();
	init_mainpll();
	init_msdcpll();
	init_univpll();
	init_mmpll();

	/* MEMPLL Init */
	mt_mempll_init();

	/* MEMPLL Calibration */
	ret = mt_mempll_cali();

	init_vdecpll();
	init_audpll();
	init_tvdpll();
	init_lvdspll();

	temp = DRV_Reg32(TOP_DCMCTL);
	DRV_WriteReg32(TOP_DCMCTL, temp | 0x1); /* enable infrasys DCM */

	DRV_WriteReg32(CLK_MODE, 0x0);          /* output clock is enabled */

	/* Clock Divider */
	DRV_WriteReg32(TOP_CKDIV1, 0x0);        /* CPU clock divide by 1 */

	DRV_WriteReg32(TOP_CKDIV2, 0x0);        /* CPU (CA15) clock divide by 1 */

	/* Clock Mux */
	temp = DRV_Reg32(TOP_CKMUXSEL) & ~0xC;
	DRV_WriteReg32(TOP_CKMUXSEL, temp | 0x4); /* switch CPU clock to ARMPLL */

	/* switch CPU clock to ARMPLL (CA15PLL) */
	temp = DRV_Reg32(TOP_CKMUXSEL) & ~0x30;
	DRV_WriteReg32(TOP_CKMUXSEL, temp | 0x10);

	if (ver == 0x0000) {
		/* E1 */
		DRV_WriteReg32(CLK_CFG_0, 0x02050304);
		DRV_WriteReg32(CLK_CFG_1, 0x04060104);
		DRV_WriteReg32(CLK_CFG_2, 0x05050505);
		DRV_WriteReg32(CLK_CFG_3, 0x80808002);
		DRV_WriteReg32(CLK_CFG_4, 0x01010300);
		DRV_WriteReg32(CLK_CFG_6, 0x00000201);
		DRV_WriteReg32(CLK_CFG_7, 0x02010102);
		DRV_WriteReg32(CLK_CFG_8, 0x01010501);
		DRV_WriteReg32(CLK_CFG_9, 0x01010101);
		DRV_WriteReg32(DCM_CFG, 0x10);
	} else {
		/* E2 */
		DRV_WriteReg32(CLK_CFG_0, 0x02080601);
		DRV_WriteReg32(CLK_CFG_1, 0x05030101);
		DRV_WriteReg32(CLK_CFG_2, 0x05050505);
		DRV_WriteReg32(CLK_CFG_3, 0x80808001);
		DRV_WriteReg32(CLK_CFG_4, 0x01010107);
		DRV_WriteReg32(CLK_CFG_6, 0x00010201);
		DRV_WriteReg32(CLK_CFG_7, 0x01010104);
		DRV_WriteReg32(CLK_CFG_8, 0x01010501);
		DRV_WriteReg32(CLK_CFG_9, 0x01010101);
		DRV_WriteReg32(DCM_CFG, 0x10);
	}

	/* Change CLKSQ, ARMPLL1, ARMPLL2, MAINPLL, UNIVPLL and MMPLL to HW */
	/* control */
	DRV_WriteReg32(AP_PLL_CON3, 0x0);
	DRV_WriteReg32(AP_PLL_CON1, 0x0);

	/* HW Control */
	temp = DRV_Reg32(CLK_CFG_5);
	DRV_WriteReg32(CLK_CFG_5, temp | 0x3FF);

	/* HW Control */
	temp = DRV_Reg32(CLK_MISC_CFG_2);
	DRV_WriteReg32(CLK_MISC_CFG_2, temp | 0x11);

	return ret;
}
