subdirs-y += bootblock
subdirs-y += microcode
subdirs-y += romstage
subdirs-y += ../common
subdirs-y += ../../../cpu/x86/tsc

ramstage-y += memmap.c
romstage-y += memmap.c
ramstage-y += tsc_freq.c
romstage-y += tsc_freq.c
ramstage-y += iosf.c
romstage-y += iosf.c
ramstage-y += ramstage.c
romstage-y += reset.c
ramstage-y += reset.c
romstage-y += stage_cache.c

INCLUDES += -Isrc/soc/intel/braswell/
INCLUDES += -Isrc/soc/intel/braswell/include

INCLUDES += -Isrc/drivers/intel/fsp
INCLUDES += -Isrc/vendorcode/intel/fsp/fsp_1_1
INCLUDES += -Isrc/vendorcode/intel/edk2/uefi_2.4
INCLUDES += -Isrc/vendorcode/intel/edk2/uefi_2.4/MdePkg/Include
INCLUDES += -Isrc/vendorcode/intel/edk2/uefi_2.4/MdePkg/Include/Ia32
INCLUDES += -I$(CONFIG_FSP_INCLUDE_PATH)

# Run an intermediate step when producing coreboot.rom
# that adds additional components to the final firmware
# image outside of CBFS
INTERMEDIATE:=braswell_add_me

braswell_add_me: $(obj)/coreboot.pre $(IFDTOOL)
	printf "    DD         Adding Intel Firmware Descriptor\n"
	dd if=3rdparty/mainboard/$(MAINBOARDDIR)/descriptor.bin \
		of=$(obj)/coreboot.pre conv=notrunc >/dev/null 2>&1
	printf "    IFDTOOL    me.bin -> coreboot.pre\n"
	$(objutil)/ifdtool/ifdtool \
		-i ME:3rdparty/mainboard/$(MAINBOARDDIR)/me.bin \
		$(obj)/coreboot.pre
	mv $(obj)/coreboot.pre.new $(obj)/coreboot.pre
ifeq ($(CONFIG_LOCK_MANAGEMENT_ENGINE),y)
	printf "    IFDTOOL    Locking Management Engine\n"
	$(objutil)/ifdtool/ifdtool -l $(obj)/coreboot.pre
	mv $(obj)/coreboot.pre.new $(obj)/coreboot.pre
else
	printf "    IFDTOOL    Unlocking Management Engine\n"
	$(objutil)/ifdtool/ifdtool -u $(obj)/coreboot.pre
	mv $(obj)/coreboot.pre.new $(obj)/coreboot.pre
endif