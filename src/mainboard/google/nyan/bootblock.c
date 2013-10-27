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

#include <arch/io.h>
#include <bootblock_common.h>
#include <console/console.h>
#include <device/i2c.h>
#include <soc/addressmap.h>
#include <soc/clock.h>
#include <soc/nvidia/tegra/i2c.h>
#include <soc/nvidia/tegra124/clk_rst.h>
#include <soc/nvidia/tegra124/pinmux.h>
#include <soc/nvidia/tegra124/spi.h>	/* FIXME: move back to soc code? */

#include "pmic.h"

static struct clk_rst_ctlr *clk_rst = (void *)TEGRA_CLK_RST_BASE;

static void set_clock_sources(void)
{
	clock_configure_source(mselect, PLLP, 102000);

	/* TODO: is the 1.333MHz correct? This may have always been bogus... */
	clock_configure_source(i2c1, CLK_M, 1333);
	clock_configure_source(i2c2, CLK_M, 1333);
	clock_configure_source(i2c3, CLK_M, 1333);
	clock_configure_source(i2c4, CLK_M, 1333);
	clock_configure_source(i2c5, CLK_M, 1333);

	/* UARTA gets PLLP, deactivate CLK_UART_DIV_OVERRIDE */
	writel(PLLP << CLK_SOURCE_SHIFT, &clk_rst->clk_src_uarta);

	/*
	 * MMC3 and MMC4: Set base clock frequency for SD Clock to Tegra MMC's
	 * maximum speed (48MHz) so we can change SDCLK by second stage divisor
	 * in payloads, without touching base clock.
	 */
	clock_configure_source(sdmmc3, PLLP, 48000);
	clock_configure_source(sdmmc4, PLLP, 48000);

	/* PLLP and PLLM are switched for HOST1x for no apparent reason. */
	write32(4 /* PLLP! */ << CLK_SOURCE_SHIFT |
		/* TODO(rminnich): The divisor isn't accurate enough to get to
		 * 144MHz (it goes to 163 instead). What should we do here? */
		CLK_DIVIDER(TEGRA_PLLP_KHZ, 144000),
		&clk_rst->clk_src_host1x);

	/* DISP1 doesn't support a divisor. Use PLLC which runs at 600MHz. */
	clock_configure_source(disp1, PLLC, 600000);
}

void bootblock_mainboard_init(void)
{
	set_clock_sources();

	clock_enable_clear_reset(
		// l clocks.
		CLK_L_CACHE2 | CLK_L_GPIO | CLK_L_TMR | CLK_L_I2C1 |
		CLK_L_SDMMC4,
		// h clocks.
		CLK_H_EMC | CLK_H_I2C2 | CLK_H_I2C5 | CLK_H_SBC1 | CLK_H_PMC |
		CLK_H_APBDMA | CLK_H_MEM,
		// u clocks.
		CLK_U_I2C3 | CLK_U_CSITE | CLK_U_SDMMC3,
		// v clocks.
		CLK_V_MSELECT | CLK_V_I2C4,
		// w clocks.
		CLK_W_DVFS);

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
	// I2C4 (DDC) clock.
	pinmux_set_config(PINMUX_DDC_SCL_INDEX,
			  PINMUX_DDC_SCL_FUNC_I2C4 | PINMUX_INPUT_ENABLE);
	// I2C4 (DDC) data.
	pinmux_set_config(PINMUX_DDC_SDA_INDEX,
			  PINMUX_DDC_SDA_FUNC_I2C4 | PINMUX_INPUT_ENABLE);
	// I2C5 (PMU) clock.
	pinmux_set_config(PINMUX_PWR_I2C_SCL_INDEX,
			  PINMUX_PWR_I2C_SCL_FUNC_I2CPMU | PINMUX_INPUT_ENABLE);
	// I2C5 (PMU) data.
	pinmux_set_config(PINMUX_PWR_I2C_SDA_INDEX,
			  PINMUX_PWR_I2C_SDA_FUNC_I2CPMU | PINMUX_INPUT_ENABLE);

	i2c_init(0);
	i2c_init(1);
	i2c_init(2);
	i2c_init(3);
	i2c_init(4);

	pmic_init(4);

	/* SPI4 data out (MOSI) */
	pinmux_set_config(PINMUX_SDMMC1_CMD_INDEX,
			  PINMUX_SDMMC1_CMD_FUNC_SPI4 | PINMUX_INPUT_ENABLE);
	/* SPI4 data in (MISO) */
	pinmux_set_config(PINMUX_SDMMC1_DAT1_INDEX,
			  PINMUX_SDMMC1_DAT1_FUNC_SPI4 | PINMUX_INPUT_ENABLE);
	/* SPI4 clock */
	pinmux_set_config(PINMUX_SDMMC1_DAT2_INDEX,
			  PINMUX_SDMMC1_DAT2_FUNC_SPI4 | PINMUX_INPUT_ENABLE);
	/* SPI4 chip select 0 */
	pinmux_set_config(PINMUX_SDMMC1_DAT3_INDEX,
			  PINMUX_SDMMC1_DAT3_FUNC_SPI4 | PINMUX_INPUT_ENABLE);
//	spi_init();
	tegra_spi_init(4);
}
