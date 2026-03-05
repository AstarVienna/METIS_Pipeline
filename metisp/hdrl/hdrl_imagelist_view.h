/*
 * This file is part of the HDRL
 * Copyright (C) 2014 European Southern Observatory
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

#ifndef HDRL_IMAGELIST_VIEW_H
#define HDRL_IMAGELIST_VIEW_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_imagelist.h"

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/
#if defined HDRL_USE_EXPERIMENTAL || defined HDRL_USE_PRIVATE

hdrl_imagelist * hdrl_imagelist_row_view(
    hdrl_imagelist * hl,
    cpl_size ly,
    cpl_size uy);

const hdrl_imagelist * hdrl_imagelist_const_row_view(
    const hdrl_imagelist * hl,
    cpl_size ly,
    cpl_size uy);

hdrl_imagelist * hdrl_imagelist_image_view(
    hdrl_imagelist * hl,
    cpl_size lz,
    cpl_size uz);

const hdrl_imagelist * hdrl_imagelist_const_cpl_row_view(
    const cpl_imagelist * imglist,
    const cpl_imagelist * errlist,
    cpl_size ly,
    cpl_size uy);

#endif

CPL_END_DECLS

#endif 
