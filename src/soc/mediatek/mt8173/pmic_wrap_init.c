/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 MediaTek Inc.
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

#include <console/console.h>
#include <timer.h>
#include <soc/pmic_wrap_init.h>
#include <soc/pll.h>
#include <types.h>

/* internal API */
static s32 _pwrap_wacs2_nochk(u32 write, u32 adr, u32 wdata, u32 *rdata);
static s32 _pwrap_reset_spislv(void);
static s32 _pwrap_init_dio(u32 dio_en);
static s32 _pwrap_init_cipher(void);
static s32 _pwrap_init_sidly(void);
static s32 _pwrap_init_reg_clock(u32 regck_sel);
static s32 pwrap_read_nochk(u32 adr, u32 *rdata);
static s32 pwrap_write_nochk(u32 adr, u32 wdata);

void pwrap_dump_ap_register(void);

/* define macro and inline function (for do while loop) */

typedef u32 (*loop_condition_fp)(u32);

static inline uint32_t get_staupd_dle_cnt(uint32_t x)
{
	return ((x >> 0) & 0x00000007);
}

static inline uint32_t get_staupd_ale_cnt(uint32_t x)
{
	return ((x >> 3) & 0x00000007);
}

static inline uint32_t get_staupd_fsm(uint32_t x)
{
	return ((x >> 6) & 0x00000007);
}

static inline uint32_t get_wrap_ch_dle_restcnt(uint32_t x)
{
	return ((x >> 0) & 0x00000007);
}

static inline uint32_t get_wrap_ch_ale_restcnt(uint32_t x)
{
	return ((x >> 3) & 0x00000003);
}

static inline uint32_t get_wrap_ag_dle_restcnt(uint32_t x)
{
	return ((x >> 5) & 0x00000003);
}

static inline uint32_t get_wrap_ch_w(uint32_t x)
{
	return ((x >> 7) & 0x00000001);
}

static inline uint32_t get_wrap_ch_req(uint32_t x)
{
	return ((x >> 8) & 0x00000001);
}

static inline uint32_t get_ag_wrap_w(uint32_t x)
{
	return ((x >> 9) & 0x00000001);
}

static inline uint32_t get_ag_wrap_req(uint32_t x)
{
	return ((x >> 10) & 0x00000001);
}

static inline uint32_t get_wrap_fsm(uint32_t x)
{
	return ((x >> 11) & 0x0000000f);
}

static inline uint32_t get_harb_wrap_wdata(uint32_t x)
{
	return ((x >> 0) & 0x0000ffff);
}

static inline uint32_t get_harb_wrap_adr(uint32_t x)
{
	return ((x >> 16) & 0x00007fff);
}

static inline uint32_t get_harb_wrap_req(uint32_t x)
{
	return ((x >> 31) & 0x00000001);
}

static inline uint32_t get_harb_dle_empty(uint32_t x)
{
	return ((x >> 0) & 0x00000001);
}

static inline uint32_t get_harb_dle_full(uint32_t x)
{
	return ((x >> 1) & 0x00000001);
}

static inline uint32_t get_harb_vld(uint32_t x)
{
	return ((x >> 2) & 0x00000001);
}

static inline uint32_t get_harb_dle_own(uint32_t x)
{
	return ((x >> 3) & 0x00000007);
}

static inline uint32_t get_harb_own(uint32_t x)
{
	return ((x >> 6) & 0x00000007);
}

static inline uint32_t get_harb_dle_restcnt(uint32_t x)
{
	return ((x >> 9) & 0x00000007);
}

static inline uint32_t get_ag_harb_req(uint32_t x)
{
	return ((x >> 12) & 0x0000003f);
}

static inline uint32_t get_harb_wrap_w(uint32_t x)
{
	return ((x >> 18) & 0x00000001);
}

static inline uint32_t get_harb_wrap_req0(uint32_t x)
{
	return ((x >> 19) & 0x00000001);
}

static inline uint32_t get_spi_wdata(uint32_t x)
{
	return ((x >> 0) & 0x000000ff);
}

static inline uint32_t get_spi_op(uint32_t x)
{
	return ((x >> 8) & 0x0000001f);
}

static inline uint32_t get_spi_w(uint32_t x)
{
	return ((x >> 13) & 0x00000001);
}

static inline uint32_t get_man_rdata(uint32_t x)
{
	return ((x >> 0) & 0x000000ff);
}

static inline uint32_t get_man_fsm(uint32_t x)
{
	return ((x >> 8) & 0x00000007);
}

static inline uint32_t get_man_req(uint32_t x)
{
	return ((x >> 11) & 0x00000001);
}

static inline uint32_t get_wacs0_wdata(uint32_t x)
{
	return ((x >> 0) & 0x0000ffff);
}

static inline uint32_t get_wacs0_adr(uint32_t x)
{
	return ((x >> 16) & 0x00007fff);
}

static inline uint32_t get_wacs0_write(uint32_t x)
{
	return ((x >> 31) & 0x00000001);
}

static inline uint32_t get_wacs0_rdata(uint32_t x)
{
	return ((x >> 0) & 0x0000ffff);
}

static inline uint32_t get_wacs0_fsm(uint32_t x)
{
	return ((x >> 16) & 0x00000007);
}

