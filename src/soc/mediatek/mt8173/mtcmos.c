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
#include <delay.h>
#include <smp/spinlock.h>
#include <console/console.h>
#include <soc/addressmap.h>
#include <soc/mtcmos.h>

enum {
	SPM_POWERON_CONFIG_SET		= SPM_BASE + 0x000,
	SPM_LITTLE_CPU0_PWR_CON		= SPM_BASE + 0x200,
	SPM_LITTLE_CPU1_PWR_CON		= SPM_BASE + 0x218,
	SPM_LITTLE_CPUTOP_L2_PDN	= SPM_BASE + 0x244,
	SPM_LITTLE_CPU0_L1_PDN		= SPM_BASE + 0x25c,
	SPM_LITTLE_CPU1_L1_PDN		= SPM_BASE + 0x264,
	SPM_BIG_CPU0_PWR_CON		= SPM_BASE + 0x2a0,
	SPM_BIG_CPU1_PWR_CON		= SPM_BASE + 0x2a4,
	SPM_BIG_CPUTOP_PWR_CON		= SPM_BASE + 0x2b0,
	SPM_BIG_L1_PWR_CON		= SPM_BASE + 0x2b4,
	SPM_BIG_L2_PWR_CON		= SPM_BASE + 0x2b8,
	SPM_SLEEP_DUAL_VCORE_PWR_CON	= SPM_BASE + 0x404,
	SPM_PWR_STATUS			= SPM_BASE + 0x60c,
	SPM_PWR_STATUS_2ND		= SPM_BASE + 0x610,
	SPM_SLEEP_TIMER_STA		= SPM_BASE + 0x720,
};

enum { SPM_PROJECT_CODE = 0xb16 };

enum {
	SRAM_ISOINT_B	= 1U << 6,
	SRAM_CKISO	= 1U << 5,
	PWR_CLK_DIS	= 1U << 4,
	PWR_ON_2ND	= 1U << 3,
	PWR_ON		= 1U << 2,
	PWR_ISO		= 1U << 1,
	PWR_RST_B	= 1U << 0,
};

enum {
	L1_PDN_ACK	= 1U << 8,
	L1_PDN		= 1U << 0,
};

enum {
	CPU1_BIG_L1_PDN_ACK	= 1U << 9,
	CPU0_BIG_L1_PDN_ACK	= 1U << 8,
	CPU1_BIG_L1_PDN		= 1U << 1,
	CPU0_BIG_L1_PDN		= 1U << 0,

	BIG_L2_PDN_ACK		= 1U << 8,
	BIG_L2_PDN_ISO		= 1U << 2,
	BIG_L2_PDN		= 1U << 0,
};

enum {
	BIG_CPU1	= 1U << 17,
	BIG_CPU0	= 1U << 16,
	BIG_CPUTOP	= 1U << 15,
	LITTLE_CPU1	= 1U << 10,
	LITTLE_CPU0	= 1U << 9,
	LITTLE_CPUTOP	= 1U << 8,
};

enum {
	BIG_CPUTOP_STANDBYWFI	= 1U << 25,
	BIG_CPU1_STANDBYWFI	= 1U << 21,
	BIG_CPU0_STANDBYWFI	= 1U << 20,
	LITTLE_CPU1_STANDBYWFI	= 1U << 17,
	LITTLE_CPU0_STANDBYWFI	= 1U << 16,
	VBIG_PWR_ISO		= 1U << 13,

	BIG_PDN_REQ		= 30,
};

enum {
	TOPAXI_PROT_EN		= INFRACFG_AO_BASE + 0x0220,
	TOPAXI_PROT_STA1	= INFRACFG_AO_BASE + 0x0228,
	BIG_MISCDBG		= MTK_MCUCFG_BASE + 0x20c,
};

#define spm_read(addr)		read32((void *)((uintptr_t)addr))
#define spm_write(addr, val)	write32((void *)((uintptr_t)addr), val)

DECLARE_SPIN_LOCK(cpu_lock);
DECLARE_SPIN_LOCK(noncpu_lock);

static inline void mtcmos_noncpu_lock(void)
{
	spin_lock(&noncpu_lock);
}

static inline void mtcmos_noncpu_unlock(void)
{
	spin_unlock(&noncpu_lock);
}

