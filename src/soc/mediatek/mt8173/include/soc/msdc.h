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

#ifndef SOC_MEDIATEK_MT8173_MSDC_H
#define SOC_MEDIATEK_MT8173_MSDC_H

#include "msdc_cfg.h"
#include "mmc_core.h"

enum {
	CMD_RETRIES = 5,
	CMD_TIMEOUT = 100,	/* 100ms */

	PERI_MSDC_SRCSEL = 0xc100000c
};

/* Common Definition */
enum {
	MSDC_FIFO_SZ = 128,
	MSDC_FIFO_THD = 128,
	MSDC_MAX_NUM = 1
};

enum {
	MSDC_MS = 0,
	MSDC_SDMMC = 1
};

enum {
	MSDC_MODE_UNKNOWN = 0,
	MSDC_MODE_PIO = 1,
	MSDC_MODE_DMA_BASIC = 2,
	MSDC_MODE_DMA_DESC = 3,
	MSDC_MODE_DMA_ENHANCED = 4,
	MSDC_MODE_MMC_STREAM = 5
};

enum {
	MSDC_BUS_1BITS = 0,
	MSDC_BUS_4BITS = 1,
	MSDC_BUS_8BITS = 2,

	MSDC_BRUST_8B = 3,
	MSDC_BRUST_16B = 4,
	MSDC_BRUST_32B = 5,
	MSDC_BRUST_64B = 6
};

enum {
	MSDC_PIN_PULL_NONE = 0,
	MSDC_PIN_PULL_DOWN = 1,
	MSDC_PIN_PULL_UP = 2,
	MSDC_PIN_KEEP = 3
};

enum {
	MSDC_MAX_SCLK = 100000000,
	MSDC_MIN_SCLK = 260000
};

enum {
	MSDC_AUTOCMD12 = 0x0001,
	MSDC_AUTOCMD23 = 0x0002,
	MSDC_AUTOCMD19 = 0x0003
};

enum {
	MAX_GPD_POOL_SZ = 2, /* include null gpd */
	MAX_BD_POOL_SZ = 4
};

/* Register Offset */
enum {
	OFFSET_MSDC_CFG = 0x0,
	OFFSET_MSDC_IOCON = 0x04,
	OFFSET_MSDC_PS = 0x08,
	OFFSET_MSDC_INT = 0x0c,
	OFFSET_MSDC_INTEN = 0x10,
	OFFSET_MSDC_FIFOCS = 0x14,
	OFFSET_MSDC_TXDATA = 0x18,
	OFFSET_MSDC_RXDATA = 0x1c,
	OFFSET_SDC_CFG = 0x30,
	OFFSET_SDC_CMD = 0x34,
	OFFSET_SDC_ARG = 0x38,
	OFFSET_SDC_STS = 0x3c,
	OFFSET_SDC_RESP0 = 0x40,
	OFFSET_SDC_RESP1 = 0x44,
	OFFSET_SDC_RESP2 = 0x48,
	OFFSET_SDC_RESP3 = 0x4c,
	OFFSET_SDC_BLK_NUM = 0x50,
	OFFSET_SDC_CSTS = 0x58,
	OFFSET_SDC_CSTS_EN = 0x5c,
	OFFSET_SDC_DCRC_STS = 0x60,
	OFFSET_EMMC_CFG0 = 0x70,
	OFFSET_EMMC_CFG1 = 0x74,
	OFFSET_EMMC_STS = 0x78,
	OFFSET_EMMC_IOCON = 0x7c,
	OFFSET_SDC_ACMD_RESP = 0x80,
	OFFSET_SDC_ACMD19_TRG = 0x84,
	OFFSET_SDC_ACMD19_STS = 0x88,
	OFFSET_MSDC_DMA_SA = 0x90,
	OFFSET_MSDC_DMA_CA = 0x94,
	OFFSET_MSDC_DMA_CTRL = 0x98,
	OFFSET_MSDC_DMA_CFG = 0x9c,
	OFFSET_MSDC_DBG_SEL = 0xa0,
	OFFSET_MSDC_DBG_OUT = 0xa4,
	OFFSET_MSDC_DMA_LEN = 0xa8,
	OFFSET_MSDC_PATCH_BIT0 = 0xb0,
	OFFSET_MSDC_PATCH_BIT1 = 0xb4,
	OFFSET_DAT0_TUNE_CRC = 0xc0,
	OFFSET_DAT1_TUNE_CRC = 0xc4,
	OFFSET_DAT2_TUNE_CRC = 0xc8,
	OFFSET_DAT3_TUNE_CRC = 0xcc,
	OFFSET_CMD_TUNE_CRC = 0xd0,
	OFFSET_SDIO_TUNE_WIND = 0xd4,
	OFFSET_MSDC_PAD_TUNE = 0xec,
	OFFSET_MSDC_DAT_RDDLY0 = 0xf0,
	OFFSET_MSDC_DAT_RDDLY1 = 0xf4,
	OFFSET_MSDC_HW_DBG = 0xf8,
	OFFSET_MSDC_VERSION = 0x100,
	OFFSET_MSDC_ECO_VER = 0x104,
	OFFSET_EMMC50_PAD_CTL0 = 0x180,
	OFFSET_EMMC50_PAD_DS_CTL0 = 0x184,
	OFFSET_EMMC50_PAD_DS_TUNE = 0x188,
	OFFSET_EMMC50_PAD_CMD_TUNE = 0x18c,
	OFFSET_EMMC50_PAD_DAT01_TUNE = 0x190,
	OFFSET_EMMC50_PAD_DAT23_TUNE = 0x194,
	OFFSET_EMMC50_PAD_DAT45_TUNE = 0x198,
	OFFSET_EMMC50_PAD_DAT67_TUNE = 0x19c,
	OFFSET_EMMC50_CFG0 = 0x208,
	OFFSET_EMMC50_CFG1 = 0x20c,
	OFFSET_EMMC50_CFG2 = 0x21c,
	OFFSET_EMMC50_CFG3  = 0x220,
	OFFSET_EMMC50_CFG4 = 0x224
};

/* Register Mask */

/* MSDC_CFG mask */
enum {
	MSDC_CFG_MODE = 0x1 << 0,	/* RW */
	MSDC_CFG_CKPDN = 0x1 << 1,	/* RW */
	MSDC_CFG_RST = 0x1 << 2,	/* A0 */
	MSDC_CFG_PIO = 0x1 << 3,	/* RW */
	MSDC_CFG_CKDRVEN = 0x1 << 4,	/* RW */
	MSDC_CFG_BV18SDT = 0x1 << 5,	/* RW */
	MSDC_CFG_BV18PSS = 0x1 << 6,	/* R  */
	MSDC_CFG_CKSTB = 0x1 << 7,	/* R  */
	MSDC_CFG_CKDIV = 0xff << 8,	/* RW */
	MSDC_CFG_CKMOD = 0x3 << 16,	/* W1C */
	MSDC_CFG_CKMOD_HS400 = 0x1 << 18,	/* RW */
	MSDC_CFG_START_BIT = 0x3 << 19,	/* RW */
	MSDC_CFG_SCLK_STOP_DDR = 0x1 << 21	/* RW */
};

