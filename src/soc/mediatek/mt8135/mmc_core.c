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

/*=======================================================================*/
/* HEADER FILES                                                          */
/*=======================================================================*/
#include <delay.h>
#include <string.h>
#include <assert.h>

#include <soc/mt8135.h>
#include <soc/msdc.h>
#include <soc/msdc_utils.h>
#include <soc/mmc_core.h>
#include <soc/emmc_device_list.h>

#if CONFIG_DEBUG_MMC
#define DEBUG_MMC(level, x...)		printk(level, x)
#else
#define DEBUG_MMC(level, x...)
#endif /* CONFIG_DEBUG_MMC */

#define NR_MMC             (MSDC_MAX_NUM)
#define CMD_RETRIES        (5)
#define CMD_TIMEOUT        (100)	/* 100ms */

static struct mmc_host sd_host[NR_MMC];
static struct mmc_card sd_card[NR_MMC];

static const unsigned int tran_exp[] = {
	10000, 100000, 1000000, 10000000,
	0, 0, 0, 0
};

static const unsigned char tran_mant[] = {
	0, 10, 12, 13, 15, 20, 25, 30,
	35, 40, 45, 50, 55, 60, 70, 80,
};

static const unsigned char mmc_tran_mant[] = {
	0, 10, 12, 13, 15, 20, 26, 30,
	35, 40, 45, 52, 55, 60, 70, 80,
};

static const unsigned int tacc_exp[] = {
	1, 10, 100, 1000, 10000, 100000, 1000000, 10000000,
};

static const unsigned int tacc_mant[] = {
	0, 10, 12, 13, 15, 20, 25, 30,
	35, 40, 45, 50, 55, 60, 70, 80,
};

static u32 unstuff_bits(u32 *resp, u32 start, u32 size)
{
	const u32 __mask = (1 << (size)) - 1;
	const int __off = 3 - ((start) / 32);
	const int __shft = (start) & 31;
	u32 __res;

	__res = resp[__off] >> __shft;
	if ((size) + __shft >= 32)
		__res |= resp[__off - 1] << (32 - __shft);
	return __res & __mask;
}

#define UNSTUFF_BITS(r, s, sz)    unstuff_bits(r, s, sz)

struct mmc_host *mmc_get_host(int id)
{
	return &sd_host[id];
}

struct mmc_card *mmc_get_card(int id)
{
	return &sd_card[id];
}

static int mmc_cmd(struct mmc_host *host, struct mmc_command *cmd)
{
	int err;
	int retry = cmd->retries;

	do {
		err = msdc_cmd(host, cmd);
		if (err == MMC_ERR_NONE)
			break;
	} while (retry--);

	return err;
}

static int mmc_app_cmd(struct mmc_host *host, struct mmc_command *cmd,
		       u32 rca, int retries)
{
	int err = MMC_ERR_FAILED;
	struct mmc_command appcmd;

	appcmd.opcode = MMC_CMD_APP_CMD;
	appcmd.arg = rca << 16;
	appcmd.rsptyp = RESP_R1;
	appcmd.retries = CMD_RETRIES;
	appcmd.timeout = CMD_TIMEOUT;

	do {
		err = mmc_cmd(host, &appcmd);

		if (err == MMC_ERR_NONE)
			err = mmc_cmd(host, cmd);
		if (err == MMC_ERR_NONE)
			break;
	} while (retries--);

	return err;
}

static u32 mmc_select_voltage(struct mmc_host *host, u32 ocr)
{
	int bit;

	ocr &= host->ocr_avail;

	bit = uffs(ocr);
	if (bit) {
		bit -= 1;
		ocr &= 3 << bit;
	} else {
		ocr = 0;
	}
	return ocr;
}

static int mmc_go_idle(struct mmc_host *host)
{
	struct mmc_command cmd;

	cmd.opcode = MMC_CMD_GO_IDLE_STATE;
	cmd.rsptyp = RESP_NONE;
	cmd.arg = 0;
	cmd.retries = CMD_RETRIES;
	cmd.timeout = CMD_TIMEOUT;

	return mmc_cmd(host, &cmd);
}

static int mmc_send_status(struct mmc_host *host, struct mmc_card *card,
			   u32 *status)
{
	int err;
	struct mmc_command cmd;

	cmd.opcode = MMC_CMD_SEND_STATUS;
	cmd.arg = card->rca << 16;
	cmd.rsptyp = RESP_R1;
	cmd.retries = CMD_RETRIES;
	cmd.timeout = CMD_TIMEOUT;

	err = mmc_cmd(host, &cmd);

	if (err == MMC_ERR_NONE) {
		*status = cmd.resp[0];
	}
	return err;
}

static int mmc_send_if_cond(struct mmc_host *host, u32 ocr)
{
	struct mmc_command cmd;
	int err;
	static const u8 test_pattern = 0xAA;
	u8 result_pattern;

	/*
	 * To support SD 2.0 cards, we must always invoke SD_SEND_IF_COND
	 * before SD_APP_OP_COND. This command will harmlessly fail for
	 * SD 1.0 cards.
	 */

	cmd.opcode = SD_CMD_SEND_IF_COND;
	cmd.arg = ((ocr & 0xFF8000) != 0) << 8 | test_pattern;
	cmd.rsptyp = RESP_R1;
	cmd.retries = 0;
	cmd.timeout = CMD_TIMEOUT;

	err = mmc_cmd(host, &cmd);

	if (err != MMC_ERR_NONE)
		return err;

	result_pattern = cmd.resp[0] & 0xFF;

	if (result_pattern != test_pattern)
		return MMC_ERR_INVALID;

	return MMC_ERR_NONE;
}

static int mmc_send_op_cond(struct mmc_host *host, u32 ocr, u32 *rocr)
{
	struct mmc_command cmd;
	int i;
	int err = 0;

	cmd.opcode = MMC_CMD_SEND_OP_COND;
	cmd.arg = ocr;
	cmd.rsptyp = RESP_R3;
	cmd.retries = 0;
	cmd.timeout = CMD_TIMEOUT;

	for (i = 100; i; i--) {
		err = mmc_cmd(host, &cmd);
		if (err)
			break;

		/* if we're just probing, do a single pass */
		if (ocr == 0)
			break;

		if (cmd.resp[0] & MMC_CARD_BUSY)
			break;

		err = MMC_ERR_TIMEOUT;

		unsigned _i;

		for (_i = 0; _i < 10; _i++)
			udelay(1000);
	}

	if (!err && rocr)
		*rocr = cmd.resp[0];

	return err;
}

static int mmc_send_app_op_cond(struct mmc_host *host, u32 ocr, u32 *rocr)
{
	struct mmc_command cmd;
	int i;
	int err = 0;

	cmd.opcode = SD_ACMD_SEND_OP_COND;
	cmd.arg = ocr;
	cmd.rsptyp = RESP_R3;
	cmd.retries = CMD_RETRIES;
	cmd.timeout = CMD_TIMEOUT;

	for (i = 100; i; i--) {
		err = mmc_app_cmd(host, &cmd, 0, CMD_RETRIES);
		if (err != MMC_ERR_NONE)
			break;

		if (cmd.resp[0] & MMC_CARD_BUSY || ocr == 0)
			break;

		err = MMC_ERR_TIMEOUT;

		unsigned _i;

		for (_i = 0; _i < 10; _i++)
			udelay(1000);
	}

	if (rocr)
		*rocr = cmd.resp[0];

	return err;
}

