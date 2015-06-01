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

/*
 * Include files
 */
#include <delay.h>
#include <boardid.h>
#include <soc/dramc_common.h>
#include <soc/dramc_register.h>
#include <soc/dramc_pi_api.h>

/* dramc register define */
extern const dramc_ao_regs *ao_regs;
extern const dramc_nao_regs *nao_regs;
extern const dramc_ddrphy_regs *ddrphy_regs;

/*
 * Global variables
 */
static u8 opt_gw_coarse_value, opt_gw_fine_value;
u8 CurrentRank = 0;
u8 opt_gw_coarse_value_R0[2], opt_gw_fine_value_R0[2];
u8 opt_gw_coarse_value_R1[2], opt_gw_fine_value_R1[2];

static u8 fgwrlevel_done[2] = {0, 0};
u8 RXPERBIT_LOG_PRINT = 1;
u8 GATINGDQS_LOG_PRINT = 0;
static s8 wrlevel_dqs_final_delay[2][DQS_NUMBER];

static s8 CATrain_ClkDelay[2];
static u8 ucDLESetting;

/* ------------------------------------------------------------------------- */
/*  DramcPllPhaseCal
 *  start PLL Phase Calibration.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  apply           (u8): 0 don't apply the register we set 1 apply
 *			          the register we set ,default don't apply.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 * ------------------------------------------------------------------------- */

DRAM_STATUS_T DramcPllPhaseCal(DRAMC_CTX_T *p)
{
	/* MEMPLL's are different for A60808 and MT6595 */
	u8 ucstatus = 0;
	u16 one_count = 0, zero_count = 0;
	u8 pll1st_done = 0, pll2nd_done = 0, pll3rd_done = 0;
	u8 pll1st_dl = 0, pll2nd_dl = 0, pll3rd_dl = 0;
	u8 pll1st_phase = 0, pll2nd_phase = 0, pll3rd_phase = 0;
	s8 ret = 0;
	u32 u4value, addr;

	/* for PoP, use MEMPLL 2,4 and MEMPLL05 2, 3 */
	/* for SBS, use MEMPLL 2,3,4 (no SBS for Rome) */

	mcSHOW_DBG_MSG("[PLL_Phase_Calib] ===== ");
	mcSHOW_DBG_MSG("PLL Phase Calibration:CHANNEL ");
	mcSHOW_DBG_MSG("%d (0: CHA, 1: CHB) =====\n", p->channel);

	/* 1. Set jitter meter clock to internal FB path */
	/* MEMPLL*_FB_MCK_SEL = 0; */
	/* Not necessary for A60808 */
	/* It has no phase difference when internal or external loop */

	/* 2. Set jitter meter count number */
	/* JMTRCNT = 0x400; JMTRCNT:0x74[30:16]. */
	/* Set jitter meter count number to 1024 */
	dramc_setfield(p, (uintptr_t)&ddrphy_regs->jmeter2, fcJMETER_COUNT, 0xffff0000, 16);

	/* MEMPLL 3 jitter metter count */
	dramc_setfield(p, (uintptr_t)&ddrphy_regs->jmeter3, fcJMETER_COUNT, 0xffff0000, 16);

	/* MEMPLL 4 jitter metter count */
	dramc_setfield(p, (uintptr_t)&ddrphy_regs->jmeter4, fcJMETER_COUNT, 0xffff0000, 16);

	while (1) {
		if (!pll1st_done) {  /* MEMPLL 2 */
			if (pll1st_phase == 0) {
				/* initial phase set to 0 for REF and FBK */
				/* RG_MEPLL2_REF_DL:0x618[12:8]. */
				/* MEMPLL2 REF_DL to 0 */
				dramc_setfield(p, (uintptr_t)&ddrphy_regs->mempll[6], 0x0, 0x00001f00, 8);

				/* RG_MEPLL2_FBK_DL:0x618[4:0]. */
				/* MEMPLL2 FBK_DL to 0 */
				dramc_setfield(p, (uintptr_t)&ddrphy_regs->mempll[6], 0x0, 0x0000001f, 0);

			} else if (pll1st_phase == 1) {
				/* REF lag FBK, delay FBK */
				/* RG_MEPLL2_REF_DL:0x618[12:8]. */
				/* MEMPLL2 REF_DL to 0 */
				dramc_setfield(p, (uintptr_t)&ddrphy_regs->mempll[6], 0x0, 0x00001f00, 8);

				/* RG_MEPLL2_FBK_DL:0x618[4:0]. */
				/* MEMPLL2 FBK_DL to increase */
				dramc_setfield(p, (uintptr_t)&ddrphy_regs->mempll[6], pll1st_dl,
						 0x0000001f, 0);

			} else { /* REF lead FBK, delay REF */
				/* RG_MEPLL2_REF_DL:0x618[12:8]. */
				/* MEMPLL2 REF_DL to increase */
				dramc_setfield(p, (uintptr_t)&ddrphy_regs->mempll[6], pll1st_dl,
						 0x00001f00, 8);

				/* RG_MEPLL2_FBK_DL:0x618[4:0]. */
				/* MEMPLL2 FBK_DL to 0 */
				dramc_setfield(p, (uintptr_t)&ddrphy_regs->mempll[6], 0x0, 0x0000001f, 0);

			}
		}
		if (!pll2nd_done) {  /* MEMPLL 3 */
			if (pll2nd_phase == 0) {
				/* initial phase set to 0 for REF and FBK */
				/* RG_MEPLL3_REF_DL:0x624[12:8]. */
				/* MEMPLL3 REF_DL to 0 */
				dramc_setfield(p, (uintptr_t)&ddrphy_regs->mempll[9], 0x0, 0x00001f00, 8);

				/* RG_MEPLL3_FBK_DL:0x624[4:0]. */
				/* MEMPLL3 FBK_DL to 0 */
				dramc_setfield(p, (uintptr_t)&ddrphy_regs->mempll[9], 0x0, 0x0000001f, 0);

			} else if (pll2nd_phase == 1) {
				/* REF lag FBK, delay FBK */
				/* RG_MEPLL3_REF_DL:0x624[12:8]. */
				/* MEMPLL3 REF_DL to 0 */
				dramc_setfield(p, (uintptr_t)&ddrphy_regs->mempll[9], 0x0, 0x00001f00, 8);

				/* RG_MEPLL3_FBK_DL:0x624[4:0]. */
				/* MEMPLL3 FBK_DL to increase */
				dramc_setfield(p, (uintptr_t)&ddrphy_regs->mempll[9], pll2nd_dl,
						 0x0000001f, 0);

			} else { /* REF lead FBK, delay REF */
				/* RG_MEPLL3_REF_DL:0x624[12:8]. */
				/* MEMPLL3 REF_DL to increase */
				dramc_setfield(p, (uintptr_t)&ddrphy_regs->mempll[9], pll2nd_dl,
						 0x00001f00, 8);
				/* RG_MEPLL4_FBK_DL:0x624[4:0]. */
				/* MEMPLL3 FBK_DL to 0 */
				dramc_setfield(p, (uintptr_t)&ddrphy_regs->mempll[9], 0x0, 0x0000001f, 0);

			}
		}
		if (!pll3rd_done) {  /* MEMPLL 4 */
			if (pll3rd_phase == 0) {
				/* initial phase set to 0 for REF and FBK */
				/* RG_MEPLL4_REF_DL:0x630[12:8]. */
				/* MEMPLL4 REF_DL to 0 */
				dramc_setfield(p, (uintptr_t)&ddrphy_regs->mempll[12], 0x0, 0x00001f00, 8);

				/* RG_MEPLL4_FBK_DL:0x630[4:0]. */
				/* MEMPLL4 FBK_DL to 0 */
				dramc_setfield(p, (uintptr_t)&ddrphy_regs->mempll[12], 0x0, 0x0000001f, 0);

			} else if (pll3rd_phase == 1) {
				/* REF lag FBK, delay FBK */
				/* RG_MEPLL4_REF_DL:0x630[12:8]. */
				/* MEMPLL4 REF_DL to 0 */
				dramc_setfield(p, (uintptr_t)&ddrphy_regs->mempll[12], 0x0, 0x00001f00, 8);

				/* RG_MEPLL4_FBK_DL:0x630[4:0]. */
				/* MEMPLL4 FBK_DL to increase */
				dramc_setfield(p, (uintptr_t)&ddrphy_regs->mempll[12], pll3rd_dl,
						 0x0000001f, 0);

			} else { /* REF lead FBK, delay REF */
				/* RG_MEPLL4_REF_DL:0x630[12:8]. */
				/* MEMPLL4 REF_DL to increase */
				dramc_setfield(p, (uintptr_t)&ddrphy_regs->mempll[12], pll3rd_dl,
						 0x00001f00, 8);

				/* RG_MEPLL4_FBK_DL:0x630[4:0]. */
				/* MEMPLL4 FBK_DL to 0 */
				dramc_setfield(p, (uintptr_t)&ddrphy_regs->mempll[12], 0x0, 0x0000001f, 0);

			}
		}

		/* delay 20us for external loop PLL stable */
		udelay(20);

		/* 4. Enable jitter meter */
		/* MEMPLL 2 jitter meter enable */
		dramc_setbit(p, (uintptr_t)&ddrphy_regs->jmeter2, 0);

		/* MEMPLL 3 jitter metter enable */
		dramc_setbit(p, (uintptr_t)&ddrphy_regs->jmeter3, 0);

		/* MEMPLL 4 jitter metter enable */
		dramc_setbit(p, (uintptr_t)&ddrphy_regs->jmeter4, 0);

		/* 5. Wait for jitter meter complete */
		/* (depend on clock 20MHz, 1024/20M=51.2us) */
		udelay(fcJMETER_WAIT_DONE_US);

		/* 6. Check jitter meter counter value */
		if (!pll1st_done) {  /* MEMPLL 2 */
			addr = mcSET_DRAMC_REG_ADDR(p, 0x320);
			ucstatus |= ucDram_Register_Read(addr, &u4value);
			one_count = (u16)mcGET_FIELD(u4value, 0xffff0000, 16);
			zero_count = (u16)mcGET_FIELD(u4value, 0x0000ffff, 0);

			if (pll1st_phase == 0) {

				mcSHOW_DBG_MSG("[PLL_Phase_Calib] ");
				mcSHOW_DBG_MSG("PLL2 initial phase: ");

				if (one_count > (zero_count+fcJMETER_COUNT/10)) {
					/* REF lag FBK */
					pll1st_phase = 1;
					pll1st_dl++;

					mcSHOW_DBG_MSG("REF lag FBK, ");
					mcSHOW_DBG_MSG("one_cnt/zero_cnt = %d/%d\n",
						       one_count, zero_count);

				} else if (zero_count > (one_count+fcJMETER_COUNT/10)) {
					/* REF lead FBK */
					pll1st_phase = 2;
					pll1st_dl++;

					mcSHOW_DBG_MSG("REF lead FBK, ");
					mcSHOW_DBG_MSG("one_cnt/zero_cnt = %d/%d\n",
						       one_count, zero_count);

				} else {
					/* in phase at initial */
					pll1st_done = 1;

					mcSHOW_DBG_MSG("REF in-phase FBK, ");
					mcSHOW_DBG_MSG("one_cnt/zero_cnt = %d/%d\n",
						       one_count, zero_count);
				}
			} else if (pll1st_phase == 1) {
				if ((zero_count+fcJMETER_COUNT/10) >= one_count) {
					pll1st_done = 1;

					mcSHOW_DBG_MSG("[PLL_Phase_Calib] ");
					mcSHOW_DBG_MSG("PLL2 REF_DL: 0x0, ");
					mcSHOW_DBG_MSG("FBK_DL: %#x, ", pll1st_dl);
					mcSHOW_DBG_MSG("one_cnt/zero_cnt = %d/%d\n",
						       one_count, zero_count);

				} else {
					pll1st_dl++;
				}
			} else {
				if ((one_count+fcJMETER_COUNT/10) >= zero_count) {
					pll1st_done = 1;

					mcSHOW_DBG_MSG("[PLL_Phase_Calib] ");
					mcSHOW_DBG_MSG("PLL2 REF_DL: ");
					mcSHOW_DBG_MSG("%#x, ", pll1st_dl);
					mcSHOW_DBG_MSG("FBK_DL: 0x0, ");
					mcSHOW_DBG_MSG("one_cnt/zero_cnt = %d/%d\n",
						       one_count, zero_count);

				} else {
					pll1st_dl++;
				}
			}
		}

		if (!pll2nd_done) {  /* MEMPLL 3 */
			addr = mcSET_DRAMC_REG_ADDR(p, 0x324);
			ucstatus |= ucDram_Register_Read(addr, &u4value);
			one_count = (u16)mcGET_FIELD(u4value, 0xffff0000, 16);
			zero_count = (u16)mcGET_FIELD(u4value, 0x0000ffff, 0);

			if (pll2nd_phase == 0) {

				mcSHOW_DBG_MSG("[PLL_Phase_Calib] ");
				mcSHOW_DBG_MSG("PLL3 initial phase: ");

				if (one_count > (zero_count+fcJMETER_COUNT/10)) {
					/* REF lag FBK */
					pll2nd_phase = 1;
					pll2nd_dl++;

					mcSHOW_DBG_MSG("REF lag FBK, ");
					mcSHOW_DBG_MSG("one_cnt/zero_cnt = %d/%d\n",
						       one_count, zero_count);

				} else if (zero_count > (one_count+fcJMETER_COUNT/10)) {
					/* REF lead FBK */
					pll2nd_phase = 2;
					pll2nd_dl++;

					mcSHOW_DBG_MSG("REF lead FBK, ");
					mcSHOW_DBG_MSG("one_cnt/zero_cnt = %d/%d\n",
						       one_count, zero_count);

				} else {
					/* in phase at initial */
					pll2nd_done = 1;

					mcSHOW_DBG_MSG("REF in-phase FBK, ");
					mcSHOW_DBG_MSG("one_cnt/zero_cnt = %d/%d\n",
						       one_count, zero_count);

				}
			} else if (pll2nd_phase == 1) {
				if ((zero_count+fcJMETER_COUNT/10) >= one_count) {
					pll2nd_done = 1;

					mcSHOW_DBG_MSG("[PLL_Phase_Calib] ");
					mcSHOW_DBG_MSG("PLL3 REF_DL: 0x0, ");
					mcSHOW_DBG_MSG("FBK_DL: ");
					mcSHOW_DBG_MSG("%#x, ", pll2nd_dl);
					mcSHOW_DBG_MSG("one_cnt/zero_cnt = %d/%d\n",
						       one_count, zero_count);

				} else {
					pll2nd_dl++;
				}
			} else {
				if ((one_count+fcJMETER_COUNT/10) >= zero_count) {
					pll2nd_done = 1;

					mcSHOW_DBG_MSG("[PLL_Phase_Calib] ");
					mcSHOW_DBG_MSG("PLL3 REF_DL: ");
					mcSHOW_DBG_MSG("%#x, ", pll2nd_dl);
					mcSHOW_DBG_MSG("FBK_DL: 0x0, ");
					mcSHOW_DBG_MSG("one_cnt/zero_cnt = %d/%d\n",
						       one_count, zero_count);

				} else {
					pll2nd_dl++;
				}
			}
		}

		if (!pll3rd_done) {  /* MEMPLL 4 */
			addr = mcSET_DRAMC_REG_ADDR(p, 0x328);
			ucstatus |= ucDram_Register_Read(addr, &u4value);
			one_count = (u16)mcGET_FIELD(u4value, 0xffff0000, 16);
			zero_count = (u16)mcGET_FIELD(u4value, 0x0000ffff, 0);

			if (pll3rd_phase == 0) {

				mcSHOW_DBG_MSG("[PLL_Phase_Calib] ");
				mcSHOW_DBG_MSG("PLL4 initial phase: ");

				if (one_count > (zero_count+fcJMETER_COUNT/10)) {
					/* REF lag FBK */
					pll3rd_phase = 1;
					pll3rd_dl++;

					mcSHOW_DBG_MSG("REF lag FBK, ");
					mcSHOW_DBG_MSG("one_cnt/zero_cnt = %d/%d\n",
						       one_count, zero_count);

				} else if (zero_count > (one_count+fcJMETER_COUNT/10)) {
					/* REF lead FBK */
					pll3rd_phase = 2;
					pll3rd_dl++;

					mcSHOW_DBG_MSG("REF lead FBK, ");
					mcSHOW_DBG_MSG("one_cnt/zero_cnt = %d/%d\n",
						       one_count, zero_count);

				} else {
					/* in phase at initial */
					pll3rd_done = 1;

					mcSHOW_DBG_MSG("REF in-phase FBK, ");
					mcSHOW_DBG_MSG("one_cnt/zero_cnt = %d/%d\n",
						       one_count, zero_count);

				}
			} else if (pll3rd_phase == 1) {
				if ((zero_count+fcJMETER_COUNT/10) >= one_count) {
					pll3rd_done = 1;

					mcSHOW_DBG_MSG("[PLL_Phase_Calib] ");
					mcSHOW_DBG_MSG("PLL4 REF_DL: 0x0, ");
					mcSHOW_DBG_MSG("FBK_DL: ");
					mcSHOW_DBG_MSG("%#x, ", pll3rd_dl);
					mcSHOW_DBG_MSG("one_cnt/zero_cnt = %d/%d\n",
						       one_count, zero_count);

				} else {
					pll3rd_dl++;
				}
			} else {
				if ((one_count+fcJMETER_COUNT/10) >= zero_count) {
					pll3rd_done = 1;

					mcSHOW_DBG_MSG("[PLL_Phase_Calib] ");
					mcSHOW_DBG_MSG("PLL4 REF_DL: ");
					mcSHOW_DBG_MSG("%#x, ", pll3rd_dl);
					mcSHOW_DBG_MSG("FBK_DL: 0x0, ");
					mcSHOW_DBG_MSG("one_cnt/zero_cnt = %d/%d\n",
						       one_count, zero_count);

				} else {
					pll3rd_dl++;
				}
			}
		}

		/* 7. Reset jitter meter value */
		/* MEMPLL 2 jitter meter disable */
		dramc_clrbit(p, (uintptr_t)&ddrphy_regs->jmeter2, 0);

		/* MEMPLL 3 jitter metter disable */
		dramc_clrbit(p, (uintptr_t)&ddrphy_regs->jmeter3, 0);

		/* MEMPLL 4 jitter metter disable */
		dramc_clrbit(p, (uintptr_t)&ddrphy_regs->jmeter4, 0);

		/* 8. All done?! early break */
		if (pll1st_done && pll2nd_done && pll3rd_done) {
			ret = 0;
			break;
		}

		/* 9. delay line overflow?! break */
		if ((pll1st_dl >= 32) || (pll2nd_dl >= 32)
			|| (pll3rd_dl >= 32)) {
				ret = -1;
				break;
		}
	}

	mcSHOW_DBG_MSG("pll done: ");
	mcSHOW_DBG_MSG("%d, %d, %d\n", pll1st_done, pll2nd_done, pll3rd_done);
	mcSHOW_DBG_MSG("pll dl: %d, %d, %d\n", pll1st_dl, pll2nd_dl, pll3rd_dl);

	if (ret != 0) {
		mcSHOW_ERR_MSG("MEMPLL calibration fail\n");
	}

	if (ucstatus != 0) {
		mcSHOW_ERR_MSG("register access fail!\n");
	}

	if ((ucstatus != 0) || (ret != 0)) {
		return DRAM_FAIL;
	} else {
		return DRAM_OK;
	}

}

