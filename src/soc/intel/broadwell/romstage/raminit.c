/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2014 Google Inc.
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

#include <arch/cbfs.h>
#include <cbfs.h>
#include <cbmem.h>
#include <console/console.h>
#include <lib.h>
#include <soc/pei_wrapper.h>
#include <soc/reset.h>
#include <soc/romstage.h>
#include <soc/systemagent.h>

/*
 * Find PEI executable in coreboot filesystem and execute it.
 */
void raminit(struct romstage_params *params)
{
	pei_wrapper_entry_t entry;
	struct pei_data *pei_data;
	int ret;

	/* Determine if mrc.bin is in the cbfs. */
	entry = (pei_wrapper_entry_t)cbfs_get_file_content(
		CBFS_DEFAULT_MEDIA, "mrc.bin", 0xab);
	if (entry == NULL) {
		printk(BIOS_DEBUG, "Couldn't find mrc.bin\n");
		return;
	}

	printk(BIOS_DEBUG, "Starting Memory Reference Code\n");

	pei_data = params->pei_data;
	ret = entry(pei_data);
	if (ret < 0)
		die("pei_data version mismatch\n");

	/* Print the MRC version after executing the UEFI PEI stage. */
	u32 version = MCHBAR32(MCHBAR_PEI_VERSION);
	printk(BIOS_DEBUG, "MRC Version %d.%d.%d Build %d\n",
		version >> 24 , (version >> 16) & 0xff,
		(version >> 8) & 0xff, version & 0xff);

	report_memory_config();

	/* Basic memory sanity test */
	quick_ram_check();

	if (pei_data->boot_mode != SLEEP_STATE_S3) {
		cbmem_initialize_empty();
	} else if (cbmem_initialize()) {
#if CONFIG_HAVE_ACPI_RESUME
		printk(BIOS_DEBUG, "Failed to recover CBMEM in S3 resume.\n");
		/* Failed S3 resume, reset to come up cleanly */
		reset_system();
#endif
	}
}
