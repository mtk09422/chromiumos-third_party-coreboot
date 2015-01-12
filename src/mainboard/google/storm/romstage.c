/*
 * This file is part of the coreboot project.
 *
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
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

#include <arch/stages.h>
#include <cbfs.h>
#include <cbmem.h>
#include <console/console.h>
#include <romstage_handoff.h>
#include <vendorcode/google/chromeos/chromeos.h>
#include <soc/soc_services.h>
#include "mmu.h"

void main(void)
{
	void *entry;

	console_init();

	initialize_dram();

	/* Add dram mappings to mmu tables. */
	setup_dram_mappings(DRAM_INITIALIZED);

	cbmem_initialize_empty();

	entry = vboot2_load_ramstage();

	/*
	 * Presumably the only reason vboot2 would return NULL is that we're
	 * running in recovgery mode, otherwise it would have reset the
	 * device.
	 */
	if (!entry)
		entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA,
					CONFIG_CBFS_PREFIX "/ramstage");

	stage_exit(entry);
}