/* ------------------------------------------------------------------------- */
/*  DramCPllGroupsCal
 *  PLL Groups Skew Calibration.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 * ------------------------------------------------------------------------- */

DRAM_STATUS_T DramCPllGroupsCal(DRAMC_CTX_T *p)
{
	u16 one_count = 0, zero_count = 0;
	u8 pll1st_done = 0;
	u8 pll1st_dl = 0;
	u8 pll1st_phase = 0;
	s8 ret = 0;
	u32 u4value;
	u8 ucstatus = 0;
	uintptr_t addr;

	addr = mcSET_DRAMC_REG_ADDR(p, 0x638);
	ucstatus |= ucDram_Register_Read(addr, &u4value);

	if ((u4value & (1<<29))) {
		/* Only CTS mode needs to do the PLL group calibration. */
		/* Seal ring mode do this may cause larger skew. */
		/* So directly return; */
		return DRAM_OK;
	}

	mcSHOW_DBG_MSG("[PLL_Group_Calib] ===== ");
	mcSHOW_DBG_MSG("PLL Group Phase calibration:(ChB vs. ChA) =====\n");

	/* 1. Set jitter meter clock to internal FB path */
	/* MEMPLL*_FB_MCK_SEL = 0; */
	/* Not necessary for A60808 */
	/* It has no phase difference when internal or external loop */

	/* 2. First K CHA & CHB */
	addr = CHA_DRAMCNAO_BASE + 0x1d8;
	*(volatile unsigned int *)(addr) &= 0x0000ffff;
	addr = CHA_DRAMCNAO_BASE + 0x1d8;
	*(volatile unsigned int *)(addr) |= (fcJMETER_COUNT<<16);

	while (1) {
		if (pll1st_phase == 0) {
			/* Fix channel B delay to 0x0. */
			addr = CHB_DDRPHY_BASE + 0x61c;
			*(volatile unsigned int *)(addr) &= 0xc0ffffff;
			addr = CHB_DDRPHY_BASE + 0x61c;
			*(volatile unsigned int *)(addr) |= (0x0 << 24);
			addr = CHB_DDRPHY_BASE + 0x628;
			*(volatile unsigned int *)(addr) &= 0xc0ffffff;
			addr = CHB_DDRPHY_BASE + 0x628;
			*(volatile unsigned int *)(addr) |= (0x0 << 24);
			addr = CHB_DDRPHY_BASE + 0x634;
			*(volatile unsigned int *)(addr) &= 0xc0ffffff;
			addr = CHB_DDRPHY_BASE + 0x634;
			*(volatile unsigned int *)(addr) |= (0x0 << 24);

			/* Fix channel A delay to 0x0 */
			addr = CHA_DDRPHY_BASE + 0x61c;
			*(volatile unsigned int *)(addr) &= 0xc0ffffff;
			addr = CHA_DDRPHY_BASE + 0x61c;
			*(volatile unsigned int *)(addr) |= (0x0 << 24);
			addr = CHA_DDRPHY_BASE + 0x628;
			*(volatile unsigned int *)(addr) &= 0xc0ffffff;
			addr = CHA_DDRPHY_BASE + 0x628;
			*(volatile unsigned int *)(addr) |= (0x0 << 24);
			addr = CHA_DDRPHY_BASE + 0x634;
			*(volatile unsigned int *)(addr) &= 0xc0ffffff;
			addr = CHA_DDRPHY_BASE + 0x634;
			*(volatile unsigned int *)(addr) |= (0x0 << 24);
		} else if (pll1st_phase == 1) { /* chB lag chA, delay chA */
			/* Fix channel B delay to 0x0. */
			addr = CHB_DDRPHY_BASE + 0x61c;
			*(volatile unsigned int *)(addr) &= 0xc0ffffff;
			addr = CHB_DDRPHY_BASE + 0x61c;
			*(volatile unsigned int *)(addr) |= (0x0 << 24);
			addr = CHB_DDRPHY_BASE + 0x628;
			*(volatile unsigned int *)(addr) &= 0xc0ffffff;
			addr = CHB_DDRPHY_BASE + 0x628;
			*(volatile unsigned int *)(addr) |= (0x0 << 24);
			addr = CHB_DDRPHY_BASE + 0x634;
			*(volatile unsigned int *)(addr) &= 0xc0ffffff;
			addr = CHB_DDRPHY_BASE + 0x634;
			*(volatile unsigned int *)(addr) |= (0x0 << 24);

			/* channel A delay to increase */
			addr = CHA_DDRPHY_BASE + 0x61c;
			*(volatile unsigned int *)(addr) &= 0xc0ffffff;
			addr = CHA_DDRPHY_BASE + 0x61c;
			*(volatile unsigned int *)(addr) |= (pll1st_dl << 24);
			addr = CHA_DDRPHY_BASE + 0x628;
			*(volatile unsigned int *)(addr) &= 0xc0ffffff;
			addr = CHA_DDRPHY_BASE + 0x628;
			*(volatile unsigned int *)(addr) |= (pll1st_dl << 24);
			addr = CHA_DDRPHY_BASE + 0x634;
			*(volatile unsigned int *)(addr) &= 0xc0ffffff;
			addr = CHA_DDRPHY_BASE + 0x634;
			*(volatile unsigned int *)(addr) |= (pll1st_dl << 24);
		} else { /* chB lead chA, delay chB */
			/* channel B delay to increase. */
			addr = CHB_DDRPHY_BASE + 0x61c;
			*(volatile unsigned int *)(addr) &= 0xc0ffffff;
			addr = CHB_DDRPHY_BASE + 0x61c;
			*(volatile unsigned int *)(addr) |= (pll1st_dl << 24);
			addr = CHB_DDRPHY_BASE + 0x628;
			*(volatile unsigned int *)(addr) &= 0xc0ffffff;
			addr = CHB_DDRPHY_BASE + 0x628;
			*(volatile unsigned int *)(addr) |= (pll1st_dl << 24);
			addr = CHB_DDRPHY_BASE + 0x634;
			*(volatile unsigned int *)(addr) &= 0xc0ffffff;
			addr = CHB_DDRPHY_BASE + 0x634;
			*(volatile unsigned int *)(addr) |= (pll1st_dl << 24);

			/* Fix channel A delay to 0x0 */
			addr = CHA_DDRPHY_BASE + 0x61c;
			*(volatile unsigned int *)(addr) &= 0xc0ffffff;
			addr = CHA_DDRPHY_BASE + 0x61c;
			*(volatile unsigned int *)(addr) |= (0x0 << 24);
			addr = CHA_DDRPHY_BASE + 0x628;
			*(volatile unsigned int *)(addr) &= 0xc0ffffff;
			addr = CHA_DDRPHY_BASE + 0x628;
			*(volatile unsigned int *)(addr) |= (0x0 << 24);
			addr = CHA_DDRPHY_BASE + 0x634;
			*(volatile unsigned int *)(addr) &= 0xc0ffffff;
			addr = CHA_DDRPHY_BASE + 0x634;
			*(volatile unsigned int *)(addr) |= (0x0 << 24);
		}

		/* delay 20us for PLL stable */
		udelay(20);

		/* 4. Enable jitter meter */
		addr = CHA_DRAMCNAO_BASE + 0x1d8;
		*(volatile unsigned int *)(addr) &= 0xfffffffe;
		addr = CHA_DRAMCNAO_BASE + 0x1d8;
		*(volatile unsigned int *)(addr) |= 0x00000001;

		/* 5. Wait for jitter meter complete */
		/* (depend on clock 20MHz, 1024/20M=51.2us) */
		/* udelay(fcJMETER_WAIT_DONE_US); */
		addr = CHA_DRAMCNAO_BASE + 0x2bc;
		while (((*(volatile unsigned int *)(addr)) & (0x01<<31)) == 0);

		addr = CHA_DRAMCNAO_BASE + 0x2bc;
		u4value = (*(volatile unsigned int *)(addr));
		one_count = (u16)mcGET_FIELD(u4value, 0x7fff0000, 16);
		zero_count = (u16)mcGET_FIELD(u4value, 0x00007fff, 0);

		if (pll1st_phase == 0) {
			if (one_count > (zero_count+fcJMETER_COUNT/10)) {
				/* Ch-B lag Ch-A */
				pll1st_phase = 1;
				pll1st_dl++;

				mcSHOW_DBG_MSG("[PLL_Group_Calib] ");
				mcSHOW_DBG_MSG("Initial phase: Ch-B lag Ch-A, ");
				mcSHOW_DBG_MSG("one_cnt/zero_cnt = ");
				mcSHOW_DBG_MSG("%d/%d\n", one_count, zero_count);

			} else if (zero_count > (one_count+fcJMETER_COUNT/10)) {
				/* Ch-B lead Ch-A */
				pll1st_phase = 2;
				pll1st_dl++;

				mcSHOW_DBG_MSG("[PLL_Group_Calib] ");
				mcSHOW_DBG_MSG("Initial phase: Ch-B lead Ch-A, ");
				mcSHOW_DBG_MSG("one_cnt/zero_cnt = ");
				mcSHOW_DBG_MSG("%d/%d\n", one_count, zero_count);

			} else {
				/* in phase at initial */
				pll1st_done = 1;

				mcSHOW_DBG_MSG("[PLL_Group_Calib] ");
				mcSHOW_DBG_MSG("Initial phase: Ch-B in-phase Ch-A, ");
				mcSHOW_DBG_MSG("one_cnt/zero_cnt = ");
				mcSHOW_DBG_MSG("%d/%d\n", one_count, zero_count);

			}
		} else if (pll1st_phase == 1) {
			if ((zero_count+fcJMETER_COUNT/10) >= one_count) {
				pll1st_done = 1;

				mcSHOW_DBG_MSG("[PLL_Group_Calib] ");
				mcSHOW_DBG_MSG("CHB_DL: 0x0, ");
				mcSHOW_DBG_MSG("CHA_DL: %#x, ", pll1st_dl);
				mcSHOW_DBG_MSG("one_cnt/zero_cnt = ");
				mcSHOW_DBG_MSG("%d/%d\n", one_count, zero_count);
			} else {
				pll1st_dl++;
			}
		} else {
			if ((one_count+fcJMETER_COUNT/10) >= zero_count) {
				pll1st_done = 1;

				mcSHOW_DBG_MSG("[PLL_Group_Calib] ");
				mcSHOW_DBG_MSG("CHB_DL: %#x, ", pll1st_dl);
				mcSHOW_DBG_MSG("CHA_DL: 0x0, ");
				mcSHOW_DBG_MSG("one_cnt/zero_cnt = ");
				mcSHOW_DBG_MSG("%d/%d\n", one_count, zero_count);

			} else {
				pll1st_dl++;
			}
		}

		/* 7. Reset jitter meter value */
		/* MEMPLL 2 jitter metter disable */
		addr = CHA_DRAMCNAO_BASE + 0x1d8;
		*(volatile unsigned int *)(addr) &= 0xfffffffe;

		/* 8. All done?! early break */
		if (pll1st_done) {
			ret = 0;
			break;
		}

		/* 9. delay line overflow?! break */
		if (pll1st_dl >= 64) {
			ret = -1;
			break;
		}
	}

	if (ret != 0) {
		mcSHOW_ERR_MSG("[PLL_Group_Calib] ");
		mcSHOW_ERR_MSG("MEMPLL group phase calibration fail ");
		mcSHOW_ERR_MSG("(channel B vs. channel A)\n");
		return DRAM_FAIL;
	}

	return DRAM_OK;
}

