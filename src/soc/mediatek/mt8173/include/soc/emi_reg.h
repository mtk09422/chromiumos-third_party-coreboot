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

#ifndef __EMI_H__
#define __EMI_H__

/* EMI PSRAM (NOR) and DRAM control registers */
enum {
	/* EMI control register for bank 0 */
	EMI_CONA   = ((P_U32)(EMI_BASE + 0x0000)),
	/* EMI control register for bank 1 */
	EMI_CONB   = ((P_U32)(EMI_BASE + 0x0008)),
	/* EMI control register for bank 2 */
	EMI_CONC   = ((P_U32)(EMI_BASE + 0x0010)),
	/* EMI control register for bank 3 */
	EMI_COND   = ((P_U32)(EMI_BASE + 0x0018)),
	/* EMI control register for bank 0 */
	EMI_CONE   = ((P_U32)(EMI_BASE + 0x0020)),
	/* EMI control register for bank 1 */
	EMI_CONF   = ((P_U32)(EMI_BASE + 0x0028)),
	/* EMI control register for bank 0 */
	EMI_CONG   = ((P_U32)(EMI_BASE + 0x0030)),
	/* EMI control register for bank 1 */
	EMI_CONH   = ((P_U32)(EMI_BASE + 0x0038)),
	/* EMI control register 0 for Mobile-RAM */
	EMI_CONI   = ((P_U32)(EMI_BASE + 0x0040)),
	/* EMI control register 1 for Mobile-RAM */
	EMI_CONJ   = ((P_U32)(EMI_BASE + 0x0048)),
	/* EMI control register 2 for Mobile-RAM */
	EMI_CONK   = ((P_U32)(EMI_BASE + 0x0050)),
	/* EMI control register 3 for Mobile-RAM */
	EMI_CONL   = ((P_U32)(EMI_BASE + 0x0058)),
	EMI_CONM   = ((P_U32)(EMI_BASE + 0x0060)),
	EMI_CONN   = ((P_U32)(EMI_BASE + 0x0068)),
	CAL_EN     = (1 << 8)
	EMI_GENA   = ((P_U32)(EMI_BASE + 0x0070)),
	EMI_REMAP  = EMI_GENA
	EMI_DRCT   = ((P_U32)(EMI_BASE + 0x0078)),
	EMI_DDRV   = ((P_U32)(EMI_BASE + 0x0080)),
	EMI_GEND   = ((P_U32)(EMI_BASE + 0x0088)),
	/* EMI Performance and Power Control Register */
	EMI_PPCT   = ((P_U32)(EMI_BASE + 0x0090)),

	EMI_DLLV   = ((P_U32)(EMI_BASE + 0x00A0)),

	EMI_DFTC   = ((P_U32)(EMI_BASE + 0x00F0)),
	EMI_DFTD   = ((P_U32)(EMI_BASE + 0x00F8))
};

/* EMI bandwidth filter and MPU control registers */
enum {
	EMI_ARBA   = ((P_U32)(EMI_BASE + 0x0100)),
	EMI_ARBB   = ((P_U32)(EMI_BASE + 0x0108)),
	EMI_ARBC   = ((P_U32)(EMI_BASE + 0x0110)),
	EMI_ARBD   = ((P_U32)(EMI_BASE + 0x0118)),
	EMI_ARBE   = ((P_U32)(EMI_BASE + 0x0120)),
	EMI_ARBF   = ((P_U32)(EMI_BASE + 0x0128)),
	EMI_ARBG   = ((P_U32)(EMI_BASE + 0x0130)),

	EMI_SLCT   = ((P_U32)(EMI_BASE + 0x0150)),
	EMI_ABCT   = ((P_U32)(EMI_BASE + 0x0158))
};

