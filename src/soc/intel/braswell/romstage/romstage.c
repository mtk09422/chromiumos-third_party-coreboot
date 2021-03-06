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

#include <cbmem.h>
#include <stddef.h>
#include <arch/early_variables.h>
#include <arch/cpu.h>
#include <arch/io.h>
#include <arch/cbfs.h>
#include <arch/stages.h>
#include <cpu/x86/mtrr.h>
#include <console/console.h>
#if IS_ENABLED(CONFIG_EC_GOOGLE_CHROMEEC)
#include <ec/google/chromeec/ec.h>
#include <ec/google/chromeec/ec_commands.h>
#endif
#include <elog.h>
#include <romstage_handoff.h>
#include <timestamp.h>
#include <ramstage_cache.h>
#include <reset.h>
#include <vendorcode/google/chromeos/chromeos.h>
#include <fsp_util.h>
#include <soc/intel/common/mrc_cache.h>

#include <soc/gpio.h>
#include <soc/iomap.h>
#include <soc/iosf.h>
#include <soc/lpc.h>
#include <soc/pci_devs.h>
#include <soc/romstage.h>
#include <soc/smm.h>
#include <soc/spi.h>

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

static void spi_init(void)
{
	void *scs = (void *)(SPI_BASE_ADDRESS + SCS);
	void *bcr = (void *)(SPI_BASE_ADDRESS + BCR);
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

void migrate_power_state(void)
{
	struct chipset_power_state *ps_cbmem;
	struct chipset_power_state *ps_car;

	ps_car = car_get_var_ptr(&power_state);
	ps_cbmem = cbmem_add(CBMEM_ID_POWER_STATE, sizeof(*ps_cbmem));

	if (ps_cbmem == NULL) {
		printk(BIOS_DEBUG, "Not adding power state to cbmem!\n");
		return;
	}
	memcpy(ps_cbmem, ps_car, sizeof(*ps_cbmem));
}
ROMSTAGE_CBMEM_INIT_HOOK(migrate_power_state);

struct chipset_power_state *fill_power_state(void)
{
	struct chipset_power_state *ps = car_get_var_ptr(&power_state);

	ps->pm1_sts = inw(ACPI_BASE_ADDRESS + PM1_STS);
	ps->pm1_en = inw(ACPI_BASE_ADDRESS + PM1_EN);
	ps->pm1_cnt = inl(ACPI_BASE_ADDRESS + PM1_CNT);
	ps->gpe0_sts = inl(ACPI_BASE_ADDRESS + GPE0_STS);
	ps->gpe0_en = inl(ACPI_BASE_ADDRESS + GPE0_EN);
	ps->tco_sts = inl(ACPI_BASE_ADDRESS + TCO_STS);
	ps->prsts = read32((void *)(PMC_BASE_ADDRESS + PRSTS));
	ps->gen_pmcon1 = read32((void *)(PMC_BASE_ADDRESS + GEN_PMCON1));
	ps->gen_pmcon2 = read32((void *)(PMC_BASE_ADDRESS + GEN_PMCON2));

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
	#if IS_ENABLED(CONFIG_HAVE_ACPI_RESUME)
		case SLP_TYP_S3:
			prev_sleep_state = SLEEP_STATE_S3;
			break;
	#endif
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
#if IS_ENABLED(CONFIG_RESET_ON_INVALID_RAMSTAGE_CACHE)
	/* Perform cold reset on invalid ramstage cache. */
	hard_reset();
#endif
}

/* SOC initialization before the console is enabled */
void soc_pre_console_init(struct romstage_params *params)
{
	/* Early chipset initialization */
	program_base_addresses();
	tco_disable();
}

/* SOC initialization after console is enabled */
void soc_romstage_init(struct romstage_params *params)
{
	/* Continue chipset initialization */
	spi_init();
	gfx_init();

#if IS_ENABLED(CONFIG_EC_GOOGLE_CHROMEEC)
	/* Ensure the EC is in the right mode for recovery */
	google_chromeec_early_init();
#endif
}

/* SOC initialization after RAM is enabled */
void soc_after_ram_init(struct romstage_params *params)
{
	u32 value;

	/* Make sure that E0000 and F0000 are RAM */
	printk(BIOS_DEBUG, "Disable ROM shadow below 1MB.\n");
	value = iosf_bunit_read(BUNIT_BMISC);
	value |= 3;
	iosf_bunit_write(BUNIT_BMISC, value);
}
