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

#ifndef SOC_MEDIATEK_MT8135_MSDC_H
#define SOC_MEDIATEK_MT8135_MSDC_H

#include "msdc_cfg.h"
#include "mmc_core.h"

/*--------------------------------------------------------------------------*/
/* Common Macro                                                             */
/*--------------------------------------------------------------------------*/
#define REG_ADDR(x)             ((volatile uint32_t *)(base + OFFSET_##x))

/*--------------------------------------------------------------------------*/
/* Common Definition                                                        */
/*--------------------------------------------------------------------------*/
#define MSDC_FIFO_SZ            (128)
#define MSDC_FIFO_THD           (128)
#define MSDC_MAX_NUM            (2)

#define MSDC_MS                 (0)
#define MSDC_SDMMC              (1)

#define MSDC_MODE_UNKNOWN       (0)
#define MSDC_MODE_PIO           (1)
#define MSDC_MODE_DMA_BASIC     (2)
#define MSDC_MODE_DMA_DESC      (3)
#define MSDC_MODE_DMA_ENHANCED  (4)
#define MSDC_MODE_MMC_STREAM    (5)

#define MSDC_BUS_1BITS          (0)
#define MSDC_BUS_4BITS          (1)
#define MSDC_BUS_8BITS          (2)

#define MSDC_BRUST_8B           (3)
#define MSDC_BRUST_16B          (4)
#define MSDC_BRUST_32B          (5)
#define MSDC_BRUST_64B          (6)

#define MSDC_PIN_PULL_NONE      (0)
#define MSDC_PIN_PULL_DOWN      (1)
#define MSDC_PIN_PULL_UP        (2)
#define MSDC_PIN_KEEP           (3)

#define MSDC_MAX_SCLK           (100000000)
#define MSDC_MIN_SCLK           (260000)

#define MSDC_AUTOCMD12          (0x0001)
#define MSDC_AUTOCMD23          (0x0002)
#define MSDC_AUTOCMD19          (0x0003)

/*--------------------------------------------------------------------------*/
/* Register Offset                                                          */
/*--------------------------------------------------------------------------*/
#define OFFSET_MSDC_CFG         (0x0)
#define OFFSET_MSDC_IOCON       (0x04)
#define OFFSET_MSDC_PS          (0x08)
#define OFFSET_MSDC_INT         (0x0c)
#define OFFSET_MSDC_INTEN       (0x10)
#define OFFSET_MSDC_FIFOCS      (0x14)
#define OFFSET_MSDC_TXDATA      (0x18)
#define OFFSET_MSDC_RXDATA      (0x1c)
#define OFFSET_SDC_CFG          (0x30)
#define OFFSET_SDC_CMD          (0x34)
#define OFFSET_SDC_ARG          (0x38)
#define OFFSET_SDC_STS          (0x3c)
#define OFFSET_SDC_RESP0        (0x40)
#define OFFSET_SDC_RESP1        (0x44)
#define OFFSET_SDC_RESP2        (0x48)
#define OFFSET_SDC_RESP3        (0x4c)
#define OFFSET_SDC_BLK_NUM      (0x50)
#define OFFSET_SDC_CSTS         (0x58)
#define OFFSET_SDC_CSTS_EN      (0x5c)
#define OFFSET_SDC_DCRC_STS     (0x60)
#define OFFSET_EMMC_CFG0        (0x70)
#define OFFSET_EMMC_CFG1        (0x74)
#define OFFSET_EMMC_STS         (0x78)
#define OFFSET_EMMC_IOCON       (0x7c)
#define OFFSET_SDC_ACMD_RESP    (0x80)
#define OFFSET_SDC_ACMD19_TRG   (0x84)
#define OFFSET_SDC_ACMD19_STS   (0x88)
#define OFFSET_MSDC_DMA_SA      (0x90)
#define OFFSET_MSDC_DMA_CA      (0x94)
#define OFFSET_MSDC_DMA_CTRL    (0x98)
#define OFFSET_MSDC_DMA_CFG     (0x9c)
#define OFFSET_MSDC_DBG_SEL     (0xa0)
#define OFFSET_MSDC_DBG_OUT     (0xa4)
#define OFFSET_MSDC_DMA_LEN     (0xa8)
#define OFFSET_MSDC_PATCH_BIT0  (0xb0)
#define OFFSET_MSDC_PATCH_BIT1  (0xb4)
#define OFFSET_MSDC_PAD_CTL0    (0xe0)
#define OFFSET_MSDC_PAD_CTL1    (0xe4)
#define OFFSET_MSDC_PAD_CTL2    (0xe8)
#define OFFSET_MSDC_PAD_TUNE    (0xec)
#define OFFSET_MSDC_DAT_RDDLY0  (0xf0)
#define OFFSET_MSDC_DAT_RDDLY1  (0xf4)
#define OFFSET_MSDC_HW_DBG      (0xf8)
#define OFFSET_MSDC_VERSION     (0x100)
#define OFFSET_MSDC_ECO_VER     (0x104)

/*--------------------------------------------------------------------------*/
/* Register Address                                                         */
/*--------------------------------------------------------------------------*/

/* common register */
#define MSDC_CFG                REG_ADDR(MSDC_CFG)
#define MSDC_IOCON              REG_ADDR(MSDC_IOCON)
#define MSDC_PS                 REG_ADDR(MSDC_PS)
#define MSDC_INT                REG_ADDR(MSDC_INT)
#define MSDC_INTEN              REG_ADDR(MSDC_INTEN)
#define MSDC_FIFOCS             REG_ADDR(MSDC_FIFOCS)
#define MSDC_TXDATA             REG_ADDR(MSDC_TXDATA)
#define MSDC_RXDATA             REG_ADDR(MSDC_RXDATA)

/* sdmmc register */
#define SDC_CFG                 REG_ADDR(SDC_CFG)
#define SDC_CMD                 REG_ADDR(SDC_CMD)
#define SDC_ARG                 REG_ADDR(SDC_ARG)
#define SDC_STS                 REG_ADDR(SDC_STS)
#define SDC_RESP0               REG_ADDR(SDC_RESP0)
#define SDC_RESP1               REG_ADDR(SDC_RESP1)
#define SDC_RESP2               REG_ADDR(SDC_RESP2)
#define SDC_RESP3               REG_ADDR(SDC_RESP3)
#define SDC_BLK_NUM             REG_ADDR(SDC_BLK_NUM)
#define SDC_CSTS                REG_ADDR(SDC_CSTS)
#define SDC_CSTS_EN             REG_ADDR(SDC_CSTS_EN)
#define SDC_DCRC_STS            REG_ADDR(SDC_DCRC_STS)

