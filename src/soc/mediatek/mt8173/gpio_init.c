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

#include <arch/io.h>
#include <console/console.h>
#include <soc/addressmap.h>
#include <soc/gpio.h>
#include <soc/pmic_wrap_init.h>
#include <mainboard/cust_gpio_boot.h>

#include <string.h>

static inline int gpioext_write (uint16_t * addr, s64 data)
{
        return pwrap_write((u32)(uintptr_t)addr, data);
}

u16 gpio_init_dir_data[] = {
	((GPIO0_DIR       <<  0) |(GPIO1_DIR       <<  1) |(GPIO2_DIR       <<  2) |(GPIO3_DIR       <<  3) |
	 (GPIO4_DIR       <<  4) |(GPIO5_DIR       <<  5) |(GPIO6_DIR       <<  6) |(GPIO7_DIR       <<  7) |
	 (GPIO8_DIR       <<  8) |(GPIO9_DIR       <<  9) |(GPIO10_DIR      << 10) |(GPIO11_DIR      << 11) |
	 (GPIO12_DIR      << 12) |(GPIO13_DIR      << 13) |(GPIO14_DIR      << 14) |(GPIO15_DIR      << 15)),

	((GPIO16_DIR      <<  0) |(GPIO17_DIR      <<  1) |(GPIO18_DIR      <<  2) |(GPIO19_DIR      <<  3) |
	 (GPIO20_DIR      <<  4) |(GPIO21_DIR      <<  5) |(GPIO22_DIR      <<  6) |(GPIO23_DIR      <<  7) |
	 (GPIO24_DIR      <<  8) |(GPIO25_DIR      <<  9) |(GPIO26_DIR      << 10) |(GPIO27_DIR      << 11) |
	 (GPIO28_DIR      << 12) |(GPIO29_DIR      << 13) |(GPIO30_DIR      << 14) |(GPIO31_DIR      << 15)),

	((GPIO32_DIR      <<  0) |(GPIO33_DIR      <<  1) |(GPIO34_DIR      <<  2) |(GPIO35_DIR      <<  3) |
	 (GPIO36_DIR      <<  4) |(GPIO37_DIR      <<  5) |(GPIO38_DIR      <<  6) |(GPIO39_DIR      <<  7) |
	 (GPIO40_DIR      <<  8) |(GPIO41_DIR      <<  9) |(GPIO42_DIR      << 10) |(GPIO43_DIR      << 11) |
	 (GPIO44_DIR      << 12) |(GPIO45_DIR      << 13) |(GPIO46_DIR      << 14) |(GPIO47_DIR      << 15)),

	((GPIO48_DIR      <<  0) |(GPIO49_DIR      <<  1) |(GPIO50_DIR      <<  2) |(GPIO51_DIR      <<  3) |
	 (GPIO52_DIR      <<  4) |(GPIO53_DIR      <<  5) |(GPIO54_DIR      <<  6) |(GPIO55_DIR      <<  7) |
	 (GPIO56_DIR      <<  8) |(GPIO57_DIR      <<  9) |(GPIO58_DIR      << 10) |(GPIO59_DIR      << 11) |
	 (GPIO60_DIR      << 12) |(GPIO61_DIR      << 13) |(GPIO62_DIR      << 14) |(GPIO63_DIR      << 15)),

	((GPIO64_DIR      <<  0) |(GPIO65_DIR      <<  1) |(GPIO66_DIR      <<  2) |(GPIO67_DIR      <<  3) |
	 (GPIO68_DIR      <<  4) |(GPIO69_DIR      <<  5) |(GPIO70_DIR      <<  6) |(GPIO71_DIR      <<  7) |
	 (GPIO72_DIR      <<  8) |(GPIO73_DIR      <<  9) |(GPIO74_DIR      << 10) |(GPIO75_DIR      << 11) |
	 (GPIO76_DIR      << 12) |(GPIO77_DIR      << 13) |(GPIO78_DIR      << 14) |(GPIO79_DIR      << 15)),

	((GPIO80_DIR      <<  0) |(GPIO81_DIR      <<  1) |(GPIO82_DIR      <<  2) |(GPIO83_DIR      <<  3) |
	 (GPIO84_DIR      <<  4) |(GPIO85_DIR      <<  5) |(GPIO86_DIR      <<  6) |(GPIO87_DIR      <<  7) |
	 (GPIO88_DIR      <<  8) |(GPIO89_DIR      <<  9) |(GPIO90_DIR      << 10) |(GPIO91_DIR      << 11) |
	 (GPIO92_DIR      << 12) |(GPIO93_DIR      << 13) |(GPIO94_DIR      << 14) |(GPIO95_DIR      << 15)),

	((GPIO96_DIR      <<  0) |(GPIO97_DIR      <<  1) |(GPIO98_DIR      <<  2) |(GPIO99_DIR      <<  3) |
	 (GPIO100_DIR     <<  4) |(GPIO101_DIR     <<  5) |(GPIO102_DIR     <<  6) |(GPIO103_DIR     <<  7) |
	 (GPIO104_DIR     <<  8) |(GPIO105_DIR     <<  9) |(GPIO106_DIR     << 10) |(GPIO107_DIR     << 11) |
	 (GPIO108_DIR     << 12) |(GPIO109_DIR     << 13) |(GPIO110_DIR     << 14) |(GPIO111_DIR     << 15)),

	((GPIO112_DIR     <<  0) |(GPIO113_DIR     <<  1) |(GPIO114_DIR     <<  2) |(GPIO115_DIR     <<  3) |
	 (GPIO116_DIR     <<  4) |(GPIO117_DIR     <<  5) |(GPIO118_DIR     <<  6) |(GPIO119_DIR     <<  7) |
	 (GPIO120_DIR     <<  8) |(GPIO121_DIR     <<  9) |(GPIO122_DIR     << 10) |(GPIO123_DIR     << 11) |
	 (GPIO124_DIR     << 12) |(GPIO125_DIR     << 13) |(GPIO126_DIR     << 14) |(GPIO127_DIR     << 15)),

	((GPIO128_DIR     <<  0) |(GPIO129_DIR     <<  1) |(GPIO130_DIR     <<  2) |(GPIO131_DIR     <<  3) |
	 (GPIO132_DIR     <<  4) |(GPIO133_DIR     <<  5) |(GPIO134_DIR     <<  6) ),
}; /* end of gpio_init_dir_data */

