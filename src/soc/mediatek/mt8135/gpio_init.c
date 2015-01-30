/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 MediaTek Inc.
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

#include <types.h>
#include <soc/gpio.h>
#include <arch/io.h>
#include <console/console.h>
#include <mainboard/cust_gpio_boot.h>

#include <string.h>
#include <soc/pmic_wrap_init.h>
/*----------------------------------------------------------------------------*/
#define GPIOTAG "[GPIO] "
#define GPIODBG(fmt, arg ...)    printk(BIOS_DEBUG, GPIOTAG "%s: " fmt, __func__, ## arg)
#define GPIOERR(fmt, arg ...)    printk(BIOS_ERR, GPIOTAG "%s: " fmt, __func__, ## arg)
#define GPIOVER(fmt, arg ...)    printk(BIOS_INFO, GPIOTAG "%s: " fmt, __func__, ## arg)

#define DRV_Reg32(addr)             read32((void *)(addr))
#define DRV_WriteReg32(addr, data)  write32(data, (void *)(addr))
#define DRV_Reg16(addr)             read16((void *)(addr))
#define DRV_WriteReg16(addr, data)  write16(data, (void *)(addr))

#define GPIO_WR32(addr, data)   DRV_WriteReg32(addr, data)
#define GPIO_RD32(addr)         DRV_Reg32(addr)
#define GPIO_WR16(addr, data)   DRV_WriteReg16(addr, data)
#define GPIO_RD16(addr)         DRV_Reg16(addr)

#define GPIOEXT_WR(addr, data)   pwrap_write(addr, data)
#define GPIOEXT_RD(addr)         ({ \
					  u32 ext_data;	\
					  (pwrap_read((u32)addr, &ext_data) != 0) ? 32767 : ext_data; })
#define GPIOEXT_BASE        (0xC000)    /* PMIC GPIO base. */

void mt_gpio_set_default_chip(void);
void mt_gpio_set_default_ext(void);
void mt_gpio_set_dsel(u8 nml2_1v8, u8 bsi_1v8, u8 bpi_1v8);
void mt_gpio_checkpoint_save_ext(void);
void mt_gpio_dump_diff(GPIO_REGS *pre, GPIO_REGS *cur);
void mt_gpio_dump_diff_ext(GPIOEXT_REGS *pre, GPIOEXT_REGS *cur);
void mt_gpio_checkpoint_compare_ext(void);

/*----------------------------------------------------------------------------*/
u16 gpio_init_dir_data[] = {
	((GPIO0_DIR << 0) | (GPIO1_DIR << 1) | (GPIO2_DIR << 2) |
	 (GPIO3_DIR << 3) | (GPIO4_DIR << 4) | (GPIO5_DIR << 5) |
	 (GPIO6_DIR << 6) | (GPIO7_DIR << 7) | (GPIO8_DIR << 8) |
	 (GPIO9_DIR << 9) | (GPIO10_DIR << 10) | (GPIO11_DIR << 11) |
	 (GPIO12_DIR << 12) | (GPIO13_DIR << 13) | (GPIO14_DIR << 14) |
	 (GPIO15_DIR << 15)),
	((GPIO16_DIR << 0) | (GPIO17_DIR << 1) | (GPIO18_DIR << 2) |
	 (GPIO19_DIR << 3) | (GPIO20_DIR << 4) | (GPIO21_DIR << 5) |
	 (GPIO22_DIR << 6) | (GPIO23_DIR << 7) | (GPIO24_DIR << 8) |
	 (GPIO25_DIR << 9) | (GPIO26_DIR << 10) | (GPIO27_DIR << 11) |
	 (GPIO28_DIR << 12) | (GPIO29_DIR << 13) | (GPIO30_DIR << 14) |
	 (GPIO31_DIR << 15)),
	((GPIO32_DIR << 0) | (GPIO33_DIR << 1) | (GPIO34_DIR << 2) |
	 (GPIO35_DIR << 3) | (GPIO36_DIR << 4) | (GPIO37_DIR << 5) |
	 (GPIO38_DIR << 6) | (GPIO39_DIR << 7) | (GPIO40_DIR << 8) |
	 (GPIO41_DIR << 9) | (GPIO42_DIR << 10) | (GPIO43_DIR << 11) |
	 (GPIO44_DIR << 12) | (GPIO45_DIR << 13) | (GPIO46_DIR << 14) |
	 (GPIO47_DIR << 15)),
	((GPIO48_DIR << 0) | (GPIO49_DIR << 1) | (GPIO50_DIR << 2) |
	 (GPIO51_DIR << 3) | (GPIO52_DIR << 4) | (GPIO53_DIR << 5) |
	 (GPIO54_DIR << 6) | (GPIO55_DIR << 7) | (GPIO56_DIR << 8) |
	 (GPIO57_DIR << 9) | (GPIO58_DIR << 10) | (GPIO59_DIR << 11) |
	 (GPIO60_DIR << 12) | (GPIO61_DIR << 13) | (GPIO62_DIR << 14) |
	 (GPIO63_DIR << 15)),
	((GPIO64_DIR << 0) | (GPIO65_DIR << 1) | (GPIO66_DIR << 2) |
	 (GPIO67_DIR << 3) | (GPIO68_DIR << 4) | (GPIO69_DIR << 5) |
	 (GPIO70_DIR << 6) | (GPIO71_DIR << 7) | (GPIO72_DIR << 8) |
	 (GPIO73_DIR << 9) | (GPIO74_DIR << 10) | (GPIO75_DIR << 11) |
	 (GPIO76_DIR << 12) | (GPIO77_DIR << 13) | (GPIO78_DIR << 14) |
	 (GPIO79_DIR << 15)),
	((GPIO80_DIR << 0) | (GPIO81_DIR << 1) | (GPIO82_DIR << 2) |
	 (GPIO83_DIR << 3) | (GPIO84_DIR << 4) | (GPIO85_DIR << 5) |
	 (GPIO86_DIR << 6) | (GPIO87_DIR << 7) | (GPIO88_DIR << 8) |
	 (GPIO89_DIR << 9) | (GPIO90_DIR << 10) | (GPIO91_DIR << 11) |
	 (GPIO92_DIR << 12) | (GPIO93_DIR << 13) | (GPIO94_DIR << 14) |
	 (GPIO95_DIR << 15)),
	((GPIO96_DIR << 0) | (GPIO97_DIR << 1) | (GPIO98_DIR << 2) |
	 (GPIO99_DIR << 3) | (GPIO100_DIR << 4) | (GPIO101_DIR << 5) |
	 (GPIO102_DIR << 6) | (GPIO103_DIR << 7) | (GPIO104_DIR << 8) |
	 (GPIO105_DIR << 9) | (GPIO106_DIR << 10) | (GPIO107_DIR << 11) |
	 (GPIO108_DIR << 12) | (GPIO109_DIR << 13) | (GPIO110_DIR << 14) |
	 (GPIO111_DIR << 15)),
	((GPIO112_DIR << 0) | (GPIO113_DIR << 1) | (GPIO114_DIR << 2) |
	 (GPIO115_DIR << 3) | (GPIO116_DIR << 4) | (GPIO117_DIR << 5) |
	 (GPIO118_DIR << 6) | (GPIO119_DIR << 7) | (GPIO120_DIR << 8) |
	 (GPIO121_DIR << 9) | (GPIO122_DIR << 10) | (GPIO123_DIR << 11) |
	 (GPIO124_DIR << 12) | (GPIO125_DIR << 13) | (GPIO126_DIR << 14) |
	 (GPIO127_DIR << 15)),
	((GPIO128_DIR << 0) | (GPIO129_DIR << 1) | (GPIO130_DIR << 2) |
	 (GPIO131_DIR << 3) | (GPIO132_DIR << 4) | (GPIO133_DIR << 5) |
	 (GPIO134_DIR << 6) | (GPIO135_DIR << 7) | (GPIO136_DIR << 8) |
	 (GPIO137_DIR << 9) | (GPIO138_DIR << 10) | (GPIO139_DIR << 11) |
	 (GPIO140_DIR << 12) | (GPIO141_DIR << 13) | (GPIO142_DIR << 14) |
	 (GPIO143_DIR << 15)),
	((GPIO144_DIR << 0) | (GPIO145_DIR << 1) | (GPIO146_DIR << 2) |
	 (GPIO147_DIR << 3) | (GPIO148_DIR << 4) | (GPIO149_DIR << 5) |
	 (GPIO150_DIR << 6) | (GPIO151_DIR << 7) | (GPIO152_DIR << 8) |
	 (GPIO153_DIR << 9) | (GPIO154_DIR << 10) | (GPIO155_DIR << 11) |
	 (GPIO156_DIR << 12) | (GPIO157_DIR << 13) | (GPIO158_DIR << 14) |
	 (GPIO159_DIR << 15)),
	((GPIO160_DIR << 0) | (GPIO161_DIR << 1) | (GPIO162_DIR << 2) |
	 (GPIO163_DIR << 3) | (GPIO164_DIR << 4) | (GPIO165_DIR << 5) |
	 (GPIO166_DIR << 6) | (GPIO167_DIR << 7) | (GPIO168_DIR << 8) |
	 (GPIO169_DIR << 9) | (GPIO170_DIR << 10) | (GPIO171_DIR << 11) |
	 (GPIO172_DIR << 12) | (GPIO173_DIR << 13) | (GPIO174_DIR << 14) |
	 (GPIO175_DIR << 15)),
	((GPIO176_DIR << 0) | (GPIO177_DIR << 1) | (GPIO178_DIR << 2) |
	 (GPIO179_DIR << 3) | (GPIO180_DIR << 4) | (GPIO181_DIR << 5) |
	 (GPIO182_DIR << 6) | (GPIO183_DIR << 7) | (GPIO184_DIR << 8) |
	 (GPIO185_DIR << 9) | (GPIO186_DIR << 10) | (GPIO187_DIR << 11) |
	 (GPIO188_DIR << 12) | (GPIO189_DIR << 13) | (GPIO190_DIR << 14) |
	 (GPIO191_DIR << 15)),
	((GPIO192_DIR << 0) | (GPIO193_DIR << 1) | (GPIO194_DIR << 2) |
	 (GPIO195_DIR << 3) | (GPIO196_DIR << 4) | (GPIO197_DIR << 5) |
	 (GPIO198_DIR << 6) | (GPIO199_DIR << 7) | (GPIO200_DIR << 8) |
	 (GPIO201_DIR << 9) | (GPIO202_DIR << 10)),
};                              /*end of gpio_init_dir_data */

