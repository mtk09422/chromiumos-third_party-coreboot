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

#ifndef _PL_MT_PMIC_H_
#define _PL_MT_PMIC_H_

/*
 * PMIC define
 */
enum {
	PMIC6391_E1_CID_CODE = 0x1091,
	PMIC6391_E2_CID_CODE = 0x2091,
	PMIC6397_E1_CID_CODE = 0x1097,
	PMIC6397_E2_CID_CODE = 0x2097,
	PMIC6397_E3_CID_CODE = 0x3097,
	PMIC6397_E4_CID_CODE = 0x4097
};

typedef enum {
	CHARGER_UNKNOWN = 0,
	STANDARD_HOST,
	CHARGING_HOST,
	NONSTANDARD_CHARGER,
	STANDARD_CHARGER,
} CHARGER_TYPE;

/*
 * PMIC Register Index
 */

/* PCHR Register Definition */
enum {
	CHR_CON0 = 0x0000,
	CHR_CON13 = 0x001A,
	CHR_CON22 = 0x002C,
	CHR_CON23 = 0x002E,
};

/* TOP Register Definition */
enum{
	CID	= 0x0100,
	TOP_CKPDN = 0x0102,
	TOP_CKPDN2 = 0x0108,
	TOP_RST_CON = 0x0114,
	WRP_CKPDN = 0x011A,
	TOP_RST_MISC = 0x0126,
	TOP_CKCON1 = 0x0128,
	TOP_CKCON2 = 0x012A,
	TOP_CKTST1 = 0x012C,
	TOP_CKTST2 = 0x012E,
	OC_DEG_EN = 0x0130,
	CHRSTATUS = 0x0144,
	OCSTATUS0 = 0x0146,
	OCSTATUS1 = 0x0148,
	OCSTATUS2 = 0x014A,
	TOP_CKPDN3 = 0x01CE,
	TOP_CKCON3 = 0x01D4
};

/* FQMTR Register Definition */
enum{
	FQMTR_CON0 = 0x0188,
	FQMTR_CON1 = 0x018A,
	FQMTR_CON2 = 0x018C
};

/* EFUSE Register Definition */
enum{
	EFUSE_DOUT_0_15 = 0x01C4,
	EFUSE_DOUT_16_31 = 0x01C6,
	EFUSE_DOUT_32_47 = 0x01C8,
	EFUSE_DOUT_48_63 = 0x01CA,
	EFUSE_DOUT_64_79 = 0x01D6,
	EFUSE_DOUT_80_95 = 0x01D8,
	EFUSE_DOUT_96_111 = 0x01DA,
	EFUSE_DOUT_112_127 = 0x01DC,
	EFUSE_DOUT_128_143 = 0x01DE,
	EFUSE_DOUT_144_159 = 0x01E0,
	EFUSE_DOUT_160_175 = 0x01E2,
	EFUSE_DOUT_176_191 = 0x01E4,
	EFUSE_DOUT_192_207 = 0x01E6,
	EFUSE_DOUT_208_223 = 0x01E8,
	EFUSE_DOUT_224_239 = 0x01EA,
	EFUSE_DOUT_240_255 = 0x01EC,
	EFUSE_DOUT_256_271 = 0x01EE,
	EFUSE_DOUT_272_287 = 0x01F0,
	EFUSE_DOUT_288_303 = 0x01F2,
	EFUSE_DOUT_304_319 = 0x01F4
};

