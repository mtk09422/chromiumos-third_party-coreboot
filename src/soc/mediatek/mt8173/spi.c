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
#include <assert.h>
#include <console/console.h>
#include <delay.h>
#include <spi_flash.h>
#include <stdlib.h>
#include <string.h>
#include <timer.h>
#include <soc/addressmap.h>
#include <soc/spi.h>

static int SPI_DEBUG;

enum {
	MT8173_FIFO_SIZE = 32,
	MT8173_PACKET_SIZE = 1024,
	MT8173_TXRX_TIMEOUT_US = 1000 * 1000,
	MT8173_ARBITRARY_VALUE = 0xdeaddead
};

struct mt8173_spi_slave {
	struct spi_slave slave;
	struct MT8173SpiRegs *regs;
	int initialized;
};

static struct mt8173_spi_slave mt8173_spi_slaves[1] = {
	{
		.slave = { .bus = 0, },
		.regs = (void *)SPI_BASE,
	}
};

static inline struct mt8173_spi_slave *to_mt8173_spi(struct spi_slave *slave)
{
	return container_of(slave, struct mt8173_spi_slave, slave);
}

static void spi_sw_reset(struct MT8173SpiRegs *regs)
{
	setbits_le32(&regs->SPI_CMD_REG, 1 << SPI_CMD_RST_OFFSET);
	clrbits_le32(&regs->SPI_CMD_REG, 1 << SPI_CMD_RST_OFFSET);
}

void spi_init(void)
{
	printk(BIOS_INFO, "MT8173 SPI driver initiated.\n");
}

static void mt8173_spi_setup(struct MT8173SpiRegs *regs)
{
	struct mt8173_spi_cfg spi_config;

	spi_config.high_time = 10;
	spi_config.low_time = 10;
	spi_config.setuptime = 10;
	spi_config.holdtime = 12;
	spi_config.cs_idletime = 12;
	spi_config.cpha = 0;
	spi_config.cpol = 0;
	spi_config.rx_endian = 0;
	spi_config.tx_endian = 0;
	spi_config.rx_mlsb = 1;
	spi_config.tx_mlsb = 1;
	spi_config.tckdly = 3;
	spi_config.pause = 1;
	spi_config.deassert = 0;

	/* set the timing */
	clrsetbits_le32(&regs->SPI_CFG0_REG, SPI_CFG0_SCK_HIGH_MASK |
			SPI_CFG0_SCK_LOW_MASK | SPI_CFG0_CS_HOLD_MASK |
			SPI_CFG0_CS_SETUP_MASK,
			((spi_config.high_time - 1) << SPI_CFG0_SCK_HIGH_OFFSET) |
			((spi_config.low_time - 1) << SPI_CFG0_SCK_LOW_OFFSET) |
			((spi_config.holdtime - 1) << SPI_CFG0_CS_HOLD_OFFSET) |
			((spi_config.setuptime - 1) << SPI_CFG0_CS_SETUP_OFFSET));
	clrsetbits_le32(&regs->SPI_CFG1_REG, SPI_CFG1_CS_IDLE_MASK |
			SPI_CFG1_GET_TICK_DLY_MASK,
			((spi_config.cs_idletime - 1) << SPI_CFG1_CS_IDLE_OFFSET) |
			((spi_config.tckdly) << SPI_CFG1_GET_TICK_DLY_OFFSET));

	/* set the mlsbx and mlsbtx */
	clrsetbits_le32(&regs->SPI_CMD_REG,
			(SPI_CMD_TX_ENDIAN_MASK | SPI_CMD_RX_ENDIAN_MASK) |
			(SPI_CMD_TXMSBF_MASK | SPI_CMD_RXMSBF_MASK) |
			(SPI_CMD_CPHA_MASK | SPI_CMD_CPOL_MASK),
			(spi_config.tx_mlsb << SPI_CMD_TXMSBF_OFFSET) |
			(spi_config.rx_mlsb << SPI_CMD_RXMSBF_OFFSET) |
			(spi_config.tx_endian << SPI_CMD_TX_ENDIAN_OFFSET) |
			(spi_config.rx_endian << SPI_CMD_RX_ENDIAN_OFFSET) |
			(spi_config.cpha << SPI_CMD_CPHA_OFFSET) |
			(spi_config.cpol << SPI_CMD_CPOL_OFFSET));

	/* set pause mode */
	clrsetbits_le32(&regs->SPI_CMD_REG, SPI_CMD_PAUSE_EN_MASK,
			(spi_config.pause << SPI_CMD_PAUSE_EN_OFFSET));

	/* set finish interrupt always enable */
	clrsetbits_le32(&regs->SPI_CMD_REG, SPI_CMD_FINISH_IE_MASK,
			1 << SPI_CMD_FINISH_IE_OFFSET);

	/* set pause interrupt always enable */
	clrsetbits_le32(&regs->SPI_CMD_REG, SPI_CMD_PAUSE_IE_MASK,
			1 << SPI_CMD_PAUSE_IE_OFFSET);

	/* set the communication of mode */
	clrbits_le32(&regs->SPI_CMD_REG, SPI_CMD_TX_DMA_MASK | SPI_CMD_RX_DMA_MASK);

	/* set deassert mode */
	clrsetbits_le32(&regs->SPI_CMD_REG, SPI_CMD_DEASSERT_MASK,
			spi_config.deassert << SPI_CMD_DEASSERT_OFFSET);

	/* pad select */
	clrsetbits_le32(&regs->SPI_PAD_MACRO_SEL_REG, SPI_PAD_SEL_MASK, SPI_PAD1_MASK);
}

