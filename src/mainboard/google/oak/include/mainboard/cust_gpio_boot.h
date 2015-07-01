/*
 * This file is part of the coreboot project.
 *
 * Copyright 2015 MediaTek Inc.
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

#ifndef SOC_MEDIATEK_MT8173_CUST_GPIO_BOOT_H
#define SOC_MEDIATEK_MT8173_CUST_GPIO_BOOT_H

/* Configuration for GPIO SMT(Schmidt Trigger) Group output start */
enum {
	GPIO_SMT_GROUP_1	= 1,
	GPIO_SMT_GROUP_2	= 0,
	GPIO_SMT_GROUP_10	= 0,
	GPIO_SMT_GROUP_26	= 0,
	GPIO_SMT_GROUP_0	= 0,
	GPIO_SMT_GROUP_34	= 0,
	GPIO_SMT_GROUP_19	= 0,
	GPIO_SMT_GROUP_3	= 0,
	GPIO_SMT_GROUP_4	= 0,
	GPIO_SMT_GROUP_5	= 0,
	GPIO_SMT_GROUP_6	= 0,
	GPIO_SMT_GROUP_7	= 0,
	GPIO_SMT_GROUP_9	= 0,
	GPIO_SMT_GROUP_16	= 0,
	GPIO_SMT_GROUP_11	= 0,
	GPIO_SMT_GROUP_12	= 0,
	GPIO_SMT_GROUP_30	= 0,
	GPIO_SMT_GROUP_31	= 0,
	GPIO_SMT_GROUP_32	= 0,
	GPIO_SMT_GROUP_33	= 0,
	GPIO_SMT_GROUP_13	= 0,
	GPIO_SMT_GROUP_14	= 0,
	GPIO_SMT_GROUP_15	= 0,
	GPIO_SMT_GROUP_20	= 0,
	GPIO_SMT_GROUP_17	= 0,
	GPIO_SMT_GROUP_18	= 0,
	GPIO_SMT_GROUP_21	= 0,
	GPIO_SMT_GROUP_22	= 0,
	GPIO_SMT_GROUP_23	= 0,
	GPIO_SMT_GROUP_40	= 0,
	GPIO_SMT_GROUP_41	= 0,
	GPIO_SMT_GROUP_24	= 0,
};
/* Configuration for GPIO SMT(Schmidt Trigger) Group output end */

enum {
	/* Configuration for Pin 0 */
	GPIO0_MODE	= 0,
	GPIO0_DIR	= 0,
	GPIO0_PULLEN	= 1,
	GPIO0_PULL	= 1,
	GPIO0_DATAOUT	= 0,

	/* Configuration for Pin 1 */
	GPIO1_MODE	= 0,
	GPIO1_DIR	= 0,
	GPIO1_PULLEN	= 1,
	GPIO1_PULL	= 1,
	GPIO1_DATAOUT	= 0,

	/* Configuration for Pin 2 */
	GPIO2_MODE	= 0,
	GPIO2_DIR	= 0,
	GPIO2_PULLEN	= 1,
	GPIO2_PULL	= 0,
	GPIO2_DATAOUT	= 0,

	/* Configuration for Pin 3 */
	GPIO3_MODE	= 0,
	GPIO3_DIR	= 0,
	GPIO3_PULLEN	= 1,
	GPIO3_PULL	= 0,
	GPIO3_DATAOUT	= 0,

	/* Configuration for Pin 4 */
	GPIO4_MODE	= 6,
	GPIO4_DIR	= 0,
	GPIO4_PULLEN	= 1,
	GPIO4_PULL	= 1,
	GPIO4_DATAOUT	= 0,

	/* Configuration for Pin 5 */
	GPIO5_MODE	= 6,
	GPIO5_DIR	= 1,
	GPIO5_PULLEN	= 1,
	GPIO5_PULL	= 1,
	GPIO5_DATAOUT	= 0,

	/* Configuration for Pin 6 */
	GPIO6_MODE	= 6,
	GPIO6_DIR	= 0,
	GPIO6_PULLEN	= 1,
	GPIO6_PULL	= 1,
	GPIO6_DATAOUT	= 0,

	/* Configuration for Pin 7 */
	GPIO7_MODE	= 6,
	GPIO7_DIR	= 0,
	GPIO7_PULLEN	= 1,
	GPIO7_PULL	= 1,
	GPIO7_DATAOUT	= 0,

	/* Configuration for Pin 8 */
	GPIO8_MODE	= 6,
	GPIO8_DIR	= 0,
	GPIO8_PULLEN	= 1,
	GPIO8_PULL	= 1,
	GPIO8_DATAOUT	= 0,

	/* Configuration for Pin 9 */
	GPIO9_MODE	= 6,
	GPIO9_DIR	= 0,
	GPIO9_PULLEN	= 1,
	GPIO9_PULL	= 1,
	GPIO9_DATAOUT	= 0,

	/* Configuration for Pin 10 */
	GPIO10_MODE	= 0,
	GPIO10_DIR	= 0,
	GPIO10_PULLEN	= 1,
	GPIO10_PULL	= 0,
	GPIO10_DATAOUT	= 0,

	/* Configuration for Pin 11 */
	GPIO11_MODE	= 0,
	GPIO11_DIR	= 0,
	GPIO11_PULLEN	= 1,
	GPIO11_PULL	= 1,
	GPIO11_DATAOUT	= 0,

	/* Configuration for Pin 12 */
	GPIO12_MODE	= 0,
	GPIO12_DIR	= 0,
	GPIO12_PULLEN	= 0,
	GPIO12_PULL	= 1,
	GPIO12_DATAOUT	= 1,

	/* Configuration for Pin 13 */
	GPIO13_MODE	= 0,
	GPIO13_DIR	= 0,
	GPIO13_PULLEN	= 0,
	GPIO13_PULL	= 1,
	GPIO13_DATAOUT	= 0,

	/* Configuration for Pin 14 */
	GPIO14_MODE	= 0,
	GPIO14_DIR	= 0,
	GPIO14_PULLEN	= 0,
	GPIO14_PULL	= 1,
	GPIO14_DATAOUT	= 0,

	/* Configuration for Pin 15 */
	GPIO15_MODE	= 1,
	GPIO15_DIR	= 0,
	GPIO15_PULLEN	= 1,
	GPIO15_PULL	= 0,
	GPIO15_DATAOUT	= 0,

