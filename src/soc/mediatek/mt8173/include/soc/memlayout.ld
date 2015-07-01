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

#include <memlayout.h>
#include <vendorcode/google/chromeos/memlayout.h>

#include <arch/header.ld>

/*
 * SRAM_L2C is the half part of L2 cache that we borrow it to be used as SRAM.
 * It will be returned before starting the ramstage.
 * SRAM_L2C and SRAM can be cached, but only SRAM is DMA-able.
 */
#define SRAM_L2C_START(addr) SYMBOL(sram_l2c, addr)
#define SRAM_L2C_END(addr) SYMBOL(esram_l2c, addr)

SECTIONS
{
	SRAM_L2C_START(0x000C0000)
	BOOTBLOCK(0x000C1000, 85K)
	VERSTAGE(0x000D7000, 114K)
	PRERAM_CBFS_CACHE(0x000F6000, 16K)
	SRAM_L2C_END(0x00100000)

	SRAM_START(0x00100000)
	VBOOT2_WORK(0x00104000, 12K)
	TIMESTAMP(0x00109000, 1K)
	ROMSTAGE(0x0010A000, 118K)
	STACK(0x00128000, 4K)
	TTB(0x00129000, 24K)
	DMA_COHERENT(0x0012F000, 4K)
	SRAM_END(0x00130000)

	DRAM_START(0x40000000)
	POSTRAM_CBFS_CACHE(0x40100000, 1M)
	RAMSTAGE(0x40200000, 256K)
}
