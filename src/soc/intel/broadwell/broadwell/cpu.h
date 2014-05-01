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

#ifndef _CPU_INTEL_HASWELL_H
#define _CPU_INTEL_HASWELL_H

#include <arch/cpu.h>

/* Haswell CPU types */
#define HASWELL_FAMILY_MOBILE		0x306c0
#define HASWELL_FAMILY_ULT		0x40650

/* Haswell CPU steppings */
#define HASWELL_STEPPING_MOBILE_A0	1
#define HASWELL_STEPPING_MOBILE_B0	2
#define HASWELL_STEPPING_MOBILE_C0	3
#define HASWELL_STEPPING_MOBILE_D0	4
#define HASWELL_STEPPING_ULT_B0		0
#define HASWELL_STEPPING_ULT_C0		1

/* Haswell bus clock is fixed at 100MHz */
#define HASWELL_BCLK			100


/* PCODE MMIO communications live in the MCHBAR. */
#define BIOS_MAILBOX_INTERFACE			0x5da4
#define  MAILBOX_RUN_BUSY			(1 << 31)
#define  MAILBOX_BIOS_CMD_READ_PCS		1
#define  MAILBOX_BIOS_CMD_WRITE_PCS		2
#define  MAILBOX_BIOS_CMD_READ_CALIBRATION	0x509
#define  MAILBOX_BIOS_CMD_FSM_MEASURE_INTVL	0x909
#define  MAILBOX_BIOS_CMD_READ_PCH_POWER	0xa
#define  MAILBOX_BIOS_CMD_READ_PCH_POWER_EXT	0xb
/* Errors are returned back in bits 7:0. */
#define  MAILBOX_BIOS_ERROR_NONE		0
#define  MAILBOX_BIOS_ERROR_INVALID_COMMAND	1
#define  MAILBOX_BIOS_ERROR_TIMEOUT		2
#define  MAILBOX_BIOS_ERROR_ILLEGAL_DATA	3
#define  MAILBOX_BIOS_ERROR_RESERVED		4
#define  MAILBOX_BIOS_ERROR_ILLEGAL_VR_ID	5
#define  MAILBOX_BIOS_ERROR_VR_INTERFACE_LOCKED	6
#define  MAILBOX_BIOS_ERROR_VR_ERROR		7
/* Data is passed through bits 31:0 of the data register. */
#define BIOS_MAILBOX_DATA			0x5da0

#if !defined(__ROMCC__) // FIXME romcc should handle below constructs

#if defined(__PRE_RAM__)
struct pei_data;
struct rcba_config_instruction;
struct romstage_params {
	struct pei_data *pei_data;
	const void *gpio_map;
	const struct rcba_config_instruction *rcba_config;
	unsigned long bist;
	void (*copy_spd)(struct pei_data *);
};
void mainboard_romstage_entry(unsigned long bist);
void romstage_common(const struct romstage_params *params);
/* romstage_main is called from the cache-as-ram assembly file. The return
 * value is the stack value to be used for romstage once cache-as-ram is
 * torn down. The following values are pushed onto the stack to setup the
 * MTRRs:
 *   +0: Number of MTRRs
 *   +4: MTTR base 0 31:0
 *   +8: MTTR base 0 63:32
 *  +12: MTTR mask 0 31:0
 *  +16: MTTR mask 0 63:32
 *  +20: MTTR base 1 31:0
 *  +24: MTTR base 1 63:32
 *  +28: MTTR mask 1 31:0
 *  +32: MTTR mask 1 63:32
 *  ...
 */
void * asmlinkage romstage_main(unsigned long bist);
/* romstage_after_car() is the C function called after cache-as-ram has
 * been torn down. It is responsible for loading the ramstage. */
void romstage_after_car(void);
#endif

#ifdef __SMM__
/* Lock MSRs */
void intel_cpu_haswell_finalize_smm(void);
#else
/* Configure power limits for turbo mode */
void set_power_limits(u8 power_limit_1_time);
int cpu_config_tdp_levels(void);
struct bus;
void bsp_init_and_start_aps(struct bus *cpu_bus);
/* Determine if HyperThreading is disabled. The variable is not valid until
 * setup_ap_init() has been called. */
extern int ht_disabled;
#endif

/* CPU identification */
int haswell_family_model(void);
int haswell_stepping(void);
int haswell_is_ult(void);

#endif

#endif
