################################################################################
##
## This file is part of the coreboot project.
##
## Copyright (C) 2013 The ChromiumOS Authors
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

armv4_flags = -marm -march=armv4t -I$(src)/arch/arm/include/armv4/ \
	-D__COREBOOT_ARM_ARCH__=4

################################################################################
## Bootblock
################################################################################
ifeq ($(CONFIG_ARM_BOOTBLOCK_ARMV4),y)

ifneq ($(CONFIG_ARM_BOOTBLOCK_CUSTOM),y)
bootblock-y += bootblock.S
endif
bootblock-$(CONFIG_ARM_BOOTBLOCK_SIMPLE) += bootblock_simple.c
bootblock-$(CONFIG_ARM_BOOTBLOCK_NORMAL) += bootblock_normal.c
bootblock-y += cache.c

bootblock-c-ccopts += $(armv4_flags)
bootblock-S-ccopts += $(armv4_flags)

endif
################################################################################
## ROM stage
################################################################################
ifeq ($(CONFIG_ARM_ROMSTAGE_ARMV4),y)

romstage-c-ccopts += $(armv4_flags)
romstage-S-ccopts += $(armv4_flags)

endif
################################################################################
## RAM stage
################################################################################
ifeq ($(CONFIG_ARM_RAMSTAGE_ARMV4),y)

ramstage-c-ccopts += $(armv4_flags)
ramstage-S-ccopts += $(armv4_flags)

endif
