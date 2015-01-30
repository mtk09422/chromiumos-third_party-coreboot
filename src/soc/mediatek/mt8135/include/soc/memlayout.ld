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

#include <memlayout.h>
/*
#include <vendorcode/google/chromeos/vboot2/memlayout.h>
*/
#include <arch/header.ld>

/*
 * [FIXME] Should be updated by someone who knows the memory layout better.
 */

/*
 * [TODO] TIMESTAMP(start, size), SRAM_END (?)
 *        add POSTRAM_CBFS_CACHE (start, size),
 *            RAMSTAGE(0x80080000, size),
 *            DMA_COHERENT(start, size)
 */

#define SRAM_M4U_START(addr) SYMBOL(sram_m4u, addr)
#define SRAM_M4U_END(addr) SYMBOL(esram_m4u, addr)

SECTIONS
{
	SRAM_START(0x00100000)
	TTB(0x00100000, 16K + 32)
	PRERAM_CBMEM_CONSOLE(0x00104020, 8K - 32)
	STACK(0x00106000, 16K)
	PRERAM_CBFS_CACHE(0x0010A000, 64K)
	ROMSTAGE(0x0011A000, 88K)
	SRAM_END(0x00130000)

	SRAM_M4U_START(0x12000000)
	TIMESTAMP(0x12000C00, 1K)
	BOOTBLOCK(0x12001000, 48K)
	SRAM_M4U_END(0x12026000)

	DMA_COHERENT(0x5a000000, 2M)

	DRAM_START(0x80000000)
	RAMSTAGE(0x80080000, 128K)
}
