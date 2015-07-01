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
	SRAM_PDN           = 0xf << 8,
	DIS_SRAM_ACK       = 0x1 << 12,
	AUD_SRAM_ACK       = 0xf << 12,
};

enum {
	DIS_PWR_STA_MASK   = 0x1 << 3,
	AUD_PWR_STA_MASK   = 0x1 << 24,
};

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