/* BUCK Register Definition */
enum{
	BUCK_CON0 = 0x0200,
	BUCK_CON1 = 0x0202,
	BUCK_CON2 = 0x0204,
	BUCK_CON3 = 0x0206,
	BUCK_CON4 = 0x0208,
	BUCK_CON5 = 0x020A,
	BUCK_CON6 = 0x020C,
	BUCK_CON7 = 0x020E,
	BUCK_CON8 = 0x0210,
	BUCK_CON9 = 0x0212,
	VCA15_CON0 = 0x0214,
	VCA15_CON1 = 0x0216,
	VCA15_CON2 = 0x0218,
	VCA15_CON3 = 0x021A,
	VCA15_CON4 = 0x021C,
	VCA15_CON5 = 0x021E,
	VCA15_CON6 = 0x0220,
	VCA15_CON7 = 0x0222,
	VCA15_CON8 = 0x0224,
	VCA15_CON9 = 0x0226,
	VCA15_CON10 = 0x0228,
	VCA15_CON11 = 0x022A,
	VCA15_CON12 = 0x022C,
	VCA15_CON13 = 0x022E,
	VCA15_CON14 = 0x0230,
	VCA15_CON15 = 0x0232,
	VCA15_CON16 = 0x0234,
	VCA15_CON17 = 0x0236,
	VCA15_CON18 = 0x0238,
	VSRMCA15_CON0 = 0x023A,
	VSRMCA15_CON1 = 0x023C,
	VSRMCA15_CON2 = 0x023E,
	VSRMCA15_CON3 = 0x0240,
	VSRMCA15_CON4 = 0x0242,
	VSRMCA15_CON5 = 0x0244,
	VSRMCA15_CON6 = 0x0246,
	VSRMCA15_CON7 = 0x0248,
	VSRMCA15_CON8 = 0x024A,
	VSRMCA15_CON9 = 0x024C,
	VSRMCA15_CON10 = 0x024E,
	VSRMCA15_CON11 = 0x0250,
	VSRMCA15_CON12 = 0x0252,
	VSRMCA15_CON13 = 0x0254,
	VSRMCA15_CON14 = 0x0256,
	VSRMCA15_CON15 = 0x0258,
	VSRMCA15_CON16 = 0x025A,
	VSRMCA15_CON17 = 0x025C,
	VSRMCA15_CON18 = 0x025E,
	VSRMCA15_CON19 = 0x0260,
	VSRMCA15_CON20 = 0x0262,
	VSRMCA15_CON21 = 0x0264,
	VCORE_CON0 = 0x0266,
	VCORE_CON1 = 0x0268,
	VCORE_CON2 = 0x026A,
	VCORE_CON3 = 0x026C,
	VCORE_CON4 = 0x026E,
	VCORE_CON5 = 0x0270,
	VCORE_CON6 = 0x0272,
	VCORE_CON7 = 0x0274,
	VCORE_CON8 = 0x0276,
	VCORE_CON9 = 0x0278,
	VCORE_CON10 = 0x027A,
	VCORE_CON11 = 0x027C,
	VCORE_CON12 = 0x027E,
	VCORE_CON13 = 0x0280,
	VCORE_CON14 = 0x0282,
	VCORE_CON15 = 0x0284,
	VCORE_CON16 = 0x0286,
	VCORE_CON17 = 0x0288,
	VCORE_CON18 = 0x028A,
	VGPU_CON0 = 0x028C,
	VGPU_CON1 = 0x028E,
	VGPU_CON2 = 0x0290,
	VGPU_CON3 = 0x0292,
	VGPU_CON4 = 0x0294,
	VGPU_CON5 = 0x0296,
	VGPU_CON6 = 0x0298,
	VGPU_CON7 = 0x029A,
	VGPU_CON8 = 0x029C,
	VGPU_CON9 = 0x029E,
	VGPU_CON10 = 0x02A0,
	VGPU_CON11 = 0x02A2,
	VGPU_CON12 = 0x02A4,
	VGPU_CON13 = 0x02A6,
	VGPU_CON14 = 0x02A8,
	VGPU_CON15 = 0x02AA,
	VGPU_CON16 = 0x02AC,
	VGPU_CON17 = 0x02AE,
	VGPU_CON18 = 0x02B0,
	VIO18_CON0 = 0x0300,
	VIO18_CON1 = 0x0302,
	VIO18_CON2 = 0x0304,
	VIO18_CON3 = 0x0306,
	VIO18_CON4 = 0x0308,
	VIO18_CON5 = 0x030A,
	VIO18_CON6 = 0x030C,
	VIO18_CON7 = 0x030E,
	VIO18_CON8 = 0x0310,
	VIO18_CON9 = 0x0312,
	VIO18_CON10 = 0x0314,
	VIO18_CON11 = 0x0316,
	VIO18_CON12 = 0x0318,
	VIO18_CON13 = 0x031A,
	VIO18_CON14 = 0x031C,
	VIO18_CON15 = 0x031E,
	VIO18_CON16 = 0x0320,
	VIO18_CON17 = 0x0322,
	VIO18_CON18 = 0x0324,
	VPCA7_CON0  = 0x0326,
	VPCA7_CON1  = 0x0328,
	VPCA7_CON2  = 0x032A,
	VPCA7_CON3  = 0x032C,
	VPCA7_CON4  = 0x032E,
	VPCA7_CON5  = 0x0330,
	VPCA7_CON6  = 0x0332,
	VPCA7_CON7  = 0x0334,
	VPCA7_CON8  = 0x0336,
	VPCA7_CON9  = 0x0338,
	VPCA7_CON10 = 0x033A,
	VPCA7_CON11 = 0x033C,
	VPCA7_CON12 = 0x033E,
	VPCA7_CON13 = 0x0340,
	VPCA7_CON14 = 0x0342,
	VPCA7_CON15 = 0x0344,
	VPCA7_CON16 = 0x0346,
	VPCA7_CON17 = 0x0348,
	VPCA7_CON18 = 0x034A,
	VSRMCA7_CON0 =  0x034C,
	VSRMCA7_CON1 =  0x034E,
	VSRMCA7_CON2 =  0x0350,
	VSRMCA7_CON3 =  0x0352,
	VSRMCA7_CON4 =  0x0354,
	VSRMCA7_CON5 =  0x0356,
	VSRMCA7_CON6 =  0x0358,
	VSRMCA7_CON7 =  0x035A,
	VSRMCA7_CON8 =  0x035C,
	VSRMCA7_CON9 =  0x035E,
	VSRMCA7_CON10 = 0x0360,
	VSRMCA7_CON11 = 0x0362,
	VSRMCA7_CON12 = 0x0364,
	VSRMCA7_CON13 = 0x0366,
	VSRMCA7_CON14 = 0x0368,
	VSRMCA7_CON15 = 0x036A,
	VSRMCA7_CON16 = 0x036C,
	VSRMCA7_CON17 = 0x036E,
	VSRMCA7_CON18 = 0x0370,
	VSRMCA7_CON19 = 0x0372,
	VSRMCA7_CON20 = 0x0374,
	VSRMCA7_CON21 = 0x0376,
	VDRM_CON0 = 0x0378,
	VDRM_CON1 = 0x037A,
	VDRM_CON2 = 0x037C,
	VDRM_CON3 = 0x037E,
	VDRM_CON4 = 0x0380,
	VDRM_CON5 = 0x0382,
	VDRM_CON6 = 0x0384,
	VDRM_CON7 = 0x0386,
	VDRM_CON8 = 0x0388,
	VDRM_CON9 = 0x038A,
	VDRM_CON10 = 0x038C,
	VDRM_CON11 = 0x038E,
	VDRM_CON12 = 0x0390,
	VDRM_CON13 = 0x0392,
	VDRM_CON14 = 0x0394,
	VDRM_CON15 = 0x0396,
	VDRM_CON16 = 0x0398,
	VDRM_CON17 = 0x039A,
	VDRM_CON18 = 0x039C,
	BUCK_K_CON0 = 0x039E,
	BUCK_K_CON1 = 0x03A0
};

