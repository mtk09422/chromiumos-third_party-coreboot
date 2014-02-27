/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2012 Advanced Micro Devices, Inc.
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

#include <device/pci_def.h>
#include <device/device.h>

/* warning: Porting.h includes an open #pragma pack(1) */
#include "Porting.h"
#include "AGESA.h"
#include "amdlib.h"
#include "dimmSpd.h"
#include "chip.h"

#if CONFIG_DDR3_SOLDERED_DOWN
#include CONFIG_PATH_TO_DDR3_SPD
AGESA_STATUS calc_fake_spd_crc( UINT8 *SPDPtr, UINT16 *crc );
#endif

#define DIMENSION(array)(sizeof (array)/ sizeof (array [0]))

#if !CONFIG_DDR3_SOLDERED_DOWN
/*-----------------------------------------------------------------------------
 *
 * readSmbusByteData - read a single SPD byte from any offset
 */

static int readSmbusByteData (int iobase, int address, char *buffer, int offset)
{
	unsigned int status;
	UINT64 limit;

	address |= 1; // set read bit

	__outbyte (iobase + 0, 0xFF);                // clear error status
	__outbyte (iobase + 1, 0x1F);                // clear error status
	__outbyte (iobase + 3, offset);              // offset in eeprom
	__outbyte (iobase + 4, address);             // slave address and read bit
	__outbyte (iobase + 2, 0x48);                // read byte command

	// time limit to avoid hanging for unexpected error status (should never happen)
	limit = __rdtsc () + 2000000000 / 10;
	for (;;)
	{
		status = __inbyte (iobase);
		if (__rdtsc () > limit) break;
		if ((status & 2) == 0) continue;               // SMBusInterrupt not set, keep waiting
		if ((status & 1) == 1) continue;               // HostBusy set, keep waiting
		break;
	}

	buffer [0] = __inbyte (iobase + 5);
	if (status == 2) status = 0;                      // check for done with no errors
	return status;
}

/*-----------------------------------------------------------------------------
 *
 * readSmbusByte - read a single SPD byte from the default offset
 *                 this function is faster function readSmbusByteData
 */

static int readSmbusByte (int iobase, int address, char *buffer)
{
	unsigned int status;
	UINT64 limit;

	__outbyte (iobase + 0, 0xFF);                // clear error status
	__outbyte (iobase + 2, 0x44);                // read command

	// time limit to avoid hanging for unexpected error status
	limit = __rdtsc () + 2000000000 / 10;
	for (;;)
	{
		status = __inbyte (iobase);
		if (__rdtsc () > limit) break;
		if ((status & 2) == 0) continue;               // SMBusInterrupt not set, keep waiting
		if ((status & 1) == 1) continue;               // HostBusy set, keep waiting
		break;
	}

	buffer [0] = __inbyte (iobase + 5);
	if (status == 2) status = 0;                      // check for done with no errors
	return status;
}

/*---------------------------------------------------------------------------
 *
 * readspd - Read one or more SPD bytes from a DIMM.
 *           Start with offset zero and read sequentially.
 *           Optimization relies on autoincrement to avoid
 *           sending offset for every byte.
 *          Reads 128 bytes in 7-8 ms at 400 KHz.
 */

static int readspd (int iobase, int SmbusSlaveAddress, char *buffer, int count)
{
	int index, error;

	/* read the first byte using offset zero */
	error = readSmbusByteData (iobase, SmbusSlaveAddress, buffer, 0);
	if (error) return error;

	/* read the remaining bytes using auto-increment for speed */
	for (index = 1; index < count; index++)
	{
		error = readSmbusByte (iobase, SmbusSlaveAddress, &buffer [index]);
		if (error) return error;
	}

	return 0;
}

static void writePmReg (int reg, int data)
{
	__outbyte (0xCD6, reg);
	__outbyte (0xCD7, data);
}

