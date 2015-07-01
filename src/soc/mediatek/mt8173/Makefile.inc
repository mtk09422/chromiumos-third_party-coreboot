##
## This file is part of the coreboot project.
##
## Copyright 2015 MediaTek Inc.
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
bootblock-y += cbfs.c
bootblock-y += pll.c mtcmos.c
bootblock-y += spi.c
bootblock-y += timer.c

ifeq ($(CONFIG_BOOTBLOCK_CONSOLE),y)
bootblock-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c
endif

bootblock-y += gpio.c gpio_init.c pmic_wrap_init.c pmic.c i2c.c da9212.c
bootblock-y += wdt.c
bootblock-y += mmu_operations.c

################################################################################

verstage-y += verstage.c
verstage-y += i2c.c
verstage-y += spi.c

verstage-$(CONFIG_CONSOLE_SERIAL) += uart.c

verstage-y += timer.c cpu_lib.S
verstage-y += wdt.c
verstage-y += cbfs.c
verstage-y += mmu_operations.c

################################################################################

romstage-y += cpu_lib.S
romstage-y += cbfs.c
romstage-y += timer.c

ifeq ($(CONFIG_EARLY_CONSOLE),y)
romstage-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c
endif
romstage-$(CONFIG_DYNAMIC_CBMEM) += cbmem.c
romstage-y += spi.c
romstage-y += gpio.c gpio_init.c
romstage-y += pmic_wrap_init.c i2c.c pmic.c da9212.c
romstage-y += addressmap.c
romstage-y += mmu_operations.c

################################################################################

ramstage-y += cpu_lib.S
ramstage-y += cbmem.c
ramstage-y += spi.c
ramstage-y += cbfs.c
ramstage-y += soc.c
ramstage-y += timer.c
ramstage-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c
ramstage-y += pmic_wrap_init.c
ramstage-y += ramstage.c mmu_operations.c
ramstage-y += addressmap.c
ramstage-y += gpio.c gpio_init.c
ramstage-y += wdt.c

################################################################################

TYPE=sf

# Generate the actual coreboot bootblock code
$(objcbfs)/bootblock.bin: $(objcbfs)/bootblock.elf
	@printf "    OBJCOPY    $(subst $(obj)/,,$(@))\n"
	$(OBJCOPY_bootblock) -O binary $< $@.tmp
	./util/mtkheader/gen-bl-img.py mt8173 $(TYPE) $@.tmp $@

INCLUDES += -Isrc/soc/mediatek/mt8173/include
