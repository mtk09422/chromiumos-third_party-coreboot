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

#include "chip.h"
#include <console/console.h>
#include <device/device.h>
#include <device/pci.h>
#include <fsp_util.h>
#include <soc/pci_devs.h>
#include <soc/ramstage.h>

static void pci_domain_set_resources(device_t dev)
{
	printk(BIOS_SPEW, "%s/%s ( %s )\n",
			__FILE__, __func__, dev_name(dev));
	assign_resources(dev->link_list);
}

static struct device_operations pci_domain_ops = {
	.read_resources   = pci_domain_read_resources,
	.set_resources    = pci_domain_set_resources,
	.enable_resources = NULL,
	.init             = NULL,
	.scan_bus         = pci_domain_scan_bus,
	.ops_pci_bus      = &pci_ops_mmconf,
};

static void cpu_bus_noop(device_t dev) { }

static struct device_operations cpu_bus_ops = {
	.read_resources   = cpu_bus_noop,
	.set_resources    = cpu_bus_noop,
	.enable_resources = cpu_bus_noop,
	.init             = braswell_init_cpus
};


static void enable_dev(device_t dev)
{
	printk(BIOS_SPEW, "----------\n%s/%s ( %s ), type: %d\n",
			__FILE__, __func__,
			dev_name(dev), dev->path.type);
	printk(BIOS_SPEW, "vendor: 0x%04x. device: 0x%04x\n",
			pci_read_config16(dev, PCI_VENDOR_ID),
			pci_read_config16(dev, PCI_DEVICE_ID));
	printk(BIOS_SPEW, "class: 0x%02x %s\n"
			"subclass: 0x%02x %s\n"
			"prog: 0x%02x\n"
			"revision: 0x%02x\n",
			pci_read_config16(dev, PCI_CLASS_DEVICE) >> 8,
			get_pci_class_name(dev),
			pci_read_config16(dev, PCI_CLASS_DEVICE) & 0xff,
			get_pci_subclass_name(dev),
			pci_read_config8(dev, PCI_CLASS_PROG),
			pci_read_config8(dev, PCI_REVISION_ID));

	/* Set the operations if it is a special bus type */
	if (dev->path.type == DEVICE_PATH_DOMAIN) {
		dev->ops = &pci_domain_ops;
	} else if (dev->path.type == DEVICE_PATH_CPU_CLUSTER) {
		dev->ops = &cpu_bus_ops;
	} else if (dev->path.type == DEVICE_PATH_PCI) {
		/* Handle south cluster enablement. */
		if (PCI_SLOT(dev->path.pci.devfn) > GFX_DEV &&
		    (dev->ops == NULL || dev->ops->enable == NULL)) {
			southcluster_enable_dev(dev);
		}
	}
}

/* Called at BS_DEV_INIT_CHIPS time -- very early. Just after BS_PRE_DEVICE. */
static void soc_init(void *chip_info)
{
	printk(BIOS_SPEW, "%s/%s\n", __FILE__, __func__);
	braswell_init_pre_device(chip_info);
}

struct chip_operations soc_intel_braswell_ops = {
	CHIP_NAME("Intel Braswell SoC")
	.enable_dev = enable_dev,
	.init = soc_init,
};

static void pci_set_subsystem(device_t dev, unsigned vendor, unsigned device)
{
	printk(BIOS_SPEW, "%s/%s ( %s, 0x%04x, 0x%04x )\n",
			__FILE__, __func__, dev_name(dev), vendor, device);
	if (!vendor || !device) {
		pci_write_config32(dev, PCI_SUBSYSTEM_VENDOR_ID,
				pci_read_config32(dev, PCI_VENDOR_ID));
	} else {
		pci_write_config32(dev, PCI_SUBSYSTEM_VENDOR_ID,
				((device & 0xffff) << 16) | (vendor & 0xffff));
	}
}

struct pci_operations soc_pci_ops = {
	.set_subsystem = &pci_set_subsystem,
};
