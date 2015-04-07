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
#include <soc/addressmap.h>
#include <soc/clk_rst.h>
#include <soc/clock.h>
#include <soc/funitcfg.h>
#include <soc/nvidia/tegra/i2c.h>
#include <soc/padconfig.h>
#include "gpio.h"

static const struct pad_config padcfgs[] = {
	PAD_CFG_GPIO_INPUT(USB_VBUS_EN1, PINMUX_PULL_NONE | PINMUX_PARKED |
			   PINMUX_INPUT_ENABLE | PINMUX_LPDR | PINMUX_IO_HV),
};

static const struct pad_config audio_codec_pads[] = {
	/* GPIO_X1_AUD(BB3) is CODEC_RST_L and DMIC1_DAT(E1) is AUDIO_ENABLE */
	PAD_CFG_GPIO_OUT1(GPIO_X1_AUD, PINMUX_PULL_DOWN),
	PAD_CFG_GPIO_OUT1(DMIC1_DAT, PINMUX_PULL_DOWN),
};

static const struct pad_config i2s1_pad[] = {
	/* I2S1 */
	PAD_CFG_SFIO(DAP1_SCLK, PINMUX_INPUT_ENABLE, I2S1),
	PAD_CFG_SFIO(DAP1_FS, PINMUX_INPUT_ENABLE, I2S1),
	PAD_CFG_SFIO(DAP1_DOUT, PINMUX_INPUT_ENABLE, I2S1),
	PAD_CFG_SFIO(DAP1_DIN, PINMUX_INPUT_ENABLE | PINMUX_TRISTATE, I2S1),
	/* codec MCLK via AUD SFIO */
	PAD_CFG_SFIO(AUD_MCLK, PINMUX_PULL_NONE, AUD),
};

static const struct funit_cfg audio_funit[] = {
	/* We need 1.5MHz for I2S1. So we use CLK_M */
	FUNIT_CFG(I2S1, CLK_M, 1500, i2s1_pad, ARRAY_SIZE(i2s1_pad)),
};

static const struct funit_cfg funits[] = {
	FUNIT_CFG_USB(USBD),
	FUNIT_CFG(SDMMC4, PLLP, 48000, NULL, 0),
	/* I2C6 for audio, temp sensor, etc. Enable codec via GPIOs/muxes */
	FUNIT_CFG(I2C6, PLLP, 400, audio_codec_pads, ARRAY_SIZE(audio_codec_pads)),
};

/* Audio init: clocks and enables/resets */
static void setup_audio(void)
{
	/* Audio codec (RT5677) uses OSC freq (via AUD_MCLK), s/b 38.4MHz */
	soc_configure_funits(audio_funit, ARRAY_SIZE(audio_funit));

	/*
	 * As per NVIDIA hardware team, we need to take ALL audio devices
	 * connected to AHUB (AUDIO, APB2APE, I2S, SPDIF, etc.) out of reset
	 * and clock-enabled, otherwise reading AHUB devices (in our case,
	 * I2S/APBIF/AUDIO<XBAR>) will hang.
	 */
	clock_enable_audio();
}
static void mainboard_init(device_t dev)
{
	soc_configure_pads(padcfgs, ARRAY_SIZE(padcfgs));
	soc_configure_funits(funits, ARRAY_SIZE(funits));

	/* I2C6 bus (audio, etc.) */
	soc_configure_i2c6pad();
	i2c_init(I2C6_BUS);
	setup_audio();
}

static void mainboard_enable(device_t dev)
{
	dev->ops->init = &mainboard_init;
}

struct chip_operations mainboard_ops = {
	.name   = "smaug",
	.enable_dev = mainboard_enable,
};
