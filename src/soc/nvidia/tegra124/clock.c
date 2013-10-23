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
#include "cpug.h"
#include "flow.h"
#include "pmc.h"

static struct clk_rst_ctlr *clk_rst = (void *)TEGRA_CLK_RST_BASE;
static struct flow_ctlr *flow = (void *)TEGRA_FLOW_BASE;
static struct tegra_pmc_regs *pmc = (void*)TEGRA_PMC_BASE;

/* only needed in this one place. Avoid namespace pollution. Be green .*/
struct clk_pll_table {
	u16	n;
	u16	m;
	u8	p;
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
	[OSC_FREQ_OSC13]{146,1,0},
	[OSC_FREQ_OSC19P2]{98,1,0},
	[OSC_FREQ_OSC12]{157,1,0},
	[OSC_FREQ_OSC26]{73,1,0},
	[OSC_FREQ_OSC16P8]{113,1,0},
	[OSC_FREQ_OSC38P4]{98,2,0},
	[OSC_FREQ_OSC48]{157,4,0},
};

void clock_ll_set_source_divisor(u32 *reg, u32 source, u32 divisor)
{
        u32 value;

        value = readl(reg);

        value &= ~CLK_SOURCE_MASK;
        value |= source << CLK_SOURCE_SHIFT;

        value &= ~CLK_DIVISOR_MASK;
        value |= divisor << CLK_DIVISOR_SHIFT;

        writel(value, reg);
}

/* Get the oscillator frequency, from the corresponding hardware
 * configuration field. This is actually a per-soc thing. Avoid the
 * temptation to make it common.
 */
static int clock_get_osc_freq(void)
{
	u32 reg;
        reg = readl(&clk_rst->osc_ctrl);
        reg >>= OSC_CTRL_OSC_FREQ_SHIFT;
	return reg;
}

int clock_get_osc_khz(void)
{
	/* Implemented better in the next patch, sorry, hard to split this up */
	switch (clock_get_osc_freq()) {
	case OSC_FREQ_OSC13:
		return 13000;
	case OSC_FREQ_OSC19P2:
		return 19200;
	default:
	case OSC_FREQ_OSC12:
		return 12000;
	case OSC_FREQ_OSC26:
		return 26000;
	case OSC_FREQ_OSC16P8:
		return 16800;
	case OSC_FREQ_OSC38P4:
		return 38400;
	case OSC_FREQ_OSC48:
		return 48000;
	}
}

static void adjust_pllp_out_freqs(void)
{
	u32 reg;
	/* Set T30 PLLP_OUT1, 2, 3 & 4 freqs to 9.6, 48, 102 & 204MHz */
	reg = readl(&clk_rst->pllp_outa); // OUTA contains OUT2 / OUT1
	reg |= (IN_408_OUT_48_DIVISOR << PLLP_OUT2_RATIO) | PLLP_OUT2_OVR
		| (IN_408_OUT_9_6_DIVISOR << PLLP_OUT1_RATIO) | PLLP_OUT1_OVR;
	writel(reg, &clk_rst->pllp_outa);

	reg = readl(&clk_rst->pllp_outb);   // OUTB, contains OUT4 / OUT3
	reg |= (IN_408_OUT_204_DIVISOR << PLLP_OUT4_RATIO) | PLLP_OUT4_OVR
		| (IN_408_OUT_102_DIVISOR << PLLP_OUT3_RATIO) | PLLP_OUT3_OVR;
	writel(reg, &clk_rst->pllp_outb);
}

static int pllx_set_rate(u32 divn, u32 divm, u32 divp)
{
	u32 reg;

	/* If PLLX is already enabled, just return */
	if (readl(&clk_rst->pllx_base) & PLL_ENABLE_MASK)
		return 0;

	/* Disable IDDQ */
	reg = readl(&clk_rst->pllx_misc3);
	reg &= ~PLLX_IDDQ_MASK;
	writel(reg, &clk_rst->pllx_misc3);
	udelay(2);

	/* Set BYPASS, m, n and p to PLLX_BASE */
	reg = PLL_BYPASS_MASK | (divm << PLL_DIVM_SHIFT);
	reg |= ((divn << PLL_DIVN_SHIFT) | (divp << PLL_DIVP_SHIFT));
	writel(reg, &clk_rst->pllx_base);

	/* Disable BYPASS */
	reg = readl(&clk_rst->pllx_base);
	reg &= ~PLL_BYPASS_MASK;
	writel(reg, &clk_rst->pllx_base);

	/* Set lock_enable to PLLX_MISC */
	reg = readl(&clk_rst->pllx_misc);
	reg |= PLL_LOCK_ENABLE_MASK;
	writel(reg, &clk_rst->pllx_misc);

	/* Enable PLLX last, as per JZ */
	reg = readl(&clk_rst->pllx_base);
	reg |= PLL_ENABLE_MASK;
	writel(reg, &clk_rst->pllx_base);

	return 0;
}

