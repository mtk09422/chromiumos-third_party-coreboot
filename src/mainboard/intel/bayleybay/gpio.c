/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2013 Google Inc.
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

#include <stdlib.h>
#include <soc/gpio.h>
#include "irqroute.h"

/* NCORE GPIOs */
static const struct soc_gpio_map gpncore_gpio_map[] = {
	GPIO_FUNC(2, PULL_UP, 20K),	/* GPIO 0 */
	GPIO_FUNC(2, PULL_DOWN, 20K),	/* GPIO 1 */
	GPIO_FUNC(2, PULL_UP, 20K),	/* GPIO 2 */
	GPIO_INPUT_PD_20K,	/* GPIO 3 */
	GPIO_INPUT_PD_20K,	/* GPIO 4 */
	GPIO_INPUT_PD_20K,	/* GPIO 5 */
	GPIO_FUNC(2, PULL_UP, 20K),	/* GPIO 6 */
	GPIO_FUNC(2, PULL_DOWN, 20K),	/* GPIO 7 */
	GPIO_FUNC(2, PULL_UP, 20K),	/* GPIO 8 */
	GPIO_FUNC(2, PULL_DOWN, 20K),	/* GPIO 9 */
	GPIO_FUNC(2, PULL_DOWN, 20K),	/* GPIO 10 */
	GPIO_FUNC(2, PULL_DOWN, 20K),	/* GPIO 11 */
	GPIO_INPUT_PD_20K,	/* GPIO 12 */
	GPIO_INPUT_PD_20K,	/* GPIO 13 */
	GPIO_INPUT_PU_20K,	/* GPIO 14 */
	GPIO_OUT_LOW,	/* GPIO 15 */
	GPIO_OUT_LOW,	/* GPIO 16 */
	GPIO_INPUT_PD_20K,	/* GPIO 17 */
	GPIO_INPUT_PD_20K,	/* GPIO 18 */
	GPIO_OUT_LOW,	/* GPIO 19 */
	GPIO_OUT_LOW,	/* GPIO 20 */
	GPIO_OUT_LOW,	/* GPIO 21 */
	GPIO_OUT_LOW,	/* GPIO 22 */
	GPIO_OUT_LOW,	/* GPIO 23 */
	GPIO_OUT_LOW,	/* GPIO 24 */
	GPIO_OUT_LOW,	/* GPIO 25 */
	GPIO_OUT_LOW,	/* GPIO 26 */
	GPIO_END
};