/* emmc register*/
#define EMMC_CFG0               REG_ADDR(EMMC_CFG0)
#define EMMC_CFG1               REG_ADDR(EMMC_CFG1)
#define EMMC_STS                REG_ADDR(EMMC_STS)
#define EMMC_IOCON              REG_ADDR(EMMC_IOCON)

/* auto command register */
#define SDC_ACMD_RESP           REG_ADDR(SDC_ACMD_RESP)
#define SDC_ACMD19_TRG          REG_ADDR(SDC_ACMD19_TRG)
#define SDC_ACMD19_STS          REG_ADDR(SDC_ACMD19_STS)

/* dma register */
#define MSDC_DMA_SA             REG_ADDR(MSDC_DMA_SA)
#define MSDC_DMA_CA             REG_ADDR(MSDC_DMA_CA)
#define MSDC_DMA_CTRL           REG_ADDR(MSDC_DMA_CTRL)
#define MSDC_DMA_CFG            REG_ADDR(MSDC_DMA_CFG)
#ifdef CONFIG_ARCH_MT8135
#define MSDC_DMA_LEN            REG_ADDR(MSDC_DMA_LEN)
#endif

/* pad ctrl register */
#define MSDC_PAD_CTL0           REG_ADDR(MSDC_PAD_CTL0)
#define MSDC_PAD_CTL1           REG_ADDR(MSDC_PAD_CTL1)
#define MSDC_PAD_CTL2           REG_ADDR(MSDC_PAD_CTL2)

/* data read delay */
#define MSDC_DAT_RDDLY0         REG_ADDR(MSDC_DAT_RDDLY0)
#define MSDC_DAT_RDDLY1         REG_ADDR(MSDC_DAT_RDDLY1)

/* debug register */
#define MSDC_DBG_SEL            REG_ADDR(MSDC_DBG_SEL)
#define MSDC_DBG_OUT            REG_ADDR(MSDC_DBG_OUT)

/* misc register */
#define MSDC_PATCH_BIT0         REG_ADDR(MSDC_PATCH_BIT0)
#define MSDC_PATCH_BIT1         REG_ADDR(MSDC_PATCH_BIT1)
#define MSDC_PAD_TUNE           REG_ADDR(MSDC_PAD_TUNE)
#define MSDC_HW_DBG             REG_ADDR(MSDC_HW_DBG)
#define MSDC_VERSION            REG_ADDR(MSDC_VERSION)
#define MSDC_ECO_VER            REG_ADDR(MSDC_ECO_VER)

/*--------------------------------------------------------------------------*/
/* Register Mask                                                            */
/*--------------------------------------------------------------------------*/

/* MSDC_CFG mask */
#define MSDC_CFG_MODE           (0x1  << 0)	/* RW */
#define MSDC_CFG_CKPDN          (0x1  << 1)	/* RW */
#define MSDC_CFG_RST            (0x1  << 2)	/* RW */
#define MSDC_CFG_PIO            (0x1  << 3)	/* RW */
#define MSDC_CFG_CKDRVEN        (0x1  << 4)	/* RW */
#define MSDC_CFG_BV18SDT        (0x1  << 5)	/* RW */
#define MSDC_CFG_BV18PSS        (0x1  << 6)	/* R  */
#define MSDC_CFG_CKSTB          (0x1  << 7)	/* R  */
#define MSDC_CFG_CKDIV          (0xff << 8)	/* RW */
#define MSDC_CFG_CKMOD          (0x3  << 16)	/* RW */

/* MSDC_IOCON mask */
#define MSDC_IOCON_SDR104CKS    (0x1  << 0)	/* RW */
#define MSDC_IOCON_RSPL         (0x1  << 1)	/* RW */
#define MSDC_IOCON_DSPL         (0x1  << 2)	/* RW */
#define MSDC_IOCON_DDLSEL       (0x1  << 3)	/* RW */
#define MSDC_IOCON_DDR50CKD     (0x1  << 4)	/* RW */
#define MSDC_IOCON_DSPLSEL      (0x1  << 5)	/* RW */
#define MSDC_IOCON_W_D_SMPL     (0x1  << 8)	/* RW */
#define MSDC_IOCON_D0SPL        (0x1  << 16)	/* RW */
#define MSDC_IOCON_D1SPL        (0x1  << 17)	/* RW */
#define MSDC_IOCON_D2SPL        (0x1  << 18)	/* RW */
#define MSDC_IOCON_D3SPL        (0x1  << 19)	/* RW */
#define MSDC_IOCON_D4SPL        (0x1  << 20)	/* RW */
#define MSDC_IOCON_D5SPL        (0x1  << 21)	/* RW */
#define MSDC_IOCON_D6SPL        (0x1  << 22)	/* RW */
#define MSDC_IOCON_D7SPL        (0x1  << 23)	/* RW */
#define MSDC_IOCON_RISCSZ       (0x3  << 24)	/* RW */

/* MSDC_PS mask */
#define MSDC_PS_CDEN            (0x1  << 0)	/* RW */
#define MSDC_PS_CDSTS           (0x1  << 1)	/* R  */
#define MSDC_PS_CDDEBOUNCE      (0xf  << 12)	/* RW */
#define MSDC_PS_DAT             (0xff << 16)	/* R  */
#define MSDC_PS_CMD             (0x1  << 24)	/* R  */
#define MSDC_PS_WP              (0x1UL << 31)	/* R  */

/* MSDC_INT mask */
#define MSDC_INT_MMCIRQ         (0x1  << 0)	/* W1C */
#define MSDC_INT_CDSC           (0x1  << 1)	/* W1C */
#define MSDC_INT_ACMDRDY        (0x1  << 3)	/* W1C */
#define MSDC_INT_ACMDTMO        (0x1  << 4)	/* W1C */
#define MSDC_INT_ACMDCRCERR     (0x1  << 5)	/* W1C */
#define MSDC_INT_DMAQ_EMPTY     (0x1  << 6)	/* W1C */
#define MSDC_INT_SDIOIRQ        (0x1  << 7)	/* W1C */
#define MSDC_INT_CMDRDY         (0x1  << 8)	/* W1C */
#define MSDC_INT_CMDTMO         (0x1  << 9)	/* W1C */
#define MSDC_INT_RSPCRCERR      (0x1  << 10)	/* W1C */
#define MSDC_INT_CSTA           (0x1  << 11)	/* R */
#define MSDC_INT_XFER_COMPL     (0x1  << 12)	/* W1C */
#define MSDC_INT_DXFER_DONE     (0x1  << 13)	/* W1C */
#define MSDC_INT_DATTMO         (0x1  << 14)	/* W1C */
#define MSDC_INT_DATCRCERR      (0x1  << 15)	/* W1C */
#define MSDC_INT_ACMD19_DONE    (0x1  << 16)	/* W1C */
#define MSDC_INT_DMA_BDCSERR    (0x1  << 17)	/* W1C */
#define MSDC_INT_DMA_GPDCSERR   (0x1  << 18)	/* W1C */
#define MSDC_INT_DMA_PROTECT    (0x1  << 19)	/* W1C */

