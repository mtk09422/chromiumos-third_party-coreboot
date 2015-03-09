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

#ifndef SOC_MEDIATEK_MT8173_EMI_H
#define SOC_MEDIATEK_MT8173_EMI_H

int get_dram_rank_nr(void);
void get_dram_rank_size(unsigned int dram_rank_size[]);
void mt_set_emi(void);
int mt_get_dram_freq_setting(void);

/* DDR type */
enum {
	DDR1 = 1,
	LPDDR2 = 2,
	LPDDR3 = 3,
	PCDDR3 = 4
};

typedef struct {
	int	sub_version;		/* sub_version: 0x1 for new version */
	int	type;			/* 	0x0000 : Invalid
						0x0001 : Discrete DDR1
						0x0002 : Discrete LPDDR2
						0x0003 : Discrete LPDDR3
						0x0004 : Discrete PCDDR3
						0x0101 : MCP(NAND+DDR1)
						0x0102 : MCP(NAND+LPDDR2)
						0x0103 : MCP(NAND+LPDDR3)
						0x0104 : MCP(NAND+PCDDR3)
						0x0201 : MCP(eMMC+DDR1)
						0x0202 : MCP(eMMC+LPDDR2)
						0x0203 : MCP(eMMC+LPDDR3)
						0x0204 : MCP(eMMC+PCDDR3)
				      */
	int   id_length;              /* EMMC and NAND ID checking length */
	int   fw_id_length;           /* FW ID checking length */
	char  ID[16];
	char  fw_id[8];               /* To save fw id */
	int   EMI_CONA_VAL;           /* @0x3000 */
	int   EMI_CONH_VAL;
	int   DRAMC_ACTIM_VAL;        /* @0x4000 */
	int   DRAMC_GDDR3CTL1_VAL;    /* @0x40F4 */
	int   DRAMC_CONF1_VAL;        /* @0x4004 */
	int   DRAMC_DDR2CTL_VAL;      /* @0x407C */
	int   DRAMC_TEST2_3_VAL;      /* @0x4044 */
	int   DRAMC_CONF2_VAL;        /* @0x4008 */
	int   DRAMC_PD_CTRL_VAL;      /* @0x41DC */
	int   DRAMC_ACTIM1_VAL;       /* @0x41E8 */
	int   DRAMC_MISCTL0_VAL;      /* @0x40FC */
	int   DRAMC_ACTIM05T_VAL;     /* @0x41F8 */
	int   DRAMC_RKCFG_VAL;
	int   DRAMC_TEST2_4_VAL;
	int   DRAM_RANK_SIZE[4];
	int   reserved[10];
	union {
		struct {
			int   iLPDDR2_MODE_REG_1;
			int   iLPDDR2_MODE_REG_2;
			int   iLPDDR2_MODE_REG_3;
			int   iLPDDR2_MODE_REG_5;
			int   iLPDDR2_MODE_REG_10;
			int   iLPDDR2_MODE_REG_63;
		};
		struct {
			int   iDDR1_MODE_REG;
			int   iDDR1_EXT_MODE_REG;
		};
		struct {
			int   iPCDDR3_MODE_REG0;
			int   iPCDDR3_MODE_REG1;
			int   iPCDDR3_MODE_REG2;
			int   iPCDDR3_MODE_REG3;
		};
		struct {
			int   iLPDDR3_MODE_REG_1;
			int   iLPDDR3_MODE_REG_2;
			int   iLPDDR3_MODE_REG_3;
			int   iLPDDR3_MODE_REG_5;
			int   iLPDDR3_MODE_REG_10;
			int   iLPDDR3_MODE_REG_63;
		};
	};
} EMI_SETTINGS;

int mt_get_dram_type(void);
/* 0: invalid */
/* 1: mDDR1 */
/* 2: LPDDR2 */
/* 3: LPDDR3 */
/* 4: PCDDR3 */

typedef enum {
	CHIP_6595 = 1,
	CHIP_6595M
} CHIP_TYPE;

#endif
