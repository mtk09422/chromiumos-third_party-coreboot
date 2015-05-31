/*
 * This file is part of the coreboot project.
 *
 * Copyright 2015 Mediatek Inc.
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

#include <boardid.h>
#include <gpio.h>
#include <console/console.h>
#include <stdlib.h>
#include "gpio.h"

static int board_id_value = -1;

static uint8_t get_board_id(void)
{
	uint8_t bid;
	bid = gpio_get(BOARD_ID_0) << 0 |
	      gpio_get(BOARD_ID_1) << 1 |
	      gpio_get(BOARD_ID_2) << 2;

	if (bid == 4)
		bid = 0;

	printk(BIOS_INFO, "Board ID %d\n", bid);

	return bid;
}

uint8_t board_id(void)
{
	if (board_id_value < 0)
		board_id_value = get_board_id();

	return board_id_value;
}

uint32_t ram_code(void)
{
	uint32_t code;

	code = gpio_get(RAM_ID_0) << 0 |
	       gpio_get(RAM_ID_1) << 1 |
	       gpio_get(RAM_ID_2) << 2 |
	       gpio_get(RAM_ID_3) << 3;

	return code;
}