static inline uint32_t get_wacs0_req(uint32_t x)
{
	return ((x >> 19) & 0x00000001);
}

static inline uint32_t get_sync_idle0(uint32_t x)
{
	return ((x >> 20) & 0x00000001);
}

static inline uint32_t get_init_done0(uint32_t x)
{
	return ((x >> 21) & 0x00000001);
}

static inline uint32_t get_sys_idle0(uint32_t x)
{
	return ((x >> 22) & 0x00000001);
}

static inline uint32_t get_wacs0_fifo_fillcnt(uint32_t x)
{
	return ((x >> 24) & 0x0000000f);
}

static inline uint32_t get_wacs0_fifo_freecnt(uint32_t x)
{
	return ((x >> 28) & 0x0000000f);
}

static inline uint32_t get_wacs1_wdata(uint32_t x)
{
	return ((x >> 0) & 0x0000ffff);
}

static inline uint32_t get_wacs1_adr(uint32_t x)
{
	return ((x >> 16) & 0x00007fff);
}

static inline uint32_t get_wacs1_write(uint32_t x)
{
	return ((x >> 31) & 0x00000001);
}

static inline uint32_t get_wacs1_rdata(uint32_t x)
{
	return ((x >> 0) & 0x0000ffff);
}

static inline uint32_t get_wacs1_fsm(uint32_t x)
{
	return ((x >> 16) & 0x00000007);
}

static inline uint32_t get_wacs1_req(uint32_t x)
{
	return ((x >> 19) & 0x00000001);
}

static inline uint32_t get_sync_idle1(uint32_t x)
{
	return ((x >> 20) & 0x00000001);
}

static inline uint32_t get_init_done1(uint32_t x)
{
	return ((x >> 21) & 0x00000001);
}

static inline uint32_t get_sys_idle1(uint32_t x)
{
	return ((x >> 22) & 0x00000001);
}

static inline uint32_t get_wacs1_fifo_fillcnt(uint32_t x)
{
	return ((x >> 24) & 0x0000000f);
}

static inline uint32_t get_wacs1_fifo_freecnt(uint32_t x)
{
	return ((x >> 28) & 0x0000000f);
}

static inline uint32_t get_wacs2_wdata(uint32_t x)
{
	return ((x >> 0) & 0x0000ffff);
}

static inline uint32_t get_wacs2_adr(uint32_t x)
{
	return ((x >> 16) & 0x00007fff);
}

static inline uint32_t get_wacs2_write(uint32_t x)
{
	return ((x >> 31) & 0x00000001);
}

static inline uint32_t get_wacs2_rdata(uint32_t x)
{
	return ((x >> 0) & 0x0000ffff);
}

static inline uint32_t get_wacs2_fsm(uint32_t x)
{
	return ((x >> 16) & 0x00000007);
}

static inline uint32_t get_wacs2_req(uint32_t x)
{
	return ((x >> 19) & 0x00000001);
}

static inline uint32_t get_sync_idle2(uint32_t x)
{
	return ((x >> 20) & 0x00000001);
}

static inline uint32_t get_init_done2(uint32_t x)
{
	return ((x >> 21) & 0x00000001);
}

static inline uint32_t get_sys_idle2(uint32_t x)
{
	return ((x >> 22) & 0x00000001);
}

static inline uint32_t get_wacs2_fifo_fillcnt(uint32_t x)
{
	return ((x >> 24) & 0x0000000f);
}

static inline uint32_t get_wacs2_fifo_freecnt(uint32_t x)
{
	return ((x >> 28) & 0x0000000f);
}

static inline u32 wait_for_fsm_idle(u32 x)
{
	return get_wacs0_fsm(x) != WACS_FSM_IDLE;
}

static inline u32 wait_for_fsm_vldclr(u32 x)
{
	return get_wacs0_fsm(x) != WACS_FSM_WFVLDCLR;
}

static inline u32 wait_for_sync(u32 x)
{
	return get_sync_idle0(x) != WACS_SYNC_IDLE;
}

static inline u32 wait_for_idle_and_sync(u32 x)
{
	return (get_wacs0_fsm(x) != WACS_FSM_IDLE)
	       || (get_sync_idle0(x) != WACS_SYNC_IDLE);
}

static inline u32 wait_for_wrap_idle(u32 x)
{
	return (get_wrap_fsm(x) != 0x0)
	       || (get_wrap_ch_dle_restcnt(x) != 0x0);
}

static inline u32 wait_for_wrap_state_idle(u32 x)
{
	return get_wrap_ag_dle_restcnt(x) != 0;
}

static inline u32 wait_for_man_idle_and_noreq(u32 x)
{
	return (get_man_req(x) != MAN_FSM_NO_REQ)
	       || (get_man_fsm(x) != MAN_FSM_IDLE);
}

static inline u32 wait_for_man_vldclr(u32 x)
{
	return get_man_fsm(x) != MAN_FSM_WFVLDCLR;
}

static inline u32 wait_for_cipher_ready(u32 x)
{
	return x != 3;
}

static inline u32 wait_for_stdupd_idle(u32 x)
{
	return get_staupd_fsm(x) != 0x0;
}

