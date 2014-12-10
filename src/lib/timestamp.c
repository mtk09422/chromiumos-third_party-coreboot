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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <console/console.h>
#include <cbmem.h>
#include <symbols.h>
#include <timestamp.h>
#include <arch/early_variables.h>

#define MAX_TIMESTAMPS 60

#define MAX_TIMESTAMP_CACHE 30

struct __attribute__((__packed__)) timestamp_cache {
	uint16_t cache_state;
	uint16_t cbmem_state;
	struct timestamp_table table;
	struct timestamp_entry entries[MAX_TIMESTAMP_CACHE];
};

#define USE_TIMESTAMP_REGION				     \
	(IS_ENABLED(CONFIG_HAS_PRECBMEM_TIMESTAMP_REGION) && \
	 defined(__PRE_RAM__))

#define USE_LOCAL_TIMESTAMP_CACHE	(!defined(__PRE_RAM__))

#define HAS_CBMEM						\
	(defined(__ROMSTAGE__) || defined(__RAMSTAGE__))

#if USE_LOCAL_TIMESTAMP_CACHE
static struct timestamp_cache timestamp_cache;
#endif

enum {
	TIMESTAMP_CACHE_UNINITIALIZED = 0,
	TIMESTAMP_CACHE_INITIALIZED,
	TIMESTAMP_CACHE_NOT_NEEDED,
};

enum {
	TIMESTAMP_CBMEM_RESET_NOT_REQD = 0,
	TIMESTAMP_CBMEM_RESET_REQD,
};

static void timestamp_cache_init(struct timestamp_cache *ts_cache,
				 uint64_t base, uint16_t cbmem_state)
{
	ts_cache->table.num_entries = 0;
	ts_cache->table.max_entries = MAX_TIMESTAMP_CACHE;
	ts_cache->table.base_time = base;
	ts_cache->cache_state = TIMESTAMP_CACHE_INITIALIZED;
	ts_cache->cbmem_state = cbmem_state;
}

static struct timestamp_cache *timestamp_cache_get(void)
{
	struct timestamp_cache *ts_cache = NULL;

#if USE_LOCAL_TIMESTAMP_CACHE
	ts_cache = &timestamp_cache;
#elif IS_ENABLED(CONFIG_HAS_PRECBMEM_TIMESTAMP_REGION)
	if (_timestamp_size < sizeof(*ts_cache))
		BUG();
	ts_cache = car_get_var_ptr((void *)_timestamp);
#endif

	if (ts_cache && ts_cache->cache_state == TIMESTAMP_CACHE_UNINITIALIZED)
		timestamp_cache_init(ts_cache, 0,
				     TIMESTAMP_CBMEM_RESET_NOT_REQD);

	return ts_cache;
}

#if HAS_CBMEM
static struct timestamp_table *timestamp_alloc_cbmem_table(void)
{
	struct timestamp_table *tst;

	tst = cbmem_add(CBMEM_ID_TIMESTAMP,
			sizeof(struct timestamp_table) +
			MAX_TIMESTAMPS * sizeof(struct timestamp_entry));

	if (!tst)
		return NULL;

	tst->base_time = 0;
	tst->max_entries = MAX_TIMESTAMPS;
	tst->num_entries = 0;

	return tst;
}
#endif

static struct timestamp_table *timestamp_table_get(void)
{
	MAYBE_STATIC struct timestamp_table *ts_table = NULL;
	struct timestamp_cache *ts_cache;

	ts_cache = timestamp_cache_get();

	if (ts_cache == NULL) {
#if HAS_CBMEM
		ts_table = cbmem_find(CBMEM_ID_TIMESTAMP);
#endif
		return ts_table;
	}

	if (ts_cache->cache_state != TIMESTAMP_CACHE_NOT_NEEDED)
		return &ts_cache->table;

#if HAS_CBMEM
	if (ts_cache->cbmem_state == TIMESTAMP_CBMEM_RESET_REQD) {
		ts_table = timestamp_alloc_cbmem_table();
		ts_cache->cbmem_state = TIMESTAMP_CBMEM_RESET_NOT_REQD;
	}

	if (!ts_table)
		ts_table = cbmem_find(CBMEM_ID_TIMESTAMP);
#endif

