/*
 * Copyright (c) 2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <console/console.h>
#include <delay.h>
#include <arch/io.h>
#include <soc/addressmap.h>
#include "clk_rst.h"
#include "clock.h"
#include "flow.h"
#include "pmc.h"

static struct clk_rst_ctlr *clk_rst = (void *)TEGRA_CLK_RST_BASE;
static struct flow_ctlr *flow = (void *)TEGRA_FLOW_BASE;
static struct pmc_ctlr *pmc = (void*)TEGRA_PMC_BASE;

/* only needed in this one place. Avoid namespace pollution. Be green .*/
struct clk_pll_table {
	u16	n;
	u16	m;
	u8	p;
	u8	cpcon;
};

/*
 * Timing tables
 */
struct clk_pll_table tegra_pll_x_table[16] = {
	/* T124: 1.9 GHz    */
	/*
	 * Field Bits Width
	 *  n    15:8   8
	 *  m     7:0   8
	 *  p    23:20  4
	 */
	[OSC_FREQ_OSC13]{216,13,1,8},
	[OSC_FREQ_OSC19P2]{180,16,1,4},
	[OSC_FREQ_OSC12]{216,12,1,8},
	[OSC_FREQ_OSC26]{216,26,1,8},
	[OSC_FREQ_OSC16P8]{180,14,1,4},
	[OSC_FREQ_OSC38P4]{180,16,1,4},
	[OSC_FREQ_OSC48]{216,12,1,8},
};

/* one of the very few funcs we might move to common code. */
static void clock_ll_set_source_divisor(u32 *reg,
	unsigned source, unsigned divisor)
{
        u32 value;

        value = readl(reg);

        value &= ~OUT_CLK_SOURCE_MASK;
        value |= source << OUT_CLK_SOURCE_SHIFT;

        value &= ~OUT_CLK_DIVISOR_MASK;
        value |= divisor << OUT_CLK_DIVISOR_SHIFT;

        writel(value, reg);
}


/* Get the oscillator frequency, from the corresponding hardware
 * configuration field. This is actually a per-soc thing. Avoid the
 * temptation to make it common.
 */
static int clock_get_osc_freq(void)
{
	u32 reg;
        reg = readl(&clk_rst->crc_osc_ctrl);
        reg >>= OSC_CTRL_OSC_FREQ_SHIFT;
	return reg;
}

static void adjust_pllp_out_freqs(void)
{
	u32 reg;
        struct clk_pll *pll = &clk_rst->crc_pll[2/*CLOCK_ID_PERIPH*/];
	/* Set T30 PLLP_OUT1, 2, 3 & 4 freqs to 9.6, 48, 102 & 204MHz */
	reg = readl(&pll->pll_out[0]);	/* OUTA, contains OUT2 / OUT1 */
	reg |= (IN_408_OUT_48_DIVISOR << PLLP_OUT2_RATIO) | PLLP_OUT2_OVR
		| (IN_408_OUT_9_6_DIVISOR << PLLP_OUT1_RATIO) | PLLP_OUT1_OVR;
	writel(reg, &pll->pll_out[0]);

	reg = readl(&pll->pll_out[1]);   /* OUTB, contains OUT4 / OUT3 */
	reg |= (IN_408_OUT_204_DIVISOR << PLLP_OUT4_RATIO) | PLLP_OUT4_OVR
		| (IN_408_OUT_102_DIVISOR << PLLP_OUT3_RATIO) | PLLP_OUT3_OVR;
	writel(reg, &pll->pll_out[1]);
}

