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

#ifndef SOC_MEDIATEK_MT8173_PMIC_WRAP_INIT_H
#define SOC_MEDIATEK_MT8173_PMIC_WRAP_INIT_H

#include <arch/io.h>
#include <soc/addressmap.h>

#define PMIC_WRAP_DEBUG

#define PWRAPTAG                "[PWRAP] "
#ifdef PMIC_WRAP_DEBUG
#define PWRAPDEB(fmt, arg ...)   printk(BIOS_DEBUG, PWRAPTAG fmt, ## arg)
#define PWRAPFUC(fmt, arg ...)   printk(BIOS_DEBUG, PWRAPTAG "%s\n", \
					__func__)
#endif
#define PWRAPLOG(fmt, arg ...)   printk(BIOS_INFO, PWRAPTAG fmt, ## arg)
#define PWRAPERR(fmt, arg ...)   printk(BIOS_ERR, PWRAPTAG "ERROR,line=%d " fmt, \
					__LINE__, ## arg)
#define PWRAPREG(fmt, arg ...)   printk(BIOS_INFO, PWRAPTAG fmt, ## arg)

/* external API */
s32 pwrap_read(u32 adr, u32 *rdata);
s32 pwrap_write(u32 adr, u32 wdata);
s32 pwrap_wacs2(u32 write, u32 adr, u32 wdata, u32 *rdata);
s32 pwrap_init(void);
s32 pwrap_init_preloader(void);

/* PMIC_WRAP reset register */
enum {
	PERI_PWRAP_BRIDGE_BASE	= 0x11017000,
	INFRA_SYS_CFG_BASE	= 0x10001000,
	INFRACFG_BASE		= 0x10001000,
	INFRA_GLOBALCON_RST0	= INFRACFG_BASE + 0x030
};

/* watchdog define */
enum {
	TOPCKGEN_BASE             = 0x10000000,
	TOPRGU_BASE               = 0x10007000,
	WDT_SWSYSRST              = TOPRGU_BASE + 0x18,
	AP_RGU_BASE               = 0x10000000
};

enum {
	WACS2 = 1 << 4
};

enum {
	DEW_BASE = 0xBC00
};

/* PMIC registers */
enum {
	PMIC_BASE = 0x0000,
	PMIC_WRP_CKPDN       = PMIC_BASE + 0x011A,          // 0x0056
	PMIC_WRP_RST_CON     = PMIC_BASE + 0x0120,          // 0x005C
	PMIC_TOP_CKCON2      = PMIC_BASE + 0x012A,
	PMIC_TOP_CKCON3      = PMIC_BASE + 0x01D4
};

/* timeout setting */
enum {
	TIMEOUT_RESET       = 0xFF,    // us
	TIMEOUT_READ        = 0xFF,    // us
	TIMEOUT_WAIT_IDLE   = 0xFF    // us
};

