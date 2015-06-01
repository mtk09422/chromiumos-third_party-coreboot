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

#include <delay.h>
#include <uart.h>
#include <console/console.h>
#include <string.h>
#include <soc/dramc_common.h>
#include <soc/dramc_register.h>
#include <soc/dramc_pi_api.h>
#include <soc/pll.h>
#include <soc/emi.h>
#include <arch/barrier.h>

/* dramc register define */
const dramc_ao_regs *ao_regs;
const dramc_nao_regs *nao_regs;
const dramc_ddrphy_regs *ddrphy_regs;

/* def DUAL_RANKS */
unsigned int uiDualRank = 0;

u8 VCOK_Cha_Mempll2, VCOK_Cha_Mempll3, VCOK_Cha_Mempll4;
u8 VCOK_Chb_Mempll2, VCOK_Chb_Mempll3, VCOK_Chb_Mempll4;
u8 VCOK_05PHY_Mempll2, VCOK_05PHY_Mempll3, VCOK_05PHY_Mempll4;

/*
 * Global variables
 */
extern const u32 uiLPDDR_PHY_Mapping_POP_CHA[32];
extern const u32 uiLPDDR_PHY_Mapping_POP_CHB[32];
extern u8 opt_gw_coarse_value_R0[2], opt_gw_fine_value_R0[2];
extern u8 opt_gw_coarse_value_R1[2], opt_gw_fine_value_R1[2];

/* ------------------------------------------------------------------------- */
/*  ucDramC_Register_Read
 *  DRAMC register read (32-bit).
 *  @param  u4reg_addr    register address in 32-bit.
 *  @param  pu4reg_value  Pointer of register read value.
 *  @retval 0: OK, 1: FAIL
 * ------------------------------------------------------------------------- */

/* This function need to be porting by BU requirement */
inline u8 ucDramC_Register_Read(u32 u4reg_addr, u32 *pu4reg_value)
{
	uintptr_t cha_dst_addr, chb_dst_addr;

	u32 u4Channel = u4reg_addr >> CH_INFO;

	u4reg_addr &= 0x3fffffff;

	if (u4Channel == CHANNEL_A) {
		/* Channel A destination address */
		cha_dst_addr = CHA_DRAMCAO_BASE + u4reg_addr;
		*pu4reg_value =	(*(volatile unsigned int *)(cha_dst_addr));
	} else {
		/* Channel B destination address */
		chb_dst_addr = CHB_DRAMCAO_BASE + u4reg_addr;
		*pu4reg_value =	(*(volatile unsigned int *)(chb_dst_addr));
	}

	return 0;
}

/* ------------------------------------------------------------------------- */
/*  ucDramC_Register_Write
 *  DRAMC register write (32-bit).
 *  @param  u4reg_addr    register address in 32-bit.
 *  @param  u4reg_value   register write value.
 *  @retval 0: OK, 1: FAIL
 * ------------------------------------------------------------------------- */

/* This function need to be porting by BU requirement */
inline u8 ucDramC_Register_Write(u32 u4reg_addr, u32 u4reg_value)
{
	uintptr_t cha_dst_addr, chb_dst_addr;

	u32 u4Channel = u4reg_addr >> CH_INFO;

	u4reg_addr &= 0x3fffffff;

	if (u4Channel == CHANNEL_A) {
		/* Channel A destination address */
		cha_dst_addr = CHA_DRAMCAO_BASE + u4reg_addr;
		(*(volatile unsigned int *)(cha_dst_addr)) = u4reg_value;
	} else {
		/* Channel B destination address */
		chb_dst_addr = CHB_DRAMCAO_BASE + u4reg_addr;
		(*(volatile unsigned int *)(chb_dst_addr)) = u4reg_value;
	}
	dsb();

	return 0;
}

/* ------------------------------------------------------------------------- */
/*  ucDram_Register_Read
 *  DRAM register read (32-bit).
 *  @param  u4reg_addr    register address in 32-bit.
 *  @param  pu4reg_value  Pointer of register read value.
 *  @retval 0: OK, 1: FAIL
 * ------------------------------------------------------------------------- */

/* This function need to be porting by BU requirement */
inline u8 ucDram_Register_Read(u32 u4reg_addr, u32 *pu4reg_value)
{
	uintptr_t ao_dst_addr, phy_dst_addr, nao_dst_addr;

	u32 u4Channel = u4reg_addr >> CH_INFO;

	u4reg_addr &= 0x3fffffff;

	if (u4Channel == CHANNEL_A) {
		/* Channel A destination address */
		ao_dst_addr = CHA_DRAMCAO_BASE + u4reg_addr;
		phy_dst_addr = CHA_DDRPHY_BASE + u4reg_addr;
		nao_dst_addr = CHA_DRAMCNAO_BASE + u4reg_addr;
	} else {
		/* Channel B destination address */
		ao_dst_addr = CHB_DRAMCAO_BASE + u4reg_addr;
		phy_dst_addr = CHB_DDRPHY_BASE  + u4reg_addr;
		nao_dst_addr = CHB_DRAMCNAO_BASE + u4reg_addr;
	}

	*pu4reg_value =	(*(volatile unsigned int *)(ao_dst_addr)) |
			(*(volatile unsigned int *)(phy_dst_addr))|
			(*(volatile unsigned int *)(nao_dst_addr));

	return 0;
}

/* ------------------------------------------------------------------------- */
/*  ucDram_Register_Write
 *  DRAM register write (32-bit).
 *  @param  u4reg_addr    register address in 32-bit.
 *  @param  u4reg_value   register write value.
 *  @retval 0: OK, 1: FAIL
 * ------------------------------------------------------------------------- */

/* This function need to be porting by BU requirement */
inline u8 ucDram_Register_Write(u32 u4reg_addr, u32 u4reg_value)
{
	uintptr_t ao_dst_addr, phy_dst_addr, nao_dst_addr;

	u32 u4Channel = u4reg_addr >> CH_INFO;

	u4reg_addr &= 0x3fffffff;

	if (u4Channel == CHANNEL_A) {
		/* Channel A destination address */
		ao_dst_addr = CHA_DRAMCAO_BASE + u4reg_addr;
		phy_dst_addr = CHA_DDRPHY_BASE + u4reg_addr;
		nao_dst_addr = CHA_DRAMCNAO_BASE + u4reg_addr;
	} else {
		/* Channel B destination address */
		ao_dst_addr = CHB_DRAMCAO_BASE + u4reg_addr;
		phy_dst_addr = CHB_DDRPHY_BASE  + u4reg_addr;
		nao_dst_addr = CHB_DRAMCNAO_BASE + u4reg_addr;
	}

	(*(volatile unsigned int *)(ao_dst_addr)) = u4reg_value;
	(*(volatile unsigned int *)(phy_dst_addr)) = u4reg_value;
	(*(volatile unsigned int *)(nao_dst_addr)) = u4reg_value;

	dsb();

	return 0;
}

u32 DramcGetMR2ByFreq(int dram_freq)
{
	u32 value;

	switch (dram_freq) {
	case 533:   /* 1160MHz */
		value = 0x00170002; /* RL9 WL5 */
		break;
	case 666:   /* 1333MHz */
		value = 0x00180002; /* RL10 WL6 */
		break;
	case 800:   /* 1600MHz */
		value = 0x001A0002; /* RL12 WL6 */
		break;
	case 933:   /* 1866MHz */
		value = 0x001C0002; /* RL14 WL8 */
		break;
	default:
		value = 0x001C0002;
		break;
	}

	return value;
}

inline u8 dramc_write32(DRAMC_CTX_T *p, u32 addr, u32 val)
{
	u8 ucstatus = 0;

	addr = mcSET_DRAMC_REG_ADDR(p, addr);
	ucstatus |= ucDram_Register_Write(addr, val);

	return ucstatus;
}

inline u8 dramc_setfield(DRAMC_CTX_T *p, u32 addr, u32 value, u32 mask, u32 pos)
{
	u8  ucstatus = 0;
	u32 u4value;

	addr = mcSET_DRAMC_REG_ADDR(p, addr);
	ucstatus |= ucDram_Register_Read(addr, &u4value);
	mcSET_FIELD(&u4value, value, mask, pos);
	ucstatus |= ucDram_Register_Write(addr, u4value);

	return ucstatus;
}

inline u8 dramc_setbit(DRAMC_CTX_T *p, u32 addr, u32 bit)
{
	u8  ucstatus = 0;
	u32 u4value;

	addr = mcSET_DRAMC_REG_ADDR(p, addr);
	ucstatus |= ucDram_Register_Read(addr, &u4value);
	mcSET_BIT(&u4value, bit);
	ucstatus |= ucDram_Register_Write(addr, u4value);

	return ucstatus;
}

inline u8 dramc_clrbit(DRAMC_CTX_T *p, u32 addr, u32 bit)
{
	u8 ucstatus = 0;
	u32 u4value;

	addr = mcSET_DRAMC_REG_ADDR(p, addr);
	ucstatus |= ucDram_Register_Read(addr, &u4value);
	mcCLR_BIT(&u4value, bit);
	ucstatus |= ucDram_Register_Write(addr, u4value);

	return ucstatus;
}

static inline u8 sel_ph_and_write_latency_setting(DRAMC_CTX_T *p)
{
	u8 ucstatus = 0;

	ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->lpddr2_3, 0x220fffff);
	ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->lpddr2_4, 0x220fffff);

	ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->selph12, 0x10f010f0);
	ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->selph13, 0xffffffff);
	ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->selph14, 0xffffffff);
	ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->selph15, 0x0000001f);

	return ucstatus;
}

static inline u8 MEMPLL_EN(DRAMC_CTX_T *p, u32 TYPE, u32 bit)
{
	u8 ucstatus = 0;

	/* Enable Reg.*/
	if (TYPE == ENABLE) {
		/* MEMPLL2_EN */
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->mempll[5], bit);
		/* MEMPLL4_EN */
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->mempll[11], bit);
		/* MEMPLL05_2_EN */
		ucstatus |= dramc_setbit(p, 0x664, bit);
		/* MEMPLL05_3_EN */
		ucstatus |= dramc_setbit(p, 0x670, bit);
		/* MEMPLL3_EN */
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->mempll[8], bit);
		/* MEMPLL05_4_EN */
		ucstatus |= dramc_setbit(p, 0x67c, bit);
	} else { /* Disable Reg. */
		/* MEMPLL2_EN */
		ucstatus |= dramc_clrbit(p, (uintptr_t)&ddrphy_regs->mempll[5], bit);
		/* MEMPLL4_EN */
		ucstatus |= dramc_clrbit(p, (uintptr_t)&ddrphy_regs->mempll[11], bit);
		/* MEMPLL05_2_EN */
		ucstatus |= dramc_clrbit(p, 0x664, bit);
		/* MEMPLL05_3_EN */
		ucstatus |= dramc_clrbit(p, 0x670, bit);
		/* MEMPLL3_EN */
		ucstatus |= dramc_clrbit(p, (uintptr_t)&ddrphy_regs->mempll[8], bit);
		/* MEMPLL05_4_EN */
		ucstatus |= dramc_clrbit(p, 0x67c, bit);
	}

	return ucstatus;
}