/*----------------------------------------------------------------------------*/
u16 gpio_init_pullen_data[] = {
	((GPIO0_PULLEN << 0) | (GPIO1_PULLEN << 1) | (GPIO2_PULLEN << 2) |
	 (GPIO3_PULLEN << 3) | (GPIO4_PULLEN << 4) | (GPIO5_PULLEN << 5) |
	 (GPIO6_PULLEN << 6) | (GPIO7_PULLEN << 7) | (GPIO8_PULLEN << 8) |
	 (GPIO9_PULLEN << 9) | (GPIO10_PULLEN << 10) | (GPIO11_PULLEN << 11) |
	 (GPIO12_PULLEN << 12) | (GPIO13_PULLEN << 13) | (GPIO14_PULLEN << 14) |
	 (GPIO15_PULLEN << 15)),

	((GPIO16_PULLEN << 0) | (GPIO17_PULLEN << 1) | (GPIO18_PULLEN << 2) |
	 (GPIO19_PULLEN << 3) | (GPIO20_PULLEN << 4) | (GPIO21_PULLEN << 5) |
	 (GPIO22_PULLEN << 6) | (GPIO23_PULLEN << 7) | (GPIO24_PULLEN << 8) |
	 (GPIO25_PULLEN << 9) | (GPIO26_PULLEN << 10) | (GPIO27_PULLEN << 11) |
	 (GPIO28_PULLEN << 12) | (GPIO29_PULLEN << 13) | (GPIO30_PULLEN << 14) |
	 (GPIO31_PULLEN << 15)),

	((GPIO32_PULLEN << 0) | (GPIO33_PULLEN << 1) | (GPIO34_PULLEN << 2) |
	 (GPIO35_PULLEN << 3) | (GPIO36_PULLEN << 4) | (GPIO37_PULLEN << 5) |
	 (GPIO38_PULLEN << 6) | (GPIO39_PULLEN << 7) | (GPIO40_PULLEN << 8) |
	 (GPIO41_PULLEN << 9) | (GPIO42_PULLEN << 10) | (GPIO43_PULLEN << 11) |
	 (GPIO44_PULLEN << 12) | (GPIO45_PULLEN << 13) | (GPIO46_PULLEN << 14) |
	 (GPIO47_PULLEN << 15)),

	((GPIO48_PULLEN << 0) | (GPIO49_PULLEN << 1) | (GPIO50_PULLEN << 2) |
	 (GPIO51_PULLEN << 3) | (GPIO52_PULLEN << 4) | (GPIO53_PULLEN << 5) |
	 (GPIO54_PULLEN << 6) | (GPIO55_PULLEN << 7) | (GPIO56_PULLEN << 8) |
	 (GPIO57_PULLEN << 9) | (GPIO58_PULLEN << 10) | (GPIO59_PULLEN << 11) |
	 (GPIO60_PULLEN << 12) | (GPIO61_PULLEN << 13) | (GPIO62_PULLEN << 14) |
	 (GPIO63_PULLEN << 15)),

	((GPIO64_PULLEN << 0) | (GPIO65_PULLEN << 1) | (GPIO66_PULLEN << 2) |
	 (GPIO67_PULLEN << 3) | (GPIO68_PULLEN << 4) | (GPIO69_PULLEN << 5) |
	 (GPIO70_PULLEN << 6) | (GPIO71_PULLEN << 7) | (GPIO72_PULLEN << 8) |
	 (GPIO73_PULLEN << 9) | (GPIO74_PULLEN << 10) | (GPIO75_PULLEN << 11) |
	 (GPIO76_PULLEN << 12) | (GPIO77_PULLEN << 13) | (GPIO78_PULLEN << 14) |
	 (GPIO79_PULLEN << 15)),

	((GPIO80_PULLEN << 0) | (GPIO81_PULLEN << 1) | (GPIO82_PULLEN << 2) |
	 (GPIO83_PULLEN << 3) | (GPIO84_PULLEN << 4) | (GPIO85_PULLEN << 5) |
	 (GPIO86_PULLEN << 6) | (GPIO87_PULLEN << 7) | (GPIO88_PULLEN << 8) |
	 (GPIO89_PULLEN << 9) | (GPIO90_PULLEN << 10) | (GPIO91_PULLEN << 11) |
	 (GPIO92_PULLEN << 12) | (GPIO93_PULLEN << 13) | (GPIO94_PULLEN << 14) |
	 (GPIO95_PULLEN << 15)),

	((GPIO96_PULLEN << 0) | (GPIO97_PULLEN << 1) | (GPIO98_PULLEN << 2) |
	 (GPIO99_PULLEN << 3) | (GPIO100_PULLEN << 4) | (GPIO101_PULLEN << 5) |
	 (GPIO102_PULLEN << 6) | (GPIO103_PULLEN << 7) | (GPIO104_PULLEN << 8) |
	 (GPIO105_PULLEN << 9) | (GPIO106_PULLEN << 10) | (GPIO107_PULLEN << 11) |
	 (GPIO108_PULLEN << 12) | (GPIO109_PULLEN << 13) | (GPIO110_PULLEN << 14) |
	 (GPIO111_PULLEN << 15)),

	((GPIO112_PULLEN << 0) | (GPIO113_PULLEN << 1) | (GPIO114_PULLEN << 2) |
	 (GPIO115_PULLEN << 3) | (GPIO116_PULLEN << 4) | (GPIO117_PULLEN << 5) |
	 (GPIO118_PULLEN << 6) | (GPIO119_PULLEN << 7) | (GPIO120_PULLEN << 8) |
	 (GPIO121_PULLEN << 9) | (GPIO122_PULLEN << 10) | (GPIO123_PULLEN << 11) |
	 (GPIO124_PULLEN << 12) | (GPIO125_PULLEN << 13) | (GPIO126_PULLEN << 14) |
	 (GPIO127_PULLEN << 15)),

	((GPIO128_PULLEN << 0) | (GPIO129_PULLEN << 1) | (GPIO130_PULLEN << 2) |
	 (GPIO131_PULLEN << 3) | (GPIO132_PULLEN << 4) | (GPIO133_PULLEN << 5) |
	 (GPIO134_PULLEN << 6) | (GPIO135_PULLEN << 7) | (GPIO136_PULLEN << 8) |
	 (GPIO137_PULLEN << 9) | (GPIO138_PULLEN << 10) | (GPIO139_PULLEN << 11) |
	 (GPIO140_PULLEN << 12) | (GPIO141_PULLEN << 13) | (GPIO142_PULLEN << 14) |
	 (GPIO143_PULLEN << 15)),

	((GPIO144_PULLEN << 0) | (GPIO145_PULLEN << 1) | (GPIO146_PULLEN << 2) |
	 (GPIO147_PULLEN << 3) | (GPIO148_PULLEN << 4) | (GPIO149_PULLEN << 5) |
	 (GPIO150_PULLEN << 6) | (GPIO151_PULLEN << 7) | (GPIO152_PULLEN << 8) |
	 (GPIO153_PULLEN << 9) | (GPIO154_PULLEN << 10) | (GPIO155_PULLEN << 11) |
	 (GPIO156_PULLEN << 12) | (GPIO157_PULLEN << 13) | (GPIO158_PULLEN << 14) |
	 (GPIO159_PULLEN << 15)),

	((GPIO160_PULLEN << 0) | (GPIO161_PULLEN << 1) | (GPIO162_PULLEN << 2) |
	 (GPIO163_PULLEN << 3) | (GPIO164_PULLEN << 4) | (GPIO165_PULLEN << 5) |
	 (GPIO166_PULLEN << 6) | (GPIO167_PULLEN << 7) | (GPIO168_PULLEN << 8) |
	 (GPIO169_PULLEN << 9) | (GPIO170_PULLEN << 10) | (GPIO171_PULLEN << 11) |
	 (GPIO172_PULLEN << 12) | (GPIO173_PULLEN << 13) | (GPIO174_PULLEN << 14) |
	 (GPIO175_PULLEN << 15)),

	((GPIO176_PULLEN << 0) | (GPIO177_PULLEN << 1) | (GPIO178_PULLEN << 2) |
	 (GPIO179_PULLEN << 3) | (GPIO180_PULLEN << 4) | (GPIO181_PULLEN << 5) |
	 (GPIO182_PULLEN << 6) | (GPIO183_PULLEN << 7) | (GPIO184_PULLEN << 8) |
	 (GPIO185_PULLEN << 9) | (GPIO186_PULLEN << 10) | (GPIO187_PULLEN << 11) |
	 (GPIO188_PULLEN << 12) | (GPIO189_PULLEN << 13) | (GPIO190_PULLEN << 14) |
	 (GPIO191_PULLEN << 15)),

	((GPIO192_PULLEN << 0) | (GPIO193_PULLEN << 1) | (GPIO194_PULLEN << 2) |
	 (GPIO195_PULLEN << 3) | (GPIO196_PULLEN << 4) | (GPIO197_PULLEN << 5) |
	 (GPIO198_PULLEN << 6) | (GPIO199_PULLEN << 7) | (GPIO200_PULLEN << 8) |
	 (GPIO201_PULLEN << 9) | (GPIO202_PULLEN << 10)),
};                              /*end of gpio_init_pullen_data */