static int mmc_all_send_cid(struct mmc_host *host, u32 *cid)
{
	int err;
	struct mmc_command cmd;

	/* send cid */
	cmd.opcode = MMC_CMD_ALL_SEND_CID;
	cmd.arg = 0;
	cmd.rsptyp = RESP_R2;
	cmd.retries = CMD_RETRIES;
	cmd.timeout = CMD_TIMEOUT;

	err = mmc_cmd(host, &cmd);

	if (err != MMC_ERR_NONE)
		return err;

	memcpy(cid, cmd.resp, sizeof(u32) * 4);

	return MMC_ERR_NONE;
}

/* code size add 1KB*/
static void mmc_decode_cid(struct mmc_card *card)
{
	u32 *resp = card->raw_cid;

	memset(&card->cid, 0, sizeof(struct mmc_cid));

	if (mmc_card_sd(card)) {
		/*
		 * SD doesn't currently have a version field so we will
		 * have to assume we can parse this.
		 */
		card->cid.manfid = UNSTUFF_BITS(resp, 120, 8);
		card->cid.oemid = UNSTUFF_BITS(resp, 104, 16);
		card->cid.prod_name[0] = UNSTUFF_BITS(resp, 96, 8);
		card->cid.prod_name[1] = UNSTUFF_BITS(resp, 88, 8);
		card->cid.prod_name[2] = UNSTUFF_BITS(resp, 80, 8);
		card->cid.prod_name[3] = UNSTUFF_BITS(resp, 72, 8);
		card->cid.prod_name[4] = UNSTUFF_BITS(resp, 64, 8);
		card->cid.hwrev = UNSTUFF_BITS(resp, 60, 4);
		card->cid.fwrev = UNSTUFF_BITS(resp, 56, 4);
		card->cid.serial = UNSTUFF_BITS(resp, 24, 32);
		card->cid.year = UNSTUFF_BITS(resp, 12, 8);
		card->cid.month = UNSTUFF_BITS(resp, 8, 4);

		card->cid.year += 2000;	/* SD cards year offset */
	} else {
		/*
		 * The selection of the format here is based upon published
		 * specs from sandisk and from what people have reported.
		 */
		switch (card->csd.mmca_vsn) {
		case 0:	/* MMC v1.0 - v1.2 */
		case 1:	/* MMC v1.4 */
			card->cid.manfid = UNSTUFF_BITS(resp, 104, 24);
			card->cid.prod_name[0] = UNSTUFF_BITS(resp, 96, 8);
			card->cid.prod_name[1] = UNSTUFF_BITS(resp, 88, 8);
			card->cid.prod_name[2] = UNSTUFF_BITS(resp, 80, 8);
			card->cid.prod_name[3] = UNSTUFF_BITS(resp, 72, 8);
			card->cid.prod_name[4] = UNSTUFF_BITS(resp, 64, 8);
			card->cid.prod_name[5] = UNSTUFF_BITS(resp, 56, 8);
			card->cid.prod_name[6] = UNSTUFF_BITS(resp, 48, 8);
			card->cid.hwrev = UNSTUFF_BITS(resp, 44, 4);
			card->cid.fwrev = UNSTUFF_BITS(resp, 40, 4);
			card->cid.serial = UNSTUFF_BITS(resp, 16, 24);
			card->cid.month = UNSTUFF_BITS(resp, 12, 4);
			card->cid.year = UNSTUFF_BITS(resp, 8, 4) + 1997;
			break;

		case 2:	/* MMC v2.0 - v2.2 */
		case 3:	/* MMC v3.1 - v3.3 */
		case 4:	/* MMC v4 */
			card->cid.manfid = UNSTUFF_BITS(resp, 120, 8);
			/* card->cid.cbx           = UNSTUFF_BITS(resp, 112, 2); */
			card->cid.oemid = UNSTUFF_BITS(resp, 104, 16);
			card->cid.prod_name[0] = UNSTUFF_BITS(resp, 96, 8);
			card->cid.prod_name[1] = UNSTUFF_BITS(resp, 88, 8);
			card->cid.prod_name[2] = UNSTUFF_BITS(resp, 80, 8);
			card->cid.prod_name[3] = UNSTUFF_BITS(resp, 72, 8);
			card->cid.prod_name[4] = UNSTUFF_BITS(resp, 64, 8);
			card->cid.prod_name[5] = UNSTUFF_BITS(resp, 56, 8);
			card->cid.serial = UNSTUFF_BITS(resp, 16, 32);
			card->cid.month = UNSTUFF_BITS(resp, 12, 4);
			card->cid.year = UNSTUFF_BITS(resp, 8, 4) + 1997;
			break;

		default:
			DEBUG_MMC(BIOS_DEBUG, "[SD%d] Unknown MMCA version %d\n",
				  mmc_card_id(card), card->csd.mmca_vsn);
			break;
		}
	}
}

#ifndef FEATURE_MMC_STRIPPED

static int mmc_decode_csd(struct mmc_card *card)
{
	struct mmc_csd *csd = &card->csd;
	unsigned int e, m, csd_struct;
	u32 *resp = card->raw_csd;

	/* common part */
	csd_struct = UNSTUFF_BITS(resp, 126, 2);
	csd->csd_struct = csd_struct;

	m = UNSTUFF_BITS(resp, 99, 4);
	e = UNSTUFF_BITS(resp, 96, 3);
	csd->max_dtr = tran_exp[e] * tran_mant[m];
	csd->cmdclass = UNSTUFF_BITS(resp, 84, 12);

	csd->write_prot_grpsz = UNSTUFF_BITS(resp, 32, 7);
	csd->write_prot_grp = UNSTUFF_BITS(resp, 31, 1);
	csd->perm_wr_prot = UNSTUFF_BITS(resp, 13, 1);
	csd->tmp_wr_prot = UNSTUFF_BITS(resp, 12, 1);
	csd->copy = UNSTUFF_BITS(resp, 14, 1);
	csd->dsr = UNSTUFF_BITS(resp, 76, 1);

	/* update later according to spec. */
	csd->read_blkbits = UNSTUFF_BITS(resp, 80, 4);
	csd->read_partial = UNSTUFF_BITS(resp, 79, 1);
	csd->write_misalign = UNSTUFF_BITS(resp, 78, 1);
	csd->read_misalign = UNSTUFF_BITS(resp, 77, 1);
	csd->r2w_factor = UNSTUFF_BITS(resp, 26, 3);
	csd->write_blkbits = UNSTUFF_BITS(resp, 22, 4);
	csd->write_partial = UNSTUFF_BITS(resp, 21, 1);

	m = UNSTUFF_BITS(resp, 115, 4);
	e = UNSTUFF_BITS(resp, 112, 3);
	csd->tacc_ns = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
	csd->tacc_clks = UNSTUFF_BITS(resp, 104, 8) * 100;

	e = UNSTUFF_BITS(resp, 47, 3);
	m = UNSTUFF_BITS(resp, 62, 12);
	csd->capacity = (1 + m) << (e + 2);

	if (mmc_card_sd(card)) {
		csd->erase_blk_en = UNSTUFF_BITS(resp, 46, 1);
		csd->erase_sctsz = UNSTUFF_BITS(resp, 39, 7) + 1;

		switch (csd_struct) {
		case 0:
			break;
		case 1:
			/*
			 * This is a block-addressed SDHC card. Most
			 * interesting fields are unused and have fixed
			 * values. To avoid getting tripped by buggy cards,
			 * we assume those fixed values ourselves.
			 */
			mmc_card_set_blockaddr(card);

			csd->tacc_ns = 0;	/* Unused */
			csd->tacc_clks = 0;	/* Unused */

			m = UNSTUFF_BITS(resp, 48, 22);
			csd->capacity = (1 + m) << 10;

			csd->read_blkbits = 9;
			csd->read_partial = 0;
			csd->write_misalign = 0;
			csd->read_misalign = 0;
			csd->r2w_factor = 4;	/* Unused */
			csd->write_blkbits = 9;
			csd->write_partial = 0;
			break;
		default:
			DEBUG_MMC(BIOS_DEBUG, "[SD%d] Unknown CSD ver %d\n",
				  mmc_card_id(card), csd_struct);
			return MMC_ERR_INVALID;
		}
	} else {
		/*
		 * We only understand CSD structure v1.1 and v1.2.
		 * v1.2 has extra information in bits 15, 11 and 10.
		 */
		if (csd_struct != CSD_STRUCT_VER_1_0
		    && csd_struct != CSD_STRUCT_VER_1_1
		    && csd_struct != CSD_STRUCT_VER_1_2
		    && csd_struct != CSD_STRUCT_EXT_CSD) {
			DEBUG_MMC(BIOS_DEBUG, "[SD%d] Unknown CSD ver %d\n",
				  mmc_card_id(card), csd_struct);
			return MMC_ERR_INVALID;
		}

		csd->mmca_vsn = UNSTUFF_BITS(resp, 122, 4);
		csd->erase_sctsz = (UNSTUFF_BITS(resp, 42, 5) + 1) *
				   (UNSTUFF_BITS(resp, 37, 5) + 1);
	}

	return 0;
}

