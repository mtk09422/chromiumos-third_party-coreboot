#include <console/console.h>
#include <delay.h>
#include <arch/io.h>

#include <soc/mt8135.h>
#include <soc/pmic_wrap_init.h>
#include <soc/pmic_6397.h>

#if CONFIG_DEBUG_PMIC
#define DEBUG_PMIC(level, x...)		printk(level, x)
#else
#define DEBUG_PMIC(level, x...)
#endif

#define DUMP_INIT_REG	0

#define Vcore_HV (0x38 + 0x11)
#define Vcore_NV (0x38 + 0x00)
#define Vcore_LV (0x38 - 0x11)
#define Vmem_HV (0x56 + 0x09)
#define Vmem_NV (0x56 + 0x00)
#define Vmem_LV (0x56 - 0x09)

#define writew(v, a)	write16(v, a)
#define readw(a)	read16(a)

#define OUTREG16(x, y)              write16((uint16_t)(y), (void *)(x))
#define INREG16(x)                  read16((void *)(x))
#define SETREG16(x, y)              setbits_le16((void *)(x), y)
#define CLRREG16(x, y)              clrbits_le16((void *)(x), y)

/* flag to indicate ca15 related power is ready */
volatile int g_ca15_ready __attribute__ ((section(".data"))) = 0;

/*
 * PMIC access API
 */
static u32 pmic_read_interface(u32 RegNum, u32 *val, u32 MASK, u32 SHIFT)
{
	u32 return_value = 0;
	u32 pmic6397_reg = 0;
	u32 rdata;

	return_value = pwrap_wacs2(0, (RegNum), 0, &rdata);
	pmic6397_reg = rdata;
	if (return_value != 0) {
		DEBUG_PMIC(BIOS_INFO,
			   "[pmic_read_interface] Reg[%x]= pmic_wrap read data fail\n",
			   RegNum);
		return return_value;
	}
	DEBUG_PMIC(BIOS_INFO, "[pmic_read_interface] Reg[%x]=0x%x\n", RegNum,
		   pmic6397_reg);

	pmic6397_reg &= (MASK << SHIFT);
	*val = (pmic6397_reg >> SHIFT);
	DEBUG_PMIC(BIOS_INFO, "[pmic_read_interface] val=0x%x\n", *val);

	return return_value;
}

u32 pmic_config_interface(u32 RegNum, u32 val, u32 MASK, u32 SHIFT)
{
	u32 return_value = 0;
	u32 pmic6397_reg = 0;
	u32 rdata;

	return_value = pwrap_wacs2(0, (RegNum), 0, &rdata);
	pmic6397_reg = rdata;
	if (return_value != 0) {
		DEBUG_PMIC(BIOS_INFO,
			   "[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n",
			   RegNum);
		return return_value;
	}
	DEBUG_PMIC(BIOS_INFO, "[pmic_config_interface] Reg[%x]=0x%x\n", RegNum,
		   pmic6397_reg);

	pmic6397_reg &= ~(MASK << SHIFT);
	pmic6397_reg |= (val << SHIFT);

	return_value = pwrap_wacs2(1, (RegNum), pmic6397_reg, &rdata);
	if (return_value != 0) {
		DEBUG_PMIC(BIOS_INFO,
			   "[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n",
			   RegNum);
		return return_value;
	}
	DEBUG_PMIC(BIOS_INFO, "[pmic_config_interface] write Reg[%x]=0x%x\n",
		   RegNum, pmic6397_reg);

	return return_value;
}

/*
 * PMIC-Charger Type Detection
 */
CHARGER_TYPE g_ret = PMIC_CHARGER_UNKNOWN;
int g_charger_in_flag = 0;
int g_first_check = 0;