/* DIGLDO Register Definition */
enum{
	DIGLDO_CON0 = 0x0410,
	DIGLDO_CON1 = 0x0412,
	DIGLDO_CON2 = 0x0414,
	DIGLDO_CON3 = 0x0416,
	DIGLDO_CON4 = 0x0418,
	DIGLDO_CON5 = 0x041A,
	DIGLDO_CON6 = 0x041C,
	DIGLDO_CON7 = 0x041E,
	DIGLDO_CON8 = 0x0420,
	DIGLDO_CON9 = 0x0422,
	DIGLDO_CON10  = 0x0424,
	DIGLDO_CON11  = 0x0426,
	DIGLDO_CON12  = 0x0428,
	DIGLDO_CON13  = 0x042A,
	DIGLDO_CON14  = 0x042C,
	DIGLDO_CON15  = 0x042E,
	DIGLDO_CON16  = 0x0430,
	DIGLDO_CON17  = 0x0432,
	DIGLDO_CON18  = 0x0434,
	DIGLDO_CON19  = 0x0436,
	DIGLDO_CON20  = 0x0438,
	DIGLDO_CON21  = 0x043A,
	DIGLDO_CON22  = 0x043C,
	DIGLDO_CON23  = 0x043E,
	DIGLDO_CON24  = 0x0440,
	DIGLDO_CON25  = 0x0442,
	DIGLDO_CON26  = 0x0444,
	DIGLDO_CON27  = 0x0446,
	DIGLDO_CON28  = 0x0448,
	DIGLDO_CON29  = 0x044A,
	DIGLDO_CON30  = 0x044C,
	DIGLDO_CON31  = 0x044E,
	DIGLDO_CON32  = 0x0450,
	DIGLDO_CON33  = 0x045A
};