#else
static int mmc_decode_csd(struct mmc_card *card)
{
	struct mmc_csd *csd = &card->csd;
	unsigned int e, m, csd_struct;
	u32 *resp = card->raw_csd;

	/* common part */
	csd_struct = UNSTUFF_BITS(resp, 126, 2);
	csd->csd_struct = csd_struct;

	/* update later according to spec. */
	m = UNSTUFF_BITS(resp, 99, 4);
	e = UNSTUFF_BITS(resp, 96, 3);
	csd->max_dtr = tran_exp[e] * tran_mant[m];

	e = UNSTUFF_BITS(resp, 47, 3);
	m = UNSTUFF_BITS(resp, 62, 12);
	csd->capacity = (1 + m) << (e + 2);
	csd->read_blkbits = UNSTUFF_BITS(resp, 80, 4);

	if (mmc_card_sd(card)) {
		switch (csd_struct) {
		case 0:
			break;
		case 1:
			/*
			 * This is a block-addressed SDHC card. Most
			 * interesting fields are unused and have fixed
			 * values. To avoid getting tripped by buggy cards,
			 * we assume those fixed values ourselves.
			 */
			mmc_card_set_blockaddr(card);

			m = UNSTUFF_BITS(resp, 48, 22);
			csd->capacity = (1 + m) << 10;
			csd->read_blkbits = 9;
			break;
		default:
			DEBUG_MMC(BIOS_DEBUG, "[SD%d] Unknown CSD ver %d\n",
				  mmc_card_id(card), csd_struct);
			return MMC_ERR_INVALID;
		}

	} else {
		/*
		 * We only understand CSD structure v1.1 and v1.2.
		 * v1.2 has extra information in bits 15, 11 and 10.
		 */
		if (csd_struct != CSD_STRUCT_VER_1_0
		    && csd_struct != CSD_STRUCT_VER_1_1
		    && csd_struct != CSD_STRUCT_VER_1_2
		    && csd_struct != CSD_STRUCT_EXT_CSD) {
			DEBUG_MMC(BIOS_DEBUG, "[SD%d] Unknown CSD ver %d\n",
				  mmc_card_id(card), csd_struct);
			return MMC_ERR_INVALID;
		}

		csd->mmca_vsn = UNSTUFF_BITS(resp, 122, 4);
	}

	return 0;
}
#endif

static void mmc_decode_ext_csd(struct mmc_card *card)
{
	u8 *ext_csd = &card->raw_ext_csd[0];

	card->ext_csd.sectors = ext_csd[EXT_CSD_SEC_CNT + 0] << 0 |
				ext_csd[EXT_CSD_SEC_CNT + 1] << 8 |
				ext_csd[EXT_CSD_SEC_CNT + 2] << 16 |
				ext_csd[EXT_CSD_SEC_CNT + 3] << 24;

#ifndef FEATURE_MMC_STRIPPED
	card->ext_csd.rev = ext_csd[EXT_CSD_REV];
	card->ext_csd.hc_erase_grp_sz =
		ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 512 * 1024;
	card->ext_csd.hc_wp_grp_sz =
		ext_csd[EXT_CSD_HC_WP_GPR_SIZE] *
		ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 512 * 1024;
	card->ext_csd.trim_tmo_ms = ext_csd[EXT_CSD_TRIM_MULT] * 300;
	card->ext_csd.boot_info = ext_csd[EXT_CSD_BOOT_INFO];
	card->ext_csd.boot_part_sz =
		ext_csd[EXT_CSD_BOOT_SIZE_MULT] * 128 * 1024;
	card->ext_csd.access_sz = (ext_csd[EXT_CSD_ACC_SIZE] & 0xf) * 512;
	card->ext_csd.rpmb_sz = ext_csd[EXT_CSD_RPMB_SIZE_MULT] * 128 * 1024;
	card->ext_csd.erased_mem_cont = ext_csd[EXT_CSD_ERASED_MEM_CONT];
	card->ext_csd.part_en =
		ext_csd[EXT_CSD_PART_SUPPORT] & EXT_CSD_PART_SUPPORT_PART_EN ?
		1 : 0;
	card->ext_csd.enh_attr_en =
		ext_csd[EXT_CSD_PART_SUPPORT] & EXT_CSD_PART_SUPPORT_ENH_ATTR_EN ?
		1 : 0;
	card->ext_csd.enh_start_addr =
		(ext_csd[EXT_CSD_ENH_START_ADDR + 0] |
		ext_csd[EXT_CSD_ENH_START_ADDR + 1] << 8 |
		ext_csd[EXT_CSD_ENH_START_ADDR + 2] << 16 |
		ext_csd[EXT_CSD_ENH_START_ADDR + 3] << 24);
	card->ext_csd.enh_sz =
		(ext_csd[EXT_CSD_ENH_SIZE_MULT + 0] |
			ext_csd[EXT_CSD_ENH_SIZE_MULT + 1] << 8 |
			ext_csd[EXT_CSD_ENH_SIZE_MULT + 2] << 16) *
		512 * 1024 * ext_csd[EXT_CSD_HC_WP_GPR_SIZE] *
		ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE];
#endif

	if (card->ext_csd.sectors)
		mmc_card_set_blockaddr(card);

	if ((ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_DDR_52_1_2V) ||
	    (ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_DDR_52)) {
		card->ext_csd.ddr_support = 1;
		card->ext_csd.hs_max_dtr = 52000000;
	} else if (ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_52) {
		card->ext_csd.hs_max_dtr = 52000000;
	} else if ((ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_26)) {
		card->ext_csd.hs_max_dtr = 26000000;
	} else {
		/* MMC v4 spec says this cannot happen */
		DEBUG_MMC(BIOS_DEBUG, "[SD%d] MMCv4 but HS unsupported\n",
			  card->host->id);
	}

	return;
}