static inline u32 wait_for_state_ready_init(loop_condition_fp fp,
					    u32 timeout_us, u32 wacs_register,
					    u32 *read_reg)
{
	u32 reg_rdata = 0x0;
	struct stopwatch sw;

	stopwatch_init_usecs_expire(&sw, timeout_us);

	do {
		reg_rdata = WRAP_RD32(wacs_register);

		if (stopwatch_expired(&sw)) {
			PWRAPERR("wait_for_state_ready_init timeout when waiting for idle\n");
			return E_PWR_WAIT_IDLE_TIMEOUT;
		}
	} while (fp(reg_rdata));        /* IDLE State */
	if (read_reg)
		*read_reg = reg_rdata;
	return 0;
}

static inline u32 wait_for_state_idle_init(loop_condition_fp fp, u32 timeout_us,
					   u32 wacs_register,
					   u32 wacs_vldclr_register,
					   u32 *read_reg)
{
	u32 reg_rdata;

	struct stopwatch sw;

	stopwatch_init_usecs_expire(&sw, timeout_us);
	do {
		reg_rdata = WRAP_RD32(wacs_register);
		/* if last read command timeout,clear vldclr bit
		read command state machine:FSM_REQ-->wfdle-->WFVLDCLR;
		write:FSM_REQ-->idle */
		switch (get_wacs0_fsm(reg_rdata)) {
		case WACS_FSM_WFVLDCLR:
			WRAP_WR32(wacs_vldclr_register, 1);
			PWRAPERR("WACS_FSM = PMIC_WRAP_WACS_VLDCLR\n");
			break;
		case WACS_FSM_WFDLE:
			PWRAPERR("WACS_FSM = WACS_FSM_WFDLE\n");
			break;
		case WACS_FSM_REQ:
			PWRAPERR("WACS_FSM = WACS_FSM_REQ\n");
			break;
		default:
			break;
		}

		if (stopwatch_expired(&sw)) {
			PWRAPERR("wait_for_state_idle_init timeout when waiting for idle\n");
			pwrap_dump_ap_register();
			return E_PWR_WAIT_IDLE_TIMEOUT;
		}
	} while (fp(reg_rdata));        /* IDLE State */
	if (read_reg)
		*read_reg = reg_rdata;
	return 0;
}

static inline u32 wait_for_state_idle(loop_condition_fp fp, u32 timeout_us,
				      u32 wacs_register,
				      u32 wacs_vldclr_register, u32 *read_reg)
{
	u32 reg_rdata;

	struct stopwatch sw;

	stopwatch_init_usecs_expire(&sw, timeout_us);
	do {
		reg_rdata = WRAP_RD32(wacs_register);
		if (get_init_done0(reg_rdata) != WACS_INIT_DONE) {
			PWRAPERR("initialization isn't finished \n");
			return E_PWR_NOT_INIT_DONE;
		}
		/* if last read command timeout,clear vldclr bit
		   read command state machine:FSM_REQ-->wfdle-->WFVLDCLR;
		   write:FSM_REQ-->idle */
		switch (get_wacs0_fsm(reg_rdata)) {
		case WACS_FSM_WFVLDCLR:
			WRAP_WR32(wacs_vldclr_register, 1);
			PWRAPERR("WACS_FSM = PMIC_WRAP_WACS_VLDCLR\n");
			break;
		case WACS_FSM_WFDLE:
			PWRAPERR("WACS_FSM = WACS_FSM_WFDLE\n");
			break;
		case WACS_FSM_REQ:
			PWRAPERR("WACS_FSM = WACS_FSM_REQ\n");
			break;
		default:
			break;
		}

		if (stopwatch_expired(&sw)) {
			PWRAPERR("wait_for_state_idle timeout when waiting for idle\n");
			pwrap_dump_ap_register();
			return E_PWR_WAIT_IDLE_TIMEOUT;
		}
	} while (fp(reg_rdata));        /* IDLE State */
	if (read_reg)
		*read_reg = reg_rdata;
	return 0;
}

static inline u32 wait_for_state_ready(loop_condition_fp fp, u32 timeout_us,
				       u32 wacs_register, u32 *read_reg)
{
	u32 reg_rdata;
	struct stopwatch sw;

	stopwatch_init_usecs_expire(&sw, timeout_us);
	do {
		reg_rdata = WRAP_RD32(wacs_register);

		if (get_init_done0(reg_rdata) != WACS_INIT_DONE) {
			PWRAPERR("initialization isn't finished \n");
			return E_PWR_NOT_INIT_DONE;
		}
		if (stopwatch_expired(&sw)) {
			PWRAPERR("timeout when waiting for idle\n");
			return E_PWR_WAIT_IDLE_TIMEOUT;
		}
	} while (fp(reg_rdata));        /* IDLE State */
	if (read_reg)
		*read_reg = reg_rdata;
	return 0;
}

/* external API for pmic_wrap user */

s32 pwrap_read(u32 adr, u32 *rdata)
{
	return pwrap_wacs2(0, adr, 0, rdata);
}

s32 pwrap_write(u32 adr, u32 wdata)
{
	return pwrap_wacs2(1, adr, wdata, 0);
}

