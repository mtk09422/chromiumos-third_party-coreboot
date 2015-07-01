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
/* Realtek RTS5411 USB Hub configuration */

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
#include <gpio.h>
#include <soc/i2c.h>
#include <soc/pmic.h>
#include <soc/usb_hub.h>
#include <types.h>

/* RTS5411 I2C address */
#define RTS5411_I2C_ADDR           0x6A

/* USB VID of Mediatek */
#define USB_VID_MEDIATEK           0x0e8d

/* Full size setting info */
#define VENDOR_INFO_LENGTH         0x60
#define VENDOR_INFO_XFER_LEN       0x18
#define VENDOR_INFO_HEADER_LEN     0x06
#define SMBUS_NOT_READY	           0x00
#define SMBUS_READY                0x01
#define SMBUS_ERROR                0x02

/* The SMBUS is supposed to be completed within 50ms */
#define SMBUS_TIMEOUT              50

/* RTS5411 hub reset time (ms) */
#define RTS5411_RESET_TIME         10

/* Wait after RTS5411 hub reset (ms) */
#define RTS5411_WAIT_TIME          200

/* HUB reset retry count */
#define RTS5411_RESET_RETRY        5

/* I2C sleep time (ms) */
#define RTS5411_I2C_SLEEP_TIME     1

/* Serial Number */
#define FW_COMPILED_YEAR_CHAR_3    2
#define FW_COMPILED_YEAR_CHAR_2    0
#define FW_COMPILED_YEAR_CHAR_1    1
#define FW_COMPILED_YEAR_CHAR_0    5
#define FW_COMPILED_MONTH_CHAR_H   0
#define FW_COMPILED_MONTH_CHAR_L   6
#define FW_COMPILED_DAY_CHAR_H     2
#define FW_COMPILED_DAY_CHAR_L     0
#define FW_COMPILED_HOUR_CHAR_H    1
#define FW_COMPILED_HOUR_CHAR_L    3
#define FW_COMPILED_MINUTE_CHAR_H  5
#define FW_COMPILED_MINUTE_CHAR_L  8

/* Debug control */
#define RTS5411_CONFIG_DEBUG       0
#if RTS5411_CONFIG_DEBUG
#define RTS5411_HUB_ASSERT(x)      assert(x)
#else
#define RTS5411_HUB_ASSERT(x)
#endif

uint8_t DLSETTING[VENDOR_INFO_LENGTH] = {
	0x55,	/* 00000000h: Valid check data */
	0xAA,
	0x0E,	/* 00000002h: Hub_VID_H_, Hub_VID_L */
	0x8D,
	0x13,	/* 00000004h: Hub_SS_PID_H, Hub_SS_PID_L */
	0x58,
	0x13,	/* 00000006h: Hub_HS_PID_H_, _Hub_HS_PID_L */
	0x58,
	0xB2,	/* 00000008h: LDO 3.3V Output & USB CONNECT DELAY TIME */
	0x00,	/* 00000009h: PHY/MAC modification and SSC */
	0x08,	/* 0000000Ah: Setup Data Valid Control 1 */
	0xFB,	/* 0000000Bh: Setup Data Valid Control 2 */
	0xFF,	/* 0000000Ch: Setup Data Valid Control 3 : DSP U1/U2 Enable */
	0xDD,	/* 0000000Dh: Primary Function Enable, reserved to zero */
	0x2A,	/* 0000000Eh: Minor Enable, reserved to zero */
	0x20,	/* 0000000Fh: Third Function Enable, reserved to zero */
	0x50,	/* 00000010h: SMBUS Configuration */
	0x38,	/* 00000011h: USP Attribute USB3 PHY */
	0x02,	/* 00000012h: USP Attribute USB2 PHY */
	0x00,	/* 00000013h: DSP1 Attribute A DSP setting */
	0xD7,	/* 00000014h: DSP1 Attribute B charging function */
	0x28,	/* 00000015h: DSP1 Attribute C USB3 PHY */
	0x40,	/* 00000016h: DSP1 Attribute D USB2 PHY */
	0x04,	/* 00000017h: DSP2 Attribute A DSP Setting */
	0xD7,	/* 00000018h: DSP2 Attribute B charging function */
	0x28,	/* 00000019h: DSP2 Attribute C USB3 PHY */
	0x40,	/* 0000001Ah: DSP2 Attribute D USB2 PHY */
	0x08,	/* 0000001Bh: DSP3 Attribute A DSP Setting */
	0xD7,	/* 0000001Ch: DSP3 Attribute B charging function */
	0x28,	/* 0000001Dh: DSP3 Attribute C USB3 PHY */
	0x40,	/* 0000001Eh: DSP3 Attribute D USB2 PHY */
	0x0C,	/* 0000001Fh: DSP4 Attribute A DSP Setting */
	0xD7,	/* 00000020h: DSP4 Attribute B charging function */
	0x28,	/* 00000021h: DSP4 Attribute C USB3 PHY */
	0x40,	/* 00000022h: DSP4 Attribute D USB2 PHY */
	0x00,	/* 00000023h: PHY Modification for any register, Address
		 * MAC Modification for any register, Address0 */
	0x00,	/* 00000024h: PHY Modification for any register, Data0
		 *            MAC Modification for any register, Address1 */
	0x00,	/* 00000025h: PHY Modification for any register, Data1
		 *            MAC Modification for any register, Data */
	0x00,	/* 00000026h: eFuse Check sum, not including SWR/LDO */
	0x85,	/* 00000027h: SWR/LDO output */
	0x64,	/* 00000028h: ECO options 1 */
	0x0B,	/* 00000029h: ECO options 2 */
	0x60,	/* 0000002Ah: String Tag and valid bits */
	0x09,	/* 0000002Bh: Manufacture String */
	0x03, 'G', 'e', 'n', 'e', 'r', 'i', 'c',
	0x0C,	/* 00000034h: Product String */
	0x03, 'U', 'S', 'B', '3', '.', '0', ' ', 'H', 'u', 'b',
	0x0E,	/* 00000040h: Serial number */
	0x03,
	FW_COMPILED_YEAR_CHAR_3, FW_COMPILED_YEAR_CHAR_2,
	FW_COMPILED_YEAR_CHAR_1, FW_COMPILED_YEAR_CHAR_0,
	FW_COMPILED_MONTH_CHAR_H, FW_COMPILED_MONTH_CHAR_L,
	FW_COMPILED_DAY_CHAR_H, FW_COMPILED_DAY_CHAR_L,
	FW_COMPILED_HOUR_CHAR_H, FW_COMPILED_HOUR_CHAR_L,
	FW_COMPILED_MINUTE_CHAR_H, FW_COMPILED_MINUTE_CHAR_L,
	0x00, 0xC0, 0x19, 0xA0, /* 0000004Eh: Container ID */
	0x81, 0xC4, 0x71, 0x4D,
	0xD2, 0x13, 0x8A, 0x72,
	0x03, 0x09, 0x1D, 0x00,
	0x00,	/*0000005Eh: Reserved */
	0x00	/*0000005Fh: SMBus checksum */
};