u16 gpio_init_pullen_data[] = {
	((GPIO0_PULLEN    <<  0) |(GPIO1_PULLEN    <<  1) |(GPIO2_PULLEN    <<  2) |(GPIO3_PULLEN    <<  3) |
	 (GPIO4_PULLEN    <<  4) |(GPIO5_PULLEN    <<  5) |(GPIO6_PULLEN    <<  6) |(GPIO7_PULLEN    <<  7) |
	 (GPIO8_PULLEN    <<  8) |(GPIO9_PULLEN    <<  9) |(GPIO10_PULLEN   << 10) |(GPIO11_PULLEN   << 11) |
	 (GPIO12_PULLEN   << 12) |(GPIO13_PULLEN   << 13) |(GPIO14_PULLEN   << 14) |(GPIO15_PULLEN   << 15)),

	((GPIO16_PULLEN   <<  0) |(GPIO17_PULLEN   <<  1) |(GPIO18_PULLEN   <<  2) |(GPIO19_PULLEN   <<  3) |
	 (GPIO20_PULLEN   <<  4) |(GPIO21_PULLEN   <<  5) |(GPIO22_PULLEN   <<  6) |(GPIO23_PULLEN   <<  7) |
	 (GPIO24_PULLEN   <<  8) |(GPIO25_PULLEN   <<  9) |(GPIO26_PULLEN   << 10) |(GPIO27_PULLEN   << 11) |
	 (GPIO28_PULLEN   << 12) |(GPIO29_PULLEN   << 13) |(GPIO30_PULLEN   << 14) |(GPIO31_PULLEN   << 15)),

	((GPIO32_PULLEN   <<  0) |(GPIO33_PULLEN   <<  1) |(GPIO34_PULLEN   <<  2) |(GPIO35_PULLEN   <<  3) |
	 (GPIO36_PULLEN   <<  4) |(GPIO37_PULLEN   <<  5) |(GPIO38_PULLEN   <<  6) |(GPIO39_PULLEN   <<  7) |
	 (GPIO40_PULLEN   <<  8) |(GPIO41_PULLEN   <<  9) |(GPIO42_PULLEN   << 10) |(GPIO43_PULLEN   << 11) |
	 (GPIO44_PULLEN   << 12) |(GPIO45_PULLEN   << 13) |(GPIO46_PULLEN   << 14) |(GPIO47_PULLEN   << 15)),

	((GPIO48_PULLEN   <<  0) |(GPIO49_PULLEN   <<  1) |(GPIO50_PULLEN   <<  2) |(GPIO51_PULLEN   <<  3) |
	 (GPIO52_PULLEN   <<  4) |(GPIO53_PULLEN   <<  5) |(GPIO54_PULLEN   <<  6) |(GPIO55_PULLEN   <<  7) |
	 (GPIO56_PULLEN   <<  8) |(GPIO57_PULLEN   <<  9) |(GPIO58_PULLEN   << 10) |(GPIO59_PULLEN   << 11) |
	 (GPIO60_PULLEN   << 12) |(GPIO61_PULLEN   << 13) |(GPIO62_PULLEN   << 14) |(GPIO63_PULLEN   << 15)),

	((GPIO64_PULLEN   <<  0) |(GPIO65_PULLEN   <<  1) |(GPIO66_PULLEN   <<  2) |(GPIO67_PULLEN   <<  3) |
	 (GPIO68_PULLEN   <<  4) |(GPIO69_PULLEN   <<  5) |(GPIO70_PULLEN   <<  6) |(GPIO71_PULLEN   <<  7) |
	 (GPIO72_PULLEN   <<  8) |(GPIO73_PULLEN   <<  9) |(GPIO74_PULLEN   << 10) |(GPIO75_PULLEN   << 11) |
	 (GPIO76_PULLEN   << 12) |(GPIO77_PULLEN   << 13) |(GPIO78_PULLEN   << 14) |(GPIO79_PULLEN   << 15)),

	((GPIO80_PULLEN   <<  0) |(GPIO81_PULLEN   <<  1) |(GPIO82_PULLEN   <<  2) |(GPIO83_PULLEN   <<  3) |
	 (GPIO84_PULLEN   <<  4) |(GPIO85_PULLEN   <<  5) |(GPIO86_PULLEN   <<  6) |(GPIO87_PULLEN   <<  7) |
	 (GPIO88_PULLEN   <<  8) |(GPIO89_PULLEN   <<  9) |(GPIO90_PULLEN   << 10) |(GPIO91_PULLEN   << 11) |
	 (GPIO92_PULLEN   << 12) |(GPIO93_PULLEN   << 13) |(GPIO94_PULLEN   << 14) |(GPIO95_PULLEN   << 15)),

	((GPIO96_PULLEN   <<  0) |(GPIO97_PULLEN   <<  1) |(GPIO98_PULLEN   <<  2) |(GPIO99_PULLEN   <<  3) |
	 (GPIO100_PULLEN  <<  4) |(GPIO101_PULLEN  <<  5) |(GPIO102_PULLEN  <<  6) |(GPIO103_PULLEN  <<  7) |
	 (GPIO104_PULLEN  <<  8) |(GPIO105_PULLEN  <<  9) |(GPIO106_PULLEN  << 10) |(GPIO107_PULLEN  << 11) |
	 (GPIO108_PULLEN  << 12) |(GPIO109_PULLEN  << 13) |(GPIO110_PULLEN  << 14) |(GPIO111_PULLEN  << 15)),

	((GPIO112_PULLEN  <<  0) |(GPIO113_PULLEN  <<  1) |(GPIO114_PULLEN  <<  2) |(GPIO115_PULLEN  <<  3) |
	 (GPIO116_PULLEN  <<  4) |(GPIO117_PULLEN  <<  5) |(GPIO118_PULLEN  <<  6) |(GPIO119_PULLEN  <<  7) |
	 (GPIO120_PULLEN  <<  8) |(GPIO121_PULLEN  <<  9) |(GPIO122_PULLEN  << 10) |(GPIO123_PULLEN  << 11) |
	 (GPIO124_PULLEN  << 12) |(GPIO125_PULLEN  << 13) |(GPIO126_PULLEN  << 14) |(GPIO127_PULLEN  << 15)),

	((GPIO128_PULLEN  <<  0) |(GPIO129_PULLEN  <<  1) |(GPIO130_PULLEN  <<  2) |(GPIO131_PULLEN  <<  3) |
	 (GPIO132_PULLEN  <<  4) |(GPIO133_PULLEN  <<  5) |(GPIO134_PULLEN  <<  6)),
}; /* end of gpio_init_pullen_data */

