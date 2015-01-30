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
#include <soc/emi.h>

#define NUM_EMI_RECORD (1)

int num_of_emi_records = NUM_EMI_RECORD;

EMI_SETTINGS emi_settings[] = {
	/* COMMON_8 */
	{
		0x0,			/* sub_version */
		0x0003,			/* TYPE */
		9,			/* EMMC ID/FW ID checking length */
		1,			/* FW length */

		{ 0x70, 0x01, 0x00, 0x4D, 0x4D, 0x43, 0x30, 0x34,
		  0x47, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 }, /* NAND_EMMC_ID */
		{ 0x05, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 }, /* FW_ID */

		0x0002F3AE,		/* EMI_CONA_VAL */
		0xaa00aa00,		/* DRAMC_DRVCTL0_VAL */
		0xaa00aa00,		/* DRAMC_DRVCTL1_VAL */
		0xC0100900,		/* DRAMC_MCKDLY_VAL */
		0x558A46A5,		/* DRAMC_ACTIM_VAL */
		0x11000000,		/* DRAMC_GDDR3CTL1_VAL */
		0xF07486a1,		/* DRAMC_CONF1_VAL */
		0x40002271,		/* DRAMC_DDR2CTL_VAL */
		0x9f0a0481,		/* DRAMC_TEST2_3_VAL */
		0x03046950,		/* DRAMC_CONF2_VAL */
		0x15942842,		/* DRAMC_PD_CTRL_VAL */
		0x0000BBBB,		/* DRAMC_PADCTL3_VAL */
		0xEEEEEEEE,		/* DRAMC_DQODLY_VAL */
		0x00000000,		/* DRAMC_ADDR_OUTPUT_DLY */
		0x00000000,		/* DRAMC_CLK_OUTPUT_DLY */
		0x00000250,		/* DRAMC_ACTIM1_VAL */
		0x07000000,		/* DRAMC_MISCTL0_VAL */

		{ 0x80000000, 0, 0, 0 },		/* DRAM RANK SIZE */
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	/* reserved 10 */
		{{ 0x00001B61,		/* DDR3_MODE_REG0 */
		   0x00002000,		/* DDR3_MODE_REG1 */
		   0x00004010,		/* DDR3_MODE_REG2 */
		   0x00006000 } }	/* DDR3_MODE_REG3 */
	}
};
