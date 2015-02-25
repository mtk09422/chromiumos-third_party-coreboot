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

#include <arch/io.h>
#include <console/console.h>
#include <delay.h>
#include <edid.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <soc/addressmap.h>
#include <soc/clock.h>

#include <soc/ddp.h>


#define ddp_write32(data, addr)	\
	write32((void *)((uintptr_t)(addr)), data)
#define ddp_read32(addr) \
	read32((void *)((uintptr_t)(addr)))

#ifdef DEBUGDPF
#define MTKFBTAG                "[MTKFB] "
#define MTKFBERRTAG             "[MTKFB ERR] "
#define MTKFBDPF(fmt, arg...)   printk(BIOS_INFO, MTKFBTAG fmt, ##arg)
#define MTKFBERR(fmt, arg...)   printk(BIOS_INFO, MTKFBERRTAG fmt, ##arg)
#else
#define MTKFBERRTAG             "[MTKFB ERR] "
#define MTKFBDPF(fmt, arg...)
#define MTKFBERR(fmt, arg...)   printk(BIOS_INFO, MTKFBERRTAG fmt, ##arg)
#endif


static struct MTK_DISP_REGS _mtk_disp_regs = {
	.regs       = DISPSYS_CONFIG_BASE,
	.ovl_regs   = DISPSYS_OVL0_BASE,
	.rdma_regs  = DISPSYS_RDMA0_BASE,
	.color_regs = DISPSYS_COLOR0_BASE,
	.aal_regs   = DISPSYS_AAL_BASE,
	.ufoe_regs  = DISPSYS_UFOE_BASE,
	.mutex_regs = DISPSYS_MUTEX_BASE,
	.dsi_reg    = DISPSYS_DSI0_BASE,
	.od_regs    = DISPSYS_OD_BASE,
	.dsi_ana_reg = DSI_ANA_BASE,
};

/* These are locked by dev->vbl_lock */
void mtk_enable_vblank(u32 disp_base)
{
	ddp_write32(0x1, disp_base + DISP_OD_INTEN);
}

void mtk_disable_vblank(u32 disp_base)
{
	ddp_write32(0x0, disp_base + DISP_OD_INTEN);
}

void mtk_clear_vblank(u32 disp_base)
{
	ddp_write32(0x0, disp_base + DISP_OD_INTS);
}

u32 mtk_get_vblank(u32 disp_base)
{
	return ddp_read32(disp_base + DISP_OD_INTS);
}

static void disp_config_main_path_connection(u32 disp_base)
{
	/* OVL0 output to COLOR0 */
	ddp_write32(0x1, disp_base + DISP_REG_CONFIG_DISP_OVL0_MOUT_EN);

	/* OD output to RDMA0 */
	ddp_write32(0x1, disp_base + DISP_REG_CONFIG_DISP_OD_MOUT_EN);

	/* UFOE output to DSI0 */
	ddp_write32(0x1, disp_base + DISP_REG_CONFIG_DISP_UFOE_MOUT_EN);

	/* COLOR0 input from OVL0 */
	ddp_write32(0x1, disp_base + DISP_REG_CONFIG_DISP_COLOR0_SEL_IN);
}

static void disp_config_main_path_mutex(u32 mutex_base)
{
	unsigned int ID = 0, val;

	/* Module: OVL0=11, RDMA0=13, COLOR0=18, AAL=20, UFOE=22, OD=25 */
	val = (1 << 11 | 1 << 13 | 1 << 18 | 1 << 20 | 1 << 22 | 1 << 25);
	ddp_write32(val, mutex_base + DISP_REG_CONFIG_MUTEX_MOD(ID));

	/* Clock source from DSI0 */
	ddp_write32(1, mutex_base + DISP_REG_CONFIG_MUTEX_SOF(ID));
	ddp_write32(1, mutex_base + DISP_REG_CONFIG_MUTEX_EN(ID));
}

/* work on OVL0 only now */
static void ovl_start(u32 ovl_base)
{
	ddp_write32(0x01, ovl_base + DISP_REG_OVL_EN);
	ddp_write32(0x0f, ovl_base + DISP_REG_OVL_INTEN);
}

