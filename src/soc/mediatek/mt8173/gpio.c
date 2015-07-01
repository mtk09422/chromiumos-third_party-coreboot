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
#include <assert.h>
#include <gpio.h>
#include <soc/gpio.h>
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
	{119,	0,	2},     // KROW0
	{120,	0,	6},     // KROW1
	{121,	0,	10},    // KROW2
	{122,	1,	2},     // KCOL0
	{123,	1,	6},     // KCOL1
	{124,	1,	10}     // KCOL2
};

static const struct msdc_pupd msdc_pupd_spec[]=
{
	/* msdc0 pin:GPIO57~GPIO67 */
	{57, 64, 0xC20, 0},// GPIO57
	{65, 65, 0xC00, 0},// GPIO65
	{66, 66, 0xC10, 0},// GPIO66
	{67, 67, 0xD10, 0},// GPIO67
	{68, 68, 0xD00, 0},// GPIO68
	/* msdc1 pin:GPIO73~GPIO78 */
	{73, 73, 0xD20, 0},// GPIO73
	{74, 74, 0xD20, 4},// GPIO74
	{75, 75, 0xD20, 8},// GPIO75
	{76, 76, 0xD20, 12},// GPIO76
	{77, 77, 0xC40, 0},// GPIO77
	{78, 78, 0xC50, 0},// GPIO78
	/* msdc2 pin:GPIO100~GPIO105 */
	{100, 100, 0xD40, 0},// GPIO100
	{101, 101, 0xD40, 4},// GPIO101
	{102, 102, 0xD40, 8},// GPIO102
	{103, 103, 0xD40, 12},// GPIO103
	{104, 104, 0xC80, 0},// GPIO104
	{105, 105, 0xC90, 0},// GPIO105
	/* msdc3 pin */
	{22, 22, 0xD60, 0},// GPIO22
	{23, 23, 0xD60, 4},// GPIO23
	{24, 24, 0xD60, 8},// GPIO24
	{25, 25, 0xD60, 12},// GPIO25
	{26, 26, 0xCC0, 0},// GPIO26
	{27, 27, 0xCD0, 0},// GPIO27
};
#define PUPD_OFFSET_TO_REG(msdc_pupd_ctrl) ((struct val_regs*)(uintptr_t)(GPIO_BASE + msdc_pupd_ctrl->reg))

static const struct msdc_pupd * mt_get_msdc_ctrl(unsigned long pin)
{
	unsigned int idx = 255;
	unsigned int spec_size = sizeof(msdc_pupd_spec)/sizeof(msdc_pupd_spec[0]);
	for(idx = 0; idx < spec_size; idx++){
		if(pin >= msdc_pupd_spec[idx].start && pin <= msdc_pupd_spec[idx].end)
			return &msdc_pupd_spec[idx];
	}
	return NULL;
}

static s32 mt_get_kpad_pupd_spec (u32 pin)
{
	u32 i;

	for(i = 0; i < sizeof(kpad_pupd_spec)/sizeof(kpad_pupd_spec[0]); i++){
		if (pin == kpad_pupd_spec[i].pin)
			return i;
	}
	return -1;
}

static void mt_pos_bit_calc(u32 pin, u32 *pos, u32 *bit)
{
	if (pin >= GPIO_EXTEND_START)
		pin = pin - GPIO_EXTEND_START;

	*pos = pin / MAX_GPIO_REG_BITS;
	*bit = pin % MAX_GPIO_REG_BITS;
}

static void mt_pos_bit_calc_mode(u32 pin, u32 *pos, u32 *bit)
{
	if (pin >= GPIO_EXTEND_START)
		pin = pin - GPIO_EXTEND_START;

	*pos = pin / MAX_GPIO_MODE_PER_REG;
	*bit = pin % MAX_GPIO_MODE_PER_REG;
}

static s32 mt_set_gpio_dir(u32 pin, u32 dir)
{
	u32 pos = 0;
	u32 bit = 0;
	uint16_t *reg;
	u32 data = 0;
	int ret = 0;

	mt_pos_bit_calc(pin, &pos, &bit);

	data = 1L << bit;

	if (pin <  GPIO_EXTEND_START) {
	        if (dir == 0)
			reg = &gpio_obj->dir[pos].rst;
	        else
			reg = &gpio_obj->dir[pos].set;
		write16(reg, data);
	} else {
	        if (dir == 0)
			reg = &gpioext_obj->dir[pos].rst;
	        else
			reg = &gpioext_obj->dir[pos].set;
		ret = gpioext_set_bits(data, reg);

		if(ret != 0) return -1;
	}

	return 0;
}

