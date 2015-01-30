/*
 * Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of Google, Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <soc/i2c.h>

#if CONFIG_DEBUG_I2C
#define I2CTAG                "[I2C][PL] "
#define I2CLOG(fmt, arg...)   printk(BIOS_INFO, I2CTAG fmt, ##arg)
#define I2CMSG(fmt, arg...)   printk(BIOS_INFO, fmt, ##arg)
#define I2CERR(fmt, arg...)   printk(BIOS_ERR, I2CTAG fmt, ##arg)
#else
#define I2CLOG(fmt, arg...)
#define I2CMSG(fmt, arg...)
#define I2CERR(fmt, arg...)
#endif /* CONFIG_DEBUG_I2C */

static inline void I2C_PMIC_WR(uint32_t addr, uint32_t data)
{
	pwrap_write((uint32_t)addr, data);
}

static inline int I2C_PMIC_RD(uint32_t addr)
{
	uint32_t ext_data;

	return pwrap_read((uint32_t)addr, (uint32_t *)(&ext_data)) != 0
				? -1 : ext_data;
}

static struct mtk_i2c_t g_i2c[7];

uint32_t mtk_i2c_set_speed(uint8_t channel, uint32_t clock,
			   I2C_SPD_MODE mode, uint32_t khz)
{
	uint32_t ret_code = I2C_OK;
	uint32_t i2c_base = I2C0_BASE + channel * 0x1000;

	uint16_t sample_cnt_div;
	uint16_t step_cnt_div;
	uint16_t max_step_cnt_div =
	    (mode == HS_MODE) ? MAX_HS_STEP_CNT_DIV : MAX_STEP_CNT_DIV;
	uint32_t tmp;
	uint32_t sclk;

	uint32_t diff;
	uint32_t min_diff = I2C_CLK_RATE;
	uint16_t sample_div = MAX_SAMPLE_CNT_DIV;
	uint16_t step_div = max_step_cnt_div;

	for (sample_cnt_div = 1; sample_cnt_div <= MAX_SAMPLE_CNT_DIV;
		sample_cnt_div++) {

		for (step_cnt_div = 1; step_cnt_div <= max_step_cnt_div;
			step_cnt_div++) {
			sclk = (clock >> 1) /
				(sample_cnt_div * step_cnt_div);
			if (sclk > khz)
				continue;
			diff = khz - sclk;

			if (diff < min_diff) {
				min_diff = diff;
				sample_div = sample_cnt_div;
				step_div = step_cnt_div;
			}
		}
	}

	sample_cnt_div = sample_div;
	step_cnt_div = step_div;

	sclk = clock / (2 * sample_cnt_div * step_cnt_div);
	if (sclk > khz) {
		ret_code = I2C_SET_SPEED_FAIL_OVER_SPEED;
		return ret_code;
	}

	step_cnt_div--;
	sample_cnt_div--;

	if (mode == HS_MODE) {
		tmp = read32((void *)(uintptr_t)(i2c_base + MTK_I2C_HS))
				& ((0x7 << 12) | (0x7 << 8));
		tmp = (sample_cnt_div & 0x7) << 12 | (step_cnt_div & 0x7) << 8 |
		      tmp;
		write32(tmp, (void *)(uintptr_t)(i2c_base + MTK_I2C_HS));
		I2C_SET_HS_MODE(i2c_base, 1);
	} else {
		tmp = read32((void *)(uintptr_t)(i2c_base + MTK_I2C_TIMING))
				& ~((0x7 << 8) | (0x3f << 0));
		tmp = (sample_cnt_div & 0x7) << 8 | (step_cnt_div & 0x3f) << 0
				| tmp;
		write32(tmp, (void *)(uintptr_t)(i2c_base + MTK_I2C_TIMING));
		I2C_SET_HS_MODE(i2c_base, 0);
	}

	I2CLOG("[i2c%d set speed] Set sclk to %ld khz (orig: %ld khz)\n",
	       channel, sclk, khz);
	I2CLOG("[i2c_set_speed] I2C Timing parameter sample_cnt_div(%d)\n",
	       sample_cnt_div);
	I2CLOG("step_cnt_div(%d)\n", step_cnt_div);

	return ret_code;
}

