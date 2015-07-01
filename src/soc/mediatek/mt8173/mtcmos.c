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
#include <console/console.h>
#include <delay.h>
#include <smp/spinlock.h>
#include <soc/addressmap.h>
#include <soc/mtcmos.h>
#include <soc/spm.h>

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

enum {
	SRAM_PDN           = 0xf << 8,
	DIS_SRAM_ACK       = 0x1 << 12,
	AUD_SRAM_ACK       = 0xf << 12,
};

enum {
	DIS_PWR_STA_MASK   = 0x1 << 3,
	AUD_PWR_STA_MASK   = 0x1 << 24,
};

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
		write32((void *)TOPAXI_PROT_EN,
			read32((void *)TOPAXI_PROT_EN) | (1 << bit));

		while (!(read32((void *)TOPAXI_PROT_STA1) & (1 << bit)))
			continue;
	} else {
		write32((void *)TOPAXI_PROT_EN,
			read32((void *)TOPAXI_PROT_EN) & ~(1 << bit));

		while (read32((void *)TOPAXI_PROT_STA1) & (1 << bit))
			continue;
	}

	mtcmos_noncpu_unlock();

	return 0;
}

static int mtcmos_ctrl_cpusys1(int state, int check_wfi)
{
	/* enable register control */
	write32(&mt8173_spm->poweron_config_set,
		(SPM_PROJECT_CODE << 16) | (1U << 0));

	if (state == STA_POWER_DOWN) {
		write32((void *)BIG_MISCDBG, read32((void *)BIG_MISCDBG) | 0x1);

		if (check_wfi)
			while ((read32(&mt8173_spm->sleep_timer_sta) &
				BIG_CPUTOP_STANDBYWFI) == 0)
				continue;

		mtcmos_topaxi_prot(BIG_PDN_REQ, 1);

		mtcmos_cpu_lock();

		write32(&mt8173_spm->ca15_cputop_pwr_con,
			read32(&mt8173_spm->ca15_cputop_pwr_con) | SRAM_CKISO);

		write32(&mt8173_spm->ca15_l2_pwr_con,
			read32(&mt8173_spm->ca15_l2_pwr_con) | BIG_L2_PDN);

		while (!(read32(&mt8173_spm->ca15_l2_pwr_con) & BIG_L2_PDN_ACK))
			continue;

		write32(&mt8173_spm->ca15_cputop_pwr_con,
			(read32(&mt8173_spm->ca15_cputop_pwr_con) | PWR_CLK_DIS) &
				~PWR_RST_B);
		write32(&mt8173_spm->ca15_cputop_pwr_con,
			read32(&mt8173_spm->ca15_cputop_pwr_con) | PWR_ISO);

		write32(&mt8173_spm->ca15_cputop_pwr_con,
			read32(&mt8173_spm->ca15_cputop_pwr_con) & ~PWR_ON);
		write32(&mt8173_spm->ca15_cputop_pwr_con,
			read32(&mt8173_spm->ca15_cputop_pwr_con) & ~PWR_ON_2ND);

		while (((read32(&mt8173_spm->pwr_status) & BIG_CPUTOP) != 0) ||
		       ((read32(&mt8173_spm->pwr_status_2nd) & BIG_CPUTOP) != 0))
			continue;

		mtcmos_cpu_unlock();

		if (!(read32(&mt8173_spm->sleep_dual_vcore_pwr_con) & VBIG_PWR_ISO)) {
			write32(&mt8173_spm->sleep_dual_vcore_pwr_con,
				read32(&mt8173_spm->sleep_dual_vcore_pwr_con) |
				VBIG_PWR_ISO);
		}
	} else {
		if ((read32(&mt8173_spm->sleep_dual_vcore_pwr_con) & VBIG_PWR_ISO)) {
			write32(&mt8173_spm->sleep_dual_vcore_pwr_con,
				read32(&mt8173_spm->sleep_dual_vcore_pwr_con) &
				~VBIG_PWR_ISO);
		}

		mtcmos_cpu_lock();

		write32(&mt8173_spm->ca15_cputop_pwr_con,
			read32(&mt8173_spm->ca15_cputop_pwr_con) | PWR_ON);

		while (!(read32(&mt8173_spm->pwr_status) & BIG_CPUTOP))
			continue;

		write32(&mt8173_spm->ca15_cputop_pwr_con,
			read32(&mt8173_spm->ca15_cputop_pwr_con) | PWR_ON_2ND);

		while (!(read32(&mt8173_spm->pwr_status_2nd) & BIG_CPUTOP))
			continue;

		write32(&mt8173_spm->ca15_l2_pwr_con,
			read32(&mt8173_spm->ca15_l2_pwr_con) & ~BIG_L2_PDN_ISO);

		write32(&mt8173_spm->ca15_cputop_pwr_con,
			read32(&mt8173_spm->ca15_cputop_pwr_con) & ~PWR_CLK_DIS);

		write32(&mt8173_spm->ca15_l2_pwr_con,
			read32(&mt8173_spm->ca15_l2_pwr_con) & ~BIG_L2_PDN);

		while ((read32(&mt8173_spm->ca15_l2_pwr_con) & BIG_L2_PDN_ACK) != 0)
			continue;

		write32(&mt8173_spm->ca15_cputop_pwr_con,
			read32(&mt8173_spm->ca15_cputop_pwr_con) | SRAM_ISOINT_B);

		write32(&mt8173_spm->ca15_cputop_pwr_con,
			read32(&mt8173_spm->ca15_cputop_pwr_con) & ~SRAM_CKISO);

		write32(&mt8173_spm->ca15_cputop_pwr_con,
			read32(&mt8173_spm->ca15_cputop_pwr_con) & ~PWR_ISO);

		write32(&mt8173_spm->ca15_cputop_pwr_con,
			read32(&mt8173_spm->ca15_cputop_pwr_con) | PWR_RST_B);

		mtcmos_cpu_unlock();

		mtcmos_topaxi_prot(BIG_PDN_REQ, 0);
	}

	return 0;
}

