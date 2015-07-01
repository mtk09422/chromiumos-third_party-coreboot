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
#include <delay.h>
#include <soc/addressmap.h>
#include <soc/pll.h>
#include <soc/pmic.h>

#define GENMASK(h, l)	(((1U << ((h) - (l) + 1)) - 1) << (l))

enum mux_id {
	TOP_AXI_SEL,
	TOP_MEM_SEL,
	TOP_DDRPHYCFG_SEL,
	TOP_MM_SEL,
	TOP_PWM_SEL,
	TOP_VDEC_SEL,
	TOP_VENC_SEL,
	TOP_MFG_SEL,
	TOP_CAMTG_SEL,
	TOP_UART_SEL,
	TOP_SPI_SEL,
	TOP_USB20_SEL,
	TOP_USB30_SEL,
	TOP_MSDC50_0_H_SEL,
	TOP_MSDC50_0_SEL,
	TOP_MSDC30_1_SEL,
	TOP_MSDC30_2_SEL,
	TOP_MSDC30_3_SEL,
	TOP_AUDIO_SEL,
	TOP_AUD_INTBUS_SEL,
	TOP_PMICSPI_SEL,
	TOP_SCP_SEL,
	TOP_ATB_SEL,
	TOP_VENC_LT_SEL,
	TOP_DPI0_SEL,
	TOP_IRDA_SEL,
	TOP_CCI400_SEL,
	TOP_AUD_1_SEL,
	TOP_AUD_2_SEL,
	TOP_MEM_MFG_IN_SEL,
	TOP_AXI_MFG_IN_SEL,
	TOP_SCAM_SEL,
	TOP_SPINFI_IFR_SEL,
	TOP_HDMI_SEL,
	TOP_DPILVDS_SEL,
	TOP_MSDC50_2_H_SEL,
	TOP_HDCP_SEL,
	TOP_HDCP_24M_SEL,
	TOP_RTC_SEL,
	TOP_NR_MUX
};

struct mux {
	void *reg;
	u8 mux_shift;
	u8 mux_width;
	s8 gate_shift;
};

#define MUX(_id, _reg, _mux_shift, _mux_width, _gate_shift)	\
	[_id] = {						\
		.reg = (void *)_reg,				\
		.mux_shift = _mux_shift,			\
		.mux_width = _mux_width,			\
		.gate_shift = _gate_shift,			\
	}

