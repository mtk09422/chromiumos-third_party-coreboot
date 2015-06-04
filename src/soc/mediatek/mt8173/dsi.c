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
#include <soc/i2c.h>
#include <soc/gpio.h>
#include <soc/dsi.h>
#include <soc/it6151.h>
#include <soc/ps8640.h>

#ifdef DEBUGDPF
#define MTKFBTAG                "[MTKFB] "
#define MTKFBERRTAG             "[MTKFB ERR] "
#define MTKFBDPF(fmt, arg...)   printk(BIOS_INFO, MTKFBTAG fmt, ##arg)
#define MTKFBERR(fmt, arg...)   printk(BIOS_INFO, MTKFBERRTAG fmt, ##arg)
#else
#define MTKFBERRTAG             "[MTKFB ERR] "
#define MTKFBDPF(fmt, arg...)
#define MTKFBERR(fmt, arg...)   printk(BIOS_INFO, MTKFBERRTAG fmt, ##arg)
#endif

struct mtk_dsi {
	void *dsi_reg_base;
	void *dsi_tx_reg_base;
	u32 pll_clk_rate;
	unsigned long mode_flags;
	enum mipi_dsi_pixel_format format;
	unsigned int lanes;
	struct videomode vm;
	u8 enabled;
};

static struct mtk_dsi _mtk_dsi = {
	.dsi_reg_base = (void *)0x1401b000,
	.dsi_tx_reg_base = (void *)0x10215000,
	.pll_clk_rate = 230,
	.mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_SYNC_PULSE,
	.format = MIPI_DSI_FMT_RGB888,
	.lanes = 4,
};

static inline void dsi_write(u32 data, void *base, u32 offset)
{
	write32(base + offset, data);
}

static inline u32 dsi_read(void *base, u32 offset)
{
	return read32(base + offset);
}


