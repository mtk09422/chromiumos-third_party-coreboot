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
#include <delay.h>

#include <soc/mt8135.h>
#include <soc/key.h>
#include <soc/pmic_6397.h>
#include <soc/gpio.h>
#include <soc/pmic_wrap_init.h>

#if CONFIG_DEBUG_KEY
#define DEBUG_KEY(level, x...)	printk(level, "[KEY]" x)
#else
#define DEBUG_KEY(level, x...)
#endif /* CONFIG_DEBUG_KEY */

#define DRV_Reg16(addr)		read16((void *)(addr))

#define GPIO_DIN_BASE		(GPIO_BASE + 0x0a00)

#define NOT_READY

static void detect_key_debug_info(void)
{
#if CONFIG_DEBUG_KEY
	unsigned int addr1[] = { 0x8404, 0x8408, 0x840C, 0x8410, 0x8414 };
	unsigned int addr2[] = { KP_MEM1, KP_MEM2, KP_MEM3, KP_MEM4, KP_MEM5 };

	for (unsigned int j = 0; j < 5; j++) {
		unsigned int c;
		unsigned int a = pwrap_read(addr1[j], &c);

		if (a != 0)
			print_info("kpd read fail, addr: 0x%x\n", addr1[j]);
		print_info("kpd read addr: 0x%x: data:0x%x\n", addr1[j], c);
	}

	for (unsigned int j = 0; j < 5; j++) {
		unsigned int a = *(volatile u16 *)(addr2[j]);

		print_info("kpd read addr: 0x%x: data:0x%x\n", addr2[j], a);
	}
#endif	/* CONFIG_DEBUG_KEY */
}

static void mtk_kpd_gpios_get(unsigned int ROW_REG[], unsigned int COL_REG[])
{
	int i;

	for (i = 0; i < 8; i++) {
		ROW_REG[i] = 0;
		COL_REG[i] = 0;
	}
#ifdef GPIO_KPD_KROW0_PIN
	ROW_REG[0] = GPIO_KPD_KROW0_PIN;
#endif

#ifdef GPIO_KPD_KROW1_PIN
	ROW_REG[1] = GPIO_KPD_KROW1_PIN;
#endif

#ifdef GPIO_KPD_KROW2_PIN
	ROW_REG[2] = GPIO_KPD_KROW2_PIN;
#endif

#ifdef GPIO_KPD_KROW3_PIN
	ROW_REG[3] = GPIO_KPD_KROW3_PIN;
#endif

#ifdef GPIO_KPD_KROW4_PIN
	ROW_REG[4] = GPIO_KPD_KROW4_PIN;
#endif

#ifdef GPIO_KPD_KROW5_PIN
	ROW_REG[5] = GPIO_KPD_KROW5_PIN;
#endif

#ifdef GPIO_KPD_KROW6_PIN
	ROW_REG[6] = GPIO_KPD_KROW6_PIN;
#endif

#ifdef GPIO_KPD_KROW7_PIN
	ROW_REG[7] = GPIO_KPD_KROW7_PIN;
#endif

#ifdef GPIO_KPD_KCOL0_PIN
	COL_REG[0] = GPIO_KPD_KCOL0_PIN;
#endif

#ifdef GPIO_KPD_KCOL1_PIN
	COL_REG[1] = GPIO_KPD_KCOL1_PIN;
#endif

#ifdef GPIO_KPD_KCOL2_PIN
	COL_REG[2] = GPIO_KPD_KCOL2_PIN;
#endif

#ifdef GPIO_KPD_KCOL3_PIN
	COL_REG[3] = GPIO_KPD_KCOL3_PIN;
#endif

#ifdef GPIO_KPD_KCOL4_PIN
	COL_REG[4] = GPIO_KPD_KCOL4_PIN;
#endif

#ifdef GPIO_KPD_KCOL5_PIN
	COL_REG[5] = GPIO_KPD_KCOL5_PIN;
#endif

#ifdef GPIO_KPD_KCOL6_PIN
	COL_REG[6] = GPIO_KPD_KCOL6_PIN;
#endif

#ifdef GPIO_KPD_KCOL7_PIN
	COL_REG[7] = GPIO_KPD_KCOL7_PIN;
#endif
}