void configure_rts5411(void)
{
	int ret, timeout;
	int remaining, len, data_offset;
	uint8_t *data = (uint8_t *)DLSETTING;
	/*
	 * Header length: 6 bytes
	 * Transfer data length: 24 bytes
	 */
	uint8_t buf[VENDOR_INFO_HEADER_LEN + VENDOR_INFO_XFER_LEN] = {0};
	uint8_t hub_status;

	/* setup the VPCA7 of PMIC to 1.2v (in pmic.c) */
	/* setup the VGP6 to 3.3v (in pmic.c) */
	/* setup the reset pin */
	if (GPIO_USB_HUB_HX3_RESET_L_PIN == 0) {
		printk(BIOS_DEBUG, "USB Hub Reset pin doesn't define!\n");
		RTS5411_HUB_ASSERT(0);
	}

	/*
	 * Pull RTS5411 VBUS_DET Low 10ms to reset RTS5411
	 * Delay 10ms (RTS5411_RESET_TIME)
	 */
	gpio_output(GPIO_USB_HUB_HX3_RESET_L_PIN, 0);
	mdelay(RTS5411_RESET_TIME);

	/*
	 * Pull RTS5411 VBUS_DET High and delay 200ms to wait RTS5411 ready to
	 * configure.
	 * Delay 200ms (RTS5411_RESET_TIME)
	 */
	gpio_output(GPIO_USB_HUB_HX3_RESET_L_PIN, 1);
	mdelay(RTS5411_RESET_TIME);

	/* Prepare the transfer */
	remaining = VENDOR_INFO_LENGTH;
	data_offset = 0;

	while (remaining > 0) {
		len = MIN(remaining, VENDOR_INFO_XFER_LEN);
		/*
		 * 28-byte Write
		 * BLOCK OUT Command : Send I2C data out, including 24 bytes
		 * configuration data
		 */
		buf[0] = 0x0F;              /* CMD code: 0x0F */
		buf[1] = len + 4;           /* Byte Cnt: len + 4 */
		buf[2] = 0x00;              /* CMD Type: 0x00 */
		buf[3] = data_offset;       /* Conf Addr L: data_offset */
		buf[4] = 0x00;              /* Conf Addr_H: 0x00 */
		buf[5] = len;               /* Data Cnt: len */
		memcpy(&buf[6], data, len); /* data: Configuration data */
		ret = i2c_write_raw(I2C_USB_HUB_CHANNEL, RTS5411_I2C_ADDR, buf,
				    sizeof(buf));

		/*
		 * Receive byte Command : Polling the SMBUS status until the
		 * result is ready(0x01) or timeout, the timeout is supposed
		 * to be decreased in other TIMER function.
		 * | Slave addr |           Data byte               |
		 * |    0xD5    | 0x00 or 0x01 (Not Ready or Ready) |
		 */
		timeout = SMBUS_TIMEOUT;

		do {
			ret = i2c_read_raw(I2C_USB_HUB_CHANNEL,
					   RTS5411_I2C_ADDR, &hub_status,
					   sizeof(hub_status));
			/* wait 1 ms */
			mdelay(RTS5411_I2C_SLEEP_TIME);
			timeout--;
		} while ((hub_status == SMBUS_NOT_READY) &&
			 (timeout != 0) && (ret != 0));

		if ((hub_status != SMBUS_READY) || (ret != 0))
			printk(BIOS_DEBUG, "RTS5411 transfer failed %d\n",
			       (int)hub_status);

		remaining -= VENDOR_INFO_XFER_LEN;
		data_offset += VENDOR_INFO_XFER_LEN;
		data += VENDOR_INFO_XFER_LEN;
	}
}