s32 pwrap_wacs2(u32 write, u32 adr, u32 wdata, u32 *rdata)
{
	u32 reg_rdata = 0;
	u32 wacs_write = 0;
	u32 wacs_adr = 0;
	u32 wacs_cmd = 0;
	u32 return_value = 0;

	/* Check argument validation */
	if ((write & ~(0x1)) != 0)
		return E_PWR_INVALID_RW;
	if ((adr & ~(0xffff)) != 0)
		return E_PWR_INVALID_ADDR;
	if ((wdata & ~(0xffff)) != 0)
		return E_PWR_INVALID_WDAT;

	/* Check IDLE & INIT_DONE in advance */
	return_value = wait_for_state_idle(wait_for_fsm_idle, TIMEOUT_WAIT_IDLE,
					   PMIC_WRAP_WACS2_RDATA,
					   PMIC_WRAP_WACS2_VLDCLR, 0);
	if (return_value != 0) {
		PWRAPERR("wait_for_fsm_idle fail,return_value=%d\n",
			 return_value);
		goto FAIL;
	}
	wacs_write = write << 31;
	wacs_adr = (adr >> 1) << 16;
	wacs_cmd = wacs_write | wacs_adr | wdata;

	WRAP_WR32(PMIC_WRAP_WACS2_CMD, wacs_cmd);
	if (write == 0) {
		if (NULL == rdata) {
			PWRAPERR("rdata is a NULL pointer\n");
			return_value = E_PWR_INVALID_ARG;
			goto FAIL;
		}
		return_value = wait_for_state_ready(wait_for_fsm_vldclr,
						    TIMEOUT_READ,
						    PMIC_WRAP_WACS2_RDATA,
						    &reg_rdata);
		if (return_value != 0) {
			PWRAPERR("wait_for_fsm_vldclr fail,return_value=%d\n",
				 return_value);
			/*
			 * E_PWR_NOT_INIT_DONE_READ or
			 * E_PWR_WAIT_IDLE_TIMEOUT_READ
			 */
			return_value += 1;
			goto FAIL;
		}
		*rdata = get_wacs0_rdata(reg_rdata);
		WRAP_WR32(PMIC_WRAP_WACS2_VLDCLR, 1);
	}
FAIL:
	return return_value;
}

static s32 pwrap_read_nochk(u32 adr, u32 *rdata)
{
	return _pwrap_wacs2_nochk(0, adr, 0, rdata);
}

static s32 pwrap_write_nochk(u32 adr, u32 wdata)
{
	return _pwrap_wacs2_nochk(1, adr, wdata, 0);
}
static s32 _pwrap_wacs2_nochk(u32 write, u32 adr, u32 wdata, u32 *rdata)
{
	u32 reg_rdata = 0x0;
	u32 wacs_write = 0x0;
	u32 wacs_adr = 0x0;
	u32 wacs_cmd = 0x0;
	u32 return_value = 0x0;

	/* Check argument validation */
	if ((write & ~(0x1)) != 0)
		return E_PWR_INVALID_RW;
	if ((adr & ~(0xffff)) != 0)
		return E_PWR_INVALID_ADDR;
	if ((wdata & ~(0xffff)) != 0)
		return E_PWR_INVALID_WDAT;

	/* Check IDLE */
	return_value = wait_for_state_idle_init(wait_for_fsm_idle,
						TIMEOUT_WAIT_IDLE,
						PMIC_WRAP_WACS2_RDATA,
						PMIC_WRAP_WACS2_VLDCLR, 0);
	if (return_value != 0) {
		PWRAPERR("wait_for_fsm_idle fail,return_value=%d\n",
			 return_value);
		return return_value;
	}
	wacs_write = write << 31;
	wacs_adr = (adr >> 1) << 16;
	wacs_cmd = wacs_write | wacs_adr | wdata;
	WRAP_WR32(PMIC_WRAP_WACS2_CMD, wacs_cmd);

	if (write == 0) {
		if (NULL == rdata) {
			PWRAPERR("rdata is a NULL pointer\n");
			return_value = E_PWR_INVALID_ARG;
			return return_value;
		}
		return_value =
			wait_for_state_ready_init(wait_for_fsm_vldclr, TIMEOUT_READ,
						  PMIC_WRAP_WACS2_RDATA,
						  &reg_rdata);
		if (return_value != 0) {
			PWRAPERR("wait_for_fsm_vldclr fail,return_value=%d\n",
				 return_value);
			/*
			 * E_PWR_NOT_INIT_DONE_READ or
			 * E_PWR_WAIT_IDLE_TIMEOUT_READ
			 */
			return_value += 1;
			return return_value;
		}
		*rdata = get_wacs0_rdata(reg_rdata);
		WRAP_WR32(PMIC_WRAP_WACS2_VLDCLR, 1);
	}
	return 0;
}

