/*
 * This file is part of the coreboot project.
 *
 * Copyright 2015 Google Inc.
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

#ifndef __MAINBOARD_GOOGLE_OAK_GPIO_H__
#define __MAINBOARD_GOOGLE_OAK_GPIO_H__

#include <gpio.h>

enum {
	/* Board ID related GPIOS. */
	LID		= GPIO(12),
	BOARD_ID_0	= GPIO(55),
	BOARD_ID_1	= GPIO(56),
	BOARD_ID_2	= GPIO(53),
	/* Warm reset */
	AP_SYS_RESET_L	= GPIO(121),
	/* Write Protect */
	WRITE_PROTECT_L	= GPIO(4),
	/* Power button - Depending on board id, maybe active high / low */
	POWER_BUTTON	= GPIO(14),
	/* EC in RW signal */
	EC_IN_RW	= GPIO(34),
};

#endif /* __MAINBOARD_GOOGLE_OAK_GPIO_H__ */
