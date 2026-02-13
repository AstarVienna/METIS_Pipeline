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

#ifndef HDRL_BPM_2D_H
#define HDRL_BPM_2D_H

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
    HDRL_BPM_2D_LEGENDRESMOOTH,
    HDRL_BPM_2D_FILTERSMOOTH
} hdrl_bpm_2d_method ;

/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
                            BPM_2D Parameters
  ----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_bpm_2d_parameter_create_legendresmooth(double, double, 
        int, int, int, int, int, int, int) ;
hdrl_parameter * hdrl_bpm_2d_parameter_create_filtersmooth(double, double, 
        int, cpl_filter_mode, cpl_border_mode, int, int) ;
cpl_error_code hdrl_bpm_2d_parameter_verify(const hdrl_parameter *) ;
cpl_boolean hdrl_bpm_2d_parameter_check(const hdrl_parameter *) ;

/* Accessors */
cpl_filter_mode hdrl_bpm_2d_parameter_get_filter(const hdrl_parameter *) ;
cpl_border_mode hdrl_bpm_2d_parameter_get_border(const hdrl_parameter *) ;
double hdrl_bpm_2d_parameter_get_kappa_low(const hdrl_parameter *) ;
double hdrl_bpm_2d_parameter_get_kappa_high(const hdrl_parameter *) ;
int hdrl_bpm_2d_parameter_get_maxiter(const hdrl_parameter *) ;
int hdrl_bpm_2d_parameter_get_steps_x(const hdrl_parameter *) ;
int hdrl_bpm_2d_parameter_get_steps_y(const hdrl_parameter *) ;
int hdrl_bpm_2d_parameter_get_filter_size_x(const hdrl_parameter *) ;
int hdrl_bpm_2d_parameter_get_filter_size_y(const hdrl_parameter *) ;
int hdrl_bpm_2d_parameter_get_order_x(const hdrl_parameter *) ;
int hdrl_bpm_2d_parameter_get_order_y(const hdrl_parameter *) ;
int hdrl_bpm_2d_parameter_get_smooth_x(const hdrl_parameter *) ;
int hdrl_bpm_2d_parameter_get_smooth_y(const hdrl_parameter *) ;
hdrl_bpm_2d_method hdrl_bpm_2d_parameter_get_method(const hdrl_parameter *) ;

/* Parameter Lists */
cpl_parameterlist * hdrl_bpm_2d_parameter_create_parlist(
        const char      *   base_context,
        const char      *   prefix,
        const char      *   method_def,
        const hdrl_parameter * filtersmooth_def,
        const hdrl_parameter * legendresmooth_def);
hdrl_parameter * hdrl_bpm_2d_parameter_parse_parlist(const cpl_parameterlist *, 
        const char *) ; 

/*----------------------------------------------------------------------------
                            BPM_2D Computation
  ----------------------------------------------------------------------------*/

cpl_mask * hdrl_bpm_2d_compute(
        const hdrl_image        *   img_in,
        const hdrl_parameter    *   params) ;

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
 -----------------------------------------------------------------------------*/

#ifdef HDRL_USE_PRIVATE

#endif

CPL_END_DECLS

#endif