static const struct mux muxes[] = {
	/* CLK_CFG_0 */
	MUX(TOP_AXI_SEL, CLK_CFG_0, 0, 3, 7),
	MUX(TOP_MEM_SEL, CLK_CFG_0, 8, 1, 15),
	MUX(TOP_DDRPHYCFG_SEL, CLK_CFG_0, 16, 1, 23),
	MUX(TOP_MM_SEL, CLK_CFG_0, 24, 4, 31),
	/* CLK_CFG_1 */
	MUX(TOP_PWM_SEL, CLK_CFG_1, 0, 2, 7),
	MUX(TOP_VDEC_SEL, CLK_CFG_1, 8, 4, 15),
	MUX(TOP_VENC_SEL, CLK_CFG_1, 16, 4, 23),
	MUX(TOP_MFG_SEL, CLK_CFG_1, 24, 4, 31),
	/* CLK_CFG_2 */
	MUX(TOP_CAMTG_SEL, CLK_CFG_2, 0, 3, 7),
	MUX(TOP_UART_SEL, CLK_CFG_2, 8, 1, 15),
	MUX(TOP_SPI_SEL, CLK_CFG_2, 16, 3, 23),
	MUX(TOP_USB20_SEL, CLK_CFG_2, 24, 2, 31),
	/* CLK_CFG_3 */
	MUX(TOP_USB30_SEL, CLK_CFG_3, 0, 2, 7),
	MUX(TOP_MSDC50_0_H_SEL, CLK_CFG_3, 8, 3, 15),
	MUX(TOP_MSDC50_0_SEL, CLK_CFG_3, 16, 4, 23),
	MUX(TOP_MSDC30_1_SEL, CLK_CFG_3, 24, 3, 31),
	/* CLK_CFG_4 */
	MUX(TOP_MSDC30_2_SEL, CLK_CFG_4, 0, 3, 7),
	MUX(TOP_MSDC30_3_SEL, CLK_CFG_4, 8, 4, 15),
	MUX(TOP_AUDIO_SEL, CLK_CFG_4, 16, 2, 23),
	MUX(TOP_AUD_INTBUS_SEL, CLK_CFG_4, 24, 3, 31),
	/* CLK_CFG_5 */
	MUX(TOP_PMICSPI_SEL, CLK_CFG_5, 0, 3, 7),
	MUX(TOP_SCP_SEL, CLK_CFG_5, 8, 3, 15),
	MUX(TOP_ATB_SEL, CLK_CFG_5, 16, 2, 23),
	MUX(TOP_VENC_LT_SEL, CLK_CFG_5, 24, 4, 31),
	/* CLK_CFG_6 */
	MUX(TOP_DPI0_SEL, CLK_CFG_6, 0, 3, 7),
	MUX(TOP_IRDA_SEL, CLK_CFG_6, 8, 2, 15),
	MUX(TOP_CCI400_SEL, CLK_CFG_6, 16, 3, 23),
	MUX(TOP_AUD_1_SEL, CLK_CFG_6, 24, 2, 31),
	/* CLK_CFG_7 */
	MUX(TOP_AUD_2_SEL, CLK_CFG_7, 0, 2, 7),
	MUX(TOP_MEM_MFG_IN_SEL, CLK_CFG_7, 8, 2, 15),
	MUX(TOP_AXI_MFG_IN_SEL, CLK_CFG_7, 16, 2, 23),
	MUX(TOP_SCAM_SEL, CLK_CFG_7, 24, 2, 31),
	/* CLK_CFG_12 */
	MUX(TOP_SPINFI_IFR_SEL, CLK_CFG_12, 0, 3, 7),
	MUX(TOP_HDMI_SEL, CLK_CFG_12, 8, 2, 15),
	MUX(TOP_DPILVDS_SEL, CLK_CFG_12, 24, 3, 31),
	/* CLK_CFG_13 */
	MUX(TOP_MSDC50_2_H_SEL, CLK_CFG_13, 0, 3, 7),
	MUX(TOP_HDCP_SEL, CLK_CFG_13, 8, 2, 15),
	MUX(TOP_HDCP_24M_SEL, CLK_CFG_13, 16, 2, 23),
	MUX(TOP_RTC_SEL, CLK_CFG_13, 24, 2, 31),
};

static void setbits(void *addr, u32 mask)
{
	write32(addr, read32(addr) | mask);
}

static void clrbits(void *addr, u32 mask)
{
	write32(addr, read32(addr) & ~mask);
}

static void mux_set_sel(const struct mux *mux, u32 sel)
{
	u32 mask = GENMASK(mux->mux_width - 1, 0);
	u32 val = read32(mux->reg);

	val &= ~(mask << mux->mux_shift);
	val |= (sel & mask) << mux->mux_shift;
	write32(mux->reg, val);
}

#define MHZ (1000 * 1000)

#define PLL_PWR_ON		(1 << 0)
#define PLL_EN			(1 << 0)
#define PLL_ISO			(1 << 1)
#define PLL_RSTB		(1 << 24)
#define PLL_PCW_CHG		(1 << 31)
#define PLL_POSTDIV_MASK	0x7
#define PLL_INTEGER_BITS	7

enum pll_id {
	APMIXED_ARMCA15PLL,
	APMIXED_ARMCA7PLL,
	APMIXED_MAINPLL,
	APMIXED_UNIVPLL,
	APMIXED_MMPLL,
	APMIXED_MSDCPLL,
	APMIXED_VENCPLL,
	APMIXED_TVDPLL,
	APMIXED_MPLL,
	APMIXED_VCODECPLL,
	APMIXED_APLL1,
	APMIXED_APLL2,
	APMIXED_LVDSPLL,
	APMIXED_MSDCPLL2,
	APMIXED_NR_PLL
};