/* PMIC_WRAP registers */
enum {
	PMIC_WRAP_MUX_SEL            = PMIC_WRAP_BASE + 0x0,
	PMIC_WRAP_WRAP_EN            = PMIC_WRAP_BASE + 0x4,
	PMIC_WRAP_DIO_EN             = PMIC_WRAP_BASE + 0x8,
	PMIC_WRAP_SIDLY              = PMIC_WRAP_BASE + 0xC,
	PMIC_WRAP_RDDMY              = PMIC_WRAP_BASE + 0x10,
	PMIC_WRAP_SI_CK_CON          = PMIC_WRAP_BASE + 0x14,
	PMIC_WRAP_CSHEXT_WRITE       = PMIC_WRAP_BASE + 0x18,
	PMIC_WRAP_CSHEXT_READ        = PMIC_WRAP_BASE + 0x1C,
	PMIC_WRAP_CSLEXT_START       = PMIC_WRAP_BASE + 0x20,
	PMIC_WRAP_CSLEXT_END         = PMIC_WRAP_BASE + 0x24,
	PMIC_WRAP_STAUPD_PRD         = PMIC_WRAP_BASE + 0x28,
	PMIC_WRAP_STAUPD_GRPEN       = PMIC_WRAP_BASE + 0x2C,
	PMIC_WRAP_STAUPD_MAN_TRIG    = PMIC_WRAP_BASE + 0x40,
	PMIC_WRAP_STAUPD_STA         = PMIC_WRAP_BASE + 0x44,
	PMIC_WRAP_WRAP_STA           = PMIC_WRAP_BASE + 0x48,
	PMIC_WRAP_HARB_INIT          = PMIC_WRAP_BASE + 0x4C,
	PMIC_WRAP_HARB_HPRIO         = PMIC_WRAP_BASE + 0x50,
	PMIC_WRAP_HIPRIO_ARB_EN      = PMIC_WRAP_BASE + 0x54,
	PMIC_WRAP_HARB_STA0          = PMIC_WRAP_BASE + 0x58,
	PMIC_WRAP_HARB_STA1          = PMIC_WRAP_BASE + 0x5C,
	PMIC_WRAP_MAN_EN             = PMIC_WRAP_BASE + 0x60,
	PMIC_WRAP_MAN_CMD            = PMIC_WRAP_BASE + 0x64,
	PMIC_WRAP_MAN_RDATA          = PMIC_WRAP_BASE + 0x68,
	PMIC_WRAP_MAN_VLDCLR         = PMIC_WRAP_BASE + 0x6C,
	PMIC_WRAP_WACS0_EN           = PMIC_WRAP_BASE + 0x70,
	PMIC_WRAP_INIT_DONE0         = PMIC_WRAP_BASE + 0x74,
	PMIC_WRAP_WACS0_CMD          = PMIC_WRAP_BASE + 0x78,
	PMIC_WRAP_WACS0_RDATA        = PMIC_WRAP_BASE + 0x7C,
	PMIC_WRAP_WACS0_VLDCLR       = PMIC_WRAP_BASE + 0x80,
	PMIC_WRAP_WACS1_EN           = PMIC_WRAP_BASE + 0x84,
	PMIC_WRAP_INIT_DONE1         = PMIC_WRAP_BASE + 0x88,
	PMIC_WRAP_WACS1_CMD          = PMIC_WRAP_BASE + 0x8C,
	PMIC_WRAP_WACS1_RDATA        = PMIC_WRAP_BASE + 0x90,
	PMIC_WRAP_WACS1_VLDCLR       = PMIC_WRAP_BASE + 0x94,
	PMIC_WRAP_WACS2_EN           = PMIC_WRAP_BASE + 0x98,
	PMIC_WRAP_INIT_DONE2         = PMIC_WRAP_BASE + 0x9C,
	PMIC_WRAP_WACS2_CMD          = PMIC_WRAP_BASE + 0xA0,
	PMIC_WRAP_WACS2_RDATA        = PMIC_WRAP_BASE + 0xA4,
	PMIC_WRAP_WACS2_VLDCLR       = PMIC_WRAP_BASE + 0xA8,
	PMIC_WRAP_INT_EN             = PMIC_WRAP_BASE + 0xAC,
	PMIC_WRAP_INT_FLG_RAW        = PMIC_WRAP_BASE + 0xB0,
	PMIC_WRAP_INT_FLG            = PMIC_WRAP_BASE + 0xB4,
	PMIC_WRAP_INT_CLR            = PMIC_WRAP_BASE + 0xB8,
	PMIC_WRAP_SIG_ADR            = PMIC_WRAP_BASE + 0xBC,
	PMIC_WRAP_SIG_MODE           = PMIC_WRAP_BASE + 0xC0,
	PMIC_WRAP_SIG_VALUE          = PMIC_WRAP_BASE + 0xC4,
	PMIC_WRAP_SIG_ERRVAL         = PMIC_WRAP_BASE + 0xC8,
	PMIC_WRAP_CRC_EN             = PMIC_WRAP_BASE + 0xCC,
	PMIC_WRAP_TIMER_EN           = PMIC_WRAP_BASE + 0xD0,
	PMIC_WRAP_TIMER_STA          = PMIC_WRAP_BASE + 0xD4,
	PMIC_WRAP_WDT_UNIT           = PMIC_WRAP_BASE + 0xD8,
	PMIC_WRAP_WDT_SRC_EN         = PMIC_WRAP_BASE + 0xDC,
	PMIC_WRAP_WDT_FLG            = PMIC_WRAP_BASE + 0xE0,
	PMIC_WRAP_DEBUG_INT_SEL      = PMIC_WRAP_BASE + 0xE4,
	PMIC_WRAP_DVFS_ADR0          = PMIC_WRAP_BASE + 0xE8,
	PMIC_WRAP_DVFS_WDATA0        = PMIC_WRAP_BASE + 0xEC,
	PMIC_WRAP_DVFS_ADR1          = PMIC_WRAP_BASE + 0xF0,
	PMIC_WRAP_DVFS_WDATA1        = PMIC_WRAP_BASE + 0xF4,
	PMIC_WRAP_DVFS_ADR2          = PMIC_WRAP_BASE + 0xF8,
	PMIC_WRAP_DVFS_WDATA2        = PMIC_WRAP_BASE + 0xFC,
	PMIC_WRAP_DVFS_ADR3          = PMIC_WRAP_BASE + 0x100,
	PMIC_WRAP_DVFS_WDATA3        = PMIC_WRAP_BASE + 0x104,
	PMIC_WRAP_DVFS_ADR4          = PMIC_WRAP_BASE + 0x108,
	PMIC_WRAP_DVFS_WDATA4        = PMIC_WRAP_BASE + 0x10C,
	PMIC_WRAP_DVFS_ADR5          = PMIC_WRAP_BASE + 0x110,
	PMIC_WRAP_DVFS_WDATA5        = PMIC_WRAP_BASE + 0x114,
	PMIC_WRAP_DVFS_ADR6          = PMIC_WRAP_BASE + 0x118,
	PMIC_WRAP_DVFS_WDATA6        = PMIC_WRAP_BASE + 0x11C,
	PMIC_WRAP_DVFS_ADR7          = PMIC_WRAP_BASE + 0x120,
	PMIC_WRAP_DVFS_WDATA7        = PMIC_WRAP_BASE + 0x124,
	PMIC_WRAP_SPMINF_STA         = PMIC_WRAP_BASE + 0x128,
	PMIC_WRAP_CIPHER_KEY_SEL     = PMIC_WRAP_BASE + 0x12C,
	PMIC_WRAP_CIPHER_IV_SEL      = PMIC_WRAP_BASE + 0x130,
	PMIC_WRAP_CIPHER_EN          = PMIC_WRAP_BASE + 0x134,
	PMIC_WRAP_CIPHER_RDY         = PMIC_WRAP_BASE + 0x138,
	PMIC_WRAP_CIPHER_MODE        = PMIC_WRAP_BASE + 0x13C,
	PMIC_WRAP_CIPHER_SWRST       = PMIC_WRAP_BASE + 0x140,
	PMIC_WRAP_DCM_EN             = PMIC_WRAP_BASE + 0x144,
	PMIC_WRAP_DCM_DBC_PRD        = PMIC_WRAP_BASE + 0x148
};

