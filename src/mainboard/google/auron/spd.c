/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2014 Google Inc.
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

#include <arch/byteorder.h>
#include <cbfs.h>
#include <console/console.h>
#include <string.h>
#include <broadwell/gpio.h>
#include <broadwell/pei_data.h>
#include <broadwell/romstage.h>
#include "gpio.h"
#include "spd.h"

/* Copy SPD data for on-board memory */
void mainboard_fill_spd_data(struct pei_data *pei_data)
{
	int spd_gpio[3];
	int spd_index;
	int spd_file_len;
	struct cbfs_file *spd_file;

	spd_gpio[0] = get_gpio(SPD_GPIO_BIT0);
	spd_gpio[1] = get_gpio(SPD_GPIO_BIT1);
	spd_gpio[2] = get_gpio(SPD_GPIO_BIT2);

	spd_index = spd_gpio[2] << 2 | spd_gpio[1] << 1 | spd_gpio[0];

	printk(BIOS_DEBUG, "SPD: index %d (GPIO%d=%d GPIO%d=%d GPIO%d=%d)\n",
	       spd_index,
	       SPD_GPIO_BIT2, spd_gpio[2],
	       SPD_GPIO_BIT1, spd_gpio[1],
	       SPD_GPIO_BIT0, spd_gpio[0]);

	spd_file = cbfs_get_file(CBFS_DEFAULT_MEDIA, "spd.bin");
	if (!spd_file)
		die("SPD data not found.");
	spd_file_len = ntohl(spd_file->len);

	if (spd_index > 3)
		pei_data->dimm_channel1_disabled = 3;

	if (spd_file_len < ((spd_index + 1) * SPD_LEN)) {
		printk(BIOS_ERR, "SPD index override to 0 - old hardware?\n");
		spd_index = 0;
	}

	if (spd_file_len < SPD_LEN)
		die("Missing SPD data.");

	spd_index *= SPD_LEN;

	memcpy(pei_data->spd_data[0][0],
		((char*)CBFS_SUBHEADER(spd_file)) + spd_index, SPD_LEN);
	/* Index 0-2 are 4GB config with both CH0 and CH1.
	 * Index 4-6 are 2GB config with CH0 only. */
	if (spd_index > 3)
		pei_data->dimm_channel1_disabled = 3;
	else
		memcpy(pei_data->spd_data[1][0],
			((char*)CBFS_SUBHEADER(spd_file)) + spd_index, SPD_LEN);
}

