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

#ifndef SOC_MEDIATEK_MT8135_RTC_H
#define SOC_MEDIATEK_MT8135_RTC_H

#include <stdint.h>

#ifndef bool
typedef unsigned char bool;
#endif

#ifndef boolean
enum boolean { false, true };
#endif

/* RTC registers */
enum {
	RTC_BBPU = 0xE000,
	RTC_IRQ_STA = 0xE002,
	RTC_IRQ_EN = 0xE004,
	RTC_CII_EN = 0xE006
};

enum {
	RTC_TC_SEC = 0xE00A,
	RTC_TC_MIN = 0xE00C,
	RTC_TC_HOU = 0xE00E,
	RTC_TC_DOM = 0xE010,
	RTC_TC_DOW = 0xE012,
	RTC_TC_MTH = 0xE014,
	RTC_TC_YEA = 0xE016
};

enum {
	RTC_AL_SEC = 0xE018,
	RTC_AL_MIN = 0xE01A,
	RTC_AL_HOU = 0xE01C,
	RTC_AL_DOM = 0xE01E,
	RTC_AL_DOW = 0xE020,
	RTC_AL_MTH = 0xE022,
	RTC_AL_YEA = 0xE024,
	RTC_AL_MASK = 0xE008,
};

enum {
	RTC_OSC32CON = 0xE026,
	RTC_CON = 0xE03E,
	RTC_WRTGR = 0xE03C
};

enum {
	RTC_POWERKEY1 = 0xE028,
	RTC_POWERKEY2 = 0xE02A
};

enum {
	RTC_PDN1 = 0xE02C,
	RTC_PDN2 = 0xE02E,
	RTC_SPAR0 = 0xE030,
	RTC_SPAR1 = 0xE032,
	RTC_PROT = 0xE036,
	RTC_DIFF = 0xE038,
	RTC_CALI = 0xE03A
};

#define RTC_BBPU_PWREN          (1U << 0)	/* BBPU = 1 when alarm occurs */
#define RTC_BBPU_BBPU           (1U << 2)	/* 1: power on, 0: power down */
#define RTC_BBPU_AUTO           (1U << 3)	/* BBPU = 0 when xreset_rstb goes low */
#define RTC_BBPU_CLRPKY         (1U << 4)
#define RTC_BBPU_RELOAD         (1U << 5)
#define RTC_BBPU_CBUSY          (1U << 6)
#define RTC_BBPU_KEY            (0x43 << 8)

#define RTC_IRQ_STA_AL          (1U << 0)
#define RTC_IRQ_STA_TC          (1U << 1)
#define RTC_IRQ_STA_LP          (1U << 3)

#define RTC_IRQ_EN_AL           (1U << 0)
#define RTC_IRQ_EN_TC           (1U << 1)
#define RTC_IRQ_EN_ONESHOT      (1U << 2)
#define RTC_IRQ_EN_LP           (1U << 3)
#define RTC_IRQ_EN_ONESHOT_AL   (RTC_IRQ_EN_ONESHOT | RTC_IRQ_EN_AL)

#define RTC_OSC32CON_AMPEN      (1U << 8)	/* enable low power mode */
#define RTC_OSC32CON_LNBUFEN    (1U << 11)	/* ungate 32K to ABB */

#define RTC_POWERKEY1_KEY       0xa357
#define RTC_POWERKEY2_KEY       0x67d2

#define RTC_CON_LPEN            (1U << 2)
#define RTC_CON_LPRST           (1U << 3)
#define RTC_CON_CDBO            (1U << 4)
#define RTC_CON_F32KOB          (1U << 5)	/* 0: RTC_GPIO exports 32K */
#define RTC_CON_GPO             (1U << 6)
#define RTC_CON_GOE             (1U << 7)	/* 1: GPO mode, 0: GPI mode */
#define RTC_CON_GSR             (1U << 8)
#define RTC_CON_GSMT            (1U << 9)
#define RTC_CON_GPEN            (1U << 10)
#define RTC_CON_GPU             (1U << 11)
#define RTC_CON_GE4             (1U << 12)
#define RTC_CON_GE8             (1U << 13)
#define RTC_CON_GPI             (1U << 14)
#define RTC_CON_LPSTA_RAW       (1U << 15)	/* 32K was stopped */
#define RG_FQMTR_PDN            (1U << PMIC_RG_FQMTR_PDN_SHIFT)
#define RTC_CALI_BBPU_2SEC_EN			(1U << 8)
#define RTC_CALI_BBPU_2SEC_MODE_SHIFT		9
#define RTC_CALI_BBPU_2SEC_MODE_MSK		(3U << RTC_CALI_BBPU_2SEC_MODE_SHIFT)
#define RTC_CALI_BBPU_2SEC_STAT			(1U << 11)

/* we map HW YEA 0 (2000) to 1968 not 1970 because 2000 is the leap year */
#define RTC_MIN_YEAR            1968
#define RTC_NUM_YEARS           128

void rtc_bbpu_power_on(void);
void rtc_mark_bypass_pwrkey(void);
u16 rtc_rdwr_uart_bits(u16 *val);
bool rtc_boot_check(void);
void pl_power_off(void);
bool rtc_2sec_reboot_check(void);

#endif /* SOC_MEDIATEK_MT8135_RTC_H */
