romstage-$(CONFIG_CAR_MIGRATION) += car.c

subdirs-$(CONFIG_PARALLEL_MP) += name
ramstage-$(CONFIG_PARALLEL_MP) += mp_init.c

SIPI_ELF=$(obj)/cpu/x86/sipi_vector.elf
SIPI_RMOD=$(SIPI_ELF).rmod
SIPI_BIN=$(SIPI_ELF:.elf=)
SIPI_DOTO=$(SIPI_ELF:.elf=.o)

ifeq ($(CONFIG_PARALLEL_MP),y)
ramstage-srcs += $(SIPI_BIN)
endif
rmodules_$(ARCH-RAMSTAGE-y)-$(CONFIG_PARALLEL_MP) += sipi_vector.S

$(SIPI_DOTO): $(dir $(SIPI_ELF))sipi_vector.rmodules_$(ARCH-RAMSTAGE-y).o
	$(CC_ramstage) $(LDFLAGS) -nostdlib -r -o $@ $^

$(eval $(call rmodule_link,$(SIPI_ELF), $(SIPI_ELF:.elf=.o), 0,x86_32))

$(SIPI_BIN): $(SIPI_RMOD)
	$(OBJCOPY_ramstage) -O binary $< $@

$(SIPI_BIN).ramstage.o: $(SIPI_BIN)
	@printf "    OBJCOPY    $(subst $(obj)/,,$(@))\n"
	cd $(dir $@); $(OBJCOPY_ramstage) -I binary $(notdir $<) -O elf32-i386 -B i386 $(notdir $@)
