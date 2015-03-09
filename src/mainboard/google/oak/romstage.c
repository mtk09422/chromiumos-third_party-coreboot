/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 Google Inc.
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
#include <arch/cache.h>
#include <arch/cpu.h>

#include <arch/stages.h>
#include <arch/io.h>
#include <arch/mmu.h>

#include <cbfs.h>
#include <cbmem.h>
#include <console/console.h>
#include <symbols.h>
#include <timestamp.h>
#include <delay.h>
#include <romstage_handoff.h>
#include <vendorcode/google/chromeos/chromeos.h>

#include <soc/addressmap.h>
#include <soc/pll.h>
#include <soc/i2c.h>
#include <soc/mt8173.h>
#include <soc/i2c.h>
#include <soc/wdt.h>

void main(void)
{
	void *entry = NULL;
	timestamp_add_now(TS_START_ROMSTAGE);

	mt_pll_post_init();

	/* post init pll */
	mt_pll_post_init();
	mt_arm_pll_sel();

	/* Setup TPM */
	mtk_i2c_init(0x11009000, 2, 0, 0x20, 0);

	/* init watch dog, will disable AP watch dog */
	mtk_wdt_init();

	/* init uart baudrate when pll on */
	console_init();

	trustzone_region_init();

	/* should be called after memory init */
	cbmem_initialize_empty();

#if IS_ENABLED(CONFIG_VBOOT2_VERIFY_FIRMWARE)
	entry = vboot2_load_ramstage();
#else
	entry = vboot_verify_firmware_get_entry(NULL);
#endif

	timestamp_add_now(TS_START_COPYRAM);

	if (entry == NULL)
		entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA,
			CONFIG_CBFS_PREFIX "/ramstage");

	timestamp_add_now(TS_END_COPYRAM);

	stage_exit(entry);
}
