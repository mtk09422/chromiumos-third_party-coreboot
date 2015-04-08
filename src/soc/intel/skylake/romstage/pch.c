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

#include <arch/io.h>
#include <console/console.h>
#include <device/device.h>
#include <device/pci_def.h>
#include <reg_script.h>
#include <soc/iomap.h>
#include <soc/lpc.h>
#include <soc/pch.h>
#include <soc/pci_devs.h>
#include <soc/pm.h>
#include <soc/rcba.h>
#include <soc/romstage.h>
#include <soc/smbus.h>
#include <soc/intel/skylake/chip.h>

const struct reg_script pch_early_init_script[] = {
	/* Setup southbridge BARs */
	REG_PCI_WRITE32(RCBA, RCBA_BASE_ADDRESS | 1),
	REG_PCI_WRITE32(PMBASE, ACPI_BASE_ADDRESS | 1),
	REG_PCI_WRITE8(ACPI_CNTL, ACPI_EN),
	REG_PCI_WRITE32(GPIO_BASE, GPIO_BASE_ADDRESS | 1),
	REG_PCI_WRITE8(GPIO_CNTL, GPIO_EN),

	/* Set COM1/COM2 decode range */
	REG_PCI_WRITE16(LPC_IO_DEC, 0x0010),
	/* Enable legacy decode ranges */
	REG_PCI_WRITE16(LPC_EN, CNF1_LPC_EN | CNF2_LPC_EN | GAMEL_LPC_EN |
			COMA_LPC_EN | KBC_LPC_EN | MC_LPC_EN),

	/* Enable IOAPIC */
	REG_MMIO_WRITE16(RCBA_BASE_ADDRESS + OIC, 0x0100),
	/* Read back for posted write */
	REG_MMIO_READ16(RCBA_BASE_ADDRESS + OIC),

	/* Disable reset */
	REG_MMIO_OR32(RCBA_BASE_ADDRESS + GCS, (1 << 5)),
	/* TCO timer halt */
	REG_IO_OR16(ACPI_BASE_ADDRESS + TCO1_CNT, TCO_TMR_HLT),

	/* Enable upper 128 bytes of CMOS */
	REG_MMIO_OR32(RCBA_BASE_ADDRESS + RC, (1 << 2)),

	/* Disable unused device (always) */
	REG_MMIO_OR32(RCBA_BASE_ADDRESS + FD, PCH_DISABLE_ALWAYS),

	REG_SCRIPT_END
};

static void pch_enable_lpc(void)
{
	/* Lookup device tree in romstage */
	const struct device *dev;
	const config_t *config;

	dev = dev_find_slot(0, PCI_DEVFN(PCH_DEV_SLOT_LPC, 0));
	if (!dev || !dev->chip_info)
		return;
	config = dev->chip_info;

	pci_write_config32(PCH_DEV_LPC, LPC_GEN1_DEC, config->gen1_dec);
	pci_write_config32(PCH_DEV_LPC, LPC_GEN2_DEC, config->gen2_dec);
	pci_write_config32(PCH_DEV_LPC, LPC_GEN3_DEC, config->gen3_dec);
	pci_write_config32(PCH_DEV_LPC, LPC_GEN4_DEC, config->gen4_dec);
}

void pch_early_init(void)
{
	reg_script_run_on_dev(PCH_DEV_LPC, pch_early_init_script);

	pch_enable_lpc();

	enable_smbus();

}
