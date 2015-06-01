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

#ifndef SOC_MEDIATEK_MT8173_I2C_H
#define SOC_MEDIATEK_MT8173_I2C_H

#include <arch/io.h>
#include <console/console.h>
#include <device/i2c.h>

/* I2C Configuration */

enum {
	I2C_TIMING_DEFAULT_VALUE = 0x1303,
	I2C_HS_DEFAULT_VALUE	 = 0x2700,
};

enum {
	I2C_WRITE_MODE		= 0,
	I2C_READ_MODE		= 1,
	I2C_WRITE_READ_MODE	= 2,
};

enum {
	I2C_DMA_CON_TX          = 0x0,
	I2C_DMA_CON_RX          = 0x1,
	I2C_DMA_START_EN        = 0x1,
	I2C_DMA_INT_FLAG_NONE   = 0x0,
	I2C_DMA_CLR_FLAG        = 0x0,
	I2C_DMA_FLUSH_FLAG      = 0x1,
};

enum DMA_REGS_OFFSET {
	OFFSET_INT_FLAG = 0x0,
	OFFSET_INT_EN = 0x04,
	OFFSET_EN = 0x08,
	OFFSET_RST = 0x0c,
	OFFSET_FLUSH = 0x14,
	OFFSET_CON = 0x18,
	OFFSET_TX_MEM_ADDR = 0x1c,
	OFFSET_RX_MEM_ADDR = 0x20,
	OFFSET_TX_LEN = 0x24,
	OFFSET_RX_LEN = 0x28,
};

struct mtk_i2c {
	uint32_t i2c_addr;              /* The base address of i2c register */
	uint32_t i2c_dma_addr;          /* The base address of i2c dma register */
};

/* I2C Register */
enum {
	MTK_I2C_DATA_PORT = 0x0000,
	MTK_I2C_SLAVE_ADDR = 0x0004,
	MTK_I2C_INTR_MASK = 0x0008,
	MTK_I2C_INTR_STAT = 0x000c,
	MTK_I2C_CONTROL = 0x0010,
	MTK_I2C_TRANSFER_LEN = 0x0014,
	MTK_I2C_TRANSAC_LEN = 0x0018,
	MTK_I2C_DELAY_LEN = 0x001c,
	MTK_I2C_TIMING = 0x0020,
	MTK_I2C_START = 0x0024,
	MTK_I2C_EXT_CONF = 0x0028,
	MTK_I2C_FIFO_STAT = 0x0030,
	MTK_I2C_FIFO_THRESH = 0x0034,
	MTK_I2C_FIFO_ADDR_CLR = 0x0038,
	MTK_I2C_IO_CONFIG = 0x0040,
	MTK_I2C_DEBUG = 0x0044,
	MTK_I2C_HS = 0x0048,
	MTK_I2C_SOFTRESET = 0x0050,
	MTK_I2C_PATH_DIR = 0x0060,
	MTK_I2C_DEBUGSTAT = 0x0064,
	MTK_I2C_DEBUGCTRL = 0x0068,
	MTK_I2C_TRANSFER_AUX_LEN = 0x006c
};

enum {
	I2C_TRANS_LEN_MASK = (0xff),
	I2C_TRANS_AUX_LEN_MASK = (0x1f << 8),
	I2C_CONTROL_MASK = (0x3f << 1)
};