int mmc_select_card(struct mmc_host *host, struct mmc_card *card)
{
	int err;
	struct mmc_command cmd;

	cmd.opcode = MMC_CMD_SELECT_CARD;
	cmd.arg = card->rca << 16;
	cmd.rsptyp = RESP_R1B;
	cmd.retries = CMD_RETRIES;
	cmd.timeout = CMD_TIMEOUT;

	err = mmc_cmd(host, &cmd);

	return err;
}

static int mmc_send_relative_addr(struct mmc_host *host, struct mmc_card *card,
				  unsigned int *rca)
{
	int err;
	struct mmc_command cmd;

	memset(&cmd, 0, sizeof(struct mmc_command));

	if (mmc_card_mmc(card)) {	/* set rca */
		cmd.opcode = MMC_CMD_SET_RELATIVE_ADDR;
		cmd.arg = *rca << 16;
		cmd.rsptyp = RESP_R1;
		cmd.retries = CMD_RETRIES;
		cmd.timeout = CMD_TIMEOUT;
	} else {		/* send rca */
		cmd.opcode = SD_CMD_SEND_RELATIVE_ADDR;
		cmd.arg = 0;
		cmd.rsptyp = RESP_R6;
		cmd.retries = CMD_RETRIES;
		cmd.timeout = CMD_TIMEOUT;
	}
	err = mmc_cmd(host, &cmd);
	if ((err == MMC_ERR_NONE) && !mmc_card_mmc(card))
		*rca = cmd.resp[0] >> 16;

	return err;
}

int mmc_switch(struct mmc_host *host, struct mmc_card *card,
	       u8 set, u8 index, u8 value)
{
	int err;
	u32 status;
	struct mmc_command cmd;

	cmd.opcode = MMC_CMD_SWITCH;
	cmd.arg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
	    (index << 16) | (value << 8) | set;
	cmd.rsptyp = RESP_R1B;
	cmd.retries = CMD_RETRIES;
	cmd.timeout = CMD_TIMEOUT;

	err = mmc_cmd(host, &cmd);

	if (err != MMC_ERR_NONE)
		return err;

	do {
		err = mmc_send_status(host, card, &status);
		if (err) {
			DEBUG_MMC(BIOS_DEBUG, "[SD%d] Fail to send status %d\n",
				  host->id, err);
			break;
		}
		if (status & R1_SWITCH_ERROR) {
			DEBUG_MMC(BIOS_DEBUG, "[SD%d] switch error. arg(0x%x)\n",
				  host->id, cmd.arg);
			return MMC_ERR_FAILED;
		}
	} while (!(status & R1_READY_FOR_DATA)
		 || (R1_CURRENT_STATE(status) == 7));

	return err;
}

static int mmc_sd_switch(struct mmc_host *host,
			 struct mmc_card *card,
			 int mode, int group, u8 value, mmc_switch_t *resp)
{
	int err = MMC_ERR_FAILED;
	int result = 0;
	struct mmc_command cmd;
	u32 *sts = (u32 *)resp;
	int retries;

	mode = !!mode;
	value &= 0xF;

	/* argument: mode[31]= 0 (for check func.) and 1 (for switch func) */
	cmd.opcode = SD_CMD_SWITCH;
	cmd.arg = mode << 31 | 0x00FFFFFF;
	cmd.arg &= ~(0xF << (group * 4));
	cmd.arg |= value << (group * 4);
	cmd.rsptyp = RESP_R1;
	cmd.retries = CMD_RETRIES;
	cmd.timeout = 100;	/* 100ms */
	msdc_reset_tune_counter(host);
	do {
		msdc_set_blknum(host, 1);
		msdc_set_blklen(host, 64);
		msdc_set_timeout(host, 100000000, 0);
		err = mmc_cmd(host, &cmd);

		if (err != MMC_ERR_NONE)
			goto out;

		retries = 50000;

		/* 512 bits = 64 bytes = 16 words */
		err = msdc_pio_read(host, sts, 64);
		if (err != MMC_ERR_NONE) {
			if (msdc_abort_handler(host, 1))
				DEBUG_MMC(BIOS_DEBUG,
					  "[SD%d] data abort failed\n", host->id);
			result = MMC_ERR_FAILED;
		}
	} while (err && result != MMC_ERR_READTUNEFAIL);
	msdc_reset_tune_counter(host);

out:
	return err;
}

static int mmc_switch_hs(struct mmc_host *host, struct mmc_card *card)
{
	int err;
	u8 status[64];
	int val = MMC_SWITCH_MODE_SDR25;

	err =
	    mmc_sd_switch(host, card, 1, 0, val, (mmc_switch_t *)&status[0]);

	if (err != MMC_ERR_NONE)
		goto out;

	if ((status[16] & 0xF) != 1) {
		DEBUG_MMC(BIOS_DEBUG, "[SD%d] HS mode not supported!\n",
			  host->id);
		err = MMC_ERR_FAILED;
	} else {
		DEBUG_MMC(BIOS_DEBUG, "[SD%d] Switch to HS mode!\n", host->id);
		mmc_card_set_highspeed(card);
	}

out:
	return err;
}

static int mmc_read_csds(struct mmc_host *host, struct mmc_card *card)
{
	int err;
	struct mmc_command cmd;

	cmd.opcode = MMC_CMD_SEND_CSD;
	cmd.arg = card->rca << 16;
	cmd.rsptyp = RESP_R2;
	cmd.retries = CMD_RETRIES;
	cmd.timeout = CMD_TIMEOUT * 100;

	err = mmc_cmd(host, &cmd);
	if (err == MMC_ERR_NONE)
		memcpy(&card->raw_csd, &cmd.resp[0], sizeof(u32) * 4);
	return err;
}

