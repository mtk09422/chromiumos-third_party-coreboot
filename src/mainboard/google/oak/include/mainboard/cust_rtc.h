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

#ifndef SOC_MEDIATEK_MT8173_CUST_RTC_H
#define SOC_MEDIATEK_MT8173_CUST_RTC_H

/*
 * Default values for RTC initialization
 * Year (YEA)        : 1970 ~ 2037
 * Month (MTH)       : 1 ~ 12
 * Day of Month (DOM): 1 ~ 31
 */

enum {
	RTC_DEFAULT_YEA	= 2010,
	RTC_DEFAULT_MTH	= 1,
	RTC_DEFAULT_DOM	= 1
};

enum {
	RTC_2SEC_REBOOT_ENABLE	= 1,
	RTC_2SEC_MODE	= 2
};

#endif /* SOC_MEDIATEK_MT8173_CUST_RTC_H */
