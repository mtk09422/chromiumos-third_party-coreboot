/*
 * Copyright (c) 2013-2015, NVIDIA CORPORATION.  All rights reserved.
 * Copyright 2014 Google Inc.
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
#include <arch/clock.h>
#include <arch/io.h>
#include <console/console.h>
#include <delay.h>
#include <stdlib.h>
#include <soc/addressmap.h>
#include <soc/clk_rst.h>
#include <soc/clock.h>
#include <soc/clst_clk.h>
#include <soc/flow.h>
#include <soc/maincpu.h>
#include <soc/pmc.h>
#include <soc/sysctr.h>

static struct flow_ctlr *flow = (void *)TEGRA_FLOW_BASE;
static struct tegra_pmc_regs *pmc = (void *)TEGRA_PMC_BASE;
static struct sysctr_regs *sysctr = (void *)TEGRA_SYSCTR0_BASE;

struct pll_fields {
	u64	n:8;		/* the feedback divider bits width */
	u64	n_shift:5;	/* n bits location */
	u64	m:8;		/* the input divider bits width */
	u64	m_shift:5;	/* m bits location */
	u64	p:5;		/* the post divider bits witch */
	u64	p_shift:5;	/* p bits location */
	u64	kcp:2;		/* charge pump gain control */
	u64	kvco:1;	/* vco gain */
	u64	kcp_shift:5;	/* kcp bits starting bit in misc reg */
	u64	kvco_shift:5;	/* kvco bit in misc reg */
	u64	rsvd:15;
};

/* This table defines the frequency dividers for every PLL to turn the external
 * OSC clock into the frequencies defined by TEGRA_PLL*_KHZ in soc/clock.h.
 * All PLLs have three dividers (n, m and p), with the governing formula for
 * the output frequency being CF = (IN / m), VCO = CF * n and OUT = VCO / (2^p).
 * All divisor configurations must meet the PLL's constraints for VCO and CF:
 * PLLX:  12 MHz < CF < 50 MHz, 700 MHz < VCO < 3000 MHz
 * PLLC:  12 MHz < CF < 50 MHz, 600 MHz < VCO < 1400 MHz
 * PLLM:  12 MHz < CF < 50 MHz, 400 MHz < VCO < 1066 MHz
 * PLLP:   1 MHz < CF <  6 MHz, 200 MHz < VCO <  700 MHz
 * PLLD:   1 MHz < CF <  6 MHz, 500 MHz < VCO < 1000 MHz
 * PLLU:   1 MHz < CF <  6 MHz, 480 MHz < VCO <  960 MHz
 * PLLDP: 12 MHz < CF < 38 MHz, 600 MHz < VCO < 1200 MHz
 * (values taken from Linux' drivers/clk/tegra/clk-tegra124.c). */
