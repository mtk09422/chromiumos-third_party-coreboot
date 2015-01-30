/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 MediaTek Inc.
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
#include <soc/gpio.h>
#include <soc/mt8135.h>
#include <soc/pmic_wrap_init.h>

/*
 * MACRO Definition
 */

#ifndef s64
#define s64 signed long long
#endif

#define GPIO_DEVICE "mt-gpio"
#define VERSION     "$Revision$"
#define GPIO_SET_BITS(BIT, REG)   ((*(volatile u16 *)(REG)) = (u32)(BIT))

static inline int gpioext_write(void *addr, s64 data)
{
	return pwrap_write((u32)addr, data);
}

static inline int gpioext_read(void *addr)
{
	u32 ext_data;
	int __ret;

	__ret = pwrap_read((u32)addr, &ext_data);

	return (__ret != 0) ? -1 : ext_data;
}

#define GPIOEXT_SET_BITS(BIT, REG)   (gpioext_write(REG, (u32)(BIT)))

static inline int gpioext_set_bits(u32 bit, void *reg)
{
	return gpioext_write(reg, bit);
}

static inline int gpioext_clr_bits(u32 bit, void *reg)
{
	u32 ext_data;
	int __ret;

	__ret = gpioext_read(reg);
	ext_data = __ret;

	return (__ret < 0) ? 0xFFFFFFFF : (gpioext_write(reg, ext_data & ~((u32)(bit))));
}

#define GPIOEXT_BASE        (0xC000)	/* PMIC GPIO base. */
#define TRUE                   1
#define FALSE                  0
#define MAX_GPIO_REG_BITS      16
#define MAX_GPIO_MODE_PER_REG  5
#define GPIO_MODE_BITS         3
#define GPIOTAG                "[GPIO] "
#define GPIOLOG(fmt, arg...)   printk(BIOS_INFO, GPIOTAG fmt, ##arg)
#define GPIOMSG(fmt, arg...)   printk(BIOS_INFO, fmt, ##arg)
#define GPIOERR(fmt, arg...)   printk(BIOS_INFO, GPIOTAG "%5d: "fmt, __LINE__, ##arg)
#define GIO_INVALID_OBJ(ptr)   ((ptr) != gpio_obj)

#define GPIO_DRV_BASE1					  0x10005500

#define GPIO_DRV_BASE2					  0x1020c510

/*
 * Enumeration/Structure
 */
#define CLK_NUM 6
static u32 clkout_reg_addr[CLK_NUM] = {
	(0xF0001A00),
	(0xF0001A04),
	(0xF0001A08),
	(0xF0001A0C),
	(0xF0001A10),
	(0xF0001A14)
};

struct mt_gpio_obj {
	GPIO_REGS *reg;
};

struct mt_gpioext_obj {
	GPIOEXT_REGS *reg;
};

static struct mt_gpio_obj gpio_dat = {
	.reg = (GPIO_REGS *)(GPIO_BASE),
};

static struct mt_gpio_obj gpio1_dat = {
	.reg = (GPIO_REGS *)(GPIO1_BASE),
};

static struct mt_gpioext_obj gpioext_dat = {
	.reg = (GPIOEXT_REGS *)(GPIOEXT_BASE),
};

static struct mt_gpio_obj *gpio_obj = &gpio_dat;
static struct mt_gpio_obj *gpio1_obj = &gpio1_dat;
static struct mt_gpioext_obj *gpioext_obj = &gpioext_dat;

typedef struct PIN_DRV_GROUP {
	u32 max_driving;
	u32 min_driving;
	u32 step;
} Pin_Drv_Group;

Pin_Drv_Group pin_group[5] = {
	{16, 2, 2},
	{16, 4, 4},
	{8, 2, 2},
	{32, 4, 4},
	{0, 0, 0}
};

typedef struct ADDRESS_PIN_T {
	u32 pinnum;
	u32 regaddress;
	u32 flag;
	u32 pingroup;
} Address_Pin;

