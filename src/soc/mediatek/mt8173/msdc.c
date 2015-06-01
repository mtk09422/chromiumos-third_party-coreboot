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
#include <console/console.h>
#include <string.h>

#include <soc/addressmap.h>
#include <soc/cust_msdc.h>
#include <soc/msdc.h>
#include <soc/msdc_utils.h>
#include <soc/mmc_core.h>
#include <soc/pmic_wrap_init.h>

/* Tuning Parameter */
#define DEFAULT_DEBOUNCE   (8)	/* 8 cycles */
#define DEFAULT_DTOC       (40)	/* data timeout counter. 65536x40 sclk. */
#define DEFAULT_WDOD       (0)	/* write data output delay. no delay. */
#define DEFAULT_BSYDLY     (8)	/* card busy delay. 8 extend sclk */

#define MAX_DMA_CNT        (32768)
#define MAX_SG_POOL_SZ     (MAX_BD_POOL_SZ)
#define MAX_SG_BUF_SZ      (MAX_DMA_CNT)
#define MAX_BD_PER_GPD     (MAX_BD_POOL_SZ/(MAX_GPD_POOL_SZ-1))	/* except null gpd */

#define MAX_DMA_TRAN_SIZE  (MAX_SG_POOL_SZ*MAX_SG_BUF_SZ)

#if MAX_SG_BUF_SZ > MAX_DMA_CNT
#error "incorrect max sg buffer size"
#endif

unsigned int g_crc_count = 0;
#define CRC_ERR_DUMP_NUM (5)

typedef struct {
	int autocmd;
	struct dma_config cfg;
	struct scatterlist sg[MAX_SG_POOL_SZ];
	int alloc_gpd;
	int alloc_bd;
	int dsmpl;
	int rsmpl;
	gpd_t *active_head;
	gpd_t *active_tail;
	gpd_t *gpd_pool;
	bd_t *bd_pool;
} msdc_priv_t;

static int msdc_rsp[] = {
	0,			/* RESP_NONE */
	1,			/* RESP_R1 */
	2,			/* RESP_R2 */
	3,			/* RESP_R3 */
	4,			/* RESP_R4 */
	1,			/* RESP_R5 */
	1,			/* RESP_R6 */
	1,			/* RESP_R7 */
	7,			/* RESP_R1b */
};

static msdc_priv_t msdc_priv[MSDC_MAX_NUM];
#if MSDC_USE_DMA_MODE
static gpd_t msdc_gpd_pool[MSDC_MAX_NUM][MAX_GPD_POOL_SZ];
static bd_t msdc_bd_pool[MSDC_MAX_NUM][MAX_BD_POOL_SZ];
#endif

#if IS_ENABLED(CONFIG_DEBUG_MSDC)
static void msdc_dump_padctl(struct mmc_host *host, u32 pin)
{
	switch(pin){
		case GPIO_CLK_CTRL:
			printk(BIOS_DEBUG, "  CLK_PAD_CTRL = 0x%x\n", MSDC_READ32(MSDC0_GPIO_CLK_BASE));
			break;
		case GPIO_CMD_CTRL:
			printk(BIOS_DEBUG, "  CMD_PAD_CTRL = 0x%x\n", MSDC_READ32(MSDC0_GPIO_CMD_BASE));
			break;
		case GPIO_DAT_CTRL:
			printk(BIOS_DEBUG, "  DAT_PAD_CTRL = 0x%x\n", MSDC_READ32(MSDC0_GPIO_DAT_BASE));
			break;
		case GPIO_RST_CTRL:
			printk(BIOS_DEBUG, "  RST_PAD_CTRL = 0x%x\n", MSDC_READ32(MSDC0_GPIO_RST_BASE));
			break;
		case GPIO_DS_CTRL:
			printk(BIOS_DEBUG, "  DS_PAD_CTRL = 0x%x\n", MSDC_READ32(MSDC0_GPIO_DS_BASE));
			break;
		case GPIO_MODE_CTRL:
			printk(BIOS_DEBUG, "  PAD_MODE_CTRL = 0x%x\n", MSDC_READ32(MSDC0_GPIO_MODE0_BASE));
			printk(BIOS_DEBUG, "  PAD_MODE_CTRL = 0x%x\n", MSDC_READ32(MSDC0_GPIO_MODE1_BASE));
			printk(BIOS_DEBUG, "  PAD_MODE_CTRL = 0x%x\n", MSDC_READ32(MSDC0_GPIO_MODE2_BASE));
		default:
			break;
	}
}

static void msdc_dump_register(struct mmc_host *host)
{
	u32 base = host->base;

	printk(BIOS_DEBUG, "Reg[%x] MSDC_CFG = 0x%x\n", OFFSET_MSDC_CFG,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_CFG))));
	printk(BIOS_DEBUG, "Reg[%x] MSDC_IOCON = 0x%x\n", OFFSET_MSDC_IOCON,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_IOCON))));
	printk(BIOS_DEBUG, "Reg[%x] MSDC_PS = 0x%x\n", OFFSET_MSDC_PS,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_PS))));
	printk(BIOS_DEBUG, "Reg[%x] MSDC_INT = 0x%x\n", OFFSET_MSDC_INT,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_INT))));
	printk(BIOS_DEBUG, "Reg[%x] MSDC_INTEN = 0x%x\n", OFFSET_MSDC_INTEN,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_INTEN))));
	printk(BIOS_DEBUG, "Reg[%x] MSDC_FIFOCS = 0x%x\n", OFFSET_MSDC_FIFOCS,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_FIFOCS))));
	printk(BIOS_DEBUG, "Reg[%x] MSDC_TXDATA = not read\n", OFFSET_MSDC_TXDATA);
	printk(BIOS_DEBUG, "Reg[%x] MSDC_RXDATA = not read\n", OFFSET_MSDC_RXDATA);
	printk(BIOS_DEBUG, "Reg[%x] SDC_CFG = 0x%x\n", OFFSET_SDC_CFG,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_CFG))));
	printk(BIOS_DEBUG, "Reg[%x] SDC_CMD = 0x%x\n", OFFSET_SDC_CMD,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_CMD))));
	printk(BIOS_DEBUG, "Reg[%x] SDC_ARG = 0x%x\n", OFFSET_SDC_ARG,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_ARG))));
	printk(BIOS_DEBUG, "Reg[%x] SDC_STS = 0x%x\n", OFFSET_SDC_STS,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_STS))));
	printk(BIOS_DEBUG, "Reg[%x] SDC_RESP0 = 0x%x\n", OFFSET_SDC_RESP0,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_RESP0))));
	printk(BIOS_DEBUG, "Reg[%x] SDC_RESP1 = 0x%x\n", OFFSET_SDC_RESP1,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_RESP1))));
	printk(BIOS_DEBUG, "Reg[%x] SDC_RESP2 = 0x%x\n", OFFSET_SDC_RESP2,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_RESP2))));
	printk(BIOS_DEBUG, "Reg[%x] SDC_RESP3 = 0x%x\n", OFFSET_SDC_RESP3,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_RESP3))));
	printk(BIOS_DEBUG, "Reg[%x] SDC_BLK_NUM = 0x%x\n", OFFSET_SDC_BLK_NUM,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_BLK_NUM))));
	printk(BIOS_DEBUG, "Reg[%x] SDC_CSTS = 0x%x\n", OFFSET_SDC_CSTS,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_CSTS))));
	printk(BIOS_DEBUG, "Reg[%x] SDC_CSTS_EN = 0x%x\n", OFFSET_SDC_CSTS_EN,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_CSTS_EN))));
	printk(BIOS_DEBUG, "Reg[%x] SDC_DATCRC_STS = 0x%x\n", OFFSET_SDC_DCRC_STS,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_DCRC_STS))));
	printk(BIOS_DEBUG, "Reg[%x] EMMC_CFG0 = 0x%x\n", OFFSET_EMMC_CFG0,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_EMMC_CFG0))));
	printk(BIOS_DEBUG, "Reg[%x] EMMC_CFG1 = 0x%x\n", OFFSET_EMMC_CFG1,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_EMMC_CFG1))));
	printk(BIOS_DEBUG, "Reg[%x] EMMC_STS = 0x%x\n", OFFSET_EMMC_STS,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_EMMC_STS))));
	printk(BIOS_DEBUG, "Reg[%x] EMMC_IOCON = 0x%x\n", OFFSET_EMMC_IOCON,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_EMMC_IOCON))));
	printk(BIOS_DEBUG, "Reg[%x] SDC_ACMD_RESP = 0x%x\n", OFFSET_SDC_ACMD_RESP,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_ACMD_RESP))));
	printk(BIOS_DEBUG, "Reg[%x] SDC_ACMD19_TRG = 0x%x\n", OFFSET_SDC_ACMD19_TRG,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_ACMD19_TRG))));
	printk(BIOS_DEBUG, "Reg[%x] SDC_ACMD19_STS = 0x%x\n", OFFSET_SDC_ACMD19_STS,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_ACMD19_STS))));
	printk(BIOS_DEBUG, "Reg[%x] DMA_SA = 0x%x\n", OFFSET_MSDC_DMA_SA,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_SA))));
	printk(BIOS_DEBUG, "Reg[%x] DMA_CA = 0x%x\n", OFFSET_MSDC_DMA_CA,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_CA))));
	printk(BIOS_DEBUG, "Reg[%x] DMA_CTRL = 0x%x\n", OFFSET_MSDC_DMA_CTRL,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_CTRL))));
	printk(BIOS_DEBUG, "Reg[%x] DMA_CFG = 0x%x\n", OFFSET_MSDC_DMA_CFG,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_CFG))));
	printk(BIOS_DEBUG, "Reg[%x] SW_DBG_SEL = 0x%x\n", OFFSET_MSDC_DBG_SEL,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DBG_SEL))));
	printk(BIOS_DEBUG, "Reg[%x] SW_DBG_OUT = 0x%x\n", OFFSET_MSDC_DBG_OUT,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DBG_OUT))));
	printk(BIOS_DEBUG, "Reg[%x] PATCH_BIT0 = 0x%x\n", OFFSET_MSDC_PATCH_BIT0,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_PATCH_BIT0))));
	printk(BIOS_DEBUG, "Reg[%x] PATCH_BIT1 = 0x%x\n", OFFSET_MSDC_PATCH_BIT1,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_PATCH_BIT1))));
	printk(BIOS_DEBUG, "Reg[%x] PAD_TUNE = 0x%x\n", OFFSET_MSDC_PAD_TUNE,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_PAD_TUNE))));
	printk(BIOS_DEBUG, "Reg[%x] DAT_RD_DLY0 = 0x%x\n", OFFSET_MSDC_DAT_RDDLY0,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DAT_RDDLY0))));
	printk(BIOS_DEBUG, "Reg[%x] DAT_RD_DLY1 = 0x%x\n", OFFSET_MSDC_DAT_RDDLY1,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DAT_RDDLY1))));
	printk(BIOS_DEBUG, "Reg[%x] HW_DBG_SEL = 0x%x\n", OFFSET_MSDC_HW_DBG,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_HW_DBG))));
	printk(BIOS_DEBUG, "Reg[%x] MAIN_VER = 0x%x\n", OFFSET_MSDC_VERSION,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_VERSION))));
	printk(BIOS_DEBUG, "Reg[%x] EMMC50_CFG0 = 0x%x\n", OFFSET_EMMC50_CFG0,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_EMMC50_CFG0))));
	printk(BIOS_DEBUG, "Reg[%x] EMMC50_CFG1 = 0x%x\n", OFFSET_EMMC50_CFG1,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_EMMC50_CFG1))));
	printk(BIOS_DEBUG, "Reg[%x] EMMC50_CFG2 = 0x%x\n", OFFSET_EMMC50_CFG2,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_EMMC50_CFG2))));
	printk(BIOS_DEBUG, "Reg[%x] EMMC50_CFG3 = 0x%x\n", OFFSET_EMMC50_CFG3,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_EMMC50_CFG3))));
	printk(BIOS_DEBUG, "Reg[%x] EMMC50_CFG4 = 0x%x\n", OFFSET_EMMC50_CFG4,
		MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_EMMC50_CFG4))));
}