	/* Configuration for Pin 16 */
	GPIO16_MODE	= 0,
	GPIO16_DIR	= 0,
	GPIO16_PULLEN	= 1,
	GPIO16_PULL	= 1,
	GPIO16_DATAOUT	= 0,

	/* Configuration for Pin 17 */
	GPIO17_MODE	= 1,
	GPIO17_DIR	= 0,
	GPIO17_PULLEN	= 1,
	GPIO17_PULL	= 0,
	GPIO17_DATAOUT	= 0,

	/* Configuration for Pin 18 */
	GPIO18_MODE	= 1,
	GPIO18_DIR	= 0,
	GPIO18_PULLEN	= 0,
	GPIO18_PULL	= 1,
	GPIO18_DATAOUT	= 0,

	/* Configuration for Pin 19 */
	GPIO19_MODE	= 1,
	GPIO19_DIR	= 0,
	GPIO19_PULLEN	= 0,
	GPIO19_PULL	= 1,
	GPIO19_DATAOUT	= 0,

	/* Configuration for Pin 20 */
	GPIO20_MODE	= 1,
	GPIO20_DIR	= 0,
	GPIO20_PULLEN	= 0,
	GPIO20_PULL	= 1,
	GPIO20_DATAOUT	= 0,

	/* Configuration for Pin 21 */
	GPIO21_MODE	= 0,
	GPIO21_DIR	= 0,
	GPIO21_PULLEN	= 0,
	GPIO21_PULL	= 1,
	GPIO21_DATAOUT	= 0,

	/* Configuration for Pin 22 */
	GPIO22_MODE	= 1,
	GPIO22_DIR	= 0,
	GPIO22_PULLEN	= 1,
	GPIO22_PULL	= 1,
	GPIO22_DATAOUT	= 0,

	/* Configuration for Pin 23 */
	GPIO23_MODE	= 1,
	GPIO23_DIR	= 0,
	GPIO23_PULLEN	= 1,
	GPIO23_PULL	= 1,
	GPIO23_DATAOUT	= 0,

	/* Configuration for Pin 24 */
	GPIO24_MODE	= 1,
	GPIO24_DIR	= 0,
	GPIO24_PULLEN	= 1,
	GPIO24_PULL	= 1,
	GPIO24_DATAOUT	= 0,

	/* Configuration for Pin 25 */
	GPIO25_MODE	= 1,
	GPIO25_DIR	= 0,
	GPIO25_PULLEN	= 1,
	GPIO25_PULL	= 1,
	GPIO25_DATAOUT	= 0,

	/* Configuration for Pin 26 */
	GPIO26_MODE	= 1,
	GPIO26_DIR	= 1,
	GPIO26_PULLEN	= 1,
	GPIO26_PULL	= 0,
	GPIO26_DATAOUT	= 0,

	/* Configuration for Pin 27 */
	GPIO27_MODE	= 1,
	GPIO27_DIR	= 0,
	GPIO27_PULLEN	= 1,
	GPIO27_PULL	= 1,
	GPIO27_DATAOUT	= 0,

	/* Configuration for Pin 28 */
	GPIO28_MODE	= 1,
	GPIO28_DIR	= 0,
	GPIO28_PULLEN	= 0,
	GPIO28_PULL	= 0,
	GPIO28_DATAOUT	= 0,

	/* Configuration for Pin 29 */
	GPIO29_MODE	= 0,
	GPIO29_DIR	= 1,
	GPIO29_PULLEN	= 1,
	GPIO29_PULL	= 0,
	GPIO29_DATAOUT	= 1,

	/* Configuration for Pin 30 */
	GPIO30_MODE	= 0,
	GPIO30_DIR	= 1,
	GPIO30_PULLEN	= 1,
	GPIO30_PULL	= 1,
	GPIO30_DATAOUT	= 1,

	/* Configuration for Pin 31 */
	GPIO31_MODE	= 1,
	GPIO31_DIR	= 0,
	GPIO31_PULLEN	= 1,
	GPIO31_PULL	= 1,
	GPIO31_DATAOUT	= 0,

	/* Configuration for Pin 32 */
	GPIO32_MODE	= 1,
	GPIO32_DIR	= 1,
	GPIO32_PULLEN	= 1,
	GPIO32_PULL	= 0,
	GPIO32_DATAOUT	= 0,

	/* Configuration for Pin 33 */
	GPIO33_MODE	= 0,
	GPIO33_DIR	= 1,
	GPIO33_PULLEN	= 1,
	GPIO33_PULL	= 0,
	GPIO33_DATAOUT	= 0,

	/* Configuration for Pin 34 */
	GPIO34_MODE	= 0,
	GPIO34_DIR	= 0,
	GPIO34_PULLEN	= 0,
	GPIO34_PULL	= 0,
	GPIO34_DATAOUT	= 0,

	/* Configuration for Pin 35 */
	GPIO35_MODE	= 0,
	GPIO35_DIR	= 1,
	GPIO35_PULLEN	= 1,
	GPIO35_PULL	= 1,
	GPIO35_DATAOUT	= 0,

	/* Configuration for Pin 36 */
	GPIO36_MODE	= 0,
	GPIO36_DIR	= 1,
	GPIO36_PULLEN	= 0,
	GPIO36_PULL	= 0,
	GPIO36_DATAOUT	= 1,

	/* Configuration for Pin 37, TPM INT */
	GPIO37_MODE	= 0,
	GPIO37_DIR	= 0,
	GPIO37_PULLEN	= 1,
	GPIO37_PULL	= 0,
	GPIO37_DATAOUT	= 0,

	/* Configuration for Pin 38 */
	GPIO38_MODE	= 1,
	GPIO38_DIR	= 0,
	GPIO38_PULLEN	= 0,
	GPIO38_PULL	= 0,
	GPIO38_DATAOUT	= 0,

	/* Configuration for Pin 39 */
	GPIO39_MODE	= 0,
	GPIO39_DIR	= 1,
	GPIO39_PULLEN	= 1,
	GPIO39_PULL	= 1,
	GPIO39_DATAOUT	= 0,

	/* Configuration for Pin 40 */
	GPIO40_MODE	= 1,
	GPIO40_DIR	= 0,
	GPIO40_PULLEN	= 1,
	GPIO40_PULL	= 0,
	GPIO40_DATAOUT	= 0,