static CHARGER_TYPE hw_charger_type_detection(void)
{
	CHARGER_TYPE ret = PMIC_CHARGER_UNKNOWN;

	unsigned int USB_U2PHYACR6_2 = 0x1122081A;
	unsigned int USBPHYRegs = 0x11220800;	/* U2B20_Base+0x800 */
	u16 bLineState_B = 0;
	u32 wChargerAvail = 0;
	u32 bLineState_C = 0;
	u32 ret_val = 0;
	u32 reg_val = 0;

	/********* Step 1.0 : PMU_BC11_Detect_Init ***************/
	SETREG16(USB_U2PHYACR6_2, 0x80);	/* bit 7 = 1 : switch to PMIC */

	/* BC11_RST=1 */
	ret_val = pmic_config_interface(CHR_CON18, 0x1, PMIC_RG_BC11_RST_MASK,
					PMIC_RG_BC11_RST_SHIFT);
	/* BC11_BB_CTRL=1 */
	ret_val = pmic_config_interface(CHR_CON18, 0x1,
					PMIC_RG_BC11_BB_CTRL_MASK,
					PMIC_RG_BC11_BB_CTRL_SHIFT);

	/* RG_BC11_BIAS_EN=1 */
	ret_val = pmic_config_interface(CHR_CON19, 0x1,
					PMIC_RG_BC11_BIAS_EN_MASK,
					PMIC_RG_BC11_BIAS_EN_SHIFT);
	/* RG_BC11_VSRC_EN[1:0]=00 */
	ret_val = pmic_config_interface(CHR_CON18, 0x0,
					PMIC_RG_BC11_VSRC_EN_MASK,
					PMIC_RG_BC11_VSRC_EN_SHIFT);
	/* RG_BC11_VREF_VTH = 0 */
	ret_val = pmic_config_interface(CHR_CON19, 0x0,
					PMIC_RG_BC11_VREF_VTH_MASK,
					PMIC_RG_BC11_VREF_VTH_SHIFT);
	/* RG_BC11_CMP_EN[1.0] = 00 */
	ret_val = pmic_config_interface(CHR_CON19, 0x0,
					PMIC_RG_BC11_CMP_EN_MASK,
					PMIC_RG_BC11_CMP_EN_SHIFT);
	/* RG_BC11_IPU_EN[1.0] = 00 */
	ret_val = pmic_config_interface(CHR_CON19, 0x0,
					PMIC_RG_BC11_IPU_EN_MASK,
					PMIC_RG_BC11_IPU_EN_SHIFT);
	/* RG_BC11_IPD_EN[1.0] = 00 */
	ret_val = pmic_config_interface(CHR_CON19, 0x0,
					PMIC_RG_BC11_IPD_EN_MASK,
					PMIC_RG_BC11_IPD_EN_SHIFT);

	/********* Step A *************************************/

	/* RG_BC11_IPU_EN[1.0] = 00 */
	ret_val = pmic_config_interface(CHR_CON19, 0x0,
					PMIC_RG_BC11_IPU_EN_MASK,
					PMIC_RG_BC11_IPU_EN_SHIFT);

	SETREG16(USBPHYRegs + 0x1C, 0x1000);	/* RG_PUPD_BIST_EN = 1 */
	CLRREG16(USBPHYRegs + 0x1C, 0x0400);	/* RG_EN_PD_DM=0 */

	/* RG_BC11_VSRC_EN[1.0] = 10 */
	ret_val = pmic_config_interface(CHR_CON18, 0x2,
					PMIC_RG_BC11_VSRC_EN_MASK,
					PMIC_RG_BC11_VSRC_EN_SHIFT);
	/* RG_BC11_IPD_EN[1:0] = 01 */
	ret_val = pmic_config_interface(CHR_CON19, 0x1,
					PMIC_RG_BC11_IPD_EN_MASK,
					PMIC_RG_BC11_IPD_EN_SHIFT);
	/* RG_BC11_VREF_VTH = 0 */
	ret_val = pmic_config_interface(CHR_CON19, 0x0,
					PMIC_RG_BC11_VREF_VTH_MASK,
					PMIC_RG_BC11_VREF_VTH_SHIFT);
	/* RG_BC11_CMP_EN[1.0] = 01 */
	ret_val = pmic_config_interface(CHR_CON19, 0x1,
					PMIC_RG_BC11_CMP_EN_MASK,
					PMIC_RG_BC11_CMP_EN_SHIFT);

	mdelay(100);

	ret_val = pmic_read_interface(CHR_CON18, &wChargerAvail,
				      PMIC_RGS_BC11_CMP_OUT_MASK,
				      PMIC_RGS_BC11_CMP_OUT_SHIFT);

	/* RG_BC11_VSRC_EN[1:0]=00 */
	ret_val = pmic_config_interface(CHR_CON18, 0x0,
					PMIC_RG_BC11_VSRC_EN_MASK,
					PMIC_RG_BC11_VSRC_EN_SHIFT);
	/* RG_BC11_IPD_EN[1.0] = 00 */
	ret_val = pmic_config_interface(CHR_CON19, 0x0,
					PMIC_RG_BC11_IPD_EN_MASK,
					PMIC_RG_BC11_IPD_EN_SHIFT);
	/* RG_BC11_CMP_EN[1.0] = 00 */
	ret_val = pmic_config_interface(CHR_CON19, 0x0,
					PMIC_RG_BC11_CMP_EN_MASK,
					PMIC_RG_BC11_CMP_EN_SHIFT);

	mdelay(50);

	if (wChargerAvail == 1) {
		/********* Step B *************************************/

		/* RG_BC11_IPU_EN[1:0]=10 */
		ret_val = pmic_config_interface(CHR_CON19, 0x2,
						PMIC_RG_BC11_IPU_EN_MASK,
						PMIC_RG_BC11_IPU_EN_SHIFT);

		mdelay(80);

		bLineState_B = INREG16(USBPHYRegs + 0x76);
		if (bLineState_B & 0x80) {
			ret = PMIC_STANDARD_CHARGER;
			DEBUG_PMIC(BIOS_INFO,
				   "[PL] mt_charger_type_detection : step B : STANDARD CHARGER!\r\n");
		} else {
			ret = PMIC_CHARGING_HOST;
			DEBUG_PMIC(BIOS_INFO,
				   "[PL] mt_charger_type_detection : step B : Charging Host!\r\n");
		}
	} else {
		/********* Step C *************************************/

		/* RG_BC11_IPU_EN[1:0]=01 */
		ret_val = pmic_config_interface(CHR_CON19, 0x1,
						PMIC_RG_BC11_IPU_EN_MASK,
						PMIC_RG_BC11_IPU_EN_SHIFT);
		/* RG_BC11_CMP_EN[1.0] = 01 */
		ret_val = pmic_config_interface(CHR_CON19, 0x1,
						PMIC_RG_BC11_CMP_EN_MASK,
						PMIC_RG_BC11_CMP_EN_SHIFT);

		mdelay(80);

		ret_val = pmic_read_interface(CHR_CON18, &bLineState_C,
					      0xFFFF, 0);
		if (bLineState_C & 0x0080) {
			ret = PMIC_NONSTANDARD_CHARGER;
			DEBUG_PMIC(BIOS_INFO,
				   "[PL] mt_charger_type_detection : step C : UNSTANDARD CHARGER!!!\r\n");

			/* RG_BC11_IPU_EN[1:0]=10 */
			ret_val = pmic_config_interface(CHR_CON19, 0x2,
							PMIC_RG_BC11_IPU_EN_MASK,
						PMIC_RG_BC11_IPU_EN_SHIFT);
			mdelay(80);
		} else {
			ret = PMIC_STANDARD_HOST;
			DEBUG_PMIC(BIOS_INFO,
				   "[PL] mt_charger_type_detection : step C : Standard USB Host!!\r\n");
		}
	}
	/********* Finally setting *******************************/

	/* RG_BC11_VSRC_EN[1:0]=00 */
	ret_val = pmic_config_interface(CHR_CON18, 0x0,
					PMIC_RG_BC11_VSRC_EN_MASK,
					PMIC_RG_BC11_VSRC_EN_SHIFT);
	/* RG_BC11_VREF_VTH = 0 */
	ret_val = pmic_config_interface(CHR_CON19, 0x0,
					PMIC_RG_BC11_VREF_VTH_MASK,
					PMIC_RG_BC11_VREF_VTH_SHIFT);
	/* RG_BC11_CMP_EN[1.0] = 00 */
	ret_val = pmic_config_interface(CHR_CON19, 0x0,
					PMIC_RG_BC11_CMP_EN_MASK,
					PMIC_RG_BC11_CMP_EN_SHIFT);
	/* RG_BC11_IPU_EN[1.0] = 00 */
	ret_val = pmic_config_interface(CHR_CON19, 0x0,
					PMIC_RG_BC11_IPU_EN_MASK,
					PMIC_RG_BC11_IPU_EN_SHIFT);
	/* RG_BC11_IPD_EN[1.0] = 00 */
	ret_val = pmic_config_interface(CHR_CON19, 0x0,
					PMIC_RG_BC11_IPD_EN_MASK,
					PMIC_RG_BC11_IPD_EN_SHIFT);
	/* RG_BC11_BIAS_EN=0 */
	ret_val = pmic_config_interface(CHR_CON19, 0x0,
					PMIC_RG_BC11_BIAS_EN_MASK,
					PMIC_RG_BC11_BIAS_EN_SHIFT);

	CLRREG16(USB_U2PHYACR6_2, 0x80);	/* bit 7 = 0 : switch to USB */

	if ((ret == PMIC_STANDARD_HOST) || (ret == PMIC_CHARGING_HOST)) {
		DEBUG_PMIC(BIOS_INFO,
			   "[PL] mt_charger_type_detection : SW workaround for USB\r\n");
		/* RG_BC11_BB_CTRL=1 */
		ret_val = pmic_config_interface(CHR_CON18, 0x1,
						PMIC_RG_BC11_BB_CTRL_MASK,
						PMIC_RG_BC11_BB_CTRL_SHIFT);
		/* RG_BC11_BIAS_EN=1 */
		ret_val = pmic_config_interface(CHR_CON19, 0x1,
						PMIC_RG_BC11_BIAS_EN_MASK,
						PMIC_RG_BC11_BIAS_EN_SHIFT);
		/* RG_BC11_VSRC_EN[1.0] = 11 */
		ret_val = pmic_config_interface(CHR_CON18, 0x3,
						PMIC_RG_BC11_VSRC_EN_MASK,
						PMIC_RG_BC11_VSRC_EN_SHIFT);
		/* check */
		ret_val = pmic_read_interface(CHR_CON18, &reg_val, 0xFFFF, 0);
		DEBUG_PMIC(BIOS_INFO, "Reg[0x%x]=0x%x\n", CHR_CON18, reg_val);
		ret_val = pmic_read_interface(CHR_CON19, &reg_val, 0xFFFF, 0);
		DEBUG_PMIC(BIOS_INFO, "Reg[0x%x]=0x%x\n", CHR_CON19, reg_val);
	}

	/* step4:done, ret the type */
	return ret;
}

