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
#include <console/console.h>
#include <device/device.h>
#include <delay.h>
#include <edid.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <soc/addressmap.h>
#include <soc/clock.h>
#include <soc/display.h>
#include <soc/ddp.h>
#include <soc/dsi.h>

#include "chip.h"

static void convert_config_to_vidoemode(struct soc_mediatek_mt8173_config *conf,
				   struct videomode *mode)
{
	mode->pixelclock = (unsigned long)conf->pixelclock,
	mode->hactive    = conf->hactive;
	mode->hback_porch  = conf->hback_porch;
	mode->hfront_porch = conf->hfront_porch;
	mode->hsync_len    = conf->hsync_len;
	mode->vactive      = conf->vactive;
	mode->vback_porch  = conf->vback_porch;
	mode->vfront_porch = conf->vfront_porch;
	mode->vsync_len = conf->vsync_len;
}

void mt8173_display_init(device_t dev)
{
	struct edid edid;
	struct soc_mediatek_mt8173_config *conf = dev->chip_info;
	struct videomode mode;

	edid.ha = conf->hactive;
	edid.va = conf->vactive;
	edid.framebuffer_bits_per_pixel = conf->framebuffer_bits_per_pixel;
	edid.bytes_per_line = edid.ha * conf->framebuffer_bits_per_pixel / 8;
	edid.x_resolution = edid.ha;
	edid.y_resolution = edid.va;

	convert_config_to_vidoemode(conf, &mode);
	mtk_ddp_init();
	mtk_dsi_int(&mode);

	mtk_ddp_mode_set(0, &edid);

	set_vbe_mode_info_valid(&edid, (uintptr_t)0);
	mainboard_power_on_backlight();
}
