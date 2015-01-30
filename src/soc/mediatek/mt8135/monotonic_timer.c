/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 Google Inc.
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

#include <arch/io.h>
#include <timer.h>
#include <soc/mt8135.h>

#define GPT4_MHZ    13
#define TIMER_TICKS(us) ((GPT4_MHZ*(us))
#define TIMER_USECS(ticks) ((ticks)/GPT4_MHZ)

#define GPT4_DAT                    (GPT_BASE + 0x0048)

void timer_monotonic_get(struct mono_time *mt)
{
	mono_time_set_usecs(mt, TIMER_USECS(read32((void *)GPT4_DAT)));
}
