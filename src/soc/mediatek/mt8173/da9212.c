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

#include <soc/mt8173.h>
#include <soc/i2c.h>
#include <soc/gpio.h>
#include <soc/da9212.h>
#include <mainboard/cust_gpio_boot.h>
#include <mainboard/cust_i2c.h>
#include <mainboard/cust_gpio_usage.h>
#include <console/console.h>

enum {
	DA9212_SLAVE_ADDR_WRITE = 0xD0,
	DA9212_SLAVE_ADDR_READ = 0xD1,
};

enum {
	IO_FAIL,
	IO_OK
};

unsigned int da9212_write_byte(unsigned char addr, unsigned char value);
unsigned int da9212_read_byte(unsigned char addr, unsigned char *dataBuffer);
unsigned int da9212_config_interface(unsigned char RegNum, unsigned char val,
                                     unsigned char MASK, unsigned char SHIFT);
unsigned int da9212_get_reg_value(unsigned int reg);
unsigned int da9212_read_interface(unsigned char RegNum, unsigned char *val,
                                   unsigned char mask, unsigned char shift);
int get_da9212_i2c_ch_num(void);
void da9212_hw_init(void);
int da9212_hw_component_detect(void);

#define da9212_print(fmt, args...) printk(BIOS_INFO, fmt, ##args);

unsigned int da9212_write_byte(unsigned char addr, unsigned char value)
{
	unsigned int ret_code = I2C_OK;

	ret_code = i2c_writeb(I2C_EXT_BUCK_CHANNEL, DA9212_SLAVE_ADDR_WRITE >> 1,
			      addr, value);

	if (ret_code == 0)
		return IO_OK; /* ok */
	else
		return IO_FAIL; /* fail */
}

unsigned int da9212_read_byte(unsigned char addr, unsigned char *dataBuffer)
{
	unsigned int ret_code = I2C_OK;

	ret_code = i2c_readb(I2C_EXT_BUCK_CHANNEL, DA9212_SLAVE_ADDR_WRITE >> 1,
			     addr, dataBuffer);
	/* da9212_print("%s: i2c_read: ret_code: %d\n", __func__, ret_code); */

	if (ret_code == 0)
		return IO_OK;  /* ok */
	else
		return IO_FAIL; /* fail */
}

unsigned int da9212_read_interface(unsigned char RegNum, unsigned char *val,
                                   unsigned char mask, unsigned char shift)
{
	unsigned char da9212_reg = 0;
	unsigned int ret = 0;

	ret = da9212_read_byte(RegNum, &da9212_reg);

	da9212_reg &= (mask << shift);
	*val = (da9212_reg >> shift);

	return ret;
}

unsigned int da9212_config_interface(unsigned char RegNum, unsigned char val,
                                     unsigned char mask, unsigned char shift)
{
	unsigned char da9212_reg = 0;
	unsigned int ret = 0;

	ret = da9212_read_byte(RegNum, &da9212_reg);

	da9212_reg &= ~(mask << shift);
	da9212_reg |= (val << shift);

	ret = da9212_write_byte(RegNum, da9212_reg);

	return ret;
}

unsigned int da9212_get_reg_value(unsigned int reg)
{
	unsigned int ret = 0;
	unsigned char reg_val = 0;

	ret = da9212_read_interface((unsigned char) reg, &reg_val, 0xFF, 0x0);

	return reg_val;
}

int get_da9212_i2c_ch_num(void)
{
	return I2C_EXT_BUCK_CHANNEL;
}

