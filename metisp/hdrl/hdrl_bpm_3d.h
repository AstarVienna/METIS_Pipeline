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

#ifndef HDRL_BPM_3D_H
#define HDRL_BPM_3D_H

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>
#include <stdint.h>
#include "hdrl_image.h"
#include "hdrl_imagelist.h"

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                                Defines
 -----------------------------------------------------------------------------*/

typedef enum {
    HDRL_BPM_3D_THRESHOLD_ABSOLUTE,
    HDRL_BPM_3D_THRESHOLD_RELATIVE,
    HDRL_BPM_3D_THRESHOLD_ERROR
} hdrl_bpm_3d_method ;

/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
                            BPM Parameters
  ----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_bpm_3d_parameter_create(double,double,hdrl_bpm_3d_method);
cpl_error_code hdrl_bpm_3d_parameter_verify(const hdrl_parameter *) ;
cpl_boolean hdrl_bpm_3d_parameter_check(const hdrl_parameter *) ;

/* Accessors */
double hdrl_bpm_3d_parameter_get_kappa_low(const hdrl_parameter *) ;
double hdrl_bpm_3d_parameter_get_kappa_high(const hdrl_parameter *) ;
hdrl_bpm_3d_method hdrl_bpm_3d_parameter_get_method(const hdrl_parameter *) ;

/* Parameter Lists */
cpl_parameterlist * hdrl_bpm_3d_parameter_create_parlist(const char *, 
        const char *, const hdrl_parameter *) ;
hdrl_parameter * hdrl_bpm_3d_parameter_parse_parlist(const cpl_parameterlist *, 
        const char *) ; 

/*----------------------------------------------------------------------------
                            BPM_3D Computation
  ----------------------------------------------------------------------------*/

cpl_imagelist * hdrl_bpm_3d_compute(
        const hdrl_imagelist    *   imglist,
        const hdrl_parameter    *   params) ;

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
 -----------------------------------------------------------------------------*/

#ifdef HDRL_USE_PRIVATE

#endif

CPL_END_DECLS

#endif