struct {
	int khz;
	struct pll_fields	pllx;	/* target:  CONFIG_PLLX_KHZ */
	struct pll_fields	pllc;	/* target:  600 MHz */
	/* PLLM is set up dynamically by clock_sdram(). */
	/* PLLP is hardwired to 408 MHz in HW (unless we set BASE_OVRD). */
	struct pll_fields	pllu;	/* target;  960 MHz */
	struct pll_fields	plldp;	/* target;  270 MHz */
	/* PLLDP treats p differently (OUT = VCO / (p + 1) for p < 6). */
} static const osc_table[16] = {
	[OSC_FREQ_12]{
		.khz = 12000,
		.pllx = {.n = TEGRA_PLLX_KHZ / 12000, .n_shift = 8, .m =  1,
			 .m_shift = 0, .p = 0, .p_shift = 20},
		.pllc = {.n =  50, .n_shift = 10, .m =  1, .m_shift = 0,
			 .p = 0, .p_shift = 20},
		.pllu = {.n = 240, .n_shift = 8, .m = 3, .m_shift = 0,
			 .p = 0, .p_shift = 16},
		.plldp = {.n = 90, .n_shift = 8, .m =  1, .m_shift = 0,
			 .p = 3, .p_shift = 19},
	},
	[OSC_FREQ_13]{
		.khz = 13000,
		.pllx = {.n = TEGRA_PLLX_KHZ / 13000, .n_shift = 8, .m =  1,
			 .m_shift = 0, .p = 0, .p_shift = 20},
		.pllc = {.n =  46, .n_shift = 10, .m =  1, .m_shift = 0,
			 .p = 0, .p_shift = 20},		 /* 598.0 MHz */
		.pllu = {.n = 148, .n_shift = 8, .m = 2, .m_shift = 0,
			 .p = 0, .p_shift = 16},		 /* 962.0 MHz */
		.plldp = {.n = 83, .n_shift = 8, .m =  1, .m_shift = 0,
			  .p = 3, .p_shift = 19},		 /* 269.8 MHz */
	},
	[OSC_FREQ_16P8]{
		.khz = 16800,
		.pllx = {.n = TEGRA_PLLX_KHZ / 16800, .n_shift = 8, .m =  1,
			 .m_shift = 0, .p = 0, .p_shift = 20},
		.pllc = {.n =  71, .n_shift = 10, .m =  1, .m_shift = 0,
			 .p = 1, .p_shift = 20},		 /* 596.4 MHz */
		.pllu = {.n = 229, .n_shift = 8, .m =  4, .m_shift = 0,
			 .p = 0, .p_shift = 16},		 /* 961.8 MHz */
		.plldp = {.n = 64, .n_shift = 8, .m =  1, .m_shift = 0,
			  .p = 3, .p_shift = 19},		 /* 268.8 MHz */
	},
	[OSC_FREQ_19P2]{
		.khz = 19200,
		.pllx = {.n = TEGRA_PLLX_KHZ / 19200, .n_shift = 8, .m =  1,
			 .m_shift = 0, .p = 0, .p_shift = 20},
		.pllc = {.n =  62, .n_shift = 10, .m =  1, .m_shift = 0,
			 .p = 1, .p_shift = 20},		 /* 595.2 MHz */
		.pllu = {.n = 200, .n_shift = 8, .m =  4, .m_shift = 0,
			 .p = 0, .p_shift = 16},
		.plldp = {.n = 56, .n_shift = 8, .m =  1, .m_shift = 0,
			  .p = 3, .p_shift = 19},		 /* 268.8 MHz */
	},
	[OSC_FREQ_26]{
		.khz = 26000,
		.pllx = {.n = TEGRA_PLLX_KHZ / 26000, .n_shift = 8, .m =  1,
			 .m_shift = 0, .p = 0, .p_shift = 20},
		.pllc = {.n =  23, .n_shift = 10, .m =  1, .m_shift = 0,
			 .p = 0, .p_shift = 20},		 /* 598.0 MHz */
		.pllu = {.n = 148, .n_shift = 8, .m = 4, .m_shift = 0,
			 .p = 0, .p_shift = 16},		 /* 962.0 MHz */
		.plldp = {.n = 83, .n_shift = 8, .m =  2, .m_shift = 0,
			 .p = 3, .p_shift = 19},		 /* 269.8 MHz */
	},
	[OSC_FREQ_38P4]{
		.khz = 38400,
		.pllx = {.n = TEGRA_PLLX_KHZ / 38400, .n_shift = 8, .m =  1,
			 .m_shift = 0, .p = 0, .p_shift = 20},
		.pllc = {.n =  62, .n_shift = 10, .m =  2, .m_shift = 0,
			 .p = 1, .p_shift = 20},		/* 595.2 MHz */
		.pllu = {.n = 200, .n_shift = 8, .m = 8, .m_shift = 0,
			 .p = 0, .p_shift = 16},
		.plldp = {.n = 56, .n_shift = 8, .m =  2, .m_shift = 0,
			 .p = 3, .p_shift = 19},		/* 268.8 MHz */
	},
	[OSC_FREQ_48]{
		.khz = 48000,
		.pllx = {.n = TEGRA_PLLX_KHZ / 48000, .n_shift = 8, .m =  1,
			 .m_shift = 0, .p = 0, .p_shift = 20},
		.pllc = {.n =  50, .n_shift = 10, .m =  1, .m_shift = 0,
			 .p = 0, .p_shift = 20},
		.pllu = {.n = 240, .n_shift = 8, .m = 12, .m_shift = 0,
			 .p = 0, .p_shift = 16},
		.plldp = {.n = 90, .n_shift = 8, .m =  1, .m_shift = 0,
			 .p = 3, .p_shift = 19},
	},
};