static void dsi_phy_clk_switch(struct mtk_dsi *dsi, u8 on)
{
	u32 tmp_reg;
	void *base = dsi->dsi_tx_reg_base;

	if (on) {
		tmp_reg = dsi_read(base, MIPITX_DSI_SW_CTRL);
		tmp_reg = (tmp_reg & (~SW_CTRL_EN));
		dsi_write(tmp_reg, base, MIPITX_DSI_SW_CTRL);
	} else {
		tmp_reg = dsi_read(base, MIPITX_DSI_SW_CTRL_CON0);
		tmp_reg = tmp_reg  | (SW_LNTC_LPTX_PRE_OE | SW_LNTC_LPTX_OE |
			  SW_LNTC_HSTX_PRE_OE | SW_LNTC_HSTX_OE |
			  SW_LNT0_LPTX_PRE_OE | SW_LNT0_LPTX_OE |
			  SW_LNT0_HSTX_PRE_OE | SW_LNT0_HSTX_OE |
			  SW_LNT1_LPTX_PRE_OE | SW_LNT1_LPTX_OE |
			  SW_LNT1_HSTX_PRE_OE | SW_LNT1_HSTX_OE |
			  SW_LNT2_LPTX_PRE_OE | SW_LNT2_LPTX_OE |
			  SW_LNT2_HSTX_PRE_OE | SW_LNT2_HSTX_OE);
		dsi_write(tmp_reg, base, MIPITX_DSI_SW_CTRL_CON0);

		tmp_reg = dsi_read(base, MIPITX_DSI_SW_CTRL);
		tmp_reg = (tmp_reg | SW_CTRL_EN);
		dsi_write(tmp_reg, base, MIPITX_DSI_SW_CTRL);


		tmp_reg = dsi_read(base, MIPITX_DSI_PLL_CON0);
		tmp_reg = (tmp_reg & (~RG_DSI0_MPPLL_PLL_EN));
		dsi_write(tmp_reg, base, MIPITX_DSI_PLL_CON0);

		tmp_reg = dsi_read(base, MIPITX_DSI_TOP_CON);
		tmp_reg = (tmp_reg & (~(RG_DSI_LNT_HS_BIAS_EN |
			RG_DSI_LNT_IMP_CAL_EN |
			RG_DSI_LNT_TESTMODE_EN)));
		dsi_write(tmp_reg, base, MIPITX_DSI_TOP_CON);

		tmp_reg = dsi_read(base, MIPITX_DSI0_CLOCK_LANE);
		tmp_reg = tmp_reg & (~RG_DSI0_LNTC_LDOOUT_EN);
		dsi_write(tmp_reg, base, MIPITX_DSI0_CLOCK_LANE);

		tmp_reg = dsi_read(base, MIPITX_DSI0_DATA_LANE0);
		tmp_reg = tmp_reg & (~RG_DSI0_LNT0_LDOOUT_EN);
		dsi_write(tmp_reg, base, MIPITX_DSI0_DATA_LANE0);

		tmp_reg = dsi_read(base, MIPITX_DSI0_DATA_LANE1);
		tmp_reg = tmp_reg & (~RG_DSI0_LNT1_LDOOUT_EN);
		dsi_write(tmp_reg, base, MIPITX_DSI0_DATA_LANE1);

		tmp_reg = dsi_read(base, MIPITX_DSI0_DATA_LANE2);
		tmp_reg = tmp_reg & (~RG_DSI0_LNT2_LDOOUT_EN);
		dsi_write(tmp_reg, base, MIPITX_DSI0_DATA_LANE2);

		tmp_reg = dsi_read(base, MIPITX_DSI0_DATA_LANE3);
		tmp_reg = tmp_reg & (~RG_DSI0_LNT3_LDOOUT_EN);
		dsi_write(tmp_reg, base, MIPITX_DSI0_DATA_LANE3);

		tmp_reg = dsi_read(base, MIPITX_DSI0_CON);
		tmp_reg = tmp_reg & (~(RG_DSI0_CKG_LDOOUT_EN |
			RG_DSI0_LDOCORE_EN));
		dsi_write(tmp_reg, base, MIPITX_DSI0_CON);

		tmp_reg = dsi_read(base, MIPITX_DSI_BG_CON);
		tmp_reg = tmp_reg & (~(RG_DSI_BG_CKEN | RG_DSI_BG_CORE_EN));
		dsi_write(tmp_reg, base, MIPITX_DSI_BG_CON);
	}

	if (on) {
		tmp_reg = dsi_read(dsi->dsi_tx_reg_base, MIPITX_DSI_PLL_CON0);
		tmp_reg = (tmp_reg | RG_DSI0_MPPLL_PLL_EN);
		dsi_write(tmp_reg, dsi->dsi_tx_reg_base, MIPITX_DSI_PLL_CON0);
	} else {
		tmp_reg = dsi_read(dsi->dsi_tx_reg_base, MIPITX_DSI_PLL_CON0);
		tmp_reg = (tmp_reg & (~RG_DSI0_MPPLL_PLL_EN));
		dsi_write(tmp_reg, dsi->dsi_tx_reg_base, MIPITX_DSI_PLL_CON0);
	}
}