u16 gpio_init_pullsel_data[] = {
	((GPIO0_PULL      <<  0) |(GPIO1_PULL      <<  1) |(GPIO2_PULL      <<  2) |(GPIO3_PULL      <<  3) |
	 (GPIO4_PULL      <<  4) |(GPIO5_PULL      <<  5) |(GPIO6_PULL      <<  6) |(GPIO7_PULL      <<  7) |
	 (GPIO8_PULL      <<  8) |(GPIO9_PULL      <<  9) |(GPIO10_PULL     << 10) |(GPIO11_PULL     << 11) |
	 (GPIO12_PULL     << 12) |(GPIO13_PULL     << 13) |(GPIO14_PULL     << 14) |(GPIO15_PULL     << 15)),

	((GPIO16_PULL     <<  0) |(GPIO17_PULL     <<  1) |(GPIO18_PULL     <<  2) |(GPIO19_PULL     <<  3) |
	 (GPIO20_PULL     <<  4) |(GPIO21_PULL     <<  5) |(GPIO22_PULL     <<  6) |(GPIO23_PULL     <<  7) |
	 (GPIO24_PULL     <<  8) |(GPIO25_PULL     <<  9) |(GPIO26_PULL     << 10) |(GPIO27_PULL     << 11) |
	 (GPIO28_PULL     << 12) |(GPIO29_PULL     << 13) |(GPIO30_PULL     << 14) |(GPIO31_PULL     << 15)),

	((GPIO32_PULL     <<  0) |(GPIO33_PULL     <<  1) |(GPIO34_PULL     <<  2) |(GPIO35_PULL     <<  3) |
	 (GPIO36_PULL     <<  4) |(GPIO37_PULL     <<  5) |(GPIO38_PULL     <<  6) |(GPIO39_PULL     <<  7) |
	 (GPIO40_PULL     <<  8) |(GPIO41_PULL     <<  9) |(GPIO42_PULL     << 10) |(GPIO43_PULL     << 11) |
	 (GPIO44_PULL     << 12) |(GPIO45_PULL     << 13) |(GPIO46_PULL     << 14) |(GPIO47_PULL     << 15)),

	((GPIO48_PULL     <<  0) |(GPIO49_PULL     <<  1) |(GPIO50_PULL     <<  2) |(GPIO51_PULL     <<  3) |
	 (GPIO52_PULL     <<  4) |(GPIO53_PULL     <<  5) |(GPIO54_PULL     <<  6) |(GPIO55_PULL     <<  7) |
	 (GPIO56_PULL     <<  8) |(GPIO57_PULL     <<  9) |(GPIO58_PULL     << 10) |(GPIO59_PULL     << 11) |
	 (GPIO60_PULL     << 12) |(GPIO61_PULL     << 13) |(GPIO62_PULL     << 14) |(GPIO63_PULL     << 15)),

	((GPIO64_PULL     <<  0) |(GPIO65_PULL     <<  1) |(GPIO66_PULL     <<  2) |(GPIO67_PULL     <<  3) |
	 (GPIO68_PULL     <<  4) |(GPIO69_PULL     <<  5) |(GPIO70_PULL     <<  6) |(GPIO71_PULL     <<  7) |
	 (GPIO72_PULL     <<  8) |(GPIO73_PULL     <<  9) |(GPIO74_PULL     << 10) |(GPIO75_PULL     << 11) |
	 (GPIO76_PULL     << 12) |(GPIO77_PULL     << 13) |(GPIO78_PULL     << 14) |(GPIO79_PULL     << 15)),

	((GPIO80_PULL     <<  0) |(GPIO81_PULL     <<  1) |(GPIO82_PULL     <<  2) |(GPIO83_PULL     <<  3) |
	 (GPIO84_PULL     <<  4) |(GPIO85_PULL     <<  5) |(GPIO86_PULL     <<  6) |(GPIO87_PULL     <<  7) |
	 (GPIO88_PULL     <<  8) |(GPIO89_PULL     <<  9) |(GPIO90_PULL     << 10) |(GPIO91_PULL     << 11) |
	 (GPIO92_PULL     << 12) |(GPIO93_PULL     << 13) |(GPIO94_PULL     << 14) |(GPIO95_PULL     << 15)),

	((GPIO96_PULL     <<  0) |(GPIO97_PULL     <<  1) |(GPIO98_PULL     <<  2) |(GPIO99_PULL     <<  3) |
	 (GPIO100_PULL    <<  4) |(GPIO101_PULL    <<  5) |(GPIO102_PULL    <<  6) |(GPIO103_PULL    <<  7) |
	 (GPIO104_PULL    <<  8) |(GPIO105_PULL    <<  9) |(GPIO106_PULL    << 10) |(GPIO107_PULL    << 11) |
	 (GPIO108_PULL    << 12) |(GPIO109_PULL    << 13) |(GPIO110_PULL    << 14) |(GPIO111_PULL    << 15)),

	((GPIO112_PULL    <<  0) |(GPIO113_PULL    <<  1) |(GPIO114_PULL    <<  2) |(GPIO115_PULL    <<  3) |
	 (GPIO116_PULL    <<  4) |(GPIO117_PULL    <<  5) |(GPIO118_PULL    <<  6) |(GPIO119_PULL    <<  7) |
	 (GPIO120_PULL    <<  8) |(GPIO121_PULL    <<  9) |(GPIO122_PULL    << 10) |(GPIO123_PULL    << 11) |
	 (GPIO124_PULL    << 12) |(GPIO125_PULL    << 13) |(GPIO126_PULL    << 14) |(GPIO127_PULL    << 15)),

	((GPIO128_PULL    <<  0) |(GPIO129_PULL    <<  1) |(GPIO130_PULL    <<  2) |(GPIO131_PULL    <<  3) |
	 (GPIO132_PULL    <<  4) |(GPIO133_PULL    <<  5) |(GPIO134_PULL    <<  6)),
}; /* end of gpio_init_pullsel_data */