/* Get the oscillator frequency, from the corresponding hardware
 * configuration field. This is actually a per-soc thing. Avoid the
 * temptation to make it common.
 */
static u32 clock_get_osc_bits(void)
{
	return (read32(CLK_RST_REG(osc_ctrl)) & OSC_FREQ_MASK) >> OSC_FREQ_SHIFT;
}

int clock_get_osc_khz(void)
{
	return osc_table[clock_get_osc_bits()].khz;
}

int clock_get_pll_input_khz(void)
{
	u32 osc_ctrl = read32(CLK_RST_REG(osc_ctrl));
	u32 osc_bits = (osc_ctrl & OSC_FREQ_MASK) >> OSC_FREQ_SHIFT;
	u32 pll_ref_div = (osc_ctrl & OSC_PREDIV_MASK) >> OSC_PREDIV_SHIFT;
	return osc_table[osc_bits].khz >> pll_ref_div;
}

void clock_init_arm_generic_timer(void)
{
	uint32_t freq = clock_get_osc_khz() * 1000;
	// Set the cntfrq register.
	set_cntfrq(freq);

	// Record the system timer frequency.
	write32(&sysctr->cntfid0, freq);
	// Enable the system counter.
	uint32_t cntcr = read32(&sysctr->cntcr);
	cntcr |= SYSCTR_CNTCR_EN | SYSCTR_CNTCR_HDBG;
	write32(&sysctr->cntcr, cntcr);
}

#define SOR0_CLK_SEL0			(1 << 14)
#define SOR0_CLK_SEL1			(1 << 15)

void sor_clock_stop(void)
{
	/* The Serial Output Resource clock has to be off
	 * before we start the plldp. Learned the hard way.
	 * FIXME: this has to be cleaned up a bit more.
	 * Waiting on some new info from Nvidia.
	 */
	clrbits_le32(CLK_RST_REG(clk_src_sor), SOR0_CLK_SEL0 | SOR0_CLK_SEL1);
}

void sor_clock_start(void)
{
	/* uses PLLP, has a non-standard bit layout. */
	setbits_le32(CLK_RST_REG(clk_src_sor), SOR0_CLK_SEL0);
}

static void init_pll(u32 *base, u32 *misc, struct pll_fields pll, u32 lock)
{
	u32 dividers =  pll.n << pll.n_shift |
			pll.m << pll.m_shift |
			pll.p << pll.p_shift;

	/* Write dividers but BYPASS the PLL while we're messing with it. */
	write32(base, dividers | PLL_BASE_BYPASS);

	/*
	 * Set Lock bit if needed
	 */
	if (lock)
		write32(misc, lock);

	/* Enable PLL and take it back out of BYPASS */
	write32(base, dividers | PLL_BASE_ENABLE);

	/* Wait for lock ready */
	if (lock)
		while (!(read32(base) & PLL_BASE_LOCK))
			;
}

static void init_utmip_pll(void)
{
	int khz = clock_get_pll_input_khz();

	/* Shut off PLL crystal clock while we mess with it */
	clrbits_le32(CLK_RST_REG(utmip_pll_cfg2), 1 << 30); /* PHY_XTAL_CLKEN */
	udelay(1);

	write32(CLK_RST_REG(utmip_pll_cfg0),	/* 960MHz * 1 / 80 == 12 MHz */
		80 << 16 |			/* (rst) phy_divn */
		 1 <<  8);			/* (rst) phy_divm */

	write32(CLK_RST_REG(utmip_pll_cfg1),
		div_round_up(khz, 8000) << 27 |	/* pllu_enbl_cnt / 8 (1us) */
				      0 << 16 | /* PLLU pwrdn */
				      0 << 14 | /* pll_enable pwrdn */
				      0 << 12 | /* pll_active pwrdn */
		 div_round_up(khz, 102) << 0);  /* phy_stbl_cnt / 256 (2.5ms) */

	/* TODO: TRM can't decide if actv is 5us or 10us, keep an eye on it */
	write32(CLK_RST_REG(utmip_pll_cfg2),
				      0 << 24 |	/* SAMP_D/XDEV pwrdn */
		div_round_up(khz, 3200) << 18 |	/* phy_actv_cnt / 16 (5us) */
		 div_round_up(khz, 256) <<  6 |	/* pllu_stbl_cnt / 256 (1ms) */
				      0 <<  4 |	/* SAMP_C/USB3 pwrdn */
				      0 <<  2 |	/* SAMP_B/XHOST pwrdn */
				      0 <<  0);	/* SAMP_A/USBD pwrdn */

	setbits_le32(CLK_RST_REG(utmip_pll_cfg2), 1 << 30); /* PHY_XTAL_CLKEN */
}

