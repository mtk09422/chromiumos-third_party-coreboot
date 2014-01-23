/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 Google Inc.
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
#include <soc/nvidia/tegra124/sdram.h>
#include "sdram_configs.h"

/*
 * Note for board bring up, we've temporarily filled SDRAM table with
 * 0001-204-2GB configuration (except the 0100-204-4GB entry).
 */
static struct sdram_params sdram_configs[] = {
#include "bct/sdram-0001-204-2GB.inc"
#include "bct/sdram-0001-204-2GB.inc"
#include "bct/sdram-0001-204-2GB.inc"
#include "bct/sdram-0001-204-2GB.inc"
#include "bct/sdram-0100-204-4GB.inc"
#include "bct/sdram-0001-204-2GB.inc"
#include "bct/sdram-0001-204-2GB.inc"
#include "bct/sdram-0001-204-2GB.inc"
#include "bct/sdram-0001-204-2GB.inc"
#include "bct/sdram-0001-204-2GB.inc"
#include "bct/sdram-0001-204-2GB.inc"
#include "bct/sdram-0001-204-2GB.inc"
#include "bct/sdram-0001-204-2GB.inc"
#include "bct/sdram-0001-204-2GB.inc"
#include "bct/sdram-0001-204-2GB.inc"
#include "bct/sdram-0001-204-2GB.inc"
};

const struct sdram_params *get_sdram_config()
{
	uint32_t ramcode = sdram_get_ram_code();
	/*
	 * If we need to apply some special hacks to RAMCODE mapping (ex, by
	 * board_id), do that now.
	 */

	printk(BIOS_SPEW, "%s: RAMCODE=%d\n", __func__, ramcode);
	if (ramcode >= sizeof(sdram_configs) / sizeof(sdram_configs[0]) ||
	    sdram_configs[ramcode].AhbArbitrationXbarCtrlMemInitDone == 0) {
		die("Invalid RAMCODE.");
	}

	return &sdram_configs[ramcode];
}
