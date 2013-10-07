/*
 * Copyright (c) 2010 - 2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TEGRA124_PMC_H_
#define _TEGRA124_PMC_H_

/* Power Management Controller (APBDEV_PMC_) registers */
struct pmc_ctlr {
	u32 pmc_cntrl;			/* _CNTRL_0, offset 00 */
	u32 pmc_sec_disable;		/* _SEC_DISABLE_0, offset 04 */
	u32 pmc_pmc_swrst;		/* _PMC_SWRST_0, offset 08 */
	u32 pmc_wake_mask;		/* _WAKE_MASK_0, offset 0C */
	u32 pmc_wake_lvl;		/* _WAKE_LVL_0, offset 10 */
	u32 pmc_wake_status;		/* _WAKE_STATUS_0, offset 14 */
	u32 pmc_sw_wake_status;	/* _SW_WAKE_STATUS_0, offset 18 */
	u32 pmc_dpd_pads_oride;	/* _DPD_PADS_ORIDE_0, offset 1C */
	u32 pmc_dpd_sample;		/* _DPD_PADS_SAMPLE_0, offset 20 */
	u32 pmc_dpd_enable;		/* _DPD_PADS_ENABLE_0, offset 24 */
	u32 pmc_pwrgate_timer_off;	/* _PWRGATE_TIMER_OFF_0, offset 28 */
	u32 pmc_clamp_status;		/* _PMC_CLAMP_STATUS_0, offset 2C */
	u32 pmc_pwrgate_toggle;	/* _PWRGATE_TOGGLE_0, offset 30 */
	u32 pmc_remove_clamping;	/* _REMOVE_CLAMPING_CMD_0, offset 34 */
	u32 pmc_pwrgate_status;	/* _PWRGATE_STATUS_0, offset 38 */
	u32 pmc_pwrgood_timer;		/* _PWRGOOD_TIMER_0, offset 3C */
	u32 pmc_blink_timer;		/* _BLINK_TIMER_0, offset 40 */
	u32 pmc_no_iopower;		/* _NO_IOPOWER_0, offset 44 */
	u32 pmc_pwr_det;		/* _PWR_DET_0, offset 48 */
	u32 pmc_pwr_det_latch;		/* _PWR_DET_LATCH_0, offset 4C */

	u32 pmc_scratch0;		/* _SCRATCH0_0, offset 50 */
	u32 pmc_scratch1;		/* _SCRATCH1_0, offset 54 */
	u32 pmc_scratch2;		/* _SCRATCH2_0, offset 58 */
	u32 pmc_scratch3;		/* _SCRATCH3_0, offset 5C */
	u32 pmc_scratch4;		/* _SCRATCH4_0, offset 60 */
	u32 pmc_scratch5;		/* _SCRATCH5_0, offset 64 */
	u32 pmc_scratch6;		/* _SCRATCH6_0, offset 68 */
	u32 pmc_scratch7;		/* _SCRATCH7_0, offset 6C */
	u32 pmc_scratch8;		/* _SCRATCH8_0, offset 70 */
	u32 pmc_scratch9;		/* _SCRATCH9_0, offset 74 */
	u32 pmc_scratch10;		/* _SCRATCH10_0, offset 78 */
	u32 pmc_scratch11;		/* _SCRATCH11_0, offset 7C */
	u32 pmc_scratch12;		/* _SCRATCH12_0, offset 80 */
	u32 pmc_scratch13;		/* _SCRATCH13_0, offset 84 */
	u32 pmc_scratch14;		/* _SCRATCH14_0, offset 88 */
	u32 pmc_scratch15;		/* _SCRATCH15_0, offset 8C */
	u32 pmc_scratch16;		/* _SCRATCH16_0, offset 90 */
	u32 pmc_scratch17;		/* _SCRATCH17_0, offset 94 */
	u32 pmc_scratch18;		/* _SCRATCH18_0, offset 98 */
	u32 pmc_scratch19;		/* _SCRATCH19_0, offset 9C */
	u32 pmc_scratch20;		/* _SCRATCH20_0, offset A0 */
	u32 pmc_scratch21;		/* _SCRATCH21_0, offset A4 */
	u32 pmc_scratch22;		/* _SCRATCH22_0, offset A8 */
	u32 pmc_scratch23;		/* _SCRATCH23_0, offset AC */

