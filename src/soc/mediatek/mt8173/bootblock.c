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

#include <arch/exception.h>
#include <arch/hlt.h>
#include <arch/stages.h>
#include <bootblock_common.h>
#include <cbfs.h>
#include <console/console.h>
#include <timestamp.h>
#include <soc/mmu_operations.h>

void main(void)
{
	void *entry;

	timestamp_early_init(0);

	if (IS_ENABLED(CONFIG_BOOTBLOCK_CONSOLE)) {
		console_init();
		exception_init();
	}

	bootblock_mainboard_init();

	mt8173_vboot2_mmu_init();

	if (IS_ENABLED(CONFIG_VBOOT2_VERIFY_FIRMWARE))
		entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA,
					CONFIG_CBFS_PREFIX "/verstage");
	else
		entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA,
					CONFIG_CBFS_PREFIX "/romstage");

	if (entry)
		stage_exit(entry);
	hlt();
}

void bootblock_soc_init(void)
{
}