static void ovl_stop(u32 ovl_base)
{
	ddp_write32(0x00, ovl_base + DISP_REG_OVL_INTEN);
	ddp_write32(0x00, ovl_base + DISP_REG_OVL_EN);
	ddp_write32(0x00, ovl_base + DISP_REG_OVL_INTSTA);
}

static void ovl_set_roi(u32 ovl_base, unsigned int w, unsigned int h,
	unsigned int color)
{
	ddp_write32(h << 16 | w, ovl_base + DISP_REG_OVL_ROI_SIZE);
	ddp_write32(color, ovl_base + DISP_REG_OVL_ROI_BGCLR);
}

static void ovl_layer_switch(u32 ovl_base, unsigned layer, int en)
{
	u32 reg;

	reg = ddp_read32(ovl_base + DISP_REG_OVL_SRC_CON);

	if (en)
		reg |= (1U<<layer);
	else
		reg &= ~(1U<<layer);

	ddp_write32(reg, ovl_base + DISP_REG_OVL_SRC_CON);
	ddp_write32(0x1, ovl_base + DISP_REG_OVL_RST);
	ddp_write32(0x0, ovl_base + DISP_REG_OVL_RST);
}

static void rdma_start(u32 rdma_base, unsigned idx)
{
	unsigned int reg;
	unsigned int offset = idx * RDMA_N_OFST;

	if (idx >= 2)
		return;

	ddp_write32(0x4, rdma_base + DISP_REG_RDMA_INT_ENABLE + offset);
	reg = ddp_read32(rdma_base + DISP_REG_RDMA_GLOBAL_CON + offset);
	reg |= 1;
	ddp_write32(reg, rdma_base + DISP_REG_RDMA_GLOBAL_CON + offset);
}

static void rdma_stop(u32 rdma_base, unsigned idx)
{
	unsigned int reg;
	unsigned int offset = idx * RDMA_N_OFST;

	if (idx >= 2)
		return;

	reg = ddp_read32(rdma_base + DISP_REG_RDMA_GLOBAL_CON + offset);
	reg &= ~(1U);
	ddp_write32(reg, rdma_base + DISP_REG_RDMA_GLOBAL_CON + offset);
	ddp_write32(0, rdma_base + DISP_REG_RDMA_INT_ENABLE + offset);
	ddp_write32(0, rdma_base + DISP_REG_RDMA_INT_STATUS + offset);
}

static void rdma_reset(u32 rdma_base, unsigned idx)
{
	unsigned int reg;
	unsigned int delay_cnt;
	unsigned int offset = idx * RDMA_N_OFST;

	if (idx >= 2)
		return;

	reg = ddp_read32(rdma_base + DISP_REG_RDMA_GLOBAL_CON + offset);
	reg |= 0x10;
	ddp_write32(reg, rdma_base + DISP_REG_RDMA_GLOBAL_CON + offset);

	delay_cnt = 0;
	while ((ddp_read32(rdma_base + DISP_REG_RDMA_GLOBAL_CON + offset)
		& 0x700) == 0x100) {

		delay_cnt++;
		if (delay_cnt > 10000) {
			MTKFBERR("rdma_reset timeout\n");
			break;
		}
	}

	reg = ddp_read32(rdma_base + DISP_REG_RDMA_GLOBAL_CON + offset);
	reg &= ~(0x10U);
	ddp_write32(reg, rdma_base + DISP_REG_RDMA_GLOBAL_CON + offset);

	delay_cnt = 0;
	while ((ddp_read32(rdma_base + DISP_REG_RDMA_GLOBAL_CON + offset)
		& 0x700) != 0x100) {

		delay_cnt++;
		if (delay_cnt > 10000) {
			MTKFBERR("rdma_reset timeout\n");
			break;
		}
	}
}

