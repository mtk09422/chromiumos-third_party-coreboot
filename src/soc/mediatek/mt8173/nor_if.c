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

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arch/io.h>
#include <console/console.h>
#include <soc/blkdev.h>
#include <soc/mt8173.h>
#include <soc/nor_if.h>

struct blkdev nor_block_dev;
static SFLASH_INFO_T sflash_info;

#define SFLASH_POLLINGREG_COUNTER	500000
#define SFLASH_WRITEBUSY_TIMEOUT	500000
#define SFLASH_HW_ALIGNMENT		4
#define DUAL_READ	0x01
#define DUAL_IO		0x02
#define QUAD_READ	0x04
#define QUAD_IO		0x08

/* Type definitions */
static SFLASHHW_VENDOR_T array_vendor_flash[] =
{
	{ 0xC2, 0x20, 0x19, 0x0, 0x2000000, 0x10000, 60000,  0x06, 0x04, 0x05, 0x01, 0x03, 0x0B, 0x9F, 0xD8, 0xC7, 0x02, 0x00, 0x3b, 0x00,	"(25L256)" },
	{ 0xC2, 0x20, 0x1A, 0x0, 0x2000000, 0x10000, 60000,  0x06, 0x04, 0x05, 0x01, 0x03, 0x0B, 0x9F, 0xD8, 0xC7, 0x02, 0x00, 0x3b, 0x00,	"(25L512)" },
	{ 0xC2, 0x25, 0x39, 0x0, 0x2000000, 0x10000, 60000,  0x06, 0x04, 0x05, 0x01, 0x03, 0x0B, 0x9F, 0xD8, 0xC7, 0x02, 0x00, 0x3b, 0x00,	"(25U256)" },
	{ 0xEF, 0x30, 0x15, 0x0, 0x200000,  0x10000, 60000,  0x06, 0x04, 0x05, 0x01, 0x03, 0x0B, 0x9F, 0xD8, 0xC7, 0x02, 0x00, 0x00, 0x00,	"(W25X16)" },
	{ 0xEF, 0x30, 0x16, 0x0, 0x400000,  0x10000, 60000,  0x06, 0x04, 0x05, 0x01, 0x03, 0x0B, 0x9F, 0xD8, 0xC7, 0x02, 0x00, 0x00, 0x00,	"(W25X32)" },
	{ 0xEF, 0x60, 0x16, 0x0, 0x400000,  0x10000, 60000,  0x06, 0x04, 0x05, 0x01, 0x03, 0x0B, 0x9F, 0xD8, 0xC7, 0x02, 0x00, 0x00, 0x00,	"(W25X32)" },
	{ 0xEF, 0x30, 0x17, 0x0, 0x800000,  0x10000, 60000,  0x06, 0x04, 0x05, 0x01, 0x03, 0x0B, 0x9F, 0xD8, 0xC7, 0x02, 0x00, 0x00, 0x00,	"(W25X64)" },
	{ 0xEF, 0x40, 0x19, 0x0, 0x2000000, 0x10000, 60000,  0x06, 0x04, 0x05, 0x01, 0x03, 0x0B, 0x9F, 0xD8, 0xC7, 0x02, 0x00, 0x00, 0x00,	"(W25Q256FV)" },
	{ 0xEF, 0x40, 0x15, 0x0, 0x200000,  0x10000, 60000,  0x06, 0x04, 0x05, 0x01, 0x03, 0x0B, 0x9F, 0xD8, 0xC7, 0x02, 0x00, 0xBB, 0x00,	"(W25Q16CV)" },
	{ 0xEF, 0x40, 0x16, 0x0, 0x400000,  0x10000, 60000,  0x06, 0x04, 0x05, 0x01, 0x03, 0x0B, 0x9F, 0xD8, 0xC7, 0x02, 0x00, 0x00, 0x00,	"(W25Q32BV)" },
	{ 0xEF, 0x40, 0x17, 0x0, 0x800000,  0x10000, 60000,  0x06, 0x04, 0x05, 0x01, 0x03, 0x0B, 0x9F, 0xD8, 0xC7, 0x02, 0x00, 0xBB, 0x00,	"(W25Q64BV)" },
	{ 0xEF, 0x40, 0x18, 0x0, 0x1000000, 0x10000, 60000,  0x06, 0x04, 0x05, 0x01, 0x03, 0x0B, 0x9F, 0xD8, 0xC7, 0x02, 0x00, 0x00, 0x00,	"(W25Q128BV)" },
	{ 0xFF, 0xFF, 0x16, 0x0, 0x400000,  0x10000, 60000,  0x06, 0x04, 0x05, 0x01, 0x03, 0x0B, 0x9F, 0xD8, 0xC7, 0x02, 0x00, 0x00, 0x00,	"32Mb" },
	{ 0xFF, 0xFF, 0x17, 0x0, 0x800000,  0x10000, 60000,  0x06, 0x04, 0x05, 0x01, 0x03, 0x0B, 0x9F, 0xD8, 0xC7, 0x02, 0x00, 0x00, 0x00,	"64Mb" },
	{ 0xFF, 0xFF, 0x18, 0x0, 0x1000000, 0x10000, 60000,  0x06, 0x04, 0x05, 0x01, 0x03, 0x0B, 0x9F, 0xD8, 0xC7, 0x02, 0x00, 0x00, 0x00,	"128Mb" },
	{ 0x00, 0x00, 0x00, 0x0, 0x000000,  0x00000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	"0 Device" },
};

