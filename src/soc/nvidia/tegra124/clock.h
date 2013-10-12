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

#define CLK_CONSTANTS(name, reg_index, reg_bit) \
	CLK_##name##_REG = reg_index, \
	CLK_##name##_MASK = (1 << reg_bit)

enum {
	CLK_CONSTANTS(CPU, 0, 0),
	CLK_CONSTANTS(COP, 0, 1),
	CLK_CONSTANTS(TRIG_SYS, 0, 2),
	CLK_CONSTANTS(RTC, 0, 4),
	CLK_CONSTANTS(TMR, 0, 5),
	CLK_CONSTANTS(UARTA, 0, 6),
	CLK_CONSTANTS(UARTB, 0, 7),
	CLK_CONSTANTS(GPIO, 0, 8),
	CLK_CONSTANTS(SDMMC2, 0, 9),
	CLK_CONSTANTS(SPDIF, 0, 10),
	CLK_CONSTANTS(I2S1, 0, 11),
	CLK_CONSTANTS(I2C1, 0, 12),
	CLK_CONSTANTS(NDFLASH, 0, 13),
	CLK_CONSTANTS(SDMMC1, 0, 14),
	CLK_CONSTANTS(SDMMC4, 0, 15),
	CLK_CONSTANTS(PWM, 0, 17),
	CLK_CONSTANTS(I2S2, 0, 18),
	CLK_CONSTANTS(EPP, 0, 19),
	CLK_CONSTANTS(VI, 0, 20),
	CLK_CONSTANTS(2D, 0, 21),
	CLK_CONSTANTS(USBD, 0, 22),
	CLK_CONSTANTS(ISP, 0, 23),
	CLK_CONSTANTS(3D, 0, 24),
	CLK_CONSTANTS(DISP2, 0, 26),
	CLK_CONSTANTS(DISP1, 0, 27),
	CLK_CONSTANTS(HOST1X, 0, 28),
	CLK_CONSTANTS(VCP, 0, 29),
	CLK_CONSTANTS(I2S0, 0, 30),
	CLK_CONSTANTS(CACHE2, 0, 31),

	CLK_CONSTANTS(MEM, 1, 0),
	CLK_CONSTANTS(AHBDMA, 1, 1),
	CLK_CONSTANTS(APBDMA, 1, 2),
	CLK_CONSTANTS(KBC, 1, 4),
	CLK_CONSTANTS(STAT_MON, 1, 5),
	CLK_CONSTANTS(PMC, 1, 6),
	CLK_CONSTANTS(FUSE, 1, 7),
	CLK_CONSTANTS(KFUSE, 1, 8),
	CLK_CONSTANTS(SBC1, 1, 9),
	CLK_CONSTANTS(SNOR, 1, 10),
	CLK_CONSTANTS(JTAG2TBC, 1, 11),
	CLK_CONSTANTS(SBC2, 1, 12),
	CLK_CONSTANTS(SBC3, 1, 14),
	CLK_CONSTANTS(I2C5, 1, 15),
	CLK_CONSTANTS(DSI, 1, 16),
	CLK_CONSTANTS(HSI, 1, 18),
	CLK_CONSTANTS(HDMI, 1, 19),
	CLK_CONSTANTS(CSI, 1, 20),
	CLK_CONSTANTS(I2C2, 1, 22),
	CLK_CONSTANTS(UARTC, 1, 23),
	CLK_CONSTANTS(MIPI_CAL, 1, 24),
	CLK_CONSTANTS(EMC, 1, 25),
	CLK_CONSTANTS(USB2, 1, 26),
	CLK_CONSTANTS(USB3, 1, 27),
	CLK_CONSTANTS(MPE, 1, 28),
	CLK_CONSTANTS(VDE, 1, 29),
	CLK_CONSTANTS(BSEA, 1, 30),
	CLK_CONSTANTS(BSEV, 1, 31),