CHARGER_TYPE mt_charger_type_detection(void)
{
	if (g_first_check == 0) {
		g_first_check = 1;
		g_ret = hw_charger_type_detection();
	} else {
		DEBUG_PMIC(BIOS_INFO,
			   "[mt_charger_type_detection] Got data !!, %d, %d\r\n",
			g_charger_in_flag, g_first_check);
	}

	return g_ret;
}

/*
 * PMIC Usage APIs
 */
u32 pmic_IsUsbCableIn(void)
{
	u32 ret = 0;
	u32 val = 0;

	ret = pmic_read_interface((u32)(CHR_CON0),
				  (&val),
				  (u32)(PMIC_RGS_CHRDET_MASK),
				  (u32)(PMIC_RGS_CHRDET_SHIFT));

	if (val)
		return PMIC_CHRDET_EXIST;
	else
		return PMIC_CHRDET_NOT_EXIST;
}

int pmic_detect_powerkey(void)
{
	u32 ret = 0;
	u32 val = 0;

	ret = pmic_read_interface((u32)(CHRSTATUS),
				  (&val),
				  (u32)(PMIC_PWRKEY_DEB_MASK),
				  (u32)(PMIC_PWRKEY_DEB_SHIFT));
	if (val == 1) {
		DEBUG_PMIC(BIOS_INFO, "[pmic_detect_powerkey_PL] Release\n");
		return 0;
	} else {
		DEBUG_PMIC(BIOS_INFO, "[pmic_detect_powerkey_PL] Press\n");
		return 1;
	}
}