	/* Configuration for Pin 41 */
	GPIO41_MODE	= 0,
	GPIO41_DIR	= 1,
	GPIO41_PULLEN	= 1,
	GPIO41_PULL	= 0,
	GPIO41_DATAOUT	= 1,

	/* Configuration for Pin 42 */
	GPIO42_MODE	= 0,
	GPIO42_DIR	= 0,
	GPIO42_PULLEN	= 1,
	GPIO42_PULL	= 1,
	GPIO42_DATAOUT	= 0,

	/* Configuration for Pin 43 */
	GPIO43_MODE	= 1,
	GPIO43_DIR	= 0,
	GPIO43_PULLEN	= 0,
	GPIO43_PULL	= 0,
	GPIO43_DATAOUT	= 0,

	/* Configuration for Pin 44 */
	GPIO44_MODE	= 1,
	GPIO44_DIR	= 0,
	GPIO44_PULLEN	= 0,
	GPIO44_PULL	= 0,
	GPIO44_DATAOUT	= 0,

	/* Configuration for Pin 45 */
	GPIO45_MODE	= 1,
	GPIO45_DIR	= 0,
	GPIO45_PULLEN	= 0,
	GPIO45_PULL	= 0,
	GPIO45_DATAOUT	= 0,

	/* Configuration for Pin 46 */
	GPIO46_MODE	= 1,
	GPIO46_DIR	= 0,
	GPIO46_PULLEN	= 0,
	GPIO46_PULL	= 0,
	GPIO46_DATAOUT	= 0,

	/* Configuration for Pin 47 */
	GPIO47_MODE	= 0,
	GPIO47_DIR	= 1,
	GPIO47_PULLEN	= 0,
	GPIO47_PULL	= 0,
	GPIO47_DATAOUT	= 1,

	/* Configuration for Pin 48 */
	GPIO48_MODE	= 0,
	GPIO48_DIR	= 0,
	GPIO48_PULLEN	= 1,
	GPIO48_PULL	= 0,
	GPIO48_DATAOUT	= 0,

	/* Configuration for Pin 49 */
	GPIO49_MODE	= 0,
	GPIO49_DIR	= 0,
	GPIO49_PULLEN	= 1,
	GPIO49_PULL	= 0,
	GPIO49_DATAOUT	= 0,

	/* Configuration for Pin 50 */
	GPIO50_MODE	= 0,
	GPIO50_DIR	= 1,
	GPIO50_PULLEN	= 1,
	GPIO50_PULL	= 1,
	GPIO50_DATAOUT	= 1,

	/* Configuration for Pin 51 */
	GPIO51_MODE	= 0,
	GPIO51_DIR	= 1,
	GPIO51_PULLEN	= 0,
	GPIO51_PULL	= 0,
	GPIO51_DATAOUT	= 1,

	/* Configuration for Pin 52 */
	GPIO52_MODE	= 0,
	GPIO52_DIR	= 0,
	GPIO52_PULLEN	= 0,
	GPIO52_PULL	= 0,
	GPIO52_DATAOUT	= 0,

	/* Configuration for Pin 53 */
	GPIO53_MODE	= 0,
	GPIO53_DIR	= 0,
	GPIO53_PULLEN	= 0,
	GPIO53_PULL	= 0,
	GPIO53_DATAOUT	= 0,

	/* Configuration for Pin 54 */
	GPIO54_MODE	= 0,
	GPIO54_DIR	= 0,
	GPIO54_PULLEN	= 1,
	GPIO54_PULL	= 0,
	GPIO54_DATAOUT	= 0,

	/* Configuration for Pin 55 */
	GPIO55_MODE	= 0,
	GPIO55_DIR	= 0,
	GPIO55_PULLEN	= 1,
	GPIO55_PULL	= 0,
	GPIO55_DATAOUT	= 0,

	/* Configuration for Pin 56 */
	GPIO56_MODE	= 0,
	GPIO56_DIR	= 0,
	GPIO56_PULLEN	= 1,
	GPIO56_PULL	= 0,
	GPIO56_DATAOUT	= 0,

	/* Configuration for Pin 57 */
	GPIO57_MODE	= 1,
	GPIO57_DIR	= 0,
	GPIO57_PULLEN	= 1,
	GPIO57_PULL	= 1,
	GPIO57_DATAOUT	= 0,

	/* Configuration for Pin 58 */
	GPIO58_MODE	= 1,
	GPIO58_DIR	= 0,
	GPIO58_PULLEN	= 1,
	GPIO58_PULL	= 1,
	GPIO58_DATAOUT	= 0,

	/* Configuration for Pin 59 */
	GPIO59_MODE	= 1,
	GPIO59_DIR	= 0,
	GPIO59_PULLEN	= 1,
	GPIO59_PULL	= 1,
	GPIO59_DATAOUT	= 0,

	/* Configuration for Pin 60 */
	GPIO60_MODE	= 1,
	GPIO60_DIR	= 0,
	GPIO60_PULLEN	= 1,
	GPIO60_PULL	= 1,
	GPIO60_DATAOUT	= 0,

	/* Configuration for Pin 61 */
	GPIO61_MODE	= 1,
	GPIO61_DIR	= 0,
	GPIO61_PULLEN	= 1,
	GPIO61_PULL	= 1,
	GPIO61_DATAOUT	= 0,

	/* Configuration for Pin 62 */
	GPIO62_MODE	= 1,
	GPIO62_DIR	= 0,
	GPIO62_PULLEN	= 1,
	GPIO62_PULL	= 1,
	GPIO62_DATAOUT	= 0,

	/* Configuration for Pin 63 */
	GPIO63_MODE	= 1,
	GPIO63_DIR	= 0,
	GPIO63_PULLEN	= 1,
	GPIO63_PULL	= 1,
	GPIO63_DATAOUT	= 0,

	/* Configuration for Pin 64 */
	GPIO64_MODE	= 1,
	GPIO64_DIR	= 0,
	GPIO64_PULLEN	= 1,
	GPIO64_PULL	= 1,
	GPIO64_DATAOUT	= 0,

	/* Configuration for Pin 65 */
	GPIO65_MODE	= 1,
	GPIO65_DIR	= 1,
	GPIO65_PULLEN	= 1,
	GPIO65_PULL	= 0,
	GPIO65_DATAOUT	= 0,