const unsigned long pll_div_rate[] = {
	3000000000UL,
	1000000000,
	500000000,
	250000000,
	125000000,
	0,
};

const unsigned long mmpll_div_rate[] = {
	3000000000UL,
	1000000000,
	702000000,
	253500000,
	126750000,
	0,
};

struct pll {
	void *reg;
	void *pwr_reg;
	u32 rstb;
	u8 pcwbits;
	void *div_reg;
	u8 div_shift;
	void *pcw_reg;
	u8 pcw_shift;
	const unsigned long *div_rate;
};

#define PLL(_id, _reg, _pwr_reg, _rstb, _pcwbits, _div_reg, _div_shift, \
			_pcw_reg, _pcw_shift, _div_rate)		\
	[_id] = {							\
		.reg = (void *)_reg,					\
		.pwr_reg = (void *)_pwr_reg,				\
		.rstb = _rstb,						\
		.pcwbits = _pcwbits,					\
		.div_reg = (void *)_div_reg,				\
		.div_shift = _div_shift,				\
		.pcw_reg = (void *)_pcw_reg,				\
		.pcw_shift = _pcw_shift,				\
		.div_rate = _div_rate,					\
	}

static const struct pll plls[] = {
	PLL(APMIXED_ARMCA15PLL, ARMCA15PLL_CON0, ARMCA15PLL_PWR_CON0, 0, 21,
		ARMCA15PLL_CON1, 24, ARMCA15PLL_CON1, 0, pll_div_rate),
	PLL(APMIXED_ARMCA7PLL, ARMCA7PLL_CON0, ARMCA7PLL_PWR_CON0, PLL_RSTB, 21,
		ARMCA7PLL_CON1, 24, ARMCA7PLL_CON1, 0, pll_div_rate),
	PLL(APMIXED_MAINPLL, MAINPLL_CON0, MAINPLL_PWR_CON0, PLL_RSTB, 21,
		MAINPLL_CON0, 4, MAINPLL_CON1, 0, pll_div_rate),
	PLL(APMIXED_UNIVPLL, UNIVPLL_CON0, UNIVPLL_PWR_CON0, PLL_RSTB, 7,
		UNIVPLL_CON0, 4, UNIVPLL_CON1, 14, pll_div_rate),
	PLL(APMIXED_MMPLL, MMPLL_CON0, MMPLL_PWR_CON0, 0, 21,
		MMPLL_CON1, 24, MMPLL_CON1, 0, mmpll_div_rate),
	PLL(APMIXED_MSDCPLL, MSDCPLL_CON0, MSDCPLL_PWR_CON0, 0, 21,
		MSDCPLL_CON0, 4, MSDCPLL_CON1, 0, pll_div_rate),
	PLL(APMIXED_VENCPLL, VENCPLL_CON0, VENCPLL_PWR_CON0, 0, 21,
		VENCPLL_CON0, 4, VENCPLL_CON1, 0, pll_div_rate),
	PLL(APMIXED_TVDPLL, TVDPLL_CON0, TVDPLL_PWR_CON0, 0, 21,
		TVDPLL_CON0, 4, TVDPLL_CON1, 0, pll_div_rate),
	PLL(APMIXED_MPLL, MPLL_CON0, MPLL_PWR_CON0, 0, 21,
		MPLL_CON0, 4, MPLL_CON1, 0, pll_div_rate),
	PLL(APMIXED_VCODECPLL, VCODECPLL_CON0, VCODECPLL_PWR_CON0, 0, 21,
		VCODECPLL_CON0, 4, VCODECPLL_CON1, 0, pll_div_rate),
	PLL(APMIXED_APLL1, APLL1_CON0, APLL1_PWR_CON0, 0, 31,
		APLL1_CON0, 4, APLL1_CON1, 0, pll_div_rate),
	PLL(APMIXED_APLL2, APLL2_CON0, APLL2_PWR_CON0, 0, 31,
		APLL2_CON0, 4, APLL2_CON1, 0, pll_div_rate),
	PLL(APMIXED_LVDSPLL, LVDSPLL_CON0, LVDSPLL_PWR_CON0, 0, 21,
		LVDSPLL_CON0, 4, LVDSPLL_CON1, 0, pll_div_rate),
	PLL(APMIXED_MSDCPLL2, MSDCPLL2_CON0, MSDCPLL2_PWR_CON0, 0, 21,
		MSDCPLL2_CON0, 4, MSDCPLL2_CON1, 0, pll_div_rate),
};

