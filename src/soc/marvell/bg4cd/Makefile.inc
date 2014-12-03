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
bootblock-y += cbmem.c
bootblock-y += i2c.c
bootblock-y += monotonic_timer.c
bootblock-$(CONFIG_SPI_FLASH) += spi.c
bootblock-$(CONFIG_CONSOLE_SERIAL) += uart.c

verstage-y += i2c.c
verstage-y += monotonic_timer.c
verstage-$(CONFIG_SPI_FLASH) += spi.c
verstage-$(CONFIG_CONSOLE_SERIAL) += uart.c

romstage-y += cbmem.c
romstage-y += i2c.c
romstage-y += monotonic_timer.c
romstage-y += sdram.c
romstage-$(CONFIG_SPI_FLASH) += spi.c
romstage-$(CONFIG_CONSOLE_SERIAL) += uart.c

ramstage-y += cbmem.c
ramstage-y += i2c.c
ramstage-y += monotonic_timer.c
ramstage-$(CONFIG_SPI_FLASH) += spi.c
ramstage-$(CONFIG_CONSOLE_SERIAL) += uart.c

INCLUDES += -Isrc/soc/marvell/bg4cd/include/

# bootblock and verstage are built in one piece.
postprocessors += $$(eval bootblock-objs += $$(call src-to-obj,bootblock,$$(verstage-srcs)))
postprocessors += $$(eval bootblock-objs += $$(VB2_LIB))
postprocessors += $$(eval bootblock-srcs += $$(verstage-srcs))
postprocessors += $$(eval bootblock-srcs:=$$(sort $$(bootblock-srcs)))
# we need to avoid building verstage.elf
postprocessors += $$(eval cbfs-files:=$$(filter-out $$(VERSTAGE_ELF)|%, $$(cbfs-files)))

$(objcbfs)/bootblock.raw: $(objcbfs)/bootblock.elf
	@printf "    OBJCOPY    $(subst $(obj)/,,$(@))\n"
	$(OBJCOPY_bootblock) -O binary $< $@.tmp
	@mv $@.tmp $@

$(objcbfs)/bootblock.bin: $(objcbfs)/bootblock.raw
	@printf "Generating: $(subst $(obj)/,,$(@))\n"
	@cp $< $@
