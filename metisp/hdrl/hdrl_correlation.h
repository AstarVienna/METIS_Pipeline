/* $Id: hdrl_correlation.h,v 0.1 2017-04-12 16:49:47 msalmist Exp $
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
 * $Date: 2017-04-12 16:49:47 $
 * $Revision: 0.1 $
 * $Name: not supported by cvs2svn $
 */

#ifndef HDRL_CORRELATION_H
#define HDRL_CORRELATION_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>
#include "hdrl_types.h"

CPL_BEGIN_DECLS


typedef struct{
    double peakpos; /* Position of the peak of the gaussian fitted  to the cross correlation */
    double sigma;   /* Width of the gaussian fitted  to the cross correlation */
    double area;    /* Area of the gaussian fitted  to the cross correlation */
    double offset;  /* Fitted background level */
    double mse;     /* Mean squared error of the best fit */
    cpl_array * xcorr; /* Cross-correlation */
    cpl_size pix_peakpos; /* Pixel position of the peak */
    cpl_size half_window;
}hdrl_xcorrelation_result;

hdrl_xcorrelation_result *
hdrl_xcorrelation_result_wrap(cpl_array * x_corr,  const cpl_size max_idx,
        const cpl_size half_window);

void hdrl_xcorrelation_result_delete(hdrl_xcorrelation_result * self);

cpl_size
hdrl_xcorrelation_result_get_peak_pixel(const hdrl_xcorrelation_result * self);

double
hdrl_xcorrelation_result_get_peak_subpixel(const hdrl_xcorrelation_result * self);

cpl_size
hdrl_xcorrelation_result_get_half_window (const hdrl_xcorrelation_result * self);

double
hdrl_xcorrelation_result_get_sigma(const hdrl_xcorrelation_result * self);

const cpl_array *
hdrl_xcorrelation_result_get_correlation(const hdrl_xcorrelation_result * self);


hdrl_xcorrelation_result * hdrl_compute_xcorrelation(
       const cpl_array * arr1,
       const cpl_array  * arr2,
       const cpl_size half_window, const cpl_boolean normalize);

hdrl_xcorrelation_result * hdrl_compute_offset_gaussian(
        const cpl_array * arr1,
        const cpl_array  * arr2,
        const cpl_size half_win, const cpl_boolean normalize,
        const double bin, const double wrange);

#if defined HDRL_USE_PRIVATE

hdrl_xcorrelation_result *
hdrl_compute_offset_gaussian_internal(
        const cpl_array * arr1, const cpl_array * arr2,
        const cpl_size half_win, const cpl_boolean normalize,
        const double bin, const double wrange);

#endif

CPL_END_DECLS

#endif 