/* ------------------------------------------------------------------------- */
/*  DramcSwImpedanceCal
 *  start TX OCD impedance calibration.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  apply           (u8): 0 don't apply the register we set
 *				1 apply the register we set, default don't apply.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 * ------------------------------------------------------------------------- */

DRAM_STATUS_T DramcSwImpedanceCal(DRAMC_CTX_T *p, u8 apply)
{
	/* drv*_status */
	/* 0: idle */
	/* 1: to be done */
	/* 2: Calibration OK */
	/* -1: Calibration FAIL */
	/* -2: Calibration NOT FOUND */
	s8 drvp_status, drvn_status;
	u8 drvp = 0xf, drvn = 0xf;
	u8 ucstatus = 0;
	u32 u4value, addr;

	mcSHOW_DBG_MSG("[Imp Calibration] Start SW impedance calibration...\n");
	mcFPRINTF((fp_A60808, "[Imp Calibration] Start SW "
				"impedance calibration...\n"));

	/* calibration value */
	drvp = 9;
	drvn = 10;

	mcSHOW_DBG_MSG("[Imp Calibration] Calibration done...DRVP:%d\n", drvp);
	mcFPRINTF((fp_A60808, "[Imp Calibration] "
				"Calibration done...DRVP:%d\n", drvp));

	mcSHOW_DBG_MSG("[Imp Calibration] Calibration done...DRVN:%d\n", drvn);
	mcFPRINTF((fp_A60808, "[Imp Calibration] "
				"Calibration done...DRVN:%d\n", drvn));

	/* Derive calibration result successfully */
	drvp_status = drvn_status = 2;

	/* Set calibration result to output driving */
	if (apply == 1) {
		if ((drvp_status == 2) && (drvn_status == 2)) {
SET_DRIVING:
			/* DQS, DQ */
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_DRVCTL0);
			ucstatus |= ucDram_Register_Read(addr, &u4value);
			mcSET_FIELD(&u4value, drvp, MASK_DRVCTL0_DQSDRVP,
				    POS_DRVCTL0_DQSDRVP);
			mcSET_FIELD(&u4value, drvn, MASK_DRVCTL0_DQSDRVN,
				    POS_DRVCTL0_DQSDRVN);
			mcSET_FIELD(&u4value, drvp, MASK_DRVCTL0_DQDRVP,
				    POS_DRVCTL0_DQDRVP);
			mcSET_FIELD(&u4value, drvn, MASK_DRVCTL0_DQDRVN,
				    POS_DRVCTL0_DQDRVN);
			ucstatus |= ucDram_Register_Write(addr, u4value);

			mcSHOW_DBG_MSG2("[Imp Calibration] ");
			mcSHOW_DBG_MSG2("Reg: %#x, Val: %#x\n",
					DRAMC_REG_DRVCTL0, u4value);

			/* CLK, CMD */
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_DRVCTL1);
			ucstatus |= ucDram_Register_Read(addr, &u4value);
			mcSET_FIELD(&u4value, drvp, MASK_DRVCTL1_CLKDRVP,
				    POS_DRVCTL1_CLKDRVP);
			mcSET_FIELD(&u4value, drvn, MASK_DRVCTL1_CLKDRVN,
				    POS_DRVCTL1_CLKDRVN);
			mcSET_FIELD(&u4value, drvp, MASK_DRVCTL1_CMDDRVP,
				    POS_DRVCTL1_CMDDRVP);
			mcSET_FIELD(&u4value, drvn, MASK_DRVCTL1_CMDDRVN,
				    POS_DRVCTL1_CMDDRVN);
			ucstatus |= ucDram_Register_Write(addr, u4value);

			mcSHOW_DBG_MSG2("[Imp Calibration] ");
			mcSHOW_DBG_MSG2("Reg: %#x, Val: %#x\n",
					DRAMC_REG_DRVCTL1, u4value);

			/* DQ_2, CMD_2 */
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_IODRV4);
			ucstatus |= ucDram_Register_Read(addr, &u4value);
			mcSET_FIELD(&u4value, drvp, MASK_IODRV4_DQDRVP_2,
				    POS_IODRV4_DQDRVP_2);
			mcSET_FIELD(&u4value, drvn, MASK_IODRV4_DQDRVN_2,
				    POS_IODRV4_DQDRVN_2);
			mcSET_FIELD(&u4value, drvp, MASK_IODRV4_CMDDRVP_2,
				    POS_IODRV4_CMDDRVP_2);
			mcSET_FIELD(&u4value, drvn, MASK_IODRV4_CMDDRVN_2,
				    POS_IODRV4_CMDDRVN_2);
			ucstatus |= ucDram_Register_Write(addr, u4value);

			mcSHOW_DBG_MSG2("[Imp Calibration] ");
			mcSHOW_DBG_MSG2("Reg: %#x, Val: %#x\n",
					DRAMC_REG_IODRV4, u4value);

			if (p->channel == CHANNEL_A) {
				p->channel  = CHANNEL_B;
				goto SET_DRIVING;
			} else {
				p->channel  = CHANNEL_A;
			}
		}
	}

	/* R_DMIMP_CALI_ENP (REG.1C8[5]) = 0 */
	/* R_DMIMP_CALI_ENN (REG.1C8[4]) = 0 */
	/* R_DMIMPCALI_EN (REG.1C8[0]) = 0 */
	/* R_DMIMPDRVP / R_DMIMPDRVN = 0 to avoid current leakage */
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_IMPCAL);
	ucstatus |= ucDramC_Register_Read(addr, &u4value);
	mcCLR_BIT(&u4value, POS_IMPCAL_IMP_CALI_ENP);
	mcCLR_BIT(&u4value, POS_IMPCAL_IMP_CALI_ENN);
	mcCLR_BIT(&u4value, POS_IMPCAL_IMPCALI_EN);
	mcSET_FIELD(&u4value, 0, MASK_IMPCAL_IMPDRVP, POS_IMPCAL_IMPDRVP);
	mcSET_FIELD(&u4value, 0, MASK_IMPCAL_IMPDRVN, POS_IMPCAL_IMPDRVN);
	ucstatus |= ucDramC_Register_Write(addr, u4value);

	if (ucstatus) {
		mcSHOW_ERR_MSG("register access fail!\n");
	}

	if ((ucstatus != 0) || (drvp_status != 2) || (drvn_status != 2)) {
		return DRAM_FAIL;
	} else {
		return DRAM_OK;
	}
}

/* ------------------------------------------------------------------------- */
/*  DramcHwImpedanceCal
 *  start TX OCD impedance calibration.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  apply           (u8): 0 don't apply the register we set
				1 apply the register we set ,default don't apply.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 * ------------------------------------------------------------------------- */

/* only for ES validation */
DRAM_STATUS_T DramcHwImpedanceCal(DRAMC_CTX_T *p)
{
	u8 ucstatus = 0, uctemp;
	u32 u4value, addr;

	mcSHOW_DBG_MSG("[HW Imp Calibr] Start HW impedance calibration...\n");
	mcFPRINTF((fp_A60808, "[HW Imp Calibr] Start HW "
						"impedance calibration...\n"));
	/* DRVREF (REG.100[24]) = 0: change will be apply directly */
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_OCDK);
	ucstatus |= ucDram_Register_Read(addr, &u4value);
	mcCLR_BIT(&u4value, POS_OCDK_DRVREF);
	ucstatus |= ucDram_Register_Write(addr, u4value);

	/* Set R_DMIMPCAL_HW (REG.1C8[1]) = 1 (HW) */
	/* Set R_DMIMPCAL_CHKCYCLE (REG.1C8[19][3:2]) = 3 */
	/* Set R_DMIMPCAL_CALEN_CYCLE (REG.1C8[18:16]) = 3 */
	/* Set R_DMIMPCAL_CALICNT (REG.1C8[31:28]) = 7 */
	/* Set R_DMIMPCALCNT (REG.1C8[27:20]) = 0 @ init; 0xf @ run-time */
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_IMPCAL);
	ucstatus |= ucDramC_Register_Read(addr, &u4value);
	mcSET_BIT(&u4value, 1);
	mcSET_FIELD(&u4value, 0x3, 0x0000000c, 2);
	mcSET_FIELD(&u4value, 0x3, 0x00070000, 16);
	mcSET_FIELD(&u4value, 0x7, 0xf0000000, 28);
	mcSET_FIELD(&u4value, 0x0, 0x0ff00000, 20);
	ucstatus |= ucDramC_Register_Write(addr, u4value);

	/* Set R_DMIMPCALI_EN (REG.1C8[0]) = 1 */
	dramc_setbit(p, DRAMC_REG_IMPCAL, 0);

	/* Check calibrated result */
	/* only show in drvp_status in A60808, */
	/* not sure if HW has filled into driving */
	/* (Rome will have registers to read) */
	/* ~1.5us */
	udelay(5);

	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_DLLSTATUS0);
	ucstatus |= ucDram_Register_Read(addr, &u4value);
	uctemp = (u8)mcGET_FIELD(u4value, 0x3c000000, 26);
	mcSHOW_DBG_MSG("[HW Imp Calibr] drvp_save  : %d\n", uctemp);
	mcFPRINTF((fp_A60808, "[HW Imp Calibr] drvp_save  : %2d\n", uctemp));
	uctemp = (u8)mcGET_FIELD(u4value, 0x03c00000, 22);
	mcSHOW_DBG_MSG("[HW Imp Calibr] drvn_save  : %d\n", uctemp);
	mcFPRINTF((fp_A60808, "[HW Imp Calibr] drvn_save  : %2d\n", uctemp));
	uctemp = (u8)mcGET_FIELD(u4value, 0x003c0000, 18);
	mcSHOW_DBG_MSG("[HW Imp Calibr] drvp_save_2: %d\n", uctemp);
	mcFPRINTF((fp_A60808, "[HW Imp Calibr] drvp_save_2: %2d\n", uctemp));
	uctemp = (u8)mcGET_FIELD(u4value, 0x0003c000, 14);
	mcSHOW_DBG_MSG("[HW Imp Calibr] drvp_save_2: %d\n", uctemp);
	mcFPRINTF((fp_A60808, "[HW Imp Calibr] drvp_save_2: %2d\n", uctemp));

	/* restore settings */
	/* R_DMIMPCALI_EN (REG.1C8[0]) = 0 */
	dramc_clrbit(p, DRAMC_REG_IMPCAL, POS_IMPCAL_IMPCALI_EN);

	/* DRVREF (REG.100[24]) = 1: change will be apply during refresh */
	dramc_setbit(p, DRAMC_REG_OCDK, POS_OCDK_DRVREF);

	if (ucstatus) {
		mcSHOW_ERR_MSG("register access fail!\n");
	}

	if (ucstatus != 0) {
		return DRAM_FAIL;
	} else {
		return DRAM_OK;
	}
}

/* MT8173 LPDDR DQ -> PHY DQ mapping */
const u32 uiLPDDR_PHY_Mapping_POP_CHA[32] = {	17, 19, 20, 18, 16, 22, 23, 21,
						10, 12, 13, 8, 11, 14, 15, 9,
						31, 29, 27, 26, 30, 28, 24, 25,
						0, 2, 4, 3, 1, 5, 7, 6
					};

const u32 uiLPDDR_PHY_Mapping_POP_CHB[32] = {	14, 12, 13, 10, 8, 15, 9, 11,
						22, 23, 20, 16, 21, 18, 19, 17,
						6, 7, 3, 2, 4, 5, 1, 0,
						25, 24, 28, 29, 26, 27, 30, 31
					};

/* ------------------------------------------------------------------------- */
/*  DramcCATraining
 *  start the calibrate the skew between Clk pin and CAx pins.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 * ------------------------------------------------------------------------- */
enum {
	MAX_CLKO_DELAY = 15,
	CATRAINING_NUM = 10
};

