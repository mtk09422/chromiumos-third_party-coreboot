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

#include <soc/blkdev.h>

static struct blkdev *blkdev_list;

int blkdev_register(struct blkdev *bdev)
{
	struct blkdev *tail = blkdev_list;

	bdev->next = NULL;

	while (tail && tail->next)
		tail = tail->next;

	if (tail)
		tail->next = bdev;
	else
		blkdev_list = bdev;

	return 0;
}

struct blkdev *blkdev_get(u32 type)
{
	struct blkdev *bdev = blkdev_list;

	while (bdev) {
		if (bdev->type == type)
			break;
		bdev = bdev->next;
	}
	return bdev;
}

int blkdev_bread(struct blkdev *bdev, u32 blknr, u32 blks, u8 *buf)
{
	return bdev->bread(bdev, blknr, blks, buf);
}

int blkdev_bwrite(struct blkdev *bdev, u32 blknr, u32 blks, u8 *buf)
{
	return bdev->bwrite(bdev, blknr, blks, buf);
}

int blkdev_read(struct blkdev *bdev, u64 src, u32 size, u8 *dst)
{
	if (NULL == bdev->blkbuf)
		return -1;
	u8 *buf = (u8 *)bdev->blkbuf;

	u32 blksz = bdev->blksz;
	u64 part_start;
	u64 part_end;
	u64 part_len;
	u64 aligned_start;
	u64 aligned_end;
	u64 end;
	u32 blknr;
	u32 blks;

	if (!bdev)
		return -1;

	if (size == 0)
		return 0;

	end = src + size;

	part_start = src & (blksz - 1);
	part_end = end & (blksz - 1);
	aligned_start = src & ~(blksz - 1);
	aligned_end = end & ~(blksz - 1);

	if (part_start) {
		blknr = aligned_start / blksz;
		part_len =
		    part_start + size > blksz ? blksz - part_start : size;
		if ((bdev->bread(bdev, blknr, 1, buf)) != 0)
			return -1;
		memcpy(dst, buf + part_start, part_len);
		dst += part_len;
		src += part_len;
		size -= part_len;
	}

	if (size >= blksz) {
		aligned_start = src & ~(blksz - 1);
		blknr = aligned_start / blksz;
		blks = (aligned_end - aligned_start) / blksz;

		if (blks && 0 != bdev->bread(bdev, blknr, blks, dst))
			return -1;

		src += (blks * blksz);
		dst += (blks * blksz);
		size -= (blks * blksz);
	}
	if (size && part_end && src < end) {
		blknr = aligned_end / blksz;
		if ((bdev->bread(bdev, blknr, 1, buf)) != 0)
			return -1;
		memcpy(dst, buf, part_end);
	}
	return 0;
}

int blkdev_write(struct blkdev *bdev, u64 dst, u32 size, u8 *src)
{
	if (NULL == bdev->blkbuf)
		return -1;
	u8 *buf = (u8 *)bdev->blkbuf;

	u32 blksz = bdev->blksz;
	u64 part_start;
	u64 part_end;
	u64 part_len;
	u64 aligned_start;
	u64 aligned_end;
	u64 end;
	u32 blknr;
	u32 blks;

	if (!bdev)
		return -1;

	if (size == 0)
		return 0;

	end = dst + size;

	part_start = dst & (blksz - 1);
	part_end = end & (blksz - 1);
	aligned_start = dst & ~(blksz - 1);
	aligned_end = end & ~(blksz - 1);

	if (part_start) {
		blknr = aligned_start / blksz;
		part_len =
		    part_start + size > blksz ? blksz - part_start : size;
		if ((bdev->bread(bdev, blknr, 1, buf)) != 0)
			return -1;
		memcpy(buf + part_start, src, part_len);
		if ((bdev->bwrite(bdev, blknr, 1, buf)) != 0)
			return -1;
		dst += part_len;
		src += part_len;
		size -= part_len;
	}

	if (size >= blksz) {
		aligned_start = dst & ~(blksz - 1);
		blknr = aligned_start / blksz;
		blks = (aligned_end - aligned_start) / blksz;

		if (blks && 0 != bdev->bwrite(bdev, blknr, blks, src))
			return -1;

		src += (blks * blksz);
		dst += (blks * blksz);
		size -= (blks * blksz);
	}

	if (size && part_end && dst < end) {
		blknr = aligned_end / blksz;
		if ((bdev->bread(bdev, blknr, 1, buf)) != 0)
			return -1;
		memcpy(buf, src, part_end);
		if ((bdev->bwrite(bdev, blknr, 1, buf)) != 0)
			return -1;
	}
	return 0;
}