static int pllx_set_rate(struct clk_pll_simple *pll , u32 divn, u32 divm,
		u32 divp, u32 cpcon)
{
	u32 reg;

	/* If PLLX is already enabled, just return */
	if (readl(&pll->pll_base) & PLL_ENABLE_MASK) {
		return 0;
	}


	/* Disable IDDQ */
	reg = readl(&clk_rst->crc_pllx_misc3);
	reg &= ~PLLX_IDDQ_MASK;
	writel(reg, &clk_rst->crc_pllx_misc3);
	udelay(2);

	/* Set BYPASS, m, n and p to PLLX_BASE */
	reg = PLL_BYPASS_MASK | (divm << PLL_DIVM_SHIFT);
	reg |= ((divn << PLL_DIVN_SHIFT) | (divp << PLL_DIVP_SHIFT));
	writel(reg, &pll->pll_base);

	/* Set cpcon to PLLX_MISC */
	reg = (cpcon << PLL_CPCON_SHIFT);

	/* Set dccon to PLLX_MISC if freq > 600MHz - still needed for T124? */
	if (divn > 600)
		reg |= (1 << PLL_DCCON_SHIFT);
	writel(reg, &pll->pll_misc);

	/* Disable BYPASS */
	reg = readl(&pll->pll_base);
	reg &= ~PLL_BYPASS_MASK;
	writel(reg, &pll->pll_base);

	/* Set lock_enable to PLLX_MISC */
	reg = readl(&pll->pll_misc);
	reg |= PLL_LOCK_ENABLE_MASK;
	writel(reg, &pll->pll_misc);

	/* Enable PLLX last, as per JZ */
	reg = readl(&pll->pll_base);
	reg |= PLL_ENABLE_MASK;
	writel(reg, &pll->pll_base);

	return 0;
}

static void init_pllx(void)
{
	int osc;
	struct clk_pll_table *sel;
        struct clk_pll_simple *pll = &clk_rst->crc_pll_simple[0/*SIMPLE_PLLX*/];


	/* get osc freq */
	osc = clock_get_osc_freq();

	/* set pllx */
	sel = &tegra_pll_x_table[osc];
	if (sel->n == 0){
		return;
	}

	pllx_set_rate(pll, sel->n, sel->m, sel->p, sel->cpcon);

	adjust_pllp_out_freqs();
}

void clock_uart_config(void)
{
	/* Enable clocks to required peripherals. TBD - minimize this list */
	/* The UART is super special so Just Do It right here. */

	setbits_le32(clkreset(CLK_UARTA_REG), CLK_UARTA_MASK);
	setbits_le32(clkenable(CLK_UARTA_REG), CLK_UARTA_MASK);
	clock_ll_set_source_divisor(&clk_rst->src_uarta, 0, 2);
	udelay(2);
	clrbits_le32(clkreset(CLK_UARTA_REG), CLK_UARTA_MASK);
}

/**
 * The T124 requires some special clock initialization, including setting up
 * the DVC I2C, turning on MSELECT and selecting the G CPU cluster
 */
void clock_init(void)
{
	u32 val;

	/*
	 * On poweron, AVP clock source (also called system clock) is set to
	 * PLLP_out0 with frequency set at 1MHz. Before initializing PLLP, we
	 * need to move the system clock's source to CLK_M temporarily. And
	 * then switch it to PLLP_out4 (204MHz) at a later time.
	 */
	val = (SCLK_SOURCE_CLKM << SCLK_SWAKEUP_FIQ_SOURCE_SHIFT) |
		(SCLK_SOURCE_CLKM << SCLK_SWAKEUP_IRQ_SOURCE_SHIFT) |
		(SCLK_SOURCE_CLKM << SCLK_SWAKEUP_RUN_SOURCE_SHIFT) |
		(SCLK_SOURCE_CLKM << SCLK_SWAKEUP_IDLE_SOURCE_SHIFT) |
		(SCLK_SYS_STATE_RUN << SCLK_SYS_STATE_SHIFT);
	writel(val, &clk_rst->crc_sclk_brst_pol);
	udelay(2);

	/* Set active CPU cluster to G */
	clrbits_le32(&flow->cluster_control, 1);

	/* Change the oscillator drive strength */
	val = readl(&clk_rst->crc_osc_ctrl);
	val &= ~OSC_XOFS_MASK;
	val |= (OSC_DRIVE_STRENGTH << OSC_XOFS_SHIFT);
	writel(val, &clk_rst->crc_osc_ctrl);

	/* Ambiguous quote from u-boot. TODO: what's this mean?
	 * "should update same value in PMC_OSC_EDPD_OVER XOFS
	   field for warmboot "*/
	val = readl(&pmc->pmc_osc_edpd_over);
	val &= ~PMC_XOFS_MASK;
	val |= (OSC_DRIVE_STRENGTH << PMC_XOFS_SHIFT);
	writel(val, &pmc->pmc_osc_edpd_over);

	/* Set HOLD_CKE_LOW_EN to 1 */
	setbits_le32(&pmc->pmc_cntrl2, HOLD_CKE_LOW_EN);

	init_pllx();

	val = (1 << CLK_SYS_RATE_AHB_RATE_SHIFT);
	writel(val, &clk_rst->crc_clk_sys_rate);
}