u16 gpio_init_dout_data[] = {
	((GPIO0_DATAOUT   <<  0) |(GPIO1_DATAOUT   <<  1) |(GPIO2_DATAOUT   <<  2) |(GPIO3_DATAOUT   <<  3) |
	 (GPIO4_DATAOUT   <<  4) |(GPIO5_DATAOUT   <<  5) |(GPIO6_DATAOUT   <<  6) |(GPIO7_DATAOUT   <<  7) |
	 (GPIO8_DATAOUT   <<  8) |(GPIO9_DATAOUT   <<  9) |(GPIO10_DATAOUT  << 10) |(GPIO11_DATAOUT  << 11) |
	 (GPIO12_DATAOUT  << 12) |(GPIO13_DATAOUT  << 13) |(GPIO14_DATAOUT  << 14) |(GPIO15_DATAOUT  << 15)),

	((GPIO16_DATAOUT  <<  0) |(GPIO17_DATAOUT  <<  1) |(GPIO18_DATAOUT  <<  2) |(GPIO19_DATAOUT  <<  3) |
	 (GPIO20_DATAOUT  <<  4) |(GPIO21_DATAOUT  <<  5) |(GPIO22_DATAOUT  <<  6) |(GPIO23_DATAOUT  <<  7) |
	 (GPIO24_DATAOUT  <<  8) |(GPIO25_DATAOUT  <<  9) |(GPIO26_DATAOUT  << 10) |(GPIO27_DATAOUT  << 11) |
	 (GPIO28_DATAOUT  << 12) |(GPIO29_DATAOUT  << 13) |(GPIO30_DATAOUT  << 14) |(GPIO31_DATAOUT  << 15)),

	((GPIO32_DATAOUT  <<  0) |(GPIO33_DATAOUT  <<  1) |(GPIO34_DATAOUT  <<  2) |(GPIO35_DATAOUT  <<  3) |
	 (GPIO36_DATAOUT  <<  4) |(GPIO37_DATAOUT  <<  5) |(GPIO38_DATAOUT  <<  6) |(GPIO39_DATAOUT  <<  7) |
	 (GPIO40_DATAOUT  <<  8) |(GPIO41_DATAOUT  <<  9) |(GPIO42_DATAOUT  << 10) |(GPIO43_DATAOUT  << 11) |
	 (GPIO44_DATAOUT  << 12) |(GPIO45_DATAOUT  << 13) |(GPIO46_DATAOUT  << 14) |(GPIO47_DATAOUT  << 15)),

	((GPIO48_DATAOUT  <<  0) |(GPIO49_DATAOUT  <<  1) |(GPIO50_DATAOUT  <<  2) |(GPIO51_DATAOUT  <<  3) |
	 (GPIO52_DATAOUT  <<  4) |(GPIO53_DATAOUT  <<  5) |(GPIO54_DATAOUT  <<  6) |(GPIO55_DATAOUT  <<  7) |
	 (GPIO56_DATAOUT  <<  8) |(GPIO57_DATAOUT  <<  9) |(GPIO58_DATAOUT  << 10) |(GPIO59_DATAOUT  << 11) |
	 (GPIO60_DATAOUT  << 12) |(GPIO61_DATAOUT  << 13) |(GPIO62_DATAOUT  << 14) |(GPIO63_DATAOUT  << 15)),

	((GPIO64_DATAOUT  <<  0) |(GPIO65_DATAOUT  <<  1) |(GPIO66_DATAOUT  <<  2) |(GPIO67_DATAOUT  <<  3) |
	 (GPIO68_DATAOUT  <<  4) |(GPIO69_DATAOUT  <<  5) |(GPIO70_DATAOUT  <<  6) |(GPIO71_DATAOUT  <<  7) |
	 (GPIO72_DATAOUT  <<  8) |(GPIO73_DATAOUT  <<  9) |(GPIO74_DATAOUT  << 10) |(GPIO75_DATAOUT  << 11) |
	 (GPIO76_DATAOUT  << 12) |(GPIO77_DATAOUT  << 13) |(GPIO78_DATAOUT  << 14) |(GPIO79_DATAOUT  << 15)),

	((GPIO80_DATAOUT  <<  0) |(GPIO81_DATAOUT  <<  1) |(GPIO82_DATAOUT  <<  2) |(GPIO83_DATAOUT  <<  3) |
	 (GPIO84_DATAOUT  <<  4) |(GPIO85_DATAOUT  <<  5) |(GPIO86_DATAOUT  <<  6) |(GPIO87_DATAOUT  <<  7) |
	 (GPIO88_DATAOUT  <<  8) |(GPIO89_DATAOUT  <<  9) |(GPIO90_DATAOUT  << 10) |(GPIO91_DATAOUT  << 11) |
	 (GPIO92_DATAOUT  << 12) |(GPIO93_DATAOUT  << 13) |(GPIO94_DATAOUT  << 14) |(GPIO95_DATAOUT  << 15)),

	((GPIO96_DATAOUT  <<  0) |(GPIO97_DATAOUT  <<  1) |(GPIO98_DATAOUT  <<  2) |(GPIO99_DATAOUT  <<  3) |
	 (GPIO100_DATAOUT <<  4) |(GPIO101_DATAOUT <<  5) |(GPIO102_DATAOUT <<  6) |(GPIO103_DATAOUT <<  7) |
	 (GPIO104_DATAOUT <<  8) |(GPIO105_DATAOUT <<  9) |(GPIO106_DATAOUT << 10) |(GPIO107_DATAOUT << 11) |
	 (GPIO108_DATAOUT << 12) |(GPIO109_DATAOUT << 13) |(GPIO110_DATAOUT << 14) |(GPIO111_DATAOUT << 15)),

	((GPIO112_DATAOUT <<  0) |(GPIO113_DATAOUT <<  1) |(GPIO114_DATAOUT <<  2) |(GPIO115_DATAOUT <<  3) |
	 (GPIO116_DATAOUT <<  4) |(GPIO117_DATAOUT <<  5) |(GPIO118_DATAOUT <<  6) |(GPIO119_DATAOUT <<  7) |
	 (GPIO120_DATAOUT <<  8) |(GPIO121_DATAOUT <<  9) |(GPIO122_DATAOUT << 10) |(GPIO123_DATAOUT << 11) |
	 (GPIO124_DATAOUT << 12) |(GPIO125_DATAOUT << 13) |(GPIO126_DATAOUT << 14) |(GPIO127_DATAOUT << 15)),

	((GPIO128_DATAOUT <<  0) |(GPIO129_DATAOUT <<  1) |(GPIO130_DATAOUT <<  2) |(GPIO131_DATAOUT <<  3) |
	 (GPIO132_DATAOUT <<  4) |(GPIO133_DATAOUT <<  5) |(GPIO134_DATAOUT <<  6)),
}; /* end of gpio_init_dout_data*/

