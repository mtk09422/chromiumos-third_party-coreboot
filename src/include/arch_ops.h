/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2014 Imagination Technologies
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of the License.
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

#ifndef _ARCH_OPS_H_
#define _ARCH_OPS_H_

#include <stddef.h>
#include <stdint.h>

/* For each segment of a program loaded this function is called*/
void arch_program_segment_loaded(uintptr_t start, size_t size);

/* Upon completion of loading a program this function is called */
void arch_program_loaded(void);

#endif /* _ARCH_OPS_H_ */