	/* Configuration for Pin 66 */
	GPIO66_MODE	= 1,
	GPIO66_DIR	= 0,
	GPIO66_PULLEN	= 1,
	GPIO66_PULL	= 1,
	GPIO66_DATAOUT	= 0,

	/* Configuration for Pin 67 */
	GPIO67_MODE	= 1,
	GPIO67_DIR	= 0,
	GPIO67_PULLEN	= 1,
	GPIO67_PULL	= 0,
	GPIO67_DATAOUT	= 0,

	/* Configuration for Pin 68 */
	GPIO68_MODE	= 1,
	GPIO68_DIR	= 1,
	GPIO68_PULLEN	= 1,
	GPIO68_PULL	= 0,
	GPIO68_DATAOUT	= 1,

	/* Configuration for Pin 69 */
	GPIO69_MODE	= 1,
	GPIO69_DIR	= 1,
	GPIO69_PULLEN	= 1,
	GPIO69_PULL	= 0,
	GPIO69_DATAOUT	= 0,

	/* Configuration for Pin 70 */
	GPIO70_MODE	= 1,
	GPIO70_DIR	= 0,
	GPIO70_PULLEN	= 1,
	GPIO70_PULL	= 0,
	GPIO70_DATAOUT	= 0,

	/* Configuration for Pin 71 */
	GPIO71_MODE	= 1,
	GPIO71_DIR	= 1,
	GPIO71_PULLEN	= 1,
	GPIO71_PULL	= 0,
	GPIO71_DATAOUT	= 0,

	/* Configuration for Pin 72 */
	GPIO72_MODE	= 1,
	GPIO72_DIR	= 1,
	GPIO72_PULLEN	= 1,
	GPIO72_PULL	= 0,
	GPIO72_DATAOUT	= 0,

	/* Configuration for Pin 73 */
	GPIO73_MODE	= 1,
	GPIO73_DIR	= 0,
	GPIO73_PULLEN	= 1,
	GPIO73_PULL	= 1,
	GPIO73_DATAOUT	= 0,

	/* Configuration for Pin 74 */
	GPIO74_MODE	= 1,
	GPIO74_DIR	= 0,
	GPIO74_PULLEN	= 1,
	GPIO74_PULL	= 1,
	GPIO74_DATAOUT	= 0,

	/* Configuration for Pin 75 */
	GPIO75_MODE	= 1,
	GPIO75_DIR	= 0,
	GPIO75_PULLEN	= 1,
	GPIO75_PULL	= 1,
	GPIO75_DATAOUT	= 0,

	/* Configuration for Pin 76 */
	GPIO76_MODE	= 1,
	GPIO76_DIR	= 0,
	GPIO76_PULLEN	= 1,
	GPIO76_PULL	= 1,
	GPIO76_DATAOUT	= 0,

	/* Configuration for Pin 77 */
	GPIO77_MODE	= 1,
	GPIO77_DIR	= 1,
	GPIO77_PULLEN	= 1,
	GPIO77_PULL	= 0,
	GPIO77_DATAOUT	= 0,

	/* Configuration for Pin 78 */
	GPIO78_MODE	= 1,
	GPIO78_DIR	= 0,
	GPIO78_PULLEN	= 1,
	GPIO78_PULL	= 1,
	GPIO78_DATAOUT	= 0,

	/* Configuration for Pin 79 */
	GPIO79_MODE	= 1,
	GPIO79_DIR	= 0,
	GPIO79_PULLEN	= 1,
	GPIO79_PULL	= 0,
	GPIO79_DATAOUT	= 0,

	/* Configuration for Pin 80 */
	GPIO80_MODE	= 1,
	GPIO80_DIR	= 0,
	GPIO80_PULLEN	= 1,
	GPIO80_PULL	= 0,
	GPIO80_DATAOUT	= 0,

	/* Configuration for Pin 81 */
	GPIO81_MODE	= 1,
	GPIO81_DIR	= 0,
	GPIO81_PULLEN	= 1,
	GPIO81_PULL	= 0,
	GPIO81_DATAOUT	= 0,

	/* Configuration for Pin 82 */
	GPIO82_MODE	= 1,
	GPIO82_DIR	= 0,
	GPIO82_PULLEN	= 1,
	GPIO82_PULL	= 1,
	GPIO82_DATAOUT	= 0,

	/* Configuration for Pin 83 */
	GPIO83_MODE	= 1,
	GPIO83_DIR	= 1,
	GPIO83_PULLEN	= 1,
	GPIO83_PULL	= 0,
	GPIO83_DATAOUT	= 0,

	/* Configuration for Pin 84 */
	GPIO84_MODE	= 1,
	GPIO84_DIR	= 0,
	GPIO84_PULLEN	= 1,
	GPIO84_PULL	= 0,
	GPIO84_DATAOUT	= 0,

	/* Configuration for Pin 85 */
	GPIO85_MODE	= 1,
	GPIO85_DIR	= 1,
	GPIO85_PULLEN	= 1,
	GPIO85_PULL	= 0,
	GPIO85_DATAOUT	= 0,

	/* Configuration for Pin 86 */
	GPIO86_MODE	= 1,
	GPIO86_DIR	= 0,
	GPIO86_PULLEN	= 1,
	GPIO86_PULL	= 0,
	GPIO86_DATAOUT	= 0,

	/* Configuration for Pin 87 */
	GPIO87_MODE	= 0,
	GPIO87_DIR	= 1,
	GPIO87_PULLEN	= 1,
	GPIO87_PULL	= 1,
	GPIO87_DATAOUT	= 1,

	/* Configuration for Pin 88 */
	GPIO88_MODE	= 1,
	GPIO88_DIR	= 0,
	GPIO88_PULLEN	= 1,
	GPIO88_PULL	= 0,
	GPIO88_DATAOUT	= 0,

	/* Configuration for Pin 89 */
	GPIO89_MODE	= 0,
	GPIO89_DIR	= 0,
	GPIO89_PULLEN	= 1,
	GPIO89_PULL	= 0,
	GPIO89_DATAOUT	= 0,

	/* Configuration for Pin 90 */
	GPIO90_MODE	= 1,
	GPIO90_DIR	= 0,
	GPIO90_PULLEN	= 1,
	GPIO90_PULL	= 1,
	GPIO90_DATAOUT	= 0,

	/* Configuration for Pin 91 */
	GPIO91_MODE	= 1,
	GPIO91_DIR	= 0,
	GPIO91_PULLEN	= 1,
	GPIO91_PULL	= 1,
	GPIO91_DATAOUT	= 0,

