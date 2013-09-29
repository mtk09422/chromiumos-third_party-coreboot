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
################################################################################

# Take care of subdirectories
subdirs-y += boot/
subdirs-y += lib/

# Things that appear in every board
ramstage-y += exception.c
ramstage-y += exception_asm.S

bootblock-$(CONFIG_BOOTBLOCK_CONSOLE) += early_console.c
bootblock-y += cache.c
bootblock-y += mmu.c

bootblock-y += bootblock.S
bootblock-y += id.S
$(obj)/arch/armv7/id.bootblock.o: $(obj)/build.h
bootblock-y += $(call strip_quotes,$(CONFIG_BOOTBLOCK_SOURCE))

bootblock-y += stages.c
romstage-y += stages.c
ramstage-y += stages.c

romstage-y += cache.c
romstage-y += div0.c
romstage-$(CONFIG_EARLY_CONSOLE) += early_console.c

ramstage-y += div0.c
ramstage-y += cache.c
ramstage-y += cpu.c
ramstage-y += mmu.c

bootblock-y += eabi_compat.c
romstage-y += eabi_compat.c
ramstage-y += eabi_compat.c

ramstage-y += boot.c
ramstage-y += tables.c

bootblock-y += memset.S
bootblock-y += memcpy.S
bootblock-y += memmove.S

romstage-y += memset.S
romstage-y += memcpy.S
romstage-y += memmove.S

ramstage-y += memset.S
ramstage-y += memcpy.S
ramstage-y += memmove.S

rmodules-y += memset.S
rmodules-y += memcpy.S
rmodules-y += memmove.S
rmodules-y += eabi_compat.c

VBOOT_STUB_DEPS += $(obj)/arch/armv7/eabi_compat.rmodules.o

romstage-$(CONFIG_COLLECT_TIMESTAMPS) += timestamp.c
ramstage-$(CONFIG_COLLECT_TIMESTAMPS) += timestamp.c

ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/mainboard.c

################################################################################
# armv7 specific tools

################################################################################
# Common recipes for all stages

CFLAGS += \
	-ffixed-r8\
	-march=armv7-a\
	-marm\
	-mno-unaligned-access\
	-mthumb\
	-mthumb-interwork

$(objcbfs)/%.bin: $(objcbfs)/%.elf
	@printf "    OBJCOPY    $(subst $(obj)/,,$(@))\n"
	$(OBJCOPY) -O binary $< $@

$(objcbfs)/%.elf: $(objcbfs)/%.debug
	@printf "    OBJCOPY    $(subst $(obj)/,,$(@))\n"
	cp $< $@.tmp
	$(NM) -n $@.tmp | sort > $(basename $@).map
	$(OBJCOPY) --strip-debug $@.tmp
	$(OBJCOPY) --add-gnu-debuglink=$< $@.tmp
	mv $@.tmp $@

################################################################################
# Build the final rom image

COREBOOT_ROM_DEPENDENCIES:=
ifeq ($(CONFIG_PAYLOAD_ELF),y)
COREBOOT_ROM_DEPENDENCIES+=$(CONFIG_PAYLOAD_FILE)
endif

extract_nth=$(word $(1), $(subst |, ,$(2)))

ifneq ($(CONFIG_UPDATE_IMAGE),y)
prebuild-files = \
	$(foreach file,$(cbfs-files), \
	$(CBFSTOOL) $@.tmp \
	add$(if $(filter stage,$(call extract_nth,3,$(file))),-stage)$(if $(filter payload,$(call extract_nth,3,$(file))),-payload) \
	-f $(call extract_nth,1,$(file)) \
	-n $(call extract_nth,2,$(file)) $(if $(filter-out stage,$(call extract_nth,3,$(file))),-t $(call extract_nth,3,$(file))) \
	$(if $(call extract_nth,4,$(file)),-b $(call extract_nth,4,$(file))) &&)
prebuilt-files = $(foreach file,$(cbfs-files), $(call extract_nth,1,$(file)))

# TODO Change -b to Kconfig variable.
$(obj)/coreboot.pre1: $(objcbfs)/bootblock.bin $$(prebuilt-files) $(CBFSTOOL) $$(cpu_ucode_cbfs_file)
	$(CBFSTOOL) $@.tmp create -m armv7 -s $(CONFIG_COREBOOT_ROMSIZE_KB)K \
		-B $(objcbfs)/bootblock.bin -a 64 \
		-b $(CONFIG_BOOTBLOCK_ROM_OFFSET) \
		-H $(CONFIG_CBFS_HEADER_ROM_OFFSET) \
		-o $(CONFIG_CBFS_ROM_OFFSET)
	$(prebuild-files) true
	$(call add-cpu-microcode-to-cbfs,$@.tmp)
	mv $@.tmp $@
else
.PHONY: $(obj)/coreboot.pre1
$(obj)/coreboot.pre1: $(CBFSTOOL)
	mv $(obj)/coreboot.rom $@
endif

