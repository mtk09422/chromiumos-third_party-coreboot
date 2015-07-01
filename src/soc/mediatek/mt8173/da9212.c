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

#include <soc/i2c.h>
#include <gpio.h>
#include <soc/da9212.h>
#include <mainboard/cust_i2c.h>
#include <mainboard/cust_gpio_usage.h>
#include <console/console.h>

enum {
	DA9212_SLAVE_ADDR_WRITE = 0xD0,
	DA9212_SLAVE_ADDR_READ = 0xD1,
};

enum {
	EXT_BUCK_DISABLE,
	EXT_BUCK_ENABLE
};

#define da9212_print(fmt, args...) printk(BIOS_INFO, fmt, ##args);

static unsigned char da9212_read(unsigned char reg, unsigned char mask,
				 unsigned char shift)
{
	unsigned char da9212_reg = 0;
	unsigned int ret = 0;
	unsigned char val;

	ret = i2c_readb(I2C_EXTBUCK_CHANNEL, DA9212_SLAVE_ADDR_WRITE >> 1,
			reg, &da9212_reg);
	if (ret)
		da9212_print("%s: i2c_readb ret = %d\n", __func__, ret);

	da9212_reg &= (mask << shift);
	val = (da9212_reg >> shift);

	return val;
}

static void da9212_write(unsigned char reg, unsigned char val,
			 unsigned char mask, unsigned char shift)
{
	unsigned char da9212_reg = 0;
	unsigned int ret = 0;

	ret = i2c_readb(I2C_EXTBUCK_CHANNEL, DA9212_SLAVE_ADDR_WRITE >> 1,
			reg, &da9212_reg);
	if (ret) {
		da9212_print("%s: i2c_readb ret = %d\n", __func__, ret);
		return;
	}

	da9212_reg &= ~(mask << shift);
	da9212_reg |= (val << shift);

	ret = i2c_writeb(I2C_EXTBUCK_CHANNEL, DA9212_SLAVE_ADDR_WRITE >> 1,
			 reg, da9212_reg);

	if (ret) {
		da9212_print("%s: i2c_writeb ret = %d\n", __func__, ret);
		return;
	}

	return;
}

static void ext_buck_en(int val)
{
	if (GPIO_EXT_BUCK_IC_EN_PIN != 0) {
		gpio_output(GPIO_EXT_BUCK_IC_EN_PIN, val);
	}
	if (GPIO_EXT_BUCK_EN_A_PIN != 0) {
		gpio_output(GPIO_EXT_BUCK_EN_A_PIN, val);
	}
	if (GPIO_EXT_BUCK_EN_B_PIN != 0) {
		gpio_output(GPIO_EXT_BUCK_EN_B_PIN, val);
	}
}

static void da9212_hw_init(void)
{
	unsigned char reg_val = 0;

	/* page select to 0 after one access */
	da9212_write(DA9212_REG_PAGE_CON, 0x0, 0xF, DA9212_REG_PAGE_SHIFT);

	/* BUCKA_GPI = GPIO0 */
	if (GPIO_EXT_BUCK_EN_A_PIN != 0)
		da9212_write(DA9212_REG_BUCKA_CONT, 0x01, 0x03,
			     DA9212_BUCK_GPI_SHIFT);
	else
		da9212_write(DA9212_REG_BUCKA_CONT, 0x00, 0x03,
			     DA9212_BUCK_GPI_SHIFT);

	/* BUCKB_GPI = GPIO1 */
	if (GPIO_EXT_BUCK_EN_B_PIN != 0)
		da9212_write(DA9212_REG_BUCKB_CONT, 0x02, 0x03,
			     DA9212_BUCK_GPI_SHIFT);
	else
		da9212_write(DA9212_REG_BUCKB_CONT, 0x00, 0x03,
			     DA9212_BUCK_GPI_SHIFT);

	da9212_write(DA9212_REG_BUCKA_CONT, 0x00, 0x01,
		     DA9212_VBUCK_SEL_SHIFT); /* VBUCKA_A */
	da9212_write(DA9212_REG_BUCKB_CONT, 0x00, 0x01,
		     DA9212_VBUCK_SEL_SHIFT); /* VBUCKB_A */

	da9212_write(DA9212_REG_PAGE_CON, DA9212_REG_PAGE4, 0xF,
		     DA9212_REG_PAGE_SHIFT);

	reg_val = da9212_read((unsigned char)DA9212_VARIANT_ID, 0xFF, 0);

	da9212_write(DA9212_REG_PAGE_CON, DA9212_REG_PAGE0, 0xF,
		     DA9212_REG_PAGE_SHIFT);
	da9212_print("[da9212] DA9212_VARIANT_ID = 0x%x\n", reg_val);
	if (reg_val == DA9212_VARIANT_ID_AB) {
		/* Disable force PWM mode (this is reserve register) */
		da9212_write(DA9212_REG_BUCKA_CONF, DA9212_BUCK_MODE_PWM, 0x3,
			     DA9212_BUCK_MODE_SHIFT);
		/* Disable force PWM mode (this is reserve register) */
		da9212_write(DA9212_REG_BUCKB_CONF, DA9212_BUCK_MODE_PWM, 0x3,
			     DA9212_BUCK_MODE_SHIFT);
	} else {
		/* Disable force AUTO mode (this is reserve register) */
		da9212_write(DA9212_REG_BUCKA_CONF, DA9212_BUCK_MODE_AUTO, 0x3,
			     DA9212_BUCK_MODE_SHIFT);
		/* Disable force AUTO mode (this is reserve register) */
		da9212_write(DA9212_REG_BUCKB_CONF, DA9212_BUCK_MODE_AUTO, 0x3,
			     DA9212_BUCK_MODE_SHIFT);
	}

	ext_buck_en(EXT_BUCK_ENABLE);

}

void da9212_probe(void)
{
	unsigned char val = 0;

	/* select to page 4, clear REVERT at first time*/
	da9212_write(DA9212_REG_PAGE_CON, DA9212_REG_PAGE4, 0xF,
		     DA9212_REG_PAGE_SHIFT);

	val = da9212_read((unsigned char)DA9212_DEVICE_ID, 0xFF, 0);

	/* Check device ID is DA9212*/
	if (val != DA9212_ID) {
		da9212_print("%s: device_id = %d\n", __func__, val);
		return;
	}

	da9212_hw_init();
}