/* Register mask */
enum {
	I2C_3_BIT_MASK = 0x07,
	I2C_4_BIT_MASK = 0x0f,
	I2C_8_BIT_MASK = 0xff,
	I2C_6_BIT_MASK = 0x3f,
	I2C_MASTER_READ = 0x01,
	I2C_MASTER_WRITE = 0x00,
	I2C_FIFO_THRESH_MASK = 0x07,
	I2C_AUX_LEN_MASK = 0x1f00,
	I2C_CTL_RS_STOP_BIT = 0x02,
	I2C_CTL_DMA_EN_BIT = 0x04,
	I2C_CTL_ACK_ERR_DET_BIT = 0x20,
	I2C_CTL_CLK_EXT_EN_BIT = 0x08,
	I2C_CTL_DIR_CHANGE_BIT = 0x10,
	I2C_CTL_TRANSFER_LEN_CHG_BIT = 0x40,
	I2C_DATA_READ_ADJ_BIT = 0x8000,
	I2C_SDA_MODE_BIT = 0x02,
	I2C_SCL_MODE_BIT = 0x01,
	I2C_ARBITRATION_BIT = 0x01,
	I2C_CLOCK_SYNC_BIT = 0x02,
	I2C_BUS_DETECT_EN_BIT = 0x04,
	I2C_HS_EN_BIT = 0x01,
	I2C_HS_NACK_ERR_DET_EN_BIT = 0x02,
	I2C_BUS_BUSY_DET_BIT = 0x04,
	I2C_HS_MASTER_CODE_MASK = 0x70,
	I2C_HS_STEP_CNT_DIV_MASK = 0x700,
	I2C_HS_SAMPLE_CNT_DIV_MASK = 0x7000,
	I2C_FIFO_FULL_STATUS = 0x01,
	I2C_FIFO_EMPTY_STATUS = 0x02,

	I2C_DEBUG = (1 << 3),
	I2C_HS_NACKERR = (1 << 2),
	I2C_ACKERR = (1 << 1),
	I2C_TRANSAC_COMP = (1 << 0),

	I2C_TX_THR_OFFSET = 8,
	I2C_RX_THR_OFFSET = 0
};

/* i2c control bits */
enum {
	TRANS_LEN_CHG = (1 << 6),
	ACK_ERR_DET_EN = (1 << 5),
	DIR_CHG = (1 << 4),
	CLK_EXT = (1 << 3),
	DMA_EN = (1 << 2),
	REPEATED_START_FLAG = (1 << 1),
	STOP_FLAG = (0 << 1)
};

/* I2C Status Code */

enum {
	I2C_OK = 0x0000,
	I2C_SET_SPEED_FAIL_OVER_SPEED = 0xA001,
	I2C_TRANSFER_INVALID_LENGTH = 0xA002,
	I2C_TRANSFER_FAIL_HS_NACKERR = 0xA003,
	I2C_TRANSFER_FAIL_ACKERR = 0xA004,
	I2C_TRANSFER_FAIL_TIMEOUT = 0xA005,
	I2C_TRANSFER_INVALID_ARGUMENT = 0xA006
};

/* -----------------------------------------------------------------------
 * read interface: Read bytes
 *   bus:     i2c bus number.
 *   addr:    i2c device address.
 *   buffer:  Where to read/write the data.
 *   len:     How many bytes to read/write
 *   Returns: ERROR_CODE
 */
uint32_t mtk_i2c_read(uint8_t bus, uint8_t addr, uint8_t *buffer, uint32_t len);

/* -----------------------------------------------------------------------
 * write interface: Write bytes
 *   bus:     i2c bus number.
 *   addr:    i2c device address.
 *   buffer:  Where to read/write the data.
 *   len:     How many bytes to read/write
 *   Returns: ERROR_CODE
 */
uint32_t mtk_i2c_write(uint8_t bus, uint8_t addr, uint8_t *buffer, uint32_t len);

/* -----------------------------------------------------------------------
 * write then read back interface: Write bytes then read bytes,
 * and i2c repeat start bit will be sent between write and read.
 *   bus:       i2c bus number.
 *   addr:      i2c device address.
 *   buffer:    Where to read/write the data.
 *   write_len: How many bytes to write
 *   read_len:  How many bytes to read
 *   Returns:   ERROR_CODE
 */
uint32_t mtk_i2c_write_read(uint8_t bus, uint8_t addr, uint8_t *buffer,
			    uint32_t write_len, uint32_t read_len);

#endif /* SOC_MEDIATEK_MT8173_I2C_H */