static void msdc_dump_dbg_register(struct mmc_host *host)
{
	u32 base = host->base;
	u32 i;

	for (i = 0; i < 26; i++) {
		MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DBG_SEL)), i);
		printk(BIOS_DEBUG, "SW_DBG_SEL: write reg[%x] to 0x%x\n", OFFSET_MSDC_DBG_SEL, i);
		printk(BIOS_DEBUG, "SW_DBG_OUT: read  reg[%x] to 0x%x\n", OFFSET_MSDC_DBG_OUT,
			MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DBG_OUT))));
	}

	MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DBG_SEL)), 0);
}

static void msdc_dump_info(struct mmc_host *host)
{
	msdc_dump_register(host);

	msdc_dump_padctl(host, GPIO_CLK_CTRL);
	msdc_dump_padctl(host, GPIO_CMD_CTRL);
	msdc_dump_padctl(host, GPIO_DAT_CTRL);
	msdc_dump_padctl(host, GPIO_RST_CTRL);
	msdc_dump_padctl(host, GPIO_DS_CTRL);
	msdc_dump_padctl(host, GPIO_MODE_CTRL);

	/* 5: For designer */
	msdc_dump_dbg_register(host);
}
#else
#define msdc_dump_padctl(x)
#define msdc_dump_register(x)
#define msdc_dump_dbg_register(x)
#define msdc_dump_info(x)
#endif /* IS_ENABLED(CONFIG_DEBUG_MSDC) */

/* add function for MSDC_PAD_CTL handle */
static void msdc_pin_pud(u32 pin, u32 mode, u32 val)
{
	switch (pin) {
	case GPIO_CLK_CTRL:
		MSDC_SET_FIELD(MSDC0_GPIO_CLK_BASE, GPIO_MSDC_R1R0_MASK, val);
		MSDC_SET_FIELD(MSDC0_GPIO_CLK_BASE, GPIO_MSDC_PUPD_MASK, mode);
		break;
	case GPIO_CMD_CTRL:
		MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_MSDC_R1R0_MASK, val);
		MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_MSDC_PUPD_MASK, mode);
		break;
	case GPIO_DAT_CTRL:
		MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_MSDC_R1R0_MASK, val);
		MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_MSDC_PUPD_MASK, mode);
		break;
	case GPIO_RST_CTRL:
		MSDC_SET_FIELD(MSDC0_GPIO_RST_BASE, GPIO_MSDC_R1R0_MASK, val);
		MSDC_SET_FIELD(MSDC0_GPIO_RST_BASE, GPIO_MSDC_PUPD_MASK, mode);
		break;
	case GPIO_DS_CTRL:
		MSDC_SET_FIELD(MSDC0_GPIO_DS_BASE, GPIO_MSDC_R1R0_MASK, val);
		MSDC_SET_FIELD(MSDC0_GPIO_DS_BASE, GPIO_MSDC_PUPD_MASK, mode);
		break;
	default:
		break;
	}
}

static void msdc_set_rdsel(void)
{
	MSDC_SET_FIELD(MSDC0_GPIO_PAD_BASE, GPIO_PAD_RDSEL_MASK, 0x0);
}

static void msdc_set_tdsel(void)
{
	MSDC_SET_FIELD(MSDC0_GPIO_PAD_BASE, GPIO_PAD_TDSEL_MASK, 0x0);
}

/* host can modify from 0-7 */
static void msdc_set_driving(u8 clkdrv, u8 cmddrv, u8 datdrv, u8 rstdrv, u8 dsdrv)
{
	MSDC_SET_FIELD(MSDC0_GPIO_CLK_BASE, GPIO_MSDC_DRV_MASK, clkdrv);
	MSDC_SET_FIELD(MSDC0_GPIO_CMD_BASE, GPIO_MSDC_DRV_MASK, cmddrv);
	MSDC_SET_FIELD(MSDC0_GPIO_DAT_BASE, GPIO_MSDC_DRV_MASK, datdrv);
	MSDC_SET_FIELD(MSDC0_GPIO_RST_BASE, GPIO_MSDC_DRV_MASK, rstdrv);
	MSDC_SET_FIELD(MSDC0_GPIO_DS_BASE, GPIO_MSDC_DRV_MASK, dsdrv);
}

static void msdc_clr_fifo(struct mmc_host *host)
{
	u32 base = host->base;
	MSDC_CLR_FIFO(base);
}

static void msdc_reset(struct mmc_host *host)
{
	u32 base = host->base;
	MSDC_RESET(base);
}

static void msdc_abort(struct mmc_host *host)
{
	u32 base = host->base;

	MSG(INF, "[SD%d] Abort: MSDC_FIFOCS=%xh MSDC_PS=%xh SDC_STS=%xh\n",
			host->id, MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_FIFOCS))),
			MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_PS))),
			MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_STS))));

	/* reset controller */
	msdc_reset(host);

	/* clear fifo */
	msdc_clr_fifo(host);

	/* make sure txfifo and rxfifo are empty */
	if (MSDC_TXFIFOCNT(base) != 0 || MSDC_RXFIFOCNT(base) != 0) {
		MSG(INF, "[SD%d] Abort: TXFIFO(%d), RXFIFO(%d) != 0\n",
				host->id, MSDC_TXFIFOCNT(base), MSDC_RXFIFOCNT(base));
	}

	/* clear all interrupts */
	MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_INT)), MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_INT))));
}

#if MSDC_USE_DMA_MODE
static u8 msdc_cal_checksum(u8 *buf, u32 len)
{
	u32 i;
	u32 sum = 0;

	for (i = 0; i < len; i++)
		sum += buf[i];

	return 0xFF - (u8)sum;
}

