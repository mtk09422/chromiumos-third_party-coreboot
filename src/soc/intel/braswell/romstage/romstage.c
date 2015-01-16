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

#include <soc/gpio.h>
#include <soc/iomap.h>
#include <soc/lpc.h>
#include <soc/pci_devs.h>
#include <soc/pmc.h>
#include <soc/reset.h>
#include <soc/romstage.h>
#include <soc/smm.h>
#include <soc/spi.h>
#include <console/console.h>
#include <fsp_util.h>
#include <rmodule.h>
#include <ramstage_cache.h>
#include <timestamp.h>
#include <vendorcode/google/chromeos/chromeos.h>

int get_recovery_mode_from_vbnv(void)
{
	return 0;
}

int rmodule_stage_load_from_cbfs(struct rmod_stage_load *rsl)
{
	return 0;
}

void program_base_addresses(void)
{
	uint32_t reg;
	const uint32_t lpc_dev = PCI_DEV(0, LPC_DEV, LPC_FUNC);

	/* Memory Mapped IO registers. */
	reg = PMC_BASE_ADDRESS | 2;
	pci_write_config32(lpc_dev, PBASE, reg);
	reg = IO_BASE_ADDRESS | 2;
	pci_write_config32(lpc_dev, IOBASE, reg);
	reg = ILB_BASE_ADDRESS | 2;
	pci_write_config32(lpc_dev, IBASE, reg);
	reg = SPI_BASE_ADDRESS | 2;
	pci_write_config32(lpc_dev, SBASE, reg);
	reg = MPHY_BASE_ADDRESS | 2;
	pci_write_config32(lpc_dev, MPBASE, reg);
	reg = PUNIT_BASE_ADDRESS | 2;
	pci_write_config32(lpc_dev, PUBASE, reg);
	reg = RCBA_BASE_ADDRESS | 1;
	pci_write_config32(lpc_dev, RCBA, reg);

	/* IO Port Registers. */
	reg = ACPI_BASE_ADDRESS | 2;
	pci_write_config32(lpc_dev, ABASE, reg);
	reg = GPIO_BASE_ADDRESS | 2;
	pci_write_config32(lpc_dev, GBASE, reg);
}

void ramstage_cache_invalid(struct ramstage_cache *cache)
{
}

/*
 * The cache-as-ram assembly file calls romstage_main() after setting up
 * cache-as-ram.  romstage_main() will then call the mainboards's
 * mainboard_romstage_entry() function. That function then calls
 * romstage_common() below. The reason for the back and forth is to provide
 * common entry point from cache-as-ram while still allowing for code sharing.
 * Because we can't use global variables the stack is used for allocations --
 * thus the need to call back and forth.
 */


/* Entry from cache-as-ram.inc. */
asmlinkage void *romstage_main(unsigned int bist,
			      uint32_t tsc_low, uint32_t tsc_high)
{
	post_code(0x30);

	/* Save timestamp information */
	timestamp_early_init((((uint64_t)tsc_high) << 32) | (uint64_t)tsc_low);
	timestamp_add_now(TS_START_ROMSTAGE);

	/* Early chipset initialization */
	program_base_addresses();
	tco_disable();
	config_com1_and_enable();
	console_init();

	/* Display parameters */
	printk(BIOS_SPEW, "bist: 0x%08x\n", bist);
	printk(BIOS_SPEW, "tsc_low: 0x%08x\n", tsc_low);
	printk(BIOS_SPEW, "tsc_hi: 0x%08x\n", tsc_high);
	printk(BIOS_SPEW, "CONFIG_MMCONF_BASE_ADDRESS: 0x%08x\n",
		 CONFIG_MMCONF_BASE_ADDRESS);
	printk(BIOS_INFO, "Using: %s\n",
		IS_ENABLED(CONFIG_PLATFORM_USES_FSP) ? "FSP" :
		(IS_ENABLED(CONFIG_HAVE_MRC) ? "MRC" :
		"No Memory Support"));

	/* Display FSP banner */
	printk(BIOS_DEBUG, "FSP TempRamInit successful\n");
	print_fsp_info(find_fsp());

	printk(BIOS_ERR, "Hanging in romstage_main!\n");
	post_code(0x35);
	while (1)
		;

	return NULL;
}