/* SCORE GPIOs */
static const struct soc_gpio_map gpscore_gpio_map[] = {
	GPIO_DIRQ_EDGEHIGH_PD_20K, /* GPIO 0 */
	GPIO_FUNC(2, PULL_DOWN, 20K), /* GPIO 1 */
	GPIO_FUNC(1, PULL_UP, 20K), /* GPIO 2 */
	GPIO_FUNC(1, PULL_UP, 20K), /* GPIO 3 */
	GPIO_FUNC(1, PULL_UP, 20K), /* GPIO 4 */
	GPIO_FUNC(1, PULL_UP, 20K), /* GPIO 5 */
	GPIO_FUNC(1, PULL_UP, 20K), /* GPIO 6 */
	GPIO_FUNC(2, PULL_UP, 20K), /* GPIO 7 */
	GPIO_FUNC(2, PULL_DOWN, 20K), /* GPIO 8 */
	GPIO_FUNC(2, PULL_DOWN, 20K), /* GPIO 9 */
	GPIO_FUNC(2, PULL_DOWN, 20K), /* GPIO 10 */
	GPIO_FUNC(2, PULL_DOWN, 20K), /* GPIO 11 */
	GPIO_FUNC(2, PULL_DOWN, 20K), /* GPIO 12 */
	GPIO_FUNC(2, PULL_DOWN, 20K), /* GPIO 13 */
	GPIO_FUNC(2, PULL_DOWN, 20K), /* GPIO 14 */
	GPIO_FUNC(2, PULL_DOWN, 20K), /* GPIO 15 */
	GPIO_FUNC(1, PULL_DOWN, 20K), /* GPIO 16 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 17 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 18 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 19 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 20 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 21 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 22 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 23 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 24 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 25 */
	GPIO_FUNC(1, PULL_DOWN, 20K), /* GPIO 26 */
	GPIO_FUNC(1, PULL_DOWN, 20K), /* GPIO 27 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 28 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 29 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 30 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 31 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 32 */
	GPIO_FUNC(1, PULL_DOWN, 20K), /* GPIO 33 */
	GPIO_FUNC(1, PULL_UP, 20K), /* GPIO 34 */
	GPIO_FUNC(1, PULL_UP, 20K), /* GPIO 35 */
	GPIO_FUNC(1, PULL_UP, 20K), /* GPIO 36 */
	GPIO_FUNC(1, PULL_UP, 20K), /* GPIO 37 */
	GPIO_DIRQ_EDGEBOTH_PU_20K, /* GPIO 38 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 39 */
	GPIO_FUNC(1, PULL_DOWN, 20K),	/* GPIO 40 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 41 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 42 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 43 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 44 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 45 */
	GPIO_FUNC(1, PULL_UP, 20K),	/* GPIO 46 */
	GPIO_FUNC(1, PULL_DOWN, 20K), /* GPIO 47 */
	GPIO_FUNC(1, PULL_DOWN, 20K), /* GPIO 48 */
	GPIO_FUNC(1, PULL_UP, 20K), /* GPIO 49 */
	GPIO_FUNC(1, PULL_UP, 20K), /* GPIO 50 */
	GPIO_FUNC(1, PULL_UP, 20K), /* GPIO 51 */
	GPIO_FUNC(1, PULL_UP, 20K), /* GPIO 52 */
	GPIO_FUNC(1, PULL_UP, 20K), /* GPIO 53 */
	GPIO_FUNC(1, PULL_DOWN, 20K), /* GPIO 54 */
	GPIO_OUT_LOW, /* GPIO 55 */
	GPIO_INPUT, /* GPIO 56 */
	GPIO_FUNC1, /* GPIO 57 */
	GPIO_INPUT, /* GPIO 58 */
	GPIO_OUT_HIGH, /* GPIO 59 */
	GPIO_OUT_HIGH, /* GPIO 60 */
	GPIO_FUNC1, /* GPIO 61 */
	GPIO_FUNC1, /* GPIO 62 */
	GPIO_FUNC1, /* GPIO 63 */
	GPIO_FUNC1, /* GPIO 64 */
	GPIO_FUNC1, /* GPIO 65 */
	GPIO_FUNC1, /* GPIO 66 */
	GPIO_FUNC1, /* GPIO 67 */
	GPIO_FUNC1, /* GPIO 68 */
	GPIO_FUNC1, /* GPIO 69 */
	GPIO_FUNC1, /* GPIO 70 */
	GPIO_FUNC1, /* GPIO 71 */
	GPIO_FUNC1, /* GPIO 72 */
	GPIO_FUNC1, /* GPIO 73 */
	GPIO_FUNC1, /* GPIO 74 */
	GPIO_FUNC1, /* GPIO 75 */
	GPIO_FUNC1, /* GPIO 76 */
	GPIO_FUNC1, /* GPIO 77 */
	GPIO_FUNC1, /* GPIO 78 */
	GPIO_FUNC1, /* GPIO 79 */
	GPIO_FUNC1, /* GPIO 80 */
	GPIO_FUNC1, /* GPIO 81 */
	GPIO_FUNC1, /* GPIO 82 */
	GPIO_FUNC1, /* GPIO 83 */
	GPIO_FUNC1, /* GPIO 84 */
	GPIO_FUNC1, /* GPIO 85 */
	GPIO_FUNC1, /* GPIO 86 */
	GPIO_FUNC1, /* GPIO 87 */
	GPIO_FUNC1, /* GPIO 88 */
	GPIO_FUNC1, /* GPIO 89 */
	GPIO_FUNC1, /* GPIO 90 */
	GPIO_FUNC1, /* GPIO 91 */
	GPIO_FUNC0, /* GPIO 92 */
	GPIO_FUNC0, /* GPIO 93 */
	GPIO_DIRQ, /* GPIO 94 */
	GPIO_OUT_LOW, /* GPIO 95 */
	GPIO_FUNC1, /* GPIO 96 */
	GPIO_OUT_LOW, /* GPIO 97 */
	GPIO_OUT_LOW, /* GPIO 98 */
	GPIO_FUNC1, /* GPIO 99 */
	GPIO_OUT_HIGH, /* GPIO 100 */
	GPIO_FUNC1, /* GPIO 101 */
	GPIO_END
};