static inline u8 MEMPLL_FB_MCK_SEL(DRAMC_CTX_T *p, u32 TYPE, u32 bit)
{
	u8 ucstatus = 0;

	if (TYPE == ENABLE) {
		/* MEMPLL2_FB_MCK_SEL */
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->mempll[6], bit);
		/* MEMPLL4_FB_MCK_SEL */
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->mempll[12], bit);
		/* MEMPLL05_2_FB_MCK_SEL */
		ucstatus |= dramc_setbit(p, 0x668, bit);
		/* MEMPLL05_3_FB_MCK_SEL */
		ucstatus |= dramc_setbit(p, 0x674, bit);
		/* MEMPLL3_FB_MCK_SEL */
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->mempll[9], bit);
		/* MEMPLL05_4_FB_MCK_SEL */
		ucstatus |= dramc_setbit(p, 0x680, bit);
	}

	return ucstatus;
}

/* ------------------------------------------------------------------------- */
/*  MemPllInit
 *  MEMPLL Initialization.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 * ------------------------------------------------------------------------- */

DRAM_STATUS_T MemPllPreInit(DRAMC_CTX_T *p)
{
	u8 ucstatus = 0;

	/* error handling */
	if (!p) {
		return DRAM_FAIL;
	}

	/* PLL: LEW_SIMULATION */
	p->channel = CHANNEL_A;
	ucstatus |= sel_ph_and_write_latency_setting(p);

	p->channel = CHANNEL_B;
	ucstatus |= sel_ph_and_write_latency_setting(p);

	/* SPM_MODIFY */
	p->channel = CHANNEL_A;
	ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->peri[2], 0x1111ff11);
	ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->peri[3], 0x11511111);

	p->channel = CHANNEL_B;
	ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->peri[2], 0x1111ff11);
	ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->peri[3], 0x11511111);

	if ((p->package == PACKAGE_POP) || (p->package == PACKAGE_SBS)) {
		/* =============== Integration Part (TOP) =================== */
		/* Enable MTCMOS power ack switch before ACK_POP_BYP and */
		/* ACK_SBS_BYP to avoid glitch */
		/* LDO_PHY2POP_NDIS=1 is better from LDO designer */
		/* Default: 0x09000000, [15]ACK_SWITCH, [2]MEMCLKENSYNC_SOURCE */
		/* 0x1000F640[16] & 0x10012640[16] SbS CMDPHYMTCMOS}. */
		/* Set to 1 to bypass. */
		p->channel = CHANNEL_A;
		/* SPM_MODIFY */
		/* PLL: LEW_SIMULATION */
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll_divider, 0x09008023);

		/* PLL: LEW_SIMULATION */
		/* SPM control clock enable */
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll05_divider, 0x09008004);

		p->channel = CHANNEL_B;
		/* SPM_MODIFY */
		/* PLL: LEW_SIMULATION */
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll_divider, 0x09008023);

		/* PLL: LEW_SIMULATION */
		/* SPM control clock enable */
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll05_divider, 0x09008004);

		/* ALLCLK_EN */
		p->channel = CHANNEL_A;
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->mempll_divider, 4);

		p->channel = CHANNEL_B;
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->mempll_divider, 4);

		/* Disable gating function */
		p->channel = CHANNEL_A;
		ucstatus |= dramc_clrbit(p, (uintptr_t)&ddrphy_regs->ddrphy_cg_ctrl, 2);
		ucstatus |= dramc_clrbit(p, (uintptr_t)&ddrphy_regs->ddrphy_cg_ctrl, 1);

		p->channel = CHANNEL_B;
		ucstatus |= dramc_clrbit(p, (uintptr_t)&ddrphy_regs->ddrphy_cg_ctrl, 2);
		ucstatus |= dramc_clrbit(p, (uintptr_t)&ddrphy_regs->ddrphy_cg_ctrl, 1);

		/* PLL: LEW_SIMULATION */
		/* SPM control clock enable */
		p->channel = CHANNEL_A;
		ucstatus |= dramc_clrbit(p, (uintptr_t)&ddrphy_regs->mempll_divider, 15);
		ucstatus |= dramc_clrbit(p, (uintptr_t)&ddrphy_regs->mempll_divider, 0);

		/* PLL: LEW_SIMULATION */
		/* SPM control clock enable */
		p->channel = CHANNEL_B;
		ucstatus |= dramc_clrbit(p, (uintptr_t)&ddrphy_regs->mempll_divider, 15);
		ucstatus |= dramc_clrbit(p, (uintptr_t)&ddrphy_regs->mempll_divider, 0);

		/* DRAMC 2X mode */
		/* [0]FDIV2=1 */
		p->channel = CHANNEL_A;
		ucstatus |= dramc_setbit(p, (uintptr_t)&ao_regs->ddr2ctl, 0);

		p->channel = CHANNEL_B;
		ucstatus |= dramc_setbit(p, (uintptr_t)&ao_regs->ddr2ctl, 0);

		/* Delay counters for SPM power-up MEMPLL */
		p->channel = CHANNEL_A;
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->peri[0], 0x21271b03);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->peri[1], 0x5096061e);

		p->channel = CHANNEL_B;
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->peri[0], 0x21271b03);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->peri[1], 0x5096061e);

		/* [27]LDO_PHY2POP_NDIS, [24]LDO_PHY2POP_EN, */
		/* [7]MEMCLKENMODE, [3]MEMCLKEN_SEL, [0]MEMCLKENB_SEL */
		p->channel = CHANNEL_A;
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->mempll_divider, 7);
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->mempll_divider, 24);
		ucstatus |= dramc_clrbit(p, (uintptr_t)&ddrphy_regs->mempll_divider, 3);
		/* must clear to select external source to get */
		/* correct phase for channel A */

		p->channel = CHANNEL_B;
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->mempll_divider, 7);
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->mempll_divider, 24);
	}
	return ucstatus;
}

