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

#ifndef _CUST_I2C_H
#define _CUST_I2C_H


#define I2C_CHANNEL_0					0
#define I2C_CHANNEL_1					1
#define I2C_CHANNEL_2					2
#define I2C_CHANNEL_3					3
#define I2C_CHANNEL_4					4
#define I2C_CHANNEL_5					5


#define I2C_EXT_BUCK_AUTO_DETECT			FALSE
#define I2C_EXT_BUCK_CHANNEL				I2C_CHANNEL_1
#define I2C_EXT_BUCK_SLAVE_7_BIT_ADDR		0x68


#define I2C_SWITHING_CHARGER_AUTO_DETECT			FALSE
#define I2C_SWITHING_CHARGER_CHANNEL				I2C_CHANNEL_3
#define I2C_SWITHING_CHARGER_SLAVE_7_BIT_ADDR		0x6b


#define I2C_NFC_AUTO_DETECT			FALSE
#define I2C_NFC_CHANNEL				I2C_CHANNEL_0
#define I2C_NFC_SLAVE_7_BIT_ADDR		0x28


#endif /* _CUST_I2C_H */