/* call it in pwrap_init,mustn't check init done */
static s32 _pwrap_init_dio(u32 dio_en)
{
	u32 arb_en_backup = 0x0;
	u32 rdata = 0x0;
	u32 return_value = 0;

	arb_en_backup = WRAP_RD32(PMIC_WRAP_HIPRIO_ARB_EN);
	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN, WACS2);      /* only WACS2 */
	pwrap_write_nochk(DEW_DIO_EN, dio_en);

	/* Check IDLE & INIT_DONE in advance */
	return_value =
		wait_for_state_ready_init(wait_for_idle_and_sync, TIMEOUT_WAIT_IDLE,
					  PMIC_WRAP_WACS2_RDATA, 0);
	if (return_value != 0) {
		PWRAPERR("_pwrap_init_dio fail,return_value=%x\n",
			 return_value);
		return return_value;
	}
	WRAP_WR32(PMIC_WRAP_DIO_EN, dio_en);
	/* Read Test */
	pwrap_read_nochk(DEW_READ_TEST, &rdata);
	if (rdata != DEFAULT_VALUE_READ_TEST) {
		PWRAPERR("[Dio_mode][Read Test] fail,dio_en = %x, READ_TEST rdata=%x, exp=0x5aa5\n",
			 dio_en, rdata);
		return E_PWR_READ_TEST_FAIL;
	}
	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN, arb_en_backup);
	return 0;
}

static s32 _pwrap_init_cipher(void)
{
	u32 arb_en_backup = 0;
	u32 rdata = 0;
	u32 return_value = 0;
	struct stopwatch sw;

	arb_en_backup = WRAP_RD32(PMIC_WRAP_HIPRIO_ARB_EN);

	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN, WACS2);      /* only WACS0 */

	WRAP_WR32(PMIC_WRAP_CIPHER_SWRST, 1);
	WRAP_WR32(PMIC_WRAP_CIPHER_SWRST, 0);
	WRAP_WR32(PMIC_WRAP_CIPHER_KEY_SEL, 1);
	WRAP_WR32(PMIC_WRAP_CIPHER_IV_SEL, 2);
	WRAP_WR32(PMIC_WRAP_CIPHER_EN, 1);

	/* Config CIPHER @ PMIC */
	pwrap_write_nochk(DEW_CIPHER_SWRST, 0x1);
	pwrap_write_nochk(DEW_CIPHER_SWRST, 0x0);
	pwrap_write_nochk(DEW_CIPHER_KEY_SEL, 0x1);
	pwrap_write_nochk(DEW_CIPHER_IV_SEL, 0x2);
	pwrap_write_nochk(DEW_CIPHER_LOAD, 0x1);
	pwrap_write_nochk(DEW_CIPHER_START, 0x1);

	/* wait for cipher data ready@AP */
	return_value =
		wait_for_state_ready_init(wait_for_cipher_ready, TIMEOUT_WAIT_IDLE,
					  PMIC_WRAP_CIPHER_RDY, 0);
	if (return_value != 0) {
		PWRAPERR("wait for cipher data ready@AP fail,return_value=%x\n",
			 return_value);
		return return_value;
	}
	/* wait for cipher data ready@PMIC */
	stopwatch_init_usecs_expire(&sw, TIMEOUT_WAIT_IDLE);

	do {
		pwrap_read_nochk(DEW_CIPHER_RDY, &rdata);
		if (stopwatch_expired(&sw)) {
			PWRAPERR("wait for cipher data ready:timeout when waiting for idle\n");
			return E_PWR_WAIT_IDLE_TIMEOUT;
		}
	} while (rdata != 0x1); /* cipher_ready */

	pwrap_write_nochk(DEW_CIPHER_MODE, 0x1);
	/* wait for cipher mode idle */
	return_value =
		wait_for_state_ready_init(wait_for_idle_and_sync, TIMEOUT_WAIT_IDLE,
					  PMIC_WRAP_WACS2_RDATA, 0);
	if (return_value != 0) {
		PWRAPERR("wait for cipher mode idle fail,return_value=%x\n",
			 return_value);
		return return_value;
	}
	WRAP_WR32(PMIC_WRAP_CIPHER_MODE, 1);

	/* Read Test */
	pwrap_read_nochk(DEW_READ_TEST, &rdata);
	if (rdata != DEFAULT_VALUE_READ_TEST) {
		PWRAPERR("_pwrap_init_cipher,read test error,error code=%x, rdata=%x\n",
			 1, rdata);
		return E_PWR_READ_TEST_FAIL;
	}

	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN, arb_en_backup);
	return 0;
}

