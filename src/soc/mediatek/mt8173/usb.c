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
#include <soc/usb.h>

#define USBTAG	"[SSUSB] "
#define u3p_msg(fmt, arg...)   printk(BIOS_INFO, USBTAG fmt, ##arg)
#define u3p_err(fmt, arg...)   printk(BIOS_ERR, USBTAG fmt, ##arg)

struct usb_mtk_u3phy {
	void *mac_base;	/* only device-mac regs, exclude xhci's */
	void *sif_base;	/* include sif & sif2 */
	void *ap_pll;
};

static struct usb_mtk_u3phy g_usb_mtk;

static inline void u3p_writel(void *base, u32 offset, u32 data)
{
	write32(base + offset, data);
}

static inline u32 u3p_readl(void *base, u32 offset)
{
	return read32(base + offset);
}

static inline void u3p_setmsk(void *base, u32 offset, u32 msk)
{
	void *addr = base + offset;

	write32(addr, (read32(addr) | msk));
}

static inline void u3p_clrmsk(void *base, u32 offset, u32 msk)
{
	void *addr = base + offset;

	write32(addr, (read32(addr) & ~msk));
}

static inline void u3p_setval(void *base, u32 offset, u32 mask, u32 value)
{
	void *addr = base + offset;
	unsigned int new_value;

	new_value = (read32(addr) & ~mask) | value;
	write32(addr, new_value);
}

static void phy_index_power_on(struct usb_mtk_u3phy *u3phy, int index)
{
	void *sif_base = u3phy->sif_base + U3P_PHY_BASE(index);

	if (!index) {
		/* Set RG_SSUSB_VUSB10_ON as 1 after VUSB10 ready */
		u3p_setmsk(sif_base, U3P_U3_PHYA_REG0, P3A_RG_U3_VUSB10_ON);
		/* power domain iso disable */
		u3p_clrmsk(sif_base, U3P_USBPHYACR6, PA6_RG_U2_ISO_EN);
	}
	/* switch to USB function. (system register, force ip into usb mode) */
	u3p_clrmsk(sif_base, U3P_U2PHYDTM0, P2C_FORCE_UART_EN);
	u3p_clrmsk(sif_base, U3P_U2PHYDTM1, P2C_RG_UART_EN);
	if (!index)
		u3p_clrmsk(sif_base, U3P_U2PHYACR4, P2C_U2_GPIO_CTR_MSK);

	/* (force_suspendm=0) (let suspendm=1, enable usb 480MHz pll) */
	u3p_clrmsk(sif_base, U3P_U2PHYDTM0, P2C_FORCE_SUSPENDM);
	u3p_clrmsk(sif_base, U3P_U2PHYDTM0,
		   P2C_RG_XCVRSEL | P2C_RG_DATAIN | P2C_DTM0_PART_MASK);

	/* DP/DM BC1.1 path Disable */
	u3p_clrmsk(sif_base, U3P_USBPHYACR6, PA6_RG_U2_BC11_SW_EN);
	/* OTG Enable */
	u3p_setmsk(sif_base, U3P_USBPHYACR6, PA6_RG_U2_OTG_VBUSCMP_EN);
	u3p_setval(sif_base, U3P_U3PHYA_DA_REG0, P3A_RG_XTAL_EXT_EN_U3,
		   P3A_RG_XTAL_EXT_EN_U3_VAL(2));
	u3p_setval(sif_base, U3P_U3_PHYA_REG9, P3A_RG_RX_DAC_MUX,
		   P3A_RG_RX_DAC_MUX_VAL(4));
	if (!index) {
		u3p_setmsk(sif_base, U3P_XTALCTL3, XC3_RG_U3_XTAL_RX_PWD);
		u3p_setmsk(sif_base, U3P_XTALCTL3, XC3_RG_U3_FRC_XTAL_RX_PWD);
		/* [mt8173]disable Change 100uA current from SSUSB */
		u3p_clrmsk(sif_base, U3P_USBPHYACR5, PA5_RG_U2_HS_100U_U3_EN);
	}
	u3p_setval(sif_base, U3P_U3_PHYA_REG6, P3A_RG_TX_EIDLE_CM,
		   P3A_RG_TX_EIDLE_CM_VAL(0xe));
	u3p_setval(sif_base, U3P_PHYD_CDR1, P3D_RG_CDR_BIR_LTD0,
		   P3D_RG_CDR_BIR_LTD0_VAL(0xc));
	u3p_setval(sif_base, U3P_PHYD_CDR1, P3D_RG_CDR_BIR_LTD1,
		   P3D_RG_CDR_BIR_LTD1_VAL(0x3));

	udelay(800);
	u3p_setmsk(sif_base, U3P_U2PHYDTM1, P2C_RG_VBUSVALID | P2C_RG_AVALID);
	u3p_clrmsk(sif_base, U3P_U2PHYDTM1, P2C_RG_SESSEND);

	/* USB 2.0 slew rate calibration */
	u3p_setval(sif_base, U3P_USBPHYACR5, PA5_RG_U2_HSTX_SRCTRL,
		   PA5_RG_U2_HSTX_SRCTRL_VAL(4));

	u3p_msg("%s(%d)\n", __func__, index);
}

