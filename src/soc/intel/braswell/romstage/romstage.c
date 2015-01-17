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

#include <arch/early_variables.h>
#include <arch/cpu.h>
#include <arch/hlt.h>
#include <soc/intel/common/mrc_cache.h>

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

void spi_init(void)
{
	const unsigned long scs = SPI_BASE_ADDRESS + SCS;
	const unsigned long bcr = SPI_BASE_ADDRESS + BCR;
	uint32_t reg;

	/* Disable generating SMI when setting WPD bit. */
	write32(scs, read32(scs) & ~SMIWPEN);
	/*
	 * Enable caching and prefetching in the SPI controller. Disable
	 * the SMM-only BIOS write and set WPD bit.
	 */
	reg = (read32(bcr) & ~SRC_MASK) | SRC_CACHE_PREFETCH | BCR_WPD;
	reg &= ~EISS;
	write32(bcr, reg);
}

static struct chipset_power_state power_state CAR_GLOBAL;

struct chipset_power_state *fill_power_state(void)
{
	struct chipset_power_state *ps = car_get_var_ptr(&power_state);

	ps->pm1_sts = inw(ACPI_BASE_ADDRESS + PM1_STS);
	ps->pm1_en = inw(ACPI_BASE_ADDRESS + PM1_EN);
	ps->pm1_cnt = inl(ACPI_BASE_ADDRESS + PM1_CNT);
	ps->gpe0_sts = inl(ACPI_BASE_ADDRESS + GPE0_STS);
	ps->gpe0_en = inl(ACPI_BASE_ADDRESS + GPE0_EN);
	ps->tco_sts = inl(ACPI_BASE_ADDRESS + TCO_STS);
	ps->prsts = read32(PMC_BASE_ADDRESS + PRSTS);
	ps->gen_pmcon1 = read32(PMC_BASE_ADDRESS + GEN_PMCON1);
	ps->gen_pmcon2 = read32(PMC_BASE_ADDRESS + GEN_PMCON2);

	ps->prev_sleep_state = chipset_prev_sleep_state(ps);

	return ps;
}

/* Return 0, 3, or 5 to indicate the previous sleep state. */
int chipset_prev_sleep_state(struct chipset_power_state *ps)
{
	/* Default to S0. */
	int prev_sleep_state = SLEEP_STATE_S0;

	if (ps->pm1_sts & WAK_STS) {
		switch ((ps->pm1_cnt & SLP_TYP) >> SLP_TYP_SHIFT) {
		case SLP_TYP_S5:
			prev_sleep_state = SLEEP_STATE_S5;
			break;
		}

		/* Clear SLP_TYP. */
		outl(ps->pm1_cnt & ~(SLP_TYP), ACPI_BASE_ADDRESS + PM1_CNT);
	}

	if (ps->gen_pmcon1 & (PWR_FLR | SUS_PWR_FLR))
		prev_sleep_state = SLEEP_STATE_S5;

	return prev_sleep_state;
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
	struct romstage_params rp = {
		.bist = bist,
		.power_state = NULL,
		.pei_data = NULL,
	};

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

	/* Continue chipset initialization */
	spi_init();
	set_max_freq();
	gfx_init();

#if IS_ENABLED(CONFIG_EC_GOOGLE_CHROMEEC)
	/* Ensure the EC is in the right mode for recovery */
	google_chromeec_early_init();
#endif

	/* Get power management information. */
	rp.power_state = fill_power_state();

	/* Call into mainboard. */
	mainboard_romstage_entry(&rp);

	printk(BIOS_ERR, "Hanging in romstage_main!\n");
	post_code(0x35);
	while (1)
		;

	return NULL;
}

void romstage_common(struct romstage_params *rp)
{
	timestamp_add_now(TS_BEFORE_INITRAM);

	/* Fill in any extra pei_data fields */
	rp->pei_data->boot_mode = rp->power_state->prev_sleep_state;

#if IS_ENABLED(CONFIG_ELOG_BOOT_COUNT)
	boot_count_increment();
#endif

	/* Check recovery and MRC cache */
	rp->pei_data->saved_data_size = 0;
	rp->pei_data->saved_data = NULL;
	if (!rp->pei_data->disable_saved_data) {
		if (recovery_mode_enabled()) {
			/* Recovery mode does not use MRC cache */
			printk(BIOS_DEBUG,
			       "Recovery mode: not using MRC cache.\n");
		} else {
			printk(BIOS_DEBUG, "No MRC cache found.\n");
#if IS_ENABLED(CONFIG_EC_GOOGLE_CHROMEEC)
			if (rp->pei_data->boot_mode == SLEEP_STATE_S0) {
				/* Ensure EC is running RO firmware. */
				google_chromeec_check_ec_image(EC_IMAGE_RO);
			}
#endif
		}
	}

	/* Perform RAM initialization */
	raminit(rp, rp->pei_data);
	timestamp_add_now(TS_AFTER_INITRAM);
}
