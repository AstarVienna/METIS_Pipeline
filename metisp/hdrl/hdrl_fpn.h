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

#ifndef HDRL_FPN_H_
#define HDRL_FPN_H_

/*----------------------------------------------------------------------------*/
/**
 *                              Includes
 */
/*----------------------------------------------------------------------------*/

#include <cpl.h>

#include "hdrl_image.h"

CPL_BEGIN_DECLS

/*----------------------------------------------------------------------------*/
/**
 *                              Defines
 */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
 *                 Typedefs: Structs and enum types
 */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
 *                  Functions prototypes
 */
/*----------------------------------------------------------------------------*/

/* Fix the pattern noise in a input 2D image */
cpl_error_code hdrl_fpn_compute(
    cpl_image      *img_in,
    const cpl_mask *mask_in,
    const cpl_size dc_mask_x,
    const cpl_size dc_mask_y,
    cpl_image      **power_spectrum,
    double         *std,
    double         *std_mad);

CPL_END_DECLS


#endif  /* HDRL_FPN_H_ */