	/* Configuration for Pin 92 */
	GPIO92_MODE	= 0,
	GPIO92_DIR	= 1,
	GPIO92_PULLEN	= 1,
	GPIO92_PULL	= 0,
	GPIO92_DATAOUT	= 0,

	/* Configuration for Pin 93 */
	GPIO93_MODE	= 0,
	GPIO93_DIR	= 1,
	GPIO93_PULLEN	= 1,
	GPIO93_PULL	= 1,
	GPIO93_DATAOUT	= 1,

	/* Configuration for Pin 94 */
	GPIO94_MODE	= 0,
	GPIO94_DIR	= 0,
	GPIO94_PULLEN	= 0,
	GPIO94_PULL	= 1,
	GPIO94_DATAOUT	= 1,

	/* Configuration for Pin 95 */
	GPIO95_MODE	= 0,
	GPIO95_DIR	= 1,
	GPIO95_PULLEN	= 1,
	GPIO95_PULL	= 0,
	GPIO95_DATAOUT	= 1,

	/* Configuration for Pin 96 */
	GPIO96_MODE	= 0,
	GPIO96_DIR	= 0,
	GPIO96_PULLEN	= 0,
	GPIO96_PULL	= 1,
	GPIO96_DATAOUT	= 1,

	/* Configuration for Pin 97
	 * EC_FW_UPDATE_1V8_L (T20_REC button on servo board)(No related to EC)
	 */
	GPIO97_MODE	= 1,
	GPIO97_DIR	= 0,
	GPIO97_PULLEN	= 0,
	GPIO97_PULL	= 1,
	GPIO97_DATAOUT	= 1,

	/* Configuration for Pin 98 */
	GPIO98_MODE	= 0,
	GPIO98_DIR	= 1,
	GPIO98_PULLEN	= 1,
	GPIO98_PULL	= 0,
	GPIO98_DATAOUT	= 0,

	/* Configuration for Pin 99 */
	GPIO99_MODE	= 0,
	GPIO99_DIR	= 0,
	GPIO99_PULLEN	= 1,
	GPIO99_PULL	= 0,
	GPIO99_DATAOUT	= 0,

	/* Configuration for Pin 100 */
	GPIO100_MODE	= 0,
	GPIO100_DIR	= 1,
	GPIO100_PULLEN	= 1,
	GPIO100_PULL	= 0,
	GPIO100_DATAOUT	= 1,

	/* Configuration for Pin 101 */
	GPIO101_MODE	= 0,
	GPIO101_DIR	= 1,
	GPIO101_PULLEN	= 0,
	GPIO101_PULL	= 1,
	GPIO101_DATAOUT	= 1,

	/* Configuration for Pin 102 */
	GPIO102_MODE	= 6,
	GPIO102_DIR	= 1,
	GPIO102_PULLEN	= 0,
	GPIO102_PULL	= 0,
	GPIO102_DATAOUT	= 0,

	/* Configuration for Pin 103 */
	GPIO103_MODE	= 6,
	GPIO103_DIR	= 0,
	GPIO103_PULLEN	= 0,
	GPIO103_PULL	= 0,
	GPIO103_DATAOUT	= 0,

	/* Configuration for Pin 104 */
	GPIO104_MODE	= 6,
	GPIO104_DIR	= 1,
	GPIO104_PULLEN	= 0,
	GPIO104_PULL	= 0,
	GPIO104_DATAOUT	= 0,

	/* Configuration for Pin 105 */
	GPIO105_MODE	= 6,
	GPIO105_DIR	= 1,
	GPIO105_PULLEN	= 0,
	GPIO105_PULL	= 0,
	GPIO105_DATAOUT	= 0,

	/* Configuration for Pin 106 */
	GPIO106_MODE	= 1,
	GPIO106_DIR	= 0,
	GPIO106_PULLEN	= 1,
	GPIO106_PULL	= 1,
	GPIO106_DATAOUT	= 0,

	/* Configuration for Pin 107 */
	GPIO107_MODE	= 1,
	GPIO107_DIR	= 0,
	GPIO107_PULLEN	= 1,
	GPIO107_PULL	= 1,
	GPIO107_DATAOUT	= 0,

	/* Configuration for Pin 108 */
	GPIO108_MODE	= 1,
	GPIO108_DIR	= 0,
	GPIO108_PULLEN	= 1,
	GPIO108_PULL	= 1,
	GPIO108_DATAOUT	= 0,

	/* Configuration for Pin 109 */
	GPIO109_MODE	= 1,
	GPIO109_DIR	= 0,
	GPIO109_PULLEN	= 1,
	GPIO109_PULL	= 1,
	GPIO109_DATAOUT	= 0,

	/* Configuration for Pin 110 */
	GPIO110_MODE	= 1,
	GPIO110_DIR	= 0,
	GPIO110_PULLEN	= 1,
	GPIO110_PULL	= 1,
	GPIO110_DATAOUT	= 0,

	/* Configuration for Pin 111 */
	GPIO111_MODE	= 1,
	GPIO111_DIR	= 1,
	GPIO111_PULLEN	= 1,
	GPIO111_PULL	= 1,
	GPIO111_DATAOUT	= 0,

	/* Configuration for Pin 112 */
	GPIO112_MODE	= 1,
	GPIO112_DIR	= 0,
	GPIO112_PULLEN	= 1,
	GPIO112_PULL	= 0,
	GPIO112_DATAOUT	= 0,

	/* Configuration for Pin 113 */
	GPIO113_MODE	= 1,
	GPIO113_DIR	= 0,
	GPIO113_PULLEN	= 1,
	GPIO113_PULL	= 1,
	GPIO113_DATAOUT	= 0,

	/* Configuration for Pin 114 */
	GPIO114_MODE	= 1,
	GPIO114_DIR	= 1,
	GPIO114_PULLEN	= 1,
	GPIO114_PULL	= 1,
	GPIO114_DATAOUT	= 0,

	/* Configuration for Pin 115 */
	GPIO115_MODE	= 0,
	GPIO115_DIR	= 1,
	GPIO115_PULLEN	= 1,
	GPIO115_PULL	= 0,
	GPIO115_DATAOUT	= 1,