DRAM_STATUS_T DramcCATraining(DRAMC_CTX_T *p)
{
	/* The same as above */
	u8 ucstatus = 0;
	u32 uiTemp, uiCA, addr, val;
	u32 u4prv_register_0fc, u4prv_register_044, u4prv_register_63c;

	s8 iMaxCenter;
	s8 iCAShift[CATRAINING_NUM];

	s8 CAShift_Avg = 0;
	u32 u4CAshift_Avg = 0x00;

	/* error handling */
	if (!p) {
		mcSHOW_ERR_MSG("context is NULL\n");
		return DRAM_FAIL;
	}

	if (p->dram_type != TYPE_LPDDR3) {
		mcSHOW_ERR_MSG("Wrong DRAM TYPE. ");
		mcSHOW_ERR_MSG("Only support LPDDR3 in CA training!!\n");
		return DRAM_FAIL;
	}

	/* Disable clock gating to prevent DDRPHY enter idle. */
	addr = mcSET_DRAMC_REG_ADDR(p, 0x63c);
	ucstatus |= ucDram_Register_Read(addr, &uiTemp);
	u4prv_register_63c = uiTemp;
	mcCLR_BIT(&uiTemp, 2);
	mcCLR_BIT(&uiTemp, 1);
	ucstatus |= ucDram_Register_Write(addr, uiTemp);

	/* Edward : no idea why TXP>1 will cause CA training fail. */
	/* Now set it after CA training. */
	addr = mcSET_DRAMC_REG_ADDR(p, 0x0fc);
	ucstatus |= ucDram_Register_Read(addr, &u4prv_register_0fc);
	val = u4prv_register_0fc & 0x8fffffff;
	ucstatus |= ucDram_Register_Write(addr, val);

	/* disable auto refresh: */
	/* REFCNT_FR_CLK = 0 (0x1dc[23:16]), ADVREFEN = 0 (0x44[30]) */
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_TEST2_3);
	ucstatus |= ucDram_Register_Read(addr, &uiTemp);
	u4prv_register_044 = uiTemp;
	mcCLR_BIT(&uiTemp, POS_TEST2_3_ADVREFEN);
	ucstatus |= ucDram_Register_Write(addr, uiTemp);

	/* Calculate the shift value */
	mcSHOW_DBG_MSG3("=========================================\n");
	mcSHOW_DBG_MSG3(" [Channel %d] CA training\n", p->channel);
	mcSHOW_DBG_MSG3("=========================================\n");

	mcFPRINTF((fp_A60808, "=========================================\n"));
	mcFPRINTF((fp_A60808, " [Channel %d] CA training\n", p->channel));
	mcFPRINTF((fp_A60808, "=========================================\n"));

	/* Set CA Shift Delay */
	if (p->channel == CHANNEL_A) {

		iCAShift[0] = 7;
		iCAShift[1] = 7;
		iCAShift[2] = 5;
		iCAShift[3] = 6;
		iCAShift[4] = 2;
		iCAShift[5] = 1;
		iCAShift[6] = 0;
		iCAShift[7] = 1;
		iCAShift[8] = 0;
		iCAShift[9] = 2;

		for (uiCA = 0; uiCA < CATRAINING_NUM; uiCA++)
			CAShift_Avg += iCAShift[uiCA];

		iMaxCenter = 2;

	} else {

		iCAShift[0] = 1;
		iCAShift[1] = 2;
		iCAShift[2] = 2;
		iCAShift[3] = 0;
		iCAShift[4] = 2;
		iCAShift[5] = 3;
		iCAShift[6] = 3;
		iCAShift[7] = 3;
		iCAShift[8] = 3;
		iCAShift[9] = 3;

		for (uiCA = 0; uiCA < CATRAINING_NUM; uiCA++)
			CAShift_Avg += iCAShift[uiCA];

		iMaxCenter = 2;
	}

	/* Show shift message on log */
	for (uiCA = 0; uiCA < CATRAINING_NUM; uiCA++) {
		mcSHOW_DBG_MSG3("CA%d     ", uiCA);
		mcSHOW_DBG_MSG3("Shift %d\n", iCAShift[uiCA]);
		mcFPRINTF((fp_A60808, "CA%d     ", uiCA));
		mcFPRINTF((fp_A60808, "Shift %d\n", iCAShift[uiCA]));
	}

	/* Write shift value into CA output delay. */
	if (p->channel == CHANNEL_A) {
		addr = mcSET_DRAMC_REG_ADDR(p, 0x1A8);
		ucstatus |= ucDram_Register_Read(addr, &uiTemp);
		mcSET_FIELD(&uiTemp, iCAShift[7] , 0x000f0000, 16);
		ucstatus |= ucDram_Register_Write(addr, uiTemp);

		addr = mcSET_DRAMC_REG_ADDR(p, 0x1AC);
		ucstatus |= ucDram_Register_Read(addr, &uiTemp);
		mcSET_FIELD(&uiTemp, iCAShift[5] , 0x0f000000, 24);
		ucstatus |= ucDram_Register_Write(addr, uiTemp);

		addr = mcSET_DRAMC_REG_ADDR(p, 0x1B4);
		ucstatus |= ucDram_Register_Read(addr, &uiTemp);
		mcSET_FIELD(&uiTemp, iCAShift[6] , 0x00000f00, 8);
		mcSET_FIELD(&uiTemp, iCAShift[1] , 0x000f0000, 16);
		mcSET_FIELD(&uiTemp, iCAShift[3] , 0x0f000000, 24);
		ucstatus |= ucDram_Register_Write(addr, uiTemp);

		addr = mcSET_DRAMC_REG_ADDR(p, 0x1B8);
		ucstatus |= ucDram_Register_Read(addr, &uiTemp);
		mcSET_FIELD(&uiTemp, iCAShift[0] , 0x000f0000, 16);
		mcSET_FIELD(&uiTemp, iCAShift[9] , 0x0f000000, 24);
		ucstatus |= ucDram_Register_Write(addr, uiTemp);

		addr = mcSET_DRAMC_REG_ADDR(p, 0x1BC);
		ucstatus |= ucDram_Register_Read(addr, &uiTemp);
		mcSET_FIELD(&uiTemp, iCAShift[8] , 0x000f0000, 16);
		mcSET_FIELD(&uiTemp, iCAShift[2] , 0x00000f00, 8);
		ucstatus |= ucDram_Register_Write(addr, uiTemp);

		addr = mcSET_DRAMC_REG_ADDR(p, 0x1C0);
		ucstatus |= ucDram_Register_Read(addr, &uiTemp);
		mcSET_FIELD(&uiTemp, iCAShift[4] , 0x0f000000, 24);
		ucstatus |= ucDram_Register_Write(addr, uiTemp);
	} else {
		/* cnannel B */
		addr = mcSET_DRAMC_REG_ADDR(p, 0x1A8);
		ucstatus |= ucDram_Register_Read(addr, &uiTemp);
		mcSET_FIELD(&uiTemp, iCAShift[2] , 0x000f0000, 16);
		ucstatus |= ucDram_Register_Write(addr, uiTemp);

		addr = mcSET_DRAMC_REG_ADDR(p, 0x1AC);
		ucstatus |= ucDram_Register_Read(addr, &uiTemp);
		mcSET_FIELD(&uiTemp, iCAShift[0] , 0x0f000000, 24);
		ucstatus |= ucDram_Register_Write(addr, uiTemp);

		addr = mcSET_DRAMC_REG_ADDR(p, 0x1B4);
		ucstatus |= ucDram_Register_Read(addr, &uiTemp);
		mcSET_FIELD(&uiTemp, iCAShift[3] , 0x00000f00, 8);
		mcSET_FIELD(&uiTemp, iCAShift[7] , 0x000f0000, 16);
		mcSET_FIELD(&uiTemp, iCAShift[5] , 0x0f000000, 24);
		ucstatus |= ucDram_Register_Write(addr, uiTemp);

		addr = mcSET_DRAMC_REG_ADDR(p, 0x1B8);
		ucstatus |= ucDram_Register_Read(addr, &uiTemp);
		mcSET_FIELD(&uiTemp, iCAShift[9] , 0x000f0000, 16);
		mcSET_FIELD(&uiTemp, iCAShift[4] , 0x0f000000, 24);
		ucstatus |= ucDram_Register_Write(addr, uiTemp);

		addr = mcSET_DRAMC_REG_ADDR(p, 0x1BC);
		ucstatus |= ucDram_Register_Read(addr, &uiTemp);
		mcSET_FIELD(&uiTemp, iCAShift[1] , 0x000f0000, 16);
		mcSET_FIELD(&uiTemp, iCAShift[6] , 0x00000f00, 8);
		ucstatus |= ucDram_Register_Write(addr, uiTemp);

		addr = mcSET_DRAMC_REG_ADDR(p, 0x1C0);
		ucstatus |= ucDram_Register_Read(addr, &uiTemp);
		mcSET_FIELD(&uiTemp, iCAShift[8] , 0x0f000000, 24);
		ucstatus |= ucDram_Register_Write(addr, uiTemp);
	}

	/* CKE_CS_DLY_SETTING */
	u4CAshift_Avg = (u32)(CAShift_Avg + (CATRAINING_NUM>>1)) / CATRAINING_NUM;
	/* CKEDLY : Reg.1B8h[12:8].  CSDLY : Reg.1B8h[4:0] */
	addr = mcSET_DRAMC_REG_ADDR(p, 0x1b8);
	ucstatus |= ucDram_Register_Read(addr, &uiTemp);
	uiTemp = (uiTemp & 0xffffe0e0) | (u4CAshift_Avg << 8);
	uiTemp |= u4CAshift_Avg << 0;
	ucstatus |= ucDram_Register_Write(addr, uiTemp);
	mcSHOW_DBG_MSG3("Reg.1B8h=%xh\n", uiTemp);

	/* CKE1DLY : Reg.1C4h[28:24] */
	addr = mcSET_DRAMC_REG_ADDR(p, 0x1c4);
	ucstatus |= ucDram_Register_Read(addr, &uiTemp);
	uiTemp = (uiTemp & 0xe0ffffff) | (u4CAshift_Avg << 24);
	ucstatus |= ucDram_Register_Write(addr, uiTemp);
	mcSHOW_DBG_MSG3("Reg.1C4h=%xh\n", uiTemp);

	/* CS1DLY : Reg.0Ch[31:28] */
	addr = mcSET_DRAMC_REG_ADDR(p, 0x00c);
	ucstatus |= ucDram_Register_Read(addr, &uiTemp);
	uiTemp = (uiTemp & 0x0fffffff) | (u4CAshift_Avg << 28);
	ucstatus |= ucDram_Register_Write(addr, uiTemp);
	mcSHOW_DBG_MSG3("Reg.00ch=%xh\n", uiTemp);

	DramcEnterSelfRefresh(p, 1);

	/* Write max center value into Clk output delay. */
	addr = mcSET_DRAMC_REG_ADDR(p, 0x0c);
	ucstatus |= ucDram_Register_Read(addr, &uiTemp);
	mcSET_FIELD(&uiTemp, iMaxCenter + CATRAINING_STEP, 0x0f000000, 24);
	ucstatus |= ucDram_Register_Write(addr, uiTemp);
	mcSHOW_DBG_MSG3("Reg.0Ch=%xh\n", uiTemp);
	mcFPRINTF((fp_A60808, "Reg.0Ch=%xh\n", uiTemp));

	DramcEnterSelfRefresh(p, 0);

	/* WL_CLKADJUST */
	CATrain_ClkDelay[p->channel] = iMaxCenter;
	mcSHOW_DBG_MSG3("CATrain_ClkDelay=%d...\n",
			CATrain_ClkDelay[p->channel]);

	addr = mcSET_DRAMC_REG_ADDR(p, 0x0fc);
	ucstatus |= ucDram_Register_Write(addr, u4prv_register_0fc);
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_TEST2_3);
	ucstatus |= ucDram_Register_Write(addr, u4prv_register_044);
	addr = mcSET_DRAMC_REG_ADDR(p, 0x63c);
	ucstatus |= ucDram_Register_Write(addr, u4prv_register_63c);

	if (ucstatus) {
		mcSHOW_ERR_MSG("register access fail!\n");
		return DRAM_FAIL;
	} else {
		return DRAM_OK;
	}
}

/* ------------------------------------------------------------------------- */
/*  DramcWriteLeveling
 *  start Write Leveling Calibration.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  apply           (u8): 0 don't apply the register we set
			1 apply the register we set ,default don't apply.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 * ------------------------------------------------------------------------- */

DRAM_STATUS_T DramcWriteLeveling(DRAMC_CTX_T *p, EMI_SETTINGS *emi_set)
{
	/* Note that below procedure is based on "ODT off" */
	u8 ii, byte_i, ucstatus = 0;
	u8 ucdq_o1_index[DQS_NUMBER];
	u32 u4value, u4temp, MR2, delay, addr;

	fgwrlevel_done[p->channel] = 0;

	/* error handling */
	if (!p) {
		mcSHOW_ERR_MSG("context is NULL\n");
		return DRAM_FAIL;
	}

	if ((p->dram_type != TYPE_PCDDR3) && (p->dram_type != TYPE_LPDDR3)) {
		mcSHOW_ERR_MSG("Wrong DRAM TYPE. ");
		mcSHOW_ERR_MSG("Only support DDR3 and LPDDR3 "
				"in write leveling!!\n");
		return DRAM_FAIL;
	}

	/* this depends on pinmux */
	/* select first bit of each byte */

	/* channel A/B, LP3-POP and DDR3-SBS */
	if (p->channel == CHANNEL_A) {
		if (p->dram_type == TYPE_LPDDR3) {
			ucdq_o1_index[0] = (u8)uiLPDDR_PHY_Mapping_POP_CHA[0];
			ucdq_o1_index[1] = (u8)uiLPDDR_PHY_Mapping_POP_CHA[8];
			ucdq_o1_index[2] = (u8)uiLPDDR_PHY_Mapping_POP_CHA[16];
			ucdq_o1_index[3] = (u8)uiLPDDR_PHY_Mapping_POP_CHA[24];
		} else {
			mcSHOW_ERR_MSG("[WL] Only support LPDDR3 for this board!");
			return DRAM_FAIL;
		}
	} else {
		if (p->dram_type == TYPE_LPDDR3) {
			ucdq_o1_index[0] = (u8)uiLPDDR_PHY_Mapping_POP_CHB[0];
			ucdq_o1_index[1] = (u8)uiLPDDR_PHY_Mapping_POP_CHB[8];
			ucdq_o1_index[2] = (u8)uiLPDDR_PHY_Mapping_POP_CHB[16];
			ucdq_o1_index[3] = (u8)uiLPDDR_PHY_Mapping_POP_CHB[24];
		} else {
			mcSHOW_ERR_MSG("[WL] Only support LPDDR3 for this board!");
			return DRAM_FAIL;
		}
	}

	/* set dram frequency */
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_MRS);
	MR2 = DramcGetMR2ByFreq(mt_get_dram_freq_setting());
	ucstatus |= ucDram_Register_Write(addr, MR2 | 0x00800000);

	mcSHOW_DBG_MSG("=======================================");
	mcSHOW_DBG_MSG("========================================\n");
	mcSHOW_DBG_MSG("    [Channel %d]    ", p->channel);
	mcSHOW_DBG_MSG("dramc_write_leveling_swcal\n");
	mcSHOW_DBG_MSG("=======================================");
	mcSHOW_DBG_MSG("========================================\n");

	mcFPRINTF((fp_A60808, "======================================"));
	mcFPRINTF((fp_A60808, "=========================================\n"));
	mcFPRINTF((fp_A60808, "          dramc_write_leveling_swcal\n"));
	mcFPRINTF((fp_A60808, "======================================"));
	mcFPRINTF((fp_A60808, "=========================================\n"));

	if (p->channel == CHANNEL_A) {

		wrlevel_dqs_final_delay[p->channel][0] = 5;
		wrlevel_dqs_final_delay[p->channel][1] = 6;
		wrlevel_dqs_final_delay[p->channel][2] = 5;
		wrlevel_dqs_final_delay[p->channel][3] = 6;

	} else {

		wrlevel_dqs_final_delay[p->channel][0] = 6;
		wrlevel_dqs_final_delay[p->channel][1] = 0;
		wrlevel_dqs_final_delay[p->channel][2] = 6;
		wrlevel_dqs_final_delay[p->channel][3] = 4;
	}

	/* this is used in Tx */
	/* add clk delay after WL */
	fgwrlevel_done[p->channel] = 1;

	/* CA_WL_BYTE2_FIX */
	if (p->channel == CHANNEL_B) {
		s8 val = wrlevel_dqs_final_delay[p->channel][0];

		wrlevel_dqs_final_delay[p->channel][1] = val;
	}

	/* set to best values */
	/* DQS */
	u4value = 0;
	for (byte_i = 0; byte_i < (p->data_width/DQS_BIT_NUMBER); byte_i++) {
		delay = (u32)wrlevel_dqs_final_delay[p->channel][byte_i];
		u4value += delay << (4*byte_i);
	}

	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_PADCTL3);
	ucstatus |= ucDram_Register_Write(addr, u4value);

	mcSHOW_DBG_MSG2("[write leveling]DQS: %#x", u4value);

	/* DQM */
	if (p->data_width == DATA_WIDTH_16BIT) {
		/* for DQC case, DQM3 is CS# */
		addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_PADCTL2);
		ucstatus |= ucDram_Register_Read(addr, &u4temp);
		mcSET_FIELD(&u4temp, u4value, 0x000000ff, 0);
		u4value = u4temp;
	} else {
		addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_PADCTL2);
		ucstatus |= ucDram_Register_Read(addr, &u4temp);
		mcSET_FIELD(&u4temp, u4value, 0x0000ffff, 0);
		u4value = u4temp;
	}

	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_PADCTL2);
	ucstatus |= ucDram_Register_Write(addr, u4value);

	mcSHOW_DBG_MSG2(" DQM: %#x\n", u4value);

	/* DQ delay, each DQ has 4 bits. Each register contains 8-bit DQ's */
	for (byte_i = 0; byte_i < (p->data_width/DQS_BIT_NUMBER); byte_i++) {
		u4value = 0;
		for (ii = 0; ii < DQS_BIT_NUMBER; ii++) {
			s8 val = wrlevel_dqs_final_delay[p->channel][byte_i];

			u4value += (((u32)val) << (4*ii));
		}
		addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_DQODLY1+4*byte_i);
		ucstatus |= ucDram_Register_Write(addr, u4value);
		mcSHOW_DBG_MSG2("[write leveling]DQ byte%d ", byte_i);
		mcSHOW_DBG_MSG2("reg: %#x val: %#x\n", addr, u4value);
	}

	if (ucstatus) {
		mcSHOW_ERR_MSG("register access fail!\n");
		return DRAM_FAIL;
	} else {
		return DRAM_OK;
	}

	/* log example */
	/*
	=============================================

		dramc_write_leveling_swcal
		apply=1 channel=2(2:cha, 3:chb)
	=============================================
	delay  byte0  byte1  byte2  byte3
	-----------------------------
	  0    0    0    0    1
	  1    0    0    0    1
	  2    0    0    1    1
	  3    0    0    1    1
	  4    0    0    1    1
	  5    0    0    1    1
	  6    0    0    1    1
	  7    0    0    1    1
	  8    0    0    1    1
	  9    0    0    1    1
	 10    0    0    1    1
	 11    1    1    1    1
	 pass bytecount = 4
	 byte_i    status    best delay
	 0         2         11
	 1         2         11
	 2         2         2
	 3         2         0
	 */

	return DRAM_OK;
}

