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

#ifndef SOC_MEDIATEK_MT8135_KEY_H
#define SOC_MEDIATEK_MT8135_KEY_H

#define KP_STA		(KPD_BASE + 0x0000)
#define KP_MEM1		(KPD_BASE + 0x0004)
#define KP_MEM2		(KPD_BASE + 0x0008)
#define KP_MEM3		(KPD_BASE + 0x000c)
#define KP_MEM4		(KPD_BASE + 0x0010)
#define KP_MEM5		(KPD_BASE + 0x0014)
#define KP_DEBOUNCE	(KPD_BASE + 0x0018)
#define KP_PMIC		(KPD_BASE + 0x001C)

#define KPD_NUM_MEMS	5
#define KPD_MEM5_BITS	8

#define KPD_NUM_KEYS	72	/* 4 * 16 + KPD_MEM5_BITS */

/* timeout period. 0: 5sec; 1: 7sec; 2: 9sec; 3: 11sec */
#define KPD_PMIC_LPRST_TD 1
#define ONEKEY_REBOOT_NORMAL_MODE_PL
/* #define TWOKEY_REBOOT_NORMAL_MODE_PL */

void set_kpd_pmic_mode(void);
void disable_PMIC_kpd_clock(void);
void enable_PMIC_kpd_clock(void);
u8 mtk_detect_key(unsigned short key);

#endif /* SOC_MEDIATEK_MT8135_KEY_H */
