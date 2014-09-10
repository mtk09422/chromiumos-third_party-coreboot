################################################################################
##
## This file is part of the coreboot project.
##
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
subdirs-y += boot
# subdirs-y += init
subdirs-y += lib
subdirs-y += smp

################################################################################
# i386 specific tools
################################################################################

NVRAMTOOL:=$(objutil)/nvramtool/nvramtool

OPTION_TABLE_H:=
ifeq ($(CONFIG_HAVE_OPTION_TABLE),y)

cbfs-files-y += cmos_layout.bin
cmos_layout.bin-file = $(obj)/cmos_layout.bin
cmos_layout.bin-type = 0x01aa

$(obj)/cmos_layout.bin: $(NVRAMTOOL) $(top)/src/mainboard/$(MAINBOARDDIR)/cmos.layout
	@printf "    OPTION     $(subst $(obj)/,,$(@))\n"
	$(NVRAMTOOL) -y $(top)/src/mainboard/$(MAINBOARDDIR)/cmos.layout -L $@

OPTION_TABLE_H:=$(obj)/option_table.h
generic-deps += $(OPTION_TABLE_H)

$(OPTION_TABLE_H): $(NVRAMTOOL) $(top)/src/mainboard/$(MAINBOARDDIR)/cmos.layout
	@printf "    OPTION     $(subst $(obj)/,,$(@))\n"
	$(NVRAMTOOL) -y $(top)/src/mainboard/$(MAINBOARDDIR)/cmos.layout -H $@

endif # CONFIG_HAVE_OPTION_TABLE

################################################################################
# X86 specific options
################################################################################

ifeq ($(CONFIG_ARCH_ROMSTAGE_X86_32),y)
CBFSTOOL_PRE1_OPTS = -m x86 -o $$(( $(CONFIG_ROM_SIZE) - $(CONFIG_CBFS_SIZE) ))
CBFSTOOL_PRE_OPTS  = -b $(shell cat $(objcbfs)/base_xip.txt)
endif

LIBGCC_WRAP_LDFLAGS := --wrap __divdi3 --wrap __udivdi3 --wrap __moddi3 --wrap __umoddi3

stripped_vgabios_id = $(call strip_quotes,$(CONFIG_VGA_BIOS_ID))
cbfs-files-$(CONFIG_VGA_BIOS) += pci$(stripped_vgabios_id).rom
pci$(stripped_vgabios_id).rom-file := $(call strip_quotes,$(CONFIG_VGA_BIOS_FILE))
pci$(stripped_vgabios_id).rom-type := optionrom

cbfs-files-$(CONFIG_INTEL_MBI) += mbi.bin
mbi.bin-file := $(call strip_quotes,$(CONFIG_MBI_FILE))
mbi.bin-type := mbi

cbfs-files-$(CONFIG_BOOTSPLASH) += bootsplash.jpg
bootsplash.jpg-file := $(call strip_quotes,$(CONFIG_BOOTSPLASH_FILE))
bootsplash.jpg-type := bootsplash

###############################################################################
# bootblock
###############################################################################

ifeq ($(CONFIG_ARCH_BOOTBLOCK_X86_32),y)
bootblock-srcs += $(src)/arch/x86/init/failover.ld
bootblock-srcs += $(src)/cpu/x86/16bit/entry16.ld
bootblock-srcs += $(src)/cpu/x86/16bit/reset16.ld
bootblock-srcs += $(src)/arch/x86/lib/id.ld
ifeq ($(CONFIG_CPU_INTEL_FIRMWARE_INTERFACE_TABLE),y)
bootblock-srcs += $(src)/cpu/intel/fit/fit.ld
endif

