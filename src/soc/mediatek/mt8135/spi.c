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

#include <console/console.h>
#include <arch/io.h>
#include <stdlib.h>
#include <assert.h>
#include <spi_flash.h>
#include <string.h>
#include <soc/spi.h>

enum {
	MT8135_PACKET_SIZE = 1024,
	MT8135_SPI_BASE_ADDR = 0x11016000
};

#define SPI_DEBUG

#if defined(CONFIG_DEBUG_SPI) && CONFIG_DEBUG_SPI
# define DEBUG_SPI(x, ...)	printk(BIOS_DEBUG, "MTK_SPI: " x)
#else
# define DEBUG_SPI(x, ...)
#endif

struct mt8135_spi_slave {
	struct spi_slave slave;
	struct MT8135SpiRegs *regs;
	int initialized;
};

static struct mt8135_spi_slave mt8135_spi_slaves[3] = {
	{
		.slave = { .bus = 0, },
		.regs = (void *)MT8135_SPI_BASE_ADDR,
	},
};

static inline struct mt8135_spi_slave *to_mt8135_spi(struct spi_slave *slave)
{
	return container_of(slave, struct mt8135_spi_slave, slave);
}

static void spi_sw_reset(struct MT8135SpiRegs *regs)
{
	setbits_le32(&regs->SPI_CMD_REG, 1 << SPI_CMD_RST_OFFSET);
	clrbits_le32(&regs->SPI_CMD_REG, 1 << SPI_CMD_RST_OFFSET);
}

void spi_init(void)
{
	printk(BIOS_INFO, "MT8135 SPI driver initiated.\n");
}

static void mt8135_spi_setup(struct MT8135SpiRegs *regs)
{
	struct mt8135_spi_cfg spi_config;

	spi_config.high_time = 50;
	spi_config.low_time = 50;
	spi_config.setuptime = 25;
	spi_config.holdtime = 25;
	spi_config.cs_idletime = 25;
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
									SPI_CFG0_SCK_LOW_MASK |
									SPI_CFG0_CS_HOLD_MASK |
									SPI_CFG0_CS_SETUP_MASK,
									((spi_config.high_time-1) << SPI_CFG0_SCK_HIGH_OFFSET) |
									((spi_config.low_time-1) << SPI_CFG0_SCK_LOW_OFFSET) |
									((spi_config.holdtime-1) << SPI_CFG0_CS_HOLD_OFFSET) |
									((spi_config.setuptime-1) << SPI_CFG0_CS_SETUP_OFFSET));
	clrsetbits_le32(&regs->SPI_CFG1_REG, SPI_CFG1_CS_IDLE_MASK | SPI_CFG1_GET_TICK_DLY_MASK,
			((spi_config.cs_idletime-1) << SPI_CFG1_CS_IDLE_OFFSET) |
									((spi_config.tckdly) << SPI_CFG1_GET_TICK_DLY_OFFSET));

	/* set the mlsbx and mlsbtx */
	clrsetbits_le32(&regs->SPI_CMD_REG, (SPI_CMD_TX_ENDIAN_MASK | SPI_CMD_RX_ENDIAN_MASK) |
									(SPI_CMD_TXMSBF_MASK | SPI_CMD_RXMSBF_MASK) |
									(SPI_CMD_CPHA_MASK | SPI_CMD_CPOL_MASK),
									(spi_config.tx_mlsb << SPI_CMD_TXMSBF_OFFSET) |
									(spi_config.rx_mlsb << SPI_CMD_RXMSBF_OFFSET) |
									(spi_config.tx_endian << SPI_CMD_TX_ENDIAN_OFFSET) |
									(spi_config.rx_endian << SPI_CMD_RX_ENDIAN_OFFSET) |
									(spi_config.cpha << SPI_CMD_CPHA_OFFSET) |
									(spi_config.cpol << SPI_CMD_CPOL_OFFSET));

	/* set pause mode */
	clrsetbits_le32(&regs->SPI_CMD_REG, SPI_CMD_PAUSE_EN_MASK, (spi_config.pause << SPI_CMD_PAUSE_EN_OFFSET));

	/* set finish interrupt always enable */
	clrsetbits_le32(&regs->SPI_CMD_REG, SPI_CMD_FINISH_IE_MASK, 1 << SPI_CMD_FINISH_IE_OFFSET);

	/* set pause interrupt always enable */
	clrsetbits_le32(&regs->SPI_CMD_REG, SPI_CMD_PAUSE_IE_MASK, 1 << SPI_CMD_PAUSE_IE_OFFSET);

	/* set the communication of mode */
	clrbits_le32(&regs->SPI_CMD_REG, SPI_CMD_TX_DMA_MASK | SPI_CMD_RX_DMA_MASK);

	/* set deassert mode */
	clrsetbits_le32(&regs->SPI_CMD_REG, SPI_CMD_DEASSERT_MASK, spi_config.deassert << SPI_CMD_DEASSERT_OFFSET);
}

