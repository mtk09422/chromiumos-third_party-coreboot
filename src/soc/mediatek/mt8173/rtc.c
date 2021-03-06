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
#include <timer.h>
#include <console/console.h>
#include <assert.h>
#include <delay.h>
#include <soc/mt8173.h>
#include <rtc.h>
#include <soc/rtc.h>
#include <soc/pmic_wrap_init.h>
#include <mainboard/cust_rtc.h>
#include <soc/pmic.h>

#define RTC_GPIO_USER_MASK	  (((1U << 13) - 1) & 0xff00)

static bool recovery_flag = false;
static bool g_rtc_2sec_stat;

static u16 RTC_Read(u32 addr)
{
	u32 rdata = 0;
	pwrap_read((u32) addr, &rdata);
	return (u16) rdata;
}

static void RTC_Write(u32 addr, u16 data)
{
	pwrap_write((u32) addr, (u32) data);
}

static inline bool rtc_busy_wait(void)
{
	struct stopwatch sw;
	stopwatch_init_usecs_expire(&sw, 1000 * USECS_PER_MSEC);

	while (RTC_Read(RTC_BBPU) & RTC_BBPU_CBUSY) {
		/* Time > 1sec, time out and set recovery mode enable.*/
		if (stopwatch_expired(&sw)) {
			print_info("[RTC] rtc cbusy time out!!!!!\n");
			return false;
		}
	}

	return true;
}
static bool Write_trigger(void)
{
	RTC_Write(RTC_WRTGR, 1);
	if (rtc_busy_wait())
		return true;
	else
		return false;
}

static bool Writeif_unlock(void)
{
	RTC_Write(RTC_PROT, RTC_PROT_UNLOCK1);
	if (!Write_trigger())
		return false;
	RTC_Write(RTC_PROT, RTC_PROT_UNLOCK2);
	if (!Write_trigger())
		return false;

	return true;
}

int rtc_set(const struct rtc_time *time)
{

	RTC_Write(RTC_TC_YEA, time->year - RTC_MIN_YEAR);
	RTC_Write(RTC_TC_MTH, time->mon);
	RTC_Write(RTC_TC_DOM, time->mday);
	RTC_Write(RTC_TC_HOU, time->hour);
	RTC_Write(RTC_TC_MIN, time->min);
	RTC_Write(RTC_TC_SEC, time->sec);

	return 0;
}

int rtc_get(struct rtc_time *time)
{
	time->sec = RTC_Read(RTC_TC_SEC);
	time->min = RTC_Read(RTC_TC_MIN);
	time->hour = RTC_Read(RTC_TC_HOU);
	time->mday = RTC_Read(RTC_TC_DOM);
	time->mon = RTC_Read(RTC_TC_MTH);
	time->year = RTC_Read(RTC_TC_YEA) + RTC_MIN_YEAR;

	return 0;
}

static void rtc_set_recovery_mode_stat(bool enable)
{
	recovery_flag = enable;
}

static bool rtc_get_recovery_mode_stat(void)
{
	return recovery_flag;
}

static bool rtc_2sec_stat_clear(void)
{
	printk(BIOS_INFO, "[RTC]%s\n", __func__);
	RTC_Write(RTC_CALI, RTC_Read(RTC_CALI) & ~RTC_CALI_BBPU_2SEC_STAT);
	if (!Write_trigger())
		return false;
	RTC_Write(RTC_CALI, RTC_Read(RTC_CALI) | RTC_CALI_BBPU_2SEC_STAT);
	if (!Write_trigger())
		return false;
	RTC_Write(RTC_CALI, RTC_Read(RTC_CALI) & ~RTC_CALI_BBPU_2SEC_STAT);
	if (!Write_trigger())
		return false;

	return true;
}

static void rtc_save_2sec_stat(void)