/* SSUS GPIOs */
static const struct soc_gpio_map gpssus_gpio_map[] = {
	GPIO_DIRQ_EDGELOW_PU_20K,/* GPIO 0 */
	GPIO_FUNC6, /* GPIO 1 */
	GPIO_FUNC6, /* GPIO 2 */
	GPIO_DIRQ_LEVELLOW_PU_20K, /* GPIO 3 */
	GPIO_DIRQ_EDGEHIGH_PD_20K, /* GPIO 4 */
	GPIO_FUNC1, /* GPIO 5 */
	GPIO_DIRQ_LEVELLOW_PU_20K, /* GPIO 6 */
	GPIO_INPUT_PD_20K, /* GPIO 7 */
	GPIO_OUT_HIGH, /* GPIO 8 */
	GPIO_OUT_LOW, /* GPIO 9 */
	GPIO_DIRQ_LEVELLOW_PU_20K, /* GPIO 10 */
	GPIO_FUNC0, /* GPIO 11 */
	GPIO_FUNC0, /* GPIO 12 */
	GPIO_FUNC0, /* GPIO 13 */
	GPIO_OUT_HIGH, /* GPIO 14 */
	GPIO_FUNC0, /* GPIO 15 */
	GPIO_FUNC0, /* GPIO 16 */
	GPIO_DIRQ_LEVELHIGH_NO_PULL, /* GPIO 17 */
	GPIO_FUNC0, /* GPIO 18 */
	GPIO_FUNC0, /* GPIO 19 */
	GPIO_FUNC0, /* GPIO 20 */
	GPIO_NC, /* GPIO 21 */
	GPIO_NC, /* XDP_GPIO_DFX0 */
	GPIO_NC, /* XDP_GPIO_DFX1 */
	GPIO_NC, /* XDP_GPIO_DFX2 */
	GPIO_NC, /* XDP_GPIO_DFX3 */
	GPIO_NC, /* XDP_GPIO_DFX4 */
	GPIO_NC, /* XDP_GPIO_DFX5 */
	GPIO_NC, /* XDP_GPIO_DFX6 */
	GPIO_NC, /* XDP_GPIO_DFX7 */
	GPIO_NC, /* XDP_GPIO_DFX8 */
	GPIO_OUT_LOW, /* GPIO 31 */
	GPIO_OUT_LOW, /* GPIO 32 */
	GPIO_OUT_LOW, /* GPIO 33 */
	GPIO_INPUT, /* GPIO 34 */
	GPIO_OUT_LOW, /* GPIO 35 */
	GPIO_OUT_LOW, /* GPIO 36 */
	GPIO_OUT_HIGH,	/* GPIO 37 */
	GPIO_INPUT,	/* GPIO 38 */
	GPIO_INPUT,	/* GPIO 39 */
	GPIO_INPUT, /* GPIO 40 */
	GPIO_OUT_HIGH, /* GPIO 41 */
	GPIO_INPUT, /* GPIO 42 */
	GPIO_INPUT, /* GPIO 43 */
	GPIO_END
};



static struct soc_gpio_config gpio_config = {
	.ncore = gpncore_gpio_map,
	.score = gpscore_gpio_map,
	.ssus  = gpssus_gpio_map,
	.core_dirq = NULL,
	.sus_dirq = NULL,
};

struct soc_gpio_config* mainboard_get_gpios(void)
{
	return &gpio_config;
}
