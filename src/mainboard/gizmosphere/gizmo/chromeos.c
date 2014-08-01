/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2014 The ChromiumOS Authors.  All rights reserved.
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
#include <console/console.h>
#include <vendorcode/google/chromeos/chromeos.h>
#include <arch/io.h>

u32 acpi_mmio_base = 0;

#define GPIO_BASE	0x100
#define ACPI_BLK_BASE	0x800
#define IOMUX_BASE	0xD00

/* Gizmo Explorer Board GPIOS: GPIO_A, B, C, D, E, F, O, P */
static const u8 GPIOS[] = {64, 63, 60, 55, 59, 61, 43, 47};

#define GPIO_SPI_WP	GPIOS[0]
#define GPIO_REC_MODE	GPIOS[1]
#define GPIO_DEV_MODE	GPIOS[2]

static void gpio_configure(int mode, int function, int gpiox)
{
	u8 bdata;

	bdata = read8(acpi_mmio_base + GPIO_BASE + gpiox);
	if (bdata & 0x01)
		printk(BIOS_DEBUG,"ERROR: GPIO is owned by IMC!\n");
	bdata &= 0x07;

	/* mode: 1=GPIO_IN 0=GPIO_OUT */
	if (mode == 0)
		bdata |= 0x08;
	else
		bdata |= 0x28;

	write8(acpi_mmio_base + GPIO_BASE + gpiox, bdata);
	write8(acpi_mmio_base + IOMUX_BASE + gpiox, (u8)function);
}

static void gpio_init(void)
{
	u8 pm_index;
	/* Find the ACPImmioAddr base address */
	for (pm_index = 0x27; pm_index > 0x23; pm_index--) {
		acpi_mmio_base = acpi_mmio_base << 8;
		outb(pm_index, 0xCD6);
		acpi_mmio_base |= (unsigned long int) inb(0xCD7);
	}
	acpi_mmio_base &= 0xFFFFF000;

	gpio_configure(1, 1, GPIO_SPI_WP);
	gpio_configure(1, 1, GPIO_REC_MODE);
	gpio_configure(1, 1, GPIO_DEV_MODE);
}

static u8 gpio_read(int gpiox)
{
	if (!acpi_mmio_base)
		gpio_init();

	return !((read8(acpi_mmio_base + GPIO_BASE + gpiox) & 0x80) >> 7);
}

#ifndef __PRE_RAM__
#include <boot/coreboot_tables.h>

#define GPIO_COUNT	6

void fill_lb_gpios(struct lb_gpios *gpios)
{
	gpios->size = sizeof(*gpios) + (GPIO_COUNT * sizeof(struct lb_gpio));
	gpios->count = GPIO_COUNT;

	/* Write Protect: GPIO64 = CHP3_SPI_WP */
	gpios->gpios[0].port = GPIO_SPI_WP;
	gpios->gpios[0].polarity = ACTIVE_HIGH;
	gpios->gpios[0].value = gpio_read(GPIO_SPI_WP);
	strncpy((char *)gpios->gpios[0].name,"write protect",
							GPIO_MAX_NAME_LENGTH);

	/* Recovery: GPIO63 = CHP3_REC_MODE# */
	gpios->gpios[1].port = GPIO_REC_MODE;
	gpios->gpios[1].polarity = ACTIVE_HIGH;
	gpios->gpios[1].value = get_recovery_mode_switch();
	strncpy((char *)gpios->gpios[1].name,"recovery", GPIO_MAX_NAME_LENGTH);

	/* Developer: GPIO60 = KBC3_DVP_MODE */
	gpios->gpios[2].port = GPIO_DEV_MODE;
	gpios->gpios[2].polarity = ACTIVE_HIGH;
	gpios->gpios[2].value = get_developer_mode_switch();
	strncpy((char *)gpios->gpios[2].name,"developer", GPIO_MAX_NAME_LENGTH);

	/* Hard code the lid switch GPIO to open. */
	gpios->gpios[3].port = 100;
	gpios->gpios[3].polarity = ACTIVE_HIGH;
	gpios->gpios[3].value = 1;
	strncpy((char *)gpios->gpios[3].name,"lid", GPIO_MAX_NAME_LENGTH);

	/* Power Button */
	gpios->gpios[4].port = 101;
	gpios->gpios[4].polarity = ACTIVE_HIGH;
	gpios->gpios[4].value = (read32(acpi_mmio_base + ACPI_BLK_BASE) >>
							8) & 1;
	strncpy((char *)gpios->gpios[4].name,"power", GPIO_MAX_NAME_LENGTH);

	/* Did we load the VGA Option ROM? */
	gpios->gpios[5].port = -1; /* Indicate that this is a pseudo GPIO */
	gpios->gpios[5].polarity = ACTIVE_HIGH;
	gpios->gpios[5].value = oprom_is_loaded;
	strncpy((char *)gpios->gpios[5].name,"oprom", GPIO_MAX_NAME_LENGTH);
}
#endif

int get_developer_mode_switch(void)
{
	return gpio_read(GPIO_DEV_MODE);
}

int get_recovery_mode_switch(void)
{
	return gpio_read(GPIO_REC_MODE);
}