/* Graphics just has to be different. There's a few more bits we
 * need to set in here, but it makes sense just to restrict all the
 * special bits to this one function.
 */
static void graphics_pll(void)
{
	int osc = clock_get_osc_bits();
	u32 *cfg = CLK_RST_REG(plldp_ss_cfg);
	/* the vendor code sets the dither bit (28)
	 * an undocumented bit (24)
	 * and clamp while we mess with it (22)
	 * Dither is pretty important to display port
	 * so we really do need to handle these bits.
	 * I'm not willing to not clamp it, even if
	 * it might "mostly work" with it not set,
	 * I don't want to find out in a few months
	 * that it is needed.
	 */
	u32 scfg = (1<<28) | (1<<24) | (1<<22);
	write32(cfg, scfg);
	init_pll(CLK_RST_REG(plldp_base), CLK_RST_REG(plldp_misc),
		osc_table[osc].plldp, PLLDPD2_MISC_LOCK_ENABLE);
	/* leave dither and undoc bits set, release clamp */
	scfg = (1<<28) | (1<<24);
	write32(cfg, scfg);
}

/*
 * Init PLLD clock source.
 *
 * @frequency: the requested plld frequency
 *
 * Return the plld frequency if success, otherwise return 0.
 */
u32 clock_configure_plld(u32 frequency)
{
	/**
	 * plld (fo) = vco >> p, where 500MHz < vco < 1000MHz
	 *           = (cf * n) >> p, where 1MHz < cf < 6MHz
	 *           = ((ref / m) * n) >> p
	 *
	 * Iterate the possible values of p (3 bits, 2^7) to find out a minimum
	 * safe vco, then find best (m, n). since m has only 5 bits, we can
	 * iterate all possible values.  Note Tegra1xx supports 11 bits for n,
	 * but our pll_fields has only 10 bits for n.
	 *
	 * Note values undershoot or overshoot target output frequency may not
	 * work if the values are not in "safe" range by panel specification.
	 */
	struct pll_fields plld = { 0 };
	u32 ref = clock_get_pll_input_khz() * 1000, m, n, p = 0;
	u32 cf, vco, rounded_rate = frequency;
	u32 diff, best_diff;
	const u32 max_m = 1 << 5, max_n = 1 << 10, max_p = 1 << 3,
		  mhz = 1000 * 1000, min_vco = 500 * mhz, max_vco = 1000 * mhz,
		  min_cf = 1 * mhz, max_cf = 6 * mhz;

	for (vco = frequency; vco < min_vco && p < max_p; p++)
		vco <<= 1;

	if (vco < min_vco || vco > max_vco) {
		printk(BIOS_ERR, "%s: Cannot find out a supported VCO"
			" for Frequency (%u).\n", __func__, frequency);
		return 0;
	}

	plld.p = p;
	best_diff = vco;

	for (m = 1; m < max_m && best_diff; m++) {
		cf = ref / m;
		if (cf < min_cf)
			break;
		if (cf > max_cf)
			continue;

		n = vco / cf;
		if (n >= max_n)
			continue;

		diff = vco - n * cf;
		if (n + 1 < max_n && diff > cf / 2) {
			n++;
			diff = cf - diff;
		}

		if (diff >= best_diff)
			continue;

		best_diff = diff;
		plld.m = m;
		plld.n = n;
	}

	if (best_diff) {
		printk(BIOS_WARNING, "%s: Failed to match output frequency %u, "
		       "best difference is %u.\n", __func__, frequency,
		       best_diff);
		rounded_rate = (ref / plld.m * plld.n) >> plld.p;
	}

	printk(BIOS_DEBUG, "%s: PLLD=%u ref=%u, m/n/p=%u/%u/%u\n",
	       __func__, rounded_rate, ref, plld.m, plld.n, plld.p);

	plld.n_shift = PLLD_N_SHIFT;
	plld.m_shift = PLLD_M_SHIFT;
	plld.p_shift = PLLD_P_SHIFT;
	init_pll(CLK_RST_REG(plld_base), CLK_RST_REG(plld_misc), plld,
		 (PLLD_MISC_LOCK_ENABLE | PLLD_MISC_CLK_ENABLE));

	if (rounded_rate != frequency)
		printk(BIOS_DEBUG, "PLLD rate: %u vs %u\n", rounded_rate,
			frequency);

	return rounded_rate;
}

