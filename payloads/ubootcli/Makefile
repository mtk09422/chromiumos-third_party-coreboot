##
## This file is part of the bayou project.
##
## Copyright (C) 2008 Advanced Micro Devices, Inc.
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License version 2 as
## published by the Free Software Foundation.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
##

include build/libpayload/libpayload.config

export src := $(shell pwd)
export obj := $(src)/build

LIBPAYLOAD_DIR := $(obj)/libpayload

$(if $(wildcard .xcompile),,$(eval $(shell bash ../libpayload/util/xcompile/xcompile > .xcompile)))
include .xcompile

ARCHDIR-$(CONFIG_LP_ARCH_ARM)     := arm
ARCHDIR-$(CONFIG_LP_ARCH_X86)     := x86

ARCH-y := $(ARCHDIR-y)

classes-$(CONFIG_LP_PCI) += build/libpayload/lib/libpci.a
classes-$(CONFIG_LP_CURSES) += build/libpayload/lib/libcurses.a
classes-$(CONFIG_LP_PDCURSES) += build/libpayload/lib/libmenu.a
classes-$(CONFIG_LP_PDCURSES) += build/libpayload/lib/libform.a
classes-$(CONFIG_LP_PDCURSES) +=  build/libpayload/lib/libpanel.a
classes-$(CONFIG_LP_CBFS) += build/libpayload/lib/libcbfs.a
classes-$(CONFIG_LP_LZMA) += build/libpayload/lib/liblzma.a
classes-$(CONFIG_LP_LIBC) += build/libpayload/lib/libc.a
headdoto= build/libpayload/lib/$(ARCHDIR-y)/head.o
libraries := $(classes-y)

# If architecture folder name is different from GCC binutils architecture name,
# override here.
ARCH-$(CONFIG_LP_ARCH_ARM)     := arm
ARCH-$(CONFIG_LP_ARCH_X86)     := i386

LPCC := $(CC_$(ARCH-y))
LPAS := $(AS_$(ARCH-y))
LPLD := $(LD_$(ARCH-y))
LPNM := $(NM_$(ARCH-y))
OBJCOPY := $(OBJCOPY_$(ARCH-y))
OBJDUMP := $(OBJDUMP_$(ARCH-y))
READELF := $(READELF_$(ARCH-y))
STRIP := $(STRIP_$(ARCH-y))
AR := $(AR_$(ARCH-y))

CFLAGS_arm= -mfloat-abi=softfp -marm -mabi=aapcs
CFLAGS_arm += -march=armv7-a -Os -fno-builtin
CFLAGS_arm += -ffreestanding -fomit-frame-pointer

LIBGCC_FILE_NAME := $(shell test -r `$(LPCC) -print-libgcc-file-name` && \
		      $(LPCC) -print-libgcc-file-name)

OBJECTS-y=main.o command.o cmd_help.o cmd_license.o

OBJECTS-y += $(libraries) $(LIBGCC_FILE_NAME)

CFLAGS= -Wall -Werror -Os $(FFLAGS-y) -Ibuild/libpayload/include -I.
CFLAGS+= -Ibuild/libpayload/include/$(ARCHDIR-y)
CFLAGS+= $(CFLAGS_$(ARCH-y))

LDFLAGS=-Wl,-T,ubootcli.ldscript.$(ARCH-y) -static -nostartfiles -nostdlib
LIBGCC=$(shell $(CC) -m32 -print-libgcc-file-name)


ubootcli.elf: $(OBJECTS-y)
	$(LPCC) $(LDFLAGS) -o $@ $(headdoto) $(OBJECTS-y)  $(libraries)
	@$(STRIP) $@

%.o: %.c
	$(LPCC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o ubootcli.elf