{
	u16 cali;
	static bool save_stat=false;

	if(save_stat==true)
		return;
	else
		save_stat = true;

	cali = RTC_Read(RTC_CALI);
	printk(BIOS_INFO, "rtc_2sec_reboot_check: CALI = 0x%x\n",cali);
	if (cali & RTC_CALI_BBPU_2SEC_EN) {
		switch((cali & RTC_CALI_BBPU_2SEC_MODE_MSK) >> RTC_CALI_BBPU_2SEC_MODE_SHIFT) {
			case 0:
			case 1:
			case 2:
				if(cali & RTC_CALI_BBPU_2SEC_STAT) {
					rtc_2sec_stat_clear();
					g_rtc_2sec_stat = true;
				} else {
					rtc_2sec_stat_clear();
					g_rtc_2sec_stat = false;
				}
				break;
			case 3:
				rtc_2sec_stat_clear();
				g_rtc_2sec_stat = true;
			default:
				break;
		}
	}
}

bool rtc_2sec_reboot_check(void)
{
	if(1 == RTC_2SEC_REBOOT_ENABLE)
		return g_rtc_2sec_stat;
	else
		return false;
}

static void rtc_enable_2sec_reboot(void)
{
	u16 cali;

	cali = RTC_Read(RTC_CALI) | RTC_CALI_BBPU_2SEC_EN;
	cali = (cali & ~(RTC_CALI_BBPU_2SEC_MODE_MSK)) | (RTC_2SEC_MODE << RTC_CALI_BBPU_2SEC_MODE_SHIFT);
	RTC_Write(RTC_CALI, cali);
	Write_trigger();
}

static void rtc_xosc_write(u16 val)
{
	u16 bbpu;

	RTC_Write(RTC_OSC32CON, RTC_OSC32CON_UNLOCK1);
	mdelay(1);
	RTC_Write(RTC_OSC32CON, RTC_OSC32CON_UNLOCK2);
	mdelay(1);

	RTC_Write(RTC_OSC32CON, val);
	mdelay(1);
	bbpu = RTC_Read(RTC_BBPU) | RTC_BBPU_KEY | RTC_BBPU_RELOAD;
	RTC_Write(RTC_BBPU, bbpu);
	Write_trigger();
}

static bool rtc_android_init(void)
{
	u16 irqsta;

	RTC_Write(RTC_IRQ_EN, 0);
	RTC_Write(RTC_CII_EN, 0);
	RTC_Write(RTC_AL_MASK, 0);

	RTC_Write(RTC_AL_YEA, 1970 - RTC_MIN_YEAR);
	RTC_Write(RTC_AL_MTH, 1);
	RTC_Write(RTC_AL_DOM, 1);
	RTC_Write(RTC_AL_DOW, 1);
	RTC_Write(RTC_AL_HOU, 0);
	RTC_Write(RTC_AL_MIN, 0);
	RTC_Write(RTC_AL_SEC, 0);

	RTC_Write(RTC_PDN1, 0x8000);   /* set Debug bit */
	RTC_Write(RTC_PDN2, ((1970 - RTC_MIN_YEAR) << 8) | 1);
	RTC_Write(RTC_SPAR0, 0);
	RTC_Write(RTC_SPAR1, (1 << 11));

	RTC_Write(RTC_DIFF, 0);
	RTC_Write(RTC_CALI, 0);
	if (!rtc_2sec_stat_clear())
		return false;
	if (!Write_trigger())
		return false;

	irqsta = RTC_Read(RTC_IRQ_STA);	/* read clear */

	/* init time counters after resetting RTC_DIFF and RTC_CALI */
	RTC_Write(RTC_TC_YEA, RTC_DEFAULT_YEA - RTC_MIN_YEAR);
	RTC_Write(RTC_TC_MTH, RTC_DEFAULT_MTH);
	RTC_Write(RTC_TC_DOM, RTC_DEFAULT_DOM);
	RTC_Write(RTC_TC_DOW, 1);
	RTC_Write(RTC_TC_HOU, 0);
	RTC_Write(RTC_TC_MIN, 0);
	RTC_Write(RTC_TC_SEC, 0);
	if(!Write_trigger())
		return false;

	return true;
}

