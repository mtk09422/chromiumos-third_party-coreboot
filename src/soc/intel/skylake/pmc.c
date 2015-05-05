/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2008-2009 coresystems GmbH
 * Copyright (C) 2014 Google Inc.
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
 */

#include <console/console.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <arch/io.h>
#include <arch/ioapic.h>
#include <arch/acpi.h>
#include <cpu/cpu.h>
#include <reg_script.h>
#include <string.h>
#include <soc/iomap.h>
#include <soc/pci_devs.h>
#include <soc/pmc.h>
#include <soc/pm.h>
#include <cpu/x86/smm.h>
#include <soc/pcr.h>
#include <soc/ramstage.h>
#include <soc/intel/skylake/chip.h>

static const struct reg_script pch_pmc_misc_init_script[] = {
	/* Setup SLP signal assertion, SLP_S4=4s, SLP_S3=50ms */
	REG_PCI_RMW16(GEN_PMCON_B_1, ~((3 << 4)|(1 << 10)),
			(1 << 3)|(1 << 11)|(1 << 12)),
	REG_IO_RMW32(ACPI_BASE_ADDRESS + PM1_CNT, ~SLP_TYP, SCI_EN),
	/* Indicate DRAM init done for MRC */
	REG_PCI_OR8(GEN_PMCON_A_2, (1 << 7)),
	REG_SCRIPT_END
};

static void pch_pmc_add_mmio_resources(device_t dev)
{
	struct resource *res;
	const u32 default_decode_base = PCH_PCR_BASE_ADDRESS;

	/*
	 * Till PCI enumeration happens we need to allocate the PMC base
	 * statically. Above the PCR base.
	 */
	if (PCH_PWRM_BASE_ADDRESS < default_decode_base) {
		res = new_resource(dev, PWRM);
		res->base = PCH_PWRM_BASE_ADDRESS;
		/* 64KB PMC size */
		res->size = 0x10000;
		res->flags = IORESOURCE_MEM | IORESOURCE_ASSIGNED |
				IORESOURCE_FIXED | IORESOURCE_RESERVE;
	}
}

static void pch_pmc_add_io_resource(device_t dev, u16 base, u16 size, int index)
{
	struct resource *res;
	res = new_resource(dev, index);
	res->base = base;
	res->size = size;
	res->flags = IORESOURCE_IO | IORESOURCE_ASSIGNED | IORESOURCE_FIXED;
}

static void pch_pmc_add_io_resources(device_t dev)
{
	/* PMBASE */
	pch_pmc_add_io_resource(dev, ACPI_BASE_ADDRESS, ACPI_BASE_SIZE, PMBASE);
}

static void pch_pmc_read_resources(device_t dev)
{
	/* Get the normal PCI resources of this device. */
	pci_dev_read_resources(dev);

	/* Add non-standard MMIO resources. */
	pch_pmc_add_mmio_resources(dev);

	/* Add IO resources. */
	pch_pmc_add_io_resources(dev);
}

static void pch_set_acpi_mode(void)
{
	if (IS_ENABLED(CONFIG_HAVE_SMI_HANDLER) && acpi_slp_type != 3) {
		printk(BIOS_DEBUG, "Disabling ACPI via APMC:\n");
		outb(APM_CNT_ACPI_DISABLE, APM_CNT);
		printk(BIOS_DEBUG, "done.\n");
	}
}

static void pmc_init(struct device *dev)
{
	reg_script_run_on_dev(dev, pch_pmc_misc_init_script);
	pch_set_acpi_mode();
}

static struct device_operations device_ops = {
	.read_resources		= &pch_pmc_read_resources,
	.set_resources		= &pci_dev_set_resources,
	.enable_resources	= &pci_dev_enable_resources,
	.init			= &pmc_init,
	.scan_bus		= &scan_static_bus,
	.ops_pci		= &skylake_pci_ops,
};

static const unsigned short pci_device_ids[] = {
	0x9d21,
	0
};

static const struct pci_driver pch_lpc __pci_driver = {
	.ops	 = &device_ops,
	.vendor	 = PCI_VENDOR_ID_INTEL,
	.devices = pci_device_ids,
};
