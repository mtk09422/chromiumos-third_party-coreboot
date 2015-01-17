##
## This file is part of the coreboot project.
##
## Copyright (C) 2013 Google Inc.
## Copyright (C) 2015 Intel Corp.
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

SPD_BIN = $(obj)/spd.bin

ifeq ($(CONFIG_RVP_SUPPORT),y)
# Add RVP SPD information as needed here.  If left blank a NULL SPD pointer
# will be passed to FSP.
SPD_SOURCES =
else
# Currently only one memory configuration is used for strago, yet verification using BSW RVP
# will require using an SPD hex file to match the memory SODIMM used on BSW RVP.
# 2GiB total - 2 x 1GiB Hynix  H5TC2G63FFR-PBA 1600MHz
SPD_SOURCES = hynix_2GiB_dimm_H5TC4G63AFR-PBA
endif

SPD_DEPS := $(foreach f, $(SPD_SOURCES), src/mainboard/$(MAINBOARDDIR)/spd/$(f).spd.hex)

# Include spd rom data
$(SPD_BIN): $(SPD_DEPS)
	for f in $+; \
	  do for c in $$(cat $$f | grep -v ^#); \
	    do echo -e -n "\\x$$c"; \
	  done; \
	done > $@

cbfs-files-y += spd.bin
spd.bin-file := $(SPD_BIN)
spd.bin-type := 0xab
spd.bin-position := $(CONFIG_SPD_CBFS_ADDRESS)