static u32 div64_16(u32 *h, u32 *l, u16 base)
{
	u32 low, low2, high, rem;

	low   = *l;
	high  = *h;
	rem   = high % base;
	high  = high / base;
	low2  = low >> 16;
	low2 += rem << 16;
	rem   = low2 % base;
	low2  = low2 / base;
	low   = low  & 0xffff;
	low  += rem << 16;
	rem   = low  % base;
	low   = low  / base;

	*l = low + (low2 << 16);
	*h = high + (low2 >> 16);

	return rem;
}

static void pll_set_rate_regs(const struct pll *pll, u32 pcw, u32 postdiv)
{
	u32 val;

	/* set postdiv */
	val = read32(pll->div_reg);
	val &= ~(PLL_POSTDIV_MASK << pll->div_shift);
	val |= postdiv << pll->div_shift;

	/* postdiv and pcw need to set at the same time if on same register */
	if (pll->div_reg != pll->pcw_reg) {
		val |= PLL_PCW_CHG;
		write32(pll->div_reg, val);
		val = read32(pll->pcw_reg);
	}

	/* set pcw */
	val &= ~GENMASK(pll->pcw_shift + pll->pcwbits - 1, pll->pcw_shift);
	val |= pcw << pll->pcw_shift;
	val |= PLL_PCW_CHG;
	write32(pll->pcw_reg, val);
}

static void pll_calc_values(const struct pll *pll, u32 *pcw, u32 *postdiv,
			    u32 freq)
{
	const u16 fin_khz = 26000;
	const unsigned long *div_rate = pll->div_rate;
	u32 pcw_h, pcw_l;
	u32 val;

	if (freq > div_rate[0])
		freq = div_rate[0];

	for (val = 1; div_rate[val] != 0; val++) {
		if (freq > div_rate[val])
			break;
	}
	val--;
	*postdiv = val;

	/* _pcw = freq * postdiv / fin * 2^pcwfbits */
	val += pll->pcwbits - PLL_INTEGER_BITS;
	freq /= 1000;
	pcw_l = freq << val;
	if (val > 32)
		pcw_h = freq << (val - 32);
	else
		pcw_h = freq >> (32 - val);

	div64_16(&pcw_h, &pcw_l, fin_khz);

	*pcw = pcw_l;
}

static int pll_set_rate(const struct pll *pll, unsigned long rate)
{
	u32 pcw = 0;
	u32 postdiv;

	pll_calc_values(pll, &pcw, &postdiv, rate);
	pll_set_rate_regs(pll, pcw, postdiv);

	return 0;
}

