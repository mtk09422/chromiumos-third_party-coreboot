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

static struct mt8173_wdt_regs * const mt8173_wdt = (void *)RGU_BASE;

enum {
	WRITE_REG_DELAY = 100,
	RESET_MD32_DELAY = 1000
};

static void mtk_wdt_mode_config(uint8_t dual_mode_en, uint8_t irq,
				uint8_t ext_en, uint8_t ext_pol, uint8_t wdt_en)
{
	uint32_t mode;

	mode = read32(&mt8173_wdt->wdt_mode);
	mode |= MTK_WDT_MODE_KEY;

	clrsetbits_le32(&mt8173_wdt->wdt_mode, MTK_WDT_MODE_DUAL_MODE,
			(dual_mode_en ? MTK_WDT_MODE_DUAL_MODE : 0) | MTK_WDT_MODE_KEY);

	clrsetbits_le32(&mt8173_wdt->wdt_mode, MTK_WDT_MODE_IRQ,
			(irq ? MTK_WDT_MODE_IRQ : 0) | MTK_WDT_MODE_KEY);

	clrsetbits_le32(&mt8173_wdt->wdt_mode, MTK_WDT_MODE_EXTEN,
			(ext_en ? MTK_WDT_MODE_EXTEN : 0) | MTK_WDT_MODE_KEY);

	clrsetbits_le32(&mt8173_wdt->wdt_mode, MTK_WDT_MODE_EXT_POL,
			(ext_pol ? MTK_WDT_MODE_EXT_POL : 0) | MTK_WDT_MODE_KEY);

	clrsetbits_le32(&mt8173_wdt->wdt_mode, MTK_WDT_MODE_ENABLE,
			(wdt_en ? MTK_WDT_MODE_ENABLE : 0) | MTK_WDT_MODE_KEY);

	printk(BIOS_INFO, "%s  mode value=%#x, mode: %#x \n", __func__,
	       read32(&mt8173_wdt->wdt_mode), mode);
}

static void mtk_wdt_set_time_out_value(uint32_t value)
{
	uint32_t timeout;

	/*
	 * TimeOut = BitField 15:5
	 * Key = BitField  4:0 = 0x08
	 */

	/* sec * 32768 / 512 = sec * 64 = sec * 1 << 6 */
	timeout = (uint32_t)(value * (1 << 6));
	timeout = timeout << 5;
	write32(&mt8173_wdt->wdt_length, timeout | MTK_WDT_LENGTH_KEY);
}

void mtk_wdt_init(void)
{
	uint32_t wdt_sta;
	uint32_t rgu_status;

	/* Write Mode register will clear status register */
	wdt_sta = read32(&mt8173_wdt->wdt_status);

	if (wdt_sta & MTK_WDT_STATUS_HWWDT_RST)
		rgu_status = RE_BOOT_BY_WDT_HW;
	else if (wdt_sta & MTK_WDT_STATUS_SWWDT_RST)
		rgu_status = RE_BOOT_BY_WDT_SW;
	else
		rgu_status = RE_BOOT_REASON_UNKNOW;

	if (wdt_sta & MTK_WDT_STATUS_IRQWDT_RST)
		rgu_status |= RE_BOOT_WITH_INTTERUPT;

	if (wdt_sta & MTK_WDT_STATUS_SPM_THERMAL_RST)
		rgu_status |= RE_BOOT_BY_SPM_THERMAL;

	if (wdt_sta & MTK_WDT_STATUS_SPMWDT_RST)
		rgu_status |= RE_BOOT_BY_SPM;

	if (wdt_sta & MTK_WDT_STATUS_THERMAL_DIRECT_RST)
		rgu_status |= RE_BOOT_BY_THERMAL_DIRECT;

	if (wdt_sta & MTK_WDT_STATUS_DEBUGWDT_RST)
		rgu_status |= RE_BOOT_BY_DEBUG;

	if (wdt_sta & MTK_WDT_STATUS_SECURITY_RST)
		rgu_status |= RE_BOOT_BY_SECURITY;

	if (rgu_status) {
		printk(BIOS_INFO, "WDT reset\n");
		printk(BIOS_INFO, "RGU: rgu_staus:%d\n", rgu_status);
		printk(BIOS_INFO, "RGU STA: %#x\n", wdt_sta);
	} else
		printk(BIOS_INFO, "WDT does not trigger reboot\n");

	/* Setting timeout 20s */
	mtk_wdt_set_time_out_value(20);

	/* Config HW reboot mode */
	mtk_wdt_mode_config(0, 0, 1, 0, 0);
	write32(&mt8173_wdt->wdt_restart, MTK_WDT_RESTART_KEY);
}

int rgu_dram_reserved(int enable)
{
	clrsetbits_le32(&mt8173_wdt->wdt_mode, MTK_WDT_MODE_DDR_RESERVE,
			(enable ? MTK_WDT_MODE_DDR_RESERVE : 0) | MTK_WDT_MODE_KEY);
	printk(BIOS_INFO, "RGU %s:MTK_WDT_MODE(%#x)\n", __func__, read32(&mt8173_wdt->wdt_mode));
	return 0;
}

void rgu_swsys_reset(void)
{
	setbits_le32(&mt8173_wdt->wdt_swsysrst, MTK_WDT_SWSYS_RST_KEY | 0x80);
	/*
	 * set bit[7] of mt8173_wdt->wdt_swsysrst to 1 will reset MD32(a micro chip).
	 * we delay 1ms to wait reset md32 done.
	 */
	udelay(RESET_MD32_DELAY);
	/* clear reset bit of md32*/
	clrsetbits_le32(&mt8173_wdt->wdt_swsysrst,
			0x80, MTK_WDT_SWSYS_RST_KEY);
	printk(BIOS_INFO, "rgu pl md reset\n");
}
