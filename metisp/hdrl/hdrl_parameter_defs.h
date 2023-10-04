/*
 * This file is part of the HDRL
 * Copyright (C) 2013 European Southern Observatory
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef HDRL_PARAMETER_DEFS_H
#define HDRL_PARAMETER_DEFS_H

#ifndef HDRL_USE_PRIVATE
#error This file is not allowed to be included outside of hdrl
#endif

#include "hdrl_parameter.h"

#include <stdint.h>

/** @cond PRIVATE */

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief base class of hdrl_parameter
 *
 * needs to be the first element of each child class structure, e.g:
 *   struct {
 *     HDRL_PARAMETER_HEAD;
 *     int val;
 *   } child_class;
 */
/* ---------------------------------------------------------------------------*/
struct _hdrl_parameter_  {
    const hdrl_parameter_typeobj * base;
};

/** @endcond */

#endif