/*----------------------------------------------------------------------------*/
u16 gpio_init_pullsel_data[] = {
	((GPIO0_PULL << 0) | (GPIO1_PULL << 1) | (GPIO2_PULL << 2) |
	 (GPIO3_PULL << 3) | (GPIO4_PULL << 4) | (GPIO5_PULL << 5) |
	 (GPIO6_PULL << 6) | (GPIO7_PULL << 7) | (GPIO8_PULL << 8) |
	 (GPIO9_PULL << 9) | (GPIO10_PULL << 10) | (GPIO11_PULL << 11) |
	 (GPIO12_PULL << 12) | (GPIO13_PULL << 13) | (GPIO14_PULL << 14) |
	 (GPIO15_PULL << 15)),

	((GPIO16_PULL << 0) | (GPIO17_PULL << 1) | (GPIO18_PULL << 2) |
	 (GPIO19_PULL << 3) | (GPIO20_PULL << 4) | (GPIO21_PULL << 5) |
	 (GPIO22_PULL << 6) | (GPIO23_PULL << 7) | (GPIO24_PULL << 8) |
	 (GPIO25_PULL << 9) | (GPIO26_PULL << 10) | (GPIO27_PULL << 11) |
	 (GPIO28_PULL << 12) | (GPIO29_PULL << 13) | (GPIO30_PULL << 14) |
	 (GPIO31_PULL << 15)),

	((GPIO32_PULL << 0) | (GPIO33_PULL << 1) | (GPIO34_PULL << 2) |
	 (GPIO35_PULL << 3) | (GPIO36_PULL << 4) | (GPIO37_PULL << 5) |
	 (GPIO38_PULL << 6) | (GPIO39_PULL << 7) | (GPIO40_PULL << 8) |
	 (GPIO41_PULL << 9) | (GPIO42_PULL << 10) | (GPIO43_PULL << 11) |
	 (GPIO44_PULL << 12) | (GPIO45_PULL << 13) | (GPIO46_PULL << 14) |
	 (GPIO47_PULL << 15)),

	((GPIO48_PULL << 0) | (GPIO49_PULL << 1) | (GPIO50_PULL << 2) |
	 (GPIO51_PULL << 3) | (GPIO52_PULL << 4) | (GPIO53_PULL << 5) |
	 (GPIO54_PULL << 6) | (GPIO55_PULL << 7) | (GPIO56_PULL << 8) |
	 (GPIO57_PULL << 9) | (GPIO58_PULL << 10) | (GPIO59_PULL << 11) |
	 (GPIO60_PULL << 12) | (GPIO61_PULL << 13) | (GPIO62_PULL << 14) |
	 (GPIO63_PULL << 15)),

	((GPIO64_PULL << 0) | (GPIO65_PULL << 1) | (GPIO66_PULL << 2) |
	 (GPIO67_PULL << 3) | (GPIO68_PULL << 4) | (GPIO69_PULL << 5) |
	 (GPIO70_PULL << 6) | (GPIO71_PULL << 7) | (GPIO72_PULL << 8) |
	 (GPIO73_PULL << 9) | (GPIO74_PULL << 10) | (GPIO75_PULL << 11) |
	 (GPIO76_PULL << 12) | (GPIO77_PULL << 13) | (GPIO78_PULL << 14) |
	 (GPIO79_PULL << 15)),

	((GPIO80_PULL << 0) | (GPIO81_PULL << 1) | (GPIO82_PULL << 2) |
	 (GPIO83_PULL << 3) | (GPIO84_PULL << 4) | (GPIO85_PULL << 5) |
	 (GPIO86_PULL << 6) | (GPIO87_PULL << 7) | (GPIO88_PULL << 8) |
	 (GPIO89_PULL << 9) | (GPIO90_PULL << 10) | (GPIO91_PULL << 11) |
	 (GPIO92_PULL << 12) | (GPIO93_PULL << 13) | (GPIO94_PULL << 14) |
	 (GPIO95_PULL << 15)),

	((GPIO96_PULL << 0) | (GPIO97_PULL << 1) | (GPIO98_PULL << 2) |
	 (GPIO99_PULL << 3) | (GPIO100_PULL << 4) | (GPIO101_PULL << 5) |
	 (GPIO102_PULL << 6) | (GPIO103_PULL << 7) | (GPIO104_PULL << 8) |
	 (GPIO105_PULL << 9) | (GPIO106_PULL << 10) | (GPIO107_PULL << 11) |
	 (GPIO108_PULL << 12) | (GPIO109_PULL << 13) | (GPIO110_PULL << 14) |
	 (GPIO111_PULL << 15)),

	((GPIO112_PULL << 0) | (GPIO113_PULL << 1) | (GPIO114_PULL << 2) |
	 (GPIO115_PULL << 3) | (GPIO116_PULL << 4) | (GPIO117_PULL << 5) |
	 (GPIO118_PULL << 6) | (GPIO119_PULL << 7) | (GPIO120_PULL << 8) |
	 (GPIO121_PULL << 9) | (GPIO122_PULL << 10) | (GPIO123_PULL << 11) |
	 (GPIO124_PULL << 12) | (GPIO125_PULL << 13) | (GPIO126_PULL << 14) |
	 (GPIO127_PULL << 15)),

	((GPIO128_PULL << 0) | (GPIO129_PULL << 1) | (GPIO130_PULL << 2) |
	 (GPIO131_PULL << 3) | (GPIO132_PULL << 4) | (GPIO133_PULL << 5) |
	 (GPIO134_PULL << 6) | (GPIO135_PULL << 7) | (GPIO136_PULL << 8) |
	 (GPIO137_PULL << 9) | (GPIO138_PULL << 10) | (GPIO139_PULL << 11) |
	 (GPIO140_PULL << 12) | (GPIO141_PULL << 13) | (GPIO142_PULL << 14) |
	 (GPIO143_PULL << 15)),

	((GPIO144_PULL << 0) | (GPIO145_PULL << 1) | (GPIO146_PULL << 2) |
	 (GPIO147_PULL << 3) | (GPIO148_PULL << 4) | (GPIO149_PULL << 5) |
	 (GPIO150_PULL << 6) | (GPIO151_PULL << 7) | (GPIO152_PULL << 8) |
	 (GPIO153_PULL << 9) | (GPIO154_PULL << 10) | (GPIO155_PULL << 11) |
	 (GPIO156_PULL << 12) | (GPIO157_PULL << 13) | (GPIO158_PULL << 14) |
	 (GPIO159_PULL << 15)),

	((GPIO160_PULL << 0) | (GPIO161_PULL << 1) | (GPIO162_PULL << 2) |
	 (GPIO163_PULL << 3) | (GPIO164_PULL << 4) | (GPIO165_PULL << 5) |
	 (GPIO166_PULL << 6) | (GPIO167_PULL << 7) | (GPIO168_PULL << 8) |
	 (GPIO169_PULL << 9) | (GPIO170_PULL << 10) | (GPIO171_PULL << 11) |
	 (GPIO172_PULL << 12) | (GPIO173_PULL << 13) | (GPIO174_PULL << 14) |
	 (GPIO175_PULL << 15)),

	((GPIO176_PULL << 0) | (GPIO177_PULL << 1) | (GPIO178_PULL << 2) |
	 (GPIO179_PULL << 3) | (GPIO180_PULL << 4) | (GPIO181_PULL << 5) |
	 (GPIO182_PULL << 6) | (GPIO183_PULL << 7) | (GPIO184_PULL << 8) |
	 (GPIO185_PULL << 9) | (GPIO186_PULL << 10) | (GPIO187_PULL << 11) |
	 (GPIO188_PULL << 12) | (GPIO189_PULL << 13) | (GPIO190_PULL << 14) |
	 (GPIO191_PULL << 15)),

	((GPIO192_PULL << 0) | (GPIO193_PULL << 1) | (GPIO194_PULL << 2) |
	 (GPIO195_PULL << 3) | (GPIO196_PULL << 4) | (GPIO197_PULL << 5) |
	 (GPIO198_PULL << 6) | (GPIO199_PULL << 7) | (GPIO200_PULL << 8) |
	 (GPIO201_PULL << 9) | (GPIO202_PULL << 10)),
};                              /*end of gpio_init_pullsel_data */

