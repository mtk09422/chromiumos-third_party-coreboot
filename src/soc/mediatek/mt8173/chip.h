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


#ifndef __SOC_MEDIATEK_MT8173_CHIP_H__
#define __SOC_MEDIATEK_MT8173_CHIP_H__

#include <soc/gpio.h>

struct soc_mediatek_mt8173_config {
	u32 framebuffer_bits_per_pixel;

	u32 pixelclock;
	u32 hactive;
	u32 hfront_porch;
	u32 hback_porch;
	u32 hsync_len;
	u32 vactive;
	u32 vfront_porch;
	u32 vback_porch;
	u32 vsync_len;

};

#endif /* __SOC_MEDIATEK_MT8173_CHIP_H__ */