static inline void mtk_i2c_dump_info(struct mtk_i2c_t *i2c)
{
	uint32_t i2c_base;

	i2c_base = I2C0_BASE + i2c->id * 0x1000;

	I2CERR("I2C register:\nSLAVE_ADDR %x\nINTR_MASK %x\nINTR_STAT %x\n"
	       "CONTROL %x\nTRANSFER_LEN %x\nTRANSAC_LEN %x\nDELAY_LEN %x\n"
	       "TIMING %x\nSTART %x\nFIFO_STAT %x\nIO_CONFIG %x\nHS %x\n"
	       "DEBUGSTAT %x\nEXT_CONF %x\nPATH_DIR %x\n",
		(read32((void *)(uintptr_t)(i2c_base + MTK_I2C_SLAVE_ADDR))),
		(read32((void *)(uintptr_t)(i2c_base + MTK_I2C_INTR_MASK))),
		(read32((void *)(uintptr_t)(i2c_base + MTK_I2C_INTR_STAT))),
		(read32((void *)(uintptr_t)(i2c_base + MTK_I2C_CONTROL))),
		(read32((void *)(uintptr_t)(i2c_base + MTK_I2C_TRANSFER_LEN))),
		(read32((void *)(uintptr_t)(i2c_base + MTK_I2C_TRANSAC_LEN))),
		(read32((void *)(uintptr_t)(i2c_base + MTK_I2C_DELAY_LEN))),
		(read32((void *)(uintptr_t)(i2c_base + MTK_I2C_TIMING))),
		(read32((void *)(uintptr_t)(i2c_base + MTK_I2C_START))),
		(read32((void *)(uintptr_t)(i2c_base + MTK_I2C_FIFO_STAT))),
		(read32((void *)(uintptr_t)(i2c_base + MTK_I2C_IO_CONFIG))),
		(read32((void *)(uintptr_t)(i2c_base + MTK_I2C_HS))),
		(read32((void *)(uintptr_t)(i2c_base + MTK_I2C_DEBUGSTAT))),
		(read32((void *)(uintptr_t)(i2c_base + MTK_I2C_EXT_CONF))),
		(read32((void *)(uintptr_t)(i2c_base + MTK_I2C_PATH_DIR))));

	/* 8135 side and PMIC side clock */
	I2CERR("Clock %s\n", (((read32((void *)0x10003018) >> 26) |
			     (read32((void *)0x1000301c) & 0x1 << 6)) &
			     (1 << i2c->id)) ?  "disable" : "enable");
	if (i2c->id >= 4)
		I2CERR("Clock PMIC %s\n",
		       ((I2C_PMIC_RD(0x011A) & 0x7) & (1 << (i2c->id - 4))) ?
		       "disable" : "enable");

	I2CERR("base address %x\n", (uint32_t)i2c_base);
	return;

}

