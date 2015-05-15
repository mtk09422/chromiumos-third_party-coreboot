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

#include <chip.h>
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

void soc_silicon_init_params(UPD_DATA_REGION *upd_ptr)
{
	device_t dev = dev_find_slot(0, PCI_DEVFN(LPC_DEV, LPC_FUNC));
	struct soc_intel_braswell_config *config = dev->chip_info;

	/* Set the parameters for SiliconInit */
	printk(BIOS_DEBUG, "Updating UPD values for SiliconInit\n");
	upd_ptr->PcdSdcardMode = config->PcdSdcardMode;
	upd_ptr->PcdEnableHsuart0 = config->PcdEnableHsuart0;
	upd_ptr->PcdEnableHsuart1 = config->PcdEnableHsuart1;
	upd_ptr->PcdEnableAzalia = config->PcdEnableAzalia;
	upd_ptr->AzaliaConfigPtr = config->AzaliaConfigPtr;
	upd_ptr->PcdEnableSata = config->PcdEnableSata;
	upd_ptr->PcdEnableXhci = config->PcdEnableXhci;
	upd_ptr->PcdEnableLpe = config->PcdEnableLpe;
	upd_ptr->PcdEnableDma0 = config->PcdEnableDma0;
	upd_ptr->PcdEnableDma1 = config->PcdEnableDma1;
	upd_ptr->PcdEnableI2C0 = config->PcdEnableI2C0;
	upd_ptr->PcdEnableI2C1 = config->PcdEnableI2C1;
	upd_ptr->PcdEnableI2C2 = config->PcdEnableI2C2;
	upd_ptr->PcdEnableI2C3 = config->PcdEnableI2C3;
	upd_ptr->PcdEnableI2C4 = config->PcdEnableI2C4;
	upd_ptr->PcdEnableI2C5 = config->PcdEnableI2C5;
	upd_ptr->PcdEnableI2C6 = config->PcdEnableI2C6;
	upd_ptr->PcdGraphicsConfigPtr = config->PcdGraphicsConfigPtr;
	upd_ptr->PunitPwrConfigDisable = config->PunitPwrConfigDisable;
	upd_ptr->ChvSvidConfig = config->ChvSvidConfig;
	upd_ptr->DptfDisable = config->DptfDisable;
	upd_ptr->PcdEmmcMode = config->PcdEmmcMode;
	upd_ptr->PcdPciClkSsc = config->PcdPciClkSsc;
	upd_ptr->PcdUsb3ClkSsc = config->PcdUsb3ClkSsc;
	upd_ptr->PcdDispClkSsc = config->PcdDispClkSsc;
	upd_ptr->PcdSataClkSsc = config->PcdSataClkSsc;
	upd_ptr->Usb2Port0PerPortPeTxiSet = config->Usb2Port0PerPortPeTxiSet;
	upd_ptr->Usb2Port0PerPortTxiSet = config->Usb2Port0PerPortTxiSet;
	upd_ptr->Usb2Port0IUsbTxEmphasisEn = config->Usb2Port0IUsbTxEmphasisEn;
	upd_ptr->Usb2Port0PerPortTxPeHalf = config->Usb2Port0PerPortTxPeHalf;
	upd_ptr->Usb2Port1PerPortPeTxiSet = config->Usb2Port1PerPortPeTxiSet;
	upd_ptr->Usb2Port1PerPortTxiSet = config->Usb2Port1PerPortTxiSet;
	upd_ptr->Usb2Port1IUsbTxEmphasisEn = config->Usb2Port1IUsbTxEmphasisEn;
	upd_ptr->Usb2Port1PerPortTxPeHalf = config->Usb2Port1PerPortTxPeHalf;
	upd_ptr->Usb2Port2PerPortPeTxiSet = config->Usb2Port2PerPortPeTxiSet;
	upd_ptr->Usb2Port2PerPortTxiSet = config->Usb2Port2PerPortTxiSet;
	upd_ptr->Usb2Port2IUsbTxEmphasisEn = config->Usb2Port2IUsbTxEmphasisEn;
	upd_ptr->Usb2Port2PerPortTxPeHalf = config->Usb2Port2PerPortTxPeHalf;
	upd_ptr->Usb2Port3PerPortPeTxiSet = config->Usb2Port3PerPortPeTxiSet;
	upd_ptr->Usb2Port3PerPortTxiSet = config->Usb2Port3PerPortTxiSet;
	upd_ptr->Usb2Port3IUsbTxEmphasisEn = config->Usb2Port3IUsbTxEmphasisEn;
	upd_ptr->Usb2Port3PerPortTxPeHalf = config->Usb2Port3PerPortTxPeHalf;
	upd_ptr->Usb2Port4PerPortPeTxiSet = config->Usb2Port4PerPortPeTxiSet;
	upd_ptr->Usb2Port4PerPortTxiSet = config->Usb2Port4PerPortTxiSet;
	upd_ptr->Usb2Port4IUsbTxEmphasisEn = config->Usb2Port4IUsbTxEmphasisEn;
	upd_ptr->Usb2Port4PerPortTxPeHalf = config->Usb2Port4PerPortTxPeHalf;
	upd_ptr->Usb3Lane0Ow2tapgen2deemph3p5 =
		config->Usb3Lane0Ow2tapgen2deemph3p5;
	upd_ptr->Usb3Lane1Ow2tapgen2deemph3p5 =
		config->Usb3Lane1Ow2tapgen2deemph3p5;
	upd_ptr->Usb3Lane2Ow2tapgen2deemph3p5 =
		config->Usb3Lane2Ow2tapgen2deemph3p5;
	upd_ptr->Usb3Lane3Ow2tapgen2deemph3p5 =
		config->Usb3Lane3Ow2tapgen2deemph3p5;
	upd_ptr->PcdSataInterfaceSpeed = config->PcdSataInterfaceSpeed;
	upd_ptr->PcdPchUsbSsicPort = config->PcdPchUsbSsicPort;
	upd_ptr->PcdPchUsbHsicPort = config->PcdPchUsbHsicPort;
	upd_ptr->PcdPcieRootPortSpeed = config->PcdPcieRootPortSpeed;
	upd_ptr->PcdPchSsicEnable = config->PcdPchSsicEnable;
	upd_ptr->PcdLogoPtr = config->PcdLogoPtr;
	upd_ptr->PcdLogoSize = config->PcdLogoSize;
	upd_ptr->PcdRtcLock = config->PcdRtcLock;
}

