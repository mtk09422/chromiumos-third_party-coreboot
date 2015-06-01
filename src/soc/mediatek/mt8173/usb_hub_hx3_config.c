/*
 * This file is part of the coreboot project.
 *
 * Copyright 2015 MediaTek Inc.
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
/* Cypress HX3 USB Hub configuration */

#include <assert.h>
#include <console/console.h>
#include <delay.h>
#include <device/i2c.h>
#include <endian.h>
#include <mainboard/cust_gpio_usage.h>
#include <mainboard/cust_i2c.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <soc/gpio.h>
#include <soc/i2c.h>
#include <soc/pmic.h>
#include <soc/usb_hub.h>
#include <types.h>

/* I2C bus id */
#define HX3_I2C_ID 4

/* USB Descriptor types */
#define USB_DT_STRING 0x03

/* Cypress HX3 I2C address */
#define HX3_I2C_ADDR 0x60

/* Full size setting blob */
#define HX3_SETTINGS_SIZE 192

/* USB VID of Mediatek */
#define USB_VID_MEDIATEK 0x0e8d

/* I2C speed of MASTER (MT8173) */
#define HX3_I2C4_SPEED 400 /* kHz */

/* I2C speed of SLAVE (HUB) */
#if (HX3_I2C4_SPEED <= 100)
#define HUB_I2C_SPEED  0x11 /* I2C speed : 100kHz */
#elif (HX3_I2C4_SPEED == 400)
#define HUB_I2C_SPEED  0x01 /* I2C speed : 400kHz */
#else
#error "HX3: Not support this I2C speed"
#endif

/* HX3 hub reset time */
#define HX3_RESET_TIME 50 /* ms */
/* Wait after HX3 hub reset */
#define HX3_WAIT_TIME  50 /* ms */

/* HUB reset retry count */
#define HX3_RESET_RETRY 5

/* Debug control */
#define HX3_CONFIG_DEBUG 0
#if HX3_CONFIG_DEBUG
#define HX3_HUB_ASSERT(x) assert(x)
#else
#define HX3_HUB_ASSERT(x)
#endif

/* Cypress HX3 hub settings blob */
const uint8_t hx3_settings[5 + HX3_SETTINGS_SIZE] = {
	 'C', 'Y', /* Cypress magic signature */
	 HUB_I2C_SPEED, /* I2C speed : 100kHz */
	 0xd4, /* Image type: Only settings, no firmware */
	 HX3_SETTINGS_SIZE, /* 192 bytes payload */
	 0x8d, 0x0e, 0x58, 0x13,/* USB VID:PID 0x0e8d:0x1358 (Mediatek:1358) */
	 0x00, 0x10, /* bcdDevice 1.00 */
	 0x00, /* Reserved */
	 0x07, /* Offset 12: 3 SuperSpeed ports (DS3~DS1), no shared link */
	 0x32, /* Offset 13: bPwrOn2PwrGood : 100 ms */
	 0x77, /* Offset 14: 3 Downstream ports (DS3~DS1) */
	 0x10, /* Offset 15: Port indicators(bit 4); Compound device (bit 3) */
	 0xa0, /* Offset 16: Suspend indicator disabled,
		* Power switch : active HIGH
		*/
	 0x15, /* Offset 17: BC1.2 + ACA Dock + Ghost charging */
	 0xf0, /* Offset 18: CDP enabled, DCP disabled */
	 0x68, /* Offset 19: Embedded_HUB (bit 7);
		* Overcurrent input(bit 4): LOW
		*/
	 0x00, /* Reserved */
	 0x08, /* Offset 21: USB String descriptors enabled */
	 0x00, /* Reserved */
	 0x00, /* Offset 23: HS DS driver amplitude control */
	 0x12, /* Offset 24: HS US amplitude control (bit 6,7);
		* HS driver slope control for all ports
		*/
	 0x00, /* Offset 25: HS driver pre-emphasis enable & depth */
	 0x2c, /* Offset 26: HS driver pre-emphasis depth */
	 0x66, 0x66, /* USB3.0 TX driver de-emphasis */
	 0x69, 0x29, 0x29, 0x29, 0x29, /* TX amplitude */
	 0x00, /* Offset 34: Reserved */
	 0x58, 0x13, /* Offset 35: USB2.0 PID: 0x1358 */
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* Reserved */
	 0x04, USB_DT_STRING, 0x09, 0x04,
	 /* Offset 45-48: LangID = 0x0409 US English */

	 0x1C, USB_DT_STRING, /* Offset 49: Manufacturer string descriptor */
	 0x4D, 0x00, 0x65, 0x00, 0x64, 0x00, 0x69, 0x00, /* Mediatek Inc. */
	 0x61, 0x00, 0x74, 0x00, 0x65, 0x00, 0x6B, 0x00, /* as UTF-8 */
	 0x20, 0x00, 0x49, 0x00, 0x6E, 0x00, 0x63, 0x00,
	 0x2E, 0x00,

	 0x10, USB_DT_STRING, /* Product string descriptor */
	 0x4F, 0x00, 0x61, 0x00, 0x6B, 0x00, 0x20, 0x00, /* Oak Hub */
	 0x48, 0x00, 0x75, 0x00, 0x62, 0x00,             /* as UTF-8 */

	 0x02, USB_DT_STRING, /* Serial string descriptor : empty */
	 /* Free space for more strings */
};

void configure_hx3(void)
{
	int ret;
	int retry_cnt;
	uint16_t addr = 0x0000;
	uint8_t *data = (uint8_t *)hx3_settings;

	/* Setup the VPCA7 of PMIC to 1.2v (in pmic.c) */
	/* Setup the VGP6 to 3.3v (in pmic.c) */
	/* Reset the bridge to put it back in bootloader mode */
	if (GPIO_USB_HUB_HX3_RESET_L_PIN != 0) {
		/* 0: GPIO mode */
		mt_set_gpio_mode(GPIO_USB_HUB_HX3_RESET_L_PIN, 0);
		/* dir = output */
		mt_set_gpio_dir(GPIO_USB_HUB_HX3_RESET_L_PIN, 1);
	} else {
		printk(BIOS_DEBUG, "USB Hub HX3 Reset pin doesn't define!\n");
		HX3_HUB_ASSERT(0);
	}

	retry_cnt = 0;
	do {
		retry_cnt++;
		/* Reset the hub */
		/* output = LOW */
		mt_set_gpio_out(GPIO_USB_HUB_HX3_RESET_L_PIN, 0);
		/* Keep the reset low at least 10ms (same as the RC) */
		mdelay(HX3_RESET_TIME);
		/* output = HIGH */
		mt_set_gpio_out(GPIO_USB_HUB_HX3_RESET_L_PIN, 1);
		mdelay(HX3_WAIT_TIME);

		/* do 64-byte Page Write */
		ret = i2c_write_raw(HX3_I2C_ID, HX3_I2C_ADDR, (uint8_t *)&addr, 2);
		if (ret != 0) {
			printk(BIOS_DEBUG, "HX3 set page write fail %d\n", ret);
			continue;
		}

		/* send Page Write address */
		ret = i2c_write_raw(HX3_I2C_ID, HX3_I2C_ADDR, data,
				    sizeof(hx3_settings));
		if (ret != 0) {
			printk(BIOS_DEBUG, "HX3 transfer failed %d\n", ret);
			continue;
		}
	} while ((retry_cnt < HX3_RESET_RETRY) && (ret != 0));

	if (ret != 0) {
		printk(BIOS_DEBUG, "Config fail with retry#%d\n", retry_cnt);
		HX3_HUB_ASSERT(0);
	}
}

