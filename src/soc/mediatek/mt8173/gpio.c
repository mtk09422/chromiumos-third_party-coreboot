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
#include <console/console.h>
#include <arch/io.h>
#include <types.h>
#include <gpio.h>
#include <soc/gpio.h>
#include <soc/mt8173.h>
#include <soc/addressmap.h>
#include <soc/pmic_wrap_init.h>

static inline int gpioext_write (uint16_t * addr, s64 data)
{
	return pwrap_write((u32)(uintptr_t)addr, data);
}

static inline int gpioext_read (uint16_t *addr)
{
	u32 ext_data;
	int ret;

	ret = pwrap_read((u32)(uintptr_t)addr, &ext_data);

	return (ret != 0) ? 0xFFFFFFFF : ext_data;
}

static inline int gpioext_set_bits (u32 bit, uint16_t *reg)
{
	return gpioext_write(reg, bit);
}

static inline int gpioext_clr_bits (u32 bit, uint16_t *reg)
{
	u32 ext_data;

	int ret;
	ret = gpioext_read(reg);
	ext_data = ret;

	return (ret < 0) ? 0xFFFFFFFF : (gpioext_write(reg, ext_data & ~((u32)(bit))));
}

enum {
	RG_USBDL_EN_CTL_REG = 0x502,
};

enum {
	GPIO_FALSE = 0,
	GPIO_TRUE = 1,
};

enum {
	MAX_GPIO_REG_BITS = 16,

	MAX_GPIO_MODE_PER_REG = 5,

	GPIO_MODE_BITS = 3,
};

/* for special kpad pupd */
struct kpad_pupd {
	unsigned char pin;
	unsigned char reg;
	unsigned char bit;
};

static struct kpad_pupd kpad_pupd_spec[] = {
	{GPIO119,	0,	2},     // KROW0
	{GPIO120,	0,	6},     // KROW1
	{GPIO121,	0,	10},    // KROW2
	{GPIO122,	1,	2},     // KCOL0
	{GPIO123,	1,	6},     // KCOL1
	{GPIO124,	1,	10}     // KCOL2
};

/* for special msdc pupd */
enum {
	MSDC0_MIN_PIN = GPIO57,
	MSDC0_MAX_PIN = GPIO67,
	MSDC1_MIN_PIN = GPIO73,
	MSDC1_MAX_PIN = GPIO78,
	MSDC2_MIN_PIN = GPIO100,
	MSDC2_MAX_PIN = GPIO105,
	MSDC3_MIN_PIN = GPIO22,
	MSDC3_MAX_PIN = GPIO27,
};

static inline u32 get_msdc0_pin_cnt(void)
{
	return (MSDC0_MAX_PIN - MSDC0_MIN_PIN + 1);
}

static inline u32 get_msdc1_pin_cnt(void)
{
	return (MSDC1_MAX_PIN - MSDC1_MIN_PIN + 1);
}

static inline u32 get_msdc2_pin_cnt(void)
{
	return (MSDC2_MAX_PIN - MSDC2_MIN_PIN + 1);
}

static const struct msdc_pupd msdc_pupd_spec[]=
{
	/* msdc0 pin:GPIO57~GPIO67 */
	{0xC20, 0},// GPIO57
	{0xC20, 0},// GPIO58
	{0xC20, 0},// GPIO59
	{0xC20, 0},// GPIO60
	{0xC20, 0},// GPIO61
	{0xC20, 0},// GPIO62
	{0xC20, 0},// GPIO63
	{0xC20, 0},// GPIO64
	{0xC00, 0},// GPIO65
	{0xC10, 0},// GPIO66
	{0xD10, 0},// GPIO67
	{0xD00, 0},// GPIO68
	/* msdc1 pin:GPIO73~GPIO78 */
	{0xD20, 0},// GPIO73
	{0xD20, 4},// GPIO74
	{0xD20, 8},// GPIO75
	{0xD20, 12},// GPIO76
	{0xC40, 0},// GPIO77
	{0xC50, 0},// GPIO78
	/* msdc2 pin:GPIO100~GPIO105 */
	{0xD40, 0},// GPIO100
	{0xD40, 4},// GPIO101
	{0xD40, 8},// GPIO102
	{0xD40, 12},// GPIO103
	{0xC80, 0},// GPIO104
	{0xC90, 0},// GPIO105
	/* msdc3 pin */
	{0xD60, 0},// GPIO22
	{0xD60, 4},// GPIO23
	{0xD60, 8},// GPIO24
	{0xD60, 12},// GPIO25
	{0xCC0, 0},// GPIO26
	{0xCD0, 0},// GPIO27
};
#define PUPD_OFFSET_TO_REG(msdc_pupd_ctrl) ((VAL_REGS*)(uintptr_t)(GPIO_BASE + msdc_pupd_ctrl->reg))