#define LoWord(d)     ((u16)(d & 0x0000ffffL))
#define HiWord(d)     ((u16)((d >> 16) & 0x0000ffffL))
#define LoByte(w)     ((uint8_t)(w & 0x00ff))
#define HiByte(w)     ((uint8_t)((w >> 8) & 0x00ff))
#define MidWord(d)    ((u16)((d >> 8) & 0x00ffff))

/* Static variables  */
SFLASHHW_VENDOR_T array_flash_chip[MAX_FLASH_COUNTER];
static uint32_t chip_counter;
static uint32_t sflash_isr_status;
static unsigned char sf_isr_en = 1;
static unsigned char do_protect = 0;
unsigned char write_protect = 1;
static unsigned char aai_write = 0;
uint32_t nor_partial_protection = 0;
static u8 menu_id, dev_id_1, dev_id_2;

#define EFUSE_HW2_RES0_IO_33V	0x00000008
#define EFUSE_HW2_RES0			((volatile unsigned int*)((uintptr_t)(EFUSE_BASE + 0x04B0)))
#define efuse_is_io_33v()		(((*EFUSE_HW2_RES0) & EFUSE_HW2_RES0_IO_33V) ? 0 : 1) // 0 : 3.3v (MT8130 default), 1 : 1.8v

#define gpio_cfg_bit16(reg, field, val) \
	do {	\
		unsigned short tv = (unsigned short)(*(volatile unsigned short*)(reg));	\
		tv &= ~(field);	\
		tv |= val; \
		(*(volatile unsigned short*)(reg) = (unsigned short)(tv)); \
	} while (0)

#define EXMD_CTRL		(GPIO_BASE + 0x0dc0)
#define GPIO_DRVSEL(x)		(GPIO_BASE + 0x0B00 + 0x10 * x)
#define MODE_BIT(ID)		(0x7 << (ID * 3))
#define MODE_DAT(ID, VAL)	(VAL << (ID * 3))
#define PULL_BIT(ID)		(0x1 << (ID))
#define PULL_DAT(ID, VAL)	(VAL << (ID))
#define DRVS_BIT(ID)		(0xf << (ID * 4))
#define DRVS_DAT(ID, VAL)	(VAL << (ID * 4))
#define MSDC_DRV_DAT(VAL)	(VAL << 8)
#define GPIO_PULLSEL(x)		(GPIO_BASE + 0x0200 + 0x10 * (x - 1))

#define EXMD_TDSEL		(0xf << 0)
#define EXMD_RDSEL		(0x3f << 4)
#define EXMD_BIAS		(0xf << 12)
#define GPIO_MODE(x)	(GPIO_BASE + 0x0600 + 0x10 * (x - 1))   /* x = 1 ~ 28 */

typedef enum {
	GPIO_MODE0    = 0,
	GPIO_MODE1    = 1,
	GPIO_MODE2    = 2,
	GPIO_MODE3    = 3,
	GPIO_MODE4    = 4,
	GPIO_MODE5    = 5,
	GPIO_MODE6    = 6,
	GPIO_MODE7    = 7,
} GMODE_ID;