static void rdma_config_direct_link(u32 rdma_base, unsigned idx,
	unsigned width, unsigned height)
{
	unsigned int reg;
	enum RDMA_MODE mode = RDMA_MODE_DIRECT_LINK;
	enum RDMA_OUTPUT_FORMAT output_format = RDMA_OUTPUT_FORMAT_ARGB;
	unsigned int offset = idx * RDMA_N_OFST;

	if (idx >= 2)
		return;

	/* Config mode */
	reg = ddp_read32(rdma_base + DISP_REG_RDMA_GLOBAL_CON + offset);
	if (mode == RDMA_MODE_DIRECT_LINK)
		reg &= ~(0x2U);
	ddp_write32(reg, rdma_base + DISP_REG_RDMA_GLOBAL_CON + offset);

	/* Config output format */
	reg = ddp_read32(rdma_base + DISP_REG_RDMA_SIZE_CON_0 + offset);
	if (output_format == RDMA_OUTPUT_FORMAT_ARGB)
		reg &= ~(0x20000000U);
	else
		reg |= 0x20000000U;
	ddp_write32(reg, rdma_base + DISP_REG_RDMA_SIZE_CON_0 + offset);

	/* Config width */
	reg = ddp_read32(rdma_base + DISP_REG_RDMA_SIZE_CON_0 + offset);
	reg = (reg & ~(0xFFFU)) | (width & 0xFFFU);
	ddp_write32(reg, rdma_base + DISP_REG_RDMA_SIZE_CON_0 + offset);

	/* Config height */
	reg = ddp_read32(rdma_base + DISP_REG_RDMA_SIZE_CON_1 + offset);
	reg = (reg & ~(0xFFFFFU)) | (height & 0xFFFFFU);
	ddp_write32(reg, rdma_base + DISP_REG_RDMA_SIZE_CON_1 + offset);

	/* Config FIFO control */
	reg = 0x80F00008;
	ddp_write32(reg, rdma_base + DISP_REG_RDMA_FIFO_CON + offset);
}

static void od_start(u32 od_base, unsigned int W, unsigned int H)
{
	ddp_write32(W << 16 | H, od_base + DISP_OD_SIZE);
	ddp_write32(1, od_base + DISP_OD_CFG); /* RELAY mode */

	ddp_write32(1, od_base + DISP_OD_EN);
}

static void ufoe_start(u32 ufoe_base)
{
	ddp_write32(0x4, ufoe_base + DISPSYS_UFOE_BASE);
}

static void color_start(u32 color_base)
{
	ddp_write32(0x2080, color_base + DISP_COLOR_CFG_MAIN);
	ddp_write32(0x1, color_base + DISP_COLOR_START);
}

static void ovl_layer_config(unsigned int fmt, unsigned int addr,
			   unsigned int width, unsigned int height)
{
	u32 drm_disp_base = _mtk_disp_regs.ovl_regs;
	unsigned int reg;
	unsigned int source = 0;	/* from memory */
	unsigned int src_x = 0;	/* ROI x offset */
	unsigned int src_y = 0;	/* ROI y offset */
	unsigned int src_pitch;
	unsigned int dst_x = 0;	/* ROI x offset */
	unsigned int dst_y = 0;	/* ROI y offset */
	unsigned int dst_w = width;	/* ROT width */
	unsigned int dst_h = height;	/* ROI height */
	int color_key_enable = 1;
	unsigned int key = 0xFF000000;	/* color key */
	int aen = 0;			/* alpha enable */
	unsigned char alpha = 0x0;
	unsigned int rgb_swap, bpp;

	if (fmt == OVL_INFMT_BGR888 || fmt == OVL_INFMT_BGR565 ||
		fmt == OVL_INFMT_ABGR8888 || fmt == OVL_INFMT_BGRA8888) {
		fmt -= OVL_COLOR_BASE;
		rgb_swap = 1;
	} else {
		rgb_swap = 0;
	}

	switch (fmt) {
	case OVL_INFMT_ARGB8888:
	case OVL_INFMT_RGBA8888:
		bpp = 4;
		break;
	case OVL_INFMT_RGB888:
		bpp = 3;
		break;
	case OVL_INFMT_RGB565:
	case OVL_INFMT_YUYV:
	case OVL_INFMT_UYVY:
		bpp = 2;
		break;
	default:
		bpp = 1;	/* invalid input format */
	}

	src_pitch = width * bpp;
	ovl_layer_switch(drm_disp_base, 0, 1);

	ddp_write32(0x1, drm_disp_base + DISP_REG_OVL_RST);
	ddp_write32(0x0, drm_disp_base + DISP_REG_OVL_RST);

	ddp_write32(0x3, drm_disp_base + DISP_REG_OVL_SRC_CON);

	ddp_write32(0x00000001, drm_disp_base + DISP_REG_OVL_RDMA0_CTRL);
	ddp_write32(0x40402020,
		    drm_disp_base + DISP_REG_OVL_RDMA0_MEM_GMC_SETTING);

	reg = color_key_enable << 30 | source << 28 | rgb_swap << 25 |
	      fmt << 12 | aen << 8 | alpha;
	ddp_write32(reg, drm_disp_base + DISP_REG_OVL_L0_CON);
	ddp_write32(key, drm_disp_base + DISP_REG_OVL_L0_SRCKEY);
	ddp_write32(dst_h << 16 | dst_w,
		    drm_disp_base + DISP_REG_OVL_L0_SRC_SIZE);
	ddp_write32(dst_y << 16 | dst_x,
		    drm_disp_base + DISP_REG_OVL_L0_OFFSET);
	reg = addr + src_x * bpp + src_y * src_pitch;
	ddp_write32(reg, drm_disp_base + DISP_REG_OVL_L0_ADDR);
	ddp_write32(src_pitch & 0xFFFF,
		    drm_disp_base + DISP_REG_OVL_L0_PITCH);
}

