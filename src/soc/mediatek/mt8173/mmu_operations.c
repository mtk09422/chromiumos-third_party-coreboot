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
#include <arch/cache.h>
#include <arch/mmu.h>
#include <console/console.h>
#include <memrange.h>
#include <symbols.h>
#include <stdlib.h>
#include <stdint.h>
#include <soc/addressmap.h>
#include <soc/mmu_operations.h>
#include <soc/mt8173.h>

/* This structure keeps track of all the mmap memory ranges for mt8173 */
static struct memranges mt8173_mmap_ranges;

static void mt8173_memrange_init(struct memranges *map)
{
	const unsigned long devmem = MA_DEV | MA_S | MA_RW;
	const unsigned long cachedmem = MA_MEM | MA_NS | MA_RW;
	const unsigned long secure_mem = MA_MEM | MA_S | MA_RW;
	uint64_t dram_start = ((uintptr_t)_dram);
	uint64_t dram_size = (uint64_t)CONFIG_DRAM_SIZE_MB * MiB;
	uint64_t ttb_size = TTB_SIZE * MiB;
	uint64_t tz_start;
	size_t tz_size;

	memranges_init_empty(map);

	/* Device memory below DRAM is uncached */
	memranges_insert(map, 0, dram_start, devmem);

	/* SRAM is cached */
	memranges_insert(map, (uintptr_t)_sram, _sram_size, cachedmem);

	/* DRAM is cached */
	memranges_insert(map, dram_start, dram_size, cachedmem);

	/* Add trustzone carveout region */
	carveout_range(CARVEOUT_TZ, &tz_start, &tz_size);
	tz_start *= MiB;
	memranges_insert(map, tz_start, tz_size * MiB, secure_mem);

	mmu_init(map, (void *)tz_start, ttb_size);
}

static void mt8173_sramrange_init(struct memranges *map)
{
	const unsigned long devmem = MA_DEV | MA_S | MA_RW;
	const unsigned long cachedmem = MA_MEM | MA_NS | MA_RW;
	const unsigned long secure_mem = MA_MEM | MA_S | MA_RW;
	const uint64_t dram_size = (uint64_t)CONFIG_DRAM_SIZE_MB * MiB;

	memranges_init_empty(map);
	/* Set 0x0 to end of dram as device memory */
	memranges_insert(map, 0, (uintptr_t)(_dram + dram_size), devmem);
	/* SRAM is cached */
	memranges_insert(map, (uintptr_t)_sram_l2c, _sram_l2c_size + _sram_size,
			 cachedmem);

	memranges_insert(map, (uintptr_t)_sram_ttb, _sram_ttb_size, secure_mem);
	mmu_init(map, (void *)_sram_ttb, _sram_ttb_size);
}

void mainboard_add_memory_ranges(struct memranges *map)
{
	/* Don't add any ranges by default. */
}

void mt8173_vboot2_mmu_init(void)
{
	struct memranges *map = &mt8173_mmap_ranges;

	mt8173_sramrange_init(map);
	mainboard_add_memory_ranges(map);
	mmu_enable();
}

void mt8173_mmu_init(void)
{
	struct memranges *map = &mt8173_mmap_ranges;

	/* Return L2C from SRAM */
	write32((uint32_t *)MTK_MCUCFG_BASE, 0x300);

	mt8173_memrange_init(map);
	mainboard_add_memory_ranges(map);
	mmu_enable();
}