static void sf_nor_gpio_init(void)
{
	printk(BIOS_DEBUG, "NOR GPIO Init here !\n");
	// driving strength
	gpio_cfg_bit16(GPIO_DRVSEL(2),  DRVS_BIT(4), DRVS_DAT(4, 0x2));
	gpio_cfg_bit16(GPIO_DRVSEL(3),  DRVS_BIT(0), DRVS_DAT(0, 0x2));
	// Pull up
	gpio_cfg_bit16(GPIO_PULLSEL(1),  PULL_BIT(4) | PULL_BIT(5) | PULL_BIT(6) | PULL_BIT(7) | PULL_BIT(8) | PULL_BIT(9),
		       PULL_DAT(4, 0x1) |               // 0x10005200[4]     = 0x1;  // SFWP_B
		       PULL_DAT(5, 0x1) |               // 0x10005200[5]     = 0x1;  // SFOUT
		       PULL_DAT(6, 0x1) |               // 0x10005200[6]     = 0x1;  // SFCS0
		       PULL_DAT(7, 0x1) |               // 0x10005200[7]     = 0x1;  // SFHOLD
		       PULL_DAT(8, 0x1) |               // 0x10005200[8]     = 0x1;  // SFIN
		       PULL_DAT(9, 0x1) );              // 0x10005200[9]     = 0x1;  // SFCK

	gpio_cfg_bit16(EXMD_CTRL, EXMD_TDSEL, 0xa);
	if (efuse_is_io_33v()) 
		gpio_cfg_bit16(EXMD_CTRL, EXMD_RDSEL, 0x0c0);
		
	if (efuse_is_io_33v()) 
		gpio_cfg_bit16(EXMD_CTRL, EXMD_BIAS, 0x5000);
	else
		printk(BIOS_DEBUG, "IO isnot 3.3V !\n");

	gpio_cfg_bit16(GPIO_MODE(1), MODE_BIT(4),
		       MODE_DAT(4, GPIO_MODE6) );               // 0x10005600[14:12]  = 0x6;  // SFWP_B
	gpio_cfg_bit16(GPIO_MODE(2), MODE_BIT(0) | MODE_BIT(1) | MODE_BIT(2) | MODE_BIT(3) | MODE_BIT(4),
		       MODE_DAT(0, GPIO_MODE6) |                // 0x10005610[2:0]    = 0x6;  // SFOUT
		       MODE_DAT(1, GPIO_MODE6) |                // 0x10005610[5:3]    = 0x6;  // SFCS0
		       MODE_DAT(2, GPIO_MODE6) |                // 0x10005610[8:6]    = 0x6;  // SFHOLD
		       MODE_DAT(3, GPIO_MODE6) |                // 0x10005610[11:9]   = 0x6;  // SFIN
		       MODE_DAT(4, GPIO_MODE6) );               // 0x10005610[14:12]  = 0x6;  // SFCK
}

static void send_flash_command(uint32_t val)
{
	write32(&mt8173_nor->sflash_cmd_reg, val);
}

static int polling_reg(uint32_t compare)
{
	uint32_t polling;
	uint32_t reg;

	polling = 0;
	while (1) {
		reg = read32(&mt8173_nor->sflash_cmd_reg);
		if (0x00 == (reg & compare))
			break;
		polling++;
		if (polling > SFLASH_POLLINGREG_COUNTER)
			return RETUEN_ERROR;
	}
	return RETURN_SUCCESS;
}

static void set_flash_counterreg(uint32_t counter)
{
	u8 reg;

	reg = read8(&mt8173_nor->sflash_cfg1_reg);
	reg &= (~0xC);
	if (counter == 1)
		write8(&mt8173_nor->sflash_cfg1_reg, reg);
	else
		assert(0);
}
static int sflashhw_get_set_id(uint32_t index, uint8_t *manufacture_id, uint8_t *device_id_1, uint8_t *device_id_2)
{
	write8(&mt8173_nor->sflash_prgdata5_reg, 0x90);
	write8(&mt8173_nor->sflash_prgdata4_reg, 0x00);
	write8(&mt8173_nor->sflash_prgdata3_reg, 0x00);
	write8(&mt8173_nor->sflash_prgdata2_reg, 0x00);
	write8(&mt8173_nor->sflash_prgdata1_reg, 0x00);
	write8(&mt8173_nor->sflash_prgdata0_reg, 0x00);
	write8(&mt8173_nor->sflash_cnt_reg, 0x30);

	if (index == 0)
		send_flash_command(0x4);
	else
		assert(0);
	if ( polling_reg( 0x04) != 0) // timeout
		return RETUEN_ERROR;
	if (device_id_2 != 0)
		*device_id_2 = 0x10;
	if (device_id_1 != 0)
		*device_id_1 = read8(&mt8173_nor->sflash_shreg0_reg);
	if (manufacture_id != 0)
		*manufacture_id = read8(&mt8173_nor->sflash_shreg1_reg);
	send_flash_command(0x0);
	return RETURN_SUCCESS;
}

