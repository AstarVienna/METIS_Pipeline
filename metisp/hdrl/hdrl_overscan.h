/*
 * This file is part of the HDRL
 * Copyright (C) 2012,2013 European Southern Observatory
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

#ifndef HDRL_OVERSCAN_H
#define HDRL_OVERSCAN_H

/*-----------------------------------------------------------------------------
                                Include
 -----------------------------------------------------------------------------*/

#include "hdrl_utils.h"
#include "hdrl_image.h"

#include <cpl.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/

#define HDRL_OVERSCAN_FULL_BOX -1
typedef struct _hdrl_overscan_compute_result_ hdrl_overscan_compute_result;
typedef struct _hdrl_overscan_correct_result_ hdrl_overscan_correct_result;

/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
                           Overscan Computation Parameters
  ----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_overscan_parameter_create(hdrl_direction, double, int, 
        hdrl_parameter *, hdrl_parameter *) ;
cpl_error_code hdrl_overscan_parameter_verify(const hdrl_parameter *,
        cpl_size, cpl_size) ;
cpl_boolean hdrl_overscan_parameter_check(const hdrl_parameter *) ;

/* Accessors */
hdrl_direction hdrl_overscan_parameter_get_correction_direction(
        const hdrl_parameter *) ;
double hdrl_overscan_parameter_get_ccd_ron(const hdrl_parameter *) ;
int hdrl_overscan_parameter_get_box_hsize(const hdrl_parameter *) ;
hdrl_parameter * hdrl_overscan_parameter_get_collapse(const hdrl_parameter *) ;
hdrl_parameter * hdrl_overscan_parameter_get_rect_region(const hdrl_parameter*);

/* Parameter Lists */
cpl_parameterlist * hdrl_overscan_parameter_create_parlist(const char *, 
        const char *, const char *, int, double, hdrl_parameter *,
	const char *, hdrl_parameter *, hdrl_parameter *, hdrl_parameter *) ;
hdrl_parameter * hdrl_overscan_parameter_parse_parlist(
        const cpl_parameterlist *, const char *) ;

/*----------------------------------------------------------------------------
                           Overscan Computation
  ----------------------------------------------------------------------------*/
hdrl_overscan_compute_result * hdrl_overscan_compute(
        const cpl_image         *   source,
        const hdrl_parameter    *   params) ;

/*----------------------------------------------------------------------------
                           Overscan Computation Result
  ----------------------------------------------------------------------------*/
hdrl_image * hdrl_overscan_compute_result_get_correction( 
        const hdrl_overscan_compute_result *);
hdrl_image * hdrl_overscan_compute_result_unset_correction( 
        hdrl_overscan_compute_result *); 

cpl_image * hdrl_overscan_compute_result_get_contribution(
        const hdrl_overscan_compute_result *);
cpl_image * hdrl_overscan_compute_result_unset_contribution(
        hdrl_overscan_compute_result *); 

cpl_image * hdrl_overscan_compute_result_get_chi2(
        const hdrl_overscan_compute_result *);
cpl_image * hdrl_overscan_compute_result_unset_chi2(
        hdrl_overscan_compute_result *); 

cpl_image * hdrl_overscan_compute_result_get_red_chi2(
        const hdrl_overscan_compute_result *);
cpl_image * hdrl_overscan_compute_result_unset_red_chi2(
        hdrl_overscan_compute_result *); 

cpl_image * hdrl_overscan_compute_result_get_sigclip_reject_low(
        const hdrl_overscan_compute_result *);
cpl_image * hdrl_overscan_compute_result_unset_sigclip_reject_low(
        hdrl_overscan_compute_result *); 

cpl_image * hdrl_overscan_compute_result_get_sigclip_reject_high(
        const hdrl_overscan_compute_result *);
cpl_image * hdrl_overscan_compute_result_unset_sigclip_reject_high(
        hdrl_overscan_compute_result *); 

cpl_image * hdrl_overscan_compute_result_get_minmax_reject_low(
        const hdrl_overscan_compute_result *);
cpl_image * hdrl_overscan_compute_result_unset_minmax_reject_low(
        hdrl_overscan_compute_result *); 

cpl_image * hdrl_overscan_compute_result_get_minmax_reject_high(
        const hdrl_overscan_compute_result *);
cpl_image * hdrl_overscan_compute_result_unset_minmax_reject_high(
        hdrl_overscan_compute_result *); 

void hdrl_overscan_compute_result_delete(hdrl_overscan_compute_result *);

/*----------------------------------------------------------------------------
                           Overscan Correction
  ----------------------------------------------------------------------------*/
hdrl_overscan_correct_result * hdrl_overscan_correct(
        const hdrl_image                        *   source,
        const hdrl_parameter                    *   region,
        const hdrl_overscan_compute_result      *   os_computation) ;

/*----------------------------------------------------------------------------
                           Overscan Correction Result
  ----------------------------------------------------------------------------*/
hdrl_image * hdrl_overscan_correct_result_get_corrected(
        const hdrl_overscan_correct_result *);
hdrl_image * hdrl_overscan_correct_result_unset_corrected(
        hdrl_overscan_correct_result *);

cpl_image * hdrl_overscan_correct_result_get_badmask(
        const hdrl_overscan_correct_result *);
cpl_image * hdrl_overscan_correct_result_unset_badmask(
        hdrl_overscan_correct_result *);
void hdrl_overscan_correct_result_delete(hdrl_overscan_correct_result *);

CPL_END_DECLS

#endif
