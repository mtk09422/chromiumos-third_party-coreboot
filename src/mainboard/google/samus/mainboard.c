/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2009 coresystems GmbH
 * Copyright (C) 2013 Google Inc.
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

#include <types.h>
#include <string.h>
#include <smbios.h>
#include <device/device.h>
#include <device/device.h>
#include <device/pci_def.h>
#include <device/pci_ops.h>
#include <console/console.h>
#if CONFIG_PCI_ROM_RUN || CONFIG_VGA_ROM_RUN
#include <x86emu/x86emu.h>
#endif
#include <pc80/mc146818rtc.h>
#include <arch/acpi.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <boot/coreboot_tables.h>
#include <ec/google/chromeec/ec.h>
#include "ec.h"

#if CONFIG_PCI_ROM_RUN || CONFIG_VGA_ROM_RUN
static int int15_handler(void)
{
	int res = 0;

	printk(BIOS_DEBUG, "%s: AX=%04x BX=%04x CX=%04x DX=%04x\n",
	       __func__, X86_AX, X86_BX, X86_CX, X86_DX);

	switch (X86_AX) {
	case 0x5f35:
		/*
		 * Boot Display Device Hook:
		 *  bit 0 = CRT
		 *  bit 1 = RESERVED
		 *  bit 2 = EFP
		 *  bit 3 = LFP
		 *  bit 4 = RESERVED
		 *  bit 5 = EFP3
		 *  bit 6 = EFP2
		 *  bit 7 = RESERVED
		 */
		X86_AX = 0x005f;
		X86_CX = 0x0008;
		res = 1;
		break;
        default:
		printk(BIOS_DEBUG, "Unknown INT15 function %04x!\n", X86_AX);
		break;
	}
	return res;
}
#endif

void mainboard_suspend_resume(void)
{
}

const char *smbios_mainboard_version(void)
{
	switch (google_chromeec_get_board_version()) {
	case SAMUS_EC_BOARD_VERSION_EVT:
		return "EVT";
	case SAMUS_EC_BOARD_VERSION_EVT2:
		return "EVT2";
	case SAMUS_EC_BOARD_VERSION_EVT3:
		return "EVT3";
	}
	return "Unknown";
}

static void mainboard_init(device_t dev)
{
	mainboard_ec_init();
}

// mainboard_enable is executed as first thing after
// enumerate_buses().

static void mainboard_enable(device_t dev)
{
	dev->ops->init = mainboard_init;
#if CONFIG_PCI_ROM_RUN || CONFIG_VGA_ROM_RUN
	/* Install custom int15 handler for VGA OPROM */
	mainboard_interrupt_handlers(0x15, &int15_handler);
#endif
}

struct chip_operations mainboard_ops = {
	.enable_dev = mainboard_enable,
};
