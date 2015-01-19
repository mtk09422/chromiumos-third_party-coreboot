/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2009 coresystems GmbH
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

#include <arch/acpi.h>
#include <arch/acpigen.h>
#include <arch/cpu.h>
#include <arch/io.h>
#include <arch/smp/mpspec.h>
#include <cbmem.h>
#include <console/console.h>
#include <cpu/intel/turbo.h>
#include <cpu/x86/msr.h>
#include <cpu/x86/smm.h>
#include <cpu/x86/tsc.h>
#include <ec/google/chromeec/ec.h>
#include <soc/acpi.h>
#include <soc/iomap.h>
#include <soc/irq.h>
#include <soc/msr.h>
#include <soc/pattrs.h>
#include <soc/pm.h>
#include <string.h>
#include <types.h>
#include <vendorcode/google/chromeos/gnvs.h>


void acpi_init_gnvs(global_nvs_t *gnvs)
{
	/* Set unknown wake source */
	gnvs->pm1i = -1;

	/* CPU core count */
	gnvs->pcnt = dev_count_cpu();

	/* Top of Low Memory (start of resource allocation) */
	gnvs->tolm = nc_read_top_of_low_memory();

#if IS_ENABLED(CONFIG_CONSOLE_CBMEM)
	/* Update the mem console pointer. */
	gnvs->cbmc = (u32)cbmem_find(CBMEM_ID_CONSOLE);
#endif

#if IS_ENABLED(CONFIG_CHROMEOS)
	/* Initialize Verified Boot data */
	chromeos_init_vboot(&(gnvs->chromeos));
#if IS_ENABLED(CONFIG_EC_GOOGLE_CHROMEEC)
	gnvs->chromeos.vbt2 = google_ec_running_ro() ?
		ACTIVE_ECFW_RO : ACTIVE_ECFW_RW;
#endif
#endif
}

static int acpi_sci_irq(void)
{
	void *actl = (void *)(ILB_BASE_ADDRESS + ACTL);
	int scis;
	static int sci_irq;

	if (sci_irq)
		return sci_irq;

	/* Determine how SCI is routed. */
	scis = read32(actl) & SCIS_MASK;
	switch (scis) {
	case SCIS_IRQ9:
	case SCIS_IRQ10:
	case SCIS_IRQ11:
		sci_irq = scis - SCIS_IRQ9 + 9;
		break;
	case SCIS_IRQ20:
	case SCIS_IRQ21:
	case SCIS_IRQ22:
	case SCIS_IRQ23:
		sci_irq = scis - SCIS_IRQ20 + 20;
		break;
	default:
		printk(BIOS_DEBUG, "Invalid SCI route! Defaulting to IRQ9.\n");
		sci_irq = 9;
		break;
	}

	printk(BIOS_DEBUG, "SCI is IRQ%d\n", sci_irq);
	return sci_irq;
}

unsigned long acpi_fill_mcfg(unsigned long current)
{
	current += acpi_create_mcfg_mmconfig((acpi_mcfg_mmconfig_t *)current,
					     MCFG_BASE_ADDRESS, 0, 0, 255);
	return current;
}

