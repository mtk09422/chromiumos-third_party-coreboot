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

#ifndef __SOC_MEDIATEK_MT8173_MTCMOS_H__
#define __SOC_MEDIATEK_MT8173_MTCMOS_H__

enum {
	STA_POWER_DOWN = 0,
	STA_POWER_ON = 1
};

void mtcmos_ctrl_disp(int state);
void mtcmos_ctrl_audio(int state);
#endif /* __SOC_MEDIATEK_MT8173_MTCMOS_H__ */