/* staupd sta fsm */
enum {
	STAUPD_FSM_IDLE         = 0x00,
	STAUPD_FSM_REQ          = 0x02,
	STAUPD_FSM_WFDLE        = 0x04       // wait for dle,wait for read data done,
};

/* MAN_RDATA  FSM */
enum {
	MAN_FSM_NO_REQ           = 0x00,
	MAN_FSM_IDLE             = 0x00,
	MAN_FSM_REQ              = 0x02,
	MAN_FSM_WFDLE            = 0x04,       // wait for dle,wait for read data done,
	MAN_FSM_WFVLDCLR         = 0x06
};

/* WACS_FSM */
enum {
	WACS_FSM_IDLE            = 0x00,
	WACS_FSM_REQ             = 0x02,
	WACS_FSM_WFDLE           = 0x04,       // wait for dle,wait for read data done,
	WACS_FSM_WFVLDCLR        = 0x06,       // finish read data , wait for valid flag clearing
	WACS_INIT_DONE           = 0x01,
	WACS_SYNC_IDLE           = 0x01,
	WACS_SYNC_BUSY           = 0x00
};

/* wrapper bridge regsister */
/* APB Module peri_pwrap_bridge */
enum {
	PERI_PWRAP_BRIDGE_IARB_INIT         = PERI_PWRAP_BRIDGE_BASE + 0x0,
	PERI_PWRAP_BRIDGE_IORD_ARB_EN       = PERI_PWRAP_BRIDGE_BASE + 0x4,
	PERI_PWRAP_BRIDGE_IARB_STA0         = PERI_PWRAP_BRIDGE_BASE + 0x8,
	PERI_PWRAP_BRIDGE_IARB_STA1         = PERI_PWRAP_BRIDGE_BASE + 0xC,
	PERI_PWRAP_BRIDGE_WACS3_EN          = PERI_PWRAP_BRIDGE_BASE + 0x10,
	PERI_PWRAP_BRIDGE_INIT_DONE3        = PERI_PWRAP_BRIDGE_BASE + 0x14,
	PERI_PWRAP_BRIDGE_WACS3_CMD         = PERI_PWRAP_BRIDGE_BASE + 0x18,
	PERI_PWRAP_BRIDGE_WACS3_RDATA       = PERI_PWRAP_BRIDGE_BASE + 0x1C,
	PERI_PWRAP_BRIDGE_WACS3_VLDCLR      = PERI_PWRAP_BRIDGE_BASE + 0x20,
	PERI_PWRAP_BRIDGE_WACS4_EN          = PERI_PWRAP_BRIDGE_BASE + 0x24,
	PERI_PWRAP_BRIDGE_INIT_DONE4        = PERI_PWRAP_BRIDGE_BASE + 0x28,
	PERI_PWRAP_BRIDGE_WACS4_CMD         = PERI_PWRAP_BRIDGE_BASE + 0x2C,
	PERI_PWRAP_BRIDGE_WACS4_RDATA       = PERI_PWRAP_BRIDGE_BASE + 0x30,
	PERI_PWRAP_BRIDGE_WACS4_VLDCLR      = PERI_PWRAP_BRIDGE_BASE + 0x34,
	PERI_PWRAP_BRIDGE_INT_EN            = PERI_PWRAP_BRIDGE_BASE + 0x38,
	PERI_PWRAP_BRIDGE_INT_FLG_RAW       = PERI_PWRAP_BRIDGE_BASE + 0x3C,
	PERI_PWRAP_BRIDGE_INT_FLG           = PERI_PWRAP_BRIDGE_BASE + 0x40,
	PERI_PWRAP_BRIDGE_INT_CLR           = PERI_PWRAP_BRIDGE_BASE + 0x44,
	PERI_PWRAP_BRIDGE_TIMER_EN          = PERI_PWRAP_BRIDGE_BASE + 0x48,
	PERI_PWRAP_BRIDGE_TIMER_STA         = PERI_PWRAP_BRIDGE_BASE + 0x4C,
	PERI_PWRAP_BRIDGE_WDT_UNIT          = PERI_PWRAP_BRIDGE_BASE + 0x50,
	PERI_PWRAP_BRIDGE_WDT_SRC_EN        = PERI_PWRAP_BRIDGE_BASE + 0x54,
	PERI_PWRAP_BRIDGE_WDT_FLG           = PERI_PWRAP_BRIDGE_BASE + 0x58,
	PERI_PWRAP_BRIDGE_DEBUG_INT_SEL     = PERI_PWRAP_BRIDGE_BASE + 0x5C
};

