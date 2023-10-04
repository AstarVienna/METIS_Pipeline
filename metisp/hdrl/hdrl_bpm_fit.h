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

#ifndef HDRL_BPM_FIT_H
#define HDRL_BPM_FIT_H

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_parameter.h"
#include "hdrl_image.h"
#include "hdrl_imagelist.h"
#include <cpl.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
                            BPM Parameters
  ----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_bpm_fit_parameter_create_pval(int, double);
hdrl_parameter * hdrl_bpm_fit_parameter_create_rel_chi(int, double, double);
hdrl_parameter * hdrl_bpm_fit_parameter_create_rel_coef(int, double, double);
cpl_boolean hdrl_bpm_fit_parameter_check(const hdrl_parameter * self);
cpl_error_code hdrl_bpm_fit_parameter_verify(const hdrl_parameter *) ;

/* Accessors */
int hdrl_bpm_fit_parameter_get_degree(const hdrl_parameter *);
double hdrl_bpm_fit_parameter_get_pval(const hdrl_parameter *);
double hdrl_bpm_fit_parameter_get_rel_chi_low(const hdrl_parameter *);
double hdrl_bpm_fit_parameter_get_rel_chi_high(const hdrl_parameter *);
double hdrl_bpm_fit_parameter_get_rel_coef_low(const hdrl_parameter *);
double hdrl_bpm_fit_parameter_get_rel_coef_high(const hdrl_parameter *);

/* Parameter Lists */
cpl_parameterlist * hdrl_bpm_fit_parameter_create_parlist(const char *,
        const char *, const hdrl_parameter *);
hdrl_parameter * hdrl_bpm_fit_parameter_parse_parlist(
                                              const cpl_parameterlist *,
                                              const char *);

/*----------------------------------------------------------------------------
                            BPM_FIT Computation
  ----------------------------------------------------------------------------*/

cpl_error_code
hdrl_bpm_fit_compute(const hdrl_parameter * par,
                     const hdrl_imagelist * data,
                     const cpl_vector * sample_pos,
                     cpl_image ** out_mask);

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
 -----------------------------------------------------------------------------*/

#ifdef HDRL_USE_PRIVATE

#endif

CPL_END_DECLS

#endif