static int sflashhw_getid(uint32_t index, uint8_t *manufacture_id, uint8_t *device_id_1, uint8_t *device_id_2)
{
	write8(&mt8173_nor->sflash_prgdata5_reg, 0x9F);
	write8(&mt8173_nor->sflash_prgdata4_reg, 0x00);
	write8(&mt8173_nor->sflash_prgdata3_reg, 0x00);
	write8(&mt8173_nor->sflash_prgdata2_reg, 0x00);
	write8(&mt8173_nor->sflash_cnt_reg, 32);

	if (index == 0)
		send_flash_command(0x4);
	else
		assert(0);
	if ( polling_reg(0x04) != 0)
		return RETUEN_ERROR;

	if (device_id_2 != 0)
		*device_id_2 = read8(&mt8173_nor->sflash_shreg0_reg);
	if (device_id_1 != 0)
		*device_id_1 = read8(&mt8173_nor->sflash_shreg1_reg);
	if (manufacture_id != 0)
		*manufacture_id = read8(&mt8173_nor->sflash_shreg2_reg);
	send_flash_command(0x0);
	if ((manufacture_id != 0) && (device_id_1 != 0)) {
		if ((*manufacture_id == 0xFF) || (*device_id_1 == 0x00)) {
			printk(BIOS_DEBUG, "Try not JEDEC ID\n");
			if (sflashhw_get_set_id(index, manufacture_id, device_id_1, device_id_2) != 0)
				return RETUEN_ERROR;
		}
	}
	return RETURN_SUCCESS;
}

static int do_identify(uint32_t index, uint32_t *vendor)
{
	uint32_t i;

	if (vendor == 0)
		return RETUEN_ERROR;

	*vendor = 0xFFFFFFFF;
	if (sflashhw_getid(index, &menu_id, &dev_id_1, &dev_id_2) != 0)
		return RETUEN_ERROR;

	printk(BIOS_DEBUG, "Flash index :%d,menu_id: 0x%X, DeviceID1: 0x%X, DeviceID2: 0x%X\n", index, menu_id, dev_id_1, dev_id_2);
	i = 0;
	while (array_vendor_flash[i].menu_id != (u8)0x0) {
		if ( (array_vendor_flash[i].menu_id == menu_id) &&
		     (array_vendor_flash[i].dev_id_1 == dev_id_1) &&
		     (array_vendor_flash[i].dev_id_2 == dev_id_2)) {
			*vendor = i;
			return RETURN_SUCCESS;
		}
		i++;
	}
	return RETUEN_ERROR;
}

static int wb_enable(void)
{
	u32 polling;
	u8 reg;
	u8 temp = 0x01;
	if (aai_write == 1) {
		if (array_flash_chip[0].aai_program_cmd != 0xAF)
			temp |= 0x40;
	}
	write8(&mt8173_nor->sflash_cfg2_reg, temp);
	polling = 0;
	while (1) {
		reg = read8(&mt8173_nor->sflash_cfg2_reg);
		if (0x01 == (reg & 0x1))
			break;
		polling++;
		if (polling > SFLASH_POLLINGREG_COUNTER)
			return RETUEN_ERROR;
	}
	return RETURN_SUCCESS;
}

static int wb_disable(void)
{
	uint32_t polling;
	uint8_t reg;
	uint8_t temp = 0x00;
	write8(&mt8173_nor->sflash_cfg2_reg, temp);
	polling = 0;
	while (1) {
		reg = read8(&mt8173_nor->sflash_cfg2_reg);
		if (temp == (reg & 0xF))
			break;
		polling++;
		if (polling > SFLASH_POLLINGREG_COUNTER)
			return RETUEN_ERROR;
	}
	return RETURN_SUCCESS;
}

static int sflashhw_read_flash_status(uint32_t index, uint8_t *value)
{
	if (value == 0)
		return RETUEN_ERROR;

	if (index == 0)
		send_flash_command(0x2);
	else
		assert(0);
	if (polling_reg( 0x2) != 0)
		return RETUEN_ERROR;

	*value = read8(&mt8173_nor->sflash_rdsr_reg);
	return RETURN_SUCCESS;
}


static int wait_for_write_busy(uint32_t index, uint32_t Timeout)
{
	uint32_t counter;
	uint8_t reg;
	counter = 0;
	while (1) {
		if (sflashhw_read_flash_status(index, &reg) != 0)
			return RETUEN_ERROR;

		if (0 == (reg & 0x1))
			break;
		counter++;
		if (counter > Timeout)
			return RETUEN_ERROR;
	}
	return RETURN_SUCCESS;
}

