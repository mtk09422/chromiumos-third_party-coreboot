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

#ifndef __SOC_MEDIATEK_MT8173_INCLUDE_SOC_ADDRESS_MAP_H__
#define __SOC_MEDIATEK_MT8173_INCLUDE_SOC_ADDRESS_MAP_H__

#include <stddef.h>
#include <stdint.h>

enum {
	IO_PHYS		= 0x10000000,
	VER_BASE	= 0x08000000
};

enum {
	CKSYS_BASE		= IO_PHYS,
	INFRACFG_AO_BASE	= IO_PHYS + 0x1000,
	PERI_CON_BASE		= IO_PHYS + 0x3000,
	DRAMC0_BASE		= IO_PHYS + 0x4000,
	GPIO_BASE		= IO_PHYS + 0x5000,
	SPM_BASE		= IO_PHYS + 0x6000,
	RGU_BASE		= IO_PHYS + 0x7000,
	GPT_BASE		= IO_PHYS + 0x8000,
	PMIC_WRAP_BASE		= IO_PHYS + 0xD000,
	DDRPHY_BASE		= IO_PHYS + 0xF000,
	CA15L_CONFIG_BASE	= IO_PHYS + 0x200200,
	EMI_BASE		= IO_PHYS + 0x203000,
	APMIXED_BASE		= IO_PHYS + 0x209000,
	UART0_BASE		= IO_PHYS + 0x1002000,
	SPI_BASE		= IO_PHYS + 0x100A000,
	SFLASH_REG_BASE		= IO_PHYS + 0x100D000,
	MSDC0_BASE		= IO_PHYS + 0x1230000,
	SSUSB_MAC_BASE		= IO_PHYS + 0x1270000,
	SSUSB_SIF_BASE		= IO_PHYS + 0x1280000,
	MMSYS_CONFIG_BASE	= IO_PHYS + 0x4000000,
	EFUSE_BASE		= IO_PHYS + 0x206000
};

enum {
	AP_PERI_GLOBALCON_PDN0	= PERI_CON_BASE + 0x10,
};

enum {
	MTK_MCUCFG_BASE		= 0x10200000,
	MTK_GICD_BASE		= 0x10221000,
	MTK_GICC_BASE		= 0x10222000
};

enum {
	CARVEOUT_TZ,
	CARVEOUT_NUM
};

/* Find memory below and above 4GiB boundary repsectively. All units 1MiB. */
void memory_in_range_below_4gb(uintptr_t *base_mib, uintptr_t *end_mib);
void memory_in_range_above_4gb(uintptr_t *base_mib, uintptr_t *end_mib);

/* Provided the careout id, obtain the base and size in 1MiB units. */
void carveout_range(int id, uintptr_t *base_mib, size_t *size_mib);

void trustzone_region_init(void);

#endif /* __SOC_MEDIATEK_MT8173_INCLUDE_SOC_ADDRESS_MAP_H___ */