Address_Pin address_pin[203] = {
	{0, GPIO_DRV_BASE1, 0, 0},
	{1, GPIO_DRV_BASE1, 0, 0},
	{2, GPIO_DRV_BASE1, 0, 0},
	{3, GPIO_DRV_BASE1, 0, 0},
	{4, GPIO_DRV_BASE1, 4, 0},
	{5, GPIO_DRV_BASE1, 8, 0},
	{6, GPIO_DRV_BASE1, 0, 0},
	{7, GPIO_DRV_BASE1, 0, 0},
	{8, GPIO_DRV_BASE1, 0, 0},
	{9, GPIO_DRV_BASE1, 0, 0},
	{10, GPIO_DRV_BASE1, 12, 1},
	{11, GPIO_DRV_BASE1, 12, 1},
	{12, GPIO_DRV_BASE1, 12, 1},
	{13, GPIO_DRV_BASE1, 12, 1},
	{14, GPIO_DRV_BASE1, 12, 1},
	{15, GPIO_DRV_BASE1, 12, 1},
	{16, GPIO_DRV_BASE1, 12, 1},
	{17, GPIO_DRV_BASE1, 16, 1},
	{18, GPIO_DRV_BASE1, 16, 1},
	{19, GPIO_DRV_BASE1, 16, 1},
	{20, GPIO_DRV_BASE1, 16, 1},
	{21, GPIO_DRV_BASE1, 16, 1},
	{22, GPIO_DRV_BASE1, 16, 1},
	{23, GPIO_DRV_BASE1, 16, 1},
	{24, GPIO_DRV_BASE1, 16, 1},
	{25, GPIO_DRV_BASE1, 20, 1},
	{26, GPIO_DRV_BASE1, 20, 1},
	{27, GPIO_DRV_BASE1, 20, 1},
	{28, GPIO_DRV_BASE1, 20, 1},
	{29, GPIO_DRV_BASE1, 20, 1},
	{30, GPIO_DRV_BASE1, 20, 1},
	{31, GPIO_DRV_BASE1, 20, 1},
	{32, GPIO_DRV_BASE1, 20, 1},
	{33, GPIO_DRV_BASE1, 24, 1},
	{34, GPIO_DRV_BASE2, 12, 2},
	{35, GPIO_DRV_BASE2, 16, 4},
	{36, GPIO_DRV_BASE2, 16, 4},
	{37, GPIO_DRV_BASE2, 20, 1},
	{38, GPIO_DRV_BASE2, 20, 1},
	{39, GPIO_DRV_BASE2, 20, 1},
	{40, GPIO_DRV_BASE2, 24, 1},
	{41, GPIO_DRV_BASE2, 24, 1},
	{42, GPIO_DRV_BASE2, 24, 1},
	{43, GPIO_DRV_BASE2, 28, 1},
	{44, GPIO_DRV_BASE2, 28, 1},
	{45, GPIO_DRV_BASE2, 28, 1},
	{46, GPIO_DRV_BASE2, 28, 1},
	{47, GPIO_DRV_BASE2, 28, 1},
	{48, GPIO_DRV_BASE2, 16, 4},
	{49, GPIO_DRV_BASE2 + 0x10, 0, 1},
	{50, GPIO_DRV_BASE2 + 0x10, 4, 1},
	{51, GPIO_DRV_BASE2 + 0x10, 8, 1},
	{52, GPIO_DRV_BASE2 + 0x10, 12, 2},
	{53, GPIO_DRV_BASE2 + 0x10, 16, 1},
	{54, GPIO_DRV_BASE2 + 0x10, 20, 1},
	{55, GPIO_DRV_BASE2 + 0x10, 24, 1},
	{56, GPIO_DRV_BASE2 + 0x10, 28, 1},
	{57, GPIO_DRV_BASE2 + 0x20, 0, 1},
	{58, GPIO_DRV_BASE2 + 0x20, 0, 1},
	{59, GPIO_DRV_BASE2 + 0x20, 0, 1},
	{60, GPIO_DRV_BASE2 + 0x20, 0, 1},
	{61, GPIO_DRV_BASE2 + 0x20, 0, 1},
	{62, GPIO_DRV_BASE2 + 0x20, 0, 1},
	{63, GPIO_DRV_BASE2 + 0x20, 4, 1},
	{64, GPIO_DRV_BASE2 + 0x20, 8, 1},
	{65, GPIO_DRV_BASE2 + 0x20, 12, 1},
	{66, GPIO_DRV_BASE2 + 0x20, 16, 1},
	{67, GPIO_DRV_BASE2 + 0x20, 20, 1},
	{68, GPIO_DRV_BASE2 + 0x20, 24, 1},
	{69, GPIO_DRV_BASE2 + 0x20, 28, 1},
	{70, GPIO_DRV_BASE2 + 0x30, 0, 1},
	{71, GPIO_DRV_BASE2 + 0x30, 4, 1},
	{72, GPIO_DRV_BASE2 + 0x30, 8, 1},
	{73, GPIO_DRV_BASE2 + 0x30, 12, 1},
	{74, GPIO_DRV_BASE2 + 0x30, 16, 1},
	{75, GPIO_DRV_BASE2 + 0x30, 20, 1},
	{76, GPIO_DRV_BASE2 + 0x30, 24, 1},
	{77, GPIO_DRV_BASE2 + 0x30, 28, 3},
	{78, GPIO_DRV_BASE2 + 0x30, 28, 3},
	{79, GPIO_DRV_BASE2 + 0x40, 0, 3},
	{80, GPIO_DRV_BASE2 + 0x40, 4, 3},
	{81, GPIO_DRV_BASE2 + 0x30, 28, 3},
	{82, GPIO_DRV_BASE2 + 0x30, 28, 3},
	{83, GPIO_DRV_BASE2 + 0x40, 8, 3},
	{84, GPIO_DRV_BASE2 + 0x40, 8, 3},
	{85, GPIO_DRV_BASE2 + 0x40, 12, 3},
	{86, GPIO_DRV_BASE2 + 0x40, 16, 3},
	{87, GPIO_DRV_BASE2 + 0x40, 8, 3},
	{88, GPIO_DRV_BASE2 + 0x40, 8, 3},
	{89, GPIO_DRV_BASE2 + 0x50, 12, 0},
	{90, GPIO_DRV_BASE2 + 0x50, 12, 0},
	{91, GPIO_DRV_BASE2 + 0x50, 12, 0},
	{92, GPIO_DRV_BASE2 + 0x50, 12, 0},
	{93, GPIO_DRV_BASE2 + 0x50, 12, 0},
	{94, GPIO_DRV_BASE2 + 0x50, 12, 0},
	{95, GPIO_DRV_BASE2 + 0x50, 12, 0},
	{96, GPIO_DRV_BASE1 + 0xb0, 28, 0},
	{97, GPIO_DRV_BASE2 + 0x50, 12, 0},
	{98, GPIO_DRV_BASE2 + 0x50, 16, 0},
	{99, GPIO_DRV_BASE2 + 0x50, 20, 1},
	{100, GPIO_DRV_BASE2, 16, 4},
	{101, GPIO_DRV_BASE2, 16, 4},
	{102, GPIO_DRV_BASE2 + 0x50, 24, 1},
	{103, GPIO_DRV_BASE2 + 0x50, 28, 1},
	{104, GPIO_DRV_BASE2 + 0x60, 0, 1},
	{105, GPIO_DRV_BASE2 + 0x60, 4, 1},
	{106, GPIO_DRV_BASE2 + 0x60, 4, 1},
	{107, GPIO_DRV_BASE2 + 0x60, 4, 1},
	{108, GPIO_DRV_BASE2 + 0x60, 4, 1},
	{109, GPIO_DRV_BASE2 + 0x60, 8, 2},
	{110, GPIO_DRV_BASE2 + 0x60, 12, 2},
	{111, GPIO_DRV_BASE2 + 0x60, 16, 2},
	{112, GPIO_DRV_BASE2 + 0x60, 20, 2},
	{113, GPIO_DRV_BASE2 + 0x60, 24, 2},
	{114, GPIO_DRV_BASE2 + 0x60, 28, 2},
	{115, GPIO_DRV_BASE2 + 0x70, 0, 2},
	{116, GPIO_DRV_BASE2 + 0x70, 4, 2},
	{117, GPIO_DRV_BASE2 + 0x70, 8, 2},
	{118, GPIO_DRV_BASE2 + 0x70, 12, 2},
	{119, GPIO_DRV_BASE2 + 0x70, 16, 2},
	{120, GPIO_DRV_BASE2 + 0x70, 20, 2},
	{121, GPIO_DRV_BASE2 + 0x70, 24, 1},
	{122, GPIO_DRV_BASE2 + 0x70, 24, 1},
	{123, GPIO_DRV_BASE2 + 0x80, 0, 1},
	{124, GPIO_DRV_BASE2 + 0x80, 0, 1},
	{125, GPIO_DRV_BASE2 + 0x80, 4, 1},
	{126, GPIO_DRV_BASE2 + 0x80, 0, 1},
	{127, GPIO_DRV_BASE2 + 0x70, 24, 1},
	{128, GPIO_DRV_BASE2 + 0x70, 24, 1},
	{129, GPIO_DRV_BASE2 + 0x70, 28, 1},
	{130, GPIO_DRV_BASE2 + 0x70, 28, 1},
	{131, GPIO_DRV_BASE2 + 0x70, 28, 1},
	{132, GPIO_DRV_BASE2 + 0x70, 28, 1},
	{133, GPIO_DRV_BASE2 + 0x70, 28, 1},
	{134, GPIO_DRV_BASE2 + 0x70, 28, 1},
	{135, GPIO_DRV_BASE2 + 0x70, 28, 1},
	{136, GPIO_DRV_BASE2 + 0x70, 28, 1},
	{137, GPIO_DRV_BASE2 + 0x80, 0, 1},
	{138, GPIO_DRV_BASE2 + 0x80, 0, 1},
	{139, GPIO_DRV_BASE2 + 0x80, 0, 1},
	{140, GPIO_DRV_BASE2 + 0x80, 0, 1},
	{141, GPIO_DRV_BASE2 + 0x80, 0, 1},
	{142, GPIO_DRV_BASE2 + 0x80, 4, 1},
	{143, GPIO_DRV_BASE2 + 0x80, 4, 1},
	{144, GPIO_DRV_BASE2 + 0x80, 4, 1},
	{145, GPIO_DRV_BASE2 + 0x80, 4, 1},
	{146, GPIO_DRV_BASE2 + 0x80, 4, 1},
	{147, GPIO_DRV_BASE2 + 0x80, 4, 1},
	{148, GPIO_DRV_BASE2 + 0x80, 4, 1},
	{149, GPIO_DRV_BASE1 + 0xa0, 4, 4},
	{150, GPIO_DRV_BASE1 + 0xa0, 4, 4},
	{151, GPIO_DRV_BASE1 + 0xa0, 4, 4},
	{152, GPIO_DRV_BASE1 + 0xa0, 4, 4},
	{153, GPIO_DRV_BASE1 + 0xa0, 4, 4},
	{154, GPIO_DRV_BASE1 + 0xa0, 4, 4},
	{155, GPIO_DRV_BASE1 + 0xa0, 4, 4},
	{156, GPIO_DRV_BASE1 + 0xa0, 4, 4},
	{157, GPIO_DRV_BASE1 + 0xa0, 4, 4},
	{158, GPIO_DRV_BASE1 + 0xa0, 4, 4},
	{159, GPIO_DRV_BASE1 + 0xa0, 8, 4},
	{160, GPIO_DRV_BASE1 + 0xa0, 8, 4},
	{161, GPIO_DRV_BASE1 + 0xa0, 8, 4},
	{162, GPIO_DRV_BASE1 + 0xa0, 8, 4},
	{163, GPIO_DRV_BASE1 + 0xa0, 8, 4},
	{164, GPIO_DRV_BASE1 + 0xa0, 8, 4},
	{165, GPIO_DRV_BASE1 + 0xa0, 8, 4},
	{166, GPIO_DRV_BASE1 + 0xa0, 8, 4},
	{167, GPIO_DRV_BASE1 + 0xa0, 8, 4},
	{168, GPIO_DRV_BASE1 + 0xa0, 8, 4},
	{169, GPIO_DRV_BASE2, 16, 4},
	{170, GPIO_DRV_BASE2, 16, 4},
	{171, GPIO_DRV_BASE2, 16, 4},
	{172, GPIO_DRV_BASE2, 16, 4},
	{173, GPIO_DRV_BASE2, 16, 4},
	{174, GPIO_DRV_BASE2, 16, 4},
	{175, GPIO_DRV_BASE2, 16, 4},
	{176, GPIO_DRV_BASE2, 16, 4},
	{177, GPIO_DRV_BASE2, 16, 4},
	{178, GPIO_DRV_BASE2, 16, 4},
	{179, GPIO_DRV_BASE2, 16, 4},
	{180, GPIO_DRV_BASE2, 16, 4},
	{181, GPIO_DRV_BASE1 + 0xa0, 12, 1},
	{182, GPIO_DRV_BASE1 + 0xa0, 16, 1},
	{183, GPIO_DRV_BASE1 + 0xa0, 20, 1},
	{184, GPIO_DRV_BASE1 + 0xa0, 24, 1},
	{185, GPIO_DRV_BASE1 + 0xa0, 28, 1},
	{186, GPIO_DRV_BASE1 + 0xb0, 0, 2},
	{187, GPIO_DRV_BASE1 + 0xb0, 0, 2},
	{188, GPIO_DRV_BASE1 + 0xb0, 0, 2},
	{189, GPIO_DRV_BASE1 + 0xb0, 0, 2},
	{190, GPIO_DRV_BASE1 + 0xb0, 4, 1},
	{191, GPIO_DRV_BASE1 + 0xb0, 8, 1},
	{192, GPIO_DRV_BASE1 + 0xb0, 12, 1},
	{193, GPIO_DRV_BASE2, 16, 4},
	{194, GPIO_DRV_BASE2, 16, 4},
	{195, GPIO_DRV_BASE2, 16, 4},
	{196, GPIO_DRV_BASE2, 16, 4},
	{197, GPIO_DRV_BASE1 + 0xb0, 16, 0},
	{198, GPIO_DRV_BASE1 + 0xb0, 16, 0},
	{199, GPIO_DRV_BASE1 + 0xb0, 20, 0},
	{200, GPIO_DRV_BASE1 + 0xb0, 24, 0},
	{201, GPIO_DRV_BASE1 + 0xb0, 16, 0},
	{202, GPIO_DRV_BASE1 + 0xb0, 16, 0}
};

