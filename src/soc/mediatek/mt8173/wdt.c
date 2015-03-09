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
#include <soc/mt8173.h>
#include <soc/wdt.h>

static uint32_t g_rgu_status = RE_BOOT_REASON_UNKNOW;
static uint32_t timeout;
static uint32_t reboot_from = RE_BOOT_FROM_UNKNOW;
static uint32_t rgu_mode;

enum {
	WRITE_REG_DELAY = 100,
	RESET_MD32_DELAY = 1000
};

static void mtk_wdt_reset(uint8_t mode)
{
	/* Watchdog Restart */
	write32((uint32_t *) MTK_WDT_RESTART, MTK_WDT_RESTART_KEY);

	/*
	 * mode != 0 means bypass power key reboot, which means the system can boot up directly
	 * without pressing powerkey to reboot system after watchdog outputs reset signal to PMIC.
	 * We use AUTO_RESTART bit as the flag.
	 * PMIC will check the bit in preloader to decide to poweroff or power on.
	 */
	setbits_le32((uint32_t *) MTK_WDT_MODE, MTK_WDT_MODE_KEY |
		     MTK_WDT_MODE_EXTEN | (mode ? MTK_WDT_MODE_AUTO_RESTART : 0));

	/* Watchdog will trigger reset signal to PMIC as soon as
	 * we write MTK_WDT_SWRST with MTK_WDT_SWRST_KEY,
	 * but writing register MTK_WDT_MODE has latency, so we add delay here.
	 */
	udelay(WRITE_REG_DELAY);
	write32((uint32_t *) MTK_WDT_SWRST, MTK_WDT_SWRST_KEY);
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
		status = read32((uint32_t *) MTK_WDT_STATUS);

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
	uint32_t interval_val = read32((uint32_t *) MTK_WDT_INTERVAL);

	reboot_from = RE_BOOT_FROM_UNKNOW;

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
			/* RGU reset, but not pr-loader, u-boot, kernel,
			   from where???
			*/
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
	clrsetbits_le32((uint32_t *) MTK_WDT_INTERVAL,
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
	uint32_t mode;

	mode = read32((uint32_t *) MTK_WDT_MODE);
	mode|= MTK_WDT_MODE_KEY;

	if (wdt_en == 1)
		mode |= MTK_WDT_MODE_ENABLE;
	else
		mode &= ~MTK_WDT_MODE_ENABLE;

	if (ext_pol == 1)
		mode |= MTK_WDT_MODE_EXT_POL;
	else
		mode &= ~MTK_WDT_MODE_EXT_POL;

	if (ext_en == 1)
		mode |= MTK_WDT_MODE_EXTEN;
	else
		mode &= ~MTK_WDT_MODE_EXTEN;

	if (irq == 1)
		mode |= MTK_WDT_MODE_IRQ;
	else
		mode &= ~MTK_WDT_MODE_IRQ;

	if (dual_mode_en == 1)
		mode |= MTK_WDT_MODE_DUAL_MODE;
	else
		mode &= ~MTK_WDT_MODE_DUAL_MODE;

	/* Bit 4: WDT_Auto_restart, this is a reserved bit, we use it as
	   bypass powerkey flag.
	   Because HW reboot always need reboot to kernel, we set it always.
	*/
	mode |= MTK_WDT_MODE_AUTO_RESTART;

	write32((uint32_t *) MTK_WDT_MODE, mode);
	printk(BIOS_INFO, "%s  mode value=%#x, mode: %#x \n", __func__,
	       read32((uint32_t *) MTK_WDT_MODE), mode);
}

static void mtk_wdt_set_time_out_value(uint32_t value)
{
	/*
	 * TimeOut = BitField 15:5
	 * Key      = BitField  4:0 = 0x08
	 */

	/* sec * 32768 / 512 = sec * 64 = sec * 1 << 6 */
	timeout = (uint32_t)(value * (1 << 6));
	timeout = timeout << 5;
	write32((uint32_t *) MTK_WDT_LENGTH, timeout | MTK_WDT_LENGTH_KEY);
}

static void mtk_wdt_restart(void)
{
	write32((uint32_t *) MTK_WDT_RESTART, MTK_WDT_RESTART_KEY);
}

uint8_t mtk_is_rgu_trigger_reset()
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
	     MTK_WDT_STATUS_SPM_THERMAL_RST | MTK_WDT_STATUS_SPMWDT_RST |
	     MTK_WDT_STATUS_THERMAL_DIRECT_RST | MTK_WDT_STATUS_SECURITY_RST
	     | MTK_WDT_STATUS_DEBUGWDT_RST))
	{
		if (rgu_mode & MTK_WDT_MODE_AUTO_RESTART) {
			/* HW/SW reboot, and auto restart is set, means
			 * bypass power key */
			print_info("SW reset with bypass power key flag \n");
			return WDT_BY_PASS_PWK_REBOOT;
		} else {
			print_info("SW reset without bypass power key flag \n");
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
	printk(BIOS_INFO, "RGU MODE:     %#x\n",
	       read32((uint32_t *) MTK_WDT_MODE));
	printk(BIOS_INFO, "RGU LENGTH:   %#x\n",
	       read32((uint32_t *) MTK_WDT_LENGTH));
	printk(BIOS_INFO, "RGU STA:      %#x\n",
	       read32((uint32_t *) MTK_WDT_STATUS));
	printk(BIOS_INFO, "RGU INTERVAL: %#x\n",
	       read32((uint32_t *) MTK_WDT_INTERVAL));
	printk(BIOS_INFO, "RGU SWSYSRST: %#x\n",
	       read32((uint32_t *) MTK_WDT_SWSYSRST));
	printk(BIOS_INFO, "==== Dump RGU Reg End ====\n");

	rgu_mode = read32((uint32_t *) MTK_WDT_MODE);

	/* This function will store the reset reason: Time out/ SW trigger */
	wdt_sta = mtk_wdt_check_status();
	if ((wdt_sta & MTK_WDT_STATUS_HWWDT_RST) &&
	    (rgu_mode & MTK_WDT_MODE_AUTO_RESTART)) {
		/* Time out reboot always by pass power key */
		g_rgu_status = RE_BOOT_BY_WDT_HW;
	} else if (wdt_sta & MTK_WDT_STATUS_SWWDT_RST) {
		g_rgu_status = RE_BOOT_BY_WDT_SW;
	} else {
		g_rgu_status = RE_BOOT_REASON_UNKNOW;
	}

	if (wdt_sta & MTK_WDT_STATUS_IRQWDT_RST)
		g_rgu_status |= RE_BOOT_WITH_INTTERUPT;

	if (wdt_sta & MTK_WDT_STATUS_SPM_THERMAL_RST)
		g_rgu_status |= RE_BOOT_BY_SPM_THERMAL;

	if (wdt_sta & MTK_WDT_STATUS_SPMWDT_RST)
		g_rgu_status |= RE_BOOT_BY_SPM;

	if (wdt_sta & MTK_WDT_STATUS_THERMAL_DIRECT_RST)
		g_rgu_status |= RE_BOOT_BY_THERMAL_DIRECT;

	if (wdt_sta & MTK_WDT_STATUS_DEBUGWDT_RST)
		g_rgu_status |= RE_BOOT_BY_DEBUG;

	if (wdt_sta & MTK_WDT_STATUS_SECURITY_RST)
		g_rgu_status |= RE_BOOT_BY_SECURITY;

	printk(BIOS_INFO, "RGU: g_rgu_satus:%d\n", g_rgu_status);

	/* Wirte Mode register will clear status register */
	mtk_wdt_mode_config(0, 0, 0, 0, 0);
	mtk_wdt_check_trig_reboot_reason();
	/* Setting timeout 20s */
	mtk_wdt_set_time_out_value(20);

	/* Config HW reboot mode */
	mtk_wdt_mode_config(1, 1, 1, 0, 0);
	mtk_wdt_restart();
	/* set mcu_lath_en requir by pl owner confirmed by RGU DE */

	setbits_le32((uint32_t *) MTK_WDT_DEBUG_CTL,
		     MTK_DEBUG_CTL_KEY | MTK_RG_MCU_LATH_EN);
	printk(BIOS_INFO, "RGU %s:MTK_WDT_DEBUG_CTL(%#x)\n", __func__,
		read32((uint32_t *) MTK_WDT_DEBUG_CTL));
}

uint32_t get_g_rgu_status(void)
{
	return g_rgu_status;
}

void mtk_arch_reset(uint8_t mode)
{
	print_info("mtk_arch_reset at pre-loader!\n");

	mtk_wdt_reset(mode);
	/*
	 * Watchdog will output reset signal to PMIC as soon as mtk_wdt_reset() executes.
	 * We do not want the thread keeps running, so add while(1) to hang cpu here.
	 */
	while (1);
}

int rgu_dram_reserved(int enable)
{
	int ret = 0;
	if (1 == enable) {
		/* enable ddr reserved mode */
		setbits_le32((uint32_t *) MTK_WDT_MODE,
			     MTK_WDT_MODE_KEY | MTK_WDT_MODE_DDR_RESERVE);

	} else if (0 == enable) {
		/* disable ddr reserved mode, set reset mode,
		   disable watchdog output reset signal */
		clrsetbits_le32((uint32_t *) MTK_WDT_MODE,
				MTK_WDT_MODE_DDR_RESERVE, MTK_WDT_MODE_KEY);
	} else {
		printk(BIOS_INFO,
		       "Wrong input %d, should be 1(enable) or 0(disable) in %s\n",
		       enable, __func__);
		ret = -1;
	}
	printk(BIOS_INFO, "RGU %s:MTK_WDT_MODE(%#x)\n", __func__, read32((uint32_t *) MTK_WDT_MODE));
	return ret;
}

int rgu_is_reserve_ddr_enabled(void)
{
	uint32_t wdt_mode;
	wdt_mode = read32((uint32_t *) MTK_WDT_MODE);
	if (wdt_mode & MTK_WDT_MODE_DDR_RESERVE)
		return 1;
	else
		return 0;
}

int rgu_is_dram_slf(void)
{
	uint32_t wdt_dbg_ctrl;
	wdt_dbg_ctrl = read32((uint32_t *) MTK_WDT_DEBUG_CTL);
	printk(BIOS_INFO, "DDR is in self-refresh. %#x\n", wdt_dbg_ctrl);
	if (wdt_dbg_ctrl & MTK_DDR_SREF_STA)
		return 1;
	else
		return 0;
}

void rgu_release_rg_dramc_conf_iso(void)
{
	clrsetbits_le32((uint32_t *) MTK_WDT_DEBUG_CTL,
			MTK_RG_CONF_ISO, MTK_DEBUG_CTL_KEY);
	printk(BIOS_INFO, "RGU %s:MTK_WDT_DEBUG_CTL(%#x)\n", __func__,
	       read32((uint32_t *) MTK_WDT_DEBUG_CTL));
}

void rgu_release_rg_dramc_iso(void)
{
	clrsetbits_le32((uint32_t *) MTK_WDT_DEBUG_CTL, MTK_RG_DRAMC_ISO,
			MTK_DEBUG_CTL_KEY);
	printk(BIOS_INFO, "RGU %s:MTK_WDT_DEBUG_CTL(%#x)\n", __func__,
	       read32((uint32_t *) MTK_WDT_DEBUG_CTL));
}

void rgu_release_rg_dramc_sref(void)
{
	clrsetbits_le32((uint32_t *) MTK_WDT_DEBUG_CTL, MTK_RG_DRAMC_SREF,
			MTK_DEBUG_CTL_KEY);
	printk(BIOS_INFO, "RGU %s:MTK_WDT_DEBUG_CTL(%#x)\n", __func__,
	       read32((uint32_t *) MTK_WDT_DEBUG_CTL));
}

int rgu_is_reserve_ddr_mode_success(void)
{
	uint32_t wdt_dbg_ctrl;
	wdt_dbg_ctrl = read32((uint32_t *) MTK_WDT_DEBUG_CTL);
	if (wdt_dbg_ctrl & MTK_DDR_RESERVE_RTA) {
		printk(BIOS_INFO, "WDT DDR reserve mode success! %#x\n", wdt_dbg_ctrl);
		return 1;
	} else {
		printk(BIOS_INFO, "WDT DDR reserve mode FAIL! %#x\n", wdt_dbg_ctrl);
		return 0;
	}
}

void rgu_swsys_reset(WD_SYS_RST_TYPE reset_type)
{
	if (WD_MD_RST == reset_type) {
		setbits_le32((uint32_t *) MTK_WDT_SWSYSRST,
			MTK_WDT_SWSYS_RST_KEY | 0x80);
		/*
		 * set bit[7] of MTK_WDT_SWSYSRST to 1 will reset MD32(a micro chip).
		 * we delay 1ms to wait reset md32 done.
		 */
		udelay(RESET_MD32_DELAY);
		/* clear reset bit of md32*/
		clrsetbits_le32((uint32_t *) MTK_WDT_SWSYSRST,
			0x80, MTK_WDT_SWSYS_RST_KEY);
		printk(BIOS_INFO, "rgu pl md reset\n");
	}
}
