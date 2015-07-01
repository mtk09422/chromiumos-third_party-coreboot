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

#include <cpu/cpu.h>
#include <console/console.h>
#include <device/device.h>
#include <symbols.h>
#include <gic.h>
#include <soc/cpu.h>

#include <soc/addressmap.h>
#include <soc/display.h>
#include <vendorcode/google/chromeos/chromeos.h>
#include "chip.h"

static void soc_read_resources(device_t dev)
{
	unsigned long index = 0;
	int i; uintptr_t begin, end;
	size_t size;
	for (i = 0; i < CARVEOUT_NUM; i++) {
		carveout_range(i, &begin, &size);
		if (size == 0)
			continue;
		reserved_ram_resource(dev, index++, begin * KiB, size * KiB);
	}

	memory_in_range_below_4gb(&begin, &end);
	size = end - begin;
	ram_resource(dev, index++, begin * KiB, size * KiB);

	memory_in_range_above_4gb(&begin, &end);
	size = end - begin;
	ram_resource(dev, index++, begin * KiB, size * KiB);
}

static size_t cntrl_total_cpus(void)
{
	return CONFIG_MAX_CPUS;
}

static int cntrl_start_cpu(unsigned int id, void (*entry)(void))
{
	start_cpu(id, (uintptr_t)entry);
	return 0;
}

static struct cpu_control_ops cntrl_ops = {
	.total_cpus = cntrl_total_cpus,
	.start_cpu = cntrl_start_cpu,
};

static void soc_init(device_t dev)
{
	printk(BIOS_INFO, "CPU: Mediatek MT8173\n");

	arch_initialize_cpus(dev, &cntrl_ops);

#if IS_ENABLED(CONFIG_MEDIATEK_DISPLAY_INIT)
	if (vboot_skip_display_init())
		printk(BIOS_INFO, "Skipping display init.\n");
	else
		mt8173_display_init(dev);
#endif

}

static struct device_operations soc_ops = {
	.read_resources = soc_read_resources,
	.init = soc_init,
};

static void enable_soc_dev(device_t dev)
{
	if (dev->path.type == DEVICE_PATH_CPU_CLUSTER)
		dev->ops = &soc_ops;
}

struct chip_operations soc_mediatek_mt8173_ops = {
	CHIP_NAME("SOC Mediatek MT8173\n")
	    .enable_dev = enable_soc_dev,
};

static void mt8173_cpu_init(device_t cpu)
{
}

static const struct cpu_device_id ids[] = {
	{ 0x410fd032 },	/* A53 */
	{ 0x410fd072 },	/* A57 */
	{ CPU_ID_END },
};

static struct device_operations cpu_dev_ops = {
	.init = mt8173_cpu_init,
};

static const struct cpu_driver driver __cpu_driver = {
	.ops	  = &cpu_dev_ops,
	.id_table = ids,
};