DRAM_STATUS_T MemPllInit(DRAMC_CTX_T *p)
{
	u8 ucstatus = 0;
	u32 u4value;
	u32 reg_addr;

	/* MPLL frequency-dependent parameters */
	u32 MPLL_POSDIV_2_0, MPLL_SDM_PCW_20_0;

	/* MEMPLL frequency-dependent parameters */
	u32 MEMPLL_FBDIV_6_0, MEMPLL_M4PDIV_1_0;
	u32 MEMPLL_BR_1_0, MEMPLL_BC_1_0, MEMPLL_IR_3_0;
	u32 MEMPLL_IC_3_0, MEMPLL_BP_3_0;

	/* error handling */
	if (!p)
		return DRAM_FAIL;

	/* frequency-dependent parameters */
	if (p->frequency == 533) { /* 1066Mbps */

		/*
		 * change to (1163/2) MHz for simulation model pass,
		 * only update here for simplicity
		 */

		/* MPLL */
		MPLL_POSDIV_2_0 = 0x00000000<<4;
		MPLL_SDM_PCW_20_0 = 0x000FA7E0<<0;
		/* MEMPLL */
		MEMPLL_FBDIV_6_0 = 0x00000004<<16;
		MEMPLL_BR_1_0 = 0x00000002<<10;
		MEMPLL_BC_1_0 = 0x00000000<<8;
		MEMPLL_IR_3_0 = 0x0000000d<<28;
		MEMPLL_IC_3_0 = 0x00000003<<8;
		MEMPLL_BP_3_0 = 0x00000002<<12;
		MEMPLL_M4PDIV_1_0 = 0x00000000<<28;
	} else if (p->frequency == 800) {
		/* 1600Mbps */
		/* MPLL */
		MPLL_POSDIV_2_0 = 0x00000000<<4;
		MPLL_SDM_PCW_20_0 = 0x000F6276<<0;
		/* MEMPLL */
		MEMPLL_FBDIV_6_0 = 0x00000006<<16;
		MEMPLL_BR_1_0 = 0x00000001<<10;
		MEMPLL_BC_1_0 = 0x00000001<<8;
		MEMPLL_IR_3_0 = 0x0000000a<<28;
		MEMPLL_IC_3_0 = 0x00000007<<8;
		MEMPLL_BP_3_0 = 0x00000001<<12;
		MEMPLL_M4PDIV_1_0 = 0x00000000<<28;
	} else if (p->frequency == 896) {
		/* 1792Mbps */
		/* MPLL */
		MPLL_POSDIV_2_0 = 0x00000000<<4;
		MPLL_SDM_PCW_20_0 = 0x000F13B1<<0;
		/* MEMPLL */
		MEMPLL_FBDIV_6_0 = 0x00000007<<16;
		MEMPLL_BR_1_0 = 0x00000001<<10;
		MEMPLL_BC_1_0 = 0x00000000<<8;
		MEMPLL_IR_3_0 = 0x0000000c<<28;
		MEMPLL_IC_3_0 = 0x00000006<<8;
		MEMPLL_BP_3_0 = 0x00000001<<12;
		MEMPLL_M4PDIV_1_0 = 0x00000000<<28;
	} else {
		/* highest data rate supported, 1780Mbps */
		/* MPLL */
		MPLL_POSDIV_2_0 = 0x00000000<<4;
		MPLL_SDM_PCW_20_0 = 0x000EF9D8<<0;
		/* MEMPLL */
		MEMPLL_FBDIV_6_0 = 0x00000007<<16;
		MEMPLL_BR_1_0 = 0x00000001<<10;
		MEMPLL_BC_1_0 = 0x00000000<<8;
		MEMPLL_IR_3_0 = 0x0000000c<<28;
		MEMPLL_IC_3_0 = 0x00000006<<8;
		MEMPLL_BP_3_0 = 0x00000001<<12;
		MEMPLL_M4PDIV_1_0 = 0x00000000<<28;
	}

	/* RG_MPLL_EN=0 : 0x10209280[0] (disable MPLL first) */
	u4value = (*(volatile unsigned int *)(0x10209280));
	mcCLR_BIT(&u4value, 0);
	(*(volatile unsigned int *)(0x10209280)) = u4value;

	/* MPLL config */
	/* RG_MPLL_MODE_SEL: 0x10209280[16]=1 (from MPLL or XTAL) */
	/* RG_MPLL_POSDIV[2:0]: 0x10209280[6:4] */
	u4value = (*(volatile unsigned int *)(0x10209280));
	mcSET_BIT(&u4value, 16);
	u4value = u4value & 0xffffff8f;
	u4value = u4value | MPLL_POSDIV_2_0;
	(*(volatile unsigned int *)(0x10209280)) = u4value;

	/* RG_PLLGP_RESERVE[15]: 0x10209040[31]=0 */
	u4value = (*(volatile unsigned int *)(0x10209040));
	mcCLR_BIT(&u4value, 31);
	(*(volatile unsigned int *)(0x10209040)) = u4value;

	/*
	 * RG_MPLL_SDM_PCW[30:10]:
	 * 0x10209284[20:0] (only 21 bits to registers)
	 */
	u4value = (*(volatile unsigned int *)(0x10209284));
	u4value = u4value & 0xffe00000;
	u4value = u4value | MPLL_SDM_PCW_20_0;
	(*(volatile unsigned int *)(0x10209284)) = u4value;

	if ((p->package == PACKAGE_POP) || (p->package == PACKAGE_SBS)) {

		/* == MEMPLL IP Part == */
		/* mempll config: */
		/* set to seal ring */
		/* [29]RG_MEMPLL_REFCK_MODE_SEL -> 1: seal-ring, 0: cts */
		/* [28]RG_MEMPLL_REFCK_SEL=0 */
		/* [0]RG_MEMPLL_REFCK_EN=0 (only 0x688) */
		p->channel = CHANNEL_A;
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[14], 0x00000000);
		ucstatus |= dramc_write32(p, 0x688, 0x00000000);

		p->channel = CHANNEL_B;
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[14], 0x00000000);
		ucstatus |= dramc_write32(p, 0x688, 0x00000000);

		/* MEMPLL_BIAS_EN=0 : 0x60c[6]/0x65c[6] */
		/* MEMPLL_BIAS_LPF_EN=0 : 0x60c[7]/0x65c[7] */
		p->channel = CHANNEL_A;
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[3], 0xd0000000);
		ucstatus |= dramc_write32(p, 0x65c, 0xd0000000);

		p->channel = CHANNEL_B;
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[3], 0xd0000000);
		ucstatus |= dramc_write32(p, 0x65c, 0xd0000000);

		/*
		 * RG_MEMPLL_REFCK_BYPASS (RG_MEMPLL_RESERVE[3]) = 0
		 * only for Rome. Bypass MEMPLL1 to reduce Conduit reference
		 * clock skew (HW default value) 0x610[27]=0, 0x660[27]=0
		 */

		/* mempll 2 config */

		/* three patterns used below */
		u32 pattern1 = MEMPLL_IR_3_0 | MEMPLL_FBDIV_6_0 | MEMPLL_IC_3_0;
		u32 pattern2 = MEMPLL_M4PDIV_1_0;
		u32 pattern3 = MEMPLL_BP_3_0 | MEMPLL_BR_1_0 | MEMPLL_BC_1_0;

		/* MEMPLL2_AUTOK_EN=1, MEMPLL2_AUTOK_LOAD=1 */
		p->channel = CHANNEL_A;
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[5], 0x07800000 | pattern1);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[6], 0x4c00c000 | pattern2);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[7], 0x00020001 | pattern3);

		/* mempll 4 config */
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[11], 0x07800000 | pattern1);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[12], 0x4c00c000 | pattern2);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[13], 0x00000001 | pattern3);

		/* mempll05 2  config */
		ucstatus |= dramc_write32(p, 0x664, 0x07800000 | pattern1);
		ucstatus |= dramc_write32(p, 0x668, 0x4c00c000 | pattern2);
		ucstatus |= dramc_write32(p, 0x66c, 0x00020001 | pattern3);

		/* mempll05 3 config */
		ucstatus |= dramc_write32(p, 0x670, 0x07800000 | pattern1);
		ucstatus |= dramc_write32(p, 0x674, 0x4c00c000 | pattern2);
		ucstatus |= dramc_write32(p, 0x678, 0x00000001 | pattern3);

		/* mempll 3 */
		/* (Enable signal tie together.*/
		/*  Sim error due to unknown dividor. E2 will fix.) */
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[8], 0x07800000 | pattern1);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[9], 0x4c00c000 | pattern2);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[10], 0x00020001 | pattern3);

		/* mempll05 4 */
		/* (Enable signal tie together. */
		/*  Sim error due to unknown dividor. E2 will fix.) */
		ucstatus |= dramc_write32(p, 0x67c, 0x07800000 | pattern1);
		ucstatus |= dramc_write32(p, 0x680, 0x4c00c000 | pattern2);
		ucstatus |= dramc_write32(p, 0x684, 0x00020001 | pattern3);

		p->channel = CHANNEL_B;
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[5], 0x07800000 | pattern1);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[6], 0x4c00c000 | pattern2);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[7], 0x00020001 | pattern3);

		/* mempll 4 config */
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[11], 0x07800000 | pattern1);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[12], 0x4c00c000 | pattern2);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[13], 0x00000001 | pattern3);

		/* mempll05 2  config */
		ucstatus |= dramc_write32(p, 0x664, 0x07800000 | pattern1);
		ucstatus |= dramc_write32(p, 0x668, 0x4c00c000 | pattern2);
		ucstatus |= dramc_write32(p, 0x66c, 0x00020001 | pattern3);

		/* mempll05 3 config */
		ucstatus |= dramc_write32(p, 0x670, 0x07800000 | pattern1);
		ucstatus |= dramc_write32(p, 0x674, 0x4c00c000 | pattern2);
		ucstatus |= dramc_write32(p, 0x678, 0x00000001 | pattern3);

		/* mempll 3 */
		/* (Enable signal tie together. */
		/*  Sim error due to unknown dividor. E2 will fix.) */
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[8], 0x07800000 | pattern1);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[9], 0x4c00c000 | pattern2);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[10], 0x00020001 | pattern3);

		/* mempll05 4 */
		/* (Enable signal tie together. */
		/* Sim error due to unknown dividor. E2 will fix.) */
		ucstatus |= dramc_write32(p, 0x67c, 0x07800000 | pattern1);
		ucstatus |= dramc_write32(p, 0x680, 0x4c00c000 | pattern2);
		ucstatus |= dramc_write32(p, 0x684, 0x00020001 | pattern3);

		/* wait 100ns (after DA_MPLL_SDM_ISO_EN goes LOW) */
		udelay(1);

		/* Power up sequence starts here... */
		/* MPLL_EN=1 */
		/* RG_MPLL_EN=1 : 0x10209280[0] */
		u4value = (*(volatile unsigned int *)(0x10209280));
		mcSET_BIT(&u4value, 0);
		(*(volatile unsigned int *)(0x10209280)) = u4value;

		/* MEMPLL_REFCK_EN=1 : 0x688[0] */
		/* (only one in chip, seal-ring buffer enable) */
		p->channel = CHANNEL_A;
		ucstatus |= dramc_clrbit(p, 0x688, 0);

		p->channel = CHANNEL_B;
		ucstatus |= dramc_clrbit(p, 0x688, 0);

		/* wait 100us */
		udelay(100);

		p->channel = CHANNEL_A;
		/* MEMPLL_BIAS_EN=1 : 0x60c[6] */
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[3], 0xd0000040);
		/* MEMPLL_BIAS05_EN = 1 : 0x65c[6] */
		ucstatus |= dramc_write32(p, 0x65c, 0xd0000040);

		p->channel = CHANNEL_B;
		/* MEMPLL_BIAS_EN=1 : 0x60c[6] */
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll[3], 0xd0000040);
		/* MEMPLL_BIAS05_EN = 1 : 0x65c[6] */
		ucstatus |= dramc_write32(p, 0x65c, 0xd0000040);

		/* wait 2us */
		udelay(2);

		/* MEMPLL*_EN=1 */
		p->channel = CHANNEL_A;
		/* MEMPLL_EN */
		ucstatus |= MEMPLL_EN(p, ENABLE, 0);

		p->channel = CHANNEL_B;
		ucstatus |= MEMPLL_EN(p, ENABLE, 0);

		/* wait 100us */
		udelay(100);

		p->channel = CHANNEL_A;
		/* MEMPLL_BIAS_LPF_EN=1 : 0x60c[7] */
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->mempll[3], 7);
		/* MEMPLL05_BIAS_LPF_EN=1 : 0x65c[7] */
		ucstatus |= dramc_setbit(p, 0x65c, 7);

		p->channel = CHANNEL_B;
		/* MEMPLL_BIAS_LPF_EN=1 : 0x60c[7] */
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->mempll[3], 7);
		/* MEMPLL05_BIAS_LPF_EN=1 : 0x65c[7] */
		ucstatus |= dramc_setbit(p, 0x65c, 7);

		/* wait 20us or 30us */
		udelay(30);

		/* check VCO K status and band */
		p->channel = CHANNEL_A;
		mcSHOW_DBG_MSG3("MEMPLL AUTOK status log...");
		mcSHOW_DBG_MSG3("channel=%d\n", p->channel);
		mcSHOW_DBG_MSG3("0x3e0 (MEMPLL 2/3/4 status), ");
		mcSHOW_DBG_MSG3("0x3e4 (MEMPLL05 2/3/4 status)\n");
		mcSHOW_DBG_MSG3("[29]/[27]/[25] : MEMPLL 2/3/4_AUTOK_PASS\n");
		mcSHOW_DBG_MSG3("[22:16]/[14:8] : MEMPLL 2/3_AUTOK_BAND\n");

		reg_addr = mcSET_DRAMC_REG_ADDR(p, 0x3e0);
		ucstatus |= ucDram_Register_Read(reg_addr, &u4value);
		VCOK_Cha_Mempll2 = (u4value >> 16) & 0x7f;
		VCOK_Cha_Mempll3 = (u4value >> 8) & 0x7f;
		mcSHOW_DBG_MSG3("0x3e0=%#x, ", u4value);
		mcSHOW_DBG_MSG3("VCOK_Cha_Mempll2=%#x, ", VCOK_Cha_Mempll2);
		mcSHOW_DBG_MSG3("VCOK_Cha_Mempll3=%#x\n", VCOK_Cha_Mempll3);

		reg_addr = mcSET_DRAMC_REG_ADDR(p, 0x3e4);
		ucstatus |= ucDram_Register_Read(reg_addr, &u4value);
		VCOK_05PHY_Mempll2 = (u4value >> 16) & 0x7f;
		VCOK_05PHY_Mempll3 = (u4value >> 8) & 0x7f;
		mcSHOW_DBG_MSG3("0x3e4=%#x, ", u4value);
		mcSHOW_DBG_MSG3("VCOK_05PHY_Mempll2=%#x, ",
				VCOK_05PHY_Mempll2);
		mcSHOW_DBG_MSG3("VCOK_05PHY_Mempll3=%#x\n",
				VCOK_05PHY_Mempll3);

		/* RG_MEMPLL_RESERVE[2]=1, to select MEMPLL4 band register */
		/* RGS_MEMPLL4_AUTOK_BAND[6:0]= RGS_MEMPLL4_AUTOK_BAND[6] + */
		/* RGS_MEMPLL3_AUTOK_BAND[5:0] */
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->mempll[4], 26);

		mcSHOW_DBG_MSG3("[6]+[13:8] : MEMPLL 4_AUTOK_BAND\n");

		reg_addr = mcSET_DRAMC_REG_ADDR(p, 0x3e0);
		ucstatus |= ucDram_Register_Read(reg_addr, &u4value);
		VCOK_Cha_Mempll4 = ((u4value >> 8) & 0x3f) | (u4value & 0x40);
		mcSHOW_DBG_MSG3("0x3e0=%#x, ", u4value);
		mcSHOW_DBG_MSG3("VCOK_Cha_Mempll4=%#x\n", VCOK_Cha_Mempll4);

		reg_addr = mcSET_DRAMC_REG_ADDR(p, 0x3e4);
		ucstatus |= ucDram_Register_Read(reg_addr, &u4value);
		VCOK_05PHY_Mempll4 = ((u4value >> 8) & 0x3f) | (u4value & 0x40);
		mcSHOW_DBG_MSG3("0x3e4=%#x, ", u4value);
		mcSHOW_DBG_MSG3("VCOK_05PHY_Mempll4=%#x\n",
				VCOK_05PHY_Mempll4);

		/* RG_MEMPLL_RESERVE[2]=0, recover back */
		ucstatus |= dramc_clrbit(p, (uintptr_t)&ddrphy_regs->mempll[4], 26);

		p->channel = CHANNEL_B;
		mcSHOW_DBG_MSG3("MEMPLL AUTOK status log...");
		mcSHOW_DBG_MSG3("channel=%d\n", p->channel);
		mcSHOW_DBG_MSG3("0x3e0 (MEMPLL 2/3/4 status), ");
		mcSHOW_DBG_MSG3("0x3e4 (MEMPLL05 2/3/4 status)\n");
		mcSHOW_DBG_MSG3("[29]/[27]/[25] : MEMPLL 2/3/4_AUTOK_PASS\n");
		mcSHOW_DBG_MSG3("[14:8]/[6:0] : MEMPLL 3/4_AUTOK_BAND\n");

		reg_addr = mcSET_DRAMC_REG_ADDR(p, 0x3e0);
		ucstatus |= ucDram_Register_Read(reg_addr, &u4value);
		VCOK_Chb_Mempll3 = (u4value >> 8) & 0x7f;
		VCOK_Chb_Mempll4 = u4value & 0x7f;
		mcSHOW_DBG_MSG3("0x3e0=%#x, ", u4value);
		mcSHOW_DBG_MSG3("VCOK_Chb_Mempll3=%#x, ", VCOK_Chb_Mempll3);
		mcSHOW_DBG_MSG3("VCOK_Chb_Mempll4=%#x\n", VCOK_Chb_Mempll4);

		/* RG_MEMPLL_RESERVE[2]=1, to select MEMPLL4 band register */
		/* RGS_MEMPLL4_AUTOK_BAND[6:0]= RGS_MEMPLL4_AUTOK_BAND[6] */
		/*                              +RGS_MEMPLL3_AUTOK_BAND[5:0] */
		/* channel B mempll2 <-> mempll4 (HW and register have swap) */
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->mempll[4], 26);

		mcSHOW_DBG_MSG3("[6]+[13:8] : MEMPLL 2_AUTOK_BAND\n");

		reg_addr = mcSET_DRAMC_REG_ADDR(p, 0x3e0);
		ucstatus |= ucDram_Register_Read(reg_addr, &u4value);
		VCOK_Chb_Mempll2 = ((u4value >> 8) & 0x3f) | (u4value & 0x40);
		mcSHOW_DBG_MSG3("0x3e0=%#x, ", u4value);
		mcSHOW_DBG_MSG3("VCOK_Chb_Mempll2=%#x\n", VCOK_Chb_Mempll2);

		/* RG_MEMPLL_RESERVE[2]=0, recover back */
		ucstatus |= dramc_clrbit(p, (uintptr_t)&ddrphy_regs->mempll[4], 26);

		/* PLL ready */

		/* MEMPLL*_EN=0 */
		p->channel = CHANNEL_A;
		ucstatus |= MEMPLL_EN(p, DISABLE, 0);

		p->channel = CHANNEL_B;
		ucstatus |= MEMPLL_EN(p, DISABLE, 0);

		/* MEMPLL*_AUTOK_EN=0 */
		p->channel = CHANNEL_A;
		ucstatus |= MEMPLL_EN(p, DISABLE, 23);

		p->channel = CHANNEL_B;
		ucstatus |= MEMPLL_EN(p, DISABLE, 23);

		/* wait 1us */
		udelay(1);

		/* MEMPLL*_FB_MCK_SEL=1 (switch to outer loop) */
		p->channel = CHANNEL_A;
		ucstatus |= MEMPLL_FB_MCK_SEL(p, ENABLE, 25);

		p->channel = CHANNEL_B;
		ucstatus |= MEMPLL_FB_MCK_SEL(p, ENABLE, 25);

		/* wait 100ns */
		udelay(1);

		/* MEMPLL*_EN=1 */
		p->channel = CHANNEL_A;
		ucstatus |= MEMPLL_EN(p, ENABLE, 0);

		p->channel = CHANNEL_B;
		ucstatus |= MEMPLL_EN(p, ENABLE, 0);

		/* MEMPLL_NEW_POWERON defined */
		*(volatile unsigned int *)(0x10006000) = 0x0b160001;
		/* (4) 0x10006010[27]=1 Request MEMPLL reset/pdn mode */
		*(volatile unsigned int *)(0x10006010) |= 0x08000000;
		udelay(2);
		/* (1) 0x10006010[27]=0 */
		/* Unrequest MEMPLL reset/pdn mode and wait settle */
		/* (1us for reset) */
		*(volatile unsigned int *)(0x10006010) &= ~(0x08000000);
		udelay(30);

		/* PLL ready */

		/* wait 1us */
		udelay(1);

		/* PLL: LEW_SIMULATION */
		p->channel = CHANNEL_A;
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll_divider, 0x090000B1);
		/* SPM control clock enable */
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll_divider, 0x090000B2);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll_divider, 0x090000B1);

		p->channel = CHANNEL_B;
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll_divider, 0x090000B1);
		/* SPM control clock enable */
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll_divider, 0x090000B2);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->mempll_divider, 0x090000B1);

	} else { /* SBS */
		/* TBD Prb */
	}

	if (ucstatus) {
		return DRAM_FAIL;
	} else {
		return DRAM_OK;
	}
}

