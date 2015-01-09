/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2008, Jordan Crouse <jordan@cosmicpenguin.net>
 * Copyright (C) 2013 The Chromium OS Authors. All rights reserved.
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

#ifdef LIBPAYLOAD
# include <libpayload-config.h>
# ifdef CONFIG_LZMA
#  include <lzma.h>
#  define CBFS_CORE_WITH_LZMA
# endif
# define CBFS_MINI_BUILD
#elif defined(__SMM__)
# define CBFS_MINI_BUILD
#elif defined(__PRE_RAM__) && \
	(!defined(__ROMSTAGE__) || !IS_ENABLED(CONFIG_COMPRESS_RAMSTAGE))
  /* No LZMA before romstage, and not even there without ramstage compression */
#else
# define CBFS_CORE_WITH_LZMA
# include <lib.h>
#endif

#include <cbfs.h>
#include <string.h>
#include <cbmem.h>
#include <arch_ops.h>

#ifdef LIBPAYLOAD
# include <stdio.h>
# define DEBUG(x...)
# define LOG(x...) printf(x)
# define ERROR(x...) printf(x)
#else
# include <console/console.h>
# define ERROR(x...) printk(BIOS_ERR, "CBFS: " x)
# define LOG(x...) printk(BIOS_INFO, "CBFS: " x)
# if CONFIG_DEBUG_CBFS
#  define DEBUG(x...) printk(BIOS_SPEW, "CBFS: " x)
# else
#  define DEBUG(x...)
# endif
#endif

#include "cbfs_core.c"

#include <vendorcode/google/chromeos/chromeos.h>

#ifndef __SMM__
static inline int tohex4(unsigned int c)
{
	return (c <= 9) ? (c + '0') : (c - 10 + 'a');
}

static void tohex16(unsigned int val, char* dest)
{
	dest[0] = tohex4(val>>12);
	dest[1] = tohex4((val>>8) & 0xf);
	dest[2] = tohex4((val>>4) & 0xf);
	dest[3] = tohex4(val & 0xf);
}

void *cbfs_load_optionrom(struct cbfs_media *media, uint16_t vendor,
			  uint16_t device, void *dest)
{
	char name[17] = "pciXXXX,XXXX.rom";
	struct cbfs_optionrom *orom;
	uint8_t *src;

	tohex16(vendor, name+3);
	tohex16(device, name+8);

	orom = (struct cbfs_optionrom *)
		cbfs_get_file_content(media, name, CBFS_TYPE_OPTIONROM);

	if (orom == NULL)
		return NULL;

	/* They might have specified a dest address. If so, we can decompress.
	 * If not, there's not much hope of decompressing or relocating the rom.
	 * in the common case, the expansion rom is uncompressed, we
	 * pass 0 in for the dest, and all we have to do is find the rom and
	 * return a pointer to it.
	 */

	/* BUG: the cbfstool is (not yet) including a cbfs_optionrom header */
	src = (uint8_t *)orom; // + sizeof(struct cbfs_optionrom);

	if (! dest)
		return src;

	if (cbfs_decompress(ntohl(orom->compression),
			     src,
			     dest,
			     ntohl(orom->len)))
		return NULL;

	return dest;
}

#if CONFIG_RELOCATABLE_RAMSTAGE && defined(__ROMSTAGE__)

#include <rmodule.h>
#include <romstage_handoff.h>
static void *load_stage_from_cbfs(struct cbfs_media *media, const char *name,
                                  struct romstage_handoff *handoff)
{
	size_t ramstage_size;
	uintptr_t ramstage_base;

	struct rmod_stage_load rmod_ram = {
		.cbmem_id = CBMEM_ID_RAMSTAGE,
		.name = name,
	};

	if (rmodule_stage_load_from_cbfs(&rmod_ram)) {
		printk(BIOS_DEBUG, "Could not load ramstage.\n");
		return (void *) -1;
	}

	cache_loaded_ramstage(handoff, rmod_ram.cbmem_entry, rmod_ram.entry);

	/*
	 * Each architecture can perform additonal operations once the
	 * stage is loaded
	 */
	ramstage_size = cbmem_entry_size(rmod_ram.cbmem_entry);
	ramstage_base = (uintptr_t)cbmem_entry_start(rmod_ram.cbmem_entry);
	arch_program_segment_loaded(ramstage_base, ramstage_size);
	arch_program_loaded();

	return rmod_ram.entry;
}