static const struct msdc_pupd * mt_get_msdc_ctrl(unsigned long pin)
{
	unsigned int idx = 255;

	if ((pin >= MSDC0_MIN_PIN) && (pin <= MSDC0_MAX_PIN)){
		idx = pin - MSDC0_MIN_PIN;
	} else if ((pin >= MSDC1_MIN_PIN) && (pin <= MSDC1_MAX_PIN)){
		idx = get_msdc0_pin_cnt() + pin-MSDC1_MIN_PIN;// pin-94
	} else if ((pin >= MSDC2_MIN_PIN) && (pin <= MSDC2_MAX_PIN)){
		idx = get_msdc0_pin_cnt() + get_msdc1_pin_cnt() + pin-MSDC2_MIN_PIN;// pin-68
	} else if ((pin >= MSDC3_MIN_PIN) && (pin <= MSDC3_MAX_PIN)){
	    	idx = get_msdc0_pin_cnt() + get_msdc1_pin_cnt() + get_msdc2_pin_cnt() + pin-MSDC3_MIN_PIN;// pin-226
	} else {
		return NULL;
	}

	return &msdc_pupd_spec[idx];
}

struct mt_gpio_obj {
	GPIO_REGS *reg;
};

static struct mt_gpio_obj gpio_dat = {
	.reg = (GPIO_REGS *) (GPIO_BASE),
};

static struct mt_gpio_obj *gpio_obj = &gpio_dat;

struct mt_gpioext_obj {
	GPIOEXT_REGS	*reg;
};

static struct mt_gpioext_obj gpioext_dat = {
	.reg = (GPIOEXT_REGS *) (GPIOEXT_BASE),
};

static struct mt_gpioext_obj *gpioext_obj = &gpioext_dat;

static inline struct mt_gpio_obj * mt_get_gpio_obj(void)
{
	return gpio_obj;
}

static inline struct mt_gpioext_obj * mt_get_gpioext_obj(void)
{
	return gpioext_obj;
}
enum {
	GPIO_PRO_DIR = 0,
	GPIO_PRO_DOUT,
	GPIO_PRO_DIN,
	GPIO_PRO_PULLEN,
	GPIO_PRO_PULLSEL,
	GPIO_PRO_MODE,
	GPIO_PRO_MAX,
};

static s32 mt_get_kpad_pupd_spec (u32 pin)
{
	u32 i;

	for(i = 0; i < sizeof(kpad_pupd_spec)/sizeof(kpad_pupd_spec[0]); i++){
		if (pin == kpad_pupd_spec[i].pin)
			return i;
	}
	return -ERINVAL;
}