static void mt8173_spi_dump_data(const char *name, const uint8_t *data, int size)
{
	int i;

	printk(BIOS_INFO, "%s: 0x ", name);
	for (i = 0; i < size; i++)
		printk(BIOS_INFO, "%#x ", data[i]);
	printk(BIOS_INFO, "\n");
}

static void mt8173_spi_init(struct MT8173SpiRegs *regs)
{
	spi_sw_reset(regs);
	mt8173_spi_setup(regs);
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs)
{
	ASSERT(bus == 0);
	struct mt8173_spi_slave *eslave = &mt8173_spi_slaves[bus];

	if (!eslave->initialized) {
		mt8173_spi_init(eslave->regs);
		eslave->initialized = 1;
	}

	return &eslave->slave;
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct MT8173SpiRegs *regs = to_mt8173_spi(slave)->regs;

	mt8173_spi_init(regs);

	/* set pause mode */
	setbits_le32(&regs->SPI_CMD_REG, 1 << SPI_CMD_PAUSE_EN_OFFSET);

	return 0;
}

static int mt8173_spi_fifo_transfer(struct MT8173SpiRegs *regs, void *in,
				    const void *out, u32 size)
{
	uint8_t *inb = (uint8_t *)in;
	const uint32_t *outb = (const uint32_t *)out;
	uint32_t packet_len, packet_loop, reg_val, i, cnt, failed = 0;
	struct stopwatch sw;

	if (!size)
		return 0;

	if (size > MT8173_FIFO_SIZE) {
		printk(BIOS_INFO, "Error: The FIFO_TRANSFER buffer size is %d byte limited.\n",
		       MT8173_FIFO_SIZE);
		return -1;
	}

	if (size < MT8173_PACKET_SIZE)
		packet_len = size;
	else
		packet_len = MT8173_PACKET_SIZE;

	if (size % packet_len) {
		packet_loop = size / packet_len + 1;
	} else
		packet_loop = size / packet_len;

	clrsetbits_le32(&regs->SPI_CFG1_REG,
			SPI_CFG1_PACKET_LENGTH_MASK | SPI_CFG1_PACKET_LOOP_MASK,
			((packet_len - 1) << SPI_CFG1_PACKET_LENGTH_OFFSET) |
			((packet_loop - 1) << SPI_CFG1_PACKET_LOOP_OFFSET));

	/* Disable the tx&rx DMA */
	clrbits_le32(&regs->SPI_CMD_REG, SPI_CMD_TX_DMA_MASK | SPI_CMD_RX_DMA_MASK);

	if (!failed && inb) {
		cnt = (size + 3) >> 2;
		for (i = 0; i < cnt; i++)
			write32(&regs->SPI_TX_DATA_REG, MT8173_ARBITRARY_VALUE);

		setbits_le32(&regs->SPI_CMD_REG, 1 << SPI_CMD_RESUME_OFFSET);

		stopwatch_init_usecs_expire(&sw, MT8173_TXRX_TIMEOUT_US);
		while ((read32(&regs->SPI_STATUS0_REG) & 0x3) == 0) {
			if (stopwatch_expired(&sw)) {
				printk(BIOS_INFO, "Timeout waiting for inb status.\n");
				failed = 1;
				break;
			}
		}

		for (i = 0; !failed && i < size; i++) {
			if (i % 4 == 0) {
				reg_val = read32(&regs->SPI_RX_DATA_REG);
			}
			*((uint8_t *)(inb + i)) = (reg_val >> ((i % 4) * 8)) & 0xff;
		}

		if (SPI_DEBUG)
			mt8173_spi_dump_data("the inb data is", inb, size);
	}

	if (!failed && outb) {
		cnt = (size % 4) ? (size / 4 + 1) : (size / 4);
		for (i = 0; i < cnt; i++)
			write32(&regs->SPI_TX_DATA_REG, outb[i]);

		if (SPI_DEBUG)
			mt8173_spi_dump_data("the outb data is", (const uint8_t *)outb, size);

		setbits_le32(&regs->SPI_CMD_REG, 1 << SPI_CMD_ACT_OFFSET);

		stopwatch_init_usecs_expire(&sw, MT8173_TXRX_TIMEOUT_US);
		while ((read32(&regs->SPI_STATUS1_REG) & 0x1) == 0) {
			if (stopwatch_expired(&sw)) {
				printk(BIOS_INFO, "Timeout waiting for outb status.\n");
				failed = 1;
				break;
			}
		}

		read32(&regs->SPI_STATUS0_REG);
	}

	if (failed) {
		spi_sw_reset(regs);
		return -1;
	} else
		return 0;
}