/* MSDC_INTEN mask */
#define MSDC_INTEN_MMCIRQ       (0x1  << 0)	/* RW */
#define MSDC_INTEN_CDSC         (0x1  << 1)	/* RW */
#define MSDC_INTEN_ACMDRDY      (0x1  << 3)	/* RW */
#define MSDC_INTEN_ACMDTMO      (0x1  << 4)	/* RW */
#define MSDC_INTEN_ACMDCRCERR   (0x1  << 5)	/* RW */
#define MSDC_INTEN_DMAQ_EMPTY   (0x1  << 6)	/* RW */
#define MSDC_INTEN_SDIOIRQ      (0x1  << 7)	/* RW */
#define MSDC_INTEN_CMDRDY       (0x1  << 8)	/* RW */
#define MSDC_INTEN_CMDTMO       (0x1  << 9)	/* RW */
#define MSDC_INTEN_RSPCRCERR    (0x1  << 10)	/* RW */
#define MSDC_INTEN_CSTA         (0x1  << 11)	/* RW */
#define MSDC_INTEN_XFER_COMPL   (0x1  << 12)	/* RW */
#define MSDC_INTEN_DXFER_DONE   (0x1  << 13)	/* RW */
#define MSDC_INTEN_DATTMO       (0x1  << 14)	/* RW */
#define MSDC_INTEN_DATCRCERR    (0x1  << 15)	/* RW */
#define MSDC_INTEN_ACMD19_DONE  (0x1  << 16)	/* RW */
#define MSDC_INTEN_DMA_BDCSERR  (0x1  << 17)	/* RW */
#define MSDC_INTEN_DMA_GPDCSERR (0x1  << 18)	/* RW */
#define MSDC_INTEN_DMA_PROTECT  (0x1  << 19)	/* RW */

/* MSDC_FIFOCS mask */
#define MSDC_FIFOCS_RXCNT       (0xff << 0)	/* R */
#define MSDC_FIFOCS_TXCNT       (0xff << 16)	/* R */
#define MSDC_FIFOCS_CLR         (0x1UL << 31)	/* RW */

/* SDC_CFG mask */
#define SDC_CFG_SDIOINTWKUP     (0x1  << 0)	/* RW */
#define SDC_CFG_INSWKUP         (0x1  << 1)	/* RW */
#define SDC_CFG_BUSWIDTH        (0x3  << 16)	/* RW */
#define SDC_CFG_SDIO            (0x1  << 19)	/* RW */
#define SDC_CFG_SDIOIDE         (0x1  << 20)	/* RW */
#define SDC_CFG_INTATGAP        (0x1  << 21)	/* RW */
#define SDC_CFG_DTOC            (0xffUL << 24)	/* RW */

/* SDC_CMD mask */
#define SDC_CMD_OPC             (0x3f << 0)	/* RW */
#define SDC_CMD_BRK             (0x1  << 6)	/* RW */
#define SDC_CMD_RSPTYP          (0x7  << 7)	/* RW */
#define SDC_CMD_DTYP            (0x3  << 11)	/* RW */
#define SDC_CMD_RW              (0x1  << 13)	/* RW */
#define SDC_CMD_STOP            (0x1  << 14)	/* RW */
#define SDC_CMD_GOIRQ           (0x1  << 15)	/* RW */
#define SDC_CMD_BLKLEN          (0xfff << 16)	/* RW */
#define SDC_CMD_AUTOCMD         (0x3  << 28)	/* RW */
#define SDC_CMD_VOLSWTH         (0x1  << 30)	/* RW */

/* SDC_STS mask */
#define SDC_STS_SDCBUSY         (0x1  << 0)	/* RW */
#define SDC_STS_CMDBUSY         (0x1  << 1)	/* RW */
#define SDC_STS_SWR_COMPL       (0x1UL << 31)	/* RW */

/* SDC_DCRC_STS mask */
#define SDC_DCRC_STS_POS        (0xff << 0)	/* RO */
#define SDC_DCRC_STS_NEG        (0xff << 8)	/* RO */

/* EMMC_CFG0 mask */
#define EMMC_CFG0_BOOTSTART     (0x1  << 0)	/* W */
#define EMMC_CFG0_BOOTSTOP      (0x1  << 1)	/* W */
#define EMMC_CFG0_BOOTMODE      (0x1  << 2)	/* RW */
#define EMMC_CFG0_BOOTACKDIS    (0x1  << 3)	/* RW */
#define EMMC_CFG0_BOOTWDLY      (0x7  << 12)	/* RW */
#define EMMC_CFG0_BOOTSUPP      (0x1  << 15)	/* RW */

/* EMMC_CFG1 mask */
#define EMMC_CFG1_BOOTDATTMC    (0xfffff << 0)	/* RW */
#define EMMC_CFG1_BOOTACKTMC    (0xfffUL << 20)	/* RW */

/* EMMC_STS mask */
#define EMMC_STS_BOOTCRCERR     (0x1  << 0)	/* W1C */
#define EMMC_STS_BOOTACKERR     (0x1  << 1)	/* W1C */
#define EMMC_STS_BOOTDATTMO     (0x1  << 2)	/* W1C */
#define EMMC_STS_BOOTACKTMO     (0x1  << 3)	/* W1C */
#define EMMC_STS_BOOTUPSTATE    (0x1  << 4)	/* R */
#define EMMC_STS_BOOTACKRCV     (0x1  << 5)	/* W1C */
#define EMMC_STS_BOOTDATRCV     (0x1  << 6)	/* R */

/* EMMC_IOCON mask */
#define EMMC_IOCON_BOOTRST      (0x1  << 0)	/* RW */

/* SDC_ACMD19_TRG mask */
#define SDC_ACMD19_TRG_TUNESEL  (0xf  << 0)	/* RW */

/* MSDC_DMA_CTRL mask */
#define MSDC_DMA_CTRL_START     (0x1  << 0)	/* W */
#define MSDC_DMA_CTRL_STOP      (0x1  << 1)	/* W */
#define MSDC_DMA_CTRL_RESUME    (0x1  << 2)	/* W */
#define MSDC_DMA_CTRL_MODE      (0x1  << 8)	/* RW */
#define MSDC_DMA_CTRL_LASTBUF   (0x1  << 10)	/* RW */
#define MSDC_DMA_CTRL_BRUSTSZ   (0x7  << 12)	/* RW */
#ifndef CONFIG_ARCH_MT8135
#define MSDC_DMA_CTRL_XFERSZ    (0xffffUL << 16)	/* RW */
#endif