uint32_t mtk_i2c_read(struct mtk_i2c_t *i2c, uint8_t *buffer, uint32_t len)
{
	uint32_t ret_code = I2C_OK;
	uint32_t i2c_clk;
	uint8_t *ptr = buffer;
	uint16_t status;
	int ret = len;
	uint32_t time_out_val = 0;
	uint32_t count = 0;
	uint32_t temp_len = 0;
	uint32_t total_count = 0;
	uint8_t tpm_data_fifo_v = 0x5;
	uint32_t i2c_base;

	i2c_base = I2C0_BASE + i2c->id * 0x1000;

	/* CHECKME. mt65xx doesn't support len = 0. */
	if ((len == 0) || (len > 255)) {
		I2CLOG("[i2c_read] I2C doesn't support len = %d.\n", len);
		return I2C_READ_FAIL_ZERO_LENGTH;
	}

	if (i2c->dir == 0) {
		I2C_PATH_NORMAL(i2c_base);
		i2c_clk = I2C_CLK_RATE;
	} else {
		I2C_PATH_PMIC(i2c_base);
		i2c_clk = I2C_CLK_RATE_PMIC;
	}

	count = len / 8;
	if (len % 8 != 0)
		count += 1;

	if (count > 1)
		total_count = count;

	while (count) {
		/* FOR TPM */
		if (count < total_count && count > 0 && i2c->id == 6)
		{
			mtk_i2c_write(i2c, &tpm_data_fifo_v, 1);
		}

		if (count == 1) {
			temp_len = len % 8;
			if (temp_len == 0)
				temp_len = 8;
		} else
		{
			temp_len = 8;
		}
		I2C_SET_TRANS_LEN(i2c_base, temp_len);

		I2C_CLR_INTR_STATUS(i2c_base, I2C_TRANSAC_COMP | I2C_ACKERR
					| I2C_HS_NACKERR);
		/* setting speed */
		mtk_i2c_set_speed(i2c->id, i2c_clk, i2c->mode, i2c->speed);
		if (i2c->speed <= 100) {
			write32(0x8001, (void *)(uintptr_t)(i2c_base + MTK_I2C_EXT_CONF));
		}
		/* control registers */
		I2C_SET_SLAVE_ADDR(i2c_base, ((i2c->addr<<1) | 0x1));

		/* I2C_SET_TRANS_LEN(len); */
		I2C_SET_TRANSAC_LEN(i2c_base, 1);
		I2C_SET_INTR_MASK(i2c_base, I2C_HS_NACKERR | I2C_ACKERR
					| I2C_TRANSAC_COMP);
		I2C_FIFO_CLR_ADDR(i2c_base);
		I2C_SET_TRANS_CTRL(i2c_base, ACK_ERR_DET_EN
				| (i2c->is_clk_ext_disable ?  0 : CLK_EXT)
				| (i2c->is_rs_enable
				? REPEATED_START_FLAG : STOP_FLAG));

		/* start trnasfer transaction */
		I2C_START_TRANSAC(i2c_base);

		/* polling mode : see if transaction complete */
		while (1) {
			status = I2C_INTR_STATUS(i2c_base);

			if (status & I2C_TRANSAC_COMP &&
			    (!I2C_FIFO_IS_EMPTY(i2c_base))) {
				ret = 0;
				ret_code = I2C_OK;
				break;
			} else if (status & I2C_HS_NACKERR) {
				ret = 1;
				ret_code = I2C_READ_FAIL_HS_NACKERR;
				I2CERR
				    ("[i2c%d read] transaction NACK error (%x)\n",
				     i2c->id, status);
				mtk_i2c_dump_info(i2c);
				break;
			} else if (status & I2C_ACKERR) {
				ret = 2;
				ret_code = I2C_READ_FAIL_ACKERR;
				I2CERR
				    ("[i2c%d read] transaction ACK error (%x)\n",
				     i2c->id, status);
				mtk_i2c_dump_info(i2c);
				break;
			} else if (time_out_val > 100000) {
				ret = 3;
				ret_code = I2C_READ_FAIL_TIMEOUT;
				I2C_SOFTRESET(i2c_base);
				I2CERR("[i2c%d read] transaction timeout:%d\n",
				       i2c->id, time_out_val);
				mtk_i2c_dump_info(i2c);
				break;
			}
			time_out_val++;
		}

		I2C_CLR_INTR_STATUS(i2c_base, I2C_TRANSAC_COMP | I2C_ACKERR
					| I2C_HS_NACKERR);

		if (!ret) {
			while (temp_len > 0) {
				I2C_READ_BYTE(i2c_base, ptr);
				ptr++;
				temp_len--;
			}
		}

		/* clear bit mask */
		I2C_CLR_INTR_MASK(i2c_base, I2C_HS_NACKERR | I2C_ACKERR
					| I2C_TRANSAC_COMP);

		I2C_SOFTRESET(i2c_base);
		count--;
	}

	return ret_code;
}