# TODO: Why can't this use the real bootblock-y += xxx.S mechanism instead?
bootblock_inc = $(src)/arch/x86/init/prologue.inc
bootblock_inc += $(src)/cpu/x86/16bit/entry16.inc
bootblock_inc += $(src)/cpu/x86/16bit/reset16.inc
bootblock_inc += $(src)/cpu/x86/32bit/entry32.inc
bootblock_inc += $(src)/arch/x86/lib/id.inc
ifeq ($(CONFIG_CPU_INTEL_FIRMWARE_INTERFACE_TABLE),y)
bootblock_inc += $(src)/cpu/intel/fit/fit.inc
endif
bootblock_inc += $(chipset_bootblock_inc)

ifeq ($(CONFIG_SSE),y)
bootblock_inc += $(src)/cpu/x86/sse_enable.inc
endif
bootblock_inc += $(objgenerated)/bootblock.inc
bootblock_inc += $(src)/arch/x86/lib/walkcbfs.S

ifeq ($(CONFIG_SSE),y)
bootblock_romccflags = -mcpu=k7 -msse
else
bootblock_romccflags = -mcpu=i386
endif
bootblock_romccflags += -O2 $(bootblock-c-ccopts) $(bootblock-generic-ccopts)

$(objgenerated)/bootblock.inc: $(src)/arch/x86/init/$(subst ",,$(CONFIG_BOOTBLOCK_SOURCE)) $(objutil)/romcc/romcc $$(generic-deps)
	@printf "    ROMCC      $(subst $(obj)/,,$(@))\n"
	$(CC_bootblock) $(INCLUDES) $(INCLUDES_bootblock) -MM -MT$(objgenerated)/bootblock.inc \
		$< > $(objgenerated)/bootblock.inc.d
	$(ROMCC) -c -S $(bootblock_romccflags) -I. $(INCLUDES) $(INCLUDES_bootblock) $< -o $@

$(objgenerated)/bootblock.ld: $$(filter %.ld,$$(bootblock-objs))
	@printf "    GEN        $(subst $(obj)/,,$(@))\n"
	printf '$(foreach ldscript,$(^),INCLUDE "$(ldscript)"\n)' > $@

$(objgenerated)/bootblock.S: $$(bootblock_inc)
	@printf "    GEN        $(subst $(obj)/,,$(@))\n"
	printf '$(foreach inc,$(bootblock_inc),#include "$(inc)"\n)' > $@

bootblock-srcs += $(objgenerated)/bootblock.S

bootblock-S-ccopts += -I. -Wa,-acdlns=$$$$(basename $$$$(@)).disasm

$(objcbfs)/bootblock.debug: $(objgenerated)/bootblock.bootblock.o $(objgenerated)/bootblock.ld
	@printf "    LINK       $(subst $(obj)/,,$(@))\n"
	$(call link,bootblock,$(<),-o $(@) -L$(obj) -T $(objgenerated)/bootblock.ld,)

endif #CONFIG_ARCH_BOOTBLOCK_X86_32

###############################################################################
# romstage
###############################################################################

ifeq ($(CONFIG_ARCH_ROMSTAGE_X86_32),y)

crt0s = $(src)/arch/x86/init/prologue.inc
romstage-srcs += $(src)/arch/x86/init/romstage.ld
crt0s += $(src)/cpu/x86/32bit/entry32.inc
romstage-srcs += $(src)/cpu/x86/32bit/entry32.ld

crt0s += $(src)/cpu/x86/fpu_enable.inc
ifeq ($(CONFIG_SSE),y)
crt0s += $(src)/cpu/x86/sse_enable.inc
endif

crt0s += $(cpu_incs)
crt0s += $(cpu_incs-y)

ifeq ($(CONFIG_LLSHELL),y)
crt0s += $(src)/arch/x86/llshell/llshell.inc
endif

crt0s += $(obj)/mainboard/$(MAINBOARDDIR)/romstage.inc

ifeq ($(CONFIG_ROMCC),y)
crt0s += $(src)/arch/x86/init/crt0_romcc_epilogue.inc
endif

ifeq ($(CONFIG_ROMCC),y)
ifeq ($(CONFIG_MMX),y)
 ifeq ($(CONFIG_SSE),y)
  ROMCCFLAGS = -mcpu=p4 -O2 # MMX, SSE
 else
  ROMCCFLAGS = -mcpu=p2 -O2 # MMX, !SSE
 endif
else
 ROMCCFLAGS = -mcpu=i386 -O2 # !MMX, !SSE
endif # CONFIG_MMX
ROMCCFLAGS += $(romstage-c-ccopts) $(romstage-generic-ccopts)

$(objcbfs)/romstage_%.bin: $(objcbfs)/romstage_%.elf
	@printf "    OBJCOPY    $(subst $(obj)/,,$(@))\n"
	$(OBJCOPY_romstage) -O binary $< $@

$(objcbfs)/romstage_%.elf: $(objcbfs)/romstage_%.debug
	@printf "    OBJCOPY    $(subst $(obj)/,,$(@))\n"
	cp $< $@.tmp
	$(NM_romstage) -n $@.tmp | sort > $(basename $@).map
	$(OBJCOPY_romstage) --strip-debug $@.tmp
	$(OBJCOPY_romstage) --add-gnu-debuglink=$< $@.tmp
	mv $@.tmp $@

$(obj)/mainboard/$(MAINBOARDDIR)/romstage.inc: $(src)/mainboard/$(MAINBOARDDIR)/romstage.c $(objutil)/romcc/romcc $$(generic-deps)
	printf "    ROMCC      romstage.inc\n"
	$(ROMCC) -c -S $(ROMCCFLAGS) -I. $(INCLUDES) $(INCLUDES_romstage) $< -o $@
else # CONFIG_ROMCC

$(obj)/mainboard/$(MAINBOARDDIR)/romstage.pre.inc: $(src)/mainboard/$(MAINBOARDDIR)/romstage.c $$(generic-deps)
	@printf "    CC         romstage.inc\n"
	$(CC_romstage) -MMD $(CFLAGS_romstage) $(romstage-c-ccopts) $(romstage-generic-ccopts) -I$(src) -I. -I$(obj) -c -S $< -o $@

$(obj)/mainboard/$(MAINBOARDDIR)/romstage.inc: $(obj)/mainboard/$(MAINBOARDDIR)/romstage.pre.inc
	@printf "    POST       romstage.inc\n"
	sed -e 's/\.rodata/.rom.data/g' -e 's/\^\.text/.section .rom.text/g' \
		-e 's/\^\.section \.text/.section .rom.text/g' $^ > $@.tmp
	mv $@.tmp $@
endif # CONFIG_ROMCC

$(objcbfs)/romstage_null.debug: $$(romstage-objs) $(LIBGCC_FILE_NAME_romstage) $(objgenerated)/romstage_null.ld
	@printf "    LINK       $(subst $(obj)/,,$(@))\n"
	$(call link,romstage,$(filter %.o %.a,$(^)),-o $(@) -L$(obj) -T $(objgenerated)/romstage_null.ld,$(LIBGCC_WRAP_LDFLAGS))
	$(NM_romstage) $@ | grep -q " [DdBb] "; if [ $$? -eq 0 ]; then \
		echo "Forbidden global variables in romstage:"; \
		$(NM_romstage) $@ | grep " [DdBb] "; test "$(CONFIG_CPU_AMD_AGESA)" = y; \
		else true; fi

$(objcbfs)/romstage_xip.debug: $$(romstage-objs) $(LIBGCC_FILE_NAME_romstage) $(objgenerated)/romstage_xip.ld
	@printf "    LINK       $(subst $(obj)/,,$(@))\n"
	$(call link,romstage,$(filter %.o %.a,$(^)),-o $(@) -L$(obj) -T $(objgenerated)/romstage_xip.ld,$(LIBGCC_WRAP_LDFLAGS))

$(objgenerated)/romstage_null.ld: $$(filter %.ld,$$(romstage-objs))
	@printf "    GEN        $(subst $(obj)/,,$(@))\n"
	rm -f $@
	printf "ROMSTAGE_BASE = 0x0;\n" > $@.tmp
	printf '$(foreach ldscript,$(^),INCLUDE "$(ldscript)"\n)' >> $@.tmp
	mv $@.tmp $@

$(objgenerated)/romstage_xip.ld: $(objgenerated)/romstage_null.ld $(objcbfs)/base_xip.txt
	@printf "    GEN        $(subst $(obj)/,,$(@))\n"
	rm -f $@
	sed -e 's/^/ROMSTAGE_BASE = /g' -e 's/$$/;/g' $(objcbfs)/base_xip.txt > $@.tmp
	sed -e '/ROMSTAGE_BASE/d' $(objgenerated)/romstage_null.ld >> $@.tmp
	mv $@.tmp $@

$(objcbfs)/base_xip.txt: $(obj)/coreboot.pre1 $(objcbfs)/romstage_null.bin
	rm -f $@
	$(CBFSTOOL) $(obj)/coreboot.pre1 locate -T -f $(objcbfs)/romstage_null.bin -n $(CONFIG_CBFS_PREFIX)/romstage -P $(CONFIG_XIP_ROM_SIZE) > $@.tmp \
	 || { echo "The romstage is larger than XIP size. Please expand the CONFIG_XIP_ROM_SIZE" ; exit 1; }
	mv $@.tmp $@

$(objgenerated)/crt0.S: $$(crt0s)
	@printf "    GEN        $(subst $(obj)/,,$(@))\n"
	printf '$(foreach crt0,$(crt0s),#include "$(crt0)"\n)' > $@

romstage-srcs += $(objgenerated)/crt0.S

# Compiling crt0 with -g seems to trigger https://sourceware.org/bugzilla/show_bug.cgi?id=6428
romstage-S-ccopts += -I. -Wa,-acdlns=$$$$(basename $$$$(@)).disasm -g0

endif # CONFIG_ARCH_ROMSTAGE_X86_32

###############################################################################
# ramstage
###############################################################################

ifeq ($(CONFIG_ARCH_RAMSTAGE_X86_32),y)

ramstage-srcs += $(wildcard src/mainboard/$(MAINBOARDDIR)/mainboard.c)
ifeq ($(CONFIG_GENERATE_MP_TABLE),y)
ifneq ($(wildcard src/mainboard/$(MAINBOARDDIR)/mptable.c),)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/mptable.c
endif
endif
ifeq ($(CONFIG_GENERATE_PIRQ_TABLE),y)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/irq_tables.c
endif
ifneq ($(wildcard src/mainboard/$(MAINBOARDDIR)/reset.c),)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/reset.c
endif
ifeq ($(CONFIG_GENERATE_ACPI_TABLES),y)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/acpi_tables.c
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/dsdt.asl
ifneq ($(wildcard src/mainboard/$(MAINBOARDDIR)/ssdt2.asl),)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/ssdt2.asl
endif
ifneq ($(wildcard src/mainboard/$(MAINBOARDDIR)/ssdt3.asl),)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/ssdt3.asl
endif
ifneq ($(wildcard src/mainboard/$(MAINBOARDDIR)/ssdt4.asl),)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/ssdt4.asl
endif
ifneq ($(wildcard src/mainboard/$(MAINBOARDDIR)/ssdt5.asl),)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/ssdt5.asl
endif
ifneq ($(wildcard src/mainboard/$(MAINBOARDDIR)/fadt.c),)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/fadt.c
endif
endif # CONFIG_GENERATE_ACPI_TABLES
ifeq ($(CONFIG_HAVE_SMI_HANDLER),y)
ifneq ($(wildcard src/mainboard/$(MAINBOARDDIR)/smihandler.c),)
smm-srcs += src/mainboard/$(MAINBOARDDIR)/smihandler.c
endif
endif
ifneq ($(wildcard src/mainboard/$(MAINBOARDDIR)/get_bus_conf.c),)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/get_bus_conf.c
endif

ifeq ($(CONFIG_RELOCATABLE_RAMSTAGE),y)

$(eval $(call rmodule_link,$(objcbfs)/ramstage.debug, $(objgenerated)/ramstage.o, $(CONFIG_HEAP_SIZE),x86_32))

# The rmodule_link defintion creates an elf file with .rmod extension.
$(objcbfs)/ramstage.elf: $(objcbfs)/ramstage.debug.rmod
	cp $< $@

else # CONFIG_RELOCATABLE_RAMSTAGE

ramstage-srcs += $(src)/arch/x86/ramstage.ld

$(objcbfs)/ramstage.debug: $(objgenerated)/ramstage.o
	@printf "    CC         $(subst $(obj)/,,$(@))\n"
	$(call link,ramstage,$(<),-o $(@) -L$(obj) -T $(obj)/arch/x86/ramstage.ramstage.ld,)

endif # CONFIG_RELOCATABLE_RAMSTAGE

$(objgenerated)/ramstage.o: $$(ramstage-objs) $(LIBGCC_FILE_NAME_ramstage)
	@printf "    CC         $(subst $(obj)/,,$(@))\n"
	$(call link,ramstage,$(filter %.o %.a,$(^)),-o $(@) -L$(obj),-r $(LIBGCC_WRAP_LDFLAGS))

endif # CONFIG_ARCH_ROMSTAGE_X86_32


################################################################################
# ACPI ASL
################################################################################

# Compile ASLs in three steps: src/%.asl -CPP-> obj/%.<class>.asl -IASL-> obj/%.<class>.c -CC-> obj/%.<class>.o
generic-asl-ccopts += $(PREPROCESS_ONLY) -D__ACPI__ -I$(src)/mainboard/$(MAINBOARDDIR)

%.c: %.asl
	@printf "    IASL       $(subst $(top)/,,$(@))\n"
	cd $(dir $(<)); $(IASL) -p $(notdir $(<)) -tc $(notdir $(<))
	mv $(<:%.asl=%.hex) $(@)
# Change array identifier in generated files for everything but dsdt to avoid conflicts
# (TODO: Shouldn't we just call that one AmlCode_dsdt as well for consistency?)
	$(if $(subst dsdt,,$(basename $(basename $(notdir $(<))))),sed -ie 's/AmlCode/AmlCode_$(basename $(basename $(notdir $(<))))/' $(@))

# Prepend function that does equivalent of '<class>-y += <aslfile>.o' for all <aslfile>.asl in <class> to postprocessors
postprocessors := $$(foreach class,$$(classes), \
	$$(foreach asl,$$(filter %.asl,$$(call src-to-obj,$$(class),$$($$(class)-srcs))), \
		$$(eval ramstage-objs += $$(asl:%.asl=%.o)) \
		$$(eval ramstage-srcs += $$(asl:%.asl=%.c)))) \
	$(postprocessors)


################################################################################

seabios:
	$(MAKE) -C payloads/external/SeaBIOS -f Makefile.inc \
			HOSTCC="$(HOSTCC)" \
			CC="$(CC_x86_32)" LD="$(LD_x86_32)" OBJDUMP="$(OBJDUMP_x86_32)" \
			OBJCOPY="$(OBJCOPY_x86_32)" STRIP="$(STRIP_x86_32)" \
			CONFIG_SEABIOS_MASTER=$(CONFIG_SEABIOS_MASTER) \
			CONFIG_SEABIOS_STABLE=$(CONFIG_SEABIOS_STABLE) \
			OUT=$(abspath $(obj)) IASL="$(IASL)"

filo:
	$(MAKE) -C payloads/external/FILO -f Makefile.inc \
			HOSTCC="$(HOSTCC)" \
			CC="$(CC_x86_32)" LD="$(LD_x86_32)" OBJDUMP="$(OBJDUMP_x86_32)" \
			OBJCOPY="$(OBJCOPY_x86_32)" STRIP="$(STRIP_x86_32)" \
			CONFIG_FILO_MASTER=$(CONFIG_FILO_MASTER) \
			CONFIG_FILO_STABLE=$(CONFIG_FILO_STABLE)
