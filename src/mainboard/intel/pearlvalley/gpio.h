/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 Google Inc.
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

#ifndef MAINBOARD_GPIO_H
#define MAINBOARD_GPIO_H

#include <broadwell/gpio.h>

static const struct gpio_config mainboard_gpio_config[] = {
	PCH_GPIO_NATIVE,        /* 0: UART1_RXD_R */
	PCH_GPIO_NATIVE,        /* 1: UART1_TXD_R */
	PCH_GPIO_NATIVE,        /* 2: UART1_RST_N_R */
	PCH_GPIO_NATIVE,        /* 3: UART1_CTS_N_R */
	PCH_GPIO_NATIVE,        /* 4: I2C0_SDA_R */
	PCH_GPIO_NATIVE,        /* 5: I2C0_SCL */
	PCH_GPIO_NATIVE,        /* 6: I2C1_SDA */
	PCH_GPIO_NATIVE,        /* 7: I2C1_SCL */
	PCH_GPIO_INPUT,         /* 8: M.2_WLAN_WAKE_CTRL_N */
	PCH_GPIO_INPUT,         /* 9: M.2_BT_UART_WAKE_N */
	PCH_GPIO_INPUT,         /* 10: M.2_WWAN_RSVD_WAKE_N */
	PCH_GPIO_UNUSED,        /* 11: PCH_GPIO11 / BT_DEV_WAKE */
	PCH_GPIO_NATIVE,        /* 12: PM_LANPHY_ENABLE */
	PCH_GPIO_OUT_HIGH,      /* 13: LAN_PWREN_N */
	PCH_GPIO_IRQ_EDGE,      /* 14: SENSOR_HUB_INT_R_N */
	PCH_GPIO_OUT_HIGH,      /* 15: USB32_P0_PWREN_R */
	PCH_GPIO_OUT_LOW,       /* 16: LAN_RST_N */
	PCH_GPIO_OUT_LOW,       /* 17: CRIT_TEMP_REP_R_N */
	PCH_GPIO_NATIVE,        /* 18: CLK_REQ_P0_SLOT3_R_N */
	PCH_GPIO_NATIVE,        /* 19: CLK_REQ_P1 */
	PCH_GPIO_NATIVE,        /* 20: CLK_REQ_P2_M.2_WLAN_R_N */
	PCH_GPIO_NATIVE,        /* 21: CLK_REQ_P3_SLOT2_R_N */
	PCH_GPIO_NATIVE,        /* 22: CLK_REQ_P4_SLOT1_LAN_N */
	PCH_GPIO_NATIVE,        /* 23: CLK_REQ_P5_M.2_SSD_FLEX_R_N */
	PCH_GPIO_OUT_LOW,       /* 24: M.2_WWAN_DISABLE_N_ME_LED */
	PCH_GPIO_INPUT,         /* 25: USB_WAKEOUT_INTRUDET_N */
	PCH_GPIO_IRQ_EDGE,      /* 26: NFC_IRQ_MGP5 */
	PCH_GPIO_ACPI_SCI,      /* 27: SMC_WAKE_SCI_N */
	PCH_GPIO_OUT_LOW,       /* 28: PCH_NFC_RESET */
	PCH_GPIO_OUT_HIGH,      /* 29: PCH_SLP_WLAN_N */
	PCH_GPIO_NATIVE,        /* 30: SUS_PWR_ACK_R */
	PCH_GPIO_NATIVE,        /* 31: AC_PRESENT_R */
	PCH_GPIO_NATIVE,        /* 32: PM_CKRUN_N */
	PCH_GPIO_NATIVE,        /* 33: SATA3_PHYSLP */
	PCH_GPIO_INPUT,         /* 34: SATA_ESATA_ODD_PRSNT_R_N */
	PCH_GPIO_NATIVE,        /* 35: GP35_SATAPHYPC */
	PCH_GPIO_INPUT,         /* 36: M.2_SSD_SATA2_PCIE1_DET_N */
	PCH_GPIO_INPUT,         /* 37: M.2_SSD_SATA3_PCIE0_DET_N */
	PCH_GPIO_NATIVE,        /* 38: SATA1_PHYSLP_DIRECT */
	PCH_GPIO_ACPI_SMI,      /* 39: SMC_EXTSMI_R_N */
	PCH_GPIO_NATIVE,        /* 40: USB_OC_0_N */
	PCH_GPIO_NATIVE,        /* 41: USB_OC_1_3_N */
	PCH_GPIO_NATIVE,        /* 42: TP_USB_OC */
	PCH_GPIO_OUT_HIGH,      /* 43: USB32_P1_PWREN_R */
	PCH_GPIO_OUT_LOW,       /* 44: SH_DFU_R */
	PCH_GPIO_INPUT,         /* 45: M.2_WLAN_WIFI_WAKE_N */
	PCH_GPIO_OUT_HIGH,      /* 46: POWER_BUTTON */
	PCH_GPIO_IRQ_EDGE,      /* 47: SPI_TPM_HDR_IRQ_N */
	PCH_GPIO_OUT_LOW,       /* 48: CLK_REQ_PCIE_PWRGD_R */
	PCH_GPIO_INPUT,         /* 49: SENSOR_HUB_I2C_WAKE_R */
	PCH_GPIO_IRQ_EDGE,      /* 50: EC_HID_INTR */
	PCH_GPIO_INPUT,         /* 51: GPIO_51 */
	PCH_GPIO_ACPI_SCI,      /* 52: SMC_RUNTIME_SCI_R_N */
	PCH_GPIO_INPUT,         /* 53: GNSS_I2C_IRQ_WAKE */
	PCH_GPIO_INPUT,         /* 54: GP54_M.2_WWAN_UIM_SIM_DET */
	PCH_GPIO_INPUT,         /* 55: CARD_READER_DET_N */
	PCH_GPIO_UNUSED,        /* 56: TP_GPIO56 (NO CONNECTION) */
	PCH_GPIO_OUT_HIGH,      /* 57: M.2_WWAN_PWREN */
	PCH_GPIO_OUT_LOW,       /* 58: WIFI_RF_KILL_SH_PWREN */
	PCH_GPIO_OUT_LOW,       /* 59: PCH_BT_RF_KILL_N */
	PCH_GPIO_UNUSED,        /* 60: TP_GP60 */
	PCH_GPIO_NATIVE,        /* 61: PM_SUS_STAT_N */
	PCH_GPIO_NATIVE,        /* 62: SUS_CLK_PCH */
	PCH_GPIO_NATIVE,        /* 63: SLP_S5_R_N */
	PCH_GPIO_NATIVE,        /* 64: M.2_WLAN_SDIO_CLK_R */
	PCH_GPIO_NATIVE,        /* 65: M.2_WLAN_SDIO_CMD_R */
	PCH_GPIO_NATIVE,        /* 66: M.2_WLAN_SDIO_DAT0_R */
	PCH_GPIO_NATIVE,        /* 67: M.2_WLAN_SDIO_DAT1_R */
	PCH_GPIO_NATIVE,        /* 68: M.2_WLAN_SDIO_DAT2_R */
	PCH_GPIO_NATIVE,        /* 69: M.2_WLAN_SDIO_DAT3_R */
	PCH_GPIO_NATIVE,        /* 70: M.2_WLAN_WIFI_PWREN */
	PCH_GPIO_NATIVE,        /* 71: MPHY_PWREN */
	PCH_GPIO_NATIVE,        /* 72: PM_BATLOW_R_N */
	PCH_GPIO_NATIVE,        /* 73: PCH_NOT_N */
	PCH_GPIO_NATIVE,        /* 74: SML1_DATA */
	PCH_GPIO_NATIVE,        /* 75: SML1_CLK */
	PCH_GPIO_NATIVE,        /* 76: GP76_USB3MPHYPC */
	PCH_GPIO_OUT_LOW,       /* 77: M.2_WIFI_RST_N */
	PCH_GPIO_INPUT,         /* 78: EXTTS_SNI_DRV0_PCH */
	PCH_GPIO_IRQ_EDGE,      /* 79: TP_GPIO79 */
	PCH_GPIO_INPUT,         /* 80: EXTTS_SNI_DRV1_PCH */
	PCH_GPIO_NATIVE,        /* 81: PCH_HDA_SPKR */
	PCH_GPIO_NATIVE,        /* 82: H_RCIN_N */
	PCH_GPIO_NATIVE,        /* 83: GSPI0_CS_R_N */
	PCH_GPIO_NATIVE,        /* 84: GSPI0_CLK_R */
	PCH_GPIO_NATIVE,        /* 85: GSPI0_MISO_R */
	PCH_GPIO_NATIVE,        /* 86: GSPI0_MOSI_BBS0_R */
	PCH_GPIO_OUT_HIGH,      /* 87: GSPI1_CS_R_N / TOUCH_PNL_PWREN */
	PCH_GPIO_NATIVE,        /* 88: GSPI1_CLK_R */
	PCH_GPIO_OUT_LOW,       /* 89: GSPI1_MISO_R / TOUCH_PANEL_RST_N */
	PCH_GPIO_OUT_HIGH,      /* 90: GSPI1_MOSI_R / SATA1_PWR_EN */
	PCH_GPIO_NATIVE,        /* 91: UART0_RXD */
	PCH_GPIO_NATIVE,        /* 92: UART0_TXD */
	PCH_GPIO_NATIVE,        /* 93: UART0_RTS_N */
	PCH_GPIO_NATIVE,        /* 94: UART0_CTS_N */
	PCH_GPIO_END
};

#endif
