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

#ifndef SOC_MEDIATEK_MT8173_MEMORY_H
#define SOC_MEDIATEK_MT8173_MEMORY_H

/*
 * DEBUG CONTROL
 */
#define MEM_TEST                (1)

/* do not change the test size !!!! */
#define MEM_TEST_SIZE           (0x2000)

/*
 * DRAM SIZE
 */
enum {
	E1_DRAM_SIZE = (0x10000000),
	E2_DRAM_SIZE = (0x08000000)
};

/*
 * EXPOSED API
 */
u32 mt6516_get_hardware_ver(void);
void mt_mem_init(void);

#endif /* SOC_MEDIATEK_MT8173_MEMORY_H */
