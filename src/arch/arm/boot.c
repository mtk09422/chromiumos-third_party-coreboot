/*
 * This file is part of the coreboot project.
 *
 * Copyright 2013 Google Inc.
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
#include <arch/stages.h>
#include <cbmem.h>
#include <console/console.h>

void jmp_to_elf_entry(void *entry, unsigned long buffer, unsigned long size)
{
	void (*doit)(void *) = entry;
	void *cb_tables = cbmem_find(CBMEM_ID_CBTABLE);

	printk(BIOS_SPEW, "entry    = %p\n", entry);
	cache_sync_instructions();
	doit(cb_tables);
}