static bool rtc_gpio_init(void)
{
	u16 con;

	/* GPI mode and pull enable + pull down */
	con = RTC_Read(RTC_CON) & 0x800c;
	con |= (RTC_CON_GPEN | RTC_CON_GOE);
	con &= ~(RTC_CON_F32KOB);
	RTC_Write(RTC_CON, con);
	if (Write_trigger())
		return true;
	else
		return false;
}

static u16 get_frequency_meter(u16 val, u16 measureSrc, u16 window_size)
{
	u16 ret;

	struct stopwatch sw;

	if (val != 0)
		rtc_xosc_write(val);

	RTC_Write(TOP_RST_CON, 0x0100);	/* FQMTR reset */
	while (!(RTC_Read(FQMTR_CON2) == 0)
	       && (0x8 & RTC_Read(FQMTR_CON0)) == 0x8) ;
	RTC_Write(TOP_RST_CON, 0x0000);	/* FQMTR normal */

	/* set freq. meter window value (0=1X32K(fix clock)) */
	RTC_Write(FQMTR_CON1, window_size);

	/* enable freq. meter, set measure clock to 26Mhz */
	RTC_Write(FQMTR_CON0, 0x8000 | measureSrc);

	mdelay(1);
	stopwatch_init_usecs_expire(&sw, 1000 * USECS_PER_MSEC);
	while ((0x8 & RTC_Read(FQMTR_CON0)) == 0x8) {
		if (stopwatch_expired(&sw)) {
			print_info("get frequency time out\n");
			break;
		}
	};

	/* read data should be closed to 26M/32k = 812.5 */
	ret = RTC_Read(FQMTR_CON2);
	printk(BIOS_INFO, "[RTC] get_frequency_meter: input=0x%x, ouput=%d\n",
	       val, ret);

	return ret;
}

static u16 eosc_cali(void)
{
	u16 val;
	u16 diff;
	int middle;

	printk(BIOS_INFO, "[RTC] EOSC_Cali: TOP_CKCON1=0x%x\n",
	       RTC_Read(TOP_CKCON1));

	int left = 0x24C0, right = 0x24DF;

	RTC_Write(TOP_CKCON2, (RTC_Read(TOP_CKCON2) | 0x8000));
	while (left <= (right)) {
		middle = (right + left) / 2;
		if (middle == left)
			break;

		val = get_frequency_meter(middle, 0x0004, 0);
		if ((val > 792) && (val < 796))
			return middle;
		if (val > 795)
			right = middle;
		else
			left = middle;
	}

	val = get_frequency_meter(left, 0x0004, 0);
	diff = 793 - val;
	val = get_frequency_meter(right, 0x0004, 0);
	if (diff < (val - 793))
		return left;
	else
		return right;
}

static bool rtc_get_xosc_mode(void)
{
	u16 con, xosc_mode;;

	con = RTC_Read(RTC_OSC32CON);

	if((con & 0x0020) == 0)
	{
		xosc_mode = 1;
	}
	else
		xosc_mode = 0;
	return xosc_mode;
}

static void rtc_osc_init(void)
{
	/* disable 32K export if there are no RTC_GPIO users */
	if (!(RTC_Read(RTC_PDN1) & RTC_GPIO_USER_MASK))
		rtc_gpio_init();

	if (rtc_get_xosc_mode()) {
		u16 con;
		RTC_Write(TOP_CKTST2, 0x0);
		con = RTC_Read(RTC_OSC32CON);
		if ((con & 0x000f) != 0x0) {	/* check XOSCCALI */
			/* crystal exist mode + XOSCCALI = 0x3 */
			rtc_xosc_write(0x0003);
			udelay(200);
		}

		/* crystal exist mode + XOSCCALI = 0x0 */
		rtc_xosc_write(0x0000);
	} else {
		u16 val;
		RTC_Write(TOP_CKTST2, 0x80);
		val = eosc_cali();
		printk(BIOS_INFO, "[RTC] EOSC cali val = 0x%x\n", val);
		/* EMB_HW_Mode */
		val = (val & 0x001f) | 0x2400;
		rtc_xosc_write(val);
	}
}