static void mtk_kpd_gpio_set(void)
{
	unsigned int ROW_REG[8];
	unsigned int COL_REG[8];
	int i;

	DEBUG_KEY(BIOS_DEBUG, "Enter mtk_kpd_gpio_set!\n");
	mtk_kpd_gpios_get(ROW_REG, COL_REG);

	DEBUG_KEY(BIOS_DEBUG,
		  "kpd debug column : %d, %d, %d, %d, %d, %d, %d, %d\n",
			COL_REG[0], COL_REG[1], COL_REG[2], COL_REG[3],
			COL_REG[4], COL_REG[5], COL_REG[6], COL_REG[7]);
	DEBUG_KEY(BIOS_DEBUG,
		  "kpd debug row : %d, %d, %d, %d, %d, %d, %d, %d\n",
			ROW_REG[0], ROW_REG[1], ROW_REG[2], ROW_REG[3],
			ROW_REG[4], ROW_REG[5], ROW_REG[6], ROW_REG[7]);

	for (i = 0; i < 8; i++) {
		if (COL_REG[i] != 0) {
			/* KCOL: GPIO INPUT + PULL ENABLE + PULL UP */
			mt_set_gpio_mode(COL_REG[i], 1);
			mt_set_gpio_dir(COL_REG[i], 0);
			mt_set_gpio_pull_enable(COL_REG[i], 1);
			mt_set_gpio_pull_select(COL_REG[i], 1);
		}

		if (ROW_REG[i] != 0) {
			/* KROW: GPIO output + pull disable + pull down */
			mt_set_gpio_mode(ROW_REG[i], 1);
			mt_set_gpio_dir(ROW_REG[i], 1);
			mt_set_gpio_pull_enable(ROW_REG[i], 0);
			mt_set_gpio_pull_select(ROW_REG[i], 0);
		}
	}
	udelay(33000);
}

void set_kpd_pmic_mode(void)
{
	unsigned int a, c;

	a = pwrap_read(0x0502, &c);
	if (a != 0)
		DEBUG_KEY(BIOS_INFO, "kpd write fail, addr: 0x0502\n");

	DEBUG_KEY(BIOS_DEBUG, "kpd read addr: 0x0502: data:0x%x\n", c);
	c = c & 0xFFFE;
	a = pwrap_write(0x0502, c);
	if (a != 0)
		DEBUG_KEY(BIOS_INFO, "kpd write fail, addr: 0x0502\n");

	mtk_kpd_gpio_set();

	int rel = 0;

	rel =
	    pmic_config_interface(WRP_CKPDN, 0x0, PMIC_RG_WRP_KP_PDN_MASK,
				  PMIC_RG_WRP_KP_PDN_SHIFT);
	if (rel != 0) {
		DEBUG_KEY(BIOS_INFO, "kpd set clock register fail!\n");
	}

	*(volatile u16 *)(KP_PMIC) = 0x1;
	DEBUG_KEY(BIOS_INFO, "kpd register for pmic set!\n");
	return;
}

void disable_PMIC_kpd_clock(void)
{
	int rel = 0;

	DEBUG_KEY(BIOS_INFO, "kpd disable_PMIC_kpd_clock register!\n");
	rel = pmic_config_interface(WRP_CKPDN, 0x1,
				    PMIC_RG_WRP_32K_PDN_MASK,
				    PMIC_RG_WRP_32K_PDN_SHIFT);
	if (rel != 0) {
		DEBUG_KEY(BIOS_INFO,
			  "kpd disable_PMIC_kpd_clock register fail!\n");
	}
}

void enable_PMIC_kpd_clock(void)
{
	int rel = 0;

	DEBUG_KEY(BIOS_INFO, "kpd enable_PMIC_kpd_clock register!\n");
	rel = pmic_config_interface(WRP_CKPDN, 0x0,
				    PMIC_RG_WRP_32K_PDN_MASK,
				    PMIC_RG_WRP_32K_PDN_SHIFT);
	if (rel != 0) {
		DEBUG_KEY(BIOS_INFO,
			  "kpd enable_PMIC_kpd_clock register fail!\n");
	}
}

u8 mtk_detect_key(unsigned short key)
{
	/* key: HW keycode */
	unsigned short idx, bit, din;

	if (key >= KPD_NUM_KEYS)
		return 0;

	if (key % 9 == 8)
		key = 8;

	if (key == 8) {		/* Power key */
#if !defined(NOT_READY)
		/* [FIXME] This code block was enclosed by #if 0, remove this */
		/* code block if we don't need this. */

		/* for long press reboot not boot up from a reset */
		pmic_read_interface(0x050C, &just_rst, 0x01, 14);
		if (just_rst) {
			pmic_config_interface(0x050C, 0x01, 0x01, 4);
			print_info("Just recover from a reset\n");
			return 0;
		}
#endif
		if (1 == pmic_detect_powerkey()) {
			print_info("power key is pressed\n");
			return 1;
		}
		return 0;
	}

	idx = key / 16;
	bit = key % 16;

	detect_key_debug_info();

	din = DRV_Reg16(KP_MEM1 + (idx << 2)) & (1U << bit);
	if (!din) {
		detect_key_debug_info();
		DEBUG_KEY(BIOS_INFO, "key %d is pressed\n", key);
		return 1;
	}
	return 0;
}