DRAM_STATUS_T DramcPreInit(DRAMC_CTX_T *p, EMI_SETTINGS *emi_set)
{
	u8 ucstatus = 0;
	u32 u4WL, u4MR2, reg_addr;

	/* AP u4value, */
	u4MR2 = (DramcGetMR2ByFreq(mt_get_dram_freq_setting()) >> 16) & 0xFF;

	switch (u4MR2&0x0F) {

	case 0x07:		/* RL9 WL5 */
		u4WL = 0x5;	/* 800, 1066 */
		break;
	case 0x08:		/* RL10 WL6 */
	case 0x09:		/* RL11 WL6 */
	case 0x0A:		/* RL12 WL6 */
		u4WL = 0x6;	/* 1333, 1466, 1600 */
		break;
	case 0x0C:		/* RL14 WL8 */
	case 0x0E:		/* RL16 WL8 */
		u4WL = 0x08;	/* 1792, 1866, 2133 */
		break;
	default:
		u4WL = 0x08;
		break;
	}

	printk(BIOS_INFO, "DramcPreInit: MR2 = %#x, WL = %#x\n", u4MR2, u4WL);

	/* sel_ph and write latency setting */
	ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->selph12, 0x10f010f0);
	/* for testing, 1600 PoP WL issue debug, */
	/* DQS T/4 in advance (DQS: 1, DQM: 0) */
	ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->selph13, 0xffffffff);
	ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->selph14, 0xffffffff);
	ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->selph15, 0x0000001f);

	/* TX_ADV_1T defined */
	reg_addr = mcSET_DRAMC_REG_ADDR(p, 0x400);
	/* TXDLY_CS, TXDLY_CS1 */
	ucstatus |= ucDram_Register_Write(reg_addr, 0x00000000);
	reg_addr = mcSET_DRAMC_REG_ADDR(p, 0x404);
	/* TXDLY_DQSGate, TXDLY_DQSGate_P1 */
	ucstatus |= ucDram_Register_Write(reg_addr, 0x00302000);
	reg_addr = mcSET_DRAMC_REG_ADDR(p, 0x408);
	/* TXLDY_RA* */
	ucstatus |= ucDram_Register_Write(reg_addr, 0x00000000);
	reg_addr = mcSET_DRAMC_REG_ADDR(p, 0x40c);
	/* TXLDY_RA* */
	ucstatus |= ucDram_Register_Write(reg_addr, 0x00000000);

	/* FREQ_BY_CHIP not defined */
	/* WL settings. */
	if (u4WL == 5) {
		/* WL5 */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[7], 0x11112222);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[8], 0x11112222);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[9], 0x11112222);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[10], 0xffff5555);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[11], 0x00ff0055);
	} else if (u4WL == 6) {
		/* WL6 */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[7], 0x22222222);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[8], 0x22222222);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[9], 0x22222222);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[10], 0x5555ffff);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[11], 0x005500ff);
	} else {
		/* WL8 */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[7], 0x33333333);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[8], 0x33333333);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[9], 0x33333333);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[10], 0x5555ffff);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[11], 0x005500ff);
	}

	/* CS_ADV not defined */
	ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[4], 0x04955555);
	ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[6], 0x00000432);

	/* CA phase select */
	/* CS_ADV not defined */
	ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->lpddr2_3, 0x220fffff);

	/* 0.5T? */
	ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->ac_time_05t, emi_set->DRAMC_ACTIM05T_VAL);

	/* CS_ADV not defined */
	ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->lpddr2_4, 0x220fffff);

	return ucstatus;
}

/* ------------------------------------------------------------------------- */
/*  DramcInit
 *  DRAMC Initialization.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 * ------------------------------------------------------------------------- */

