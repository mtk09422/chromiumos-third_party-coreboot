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

#ifndef _PS8640_H_
#define _PS8640_H_

struct ps8640_bridge {
	int gpio_pd_n;
	int gpio_rst_n;
	int gpio_1v2_n;
	int gpio_mode_sel_n;
	int enabled;
	int gpio_panel_3v3_n;
	int gpio_panel_bl_n;
};

void ps8640_enable(void);
void ps8640_disable(void);
void ps8640_pre_enable(void);
int  ps8640_init(void);

#endif

