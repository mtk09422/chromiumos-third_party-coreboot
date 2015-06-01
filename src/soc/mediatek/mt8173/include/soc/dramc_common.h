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

#ifndef _DRAMC_COMMON_H_
#define _DRAMC_COMMON_H_

#include <console/console.h>

/*
 * macros, defines, typedefs, enums, bit process
 */
static inline u32 mcBITL(u32 b)
{
	return (1L << (b));
};

static inline u32 mcBIT(u32 b)
{
	return mcBITL(b);
}

static inline u32 mcMASK(u32 w)
{
	return (mcBIT(w) - 1);
}

static inline u32 mcMASKS(u32 w, u32 b)
{
	return (mcMASK(w) << (b));
}

static inline void mcSET_MASK(u32 *a, u32 b)
{
	*a |= b;
}

static inline void mcCLR_MASK(u32 *a, u32 b)
{
	*a &= (~(b));
}

static inline void mcSET_BIT(u32 *a, u32 b)
{
	mcSET_MASK(a, mcBIT(b));
}

static inline void mcCLR_BIT(u32 *a, u32 b)
{
	mcCLR_MASK(a, mcBIT(b));
}

static inline u32 mcCHK_BIT1(u32 a, u32 b)
{
	return ((a) & mcBIT(b));
}

static inline u32 mcCHK_BITM(u32 a, u32 b, u32 m)
{
	return (((a) >> (b)) & (m));
}

static inline u32 mcCHK_BITS(u32 a, u32 b, u32 w)
{
	return mcCHK_BITM(a, b, mcMASK(w));
}

static inline u32 mcTEST_BIT(u32 a, u32 b)
{
	return mcCHK_BIT1(a, b);
}

static inline void mcCHG_BIT(u32 *a, u32 b)
{
	((*a) ^= mcBIT(b));
}

static inline void mcSET_FIELD0(u32 *var, u32 value, u32 mask, u32 pos)
{
	mcSET_MASK(var, ((value) << (pos)) & (mask));
}

static inline void mcSET_FIELD(u32 *var, u32 value, u32 mask, u32 pos)
{
	mcCLR_MASK(var, mask);
	mcSET_FIELD0(var, value, mask, pos);
}

static inline u32 mcGET_FIELD(u32 var, u32 mask, u32 pos)
{
	return (((var) & (mask)) >> (pos));
}

/* Log message */
#define mcSHOW_HW_MSG(_x_...)
#define mcSHOW_DBG_MSG(_x_...) printk(BIOS_DEBUG, _x_)
#define mcSHOW_DBG_MSG2(_x_...) printk(BIOS_DEBUG, _x_)
#define mcSHOW_DBG_MSG3(_x_...) if (RXPERBIT_LOG_PRINT) {printk(BIOS_DEBUG, _x_); }
#define mcSHOW_DBG_MSG4(_x_...) if (RXPERBIT_LOG_PRINT) {printk(BIOS_DEBUG, _x_); }
#define mcSHOW_DBG_MSG5(_x_...)
#define mcSHOW_USER_MSG(_x_...)
#define mcSHOW_DRVAPI_MSG(_x_...) /* printk(BIOS_DEBUG, _x_) */
#define mcSHOW_ERR_MSG(_x_...) /* printk(BIOS_DEBUG, _x_) */
#define mcFPRINTF(_x_...)

#endif   /* _DRAMC_COMMON_H_ */
