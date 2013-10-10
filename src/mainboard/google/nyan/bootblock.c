/*
 * This file is part of the coreboot project.
 *
 * Copyright 2013 Google Inc.
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

#include <bootblock_common.h>
#include <console/console.h>
#include <device/i2c.h>
#include <soc/nvidia/tegra/i2c.h>
#include <soc/nvidia/tegra124/clock.h>
#include <soc/nvidia/tegra124/pinmux.h>

#include "pmic.h"

void bootblock_mainboard_init(void)
{
	clock_config();

	// I2C1 clock.
	pinmux_set_config(PINMUX_GEN1_I2C_SCL_INDEX,
			  PINMUX_GEN1_I2C_SCL_FUNC_I2C1 | PINMUX_INPUT_ENABLE);
	// I2C1 data.
	pinmux_set_config(PINMUX_GEN1_I2C_SDA_INDEX,
			  PINMUX_GEN1_I2C_SDA_FUNC_I2C1 | PINMUX_INPUT_ENABLE);
	// I2C2 clock.
	pinmux_set_config(PINMUX_GEN2_I2C_SCL_INDEX,
			  PINMUX_GEN2_I2C_SCL_FUNC_I2C2 | PINMUX_INPUT_ENABLE);
	// I2C2 data.
	pinmux_set_config(PINMUX_GEN2_I2C_SDA_INDEX,
			  PINMUX_GEN2_I2C_SDA_FUNC_I2C2 | PINMUX_INPUT_ENABLE);
	// I2C3 (cam) clock.
	pinmux_set_config(PINMUX_CAM_I2C_SCL_INDEX,
			  PINMUX_CAM_I2C_SCL_FUNC_I2C3 | PINMUX_INPUT_ENABLE);
	// I2C3 (cam) data.
	pinmux_set_config(PINMUX_CAM_I2C_SDA_INDEX,
			  PINMUX_CAM_I2C_SDA_FUNC_I2C3 | PINMUX_INPUT_ENABLE);
	// I2C5 (PMU) clock.
	pinmux_set_config(PINMUX_PWR_I2C_SCL_INDEX,
			  PINMUX_PWR_I2C_SCL_FUNC_I2CPMU | PINMUX_INPUT_ENABLE);
	// I2C5 (PMU) data.
	pinmux_set_config(PINMUX_PWR_I2C_SDA_INDEX,
			  PINMUX_PWR_I2C_SDA_FUNC_I2CPMU | PINMUX_INPUT_ENABLE);

	i2c_init(0);
	i2c_init(1);
	i2c_init(2);
	i2c_init(4);

	pmic_init(4);
}
