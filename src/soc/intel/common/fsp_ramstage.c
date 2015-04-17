/*
 * This file is part of the coreboot project.
 *
 * Copyright 2015 Google Inc.
 * Copyright (C) 2015 Intel Corporation.
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
 * Foundation, Inc.
 */

#include <cbmem.h>
#include <cbfs.h>
#include <console/console.h>
#include <fsp_util.h>
#include <romstage_handoff.h>
#include <soc/intel/common/ramstage.h>
#include <timestamp.h>

/* SOC initialization after FSP silicon init */
__attribute__((weak)) void soc_after_silicon_init(void)
{
	printk(BIOS_DEBUG, "WEAK: %s/%s called\n", __FILE__, __func__);
}

__attribute__((weak)) void soc_save_support_code(void *start, size_t size,
							void *entry)
{
	printk(BIOS_DEBUG, "WEAK: %s/%s called\n", __FILE__, __func__);
}

__attribute__((weak)) void *soc_restore_support_code(void)
{
	printk(BIOS_DEBUG, "WEAK: %s/%s called\n", __FILE__, __func__);
	return NULL;
}

static void fsp_run_silicon_init(void)
{
	FSP_INFO_HEADER *fsp_info_header;
	FSP_SILICON_INIT fsp_silicon_init;
	EFI_STATUS status;

	/* Find the FSP image */
	timestamp_add_now(TS_FSP_FIND_START);
	fsp_info_header = fsp_get_fih();
	timestamp_add_now(TS_FSP_FIND_END);

	if (fsp_info_header == NULL) {
		printk(BIOS_ERR, "FSP_INFO_HEADER not set!\n");
		return;
	}

	/* Perform silicon initialization after RAM is configured */
	printk(BIOS_DEBUG, "Calling FspSiliconInit\n");
	fsp_silicon_init = (FSP_SILICON_INIT)(fsp_info_header->ImageBase
		+ fsp_info_header->FspSiliconInitEntryOffset);
	timestamp_add_now(TS_FSP_SILICON_INIT_START);
	status = fsp_silicon_init(NULL);
	timestamp_add_now(TS_FSP_SILICON_INIT_END);
	printk(BIOS_DEBUG, "FspSiliconInit returned 0x%08x\n", status);

#if IS_ENABLED(CONFIG_DISPLAY_HOBS)
	/* Verify the HOBs */
	const EFI_GUID graphics_info_guid = EFI_PEI_GRAPHICS_INFO_HOB_GUID;
	void *hob_list_ptr = get_hob_list();
	int missing_hob = 0;

	if (hob_list_ptr == NULL)
		die("ERROR - HOB pointer is NULL!\n");
	print_hob_type_structure(0, hob_list_ptr);

	/*
	 * Verify that FSP is generating the required HOBs:
	 *	7.1: FSP_BOOTLOADER_TEMP_MEMORY_HOB only produced for FSP 1.0
	 *	7.2: FSP_RESERVED_MEMORY_RESOURCE_HOB verified by raminit
	 *	7.3: FSP_NON_VOLATILE_STORAGE_HOB verified by raminit
	 *	7.4: FSP_BOOTLOADER_TOLUM_HOB verified by raminit
	 */
	if (NULL == get_next_guid_hob(&graphics_info_guid, hob_list_ptr)) {
		printk(BIOS_ERR, "7.5: EFI_PEI_GRAPHICS_INFO_HOB missing!\n");
		missing_hob = 1;
	}
	if (missing_hob)
		die("ERROR - Missing one or more required FSP HOBs!\n");
#endif

	soc_after_silicon_init();
}

static void fsp_cache_save(void)
{
	const struct cbmem_entry *fsp_entry;
	FSP_INFO_HEADER *fih;

	fsp_entry = cbmem_entry_find(CBMEM_ID_REFCODE);

	if (fsp_entry == NULL) {
		printk(BIOS_ERR, "ERROR: FSP not found in CBMEM.\n");
		return;
	}

	fih = fsp_get_fih();

	if (fih == NULL) {
		printk(BIOS_ERR, "ERROR: No FIH found.\n");
		return;
	}

	soc_save_support_code(cbmem_entry_start(fsp_entry),
				cbmem_entry_size(fsp_entry), fih);
}

static int fsp_find_and_relocate(void)
{
	struct cbfs_file *file;
	void *fih;

	file = cbfs_get_file(CBFS_DEFAULT_MEDIA, "fsp.bin");

	if (file == NULL) {
		printk(BIOS_ERR, "Couldn't find fsp.bin in CBFS.\n");
		return -1;
	}

	fih = fsp_relocate(CBFS_SUBHEADER(file), ntohl(file->len));

	fsp_update_fih(fih);

	return 0;
}

void intel_silicon_init(void)
{
	struct romstage_handoff *handoff;

	handoff = cbmem_find(CBMEM_ID_ROMSTAGE_INFO);

	if (handoff != NULL && handoff->s3_resume)
		fsp_update_fih(soc_restore_support_code());
	else {
		fsp_find_and_relocate();
		fsp_cache_save();
	}

	fsp_run_silicon_init();
}