/*
 * Initialize the UART and use PLLP as clock source. PLLP is hardwired to 408
 * MHz in HW (unless we set BASE_OVRD). We override the 16.0 UART divider with
 * the 15.1 CLK_SOURCE divider to get more precision. The 1843(KHZ) is
 * calculated thru BAUD_RATE*16/1000, ie, 115200*16/1000.
 */
void clock_early_uart(void)
{
	write32(CLK_RST_REG(clk_src_uarta),
		CLK_SRC_DEV_ID(UARTA, PLLP) << CLK_SOURCE_SHIFT |
		CLK_UART_DIV_OVERRIDE |
		CLK_DIVIDER(TEGRA_PLLP_KHZ, 1843));

	clock_enable_clear_reset_l(CLK_L_UARTA);
}

/* Enable output clock (CLK1~3) for external peripherals. */
void clock_external_output(int clk_id)
{
	switch (clk_id) {
	case 1:
		setbits_le32(&pmc->clk_out_cntrl, 1 << 2);
		break;
	case 2:
		setbits_le32(&pmc->clk_out_cntrl, 1 << 10);
		break;
	case 3:
		setbits_le32(&pmc->clk_out_cntrl, 1 << 18);
		break;
	default:
		printk(BIOS_CRIT, "ERROR: Unknown output clock id %d\n",
		       clk_id);
		break;
	}
}

/* Start PLLM for SDRAM. */
void clock_sdram(u32 m, u32 n, u32 p, u32 setup, u32 ph45, u32 ph90,
		 u32 ph135, u32 kvco, u32 kcp, u32 stable_time, u32 emc_source,
		 u32 same_freq)
{
	u32 misc1 = ((setup << PLLM_MISC1_SETUP_SHIFT) |
		     (ph45 << PLLM_MISC1_PD_LSHIFT_PH45_SHIFT) |
		     (ph90 << PLLM_MISC1_PD_LSHIFT_PH90_SHIFT) |
		     (ph135 << PLLM_MISC1_PD_LSHIFT_PH135_SHIFT)),
	    misc2 = ((kvco << PLLM_MISC2_KVCO_SHIFT) |
		     (kcp << PLLM_MISC2_KCP_SHIFT)),
	    base;

	if (same_freq)
		emc_source |= CLK_SOURCE_EMC_MC_EMC_SAME_FREQ;
	else
		emc_source &= ~CLK_SOURCE_EMC_MC_EMC_SAME_FREQ;

	/*
	 * Note PLLM_BASE.PLLM_OUT1_RSTN must be in RESET_ENABLE mode, and
	 * PLLM_BASE.ENABLE must be in DISABLE state (both are the default
	 * values after coldboot reset).
	 */

	write32(CLK_RST_REG(pllm_misc1), misc1);
	write32(CLK_RST_REG(pllm_misc2), misc2);

	/* PLLM.BASE needs BYPASS=0, different from general init_pll */
	base = read32(CLK_RST_REG(pllm_base));
	base &= ~(PLLCMX_BASE_DIVN_MASK | PLLCMX_BASE_DIVM_MASK |
		  PLLM_BASE_DIVP_MASK | PLL_BASE_BYPASS);
	base |= ((m << PLL_BASE_DIVM_SHIFT) | (n << PLL_BASE_DIVN_SHIFT) |
		 (p << PLL_BASE_DIVP_SHIFT));
	write32(CLK_RST_REG(pllm_base), base);

	setbits_le32(CLK_RST_REG(pllm_base), PLL_BASE_ENABLE);
	/* stable_time is required, before we can start to check lock. */
	udelay(stable_time);

	while (!(read32(CLK_RST_REG(pllm_base)) & PLL_BASE_LOCK))
		udelay(1);

	/*
	 * After PLLM reports being locked, we have to delay 10us before
	 * enabling PLLM_OUT.
	 */
	udelay(10);

	/* Put OUT1 out of reset state (start to output). */
	setbits_le32(CLK_RST_REG(pllm_out), PLLM_OUT1_RSTN_RESET_DISABLE);

	/* Enable and start MEM(MC) and EMC. */
	clock_enable_clear_reset(0, CLK_H_MEM | CLK_H_EMC, 0, 0, 0, 0, 0);
	write32(CLK_RST_REG(clk_src_emc), emc_source);
	udelay(IO_STABILIZATION_DELAY);
}

