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

#ifndef _DSI_REG_H_
#define _DSI_REG_H_

#define BIT(nr)                 (1UL << (nr))

/* video mode */
enum {
	MIPI_DSI_MODE_VIDEO = BIT(0),
	/* video burst mode */
	MIPI_DSI_MODE_VIDEO_BURST = BIT(1),
	/* video pulse mode */
	MIPI_DSI_MODE_VIDEO_SYNC_PULSE = BIT(2),
	/* enable auto vertical count mode */
	MIPI_DSI_MODE_VIDEO_AUTO_VERT = BIT(3),
	/* enable hsync-end packets in vsync-pulse and v-porch area */
	MIPI_DSI_MODE_VIDEO_HSE = BIT(4),
	/* disable hfront-porch area */
	MIPI_DSI_MODE_VIDEO_HFP = BIT(5),
	/* disable hback-porch area */
	MIPI_DSI_MODE_VIDEO_HBP = BIT(6),
	/* disable hsync-active area */
	MIPI_DSI_MODE_VIDEO_HSA = BIT(7),
	/* flush display FIFO on vsync pulse */
	MIPI_DSI_MODE_VSYNC_FLUSH = BIT(8),
	/* disable EoT packets in HS mode */
	MIPI_DSI_MODE_EOT_PACKET = BIT(9),
	/* device supports non-continuous clock behavior (DSI spec 5.6.1) */
	MIPI_DSI_CLOCK_NON_CONTINUOUS = BIT(10),
	/* transmit data in low power */
	MIPI_DSI_MODE_LPM = BIT(11)
};

enum mipi_dsi_pixel_format {
	MIPI_DSI_FMT_RGB888,
	MIPI_DSI_FMT_RGB666,
	MIPI_DSI_FMT_RGB666_PACKED,
	MIPI_DSI_FMT_RGB565
};


/* video/viedomode.h */
struct videomode {
	unsigned long pixelclock;
	u32 hactive;
	u32 hfront_porch;
	u32 hback_porch;
	u32 hsync_len;
	u32 vactive;
	u32 vfront_porch;
	u32 vback_porch;
	u32 vsync_len;
	u32 flags;
};

enum mtk_dsi_format {
	MTK_DSI_FORMAT_16P,
	MTK_DSI_FORMAT_18NP,
	MTK_DSI_FORMAT_18P,
	MTK_DSI_FORMAT_24P
};

enum {
	DSI_START	= 0x00,
	DSI_INTEN	= 0x08,
	DSI_INTSTA	= 0x0c,
	DSI_CON_CTRL	= 0x10,
	DSI_MODE_CTRL   = 0x14,
	DSI_TXRX_CTRL	= 0x18,
	DSI_PSCTRL	= 0x1c,
	DSI_VSA_NL   = 0x20,
	DSI_VBP_NL   = 0x24,
	DSI_VFP_NL   = 0x28,
	DSI_VACT_NL  = 0x2C,
	DSI_HSA_WC   = 0x50,
	DSI_HBP_WC   = 0x54,
	DSI_HFP_WC   = 0x58,
	DSI_BLLP_WC  = 0x5c,
	DSI_PHY_LCCON = 0x104,
	DSI_PHY_LD0CON = 0x108,
	DSI_PHY_TIMECON0 = 0x110,
	DSI_PHY_TIMECON1 = 0x114,
	DSI_PHY_TIMECON2 = 0x118,
	DSI_PHY_TIMECON3 = 0x11c
};

/* DSI_INTSTA */
enum {
	RD_RDY		= (1),
	CMD_DONE	= (1 << 1),
	TE_RDY		= (1 << 2),
	VM_DONE		= (1 << 3),
	EXT_TE		= (1 << 4),
	VM_CMD_DONE	= (1 << 5),
	BUSY		= (1 << 31)
};

/* DSI_CON_CTRL */
enum {
	DSI_RESET	= (1),
};

/* DSI_MODE_CTRL */
enum {
	MODE = 2,
	CMD_MODE = 0,
	SYNC_PULSE_MODE = 1,
	SYNC_EVENT_MODE = 2,
	BURST_MODE = 3,
	FRM_MODE = (1 << 16),
	MIX_MODE = (1 << 17)
};

