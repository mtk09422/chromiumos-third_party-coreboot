#
# This file is part of the coreboot project.
#
# Copyright (C) 2009 coresystems GmbH
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
#

bootblock-y += cbfs.c
bootblock-$(CONFIG_COMMON_CBFS_SPI_WRAPPER) += cbfs_spi.c
bootblock-y += memchr.c
bootblock-y += memcmp.c
ifeq ($(CONFIG_ARCH_BOOTBLOCK_ARM_V7_M),y)
bootblock-y += memset.c memcpy.c memmove.c
endif
bootblock-$(CONFIG_COLLECT_TIMESTAMPS) += timestamp.c
bootblock-$(CONFIG_GENERIC_UDELAY) += timer.c

ifeq ($(CONFIG_BOOTBLOCK_CONSOLE),y)
bootblock-$(CONFIG_CONSOLE_SERIAL8250) += uart8250.c
bootblock-$(CONFIG_CONSOLE_SERIAL8250MEM) += uart8250mem.c
bootblock-$(CONFIG_CONSOLE_CBMEM) += cbmem_console.c
bootblock-$(CONFIG_CONSOLE_NE2K) += ne2k.c
endif

romstage-y += memchr.c
romstage-y += memcmp.c

$(foreach arch,$(ARCH_SUPPORTED),\
	  $(eval rmodules_$(ARCH_TO_TOOLCHAIN_$(arch))-y += memcmp.c) \
	  $(eval rmodules_$(ARCH_TO_TOOLCHAIN_$(arch))-y += rmodule.ld))

verstage-y += delay.c
verstage-y += cbfs.c
verstage-y += memcmp.c
ifeq ($(CONFIG_ARCH_VERSTAGE_ARM_V7_M),y)
verstage-y += memset.c memcpy.c memmove.c
endif
verstage-$(CONFIG_CONSOLE_CBMEM) += cbmem_console.c
verstage-$(CONFIG_COMMON_CBFS_SPI_WRAPPER) += cbfs_spi.c
verstage-y += tlcl.c
verstage-$(CONFIG_GENERIC_UDELAY) += timer.c
verstage-$(CONFIG_COLLECT_TIMESTAMPS) += timestamp.c

romstage-y += delay.c
romstage-y += cbfs.c
romstage-$(CONFIG_COMMON_CBFS_SPI_WRAPPER) += cbfs_spi.c
romstage-y += lzma.c
#romstage-y += lzmadecode.c
romstage-$(CONFIG_PRIMITIVE_MEMTEST) += primitive_memtest.c
ramstage-$(CONFIG_PRIMITIVE_MEMTEST) += primitive_memtest.c
romstage-$(CONFIG_CACHE_AS_RAM) += ramtest.c
romstage-$(CONFIG_CONSOLE_SERIAL8250) += uart8250.c
romstage-$(CONFIG_CONSOLE_SERIAL8250MEM) += uart8250mem.c
romstage-$(CONFIG_CONSOLE_CBMEM) += cbmem_console.c
romstage-$(CONFIG_CONSOLE_NE2K) += ne2k.c
romstage-$(CONFIG_USBDEBUG) += usbdebug.c
romstage-$(CONFIG_COLLECT_TIMESTAMPS) += timestamp.c
romstage-$(CONFIG_EARLY_CBMEM_INIT) += cbmem.c
romstage-y += compute_ip_checksum.c
romstage-$(CONFIG_ARCH_ROMSTAGE_X86_32) += gcc.c
romstage-$(CONFIG_GENERIC_UDELAY) += timer.c

