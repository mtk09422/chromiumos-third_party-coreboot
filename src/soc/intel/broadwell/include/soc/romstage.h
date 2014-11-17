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

#ifndef _BROADWELL_ROMSTAGE_H_
#define _BROADWELL_ROMSTAGE_H_

#include <stdint.h>
#include <arch/cpu.h>

struct chipset_power_state;
struct pei_data;
struct romstage_params {
	unsigned long bist;
	struct chipset_power_state *power_state;
	struct pei_data *pei_data;
};

/*
 * FSP Boot Flow:
 *   1.  src/cpu/x86/16bit/reset.inc
 *   2.  src/cpu/x86/16bit/entry.inc
 *   3.  other modules
 *   4.  src/soc/intel/broadwell/romstage/fsp_1_1.inc
 *   5.  src/drivers/intel/fsp/fsp_util.c/find_fsp
 *   6.  FSP binary/TempRamInit
 *   7.  src/soc/intel/broadwell/romstage/fsp_1_1.inc - return
 *   8.  src/soc/intel/broadwell/romstage/romstage.c/romstage_main
 *   9.  src/mainboard/.../romstage.c/mainboard_romstage_entry
 *  10.  src/soc/intel/broadwell/romstage/romstage.c/romstage_common
 *  11.  src/soc/intel/broadwell/romstage/fsp.c/chipset_fsp_memory_init_params
 *  12.  src/mainboard/.../fsp.c/board_fsp_memory_init_params
 *  13.  FSP binary/MemoryInit
 *  14.  src/soc/intel/broadwell/romstage/romstage.c/romstage_common - return
 *  15.  src/mainboard/.../romstage.c/mainboard_romstage_entry - return
 *  16.  src/soc/intel/broadwell/romstage/romstage.c/romstage_main - return
 *  17.  src/soc/intel/broadwell/stack.c/setup_stack_and_mttrs
 *  18.  src/soc/intel/broadwell/romstage/fsp_1_1.inc - return, cleanup
 *       after call to romstage_main
 *  19.  FSP binary/TempRamExit
 *  20.  src/soc/intel/broadwell/romstage.c/romstage_after_car
 *  21.  FSP binary/SiliconInit
 *  22.  src/soc/intel/broadwell/romstage.c/romstage_after_car - return
 *  23.  src/soc/intel/broadwell/chip.c/broadwell_final
 *  24.  src/drivers/intel/fsp/fsp_util.c/fsp_notify
 *  25.  FSP binary/FspNotify
 *
 *
 * MRC Boot Flow:
 *   1.  src/cpu/x86/16bit/reset.inc
 *   2.  src/cpu/x86/16bit/entry.inc
 *   3.  other modules
 *   4.  src/soc/intel/broadwell/romstage/cache_as_ram.inc
 *   5.  src/soc/intel/broadwell/romstage/romstage.c/romstage_main
 *   6.  src/mainboard/.../romstage.c/mainboard_romstage_entry
 *   7.  src/soc/intel/broadwell/romstage/romstage.c/romstage_common
 *   8.  src/soc/intel/broadwell/ram_init.c/ram_init
 *   9.  src/soc/intel/broadwell/romstage/romstage.c/romstage_common - return
 *  10.  src/mainboard/.../romstage.c/mainboard_romstage_entry - return
 *  11.  src/soc/intel/broadwell/romstage/romstage.c/romstage_main - return
 *  12.  src/soc/intel/broadwell/stack.c/setup_stack_and_mttrs
 *  13.  src/soc/intel/broadwell/romstage/cache_as_ram.inc - return, cleanup
 *       after call to romstage_main
 *  14.  src/soc/intel/broadwell/romstage.c/romstage_after_car
 */

asmlinkage void *romstage_main(unsigned int bist, uint32_t tsc_lo,
			       uint32_t tsc_high);
void mainboard_romstage_entry(struct romstage_params *params);
void romstage_common(struct romstage_params *params);
void asmlinkage romstage_after_car(void);
void raminit(struct pei_data *pei_data);
void *setup_stack_and_mttrs(void);

struct chipset_power_state;
struct chipset_power_state *fill_power_state(void);
void report_platform_info(void);
void report_memory_config(void);

void set_max_freq(void);

void systemagent_early_init(void);
void pch_early_init(void);
void pch_uart_init(void);
void intel_early_me_status(void);

void enable_smbus(void);
int smbus_read_byte(unsigned device, unsigned address);

int early_spi_read(u32 offset, u32 size, u8 *buffer);
int early_spi_read_wpsr(u8 *sr);

void mainboard_pre_console_init(void);
#endif