static void mt8135_spi_init(struct MT8135SpiRegs *regs)
{
	spi_sw_reset(regs);
	mt8135_spi_setup(regs);
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs)
{
	ASSERT(bus == 0);
	struct mt8135_spi_slave *eslave = &mt8135_spi_slaves[bus];

	if (!eslave->initialized) {
		mt8135_spi_init(eslave->regs);
		eslave->initialized = 1;
	}

	return &eslave->slave;
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct MT8135SpiRegs *regs = to_mt8135_spi(slave)->regs;

	mt8135_spi_init(regs);

	return 0;
}

static void spi_transfer(struct MT8135SpiRegs *regs, void *in, const void *out,
			 u32 size)
{
	struct mt8135_spi_cfg spi_config;
	uint32_t data_length, data_loop;
	uint8_t *inb = in;
	const uint8_t *outb = out;

	/* set transfer packet and loop */
	if (size < MT8135_PACKET_SIZE)
		data_length = size;
	else
		data_length = MT8135_PACKET_SIZE;

	if (size % data_length) {
		data_loop = size / data_length + 1;
		printk(BIOS_INFO, "ERROR!!The lens must be a multiple of %d, your len %u\n\n", MT8135_PACKET_SIZE, size);
	} else
		data_loop = size / data_length;

	printk(BIOS_INFO, "The packet_len:0x%x packet_loop:0x%x\n", data_length, data_loop);

	spi_config.packet_len = data_length;
	spi_config.packet_loop = data_loop;

	clrsetbits_le32(&regs->SPI_CFG1_REG, SPI_CFG1_PACKET_LENGTH_MASK |
							SPI_CFG1_PACKET_LOOP_MASK,
							((spi_config.packet_len - 1) << SPI_CFG1_PACKET_LENGTH_OFFSET) |
							((spi_config.packet_loop - 1) << SPI_CFG1_PACKET_LOOP_OFFSET));

	/* enable the RX/TX DMA */
	setbits_le32(&regs->SPI_CMD_REG, (1 << SPI_CMD_RX_DMA_OFFSET) | (1 << SPI_CMD_TX_DMA_OFFSET));

	if (inb) {
		/* resume the transfer */
		setbits_le32(&regs->SPI_CMD_REG, 1 << SPI_CMD_RESUME_OFFSET);
		writel((uint32_t)inb, &regs->SPI_RX_DST_REG);
#ifdef SPI_DEBUG
		int i;

		printk(BIOS_INFO, "the received data is");
		for (i = 0; i < size; ++i) {
			printk(BIOS_INFO, "0x%x", (uint8_t)*inb++);
		}
		printk(BIOS_INFO, "\n");
#endif

	}
	if (outb) {
		writel((uint32_t)outb, &regs->SPI_TX_SRC_REG);
#ifdef SPI_DEBUG
		int i;

		printk(BIOS_INFO, "the output data is");
		for (i = 0; i < size; ++i) {
			printk(BIOS_INFO, "0x%x", (uint8_t)*outb++);
		}
		printk(BIOS_INFO, "\n");
#endif
		/* start the transfer */
		setbits_le32(&regs->SPI_CMD_REG, 1 << SPI_CMD_ACT_OFFSET);
	}
}