uint32_t mtk_i2c_write(struct mtk_i2c_t *i2c, uint8_t *buffer, uint32_t len)
{
	uint32_t ret_code = I2C_OK;
	uint32_t i2c_clk;
	uint8_t *ptr = buffer;

	uint16_t status;
	uint32_t time_out_val = 0;
	uint32_t count = 0;
	uint32_t total_count = 0;
	uint32_t temp_len = 0;
	uint8_t tpm_data_fifo_v = 0x5;
	uint32_t i2c_base;

	i2c_base = I2C0_BASE + i2c->id * 0x1000;

	/* CHECKME. mt65xx doesn't support len = 0. */
	if ((len == 0) || (len > 255)) {
		I2CLOG("[i2c_write] I2C doesn't support len = %d.\n", len);
		return I2C_WRITE_FAIL_ZERO_LENGTH;
	}

	if (i2c->dir == 0) {
		I2C_PATH_NORMAL(i2c_base);
		i2c_clk = I2C_CLK_RATE;
	} else {
		I2C_PATH_PMIC(i2c_base);
		i2c_clk = I2C_CLK_RATE_PMIC;
	}

	count = len / 8;
	if (len % 8 != 0)
		count += 1;

	if (count > 1)
		total_count = count;

	while (count)
	{
		/* FOR TPM */
		if (count < total_count && count > 0 && i2c->id == 6) {
			mtk_i2c_write(i2c, &tpm_data_fifo_v, 1);
		}

		if (count == 1) {
			temp_len = len % 8;
			if (temp_len == 0)
				temp_len = 8;
		} else {
			temp_len = 8;
		}
		I2C_SET_TRANS_LEN(i2c_base, temp_len);

		I2C_CLR_INTR_STATUS(i2c_base, I2C_TRANSAC_COMP | I2C_ACKERR
					| I2C_HS_NACKERR);
		/* setting speed */
		mtk_i2c_set_speed(i2c->id, i2c_clk, i2c->mode, i2c->speed);
		if (i2c->speed <= 100)
			write32(0x8001, (void *)(uintptr_t)(i2c_base + MTK_I2C_EXT_CONF));

		/* control registers */
		I2C_SET_SLAVE_ADDR(i2c_base, i2c->addr<<1);

		I2C_SET_TRANSAC_LEN(i2c_base, 1);
		I2C_SET_INTR_MASK(i2c_base, I2C_HS_NACKERR | I2C_ACKERR
					| I2C_TRANSAC_COMP);

		I2C_FIFO_CLR_ADDR(i2c_base);
		I2C_SET_TRANS_CTRL(i2c_base, ACK_ERR_DET_EN |
				   (i2c->is_clk_ext_disable ? 0 : CLK_EXT) |
				   (i2c->is_rs_enable ?
					REPEATED_START_FLAG : STOP_FLAG));

		/* start to write data */
		while (temp_len > 0) {
			I2C_WRITE_BYTE(i2c_base, *ptr);
			ptr++;
			temp_len--;
		}
		/* start trnasfer transaction */
		I2C_START_TRANSAC(i2c_base);

		/* polling mode : see if transaction complete */
		while (1) {
			status = I2C_INTR_STATUS(i2c_base);
			if (status & I2C_HS_NACKERR) {
				ret_code = I2C_WRITE_FAIL_HS_NACKERR;
				I2CERR("[i2c%d write] transaction NACK error\n",
				       i2c->id);
				mtk_i2c_dump_info(i2c);
				break;
			} else if (status & I2C_ACKERR) {
				ret_code = I2C_WRITE_FAIL_ACKERR;
				I2CERR("[i2c%d write] transaction ACK error\n",
				       i2c->id);
				mtk_i2c_dump_info(i2c);
				break;
			} else if (status & I2C_TRANSAC_COMP) {
				ret_code = I2C_OK;
				break;
			} else if (time_out_val > 100000) {
				ret_code = I2C_WRITE_FAIL_TIMEOUT;
				I2CERR("[i2c%d write] transaction timeout:%d\n",
				       i2c->id, time_out_val);
				mtk_i2c_dump_info(i2c);
				break;
			}
			time_out_val++;
		}

		I2C_CLR_INTR_STATUS(i2c_base, I2C_TRANSAC_COMP | I2C_ACKERR
					| I2C_HS_NACKERR);

		/* clear bit mask */
		I2C_CLR_INTR_MASK(i2c_base, I2C_HS_NACKERR | I2C_ACKERR
					| I2C_TRANSAC_COMP);
		I2C_SOFTRESET(i2c_base);

		count--;
	}
	return ret_code;
}