	/* Configuration for Pin 116 */
	GPIO116_MODE	= 0,
	GPIO116_DIR	= 0,
	GPIO116_PULLEN	= 1,
	GPIO116_PULL	= 0,
	GPIO116_DATAOUT	= 0,

	/* Configuration for Pin 117 */
	GPIO117_MODE	= 1,
	GPIO117_DIR	= 0,
	GPIO117_PULLEN	= 1,
	GPIO117_PULL	= 1,
	GPIO117_DATAOUT	= 0,

	/* Configuration for Pin 118 */
	GPIO118_MODE	= 1,
	GPIO118_DIR	= 1,
	GPIO118_PULLEN	= 1,
	GPIO118_PULL	= 1,
	GPIO118_DATAOUT	= 0,

	/* Configuration for Pin 119 */
	GPIO119_MODE	= 1,
	GPIO119_DIR	= 1,
	GPIO119_PULLEN	= 0,
	GPIO119_PULL	= 0,
	GPIO119_DATAOUT	= 0,

	/* Configuration for Pin 120, EC AP suspend (EC_SUSPEND_L_1V8) */
	GPIO120_MODE	= 0,
	GPIO120_DIR	= 1,
	GPIO120_PULLEN	= 0,
	GPIO120_PULL	= 1,
	GPIO120_DATAOUT	= 1,

	/* Configuration for Pin 121, EC AP warm reset (EC_WARM_RESET_1V8) */
	GPIO121_MODE	= 0,
	GPIO121_DIR	= 0,
	GPIO121_PULLEN	= 0,
	GPIO121_PULL	= 0,
	GPIO121_DATAOUT	= 0,

	/* Configuration for Pin 122 */
	GPIO122_MODE	= 1,
	GPIO122_DIR	= 0,
	GPIO122_PULLEN	= 1,
	GPIO122_PULL	= 1,
	GPIO122_DATAOUT	= 0,

	/* Configuration for Pin 123 */
	GPIO123_MODE	= 1,
	GPIO123_DIR	= 0,
	GPIO123_PULLEN	= 1,
	GPIO123_PULL	= 1,
	GPIO123_DATAOUT	= 0,

	/* Configuration for Pin 124, touch pad interrupt (TP_INT_1V8) */
	GPIO124_MODE	= 0,
	GPIO124_DIR	= 0,
	GPIO124_PULLEN	= 0,
	GPIO124_PULL	= 1,
	GPIO124_DATAOUT	= 0,

	/* Configuration for Pin 125 */
	GPIO125_MODE	= 1,
	GPIO125_DIR	= 0,
	GPIO125_PULLEN	= 1,
	GPIO125_PULL	= 1,
	GPIO125_DATAOUT	= 0,

	/* Configuration for Pin 126 */
	GPIO126_MODE	= 1,
	GPIO126_DIR	= 0,
	GPIO126_PULLEN	= 1,
	GPIO126_PULL	= 1,
	GPIO126_DATAOUT	= 0,

	/* Configuration for Pin 127 */
	GPIO127_MODE	= 0,
	GPIO127_DIR	= 1,
	GPIO127_PULLEN	= 1,
	GPIO127_PULL	= 1,
	GPIO127_DATAOUT	= 1,

	/* Configuration for Pin 128 */
	GPIO128_MODE	= 2,
	GPIO128_DIR	= 1,
	GPIO128_PULLEN	= 1,
	GPIO128_PULL	= 0,
	GPIO128_DATAOUT	= 0,

	/* Configuration for Pin 129 */
	GPIO129_MODE	= 2,
	GPIO129_DIR	= 1,
	GPIO129_PULLEN	= 1,
	GPIO129_PULL	= 0,
	GPIO129_DATAOUT	= 0,

	/* Configuration for Pin 130 */
	GPIO130_MODE	= 2,
	GPIO130_DIR	= 1,
	GPIO130_PULLEN	= 1,
	GPIO130_PULL	= 0,
	GPIO130_DATAOUT	= 0,

	/* Configuration for Pin 131 */
	GPIO131_MODE	= 2,
	GPIO131_DIR	= 1,
	GPIO131_PULLEN	= 1,
	GPIO131_PULL	= 0,
	GPIO131_DATAOUT	= 0,

	/* Configuration for Pin 132 */
	GPIO132_MODE	= 3,
	GPIO132_DIR	= 0,
	GPIO132_PULLEN	= 1,
	GPIO132_PULL	= 0,
	GPIO132_DATAOUT	= 0,

	/* Configuration for Pin 133 */
	GPIO133_MODE	= 1,
	GPIO133_DIR	= 0,
	GPIO133_PULLEN	= 0,
	GPIO133_PULL	= 0,
	GPIO133_DATAOUT	= 0,

	/* Configuration for Pin 134 */
	GPIO134_MODE	= 1,
	GPIO134_DIR	= 0,
	GPIO134_PULLEN	= 0,
	GPIO134_PULL	= 0,
	GPIO134_DATAOUT	= 0,

	/* Configuration for pmic gpio */

	/* Configuration for Pin 0 */
	GPIOEXT0_MODE		= 1,
	GPIOEXT0_DIR		= 0,
	GPIOEXT0_PULLEN		= 0,
	GPIOEXT0_PULL		= 0,
	GPIOEXT0_DATAOUT	= 0,
	GPIOEXT0_DATAINV	= 0,

	/* Configuration for Pin 1 */
	GPIOEXT1_MODE		= 1,
	GPIOEXT1_DIR		= 0,
	GPIOEXT1_PULLEN		= 0,
	GPIOEXT1_PULL		= 0,
	GPIOEXT1_DATAOUT	= 0,
	GPIOEXT1_DATAINV	= 0,

	/* Configuration for Pin 2 */
	GPIOEXT2_MODE		= 1,
	GPIOEXT2_DIR		= 0,
	GPIOEXT2_PULLEN		= 0,
	GPIOEXT2_PULL		= 0,
	GPIOEXT2_DATAOUT	= 0,
	GPIOEXT2_DATAINV	= 0,

	/* Configuration for Pin 3 */
	GPIOEXT3_MODE		= 1,
	GPIOEXT3_DIR		= 0,
	GPIOEXT3_PULLEN		= 0,
	GPIOEXT3_PULL		= 0,
	GPIOEXT3_DATAOUT	= 0,
	GPIOEXT3_DATAINV	= 0,

