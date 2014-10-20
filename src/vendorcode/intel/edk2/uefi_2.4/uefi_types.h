/**

Copyright (C) 2013, Intel Corporation

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.
* Neither the name of Intel Corporation nor the names of its contributors may
  be used to endorse or promote products derived from this software without
  specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
  THE POSSIBILITY OF SUCH DAMAGE.

**/

#ifndef __UEFI_TYPES_H__
#define __UEFI_TYPES_H__

//
// Set the UEFI types and attributes
//
#define __APPLE__ 0
#include <stdlib.h>
#include <Uefi/UefiBaseType.h>
#include <MdePkg/Include/Pi/PiBootMode.h>
#include <MdePkg/Include/Pi/PiFirmwareFile.h>
#include <MdePkg/Include/Pi/PiFirmwareVolume.h>
#include <MdePkg/Include/Uefi/UefiMultiPhase.h>
#include <MdePkg/Include/Pi/PiHob.h>
#include <MdePkg/Include/Library/HobLib.h>

///
/// For GNU assembly code, .global or .globl can declare global symbols.
/// Define this macro to unify the usage.
///
#if defined(ASM_GLOBAL)
#undef ASM_GLOBAL
#endif
#define ASM_GLOBAL .global

//
// Define the ASSERT support
//
static inline void debug_dead_loop(void)
{
	for (;;)
		;
}

#define _ASSERT(expression)  debug_dead_loop()
#define ASSERT(expression)			\
	do {					\
		if (!(expression)) {		\
			_ASSERT(expression);	\
		}				\
	} while (FALSE)

//
// Define the known GUIDs
//
#define FSP_CACHE_AS_RAM_DATA_HOB_GUID				\
{								\
	0xbbcff46c, 0xc8d3, 0x4113,				\
	{0x89, 0x85, 0xb9, 0xd4, 0xf3, 0xb3, 0xf6, 0x4e}	\
}

#define FSP_INFO_HEADER_GUID					\
{								\
	0x912740BE, 0x2284, 0x4734,				\
	{0xB9, 0x71, 0x84, 0xB0, 0x27, 0x35, 0x3F, 0x0C}	\
}

#define FSP_NON_VOLATILE_STORAGE_HOB_GUID			\
{								\
	0x721acf02, 0x4d77, 0x4c2a,				\
	{0xb3, 0xdc, 0x27, 0x0b, 0x7b, 0xa9, 0xe4, 0xb0}	\
}

#endif	/* __UEFI_TYPES_H__*/