/* MSDC_DMA_CFG mask */
#define MSDC_DMA_CFG_STS        (0x1  << 0)	/* R */
#define MSDC_DMA_CFG_DECSEN     (0x1  << 1)	/* RW */
#define MSDC_DMA_CFG_AHBHPROT2  (0x2  << 8)	/* RW */
#define MSDC_DMA_CFG_ACTIVEEN   (0x2  << 12)	/* RW */
#define MSDC_DMA_CFG_CS12B16B   (0x1  << 16)	/* RW */

/* MSDC_PATCH_BIT mask */
#define MSDC_PATCH_BIT_ODDSUPP    (0x1  <<  1)	/* RW */
#define MSDC_INT_DAT_LATCH_CK_SEL (0x7  <<  7)
#define MSDC_CKGEN_MSDC_DLY_SEL   (0x1F << 10)
#define MSDC_PATCH_BIT_IODSSEL    (0x1  << 16)	/* RW */
#define MSDC_PATCH_BIT_IOINTSEL   (0x1  << 17)	/* RW */
#define MSDC_PATCH_BIT_BUSYDLY    (0xf  << 18)	/* RW */
#define MSDC_PATCH_BIT_WDOD       (0xf  << 22)	/* RW */
#define MSDC_PATCH_BIT_IDRTSEL    (0x1  << 26)	/* RW */
#define MSDC_PATCH_BIT_CMDFSEL    (0x1  << 27)	/* RW */
#define MSDC_PATCH_BIT_INTDLSEL   (0x1  << 28)	/* RW */
#define MSDC_PATCH_BIT_SPCPUSH    (0x1  << 29)	/* RW */
#define MSDC_PATCH_BIT_DECRCTMO   (0x1  << 30)	/* RW */

/* MSDC_PATCH_BIT1 mask */
#define MSDC_PATCH_BIT1_WRDAT_CRCS  (0x7 << 0)
#define MSDC_PATCH_BIT1_CMD_RSP     (0x7 << 3)

/* MSDC_PAD_CTL0 mask */
#define MSDC_PAD_CTL0_CLKDRVN   (0x7  << 0)	/* RW */
#define MSDC_PAD_CTL0_CLKDRVP   (0x7  << 4)	/* RW */
#define MSDC_PAD_CTL0_CLKSR     (0x1  << 8)	/* RW */
#define MSDC_PAD_CTL0_CLKPD     (0x1  << 16)	/* RW */
#define MSDC_PAD_CTL0_CLKPU     (0x1  << 17)	/* RW */
#define MSDC_PAD_CTL0_CLKSMT    (0x1  << 18)	/* RW */
#define MSDC_PAD_CTL0_CLKIES    (0x1  << 19)	/* RW */
#define MSDC_PAD_CTL0_CLKTDSEL  (0xf  << 20)	/* RW */
#define MSDC_PAD_CTL0_CLKRDSEL  (0xffUL << 24)	/* RW */

/* MSDC_PAD_CTL1 mask */
#define MSDC_PAD_CTL1_CMDDRVN   (0x7  << 0)	/* RW */
#define MSDC_PAD_CTL1_CMDDRVP   (0x7  << 4)	/* RW */
#define MSDC_PAD_CTL1_CMDSR     (0x1  << 8)	/* RW */
#define MSDC_PAD_CTL1_CMDPD     (0x1  << 16)	/* RW */
#define MSDC_PAD_CTL1_CMDPU     (0x1  << 17)	/* RW */
#define MSDC_PAD_CTL1_CMDSMT    (0x1  << 18)	/* RW */
#define MSDC_PAD_CTL1_CMDIES    (0x1  << 19)	/* RW */
#define MSDC_PAD_CTL1_CMDTDSEL  (0xf  << 20)	/* RW */
#define MSDC_PAD_CTL1_CMDRDSEL  (0xffUL << 24)	/* RW */

/* MSDC_PAD_CTL2 mask */
#define MSDC_PAD_CTL2_DATDRVN   (0x7  << 0)	/* RW */
#define MSDC_PAD_CTL2_DATDRVP   (0x7  << 4)	/* RW */
#define MSDC_PAD_CTL2_DATSR     (0x1  << 8)	/* RW */
#define MSDC_PAD_CTL2_DATPD     (0x1  << 16)	/* RW */
#define MSDC_PAD_CTL2_DATPU     (0x1  << 17)	/* RW */
#define MSDC_PAD_CTL2_DATIES    (0x1  << 19)	/* RW */
#define MSDC_PAD_CTL2_DATSMT    (0x1  << 18)	/* RW */
#define MSDC_PAD_CTL2_DATTDSEL  (0xf  << 20)	/* RW */
#define MSDC_PAD_CTL2_DATRDSEL  (0xffUL << 24)	/* RW */

/* MSDC_PAD_TUNE mask */
#define MSDC_PAD_TUNE_DATWRDLY  (0x1F << 0)	/* RW */
#define MSDC_PAD_TUNE_DATRRDLY  (0x1F << 8)	/* RW */
#define MSDC_PAD_TUNE_CMDRDLY   (0x1F << 16)	/* RW */
#define MSDC_PAD_TUNE_CMDRRDLY  (0x1FUL << 22)	/* RW */
#define MSDC_PAD_TUNE_CLKTXDLY  (0x1FUL << 27)	/* RW */

/* MSDC_DAT_RDDLY0/1 mask */
#define MSDC_DAT_RDDLY0_D3      (0x1F << 0)	/* RW */
#define MSDC_DAT_RDDLY0_D2      (0x1F << 8)	/* RW */
#define MSDC_DAT_RDDLY0_D1      (0x1F << 16)	/* RW */
#define MSDC_DAT_RDDLY0_D0      (0x1FUL << 24)	/* RW */

#define MSDC_DAT_RDDLY1_D7      (0x1F << 0)	/* RW */
#define MSDC_DAT_RDDLY1_D6      (0x1F << 8)	/* RW */
#define MSDC_DAT_RDDLY1_D5      (0x1F << 16)	/* RW */
#define MSDC_DAT_RDDLY1_D4      (0x1FUL << 24)	/* RW */

