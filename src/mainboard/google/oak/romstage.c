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
#include <arch/io.h>
#include <arch/mmu.h>

#include <cbfs.h>
#include <cbmem.h>
#include <console/console.h>
#include <symbols.h>
#include <delay.h>

#include <soc/addressmap.h>
#include <soc/mt8173.h>

void main(void)
{
	void *entry = NULL;

	/* init uart baudrate when pll on */
	console_init();

	trustzone_region_init();

	/* should be called after memory init */
	cbmem_initialize_empty();

	entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA,
		CONFIG_CBFS_PREFIX "/ramstage");

	stage_exit(entry);
}
