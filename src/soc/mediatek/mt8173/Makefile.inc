##
## This file is part of the coreboot project.
##
## Copyright 2014 Google Inc.
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

bootblock-y += bootblock.c
bootblock-y += cpu_lib.S
bootblock-y += emmc_cbfs.c
bootblock-y += timer.c monotonic_timer.c
ifeq ($(CONFIG_BOOTBLOCK_CONSOLE),y)
bootblock-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c
endif

################################################################################

romstage-y += cpu_lib.S
romstage-y += emmc_cbfs.c
romstage-y += timer.c monotonic_timer.c
ifeq ($(CONFIG_EARLY_CONSOLE),y)
romstage-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c
endif
romstage-$(CONFIG_DYNAMIC_CBMEM) += cbmem.c

################################################################################

ramstage-y += cpu_lib.S
ramstage-y += cbmem.c
ramstage-y += emmc_cbfs.c
ramstage-y += soc.c
ramstage-y += timer.c monotonic_timer.c
ramstage-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c
ramstage-y += ramstage.c mmu_operations.c
ramstage-y += gic.c

rmodules_$(ARCH-ROMSTAGE-y)-y += monotonic_timer.c
# TODO: remove this when vbootstub 64-bit align problem has been fixed.
rmodules_$(ARCH-ROMSTAGE-y)-c-ccopts += -mstrict-align
VBOOT_STUB_DEPS += $(obj)/soc/mediatek/mt8173/monotonic_timer.rmodules_$(ARCH-ROMSTAGE-y).o

INCLUDES += -Isrc/soc/mediatek/mt8173/include