#ifndef FEATURE_MMC_STRIPPED
static int mmc_read_scrs(struct mmc_host *host, struct mmc_card *card)
{
	int err = MMC_ERR_NONE;
	int result;
	int retries;
	struct mmc_command cmd;
	struct sd_scr *scr = &card->scr;
	u32 resp[4];
	u32 tmp;

	msdc_set_blknum(host, 1);
	msdc_set_blklen(host, 8);
	msdc_set_timeout(host, 100000000, 0);

	cmd.opcode = SD_ACMD_SEND_SCR;
	cmd.arg = 0;
	cmd.rsptyp = RESP_R1;
	cmd.retries = CMD_RETRIES;
	cmd.timeout = CMD_TIMEOUT;
	msdc_reset_tune_counter(host);
	do {
		mmc_app_cmd(host, &cmd, card->rca, CMD_RETRIES);
		if ((err != MMC_ERR_NONE) || !(cmd.resp[0] & R1_APP_CMD))
			return MMC_ERR_FAILED;

		retries = 50000;

		/* 8 bytes = 2 words */
		err = msdc_pio_read(host, card->raw_scr, 8);
		if (err != MMC_ERR_NONE) {
			if (msdc_abort_handler(host, 1))
				DEBUG_MMC(BIOS_DEBUG,
					  "[SD%d] data abort failed\n", host->id);
			result = MMC_ERR_FAILED;
		}
	} while (err && result != MMC_ERR_READTUNEFAIL);
	msdc_reset_tune_counter(host);
	MSG(INF, "[SD%d] SCR: %x %x (raw)\n", host->id, card->raw_scr[0],
	    card->raw_scr[1]);

	tmp = ntohl(card->raw_scr[0]);
	card->raw_scr[0] = ntohl(card->raw_scr[1]);
	card->raw_scr[1] = tmp;

	MSG(INF, "[SD%d] SCR: %x %x (ntohl)\n", host->id, card->raw_scr[0],
	    card->raw_scr[1]);

	resp[2] = card->raw_scr[1];
	resp[3] = card->raw_scr[0];

	if (UNSTUFF_BITS(resp, 60, 4) != 0) {
		DEBUG_MMC(BIOS_DEBUG, "[SD%d] Unknown SCR ver %d\n",
			  mmc_card_id(card), UNSTUFF_BITS(resp, 60, 4));
		return MMC_ERR_INVALID;
	}

	scr->scr_struct = UNSTUFF_BITS(resp, 60, 4);
	scr->sda_vsn = UNSTUFF_BITS(resp, 56, 4);
	scr->data_bit_after_erase = UNSTUFF_BITS(resp, 55, 1);
	scr->security = UNSTUFF_BITS(resp, 52, 3);
	scr->bus_widths = UNSTUFF_BITS(resp, 48, 4);
	scr->sda_vsn3 = UNSTUFF_BITS(resp, 47, 1);
	scr->ex_security = UNSTUFF_BITS(resp, 43, 4);
	scr->cmd_support = UNSTUFF_BITS(resp, 32, 2);
	DEBUG_MMC(BIOS_DEBUG, "[SD%d] SD_SPEC(%d) SD_SPEC3(%d) SD_BUS_WIDTH=%d\n",
		  mmc_card_id(card), scr->sda_vsn, scr->sda_vsn3,
		scr->bus_widths);
	DEBUG_MMC(BIOS_DEBUG,
		  "[SD%d] SD_SECU(%d) EX_SECU(%d), CMD_SUPP(%d): CMD23(%d), CMD20(%d)\n",
		mmc_card_id(card), scr->security, scr->ex_security,
		scr->cmd_support, (scr->cmd_support >> 1) & 0x1,
		scr->cmd_support & 0x1);
	return err;
}
#endif

/* Read and decode extended CSD. */
int mmc_read_ext_csd(struct mmc_host *host, struct mmc_card *card)
{
	int err = MMC_ERR_NONE;
	u32 *ptr;
	int result = MMC_ERR_NONE;
	struct mmc_command cmd;

	if (card->csd.mmca_vsn < CSD_SPEC_VER_4) {
		DEBUG_MMC(BIOS_DEBUG, "[SD%d] MMCA_VSN: %d. Skip EXT_CSD\n",
			  host->id, card->csd.mmca_vsn);
		return MMC_ERR_NONE;
	}

	/*
	 * As the ext_csd is so large and mostly unused, we don't store the
	 * raw block in mmc_card.
	 */
	memset(&card->raw_ext_csd[0], 0, 512);
	ptr = (u32 *)&card->raw_ext_csd[0];

	cmd.opcode = MMC_CMD_SEND_EXT_CSD;
	cmd.arg = 0;
	cmd.rsptyp = RESP_R1;
	cmd.retries = CMD_RETRIES;
	cmd.timeout = CMD_TIMEOUT;
	msdc_reset_tune_counter(host);
	do {
		msdc_set_blknum(host, 1);
		msdc_set_blklen(host, 512);
		msdc_set_timeout(host, 100000000, 0);
		err = mmc_cmd(host, &cmd);
		if (err != MMC_ERR_NONE)
			goto out;

		err = msdc_pio_read(host, ptr, 512);
		if (err != MMC_ERR_NONE) {
			if (msdc_abort_handler(host, 1))
				DEBUG_MMC(BIOS_DEBUG,
					  "[SD%d] data abort failed\n", host->id);
			result = MMC_ERR_FAILED;
		}
	} while (err && result != MMC_ERR_READTUNEFAIL);
	msdc_reset_tune_counter(host);
	mmc_decode_ext_csd(card);

out:
	return err;
}

/* Fetches and decodes switch information */
static int mmc_read_switch(struct mmc_host *host, struct mmc_card *card)
{
	int err;
	u8 status[64];

	err = mmc_sd_switch(host, card, 0, 0, 1, (mmc_switch_t *)&status[0]);
	if (err != MMC_ERR_NONE) {
		/* Card not supporting high-speed will ignore the command. */
		err = MMC_ERR_NONE;
		goto out;
	}

	/* bit 511:480 in status[0]. bit 415:400 in status[13] */
	if (status[13] & 0x01) {
		DEBUG_MMC(BIOS_DEBUG, "[SD%d] Support: Default/SDR12\n",
			  host->id);
		card->sw_caps.hs_max_dtr = 25000000;	/* default or sdr12 */
	}
	if (status[13] & 0x02) {
		DEBUG_MMC(BIOS_DEBUG, "[SD%d] Support: HS/SDR25\n", host->id);
		card->sw_caps.hs_max_dtr = 50000000;	/* high-speed or sdr25 */
	}
	if (status[13] & 0x10) {
		DEBUG_MMC(BIOS_DEBUG, "[SD%d] Support: DDR50\n", host->id);
		card->sw_caps.hs_max_dtr = 50000000;	/* ddr50 */
		card->sw_caps.ddr = 1;
	}

out:
	return err;
}

int mmc_erase_start(struct mmc_card *card, u32 addr)
{
	struct mmc_command cmd;

	if (!(card->csd.cmdclass & CCC_ERASE)) {
		DEBUG_MMC(BIOS_DEBUG,
			  "[SD%d] Card doesn't support Erase commands\n",
			card->host->id);
		return MMC_ERR_INVALID;
	}

	if (mmc_card_highcaps(card))
		addr /= MMC_BLOCK_SIZE;	/* in sector unit */

	if (mmc_card_mmc(card))
		cmd.opcode = MMC_CMD_ERASE_GROUP_START;
	else
		cmd.opcode = MMC_CMD_ERASE_WR_BLK_START;

	cmd.rsptyp = RESP_R1;
	cmd.arg = addr;
	cmd.retries = 3;
	cmd.timeout = CMD_TIMEOUT;

	return mmc_cmd(card->host, &cmd);
}

int mmc_erase_end(struct mmc_card *card, u32 addr)
{
	struct mmc_command cmd;

	if (!(card->csd.cmdclass & CCC_ERASE)) {
		DEBUG_MMC(BIOS_DEBUG,
			  "[SD%d] Erase isn't supported\n", card->host->id);
		return MMC_ERR_INVALID;
	}

	if (mmc_card_highcaps(card))
		addr /= MMC_BLOCK_SIZE;	/* in sector unit */

	if (mmc_card_mmc(card))
		cmd.opcode = MMC_CMD_ERASE_GROUP_END;
	else
		cmd.opcode = MMC_CMD_ERASE_WR_BLK_END;

	cmd.rsptyp = RESP_R1;
	cmd.arg = addr;
	cmd.retries = 3;
	cmd.timeout = CMD_TIMEOUT;

	return mmc_cmd(card->host, &cmd);
}