void clock_halt_avp(void)
{
	for (;;) {
		write32(&flow->halt_cop_events,
			FLOW_EVENT_JTAG | FLOW_EVENT_LIC_IRQ |
			FLOW_EVENT_GIC_IRQ | FLOW_MODE_WAITEVENT);
	}
}

void clock_init(void)
{
	u32 osc = clock_get_osc_bits();
	u32 reg;

	/* Clear PLLC reset */
	reg = read32(CLK_RST_REG(pllc_misc));
	reg &= ~PLLC_MISC_RESET;
	write32(CLK_RST_REG(pllc_misc), reg);

	/* Clear PLLC IDDQ */
	reg = read32(CLK_RST_REG(pllc_misc_1));
	reg &= ~PLLC_MISC_1_IDDQ;
	write32(CLK_RST_REG(pllc_misc_1), reg);

	/* Max out the AVP clock before everything else (need PLLC for that). */
	init_pll(CLK_RST_REG(pllc_base), CLK_RST_REG(pllc_misc),
		osc_table[osc].pllc, 0);

	/* wait for pllc_lock (not the normal bit 27) */
	while (!(read32(CLK_RST_REG(pllc_base)) & PLLC_BASE_LOCK))
		;

	/* Typical ratios are 1:2:2 or 1:2:3 sclk:hclk:pclk (See: APB DMA
	 * features section in the TRM). */
	write32(CLK_RST_REG(clk_sys_rate),	/* pclk = hclk = sclk/2 */
		1 << HCLK_DIVISOR_SHIFT | 0 << PCLK_DIVISOR_SHIFT);
	write32(CLK_RST_REG(pllc_out),
		CLK_DIVIDER(TEGRA_PLLC_KHZ, 300000) << PLL_OUT_RATIO_SHIFT |
		PLL_OUT_CLKEN | PLL_OUT_RSTN);
	write32(CLK_RST_REG(sclk_brst_pol),		/* sclk = 300 MHz */
		SCLK_SYS_STATE_RUN << SCLK_SYS_STATE_SHIFT |
		SCLK_SOURCE_PLLC_OUT1 << SCLK_RUN_SHIFT);

	/* Change the oscillator drive strength (from U-Boot -- why?) */
	clrsetbits_le32(CLK_RST_REG(osc_ctrl), OSC_XOFS_MASK,
			OSC_DRIVE_STRENGTH << OSC_XOFS_SHIFT);

	/*
	 * Ambiguous quote from u-boot. TODO: what's this mean?
	 * "should update same value in PMC_OSC_EDPD_OVER XOFS
	 * field for warmboot "
	 */
	clrsetbits_le32(&pmc->osc_edpd_over, PMC_OSC_EDPD_OVER_XOFS_MASK,
			OSC_DRIVE_STRENGTH << PMC_OSC_EDPD_OVER_XOFS_SHIFT);

	/* Disable IDDQ for PLLX before we set it up (from U-Boot -- why?) */
	clrbits_le32(CLK_RST_REG(pllx_misc3), PLLX_IDDQ_MASK);

	/* Set up PLLP_OUT(1|2|3|4) divisor to generate (9.6|48|102|204)MHz */
	write32(CLK_RST_REG(pllp_outa),
		(CLK_DIVIDER(TEGRA_PLLP_KHZ, 9600) << PLL_OUT_RATIO_SHIFT |
		PLL_OUT_OVR | PLL_OUT_CLKEN | PLL_OUT_RSTN) << PLL_OUT1_SHIFT |
		(CLK_DIVIDER(TEGRA_PLLP_KHZ, 48000) << PLL_OUT_RATIO_SHIFT |
		PLL_OUT_OVR | PLL_OUT_CLKEN | PLL_OUT_RSTN) << PLL_OUT2_SHIFT);
	write32(CLK_RST_REG(pllp_outb),
		(CLK_DIVIDER(TEGRA_PLLP_KHZ, 102000) << PLL_OUT_RATIO_SHIFT |
		PLL_OUT_OVR | PLL_OUT_CLKEN | PLL_OUT_RSTN) << PLL_OUT3_SHIFT |
		(CLK_DIVIDER(TEGRA_PLLP_KHZ, 204000) << PLL_OUT_RATIO_SHIFT |
		PLL_OUT_OVR | PLL_OUT_CLKEN | PLL_OUT_RSTN) << PLL_OUT4_SHIFT);

	/* init pllx */
	init_pll(CLK_RST_REG(pllx_base), CLK_RST_REG(pllx_misc),
		osc_table[osc].pllx, PLLPAXS_MISC_LOCK_ENABLE);
	write32(CLK_RST_REG(cclk_brst_pol), CCLK_BURST_POLICY_VAL);

	/* init pllu */
	init_pll(CLK_RST_REG(pllu_base), CLK_RST_REG(pllu_misc),
		osc_table[osc].pllu, PLLU_MISC_LOCK_ENABLE);

	init_utmip_pll();
	graphics_pll();
}

