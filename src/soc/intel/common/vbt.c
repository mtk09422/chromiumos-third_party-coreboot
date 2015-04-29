/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2015 Google Inc.
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
#include <soc/romstage.h>
#include <string.h>

/* Locate VBT and pass it to FSP GOP */
void load_vbt(struct romstage_params *rp)
{
	void *vbt_content;
	uint32_t vbt_len;
	struct pei_data *ps = rp->pei_data;

	/* Check boot mode - for S3 resume path VBT loading is not needed */
	if (rp->power_state->prev_sleep_state != SLEEP_STATE_S3) {
		/* Get VBT data */
		vbt_content = (void *)fsp_get_vbt(&vbt_len);
		if (vbt_content != NULL) {
			ps->vbt_data = vbt_content;
			printk(BIOS_DEBUG, "Find and pass VBT to GOP\n");
		}
	} else {
		ps->vbt_data = NULL;
		printk(BIOS_DEBUG, "S3 resume do not pass VBT to GOP\n");
	}
}
