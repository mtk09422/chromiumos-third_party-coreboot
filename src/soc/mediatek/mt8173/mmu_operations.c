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

#include <arch/mmu.h>
#include <symbols.h>
#include <stdlib.h>
#include <stdint.h>
#include <soc/addressmap.h>
#include <soc/mcucfg.h>
#include <soc/mmu_operations.h>
#include <soc/mt8173.h>

static void mt8173_memrange_init(void)
{
	const unsigned long devmem = MA_DEV | MA_S | MA_RW;
	const unsigned long cachedmem = MA_MEM | MA_NS | MA_RW;
	const unsigned long secure_mem = MA_MEM | MA_S | MA_RW;
	uint64_t dram_start = ((uintptr_t)_dram);
	uint64_t dram_size = (uint64_t)CONFIG_DRAM_SIZE_MB * MiB;

	/* keep xlat table in sram */
	mmu_init(NULL, (uint64_t *)_sram_ttb, _sram_ttb_size);

	/* Device memory below DRAM is uncached */
	mmu_config_range(0, dram_start, devmem);

	/* SRAM is cached */
	mmu_config_range((void *)_sram, _sram_size, cachedmem);

	/* DRAM is cached */
	mmu_config_range((void *)dram_start, dram_size, cachedmem);

	/* set ttb as secure */
	mmu_config_range((void *)_sram_ttb, _sram_ttb_size, secure_mem);
}

void mt8173_mmu_init(void)
{
	mt8173_memrange_init();
	mmu_enable();
}
