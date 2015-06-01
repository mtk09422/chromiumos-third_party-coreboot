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

#include <arch/io.h>
#include <reset.h>
#include <soc/wdt.h>

static struct mt8173_wdt_regs * const mt8173_wdt = (void *)RGU_BASE;

void hard_reset(void)
{
	clrsetbits_le32(&mt8173_wdt->wdt_mode,
			MTK_WDT_MODE_DUAL_MODE | MTK_WDT_MODE_IRQ,
			MTK_WDT_MODE_KEY | MTK_WDT_MODE_EXTEN);
	setbits_le32(&mt8173_wdt->wdt_swrst, MTK_WDT_SWRST_KEY);

	while (1)
		;
}