/*----------------------------------------------------------------------------*/
u16 gpio_init_dinv_data[] = {
	((GPIO0_DATAINV << 0) | (GPIO1_DATAINV << 1) | (GPIO2_DATAINV << 2) |
	 (GPIO3_DATAINV << 3) | (GPIO4_DATAINV << 4) | (GPIO5_DATAINV << 5) |
	 (GPIO6_DATAINV << 6) | (GPIO7_DATAINV << 7) | (GPIO8_DATAINV << 8) |
	 (GPIO9_DATAINV << 9) | (GPIO10_DATAINV << 10) | (GPIO11_DATAINV << 11) |
	 (GPIO12_DATAINV << 12) | (GPIO13_DATAINV << 13) | (GPIO14_DATAINV << 14) |
	 (GPIO15_DATAINV << 15)),

	((GPIO16_DATAINV << 0) | (GPIO17_DATAINV << 1) | (GPIO18_DATAINV << 2) |
	 (GPIO19_DATAINV << 3) | (GPIO20_DATAINV << 4) | (GPIO21_DATAINV << 5) |
	 (GPIO22_DATAINV << 6) | (GPIO23_DATAINV << 7) | (GPIO24_DATAINV << 8) |
	 (GPIO25_DATAINV << 9) | (GPIO26_DATAINV << 10) | (GPIO27_DATAINV << 11) |
	 (GPIO28_DATAINV << 12) | (GPIO29_DATAINV << 13) | (GPIO30_DATAINV << 14) |
	 (GPIO31_DATAINV << 15)),

	((GPIO32_DATAINV << 0) | (GPIO33_DATAINV << 1) | (GPIO34_DATAINV << 2) |
	 (GPIO35_DATAINV << 3) | (GPIO36_DATAINV << 4) | (GPIO37_DATAINV << 5) |
	 (GPIO38_DATAINV << 6) | (GPIO39_DATAINV << 7) | (GPIO40_DATAINV << 8) |
	 (GPIO41_DATAINV << 9) | (GPIO42_DATAINV << 10) | (GPIO43_DATAINV << 11) |
	 (GPIO44_DATAINV << 12) | (GPIO45_DATAINV << 13) | (GPIO46_DATAINV << 14) |
	 (GPIO47_DATAINV << 15)),

	((GPIO48_DATAINV << 0) | (GPIO49_DATAINV << 1) | (GPIO50_DATAINV << 2) |
	 (GPIO51_DATAINV << 3) | (GPIO52_DATAINV << 4) | (GPIO53_DATAINV << 5) |
	 (GPIO54_DATAINV << 6) | (GPIO55_DATAINV << 7) | (GPIO56_DATAINV << 8) |
	 (GPIO57_DATAINV << 9) | (GPIO58_DATAINV << 10) | (GPIO59_DATAINV << 11) |
	 (GPIO60_DATAINV << 12) | (GPIO61_DATAINV << 13) | (GPIO62_DATAINV << 14) |
	 (GPIO63_DATAINV << 15)),

	((GPIO64_DATAINV << 0) | (GPIO65_DATAINV << 1) | (GPIO66_DATAINV << 2) |
	 (GPIO67_DATAINV << 3) | (GPIO68_DATAINV << 4) | (GPIO69_DATAINV << 5) |
	 (GPIO70_DATAINV << 6) | (GPIO71_DATAINV << 7) | (GPIO72_DATAINV << 8) |
	 (GPIO73_DATAINV << 9) | (GPIO74_DATAINV << 10) | (GPIO75_DATAINV << 11) |
	 (GPIO76_DATAINV << 12) | (GPIO77_DATAINV << 13) | (GPIO78_DATAINV << 14) |
	 (GPIO79_DATAINV << 15)),

	((GPIO80_DATAINV << 0) | (GPIO81_DATAINV << 1) | (GPIO82_DATAINV << 2) |
	 (GPIO83_DATAINV << 3) | (GPIO84_DATAINV << 4) | (GPIO85_DATAINV << 5) |
	 (GPIO86_DATAINV << 6) | (GPIO87_DATAINV << 7) | (GPIO88_DATAINV << 8) |
	 (GPIO89_DATAINV << 9) | (GPIO90_DATAINV << 10) | (GPIO91_DATAINV << 11) |
	 (GPIO92_DATAINV << 12) | (GPIO93_DATAINV << 13) | (GPIO94_DATAINV << 14) |
	 (GPIO95_DATAINV << 15)),

	((GPIO96_DATAINV << 0) | (GPIO97_DATAINV << 1) | (GPIO98_DATAINV << 2) |
	 (GPIO99_DATAINV << 3) | (GPIO100_DATAINV << 4) | (GPIO101_DATAINV << 5) |
	 (GPIO102_DATAINV << 6) | (GPIO103_DATAINV << 7) | (GPIO104_DATAINV << 8) |
	 (GPIO105_DATAINV << 9) | (GPIO106_DATAINV << 10) | (GPIO107_DATAINV << 11) |
	 (GPIO108_DATAINV << 12) | (GPIO109_DATAINV << 13) | (GPIO110_DATAINV << 14) |
	 (GPIO111_DATAINV << 15)),

	((GPIO112_DATAINV << 0) | (GPIO113_DATAINV << 1) | (GPIO114_DATAINV << 2) |
	 (GPIO115_DATAINV << 3) | (GPIO116_DATAINV << 4) | (GPIO117_DATAINV << 5) |
	 (GPIO118_DATAINV << 6) | (GPIO119_DATAINV << 7) | (GPIO120_DATAINV << 8) |
	 (GPIO121_DATAINV << 9) | (GPIO122_DATAINV << 10) | (GPIO123_DATAINV << 11) |
	 (GPIO124_DATAINV << 12) | (GPIO125_DATAINV << 13) | (GPIO126_DATAINV << 14) |
	 (GPIO127_DATAINV << 15)),

	((GPIO128_DATAINV << 0) | (GPIO129_DATAINV << 1) | (GPIO130_DATAINV << 2) |
	 (GPIO131_DATAINV << 3) | (GPIO132_DATAINV << 4) | (GPIO133_DATAINV << 5) |
	 (GPIO134_DATAINV << 6) | (GPIO135_DATAINV << 7) | (GPIO136_DATAINV << 8) |
	 (GPIO137_DATAINV << 9) | (GPIO138_DATAINV << 10) | (GPIO139_DATAINV << 11) |
	 (GPIO140_DATAINV << 12) | (GPIO141_DATAINV << 13) | (GPIO142_DATAINV << 14) |
	 (GPIO143_DATAINV << 15)),

	((GPIO144_DATAINV << 0) | (GPIO145_DATAINV << 1) | (GPIO146_DATAINV << 2) |
	 (GPIO147_DATAINV << 3) | (GPIO148_DATAINV << 4) | (GPIO149_DATAINV << 5) |
	 (GPIO150_DATAINV << 6) | (GPIO151_DATAINV << 7) | (GPIO152_DATAINV << 8) |
	 (GPIO153_DATAINV << 9) | (GPIO154_DATAINV << 10) | (GPIO155_DATAINV << 11) |
	 (GPIO156_DATAINV << 12) | (GPIO157_DATAINV << 13) | (GPIO158_DATAINV << 14) |
	 (GPIO159_DATAINV << 15)),

	((GPIO160_DATAINV << 0) | (GPIO161_DATAINV << 1) | (GPIO162_DATAINV << 2) |
	 (GPIO163_DATAINV << 3) | (GPIO164_DATAINV << 4) | (GPIO165_DATAINV << 5) |
	 (GPIO166_DATAINV << 6) | (GPIO167_DATAINV << 7) | (GPIO168_DATAINV << 8) |
	 (GPIO169_DATAINV << 9) | (GPIO170_DATAINV << 10) | (GPIO171_DATAINV << 11) |
	 (GPIO172_DATAINV << 12) | (GPIO173_DATAINV << 13) | (GPIO174_DATAINV << 14) |
	 (GPIO175_DATAINV << 15)),

	((GPIO176_DATAINV << 0) | (GPIO177_DATAINV << 1) | (GPIO178_DATAINV << 2) |
	 (GPIO179_DATAINV << 3) | (GPIO180_DATAINV << 4) | (GPIO181_DATAINV << 5) |
	 (GPIO182_DATAINV << 6) | (GPIO183_DATAINV << 7) | (GPIO184_DATAINV << 8) |
	 (GPIO185_DATAINV << 9) | (GPIO186_DATAINV << 10) | (GPIO187_DATAINV << 11) |
	 (GPIO188_DATAINV << 12) | (GPIO189_DATAINV << 13) | (GPIO190_DATAINV << 14) |
	 (GPIO191_DATAINV << 15)),

	((GPIO192_DATAINV << 0) | (GPIO193_DATAINV << 1) | (GPIO194_DATAINV << 2) |
	 (GPIO195_DATAINV << 3) | (GPIO196_DATAINV << 4) | (GPIO197_DATAINV << 5) |
	 (GPIO198_DATAINV << 6) | (GPIO199_DATAINV << 7) | (GPIO200_DATAINV << 8) |
	 (GPIO201_DATAINV << 9) | (GPIO202_DATAINV << 10)),
};                              /*end of gpio_init_dinv_data */