	return ts_table;
}

static void timestamp_add_table_entry(struct timestamp_table *ts_table,
				      enum timestamp_id id, uint64_t ts_time)
{
	struct timestamp_entry *tse;

	if (ts_table->num_entries == ts_table->max_entries) {
		printk(BIOS_ERR, "ERROR: Dropped a timestamp entry\n");
		return;
	}

	tse = &ts_table->entries[ts_table->num_entries++];
	tse->entry_id = id;
	tse->entry_stamp = ts_time - ts_table->base_time;
}

static void timestamp_sync(void)
{
	uint32_t i;

	struct timestamp_cache *ts_cache;
	struct timestamp_table *ts_cache_table;
	struct timestamp_table *ts_cbmem_table = NULL;

	ts_cache = timestamp_cache_get();

	/* No timestamp cache found */
	if (ts_cache == NULL) {
		printk(BIOS_ERR, "ERROR: No timestamp cache found\n");
		return;
	}

	ts_cache_table = &ts_cache->table;

#if HAS_CBMEM
	ts_cbmem_table = cbmem_find(CBMEM_ID_TIMESTAMP);

	if ((ts_cache->cbmem_state == TIMESTAMP_CBMEM_RESET_REQD) ||
	    (ts_cbmem_table == NULL))
		ts_cbmem_table = timestamp_alloc_cbmem_table();
#endif

	if (ts_cbmem_table == NULL) {
		printk(BIOS_ERR, "ERROR: No timestamp table allocated\n");
		return;
	}

	if (ts_cbmem_table == ts_cache_table) {
		printk(BIOS_ERR, "ERROR: Table error to sync timestamps\n");
		return;
	}

	ts_cache->cache_state = TIMESTAMP_CACHE_NOT_NEEDED;
	ts_cache->cbmem_state = TIMESTAMP_CBMEM_RESET_NOT_REQD;

	/*
	 * There can be two cases for timestamp sync:
	 * 1. Newly added cbmem_table will have base_time of 0. Thus, no
	 * adjusments are needed for the timestamps being added from cache to
	 * cbmem table.
	 * 2. Timestamps added to cache before ramstage: In this case, the
	 * base_time in cache table would be 0 and add_table_entry will take
	 * care of subtracting the appropriate base_time.
	 */
	for (i = 0; i < ts_cache_table->num_entries; i++) {
		struct timestamp_entry *tse = &ts_cache_table->entries[i];
		timestamp_add_table_entry(ts_cbmem_table, tse->entry_id,
					  tse->entry_stamp);
	}

	ts_cache_table->num_entries = 0;
	/* Freshly added cbmem table has base_time 0. Inherit cache base_time */
	if (ts_cbmem_table->base_time == 0)
		ts_cbmem_table->base_time = ts_cache_table->base_time;
}
ROMSTAGE_CBMEM_INIT_HOOK(timestamp_sync)
RAMSTAGE_CBMEM_INIT_HOOK(timestamp_sync)

void timestamp_early_init(uint64_t base)
{
	struct timestamp_cache *ts_cache;
	ts_cache = timestamp_cache_get();
	if (ts_cache == NULL)
		BUG();
	timestamp_cache_init(ts_cache, base, TIMESTAMP_CBMEM_RESET_REQD);
}

void timestamp_init(uint64_t base)
{
	struct timestamp_cache *ts_cache;
	struct timestamp_table *tst;

	ts_cache = timestamp_cache_get();

	ts_cache->cbmem_state = TIMESTAMP_CBMEM_RESET_REQD;

	tst = timestamp_table_get();

	if (!tst) {
		printk(BIOS_ERR, "ERROR: No timestamp table to init\n");
		return;
	}

	tst->base_time = base;
}

void timestamp_add(enum timestamp_id id, uint64_t ts_time)
{
	struct timestamp_table *ts_table;

	ts_table = timestamp_table_get();

	if (!ts_table) {
		printk(BIOS_ERR, "ERROR: No timestamp table found\n");
		return;
	}

	timestamp_add_table_entry(ts_table, id, ts_time);
}

void timestamp_add_now(enum timestamp_id id)
{
	timestamp_add(id, timestamp_get());
}
