/*
 * This file is part of the coreboot project.
 *
 * Copyright 2015 Google Inc.
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
 * Foundation, Inc.
 */

#ifndef _INTEL_COMMON_RAMSTAGE_H_
#define _INTEL_COMMON_RAMSTAGE_H_

#include <stdint.h>

/* Perform Intel silicon init. */
void intel_silicon_init(void);
/* Called after the silicon init code has run. */
void soc_after_silicon_init(void);

#endif /* _INTEL_COMMON_RAMSTAGE_H_ */