/********************MSDC0*************************************************/
#define MSDC0_TDSEL_BASE		(GPIO_BASE+0x0700)
#define MSDC0_RDSEL_BASE		(GPIO_BASE+0x0700)
#define MSDC0_TDSEL				(0xF << 0)
#define MSDC0_RDSEL				(0x3FUL << 4)
#define MSDC0_DAT_DRVING_BASE	(GPIO_BASE+0x0500)
#define MSDC0_DAT_DRVING		(0x7 << 0)
#define MSDC0_CMD_DRVING_BASE	(GPIO_BASE+0x0500)
#define MSDC0_CMD_DRVING		(0x7 << 4)
#define MSDC0_CLK_DRVING_BASE	(GPIO_BASE+0x0500)
#define MSDC0_CLK_DRVING		(0x7 << 8)
#define MSDC0_DAT_SR_BASE		(GPIO_BASE+0x0500)
#define MSDC0_DAT_SR			(0x1 << 3)
#define MSDC0_CMD_SR_BASE		(GPIO_BASE+0x0500)
#define MSDC0_CMD_SR			(0x1 << 7)
#define MSDC0_CLK_SR_BASE		(GPIO_BASE+0x0500)
#define MSDC0_CLK_SR			(0x1 << 11)
#define MSDC0_IES_BASE			(GPIO_BASE+0x0100)
#define MSDC0_IES_DAT			(0x1 << 9)
#define MSDC0_IES_CMD			(0x1 << 4)
#define MSDC0_IES_CLK			(0x1 << 5)

#define MSDC0_SMT_BASE			(GPIO_BASE+0x0300)
/* MASK:DAT0|DAT1|DAT2|DAT3|CLK|CMD|RSTB(Please use default value)|DAT4|DAT5| */
/* DAT6|DAT7 */
#define MSDC0_SMT_DAT			(0x3CF << 0)
#define MSDC0_SMT_CMD			(0x1 << 4)
#define MSDC0_SMT_CLK			(0x1 << 5)

/* 1.8v 10K resistor control */
#define MSDC0_R0_BASE			(GPIO1_BASE+0x04D0)
/* MASK:DAT7|DAT6|DAT5|DAT4|DAT3|DAT2|DAT1|DAT0|CMD|CLK */
#define MSDC0_R0_DAT			(0xFF << 2)
#define MSDC0_R0_CMD			(0x1 << 1)
#define MSDC0_R0_CLK			(0x1 << 0)

#define MSDC0_R1_BASE			(GPIO_BASE+0x0200)
/* MASK:DAT0|DAT1|DAT2|DAT3|CLK|CMD|RSTB(Please use default value)|DAT4|DAT5| */
/* DAT6|DAT7 */
#define MSDC0_R1_DAT			(0x3CF << 0)
#define MSDC0_R1_CMD			(0x1 << 4)
#define MSDC0_R1_CLK			(0x1 << 5)

/* '1' = pull up '0' =pull down */
#define MSDC0_PUPD_BASE			(GPIO_BASE+0x0400)
/* MASK:DAT0|DAT1|DAT2|DAT3|CLK|CMD|RSTB(Please use default value)|DAT4|DAT5| */
/* DAT6|DAT7 */
#define MSDC0_PUPD_DAT			(0x3CF << 0)
#define MSDC0_PUPD_CMD			(0x1 << 4)
#define MSDC0_PUPD_CLK			(0x1 << 5)

/****************************MSDC1*********************************************/

#define MSDC1_TDSEL_BASE		(GPIO_BASE+0x0780)
#define MSDC1_RDSEL_BASE		(GPIO_BASE+0x0780)
#define MSDC1_TDSEL			(0xFUL  << 16)
#define MSDC1_RDSEL			(0x3FUL << 24)

#define MSDC1_DAT_DRVING_BASE		(GPIO_BASE+0x05A0)
#define MSDC1_DAT_DRVING		(0x7UL  << 20)

#define MSDC1_CMD_DRVING_BASE		(GPIO_BASE+0x05A0)
#define MSDC1_CMD_DRVING		(0x7UL  << 24)

#define MSDC1_CLK_DRVING_BASE		(GPIO_BASE+0x05C0)
#define MSDC1_CLK_DRVING		(0x7 << 16)

#define MSDC1_DAT_SR_BASE		(GPIO_BASE+0x05A0)
#define MSDC1_DAT_SR			(0x1 << 23)
#define MSDC1_CMD_SR_BASE		(GPIO_BASE+0x05A0)
#define MSDC1_CMD_SR			(0x1 << 27)
#define MSDC1_CLK_SR_BASE		(GPIO_BASE+0x05C0)
#define MSDC1_CLK_SR			(0x1 << 19)

#define MSDC1_IES_BASE			(GPIO_BASE+0x01B0)
#define MSDC1_IES_DAT			(0x1 << 4)
#define MSDC1_IES_CMD			(0x1 << 7)
#define MSDC1_IES_CLK			(0x1 << 8)

#define MSDC1_SMT_BASE			(GPIO_BASE+0x03B0)

/* DATA3|DATA2...DATA1|DATA0 */
#define MSDC1_SMT_DAT			((0x3 << 9) | (0x3 << 4))
#define MSDC1_SMT_CMD			(0x1 << 7)
#define MSDC1_SMT_CLK			(0x1 << 8)

/* '1'= enable '0' = disable */
#define MSDC1_PUPD_ENABLE_BASE			(GPIO_BASE+0x02B0)
/* '1' = pull up '0' =pull down */
#define MSDC1_PUPD_POLARITY_BASE		(GPIO_BASE+0x04B0)

/* DATA3|DATA2...DATA1|DATA0 */
#define MSDC1_PUPD_DAT			((0x3 << 9) | (0x3 << 4))
#define MSDC1_PUPD_CMD			(0x1 << 7)
#define MSDC1_PUPD_CLK			(0x1 << 8)

/****************************MSDC2*********************************************/

#define MSDC2_TDSEL_BASE		(GPIO_BASE+0x0780)
#define MSDC2_RDSEL_BASE		(GPIO_BASE+0x0780)
#define MSDC2_TDSEL			(0xFUL  << 0)
#define MSDC2_RDSEL			(0x3FUL << 8)

#define MSDC2_DAT_DRVING_BASE		(GPIO_BASE+0x05A0)
#define MSDC2_DAT_DRVING		(0x7	<< 8)

#define MSDC2_CMD_DRVING_BASE		(GPIO_BASE+0x05A0)
#define MSDC2_CMD_DRVING		(0x7	<< 12)

#define MSDC2_CLK_DRVING_BASE		(GPIO_BASE+0x05C0)
#define MSDC2_CLK_DRVING		(0x7UL	<< 20)

#define MSDC2_DAT_SR_BASE		(GPIO_BASE+0x05A0)
#define MSDC2_DAT_SR			(0x1 << 11)
#define MSDC2_CMD_SR_BASE		(GPIO_BASE+0x05A0)
#define MSDC2_CMD_SR			(0x1 << 15)
#define MSDC2_CLK_SR_BASE		(GPIO_BASE+0x05C0)
#define MSDC2_CLK_SR			(0x1 << 23)

#define MSDC2_IES_BASE			(GPIO_BASE+0x01B0)
#define MSDC2_IES_DAT			(0x1 << 3)
#define MSDC2_IES_CMD			(0x1 << 0)
#define MSDC2_IES_CLK			(0x1 << 1)

