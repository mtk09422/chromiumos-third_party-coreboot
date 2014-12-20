##
## This file is part of the coreboot project.
##
## Copyright (C) 2014 Google Inc
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; version 2 of the License.
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

#######################################################################

ARCH_SUPPORTED := ARM ARM64 X86_32 MIPS

ARCH_TO_TOOLCHAIN_X86_32 := x86_32
ARCH_TO_TOOLCHAIN_ARM    := arm
ARCH_TO_TOOLCHAIN_ARM64  := arm64
ARCH_TO_TOOLCHAIN_MIPS   := mipsel

COREBOOT_STANDARD_STAGES := bootblock verstage romstage ramstage

ARCHDIR-i386      := x86
ARCHDIR-arm       := arm
ARCHDIR-arm64     := arm64
ARCHDIR-x86_32    := x86
ARCHDIR-mipsel    := mips

# About -Wstack-usage: if you arrived here via grep, you were probably trying to
# do something naughty that could've caused boards with 2K stack size to have a
# very bad time somewhere down the road. Since stack overflows are both very
# dangerous and almost impossible to prevent, we're drawing the line at 1.5K
# for a single function frame (with the assumption that you hopefully don't
# chain more than one of that size together). Buffers larger than that should
# be allocated in the BSS (use MAYBE_STATIC if you need to share code with
# __PRE_RAM__ x86). Since GCCs detection of dynamic array bounds unfortunately
# seems to be very basic, you'll sometimes have to use a static upper bound
# for the size and an assert() to make sure it's honored (see gpio_base3_value()
# for an example). (If you absolutely need a larger stack frame and are 100%
# sure it cannot cause problems, you can whitelist it with #pragma diagnostic.)

CFLAGS_nonx86     := -ffunction-sections -fdata-sections -Wstack-usage=1536

CFLAGS_arm        := $(CFLAGS_nonx86) -mno-unaligned-access
CFLAGS_arm64      := $(CFLAGS_nonx86)
CFLAGS_mipsel     := $(CFLAGS_nonx86) -mips32r2 -G 0

toolchain_to_dir = \
	$(foreach arch,$(ARCH_SUPPORTED),\
	$(eval INCLUDES_$(ARCH_TO_TOOLCHAIN_$(arch)) = \
	-Isrc/arch/$(ARCHDIR-$(ARCH_TO_TOOLCHAIN_$(arch)))/include))

# set_stage_toolchain: Decides the toolchain to be used by every stage
# E.g.: If bootblock is x86_32, it sets ARCH-BOOTBLOCK-y = x86_32, whereas
# ARCH-BOOTBLOCK-n = armv7. Then, ARCH-BOOTBLOCK-y can be used anywhere to
# decide the compiler toolchain for bootblock stage
# This step is essential for initializing the toolchain for coreboot standard
# stages i.e. bootblock, romstage and ramstage, since it acts as the second
# parameter to create_class_compiler below in init_standard_toolchain
set_stage_toolchain= \
	$(foreach arch,$(ARCH_SUPPORTED),$(eval ARCH-$(1)-$(CONFIG_ARCH_$(1)_$(arch)) := $(ARCH_TO_TOOLCHAIN_$(arch))))


# create_class_compiler: Used to create compiler tool set for
# special classes
# @1: special class
# @2: compiler set to be used
# e.g.: smm special class uses i386 as compiler set
define create_class_compiler
CC_$(1) := $(CC_$(2))
LD_$(1) := $(LD_$(2))
NM_$(1) := $(NM_$(2))
AR_$(1) := $(AR_$(2))
OBJCOPY_$(1) := $(OBJCOPY_$(2))
OBJDUMP_$(1) := $(OBJDUMP_$(2))
STRIP_$(1) := $(STRIP_$(2))
READELF_$(1) := $(READELF_$(2))
INCLUDES_$(1) = $$(INCLUDES) -Isrc/arch/$(ARCHDIR-$(2))/include
CFLAGS_$(1) = $$(CFLAGS_common) $$(INCLUDES_$(1)) $(CFLAGS_$(2))
LIBGCC_FILE_NAME_$(1) = $(shell test -n "$(CC_$(2))" && \
		      test -r `$(CC_$(2)) -print-libgcc-file-name` && \
		      $(CC_$(2)) -print-libgcc-file-name)
endef

# define_class: Allows defining any program as dynamic class and compiler tool
# set for the same based on the architecture for which the program is to be
# compiled
# @1: program (class name)
# @2: architecture for which the program needs to be compiled
# IMP: Ensure that define_class is called before any .c or .S files are added to
# the class of the program. Check subdirs-y for order of subdirectory inclusions
define define_class
classes-y += $(1)
$(eval $(call create_class_compiler,$(1),$(2)))
endef

# initialize standard toolchain (CC,AS and others) for given stage
# @1 : stage for which the toolchain is to be initialized
init_standard_toolchain = \
	$(eval stage_caps := $(shell printf "%s" $(1) | tr '[:lower:]' '[:upper:]' )) \
	$(eval $(call set_stage_toolchain,$(stage_caps))) \
	$(eval $(call create_class_compiler,$(1),$(ARCH-$(stage_caps)-y))) \
	$(eval $(call set_stage_libgcc,$(1)))

init_stages = \
	$(foreach stage,$(COREBOOT_STANDARD_STAGES),$(eval $(call init_standard_toolchain,$(stage))))

# This mapping is created to have consistency with xcompile naming
$(eval $(call create_class_compiler,x86_32,i386))
$(eval $(call create_class_compiler,arm64,aarch64))

$(eval $(call toolchain_to_dir))

$(eval $(call init_stages))