/* allocate gpd link-list from gpd_pool */
static gpd_t *msdc_alloc_gpd(struct mmc_host *host, int num)
{
	msdc_priv_t *priv = (msdc_priv_t *) host->priv;
	gpd_t *gpd;
	gpd_t *ptr;
	gpd_t *prev;

	if (priv->alloc_gpd + num + 1 > MAX_GPD_POOL_SZ || num == 0)
		return NULL;

	gpd = priv->gpd_pool + priv->alloc_gpd;
	priv->alloc_gpd += (num + 1); /* include null gpd */

	memset(gpd, 0, sizeof(gpd_t) * (num + 1));

	ptr = gpd + num - 1;
	ptr->next = (void *)(gpd + num); /* pointer to null gpd */

	/* create link-list */
	if (ptr != gpd) {
		do {
			prev = ptr - 1;
			prev->next = ptr;
			ptr = prev;
		} while (ptr != gpd);
	}

	return gpd;
}

/* allocate bd link-list from bd_pool */
static bd_t *msdc_alloc_bd(struct mmc_host *host, int num)
{
	msdc_priv_t *priv = (msdc_priv_t *) host->priv;
	bd_t *bd;
	bd_t *ptr;
	bd_t *prev;

	if (priv->alloc_bd + num > MAX_BD_POOL_SZ || num == 0)
		return NULL;

	bd = priv->bd_pool + priv->alloc_bd;
	priv->alloc_bd += num;

	memset(bd, 0, sizeof(bd_t) * num);

	ptr = bd + num - 1;
	ptr->eol = 1;
	ptr->next = 0;

	/* create link-list */
	if (ptr != bd) {
		do {
			prev = ptr - 1;
			prev->next = ptr;
			prev->eol = 0;
			ptr = prev;
		} while (ptr != bd);
	}

	return bd;
}

/* queue bd link-list to one gpd */
static void msdc_queue_bd(struct mmc_host *host, gpd_t *gpd, bd_t *bd)
{
	msdc_priv_t *priv = (msdc_priv_t *) host->priv;

	BUG_ON(gpd->ptr);

	gpd->hwo = 1;
	gpd->bdp = 1;
	gpd->ptr = (void *)bd;

	if ((priv->cfg.flags & DMA_FLAG_EN_CHKSUM) == 0)
		return;

	/* calculate and fill bd checksum */
	while (bd) {
		bd->chksum = msdc_cal_checksum((u8 *) bd, 16);
		bd = bd->next;
	}
}

/* add gpd link-list to active list */
static void msdc_add_gpd(struct mmc_host *host, gpd_t *gpd, int num)
{
	msdc_priv_t *priv = (msdc_priv_t *) host->priv;

	if (num > 0) {
		if (!priv->active_head)
			priv->active_head = gpd;
		else
			priv->active_tail->next = gpd;

		priv->active_tail = gpd + num - 1;

		if ((priv->cfg.flags & DMA_FLAG_EN_CHKSUM) == 0)
			return;

		/* calculate and fill gpd checksum */
		while (gpd) {
			gpd->chksum = msdc_cal_checksum((u8 *)gpd, 16);
			gpd = gpd->next;
		}
	}
}

static void msdc_reset_gpd(struct mmc_host *host)
{
	msdc_priv_t *priv = (msdc_priv_t *) host->priv;

	priv->alloc_bd = 0;
	priv->alloc_gpd = 0;
	priv->active_head = NULL;
	priv->active_tail = NULL;
}
#endif

#define msdc_set_host_level_pwr(id, level)  do {} while (0)
#define msdc_set_card_pwr(id, on)           do {} while (0)

void msdc_set_timeout(struct mmc_host *host, u32 ns, u32 clks)
{
	u32 base = host->base;
	u32 timeout;
	u32 clk_ns;

	clk_ns = 1000000000UL / host->sclk;
	timeout = ns / clk_ns + clks;
	timeout = timeout >> 20; /* in 1048576 sclk cycle unit */
	timeout = timeout > 1 ? timeout - 1 : 0;
	timeout = timeout > 255 ? 255 : timeout;

	MSDC_SET_FIELD((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_CFG)), SDC_CFG_DTOC, timeout);

	MSG(OPS,
			"[SD%d] Set read data timeout: %dns %dclks -> %d x 65536 cycles\n",
			host->id, ns, clks, timeout + 1);
}

void msdc_set_blklen(struct mmc_host *host, u32 blklen)
{
	msdc_priv_t *priv = (msdc_priv_t *) host->priv;

	host->blklen = blklen;
	priv->cfg.blklen = blklen;
	msdc_clr_fifo(host);
}

void msdc_set_blknum(struct mmc_host *host, u32 blknum)
{
	u32 base = host->base;

	MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_BLK_NUM)), blknum);
}

static int msdc_get_card_status(struct mmc_host *host, u32 *status)
{
	int err;
	struct mmc_command cmd;

	cmd.opcode = MMC_CMD_SEND_STATUS;
	cmd.arg = host->card->rca << 16;
	cmd.rsptyp = RESP_R1;
	cmd.retries = CMD_RETRIES;
	cmd.timeout = CMD_TIMEOUT;

	err = msdc_cmd(host, &cmd);

	if (err == MMC_ERR_NONE)
		*status = cmd.resp[0];
	return err;
}

int msdc_abort_handler(struct mmc_host *host, int abort_card)
{
	struct mmc_command stop;
	u32 status = 0;
	u32 state = 0;

	while (state != 4) {	// until status to "tran"
		msdc_abort(host);
		if (msdc_get_card_status(host, &status)) {
			printk(BIOS_INFO, "Get card status failed\n");
			return 1;
		}
		state = STA_CURRENT_STATE(status);
		printk(BIOS_INFO, "check card state<%d>\n", state);
		if (state == 5 || state == 6) {
			printk(BIOS_INFO, "state<%d> need cmd12 to stop\n",
					state);
			if (abort_card) {
				stop.opcode = MMC_CMD_STOP_TRANSMISSION;
				stop.rsptyp = RESP_R1B;
				stop.arg = 0;
				stop.retries = CMD_RETRIES;
				stop.timeout = CMD_TIMEOUT;
				msdc_send_cmd(host, &stop);
				msdc_wait_rsp(host, &stop); /* don't tuning */
			} else if (state == 7) { /* busy in programing */
				printk(BIOS_INFO, "state<%d> card is busy\n",
						state);
				unsigned _i;
				for (_i = 0; _i < 100; _i++)
					udelay(1000);
			} else if (state != 4) {
				printk(BIOS_INFO, "state<%d> ??? \n", state);
				return 1;
			}
		}
	}
	msdc_abort(host);
	return 0;
}

static u32 msdc_intr_wait(struct mmc_host *host, u32 intrs)
{
	u32 base = host->base;
	u32 sts;
	u32 tmo = 0xFFFF;

	/* warning that interrupts are not enabled */
	WARN_ON((MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_INTEN))) & intrs) != intrs);
	WAIT_COND(((sts = MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_INT)))) & intrs), tmo, tmo);
	if(tmo == 0) {
		printk(BIOS_DEBUG, "[%s]: failed to get event\n", __func__);
		msdc_dump_info(host);
	} else {
		MSG(INT, "[%s]: INT(0x%x)\n", __func__, sts);
	}

	MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_INT)), (sts & intrs));
	if (~intrs & sts) {
		MSG(WRN, "[SD%d]<CHECKME> Unexpected INT(0x%x)\n",
				host->id, ~intrs & sts);
	}
	return sts;
}

static void msdc_intr_unmask(struct mmc_host *host, u32 bits)
{
	u32 base = host->base;
	u32 val;

	val = MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_INTEN)));
	val |= bits;
	MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_INTEN)), val);
}

