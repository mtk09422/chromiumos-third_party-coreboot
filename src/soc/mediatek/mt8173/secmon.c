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

#include <arch/secmon.h>
#include <console/console.h>
#include <soc/addressmap.h>
#include <soc/mmu_operations.h>

void soc_get_secmon_base_size(uint64_t *base, size_t *size)
{
	uintptr_t tz_base_mib;
	size_t tz_size_mib;

	carveout_range(CARVEOUT_TZ, &tz_base_mib, &tz_size_mib);

	*base = tz_base_mib * MiB;
	*size = tz_size_mib * MiB;
}