/* DCXO Register Definition */
enum{
	RG_DCXO_CON0	= 0x83a,
	RG_DCXO_CON1	= 0x83c,
	RG_DCXO_CON2	= 0x83e,
	RG_DCXO_CON3	= 0x840,
	RG_DCXO_MANUAL_CON3	= 0x842,
	RG_DCXO_MANUAL_CON1	= 0x844,
	RG_DCXO_MANUAL_CON2	= 0x846,
	RG_DCXO_ANALOG_CON0	= 0x848,
	RG_DCXO_ANALOG_CON1	= 0x84a,
	RG_DCXO_ANALOG_CON2	= 0x84c,
	RG_DCXO_ANALOG_CON3	= 0x84e,
	RG_DCXO_ANALOG_CON4	= 0x850
};

/*
 * Register MASK and SHIFT Definition
 * MASK is HEX, SHIFT is Integer
 */
/* PCHR MASK and SHIFT Definition */
enum {
	PMIC_RGS_CHRDET_MASK = 0x1,
	PMIC_RGS_CHRDET_SHIFT = 5,
	PMIC_RG_CHRWDT_EN_MASK = 0x1,
	PMIC_RG_CHRWDT_EN_SHIFT = 4,
	PMIC_RG_LOW_ICH_DB_MASK = 0x3F,
	PMIC_RG_LOW_ICH_DB_SHIFT = 0,
	PMIC_RG_ULC_DET_EN_MASK = 0x1,
	PMIC_RG_ULC_DET_EN_SHIFT = 7
};

/* TOP MASK and SHIFT Definition */
enum{
	PMIC_CID_MASK = 0xFFFF,
	PMIC_CID_SHIFT = 0,
	PMIC_RG_FQMTR_PDN_SHIFT = 1,
	PMIC_RG_AP_RST_DIS_MASK = 0x1,
	PMIC_RG_AP_RST_DIS_SHIFT = 0,
	PMIC_RG_RST_PART_SEL_MASK = 0x1,
	PMIC_RG_RST_PART_SEL_SHIFT = 4,
	PMIC_RG_TOP_RST_MISC_RSV_3_MASK = 0x1,
	PMIC_RG_TOP_RST_MISC_RSV_3_SHIFT = 3,
	PMIC_RG_STRUP_MAN_RST_EN_MASK = 0x1,
	PMIC_RG_STRUP_MAN_RST_EN_SHIFT = 2,
	PMIC_RG_SYSRSTB_EN_MASK = 0x1,
	PMIC_RG_SYSRSTB_EN_SHIFT = 1,
	PMIC_PWRKEY_DEB_MASK = 0x1,
	PMIC_PWRKEY_DEB_SHIFT = 3,
	PMIC_HOMEKEY_DEB_MASK = 0x1,
	PMIC_HOMEKEY_DEB_SHIFT = 4
};