static int set_flash_write_enable(uint32_t index)
{

	write8(&mt8173_nor->sflash_prgdata5_reg, array_flash_chip[index].wren_cmd);
	write8(&mt8173_nor->sflash_cnt_reg, 8);

	if (index == 0)
		send_flash_command(0x4);
	else
		assert(0);
	if (polling_reg(0x04) != 0)
		return RETUEN_ERROR;

	return RETURN_SUCCESS;
}

static void sflashhw_get_flash_info(SFLASH_INFO_T *pr_info)
{
	uint32_t i;

	if (pr_info == 0)
		return;

	pr_info->flash_counter = (u8)(chip_counter & 0xFF);
	for (i = 0; i < chip_counter; i++) {
		pr_info->ar_flash_info[i].menu_id = array_flash_chip[i].menu_id;
		pr_info->ar_flash_info[i].dev_id_1 = array_flash_chip[i].dev_id_1;
		pr_info->ar_flash_info[i].dev_id_2 = array_flash_chip[i].dev_id_2;
		pr_info->ar_flash_info[i].chip_size = array_flash_chip[i].chip_size;
		pr_info->ar_flash_info[i].sector_size = array_flash_chip[i].sector_size;
		pr_info->ar_flash_info[i].sector_counter =
			array_flash_chip[i].chip_size / array_flash_chip[i].sector_size;

		memcpy(pr_info->ar_flash_info[i].pc_flash_str, array_flash_chip[i].pc_flash_str, SFLASHNAME_LENGTH);
	}

}

static int sflashhw_read(uint32_t addr, uint32_t length, u8* buffer)
{
	uint32_t i;

	if (buffer == 0)
		return RETUEN_ERROR;

	write8(&mt8173_nor->sflash_cfg2_reg, 0x0C);
	write8(&mt8173_nor->sflash_read_dual_reg, read8(&mt8173_nor->sflash_read_dual_reg) & 0x10);
	{
		write8(&mt8173_nor->sflash_read_dual_reg, read8(&mt8173_nor->sflash_read_dual_reg) | 0x0);
	}
	write8(&mt8173_nor->sflash_radr2_reg, LoByte(HiWord(addr)));
	write8(&mt8173_nor->sflash_radr1_reg, HiByte(LoWord(addr)));
	write8(&mt8173_nor->sflash_radr0_reg, LoByte(LoWord(addr)));

	for (i = 0; i < length; i++) {
		send_flash_command(0x81);
		if ( polling_reg( 0x01) != 0) {
			printk(BIOS_DEBUG, "...cdd sflashhw_read timeout !\n");
			return RETUEN_ERROR;
		}
		buffer[i] = read8(&mt8173_nor->sflash_rdata_reg);
	}
	write8(&mt8173_nor->sflash_read_dual_reg, read8(&mt8173_nor->sflash_read_dual_reg) & 0x10);

	return RETURN_SUCCESS;
}


static int execute_write_cmd(uint32_t index)
{
	u8 reg;

	if (sf_isr_en) {
		sflash_isr_status = 0;
		if (aai_write == 0) {
			write32(&mt8173_nor->sflash_sf_intrstus_reg, 0x10);    // Clear interrupt
			write32(&mt8173_nor->sflash_sf_intren_reg, 0x10);
		}else {
			write32(&mt8173_nor->sflash_sf_intrstus_reg, 0x40);    // Clear interrupt
			write32(&mt8173_nor->sflash_sf_intren_reg, 0x40);
		}
	}
	if (index == 0) {
		if (aai_write == 0)
			send_flash_command(0x10);
		else
			write8(&mt8173_nor->sflash_cmd2_reg, 0x01);
	}else {
		assert(0);
	}

	if (sf_isr_en) {
		while (((aai_write == 0) && (!(sflash_isr_status & 0x10))) || ((aai_write == 1) && (!(sflash_isr_status & 0x40)))) {
		}
	}else {
		while (1) {
			if (aai_write == 0) {
				reg = read8(&mt8173_nor->sflash_cmd_reg);
				if (0x0 == (reg & 0x10))
					break;
			}else {
				reg = read8(&mt8173_nor->sflash_cmd2_reg);
				if (0x0 == (reg & 0x01))
					break;
			}
		}
	}
	if (sf_isr_en)
		write32(&mt8173_nor->sflash_sf_intren_reg, 0x0);   // Disable interrupt
	send_flash_command(0x0);
	return RETURN_SUCCESS;
}