static void set_freq(int mode, unsigned long clks)
{
	int wait_pll = 0;

	if (clks & FCLK_VENCPLL) {
		if (mode == FREQ_SP) {
			/* 660 MHz */
			pll_set_rate(&plls[APMIXED_VENCPLL], 660 * MHZ);
		} else if (mode == FREQ_HP) {
			/* 800 MHz */
			pll_set_rate(&plls[APMIXED_VENCPLL], 800 * MHZ);
		}
		wait_pll = 1;
	}

	if (clks & FCLK_VCODECPLL) {
		if (mode == FREQ_SP) {
			/* 384 MHz (1152 / 3) */
			pll_set_rate(&plls[APMIXED_VCODECPLL], 1152 * MHZ);
		} else if (mode == FREQ_HP) {
			/* 494 MHz (1482 / 3) */
			pll_set_rate(&plls[APMIXED_VCODECPLL], 1482 * MHZ);
		}
		wait_pll = 1;
	}

	if (clks & FCLK_MMPLL) {
		if (mode == FREQ_SP) {
			/* 455 MHz */
			pll_set_rate(&plls[APMIXED_MMPLL], 455 * MHZ);
		} else if (mode == FREQ_HP) {
			/* 600 MHz */
			pll_set_rate(&plls[APMIXED_MMPLL], 600 * MHZ);
		}
		wait_pll = 1;
	}

	if (clks & FCLK_AXI_CK) {
		if (mode == FREQ_SP) {
			/* axi_ck = 208 MHz (UNIVPLL2_D2) */
			mux_set_sel(&muxes[TOP_AXI_SEL], 5);
		} else if (mode == FREQ_HP) {
			/* axi_ck = 273 MHz (SYSPLL1_D2) */
			mux_set_sel(&muxes[TOP_AXI_SEL], 1);
		}
	}

	if (clks & FCLK_VENCLT_CK) {
		if (mode == FREQ_SP) {
			/* venclite_ck = 312 MHz (UNIVPLL1_D2) */
			mux_set_sel(&muxes[TOP_VENC_LT_SEL], 6);
		} else if (mode == FREQ_HP) {
			/* venclite_ck = 370.5 MHz (VCODECPLL_370P5_CK) */
			mux_set_sel(&muxes[TOP_VENC_LT_SEL], 10);
		}
	}

	if (clks & FCLK_CCI400_CK) {
		if (mode == FREQ_SP) {
			/* cci400_ck = 546 MHz (SYSPLL_D2) */
			mux_set_sel(&muxes[TOP_CCI400_SEL], 5);
		} else if (mode == FREQ_HP) {
			/* cci400_ck = 624 MHz (UNIVPLL_D2) */
			mux_set_sel(&muxes[TOP_CCI400_SEL], 4);
		}
	}

	if (wait_pll)
		udelay(5);
}

