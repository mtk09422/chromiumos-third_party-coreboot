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

#include <delay.h>
#include <console/console.h>
#include <arch/io.h>
#include <soc/wdt.h>

static struct mt8173_wdt_regs * const mt8173_wdt = (void *)RGU_BASE;

int mtk_wdt_init(void)
{
	uint32_t wdt_sta;

	/* Write Mode register will clear status register */
	wdt_sta = read32(&mt8173_wdt->wdt_status);

	if (wdt_sta)
		printk(BIOS_INFO, "WDT_STATUS: %#x\n", wdt_sta);
	else
		printk(BIOS_INFO, "WDT does not trigger reboot\n");

	/* Config HW reboot mode */
	clrsetbits_le32(&mt8173_wdt->wdt_mode,
			MTK_WDT_MODE_DUAL_MODE | MTK_WDT_MODE_IRQ |
			MTK_WDT_MODE_EXT_POL | MTK_WDT_MODE_ENABLE,
			MTK_WDT_MODE_EXTEN | MTK_WDT_MODE_KEY);

	printk(BIOS_INFO, "%s  WDT_MODE: %#x\n", __func__,
	       read32(&mt8173_wdt->wdt_mode));
	return wdt_sta;
}

void hard_reset(void)
{
	write32(&mt8173_wdt->wdt_swrst, MTK_WDT_SWRST_KEY);

	while (1)
		;
}
