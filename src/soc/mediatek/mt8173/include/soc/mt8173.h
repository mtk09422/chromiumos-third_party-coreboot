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

#ifndef SOC_MEDIATEK_MT8173_MT8173_H
#define SOC_MEDIATEK_MT8173_MT8173_H

#define MTK_EMMC_SUPPORT           1
#define BOOTDEV_SDMMC              (1)
#define CFG_BOOT_DEV                (BOOTDEV_SDMMC)

#define CFG_META_BAUDRATE           (115200)

#define CFG_HW_WATCHDOG                 (1)
#define MT8173			(1)
#define CFG_FPGA_PLATFORM 0

extern unsigned char _sram_l2c[];
extern unsigned char _esram_l2c[];
#define _sram_l2c_size (_esram_l2c - _sram_l2c)

extern unsigned char _sram_ttb[];
extern unsigned char _esram_ttb[];
#define _sram_ttb_size	(_esram_ttb - _sram_ttb)

#endif /* SOC_MEDIATEK_MT8173_MT8173_H */