/* MSDC_IOCON mask */
enum {
	MSDC_IOCON_SDR104CKS = 0x1 << 0,		/* RW */
	MSDC_IOCON_RSPL = 0x1 << 1,	/* RW */
	MSDC_IOCON_R_D_SMPL = 0x1 << 2,	/* RW */
	MSDC_IOCON_DDLSEL = 0x1 << 3,	/* RW */
	MSDC_IOCON_DDR50CKD = 0x1 << 4,	/* RW */
	MSDC_IOCON_R_D_SMPL_SEL = 0x1 << 5,	/* RW */
	MSDC_IOCON_W_D_SMPL = 0x1 << 8,	/* RW */
	MSDC_IOCON_W_D_SMPL_SEL = 0x1 << 9,	/* RW */
	MSDC_IOCON_W_D0SPL = 0x1 << 10,	/* RW */
	MSDC_IOCON_W_D1SPL = 0x1 << 11,	/* RW */
	MSDC_IOCON_W_D2SPL = 0x1 << 12,	/* RW */
	MSDC_IOCON_W_D3SPL = 0x1 << 13,	/* RW */

	MSDC_IOCON_R_D0SPL = 0x1 << 16,	/* RW */
	MSDC_IOCON_R_D1SPL = 0x1 << 17,	/* RW */
	MSDC_IOCON_R_D2SPL = 0x1 << 18,	/* RW */
	MSDC_IOCON_R_D3SPL = 0x1 << 19,	/* RW */
	MSDC_IOCON_R_D4SPL = 0x1 << 20,	/* RW */
	MSDC_IOCON_R_D5SPL = 0x1 << 21,	/* RW */
	MSDC_IOCON_R_D6SPL = 0x1 << 22,	/* RW */
	MSDC_IOCON_R_D7SPL = 0x1 << 23,	/* RW */
	MSDC_IOCON_RISCSZ = 0x3 << 24	/* RW */
};

/* MSDC_PS mask */
enum {
	MSDC_PS_CDEN = 0x1 << 0,	/* RW */
	MSDC_PS_CDSTS = 0x1 << 1,	/* R  */
	MSDC_PS_CDDEBOUNCE = 0xf << 12,	/* RW */
	MSDC_PS_DAT = 0xff << 16,	/* R  */
	MSDC_PS_CMD = 0x1 << 24,	/* R  */
	MSDC_PS_WP = 0x1UL << 31	/* R  */
};

/* MSDC_INT mask */
enum {
	MSDC_INT_MMCIRQ = 0x1 << 0,	/* W1C */
	MSDC_INT_CDSC = 0x1 << 1,	/* W1C */
	MSDC_INT_ACMDRDY = 0x1 << 3,	/* W1C */
	MSDC_INT_ACMDTMO = 0x1 << 4,	/* W1C */
	MSDC_INT_ACMDCRCERR = 0x1 << 5,	/* W1C */
	MSDC_INT_DMAQ_EMPTY = 0x1 << 6,	/* W1C */
	MSDC_INT_SDIOIRQ = 0x1 << 7,	/* W1C */
	MSDC_INT_CMDRDY = 0x1 << 8,	/* W1C */
	MSDC_INT_CMDTMO = 0x1 << 9,	/* W1C */
	MSDC_INT_RSPCRCERR = 0x1 << 10,	/* W1C */
	MSDC_INT_CSTA = 0x1 << 11,	/* R */
	MSDC_INT_XFER_COMPL = 0x1 << 12,	/* W1C */
	MSDC_INT_DXFER_DONE = 0x1 << 13,	/* W1C */
	MSDC_INT_DATTMO = 0x1 << 14,	/* W1C */
	MSDC_INT_DATCRCERR = 0x1 << 15,	/* W1C */
	MSDC_INT_ACMD19_DONE = 0x1 << 16,	/* W1C */
	MSDC_INT_BDCSERR = 0x1 << 17,	/* W1C */
	MSDC_INT_GPDCSERR = 0x1 << 18,	/* W1C */
	MSDC_INT_DMAPRO = 0x1 << 19,	/* W1C */
	MSDC_INT_AXI_RESP_ERR = 0x1 << 23,	/* W1C */
#ifdef MTK_SDIO30_TEST_MODE_SUPPORT
	MSDC_INT_GRAE_OUT_BOUND = 0x1 << 20,	/* W1C */
	MSDC_INT_ACMD53_DONE = 0x1 << 21,	/* W1C */
	MSDC_INT_ACMD53_FAIL = 0x1 << 22	/* W1C */
#endif
};

/* MSDC_INTEN mask */
enum {
	MSDC_INTEN_MMCIRQ = 0x1 << 0,	/* RW */
	MSDC_INTEN_CDSC = 0x1 << 1,	/* RW */
	MSDC_INTEN_ACMDRDY = 0x1 << 3,	/* RW */
	MSDC_INTEN_ACMDTMO = 0x1 << 4,	/* RW */
	MSDC_INTEN_ACMDCRCERR = 0x1 << 5,	/* RW */
	MSDC_INTEN_DMAQ_EMPTY = 0x1 << 6,	/* RW */
	MSDC_INTEN_SDIOIRQ = 0x1 << 7,	/* RW */
	MSDC_INTEN_CMDRDY = 0x1 << 8,	/* RW */
	MSDC_INTEN_CMDTMO = 0x1 << 9,	/* RW */
	MSDC_INTEN_RSPCRCERR = 0x1 << 10,	/* RW */
	MSDC_INTEN_CSTA = 0x1 << 11,	/* RW */
	MSDC_INTEN_XFER_COMPL = 0x1 << 12,	/* RW */
	MSDC_INTEN_DXFER_DONE = 0x1 << 13,	/* RW */
	MSDC_INTEN_DATTMO = 0x1 << 14,	/* RW */
	MSDC_INTEN_DATCRCERR = 0x1 << 15,	/* RW */
	MSDC_INTEN_ACMD19_DONE = 0x1 << 16,	/* RW */
	MSDC_INTEN_BDCSERR = 0x1 << 17,	/* RW */
	MSDC_INTEN_GPDCSERR = 0x1 << 18,	/* RW */
	MSDC_INTEN_DMAPRO = 0x1 << 19,	/* RW */
	MSDC_INTEN_GOBOUND = 0x1 << 20,	/* RW */
	MSDC_INTEN_ACMD53_DONE = 0x1 << 21,    /* RW */
	MSDC_INTEN_ACMD53_FAIL = 0x1 << 22,    /* RW */
	MSDC_INTEN_AXI_RESP_ERR = 0x1 << 23    /* RW */
};

/* MSDC_FIFOCS mask */
enum {
	MSDC_FIFOCS_RXCNT = 0xff << 0,	/* R */
	MSDC_FIFOCS_TXCNT = 0xff << 16,	/* R */
	MSDC_FIFOCS_CLR = 0x1UL << 31	/* RW */
};

/* SDC_CFG mask */
enum {
	SDC_CFG_SDIOINTWKUP = 0x1 << 0,	/* RW */
	SDC_CFG_INSWKUP = 0x1 << 1,	/* RW */
	SDC_CFG_BUSWIDTH = 0x3 << 16,	/* RW */
	SDC_CFG_SDIO = 0x1 << 19,	/* RW */
	SDC_CFG_SDIOIDE = 0x1 << 20,	/* RW */
	SDC_CFG_INTATGAP = 0x1 << 21,	/* RW */
	SDC_CFG_DTOC = 0xffUL << 24	/* RW */
};

