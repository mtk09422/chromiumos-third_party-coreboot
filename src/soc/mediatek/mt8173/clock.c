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

#include <stdint.h>
#include <arch/io.h>
#include <arch/clock.h>
#include <soc/addressmap.h>
#include <soc/clock.h>

enum {
	MTK_XGPT = MTK_MCUCFG_BASE + 0x670, // XGPT control register
	MTK_XGPT_IDX = MTK_MCUCFG_BASE + 0x674 // XGPT control index register
};

void clock_init_mt8173_timer(void)
{
	const uint32_t freq = 13 * MHz;

	/* align the clock of coprocessor to 13MHz */
	write32((uint32_t *)MTK_XGPT_IDX, 0);
	write32((uint32_t *)MTK_XGPT, (0x1 | (0x2 << 8)));
	set_cntfrq(freq);
}
