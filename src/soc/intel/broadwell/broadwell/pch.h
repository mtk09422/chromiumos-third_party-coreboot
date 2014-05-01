/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2008-2009 coresystems GmbH
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

#ifndef SOUTHBRIDGE_INTEL_LYNXPOINT_PCH_H
#define SOUTHBRIDGE_INTEL_LYNXPOINT_PCH_H

/* PCH types */
#define PCH_TYPE_LPT		0x8c
#define PCH_TYPE_LPT_LP		0x9c

/* PCH stepping values for LPC device */
#define LPT_H_STEP_B0		0x02
#define LPT_H_STEP_C0		0x03
#define LPT_H_STEP_C1		0x04
#define LPT_H_STEP_C2		0x05
#define LPT_LP_STEP_B0		0x02
#define LPT_LP_STEP_B1		0x03
#define LPT_LP_STEP_B2		0x04

#ifndef __ACPI__

#if defined (__SMM__) && !defined(__ASSEMBLER__)
void intel_pch_finalize_smm(void);
#endif

#if !defined(__ASSEMBLER__)
void pch_config_rcba(const struct rcba_config_instruction *rcba_config);
int pch_silicon_revision(void);
int pch_silicon_type(void);
int pch_is_lp(void);
u16 get_pmbase(void);
u16 get_gpiobase(void);

#if !defined(__PRE_RAM__) && !defined(__SMM__)
#include <device/device.h>
#include <arch/acpi.h>
#include "chip.h"
void pch_enable(device_t dev);
void pch_disable_devfn(device_t dev);
#if CONFIG_ELOG
void pch_log_state(void);
#endif
#endif /* __ASSEMBLER__ */


#ifndef CONFIG_MAINBOARD_POWER_ON_AFTER_POWER_FAIL
#define CONFIG_MAINBOARD_POWER_ON_AFTER_POWER_FAIL MAINBOARD_POWER_ON
#endif

/* PCI Configuration Space (D30:F0): PCI2PCI */
#define PSTS	0x06
#define SMLT	0x1b
#define SECSTS	0x1e
#define INTR	0x3c
#define BCTRL	0x3e
#define   SBR	(1 << 6)
#define   SEE	(1 << 1)
#define   PERE	(1 << 0)

/* Power Management Control and Status */
#define PCH_PCS			0x84
#define  PCH_PCS_PS_D3HOT	3


#endif /* __ACPI__ */
#endif /* SOUTHBRIDGE_INTEL_LYNXPOINT_PCH_H */