void ext_buck_en(int val)
{
	if (GPIO_EXT_BUCK_IC_EN_PIN != 0) {
		mt_set_gpio_mode(GPIO_EXT_BUCK_IC_EN_PIN, 0); /* 0:GPIO mode */
		mt_set_gpio_dir(GPIO_EXT_BUCK_IC_EN_PIN, 1); /* dir = output */
		mt_set_gpio_out(GPIO_EXT_BUCK_IC_EN_PIN, val);
	}
	if (GPIO_EXT_BUCK_EN_A_PIN != 0) {
		mt_set_gpio_mode(GPIO_EXT_BUCK_EN_A_PIN, 0); /* 0:GPIO mode */
		mt_set_gpio_dir(GPIO_EXT_BUCK_EN_A_PIN, 1); /* dir = output */
		mt_set_gpio_out(GPIO_EXT_BUCK_EN_A_PIN, val);
	}
	if (GPIO_EXT_BUCK_EN_B_PIN != 0) {
		mt_set_gpio_mode(GPIO_EXT_BUCK_EN_B_PIN, 0); /* 0:GPIO mode */
		mt_set_gpio_dir(GPIO_EXT_BUCK_EN_B_PIN, 1); /* dir = output */
		mt_set_gpio_out(GPIO_EXT_BUCK_EN_B_PIN, val);
	}
}