static void u3phy_power_on(struct usb_mtk_u3phy *u3phy)
{
	phy_index_power_on(u3phy, 0);
	phy_index_power_on(u3phy, 1);
}

static int wait_for_value(void *base, int addr,
			  int msk, int value, int count)
{
	int i;

	for (i = 0; i < count; i++) {
		if ((u3p_readl(base, addr) & msk) == value)
			return 0;
		udelay(100);
	}

	return -1;
}

static int check_ip_clk_status(struct usb_mtk_u3phy *u3phy)
{
	int ret;
	int u3_port_num;
	int u2_port_num;
	u32 xhci_cap;
	void *sif_base = u3phy->sif_base;

	xhci_cap = u3p_readl(sif_base, U3P_IP_XHCI_CAP);
	u3_port_num = CAP_U3_PORT_NUM(xhci_cap);
	u2_port_num = CAP_U2_PORT_NUM(xhci_cap);

	ret = wait_for_value(sif_base, U3P_IP_PW_STS1, STS1_SYSPLL_STABLE,
			     STS1_SYSPLL_STABLE, 100);
	if (ret) {
		u3p_err("sypll is not stable!!!\n");
		goto err;
	}

	ret = wait_for_value(sif_base, U3P_IP_PW_STS1, STS1_REF_RST,
			     STS1_REF_RST, 100);
	if (ret) {
		u3p_err("ref_clk is still active!!!\n");
		goto err;
	}

	ret = wait_for_value(sif_base, U3P_IP_PW_STS1, STS1_SYS125_RST,
			     STS1_SYS125_RST, 100);
	if (ret) {
		u3p_err("sys125_ck is still active!!!\n");
		goto err;
	}

	if (u3_port_num) {
		ret = wait_for_value(sif_base, U3P_IP_PW_STS1, STS1_U3_MAC_RST,
				     STS1_U3_MAC_RST, 100);
		if (ret) {
			u3p_err("mac3_mac_ck is still active!!!\n");
			goto err;
		}
	}

	if (u2_port_num) {
		ret = wait_for_value(sif_base, U3P_IP_PW_STS2, STS2_U2_MAC_RST,
				     STS2_U2_MAC_RST, 100);
		if (ret) {
			u3p_err("mac2_sys_ck is still active!!!\n");
			goto err;
		}
	}
	return 0;

err:
	return -1;
}

static int u3phy_ports_enable(struct usb_mtk_u3phy *u3phy)
{
	int i;
	u32 temp;
	int u3_port_num;
	int u2_port_num;
	void *sif_base = u3phy->sif_base;

	temp = u3p_readl(sif_base, U3P_IP_XHCI_CAP);
	u3_port_num = CAP_U3_PORT_NUM(temp);
	u2_port_num = CAP_U2_PORT_NUM(temp);
	u3p_msg("%s u2p:%d, u3p:%d\n", __func__, u2_port_num, u3_port_num);

	/* power on host ip */
	u3p_clrmsk(sif_base, U3P_IP_PW_CTRL1, CTRL1_IP_HOST_PDN);

	/* power on and enable all u3 ports */
	for (i = 0; i < u3_port_num; i++) {
		temp = u3p_readl(sif_base, U3P_U3_CTRL(i));
		temp &= ~(CTRL_U3_PORT_PDN | CTRL_U3_PORT_DIS);
		temp |= CTRL_U3_PORT_HOST_SEL;
		u3p_writel(sif_base, U3P_U3_CTRL(i), temp);
	}

	/* power on and enable all u2 ports */
	for (i = 0; i < u2_port_num; i++) {
		temp = u3p_readl(sif_base, U3P_U2_CTRL(i));
		temp &= ~(CTRL_U2_PORT_PDN | CTRL_U2_PORT_DIS);
		temp |= CTRL_U2_PORT_HOST_SEL;
		u3p_writel(sif_base, U3P_U2_CTRL(i), temp);
	}
	return check_ip_clk_status(u3phy);
}