/* ------------------------------------------------------------------------- */
/*  DramcRxdqsGatingCal
 *  start the dqsien software calibration.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 * ------------------------------------------------------------------------- */

static void dqsi_gw_dly_coarse_factor_handler(DRAMC_CTX_T *p, u8 curr_val)
{
	u8 ucstatus = 0;
	u8 curr_val_P1;
	u32 u4value;
	u32 u4Temp, addr;

	u32 u4CoarseTuneStart = curr_val>>2;
	/* This margin is to have rank1 coarse tune */
	/* 3 M_CK smaller margin (up & down). */
	/* According to SY, DQSINCTL=0 will have problem. */
	if (u4CoarseTuneStart > 3) {
		u4CoarseTuneStart -= 3;
	} else {
		if (u4CoarseTuneStart) {
			u4CoarseTuneStart = 1;
		}
	}

	if (u4CoarseTuneStart > 15) {
		u4CoarseTuneStart = 15;
	}

	curr_val_P1 = curr_val + 2; /* diff is 0.5T (need to check with DE) */

	/* Rank 0 P0/P1 coarse tune settings. */
	/* DQSINCTL: 0xe0[27:24]: A60808 with extra 1 bit */
	/* (unit : 1 DRAMC clock) */
	/* From coarse tune 22, so set DQSINCTL = 4 first.... */
	/* or (DQS_GW_COARSE_START)>>2?? */
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_DQSCTL1);
	ucstatus |= ucDram_Register_Read(addr, &u4value);
	mcSET_FIELD(&u4value, u4CoarseTuneStart,
		    MASK_DQSCTL1_DQSINCTL, POS_DQSCTL1_DQSINCTL);
	ucstatus |= ucDram_Register_Write(addr, u4value);

	/* TXDLY_DQSGATE: 0x404[14:12] */
	/* (unit : 1 DRAMC clock). */
	/* DQSINCTL does not have P1. */
	/* So need to use TXDLY_DQSGATE/TXDLY_DQSGATE_P1 */
	/* to set different 1 M_CK coarse tune values for P0 & P1. */
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_SELPH2);
	ucstatus |= ucDram_Register_Read(addr, &u4value);
	u4Temp = (curr_val>>2) - (u4CoarseTuneStart);
	mcSET_FIELD(&u4value, u4Temp, MASK_SELPH2_TXDLY_DQSGATE,
		    POS_SELPH2_TXDLY_DQSGATE);
	u4Temp = (curr_val_P1>>2) - (u4CoarseTuneStart);
	mcSET_FIELD(&u4value, u4Temp, MASK_SELPH2_TXDLY_DQSGATE_P1,
		    POS_SELPH2_TXDLY_DQSGATE_P1);
	ucstatus |= ucDram_Register_Write(addr, u4value);

	/* dly_DQSGATE: 0x410[23:22] (unit : 0.25 DRAMC clock) */
	/* dly_DQSGATE_P1: 0x410[25:24] (unit : 0.25 DRAMC clock) */
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_SELPH5);
	ucstatus |= ucDram_Register_Read(addr, &u4value);
	mcSET_FIELD(&u4value, curr_val&0x3,
		    MASK_SELPH5_dly_DQSGATE, POS_SELPH5_dly_DQSGATE);
	mcSET_FIELD(&u4value, curr_val_P1&0x3,
		    MASK_SELPH5_dly_DQSGATE_P1, POS_SELPH5_dly_DQSGATE_P1);
	ucstatus |= ucDram_Register_Write(addr, u4value);
}

static void dqsi_gw_dly_fine_factor_handler(DRAMC_CTX_T *p, u8 curr_val)
{
	u8 ucstatus = 0;
	u32 u4value, addr;

	/* DQS?IEN: 0x94 (each with 7 bits) */
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_DQSIEN);
	ucstatus |= ucDram_Register_Read(addr, &u4value);
	mcSET_FIELD(&u4value, curr_val&0x7f,
		    MASK_DQSIEN_R0DQS0IEN, POS_DQSIEN_R0DQS0IEN);
	mcSET_FIELD(&u4value, curr_val&0x7f,
		    MASK_DQSIEN_R0DQS1IEN, POS_DQSIEN_R0DQS1IEN);
	mcSET_FIELD(&u4value, curr_val&0x7f,
		    MASK_DQSIEN_R0DQS2IEN, POS_DQSIEN_R0DQS2IEN);
	mcSET_FIELD(&u4value, curr_val&0x7f,
		    MASK_DQSIEN_R0DQS3IEN, POS_DQSIEN_R0DQS3IEN);
	ucstatus |= ucDram_Register_Write(addr, u4value);
}

static void dqsi_gw_dly_coarse_factor_handler_rank1(DRAMC_CTX_T *p, u8 curr_val,
						    u8 DQSINCTL)
{
	u8 ucstatus = 0;
	u32 u4value;
	u8 curr_val_P1;
	u32 u4Temp, addr;
	/* According to DE, DQSINCTL & R1DQSINCTL set to the same. */
	u32 u4CoarseTuneStart = DQSINCTL;

	curr_val_P1 = curr_val + 2; /* diff is 0.5T (need to check with DE) */

	/* TO be simple, assume curr_val < 62 */
	/* R1DQSINCTL: 0x118[3:0]: (unit : 1 DRAMC clock) */
	/* From coarse tune 22, so set DQSINCTL = 4 first.... */
	/* or (DQS_GW_COARSE_START)>>2. */
	/* According to Derping, 6595 will only check DQSINCTL */
	/* and ignore R1DQSINCTL because of P1 supporting. */
	/* So R1DQSINCTL may be removed. But here still set the same value */
	/* as DQSINCTL for safety. */
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_DQSCTL2);
	ucstatus |= ucDram_Register_Read(addr, &u4value);
	mcSET_FIELD(&u4value, u4CoarseTuneStart, MASK_DQSCTL2_DQSINCTL,
		    POS_DQSCTL2_DQSINCTL);
	ucstatus |= ucDram_Register_Write(addr, u4value);

	/* TXDLY_R1DQSGATE:      0x418[6:4] (unit : 1 DRAMC clock) */
	/* TXDLY_R1DQSGATE_P1: 0x418[10:8] (unit : 1 DRAMC clock) */
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_SELPH6_1);
	ucstatus |= ucDram_Register_Read(addr, &u4value);
	u4Temp = (curr_val>>2) - (u4CoarseTuneStart);
	mcSET_FIELD(&u4value, u4Temp, MASK_SELPH6_1_TXDLY_R1DQSGATE,
		    POS_SELPH6_1_TXDLY_R1DQSGATE);
	u4Temp = (curr_val_P1>>2) - (u4CoarseTuneStart);
	mcSET_FIELD(&u4value, u4Temp, MASK_SELPH6_1_TXDLY_R1DQSGATE_P1,
		    POS_SELPH6_1_TXDLY_R1DQSGATE_P1);
	ucstatus |= ucDram_Register_Write(addr, u4value);

	/* dly_R1DQSGATE:      0x418[1:0] (unit : 0.25 DRAMC clock) */
	/* dly_R1DQSGATE_P1: 0x418[3:2] (unit : 0.25 DRAMC clock) */
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_SELPH6_1);
	ucstatus |= ucDram_Register_Read(addr, &u4value);
	mcSET_FIELD(&u4value, curr_val&0x3, MASK_SELPH6_1_dly_R1DQSGATE,
		    POS_SELPH6_1_dly_R1DQSGATE);
	mcSET_FIELD(&u4value, curr_val_P1&0x3, MASK_SELPH6_1_dly_R1DQSGATE_P1,
		    POS_SELPH6_1_dly_R1DQSGATE_P1);
	ucstatus |= ucDram_Register_Write(addr, u4value);

}

static void dqsi_gw_dly_fine_factor_handler_rank1(DRAMC_CTX_T *p, u8 curr_val)
{
	u8 ucstatus = 0;
	u32 u4value, addr;

	/* R1DQS?IEN: 0x98 (each with 7 bits) */
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_R1DQSIEN);
	ucstatus |= ucDram_Register_Read(addr, &u4value);
	mcSET_FIELD(&u4value, curr_val&0x7f,
		    MASK_DQSIEN_R1DQS0IEN, POS_DQSIEN_R1DQS0IEN);
	mcSET_FIELD(&u4value, curr_val&0x7f,
		    MASK_DQSIEN_R1DQS1IEN, POS_DQSIEN_R1DQS1IEN);
	mcSET_FIELD(&u4value, curr_val&0x7f,
		    MASK_DQSIEN_R1DQS2IEN, POS_DQSIEN_R1DQS2IEN);
	mcSET_FIELD(&u4value, curr_val&0x7f,
		    MASK_DQSIEN_R1DQS3IEN, POS_DQSIEN_R1DQS3IEN);
	ucstatus |= ucDram_Register_Write(addr, u4value);
}

/* Variables for discrete DDR MDL search */
extern u8 R0_gw_coarse_valid[2], R1_gw_coarse_valid[2];
extern u8 MDL_first_search_done;

DRAM_STATUS_T DramcRxdqsGatingCal(DRAMC_CTX_T *p)
{

	if (MDL_first_search_done == 0)
		mcSHOW_DBG_MSG("\n\n[EMI] [Channel %d] [Rank %d] "
				"MDL searching ...\n\n", p->channel, CurrentRank);
	else
		mcSHOW_DBG_MSG("\n\n[Channel %d] [Rank %d] "
				"Start tuning DRAMC Gating Window for rank ...\n\n",
				p->channel, CurrentRank);

	/* disable HW gating first 0x1c0[31] */
	dramc_clrbit(p, DRAMC_REG_DQSCAL0, POS_DQSCAL0_STBCALEN);

	/* enable burst mode for gating window */
	/* enable DQS gating window counter 0x0e0[28]=1 & 0x1e4[8]=1 */
	dramc_setbit(p, DRAMC_REG_DQSCTL1, POS_DQSCTL1_DQSIENMODE);
	dramc_setbit(p, DRAMC_REG_SPCMD, POS_SPCMD_DQSGCNTEN);

	/* dual-phase DQS clock gating control enabling 0x124[30]=1 */
	dramc_setbit(p, DRAMC_REG_DQSGCTL, POS_DQSGCTL_DQSGDUALP);

	/* Gating calibration value */
	if (p->channel == CHANNEL_A) {
		/* channel 0 (cha) */
		if( ram_code() == 0 ){
			opt_gw_coarse_value = (CurrentRank == 0)? 28: 0;
			opt_gw_fine_value = (CurrentRank == 0)? 56: 0;
		} else {
			opt_gw_coarse_value = (CurrentRank == 0)? 28: 27;
			opt_gw_fine_value = (CurrentRank == 0)? 56: 64;
		}

	} else {
		/* channel 1 (chb) */
		if( ram_code() == 0 ){
			opt_gw_coarse_value = (CurrentRank == 0)? 28: 0;
			opt_gw_fine_value = (CurrentRank == 0)? 56: 0;
		} else {
			opt_gw_coarse_value = (CurrentRank == 0)? 28: 27;
			opt_gw_fine_value = (CurrentRank == 0)? 56: 64;
		}
	}

	/* dual ranks */
	if (CurrentRank == 0) {
		opt_gw_coarse_value_R0[p->channel] = opt_gw_coarse_value;
		opt_gw_fine_value_R0[p->channel] = opt_gw_fine_value;
	} else {
		opt_gw_coarse_value_R1[p->channel] = opt_gw_coarse_value;
		opt_gw_fine_value_R1[p->channel] = opt_gw_fine_value;
	}

	mcSHOW_DBG_MSG("****************************"
			"*****************************\n");
	mcSHOW_DBG_MSG("Rank %d DQS GW Calibration\n", CurrentRank);
	mcSHOW_DBG_MSG("Optimal coarse tune value %d, "
			"optimal fine tune value %d\n",
			opt_gw_coarse_value, opt_gw_fine_value);
	mcSHOW_DBG_MSG("****************************"
			"*****************************\n");

	mcFPRINTF((fp_A60808, "******************************"
				"***************************\n"));
	mcFPRINTF((fp_A60808, "Rank %d DQS GW Calibration\n", CurrentRank));
	mcFPRINTF((fp_A60808, "Optimal coarse tune value %d, "
				"optimal fine tune value %d\n",
				opt_gw_coarse_value, opt_gw_fine_value));
	mcFPRINTF((fp_A60808, "**************************"
				"*******************************\n"));

	/* set the opt coarse value and fine value */
	dqsi_gw_dly_coarse_factor_handler(p, opt_gw_coarse_value);
	dqsi_gw_dly_fine_factor_handler(p, opt_gw_fine_value);

	return DRAM_OK;

	/*
		0  8 16 24 32 40 48 56 64 72 80 88 96 104 112 120
	      ---------------------------------------------------
	0007:|  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
	0008:|  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
	0009:|  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
	000A:|  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
	000B:|  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
	000C:|  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
	000D:|  0  0  0  0  0  0  0  0  0  0  0  0  1  1  1  1
	000E:|  0  0  0  0  0  0  0  1  1  1  1  1  1  1  1  1
	000F:|  0  0  0  1  1  1  1  1  1  1  1  0  0  0  0  0
	0010:|  1  1  1  1  1  1  1  0  0  0  0  0  0  0  0  0
	0011:|  1  1  1  0  0  0  0  0  0  0  0  0  0  0  0  0
	0012:|  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
	0013:|  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
	0014:|  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
	0015:|  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
	0016:|  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
	*/
}

