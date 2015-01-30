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
bootblock-y += bootblock_asm.S
bootblock-y += emmc_cbfs.c
##	bootblock-$(CONFIG_SPI_FLASH) += spi.c
bootblock-y += timer.c monotonic_timer.c
bootblock-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c

################################################################################

romstage-y += emmc_cbfs.c
##	romstage-$(CONFIG_SPI_FLASH) += spi.c
romstage-y += timer.c monotonic_timer.c
romstage-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c
romstage-$(CONFIG_DYNAMIC_CBMEM) += cbmem.c

romstage-y += 	wdt.c

################################################################################

ramstage-y += cbmem.c
ramstage-y += emmc_cbfs.c
ramstage-y += soc.c
##	ramstage-$(CONFIG_SPI_FLASH) += spi.c
ramstage-y += timer.c monotonic_timer.c
ramstage-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c
##	ramstage-y += usb.c

rmodules_$(ARCH-ROMSTAGE-y)-y += monotonic_timer.c
VBOOT_STUB_DEPS += $(obj)/soc/mediatek/mt8135/monotonic_timer.rmodules_$(ARCH-ROMSTAGE-y).o

INCLUDES += -Isrc/soc/mediatek/mt8135/include
