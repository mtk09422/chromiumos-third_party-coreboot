/*
 * (C) Copyright 2010,2011
 * NVIDIA Corporation <www.nvidia.com>
 * Copyright 2013 Google Inc.
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

#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <console/console.h>

#include <arch/io.h>
#include <soc/addressmap.h>

#include "dma.h"

enum {
	APB_DMA_CH0 =		TEGRA_APB_DMA_BASE + 0x1000,
	APB_DMA_CH1 =		TEGRA_APB_DMA_BASE + 0x1040,
	APB_DMA_CH2 =		TEGRA_APB_DMA_BASE + 0x1080,
	APB_DMA_CH3 =		TEGRA_APB_DMA_BASE + 0x10c0,
	APB_DMA_CH4 =		TEGRA_APB_DMA_BASE + 0x1100,
	APB_DMA_CH5 =		TEGRA_APB_DMA_BASE + 0x1140,
	APB_DMA_CH6 =		TEGRA_APB_DMA_BASE + 0x1180,
	APB_DMA_CH7 =		TEGRA_APB_DMA_BASE + 0x11c0,
	APB_DMA_CH8 =		TEGRA_APB_DMA_BASE + 0x1200,
	APB_DMA_CH9 =		TEGRA_APB_DMA_BASE + 0x1240,
	APB_DMA_CH10 =		TEGRA_APB_DMA_BASE + 0x1280,
	APB_DMA_CH11 =		TEGRA_APB_DMA_BASE + 0x12c0,
	APB_DMA_CH12 =		TEGRA_APB_DMA_BASE + 0x1300,
	APB_DMA_CH13 =		TEGRA_APB_DMA_BASE + 0x1340,
	APB_DMA_CH14 =		TEGRA_APB_DMA_BASE + 0x1380,
	APB_DMA_CH15 =		TEGRA_APB_DMA_BASE + 0x13c0,
	APB_DMA_CH16 =		TEGRA_APB_DMA_BASE + 0x1400,
	APB_DMA_CH17 =		TEGRA_APB_DMA_BASE + 0x1440,
	APB_DMA_CH18 =		TEGRA_APB_DMA_BASE + 0x1480,
	APB_DMA_CH19 =		TEGRA_APB_DMA_BASE + 0x14c0,
	APB_DMA_CH20 =		TEGRA_APB_DMA_BASE + 0x1500,
	APB_DMA_CH21 =		TEGRA_APB_DMA_BASE + 0x1540,
	APB_DMA_CH22 =		TEGRA_APB_DMA_BASE + 0x1580,
	APB_DMA_CH23 =		TEGRA_APB_DMA_BASE + 0x15c0,
	APB_DMA_CH24 =		TEGRA_APB_DMA_BASE + 0x1600,
	APB_DMA_CH25 =		TEGRA_APB_DMA_BASE + 0x1640,
	APB_DMA_CH26 =		TEGRA_APB_DMA_BASE + 0x1680,
	APB_DMA_CH27 =		TEGRA_APB_DMA_BASE + 0x16c0,
	APB_DMA_CH28 =		TEGRA_APB_DMA_BASE + 0x1700,
	APB_DMA_CH29 =		TEGRA_APB_DMA_BASE + 0x1740,
	APB_DMA_CH30 =		TEGRA_APB_DMA_BASE + 0x1780,
	APB_DMA_CH31 =		TEGRA_APB_DMA_BASE + 0x17c0,
};

/*
 * Note: Many APB DMA controller registers are laid out such that each
 * bit controls or represents the status for the corresponding channel.
 * So we will not bother to list each individual bit in this case.
 */
#define APBDMA_COMMAND_GEN			(1 << 31)

#define APBDMA_CNTRL_REG_COUNT_VALUE_MASK	0xffff
#define APBDMA_CNTRL_REG_COUNT_VALUE_SHIFT	0

