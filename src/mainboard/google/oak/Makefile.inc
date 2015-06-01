##
## This file is part of the coreboot project.
##
## Copyright 2014 Google Inc.
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

bootblock-y += reset.c
bootblock-y += memlayout.ld
bootblock-y += boardid.c
bootblock-c-ccopts += -mstrict-align

verstage-y += verstage.c
verstage-y += chromeos.c
verstage-y += reset.c
verstage-y += memlayout.ld
verstage-c-ccopts += -mstrict-align

romstage-y += romstage.c custom_emi.c
romstage-y += reset.c
romstage-y += memlayout.ld
romstage-y += boardid.c
romstage-c-ccopts += -mstrict-align

ramstage-y += mainboard.c
ramstage-y += chromeos.c
ramstage-y += reset.c
ramstage-y += memlayout.ld
ramstage-y += boardid.c

INCLUDES += -Isrc/mainboard/google/oak/include