/*----------------------------------------------------------------------------*/
u16 gpio_init_dout_data[] = {
	((GPIO0_DATAOUT << 0) | (GPIO1_DATAOUT << 1) | (GPIO2_DATAOUT << 2) |
	 (GPIO3_DATAOUT << 3) | (GPIO4_DATAOUT << 4) | (GPIO5_DATAOUT << 5) |
	 (GPIO6_DATAOUT << 6) | (GPIO7_DATAOUT << 7) | (GPIO8_DATAOUT << 8) |
	 (GPIO9_DATAOUT << 9) | (GPIO10_DATAOUT << 10) | (GPIO11_DATAOUT << 11) |
	 (GPIO12_DATAOUT << 12) | (GPIO13_DATAOUT << 13) | (GPIO14_DATAOUT << 14) |
	 (GPIO15_DATAOUT << 15)),

	((GPIO16_DATAOUT << 0) | (GPIO17_DATAOUT << 1) | (GPIO18_DATAOUT << 2) |
	 (GPIO19_DATAOUT << 3) | (GPIO20_DATAOUT << 4) | (GPIO21_DATAOUT << 5) |
	 (GPIO22_DATAOUT << 6) | (GPIO23_DATAOUT << 7) | (GPIO24_DATAOUT << 8) |
	 (GPIO25_DATAOUT << 9) | (GPIO26_DATAOUT << 10) | (GPIO27_DATAOUT << 11) |
	 (GPIO28_DATAOUT << 12) | (GPIO29_DATAOUT << 13) | (GPIO30_DATAOUT << 14) |
	 (GPIO31_DATAOUT << 15)),

	((GPIO32_DATAOUT << 0) | (GPIO33_DATAOUT << 1) | (GPIO34_DATAOUT << 2) |
	 (GPIO35_DATAOUT << 3) | (GPIO36_DATAOUT << 4) | (GPIO37_DATAOUT << 5) |
	 (GPIO38_DATAOUT << 6) | (GPIO39_DATAOUT << 7) | (GPIO40_DATAOUT << 8) |
	 (GPIO41_DATAOUT << 9) | (GPIO42_DATAOUT << 10) | (GPIO43_DATAOUT << 11) |
	 (GPIO44_DATAOUT << 12) | (GPIO45_DATAOUT << 13) | (GPIO46_DATAOUT << 14) |
	 (GPIO47_DATAOUT << 15)),

	((GPIO48_DATAOUT << 0) | (GPIO49_DATAOUT << 1) | (GPIO50_DATAOUT << 2) |
	 (GPIO51_DATAOUT << 3) | (GPIO52_DATAOUT << 4) | (GPIO53_DATAOUT << 5) |
	 (GPIO54_DATAOUT << 6) | (GPIO55_DATAOUT << 7) | (GPIO56_DATAOUT << 8) |
	 (GPIO57_DATAOUT << 9) | (GPIO58_DATAOUT << 10) | (GPIO59_DATAOUT << 11) |
	 (GPIO60_DATAOUT << 12) | (GPIO61_DATAOUT << 13) | (GPIO62_DATAOUT << 14) |
	 (GPIO63_DATAOUT << 15)),

	((GPIO64_DATAOUT << 0) | (GPIO65_DATAOUT << 1) | (GPIO66_DATAOUT << 2) |
	 (GPIO67_DATAOUT << 3) | (GPIO68_DATAOUT << 4) | (GPIO69_DATAOUT << 5) |
	 (GPIO70_DATAOUT << 6) | (GPIO71_DATAOUT << 7) | (GPIO72_DATAOUT << 8) |
	 (GPIO73_DATAOUT << 9) | (GPIO74_DATAOUT << 10) | (GPIO75_DATAOUT << 11) |
	 (GPIO76_DATAOUT << 12) | (GPIO77_DATAOUT << 13) | (GPIO78_DATAOUT << 14) |
	 (GPIO79_DATAOUT << 15)),

	((GPIO80_DATAOUT << 0) | (GPIO81_DATAOUT << 1) | (GPIO82_DATAOUT << 2) |
	 (GPIO83_DATAOUT << 3) | (GPIO84_DATAOUT << 4) | (GPIO85_DATAOUT << 5) |
	 (GPIO86_DATAOUT << 6) | (GPIO87_DATAOUT << 7) | (GPIO88_DATAOUT << 8) |
	 (GPIO89_DATAOUT << 9) | (GPIO90_DATAOUT << 10) | (GPIO91_DATAOUT << 11) |
	 (GPIO92_DATAOUT << 12) | (GPIO93_DATAOUT << 13) | (GPIO94_DATAOUT << 14) |
	 (GPIO95_DATAOUT << 15)),

	((GPIO96_DATAOUT << 0) | (GPIO97_DATAOUT << 1) | (GPIO98_DATAOUT << 2) |
	 (GPIO99_DATAOUT << 3) | (GPIO100_DATAOUT << 4) | (GPIO101_DATAOUT << 5) |
	 (GPIO102_DATAOUT << 6) | (GPIO103_DATAOUT << 7) | (GPIO104_DATAOUT << 8) |
	 (GPIO105_DATAOUT << 9) | (GPIO106_DATAOUT << 10) | (GPIO107_DATAOUT << 11) |
	 (GPIO108_DATAOUT << 12) | (GPIO109_DATAOUT << 13) | (GPIO110_DATAOUT << 14) |
	 (GPIO111_DATAOUT << 15)),

	((GPIO112_DATAOUT << 0) | (GPIO113_DATAOUT << 1) | (GPIO114_DATAOUT << 2) |
	 (GPIO115_DATAOUT << 3) | (GPIO116_DATAOUT << 4) | (GPIO117_DATAOUT << 5) |
	 (GPIO118_DATAOUT << 6) | (GPIO119_DATAOUT << 7) | (GPIO120_DATAOUT << 8) |
	 (GPIO121_DATAOUT << 9) | (GPIO122_DATAOUT << 10) | (GPIO123_DATAOUT << 11) |
	 (GPIO124_DATAOUT << 12) | (GPIO125_DATAOUT << 13) | (GPIO126_DATAOUT << 14) |
	 (GPIO127_DATAOUT << 15)),

	((GPIO128_DATAOUT << 0) | (GPIO129_DATAOUT << 1) | (GPIO130_DATAOUT << 2) |
	 (GPIO131_DATAOUT << 3) | (GPIO132_DATAOUT << 4) | (GPIO133_DATAOUT << 5) |
	 (GPIO134_DATAOUT << 6) | (GPIO135_DATAOUT << 7) | (GPIO136_DATAOUT << 8) |
	 (GPIO137_DATAOUT << 9) | (GPIO138_DATAOUT << 10) | (GPIO139_DATAOUT << 11) |
	 (GPIO140_DATAOUT << 12) | (GPIO141_DATAOUT << 13) | (GPIO142_DATAOUT << 14) |
	 (GPIO143_DATAOUT << 15)),

	((GPIO144_DATAOUT << 0) | (GPIO145_DATAOUT << 1) | (GPIO146_DATAOUT << 2) |
	 (GPIO147_DATAOUT << 3) | (GPIO148_DATAOUT << 4) | (GPIO149_DATAOUT << 5) |
	 (GPIO150_DATAOUT << 6) | (GPIO151_DATAOUT << 7) | (GPIO152_DATAOUT << 8) |
	 (GPIO153_DATAOUT << 9) | (GPIO154_DATAOUT << 10) | (GPIO155_DATAOUT << 11) |
	 (GPIO156_DATAOUT << 12) | (GPIO157_DATAOUT << 13) | (GPIO158_DATAOUT << 14) |
	 (GPIO159_DATAOUT << 15)),

	((GPIO160_DATAOUT << 0) | (GPIO161_DATAOUT << 1) | (GPIO162_DATAOUT << 2) |
	 (GPIO163_DATAOUT << 3) | (GPIO164_DATAOUT << 4) | (GPIO165_DATAOUT << 5) |
	 (GPIO166_DATAOUT << 6) | (GPIO167_DATAOUT << 7) | (GPIO168_DATAOUT << 8) |
	 (GPIO169_DATAOUT << 9) | (GPIO170_DATAOUT << 10) | (GPIO171_DATAOUT << 11) |
	 (GPIO172_DATAOUT << 12) | (GPIO173_DATAOUT << 13) | (GPIO174_DATAOUT << 14) |
	 (GPIO175_DATAOUT << 15)),

	((GPIO176_DATAOUT << 0) | (GPIO177_DATAOUT << 1) | (GPIO178_DATAOUT << 2) |
	 (GPIO179_DATAOUT << 3) | (GPIO180_DATAOUT << 4) | (GPIO181_DATAOUT << 5) |
	 (GPIO182_DATAOUT << 6) | (GPIO183_DATAOUT << 7) | (GPIO184_DATAOUT << 8) |
	 (GPIO185_DATAOUT << 9) | (GPIO186_DATAOUT << 10) | (GPIO187_DATAOUT << 11) |
	 (GPIO188_DATAOUT << 12) | (GPIO189_DATAOUT << 13) | (GPIO190_DATAOUT << 14) |
	 (GPIO191_DATAOUT << 15)),

	((GPIO192_DATAOUT << 0) | (GPIO193_DATAOUT << 1) | (GPIO194_DATAOUT << 2) |
	 (GPIO195_DATAOUT << 3) | (GPIO196_DATAOUT << 4) | (GPIO197_DATAOUT << 5) |
	 (GPIO198_DATAOUT << 6) | (GPIO199_DATAOUT << 7) | (GPIO200_DATAOUT << 8) |
	 (GPIO201_DATAOUT << 9) | (GPIO202_DATAOUT << 10)),
};                              /*end of gpio_init_dout_data */