/* SDC_CMD mask */
enum {
	SDC_CMD_OPC = 0x3f << 0,	/* RW */
	SDC_CMD_BRK = 0x1 << 6,	/* RW */
	SDC_CMD_RSPTYP = 0x7 << 7,	/* RW */
	SDC_CMD_DTYP = 0x3 << 11,	/* RW */
	SDC_CMD_RW = 0x1 << 13,	/* RW */
	SDC_CMD_STOP = 0x1 << 14,	/* RW */
	SDC_CMD_GOIRQ = 0x1 << 15,	/* RW */
	SDC_CMD_BLKLEN = 0xfff << 16,	/* RW */
	SDC_CMD_AUTOCMD = 0x3 << 28,	/* RW */
	SDC_CMD_VOLSWTH = 0x1 << 30,	/* RW */
	SDC_CMD_ACMD53 = 0x1 << 31    /* RW */
};

/* SDC_STS mask */
enum {
	SDC_STS_SDCBUSY = 0x1 << 0,	/* RW */
	SDC_STS_CMDBUSY = 0x1 << 1,	/* RW */
	SDC_STS_SWR_COMPL = 0x1 << 31    /* RW */
};

/* SDC_DCRC_STS mask */
enum {
	SDC_DCRC_STS_NEG = 0xff << 8,     /* RO */
	SDC_DCRC_STS_POS = 0xff << 0	/* RO */
};

/* EMMC_CFG0 mask */
enum {
	EMMC_CFG0_BOOTSTART = 0x1 << 0,	/* W */
	EMMC_CFG0_BOOTSTOP = 0x1 << 1,	/* W */
	EMMC_CFG0_BOOTMODE = 0x1 << 2,	/* RW */
	EMMC_CFG0_BOOTACKDIS = 0x1 << 3,	/* RW */
	EMMC_CFG0_BOOTWDLY = 0x7 << 12,	/* RW */
	EMMC_CFG0_BOOTSUPP = 0x1 << 15	/* RW */
};

/* EMMC_CFG1 mask */
enum {
	EMMC_CFG1_BOOTDATTMC = 0xfffff << 0,	/* RW */
	EMMC_CFG1_BOOTACKTMC = 0xfffUL << 20	/* RW */
};

/* EMMC_STS mask */
enum {
	EMMC_STS_BOOTCRCERR = 0x1 << 0,	/* W1C */
	EMMC_STS_BOOTACKERR = 0x1 << 1,	/* W1C */
	EMMC_STS_BOOTDATTMO = 0x1 << 2,	/* W1C */
	EMMC_STS_BOOTACKTMO = 0x1 << 3,	/* W1C */
	EMMC_STS_BOOTUPSTATE = 0x1 << 4,	/* R */
	EMMC_STS_BOOTACKRCV = 0x1 << 5,	/* W1C */
	EMMC_STS_BOOTDATRCV = 0x1 << 6	/* R */
};

/* EMMC_IOCON mask */
enum {
	EMMC_IOCON_BOOTRST = 0x1 << 0	/* RW */
};

/* SDC_ACMD19_TRG mask */
enum {
	SDC_ACMD19_TRG_TUNESEL = 0xf << 0	/* RW */
};

/* MSDC_DMA_CTRL mask */
enum {
	MSDC_DMA_CTRL_START = 0x1 << 0,	/* W */
	MSDC_DMA_CTRL_STOP = 0x1 << 1,	/* W */
	MSDC_DMA_CTRL_RESUME = 0x1 << 2,	/* W */
	MSDC_DMA_CTRL_MODE = 0x1 << 8,	/* RW */
	MSDC_DMA_CTRL_LASTBUF = 0x1 << 10,	/* RW */
	MSDC_DMA_CTRL_BRUSTSZ = 0x7 << 12	/* RW */
};

/* MSDC_DMA_CFG mask */
enum {
	MSDC_DMA_CFG_STS = 0x1 << 0,	/* R */
	MSDC_DMA_CFG_DECSEN = 0x1 << 1,	/* RW */
	MSDC_DMA_CFG_AHBEN = 0x3 << 8,     /* RW */
	MSDC_DMA_CFG_ACTEN = 0x3 << 12,    /* RW */
	MSDC_DMA_CFG_CS12B = 0x1 << 16,    /* RW */
	MSDC_DMA_CFG_OUTB_STOP = 0x1 << 17    /* RW */
};

/* MSDC_PATCH_BIT mask */
enum {
	MSDC_PATCH_BIT_ODDSUPP = 0x1 <<  1,	/* RW */

#ifdef MTK_SDIO30_TEST_MODE_SUPPORT
	MSDC_MASK_ACMD53_CRC_ERR_INTR = 0x1 << 4,
	MSDC_ACMD53_FAIL_ONE_SHOT = 0X1 << 5
#endif
};

/* MSDC_PAD_TUNE mask */
enum {
	MSDC_PAD_TUNE_DATWRDLY = 0x1F << 0,	/* RW */
	MSDC_PAD_TUNE_DATRRDLY = 0x1F << 8,	/* RW */
	MSDC_PAD_TUNE_CMDRDLY = 0x1F << 16,	/* RW */
	MSDC_PAD_TUNE_CMDRRDLY = 0x1FUL << 22,	/* RW */
	MSDC_PAD_TUNE_CLKTXDLY = 0x1FUL << 27	/* RW */
};

/* MSDC_DAT_RDDLY0/1 mask */
enum {
	MSDC_DAT_RDDLY0_D3 = 0x1F << 0,	/* RW */
	MSDC_DAT_RDDLY0_D2 = 0x1F << 8,	/* RW */
	MSDC_DAT_RDDLY0_D1 = 0x1F << 16,	/* RW */
	MSDC_DAT_RDDLY0_D0 = 0x1FUL<< 24,	/* RW */

	MSDC_DAT_RDDLY1_D7 = 0x1F << 0,	/* RW */
	MSDC_DAT_RDDLY1_D6 = 0x1F << 8,	/* RW */
	MSDC_DAT_RDDLY1_D5 = 0x1F << 16,	/* RW */
	MSDC_DAT_RDDLY1_D4 = 0x1FUL << 24	/* RW */
};

/* MSDC_PATCH_BIT0 mask */
enum {
	MSDC_PB0_RESV1 = 0x1 << 0,
	MSDC_PB0_EN_8BITSUP = 0x1 << 1,
	MSDC_PB0_DIS_RECMDWR = 0x1 << 2,
	MSDC_PB0_RESV2 = 0x1 << 3,
	MSDC_PB0_ACMD53_CRCINTR = 0x1 << 4,
	MSDC_PB0_ACMD53_ONESHOT = 0x1 << 5,
	MSDC_PB0_RESV3 = 0x1 << 6,
	MSDC_PB0_INT_DAT_LATCH_CK_SEL = 0x7 << 7,
	MSDC_PB0_CKGEN_MSDC_DLY_SEL = 0x1F << 10,
	MSDC_PB0_FIFORD_DIS = 0x1 << 15,
	MSDC_PB0_SDIO_DBSSEL = 0x1 << 16,
	MSDC_PB0_SDIO_INTCSEL = 0x1 << 17,
	MSDC_PB0_SDIO_BSYDLY = 0xf << 18,
	MSDC_PB0_SDC_WDOD = 0xf << 22,
	MSDC_PB0_CMDIDRTSEL = 0x1 << 26,
	MSDC_PB0_CMDFAILSEL = 0x1 << 27,
	MSDC_PB0_SDIO_INTDLYSEL = 0x1 << 28,
	MSDC_PB0_SPCPUSH = 0x1 << 29,
	MSDC_PB0_DETWR_CRCTMO = 0x1 << 30,
	MSDC_PB0_EN_DRVRSP = 0x1 << 31
};