u16 gpio_init_mode_data[] = {
	((GPIO0_MODE      <<  0) |(GPIO1_MODE      <<  3) |(GPIO2_MODE      <<  6) |(GPIO3_MODE      <<  9) |(GPIO4_MODE      << 12)),
	((GPIO5_MODE      <<  0) |(GPIO6_MODE      <<  3) |(GPIO7_MODE      <<  6) |(GPIO8_MODE      <<  9) |(GPIO9_MODE      << 12)),
	((GPIO10_MODE     <<  0) |(GPIO11_MODE     <<  3) |(GPIO12_MODE     <<  6) |(GPIO13_MODE     <<  9) |(GPIO14_MODE     << 12)),
	((GPIO15_MODE     <<  0) |(GPIO16_MODE     <<  3) |(GPIO17_MODE     <<  6) |(GPIO18_MODE     <<  9) |(GPIO19_MODE     << 12)),
	((GPIO20_MODE     <<  0) |(GPIO21_MODE     <<  3) |(GPIO22_MODE     <<  6) |(GPIO23_MODE     <<  9) |(GPIO24_MODE     << 12)),
	((GPIO25_MODE     <<  0) |(GPIO26_MODE     <<  3) |(GPIO27_MODE     <<  6) |(GPIO28_MODE     <<  9) |(GPIO29_MODE     << 12)),
	((GPIO30_MODE     <<  0) |(GPIO31_MODE     <<  3) |(GPIO32_MODE     <<  6) |(GPIO33_MODE     <<  9) |(GPIO34_MODE     << 12)),
	((GPIO35_MODE     <<  0) |(GPIO36_MODE     <<  3) |(GPIO37_MODE     <<  6) |(GPIO38_MODE     <<  9) |(GPIO39_MODE     << 12)),
	((GPIO40_MODE     <<  0) |(GPIO41_MODE     <<  3) |(GPIO42_MODE     <<  6) |(GPIO43_MODE     <<  9) |(GPIO44_MODE     << 12)),
	((GPIO45_MODE     <<  0) |(GPIO46_MODE     <<  3) |(GPIO47_MODE     <<  6) |(GPIO48_MODE     <<  9) |(GPIO49_MODE     << 12)),
	((GPIO50_MODE     <<  0) |(GPIO51_MODE     <<  3) |(GPIO52_MODE     <<  6) |(GPIO53_MODE     <<  9) |(GPIO54_MODE     << 12)),
	((GPIO55_MODE     <<  0) |(GPIO56_MODE     <<  3) |(GPIO57_MODE     <<  6) |(GPIO58_MODE     <<  9) |(GPIO59_MODE     << 12)),
	((GPIO60_MODE     <<  0) |(GPIO61_MODE     <<  3) |(GPIO62_MODE     <<  6) |(GPIO63_MODE     <<  9) |(GPIO64_MODE     << 12)),
	((GPIO65_MODE     <<  0) |(GPIO66_MODE     <<  3) |(GPIO67_MODE     <<  6) |(GPIO68_MODE     <<  9) |(GPIO69_MODE     << 12)),
	((GPIO70_MODE     <<  0) |(GPIO71_MODE     <<  3) |(GPIO72_MODE     <<  6) |(GPIO73_MODE     <<  9) |(GPIO74_MODE     << 12)),
	((GPIO75_MODE     <<  0) |(GPIO76_MODE     <<  3) |(GPIO77_MODE     <<  6) |(GPIO78_MODE     <<  9) |(GPIO79_MODE     << 12)),
	((GPIO80_MODE     <<  0) |(GPIO81_MODE     <<  3) |(GPIO82_MODE     <<  6) |(GPIO83_MODE     <<  9) |(GPIO84_MODE     << 12)),
	((GPIO85_MODE     <<  0) |(GPIO86_MODE     <<  3) |(GPIO87_MODE     <<  6) |(GPIO88_MODE     <<  9) |(GPIO89_MODE     << 12)),
	((GPIO90_MODE     <<  0) |(GPIO91_MODE     <<  3) |(GPIO92_MODE     <<  6) |(GPIO93_MODE     <<  9) |(GPIO94_MODE     << 12)),
	((GPIO95_MODE     <<  0) |(GPIO96_MODE     <<  3) |(GPIO97_MODE     <<  6) |(GPIO98_MODE     <<  9) |(GPIO99_MODE     << 12)),
	((GPIO100_MODE    <<  0) |(GPIO101_MODE    <<  3) |(GPIO102_MODE    <<  6) |(GPIO103_MODE    <<  9) |(GPIO104_MODE    << 12)),
	((GPIO105_MODE    <<  0) |(GPIO106_MODE    <<  3) |(GPIO107_MODE    <<  6) |(GPIO108_MODE    <<  9) |(GPIO109_MODE    << 12)),
	((GPIO110_MODE    <<  0) |(GPIO111_MODE    <<  3) |(GPIO112_MODE    <<  6) |(GPIO113_MODE    <<  9) |(GPIO114_MODE    << 12)),
	((GPIO115_MODE    <<  0) |(GPIO116_MODE    <<  3) |(GPIO117_MODE    <<  6) |(GPIO118_MODE    <<  9) |(GPIO119_MODE    << 12)),
	((GPIO120_MODE    <<  0) |(GPIO121_MODE    <<  3) |(GPIO122_MODE    <<  6) |(GPIO123_MODE    <<  9) |(GPIO124_MODE    << 12)),
	((GPIO125_MODE    <<  0) |(GPIO126_MODE    <<  3) |(GPIO127_MODE    <<  6) |(GPIO128_MODE    <<  9) |(GPIO129_MODE    << 12)),
	((GPIO130_MODE    <<  0) |(GPIO131_MODE    <<  3) |(GPIO132_MODE    <<  6) |(GPIO133_MODE    <<  9) |(GPIO134_MODE    << 12)),
}; /* end of gpio_init_mode_more_data */

u16 gpio_init_smt_data[] = {
	((GPIO_SMT_GROUP_0  <<  0) |(GPIO_SMT_GROUP_1  <<  1) |(GPIO_SMT_GROUP_2  <<  2) |(GPIO_SMT_GROUP_3  <<  3) |
	 (GPIO_SMT_GROUP_4  <<  4) |(GPIO_SMT_GROUP_5  <<  5) |(GPIO_SMT_GROUP_6  <<  6) |(GPIO_SMT_GROUP_7  <<  7) |
	 (0  <<  8)                |(GPIO_SMT_GROUP_9  <<  9) |(GPIO_SMT_GROUP_10 << 10) |(GPIO_SMT_GROUP_11 << 11) |
	 (GPIO_SMT_GROUP_12 << 12) |(GPIO_SMT_GROUP_13 << 13) |(GPIO_SMT_GROUP_14 << 14) |(GPIO_SMT_GROUP_15 << 15)),

	((GPIO_SMT_GROUP_16 <<  1) |(GPIO_SMT_GROUP_17 <<  1) |(GPIO_SMT_GROUP_18 <<  2) |(GPIO_SMT_GROUP_19 <<  3) |
	 (GPIO_SMT_GROUP_20 <<  4) |(GPIO_SMT_GROUP_21 <<  5) |(GPIO_SMT_GROUP_22 <<  6) |(GPIO_SMT_GROUP_23 <<  7) |
	 (GPIO_SMT_GROUP_24 <<  8) |(0 <<  9)                 |(GPIO_SMT_GROUP_26 << 10) |(0 << 11)                 |
	 (0 << 12)                 |(0 << 13)                 |(GPIO_SMT_GROUP_30 << 14) |(GPIO_SMT_GROUP_31 << 15)),

	((GPIO_SMT_GROUP_32 <<  0) |(GPIO_SMT_GROUP_33 <<  1) |(GPIO_SMT_GROUP_34 <<  2) |(0 <<  3)                 |
	 (0 <<  4)                 |(0 <<  5)                 |(0 <<  6)                 |(0 <<  7)                 |
	 (GPIO_SMT_GROUP_40 <<  8) |(GPIO_SMT_GROUP_41 <<  9)),
}; /* end of gpio_init_smt_more_data */