static int write_buffer(uint32_t index, uint32_t addr, uint32_t length, const uint8_t* buf)
{
	uint32_t i, j, buf_idx, data;

	if (buf == 0)
		return RETUEN_ERROR;

	assert(length <= SFLASH_WRBUF_SIZE);
	assert((addr % SFLASH_HW_ALIGNMENT) == 0);
	assert((length % SFLASH_HW_ALIGNMENT) == 0);
	write8(&mt8173_nor->sflash_radr2_reg, LoByte(HiWord(addr)));
	write8(&mt8173_nor->sflash_radr1_reg, HiByte(LoWord(addr)));
	write8(&mt8173_nor->sflash_radr0_reg, LoByte(LoWord(addr)));

	buf_idx = 0;
	for (i = 0; i < length; i += 4) {
		for (j = 0; j < 4; j++) {
			(*((u8*)&data + j)) = buf[buf_idx];
			buf_idx++;
		}
		write32(&mt8173_nor->sflash_pp_data_reg, data);
	}
	if (execute_write_cmd(index) != 0)
		return RETUEN_ERROR;

	if (wait_for_write_busy(index, SFLASH_WRITEBUSY_TIMEOUT) != 0)
		return RETUEN_ERROR;

	return RETURN_SUCCESS;
}
static int write_single_byte(uint32_t index, uint32_t addr, uint8_t data)
{
	write8(&mt8173_nor->sflash_radr2_reg, LoByte(HiWord(addr)));
	write8(&mt8173_nor->sflash_radr1_reg, HiByte(LoWord(addr)));
	write8(&mt8173_nor->sflash_radr0_reg, LoByte(LoWord(addr)));
	write8(&mt8173_nor->sflash_wdata_reg, data);
	if (execute_write_cmd(index) != 0)
		return RETUEN_ERROR;

	if (wait_for_write_busy(index, SFLASH_WRITEBUSY_TIMEOUT) != 0)
		return RETUEN_ERROR;

	return RETURN_SUCCESS;
}

static int sflashhw_write_sector(uint32_t index, uint32_t addr, uint32_t length,
			 const u8* buffer)
{
	uint32_t i, counter, page_align;

	if (index >= MAX_FLASH_COUNTER) {
		printk(BIOS_DEBUG, "Nor Flash index is out fo Max Flash counter\n");
		return RETUEN_ERROR;
	}
	assert(array_flash_chip[index].menu_id != 0x00);
	assert(length <= array_flash_chip[index].sector_size);

	if (length == 0)
		return RETURN_SUCCESS;

	if (buffer == 0)
		return RETUEN_ERROR;

	if (set_flash_write_enable(index) != 0)
		return RETUEN_ERROR;

	page_align = addr % SFLASH_WRBUF_SIZE;
	if (page_align != 0) {
		aai_write = 0;
		for (i = 0; i < (SFLASH_WRBUF_SIZE - page_align); i++) {
			if (write_single_byte(index, addr, *buffer) != 0)
				return RETUEN_ERROR;

			addr++;
			buffer++;
			length--;

			if (length == 0)
				return RETURN_SUCCESS;
		}
	}
	if (wb_enable() != 0)
		return RETUEN_ERROR;

	if (array_flash_chip[index].pp_type == 0) {
		while ((int)length > 0) {
			if (length >= SFLASH_WRBUF_SIZE)
				counter = SFLASH_WRBUF_SIZE;
			else
				/* Not write-buffer alignment */
				break;
			if (write_buffer(index, addr, counter, buffer) != 0) {
				printk(BIOS_DEBUG, "Write flash error!! faddr = 0x%x, length = 0x%x\n", addr, counter);
				if (wb_disable() != 0)
					return RETUEN_ERROR;

				return RETUEN_ERROR;
			}
			length -= counter;
			addr += counter;
			buffer += counter;

		}
		if (wb_disable() != 0)
			return RETUEN_ERROR;
	}
	if ((int)length > 0) {
		aai_write = 0;
		for (i = 0; i < length; i++) {
			if (write_single_byte(index, addr, *buffer) != 0) {
				if (wb_disable() != 0)
					return RETUEN_ERROR;

				return RETUEN_ERROR;
			}
			addr++;
			buffer++;
		}
	}
	return RETURN_SUCCESS;
}

