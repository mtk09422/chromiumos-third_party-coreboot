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

#include <delay.h>
#include <console/console.h>
#include <arch/io.h>

#include <soc/mt8135.h>
#include <soc/wdt.h>

#if CFG_HW_WATCHDOG
static unsigned int timeout;
static unsigned int reboot_from = RE_BOOT_FROM_UNKNOWN;
static unsigned int rgu_mode;
unsigned int g_rgu_status = RE_BOOT_REASON_UNKNOWN;

static void mtk_wdt_reset(uint8_t mode)
{
	/* Watchdog Reset */
	write32(MTK_WDT_RESTART_KEY, (uint32_t *)MTK_WDT_RESTART);

	/*
	 * mode != 0 means bypass power key reboot, We using auto_restart
	 * bit as by pass power key flag
	 */
	setbits_le32((uint32_t *)MTK_WDT_MODE,
		     MTK_WDT_MODE_KEY | MTK_WDT_MODE_EXTEN |
		     (mode ? MTK_WDT_MODE_AUTO_RESTART : 0));
	/* Watchdog will trigger reset signal to PMIC as soon as
	 * write MTK_WDT_SWRST with MTK_WDT_SWRST_KEY,
	 but write register MTK_WDT_MODE has latency, so we add delay here.
	 */
	udelay(100);
	write32(MTK_WDT_SWRST_KEY, (uint32_t *)MTK_WDT_SWRST);
}

static unsigned int mtk_wdt_check_status(void)
{
	static uint32_t status;

	/**
	 * Note:
	 *   Because WDT_STA register will be cleared after writing WDT_MODE,
	 *   we use a static variable to store WDT_STA.
	 *   After reset, static varialbe will always be clear to 0,
	 *   so only read WDT_STA when static variable is 0 is OK
	 */
	if (0 == status)
		status = read32((uint32_t *)MTK_WDT_STATUS);

	return status;
}

/**
 * For Power off and power on reset, the INTERVAL default value is 0x7FF.
 * We set Interval[1:0] to different value to distinguish different stage.
 * Enter pre-loader, we will set it to 0x0
 * Enter u-boot, we will set it to 0x1
 * Enter kernel, we will set it to 0x2
 * And the default value is 0x3 which means reset from a power off and power
 * on reset
 */
enum {
	POWER_OFF_ON_MAGIC = 0x3,
	PRE_LOADER_MAGIC = 0x0,
	U_BOOT_MAGIC = 0x1,
	KERNEL_MAGIC = 0x2,
	MAGIC_NUM_MASK = 0x3
};

/**
 * If the reset is trigger by RGU(Time out or SW trigger), we hope the system
 * can boot up directly; we DO NOT hope we must press power key to reboot
 * system after reset.
 * This message should tell pre-loader and u-boot, and we use Interval[2] to
 * store this information.
 * And this information will be cleared after uboot check it.
 */
enum {
	IS_POWER_ON_RESET = 0x1 << 2,
	RGU_TRIGGER_RESET_MASK = 0x1 << 2
};