static void dsi_phy_clk_setting(struct mtk_dsi *dsi)
{
	unsigned int data_rate = dsi->vm.pixelclock * 3 * 21 /(1 * 1000 * 10);
	unsigned int txdiv = 0;
	unsigned int txdiv0 = 0;
	unsigned int txdiv1 = 0;
	unsigned int pcw = 0;
	u32 reg;
	u32 temp;

	reg = dsi_read(dsi->dsi_tx_reg_base, MIPITX_DSI_BG_CON);
	reg = (reg & (~RG_DSI_V032_SEL)) | (4 << 17);
	reg = (reg & (~RG_DSI_V04_SEL)) | (4 << 14);
	reg = (reg & (~RG_DSI_V072_SEL)) | (4 << 11);
	reg = (reg & (~RG_DSI_V10_SEL)) | (4 << 8);
	reg = (reg & (~RG_DSI_V12_SEL)) | (4 << 5);
	reg = (reg & (~RG_DSI_BG_CKEN)) | (1 << 1);
	reg = (reg & (~RG_DSI_BG_CORE_EN)) | (1);
	dsi_write(reg, dsi->dsi_tx_reg_base, MIPITX_DSI_BG_CON);

	udelay(30);

	reg = dsi_read(dsi->dsi_tx_reg_base, MIPITX_DSI_TOP_CON);
	reg = (reg & (~RG_DSI_LNT_IMP_CAL_CODE)) | (8 << 4);
	reg = (reg & (~RG_DSI_LNT_HS_BIAS_EN)) | (1 << 1);
	dsi_write(reg, dsi->dsi_tx_reg_base, MIPITX_DSI_TOP_CON);

	reg = dsi_read(dsi->dsi_tx_reg_base, MIPITX_DSI0_CON);
	reg = (reg & (~RG_DSI0_CKG_LDOOUT_EN)) | (1 << 1);
	reg = (reg & (~RG_DSI0_LDOCORE_EN)) | (1);
	dsi_write(reg, dsi->dsi_tx_reg_base, MIPITX_DSI0_CON);

	reg = dsi_read(dsi->dsi_tx_reg_base, MIPITX_DSI_PLL_PWR);
	reg = (reg & (~RG_DSI_MPPLL_SDM_PWR_ON)) | (1 << 0);
	reg = (reg & (~RG_DSI_MPPLL_SDM_ISO_EN));
	dsi_write(reg, dsi->dsi_tx_reg_base, MIPITX_DSI_PLL_PWR);

	reg = dsi_read(dsi->dsi_tx_reg_base, MIPITX_DSI_PLL_CON0);
	reg = (reg & (~RG_DSI0_MPPLL_PLL_EN));
	dsi_write(reg, dsi->dsi_tx_reg_base, MIPITX_DSI_PLL_CON0);

	if (data_rate > 1250) {
		txdiv = 1;
		txdiv0 = 0;
		txdiv1 = 0;
	} else if (data_rate >= 500) {
		txdiv = 1;
		txdiv0 = 0;
		txdiv1 = 0;
	} else if (data_rate >= 250) {
		txdiv = 2;
		txdiv0 = 1;
		txdiv1 = 0;
	} else if (data_rate >= 125) {
		txdiv = 4;
		txdiv0 = 2;
		txdiv1 = 0;
	} else if (data_rate > 62) {
		txdiv = 8;
		txdiv0 = 2;
		txdiv1 = 1;
	} else if (data_rate >= 50) {
		txdiv = 16;
		txdiv0 = 2;
		txdiv1 = 2;
	} else {
	}

	reg = dsi_read(dsi->dsi_tx_reg_base, MIPITX_DSI_PLL_CON0);

	switch (txdiv) {
	case 1:
		reg = (reg & (~RG_DSI0_MPPLL_TXDIV0)) | (0 << 3);
		reg = (reg & (~RG_DSI0_MPPLL_TXDIV1)) | (0 << 5);

		break;
	case 2:
		reg = (reg & (~RG_DSI0_MPPLL_TXDIV0)) | (1 << 3);
		reg = (reg & (~RG_DSI0_MPPLL_TXDIV1)) | (0 << 5);
		break;
	case 4:
		reg = (reg & (~RG_DSI0_MPPLL_TXDIV0)) | (2 << 3);
		reg = (reg & (~RG_DSI0_MPPLL_TXDIV1)) | (0 << 5);
		break;
	case 8:
		reg = (reg & (~RG_DSI0_MPPLL_TXDIV0)) | (2 << 3);
		reg = (reg & (~RG_DSI0_MPPLL_TXDIV1)) | (1 << 5);
		break;
	case 16:
		reg = (reg & (~RG_DSI0_MPPLL_TXDIV0)) | (2 << 3);
		reg = (reg & (~RG_DSI0_MPPLL_TXDIV1)) | (2 << 5);
		break;

	default:
		break;
	}
	reg = (reg & (~RG_DSI0_MPPLL_PREDIV));
	dsi_write(reg, dsi->dsi_tx_reg_base, MIPITX_DSI_PLL_CON0);

	/*
	 * PLL PCW config
	 * PCW bit 24~30 = floor(pcw)
	 * PCW bit 16~23 = (pcw - floor(pcw))*256
	 * PCW bit 8~15 = (pcw*256 - floor(pcw)*256)*256
	 * PCW bit 8~15 = (pcw*256*256 - floor(pcw)*256*256)*256
	 * pcw = data_rate*4*txdiv/(26*2); Post DIV =4, so need data_rate*4
	 */
	pcw = data_rate * txdiv / 13;
	temp = data_rate * txdiv % 13;
	reg = ((pcw & 0x7f) << 24) + (((256 * temp / 13) & 0xff) << 16) +
	      (((256 * (256 * temp % 13) / 13) & 0xFF) << 8) +
	      ((256 * (256 * (256 * temp % 13) % 13) / 13) & 0xff);
	dsi_write(reg, dsi->dsi_tx_reg_base, MIPITX_DSI_PLL_CON2);


	reg = dsi_read(dsi->dsi_tx_reg_base, MIPITX_DSI_PLL_CON1);
	reg = (reg & (~RG_DSI0_MPPLL_SDM_FRA_EN)) | (1 << 0);
	dsi_write(reg, dsi->dsi_tx_reg_base, MIPITX_DSI_PLL_CON1);

	reg = dsi_read(dsi->dsi_tx_reg_base, MIPITX_DSI0_CLOCK_LANE);
	reg = (reg & (~RG_DSI0_LNTC_LDOOUT_EN)) | (1 << 0);
	dsi_write(reg, dsi->dsi_tx_reg_base, MIPITX_DSI0_CLOCK_LANE);

	reg = dsi_read(dsi->dsi_tx_reg_base, MIPITX_DSI0_DATA_LANE0);
	reg = (reg & (~RG_DSI0_LNT0_LDOOUT_EN)) | (1 << 0);
	dsi_write(reg, dsi->dsi_tx_reg_base, MIPITX_DSI0_DATA_LANE0);

	reg = dsi_read(dsi->dsi_tx_reg_base, MIPITX_DSI0_DATA_LANE1);
	reg = (reg & (~RG_DSI0_LNT1_LDOOUT_EN)) | (1 << 0);
	dsi_write(reg, dsi->dsi_tx_reg_base, MIPITX_DSI0_DATA_LANE1);

	reg = dsi_read(dsi->dsi_tx_reg_base, MIPITX_DSI0_DATA_LANE2);
	reg = (reg & (~RG_DSI0_LNT2_LDOOUT_EN)) | (1 << 0);
	dsi_write(reg, dsi->dsi_tx_reg_base, MIPITX_DSI0_DATA_LANE2);

	reg = dsi_read(dsi->dsi_tx_reg_base, MIPITX_DSI0_DATA_LANE3);
	reg = (reg & (~RG_DSI0_LNT3_LDOOUT_EN)) | (1 << 0);
	dsi_write(reg, dsi->dsi_tx_reg_base, MIPITX_DSI0_DATA_LANE3);

	reg = dsi_read(dsi->dsi_tx_reg_base, MIPITX_DSI_PLL_CON0);
	reg = (reg & (~RG_DSI0_MPPLL_PLL_EN)) | (1 << 0);
	dsi_write(reg, dsi->dsi_tx_reg_base, MIPITX_DSI_PLL_CON0);

	udelay(40);

	reg = dsi_read(dsi->dsi_tx_reg_base, MIPITX_DSI_PLL_CON1);
	reg = (reg & (~RG_DSI0_MPPLL_SDM_SSC_EN));
	dsi_write(reg, dsi->dsi_tx_reg_base, MIPITX_DSI_PLL_CON1);

	reg = dsi_read(dsi->dsi_tx_reg_base, MIPITX_DSI_TOP_CON);
	reg = (reg & (~RG_DSI_PAD_TIE_LOW_EN));
	dsi_write(reg, dsi->dsi_tx_reg_base, MIPITX_DSI_TOP_CON);

}

