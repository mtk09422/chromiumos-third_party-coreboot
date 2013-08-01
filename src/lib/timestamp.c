/*
 * This file is part of the coreboot project.
 *
 * Copyright 2011 Google Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA, 02110-1301 USA
 */

#include <stdint.h>
#include <console/console.h>
#include <cbmem.h>
#include <timestamp.h>
#ifndef __PRE_RAM__
#include <arch/early_variables.h>

static struct timestamp_table* ts_table;
#endif

#define MAX_TIMESTAMPS 30

void timestamp_init(uint64_t base)
{
	struct timestamp_table* tst;

	tst = cbmem_add(CBMEM_ID_TIMESTAMP,
			sizeof(struct timestamp_table) +
			MAX_TIMESTAMPS * sizeof(struct timestamp_entry));

	if (!tst) {
		printk(BIOS_ERR, "ERROR: failed to allocate timestamp table\n");
		return;
	}

	tst->base_time = base;
	tst->max_entries = MAX_TIMESTAMPS;
	tst->num_entries = 0;
}

void timestamp_add(enum timestamp_id id, uint64_t ts_time)
{
	struct timestamp_entry *tse;
#ifdef __PRE_RAM__
	struct timestamp_table *ts_table = cbmem_find(CBMEM_ID_TIMESTAMP);
#else
	if (!ts_table)
		ts_table = cbmem_find(CBMEM_ID_TIMESTAMP);
#endif
	if (!ts_table || (ts_table->num_entries == ts_table->max_entries))
		return;

	tse = &ts_table->entries[ts_table->num_entries++];
	tse->entry_id = id;
	tse->entry_stamp = ts_time - ts_table->base_time;
}

void timestamp_add_now(enum timestamp_id id)
{
	timestamp_add(id, timestamp_get());
}

#ifndef __PRE_RAM__

#define MAX_TIMESTAMP_CACHE 8
struct timestamp_cache {
	enum timestamp_id id;
	uint64_t time;
} timestamp_cache[MAX_TIMESTAMP_CACHE] CAR_GLOBAL;

static int timestamp_entries CAR_GLOBAL = 0;

/**
 * timestamp_stash() allows to temporarily cache time stamps.
 * This is needed when time stamping before the CBMEM area
 * is initialized. The function timestamp_sync() is used to
 * write the time stamps to the CBMEM area. This is done in
 * ram stage main()
 */

void timestamp_stash(enum timestamp_id id)
{
	if (timestamp_entries >= MAX_TIMESTAMP_CACHE) {
		printk(BIOS_ERR, "ERROR: failed to add timestamp to cache\n");
		return;
	}
	timestamp_cache[timestamp_entries].id = id;
	timestamp_cache[timestamp_entries].time = timestamp_get();
	timestamp_entries++;
}

void timestamp_sync(void)
{
	int i;
	for (i = 0; i < timestamp_entries; i++)
		timestamp_add(timestamp_cache[i].id, timestamp_cache[i].time);
	timestamp_entries = 0;
}

#endif