/* DSI_TXRX_CTRL */
enum {
	VC_NUM	= (2 << 0),
	LANE_NUM = (0xf << 2),
	DIS_EOT	 = (1 << 6),
	NULL_EN	 = (1 << 7),
	TE_FREERUN = (1 << 8),
	EXT_TE_EN  = (1 << 9),
	EXT_TE_EDGE = (1 << 10),
	MAX_RTN_SIZE = (0xf << 12),
	HSTX_CKLP_EN = (1 << 16)
};

/* DSI_PSCTRL */
enum {
	DSI_PS_WC = 0x3fff,
	DSI_PS_SEL = (2 << 16),
	PACKED_PS_16BIT_RGB565	= (0 << 16),
	LOOSELY_PS_18BIT_RGB666	= (1 << 16),
	PACKED_PS_18BIT_RGB666	= (2 << 16),
	PACKED_PS_24BIT_RGB888	= (3 << 16)
};

/* DSI_BLLP_WC */
enum {
	DSI_HSTX_CKL_WC	= 0x64
};

/* DSI_PHY_LCCON */
enum {
	LC_HS_TX_EN = (1),
	LC_ULPM_EN  = (1 << 1),
	LC_WAKEUP_EN = (1 << 2)
};

/*DSI_PHY_LD0CON */
enum {
	LD0_HS_TX_EN = (1),
	LD0_ULPM_EN = (1 << 1),
	LD0_WAKEUP_EN = (1 << 2)
};

enum {
	LPX	= (0xff << 0),
	HS_PRPR	= (0xff << 8),
	HS_ZERO	= (0xff << 16),
	HS_TRAIL = (0xff << 24)
};

enum {
	TA_GO	 = (0xff << 0),
	TA_SURE	 = (0xff << 8),
	TA_GET	 = (0xff << 16),
	DA_HS_EXIT = (0xff << 24)
};

enum {
	CONT_DET = (0xff << 0),
	CLK_ZERO = (0xf << 16),
	CLK_TRAIL = (0xff << 24)
};

enum {
	CLK_HS_PRPR = (0xff << 0),
	CLK_HS_POST = (0xff << 8),
	CLK_HS_EXIT = (0xf << 16)
};


/* MIPITX_REG */
enum {
	MIPITX_DSI0_CON		= 0x00,
	MIPITX_DSI0_CLOCK_LANE	= 0x04,
	MIPITX_DSI0_DATA_LANE0	= 0x08,
	MIPITX_DSI0_DATA_LANE1	= 0x0c,
	MIPITX_DSI0_DATA_LANE2	= 0x10,
	MIPITX_DSI0_DATA_LANE3	= 0x14,
	MIPITX_DSI_TOP_CON	= 0x40,
	MIPITX_DSI_BG_CON	= 0x44,
	MIPITX_DSI_PLL_CON0	= 0x50,
	MIPITX_DSI_PLL_CON1	= 0x54,
	MIPITX_DSI_PLL_CON2	= 0x58,
	MIPITX_DSI_PLL_PWR	= 0x68,
	MIPITX_DSI_SW_CTRL	= 0x80,
	MIPITX_DSI_SW_CTRL_CON0	= 0x84
};

/* MIPITX_DSI0_CON */
enum {
	RG_DSI0_LDOCORE_EN	= (1),
	RG_DSI0_CKG_LDOOUT_EN	= (1 << 1),
	RG_DSI0_BCLK_SEL	= (3 << 2),
	RG_DSI0_LD_IDX_SEL	= (7 << 4),
	RG_DSI0_PHYCLK_SEL	= (2 << 8),
	RG_DSI0_DSICLK_FREQ_SEL	= (1 << 10),
	RG_DSI0_LPTX_CLMP_EN	= (1 << 11)
};

/* MIPITX_DSI0_CLOCK_LANE */
enum {
	RG_DSI0_LNTC_LDOOUT_EN	 = (1),
	RG_DSI0_LNTC_CKLANE_EN	 = (1 << 1),
	RG_DSI0_LNTC_LPTX_IPLUS1 = (1 << 2),
	RG_DSI0_LNTC_LPTX_IPLUS2 = (1 << 3),
	RG_DSI0_LNTC_LPTX_IMINUS = (1 << 4),
	RG_DSI0_LNTC_LPCD_IPLUS	 = (1 << 5),
	RG_DSI0_LNTC_LPCD_IMLUS	 = (1 << 6),
	RG_DSI0_LNTC_RT_CODE	 = (0xf << 8)
};

