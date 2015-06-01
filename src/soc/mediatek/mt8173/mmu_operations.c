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
	const unsigned long non_cachedmem = MA_MEM | MA_NS | MA_RW | MA_MEM_NC;
	uint64_t dram_start = ((uintptr_t)_dram);
	uint64_t dram_size = (uint64_t)CONFIG_DRAM_SIZE_MB * MiB;

	/* keep xlat table in sram */
	mmu_init(NULL, (uint64_t *)_sram_ttb, _sram_ttb_size);

	/* Device memory below DRAM is uncached */
	mmu_config_range(0, dram_start, devmem);

	/* SRAM is cached */
	mmu_config_range((void *)_sram, _sram_size, cachedmem);

	/* DMA is non-cached and is reserved for it6151 I2C DMA use */
	mmu_config_range((void *)_dma, _dma_size, non_cachedmem);

	/* DRAM is cached */
	mmu_config_range((void *)dram_start, dram_size, cachedmem);

	/* set ttb as secure */
	mmu_config_range((void *)_sram_ttb, _sram_ttb_size, secure_mem);
}

static void mt8173_sramrange_init(void)
{
	const unsigned long devmem = MA_DEV | MA_S | MA_RW;
	const unsigned long cachedmem = MA_MEM | MA_NS | MA_RW;
	const unsigned long secure_mem = MA_MEM | MA_S | MA_RW;
	const unsigned long non_cachedmem = MA_MEM | MA_NS | MA_RW | MA_MEM_NC;

	const uint64_t dram_size = (uint64_t)CONFIG_DRAM_SIZE_MB * MiB;

	mmu_init(NULL, (uint64_t *)_sram_ttb, _sram_ttb_size);

	/* Set 0x0 to end of dram as device memory */
	mmu_config_range((void *)0, (uintptr_t)_dram + dram_size, devmem);

	/* SRAM is cached */
	mmu_config_range((void *)_sram_l2c, _sram_l2c_size + _sram_size,
			 cachedmem);

	/* DMA is non-cached and is reserved for TPM & da9212 I2C DMA */
	mmu_config_range((void *)_dma, _dma_size, non_cachedmem);

	/* set ttb as secure */
	mmu_config_range((void *)_sram_ttb, _sram_ttb_size, secure_mem);
}

void mt8173_vboot2_mmu_init(void)
{
	mt8173_sramrange_init();
	mmu_enable();
}

void mt8173_mmu_init(void)
{
	/* Return L2C from SRAM.
	 * Sets it to 512KB which is the maximum available L2 cache for the
	 * A53s on MT8173
	 */
	write32(&mt8173_mcucfg->mp0_ca7l_cache_config, 3 << 8);

	mt8173_memrange_init();
	mmu_enable();
}