int msdc_send_cmd(struct mmc_host *host, struct mmc_command *cmd)
{
	msdc_priv_t *priv = (msdc_priv_t *) host->priv;
	u32 base = host->base;
	u32 opcode = cmd->opcode;
	u32 rsptyp = cmd->rsptyp;
	u32 rawcmd;
	u32 timeout = cmd->timeout;
	u32 error = MMC_ERR_NONE;

	/* rawcmd :
	 * vol_swt << 30 | auto_cmd << 28 | blklen << 16 | go_irq << 15 |
	 * stop << 14 | rw << 13 | dtype << 11 | rsptyp << 7 | brk << 6 | opcode
	 */
	rawcmd = (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)) |
		msdc_rsp[rsptyp] << 7 | host->blklen << 16;

	if (opcode == MMC_CMD_WRITE_MULTIPLE_BLOCK) {
		rawcmd |= ((2 << 11) | (1 << 13));
		if (priv->autocmd & MSDC_AUTOCMD12)
			rawcmd |= (1 << 28);
		else if (priv->autocmd & MSDC_AUTOCMD23)
			rawcmd |= (2 << 28);
	} else if (opcode == MMC_CMD_WRITE_BLOCK || opcode == MMC_CMD50) {
		rawcmd |= ((1 << 11) | (1 << 13));
	} else if (opcode == MMC_CMD_READ_MULTIPLE_BLOCK) {
		rawcmd |= (2 << 11);
		if (priv->autocmd & MSDC_AUTOCMD12)
			rawcmd |= (1 << 28);
		else if (priv->autocmd & MSDC_AUTOCMD23)
			rawcmd |= (2 << 28);
	} else if (opcode == MMC_CMD_READ_SINGLE_BLOCK ||
			opcode == SD_ACMD_SEND_SCR ||
			opcode == SD_CMD_SWITCH ||
			opcode == MMC_CMD_SEND_EXT_CSD ||
			opcode == MMC_CMD_SEND_WRITE_PROT ||
			opcode == MMC_CMD_SEND_WRITE_PROT_TYPE ||
			opcode == MMC_CMD21) {
		rawcmd |= (1 << 11);
	} else if (opcode == MMC_CMD_STOP_TRANSMISSION) {
		rawcmd |= (1 << 14);
		rawcmd &= ~(0x0FFF << 16);
	} else if (opcode == SD_IO_RW_EXTENDED) {
		if (cmd->arg & 0x80000000)	/* R/W flag */
			rawcmd |= (1 << 13);
		if ((cmd->arg & 0x08000000) && ((cmd->arg & 0x1FF) > 1))
			rawcmd |= (2 << 11);	/* multiple block mode */
		else
			rawcmd |= (1 << 11);
	} else if (opcode == SD_IO_RW_DIRECT) {
		/* I/O abt */
		if ((cmd->arg & 0x80000000) && ((cmd->arg >> 9) & 0x1FFFF))
			rawcmd |= (1 << 14);
	} else if (opcode == SD_CMD_VOL_SWITCH) {
		rawcmd |= (1 << 30);
	} else if (opcode == SD_CMD_SEND_TUNING_BLOCK) {
		rawcmd |= (1 << 11);	/* CHECKME */
		if (priv->autocmd & MSDC_AUTOCMD19)
			rawcmd |= (3 << 28);
	} else if (opcode == MMC_CMD_GO_IRQ_STATE) {
		rawcmd |= (1 << 15);
	} else if (opcode == MMC_CMD_WRITE_DAT_UNTIL_STOP) {
		rawcmd |= ((1 << 13) | (3 << 11));
	} else if (opcode == MMC_CMD_READ_DAT_UNTIL_STOP) {
		rawcmd |= (3 << 11);
	}

	MSG(CMD, "[SD%d] CMD(%d): ARG(0x%x), RAW(0x%x), RSP(%d)\n",
			host->id, (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)), cmd->arg,
			rawcmd, rsptyp);

	/* Need to check if SDC is busy before data read/write transfer */
	if (opcode == MMC_CMD_SEND_STATUS) {
		if (SDC_IS_CMD_BUSY(base)) {
			WAIT_COND(!SDC_IS_CMD_BUSY(base), cmd->timeout, timeout);
			if (timeout == 0) {
				error = MMC_ERR_TIMEOUT;
				printk(BIOS_INFO,
						"[SD%d] CMD(%d): SDC_IS_CMD_BUSY timeout\n",
						host->id,
						(opcode &
						 ~(SD_CMD_BIT | SD_CMD_APP_BIT)));
				goto end;
			}
		}
	} else {
		if (SDC_IS_BUSY(base)) {
			WAIT_COND(!SDC_IS_BUSY(base), 1000, timeout);
			if (timeout == 0) {
				error = MMC_ERR_TIMEOUT;
				printk(BIOS_INFO,
						"[SD%d] CMD(%d): SDC_IS_BUSY timeout\n",
						host->id,
						(opcode &
						 ~(SD_CMD_BIT | SD_CMD_APP_BIT)));
				goto end;
			}
		}
	}

	MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_ARG)), cmd->arg);
	MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_CMD)), rawcmd);

end:
	cmd->error = error;

	return error;
}

int msdc_wait_rsp(struct mmc_host *host, struct mmc_command *cmd)
{
	u32 base = host->base;
	u32 rsptyp = cmd->rsptyp;
	u32 status;
	u32 opcode = (cmd->opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT));
	u32 error = MMC_ERR_NONE;
	u32 wints = MSDC_INT_CMDTMO | MSDC_INT_CMDRDY | MSDC_INT_RSPCRCERR |
		MSDC_INT_ACMDRDY | MSDC_INT_ACMDCRCERR | MSDC_INT_ACMDTMO |
		MSDC_INT_ACMD19_DONE;

	if (cmd->opcode == MMC_CMD_GO_IRQ_STATE)
		wints |= MSDC_INT_MMCIRQ;

	status = msdc_intr_wait(host, wints);

	if (status == 0) {
		error = MMC_ERR_TIMEOUT;
		goto end;
	}

	if ((status & MSDC_INT_CMDRDY) || (status & MSDC_INT_ACMDRDY) ||
			(status & MSDC_INT_ACMD19_DONE)) {
		switch (rsptyp) {
			case RESP_NONE:
				MSG(RSP, "[SD%d] CMD(%d): RSP(%d)\n", host->id, opcode,
						rsptyp);
				break;
			case RESP_R2:
				{
					u32 *resp = &cmd->resp[0];
					*resp++ = MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_RESP3)));
					*resp++ = MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_RESP2)));
					*resp++ = MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_RESP1)));
					*resp++ = MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_RESP0)));
					MSG(RSP,
							"[SD%d] CMD(%d): RSP(%d) = 0x%x 0x%x 0x%x 0x%x\n",
							host->id, opcode, cmd->rsptyp, cmd->resp[0],
							cmd->resp[1], cmd->resp[2], cmd->resp[3]);
					break;
				}
			default:	/* Response types 1, 3, 4, 5, 6, 7(1b) */
				if ((status & MSDC_INT_ACMDRDY)
						|| (status & MSDC_INT_ACMD19_DONE))
					cmd->resp[0] = MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_ACMD_RESP)));
				else
					cmd->resp[0] = MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_RESP0)));
				MSG(RSP, "[SD%d] CMD(%d): RSP(%d) = 0x%x AUTO(%d)\n",
						host->id, opcode, cmd->rsptyp, cmd->resp[0],
						((status & MSDC_INT_ACMDRDY)
						 || (status & MSDC_INT_ACMD19_DONE)) ? 1 : 0);
				break;
		}
	} else if ((status & MSDC_INT_RSPCRCERR)
			|| (status & MSDC_INT_ACMDCRCERR)) {
		error = MMC_ERR_BADCRC;
		g_crc_count++;
		printk(BIOS_INFO, "[SD%d] CMD(%d): RSP(%d) ERR(BADCRC)\n",
				host->id, opcode, cmd->rsptyp);
	} else if ((status & MSDC_INT_CMDTMO) || (status & MSDC_INT_ACMDTMO)) {
		error = MMC_ERR_TIMEOUT;
		if ((opcode != 55) && (opcode != 5) && (opcode != 8))
			MSG(RSP, "[SD%d] CMD(%d): RSP(%d) ERR(CMDTO) AUTO(%d)\n",
					host->id, opcode, cmd->rsptyp,
					status & MSDC_INT_ACMDTMO ? 1 : 0);
	} else {
		error = MMC_ERR_INVALID;
		printk(BIOS_INFO,
				"[SD%d] CMD(%d): RSP(%d) ERR(INVALID), Status:%x\n",
				host->id, opcode, cmd->rsptyp, status);
	}

end:
	if(((error == MMC_ERR_TIMEOUT) && (opcode != 8) && (opcode != 55) && (opcode !=5))
			|| ((error == MMC_ERR_BADCRC) && (g_crc_count % CRC_ERR_DUMP_NUM == 0))) {
		msdc_dump_info(host);
	}

	if (rsptyp == RESP_R1B)
		while ((MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_PS))) & 0x10000) != 0x10000);

	cmd->error = error;
	if (cmd->opcode == MMC_CMD_APP_CMD && error == MMC_ERR_NONE) {
		host->app_cmd = 1;
		host->app_cmd_arg = cmd->arg;
	} else
		host->app_cmd = 0;

	return error;
}


int msdc_cmd(struct mmc_host *host, struct mmc_command *cmd)
{
	int err;
	err = msdc_send_cmd(host, cmd);
	if (err != MMC_ERR_NONE)
		return err;

	err = msdc_wait_rsp(host, cmd);

	if (err == MMC_ERR_BADCRC) {
		u32 base = host->base;
		u32 tmp = MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_CMD)));

		/* check if data is used by the command or not */
		if (tmp & 0x1800) {
			if(msdc_abort_handler(host, 1))
				printk(BIOS_DEBUG, "[SD%d] abort failed\n",host->id);
		}
	}
	return err;
}
#if MSDC_USE_DMA_MODE
static int msdc_sg_init(struct scatterlist *sg, void *buf, u32 buflen)
{
	int i = MAX_SG_POOL_SZ;
	char *ptr = (char *)buf;

	BUG_ON(buflen > MAX_SG_POOL_SZ * MAX_SG_BUF_SZ);
	while (i > 0) {
		if (buflen > MAX_SG_BUF_SZ) {
			sg->addr = (u32)((uintptr_t)ptr);
			sg->len = MAX_SG_BUF_SZ;
			buflen -= MAX_SG_BUF_SZ;
			ptr += MAX_SG_BUF_SZ;
			sg++;
			i--;
		} else {
			sg->addr = (u32)((uintptr_t)ptr);
			sg->len = buflen;
			i--;
			break;
		}
	}

	return MAX_SG_POOL_SZ - i;
}