/* MSDC_PATCH_BIT1 mask */
enum {
	MSDC_PB1_WRDAT_CRCS_TA_CNTR = 0x7 << 0,
	MSDC_PB1_CMD_RSP_TA_CNTR = 0x7 << 3,
	MSDC_PB1_RESV3 = 0x3 << 6,
	MSDC_PB1_BIAS_TUNE_28NM = 0xf << 8,
	MSDC_PB1_BIAS_EN18IO_28NM = 0x1 << 12,
	MSDC_PB1_BIAS_EXT_28NM = 0x1 << 13,
	MSDC_PB1_RESV2 = 0x3 << 14,
	MSDC_PB1_RESV1 = 0x7f << 16,
	MSDC_PB1_AHBCKEN = 0x1 << 23,
	MSDC_PB1_CKSPCEN = 0x1 << 24,
	MSDC_PB1_CKPSCEN = 0x1 << 25,
	MSDC_PB1_CKVOLDETEN = 0x1 << 26,
	MSDC_PB1_CKACMDEN = 0x1 << 27,
	MSDC_PB1_CKSDEN = 0x1 << 28,
	MSDC_PB1_CKWCTLEN = 0x1 << 29,
	MSDC_PB1_CKRCTLEN = 0x1 << 30,
	MSDC_PB1_CKSHBFFEN = 0x1 << 31
};

/* MSDC_HW_DBG_SEL mask */
enum {
	MSDC_HW_DBG3_SEL = 0xff << 0,
	MSDC_HW_DBG2_SEL = 0x3f << 8,
	MSDC_HW_DBG1_SEL = 0x3f << 16,
	MSDC_HW_DBG_WRAPTYPE_SEL = 0x3 << 22,
	MSDC_HW_DBG0_SEL = 0x3f << 24,
	MSDC_HW_DBG_WRAP_SEL = 0x1  << 30
};

/* EMMC50_PAD_DS_TUNE mask */
enum {
	MSDC_EMMC50_PAD_DS_TUNE_DLYSEL = 0x1 << 0,
	MSDC_EMMC50_PAD_DS_TUNE_DLY2SEL = 0x1 << 1,
	MSDC_EMMC50_PAD_DS_TUNE_DLY1 = 0x1f << 2,
	MSDC_EMMC50_PAD_DS_TUNE_DLY2 = 0x1f << 7,
	MSDC_EMMC50_PAD_DS_TUNE_DLY3 = 0x1F << 12
};

/* EMMC50_PAD_CMD_TUNE mask */
enum {
	MSDC_EMMC50_PAD_CMD_TUNE_DLY3SEL = 0x1 << 0,
	MSDC_EMMC50_PAD_CMD_TUNE_RXDLY3 = 0x1f << 1,
	MSDC_EMMC50_PAD_CMD_TUNE_TXDLY = 0x1f << 6
};

/* EMMC50_PAD_DAT01_TUNE mask */
enum {
	MSDC_EMMC50_PAD_DAT0_RXDLY3SEL = 0x1 << 0,
	MSDC_EMMC50_PAD_DAT0_RXDLY3 = 0x1f << 1,
	MSDC_EMMC50_PAD_DAT0_TXDLY = 0x1f << 6,
	MSDC_EMMC50_PAD_DAT1_RXDLY3SEL = 0x1 << 16,
	MSDC_EMMC50_PAD_DAT1_RXDLY3 = 0x1f << 17,
	MSDC_EMMC50_PAD_DAT1_TXDLY = 0x1f << 22
};

/* EMMC50_PAD_DAT23_TUNE mask */
enum {
	MSDC_EMMC50_PAD_DAT2_RXDLY3SEL = 0x1 << 0,
	MSDC_EMMC50_PAD_DAT2_RXDLY3 = 0x1f << 1,
	MSDC_EMMC50_PAD_DAT2_TXDLY = 0x1f << 6,
	MSDC_EMMC50_PAD_DAT3_RXDLY3SEL = 0x1 << 16,
	MSDC_EMMC50_PAD_DAT3_RXDLY3 = 0x1f << 17,
	MSDC_EMMC50_PAD_DAT3_TXDLY = 0x1f << 22
};

/* EMMC50_PAD_DAT45_TUNE mask */
enum {
	MSDC_EMMC50_PAD_DAT4_RXDLY3SEL = 0x1 << 0,
	MSDC_EMMC50_PAD_DAT4_RXDLY3 = 0x1f << 1,
	MSDC_EMMC50_PAD_DAT4_TXDLY = 0x1f << 6,
	MSDC_EMMC50_PAD_DAT5_RXDLY3SEL = 0x1 << 16,
	MSDC_EMMC50_PAD_DAT5_RXDLY3 = 0x1f << 17,
	MSDC_EMMC50_PAD_DAT5_TXDLY = 0x1f << 22
};

/* EMMC50_PAD_DAT67_TUNE mask */
enum {
	MSDC_EMMC50_PAD_DAT6_RXDLY3SEL = 0x1 << 0,
	MSDC_EMMC50_PAD_DAT6_RXDLY3 = 0x1f << 1,
	MSDC_EMMC50_PAD_DAT6_TXDLY = 0x1f << 6,
	MSDC_EMMC50_PAD_DAT7_RXDLY3SEL = 0x1 << 16,
	MSDC_EMMC50_PAD_DAT7_RXDLY3 = 0x1f << 17,
	MSDC_EMMC50_PAD_DAT7_TXDLY = 0x1f << 22
};

/* EMMC50_CFG0 mask */
enum {
	MSDC_EMMC50_CFG_PADCMD_LATCHCK = 0x1 << 0,
	MSDC_EMMC50_CFG_CRCSTS_EDGE = 0x1 << 3,
	MSDC_EMMC50_CFG_CFCSTS_SEL = 0x1 << 4,
	MSDC_EMMC50_CFG_ENDBIT_CHK_BIT = 0xf << 5,
	MSDC_EMMC50_CFG_CMDRSP_SEL = 0x1 << 9,
	MSDC_EMMC50_CFG_CMDEDGE_SEL = 0x1 << 10,
	MSDC_EMMC50_CFG_ENDBIT_CNT = 0x3ff << 11,
	MSDC_EMMC50_CFG_RDAT_CNT = 0x1fff << 21,
	MSDC_EMMC50_CFG_SPARE_BAK0 = 0x7f<< 24,
	MSDC_EMMC50_CFG_GDMA_RESET = 0x1 << 31
};

/* EMMC50_CFG1 mask */
enum {
	MSDC_EMMC50_CFG1_WRPTR_MARGIN = 0xff << 0,
	MSDC_EMMC50_CFG1_CKSWITCH_CNT = 0x7 << 8,
	MSDC_EMMC50_CFG1_RDDAT_STOP = 0x1 << 11,
	MSDC_EMMC50_CFG1_WAITCLK_CNT = 0xf << 12,
	MSDC_EMMC50_CFG1_DBG_SEL = 0xff << 16,
	MSDC_EMMC50_CFG1_SPARE1 = 0xff << 24
};

