/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 Google Inc.
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

#include <boardid.h>
#include <boot/coreboot_tables.h>
#include <console/console.h>
#include <delay.h>
#include <drivers/i2c/ww_ring/ww_ring.h>
#include <gpio.h>
#include <soc/cdp.h>
#include <soc/gsbi.h>
#include <string.h>
#include <timer.h>
#include <vendorcode/google/chromeos/chromeos.h>

#define FAKE_GPIO_NUM		-1

struct gpio_desc {
	gpio_t gpio_num;
	const char *gpio_name;
	uint32_t fake_value;
	int last_reported;
};

/* Actual GPIO switch names */
#define DEVELOPER_GPIO_NAME	"developer"
#define RECOVERY_GPIO_NAME	"recovery"
#define WRITE_PROTECT_GPIO_NAME	"write protect"

static struct gpio_desc descriptors[] = {
	{ 15, DEVELOPER_GPIO_NAME },
	{ 16, RECOVERY_GPIO_NAME },
	{ 17, WRITE_PROTECT_GPIO_NAME },
	{ FAKE_GPIO_NUM, "power", 1 },	/* Power never pressed. */
	{ FAKE_GPIO_NUM, "lid", 0 }	/* Lid always open. */
};

static void fill_lb_gpio(struct lb_gpio *pgpio, struct gpio_desc *pdesc)
{
	gpio_t gpio_num = pdesc->gpio_num;

	pgpio->port = gpio_num;
	if (gpio_num == FAKE_GPIO_NUM) {
		pgpio->value = pdesc->fake_value;
	} else {
		gpio_tlmm_config_set(gpio_num, GPIO_FUNC_DISABLE,
				     GPIO_NO_PULL, GPIO_2MA, GPIO_DISABLE);
		udelay(10); /* Should be enough to settle. */
		pgpio->value = gpio_get(gpio_num);
	}
	pgpio->polarity = ACTIVE_LOW;
	strncpy((char *)pgpio->name, pdesc->gpio_name, sizeof(pgpio->name));

	if (pdesc->last_reported != (pgpio->value + 1)) {
		pdesc->last_reported = (pgpio->value + 1);
		printk(BIOS_INFO, "%s: %s: port %d value %d\n",
		       __func__, pgpio->name, pgpio->port, pgpio->value);
	}
}

void fill_lb_gpios(struct lb_gpios *gpios)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(descriptors); i++)
		fill_lb_gpio(gpios->gpios + i, descriptors + i);


	gpios->size = sizeof(*gpios) + sizeof(struct lb_gpio) * i;
	gpios->count = i;
}

static int get_switch_value(const char *switch_name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(descriptors); i++)
		if (!strcmp(descriptors[i].gpio_name, switch_name)) {
			struct lb_gpio gpio;

			fill_lb_gpio(&gpio, descriptors + i);
			return gpio.value ^ !gpio.polarity;
		}
	return -1;
}

int get_developer_mode_switch(void)
{
	return get_switch_value(DEVELOPER_GPIO_NAME);
}

/*
 * The recovery switch on storm is overloaded: it needs to be pressed for a
 * certain duration at startup to signal different requests:
 *
 * - keeping it pressed for 8 to 16 seconds after startup signals the need for
 *   factory reset (wipeout);
 * - keeping it pressed for longer than 16 seconds signals the need for Chrome
 *   OS recovery.
 *
 * The state is read once and cached for following inquiries. The below enum
 * lists possible states.
 */
enum switch_state {
	not_probed = -1,
	no_req,
	recovery_req,
	wipeout_req
};

static void display_pattern(int pattern)
{
	if (board_id() == BOARD_ID_WHIRLWIND_SP5)
		ww_ring_display_pattern(GSBI_ID_7, pattern);
}

#define WIPEOUT_MODE_DELAY_MS (8 * 1000)
#define RECOVERY_MODE_EXTRA_DELAY_MS (8 * 1000)

static enum switch_state get_switch_state(void)
{
	struct stopwatch sw;
	int sampled_value;
	static enum switch_state saved_state = not_probed;

	if (saved_state != not_probed)
		return saved_state;

	sampled_value = get_switch_value(RECOVERY_GPIO_NAME);

	if (!sampled_value) {
		saved_state = no_req;
		return saved_state;
	}

	display_pattern(WWR_RECOVERY_PUSHED);
	printk(BIOS_INFO, "recovery button pressed\n");

	stopwatch_init_msecs_expire(&sw, WIPEOUT_MODE_DELAY_MS);

	do {
		sampled_value = get_switch_value(RECOVERY_GPIO_NAME);
		if (!sampled_value)
			break;
	} while (!stopwatch_expired(&sw));

	if (sampled_value) {
		display_pattern(WWR_WIPEOUT_REQUEST);
		printk(BIOS_INFO, "wipeout requested, checking recovery\n");
		stopwatch_init_msecs_expire(&sw, RECOVERY_MODE_EXTRA_DELAY_MS);
		do {
			sampled_value = get_switch_value(RECOVERY_GPIO_NAME);
			if (!sampled_value)
				break;
		} while (!stopwatch_expired(&sw));

		if (sampled_value) {
			saved_state = recovery_req;
			display_pattern(WWR_RECOVERY_REQUEST);
			printk(BIOS_INFO, "recovery requested\n");
		} else {
			saved_state = wipeout_req;
		}
	} else {
		saved_state = no_req;
		display_pattern(WWR_ALL_OFF);
	}

	return saved_state;
}

int get_recovery_mode_switch(void)
{
	return get_switch_state() == recovery_req;
}

int get_wipeout_mode_switch(void)
{
	return get_switch_state() == wipeout_req;
}

int get_write_protect_state(void)
{
	return get_switch_value(WRITE_PROTECT_GPIO_NAME);
}
