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

#include <arch/cache.h>
#include <assert.h>
#include <delay.h>
#include <string.h>
#include <symbols.h>
#include <timer.h>
#include <soc/addressmap.h>
#include <soc/i2c.h>

static struct mtk_i2c i2c[7] = {
	/* i2c0 setting */
	{
		.i2c_addr = I2C_BASE,
		.i2c_dma_addr = I2C_DMA_BASE + 0x80,
	},

	/* i2c1 setting */
	{
		.i2c_addr = I2C_BASE + 0x1000,
		.i2c_dma_addr = I2C_DMA_BASE + 0x100,
	},

	/* i2c2 setting */
	{
		.i2c_addr = I2C_BASE + 0x2000,
		.i2c_dma_addr = I2C_DMA_BASE + 0x180,
	},

	/* i2c3 setting */
	{
		.i2c_addr = I2C_BASE + 0x9000,
		.i2c_dma_addr = I2C_DMA_BASE + 0x200,
	},

	 /* i2c4 setting */
	{
		.i2c_addr = I2C_BASE + 0xa000,
		.i2c_dma_addr = I2C_DMA_BASE + 0x280,
	},

	/* i2c5 is reserved for internal use. */
	{
	},

	/* i2c6 setting */
	{
		.i2c_addr = I2C_BASE + 0xc000,
		.i2c_dma_addr = I2C_DMA_BASE,
	}
};

#define I2CTAG                "[I2C][PL] "

#if CONFIG_DEBUG_I2C
#define I2CLOG(fmt, arg...)   printk(BIOS_INFO, I2CTAG fmt, ##arg)
#else
#define I2CLOG(fmt, arg...)
#define I2CMSG(fmt, arg...)
#endif /* CONFIG_DEBUG_I2C */

#define I2CERR(fmt, arg...)   printk(BIOS_ERR, I2CTAG fmt, ##arg)

static const void *I2CADDR_R(uint32_t addr, uint32_t offset)
{
	return (const void *)(uintptr_t)(addr + offset);
}

static void *I2CADDR_W(uint32_t addr, uint32_t offset)
{
	return (void *)(uintptr_t)(addr + offset);
}

/* Register Settings */
static inline void I2C_START_TRANSAC(uint32_t i2c_addr)
{
	write32(I2CADDR_W(i2c_addr, MTK_I2C_START), 0x1);
}

static inline void I2C_FIFO_CLR_ADDR(uint32_t i2c_addr)
{
	write32(I2CADDR_W(i2c_addr, MTK_I2C_FIFO_ADDR_CLR), 0x1);
}

static inline uint16_t I2C_FIFO_OFFSET(uint32_t i2c_addr)
{
	return (read32(I2CADDR_R(i2c_addr, MTK_I2C_FIFO_STAT)) >> 4 & 0xf);
}

static inline uint16_t I2C_FIFO_IS_EMPTY(uint32_t i2c_addr)
{
	return (read32(I2CADDR_R(i2c_addr, MTK_I2C_FIFO_STAT)) >> 0 & 0x1);
}

static inline void I2C_SOFTRESET(uint32_t i2c_addr)
{
	write32(I2CADDR_W(i2c_addr, MTK_I2C_SOFTRESET), 0x1);
}

static inline void I2C_PATH_PMIC(uint32_t i2c_addr)
{
	write32(I2CADDR_W(i2c_addr, MTK_I2C_PATH_DIR), 0x1);
}

static inline void I2C_PATH_NORMAL(uint32_t i2c_addr)
{
	write32(I2CADDR_W(i2c_addr, MTK_I2C_PATH_DIR), 0x0);
}

static inline uint16_t I2C_INTR_STATUS(uint32_t i2c_addr)
{
	return read32(I2CADDR_R(i2c_addr, MTK_I2C_INTR_STAT));
}