static s32 mt_set_gpio_dir_chip(u32 pin, u32 dir)
{
	u32 pos = 0;
	u32 bit = 0;
	struct mt_gpio_obj *obj = gpio_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	if (dir >= GPIO_DIR_MAX)
		return -ERINVAL;

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	if (dir == GPIO_DIR_IN) {
		GPIO_SET_BITS((1L << bit), (u32)(&obj->reg->dir[pos].rst));
	} else {
		GPIO_SET_BITS((1L << bit), (u32)(&obj->reg->dir[pos].set));
	}

	return RSUCCESS;
}

static s32 mt_get_gpio_dir_chip(u32 pin)
{
	u32 pos = 0;
	u32 bit = 0;
	u32 reg;
	struct mt_gpio_obj *obj = gpio_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	reg = read16(&obj->reg->dir[pos].val);

	return (((reg & (1L << bit)) != 0) ? 1 : 0);
}

static s32 mt_set_gpio_pull_enable_chip(u32 pin, u32 enable)
{
	u32 pos = 0;
	u32 bit = 0;

	struct mt_gpio_obj *obj = gpio_obj;
	/* GPIO34~GPIO148 BASE_ADDRESS is different with others */
	if ((pin >= 34) && (pin <= 148)) {
		if (enable == GPIO_PULL_DISABLE)
			GPIO_SET_BITS((1L << bit),
				      (u32)(&obj->reg->pullen[pos].rst) +
				      GPIO_OFFSET);
		else
			GPIO_SET_BITS((1L << bit),
				      (u32)(&obj->reg->pullen[pos].set) +
				      GPIO_OFFSET);

	} else {
		if (enable == GPIO_PULL_DISABLE)
			GPIO_SET_BITS((1L << bit),
				      (u32)(&obj->reg->pullen[pos].rst));
		else
			GPIO_SET_BITS((1L << bit),
				      (u32)(&obj->reg->pullen[pos].set));
	}

	return RSUCCESS;
}

