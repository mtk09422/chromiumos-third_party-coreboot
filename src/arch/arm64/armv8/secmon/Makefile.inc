################################################################################
##
## This file is part of the coreboot project.
##
## Copyright (C) 2014 Google Inc.
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

$(eval $(call create_class_compiler,secmon,arm64))

SECMON_DIR=$(obj)/arch/arm64/armv8/secmon
SECMON_BIN=$(SECMON_DIR)/secmon
SECMON_ELF=$(SECMON_DIR)/secmon.elf
SECMON_RMOD=$(SECMON_DIR)/secmon.elf.rmod
SECMON_RAMSTAGE=$(SECMON_DIR)/secmon.ramstage.o

secmon-generic-ccopts += -D__SECMON__
secmon-c-ccopts += $(armv8_flags)
secmon-S-ccopts += $(armv8_asm_flags)

secmon-y += secmon_init.c
secmon-y += psci.c
secmon-y += smc.c
secmon-y += trampoline.S
secmon-y += ../exception.c
secmon-y += ../../cpu.c
secmon-y += ../../transition_asm.S ../../transition.c

ramstage-objs += $(SECMON_RAMSTAGE)

$(eval $(call rmodule_link,$(SECMON_ELF), $$$$(secmon-objs), 0,arm64))

$(SECMON_BIN): $(SECMON_RMOD)
	       $(OBJCOPY_secmon) -O binary $< $@

$(SECMON_RAMSTAGE): $(SECMON_BIN)
		    @printf "    OBJCOPY $(subst $(obj)/,,$(@))\n"
		    cd $(dir $@); $(OBJCOPY_secmon) -I binary $(notdir $<) -O elf64-littleaarch64 -B aarch64 $(notdir $@)
