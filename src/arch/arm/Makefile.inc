################################################################################
##
## This file is part of the coreboot project.
##
## Copyright (C) 2012-2013 The ChromiumOS Authors
## Copyright (C) 2012 Alexandru Gagniuc <mr.nuke.me@gmail.com>
## Copyright (C) 2009-2010 coresystems GmbH
## Copyright (C) 2009 Ronald G. Minnich
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
###############################################################################
# Take care of subdirectories
###############################################################################
subdirs-y += boot/
subdirs-y += lib/
subdirs-y += libgcc/
subdirs-y += armv4/ armv7/

###############################################################################
# ARM specific options
###############################################################################

ifeq ($(CONFIG_ARCH_ROMSTAGE_ARM),y)
CBFSTOOL_PRE1_OPTS = -m arm -b $(CONFIG_BOOTBLOCK_ROM_OFFSET) -H $(CONFIG_CBFS_HEADER_ROM_OFFSET) -o $(CONFIG_CBFS_ROM_OFFSET)
endif

###############################################################################
# bootblock
###############################################################################

ifeq ($(CONFIG_ARCH_BOOTBLOCK_ARM),y)

bootblock-y += div0.c
bootblock-$(CONFIG_BOOTBLOCK_CONSOLE) += early_console.c
bootblock-y += id.S
bootblock-y += clock.c

$(obj)/arch/arm/id.bootblock.o: $(obj)/build.h

bootblock-y += stages.c
bootblock-y += eabi_compat.c
bootblock-y += memset.S
bootblock-y += memcpy.S
bootblock-y += memmove.S

$(objcbfs)/bootblock.debug: $(src)/arch/arm/bootblock.ld $(obj)/ldoptions $$(bootblock-objs) $$(VERSTAGE_LIB) $$(VB2_LIB) $(obj)/config.h
	@printf "    LINK       $(subst $(obj)/,,$(@))\n"
ifeq ($(CONFIG_COMPILER_LLVM_CLANG),y)
	$(LD_bootblock) -nostdlib -m armelf_linux_eabi --gc-sections -static -o $@ -L$(obj) $< -T $(src)/arch/arm/bootblock.ld
else
	# This is based on the assumption that bootblock and verstage are compatible
	# from the linker's perspective.
	$(CC_bootblock) $(CFLAGS_bootblock) -nostdlib -Wl,--gc-sections -static -o $@ -L$(obj) -T $(src)/arch/arm/bootblock.ld -Wl,--start-group $(bootblock-objs) $(VERSTAGE_LIB) $(VB2_LIB) -Wl,--end-group
endif

endif

###############################################################################
# verification stage
###############################################################################

verstage-$(CONFIG_EARLY_CONSOLE) += early_console.c
verstage-y += div0.c
verstage-y += eabi_compat.c
verstage-y += memset.S
verstage-y += memcpy.S
verstage-y += memmove.S
verstage-y += stages.c

###############################################################################
# romstage
###############################################################################

ifeq ($(CONFIG_ARCH_ROMSTAGE_ARM),y)

romstage-y += stages.c
romstage-y += div0.c
romstage-$(CONFIG_EARLY_CONSOLE) += early_console.c
romstage-y += eabi_compat.c
romstage-y += memset.S
romstage-y += memcpy.S
romstage-y += memmove.S
romstage-$(CONFIG_COLLECT_TIMESTAMPS) += timestamp.c
romstage-y += clock.c

$(objcbfs)/romstage.debug: $$(romstage-objs) $(src)/arch/arm/romstage.ld $(obj)/ldoptions
	@printf "    LINK       $(subst $(obj)/,,$(@))\n"
ifeq ($(CONFIG_COMPILER_LLVM_CLANG),y)
	$(LD_romstage) -nostdlib --gc-sections -static -o $@ -L$(obj) $(romstage-objs) -T $(src)/arch/arm/romstage.ld
else
	$(CC_romstage) $(CFLAGS_romstage) -nostdlib -Wl,--gc-sections -static -o $@ -L$(obj) -T $(src)/arch/arm/romstage.ld -Wl,--start-group $(romstage-objs) -Wl,--end-group
endif

endif

###############################################################################
# ramstage
###############################################################################

ifeq ($(CONFIG_ARCH_RAMSTAGE_ARM),y)

ramstage-y += stages.c
ramstage-y += div0.c
ramstage-y += cpu.c
ramstage-y += eabi_compat.c
ramstage-y += boot.c
ramstage-y += tables.c
ramstage-y += memset.S
ramstage-y += memcpy.S
ramstage-y += memmove.S
ramstage-y += clock.c

rmodules_arm-y += memset.S
rmodules_arm-y += memcpy.S
rmodules_arm-y += memmove.S
rmodules_arm-y += eabi_compat.c

VBOOT_STUB_DEPS += $(obj)/arch/arm/eabi_compat.rmodules_arm.o

ramstage-$(CONFIG_COLLECT_TIMESTAMPS) += timestamp.c

ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/mainboard.c

$(objcbfs)/ramstage.debug: $$(ramstage-objs) $(src)/arch/arm/ramstage.ld $(obj)/ldoptions
	@printf "    CC         $(subst $(obj)/,,$(@))\n"
ifeq ($(CONFIG_COMPILER_LLVM_CLANG),y)
	$(LD_ramstage) -nostdlib -m armelf_linux_eabi --gc-sections -o $@ --wrap __divdi3 --wrap __udivdi3 --wrap __moddi3 --wrap __umoddi3 --wrap __uidiv --start-group $(ramstage-objs) --end-group -T $(src)/arch/arm/ramstage.ld
else
	$(CC_ramstage) $(CFLAGS_ramstage) -nostdlib -Wl,--gc-sections -static -o $@ -L$(obj) -Wl,--start-group $(ramstage-objs) -Wl,--end-group -T $(src)/arch/arm/ramstage.ld
endif

endif