void hw_set_cc(int cc_val)
{
	u32 ret_val = 0;
	u32 reg_val = 0;
	u32 i = 0;
	u32 hw_charger_ov_flag = 0;

	DEBUG_PMIC(BIOS_INFO, "hw_set_cc: %d\r\n", cc_val);

	/* VCDT_HV_VTH, 7V */
	ret_val = pmic_config_interface(CHR_CON1, 0x0B,
					PMIC_RG_VCDT_HV_VTH_MASK,
					PMIC_RG_VCDT_HV_VTH_SHIFT);
	/* VCDT_HV_EN=1 */
	ret_val = pmic_config_interface(CHR_CON0, 0x01, PMIC_RG_VCDT_HV_EN_MASK,
					PMIC_RG_VCDT_HV_EN_SHIFT);
	/* CS_EN=1 */
	ret_val = pmic_config_interface(CHR_CON2, 0x01, PMIC_RG_CS_EN_MASK,
					PMIC_RG_CS_EN_SHIFT);
	/* CSDAC_MODE=1 */
	ret_val = pmic_config_interface(CHR_CON23, 0x01,
					PMIC_RG_CSDAC_MODE_MASK,
					PMIC_RG_CSDAC_MODE_SHIFT);

	ret_val = pmic_read_interface(CHR_CON0, &hw_charger_ov_flag,
				      PMIC_RGS_VCDT_HV_DET_MASK,
				      PMIC_RGS_VCDT_HV_DET_SHIFT);
	if (hw_charger_ov_flag == 1) {
		ret_val = pmic_config_interface(CHR_CON0, 0x00,
						PMIC_RG_CHR_EN_MASK,
						PMIC_RG_CHR_EN_SHIFT);
		DEBUG_PMIC(BIOS_INFO,
			   "[PreLoader_charger_ov] turn off charging\n");
		return;
	}
	/* CS_VTH */
	switch (cc_val) {
	case 1600:
		ret_val = pmic_config_interface(CHR_CON4, 0x00,
						PMIC_RG_CS_VTH_MASK,
						PMIC_RG_CS_VTH_SHIFT);
		break;
	case 1500:
		ret_val = pmic_config_interface(CHR_CON4, 0x01,
						PMIC_RG_CS_VTH_MASK,
						PMIC_RG_CS_VTH_SHIFT);
		break;
	case 1400:
		ret_val = pmic_config_interface(CHR_CON4, 0x02,
						PMIC_RG_CS_VTH_MASK,
						PMIC_RG_CS_VTH_SHIFT);
		break;
	case 1300:
		ret_val = pmic_config_interface(CHR_CON4, 0x03,
						PMIC_RG_CS_VTH_MASK,
						PMIC_RG_CS_VTH_SHIFT);
		break;
	case 1200:
		ret_val = pmic_config_interface(CHR_CON4, 0x04,
						PMIC_RG_CS_VTH_MASK,
						PMIC_RG_CS_VTH_SHIFT);
		break;
	case 1100:
		ret_val = pmic_config_interface(CHR_CON4, 0x05,
						PMIC_RG_CS_VTH_MASK,
						PMIC_RG_CS_VTH_SHIFT);
		break;
	case 1000:
		ret_val = pmic_config_interface(CHR_CON4, 0x06,
						PMIC_RG_CS_VTH_MASK,
						PMIC_RG_CS_VTH_SHIFT);
		break;
	case 900:
		ret_val = pmic_config_interface(CHR_CON4, 0x07,
						PMIC_RG_CS_VTH_MASK,
						PMIC_RG_CS_VTH_SHIFT);
		break;
	case 800:
		ret_val = pmic_config_interface(CHR_CON4, 0x08,
						PMIC_RG_CS_VTH_MASK,
						PMIC_RG_CS_VTH_SHIFT);
		break;
	case 700:
		ret_val = pmic_config_interface(CHR_CON4, 0x09,
						PMIC_RG_CS_VTH_MASK,
						PMIC_RG_CS_VTH_SHIFT);
		break;
	case 650:
		ret_val = pmic_config_interface(CHR_CON4, 0x0A,
						PMIC_RG_CS_VTH_MASK,
						PMIC_RG_CS_VTH_SHIFT);
		break;
	case 550:
		ret_val = pmic_config_interface(CHR_CON4, 0x0B,
						PMIC_RG_CS_VTH_MASK,
						PMIC_RG_CS_VTH_SHIFT);
		break;
	case 450:
		ret_val = pmic_config_interface(CHR_CON4, 0x0C,
						PMIC_RG_CS_VTH_MASK,
						PMIC_RG_CS_VTH_SHIFT);
		break;
	case 400:
		ret_val = pmic_config_interface(CHR_CON4, 0x0D,
						PMIC_RG_CS_VTH_MASK,
						PMIC_RG_CS_VTH_SHIFT);
		break;
	case 200:
		ret_val = pmic_config_interface(CHR_CON4, 0x0E,
						PMIC_RG_CS_VTH_MASK,
						PMIC_RG_CS_VTH_SHIFT);
		break;
	case 70:
		ret_val = pmic_config_interface(CHR_CON4, 0x0F,
						PMIC_RG_CS_VTH_MASK,
						PMIC_RG_CS_VTH_SHIFT);
		break;
	default:
		DEBUG_PMIC(BIOS_INFO, "hw_set_cc: argument invalid!!\r\n");
		break;
	}

	ret_val = pmic_config_interface(CHR_CON21, 0x04,
					PMIC_RG_CSDAC_DLY_MASK,
					PMIC_RG_CSDAC_DLY_SHIFT);
	ret_val = pmic_config_interface(CHR_CON21, 0x01, PMIC_RG_CSDAC_STP_MASK,
					PMIC_RG_CSDAC_STP_SHIFT);
	ret_val = pmic_config_interface(CHR_CON20, 0x01,
					PMIC_RG_CSDAC_STP_INC_MASK,
					PMIC_RG_CSDAC_STP_INC_SHIFT);
	ret_val = pmic_config_interface(CHR_CON20, 0x02,
					PMIC_RG_CSDAC_STP_DEC_MASK,
					PMIC_RG_CSDAC_STP_DEC_SHIFT);
	ret_val = pmic_config_interface(CHR_CON13, 0x00, PMIC_RG_CHRWDT_TD_MASK,
					PMIC_RG_CHRWDT_TD_SHIFT);
	ret_val = pmic_config_interface(CHR_CON15, 0x01,
					PMIC_RG_CHRWDT_INT_EN_MASK,
					PMIC_RG_CHRWDT_INT_EN_SHIFT);
	ret_val = pmic_config_interface(CHR_CON13, 0x01, PMIC_RG_CHRWDT_EN_MASK,
					PMIC_RG_CHRWDT_EN_SHIFT);
	ret_val = pmic_config_interface(CHR_CON15, 0x01,
					PMIC_RG_CHRWDT_FLAG_WR_MASK,
					PMIC_RG_CHRWDT_FLAG_WR_SHIFT);
	ret_val = pmic_config_interface(CHR_CON0, 0x01, PMIC_RG_CSDAC_EN_MASK,
					PMIC_RG_CSDAC_EN_SHIFT);
	ret_val = pmic_config_interface(CHR_CON23, 0x01, PMIC_RG_HWCV_EN_MASK,
					PMIC_RG_HWCV_EN_SHIFT);
	ret_val = pmic_config_interface(CHR_CON0, 0x01, PMIC_RG_CHR_EN_MASK,
					PMIC_RG_CHR_EN_SHIFT);

	for (i = CHR_CON0; i <= CHR_CON29; i++) {
		ret_val = pmic_read_interface(i, &reg_val, 0xFFFF, 0x0);
		DEBUG_PMIC(BIOS_INFO, "[PreLoader] Bank0[0x%x]=0x%x\n", i,
			   reg_val);
	}

	DEBUG_PMIC(BIOS_INFO, "hw_set_cc: done\r\n");
}

void pl_hw_ulc_det(void)
{
	u32 ret_val = 0;

	ret_val = pmic_config_interface(CHR_CON23, 0x01,
					PMIC_RG_ULC_DET_EN_MASK,
					PMIC_RG_ULC_DET_EN_SHIFT);
	ret_val = pmic_config_interface(CHR_CON22, 0x01,
					PMIC_RG_LOW_ICH_DB_MASK,
					PMIC_RG_LOW_ICH_DB_SHIFT);
}