static void mtk_wdt_check_trig_reboot_reason(void)
{
	uint32_t interval_val = read32((uint32_t *)MTK_WDT_INTERVAL);

	reboot_from = RE_BOOT_FROM_UNKNOWN;

	/* 1. Get reboot reason */
	if (0 != mtk_wdt_check_status()) {
		/* Enter here means this reset is triggered by RGU(WDT) */
		print_info("PL RGU RST: ");
		switch (interval_val & MAGIC_NUM_MASK) {
		case PRE_LOADER_MAGIC:
			reboot_from = RE_BOOT_FROM_PRE_LOADER;
			print_info("P\n");
			break;
		case U_BOOT_MAGIC:
			reboot_from = RE_BOOT_FROM_U_BOOT;
			print_info("U\n");
			break;
		case KERNEL_MAGIC:
			reboot_from = RE_BOOT_FROM_KERNEL;
			print_info("K\n");
			break;
		default:
			/* RGU reset, but not pr-loader, u-boot, kernel, */
			/* from where??? */
			print_info("??\n");
			break;
		}
	} else {
		/* Enter here means reset may triggered by power off power on */
		if ((interval_val & MAGIC_NUM_MASK) == POWER_OFF_ON_MAGIC) {
			reboot_from = RE_BOOT_FROM_POWER_ON;
			print_info("PL P ON\n");
		} else {
			/* Not RGU trigger reset, and not defautl value, why? */
			print_info("PL ?!\n");
		}
	}

	/* 2. Update interval register value and set reboot flag for u-boot */
	clrsetbits_le32((uint32_t *)MTK_WDT_INTERVAL,
			RGU_TRIGGER_RESET_MASK | MAGIC_NUM_MASK,
			PRE_LOADER_MAGIC |
			((reboot_from ==
			  RE_BOOT_FROM_POWER_ON) ? IS_POWER_ON_RESET : 0));
	/* 3. By pass power key info */
	if (mtk_wdt_boot_check() == WDT_BY_PASS_PWK_REBOOT)
		print_info("Find bypass powerkey flag\n");
	else if (mtk_wdt_boot_check() == WDT_NORMAL_REBOOT)
		print_info("No bypass powerkey flag\n");
	else
		print_info("WDT does not trigger reboot\n");
}

static void mtk_wdt_mode_config(uint8_t dual_mode_en, uint8_t irq,
				uint8_t ext_en, uint8_t ext_pol, uint8_t wdt_en)
{
	unsigned int tmp;

	tmp = read32((uint32_t *)MTK_WDT_MODE);
	tmp |= MTK_WDT_MODE_KEY;

	if (wdt_en == 1)
		tmp |= MTK_WDT_MODE_ENABLE;
	else
		tmp &= ~MTK_WDT_MODE_ENABLE;

	if (ext_pol == 1)
		tmp |= MTK_WDT_MODE_EXT_POL;
	else
		tmp &= ~MTK_WDT_MODE_EXT_POL;

	if (ext_en == 1)
		tmp |= MTK_WDT_MODE_EXTEN;
	else
		tmp &= ~MTK_WDT_MODE_EXTEN;

	if (irq == 1)
		tmp |= MTK_WDT_MODE_IRQ;
	else
		tmp &= ~MTK_WDT_MODE_IRQ;

	if (dual_mode_en == 1)
		tmp |= MTK_WDT_MODE_DUAL_MODE;
	else
		tmp &= ~MTK_WDT_MODE_DUAL_MODE;

	/* Bit 4: WDT_Auto_restart, this is a reserved bit, we use it as
	 *  bypass powerkey flag.
	 * Because HW reboot always need reboot to kernel, we set it always.
	 */
	tmp |= MTK_WDT_MODE_AUTO_RESTART;

	write32(tmp, (uint32_t *)MTK_WDT_MODE);
	printk(BIOS_INFO, "mtk_wdt_mode_config  mode value=%x, tmp:%x\n",
	       read32((uint32_t *)MTK_WDT_MODE), tmp);
}

static void mtk_wdt_set_time_out_value(uint32_t value)
{
	/*
	 * TimeOut = BitField 15:5
	 * Key = BitField  4:0 = 0x08
	 */

	/* sec * 32768 / 512 = sec * 64 = sec * 1 << 6 */
	timeout = (uint32_t)(value * (1 << 6));
	timeout = timeout << 5;
	write32(timeout | MTK_WDT_LENGTH_KEY, (uint32_t *)MTK_WDT_LENGTH);
}

void mtk_wdt_restart(void)
{
	/* reset WatchdogTimer's counter value to timeout value
	   ie., keepalive()
	 */
	write32(MTK_WDT_RESTART_KEY, (uint32_t *)MTK_WDT_RESTART);
}

uint8_t mtk_is_rgu_trigger_reset(void)
{
	if (reboot_from == RE_BOOT_FROM_POWER_ON)
		return 0;
	return 1;
}