void mt_pll_init(void)
{
	u32 reg_value;
	int i;

	/* reduce CLKSQ disable time */
	write32((void *)(uintptr_t)CLKSQ_STB_CON0, 0x05010501);
	/* extend PWR/ISO control timing to 1us */
	write32((void *)(uintptr_t)PLL_ISO_CON0, 0x00080008);
	write32((void *)(uintptr_t)AP_PLL_CON6, 0x00000000);

	/*************
	 * xPLL PWR ON
	 **************/
	for (i = 0; i < APMIXED_NR_PLL; i++)
		setbits(plls[i].pwr_reg, PLL_PWR_ON);

	udelay(5); /* wait for xPLL_PWR_ON ready (min delay is 1us) */

	/******************
	 * xPLL ISO Disable
	 *******************/
	for (i = 0; i < APMIXED_NR_PLL; i++)
		clrbits(plls[i].pwr_reg, PLL_ISO);

	/********************
	 * xPLL Frequency Set
	 *********************/

	pll_set_rate(&plls[APMIXED_ARMCA15PLL], 851.5 * MHZ);
	pll_set_rate(&plls[APMIXED_ARMCA7PLL], 1105 * MHZ);
	pll_set_rate(&plls[APMIXED_MAINPLL], 1092 * MHZ);
	pll_set_rate(&plls[APMIXED_MMPLL], 455 * MHZ);
	pll_set_rate(&plls[APMIXED_MSDCPLL], 800 * MHZ);
	pll_set_rate(&plls[APMIXED_VENCPLL], 660 * MHZ);
	pll_set_rate(&plls[APMIXED_TVDPLL], 1782 * MHZ);
	pll_set_rate(&plls[APMIXED_MPLL], 1456 * MHZ);
	pll_set_rate(&plls[APMIXED_VCODECPLL], 1104 * MHZ);
	pll_set_rate(&plls[APMIXED_LVDSPLL], 150 * MHZ);
	pll_set_rate(&plls[APMIXED_MSDCPLL2], 800 * MHZ);

	/*
	 * assign pcw value directly for APLLs due to the precision of
	 * div64_16() is not enough
	 */
	pll_set_rate_regs(&plls[APMIXED_APLL1], 0x37945EA6, 3); /* 180.6336M */
	pll_set_rate_regs(&plls[APMIXED_APLL2], 0x3C7EA932, 3); /* 196.608M */

	/***********************
	 * xPLL Frequency Enable
	 ************************/
	for (i = 0; i < APMIXED_NR_PLL; i++)
		setbits(plls[i].reg, PLL_EN);

	udelay(40); /* wait for PLL stable (min delay is 20us) */

	/***************
	 * xPLL DIV RSTB
	 ****************/
	for (i = 0; i < APMIXED_NR_PLL; i++) {
		if (plls[i].rstb)
			setbits(plls[i].reg, plls[i].rstb);
	}

	/**************
	 * INFRA CLKMUX
	 ***************/

	reg_value = read32((void *)(uintptr_t)TOP_DCMCTL);
	/* enable infrasys DCM */
	write32((void *)(uintptr_t)TOP_DCMCTL, reg_value | 0x1);

	write32((void *)(uintptr_t)CLK_MODE, 0x1);
	write32((void *)(uintptr_t)CLK_MODE, 0x0); /* enable TOPCKGEN */

	/************
	 * TOP CLKMUX
	 *************/

	/* CLK_CFG_0 */
	mux_set_sel(&muxes[TOP_AXI_SEL], 5);		/* 5: univpll2_d2 */
	mux_set_sel(&muxes[TOP_MEM_SEL], 0);		/* 0: clk26m */
	mux_set_sel(&muxes[TOP_DDRPHYCFG_SEL], 0);	/* 0: clk26m */
	mux_set_sel(&muxes[TOP_MM_SEL], 1);		/* 1: vencpll_d2 */
	/* CLK_CFG_1 */
	mux_set_sel(&muxes[TOP_PWM_SEL], 0);		/* 0: clk26m */
	mux_set_sel(&muxes[TOP_VDEC_SEL], 1);		/* 1: vcodecpll_ck */
	mux_set_sel(&muxes[TOP_VENC_SEL], 1);		/* 1: vcodecpll_ck */
	mux_set_sel(&muxes[TOP_MFG_SEL], 1);		/* 1: mmpll_ck */
	/* CLK_CFG_2 */
	mux_set_sel(&muxes[TOP_CAMTG_SEL], 0);		/* 0: clk26m */
	mux_set_sel(&muxes[TOP_UART_SEL], 0);		/* 0: clk26m */
	mux_set_sel(&muxes[TOP_SPI_SEL], 1);		/* 1: syspll3_d2 */
	mux_set_sel(&muxes[TOP_USB20_SEL], 1);		/* 1: univpll1_d8 */
	/* CLK_CFG_4 */
	mux_set_sel(&muxes[TOP_MSDC30_2_SEL], 2);	/* 2: msdcpll_d4 */
	mux_set_sel(&muxes[TOP_MSDC30_3_SEL], 5);	/* 5: msdcpll_d4 */
	mux_set_sel(&muxes[TOP_AUDIO_SEL], 0);		/* 0: clk26m */
	mux_set_sel(&muxes[TOP_AUD_INTBUS_SEL], 1);	/* 1: syspll1_d4 */
	/* CLK_CFG_5 */
	mux_set_sel(&muxes[TOP_PMICSPI_SEL], 0);	/* 0: clk26m */
	mux_set_sel(&muxes[TOP_SCP_SEL], 1);		/* 1: syspll1_d2 */
	mux_set_sel(&muxes[TOP_ATB_SEL], 0);		/* 0: clk26m */
	mux_set_sel(&muxes[TOP_VENC_LT_SEL], 6);	/* 6: univpll1_d2 */
	/* CLK_CFG_6 */
	mux_set_sel(&muxes[TOP_DPI0_SEL], 1);		/* 1: tvdpll_d2 */
	mux_set_sel(&muxes[TOP_IRDA_SEL], 1);		/* 1: univpll2_d4 */
	mux_set_sel(&muxes[TOP_CCI400_SEL], 5);		/* 5: syspll_d2 */
	mux_set_sel(&muxes[TOP_AUD_1_SEL], 1);		/* 1: apll1_ck */
	/* CLK_CFG_7 */
	mux_set_sel(&muxes[TOP_AUD_2_SEL], 1);		/* 1: apll2_ck */
	mux_set_sel(&muxes[TOP_MEM_MFG_IN_SEL], 1);	/* 1: mmpll_ck */
	mux_set_sel(&muxes[TOP_AXI_MFG_IN_SEL], 1);	/* 1: hd_faxi_ck */
	mux_set_sel(&muxes[TOP_SCAM_SEL], 1);		/* 1: syspll3_d2 */
	/* CLK_CFG_12 */
	mux_set_sel(&muxes[TOP_SPINFI_IFR_SEL], 0);	/* 0: clk26m */
	mux_set_sel(&muxes[TOP_HDMI_SEL], 1);		/* 1: AD_HDMITX_CLK */
	mux_set_sel(&muxes[TOP_DPILVDS_SEL], 1);	/* 1: AD_LVDSPLL_CK */
	/* CLK_CFG_13 */
	mux_set_sel(&muxes[TOP_MSDC50_2_H_SEL], 2);	/* 2: syspll2_d2 */
	mux_set_sel(&muxes[TOP_HDCP_SEL], 2);		/* 2: syspll3_d4 */
	mux_set_sel(&muxes[TOP_HDCP_24M_SEL], 2);	/* 2: univpll_d52 */
	mux_set_sel(&muxes[TOP_RTC_SEL], 1);		/* 1: clkrtc_ext */
	/* CLK_CFG_3 */
	mux_set_sel(&muxes[TOP_USB30_SEL], 1);		/* 1: univpll3_d2 */
	mux_set_sel(&muxes[TOP_MSDC50_0_H_SEL], 2);	/* 2: syspll2_d2 */
	mux_set_sel(&muxes[TOP_MSDC50_0_SEL], 6);	/* 6: msdcpll_d4 */
	mux_set_sel(&muxes[TOP_MSDC30_1_SEL], 2);	/* 2: msdcpll_d4 */

	/* enable scpsys clock off control */
	write32((void *)(uintptr_t)CLK_SCP_CFG_0, 0x7FF);
	write32((void *)(uintptr_t)CLK_SCP_CFG_1, 0x15);
}

