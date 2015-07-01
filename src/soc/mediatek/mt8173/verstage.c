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

#include <arch/cache.h>
#include <arch/exception.h>
#include <arch/hlt.h>
#include <arch/stages.h>
#include <console/console.h>
#include <soc/mmu_operations.h>
#include <soc/verstage.h>
#include <symbols.h>
#include <timestamp.h>
#include <vendorcode/google/chromeos/chromeos.h>

void __attribute__((weak)) verstage_mainboard_init(void)
{
	/* Default empty implementation. */
}

void main(void)
{
	void *entry;

	timestamp_add_now(TS_START_VBOOT);
	console_init();
	exception_init();
	verstage_mainboard_init();

	mt8173_mmu_enable();

	entry = vboot2_verify_firmware();

	if (entry)
		stage_exit(entry);
	hlt();
}
