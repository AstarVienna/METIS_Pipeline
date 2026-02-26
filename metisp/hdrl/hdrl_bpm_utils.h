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

#ifndef HDRL_BPM_UTILS_H
#define HDRL_BPM_UTILS_H

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>
#include <stdint.h>
#include "hdrl_image.h"
#include "hdrl_imagelist.h"

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

cpl_mask * hdrl_bpm_filter(const cpl_mask *, cpl_size, cpl_size, cpl_filter_mode) ;
cpl_imagelist * hdrl_bpm_filter_list(const cpl_imagelist *, cpl_size, cpl_size,
        cpl_filter_mode) ;

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
 -----------------------------------------------------------------------------*/

#ifdef HDRL_USE_PRIVATE
cpl_mask * hdrl_bpm_to_mask(const cpl_image *, uint64_t) ;
cpl_image * hdrl_mask_to_bpm(const cpl_mask *, uint64_t) ;
cpl_error_code hdrl_set_masks_on_imagelist(cpl_imagelist *, cpl_mask **) ;
cpl_error_code hdrl_join_mask_on_imagelist(cpl_imagelist *, cpl_mask *, 
        cpl_mask ***);
#endif

CPL_END_DECLS

#endif