/* wrapper bridge regsister */
#define GET_WACS3_RDATA(x)         ((x >> 0) & 0x0000ffff)
#define GET_WACS3_FSM(x)           ((x >> 16) & 0x00000007)
#define GET_WACS3_REQ(x)           ((x >> 19) & 0x00000001)
#define GET_SYNC_IDLE3(x)          ((x >> 20) & 0x00000001)
#define GET_INIT_DONE3(x)          ((x >> 21) & 0x00000001)
#define GET_WACS4_RDATA(x)         ((x >> 0) & 0x0000ffff)
#define GET_WACS4_FSM(x)           ((x >> 16) & 0x00000007)
#define GET_WACS4_REQ(x)           ((x >> 19) & 0x00000001)
#define GET_SYNC_IDLE4(x)          ((x >> 20) & 0x00000001)
#define GET_INIT_DONE4(x)          ((x >> 21) & 0x00000001)

/* dewrapper regsister */
enum {
	DEW_EVENT_OUT_EN   = DEW_BASE + 0x0,
	DEW_DIO_EN         = DEW_BASE + 0x2,
	DEW_EVENT_SRC_EN   = DEW_BASE + 0x4,
	DEW_EVENT_SRC      = DEW_BASE + 0x6,
	DEW_EVENT_FLAG     = DEW_BASE + 0x8,
	DEW_READ_TEST      = DEW_BASE + 0xA,
	DEW_WRITE_TEST     = DEW_BASE + 0xC,
	DEW_CRC_EN         = DEW_BASE + 0xE,
	DEW_CRC_VAL        = DEW_BASE + 0x10,
	DEW_MON_GRP_SEL    = DEW_BASE + 0x12,
	DEW_MON_FLAG_SEL   = DEW_BASE + 0x14,
	DEW_EVENT_TEST     = DEW_BASE + 0x16,
	DEW_CIPHER_KEY_SEL = DEW_BASE + 0x18,
	DEW_CIPHER_IV_SEL  = DEW_BASE + 0x1A,
	DEW_CIPHER_LOAD    = DEW_BASE + 0x1C,
	DEW_CIPHER_START   = DEW_BASE + 0x1E,
	DEW_CIPHER_RDY     = DEW_BASE + 0x20,
	DEW_CIPHER_MODE    = DEW_BASE + 0x22,
	DEW_CIPHER_SWRST   = DEW_BASE + 0x24,
	DEW_CIPHER_IV0     = DEW_BASE + 0x26,
	DEW_CIPHER_IV1     = DEW_BASE + 0x28,
	DEW_CIPHER_IV2     = DEW_BASE + 0x2A,
	DEW_CIPHER_IV3     = DEW_BASE + 0x2C,
	DEW_CIPHER_IV4     = DEW_BASE + 0x2E,
	DEW_CIPHER_IV5     = DEW_BASE + 0x30
};

