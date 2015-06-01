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
#include <arch/cpu.h>
#include <arch/exception.h>
#include <arch/io.h>
#include <arch/mmu.h>
#include <arch/stages.h>

#include <cbfs.h>
#include <cbmem.h>
#include <console/console.h>
#include <delay.h>
#include <romstage_handoff.h>
#include <symbols.h>
#include <timestamp.h>
#include <vendorcode/google/chromeos/chromeos.h>

#include <soc/addressmap.h>
#include <soc/memory.h>
#include <soc/mt8173.h>
#include <soc/pll.h>

void main(void)
{
	void *entry = NULL;
	timestamp_add_now(TS_START_ROMSTAGE);

	/* init uart baudrate when pll on */
	console_init();
	exception_init();

	/* init memory */
	mt_mempll_pre();
	mt_mempll_post();
	/* set mem_clk */
	write32((void *)(uintptr_t)CLK_CFG_0, 0x01000105);
	mt_mem_init();

	trustzone_region_init();

	/* should be called after memory init */
	cbmem_initialize_empty();

	entry = vboot2_load_ramstage();

	timestamp_add_now(TS_START_COPYRAM);

	if (entry == NULL)
		entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA,
			CONFIG_CBFS_PREFIX "/ramstage");

	timestamp_add_now(TS_END_ROMSTAGE);

	stage_exit(entry);
}