void * cbfs_load_stage(struct cbfs_media *media, const char *name)
{
	struct romstage_handoff *handoff;
	const struct cbmem_entry *ramstage;
	void *entry;

	handoff = romstage_handoff_find_or_add();

	if (handoff == NULL) {
		LOG("Couldn't find or allocate romstage handoff.\n");
		return load_stage_from_cbfs(media, name, handoff);
	} else if (!handoff->s3_resume)
		return load_stage_from_cbfs(media, name, handoff);

	ramstage = cbmem_entry_find(CBMEM_ID_RAMSTAGE);

	if (ramstage == NULL)
		return load_stage_from_cbfs(media, name, handoff);

	/* S3 resume path. Load a cached copy of the loaded ramstage. If
	 * return value is NULL load from cbfs. */
	entry = load_cached_ramstage(handoff, ramstage);
	if (entry == NULL)
		return load_stage_from_cbfs(media, name, handoff);

	return entry;
}

#else

void *cbfs_load_stage_by_offset(struct cbfs_media *media, ssize_t offset)
{
	struct cbfs_stage stage;

	DEBUG("reading stage header: offset=0x%zx\n", offset);
	if (cbfs_read(media, &stage, offset, sizeof(stage)) != sizeof(stage)) {
		ERROR("ERROR: failed to read stage header\n");
		return CBFS_LOAD_ERROR;
	}

	LOG("loading stage @ 0x%llx (%d bytes), entry @ 0x%llx\n",
	    stage.load, stage.memlen, stage.entry);

	/* Stages rely the below clearing so that the bss is initialized. */
	memset((void *)(uintptr_t)stage.load, 0, stage.memlen);

	if (stage.compression == CBFS_COMPRESS_NONE) {
		if (cbfs_read(media, (void *)(uintptr_t)stage.load,
			      offset + sizeof(stage), stage.len) != stage.len) {
			ERROR("ERROR: Reading stage failed.\n");
			return CBFS_LOAD_ERROR;
		}
	} else {
		void *data = media->map(media, offset + sizeof(stage),
					stage.len);
		if (data == CBFS_MEDIA_INVALID_MAP_ADDRESS) {
			ERROR("ERROR: Mapping stage failed.\n");
			return CBFS_LOAD_ERROR;
		}
		if (cbfs_decompress(stage.compression, data,
				    (void *)(uintptr_t)stage.load, stage.len))
			return CBFS_LOAD_ERROR;
		media->unmap(media, data);
	}

	/*
	 * Each architecture can perform additonal operations once the
	 * stage is loaded
	 */
	arch_program_segment_loaded((uintptr_t)stage.load, stage.memlen);
	arch_program_loaded();

	DEBUG("stage loaded\n");

	return (void *)(uintptr_t)stage.entry;
}

void *cbfs_load_stage(struct cbfs_media *media, const char *name)
{
	struct cbfs_media default_media;
	struct cbfs_file file;
	ssize_t offset;

	init_media(&media, &default_media);

	offset = cbfs_locate_file(media, &file, name);
	if (offset < 0 || file.type != CBFS_TYPE_STAGE)
		return CBFS_LOAD_ERROR;

	return cbfs_load_stage_by_offset(media, offset);
}
#endif /* CONFIG_RELOCATABLE_RAMSTAGE */

int cbfs_execute_stage(struct cbfs_media *media, const char *name)
{
	struct cbfs_stage *stage = (struct cbfs_stage *)
		cbfs_get_file_content(media, name, CBFS_TYPE_STAGE);

	if (stage == NULL)
		return 1;

	if (ntohl(stage->compression) != CBFS_COMPRESS_NONE) {
		LOG("Unable to run %s:  Compressed file"
		       "Not supported for in-place execution\n", name);
		return 1;
	}

	LOG("run @ %p\n", (void *) (uintptr_t) ntohll(stage->entry));
	return run_address((void *)(uintptr_t)ntohll(stage->entry));
}

/* Get all the payload headers.
 * One might be tempted to implement cbfs operations as
 * - get all headers into memory
 * - scan the headers we got for a [payload, stage, file name]
 *
 * But: FLASH IO can be a very expensive operation on some systems.
 * For that reason, we keep this operation separate from anything else.
 * In other words, it is not intended to be a building block; in fact
 * the only current use is for the payload choose (Bayou) which does need
 * to scan all payloads. In future, we might decide we want to return all
 * the headers, but realistically we only need this for payloads right now
 * and it makes the bayou code less complex to just return payloads.
 * Return a count of the payloads found, up to maxentries payloads.
 */