static void setupFch (int ioBase)
{
	writePmReg (0x2D, ioBase >> 8);
	writePmReg (0x2C, ioBase | 1);
	__outbyte (ioBase + 0x0E, 66000000 / 400000 / 4); // set SMBus clock to 400 KHz
}

AGESA_STATUS AmdMemoryReadSPD (UINT32 unused1, UINT32 unused2, AGESA_READ_SPD_PARAMS *info)
{
	int spdAddress, ioBase;
	ROMSTAGE_CONST struct device *dev = dev_find_slot(0, PCI_DEVFN(0x18, 2));
	ROMSTAGE_CONST struct northbridge_amd_agesa_family15tn_config *config = dev->chip_info;

	if ((dev == 0) || (config == 0))
		return AGESA_ERROR;

	if (info->SocketId     >= DIMENSION(config->spdAddrLookup      ))
		return AGESA_ERROR;
	if (info->MemChannelId >= DIMENSION(config->spdAddrLookup[0]   ))
		return AGESA_ERROR;
	if (info->DimmId       >= DIMENSION(config->spdAddrLookup[0][0]))
		return AGESA_ERROR;

	spdAddress = config->spdAddrLookup
		[info->SocketId] [info->MemChannelId] [info->DimmId];

	if (spdAddress == 0) return AGESA_ERROR;
	ioBase = 0xB00;
	setupFch (ioBase);
	return readspd (ioBase, spdAddress, (void *) info->Buffer, 128);
}

#else // CONFIG_DDR3_SOLDERED_DOWN
/*
 * Get the SPD from the mainboard
 */
AGESA_STATUS AmdMemoryReadSPD (UINT32 unused1, UINT32 unused2, AGESA_READ_SPD_PARAMS *info)
{
	UINT8 *spd_ptr;
	UINT16 index, crc;

	ROMSTAGE_CONST struct device *dev = dev_find_slot(0, PCI_DEVFN(0x18, 2));

	if ((dev == 0) || (dev->chip_info == 0))
		return AGESA_ERROR;

	if (info->MemChannelId > CONFIG_DDR3_CHANNEL_MAX) return AGESA_ERROR;
	if (info->SocketId     != 0)  return AGESA_ERROR;
	if (info->DimmId       != 0)  return AGESA_ERROR;

	/* read the bytes from the table */
	spd_ptr = (UINT8 *)info->Buffer;
	for (index = 0; index < 128; index++)
		spd_ptr[index] = ddr3_fake_spd[index];

	/* If CRC bytes are zeroes, calculate and store the CRC of the fake table */
	if ((spd_ptr[126] == 0) && (spd_ptr[127] == 0)) {
		calc_fake_spd_crc( spd_ptr, &crc );
		spd_ptr[126] = (UINT8)(crc & 0xFF);
		spd_ptr[127] = (UINT8)(crc>>8);
	}

	/* print out the table */
	printk(BIOS_SPEW, "\nDump the fake SPD for Channel %d\n",info->MemChannelId);
	for (index = 0; index < 128; index++) {
		if((index&0x0F)==0x00) printk(BIOS_SPEW, "%02x:  ",index);
		printk(BIOS_SPEW, "%02x ", spd_ptr[index]);
		if((index&0x0F)==0x0F) printk(BIOS_SPEW, "\n");
	}
	return AGESA_SUCCESS;
}

AGESA_STATUS calc_fake_spd_crc( UINT8 *SPDPtr, UINT16 *crc )
{
	INT16 i;
	INT16 j;
	INT16 jmax;

	/* should the CRC be done on bytes 0-116 or 0-125 ? */
	if (SPDPtr[0] & 0x80)
		 jmax = 117;
	else jmax = 126;

	*crc = 0; /* zero out the CRC */

	for (j = 0; j < jmax; j++) {
		*crc = *crc ^ ((UINT16)SPDPtr[j] << 8);
		for (i = 0; i < 8; i++) {
			if (*crc & 0x8000) {
				*crc = (*crc << 1) ^ 0x1021;
			} else {
				*crc = (*crc << 1);
			}
		}
	}
	return TRUE;
}
#endif
