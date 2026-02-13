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

#ifndef HDRL_ITER_H
#define HDRL_ITER_H

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_types.h"
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

typedef struct _hdrl_iter_ hdrl_iter;
typedef enum {
    /* next returns a imagelist */
    HDRL_ITER_IMAGELIST = 1<<0,
    /* next returns a image */
    HDRL_ITER_IMAGE = 1<<1,
    /* next returns an initialized input buffer */
    HDRL_ITER_INPUT = 1<<2,
    /* next returns an uninitialized output buffer */
    HDRL_ITER_OUTPUT = 1<<3,
    /* iterator owns its next output and will deallocate it */
    HDRL_ITER_OWNS_DATA = 1<<4,
    /* iterating over const data, views make have NULL bpms */
    HDRL_ITER_CONST = 1<<5,
    /* multi-iterator may return empty results */
    HDRL_ITER_ALLOW_EMPTY = 1<<6,
} hdrl_iter_flags;

void * hdrl_iter_next(hdrl_iter * it);
cpl_size hdrl_iter_length(hdrl_iter * it);
void hdrl_iter_reset(hdrl_iter * it);
void hdrl_iter_delete(hdrl_iter * it);

/** @endcond */
#endif

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
------------------------------------------------------------------------------*/

#ifdef HDRL_USE_PRIVATE
/** @cond PRIVATE */

typedef void * (hdrl_iter_next_f)(hdrl_iter * it);
typedef void (hdrl_iter_reset_f)(hdrl_iter * it);
typedef cpl_size (hdrl_iter_length_f)(hdrl_iter * it);

hdrl_iter *
hdrl_iter_init(hdrl_iter_next_f * next,
                  hdrl_iter_reset_f * reset,
                  hdrl_iter_length_f * length,
                  hdrl_free * destructor,
                  hdrl_iter_flags flags,
                  void * state);

void * hdrl_iter_state(const hdrl_iter * it);

cpl_boolean hdrl_iter_check(hdrl_iter * it, hdrl_iter_flags flags);

/** @endcond */
#endif

CPL_END_DECLS

#endif