uint32_t mtk_i2c_write_read(struct mtk_i2c_t *i2c, uint8_t *buffer,
			    uint32_t write_len, uint32_t read_len)
{
	uint32_t ret_code = I2C_OK;
	uint32_t i2c_clk;
	uint8_t *ptr = buffer;

	uint16_t status;
	uint32_t time_out_val = 0;
	uint32_t i2c_base;

	i2c_base = I2C0_BASE + i2c->id * 0x1000;

	/* CHECKME. mt65xx doesn't support len = 0. */
	if ((write_len == 0) || (read_len == 0) ||
	    (write_len > 255) || (read_len > 255)) {
		I2CLOG
		    ("[i2c_write_read] I2C doesn't support w,r len = %d,%d.\n",
		     write_len, read_len);
		return I2C_WRITE_FAIL_ZERO_LENGTH;
	}

	if (i2c->dir == 0) {
		I2C_PATH_NORMAL(i2c_base);
		i2c_clk = I2C_CLK_RATE;
	} else {
		I2C_PATH_PMIC(i2c_base);
		i2c_clk = I2C_CLK_RATE_PMIC;
	}
	I2C_CLR_INTR_STATUS(i2c_base, I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);

	/* setting speed */
	mtk_i2c_set_speed(i2c->id, i2c_clk, i2c->mode, i2c->speed);
	if (i2c->speed <= 100) {
		write32(0x8001, (void *)(uintptr_t)(i2c_base + MTK_I2C_EXT_CONF));
	}

	/* control registers */
	I2C_SET_SLAVE_ADDR(i2c_base, i2c->addr << 1);
	I2C_SET_TRANS_LEN(i2c_base, write_len | read_len << 8);
	I2C_SET_TRANSAC_LEN(i2c_base, 2);
	I2C_SET_INTR_MASK(i2c_base, I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
	I2C_FIFO_CLR_ADDR(i2c_base);
	I2C_SET_TRANS_CTRL(i2c_base, DIR_CHG | ACK_ERR_DET_EN |
			   (i2c->is_clk_ext_disable ? 0 : CLK_EXT) |
			   (i2c->is_rs_enable ?
				REPEATED_START_FLAG : STOP_FLAG));

	/* start to write data */
	while (write_len--) {
		I2C_WRITE_BYTE(i2c_base, *ptr);
		ptr++;
	}

	/* start trnasfer transaction */
	I2C_START_TRANSAC(i2c_base);
	/* polling mode : see if transaction complete */
	while (1) {
		status = I2C_INTR_STATUS(i2c_base);
		if (status & I2C_HS_NACKERR) {
			ret_code = I2C_WRITE_FAIL_HS_NACKERR;
			I2CERR("[i2c%d write_read] transaction NACK error\n",
			       i2c->id);
			mtk_i2c_dump_info(i2c);
			break;
		} else if (status & I2C_ACKERR) {
			ret_code = I2C_WRITE_FAIL_ACKERR;
			I2CERR("[i2c%d write_read] transaction ACK error\n",
			       i2c->id);
			mtk_i2c_dump_info(i2c);
			break;
		} else if (status & I2C_TRANSAC_COMP) {
			ret_code = I2C_OK;
			break;
		} else if (time_out_val > 100000) {
			ret_code = I2C_WRITE_FAIL_TIMEOUT;
			I2CERR("[i2c%d write_read] transaction timeout:%d\n",
			       i2c->id, time_out_val);
			mtk_i2c_dump_info(i2c);
			break;
		}
		time_out_val++;
	}

	if (ret_code == I2C_OK) {
		ptr = buffer;
		while (read_len--) {
			I2C_READ_BYTE(i2c_base, ptr);
			ptr++;
		}
	}
	I2C_CLR_INTR_STATUS(i2c_base, I2C_TRANSAC_COMP | I2C_ACKERR
				| I2C_HS_NACKERR);

	/* clear bit mask */
	I2C_CLR_INTR_MASK(i2c_base, I2C_HS_NACKERR | I2C_ACKERR
				| I2C_TRANSAC_COMP);
	I2C_SOFTRESET(i2c_base);

	return ret_code;
}

int platform_i2c_transfer(unsigned bus, struct i2c_seg *segments, int count)
{

	int ret = 0;
	int i;

	g_i2c[bus].id = bus;
	struct mtk_i2c_t *i2c = &g_i2c[bus];
	uint32_t read_len = i2c->aux_length;

	for (i = 0; i < count; i++) {
		if (segments[i].read == 2) {
			ret = mtk_i2c_write_read(i2c, segments[i].buf,
						 segments[i].len, read_len);
		} else if (segments[i].read == 1) {
			ret = mtk_i2c_read(i2c, segments[i].buf,
					   segments[i].len);
		} else if (segments[i].read == 0) {
			ret = mtk_i2c_write(i2c, segments[i].buf,
					    segments[i].len);
		}
		if (ret)
			break;
	}
	return ret;
}

void mtk_i2c_init(uint8_t id, uint8_t dir, uint8_t addr,
		  uint8_t aux_length)
{
	g_i2c[id].dir = dir;
	g_i2c[id].addr = addr;
	g_i2c[id].mode = ST_MODE;
	g_i2c[id].speed = 100;
	g_i2c[id].aux_length = aux_length;	/* for write then read */
}
