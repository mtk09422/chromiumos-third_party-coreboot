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

#ifndef NOR_IF_H
#define NOR_IF_H

#include <stdint.h>
#include <cbfs.h>
#include <soc/addressmap.h>
#include <soc/blkdev.h>

#define MAX_FLASH_COUNTER  1
#define SFLASH_WRBUF_SIZE		128
#define SFLASHNAME_LENGTH		48
#define RETURN_SUCCESS			0
#define RETUEN_ERROR			1
/* register Offset */

struct mt8173_nor_regs {
	u32 sflash_cmd_reg;
	u32 sflash_cnt_reg;
	u32 sflash_rdsr_reg;
	u32 sflash_rdata_reg;
	u32 sflash_radr0_reg;
	u32 sflash_radr1_reg;
	u32 sflash_radr2_reg;
	u32 sflash_wdata_reg;
	u32 sflash_prgdata0_reg;
	u32 sflash_prgdata1_reg;
	u32 sflash_prgdata2_reg;
	u32 sflash_prgdata3_reg;
	u32 sflash_prgdata4_reg;
	u32 sflash_prgdata5_reg;
	u32 sflash_shreg0_reg;
	u32 sflash_shreg1_reg;
	u32 sflash_shreg2_reg;
	u32 sflash_shreg3_reg;
	u32 sflash_shreg4_reg;
	u32 sflash_shreg5_reg;
	u32 sflash_shreg6_reg;
	u32 sflash_shreg7_reg;
	u32 sflash_shreg8_reg;
	u32 sflash_shreg9_reg;
	u32 sflash_cfg1_reg;
	u32 sflash_cfg2_reg;
	u32 sflash_cfg3_reg;
	u32 reserved0;
	u32 sflash_status0_reg;
	u32 sflash_status1_reg;
	u32 sflash_status2_reg;
	u32 sflash_status3_reg;
	u32 reserved1;
	u32 sflash_flhcfg_reg;
	u32 reserved2[3];
	u32 sflash_sf_time_reg;
	u32 sflash_pp_data_reg;
	u32 sflash_prebuf_stus_reg;
	u32 sflash_delsel0_reg;
	u32 sflash_delsel1_reg;
	u32 sflash_sf_intrstus_reg;
	u32 sflash_sf_intren_reg;
	u32 reserved3[2];
	u32 sflash_chksum_ctl_reg;
	u32 sflash_chksum_reg;
	u32 sflash_cmd2_reg;
	u32 sflash_wrprot_reg;
	u32 sflash_radr3_reg;
	u32 sflash_read_dual_reg;
	u32 sflash_delsel2_reg;
	u32 sflash_delsel3_reg;
	u32 sflash_delsel4_reg;
};
static struct mt8173_nor_regs * const mt8173_nor = (void *)SFLASH_REG_BASE;

typedef struct {
	u8 menu_id;
	u8 dev_id_1;
	u8 dev_id_2;
	u8 pp_type;
	u32 chip_size;
	u32 sector_size;
	u32 erase_time_out_msec;
	u8 wren_cmd;
	u8 wrdi_cmd;
	u8 rdsr_cmd;
	u8 wrsr_cmd;
	u8 read_cmd;
	u8 fast_read_cmd;
	u8 read_id_cmd;
	u8 sec_erase_cmd;
	u8 chip_erase_cmd;
	u8 page_program_cmd;
	u8 aai_program_cmd;
	u8 dual_read_cmd;
	u8 protection;
	char pc_flash_str[SFLASHNAME_LENGTH];
} SFLASHHW_VENDOR_T;

typedef struct {
	u8 menu_id;
	u8 dev_id_1;
	u8 dev_id_2;
	u32 chip_size;
	u32 sector_size;
	u32 sector_counter;
	char pc_flash_str[SFLASHNAME_LENGTH];
} SFLASH_CHIPINFO_T;

typedef struct {
	u8 flash_counter;
	SFLASH_CHIPINFO_T ar_flash_info[MAX_FLASH_COUNTER];
} SFLASH_INFO_T;

/* Public functions */

int init_mt8173_nor_cbfs_media(struct cbfs_media *media);
u32 nor_init_device(void);
int sflash_init(void);
#endif /* DRV_NOR_H */

