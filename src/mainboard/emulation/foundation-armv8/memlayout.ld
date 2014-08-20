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
 * Memory map for Foundation v2 model
 * Reference: ARM v8-A Foundation Model User Guide
 *
 * 0x00_0000_0000 - 0x00_03FF_FFFF		Trusted Boot ROM
 * 0x00_0400_0000 - 0x00_0403_FFFF		Trusted SRAM
 * 0x00_0600_0000 - 0x00_07FF_FFFF		Trusted DRAM
 * 0x00_1C01_0000 - 0x00_1C01_FFFF		System Registers
 * 0x00_1C09_0000 - 0x00_1C09_FFFF		UART0 (PL011)
 * 0x00_2E00_0000 - 0x00_2E00_FFFF		Non-trusted SRAM
 * 0x00_8000_0000 - 0x00_FFFF_FFFF		DRAM (0GB - 2GB)
 * 0x08_8000_0000 - 0x09_FFFF_FFFF		DRAM (2GB - 8GB)
 */

SECTIONS
{
	/* ROM_START(0x00000000) */
	BOOTBLOCK(0x00000000, 32K)
	/* ROM_END(0x04000000) */

	SRAM_START(0x04000000)
	ROMSTAGE(0x04000000, 128K)
	SRAM_END(0x04030000)

	DRAM_START(0x80000000)
	RAMSTAGE(0x80000000, 128K)
	STACK(0x90000000, 16K)
}