DRAM_STATUS_T DramcInit(DRAMC_CTX_T *p, EMI_SETTINGS *emi_set)
{
	/* This function is implemented based on DE's bring up flow for DRAMC */
	u8 ucstatus = 0;
	u32 u4value;
	u32 addr, val;

	u4value = *(volatile unsigned *)(EMI_APB_BASE+0x0);

	if (p->channel == CHANNEL_A) {
		/* EMI_CONA[17] */
		if (u4value & 0x00020000) {
			uiDualRank = 1;
		} else {
			uiDualRank = 0;
		}
	} else {
		/* EMI_CONA[16] */
		if (u4value & 0x00010000) {
			uiDualRank = 1;
		} else {
			uiDualRank = 0;
		}
	}

	/* error handling */
	if (!p) {
		mcSHOW_ERR_MSG("context is NULL\n");
		return DRAM_FAIL;
	}

	if ((p->package != PACKAGE_SBS) && (p->package != PACKAGE_POP)) {
		mcSHOW_ERR_MSG("argument POP should be 0(SBS dram) "
				"or 1(POP dram)!\n");
		return DRAM_FAIL;
	}

	if (p->dram_type == TYPE_LPDDR3) {
		/* SPM_MODIFY defined */
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->peri[2], 0x00001010);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->peri[3], 0x00000000);

		/* 1800Mbps */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->conf2, 0x00403300);

		/* COMBO_MCP defined */
		val = emi_set->DRAMC_TEST2_4_VAL;
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->test2_4, val);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->clk1delay, 0x00e00000);

		/* COMBO_MCP & DUAL_RANKS defined */
		if (uiDualRank) {
			val = emi_set->DRAMC_RKCFG_VAL | 0x00000001;
			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->rkcfg, val);
		} else {
			val = emi_set->DRAMC_RKCFG_VAL & 0xfffffffe;
			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->rkcfg, val);
		}

		/* pimux */
		/* [11:10] DQIENQKEND 01 -> 00 for DATLAT calibration issue, */
		/* DQS input enable will refer to DATLAT */
		/* if need to enable this (for power saving), */
		/* do it after all calibration done */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->mckdly, 0x40100110);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->padctl4, 0x00000001);

		if (p->frequency > 800) {
			/* Reg.138[4] tCKEH/tCKEL extend 1T */
			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dummy, 0x80000c10);
		} else {
			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dummy, 0x80000c00);
		}

		/* FTTEST_ZQONLY not defined */
		val = 0x99009900 | (DRIVING_DS2_0<<20) | (DRIVING_DS2_0<<4);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->iodrv[5], val);
		val = 0x99009900 | (DRIVING_DS2_0<<20) | (DRIVING_DS2_0<<4);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->rsvd_ao6[0], val);

		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->rsvd_ao4[0], 0x00000000);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->r0dqsien, 0x40404040);

		/* [3:0] move to 0x80[4:0]. This value should be DATLAT-3 */
		/* It is used for runtime RX DQ/DQS K (not use??). */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqsctl1, 0x15000000);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqsctl2, 0x00000005);

		/* [25] DATLAT4 */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->phyctl1, 0x02000000);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->gddr3ctl1, 0x11000000);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->arbctl0, 0x00000080);
		/* Only enable clock pad 0. */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->clkctl, 0x10000000);
		udelay(1);

		/* COMBO_MCP defined */
		val = emi_set->DRAMC_CONF1_VAL;
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->conf1, val);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->dqsgctl, 0xc0000011);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqscal0, 0x00000000);

		/* COMBO_MCP defined */
		val = emi_set->DRAMC_ACTIM_VAL;
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->actim0, val);
		val = emi_set->DRAMC_MISCTL0_VAL;
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->misctl0, val);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->perfctl0, 0x00100000);

		/* COMBO_MCP defined */
		val = emi_set->DRAMC_DDR2CTL_VAL;
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->ddr2ctl, val);

		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->misc, 0x00000be0);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dllconf, 0xf1000000);
		/* COMBO_MCP */
		val = emi_set->DRAMC_ACTIM1_VAL;
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->actim1, val);

		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->dqsisel, 0x00000000);
		/* Disable ODT before ZQ calibration. */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->wodt, 0x00000001);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->padctl4, 0x00000005);

		udelay(200);	/* tINIT3 > 200us */

		/* MR63 -> Reset */
		/* COMBO_MCP defined */
		val = emi_set->iLPDDR3_MODE_REG_63;
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->mrs, val);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000001);

		udelay(10);	/* Wait >=10us if not check DAI. */

		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000000);

		/* MR10 -> ZQ Init */
		/* COMBO_MCP defined */
		val = emi_set->iLPDDR3_MODE_REG_10;
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->mrs, val);

		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000001);
		udelay(1);/* tZQINIT>=1us */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000000);

		/* LPDDR_MAX_DRIVE defined */
		/* MR3, driving stregth */
		/* for testing, set to max */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->mrs, 0x00010003);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000001);
		udelay(1);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000000);

		/* MR1 */
		/* COMBO_MCP */
		val = emi_set->iLPDDR3_MODE_REG_1;
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->mrs, val);

		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000001);
		udelay(1);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000000);

		/* MR2 */
		/* COMBO_MCP defined */
		val = DramcGetMR2ByFreq(mt_get_dram_freq_setting());
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->mrs, val);

		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000001);
		udelay(1);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000000);
		/* MR11, ODT disable. */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->mrs, 0x0000000b);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000001);
		udelay(1);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000000);

		/* [24] CKE1 on=1 */
		/* no effect because Reg.1ech already set CKE1=CKE0. */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->gddr3ctl1, 0x11100000);

		if (uiDualRank) {
			/* MR63 -> Reset */
			/* COMBO_MCP defined */
			val = 0x10000000 | emi_set->iLPDDR3_MODE_REG_63;
			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->mrs, val);

			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000001);
			udelay(10);	/* Wait >=10us if not check DAI. */
			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000000);

			/* MR10 -> ZQ Init */
			/* COMBO_MCP defined */
			val = 0x10000000 | emi_set->iLPDDR3_MODE_REG_10;
			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->mrs, val);

			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000001);
			udelay(1); /* tZQINIT>=1us */
			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000000);

			/* MR3, driving stregth */
			/* for testing, set to max */
			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->mrs, 0x10010003);
			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000001);
			udelay(1);
			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000000);

			/* MR1 */
			/* COMBO_MCP defined */
			val = 0x10000000 | emi_set->iLPDDR3_MODE_REG_1;
			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->mrs, val);

			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000001);
			udelay(1);
			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000000);

			/* MR2 */
			/* COMBO_MCP defined */
			int freq = mt_get_dram_freq_setting();

			val = 0x10000000;
			val |= DramcGetMR2ByFreq(freq);
			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->mrs, val);

			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000001);
			udelay(1);
			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000000);

			/* MR11, ODT disable. */
			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->mrs, 0x1000000b);
			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000001);
			udelay(1);
			ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000000);

		}

		/* COMBO_MCP defined */
		/* [24] CKE1 on=0 no effect */
		/* because Reg.1ech already set CKE1=CKE0. */
		val = emi_set->DRAMC_GDDR3CTL1_VAL;
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->gddr3ctl1, val);

		/* COMBO_MCP defined */
		val = emi_set->DRAMC_PD_CTRL_VAL;
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dramc_pd_ctrl, val);

		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->padctl4, 0x00000001);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->perfctl0, 0x00100001);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->zqcs, 0x00000a56);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->rsvd_ao1[0], 0x00000000);

		/* disable DQSICALI_NEW [23] (HW RX window calibration) */
		/* COMBO_MCP defined */
		val = emi_set->DRAMC_TEST2_3_VAL;
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->test2_3, val);

		/* COMBO_MCP defined */
		val = emi_set->DRAMC_CONF2_VAL;
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->conf2, val);

		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->padctl[1], 0x00000000);

		/* DISABLE_DRVREF defined */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->ocdk, 0x00000000);

		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->r1deldly, 0x12121212);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->padctl7, 0x00000000);

		/* CLKTDN_ENABLE defined */
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->tdsel[2], 1);	/* CLKTDN */
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->tdsel[2], 31);	/* DS3TDN */
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->tdsel[2], 29);	/* DS2TDN */
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->tdsel[2], 27);	/* DS1TDN */
		ucstatus |= dramc_setbit(p, (uintptr_t)&ddrphy_regs->tdsel[2], 25);	/* DS0TDN */

		/* DISABLE_PERBANK_REFRESH defined */
		ucstatus |= dramc_clrbit(p, (uintptr_t)&ao_regs->rkcfg, 7);

		/* REFTHD_ADJUST_0 defined */
		/* Clear R_DMREFTHD(dramc AO) 0x8[26:24]=0 */
		/* for reduce special command (MR4) wait refresh queue time. */
		addr = mcSET_DRAMC_REG_ADDR(p, 0x008);
		ucstatus |= ucDram_Register_Read(addr, &u4value);
		u4value &= 0xf8ffffff;
		ucstatus |= ucDram_Register_Write(addr, u4value);

		/* Duty default value. */
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->phyclkduty, 0x10010000);

		if (uiDualRank == 0) {
			/* Single rank. CKE1 always off. [21]=1 */
			ucstatus |= dramc_setbit(p, 0xf4, 21);
		}

		/* For DLE issue, TX delay not set now. */
		/* RX per-bit calibration. */
		if (p->channel == CHANNEL_A) {
			if (uiDualRank) {
				/* DQS RX input delay */
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->r0deldly,
								0x08080908);
				/* Need to adjust further. */
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->r1deldly,
								0x08080908);
				/* DQ input delay */
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[0],
								0x01010300);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[1],
								0x06030002);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[2],
								0x01010201);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[3],
								0x03020002);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[4],
								0x00010103);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[5],
								0x02010201);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[6],
								0x02040200);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[7],
								0x02020201);
			} else {
				/* DQS RX input delay */
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->r0deldly,
								0x110E0B0B);
				/* Need to adjust further */
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->r1deldly,
								0x110E0B0B);
				/* DQ input delay */
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[0],
								0x01040302);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[1],
								0x04010300);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[2],
								0x02040300);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[3],
								0x04030302);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[4],
								0x04070400);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[5],
								0x07070707);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[6],
								0x05070808);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[7],
								0x00010404);
			}
		} else {
			if (uiDualRank) {
				/* DQS RX input delay */
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->r0deldly,
								0x0B0B060B);
				/* Need to adjust further. */
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->r1deldly,
								0x0B0B060B);
				/* DQ input delay */
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[0],
								0x00020202);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[1],
								0x02020202);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[2],
								0x01020201);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[3],
								0x01010100);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[4],
								0x01010101);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[5],
								0x01000002);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[6],
								0x02000201);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[7],
								0x00010101);
			} else {
				/* DQS RX input delay */
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->r0deldly,
								0x12100D0D);
				/* Need to adjust further. */
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->r1deldly,
								0x12100D0D);
				/* DQ input delay */
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[0],
								0x05060604);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[1],
								0x04010400);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[2],
								0x05070300);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[3],
								0x05030504);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[4],
								0x07090500);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[5],
								0x08090707);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[6],
								0x080A0A0A);
				ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqidly[7],
								0x02000604);
			}
		}

	} else if (p->dram_type == TYPE_PCDDR3) {
		/* if (p->package == PACKAGE_POP) */
		{
			mcSHOW_ERR_MSG("don't support PCDDR3 now!\n");
			return DRAM_FAIL;
		}

		/* sel_ph and write latency */
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->selph12, 0x00f000f0);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->selph13, 0xffffffff);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->selph14, 0xffffffff);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->selph15, 0x0000001f);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[0], 0x11111111);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[1], 0x01312111);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[2], 0x11111111);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[3], 0x11111111);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[4], 0x05d55555);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[6], 0x00000217);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[7], 0x33333333);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[8], 0x33333333);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[9], 0x33333333);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[10], 0x5555ffff);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->selph[11], 0x005500ff);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->ac_time_05t, 0x040006e4);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->lpddr2_3, 0x08000000);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->lpddr2_4, 0x00000000);

		/* SPM_MODIFY */
		/* SPM control */
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->peri[2], 0x00001010);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->peri[3], 0x00000000);

		/* ========dramc_init============ */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->test2_4, 0x2300d10d);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->clk1delay, 0x00e00000);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->rkcfg, 0x00011400);
		/* [11:10] DQIENQKEND 01 -> 00 for DATLAT calibration issue, */
		/* DQS input enable will refer to DATLAT */
		/* if need to enable this (for power saving), */
		/* do it after all calibration done */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->mckdly, 0x40100110);

		/* DDR3 reset */
		udelay(200);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->padctl4, 0x000000a3);
		udelay(500);

		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dummy, 0x80000c00);

		val = 0x99009900 | (DRIVING_DS2_0<<20) | (DRIVING_DS2_0<<4);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->iodrv[5], val);
		val = 0x99009900 | (DRIVING_DS2_0<<20) | (DRIVING_DS2_0<<4);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->rsvd_ao6[0], val);

		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->rsvd_ao4[0], 0x00000000);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->r0dqsien, 0x40404040);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->rsvd_ao8[0], 0x00000000);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqsctl1, 0x14000000);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqsctl2, 0x00000000);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->phyctl1, 0x02000000);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->gddr3ctl1, 0x11000000);
		/* sync mod (Rome will use sync mode only) */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->misctl0, 0x17000000);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->arbctl0, 0x00000080);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->clkctl, 0x30000000);
		udelay(1);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->conf1, 0xc0748481);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->dqsgctl, 0xc0000011);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dqscal0, 0x00000000);
		/* for DDR3, DISDMOEDIS [16] must be 1 */
		/* due to self refresh exit clock stable */
		/* timing (tCKSRX) issue */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->perfctl0, 0x00110001);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->ddr2ctl, 0x00003301);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->misc, 0x000008e0);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dllconf, 0xf1000000);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->dqsisel, 0x00000000);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->padctl4, 0x000000a7);
		udelay(2);
		/* write ODT fixed to disable */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->wodt, 0x00000001);

		/* MR2 */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->mrs, 0x00004020);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000001);
		udelay(1);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000000);

		/* MR3 */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->mrs, 0x00006000);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000001);
		udelay(1);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000000);

		/* MR1 */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->mrs, 0x00002000);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000001);
		udelay(1);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000000);

		/* MR0 */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->mrs, 0x00001f15);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000001);
		udelay(1);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000000);

		/* ZQ calibration enable */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->mrs, 0x00000400);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000010);
		udelay(1);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00000000);
		udelay(1);

		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->spcmd, 0x00001100);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->padctl4, 0x000000a3);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->dramc_pd_ctrl, 0xd5643840);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->rsvd_ao1[0], 0x00000000);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->actim0, 0x558d48e7);
		/* disable DQSICALI_NEW [23] : HW RX window calibration */
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->test2_3, 0x9f3A0480);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->actim1, 0x80000060);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->conf2, 0x0000006c);
		ucstatus |= dramc_write32(p, (uintptr_t)&ddrphy_regs->padctl[1], 0x00000000);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->ocdk, 0x00000000);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->r1deldly, 0x12121212);
		ucstatus |= dramc_write32(p, (uintptr_t)&ao_regs->padctl7, 0x00000000);
		/* =========finish dramc_init====== */
	} else {
		mcSHOW_ERR_MSG("unknown dram type  should be lpddr2 "
				"or lpddr3 or ddr3!\n");
		return DRAM_FAIL;
	}

	if (ucstatus) {
		mcSHOW_ERR_MSG("register access fail!\n");
		return DRAM_FAIL;
	} else {
		return DRAM_OK;
	}
}

