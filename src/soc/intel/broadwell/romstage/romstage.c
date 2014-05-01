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

#include <stdint.h>
#include <string.h>
#include <cbmem.h>
#include <console/console.h>
#include <arch/cpu.h>
#include <cpu/x86/bist.h>
#include <cpu/x86/msr.h>
#include <cpu/x86/mtrr.h>
#include <cpu/x86/stack.h>
#include <lib.h>
#include <timestamp.h>
#include <arch/io.h>
#include <arch/stages.h>
#include <device/pci_def.h>
#include <cpu/x86/lapic.h>
#include <cbfs.h>
#include <ramstage_cache.h>
#include <romstage_handoff.h>
#include <reset.h>
#include <vendorcode/google/chromeos/chromeos.h>
#if CONFIG_EC_GOOGLE_CHROMEEC
#include <ec/google/chromeec/ec.h>
#endif
#include "haswell.h"
#include "northbridge/intel/haswell/haswell.h"
#include "northbridge/intel/haswell/raminit.h"
#include "southbridge/intel/lynxpoint/pch.h"
#include "southbridge/intel/lynxpoint/me.h"


static inline void reset_system(void)
{
	hard_reset();
	while (1) {
		hlt();
	}
}

/* The cache-as-ram assembly file calls romstage_main() after setting up
 * cache-as-ram.  romstage_main() will then call the mainboards's
 * mainboard_romstage_entry() function. That function then calls
 * romstage_common() below. The reason for the back and forth is to provide
 * common entry point from cache-as-ram while still allowing for code sharing.
 * Because we can't use global variables the stack is used for allocations --
 * thus the need to call back and forth. */

void * asmlinkage romstage_main(unsigned long bist)
{
	int i;
	void *romstage_stack_after_car;
	const int num_guards = 4;
	const u32 stack_guard = 0xdeadbeef;
	u32 *stack_base = (void *)(CONFIG_DCACHE_RAM_BASE +
	                           CONFIG_DCACHE_RAM_SIZE -
	                           CONFIG_DCACHE_RAM_ROMSTAGE_STACK_SIZE);

	printk(BIOS_DEBUG, "Setting up stack guards.\n");
	for (i = 0; i < num_guards; i++)
		stack_base[i] = stack_guard;

	mainboard_romstage_entry(bist);

	/* Check the stack. */
	for (i = 0; i < num_guards; i++) {
		if (stack_base[i] == stack_guard)
			continue;
		printk(BIOS_DEBUG, "Smashed stack detected in romstage!\n");
	}

	/* Get the stack to use after cache-as-ram is torn down. */
	romstage_stack_after_car = setup_romstage_stack_after_car();

	return romstage_stack_after_car;
}

