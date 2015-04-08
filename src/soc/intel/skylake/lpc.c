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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <console/console.h>
#include <delay.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <pc80/mc146818rtc.h>
#include <pc80/isa-dma.h>
#include <pc80/i8259.h>
#include <arch/io.h>
#include <arch/ioapic.h>
#include <arch/acpi.h>
#include <cpu/cpu.h>
#include <cpu/x86/smm.h>
#include <cbmem.h>
#include <reg_script.h>
#include <string.h>
#include <soc/gpio.h>
#include <soc/iomap.h>
#include <soc/lpc.h>
#include <soc/nvs.h>
#include <soc/pch.h>
#include <soc/pci_devs.h>
#include <soc/pm.h>
#include <soc/ramstage.h>
#include <soc/rcba.h>
#include <soc/intel/skylake/chip.h>

#if IS_ENABLED(CONFIG_CHROMEOS)
#include <vendorcode/google/chromeos/chromeos.h>
#endif

static void pch_enable_ioapic(struct device *dev)
{
	u32 reg32;

	set_ioapic_id((void *)IO_APIC_ADDR, 0x02);

	/* affirm full set of redirection table entries ("write once") */
	reg32 = io_apic_read((void *)IO_APIC_ADDR, 0x01);

	/* PCH-LP has 39 redirection entries */
	reg32 &= ~0x00ff0000;
	reg32 |= 0x00270000;

	io_apic_write((void *)IO_APIC_ADDR, 0x01, reg32);

	/*
	 * Select Boot Configuration register (0x03) and
	 * use Processor System Bus (0x01) to deliver interrupts.
	 */
	io_apic_write((void *)IO_APIC_ADDR, 0x03, 0x01);
}

static void pch_power_options(device_t dev)
{
	/* Get the chip configuration */
	config_t *config = dev->chip_info;

	/* GPE setup based on device tree configuration */
	enable_all_gpe(config->gpe0_en_1, config->gpe0_en_2,
			config->gpe0_en_3, config->gpe0_en_4);

	/* SMI setup based on device tree configuration */
	enable_alt_smi(config->alt_gp_smi_en);
}

#if IS_ENABLED(CONFIG_CHROMEOS_VBNV_CMOS)
/*
 * Preserve Vboot NV data when clearing CMOS as it will
 * have been re-initialized already by Vboot firmware init.
 */
static void pch_cmos_init_preserve(int reset)
{
	uint8_t vbnv[CONFIG_VBNV_SIZE];

	if (reset)
		read_vbnv(vbnv);

	cmos_init(reset);

	if (reset)
		save_vbnv(vbnv);
}
#endif

static void pch_rtc_init(struct device *dev)
{
	u8 reg8;
	int rtc_failed;

	reg8 = pci_read_config8(dev, GEN_PMCON_3);
	rtc_failed = reg8 & RTC_BATTERY_DEAD;
	if (rtc_failed) {
		reg8 &= ~RTC_BATTERY_DEAD;
		pci_write_config8(dev, GEN_PMCON_3, reg8);
		printk(BIOS_DEBUG, "rtc_failed = 0x%x\n", rtc_failed);
	}

#if IS_ENABLED(CONFIG_CHROMEOS_VBNV_CMOS)
	pch_cmos_init_preserve(rtc_failed);
#else
	cmos_init(rtc_failed);
#endif
}

static const struct reg_script pch_misc_init_script[] = {
	/* Setup NMI on errors, disable SERR */
	REG_IO_RMW8(0x61, ~0xf0, (1 << 2)),
	/* Disable NMI sources */
	REG_IO_OR8(0x70, (1 << 7)),
	/* Indicate DRAM init done for MRC */
	REG_PCI_OR8(GEN_PMCON_2, (1 << 7)),
	/* Enable BIOS updates outside of SMM */
	REG_PCI_RMW8(0xdc, ~(1 << 5), 0),
	/* Setup SERIRQ, enable continuous mode */
	REG_PCI_OR8(SERIRQ_CNTL, (1 << 7) | (1 << 6)),
#if !IS_ENABLED(CONFIG_SERIRQ_CONTINUOUS_MODE)
	REG_PCI_RMW8(SERIRQ_CNTL, ~(1 << 6), 0),
#endif
	REG_SCRIPT_END
};


static void pch_set_acpi_mode(void)
{
#if IS_ENABLED(CONFIG_HAVE_SMI_HANDLER)
	if (acpi_slp_type != 3) {
		printk(BIOS_DEBUG, "Disabling ACPI via APMC:\n");
		outb(APM_CNT_ACPI_DISABLE, APM_CNT);
		printk(BIOS_DEBUG, "done.\n");
	}
#endif /* CONFIG_HAVE_SMI_HANDLER */
}

static void lpc_init(struct device *dev)
{
	/* Legacy initialization */
	isa_dma_init();
	pch_rtc_init(dev);
	reg_script_run_on_dev(dev, pch_misc_init_script);

	/* Interrupt configuration */
	pch_enable_ioapic(dev);
	setup_i8259();
	i8259_configure_irq_trigger(9, 1);

	/* Initialize power management */
	pch_power_options(dev);

	pch_set_acpi_mode();
}