static void msdc_dma_init(struct mmc_host *host, struct dma_config *cfg, void *buf,
		u32 buflen)
{
	u32 base = host->base;

	cfg->xfersz = buflen;

	if (cfg->mode == MSDC_MODE_DMA_BASIC) {
		cfg->sglen = 1;
		cfg->sg[0].addr = (u32)((uintptr_t)buf);
		cfg->sg[0].len = buflen;
	} else {
		cfg->sglen = msdc_sg_init(cfg->sg, buf, buflen);
	}

	msdc_clr_fifo(host);
	MSDC_CLR_BIT32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_CFG)), MSDC_CFG_PIO);
}

static int msdc_dma_config(struct mmc_host *host, struct dma_config *cfg)
{
	u32 base = host->base;
	u32 sglen = cfg->sglen;
	u32 j;
	u8 blkpad;
	u8 dwpad;
	u8 chksum;
	struct scatterlist *sg = cfg->sg;
	gpd_t *gpd;
	bd_t *bd;

	switch (cfg->mode) {
		case MSDC_MODE_DMA_BASIC:
			BUG_ON(cfg->xfersz > 65535);
			BUG_ON(cfg->sglen != 1);
			MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_SA)), sg->addr);
			MSDC_SET_FIELD((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_CTRL)), MSDC_DMA_CTRL_LASTBUF, 1);
			MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_LEN)), sg->len);
			MSDC_SET_FIELD((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_CTRL)), MSDC_DMA_CTRL_BRUSTSZ, cfg->burstsz);
			MSDC_SET_FIELD((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_CTRL)), MSDC_DMA_CTRL_MODE, 0);
			break;
		case MSDC_MODE_DMA_DESC:
			blkpad = (cfg->flags & DMA_FLAG_PAD_BLOCK) ? 1 : 0;
			dwpad = (cfg->flags & DMA_FLAG_PAD_DWORD) ? 1 : 0;
			chksum = (cfg->flags & DMA_FLAG_EN_CHKSUM) ? 1 : 0;
			/* calculate the required number of gpd */
			BUG_ON(sglen > MAX_BD_POOL_SZ);

			gpd = msdc_alloc_gpd(host, 1);
			gpd->intr = 0;

			bd = msdc_alloc_bd(host, sglen);
			for (j = 0; j < sglen; j++) {
				MSDC_INIT_BD(((uintptr_t)&bd[j]), blkpad, dwpad, (uintptr_t)sg->addr, sg->len);
				BUG_ON(dlen > 0xFFFFUL);
				((bd_t*)&bd[j])->blkpad = blkpad;
				((bd_t*)&bd[j])->dwpad  = dwpad;
				((bd_t*)&bd[j])->ptr    = (void*)sg->addr;
				((bd_t*)&bd[j])->buflen = sg->len;

				sg++;
			}
			msdc_queue_bd(host, &gpd[0], bd);
			msdc_add_gpd(host, gpd, 1);
			MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_SA)), (u32)((uintptr_t)(&gpd[0])));
			MSDC_SET_FIELD((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_CFG)), MSDC_DMA_CFG_DECSEN, chksum);
			MSDC_SET_FIELD((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_CTRL)), MSDC_DMA_CTRL_BRUSTSZ, cfg->burstsz);
			MSDC_SET_FIELD((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_CTRL)), MSDC_DMA_CTRL_MODE, 1);

			break;
		default:
			break;
	}
	MSG(DMA, "[SD%d] DMA_SA   = 0x%x\n", host->id,
			MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_SA))));
	MSG(DMA, "[SD%d] DMA_CA   = 0x%x\n", host->id,
			MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_CA))));
	MSG(DMA, "[SD%d] DMA_CTRL = 0x%x\n", host->id,
			MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_CTRL))));
	MSG(DMA, "[SD%d] DMA_CFG  = 0x%x\n", host->id,
			MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_CFG))));

	return 0;
}

static void msdc_dma_start(struct mmc_host *host)
{
	u32 base = host->base;

	MSDC_SET_FIELD((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_CTRL)), MSDC_DMA_CTRL_START, 1);

	MSG(DMA, "[SD%d] DMA start\n", host->id);
}

static void msdc_dma_stop(struct mmc_host *host)
{
	u32 base = host->base;

	MSDC_SET_FIELD((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_CTRL)), MSDC_DMA_CTRL_STOP, 1);
	while ((MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_CFG))) & MSDC_DMA_CFG_STS) != 0) ;
	MSDC_SET_BIT32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_CFG)), MSDC_CFG_PIO);

	MSG(DMA, "[SD%d] DMA Stopped\n", host->id);

	msdc_reset_gpd(host);
}

static int msdc_dma_wait_done(struct mmc_host *host, u32 timeout)
{
	u32 base = host->base;
	msdc_priv_t *priv = (msdc_priv_t *) host->priv;
	struct dma_config *cfg = &priv->cfg;
	u32 status;
	u32 error = MMC_ERR_NONE;
	u32 wints = MSDC_INT_XFER_COMPL | MSDC_INT_DATTMO | MSDC_INT_DATCRCERR |
		MSDC_INT_DXFER_DONE | MSDC_INT_DMAQ_EMPTY |
		MSDC_INT_ACMDRDY | MSDC_INT_ACMDTMO | MSDC_INT_ACMDCRCERR |
		MSDC_INT_CMDRDY | MSDC_INT_CMDTMO | MSDC_INT_RSPCRCERR;

	do {
		MSG(DMA, "[SD%d] DMA Curr Addr: 0x%x, Active: %d\n", host->id,
				MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_CA))),
				MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DMA_CFG))) & 0x1);

		status = msdc_intr_wait(host, wints);

		if (status == 0 || status & MSDC_INT_DATTMO) {
			MSG(DMA, "[SD%d] DMA DAT timeout(%xh)\n", host->id,
					status);
			error = MMC_ERR_TIMEOUT;
			goto end;
		} else if (status & MSDC_INT_DATCRCERR) {
			MSG(DMA, "[SD%d] DMA DAT CRC error(%xh)\n", host->id,
					status);
			error = MMC_ERR_BADCRC;
			g_crc_count++;
			goto end;
		} else if (status & MSDC_INT_CMDTMO) {
			MSG(DMA, "[SD%d] DMA CMD timeout(%xh)\n", host->id,
					status);
			error = MMC_ERR_TIMEOUT;
			goto end;
		} else if (status & MSDC_INT_RSPCRCERR) {
			MSG(DMA, "[SD%d] DMA CMD CRC error(%xh)\n", host->id,
					status);
			error = MMC_ERR_BADCRC;
			g_crc_count++;
			goto end;
		} else if (status & MSDC_INT_ACMDTMO) {
			MSG(DMA, "[SD%d] DMA ACMD timeout(%xh)\n", host->id,
					status);
			error = MMC_ERR_TIMEOUT;
			goto end;
		} else if (status & MSDC_INT_ACMDCRCERR) {
			MSG(DMA, "[SD%d] DMA ACMD CRC error(%xh)\n", host->id,
					status);
			error = MMC_ERR_BADCRC;
			g_crc_count++;
			goto end;
		}
		if (status & MSDC_INT_ACMDRDY) {
			cfg->autorsp = MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_ACMD_RESP)));
			MSG(DMA, "[SD%d] DMA AUTO CMD Rdy, Resp(%xh)\n",
					host->id, cfg->autorsp);
		}
		{
			if (status & MSDC_INT_XFER_COMPL)
				break;
		}
	} while (1);

	/* check dma status */
	do {
		status = MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_INT)));
		if (status & MSDC_INT_GPDCSERR) {
			MSG(DMA, "[SD%d] GPD checksum error\n", host->id);
			error = MMC_ERR_BADCRC;
			g_crc_count++;
			break;
		} else if (status & MSDC_INT_BDCSERR) {
			MSG(DMA, "[SD%d] BD checksum error\n", host->id);
			error = MMC_ERR_BADCRC;
			g_crc_count++;
			break;
		} else if ((status & MSDC_DMA_CFG_STS) == 0) {
			break;
		}
	} while (1);
end:
	if((error == MMC_ERR_TIMEOUT) || ((error == MMC_ERR_BADCRC) && (g_crc_count % CRC_ERR_DUMP_NUM == 0))) {
		msdc_dump_info(host);
	}

	if (error)
		MSDC_RESET(base);
	return error;
}

static int msdc_dma_transfer(struct mmc_host *host, struct mmc_command *cmd,
		struct mmc_data *data)
{
	int err = MMC_ERR_NONE, derr = MMC_ERR_NONE;
	int multi;
	u32 blksz = host->blklen;
	msdc_priv_t *priv = (msdc_priv_t *) host->priv;
	struct dma_config *cfg = &priv->cfg;
	struct mmc_command stop;
	u32 base = host->base;
	uint8_t *buf = data->buf;
	uint32_t nblks = data->blks;

	BUG_ON(nblks * blksz > MAX_DMA_TRAN_SIZE);

	multi = nblks > 1 ? 1 : 0;

	u32 left_sz;
	u32 xfer_sz;

