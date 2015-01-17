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

#include <string.h>
#include <cbfs.h>
#include <console/console.h>
#include <soc/romstage.h>
#include <lib.h>
#include <soc/pci_devs.h>

/*
 * Only a single memory configuration for strago so no need to check any
 * GPIO values.
 * Index   Size         Configuration
 * 0x00  - 2GiB total - 2 x 1GiB Hynix  H5TC2G63FFR-PBA 1600MHz
 */
static const uint32_t dual_channel_config =
	(1 << 0);

#define SPD_SIZE 256

static void *get_spd_pointer(char *spd_file_content, int total_spds, int *dual)
{
	int ram_id = 0;

	/* TODO: Add memory configuration detection if needed. */
	printk(BIOS_DEBUG, "ram_id=%d, total_spds: %d\n", ram_id, total_spds);

	if (ram_id >= total_spds)
		return NULL;

	/* Single channel configs */
	if (dual_channel_config & (1 << ram_id))
		*dual = 1;

	return &spd_file_content[SPD_SIZE * ram_id];
}

/* All FSP specific code goes in this block */
void mainboard_romstage_entry(struct romstage_params *rp)
{
	struct cbfs_file *spd_file;
	void *spd_content;
	int dual_channel = 0;
	struct pei_data ps;

	/* Find the SPD data in CBFS. */
	spd_file = cbfs_get_file(CBFS_DEFAULT_MEDIA, "spd.bin");
	if (!spd_file)
		die("SPD data not found.");

	/*
	 * Both channels are always present in SPD data. Always use matched
	 * DIMMs so use the same SPD data for each DIMM.
	 */
	spd_content = get_spd_pointer(CBFS_SUBHEADER(spd_file),
				      ntohl(spd_file->len) / SPD_SIZE,
				      &dual_channel);
	if (spd_content != NULL) {
		printk(BIOS_DEBUG, "SPD Data:\n");
		hexdump(spd_content, SPD_SIZE);
		printk(BIOS_DEBUG, "\n");
	}

	/* Initialize the pei data structure */
	memset(&ps, 0, sizeof(ps));

	/*
	 * Set SPD and memory configuration:
	 * Memory type: 0=DimmInstalled,
	 *              1=SolderDownMemory,
	 *              2=DimmDisabled
	 */
	if (spd_content != NULL) {
		ps.spd_data_ch0 = spd_content;
		ps.spd_ch0_config = 1;
		ps.spd_ch1_config = 2;
	}

	/* Set device state/enable information */
	ps.sdcard_mode = PCH_ACPI_MODE;
	ps.emmc_mode = PCH_ACPI_MODE;
	ps.enable_azalia = 1;

	/* Call back into chipset code with platform values updated. */
	rp->pei_data = &ps;
	romstage_common(rp);
}