	u32 pmc_secure_scratch0;	/* _SECURE_SCRATCH0_0, offset B0 */
	u32 pmc_secure_scratch1;	/* _SECURE_SCRATCH1_0, offset B4 */
	u32 pmc_secure_scratch2;	/* _SECURE_SCRATCH2_0, offset B8 */
	u32 pmc_secure_scratch3;	/* _SECURE_SCRATCH3_0, offset BC */
	u32 pmc_secure_scratch4;	/* _SECURE_SCRATCH4_0, offset C0 */
	u32 pmc_secure_scratch5;	/* _SECURE_SCRATCH5_0, offset C4 */

	u32 pmc_cpupwrgood_timer;	/* _CPUPWRGOOD_TIMER_0, offset C8 */
	u32 pmc_cpupwroff_timer;	/* _CPUPWROFF_TIMER_0, offset CC */
	u32 pmc_pg_mask;		/* _PG_MASK_0, offset D0 */
	u32 pmc_pg_mask_1;		/* _PG_MASK_1_0, offset D4 */
	u32 pmc_auto_wake_lvl;		/* _AUTO_WAKE_LVL_0, offset D8 */
	u32 pmc_auto_wake_lvl_mask;	/* _AUTO_WAKE_LVL_MASK_0, offset DC */
	u32 pmc_wake_delay;		/* _WAKE_DELAY_0, offset E0 */
	u32 pmc_pwr_det_val;		/* _PWR_DET_VAL_0, offset E4 */
	u32 pmc_ddr_pwr;		/* _DDR_PWR_0, offset E8 */
	u32 pmc_usb_debounce_del;	/* _USB_DEBOUNCE_DEL_0, offset EC */
	u32 pmc_usb_ao;		/* _USB_AO_0, offset F0 */
	u32 pmc_crypto_op;		/* _CRYPTO_OP__0, offset F4 */
	u32 pmc_pllp_wb0_override;	/* _PLLP_WB0_OVERRIDE_0, offset F8 */

	u32 pmc_scratch24;		/* _SCRATCH24_0, offset FC */
	u32 pmc_scratch25;		/* _SCRATCH24_0, offset 100 */
	u32 pmc_scratch26;		/* _SCRATCH24_0, offset 104 */
	u32 pmc_scratch27;		/* _SCRATCH24_0, offset 108 */
	u32 pmc_scratch28;		/* _SCRATCH24_0, offset 10C */
	u32 pmc_scratch29;		/* _SCRATCH24_0, offset 110 */
	u32 pmc_scratch30;		/* _SCRATCH24_0, offset 114 */
	u32 pmc_scratch31;		/* _SCRATCH24_0, offset 118 */
	u32 pmc_scratch32;		/* _SCRATCH24_0, offset 11C */
	u32 pmc_scratch33;		/* _SCRATCH24_0, offset 120 */
	u32 pmc_scratch34;		/* _SCRATCH24_0, offset 124 */
	u32 pmc_scratch35;		/* _SCRATCH24_0, offset 128 */
	u32 pmc_scratch36;		/* _SCRATCH24_0, offset 12C */
	u32 pmc_scratch37;		/* _SCRATCH24_0, offset 130 */
	u32 pmc_scratch38;		/* _SCRATCH24_0, offset 134 */
	u32 pmc_scratch39;		/* _SCRATCH24_0, offset 138 */
	u32 pmc_scratch40;		/* _SCRATCH24_0, offset 13C */
	u32 pmc_scratch41;		/* _SCRATCH24_0, offset 140 */
	u32 pmc_scratch42;		/* _SCRATCH24_0, offset 144 */

