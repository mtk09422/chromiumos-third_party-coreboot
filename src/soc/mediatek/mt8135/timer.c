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

#include <arch/io.h>
#include <console/console.h>
#include <timer.h>
#include <delay.h>
#include <thread.h>

#include <soc/mt8135.h>

enum {
	GPT4_CON = GPT_BASE + 0x0040,
	GPT4_CLK = GPT_BASE + 0x0044
};

enum {
	GPT4_EN = 0x01,
	GPT4_CLR = 0x02,
	GPT4_FREERUN = 0x30,
	GPT4_SYS_CLK = 0x00,
	GPT4_CLK_DIV1 = 0x0000,
	GPT4_CLK_DIV2 = 0x0001,
	GPT4_CLK_SETTING = GPT4_SYS_CLK | GPT4_CLK_DIV1
};

void init_timer(void)
{
	/* power on GPT4 */
	clrbits_le32((void *)AP_PERI_GLOBALCON_PDN0, 1 << 13);

	/* stop GPT4 and clear its counter */
	clrbits_le32((void *)GPT4_CON, GPT4_EN);
	setbits_le32((void *)GPT4_CON, GPT4_CLR);

	/* start GPT4 */
	write32(GPT4_CLK_SETTING, (void *)GPT4_CLK);
	write32(GPT4_FREERUN | GPT4_EN, (void *)GPT4_CON);
}

void udelay(unsigned usec)
{
	struct mono_time current, end;

	if (!thread_yield_microseconds(usec))
		return;

	timer_monotonic_get(&current);
	end = current;
	mono_time_add_usecs(&end, usec);

	if (mono_time_after(&current, &end)) {
		printk(BIOS_EMERG, "udelay: 0x%08x is impossibly large\n",
		       usec);
		/* There's not much we can do if usec is too big. Use a long,
		 * paranoid delay value and hope for the best... */
		end = current;
		mono_time_add_usecs(&end, USECS_PER_SEC);
	}

	while (mono_time_before(&current, &end))
		timer_monotonic_get(&current);
}
