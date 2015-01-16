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

#ifndef _BRASWELL_ROMSTAGE_H_
#define _BRASWELL_ROMSTAGE_H_

#if !defined(__PRE_RAM__)
#error "Don't include romstage.h from a ramstage compilation unit!"
#endif

#include <stdint.h>
#include <arch/cpu.h>
#include <soc/pmc.h>

/* FSP specific includes here. */
#include <fsp_util.h>

struct pei_data {
	/* Chip settings */
	void *spd_data_ch0;
	void *spd_data_ch1;
	uint8_t spd_ch0_config;
	uint8_t spd_ch1_config;
	uint8_t sdcard_mode;
	uint8_t emmc_mode;
	uint8_t enable_azalia;

	/* System state information */
	int boot_mode;

	/* Fast boot and S3 resume MRC data */
	int saved_data_size;
	const void *saved_data;
	int disable_saved_data;

	/* New save data from MRC */
	int data_to_save_size;
	void *data_to_save;

	struct mrc_params *mrc_params;
};

struct romstage_params {
	unsigned long bist;
	struct chipset_power_state *power_state;
	struct pei_data *pei_data;
};

asmlinkage void *romstage_main(unsigned int bist,
			       uint32_t tsc_low, uint32_t tsc_high);
void mainboard_romstage_entry(struct romstage_params *rp);
void romstage_common(struct romstage_params *rp);
void board_fsp_memory_init_params(
	struct romstage_params *params,
	FSP_INFO_HEADER *fsp_header,
	FSP_MEMORY_INIT_PARAMS *fsp_memory_init_params);
asmlinkage void romstage_after_car(void);
void raminit(struct romstage_params *params, struct pei_data *pei_data);
void gfx_init(void);
void tco_disable(void);
void punit_init(void);
void set_max_freq(void);
int early_spi_read_wpsr(u8 *sr);

#if IS_ENABLED(CONFIG_ENABLE_BUILTIN_COM1)
void config_com1_and_enable(void);
#else
static inline void config_com1_and_enable(void) { }
#endif

/* romstage_common.c functions */
void program_base_addresses(void);
void reset_system(void);
void spi_init(void);
void migrate_power_state(void);
struct chipset_power_state *fill_power_state(void);
int chipset_prev_sleep_state(struct chipset_power_state *ps);
void *setup_stack_and_mtrrs(void);

/* Workaround Code. Remove at some point for production. */
void disable_rom_shadow(void);

#endif /* _BRASWELL_ROMSTAGE_H_ */
