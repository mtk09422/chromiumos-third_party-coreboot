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

#include <arch/io.h>
#include <console/console.h>
#include <timer.h>
#include <delay.h>
#include <thread.h>

#include <soc/addressmap.h>

enum {
	GPT4_CON = GPT_BASE + 0x0040,
	GPT4_CLK = GPT_BASE + 0x0044
};

enum {
	GPT4_EN	= 0x01,
	GPT4_CLR = 0x02,
	GPT4_FREERUN = 0x30,
	GPT4_SYS_CLK = 0x00,
	GPT4_CLK_DIV1 = 0x0000,
	GPT4_CLK_DIV2 = 0x0001,
	GPT4_CLK_SETTING = GPT4_SYS_CLK | GPT4_CLK_DIV1
};

enum {
	XGPT_CTL = 0x10200670,
	XGPT_IDX = 0x10200674
};
/**
 * init_timer - initialize timer
 */
void init_timer (void)
{
	/* power on GPT4 */
	write32((void *)XGPT_IDX, 0);
	write32((void *)XGPT_CTL, 1);

	/* GPT4 stop */
	clrbits_le32((void *)GPT4_CON, GPT4_EN);
	setbits_le32((void *)GPT4_CON, GPT4_CLR);

	/* GPT4 start */
	write32((void *)GPT4_CLK, GPT4_CLK_SETTING);
	write32((void *)GPT4_CON, GPT4_EN | GPT4_FREERUN);
}

