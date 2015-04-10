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

bootblock-y += bootblock.c
bootblock-y += cpu_lib.S
bootblock-y += cbfs.c
bootblock-y += pll.c
bootblock-y += spi.c
bootblock-y += timer.c

ifeq ($(CONFIG_BOOTBLOCK_CONSOLE),y)
bootblock-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c
endif

bootblock-y += gpio.c gpio_init.c pmic_wrap_init.c pmic.c da9212.c
bootblock-y += wdt.c rtc.c
bootblock-$(CONFIG_MTK_EMMC_CBFS) += emmc_cbfs.c  blkdev.c mmc_common_inter.c mmc_core.c \
				     msdc.c msdc_utils.c cust_msdc.c
bootblock-$(CONFIG_MTK_SPI_CBFS) += nor_cbfs.c  blkdev.c nor_if.c

################################################################################

verstage-y += verstage.c
verstage-y += i2c.c
verstage-y += spi.c

verstage-$(CONFIG_CONSOLE_SERIAL) += uart.c

verstage-y += timer.c cpu_lib.S
verstage-y += gpio.c gpio_init.c pmic_wrap_init.c pmic.c da9212.c
verstage-$(CONFIG_MTK_EMMC_CBFS) += emmc_cbfs.c blkdev.c mmc_common_inter.c \
				    mmc_core.c msdc.c msdc_utils.c cust_msdc.c
verstage-$(CONFIG_MTK_SPI_CBFS) += nor_cbfs.c blkdev.c nor_if.c \
				   cbfs.c
verstage-y += mmu_operations.c

################################################################################

romstage-y += cpu_lib.S cpu.c
romstage-$(CONFIG_MTK_EMMC_CBFS) += emmc_cbfs.c \
   blkdev.c mmc_common_inter.c mmc_core.c msdc.c msdc_utils.c cust_msdc.c
romstage-$(CONFIG_MTK_SPI_CBFS) += nor_cbfs.c blkdev.c nor_if.c cbfs.c
romstage-y += pll.c mempll.c
romstage-y += timer.c

ifeq ($(CONFIG_EARLY_CONSOLE),y)
romstage-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c
endif
romstage-$(CONFIG_DYNAMIC_CBMEM) += cbmem.c
romstage-y += spi.c
romstage-y += gpio.c gpio_init.c
romstage-y += pmic_wrap_init.c i2c.c pmic.c da9212.c
romstage-y += stdlib.c memory.c
romstage-y += addressmap.c
romstage-y += emi.c dramc_pi_basic_api.c dramc_pi_calibration_api.c

################################################################################

ramstage-y += cpu_lib.S
ramstage-y += cbmem.c
ramstage-y += spi.c
ramstage-$(CONFIG_MTK_EMMC_CBFS) += emmc_cbfs.c blkdev.c mmc_common_inter.c mmc_core.c msdc.c msdc_utils.c cust_msdc.c
ramstage-$(CONFIG_MTK_SPI_CBFS) += nor_cbfs.c blkdev.c  nor_if.c cbfs.c
ramstage-y += soc.c cpu.c mtcmos.c
ramstage-y += timer.c
ramstage-$(CONFIG_CONSOLE_SERIAL_UART) += uart.c
ramstage-y += pmic_wrap_init.c pmic.c i2c.c
ramstage-y += ramstage.c mmu_operations.c
ramstage-$(CONFIG_ARM64_USE_SECURE_MONITOR) += gic.c
ramstage-y += addressmap.c
ramstage-y += gpio.c gpio_init.c

ramstage-${CONFIG_MEDIATEK_DISPLAY_INIT} += display.c
ramstage-${CONFIG_MEDIATEK_DISPLAY_INIT} += ddp.c
ramstage-${CONFIG_MEDIATEK_DISPLAY_INIT} += dsi.c
ramstage-${CONFIG_MEDIATEK_DISPLAY_INIT} += it6151.c

ramstage-y += usb.c
ramstage-$(CONFIG_HAVE_USB_HUB_HX3) += usb_hub_hx3_config.c

ramstage-$(CONFIG_ARM64_USE_SECURE_MONITOR) += secmon.c

secmon-$(CONFIG_ARM64_USE_SECURE_MONITOR) += cpu_lib.S
secmon-$(CONFIG_ARM64_USE_SECURE_MONITOR) += psci.c
secmon-$(CONFIG_ARM64_USE_SECURE_MONITOR) += cpu.c mtcmos.c
secmon-$(CONFIG_ARM64_USE_SECURE_MONITOR) += timer.c
secmon-$(CONFIG_ARM64_USE_SECURE_MONITOR) += gic.c

ramstage-$(CONFIG_ARM64_USE_ARM_TRUSTED_FIRMWARE) += bl31_plat_param.c

ifeq ($(CONFIG_ARM64_USE_ARM_TRUSTED_FIRMWARE),y)
BL31_MAKEARGS += PLAT=mt8173
endif

################################################################################

ifeq ($(CONFIG_MTK_EMMC_CBFS), y)
TYPE:=mmc
else
TYPE=sf
endif

# Generate the actual coreboot bootblock code
$(objcbfs)/bootblock.bin: $(objcbfs)/bootblock.elf
	@printf "    OBJCOPY    $(subst $(obj)/,,$(@))\n"
	$(OBJCOPY_bootblock) -O binary $< $@.tmp
	./util/mtkheader/gen-bl-img.py mt8173 $(TYPE) $@.tmp $@

INCLUDES += -Isrc/soc/mediatek/mt8173/include