static void init_pllx(void)
{
	int osc = clock_get_osc_freq();
	struct clk_pll_table *sel = &tegra_pll_x_table[osc];

	if (sel->n == 0)
		return;

	pllx_set_rate(sel->n, sel->m, sel->p);

	adjust_pllp_out_freqs();
}

/* Initialize the UART and put it on CLK_M so we can use it during clock_init().
 * Will later move it to PLLP in clock_config(). The divisor must be very small
 * to accomodate 12KHz OSCs, so we override the 16.0 UART divider with the 15.1
 * CLK_SOURCE divider to get more precision. (This might still not be enough for
 * some OSCs... if you use 13KHz, be prepared to have a bad time.) The 1800 has
 * been determined through trial and error (must lead to div 13 at 24MHz). */
void clock_early_uart(void)
{
	clock_ll_set_source_divisor(&clk_rst->clk_src_uarta, 3,
		CLK_UART_DIV_OVERRIDE | CLK_DIVIDER(clock_get_osc_khz(), 1800));
	setbits_le32(&clk_rst->clk_out_enb_l, CLK_L_UARTA);
	udelay(2);
	clrbits_le32(&clk_rst->rst_dev_l, CLK_L_UARTA);
}

void clock_cpu0_config_and_reset(void *entry)
{
	void * const evp_cpu_reset = (uint8_t *)TEGRA_EVP_BASE + 0x100;

	write32(CONFIG_STACK_TOP, &cpug_stack_pointer);
	write32((uintptr_t)entry, &cpug_entry_point);
	write32((uintptr_t)&cpug_setup, evp_cpu_reset);

	// Wait for PLLX to lock.
	while (!(readl(&clk_rst->pllx_base) & (0x1 << 27)))
		;

	// Set up cclk_brst and divider.
	write32((CRC_CCLK_BRST_POL_PLLX_OUT0 << 0) |
		(CRC_CCLK_BRST_POL_PLLX_OUT0 << 4) |
		(CRC_CCLK_BRST_POL_PLLX_OUT0 << 8) |
		(CRC_CCLK_BRST_POL_PLLX_OUT0 << 12) |
		(CRC_CCLK_BRST_POL_CPU_STATE_RUN << 28),
		&clk_rst->cclk_brst_pol);
	write32(CRC_SUPER_CCLK_DIVIDER_SUPER_CDIV_ENB,
		&clk_rst->super_cclk_div);

	// Enable the clocks for CPUs 0-3.
	uint32_t cpu_cmplx_clr = read32(&clk_rst->clk_cpu_cmplx_clr);
	cpu_cmplx_clr |= CRC_CLK_CLR_CPU0_STP | CRC_CLK_CLR_CPU1_STP |
			 CRC_CLK_CLR_CPU2_STP | CRC_CLK_CLR_CPU3_STP;
	write32(cpu_cmplx_clr, &clk_rst->clk_cpu_cmplx_clr);

	// Enable other CPU related clocks.
	setbits_le32(&clk_rst->clk_out_enb_l, CLK_L_CPU);
	setbits_le32(&clk_rst->clk_out_enb_v, CLK_V_CPUG);

	// Disable the reset on the non-CPU parts of the fast cluster.
	write32(CRC_RST_CPUG_CLR_NONCPU,
		&clk_rst->rst_cpug_cmplx_clr);
	// Disable the various resets on the CPUs.
	write32(CRC_RST_CPUG_CLR_CPU0 | CRC_RST_CPUG_CLR_CPU1 |
		CRC_RST_CPUG_CLR_CPU2 | CRC_RST_CPUG_CLR_CPU3 |
		CRC_RST_CPUG_CLR_DBG0 | CRC_RST_CPUG_CLR_DBG1 |
		CRC_RST_CPUG_CLR_DBG2 | CRC_RST_CPUG_CLR_DBG3 |
		CRC_RST_CPUG_CLR_CORE0 | CRC_RST_CPUG_CLR_CORE1 |
		CRC_RST_CPUG_CLR_CORE2 | CRC_RST_CPUG_CLR_CORE3 |
		CRC_RST_CPUG_CLR_CX0 | CRC_RST_CPUG_CLR_CX1 |
		CRC_RST_CPUG_CLR_CX2 | CRC_RST_CPUG_CLR_CX3 |
		CRC_RST_CPUG_CLR_L2 | CRC_RST_CPUG_CLR_PDBG,
		&clk_rst->rst_cpug_cmplx_clr);
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
	writel(val, &clk_rst->sclk_brst_pol);
	udelay(2);

	/* Set active CPU cluster to G */
	clrbits_le32(&flow->cluster_control, 1);

	/* Change the oscillator drive strength */
	val = readl(&clk_rst->osc_ctrl);
	val &= ~OSC_XOFS_MASK;
	val |= (OSC_DRIVE_STRENGTH << OSC_XOFS_SHIFT);
	writel(val, &clk_rst->osc_ctrl);

	/* Ambiguous quote from u-boot. TODO: what's this mean?
	 * "should update same value in PMC_OSC_EDPD_OVER XOFS
	   field for warmboot "*/
	val = readl(&pmc->osc_edpd_over);
	val &= ~PMC_OSC_EDPD_OVER_XOFS_MASK;
	val |= (OSC_DRIVE_STRENGTH << PMC_OSC_EDPD_OVER_XOFS_SHIFT);
	writel(val, &pmc->osc_edpd_over);

	init_pllx();

	val = (1 << CLK_SYS_RATE_AHB_RATE_SHIFT);
	writel(val, &clk_rst->clk_sys_rate);
}

