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

ifeq ($(CONFIG_BUILD_WITH_FAKE_IFD),y)
IFD_BIN_PATH := $(objgenerated)/ifdfake.bin
IFD_SECTIONS := $(addprefix -b ,$(CONFIG_IFD_BIOS_SECTION:"%"=%)) \
		$(addprefix -m ,$(CONFIG_IFD_ME_SECTION:"%"=%)) \
		$(addprefix -p ,$(CONFIG_IFD_PLATFORM_SECTION:"%"=%))
else
IFD_BIN_PATH := $(CONFIG_IFD_BIN_PATH)
endif

pch_add_me: $(obj)/coreboot.pre $(IFDTOOL) $(IFDFAKE)
ifeq ($(CONFIG_BUILD_WITH_FAKE_IFD),y)
	printf "\n** WARNING **\n"
	printf "Coreboot will be built with a fake Intel Firmware Descriptor (IFD).\n"
	printf "Never write a complete coreboot.rom with a fake IFD to your board's\n"
	printf "flash ROM! Make sure that you only write valid flash regions.\n\n"
	printf "    IFDFAKE    Building a fake Intel Firmware Descriptor\n"
	$(IFDFAKE) $(IFD_SECTIONS) $(IFD_BIN_PATH)
endif
	printf "    DD         Adding Intel Firmware Descriptor\n"
	dd if=$(IFD_BIN_PATH) \
		of=$(obj)/coreboot.pre conv=notrunc >/dev/null 2>&1
ifeq ($(CONFIG_HAVE_ME_BIN),y)
	printf "    IFDTOOL    me.bin -> coreboot.pre\n"
	$(objutil)/ifdtool/ifdtool \
		-i ME:$(CONFIG_ME_BIN_PATH) \
		$(obj)/coreboot.pre
	mv $(obj)/coreboot.pre.new $(obj)/coreboot.pre
endif

PHONY += pch_add_me
