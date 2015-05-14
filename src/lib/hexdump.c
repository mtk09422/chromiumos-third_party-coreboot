/*
 * Copyright 2013 Google Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <console/console.h>
#include <lib.h>

static int isprint(int c)
{
	return (c >= 32 && c <= 126);
}

void hexdump_bounds(const void *memory, size_t length, const void *base,
	char separator, uint32_t extra_spaces, const char *bounds,
	const char *ellipse, uint32_t match_zeros_only,
	uint32_t print_duplicate_lines)
{
	int i;
	uint8_t *m;
	int matched_lines = -1;
	uint8_t *offset;
	uint8_t previous_values[16];

	m = (uint8_t *) memory;
	offset = (uint8_t *)(m - (uint8_t *)base);

	for (i = 0; i < length; i += 16) {
		int j;

		matched_lines++;
		for (j = 0; j < 16; j++) {
			if (m[i + j] != (match_zeros_only ? 0 :
				previous_values[j])) {
				matched_lines = 0;
				break;
			}
		}
		if (matched_lines == 0) {
			for (j = 0; j < 16; j++)
				previous_values[j] = m[i + j];
		}

		if (matched_lines < print_duplicate_lines) {
			printk(BIOS_DEBUG, "%p%c", offset + i, separator);
			for (j = 0; j < 16; j++) {
				if ((extra_spaces & (1 << j)) != 0)
					printk(BIOS_DEBUG, " ");
				printk(BIOS_DEBUG, " %02x", m[i + j]);
			}
			printk(BIOS_DEBUG, "  ");
			if (bounds)
				printk(BIOS_DEBUG, "%s", bounds);
			for (j = 0; j < 16; j++)
				printk(BIOS_DEBUG, "%c",
				       isprint(m[i + j]) ? m[i + j] : '.');
			if (bounds)
				printk(BIOS_DEBUG, "%s", bounds);
			printk(BIOS_DEBUG, "\n");
		} else if (matched_lines == print_duplicate_lines) {
			printk(BIOS_DEBUG, "%s\n", ellipse);
		}
	}
}

void hexdump(const void *memory, size_t length)
{
	hexdump_bounds(memory, length, NULL, ':', 0, NULL, "...", 1, 2);
}

void hexdump32(char LEVEL, const void *d, size_t len)
{
	int count = 0;

	while (len > 0) {
		if (count % 8 == 0) {
			printk(LEVEL, "\n");
			printk(LEVEL, "%p:", d);
		}
		printk(LEVEL, " 0x%08lx", *(unsigned long *)d);
		count++;
		len--;
		d += 4;
	}

	printk(LEVEL, "\n\n");
}
