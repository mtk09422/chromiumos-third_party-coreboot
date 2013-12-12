/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __COMMON_H_
#define __COMMON_H_	1

//#include <linux/bitops.h>
//#include <linux/types.h>
//#include <linux/string.h>
//#include <linux/stringify.h>
//#include <asm/ptrace.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
//#include <pci.h>
#include <string.h>
#include <stdlib.h>
#include "linker_lists.h"

/*
 * Output a debug text when condition "cond" is met. The "cond" should be
 * computed by a preprocessor in the best case, allowing for the best
 * optimization.
 */
#define debug_cond(cond, fmt, args...)		\
	do {					\
		if (cond)			\
			printf(fmt, ##args);	\
	} while (0)

#define debug(fmt, args...)			\
	debug_cond(_DEBUG, fmt, ##args)

/*
 * General Purpose Utilities
 */
#define min(X, Y)				\
	({ typeof(X) __x = (X);			\
		typeof(Y) __y = (Y);		\
		(__x < __y) ? __x : __y; })

#define max(X, Y)				\
	({ typeof(X) __x = (X);			\
		typeof(Y) __y = (Y);		\
		(__x > __y) ? __x : __y; })

#define min3(X, Y, Z)				\
	({ typeof(X) __x = (X);			\
		typeof(Y) __y = (Y);		\
		typeof(Z) __z = (Z);		\
		__x < __y ? (__x < __z ? __x : __z) :	\
		(__y < __z ? __y : __z); })

#define max3(X, Y, Z)				\
	({ typeof(X) __x = (X);			\
		typeof(Y) __y = (Y);		\
		typeof(Z) __z = (Z);		\
		__x > __y ? (__x > __z ? __x : __z) :	\
		(__y > __z ? __y : __z); })

#define MIN3(x, y, z)  min3(x, y, z)
#define MAX3(x, y, z)  max3(x, y, z)

/*
 * Return the absolute value of a number.
 *
 * This handles unsigned and signed longs, ints, shorts and chars.  For all
 * input types abs() returns a signed long.
 *
 * For 64-bit types, use abs64()
 */
#define abs(x) ({						\
		long ret;					\
		if (sizeof(x) == sizeof(long)) {		\
			long __x = (x);				\
			ret = (__x < 0) ? -__x : __x;		\
		} else {					\
			int __x = (x);				\
			ret = (__x < 0) ? -__x : __x;		\
		}						\
		ret;						\
	})

#define abs64(x) ({				\
		s64 __x = (x);			\
		(__x < 0) ? -__x : __x;		\
	})

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

/*
 * Function Prototypes
 */

/* common/main.c */
void	main_loop	(void);
int run_command(const char *cmd, int flag);

/**
 * Run a list of commands separated by ; or even \0
 *
 * Note that if 'len' is not -1, then the command does not need to be nul
 * terminated, Memory will be allocated for the command in that case.
 *
 * @param cmd	List of commands to run, each separated bu semicolon
 * @param len	Length of commands excluding terminator if known (-1 if not)
 * @param flag	Execution flags (CMD_FLAG_...)
 * @return 0 on success, or != 0 on error.
 */
int run_command_list(const char *cmd, int len, int flag);
int	ubreadline	(const char *const prompt);
int	ubreadline_into_buffer(const char *const prompt, char *buffer,
			int timeout);
int	parse_line (char *, char *[]);
void	init_cmd_timeout(void);
void	reset_cmd_timeout(void);
extern char console_buffer[];

/**
 * Show the DRAM size in a board-specific way
 *
 * This is used by boards to display DRAM information in their own way.
 *
 * @param size	Size of DRAM (which should be displayed along with other info)
 */
void board_show_dram(uint32_t size);
/*
 *
 * Defined in arch/$(ARCH)/lib/bootm.c
 *
 * @blob:	FDT blob to write to
 * @return 0 if ok, or -ve FDT_ERR_... on failure
 */
int arch_fixup_memory_node(void *blob);

/* common/flash.c */
void flash_perror (int);

/* common/cmd_source.c */
int	source (uint32_t addr, const char *fit_uname);

extern uint32_t load_addr;		/* Default Load Address */
extern uint32_t save_addr;		/* Default Save Address */
extern uint32_t save_size;		/* Default Save Size */

/* common/cmd_doc.c */
void	doc_probe(unsigned long physadr);

/* common/cmd_net.c */
int do_tftpb(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

/* common/cmd_fat.c */
int do_fat_fsload(cmd_tbl_t *, int, int, char * const []);

/* common/cmd_ext2.c */
int do_ext2load(cmd_tbl_t *, int, int, char * const []);

/* common/cmd_nvedit.c */
int	env_init     (void);
void	env_relocate (void);
int	envmatch     (uint8_t *, int);

/* Avoid unfortunate conflict with libc's getenv() */
#ifdef CONFIG_SANDBOX
#define getenv uboot_getenv
#endif
char	*getenv	     (const char *);
int	getenv_f     (const char *name, char *buf, unsigned len);
uint32_t getenv_ulong(const char *name, int base, uint32_t default_val);

/**
 * getenv_hex() - Return an environment variable as a hex value
 *
 * Decode an environment as a hex number (it may or may not have a 0x
 * prefix). If the environment variable cannot be found, or does not start
 * with hex digits, the default value is returned.
 *
 * @varname:		Variable to decode
 * @default_val:	Value to return on error
 */
uint32_t getenv_hex(const char *varname, uint32_t default_val);

/*
 * Read an environment variable as a boolean
 * Return -1 if variable does not exist (default to true)
 */
int getenv_yesno(const char *var);
int	saveenv	     (void);
//int	setenv	     (const char *, const char *);
int setenv_ulong(const char *varname, uint32_t value);
int setenv_hex(const char *varname, uint32_t value);
/**
 * setenv_addr - Set an environment variable to an address in hex
 *
 * @varname:	Environmet variable to set
 * @addr:	Value to set it to
 * @return 0 if ok, 1 on error

static inline int setenv_addr(const char *varname, const void *addr)
{
	uint32_t stupid_u_boot = (uint32_t) addr;
	return setenv_hex(varname, stupid_u_boot);
}
 */
/* HACK */
#define _DEBUG 1
#define CONFIG_CMDLINE 1
#define CONFIG_SYS_CBSIZE 512
#define CONFIG_SYS_PROMPT "C: "
#define CONFIG_SYS_MAXARGS 32
#define CONFIG_CMDLINE_EDITING 1
#define WATCHDOG_RESET()
#endif	/* __COMMON_H_ */