#define MSDC2_SMT_BASE1			(GPIO_BASE+0x03B0)
#define MSDC2_SMT_DAT1_0		(0x3 << 2)	/* ...DAT1|DAT0... */
#define MSDC2_SMT_CMD			(0x1 << 0)
#define MSDC2_SMT_CLK			(0x1 << 1)
#define MSDC2_SMT_BASE2			(GPIO_BASE+0x03A0)
#define MSDC2_SMT_DAT2_3		(0x3 << 14)	/* ...DAT2|DAT3... */

/* '1'= enable '0' = disable */
#define MSDC2_PUPD_ENABLE_BASE1		(GPIO_BASE+0x02B0)
/* '1' = pull up '0' =pull down */
#define MSDC2_PUPD_POLARITY_BASE1	(GPIO_BASE+0x04B0)
#define MSDC2_PUPD_DAT1_0		(0x3 << 2)	/* ...DAT1|DAT0... */
#define MSDC2_PUPD_CMD			(0x1 << 0)
#define MSDC2_PUPD_CLK			(0x1 << 1)

/* '1'= enable '0' = disable */
#define MSDC2_PUPD_ENABLE_BASE2			(GPIO_BASE+0x02A0)
/* '1' = pull up '0' =pull down */
#define MSDC2_PUPD_POLARITY_BASE2		(GPIO_BASE+0x04A0)
#define MSDC2_PUPD_DAT2_3		(0x3 << 14)

/****************************MSDC3*********************************************/

#define MSDC3_TDSEL_BASE		(GPIO_BASE+0x0790)
#define MSDC3_RDSEL_BASE		(GPIO_BASE+0x0790)
#define MSDC3_TDSEL			(0xFUL  << 16)
#define MSDC3_RDSEL			(0x3FUL << 24)

#define MSDC3_DAT_DRVING_BASE		(GPIO_BASE+0x05C0)
#define MSDC3_DAT_DRVING		(0x7	<< 4)

#define MSDC3_CMD_DRVING_BASE		(GPIO_BASE+0x05C0)
#define MSDC3_CMD_DRVING		(0x7    << 8)

#define MSDC3_CLK_DRVING_BASE		(GPIO_BASE+0x05C0)
#define MSDC3_CLK_DRVING		(0x7UL  << 24)

#define MSDC3_DAT_SR_BASE		(GPIO_BASE+0x05C0)
#define MSDC3_DAT_SR			(0x1 << 7)
#define MSDC3_CMD_SR_BASE		(GPIO_BASE+0x05C0)
#define MSDC3_CMD_SR			(0x1 << 11)
#define MSDC3_CLK_SR_BASE		(GPIO_BASE+0x05C0)
#define MSDC3_CLK_SR			(0x1 << 27)

#define MSDC3_IES_BASE			(GPIO_BASE+0x01E0)
#define MSDC3_IES_DAT			(0x1 << 7)
#define MSDC3_IES_CMD			(0x1 << 4)
#define MSDC3_IES_CLK			(0x1 << 5)

#define MSDC3_SMT_BASE			(GPIO_BASE+0x03E0)

/* ..DAT0|DAT1..DAT3|DAT2 */
#define MSDC3_SMT_DAT			((0x3 << 6) | (0x3 << 2))
#define MSDC3_SMT_CMD			(0x1 << 4)
#define MSDC3_SMT_CLK			(0x1 << 5)

/* 1.8v 10K resistor control */
#define MSDC3_R0_BASE			(GPIO_BASE+0x04F0)

/* MASK:DAT3|DAT2|DAT1|DAT0 */
#define MSDC3_R0_DAT			(0xF << 12)
#define MSDC3_R0_CMD			(0x1 << 11)
#define MSDC3_R0_CLK			(0x1 << 10)

#define MSDC3_R1_BASE			(GPIO_BASE+0x02E0)
/* MASK:DAT0|DAT1...DAT3|DAT2 */
#define MSDC3_R1_DAT			((0x3 << 6) | (0x3 << 2))
#define MSDC3_R1_CMD			(0x1 << 4)
#define MSDC3_R1_CLK			(0x1 << 5)

/* '1' = pull up '0' =pull down */
#define MSDC3_PUPD_BASE			(GPIO_BASE+0x04E0)
/* MASK:DAT0|DAT1...DAT3|DAT2 */
#define MSDC3_PUPD_DAT			((0x3 << 6) | (0x3 << 2))
#define MSDC3_PUPD_CMD			(0x1 << 4)
#define MSDC3_PUPD_CLK			(0x1 << 5)

/********************MSDC4*************************************************/
#define	GPIO_BASE2			(0x1020C000)
#define MSDC4_TDSEL_BASE		(GPIO_BASE2+0x0760)
#define MSDC4_RDSEL_BASE		(GPIO_BASE2+0x0770)
#define MSDC4_TDSEL			(0xFUL << 24)
#define MSDC4_RDSEL			(0x3F  << 0)
#define MSDC4_DAT_DRVING_BASE		(GPIO_BASE2+0x0580)
#define MSDC4_DAT_DRVING		(0x7 << 20)
#define MSDC4_CMD_DRVING_BASE		(GPIO_BASE2+0x0570)
#define MSDC4_CMD_DRVING		(0x7 << 16)
#define MSDC4_CLK_DRVING_BASE		(GPIO_BASE2+0x0580)
#define MSDC4_CLK_DRVING		(0x7 << 0)
#define MSDC4_DAT_SR_BASE		(GPIO_BASE2+0x0580)
#define MSDC4_DAT_SR			(0x1 << 23)
#define MSDC4_CMD_SR_BASE		(GPIO_BASE2+0x0570)
#define MSDC4_CMD_SR			(0x1 << 19)
#define MSDC4_CLK_SR_BASE		(GPIO_BASE2+0x0580)
#define MSDC4_CLK_SR			(0x1 << 3)
#define MSDC4_IES_BASE			(GPIO_BASE2+0x0180)
#define MSDC4_IES_DAT			(0x1 << 2)
#define MSDC4_IES_CMD			(0x1 << 13)
#define MSDC4_IES_CLK			(0x1 << 11)

#define MSDC4_SMT_BASE			(GPIO_BASE2+0x0380)
/* MASK:DAT3..DAT2|DAT4|DAT7|DAT6|DAT5|DAT1|DAT0... */
#define MSDC4_SMT_DAT			((0x1 << 12) | (0x1F << 6) | (0x3 << 2))
#define MSDC4_SMT_CMD			(0x1 << 13)
#define MSDC4_SMT_CLK			(0x1 << 11)

/* 1.8v 10K resistor control */
#define MSDC4_R0_BASE			(GPIO_BASE+0x04F0)
/* MASK:DAT7|DAT6|DAT5|DAT4|DAT3|DAT2|DAT1|DAT0|CMD|CLK */
#define MSDC4_R0_DAT			(0xFFUL << 18)
#define MSDC4_R0_CMD			(0x1 << 17)
#define MSDC4_R0_CLK			(0x1 << 16)

