/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2014-2015 Intel Corporation
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

#include <cbmem.h>
#include <console/console.h>
#include <fsp_util.h>
#include <lib.h> /* hexdump */
#include <reset.h>
#include <soc/intel/common/memmap.h>
#include <soc/pei_data.h>
#include <soc/romstage.h>
#include <string.h>
#include <timestamp.h>

void raminit(struct romstage_params *params)
{
	FSP_INFO_HEADER *fsp_header;
	EFI_HOB_RESOURCE_DESCRIPTOR *fsp_memory;
	FSP_MEMORY_INIT fsp_memory_init;
	FSP_MEMORY_INIT_PARAMS fsp_memory_init_params;
	const EFI_GUID fsp_reserved_guid =
		FSP_RESERVED_MEMORY_RESOURCE_HOB_GUID;
	FSP_INIT_RT_COMMON_BUFFER fsp_rt_common_buffer;
	void *hob_list_ptr;
	const EFI_GUID mrc_guid = FSP_NON_VOLATILE_STORAGE_HOB_GUID;
	u32 *mrc_hob;
	EFI_STATUS status;
	struct pei_data *pei_ptr;
	VPD_DATA_REGION *vpd_ptr;
	UPD_DATA_REGION *upd_ptr;
	UPD_DATA_REGION upd_data_buffer;
#if IS_ENABLED(CONFIG_DISPLAY_HOBS)
	const EFI_GUID bootldr_tolum_guid = FSP_BOOTLOADER_TOLUM_HOB_GUID;
	EFI_HOB_RESOURCE_DESCRIPTOR *cbmem_root;
	unsigned long int data;
	void *fsp_reserved_memory_area;
	int missing_hob = 0;
	EFI_PEI_HOB_POINTERS hob_ptr;
#endif

	/*
	 * Find and copy the UPD region to the stack so the platform can modify
	 * the settings if needed.  Modifications to the UPD buffer are done in
	 * the platform callback code.  The platform callback code is also
	 * responsible for assigning the UpdDataRngPtr to this buffer if any
	 * updates are made.  The default state is to leave the UpdDataRngPtr
	 * set to NULL.  This indicates that the FSP code will use the UPD
	 * region in the FSP binary.
	 */
	post_code(0x34);
	fsp_header = params->chipset_context;
	vpd_ptr = (VPD_DATA_REGION *)(fsp_header->CfgRegionOffset +
					fsp_header->ImageBase);
	printk(BIOS_DEBUG, "VPD Data: 0x%p\n", vpd_ptr);
	upd_ptr = (UPD_DATA_REGION *)(vpd_ptr->PcdUpdRegionOffset +
					fsp_header->ImageBase);
	printk(BIOS_DEBUG, "UPD Data: 0x%p\n", upd_ptr);
	memcpy(&upd_data_buffer, upd_ptr, sizeof(upd_data_buffer));

	/* Zero fill RT Buffer data and start populating fields. */
	memset(&fsp_rt_common_buffer, 0, sizeof(fsp_rt_common_buffer));
	pei_ptr = params->pei_data;
	if (pei_ptr->boot_mode == SLEEP_STATE_S3) {
		fsp_rt_common_buffer.BootMode = BOOT_ON_S3_RESUME;
	} else if (pei_ptr->saved_data != NULL) {
		fsp_rt_common_buffer.BootMode =
			BOOT_ASSUMING_NO_CONFIGURATION_CHANGES;
	} else {
		fsp_rt_common_buffer.BootMode = BOOT_WITH_FULL_CONFIGURATION;
	}
	fsp_rt_common_buffer.UpdDataRgnPtr = &upd_data_buffer;
	fsp_rt_common_buffer.BootLoaderTolumSize = CBMEM_ROOT_SIZE;

	/* Get any board specific changes */
	fsp_memory_init_params.NvsBufferPtr = (void *)pei_ptr->saved_data;
	fsp_memory_init_params.RtBufferPtr = &fsp_rt_common_buffer;
	fsp_memory_init_params.HobListPtr = &hob_list_ptr;
	board_fsp_memory_init_params(params, fsp_header,
		&fsp_memory_init_params);
	post_code(0x36);

	/* Display the UPD data */
#if IS_ENABLED(CONFIG_DISPLAY_UPD_DATA)
	printk(BIOS_SPEW, "Updated Product Data (UPD):\n");
	hexdump32(BIOS_SPEW, (void *)&upd_data_buffer, sizeof(upd_data_buffer));
#endif

	/* Call FspMemoryInit to initialize RAM */
	fsp_memory_init = (FSP_MEMORY_INIT)(fsp_header->ImageBase
		+ fsp_header->FspMemoryInitEntryOffset);
	printk(BIOS_DEBUG, "Calling FspMemoryInit: 0x%p\n", fsp_memory_init);
	printk(BIOS_SPEW, "    0x%p: NvsBufferPtr\n",
		fsp_memory_init_params.NvsBufferPtr);
	printk(BIOS_SPEW, "    0x%p: RtBufferPtr\n",
		fsp_memory_init_params.RtBufferPtr);
	printk(BIOS_SPEW, "    0x%p: HobListPtr\n",
		fsp_memory_init_params.HobListPtr);

	timestamp_add_now(TS_FSP_MEMORY_INIT_START);
	status = fsp_memory_init(&fsp_memory_init_params);
	post_code(0x37);
	timestamp_add_now(TS_FSP_MEMORY_INIT_END);

	printk(BIOS_DEBUG, "FspMemoryInit returned 0x%08x\n", status);
	if (status != EFI_SUCCESS)
		die("ERROR - FspMemoryInit failed to initialize memory!\n");

	/* Locate the FSP reserved memory area */
	fsp_memory = get_next_resource_hob(&fsp_reserved_guid, hob_list_ptr);
	if (fsp_memory == NULL)
		die("FSP_RESERVED_MEMORY_RESOURCE_HOB missing!\n");
	printk(BIOS_DEBUG, "Reserving 0x%016lx bytes for FSP\n",
		(unsigned long int)fsp_memory->ResourceLength);

	/* Display SMM area */
#if IS_ENABLED(CONFIG_HAVE_SMI_HANDLER)
	char *smm_base;
	size_t smm_size;

	smm_region((void **)&smm_base, &smm_size);
	printk(BIOS_DEBUG, "0x%08x: smm_size\n", smm_size);
	printk(BIOS_DEBUG, "0x%p: smm_base\n", smm_base);
#endif

	/* Migrate CAR data */
	printk(BIOS_DEBUG, "0x%p: cbmem_top\n", cbmem_top());
	if (pei_ptr->boot_mode != SLEEP_STATE_S3) {
		cbmem_initialize_empty_id_size(CBMEM_ID_FSP_RESERVED_MEMORY,
			fsp_memory->ResourceLength);
	} else if (cbmem_initialize_id_size(CBMEM_ID_FSP_RESERVED_MEMORY,
		fsp_memory->ResourceLength)) {
#if IS_ENABLED(CONFIG_HAVE_ACPI_RESUME)
		printk(BIOS_DEBUG, "Failed to recover CBMEM in S3 resume.\n");
		/* Failed S3 resume, reset to come up cleanly */
		hard_reset();
#endif
	}

	/* Save the FSP runtime parameters. */
	fsp_set_runtime(params->chipset_context, hob_list_ptr);

#if IS_ENABLED(CONFIG_DISPLAY_HOBS)
	if (hob_list_ptr == NULL)
		die("ERROR - HOB pointer is NULL!\n");

	/*
	 * Verify that FSP is generating the required HOBs:
	 *	7.1: FSP_BOOTLOADER_TEMP_MEMORY_HOB only produced for FSP 1.0
	 *	7.5: EFI_PEI_GRAPHICS_INFO_HOB produced by SiliconInit
	 */
	cbmem_root = NULL;
	hob_ptr.Raw = get_next_resource_hob(&bootldr_tolum_guid, hob_list_ptr);
	if ((NULL == hob_ptr.Raw)
		&& (fsp_rt_common_buffer.BootLoaderTolumSize != 0)) {
		printk(BIOS_ERR, "7.4: FSP_BOOTLOADER_TOLUM_HOB missing!\n");
		printk(BIOS_ERR, "BootLoaderTolumSize: 0x%08x bytes\n",
			fsp_rt_common_buffer.BootLoaderTolumSize);
		missing_hob = 1;
	} else {
		printk(BIOS_DEBUG,
			"7.4: FSP_BOOTLOADER_TOLUM_HOB: 0x%p\n",
			hob_ptr.Raw);
		data = hob_ptr.ResourceDescriptor->PhysicalStart;
		printk(BIOS_DEBUG, "    0x%016lx: PhysicalStart\n", data);
		data = hob_ptr.ResourceDescriptor->ResourceLength;
		printk(BIOS_DEBUG, "    0x%016lx: ResourceLength\n", data);
		cbmem_root = hob_ptr.ResourceDescriptor;
	}
	hob_ptr.Raw = get_next_guid_hob(&mrc_guid, hob_list_ptr);
	if (NULL == hob_ptr.Raw) {
		printk(BIOS_ERR, "7.3: FSP_NON_VOLATILE_STORAGE_HOB missing!\n");
		missing_hob = 1;
	} else {
		printk(BIOS_DEBUG,
			"7.3: FSP_NON_VOLATILE_STORAGE_HOB: 0x%p\n",
			hob_ptr.Raw);
	}
	printk(BIOS_DEBUG,
		"7.2: FSP_RESERVED_MEMORY_RESOURCE_HOB: 0x%p\n",
		fsp_memory);
	data = fsp_memory->PhysicalStart;
	printk(BIOS_DEBUG, "    0x%016lx: PhysicalStart\n", data);
	data = fsp_memory->ResourceLength;
	printk(BIOS_DEBUG, "    0x%016lx: ResourceLength\n", data);
	fsp_reserved_memory_area = cbmem_find(CBMEM_ID_FSP_RESERVED_MEMORY);
	printk(BIOS_DEBUG, "0x%p: fsp_reserved_memory_area\n",
		fsp_reserved_memory_area);

	/* Display the HOBs */
	print_hob_type_structure(0, hob_list_ptr);

	/* Verify that the FSP memory was properly reserved */
	if ((fsp_memory != NULL) && ((fsp_reserved_memory_area == NULL) ||
		(cbmem_root->PhysicalStart !=
			(unsigned int)fsp_reserved_memory_area)))
		die("ERROR - Reserving FSP memory area!\n");

	/* Verify the order of CBMEM root and FSP memory */
	if ((fsp_memory != NULL) && (cbmem_root != NULL) &&
		(cbmem_root->PhysicalStart <= fsp_memory->PhysicalStart))
		die("ERROR - FSP reserved memory above CBMEM root!\n");

	/* Verify all the HOBs are present */
	if (missing_hob)
		die("ERROR - Missing one or more required FSP HOBs!\n");
#endif

	/* Display the memory configuration */
	report_memory_config();

	/* Locate the memory configuration data to speed up the next reboot */
	mrc_hob = get_next_guid_hob(&mrc_guid, hob_list_ptr);
	if (mrc_hob == NULL)
		printk(BIOS_DEBUG,
			"Memory Configuration Data Hob not present\n");
	else {
		pei_ptr->data_to_save = GET_GUID_HOB_DATA(mrc_hob);
		pei_ptr->data_to_save_size = ALIGN(
			((u32)GET_HOB_LENGTH(mrc_hob)), 16);
	}
}