void romstage_common(const struct romstage_params *params)
{
	int boot_mode;
	int wake_from_s3;
	struct romstage_handoff *handoff;

#if CONFIG_COLLECT_TIMESTAMPS
	uint64_t start_romstage_time;
	uint64_t before_dram_time;
	uint64_t after_dram_time;
	uint64_t base_time =
		(uint64_t)pci_read_config32(PCI_DEV(0, 0x1f, 2), 0xd0) << 32 ||
		pci_read_config32(PCI_DEV(0, 0x00, 0), 0xdc);
#endif

#if CONFIG_COLLECT_TIMESTAMPS
	start_romstage_time = timestamp_get();
#endif

	if (params->bist == 0)
		enable_lapic();

	wake_from_s3 = early_pch_init(params->gpio_map, params->rcba_config);

#if CONFIG_EC_GOOGLE_CHROMEEC
	/* Ensure the EC is in the right mode for recovery */
	google_chromeec_early_init();
#endif

	/* Halt if there was a built in self test failure */
	report_bist_failure(params->bist);

	/* Perform some early chipset initialization required
	 * before RAM initialization can work
	 */
	haswell_early_initialization(HASWELL_MOBILE);
	printk(BIOS_DEBUG, "Back from haswell_early_initialization()\n");

	if (wake_from_s3) {
#if CONFIG_HAVE_ACPI_RESUME
		printk(BIOS_DEBUG, "Resume from S3 detected.\n");
#else
		printk(BIOS_DEBUG, "Resume from S3 detected, but disabled.\n");
		wake_from_s3 = 0;
#endif
	}

	/* There are hard coded assumptions of 2 meaning s3 wake. Normalize
	 * the users of the 2 literal here based off wake_from_s3. */
	boot_mode = wake_from_s3 ? 2 : 0;

	/* Prepare USB controller early in S3 resume */
	if (wake_from_s3)
		enable_usb_bar();

	post_code(0x3a);
	params->pei_data->boot_mode = boot_mode;
#if CONFIG_COLLECT_TIMESTAMPS
	before_dram_time = timestamp_get();
#endif

	report_platform_info();

	if (params->copy_spd != NULL)
		params->copy_spd(params->pei_data);

	sdram_initialize(params->pei_data);

#if CONFIG_COLLECT_TIMESTAMPS
	after_dram_time = timestamp_get();
#endif
	post_code(0x3b);

	intel_early_me_status();

	quick_ram_check();
	post_code(0x3e);

	if (!wake_from_s3) {
		cbmem_initialize_empty();
		/* Save data returned from MRC on non-S3 resumes. */
		save_mrc_data(params->pei_data);
	} else if (cbmem_initialize()) {
	#if CONFIG_HAVE_ACPI_RESUME
		/* Failed S3 resume, reset to come up cleanly */
		reset_system();
	#endif
	}

	handoff = romstage_handoff_find_or_add();
	if (handoff != NULL)
		handoff->s3_resume = wake_from_s3;
	else
		printk(BIOS_DEBUG, "Romstage handoff structure not added!\n");

	post_code(0x3f);
#if CONFIG_CHROMEOS
	init_chromeos(boot_mode);
#endif
#if CONFIG_COLLECT_TIMESTAMPS
	timestamp_init(base_time);
	timestamp_add(TS_START_ROMSTAGE, start_romstage_time );
	timestamp_add(TS_BEFORE_INITRAM, before_dram_time );
	timestamp_add(TS_AFTER_INITRAM, after_dram_time );
	timestamp_add_now(TS_END_ROMSTAGE);
#endif
}

static inline void prepare_for_resume(struct romstage_handoff *handoff)
{
/* Only need to save memory when ramstage isn't relocatable. */
#if !CONFIG_RELOCATABLE_RAMSTAGE
#if CONFIG_HAVE_ACPI_RESUME
	/* Back up the OS-controlled memory where ramstage will be loaded. */
	if (handoff != NULL && handoff->s3_resume) {
		void *src = (void *)CONFIG_RAMBASE;
		void *dest = cbmem_find(CBMEM_ID_RESUME);
		if (dest != NULL)
			memcpy(dest, src, HIGH_MEMORY_SAVE);
	}
#endif
#endif
}

void romstage_after_car(void)
{
	struct romstage_handoff *handoff;

	handoff = romstage_handoff_find_or_add();

	prepare_for_resume(handoff);

	vboot_verify_firmware(handoff);

	/* Load the ramstage. */
	copy_and_run();
}


#if CONFIG_RELOCATABLE_RAMSTAGE
#include <ramstage_cache.h>

struct ramstage_cache *ramstage_cache_location(long *size)
{
	/* The ramstage cache lives in the TSEG region at RESERVED_SMM_OFFSET.
	 * The top of ram is defined to be the TSEG base address. */
	*size = RESERVED_SMM_SIZE;
	return (void *)(get_top_of_ram() + RESERVED_SMM_OFFSET);
}

void ramstage_cache_invalid(struct ramstage_cache *cache)
{
#if CONFIG_RESET_ON_INVALID_RAMSTAGE_CACHE
	reset_system();
#endif
}
#endif
