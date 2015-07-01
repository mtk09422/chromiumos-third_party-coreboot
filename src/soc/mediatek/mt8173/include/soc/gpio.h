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
#ifndef SOC_MEDIATEK_MT8173_GPIO_H
#define SOC_MEDIATEK_MT8173_GPIO_H

#include <stdint.h>
#include <soc/addressmap.h>

typedef u32 gpio_t;

enum {
        GPIOEXT_BASE = 0xC000,
};

#define GPIO_EXTEND_START (135)

struct val_regs {
	uint16_t val;
	uint16_t _align1;
	uint16_t set;
	uint16_t _align2;
	uint16_t rst;
	uint16_t _align3[3];
};

struct gpio_regs {
	struct val_regs dir[9];		/* 0x0000 ~ 0x008F: 144 bytes */
	uint8_t rsv00[112];		/* 0x0090 ~ 0x00FF: 112 bytes */
	struct val_regs pullen[9];	/* 0x0100 ~ 0x018F: 144 bytes */
	uint8_t rsv01[112];		/* 0x0190 ~ 0x01FF: 112 bytes */
	struct val_regs pullsel[9];	/* 0x0200 ~ 0x028F: 144 bytes */
	uint8_t rsv02[112];		/* 0x0290 ~ 0x02FF: 112 bytes */
	uint8_t rsv03[256];		/* 0x0300 ~ 0x03FF: 256 bytes */
	struct val_regs dout[9];	/* 0x0400 ~ 0x048F: 144 bytes */
	uint8_t rsv04[112];		/* 0x0490 ~ 0x04FF: 112 bytes */
	struct val_regs din[9];		/* 0x0500 ~ 0x058F: 144 bytes */
	uint8_t rsv05[112];		/* 0x0590 ~ 0x05FF: 112 bytes */
	struct val_regs mode[27];	/* 0x0600 ~ 0x07AF: 432 bytes */
	uint8_t rsv06[336];		/* 0x07B0 ~ 0x08FF: 336 bytes */
	struct val_regs ies[3];		/* 0x0900 ~ 0x092F:  48 bytes */
	struct val_regs smt[3];		/* 0x0930 ~ 0x095F:  48 bytes */
	uint8_t rsv07[160];		/* 0x0960 ~ 0x09FF: 160 bytes */
	struct val_regs tdsel[8];	/* 0x0A00 ~ 0x0A7F: 128 bytes */
	struct val_regs rdsel[6];	/* 0x0A80 ~ 0x0ADF:  96 bytes */
	uint8_t rsv08[32];		/* 0x0AE0 ~ 0x0AFF:  32 bytes */
	struct val_regs drv_mode[10];	/* 0x0B00 ~ 0x0B9F: 160 bytes */
	uint8_t rsv09[96];		/* 0x0BA0 ~ 0x0BFF:  96 bytes */
	struct val_regs msdc_rsv0[11];	/* 0x0C00 ~ 0x0CAF: 176 bytes */
	struct val_regs msdc2_ctrl5;	/* 0x0CB0 ~ 0x0CBF:  16 bytes */
	struct val_regs msdc_rsv1[12];	/* 0x0CC0 ~ 0x0D7F: 192 bytes */
	uint8_t rsv10[64];		/* 0x0D80 ~ 0x0DBF:  64 bytes */
	struct val_regs exmd_ctrl[1];	/* 0x0DC0 ~ 0x0DCF:  16 bytes */
	uint8_t rsv11[48];		/* 0x0DD0 ~ 0x0DFF:  48 bytes */
	struct val_regs kpad_ctrl[2];	/* 0x0E00 ~ 0x0E1F:  32 bytes */
	struct val_regs hsic_ctrl[4];	/* 0x0E20 ~ 0x0E5F:  64 bytes */
};

check_member(gpio_regs, msdc2_ctrl5, 0xcb0);

static struct gpio_regs *const gpio_obj = (void *) (GPIO_BASE);

struct ext_val_regs {
	uint16_t val;
	uint16_t set;
	uint16_t rst;
	uint16_t _align;
};

struct gpioext_regs {
	struct ext_val_regs dir[4];	/* 0x0000 ~ 0x001F: 32 bytes */
	struct ext_val_regs pullen[4];	/* 0x0020 ~ 0x003F: 32 bytes */
	struct ext_val_regs pullsel[4];	/* 0x0040 ~ 0x005F: 32 bytes */
	struct ext_val_regs dinv[4];	/* 0x0060 ~ 0x007F: 32 bytes */
	struct ext_val_regs dout[4];	/* 0x0080 ~ 0x009F: 32 bytes */
	struct ext_val_regs din[4];	/* 0x00A0 ~ 0x00BF: 32 bytes */
	struct ext_val_regs mode[10];	/* 0x00C0 ~ 0x010F: 80 bytes */
};

static struct gpioext_regs *const gpioext_obj = (void *) (GPIOEXT_BASE);

/* for msdc pupd */
struct msdc_pupd {
	u32 start;
	u32 end;
	uint16_t reg;
	uint8_t	bit;
};

enum {
	R0R1=0,
	PUPD=2,
};

/* GPIO Driver interface */
/* pull enable and  select */
s32 mt_set_gpio_pull(u32 pin, u32 enable, u32 select);

void mt_gpio_init(void);
void mt_gpio_set_default(void);
void mt_gpio_set_default_ext(void);

#endif /* SOC_MEDIATEK_MT8173_GPIO_H */
