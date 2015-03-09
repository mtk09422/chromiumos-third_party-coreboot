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

//JH #include <types.h>
//JH #include <soc/msdc_types.h>
#include <soc/msdc_utils.h>

unsigned int msdc_uffs(unsigned int x)
{
	unsigned int r = 1;

	if (!x)
		return 0;
	if (!(x & 0xffff)) {
		x >>= 16;
		r += 16;
	}
	if (!(x & 0xff)) {
		x >>= 8;
		r += 8;
	}
	if (!(x & 0xf)) {
		x >>= 4;
		r += 4;
	}
	if (!(x & 3)) {
		x >>= 2;
		r += 2;
	}
	if (!(x & 1)) {
		x >>= 1;
		r += 1;
	}
	return r;
}

unsigned int msdc_ntohl(unsigned int n)
{
	unsigned int t;
	unsigned char *b = (unsigned char *)&t;
	*b++ = ((n >> 24) & 0xFF);
	*b++ = ((n >> 16) & 0xFF);
	*b++ = ((n >> 8) & 0xFF);
	*b = ((n) & 0xFF);
	return t;
}

void msdc_get_field(volatile u32 * reg, u32 field, u32 * val)
{
	u32 tv = (u32) * reg;
	*val = ((tv & (field)) >> (msdc_uffs((unsigned int)field) - 1));
}