/* EFUSE MASK and SHIFT Definition */
enum{
	PMIC_RG_EFUSE_ADDR_MASK = 0x3F,
	PMIC_RG_EFUSE_ADDR_SHIFT = 0,
	PMIC_RG_EFUSE_PROG_MASK = 0x1F,
	PMIC_RG_EFUSE_PROG_SHIFT = 0,
	PMIC_RG_EFUSE_EN_MASK = 0x1,
	PMIC_RG_EFUSE_EN_SHIFT = 0,
	PMIC_RG_EFUSE_PKEY_MASK = 0xFFFF,
	PMIC_RG_EFUSE_PKEY_SHIFT = 0,
	PMIC_RG_EFUSE_RD_TRIG_MASK = 0x1,
	PMIC_RG_EFUSE_RD_TRIG_SHIFT = 0,
	PMIC_RG_RD_RDY_BYPASS_MASK = 0x1,
	PMIC_RG_RD_RDY_BYPASS_SHIFT = 4,
	PMIC_RG_SKIP_EFUSE_OUT_MASK = 0x1,
	PMIC_RG_SKIP_EFUSE_OUT_SHIFT = 2,
	PMIC_RG_EFUSE_PROG_SRC_MASK = 0x1,
	PMIC_RG_EFUSE_PROG_SRC_SHIFT = 0,
	PMIC_RG_EFUSE_BUSY_MASK = 0x1,
	PMIC_RG_EFUSE_BUSY_SHIFT = 2,
	PMIC_RG_EFUSE_RD_ACK_MASK = 0x1,
	PMIC_RG_EFUSE_RD_ACK_SHIFT = 0,
	PMIC_RG_EFUSE_VAL_0_15_MASK = 0xFFFF,
	PMIC_RG_EFUSE_VAL_0_15_SHIFT = 0,
	PMIC_RG_EFUSE_VAL_16_31_MASK = 0xFFFF,
	PMIC_RG_EFUSE_VAL_16_31_SHIFT = 0,
	PMIC_RG_EFUSE_VAL_32_47_MASK = 0xFFFF,
	PMIC_RG_EFUSE_VAL_32_47_SHIFT = 0,
	PMIC_RG_EFUSE_VAL_48_63_MASK = 0xFFFF,
	PMIC_RG_EFUSE_VAL_48_63_SHIFT = 0,
	PMIC_RG_EFUSE_VAL_64_79_MASK = 0xFFFF,
	PMIC_RG_EFUSE_VAL_64_79_SHIFT = 0,
	PMIC_RG_EFUSE_VAL_80_95_MASK = 0xFFFF,
	PMIC_RG_EFUSE_VAL_80_95_SHIFT = 0,
	PMIC_RG_EFUSE_VAL_96_111_MASK = 0xFFFF,
	PMIC_RG_EFUSE_VAL_96_111_SHIFT = 0,
	PMIC_RG_EFUSE_VAL_112_127_MASK = 0xFFFF,
	PMIC_RG_EFUSE_VAL_112_127_SHIFT = 0,
	PMIC_RG_EFUSE_VAL_128_143_MASK = 0xFFFF,
	PMIC_RG_EFUSE_VAL_128_143_SHIFT = 0,
	PMIC_RG_EFUSE_VAL_144_159_MASK = 0xFFFF,
	PMIC_RG_EFUSE_VAL_144_159_SHIFT = 0,
	PMIC_RG_EFUSE_VAL_160_175_MASK = 0xFFFF,
	PMIC_RG_EFUSE_VAL_160_175_SHIFT = 0,
	PMIC_RG_EFUSE_VAL_176_191_MASK = 0xFFFF,
	PMIC_RG_EFUSE_VAL_176_191_SHIFT = 0,
	PMIC_RG_EFUSE_VAL_192_207_MASK = 0xFFFF,
	PMIC_RG_EFUSE_VAL_192_207_SHIFT = 0,
	PMIC_RG_EFUSE_VAL_208_223_MASK = 0xFFFF,
	PMIC_RG_EFUSE_VAL_208_223_SHIFT = 0,
	PMIC_RG_EFUSE_VAL_224_239_MASK = 0xFFFF,
	PMIC_RG_EFUSE_VAL_224_239_SHIFT = 0,
	PMIC_RG_EFUSE_VAL_240_255_MASK = 0xFFFF,
	PMIC_RG_EFUSE_VAL_240_255_SHIFT = 0,
	PMIC_RG_EFUSE_VAL_256_271_MASK = 0xFFFF,
	PMIC_RG_EFUSE_VAL_256_271_SHIFT = 0,
	PMIC_RG_EFUSE_VAL_272_287_MASK = 0xFFFF,
	PMIC_RG_EFUSE_VAL_272_287_SHIFT = 0,
	PMIC_RG_EFUSE_VAL_288_303_MASK = 0xFFFF,
	PMIC_RG_EFUSE_VAL_288_303_SHIFT = 0,
	PMIC_RG_EFUSE_VAL_304_319_MASK = 0xFFFF,
	PMIC_RG_EFUSE_VAL_304_319_SHIFT = 0,
	PMIC_RG_EFUSE_DOUT_0_15_MASK = 0xFFFF,
	PMIC_RG_EFUSE_DOUT_0_15_SHIFT = 0,
	PMIC_RG_EFUSE_DOUT_16_31_MASK = 0xFFFF,
	PMIC_RG_EFUSE_DOUT_16_31_SHIFT = 0,
	PMIC_RG_EFUSE_DOUT_32_47_MASK = 0xFFFF,
	PMIC_RG_EFUSE_DOUT_32_47_SHIFT = 0,
	PMIC_RG_EFUSE_DOUT_48_63_MASK = 0xFFFF,
	PMIC_RG_EFUSE_DOUT_48_63_SHIFT = 0,
	PMIC_EFUSE_DOUT_64_79_MASK = 0xFFFF,
	PMIC_EFUSE_DOUT_64_79_SHIFT = 0,
	PMIC_EFUSE_DOUT_80_95_MASK = 0xFFFF,
	PMIC_EFUSE_DOUT_80_95_SHIFT = 0,
	PMIC_EFUSE_DOUT_96_111_MASK = 0xFFFF,
	PMIC_EFUSE_DOUT_96_111_SHIFT = 0,
	PMIC_EFUSE_DOUT_112_127_MASK = 0xFFFF,
	PMIC_EFUSE_DOUT_112_127_SHIFT = 0,
	PMIC_EFUSE_DOUT_128_143_MASK = 0xFFFF,
	PMIC_EFUSE_DOUT_128_143_SHIFT = 0,
	PMIC_EFUSE_DOUT_144_159_MASK = 0xFFFF,
	PMIC_EFUSE_DOUT_144_159_SHIFT = 0,
	PMIC_EFUSE_DOUT_160_175_MASK = 0xFFFF,
	PMIC_EFUSE_DOUT_160_175_SHIFT = 0,
	PMIC_EFUSE_DOUT_176_191_MASK = 0xFFFF,
	PMIC_EFUSE_DOUT_176_191_SHIFT = 0,
	PMIC_EFUSE_DOUT_192_207_MASK = 0xFFFF,
	PMIC_EFUSE_DOUT_192_207_SHIFT = 0,
	PMIC_EFUSE_DOUT_208_223_MASK = 0xFFFF,
	PMIC_EFUSE_DOUT_208_223_SHIFT = 0,
	PMIC_EFUSE_DOUT_224_239_MASK = 0xFFFF,
	PMIC_EFUSE_DOUT_224_239_SHIFT = 0,
	PMIC_EFUSE_DOUT_240_255_MASK = 0xFFFF,
	PMIC_EFUSE_DOUT_240_255_SHIFT = 0,
	PMIC_EFUSE_DOUT_256_271_MASK = 0xFFFF,
	PMIC_EFUSE_DOUT_256_271_SHIFT = 0,
	PMIC_EFUSE_DOUT_272_287_MASK = 0xFFFF,
	PMIC_EFUSE_DOUT_272_287_SHIFT = 0,
	PMIC_EFUSE_DOUT_288_303_MASK = 0xFFFF,
	PMIC_EFUSE_DOUT_288_303_SHIFT = 0,
	PMIC_EFUSE_DOUT_304_319_MASK = 0xFFFF,
	PMIC_EFUSE_DOUT_304_319_SHIFT = 0
};