	msdc_set_blklen(host, blksz);
	msdc_set_timeout(host, data->timeout * 1000000, 0);

	left_sz = nblks * blksz;

	if (cfg->mode == MSDC_MODE_DMA_BASIC) {
		xfer_sz = left_sz > MAX_DMA_CNT ? MAX_DMA_CNT : left_sz;
		nblks = xfer_sz / blksz;
	} else {
		xfer_sz = left_sz;
	}

	while (left_sz) {
		msdc_set_blknum(host, nblks);
		msdc_dma_init(host, cfg, (void *)buf, xfer_sz);

		err = msdc_send_cmd(host, cmd);
		msdc_dma_config(host, cfg);

		if (err != MMC_ERR_NONE) {
			msdc_reset_gpd(host);
			goto done;
		}

		err = msdc_wait_rsp(host, cmd);

		if (err == MMC_ERR_BADCRC) {
			u32 tmp = MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_CMD)));

			/* check if data is used by the command or not */
			if (tmp & 0x1800) {
				if (msdc_abort_handler(host, 1))
					printk(BIOS_INFO,
							"[SD%d] abort failed\n",
							host->id);
			}
			if (err != MMC_ERR_NONE) {
				msdc_reset_gpd(host);
				goto done;
			}
		}
		msdc_dma_start(host);
		err = derr = msdc_dma_wait_done(host, 0xFFFFFFFF);
		msdc_dma_stop(host);

		if (multi && (priv->autocmd == 0)) {
			stop.opcode = MMC_CMD_STOP_TRANSMISSION;
			stop.rsptyp = RESP_R1B;
			stop.arg = 0;
			stop.retries = CMD_RETRIES;
			stop.timeout = CMD_TIMEOUT;
			err = msdc_cmd(host, &stop) != MMC_ERR_NONE ?
				MMC_ERR_FAILED : err;
		}
		if (err != MMC_ERR_NONE)
			goto done;
		buf += xfer_sz;
		left_sz -= xfer_sz;

		/* left_sz > 0 only when in basic dma mode */
		if (left_sz) {
			cmd->arg += nblks; /* update to next start address */
			xfer_sz = (xfer_sz > left_sz) ?
				left_sz : xfer_sz;
			nblks = (left_sz > xfer_sz) ?
				nblks : left_sz / blksz;
		}
	}
done:
	if (derr != MMC_ERR_NONE) {
		printk(BIOS_INFO, "[SD%d] <CMD%d> DMA data error (%d)\n",
				host->id, cmd->opcode, derr);
		if (msdc_abort_handler(host, 1))
			printk(BIOS_INFO, "[SD%d] abort failed\n", host->id);
	}

	return err;
}

int msdc_dma_bread(struct mmc_host *host, uint8_t *dst, uint32_t src,
		uint32_t nblks)
{
	int multi;
	struct mmc_command cmd;
	struct mmc_data data;

	BUG_ON(nblks > host->max_phys_segs);

	MSG(OPS, "[SD%d] Read data %d blks from 0x%x\n", host->id, nblks, src);

	multi = nblks > 1 ? 1 : 0;

	/* send read command */
	cmd.opcode = multi ?
		MMC_CMD_READ_MULTIPLE_BLOCK : MMC_CMD_READ_SINGLE_BLOCK;
	cmd.rsptyp = RESP_R1;
	cmd.arg = src;
	cmd.retries = 0;
	cmd.timeout = CMD_TIMEOUT;

	data.blks = nblks;
	data.buf = (u8 *) dst;
	data.timeout = 100;	/* 100ms */

	return msdc_dma_transfer(host, &cmd, &data);
}

int msdc_dma_bwrite(struct mmc_host *host, uint32_t dst, uint8_t *src,
		uint32_t nblks)
{
	int multi;
	struct mmc_command cmd;
	struct mmc_data data;

	BUG_ON(nblks > host->max_phys_segs);

	MSG(OPS, "[SD%d] Write data %d blks to 0x%x\n", host->id, nblks, dst);

	multi = nblks > 1 ? 1 : 0;

	/* send write command */
	cmd.opcode = multi ? MMC_CMD_WRITE_MULTIPLE_BLOCK : MMC_CMD_WRITE_BLOCK;
	cmd.rsptyp = RESP_R1;
	cmd.arg = dst;
	cmd.retries = 0;
	cmd.timeout = CMD_TIMEOUT;

	data.blks = nblks;
	data.buf = (u8 *) src;
	data.timeout = 250;	/* 250ms */

	return msdc_dma_transfer(host, &cmd, &data);
}
#endif

static int msdc_pio_read_word(struct mmc_host *host, u32 *ptr, u32 size)
{
	int err = MMC_ERR_NONE;
	u32 base = host->base;
	u32 ints = MSDC_INT_DATCRCERR | MSDC_INT_DATTMO | MSDC_INT_XFER_COMPL;
	u32 status;
	u32 totalsz = size;
	u8 done = 0;
	u8 *u8ptr;

	while (1) {
		status = MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_INT)));
		MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_INT)), status);
		if (status & ~ints) {
			MSG(WRN, "[SD%d]<CHECKME> Unexpected INT(0x%x)\n",
					host->id, status);
		}
		if (status & MSDC_INT_DATCRCERR) {
			printk(BIOS_INFO,
					"[SD%d] DAT CRC error (0x%x), Left:%u/%u bytes, RXFIFO:%u\n",
					host->id, status, size, totalsz,
					MSDC_RXFIFOCNT(base));
			err = MMC_ERR_BADCRC;
			g_crc_count++;
			break;
		} else if (status & MSDC_INT_DATTMO) {
			printk(BIOS_INFO,
					"[SD%d] DAT TMO error (0x%x), Left: %u/%u bytes, RXFIFO:%u\n",
					host->id, status, size, totalsz,
					MSDC_RXFIFOCNT(base));
			err = MMC_ERR_TIMEOUT;
			break;
		} else if (status & MSDC_INT_XFER_COMPL) {
			done = 1;
		}

		if (size == 0 && done)
			break;

		/* Note. RXFIFO count would be aligned to 4-bytes alignment size */
		if ((size >= MSDC_FIFO_THD)
				&& (MSDC_RXFIFOCNT(base) >= MSDC_FIFO_THD)) {
			int left = MSDC_FIFO_THD >> 2;
			do {
				*ptr++ = MSDC_FIFO_READ32(base);
			} while (--left);
			size -= MSDC_FIFO_THD;
			MSG(FIO,
					"[SD%d] Read %d bytes, RXFIFOCNT: %d,  Left: %d/%d\n",
					host->id, MSDC_FIFO_THD, MSDC_RXFIFOCNT(base), size,
					totalsz);
		} else if ((size < MSDC_FIFO_THD) && MSDC_RXFIFOCNT(base) >= size) {
			while (size) {
				if (size > 3) {
					*ptr++ = MSDC_FIFO_READ32(base);
					size -= 4;
				} else {
					u8ptr = (u8 *) ptr;
					while(size --){
						*u8ptr++ = MSDC_FIFO_READ8(base);
					}
				}
			}
			MSG(FIO,
					"[SD%d] Read left bytes, RXFIFOCNT: %d, Left: %d/%d\n",
					host->id, MSDC_RXFIFOCNT(base), size, totalsz);
		}
	}

	if((err == MMC_ERR_TIMEOUT) || ((err == MMC_ERR_BADCRC) && (g_crc_count % CRC_ERR_DUMP_NUM == 0))) {
		msdc_dump_info(host);
	}
	return err;
}

int msdc_pio_read(struct mmc_host *host, u32 *ptr, u32 size)
{
	int err = msdc_pio_read_word(host, (u32 *) ptr, size);

	if (err != MMC_ERR_NONE) {
		msdc_abort(host); /* reset internal fifo and state machine */
		MSG(OPS, "[SD%d] PIO Read Error (%d)\n", host->id, err);
	}

	return err;
}