/* EMI Memory Protect Unit */
enum {
	EMI_MPUA   = ((P_U32)(EMI_BASE + 0x0160)),
	EMI_MPUB   = ((P_U32)(EMI_BASE + 0x0168)),
	EMI_MPUC   = ((P_U32)(EMI_BASE + 0x0170)),
	EMI_MPUD   = ((P_U32)(EMI_BASE + 0x0178)),
	EMI_MPUE   = ((P_U32)(EMI_BASE + 0x0180)),
	EMI_MPUF   = ((P_U32)(EMI_BASE + 0x0188)),
	EMI_MPUG   = ((P_U32)(EMI_BASE + 0x0190)),
	EMI_MPUH   = ((P_U32)(EMI_BASE + 0x0198)),

	EMI_MPUI   = ((P_U32)(EMI_BASE + 0x01A0)),
	EMI_MPUJ   = ((P_U32)(EMI_BASE + 0x01A8)),
	EMI_MPUK   = ((P_U32)(EMI_BASE + 0x01B0)),
	EMI_MPUL   = ((P_U32)(EMI_BASE + 0x01B8)),
	EMI_MPUM   = ((P_U32)(EMI_BASE + 0x01C0)),
	EMI_MPUN   = ((P_U32)(EMI_BASE + 0x01C8)),
	EMI_MPUO   = ((P_U32)(EMI_BASE + 0x01D0)),
	EMI_MPUP   = ((P_U32)(EMI_BASE + 0x01D8)),
	EMI_MPUQ   = ((P_U32)(EMI_BASE + 0x01E0)),
	EMI_MPUR   = ((P_U32)(EMI_BASE + 0x01E8)),
	EMI_MPUS   = ((P_U32)(EMI_BASE + 0x01F0)),
	EMI_MPUT   = ((P_U32)(EMI_BASE + 0x01F8))
};

/* EMI IO delay, driving and MISC control registers */
enum {
	EMI_IDLA   = ((P_U32)(EMI_BASE + 0x0200)),
	EMI_IDLB   = ((P_U32)(EMI_BASE + 0x0208)),
	EMI_IDLC   = ((P_U32)(EMI_BASE + 0x0210)),
	EMI_IDLD   = ((P_U32)(EMI_BASE + 0x0218)),
	EMI_IDLE   = ((P_U32)(EMI_BASE + 0x0220)),
	EMI_IDLF   = ((P_U32)(EMI_BASE + 0x0228)),
	EMI_IDLG   = ((P_U32)(EMI_BASE + 0x0230)),
	EMI_IDLH   = ((P_U32)(EMI_BASE + 0x0238)),
	/* IO input delay (DQS0 ~ DQS4) */
	EMI_IDLI   = ((P_U32)(EMI_BASE + 0x0240)),
	EMI_IDLJ   = ((P_U32)(EMI_BASE + 0x0248)),
	EMI_IDLK   = ((P_U32)(EMI_BASE + 0x0250)),

	EMI_ODLA   = ((P_U32)(EMI_BASE + 0x0258)),
	EMI_ODLB   = ((P_U32)(EMI_BASE + 0x0260)),
	EMI_ODLC   = ((P_U32)(EMI_BASE + 0x0268)),
	EMI_ODLD   = ((P_U32)(EMI_BASE + 0x0270)),
	EMI_ODLE   = ((P_U32)(EMI_BASE + 0x0278)),
	EMI_ODLF   = ((P_U32)(EMI_BASE + 0x0280)),
	EMI_ODLG   = ((P_U32)(EMI_BASE + 0x0288)),

	EMI_DUTA   = ((P_U32)(EMI_BASE + 0x0290)),
	EMI_DUTB   = ((P_U32)(EMI_BASE + 0x0298)),
	EMI_DUTC   = ((P_U32)(EMI_BASE + 0x02A0)),

	EMI_DRVA   = ((P_U32)(EMI_BASE + 0x02A8)),
	EMI_DRVB   = ((P_U32)(EMI_BASE + 0x02B0)),

	EMI_IOCL   = ((P_U32)(EMI_BASE + 0x02B8)),
	/* IvanTseng, for 4T mode */
	EMI_IOCM   = ((P_U32)(EMI_BASE + 0x02C0)),
	EMI_IODC   = ((P_U32)(EMI_BASE + 0x02C8)),

