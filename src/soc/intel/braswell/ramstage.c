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

#include <soc/ramstage.h>

#include <cpu/x86/mtrr.h>
long x86_mtrr_rom_cache_var_index(void)
{
	return 0;
}

#include <vendorcode/google/chromeos/chromeos.h>
chromeos_acpi_t *vboot_data;
int get_recovery_mode_from_vbnv(void)
{
	return 0;
}
int vboot_wants_oprom(void)
{
	return 0;
}
#include <cbfs.h>
void *cbfs_load_payload(struct cbfs_media *media, const char *name)
{
	return NULL;
}

void braswell_init_pre_device(struct soc_intel_braswell_config *config)
{
}
