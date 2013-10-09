/*
 * Copyright (c) 2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __NVIDIA_TEGRA124_SPI_H__
#define __NVIDIA_TEGRA124_SPI_H__

#include <stddef.h>
#include <spi-generic.h>

struct tegra_spi_channel {
	struct spi_slave slave;
	struct tegra_spi_regs *regs;

	/*
	 * max transfer size and dma_buf address will be set by
	 * mainboard-specific code, depending on the requirements for
	 * the device connected
	 */
	unsigned int max_transfer_size;
	void *dma_buf;
};

#define TEGRA124_NUM_SPI_CHANNELS 6
extern struct tegra_spi_channel tegra_spi_channels[];

struct cbfs_media;
int initialize_tegra_spi_cbfs_media(struct cbfs_media *media,
				     void *buffer_address,
				     size_t buffer_size);

void tegra_spi_init(unsigned int bus);

#endif	/* __NVIDIA_TEGRA124_SPI_H__ */
