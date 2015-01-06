/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2014 Intel Corporation
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
#include <lib.h> // hexdump
#include <soc/pei_data.h>
#include <soc/reset.h>
#include <soc/romstage.h>
#include <string.h>

void raminit(struct romstage_params *params, struct pei_data *pei_data)
{
	FSP_INFO_HEADER *fsp_header;
	FSP_MEMORY_INIT fsp_memory_init;
	FSP_MEMORY_INIT_PARAMS fsp_memory_init_params;
	FSP_INIT_RT_COMMON_BUFFER fsp_rt_common_buffer;
	void *hob_list_ptr;
	EFI_STATUS status;
	VPD_DATA_REGION *vpd_data;
	UPD_DATA_REGION *upd_data;
	UPD_DATA_REGION upd_data_buffer;

	/* Find and copy the UPD region to the stack so the platform can modify
	 * the settings if needed.  Modifications to the UPD buffer are done in
	 * the platform callback code.  The platform callback code is also
	 * responsible for assigning the UpdDataRngPtr to this buffer if any
	 * updates are made.  The default state is to leave the UpdDataRngPtr
	 * set to NULL.  This indicates that the FSP code will use the UPD
	 * region in the FSP binary.*/
	fsp_header = find_fsp();
	vpd_data = (VPD_DATA_REGION *)(fsp_header->CfgRegionOffset +
					fsp_header->ImageBase);
	printk(BIOS_DEBUG, "VPD Data: 0x%p\n", vpd_data);
	upd_data = (UPD_DATA_REGION *)(vpd_data->PcdUpdRegionOffset +
					fsp_header->ImageBase);
	printk(BIOS_DEBUG, "UPD Data: 0x%p\n", upd_data);
	memcpy(&upd_data_buffer, upd_data, sizeof(upd_data_buffer));

	/* Zero fill RT Buffer data and start populating fields. */
	memset(&fsp_rt_common_buffer, sizeof(fsp_rt_common_buffer), 0);
	fsp_rt_common_buffer.BootMode = pei_data->boot_mode;
	fsp_rt_common_buffer.UpdDataRgnPtr = &upd_data_buffer;

	/* Get any board specific changes */
	fsp_memory_init_params.NvsBufferPtr = NULL;
	fsp_memory_init_params.RtBufferPtr = &fsp_rt_common_buffer;
	fsp_memory_init_params.HobListPtr = &hob_list_ptr;
	board_fsp_memory_init_params(params, fsp_header,
		&fsp_memory_init_params);

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
	status = fsp_memory_init(&fsp_memory_init_params);
	printk(BIOS_DEBUG, "FspMemoryInit returned 0x%08x\n", status);
	if (status != EFI_SUCCESS)
		die("ERROR - FspMemoryInit failed to initialize memory!\n");

	/* Display the memory configuration */
	report_memory_config();

	if (pei_data->boot_mode != SLEEP_STATE_S3) {
		cbmem_initialize_empty();
	} else if (cbmem_initialize()) {
#if CONFIG_HAVE_ACPI_RESUME
		printk(BIOS_DEBUG, "Failed to recover CBMEM in S3 resume.\n");
		/* Failed S3 resume, reset to come up cleanly */
		reset_system();
#endif
	}

	/* Save the HOB list */
	set_hob_list(hob_list_ptr);

	/* Display the HOBs */
#if IS_ENABLED(CONFIG_DISPLAY_HOBS)
	if (hob_list_ptr == NULL)
		die("ERROR - HOB pointer is NULL!\n");
	print_hob_type_structure(0, hob_list_ptr);
#endif
}