/* BUCK MASK and SHIFT Definition */
enum{
	PMIC_VCA15_EN_MASK = 0x1,
	PMIC_VCA15_EN_SHIFT = 0,
	PMIC_VCA15_VOSEL_MASK = 0x7F,
	PMIC_VCA15_VOSEL_SHIFT = 0,
	PMIC_VCA15_VOSEL_ON_MASK = 0x7F,
	PMIC_VCA15_VOSEL_ON_SHIFT = 0,
	PMIC_VCA15_VOSEL_SLEEP_MASK = 0x7F,
	PMIC_VCA15_VOSEL_SLEEP_SHIFT = 0,
	PMIC_VSRMCA15_EN_MASK = 0x1,
	PMIC_VSRMCA15_EN_SHIFT = 0,
	PMIC_VSRMCA15_VOSEL_MASK = 0x7F,
	PMIC_VSRMCA15_VOSEL_SHIFT = 0,
	PMIC_VSRMCA15_VOSEL_ON_MASK = 0x7F,
	PMIC_VSRMCA15_VOSEL_ON_SHIFT = 0,
	PMIC_VSRMCA15_VOSEL_SLEEP_MASK = 0x7F,
	PMIC_VSRMCA15_VOSEL_SLEEP_SHIFT = 0,
	PMIC_VCORE_VOSEL_MASK = 0x7F,
	PMIC_VCORE_VOSEL_SHIFT = 0,
	PMIC_VCORE_VOSEL_ON_MASK = 0x7F,
	PMIC_VCORE_VOSEL_ON_SHIFT = 0,
	PMIC_VCORE_VOSEL_SLEEP_MASK = 0x7F,
	PMIC_VCORE_VOSEL_SLEEP_SHIFT = 0,
	PMIC_VPCA7_EN_MASK = 0x1,
	PMIC_VPCA7_EN_SHIFT = 0,
	PMIC_VPCA7_VOSEL_MASK = 0x7F,
	PMIC_VPCA7_VOSEL_SHIFT = 0,
	PMIC_VPCA7_VOSEL_ON_MASK = 0x7F,
	PMIC_VPCA7_VOSEL_ON_SHIFT = 0,
	PMIC_VPCA7_VOSEL_SLEEP_MASK = 0x7F,
	PMIC_VPCA7_VOSEL_SLEEP_SHIFT = 0,
	PMIC_VSRMCA7_VOSEL_MASK = 0x7F,
	PMIC_VSRMCA7_VOSEL_SHIFT = 0,
	PMIC_VSRMCA7_VOSEL_ON_MASK = 0x7F,
	PMIC_VSRMCA7_VOSEL_ON_SHIFT = 0,
	PMIC_VSRMCA7_VOSEL_SLEEP_MASK = 0x7F,
	PMIC_VSRMCA7_VOSEL_SLEEP_SHIFT = 0
};