#if !MSDC_USE_DMA_MODE
static int msdc_pio_write_word(struct mmc_host *host, u32 *ptr, u32 size)
{
	int err = MMC_ERR_NONE;
	u32 base = host->base;
	u32 ints = MSDC_INT_DATCRCERR | MSDC_INT_DATTMO | MSDC_INT_XFER_COMPL;
	u32 status;
	u8 *u8ptr;

	while (1) {
		status = MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_INT)));
		MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_INT)), status);
		if (status & ~ints) {
			MSG(WRN, "[SD%d]<CHECKME> Unexpected INT(0x%x)\n",
					host->id, status);
		}
		if (status & MSDC_INT_DATCRCERR) {
			printk(BIOS_INFO,
					"[SD%d] DAT CRC error (0x%x), Left DAT: %d bytes\n",
					host->id, status, size);
			err = MMC_ERR_BADCRC;
			g_crc_count++;
			break;
		} else if (status & MSDC_INT_DATTMO) {
			printk(BIOS_INFO,
					"[SD%d] DAT TMO error (0x%x), Left DAT: %d bytes\n",
					host->id, status, size);
			err = MMC_ERR_TIMEOUT;
			break;
		} else if (status & MSDC_INT_XFER_COMPL) {
			if (size == 0) {
				MSG(OPS, "[SD%d] all data flushed to card\n",
						host->id);
				break;
			} else {
				MSG(WRN,
						"[SD%d]<CHECKME> XFER_COMPL before all data written\n",
						host->id);
			}
		}

		if (size == 0)
			continue;

		if (size >= MSDC_FIFO_SZ) {
			if (MSDC_TXFIFOCNT(base) == 0) {
				int left = MSDC_FIFO_SZ >> 2;
				do {
					MSDC_FIFO_WRITE32(base, *ptr);
					ptr++;
				} while (--left);
				size -= MSDC_FIFO_SZ;
			}
		} else if (size < MSDC_FIFO_SZ && MSDC_TXFIFOCNT(base) == 0) {
			while (size) {
				if (size > 3) {
					MSDC_FIFO_WRITE32(base, *ptr);
					ptr++;
					size -= 4;
				} else {
					u8ptr = (u8 *) ptr;
					while (size--) {
						MSDC_FIFO_WRITE8(base, *u8ptr);
						u8ptr++;
					}
				}
			}
		}
	}

	if((err == MMC_ERR_TIMEOUT) || ((err == MMC_ERR_BADCRC) && (g_crc_count % CRC_ERR_DUMP_NUM == 0))) {
		msdc_dump_info(host);
	}

	return err;
}

static int msdc_pio_write(struct mmc_host *host, u32 *ptr, u32 size)
{
	int err = msdc_pio_write_word(host, (u32 *) ptr, size);

	if (err != MMC_ERR_NONE) {
		msdc_abort(host); /* reset internal fifo and state machine */
		MSG(OPS, "[SD%d] PIO Write Error (%d)\n", host->id, err);
	}

	return err;
}

int msdc_pio_bread(struct mmc_host *host, uint8_t *dst, uint32_t src,
		uint32_t nblks)
{
	msdc_priv_t *priv = (msdc_priv_t *) host->priv;
	u32 blksz = host->blklen;
	int err = MMC_ERR_NONE, derr = MMC_ERR_NONE;
	int multi;
	struct mmc_command cmd;
	struct mmc_command stop;
	uint32_t *ptr = (uint32_t *) dst;

	MSG(OPS, "[SD%d] Read data %d bytes from 0x%x\n", host->id,
			nblks * blksz, src);

	multi = nblks > 1 ? 1 : 0;

	msdc_clr_fifo(host);
	msdc_set_blknum(host, nblks);
	msdc_set_blklen(host, blksz);
	msdc_set_timeout(host, 100000000, 0);

	/* send read command */
	cmd.opcode =
		multi ? MMC_CMD_READ_MULTIPLE_BLOCK : MMC_CMD_READ_SINGLE_BLOCK;
	cmd.rsptyp = RESP_R1;
	cmd.arg = src;
	cmd.retries = 0;
	cmd.timeout = CMD_TIMEOUT;
	err = msdc_cmd(host, &cmd);

	if (err != MMC_ERR_NONE)
		goto done;

	err = derr = msdc_pio_read(host, (u32 *) ptr, nblks * blksz);

	if (multi && (priv->autocmd == 0)) {
		stop.opcode = MMC_CMD_STOP_TRANSMISSION;
		stop.rsptyp = RESP_R1B;
		stop.arg = 0;
		stop.retries = CMD_RETRIES;
		stop.timeout = CMD_TIMEOUT;
		err =
			msdc_cmd(host,
					&stop) != MMC_ERR_NONE ? MMC_ERR_FAILED : err;
	}

done:
	if (err != MMC_ERR_NONE) {
		if (derr != MMC_ERR_NONE) {
			printk(BIOS_INFO, "[SD%d] Read data error (%d)\n",
					host->id, derr);
			if (msdc_abort_handler(host, 1))
				printk(BIOS_INFO, "[SD%d] abort failed\n",
						host->id);
		} else {
			printk(BIOS_INFO, "[SD%d] Read error (%d)\n", host->id,
					err);
		}
	}
	return (derr == MMC_ERR_NONE) ? err : derr;
}

int msdc_pio_bwrite(struct mmc_host *host, uint32_t dst, uint8_t *src,
		uint32_t nblks)
{
	msdc_priv_t *priv = (msdc_priv_t *) host->priv;
	int err = MMC_ERR_NONE, derr = MMC_ERR_NONE;
	int multi;
	u32 blksz = host->blklen;
	struct mmc_command cmd;
	struct mmc_command stop;
	uint32_t *ptr = (uint32_t *) src;

	MSG(OPS, "[SD%d] Write data %d bytes to 0x%x\n", host->id,
			nblks * blksz, dst);

	multi = nblks > 1 ? 1 : 0;

	msdc_clr_fifo(host);
	msdc_set_blknum(host, nblks);
	msdc_set_blklen(host, blksz);

	/* No need since MSDC always waits 8 cycles for write data timeout */

	/* send write command */
	cmd.opcode = multi ? MMC_CMD_WRITE_MULTIPLE_BLOCK : MMC_CMD_WRITE_BLOCK;
	cmd.rsptyp = RESP_R1;
	cmd.arg = dst;
	cmd.retries = 0;
	cmd.timeout = CMD_TIMEOUT;
	err = msdc_cmd(host, &cmd);

	if (err != MMC_ERR_NONE)
		goto done;

	err = derr = msdc_pio_write(host, (u32 *) ptr, nblks * blksz);

	if (multi && (priv->autocmd == 0)) {
		stop.opcode = MMC_CMD_STOP_TRANSMISSION;
		stop.rsptyp = RESP_R1B;
		stop.arg = 0;
		stop.retries = CMD_RETRIES;
		stop.timeout = CMD_TIMEOUT;
		err = msdc_cmd(host, &stop) != MMC_ERR_NONE ?
			MMC_ERR_FAILED : err;
	}

done:
	if (err != MMC_ERR_NONE) {
		if (derr != MMC_ERR_NONE) {
			printk(BIOS_INFO, "[SD%d] Write data error (%d)\n",
					host->id, derr);
			if (msdc_abort_handler(host, 1))
				printk(BIOS_INFO, "[SD%d] abort failed\n",
						host->id);
		} else {
			printk(BIOS_INFO, "[SD%d] Write error (%d)\n", host->id,
					err);
		}
	}
	return (derr == MMC_ERR_NONE) ? err : derr;
}
#endif

static void msdc_config_clksrc(struct mmc_host *host, clk_source_t clksrc)
{
	host->clksrc = clksrc;

	host->clk  = 200000000;
}

void msdc_config_clock(struct mmc_host *host, int ddr, u32 hz)
{
	u32 base = host->base;
	u32 mode;
	u32 div;
	u32 sclk;
	u32 orig_clksrc = host->clksrc;
	u32 start_bit = 0;

	if (ddr) {
		mode = 0x2;	/* ddr mode and use divisor */
		start_bit = 3;
		if (hz >= (host->clk >> 2)) {
			div = 0; /* mean div = 1/2 */
			sclk = host->clk >> 2; /* sclk = clk/div/2. 2: internal divisor */
		} else {
			div = (host->clk + ((hz << 2) - 1)) / (hz << 2);
			sclk = (host->clk >> 2) / div;
			div = (div >> 1); /* since there is 1/2 internal divisor */
		}
	} else if (hz >= host->clk) {
		mode = 0x1; /* no divisor and divisor is ignored */
		div = 0;
		sclk = host->clk;
	} else {
		mode = 0x0; /* use divisor */
		if (hz >= (host->clk >> 1)) {
			div = 0; /* mean div = 1/2 */
			sclk = host->clk >> 1; /* sclk = clk / 2 */
		} else {
			div = (host->clk + ((hz << 2) - 1)) / (hz << 2);
			sclk = (host->clk >> 2) / div;
		}
	}
	host->sclk = sclk;

	/* set clock mode and divisor */
	MSDC_SET_FIELD((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_CFG)), MSDC_CFG_START_BIT | MSDC_CFG_CKMOD | MSDC_CFG_CKDIV,
			(start_bit << 11) | (mode << 8) | div);

	msdc_config_clksrc(host, orig_clksrc);

	/* wait clock stable */
	while (!(MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_CFG))) & MSDC_CFG_CKSTB));

	printk(BIOS_INFO,
			"[SD%d] SET_CLK(%dkHz): SCLK(%dkHz) MODE(%d) DDR(%d) DIV(%d) DS(%d) RS(%d)\n",
			host->id, hz / 1000, sclk / 1000, mode, ddr > 0 ? 1 : 0, div,
			msdc_cap.data_edge, msdc_cap.cmd_edge);
}

void msdc_config_bus(struct mmc_host *host, u32 width)
{
	u32 base = host->base;
	u32 val = MSDC_READ32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_CFG)));

	val &= ~SDC_CFG_BUSWIDTH;

	switch (width) {
	default:
		width = HOST_BUS_WIDTH_1;
	case HOST_BUS_WIDTH_1:
		val |= (MSDC_BUS_1BITS << 16);
		break;
	case HOST_BUS_WIDTH_4:
		val |= (MSDC_BUS_4BITS << 16);
		break;
	case HOST_BUS_WIDTH_8:
		val |= (MSDC_BUS_8BITS << 16);
		break;
	}
	MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_CFG)), val);

	printk(BIOS_INFO, "[SD%d] Bus Width: %d\n", host->id, width);
}