int hw_check_battery(void)
{
	u32 ret_val = 0;
	u32 reg_val = 0;

	/* BATON_EN=1 */
	ret_val = pmic_config_interface(CHR_CON7, 0x01, PMIC_RG_BATON_EN_MASK,
					PMIC_RG_BATON_EN_SHIFT);
	/* BATON_TDET_EN=0 */
	ret_val = pmic_config_interface(CHR_CON7, 0x00, PMIC_BATON_TDET_EN_MASK,
					PMIC_BATON_TDET_EN_SHIFT);
	/* RG_BUF_PWD_B=0 */
	ret_val = pmic_config_interface(AUXADC_CON0, 0x00,
					PMIC_RG_BUF_PWD_B_MASK,
					PMIC_RG_BUF_PWD_B_SHIFT);
	/* dump to check */
	ret_val = pmic_read_interface(CHR_CON7, &reg_val, 0xFFFF, 0x0);
	DEBUG_PMIC(BIOS_INFO, "[hw_check_battery+] [0x%x]=0x%x\n", CHR_CON7,
		   reg_val);
	ret_val = pmic_read_interface(AUXADC_CON0, &reg_val, 0xFFFF, 0x0);
	DEBUG_PMIC(BIOS_INFO, "[hw_check_battery+] [0x%x]=0x%x\n", AUXADC_CON0,
		   reg_val);

	ret_val = pmic_read_interface(CHR_CON7, &reg_val,
				      PMIC_RGS_BATON_UNDET_MASK,
				      PMIC_RGS_BATON_UNDET_SHIFT);

	if (reg_val == 1) {
		DEBUG_PMIC(BIOS_INFO, "[hw_check_battery] No Battery!!\n");

		/* dump to check */
		ret_val = pmic_read_interface(CHR_CON7, &reg_val, 0xFFFF, 0x0);
		DEBUG_PMIC(BIOS_INFO, "[hw_check_battery-] [0x%x]=0x%x\n",
			   CHR_CON7, reg_val);
		ret_val = pmic_read_interface(AUXADC_CON0, &reg_val, 0xFFFF,
					      0x0);
		DEBUG_PMIC(BIOS_INFO, "[hw_check_battery-] [0x%x]=0x%x\n",
			   AUXADC_CON0, reg_val);

		return 0;
	} else {
		DEBUG_PMIC(BIOS_INFO, "[hw_check_battery] Battery exist!!\n");

		/* dump to check */
		ret_val = pmic_read_interface(CHR_CON7, &reg_val, 0xFF, 0x0);
		DEBUG_PMIC(BIOS_INFO, "[hw_check_battery-] [0x%x]=0x%x\n",
			   CHR_CON7, reg_val);
		ret_val = pmic_read_interface(AUXADC_CON0, &reg_val, 0xFF, 0x0);
		DEBUG_PMIC(BIOS_INFO, "[hw_check_battery-] [0x%x]=0x%x\n",
			   AUXADC_CON0, reg_val);

		pl_hw_ulc_det();

		return 1;
	}
}

void pl_charging(int en_chr)
{
	u32 ret_val = 0;
	u32 reg_val = 0;
	u32 i = 0;

	if (en_chr == 1) {
		DEBUG_PMIC(BIOS_INFO, "[pl_charging] enable\n");

		hw_set_cc(450);

		/* USBDL set 1 */
		ret_val = pmic_config_interface(CHR_CON16, 0x01,
						PMIC_RG_USBDL_SET_MASK,
						PMIC_RG_USBDL_SET_SHIFT);
	} else {
		DEBUG_PMIC(BIOS_INFO, "[pl_charging] disable\n");

		/* USBDL set 0 */
		ret_val = pmic_config_interface(CHR_CON16, 0x00,
						PMIC_RG_USBDL_SET_MASK,
						PMIC_RG_USBDL_SET_SHIFT);

		/* HWCV_EN */
		ret_val = pmic_config_interface(CHR_CON23, 0x00,
						PMIC_RG_HWCV_EN_MASK,
						PMIC_RG_HWCV_EN_SHIFT);
		/* CHR_EN */
		ret_val = pmic_config_interface(CHR_CON0, 0x00,
						PMIC_RG_CHR_EN_MASK,
						PMIC_RG_CHR_EN_SHIFT);
	}

	for (i = CHR_CON0; i <= CHR_CON29; i++) {
		ret_val = pmic_read_interface(i, &reg_val, 0xFFFF, 0x0);
		DEBUG_PMIC(BIOS_INFO, "[pl_charging] Bank0[0x%x]=0x%x\n", i,
			   reg_val);
	}
}

void pl_kick_chr_wdt(void)
{
	int ret_val = 0;

	/* CHRWDT_TD */
	ret_val =
	    pmic_config_interface(CHR_CON13, 0x03, PMIC_RG_CHRWDT_TD_MASK,
				  PMIC_RG_CHRWDT_TD_SHIFT);
	/* CHRWDT_INT_EN */
	ret_val = pmic_config_interface(CHR_CON15, 0x01,
					PMIC_RG_CHRWDT_INT_EN_MASK,
					PMIC_RG_CHRWDT_INT_EN_SHIFT);
	/* CHRWDT_EN */
	ret_val = pmic_config_interface(CHR_CON13, 0x01,
					PMIC_RG_CHRWDT_EN_MASK,
					PMIC_RG_CHRWDT_EN_SHIFT);
	/* CHRWDT_FLAG */
	ret_val = pmic_config_interface(CHR_CON15, 0x01,
					PMIC_RG_CHRWDT_FLAG_WR_MASK,
					PMIC_RG_CHRWDT_FLAG_WR_SHIFT);
}

void pl_close_pre_chr_led(void)
{
	u32 ret_val = 0;

	ret_val = pmic_config_interface(CHR_CON22, 0x00, PMIC_RG_CHRIND_ON_MASK,
					PMIC_RG_CHRIND_ON_SHIFT);

	DEBUG_PMIC(BIOS_INFO, "[pmic6397_init] Close pre-chr LED\n");
}

/*
 * PMIC Init Code
 */