static s32 mt_set_gpio_chip(u32 pin, u32 property, u32 val)
{
	u32 pos = 0;
	u32 bit = 0;
	struct mt_gpio_obj *obj = mt_get_gpio_obj();
	uint16_t *reg;
	u32 data = 0;
	s32 i = 0;
	const struct msdc_pupd * msdc_pupd_ctrl;

	if (!obj)
		return -ERACCESS;

	if (pin >= GPIO_EXTEND_START)
		return -ERINVAL;

	if (property >= GPIO_PRO_MAX)
		return -ERINVAL;

        pos = pin / MAX_GPIO_REG_BITS;
        bit = pin % MAX_GPIO_REG_BITS;
	data = 1L << bit;

	switch (property)
	{
	case GPIO_PRO_DIR:
	        if (val == GPIO_DIR_IN)
			reg = &obj->reg->dir[pos].rst;
	        else
			reg = &obj->reg->dir[pos].set;
		break;
	case GPIO_PRO_DOUT:
		if (val == GPIO_OUT_ZERO)
			reg = &obj->reg->dout[pos].rst;
		else
			reg = &obj->reg->dout[pos].set;
		break;
	case GPIO_PRO_PULLEN:
		if ((i = mt_get_kpad_pupd_spec(pin)) >= 0){
			if (val == GPIO_PULL_DISABLE){
				reg = &obj->reg->kpad_ctrl[kpad_pupd_spec[i].reg].rst;
				data = 3L << (kpad_pupd_spec[i].bit - 2);
			} else {
				reg = &obj->reg->kpad_ctrl[kpad_pupd_spec[i].reg].set;
				data = 1L << (kpad_pupd_spec[i].bit - 2);
			}
		} else if (NULL != (msdc_pupd_ctrl = mt_get_msdc_ctrl(pin))) {
			if (val == GPIO_PULL_DISABLE)
			{
				reg = &((PUPD_OFFSET_TO_REG(msdc_pupd_ctrl))->rst);
				data = 3L << (msdc_pupd_ctrl->bit + R0R1);
			} else {
				reg = &((PUPD_OFFSET_TO_REG(msdc_pupd_ctrl))->set);
				data = 1L << (msdc_pupd_ctrl->bit + R0R1);
			}
		} else {
			if (val == GPIO_PULL_DISABLE)
                		reg = &obj->reg->pullen[pos].rst;
		        else
                		reg = &obj->reg->pullen[pos].set;
		}
		break;
	case GPIO_PRO_PULLSEL:
		if ((i = mt_get_kpad_pupd_spec(pin)) >= 0){
			if (val == GPIO_PULL_DOWN)
				reg = &obj->reg->kpad_ctrl[kpad_pupd_spec[i].reg].set;
			else
				reg = &obj->reg->kpad_ctrl[kpad_pupd_spec[i].reg].rst;
			data = 1L << kpad_pupd_spec[i].bit;

		} else if(NULL != (msdc_pupd_ctrl = mt_get_msdc_ctrl(pin)))
		{
			if (val == GPIO_PULL_DOWN)
				reg = &(PUPD_OFFSET_TO_REG(msdc_pupd_ctrl))->set;
			else
				reg = &(PUPD_OFFSET_TO_REG(msdc_pupd_ctrl))->rst;
			data = 1L << (msdc_pupd_ctrl->bit+PUPD);
		} else {
			if (val == GPIO_PULL_DOWN)
				reg = &obj->reg->pullsel[pos].rst;
			else
				reg = &obj->reg->pullsel[pos].set;
		}
		break;
	}
	write16(reg, data);

        return RSUCCESS;
}

static s32 mt_get_gpio_chip(u32 pin, u32 property)
{
	u32 pos = 0;
	u32 bit = 0;
	struct mt_gpio_obj *obj = mt_get_gpio_obj();
	uint16_t *reg;
	s32 data = 0;
	u32 mask = 0;
	s32 i = 0;
	const struct msdc_pupd * msdc_pupd_ctrl;

	if (!obj)
		return -ERACCESS;

	if (pin >= GPIO_EXTEND_START)
		return -ERINVAL;

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;
	mask = 1L << bit;

	switch(property)
	{
	case GPIO_PRO_DIR:
		reg = &obj->reg->dir[pos].val;
		break;
	case GPIO_PRO_DOUT:
		reg = &obj->reg->dout[pos].val;
		break;
	case GPIO_PRO_DIN:
		reg = &obj->reg->din[pos].val;
		break;
	case GPIO_PRO_PULLEN:
		if ((i = mt_get_kpad_pupd_spec(pin)) >= 0){
			data = read32(&obj->reg->kpad_ctrl[kpad_pupd_spec[i].reg].val);
			mask = 3L << (kpad_pupd_spec[i].bit-2);
			return (((data & mask) != 0) ? 0 : 1);
	    	} else if(NULL != (msdc_pupd_ctrl = mt_get_msdc_ctrl(pin)))
		{
			data = read32(&(PUPD_OFFSET_TO_REG(msdc_pupd_ctrl))->val);
			mask = 3L << (msdc_pupd_ctrl->bit + R0R1);
			return (((data & mask) != 0) ? 0 : 1);
		} else {
			reg = &obj->reg->pullen[pos].val;
		}
		break;
	case GPIO_PRO_PULLSEL:
		if ((i = mt_get_kpad_pupd_spec(pin)) >= 0){
			data = read32(&obj->reg->kpad_ctrl[kpad_pupd_spec[i].reg].val);

			return (((data & (1L << kpad_pupd_spec[i].bit)) != 0)? 0: 1);
		} else if(NULL != (msdc_pupd_ctrl = mt_get_msdc_ctrl(pin)))
		{
			data = read32(&((PUPD_OFFSET_TO_REG(msdc_pupd_ctrl))->val));

			return (((data & (1L << (msdc_pupd_ctrl->bit+PUPD))) != 0) ? 0 : 1);
		} else {
			reg  = &obj->reg->pullsel[pos].val;
		}
		break;
	}
	data = read32(reg);

	return (((data & mask) != 0) ? 1 : 0);
}