static bool rtc_lpd_init(void)
{
	u16 con;

	con = RTC_Read(RTC_CON) | RTC_CON_LPEN;
	con &= ~RTC_CON_LPRST;
	RTC_Write(RTC_CON, con);
	if (!Write_trigger())
		return false;

	con |= RTC_CON_LPRST;
	RTC_Write(RTC_CON, con);
	if (!Write_trigger())
		return false;

	con &= ~RTC_CON_LPRST;
	RTC_Write(RTC_CON, con);
	if (!Write_trigger())
		return false;

	return true;
}

static void rtc_call_exception(void)
{
	BUG();
}

static bool rtc_xosc_check_clock(u16 * result)
{
	if ((result[0] >= 3 && result[0] <= 7) &&
	    (result[1] > 1500 && result[1] < 6000) &&
	    (result[2] == 0) && (result[3] == 0))
		return true;
	else
		return false;
}

static bool rtc_eosc_check_clock(u16 * result)
{
	if ((result[0] >= 3 && result[0] <= 7) &&
	    (result[1] < 500) &&
	    (result[2] > 2 && result[2] < 9) &&
	    (result[3] > 300 && result[3] < 10400))
		return true;
	else
		return false;
}

static void rtc_measure_four_clock(u16 * result)
{
	u16 window_size;

	RTC_Write(TOP_CKCON2, (RTC_Read(TOP_CKCON2) & ~0x8000));
	window_size = 4;
	mdelay(1);

	/* select 26M as target clock */
	result[0] = get_frequency_meter(0, 0x0004, window_size);

	/* select XOSC_DET as fixed clock */
	RTC_Write(TOP_CKTST2, (RTC_Read(TOP_CKTST2) & ~0x00C0));
	RTC_Write(TOP_CKCON2, (RTC_Read(TOP_CKCON2) | 0x8000));
	window_size = 4;
	mdelay(1);

	/* select 26M as target clock */
	result[1] = get_frequency_meter(0, 0x0004, window_size);

	/* select 26M as fixed clock */
	RTC_Write(TOP_CKCON2, (RTC_Read(TOP_CKCON2) & ~0x8000));
	window_size = 794 * 5;
	mdelay(1);

	/* select DCXO_32 as target clock */
	result[2] = get_frequency_meter(0, 0x0000, window_size);

	/* select DCXO_32 as target clock */
	result[2] = get_frequency_meter(0, 0x0001, window_size);

	/* select EOSC_32 as fixed clock */
	RTC_Write(TOP_CKTST2, ((RTC_Read(TOP_CKTST2) | 0x0080) & ~0x0040));
	RTC_Write(TOP_CKCON2, (RTC_Read(TOP_CKCON2) | 0x8000));
	window_size = 4;
	mdelay(1);

	/* select 26M as target clock */
	result[3] = get_frequency_meter(0, 0x0004, window_size);
}

static void rtc_switch_mode(bool XOSC, bool recovery)
{
	if (XOSC) {
		if (recovery) {
			/* HW bypass switch mode control and set to XOSC */
			RTC_Write(CHRSTATUS,
				  ((RTC_Read(CHRSTATUS) | 0x0800) & ~0x0200));
		}
		/* assume crystal exist mode + XOSCCALI = 0x3 */
		rtc_xosc_write(0x0003);
		if (recovery)
			mdelay(1000);
	} else {
		if (recovery) {
			/* HW bypass switch mode control and set to DCXO */
			RTC_Write(CHRSTATUS, (RTC_Read(CHRSTATUS) | 0x0A00));
		}
		rtc_xosc_write(0x240F);	/*crystal not exist + eosc cali = 0xF */
		mdelay(10);
	}
}

static void rtc_switch_to_xosc_mode(void)
{
	rtc_switch_mode(true, false);
}

static void rtc_switch_to_dcxo_mode(void)
{
	rtc_switch_mode(false, false);
}

static void rtc_switch_to_xosc_recv_mode(void)
{
	rtc_switch_mode(true, true);
}

static void rtc_switch_to_dcxo_recv_mode(void)
{
	rtc_switch_mode(false, true);
}

