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

#include <cbfs.h>
#include <soc/mmc_common_inter.h>
#include <soc/nor_if.h>

int init_default_cbfs_media(struct cbfs_media *media)
{
	#if IS_ENABLED(CONFIG_MTK_SPI_CBFS)
	/*for mt8173E2 support spi nor flash  */
	return init_mt8173_nor_cbfs_media(media);
	#else
	/*default cbfs media :emmc */
	return init_mt8173_emmc_cbfs_media(media);
	#endif
}