/* EMMC50_CFG2_mask */
enum {
	MSDC_EMMC50_CFG2_AXI_GPD_UP = 0x1 << 0,
	MSDC_EMMC50_CFG2_AXI_IOMMU_WR_EMI = 0x1 << 1,
	MSDC_EMMC50_CFG2_AXI_SHARE_EN_WR_EMI = 0x1 << 2,
	MSDC_EMMC50_CFG2_AXI_IOMMU_RD_EMI = 0x1 << 7,
	MSDC_EMMC50_CFG2_AXI_SHARE_EN_RD_EMI = 0x1 << 8,
	MSDC_EMMC50_CFG2_AXI_BOUND_128B = 0x1 << 13,
	MSDC_EMMC50_CFG2_AXI_BOUND_256B = 0x1 << 14,
	MSDC_EMMC50_CFG2_AXI_BOUND_512B = 0x1 << 15,
	MSDC_EMMC50_CFG2_AXI_BOUND_1K = 0x1 << 16,
	MSDC_EMMC50_CFG2_AXI_BOUND_2K = 0x1 << 17,
	MSDC_EMMC50_CFG2_AXI_BOUND_4K = 0x1 << 18,
	MSDC_EMMC50_CFG2_AXI_RD_OUTS_NUM = 0x1f << 19,
	MSDC_EMMC50_CFG2_AXI_BURSTSZ = 0xf << 24
};

/* EMMC50_CFG3_mask */
enum {
	MSDC_EMMC50_CFG3_OUTS_WR = 0x1f << 0,
	MSDC_EMMC50_CFG3_ULTRA_SET_WR = 0x3f << 5,
	MSDC_EMMC50_CFG3_PREULTRA_SET_WR = 0x3f << 11,
	MSDC_EMMC50_CFG3_ULTRA_SET_RD = 0x3f << 17,
	MSDC_EMMC50_CFG3_PREULTRA_SET_RD = 0x3f <<23
};

/* EMMC50_CFG4_mask */
enum {
	MSDC_EMMC50_CFG4_IMPR_ULTRA_SET_WR = 0xff << 0,
	MSDC_EMMC50_CFG4_IMPR_ULTRA_SET_RD = 0xff << 8,
	MSDC_EMMC50_CFG4_ULTRA_EN = 0x3 << 16
};

/* SDIO_TUNE_WIND mask*/
enum {
	MSDC_SDIO_TUNE_WIND = 0x1f << 0
};

/* MSDC pad control at top layer */
#define MSDC0_GPIO_CLK_BASE	(GPIO_BASE + 0xC00)
#define MSDC0_GPIO_CMD_BASE	(GPIO_BASE + 0xC10)
#define MSDC0_GPIO_DAT_BASE	(GPIO_BASE + 0xC20)
#define MSDC0_GPIO_PAD_BASE	(GPIO_BASE + 0xC30)
#define MSDC0_GPIO_RST_BASE	(GPIO_BASE + 0xD00)
#define MSDC0_GPIO_DS_BASE	(GPIO_BASE + 0xD10)
#define MSDC0_GPIO_MODE0_BASE	(GPIO_BASE + 0x6B0)
#define MSDC0_GPIO_MODE1_BASE	(GPIO_BASE + 0x6C0)
#define MSDC0_GPIO_MODE2_BASE	(GPIO_BASE + 0x6D0)

enum {
	GPIO_PAD_TDSEL_MASK = 0xFUL  <<  0,
	GPIO_PAD_RDSEL_MASK = 0x3FUL <<  4,
	GPIO_PAD_BIAS_MASK = 0xFUL  << 12
};

enum {
	GPIO_MSDC_R1R0_MASK = 0x3UL  <<  0,
	GPIO_MSDC_PUPD_MASK = 0x1UL  <<  2,
	GPIO_MSDC_DRV_MASK = 0x7UL  <<  8,
	GPIO_MSDC_SR_MASK = 0x1UL  << 12,
	GPIO_MSDC_SMT_MASK = 0x1UL  << 13,
	GPIO_MSDC_IES_MASK = 0x1UL  << 14,

	GPIO_MSDC_DAT0_R1R0_MASK = 0x3UL  <<  0,
	GPIO_MSDC_DAT0_PUPD_MASK = 0x1UL  <<  2,
	GPIO_MSDC_DAT1_R1R0_MASK = 0x3UL  <<  4,
	GPIO_MSDC_DAT1_PUPD_MASK = 0x1UL  <<  6,
	GPIO_MSDC_DAT2_R1R0_MASK = 0x3UL  <<  8,
	GPIO_MSDC_DAT2_PUPD_MASK = 0x1UL  << 10,
	GPIO_MSDC_DAT3_R1R0_MASK = 0x3UL  << 12,
	GPIO_MSDC_DAT3_PUPD_MASK = 0x1UL  << 14
};

/* add pull down/up mode define */
#define MSDC_GPIO_PULL_UP        (0)
#define MSDC_GPIO_PULL_DOWN      (1)

#define GPIO_CLK_CTRL       (0)
#define GPIO_CMD_CTRL       (1)
#define GPIO_DAT_CTRL       (2)
#define GPIO_RST_CTRL       (3)
#define GPIO_DS_CTRL        (4)
#define GPIO_MODE_CTRL      (5)

#define MSDC_PULL_0K        (0)
#define MSDC_PULL_10K       (1)
#define MSDC_PULL_50K       (2)
#define MSDC_PULL_8K        (3)

typedef enum MSDC_POWER {
	MSDC_VIO18_MC1 = 0,
	MSDC_VIO18_MC2,
	MSDC_VIO28_MC1,
	MSDC_VIO28_MC2,
	MSDC_VMC,
	MSDC_VGP,
} MSDC_POWER_DOMAIN;

/* Descriptor Structure */
typedef struct {
	uint32_t hwo:1; /* could be changed by hw */
	uint32_t bdp:1;
	uint32_t rsv0:6;
	uint32_t chksum:8;
	uint32_t intr:1;
	uint32_t rsv1:15;
	void *next;
	void *ptr;
	uint32_t buflen:16;
	uint32_t extlen:8;
	uint32_t rsv2:8;
	uint32_t arg;
	uint32_t blknum;
	uint32_t cmd;
} gpd_t;

typedef struct {
	uint32_t eol:1;
	uint32_t rsv0:7;
	uint32_t chksum:8;
	uint32_t rsv1:1;
	uint32_t blkpad:1;
	uint32_t dwpad:1;
	uint32_t rsv2:13;
	void *next;
	void *ptr;
	uint32_t buflen:16;
	uint32_t rsv3:16;
} bd_t;

/* Register Debugging Structure */
typedef struct {
	uint32_t msdc:1;
	uint32_t ckpwn:1;
	uint32_t rst:1;
	uint32_t pio:1;
	uint32_t ckdrven:1;
	uint32_t start18v:1;
	uint32_t pass18v:1;
	uint32_t ckstb:1;
	uint32_t ckdiv:8;
	uint32_t ckmod:2;
	uint32_t pad:14;
} msdc_cfg_reg;

typedef struct {
	uint32_t sdr104cksel:1;
	uint32_t rsmpl:1;
	uint32_t dsmpl:1;
	uint32_t ddlysel:1;
	uint32_t ddr50ckd:1;
	uint32_t dsplsel:1;
	uint32_t pad1:10;
	uint32_t d0spl:1;
	uint32_t d1spl:1;
	uint32_t d2spl:1;
	uint32_t d3spl:1;
	uint32_t d4spl:1;
	uint32_t d5spl:1;
	uint32_t d6spl:1;
	uint32_t d7spl:1;
	uint32_t riscsz:1;
	uint32_t pad2:7;
} msdc_iocon_reg;

typedef struct {
	uint32_t cden:1;
	uint32_t cdsts:1;
	uint32_t pad1:10;
	uint32_t cddebounce:4;
	uint32_t dat:8;
	uint32_t cmd:1;
	uint32_t pad2:6;
	uint32_t wp:1;
} msdc_ps_reg;