static void pch_lpc_add_mmio_resources(device_t dev)
{
	u32 reg;
	struct resource *res;
	const u32 default_decode_base = IO_APIC_ADDR;

	/*
	 * Just report all resources from IO-APIC base to 4GiB. Don't mark
	 * them reserved as that may upset the OS if this range is marked
	 * as reserved in the e820.
	 */
	res = new_resource(dev, OIC);
	res->base = default_decode_base;
	res->size = 0 - default_decode_base;
	res->flags = IORESOURCE_MEM | IORESOURCE_ASSIGNED | IORESOURCE_FIXED;

	/* RCBA */
	if (RCBA_BASE_ADDRESS < default_decode_base) {
		res = new_resource(dev, RCBA);
		res->base = RCBA_BASE_ADDRESS;
		res->size = 16 * 1024;
		res->flags = IORESOURCE_MEM | IORESOURCE_ASSIGNED |
			     IORESOURCE_FIXED | IORESOURCE_RESERVE;
	}

	/* Check LPC Memory Decode register. */
	reg = pci_read_config32(dev, LGMR);
	if (reg & 1) {
		reg &= ~0xffff;
		if (reg < default_decode_base) {
			res = new_resource(dev, LGMR);
			res->base = reg;
			res->size = 16 * 1024;
			res->flags = IORESOURCE_MEM | IORESOURCE_ASSIGNED |
				     IORESOURCE_FIXED | IORESOURCE_RESERVE;
		}
	}
}

/* Default IO range claimed by the LPC device. The upper bound is exclusive. */
#define LPC_DEFAULT_IO_RANGE_LOWER 0
#define LPC_DEFAULT_IO_RANGE_UPPER 0x1000

static inline int pch_io_range_in_default(u16 base, u16 size)
{
	/* Does it start above the range? */
	if (base >= LPC_DEFAULT_IO_RANGE_UPPER)
		return 0;

	/* Is it entirely contained? */
	if (base >= LPC_DEFAULT_IO_RANGE_LOWER &&
	    (base + size) < LPC_DEFAULT_IO_RANGE_UPPER)
		return 1;

	/* This will return not in range for partial overlaps. */
	return 0;
}

/*
 * Note: this function assumes there is no overlap with the default LPC device's
 * claimed range: LPC_DEFAULT_IO_RANGE_LOWER -> LPC_DEFAULT_IO_RANGE_UPPER.
 */
static void pch_lpc_add_io_resource(device_t dev, u16 base, u16 size, int index)
{
	struct resource *res;

	if (pch_io_range_in_default(base, size))
		return;

	res = new_resource(dev, index);
	res->base = base;
	res->size = size;
	res->flags = IORESOURCE_IO | IORESOURCE_ASSIGNED | IORESOURCE_FIXED;
}

static void pch_lpc_add_gen_io_resources(device_t dev, int reg_value, int index)
{
	/*
	 * Check if the register is enabled. If so and the base exceeds the
	 * device's deafult claim range add the resoure.
	 */
	if (reg_value & 1) {
		u16 base = reg_value & 0xfffc;
		u16 size = (0x3 | ((reg_value >> 16) & 0xfc)) + 1;
		pch_lpc_add_io_resource(dev, base, size, index);
	}
}

static void pch_lpc_add_io_resources(device_t dev)
{
	struct resource *res;
	config_t *config = dev->chip_info;

	/* Add the default claimed IO range for the LPC device. */
	res = new_resource(dev, 0);
	res->base = LPC_DEFAULT_IO_RANGE_LOWER;
	res->size = LPC_DEFAULT_IO_RANGE_UPPER - LPC_DEFAULT_IO_RANGE_LOWER;
	res->flags = IORESOURCE_IO | IORESOURCE_ASSIGNED | IORESOURCE_FIXED;

	/* GPIOBASE */
	pch_lpc_add_io_resource(dev, GPIO_BASE_ADDRESS,
				GPIO_BASE_SIZE, GPIO_BASE);

	/* PMBASE */
	pch_lpc_add_io_resource(dev, ACPI_BASE_ADDRESS, ACPI_BASE_SIZE, PMBASE);

	/* LPC Generic IO Decode range. */
	pch_lpc_add_gen_io_resources(dev, config->gen1_dec, LPC_GEN1_DEC);
	pch_lpc_add_gen_io_resources(dev, config->gen2_dec, LPC_GEN2_DEC);
	pch_lpc_add_gen_io_resources(dev, config->gen3_dec, LPC_GEN3_DEC);
	pch_lpc_add_gen_io_resources(dev, config->gen4_dec, LPC_GEN4_DEC);
}

static void pch_lpc_read_resources(device_t dev)
{
	global_nvs_t *gnvs;

	/* Get the normal PCI resources of this device. */
	pci_dev_read_resources(dev);

	/* Add non-standard MMIO resources. */
	pch_lpc_add_mmio_resources(dev);

	/* Add IO resources. */
	pch_lpc_add_io_resources(dev);

	/* Allocate ACPI NVS in CBMEM */
	gnvs = cbmem_add(CBMEM_ID_ACPI_GNVS, sizeof(global_nvs_t));
	if (acpi_slp_type != 3 && gnvs)
		memset(gnvs, 0, sizeof(global_nvs_t));
}

static struct device_operations device_ops = {
	.read_resources		= &pch_lpc_read_resources,
	.set_resources		= &pci_dev_set_resources,
	.enable_resources	= &pci_dev_enable_resources,
	.init			= &lpc_init,
	.scan_bus		= &scan_static_bus,
	.ops_pci		= &skylake_pci_ops,
};

static const unsigned short pci_device_ids[] = {
	PCH_SPT_LP,
	0
};

static const struct pci_driver pch_lpc __pci_driver = {
	.ops	 = &device_ops,
	.vendor	 = PCI_VENDOR_ID_INTEL,
	.devices = pci_device_ids,
};
