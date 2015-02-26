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

#include <arch/io.h>
#include <console/console.h>
#include <stdlib.h>
#include <symbols.h>
#include <soc/addressmap.h>

int sdram_size_mb(void);

static uintptr_t tz_base_mib;
static const size_t tz_size_mib = CONFIG_TRUSTZONE_CARVEOUT_SIZE_MB;

/* returns total amount of DRAM (in MB) from memory controller registers */
int sdram_size_mb(void)
{
	int total_size = CONFIG_DRAM_SIZE_MB;
	return total_size;
}

void carveout_range(int id, uintptr_t *base_mib, size_t *size_mib)
{
	*base_mib = 0;
	*size_mib = 0;

	switch (id) {
	case CARVEOUT_TZ:
		*base_mib = tz_base_mib;
		*size_mib = tz_size_mib;
		break;
	default:
		break;
	}
}

static void memory_in_range(uintptr_t *base_mib, uintptr_t *end_mib,
			    int ignore_tz)
{
	uintptr_t base;
	uintptr_t end;
	int i;

	base = (uintptr_t)_dram / MiB;
	end = base + sdram_size_mb();

	/* Requested limits out of range. */
	if (*end_mib <= base || *base_mib >= end) {
		printk(BIOS_ERR, "%s: requested limits (%#lx ~ %#lx) out of range (%#lx ~ %#lx)\n",
		       __func__,
		       (unsigned long)*base_mib, (unsigned long)*end_mib,
		       (unsigned long)base, (unsigned long)end);
		*end_mib = *base_mib = 0;
		return;
	}

	/* Clip region to passed in limits. */
	if (*end_mib < end)
		end = *end_mib;
	if (*base_mib > base)
		base = *base_mib;

	for (i = 0; i < CARVEOUT_NUM; i++) {
		uintptr_t carveout_base;
		uintptr_t carveout_end;
		size_t carveout_size;

		if (i == CARVEOUT_TZ && ignore_tz)
			continue;

		carveout_range(i, &carveout_base, &carveout_size);

		if (carveout_size == 0)
			continue;

		carveout_end = carveout_base + carveout_size;

		/* Bypass carveouts out of requested range. */
		if (carveout_base >= end || carveout_end <= base)
			continue;

		/*
		 * This is crude, but the assumption is that carveouts live
		 * at the upper range of physical memory. Therefore, update
		 * the end address to be equal to the base of the carveout.
		 */
		end = carveout_base;
	}
	*base_mib = base;
	*end_mib = end;
}

void memory_in_range_below_4gb(uintptr_t *base_mib, uintptr_t *end_mib)
{
	*base_mib = 0;
	*end_mib = 4096;
	memory_in_range(base_mib, end_mib, 0);
}

void memory_in_range_above_4gb(uintptr_t *base_mib, uintptr_t *end_mib)
{
	*base_mib = 4096;
	*end_mib = ~0UL;
	memory_in_range(base_mib, end_mib, 0);
}

void trustzone_region_init(void)
{
	uintptr_t end = 3072;

	if (((uintptr_t)_dram / MiB) >= 0x1000) {
		tz_base_mib += 0x1000;
		end += 0x1000;
	}

	/*
	 * Get memory layout ignoring the TZ carveout because
	 * that's the one to initialize.
	 */
	memory_in_range(&tz_base_mib, &end, 1);
	tz_base_mib = end - tz_size_mib;

	if (end <= 0)
		printk(BIOS_DEBUG, "trustzone init fail !\n");

	printk(BIOS_DEBUG, "%s tz: %#lx, end: %#lx\n",
		__func__, (unsigned long)tz_base_mib, (unsigned long)end);
}