void soc_display_silicon_init_params(const UPD_DATA_REGION *original,
	UPD_DATA_REGION *upd_ptr)
{
	/* Display the parameters for SiliconInit */
	printk(BIOS_SPEW, "UPD values for SiliconInit:\n");
	soc_display_upd_value("PcdSdcardMode", 1, original->PcdSdcardMode,
		upd_ptr->PcdSdcardMode);
	soc_display_upd_value("PcdEnableHsuart0", 1, original->PcdEnableHsuart0,
		upd_ptr->PcdEnableHsuart0);
	soc_display_upd_value("PcdEnableHsuart1", 1, original->PcdEnableHsuart1,
		upd_ptr->PcdEnableHsuart1);
	soc_display_upd_value("PcdEnableAzalia", 1, original->PcdEnableAzalia,
		upd_ptr->PcdEnableAzalia);
	soc_display_upd_value("AzaliaVerbTablePtr", 4,
		(uint32_t)original->AzaliaVerbTablePtr,
		(uint32_t)upd_ptr->AzaliaVerbTablePtr);
	soc_display_upd_value("AzaliaConfigPtr", 4, original->AzaliaConfigPtr,
		upd_ptr->AzaliaConfigPtr);
	soc_display_upd_value("PcdEnableSata", 1, original->PcdEnableSata,
		upd_ptr->PcdEnableSata);
	soc_display_upd_value("PcdEnableXhci", 1, original->PcdEnableXhci,
		upd_ptr->PcdEnableXhci);
	soc_display_upd_value("PcdEnableLpe", 1, original->PcdEnableLpe,
		upd_ptr->PcdEnableLpe);
	soc_display_upd_value("PcdEnableDma0", 1, original->PcdEnableDma0,
		upd_ptr->PcdEnableDma0);
	soc_display_upd_value("PcdEnableDma1", 1, original->PcdEnableDma1,
		upd_ptr->PcdEnableDma1);
	soc_display_upd_value("PcdEnableI2C0", 1, original->PcdEnableI2C0,
		upd_ptr->PcdEnableI2C0);
	soc_display_upd_value("PcdEnableI2C1", 1, original->PcdEnableI2C1,
		upd_ptr->PcdEnableI2C1);
	soc_display_upd_value("PcdEnableI2C2", 1, original->PcdEnableI2C2,
		upd_ptr->PcdEnableI2C2);
	soc_display_upd_value("PcdEnableI2C3", 1, original->PcdEnableI2C3,
		upd_ptr->PcdEnableI2C3);
	soc_display_upd_value("PcdEnableI2C4", 1, original->PcdEnableI2C4,
		upd_ptr->PcdEnableI2C4);
	soc_display_upd_value("PcdEnableI2C5", 1, original->PcdEnableI2C5,
		upd_ptr->PcdEnableI2C5);
	soc_display_upd_value("PcdEnableI2C6", 1, original->PcdEnableI2C6,
		upd_ptr->PcdEnableI2C6);
	soc_display_upd_value("PcdGraphicsConfigPtr", 4,
		original->PcdGraphicsConfigPtr, upd_ptr->PcdGraphicsConfigPtr);
	soc_display_upd_value("GpioFamilyInitTablePtr", 4,
		(uint32_t)original->GpioFamilyInitTablePtr,
		(uint32_t)upd_ptr->GpioFamilyInitTablePtr);
	soc_display_upd_value("GpioPadInitTablePtr", 4,
		(uint32_t)original->GpioPadInitTablePtr,
		(uint32_t)upd_ptr->GpioPadInitTablePtr);
	soc_display_upd_value("PunitPwrConfigDisable", 1,
		original->PunitPwrConfigDisable,
		upd_ptr->PunitPwrConfigDisable);
	soc_display_upd_value("ChvSvidConfig", 1, original->ChvSvidConfig,
		upd_ptr->ChvSvidConfig);
	soc_display_upd_value("DptfDisable", 1, original->DptfDisable,
		upd_ptr->DptfDisable);
	soc_display_upd_value("PcdEmmcMode", 1, original->PcdEmmcMode,
		upd_ptr->PcdEmmcMode);
	soc_display_upd_value("PcdPciClkSsc", 1, original->PcdPciClkSsc,
		upd_ptr->PcdPciClkSsc);
	soc_display_upd_value("PcdUsb3ClkSsc", 1, original->PcdUsb3ClkSsc,
		upd_ptr->PcdUsb3ClkSsc);
	soc_display_upd_value("PcdDispClkSsc", 1, original->PcdDispClkSsc,
		upd_ptr->PcdDispClkSsc);
	soc_display_upd_value("PcdSataClkSsc", 1, original->PcdSataClkSsc,
		upd_ptr->PcdSataClkSsc);
	soc_display_upd_value("Usb2Port0PerPortPeTxiSet", 1,
		original->Usb2Port0PerPortPeTxiSet,
		upd_ptr->Usb2Port0PerPortPeTxiSet);
	soc_display_upd_value("Usb2Port0PerPortTxiSet", 1,
		original->Usb2Port0PerPortTxiSet,
		upd_ptr->Usb2Port0PerPortTxiSet);
	soc_display_upd_value("Usb2Port0IUsbTxEmphasisEn", 1,
		original->Usb2Port0IUsbTxEmphasisEn,
		upd_ptr->Usb2Port0IUsbTxEmphasisEn);
	soc_display_upd_value("Usb2Port0PerPortTxPeHalf", 1,
		original->Usb2Port0PerPortTxPeHalf,
		upd_ptr->Usb2Port0PerPortTxPeHalf);
	soc_display_upd_value("Usb2Port1PerPortPeTxiSet", 1,
		original->Usb2Port1PerPortPeTxiSet,
		upd_ptr->Usb2Port1PerPortPeTxiSet);
	soc_display_upd_value("Usb2Port1PerPortTxiSet", 1,
		original->Usb2Port1PerPortTxiSet,
		upd_ptr->Usb2Port1PerPortTxiSet);
	soc_display_upd_value("Usb2Port1IUsbTxEmphasisEn", 1,
		original->Usb2Port1IUsbTxEmphasisEn,
		upd_ptr->Usb2Port1IUsbTxEmphasisEn);
	soc_display_upd_value("Usb2Port1PerPortTxPeHalf", 1,
		original->Usb2Port1PerPortTxPeHalf,
		upd_ptr->Usb2Port1PerPortTxPeHalf);
	soc_display_upd_value("Usb2Port2PerPortPeTxiSet", 1,
		original->Usb2Port2PerPortPeTxiSet,
		upd_ptr->Usb2Port2PerPortPeTxiSet);
	soc_display_upd_value("Usb2Port2PerPortTxiSet", 1,
		original->Usb2Port2PerPortTxiSet,
		upd_ptr->Usb2Port2PerPortTxiSet);
	soc_display_upd_value("Usb2Port2IUsbTxEmphasisEn", 1,
		original->Usb2Port2IUsbTxEmphasisEn,
		upd_ptr->Usb2Port2IUsbTxEmphasisEn);
	soc_display_upd_value("Usb2Port2PerPortTxPeHalf", 1,
		original->Usb2Port2PerPortTxPeHalf,
		upd_ptr->Usb2Port2PerPortTxPeHalf);
	soc_display_upd_value("Usb2Port3PerPortPeTxiSet", 1,
		original->Usb2Port3PerPortPeTxiSet,
		upd_ptr->Usb2Port3PerPortPeTxiSet);
	soc_display_upd_value("Usb2Port3PerPortTxiSet", 1,
		original->Usb2Port3PerPortTxiSet,
		upd_ptr->Usb2Port3PerPortTxiSet);
	soc_display_upd_value("Usb2Port3IUsbTxEmphasisEn", 1,
		original->Usb2Port3IUsbTxEmphasisEn,
		upd_ptr->Usb2Port3IUsbTxEmphasisEn);
	soc_display_upd_value("Usb2Port3PerPortTxPeHalf", 1,
		original->Usb2Port3PerPortTxPeHalf,
		upd_ptr->Usb2Port3PerPortTxPeHalf);
	soc_display_upd_value("Usb2Port4PerPortPeTxiSet", 1,
		original->Usb2Port4PerPortPeTxiSet,
		upd_ptr->Usb2Port4PerPortPeTxiSet);
	soc_display_upd_value("Usb2Port4PerPortTxiSet", 1,
		original->Usb2Port4PerPortTxiSet,
		upd_ptr->Usb2Port4PerPortTxiSet);
	soc_display_upd_value("Usb2Port4IUsbTxEmphasisEn", 1,
		original->Usb2Port4IUsbTxEmphasisEn,
		upd_ptr->Usb2Port4IUsbTxEmphasisEn);
	soc_display_upd_value("Usb2Port4PerPortTxPeHalf", 1,
		original->Usb2Port4PerPortTxPeHalf,
		upd_ptr->Usb2Port4PerPortTxPeHalf);
	soc_display_upd_value("Usb3Lane0Ow2tapgen2deemph3p5", 1,
		original->Usb3Lane0Ow2tapgen2deemph3p5,
		upd_ptr->Usb3Lane0Ow2tapgen2deemph3p5);
	soc_display_upd_value("Usb3Lane1Ow2tapgen2deemph3p5", 1,
		original->Usb3Lane1Ow2tapgen2deemph3p5,
		upd_ptr->Usb3Lane1Ow2tapgen2deemph3p5);
	soc_display_upd_value("Usb3Lane2Ow2tapgen2deemph3p5", 1,
		original->Usb3Lane2Ow2tapgen2deemph3p5,
		upd_ptr->Usb3Lane2Ow2tapgen2deemph3p5);
	soc_display_upd_value("Usb3Lane3Ow2tapgen2deemph3p5", 1,
		original->Usb3Lane3Ow2tapgen2deemph3p5,
		upd_ptr->Usb3Lane3Ow2tapgen2deemph3p5);
	soc_display_upd_value("PcdSataInterfaceSpeed", 1,
		original->PcdSataInterfaceSpeed,
		upd_ptr->PcdSataInterfaceSpeed);
	soc_display_upd_value("PcdPchUsbSsicPort", 1,
		original->PcdPchUsbSsicPort, upd_ptr->PcdPchUsbSsicPort);
	soc_display_upd_value("PcdPchUsbHsicPort", 1,
		original->PcdPchUsbHsicPort, upd_ptr->PcdPchUsbHsicPort);
	soc_display_upd_value("PcdPcieRootPortSpeed", 1,
		original->PcdPcieRootPortSpeed, upd_ptr->PcdPcieRootPortSpeed);
	soc_display_upd_value("PcdPchSsicEnable", 1, original->PcdPchSsicEnable,
		upd_ptr->PcdPchSsicEnable);
	soc_display_upd_value("PcdLogoPtr", 4, original->PcdLogoPtr,
		upd_ptr->PcdLogoPtr);
	soc_display_upd_value("PcdLogoSize", 4, original->PcdLogoSize,
		upd_ptr->PcdLogoSize);
	soc_display_upd_value("PcdRtcLock", 1, original->PcdRtcLock,
		upd_ptr->PcdRtcLock);
	soc_display_upd_value("PcdRegionTerminator", 2,
		original->PcdRegionTerminator, upd_ptr->PcdRegionTerminator);
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
