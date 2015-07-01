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

#include <arch/io.h>
#include <console/console.h>
#include <delay.h>
#include <soc/gpio.h>
#include <soc/mt8173.h>
#include <soc/mtcmos.h>
#include <soc/pericfg.h>
#include <soc/pll.h>
#include <soc/pmic.h>
#include <soc/pmic_wrap_init.h>
#include <soc/wdt.h>

extern void bootblock_soc_init(void);

void bootblock_mainboard_init(void);

void bootblock_mainboard_init(void)
{

	/* Clear UART0 power down signal */
	clrbits_le32(&mt8173_pericfg->pdn0_set, PERICFG_UART0_PDN);

	init_timer();

	mt_pll_init();
	mtcmos_ctrl_disp(STA_POWER_ON);
	mtcmos_ctrl_audio(STA_POWER_ON);

	mt_gpio_init();

	pwrap_init_preloader();

	/* init pmic i2c interface and pmic */
	pmic_init();

	mt_gpio_set_default_ext();

	/* post init pll */
	mt_pll_post_init();
	mt_arm_pll_sel();

	/* init watch dog, will disable AP watch dog */
	mtk_wdt_init();

	bootblock_soc_init();
}
