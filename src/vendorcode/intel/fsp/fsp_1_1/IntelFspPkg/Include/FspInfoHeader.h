/** @file
  Intel FSP Info Header definition from Intel Firmware Support Package External
  Architecture Specification, April 2014, revision 001.

  Copyright (c) 2014, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _FSP_INFO_HEADER_H_
#define _FSP_INFO_HEADER_H_

#define FSP_HEADER_REVISION_1_0   1
#define FSP_HEADER_REVISION_1_1   2

///
/// Fixed FSP header offset in the FSP image
///
#define  FSP_INFO_HEADER_OFF    0x94

#define  OFFSET_IN_FSP_INFO_HEADER(x)  (UINT32)&((FSP_INFO_HEADER *)(UINTN)0)->x

#pragma pack(1)

typedef struct  {
  ///
  /// Byte 0: Signature ('FSPH') for the FSP Information Header
  ///
  UINT32  Signature;
  ///
  /// Byte 4: Length of the FSP Information Header
  ///
  UINT32  HeaderLength;
  ///
  /// Byte 8: Reserved
  ///
  UINT8   Reserved1[3];
  ///
  /// Byte 11: Revision of the FSP Information Header
  ///
  UINT8   HeaderRevision;
  ///
  /// Byte 12: Revision of the FSP binary
  ///
  UINT32  ImageRevision;


  ///
  /// Byte 16: Signature string that will help match the FSP Binary to a supported
  /// hardware configuration.
  ///
  CHAR8   ImageId[8];
  ///
  /// Byte 24: Size of the entire FSP binary
  ///
  UINT32  ImageSize;
  ///
  /// Byte 28: FSP binary preferred base address
  ///
  UINT32  ImageBase;


  ///
  /// Byte 32: Attribute for the FSP binary
  ///
  UINT32  ImageAttribute;
  ///
  /// Byte 36: Offset of the FSP configuration region
  ///
  UINT32  CfgRegionOffset;
  ///
  /// Byte 40: Size of the FSP configuration region
  ///
  UINT32  CfgRegionSize;
  ///
  /// Byte 44: Number of API entries this FSP supports
  ///
  UINT32  ApiEntryNum;

  ///
  /// Byte 48: The offset for the API to setup a temporary stack till the memory
  ///          is initialized.
  ///
  UINT32  TempRamInitEntryOffset;
  ///
  /// Byte 52: The offset for the API to initialize the CPU and the chipset (SOC)
  ///
  UINT32  FspInitEntryOffset;
  ///
  /// Byte 56: The offset for the API to inform the FSP about the different stages
  ///          in the boot process
  ///
  UINT32  NotifyPhaseEntryOffset;
  ///
  /// Reserved
  ///
  //UINT32  Reserved2;
  ///
  /// Byte 60: The offset for the API to initialize the memory
  ///
  UINT32  FspMemoryInitEntryOffset;
  ///
  /// Byte 64: The offset for the API to tear down temporary RAM
  ///
  UINT32  TempRamExitEntryOffset;
  ///
  /// Byte 68: The offset for the API to initialize the CPU and chipset
  ///
  UINT32  FspSiliconInitEntryOffset;
} FSP_INFO_HEADER;

typedef struct  {
  ///
  /// Byte 0: Signature ('FSPE') for the FSP Extended Information Header
  ///
  UINT32  Signature;
  ///
  /// Byte 4: Length of the FSP Extended Header
  ///
  UINT32  HeaderLength;
  ///
  /// Byte 8: Revision of the FSP Extended Header
  ///
  UINT8   Revision;
  ///
  /// Byte 9: Reserved for future use.
  ///
  UINT8   Reserved;
  ///
  /// Byte 10: An OEM-supplied string that defines the OEM
  ///
  CHAR8   OemId[6];
  ///
  /// Byte 16: An OEM-supplied revision number. Larger numbers are assumed to be newer revisions.
  ///
  UINT32  OemRevision;

} FSP_EXTENTED_HEADER;
#pragma pack()

#endif
