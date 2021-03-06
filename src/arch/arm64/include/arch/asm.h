/*
 * This file is part of the coreboot project.
 *
 * Copyright 2013 Google Inc.
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

#ifndef __ARM_ARM64_ASM_H
#define __ARM_ARM64_ASM_H

#define ENDPROC(name) \
	.type name, %function; \
	END(name)

#define ENTRY_WITH_ALIGN(name, bits) \
	.section .text.name, "ax", %progbits; \
	.global name; \
	.align bits; \
	name:

#define ENTRY(name) ENTRY_WITH_ALIGN(name, 0)

#define END(name) \
	.size name, .-name

/*
 * Certain SoCs have an alignment requiremnt for the CPU reset vector.
 * Align to a 64 byte typical cacheline for now.
 */
#define CPU_RESET_ENTRY(name) ENTRY_WITH_ALIGN(name, 6)

#endif	/* __ARM_ARM64_ASM_H */
