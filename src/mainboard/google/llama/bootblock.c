/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 Google Inc.
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

#include <bootblock_common.h>
#include <arch/io.h>
#include <console/console.h>
#include <delay.h>
#include <soc/mt8135.h>
#include <soc/pll.h>
#include <soc/gpio.h>
#include <soc/pmic_wrap_init.h>
#include <soc/pmic_6397.h>
#include <soc/key.h>

void bootblock_mainboard_init(void)
{
	printk(BIOS_INFO, "bootblock_mainboard_init\n");

	init_timer();

	mt_pll_init();
	mt_gpio_init();

	/* init pmic i2c interface and pmic */
	/* retry 3 times for pmic wrapper init */
	pwrap_init_preloader();
	pmic6397_init();

	bootblock_soc_init();

	mt_gpio_set_default();

	/*init kpd PMIC mode support */
	set_kpd_pmic_mode();
}
