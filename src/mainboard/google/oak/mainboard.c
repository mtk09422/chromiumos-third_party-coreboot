/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 Google Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <arch/cache.h>
#include <arch/io.h>
#include <boardid.h>
#include <boot/coreboot_tables.h>
#include <device/device.h>
#include <edid.h>
#include <soc/display.h>
#include <soc/ddp.h>
#include <soc/gpio.h>
#include <soc/pmic.h>
#include <soc/spm.h>
#include <soc/usb.h>
#include <soc/usb_hub.h>

static void configure_regulator(void)
{
	/* set vgp1 */
	pmic_config_interface(DIGLDO_CON30, 0, PMIC_VCAMD_ON_CTRL_MASK,
			      PMIC_VCAMD_ON_CTRL_SHIFT);
	pmic_config_interface(DIGLDO_CON5, 1, PMIC_RG_VCAMD_SW_EN_MASK,
			      PMIC_RG_VCAMD_SW_EN_SHIFT);
	switch (board_id()) {
	case 0:
		/* vgp1 set to 1.22V */
		pmic_config_interface(DIGLDO_CON19, 0, PMIC_RG_VCAMD_VOSEL_MASK,
				      PMIC_RG_VCAMD_VOSEL_SHIFT);
		/* vgp4 set to 1.8V */
		pmic_config_interface(DIGLDO_CON30, 0, PMIC_VGP4_ON_CTRL_MASK,
				      PMIC_VGP4_ON_CTRL_SHIFT);
		pmic_config_interface(DIGLDO_CON8, 1, PMIC_RG_VGP4_SW_EN_MASK,
				      PMIC_RG_VGP4_SW_EN_SHIFT);
		pmic_config_interface(DIGLDO_CON22, 3, PMIC_RG_VGP4_VOSEL_MASK,
				      PMIC_RG_VGP4_VOSEL_SHIFT);
		break;
	case 1:
		/* vgp1 set to 1.8V */
		pmic_config_interface(DIGLDO_CON19, 3, PMIC_RG_VCAMD_VOSEL_MASK,
				      PMIC_RG_VCAMD_VOSEL_SHIFT);
		break;
	default:
		break;
	}
}

static void configure_hdmi(void)
{
	u32 reg;

	reg = read32((void *)DISPSYS_CONFIG_BASE + DISP_REG_CONFIG_HDMI_EN);
	reg = reg & 0xffff7fff;
	write32((void *)DISPSYS_CONFIG_BASE + DISP_REG_CONFIG_HDMI_EN, reg);
}

#define USB_PORT_SWITCH_GPIO GPIO3

static void usb_port_switch(void)
{
	switch (board_id()) {
	case 0:
		/* Rev0: switch download port to type A */
		mt_set_gpio_mode(USB_PORT_SWITCH_GPIO, GPIO_MODE_00);
		mt_set_gpio_dir(USB_PORT_SWITCH_GPIO, GPIO_DIR_OUT);
		mt_set_gpio_out(USB_PORT_SWITCH_GPIO, GPIO_OUT_ONE);
		break;
	default:
		break;
	}

}

/* vpu sram power configuration */
static inline void configure_vpu_sram(void)
{
	write32((void *)SPM_VPU_SRAM_CON, 0xfffffff0);
}

static void configure_usb(void)
{
	switch (board_id()) {
	case 0:
		break; /* Rev0 doesn't have USB Hub */
	default:
		/* Config Cypress HX3 USB3 Hub */
		configure_hx3();
		break;
	}
}

static void mainboard_init(device_t dev)
{
	setup_usb_host();
	usb_port_switch();
	configure_hdmi();
	configure_vpu_sram();
	configure_regulator();
	configure_usb();
}

static void mainboard_enable(device_t dev)
{
	dev->ops->init = &mainboard_init;
}

struct chip_operations mainboard_ops = {
	.name = "oak",
	.enable_dev = mainboard_enable,
};

void mainboard_power_on_backlight(void)
{

}