static void dump_pmic_init_regs(void)
{
#if DUMP_INIT_REG
	u32 val = 0;

	/* pmic_read_interface will print the value of the read register */
	pmic_read_interface(0x002, &val, 0xFFFF, 0);
	pmic_read_interface(0x00C, &val, 0xFFFF, 0);
	pmic_read_interface(0x01A, &val, 0xFFFF, 0);
	pmic_read_interface(0x024, &val, 0xFFFF, 0);
	pmic_read_interface(0x02A, &val, 0xFFFF, 0);
	pmic_read_interface(0x02E, &val, 0xFFFF, 0);
	pmic_read_interface(0x102, &val, 0xFFFF, 0);
	pmic_read_interface(0x128, &val, 0xFFFF, 0);
	pmic_read_interface(0x130, &val, 0xFFFF, 0);
	pmic_read_interface(0x134, &val, 0xFFFF, 0);
	pmic_read_interface(0x178, &val, 0xFFFF, 0);
	pmic_read_interface(0x17E, &val, 0xFFFF, 0);
	pmic_read_interface(0x206, &val, 0xFFFF, 0);
	pmic_read_interface(0x210, &val, 0xFFFF, 0);
	pmic_read_interface(0x216, &val, 0xFFFF, 0);
	pmic_read_interface(0x21E, &val, 0xFFFF, 0);
	pmic_read_interface(0x224, &val, 0xFFFF, 0);
	pmic_read_interface(0x238, &val, 0xFFFF, 0);
	pmic_read_interface(0x244, &val, 0xFFFF, 0);
	pmic_read_interface(0x24A, &val, 0xFFFF, 0);
	pmic_read_interface(0x25E, &val, 0xFFFF, 0);
	pmic_read_interface(0x260, &val, 0xFFFF, 0);
	pmic_read_interface(0x262, &val, 0xFFFF, 0);
	pmic_read_interface(0x264, &val, 0xFFFF, 0);
	pmic_read_interface(0x270, &val, 0xFFFF, 0);
	pmic_read_interface(0x276, &val, 0xFFFF, 0);
	pmic_read_interface(0x27C, &val, 0xFFFF, 0);
	pmic_read_interface(0x28A, &val, 0xFFFF, 0);
	pmic_read_interface(0x28E, &val, 0xFFFF, 0);
	pmic_read_interface(0x29C, &val, 0xFFFF, 0);
	pmic_read_interface(0x2B0, &val, 0xFFFF, 0);
	pmic_read_interface(0x330, &val, 0xFFFF, 0);
	pmic_read_interface(0x332, &val, 0xFFFF, 0);
	pmic_read_interface(0x336, &val, 0xFFFF, 0);
	pmic_read_interface(0x33C, &val, 0xFFFF, 0);
	pmic_read_interface(0x34A, &val, 0xFFFF, 0);
	pmic_read_interface(0x356, &val, 0xFFFF, 0);
	pmic_read_interface(0x358, &val, 0xFFFF, 0);
	pmic_read_interface(0x35C, &val, 0xFFFF, 0);
	pmic_read_interface(0x362, &val, 0xFFFF, 0);
	pmic_read_interface(0x370, &val, 0xFFFF, 0);
	pmic_read_interface(0x372, &val, 0xFFFF, 0);
	pmic_read_interface(0x374, &val, 0xFFFF, 0);
	pmic_read_interface(0x376, &val, 0xFFFF, 0);
	pmic_read_interface(0x39C, &val, 0xFFFF, 0);
	pmic_read_interface(0x440, &val, 0xFFFF, 0);
	pmic_read_interface(0x500, &val, 0xFFFF, 0);
	pmic_read_interface(0x502, &val, 0xFFFF, 0);
	pmic_read_interface(0x508, &val, 0xFFFF, 0);
	pmic_read_interface(0x50C, &val, 0xFFFF, 0);
	pmic_read_interface(0x512, &val, 0xFFFF, 0);
	pmic_read_interface(0x55E, &val, 0xFFFF, 0);
	pmic_read_interface(0x560, &val, 0xFFFF, 0);
	pmic_read_interface(0x566, &val, 0xFFFF, 0);
	pmic_read_interface(0x600, &val, 0xFFFF, 0);
	pmic_read_interface(0x604, &val, 0xFFFF, 0);
	pmic_read_interface(0x606, &val, 0xFFFF, 0);
	pmic_read_interface(0x60A, &val, 0xFFFF, 0);
	pmic_read_interface(0x612, &val, 0xFFFF, 0);
	pmic_read_interface(0x632, &val, 0xFFFF, 0);
	pmic_read_interface(0x638, &val, 0xFFFF, 0);
	pmic_read_interface(0x63A, &val, 0xFFFF, 0);
	pmic_read_interface(0x63C, &val, 0xFFFF, 0);
	pmic_read_interface(0x714, &val, 0xFFFF, 0);
	pmic_read_interface(0x71A, &val, 0xFFFF, 0);
#endif	/* DUMP_INIT_REG */
}

