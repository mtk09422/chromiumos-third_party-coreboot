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
#include <arch/cache.h>
#include <console/console.h>
#include <symbols.h>
#include <stdlib.h>
#include <stdint.h>
#include <soc/addressmap.h>
#include <soc/mcucfg.h>
#include <soc/mmu_operations.h>
#include <soc/mt8173.h>

static const uint64_t dram_size = (uint64_t)CONFIG_DRAM_SIZE_MB * MiB;

static void mt8173_memrange_init(void)
{
	/* keep xlat table in sram */
	mmu_init(NULL, (uint64_t *)_ttb, _ttb_size);

	/* Device memory below DRAM is uncached */
	mmu_config_range(0, (uintptr_t)_dram, DEV_MEM);

	/* SRAM is cached */
	mmu_config_range(_sram, _sram_size, CACHED_MEM);

	/* DMA is non-cached and is reserved for it6151 I2C DMA use */
	mmu_config_range(_dma_coherent, _dma_coherent_size, UNCACHED_MEM);

	/* DRAM is cached */
	mmu_config_range(_dram, dram_size, CACHED_MEM);

	/* set ttb as secure */
	mmu_config_range(_ttb, _ttb_size, SECURE_MEM);
}

void mt8173_mmu_init(void)
{
	/* Return L2C from SRAM.
	 * Sets it to 512KB which is the maximum available L2 cache for the
	 * A53s on MT8173
	 */
	write32(&mt8173_mcucfg->mp0_ca7l_cache_config, 3 << 8);

	cache_sync_instructions();
	mt8173_memrange_init();
	mmu_enable();
}

void mt8173_mmu_enable(void)
{
	uint32_t sctlr;

	cache_sync_instructions();

	/* Ensure system register writes are committed before enabling MMU */
	isb();

	/* Enable MMU */
	sctlr = raw_read_sctlr_el3();
	sctlr |= SCTLR_C | SCTLR_M | SCTLR_I;
	raw_write_sctlr_el3(sctlr);

	isb();
}
