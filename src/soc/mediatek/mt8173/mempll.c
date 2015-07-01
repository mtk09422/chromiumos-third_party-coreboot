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

#include <arch/io.h>
#include <soc/pll.h>

#include <soc/dramc_pi_api.h>
#include <soc/dramc_common.h>
#include <soc/dramc_register.h>

extern DRAMC_CTX_T DramCtx_LPDDR3;

void mt_mempll_cali(DRAMC_CTX_T *p)
{
	/* called after chA and chB init done */
	/* MEMPLL05 registers, some are located @ chA and others are @ chB */
	p->channel = CHANNEL_A;
	DramcPllPhaseCal(p);
	p->channel = CHANNEL_B;
	DramcPllPhaseCal(p);
}

static void mt_mempll_init(DRAMC_CTX_T *p)
{
	MemPllPreInit((DRAMC_CTX_T *) p);
	MemPllInit((DRAMC_CTX_T *) p);
}

void mt_mempll_pre(void)
{
	DRAMC_CTX_T *psDramCtx;

	psDramCtx = &DramCtx_LPDDR3;
	printk(BIOS_DEBUG, "[PLL] mempll_init\n");
	mt_mempll_init(psDramCtx);
	return;
}

void mt_mempll_post(void)
{
	DRAMC_CTX_T *psDramCtx;

	psDramCtx = &DramCtx_LPDDR3;
	printk(BIOS_DEBUG, "[PLL] mempll_cali\n");
	mt_mempll_cali(psDramCtx);
	return;
}

