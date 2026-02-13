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

#ifndef HDRL_IMAGELIST_BASIC_H
#define HDRL_IMAGELIST_BASIC_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_collapse.h"
#include "hdrl_parameter.h"
#include "hdrl_imagelist.h"

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/

/* Imagelist Basic Operations */
cpl_error_code hdrl_imagelist_add_imagelist(hdrl_imagelist *, 
        const hdrl_imagelist *);
cpl_error_code hdrl_imagelist_sub_imagelist(hdrl_imagelist *, 
        const hdrl_imagelist *);
cpl_error_code hdrl_imagelist_mul_imagelist(hdrl_imagelist *, 
        const hdrl_imagelist *);
cpl_error_code hdrl_imagelist_div_imagelist(hdrl_imagelist *, 
        const hdrl_imagelist *);

cpl_error_code hdrl_imagelist_add_image(hdrl_imagelist *, const hdrl_image *);
cpl_error_code hdrl_imagelist_sub_image(hdrl_imagelist *, const hdrl_image *);
cpl_error_code hdrl_imagelist_mul_image(hdrl_imagelist *, const hdrl_image *);
cpl_error_code hdrl_imagelist_div_image(hdrl_imagelist *, const hdrl_image *);

cpl_error_code hdrl_imagelist_add_scalar(hdrl_imagelist *, hdrl_value);
cpl_error_code hdrl_imagelist_sub_scalar(hdrl_imagelist *, hdrl_value);
cpl_error_code hdrl_imagelist_mul_scalar(hdrl_imagelist *, hdrl_value);
cpl_error_code hdrl_imagelist_div_scalar(hdrl_imagelist *, hdrl_value);
cpl_error_code hdrl_imagelist_pow_scalar(hdrl_imagelist *, hdrl_value);

/* Collapsing functions */
cpl_error_code hdrl_imagelist_collapse(
        const hdrl_imagelist    *   himlist,
        const hdrl_parameter    *   param,
        hdrl_image              **  out,
        cpl_image               **  contrib);
cpl_error_code hdrl_imagelist_collapse_mean(const hdrl_imagelist *, 
        hdrl_image **, cpl_image **) ;
cpl_error_code hdrl_imagelist_collapse_weighted_mean(const hdrl_imagelist *, 
        hdrl_image **, cpl_image **) ;
cpl_error_code hdrl_imagelist_collapse_median(const hdrl_imagelist *, 
        hdrl_image **, cpl_image **) ;
cpl_error_code hdrl_imagelist_collapse_sigclip(const hdrl_imagelist *, 
        double, double, int, hdrl_image **, cpl_image **,
        cpl_image **, cpl_image **) ;
cpl_error_code hdrl_imagelist_collapse_minmax(const hdrl_imagelist *,
        double, double, hdrl_image **, cpl_image **,
        cpl_image **, cpl_image **) ;
cpl_error_code hdrl_imagelist_collapse_mode(const hdrl_imagelist *, double,
					    double, double, hdrl_mode_type,
					    cpl_size, hdrl_image **,
					    cpl_image **) ;

CPL_END_DECLS

#endif 