static inline void mtcmos_cpu_lock(void)
{
	spin_lock(&cpu_lock);
}

static inline void mtcmos_cpu_unlock(void)
{
	spin_unlock(&cpu_lock);
}

static int mtcmos_topaxi_prot(int bit, int en)
{
	mtcmos_noncpu_lock();

	if (en == 1) {
		spm_write(TOPAXI_PROT_EN,
			spm_read(TOPAXI_PROT_EN) | (1 << bit));

		while (!(spm_read(TOPAXI_PROT_STA1) & (1 << bit)))
			continue;
	} else {
		spm_write(TOPAXI_PROT_EN,
			spm_read(TOPAXI_PROT_EN) & ~(1 << bit));

		while (spm_read(TOPAXI_PROT_STA1) & (1 << bit))
			continue;
	}

	mtcmos_noncpu_unlock();

	return 0;
}

static int mtcmos_ctrl_cpusys1(int state, int check_wfi)
{
	/* enable register control */
	spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

	if (state == STA_POWER_DOWN) {
		spm_write(BIG_MISCDBG, spm_read(BIG_MISCDBG) | 0x1);

		if (check_wfi)
			while ((spm_read(SPM_SLEEP_TIMER_STA) &
				BIG_CPUTOP_STANDBYWFI) == 0)
				continue;

		mtcmos_topaxi_prot(BIG_PDN_REQ, 1);

		mtcmos_cpu_lock();

		spm_write(SPM_BIG_CPUTOP_PWR_CON,
			spm_read(SPM_BIG_CPUTOP_PWR_CON) | SRAM_CKISO);

		spm_write(SPM_BIG_L2_PWR_CON,
			spm_read(SPM_BIG_L2_PWR_CON) | BIG_L2_PDN);

		while (!(spm_read(SPM_BIG_L2_PWR_CON) & BIG_L2_PDN_ACK))
			continue;

		spm_write(SPM_BIG_CPUTOP_PWR_CON,
			  (spm_read(SPM_BIG_CPUTOP_PWR_CON) | PWR_CLK_DIS) &
				~PWR_RST_B);
		spm_write(SPM_BIG_CPUTOP_PWR_CON,
			spm_read(SPM_BIG_CPUTOP_PWR_CON) | PWR_ISO);

		spm_write(SPM_BIG_CPUTOP_PWR_CON,
			spm_read(SPM_BIG_CPUTOP_PWR_CON) & ~PWR_ON);
		spm_write(SPM_BIG_CPUTOP_PWR_CON,
			spm_read(SPM_BIG_CPUTOP_PWR_CON) & ~PWR_ON_2ND);

		while (((spm_read(SPM_PWR_STATUS) & BIG_CPUTOP) != 0) ||
		       ((spm_read(SPM_PWR_STATUS_2ND) & BIG_CPUTOP) != 0))
			continue;

		mtcmos_cpu_unlock();

		if (!(spm_read(SPM_SLEEP_DUAL_VCORE_PWR_CON) & VBIG_PWR_ISO)) {
			spm_write(SPM_SLEEP_DUAL_VCORE_PWR_CON,
				  spm_read(SPM_SLEEP_DUAL_VCORE_PWR_CON) |
					VBIG_PWR_ISO);
		}
	} else {
		if ((spm_read(SPM_SLEEP_DUAL_VCORE_PWR_CON) & VBIG_PWR_ISO)) {
			spm_write(SPM_SLEEP_DUAL_VCORE_PWR_CON,
				  spm_read(SPM_SLEEP_DUAL_VCORE_PWR_CON) &
					~VBIG_PWR_ISO);
		}

		mtcmos_cpu_lock();

		spm_write(SPM_BIG_CPUTOP_PWR_CON,
			spm_read(SPM_BIG_CPUTOP_PWR_CON) | PWR_ON);

		while (!(spm_read(SPM_PWR_STATUS) & BIG_CPUTOP))
			continue;

		spm_write(SPM_BIG_CPUTOP_PWR_CON,
			spm_read(SPM_BIG_CPUTOP_PWR_CON) | PWR_ON_2ND);

		while (!(spm_read(SPM_PWR_STATUS_2ND) & BIG_CPUTOP))
			continue;

		spm_write(SPM_BIG_L2_PWR_CON,
			spm_read(SPM_BIG_L2_PWR_CON) & ~BIG_L2_PDN_ISO);

		spm_write(SPM_BIG_CPUTOP_PWR_CON,
			  spm_read(SPM_BIG_CPUTOP_PWR_CON) & ~PWR_CLK_DIS);

		spm_write(SPM_BIG_L2_PWR_CON,
			spm_read(SPM_BIG_L2_PWR_CON) & ~BIG_L2_PDN);

		while ((spm_read(SPM_BIG_L2_PWR_CON) & BIG_L2_PDN_ACK) != 0)
			continue;

		spm_write(SPM_BIG_CPUTOP_PWR_CON,
			  spm_read(SPM_BIG_CPUTOP_PWR_CON) | SRAM_ISOINT_B);

		spm_write(SPM_BIG_CPUTOP_PWR_CON,
			spm_read(SPM_BIG_CPUTOP_PWR_CON) & ~SRAM_CKISO);

		spm_write(SPM_BIG_CPUTOP_PWR_CON,
			spm_read(SPM_BIG_CPUTOP_PWR_CON) & ~PWR_ISO);

		spm_write(SPM_BIG_CPUTOP_PWR_CON,
			spm_read(SPM_BIG_CPUTOP_PWR_CON) | PWR_RST_B);

		mtcmos_cpu_unlock();

		mtcmos_topaxi_prot(BIG_PDN_REQ, 0);
	}

	return 0;
}

