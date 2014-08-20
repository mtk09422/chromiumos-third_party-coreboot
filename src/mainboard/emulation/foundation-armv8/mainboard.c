/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2013 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <console/console.h>
#include <device/device.h>
#include <symbols.h>

static void mainboard_enable(device_t dev)
{
	printk(BIOS_INFO, "Enable foundation/armv8 device...\n");

	ram_resource(dev, 0, (uintptr_t)_dram / KiB,
	             (uintptr_t)_dram / KiB + CONFIG_DRAM_SIZE_MB * KiB);
}

struct chip_operations mainboard_ops = {
	CHIP_NAME("Foundation ARMv8 Model")
	.enable_dev = mainboard_enable,
};