int spi_xfer(struct spi_slave *slave, const void *dout, unsigned int bytes_out,
	     void *din, unsigned int bytes_in)
{
	struct MT8135SpiRegs *regs = to_mt8135_spi(slave)->regs;

	if (bytes_out && bytes_in) {
		u32 min_size = MIN(bytes_out, bytes_in);

		spi_transfer(regs, din, dout, min_size);

		bytes_out -= min_size;
		bytes_in -= min_size;

		din = (uint8_t *)din + min_size;
		dout = (const uint8_t *)dout + min_size;
	}

	if (bytes_in)
		spi_transfer(regs, din, NULL, bytes_in);
	else if (bytes_out)
		spi_transfer(regs, NULL, dout, bytes_out);

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	/* The cs signal will be inative in non-pause mode */
}

static int mt8135_spi_read(struct spi_slave *slave, void *dest, uint32_t len,
			   uint32_t off)
{
	struct MT8135SpiRegs *regs = to_mt8135_spi(slave)->regs;
	u32 command;

	spi_claim_bus(slave);

	/* Send address. */
	ASSERT(off < (1 << 24));
	command = htonl(SF_READ_DATA_CMD << 24 | off);
	spi_transfer(regs, NULL, &command, sizeof(command));

	/* Read the data. */
	spi_transfer(regs, dest, NULL, len);
	spi_release_bus(slave);

	return len;
}

/* SPI as CBFS media. */
struct mt8135_spi_media {
	struct spi_slave *slave;
	struct cbfs_simple_buffer buffer;
};

static int mt8135_spi_cbfs_open(struct cbfs_media *media)
{
	struct mt8135_spi_media *spi = (struct mt8135_spi_media *)media->context;

	DEBUG_SPI("mt8135_spi_cbfs_open\n");
	return spi_claim_bus(spi->slave);
}

static int mt8135_spi_cbfs_close(struct cbfs_media *media)
{
	struct mt8135_spi_media *spi = (struct mt8135_spi_media *)media->context;

	DEBUG_SPI("mt8135_spi_cbfs_close\n");
	spi_release_bus(spi->slave);
	return 0;
}

static size_t mt8135_spi_cbfs_read(struct cbfs_media *media, void *dest,
				   size_t offset, size_t count)
{
	struct mt8135_spi_media *spi = (struct mt8135_spi_media *)media->context;
	int bytes;

	DEBUG_SPI("mt8135_spi_cbfs_read(%u)\n", count);
	bytes = mt8135_spi_read(spi->slave, dest, count, offset);
	return bytes;
}

static void *mt8135_spi_cbfs_map(struct cbfs_media *media, size_t offset,
				 size_t count)
{
	struct mt8135_spi_media *spi = (struct mt8135_spi_media *)media->context;

	DEBUG_SPI("mt8135_spi_cbfs_map\n");
	/* mt8135: spi_rx_tx may work in 4 byte-width-transmission mode and */
	/* requires buffer memory address to be aligned. */
	if (count % 4)
		count += 4 - (count % 4);
	return cbfs_simple_buffer_map(&spi->buffer, media, offset, count);
}

static void *mt8135_spi_cbfs_unmap(struct cbfs_media *media,
				   const void *address)
{
	struct mt8135_spi_media *spi = (struct mt8135_spi_media *)media->context;

	DEBUG_SPI("mt8135_spi_cbfs_unmap\n");
	return cbfs_simple_buffer_unmap(&spi->buffer, address);
}

int initialize_mt8135_spi_cbfs_media(struct cbfs_media *media,
				     void *buffer_address,
				     size_t buffer_size)
{
	/* TODO Replace static variable to support multiple streams. */
	static struct mt8135_spi_media context;
	static struct mt8135_spi_slave *eslave = &mt8135_spi_slaves[1];

	DEBUG_SPI("initialize_mt8135_spi_cbfs_media\n");

	context.slave = &eslave->slave;
	context.buffer.allocated = context.buffer.last_allocate = 0;
	context.buffer.buffer = buffer_address;
	context.buffer.size = buffer_size;
	media->context = (void *)&context;
	media->open = mt8135_spi_cbfs_open;
	media->close = mt8135_spi_cbfs_close;
	media->read = mt8135_spi_cbfs_read;
	media->map = mt8135_spi_cbfs_map;
	media->unmap = mt8135_spi_cbfs_unmap;

	return 0;
}