static int mtcmos_ctrl_little(unsigned int linear_id, int state, int check_wfi)
{
	void *reg_pwr_con;
	void *reg_l1_pdn;
	uint32_t bit_stby_wfi;
	uint32_t bit_cpu;

	switch (linear_id) {
	case CPU_ID_LITTLE_CPU0:
		reg_pwr_con = &mt8173_spm->ca7_cpu0_pwr_con;
		reg_l1_pdn = &mt8173_spm->ca7_cpu[0].l1_pdn;
		bit_stby_wfi = LITTLE_CPU0_STANDBYWFI;
		bit_cpu = LITTLE_CPU0;
		break;
	case CPU_ID_LITTLE_CPU1:
		reg_pwr_con = &mt8173_spm->ca7_cpu1_pwr_con;
		reg_l1_pdn = &mt8173_spm->ca7_cpu[1].l1_pdn;
		bit_stby_wfi = LITTLE_CPU1_STANDBYWFI;
		bit_cpu = LITTLE_CPU1;
		break;
	default:
		/* should never come to here */
		return -1;
	}

	/* enable register control */
	write32(&mt8173_spm->poweron_config_set, (SPM_PROJECT_CODE << 16) | (1U << 0));

	if (state == STA_POWER_DOWN) {
		if (check_wfi)
			while (!(read32(&mt8173_spm->sleep_timer_sta) & bit_stby_wfi))
				continue;

		mtcmos_cpu_lock();

		write32(reg_pwr_con, read32(reg_pwr_con) | PWR_ISO);

		write32(reg_pwr_con, read32(reg_pwr_con) | SRAM_CKISO);
		write32(reg_pwr_con, read32(reg_pwr_con) & ~SRAM_ISOINT_B);
		write32(reg_l1_pdn, read32(reg_l1_pdn) | L1_PDN);

		while (!(read32(reg_l1_pdn) & L1_PDN_ACK))
			continue;

		write32(reg_pwr_con, read32(reg_pwr_con) & ~PWR_RST_B);
		write32(reg_pwr_con, read32(reg_pwr_con) | PWR_CLK_DIS);

		write32(reg_pwr_con, read32(reg_pwr_con) & ~PWR_ON);
		write32(reg_pwr_con, read32(reg_pwr_con) & ~PWR_ON_2ND);

		while (((read32(&mt8173_spm->pwr_status) & bit_cpu) != 0) ||
		       ((read32(&mt8173_spm->pwr_status_2nd) & bit_cpu) != 0))
			continue;

		mtcmos_cpu_unlock();
	} else {
		mtcmos_cpu_lock();

		write32(reg_pwr_con, read32(reg_pwr_con) | PWR_ON);

		udelay(1);

		write32(reg_pwr_con, read32(reg_pwr_con) | PWR_ON_2ND);

		while (!(read32(&mt8173_spm->pwr_status) & bit_cpu) ||
		       !(read32(&mt8173_spm->pwr_status_2nd) & bit_cpu))
			continue;

		write32(reg_pwr_con, read32(reg_pwr_con) & ~PWR_ISO);

		write32(reg_l1_pdn, read32(reg_l1_pdn) & ~L1_PDN);

		while ((read32(reg_l1_pdn) & L1_PDN_ACK) != 0)
			continue;

		udelay(1);

		write32(reg_pwr_con, read32(reg_pwr_con) | SRAM_ISOINT_B);
		write32(reg_pwr_con, read32(reg_pwr_con) & ~SRAM_CKISO);

		write32(reg_pwr_con, read32(reg_pwr_con) & ~PWR_CLK_DIS);
		write32(reg_pwr_con, read32(reg_pwr_con) | PWR_RST_B);

		mtcmos_cpu_unlock();
	}

	return 0;
}

