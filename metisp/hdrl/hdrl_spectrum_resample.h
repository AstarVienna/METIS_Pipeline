/* $Id: hdrl_spectrum_resample.h,v 0.1 2017-03-14 16:49:00 msalmist Exp $
 *
 * This file is part of the HDRL
 * Copyright (C) 2017 European Southern Observatory
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
 * $Author: msalmist $
 * $Date: 2017-03-14 16:49:00 $
 * $Revision: 0.1 $
 * $Name: not supported by cvs2svn $
 */

#ifndef HDRL_SPECTRUM_RESAMPLE_H
#define HDRL_SPECTRUM_RESAMPLE_H

/*-----------------------------------------------------------------------------
                                   New types
 -----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_spectrum.h"
#include "hdrl_parameter.h"

#include <cpl.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                                   Data structures
 -----------------------------------------------------------------------------*/
/* Possible methods for interpolation possible */
typedef enum{
    hdrl_spectrum1D_interp_linear,
    hdrl_spectrum1D_interp_cspline,
    hdrl_spectrum1D_interp_akima,
}hdrl_spectrum1D_interpolation_method;


hdrl_parameter* hdrl_spectrum1D_resample_interpolate_parameter_create
(const hdrl_spectrum1D_interpolation_method method);

hdrl_parameter* hdrl_spectrum1D_resample_integrate_parameter_create(void);

hdrl_parameter * hdrl_spectrum1D_resample_interpolate_parameter_parse_parlist(
        const cpl_parameterlist *, const char *);
cpl_parameterlist * hdrl_spectrum1D_resample_interpolate_parameter_create_parlist(
        const char *, const char *, const char *) ;

hdrl_parameter*
hdrl_spectrum1D_resample_fit_parameter_create(const int k, const int nCoeff);

#if defined HDRL_USE_EXPERIMENTAL || defined HDRL_USE_PRIVATE
hdrl_parameter*
hdrl_spectrum1D_resample_fit_windowed_parameter_create(
        const int k, const int nCoeff, const long window, const double factor);
#endif

cpl_error_code hdrl_resample_parameter_verify(const hdrl_parameter *);

/*-----------------------------------------------------------------------------
                                   Functions
 -----------------------------------------------------------------------------*/

hdrl_spectrum1D *
hdrl_spectrum1D_resample(const hdrl_spectrum1D * self,
                         const hdrl_spectrum1D_wavelength* waves,
                         const hdrl_parameter* par);

hdrl_spectrum1D *
hdrl_spectrum1D_resample_on_array(const hdrl_spectrum1D * self,
                                  const cpl_array* waves,
                                  const hdrl_parameter* par);

#if defined HDRL_USE_EXPERIMENTAL || defined HDRL_USE_PRIVATE

void
hdrl_spectrum1D_resample_sort_on_x(double * x, double * y1, double * y2, cpl_size sample_len);

cpl_size hdrl_spectrum1D_resample_filter_dups_and_substitute_with_median(double * x, double * y1,
        double * y2, const cpl_size sample_len);


cpl_boolean hdrl_spectrum1D_resample_is_strictly_monotonic_increasing(double * x, cpl_size l);

#endif

CPL_END_DECLS

#endif 