static bool rtc_frequency_meter_check(void)
{
	u16 result[4];

	if (rtc_get_recovery_mode_stat())
		rtc_switch_to_xosc_recv_mode();

	rtc_measure_four_clock(result);
	if (rtc_xosc_check_clock(result)) {
		rtc_xosc_write(0x0000);	/* crystal exist mode + XOSCCALI = 0 */
		return true;
	} else {
		if (!rtc_get_recovery_mode_stat())
			rtc_switch_to_dcxo_mode();
		else
			rtc_switch_to_dcxo_recv_mode();
	}

	rtc_measure_four_clock(result);

	if (rtc_eosc_check_clock(result)) {
		u16 val;

		val = eosc_cali();
		printk(BIOS_INFO, "[RTC] EOSC cali val = 0x%x\n", val);
		/* EMB_HW_Mode */
		val = (val & 0x001f) | 0x2400;
		rtc_xosc_write(val);
		return true;
	} else {
		return false;
	}
}

static bool rtc_init_after_recovery(void)
{
	if (!Writeif_unlock())
		return false;
	/* write powerkeys */
	RTC_Write(RTC_POWERKEY1, RTC_POWERKEY1_KEY);
	RTC_Write(RTC_POWERKEY2, RTC_POWERKEY2_KEY);
	if (!Write_trigger())
		return false;

	RTC_Write(CHRSTATUS, (RTC_Read(CHRSTATUS) & ~0x0800));

	if (!rtc_gpio_init())
		return false;
	if (!rtc_android_init())
		return false;
	if (!rtc_lpd_init())
		return false;

	return true;
}

static bool rtc_recovery_mode_check(void)
{
	if (!rtc_frequency_meter_check()) {
		rtc_call_exception();
		return false;
	}
	return true;
}

static void rtc_recovery_flow(void)
{
	u8 count = 0;
	printk(BIOS_INFO, "[RTC]%s\n", __func__);
	rtc_set_recovery_mode_stat(true);
	while (count < 3) {
		if (rtc_recovery_mode_check()) {
			if (rtc_init_after_recovery())
				break;
		}
		count++;
	}
	rtc_set_recovery_mode_stat(false);
	if (count == 3)
		rtc_call_exception();

}

static unsigned long rtc_mktime(struct rtc_time *tm)
{
	unsigned long d1, d2, d3;

	tm->mon -= 2;
	if (tm->mon <= 0) {
		tm->mon += 12;
		tm->year -= 1;
	}

	d1 = (tm->year - 1) * 365 + (tm->year / 4 - tm->year / 100 + tm->year / 400);
	d2 = (367 * tm->mon / 12 - 30) + 59;
	d3 = d1 + d2 + (tm->mday - 1) - 719162;

	return ((d3 * 24 + tm->hour) * 60 + tm->min) * 60 + tm->sec;
}

static bool rtc_first_boot_init(void)
{
	printk(BIOS_INFO, "[RTC]%s\n", __func__);

	if (!Writeif_unlock())
		return false;

	if (!rtc_gpio_init())
		return false;
	rtc_switch_to_xosc_mode();
	/* write powerkeys */
	RTC_Write(RTC_POWERKEY1, RTC_POWERKEY1_KEY);
	RTC_Write(RTC_POWERKEY2, RTC_POWERKEY2_KEY);
	if (!Write_trigger())
		return false;
	mdelay(1000);

	if (!rtc_frequency_meter_check())
		return false;
	if (!rtc_android_init())
		return false;
	if (!rtc_lpd_init())
		return false;

	return true;
}

static void rtc_bbpu_power_down(void)
{
	u16 bbpu;

	/* pull PWRBB low */
	bbpu = RTC_BBPU_KEY | RTC_BBPU_AUTO | RTC_BBPU_PWREN;
	Writeif_unlock();
	RTC_Write(RTC_BBPU, bbpu);
	Write_trigger();
}

static bool rtc_crystal_exist_status(void)
{
	/* RTC_SPAR0:
	   bit 6: 32K less bit. True:with 32K, False:Without 32K */
	u16 spar0;

	spar0 = RTC_Read(RTC_SPAR0);
	if (spar0 & RTC_SPAR0_32K_LESS)
		return true;
	else
		return false;
}