typedef struct {
	uint32_t mmcirq:1;
	uint32_t cdsc:1;
	uint32_t pad1:1;
	uint32_t atocmdrdy:1;
	uint32_t atocmdtmo:1;
	uint32_t atocmdcrc:1;
	uint32_t dmaqempty:1;
	uint32_t sdioirq:1;
	uint32_t cmdrdy:1;
	uint32_t cmdtmo:1;
	uint32_t rspcrc:1;
	uint32_t csta:1;
	uint32_t xfercomp:1;
	uint32_t dxferdone:1;
	uint32_t dattmo:1;
	uint32_t datcrc:1;
	uint32_t atocmd19done:1;
	uint32_t pad2:15;
} msdc_int_reg;

typedef struct {
	uint32_t mmcirq:1;
	uint32_t cdsc:1;
	uint32_t pad1:1;
	uint32_t atocmdrdy:1;
	uint32_t atocmdtmo:1;
	uint32_t atocmdcrc:1;
	uint32_t dmaqempty:1;
	uint32_t sdioirq:1;
	uint32_t cmdrdy:1;
	uint32_t cmdtmo:1;
	uint32_t rspcrc:1;
	uint32_t csta:1;
	uint32_t xfercomp:1;
	uint32_t dxferdone:1;
	uint32_t dattmo:1;
	uint32_t datcrc:1;
	uint32_t atocmd19done:1;
	uint32_t pad2:15;
} msdc_inten_reg;

typedef struct {
	uint32_t rxcnt:8;
	uint32_t pad1:8;
	uint32_t txcnt:8;
	uint32_t pad2:7;
	uint32_t clr:1;
} msdc_fifocs_reg;

typedef struct {
	uint32_t val;
} msdc_txdat_reg;

typedef struct {
	uint32_t val;
} msdc_rxdat_reg;

typedef struct {
	uint32_t sdiowkup:1;
	uint32_t inswkup:1;
	uint32_t pad1:14;
	uint32_t buswidth:2;
	uint32_t pad2:1;
	uint32_t sdio:1;
	uint32_t sdioide:1;
	uint32_t intblkgap:1;
	uint32_t pad4:2;
	uint32_t dtoc:8;
} sdc_cfg_reg;

typedef struct {
	uint32_t cmd:6;
	uint32_t brk:1;
	uint32_t rsptyp:3;
	uint32_t pad1:1;
	uint32_t dtype:2;
	uint32_t rw:1;
	uint32_t stop:1;
	uint32_t goirq:1;
	uint32_t blklen:12;
	uint32_t atocmd:2;
	uint32_t volswth:1;
	uint32_t pad2:1;
} sdc_cmd_reg;

typedef struct {
	uint32_t arg;
} sdc_arg_reg;

typedef struct {
	uint32_t sdcbusy:1;
	uint32_t cmdbusy:1;
	uint32_t pad:29;
	uint32_t swrcmpl:1;
} sdc_sts_reg;

typedef struct {
	uint32_t val;
} sdc_resp0_reg;

typedef struct {
	uint32_t val;
} sdc_resp1_reg;

typedef struct {
	uint32_t val;
} sdc_resp2_reg;

typedef struct {
	uint32_t val;
} sdc_resp3_reg;

typedef struct {
	uint32_t num;
} sdc_blknum_reg;

typedef struct {
	uint32_t sts;
} sdc_csts_reg;

typedef struct {
	uint32_t sts;
} sdc_cstsen_reg;

typedef struct {
	uint32_t datcrcsts:8;
	uint32_t ddrcrcsts:4;
	uint32_t pad:20;
} sdc_datcrcsts_reg;

typedef struct {
	uint32_t bootstart:1;
	uint32_t bootstop:1;
	uint32_t bootmode:1;
	uint32_t pad1:9;
	uint32_t bootwaidly:3;
	uint32_t bootsupp:1;
	uint32_t pad2:16;
} emmc_cfg0_reg;

typedef struct {
	uint32_t bootcrctmc:16;
	uint32_t pad:4;
	uint32_t bootacktmc:12;
} emmc_cfg1_reg;

typedef struct {
	uint32_t bootcrcerr:1;
	uint32_t bootackerr:1;
	uint32_t bootdattmo:1;
	uint32_t bootacktmo:1;
	uint32_t bootupstate:1;
	uint32_t bootackrcv:1;
	uint32_t bootdatrcv:1;
	uint32_t pad:25;
} emmc_sts_reg;

typedef struct {
	uint32_t bootrst:1;
	uint32_t pad:31;
} emmc_iocon_reg;

typedef struct {
	uint32_t val;
} msdc_acmd_resp_reg;

typedef struct {
	uint32_t tunesel:4;
	uint32_t pad:28;
} msdc_acmd19_trg_reg;

typedef struct {
	uint32_t val;
} msdc_acmd19_sts_reg;

typedef struct {
	uint32_t addr;
} msdc_dma_sa_reg;

typedef struct {
	uint32_t addr;
} msdc_dma_ca_reg;

typedef struct {
	uint32_t start:1;
	uint32_t stop:1;
	uint32_t resume:1;
	uint32_t pad1:5;
	uint32_t mode:1;
	uint32_t pad2:1;
	uint32_t lastbuf:1;
	uint32_t pad3:1;
	uint32_t brustsz:3;
	uint32_t pad4:1;
	uint32_t xfersz:16;
} msdc_dma_ctrl_reg;

typedef struct {
	uint32_t status:1;
	uint32_t decsen:1;
	uint32_t pad1:2;
	uint32_t bdcsen:1;
	uint32_t gpdcsen:1;
	uint32_t pad2:26;
} msdc_dma_cfg_reg;

typedef struct {
	uint32_t sel:16;
	uint32_t pad2:16;
} msdc_dbg_sel_reg;

typedef struct {
	uint32_t val;
} msdc_dbg_out_reg;

typedef struct {
	uint32_t clkdrvn:3;
	uint32_t rsv0:1;
	uint32_t clkdrvp:3;
	uint32_t rsv1:1;
	uint32_t clksr:1;
	uint32_t rsv2:7;
	uint32_t clkpd:1;
	uint32_t clkpu:1;
	uint32_t clksmt:1;
	uint32_t clkies:1;
	uint32_t clktdsel:4;
	uint32_t clkrdsel:8;
} msdc_pad_ctl0_reg;

typedef struct {
	uint32_t cmddrvn:3;
	uint32_t rsv0:1;
	uint32_t cmddrvp:3;
	uint32_t rsv1:1;
	uint32_t cmdsr:1;
	uint32_t rsv2:7;
	uint32_t cmdpd:1;
	uint32_t cmdpu:1;
	uint32_t cmdsmt:1;
	uint32_t cmdies:1;
	uint32_t cmdtdsel:4;
	uint32_t cmdrdsel:8;
} msdc_pad_ctl1_reg;

typedef struct {
	uint32_t datdrvn:3;
	uint32_t rsv0:1;
	uint32_t datdrvp:3;
	uint32_t rsv1:1;
	uint32_t datsr:1;
	uint32_t rsv2:7;
	uint32_t datpd:1;
	uint32_t datpu:1;
	uint32_t datsmt:1;
	uint32_t daties:1;
	uint32_t dattdsel:4;
	uint32_t datrdsel:8;
} msdc_pad_ctl2_reg;

