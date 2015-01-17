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

#include <arch/io.h>
#include <soc/gpio.h>
#include <soc/iomap.h>
#include <soc/lpc.h>
#include <soc/pci_devs.h>
#include <soc/romstage.h>

/*
 * return family number and internal pad number in that community
 * by pad number and which community it is in.
 */
static uint16_t family_number(uint8_t community, uint8_t pad)
{
	uint8_t i;
	uint16_t base;
	uint8_t community_base[4][7] = {
		{ 11, 23,  0xff, 0xff, 0xff, 0xff, 0xff },
		{ 8,  20,  33,   45,   58,   0xff, 0xff },
		{ 7,  19,  25,   33,   43,   54,   0xff },
		{ 7,  15,  23,   31,   39,   47,   0xff }
	};

	if ((community == GP_NORTH) && (pad == 28))
		return (1 << 8) + 12;

	base = 0;
	for (i = 0; i <= 7; i++) {
		if (pad <= community_base[community][i])
			break;
		base = (uint16_t)community_base[community][i] + 1;
	}

	/* family number in high byte and inner pad number in lowest byte */
	return (i << 8) + pad - base;
}

void config_com1_and_enable(void)
{
	uint32_t reg;
	uint16_t fpad;
	uint32_t *pad_config_reg;

	/* Enable the UART hardware for COM1. */
	reg = 1;
	pci_write_config32(PCI_DEV(0, LPC_DEV, 0), UART_CONT, reg);

	/*
	 * Set up the pads to select the UART function for Strago
	 * AD12 SW16(UART1_DATAIN/UART0_DATAIN)   - Setting Mode 2 for UART0_RXD
	 * AD10 SW20(UART1_DATAOUT/UART0_DATAOUT) - Setting Mode 2 for UART0_TXD
	 */
	fpad = family_number(GP_SOUTHWEST, UART1_RXD_PAD);
	pad_config_reg = (uint32_t *)(COMMUNITY_GPSOUTHWEST_BASE + 0x4400 +
		(0x400 * (fpad >> 8)) + (8 * (fpad & 0xff)));
	write32(pad_config_reg, 0x00920300);

	fpad = family_number(GP_SOUTHWEST, UART1_TXD_PAD);
	write32(pad_config_reg, 0x00920300);
}
