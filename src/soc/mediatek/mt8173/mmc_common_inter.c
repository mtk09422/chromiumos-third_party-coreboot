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
#include <string.h>
#include <console/console.h>
#include <soc/mt8173.h>

//JH #include <soc/platform.h>
#include <soc/mmc_core.h>
//JH #include <soc/nand_core.h>
//JH #include <soc/dram_buffer.h>

#include <soc/blkdev.h>
#include <soc/mmc_common_inter.h>

#define MMC_HOST_ID                 0
#define BUF_BLK_NUM                 4	/* 4 * 512bytes = 2KB */

/**************************************************************************
*  DEBUG CONTROL
**************************************************************************/

/**************************************************************************
*  MACRO DEFINITION
**************************************************************************/

/**************************************************************************
*  EXTERNAL DECLARATION
**************************************************************************/
//JH extern struct nand_chip g_nand_chip;
//JH #define STORAGE_BUFFER_SIZE 0x10000

//AP added global storage buffer to get rid of dram_buf.h/c
//JH unsigned char g_storage_buffer[STORAGE_BUFFER_SIZE];

static struct blkdev g_mmc_bdev;

enum {
	EMMC_PART_UNKNOWN = 0,
	EMMC_PART_BOOT1,
	EMMC_PART_BOOT2,
	EMMC_PART_RPMB,
	EMMC_PART_GP1,
	EMMC_PART_GP2,
	EMMC_PART_GP3,
	EMMC_PART_GP4,
	EMMC_PART_USER,
	EMMC_PART_END
};

u64 g_emmc_size = 0;
u64 g_emmc_user_size = 0;

static int mmc_switch_part(u32 part_id)
{
	int err = MMC_ERR_NONE;
	struct mmc_card *card;
	struct mmc_host *host;
	u8 part = (u8) part_id;
	u8 cfg;
	u8 *ext_csd;

	card = mmc_get_card(MMC_HOST_ID);
	host = mmc_get_host(MMC_HOST_ID);

	if (!card)
		return MMC_ERR_INVALID;

	ext_csd = &card->raw_ext_csd[0];

	if (mmc_card_mmc(card) && ext_csd[EXT_CSD_REV] >= 3) {
		if (part_id == EMMC_PART_USER)
			part = EXT_CSD_PART_CFG_DEFT_PART;

		cfg = card->raw_ext_csd[EXT_CSD_PART_CFG];

		/* already set to specific partition */
		if (part == (cfg & 0x7))
			return MMC_ERR_NONE;

		cfg = (cfg & ~0x7) | part;

		err =
		    mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL,
			       EXT_CSD_PART_CFG, cfg);

		if (err == MMC_ERR_NONE) {
			err = mmc_read_ext_csd(host, card);
			if (err == MMC_ERR_NONE) {
				ext_csd = &card->raw_ext_csd[0];
				if (ext_csd[EXT_CSD_PART_CFG] != cfg)
					err = MMC_ERR_FAILED;
			}
		}
	}
	return err;
}

static int mmc_addr_trans_tbl_init(struct mmc_card *card, struct blkdev *bdev)
{
	u32 wpg_sz;
	u8 *ext_csd;

	ext_csd = &card->raw_ext_csd[0];
	bdev->offset = 0;

	if (mmc_card_mmc(card) && ext_csd[EXT_CSD_REV] >= 3) {
		u64 size[EMMC_PART_END];
		u32 i;

		if ((ext_csd[EXT_CSD_ERASE_GRP_DEF] & EXT_CSD_ERASE_GRP_DEF_EN)
		    && (ext_csd[EXT_CSD_HC_WP_GPR_SIZE] > 0)) {
			wpg_sz =
			    512 * 1024 * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] *
			    ext_csd[EXT_CSD_HC_WP_GPR_SIZE];
		} else {
			wpg_sz = card->csd.write_prot_grpsz;
		}

		size[EMMC_PART_BOOT1] =
		    ext_csd[EXT_CSD_BOOT_SIZE_MULT] * 128 * 1024;
		size[EMMC_PART_BOOT2] =
		    ext_csd[EXT_CSD_BOOT_SIZE_MULT] * 128 * 1024;
		size[EMMC_PART_RPMB] =
		    ext_csd[EXT_CSD_RPMB_SIZE_MULT] * 128 * 1024;
		size[EMMC_PART_GP1] =
		    ext_csd[EXT_CSD_GP1_SIZE_MULT + 2] * 256 * 256 +
		    ext_csd[EXT_CSD_GP1_SIZE_MULT + 1] * 256 +
		    ext_csd[EXT_CSD_GP1_SIZE_MULT + 0];
		size[EMMC_PART_GP2] =
		    ext_csd[EXT_CSD_GP2_SIZE_MULT + 2] * 256 * 256 +
		    ext_csd[EXT_CSD_GP2_SIZE_MULT + 1] * 256 +
		    ext_csd[EXT_CSD_GP2_SIZE_MULT + 0];
		size[EMMC_PART_GP3] =
		    ext_csd[EXT_CSD_GP3_SIZE_MULT + 2] * 256 * 256 +
		    ext_csd[EXT_CSD_GP3_SIZE_MULT + 1] * 256 +
		    ext_csd[EXT_CSD_GP3_SIZE_MULT + 0];
		size[EMMC_PART_GP4] =
		    ext_csd[EXT_CSD_GP4_SIZE_MULT + 2] * 256 * 256 +
		    ext_csd[EXT_CSD_GP4_SIZE_MULT + 1] * 256 +
		    ext_csd[EXT_CSD_GP4_SIZE_MULT + 0];
		size[EMMC_PART_USER] = (u64) card->blklen * card->nblks;

		size[EMMC_PART_GP1] *= wpg_sz;
		size[EMMC_PART_GP2] *= wpg_sz;
		size[EMMC_PART_GP3] *= wpg_sz;
		size[EMMC_PART_GP4] *= wpg_sz;

		for (i = EMMC_PART_BOOT1; i < EMMC_PART_END; i++) {
			g_emmc_size += size[i];
		}
		g_emmc_user_size = size[EMMC_PART_USER];
		/* determine user area offset */
		for (i = EMMC_PART_BOOT1; i < EMMC_PART_USER; i++)
			bdev->offset += size[i];

		bdev->offset /= bdev->blksz;	/* in blksz unit */
	}

	return 0;
}

