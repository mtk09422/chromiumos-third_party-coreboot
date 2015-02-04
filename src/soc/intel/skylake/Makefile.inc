subdirs-y += bootblock
subdirs-y += microcode
subdirs-y += romstage
subdirs-y += ../common
subdirs-y += ../../../cpu/x86/tsc

romstage-y += iobp.c
romstage-y += me_status.c
romstage-y += memmap.c
romstage-y += pch.c
romstage-y += pei_data.c
romstage-y += stage_cache.c
romstage-y += tsc_freq.c

ramstage-y += memmap.c
ramstage-y += ramstage.c
ramstage-y += tsc_freq.c

INCLUDES += -Isrc/drivers/intel/fsp
INCLUDES += -Isrc/soc/intel/skylake
INCLUDES += -Isrc/soc/intel/skylake/include
INCLUDES += -Isrc/vendorcode/intel/fsp/fsp_1_1
INCLUDES += -Isrc/vendorcode/intel/edk2/uefi_2.4
INCLUDES += -Isrc/vendorcode/intel/edk2/uefi_2.4/MdePkg/Include
INCLUDES += -Isrc/vendorcode/intel/edk2/uefi_2.4/MdePkg/Include/Ia32
INCLUDES += -I$(CONFIG_FSP_INCLUDE_PATH)

# Run an intermediate step when producing coreboot.rom
# that adds additional components to the final firmware
# image outside of CBFS
INTERMEDIATE := pch_add_me

pch_add_me: $(obj)/coreboot.pre $(IFDTOOL)
	printf "    DD         Adding Intel Firmware Descriptor $(MAINBOARDDIR)/$(CONFIG_DESC_FILE)\n"
	dd if=3rdparty/mainboard/$(MAINBOARDDIR)/$(CONFIG_DESC_FILE) \
		of=$(obj)/coreboot.pre conv=notrunc >/dev/null 2>&1
	printf "    IFDTOOL    me.bin -> coreboot.pre\n"
	$(objutil)/ifdtool/ifdtool \
		-i ME:3rdparty/mainboard/$(MAINBOARDDIR)/me.bin \
		$(obj)/coreboot.pre
	mv $(obj)/coreboot.pre.new $(obj)/coreboot.pre

PHONY += pch_add_me