static void dsi_phy_timconfig(struct mtk_dsi *dsi)
{
	u32 timcon0 = 0;
	u32 timcon1 = 0;
	u32 timcon2 = 0;
	u32 timcon3 = 0;
	unsigned int lane_no = dsi->lanes;

	unsigned int cycle_time;
	unsigned int ui;
	unsigned int hs_trail_m, hs_trail_n;

	ui = 1000 / (250 * 2) + 0x01;
	cycle_time = 8000 / (250 * 2) + 0x01;

	#define NS_TO_CYCLE(n, c)    ((n) / c + (((n) % c) ? 1 : 0))

	hs_trail_m = lane_no;
	hs_trail_n =  NS_TO_CYCLE(((lane_no * 4 * ui) + 60), cycle_time);

	timcon0 = (timcon0 & (~HS_TRAIL)) | (8 << 24);
	timcon0 = (timcon0 & (~HS_PRPR)) | 0x6 << 8;

	if ((timcon0 & HS_PRPR) == 0)
		timcon0 = (timcon0 & (~HS_PRPR)) | 1 << 8;

	timcon0 =  (timcon0 & (~HS_ZERO)) | 0xa << 16;
	timcon0 =  (timcon0 & (~LPX)) | 5;

	if ((timcon0 & LPX) == 0)
		timcon0 =  (timcon0 & (~LPX)) | 1;

	timcon1 = (timcon1 & (~TA_GET)) | (5 * (timcon0 & LPX) << 16);

	timcon1 = (timcon1 & (~TA_SURE)) | ((3 * (timcon0 & LPX) / 2) << 8);
	timcon1 = (timcon1 & (~TA_GO)) | (4 * (timcon0 & LPX));

	timcon1 = (timcon1 & (~DA_HS_EXIT)) | (7 << 24);

	timcon2 = (timcon2 & (~CLK_TRAIL)) |
		  ((NS_TO_CYCLE(0x64, cycle_time) + 0x0a) << 24);

	if (((timcon2 & CLK_TRAIL) >> 24) < 2)
		timcon2 = (timcon2 & (~CLK_TRAIL)) | (2 << 24);


	timcon2 = (timcon2 & (~CONT_DET));
	timcon3 = (timcon3 & (~CLK_HS_PRPR)) | NS_TO_CYCLE(0x40, cycle_time);
	if ((timcon3 & CLK_HS_PRPR) == 0)
		timcon3 = (timcon3 & (~CLK_HS_PRPR)) | 1;

	timcon2 = (timcon2 & (~CLK_ZERO)) |
		  (NS_TO_CYCLE(0x190 - (timcon3 & CLK_HS_PRPR) * cycle_time,
			       cycle_time) << 16);


	timcon3 =  (timcon3 & (~CLK_HS_EXIT)) | ((2 * (timcon0 & LPX)) << 16);

	timcon3 =  (timcon3 & (~CLK_HS_POST)) |
		   (NS_TO_CYCLE((80 + 52 * ui), cycle_time) << 8);

	dsi_write(timcon0, dsi->dsi_reg_base, DSI_PHY_TIMECON0);
	dsi_write(timcon1, dsi->dsi_reg_base, DSI_PHY_TIMECON1);
	dsi_write(timcon2, dsi->dsi_reg_base, DSI_PHY_TIMECON2);
	dsi_write(timcon3, dsi->dsi_reg_base, DSI_PHY_TIMECON3);

}