static inline void I2C_SET_FIFO_THRESH(uint32_t i2c_addr, uint8_t tx, uint8_t rx)
{
	uint16_t tmp = (((tx) & 0x7) << I2C_TX_THR_OFFSET) |
			(((rx) & 0x7) << I2C_RX_THR_OFFSET);
	write32(I2CADDR_W(i2c_addr, MTK_I2C_FIFO_THRESH), tmp);
}

static inline void I2C_SET_INTR_MASK(uint32_t i2c_addr, uint16_t mask)
{
	write32(I2CADDR_W(i2c_addr, MTK_I2C_INTR_MASK), mask);
}

static inline void I2C_CLR_INTR_MASK(uint32_t i2c_addr, uint16_t mask)
{
	uint16_t tmp = read32(I2CADDR_R(i2c_addr, MTK_I2C_INTR_MASK));
	tmp &= ~(mask);
	write32(I2CADDR_W(i2c_addr, MTK_I2C_INTR_MASK), tmp);
}

static inline void I2C_SET_SLAVE_ADDR(uint32_t i2c_addr, uint16_t addr)
{
	write32(I2CADDR_W(i2c_addr, MTK_I2C_SLAVE_ADDR), addr);
}

static inline void I2C_SET_TRANS_LEN(uint32_t i2c_addr, uint16_t len)
{
	write32(I2CADDR_W(i2c_addr, MTK_I2C_TRANSFER_LEN), len);
}

static inline void I2C_SET_TRANS_AUX_LEN(uint32_t i2c_addr, uint16_t len)
{
	write32(I2CADDR_W(i2c_addr, MTK_I2C_TRANSFER_AUX_LEN), len);
}

static inline void I2C_SET_TRANSAC_LEN(uint32_t i2c_addr, uint16_t len)
{
	write32(I2CADDR_W(i2c_addr, MTK_I2C_TRANSAC_LEN), len);
}

static inline void I2C_SET_TRANS_DELAY(uint32_t i2c_addr, uint16_t delay)
{
	write32(I2CADDR_W(i2c_addr, MTK_I2C_DELAY_LEN), delay);
}

static inline void I2C_SET_TRANS_CTRL(uint32_t i2c_addr, uint16_t ctrl)
{
	uint16_t tmp = read32(I2CADDR_R(i2c_addr, MTK_I2C_CONTROL)) &
			      ~I2C_CONTROL_MASK;
	tmp |= ((ctrl) & I2C_CONTROL_MASK);
	write32(I2CADDR_W(i2c_addr, MTK_I2C_CONTROL), tmp);
}

static inline void I2C_SET_HS_MODE(uint32_t i2c_addr, uint16_t on_off)
{
	uint16_t tmp = read32(I2CADDR_R(i2c_addr, MTK_I2C_HS)) & ~0x1;
	tmp |= (on_off & 0x1);
	write32(I2CADDR_W(i2c_addr, MTK_I2C_HS), tmp);
}

static inline void I2C_READ_BYTE(uint32_t i2c_addr, uint8_t *byte)
{
	*byte = read32(I2CADDR_R(i2c_addr, MTK_I2C_DATA_PORT));
}

static inline void I2C_WRITE_BYTE(uint32_t i2c_addr, uint8_t byte)
{
	write32(I2CADDR_W(i2c_addr, MTK_I2C_DATA_PORT), byte);
}

static inline void I2C_CLR_INTR_STATUS(uint32_t i2c_addr, uint16_t status)
{
	write32(I2CADDR_W(i2c_addr, MTK_I2C_INTR_STAT), status);
}

static inline void I2C_DMA_RESET(uint32_t i2c_dma_addr)
{
	write32(I2CADDR_W(i2c_dma_addr, OFFSET_RST), 0x1);
	udelay(50);
	write32(I2CADDR_W(i2c_dma_addr, OFFSET_RST), 0x2);
	udelay(50);
	write32(I2CADDR_W(i2c_dma_addr, OFFSET_RST), 0x0);
	udelay(50);
}