/*----------------------------------------------------------------------------*/
u16 gpio_init_mode_data[] = {
	((GPIO0_MODE << 0) | (GPIO1_MODE << 3) | (GPIO2_MODE << 6) |
	 (GPIO3_MODE << 9) | (GPIO4_MODE << 12)),
	((GPIO5_MODE << 0) | (GPIO6_MODE << 3) | (GPIO7_MODE << 6) |
	 (GPIO8_MODE << 9) | (GPIO9_MODE << 12)),
	((GPIO10_MODE << 0) | (GPIO11_MODE << 3) | (GPIO12_MODE << 6) |
	 (GPIO13_MODE << 9) | (GPIO14_MODE << 12)),
	((GPIO15_MODE << 0) | (GPIO16_MODE << 3) | (GPIO17_MODE << 6) |
	 (GPIO18_MODE << 9) | (GPIO19_MODE << 12)),
	((GPIO20_MODE << 0) | (GPIO21_MODE << 3) | (GPIO22_MODE << 6) |
	 (GPIO23_MODE << 9) | (GPIO24_MODE << 12)),
	((GPIO25_MODE << 0) | (GPIO26_MODE << 3) | (GPIO27_MODE << 6) |
	 (GPIO28_MODE << 9) | (GPIO29_MODE << 12)),
	((GPIO30_MODE << 0) | (GPIO31_MODE << 3) | (GPIO32_MODE << 6) |
	 (GPIO33_MODE << 9) | (GPIO34_MODE << 12)),
	((GPIO35_MODE << 0) | (GPIO36_MODE << 3) | (GPIO37_MODE << 6) |
	 (GPIO38_MODE << 9) | (GPIO39_MODE << 12)),
	((GPIO40_MODE << 0) | (GPIO41_MODE << 3) | (GPIO42_MODE << 6) |
	 (GPIO43_MODE << 9) | (GPIO44_MODE << 12)),
	((GPIO45_MODE << 0) | (GPIO46_MODE << 3) | (GPIO47_MODE << 6) |
	 (GPIO48_MODE << 9) | (GPIO49_MODE << 12)),
	((GPIO50_MODE << 0) | (GPIO51_MODE << 3) | (GPIO52_MODE << 6) |
	 (GPIO53_MODE << 9) | (GPIO54_MODE << 12)),
	((GPIO55_MODE << 0) | (GPIO56_MODE << 3) | (GPIO57_MODE << 6) |
	 (GPIO58_MODE << 9) | (GPIO59_MODE << 12)),
	((GPIO60_MODE << 0) | (GPIO61_MODE << 3) | (GPIO62_MODE << 6) |
	 (GPIO63_MODE << 9) | (GPIO64_MODE << 12)),
	((GPIO65_MODE << 0) | (GPIO66_MODE << 3) | (GPIO67_MODE << 6) |
	 (GPIO68_MODE << 9) | (GPIO69_MODE << 12)),
	((GPIO70_MODE << 0) | (GPIO71_MODE << 3) | (GPIO72_MODE << 6) |
	 (GPIO73_MODE << 9) | (GPIO74_MODE << 12)),
	((GPIO75_MODE << 0) | (GPIO76_MODE << 3) | (GPIO77_MODE << 6) |
	 (GPIO78_MODE << 9) | (GPIO79_MODE << 12)),
	((GPIO80_MODE << 0) | (GPIO81_MODE << 3) | (GPIO82_MODE << 6) |
	 (GPIO83_MODE << 9) | (GPIO84_MODE << 12)),
	((GPIO85_MODE << 0) | (GPIO86_MODE << 3) | (GPIO87_MODE << 6) |
	 (GPIO88_MODE << 9) | (GPIO89_MODE << 12)),
	((GPIO90_MODE << 0) | (GPIO91_MODE << 3) | (GPIO92_MODE << 6) |
	 (GPIO93_MODE << 9) | (GPIO94_MODE << 12)),
	((GPIO95_MODE << 0) | (GPIO96_MODE << 3) | (GPIO97_MODE << 6) |
	 (GPIO98_MODE << 9) | (GPIO99_MODE << 12)),
	((GPIO100_MODE << 0) | (GPIO101_MODE << 3) | (GPIO102_MODE << 6) |
	 (GPIO103_MODE << 9) | (GPIO104_MODE << 12)),
	((GPIO105_MODE << 0) | (GPIO106_MODE << 3) | (GPIO107_MODE << 6) |
	 (GPIO108_MODE << 9) | (GPIO109_MODE << 12)),
	((GPIO110_MODE << 0) | (GPIO111_MODE << 3) | (GPIO112_MODE << 6) |
	 (GPIO113_MODE << 9) | (GPIO114_MODE << 12)),
	((GPIO115_MODE << 0) | (GPIO116_MODE << 3) | (GPIO117_MODE << 6) |
	 (GPIO118_MODE << 9) | (GPIO119_MODE << 12)),
	((GPIO120_MODE << 0) | (GPIO121_MODE << 3) | (GPIO122_MODE << 6) |
	 (GPIO123_MODE << 9) | (GPIO124_MODE << 12)),
	((GPIO125_MODE << 0) | (GPIO126_MODE << 3) | (GPIO127_MODE << 6) |
	 (GPIO128_MODE << 9) | (GPIO129_MODE << 12)),
	((GPIO130_MODE << 0) | (GPIO131_MODE << 3) | (GPIO132_MODE << 6) |
	 (GPIO133_MODE << 9) | (GPIO134_MODE << 12)),
	((GPIO135_MODE << 0) | (GPIO136_MODE << 3) | (GPIO137_MODE << 6) |
	 (GPIO138_MODE << 9) | (GPIO139_MODE << 12)),
	((GPIO140_MODE << 0) | (GPIO141_MODE << 3) | (GPIO142_MODE << 6) |
	 (GPIO143_MODE << 9) | (GPIO144_MODE << 12)),
	((GPIO145_MODE << 0) | (GPIO146_MODE << 3) | (GPIO147_MODE << 6) |
	 (GPIO148_MODE << 9) | (GPIO149_MODE << 12)),
	((GPIO150_MODE << 0) | (GPIO151_MODE << 3) | (GPIO152_MODE << 6) |
	 (GPIO153_MODE << 9) | (GPIO154_MODE << 12)),
	((GPIO155_MODE << 0) | (GPIO156_MODE << 3) | (GPIO157_MODE << 6) |
	 (GPIO158_MODE << 9) | (GPIO159_MODE << 12)),
	((GPIO160_MODE << 0) | (GPIO161_MODE << 3) | (GPIO162_MODE << 6) |
	 (GPIO163_MODE << 9) | (GPIO164_MODE << 12)),
	((GPIO165_MODE << 0) | (GPIO166_MODE << 3) | (GPIO167_MODE << 6) |
	 (GPIO168_MODE << 9) | (GPIO169_MODE << 12)),
	((GPIO170_MODE << 0) | (GPIO171_MODE << 3) | (GPIO172_MODE << 6) |
	 (GPIO173_MODE << 9) | (GPIO174_MODE << 12)),
	((GPIO175_MODE << 0) | (GPIO176_MODE << 3) | (GPIO177_MODE << 6) |
	 (GPIO178_MODE << 9) | (GPIO179_MODE << 12)),
	((GPIO180_MODE << 0) | (GPIO181_MODE << 3) | (GPIO182_MODE << 6) |
	 (GPIO183_MODE << 9) | (GPIO184_MODE << 12)),
	((GPIO185_MODE << 0) | (GPIO186_MODE << 3) | (GPIO187_MODE << 6) |
	 (GPIO188_MODE << 9) | (GPIO189_MODE << 12)),
	((GPIO190_MODE << 0) | (GPIO191_MODE << 3) | (GPIO192_MODE << 6) |
	 (GPIO193_MODE << 9) | (GPIO194_MODE << 12)),
	((GPIO195_MODE << 0) | (GPIO196_MODE << 3) | (GPIO197_MODE << 6) |
	 (GPIO198_MODE << 9) | (GPIO199_MODE << 12)),
	((GPIO200_MODE << 0) | (GPIO201_MODE << 3) | (GPIO202_MODE << 6)),
};                              /*end of gpio_init_mode_data */