static void PMIC_INIT_SETTING_V1(void)
{
	u32 chip_version = 0;
	u32 ret = 0;

	ret = pmic_read_interface((u32)(CID), &chip_version,
				  (u32)(PMIC_CID_MASK),
				  (u32)(PMIC_CID_SHIFT));

	if (chip_version >= PMIC6397_E1_CID_CODE) {
		DEBUG_PMIC(BIOS_INFO,
			   "[pmic6397_init][%s] PMIC Chip = %x\n", __func__,
			   chip_version);

		/* pmic initialization settings */
		ret = pmic_config_interface(0x2, 0xB, 0xF, 4);
		ret = pmic_config_interface(0xC, 0x1, 0x7, 1);
		ret = pmic_config_interface(0x1A, 0x3, 0xF, 0);
		ret = pmic_config_interface(0x24, 0x1, 0x1, 1);
		ret = pmic_config_interface(0x2A, 0x0, 0x7, 4);
		ret = pmic_config_interface(0x2E, 0x1, 0x1, 7);
		ret = pmic_config_interface(0x2E, 0x1, 0x1, 6);
		ret = pmic_config_interface(0x2E, 0x1, 0x1, 2);
		ret = pmic_config_interface(0x102, 0x0, 0x1, 3);
		ret = pmic_config_interface(0x128, 0x1, 0x1, 9);
		ret = pmic_config_interface(0x128, 0x1, 0x1, 8);
		ret = pmic_config_interface(0x128, 0x1, 0x1, 6);
		ret = pmic_config_interface(0x128, 0x1, 0x1, 5);
		ret = pmic_config_interface(0x130, 0x1, 0x1, 7);
		ret = pmic_config_interface(0x130, 0x1, 0x1, 6);
		ret = pmic_config_interface(0x130, 0x1, 0x1, 5);
		ret = pmic_config_interface(0x130, 0x1, 0x1, 4);
		ret = pmic_config_interface(0x130, 0x1, 0x1, 3);
		ret = pmic_config_interface(0x130, 0x1, 0x1, 2);
		ret = pmic_config_interface(0x130, 0x1, 0x1, 1);
		ret = pmic_config_interface(0x130, 0x1, 0x1, 0);
		ret = pmic_config_interface(0x178, 0x1, 0x1, 11);
		ret = pmic_config_interface(0x178, 0x1, 0x1, 10);
		ret = pmic_config_interface(0x178, 0x1, 0x1, 4);
		ret = pmic_config_interface(0x17E, 0x1, 0x1, 11);
		ret = pmic_config_interface(0x17E, 0x1, 0x1, 8);
		ret = pmic_config_interface(0x206, 0x600, 0x1FFF, 0);
		ret = pmic_config_interface(0x210, 0x0, 0x3, 10);
		ret = pmic_config_interface(0x210, 0x0, 0x3, 6);
		ret = pmic_config_interface(0x210, 0x0, 0x3, 2);
		ret = pmic_config_interface(0x216, 0x0, 0x3, 12);
		ret = pmic_config_interface(0x216, 0x0, 0x3, 10);
		ret = pmic_config_interface(0x224, 0x1, 0x1, 15);
		ret = pmic_config_interface(0x224, 0x5, 0x7F, 8);
		ret = pmic_config_interface(0x224, 0x1, 0x1, 7);
		ret = pmic_config_interface(0x224, 0x17, 0x7F, 0);
		ret = pmic_config_interface(0x238, 0x1, 0x1, 8);
		ret = pmic_config_interface(0x238, 0x3, 0x3, 4);
		ret = pmic_config_interface(0x24A, 0x1, 0x1, 15);
		ret = pmic_config_interface(0x24A, 0x5, 0x7F, 8);
		ret = pmic_config_interface(0x24A, 0x1, 0x1, 7);
		ret = pmic_config_interface(0x24A, 0x17, 0x7F, 0);
		ret = pmic_config_interface(0x25E, 0x1, 0x1, 8);
		ret = pmic_config_interface(0x25E, 0x3, 0x3, 4);
		ret = pmic_config_interface(0x270, 0x1, 0x1, 1);
		ret = pmic_config_interface(0x276, 0x1, 0x1, 15);
		ret = pmic_config_interface(0x276, 0x5, 0x7F, 8);
		ret = pmic_config_interface(0x276, 0x17, 0x7F, 0);
		ret = pmic_config_interface(0x27C, 0x18, 0x7F, 0);
		ret = pmic_config_interface(0x28A, 0x1, 0x1, 8);
		ret = pmic_config_interface(0x28A, 0x0, 0x3, 4);
		ret = pmic_config_interface(0x28A, 0x3, 0x3, 0);
		ret = pmic_config_interface(0x28E, 0x1, 0x3, 8);
		ret = pmic_config_interface(0x29C, 0x1, 0x1, 15);
		ret = pmic_config_interface(0x29C, 0x5, 0x7F, 8);
		ret = pmic_config_interface(0x29C, 0x17, 0x7F, 0);
		ret = pmic_config_interface(0x2B0, 0x0, 0x3, 4);
		ret = pmic_config_interface(0x2B0, 0x3, 0x3, 0);
		ret = pmic_config_interface(0x332, 0x0, 0x3, 4);
		ret = pmic_config_interface(0x336, 0x1, 0x1, 15);
		ret = pmic_config_interface(0x336, 0x5, 0x7F, 8);
		ret = pmic_config_interface(0x336, 0x1, 0x1, 7);
		ret = pmic_config_interface(0x336, 0x17, 0x7F, 0);
		ret = pmic_config_interface(0x33C, 0x18, 0x7F, 0);
		ret = pmic_config_interface(0x34A, 0x1, 0x1, 8);
		ret = pmic_config_interface(0x34A, 0x3, 0x3, 4);
		ret = pmic_config_interface(0x356, 0x1, 0x1, 5);
		ret = pmic_config_interface(0x358, 0x0, 0x3, 4);
		ret = pmic_config_interface(0x35C, 0x1, 0x1, 15);
		ret = pmic_config_interface(0x35C, 0x5, 0x7F, 8);
		ret = pmic_config_interface(0x35C, 0x1, 0x1, 7);
		ret = pmic_config_interface(0x35C, 0x17, 0x7F, 0);
		ret = pmic_config_interface(0x362, 0x18, 0x7F, 0);
		ret = pmic_config_interface(0x370, 0x1, 0x1, 8);
		ret = pmic_config_interface(0x370, 0x3, 0x3, 4);
		ret = pmic_config_interface(0x39C, 0x1, 0x1, 8);
		ret = pmic_config_interface(0x440, 0x1, 0x1, 2);
		ret = pmic_config_interface(0x500, 0x1, 0x1, 5);
		ret = pmic_config_interface(0x502, 0x1, 0x1, 3);
		ret = pmic_config_interface(0x502, 0x1, 0x1, 2);
		ret = pmic_config_interface(0x508, 0x1, 0x1, 1);
		ret = pmic_config_interface(0x50C, 0x1, 0x1, 12);
		ret = pmic_config_interface(0x50C, 0x1, 0x1, 11);
		ret = pmic_config_interface(0x50C, 0x1, 0x1, 10);
		ret = pmic_config_interface(0x50C, 0x1, 0x1, 9);
		ret = pmic_config_interface(0x50C, 0x1, 0x1, 8);
		ret = pmic_config_interface(0x512, 0x1, 0x1, 1);
		ret = pmic_config_interface(0x512, 0x1, 0x1, 0);
		ret = pmic_config_interface(0x55E, 0xFC, 0xFF, 8);
		ret = pmic_config_interface(0x560, 0x1, 0x1, 1);
		ret = pmic_config_interface(0x566, 0x1, 0x1, 1);
		ret = pmic_config_interface(0x600, 0x1, 0x1, 9);
		ret = pmic_config_interface(0x604, 0x1, 0x1, 0);
		ret = pmic_config_interface(0x606, 0x1, 0x1, 9);
		ret = pmic_config_interface(0x60A, 0x1, 0xF, 11);
		ret = pmic_config_interface(0x612, 0x1, 0xF, 8);
		ret = pmic_config_interface(0x632, 0x1, 0x1, 8);
		ret = pmic_config_interface(0x638, 0xFFC2, 0xFFFF, 0);
		ret = pmic_config_interface(0x63A, 0x14, 0xFF, 0);
		ret = pmic_config_interface(0x63C, 0xFF, 0xFF, 8);
		ret = pmic_config_interface(0x714, 0x1, 0x1, 7);
		ret = pmic_config_interface(0x714, 0x1, 0x1, 4);
		ret = pmic_config_interface(0x714, 0x1, 0x1, 1);
		ret = pmic_config_interface(0x71A, 0x1, 0x1, 15);
		ret = pmic_config_interface(0x260, 0x4, 0x7F, 8);
		ret = pmic_config_interface(0x260, 0x0, 0x7F, 0);
		ret = pmic_config_interface(0x262, 0x5C, 0x7F, 8);
		ret = pmic_config_interface(0x262, 0x38, 0x7F, 0);
		ret = pmic_config_interface(0x264, 0x18, 0x7F, 0);
		ret = pmic_config_interface(0x372, 0x4, 0x7F, 8);
		ret = pmic_config_interface(0x372, 0x0, 0x7F, 0);
		ret = pmic_config_interface(0x374, 0x5C, 0x7F, 8);
		ret = pmic_config_interface(0x374, 0x38, 0x7F, 0);
		ret = pmic_config_interface(0x376, 0x18, 0x7F, 0);
		ret = pmic_config_interface(0x21E, 0x1, 0x1, 1);
		ret = pmic_config_interface(0x244, 0x1, 0x1, 1);
		ret = pmic_config_interface(0x330, 0x1, 0x1, 1);
		ret = pmic_config_interface(0x356, 0x1, 0x1, 1);
		ret = pmic_config_interface(0x21E, 0x1, 0x1, 4);
		ret = pmic_config_interface(0x244, 0x1, 0x1, 4);
		ret = pmic_config_interface(0x330, 0x1, 0x1, 4);
		ret = pmic_config_interface(0x356, 0x1, 0x1, 4);
		ret = pmic_config_interface(0x134, 0x3, 0x3, 14);
		ret = pmic_config_interface(0x134, 0x3, 0x3, 2);

		dump_pmic_init_regs();
	} else {
		DEBUG_PMIC(BIOS_INFO,
			   "[pmic6397_init][%s] Unknown PMIC Chip = %x\n", __func__,
		       chip_version);

	}
}

