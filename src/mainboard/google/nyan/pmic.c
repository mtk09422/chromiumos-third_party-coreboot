/*
 * This file is part of the coreboot project.
 *
 * Copyright 2013 Google Inc.
 * Copyright (c) 2013, NVIDIA CORPORATION.  All rights reserved.
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

#include <delay.h>
#include <device/i2c.h>
#include <stdint.h>
#include <stdlib.h>

#include "pmic.h"

enum {
	AS3722_I2C_ADDR = 0x40
};

static void pmic_write_reg(unsigned bus, uint8_t reg, uint8_t val)
{
	i2c_write(bus, AS3722_I2C_ADDR, reg, 1, &val, 1);
	udelay(10 * 1000);
}

void pmic_init(unsigned bus)
{
	/*
	 * Don't need to set up VDD_CORE - already done - by OTP
	 * Don't write SDCONTROL - it's already 0x7F, i.e. all SDs enabled.
	 * Don't write LDCONTROL - it's already 0xFF, i.e. all LDOs enabled.
	 */

	/* First set VDD_CPU to 1.2V, then enable the VDD_CPU regulator. */
	if (CONFIG_NYAN_IN_A_PIXEL)
		pmic_write_reg(bus, 0x00, 0x3c);
	else
		pmic_write_reg(bus, 0x00, 0x50);

	/* First set VDD_GPU to 1.0V, then enable the VDD_GPU regulator. */
	pmic_write_reg(bus, 0x06, 0x28);

	/* First set VPP_FUSE to 1.2V, then enable the VPP_FUSE regulator. */
	pmic_write_reg(bus, 0x12, 0x10);

	/*
	 * Bring up VDD_SDMMC via the AS3722 PMIC on the PWR I2C bus.
	 * First set it to bypass 3.3V straight thru, then enable the regulator
	 *
	 * NOTE: We do this early because doing it later seems to hose the CPU
	 * power rail/partition startup. Need to debug.
	 */
	pmic_write_reg(bus, 0x16, 0x3f);

	/*
	 * Panel power GPIO O4. Set mode for GPIO4 (0x0c to 7), then set
	 * the value (register 0x20 bit 4)
	 */
	pmic_write_reg(bus, 0x0c, 0x07);
	pmic_write_reg(bus, 0x20, 0x10);
}
