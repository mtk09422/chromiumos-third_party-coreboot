/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2014 Google Inc.
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

#include <stddef.h>
#include <stdint.h>
#include <arch/cpu.h>
#include <arch/io.h>
#include <arch/cbfs.h>
#include <arch/stages.h>
#include <arch/early_variables.h>
#include <console/console.h>
#include <cbmem.h>
#include <cpu/x86/mtrr.h>
#include <ec/google/chromeec/ec.h>
#include <ec/google/chromeec/ec_commands.h>
#include <elog.h>
#include <ramstage_cache.h>
#include <romstage_handoff.h>
#include <timestamp.h>
#include <vendorcode/google/chromeos/chromeos.h>
#include <soc/me.h>
#include <soc/intel/common/mrc_cache.h>
#include <soc/pei_wrapper.h>
#include <soc/pm.h>
#include <soc/reset.h>
#include <soc/romstage.h>
#include <soc/spi.h>

/* Entry from cache-as-ram.inc. */
asmlinkage void *romstage_main(unsigned int bist,
				uint32_t tsc_low, uint32_t tsc_hi)
{
	void *top_of_stack;
	struct romstage_params rp = {
		.bist = bist,
		.pei_data = NULL,
	};

	post_code(0x30);

	timestamp_early_init((((uint64_t)tsc_hi) << 32) | (uint64_t)tsc_low);
	timestamp_add_now(TS_START_ROMSTAGE);

	/* System Agent Early Initialization */
	systemagent_early_init();

	/* PCH Early Initialization */
	pch_early_init();

	/* Call into mainboard pre console init. Needed to enable serial port
	   on IT8772 */
	mainboard_pre_console_init();

	/* Start console drivers */
	console_init();

	/* Display parameters */
	printk(BIOS_SPEW, "bist: 0x%08x\n", bist);
	printk(BIOS_SPEW, "tsc_low: 0x%08x\n", tsc_low);
	printk(BIOS_SPEW, "tsc_hi: 0x%08x\n", tsc_hi);
	printk(BIOS_SPEW, "CONFIG_MMCONF_BASE_ADDRESS: 0x%08x\n",
		 CONFIG_MMCONF_BASE_ADDRESS);
	printk(BIOS_INFO, "Using: %s\n",
		IS_ENABLED(CONFIG_PLATFORM_USES_FSP) ? "FSP" :
		(IS_ENABLED(CONFIG_HAVE_MRC) ? "MRC" :
		"No Memory Support"));

	/* Display FSP banner */
#if IS_ENABLED(CONFIG_PLATFORM_USES_FSP)
	printk(BIOS_DEBUG, "FSP TempRamInit successful\n");
	print_fsp_info(find_fsp());
#endif	/* CONFIG_PLATFORM_USES_FSP */

	/* Get power state */
	rp.power_state = fill_power_state();

	/* Print useful platform information */
	report_platform_info();

	/* Set CPU frequency to maximum */
	set_max_freq();

	/* Call into mainboard. */
	mainboard_romstage_entry(&rp);

	top_of_stack = setup_stack_and_mttrs();

#if IS_ENABLED(CONFIG_PLATFORM_USES_FSP)
	printk(BIOS_DEBUG, "Calling FspTempRamExit API\n");
#endif	/* CONFIG_PLATFORM_USES_FSP */

	return top_of_stack;
}

static inline void chromeos_init(int prev_sleep_state)
{
#if CONFIG_CHROMEOS
	/* Normalize the sleep state to what init_chromeos() wants for S3: 2 */
	init_chromeos(prev_sleep_state == SLEEP_STATE_S3 ? 2 : 0);
#endif
}