	CLK_CONSTANTS(UARTD, 2, 1),
	CLK_CONSTANTS(UARTE, 2, 2),
	CLK_CONSTANTS(I2C3, 2, 3),
	CLK_CONSTANTS(SBC4, 2, 4),
	CLK_CONSTANTS(SDMMC3, 2, 5),
	CLK_CONSTANTS(PCIE, 2, 6),
	CLK_CONSTANTS(OWR, 2, 7),
	CLK_CONSTANTS(AFI, 2, 8),
	CLK_CONSTANTS(CSITE, 2, 9),
	CLK_CONSTANTS(PCIEXCLK, 2, 10),
	CLK_CONSTANTS(AVPUCQ, 2, 11),
	CLK_CONSTANTS(TRACECLKIN, 2, 13),
	CLK_CONSTANTS(SOC_THERM, 2, 14),
	CLK_CONSTANTS(DTV, 2, 15),
	CLK_CONSTANTS(NAND_SPEED, 2, 16),
	CLK_CONSTANTS(I2C_SLOW, 2, 17),
	CLK_CONSTANTS(DSIB, 2, 18),
	CLK_CONSTANTS(TSEC, 2, 19),
	CLK_CONSTANTS(IRAMA, 2, 20),
	CLK_CONSTANTS(IRAMB, 2, 21),
	CLK_CONSTANTS(IRAMC, 2, 22),

	// Clock reset.
	CLK_CONSTANTS(EMUCIF, 2, 23),
	// Clock enable.
	CLK_CONSTANTS(IRAMD, 2, 23),

	CLK_CONSTANTS(CRAM2, 2, 24),
	CLK_CONSTANTS(XUSB_HOST, 2, 25),
	CLK_CONSTANTS(MSENC, 2, 27),
	CLK_CONSTANTS(SUS_OUT, 2, 28),
	CLK_CONSTANTS(DEV2_OUT, 2, 29),
	CLK_CONSTANTS(DEV1_OUT, 2, 30),
	CLK_CONSTANTS(XUSB_DEV, 2, 31),

	CLK_CONSTANTS(VW_CPUG, 0, 0),
	CLK_CONSTANTS(VW_CPULP, 0, 1),
	CLK_CONSTANTS(VW_3D2, 0, 2),
	CLK_CONSTANTS(VW_MSELECT, 0, 3),
	CLK_CONSTANTS(VW_I2S3, 0, 5),
	CLK_CONSTANTS(VW_I2S4, 0, 6),
	CLK_CONSTANTS(VW_I2C4, 0, 7),
	CLK_CONSTANTS(VW_SBC5, 0, 8),
	CLK_CONSTANTS(VW_SBC6, 0, 9),
	CLK_CONSTANTS(VW_AUDIO, 0, 10),
	CLK_CONSTANTS(VW_APBIF, 0, 11),
	CLK_CONSTANTS(VW_DAM0, 0, 12),
	CLK_CONSTANTS(VW_DAM1, 0, 13),
	CLK_CONSTANTS(VW_DAM2, 0, 14),
	CLK_CONSTANTS(VW_HDA2CODEC_2X, 0, 15),
	CLK_CONSTANTS(VW_ATOMICS, 0, 16),
	CLK_CONSTANTS(VW_ACTMON, 0, 23),
	CLK_CONSTANTS(VW_SATA, 0, 28),
	CLK_CONSTANTS(VW_HDA, 0, 29),

	CLK_CONSTANTS(VW_HDA2HDMICODEC, 1, 0),
	CLK_CONSTANTS(VW_SATACOLD, 1, 1),
	CLK_CONSTANTS(VW_CEC, 1, 8),
	CLK_CONSTANTS(VW_XUSB_PADCTL, 1, 14),
	CLK_CONSTANTS(VW_ENTROPY, 1, 21),
	CLK_CONSTANTS(VW_AMX0, 1, 25),
	CLK_CONSTANTS(VW_ADX0, 1, 26),
	CLK_CONSTANTS(VW_DVFS, 1, 27),
	CLK_CONSTANTS(VW_XUSB_SS, 1, 28),
	CLK_CONSTANTS(VW_MC1, 1, 30),
	CLK_CONSTANTS(VW_EMC1, 1, 31),
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

/* make this a macro, rather than a function; let the C compiler find
 * the error for you if you use an out of range index.. Requires you
 * to declare a clk_rst pointer.  */
#define clkreset(x) &clk_rst->crc_rst_dev[(x)]
#define clkenable(x) &clk_rst->crc_clk_out_enb[(x)]
#define clkresetvw(x) &clk_rst->crc_rst_dev_vw[(x)]
#define clkenablevw(x) &clk_rst->crc_clk_out_enb_vw[(x)]

void clock_uart_config(void);
void clock_cpu0_config_and_reset(void * entry);
void clock_config(void);
void clock_init(void);

#endif /* __SOC_NVIDIA_TEGRA124_CLOCK_H__ */