static s32 _pwrap_init_sidly(void)
{
	u32 arb_en_backup = 0;
	u32 rdata = 0;
	u32 ind = 0;
	u32 lead = 0;
	u32 tail = 0;
	u32 result = 0;
	u32 result_faulty = 0;
	u32 leading_one, tailing_one;

	arb_en_backup = WRAP_RD32(PMIC_WRAP_HIPRIO_ARB_EN);

	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN, WACS2);      /* only WACS2 */

	/* Scan all possible input strobe by READ_TEST */
	/* 24 sampling clock edge */
	for (ind = 0; ind < 4; ind++) {
		WRAP_WR32(PMIC_WRAP_SI_CK_CON, (ind >> 2) & 0x7);
		WRAP_WR32(PMIC_WRAP_SIDLY, 0x3 - (ind & 0x3));
		_pwrap_wacs2_nochk(0, DEW_READ_TEST, 0, &rdata);
		if (rdata == DEFAULT_VALUE_READ_TEST) {
			PWRAPLOG("_pwrap_init_sidly [Read Test] pass,index=%d rdata=%x\n",
				 ind, rdata);
			result |= (0x1 << ind);
		} else {
			PWRAPLOG("_pwrap_init_sidly [Read Test] fail,SIDLY=%x,rdata=%x\n",
				 ind, rdata);
		}
	}

	/* Locate the leading one and trailing one of PMIC 1/2 */
	for (ind = 23; ind >= 0; ind--)
		if (result & (0x1 << ind)) break;
	leading_one = ind;

	for (ind = 0; ind < 24; ind++)
		if (result & (0x1 << ind)) break;
	tailing_one = ind;

	/* Check the continuity of pass range */
	lead = (0x1 << (leading_one + 1)) - 1;
	tail = (0x1 << tailing_one) - 1;
	if ((lead - tail) != result) {
		PWRAPERR("_pwrap_init_sidly Fail,lead=%x,tail=%x\n", lead, tail);
		result_faulty = 0x1;
	}

	/* Config SICK and SIDLY to the middle point of pass range */

	ind = (leading_one + tailing_one) / 2;
	WRAP_WR32(PMIC_WRAP_SI_CK_CON, (ind >> 2) & 0x7);
	WRAP_WR32(PMIC_WRAP_SIDLY, 0x3 - (ind & 0x3));

	/* Restore */
	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN, arb_en_backup);
	if (result_faulty == 0) {
		return 0;
	} else {
		PWRAPERR("_pwrap_init_sidly fail,result=%x\n", result);
		return result_faulty;
	}
}

static s32 _pwrap_reset_spislv(void)
{
	u32 ret = 0;
	u32 return_value = 0;

	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN, 0);
	WRAP_WR32(PMIC_WRAP_WRAP_EN, 0);
	WRAP_WR32(PMIC_WRAP_MUX_SEL, 1);
	WRAP_WR32(PMIC_WRAP_MAN_EN, 1);
	WRAP_WR32(PMIC_WRAP_DIO_EN, 0);

	WRAP_WR32(PMIC_WRAP_MAN_CMD, (OP_WR << 13) | (OP_CSL << 8));
	/* to reset counter */
	WRAP_WR32(PMIC_WRAP_MAN_CMD, (OP_WR << 13) | (OP_OUTS << 8));
	WRAP_WR32(PMIC_WRAP_MAN_CMD, (OP_WR << 13) | (OP_CSH << 8));
	WRAP_WR32(PMIC_WRAP_MAN_CMD, (OP_WR << 13) | (OP_OUTS << 8));
	WRAP_WR32(PMIC_WRAP_MAN_CMD, (OP_WR << 13) | (OP_OUTS << 8));
	WRAP_WR32(PMIC_WRAP_MAN_CMD, (OP_WR << 13) | (OP_OUTS << 8));
	WRAP_WR32(PMIC_WRAP_MAN_CMD, (OP_WR << 13) | (OP_OUTS << 8));

	return_value = wait_for_state_ready_init(wait_for_sync,
						 TIMEOUT_WAIT_IDLE,
						 PMIC_WRAP_WACS2_RDATA, 0);
	if (return_value != 0) {
		PWRAPERR("_pwrap_reset_spislv fail,return_value=%x\n",
			 return_value);
		ret = E_PWR_TIMEOUT;
	}

	WRAP_WR32(PMIC_WRAP_MAN_EN, 0);
	WRAP_WR32(PMIC_WRAP_MUX_SEL, 0);

	return ret;
}

static s32 _pwrap_init_reg_clock(u32 regck_sel)
{
	u32 wdata = 0;
	u32 rdata = 0;

	/* Set reg clk freq */
	pwrap_read_nochk(PMIC_TOP_CKCON2, &rdata);

	if (regck_sel == 1)
		wdata = (rdata & (~(0x3 << 10))) | (0x1 << 10);
	else
		wdata = rdata & (~(0x3 << 10));

	pwrap_write_nochk(PMIC_TOP_CKCON2, wdata);
	pwrap_read_nochk(PMIC_TOP_CKCON2, &rdata);
	if (rdata != wdata) {
		PWRAPERR("_pwrap_init_reg_clock,PMIC_TOP_CKCON2 Write [15]=1 Fail, rdata=%x\n",
			 rdata);
		return E_PWR_INIT_REG_CLOCK;
	}
	/* Config SPI Waveform according to reg clk */
	if (regck_sel == 1) {   /* 18MHz */
		WRAP_WR32(PMIC_WRAP_RDDMY, 0xc);
		WRAP_WR32(PMIC_WRAP_CSHEXT_WRITE, 0x0);
		WRAP_WR32(PMIC_WRAP_CSHEXT_READ, 0x4);
		WRAP_WR32(PMIC_WRAP_CSLEXT_START, 0x0);
		WRAP_WR32(PMIC_WRAP_CSLEXT_END, 0x4);
	} else if (regck_sel == 2) {    /* 24MHz */
		WRAP_WR32(PMIC_WRAP_RDDMY, 0xc);
		WRAP_WR32(PMIC_WRAP_CSHEXT_WRITE, 0x0);
		WRAP_WR32(PMIC_WRAP_CSHEXT_READ, 0x4);
		WRAP_WR32(PMIC_WRAP_CSLEXT_START, 0x2);
		WRAP_WR32(PMIC_WRAP_CSLEXT_END, 0x2);
	} else {                /* Safe mode */
		WRAP_WR32(PMIC_WRAP_RDDMY, 0xf);
		WRAP_WR32(PMIC_WRAP_CSHEXT_WRITE, 0xf);
		WRAP_WR32(PMIC_WRAP_CSHEXT_READ, 0xf);
		WRAP_WR32(PMIC_WRAP_CSLEXT_START, 0xf);
		WRAP_WR32(PMIC_WRAP_CSLEXT_END, 0xf);
	}
	return 0;
}