	u32 pmc_bo_mirror0;		/* _BOUNDOUT_MIRROR0_0, offset 148 */
	u32 pmc_bo_mirror1;		/* _BOUNDOUT_MIRROR1_0, offset 14C */
	u32 pmc_bo_mirror2;		/* _BOUNDOUT_MIRROR2_0, offset 150 */
	u32 pmc_sys_33v_en;		/* _SYS_33V_EN_0, offset 154 */
	u32 pmc_bo_mirror_access;	/* _BOUNDOUT_MIRROR_ACCESS_0, off158 */
	u32 pmc_gate;			/* _GATE_0, offset 15C */
	u32 pmc_wake2_mask;		/* _WAKE2_MASK_0, offset 160 */
	u32 pmc_wake2_lvl;		/* _WAKE2_LVL_0,  offset 164 */
	u32 pmc_wake2_stat;		/* _WAKE2_STATUS_0, offset 168 */
	u32 pmc_sw_wake2_stat;		/* _SW_WAKE2_STATUS_0, offset 16C */
	u32 pmc_auto_wake2_lvl_mask;	/* _AUTO_WAKE2_LVL_MASK_0, offset 170 */
	u32 pmc_pg_mask2;		/* _PG_MASK_2_0, offset 174 */
	u32 pmc_pg_mask_ce1;		/* _PG_MASK_CE1_0, offset 178 */
	u32 pmc_pg_mask_ce2;		/* _PG_MASK_CE2_0, offset 17C */
	u32 pmc_pg_mask_ce3;		/* _PG_MASK_CE3_0, offset 180 */
	u32 pmc_pwrgate_timer_ce0;	/* _PWRGATE_TIMER_CE_0_0, offset 184 */
	u32 pmc_pwrgate_timer_ce1;	/* _PWRGATE_TIMER_CE_1_0, offset 188 */
	u32 pmc_pwrgate_timer_ce2;	/* _PWRGATE_TIMER_CE_2_0, offset 18C */
	u32 pmc_pwrgate_timer_ce3;	/* _PWRGATE_TIMER_CE_3_0, offset 190 */
	u32 pmc_pwrgate_timer_ce4;	/* _PWRGATE_TIMER_CE_4_0, offset 194 */
	u32 pmc_pwrgate_timer_ce5;	/* _PWRGATE_TIMER_CE_5_0, offset 198 */
	u32 pmc_pwrgate_timer_ce6;	/* _PWRGATE_TIMER_CE_6_0, offset 19C */
	u32 pmc_pcx_edpd_cntrl;	/* _PCX_EDPD_CNTRL_0, offset 1A0 */
	u32 pmc_osc_edpd_over;		/* _OSC_EDPD_OVER_0, offset 1A4 */
	u32 pmc_clk_out_cntrl;		/* _CLK_OUT_CNTRL_0, offset 1A8 */
	u32 pmc_sata_pwrgate;		/* _SATA_PWRGT_0, offset 1AC */
	u32 pmc_sensor_ctrl;		/* _SENSOR_CTRL_0, offset 1B0 */
	u32 pmc_reset_status;		/* _RTS_STATUS_0, offset 1B4 */
	u32 pmc_io_dpd_req;		/* _IO_DPD_REQ_0, offset 1B8 */
	u32 pmc_io_dpd_stat;		/* _IO_DPD_STATUS_0, offset 1BC */
	u32 pmc_io_dpd2_req;		/* _IO_DPD2_REQ_0, offset 1C0 */
	u32 pmc_io_dpd2_stat;		/* _IO_DPD2_STATUS_0, offset 1C4 */
	u32 pmc_sel_dpd_tim;		/* _SEL_DPD_TIM_0, offset 1C8 */
	u32 pmc_vddp_sel;		/* _VDDP_SEL_0, offset 1CC */

	u32 pmc_ddr_cfg;		/* _DDR_CFG_0, offset 1D0 */
	u32 pmc_e_no_vttgen;		/* _E_NO_VTTGEN_0, offset 1D4 */
	u32 pmc_reserved0;		/* _RESERVED, offset 1D8 */
	u32 pmc_pllm_wb0_ovrride_frq;	/* _PLLM_WB0_OVERRIDE_FREQ_0, off 1DC */
	u32 pmc_test_pwrgate;		/* _TEST_PWRGATE_0, offset 1E0 */
	u32 pmc_pwrgate_timer_mult;	/* _PWRGATE_TIMER_MULT_0, offset 1E4 */
	u32 pmc_dsi_sel_dpd;		/* _DSI_SEL_DPD_0, offset 1E8 */
	u32 pmc_utmip_uhsic_triggers;	/* _UTMIP_UHSIC_TRIGGERS_0, off 1EC */
	u32 pmc_utmip_uhsic_saved_st;  /* _UTMIP_UHSIC_SAVED_STATE_0, off1F0 */
	u32 pmc_utmip_pad_cfg;		/* _UTMIP_PAD_CFG_0, offset 1F4 */
	u32 pmc_utmip_term_pad_cfg;	/* _UTMIP_TERM_PAD_CFG_0, offset 1F8 */
	u32 pmc_utmip_uhsic_sleep_cfg;	/* _UTMIP_UHSIC_SLEEP_CFG_0, off 1FC */