static int mtcmos_ctrl_little(unsigned int linear_id, int state, int check_wfi)
{
	uint32_t reg_pwr_con;
	uint32_t reg_l1_pdn;
	uint32_t bit_stby_wfi;
	uint32_t bit_cpu;

	switch (linear_id) {
	case CPU_ID_LITTLE_CPU0:
		reg_pwr_con = SPM_LITTLE_CPU0_PWR_CON;
		reg_l1_pdn = SPM_LITTLE_CPU0_L1_PDN;
		bit_stby_wfi = LITTLE_CPU0_STANDBYWFI;
		bit_cpu = LITTLE_CPU0;
		break;
	case CPU_ID_LITTLE_CPU1:
		reg_pwr_con = SPM_LITTLE_CPU1_PWR_CON;
		reg_l1_pdn = SPM_LITTLE_CPU1_L1_PDN;
		bit_stby_wfi = LITTLE_CPU1_STANDBYWFI;
		bit_cpu = LITTLE_CPU1;
		break;
	default:
		/* should never come to here */
		return -1;
	}

	/* enable register control */
	spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

	if (state == STA_POWER_DOWN) {
		if (check_wfi)
			while (!(spm_read(SPM_SLEEP_TIMER_STA) & bit_stby_wfi))
				continue;

		mtcmos_cpu_lock();

		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | PWR_ISO);

		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | SRAM_CKISO);
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~SRAM_ISOINT_B);
		spm_write(reg_l1_pdn, spm_read(reg_l1_pdn) | L1_PDN);

		while (!(spm_read(reg_l1_pdn) & L1_PDN_ACK))
			continue;

		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~PWR_RST_B);
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | PWR_CLK_DIS);

		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~PWR_ON);
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~PWR_ON_2ND);

		while (((spm_read(SPM_PWR_STATUS) & bit_cpu) != 0) ||
		       ((spm_read(SPM_PWR_STATUS_2ND) & bit_cpu) != 0))
			continue;

		mtcmos_cpu_unlock();
	} else {
		mtcmos_cpu_lock();

		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | PWR_ON);

		udelay(1);

		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | PWR_ON_2ND);

		while (!(spm_read(SPM_PWR_STATUS) & bit_cpu) ||
		       !(spm_read(SPM_PWR_STATUS_2ND) & bit_cpu))
			continue;

		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~PWR_ISO);

		spm_write(reg_l1_pdn, spm_read(reg_l1_pdn) & ~L1_PDN);

		while ((spm_read(reg_l1_pdn) & L1_PDN_ACK) != 0)
			continue;

		udelay(1);

		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | SRAM_ISOINT_B);
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~SRAM_CKISO);

		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~PWR_CLK_DIS);
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | PWR_RST_B);

		mtcmos_cpu_unlock();
	}

	return 0;
}