void DramcEnterSelfRefresh(DRAMC_CTX_T *p, u8 op)
{
	u8 ucstatus = 0;
	u32 uiTemp;
	u32 addr;

	if (op == 1) { /* enter self refresh */

		dramc_setbit(p, DRAMC_REG_CONF1, POS_CONF1_SELFREF);
		udelay(2);

		addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_SPCMDRESP);
		ucstatus |= ucDram_Register_Read(addr, &uiTemp);

		while ((mcTEST_BIT(uiTemp, POS_SPCMDRESP_SREF_STATE)) == 0) {
			mcSHOW_DBG_MSG3("Still not enter self refresh...\n");
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_SPCMDRESP);
			ucstatus |= ucDram_Register_Read(addr, &uiTemp);
		}

	} else { /* exit self refresh */

		dramc_clrbit(p, DRAMC_REG_CONF1, POS_CONF1_SELFREF);
		udelay(2);

		addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_SPCMDRESP);
		ucstatus |= ucDram_Register_Read(addr, &uiTemp);

		while ((mcTEST_BIT(uiTemp, POS_SPCMDRESP_SREF_STATE)) != 0) {
			mcSHOW_DBG_MSG3("Still not exit self refresh...\n");
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_SPCMDRESP);
			ucstatus |= ucDram_Register_Read(addr, &uiTemp);
		}
	}
}

void DramcDiv2PhaseSync(DRAMC_CTX_T *p)
{
	/* AP    u8 ucstatus = 0; */
	u32 u4value;

	/* SPM_MODIFY defined */
	u4value = (*(volatile unsigned int *)(CHB_DDRPHY_BASE + 0x640));
	mcCLR_BIT(&u4value, 5);
	(*(volatile unsigned int *)(CHB_DDRPHY_BASE + 0x640)) = u4value;

	udelay(1);

	u4value = (*(volatile unsigned int *)(CHB_DDRPHY_BASE + 0x640));
	mcSET_BIT(&u4value, 5);
	(*(volatile unsigned int *)(CHB_DDRPHY_BASE + 0x640)) = u4value;
}

void DramcPhyReset(DRAMC_CTX_T *p)
{
	/* reset phy */
	/* 0x0f0[28] = 1 -> 0 */
	dramc_setbit(p, DRAMC_REG_PHYCTL1, POS_PHYCTL1_PHYRST);

	/* read data counter reset */
	/* 0x0f4[25] = 1 -> 0 */
	dramc_setbit(p, DRAMC_REG_GDDR3CTL1, POS_GDDR3CTL1_RDATRST);

	/* delay 10ns, need to change when porting */
	udelay(1);

	dramc_clrbit(p, DRAMC_REG_PHYCTL1, POS_PHYCTL1_PHYRST);
	dramc_clrbit(p, DRAMC_REG_GDDR3CTL1, POS_GDDR3CTL1_RDATRST);
}

void DramcRANKINCTLConfig(DRAMC_CTX_T *p)
{
	u8 ucstatus = 0;
	u32 u4value, addr;
	u32 u4CoarseTune_MCK;

	if (uiDualRank) {
		/* RANKINCTL_ROOT1 = DQSINCTL+reg_TX_DLY_DQSGATE */
		/* (min of RK0 and RK1)-1. */
		if (opt_gw_coarse_value_R0[p->channel]
			< opt_gw_coarse_value_R1[p->channel]) {
			u32 tmp = opt_gw_coarse_value_R0[p->channel] >> 2;

			u4CoarseTune_MCK = tmp;
		} else {
			u32 tmp = opt_gw_coarse_value_R1[p->channel] >> 2;

			u4CoarseTune_MCK = tmp;
		}

		addr = mcSET_DRAMC_REG_ADDR(p, 0x138);
		ucstatus |= ucDram_Register_Read(addr, &u4value);
		mcSET_FIELD(&u4value, u4CoarseTune_MCK, 0x0f, 0);
		ucstatus |= ucDram_Register_Write(addr, u4value);

		/* RANKINCTL = RANKINCTL_ROOT1+0 */
		addr = mcSET_DRAMC_REG_ADDR(p, 0x1c4);
		ucstatus |= ucDram_Register_Read(addr, &u4value);
		/* u4CoarseTune_MCK += 2; */
		u4CoarseTune_MCK += 0;
		mcSET_FIELD(&u4value, u4CoarseTune_MCK, 0x000f0000, 16);
		ucstatus |= ucDram_Register_Write(addr, u4value);
	}

	if (p->dram_type == TYPE_LPDDR3) {
		/* Disable Per-bank Refresh when refresh rate >= 5 */
		/* (only for LPDDR3) Set (0x110[6]) */
		addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_RKCFG);
		ucstatus |= ucDram_Register_Read(addr, &u4value);
		mcSET_BIT(&u4value, POS_RKCFG_PBREF_DISBYRATE);
		addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_RKCFG);
		ucstatus |= ucDram_Register_Write(addr, u4value);
	}

	/* COMBPHY_GATING_SAME_DDRPHY not defined */
	addr = mcSET_DRAMC_REG_ADDR(p, 0x1dc);
	ucstatus |= ucDram_Register_Write(addr, u4value);
}

void DramcRunTimeConfig(DRAMC_CTX_T *p)
{
	u8 ucstatus = 0;
	u32 u4value, addr;

	volatile unsigned int *ptr;

	/* Enable HW gating here?! */
	/* HW_GATING defined */
	/* 0x1c0[31] */
	ucstatus |= dramc_setbit(p, DRAMC_REG_DQSCAL0, POS_DQSCAL0_STBCALEN);

	addr = mcSET_DRAMC_REG_ADDR(p, 0x138);
	ucstatus |= ucDram_Register_Read(addr, &u4value);
	if (p->frequency > 800) {
		/* if frequency >1600 , tCKE should >7 clk */
		mcSET_BIT(&u4value, 4);
	}
	ucstatus |= ucDram_Register_Write(addr, u4value);

	/* CLK_UNGATING not defined */
	addr = mcSET_DRAMC_REG_ADDR(p, 0x63c);
	ucstatus |= ucDram_Register_Read(addr, &u4value);
	/* ENABLE_DDYPHY_GATING defined */
	mcSET_BIT(&u4value, 2);	/* DDRPHY C/A M_CK clock gating enable */
	mcSET_BIT(&u4value, 1);	/* DDRPHY DQ M_CK clock gating enable */
	ucstatus |= ucDram_Register_Write(addr, u4value);

	/* EMI_OPTIMIZATION defined */
	addr = mcSET_DRAMC_REG_ADDR(p, 0x1ec);
	ucstatus |= ucDram_Register_Read(addr, &u4value);
	/* RWSPLIT_ENABLE not defined */
	u4value |= 0x00084f11;
	ucstatus |= ucDram_Register_Write(addr, u4value);

	/* ZQCS_ENABLE defined */
	if (p->dram_type == TYPE_LPDDR3) {
		ptr = (volatile unsigned *)(EMI_APB_BASE+0x00000000);
		if ((*ptr & 0x1) == 0x1) {
			/* dual channel */
			/* 1. Set (0x1e4[23:16] ZQCSCNT)=0 */
			addr = mcSET_DRAMC_REG_ADDR(p, 0x1e4);
			ucstatus |= ucDram_Register_Read(addr, &u4value);
			mcSET_FIELD(&u4value, 0x00, 0x00ff0000, 16);
			addr = mcSET_DRAMC_REG_ADDR(p, 0x1e4);
			ucstatus |= ucDram_Register_Write(addr, u4value);
			/* 2. Set (0x1ec[24] ZQCSMASK) for different channels */
			addr = mcSET_DRAMC_REG_ADDR(p, 0x1ec);
			ucstatus |= ucDram_Register_Read(addr, &u4value);
			if (p->channel == CHANNEL_A) {
				mcCLR_BIT(&u4value, 24);
			} else {
				mcSET_BIT(&u4value, 24);
			}
			addr = mcSET_DRAMC_REG_ADDR(p, 0x1ec);
			ucstatus |= ucDram_Register_Write(addr, u4value);
			/* 3. Enable (0x1ec[25] ZQCSDUAL) */
			ucstatus |= dramc_setbit(p, (uintptr_t)&ao_regs->perfctl0, 25);
		} else {
			/* single channel */
			/* Set (0x1e4[23:16] ZQCSCNT) */
			addr = mcSET_DRAMC_REG_ADDR(p, 0x1e4);
			ucstatus |= ucDram_Register_Read(addr, &u4value);
			mcSET_FIELD(&u4value, 0xff, 0x00080000, 16);
			addr = mcSET_DRAMC_REG_ADDR(p, 0x1e4);
			ucstatus |= ucDram_Register_Write(addr, u4value);
		}
	}
}

