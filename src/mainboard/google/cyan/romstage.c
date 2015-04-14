/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2013 Google Inc.
 * Copyright (C) 2015 Intel Corp.
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

#include <cbfs.h>
#include <console/console.h>
#include <fsp_util.h>
#include <lib.h>
#include <soc/gpio.h>
#include <soc/pci_devs.h>
#include <soc/romstage.h>
#include <string.h>

/* All FSP specific code goes in this block */
void mainboard_romstage_entry(struct romstage_params *rp)
{
#if IS_ENABLED(CONFIG_GOP_SUPPORT)
	void *vbt_content;
	//struct cbfs_file *vbt_file;
	uint32_t vbt_len;
#endif
	struct pei_data *ps = rp->pei_data;

	mainboard_fill_spd_data(ps);

#if IS_ENABLED(CONFIG_GOP_SUPPORT)
	/* Get VBT data */
	vbt_content = (void *)fsp_get_vbt(&vbt_len);
	if (vbt_content != NULL)
		ps->vbt_data = vbt_content;
#endif

	/* Set device state/enable information */
	ps->sdcard_mode = PCH_ACPI_MODE;
	ps->emmc_mode = PCH_ACPI_MODE;
	ps->enable_azalia = 1;

	/* Call back into chipset code with platform values updated. */
	romstage_common(rp);
}
