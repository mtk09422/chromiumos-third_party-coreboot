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
#include <arch/io.h>
#include <boardid.h>
#include <boot/coreboot_tables.h>
#include <device/device.h>
#include <edid.h>
#include <gpio.h>
#include <soc/display.h>
#include <soc/ddp.h>
#include <soc/usb.h>

static void configure_hdmi(void)
{
	u32 reg;

	reg = read32((void *)DISPSYS_CONFIG_BASE + DISP_REG_CONFIG_HDMI_EN);
	reg = reg & 0xffff7fff;
	write32((void *)DISPSYS_CONFIG_BASE + DISP_REG_CONFIG_HDMI_EN, reg);
}

#define USB_PORT_SWITCH_GPIO 3

static void usb_port_switch(void)
{
	switch (board_id()) {
	case 0:
		/* Rev0: switch download port to type A */
		gpio_output(USB_PORT_SWITCH_GPIO, 1);
		break;
	default:
		break;
	}

}

static void mainboard_init(device_t dev)
{
	setup_usb_host();
	usb_port_switch();
	configure_hdmi();
}

static void mainboard_enable(device_t dev)
{
	dev->ops->init = &mainboard_init;
}

struct chip_operations mainboard_ops = {
	.name = "oak",
	.enable_dev = mainboard_enable,
};

void mainboard_power_on_backlight(void)
{

}

