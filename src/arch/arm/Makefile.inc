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
bootblock-y += stages.c
bootblock-y += eabi_compat.c
bootblock-y += memset.S
bootblock-y += memcpy.S
bootblock-y += memmove.S

bootblock-y += bootblock.ld

$(objcbfs)/bootblock.debug: $$(bootblock-objs)
	@printf "    LINK       $(subst $(obj)/,,$(@))\n"
	$(call link,bootblock,$(filter %.a %.o,$(^)),-o $(@) -L$(obj) -T $(obj)/arch/arm/bootblock.bootblock.ld,--gc-sections)

endif # CONFIG_ARCH_BOOTBLOCK_ARM

###############################################################################
# verification stage
###############################################################################

$(objcbfs)/verstage.debug: $$(verstage-objs) $$(VB2_LIB)
	@printf "    LINK       $(subst $(obj)/,,$(@))\n"
	$(call link,verstage,$(filter %.a %.o,$(^)),-o $(@) -L$(obj) -T $(obj)/arch/arm/verstage.verstage.ld,--gc-sections)

verstage-$(CONFIG_EARLY_CONSOLE) += early_console.c
verstage-y += div0.c
verstage-y += eabi_compat.c
verstage-y += memset.S
verstage-y += memcpy.S
verstage-y += memmove.S
verstage-y += stages.c

verstage-y += verstage.ld

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

rmodules_arm-y += memset.S
rmodules_arm-y += memcpy.S
rmodules_arm-y += memmove.S
rmodules_arm-y += eabi_compat.c

romstage-y += romstage.ld

VBOOT_STUB_DEPS += $(obj)/arch/arm/eabi_compat.rmodules_arm.o

$(objcbfs)/romstage.debug: $$(romstage-objs)
	@printf "    LINK       $(subst $(obj)/,,$(@))\n"
	$(call link,romstage,$(filter %.a %.o,$(^)),-o $(@) -L$(obj) -T $(obj)/arch/arm/romstage.romstage.ld,--gc-sections)

endif # CONFIG_ARCH_ROMSTAGE_ARM

###############################################################################
# ramstage
###############################################################################

ifeq ($(CONFIG_ARCH_RAMSTAGE_ARM),y)

ramstage-y += stages.c
ramstage-y += div0.c
ramstage-$(CONFIG_COOP_MULTITASKING) += cpu.c
ramstage-y += eabi_compat.c
ramstage-y += boot.c
ramstage-y += tables.c
ramstage-y += memset.S
ramstage-y += memcpy.S
ramstage-y += memmove.S
ramstage-y += clock.c

ramstage-y += ramstage.ld

rmodules_arm-y += memset.S
rmodules_arm-y += memcpy.S
rmodules_arm-y += memmove.S
rmodules_arm-y += eabi_compat.c

ramstage-$(CONFIG_COLLECT_TIMESTAMPS) += timestamp.c

ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/mainboard.c

$(objcbfs)/ramstage.debug: $$(ramstage-objs)
	@printf "    CC         $(subst $(obj)/,,$(@))\n"
	$(call link,ramstage,$(filter %.a %.o,$(^)),-o $(@) -L$(obj) -T $(obj)/arch/arm/ramstage.ramstage.ld,--gc-sections)

endif # CONFIG_ARCH_RAMSTAGE_ARM
