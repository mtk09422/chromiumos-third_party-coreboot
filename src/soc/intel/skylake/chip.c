/*
 * This file is part of the coreboot project.
 *
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
	assign_resources(dev->link_list);
}

static struct device_operations pci_domain_ops = {
	.read_resources   = &pci_domain_read_resources,
	.set_resources    = &pci_domain_set_resources,
	.scan_bus         = &pci_domain_scan_bus,
	.ops_pci_bus      = &pci_ops_mmconf,
};

static void cpu_bus_noop(device_t dev) { }

static void chip_final(device_t dev)
{
	/* Notify FSP done device setup */
	printk(BIOS_DEBUG,
		"Calling FspNotify(EnumInitPhaseAfterPciEnumeration)\n");
	fsp_notify(EnumInitPhaseAfterPciEnumeration);
}

static struct device_operations cpu_bus_ops = {
	.read_resources   = &cpu_bus_noop,
	.set_resources    = &cpu_bus_noop,
	.enable_resources = &cpu_bus_noop,
	.init             = &skylake_init_cpus,
	.final            = &chip_final,
};

static void skylake_enable(device_t dev)
{
	/* Set the operations if it is a special bus type */
	if (dev->path.type == DEVICE_PATH_DOMAIN) {
		dev->ops = &pci_domain_ops;
	} else if (dev->path.type == DEVICE_PATH_CPU_CLUSTER) {
		dev->ops = &cpu_bus_ops;
	} else if (dev->path.type == DEVICE_PATH_PCI) {
		/* Handle PCH device enable */
		if (PCI_SLOT(dev->path.pci.devfn) > SA_DEV_SLOT_IGD &&
		    (dev->ops == NULL || dev->ops->enable == NULL)) {
			skylake_pch_enable_dev(dev);
		}
	}
}

struct chip_operations soc_intel_skylake_ops = {
	CHIP_NAME("Intel Skylake")
	.enable_dev = &skylake_enable,
	.init       = &skylake_init_pre_device,
};

/* UPD parameters to be initialized before SiliconInit */
void soc_silicon_init_params(UPD_DATA_REGION *upd_ptr)
{
	const struct device *dev;
	const struct soc_intel_skylake_config *config;

	/* Set the parameters for SiliconInit */
	dev = dev_find_slot(0, PCI_DEVFN(PCH_DEV_SLOT_LPC, 0));
	if (!dev || !dev->chip_info)
		return;
	config = dev->chip_info;

	upd_ptr->DspEnable = config->dsp_enable;
	upd_ptr->EnableAzalia = config->enable_azalia;
	upd_ptr->IoBufferOwnership = config->io_buffer_ownership;
	upd_ptr->SmbusEnable = config->smbus_enable;
	upd_ptr->ScsEmmcEnabled = config->scs_emmc_enabled;
	upd_ptr->ScsEmmcHs400Enabled = config->scs_emmchs400_enabled;
	upd_ptr->ScsSdCardEnabled = config->scs_sdcard_enabled;
	upd_ptr->Cio2Enable = config->cio2_enable;
	upd_ptr->IshEnable = config->ish_enable;
}

