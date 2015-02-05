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

#include <console/console.h>
#include <ramstage_cache.h>
#include <romstage_handoff.h>
#include <soc/me.h>
#include <soc/pei_wrapper.h>
#include <soc/romstage.h>
#include <timestamp.h>

#include <rmodule.h>
int rmodule_stage_load_from_cbfs(struct rmod_stage_load *rsl)
{
	return 0;
}

/* SOC initialization before the console is enabled */
void soc_pre_console_init(struct romstage_params *params)
{
	/* System Agent Early Initialization */
	systemagent_early_init();
}

/* SOC initialization before RAM is enabled */
void soc_pre_ram_init(struct romstage_params *params)
{
	/* Print ME state before MRC */
	intel_me_status();

	/* Save ME HSIO version */
	intel_me_hsio_version(&params->power_state->hsio_version,
			      &params->power_state->hsio_checksum);

	/* Prepare to initialize memory */
	skylake_fill_pei_data(params->pei_data);
}

/* SOC initialization after temporary RAM is disabled */
void soc_after_temp_ram_exit(void)
{
	post_code(0x35);
	die("Hang in soc_after_temp_ram_exit!\n");
}

void ramstage_cache_invalid(struct ramstage_cache *cache)
{
}
