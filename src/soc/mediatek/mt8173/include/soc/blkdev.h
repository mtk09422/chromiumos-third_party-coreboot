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

#ifndef SOC_MEDIATEK_MT8173_BLKDEV_H
#define SOC_MEDIATEK_MT8173_BLKDEV_H

#include <stdint.h>

struct blkdev {
	u32 type;		/* block device type */
	u32 blksz;		/* block size. (read/write unit) */
	u32 erasesz;		/* erase size */
	u32 blks;		/* number of blocks in the device */
	u32 offset;		/* user area offset in blksz unit */
	u8 *blkbuf;		/* block size buffer */
	void *priv;		/* device private data */
    struct blkdev *next;	/* next block device */
    int (*bread)(struct blkdev *bdev, u32 blknr, u32 blks, u8 *buf, u32 part_id);
    int (*bwrite)(struct blkdev *bdev, u32 blknr, u32 blks, u8 *buf, u32 part_id);
};

int blkdev_register(struct blkdev *bdev);
int blkdev_read(struct blkdev *bdev, u64 src, u32 size, u8 *dst, u32 part_id);
int blkdev_write(struct blkdev *bdev, u64 dst, u32 size, u8 *src, u32 part_id);
int blkdev_bread(struct blkdev *bdev, u32 blknr, u32 blks, u8 *buf, u32 part_id);
int blkdev_bwrite(struct blkdev *bdev, u32 blknr, u32 blks, u8 *buf, u32 part_id);
struct blkdev *blkdev_get(u32 type);

#endif /* SOC_MEDIATEK_MT8173_BLKDEV_H */