void ovl_dump_register(u32 disp_base)
{
	int i;

	MTKFBDPF("OVL Register:\n");
	for (i = 0; i < 0x300; i += 16)
		MTKFBDPF("%#08X: %08X %08X %08X %08X\n", i,
			ddp_read32(disp_base + DISPSYS_OVL0_BASE + i),
			ddp_read32(disp_base + DISPSYS_OVL0_BASE + i + 4),
			ddp_read32(disp_base + DISPSYS_OVL0_BASE + i + 8),
			ddp_read32(disp_base + DISPSYS_OVL0_BASE + i + 12));
}

void rdma_dump_register(u32 disp_base)
{
	int i;

	MTKFBDPF("RDMA Register:\n");
	for (i = 0; i < 0x100; i += 16)
		MTKFBDPF("%#08X: %08X %08X %08X %08X\n", i,
			ddp_read32(disp_base + DISPSYS_RDMA0_BASE + i),
			ddp_read32(disp_base + DISPSYS_RDMA0_BASE + i + 4),
			ddp_read32(disp_base + DISPSYS_RDMA0_BASE + i + 8),
			ddp_read32(disp_base + DISPSYS_RDMA0_BASE + i + 12));
}

void disp_config_dump_register(u32 disp_base)
{
	int i;

	MTKFBDPF("Disp Config Register:\n");
	for (i = 0; i < 0x180; i += 16)
		MTKFBDPF("%#08X: %08X %08X %08X %08X\n", i,
			ddp_read32(disp_base + DISPSYS_CONFIG_BASE + i),
			ddp_read32(disp_base + DISPSYS_CONFIG_BASE + i + 4),
			ddp_read32(disp_base + DISPSYS_CONFIG_BASE + i + 8),
			ddp_read32(disp_base + DISPSYS_CONFIG_BASE + i + 12));
}

void main_disp_path_power_on(void)
{
	ovl_start(_mtk_disp_regs.ovl_regs);
	rdma_start(_mtk_disp_regs.rdma_regs, 0);
}

void main_disp_path_power_off(void)
{
	ovl_stop(_mtk_disp_regs.ovl_regs);
	rdma_stop(_mtk_disp_regs.rdma_regs, 0);
	rdma_reset(_mtk_disp_regs.rdma_regs, 0);

	disp_config_dump_register(_mtk_disp_regs.regs);
}

static void main_disp_path_setup(const struct edid *edid)
{

	unsigned int width, height;

	width = edid->ha;
	height = edid->va;

	MTKFBDPF("main_disp_path_setup %d %d\n", width, height);
	/* Setup OVL */
	ovl_set_roi(_mtk_disp_regs.ovl_regs, width, height, 0x00000000);
	ovl_layer_switch(_mtk_disp_regs.ovl_regs, 0, 1);

	/* Setup RDMA0 */
	rdma_config_direct_link(_mtk_disp_regs.rdma_regs, 0, width, height);

	/* Setup OD */
	od_start(_mtk_disp_regs.od_regs, width, height);

	/* Setup UFOE */
	ufoe_start(_mtk_disp_regs.ufoe_regs);

	/* Setup Color */
	color_start(_mtk_disp_regs.color_regs);

	/* Setup main path connection */
	disp_config_main_path_connection(_mtk_disp_regs.regs);

	/* Setup main path mutex */
	disp_config_main_path_mutex(_mtk_disp_regs.mutex_regs);
}

