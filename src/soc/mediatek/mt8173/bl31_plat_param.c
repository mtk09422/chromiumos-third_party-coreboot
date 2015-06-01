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
#include <arm_tf.h>

/* Special value used to verify platform parameters from BL2 to BL3-1.
 * The value must be same with the one defined in ARM TF mediatek platform
 * code.
 */
enum { MT_BL31_PLAT_PARAM_VAL = 0x0f1e2d3c4b5a6978ULL };

void *soc_get_bl31_plat_params(bl31_params_t *bl31_params)
{
	return (void *)MT_BL31_PLAT_PARAM_VAL;
}