static void dcxo_setting_correct_frequency(void)
{
	u16 tmp = 0;

	tmp = RTC_Read(RG_DCXO_CON2);
	RTC_Write(RG_DCXO_CON2,
		  tmp & (~(PMIC_RG_DCXO_C2_UNTRIM_MASK <<
			   PMIC_RG_DCXO_C2_UNTRIM_SHIFT)));

	tmp = RTC_Read(RG_DCXO_MANUAL_CON1);
	RTC_Write(RG_DCXO_MANUAL_CON1,
		  tmp | (PMIC_RG_DCXO_MANUAL_SYNC_EN_MASK <<
			 PMIC_RG_DCXO_MANUAL_SYNC_EN_SHIFT));

	tmp = RTC_Read(RG_DCXO_MANUAL_CON1);
	RTC_Write(RG_DCXO_MANUAL_CON1,
		  tmp | (PMIC_RG_DCXO_MANUAL_C1C2_SYNC_EN_MASK <<
			 PMIC_RG_DCXO_MANUAL_C1C2_SYNC_EN_SHIFT));

	udelay(1);

	tmp = RTC_Read(RG_DCXO_MANUAL_CON1);
	RTC_Write(RG_DCXO_MANUAL_CON1,
		  tmp & (~(PMIC_RG_DCXO_MANUAL_SYNC_EN_MASK <<
			    PMIC_RG_DCXO_MANUAL_SYNC_EN_SHIFT)));

	tmp = RTC_Read(RG_DCXO_MANUAL_CON1);
	RTC_Write(RG_DCXO_MANUAL_CON1,
		  tmp & (~(PMIC_RG_DCXO_MANUAL_C1C2_SYNC_EN_MASK <<
			   PMIC_RG_DCXO_MANUAL_C1C2_SYNC_EN_SHIFT)));
}

void rtc_bbpu_power_on(void)
{
	u16 bbpu;
	bool ret;

	/* pull PWRBB high */
	bbpu = RTC_BBPU_KEY | RTC_BBPU_AUTO | RTC_BBPU_BBPU | RTC_BBPU_PWREN;
	RTC_Write(RTC_BBPU, bbpu);
	ret = Write_trigger();
	printk(BIOS_INFO, "[RTC] rtc_bbpu_power_on Write_trigger=%d\n", ret);

	if (rtc_crystal_exist_status()) {
		RTC_Write(0x854, RTC_Read(0x854) | 0x0807);
		RTC_Write(0x85c, RTC_Read(0x85c) | 0x0F07);
		printk(BIOS_INFO, "[RTC] EXT32k Mode \n");
	} else {
		printk(BIOS_INFO, "[RTC] INT32k Mode \n");
	}

	RTC_Write(0x83e, RTC_Read(0x83e) | 0x286A);

	printk(BIOS_INFO,
	       "[RTC] EXT32k Mode, DCXO REG Value 0x854=0x%x, 0x85c=0x%x, 0x83e=0x%x\n",
	       RTC_Read(0x854), RTC_Read(0x85c), RTC_Read(0x83e));

	printk(BIOS_INFO, "[RTC] rtc_bbpu_power_on done BBPU=0x%x\n",
	       RTC_Read(RTC_BBPU));

	dcxo_setting_correct_frequency();
	/* detect hw clock done,close RG_RTC_75K_PDN for low power golden setting. */
	RTC_Write(TOP_CKPDN2,RTC_Read(TOP_CKPDN2)|0x4000);
	if (1 == RTC_2SEC_REBOOT_ENABLE)
		rtc_enable_2sec_reboot();
	else
		RTC_Write(RTC_CALI, RTC_Read(RTC_CALI) & ~RTC_CALI_BBPU_2SEC_EN);

	printk(BIOS_INFO,"[RTC] rtc_bbpu_power_on done\n");
}