void clock_grp_enable_clear_reset(u32 val, u32* clk_enb_set_reg,
				  u32 *rst_dev_clr_reg)
{
	write32(clk_enb_set_reg, val);
	udelay(IO_STABILIZATION_DELAY);
	write32(rst_dev_clr_reg, val);
}

static u32 * const clk_enb_set_arr[DEV_CONFIG_BLOCKS] = {
	CLK_RST_REG(clk_enb_l_set),
	CLK_RST_REG(clk_enb_h_set),
	CLK_RST_REG(clk_enb_u_set),
	CLK_RST_REG(clk_enb_v_set),
	CLK_RST_REG(clk_enb_w_set),
	CLK_RST_REG(clk_enb_x_set),
	CLK_RST_REG(clk_enb_y_set),
};

static u32 * const clk_enb_clr_arr[DEV_CONFIG_BLOCKS] = {
	CLK_RST_REG(clk_enb_l_clr),
	CLK_RST_REG(clk_enb_h_clr),
	CLK_RST_REG(clk_enb_u_clr),
	CLK_RST_REG(clk_enb_v_clr),
	CLK_RST_REG(clk_enb_w_clr),
	CLK_RST_REG(clk_enb_x_clr),
	CLK_RST_REG(clk_enb_y_clr),
};

static u32 * const rst_dev_set_arr[DEV_CONFIG_BLOCKS] = {
	CLK_RST_REG(rst_dev_l_set),
	CLK_RST_REG(rst_dev_h_set),
	CLK_RST_REG(rst_dev_u_set),
	CLK_RST_REG(rst_dev_v_set),
	CLK_RST_REG(rst_dev_w_set),
	CLK_RST_REG(rst_dev_x_set),
	CLK_RST_REG(rst_dev_y_set),
};

static u32 * const rst_dev_clr_arr[DEV_CONFIG_BLOCKS] = {
	CLK_RST_REG(rst_dev_l_clr),
	CLK_RST_REG(rst_dev_h_clr),
	CLK_RST_REG(rst_dev_u_clr),
	CLK_RST_REG(rst_dev_v_clr),
	CLK_RST_REG(rst_dev_w_clr),
	CLK_RST_REG(rst_dev_x_clr),
	CLK_RST_REG(rst_dev_y_clr),
};

