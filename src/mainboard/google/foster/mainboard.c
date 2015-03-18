/*
 * This file is part of the coreboot project.
 *
 * Copyright 2015 Google Inc.
 * Copyright (c) 2015, NVIDIA CORPORATION.  All rights reserved.
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

#include <arch/mmu.h>
#include <boot/coreboot_tables.h>
#include <device/device.h>
#include <memrange.h>
#include <soc/addressmap.h>
#include <soc/clk_rst.h>
#include <soc/clock.h>
#include <soc/funitcfg.h>
#include <soc/nvidia/tegra/i2c.h>
#include <soc/nvidia/tegra/usb.h>
#include <soc/padconfig.h>
#include <soc/spi.h>
#include <soc/nvidia/tegra/dc.h>
#include <soc/display.h>

#include <vboot_struct.h>
#include <vendorcode/google/chromeos/chromeos.h>
#include <delay.h>

static const struct pad_config sdmmc1_pad[] = {
	/* MMC1(SDCARD) */
	PAD_CFG_SFIO(SDMMC1_CLK, PINMUX_INPUT_ENABLE, SDMMC1),
	PAD_CFG_SFIO(SDMMC1_CMD, PINMUX_INPUT_ENABLE | PINMUX_PULL_UP, SDMMC1),
	PAD_CFG_SFIO(SDMMC1_DAT0, PINMUX_INPUT_ENABLE | PINMUX_PULL_UP, SDMMC1),
	PAD_CFG_SFIO(SDMMC1_DAT1, PINMUX_INPUT_ENABLE | PINMUX_PULL_UP, SDMMC1),
	PAD_CFG_SFIO(SDMMC1_DAT2, PINMUX_INPUT_ENABLE | PINMUX_PULL_UP, SDMMC1),
	PAD_CFG_SFIO(SDMMC1_DAT3, PINMUX_INPUT_ENABLE | PINMUX_PULL_UP, SDMMC1),
	/* MMC1 Card Detect pin */
	PAD_CFG_GPIO_INPUT(GPIO_PZ1, PINMUX_PULL_UP),
	/* Disable SD card reader power so it can be reset even on warm boot.
	   Payloads must enable power before accessing SD card slots. */
	PAD_CFG_GPIO_OUT0(GPIO_PZ4, PINMUX_PULL_NONE),
};

static const struct pad_config sdmmc4_pad[] = {
	/* No SDMMC4 pins in T210 pinmux ?? */
};

static const struct pad_config padcfgs[] = {
	/* We pull the USB VBUS signals up but keep them as inputs since the
	 * voltage source likes to drive them low on overcurrent conditions */
	PAD_CFG_GPIO_INPUT(USB_VBUS_EN0, PINMUX_PULL_UP),
	PAD_CFG_GPIO_INPUT(USB_VBUS_EN1, PINMUX_PULL_UP),

	/* Add backlight vdd/enable/pwm/dp hpd pad cfgs here */
};

static const struct pad_config i2c1_pad[] = {
	/* GEN1 I2C */
	PAD_CFG_SFIO(GEN1_I2C_SCL, PINMUX_INPUT_ENABLE, I2C1),
	PAD_CFG_SFIO(GEN1_I2C_SDA, PINMUX_INPUT_ENABLE, I2C1),
};

static const struct funit_cfg funitcfgs[] = {
	FUNIT_CFG(SDMMC1, PLLP, 48000, sdmmc1_pad, ARRAY_SIZE(sdmmc1_pad)),
	FUNIT_CFG(SDMMC4, PLLP, 48000, sdmmc4_pad, ARRAY_SIZE(sdmmc4_pad)),
	FUNIT_CFG(I2C1, PLLP, 100, i2c1_pad, ARRAY_SIZE(i2c1_pad)),
};

static void setup_usb(void)
{
	clock_enable_clear_reset(CLK_L_USBD, CLK_H_USB3, 0, 0, 0, 0, 0);

	usb_setup_utmip((void *)TEGRA_USBD_BASE);
	usb_setup_utmip((void *)TEGRA_USB3_BASE);
}

static void mainboard_init(device_t dev)
{
	soc_configure_pads(padcfgs, ARRAY_SIZE(padcfgs));
	soc_configure_funits(funitcfgs, ARRAY_SIZE(funitcfgs));

	setup_usb();
	i2c_init(I2C1_BUS);
}

static void mainboard_enable(device_t dev)
{
	dev->ops->init = &mainboard_init;
}

struct chip_operations mainboard_ops = {
	.name   = "foster",
	.enable_dev = mainboard_enable,
};