u16 gpio_init_mode_more_data[] = {
	((GPIO44_MODE << 0) | (GPIO45_MODE << 4) | (GPIO46_MODE << 8)),
	((GPIO47_MODE << 0) | (GPIO48_MODE << 4) | (GPIO49_MODE << 8)),
};                              /*end of gpio_init_mode_more_data */

u16 gpio_init_sim1_2_data[] = {
	((GPIO44_DATAINV << 0) | (GPIO45_DATAINV << 1) | (GPIO46_DATAINV << 2) |
	 (GPIO44_PULLEN << 4) | (GPIO45_PULLEN << 5) | (GPIO46_PULLEN << 6) |
	 (GPIO44_PULL << 8) | (GPIO45_PULL << 9) | (GPIO46_PULL << 10)),
};                              /*end of gpio_init_mode_more_data */

u16 gpio_init_sim2_2_data[] = {
	((GPIO47_DATAINV << 0) | (GPIO48_DATAINV << 1) | (GPIO49_DATAINV << 2) |
	 (GPIO47_PULLEN << 4) | (GPIO48_PULLEN << 5) | (GPIO49_PULLEN << 6) |
	 (GPIO47_PULL << 8) | (GPIO48_PULL << 9) | (GPIO49_PULL << 10)),
};                              /*end of gpio_init_mode_more_data */

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
};                              /*end of gpioext_init_dir_data */

/*----------------------------------------------------------------------------*/
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
};                              /*end of gpioext_init_pullen_data */

/*----------------------------------------------------------------------------*/
u16 gpioext_init_pullsel_data[4] = {
	((GPIOEXT0_PULL << 0) | (GPIOEXT1_PULL << 1) | (GPIOEXT2_PULL << 2) |
	 (GPIOEXT3_PULL << 3) | (GPIOEXT4_PULL << 4) | (GPIOEXT5_PULL << 5) |
	 (GPIOEXT6_PULL << 6) | (GPIOEXT7_PULL << 7) | (GPIOEXT8_PULL << 8) |
	 (GPIOEXT9_PULL << 9) | (GPIOEXT10_PULL << 10) | (GPIOEXT11_PULL << 11) |
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
};                              /*end of gpioext_init_pullsel_data */

/*----------------------------------------------------------------------------*/
u16 gpioext_init_dinv_data[4] = {
	((GPIOEXT0_DATAINV << 0) | (GPIOEXT1_DATAINV << 1) |
	 (GPIOEXT2_DATAINV << 2) | (GPIOEXT3_DATAINV << 3) |
	 (GPIOEXT4_DATAINV << 4) | (GPIOEXT5_DATAINV << 5) |
	 (GPIOEXT6_DATAINV << 6) | (GPIOEXT7_DATAINV << 7) |
	 (GPIOEXT8_DATAINV << 8) | (GPIOEXT9_DATAINV << 9) |
	 (GPIOEXT10_DATAINV << 10) | (GPIOEXT11_DATAINV << 11) |
	 (GPIOEXT12_DATAINV << 12) | (GPIOEXT13_DATAINV << 13) |
	 (GPIOEXT14_DATAINV << 14) | (GPIOEXT15_DATAINV << 15)),

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
};                              /*end of gpioext_init_dinv_data */

/*----------------------------------------------------------------------------*/
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
};                              /*end of gpioext_init_dout_data */

/*----------------------------------------------------------------------------*/
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
};                              /*end of gpioext_init_mode_data */

/*----------------------------------------------------------------------------*/

void mt_gpio_set_default_chip(void)
{
	GPIO_REGS *pReg = (GPIO_REGS *)(GPIO_BASE);
	GPIO_REGS *pReg1 = (GPIO_REGS *)(GPIO1_BASE);
	unsigned int idx;
	u32 val;
	unsigned int dirSize =
		sizeof(gpio_init_dir_data) / sizeof(gpio_init_dir_data[0]);
	unsigned int pullenSize =
		sizeof(gpio_init_pullen_data) / sizeof(gpio_init_pullen_data[0]);
	unsigned int pullselSize =
		sizeof(gpio_init_pullsel_data) / sizeof(gpio_init_pullsel_data[0]);
	unsigned int dinvSize =
		sizeof(gpio_init_dinv_data) / sizeof(gpio_init_dinv_data[0]);
	unsigned int doutSize =
		sizeof(gpio_init_dout_data) / sizeof(gpio_init_dout_data[0]);
	unsigned int modeSize =
		sizeof(gpio_init_mode_data) / sizeof(gpio_init_mode_data[0]);

	GPIODBG("GPIO_BASE(0x%x), GPIO1_BASE(0x%x)\n", GPIO_BASE, GPIO1_BASE);
	GPIODBG("dirSize(%d), pullenSize(%d), pullselSize(%d),"
		"dinvSize(%d), doutSize(%d), modeSize(%d)\n",
		 dirSize, pullenSize, pullselSize,
		 dinvSize, doutSize, modeSize);

	for (idx = 0; idx < dirSize; idx++) {
		val = gpio_init_dir_data[idx];
		GPIO_WR16(&pReg->dir[idx], val);
	}

	for (idx = 0; idx < pullenSize; idx++) {
		val = gpio_init_pullen_data[idx];
		if ((idx >= 3) && (idx <= 8))
			GPIO_WR32(((u32)(&pReg1->pullen[idx]) + 0), val);
		GPIO_WR16(&pReg->pullen[idx], val);
	}

	for (idx = 0; idx < pullselSize; idx++) {
		val = gpio_init_pullsel_data[idx];
		if ((idx >= 3) && (idx <= 8))
			GPIO_WR32(((u32)(&pReg1->pullsel[idx]) + 0), val);
		GPIO_WR16(&pReg->pullsel[idx], val);
	}

	for (idx = 0; idx < dinvSize; idx++) {
		val = gpio_init_dinv_data[idx];
		GPIO_WR16(&pReg->dinv[idx], val);
	}

	for (idx = 0; idx < doutSize; idx++) {
		val = gpio_init_dout_data[idx];
		GPIO_WR16(&pReg->dout[idx], val);
	}

	for (idx = 0; idx < modeSize; idx++) {
		val = gpio_init_mode_data[idx];
		GPIO_WR16(&pReg->mode[idx], val);
	}
	/* set sim GPIO configuration */
	GPIO_WR32(GPIO_BASE + 0x980, gpio_init_mode_more_data[0]);
	GPIO_WR32(GPIO_BASE + 0x9A0, gpio_init_mode_more_data[1]);
	GPIO_WR32(GPIO_BASE + 0x990, gpio_init_sim1_2_data[0]);
	GPIO_WR32(GPIO_BASE + 0x9B0, gpio_init_sim2_2_data[0]);

	GPIOVER("mt_gpio_set_default() done\n");
}