typedef struct {
	uint32_t datwrdly:5;
	uint32_t pad1:3;
	uint32_t datrddly:5;
	uint32_t pad2:3;
	uint32_t cmdrxdly:5;
	uint32_t pad3:1;
	uint32_t cmdrsprxdly:5;
	uint32_t clktxdly:5;
} msdc_pad_tune_reg;

typedef struct {
	uint32_t dat0:5;
	uint32_t rsv0:3;
	uint32_t dat1:5;
	uint32_t rsv1:3;
	uint32_t dat2:5;
	uint32_t rsv2:3;
	uint32_t dat3:5;
	uint32_t rsv3:3;
} msdc_dat_rddly0;

typedef struct {
	uint32_t dat4:5;
	uint32_t rsv4:3;
	uint32_t dat5:5;
	uint32_t rsv5:3;
	uint32_t dat6:5;
	uint32_t rsv6:3;
	uint32_t dat7:5;
	uint32_t rsv7:3;
} msdc_dat_rddly1;

typedef struct {
	uint32_t dbg0sel:8;
	uint32_t dbg1sel:6;
	uint32_t pad1:2;
	uint32_t dbg2sel:6;
	uint32_t pad2:2;
	uint32_t dbg3sel:6;
	uint32_t pad3:2;
} msdc_hw_dbg_reg;

typedef struct {
	uint32_t val;
} msdc_version_reg;

typedef struct {
	uint32_t val;
} msdc_eco_ver_reg;

struct msdc_regs {
	msdc_cfg_reg msdc_cfg;		/* base+0x00h */
	msdc_iocon_reg msdc_iocon;	/* base+0x04h */
	msdc_ps_reg msdc_ps;		/* base+0x08h */
	msdc_int_reg msdc_int;		/* base+0x0ch */
	msdc_inten_reg msdc_inten;	/* base+0x10h */
	msdc_fifocs_reg msdc_fifocs;	/* base+0x14h */
	msdc_txdat_reg msdc_txdat;	/* base+0x18h */
	msdc_rxdat_reg msdc_rxdat;	/* base+0x1ch */
	uint32_t rsv1[4];
	sdc_cfg_reg sdc_cfg;		/* base+0x30h */
	sdc_cmd_reg sdc_cmd;		/* base+0x34h */
	sdc_arg_reg sdc_arg;		/* base+0x38h */
	sdc_sts_reg sdc_sts;		/* base+0x3ch */
	sdc_resp0_reg sdc_resp0;	/* base+0x40h */
	sdc_resp1_reg sdc_resp1;	/* base+0x44h */
	sdc_resp2_reg sdc_resp2;	/* base+0x48h */
	sdc_resp3_reg sdc_resp3;	/* base+0x4ch */
	sdc_blknum_reg sdc_blknum;	/* base+0x50h */
	uint32_t rsv2[1];
	sdc_csts_reg sdc_csts;		/* base+0x58h */
	sdc_cstsen_reg sdc_cstsen;	/* base+0x5ch */
	sdc_datcrcsts_reg sdc_dcrcsta;	/* base+0x60h */
	uint32_t rsv3[3];
	emmc_cfg0_reg emmc_cfg0;	/* base+0x70h */
	emmc_cfg1_reg emmc_cfg1;	/* base+0x74h */
	emmc_sts_reg emmc_sts;		/* base+0x78h */
	emmc_iocon_reg emmc_iocon;	/* base+0x7ch */
	msdc_acmd_resp_reg acmd_resp;	/* base+0x80h */
	msdc_acmd19_trg_reg acmd19_trg;	/* base+0x84h */
	msdc_acmd19_sts_reg acmd19_sts;	/* base+0x88h */
	uint32_t rsv4[1];
	msdc_dma_sa_reg dma_sa;		/* base+0x90h */
	msdc_dma_ca_reg dma_ca;		/* base+0x94h */
	msdc_dma_ctrl_reg dma_ctrl;	/* base+0x98h */
	msdc_dma_cfg_reg dma_cfg;	/* base+0x9ch */
	msdc_dbg_sel_reg dbg_sel;	/* base+0xa0h */
	msdc_dbg_out_reg dbg_out;	/* base+0xa4h */
	uint32_t rsv5[2];
	uint32_t patch0;		/* base+0xb0h */
	uint32_t patch1;		/* base+0xb4h */
	uint32_t rsv6[10];
	msdc_pad_ctl0_reg pad_ctl0;	/* base+0xe0h */
	msdc_pad_ctl1_reg pad_ctl1;	/* base+0xe4h */
	msdc_pad_ctl2_reg pad_ctl2;	/* base+0xe8h */
	msdc_pad_tune_reg pad_tune;	/* base+0xech */
	msdc_dat_rddly0 dat_rddly0;	/* base+0xf0h */
	msdc_dat_rddly1 dat_rddly1;	/* base+0xf4h */
	msdc_hw_dbg_reg hw_dbg;		/* base+0xf8h */
	uint32_t rsv7[1];
	msdc_version_reg version;	/* base+0x100h */
	msdc_eco_ver_reg eco_ver;	/* base+0x104h */
};

struct scatterlist {
	u32 addr;
	u32 len;
};

struct scatterlist_ex {
	u32 cmd;
	u32 arg;
	u32 sglen;
	struct scatterlist *sg;
};

enum {
	DMA_FLAG_NONE = 0x00000000,
	DMA_FLAG_EN_CHKSUM = 0x00000001,
	DMA_FLAG_PAD_BLOCK = 0x00000002,
	DMA_FLAG_PAD_DWORD = 0x00000004
};

struct dma_config {
	u32 flags;		/* flags */
	u32 xfersz;		/* xfer size in bytes */
	u32 sglen;		/* size of scatter list */
	u32 blklen;		/* block size */
	struct scatterlist *sg;	/* I/O scatter list */
	struct scatterlist_ex *esg;	/* extended I/O scatter list */
	u8 mode;		/* dma mode */
	u8 burstsz;		/* burst size */
	u8 intr;		/* dma done interrupt */
	u8 padding;		/* padding */
	u32 cmd;		/* enhanced mode command */
	u32 arg;		/* enhanced mode arg */
	u32 rsp;		/* enhanced mode command response */
	u32 autorsp;		/* auto command response */
};

#define MSDC_WRITE32(addr, data)	write32((void *)addr, data)
#define MSDC_READ32(addr)		read32((const void *)addr)
#define MSDC_WRITE16(addr, data)	write16((void *)addr, data)
#define MSDC_READ16(addr)		read16((const void *)addr)
#define MSDC_WRITE8(addr, data)		write8((void *)addr, data)
#define MSDC_READ8(addr)		read8((const void *)addr)

#define MSDC_SET_BIT32(addr, mask)	\
	do { \
	unsigned int tv = MSDC_READ32(addr); \
	tv |=((u32)(mask)); \
	MSDC_WRITE32(addr, tv); \
	} while (0)

#define MSDC_CLR_BIT32(addr, mask)	\
	do { \
	unsigned int tv = MSDC_READ32(addr); \
	tv &= ~((u32)(mask)); \
	MSDC_WRITE32(addr, tv); \
	} while (0)

#define MSDC_SET_BIT16(addr, mask)	\
	do { \
	unsigned short tv = MSDC_READ16(addr); \
	tv |= ((u16)(mask)); \
	MSDC_WRITE16(addr, tv); \
	} while (0)

#define MSDC_CLR_BIT16(addr, mask)	\
	do { \
	unsigned short tv = MSDC_READ16(addr); \
	tv &= ~((u16)(mask)); \
	MSDC_WRITE16(addr, tv); \
} while(0)