static void clock_write_regs(u32 * const regs[DEV_CONFIG_BLOCKS],
			     u32 bits[DEV_CONFIG_BLOCKS])
{
	int i = 0;

	for (; i < DEV_CONFIG_BLOCKS; i++)
		if (bits[i])
			write32(regs[i], bits[i]);
}

void clock_enable_regs(u32 bits[DEV_CONFIG_BLOCKS])
{
	clock_write_regs(clk_enb_set_arr, bits);
}

void clock_disable_regs(u32 bits[DEV_CONFIG_BLOCKS])
{
	clock_write_regs(clk_enb_clr_arr, bits);
}

void clock_set_reset_regs(u32 bits[DEV_CONFIG_BLOCKS])
{
	clock_write_regs(rst_dev_set_arr, bits);
}

void clock_clr_reset_regs(u32 bits[DEV_CONFIG_BLOCKS])
{
	clock_write_regs(rst_dev_clr_arr, bits);
}

void clock_enable_clear_reset(u32 l, u32 h, u32 u, u32 v, u32 w, u32 x, u32 y)
{
	clock_enable(l, h, u, v, w, x, y);

	/* Give clocks time to stabilize. */
	udelay(IO_STABILIZATION_DELAY);

	clock_clr_reset(l, h, u, v, w, x, y);
}

static void clock_reset_dev(u32 *setaddr, u32 *clraddr, u32 bit)
{
	write32(setaddr, bit);
	udelay(LOGIC_STABILIZATION_DELAY);
	write32(clraddr, bit);
}

void clock_reset_l(u32 bit)
{
	clock_reset_dev(CLK_RST_REG(rst_dev_l_set), CLK_RST_REG(rst_dev_l_clr),
			bit);
}

void clock_reset_h(u32 bit)
{
	clock_reset_dev(CLK_RST_REG(rst_dev_h_set), CLK_RST_REG(rst_dev_h_clr),
			bit);
}

void clock_reset_u(u32 bit)
{
	clock_reset_dev(CLK_RST_REG(rst_dev_u_set), CLK_RST_REG(rst_dev_u_clr),
			bit);
}

void clock_reset_v(u32 bit)
{
	clock_reset_dev(CLK_RST_REG(rst_dev_v_set), CLK_RST_REG(rst_dev_v_clr),
			bit);
}

void clock_reset_w(u32 bit)
{
	clock_reset_dev(CLK_RST_REG(rst_dev_w_set), CLK_RST_REG(rst_dev_w_clr),
			bit);
}

void clock_reset_x(u32 bit)
{
	clock_reset_dev(CLK_RST_REG(rst_dev_x_set), CLK_RST_REG(rst_dev_x_clr),
			bit);
}

void clock_reset_y(u32 bit)
{
	clock_reset_dev(CLK_RST_REG(rst_dev_y_set), CLK_RST_REG(rst_dev_y_clr),
			bit);
}

/* Enable/unreset all audio toys under AHUB */
void clock_enable_audio(void)
{
	/*
	 * Confirmed by NVIDIA hardware team, we need to take ALL audio devices
	 * connected to AHUB (AUDIO, APBIF, I2S, DAM, AMX, ADX, SPDIF, AFC) out
	 * of reset and clock-enabled, otherwise reading AHUB devices (in our
	 * case, I2S/APBIF/AUDIO<XBAR>) will hang.
	 */
	clock_enable_clear_reset(CLK_L_I2S0 | CLK_L_I2S1 | CLK_L_I2S2 | CLK_L_SPDIF,
				 0, 0,
				 CLK_V_I2S3 | CLK_V_I2S4 | CLK_V_AUDIO | CLK_V_APBIF |
				 CLK_V_DAM0 | CLK_V_DAM1 | CLK_V_DAM2 | CLK_V_EXTPERIPH1,
				 CLK_W_AMX0 | CLK_W_ADX0,
				 CLK_X_ADX1 | CLK_X_AFC0 | CLK_X_AFC1 | CLK_X_AFC2 |
				 CLK_X_AFC3 | CLK_X_AFC4 | CLK_X_AFC5 |
				 CLK_X_AMX1, 0);
}
