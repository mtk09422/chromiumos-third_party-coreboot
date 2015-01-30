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

#include <soc/mt8135.h>

#define AP_M4U_SRAM_CTRL    0x10205100 + 0x4
#define AUDIO_TOP_CON0      0x12070000 + 0x0

#define mt_delay() \
	do {	\
		int i;	\
		dsb();	\
		dmb();	\
		for (i = 0; i < 1000; i++) barrier(); \
	} while (0)

void main(void)
{
	const char *stage_name = "fallback/romstage";
	void *entry;

	timestamp_early_init(0);
	bootblock_mainboard_init();

	/* enable M4U SRAM (0x1202_0000 - 0x1202_5FFF) */
	setbits_le32((void *)AP_M4U_SRAM_CTRL, 0x10);
	clrbits_le32((void *)AUDIO_TOP_CON0, 0x4);

	entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA, stage_name);

	if (entry)
		stage_exit(entry);
	hlt();
}

extern void _start(void);

void bootblock_soc_init(void)
{
	*BOOTROM_PWR_ADDR = (uint32_t)&_start;
	*BOOTROM_PWR_CTRL |= 1;
	printk(BIOS_INFO, "wait for CA15 BUCK power on 0x%p (%x)\n", BOOTROM_PWR_ADDR, *BOOTROM_PWR_ADDR);

	/* wait for CA15 BUCK power on */
	while (g_ca15_ready == 0) ;

	/* 0. Enable SPM APB bus and init setting */
	*POWERON_CONFIG_SET = 0x0B160001;

	*SPM_MD1_PWR_CON = *SPM_MD1_PWR_CON | EXT_CA15_OFF;
	*RGUCFG = *RGUCFG & ~0x3;
	*CONFIG_RES = *CONFIG_RES | 0x3b;
	*CA15_RST_CTL = *CA15_RST_CTL & ~CA15_L2RSTDISABLE;

	/* Coherence setting */
	*CA7_CCI400_DVM_EN = 3;
	*CA15_CA7_CONNECT |= 0xC000;

	*CA15_MISC_DBG = 2;
	*CA15_CCI400_DVM_EN |= 3;

	/* 1. Power on CA15 CPUTOP */
	/* Related registers: CA15_CPUTOP_PWR_CON / CA15_L2_PWR_CON */
	*SPM_CA15_CPUTOP_PWR_CON = *SPM_CA15_CPUTOP_PWR_CON & ~PWR_RST_B;

	*SPM_CA15_CPUTOP_PWR_CON = *SPM_CA15_CPUTOP_PWR_CON | PWR_ON;
	while ((*SPM_PWR_STATUS & CA15_CPUTOP) != CA15_CPUTOP) ;
	*SPM_CA15_CPUTOP_PWR_CON = *SPM_CA15_CPUTOP_PWR_CON | PWR_ON_S;
	while ((*SPM_PWR_STATUS_S & CA15_CPUTOP) != CA15_CPUTOP) ;

	*SPM_CA15_CPUTOP_PWR_CON = *SPM_CA15_CPUTOP_PWR_CON & ~PWR_CLK_DIS;
	*SPM_CA15_CPUTOP_PWR_CON = *SPM_CA15_CPUTOP_PWR_CON & ~PWR_ISO;
	*SPM_CA15_CPUTOP_PWR_CON = *SPM_CA15_CPUTOP_PWR_CON | PWR_RST_B;

	*SPM_CA15_L2_PWR_CON = *SPM_CA15_L2_PWR_CON & ~CA15_L2_ISO;  /* add */
	*SPM_CA15_L2_PWR_CON = *SPM_CA15_L2_PWR_CON & ~CA15_L2_PDN;
	while ((*SPM_CA15_L2_PWR_CON & CA15_L2_PDN_ACK) != 0) ;

	*SPM_CA15_CPUTOP_PWR_CON = *SPM_CA15_CPUTOP_PWR_CON | SRAM_ISOINT_B;
	*SPM_CA15_CPUTOP_PWR_CON = *SPM_CA15_CPUTOP_PWR_CON & ~SRAM_CKISO;

	/* // Power on sequence: For CPU1 */
	*SPM_CA15_CPU1_PWR_CON = *SPM_CA15_CPU1_PWR_CON & ~PWR_RST_B;

	*SPM_CA15_CPU1_PWR_CON = *SPM_CA15_CPU1_PWR_CON | PWR_ON;
	while ((*SPM_PWR_STATUS & CA15_CPU1) != CA15_CPU1) ;
	*SPM_CA15_CPU1_PWR_CON = *SPM_CA15_CPU1_PWR_CON | PWR_ON_S;
	while ((*SPM_PWR_STATUS_S & CA15_CPU1) != CA15_CPU1) ;

	*SPM_CA15_CX1_PWR_CON = *SPM_CA15_CX1_PWR_CON | PWR_ON;
	while ((*SPM_PWR_STATUS & CA15_CX1) != CA15_CX1) ;
	*SPM_CA15_CX1_PWR_CON = *SPM_CA15_CX1_PWR_CON | PWR_ON_S;
	while ((*SPM_PWR_STATUS_S & CA15_CX1) != CA15_CX1) ;

	*SPM_CA15_L1_PWR_CON = *SPM_CA15_L1_PWR_CON & ~CPU1_L1_PDN_ISO;
	*SPM_CA15_L1_PWR_CON = *SPM_CA15_L1_PWR_CON & ~CPU1_L1_PDN;
	while ((*SPM_CA15_L1_PWR_CON & CPU1_L1_PDN_ACK) != 0) ;

	*SPM_CA15_CX1_PWR_CON = *SPM_CA15_CX1_PWR_CON & ~PWR_CLK_DIS;
	*SPM_CA15_CPU1_PWR_CON = *SPM_CA15_CPU1_PWR_CON & ~PWR_CLK_DIS;

	*SPM_CA15_CX1_PWR_CON = *SPM_CA15_CX1_PWR_CON & ~PWR_ISO;
	*SPM_CA15_CPU1_PWR_CON = *SPM_CA15_CPU1_PWR_CON & ~PWR_ISO;

	mt_delay();
	*SPM_CA15_CX1_PWR_CON = *SPM_CA15_CX1_PWR_CON | PWR_RST_B;
	*SPM_CA15_CX1_PWR_CON = *SPM_CA15_CX1_PWR_CON | SRAM_ISOINT_B;
	*SPM_CA15_CX1_PWR_CON = *SPM_CA15_CX1_PWR_CON & ~SRAM_CKISO;

	mt_delay();

	*SPM_CA15_CPU1_PWR_CON = *SPM_CA15_CPU1_PWR_CON | SRAM_ISOINT_B;
	*SPM_CA15_CPU1_PWR_CON = *SPM_CA15_CPU1_PWR_CON & ~SRAM_CKISO;

	*SPM_CA15_CPU1_PWR_CON = *SPM_CA15_CPU1_PWR_CON | PWR_RST_B;

	/* Power on sequence: For CPU0 */
	*SPM_CA15_CPU0_PWR_CON = *SPM_CA15_CPU0_PWR_CON & ~PWR_RST_B;

	*SPM_CA15_CPU0_PWR_CON = *SPM_CA15_CPU0_PWR_CON | PWR_ON;
	while ((*SPM_PWR_STATUS & CA15_CPU0) != CA15_CPU0) ;
	*SPM_CA15_CPU0_PWR_CON = *SPM_CA15_CPU0_PWR_CON | PWR_ON_S;
	while ((*SPM_PWR_STATUS_S & CA15_CPU0) != CA15_CPU0) ;

	*SPM_CA15_CX0_PWR_CON = *SPM_CA15_CX0_PWR_CON | PWR_ON;
	while ((*SPM_PWR_STATUS & CA15_CX0) != CA15_CX0) ;
	*SPM_CA15_CX0_PWR_CON = *SPM_CA15_CX0_PWR_CON | PWR_ON_S;
	while ((*SPM_PWR_STATUS_S & CA15_CX0) != CA15_CX0) ;

	*SPM_CA15_L1_PWR_CON = *SPM_CA15_L1_PWR_CON & ~CPU0_L1_PDN_ISO;
	*SPM_CA15_L1_PWR_CON = *SPM_CA15_L1_PWR_CON & ~CPU0_L1_PDN;
	while ((*SPM_CA15_L1_PWR_CON & CPU0_L1_PDN_ACK) != 0) ;

	*SPM_CA15_CX0_PWR_CON = *SPM_CA15_CX0_PWR_CON & ~PWR_CLK_DIS;
	*SPM_CA15_CPU0_PWR_CON = *SPM_CA15_CPU0_PWR_CON & ~PWR_CLK_DIS;

	*SPM_CA15_CX0_PWR_CON = *SPM_CA15_CX0_PWR_CON & ~PWR_ISO;
	*SPM_CA15_CPU0_PWR_CON = *SPM_CA15_CPU0_PWR_CON & ~PWR_ISO;

	mt_delay();
	/* udelay(1); // >16 CPU CLK */

	*SPM_CA15_CX0_PWR_CON = *SPM_CA15_CX0_PWR_CON | SRAM_ISOINT_B;
	*SPM_CA15_CX0_PWR_CON = *SPM_CA15_CX0_PWR_CON & ~SRAM_CKISO;

	*SPM_CA15_CX0_PWR_CON = *SPM_CA15_CX0_PWR_CON | PWR_RST_B;

	mt_delay();
	*SPM_CA15_CPU0_PWR_CON = *SPM_CA15_CPU0_PWR_CON | PWR_RST_B;
	*SPM_CA15_CPU0_PWR_CON = *SPM_CA15_CPU0_PWR_CON | SRAM_ISOINT_B;
	*SPM_CA15_CPU0_PWR_CON = *SPM_CA15_CPU0_PWR_CON & ~SRAM_CKISO;
}