static int spi_transfer(struct spi_slave *slave, void *in, const void *out, u32 size)
{
	struct MT8173SpiRegs *regs = to_mt8173_spi(slave)->regs;

	return mt8173_spi_fifo_transfer(regs, in, out, size);
}

int spi_xfer(struct spi_slave *slave, const void *dout, unsigned int bytes_out,
	     void *din, unsigned int bytes_in)
{
	int ret = 0;

	if (bytes_out && bytes_in) {
		u32 min_size = MIN(bytes_out, bytes_in);

		ret = spi_transfer(slave, din, dout, min_size);
		if (ret != 0)
			return ret;

		bytes_out -= min_size;
		bytes_in -= min_size;

		din = (uint8_t *)din + min_size;
		dout = (const uint8_t *)dout + min_size;
	}

	if (bytes_in)
		ret = spi_transfer(slave, din, dout, bytes_in);
	else if (bytes_out)
		ret = spi_transfer(slave, din, dout, bytes_out);
	return ret;
}

void spi_release_bus(struct spi_slave *slave)
{
	/* The cs signal will be inative in non-pause mode */
	/* clear pause mode */
	struct MT8173SpiRegs *regs = to_mt8173_spi(slave)->regs;

	clrbits_le32(&regs->SPI_CMD_REG, 1 << SPI_CMD_PAUSE_EN_OFFSET);
}