static int mtcmos_ctrl_big(unsigned int linear_id, int state, int check_wfi)
{
	uint32_t reg_pwr_con;
	uint32_t bit_stby_wfi;
	uint32_t bit_l1_pdn;
	uint32_t bit_l1_pdn_ack;
	uint32_t bit_cpu;
	uint32_t bit_other_cpus; /* other CPUs bit mask in same cluster */

	switch (linear_id) {
	case CPU_ID_BIG_CPU0:
		reg_pwr_con = SPM_BIG_CPU0_PWR_CON;
		bit_stby_wfi = BIG_CPU0_STANDBYWFI;
		bit_l1_pdn = CPU0_BIG_L1_PDN;
		bit_l1_pdn_ack = CPU0_BIG_L1_PDN_ACK;
		bit_cpu = BIG_CPU0;
		bit_other_cpus = BIG_CPU1;
		break;
	case CPU_ID_BIG_CPU1:
		reg_pwr_con = SPM_BIG_CPU1_PWR_CON;
		bit_stby_wfi = BIG_CPU1_STANDBYWFI;
		bit_l1_pdn = CPU1_BIG_L1_PDN;
		bit_l1_pdn_ack = CPU1_BIG_L1_PDN_ACK;
		bit_cpu = BIG_CPU1;
		bit_other_cpus = BIG_CPU0;
		break;
	default:
		/* should never come to here */
		return -1;
	}

	/* enable register control */
	spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

	if (state == STA_POWER_DOWN) {
		if (check_wfi)
			while (!(spm_read(SPM_SLEEP_TIMER_STA) & bit_stby_wfi))
				continue;

		mtcmos_cpu_lock();

		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | SRAM_CKISO);
		spm_write(SPM_BIG_L1_PWR_CON,
			spm_read(SPM_BIG_L1_PWR_CON) | bit_l1_pdn);

		while (!(spm_read(SPM_BIG_L1_PWR_CON) & bit_l1_pdn_ack))
			continue;

		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | PWR_ISO);

		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~PWR_ON);
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~PWR_ON_2ND);

		while (((spm_read(SPM_PWR_STATUS) & bit_cpu) != 0) ||
		       ((spm_read(SPM_PWR_STATUS_2ND) & bit_cpu) != 0))
			continue;

		mtcmos_cpu_unlock();

		if (!(spm_read(SPM_PWR_STATUS) & bit_other_cpus) &&
		    !(spm_read(SPM_PWR_STATUS_2ND) & bit_other_cpus))
			mtcmos_ctrl_cpusys1(state, check_wfi);
	} else {
		if (!(spm_read(SPM_PWR_STATUS) & BIG_CPUTOP) &&
		    !(spm_read(SPM_PWR_STATUS_2ND) & BIG_CPUTOP))
			mtcmos_ctrl_cpusys1(state, check_wfi);

		mtcmos_cpu_lock();

		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~PWR_RST_B);

		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | PWR_ON);

		while (!(spm_read(SPM_PWR_STATUS) & bit_cpu))
			continue;

		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | PWR_ON_2ND);

		while (!(spm_read(SPM_PWR_STATUS_2ND) & bit_cpu))
			continue;

		spm_write(SPM_BIG_L1_PWR_CON,
			spm_read(SPM_BIG_L1_PWR_CON) & ~bit_l1_pdn);

		while ((spm_read(SPM_BIG_L1_PWR_CON) & bit_l1_pdn_ack) != 0)
			continue;

		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~SRAM_CKISO);
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~PWR_ISO);

		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | PWR_RST_B);

		mtcmos_cpu_unlock();
	}

	return 0;
}

int mtcmos_ctrl_cpu(unsigned int linear_id, int state, int check_wfi)
{
	int ret;

	switch (linear_id) {
	case CPU_ID_LITTLE_CPU0:
	case CPU_ID_LITTLE_CPU1:
		ret = mtcmos_ctrl_little(linear_id, state, check_wfi);
		break;
	case CPU_ID_BIG_CPU0:
	case CPU_ID_BIG_CPU1:
		ret = mtcmos_ctrl_big(linear_id, state, check_wfi);
		break;
	default:
		printk(BIOS_DEBUG, ":%s: error occurs, linear_id=%d\n",
			__func__, linear_id);
		return -1;
	}

	return ret;
}
