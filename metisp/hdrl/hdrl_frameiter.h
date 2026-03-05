/*
 * This file is part of the HDRL
 * Copyright (C) 2016 European Southern Observatory
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

#ifndef HDRL_FRAMEITER_H
#define HDRL_FRAMEITER_H

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_types.h"
#include "hdrl_iter.h"
#include <cpl.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Experimental declarations - can be used, but no guarantees on api stability
 -----------------------------------------------------------------------------*/
#if defined HDRL_USE_EXPERIMENTAL || defined HDRL_USE_PRIVATE
/** @cond EXPERIMENTAL */

typedef enum {
    HDRL_FRAMEITER_AXIS_FRAME = 0,
    HDRL_FRAMEITER_AXIS_EXT = 1,
    HDRL_FRAMEITER_AXIS_NAXIS1 = 2,
    HDRL_FRAMEITER_AXIS_NAXIS2 = 3,
    HDRL_FRAMEITER_AXIS_NAXIS3 = 4
} hdrl_frameiter_axis;

typedef struct {
    cpl_image * image;
    cpl_propertylist * plist;
} hdrl_frameiter_data;

hdrl_iter *
hdrl_frameiter_new(const cpl_frameset * frames, hdrl_iter_flags flags,
                   intptr_t naxes, intptr_t * axes, intptr_t * offsets,
                   intptr_t *strides, intptr_t * dims);

/** @endcond */
#endif

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
------------------------------------------------------------------------------*/

#ifdef HDRL_USE_PRIVATE
/** @cond PRIVATE */

/** @endcond */
#endif

CPL_END_DECLS

#endif
