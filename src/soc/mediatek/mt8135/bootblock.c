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

#include <arch/cache.h>
#include <arch/exception.h>
#include <arch/hlt.h>
#include <arch/stages.h>
#include <arch/io.h>
#include <bootblock_common.h>
#include <cbfs.h>
#include <console/console.h>
#include <timestamp.h>

#include <soc/mt8135.h>

#define AP_M4U_SRAM_CTRL    0x10205100+0x4
#define AUDIO_TOP_CON0      0x12070000+0x0

void main(void)
{
	const char *stage_name = "fallback/romstage";
	void *entry;

	timestamp_early_init(0);
//	bootblock_cpu_init();
	bootblock_mainboard_init();

	/* enable M4U SRAM (0x1202_0000 - 0x1202_5FFF) */
	setbits_le32((void *)AP_M4U_SRAM_CTRL, 0x10);
	clrbits_le32((void *)AUDIO_TOP_CON0, 0x4);

	entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA, stage_name);

	if (entry)
		stage_exit(entry);
	hlt();
}
