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

#ifndef SOC_MEDIATEK_MT8135_ADDR_TRANS_H
#define SOC_MEDIATEK_MT8135_ADDR_TRANS_H

#include <stdint.h>

typedef struct addr_trans_info {
	u32 id;
	u64 len;
} addr_trans_info_t;

typedef struct addr_trans_tbl {
	u32 num;
	addr_trans_info_t *info;
} addr_trans_tbl_t;

typedef struct virt_addr {
	u64 addr;
} virt_addr_t;

typedef struct phys_addr {
	u32 id;
	u64 addr;
} phys_addr_t;

int virt_to_phys_addr(addr_trans_tbl_t *tbl, virt_addr_t *virt,
		      phys_addr_t *phys);
int phys_to_virt_addr(addr_trans_tbl_t *tbl, phys_addr_t *phys,
		      virt_addr_t *virt);

#endif /* SOC_MEDIATEK_MT8135_ADDR_TRANS_H */