void clock_config(void)
{
	/* Enable clocks for the required peripherals. */
	setbits_le32(&clk_rst->clk_out_enb_l,
		     CLK_L_CACHE2 | CLK_L_GPIO | CLK_L_TMR | CLK_L_I2C1 |
		     CLK_L_SDMMC4);
	setbits_le32(&clk_rst->clk_out_enb_h,
		     CLK_H_EMC | CLK_H_I2C2 | CLK_H_I2C5 | CLK_H_SBC1 |
		     CLK_H_PMC | CLK_H_APBDMA | CLK_H_MEM);
	setbits_le32(&clk_rst->clk_out_enb_u,
		     CLK_U_I2C3 | CLK_U_CSITE | CLK_U_SDMMC3);
	setbits_le32(&clk_rst->clk_out_enb_v, CLK_V_MSELECT);
	setbits_le32(&clk_rst->clk_out_enb_w, CLK_W_DVFS);

	/*
	 * Set MSELECT clock source as PLLP (00)_REG, and ask for a clock
	 * divider that would set the MSELECT clock at 102MHz for a
	 * PLLP base of 408MHz.
	 */
	clock_ll_set_source_divisor(&clk_rst->clk_src_mselect, 0,
		CLK_DIVIDER(NVBL_PLLP_KHZ, 102000));

	/* Give clock time to stabilize */
	udelay(IO_STABILIZATION_DELAY);

	/* I2C1 gets CLK_M and a divisor of 17 */
	clock_ll_set_source_divisor(&clk_rst->clk_src_i2c1, 3, 16);
	/* I2C2 gets CLK_M and a divisor of 17 */
	clock_ll_set_source_divisor(&clk_rst->clk_src_i2c2, 3, 16);
	/* I2C3 (cam) gets CLK_M and a divisor of 17 */
	clock_ll_set_source_divisor(&clk_rst->clk_src_i2c3, 3, 16);
	/* I2C5 (PMU) gets CLK_M and a divisor of 17 */
	clock_ll_set_source_divisor(&clk_rst->clk_src_i2c5, 3, 16);

	/* UARTA gets PLLP, deactivate CLK_UART_DIV_OVERRIDE */
	writel(0 << CLK_SOURCE_SHIFT, &clk_rst->clk_src_uarta);

	/* Give clock time to stabilize. */
	udelay(IO_STABILIZATION_DELAY);

	/* Take required peripherals out of reset. */

	clrbits_le32(&clk_rst->rst_dev_l,
		     CLK_L_CACHE2 | CLK_L_GPIO | CLK_L_TMR | CLK_L_I2C1 |
		     CLK_L_SDMMC4);
	clrbits_le32(&clk_rst->rst_dev_h,
		     CLK_H_EMC | CLK_H_I2C2 | CLK_H_I2C5 | CLK_H_SBC1 |
		     CLK_H_PMC | CLK_H_APBDMA | CLK_H_MEM);
	clrbits_le32(&clk_rst->rst_dev_u,
		     CLK_U_I2C3 | CLK_U_CSITE | CLK_U_SDMMC3);
	clrbits_le32(&clk_rst->rst_dev_v, CLK_V_MSELECT);
	clrbits_le32(&clk_rst->rst_dev_w, CLK_W_DVFS);
}
