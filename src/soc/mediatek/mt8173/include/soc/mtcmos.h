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

enum {
	CPU_ID_LITTLE_CPU0 = 0,
	CPU_ID_LITTLE_CPU1 = 1,
	CPU_ID_BIG_CPU0 = 4,
	CPU_ID_BIG_CPU1 = 5,
};

/* Interface to control specified CPU(linear_id) power on/off */
int mtcmos_ctrl_cpu(unsigned int linear_id, int state, int check_wfi);

#endif /* __SOC_MEDIATEK_MT8173_MTCMOS_H__ */
