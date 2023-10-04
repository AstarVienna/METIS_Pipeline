/*
 * This file is part of the HDRL
 * Copyright (C) 2015 European Southern Observatory
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
 *
 * hdrl_fringe.h
 *
 *  Created on: May 11, 2015
 *      Author: agabasch
 */

#ifndef HDRL_FRINGE_H
#define HDRL_FRINGE_H
/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_imagelist.h"
#include <cpl.h>

/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

cpl_error_code
hdrl_fringe_compute(hdrl_imagelist* ilist_fringe, const cpl_imagelist * ilist_obj,
                    const cpl_mask* stat_mask, const hdrl_parameter* collapse_params,
                    hdrl_image** master, cpl_image** contrib_map,
                    cpl_table ** qctable);
cpl_error_code
hdrl_fringe_correct(hdrl_imagelist * ilist_fringe, const cpl_imagelist * ilist_obj,
                    const cpl_mask * stat_mask, const hdrl_image * masterfringe,
                    cpl_table ** qctable);

#if defined HDRL_USE_PRIVATE
cpl_matrix * hdrl_mime_fringe_amplitudes(const cpl_image * img0,
          const cpl_mask * mask0);

cpl_matrix * hdrl_mime_fringe_amplitudes_ls(const cpl_image * img0,
          const cpl_mask * mask0, const cpl_image * fringe0);

int hdrl_mime_gmix_derivs1(const double x[], const double params[],
          double result[]);
int hdrl_mime_gmix1(const double x[], const double params[], double *result);
cpl_matrix *hdrl_mime_hermite_series_create(int n, double center,
          double scale, const cpl_matrix * coeffs, const cpl_matrix * x);
cpl_matrix *hdrl_mime_hermite_functions_sums_create(int n, double center,
          double scale, const cpl_matrix * x);
#endif


#endif /* HDRL_FRINGE_H */
