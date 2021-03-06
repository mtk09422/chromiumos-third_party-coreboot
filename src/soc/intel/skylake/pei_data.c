/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2014 Google Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <console/console.h>
#include <stdlib.h>
#include <stdint.h>
#include <soc/iomap.h>
#include <soc/pei_data.h>
#include <soc/pei_wrapper.h>
#include <soc/smm.h>

static void ABI_X86 send_to_console(unsigned char b)
{
	console_tx_byte(b);
}

void skylake_fill_pei_data(struct pei_data *pei_data)
{
	pei_data->pei_version = PEI_VERSION;
	pei_data->board_type = BOARD_TYPE_ULT;
	pei_data->pciexbar = MCFG_BASE_ADDRESS;
	pei_data->smbusbar = SMBUS_BASE_ADDRESS;
	pei_data->xhcibar = EARLY_XHCI_BAR;
	pei_data->gttbar = EARLY_GTT_BAR;
	pei_data->pmbase = ACPI_BASE_ADDRESS;
	pei_data->gpiobase = GPIO_BASE_ADDRESS;
	pei_data->tseg_size = smm_region_size();
	pei_data->temp_mmio_base = EARLY_TEMP_MMIO;
	pei_data->tx_byte = &send_to_console;
	pei_data->ddr_refresh_2x = 1;
}