int mmc_erase(struct mmc_card *card, u32 arg)
{
	int err;
	u32 status;
	struct mmc_command cmd;

	if (!(card->csd.cmdclass & CCC_ERASE)) {
		DEBUG_MMC(BIOS_DEBUG,
			  "[SD%d] Erase isn't supported\n", card->host->id);
		return MMC_ERR_INVALID;
	}

	if (arg & MMC_ERASE_SECURE_REQ) {
		if (!(card->raw_ext_csd[EXT_CSD_SEC_FEATURE_SUPPORT] &
		      EXT_CSD_SEC_FEATURE_ER_EN)) {
			return MMC_ERR_INVALID;
		}
	}
	if ((arg & MMC_ERASE_GC_REQ) || (arg & MMC_ERASE_TRIM)) {
		if (!(card->raw_ext_csd[EXT_CSD_SEC_FEATURE_SUPPORT] &
		      EXT_CSD_SEC_FEATURE_GB_CL_EN)) {
			return MMC_ERR_INVALID;
		}
	}

	cmd.opcode = MMC_CMD_ERASE;
	cmd.rsptyp = RESP_R1B;
	cmd.arg = arg;
	cmd.retries = 3;
	cmd.timeout = CMD_TIMEOUT;

	err = mmc_cmd(card->host, &cmd);

	if (!err) {
		do {
			err = mmc_send_status(card->host, card, &status);
			if (err)
				break;
			if (R1_STATUS(status) != 0)
				break;
		} while (R1_CURRENT_STATE(status) == 7);
	}
	return err;
}

void mmc_set_clock(struct mmc_host *host, int ddr, unsigned int hz)
{
	if (hz >= host->f_max)
		hz = host->f_max;
	else if (hz < host->f_min)
		hz = host->f_min;

	msdc_config_clock(host, ddr > 0 ? 1 : 0, hz);
}

static int mmc_set_card_detect(struct mmc_host *host, struct mmc_card *card,
			       int connect)
{
	int err;
	struct mmc_command cmd;

	cmd.opcode = SD_ACMD_SET_CLR_CD;
	cmd.arg = connect;
	cmd.rsptyp = RESP_R1;	/* CHECKME */
	cmd.retries = CMD_RETRIES;
	cmd.timeout = CMD_TIMEOUT;

	err = mmc_app_cmd(host, &cmd, card->rca, CMD_RETRIES);
	return err;
}

int mmc_set_blk_length(struct mmc_host *host, u32 blklen)
{
	int err;
	struct mmc_command cmd;

	/* set block len */
	cmd.opcode = MMC_CMD_SET_BLOCKLEN;
	cmd.rsptyp = RESP_R1;
	cmd.arg = blklen;
	cmd.retries = 3;
	cmd.timeout = CMD_TIMEOUT;
	err = mmc_cmd(host, &cmd);

	if (err == MMC_ERR_NONE)
		msdc_set_blklen(host, blklen);

	return err;
}

int mmc_set_bus_width(struct mmc_host *host, struct mmc_card *card, int width)
{
	int err = MMC_ERR_NONE;
	u32 arg;
	struct mmc_command cmd;

	if (mmc_card_sd(card)) {
		if (width == HOST_BUS_WIDTH_8) {
			WARN_ON(width == HOST_BUS_WIDTH_8);
			width = HOST_BUS_WIDTH_4;
		}

		if ((width == HOST_BUS_WIDTH_4)
		    && (host->caps & MMC_CAP_4_BIT_DATA)) {
			arg = SD_BUS_WIDTH_4;
		} else {
			arg = SD_BUS_WIDTH_1;
			width = HOST_BUS_WIDTH_1;
		}

		cmd.opcode = SD_ACMD_SET_BUSWIDTH;
		cmd.arg = arg;
		cmd.rsptyp = RESP_R1;
		cmd.retries = CMD_RETRIES;
		cmd.timeout = CMD_TIMEOUT;

		err = mmc_app_cmd(host, &cmd, card->rca, 0);
		if (err != MMC_ERR_NONE)
			goto out;

		msdc_config_bus(host, width);
	} else if (mmc_card_mmc(card)) {
		if (card->csd.mmca_vsn < CSD_SPEC_VER_4)
			goto out;

		if (width == HOST_BUS_WIDTH_8) {
			if (host->caps & MMC_CAP_8_BIT_DATA) {
				arg = ((host->caps & MMC_CAP_DDR)
				       && card->ext_csd.ddr_support) ?
				       EXT_CSD_BUS_WIDTH_8_DDR :
				       EXT_CSD_BUS_WIDTH_8;
			} else {
				width = HOST_BUS_WIDTH_4;
			}
		}
		if (width == HOST_BUS_WIDTH_4) {
			if (host->caps & MMC_CAP_4_BIT_DATA) {
				arg = ((host->caps & MMC_CAP_DDR)
				       && card->ext_csd.ddr_support) ?
				       EXT_CSD_BUS_WIDTH_4_DDR :
				       EXT_CSD_BUS_WIDTH_4;
			} else {
				width = HOST_BUS_WIDTH_1;
			}
		}
		if (width == HOST_BUS_WIDTH_1) {
			arg = EXT_CSD_BUS_WIDTH_1;
		} else {
			goto out;
		}

		err = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL,
				 EXT_CSD_BUS_WIDTH, arg);
		if (err != MMC_ERR_NONE) {
			DEBUG_MMC(BIOS_DEBUG,
				  "[SD%d] Switch to bus width(%d) failed\n",
				host->id, arg);
			goto out;
		}
		if (arg == EXT_CSD_BUS_WIDTH_8_DDR ||
		    arg == EXT_CSD_BUS_WIDTH_4_DDR) {
			mmc_card_set_ddr(card);
		} else {
			card->state &= ~MMC_STATE_DDR;
		}
		mmc_set_clock(host, mmc_card_ddr(card), host->sclk);

		msdc_config_bus(host, width);
	} else {
		BUG();		/* card is not recognized */
	}
out:
	return err;
}

static int mmc_dev_bread(struct mmc_card *card, unsigned long blknr, u32 blkcnt,
			 u8 *dst)
{
	struct mmc_host *host = card->host;
	u32 blksz = host->blklen;
	int tune = 0;
	int retry = 3;
	int err;
	unsigned long src;

	src = mmc_card_highcaps(card) ? blknr : blknr * blksz;

	do {
		if (!tune) {
			err = host->blk_read(host, (uint8_t *)dst, src, blkcnt);
		} else {
			if (err && (host->sclk > (host->f_max >> 4)))
				mmc_set_clock(host, mmc_card_ddr(card),
					      host->sclk >> 1);
		}
		if (err == MMC_ERR_NONE)
			break;

		if (err == MMC_ERR_BADCRC || err == MMC_ERR_ACMD_RSPCRC
		    || err == MMC_ERR_CMD_RSPCRC) {
			tune = 1;
			retry++;
		} else if (err == MMC_ERR_READTUNEFAIL
			   || err == MMC_ERR_CMDTUNEFAIL) {
			DEBUG_MMC(BIOS_DEBUG, "[SD%d] Fail to tuning,%s",
				  host->id,
				(err == MMC_ERR_CMDTUNEFAIL) ?
				"cmd tune failed!\n" : "read tune failed!\n");
			break;
		}
	} while (retry--);

	return err;
}