	EMI_ODTA   = ((P_U32)(EMI_BASE + 0x02D0)),
	EMI_ODTB   = ((P_U32)(EMI_BASE + 0x02D8))
};

/* EMI auto-tracking control registers */
enum {
	EMI_DQSA   = ((P_U32)(EMI_BASE + 0x0300)),
	EMI_DQSB   = ((P_U32)(EMI_BASE + 0x0308)),
	EMI_DQSC   = ((P_U32)(EMI_BASE + 0x0310)),
	EMI_DQSD   = ((P_U32)(EMI_BASE + 0x0318)),

	EMI_DQSE   = ((P_U32)(EMI_BASE + 0x0320)),
	EMI_DQSV   = ((P_U32)(EMI_BASE + 0x0328)),

	EMI_CALA   = ((P_U32)(EMI_BASE + 0x0330)),
	EMI_CALB   = ((P_U32)(EMI_BASE + 0x0338)),
	EMI_CALC   = ((P_U32)(EMI_BASE + 0x0340)),
	EMI_CALD   = ((P_U32)(EMI_BASE + 0x0348)),

	/* DDR data auto tracking control */
	EMI_CALE   = ((P_U32)(EMI_BASE + 0x0350)),
	EMI_CALF   = ((P_U32)(EMI_BASE + 0x0358)),
	/* DDR data auto tracking control */
	EMI_CALG   = ((P_U32)(EMI_BASE + 0x0360)),
	EMI_CALH   = ((P_U32)(EMI_BASE + 0x0368)),

	EMI_CALI   = ((P_U32)(EMI_BASE + 0x0370)),
	EMI_CALJ   = ((P_U32)(EMI_BASE + 0x0378)),
	EMI_CALK   = ((P_U32)(EMI_BASE + 0x0380)),
	EMI_CALL   = ((P_U32)(EMI_BASE + 0x0388)),

	EMI_CALM   = ((P_U32)(EMI_BASE + 0x0390)),
	EMI_CALN   = ((P_U32)(EMI_BASE + 0x0398)),

	EMI_CALO   = ((P_U32)(EMI_BASE + 0x03A0)),
	EMI_CALP   = ((P_U32)(EMI_BASE + 0x03A8)),

	EMI_DUCA   = ((P_U32)(EMI_BASE + 0x03B0)),
	EMI_DUCB   = ((P_U32)(EMI_BASE + 0x03B8)),
	EMI_DUCC   = ((P_U32)(EMI_BASE + 0x03C0)),
	EMI_DUCD   = ((P_U32)(EMI_BASE + 0x03C8)),
	EMI_DUCE   = ((P_U32)(EMI_BASE + 0x03D0))
};

/* EMI bus monitor control registers */
enum {
	EMI_BMEN     = ((P_U32)(EMI_BASE + 0x0400)),
	EMI_BCNT     = ((P_U32)(EMI_BASE + 0x0408)),
	EMI_TACT     = ((P_U32)(EMI_BASE + 0x0410)),
	EMI_TSCT     = ((P_U32)(EMI_BASE + 0x0418)),
	EMI_WACT     = ((P_U32)(EMI_BASE + 0x0420)),
	EMI_WSCT     = ((P_U32)(EMI_BASE + 0x0428)),
	EMI_BACT     = ((P_U32)(EMI_BASE + 0x0430)),
	EMI_BSCT     = ((P_U32)(EMI_BASE + 0x0438)),
	EMI_MSEL     = ((P_U32)(EMI_BASE + 0x0440)),
	EMI_TSCT2    = ((P_U32)(EMI_BASE + 0x0448)),
	EMI_TSCT3    = ((P_U32)(EMI_BASE + 0x0450)),
	EMI_WSCT2    = ((P_U32)(EMI_BASE + 0x0458)),
	EMI_WSCT3    = ((P_U32)(EMI_BASE + 0x0460)),
	EMI_MSEL2    = ((P_U32)(EMI_BASE + 0x0468)),
	EMI_MSEL3    = ((P_U32)(EMI_BASE + 0x0470)),
	EMI_MSEL4    = ((P_U32)(EMI_BASE + 0x0478)),
	EMI_MSEL5    = ((P_U32)(EMI_BASE + 0x0480)),
	EMI_MSEL6    = ((P_U32)(EMI_BASE + 0x0488)),
	EMI_MSEL7    = ((P_U32)(EMI_BASE + 0x0490)),
	EMI_MSEL8    = ((P_U32)(EMI_BASE + 0x0498)),
	EMI_MSEL9    = ((P_U32)(EMI_BASE + 0x04A0)),
	EMI_MSEL10   = ((P_U32)(EMI_BASE + 0x04A8)),
	EMI_BMID0    = ((P_U32)(EMI_BASE + 0x04B0)),
	EMI_BMID1    = ((P_U32)(EMI_BASE + 0x04B8)),
	EMI_BMID2    = ((P_U32)(EMI_BASE + 0x04C0)),
	EMI_BMID3    = ((P_U32)(EMI_BASE + 0x04C8)),
	EMI_BMID4    = ((P_U32)(EMI_BASE + 0x04D0)),
	EMI_BMID5    = ((P_U32)(EMI_BASE + 0x04D8)),