/*
 * As GPIO53, GPIO55, GPIO56 is configured as GPI
 * we need set mipi registers to switch them from mipi function
 */
static void mt_set_gpi_from_mipi(void)
{
	 u32 reg;

	 reg = read32((void *)MTK_MCUCFG_BASE + 0x1804C);
	 reg = reg | 0x1041041;
	 write32((void *)MTK_MCUCFG_BASE + 0x1804C, reg);

	 reg = read32((void *)MTK_MCUCFG_BASE + 0x18050);
	 reg = reg | 0x1041041;
	 write32((void *)MTK_MCUCFG_BASE + 0x18050, reg);
}

static void mt_gpio_set_default_chip(void)
{
	int idx;
	u32 val;

	for (idx = 0; idx < sizeof(gpio_obj->dir)/sizeof(gpio_obj->dir[0]); idx++){
		val = gpio_init_dir_data[idx];
		write16(&gpio_obj->dir[idx], val);
	}
	for (idx = 0; idx < sizeof(gpio_obj->pullen)/sizeof(gpio_obj->pullen[0]); idx++){
		val = gpio_init_pullen_data[idx];
		write16(&gpio_obj->pullen[idx], val);
	}
	for (idx = 0; idx < sizeof(gpio_obj->pullsel)/sizeof(gpio_obj->pullsel[0]); idx++){
		val = gpio_init_pullsel_data[idx];
		write16(&gpio_obj->pullsel[idx], val);
	}
	/* for keypad special register */
	mt_set_gpio_pull(119, GPIO119_PULLEN, GPIO119_PULL);
	mt_set_gpio_pull(120, GPIO120_PULLEN, GPIO120_PULL);
	mt_set_gpio_pull(121, GPIO121_PULLEN, GPIO121_PULL);
	mt_set_gpio_pull(122, GPIO122_PULLEN, GPIO122_PULL);
	mt_set_gpio_pull(123, GPIO123_PULLEN, GPIO123_PULL);
	mt_set_gpio_pull(124, GPIO124_PULLEN, GPIO124_PULL);

	for (idx = 0; idx < sizeof(gpio_obj->dout)/sizeof(gpio_obj->dout[0]); idx++) {
		val = gpio_init_dout_data[idx];
		write16(&gpio_obj->dout[idx], val);
	}
	for (idx = 0; idx < sizeof(gpio_obj->mode)/sizeof(gpio_obj->mode[0]); idx++) {
		val = gpio_init_mode_data[idx];
		write16(&gpio_obj->mode[idx], val);
	}
	for (idx = 0; idx < sizeof(gpio_obj->smt)/sizeof(gpio_obj->smt[0]); idx++) {
		val = gpio_init_smt_data[idx];
		write16(&gpio_obj->smt[idx], val);
	}

	mt_set_gpi_from_mipi();

}

u16 gpioext_init_dir_data[4] = {
	((GPIOEXT0_DIR << 0) | (GPIOEXT1_DIR << 1) | (GPIOEXT2_DIR << 2) |
	 (GPIOEXT3_DIR << 3) | (GPIOEXT4_DIR << 4) | (GPIOEXT5_DIR << 5) |
	 (GPIOEXT6_DIR << 6) | (GPIOEXT7_DIR << 7) | (GPIOEXT8_DIR << 8) |
	 (GPIOEXT9_DIR << 9) | (GPIOEXT10_DIR << 10) | (GPIOEXT11_DIR << 11) |
	 (GPIOEXT12_DIR << 12) | (GPIOEXT13_DIR << 13) | (GPIOEXT14_DIR << 14) |
	 (GPIOEXT15_DIR << 15)),

	((GPIOEXT16_DIR << 0) | (GPIOEXT17_DIR << 1) | (GPIOEXT18_DIR << 2) |
	 (GPIOEXT19_DIR << 3) | (GPIOEXT20_DIR << 4) | (GPIOEXT21_DIR << 5) |
	 (GPIOEXT22_DIR << 6) | (GPIOEXT23_DIR << 7) | (GPIOEXT24_DIR << 8) |
	 (GPIOEXT25_DIR << 9) | (GPIOEXT26_DIR << 10) | (GPIOEXT27_DIR << 11) |
	 (GPIOEXT28_DIR << 12) | (GPIOEXT29_DIR << 13) | (GPIOEXT30_DIR << 14) |
	 (GPIOEXT31_DIR << 15)),

	((GPIOEXT32_DIR << 0) | (GPIOEXT33_DIR << 1) | (GPIOEXT34_DIR << 2) |
	 (GPIOEXT35_DIR << 3) | (GPIOEXT36_DIR << 4) | (GPIOEXT37_DIR << 5) |
	 (GPIOEXT38_DIR << 6) | (GPIOEXT39_DIR << 7) | (GPIOEXT40_DIR << 8)),
}; /* end of gpioext_init_dir_data */

