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

#include <cpu/cpu.h>
#include <console/console.h>
#include <device/device.h>
#include <symbols.h>
#include <gic.h>

#include <soc/clock.h>

static void soc_read_resources(device_t dev)
{
	ram_resource(dev, 0, (uintptr_t)_dram / KiB,
		     CONFIG_DRAM_SIZE_MB * KiB);
}

static size_t cntrl_total_cpus(void)
{
	return CONFIG_MAX_CPUS;
}

static int cntrl_start_cpu(unsigned int id, void (*entry)(void))
{
	return 0;
}

static struct cpu_control_ops cntrl_ops = {
	.total_cpus = cntrl_total_cpus,
	.start_cpu = cntrl_start_cpu,
};

static void soc_init(device_t dev)
{
	printk(BIOS_INFO, "CPU: Mediatek MT8173\n");

	/* Initialize MT8173 CPU timer */
	clock_init_mt8173_timer();

	arch_initialize_cpus(dev, &cntrl_ops);
}

static struct device_operations soc_ops = {
	.read_resources = soc_read_resources,
	.init = soc_init,
};

static void enable_soc_dev(device_t dev)
{
	dev->ops = &soc_ops;
}

struct chip_operations soc_mediatek_mt8173_ops = {
	CHIP_NAME("SOC Mediatek MT8173\n")
	    .enable_dev = enable_soc_dev,
};