#define msdc_set_field(reg, field, val) \
	do{ \
		u32 tv = MSDC_READ32(reg); \
		tv &= ~((u32)(field)); \
		tv |= ((val) << (msdc_uffs((unsigned int)field) - 1)); \
		MSDC_WRITE32(reg,tv); \
	}while(0)

#define MSDC_SET_FIELD(reg, field, val) msdc_set_field(reg, field, val)

#define MSDC_GET_FIELD(reg, field, val) \
	get_field(((uintptr_t)(reg)), (field), (val))

#define MSDC_SET_FIELD_DISCRETE(reg, field, val) \
	do {	\
		unsigned int tv = (unsigned int)(*(volatile u32*)((uintptr_t)(reg))); \
		tv = (val == 1) ? (tv|(field)):(tv & ~(field));\
		(*(volatile u32*)(reg) = (u32)(tv)); \
	} while (0)

#define MSDC_GET_FIELD_DISCRETE(reg, field, val) \
	do {	\
		unsigned int tv = (unsigned int)(*(volatile u32*)((uintptr_t)(reg))); \
		val = tv & (field) ; \
		val = (val != 0) ? 1 :0;\
	} while (0)

#define MSDC_RETRY(expr, retry, cnt) \
	do { \
		uint32_t t = cnt; \
		uint32_t r = retry; \
		uint32_t c = cnt; \
		while (r) { \
			if (!(expr)) \
				break; \
				if (c-- == 0) { \
					r--; udelay(200); c = t; \
				} \
		} \
		WARN_ON(r == 0); \
	} while (0)

#define MSDC_RESET(base) \
	do { \
		MSDC_SET_BIT32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_CFG)), MSDC_CFG_RST); \
		MSDC_RETRY(MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_CFG))) \
		& MSDC_CFG_RST, 5, 1000); \
	} while (0)

#define MSDC_CLR_INT(base) \
	do { \
		volatile uint32_t val = MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_INT))); \
		MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_INT)), val); \
		if (MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_INT)))) { \
			MSG(ERR, "[ASSERT] MSDC_INT is NOT clear\n"); \
		} \
	} while (0)

#define MSDC_CLR_FIFO(base) \
	do { \
		MSDC_SET_BIT32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_FIFOCS)), MSDC_FIFOCS_CLR); \
		MSDC_RETRY(MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_FIFOCS))) & MSDC_FIFOCS_CLR, 5, 1000); \
	} while (0)

#define MSDC_FIFO_WRITE32(base, val)	MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_TXDATA)), val)
#define MSDC_FIFO_READ32(base)	MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_RXDATA)))
#define MSDC_FIFO_WRITE16(base, val)	MSDC_WRITE16((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_TXDATA)), val)
#define MSDC_FIFO_READ16(base)	MSDC_READ16((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_RXDATA)))
#define MSDC_FIFO_WRITE8(base, val)   MSDC_WRITE8((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_TXDATA)), val)
#define MSDC_FIFO_READ8(base)	MSDC_READ8((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_RXDATA)))

#define MSDC_TXFIFOCNT(base) \
    ((MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_FIFOCS))) & MSDC_FIFOCS_TXCNT) >> 16)
#define MSDC_RXFIFOCNT(base) \
    ((MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_FIFOCS))) & MSDC_FIFOCS_RXCNT) >> 0)

#define MSDC_CARD_DETECTION_ON(base)  MSDC_SET_BIT32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_PS)), MSDC_PS_CDEN)
#define MSDC_CARD_DETECTION_OFF(base) MSDC_CLR_BIT32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_PS)), MSDC_PS_CDEN)

#define SDC_IS_BUSY(base) (MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_STS))) & SDC_STS_SDCBUSY)
#define SDC_IS_CMD_BUSY(base) (MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_STS))) & SDC_STS_CMDBUSY)

int msdc_reg_test(int id);
int msdc_init(struct mmc_host *host, int id);
int msdc_pio_bread(struct mmc_host *host, uint8_t *dst, uint32_t src,
		   uint32_t nblks);
int msdc_pio_bwrite(struct mmc_host *host, uint32_t dst, uint8_t *src,
		    uint32_t nblks);
int msdc_dma_bread(struct mmc_host *host, uint8_t *dst, uint32_t src,
		   uint32_t nblks);
int msdc_dma_bwrite(struct mmc_host *host, uint32_t dst, uint8_t *src,
		    uint32_t nblks);
int msdc_stream_bread(struct mmc_host *host, uint8_t *dst, uint32_t src,
		      uint32_t nblks);
int msdc_stream_bwrite(struct mmc_host *host, uint32_t dst, uint8_t *src,
		       uint32_t nblks);
int msdc_tune_bwrite(struct mmc_host *host, uint32_t dst, uint8_t *src,
		     uint32_t nblks);
int msdc_tune_bread(struct mmc_host *host, uint8_t *dst, uint32_t src,
		    uint32_t nblks);
int msdc_tune_cmdrsp(struct mmc_host *host, struct mmc_command *cmd);
void msdc_reset_tune_counter(struct mmc_host *host);
int msdc_abort_handler(struct mmc_host *host, int abort_card);
int msdc_tune_read(struct mmc_host *host);
void msdc_intr_sdio(struct mmc_host *host, int enable);
void msdc_intr_sdio_gap(struct mmc_host *host, int enable);
void msdc_config_clock(struct mmc_host *host, int ddr, u32 hz);
int msdc_pio_send_sandisk_fwid(struct mmc_host *host, uint8_t *src);
int msdc_pio_get_sandisk_fwid(struct mmc_host *host, uint8_t *src);
int msdc_dma_send_sandisk_fwid(struct mmc_host *host, uint8_t *buf, u32 opcode,
			       uint32_t nblks);

int msdc_send_cmd(struct mmc_host *host, struct mmc_command *cmd);
int msdc_wait_rsp(struct mmc_host *host, struct mmc_command *cmd);

int msdc_cmd(struct mmc_host *host, struct mmc_command *cmd);
int msdc_card_protected(struct mmc_host *host);
int msdc_card_avail(struct mmc_host *host);
void msdc_power(struct mmc_host *host, u8 mode);
void msdc_config_bus(struct mmc_host *host, u32 width);
void msdc_set_timeout(struct mmc_host *host, u32 ns, u32 clks);
int msdc_switch_volt(struct mmc_host *host, int volt);
int msdc_tune_uhs1(struct mmc_host *host, struct mmc_card *card);
int msdc_pio_read(struct mmc_host *host, u32 *ptr, u32 size);
void msdc_set_blklen(struct mmc_host *host, u32 blklen);
void msdc_set_blknum(struct mmc_host *host, u32 blknum);

int msdc_emmc_boot_reset(struct mmc_host *host, int reset);
int msdc_emmc_boot_start(struct mmc_host *host, u32 hz, int ddr, int mode,
			 int ackdis);
int msdc_emmc_boot_read(struct mmc_host *host, u32 size, u32 *to);
int msdc_emmc_boot_stop(struct mmc_host *host, int mode);

/* unused in download agent */
void msdc_sdioirq(struct mmc_host *host, int enable);
void msdc_sdioirq_register(struct mmc_host *host, hw_irq_handler_t handler);
void msdc_set_blksz(struct mmc_host *host, u32 sz);
int msdc_fifo_read(struct mmc_host *host, u32 *ptr, u32 size);
int msdc_fifo_write(struct mmc_host *host, u32 *ptr, u32 size);

#endif /* end of SOC_MEDIATEK_MT8173_MSDC_H */