u16 gpioext_init_pullen_data[4] = {
	((GPIOEXT0_PULLEN << 0) | (GPIOEXT1_PULLEN << 1) | (GPIOEXT2_PULLEN << 2) |
	 (GPIOEXT3_PULLEN << 3) | (GPIOEXT4_PULLEN << 4) | (GPIOEXT5_PULLEN << 5) |
	 (GPIOEXT6_PULLEN << 6) | (GPIOEXT7_PULLEN << 7) | (GPIOEXT8_PULLEN << 8) |
	 (GPIOEXT9_PULLEN << 9) | (GPIOEXT10_PULLEN << 10) | (GPIOEXT11_PULLEN << 11) |
	 (GPIOEXT12_PULLEN << 12) | (GPIOEXT13_PULLEN << 13) | (GPIOEXT14_PULLEN << 14) |
	 (GPIOEXT15_PULLEN << 15)),

	((GPIOEXT16_PULLEN << 0) | (GPIOEXT17_PULLEN << 1) | (GPIOEXT18_PULLEN << 2) |
	 (GPIOEXT19_PULLEN << 3) | (GPIOEXT20_PULLEN << 4) | (GPIOEXT21_PULLEN << 5) |
	 (GPIOEXT22_PULLEN << 6) | (GPIOEXT23_PULLEN << 7) | (GPIOEXT24_PULLEN << 8) |
	 (GPIOEXT25_PULLEN << 9) | (GPIOEXT26_PULLEN << 10) | (GPIOEXT27_PULLEN << 11) |
	 (GPIOEXT28_PULLEN << 12) | (GPIOEXT29_PULLEN << 13) | (GPIOEXT30_PULLEN << 14) |
	 (GPIOEXT31_PULLEN << 15)),

	((GPIOEXT32_PULLEN << 0) | (GPIOEXT33_PULLEN << 1) | (GPIOEXT34_PULLEN << 2) |
	 (GPIOEXT35_PULLEN << 3) | (GPIOEXT36_PULLEN << 4) | (GPIOEXT37_PULLEN << 5) |
	 (GPIOEXT38_PULLEN << 6) | (GPIOEXT39_PULLEN << 7) | (GPIOEXT40_PULLEN << 8)),
}; /* end of gpioext_init_pullen_data */

u16 gpioext_init_pullsel_data[4] = {
	((GPIOEXT0_PULL << 0) | (GPIOEXT1_PULL << 1) | (GPIOEXT2_PULL << 2) |
	 (GPIOEXT3_PULL << 3) | (GPIOEXT4_PULL << 4) | (GPIOEXT5_PULL << 5) |
	 (GPIOEXT6_PULL << 6) | (GPIOEXT7_PULL << 7) | (GPIOEXT8_PULL << 8) |
	 (GPIOEXT9_PULL << 9) | (GPIOEXT10_PULL << 10) | (GPIOEXT11_PULL << 11)	 |
	 (GPIOEXT12_PULL << 12) | (GPIOEXT13_PULL << 13) | (GPIOEXT14_PULL << 14) |
	 (GPIOEXT15_PULL << 15)),

	((GPIOEXT16_PULL << 0) | (GPIOEXT17_PULL << 1) | (GPIOEXT18_PULL << 2) |
	 (GPIOEXT19_PULL << 3) | (GPIOEXT20_PULL << 4) | (GPIOEXT21_PULL << 5) |
	 (GPIOEXT22_PULL << 6) | (GPIOEXT23_PULL << 7) | (GPIOEXT24_PULL << 8) |
	 (GPIOEXT25_PULL << 9) | (GPIOEXT26_PULL << 10) | (GPIOEXT27_PULL << 11) |
	 (GPIOEXT28_PULL << 12) | (GPIOEXT29_PULL << 13) | (GPIOEXT30_PULL << 14) |
	 (GPIOEXT31_PULL << 15)),

	((GPIOEXT32_PULL << 0) | (GPIOEXT33_PULL << 1) | (GPIOEXT34_PULL << 2) |
	 (GPIOEXT35_PULL << 3) | (GPIOEXT36_PULL << 4) | (GPIOEXT37_PULL << 5) |
	 (GPIOEXT38_PULL << 6) | (GPIOEXT39_PULL << 7) | (GPIOEXT40_PULL << 8)),
}; /* end of gpioext_init_pullsel_data */

u16 gpioext_init_dinv_data[4] = {
	((GPIOEXT0_DATAINV << 0) | (GPIOEXT1_DATAINV << 1) | (GPIOEXT2_DATAINV << 2) |
	 (GPIOEXT3_DATAINV << 3) | (GPIOEXT4_DATAINV << 4) | (GPIOEXT5_DATAINV << 5) |
	 (GPIOEXT6_DATAINV << 6) | (GPIOEXT7_DATAINV << 7) | (GPIOEXT8_DATAINV << 8) |
	 (GPIOEXT9_DATAINV << 9) | (GPIOEXT10_DATAINV << 10) | (GPIOEXT11_DATAINV << 11) |
	 (GPIOEXT12_DATAINV << 12) | (GPIOEXT13_DATAINV << 13) | (GPIOEXT14_DATAINV << 14) |
	 (GPIOEXT15_DATAINV << 15)),

	((GPIOEXT16_DATAINV << 0) | (GPIOEXT17_DATAINV << 1) |
	 (GPIOEXT18_DATAINV << 2) | (GPIOEXT19_DATAINV << 3) |
	 (GPIOEXT20_DATAINV << 4) | (GPIOEXT21_DATAINV << 5) |
	 (GPIOEXT22_DATAINV << 6) | (GPIOEXT23_DATAINV << 7) |
	 (GPIOEXT24_DATAINV << 8) | (GPIOEXT25_DATAINV << 9) |
	 (GPIOEXT26_DATAINV << 10) | (GPIOEXT27_DATAINV << 11) |
	 (GPIOEXT28_DATAINV << 12) | (GPIOEXT29_DATAINV << 13) |
	 (GPIOEXT30_DATAINV << 14) | (GPIOEXT31_DATAINV << 15)),

	((GPIOEXT32_DATAINV << 0) | (GPIOEXT33_DATAINV << 1) |
	 (GPIOEXT34_DATAINV << 2) | (GPIOEXT35_DATAINV << 3) |
	 (GPIOEXT36_DATAINV << 4) | (GPIOEXT37_DATAINV << 5) |
	 (GPIOEXT38_DATAINV << 6) | (GPIOEXT39_DATAINV << 7) |
	 (GPIOEXT40_DATAINV << 8)),
}; /* end of gpioext_init_dinv_data */