/* dewrapper defaule value */
enum {
	DEFAULT_VALUE_READ_TEST  = 0x5aa5,
	WRITE_TEST_VALUE         = 0xa55a
};

/* manual commnd */
enum {
	OP_WR    = 0x1,
	OP_CSH   = 0x0,
	OP_CSL   = 0x1,
	OP_OUTS  = 0x8,
	OP_OUTD  = 0x9,
	OP_INS   = 0xC,
	OP_IND   = 0xD
};

/* macro for read/write register */
#define WRAP_RD32(addr)             read32((void *)(uintptr_t)(addr))
#define WRAP_WR32(addr, val)        write32((void *)(uintptr_t)(addr), (val))

#define WRAP_SET_BIT(BS, REG)       setbits_le32((void *)(uintptr_t)(REG), BS)
#define WRAP_CLR_BIT(BS, REG)       clrbits_le32((void *)(uintptr_t)(REG), BS)

#define PWRAP_SOFT_RESET                WRAP_SET_BIT(1 << 7, INFRA_GLOBALCON_RST0)
#define PWRAP_CLEAR_SOFT_RESET_BIT      WRAP_CLR_BIT(1 << 7, INFRA_GLOBALCON_RST0)
#define PERI_GLOBALCON_RST1             (PERI_CON_BASE + 0x004)

/* error information flag */
enum {
	E_PWR_INVALID_ARG             = 1,
	E_PWR_INVALID_RW              = 2,
	E_PWR_INVALID_ADDR            = 3,
	E_PWR_INVALID_WDAT            = 4,
	E_PWR_INVALID_OP_MANUAL       = 5,
	E_PWR_NOT_IDLE_STATE          = 6,
	E_PWR_NOT_INIT_DONE           = 7,
	E_PWR_NOT_INIT_DONE_READ      = 8,
	E_PWR_WAIT_IDLE_TIMEOUT       = 9,
	E_PWR_WAIT_IDLE_TIMEOUT_READ  = 10,
	E_PWR_INIT_SIDLY_FAIL         = 11,
	E_PWR_RESET_TIMEOUT           = 12,
	E_PWR_TIMEOUT                 = 13,
	E_PWR_INIT_RESET_SPI          = 20,
	E_PWR_INIT_SIDLY              = 21,
	E_PWR_INIT_REG_CLOCK          = 22,
	E_PWR_INIT_ENABLE_PMIC        = 23,
	E_PWR_INIT_DIO                = 24,
	E_PWR_INIT_CIPHER             = 25,
	E_PWR_INIT_WRITE_TEST         = 26,
	E_PWR_INIT_ENABLE_CRC         = 27,
	E_PWR_INIT_ENABLE_DEWRAP      = 28,
	E_PWR_INIT_ENABLE_EVENT       = 29,
	E_PWR_READ_TEST_FAIL          = 30,
	E_PWR_WRITE_TEST_FAIL         = 31,
	E_PWR_SWITCH_DIO              = 32
};

#endif /* SOC_MEDIATEK_MT8173_PMIC_WRAP_INIT_H */
