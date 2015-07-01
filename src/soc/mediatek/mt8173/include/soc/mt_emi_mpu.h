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

#ifndef __MT_EMI_MPU_H
#define __MT_EMI_MPU_H

#include "mt8173.h"

/* EMI_MPU */
#define EMI_MPUA (EMI_BASE+0x0160)
#define EMI_MPUB (EMI_BASE+0x0168)
#define EMI_MPUC (EMI_BASE+0x0170)
#define EMI_MPUD (EMI_BASE+0x0178)
#define EMI_MPUE (EMI_BASE+0x0180)
#define EMI_MPUF (EMI_BASE+0x0188)
#define EMI_MPUG (EMI_BASE+0x0190)
#define EMI_MPUH (EMI_BASE+0x0198)
#define EMI_MPUI (EMI_BASE+0x01A0)
#define EMI_MPUJ (EMI_BASE+0x01A8)
#define EMI_MPUK (EMI_BASE+0x01B0)
#define EMI_MPUL (EMI_BASE+0x01B8)
#define EMI_MPUM (EMI_BASE+0x01C0)
#define EMI_MPUN (EMI_BASE+0x01C8)
#define EMI_MPUO (EMI_BASE+0x01D0)
#define EMI_MPUP (EMI_BASE+0x01D8)
#define EMI_MPUQ (EMI_BASE+0x01E0)
#define EMI_MPUR (EMI_BASE+0x01E8)
#define EMI_MPUS (EMI_BASE+0x01F0)
#define EMI_MPUT (EMI_BASE+0x01F8)

#define EMI_MPUU (EMI_BASE+0x0200)
#define EMI_MPUY (EMI_BASE+0x0220)

#define NO_PROTECTION 0
#define SEC_RW 1
#define SEC_RW_NSEC_R 2
#define SEC_RW_NSEC_W 3
#define SEC_R_NSEC_R 4
#define FORBIDDEN 5
#define SEC_R_NSEC_RW 6

/* EMI memory protection align 64K */
#define EMI_MPU_ALIGNMENT 0x10000
#define OOR_VIO 0x00000200

#define SET_ACCESS_PERMISSON(d3, d2, d1, d0) (((d3) << 9) | ((d2) << 6) | ((d1) << 3) | (d0))

extern int emi_mpu_set_region_protection(unsigned int start_addr, unsigned int end_addr, int region, unsigned int access_permission);

#endif  /* !__MT_EMI_MPU_H */
