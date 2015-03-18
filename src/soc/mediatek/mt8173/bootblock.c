/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 Google Inc.
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

#include <arch/cache.h>
#include <arch/exception.h>
#include <arch/hlt.h>
#include <arch/stages.h>
#include <arch/io.h>
#include <bootblock_common.h>
#include <cbfs.h>
#include <console/console.h>
#include <timestamp.h>
#include <vendorcode/google/chromeos/chromeos.h>

#include <soc/addressmap.h>
#include <soc/gpio.h>
#include <soc/i2c.h>
#include <soc/mt8173.h>
#include <soc/pmic_wrap_init.h>
#include <soc/pmic.h>
#include <soc/da9212.h>

void main(void)
{
	const char *stage_name = "fallback/romstage";
	void *entry;

	timestamp_early_init(0);
	/* Clear UART0 power down signal */
	clrbits_le32((void *)AP_PERI_GLOBALCON_PDN0, UART0_PDN);

	if (CONFIG_BOOTBLOCK_CONSOLE) {
		console_init();
		exception_init();
		printk(BIOS_INFO, "MT8173: Bootblock here\n");
	}

	bootblock_mainboard_init();

	if (IS_ENABLED(CONFIG_VBOOT2_VERIFY_FIRMWARE)) {

		mt_gpio_init();
		pwrap_init_preloader();

		/* init pmic i2c interface and pmic */
		ext_buck_en(1);
		/* i2c_hw_init(); */
		pmic_init();

		/* Setup TPM */
		mtk_i2c_init(0x11009000, 2, 0, 0x20, 0);

		exception_init();

		entry = vboot2_verify_firmware();
	}

	else if (IS_ENABLED(CONFIG_VBOOT_VERIFY_FIRMWARE)) {
		entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA, stage_name);
	}

	if (entry)
		stage_exit(entry);
	hlt();
}

extern void _start(void);

void bootblock_soc_init(void)
{
}