static inline void ssusb_soft_reset(struct usb_mtk_u3phy *u3phy)
{
	/* reset whole ip */
	u3p_setmsk(u3phy->sif_base, U3P_IP_PW_CTRL0, CTRL0_IP_SW_RST);
	u3p_clrmsk(u3phy->sif_base, U3P_IP_PW_CTRL0, CTRL0_IP_SW_RST);
}

static void ssusb_timing_init(struct usb_mtk_u3phy *u3phy)
{
	void *mbase = u3phy->mac_base;
	int u3_port_num;
	u32 temp;

	temp = u3p_readl(u3phy->sif_base, U3P_IP_XHCI_CAP);
	u3_port_num = CAP_U3_PORT_NUM(temp);

	if (u3_port_num) {
		/* set MAC reference clock speed */
		u3p_setval(mbase, U3P_UX_EXIT_LFPS_PARAM,
			   RX_UX_EXIT_REF, RX_UX_EXIT_REF_VAL);
		/* set REF_CLK */
		u3p_setval(mbase, U3P_REF_CLK_PARAM,
			   REF_CLK_1000NS, REF_CLK_VAL_DEF);
		/* set SYS_CLK */
		u3p_setval(mbase, U3P_TIMING_PULSE_CTRL,
			   U3T_CNT_1US, U3T_CNT_1US_VAL);
		/* set LINK_PM_TIMER=3 */
		u3p_setval(mbase, U3P_LINK_PM_TIMER,
			   PM_LC_TIMEOUT, PM_LC_TIMEOUT_VAL);
	}
	u3p_setval(mbase, U3P_U2_TIMING_PARAM, U2T_VAL_1US, U2T_VAL_1US_VAL);
}

/* Turn on/off ADA_SSUSB_XTAL_CK 26MHz */
static void u3_xtal_clock_enable(struct usb_mtk_u3phy *u3phy)
{
	u3p_setmsk(u3phy->ap_pll, AP_PLL_CON0, CON0_RG_LTECLKSQ_EN);
	udelay(100); /* wait for PLL stable */

	u3p_setmsk(u3phy->ap_pll, AP_PLL_CON0, CON0_RG_LTECLKSQ_LPF_EN);
	u3p_setmsk(u3phy->ap_pll, AP_PLL_CON2, CON2_DA_REF2USB_TX_EN);
	udelay(100); /* wait for PLL stable */

	u3p_setmsk(u3phy->ap_pll, AP_PLL_CON2, CON2_DA_REF2USB_TX_LPF_EN);
	u3p_setmsk(u3phy->ap_pll, AP_PLL_CON2, CON2_DA_REF2USB_TX_OUT_EN);
}

static inline void u3_xtal_clock_disable(struct usb_mtk_u3phy *u3phy)
{
	u3p_clrmsk(u3phy->ap_pll, AP_PLL_CON2, CON2_DA_REF2USB_TX_MASK);
}

void setup_usb_host(void)
{
	struct usb_mtk_u3phy *u3p = &g_usb_mtk;
	int ret;

	u3p_msg("Setting up USB HOST controller...\n");
	u3p->mac_base = (void *)SSUSB_MAC_BASE;
	u3p->sif_base = (void *)SSUSB_SIF_BASE;
	u3p->ap_pll = (void *)APMIXED_BASE;

	u3_xtal_clock_enable(u3p);
	ssusb_soft_reset(u3p);
	ret = u3phy_ports_enable(u3p);
	if (ret) {
		u3p_err("%s fail to enable ports\n", __func__);
		return;
	}
	ssusb_timing_init(u3p);
	u3phy_power_on(u3p);
	u3p_msg("phy power-on done.\n");
}
