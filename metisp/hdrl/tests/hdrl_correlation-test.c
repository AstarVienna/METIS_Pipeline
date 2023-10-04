/*
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                    Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_correlation.h"

#include <math.h>
#include <cpl.h>

cpl_array *
create_rect(const cpl_size sz, const cpl_size start, const cpl_size stop){

    cpl_array * v = cpl_array_new(sz, CPL_TYPE_DOUBLE);

    for(cpl_size i = 0; i < sz; ++i){

        double p = 0.0;

        if(i >= start && i <= stop)
            p = 5.0;

        cpl_array_set(v, i, p);
    }

    return v;
}

cpl_array *
create_gaussian(cpl_size num_samples, const double mean, const double stdev,
        const double center, double * delta_out){

    cpl_array * v = cpl_array_new(num_samples, CPL_TYPE_DOUBLE);
    const double size = 8.0;
    const double delta = size * stdev / (double)num_samples;
    const double start = -size * .5 * stdev + center;
    for(cpl_size i = 0; i < num_samples; ++i){
        const double x = delta * ((double)i) + start;
        const double val = (x - mean) / stdev;
        const double exponential = -.5 * pow(val, 2.0);
        const double y = exp(exponential)/(stdev * sqrt(2. * CPL_MATH_PI));
        cpl_array_set(v, i, y);
    }

    *delta_out = delta;

    return v;
}


/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_correlation_test
 */
/*----------------------------------------------------------------------------*/

void test_shift_pixel_precision(const cpl_size in_shift){

    const cpl_size sz = 28;
    const cpl_size win = 14;
    cpl_array * v1 = create_rect(sz, 3, 5);
    cpl_array * v2 = create_rect(sz, 3 + llabs(in_shift), 5 + llabs(in_shift));

    if(in_shift < 0){
        cpl_array * tmp = v1;
        v1 = v2;
        v2 = tmp;
    }

    hdrl_xcorrelation_result * res =
            hdrl_compute_xcorrelation(v1, v2, win, CPL_FALSE);

    const cpl_size idx = hdrl_xcorrelation_result_get_peak_pixel(res);
    const cpl_size shift = idx - win;

    const cpl_array * xcorr = hdrl_xcorrelation_result_get_correlation(res);

    cpl_test_eq(shift, -in_shift);

    if(in_shift == 0){
        cpl_test_rel(cpl_array_get(xcorr, idx, NULL),
                25.0 * 3.0 /((double)sz), 1e-5);
        cpl_test_rel(cpl_array_get(xcorr, idx - 1, NULL),
                                25.0 * 2.0 /((double)sz - 1.0), 1e-5);
        cpl_test_rel(cpl_array_get(xcorr, idx + 1, NULL),
                                25.0 * 2.0 /((double)sz - 1.0), 1e-5);
        cpl_test_rel(cpl_array_get(xcorr, idx - 2, NULL),
                                25.0 * 1.0 /((double)sz - 2.0), 1e-5);
        cpl_test_rel(cpl_array_get(xcorr, idx + 2, NULL),
                                25.0 * 1.0 /((double)sz - 2.0), 1e-5);
        for(cpl_size i = 0; i < win * 2 + 1; i++){
            if(i >= idx - 2 && i <= idx + 2) continue;
            cpl_test_rel(cpl_array_get(xcorr, i, NULL), 0.0, 1e-5);
        }
    }

    hdrl_xcorrelation_result_delete(res);

    cpl_array_delete(v1);
    cpl_array_delete(v2);
}

void test_shift_gaussian_fit(const double mean_diff,
        const cpl_boolean use_win_refinement){

    const double m1 = 1.0;
    const double m2 = m1 + mean_diff;
    const double std_dev = sqrt(.5);
    const cpl_size sz = 100;
    cpl_size half_w = 180;
    double delta = .0;
    cpl_array * arr1 = create_gaussian(sz, m1, std_dev, .5 * (m1 + m2), &delta);
    cpl_array * arr2 = create_gaussian(sz, m2, std_dev, .5 * (m1 + m2), &delta);

    hdrl_xcorrelation_result * r = NULL;

    if(use_win_refinement)
        r = hdrl_compute_offset_gaussian(arr1, arr2,
            half_w, CPL_TRUE, delta, 0.5);
    else
        r = hdrl_compute_offset_gaussian_internal(arr1, arr2,
            half_w, CPL_TRUE, delta, 0.5);

    const double peak = hdrl_xcorrelation_result_get_peak_subpixel(r);
    const cpl_size used_win = hdrl_xcorrelation_result_get_half_window(r);
    const double tolerance = use_win_refinement ? 5.6e-2 : 6e-2;
    if (mean_diff != 0.0) { // Needed -> check the definition of the cpl macro
        cpl_test_rel(-peak +  used_win * delta, mean_diff, tolerance);
    } else {
        cpl_test_abs(peak, used_win * delta, tolerance);
    }

    hdrl_xcorrelation_result_delete(r);
    cpl_array_delete(arr1);
    cpl_array_delete(arr2);
}

/*----------------------------------------------------------------------------*/
/**
  @brief   Unit tests of correlation and shift
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    test_shift_pixel_precision(0);
    test_shift_pixel_precision(2);
    test_shift_pixel_precision(6);
    test_shift_pixel_precision(-2);
    test_shift_pixel_precision(-6);


    test_shift_gaussian_fit(2.4, CPL_FALSE);
    test_shift_gaussian_fit(-2.4, CPL_FALSE);
    test_shift_gaussian_fit(1.8, CPL_FALSE);
    test_shift_gaussian_fit(0.0, CPL_FALSE);

    test_shift_gaussian_fit(2.4, CPL_TRUE);
    test_shift_gaussian_fit(-2.4, CPL_TRUE);
    test_shift_gaussian_fit(1.8, CPL_TRUE);
    test_shift_gaussian_fit(0.0, CPL_TRUE);

    cpl_test_error(CPL_ERROR_NONE);

    return cpl_test_end(0);
}

