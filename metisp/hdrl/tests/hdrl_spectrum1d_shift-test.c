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

#include "hdrl_spectrum_shift.h"

#include <math.h>
#include <cpl.h>

/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_spectrum1d_shift test, the xcorrelation methods are already
 * covered by the correlation tests.
 */
/*----------------------------------------------------------------------------*/

double calc_gauss(double mean, double sigma, double x){

    double exponent = - pow((x- mean), 2.0) / (2.0 * sigma * sigma);
    double v = 1.0/(2.0 * CPL_MATH_PI * sigma * sigma) * exp(exponent);
    return v;
}

hdrl_value gauss_func(hdrl_data_t lambda){

    hdrl_data_t mean = 1500;
    hdrl_data_t sigma = 250;

    double v = calc_gauss(mean, sigma, lambda);

    return (hdrl_value){v, 0.0};
}

hdrl_value absorption1_func(hdrl_data_t lambda){

    hdrl_data_t mean = 1754;
    hdrl_data_t sigma = .75;

    double v = calc_gauss(mean, sigma, lambda);

    return (hdrl_value){ exp(-v), 0.0};
}

hdrl_value absorption2_func(hdrl_data_t lambda){

    hdrl_data_t mean = 1504;
    hdrl_data_t sigma = .75;

    double v = calc_gauss(mean, sigma, lambda);

    return (hdrl_value){ exp(-v), 0.0};
}

cpl_array * get_wlengths(double start, double stop, double step){

    cpl_size sz = (cpl_size)floor((stop - start) / step);
    cpl_array * arr = cpl_array_new(sz, HDRL_TYPE_DATA);

    for(cpl_size i = 0; i < sz; i++){
        cpl_array_set(arr, i, start);
        start += step;
    }

    return arr;
}

void test_on_slope(void){
    cpl_array * wlenghts = get_wlengths(1e3, 2e3, 1.0);

    hdrl_spectrum1D * gaussian =
          hdrl_spectrum1D_create_analytic(gauss_func, wlenghts, hdrl_spectrum1D_wave_scale_linear);

    hdrl_spectrum1D * absorption =
              hdrl_spectrum1D_create_analytic(absorption1_func, wlenghts, hdrl_spectrum1D_wave_scale_linear);


    hdrl_spectrum1D * gaussian_with_abs =
          hdrl_spectrum1D_mul_spectrum_create(gaussian, absorption);

    hdrl_parameter * par =
          hdrl_spectrum1D_shift_fit_parameter_create(1750, 1730, 1770, 1740, 1760, 20);

    hdrl_data_t offset = hdrl_spectrum1D_compute_shift_fit(gaussian_with_abs, par);

    cpl_test_rel((1.0 + offset) * 1750., 1754., 1e-3);

    hdrl_spectrum1D_delete(&gaussian);
    hdrl_spectrum1D_delete(&absorption);
    hdrl_spectrum1D_delete(&gaussian_with_abs);
    cpl_array_delete(wlenghts);
    hdrl_parameter_delete(par);
}

void test_on_peak(void){

    cpl_array * wlenghts = get_wlengths(1e3, 2e3, 1.0);

    hdrl_spectrum1D * gaussian =
          hdrl_spectrum1D_create_analytic(gauss_func, wlenghts, hdrl_spectrum1D_wave_scale_linear);

    hdrl_spectrum1D * absorption =
              hdrl_spectrum1D_create_analytic(absorption2_func, wlenghts, hdrl_spectrum1D_wave_scale_linear);


    hdrl_spectrum1D * gaussian_with_abs =
          hdrl_spectrum1D_mul_spectrum_create(gaussian, absorption);

    hdrl_parameter * par =
          hdrl_spectrum1D_shift_fit_parameter_create(1500, 1480, 1520, 1490, 1510, 20);

    hdrl_data_t offset = hdrl_spectrum1D_compute_shift_fit(gaussian_with_abs, par);

    cpl_test_rel((1.0 + offset) * 1500., 1504., 1e-3);

    hdrl_spectrum1D_delete(&gaussian);
    hdrl_spectrum1D_delete(&absorption);
    hdrl_spectrum1D_delete(&gaussian_with_abs);
    cpl_array_delete(wlenghts);
    hdrl_parameter_delete(par);
}


void test_compute_shift_xcorrelation(void)
{
    cpl_array * wlenghts = get_wlengths(1e3, 2e3, 1.0);

    hdrl_spectrum1D * gaussian = hdrl_spectrum1D_create_analytic(
    		gauss_func, wlenghts, hdrl_spectrum1D_wave_scale_linear);

    hdrl_spectrum1D * absorption = hdrl_spectrum1D_create_analytic(
    		absorption2_func, wlenghts, hdrl_spectrum1D_wave_scale_linear);

    cpl_size half_win = 1;

    /* Test nulls */

    hdrl_spectrum1D_compute_shift_xcorrelation(
    		NULL, NULL, half_win, CPL_FALSE);
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    hdrl_spectrum1D_compute_shift_xcorrelation(
    		NULL, absorption, half_win, CPL_FALSE);
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    hdrl_spectrum1D_compute_shift_xcorrelation(
    		gaussian, NULL, half_win, CPL_FALSE);
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    hdrl_spectrum1D_compute_shift_xcorrelation(
    		gaussian, absorption, half_win, CPL_FALSE);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);


    hdrl_spectrum1D_delete(&gaussian);
    hdrl_spectrum1D_delete(&absorption);
    cpl_array_delete(wlenghts);
}

/*----------------------------------------------------------------------------*/
/**
  @brief   Unit tests of spectrum shift estimation module
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    test_on_slope();
    test_on_peak();

    test_compute_shift_xcorrelation();

    return cpl_test_end(0);
}

