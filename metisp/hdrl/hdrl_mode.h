/*
 * hdrl_mode.h
 *
 *  Created on: Mar 1, 2021
 *      Author: agabasch
 */

/*
 * This file is part of the HDRL
 * Copyright (C) 2021 European Southern Observatory
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

#ifndef HDRL_MODE_H
#define HDRL_MODE_H


/*-----------------------------------------------------------------------------
                                Include
 -----------------------------------------------------------------------------*/

#include "hdrl_mode_defs.h"
#include "hdrl_parameter.h"
#include <cpl.h>

CPL_BEGIN_DECLS


/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

cpl_parameterlist * hdrl_mode_parameter_create_parlist(
    const char              * base_context,
    const char              * prefix,
    const hdrl_parameter    * defaults);

cpl_error_code hdrl_mode_parameter_parse_parlist(
    const cpl_parameterlist * parlist,
    const char              * prefix,
    double                  * histo_min,
    double                  * histo_max,
    double                  * bin_size,
    hdrl_mode_type          * method,
    cpl_size                * error_niter);


cpl_error_code hdrl_mode_clip_image(
    const cpl_image         * source,
    const double              histo_min,
    const double              histo_max,
    const double              bin_size,
    const hdrl_mode_type      method,
    const cpl_size            error_niter,
    double                  * mode,
    double                  * mode_error,
    cpl_size                * naccepted);

cpl_error_code hdrl_mode_clip(
    cpl_vector              * vec,
    const double              histo_min,
    const double              histo_max,
    const double              bin_size,
    const hdrl_mode_type      method,
    const cpl_size            error_niter,
    double                  * mode,
    double                  * mode_error,
    cpl_size                * naccepted);

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
 -----------------------------------------------------------------------------*/
#ifdef HDRL_USE_PRIVATE

cpl_error_code
hdrl_mode_bootstrap(
    const cpl_vector        * vec,
    const double              histo_min,
    const double              histo_max,
    const double              bin_size,
    const hdrl_mode_type      method,
    const cpl_size            error_niter,
    double                  * mode_error);


#endif

CPL_END_DECLS

#endif /* HDRL_MODE_H */

