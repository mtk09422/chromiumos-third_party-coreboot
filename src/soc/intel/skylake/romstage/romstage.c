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
#include <elog.h>
#include <ramstage_cache.h>
#include <reset.h>
#include <romstage_handoff.h>
#include <soc/pei_wrapper.h>
#include <soc/pm.h>
#include <soc/romstage.h>
#include <soc/spi.h>
#include <timestamp.h>
#include <vendorcode/google/chromeos/chromeos.h>

/* SOC initialization before the console is enabled */
void soc_pre_console_init(struct romstage_params *params)
{
	/* System Agent Early Initialization */
	systemagent_early_init();
}

/* SOC initialization before RAM is enabled */
void soc_pre_ram_init(struct romstage_params *params)
{
	/* Prepare to initialize memory */
	skylake_fill_pei_data(params->pei_data);
}

void ramstage_cache_invalid(struct ramstage_cache *cache)
{
#if IS_ENABLED(CONFIG_RESET_ON_INVALID_RAMSTAGE_CACHE)
	/* Perform hard reset on invalid ramstage cache. */
	hard_reset();
#endif
}

#if IS_ENABLED(CONFIG_CHROMEOS)
int vboot_get_sw_write_protect(void)
{
	u8 status;

	/* Return unprotected status if status read fails. */
	return early_spi_read_wpsr(&status) ? 0 : !!(status & 0x80);
}
#endif

