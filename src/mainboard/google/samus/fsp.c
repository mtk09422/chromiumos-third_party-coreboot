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

#include <arch/early_variables.h>
#include <console/console.h>
#include <lib.h> // hexdump
#include <mainboard/google/samus/spd/spd.h>
#include <soc/iomap.h>
#include <soc/romstage.h>

void board_fsp_memory_init_params(
	struct romstage_params *params,
	FSP_INFO_HEADER *fsp_header,
	FSP_MEMORY_INIT_PARAMS *fsp_memory_init_params)
{
	FSP_INIT_RT_COMMON_BUFFER *rt_buffer;
	UPD_DATA_REGION *upd_data;

	/* Set the memory configuration */
	rt_buffer = fsp_memory_init_params->RtBufferPtr;
	upd_data = rt_buffer->UpdDataRgnPtr;
	if (params->pei_data->spd_data[0][0][0] != 0) {
		upd_data->SpdDataBuffer_0_0 =
			(UINT32)(&params->pei_data->spd_data[0][0]);
		upd_data->SpdDataBuffer_1_0 =
			(UINT32)(&params->pei_data->spd_data[1][0]);
		printk(BIOS_SPEW, "0x%08x: SpdDataBuffer_0_0\n",
			upd_data->SpdDataBuffer_0_0);
		printk(BIOS_SPEW, "0x%08x: SpdDataBuffer_0_1\n",
			upd_data->SpdDataBuffer_0_1);
		printk(BIOS_SPEW, "0x%08x: SpdDataBuffer_1_0\n",
			upd_data->SpdDataBuffer_0_0);
		printk(BIOS_SPEW, "0x%08x: SpdDataBuffer_1_1\n",
			upd_data->SpdDataBuffer_0_1);
	}

	/* Enable/disable the devices */
	upd_data->PcdEnableLan = 0;
}

