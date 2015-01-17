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
#include <lib.h> /* hexdump */
#include <mainboard/google/samus/spd/spd.h>
#include <soc/romstage.h>

void board_fsp_memory_init_params(
	struct romstage_params *params,
	FSP_INFO_HEADER *fsp_header,
	FSP_MEMORY_INIT_PARAMS *fsp_memory_init_params)
{
	FSP_INIT_RT_COMMON_BUFFER *rt_buffer;
	UPD_DATA_REGION *upd_ptr;

	/* Initialize pointers to UPD and RT buffers */
	rt_buffer = fsp_memory_init_params->RtBufferPtr;
	upd_ptr = rt_buffer->UpdDataRgnPtr;

	/* Update SPD and memory configuration data */
	upd_ptr->PcdMemorySpdPtr = (u32)params->pei_data->spd_data_ch0;
	upd_ptr->PcdMemChannel0Config = params->pei_data->spd_ch0_config;
	upd_ptr->PcdMemChannel1Config = params->pei_data->spd_ch1_config;

	/* Set the I/O map */
	upd_ptr->PcdMrcInitTsegSize = 8; /* Use 8MB by default */

	/* Enable/disable the devices */
	upd_ptr->PcdSdcardMode = params->pei_data->sdcard_mode;
	upd_ptr->PcdEmmcMode = params->pei_data->emmc_mode;
	upd_ptr->PcdEnableAzalia = params->pei_data->enable_azalia;
}

