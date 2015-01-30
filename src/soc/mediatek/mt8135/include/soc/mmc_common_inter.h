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

#ifndef SOC_MEDIATEK_MT8135_MMC_COMMON_INTER_H
#define SOC_MEDIATEK_MT8135_MMC_COMMON_INTER_H

#include <stdint.h>

u32 mmc_init_device(void);
u32 mmc_get_device_id(u8 *id, u32 len, u32 *fw_len);

#endif /* SOC_MEDIATEK_MT8135_MMC_COMMON_INTER_H */