static void mtk_dsi_reset(struct mtk_dsi *dsi)
{
	dsi_write(3, dsi->dsi_reg_base, DSI_CON_CTRL);
	dsi_write(2, dsi->dsi_reg_base, DSI_CON_CTRL);
}


static int mtk_dsi_poweron(struct mtk_dsi *dsi)
{
	dsi_phy_clk_setting(dsi);

	mtk_dsi_reset((dsi));

	dsi_phy_timconfig(dsi);

	return 0;
}


static void dsi_clk_ulp_mode(struct mtk_dsi *dsi, u8 enter)
{
	u32 tmp_reg1;

	tmp_reg1 = dsi_read(dsi->dsi_reg_base, DSI_PHY_LCCON);

	if (enter) {
		tmp_reg1 = tmp_reg1 & (~LC_HS_TX_EN);
		dsi_write(tmp_reg1, dsi->dsi_reg_base, DSI_PHY_LCCON);
		udelay(100);
		tmp_reg1 = tmp_reg1 & (~LC_ULPM_EN);
		dsi_write(tmp_reg1, dsi->dsi_reg_base, DSI_PHY_LCCON);
		udelay(100);

	} else {
		tmp_reg1 = tmp_reg1 & (~LC_ULPM_EN);
		dsi_write(tmp_reg1, dsi->dsi_reg_base, DSI_PHY_LCCON);
		udelay(100);
		tmp_reg1 = tmp_reg1 | LC_WAKEUP_EN;
		dsi_write(tmp_reg1, dsi->dsi_reg_base, DSI_PHY_LCCON);
		udelay(100);
		tmp_reg1 = tmp_reg1 & (~LC_WAKEUP_EN);
		dsi_write(tmp_reg1, dsi->dsi_reg_base, DSI_PHY_LCCON);
		udelay(100);

	}
}