void soc_display_silicon_init_params(const UPD_DATA_REGION *original,
	UPD_DATA_REGION *upd_ptr)
{
	/* Display the parameters for SiliconInit */
	printk(BIOS_SPEW, "UPD values for SiliconInit:\n");
	soc_display_upd_value("EnableLan", 1, original->EnableLan,
		upd_ptr->EnableLan);
	soc_display_upd_value("EnableSata", 1, original->EnableSata,
		upd_ptr->EnableSata);
	soc_display_upd_value("SataMode", 1, original->SataMode,
		upd_ptr->SataMode);
	soc_display_upd_value("EnableAzalia", 1, original->EnableAzalia,
		upd_ptr->EnableAzalia);
	soc_display_upd_value("EnableTraceHub", 1, original->EnableTraceHub,
		upd_ptr->EnableTraceHub);
	soc_display_upd_value("SerialDebugPortAddress", 4,
		original->SerialDebugPortAddress,
		upd_ptr->SerialDebugPortAddress);
	soc_display_upd_value("SerialDebugPortType", 1,
		original->SerialDebugPortType,
		upd_ptr->SerialDebugPortType);
	soc_display_upd_value("PcieRpEnable[0]", 1, original->PcieRpEnable[0],
		upd_ptr->PcieRpEnable[0]);
	soc_display_upd_value("PcieRpEnable[1]", 1, original->PcieRpEnable[1],
		upd_ptr->PcieRpEnable[1]);
	soc_display_upd_value("PcieRpEnable[2]", 1, original->PcieRpEnable[2],
		upd_ptr->PcieRpEnable[2]);
	soc_display_upd_value("PcieRpEnable[3]", 1, original->PcieRpEnable[3],
		upd_ptr->PcieRpEnable[3]);
	soc_display_upd_value("PcieRpEnable[4]", 1, original->PcieRpEnable[4],
		upd_ptr->PcieRpEnable[4]);
	soc_display_upd_value("PcieRpEnable[5]", 1, original->PcieRpEnable[5],
		upd_ptr->PcieRpEnable[5]);
	soc_display_upd_value("PcieRpEnable[6]", 1, original->PcieRpEnable[6],
		upd_ptr->PcieRpEnable[6]);
	soc_display_upd_value("PcieRpEnable[7]", 1, original->PcieRpEnable[7],
		upd_ptr->PcieRpEnable[7]);
	soc_display_upd_value("PcieRpEnable[8]", 1, original->PcieRpEnable[8],
		upd_ptr->PcieRpEnable[8]);
	soc_display_upd_value("PcieRpEnable[9]", 1, original->PcieRpEnable[9],
		upd_ptr->PcieRpEnable[9]);
	soc_display_upd_value("PcieRpEnable[10]", 1, original->PcieRpEnable[10],
		upd_ptr->PcieRpEnable[10]);
	soc_display_upd_value("PcieRpEnable[11]", 1, original->PcieRpEnable[11],
		upd_ptr->PcieRpEnable[11]);
	soc_display_upd_value("PcieRpEnable[12]", 1, original->PcieRpEnable[12],
		upd_ptr->PcieRpEnable[12]);
	soc_display_upd_value("PcieRpEnable[13]", 1, original->PcieRpEnable[13],
		upd_ptr->PcieRpEnable[13]);
	soc_display_upd_value("PcieRpEnable[14]", 1, original->PcieRpEnable[14],
		upd_ptr->PcieRpEnable[14]);
	soc_display_upd_value("PcieRpEnable[15]", 1, original->PcieRpEnable[15],
		upd_ptr->PcieRpEnable[15]);
	soc_display_upd_value("PcieRpEnable[16]", 1, original->PcieRpEnable[16],
		upd_ptr->PcieRpEnable[16]);
	soc_display_upd_value("PcieRpEnable[17]", 1, original->PcieRpEnable[17],
		upd_ptr->PcieRpEnable[17]);
	soc_display_upd_value("PcieRpEnable[18]", 1, original->PcieRpEnable[18],
		upd_ptr->PcieRpEnable[18]);
	soc_display_upd_value("PcieRpEnable[19]", 1, original->PcieRpEnable[19],
		upd_ptr->PcieRpEnable[19]);
	soc_display_upd_value("PcieRpClkReqSupport[0]", 1,
		original->PcieRpClkReqSupport[0],
		upd_ptr->PcieRpClkReqSupport[0]);
	soc_display_upd_value("PcieRpClkReqSupport[1]", 1,
		original->PcieRpClkReqSupport[1],
		upd_ptr->PcieRpClkReqSupport[1]);
	soc_display_upd_value("PcieRpClkReqSupport[2]", 1,
		original->PcieRpClkReqSupport[2],
		upd_ptr->PcieRpClkReqSupport[2]);
	soc_display_upd_value("PcieRpClkReqSupport[3]", 1,
		original->PcieRpClkReqSupport[3],
		upd_ptr->PcieRpClkReqSupport[3]);
	soc_display_upd_value("PcieRpClkReqSupport[4]", 1,
		original->PcieRpClkReqSupport[4],
		upd_ptr->PcieRpClkReqSupport[4]);
	soc_display_upd_value("PcieRpClkReqSupport[5]", 1,
		original->PcieRpClkReqSupport[5],
		upd_ptr->PcieRpClkReqSupport[5]);
	soc_display_upd_value("PcieRpClkReqSupport[6]", 1,
		original->PcieRpClkReqSupport[6],
		upd_ptr->PcieRpClkReqSupport[6]);
	soc_display_upd_value("PcieRpClkReqSupport[7]", 1,
		original->PcieRpClkReqSupport[7],
		upd_ptr->PcieRpClkReqSupport[7]);
	soc_display_upd_value("PcieRpClkReqSupport[8]", 1,
		original->PcieRpClkReqSupport[8],
		upd_ptr->PcieRpClkReqSupport[8]);
	soc_display_upd_value("PcieRpClkReqSupport[9]", 1,
		original->PcieRpClkReqSupport[9],
		upd_ptr->PcieRpClkReqSupport[9]);
	soc_display_upd_value("PcieRpClkReqSupport[10]", 1,
		original->PcieRpClkReqSupport[10],
		upd_ptr->PcieRpClkReqSupport[10]);
	soc_display_upd_value("PcieRpClkReqSupport[11]", 1,
		original->PcieRpClkReqSupport[11],
		upd_ptr->PcieRpClkReqSupport[11]);
	soc_display_upd_value("PcieRpClkReqSupport[12]", 1,
		original->PcieRpClkReqSupport[12],
		upd_ptr->PcieRpClkReqSupport[12]);
	soc_display_upd_value("PcieRpClkReqSupport[13]", 1,
		original->PcieRpClkReqSupport[13],
		upd_ptr->PcieRpClkReqSupport[13]);
	soc_display_upd_value("PcieRpClkReqSupport[14]", 1,
		original->PcieRpClkReqSupport[14],
		upd_ptr->PcieRpClkReqSupport[14]);
	soc_display_upd_value("PcieRpClkReqSupport[15]", 1,
		original->PcieRpClkReqSupport[15],
		upd_ptr->PcieRpClkReqSupport[15]);
	soc_display_upd_value("PcieRpClkReqSupport[16]", 1,
		original->PcieRpClkReqSupport[16],
		upd_ptr->PcieRpClkReqSupport[16]);
	soc_display_upd_value("PcieRpClkReqSupport[17]", 1,
		original->PcieRpClkReqSupport[17],
		upd_ptr->PcieRpClkReqSupport[17]);
	soc_display_upd_value("PcieRpClkReqSupport[18]", 1,
		original->PcieRpClkReqSupport[18],
		upd_ptr->PcieRpClkReqSupport[18]);
	soc_display_upd_value("PcieRpClkReqSupport[19]", 1,
		original->PcieRpClkReqSupport[19],
		upd_ptr->PcieRpClkReqSupport[19]);
	soc_display_upd_value("PcieRpClkReqNumber[0]", 1,
		original->PcieRpClkReqNumber[0],
		upd_ptr->PcieRpClkReqNumber[0]);
	soc_display_upd_value("PcieRpClkReqNumber[1]", 1,
		original->PcieRpClkReqNumber[1],
		upd_ptr->PcieRpClkReqNumber[1]);
	soc_display_upd_value("PcieRpClkReqNumber[2]", 1,
		original->PcieRpClkReqNumber[2],
		upd_ptr->PcieRpClkReqNumber[2]);
	soc_display_upd_value("PcieRpClkReqNumber[3]", 1,
		original->PcieRpClkReqNumber[3],
		upd_ptr->PcieRpClkReqNumber[3]);
	soc_display_upd_value("PcieRpClkReqNumber[4]", 1,
		original->PcieRpClkReqNumber[4],
		upd_ptr->PcieRpClkReqNumber[4]);
	soc_display_upd_value("PcieRpClkReqNumber[5]", 1,
		original->PcieRpClkReqNumber[5],
		upd_ptr->PcieRpClkReqNumber[5]);
	soc_display_upd_value("PcieRpClkReqNumber[6]", 1,
		original->PcieRpClkReqNumber[6],
		upd_ptr->PcieRpClkReqNumber[6]);
	soc_display_upd_value("PcieRpClkReqNumber[7]", 1,
		original->PcieRpClkReqNumber[7],
		upd_ptr->PcieRpClkReqNumber[7]);
	soc_display_upd_value("PcieRpClkReqNumber[8]", 1,
		original->PcieRpClkReqNumber[8],
		upd_ptr->PcieRpClkReqNumber[8]);
	soc_display_upd_value("PcieRpClkReqNumber[9]", 1,
		original->PcieRpClkReqNumber[9],
		upd_ptr->PcieRpClkReqNumber[9]);
	soc_display_upd_value("PcieRpClkReqNumber[10]", 1,
		original->PcieRpClkReqNumber[10],
		upd_ptr->PcieRpClkReqNumber[10]);
	soc_display_upd_value("PcieRpClkReqNumber[11]", 1,
		original->PcieRpClkReqNumber[11],
		upd_ptr->PcieRpClkReqNumber[11]);
	soc_display_upd_value("PcieRpClkReqNumber[12]", 1,
		original->PcieRpClkReqNumber[12],
		upd_ptr->PcieRpClkReqNumber[12]);
	soc_display_upd_value("PcieRpClkReqNumber[13]", 1,
		original->PcieRpClkReqNumber[13],
		upd_ptr->PcieRpClkReqNumber[13]);
	soc_display_upd_value("PcieRpClkReqNumber[14]", 1,
		original->PcieRpClkReqNumber[14],
		upd_ptr->PcieRpClkReqNumber[14]);
	soc_display_upd_value("PcieRpClkReqNumber[15]", 1,
		original->PcieRpClkReqNumber[15],
		upd_ptr->PcieRpClkReqNumber[15]);
	soc_display_upd_value("PcieRpClkReqNumber[16]", 1,
		original->PcieRpClkReqNumber[16],
		upd_ptr->PcieRpClkReqNumber[16]);
	soc_display_upd_value("PcieRpClkReqNumber[17]", 1,
		original->PcieRpClkReqNumber[17],
		upd_ptr->PcieRpClkReqNumber[17]);
	soc_display_upd_value("PcieRpClkReqNumber[18]", 1,
		original->PcieRpClkReqNumber[18],
		upd_ptr->PcieRpClkReqNumber[18]);
	soc_display_upd_value("PcieRpClkReqNumber[19]", 1,
		original->PcieRpClkReqNumber[19],
		upd_ptr->PcieRpClkReqNumber[19]);
	soc_display_upd_value("SataSalpSupport", 1, original->SataSalpSupport,
		upd_ptr->SataSalpSupport);
	soc_display_upd_value("SataPortsEnable[0]", 1,
		original->SataPortsEnable[0],
		upd_ptr->SataPortsEnable[0]);
	soc_display_upd_value("SataPortsEnable[1]", 1,
		original->SataPortsEnable[1],
		upd_ptr->SataPortsEnable[1]);
	soc_display_upd_value("SataPortsEnable[2]", 1,
		original->SataPortsEnable[2],
		upd_ptr->SataPortsEnable[2]);
	soc_display_upd_value("SataPortsEnable[3]", 1,
		original->SataPortsEnable[3],
		upd_ptr->SataPortsEnable[3]);
	soc_display_upd_value("SataPortsEnable[4]", 1,
		original->SataPortsEnable[4],
		upd_ptr->SataPortsEnable[4]);
	soc_display_upd_value("SataPortsEnable[5]", 1,
		original->SataPortsEnable[5],
		upd_ptr->SataPortsEnable[5]);
	soc_display_upd_value("SataPortsEnable[6]", 1,
		original->SataPortsEnable[6],
		upd_ptr->SataPortsEnable[6]);
	soc_display_upd_value("SataPortsEnable[7]", 1,
		original->SataPortsEnable[7],
		upd_ptr->SataPortsEnable[7]);
	soc_display_upd_value("PortUsb20Enable[0]", 1,
		original->PortUsb20Enable[0],
		upd_ptr->PortUsb20Enable[0]);
	soc_display_upd_value("PortUsb20Enable[1]", 1,
		original->PortUsb20Enable[1],
		upd_ptr->PortUsb20Enable[1]);
	soc_display_upd_value("PortUsb20Enable[2]", 1,
		original->PortUsb20Enable[2],
		upd_ptr->PortUsb20Enable[2]);
	soc_display_upd_value("PortUsb20Enable[3]", 1,
		original->PortUsb20Enable[3],
		upd_ptr->PortUsb20Enable[3]);
	soc_display_upd_value("PortUsb20Enable[4]", 1,
		original->PortUsb20Enable[4],
		upd_ptr->PortUsb20Enable[4]);
	soc_display_upd_value("PortUsb20Enable[5]", 1,
		original->PortUsb20Enable[5],
		upd_ptr->PortUsb20Enable[5]);
	soc_display_upd_value("PortUsb20Enable[6]", 1,
		original->PortUsb20Enable[6],
		upd_ptr->PortUsb20Enable[6]);
	soc_display_upd_value("PortUsb20Enable[7]", 1,
		original->PortUsb20Enable[7],
		upd_ptr->PortUsb20Enable[7]);
	soc_display_upd_value("PortUsb20Enable[8]", 1,
		original->PortUsb20Enable[8],
		upd_ptr->PortUsb20Enable[8]);
	soc_display_upd_value("PortUsb20Enable[9]", 1,
		original->PortUsb20Enable[9],
		upd_ptr->PortUsb20Enable[9]);
	soc_display_upd_value("PortUsb20Enable[10]", 1,
		original->PortUsb20Enable[10],
		upd_ptr->PortUsb20Enable[10]);
	soc_display_upd_value("PortUsb20Enable[11]", 1,
		original->PortUsb20Enable[11],
		upd_ptr->PortUsb20Enable[11]);
	soc_display_upd_value("PortUsb20Enable[12]", 1,
		original->PortUsb20Enable[12],
		upd_ptr->PortUsb20Enable[12]);
	soc_display_upd_value("PortUsb20Enable[13]", 1,
		original->PortUsb20Enable[13],
		upd_ptr->PortUsb20Enable[13]);
	soc_display_upd_value("PortUsb20Enable[14]", 1,
		original->PortUsb20Enable[14],
		upd_ptr->PortUsb20Enable[14]);
	soc_display_upd_value("PortUsb20Enable[15]", 1,
		original->PortUsb20Enable[15],
		upd_ptr->PortUsb20Enable[15]);
	soc_display_upd_value("PortUsb30Enable[0]", 1,
		original->PortUsb30Enable[0],
		upd_ptr->PortUsb30Enable[0]);
	soc_display_upd_value("PortUsb30Enable[1]", 1,
		original->PortUsb30Enable[1],
		upd_ptr->PortUsb30Enable[1]);
	soc_display_upd_value("PortUsb30Enable[2]", 1,
		original->PortUsb30Enable[2],
		upd_ptr->PortUsb30Enable[2]);
	soc_display_upd_value("PortUsb30Enable[3]", 1,
		original->PortUsb30Enable[3],
		upd_ptr->PortUsb30Enable[3]);
	soc_display_upd_value("PortUsb30Enable[4]", 1,
		original->PortUsb30Enable[4],
		upd_ptr->PortUsb30Enable[4]);
	soc_display_upd_value("PortUsb30Enable[5]", 1,
		original->PortUsb30Enable[5],
		upd_ptr->PortUsb30Enable[5]);
	soc_display_upd_value("PortUsb30Enable[6]", 1,
		original->PortUsb30Enable[6],
		upd_ptr->PortUsb30Enable[6]);
	soc_display_upd_value("PortUsb30Enable[7]", 1,
		original->PortUsb30Enable[7],
		upd_ptr->PortUsb30Enable[7]);
	soc_display_upd_value("PortUsb30Enable[8]", 1,
		original->PortUsb30Enable[8],
		upd_ptr->PortUsb30Enable[8]);
	soc_display_upd_value("PortUsb30Enable[9]", 1,
		original->PortUsb30Enable[9],
		upd_ptr->PortUsb30Enable[9]);
	soc_display_upd_value("SsicPortEnable", 1, original->SsicPortEnable,
		upd_ptr->SsicPortEnable);
	soc_display_upd_value("DspEnable", 1, original->DspEnable,
		upd_ptr->DspEnable);
	soc_display_upd_value("IoBufferOwnership", 1,
		original->IoBufferOwnership,
		upd_ptr->IoBufferOwnership);
	soc_display_upd_value("SmbusEnable", 1, original->SmbusEnable,
		upd_ptr->SmbusEnable);
	soc_display_upd_value("SerialIoDevMode[0]", 1,
		original->SerialIoDevMode[0],
		upd_ptr->SerialIoDevMode[0]);
	soc_display_upd_value("SerialIoDevMode[1]", 1,
		original->SerialIoDevMode[1],
		upd_ptr->SerialIoDevMode[1]);
	soc_display_upd_value("SerialIoDevMode[2]", 1,
		original->SerialIoDevMode[2],
		upd_ptr->SerialIoDevMode[2]);
	soc_display_upd_value("SerialIoDevMode[3]", 1,
		original->SerialIoDevMode[3],
		upd_ptr->SerialIoDevMode[3]);
	soc_display_upd_value("SerialIoDevMode[4]", 1,
		original->SerialIoDevMode[4],
		upd_ptr->SerialIoDevMode[4]);
	soc_display_upd_value("SerialIoDevMode[5]", 1,
		original->SerialIoDevMode[5],
		upd_ptr->SerialIoDevMode[5]);
	soc_display_upd_value("SerialIoDevMode[6]", 1,
		original->SerialIoDevMode[6],
		upd_ptr->SerialIoDevMode[6]);
	soc_display_upd_value("SerialIoDevMode[7]", 1,
		original->SerialIoDevMode[7],
		upd_ptr->SerialIoDevMode[7]);
	soc_display_upd_value("SerialIoDevMode[8]", 1,
		original->SerialIoDevMode[8],
		upd_ptr->SerialIoDevMode[8]);
	soc_display_upd_value("SerialIoDevMode[9]", 1,
		original->SerialIoDevMode[9],
		upd_ptr->SerialIoDevMode[9]);
	soc_display_upd_value("SerialIoDevMode[10]", 1,
		original->SerialIoDevMode[10],
		upd_ptr->SerialIoDevMode[10]);
	soc_display_upd_value("Cio2Enable", 1, original->Cio2Enable,
		upd_ptr->Cio2Enable);
	soc_display_upd_value("ScsEmmcEnabled", 1, original->ScsEmmcEnabled,
		upd_ptr->ScsEmmcEnabled);
	soc_display_upd_value("ScsEmmcHs400Enabled", 1,
		original->ScsEmmcHs400Enabled,
		upd_ptr->ScsEmmcHs400Enabled);
	soc_display_upd_value("ScsSdCardEnabled", 1, original->ScsSdCardEnabled,
		upd_ptr->ScsSdCardEnabled);
	soc_display_upd_value("IshEnable", 1, original->IshEnable,
		upd_ptr->IshEnable);
	soc_display_upd_value("ShowSpiController", 1,
		original->ShowSpiController, upd_ptr->ShowSpiController);
	soc_display_upd_value("SataPortsDevSlp[0]", 1,
		original->SataPortsDevSlp[0],
		upd_ptr->SataPortsDevSlp[0]);
	soc_display_upd_value("SataPortsDevSlp[1]", 1,
		original->SataPortsDevSlp[1],
		upd_ptr->SataPortsDevSlp[1]);
	soc_display_upd_value("SataPortsDevSlp[2]", 1,
		original->SataPortsDevSlp[2],
		upd_ptr->SataPortsDevSlp[2]);
	soc_display_upd_value("SataPortsDevSlp[3]", 1,
		original->SataPortsDevSlp[3],
		upd_ptr->SataPortsDevSlp[3]);
	soc_display_upd_value("SataPortsDevSlp[4]", 1,
		original->SataPortsDevSlp[4],
		upd_ptr->SataPortsDevSlp[4]);
	soc_display_upd_value("SataPortsDevSlp[5]", 1,
		original->SataPortsDevSlp[5],
		upd_ptr->SataPortsDevSlp[5]);
	soc_display_upd_value("SataPortsDevSlp[6]", 1,
		original->SataPortsDevSlp[6],
		upd_ptr->SataPortsDevSlp[6]);
	soc_display_upd_value("SataPortsDevSlp[7]", 1,
		original->SataPortsDevSlp[7],
		upd_ptr->SataPortsDevSlp[7]);
	soc_display_upd_value("GpioTablePtr", 4,
		(uint32_t)original->GpioTablePtr,
		(uint32_t)upd_ptr->GpioTablePtr);
}

static void pci_set_subsystem(device_t dev, unsigned vendor, unsigned device)
{
	if (!vendor || !device)
		pci_write_config32(dev, PCI_SUBSYSTEM_VENDOR_ID,
				   pci_read_config32(dev, PCI_VENDOR_ID));
	else
		pci_write_config32(dev, PCI_SUBSYSTEM_VENDOR_ID,
				   (device << 16) | vendor);
}

struct pci_operations skylake_pci_ops = {
	.set_subsystem = &pci_set_subsystem
};