static s32 mt_get_gpio_pull_enable_chip(u32 pin)
{
	u32 pos = 0;
	u32 bit = 0;
	u32 reg;
	struct mt_gpio_obj *obj = gpio_obj;

	if ((pin >= 34) && (pin <= 148)) {
		reg =
		    read16((&obj->reg->pullen[pos].val) + GPIO_OFFSET);
	} else {
		reg = read16(&obj->reg->pullen[pos].val);
	}

	return (((reg & (1L << bit)) != 0) ? 1 : 0);
}

static s32 mt_set_gpio_pull_select_chip(u32 pin, u32 select)
{
	u32 pos = 0;
	u32 bit = 0;

	struct mt_gpio_obj *obj = gpio_obj;

	/* GPIO114~GPIO169 BASE_ADDRESS is different with others */
	if ((pin >= 114) && (pin <= 169)) {
		obj = gpio1_obj;
	}

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	if (select >= GPIO_PULL_MAX)
		return -ERINVAL;

	if ((pin >= 34) && (pin <= 148)) {
		if (select == GPIO_PULL_DOWN)
			GPIO_SET_BITS((1L << bit),
				      (u32)(&obj->reg->pullsel[pos].rst) +
				      GPIO_OFFSET);
		else
			GPIO_SET_BITS((1L << bit),
				      (u32)(&obj->reg->pullsel[pos].set) +
				      GPIO_OFFSET);

	} else {
		if (select == GPIO_PULL_DOWN)
			GPIO_SET_BITS((1L << bit),
				      (u32)(&obj->reg->pullsel[pos].rst));
		else
			GPIO_SET_BITS((1L << bit),
				      (u32)(&obj->reg->pullsel[pos].set));
	}

	return RSUCCESS;
}