/* MIPITX_DSI0_DATA_LANE0 */
enum {
	RG_DSI0_LNT0_LDOOUT_EN	 = (1),
	RG_DSI0_LNT0_CKLANE_EN	 = (1 << 1),
	RG_DSI0_LNT0_LPTX_IPLUS1 = (1 << 2),
	RG_DSI0_LNT0_LPTX_IPLUS2 = (1 << 3),
	RG_DSI0_LNT0_LPTX_IMINUS = (1 << 4),
	RG_DSI0_LNT0_LPCD_IPLUS	 = (1 << 5),
	RG_DSI0_LNT0_LPCD_IMINUS = (1 << 6),
	RG_DSI0_LNT0_RT_CODE	 = (0xf << 8)
};


/* MIPITX_DSI0_DATA_LANE1 */
enum {
	RG_DSI0_LNT1_LDOOUT_EN	 = (1),
	RG_DSI0_LNT1_CKLANE_EN	 = (1 << 1),
	RG_DSI0_LNT1_LPTX_IPLUS1 = (1 << 2),
	RG_DSI0_LNT1_LPTX_IPLUS2 = (1 << 3),
	RG_DSI0_LNT1_LPTX_IMINUS = (1 << 4),
	RG_DSI0_LNT1_LPCD_IPLUS	 = (1 << 5),
	RG_DSI0_LNT1_LPCD_IMINUS = (1 << 6),
	RG_DSI0_LNT1_RT_CODE	 = (0xf << 8)
};

/* MIPITX_DSI0_DATA_LANE2 */
enum {
	RG_DSI0_LNT2_LDOOUT_EN	 = (1),
	RG_DSI0_LNT2_CKLANE_EN	 = (1 << 1),
	RG_DSI0_LNT2_LPTX_IPLUS1 = (1 << 2),
	RG_DSI0_LNT2_LPTX_IPLUS2 = (1 << 3),
	RG_DSI0_LNT2_LPTX_IMINUS = (1 << 4),
	RG_DSI0_LNT2_LPCD_IPLUS	 = (1 << 5),
	RG_DSI0_LNT2_LPCD_IMINUS = (1 << 6),
	RG_DSI0_LNT2_RT_CODE	 = (0xf << 8)
};

/* MIPITX_DSI0_DATA_LANE3 */
enum {
	RG_DSI0_LNT3_LDOOUT_EN	 = (1),
	RG_DSI0_LNT3_CKLANE_EN	 = (1 << 1),
	RG_DSI0_LNT3_LPTX_IPLUS1 = (1 << 2),
	RG_DSI0_LNT3_LPTX_IPLUS2 = (1 << 3),
	RG_DSI0_LNT3_LPTX_IMINUS = (1 << 4),
	RG_DSI0_LNT3_LPCD_IPLUS	 = (1 << 5),
	RG_DSI0_LNT3_LPCD_IMINUS = (1 << 6),
	RG_DSI0_LNT3_RT_CODE	 = (0xf << 8)
};

/* MIPITX_DSI_TOP_CON */
enum {
	RG_DSI_LNT_INTR_EN	= (1),
	RG_DSI_LNT_HS_BIAS_EN	= (1 << 1),
	RG_DSI_LNT_IMP_CAL_EN	= (1 << 2),
	RG_DSI_LNT_TESTMODE_EN	= (1 << 3),
	RG_DSI_LNT_IMP_CAL_CODE	= (0xf << 4),
	RG_DSI_LNT_AIO_SEL	= (7 << 8),
	RG_DSI_PAD_TIE_LOW_EN	= (1 << 11),
	RG_DSI_DEBUG_INPUT_EN	= (1 << 12),
	RG_DSI_PRESERVE		= (7 << 13)
};