static void dsi_lane0_ulp_mode(struct mtk_dsi *dsi, u8 enter)
{
	u32 tmp_reg1;

	tmp_reg1 = dsi_read(dsi->dsi_reg_base, DSI_PHY_LD0CON);

	if (enter) {

		tmp_reg1 = tmp_reg1 & (~LD0_HS_TX_EN);
		dsi_write(tmp_reg1, dsi->dsi_reg_base, DSI_PHY_LD0CON);
		udelay(100);
		tmp_reg1 = tmp_reg1 & (~LD0_ULPM_EN);
		dsi_write(tmp_reg1, dsi->dsi_reg_base, DSI_PHY_LD0CON);
		udelay(100);

	} else {
		tmp_reg1 = tmp_reg1 & (~LD0_ULPM_EN);
		dsi_write(tmp_reg1, dsi->dsi_reg_base, DSI_PHY_LD0CON);
		udelay(100);
		tmp_reg1 = tmp_reg1 | LD0_WAKEUP_EN;
		dsi_write(tmp_reg1, dsi->dsi_reg_base, DSI_PHY_LD0CON);
		udelay(100);
		tmp_reg1 = tmp_reg1 & (~LD0_WAKEUP_EN);
		dsi_write(tmp_reg1, dsi->dsi_reg_base, DSI_PHY_LD0CON);
		udelay(100);

	}
}

static u8 dsi_clk_hs_state(struct mtk_dsi *dsi)
{

	u32 tmp_reg1;

	tmp_reg1 = dsi_read(dsi->dsi_reg_base, DSI_PHY_LCCON);

	return ((tmp_reg1 & LC_HS_TX_EN) == 1) ? 1 : 0;
}


static void dsi_clk_hs_mode(struct mtk_dsi *dsi, u8 enter)
{
	u32 tmp_reg1;

	tmp_reg1 = dsi_read(dsi->dsi_reg_base, DSI_PHY_LCCON);

	if (enter && !dsi_clk_hs_state(dsi)) {
		tmp_reg1 = tmp_reg1 | LC_HS_TX_EN;
		dsi_write(tmp_reg1, dsi->dsi_reg_base, DSI_PHY_LCCON);
	} else if (!enter && dsi_clk_hs_state(dsi)) {
		tmp_reg1 = tmp_reg1 & (~LC_HS_TX_EN);
		dsi_write(tmp_reg1, dsi->dsi_reg_base, DSI_PHY_LCCON);
	}
}

static void  dsi_set_mode(struct mtk_dsi *dsi)
{
	u32 tmp_reg1;

	tmp_reg1 = 0;

	if (dsi->mode_flags & MIPI_DSI_MODE_VIDEO) {

		tmp_reg1 = SYNC_PULSE_MODE;

		if (dsi->mode_flags & MIPI_DSI_MODE_VIDEO_BURST)
			tmp_reg1 = BURST_MODE;

		if (dsi->mode_flags & MIPI_DSI_MODE_VIDEO_SYNC_PULSE)
			tmp_reg1 = SYNC_PULSE_MODE;

	}

	dsi_write(tmp_reg1, dsi->dsi_reg_base, DSI_MODE_CTRL);
}