static s32 mt_get_gpio_pull_select_chip(u32 pin)
{
	u32 pos = 0;
	u32 bit = 0;
	u32 reg;
	struct mt_gpio_obj *obj = gpio_obj;

	/* GPIO114~GPIO169 BASE_ADDRESS is different with others */
	if ((pin >= 114) && (pin <= 169)) {
		obj = gpio1_obj;
	}

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	if ((pin >= 34) && (pin <= 148)) {
		reg =
		    read16((&obj->reg->pullsel[pos].val) +
			      GPIO_OFFSET);
	} else {
		reg = read16(&obj->reg->pullsel[pos].val);
	}

	return (((reg & (1L << bit)) != 0) ? 1 : 0);
}

static s32 mt_set_gpio_inversion_chip(u32 pin, u32 enable)
{
	u32 pos = 0;
	u32 bit = 0;

	struct mt_gpio_obj *obj = gpio_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	if (enable >= GPIO_DATA_INV_MAX)
		return -ERINVAL;

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	if (enable == GPIO_DATA_UNINV)
		GPIO_SET_BITS((1L << bit), (u32)(&obj->reg->dinv[pos].rst));
	else
		GPIO_SET_BITS((1L << bit), (u32)(&obj->reg->dinv[pos].set));

	return RSUCCESS;
}

static s32 mt_get_gpio_inversion_chip(u32 pin)
{
	u32 pos = 0;
	u32 bit = 0;
	u32 reg;
	struct mt_gpio_obj *obj = gpio_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	reg = read16(&obj->reg->dinv[pos].val);

	return (((reg & (1L << bit)) != 0) ? 1 : 0);
}

static s32 mt_set_gpio_out_chip(u32 pin, u32 output)
{
	u32 pos = 0;
	u32 bit = 0;
	struct mt_gpio_obj *obj = gpio_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	if (output >= GPIO_OUT_MAX)
		return -ERINVAL;

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	if (output == GPIO_OUT_ZERO)
		GPIO_SET_BITS((1L << bit), &obj->reg->dout[pos].rst);
	else
		GPIO_SET_BITS((1L << bit), &obj->reg->dout[pos].set);

	return RSUCCESS;
}

static s32 mt_get_gpio_out_chip(u32 pin)
{
	u32 pos = 0;
	u32 bit = 0;
	u32 reg;
	struct mt_gpio_obj *obj = gpio_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	reg = read16(&obj->reg->dout[pos].val);

	return (((reg & (1L << bit)) != 0) ? 1 : 0);
}

static s32 mt_get_gpio_in_chip(u32 pin)
{
	u32 pos = 0;
	u32 bit = 0;
	u32 reg;
	struct mt_gpio_obj *obj = gpio_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	reg = read16(&obj->reg->din[pos].val);

	return (((reg & (1L << bit)) != 0) ? 1 : 0);
}

static s32 mt_set_gpio_mode_chip(u32 pin, u32 mode)
{
	u32 pos = 0;
	u32 bit = 0;
	u32 reg;
	u32 mask = (1L << GPIO_MODE_BITS) - 1;
	struct mt_gpio_obj *obj = gpio_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	if (mode >= GPIO_MODE_MAX)
		return -ERINVAL;

	pos = pin / MAX_GPIO_MODE_PER_REG;
	bit = pin % MAX_GPIO_MODE_PER_REG;

	reg = read16(&obj->reg->mode[pos].val);

	reg &= ~(mask << (GPIO_MODE_BITS * bit));
	reg |= (mode << (GPIO_MODE_BITS * bit));

	write16(reg, (void *)&obj->reg->mode[pos].val);

	return RSUCCESS;
}

static s32 mt_get_gpio_mode_chip(u32 pin)
{
	u32 pos = 0;
	u32 bit = 0;
	u32 reg;
	u32 mask = (1L << GPIO_MODE_BITS) - 1;
	struct mt_gpio_obj *obj = gpio_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	pos = pin / MAX_GPIO_MODE_PER_REG;
	bit = pin % MAX_GPIO_MODE_PER_REG;

	reg = read16(&obj->reg->mode[pos].val);

	return ((reg >> (GPIO_MODE_BITS * bit)) & mask);
}

/*
 * get driving
 * pin:GPIO0~GPIO202
 * read register address from "address_pin",then
 * read the driving value, and the return value should be within 2 to 32.
 */
