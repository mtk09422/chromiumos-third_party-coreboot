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

export src := $(shell pwd)
export obj := $(src)/build

LIBPAYLOAD_DIR ?= $(src)/../libpayload/install/libpayload

include $(LIBPAYLOAD_DIR)/libpayload.config

$(if $(wildcard .xcompile),,$(eval $(shell bash ../libpayload/util/xcompile/xcompile > .xcompile)))
include .xcompile

ARCHDIR-$(CONFIG_LP_ARCH_ARM)     := arm
ARCHDIR-$(CONFIG_LP_ARCH_X86)     := x86

ARCH-y := $(ARCHDIR-y)

classes-y += $(LIBPAYLOAD_DIR)/lib/libpayload.a
headdoto= $(LIBPAYLOAD_DIR)/lib/$(ARCHDIR-y)/head.o
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

CFLAGS_arm = -mthumb -march=armv7-a

LIBGCC_FILE_NAME := $(shell test -r `$(LPCC) -print-libgcc-file-name` && \
		      $(LPCC) -print-libgcc-file-name)

OBJECTS-y := main.o command.o cmd_help.o cmd_license.o
OBJECTS-y += $(libraries) $(LIBGCC_FILE_NAME)

# Common build params from Depthcharge
ABI_FLAGS := $(ARCH_ABI_FLAGS) -ffreestanding -fno-builtin \
		-fno-stack-protector -fomit-frame-pointer

CFLAGS= -Wall -Werror -g -Os $(FFLAGS-y) -I$(LIBPAYLOAD_DIR)/include -I.
CFLAGS+= -I$(LIBPAYLOAD_DIR)/include/$(ARCHDIR-y)
CFLAGS+= $(CFLAGS_$(ARCH-y)) $(ABI_FLAGS)
CFLAGS+= -fdata-sections -ffunction-sections -std=gnu99

LDFLAGS=-Wl,-T,ubootcli.ldscript.$(ARCH-y) -static -nostartfiles -nostdlib
LDFLAGS+=$(ARCH_LINK_FLAGS) $(ABI_FLAGS) -fuse-ld=bfd
LDFLAGS+=-Wl,--gc-sections -Wl,-Map=$@.map
LIBGCC=$(shell $(CC) -m32 -print-libgcc-file-name)


ubootcli.elf: $(OBJECTS-y)
	$(LPCC) $(LDFLAGS) -o $@ $(headdoto) $(OBJECTS-y)  $(libraries)
	@$(STRIP) $@

%.o: %.c
	$(LPCC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o ubootcli.elf