	EMI_TTYPE1     = ((P_U32)(EMI_BASE + 0x0500)),
	EMI_TTYPE2     = ((P_U32)(EMI_BASE + 0x0508)),
	EMI_TTYPE3     = ((P_U32)(EMI_BASE + 0x0510)),
	EMI_TTYPE4     = ((P_U32)(EMI_BASE + 0x0518)),
	EMI_TTYPE5     = ((P_U32)(EMI_BASE + 0x0520)),
	EMI_TTYPE6     = ((P_U32)(EMI_BASE + 0x0528)),
	EMI_TTYPE7     = ((P_U32)(EMI_BASE + 0x0530)),
	EMI_TTYPE8     = ((P_U32)(EMI_BASE + 0x0538)),
	EMI_TTYPE9     = ((P_U32)(EMI_BASE + 0x0540)),
	EMI_TTYPE10    = ((P_U32)(EMI_BASE + 0x0548)),
	EMI_TTYPE11    = ((P_U32)(EMI_BASE + 0x0550)),
	EMI_TTYPE12    = ((P_U32)(EMI_BASE + 0x0558)),
	EMI_TTYPE13    = ((P_U32)(EMI_BASE + 0x0560)),
	EMI_TTYPE14    = ((P_U32)(EMI_BASE + 0x0568)),
	EMI_TTYPE15    = ((P_U32)(EMI_BASE + 0x0570)),
	EMI_TTYPE16    = ((P_U32)(EMI_BASE + 0x0578)),
	EMI_TTYPE17    = ((P_U32)(EMI_BASE + 0x0580)),
	EMI_TTYPE18    = ((P_U32)(EMI_BASE + 0x0588)),
	EMI_TTYPE19    = ((P_U32)(EMI_BASE + 0x0590)),
	EMI_TTYPE20    = ((P_U32)(EMI_BASE + 0x0598)),
	EMI_TTYPE21    = ((P_U32)(EMI_BASE + 0x05A0))
};

/* EMI MBIST control registers*/
enum {
	EMI_MBISTA   = ((P_U32)(EMI_BASE + 0x0600)),
	EMI_MBISTB   = ((P_U32)(EMI_BASE + 0x0608)),
	EMI_MBISTC   = ((P_U32)(EMI_BASE + 0x0610)),
	EMI_MBISTD   = ((P_U32)(EMI_BASE + 0x0618)),
	/* EMI MBIST status register */
	EMI_MBISTE   = ((P_U32)(EMI_BASE + 0x0620))
};

/* EMI Flow control register A */
enum {
	EMI_RFCA    = ((P_U32)(EMI_BASE + 0x0630)),
	EMI_RFCB    = ((P_U32)(EMI_BASE + 0x0638)),
	EMI_RFCC    = ((P_U32)(EMI_BASE + 0x0640)),
	EMI_RFCD    = ((P_U32)(EMI_BASE + 0x0648))
};

#endif /* __EMI_H__ */