struct apb_dma {
	u32 command;		/* 0x00 */
	u32 status;		/* 0x04 */
	u8 rsvd1[0x8];
	u32 cntrl_reg;		/* 0x10 */
	u32 irq_sta_cpu;	/* 0x14 */
	u32 irq_sta_cop;	/* 0x18 */
	u32 irq_mask;		/* 0x1c */
	u32 irq_mask_set;	/* 0x20 */
	u32 irq_mask_clr;	/* 0x24 */
	u32 trig_reg;		/* 0x28 */
	u32 channel_trig_reg;	/* 0x2c */
	u32 dma_status;		/* 0x30 */
	u32 channel_en_reg;	/* 0x34 */
	u32 security_reg;	/* 0x38 */
	u32 channel_swid;	/* 0x3c */
	u8 rsvd[4];
	u32 chan_wt_reg0;	/* 0x44 */
	u32 chan_wt_reg1;	/* 0x48 */
	u32 chan_wt_reg2;	/* 0x4c */
	u32 chan_wr_reg3;	/* 0x50 */
	u32 channel_swid1;	/* 0x54 */
} __attribute__((packed));
struct apb_dma * const apb_dma = (struct apb_dma *)TEGRA_APB_DMA_BASE;

/*
 * FIXME: Naming in the doc included a superfluous _CHANNEL_n_ for
 * each entry which I left out for the sake of conciseness.
 */
#define APBDMACHAN_CSR_ENB			(1 << 31)
#define APBDMACHAN_CSR_IE_EOC			(1 << 30)
#define APBDMACHAN_CSR_HOLD			(1 << 29)
#define APBDMACHAN_CSR_DIR			(1 << 28)
#define APBDMACHAN_CSR_ONCE			(1 << 27)
#define APBDMACHAN_CSR_FLOW			(1 << 21)
#define APBDMACHAN_CSR_REQ_SEL_MASK		0x1f
#define APBDMACHAN_CSR_REQ_SEL_SHIFT		16

#define APBDMACHAN_STA_BSY			(1 << 31)
#define APBDMACHAN_STA_ISE_EOC			(1 << 30)
#define APBDMACHAN_STA_HALT			(1 << 29)
#define APBDMACHAN_STA_PING_PONG_STA		(1 << 28)
#define APBDMACHAN_STA_DMA_ACTIVITY		(1 << 27)
#define APBDMACHAN_STA_CHANNEL_PAUSE		(1 << 26)

#define APBDMACHAN_CSRE_CHANNEL_PAUSE		(1 << 31)
#define APBDMACHAN_CSRE_TRIG_SEL_MASK		0x3f
#define APBDMACHAN_CSRE_TRIG_SEL_SHIFT		14

#define APBDMACHAN_AHB_PTR_MASK			(0x3fffffff)
#define APBDMACHAN_AHB_PTR_SHIFT		2

#define APBDMACHAN_AHB_SEQ_INTR_ENB		(1 << 31)
#define APBDMACHAN_AHB_SEQ_AHB_BUS_WIDTH_MASK	0x7
#define APBDMACHAN_AHB_SEQ_AHB_BUS_WIDTH_SHIFT	28
#define APBDMACHAN_AHB_SEQ_AHB_DATA_SWAP	(1 << 27)
#define APBDMACHAN_AHB_SEQ_AHB_BURST_MASK	0x7
#define APBDMACHAN_AHB_SEQ_AHB_BURST_SHIFT	24
#define APBDMACHAN_AHB_SEQ_DBL_BUF		(1 << 19)
#define APBDMACHAN_AHB_SEQ_WRAP_MASK		0x7
#define APBDMACHAN_AHB_SEQ_WRAP_SHIFT		16

#define APBDMACHAN_AHB_SEQ_AHB_BUS_WIDTH_MASK	0x7
#define APBDMACHAN_AHB_SEQ_AHB_BUS_WIDTH_SHIFT	28

#define APBDMACHAN_APB_PTR_MASK			0x3fffffff
#define APBDMACHAN_APB_PTR_SHIFT		2

#define APBDMACHAN_APB_SEQ_APB_BUS_WIDTH_MASK	0x7
#define APBDMACHAN_APB_SEQ_APB_BUS_WIDTH_SHIFT	28
#define APBDMACHAN_APB_SEQ_APB_DATA_SWAP	(1 << 27)
#define APBDMACHAN_APB_SEQ_APB_ADDR_WRAP_MASK	0x7
#define APBDMACHAN_APB_SEQ_APB_ADDR_WRAP_SHIFT	16

#define APBDMACHAN_WORD_TRANSFER_

#define APBDMACHAN_WORD_TRANSFER_MASK		0x0fffffff
#define APBDMACHAN_WORD_TRANSFER_SHIFT		2

