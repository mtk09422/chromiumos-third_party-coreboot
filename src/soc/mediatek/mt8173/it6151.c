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

#include <delay.h>
#include <soc/i2c.h>
#include <gpio.h>
#include <soc/pmic_wrap_init.h>
#include <soc/pmic.h>
#include <soc/it6151.h>
#include <mainboard/cust_i2c.h>
#include <mainboard/cust_gpio_usage.h>
#include <console/console.h>
#include <edid.h>

#ifdef DEBUGDPF
#define MTKFBTAG                "[MTKFB] "
#define MTKFBERRTAG             "[MTKFB ERR] "
#define MTKFBDPF(fmt, arg...)   printk(BIOS_INFO, MTKFBTAG fmt, ##arg)
#define MTKFBERR(fmt, arg...)   printk(BIOS_INFO, MTKFBERRTAG fmt, ##arg)
#else
#define MTKFBERRTAG             "[MTKFB ERR] "
#define MTKFBDPF(fmt, arg...)
#define MTKFBERR(fmt, arg...)   printk(BIOS_INFO, MTKFBERRTAG fmt, ##arg)
#endif

struct it6151_bridge {
	int gpio_pd_n;
	int gpio_rst_n;
	int enabled;
};

static struct it6151_bridge _ite_bridge;

enum {
	DP_I2C_ADDR   = (0x5C),
	MIPI_I2C_ADDR = (0x6C)
};

enum {
	MIPI_4_LANE = (3),
	MIPI_3_LANE = (2),
	MIPI_2_LANE = (1),
	MIPI_1_LANE = (0)
};

enum {
	RGB_24b    = (0x3E),
	RGB_30b    = (0x0D),
	RGB_36b    = (0x1D),
	RGB_18b_P  = (0x1E),
	RGB_18b_L  = (0x2E),
	YCbCr_16b  = (0x2C),
	YCbCr_20b  = (0x0C),
	YCbCr_24b  = (0x1C)
};

enum {
	B_DPTXIN_6Bpp  = (0),
	B_DPTXIN_8Bpp  = (1),
	B_DPTXIN_10Bpp = (2),
	B_DPTXIN_12Bpp = (3)
};

enum {
	B_LBR = (1),
	B_HBR = (0)
};

enum {
	B_4_LANE = (3),
	B_2_LANE = (1),
	B_1_LANE = (0)
};

enum {
	B_SSC_ENABLE = (1),
	B_SSC_DISABLE =	(0)
};

struct output_setting {
	u32 training_bitrate;
	u32 dptx_ssc_setting;
	u8 high_pclk;
	u8 mp_mclk_inv;
	u8 mp_continuous_clk;
	u8 mp_lane_deskew;
	u8 mp_pclk_div;
	u8 mp_lane_swap;
	u8 mp_pn_swap;
	u8 dp_pn_swap;
	u8 dp_aux_pn_swap;
	u8 dp_lane_swap;
};

static struct output_setting setting = {
	.training_bitrate = (B_HBR),
	.dptx_ssc_setting = (B_SSC_ENABLE),

	.high_pclk = (1),
	.mp_mclk_inv = (1),
	.mp_continuous_clk = (1),
	.mp_lane_deskew = (1),
	.mp_pclk_div = (2),
	.mp_lane_swap = (0),
	.mp_pn_swap = (0),
	.dp_pn_swap = (0),
	.dp_aux_pn_swap = (0),
	.dp_lane_swap = (0),
};

enum {
	INT_MASK	= (3),
	MIPI_INT_MASK	= (0),
	TIMER_CNT	= (0x0A)
};


struct panel_timing_setting {
	u32 width;

	u8 vic;
	u8 mp_hpol;
	u8 mp_vpol;
	u8 mipi_lane_count;
	u8 dptx_lane_count;
	u8 en_ufo;
	u8 mipi_packed_fmt;
	u8 mp_h_resync;
	u8 mp_v_resync;
	u8 en_vlc;
	u8 vlc_cfg_h;
	u8 vlc_cfg_l;
};

static struct panel_timing_setting panel_timing = {
	/* default value */
	.width = 0,
	.vic = 0,
	.mp_hpol = 0,
	.mp_vpol = 1,
	.mipi_lane_count = MIPI_4_LANE,
	.dptx_lane_count = B_1_LANE,
	.en_ufo = 0,
	.mipi_packed_fmt = RGB_24b,
	.mp_h_resync = 1,
	.mp_v_resync = 0,
	.en_vlc	= 0,
	.vlc_cfg_h = 0,  /*bit7~4*/
	.vlc_cfg_l = 0,  /*bit3~0*/
};