void rtc_mark_bypass_pwrkey(void)
{
	u16 pdn1;

	pdn1 = RTC_Read(RTC_PDN1) | 0x0040;
	RTC_Write(RTC_PDN1, pdn1);
	Write_trigger();
}

static void rtc_clean_mark(void)
{
	u16 pdn1;
	u16 pdn2;

	pdn1 = RTC_Read(RTC_PDN1) & ~0x8040;	/* also clear Debug bit */
	pdn2 = RTC_Read(RTC_PDN2) & ~0x0010;
	RTC_Write(RTC_PDN1, pdn1);
	RTC_Write(RTC_PDN2, pdn2);
	Write_trigger();
}

u16 rtc_rdwr_uart_bits(u16 * val)
{
	u16 pdn2;

	if (RTC_Read(RTC_CON) & RTC_CON_LPSTA_RAW)
		return 3;	/* UART bits are invalid due to RTC uninit */

	if (val) {
		pdn2 = RTC_Read(RTC_PDN2) & ~0x0060;
		pdn2 |= (*val & 0x0003) << 5;
		RTC_Write(RTC_PDN2, pdn2);
		Write_trigger();
	}

	return (RTC_Read(RTC_PDN2) & 0x0060) >> 5;
}

bool rtc_boot_check(void)
{
	u16 irqsta;
	u16 pdn1;
	u16 pdn2;
	u16 spar0;
	u16 spar1;
	bool check_mode_flag = false;

	RTC_Write(TOP_CKPDN, 0);
	RTC_Write(TOP_CKPDN2, 0);

	printk(BIOS_INFO,
	       "[RTC] bbpu = 0x%x, con = 0x%x, raw_bpu = 0x%x, raw_con = 0x%x\n",
	       RTC_Read(RTC_BBPU), RTC_Read(RTC_CON), RTC_Read(0xE000),
	       RTC_Read(0xE028));
	if ((RTC_Read(RTC_CON) & RTC_CON_LPSTA_RAW || check_mode_flag)) {
		if (!rtc_first_boot_init())
			rtc_recovery_flow();
	} else {
		if (!rtc_busy_wait())
			rtc_recovery_flow();
		if (!Writeif_unlock())
			rtc_recovery_flow();
	}

	if (RTC_Read(RTC_POWERKEY1) != RTC_POWERKEY1_KEY ||
	    RTC_Read(RTC_POWERKEY2) != RTC_POWERKEY2_KEY) {
		printk(BIOS_INFO, "[RTC] powerkey1 = 0x%x, powerkey2 = 0x%x\n",
		       RTC_Read(RTC_POWERKEY1), RTC_Read(RTC_POWERKEY2));
		if (!rtc_first_boot_init())
			rtc_recovery_flow();
	} else {
		RTC_Write(RTC_BBPU,
			  RTC_Read(RTC_BBPU) | RTC_BBPU_KEY | RTC_BBPU_RELOAD);
		Write_trigger();
		rtc_osc_init();
	}
	rtc_clean_mark();
	spar0 = RTC_Read(RTC_SPAR0);
	if (rtc_get_xosc_mode())
		RTC_Write(RTC_SPAR0, (spar0 | 0x0040));
	else
		RTC_Write(RTC_SPAR0, (spar0 & 0xffbf));
	Write_trigger();
	rtc_save_2sec_stat();

	irqsta = RTC_Read(RTC_IRQ_STA);	/* Read clear */
	pdn1 = RTC_Read(RTC_PDN1);
	pdn2 = RTC_Read(RTC_PDN2);
	spar0 = RTC_Read(RTC_SPAR0);
	spar1 = RTC_Read(RTC_SPAR1);
	printk(BIOS_INFO, "[RTC] irqsta = 0x%x, pdn1 = 0x%x, pdn2 = 0x%x, spar0 = 0x%x, spar1 = 0x%x\n",
	       irqsta, pdn1, pdn2, spar0, spar1);
	printk(BIOS_INFO,
	       "[RTC] new_spare0 = 0x%x, new_spare1 = 0x%x, new_spare2 = 0x%x, new_spare3 = 0x%x\n",
	       RTC_Read(RTC_AL_HOU), RTC_Read(RTC_AL_DOM), RTC_Read(RTC_AL_DOW),
	       RTC_Read(RTC_AL_MTH));
	printk(BIOS_INFO, "[RTC] bbpu = 0x%x, con = 0x%x\n", RTC_Read(RTC_BBPU),
	       RTC_Read(RTC_CON));

	if (irqsta & RTC_IRQ_STA_AL) {
		/* set AUTO bit because AUTO = 0 when PWREN = 1 and alarm occurs */
		u16 bbpu = RTC_Read(RTC_BBPU) | RTC_BBPU_KEY | RTC_BBPU_AUTO;
		RTC_Write(RTC_BBPU, bbpu);
		Write_trigger();

		if (pdn1 & 0x0080) {	/* power-on time is available */
			struct rtc_time now_tm, poweron_tm ;
			u16 irqen;
			unsigned long now_time, time;

			rtc_get(&now_tm);
			if (RTC_Read(RTC_TC_SEC) < now_tm.sec)
				rtc_get(&now_tm);

			poweron_tm.sec = spar0 & 0x003f;
			poweron_tm.min = spar1 & 0x003f;
			poweron_tm.hour = (spar1 & 0x07c0) >> 6;
			poweron_tm.mday = (spar1 & 0xf800) >> 11;
			poweron_tm.mon = pdn2 & 0x000f;
			poweron_tm.year = ((pdn2 & 0x7f00) >> 8) + RTC_MIN_YEAR;

			now_time = rtc_mktime(&now_tm);
			time = rtc_mktime(&poweron_tm);

			printk(BIOS_INFO,
			       "[RTC] now = %d/%d/%d %d:%d:%d (%lu)\n", now_tm.year,
			       now_tm.mon, now_tm.mday, now_tm.hour, now_tm.min,
				now_tm.sec, now_time);
			printk(BIOS_INFO,
			       "[RTC] power-on = %d/%d/%d %d:%d:%d (%lu)\n",
			       poweron_tm.year, poweron_tm.mon, poweron_tm.mday,
				poweron_tm.hour, poweron_tm.min, poweron_tm.sec, time);

			/* power on */
			if (now_time >= time - 1 && now_time <= time + 4) {
				pdn1 = (pdn1 & ~0x0080) | 0x0040;
				RTC_Write(RTC_PDN1, pdn1);
				RTC_Write(RTC_PDN2, pdn2 | 0x0010);
				Write_trigger();
				if (!(pdn2 & 0x8000))	/* no logo means ALARM_BOOT */
					return true;
			} else if (now_time < time) {	/* set power-on alarm */
				RTC_Write(RTC_AL_YEA, poweron_tm.year- RTC_MIN_YEAR);
				RTC_Write(RTC_AL_MTH,
					  (RTC_Read(RTC_AL_MTH) & 0xff00) |
					  poweron_tm.mon);
				RTC_Write(RTC_AL_DOM,
					  (RTC_Read(RTC_AL_DOM) & 0xff00) |
					  poweron_tm.mday);
				RTC_Write(RTC_AL_HOU,
					  (RTC_Read(RTC_AL_HOU) & 0xff00) |
					  poweron_tm.hour);
				RTC_Write(RTC_AL_MIN, poweron_tm.min);
				RTC_Write(RTC_AL_SEC, poweron_tm.sec);
				RTC_Write(RTC_AL_MASK, 0x0010);	/* mask DOW */
				Write_trigger();
				irqen = RTC_Read(RTC_IRQ_EN) |
					RTC_IRQ_EN_ONESHOT_AL;
				RTC_Write(RTC_IRQ_EN, irqen);
				Write_trigger();
			}
		}
	}

	if ((pdn1 & 0x0030) == 0x0010) {	/* factory data reset */
		/* keep bit 4 set until rtc_boot_check() in U-Boot */
		return true;
	}

	return false;
}

void pl_power_off(void)
{
	printk(BIOS_INFO, "[RTC]%s\n", __func__);

	rtc_bbpu_power_down();

	while (1) ;
}