static void dsi_ps_control(struct mtk_dsi *dsi)
{
	struct videomode *vm = &dsi->vm;
	u32 dsi_buffer_bpp, ps_wc;
	u32 tmp_reg;
	u32 tmp_hstx_cklp_wc;

	tmp_reg = 0;

	if (dsi->format == MIPI_DSI_FMT_RGB565)
		dsi_buffer_bpp = 2;
	else
		dsi_buffer_bpp = 3;

	ps_wc = vm->hactive * dsi_buffer_bpp;

	tmp_reg = ps_wc;

	switch (dsi->format) {
	case MIPI_DSI_FMT_RGB888:
		tmp_reg |= PACKED_PS_24BIT_RGB888;
		break;
	case MIPI_DSI_FMT_RGB666:
		tmp_reg |= PACKED_PS_18BIT_RGB666;
		break;
	case MIPI_DSI_FMT_RGB666_PACKED:
		tmp_reg |= LOOSELY_PS_18BIT_RGB666;
		break;
	case MIPI_DSI_FMT_RGB565:
		tmp_reg |= PACKED_PS_16BIT_RGB565;
		break;
		}

	tmp_hstx_cklp_wc = ps_wc;

	dsi_write(vm->vactive, dsi->dsi_reg_base, DSI_VACT_NL);
	dsi_write(tmp_reg, dsi->dsi_reg_base, DSI_PSCTRL);
	dsi_write(tmp_hstx_cklp_wc, dsi->dsi_reg_base, DSI_HSTX_CKL_WC);
}

static void dsi_rxtx_control(struct mtk_dsi *dsi)
{
	u32 tmp_reg = 0;

	switch (dsi->lanes) {
	case 1:
		tmp_reg = 1 << 2;
		break;
	case 2:
		tmp_reg = 3 << 2;
		break;
	case 3:
		tmp_reg = 7 << 2;
		break;
	case 4:
		tmp_reg = 0xf << 2;
		break;
	default:
		tmp_reg = 0xf << 2;
		break;
	}

	dsi_write(tmp_reg, dsi->dsi_reg_base, DSI_TXRX_CTRL);
}

static void dsi_ps_control_for_vdo_timing(struct mtk_dsi *dsi)
{
	unsigned int dsi_buffer_bpp;
	u32 tmp_reg1 = 0;

	switch (dsi->format) {
	case MIPI_DSI_FMT_RGB888:
		tmp_reg1 = PACKED_PS_24BIT_RGB888;
		dsi_buffer_bpp = 3;
		break;
	case MIPI_DSI_FMT_RGB666:
		tmp_reg1 = LOOSELY_PS_18BIT_RGB666;
		dsi_buffer_bpp = 3;
		break;
	case MIPI_DSI_FMT_RGB666_PACKED:
		tmp_reg1 = PACKED_PS_18BIT_RGB666;
		dsi_buffer_bpp = 3;
		break;
	case MIPI_DSI_FMT_RGB565:
		tmp_reg1 = PACKED_PS_16BIT_RGB565;
		dsi_buffer_bpp = 2;
		break;
	default:
		tmp_reg1 = PACKED_PS_24BIT_RGB888;
		dsi_buffer_bpp = 3;
		break;
	}

	tmp_reg1 = tmp_reg1 + ((dsi->vm.hactive * dsi_buffer_bpp) & DSI_PS_WC);

	dsi_write(tmp_reg1, dsi->dsi_reg_base, DSI_PSCTRL);
}


