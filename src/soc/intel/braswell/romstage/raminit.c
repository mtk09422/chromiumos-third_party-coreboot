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
#include <arch/hlt.h>
#include <console/console.h>
#include <fsp_util.h>
#include <lib.h> /* hexdump */
#include <soc/romstage.h>
#include <string.h>
#include <soc/smm.h>
#include <timestamp.h>

void raminit(struct romstage_params *params, struct pei_data *pei_data)
{
	FSP_INFO_HEADER *fsp_header;
	FSP_MEMORY_INIT fsp_memory_init;
	FSP_MEMORY_INIT_PARAMS fsp_memory_init_params;
	FSP_INIT_RT_COMMON_BUFFER fsp_rt_common_buffer;
	void *hob_list_ptr;
	EFI_STATUS status;
	VPD_DATA_REGION *vpd_ptr;
	UPD_DATA_REGION *upd_ptr;
	UPD_DATA_REGION upd_data_buffer;

	/*
	 * Find and copy the UPD region to the stack so the platform can modify
	 * the settings if needed.  Modifications to the UPD buffer are done in
	 * the platform callback code.  The platform callback code is also
	 * responsible for assigning the UpdDataRngPtr to this buffer if any
	 * updates are made.  The default state is to leave the UpdDataRngPtr
	 * set to NULL.  This indicates that the FSP code will use the UPD
	 * region in the FSP binary.
	 */
	fsp_header = find_fsp();
	vpd_ptr = (VPD_DATA_REGION *)(fsp_header->CfgRegionOffset +
					fsp_header->ImageBase);
	printk(BIOS_DEBUG, "VPD Data: 0x%p\n", vpd_ptr);
	upd_ptr = (UPD_DATA_REGION *)(vpd_ptr->PcdUpdRegionOffset +
					fsp_header->ImageBase);
	printk(BIOS_DEBUG, "UPD Data: 0x%p\n", upd_ptr);
	memcpy(&upd_data_buffer, upd_ptr, sizeof(upd_data_buffer));

	/* Zero fill RT Buffer data and start populating fields. */
	memset(&fsp_rt_common_buffer, sizeof(fsp_rt_common_buffer), 0);
	fsp_rt_common_buffer.BootMode = BOOT_WITH_FULL_CONFIGURATION;
	fsp_rt_common_buffer.UpdDataRgnPtr = &upd_data_buffer;

	/* Get any board specific changes */
	fsp_memory_init_params.NvsBufferPtr = (void *)pei_data->saved_data;
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

	timestamp_add_now(TS_FSP_MEMORY_INIT_START);
	status = fsp_memory_init(&fsp_memory_init_params);
	timestamp_add_now(TS_FSP_MEMORY_INIT_END);

	printk(BIOS_DEBUG, "FspMemoryInit returned 0x%08x\n", status);
	if (status != EFI_SUCCESS)
		die("ERROR - FspMemoryInit failed to initialize memory!\n");
}