DRAM_STATUS_T DualRankDramcRxdqsGatingCal(DRAMC_CTX_T *p)
{
	u8 ucstatus = 0;
	u32 uiTemp, DQSINCTL, addr;
	DRAM_STATUS_T Ret = DRAM_OK;

	/* Rank 0 GW calibration. */
	CurrentRank = 0;

	DramcRxdqsGatingCal(p);

	/* Get Reg.e0h[27:24] DQSINCTL after rank 0 calibration. */
	addr = mcSET_DRAMC_REG_ADDR(p, 0xe0);
	ucstatus |= ucDram_Register_Read(addr, &DQSINCTL);
	DQSINCTL = (DQSINCTL >> 24) & 0xf;

	/* Rank 1 GW calibration. */
	/* Swap CS0 and CS1. */
	addr = mcSET_DRAMC_REG_ADDR(p, 0x110);
	ucstatus |= ucDram_Register_Read(addr, &uiTemp);
	uiTemp = uiTemp | 0x08;
	ucstatus |= ucDram_Register_Write(addr, uiTemp);

	CurrentRank = 1;
	DramcRxdqsGatingCal(p);

	/* No need to set rank 1 coarse tune because only one set registers */
	/* for coarse tune in 808 */
	dqsi_gw_dly_coarse_factor_handler_rank1(p, opt_gw_coarse_value_R1[p->channel], DQSINCTL);
	dqsi_gw_dly_fine_factor_handler_rank1(p, opt_gw_fine_value_R1[p->channel]);

	/* Swap CS back. */
	addr = mcSET_DRAMC_REG_ADDR(p, 0x110);
	ucstatus |= ucDram_Register_Read(addr, &uiTemp);
	uiTemp = uiTemp & (~0x08);
	ucstatus |= ucDram_Register_Write(addr, uiTemp);

	CurrentRank = 0;
	/* Set rank 0 coarse tune and fine tune back. */
	dqsi_gw_dly_coarse_factor_handler(p, opt_gw_coarse_value_R0[p->channel]);
	dqsi_gw_dly_fine_factor_handler(p, opt_gw_fine_value_R0[p->channel]);

	return Ret;
}

/* ------------------------------------------------------------------------- */
/*  DramcRxWindowPerbitCal (v2 version)
 *  start the rx dqs perbit sw calibration.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 * ------------------------------------------------------------------------- */

DRAM_STATUS_T DramcRxWindowPerbitCal(DRAMC_CTX_T *p)
{
	u8 ucstatus = 0, ii, jj, kk;
	u32 u4value, addr;

	RXDQS_PERBIT_DLY_T dqdqs_perbit_dly[DQ_DATA_WIDTH];
	u8 ucmax_dqsdly_byte[DQS_NUMBER];

	/* error handling */
	if (!p) {
		mcSHOW_ERR_MSG("context is NULL\n");
		return DRAM_FAIL;
	}

	/* for debug */
	mcSHOW_DBG_MSG3("\n[Channel %d] RX DQS per bit para : test pattern=%d, "
			"test2_1=%#x, test2_2=%#x, LSW_HSR=%d\n", p->channel,
			p->test_pattern, p->test2_1, p->test2_2,
			p->fglow_freq_write_en);

	if (p->channel == CHANNEL_A) {
		/* DQS */
		ucmax_dqsdly_byte[0] = 7;
		ucmax_dqsdly_byte[1] = 8;
		ucmax_dqsdly_byte[2] = 10;
		ucmax_dqsdly_byte[3] = 10;
		/* DQ0~7 */
		dqdqs_perbit_dly[0].best_dqdly = 2;
		dqdqs_perbit_dly[1].best_dqdly = 2;
		dqdqs_perbit_dly[2].best_dqdly = 1;
		dqdqs_perbit_dly[3].best_dqdly = 0;
		dqdqs_perbit_dly[4].best_dqdly = 2;
		dqdqs_perbit_dly[5].best_dqdly = 1;
		dqdqs_perbit_dly[6].best_dqdly = 2;
		dqdqs_perbit_dly[7].best_dqdly = 4;
		/* DQ8~15 */
		dqdqs_perbit_dly[8].best_dqdly = 0;
		dqdqs_perbit_dly[9].best_dqdly = 2;
		dqdqs_perbit_dly[10].best_dqdly = 2;
		dqdqs_perbit_dly[11].best_dqdly = 3;
		dqdqs_perbit_dly[12].best_dqdly = 3;
		dqdqs_perbit_dly[13].best_dqdly = 1;
		dqdqs_perbit_dly[14].best_dqdly = 3;
		dqdqs_perbit_dly[15].best_dqdly = 2;
		/* DQ16~23 */
		dqdqs_perbit_dly[16].best_dqdly = 0;
		dqdqs_perbit_dly[17].best_dqdly = 2;
		dqdqs_perbit_dly[18].best_dqdly = 4;
		dqdqs_perbit_dly[19].best_dqdly = 1;
		dqdqs_perbit_dly[20].best_dqdly = 4;
		dqdqs_perbit_dly[21].best_dqdly = 4;
		dqdqs_perbit_dly[22].best_dqdly = 4;
		dqdqs_perbit_dly[23].best_dqdly = 4;
		/* DQ24~31 */
		dqdqs_perbit_dly[24].best_dqdly = 3;
		dqdqs_perbit_dly[25].best_dqdly = 4;
		dqdqs_perbit_dly[26].best_dqdly = 3;
		dqdqs_perbit_dly[27].best_dqdly = 3;
		dqdqs_perbit_dly[28].best_dqdly = 1;
		dqdqs_perbit_dly[29].best_dqdly = 2;
		dqdqs_perbit_dly[30].best_dqdly = 1;
		dqdqs_perbit_dly[31].best_dqdly = 0;
	} else {
		/* DQS*/
		ucmax_dqsdly_byte[0] = 8;
		ucmax_dqsdly_byte[1] = 8;
		ucmax_dqsdly_byte[2] = 9;
		ucmax_dqsdly_byte[3] = 8;
		/* DQ0~7 */
		dqdqs_perbit_dly[0].best_dqdly = 2;
		dqdqs_perbit_dly[1].best_dqdly = 3;
		dqdqs_perbit_dly[2].best_dqdly = 3;
		dqdqs_perbit_dly[3].best_dqdly = 3;
		dqdqs_perbit_dly[4].best_dqdly = 0;
		dqdqs_perbit_dly[5].best_dqdly = 2;
		dqdqs_perbit_dly[6].best_dqdly = 1;
		dqdqs_perbit_dly[7].best_dqdly = 4;
		/* DQ8~15 */
		dqdqs_perbit_dly[8].best_dqdly = 0;
		dqdqs_perbit_dly[9].best_dqdly = 2;
		dqdqs_perbit_dly[10].best_dqdly = 3;
		dqdqs_perbit_dly[11].best_dqdly = 2;
		dqdqs_perbit_dly[12].best_dqdly = 3;
		dqdqs_perbit_dly[13].best_dqdly = 2;
		dqdqs_perbit_dly[14].best_dqdly = 2;
		dqdqs_perbit_dly[15].best_dqdly = 2;
		/* DQ16~23 */
		dqdqs_perbit_dly[16].best_dqdly = 0;
		dqdqs_perbit_dly[17].best_dqdly = 1;
		dqdqs_perbit_dly[18].best_dqdly = 2;
		dqdqs_perbit_dly[19].best_dqdly = 1;
		dqdqs_perbit_dly[20].best_dqdly = 2;
		dqdqs_perbit_dly[21].best_dqdly = 3;
		dqdqs_perbit_dly[22].best_dqdly = 3;
		dqdqs_perbit_dly[23].best_dqdly = 3;
		/* DQ24~31 */
		dqdqs_perbit_dly[24].best_dqdly = 3;
		dqdqs_perbit_dly[25].best_dqdly = 2;
		dqdqs_perbit_dly[26].best_dqdly = 3;
		dqdqs_perbit_dly[27].best_dqdly = 3;
		dqdqs_perbit_dly[28].best_dqdly = 2;
		dqdqs_perbit_dly[29].best_dqdly = 1;
		dqdqs_perbit_dly[30].best_dqdly = 2;
		dqdqs_perbit_dly[31].best_dqdly = 1;
	}

	mcSHOW_DBG_MSG4("========================"
			"==========================\n");
	mcSHOW_DBG_MSG4("    dramc_rxdqs_perbit_swcal\n");
	mcSHOW_DBG_MSG4("    channel=%d(0:cha, 1:chb)\n", p->channel);
	mcSHOW_DBG_MSG4("    bus width=%d\n", p->data_width);
	mcSHOW_DBG_MSG4("========================"
			"==========================\n");
	mcSHOW_DBG_MSG4("DQS Delay :\n DQS0 = %d DQS1 = %d ",
			ucmax_dqsdly_byte[0], ucmax_dqsdly_byte[1]);
	mcSHOW_DBG_MSG4("DQS2 = %d DQS3 = %d\n",
			ucmax_dqsdly_byte[2], ucmax_dqsdly_byte[3]);
	mcSHOW_DBG_MSG4("DQ Delay :\n");

	mcFPRINTF((fp_A60808, "========================="
				"=========================\n"));
	mcFPRINTF((fp_A60808, "    dramc_rxdqs_perbit_swcal\n"));
	mcFPRINTF((fp_A60808, "    channel=%d(0:cha, 1:chb)\n", p->channel));
	mcFPRINTF((fp_A60808, "    bus width=%d\n", p->data_width));
	mcFPRINTF((fp_A60808, "========================="
				"=========================\n"));
	mcFPRINTF((fp_A60808, "DQS Delay :\n DQS0 = %d DQS1 = %d ",
		   ucmax_dqsdly_byte[0], ucmax_dqsdly_byte[1]));
	mcFPRINTF((fp_A60808, "DQS2 = %d DQS3 = %d\n",
		   ucmax_dqsdly_byte[2], ucmax_dqsdly_byte[3]));
	mcFPRINTF((fp_A60808, "DQ Delay :\n"));

	for (ii = 0; ii < p->data_width; ii = ii+4) {

		mcSHOW_DBG_MSG4("DQ%d = %d DQ%d = %d DQ%d = %d DQ%d = %d\n",
				ii, dqdqs_perbit_dly[ii].best_dqdly,
				ii+1, dqdqs_perbit_dly[ii+1].best_dqdly,
				ii+2, dqdqs_perbit_dly[ii+2].best_dqdly,
				ii+3, dqdqs_perbit_dly[ii+3].best_dqdly);

		mcFPRINTF((fp_A60808, "DQ%2d = %2d DQ%2d = %2d "
					"DQ%2d = %2d DQ%2d = %2d\n",
					ii, dqdqs_perbit_dly[ii].best_dqdly,
					ii+1, dqdqs_perbit_dly[ii+1].best_dqdly,
					ii+2, dqdqs_perbit_dly[ii+2].best_dqdly,
					ii+3, dqdqs_perbit_dly[ii+3].best_dqdly));
	}

	mcSHOW_DBG_MSG4("________________________________________\n");
	mcFPRINTF((fp_A60808, "________________________________________\n"));

	/* set dqs delay */
	u4value = 0;
	for (jj = 0; jj < (p->data_width/DQS_BIT_NUMBER); jj++) {
		u4value += (((u32)ucmax_dqsdly_byte[jj])<<(8*jj));
	}

	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_DELDLY1);
	ucstatus |= ucDram_Register_Write(addr, u4value);
	addr = mcSET_DRAMC_REG_ADDR(p, 0x01c);
	ucstatus |= ucDram_Register_Write(addr, u4value);

	/* set dq delay */
	for (jj = 0; jj < p->data_width; jj = jj+4) {

		/* 20130528, no need to swap, */
		/* since DQ_DLY & CMP_ERR has swapped by HW */
		kk = jj;
		/* every 4bit dq have the same delay register address */
		u4value = ((u32)dqdqs_perbit_dly[kk].best_dqdly);
		u4value += (((u32)dqdqs_perbit_dly[kk+1].best_dqdly)<<8);
		u4value += (((u32)dqdqs_perbit_dly[kk+2].best_dqdly)<<16);
		u4value += (((u32)dqdqs_perbit_dly[kk+3].best_dqdly)<<24);

		addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_DQIDLY1+jj);
		ucstatus |= ucDram_Register_Write(addr, u4value);
	}

	/* Log example */
	/*
	 ==================================================
	   dramc_rxdqs_perbit_swcal_v2
	   channel=2(2:cha, 3:chb) apply = 1
	 ==================================================
	 DQS Delay :
	 DQS0 = 0 DQS1 = 0 DQS2 = 0 DQS3 = 0
	 DQ Delay :
	 DQ 0 =  1 DQ 1 =  1 DQ 2 =  2 DQ 3 =  1
	 DQ 4 =  1 DQ 5 =  1 DQ 6 =  1 DQ 7 =  1
	 DQ 8 =  2 DQ 9 =  1 DQ10 =  1 DQ11 =  1
	 DQ12 =  1 DQ13 =  1 DQ14 =  1 DQ15 =  2
	 DQ16 =  2 DQ17 =  1 DQ18 =  2 DQ19 =  2
	 DQ20 =  0 DQ21 =  0 DQ22 =  0 DQ23 =  0
	 DQ24 =  3 DQ25 =  3 DQ26 =  3 DQ27 =  2
	 DQ28 =  2 DQ29 =  1 DQ30 =  3 DQ31 =  1
	 ________________________________________
	*/

	if (ucstatus) {
		mcSHOW_ERR_MSG("register access fail!\n");
		return DRAM_FAIL;
	} else {
		return DRAM_OK;
	}
}