/* DIGLDO MASK and SHIFT Definition */
enum{
	PMIC_RG_VCAMIO_SW_EN_MASK = 0x1,
	PMIC_RG_VCAMIO_SW_EN_SHIFT = 15,
	PMIC_RG_VCAMD_SW_EN_MASK = 0x1,
	PMIC_RG_VCAMD_SW_EN_SHIFT = 15,
	PMIC_RG_VGP4_SW_EN_MASK = 0x1,
	PMIC_RG_VGP4_SW_EN_SHIFT = 15,
	PMIC_RG_VCAMD_VOSEL_MASK = 0x7,
	PMIC_RG_VCAMD_VOSEL_SHIFT = 5,
	PMIC_RG_VGP4_VOSEL_MASK = 0x7,
	PMIC_RG_VGP4_VOSEL_SHIFT = 5,
	PMIC_VCAMD_ON_CTRL_MASK = 0x1,
	PMIC_VCAMD_ON_CTRL_SHIFT = 15,
	PMIC_VGP4_ON_CTRL_MASK = 0x1,
	PMIC_VGP4_ON_CTRL_SHIFT = 12
};

/* DCXO MASK and SHIFT Definition */
enum{
	PMIC_RG_DCXO_C2_UNTRIM_MASK = 0x1,
	PMIC_RG_DCXO_C2_UNTRIM_SHIFT = 15,
	PMIC_RG_DCXO_MANUAL_C1C2_SYNC_EN_MASK = 0x1,
	PMIC_RG_DCXO_MANUAL_C1C2_SYNC_EN_SHIFT = 9,
	PMIC_RG_DCXO_MANUAL_SYNC_EN_MASK = 0x1,
	PMIC_RG_DCXO_MANUAL_SYNC_EN_SHIFT = 8,
	PMIC_RG_DCXO_ATTEN_BB_MASK = 0x3,
	PMIC_RG_DCXO_ATTEN_BB_SHIFT = 11,
	PMIC_RG_DCXO_LDO_BB_V_MASK = 0x3,
	PMIC_RG_DCXO_LDO_BB_V_SHIFT = 2
};

/*
 * PMIC Status Code
 */
enum {
	PMIC_TEST_PASS = 0x0000,
	PMIC_TEST_FAIL = 0xB001,
	PMIC_EXCEED_I2C_FIFO_LENGTH	= 0xB002,
	PMIC_CHRDET_EXIST = 0xB003,
	PMIC_CHRDET_NOT_EXIST = 0xB004
};

/*
 * PMIC Exported Function
 */
int pmic_detect_powerkey(void);
void pl_hw_ulc_det(void);
u32 pmic_config_interface (u32 RegNum, u32 val, u32 MASK, u32 SHIFT);
u32 pmic_init(void);

#endif /* _PL_MT_PMIC_H_ */
