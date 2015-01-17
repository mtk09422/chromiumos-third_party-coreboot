/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2013 Google Inc.
 * Copyright (C) 2015 Intel Corp.
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

#include <cbmem.h>
#include <cpu/x86/mtrr.h>
#include <soc/intel/common/stack.h>
#include <soc/romstage.h>
#include <soc/smm.h>

/*
 * Romstage needs quite a bit of stack for decompressing images since the lzma
 * lib keeps its state on the stack during romstage.
 */
static unsigned long choose_top_of_stack(void)
{
	unsigned long stack_top;
	const unsigned long romstage_ram_stack_size = 0x5000;

	/* cbmem_add() does a find() before add(). */
	stack_top = (unsigned long)cbmem_add(CBMEM_ID_ROMSTAGE_RAM_STACK,
					     romstage_ram_stack_size);
	stack_top += romstage_ram_stack_size;
	return stack_top;
}

/*
 * setup_stack_and_mtrrs() determines the stack to use after
 * cache-as-ram is torn down as well as the MTRR settings to use.
 */
void *setup_stack_and_mtrrs(void)
{
	unsigned long top_of_stack;
	int num_mtrrs;
	uint32_t *slot;
	uint32_t mtrr_mask_upper;
	uint32_t tseg_base;
	uint32_t tseg_size;

	/* Top of stack needs to be aligned to a 4-byte boundary. */
	top_of_stack = choose_top_of_stack() & ~3;
	slot = (void *)top_of_stack;
	num_mtrrs = 0;

	/*
	 * The upper bits of the MTRR mask need to set according to the number
	 * of physical address bits.
	 */
	mtrr_mask_upper = (1 << ((cpuid_eax(0x80000008) & 0xff) - 32)) - 1;

	/*
	 * The order for each MTRR is value then base with upper 32-bits of
	 * each value coming before the lower 32-bits. The reasoning for
	 * this ordering is to create a stack layout like the following:
	 *   +0: Number of MTRRs
	 *   +4: MTRR base 0 31:0
	 *   +8: MTRR base 0 63:32
	 *  +12: MTRR mask 0 31:0
	 *  +16: MTRR mask 0 63:32
	 *  +20: MTRR base 1 31:0
	 *  +24: MTRR base 1 63:32
	 *  +28: MTRR mask 1 31:0
	 *  +32: MTRR mask 1 63:32
	 */

	/* Cache the ROM as WP just below 4GiB. */
	slot = stack_push32(slot, mtrr_mask_upper); /* upper mask */
	slot = stack_push32(slot, ~(CONFIG_ROM_SIZE - 1) | MTRRphysMaskValid);
	slot = stack_push32(slot, 0); /* upper base */
	slot = stack_push32(slot, ~(CONFIG_ROM_SIZE - 1) | MTRR_TYPE_WRPROT);
	num_mtrrs++;

	/* Cache RAM as WB from 0 -> CONFIG_RAMTOP. */
	slot = stack_push32(slot, mtrr_mask_upper); /* upper mask */
	slot = stack_push32(slot, ~(CONFIG_RAMTOP - 1) | MTRRphysMaskValid);
	slot = stack_push32(slot, 0); /* upper base */
	slot = stack_push32(slot, 0 | MTRR_TYPE_WRBACK);
	num_mtrrs++;

	/*
	 * Determine the start of TSEG.  This is not the start of usable RAM
	 * because FSP has reserved some of the memory just below TSEG.
	 */
	tseg_base = (uint32_t)smm_region_start();
	tseg_size = (uint32_t)smm_region_size();

	/*
	 * Cache the same size as TSEG below TSEG Base for use by ramstage
	 * before setting up the entire RAM as cacheable. Using the size of
	 * TSEG removes determining alignment.
	 */
	slot = stack_push32(slot, mtrr_mask_upper); /* upper mask */
	slot = stack_push32(slot, ~(tseg_size - 1) | MTRRphysMaskValid);
	slot = stack_push32(slot, 0); /* upper base */
	slot = stack_push32(slot, (tseg_base - tseg_size) | MTRR_TYPE_WRBACK);
	num_mtrrs++;

	/*
	 * Cache 8MiB at the top of ram. Top of ram is where the TSEG
	 * region resides. However, it is not restricted to SMM mode until
	 * SMM has been relocated. By setting the region to cacheable it
	 * provides faster access when relocating the SMM handler as well
	 * as using the TSEG region for other purposes.
	 */
	slot = stack_push32(slot, mtrr_mask_upper); /* upper mask */
	slot = stack_push32(slot, ~(tseg_size - 1) | MTRRphysMaskValid);
	slot = stack_push32(slot, 0); /* upper base */
	slot = stack_push32(slot, tseg_base | MTRR_TYPE_WRBACK);
	num_mtrrs++;

	/*
	 * Save the number of MTRRs to setup. Return the stack location
	 * pointing to the number of MTRRs.
	 */
	slot = stack_push32(slot, num_mtrrs);

	return slot;
}

