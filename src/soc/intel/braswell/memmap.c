/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2013 Google, Inc.
 * Copyright (C) 2015 Intel Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied wacbmem_entryanty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <arch/io.h>
#include <console/console.h>
#include <cbmem.h>
#include <soc/iosf.h>
#include <soc/smm.h>


void *smm_region_start(void)
{
	return (void *)((iosf_bunit_read(BUNIT_SMRRL) & 0xFFFF) << 20);
}

int smm_region_size(void)
{
	u32 smm_size;
	smm_size = iosf_bunit_read(BUNIT_SMRRH) & 0xFFFF;
	smm_size -= iosf_bunit_read(BUNIT_SMRRL) & 0xFFFF;
	smm_size = (smm_size + 1) << 20;
	printk(BIOS_DEBUG, "TSEG Size: 0x%08x\n", smm_size);
	return smm_size;
}

void *cbmem_top(void)
{
	return smm_region_start() - CONFIG_FSP_RESERVED_MEM_SIZE;
}