ramstage-y += hardwaremain.c
ramstage-y += selfboot.c
ramstage-y += coreboot_table.c
ramstage-y += memchr.c
ramstage-y += memcmp.c
ramstage-y += malloc.c
smm-$(CONFIG_SMM_TSEG) += malloc.c
ramstage-y += delay.c
ramstage-y += fallback_boot.c
ramstage-y += compute_ip_checksum.c
ramstage-y += version.c
ramstage-y += cbfs.c
ramstage-$(CONFIG_COMMON_CBFS_SPI_WRAPPER) += cbfs_spi.c
ramstage-y += lzma.c
#ramstage-y += lzmadecode.c
ramstage-y += stack.c
ramstage-$(CONFIG_ARCH_RAMSTAGE_X86_32) += gcc.c
ramstage-y += clog2.c
ramstage-$(CONFIG_CONSOLE_SERIAL8250) += uart8250.c
ramstage-$(CONFIG_CONSOLE_SERIAL8250MEM) += uart8250mem.c
ramstage-$(CONFIG_CONSOLE_CBMEM) += cbmem_console.c
ramstage-$(CONFIG_USBDEBUG) += usbdebug.c
ramstage-$(CONFIG_BOOTSPLASH) += jpeg.c
ramstage-$(CONFIG_TRACE) += trace.c
ramstage-$(CONFIG_COLLECT_TIMESTAMPS) += timestamp.c
ramstage-$(CONFIG_COVERAGE) += libgcov.c
ramstage-$(CONFIG_MAINBOARD_DO_NATIVE_VGA_INIT) += edid.c
ramstage-y += memrange.c
ramstage-$(CONFIG_COOP_MULTITASKING) += thread.c
ramstage-$(CONFIG_TIMER_QUEUE) += timer_queue.c
ramstage-$(CONFIG_TERTIARY_BOARD_ID) += gpio.c
ramstage-$(CONFIG_GENERIC_UDELAY) += timer.c

# The CBMEM implementations are chosen based on CONFIG_DYNAMIC_CBMEM.
ifeq ($(CONFIG_DYNAMIC_CBMEM),y)
ramstage-y += dynamic_cbmem.c
romstage-y += dynamic_cbmem.c
else
ramstage-y += cbmem.c
romstage-$(CONFIG_HAVE_ACPI_RESUME) += cbmem.c
endif # CONFIG_DYNAMIC_CBMEM
ramstage-y += cbmem_info.c

ramstage-y += hexdump.c
romstage-y += hexdump.c

ramstage-$(CONFIG_REG_SCRIPT) += reg_script.c
romstage-$(CONFIG_REG_SCRIPT) += reg_script.c

ramstage-$(CONFIG_CONSOLE_NE2K) += ne2k.c

romstage-$(CONFIG_CACHE_RELOCATED_RAMSTAGE_OUTSIDE_CBMEM) += ramstage_cache.c

ifneq ($(CONFIG_ARCH_X86),y)
# X86 bootblock and romstage use custom ldscripts that are all glued together,
# so we need to exclude it here or it would pick these up as well
bootblock-y += bootblock.ld
romstage-y += romstage.ld
endif
ramstage-y += ramstage.ld

smm-y += cbfs.c memcmp.c
smm-$(CONFIG_CONSOLE_SERIAL8250) += uart8250.c
smm-$(CONFIG_CONSOLE_SERIAL8250MEM) += uart8250mem.c
smm-$(CONFIG_USBDEBUG) += usbdebug.c
smm-y += gcc.c

ifeq ($(CONFIG_RELOCATABLE_MODULES),y)
ramstage-y += rmodule.c
# Include rmodule.c in romstage if vboot verification is selected.
romstage-$(CONFIG_VBOOT_VERIFY_FIRMWARE) += rmodule.c

# rmodule_link_rules is a function that should be called with:
# (1) the object name to link
# (2) the dependencies
# (3) heap size of the relocatable module
# (4) arch for which the rmodules are to be linked
# It will create the necessary Make rules to create a rmodule. The resulting
# rmdoule is named $(1).rmod
define rmodule_link
$(strip $(1)): $(strip $(2)) $$(LIBGCC_FILE_NAME_rmodules_$(4)) $(obj)/lib/rmodule.rmodules_$(4).ld $$(RMODTOOL)
	$$(call link,rmodules_$(4),$$(filter %.a %.o,$$(^)),-o $$(@) -T $(obj)/lib/rmodule.rmodules_$(4).ld,--emit-relocs -z defs -Bsymbolic --defsym=__heap_size=$(strip $(3)))
	$$(NM_rmodules_$(4)) -n $$@ > $$(basename $$@).map

$(strip $(1)).rmod: $(strip $(1))
	$$(RMODTOOL) -i $$^ -o $$@

endef

endif

