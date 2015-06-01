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

#include <arch/header.ld>

/*
 * SRAM_L2C is the half part of L2 cache that we borrow it to be used as SRAM.
 * It will be returned before starting the ramstage.
 * SRAM_L2C and SRAM can be cached, but only SRAM is DMA-able.
 */
#define SRAM_L2C_START(addr) SYMBOL(sram_l2c, addr)
#define SRAM_L2C_END(addr) SYMBOL(esram_l2c, addr)
#define SRAM_TTB(addr, size) REGION(sram_ttb, addr, size, 4K)

SECTIONS
{
	SRAM_L2C_START(0x000C0000)
	TIMESTAMP(0x000C0C00, 1K)
	BOOTBLOCK(0x000C1000, 78K)
	PRERAM_CBMEM_CONSOLE(0x000F4020, 8K - 32)
	PRERAM_CBFS_CACHE(0x000F6000, 16K)
	SRAM_L2C_END(0x00100000)

	SRAM_START(0x00100000)
	ROMSTAGE(0x0010A000, 118K)
	STACK(0x00128000, 4K)
	SRAM_TTB(0x00129000, 24K)
	SRAM_END(0x00130000)

	DRAM_START(0x40000000)
	POSTRAM_CBFS_CACHE(0x40100000, 1M)
	RAMSTAGE(0x40200000, 256K)
}