static s32 mt_set_gpio_mode_chip(u32 pin, u32 mode)
{
	u32 pos = 0;
	u32 bit = 0;
	u32 mask = (1L << GPIO_MODE_BITS) - 1;
	struct mt_gpio_obj *obj = mt_get_gpio_obj();
	u32 offset = 0;

	if (!obj)
		return -ERACCESS;

	if (pin >= GPIO_EXTEND_START)
		return -ERINVAL;

	if (mode >= GPIO_MODE_MAX)
		return -ERINVAL;

	pos = pin / MAX_GPIO_MODE_PER_REG;
	bit = pin % MAX_GPIO_MODE_PER_REG;

	offset = GPIO_MODE_BITS * bit;
	clrsetbits_le32(&obj->reg->mode[pos].val, mask << offset, mode << offset);

	return RSUCCESS;
}

static s32 mt_get_gpio_mode_chip(u32 pin)
{
	u32 pos = 0;
	u32 bit = 0;
	u32 data = 0;
	u32 mask = (1L << GPIO_MODE_BITS) - 1;
	struct mt_gpio_obj *obj = mt_get_gpio_obj();

	if (!obj)
		return -ERACCESS;

	if (pin >= GPIO_EXTEND_START)
		return -ERINVAL;

	pos = pin / MAX_GPIO_MODE_PER_REG;
	bit = pin % MAX_GPIO_MODE_PER_REG;

	data = read32(&obj->reg->mode[pos].val);

	return ((data >> (GPIO_MODE_BITS * bit)) & mask);
}

static s32 mt_set_gpio_ext(u32 pin, u32 property, u32 val)
{
	u32 pos = 0;
	u32 bit = 0;
	struct mt_gpioext_obj *obj = mt_get_gpioext_obj();
	uint16_t *reg;
	u32 data = 0;
	int ret = 0;

	if (!obj)
		return -ERACCESS;

	if (pin >= GPIO_EXTEND_START)
		return -ERINVAL;

	if (property >= GPIO_PRO_MAX)
		return -ERINVAL;

	pin -= GPIO_EXTEND_START;
	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	switch (property)
	{
	case GPIO_PRO_DIR:
	        if (val == GPIO_DIR_IN)
			reg = &obj->reg->dir[pos].rst;
	        else
			reg = &obj->reg->dir[pos].set;
		break;
	case GPIO_PRO_DOUT:
		if (val == GPIO_OUT_ZERO)
			reg = &obj->reg->dout[pos].rst;
		else
			reg = &obj->reg->dout[pos].set;
		break;
	case GPIO_PRO_PULLEN:
		if (val == GPIO_PULL_DISABLE)
               		reg = &obj->reg->pullen[pos].rst;
	        else
               		reg = &obj->reg->pullen[pos].set;
		break;
	case GPIO_PRO_PULLSEL:
		if (val == GPIO_PULL_DOWN)
			reg = &obj->reg->pullsel[pos].rst;
		else
			reg = &obj->reg->pullsel[pos].set;

		break;
	}
	data = (1L << bit);
	ret = gpioext_set_bits(data, reg);

	if(ret != 0) return -ERWRAPPER;

        return RSUCCESS;
}