void clock_config(void)
{
	/* fixme. The stupidity of all this ... we are reading and
	 * writing the same register lots of times when we could just
	 * one lousy write with a combined mask. Sigh.
	 */
	setbits_le32(clkenable(CLK_CACHE2_REG), CLK_CACHE2_MASK);
	setbits_le32(clkenable(CLK_GPIO_REG), CLK_GPIO_MASK);
	setbits_le32(clkenable(CLK_TMR_REG), CLK_TMR_MASK);
	setbits_le32(clkenable(CLK_CPU_REG), CLK_CPU_MASK);
	setbits_le32(clkenable(CLK_EMC_REG), CLK_EMC_MASK);
	setbits_le32(clkenable(CLK_I2C1_REG), CLK_I2C1_MASK);
	setbits_le32(clkenable(CLK_I2C2_REG), CLK_I2C2_MASK);
	setbits_le32(clkenable(CLK_I2C3_REG), CLK_I2C3_MASK);
	setbits_le32(clkenable(CLK_I2C5_REG), CLK_I2C5_MASK);
	setbits_le32(clkenable(CLK_APBDMA_REG), CLK_APBDMA_MASK);
	setbits_le32(clkenable(CLK_MEM_REG), CLK_MEM_MASK);
	setbits_le32(clkenable(CLK_CSITE_REG), CLK_CSITE_MASK);
	setbits_le32(clkenablevw(CLK_VW_MSELECT_REG), CLK_VW_MSELECT_MASK);
	setbits_le32(clkenablevw(CLK_VW_DVFS_REG), CLK_VW_DVFS_MASK);

	/*
	 * Set MSELECT clock source as PLLP (00)_REG, and ask for a clock
	 * divider that would set the MSELECT clock at 102MHz for a
	 * PLLP base of 408MHz.
	 */
	clock_ll_set_source_divisor((void *)CLK_VW_MSELECT_REG, 0,
		CLK_DIVIDER(NVBL_PLLP_KHZ, 102000));

	/* Give clock time to stabilize */
	udelay(IO_STABILIZATION_DELAY);

	/* I2C1 gets CLK_M and a divisor of 17 */
	clock_ll_set_source_divisor(&clk_rst->src_i2c1, 3, 16);
	/* I2C2 gets CLK_M and a divisor of 17 */
	clock_ll_set_source_divisor(&clk_rst->src_i2c2, 3, 16);
	/* I2C3 (cam) gets CLK_M and a divisor of 17 */
	clock_ll_set_source_divisor(&clk_rst->src_i2c3, 3, 16);
	/* I2C5 (PMU) gets CLK_M and a divisor of 17 */
	clock_ll_set_source_divisor(&clk_rst->src_i2c5, 3, 16);

	/* Give clock time to stabilize */
	udelay(IO_STABILIZATION_DELAY);

	/* Take required peripherals out of reset */

	clrbits_le32(clkreset(CLK_CACHE2_REG), CLK_CACHE2_MASK);
	clrbits_le32(clkreset(CLK_GPIO_REG), CLK_GPIO_MASK);
	clrbits_le32(clkreset(CLK_TMR_REG), CLK_TMR_MASK);
	clrbits_le32(clkreset(CLK_CPU_REG), CLK_CPU_MASK);
	clrbits_le32(clkreset(CLK_EMC_REG), CLK_EMC_MASK);
	clrbits_le32(clkreset(CLK_I2C5_REG), CLK_I2C5_MASK);
	clrbits_le32(clkreset(CLK_I2C3_REG), CLK_I2C3_MASK);
	clrbits_le32(clkreset(CLK_I2C2_REG), CLK_I2C2_MASK);
	clrbits_le32(clkreset(CLK_I2C1_REG), CLK_I2C1_MASK);
	clrbits_le32(clkreset(CLK_APBDMA_REG), CLK_APBDMA_MASK);
	clrbits_le32(clkreset(CLK_MEM_REG), CLK_MEM_MASK);
	clrbits_le32(clkreset(CLK_CSITE_REG), CLK_CSITE_MASK);
	clrbits_le32(clkresetvw(CLK_VW_MSELECT_REG), CLK_VW_MSELECT_MASK);
	clrbits_le32(clkresetvw(CLK_VW_DVFS_REG), CLK_VW_DVFS_MASK);

}
