ramstage-$(CONFIG_DRIVERS_MC146818_RTC) += mc146818rtc.c
ramstage-$(CONFIG_DRIVERS_MC146818_CMOS) += mc146818.c
ramstage-y += isa-dma.c
ramstage-y += i8254.c
ramstage-y += i8259.c
ramstage-$(CONFIG_UDELAY_IO) += udelay_io.c
ramstage-y += keyboard.c

ifeq ($(CONFIG_DRIVERS_MC146818_CMOS),y)
romstage-$(CONFIG_USE_OPTION_TABLE) += mc146818rtc_early.c
endif

subdirs-y += tpm vga

cbfs-files-$(CONFIG_HAVE_CMOS_DEFAULT) += cmos.default
cmos.default-file = $(CONFIG_CMOS_DEFAULT_FILE):nvramtool
cmos.default-type = 0xaa

smm-y += mc146818.c
smm-y += mc146818rtc.c