static int mmc_bread(struct blkdev *bdev, u32 blknr, u32 blks, u8 *buf, u32 part_id)
{
	struct mmc_host *host = (struct mmc_host *)bdev->priv;

	mmc_switch_part(part_id);
	return mmc_block_read(host->id, (unsigned long)blknr, blks,
			      (unsigned long *)buf);
}

static int mmc_bwrite(struct blkdev *bdev, u32 blknr, u32 blks, u8 *buf, u32 part_id)
{
	struct mmc_host *host = (struct mmc_host *)bdev->priv;

	mmc_switch_part(part_id);
	return mmc_block_write(host->id, (unsigned long)blknr, blks,
			       (unsigned long *)buf);
}

// ==========================================================
// MMC Common Interface - Init
// ==========================================================
u32 mmc_init_device(void)
{
	int ret;
	struct mmc_card *card;

	if (!blkdev_get(BOOTDEV_SDMMC)) {

		ret = mmc_init(MMC_HOST_ID);
		if (ret != 0) {
			printk(BIOS_INFO, "[SD0] init card failed\n");
			return ret;
		}

		memset(&g_mmc_bdev, 0, sizeof(struct blkdev));

		card = mmc_get_card(MMC_HOST_ID);

		g_mmc_bdev.blksz = MMC_BLOCK_SIZE;
		g_mmc_bdev.erasesz = MMC_BLOCK_SIZE;
		g_mmc_bdev.blks = card->nblks;
		g_mmc_bdev.bread = mmc_bread;
		g_mmc_bdev.bwrite = mmc_bwrite;
		g_mmc_bdev.type = BOOTDEV_SDMMC;
		g_mmc_bdev.blkbuf = NULL;
		g_mmc_bdev.priv = (void *)mmc_get_host(MMC_HOST_ID);

		mmc_addr_trans_tbl_init(card, &g_mmc_bdev);

		blkdev_register(&g_mmc_bdev);
	}
	return 0;
}

u32 mmc_get_device_id(u8 * id, u32 len, u32 * fw_len)
{
	u8 buf[16];		/* CID = 128 bits */
	struct mmc_card *card;
	u8 buf_sanid[512];

	if (0 != mmc_init_device())
		return -1;

	card = mmc_get_card(MMC_HOST_ID);

	buf[0] = (card->raw_cid[0] >> 24) & 0xFF;	/* Manufacturer ID */
	buf[1] = (card->raw_cid[0] >> 16) & 0xFF;	/* Reserved(6)+Card/BGA(2) */
	buf[2] = (card->raw_cid[0] >> 8) & 0xFF;	/* OEM/Application ID */
	buf[3] = (card->raw_cid[0] >> 0) & 0xFF;	/* Product name [0] */
	buf[4] = (card->raw_cid[1] >> 24) & 0xFF;	/* Product name [1] */
	buf[5] = (card->raw_cid[1] >> 16) & 0xFF;	/* Product name [2] */
	buf[6] = (card->raw_cid[1] >> 8) & 0xFF;	/* Product name [3] */
	buf[7] = (card->raw_cid[1] >> 0) & 0xFF;	/* Product name [4] */
	buf[8] = (card->raw_cid[2] >> 24) & 0xFF;	/* Product name [5] */
	buf[9] = (card->raw_cid[2] >> 16) & 0xFF;	/* Product revision */
	buf[10] = (card->raw_cid[2] >> 8) & 0xFF;	/* Serial Number [0] */
	buf[11] = (card->raw_cid[2] >> 0) & 0xFF;	/* Serial Number [1] */
	buf[12] = (card->raw_cid[3] >> 24) & 0xFF;	/* Serial Number [2] */
	buf[13] = (card->raw_cid[3] >> 16) & 0xFF;	/* Serial Number [3] */
	buf[14] = (card->raw_cid[3] >> 8) & 0xFF;	/* Manufacturer date */
	buf[15] = (card->raw_cid[3] >> 0) & 0xFF;	/* CRC7 + stuff bit */
	*fw_len = 1;
	if (buf[0] == 0x45) {
		if (0 == mmc_get_sandisk_fwid(MMC_HOST_ID, buf_sanid))
			*fw_len = 6;
	}
	len = len > 16 ? 16 : len;
	memcpy(id, buf, len);
	if (*fw_len == 6)
		memcpy(id + len, buf_sanid + 32, 6);
	else
		*(id + len) = buf[9];

	return 0;
}
