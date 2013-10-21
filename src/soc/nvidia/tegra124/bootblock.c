/*
 * This file is part of the coreboot project.
 *
 * Copyright 2013 Google Inc.
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

#include <arch/hlt.h>
#include <bootblock_common.h>
#include <cbfs.h>
#include <console/console.h>

#include "clock.h"
#include "pinmux.h"
#include "power.h"

void main(void)
{
	void *entry;

	clock_init();

	clock_uart_config();

	// Serial out, tristate off.
	pinmux_set_config(PINMUX_KB_ROW9_INDEX, PINMUX_KB_ROW9_FUNC_UA3);
	// Serial in, tristate_on.
	pinmux_set_config(PINMUX_KB_ROW10_INDEX, PINMUX_KB_ROW10_FUNC_UA3 |
						 PINMUX_TRISTATE |
						 PINMUX_INPUT_ENABLE);

	if (CONFIG_BOOTBLOCK_CONSOLE)
		console_init();

	bootblock_mainboard_init();

	pinmux_set_config(PINMUX_CORE_PWR_REQ_INDEX,
			  PINMUX_CORE_PWR_REQ_FUNC_PWRON);
	pinmux_set_config(PINMUX_CPU_PWR_REQ_INDEX,
			  PINMUX_CPU_PWR_REQ_FUNC_CPU);
	pinmux_set_config(PINMUX_PWR_INT_N_INDEX,
			  PINMUX_PWR_INT_N_FUNC_PMICINTR |
			  PINMUX_TRISTATE |
			  PINMUX_INPUT_ENABLE);

	power_enable_cpu_rail();
	power_ungate_cpu();

	entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA, "fallback/romstage");

	if (entry)
		clock_cpu0_config_and_reset(entry);

	hlt();
}