static inline void mtk_i2c_dump_info(uint8_t bus)
{
	I2CLOG("I2C register:\nSLAVE_ADDR %x\nINTR_MASK %x\nINTR_STAT %x\n"
	       "CONTROL %x\nTRANSFER_LEN %x\nTRANSAC_LEN %x\nDELAY_LEN %x\n"
	       "TIMING %x\nSTART %x\nFIFO_STAT %x\nIO_CONFIG %x\nHS %x\n"
	       "DEBUGSTAT %x\nEXT_CONF %x\nPATH_DIR %x\n",
		(read32(I2CADDR_R(i2c[bus].i2c_addr, MTK_I2C_SLAVE_ADDR))),
		(read32(I2CADDR_R(i2c[bus].i2c_addr, MTK_I2C_INTR_MASK))),
		(read32(I2CADDR_R(i2c[bus].i2c_addr, MTK_I2C_INTR_STAT))),
		(read32(I2CADDR_R(i2c[bus].i2c_addr, MTK_I2C_CONTROL))),
		(read32(I2CADDR_R(i2c[bus].i2c_addr, MTK_I2C_TRANSFER_LEN))),
		(read32(I2CADDR_R(i2c[bus].i2c_addr, MTK_I2C_TRANSAC_LEN))),
		(read32(I2CADDR_R(i2c[bus].i2c_addr, MTK_I2C_DELAY_LEN))),
		(read32(I2CADDR_R(i2c[bus].i2c_addr, MTK_I2C_TIMING))),
		(read32(I2CADDR_R(i2c[bus].i2c_addr, MTK_I2C_START))),
		(read32(I2CADDR_R(i2c[bus].i2c_addr, MTK_I2C_FIFO_STAT))),
		(read32(I2CADDR_R(i2c[bus].i2c_addr, MTK_I2C_IO_CONFIG))),
		(read32(I2CADDR_R(i2c[bus].i2c_addr, MTK_I2C_HS))),
		(read32(I2CADDR_R(i2c[bus].i2c_addr, MTK_I2C_DEBUGSTAT))),
		(read32(I2CADDR_R(i2c[bus].i2c_addr, MTK_I2C_EXT_CONF))),
		(read32(I2CADDR_R(i2c[bus].i2c_addr, MTK_I2C_PATH_DIR))));

	I2CLOG("addr address %x\n", i2c[bus].i2c_addr);
}