	u32 pmc_todo_0[9];		/* offset 200-220 */
	u32 pmc_secure_scratch6;	/* _SECURE_SCRATCH6_0, offset 224 */
	u32 pmc_secure_scratch7;	/* _SECURE_SCRATCH7_0, offset 228 */
	u32 pmc_scratch43;		/* _SCRATCH43_0, offset 22C */
	u32 pmc_scratch44;		/* _SCRATCH44_0, offset 230 */
	u32 pmc_scratch45;
	u32 pmc_scratch46;
	u32 pmc_scratch47;
	u32 pmc_scratch48;
	u32 pmc_scratch49;
	u32 pmc_scratch50;
	u32 pmc_scratch51;
	u32 pmc_scratch52;
	u32 pmc_scratch53;
	u32 pmc_scratch54;
	u32 pmc_scratch55;		/* _SCRATCH55_0, offset 25C */
	u32 pmc_scratch0_eco;		/* _SCRATCH0_ECO_0, offset 260 */
	u32 pmc_por_dpd_ctrl;		/* _POR_DPD_CTRL_0, offset 264 */
	u32 pmc_scratch2_eco;		/* _SCRATCH2_ECO_0, offset 268 */
	u32 pmc_todo_1[17];		/* TODO: 26C ~ 2AC */
	u32 pmc_pllm_wb0_override2;	/* _PLLM_WB0_OVERRIDE2, offset 2B0 */
	u32 pmc_tsc_mult;		/* _TSC_MULT_0, offset 2B4 */
	u32 pmc_cpu_vsense_override;	/* _CPU_VSENSE_OVERRIDE_0, offset 2B8 */
	u32 pmc_glb_amap_cfg;		/* _GLB_AMAP_CFG_0, offset 2BC */
	u32 pmc_sticky_bits;		/* _STICKY_BITS_0, offset 2C0 */
	u32 pmc_sec_disable2;		/* _SEC_DISALBE2, offset 2C4 */
	u32 pmc_weak_bias;		/* _WEAK_BIAS_0, offset 2C8 */
	u32 pmc_todo_3[13];		/* TODO: 2CC ~ 2FC */
	u32 pmc_secure_scratch8;	/* _SECURE_SCRATCH8_0, offset 300 */
	u32 pmc_secure_scratch9;
	u32 pmc_secure_scratch10;
	u32 pmc_secure_scratch11;
	u32 pmc_secure_scratch12;
	u32 pmc_secure_scratch13;
	u32 pmc_secure_scratch14;
	u32 pmc_secure_scratch15;
	u32 pmc_secure_scratch16;
	u32 pmc_secure_scratch17;
	u32 pmc_secure_scratch18;
	u32 pmc_secure_scratch19;
	u32 pmc_secure_scratch20;
	u32 pmc_secure_scratch21;
	u32 pmc_secure_scratch22;
	u32 pmc_secure_scratch23;
	u32 pmc_secure_scratch24;	/* _SECURE_SCRATCH24_0, offset 340 */
	u32 pmc_secure_scratch25;
	u32 pmc_secure_scratch26;
	u32 pmc_secure_scratch27;
	u32 pmc_secure_scratch28;
	u32 pmc_secure_scratch29;
	u32 pmc_secure_scratch30;
	u32 pmc_secure_scratch31;
	u32 pmc_secure_scratch32;
	u32 pmc_secure_scratch33;
	u32 pmc_secure_scratch34;
	u32 pmc_secure_scratch35;	/* _SECURE_SCRATCH35_0, offset 36C */