void DramcClkDutyCal(DRAMC_CTX_T *p)
{
	u16 max_win_size = 0;
	u8  max_duty_sel, max_duty, ucstatus = 0;
	u32 u4value, addr;

	max_duty_sel = max_duty = 1;

	/* set optimal CLK duty settings */
	DramcEnterSelfRefresh(p, 1);

	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_PHYCLKDUTY);
	ucstatus |= ucDram_Register_Read(addr, &u4value);

	if (max_duty_sel == 0) {
		mcCLR_BIT(&u4value, POS_PHYCLKDUTY_CMDCLKP0DUTYSEL);
		mcSET_FIELD(&u4value, 0, MASK_PHYCLKDUTY_CMDCLKP0DUTYN,
			    POS_PHYCLKDUTY_CMDCLKP0DUTYN);
		mcSET_FIELD(&u4value, max_duty, MASK_PHYCLKDUTY_CMDCLKP0DUTYP,
			    POS_PHYCLKDUTY_CMDCLKP0DUTYP);
	} else {
		mcSET_BIT(&u4value, POS_PHYCLKDUTY_CMDCLKP0DUTYSEL);
		mcSET_FIELD(&u4value, max_duty, MASK_PHYCLKDUTY_CMDCLKP0DUTYN,
			    POS_PHYCLKDUTY_CMDCLKP0DUTYN);
		mcSET_FIELD(&u4value, 0, MASK_PHYCLKDUTY_CMDCLKP0DUTYP,
			    POS_PHYCLKDUTY_CMDCLKP0DUTYP);
	}

	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_PHYCLKDUTY);
	ucstatus |= ucDram_Register_Write(addr, u4value);

	DramcEnterSelfRefresh(p, 0);

	mcSHOW_DBG_MSG("[Channel %d CLK DUTY CALIB] ", p->channel);
	mcSHOW_DBG_MSG("Final DUTY_SEL=%d, DUTY=%d ",
		       max_duty_sel, max_duty);
	mcSHOW_DBG_MSG("--> rx window size=%d, Reg.148h=%xh\n",
		       max_win_size, u4value);

	mcFPRINTF((fp_A60808, "[CLK DUTY CALIB] "));
	mcFPRINTF((fp_A60808, "Final DUTY_SEL=%d, DUTY=%d ",
		   max_duty_sel, max_duty));
	mcFPRINTF((fp_A60808, "--> rx window size=%d\n", max_win_size));
}

/* add by KT, define the function in ett_cust.dle.c */
static void dle_factor_handler(DRAMC_CTX_T *p, u8 curr_val)
{
	u8 ucstatus = 0;
	u32 u4value, addr;
	u32 u4curr_val_DSEL;

	/* DATLAT: DRAMC_DDR2CTL[4:6], 3 bits */
	dramc_setfield(p, DRAMC_REG_DDR2CTL, (curr_val&0x7),
			 MASK_DDR2CTL_DATLAT, POS_DDR2CTL_DTALAT);

	/* DATLAT3: DRAMC_PADCTL1[4], 1 bit */
	dramc_setfield(p, DRAMC_REG_PADCTL4, ((curr_val>>3)&0x1),
			 MASK_PADCTL4_DATLAT3, POS_PADCTL4_DATLAT3);

	/* DATLAT4: 0xf0[25], 1 bit */
	dramc_setfield(p, DRAMC_REG_PHYCTL1, ((curr_val>>4)&0x1),
			 MASK_PHYCTL1_DATLAT4, POS_PHYCTL1_DATLAT4);

	/* DATLAT_DSEL (Reg.80h[12:8]) = DATLAT - 2*3 - 2*1 (Speed >=1600) */
	/* DATLAT_DSEL = DATLAT - 2*1 - 2*1 (Speed <1600) */
	addr = mcSET_DRAMC_REG_ADDR(p, 0x80);
	ucstatus |= ucDram_Register_Read(addr, &u4value);

	if (curr_val >= 8) {
		u4curr_val_DSEL = curr_val - 8;
	} else {
		u4curr_val_DSEL = 0;
	}

	dramc_setfield(p, 0x80, u4curr_val_DSEL, 0x00001f00, 8);

	/* only for HW run time test engine use, optimize bandwidth */

	if (curr_val >= 3) {
		dramc_setfield(p, (uintptr_t)&ao_regs->misc, (curr_val-3)&0x1f, 0x0000001f, 0);
	} else {
		dramc_setfield(p, (uintptr_t)&ao_regs->misc, 0, 0x0000001f, 0);
	}
}

/*
 * The difference of tDQSCK (2.5~5.5ns) is 3ns
 * which is larger than 1 M_CK (2.14ns in 1866Mbps).
 * Should set to the maximum of dual rank.
 * If calibration could detect the end of window,
 * set the center of overlapped window.
 */
DRAM_STATUS_T DramcDualRankRxdatlatCal(DRAMC_CTX_T *p)
{
	u8 ucstatus = 0, ucR0DLESetting;
	u32 u4value, addr;

	/* Rank 0 DLE calibration. */
	CurrentRank = 0;
	DramcRxdatlatCal((DRAMC_CTX_T *)p);
	ucR0DLESetting = ucDLESetting;

	/* Rank 1 DLE calibration. */
	/* Swap CS0 and CS1. */
	addr = mcSET_DRAMC_REG_ADDR(p, 0x110);
	ucstatus |= ucDram_Register_Read(addr, &u4value);
	u4value = u4value | 0x08;
	ucstatus |= ucDram_Register_Write(addr, u4value);

	/* Set rank 1 coarse tune and fine tune back. */
	dqsi_gw_dly_coarse_factor_handler(p, opt_gw_coarse_value_R1[p->channel]);
	dqsi_gw_dly_fine_factor_handler(p, opt_gw_fine_value_R1[p->channel]);

	CurrentRank = 1;

	DramcRxdatlatCal((DRAMC_CTX_T *)p);

	mcSHOW_DBG_MSG("Rank 0 DLE calibrated setting = %xh. "
			"Rank 1 DLE calibrated setting = %xh\n",
				ucR0DLESetting, ucDLESetting);

	if (ucDLESetting < ucR0DLESetting) {

		mcSHOW_DBG_MSG("Rank 0 %xh > Rank 1 %xh. Set to rank 0 %xh.\n",
			       ucR0DLESetting, ucDLESetting, ucR0DLESetting);

		dle_factor_handler(p, ucR0DLESetting);

	} else {
		mcSHOW_DBG_MSG("Rank 0 %xh < Rank 1 %xh. Use rank 1 %xh.\n",
			       ucR0DLESetting, ucDLESetting, ucDLESetting);
	}

	/* Set rank 0 coarse tune and fine tune back. */
	dqsi_gw_dly_coarse_factor_handler(p, opt_gw_coarse_value_R0[p->channel]);
	dqsi_gw_dly_fine_factor_handler(p, opt_gw_fine_value_R0[p->channel]);

	/* Swap CS back. */
	addr = mcSET_DRAMC_REG_ADDR(p, 0x110);
	ucstatus |= ucDram_Register_Read(addr, &u4value);
	u4value = u4value & (~0x08);
	ucstatus |= ucDram_Register_Write(addr, u4value);

	CurrentRank = 0;

	return ucstatus;
}

/* ------------------------------------------------------------------------- */
/*  DramcRxdatlatCal
 *  scan the pass range of DATLAT for DDRPHY read data window.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 * ------------------------------------------------------------------------- */

DRAM_STATUS_T DramcRxdatlatCal(DRAMC_CTX_T *p)
{
	/* A60807 */
	/* u8 ucstatus = 0, ii, ucStartCalVal = 0; */
	u8 ucstatus = 0;
	u32 u4prv_register_07c, u4prv_register_0e4;
	u32 u4prv_register_0f0, u4prv_register_080, u4prv_register_0d8;

	u32 u4value, addr;
	u8 ucfirst, ucbegin, ucsum, ucbest_step;

	/* error handling */
	if (!p) {
		mcSHOW_ERR_MSG("context is NULL\n");
		return DRAM_FAIL;
	}

	mcSHOW_DBG_MSG("\n===============================");
	mcSHOW_DBG_MSG("===============================\n");
	mcSHOW_DBG_MSG("    [Channel %d] [Rank %d] DATLAT calibration\n",
		       p->channel, CurrentRank);
	mcSHOW_DBG_MSG("    channel=%d(0:cha)\n", p->channel);
	mcSHOW_DBG_MSG("=================================");
	mcSHOW_DBG_MSG("=============================\n");

	mcFPRINTF((fp_A60808, "\n=============================="));
	mcFPRINTF((fp_A60808, "================================\n"));
	mcFPRINTF((fp_A60808, "    DATLAT calibration\n"));
	mcFPRINTF((fp_A60808, "    channel=%d(0:cha)\n", p->channel));
	mcFPRINTF((fp_A60808, "================================="));
	mcFPRINTF((fp_A60808, "=============================\n"));

	/* [11:10] DQIENQKEND 01 -> 00 for DATLAT calibration issue, */
	/* DQS input enable will refer to DATLAT */
	/* if need to enable this (for power saving), */
	/* do it after all calibration done */
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_MCKDLY);
	ucstatus |= ucDram_Register_Read(addr, &u4value);
	u4prv_register_0d8 = u4value;
	mcSET_FIELD(&u4value, 0x00, MASK_MCKDLY_DQIENQKEND,
		    POS_MCKDLY_DQIENQKEND);
	mcCLR_BIT(&u4value, 4);
	ucstatus |= ucDram_Register_Write(addr, u4value);

	/* pre-save */
	/* 0x07c[6:4]   DATLAT bit2-bit0 */
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_DDR2CTL);
	ucstatus |= ucDram_Register_Read(addr, &u4prv_register_07c);
	/* 0x0e4[4]     DALLAT bit3 */
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_PADCTL4);
	ucstatus |= ucDram_Register_Read(addr, &u4prv_register_0e4);
	/* 0x0f0[25]    DATLAT bit 4 */
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_PHYCTL1);
	ucstatus |= ucDram_Register_Read(addr, &u4prv_register_0f0);
	addr = mcSET_DRAMC_REG_ADDR(p, 0x80);
	ucstatus |= ucDram_Register_Read(addr, &u4prv_register_080);

	/* init best_step to default */
	ucbest_step = (u8)((u4prv_register_07c>>4) & 0x7);
	ucbest_step |= ((u4prv_register_0e4>>1) & 0x8);
	ucbest_step |= ((u4prv_register_0f0>>21) & 0x10);

	mcSHOW_DBG_MSG("DATLAT Default value = %#x\n", ucbest_step);
	mcFPRINTF((fp_A60808, "DATLAT Default value = %#x\n", ucbest_step));

	/* 1.set DATLAT 0-15 (0-21 for MT6595) */
	/* 2.enable engine1 or engine2 */
	/* 3.check result  ,3~4 taps pass */
	/* 4.set DATLAT 2nd value for optimal */

	/* Initialize */
	ucfirst = 0xff;
	ucbegin = 0;
	ucsum = 0;

	ucstatus |= ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(p, 0x80), &u4value);

	/* set calibration value */
	ucsum = 2;
	ucfirst = 19;

	/* 4 */
	if (ucsum == 0) {
		mcSHOW_ERR_MSG("no DATLAT taps pass, DATLAT calibration fail!!\n");
	} else if (ucsum == 1) {
		ucbest_step = ucfirst;
		mcSHOW_ERR_MSG("only one pass tap!!\n");
	} else {
		ucbest_step = ucfirst + 1;
	}

	mcSHOW_DBG_MSG("pattern=%d(0: ISI, 1: AUDIO, 2: TA4, 3: TA4-3) "
			"first_step=%d total pass=%d best_step=%d\n",
			p->test_pattern, ucfirst, ucsum, ucbest_step);
	mcFPRINTF((fp_A60808, "pattern=%d(0: ISI, 1: AUDIO, 2: TA4, 3: TA4-3) "
				"first_step=%d total pass=%d best_step=%d\n",
				p->test_pattern, ucfirst, ucsum, ucbest_step));

	if (ucsum == 0) {
		mcSHOW_ERR_MSG("DATLAT calibration fail, "
				"write back to default values!\n");

		addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_DDR2CTL);
		ucstatus |= ucDram_Register_Write(addr, u4prv_register_07c);
		addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_PADCTL4);
		ucstatus |= ucDram_Register_Write(addr, u4prv_register_0e4);
		addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_PHYCTL1);
		ucstatus |= ucDram_Register_Write(addr, u4prv_register_0f0);
		addr = mcSET_DRAMC_REG_ADDR(p, 0x80);
		ucstatus |= ucDram_Register_Write(addr, u4prv_register_080);
	} else {
		dle_factor_handler(p, ucbest_step);
	}

	if (ucstatus) {
		mcSHOW_ERR_MSG("register access fail!\n");
		return DRAM_FAIL;
	} else {
		return DRAM_OK;
	}
}

/*
 * This subroutine is used in function DramcTxWindowPerbitCal
 */
void TxDelayForWriteLeveling(DRAMC_CTX_T *p, TXDQS_PERBIT_DLY_T *dqdqs_perbit_dly,
			     u8 *ucave_dqdly_byte, u8 *ucmax_dqsdly_byte)
{
	u8 ii, jj, delta;
	u8 ucbit_first, ucbit_last;

	for (ii = 0; ii < (p->data_width/DQS_BIT_NUMBER); ii++) {
		mcSHOW_DBG_MSG4("DQS%d: %d  ", ii, wrlevel_dqs_final_delay[p->channel][ii]);
		mcFPRINTF((fp_A60808, "DQS%d: %d  ", ii, wrlevel_dqs_final_delay[p->channel][ii]));

		if (ucmax_dqsdly_byte[ii] <= wrlevel_dqs_final_delay[p->channel][ii]) {

			/* DQ */
			ucbit_first = DQS_BIT_NUMBER*ii;
			ucbit_last = DQS_BIT_NUMBER*ii+DQS_BIT_NUMBER-1;
			/* set diff value (delta) */
			delta = wrlevel_dqs_final_delay[p->channel][ii] - ucmax_dqsdly_byte[ii];

			for (jj = ucbit_first; jj <= ucbit_last; jj++) {

				dqdqs_perbit_dly[jj].best_dqdly += delta;
				/* max limit to 15 */
				if (dqdqs_perbit_dly[jj].best_dqdly > (MAX_TX_DQDLY_TAPS-1)) {
					dqdqs_perbit_dly[jj].best_dqdly = MAX_TX_DQDLY_TAPS-1;
				} else {
					dqdqs_perbit_dly[jj].best_dqdly = dqdqs_perbit_dly[jj].best_dqdly;
				}
			}

			/* DQM */
			ucave_dqdly_byte[ii] += delta;
			/* max limit to 15 */
			if (ucave_dqdly_byte[ii] > (MAX_TX_DQDLY_TAPS-1)) {
				ucave_dqdly_byte[ii] = MAX_TX_DQDLY_TAPS-1;
			} else {
				ucave_dqdly_byte[ii] = ucave_dqdly_byte[ii];
			}

			/* DQS */
			ucmax_dqsdly_byte[ii] = wrlevel_dqs_final_delay[p->channel][ii];

		} else {
			/* ucmax_dqsdly_byte[ii] > wrlevel_dqs_final_delay[p->channel][ii]) */
			/* Originally should move clk delay and CA delay accordingly. */
			/* Then GW calibration again. Too complicated. */
			/* DQ/DQS skew should not be large according to DE. */
			/* So sacrifice the Clk/DQS margin by keeping the clk out delay. */
			mcSHOW_DBG_MSG4("[Warning] DQSO %d in TX "
					"per-bit = %d > DQSO %d in WL = %d  ",
					ii, ucmax_dqsdly_byte[ii], ii,
					wrlevel_dqs_final_delay[p->channel][ii]);
		}
	}
}