s32 mt_get_gpio_driving_chip(u32 pin)
{
	u32 reg;
	u32 group;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	group = address_pin[pin].pingroup;
	if (group == 4) {
		return -ERINVAL;
	}

	reg = read32((void *)address_pin[pin].regaddress);
	GPIOMSG(" GPIO %d, address 0x%x, raw value 0x%X\n", pin,
		address_pin[pin].regaddress, reg);
	reg = (reg & (0xf << address_pin[pin].flag)) >> (address_pin[pin].flag);

	if (group == 2) {
		reg &= 0x3;	/* Group 2, Remove the SR and E8 bit. */
	} else {
		reg &= 0x7;	/* Remove the SR bit 3. */
	}

	if (group == 1)
		reg /= 2;	/* group one using 0, 2, 4, 6 steps */

	return pin_group[group].min_driving + reg * pin_group[group].step;
}

s32 mt_set_gpio_driving_chip(u32 pin, u32 driving)
{
	u32 group, nib;
	u32 reg;

	if (pin >= GPIO_EXTEND_START) {
		return -ERINVAL;
	}

	group = address_pin[pin].pingroup;

	if (group == 4) {
		return -ERINVAL;
	}

	if ((driving >= pin_group[group].min_driving
		&& driving <= pin_group[group].max_driving)
		&& driving % (pin_group[group].step) == 0) {
		nib = (driving / (pin_group[group].step) - 1);
		if (group == 1)
			nib *= 2;	/* 0, 2, 4, 6  for group 1 */
		if (group == 2) {
			nib |= 0xc;	/* For group 2, SR and E8 always 1. */
		} else {
			nib |= 0x8;	/* SR always 1 */
		}

		reg = read32((void *)address_pin[pin].regaddress);
		reg =
		    (reg & (~(0xf << address_pin[pin].flag))) | (nib <<
								 address_pin
								 [pin].flag);
		write32(reg, (void *)address_pin[pin].regaddress);
		GPIOMSG(" GPIO %d, write value 0x%X\n", pin, reg);

		return RSUCCESS;
	} else {
		return -ERINVAL;
	}
}

s32 mt_set_clock_output(u32 num, u32 src, u32 div)
{
	u32 pin_reg;
	u32 reg_value = 0;

	if (num >= CLK_MAX)
		return -ERINVAL;

	if (src >= CLK_SRC_MAX)
		return -ERINVAL;

	if ((div > 16) || (div <= 0))
		return -ERINVAL;

	pin_reg = clkout_reg_addr[num];

	reg_value = div - 1;
	reg_value |= (src << 4);
	write32(reg_value, (void *)pin_reg);

	return RSUCCESS;
}

s32 mt_get_clock_output(u32 num, u32 *src, u32 *div)
{
	u32 reg_value;
	u32 pin_reg;

	if (num >= CLK_MAX)
		return -ERINVAL;

	pin_reg = clkout_reg_addr[num];
	reg_value = read32((void *)pin_reg);
	*src = reg_value >> 4;
	printk(BIOS_INFO, "src==%d\n", *src);
	*div = (reg_value & 0x0f) + 1;
	printk(BIOS_INFO, "div==%d\n", *div);

	return RSUCCESS;
}

/* set extend GPIO */
static s32 mt_set_gpio_dir_ext(u32 pin, u32 dir)
{
	u32 pos = 0;
	u32 bit = 0;
	int ret = 0;
	struct mt_gpioext_obj *obj = gpioext_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	if (dir >= GPIO_DIR_MAX)
		return -ERINVAL;

	pin -= GPIO_EXTEND_START;
	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	if (dir == GPIO_DIR_IN)
		ret = GPIOEXT_SET_BITS((1L << bit), &obj->reg->dir[pos].rst);
	else
		ret = GPIOEXT_SET_BITS((1L << bit), &obj->reg->dir[pos].set);

	if (ret != 0)
		return -ERWRAPPER;

	return RSUCCESS;
}

static s32 mt_get_gpio_dir_ext(u32 pin)
{
	u32 pos = 0;
	u32 bit = 0;
	s64 reg;
	struct mt_gpioext_obj *obj = gpioext_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	pin -= GPIO_EXTEND_START;
	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	reg = gpioext_read(&obj->reg->dir[pos].val);

	if (reg < 0)
		return -ERWRAPPER;

	return (((reg & (1L << bit)) != 0) ? 1 : 0);
}

static s32 mt_set_gpio_pull_enable_ext(u32 pin, u32 enable)
{
	u32 pos = 0;
	u32 bit = 0;
	int ret = 0;
	struct mt_gpioext_obj *obj = gpioext_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	if (enable >= GPIO_PULL_EN_MAX)
		return -ERINVAL;

	pin -= GPIO_EXTEND_START;
	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	if (enable == GPIO_PULL_DISABLE)
		ret = GPIOEXT_SET_BITS((1L << bit), &obj->reg->pullen[pos].rst);
	else
		ret = GPIOEXT_SET_BITS((1L << bit), &obj->reg->pullen[pos].set);

	if (ret != 0)
		return -ERWRAPPER;

	return RSUCCESS;
}

static s32 mt_get_gpio_pull_enable_ext(u32 pin)
{
	u32 pos = 0;
	u32 bit = 0;
	s64 reg;
	struct mt_gpioext_obj *obj = gpioext_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	pin -= GPIO_EXTEND_START;
	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	reg = gpioext_read(&obj->reg->pullen[pos].val);

	if (reg < 0)
		return -ERWRAPPER;

	return (((reg & (1L << bit)) != 0) ? 1 : 0);
}