/* MIPITX_DSI_BG_CON */
enum {
	RG_DSI_BG_CORE_EN	= 1,
	RG_DSI_BG_CKEN		= (1 << 1),
	RG_DSI_BG_DIV		= (0x3 << 2),
	RG_DSI_BG_FAST_CHARGE	= (1 << 4),
	RG_DSI_V12_SEL		= (7 << 5),
	RG_DSI_V10_SEL		= (7 << 8),
	RG_DSI_V072_SEL		= (7 << 11),
	RG_DSI_V04_SEL		= (7 << 14),
	RG_DSI_V032_SEL		= (7 << 17),
	RG_DSI_V02_SEL		= (7 << 20),
	rsv_23			= (1 << 23),
	RG_DSI_BG_R1_TRIM	= (0xf << 24),
	RG_DSI_BG_R2_TRIM	= (0xf << 28)
};

/* MIPITX_DSI_PLL_CON0 */
enum {
	RG_DSI0_MPPLL_PLL_EN	= (1 << 0),
	RG_DSI0_MPPLL_PREDIV	= (3 << 1),
	RG_DSI0_MPPLL_TXDIV0	= (3 << 3),
	RG_DSI0_MPPLL_TXDIV1	= (3 << 5),
	RG_DSI0_MPPLL_POSDIV	= (7 << 7),
	RG_DSI0_MPPLL_MONVC_EN	= (1 << 10),
	RG_DSI0_MPPLL_MONREF_EN	= (1 << 11),
	RG_DSI0_MPPLL_VOD_EN	= (1 << 12)
};

/* MIPITX_DSI_PLL_CON1 */
enum {
	RG_DSI0_MPPLL_SDM_FRA_EN	= (1),
	RG_DSI0_MPPLL_SDM_SSC_PH_INIT	= (1 << 1),
	RG_DSI0_MPPLL_SDM_SSC_EN	= (1 << 2),
	RG_DSI0_MPPLL_SDM_SSC_PRD	= (0xffff << 16)
};

/* MIPITX_DSI_PLL_TOP */
enum {
	RG_MPPLL_TST_EN	= (1),
	RG_MPPLL_TSTCK_EN = (1 << 1),
	RG_MPPLL_TSTSEL	= (3 << 2),
	RG_MPPLL_PRESERVE = (0xff << 16)
};

/* MIPITX_DSI_PLL_PWR */
enum {
	RG_DSI_MPPLL_SDM_PWR_ON	 = (1 << 0),
	RG_DSI_MPPLL_SDM_ISO_EN	 = (1 << 1),
	RG_DSI_MPPLL_SDM_PWR_ACK = (1 << 8)
};

/* MIPITX_DSI_SW_CTRL */
enum {
	SW_CTRL_EN = (1<<0)
};

/* MIPITX_DSI_SW_CTRL_CON0 */
enum {
	SW_LNTC_LPTX_PRE_OE	= (1<<0),
	SW_LNTC_LPTX_OE		= (1<<1),
	SW_LNTC_LPTX_P		= (1<<2),
	SW_LNTC_LPTX_N		= (1<<3),
	SW_LNTC_HSTX_PRE_OE	= (1<<4),
	SW_LNTC_HSTX_OE		= (1<<5),
	SW_LNTC_HSTX_ZEROCLK	= (1<<6),
	SW_LNT0_LPTX_PRE_OE	= (1<<7),
	SW_LNT0_LPTX_OE		= (1<<8),
	SW_LNT0_LPTX_P		= (1<<9),
	SW_LNT0_LPTX_N		= (1<<10),
	SW_LNT0_HSTX_PRE_OE	= (1<<11),
	SW_LNT0_HSTX_OE		= (1<<12),
	SW_LNT0_LPRX_EN		= (1<<13),
	SW_LNT1_LPTX_PRE_OE	= (1<<14),
	SW_LNT1_LPTX_OE		= (1<<15),
	SW_LNT1_LPTX_P		= (1<<16),
	SW_LNT1_LPTX_N		= (1<<17),
	SW_LNT1_HSTX_PRE_OE	= (1<<18),
	SW_LNT1_HSTX_OE		= (1<<19),
	SW_LNT2_LPTX_PRE_OE	= (1<<20),
	SW_LNT2_LPTX_OE		= (1<<21),
	SW_LNT2_LPTX_P		= (1<<22),
	SW_LNT2_LPTX_N		= (1<<23),
	SW_LNT2_HSTX_PRE_OE	= (1<<24),
	SW_LNT2_HSTX_OE		= (1<<25)
};

int mtk_output_dsi_enable(void);
int mtk_output_dsi_disable(void);
int mtk_dsi_int(struct videomode *mode);

#endif
