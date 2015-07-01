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

#ifndef __SOC_MEDIATEK_MT8173_CPU_H__
#define __SOC_MEDIATEK_MT8173_CPU_H__

/* Ask a core to enter sleep mode, and boot to entry_64 address upon wakeup */
void cpu_sleep(unsigned int cpu, uintptr_t entry_64);

/* Wakeup a core specified by cpu id */
void cpu_wakeup(unsigned cpu);

/* Start a core in 64-bit mode at the entry_64 address. */
void start_cpu(unsigned int cpu, uintptr_t entry_64);

/* Config secondary CPUs to boot in 64-bit mode */
void set_secondary_cpu_boot_arm64(void);

#endif /* __SOC_MEDIATEK_MT8173_CPU_H__ */