static void sflashhw_write_sf_protect(uint32_t val)
{
	printk(BIOS_DEBUG, "...cdd sflashhw_write_sf_protect val = 0x%x!\n", val);
	write32(&mt8173_nor->sflash_wrprot_reg, val);
}

static uint32_t sflashhw_read_sf_protect(void)
{
	return read32(&mt8173_nor->sflash_wrprot_reg);
}

static int sflashhw_write_protect(uint32_t index, unsigned char enable)
{
	if (index >= MAX_FLASH_COUNTER) {
		printk(BIOS_DEBUG, "Nor Flash index is out fo Max Flash counter\n");
		return RETUEN_ERROR;
	}
	if (write_protect) {
		if (!do_protect)
			return RETURN_SUCCESS;

		if (wait_for_write_busy(index, SFLASH_WRITEBUSY_TIMEOUT) != 0)
			return RETUEN_ERROR;

		if (set_flash_write_enable(index) != 0)
			return RETUEN_ERROR;

		if (enable)
			write8(&mt8173_nor->sflash_prgdata5_reg, 0x3C);
		else{
			if (nor_partial_protection)
				write32(&mt8173_nor->sflash_prgdata5_reg, array_flash_chip[index].protection);
			else
				write32(&mt8173_nor->sflash_prgdata5_reg, 0x0);
		}
		write8(&mt8173_nor->sflash_cnt_reg, 8);

		if (index == 0)
			send_flash_command(0x20);
		else
			assert(0);

		if (polling_reg( 0x20) != 0)
			return RETUEN_ERROR;

		if (wait_for_write_busy(index, SFLASH_WRITEBUSY_TIMEOUT) != 0)
			return RETUEN_ERROR;
	}

	return RETURN_SUCCESS;
}

static int sflash_unlock(void)
{
	sflashhw_write_sf_protect(0x30);
	if (sflashhw_read_sf_protect())
		return RETURN_SUCCESS;
	else
		return RETUEN_ERROR;
}

static int sflashhw_init(void)
{
	uint32_t loop, VendorIdx;

	sflashhw_write_sf_protect(0x30);
	memset((void*)array_flash_chip, 0x0, MAX_FLASH_COUNTER * sizeof(SFLASHHW_VENDOR_T));
	sf_nor_gpio_init();

	chip_counter = 0;
	for (loop = 0; loop < MAX_FLASH_COUNTER; loop++) {
		if (do_identify(loop, &VendorIdx) != 0)
			continue;
		memcpy((void*)&(array_flash_chip[chip_counter]), (void*)&(array_vendor_flash[VendorIdx]), sizeof(SFLASHHW_VENDOR_T));
		chip_counter++;
		printk(BIOS_DEBUG, "Detect flash #%d: %s\n", loop, array_flash_chip[loop].pc_flash_str);
	}
	if (chip_counter == 0) {
		printk(BIOS_DEBUG, "There is no flash!\n");
		return RETUEN_ERROR;
	}
	/* Set flash number register */
	set_flash_counterreg(chip_counter);
#if !defined (CC_MTK_LOADER)
	write32(&mt8173_nor->sflash_sf_intren_reg, 0x50);
#endif
	sf_isr_en = 0;
	sflash_isr_status = 0;
	//Guarantee every time nor init can WriteProtect

	write_protect = 1;
	if (write_protect) {
		do_protect = 1;
		if (do_protect) {
			for (loop = 0; loop < chip_counter; loop++) {
				if (sflashhw_write_protect(loop, 1) != 0)
					return RETUEN_ERROR;
			}
		}
	}

	return RETURN_SUCCESS;
}

int sflash_init()
{
	sflash_unlock();
	memset((void*)&sflash_info, 0x0, sizeof(SFLASH_INFO_T));
	if (sflashhw_init() != 0)
		return RETUEN_ERROR;

	sflashhw_get_flash_info(&sflash_info);

	return RETURN_SUCCESS;
}

