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

#include <assert.h>
#include <string.h>
#include <symbols.h>
#include <cbfs.h>
#include <console/console.h>
#include <soc/blkdev.h>
#include <soc/mt8173.h>
#include <soc/nor_if.h>

struct mtk_nor_media {
	struct blkdev *bdev;
	struct cbfs_simple_buffer buffer; /* defined in cbfs.h. */
};

static int mtk_nor_cbfs_open(struct cbfs_media *media)
{
	u32 i;
	struct mtk_nor_media *nor = (struct mtk_nor_media*)media->context;
	i = nor_init_device();
	nor->bdev = blkdev_get(CFG_BOOT_DEV);

	return 0;
}

static int mtk_nor_cbfs_close(struct cbfs_media *media)
{
	return 0;
}

static size_t mtk_nor_cbfs_read(struct cbfs_media *media, void *dest,
				size_t offset, size_t counter)
{
	const u32 NOR_BLOCK_SIZE = 512;
	struct mtk_nor_media *nor = (struct mtk_nor_media*)media->context;
	size_t offset_within_block, counter_temp, counter_ret = 0;
	u8 buffer[NOR_BLOCK_SIZE];

	/* read unaligned data in 1st block */
	offset_within_block = offset % NOR_BLOCK_SIZE;
	counter_temp = NOR_BLOCK_SIZE - offset_within_block;
	nor->bdev->bread(nor->bdev, offset / NOR_BLOCK_SIZE, 1, buffer, 1);

	if (counter < (NOR_BLOCK_SIZE - offset_within_block))
		counter_temp = counter;

	memcpy(dest, buffer + offset_within_block, counter_temp);

	dest += counter_temp;
	offset += counter_temp;
	counter -= counter_temp;
	counter_ret += counter_temp;

	/* more aligned data to read */
	if (counter / NOR_BLOCK_SIZE) {
		nor->bdev->bread(nor->bdev, offset / NOR_BLOCK_SIZE,
				 counter / NOR_BLOCK_SIZE, dest, 1);

		counter_temp = (counter / NOR_BLOCK_SIZE) * NOR_BLOCK_SIZE;
		dest += counter_temp;
		offset += counter_temp;
		counter -= counter_temp;
		counter_ret += counter_temp;
	}

	/* read unaligned data in last block */
	if (counter % NOR_BLOCK_SIZE) {
		nor->bdev->bread(nor->bdev, offset / NOR_BLOCK_SIZE, 1,
				 buffer, 1);

		counter_temp = counter % NOR_BLOCK_SIZE;
		memcpy(dest, buffer, counter_temp);

		dest += counter_temp;
		offset += counter_temp;
		counter -= counter_temp;
		counter_ret += counter_temp;
	}

	ASSERT(counter == 0);

	return counter_ret;
}

static void *mtk_nor_cbfs_map(struct cbfs_media *media, size_t offset,
			      size_t counter)
{
	struct mtk_nor_media *nor = (struct mtk_nor_media*)media->context;
	void *map;

	printk(BIOS_DEBUG,"%s at offset: 0x%zx, counter: 0x%zx\n", __func__, offset, counter);
	map = cbfs_simple_buffer_map(&nor->buffer, media, offset, counter);
	return map;
}

static void *mtk_nor_cbfs_unmap(struct cbfs_media *media, const void *address)
{
	struct mtk_nor_media *nor = (struct mtk_nor_media*)media->context;

	printk(BIOS_DEBUG,"%s at address: %p\n", __func__, address);
	return cbfs_simple_buffer_unmap(&nor->buffer, address);
}

int init_mt8173_nor_cbfs_media(struct cbfs_media *media)
{
	static struct mtk_nor_media context;

	context.buffer.allocated = context.buffer.last_allocate = 0;
	context.buffer.buffer = (void *)_cbfs_cache;
	context.buffer.size = _cbfs_cache_size;
	media->context = (void*)&context;
	media->open = mtk_nor_cbfs_open;
	media->close = mtk_nor_cbfs_close;
	media->read = mtk_nor_cbfs_read;
	media->map = mtk_nor_cbfs_map;
	media->unmap = mtk_nor_cbfs_unmap;

	return 0;
}
