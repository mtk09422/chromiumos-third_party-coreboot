/*
 * This file is part of the coreboot project.
 *
 * Copyright 2013 Google Inc.
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

#include <assert.h>
#include <cbfs.h>		/* This driver serves as a CBFS media source. */
#include <string.h>
#include <symbols.h>
#include <console/console.h>

#include <soc/mt8173.h>
#include <soc/mmc_core.h>
#include <soc/blkdev.h>
#include <soc/mmc_common_inter.h>

#if defined(CONFIG_DEBUG_CBFS_EMMC) && CONFIG_DEBUG_CBFS_EMMC
#define DEBUG_EMMC(x...)	printk(BIOS_DEBUG, "MTK_EMMC_CBFS: " x)
#else
#define DEBUG_EMMC(x...)
#endif

struct mtk_emmc_media {
	struct blkdev *bdev;
	struct cbfs_simple_buffer buffer;
};

static int mtk_emmc_cbfs_open(struct cbfs_media *media)
{
	struct mtk_emmc_media *emmc = (struct mtk_emmc_media *)media->context;

	mmc_init_device();
	emmc->bdev = blkdev_get(CFG_BOOT_DEV);
	DEBUG_EMMC("%s with blkdev = %p\n", __func__, emmc->bdev);

	return 0;
}

static int mtk_emmc_cbfs_close(struct cbfs_media *media)
{
	DEBUG_EMMC("%s\n", __func__);
	return 0;
}

static size_t mtk_emmc_cbfs_read(struct cbfs_media *media, void *dest,
				 size_t offset, size_t count)
{
	const u32 EMMC_BLOCK_SIZE = 512;
	const u32 BOOT0_SIZE = 4096 * EMMC_BLOCK_SIZE;
	const u32 BOOT1_SIZE = 4096 * EMMC_BLOCK_SIZE;
	struct mtk_emmc_media *emmc = (struct mtk_emmc_media *)media->context;
	size_t offset_within_block, count_tmp, count_ret = 0;
	u8 buffer[EMMC_BLOCK_SIZE];

	ASSERT(offset + count < BOOT0_SIZE + BOOT1_SIZE);
	DEBUG_EMMC("%s at offset: 0x%zx, count: 0x%zx\n", __func__, offset,
		   count);

	/* read unaligned data in 1st block */
	offset_within_block = offset % EMMC_BLOCK_SIZE;
	count_tmp = EMMC_BLOCK_SIZE - offset_within_block;
	emmc->bdev->bread(emmc->bdev, offset / EMMC_BLOCK_SIZE, 1, buffer, 1);
	DEBUG_EMMC("%s 1 at block #: 0x%zx, blocks count: 0x%x\n", __func__,
		   offset / EMMC_BLOCK_SIZE, 1);

	if (count < (EMMC_BLOCK_SIZE - offset_within_block))
		count_tmp = count;

	memcpy(dest, buffer + offset_within_block, count_tmp);

	dest += count_tmp;
	offset += count_tmp;
	count -= count_tmp;
	count_ret += count_tmp;

	/* handle read that cross boot0/boot1 boundary */
	if ((offset < BOOT0_SIZE) && ((offset + count) > BOOT0_SIZE)) {
		count_tmp = BOOT0_SIZE - offset;
		emmc->bdev->bread(emmc->bdev, offset / EMMC_BLOCK_SIZE,
				  count_tmp / EMMC_BLOCK_SIZE, dest, 1);
		DEBUG_EMMC("%s 2 at block #: 0x%zx, blocks count: 0x%zx\n",
			   __func__, offset / EMMC_BLOCK_SIZE,
			   count_tmp / EMMC_BLOCK_SIZE);

		dest += count_tmp;
		offset += count_tmp;
		count -= count_tmp;
		count_ret += count_tmp;
	}

	/* more aligned data to read */
	if (count / EMMC_BLOCK_SIZE) {
		emmc->bdev->bread(emmc->bdev, offset / EMMC_BLOCK_SIZE,
				  count / EMMC_BLOCK_SIZE, dest, 1);
		DEBUG_EMMC("%s 3 at block #: 0x%zx, blocks count: 0x%zx\n",
			   __func__, offset / EMMC_BLOCK_SIZE,
			   count / EMMC_BLOCK_SIZE);

		count_tmp = (count / EMMC_BLOCK_SIZE) * EMMC_BLOCK_SIZE;
		dest += count_tmp;
		offset += count_tmp;
		count -= count_tmp;
		count_ret += count_tmp;
	}

	/* read unaligned data in last block */
	if (count % EMMC_BLOCK_SIZE) {
		emmc->bdev->bread(emmc->bdev, offset / EMMC_BLOCK_SIZE, 1,
				  buffer, 1);
		DEBUG_EMMC("%s 4 at block #: 0x%zx, blocks count: 0x%x\n",
			   __func__, offset / EMMC_BLOCK_SIZE, 1);

		count_tmp = count % EMMC_BLOCK_SIZE;
		memcpy(dest, buffer, count_tmp);

		dest += count_tmp;
		offset += count_tmp;
		count -= count_tmp;
		count_ret += count_tmp;
	}

	ASSERT(count == 0);

	return count_ret;
}

static void *mtk_emmc_cbfs_map(struct cbfs_media *media, size_t offset,
			       size_t count)
{
	struct mtk_emmc_media *emmc = (struct mtk_emmc_media *)media->context;
	void *map;
	DEBUG_EMMC("%s at offset: 0x%zx, count: 0x%zx\n", __func__, offset,
		   count);
	map = cbfs_simple_buffer_map(&emmc->buffer, media, offset, count);
	return map;
}

static void *mtk_emmc_cbfs_unmap(struct cbfs_media *media, const void *address)
{
	struct mtk_emmc_media *emmc = (struct mtk_emmc_media *)media->context;
	DEBUG_EMMC("%s at address: %p\n", __func__, address);
	return cbfs_simple_buffer_unmap(&emmc->buffer, address);
}

#ifndef __BOOTBLOCK__
uintptr_t _dram_ = (uintptr_t)_dram;
#endif
int init_default_cbfs_media(struct cbfs_media *media)
{
	static struct mtk_emmc_media context;
	DEBUG_EMMC("Initializing CBFS media on eMMC %s\n", __func__);

	context.buffer.allocated = context.buffer.last_allocate = 0;
#if defined(__BOOTBLOCK__)
	context.buffer.buffer = (void *)0x0010A000;
	context.buffer.size = 0x10000;
#else
	context.buffer.buffer = (void *)(_dram_ + 0x244020);
	context.buffer.size = 0x100000;
#endif

	media->context = (void *)&context;
	media->open = mtk_emmc_cbfs_open;
	media->close = mtk_emmc_cbfs_close;
	media->read = mtk_emmc_cbfs_read;
	media->map = mtk_emmc_cbfs_map;
	media->unmap = mtk_emmc_cbfs_unmap;

	return 0;
}