/* Entry from the mainboard. */
void romstage_common(struct romstage_params *params)
{
	struct romstage_handoff *handoff;
	struct pei_data *pei_data;

	post_code(0x32);

	timestamp_add_now(TS_BEFORE_INITRAM);

	pei_data = params->pei_data;
	pei_data->boot_mode = params->power_state->prev_sleep_state;

#if CONFIG_ELOG_BOOT_COUNT
	if (params->power_state->prev_sleep_state != SLEEP_STATE_S3)
		boot_count_increment();
#endif

	/* Print ME state before MRC */
	intel_me_status();

	/* Save ME HSIO version */
	intel_me_hsio_version(&params->power_state->hsio_version,
			      &params->power_state->hsio_checksum);

	/* Prepare to initialize memory */
	const struct mrc_saved_data *cache;
	struct memory_info *mem_info;

	broadwell_fill_pei_data(pei_data);

	if (recovery_mode_enabled()) {
		/* Recovery mode does not use MRC cache */
		printk(BIOS_DEBUG, "Recovery mode: not using MRC cache.\n");
	} else if (!mrc_cache_get_current(&cache)) {
		/* MRC cache found */
		pei_data->saved_data_size = cache->size;
		pei_data->saved_data = &cache->data[0];
	} else if (pei_data->boot_mode == SLEEP_STATE_S3) {
		/* Waking from S3 and no cache. */
		printk(BIOS_DEBUG, "No MRC cache found in S3 resume path.\n");
		post_code(POST_RESUME_FAILURE);
		reset_system();
	} else {
		printk(BIOS_DEBUG, "No MRC cache found.\n");
#if CONFIG_EC_GOOGLE_CHROMEEC
		if (pei_data->boot_mode == SLEEP_STATE_S0) {
			/* Ensure EC is running RO firmware. */
			google_chromeec_check_ec_image(EC_IMAGE_RO);
		}
#endif
	}

	/*
	 * Do not use saved pei data.  Can be set by mainboard romstage
	 * to force a full train of memory on every boot.
	 */
	if (pei_data->disable_saved_data) {
		printk(BIOS_DEBUG, "Disabling PEI saved data by request\n");
		pei_data->saved_data = NULL;
		pei_data->saved_data_size = 0;
	}

	/* Initialize RAM */
	raminit(params, pei_data);
	timestamp_add_now(TS_AFTER_INITRAM);

	printk(BIOS_DEBUG, "MRC data at %p %d bytes\n", pei_data->data_to_save,
	       pei_data->data_to_save_size);

	if (pei_data->data_to_save != NULL && pei_data->data_to_save_size > 0)
		mrc_cache_stash_data(pei_data->data_to_save,
				     pei_data->data_to_save_size);

	printk(BIOS_DEBUG, "create cbmem for dimm information\n");
	mem_info = cbmem_add(CBMEM_ID_MEMINFO, sizeof(struct memory_info));
	memcpy(mem_info, &pei_data->meminfo, sizeof(struct memory_info));

	handoff = romstage_handoff_find_or_add();
	if (handoff != NULL)
		handoff->s3_resume = (params->power_state->prev_sleep_state ==
				      SLEEP_STATE_S3);
	else
		printk(BIOS_DEBUG, "Romstage handoff structure not added!\n");

	chromeos_init(params->power_state->prev_sleep_state);
}

void asmlinkage romstage_after_car(void)
{
#if IS_ENABLED(CONFIG_PLATFORM_USES_FSP)
	FSP_INFO_HEADER *fsp_info_header;
	FSP_SILICON_INIT fsp_silicon_init;
	EFI_STATUS status;

	printk(BIOS_DEBUG, "FspTempRamExit returned successfully\n");
#endif	/* CONFIG_PLATFORM_USES_FSP */

	timestamp_add_now(TS_END_ROMSTAGE);

#if IS_ENABLED(CONFIG_PLATFORM_USES_FSP)
	printk(BIOS_DEBUG, "Calling FspSiliconInit\n");
	fsp_info_header = find_fsp();
	fsp_silicon_init = (FSP_SILICON_INIT)(fsp_info_header->ImageBase
		+ fsp_info_header->FspSiliconInitEntryOffset);
	status = fsp_silicon_init(NULL);
	printk(BIOS_DEBUG, "FspSiliconInit returned 0x%08x\n", status);
#endif	/* CONFIG_PLATFORM_USES_FSP */

#if IS_ENABLED(CONFIG_PLATFORM_USES_FSP)
/* TODO: Remove this code.  Temporary code to hang after FSP TempRamInit API */
	printk(BIOS_ERR, "Hanging in romstage_after_car!\n");
	post_code(0x35);
	while (1)
		;
#endif	/* CONFIG_PLATFORM_USES_FSP */

	/* Run vboot verification if configured. */
	vboot_verify_firmware(romstage_handoff_find_or_add());

	/* Load the ramstage. */
	copy_and_run();
	while (1);
}

void ramstage_cache_invalid(struct ramstage_cache *cache)
{
#if CONFIG_RESET_ON_INVALID_RAMSTAGE_CACHE
	/* Perform cold reset on invalid ramstage cache. */
	reset_system();
#endif
}

#if CONFIG_CHROMEOS
int vboot_get_sw_write_protect(void)
{
	u8 status;
	/* Return unprotected status if status read fails. */
	return (early_spi_read_wpsr(&status) ? 0 : !!(status & 0x80));
}

void __attribute__((weak)) mainboard_pre_console_init(void) {}
#endif