static s32 mt_get_gpio_ext(u32 pin, u32 property)
{
	u32 pos = 0;
	u32 bit = 0;
	struct mt_gpioext_obj *obj = mt_get_gpioext_obj();
	uint16_t *reg = 0;
	s32 data = 0;
	u32 mask = 0;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	pin -= GPIO_EXTEND_START;
	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	switch(property)
	{
	case GPIO_PRO_DIR:
		reg = &obj->reg->dir[pos].val;
		break;
	case GPIO_PRO_DOUT:
		reg = &obj->reg->dout[pos].val;
		break;
	case GPIO_PRO_DIN:
		reg = &obj->reg->din[pos].val;
		break;
	case GPIO_PRO_PULLEN:
		reg = &obj->reg->pullen[pos].val;
		break;
	case GPIO_PRO_PULLSEL:
		reg = &obj->reg->pullsel[pos].val;
		break;
	}

	mask = 1L << bit;
	data = gpioext_read(reg);

	if(data < 0) return -ERWRAPPER;

	return (((data & mask) != 0) ? 1 : 0);
}

static s32 mt_set_gpio_mode_ext(u32 pin, u32 mode)
{
	u32 pos = 0;
	u32 bit = 0;
	u32 data = 0;
	s64 reg;
	int ret = 0;
	u32 mask = (1L << GPIO_MODE_BITS) - 1;
	struct mt_gpioext_obj *obj = mt_get_gpioext_obj();

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	if (mode >= GPIO_MODE_MAX)
		return -ERINVAL;

	if(pin == GPIOEXT12 || pin == GPIOEXT20) {
		if(mode == GPIO_MODE_00) {
			data = gpioext_read((uint16_t*)RG_USBDL_EN_CTL_REG);
			if(data < 0) return -ERWRAPPER;
			data = data & 0xFFFFFFFE;
			gpioext_write((uint16_t*)RG_USBDL_EN_CTL_REG, data);
		} else {
			data = gpioext_read((uint16_t*)RG_USBDL_EN_CTL_REG);
			if(data < 0) return -ERWRAPPER;
			data = data | 0x00000001;
			gpioext_write((uint16_t*)RG_USBDL_EN_CTL_REG, data);
		}
	}

	pin -= GPIO_EXTEND_START;
	pos = pin / MAX_GPIO_MODE_PER_REG;
	bit = pin % MAX_GPIO_MODE_PER_REG;


	reg = gpioext_read(&obj->reg->mode[pos].val);

	if(reg < 0) return -ERWRAPPER;

	reg &= ~(mask << (GPIO_MODE_BITS * bit));
	reg |= (mode << (GPIO_MODE_BITS * bit));

	ret = gpioext_write(&obj->reg->mode[pos].val, reg);

	if(ret != 0) return -ERWRAPPER;

	return RSUCCESS;
}

static s32 mt_get_gpio_mode_ext(u32 pin)
{
	u32 pos = 0;
	u32 bit = 0;
	s64 reg;
	u32 mask = (1L << GPIO_MODE_BITS) - 1;
	struct mt_gpioext_obj *obj = mt_get_gpioext_obj();

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	pin -= GPIO_EXTEND_START;
	pos = pin / MAX_GPIO_MODE_PER_REG;
	bit = pin % MAX_GPIO_MODE_PER_REG;

	reg = gpioext_read(&obj->reg->mode[pos].val);

	if(reg < 0) return -ERWRAPPER;

	return ((reg >> (GPIO_MODE_BITS * bit)) & mask);
}

static void mt_gpio_pin_decrypt(u32 * cipher)
{
	if ((*cipher & (0x80000000)) == 0) {
		printk(BIOS_ERR, "Pin %u decrypt warning! \n", *cipher);
	}
	*cipher &= ~(0x80000000);

	return;
}

s32 mt_set_gpio_dir(u32 pin, u32 dir)
{
	mt_gpio_pin_decrypt(&pin);
	u32 gp = GPIO_PRO_DIR;

	return (pin >= GPIO_EXTEND_START) ?
		mt_set_gpio_ext(pin, gp, dir) : mt_set_gpio_chip(pin, gp, dir);
}

s32 mt_get_gpio_dir(u32 pin)
{
	mt_gpio_pin_decrypt(&pin);
	u32 gp = GPIO_PRO_DIR;

	return (pin >= GPIO_EXTEND_START) ? mt_get_gpio_ext(pin, gp): mt_get_gpio_chip(pin, gp);
}