#define MSDC4_R1_BASE			(GPIO_BASE2+0x0280)
/* MASK:DAT3..DAT2|DAT4|DAT7|DAT6|DAT5|DAT1|DAT0... */
#define MSDC4_R1_DAT			((0x1 << 12) | (0x1F << 6) | (0x3 << 2))
#define MSDC4_R1_CMD			(0x1 << 13)
#define MSDC4_R1_CLK			(0x1 << 11)

/* '1' = pull up '0' =pull down */
#define MSDC4_PUPD_BASE			(GPIO_BASE2+0x0480)
/* MASK:DAT3..DAT2|DAT4|DAT7|DAT6|DAT5|DAT1|DAT0... */
#define MSDC4_PUPD_DAT			((0x1 << 12) | (0x1F << 6) | (0x3 << 2))
#define MSDC4_PUPD_CMD			(0x1 << 13)
#define MSDC4_PUPD_CLK			(0x1 << 11)

#define EN18IOKEY_BASE			(GPIO1_BASE+0x920)

#define MSDC_EN18IO_CMP_SEL_BASE	(GPIO1_BASE+0x910)
#define	MSDC1_EN18IO_CMP_EN		(0x1 << 3)
#define MSDC1_EN18IO_SEL1		(0x7 << 0)
#define	MSDC2_EN18IO_CMP_EN		(0x1 << 7)
#define MSDC2_EN18IO_SEL		(0x7 << 4)

#define MSDC_EN18IO_SW_BASE		(GPIO_BASE+0x900)
#define MSDC1_EN18IO_SW			(0x1 << 19)
#define MSDC2_EN18IO_SW			(0x1 << 25)

typedef enum MSDC_POWER {

	MSDC_VIO18_MC1 = 0,
	MSDC_VIO18_MC2,
	MSDC_VIO28_MC1,
	MSDC_VIO28_MC2,
	MSDC_VMC,
	MSDC_VGP,
	MSDC_VEMC_3V3,
} MSDC_POWER_DOMAIN;

