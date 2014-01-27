/*
 * This file is part of the coreboot project.
 *
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

#include <arch/io.h>
#include <cbmem.h>
#include <console/console.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <reg_script.h>

#include <baytrail/iomap.h>
#include <baytrail/iosf.h>
#include <baytrail/nvs.h>
#include <baytrail/pci_devs.h>
#include <baytrail/pmc.h>
#include <baytrail/ramstage.h>
#include "chip.h"


/* The LPE audio devices needs 1MiB of memory reserved aligned to a 512MiB
 * address. Just take 1MiB @ 512MiB. */
#define FIRMWARE_PHYS_BASE (512 << 20)
#define FIRMWARE_PHYS_LENGTH (1 << 20)
#define FIRMWARE_REG_BASE 0xa8
#define FIRMWARE_REG_LENGTH 0xac

static void lpe_enable_acpi_mode(device_t dev)
{
	static const struct reg_script ops[] = {
		/* Disable PCI interrupt, enable Memory and Bus Master */
		REG_PCI_OR32(PCI_COMMAND,
			     PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER | (1<<10)),
		/* Enable ACPI mode */
		REG_IOSF_OR(IOSF_PORT_0x58, LPE_PCICFGCTR1,
			    LPE_PCICFGCTR1_PCI_CFG_DIS |
			    LPE_PCICFGCTR1_ACPI_INT_EN),
		REG_SCRIPT_END
	};
	struct resource *bar;
	global_nvs_t *gnvs;

	/* Find ACPI NVS to update BARs */
	gnvs = (global_nvs_t *)cbmem_find(CBMEM_ID_ACPI_GNVS);
	if (!gnvs) {
		printk(BIOS_ERR, "Unable to locate Global NVS\n");
		return;
	}

	/* Save BAR0 and BAR1 to ACPI NVS */
	bar = find_resource(dev, PCI_BASE_ADDRESS_0);
	if (bar)
		gnvs->dev.lpe_bar0 = (u32)bar->base;

	bar = find_resource(dev, PCI_BASE_ADDRESS_1);
	if (bar)
		gnvs->dev.lpe_bar1 = (u32)bar->base;

	/* Save reserved firmware region */
	gnvs->dev.lpe_fw = (u32)FIRMWARE_REG_BASE;

	/* Device is enabled in ACPI mode */
	gnvs->dev.lpe_en = 1;

	/* Put device in ACPI mode */
	reg_script_run_on_dev(dev, ops);
}

static void setup_codec_clock(device_t dev)
{
	uint32_t reg;
	int clk_reg;
	struct soc_intel_baytrail_config *config;
	const char *freq_str;

	config = dev->chip_info;
	switch (config->lpe_codec_clk_freq) {
	case 19:
		freq_str = "19.2";
		reg = CLK_FREQ_19P2MHZ;
		break;
	case 25:
		freq_str = "25";
		reg = CLK_FREQ_25MHZ;
		break;
	default:
		printk(BIOS_DEBUG, "LPE codec clock not required.\n");
		return;
	}

	/* Default to always running. */
	reg |= CLK_CTL_ON;

	if (config->lpe_codec_clk_num < 0 || config->lpe_codec_clk_num > 5) {
		printk(BIOS_DEBUG, "Invalid LPE codec clock number.\n");
		return;
	}

	printk(BIOS_DEBUG, "LPE Audio codec clock set to %sMHz.\n", freq_str);

	clk_reg = PMC_BASE_ADDRESS + PLT_CLK_CTL_0;
	clk_reg += 4 * config->lpe_codec_clk_num;

	write32(clk_reg, (read32(clk_reg) & ~0x7) | reg);
}

static void lpe_init(device_t dev)
{
	struct soc_intel_baytrail_config *config = dev->chip_info;

	setup_codec_clock(dev);

	if (config->lpe_acpi_mode)
		lpe_enable_acpi_mode(dev);
}

static void lpe_read_resources(device_t dev)
{
	pci_dev_read_resources(dev);

	reserved_ram_resource(dev, FIRMWARE_REG_BASE,
			      FIRMWARE_PHYS_BASE >> 10,
			      FIRMWARE_PHYS_LENGTH >> 10);
}

static void lpe_set_resources(device_t dev)
{
	struct resource *res;

	pci_dev_set_resources(dev);

	res = find_resource(dev, FIRMWARE_REG_BASE);
	if (res == NULL) {
		printk(BIOS_DEBUG, "LPE Firmware memory not found.\n");
		return;
	}
	pci_write_config32(dev, FIRMWARE_REG_BASE, res->base);
	pci_write_config32(dev, FIRMWARE_REG_LENGTH, res->size);
}

static const struct device_operations device_ops = {
	.read_resources		= lpe_read_resources,
	.set_resources		= lpe_set_resources,
	.enable_resources	= pci_dev_enable_resources,
	.init			= lpe_init,
	.enable			= NULL,
	.scan_bus		= NULL,
	.ops_pci		= &soc_pci_ops,
};

static const struct pci_driver southcluster __pci_driver = {
	.ops		= &device_ops,
	.vendor		= PCI_VENDOR_ID_INTEL,
	.device		= LPE_DEVID,
};