static int mmc_dev_bwrite(struct mmc_card *card, unsigned long blknr,
			  u32 blkcnt, u8 *src)
{
	struct mmc_host *host = card->host;
	u32 blksz = host->blklen;
	u32 status;
	int tune = 0;
	int retry = 3;
	int err;
	unsigned long dst;

	dst = mmc_card_highcaps(card) ? blknr : blknr * blksz;

	do {
		if (!tune) {
			err = host->blk_write(host, dst, (uint8_t *)src,
					      blkcnt);
		} else {
			if (err && (host->sclk > (host->f_max >> 4)))
				mmc_set_clock(host, mmc_card_ddr(card),
					      host->sclk >> 1);
		}
		if (err == MMC_ERR_NONE) {
			do {
				err = mmc_send_status(host, card, &status);
				if (err) {
					DEBUG_MMC(BIOS_DEBUG,
						  "[SD%d] Fail to send status %d\n",
						host->id, err);
					break;
				}
			} while (!(status & R1_READY_FOR_DATA) ||
				 (R1_CURRENT_STATE(status) == 7));

			MSG(OPS, "[SD%d] Write %d bytes (DONE)\n",
			    host->id, blkcnt * blksz);
			break;
		}

		if (err == MMC_ERR_BADCRC || err == MMC_ERR_ACMD_RSPCRC
		    || err == MMC_ERR_CMD_RSPCRC) {
			tune = 1;
			retry++;
		} else if (err == MMC_ERR_WRITETUNEFAIL
			   || err == MMC_ERR_CMDTUNEFAIL) {
			DEBUG_MMC(BIOS_DEBUG, "[SD%d] Fail to tuning,%s",
				  host->id, (err == MMC_ERR_CMDTUNEFAIL) ?
				"cmd tune failed!\n" : "write tune failed!\n");
			break;
		}
	} while (retry--);

	return err;
}

int mmc_block_read(int dev_num, unsigned long blknr, u32 blkcnt,
		   unsigned long *dst)
{
	struct mmc_host *host = mmc_get_host(dev_num);
	struct mmc_card *card = mmc_get_card(dev_num);
	u32 blksz = host->blklen;
	u32 maxblks = host->max_phys_segs;
	u32 xfercnt = blkcnt / maxblks;
	u32 leftblks = blkcnt % maxblks;
	u32 i;
	u8 *buf = (u8 *)dst;
	int ret;

	if (!blkcnt)
		return MMC_ERR_NONE;

	if (blknr * (blksz / MMC_BLOCK_SIZE) > card->nblks) {
		DEBUG_MMC(BIOS_DEBUG,
			  "[SD%d] Out of block range: blknr(%d) > sd_blknr(%d)\n",
			host->id, blknr, card->nblks);
		return MMC_ERR_INVALID;
	}

	for (i = xfercnt; i != 0; i--) {
		ret = mmc_dev_bread(card, (unsigned long)blknr, maxblks, buf);
		if (ret)
			return ret;
		blknr += maxblks;
		buf += maxblks * blksz;
	}
	if (leftblks)
		ret = mmc_dev_bread(card, (unsigned long)blknr, leftblks, buf);

	return ret;
}

int mmc_block_write(int dev_num, unsigned long blknr, u32 blkcnt,
		    unsigned long *src)
{
	struct mmc_host *host = mmc_get_host(dev_num);
	struct mmc_card *card = mmc_get_card(dev_num);
	u32 blksz = host->blklen;
	u32 maxblks = host->max_phys_segs;
	u32 xfercnt = blkcnt / maxblks;
	u32 leftblks = blkcnt % maxblks;
	u32 i;
	u8 *buf = (u8 *)src;
	int ret;

	if (!blkcnt)
		return MMC_ERR_NONE;

	if (blknr * (blksz / MMC_BLOCK_SIZE) > card->nblks) {
		DEBUG_MMC(BIOS_DEBUG,
			  "[SD%d] Out of block range: blknr(%d) > sd_blknr(%d)\n",
			host->id, blknr, card->nblks);
		return MMC_ERR_INVALID;
	}

	for (i = xfercnt; i != 0; i--) {
		ret = mmc_dev_bwrite(card, (unsigned long)blknr, maxblks, buf);
		if (ret)
			return ret;
		blknr += maxblks;
		buf += maxblks * blksz;
	}
	if (leftblks)
		ret = mmc_dev_bwrite(card, (unsigned long)blknr, leftblks, buf);

	return ret;
}

