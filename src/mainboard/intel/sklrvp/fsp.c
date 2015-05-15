/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2014-2015 Intel Corporation.
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
#include <soc/romstage.h>
#include <string.h>
#include <fsp_util.h>
#include "mainboard/intel/sklrvp/spd/spd.h"
#include "gpio_rvp3.h"

void mainboard_memory_init_params(struct romstage_params *params,
	UPD_DATA_REGION *upd_ptr)
{
	struct pei_data *pei_data_ptr;

	/* Find the PEI DATA information that is filled out by the platform.*/
	pei_data_ptr = params->pei_data;

	/* Get SPD data passing strucutre and initialize it.*/
	if (params->pei_data->spd_data[0][0][0] != 0) {
		upd_ptr->MemorySpdPtr00 =
				(UINT32)(pei_data_ptr->spd_data[0][0]);
		upd_ptr->MemorySpdPtr10 =
				(UINT32)(pei_data_ptr->spd_data[1][0]);
		printk(BIOS_SPEW, "0x%08x: SpdDataBuffer_0_0\n",
				upd_ptr->MemorySpdPtr00);
		printk(BIOS_SPEW, "0x%08x: SpdDataBuffer_0_1\n",
				upd_ptr->MemorySpdPtr01);
		printk(BIOS_SPEW, "0x%08x: SpdDataBuffer_1_0\n",
				upd_ptr->MemorySpdPtr10);
		printk(BIOS_SPEW, "0x%08x: SpdDataBuffer_1_1\n",
				upd_ptr->MemorySpdPtr11);
	}
	/*
	* Configure the DQ/DQS settings if required. In general the settings
	* should be set in the FSP flash image and should not need to be
	* changed.
	*/
	memcpy(upd_ptr->DqByteMapCh0, pei_data_ptr->dq_map[0],
			sizeof(pei_data_ptr->dq_map[0]));
	memcpy(upd_ptr->DqByteMapCh1, pei_data_ptr->dq_map[1],
			sizeof(pei_data_ptr->dq_map[1]));
	memcpy(upd_ptr->DqsMapCpu2DramCh0, pei_data_ptr->dqs_map[0],
			sizeof(pei_data_ptr->dqs_map[0]));
	memcpy(upd_ptr->DqsMapCpu2DramCh1, pei_data_ptr->dqs_map[1],
			sizeof(pei_data_ptr->dqs_map[1]));
	memcpy(upd_ptr->RcompResistor, pei_data_ptr->RcompResistorSkl,
			sizeof(pei_data_ptr->RcompResistorSkl));
	memcpy(upd_ptr->RcompTarget, pei_data_ptr->RcompTargetSkl,
			sizeof(pei_data_ptr->RcompTargetSkl));

	/* update spd length*/
	upd_ptr->MemorySpdDataLen = SPD_LEN;
	upd_ptr->DqPinsInterleaved = FALSE;

	/*update gpio table*/
	upd_ptr->GpioTablePtr = (UINT32 *)GpioTableRvp3;
}
