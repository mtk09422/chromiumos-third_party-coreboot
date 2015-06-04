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
#include <gpio.h>
#include <console/console.h>
#include <mainboard/cust_i2c.h>
#include <soc/mt8173.h>
#include <soc/i2c.h>
#include <soc/pmic_wrap_init.h>
#include <soc/pmic.h>
#include <soc/ps8640.h>
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

static struct ps8640_bridge ps_dp_bridge = {
	.gpio_pd_n  = 116,
	.gpio_rst_n = 115,
	.gpio_1v2_n = 30,
	.gpio_mode_sel_n = 92,
	.gpio_panel_3v3_n = 41,
	.gpio_panel_bl_n = 95,
};

static void ps8640_reg_i2c_write_byte(struct ps8640_bridge *ps_bridge,
	u8 dev_address, u8 reg_address, u8 reg_val)
{
	i2c_writeb(I2C_DISPLAY_CHANNEL, dev_address, reg_address, reg_val);
}

void ps8640_enable(void)
{
	struct ps8640_bridge *ps_bridge = &ps_dp_bridge;

	if (ps_bridge->enabled)
		return;

	ps_bridge->enabled = 1;

	gpio_output(ps_bridge->gpio_panel_3v3_n, 1);
	gpio_output(ps_bridge->gpio_panel_bl_n, 1);

	MTKFBDPF("ps8640_enable begin (set VCAMIO)\n");
	mt6391_write(PMIC_RG_DIGLDO_CON6, 0x1, PMIC_RG_VCAMIO_SW_EN_MASK,
			      PMIC_RG_VCAMIO_SW_EN_SHIFT);
	mt6391_write(PMIC_RG_DIGLDO_CON20, 0x7, 0x7, 0x5);

	gpio_output(ps_bridge->gpio_mode_sel_n, 1);
	gpio_output(ps_bridge->gpio_pd_n, 1);
	gpio_output(ps_bridge->gpio_1v2_n, 1);

	/* Configureation for Pin 94  ps8640 reset */
	gpio_output(ps_bridge->gpio_rst_n, 0);
	mdelay(1);
	gpio_output(ps_bridge->gpio_rst_n, 1);
	mdelay(1);
	ps8640_reg_i2c_write_byte(ps_bridge, 0x1b, 0xfe, 0x13);
	ps8640_reg_i2c_write_byte(ps_bridge, 0x1b, 0xff, 0x18);
	ps8640_reg_i2c_write_byte(ps_bridge, 0x1b, 0xfe, 0x13);
	ps8640_reg_i2c_write_byte(ps_bridge, 0x1b, 0xff, 0x1c);
}

void ps8640_disable(void)
{
	struct ps8640_bridge *ps_bridge = &ps_dp_bridge;

	if (!ps_bridge->enabled)
		return;

	ps_bridge->enabled = 0;

	gpio_output(ps_bridge->gpio_rst_n, 0);
	gpio_output(ps_bridge->gpio_pd_n, 0);
}

int ps8640_init(void)
{
	ps8640_enable();
	return 0;
}