static int mmc_init_mem_card(struct mmc_host *host, struct mmc_card *card,
			     u32 ocr)
{
	int err;

	/*
	 * Sanity check the voltages that the card claims to
	 * support.
	 */
	if (ocr & 0x7F)
		ocr &= ~0x7F;

	ocr = host->ocr = mmc_select_voltage(host, ocr);

	/*
	 * Can we support the voltage(s) of the card(s)?
	 */
	if (!host->ocr) {
		err = MMC_ERR_FAILED;
		goto out;
	}

	mmc_go_idle(host);

	/* send interface condition */
	if (mmc_card_sd(card))
		err = mmc_send_if_cond(host, ocr);

	/* host support HCS[30] */
	ocr |= (1 << 30);

	/* send operation condition */
	if (mmc_card_sd(card)) {
		err = mmc_send_app_op_cond(host, ocr, &card->ocr);
	} else {
		/* The extra bit indicates that we support high capacity */
		err = mmc_send_op_cond(host, ocr, &card->ocr);
	}

	if (err != MMC_ERR_NONE) {
		DEBUG_MMC(BIOS_DEBUG, "[SD%d] Fail in SEND_OP_COND cmd\n", id);
		goto out;
	}

	/* set hcs bit if a high-capacity card */
	card->state |= ((card->ocr >> 30) & 0x1) ? MMC_STATE_HIGHCAPS : 0;
	/* send cid */
	err = mmc_all_send_cid(host, card->raw_cid);

	if (err != MMC_ERR_NONE) {
		DEBUG_MMC(BIOS_DEBUG, "[SD%d] Fail in SEND_CID cmd\n", id);
		goto out;
	}

	if (mmc_card_mmc(card))
		card->rca = 0x1;	/* assign a rca */

	/* set/send rca */
	err = mmc_send_relative_addr(host, card, &card->rca);
	if (err != MMC_ERR_NONE) {
		DEBUG_MMC(BIOS_DEBUG, "[SD%d] Fail in SEND_RCA cmd\n", id);
		goto out;
	}

	/* send csd */
	err = mmc_read_csds(host, card);
	if (err != MMC_ERR_NONE) {
		DEBUG_MMC(BIOS_DEBUG, "[SD%d] Fail in SEND_CSD cmd\n", id);
		goto out;
	}

	/* decode csd */
	err = mmc_decode_csd(card);
	if (err != MMC_ERR_NONE) {
		DEBUG_MMC(BIOS_DEBUG, "[SD%d] Fail in decode csd\n", id);
		goto out;
	}

	mmc_decode_cid(card);

	/* select this card */
	err = mmc_select_card(host, card);
	if (err != MMC_ERR_NONE) {
		DEBUG_MMC(BIOS_DEBUG, "[SD%d] Fail in select card cmd\n", id);
		goto out;
	}

	if (mmc_card_sd(card)) {
#ifndef FEATURE_MMC_STRIPPED
		/* send scr */
		err = mmc_read_scrs(host, card);
		if (err != MMC_ERR_NONE) {
			DEBUG_MMC(BIOS_DEBUG, "[SD%d] Fail in SEND_SCR cmd\n",
				  id);
			goto out;
		}
#endif

		if ((card->csd.cmdclass & CCC_SWITCH) &&
		    (mmc_read_switch(host, card) == MMC_ERR_NONE)) {
			do {
				if (host->caps & MMC_CAP_SD_HIGHSPEED) {
					mmc_switch_hs(host, card);
					break;
				}
			} while (0);
		}

		/* set bus width */
		mmc_set_bus_width(host, card, HOST_BUS_WIDTH_4);

		/* compute bus speed. */
		card->maxhz = (unsigned int)-1;

		if (mmc_card_highspeed(card) || mmc_card_uhs1(card)) {
			if (card->maxhz > card->sw_caps.hs_max_dtr)
				card->maxhz = card->sw_caps.hs_max_dtr;
		} else if (card->maxhz > card->csd.max_dtr) {
			card->maxhz = card->csd.max_dtr;
		}
	} else {

		/* send ext csd */
		err = mmc_read_ext_csd(host, card);
		if (err != MMC_ERR_NONE) {
			DEBUG_MMC(BIOS_DEBUG,
				  "[SD%d] Fail in SEND_EXT_CSD cmd\n", id);
			goto out;
		}

		/* activate high speed (if supported) */
		if ((card->ext_csd.hs_max_dtr != 0)
		    && (host->caps & MMC_CAP_MMC_HIGHSPEED)) {
			err = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL,
					 EXT_CSD_HS_TIMING, 1);

			if (err == MMC_ERR_NONE) {
				DEBUG_MMC(BIOS_DEBUG,
					  "[SD%d] Switch to High-Speed mode!\n",
					host->id);
				mmc_card_set_highspeed(card);
			}
		}

		/* set bus width */
		mmc_set_bus_width(host, card, HOST_BUS_WIDTH_8);

		/* compute bus speed. */
		card->maxhz = (unsigned int)-1;

		if (mmc_card_highspeed(card)) {
			if (card->maxhz > card->ext_csd.hs_max_dtr)
				card->maxhz = card->ext_csd.hs_max_dtr;
		} else if (card->maxhz > card->csd.max_dtr) {
			card->maxhz = card->csd.max_dtr;
		}
	}

	/* set block len. note that cmd16 is illegal while mmc card is in ddr mode */
	if (!(mmc_card_mmc(card) && mmc_card_ddr(card))) {
		err = mmc_set_blk_length(host, MMC_BLOCK_SIZE);
		if (err != MMC_ERR_NONE) {
			DEBUG_MMC(BIOS_DEBUG,
				  "[SD%d] Fail in set blklen cmd\n", id);
			goto out;
		}
	}

	/* set clear card detect */
	if (mmc_card_sd(card))
		mmc_set_card_detect(host, card, 0);

	if (!mmc_card_sd(card) && mmc_card_blockaddr(card)) {
		/* The EXT_CSD sector count is in number or 512 byte sectors. */
		card->blklen = MMC_BLOCK_SIZE;
		card->nblks = card->ext_csd.sectors;
	} else {
		/* The CSD capacity field is in units of read_blkbits.
		 * set_capacity takes units of 512 bytes.
		 */
		card->blklen = MMC_BLOCK_SIZE;
		card->nblks = card->csd.capacity << (card->csd.read_blkbits - 9);
	}

	DEBUG_MMC(BIOS_DEBUG, "[SD%d] Size: %d MB, Max.Speed: %d kHz, blklen(%d), nblks(%d), ro(%d)\n",
		  id, ((card->nblks / 1024) * card->blklen) / 1024,
		card->maxhz / 1000, card->blklen, card->nblks,
		mmc_card_readonly(card));

	card->ready = 1;

	DEBUG_MMC(BIOS_DEBUG, "[SD%d] Initialized\n", id);

out:
	return err;
}

static int msdc_ett_offline_to_pl(struct mmc_host *host, struct mmc_card *card)
{
	int ret = 1;
	int size = sizeof(g_mmcTable) / sizeof(struct mmcdev_info);
	int i;
	int temp;
	u32 base = host->base;
	u8 m_id = card->cid.manfid;
	char *pro_name = card->cid.prod_name;

	DEBUG_MMC(BIOS_DEBUG, "msdc_ett_offline_to_pl: size<%d> m_id<0x%x>\n",
		  size, m_id);

	for (i = 0; i < size; i++) {
		DEBUG_MMC(BIOS_DEBUG, "msdc <%d> <%s> <%s>\n",
			  i, g_mmcTable[i].pro_name, pro_name);

		if ((g_mmcTable[i].m_id == m_id)
		    && (!strncmp(g_mmcTable[i].pro_name, pro_name, 6))) {
			DEBUG_MMC(BIOS_DEBUG, "msdc ett index<%d>: <%d> <%d> <0x%x> <0x%x> <0x%x>\n",
				  i, g_mmcTable[i].r_smpl, g_mmcTable[i].d_smpl,
			     g_mmcTable[i].cmd_rxdly, g_mmcTable[i].rd_rxdly,
			     g_mmcTable[i].wr_rxdly);

			/* set to msdc0 */
			MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL,
				       g_mmcTable[i].r_smpl);
			MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL,
				       g_mmcTable[i].d_smpl);

			MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY,
				       g_mmcTable[i].cmd_rxdly);
			MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY,
				       g_mmcTable[i].rd_rxdly);
			MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY,
				       g_mmcTable[i].wr_rxdly);

			temp = g_mmcTable[i].rd_rxdly;
			temp &= 0x1F;
			MSDC_WRITE32(MSDC_DAT_RDDLY0,
				     (temp << 0 | temp << 8 | temp << 16 |
				      temp << 24));
			MSDC_WRITE32(MSDC_DAT_RDDLY1,
				     (temp << 0 | temp << 8 | temp << 16 |
				      temp << 24));

			ret = 0;
			break;
		}
	}

	if (ret)
		DEBUG_MMC(BIOS_DEBUG, "msdc failed to find\n");
	return ret;
}

int mmc_init_card(struct mmc_host *host, struct mmc_card *card)
{
	int err;
	u32 ocr;

	memset(card, 0, sizeof(struct mmc_card));

	mmc_card_set_present(card);
	mmc_card_set_host(card, host);
	mmc_card_set_unknown(card);

	mmc_go_idle(host);

	/* send interface condition */
	mmc_send_if_cond(host, host->ocr_avail);
	/* query operation condition */
	err = mmc_send_app_op_cond(host, 0, &ocr);
	if (err != MMC_ERR_NONE) {
		err = mmc_send_op_cond(host, 0, &ocr);
		if (err != MMC_ERR_NONE) {
			DEBUG_MMC(BIOS_DEBUG,
				  "[SD%d] Fail in SEND_IF_COND cmd\n", id);
			goto out;
		}
		mmc_card_set_mmc(card);
	} else {
		mmc_card_set_sd(card);
	}

	err = mmc_init_mem_card(host, card, ocr);

	if (err)
		goto out;

	/* change clock */
	mmc_set_clock(host, mmc_card_ddr(card), card->maxhz);
	msdc_ett_offline_to_pl(host, card);

out:
	if (err)
		return err;

	host->card = card;
	return 0;
}

int mmc_init_host(struct mmc_host *host, int id)
{
	memset(host, 0, sizeof(struct mmc_host));

	return msdc_init(host, id);
}

int mmc_init(int id)
{
	int err = MMC_ERR_NONE;
	struct mmc_host *host;
	struct mmc_card *card;

	BUG_ON(id >= NR_MMC);

	host = &sd_host[id];
	card = &sd_card[id];
	err = mmc_init_host(host, id);
	if (err == MMC_ERR_NONE)
		err = mmc_init_card(host, card);

	return err;
}