	u32 pmc_reserved1[52];		/* RESERVED: 370 ~ 43C */
	u32 pmc_cntrl2;		/* _CNTRL2_0, offset 440 */
	u32 pmc_reserved2[6];		/* RESERVED: 444 ~ 458 */
	u32 pmc_io_dpd3_req;		/* _IO_DPD3_REQ_0, offset 45c */
	u32 pmc_io_dpd3_stat;		/* _IO_DPD3_STATUS_0, offset 460 */
	u32 pmc_strap_opt_a;		/* _STRAPPING_OPT_A_0, offset 464 */
	u32 pmc_reserved3[102];	/* RESERVED: 468 ~ 5FC */

	u32 pmc_scratch56;		/* _SCRATCH56_0, offset 600 */
	u32 pmc_scratch57;
	u32 pmc_scratch58;
	u32 pmc_scratch59;
	u32 pmc_scratch60;
	u32 pmc_scratch61;
	u32 pmc_scratch62;
	u32 pmc_scratch63;
	u32 pmc_scratch64;
	u32 pmc_scratch65;
	u32 pmc_scratch66;
	u32 pmc_scratch67;
	u32 pmc_scratch68;
	u32 pmc_scratch69;
	u32 pmc_scratch70;
	u32 pmc_scratch71;
	u32 pmc_scratch72;
	u32 pmc_scratch73;
	u32 pmc_scratch74;
	u32 pmc_scratch75;
	u32 pmc_scratch76;
	u32 pmc_scratch77;
	u32 pmc_scratch78;
	u32 pmc_scratch79;
	u32 pmc_scratch80;
	u32 pmc_scratch81;
	u32 pmc_scratch82;
	u32 pmc_scratch83;
	u32 pmc_scratch84;
	u32 pmc_scratch85;
	u32 pmc_scratch86;
	u32 pmc_scratch87;
	u32 pmc_scratch88;
	u32 pmc_scratch89;
	u32 pmc_scratch90;
	u32 pmc_scratch91;
	u32 pmc_scratch92;
	u32 pmc_scratch93;
	u32 pmc_scratch94;
	u32 pmc_scratch95;
	u32 pmc_scratch96;
	u32 pmc_scratch97;
	u32 pmc_scratch98;
	u32 pmc_scratch99;
	u32 pmc_scratch100;
	u32 pmc_scratch101;
	u32 pmc_scratch102;
	u32 pmc_scratch103;
	u32 pmc_scratch104;
	u32 pmc_scratch105;
	u32 pmc_scratch106;
	u32 pmc_scratch107;
	u32 pmc_scratch108;
	u32 pmc_scratch109;
	u32 pmc_scratch110;
	u32 pmc_scratch111;
	u32 pmc_scratch112;
	u32 pmc_scratch113;
	u32 pmc_scratch114;
	u32 pmc_scratch115;
	u32 pmc_scratch116;
	u32 pmc_scratch117;
	u32 pmc_scratch118;
	u32 pmc_scratch119;
	u32 pmc_scratch1_eco;	/* offset 700 */
};

#define CPU_PWRED	1
#define CPU_CLMP	1

#define PARTID_CP	0xFFFFFFF8
#define START_CP	(1 << 8)

#define CPUPWRREQ_OE	(1 << 16)
#define CPUPWRREQ_POL	(1 << 15)

#define CRAILID		(0)
#define CE0ID		(14)
#define C0NCID		(15)
#define CRAIL		(1 << CRAILID)
#define CE0		(1 << CE0ID)
#define C0NC		(1 << C0NCID)

#define PMC_XOFS_SHIFT	1
#define PMC_XOFS_MASK	(0x3F << PMC_XOFS_SHIFT)

#define TIMER_MULT_SHIFT	0
#define TIMER_MULT_MASK		(3 << TIMER_MULT_SHIFT)
#define TIMER_MULT_CPU_SHIFT	2
#define TIMER_MULT_CPU_MASK	(3 << TIMER_MULT_CPU_SHIFT)
#define MULT_1			0
#define MULT_2			1
#define MULT_4			2
#define MULT_8			3

