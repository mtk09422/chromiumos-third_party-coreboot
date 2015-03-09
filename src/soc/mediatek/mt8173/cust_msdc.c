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

#include <soc/mt8173.h>
#include <soc/cust_msdc.h>

struct msdc_cust msdc_cap = {
    MSDC_CLKSRC_200MHZ, /* host clock source */
    MSDC_SMPL_RISING,   /* command latch edge */
    MSDC_SMPL_RISING,   /* data latch edge */
    2,                  /* clock pad driving */
    2,                  /* command pad driving */
    2,                  /* data pad driving */
    2,                  /* rst pad driving */
    2,                  /* ds pad driving */
    8,                  /* data pins */
    0,                  /* data address offset */
    MSDC_HIGHSPEED, /* hardware capability flags */
};
