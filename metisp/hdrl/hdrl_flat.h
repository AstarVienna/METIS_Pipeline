/* $Id: hdrl_flat.h,v 1.7 2013-10-17 15:44:14 jtaylor Exp $
 *
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

/*
 * $Author: jtaylor $
 * $Date: 2013-10-17 15:44:14 $
 * $Revision: 1.7 $
 * $Name: not supported by cvs2svn $
 */

#ifndef HDRL_FLAT_H
#define HDRL_FLAT_H

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_image.h"
#include "hdrl_imagelist.h"
#include <cpl.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/

typedef enum {
    HDRL_FLAT_FREQ_LOW,
    HDRL_FLAT_FREQ_HIGH
} hdrl_flat_method ;

/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
                            FLAT Parameters
  ----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_flat_parameter_create(cpl_size, cpl_size, hdrl_flat_method);
cpl_boolean hdrl_flat_parameter_check(const hdrl_parameter *) ;

/* Accessors */
cpl_size hdrl_flat_parameter_get_filter_size_x(const hdrl_parameter *) ;
cpl_size hdrl_flat_parameter_get_filter_size_y(const hdrl_parameter *) ;
hdrl_flat_method hdrl_flat_parameter_get_method(const hdrl_parameter *) ;

/* Parameter Lists */
cpl_parameterlist * hdrl_flat_parameter_create_parlist(const char *,
        const char *, const hdrl_parameter *) ;
hdrl_parameter * hdrl_flat_parameter_parse_parlist(const cpl_parameterlist *,
        const char *) ;

/*----------------------------------------------------------------------------
                            FLAT Computation
  ----------------------------------------------------------------------------*/

cpl_error_code hdrl_flat_compute(
                hdrl_imagelist                       *  hdrl_data,
                const cpl_mask                       *  stat_mask,
                const hdrl_parameter                 *  collapse_params,
                hdrl_parameter                       *  flat_params,
                hdrl_image                           ** master,
                cpl_image                            ** contrib_map);

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
 -----------------------------------------------------------------------------*/

#ifdef HDRL_USE_PRIVATE

#endif

CPL_END_DECLS

#endif