/* SPM_CONTROL_AFTERK defined */
void TransferToSPMControl(void)
{
	u32 val;

	/* 0x1020900c */
	/* Bit 7 : MPLL_PWR_SEL */
	/* Bit 11 : MPLL_ISO_SEL */
	/* Bit 15 : MPLL_EN_SEL */
	val = (0x01<<7) | (0x01<<11) | (0x01<<15);
	(*(volatile unsigned int *)(0x1020900c)) &= ~(val);

	/* 0x1000f5cc */
	/* Bit 0 : 1: use ddrphy reg to control MEMPLL-ISO/sc_apbias_off; */
	/*         0: use spm to control */
	/* Bit 4 : 1: bypass MEMPLL-ISO delay chain; 0: not bypass */
	/* Bit 8 : 1: use ddrphy register to control DIV2 SYNC; */
	/*         0: use spm to control */
	val = (0x01<<0) | (0x01<<4) | (0x01<<8);
	(*(volatile unsigned int *)(CHA_DDRPHY_BASE + (0x5cc))) &= ~(val);

	/* 0x100125cc */
	/* Bit 0 : 1: use ddrphy reg to control MEMPLL-ISO/sc_apbias_off; */
	/*         0: use spm to control */
	/* Bit 8 : 1: use ddrphy register to control DIV2 SYNC; */
	/*         0: use spm to control */
	val = (0x01<<0) | (0x01<<8);
	(*(volatile unsigned int *)(CHB_DDRPHY_BASE + (0x5cc))) &= ~(val);

	/* 0x1000f5c8 */
	/* Bit 0 : 1: use ddrphy register to control ALLCLK_EN/mempllout_off; */
	/*         0: use spm to control */
	/* Bit 9 : 1: use ddrphy register to control MEMPLL2_EN/mempll1_off; */
	/*         0: use spm to control */
	/* Bit 10 : 1: use ddrphy register to control MEMPLL3_EN/mempll1_off; */
	/*          0: use spm to control */
	/* Bit 11 : 1: use ddrphy register to control MEMPLL4_EN/mempll1_off; */
	/*          0: use spm to control */
	/* Bit 16 : 1: use ddrphy register to control */
	/*             MEMPLL_BIAS_LPF_EN/mempll2_off; 0: use spm to control */
	/* Bit 24 : 1: use ddrphy register to control */
	/*             MEMPLL_BIAS_EN/sc_apbias_off; 0: use spm to control */
	val = (0x01<<9) | (0x01<<10) | (0x01<<11) | (0x01<<16) | (0x01<<24);
	(*(volatile unsigned int *)(CHA_DDRPHY_BASE + (0x5c8))) &= ~(val);

	/* 0x100125c8 */
	/* Bit 0 : 1: use ddrphy register to control ALLCLK_EN/mempllout_off; */
	/*	   0: use spm to control */
	/* Bit 9 : 1: use ddrphy register to control MEMPLL2_EN/mempll1_off; */
	/*	   0: use spm to control */
	/* Bit 10 : 1: use ddrphy register to control MEMPLL3_EN/mempll1_off; */
	/*	    0: use spm to control */
	/* Bit 11 : 1: use ddrphy register to control MEMPLL4_EN/mempll1_off; */
	/*	    0: use spm to control */
	/* Bit 16 : 1: use ddrphy register to control */
	/*	       MEMPLL_BIAS_LPF_EN/mempll2_off; 0: use spm to control */
	/* Bit 24 : 1: use ddrphy register to control */
	/*	       MEMPLL_BIAS_EN/sc_apbias_off; 0: use spm to control */
	val = (0x01<<9) | (0x01<<10) | (0x01<<11) | (0x01<<16) | (0x01<<24);
	(*(volatile unsigned int *)(CHB_DDRPHY_BASE + (0x5c8))) &= ~(val);

}

void TransferToRegControl(void)
{
	u32 val;

	/* 0x1020900c */
	/* Bit 7 : MPLL_PWR_SEL */
	/* Bit 11 : MPLL_ISO_SEL */
	/* Bit 15 : MPLL_EN_SEL */
	val = (0x01<<7) | (0x01<<11) | (0x01<<15);
	(*(volatile unsigned int *)(0x1020900c)) |= val;

	/* 0x1000f5cc */
	/* Bit 0 : 1: use ddrphy reg to control MEMPLL-ISO/sc_apbias_off; */
	/*	   0: use spm to control */
	/* Bit 4 : 1: bypass MEMPLL-ISO delay chain; */
	/*	   0: not bypass */
	/* Bit 8 : 1: use ddrphy register to control DIV2 SYNC; */
	/*	   0: use spm to control */
	val = (0x01<<0) | (0x01<<4) | (0x01<<8);
	(*(volatile unsigned int *)(CHA_DDRPHY_BASE + (0x5cc))) |= val;

	/* 0x100125cc */
	/* Bit * : 0: use spm to control (for all Bit)*/
	/* Bit 0 : 1: use ddrphy reg to control MEMPLL-ISO/sc_apbias_off; */
	/* Bit 8 : 1: use ddrphy register to control DIV2 SYNC; */
	val = (0x01<<0) | (0x01<<8);
	(*(volatile unsigned int *)(CHB_DDRPHY_BASE + (0x5cc))) = val;

	/* 0x1000f5c8 */
	/* Bit * : 0: use spm to control (for all Bit)*/
	/* Bit 0 : 1: use ddrphy register to control ALLCLK_EN/mempllout_off; */
	/* Bit 9 : 1: use ddrphy register to control MEMPLL2_EN/mempll1_off; */
	/* Bit 10 : 1: use ddrphy register to control MEMPLL3_EN/mempll1_off; */
	/* Bit 11 : 1: use ddrphy register to control MEMPLL4_EN/mempll1_off; */
	/* Bit 16 : 1: use ddrphy register to control */
	/*		MEMPLL_BIAS_LPF_EN/mempll2_off; */
	/* Bit 24 : 1: use ddrphy register to control */
	/*		MEMPLL_BIAS_EN/sc_apbias_off; */
	val = (0x01<<0) | (0x01<<9) | (0x01<<10);
	val |= (0x01<<11) | (0x01<<16) | (0x01<<24);
	(*(volatile unsigned int *)(CHA_DDRPHY_BASE + (0x5c8))) = val;

	/* 0x100125c8 */
	/* Bit * : 0: use spm to control (for all Bit)*/
	/* Bit 0 : 1: use ddrphy register to control ALLCLK_EN/mempllout_off; 0: */
	/* Bit 9 : 1: use ddrphy register to control MEMPLL2_EN/mempll1_off; 0: */
	/* Bit 10 : 1: use ddrphy register to control MEMPLL3_EN/mempll1_off; 0: */
	/* Bit 11 : 1: use ddrphy register to control MEMPLL4_EN/mempll1_off; 0: */
	/* Bit 16 : 1: use ddrphy register to control MEMPLL_BIAS_LPF_EN/mempll2_off; */
	/* Bit 24 : 1: use ddrphy register to control MEMPLL_BIAS_EN/sc_apbias_off; */
	val = (0x01<<0) | (0x01<<9) | (0x01<<10);
	val |= (0x01<<11) | (0x01<<16) | (0x01<<24);
	(*(volatile unsigned int *)(CHB_DDRPHY_BASE + (0x5c8))) = val;

}