static void dsi_config_vdo_timing(struct mtk_dsi *dsi)
{
	unsigned int horizontal_sync_active_byte;
	unsigned int horizontal_backporch_byte;
	unsigned int horizontal_frontporch_byte;
	unsigned int dsiTmpBufBpp;

	struct videomode *vm = &dsi->vm;

	if (dsi->format == MIPI_DSI_FMT_RGB565)
		dsiTmpBufBpp = 2;
	else
		dsiTmpBufBpp = 3;

	dsi_write(vm->vsync_len, dsi->dsi_reg_base, DSI_VSA_NL);
	dsi_write(vm->vback_porch, dsi->dsi_reg_base, DSI_VBP_NL);
	dsi_write(vm->vfront_porch, dsi->dsi_reg_base, DSI_VFP_NL);
	dsi_write(vm->vactive, dsi->dsi_reg_base, DSI_VACT_NL);

	if (dsi->mode_flags & MIPI_DSI_MODE_VIDEO_SYNC_PULSE) {
		horizontal_sync_active_byte = (vm->hsync_len * dsiTmpBufBpp -
			10);
		horizontal_backporch_byte = (vm->hback_porch * dsiTmpBufBpp -
			10);
	} else {
		horizontal_sync_active_byte = (vm->hsync_len * dsiTmpBufBpp -
			10);
		horizontal_backporch_byte = ((vm->hback_porch + vm->hsync_len) *
			dsiTmpBufBpp - 10);
	}

	horizontal_frontporch_byte = (vm->hfront_porch * dsiTmpBufBpp - 12);

	dsi_write(horizontal_sync_active_byte, dsi->dsi_reg_base, DSI_HSA_WC);
	dsi_write(horizontal_backporch_byte, dsi->dsi_reg_base, DSI_HBP_WC);
	dsi_write(horizontal_frontporch_byte, dsi->dsi_reg_base, DSI_HFP_WC);

	dsi_ps_control_for_vdo_timing(dsi);

}


static void mtk_dsi_start(struct mtk_dsi *dsi)
{
	dsi_write(0, dsi->dsi_reg_base, DSI_START);
	dsi_write(1, dsi->dsi_reg_base, DSI_START);
}

static void mtk_dsi_poweroff(struct mtk_dsi *dsi)
{
	dsi_phy_clk_switch(dsi, 0);
}


int mtk_output_dsi_enable(void)
{
	int ret;
	struct mtk_dsi *dsi = &_mtk_dsi;

	if (dsi->enabled == 1)
		return 0;

	ret = mtk_dsi_poweron(dsi);
	if (ret < 0)
		return ret;

	dsi_rxtx_control(dsi);

	dsi_clk_ulp_mode(dsi, 0);
	dsi_lane0_ulp_mode(dsi, 0);
	dsi_clk_hs_mode(dsi, 0);
	dsi_set_mode(dsi);

	dsi_ps_control(dsi);
	dsi_config_vdo_timing(dsi);

	dsi_set_mode(dsi);
	dsi_clk_hs_mode(dsi, 1);

	mtk_dsi_start(dsi);

	dsi->enabled = 1;

	return 0;
}

int mtk_output_dsi_disable(void)
{
	struct mtk_dsi *dsi = &_mtk_dsi;

	return 0;
	if (dsi->enabled == 0)
		return 0;

	dsi_lane0_ulp_mode(dsi, 1);
	dsi_clk_ulp_mode(dsi, 1);
	mtk_dsi_poweroff(dsi);
	dsi_phy_clk_switch(dsi, 0);

	dsi->enabled = 0;
	return 0;
}

int mtk_dsi_int(struct videomode *mode)
{
	struct mtk_dsi *dsi = &_mtk_dsi;

	dsi->vm.pixelclock = mode->pixelclock,
	dsi->vm.hactive    = mode->hactive;
	dsi->vm.hback_porch  = mode->hback_porch;
	dsi->vm.hfront_porch = mode->hfront_porch;
	dsi->vm.hsync_len    = mode->hsync_len;
	dsi->vm.vactive      = mode->vactive;
	dsi->vm.vback_porch  = mode->vback_porch;
	dsi->vm.vfront_porch = mode->vfront_porch;
	dsi->vm.vsync_len = mode->vsync_len;

	MTKFBDPF("mtk_dsi_int : ps8640_init\n");
	ps8640_init();

	mdelay(2500);

	MTKFBDPF("mtk_dsi_int : mtk_output_dsi_enable\n");
	mtk_output_dsi_enable();

	return 0;
}