static s32 mt_set_gpio_pull_select_ext(u32 pin, u32 select)
{
	u32 pos = 0;
	u32 bit = 0;
	int ret = 0;
	struct mt_gpioext_obj *obj = gpioext_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	if (select >= GPIO_PULL_MAX)
		return -ERINVAL;

	pin -= GPIO_EXTEND_START;
	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	if (select == GPIO_PULL_DOWN)
		ret =
		    GPIOEXT_SET_BITS((1L << bit), &obj->reg->pullsel[pos].rst);
	else
		ret =
		    GPIOEXT_SET_BITS((1L << bit), &obj->reg->pullsel[pos].set);
	if (ret != 0)
		return -ERWRAPPER;

	return RSUCCESS;
}

static s32 mt_get_gpio_pull_select_ext(u32 pin)
{
	u32 pos = 0;
	u32 bit = 0;
	s64 reg;
	struct mt_gpioext_obj *obj = gpioext_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	pin -= GPIO_EXTEND_START;
	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	reg = gpioext_read(&obj->reg->pullsel[pos].val);

	if (reg < 0)
		return -ERWRAPPER;

	return (((reg & (1L << bit)) != 0) ? 1 : 0);
}

static s32 mt_set_gpio_inversion_ext(u32 pin, u32 enable)
{
	u32 pos = 0;
	u32 bit = 0;
	int ret = 0;
	struct mt_gpioext_obj *obj = gpioext_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	if (enable >= GPIO_DATA_INV_MAX)
		return -ERINVAL;

	pin -= GPIO_EXTEND_START;
	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	if (enable == GPIO_DATA_UNINV)
		ret = GPIOEXT_SET_BITS((1L << bit), &obj->reg->dinv[pos].rst);
	else
		ret = GPIOEXT_SET_BITS((1L << bit), &obj->reg->dinv[pos].set);

	if (ret != 0)
		return -ERWRAPPER;

	return RSUCCESS;
}

static s32 mt_get_gpio_inversion_ext(u32 pin)
{
	u32 pos = 0;
	u32 bit = 0;
	s64 reg;
	struct mt_gpioext_obj *obj = gpioext_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	pin -= GPIO_EXTEND_START;
	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	reg = gpioext_read(&obj->reg->dinv[pos].val);

	if (reg < 0)
		return -ERWRAPPER;

	return (((reg & (1L << bit)) != 0) ? 1 : 0);
}

static s32 mt_set_gpio_out_ext(u32 pin, u32 output)
{
	u32 pos = 0;
	u32 bit = 0;
	int ret = 0;
	struct mt_gpioext_obj *obj = gpioext_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	if (output >= GPIO_OUT_MAX)
		return -ERINVAL;

	pin -= GPIO_EXTEND_START;
	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	if (output == GPIO_OUT_ZERO)
		ret = GPIOEXT_SET_BITS((1L << bit), &obj->reg->dout[pos].rst);
	else
		ret = GPIOEXT_SET_BITS((1L << bit), &obj->reg->dout[pos].set);

	if (ret != 0)
		return -ERWRAPPER;

	return RSUCCESS;
}

static s32 mt_get_gpio_out_ext(u32 pin)
{
	u32 pos = 0;
	u32 bit = 0;
	s64 reg;
	struct mt_gpioext_obj *obj = gpioext_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	pin -= GPIO_EXTEND_START;
	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	reg = gpioext_read(&obj->reg->dout[pos].val);

	if (reg < 0)
		return -ERWRAPPER;

	return (((reg & (1L << bit)) != 0) ? 1 : 0);
}

static s32 mt_get_gpio_in_ext(u32 pin)
{
	u32 pos = 0;
	u32 bit = 0;
	s64 reg;
	struct mt_gpioext_obj *obj = gpioext_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	pin -= GPIO_EXTEND_START;
	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	reg = gpioext_read(&obj->reg->din[pos].val);

	if (reg < 0)
		return -ERWRAPPER;

	return (((reg & (1L << bit)) != 0) ? 1 : 0);
}

static s32 mt_set_gpio_mode_ext(u32 pin, u32 mode)
{
	u32 pos = 0;
	u32 bit = 0;
	s64 reg;
	int ret = 0;
	u32 mask = (1L << GPIO_MODE_BITS) - 1;
	struct mt_gpioext_obj *obj = gpioext_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	if (mode >= GPIO_MODE_MAX)
		return -ERINVAL;

	pin -= GPIO_EXTEND_START;
	pos = pin / MAX_GPIO_MODE_PER_REG;
	bit = pin % MAX_GPIO_MODE_PER_REG;

	reg = gpioext_read(&obj->reg->mode[pos].val);

	if (reg < 0)
		return -ERWRAPPER;

	reg &= ~(mask << (GPIO_MODE_BITS * bit));
	reg |= (mode << (GPIO_MODE_BITS * bit));

	ret = gpioext_write(&obj->reg->mode[pos].val, reg);

	if (ret != 0)
		return -ERWRAPPER;

	return RSUCCESS;
}

static s32 mt_get_gpio_mode_ext(u32 pin)
{
	u32 pos = 0;
	u32 bit = 0;
	s64 reg;
	u32 mask = (1L << GPIO_MODE_BITS) - 1;
	struct mt_gpioext_obj *obj = gpioext_obj;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	pin -= GPIO_EXTEND_START;
	pos = pin / MAX_GPIO_MODE_PER_REG;
	bit = pin % MAX_GPIO_MODE_PER_REG;

	reg = gpioext_read(&obj->reg->mode[pos].val);

	if (reg < 0)
		return -ERWRAPPER;

	return ((reg >> (GPIO_MODE_BITS * bit)) & mask);
}

