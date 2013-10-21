/*
 * Copyright (c) 2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SOC_NVIDIA_TEGRA124_CLOCK_H__
#define __SOC_NVIDIA_TEGRA124_CLOCK_H__

enum {
	CLK_L_CPU = 0x1 << 0,
	CLK_L_COP = 0x1 << 1,
	CLK_L_TRIG_SYS = 0x1 << 2,
	CLK_L_RTC = 0x1 << 4,
	CLK_L_TMR = 0x1 << 5,
	CLK_L_UARTA = 0x1 << 6,
	CLK_L_UARTB = 0x1 << 7,
	CLK_L_GPIO = 0x1 << 8,
	CLK_L_SDMMC2 = 0x1 << 9,
	CLK_L_SPDIF = 0x1 << 10,
	CLK_L_I2S1 = 0x1 << 11,
	CLK_L_I2C1 = 0x1 << 12,
	CLK_L_NDFLASH = 0x1 << 13,
	CLK_L_SDMMC1 = 0x1 << 14,
	CLK_L_SDMMC4 = 0x1 << 15,
	CLK_L_PWM = 0x1 << 17,
	CLK_L_I2S2 = 0x1 << 18,
	CLK_L_EPP = 0x1 << 19,
	CLK_L_VI = 0x1 << 20,
	CLK_L_2D = 0x1 << 21,
	CLK_L_USBD = 0x1 << 22,
	CLK_L_ISP = 0x1 << 23,
	CLK_L_3D = 0x1 << 24,
	CLK_L_DISP2 = 0x1 << 26,
	CLK_L_DISP1 = 0x1 << 27,
	CLK_L_HOST1X = 0x1 << 28,
	CLK_L_VCP = 0x1 << 29,
	CLK_L_I2S0 = 0x1 << 30,
	CLK_L_CACHE2 = 0x1 << 31,

	CLK_H_MEM = 0x1 << 0,
	CLK_H_AHBDMA = 0x1 << 1,
	CLK_H_APBDMA = 0x1 << 2,
	CLK_H_KBC = 0x1 << 4,
	CLK_H_STAT_MON = 0x1 << 5,
	CLK_H_PMC = 0x1 << 6,
	CLK_H_FUSE = 0x1 << 7,
	CLK_H_KFUSE = 0x1 << 8,
	CLK_H_SBC1 = 0x1 << 9,
	CLK_H_SNOR = 0x1 << 10,
	CLK_H_JTAG2TBC = 0x1 << 11,
	CLK_H_SBC2 = 0x1 << 12,
	CLK_H_SBC3 = 0x1 << 14,
	CLK_H_I2C5 = 0x1 << 15,
	CLK_H_DSI = 0x1 << 16,
	CLK_H_HSI = 0x1 << 18,
	CLK_H_HDMI = 0x1 << 19,
	CLK_H_CSI = 0x1 << 20,
	CLK_H_I2C2 = 0x1 << 22,
	CLK_H_UARTC = 0x1 << 23,
	CLK_H_MIPI_CAL = 0x1 << 24,
	CLK_H_EMC = 0x1 << 25,
	CLK_H_USB2 = 0x1 << 26,
	CLK_H_USB3 = 0x1 << 27,
	CLK_H_MPE = 0x1 << 28,
	CLK_H_VDE = 0x1 << 29,
	CLK_H_BSEA = 0x1 << 30,
	CLK_H_BSEV = 0x1 << 31,

	CLK_U_UARTD = 0x1 << 1,
	CLK_U_UARTE = 0x1 << 2,
	CLK_U_I2C3 = 0x1 << 3,
	CLK_U_SBC4 = 0x1 << 4,
	CLK_U_SDMMC3 = 0x1 << 5,
	CLK_U_PCIE = 0x1 << 6,
	CLK_U_OWR = 0x1 << 7,
	CLK_U_AFI = 0x1 << 8,
	CLK_U_CSITE = 0x1 << 9,
	CLK_U_PCIEXCLK = 0x1 << 10,
	CLK_U_AVPUCQ = 0x1 << 11,
	CLK_U_TRACECLKIN = 0x1 << 13,
	CLK_U_SOC_THERM = 0x1 << 14,
	CLK_U_DTV = 0x1 << 15,
	CLK_U_NAND_SPEED = 0x1 << 16,
	CLK_U_I2C_SLOW = 0x1 << 17,
	CLK_U_DSIB = 0x1 << 18,
	CLK_U_TSEC = 0x1 << 19,
	CLK_U_IRAMA = 0x1 << 20,
	CLK_U_IRAMB = 0x1 << 21,
	CLK_U_IRAMC = 0x1 << 22,

	// Clock reset.
	CLK_U_EMUCIF = 0x1 << 23,
	// Clock enable.
	CLK_U_IRAMD = 0x1 << 23,

	CLK_U_CRAM2 = 0x2 << 24,
	CLK_U_XUSB_HOST = 0x1 << 25,
	CLK_U_MSENC = 0x1 << 27,
	CLK_U_SUS_OUT = 0x1 << 28,
	CLK_U_DEV2_OUT = 0x1 << 29,
	CLK_U_DEV1_OUT = 0x1 << 30,
	CLK_U_XUSB_DEV = 0x1 << 31,

	CLK_V_CPUG = 0x1 << 0,
	CLK_V_CPULP = 0x1 << 1,
	CLK_V_3D2 = 0x1 << 2,
	CLK_V_MSELECT = 0x1 << 3,
	CLK_V_I2S3 = 0x1 << 5,
	CLK_V_I2S4 = 0x1 << 6,
	CLK_V_I2C4 = 0x1 << 7,
	CLK_V_SBC5 = 0x1 << 8,
	CLK_V_SBC6 = 0x1 << 9,
	CLK_V_AUDIO = 0x1 << 10,
	CLK_V_APBIF = 0x1 << 11,
	CLK_V_DAM0 = 0x1 << 12,
	CLK_V_DAM1 = 0x1 << 13,
	CLK_V_DAM2 = 0x1 << 14,
	CLK_V_HDA2CODEC_2X = 0x1 << 15,
	CLK_V_ATOMICS = 0x1 << 16,
	CLK_V_ACTMON = 0x1 << 23,
	CLK_V_SATA = 0x1 << 28,
	CLK_V_HDA = 0x1 << 29,

	CLK_W_HDA2HDMICODEC = 0x1 << 0,
	CLK_W_SATACOLD = 0x1 << 1,
	CLK_W_CEC = 0x1 << 8,
	CLK_W_XUSB_PADCTL = 0x1 << 14,
	CLK_W_ENTROPY = 0x1 << 21,
	CLK_W_AMX0 = 0x1 << 25,
	CLK_W_ADX0 = 0x1 << 26,
	CLK_W_DVFS = 0x1 << 27,
	CLK_W_XUSB_SS = 0x1 << 28,
	CLK_W_MC1 = 0x1 << 30,
	CLK_W_EMC1 = 0x1 << 31
};

/* PLL stabilization delay in usec */
#define CLOCK_PLL_STABLE_DELAY_US 300

#define IO_STABILIZATION_DELAY (2)
/* Calculate clock fractional divider value from ref and target frequencies */
#define CLK_DIVIDER(REF, FREQ)	((((REF) * 2) / FREQ) - 2)

/* Calculate clock frequency value from reference and clock divider value */
#define CLK_FREQUENCY(REF, REG)	(((REF) * 2) / (REG + 2))

/* soc-specific */
#define NVBL_PLLP_KHZ   (408000)
#define NVBL_PLLC_KHZ   (600000)
#define NVBL_PLLD_KHZ   (925000)

void clock_uart_config(void);
void clock_cpu0_config_and_reset(void * entry);
void clock_config(void);
void clock_init(void);
void clock_ll_set_source_divisor(u32 *reg, u32 source, u32 divisor);
#endif /* __SOC_NVIDIA_TEGRA124_CLOCK_H__ */