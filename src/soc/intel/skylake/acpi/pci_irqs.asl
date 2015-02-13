/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2009 coresystems GmbH
 * Copyright (C) 2014 Google Inc.
 * Copyright (C) 2015 Intel Corporation.
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

Method(_PRT)
{
	If (PICM) {
		Return (Package() {
		// PCI Bridge
		// D31: cAVS, SMBus, GbE, Nothpeak
			Package(){0x001FFFFF, 0, 0, 16 },
			Package(){0x001FFFFF, 1, 0, 17 },
			Package(){0x001FFFFF, 2, 0, 18 },
			Package(){0x001FFFFF, 3, 0, 19 },
		// D30: SerialIo and SCS
			Package(){0x001EFFFF, 0, 0, 20 },
			Package(){0x001EFFFF, 1, 0, 21 },
			Package(){0x001EFFFF, 2, 0, 22 },
			Package(){0x001EFFFF, 3, 0, 23 },
		// D29: PCI Express Port 9-16
			Package(){0x001DFFFF, 0, 0, 16 },
			Package(){0x001DFFFF, 1, 0, 17 },
			Package(){0x001DFFFF, 2, 0, 18 },
			Package(){0x001DFFFF, 3, 0, 19 },
		// D28: PCI Express Port 1-8
			Package(){0x001CFFFF, 0, 0, 16 },
			Package(){0x001CFFFF, 1, 0, 17 },
			Package(){0x001CFFFF, 2, 0, 18 },
			Package(){0x001CFFFF, 3, 0, 19 },
		// D27: PCI Express Port 17-20
			Package(){0x001BFFFF, 0, 0, 16 },
			Package(){0x001BFFFF, 1, 0, 17 },
			Package(){0x001BFFFF, 2, 0, 18 },
			Package(){0x001BFFFF, 3, 0, 19 },
		// D25: SerialIo
			Package(){0x0019FFFF, 0, 0, 32 },
			Package(){0x0019FFFF, 1, 0, 33 },
			Package(){0x0019FFFF, 2, 0, 34 },
		// D22: CSME (HECI, IDE-R, Keyboard and Text redirection
			Package(){0x0016FFFF, 0, 0, 16 },
			Package(){0x0016FFFF, 1, 0, 17 },
			Package(){0x0016FFFF, 2, 0, 18 },
			Package(){0x0016FFFF, 3, 0, 19 },
		// D21: SerialIo
			Package(){0x0015FFFF, 0, 0, 16 },
			Package(){0x0015FFFF, 1, 0, 17 },
			Package(){0x0015FFFF, 2, 0, 18 },
			Package(){0x0015FFFF, 3, 0, 19 },
		// D20: xHCI, OTG, Thermal Subsystem, Camera IO Host Controller
			Package(){0x0014FFFF, 0, 0, 16 },
			Package(){0x0014FFFF, 1, 0, 17 },
			Package(){0x0014FFFF, 2, 0, 18 },
			Package(){0x0014FFFF, 3, 0, 19 },
		// D19: Integrated Sensor Hub
			Package(){0x0013FFFF, 0, 0, 20 },

		// Host Bridge
		// P.E.G. Root Port D1F0
			Package(){0x0001FFFF, 0, 0, 16 },
			Package(){0x0001FFFF, 1, 0, 17 },
			Package(){0x0001FFFF, 2, 0, 18 },
			Package(){0x0001FFFF, 3, 0, 19 },
		// P.E.G. Root Port D1F1
		// P.E.G. Root Port D1F2
		// SA IGFX Device
			Package(){0x0002FFFF, 0, 0, 16 },
		// SA Thermal Device
			Package(){0x0004FFFF, 0, 0, 16 },
		// SA SkyCam Device
			Package(){0x0005FFFF, 0, 0, 16 },
		// SA GMM Device
			Package(){0x0008FFFF, 0, 0, 16 },
		})
	} Else {
		Return (Package() {
			// Onboard graphics (IGD)	0:2.0
			Package() { 0x0002ffff, 0, \_SB.PCI0.LPCB.LNKA, 0 },
			// PCIe Root Ports		0:1c.x
			Package() { 0x001cffff, 0, \_SB.PCI0.LPCB.LNKA, 0 },
			Package() { 0x001cffff, 1, \_SB.PCI0.LPCB.LNKB, 0 },
			Package() { 0x001cffff, 2, \_SB.PCI0.LPCB.LNKC, 0 },
			Package() { 0x001cffff, 3, \_SB.PCI0.LPCB.LNKD, 0 },
			// XHCI	  			0:14.0
			Package() { 0x0014ffff, 0, \_SB.PCI0.LPCB.LNKC, 0 },
			// LPC device			0:1f.0
			Package() { 0x001fffff, 0, \_SB.PCI0.LPCB.LNKG, 0 },
			Package() { 0x001fffff, 1, \_SB.PCI0.LPCB.LNKC, 0 },
			Package() { 0x001fffff, 2, \_SB.PCI0.LPCB.LNKB, 0 },
			Package() { 0x001fffff, 3, \_SB.PCI0.LPCB.LNKA, 0 },
			// Serial IO                    0:15.0
			Package() { 0x0015ffff, 0, \_SB.PCI0.LPCB.LNKE, 0 },
			Package() { 0x0015ffff, 1, \_SB.PCI0.LPCB.LNKF, 0 },
			Package() { 0x0015ffff, 2, \_SB.PCI0.LPCB.LNKF, 0 },
			Package() { 0x0015ffff, 3, \_SB.PCI0.LPCB.LNKF, 0 },
		})
	}
}