u32 pmic6397_init(void)
{
	u32 ret_code = PMIC_TEST_PASS;
	int ret_val = 0;
	u32 reg_val = 0;

	DEBUG_PMIC(BIOS_INFO, "[pmic6397_init] Start..................\n");

	/* Enable PMIC RST function (depends on main chip RST function) */
	ret_val = pmic_config_interface(TOP_RST_MISC, 0x1,
					PMIC_RG_SYSRSTB_EN_MASK,
					PMIC_RG_SYSRSTB_EN_SHIFT);
	ret_val = pmic_read_interface(TOP_RST_MISC, &reg_val, 0xFFFF, 0);
	DEBUG_PMIC(BIOS_INFO,
		   "[pmic6397_init] Enable PMIC RST function Reg[0x%x]=0x%x\n",
	       TOP_RST_MISC, reg_val);

	/* Enable CA15 by default for different PMIC behavior */
	pmic_config_interface(VCA15_CON7, 0x1, PMIC_VCA15_EN_MASK,
			      PMIC_VCA15_EN_SHIFT);
	pmic_config_interface(VSRMCA15_CON7, 0x1, PMIC_VSRMCA15_EN_MASK,
			      PMIC_VSRMCA15_EN_SHIFT);
	udelay(200);
	g_ca15_ready = 1;

	ret_val = pmic_read_interface(VCA15_CON7, &reg_val, 0xFFFF, 0);
	DEBUG_PMIC(BIOS_INFO, "Reg[0x%x]=0x%x\n", VCA15_CON7, reg_val);
	ret_val = pmic_read_interface(VSRMCA15_CON7, &reg_val, 0xFFFF, 0);
	DEBUG_PMIC(BIOS_INFO, "Reg[0x%x]=0x%x\n", VSRMCA15_CON7, reg_val);

	/* config vsrmca7, vsrmca15 voltage by DVFS PIC request */
	pmic_config_interface(VSRMCA7_CON9, 0x5D, PMIC_VSRMCA7_VOSEL_MASK,
			      PMIC_VSRMCA7_VOSEL_SHIFT); /* VSRMCA7 1.28125V */
	pmic_config_interface(VSRMCA7_CON10, 0x5D, PMIC_VSRMCA7_VOSEL_ON_MASK,
			      PMIC_VSRMCA7_VOSEL_ON_SHIFT);
	pmic_config_interface(VSRMCA15_CON9, 0x5D, PMIC_VSRMCA15_VOSEL_MASK,
			      PMIC_VSRMCA15_VOSEL_SHIFT); /* VSRMCA15 1.28125V */
	pmic_config_interface(VSRMCA15_CON10, 0x5D, PMIC_VSRMCA15_VOSEL_ON_MASK,
			      PMIC_VSRMCA15_VOSEL_ON_SHIFT);

	/* pmic initial setting */
	PMIC_INIT_SETTING_V1();
	DEBUG_PMIC(BIOS_INFO, "[PMIC_INIT_SETTING_V1] Done\n");

	/* 26M clock amplitute adjust */
	pmic_config_interface(RG_DCXO_ANALOG_CON1, 0x0,
			      PMIC_RG_DCXO_LDO_BB_V_MASK,
			      PMIC_RG_DCXO_LDO_BB_V_SHIFT);
	pmic_config_interface(RG_DCXO_ANALOG_CON1, 0x1,
			      PMIC_RG_DCXO_ATTEN_BB_MASK,
			      PMIC_RG_DCXO_ATTEN_BB_SHIFT);

	pmic_read_interface(EFUSE_DOUT_304_319, &reg_val, 0xFFFF, 0);
	if ((reg_val & 0x8000) == 0) {
		pmic_config_interface(BUCK_K_CON0, 0x0041, 0xFFFF, 0);
		pmic_config_interface(BUCK_K_CON0, 0x0040, 0xFFFF, 0);
		pmic_config_interface(BUCK_K_CON0, 0x0050, 0xFFFF, 0);
	}

	hw_check_battery();
	DEBUG_PMIC(BIOS_INFO, "[pmic6397_init] hw_check_battery\n");
#ifdef MEMPLL_CLK_793
	pmic_config_interface(VCORE_CON9, Vcore_HV, 0x7F, 0);
	pmic_config_interface(VCORE_CON10, Vcore_HV, 0x7F, 0);
#elif defined MEMPLL_CLK_733
	pmic_config_interface(VCORE_CON9, Vcore_NV + 0xA, 0x7F, 0);
	pmic_config_interface(VCORE_CON10, Vcore_NV + 0xA, 0x7F, 0);
#endif

	DEBUG_PMIC(BIOS_INFO, "[pmic6397_init] Done...................\n");

	return ret_code;
}