static void mt_gpio_pin_decrypt(u32 *cipher)
{
	if ((*cipher & (0x80000000)) == 0) {
		printk(BIOS_ERR, "Pin %u decrypt warning!\n", *cipher);
	}
	*cipher &= ~(0x80000000);

	return;
}

s32 mt_set_gpio_dir(u32 pin, u32 dir)
{
	mt_gpio_pin_decrypt(&pin);

	return (pin >= GPIO_EXTEND_START) ? mt_set_gpio_dir_ext(pin, dir) : mt_set_gpio_dir_chip(pin, dir);
}

s32 mt_get_gpio_dir(u32 pin)
{
	mt_gpio_pin_decrypt(&pin);

	return (pin >= GPIO_EXTEND_START) ? mt_get_gpio_dir_ext(pin) : mt_get_gpio_dir_chip(pin);
}

s32 mt_set_gpio_pull_enable(u32 pin, u32 enable)
{
	mt_gpio_pin_decrypt(&pin);

	return (pin >= GPIO_EXTEND_START) ? mt_set_gpio_pull_enable_ext(pin, enable) : mt_set_gpio_pull_enable_chip(pin, enable);
}

s32 mt_get_gpio_pull_enable(u32 pin)
{
	mt_gpio_pin_decrypt(&pin);

	return (pin >=
		GPIO_EXTEND_START) ? mt_get_gpio_pull_enable_ext(pin) :
	    mt_get_gpio_pull_enable_chip(pin);
}

s32 mt_set_gpio_pull_select(u32 pin, u32 select)
{
	mt_gpio_pin_decrypt(&pin);

	return (pin >= GPIO_EXTEND_START) ? mt_set_gpio_pull_select_ext(pin,
									select)
	    : mt_set_gpio_pull_select_chip(pin, select);
}

s32 mt_get_gpio_pull_select(u32 pin)
{
	mt_gpio_pin_decrypt(&pin);

	return (pin >=
		GPIO_EXTEND_START) ? mt_get_gpio_pull_select_ext(pin) :
	    mt_get_gpio_pull_select_chip(pin);
}

s32 mt_set_gpio_inversion(u32 pin, u32 enable)
{
	mt_gpio_pin_decrypt(&pin);

	return (pin >= GPIO_EXTEND_START) ? mt_set_gpio_inversion_ext(pin,
								      enable) :
	    mt_set_gpio_inversion_chip(pin, enable);
}

s32 mt_get_gpio_inversion(u32 pin)
{
	mt_gpio_pin_decrypt(&pin);

	return (pin >=
		GPIO_EXTEND_START) ? mt_get_gpio_inversion_ext(pin) :
	    mt_get_gpio_inversion_chip(pin);
}

s32 mt_set_gpio_out(u32 pin, u32 output)
{
	mt_gpio_pin_decrypt(&pin);

	return (pin >= GPIO_EXTEND_START) ? mt_set_gpio_out_ext(pin,
								output) :
	    mt_set_gpio_out_chip(pin, output);
}

s32 mt_get_gpio_out(u32 pin)
{
	mt_gpio_pin_decrypt(&pin);

	return (pin >=
		GPIO_EXTEND_START) ? mt_get_gpio_out_ext(pin) :
	    mt_get_gpio_out_chip(pin);
}

s32 mt_get_gpio_in(u32 pin)
{
	mt_gpio_pin_decrypt(&pin);

	return (pin >=
		GPIO_EXTEND_START) ? mt_get_gpio_in_ext(pin) :
	    mt_get_gpio_in_chip(pin);
}

s32 mt_set_gpio_mode(u32 pin, u32 mode)
{
	mt_gpio_pin_decrypt(&pin);

	return (pin >= GPIO_EXTEND_START) ? mt_set_gpio_mode_ext(pin,
								 mode) :
	    mt_set_gpio_mode_chip(pin, mode);
}

s32 mt_get_gpio_mode(u32 pin)
{
	mt_gpio_pin_decrypt(&pin);

	return (pin >=
		GPIO_EXTEND_START) ? mt_get_gpio_mode_ext(pin) :
	    mt_get_gpio_mode_chip(pin);
}

static void mt_gpio_set_power(void)
{
	u32 val;
#define GPIO_BIAS_KEY_ADDR (0x1020c000 + 0x920)
	val = read32((void *)(GPIO_BASE + 0x900));
	val &= ~((1 << 1) | (1 << 7) | (1 << 13));
	/* Key unlock */
	write32(0x58, (void *)GPIO_BIAS_KEY_ADDR);
	write32(0xfa, (void *)GPIO_BIAS_KEY_ADDR);
	write32(0x65, (void *)GPIO_BIAS_KEY_ADDR);
	write32(0x83, (void *)GPIO_BIAS_KEY_ADDR);

	write32(val, (void *)GPIO_BASE + 0x900);
	/* Key lock */
	write32(0x00, (void *)GPIO_BIAS_KEY_ADDR);
	write32(0x00, (void *)GPIO_BIAS_KEY_ADDR);
	write32(0x00, (void *)GPIO_BIAS_KEY_ADDR);
	write32(0x00, (void *)GPIO_BIAS_KEY_ADDR);
}

void mt_gpio_init(void)
{
	mt_gpio_set_power();
}
