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

/*
 * MTK I2C FIFO SIZE is 8 bytes, so if i2c transfer length > 8,
 * we will separate the transfer length, and transfer 8 bytes one time.
 * Define TPM I2C number for I2C workaround.
 * TODO: DMA is supported, we will remove this part.
 */
#define TPM_I2C_BUS_NUM 2

static struct mtk_i2c_t g_i2c[7];

uint32_t mtk_i2c_set_speed(struct mtk_i2c_t *i2c, uint32_t clock)
{
	uint32_t i2c_base = i2c->base;
	I2C_SPD_MODE mode = i2c->mode;
	uint32_t khz = i2c->speed;

	uint16_t sample_cnt_div;
	uint16_t step_cnt_div;
	uint16_t max_step_cnt_div =
	    	(mode == HS_MODE) ? MAX_HS_STEP_CNT_DIV : MAX_STEP_CNT_DIV;
	uint32_t reg_value;
	uint32_t sclk;

	uint32_t diff;
	uint32_t min_diff = I2C_CLK_RATE;
	uint16_t sample_div = MAX_SAMPLE_CNT_DIV;
	uint16_t step_div = max_step_cnt_div;

	for (sample_cnt_div = 1; sample_cnt_div <= MAX_SAMPLE_CNT_DIV;
	     sample_cnt_div++) {
		for (step_cnt_div = 1; step_cnt_div <= max_step_cnt_div;
		     step_cnt_div++) {
			sclk = (clock >> 1) / (sample_cnt_div * step_cnt_div);

			if (sclk > khz) continue;

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

	if (sclk > khz)
		return I2C_SET_SPEED_FAIL_OVER_SPEED;

	step_cnt_div--;
	sample_cnt_div--;

	if (mode == HS_MODE) {
		reg_value = read32((void *)(uintptr_t)(i2c_base + MTK_I2C_HS)) &
			           ((0x7 << 12) | (0x7 << 8));
		reg_value = (sample_cnt_div & 0x7) << 12 | (step_cnt_div & 0x7) << 8
			    | reg_value;
		write32((void *)(uintptr_t)(i2c_base + MTK_I2C_HS), reg_value);
		I2C_SET_HS_MODE(i2c_base, 1);
	} else {
		reg_value = read32((void *)(uintptr_t)(i2c_base + MTK_I2C_TIMING)) &
			           ~((0x7 << 8) | (0x3f << 0));
		reg_value = (sample_cnt_div & 0x7) << 8 | (step_cnt_div & 0x3f) << 0
			    | reg_value;
		write32((void *)(uintptr_t)(i2c_base + MTK_I2C_TIMING), reg_value);
		I2C_SET_HS_MODE(i2c_base, 0);
	}

	I2CLOG("[i2c%d set speed] Set sclk to %d khz (orig: %d khz)\n",
	       channel, sclk, khz);
	I2CLOG("[i2c_set_speed] I2C Timing parameter sample_cnt_div(%d)\n",
	       sample_cnt_div);
	I2CLOG("step_cnt_div(%d)\n", step_cnt_div);

	return I2C_OK;
}

static inline void mtk_i2c_dump_info(struct mtk_i2c_t *i2c)
{
	uint32_t i2c_base;
	i2c_base = i2c->base;

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

	I2CERR("base address %x\n", (uint32_t)i2c_base);
	return;
}

uint32_t mtk_i2c_read(struct mtk_i2c_t *i2c, uint8_t *buffer, uint32_t len)
{
	uint32_t ret_code = I2C_OK;
	uint32_t i2c_clk;
	uint8_t *ptr = buffer;
	uint16_t status;
	uint32_t time_out_val = 0;
	uint32_t count = 0;
	uint32_t temp_len = 0;
	uint32_t total_count = 0;
	uint8_t tpm_data_fifo_v = 0x5;
        uint32_t i2c_base = i2c->base;

	/* mt65xx doesn't support len = 0. */
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

	while (count){
		/* I2C FIFO SIZE is 8 bytes, so if i2c transfer length > 8,
		 * we will separate the transfer length, and after transfer 8 bytes one time.
		 * we should write 0x5 to the tpm according to the tpm spec.
		 * TODO: DMA is supported, we will remove this part.
		 */
		if (count < total_count && count > 0 && i2c->id == TPM_I2C_BUS_NUM) {
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
		mtk_i2c_set_speed(i2c, i2c_clk);

		/* MTK_I2C_EXT_CONF register is only valid for speed <= 100. */
		if(i2c->speed <= 100) {
			/* 0x8001 is for setting the i2c start and stop condition extention time. */
			write32((void *)(uintptr_t)(i2c_base + MTK_I2C_EXT_CONF), 0x8001);
		}
		/* control registers */
		I2C_SET_SLAVE_ADDR(i2c_base, ((i2c->addr << 1) | 0x1));

		/* I2C_SET_TRANS_LEN(len); */
		I2C_SET_TRANSAC_LEN(i2c_base, 1);
		I2C_SET_INTR_MASK(i2c_base, I2C_HS_NACKERR | I2C_ACKERR
				  | I2C_TRANSAC_COMP);
		I2C_FIFO_CLR_ADDR(i2c_base);
		I2C_SET_TRANS_CTRL(i2c_base, ACK_ERR_DET_EN
				   | (i2c->is_clk_ext_disable ?  0 : CLK_EXT)
				   | (i2c->is_rs_enable ?
				      REPEATED_START_FLAG : STOP_FLAG));

		/* start trnasfer transaction */
		I2C_START_TRANSAC(i2c_base);

		/* polling mode : see if transaction complete */
		while (1) {
			status = I2C_INTR_STATUS(i2c_base);

			if (status & I2C_TRANSAC_COMP
			    && (!I2C_FIFO_IS_EMPTY(i2c_base))) {
				ret_code = I2C_OK;
				break;
			} else if (status & I2C_HS_NACKERR) {
				ret_code = I2C_READ_FAIL_HS_NACKERR;
				I2CERR
				    ("[i2c%d read] transaction NACK error (%x)\n",
				     i2c->id, status);
				mtk_i2c_dump_info(i2c);
				break;
			} else if (status & I2C_ACKERR) {
				ret_code = I2C_READ_FAIL_ACKERR;
				I2CERR
				    ("[i2c%d read] transaction ACK error (%x)\n",
				     i2c->id, status);
				mtk_i2c_dump_info(i2c);
				break;
			} else if (time_out_val > 100000) {
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

		if (ret_code == I2C_OK) {
			while (temp_len-- > 0) {
				I2C_READ_BYTE(i2c_base, ptr++);
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
        uint32_t i2c_base = i2c->base;

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
		/* I2C FIFO SIZE is 8 bytes, so if i2c transfer length > 8,
		 * we will separate the transfer length, and after transfer 8 bytes one time.
		 * we should write 0x5 to the tpm according to the tpm spec.
		 * TODO: DMA is supported, we will remove this part.
		 */
		if (count < total_count && count > 0 && i2c->id == TPM_I2C_BUS_NUM){
			mtk_i2c_write(i2c, &tpm_data_fifo_v, 1);
		}

		if (count == 1){
			temp_len = len % 8;
			if (temp_len == 0)
				temp_len = 8;
		}
		else {
			temp_len = 8;
		}
		I2C_SET_TRANS_LEN(i2c_base, temp_len);

		I2C_CLR_INTR_STATUS(i2c_base, I2C_TRANSAC_COMP | I2C_ACKERR
				    | I2C_HS_NACKERR);
		/* setting speed */
		mtk_i2c_set_speed(i2c, i2c_clk);

		/* MTK_I2C_EXT_CONF register is only valid for speed <= 100. */
		if (i2c->speed <= 100)
			/* 0x8001 is for setting the i2c start and stop condition extention time. */
			write32((void *)(uintptr_t)(i2c_base + MTK_I2C_EXT_CONF), 0x8001);

		/* control registers */
		I2C_SET_SLAVE_ADDR(i2c_base, i2c->addr << 1);

		I2C_SET_TRANSAC_LEN(i2c_base, 1);
		I2C_SET_INTR_MASK(i2c_base, I2C_HS_NACKERR | I2C_ACKERR
				  | I2C_TRANSAC_COMP);

		I2C_FIFO_CLR_ADDR(i2c_base);
		I2C_SET_TRANS_CTRL(i2c_base, ACK_ERR_DET_EN |
				   (i2c->is_clk_ext_disable ? 0: CLK_EXT)
				   | (i2c->is_rs_enable ?
				      REPEATED_START_FLAG : STOP_FLAG));

		/* start to write data */
		while (temp_len-- > 0) {
			I2C_WRITE_BYTE(i2c_base, *(ptr++));
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
			}
			else if ( status & I2C_TRANSAC_COMP) {
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
        uint32_t i2c_base = i2c->base;

	/* CHECKME. mt65xx doesn't support len = 0. */
	if ((write_len == 0) || (read_len == 0)
	    || (write_len > 255) || (read_len > 255)) {
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
	mtk_i2c_set_speed(i2c, i2c_clk);

	/* MTK_I2C_EXT_CONF register is only valid for speed <= 100. */
	if (i2c->speed <= 100) {
		/* 0x8001 is for setting the i2c start and stop condition extention time. */
		write32((void *)(uintptr_t)(i2c_base + MTK_I2C_EXT_CONF), 0x8001);
	}

	/* control registers */
	I2C_SET_SLAVE_ADDR(i2c_base, i2c->addr << 1);
	I2C_SET_TRANS_LEN(i2c_base, write_len);
	I2C_SET_TRANS_AUX_LEN(i2c_base, read_len);
	I2C_SET_TRANSAC_LEN(i2c_base, 2);
	I2C_SET_INTR_MASK(i2c_base, I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
	I2C_FIFO_CLR_ADDR(i2c_base);
	I2C_SET_TRANS_CTRL(i2c_base, DIR_CHG | ACK_ERR_DET_EN
			   | (i2c->is_clk_ext_disable ? 0 : CLK_EXT)
			   | (i2c->is_rs_enable ? REPEATED_START_FLAG : STOP_FLAG));

	/* start to write data */
	while (write_len--) {
		I2C_WRITE_BYTE(i2c_base, *(ptr++));
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
			I2C_READ_BYTE(i2c_base, ptr++);
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

void mtk_i2c_init(uint32_t reg_addr, uint8_t id, uint8_t dir, uint8_t addr,
		  uint8_t aux_length)
{
	g_i2c[id].base = reg_addr;
	g_i2c[id].dir = dir;
	g_i2c[id].addr = addr;
	g_i2c[id].mode = ST_MODE;
	g_i2c[id].speed = 100;
	g_i2c[id].aux_length = aux_length;	/* for write then read */
}
