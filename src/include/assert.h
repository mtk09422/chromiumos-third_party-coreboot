/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2010 coresystems GmbH
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

#ifndef __ASSERT_H__
#define __ASSERT_H__

#include <arch/hlt.h>
#include <console/console.h>

#if defined(__PRE_RAM__) && !IS_ENABLED(CONFIG_CACHE_AS_RAM) \
			 && IS_ENABLED(CONFIG_ARCH_X86)

/* ROMCC versions */
#define ASSERT(x) {						\
	if(!(x)) {						\
		print_emerg("ASSERTION ERROR: file '");		\
		print_emerg(__FILE__);				\
		print_emerg("', line 0x");			\
		print_debug_hex32(__LINE__);			\
		print_emerg("\n");				\
		if (IS_ENABLED(CONFIG_FATAL_ASSERTS)) hlt();	\
	}							\
}

#define BUG() {							\
	print_emerg("ERROR: BUG ENCOUNTERED at file '");	\
	print_emerg(__FILE__);					\
	print_emerg("', line 0x");				\
	print_debug_hex32(__LINE__);				\
	print_emerg("\n");					\
	if (IS_ENABLED(CONFIG_FATAL_ASSERTS)) hlt();		\
}

#else

/* GCC and CAR versions */
#define ASSERT(x) {						\
	if (!(x)) {						\
		printk(BIOS_EMERG, "ASSERTION ERROR: file '%s'"	\
			", line %d\n", __FILE__, __LINE__);	\
		if (IS_ENABLED(CONFIG_FATAL_ASSERTS)) hlt();	\
	}							\
}
#define BUG() {							\
	printk(BIOS_EMERG, "ERROR: BUG ENCOUNTERED at file '%s'"\
		", line %d\n", __FILE__, __LINE__);		\
	if (IS_ENABLED(CONFIG_FATAL_ASSERTS)) hlt();		\
}

#endif /* defined(__PRE_RAM__) && !CONFIG_CACHE_AS_RAM */

#define assert(statement)	ASSERT(statement)

#endif // __ASSERT_H__
