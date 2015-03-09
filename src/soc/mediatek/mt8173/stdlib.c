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

#include <string.h>
#include <soc/stdlib.h>

int atoi(const char *s)
{
	static const char digits[] = "0123456789"; /* legal digits in order */
	unsigned val = 0;	/* value we're accumulating */
	int neg = 0;		/* set to true if we see a minus sign */

	/* skip whitespace */
	while (*s == ' ' || *s == '\t') {
		s++;
	}

	/* check for sign */
	if (*s == '-') {
		neg = 1;
		s++;
	} else if (*s == '+') {
		s++;
	}

	/* process each digit */
	while (*s) {
		const char *where;
		unsigned digit;

		/* look for the digit in the list of digits */
		where = strchr(digits, *s);
		if (where == 0) {
			/* not found; not a digit, so stop */
			break;
		}

		/* get the index into the digit list, which is the value */
		digit = (where - digits);

		/* could (should?) check for overflow here */

		/* shift the number over and add in the new digit */
		val = val * 10 + digit;

		/* look at the next character */
		s++;
	}

	/* handle negative numbers */
	if (neg) {
		return -val;
	}

	/* done */
	return val;
}
