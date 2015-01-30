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

#ifndef SOC_MEDIATEK_MT8135_CUST_MSDC_H
#define SOC_MEDIATEK_MT8135_CUST_MSDC_H

#define MSDC_CD_PIN_EN      (1 << 0)	/* card detection pin is wired   */
#define MSDC_WP_PIN_EN      (1 << 1)	/* write protection pin is wired */
#define MSDC_RST_PIN_EN     (1 << 2)	/* emmc reset pin is wired       */
#define MSDC_SDIO_IRQ       (1 << 3)	/* use internal sdio irq (bus)   */
#define MSDC_EXT_SDIO_IRQ   (1 << 4)	/* use external sdio irq         */
#define MSDC_REMOVABLE      (1 << 5)	/* removable slot                */
#define MSDC_SYS_SUSPEND    (1 << 6)	/* suspended by system           */
#define MSDC_HIGHSPEED      (1 << 7)	/* high-speed mode support       */
#define MSDC_UHS1           (1 << 8)	/* uhs-1 mode support            */
#define MSDC_DDR            (1 << 9)	/* ddr mode support              */

#define MSDC_SMPL_RISING        (0)
#define MSDC_SMPL_FALLING       (1)

typedef enum {
	MSDC_CLKSRC_200MHZ = 0
} clk_source_t;

struct msdc_cust {
	unsigned char clk_src;		/* host clock source             */
	unsigned char cmd_edge;		/* command latch edge            */
	unsigned char data_edge;	/* data latch edge               */
	unsigned char clk_drv;		/* clock pad driving             */
	unsigned char cmd_drv;		/* command pad driving           */
	unsigned char dat_drv;		/* data pad driving              */
	unsigned char data_pins;	/* data pins                     */
	unsigned int data_offset;	/* data address offset           */
	unsigned int flags;		/* hardware capability flags     */
};

extern struct msdc_cust msdc_cap;

#endif /* SOC_MEDIATEK_MT8135_CUST_MSDC_H */
