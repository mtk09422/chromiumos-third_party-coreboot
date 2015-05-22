/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2009 Myles Watson <mylesgw@gmail.com>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA, 02110-1301 USA
 */

/* This file is for "nuisance prototypes" that have no other home. */

#ifndef __LIB_H__
#define __LIB_H__
#include <stddef.h>
#include <stdint.h>

/* Defined in src/lib/lzma.c */
unsigned long ulzma(unsigned char *src, unsigned char *dst);

/* Defined in src/arch/x86/boot/gdt.c */
void move_gdt(void);

/* Defined in src/lib/ramtest.c */
void ram_check(unsigned long start, unsigned long stop);
int ram_check_nodie(unsigned long start, unsigned long stop);
void quick_ram_check(void);

/* Defined in primitive_memtest.c */
int primitive_memtest(uintptr_t base, uintptr_t size);

/* Defined in src/lib/stack.c */
int checkstack(void *top_of_stack, int core);

/* Defined in romstage.c */
#if CONFIG_CPU_AMD_GEODE_LX
void cache_as_ram_main(void);
#else
void cache_as_ram_main(unsigned long bist, unsigned long cpu_init_detectedx);
#endif

/* Defined in src/lib/hexdump.c */
void hexdump_bounds(const void *memory, size_t length, const void *base,
	char separator, uint32_t extra_space, const char *bounds,
	const char *ellipse, uint32_t match_zeros_only,
	uint32_t print_duplicate_lines);
void hexdump(const void *memory, size_t length);
void hexdump32(char LEVEL, const void *d, size_t len);

#if !(defined(__ROMCC__))
/* Count Leading Zeroes: clz(0) == 32, clz(0xf) == 28, clz(1 << 31) == 0 */
static inline int clz(u32 x) { return x ? __builtin_clz(x) : sizeof(x) * 8; }
/* Integer binary logarithm (rounding down): log2(0) == -1, log2(5) == 2 */
static inline int log2(u32 x) { return sizeof(x) * 8 - clz(x) - 1; }
/* Find First Set: __ffs(1) == 0, __ffs(0) == -1, __ffs(1<<31) == 31 */
static inline int __ffs(u32 x) { return log2(x & (u32)(-(s32)x)); }
#endif

#endif /* __LIB_H__ */