int mtk_wdt_boot_check(void)
{
	uint32_t wdt_sta = mtk_wdt_check_status();

	/*
	 * For DA download hope to timeout reboot, and boot to u-boot/kernel
	 * configurable reason, we set both timeout reboot and software reboot
	 * can check whether bypass power key.
	 */
	if (wdt_sta &
	    (MTK_WDT_STATUS_HWWDT_RST | MTK_WDT_STATUS_SWWDT_RST |
	     MTK_WDT_STATUS_SPMWDT_RST)) {
		if (rgu_mode & MTK_WDT_MODE_AUTO_RESTART) {
			/* HW/SW reboot, and auto restart is set, means
			 * bypass power key */
			print_info("SW reset with bypass power key flag\n");
			return WDT_BY_PASS_PWK_REBOOT;
		} else {
			print_info("SW reset without bypass power key flag\n");
			return WDT_NORMAL_REBOOT;
		}
	}

	return WDT_NOT_WDT_REBOOT;
}

void mtk_wdt_init(void)
{
	unsigned wdt_sta;
	/* Dump RGU regisers */
	printk(BIOS_INFO, "==== Dump RGU Reg ========\n");
	printk(BIOS_INFO, "RGU MODE:     %x\n",
	       read32((uint32_t *)MTK_WDT_MODE));
	printk(BIOS_INFO, "RGU LENGTH:   %x\n",
	       read32((uint32_t *)MTK_WDT_LENGTH));
	printk(BIOS_INFO, "RGU STA:      %x\n",
	       read32((uint32_t *)MTK_WDT_STATUS));
	printk(BIOS_INFO, "RGU INTERVAL: %x\n",
	       read32((uint32_t *)MTK_WDT_INTERVAL));
	printk(BIOS_INFO, "RGU SWSYSRST: %x\n",
	       read32((uint32_t *)MTK_WDT_SWSYSRST));
	printk(BIOS_INFO, "==== Dump RGU Reg End ====\n");

	rgu_mode = read32((uint32_t *)MTK_WDT_MODE);

	/* This function will store the reset reason: Time out/ SW trigger */
	wdt_sta = mtk_wdt_check_status();
	if ((wdt_sta & MTK_WDT_STATUS_HWWDT_RST) &&
	    (rgu_mode & MTK_WDT_MODE_AUTO_RESTART)) {
		/* Time out reboot always by pass power key */
		g_rgu_status = RE_BOOT_BY_WDT_HW;
	} else if (wdt_sta & MTK_WDT_STATUS_SWWDT_RST) {
		g_rgu_status = RE_BOOT_BY_WDT_SW;
	} else {
		g_rgu_status = RE_BOOT_REASON_UNKNOWN;
	}

	if (wdt_sta & MTK_WDT_STATUS_IRQWDT_RST)
		g_rgu_status |= RE_BOOT_WITH_INTTERUPT;

	if (wdt_sta & MTK_WDT_STATUS_SPMWDT_RST)
		g_rgu_status |= RE_BOOT_BY_SPM_THERMAL;

	printk(BIOS_INFO, "RGU: g_rgu_satus:%d\n", g_rgu_status);

	/* Write Mode register will clear status register */
	mtk_wdt_mode_config(0, 0, 0, 0, 0);
	mtk_wdt_check_trig_reboot_reason();
	/* Setting timeout 10s */
	mtk_wdt_set_time_out_value(10);

	/* Config HW reboot mode */
	mtk_wdt_mode_config(1, 1, 1, 0, 1);
	mtk_wdt_restart();
}

void mtk_arch_reset(uint8_t mode)
{
	print_info("mtk_arch_reset at pre-loader!\n");

	mtk_wdt_reset(mode);

	while (1);
}

#else				/* Using dummy WDT functions */
void mtk_wdt_init(void)
{
	print_info("PL WDT Dummy init called\n");
}

uint8_t mtk_is_rgu_trigger_reset(void)
{
	print_info("PL Dummy mtk_is_rgu_trigger_reset called\n");
	return 0;
}

void mtk_arch_reset(char mode)
{
	print_info("PL WDT Dummy arch reset called\n");
}

int mtk_wdt_boot_check(void)
{
	print_info("PL WDT Dummy mtk_wdt_boot_check called\n");
	return 0;
}

#endif