static uint32_t mtk_i2c_transfer(uint8_t bus, struct i2c_seg *seg, uint8_t read)
{
	uint32_t ret_code = I2C_OK;
	uint16_t status;
	uint32_t time_out_val = 0;
	uint8_t  addr;
	uint32_t write_len = 0;
	uint32_t read_len = 0;
	uint8_t *write_buffer = NULL;
	uint8_t *read_buffer = NULL;
	uint32_t i2c_dma_addr;
	uint32_t i2c_addr;
	struct stopwatch sw;
	uint32_t sw_run_duration;

	i2c_addr = i2c[bus].i2c_addr;
	i2c_dma_addr = i2c[bus].i2c_dma_addr;
	addr = seg[0].chip;

	if (read == I2C_WRITE_MODE) {
		assert(seg[0].len > 0 && seg[0].len <=255);
		write_len = seg[0].len;
		write_buffer = seg[0].buf;
	} else if (read == I2C_READ_MODE) {
		assert(seg[0].len > 0 && seg[0].len <=255);
		read_len = seg[0].len;
		read_buffer = seg[0].buf;
	/* read = 2 is for supporting MTK i2c write then read mode. */
	} else if (read == I2C_WRITE_READ_MODE) {
		assert(seg[0].len > 0 && seg[0].len <=255);
		assert(seg[1].len > 0 && seg[1].len <=255);
		write_len = seg[0].len;
		read_len = seg[1].len;
		write_buffer = seg[0].buf;
		read_buffer = seg[1].buf;
	} else {
		return I2C_TRANSFER_INVALID_ARGUMENT;
	}

	/* setting path direction and clock first all */
	I2C_PATH_NORMAL(i2c_addr);

	write32(I2CADDR_W(i2c_addr, MTK_I2C_TIMING), I2C_TIMING_DEFAULT_VALUE);
	write32(I2CADDR_W(i2c_addr, MTK_I2C_HS), I2C_HS_DEFAULT_VALUE);
	I2C_SET_HS_MODE(i2c_addr, 1);

	/* Clear interrupt status */
	I2C_CLR_INTR_STATUS(i2c_addr, I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);
	I2C_FIFO_CLR_ADDR(i2c_addr);

	/* Enable interrupt */
	I2C_SET_INTR_MASK(i2c_addr, I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);

	if (read32(I2CADDR_R(i2c_dma_addr, OFFSET_EN)) == 1) {
		I2CERR("dma is not ready, need reset.\n");
		I2C_DMA_RESET(i2c_dma_addr);
	}

	if (read == I2C_WRITE_MODE) {
		memcpy(_dma_coherent, write_buffer, write_len);

		/* control registers */
		I2C_SET_TRANS_CTRL(i2c_addr, ACK_ERR_DET_EN | DMA_EN | CLK_EXT |
				   REPEATED_START_FLAG);

		/* Set transfer and transaction len */
		I2C_SET_TRANSAC_LEN(i2c_addr, 1);
		I2C_SET_TRANS_LEN(i2c_addr, write_len);

		/* set i2c write salve address*/
		I2C_SET_SLAVE_ADDR(i2c_addr, addr << 1);

		/* Prepare buffer data to start transfer */
		write32(I2CADDR_W(i2c_dma_addr, OFFSET_CON), I2C_DMA_CON_TX);
		write32(I2CADDR_W(i2c_dma_addr, OFFSET_TX_MEM_ADDR), (uintptr_t)_dma_coherent);
		write32(I2CADDR_W(i2c_dma_addr, OFFSET_TX_LEN), write_len);

	} else if (read == I2C_READ_MODE) {
		/* control registers */
		I2C_SET_TRANS_CTRL(i2c_addr, ACK_ERR_DET_EN | DMA_EN | CLK_EXT |
				   REPEATED_START_FLAG);

		/* Set transfer and transaction len */
		I2C_SET_TRANSAC_LEN(i2c_addr, 1);
		I2C_SET_TRANS_LEN(i2c_addr, read_len);

		/* set i2c read salve address*/
		I2C_SET_SLAVE_ADDR(i2c_addr, (addr << 1 | 0x1));

		/* Prepare buffer data to start transfer */
		write32(I2CADDR_W(i2c_dma_addr, OFFSET_CON), I2C_DMA_CON_RX);
		write32(I2CADDR_W(i2c_dma_addr, OFFSET_RX_MEM_ADDR), (uintptr_t)_dma_coherent);
		write32(I2CADDR_W(i2c_dma_addr, OFFSET_RX_LEN), read_len);

	/* read = 2 is for supporting MTK i2c write then read mode. */
	} else if (read == I2C_WRITE_READ_MODE) {
		memcpy(_dma_coherent, write_buffer, write_len);

		/* control registers */
		I2C_SET_TRANS_CTRL(i2c_addr, DIR_CHG | ACK_ERR_DET_EN | DMA_EN |
				   CLK_EXT | REPEATED_START_FLAG);

		/* Set transfer and transaction len */
		I2C_SET_TRANS_LEN(i2c_addr, write_len);
		I2C_SET_TRANS_AUX_LEN(i2c_addr, read_len);
		I2C_SET_TRANSAC_LEN(i2c_addr, 2);

		/* set i2c write salve address*/
		I2C_SET_SLAVE_ADDR(i2c_addr, addr << 1);

		/* Prepare buffer data to start transfer */
		write32(I2CADDR_W(i2c_dma_addr, OFFSET_CON), I2C_DMA_CLR_FLAG);
		write32(I2CADDR_W(i2c_dma_addr, OFFSET_TX_MEM_ADDR), (uintptr_t)_dma_coherent);
		write32(I2CADDR_W(i2c_dma_addr, OFFSET_TX_LEN), write_len);
		write32(I2CADDR_W(i2c_dma_addr, OFFSET_RX_MEM_ADDR), (uintptr_t)_dma_coherent);
		write32(I2CADDR_W(i2c_dma_addr, OFFSET_RX_LEN), read_len);
	}


	write32(I2CADDR_W(i2c_dma_addr, OFFSET_INT_FLAG), I2C_DMA_CLR_FLAG);
	write32(I2CADDR_W(i2c_dma_addr, OFFSET_EN), I2C_DMA_START_EN);

	/* start trnasfer transaction */
	I2C_START_TRANSAC(i2c_addr);

	sw_run_duration = 1000;

	stopwatch_init_msecs_expire(&sw, sw_run_duration);

	/* polling mode : see if transaction complete */
	while (1) {
		status = I2C_INTR_STATUS(i2c_addr);
		if (status & I2C_HS_NACKERR) {
			ret_code = I2C_TRANSFER_FAIL_HS_NACKERR;
			I2CERR("[i2c%d transfer] transaction NACK error\n", bus);
			mtk_i2c_dump_info(bus);
			break;
		} else if (status & I2C_ACKERR) {
			ret_code = I2C_TRANSFER_FAIL_ACKERR;
			I2CERR("[i2c%d transfer] transaction ACK error\n", bus);
			mtk_i2c_dump_info(bus);
			break;
		} else if (status & I2C_TRANSAC_COMP) {
			ret_code = I2C_OK;
			memcpy(read_buffer, _dma_coherent, read_len);
			break;
		}

		if (stopwatch_expired(&sw)) {
			ret_code = I2C_TRANSFER_FAIL_TIMEOUT;
			I2CERR("[i2c%d transfer] transaction timeout:%d\n", bus, time_out_val);
			mtk_i2c_dump_info(bus);
			break;
		}
	}

	I2C_CLR_INTR_STATUS(i2c_addr, I2C_TRANSAC_COMP | I2C_ACKERR |
			    I2C_HS_NACKERR);

	/* clear bit mask */
	I2C_CLR_INTR_MASK(i2c_addr, I2C_HS_NACKERR | I2C_ACKERR |
			  I2C_TRANSAC_COMP);

	sw_run_duration = 50;

	stopwatch_init_usecs_expire(&sw, sw_run_duration);
	while (read32(I2CADDR_R(i2c_dma_addr, OFFSET_EN)) == 1) {
		if (stopwatch_expired(&sw)) {
			I2CERR("wait DMA ready timeout.\n");
			break;
		}
	}

	/* reset the i2c controller for next i2c transfer. */
	I2C_SOFTRESET(i2c_addr);

	I2C_DMA_RESET(i2c_dma_addr);

	return ret_code;
}

static uint8_t mtk_i2c_should_combine(struct i2c_seg *seg, int left_count)
{
	if (left_count >= 2 && seg[0].read == 0 && seg[1].read == 1 &&
	    seg[0].chip == seg[1].chip)
		return 1;
	else
		return 0;
}

int platform_i2c_transfer(unsigned bus, struct i2c_seg *segments, int seg_count)
{
	int ret = 0;
	int i;
	int left_count = seg_count;
	int read;

	for (i = 0; i < seg_count; i++) {
		if (mtk_i2c_should_combine(&segments[i], left_count)) {
			read = I2C_WRITE_READ_MODE;
			left_count -= 2;
		} else {
			read = segments[i].read;
			left_count--;
		}

		ret = mtk_i2c_transfer(bus, &segments[i], read);

		if (ret)
			break;

		if (read == I2C_WRITE_READ_MODE)
			i++;
	}

	return ret;
}