s32 mt_set_gpio_pull_enable(u32 pin, u32 enable)
{
	mt_gpio_pin_decrypt(&pin);
	u32 gp = GPIO_PRO_PULLEN;

	return (pin >= GPIO_EXTEND_START) ? mt_set_gpio_ext(pin, gp, enable): mt_set_gpio_chip(pin, gp, enable);
}

s32 mt_get_gpio_pull_enable(u32 pin)
{
	mt_gpio_pin_decrypt(&pin);
	u32 gp = GPIO_PRO_PULLEN;

	return (pin >= GPIO_EXTEND_START) ? mt_get_gpio_ext(pin, gp): mt_get_gpio_chip(pin, gp);
}

s32 mt_set_gpio_pull_select(u32 pin, u32 select)
{
	mt_gpio_pin_decrypt(&pin);
	u32 gp = GPIO_PRO_PULLSEL;

	return (pin >= GPIO_EXTEND_START) ? mt_set_gpio_ext(pin, gp, select): mt_set_gpio_chip(pin, gp, select);
}

s32 mt_get_gpio_pull_select(u32 pin)
{
	mt_gpio_pin_decrypt(&pin);
	u32 gp = GPIO_PRO_PULLSEL;

	return (pin >= GPIO_EXTEND_START) ? mt_get_gpio_ext(pin, gp): mt_get_gpio_chip(pin, gp);
}

s32 mt_set_gpio_out(u32 pin, u32 output)
{
	mt_gpio_pin_decrypt(&pin);
	u32 gp = GPIO_PRO_DOUT;

	return (pin >= GPIO_EXTEND_START) ? mt_set_gpio_ext(pin, gp, output): mt_set_gpio_chip(pin, gp, output);
}

s32 mt_get_gpio_out(u32 pin)
{
	mt_gpio_pin_decrypt(&pin);
	u32 gp = GPIO_PRO_DOUT;

	return (pin >= GPIO_EXTEND_START) ? mt_get_gpio_ext(pin, gp): mt_get_gpio_chip(pin, gp);
}

s32 mt_get_gpio_in(u32 pin)
{
	mt_gpio_pin_decrypt(&pin);
	u32 gp = GPIO_PRO_DIN;

	return (pin >= GPIO_EXTEND_START) ? mt_get_gpio_ext(pin, gp): mt_get_gpio_chip(pin, gp);
}

s32 mt_set_gpio_mode(u32 pin, u32 mode)
{
	mt_gpio_pin_decrypt(&pin);

	return (pin >= GPIO_EXTEND_START) ? mt_set_gpio_mode_ext(pin,mode): mt_set_gpio_mode_chip(pin,mode);
}

s32 mt_get_gpio_mode(u32 pin)
{
	mt_gpio_pin_decrypt(&pin);

	return (pin >= GPIO_EXTEND_START) ? mt_get_gpio_mode_ext(pin): mt_get_gpio_mode_chip(pin);
}

int gpio_get(gpio_t gpio)
{
	return mt_get_gpio_in(gpio);
}

void gpio_set(gpio_t gpio, int value)
{
	mt_set_gpio_out(gpio, value);
}

void gpio_input_pulldown(gpio_t gpio)
{
	mt_set_gpio_pull_select(gpio, GPIO_PULL_DOWN);
	mt_set_gpio_dir(gpio, GPIO_DIR_IN);
	mt_set_gpio_mode(gpio, GPIO_MODE_00);
}

void gpio_input_pullup(gpio_t gpio)
{
	mt_set_gpio_pull_select(gpio, GPIO_PULL_UP);
	mt_set_gpio_dir(gpio, GPIO_DIR_IN);
	mt_set_gpio_mode(gpio, GPIO_MODE_00);
}

void gpio_input(gpio_t gpio)
{
	mt_set_gpio_dir(gpio, GPIO_DIR_IN);
	mt_set_gpio_mode(gpio, GPIO_MODE_00);
}

void gpio_output(gpio_t gpio, int value)
{
	mt_set_gpio_out(gpio, value);
	mt_set_gpio_dir(gpio, GPIO_DIR_OUT);
	mt_set_gpio_mode(gpio, GPIO_MODE_00);
}

void mt_gpio_init(void)
{
	mt_gpio_set_default();
}
