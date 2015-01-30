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

#include <arch/stages.h>
#include <arch/exception.h>
#include <arch/io.h>

#include <cbfs.h>
#include <cbmem.h>
#include <console/console.h>
#include <symbols.h>

#include <soc/mt8135.h>
#include <soc/pll.h>
#include <soc/i2c.h>
#include <soc/wdt.h>
#include <soc/memory.h>

void main(void)
{
	/* Clear UART0 reset signal */
	clrbits_le32((void *)AP_PERI_GLOBALCON_RST0, UART0_SW_RST);
	/* Clear UART0 power down signal */
	clrbits_le32((void *)AP_PERI_GLOBALCON_PDN0, UART0_PDN);

	/* init memory */
	mt_mem_init();

	/* init uart baudrate when pll on */
	console_init();

	exception_init();

	/* Setup MMU and CBMEM, in MB */
	u32 dram_start = ((uintptr_t)_dram >> 20);
	u32 dram_size = 1024;
	u32 dram_end = dram_start + dram_size;

	mmu_init();

	/* Device memory below DRAM is uncached. */
	mmu_config_range(0, dram_start, DCACHE_OFF);

	/* TTB is cached */
	mmu_config_range((uintptr_t)_ttb >> 20, 2, DCACHE_WRITEBACK);

	/* DRAM is cached. */
	mmu_config_range(dram_start, dram_size, DCACHE_WRITEBACK);

	/* A window for DMA is uncached. */
	mmu_config_range((uintptr_t)_dma_coherent >> 20,
			 _dma_coherent_size >> 20, DCACHE_OFF);
	/* The space above DRAM is uncached. */
	if (dram_end < 4096)
		mmu_config_range(dram_end, 4096 - dram_end, DCACHE_OFF);
	mmu_disable_range(0, 1);
	printk(BIOS_DEBUG, "dcache_mmu_enable\n");
	dcache_mmu_enable();

	cbmem_initialize_empty();

	/* post init pll */
	mt_pll_post_init();

	/* Setup TPM */
	mtk_i2c_init(6, 1, 0x20, 0);

	void *entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA,
				      "fallback/ramstage");

	stage_exit(entry);
}