$(obj)/coreboot.rom: $(obj)/coreboot.pre $(objcbfs)/coreboot_ram.elf $(CBFSTOOL) $(call strip_quotes,$(COREBOOT_ROM_DEPENDENCIES))
	@printf "    CBFS       $(subst $(obj)/,,$(@))\n"
	cp $(obj)/coreboot.pre $@.tmp
	$(CBFSTOOL) $@.tmp add-stage -f $(objcbfs)/coreboot_ram.elf -n $(CONFIG_CBFS_PREFIX)/coreboot_ram -c $(CBFS_COMPRESS_FLAG)
ifeq ($(CONFIG_PAYLOAD_NONE),y)
	@printf "    PAYLOAD    none (as specified by user)\n"
endif
ifeq ($(CONFIG_PAYLOAD_ELF),y)
	@printf "    PAYLOAD    $(CONFIG_PAYLOAD_FILE) (compression: $(CBFS_PAYLOAD_COMPRESS_NAME))\n"
	$(CBFSTOOL) $@.tmp add-payload -f $(CONFIG_PAYLOAD_FILE) -n $(CONFIG_CBFS_PREFIX)/payload -c $(CBFS_PAYLOAD_COMPRESS_FLAG)
endif
ifeq ($(CONFIG_INCLUDE_CONFIG_FILE),y)
	@printf "    CONFIG     $(DOTCONFIG)\n"
	if [ -f $(DOTCONFIG) ]; then \
	echo "# This image was built using git revision" `git rev-parse HEAD` > $(obj)/config.tmp ; \
	sed -e '/^#/d' -e '/^ *$$/d' $(DOTCONFIG) >> $(obj)/config.tmp ; \
	$(CBFSTOOL) $@.tmp add -f $(obj)/config.tmp -n config -t raw; rm -f $(obj)/config.tmp ; fi
endif
	mv $@.tmp $@
	@printf "    CBFSPRINT  $(subst $(obj)/,,$(@))\n\n"
	$(CBFSTOOL) $@ print

################################################################################
# Build the bootblock

$(objcbfs)/bootblock.debug: $(src)/arch/armv7/bootblock.ld $(obj)/ldoptions $$(bootblock-objs) $(obj)/config.h
	@printf "    LINK       $(subst $(obj)/,,$(@))\n"
ifeq ($(CONFIG_COMPILER_LLVM_CLANG),y)
	$(LD) -m armelf_linux_eabi -static -o $@ -L$(obj) $< -T $(src)/arch/armv7/bootblock.ld
else
	$(CC) $(CFLAGS) -nostartfiles -include $(obj)/config.h -static -o $@ -L$(obj) -T $(src)/arch/armv7/bootblock.ld -Wl,--start-group $(bootblock-objs) $(LIBGCC_FILE_NAME) -Wl,--end-group
endif

################################################################################
# Build the romstage

$(objcbfs)/romstage.debug: $$(romstage-objs) $(src)/arch/armv7/romstage.ld $(obj)/ldoptions
	@printf "    LINK       $(subst $(obj)/,,$(@))\n"
ifeq ($(CONFIG_COMPILER_LLVM_CLANG),y)
	$(LD) -nostdlib -nostartfiles -static -o $@ -L$(obj) $(romstage-objs) -T $(src)/arch/armv7/romstage.ld
else
	$(CC) $(CFLAGS) -nostartfiles -static -o $@ -L$(obj) -T $(src)/arch/armv7/romstage.ld -Wl,--start-group $(romstage-objs) $(LIBGCC_FILE_NAME) -Wl,--end-group
endif

################################################################################
# Build the ramstage

$(objcbfs)/coreboot_ram.debug: $$(ramstage-objs) $(LIBGCC_FILE_NAME) $(src)/arch/armv7/coreboot_ram.ld $(obj)/ldoptions
	@printf "    CC         $(subst $(obj)/,,$(@))\n"
ifeq ($(CONFIG_COMPILER_LLVM_CLANG),y)
	$(LD) -m -m armelf_linux_eabi -o $@ --wrap __divdi3 --wrap __udivdi3 --wrap __moddi3 --wrap __umoddi3 --wrap __uidiv --start-group $(ramstage-objs) $(LIBGCC_FILE_NAME) --end-group -T $(src)/arch/armv7/coreboot_ram.ld
else
	$(CC) $(CFLAGS) -nostartfiles -static -o $@ -L$(obj) -Wl,--start-group $(ramstage-objs) $(LIBGCC_FILE_NAME) -Wl,--end-group -T $(src)/arch/armv7/coreboot_ram.ld
endif

################################################################################
# Build the final rom image

$(obj)/coreboot.pre: $(objcbfs)/romstage.elf $(obj)/coreboot.pre1 $(CBFSTOOL)
	@printf "    CBFS       $(subst $(obj)/,,$(@))\n"
	cp $(obj)/coreboot.pre1 $@.tmp
	$(CBFSTOOL) $@.tmp add-stage \
		-f $(objcbfs)/romstage.elf \
		-n $(CONFIG_CBFS_PREFIX)/romstage -c none
	mv $@.tmp $@
