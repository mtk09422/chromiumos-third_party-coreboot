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

#define NUM_EMI_RECORD (2)

int num_of_emi_records = NUM_EMI_RECORD;

EMI_SETTINGS emi_settings[] = {
	/* H9CKNNNBKTMRPR */
	{ /* 1 Rank */
		0x0,			/* sub_version */
		0x0003,			/* TYPE */
		0,			/* EMMC ID/FW ID checking length */
		0,			/* FW length */
		{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
		  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 }, /* NAND_EMMC_ID */
		{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 }, /* FW_ID */
		0x20102017,		/* EMI_CONA_VAL */
		0x00000000,		/* EMI_CONH_VAL */
		0xAAFD478C,		/* DRAMC_ACTIM_VAL */
		0x11000000,		/* DRAMC_GDDR3CTL1_VAL */
		0x00048403,		/* DRAMC_CONF1_VAL */
		0x000063B1,		/* DRAMC_DDR2CTL_VAL */
		0xBFC70401,		/* DRAMC_TEST2_3_VAL */
		0x030000A9,		/* DRAMC_CONF2_VAL */
		0xD1976442,		/* DRAMC_PD_CTRL_VAL */
		0x91001F59,		/* DRAMC_ACTIM1_VAL */
		0x21000000,		/* DRAMC_MISCTL0_VAL */
		0x000025E1,		/* DRAMC_ACTIM05T_VAL */
		0x002156C1,		/* DRAM_CRKCFG_VAL */
		0x2801110D,		/* DRAMC_TEST2_4_VAL */
		{ 0x80000000, 0, 0, 0 },	/* DRAM RANK SIZE */
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	/* reserved 10 */
		{{ 0x00830001,		/* LPDDR3_MODE_REG1 */
		   0x001C0002,		/* LPDDR3_MODE_REG2 */
		   0x00020003,		/* LPDDR3_MODE_REG3 */
		   0x00000006,		/* LPDDR3_MODE_REG5 */
		   0x00FF000A,		/* LPDDR3_MODE_REG10 */
		   0x0000003F } }	/* LPDDR3_MODE_REG63 */
	},
	{ /* 2 Rank */
		0x0,			/* sub_version */
		0x0003,			/* TYPE */
		0,			/* EMMC ID/FW ID checking length */
		0,			/* FW length */
		{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
		  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 }, /* NAND_EMMC_ID */
		{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 }, /* FW_ID */

		/* Dual channels. 14 bits row and 10 bits column. */
		0x50535057,		/* EMI_CONA_VAL */

		0x00000000,		/* EMI_CONH_VAL */
		0xAAFD478C,		/* DRAMC_ACTIM_VAL */
		0x11000000,		/* DRAMC_GDDR3CTL1_VAL */
		0x00048403,		/* DRAMC_CONF1_VAL */
		0x000063B1,		/* DRAMC_DDR2CTL_VAL */
		0xBFC70401,		/* DRAMC_TEST2_3_VAL */
		0x030000A9,		/* DRAMC_CONF2_VAL */
		0xD1976442,		/* DRAMC_PD_CTRL_VAL */
		0x91001F59,		/* DRAMC_ACTIM1_VAL*/
		0x21000000,		/* DRAMC_MISCTL0_VAL*/
		0x000025E1,		/* DRAMC_ACTIM05T_VAL*/
		0x002156C1,		/* DRAM_CRKCFG_VAL*/
		0x2801110D,		/* DRAMC_TEST2_4_VAL*/
		{ 0x40000000, 0x40000000, 0, 0 },	/* DRAM RANK SIZE */
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	/* reserved 10 */
		{{ 0x00830001,		/* LPDDR3_MODE_REG1 */
		   0x001C0002,		/* LPDDR3_MODE_REG2 */
		   0x00020003,		/* LPDDR3_MODE_REG3 */
		   0x00000006,		/* LPDDR3_MODE_REG5 */
		   0x00FF000A,		/* LPDDR3_MODE_REG10 */
		   0x0000003F } }	/* LPDDR3_MODE_REG63 */
	}
};