void mt_gpio_set_default_ext(void)
{
	GPIOEXT_REGS *pReg = (GPIOEXT_REGS *)(GPIOEXT_BASE);
	unsigned int idx;
	u32 val;

	for (idx = 0; idx < sizeof(pReg->dir) / sizeof(pReg->dir[0]); idx++) {
		val = gpioext_init_dir_data[idx];
		GPIOEXT_WR((u32)&pReg->dir[idx], val);
	}

	for (idx = 0; idx < sizeof(pReg->pullen) / sizeof(pReg->pullen[0]);
	     idx++) {
		val = gpioext_init_pullen_data[idx];
		GPIOEXT_WR((u32)&pReg->pullen[idx], val);
	}

	for (idx = 0; idx < sizeof(pReg->pullsel) / sizeof(pReg->pullsel[0]);
	     idx++) {
		val = gpioext_init_pullsel_data[idx];
		GPIOEXT_WR((u32)&pReg->pullsel[idx], val);
	}

	for (idx = 0; idx < sizeof(pReg->dinv) / sizeof(pReg->dinv[0]); idx++) {
		val = gpioext_init_dinv_data[idx];
		GPIOEXT_WR((u32)&pReg->dinv[idx], val);
	}

	for (idx = 0; idx < sizeof(pReg->dout) / sizeof(pReg->dout[0]); idx++) {
		val = gpioext_init_dout_data[idx];
		GPIOEXT_WR((u32)&pReg->dout[idx], val);
	}

	for (idx = 0; idx < sizeof(pReg->mode) / sizeof(pReg->mode[0]); idx++) {
		val = gpioext_init_mode_data[idx];
		GPIOEXT_WR((u32)&pReg->mode[idx], val);
	}

	GPIOVER("mt_gpio_set_default_ext() done\n");
}

void mt_gpio_set_dsel(u8 nml2_1v8, u8 bsi_1v8, u8 bpi_1v8)
{
	u32 mask;
	u32 val;
	u32 bit;

#define TDSEL_1V8 0x0
#define TDSEL_3V3 0x5
#define RDSEL_1V8 0x0
#define RDSEL_3V3 0xC
#define GPIO_NML2_OFFSET 0x730
#define GPIO_NML2_PWMR_OFFSET 0x740
#define GPIO_BSI_OFFSET 0x750
#define GPIO_BPI_OFFSET 0x740
	/* set NML2 UART */
	val = GPIO_RD32(GPIO_BASE + GPIO_NML2_OFFSET);
	mask = (1L << 4) - 1;
	bit = 8;
	val &= ~(mask << (bit));
	val |= (nml2_1v8 ? TDSEL_1V8 : TDSEL_3V3) << bit;
	GPIO_WR32(GPIO_BASE + GPIO_NML2_OFFSET, val);

	val = GPIO_RD32(GPIO_BASE + GPIO_NML2_OFFSET);
	mask = (1L << 6) - 1;
	bit = 16;
	val &= ~(mask << (bit));
	val |= (nml2_1v8 ? RDSEL_1V8 : RDSEL_3V3) << bit;
	GPIO_WR32(GPIO_BASE + GPIO_NML2_OFFSET, val);

	/* set NML2 PWM */
	val = GPIO_RD32(GPIO_BASE + GPIO_NML2_OFFSET);
	mask = (1L << 4) - 1;
	bit = 24;
	val &= ~(mask << (bit));
	val |= (nml2_1v8 ? TDSEL_1V8 : TDSEL_3V3) << bit;
	GPIO_WR32(GPIO_BASE + GPIO_NML2_OFFSET, val);

	val = GPIO_RD32(GPIO_BASE + GPIO_NML2_PWMR_OFFSET);
	mask = (1L << 6) - 1;
	bit = 0;
	val &= ~(mask << (bit));
	val |= (nml2_1v8 ? RDSEL_1V8 : RDSEL_3V3) << bit;
	GPIO_WR32(GPIO_BASE + GPIO_NML2_PWMR_OFFSET, val);

	/* set BPI */
	val = GPIO_RD32(GPIO_BASE + GPIO_BPI_OFFSET);
	mask = (1L << 4) - 1;
	bit = 8;
	val &= ~(mask << (bit));
	val |= (bpi_1v8 ? TDSEL_1V8 : TDSEL_3V3) << bit;
	GPIO_WR32(GPIO_BASE + GPIO_BPI_OFFSET, val);

	val = GPIO_RD32(GPIO_BASE + GPIO_BPI_OFFSET);
	mask = (1L << 6) - 1;
	bit = 16;
	val &= ~(mask << (bit));
	val |= (bpi_1v8 ? RDSEL_1V8 : RDSEL_3V3) << bit;
	GPIO_WR32(GPIO_BASE + GPIO_BPI_OFFSET, val);

	/* set BSI */
	val = GPIO_RD32(GPIO_BASE + GPIO_BSI_OFFSET);
	mask = (1L << 4) - 1;
	bit = 0;
	val &= ~(mask << (bit));
	val |= (bsi_1v8 ? TDSEL_1V8 : TDSEL_3V3) << bit;
	GPIO_WR32(GPIO_BASE + GPIO_BSI_OFFSET, val);

	val = GPIO_RD32(GPIO_BASE + GPIO_BSI_OFFSET);
	mask = (1L << 6) - 1;
	bit = 8;
	val &= ~(mask << (bit));
	val |= (bsi_1v8 ? RDSEL_1V8 : RDSEL_3V3) << bit;
	GPIO_WR32(GPIO_BASE + GPIO_BSI_OFFSET, val);
	GPIOVER("NML2 0x%x,BPI 0x%x, BSI 0x%x\n",
		GPIO_RD32(GPIO_BASE + GPIO_NML2_OFFSET),
		GPIO_RD32(GPIO_BASE + GPIO_BPI_OFFSET),
		GPIO_RD32(GPIO_BASE + GPIO_BSI_OFFSET));
}

/* exclude those MSDC, camera, display and high speed emi GPIOs */
u32 GPIO_4ma_tbl[] = {
	10, 11, 12, 13, 14, 15,	 16,  17,  18,	19,
	20, 21, 22, 23, 24, 37,	 38,  39,  40,	41,
	42, 43, 44, 45, 46, 47,	 49,  50,  51,	53,
	54, 55, 56, 57, 58, 59,	 60,  61,  62,	67,
	68, 69, 70, 71, 72, 102, 103, 104, 190, 191,
	192
};

void mt_gpio_set_default(void)
{
	u32 i = 0;

	mt_gpio_set_default_chip();
	mt_gpio_set_default_ext();

	for (i = 0; i < ARRAY_SIZE(GPIO_4ma_tbl); i++) {
		s32 val;

		val = mt_get_gpio_driving_chip(GPIO_4ma_tbl[i]);
		GPIODBG("Before mt_get_gpio_driving_chip(%d),val %d\n",
			GPIO_4ma_tbl[i], val);
		mt_set_gpio_driving_chip(GPIO_4ma_tbl[i], 4);
		val = mt_get_gpio_driving_chip(GPIO_4ma_tbl[i]);
		GPIODBG("After mt_get_gpio_driving_chip(%d),val %d\n",
			GPIO_4ma_tbl[i], val);
	}
}