int cbfs_payload_headers(struct cbfs_media *media,
		struct cbfs_payload_info *info,
		int maxentries)
{
	int cur;
	const char *file_name;
	uint32_t offset, align, romsize, name_len;
	const struct cbfs_header *header;
	struct cbfs_file file;
	struct cbfs_media default_media;

	cur = 0;

	if (media == CBFS_DEFAULT_MEDIA) {
		media = &default_media;
		if (init_default_cbfs_media(media) != 0) {
			ERROR("Failed to initialize default media.\n");
			return 0;
		}
	}
	if (CBFS_HEADER_INVALID_ADDRESS == (header = cbfs_get_header(media)))
		return 0;

	// Logical offset (for source media) of first file.
	offset = ntohl(header->offset);
	align = ntohl(header->align);
	romsize = ntohl(header->romsize);

	// TODO Add a "size" in CBFS header for a platform independent way to
	// determine the end of CBFS data.

	DEBUG("CBFS location: 0x%x~0x%x, align: %d\n", offset, romsize, align);

	media->open(media);
	while (cur < maxentries && offset < romsize &&
		media->read(media, &file, offset, sizeof(file))
					== sizeof(file)) {
		DEBUG("cur %d offset %08x\n", cur, offset);
		if (memcmp(CBFS_FILE_MAGIC, file.magic,
				sizeof(file.magic))) {
			uint32_t new_align = align;
			if (offset % align)
				new_align += align - (offset % align);
			ERROR("ERROR: No file header found at 0x%xx - "
				"try next aligned address: 0x%x.\n", offset,
				offset + new_align);
			offset += new_align;
			continue;
		}

		// load file name (arbitrary length).
		// Do it here so we can debug if we want. It's not expensive.
		name_len = ntohl(file.offset) - sizeof(file);
		file_name = (const char*)media->map(
				media, offset + sizeof(file), name_len);
		if (file_name == CBFS_MEDIA_INVALID_MAP_ADDRESS) {
			DEBUG("ERROR: Failed to get filename: 0x%x.\n", offset);
			continue;
		}
		DEBUG("Check :%s: type %x\n", file_name, ntohl(file.type));
		if (ntohl(file.type) == CBFS_TYPE_PAYLOAD){
			DEBUG(" - add entry 0x%x file name (%d bytes)...\n",
							offset, name_len);

			info[cur].file = file;
			info[cur].name = file_name;
			/* we need the metadata too. */
			if (media->read(media, &info[cur].payload, offset,
				sizeof(info[cur].payload)) !=
						sizeof(info[cur].payload)){
				ERROR("ERROR: Failed to get payload"
					"info for %s@0x%x.\n",
					file_name, offset);
				continue;
			}
			cur++;
		}
		// Move to next file.
		offset += ntohl(file.len) + ntohl(file.offset);
		DEBUG("offset moves to %08x\n", offset);
		if (offset % align)
			offset += align - (offset % align);
	}
	media->close(media);

	return cur;
}

#if !CONFIG_ALT_CBFS_LOAD_PAYLOAD
void *cbfs_load_payload(struct cbfs_media *media, const char *name)
{
	struct cbfs_payload *payload;

	payload = vboot_get_payload(NULL);
	if (payload != NULL)
		return payload;

	payload = (struct cbfs_payload *)cbfs_get_file_content(
			media, name, CBFS_TYPE_PAYLOAD);
	if (payload)
		printk(BIOS_DEBUG, "Booting payload %s from cbfs\n", name);
	return payload;
}
#endif

/* Simple buffer */

void *cbfs_simple_buffer_map(struct cbfs_simple_buffer *buffer,
			     struct cbfs_media *media,
			     size_t offset, size_t count) {
	void *address = buffer->buffer + buffer->allocated;;
	DEBUG("simple_buffer_map(offset=%zd, count=%zd): "
	      "allocated=%zd, size=%zd, last_allocate=%zd\n",
	    offset, count, buffer->allocated, buffer->size,
	    buffer->last_allocate);
	if (buffer->allocated + count > buffer->size) {
		ERROR("simple_buffer: no room to map %zd bytes from %#zx\n",
		      count, offset);
		return CBFS_MEDIA_INVALID_MAP_ADDRESS;
	}
	if (media->read(media, address, offset, count) != count) {
		ERROR("simple_buffer: fail to read %zd bytes from 0x%zx\n",
		      count, offset);
		return CBFS_MEDIA_INVALID_MAP_ADDRESS;
	}
	buffer->allocated += count;
	buffer->last_allocate = count;
	return address;
}

void *cbfs_simple_buffer_unmap(struct cbfs_simple_buffer *buffer,
			       const void *address) {
	// TODO Add simple buffer management so we can free more than last
	// allocated one.
	DEBUG("simple_buffer_unmap(address=0x%p): "
	      "allocated=%zd, size=%zd, last_allocate=%zd\n",
	    address, buffer->allocated, buffer->size,
	    buffer->last_allocate);
	if ((buffer->buffer + buffer->allocated - buffer->last_allocate) ==
	    address) {
		buffer->allocated -= buffer->last_allocate;
		buffer->last_allocate = 0;
	}
	return NULL;
}

/**
 * run_address is passed the address of a function taking no parameters and
 * jumps to it, returning the result.
 * @param f the address to call as a function.
 * @return value returned by the function.
 */

int run_address(void *f)
{
	int (*v) (void);
	v = f;
	return v();
}

#endif