#define NUM_APB_DMA_CHANNELS	32
#define APB_DMA_CHANNEL(n) \
	{								\
		.num = n,						\
		.regs = (struct apb_dma_channel_regs *)APB_DMA_CH##n,	\
	}
struct apb_dma_channel apb_dma_channels[NUM_APB_DMA_CHANNELS] = {
	APB_DMA_CHANNEL(0),
	APB_DMA_CHANNEL(1),
	APB_DMA_CHANNEL(2),
	APB_DMA_CHANNEL(3),
	APB_DMA_CHANNEL(4),
	APB_DMA_CHANNEL(5),
	APB_DMA_CHANNEL(6),
	APB_DMA_CHANNEL(7),
	APB_DMA_CHANNEL(8),
	APB_DMA_CHANNEL(9),
	APB_DMA_CHANNEL(10),
	APB_DMA_CHANNEL(11),
	APB_DMA_CHANNEL(12),
	APB_DMA_CHANNEL(13),
	APB_DMA_CHANNEL(14),
	APB_DMA_CHANNEL(15),
	APB_DMA_CHANNEL(16),
	APB_DMA_CHANNEL(17),
	APB_DMA_CHANNEL(18),
	APB_DMA_CHANNEL(19),
	APB_DMA_CHANNEL(20),
	APB_DMA_CHANNEL(21),
	APB_DMA_CHANNEL(22),
	APB_DMA_CHANNEL(23),
	APB_DMA_CHANNEL(24),
	APB_DMA_CHANNEL(25),
	APB_DMA_CHANNEL(26),
	APB_DMA_CHANNEL(27),
	APB_DMA_CHANNEL(28),
	APB_DMA_CHANNEL(29),
	APB_DMA_CHANNEL(30),
	APB_DMA_CHANNEL(31),
};

int dma_busy(struct apb_dma_channel * const channel)
{
	/*
	 * In continuous mode, the BSY_n bit in APB_DMA_STATUS and
	 * BSY in APBDMACHAN_CHANNEL_n_STA_0 will remain set as '1' so long
	 * as the channel is enabled. So for this function we'll use the
	 * DMA_ACTIVITY bit.
	 */
	return read32(&channel->regs->sta) & APBDMACHAN_STA_DMA_ACTIVITY ? 1 : 0;
}
/* claim a DMA channel */
struct apb_dma_channel * const dma_claim(void)
{
	int i;
	struct apb_dma_channel_regs *regs = NULL;

	/*
	 * Set global enable bit, otherwise register access to channel
	 * DMA registers will not be possible.
	 */
	setbits_le32(&apb_dma->command, APBDMA_COMMAND_GEN);

	for (i = 0; i < ARRAY_SIZE(apb_dma_channels); i++) {
		regs = apb_dma_channels[i].regs;

		if (!apb_dma_channels[i].in_use) {
			u32 status = read32(&regs->sta);
			if (status & (1 << i)) {
				/* FIXME: should this be fatal? */
				printk(BIOS_DEBUG, "%s: DMA channel %d busy?\n",
						__func__, i);
			}
			break;
		}
	}

	if (i == NUM_APB_DMA_CHANNELS)
		return NULL;

	apb_dma_channels[i].in_use = 1;
	return &apb_dma_channels[i];
}

/* release a DMA channel */
void dma_release(struct apb_dma_channel * const channel)
{
	int i;
	int dma_in_use = 0;

	while (dma_busy(channel))
		;

	channel->in_use = 0;

	/* clear the global enable bit if no channels are in use */
	for (i = 0; i < ARRAY_SIZE(apb_dma_channels); i++) {
		if (apb_dma_channels[i].in_use) {
			dma_in_use = 1;
			break;
		}
	}

	if (!dma_in_use)
		clrbits_le32(&apb_dma->command, APBDMA_COMMAND_GEN);
}

int dma_start(struct apb_dma_channel * const channel)
{
	struct apb_dma_channel_regs *regs = channel->regs;

	/* Set ENB bit for this channel */
	setbits_le32(&regs->csr, APBDMACHAN_CSR_ENB);

	return 0;
}

int dma_stop(struct apb_dma_channel * const channel)
{
	struct apb_dma_channel_regs *regs = channel->regs;

	/* Clear ENB bit for this channel */
	clrbits_le32(&regs->csr, APBDMACHAN_CSR_ENB);

	return 0;
}