void mt_vcore_pll_adjust(void)
{
	/* Vcore 1.125v */
	mt6391_write(PMIC_RG_VCORE_CON9, 0x44, 0x7F, 0);
	mt6391_write(PMIC_RG_VCORE_CON10, 0x44, 0x7F, 0);

	set_freq(FREQ_HP, FCLK_MMPLL);
	set_freq(FREQ_HP, FCLK_AXI_CK);
	set_freq(FREQ_HP, FCLK_CCI400_CK);
	set_freq(FREQ_HP, FCLK_VENCPLL);
	set_freq(FREQ_HP, FCLK_VCODECPLL);
	set_freq(FREQ_HP, FCLK_VENCLT_CK);
}

/* after pmic_init */
void mt_pll_post_init(void)
{
	u32 reg_value;

	/* UNIVPLL SW Control */
	reg_value = read32((void *)(uintptr_t)AP_PLL_CON3);
	write32((void *)(uintptr_t)AP_PLL_CON3, reg_value & 0xFFF44440);

	reg_value = read32((void *)(uintptr_t)AP_PLL_CON4);
	write32((void *)(uintptr_t)AP_PLL_CON4, reg_value & 0xFFFFFFF4);
}

/* after pmic_init */
void mt_arm_pll_sel(void)
{
	u32 reg_value;

	reg_value = read32((void *)(uintptr_t)TOP_CKDIV1);
	/* CPU clock divide by 1 */
	write32((void *)(uintptr_t)TOP_CKDIV1, reg_value & 0xFFFFFC00);

	write32((void *)(uintptr_t)TOP_CKMUXSEL, 0x0245); /* select ARMPLL */

	printk(BIOS_DEBUG, "[PLL] %s done\n", __func__);
}