void da9212_hw_init(void)
{
	unsigned char reg_val = 0;
	unsigned int ret = 0;
	/* page select to 0 after one access */
	ret = da9212_config_interface(DA9212_REG_PAGE_CON,
	      0x0, 0xF, DA9212_PEG_PAGE_SHIFT);
	/* BUCKA_EN = 1 */
	ret = da9212_config_interface(DA9212_REG_BUCKA_CONT,
	      DA9212_BUCK_ON, 0x1, DA9212_BUCK_EN_SHIFT);
	/* BUCKB_EN = 1 */
	ret = da9212_config_interface(DA9212_REG_BUCKB_CONT,
	      DA9212_BUCK_ON, 0x1, DA9212_BUCK_EN_SHIFT);
	/* GPIO setting*/
	ret = da9212_config_interface(DA9212_REG_GPIO_0_1,
	      0x4, 0xF, DA9212_GPIO0_PIN_SHIFT);
	ret = da9212_config_interface(DA9212_REG_GPIO_0_1,
	      0x4, 0xF, DA9212_GPIO1_PIN_SHIFT);
	ret = da9212_config_interface(DA9212_REG_GPIO_2_3,
	      0x7, 0xF, DA9212_GPIO2_PIN_SHIFT);
	ret = da9212_config_interface(DA9212_REG_GPIO_2_3,
	      0x7, 0xF, DA9212_GPIO3_PIN_SHIFT);
	ret = da9212_config_interface(DA9212_REG_GPIO_4,
	      0x04, 0xFF, DA9212_GPIO4_PIN_SHIFT);
	/* BUCKA_GPI = GPIO0 */
	if (GPIO_EXT_BUCK_EN_A_PIN != 0)
		ret = da9212_config_interface(DA9212_REG_BUCKA_CONT,
		      0x01, 0x03, DA9212_BUCK_GPI_SHIFT);
	else
		ret = da9212_config_interface(DA9212_REG_BUCKA_CONT,
		      0x00, 0x03, DA9212_BUCK_GPI_SHIFT);

	/* BUCKB_GPI = GPIO1 */
	if (GPIO_EXT_BUCK_EN_B_PIN != 0)
		ret = da9212_config_interface(DA9212_REG_BUCKB_CONT,
		      0x02, 0x03, DA9212_BUCK_GPI_SHIFT);
	else
		ret = da9212_config_interface(DA9212_REG_BUCKB_CONT,
		      0x00, 0x03, DA9212_BUCK_GPI_SHIFT);

	ret = da9212_config_interface(DA9212_REG_BUCKA_CONT,
	      0x00, 0x01, DA9212_VBUCK_SEL_SHIFT); /* VBUCKA_A */
	ret = da9212_config_interface(DA9212_REG_BUCKB_CONT,
	      0x00, 0x01, DA9212_VBUCK_SEL_SHIFT); /* VBUCKB_A */
	/* VBUCKA_GPI = None */
	ret = da9212_config_interface(DA9212_REG_BUCKA_CONT,
	      0x00, 0x03, DA9212_VBUCK_GPI_SHIFT);
	/* VBUCKB_GPI = None */
	ret = da9212_config_interface(DA9212_REG_BUCKB_CONT,
	      0x00, 0x03, DA9212_VBUCK_GPI_SHIFT);

	ret = da9212_config_interface(DA9212_REG_PAGE_CON,
	      DA9212_REG_PAGE4, 0xF, DA9212_PEG_PAGE_SHIFT);
	da9212_read_interface((unsigned char)DA9212_VARIANT_ID, &reg_val, 0xFF, 0);
	ret = da9212_config_interface(DA9212_REG_PAGE_CON,
	      DA9212_REG_PAGE0, 0xF, DA9212_PEG_PAGE_SHIFT);
	if (reg_val == DA9212_VARIANT_ID_AB) {
		/* Disable force PWM mode (this is reserve register) */
		da9212_print("[da9212] 1DA9212_VARIANT_ID = 0x%x ", reg_val);
		ret = da9212_config_interface(DA9212_REG_BUCKA_CONF,
		      DA9212_BUCK_MODE_PWM, 0x3, DA9212_BUCK_MODE_SHIFT);
		/* Disable force PWM mode (this is reserve register) */
		ret = da9212_config_interface(DA9212_REG_BUCKB_CONF,
		      DA9212_BUCK_MODE_PWM, 0x3, DA9212_BUCK_MODE_SHIFT);
	} else {
		da9212_print("[da9212] 2DA9212_VARIANT_ID = 0x%x ", reg_val);
		/* Disable force AUTO mode (this is reserve register) */
		ret = da9212_config_interface(DA9212_REG_BUCKA_CONF,
		      DA9212_BUCK_MODE_AUTO, 0x3, DA9212_BUCK_MODE_SHIFT);
		/* Disable force AUTO mode (this is reserve register) */
		ret = da9212_config_interface(DA9212_REG_BUCKB_CONF,
		      DA9212_BUCK_MODE_AUTO, 0x3, DA9212_BUCK_MODE_SHIFT);
	}

	/* PWM mode/1.0V, Setting VBUCKA_A = 1.0V */
	ret = da9212_config_interface(DA9212_REG_VBUCKA_A,
	      0x46, 0xFF, DA9212_VBUCK_SHIFT);
	/* PWM mode/1.0V, Setting VBUCKA_B = 1.0V */
	ret = da9212_config_interface(DA9212_REG_VBUCKA_B,
	      0x46, 0xFF, DA9212_VBUCK_SHIFT);
	/* PWM mode/1.0V, Setting VBUCKB_A = 1.0V */
	ret = da9212_config_interface(DA9212_REG_VBUCKB_A,
	      0x46, 0xFF, DA9212_VBUCK_SHIFT);
	/* PWM mode/1.0V, Setting VBUCKB_B = 1.0V */
	ret = da9212_config_interface(DA9212_REG_VBUCKB_B,
	      0x46, 0xFF, DA9212_VBUCK_SHIFT);
}

int da9212_hw_component_detect(void)
{
	unsigned int ret = 0;
	unsigned char val = 0;

	/* select to page 2, clear REVERT at first time*/
	ret = da9212_config_interface(DA9212_REG_PAGE_CON,
	      0x2, 0xFF, DA9212_PEG_PAGE_SHIFT);

	ret = da9212_read_interface(0x5, &val, 0xF, 4);

	/* check default SPEC. value */
	if (val == 0xD)
		ret = IO_OK;
	else
		ret = IO_FAIL;

	da9212_print("%s: val = %d\n", __func__, val);

	return ret;
}

void da9212_driver_probe(void)
{
	int ret;

	ret = da9212_hw_component_detect();
	if (ret == IO_OK)
		da9212_hw_init();
	else
		da9212_print("[da9212_driver_probe] PL da9212 is not exist\n");
}
