################################################################################
##
## This file is part of the coreboot project.
##
## Copyright (C) 2014 Google Inc.
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
################################################################################

################################################################################
# Take care of subdirectories
################################################################################

subdirs-y += armv8/ cpu/

################################################################################
# ARM specific options
################################################################################

ifeq ($(CONFIG_ARCH_ROMSTAGE_ARM64),y)
CBFSTOOL_PRE1_OPTS = -m arm64 -s $(CONFIG_CBFS_SIZE)
endif

################################################################################
# bootblock
################################################################################

ifeq ($(CONFIG_ARCH_BOOTBLOCK_ARM64),y)

bootblock-y += div0.c
bootblock-$(CONFIG_BOOTBLOCK_CONSOLE) += early_console.c

bootblock-y += id.S
bootblock-y += c_entry.c
bootblock-y += stage_entry.S
bootblock-y += cpu-stubs.c
bootblock-y += stages.c
bootblock-y += eabi_compat.c
bootblock-y += transition.c transition_asm.S
bootblock-$(CONFIG_COLLECT_TIMESTAMPS) += timestamp.c

bootblock-y += ../../lib/memset.c
bootblock-y += ../../lib/memcpy.c
bootblock-y += ../../lib/memmove.c

# Build the bootblock

$(objcbfs)/bootblock.debug: $$(bootblock-objs)
	@printf "    LINK       $(subst $(obj)/,,$(@))\n"
	$(call link,bootblock,$(filter %.a %.o,$(^)),-o $(@) -L$(obj) -T $(obj)/mainboard/$(MAINBOARDDIR)/memlayout.bootblock.ld,--gc-sections)

endif # CONFIG_ARCH_BOOTBLOCK_ARM64

###############################################################################
# verification stage
###############################################################################

ifeq ($(CONFIG_ARCH_VERSTAGE_ARM64),y)

$(objcbfs)/verstage.debug: $$(verstage-objs) $$(VB2_LIB)
	@printf "    LINK       $(subst $(obj)/,,$(@))\n"
	$(call link,verstage,$(filter %.a %.o,$(^)),-o $(@) -L$(obj) -T $(obj)/mainboard/$(MAINBOARDDIR)/memlayout.verstage.ld,--gc-sections)

verstage-$(CONFIG_EARLY_CONSOLE) += early_console.c
verstage-y += div0.c
verstage-y += eabi_compat.c
verstage-y += ../../lib/memset.c
verstage-y += ../../lib/memcpy.c
verstage-y += ../../lib/memmove.c
verstage-y += stages.c
verstage-$(CONFIG_COLLECT_TIMESTAMPS) += timestamp.c

endif # CONFIG_ARCH_VERSTAGE_ARM64

################################################################################
# romstage
################################################################################

ifeq ($(CONFIG_ARCH_ROMSTAGE_ARM64),y)

romstage-y += c_entry.c
romstage-y += stage_entry.S
romstage-y += cpu-stubs.c
romstage-y += stages.c
romstage-y += div0.c
romstage-$(CONFIG_EARLY_CONSOLE) += early_console.c
romstage-y += eabi_compat.c
romstage-y += ../../lib/memset.c
romstage-y += ../../lib/memcpy.c
romstage-y += ../../lib/memmove.c
romstage-$(CONFIG_COLLECT_TIMESTAMPS) += timestamp.c
romstage-y += transition.c transition_asm.S

rmodules_arm64-y += ../../lib/memset.c
rmodules_arm64-y += ../../lib/memcpy.c
rmodules_arm64-y += ../../lib/memmove.c
rmodules_arm64-y += eabi_compat.c

VBOOT_STUB_DEPS += $(obj)/arch/arm64/eabi_compat.rmodules_arm64.o

# Build the romstage

$(objcbfs)/romstage.debug: $$(romstage-objs)
	@printf "    LINK       $(subst $(obj)/,,$(@))\n"
	$(call link,romstage,$(filter %.a %.o,$(^)),-o $(@) -L$(obj) -T $(obj)/mainboard/$(MAINBOARDDIR)/memlayout.romstage.ld,--gc-sections)

endif # CONFIG_ARCH_ROMSTAGE_ARM64

################################################################################
# ramstage
################################################################################

ifeq ($(CONFIG_ARCH_RAMSTAGE_ARM64),y)

ramstage-y += c_entry.c
ramstage-y += stages.c
ramstage-y += startup.c
ramstage-y += div0.c
ramstage-y += cpu.c
ramstage-y += cpu_ramstage.c
ramstage-y += eabi_compat.c
ramstage-y += boot.c
ramstage-y += tables.c
ramstage-y += ../../lib/memset.c
ramstage-y += ../../lib/memcpy.c
ramstage-y += ../../lib/memmove.c
ramstage-y += stage_entry.S
ramstage-y += cpu-stubs.c
ramstage-$(CONFIG_ARCH_SPINTABLE) += spintable.c spintable_asm.S
ramstage-y += transition.c transition_asm.S

rmodules_arm64-y += ../../lib/memset.c
rmodules_arm64-y += ../../lib/memcpy.c
rmodules_arm64-y += ../../lib/memmove.c
rmodules_arm64-y += eabi_compat.c

secmon-$(CONFIG_ARCH_USE_SECURE_MONITOR) += stage_entry.S
secmon-$(CONFIG_ARCH_USE_SECURE_MONITOR) += cpu-stubs.c
secmon-$(CONFIG_ARCH_USE_SECURE_MONITOR) += startup.c
secmon-$(CONFIG_ARCH_USE_SECURE_MONITOR) += ../../lib/malloc.c
secmon-$(CONFIG_ARCH_USE_SECURE_MONITOR) += ../../lib/memset.c
secmon-$(CONFIG_ARCH_USE_SECURE_MONITOR) += ../../lib/memcmp.c
secmon-$(CONFIG_ARCH_USE_SECURE_MONITOR) += ../../lib/memcpy.c

ramstage-$(CONFIG_COLLECT_TIMESTAMPS) += timestamp.c

ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/mainboard.c

# Build the ramstage

$(objcbfs)/ramstage.debug: $$(ramstage-objs)
	@printf "    CC         $(subst $(obj)/,,$(@))\n"
	$(call link,ramstage,$(filter %.a %.o,$(^)),-o $(@) -L$(obj) -T $(obj)/mainboard/$(MAINBOARDDIR)/memlayout.ramstage.ld,--gc-sections)

endif # CONFIG_ARCH_RAMSTAGE_ARM64