s32 mt_set_gpio_pull(u32 pin, u32 enable, u32 select)
{
	u32 pos = 0;
	u32 bit = 0;
	uint16_t *enReg, *selReg;
	u32 enData, selData = 0;
	s32 i = 0;
	int ret = 0;
	struct val_regs *regs;
	const struct msdc_pupd * msdc_pupd_ctrl;

	mt_pos_bit_calc(pin, &pos, &bit);

	if (pin <  GPIO_EXTEND_START) {
		if ((i = mt_get_kpad_pupd_spec(pin)) >= 0){
			regs = &gpio_obj->kpad_ctrl[kpad_pupd_spec[i].reg];
			if (enable == 0){
				enReg = &regs->rst;
				enData = 3L << (kpad_pupd_spec[i].bit - 2);
			} else {
				enReg = &regs->set;
				enData = 1L << (kpad_pupd_spec[i].bit - 2);
				selReg = (select == 0) ? (&regs->set): (&regs->rst);
				selData = 1L << kpad_pupd_spec[i].bit;
			}
		} else if (NULL != (msdc_pupd_ctrl = mt_get_msdc_ctrl(pin))) {
			regs = PUPD_OFFSET_TO_REG(msdc_pupd_ctrl);
			if (enable == 0) {
				enReg = &(regs->rst);
				enData = 3L << (msdc_pupd_ctrl->bit + R0R1);
			} else {
				enReg = &(regs->set);
				enData = 1L << (msdc_pupd_ctrl->bit + R0R1);
				selReg = (select == 0) ? (&regs->set):(&regs->rst);
				selData = 1L << (msdc_pupd_ctrl->bit + PUPD);
			}
		} else {
			if (enable == 0) {
				enReg = &gpio_obj->pullen[pos].rst;
			} else {
				enReg = &gpio_obj->pullen[pos].set;
				selReg = (select == 0) ? (&gpio_obj->pullsel[pos].rst) :
					 (&gpio_obj->pullsel[pos].set);
				enData = selData = 1L << bit;
			}
		}
		write16(enReg, enData);
		if (enable != 0)
			write16(selReg, selData);
	} else {

		if (enable == 0) {
			enReg = &gpioext_obj->pullen[pos].rst;
	        } else {
			enReg = &gpioext_obj->pullen[pos].set;
			selReg = (select== 0) ? (&gpioext_obj->pullsel[pos].rst) :
				(&gpioext_obj->pullsel[pos].set);
			enData = selData = 1L << bit;
		}
		ret = gpioext_set_bits(enData, enReg);

		if(ret != 0) return -1;

		if (enable != 0)
			ret = gpioext_set_bits(selData, selReg);

		if(ret != 0) return -1;
	}

	return 0;
}

static s32 mt_get_gpio_in(u32 pin)
{
	u32 pos = 0;
	u32 bit = 0;
	uint16_t *reg;
	s32 data = 0;
	u32 mask = 0;

	mt_pos_bit_calc(pin, &pos, &bit);
	mask = 1L << bit;

	if (pin <  GPIO_EXTEND_START) {
		reg = &gpio_obj->din[pos].val;
		data = read32(reg);
	} else {
		reg = &gpioext_obj->din[pos].val;
		data = gpioext_read(reg);

		if(data < 0) return -1;
	}

	return (((data & mask) != 0) ? 1 : 0);
}

static s32 mt_set_gpio_out(u32 pin, u32 output)
{
	u32 pos = 0;
	u32 bit = 0;
	uint16_t *reg;
	u32 data = 0;
	int ret = 0;

	mt_pos_bit_calc(pin, &pos, &bit);

	data = 1L << bit;

	if (pin <  GPIO_EXTEND_START) {
		if (output == 0)
			reg = &gpio_obj->dout[pos].rst;
		else
			reg = &gpio_obj->dout[pos].set;
		write16(reg, data);
	} else {

		if (output == 0)
			reg = &gpioext_obj->dout[pos].rst;
		else
			reg = &gpioext_obj->dout[pos].set;
		ret = gpioext_set_bits(data, reg);

		if(ret != 0) return -1;
	}

	return 0;
}

static s32 mt_set_gpio_mode(u32 pin, u32 mode)
{
	u32 pos = 0;
	u32 bit = 0;
	u32 mask = (1L << GPIO_MODE_BITS) - 1;
	u32 offset = 0;
	uint16_t *reg;
	u32 data = 0;
	int ret = 0;

	if(pin ==  (GPIO_EXTEND_START + 12) ||
		pin ==  (GPIO_EXTEND_START + 20)) {
		data = gpioext_read((uint16_t*)RG_USBDL_EN_CTL_REG);
		data = (mode == 0)? (data & (~(0x1 << 0))) : (data | (0x1 << 0));
		gpioext_write((uint16_t*)RG_USBDL_EN_CTL_REG, data);
	}

	mt_pos_bit_calc_mode(pin, &pos, &bit);

	offset = GPIO_MODE_BITS * bit;

	if (pin < GPIO_EXTEND_START) {
		clrsetbits_le32(&gpio_obj->mode[pos].val, mask << offset, mode << offset);
	} else {
		data = gpioext_read(&gpioext_obj->mode[pos].val);

		if(reg < 0) return -1;

		data &= ~(mask << (offset));
		data |= (mode << (offset));

		ret = gpioext_write(&gpioext_obj->mode[pos].val, data);

		if(ret != 0) return -1;
	}

	return 0;
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
	mt_set_gpio_pull(gpio, 1, 0);
	mt_set_gpio_dir(gpio, 0);
	mt_set_gpio_mode(gpio, 0);
}

void gpio_input_pullup(gpio_t gpio)
{
	mt_set_gpio_pull(gpio, 1, 1);
	mt_set_gpio_dir(gpio, 0);
	mt_set_gpio_mode(gpio, 0);
}

void gpio_input(gpio_t gpio)
{
	mt_set_gpio_pull(gpio, 0, 0);
	mt_set_gpio_dir(gpio, 0);
	mt_set_gpio_mode(gpio, 0);
}

void gpio_output(gpio_t gpio, int value)
{
	mt_set_gpio_out(gpio, value);
	mt_set_gpio_dir(gpio, 1);
	mt_set_gpio_mode(gpio, 0);
}

void mt_gpio_init(void)
{
	mt_gpio_set_default();
}