static u8 it6151_reg_i2c_read_byte(struct it6151_bridge *ite_bridge,
	u8 dev_address, u8 reg_off, u8 *data)
{
	int ret = 1;

	if (dev_address == MIPI_I2C_ADDR) {
		ret = i2c_readb(I2C_DISPLAY_CHANNEL, MIPI_I2C_ADDR, reg_off,
				data);
	} else if (dev_address == DP_I2C_ADDR) {
		ret = i2c_readb(I2C_DISPLAY_CHANNEL, DP_I2C_ADDR, reg_off,
				data);
	}

	if (ret) {
		MTKFBDPF("it6151_reg_i2c_read_byte fail : 0x%x\n", ret);
		return ret;
	}
	return 0;
}

static void it6151_reg_i2c_write_byte(struct it6151_bridge *ite_bridge,
	u8 dev_address, u8 reg_address, u8 reg_val)
{
	if (dev_address == MIPI_I2C_ADDR)
		i2c_writeb(I2C_DISPLAY_CHANNEL, MIPI_I2C_ADDR, reg_address,
			   reg_val);
	else if (dev_address == DP_I2C_ADDR)
		i2c_writeb(I2C_DISPLAY_CHANNEL, DP_I2C_ADDR, reg_address,
			   reg_val);
}

static void it6151_dptx_init(void)
{
	struct it6151_bridge *ite_bridge = &_ite_bridge;
	u8 reg_value;

	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x05, 0x29);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x05, 0x00);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x09, INT_MASK);
	/* Enable HPD_IRQ,HPD_CHG,VIDSTABLE*/
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x0A, 0x00);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x0B, 0x00);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0xC5, 0xC1);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0xB5, 0x00);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0xB7, 0x80);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0xC4, 0xF0);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x06, 0xFF);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x07, 0xFF);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x08, 0xFF);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x05, 0x00);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x0c, 0x08);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x21, 0x05);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x3a, 0x04);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x5f, 0x06);
	/*    {DP_I2C_ADDR,0xb5,0xFF,0x80},*/
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0xc9, 0xf5);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0xca, 0x4c);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0xcb, 0x37);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0xce, 0x80);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0xd3, 0x03);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0xd4, 0x60);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0xe8, 0x11);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0xec,
				  panel_timing.vic);
	mdelay(5);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x23, 0x42);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x24, 0x07);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x25, 0x01);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x26, 0x00);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x27, 0x10);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x2B, 0x05);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x23, 0x40);

	reg_value = (setting.dp_aux_pn_swap << 3) |
		    (setting.dp_pn_swap << 2) | 0x03;
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x22, reg_value);

	reg_value = (setting.dptx_ssc_setting << 4) |
		    (setting.dp_lane_swap << 3) |
		    (panel_timing.dptx_lane_count << 1) |
		    (setting.training_bitrate);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x16, reg_value);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x0f, 0x01);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x76, 0xa7);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x77, 0xaf);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x7e, 0x8f);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x7f, 0x07);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x80, 0xef);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x81, 0x5f);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x82, 0xef);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x83, 0x07);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x88, 0x38);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x89, 0x1f);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x8a, 0x48);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x0f, 0x00);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x5c, 0xf3);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x17, 0x04);
	it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x17, 0x01);
	mdelay(5);
}

static int it6151_test_bridge(void)
{
	unsigned char vendor;
	struct it6151_bridge *ite_bridge = &_ite_bridge;

	it6151_reg_i2c_read_byte(ite_bridge, DP_I2C_ADDR, 0, &vendor);
	if (vendor == 0x54)
		return 0;
	return 1;
}