	/* Configuration for Pin 4 */
	GPIOEXT4_MODE		= 1,
	GPIOEXT4_DIR		= 0,
	GPIOEXT4_PULLEN		= 0,
	GPIOEXT4_PULL		= 0,
	GPIOEXT4_DATAOUT	= 0,
	GPIOEXT4_DATAINV	= 0,

	/* Configuration for Pin 5 */
	GPIOEXT5_MODE		= 1,
	GPIOEXT5_DIR		= 0,
	GPIOEXT5_PULLEN		= 0,
	GPIOEXT5_PULL		= 0,
	GPIOEXT5_DATAOUT	= 0,
	GPIOEXT5_DATAINV	= 0,

	/* Configuration for Pin 6 */
	GPIOEXT6_MODE		= 1,
	GPIOEXT6_DIR		= 0,
	GPIOEXT6_PULLEN		= 1,
	GPIOEXT6_PULL		= 0,
	GPIOEXT6_DATAOUT	= 0,
	GPIOEXT6_DATAINV	= 0,

	/* Configuration for Pin 7 */
	GPIOEXT7_MODE		= 1,
	GPIOEXT7_DIR		= 0,
	GPIOEXT7_PULLEN		= 1,
	GPIOEXT7_PULL		= 0,
	GPIOEXT7_DATAOUT	= 0,
	GPIOEXT7_DATAINV	= 0,

	/* Configuration for Pin 8 */
	GPIOEXT8_MODE		= 1,
	GPIOEXT8_DIR		= 0,
	GPIOEXT8_PULLEN		= 1,
	GPIOEXT8_PULL		= 0,
	GPIOEXT8_DATAOUT	= 0,
	GPIOEXT8_DATAINV	= 0,

	/* Configuration for Pin 9 */
	GPIOEXT9_MODE		= 1,
	GPIOEXT9_DIR		= 0,
	GPIOEXT9_PULLEN		= 1,
	GPIOEXT9_PULL		= 0,
	GPIOEXT9_DATAOUT	= 0,
	GPIOEXT9_DATAINV	= 0,

	/* Configuration for Pin 10 */
	GPIOEXT10_MODE		= 1,
	GPIOEXT10_DIR		= 1,
	GPIOEXT10_PULLEN	= 1,
	GPIOEXT10_PULL		= 0,
	GPIOEXT10_DATAOUT	= 0,
	GPIOEXT10_DATAINV	= 0,

	/* Configuration for Pin 11 */
	GPIOEXT11_MODE		= 1,
	GPIOEXT11_DIR		= 0,
	GPIOEXT11_PULLEN	= 1,
	GPIOEXT11_PULL		= 0,
	GPIOEXT11_DATAOUT	= 0,
	GPIOEXT11_DATAINV	= 0,

	/* Configuration for Pin 12 */
	GPIOEXT12_MODE		= 0,
	GPIOEXT12_DIR		= 0,
	GPIOEXT12_PULLEN	= 1,
	GPIOEXT12_PULL		= 0,
	GPIOEXT12_DATAOUT	= 0,
	GPIOEXT12_DATAINV	= 0,

	/* Configuration for Pin 13 */
	GPIOEXT13_MODE		= 0,
	GPIOEXT13_DIR		= 1,
	GPIOEXT13_PULLEN	= 0,
	GPIOEXT13_PULL		= 0,
	GPIOEXT13_DATAOUT	= 0,
	GPIOEXT13_DATAINV	= 0,

	/* Configuration for Pin 14 */
	GPIOEXT14_MODE		= 0,
	GPIOEXT14_DIR		= 1,
	GPIOEXT14_PULLEN	= 1,
	GPIOEXT14_PULL		= 1,
	GPIOEXT14_DATAOUT	= 0,
	GPIOEXT14_DATAINV	= 0,

	/* Configuration for Pin 15 */
	GPIOEXT15_MODE		= 0,
	GPIOEXT15_DIR		= 1,
	GPIOEXT15_PULLEN	= 1,
	GPIOEXT15_PULL		= 0,
	GPIOEXT15_DATAOUT	= 1,
	GPIOEXT15_DATAINV	= 0,

	/* Configuration for Pin 16 */
	GPIOEXT16_MODE		= 0,
	GPIOEXT16_DIR		= 1,
	GPIOEXT16_PULLEN	= 1,
	GPIOEXT16_PULL		= 0,
	GPIOEXT16_DATAOUT	= 1,
	GPIOEXT16_DATAINV	= 0,

	/* Configuration for Pin 17 */
	GPIOEXT17_MODE		= 0,
	GPIOEXT17_DIR		= 1,
	GPIOEXT17_PULLEN	= 1,
	GPIOEXT17_PULL		= 0,
	GPIOEXT17_DATAOUT	= 1,
	GPIOEXT17_DATAINV	= 0,

	/* Configuration for Pin 18 */
	GPIOEXT18_MODE		= 0,
	GPIOEXT18_DIR		= 1,
	GPIOEXT18_PULLEN	= 1,
	GPIOEXT18_PULL		= 1,
	GPIOEXT18_DATAOUT	= 0,
	GPIOEXT18_DATAINV	= 0,

	/* Configuration for Pin 19 */
	GPIOEXT19_MODE		= 0,
	GPIOEXT19_DIR		= 1,
	GPIOEXT19_PULLEN	= 1,
	GPIOEXT19_PULL		= 0,
	GPIOEXT19_DATAOUT	= 1,
	GPIOEXT19_DATAINV	= 0,

	/* Configuration for Pin 20 */
	GPIOEXT20_MODE		= 0,
	GPIOEXT20_DIR		= 0,
	GPIOEXT20_PULLEN	= 1,
	GPIOEXT20_PULL		= 0,
	GPIOEXT20_DATAOUT	= 0,
	GPIOEXT20_DATAINV	= 0,

	/* Configuration for Pin 21 */
	GPIOEXT21_MODE		= 0,
	GPIOEXT21_DIR		= 0,
	GPIOEXT21_PULLEN	= 0,
	GPIOEXT21_PULL		= 0,
	GPIOEXT21_DATAOUT	= 0,
	GPIOEXT21_DATAINV	= 0,

	/* Configuration for Pin 22, touch pad level shift enable */
	GPIOEXT22_MODE		= 0,
	GPIOEXT22_DIR		= 1,
	GPIOEXT22_PULLEN	= 0,
	GPIOEXT22_PULL		= 1,
	GPIOEXT22_DATAOUT	= 1,
	GPIOEXT22_DATAINV	= 0,