u32 DramcEngine2(DRAMC_CTX_T *p, DRAM_TE_OP_T wr, u32 test2_1, u32 test2_2,
		 u8 testaudpat, s16 loopforever, u8 period, u8 log2loopcount)
{
	u8 ucengine_status;
	u8 ucstatus = 0, ucloop_count = 0;
	u32 u4value, u4result = 0xffffffff;
	u32 u4log2loopcount = (u32)log2loopcount;
	u32 addr;

	/* error handling */
	if (!p) {
		mcSHOW_ERR_MSG("context is NULL\n");
		return u4result;
	}

	/* check loop number validness */
	if (log2loopcount > 15) {
		mcSHOW_ERR_MSG("wrong parameter log2loopcount:    ");
		mcSHOW_ERR_MSG("log2loopcount just 0 to 15 !\n");
		return u4result;
	}

	/* disable self test engine1 and self test engine2 */
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_CONF2);
	ucstatus |= ucDram_Register_Read(addr, &u4value);
	mcCLR_MASK(&u4value, MASK_CONF2_TE12_ENABLE);
	addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_CONF2);
	ucstatus |= ucDram_Register_Write(addr, u4value);

	/* we get the status
	 * loopforever    period    status	mean
	 * 0              x         1		read\write one time,
	 *					then exit ,don't write log
	 * >0             x         2		read\write in a loop,
	 *					after "loopforever" seconds,
	 *					disable it ,return the R\W status
	 * -1            >0         3		read\write in a loop,
	 *					every "period" seconds,
	 *					check result,
	 *					only when we find error,
	 *					write log and exit
	 * -2            >0         4		read\write in a loop,
	 *					every "period" seconds,
	 *					write log,
	 *					only when we find error,
	 *					write log and exit
	 * -3             x         5		just enable loop forever,
	 *					then exit (so we should disable
	 *					engine1 outside the function)
	 */

	if (loopforever == 0) {
		ucengine_status = 1;
	} else if (loopforever > 0) {
		ucengine_status = 2;
	} else if (loopforever == -1) {
		if (period > 0) {
			ucengine_status = 3;
		} else {
			mcSHOW_ERR_MSG("parameter 'period' should be ");
			mcSHOW_ERR_MSG("equal or greater than 0\n");
			return u4result;
		}
	} else if (loopforever == -2) {
		if (period > 0) {
			ucengine_status = 4;
		} else {
			mcSHOW_ERR_MSG("parameter 'period' should be ");
			mcSHOW_ERR_MSG("equal or greater than 0\n");
			return u4result;
		}
	} else if (loopforever == -3) {
		if (period > 0) {
			ucengine_status = 5;
		} else {
			mcSHOW_ERR_MSG("parameter 'period' should be ");
			mcSHOW_ERR_MSG("equal or greater than 0\n");
			return u4result;
		}
	} else {
		mcSHOW_ERR_MSG("parameter 'loopforever' should be ");
		mcSHOW_ERR_MSG("0 -1 -2 -3 or greater than 0\n");
		return u4result;
	}

	/* 1.set pattern ,base address ,offset address */
	/* 2.select  ISI pattern or audio pattern or xtalk pattern */
	/* 3.set loop number */
	/* 4.enable read or write */
	/* 5.loop to check DM_CMP_CPT */
	/* 6.return CMP_ERR */
	/* currently only implement ucengine_status = 1, */
	/* others are left for future extension */
	u4result = 0;
	while (1) {
		/* 1 */
		addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_TEST2_1);
		ucstatus |= ucDram_Register_Write(addr, test2_1);
		addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_TEST2_2);
		ucstatus |= ucDram_Register_Write(addr, test2_2);

		/* 2 & 3 */
		/* (TESTXTALKPAT, TESTAUDPAT) = 00 (ISI), 01 (AUD), */
		/*				10 (XTALK), 11 (UNKNOWN) */
		if (testaudpat == 2) { /* xtalk */
			/* select XTALK pattern */
			/* set addr 0x044 [7] to 0 */
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_TEST2_3);
			ucstatus |= ucDram_Register_Read(addr, &u4value);
			mcCLR_BIT(&u4value, POS_TEST2_3_TESTAUDPAT);
			mcSET_FIELD(&u4value, u4log2loopcount,
				    MASK_TEST2_3_TESTCNT,
					POS_TEST2_3_TESTCNT);
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_TEST2_3);
			ucstatus |= ucDram_Register_Write(addr, u4value);

			/* set addr 0x48[16] to 1, TESTXTALKPAT = 1 */
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_TEST2_4);
			ucstatus |= ucDram_Register_Read(addr, &u4value);
			mcSET_BIT(&u4value, POS_TEST2_4_TESTXTALKPAT);
			mcCLR_BIT(&u4value, POS_TEST2_4_TESTAUDBITINV);
			/* for XTALK pattern characteristic, */
			/* we don't enable write after read */
			mcCLR_BIT(&u4value, POS_TEST2_4_TESTAUDMODE);
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_TEST2_4);
			ucstatus |= ucDram_Register_Write(addr, u4value);
		} else if (testaudpat == 1) { /* audio */
			/* set AUDINIT=0x11 AUDINC=0x0d AUDBITINV=1 AUDMODE=1 */
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_TEST2_4);
			ucstatus |= ucDram_Register_Read(addr, &u4value);
			mcSET_FIELD(&u4value, 0x00000011,
				    MASK_TEST2_4_TESTAUDINIT,
					POS_TEST2_4_TESTAUDINIT);
			mcSET_FIELD(&u4value, 0x0000000d,
				    MASK_TEST2_4_TESTAUDINC,
					POS_TEST2_4_TESTAUDINC);
			mcSET_BIT(&u4value, POS_TEST2_4_TESTAUDBITINV);
			mcSET_BIT(&u4value, POS_TEST2_4_TESTAUDMODE);
			mcCLR_BIT(&u4value, POS_TEST2_4_TESTXTALKPAT);
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_TEST2_4);
			ucstatus |= ucDram_Register_Write(addr, u4value);

			/* set addr 0x044 [7] to 1 ,select audio pattern */
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_TEST2_3);
			ucstatus |= ucDram_Register_Read(addr, &u4value);
			mcSET_BIT(&u4value, POS_TEST2_3_TESTAUDPAT);
			mcSET_FIELD(&u4value, u4log2loopcount,
				    MASK_TEST2_3_TESTCNT,
					POS_TEST2_3_TESTCNT);
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_TEST2_3);
			ucstatus |= ucDram_Register_Write(addr, u4value);
		} else { /* ISI */
			/* select ISI pattern */
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_TEST2_3);
			ucstatus |= ucDram_Register_Read(addr, &u4value);
			mcCLR_BIT(&u4value, POS_TEST2_3_TESTAUDPAT);
			mcSET_FIELD(&u4value, u4log2loopcount,
				    MASK_TEST2_3_TESTCNT,
					POS_TEST2_3_TESTCNT);
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_TEST2_3);
			ucstatus |= ucDram_Register_Write(addr, u4value);

			dramc_clrbit(p, DRAMC_REG_TEST2_4,
				       POS_TEST2_4_TESTXTALKPAT);
		}

		/* 4 */
		if (wr == TE_OP_READ_CHECK) {
			if ((testaudpat == 1) || (testaudpat == 2)) {
				/* if audio pattern, enable read only */
				/* (disable write after read), */
				/* AUDMODE=0x48[15]=0 */
				dramc_clrbit(p, DRAMC_REG_TEST2_4,
					       POS_TEST2_4_TESTAUDMODE);
			}

			/* enable read, 0x008[31:29] */
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_CONF2);
			ucstatus |= ucDram_Register_Read(addr, &u4value);
			mcSET_FIELD(&u4value, (u32)2,
				    MASK_CONF2_TE12_ENABLE, POS_CONF2_TEST1);
			ucstatus |= ucDram_Register_Write(addr, u4value);

		} else if (wr == TE_OP_WRITE_READ_CHECK) {
			/* enable write */
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_CONF2);
			ucstatus |= ucDram_Register_Read(addr, &u4value);
			mcSET_FIELD(&u4value, (u32)4,
				    MASK_CONF2_TE12_ENABLE, POS_CONF2_TEST1);
			ucstatus |= ucDram_Register_Write(addr, u4value);

			/* read data compare ready check */
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_TESTRPT);
			ucstatus |= ucDram_Register_Read(addr, &u4value);
			ucloop_count = 0;
			while (mcCHK_BIT1(u4value, POS_TESTRPT_DM_CMP_CPT) == 0) {
				addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_TESTRPT);
				ucstatus |= ucDram_Register_Read(addr, &u4value);
			}

			/* disable write */
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_CONF2);
			ucstatus |= ucDram_Register_Read(addr, &u4value);
			mcSET_FIELD(&u4value, (u32)0,
				    MASK_CONF2_TE12_ENABLE, POS_CONF2_TEST1);
			ucstatus |= ucDram_Register_Write(addr, u4value);

			/* enable read */
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_CONF2);
			ucstatus |= ucDram_Register_Read(addr, &u4value);
			mcSET_FIELD(&u4value, (u32)2,
				    MASK_CONF2_TE12_ENABLE, POS_CONF2_TEST1);
			ucstatus |= ucDram_Register_Write(addr, u4value);
		}

		/* 5 */
		/* read data compare ready check */
		addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_TESTRPT);
		ucstatus |= ucDram_Register_Read(addr, &u4value);
		ucloop_count = 0;
		while (mcCHK_BIT1(u4value, POS_TESTRPT_DM_CMP_CPT) == 0) {
			addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_TESTRPT);
			ucstatus |= ucDram_Register_Read(addr, &u4value);
		}

		/* delay 10ns after ready check from DE suggestion (1ms here) */
		udelay(1);

		/* 6 */
		/* return CMP_ERR, 0 is ok , others are fail, */
		/* disable test2w or test2r get result */
		addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_CMP_ERR);
		ucstatus |= ucDram_Register_Read(addr, &u4value);
		/* or all result */
		u4result |= u4value;
		/* disable read */
		addr = mcSET_DRAMC_REG_ADDR(p, DRAMC_REG_CONF2);
		ucstatus |= ucDram_Register_Read(addr, &u4value);
		mcCLR_MASK(&u4value, MASK_CONF2_TE12_ENABLE);
		ucstatus |= ucDram_Register_Write(addr, u4value);

		/* handle status */
		if (ucengine_status == 1) {
			break;
		} else if (ucengine_status == 2) {
			mcSHOW_ERR_MSG("not support for now\n");
			break;
		} else if (ucengine_status == 3) {
			mcSHOW_ERR_MSG("not support for now\n");
			break;
		} else if (ucengine_status == 4) {
			mcSHOW_ERR_MSG("not support for now\n");
			break;
		} else if (ucengine_status == 5) {
			mcSHOW_ERR_MSG("not support for now\n");
			break;
		} else {
			mcSHOW_ERR_MSG("not support for now\n");
			break;
		}
	}

	return u4result;
}

void DramcLowFreqWrite(DRAMC_CTX_T *p)
{
	/* AP    u8 ucstatus = 0; */
	u16 u2freq_orig;
	u32 u4err_value;

	if (p->fglow_freq_write_en == ENABLE) {
		u2freq_orig = p->frequency;
		p->frequency = p->frequency_low;
		mcSHOW_DBG_MSG("Enable low speed write function...\n");
		mcFPRINTF((fp_A60808, "Enable low speed write function...\n"));

		/* we will write data in memory on a low frequency, */
		/* to make sure the data we write is  right */
		/* then use engine2 read to do the calibration */
		/* so ,we will do : */
		/* 1.change freq */
		/* 2.use self test engine2 write to write data, */
		/*   and check the data is right or not */
		/* 3.change freq to original value */

		/* 1. change freq */
		p->channel = CHANNEL_A;
		DramcEnterSelfRefresh(p, 1); /* enter self refresh */
		p->channel = CHANNEL_B;
		DramcEnterSelfRefresh(p, 1); /* enter self refresh */
		udelay(1);
		TransferToRegControl();

		MemPllPreInit(p);
		MemPllInit(p);
		udelay(1);
		p->channel = CHANNEL_A;
		DramcEnterSelfRefresh(p, 0); /* exit self refresh */
		p->channel = CHANNEL_B;
		DramcEnterSelfRefresh(p, 0); /* exit self refresh */

		/* Need to do phase sync after change frequency */
		DramcDiv2PhaseSync(p);

		/* 2. use self test engine2 to write data */
		/*    (only support AUDIO or XTALK pattern) */
		p->channel = CHANNEL_A;
		if (p->test_pattern == TEST_AUDIO_PATTERN) {
			u4err_value = DramcEngine2(p, TE_OP_WRITE_READ_CHECK,
						   p->test2_1, p->test2_2,
							1, 0, 0, 0);
		} else if (p->test_pattern == TEST_XTALK_PATTERN) {
			u4err_value = DramcEngine2(p, TE_OP_WRITE_READ_CHECK,
						   p->test2_1, p->test2_2,
							2, 0, 0, 0);
		} else {
			mcSHOW_ERR_MSG("ERROR! Only support AUDIO or XTALK ");
			mcSHOW_ERR_MSG("in Low Speed Write and High Speed ");
			mcSHOW_ERR_MSG("Read calibration!!\n");
			u4err_value = 0xffffffff;
		}

		/* May error due to gating not calibrate @ low speed */
		mcSHOW_DBG_MSG("Low speed write error ");
		mcSHOW_DBG_MSG("value: %#x\n", u4err_value);
		mcFPRINTF((fp_A60808, "Low speed write error "));
		mcFPRINTF((fp_A60808, "value: %#x\n", u4err_value));

		p->channel = CHANNEL_B;
		if (p->test_pattern == TEST_AUDIO_PATTERN) {
			u4err_value = DramcEngine2(p, TE_OP_WRITE_READ_CHECK,
						   p->test2_1, p->test2_2,
							1, 0, 0, 0);
		} else if (p->test_pattern == TEST_XTALK_PATTERN) {
			u4err_value = DramcEngine2(p, TE_OP_WRITE_READ_CHECK,
						   p->test2_1, p->test2_2,
							2, 0, 0, 0);
		} else {
			mcSHOW_ERR_MSG("ERROR! Only support AUDIO or XTALK ");
			mcSHOW_ERR_MSG("in Low Speed Write and High Speed ");
			mcSHOW_ERR_MSG("Read calibration!!\n");
			u4err_value = 0xffffffff;
		}

		/* May error due to gating not calibrate @ low speed */
		mcSHOW_DBG_MSG("Low speed write error ");
		mcSHOW_DBG_MSG("value: %#x\n", u4err_value);
		mcFPRINTF((fp_A60808, "Low speed write error "));
		mcFPRINTF((fp_A60808, "value: %#x\n", u4err_value));

		/* do phy reset due to ring counter may be wrong */
		p->channel = CHANNEL_A;
		DramcPhyReset(p);
		p->channel = CHANNEL_B;
		DramcPhyReset(p);

		/* 3. change to original freq */
		p->frequency = u2freq_orig;
		p->channel = CHANNEL_A;
		DramcEnterSelfRefresh(p, 1); /* enter self refresh */
		p->channel = CHANNEL_B;
		DramcEnterSelfRefresh(p, 1); /* enter self refresh */
		udelay(1);
		TransferToRegControl();

		MemPllPreInit(p);
		MemPllInit(p);
		udelay(1);
		p->channel = CHANNEL_A;
		DramcEnterSelfRefresh(p, 0); /* exit self refresh */
		p->channel = CHANNEL_B;
		DramcEnterSelfRefresh(p, 0); /* exit self refresh */

		/* Need to do phase sync after change frequency */
		DramcDiv2PhaseSync(p);
	}
}