static int it6151_bdg_enable(void)
{
	unsigned char vendor[2], device[2], rev;
	unsigned char reg_off;
	u32 reg_value;
	struct it6151_bridge *ite_bridge = &_ite_bridge;


	reg_off = 0x00;
	it6151_reg_i2c_read_byte(ite_bridge, DP_I2C_ADDR, reg_off, &vendor[0]);
	reg_off = 0x01;
	it6151_reg_i2c_read_byte(ite_bridge, DP_I2C_ADDR, reg_off, &vendor[1]);
	reg_off = 0x02;
	it6151_reg_i2c_read_byte(ite_bridge, DP_I2C_ADDR, reg_off, &device[0]);
	reg_off = 0x03;
	it6151_reg_i2c_read_byte(ite_bridge, DP_I2C_ADDR, reg_off, &device[1]);
	reg_off = 0x04;
	it6151_reg_i2c_read_byte(ite_bridge, DP_I2C_ADDR, reg_off, &rev);


	MTKFBDPF("it6151_bdg_enable : read it6151 dp info\n");
	MTKFBDPF("data [%02x %02x %02x %02x %02x]\n",
	       vendor[0], vendor[1], device[0], device[1], rev);

	if (vendor[0] == 0x54 && vendor[1] == 0x49 &&
	    device[0] == 0x51 && device[1] == 0x61) {
		it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0x05,
					  0x04);/* DP SW Reset*/
		it6151_reg_i2c_write_byte(ite_bridge, DP_I2C_ADDR, 0xfd,
					  (MIPI_I2C_ADDR << 1) | 1);
		it6151_reg_i2c_write_byte(ite_bridge, MIPI_I2C_ADDR, 0x05,
					  0x00);
		reg_value = (setting.mp_lane_swap << 7) |
			    (setting.mp_pn_swap << 6) |
			    (panel_timing.mipi_lane_count << 4) |
			    (panel_timing.en_ufo);
		it6151_reg_i2c_write_byte(ite_bridge, MIPI_I2C_ADDR, 0x0c,
					  reg_value);
		it6151_reg_i2c_write_byte(ite_bridge, MIPI_I2C_ADDR,
					  0x11, setting.mp_mclk_inv);

		if (rev == 0xA1) {
			it6151_reg_i2c_write_byte(ite_bridge, MIPI_I2C_ADDR,
						  0x19,
						  setting.mp_lane_deskew);
		} else {
			reg_value = (setting.mp_continuous_clk << 1) |
				    setting.mp_lane_deskew;
			it6151_reg_i2c_write_byte(ite_bridge, MIPI_I2C_ADDR,
						  0x19, reg_value);
		}
		/*it6151_reg_i2c_write_byte(MIPI_I2C_ADDR,0x19,0x01);*/
		it6151_reg_i2c_write_byte(ite_bridge, MIPI_I2C_ADDR, 0x27,
					  panel_timing.mipi_packed_fmt);
		reg_value = ((panel_timing.width / 4 - 1) >> 2) & 0xC0;
		it6151_reg_i2c_write_byte(ite_bridge, MIPI_I2C_ADDR, 0x28,
					  reg_value);
		reg_value = (panel_timing.width / 4 - 1) & 0xFF;
		it6151_reg_i2c_write_byte(ite_bridge, MIPI_I2C_ADDR, 0x29,
					  reg_value);
		it6151_reg_i2c_write_byte(ite_bridge, MIPI_I2C_ADDR,
					  0x2e, 0x34);
		it6151_reg_i2c_write_byte(ite_bridge, MIPI_I2C_ADDR,
					  0x2f, 0x01);
		reg_value = (panel_timing.mp_v_resync << 3) |
			    (panel_timing.mp_h_resync << 2) |
			    (panel_timing.mp_vpol << 1) |
			    (panel_timing.mp_hpol);
		it6151_reg_i2c_write_byte(ite_bridge, MIPI_I2C_ADDR, 0x4e,
					  reg_value);
		reg_value = (panel_timing.en_ufo << 5) | setting.mp_pclk_div;
		it6151_reg_i2c_write_byte(ite_bridge, MIPI_I2C_ADDR, 0x80,
					  reg_value);
		it6151_reg_i2c_write_byte(ite_bridge, MIPI_I2C_ADDR, 0x84,
					  0x8f);
		it6151_reg_i2c_write_byte(ite_bridge, MIPI_I2C_ADDR, 0x09,
					  MIPI_INT_MASK);
		it6151_reg_i2c_write_byte(ite_bridge, MIPI_I2C_ADDR, 0x92,
					  TIMER_CNT);

		it6151_dptx_init();
		return 0;
	}

	return -1;
}

void it6151_pre_enable(void)
{
	int ret;
	int cnt = 0;

	while (1) {
		ret = it6151_test_bridge();
		if (ret == 0 || cnt > 10000)
			break;
		cnt++;
	}

	ret = it6151_bdg_enable();
	if (ret == -1)
		MTKFBERR("mtk_it6151_pre_enable fail\n");
}

static void it6151_set_gpio_direction_output(u32 pin, u32 val)
{
	gpio_output(pin, val);
}


void it6151_enable(void)
{
	struct it6151_bridge *ite_bridge = &_ite_bridge;

	if (ite_bridge->enabled)
		return;

	ite_bridge->enabled = 1;

	MTKFBDPF("it6151_enable begin (set VCAMIO)\n");
	mt6391_write(PMIC_RG_DIGLDO_CON6, 0x1, PMIC_RG_VCAMIO_SW_EN_MASK,
		     PMIC_RG_VCAMIO_SW_EN_SHIFT);

	/* Configureation for Pin 93  IT6151 1.2V power enable */
	it6151_set_gpio_direction_output(ite_bridge->gpio_pd_n, 1);

	/* Configureation for Pin 94  IT6151 reset */
	it6151_set_gpio_direction_output(ite_bridge->gpio_rst_n, 0);
	udelay(15);
	it6151_set_gpio_direction_output(ite_bridge->gpio_rst_n, 1);

}


void it6151_disable(void)
{
	struct it6151_bridge *ite_bridge = &_ite_bridge;

	if (!ite_bridge->enabled)
		return;

	ite_bridge->enabled = 0;

	it6151_set_gpio_direction_output(ite_bridge->gpio_rst_n, 0);
	it6151_set_gpio_direction_output(ite_bridge->gpio_pd_n, 0);
}

int it6151_init(u32 hactive)
{
	_ite_bridge.gpio_pd_n  = 93;
	_ite_bridge.gpio_rst_n = 94;

	if (hactive == 0)
		return -1;

	panel_timing.width = hactive;

	it6151_enable();
	return 0;
}