static void msdc_config_pin(struct mmc_host *host, int mode)
{
	MSG(CFG, "[SD%d] Pins mode(%d), none(0), down(1), up(2), keep(3)\n",
			host->id, mode);

	switch (mode) {
	case MSDC_PIN_PULL_UP:
		msdc_pin_pud(GPIO_CLK_CTRL, MSDC_GPIO_PULL_DOWN, MSDC_PULL_50K);
		msdc_pin_pud(GPIO_CMD_CTRL, MSDC_GPIO_PULL_UP, MSDC_PULL_10K);
		msdc_pin_pud(GPIO_DAT_CTRL, MSDC_GPIO_PULL_UP, MSDC_PULL_10K);
		msdc_pin_pud(GPIO_DS_CTRL, MSDC_GPIO_PULL_DOWN, MSDC_PULL_50K);
		break;
	case MSDC_PIN_PULL_DOWN:
		msdc_pin_pud(GPIO_CLK_CTRL, MSDC_GPIO_PULL_DOWN, MSDC_PULL_50K);
		msdc_pin_pud(GPIO_CMD_CTRL, MSDC_GPIO_PULL_DOWN, MSDC_PULL_50K);
		msdc_pin_pud(GPIO_DAT_CTRL, MSDC_GPIO_PULL_DOWN, MSDC_PULL_50K);
		msdc_pin_pud(GPIO_DS_CTRL, MSDC_GPIO_PULL_DOWN, MSDC_PULL_50K);
		break;
	case MSDC_PIN_PULL_NONE:
		msdc_pin_pud(GPIO_CMD_CTRL, MSDC_GPIO_PULL_DOWN, MSDC_PULL_0K);
		msdc_pin_pud(GPIO_DAT_CTRL, MSDC_GPIO_PULL_DOWN, MSDC_PULL_0K);
	default:
		break;
	}
}

static void msdc_clock(struct mmc_host *host, int on)
{
	MSG(CFG, "[SD%d] Turn %s %s clock \n", host->id, on ? "on" : "off",
			"host");
}

static void msdc_host_power(struct mmc_host *host, int on)
{
	MSG(CFG, "[SD%d] Turn %s %s power \n", host->id, on ? "on" : "off",
			"host");

	if (on) {
		msdc_config_pin(host, MSDC_PIN_PULL_UP);
		msdc_clock(host, 1);
	} else {
		msdc_clock(host, 0);
		msdc_config_pin(host, MSDC_PIN_PULL_DOWN);
	}
}

void msdc_power(struct mmc_host *host, u8 mode)
{
	if (mode == MMC_POWER_ON || mode == MMC_POWER_UP) {
		msdc_host_power(host, 1);
	} else {
		msdc_host_power(host, 0);
	}
}

int msdc_init(struct mmc_host *host, int id)
{
	u32 base = MSDC0_BASE;
#if MSDC_USE_DMA_MODE
	gpd_t *gpd;
	bd_t *bd;
#endif
	msdc_priv_t *priv;
	struct dma_config *cfg;
	clk_source_t clksrc;

	printk(BIOS_INFO,"[%s]: msdc%d Host controller intialization start in preloader\n", __func__, id);
	clksrc = msdc_cap.clk_src;

#if MSDC_USE_DMA_MODE
	gpd = &msdc_gpd_pool[id][0];
	bd = &msdc_bd_pool[id][0];
#endif
	priv = &msdc_priv[id];
	cfg = &priv->cfg;

#if MSDC_USE_DMA_MODE
	memset(gpd, 0, sizeof(gpd_t) * MAX_GPD_POOL_SZ);
	memset(bd, 0, sizeof(bd_t) * MAX_BD_POOL_SZ);
#endif
	memset(priv, 0, sizeof(msdc_priv_t));

	host->id = id;
	host->base = base;
	host->f_max = MSDC_MAX_SCLK;
	host->f_min = MSDC_MIN_SCLK;
	host->blkbits = MMC_BLOCK_BITS;
	host->blklen = 0;
	host->priv = (void *)priv;

	host->caps = MMC_CAP_MULTIWRITE;

	if (msdc_cap.flags & MSDC_HIGHSPEED)
		host->caps |= (MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED);
	if (msdc_cap.flags & MSDC_DDR)
		host->caps |= MMC_CAP_DDR;
	if (msdc_cap.data_pins == 4)
		host->caps |= MMC_CAP_4_BIT_DATA;
	if (msdc_cap.data_pins == 8)
		host->caps |= MMC_CAP_8_BIT_DATA | MMC_CAP_4_BIT_DATA;

	host->ocr_avail = MSDC_VDD_19_20;

	host->max_hw_segs = MAX_DMA_TRAN_SIZE / 512;
	host->max_phys_segs = MAX_DMA_TRAN_SIZE / 512;
	host->max_seg_size = MAX_DMA_TRAN_SIZE;
	host->max_blk_size = 2048;
	host->max_blk_count = 65535;
	host->app_cmd = 0;
	host->app_cmd_arg = 0;
#if MSDC_USE_DMA_MODE
	host->blk_read = msdc_dma_bread;
	host->blk_write = msdc_dma_bwrite;
	cfg->mode = MSDC_MODE_DMA_DESC;
#else
	host->blk_read = msdc_pio_bread;
	host->blk_write = msdc_pio_bwrite;
	cfg->mode = MSDC_MODE_PIO;
#endif

#if MSDC_USE_DMA_MODE
	priv->alloc_bd = 0;
	priv->alloc_gpd = 0;
	priv->bd_pool = bd;
	priv->gpd_pool = gpd;
	priv->active_head = NULL;
	priv->active_tail = NULL;
#endif
	priv->dsmpl = msdc_cap.data_edge;
	priv->rsmpl = msdc_cap.cmd_edge;

#if MSDC_USE_DMA_MODE
	cfg->sg = &priv->sg[0];
	cfg->burstsz = MSDC_BRUST_64B;
	cfg->flags = DMA_FLAG_NONE;
#endif
	msdc_power(host, MMC_POWER_OFF);
	msdc_power(host, MMC_POWER_ON);

	/* set to SD/MMC mode */
	MSDC_SET_FIELD((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_CFG)), MSDC_CFG_MODE, MSDC_SDMMC);
	MSDC_SET_BIT32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_CFG)), MSDC_CFG_PIO);

	msdc_reset(host);
	msdc_clr_fifo(host);
	MSDC_CLR_INT(base);

	/* reset tuning parameter */
	MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_PAD_TUNE)), 0x0000000);
	MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DAT_RDDLY0)), 0x00000000);
	MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_DAT_RDDLY1)), 0x00000000);
	MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_IOCON)), 0x00000000);
	MSDC_WRITE32((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_PATCH_BIT1)), 0xFFFF00C9);

	/* enable SDIO mode. it's must otherwise sdio command failed */
	MSDC_SET_BIT32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_CFG)), SDC_CFG_SDIO);

	/* disable detect SDIO device interupt function */
	MSDC_CLR_BIT32((volatile uint32_t*)((uintptr_t)(base + OFFSET_SDC_CFG)), SDC_CFG_SDIOIDE);

	/* eneable SMT for glitch filter */
	/* set clk, cmd, dat pad driving */
	msdc_set_driving(msdc_cap.clk_drv, msdc_cap.cmd_drv, msdc_cap.dat_drv, msdc_cap.rst_drv, msdc_cap.ds_drv);
	MSDC_SET_FIELD((void*)MSDC0_GPIO_PAD_BASE, GPIO_PAD_BIAS_MASK, 0x5);
	msdc_set_tdsel();
	msdc_set_rdsel();
	/* disable boot function, else eMMC intialization may be failed after BROM ops. */
	MSDC_CLR_BIT32((volatile uint32_t*)((uintptr_t)(base + OFFSET_EMMC_CFG0)), EMMC_CFG0_BOOTSUPP);

	/* set sampling edge */
	MSDC_SET_FIELD((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_IOCON)), MSDC_IOCON_RSPL, msdc_cap.cmd_edge);
	MSDC_SET_FIELD((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_IOCON)), MSDC_IOCON_R_D_SMPL, msdc_cap.data_edge);

	/* write crc timeout detection */
	MSDC_SET_FIELD((volatile uint32_t*)((uintptr_t)(base + OFFSET_MSDC_PATCH_BIT0)), 1 << 30, 1);

	msdc_config_clksrc(host, clksrc);
	msdc_config_bus(host, HOST_BUS_WIDTH_1);
	msdc_config_clock(host, 0, MSDC_MIN_SCLK);
	/* disable sdio interrupt by default. sdio interrupt enable upon
	 * request
	 */
	msdc_intr_unmask(host, 0x0001FF7B);
	msdc_set_timeout(host, 100000000, 0);

	printk(BIOS_INFO,"[%s]: msdc%d Host controller intialization done in preloader\n", __func__, id);
	return 0;
}
