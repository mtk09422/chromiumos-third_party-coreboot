/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2010 coresystems GmbH
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
#include <stdlib.h>
#include <string.h>
#include <cbfs.h>
#include <console/console.h>
#include <cpu/intel/haswell/haswell.h>
#include <northbridge/intel/haswell/haswell.h>
#include <northbridge/intel/haswell/raminit.h>
#include <southbridge/intel/lynxpoint/pch.h>
#include <southbridge/intel/lynxpoint/lp_gpio.h>
#include "gpio.h"
#include "spd.h"

const struct rcba_config_instruction rcba_config[] = {

	/*
	 *             GFX    INTA -> PIRQA (MSI)
	 * D28IP_P1IP  PCIE   INTA -> PIRQA
	 * D29IP_E1P   EHCI   INTA -> PIRQD
	 * D20IP_XHCI  XHCI   INTA -> PIRQC (MSI)
	 * D31IP_SIP   SATA   INTA -> PIRQF (MSI)
	 * D31IP_SMIP  SMBUS  INTB -> PIRQG
	 * D31IP_TTIP  THRT   INTC -> PIRQA
	 * D27IP_ZIP   HDA    INTA -> PIRQG (MSI)
	 */

	/* Device interrupt pin register (board specific) */
	RCBA_SET_REG_32(D31IP, (INTC << D31IP_TTIP) | (NOINT << D31IP_SIP2) |
			(INTB << D31IP_SMIP) | (INTA << D31IP_SIP)),
	RCBA_SET_REG_32(D29IP, (INTA << D29IP_E1P)),
	RCBA_SET_REG_32(D28IP, (INTA << D28IP_P1IP) | (INTC << D28IP_P3IP) |
			(INTB << D28IP_P4IP)),
	RCBA_SET_REG_32(D27IP, (INTA << D27IP_ZIP)),
	RCBA_SET_REG_32(D26IP, (INTA << D26IP_E2P)),
	RCBA_SET_REG_32(D22IP, (NOINT << D22IP_MEI1IP)),
	RCBA_SET_REG_32(D20IP, (INTA << D20IP_XHCI)),

	/* Device interrupt route registers */
	RCBA_SET_REG_32(D31IR, DIR_ROUTE(PIRQG, PIRQC, PIRQB, PIRQA)),/* LPC */
	RCBA_SET_REG_32(D29IR, DIR_ROUTE(PIRQD, PIRQD, PIRQD, PIRQD)),/* EHCI */
	RCBA_SET_REG_32(D28IR, DIR_ROUTE(PIRQA, PIRQB, PIRQC, PIRQD)),/* PCIE */
	RCBA_SET_REG_32(D27IR, DIR_ROUTE(PIRQG, PIRQG, PIRQG, PIRQG)),/* HDA */
	RCBA_SET_REG_32(D22IR, DIR_ROUTE(PIRQA, PIRQA, PIRQA, PIRQA)),/* ME */
	RCBA_SET_REG_32(D21IR, DIR_ROUTE(PIRQE, PIRQF, PIRQF, PIRQF)),/* SIO */
	RCBA_SET_REG_32(D20IR, DIR_ROUTE(PIRQC, PIRQC, PIRQC, PIRQC)),/* XHCI */
	RCBA_SET_REG_32(D23IR, DIR_ROUTE(PIRQH, PIRQH, PIRQH, PIRQH)),/* SDIO */

	/* Disable unused devices (board specific) */
	RCBA_RMW_REG_32(FD, ~0, PCH_DISABLE_ALWAYS),

	RCBA_END_CONFIG,
};

void mainboard_romstage_entry(unsigned long bist)
{
	struct pei_data pei_data;

	struct romstage_params romstage_params = {
		.pei_data = &pei_data,
		.gpio_map = &mainboard_gpio_map,
		.rcba_config = &rcba_config[0],
		.bist = bist,
	};

	/* Fill out PEI data */
	memset(&pei_data, 0, sizeof(pei_data));
	mainboard_fill_pei_data(&pei_data);
	mainboard_fill_spd_data(&pei_data);

	/* Call into the real romstage main with this board's attributes. */
	romstage_common(&romstage_params);
}