/* ------------------------------------------------------------------------- */
/*  DramcTxWindowPerbitCal (v2)
 *  TX DQS per bit SW calibration.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  apply           (u8): 0 don't apply the register we set 1 apply
 *				the register we set, default don't apply.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 * ------------------------------------------------------------------------- */

DRAM_STATUS_T DramcTxWindowPerbitCal(DRAMC_CTX_T *p)
{
	u8 ucstatus = 0, ii, jj;
	u32 u4value, addr;

	TXDQS_PERBIT_DLY_T dqdqs_perbit_dly[DQ_DATA_WIDTH];
	u8 ucbit_first, ucbit_last;
	u8 ucmax_dqsdly_byte[DQS_NUMBER], ucave_dqdly_byte[DQS_NUMBER];
	u8 ucfail = 0;

	/* error handling */
	if (!p) {
		mcSHOW_ERR_MSG("context is NULL\n");
		return DRAM_FAIL;
	}

	/* calibrated value */
	if (p->channel == CHANNEL_A) {
		/* DQS */
		ucmax_dqsdly_byte[0] = 0;
		ucmax_dqsdly_byte[1] = 0;
		ucmax_dqsdly_byte[2] = 0;
		ucmax_dqsdly_byte[3] = 0;
		/* DQM */
		ucave_dqdly_byte[0] = 1;
		ucave_dqdly_byte[1] = 1;
		ucave_dqdly_byte[2] = 1;
		ucave_dqdly_byte[3] = 1;
		/* DQ0~7 */
		dqdqs_perbit_dly[0].best_dqdly = 2;
		dqdqs_perbit_dly[1].best_dqdly = 3;
		dqdqs_perbit_dly[2].best_dqdly = 3;
		dqdqs_perbit_dly[3].best_dqdly = 2;
		dqdqs_perbit_dly[4].best_dqdly = 3;
		dqdqs_perbit_dly[5].best_dqdly = 3;
		dqdqs_perbit_dly[6].best_dqdly = 3;
		dqdqs_perbit_dly[7].best_dqdly = 3;
		/* DQ8~15 */
		dqdqs_perbit_dly[8].best_dqdly = 1;
		dqdqs_perbit_dly[9].best_dqdly = 2;
		dqdqs_perbit_dly[10].best_dqdly = 2;
		dqdqs_perbit_dly[11].best_dqdly = 2;
		dqdqs_perbit_dly[12].best_dqdly = 1;
		dqdqs_perbit_dly[13].best_dqdly = 1;
		dqdqs_perbit_dly[14].best_dqdly = 2;
		dqdqs_perbit_dly[15].best_dqdly = 2;
		/* DQ16~23 */
		dqdqs_perbit_dly[16].best_dqdly = 0;
		dqdqs_perbit_dly[17].best_dqdly = 1;
		dqdqs_perbit_dly[18].best_dqdly = 1;
		dqdqs_perbit_dly[19].best_dqdly = 1;
		dqdqs_perbit_dly[20].best_dqdly = 1;
		dqdqs_perbit_dly[21].best_dqdly = 2;
		dqdqs_perbit_dly[22].best_dqdly = 1;
		dqdqs_perbit_dly[23].best_dqdly = 2;
		/* DQ24~31 */
		dqdqs_perbit_dly[24].best_dqdly = 2;
		dqdqs_perbit_dly[25].best_dqdly = 2;
		dqdqs_perbit_dly[26].best_dqdly = 2;
		dqdqs_perbit_dly[27].best_dqdly = 2;
		dqdqs_perbit_dly[28].best_dqdly = 1;
		dqdqs_perbit_dly[29].best_dqdly = 1;
		dqdqs_perbit_dly[30].best_dqdly = 0;
		dqdqs_perbit_dly[31].best_dqdly = 1;

	} else {
		/* DQS */
		ucmax_dqsdly_byte[0] = 0;
		ucmax_dqsdly_byte[1] = 0;
		ucmax_dqsdly_byte[2] = 0;
		ucmax_dqsdly_byte[3] = 0;
		/* DQM */
		ucave_dqdly_byte[0] = 2;
		ucave_dqdly_byte[1] = 2;
		ucave_dqdly_byte[2] = 1;
		ucave_dqdly_byte[3] = 2;
		/* DQ0~7 */
		dqdqs_perbit_dly[0].best_dqdly = 2;
		dqdqs_perbit_dly[1].best_dqdly = 2;
		dqdqs_perbit_dly[2].best_dqdly = 3;
		dqdqs_perbit_dly[3].best_dqdly = 2;
		dqdqs_perbit_dly[4].best_dqdly = 2;
		dqdqs_perbit_dly[5].best_dqdly = 2;
		dqdqs_perbit_dly[6].best_dqdly = 2;
		dqdqs_perbit_dly[7].best_dqdly = 3;
		/* DQ8~15 */
		dqdqs_perbit_dly[8].best_dqdly = 1;
		dqdqs_perbit_dly[9].best_dqdly = 2;
		dqdqs_perbit_dly[10].best_dqdly = 2;
		dqdqs_perbit_dly[11].best_dqdly = 3;
		dqdqs_perbit_dly[12].best_dqdly = 2;
		dqdqs_perbit_dly[13].best_dqdly = 2;
		dqdqs_perbit_dly[14].best_dqdly = 2;
		dqdqs_perbit_dly[15].best_dqdly = 2;
		/* DQ16~23 */
		dqdqs_perbit_dly[16].best_dqdly = 0;
		dqdqs_perbit_dly[17].best_dqdly = 1;
		dqdqs_perbit_dly[18].best_dqdly = 1;
		dqdqs_perbit_dly[19].best_dqdly = 1;
		dqdqs_perbit_dly[20].best_dqdly = 1;
		dqdqs_perbit_dly[21].best_dqdly = 1;
		dqdqs_perbit_dly[22].best_dqdly = 2;
		dqdqs_perbit_dly[23].best_dqdly = 2;
		/* DQ24~31 */
		dqdqs_perbit_dly[24].best_dqdly = 2;
		dqdqs_perbit_dly[25].best_dqdly = 2;
		dqdqs_perbit_dly[26].best_dqdly = 3;
		dqdqs_perbit_dly[27].best_dqdly = 3;
		dqdqs_perbit_dly[28].best_dqdly = 2;
		dqdqs_perbit_dly[29].best_dqdly = 1;
		dqdqs_perbit_dly[30].best_dqdly = 2;
		dqdqs_perbit_dly[31].best_dqdly = 2;

	}

	mcSHOW_DBG_MSG4("========================="
			"=========================\n");
	mcSHOW_DBG_MSG4("        dramc_txdqs_perbit_swcal\n");
	mcSHOW_DBG_MSG4("           channel=%d(0:cha, 1:chb)\n", p->channel);
	mcSHOW_DBG_MSG4("           bus width=%d\n", p->data_width);
	mcSHOW_DBG_MSG4("========================="
			"=========================\n");
	mcSHOW_DBG_MSG4("DQS Delay :\n DQS0 = %d DQS1 = %d DQS2 = %d DQS3 = %d\n",
			ucmax_dqsdly_byte[0], ucmax_dqsdly_byte[1],
			ucmax_dqsdly_byte[2], ucmax_dqsdly_byte[3]);
	mcSHOW_DBG_MSG4("DQM Delay :\n DQM0 = %d DQM1 = %d DQM2 = %d DQM3 = %d\n",
			ucave_dqdly_byte[0], ucave_dqdly_byte[1],
			ucave_dqdly_byte[2], ucave_dqdly_byte[3]);
	mcSHOW_DBG_MSG4("DQ Delay :\n");

	mcFPRINTF((fp_A60808, "=========================="
				"========================\n"));
	mcFPRINTF((fp_A60808, "        dramc_txdqs_perbit_swcal\n"));
	mcFPRINTF((fp_A60808, "           channel=%d(0:cha, 1:chb)\n", p->channel));
	mcFPRINTF((fp_A60808, "           bus width=%d\n", p->data_width));
	mcFPRINTF((fp_A60808, "==========================="
				"=======================\n"));
	mcFPRINTF((fp_A60808, "DQS Delay :\n DQS0 = %d DQS1 = %d DQS2 = %d DQS3 = %d\n",
		   ucmax_dqsdly_byte[0], ucmax_dqsdly_byte[1],
				ucmax_dqsdly_byte[2], ucmax_dqsdly_byte[3]));
	mcFPRINTF((fp_A60808, "DQM Delay :\n DQM0 = %d DQM1 = %d DQM2 = %d DQM3 = %d\n",
		   ucave_dqdly_byte[0], ucave_dqdly_byte[1],
				ucave_dqdly_byte[2], ucave_dqdly_byte[3]));
	mcFPRINTF((fp_A60808, "DQ Delay :\n"));

	for (ii = 0; ii < p->data_width; ii = ii+4) {
		mcSHOW_DBG_MSG4("DQ%d = %d DQ%d = %d DQ%d = %d DQ%d = %d\n",
				ii, dqdqs_perbit_dly[ii].best_dqdly,
				ii+1, dqdqs_perbit_dly[ii+1].best_dqdly,
				ii+2, dqdqs_perbit_dly[ii+2].best_dqdly,
				ii+3, dqdqs_perbit_dly[ii+3].best_dqdly);
		mcFPRINTF((fp_A60808, "DQ%2d = %2d DQ%2d = %2d "
					"DQ%2d = %2d DQ%2d = %2d\n",
					ii, dqdqs_perbit_dly[ii].best_dqdly,
					ii+1, dqdqs_perbit_dly[ii+1].best_dqdly,
					ii+2, dqdqs_perbit_dly[ii+2].best_dqdly,
					ii+3, dqdqs_perbit_dly[ii+3].best_dqdly));
	}
	mcSHOW_DBG_MSG4("____________________________________");
	mcSHOW_DBG_MSG4("____________________________________\n");
	mcFPRINTF((fp_A60808, "___________________________________"));
	mcFPRINTF((fp_A60808, "_____________________________________\n"));

	/* Add CLK to DQS/DQ skew after write leveling */
	if (fgwrlevel_done[p->channel]) {

		mcSHOW_DBG_MSG4("Add CLK to DQS/DQ skew "
				"based on write leveling.\n");
		mcFPRINTF((fp_A60808, "Add CLK to DQS/DQ skew "
					"based on write leveling.\n"));

		/* this subroutine add clk delay to DQS/DQ after WL */
		TxDelayForWriteLeveling(p, dqdqs_perbit_dly,
					ucave_dqdly_byte, ucmax_dqsdly_byte);

		mcSHOW_DBG_MSG4("\n");
		mcFPRINTF((fp_A60808, "\n"));
	}

	/* Set best delay to registers */
	/* 0x014, DQS delay, each DQS has 4 bits. */
	/* DQS0 [3:0], DQS1 [7:4], DQS2 [11:8], DQS3 [15:12] */
	u4value = 0;
	for (jj = 0; jj < (p->data_width/DQS_BIT_NUMBER); jj++) {
		u4value += (((u32)ucmax_dqsdly_byte[jj])<<(4*jj));
	}

	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_PADCTL3);
	ucstatus |= ucDram_Register_Write(addr, u4value);
	mcSHOW_DBG_MSG4("TX DQS register %#x=%#x\n",
			DRAMC_REG_PADCTL3, u4value);
	mcFPRINTF((fp_A60808, "TX DQS register %#x=%#x\n",
		   DRAMC_REG_PADCTL3, u4value));

	/* 0x200~0x20c, DQ delay, each DQ has 4 bits. */
	/* Each register contains 8-bit DQ's */
	for (ii = 0; ii < (p->data_width/DQS_BIT_NUMBER); ii++) {
		u4value = 0;
		ucbit_first = DQS_BIT_NUMBER*ii;
		ucbit_last = DQS_BIT_NUMBER*ii+DQS_BIT_NUMBER-1;
		for (jj = ucbit_first; jj <= ucbit_last; jj++) {
			u4value += (((u32)(dqdqs_perbit_dly[jj].best_dqdly))
						<< (4*(jj-ucbit_first)));
		}

		addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_DQODLY1+4*ii);
		ucstatus |= ucDram_Register_Write(addr, u4value);
		mcSHOW_DBG_MSG4("TX DQ register %#x=%#x\n",
				DRAMC_REG_DQODLY1+4*ii, u4value);
		mcFPRINTF((fp_A60808, "TX DQ register 0x%2x=0x%8x\n",
			   DRAMC_REG_DQODLY1+4*ii, u4value));
	}
	/* set DQM delay */
	{
		addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_PADCTL2);
		ucstatus |= ucDram_Register_Read(addr, &u4value);
		u4value &= 0xffff0000;
	}
	for (jj = 0; jj < (p->data_width/DQS_BIT_NUMBER); jj++) {
		u4value += (((u32)ucave_dqdly_byte[jj]) << (4*jj));
	}
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_PADCTL2);
	ucstatus |= ucDram_Register_Write(addr, u4value);
	mcSHOW_DBG_MSG4("TX DQM register %#x=%#x\n",
			DRAMC_REG_PADCTL2, u4value);
	mcFPRINTF((fp_A60808, "TX DQM register 0x%2x=0x%8x\n",
		   DRAMC_REG_PADCTL2, u4value));

	DramcPhyReset(p);

	/* Log Example */
	/*
	 * DQS Delay :
	 * DQS0 = 1 DQS1 = 2 DQS2 = 0 DQS3 = 1
	 * DQ Delay :
	 * DQ0 = 0 DQ1 = 2 DQ2 = 1 DQ3 = 0
	 * DQ4 = 2 DQ5 = 2 DQ6 = 3 DQ7 = 1
	 * DQ8 = 1 DQ9 = 0 DQ10 = 2 DQ11 = 0
	 * DQ12 = 2 DQ13 = 0 DQ14 = 6 DQ15 = 4
	 * DQ16 = 1 DQ17 = 1 DQ18 = 0 DQ19 = 1
	 * DQ20 = 1 DQ21 = 2 DQ22 = 1 DQ23 = 0
	 * DQ24 = 1 DQ25 = 0 DQ26 = 0 DQ27 = 1
	 * DQ28 = 3 DQ29 = 0 DQ30 = 5 DQ31 = 2
	 */

	if (ucfail == 1) {
		return DRAM_FAIL;
	}

	return DRAM_OK;
}
