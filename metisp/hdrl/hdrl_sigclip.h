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

#ifndef HDRL_SIGCLIP_H_
#define HDRL_SIGCLIP_H_

/*-----------------------------------------------------------------------------
                                Include
 -----------------------------------------------------------------------------*/

#include "hdrl_parameter.h" 
#include <cpl.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

cpl_parameterlist * hdrl_sigclip_parameter_create_parlist(
        const char              * base_context,
        const char              * prefix,
        const hdrl_parameter    * defaults);

cpl_error_code hdrl_sigclip_parameter_parse_parlist(
        const cpl_parameterlist * parlist,
        const char              * prefix,
        double                  * kappa_low,
        double                  * kappa_high,
        int                     * niter);

cpl_parameterlist * hdrl_minmax_parameter_create_parlist(
        const char              * base_context,
        const char              * prefix,
        const hdrl_parameter    * defaults);

cpl_error_code hdrl_minmax_parameter_parse_parlist(
        const cpl_parameterlist * parlist,
        const char              * prefix,
        double                  * nlow,
        double                  * nhigh);

cpl_error_code hdrl_kappa_sigma_clip_image(
        const cpl_image         * source,
        const cpl_image         * error,
        const double              kappa_low,
        const double              kappa_high,
        const int                 iter,
        double                  * mean_ks,
        double                  * mean_ks_err,
        cpl_size                * naccepted,
        double                  * reject_low,
        double                  * reject_high);

cpl_error_code hdrl_minmax_clip_image(
        const cpl_image         * source,
        const cpl_image         * error,
        const double              nlow,
        const double              nhigh,
        double                  * mean_mm,
        double                  * mean_mm_err,
        cpl_size                * naccepted,
        double                  * reject_low,
        double                  * reject_high);

cpl_error_code hdrl_kappa_sigma_clip(
        cpl_vector              * vec,
        cpl_vector              * vec_err,
        const double              kappa_low,
        const double              kappa_high,
        const int                 iter,
        cpl_boolean               inplace,
        double                  * mean_ks,
        double                  * mean_ks_err,
        cpl_size                * naccepted,
        double                  * reject_low,
        double                  * reject_high);

cpl_error_code hdrl_minmax_clip(
        cpl_vector              * vec,
        cpl_vector  	        * vec_err,
        const double              nlow,
        const double              nhigh,
        cpl_boolean               inplace,
        double                  * mean_mm,
        double                  * mean_mm_err,
        cpl_size                * naccepted,
        double                  * reject_low,
        double                  * reject_high);


CPL_END_DECLS

#endif /* HDRL_SIGCLIP_H_ */