u16 gpioext_init_dout_data[4] = {
	((GPIOEXT0_DATAOUT << 0) | (GPIOEXT1_DATAOUT << 1) |
	 (GPIOEXT2_DATAOUT << 2) | (GPIOEXT3_DATAOUT << 3) |
	 (GPIOEXT4_DATAOUT << 4) | (GPIOEXT5_DATAOUT << 5) |
	 (GPIOEXT6_DATAOUT << 6) | (GPIOEXT7_DATAOUT << 7) |
	 (GPIOEXT8_DATAOUT << 8) | (GPIOEXT9_DATAOUT << 9) |
	 (GPIOEXT10_DATAOUT << 10) | (GPIOEXT11_DATAOUT << 11) |
	 (GPIOEXT12_DATAOUT << 12) | (GPIOEXT13_DATAOUT << 13) |
	 (GPIOEXT14_DATAOUT << 14) | (GPIOEXT15_DATAOUT << 15)),

	((GPIOEXT16_DATAOUT << 0) | (GPIOEXT17_DATAOUT << 1) |
	 (GPIOEXT18_DATAOUT << 2) | (GPIOEXT19_DATAOUT << 3) |
	 (GPIOEXT20_DATAOUT << 4) | (GPIOEXT21_DATAOUT << 5) |
	 (GPIOEXT22_DATAOUT << 6) | (GPIOEXT23_DATAOUT << 7) |
	 (GPIOEXT24_DATAOUT << 8) | (GPIOEXT25_DATAOUT << 9) |
	 (GPIOEXT26_DATAOUT << 10) | (GPIOEXT27_DATAOUT << 11) |
	 (GPIOEXT28_DATAOUT << 12) | (GPIOEXT29_DATAOUT << 13) |
	 (GPIOEXT30_DATAOUT << 14) | (GPIOEXT31_DATAOUT << 15)),

	((GPIOEXT32_DATAOUT << 0) | (GPIOEXT33_DATAOUT << 1) |
	 (GPIOEXT34_DATAOUT << 2) | (GPIOEXT35_DATAOUT << 3) |
	 (GPIOEXT36_DATAOUT << 4) | (GPIOEXT37_DATAOUT << 5) |
	 (GPIOEXT38_DATAOUT << 6) | (GPIOEXT39_DATAOUT << 7) |
	 (GPIOEXT40_DATAOUT << 8)),
}; /* end of gpioext_init_dout_data */

u16 gpioext_init_mode_data[10] = {
	((GPIOEXT0_MODE << 0) | (GPIOEXT1_MODE << 3) | (GPIOEXT2_MODE << 6) |
	 (GPIOEXT3_MODE << 9) | (GPIOEXT4_MODE << 12)),
	((GPIOEXT5_MODE << 0) | (GPIOEXT6_MODE << 3) | (GPIOEXT7_MODE << 6) |
	 (GPIOEXT8_MODE << 9) | (GPIOEXT9_MODE << 12)),
	((GPIOEXT10_MODE << 0) | (GPIOEXT11_MODE << 3) | (GPIOEXT12_MODE << 6) |
	 (GPIOEXT13_MODE << 9) | (GPIOEXT14_MODE << 12)),
	((GPIOEXT15_MODE << 0) | (GPIOEXT16_MODE << 3) | (GPIOEXT17_MODE << 6) |
	 (GPIOEXT18_MODE << 9) | (GPIOEXT19_MODE << 12)),
	((GPIOEXT20_MODE << 0) | (GPIOEXT21_MODE << 3) | (GPIOEXT22_MODE << 6) |
	 (GPIOEXT23_MODE << 9) | (GPIOEXT24_MODE << 12)),
	((GPIOEXT25_MODE << 0) | (GPIOEXT26_MODE << 3) | (GPIOEXT27_MODE << 6) |
	 (GPIOEXT28_MODE << 9) | (GPIOEXT29_MODE << 12)),
	((GPIOEXT30_MODE << 0) | (GPIOEXT31_MODE << 3) | (GPIOEXT32_MODE << 6) |
	 (GPIOEXT33_MODE << 9) | (GPIOEXT34_MODE << 12)),
	((GPIOEXT35_MODE << 0) | (GPIOEXT36_MODE << 3) | (GPIOEXT37_MODE << 6) |
	 (GPIOEXT38_MODE << 9) | (GPIOEXT39_MODE << 12)),
	((GPIOEXT40_MODE << 0)),
}; /* end of gpioext_init_mode_data */

void mt_gpio_set_default_ext(void)
{
	unsigned int idx;
	u32 val;

	for (idx = 0; idx < sizeof(gpioext_obj->dir) / sizeof(gpioext_obj->dir[0]); idx++) {
		val = gpioext_init_dir_data[idx];
		gpioext_write(&gpioext_obj->dir[idx].val, val);
	}

	for (idx = 0; idx < sizeof(gpioext_obj->pullen) / sizeof(gpioext_obj->pullen[0]); idx++) {
		val = gpioext_init_pullen_data[idx];
		gpioext_write(&gpioext_obj->pullen[idx].val, val);
	}

	for (idx = 0; idx < sizeof(gpioext_obj->pullsel) / sizeof(gpioext_obj->pullsel[0]); idx++) {
		val = gpioext_init_pullsel_data[idx];
		gpioext_write(&gpioext_obj->pullsel[idx].val, val);
	}

	for (idx = 0; idx < sizeof(gpioext_obj->dinv) / sizeof(gpioext_obj->dinv[0]); idx++) {
		val = gpioext_init_dinv_data[idx];
		gpioext_write(&gpioext_obj->dinv[idx].val, val);
	}

	for (idx = 0; idx < sizeof(gpioext_obj->dout) / sizeof(gpioext_obj->dout[0]); idx++) {
		val = gpioext_init_dout_data[idx];
		gpioext_write(&gpioext_obj->dout[idx].val, val);
	}

	for (idx = 0; idx < sizeof(gpioext_obj->mode) / sizeof(gpioext_obj->mode[0]); idx++) {
		val = gpioext_init_mode_data[idx];
		gpioext_write(&gpioext_obj->mode[idx].val, val);
	}
}
static void mt_gpio_set_dsel(u8 md_power)
{
	/* EXMD control reg */
	if (md_power == 1) {
		/* exmd_ctrl[9:4] = b`000000, [3:0] = b`1010 */
		write16(&gpio_obj->exmd_ctrl[0].rst, 0x3F5);
		write16(&gpio_obj->exmd_ctrl[0].set, 0xA);
	} else if (md_power == 0) {
		/* exmd_ctrl[9:4] = b`001100, [3:0] = b`1010 */
		write16(&gpio_obj->exmd_ctrl[0].rst, 0x335);
		write16(&gpio_obj->exmd_ctrl[0].set, 0xCA);
	}

	/* other R/TDSEL */
	write16(&gpio_obj->msdc2_ctrl5.set, 0xA);    // msdc1_ctrl4 , bit[3:0] = b`1010
	write16(&gpio_obj->msdc2_ctrl5.rst, 0x5);
}

void mt_gpio_set_default(void)
{
	mt_gpio_set_default_chip();
	mt_gpio_set_dsel(1);
}