	/* Configuration for Pin 23 */
	GPIOEXT23_MODE		= 0,
	GPIOEXT23_DIR		= 1,
	GPIOEXT23_PULLEN	= 1,
	GPIOEXT23_PULL		= 1,
	GPIOEXT23_DATAOUT	= 1,
	GPIOEXT23_DATAINV	= 0,

	/* Configuration for Pin 24 */
	GPIOEXT24_MODE		= 0,
	GPIOEXT24_DIR		= 1,
	GPIOEXT24_PULLEN	= 0,
	GPIOEXT24_PULL		= 0,
	GPIOEXT24_DATAOUT	= 1,
	GPIOEXT24_DATAINV	= 0,

	/* Configuration for Pin 25 */
	GPIOEXT25_MODE		= 0,
	GPIOEXT25_DIR		= 1,
	GPIOEXT25_PULLEN	= 0,
	GPIOEXT25_PULL		= 0,
	GPIOEXT25_DATAOUT	= 0,
	GPIOEXT25_DATAINV	= 0,

	/* Configuration for Pin 26 */
	GPIOEXT26_MODE		= 0,
	GPIOEXT26_DIR		= 0,
	GPIOEXT26_PULLEN	= 0,
	GPIOEXT26_PULL		= 0,
	GPIOEXT26_DATAOUT	= 0,
	GPIOEXT26_DATAINV	= 0,

	/* Configuration for Pin 27 */
	GPIOEXT27_MODE		= 0,
	GPIOEXT27_DIR		= 0,
	GPIOEXT27_PULLEN	= 1,
	GPIOEXT27_PULL		= 0,
	GPIOEXT27_DATAOUT	= 0,
	GPIOEXT27_DATAINV	= 0,

	/* Configuration for Pin 28 */
	GPIOEXT28_MODE		= 0,
	GPIOEXT28_DIR		= 0,
	GPIOEXT28_PULLEN	= 1,
	GPIOEXT28_PULL		= 0,
	GPIOEXT28_DATAOUT	= 0,
	GPIOEXT28_DATAINV	= 0,

	/* Configuration for Pin 29 */
	GPIOEXT29_MODE		= 0,
	GPIOEXT29_DIR		= 0,
	GPIOEXT29_PULLEN	= 1,
	GPIOEXT29_PULL		= 0,
	GPIOEXT29_DATAOUT	= 0,
	GPIOEXT29_DATAINV	= 0,

	/* Configuration for Pin 30 */
	GPIOEXT30_MODE		= 1,
	GPIOEXT30_DIR		= 0,
	GPIOEXT30_PULLEN	= 0,
	GPIOEXT30_PULL		= 0,
	GPIOEXT30_DATAOUT	= 0,
	GPIOEXT30_DATAINV	= 0,

	/* Configuration for Pin 31 */
	GPIOEXT31_MODE		= 0,
	GPIOEXT31_DIR		= 0,
	GPIOEXT31_PULLEN	= 1,
	GPIOEXT31_PULL		= 0,
	GPIOEXT31_DATAOUT	= 0,
	GPIOEXT31_DATAINV	= 0,

	/* Configuration for Pin 32 */
	GPIOEXT32_MODE		= 0,
	GPIOEXT32_DIR		= 0,
	GPIOEXT32_PULLEN	= 1,
	GPIOEXT32_PULL		= 0,
	GPIOEXT32_DATAOUT	= 0,
	GPIOEXT32_DATAINV	= 0,

	/* Configuration for Pin 33 */
	GPIOEXT33_MODE		= 0,
	GPIOEXT33_DIR		= 0,
	GPIOEXT33_PULLEN	= 1,
	GPIOEXT33_PULL		= 0,
	GPIOEXT33_DATAOUT	= 0,
	GPIOEXT33_DATAINV	= 0,

	/* Configuration for Pin 34 */
	GPIOEXT34_MODE		= 0,
	GPIOEXT34_DIR		= 0,
	GPIOEXT34_PULLEN	= 1,
	GPIOEXT34_PULL		= 0,
	GPIOEXT34_DATAOUT	= 0,
	GPIOEXT34_DATAINV	= 0,

	/* Configuration for Pin 35 */
	GPIOEXT35_MODE		= 0,
	GPIOEXT35_DIR		= 0,
	GPIOEXT35_PULLEN	= 1,
	GPIOEXT35_PULL		= 0,
	GPIOEXT35_DATAOUT	= 0,
	GPIOEXT35_DATAINV	= 0,

	/* Configuration for Pin 36 */
	GPIOEXT36_MODE		= 0,
	GPIOEXT36_DIR		= 0,
	GPIOEXT36_PULLEN	= 1,
	GPIOEXT36_PULL		= 0,
	GPIOEXT36_DATAOUT	= 0,
	GPIOEXT36_DATAINV	= 0,

	/* Configuration for Pin 37 */
	GPIOEXT37_MODE		= 0,
	GPIOEXT37_DIR		= 0,
	GPIOEXT37_PULLEN	= 1,
	GPIOEXT37_PULL		= 0,
	GPIOEXT37_DATAOUT	= 0,
	GPIOEXT37_DATAINV	= 0,

	/* Configuration for Pin 38 */
	GPIOEXT38_MODE		= 0,
	GPIOEXT38_DIR		= 0,
	GPIOEXT38_PULLEN	= 1,
	GPIOEXT38_PULL		= 0,
	GPIOEXT38_DATAOUT	= 0,
	GPIOEXT38_DATAINV	= 0,

	/* Configuration for Pin 39 */
	GPIOEXT39_MODE		= 0,
	GPIOEXT39_DIR		= 0,
	GPIOEXT39_PULLEN	= 1,
	GPIOEXT39_PULL		= 0,
	GPIOEXT39_DATAOUT	= 0,
	GPIOEXT39_DATAINV	= 0,

	/* Configuration for Pin 40 */
	GPIOEXT40_MODE		= 0,
	GPIOEXT40_DIR		= 0,
	GPIOEXT40_PULLEN	= 1,
	GPIOEXT40_PULL		= 0,
	GPIOEXT40_DATAOUT	= 0,
	GPIOEXT40_DATAINV	= 0,
};
#endif /* SOC_MEDIATEK_MT8173_CUST_GPIO_BOOT_H */