void main_disp_path_clear(void)
{
	int i;

	MTKFBDPF("main_disp_path_clear..all\n");
	for (i = 0; i < 0x100; i += 4)
		ddp_write32(0, _mtk_disp_regs.regs + i);

	for (i = 0; i < 0x1000; i += 4) /* OVL */
		ddp_write32(0, _mtk_disp_regs.ovl_regs + i);

	for (i = 0; i < 0x1000; i += 4) /* RDMA */
		ddp_write32(0, _mtk_disp_regs.rdma_regs + i);

	for (i = 0x400; i < 0x1000; i += 4) /* COLOR */
		ddp_write32(0, _mtk_disp_regs.color_regs + i);

	for (i = 0; i < 0x1000; i += 4) /* AAL */
		ddp_write32(0, _mtk_disp_regs.aal_regs + i);

	for (i = 0; i < 0x1000; i += 4) /* UFOE */
		ddp_write32(0, _mtk_disp_regs.ufoe_regs + i);

	for (i = 0; i < 0x1000; i += 4) /* DSI */
		ddp_write32(0, _mtk_disp_regs.dsi_reg + i);

	for (i = 0; i < 0x1000; i += 4) /* MUTEX */
		ddp_write32(0, _mtk_disp_regs.mutex_regs + i);

	for (i = 0; i < 0x1000; i += 4) /* OD */
		ddp_write32(0, _mtk_disp_regs.od_regs + i);
}

static void disp_clock_on(void)
{
	u32 addr;
	u32 cg_con;

	MTKFBDPF("disp_clock_on : force enable all\n");
	addr = _mtk_disp_regs.regs + DISP_REG_CONFIG_MMSYS_CG_CON0;
	cg_con = ddp_read32(addr);
	cg_con = cg_con & (~CG_CON0_SMI_COMMON) & (~CG_CON0_SMI_LARB0) &
		  (~CG_CON0_MUTEX_32K) & (~CG_CON0_DISP_OVL0) &
		  (~CG_CON0_DISP_RDMA0) & (~CG_CON0_DISP_COLOR0) &
		  (~CG_CON0_DISP_OD);

	ddp_write32(cg_con, addr);

	addr = _mtk_disp_regs.regs + DISP_REG_CONFIG_MMSYS_CG_CON1;
	cg_con = ddp_read32(addr);
	cg_con = cg_con & (~CG_CON1_DSI0_ENGINE) & (~CG_CON1_DSI0_DIGITAL);

	ddp_write32(cg_con, addr);
}

void disp_clock_clear(u32 drm_disp_base)
{
	unsigned int reg;

	/* ovl_disp_ck */
	reg = ddp_read32(drm_disp_base + DISP_REG_CONFIG_MMSYS_CG_CON0);

	ddp_write32(reg, drm_disp_base + DISP_REG_CONFIG_MMSYS_CG_CON0);
}

void mtk_ddp_init(void)
{
	MTKFBDPF("mtk_ddp_init\n");

	mtk_enable_vblank(_mtk_disp_regs.od_regs);
	disp_clock_on();
}

void mtk_ddp_mode_set(u32 fbbase, const struct edid *edid)
{
	unsigned int fmt;

	MTKFBDPF("mtk_ddp_mode_set fb %dX%dX%d @%08x\n",
		 edid->ha, edid->va, edid->framebuffer_bits_per_pixel, fbbase);

	main_disp_path_setup(edid);

	if (fbbase != 0)
		ovl_start(_mtk_disp_regs.ovl_regs);

	rdma_start(_mtk_disp_regs.rdma_regs, 0);

	switch (edid->framebuffer_bits_per_pixel) {
	case 16:
		fmt = OVL_INFMT_RGB565;
		break;
	case 24:
		fmt = OVL_INFMT_RGB888;
		break;
	case 32:
	default:
		fmt = OVL_INFMT_RGBA8888;
		break;
	}

	ovl_layer_config(fmt, fbbase, edid->ha, edid->va);
}
