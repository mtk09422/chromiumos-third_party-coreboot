/*
 * (C) Copyright 2007 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
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
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include "ubootcli.h"

/* COPYING is currently 15951 bytes in size */
#define LICENSE_MAX	20480

int do_license(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	puts("Go read the GPL you lazy sod. I won't print it for you\n");

	return 0;
}

U_BOOT_CMD(
	license, 1, 1, do_license,
	"print GPL license text",
	""
);