s32 pwrap_init(void)
{
	s32 sub_return = 0;
	s32 sub_return1 = 0;
	u32 rdata = 0x0;

	WRAP_SET_BIT(1 << 7, INFRA_GLOBALCON_RST0);
	PWRAPLOG("the reset register =%x\n", WRAP_RD32(INFRA_GLOBALCON_RST0));
	PWRAPLOG("PMIC_WRAP_STAUPD_GRPEN =0x%x,it should be equal to 0xc\n", WRAP_RD32(PMIC_WRAP_STAUPD_GRPEN));
	/* clear reset bit */
	WRAP_CLR_BIT(1 << 7, INFRA_GLOBALCON_RST0);

	/* Set SPI_CK_freq = 26MHz */
	WRAP_WR32(CLK_CFG_5_CLR, CLK_SPI_CK_26M);

	/* Enable DCM */
	WRAP_WR32(PMIC_WRAP_DCM_EN, 3);
	WRAP_WR32(PMIC_WRAP_DCM_DBC_PRD, 0);

	/* Reset SPISLV */
	sub_return = _pwrap_reset_spislv();
	if (sub_return != 0) {
		PWRAPERR("error,_pwrap_reset_spislv fail,sub_return=%x\n",
			 sub_return);
		return E_PWR_INIT_RESET_SPI;
	}
	/* Enable WACS2 */
	WRAP_WR32(PMIC_WRAP_WRAP_EN, 1);
	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN, WACS2);      /* Only WACS2 */
	WRAP_WR32(PMIC_WRAP_WACS2_EN, 1);

	/* SIDLY setting */
	sub_return = _pwrap_init_sidly();
	if (sub_return != 0) {
		PWRAPERR("error,_pwrap_init_sidly fail,sub_return=%x\n",
			 sub_return);
		return E_PWR_INIT_SIDLY;
	}
	/*
	 * SPI Waveform Configuration
	 * 0:safe mode, 1:12MHz, 2:24MHz
	 */
	sub_return = _pwrap_init_reg_clock(2);
	if (sub_return != 0) {
		PWRAPERR("error,_pwrap_init_reg_clock fail,sub_return=%x\n",
			 sub_return);
		return E_PWR_INIT_REG_CLOCK;
	}
	/*
	 * Enable PMIC
	 * (May not be necessary, depending on S/W partition)
	 */
	pwrap_read_nochk(PMIC_WRP_CKPDN, &rdata);
	sub_return = pwrap_write_nochk(PMIC_WRP_CKPDN, rdata & 0x50);
	/* clear dewrap reset bit */
	sub_return1 = pwrap_write_nochk(PMIC_WRP_RST_CON, 0);
	if ((sub_return != 0) || (sub_return1 != 0)) {
		PWRAPERR("Enable PMIC fail, sub_return=%x sub_return1=%x\n",
			 sub_return, sub_return1);
		return E_PWR_INIT_ENABLE_PMIC;
	}
	/* Enable DIO mode */
	sub_return = _pwrap_init_dio(1);
	if (sub_return != 0) {
		PWRAPERR("_pwrap_init_dio test error,error code=%x, sub_return=%x\n",
			 0x11, sub_return);
		return E_PWR_INIT_DIO;
	}
	/* Enable Encryption */
	sub_return = _pwrap_init_cipher();
	if (sub_return != 0) {
		PWRAPERR("Enable Encryption fail, return=%x\n", sub_return);
		return E_PWR_INIT_CIPHER;
	}
	/* Write test using WACS2 */
	sub_return = pwrap_write_nochk(DEW_WRITE_TEST, WRITE_TEST_VALUE);
	sub_return1 = pwrap_read_nochk(DEW_WRITE_TEST, &rdata);
	if ((rdata != WRITE_TEST_VALUE) || (sub_return != 0)
	    || (sub_return1 != 0)) {
		PWRAPERR("write test error,rdata=0x%x,exp=0xa55a,sub_return=0x%x,sub_return1=0x%x\n",
			 rdata, sub_return, sub_return1);
		return E_PWR_INIT_WRITE_TEST;
	}
	/*
	 * Signature Checking - Using Write Test Register
	 * should be the last to modify WRITE_TEST
	 */

	/* Signature Checking - Using CRC
	 * should be the last to modify WRITE_TEST
	 */
	sub_return = pwrap_write_nochk(DEW_CRC_EN, 0x1);
	if (sub_return != 0) {
		PWRAPERR("enable CRC fail,sub_return=%x\n", sub_return);
		return E_PWR_INIT_ENABLE_CRC;
	}
	WRAP_WR32(PMIC_WRAP_CRC_EN, 0x1);
	WRAP_WR32(PMIC_WRAP_SIG_MODE, 0x0);
	WRAP_WR32(PMIC_WRAP_SIG_ADR, DEW_CRC_VAL);

	/* PMIC_WRAP enables */
	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN, 0x1ff);
	WRAP_WR32(PMIC_WRAP_WACS0_EN, 0x1);
	WRAP_WR32(PMIC_WRAP_WACS1_EN, 0x1);
	WRAP_WR32(PMIC_WRAP_WACS2_EN, 0x1);     /* already enabled */
	/* 0x1:20us,for concurrence test,MP:0x5; 100us */
	WRAP_WR32(PMIC_WRAP_STAUPD_PRD, 0x5);
	WRAP_WR32(PMIC_WRAP_STAUPD_GRPEN, 0xf1);
	WRAP_WR32(PMIC_WRAP_WDT_UNIT, 0xf);
	WRAP_WR32(PMIC_WRAP_WDT_SRC_EN, 0xffffffff);
	WRAP_WR32(PMIC_WRAP_TIMER_EN, 0x1);
	WRAP_WR32(PMIC_WRAP_INT_EN, 0x7ffffffd); /* except for [31] debug_int */

	/*
	 * switch event pin from usbdl mode to normal mode for pmic interrupt,
	 * NEW@MT6397
	 */
	pwrap_read_nochk(PMIC_TOP_CKCON3, &rdata);
	sub_return = pwrap_write_nochk(PMIC_TOP_CKCON3, (rdata & 0x0007));
	if (sub_return != 0)
		PWRAPERR("!!switch event pin fail,sub_return=%d\n", sub_return);

	/* PERI_PWRAP_BRIDGE enables */
	WRAP_WR32(PERI_PWRAP_BRIDGE_IORD_ARB_EN, 0x7f);
	WRAP_WR32(PERI_PWRAP_BRIDGE_WACS3_EN, 0x1);
	WRAP_WR32(PERI_PWRAP_BRIDGE_WACS4_EN, 0x1);
	WRAP_WR32(PERI_PWRAP_BRIDGE_WDT_UNIT, 0xf);
	WRAP_WR32(PERI_PWRAP_BRIDGE_WDT_SRC_EN, 0xffff);
	WRAP_WR32(PERI_PWRAP_BRIDGE_TIMER_EN, 0x1);
	WRAP_WR32(PERI_PWRAP_BRIDGE_INT_EN, 0x7ff);

	/* PMIC_DEWRAP enables */
	sub_return = pwrap_write_nochk(DEW_EVENT_OUT_EN, 0x1);
	sub_return1 = pwrap_write_nochk(DEW_EVENT_SRC_EN, 0xffff);
	if ((sub_return != 0) || (sub_return1 != 0)) {
		PWRAPERR("enable dewrap fail,sub_return=%d,sub_return1=%d\n",
			 sub_return, sub_return1);
		return E_PWR_INIT_ENABLE_DEWRAP;
	}

	/* Initialization Done */
	WRAP_WR32(PMIC_WRAP_INIT_DONE2, 0x1);

	/* TBD: Should be configured by MD MCU */
	WRAP_WR32(PMIC_WRAP_INIT_DONE0, 1);
	WRAP_WR32(PMIC_WRAP_INIT_DONE1, 1);
	WRAP_WR32(PERI_PWRAP_BRIDGE_INIT_DONE3, 1);
	WRAP_WR32(PERI_PWRAP_BRIDGE_INIT_DONE4, 1);
	return 0;
}