#define AMAP_WRITE_SHIFT	20
#define AMAP_WRITE_ON		(1 << AMAP_WRITE_SHIFT)

/* SEC_DISABLE_0, 0x04 */
#define SEC_DISABLE_WRITE0_ON			(1 << 4)
#define SEC_DISABLE_READ0_ON			(1 << 5)
#define SEC_DISABLE_WRITE1_ON			(1 << 6)
#define SEC_DISABLE_READ1_ON			(1 << 7)
#define SEC_DISABLE_WRITE2_ON			(1 << 8)
#define SEC_DISABLE_READ2_ON			(1 << 9)
#define SEC_DISABLE_WRITE3_ON			(1 << 10)
#define SEC_DISABLE_READ3_ON			(1 << 11)
#define SEC_DISABLE_AMAP_WRITE_ON		(1 << 20)

/* APBDEV_PMC_PWRGATE_TOGGLE_0 0x30 */
#define PWRGATE_TOGGLE_PARTID_CRAIL		0
#define PWRGATE_TOGGLE_PARTID_TD		1
#define PWRGATE_TOGGLE_PARTID_VE		2
#define PWRGATE_TOGGLE_PARTID_VDE		4
#define PWRGATE_TOGGLE_PARTID_L2C		5
#define PWRGATE_TOGGLE_PARTID_MPE		6
#define PWRGATE_TOGGLE_PARTID_HEG		7
#define PWRGATE_TOGGLE_PARTID_CE1		9
#define PWRGATE_TOGGLE_PARTID_CE2		10
#define PWRGATE_TOGGLE_PARTID_CE3		11
#define PWRGATE_TOGGLE_PARTID_CELP		12
#define PWRGATE_TOGGLE_PARTID_CE0		14
#define PWRGATE_TOGGLE_PARTID_C0NC		15
#define PWRGATE_TOGGLE_PARTID_C1NC		16
#define PWRGATE_TOGGLE_PARTID_DIS		18
#define PWRGATE_TOGGLE_PARTID_DISB		19
#define PWRGATE_TOGGLE_PARTID_XUSBA		20
#define PWRGATE_TOGGLE_PARTID_XUSBB		21
#define PWRGATE_TOGGLE_PARTID_XUSBC		22
#define PWRGATE_TOGGLE_START			(1 << 8)

/* APBDEV_PMC_PWRGATE_STATUS_0 0x38 */
#define PWRGATE_STATUS_CRAIL_ENABLE		(1 << 0)
#define PWRGATE_STATUS_TD_ENABLE		(1 << 1)
#define PWRGATE_STATUS_VE_ENABLE		(1 << 2)
#define PWRGATE_STATUS_VDE_ENABLE		(1 << 4)
#define PWRGATE_STATUS_L2C_ENABLE		(1 << 5)
#define PWRGATE_STATUS_MPE_ENABLE		(1 << 6)
#define PWRGATE_STATUS_HEG_ENABLE		(1 << 7)
#define PWRGATE_STATUS_CE1_ENABLE		(1 << 9)
#define PWRGATE_STATUS_CE2_ENABLE		(1 << 10)
#define PWRGATE_STATUS_CE3_ENABLE		(1 << 11)
#define PWRGATE_STATUS_CELP_ENABLE		(1 << 12)
#define PWRGATE_STATUS_CE0_ENABLE		(1 << 14)
#define PWRGATE_STATUS_C0NC_ENABLE		(1 << 15)
#define PWRGATE_STATUS_C1NC_ENABLE		(1 << 16)
#define PWRGATE_STATUS_DIS_ENABLE		(1 << 18)
#define PWRGATE_STATUS_DISB_ENABLE		(1 << 19)
#define PWRGATE_STATUS_XUSBA_ENABLE		(1 << 20)
#define PWRGATE_STATUS_XUSBB_ENABLE		(1 << 21)
#define PWRGATE_STATUS_XUSBC_ENABLE		(1 << 22)

/* APBDEV_PMC_CNTRL2_0 0x440 */
#define HOLD_CKE_LOW_EN				(1 << 12)

#endif	/* _TEGRA124_PMC_H_ */