static int mtcmos_ctrl_big(unsigned int linear_id, int state, int check_wfi)
{
	void *reg_pwr_con;
	uint32_t bit_stby_wfi;
	uint32_t bit_l1_pdn;
	uint32_t bit_l1_pdn_ack;
	uint32_t bit_cpu;
	uint32_t bit_other_cpus; /* other CPUs bit mask in same cluster */

	switch (linear_id) {
	case CPU_ID_BIG_CPU0:
		reg_pwr_con = &mt8173_spm->ca15_cpu_pwr_con[0];
		bit_stby_wfi = BIG_CPU0_STANDBYWFI;
		bit_l1_pdn = CPU0_BIG_L1_PDN;
		bit_l1_pdn_ack = CPU0_BIG_L1_PDN_ACK;
		bit_cpu = BIG_CPU0;
		bit_other_cpus = BIG_CPU1;
		break;
	case CPU_ID_BIG_CPU1:
		reg_pwr_con = &mt8173_spm->ca15_cpu_pwr_con[1];
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
	write32(&mt8173_spm->poweron_config_set, (SPM_PROJECT_CODE << 16) | (1U << 0));

	if (state == STA_POWER_DOWN) {
		if (check_wfi)
			while (!(read32(&mt8173_spm->sleep_timer_sta) & bit_stby_wfi))
				continue;

		mtcmos_cpu_lock();

		write32(reg_pwr_con, read32(reg_pwr_con) | SRAM_CKISO);
		write32(&mt8173_spm->ca15_l1_pwr_con,
			read32(&mt8173_spm->ca15_l1_pwr_con) | bit_l1_pdn);

		while (!(read32(&mt8173_spm->ca15_l1_pwr_con) & bit_l1_pdn_ack))
			continue;

		write32(reg_pwr_con, read32(reg_pwr_con) | PWR_ISO);

		write32(reg_pwr_con, read32(reg_pwr_con) & ~PWR_ON);
		write32(reg_pwr_con, read32(reg_pwr_con) & ~PWR_ON_2ND);

		while (((read32(&mt8173_spm->pwr_status) & bit_cpu) != 0) ||
		       ((read32(&mt8173_spm->pwr_status_2nd) & bit_cpu) != 0))
			continue;

		mtcmos_cpu_unlock();

		if (!(read32(&mt8173_spm->pwr_status) & bit_other_cpus) &&
		    !(read32(&mt8173_spm->pwr_status_2nd) & bit_other_cpus))
			mtcmos_ctrl_cpusys1(state, check_wfi);
	} else {
		if (!(read32(&mt8173_spm->pwr_status) & BIG_CPUTOP) &&
		    !(read32(&mt8173_spm->pwr_status_2nd) & BIG_CPUTOP))
			mtcmos_ctrl_cpusys1(state, check_wfi);

		mtcmos_cpu_lock();

		write32(reg_pwr_con, read32(reg_pwr_con) & ~PWR_RST_B);

		write32(reg_pwr_con, read32(reg_pwr_con) | PWR_ON);

		while (!(read32(&mt8173_spm->pwr_status) & bit_cpu))
			continue;

		write32(reg_pwr_con, read32(reg_pwr_con) | PWR_ON_2ND);

		while (!(read32(&mt8173_spm->pwr_status_2nd) & bit_cpu))
			continue;

		write32(&mt8173_spm->ca15_l1_pwr_con,
			read32(&mt8173_spm->ca15_l1_pwr_con) & ~bit_l1_pdn);

		while ((read32(&mt8173_spm->ca15_l1_pwr_con) & bit_l1_pdn_ack) != 0)
			continue;

		write32(reg_pwr_con, read32(reg_pwr_con) & ~SRAM_CKISO);
		write32(reg_pwr_con, read32(reg_pwr_con) & ~PWR_ISO);

		write32(reg_pwr_con, read32(reg_pwr_con) | PWR_RST_B);

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

void mtcmos_ctrl_disp(int state)
{
	volatile unsigned int val;

	write32(&mt8173_spm->poweron_config_set, (SPM_PROJECT_CODE << 16) | (1U << 0));

	if (state == STA_POWER_DOWN) {

		write32(&mt8173_spm->dis_pwr_con, read32(&mt8173_spm->dis_pwr_con) | SRAM_PDN);

		while ((read32(&mt8173_spm->dis_pwr_con) & DIS_SRAM_ACK) != DIS_SRAM_ACK)
			;

		write32(&mt8173_spm->dis_pwr_con, read32(&mt8173_spm->dis_pwr_con) | PWR_ISO);

		val = read32(&mt8173_spm->dis_pwr_con);
		val = (val & ~PWR_RST_B) | PWR_CLK_DIS;
		write32(&mt8173_spm->dis_pwr_con, val);

		write32(&mt8173_spm->dis_pwr_con, read32(&mt8173_spm->dis_pwr_con) &
			~(PWR_ON | PWR_ON_2ND));

		while ((read32(&mt8173_spm->pwr_status) & DIS_PWR_STA_MASK)
		       || (read32(&mt8173_spm->pwr_status_2nd) & DIS_PWR_STA_MASK))
			;
	} else {    /* STA_POWER_ON */
		write32(&mt8173_spm->dis_pwr_con, read32(&mt8173_spm->dis_pwr_con) | PWR_ON);
		write32(&mt8173_spm->dis_pwr_con, read32(&mt8173_spm->dis_pwr_con) | PWR_ON_2ND);

		while (!(read32(&mt8173_spm->pwr_status) & DIS_PWR_STA_MASK)
		       || !(read32(&mt8173_spm->pwr_status_2nd) & DIS_PWR_STA_MASK))
			;

		write32(&mt8173_spm->dis_pwr_con, read32(&mt8173_spm->dis_pwr_con) &
			~PWR_CLK_DIS);
		write32(&mt8173_spm->dis_pwr_con, read32(&mt8173_spm->dis_pwr_con) &
			~PWR_ISO);
		write32(&mt8173_spm->dis_pwr_con, read32(&mt8173_spm->dis_pwr_con) |
			PWR_RST_B);

		write32(&mt8173_spm->dis_pwr_con, read32(&mt8173_spm->dis_pwr_con) &
			~SRAM_PDN);

		while ((read32(&mt8173_spm->dis_pwr_con) & DIS_SRAM_ACK))
			;
	}
}

void mtcmos_ctrl_audio(int state)
{
	write32(&mt8173_spm->poweron_config_set, (SPM_PROJECT_CODE << 16) | (1U << 0));

	if (state == STA_POWER_DOWN) {
		write32(&mt8173_spm->audio_pwr_con, read32(&mt8173_spm->audio_pwr_con) |
			  SRAM_PDN);
		while ((read32(&mt8173_spm->audio_pwr_con) & AUD_SRAM_ACK) !=
		       AUD_SRAM_ACK)
			;

		write32(&mt8173_spm->audio_pwr_con, read32(&mt8173_spm->audio_pwr_con) |
			  PWR_ISO);
		write32(&mt8173_spm->audio_pwr_con, (read32(&mt8173_spm->audio_pwr_con) &
			  ~PWR_RST_B) | PWR_CLK_DIS);
		write32(&mt8173_spm->audio_pwr_con, read32(&mt8173_spm->audio_pwr_con) &
			  ~(PWR_ON | PWR_ON_2ND));
		while ((read32(&mt8173_spm->pwr_status) & AUD_PWR_STA_MASK) ||
		       (read32(&mt8173_spm->pwr_status_2nd) & AUD_PWR_STA_MASK))
			;
	} else {    /* STA_POWER_ON */
		write32(&mt8173_spm->audio_pwr_con, read32(&mt8173_spm->audio_pwr_con) |
			  PWR_ON);
		write32(&mt8173_spm->audio_pwr_con, read32(&mt8173_spm->audio_pwr_con) |
			  PWR_ON_2ND);
		while (!(read32(&mt8173_spm->pwr_status) & AUD_PWR_STA_MASK) ||
		       !(read32(&mt8173_spm->pwr_status_2nd) & AUD_PWR_STA_MASK))
			;
		write32(&mt8173_spm->audio_pwr_con, read32(&mt8173_spm->audio_pwr_con) &
			  ~PWR_CLK_DIS);
		write32(&mt8173_spm->audio_pwr_con, read32(&mt8173_spm->audio_pwr_con) &
			  ~PWR_ISO);
		write32(&mt8173_spm->audio_pwr_con, read32(&mt8173_spm->audio_pwr_con) |
			  PWR_RST_B);
		write32(&mt8173_spm->audio_pwr_con, read32(&mt8173_spm->audio_pwr_con) &
			  ~SRAM_PDN);
		while ((read32(&mt8173_spm->audio_pwr_con) & AUD_SRAM_ACK))
			;
	}
}