/*--------------------------------------------------------------------------*/
/* Descriptor Structure                                                     */
/*--------------------------------------------------------------------------*/
typedef struct {
	uint32_t hwo:1;		/* could be changed by hw */
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

/*--------------------------------------------------------------------------*/
/* Register Debugging Structure                                             */
/*--------------------------------------------------------------------------*/

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

#define DMA_FLAG_NONE       (0x00000000)
#define DMA_FLAG_EN_CHKSUM  (0x00000001)
#define DMA_FLAG_PAD_BLOCK  (0x00000002)
#define DMA_FLAG_PAD_DWORD  (0x00000004)

struct dma_config {
	u32 flags;		/* flags */
	u32 xfersz;		/* xfer size in bytes */
	u32 sglen;		/* size of scatter list */
	u32 blklen;		/* block size */
	struct scatterlist *sg;	/* I/O scatter list */
	struct scatterlist_ex *esg;	/* extended I/O scatter list */
	u8 mode;		/* dma mode        */
	u8 burstsz;		/* burst size      */
	u8 intr;		/* dma done interrupt */
	u8 padding;		/* padding */
	u32 cmd;		/* enhanced mode command */
	u32 arg;		/* enhanced mode arg */
	u32 rsp;		/* enhanced mode command response */
	u32 autorsp;		/* auto command response */
};

#if MSDC_USE_REG_OPS_DUMP
static void reg32_write(volatile uint32_t *addr, uint32_t data)
{
	*addr = (uint32_t)data;
	printf("[WR32] %x = %x\n", addr, data);
}

static uint32_t reg32_read(volatile uint32_t *addr)
{
	uint32_t data = *(volatile uint32_t *)(addr);

	printf("[RD32] %x = %x\n", addr, data);
	return data;
}

static void reg16_write(volatile uint32_t *addr, uint16_t data)
{
	*(volatile uint16_t *)(addr) = data;
	printf("[WR16] %x = %x\n", addr, data);
}

static uint16_t reg16_read(volatile uint32_t *addr)
{
	uint16_t data = *(volatile uint16_t *)addr;

	printf("[RD16] %x = %x\n", addr, data);
	return data;
}

static void reg8_write(volatile uint32_t *addr, uint8_t data)
{
	*(volatile uint8_t *)(addr) = data;
	printf("[WR8] %x = %x\n", addr, data);
}

static uint8_t reg8_read(volatile uint32_t *addr)
{
	uint8_t data = *(volatile uint8_t *)addr;

	printf("[RD8] %x = %x\n", addr, data);
	return data;
}

#define MSDC_WRITE32(addr, data)      reg32_write((volatile uint32_t *)addr, data)
#define MSDC_READ32(addr)            reg32_read((volatile uint32_t *)addr)
#define MSDC_WRITE16(addr, data)      reg16_write((volatile uint32_t *)addr, data)
#define MSDC_READ16(addr)            reg16_read((volatile uint32_t *)addr)
#define MSDC_WRITE8(addr, data)      reg8_write((volatile uint32_t *)addr, data)
#define MSDC_READ8(addr)             reg8_read((volatile uint32_t *)addr)
#define MSDC_SET_BIT32(addr, mask)	\
	do { \
		(*(volatile uint32_t *)(addr) |= (mask)); \
		printk(BIOS_INFO, "[SET32] %x |= %x\n", addr, mask); \
	} while (0)
#define MSDC_CLR_BIT32(addr, mask)	\
	do { \
		(*(volatile uint32_t *)(addr) &= ~(mask)); \
		printk(BIOS_INFO, "[CLR32] %x &= ~%x\n", addr, mask); \
	} while (0)
#define MSDC_SET_BIT16(addr, mask)	\
	do { \
		(*(volatile uint16_t *)(addr) |= (mask)); \
		printk(BIOS_INFO, "[SET16] %x |= %x\n", addr, mask); \
	} while (0)
#define MSDC_CLR_BIT16(addr, mask)	\
	do { \
		(*(volatile uint16_t *)(addr) &= ~(mask)); \
		printk(BIOS_INFO, "[CLR16] %x &= ~%x\n", addr, mask); \
	} while (0)
#else
#define MSDC_WRITE32(addr, data)		(*(volatile uint32_t *)(addr) = (uint32_t)(data))
#define MSDC_READ32(addr)		(*(volatile uint32_t *)(addr))
#define MSDC_WRITE16(addr, data)		(*(volatile uint16_t *)(addr) = (uint16_t)(data))
#define MSDC_READ16(addr)		(*(volatile uint16_t *)(addr))
#define MSDC_WRITE8(addr, data)		(*(volatile uint8_t *)(addr)  = (uint8_t)(data))
#define MSDC_READ8(addr)		(*(volatile uint8_t *)(addr))
#define MSDC_SET_BIT32(addr, mask)	(*(volatile uint32_t *)(addr) |= (mask))
#define MSDC_CLR_BIT32(addr, mask)	(*(volatile uint32_t *)(addr) &= ~(mask))
#define MSDC_SET_BIT16(addr, mask)	(*(volatile uint16_t *)(addr) |= (mask))
#define MSDC_CLR_BIT16(addr, mask)	(*(volatile uint16_t *)(addr) &= ~(mask))
#endif

#define MSDC_SET_FIELD(reg, field, val) set_field(reg, field, val)
#define MSDC_GET_FIELD(reg, field, val) get_field(reg, field, val)
#define MSDC_SET_FIELD_DISCRETE(reg, field, val) \
	do {	\
		unsigned int tv = (unsigned int)(*(volatile u32 *)(reg)); \
		tv = (val == 1) ? (tv|(field)) : (tv & ~(field));\
		(*(volatile u32 *)(reg) = (u32)(tv)); \
	} while (0)

#define MSDC_GET_FIELD_DISCRETE(reg, field, val) \
	do {	\
		unsigned int tv = (unsigned int)(*(volatile u32 *)(reg)); \
		val = tv & (field) ; \
		val = (val != 0) ? 1 : 0;\
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

#define MSDC_RESET() \
	do { \
		MSDC_SET_BIT32(MSDC_CFG, MSDC_CFG_RST); \
		MSDC_RETRY(MSDC_READ32(MSDC_CFG) & MSDC_CFG_RST, 5, 1000); \
	} while (0)

#define MSDC_CLR_INT() \
	do { \
		volatile uint32_t val = MSDC_READ32(MSDC_INT); \
		MSDC_WRITE32(MSDC_INT, val); \
		if (MSDC_READ32(MSDC_INT)) { \
			MSG(ERR, "[ASSERT] MSDC_INT is NOT clear\n"); \
		} \
	} while (0)

#define MSDC_CLR_FIFO() \
	do { \
		MSDC_SET_BIT32(MSDC_FIFOCS, MSDC_FIFOCS_CLR); \
		MSDC_RETRY(MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_CLR, 5, 1000); \
	} while (0)

#define MSDC_FIFO_WRITE32(val)  MSDC_WRITE32(MSDC_TXDATA, val)
#define MSDC_FIFO_READ32()      MSDC_READ32(MSDC_RXDATA)
#define MSDC_FIFO_WRITE16(val)  MSDC_WRITE16(MSDC_TXDATA, val)
#define MSDC_FIFO_READ16()      MSDC_READ16(MSDC_RXDATA)
#define MSDC_FIFO_WRITE8(val)   MSDC_WRITE8(MSDC_TXDATA, val)
#define MSDC_FIFO_READ8()       MSDC_READ8(MSDC_RXDATA)

#define MSDC_FIFO_WRITE(val)	MSDC_FIFO_WRITE32(val)
#define MSDC_FIFO_READ()	MSDC_FIFO_READ32()

#define MSDC_TXFIFOCNT() \
    ((MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_TXCNT) >> 16)
#define MSDC_RXFIFOCNT() \
    ((MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_RXCNT) >> 0)

#define MSDC_CARD_DETECTION_ON()  MSDC_SET_BIT32(MSDC_PS, MSDC_PS_CDEN)
#define MSDC_CARD_DETECTION_OFF() MSDC_CLR_BIT32(MSDC_PS, MSDC_PS_CDEN)

#define MSDC_DMA_ON()   MSDC_CLR_BIT32(MSDC_CFG, MSDC_CFG_PIO)
#define MSDC_DMA_OFF()  MSDC_SET_BIT32(MSDC_CFG, MSDC_CFG_PIO)

#define SDC_IS_BUSY()		(MSDC_READ32(SDC_STS) & SDC_STS_SDCBUSY)
#define SDC_IS_CMD_BUSY()	(MSDC_READ32(SDC_STS) & SDC_STS_CMDBUSY)

#define SDC_SEND_CMD(cmd, arg) \
	do { \
		MSDC_WRITE32(SDC_ARG, (arg)); \
		MSDC_WRITE32(SDC_CMD, (cmd)); \
	} while (0)

#define MSDC_INIT_GPD_EX(gpd, extlen, cmd, arg, blknum) \
	do { \
		((gpd_t *)gpd)->extlen = extlen; \
		((gpd_t *)gpd)->cmd    = cmd; \
		((gpd_t *)gpd)->arg    = arg; \
		((gpd_t *)gpd)->blknum = blknum; \
	} while (0)

#define MSDC_INIT_BD(bd, blkpad, dwpad, dptr, dlen) \
	do { \
		BUG_ON(dlen > 0xFFFFUL); \
		((bd_t *)bd)->blkpad = blkpad; \
		((bd_t *)bd)->dwpad  = dwpad; \
		((bd_t *)bd)->ptr    = (void *)dptr; \
		((bd_t *)bd)->buflen = dlen; \
	} while (0)

#ifdef MMC_PROFILING
static inline void msdc_timer_init(void)
{
	/* clear. CLR[1]=1, EN[0]=0 */
	__raw_writel(0x0, GPT_BASE + 0x30);
	__raw_writel(0x2, GPT_BASE + 0x30);

	__raw_writel(0, GPT_BASE + 0x38);
	__raw_writel(32768, GPT_BASE + 0x3C);

	/* 32678HZ RTC free run */
	__raw_writel(0x30, GPT_BASE + 0x34);
	__raw_writel(0x32, GPT_BASE + 0x30);
}

static inline void msdc_timer_start(void)
{
	*(volatile unsigned int *)(GPT_BASE + 0x30) |= (1 << 0);
}

static inline void msdc_timer_stop(void)
{
	*(volatile unsigned int *)(GPT_BASE + 0x30) &= ~(1 << 0);
}

static inline void msdc_timer_stop_clear(void)
{
	*(volatile unsigned int *)(GPT_BASE + 0x30) &= ~(1 << 0); /* stop  */
	*(volatile unsigned int *)(GPT_BASE + 0x30) |= (1 << 1);  /* clear */
}

static inline unsigned int msdc_timer_get_count(void)
{
	return __raw_readl(GPT_BASE + 0x38);
}
#else
#define msdc_timer_init()       do {} while (0)
#define msdc_timer_start()      do {} while (0)
#define msdc_timer_stop()       do {} while (0)
#define msdc_timer_stop_clear() do {} while (0)
#define msdc_timer_get_count()  0
#endif

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

#endif /* end of SOC_MEDIATEK_MT8135_MSDC_H */
