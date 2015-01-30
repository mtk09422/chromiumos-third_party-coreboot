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

#ifndef MTK_MT8135_SPI_H
#define MTK_MT8135_SPI_H

/* This driver serves as a CBFS media source. */
#include <cbfs.h>

/*SPI peripheral register map.*/
typedef struct MT8135SpiRegs {
	uint32_t SPI_CFG0_REG;	/* 0x00 */
	uint32_t SPI_CFG1_REG;
	uint32_t SPI_TX_SRC_REG;	/* 0x08 */
	uint32_t SPI_RX_DST_REG;	/* 0x0c */
	uint32_t SPI_TX_DATA_REG;
	uint32_t SPI_RX_DATA_REG;	/* 0x14 */
	uint32_t SPI_CMD_REG;	/* 0x18 */
	uint32_t SPI_STATUS0_REG;	/* 0x20 */
	uint32_t SPI_STATUS1_REG;	/* 0x20 */
} MT8135SpiRegs;

check_member(MT8135SpiRegs, SPI_STATUS1_REG, 0x20);

#define SF_READ_DATA_CMD	0x3

/*SPI_CFG0_REG*/
enum {
	SPI_CFG0_SCK_HIGH_OFFSET = 0,
	SPI_CFG0_SCK_LOW_OFFSET = 8,
	SPI_CFG0_CS_HOLD_OFFSET = 16,
	SPI_CFG0_CS_SETUP_OFFSET = 24,

	SPI_CFG0_SCK_HIGH_MASK = 0xff,
	SPI_CFG0_SCK_LOW_MASK = 0xff00,
	SPI_CFG0_CS_HOLD_MASK = 0xff0000,
	SPI_CFG0_CS_SETUP_MASK = 0xff000000
};

/*SPI_CFG1_REG*/
enum {
	SPI_CFG1_CS_IDLE_OFFSET = 0,
	SPI_CFG1_PACKET_LOOP_OFFSET = 8,
	SPI_CFG1_PACKET_LENGTH_OFFSET = 16,
	SPI_CFG1_GET_TICK_DLY_OFFSET = 30,

	SPI_CFG1_CS_IDLE_MASK = 0xff,
	SPI_CFG1_PACKET_LOOP_MASK = 0xff00,
	SPI_CFG1_PACKET_LENGTH_MASK = 0x3ff0000,
	SPI_CFG1_GET_TICK_DLY_MASK = 0xc0000000
};

enum {
	SPI_CMD_ACT_OFFSET = 0,
	SPI_CMD_RESUME_OFFSET = 1,
	SPI_CMD_RST_OFFSET = 2,
	SPI_CMD_PAUSE_EN_OFFSET = 4,
	SPI_CMD_DEASSERT_OFFSET = 5,
	SPI_CMD_CPHA_OFFSET = 8,
	SPI_CMD_CPOL_OFFSET = 9,
	SPI_CMD_RX_DMA_OFFSET = 10,
	SPI_CMD_TX_DMA_OFFSET = 11,
	SPI_CMD_TXMSBF_OFFSET = 12,
	SPI_CMD_RXMSBF_OFFSET = 13,
	SPI_CMD_RX_ENDIAN_OFFSET = 14,
	SPI_CMD_TX_ENDIAN_OFFSET = 15,
	SPI_CMD_FINISH_IE_OFFSET = 16,
	SPI_CMD_PAUSE_IE_OFFSET = 17,

	SPI_CMD_ACT_MASK = 0x1,
	SPI_CMD_RESUME_MASK = 0x2,
	SPI_CMD_RST_MASK = 0x4,
	SPI_CMD_PAUSE_EN_MASK = 0x10,
	SPI_CMD_DEASSERT_MASK = 0x20,
	SPI_CMD_CPHA_MASK = 0x100,
	SPI_CMD_CPOL_MASK = 0x200,
	SPI_CMD_RX_DMA_MASK = 0x400,
	SPI_CMD_TX_DMA_MASK = 0x800,
	SPI_CMD_TXMSBF_MASK = 0x1000,
	SPI_CMD_RXMSBF_MASK = 0x2000,
	SPI_CMD_RX_ENDIAN_MASK = 0x4000,
	SPI_CMD_TX_ENDIAN_MASK = 0x8000,
	SPI_CMD_FINISH_IE_MASK = 0x10000,
	SPI_CMD_PAUSE_IE_MASK = 0x20000
};

/*basic config for spi-controller timing*/
enum spi_cpol {
	SPI_CPOL_0,
	SPI_CPOL_1
};

enum spi_cpha {
	SPI_CPHA_0,
	SPI_CPHA_1
};

enum spi_mlsb {
	SPI_LSB,
	SPI_MSB
};

enum spi_endian {
	SPI_LENDIAN,
	SPI_BENDIAN
};

enum spi_fifo {
	FIFO_TX,
	FIFO_RX,
	FIFO_ALL
};

enum spi_transfer_mode {
	FIFO_TRANSFER,	/*RX_FIFO, TX_FIFO*/
	DMA_TRANSFER,	/*RX_DMA, TX_DMA*/
	OTHER1,	/*RX_FIFO, TX_DMA*/
	OTHER2,	/*RX_DMA, TX_FIFO*/
};

enum spi_pause_mode {
	PAUSE_MODE_DISABLE,
	PAUSE_MODE_ENABLE
};

enum spi_finish_intr {
	FINISH_INTR_DIS,
	FINISH_INTR_EN,
};

enum spi_deassert_mode {
	DEASSERT_DISABLE,
	DEASSERT_ENABLE
};

enum spi_tckdly {
	TICK_DLY0,
	TICK_DLY1,
	TICK_DLY2,
	TICK_DLY3
};

struct mt8135_spi_cfg {
	uint32_t setuptime;
	uint32_t holdtime;
	uint32_t high_time;
	uint32_t low_time;
	uint32_t cs_idletime;
	uint32_t packet_len;
	uint32_t packet_loop;
	enum spi_cpol cpol;
	enum spi_cpha cpha;
	enum spi_mlsb tx_mlsb;
	enum spi_mlsb rx_mlsb;
	enum spi_endian tx_endian;
	enum spi_endian rx_endian;
	enum spi_transfer_mode com_mod;
	enum spi_pause_mode pause;
	enum spi_finish_intr finish_intr;
	enum spi_deassert_mode deassert;
	enum spi_tckdly tckdly;
} mt8135_spi_cfg;

/* Serve as CBFS media source */
int initialize_mt8135_spi_cbfs_media(struct cbfs_media *media,
				     void *buffer_address,
				     size_t buffer_size);
#endif