void acpi_fill_in_fadt(acpi_fadt_t *fadt)
{
	const uint16_t pmbase = ACPI_BASE_ADDRESS;

	fadt->sci_int = acpi_sci_irq();
	fadt->smi_cmd = APM_CNT;
	fadt->acpi_enable = APM_CNT_ACPI_ENABLE;
	fadt->acpi_disable = APM_CNT_ACPI_DISABLE;
	fadt->s4bios_req = 0x0;
	fadt->pstate_cnt = 0;

	fadt->pm1a_evt_blk = pmbase + PM1_STS;
	fadt->pm1b_evt_blk = 0x0;
	fadt->pm1a_cnt_blk = pmbase + PM1_CNT;
	fadt->pm1b_cnt_blk = 0x0;
	fadt->pm2_cnt_blk = pmbase + PM2A_CNT_BLK;
	fadt->pm_tmr_blk = pmbase + PM1_TMR;
	fadt->gpe0_blk = pmbase + GPE0_STS;
	fadt->gpe1_blk = 0;

	fadt->pm1_evt_len = 4;
	fadt->pm1_cnt_len = 2;
	fadt->pm2_cnt_len = 1;
	fadt->pm_tmr_len = 4;
	fadt->gpe0_blk_len = 2 * (GPE0_EN - GPE0_STS);
	fadt->gpe1_blk_len = 0;
	fadt->gpe1_base = 0;
	fadt->cst_cnt = 0;
	fadt->p_lvl2_lat = 1;
	fadt->p_lvl3_lat = 87;
	fadt->flush_size = 1024;
	fadt->flush_stride = 16;
	fadt->duty_offset = 1;
	fadt->duty_width = 0;
	fadt->day_alrm = 0xd;
	fadt->mon_alrm = 0x00;
	fadt->century = 0x00;
	fadt->iapc_boot_arch = ACPI_FADT_LEGACY_DEVICES | ACPI_FADT_8042;

	fadt->flags = ACPI_FADT_WBINVD | ACPI_FADT_C1_SUPPORTED |
			ACPI_FADT_C2_MP_SUPPORTED | ACPI_FADT_SLEEP_BUTTON |
			ACPI_FADT_RESET_REGISTER | ACPI_FADT_SEALED_CASE |
			ACPI_FADT_S4_RTC_WAKE | ACPI_FADT_PLATFORM_CLOCK;

	fadt->reset_reg.space_id = 1;
	fadt->reset_reg.bit_width = 8;
	fadt->reset_reg.bit_offset = 0;
	fadt->reset_reg.resv = 0;
	fadt->reset_reg.addrl = 0xcf9;
	fadt->reset_reg.addrh = 0;
	fadt->reset_value = 6;

	fadt->x_pm1a_evt_blk.space_id = 1;
	fadt->x_pm1a_evt_blk.bit_width = fadt->pm1_evt_len * 8;
	fadt->x_pm1a_evt_blk.bit_offset = 0;
	fadt->x_pm1a_evt_blk.resv = 0;
	fadt->x_pm1a_evt_blk.addrl = pmbase + PM1_STS;
	fadt->x_pm1a_evt_blk.addrh = 0x0;

	fadt->x_pm1b_evt_blk.space_id = 1;
	fadt->x_pm1b_evt_blk.bit_width = 0;
	fadt->x_pm1b_evt_blk.bit_offset = 0;
	fadt->x_pm1b_evt_blk.resv = 0;
	fadt->x_pm1b_evt_blk.addrl = 0x0;
	fadt->x_pm1b_evt_blk.addrh = 0x0;

	fadt->x_pm1a_cnt_blk.space_id = 1;
	fadt->x_pm1a_cnt_blk.bit_width = fadt->pm1_cnt_len * 8;
	fadt->x_pm1a_cnt_blk.bit_offset = 0;
	fadt->x_pm1a_cnt_blk.resv = 0;
	fadt->x_pm1a_cnt_blk.addrl = pmbase + PM1_CNT;
	fadt->x_pm1a_cnt_blk.addrh = 0x0;

	fadt->x_pm1b_cnt_blk.space_id = 1;
	fadt->x_pm1b_cnt_blk.bit_width = 0;
	fadt->x_pm1b_cnt_blk.bit_offset = 0;
	fadt->x_pm1b_cnt_blk.resv = 0;
	fadt->x_pm1b_cnt_blk.addrl = 0x0;
	fadt->x_pm1b_cnt_blk.addrh = 0x0;

	fadt->x_pm2_cnt_blk.space_id = 1;
	fadt->x_pm2_cnt_blk.bit_width = fadt->pm2_cnt_len * 8;
	fadt->x_pm2_cnt_blk.bit_offset = 0;
	fadt->x_pm2_cnt_blk.resv = 0;
	fadt->x_pm2_cnt_blk.addrl = pmbase + PM2A_CNT_BLK;
	fadt->x_pm2_cnt_blk.addrh = 0x0;

	fadt->x_pm_tmr_blk.space_id = 1;
	fadt->x_pm_tmr_blk.bit_width = fadt->pm_tmr_len * 8;
	fadt->x_pm_tmr_blk.bit_offset = 0;
	fadt->x_pm_tmr_blk.resv = 0;
	fadt->x_pm_tmr_blk.addrl = pmbase + PM1_TMR;
	fadt->x_pm_tmr_blk.addrh = 0x0;

	fadt->x_gpe0_blk.space_id = 1;
	fadt->x_gpe0_blk.bit_width = fadt->gpe0_blk_len * 8;
	fadt->x_gpe0_blk.bit_offset = 0;
	fadt->x_gpe0_blk.resv = 0;
	fadt->x_gpe0_blk.addrl = pmbase + GPE0_STS;
	fadt->x_gpe0_blk.addrh = 0x0;

	fadt->x_gpe1_blk.space_id = 1;
	fadt->x_gpe1_blk.bit_width = 0;
	fadt->x_gpe1_blk.bit_offset = 0;
	fadt->x_gpe1_blk.resv = 0;
	fadt->x_gpe1_blk.addrl = 0x0;
	fadt->x_gpe1_blk.addrh = 0x0;
}

void generate_cpu_entries(void)
{
	int len_pr, core;
	int pcontrol_blk = get_pmbase(), plen = 6;
	const struct pattrs *pattrs = pattrs_get();

	for (core = 0; core < pattrs->num_cpus; core++) {
		if (core > 0) {
			pcontrol_blk = 0;
			plen = 0;
		}

		/* Generate processor \_PR.CPUx */
		len_pr = acpigen_write_processor(
			core, pcontrol_blk, plen);

		len_pr--;
		acpigen_patch_len(len_pr);
	}
}

unsigned long acpi_madt_irq_overrides(unsigned long current)
{
	int sci_irq = acpi_sci_irq();
	acpi_madt_irqoverride_t *irqovr;
	uint16_t sci_flags = MP_IRQ_TRIGGER_LEVEL;

	/* INT_SRC_OVR */
	irqovr = (void *)current;
	current += acpi_create_madt_irqoverride(irqovr, 0, 0, 2, 0);

	if (sci_irq >= 20)
		sci_flags |= MP_IRQ_POLARITY_LOW;
	else
		sci_flags |= MP_IRQ_POLARITY_HIGH;

	irqovr = (void *)current;
	current += acpi_create_madt_irqoverride(irqovr, 0, sci_irq, sci_irq,
						sci_flags);

	return current;
}

#if CONFIG_GOP_SUPPORT
/* Initialize IGD OpRegion, called from ACPI code */
int init_igd_opregion(igd_opregion_t *opregion)
{
	return 0;
}
#endif