void pwrap_dump_ap_register(void)
{
	u32 i = 0;
	u32 reg_addr = 0;
	u32 reg_value = 0;

	PWRAPREG("dump pwrap register\n");
	for (i = 0; i <= 89; i++) {
		reg_addr = (PMIC_WRAP_BASE + i * 4);
		reg_value = WRAP_RD32(reg_addr);
		PWRAPREG("0x%x=0x%x\n", reg_addr, reg_value);
	}
	PWRAPREG("dump peri_pwrap register\n");
	for (i = 0; i <= 24; i++) {
		reg_addr = (PERI_PWRAP_BRIDGE_BASE + i * 4);
		reg_value = WRAP_RD32(reg_addr);
		PWRAPREG("0x%x=0x%x\n", reg_addr, reg_value);
	}
}

s32 pwrap_init_preloader(void)
{
	u32 pwrap_ret = 0, i = 0;

	PWRAPFUC();
	/* retry 3 times for pmic wrapper init */
	for (i = 0; i < 3; i++) {
		pwrap_ret = pwrap_init();
		if (pwrap_ret != 0) {
			printk(BIOS_INFO,
			       "[PMIC_WRAP]wrap_init fail,the return value=%x.\n",
			       pwrap_ret);
		} else {
			printk(BIOS_INFO,
			       "[PMIC_WRAP]wrap_init pass,the return value=%x.\n",
			       pwrap_ret);
			break;  /* init pass */
		}
	}
	return 0;
}