static int sflash_write(uint32_t offset, uint64_t mem_ptr, uint32_t byte_counter)
{
	uint8_t *buffer;
	uint32_t i, j, write_addr, length, write_byte, chip_offset, sect_start, sect_end;

	if (sflash_init() != 0)
		return RETUEN_ERROR;

	buffer = (uint8_t*)mem_ptr;
	write_addr = offset;
	write_byte = byte_counter;
	chip_offset = 0;
	sect_start = 0;
	sect_end = 0;

	for (i = 0; i < sflash_info.flash_counter; i++) {
		if (sflashhw_write_protect(i, 0) != 0) {
			printk(BIOS_DEBUG, "Disable Flash write protect fail!\n");
			return RETUEN_ERROR;
		}
		for (j = 0; j < sflash_info.ar_flash_info[i].sector_counter; j++) {
			sect_end = sect_start + sflash_info.ar_flash_info[i].sector_size;
			if ((sect_start <= write_addr) && (write_addr < sect_end)) {
				length = sflash_info.ar_flash_info[i].sector_size - (write_addr - sect_start);
				if (length >= write_byte)
					length = write_byte;
				if (sflashhw_write_sector(i, write_addr - chip_offset, length, buffer) != 0) {
					printk(BIOS_DEBUG, "Write flash error !\n");
					if (sflashhw_write_protect(i, 1) != 0) {
						printk(BIOS_DEBUG, "Enable flash write protect fail!\n");
						return RETUEN_ERROR;
					}
					return RETUEN_ERROR;
				}
				write_addr += length;
				write_byte -= length;
				buffer += length;
				if (write_byte == 0)
					break;
			}
			sect_start += sflash_info.ar_flash_info[i].sector_size;
		}
		chip_offset += sflash_info.ar_flash_info[i].chip_size;
		if (sflashhw_write_protect(i, 1) != 0) {
			printk(BIOS_DEBUG, "Enable flash write protect fail!\n");
			return RETUEN_ERROR;
		}
	}
	return RETURN_SUCCESS;
}

static int sflash_read(uint32_t offset, uint64_t mem_ptr, uint32_t byte_counter)
{
	if (sflashhw_read(offset, byte_counter, (u8*)mem_ptr) != 0)
		return RETUEN_ERROR;

	return RETURN_SUCCESS;
}

static int nor_init(void)
{
	int ret;
	static unsigned char nor_inition = 0;

	if (nor_inition)
		return RETURN_SUCCESS;
	nor_inition = 1;
	ret = sflash_init();
	return ret;
}

static int nor_read(u64 offset, u32 mem_ptr, u32 mem_length)
{
	sflash_read( (u32)offset, (u64)mem_ptr, mem_length);
	return RETURN_SUCCESS;
}

static int nor_write(u32 offset, u32 mem_ptr, u32 byte_counter)
{
	int ret;
	u32 write_cnt;
	u32 write_offset;
	u32 nor_xdelay = 4 * 1024;
	write_offset = 0;
	do {
		if (byte_counter > nor_xdelay)
			write_cnt = nor_xdelay;
		else
			write_cnt = byte_counter;
		ret = sflash_write((u32)(offset + write_offset), (uint64_t)(mem_ptr + write_offset), write_cnt);
		write_offset += write_cnt;
		byte_counter -= write_cnt;
	} while (byte_counter);
	return ret;
}
static int nor_bread(struct blkdev * bdev, u32 blknr, u32 blks, u8 * buf, u32 part_id)
{
	u32 ret;
	u32 offset = blknr * bdev->blksz;
	u32 byte_length = blks * bdev->blksz;

	if ((ret = nor_read((unsigned int )offset, (unsigned int)(uintptr_t)buf, byte_length)) != 0 )
		printk(BIOS_DEBUG, "partitial Read test fail !!! \n");

	return RETURN_SUCCESS;
}

static int nor_bwrite(struct blkdev * bdev, u32 blknr, u32 blks, u8 * buf, u32 part_id)
{
	u32 ret;
	u32 offset = blknr * bdev->blksz;
	u32 mem_ptr = (u32)(uintptr_t)buf;
	u32 byte_length = blks * bdev->blksz;

	if ((ret = nor_write(offset, mem_ptr, byte_length)) != 0) {
		printk(BIOS_DEBUG, "partitial write test fail !!! \n");
	}
	return RETURN_SUCCESS;
}

u32 nor_init_device(void)
{
	if (!blkdev_get(BOOTDEV_NOR)) {
		nor_init();
		write8(&mt8173_nor->sflash_read_dual_reg, 0);
		memset(&nor_block_dev, 0, sizeof(struct blkdev));
		nor_block_dev.blksz = 512;
		nor_block_dev.erasesz = 512;
		nor_block_dev.blks = array_flash_chip[0].chip_size;
		nor_block_dev.bread = nor_bread;
		nor_block_dev.bwrite = nor_bwrite;
		nor_block_dev.blkbuf = 0;
		nor_block_dev.type = BOOTDEV_NOR;
		blkdev_register(&nor_block_dev);
	}
	return RETURN_SUCCESS;
}
