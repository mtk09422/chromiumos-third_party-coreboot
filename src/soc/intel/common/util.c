/*
 * This file is part of the coreboot project.
 *
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
#include <soc/intel/common/util.h>

void soc_display_upd_value(const char *name, uint32_t size, uint64_t old,
	uint64_t new)
{
	if (old == new) {
		switch (size) {
		case 1:
			printk(BIOS_SPEW, "  0x%02llx: %s\n", new, name);
			break;

		case 2:
			printk(BIOS_SPEW, "  0x%04llx: %s\n", new, name);
			break;

		case 4:
			printk(BIOS_SPEW, "  0x%08llx: %s\n", new, name);
			break;

		case 8:
			printk(BIOS_SPEW, "  0x%016llx: %s\n", new, name);
			break;
		}
	} else {
		switch (size) {
		case 1:
			printk(BIOS_SPEW, "  0x%02llx --> 0x%02llx: %s\n", old,
				new, name);
			break;

		case 2:
			printk(BIOS_SPEW, "  0x%04llx --> 0x%04llx: %s\n", old,
				new, name);
			break;

		case 4:
			printk(BIOS_SPEW, "  0x%08llx --> 0x%08llx: %s\n", old,
				new, name);
			break;

		case 8:
			printk(BIOS_SPEW, "  0x%016llx --> 0x%016llx: %s\n",
				old, new, name);
			break;
		}
	}
}
