/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 The Chromium OS Authors. All rights reserved.
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

#include <bcd.h>
#include <pc80/mc146818rtc.h>
#include <rtc.h>

int rtc_set(const struct rtc_time *time)
{
	cmos_write(bin2bcd(time->sec), RTC_CLK_SECOND);
	cmos_write(bin2bcd(time->min), RTC_CLK_MINUTE);
	cmos_write(bin2bcd(time->hour), RTC_CLK_HOUR);
	cmos_write(bin2bcd(time->mday), RTC_CLK_DAYOFMONTH);
	cmos_write(bin2bcd(time->mon), RTC_CLK_MONTH);
	cmos_write(bin2bcd(time->year % 100), RTC_CLK_YEAR);
	if (CONFIG_DRIVERS_RTC_HAS_ALTCENTURY)
		cmos_write(bin2bcd(time->year / 100),
			   RTC_CLK_ALTCENTURY);
	cmos_write(bin2bcd(time->wday + 1), RTC_CLK_DAYOFWEEK);
	return 0;
}

int rtc_get(struct rtc_time *time)
{
	time->sec = bcd2bin(cmos_read(RTC_CLK_SECOND));
	time->min = bcd2bin(cmos_read(RTC_CLK_MINUTE));
	time->hour = bcd2bin(cmos_read(RTC_CLK_HOUR));
	time->mday = bcd2bin(cmos_read(RTC_CLK_DAYOFMONTH));
	time->mon = bcd2bin(cmos_read(RTC_CLK_MONTH));
	time->year = bcd2bin(cmos_read(RTC_CLK_YEAR));
	if (CONFIG_DRIVERS_RTC_HAS_ALTCENTURY)
		time->year += bcd2bin(cmos_read(RTC_CLK_ALTCENTURY)) * 100;
	else
		time->year += 2000;
	time->wday = bcd2bin(cmos_read(RTC_CLK_DAYOFWEEK)) - 1;
	return 0;
}
