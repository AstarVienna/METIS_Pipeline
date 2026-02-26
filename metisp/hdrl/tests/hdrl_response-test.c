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

#include "hdrl.h"

#include <math.h>
#include <cpl.h>


/*-----------------------------------------------------------------------------
                                    Define
 -----------------------------------------------------------------------------*/

#define HDRL_DELTA_COMPARE_VALUE    HDRL_EPS_DATA * 1.


const hdrl_spectrum1D_wave_scale  scale = hdrl_spectrum1D_wave_scale_linear;

hdrl_spectrum1D * create_spectrum(double * wavs,
		double * flux, double * flux_e, const cpl_size sz){
	cpl_image * flx = cpl_image_wrap_double(sz, 1, flux);
	cpl_image * flx_e = cpl_image_wrap_double(sz, 1, flux_e);
	cpl_array * wav = cpl_array_wrap_double(wavs, sz);

	hdrl_spectrum1D * s = hdrl_spectrum1D_create(flx, flx_e, wav, scale);

	cpl_image_unwrap(flx);
	cpl_image_unwrap(flx_e);
	cpl_array_unwrap(wav);

	return s;
}

cpl_bivector * create_windows(const double * w1, const double * w2,
		const cpl_size sz){

	cpl_bivector * ret = cpl_bivector_new(sz);
	cpl_vector * v1 = cpl_bivector_get_x(ret);
	cpl_vector * v2 = cpl_bivector_get_y(ret);

	for(cpl_size i = 0; i < sz; ++i){
		cpl_vector_set(v1, i, w1[i]);
		cpl_vector_set(v2, i, w2[i]);
	}

	return ret;
}

cpl_array * create_array(const double * els, const cpl_size sz){
	cpl_array * to_ret = cpl_array_new(sz, HDRL_TYPE_DATA);
	for(cpl_size i = 0; i < sz; ++i){
		cpl_array_set(to_ret, i, els[i]);
	}
	return to_ret;
}

static cpl_error_code get_error_and_reset(void){
	cpl_error_code r = cpl_error_get_code();
	cpl_error_reset();
	return r;
}

#define cpl_ensure_no_error cpl_test_eq(get_error_and_reset(), CPL_ERROR_NONE);

#define cpl_ensure_error cpl_test_noneq(get_error_and_reset(), CPL_ERROR_NONE);

/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_response_test
 */
/*----------------------------------------------------------------------------*/
void test_response_basic(void)
{
	double flx[] = {1,2,3,4,5};
	double flx_e[] = {.1, .2, .1, .1, .05};
	double wlen[] = {3,5,7,9,11};
	hdrl_spectrum1D * s = create_spectrum(wlen, flx, flx_e, 5);

	hdrl_response_result * r =
			hdrl_response_compute(NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	cpl_ensure_error;
	cpl_test_null(r);

	r = hdrl_response_compute(s, NULL, NULL, NULL, NULL, NULL, NULL);
	cpl_ensure_error;
	cpl_test_null(r);

	r = hdrl_response_compute(s, s, s, NULL, NULL, NULL, NULL);
	cpl_ensure_error;
	cpl_test_null(r);

	hdrl_parameter * calc_par =
			hdrl_response_parameter_create((hdrl_value){1,.2},
										   (hdrl_value){2,.3},
										   (hdrl_value){5,.2},
										   (hdrl_value){7,.1});

	double aPoints[5] = {3.1, 3.2, 3.3, 6.9, 7.0};
	cpl_array * fit_points = create_array(aPoints, 5);

	hdrl_parameter * fit_par =
			hdrl_response_fit_parameter_create(11, fit_points, 1.0, NULL);

	r = hdrl_response_compute(s, s, s, NULL, NULL, NULL, NULL);
	cpl_ensure_error;
	cpl_test_null(r);
	hdrl_response_result_delete(r);

	r = hdrl_response_compute(s, s, s, NULL, NULL, calc_par, NULL);
	cpl_ensure_error;
	cpl_test_null(r);
	hdrl_response_result_delete(r);

	r = hdrl_response_compute(s, s, s, NULL, NULL, NULL, fit_par);
	cpl_ensure_error;
	cpl_test_null(r);
	hdrl_response_result_delete(r);

	r = hdrl_response_compute(s, s, s, NULL, NULL, fit_par, calc_par);
	cpl_ensure_error;
	cpl_test_null(r);
	hdrl_response_result_delete(r);

	r = hdrl_response_compute(s, s, s, NULL, NULL, calc_par, fit_par);
	cpl_ensure_no_error;
	cpl_test_nonnull(r);

	const hdrl_spectrum1D * raw_resp = hdrl_response_result_get_raw_response(r);

	int rej = 0;
	hdrl_value val = hdrl_spectrum1D_get_flux_value(raw_resp, 1, &rej);

	cpl_test_eq(rej, 0);
	cpl_test_eq(val.data, 5);
	cpl_test_eq(val.error, 4);

    const hdrl_spectrum1D * sel_resp = hdrl_response_result_get_selected_response(r);

    rej = 0;
    val = hdrl_spectrum1D_get_flux_value(sel_resp, 1, &rej);

    cpl_test_eq(rej, 0);
    cpl_test_eq(val.data, 2);
    cpl_test_eq(val.error, 1);

    const hdrl_spectrum1D * resp = hdrl_response_result_get_final_response(r);

    rej = 0;
    val = hdrl_spectrum1D_get_flux_value(resp, 1, &rej);

    cpl_test_eq(rej, 0);
    cpl_test_eq(val.data, 2);
    cpl_test_eq(val.error, 1);


	hdrl_response_result_delete(r);

	hdrl_parameter_delete(fit_par);
	hdrl_parameter_delete(calc_par);
	cpl_array_delete(fit_points);
	hdrl_spectrum1D_delete(&s);
}


void test_truncation(void){

    hdrl_spectrum1D * obs_s = NULL;
    hdrl_spectrum1D * ref_s = NULL;
    hdrl_spectrum1D * E_x = NULL;

    {
        double flx[] = {1,2,3,4,5};
        double flx_e[] = {.1, .2, .1, .1, .05};
        double wlen[] = {3,5,7,9,11};
        obs_s = create_spectrum(wlen, flx, flx_e, 5);
    }

    {
        double flx[] = {1,2,3,4,5};
        double flx_e[] = {.1, .2, .1, .1, .05};
        double wlen[] = {3.5,5,7,9,11};
        ref_s = create_spectrum(wlen, flx, flx_e, 5);
    }

    {
        double flx[] = {1,2,3,4,5};
        double flx_e[] = {.1, .2, .1, .1, .05};
        double wlen[] = {3,5,7,9,10.5};
        E_x = create_spectrum(wlen, flx, flx_e, 5);
    }

    hdrl_parameter * calc_par =
                hdrl_response_parameter_create((hdrl_value){1,.2},
                                               (hdrl_value){2,.3},
                                               (hdrl_value){5,.2},
                                               (hdrl_value){7,.1});

    cpl_array * fit_points = create_array((double[]){3.1, 6.8, 6.9, 7.0, 7.5, 9.0, 11.0}, 7);
    cpl_bivector * high_abs_regions = NULL;

    hdrl_parameter * fit_par =
            hdrl_response_fit_parameter_create(11, fit_points, 1.0, high_abs_regions);

    hdrl_response_result * r = hdrl_response_compute(obs_s,
            ref_s, E_x, NULL, NULL, calc_par, fit_par);
    cpl_ensure_no_error;
    cpl_test_nonnull(r);

    const hdrl_spectrum1D * sel_resp =
            hdrl_response_result_get_selected_response(r);
    const hdrl_spectrum1D * final_resp = hdrl_response_result_get_final_response(r);

    const cpl_array * wlens_dest = hdrl_spectrum1D_get_wavelength(final_resp).wavelength;

    const double wmin = cpl_array_get_min(wlens_dest);
    const double wmax = cpl_array_get_max(wlens_dest);

    cpl_test_rel(wmin, 5, 1e-10);
    cpl_test_rel(wmax, 9, 1e-10);

    cpl_test_eq(hdrl_spectrum1D_get_size(sel_resp), 5);

    const cpl_array * wlens_sel = hdrl_spectrum1D_get_wavelength(sel_resp).wavelength;

    cpl_test_rel(cpl_array_get(wlens_sel, 0, NULL), 6.8, HDRL_DELTA_COMPARE_VALUE);
    cpl_test_rel(cpl_array_get(wlens_sel, 1, NULL), 6.9, HDRL_DELTA_COMPARE_VALUE);
    cpl_test_rel(cpl_array_get(wlens_sel, 2, NULL), 7.0, HDRL_DELTA_COMPARE_VALUE);
    cpl_test_rel(cpl_array_get(wlens_sel, 3, NULL), 7.5, HDRL_DELTA_COMPARE_VALUE);
    cpl_test_rel(cpl_array_get(wlens_sel, 4, NULL), 9.0, HDRL_DELTA_COMPARE_VALUE);

    hdrl_spectrum1D_delete(&obs_s);
    hdrl_spectrum1D_delete(&ref_s);
    hdrl_spectrum1D_delete(&E_x);
    hdrl_response_result_delete(r);
    hdrl_parameter_delete(calc_par);
    hdrl_parameter_delete(fit_par);
    cpl_array_delete(fit_points);
    cpl_bivector_delete(high_abs_regions);
}


void test_edges_of_response_outside_fit_points(void){

	hdrl_spectrum1D * obs_s = NULL;
	hdrl_spectrum1D * ref_s = NULL;
	hdrl_spectrum1D * E_x = NULL;

	{
		double flx[] = {1,2,3,4,5};
		double flx_e[] = {.1, .2, .1, .1, .05};
		double wlen[] = {3,5,7,9,11};
		obs_s = create_spectrum(wlen, flx, flx_e, 5);
	}

	{
		double flx[] = {1,2,3,4,5};
		double flx_e[] = {.1, .2, .1, .1, .05};
		double wlen[] = {3,5,7,9,11};
		ref_s = create_spectrum(wlen, flx, flx_e, 5);
	}

	{
		double flx[] = {1,2,3,4,5};
		double flx_e[] = {.1, .2, .1, .1, .05};
		double wlen[] = {3,5,7,9,11};
		E_x = create_spectrum(wlen, flx, flx_e, 5);
	}

	hdrl_parameter * calc_par =
				hdrl_response_parameter_create((hdrl_value){1,.2},
											   (hdrl_value){2,.3},
											   (hdrl_value){5,.2},
											   (hdrl_value){7,.1});

	cpl_array * fit_points = create_array((double[]){3.1, 6.8, 6.9, 7.0, 7.5, 9.0, 11.0}, 7);
	cpl_bivector * high_abs_regions = NULL;

	hdrl_parameter * fit_par =
			hdrl_response_fit_parameter_create(11, fit_points, 1.0, high_abs_regions);

	hdrl_response_result * r = hdrl_response_compute(obs_s,
			ref_s, E_x, NULL, NULL, calc_par, fit_par);
	cpl_ensure_no_error;
	cpl_test_nonnull(r);

	const hdrl_spectrum1D * sel_resp =
			hdrl_response_result_get_selected_response(r);

	const cpl_array * wlens_selected = hdrl_spectrum1D_get_wavelength(sel_resp).wavelength;

	cpl_test_eq(cpl_array_get_size(wlens_selected), 7);

	cpl_test_rel(cpl_array_get(wlens_selected, 0, NULL),  3.1, HDRL_DELTA_COMPARE_VALUE);
	cpl_test_rel(cpl_array_get(wlens_selected, 1, NULL),  6.8, HDRL_DELTA_COMPARE_VALUE);
	cpl_test_rel(cpl_array_get(wlens_selected, 2, NULL),  6.9, HDRL_DELTA_COMPARE_VALUE);
	cpl_test_rel(cpl_array_get(wlens_selected, 3, NULL),  7.0, HDRL_DELTA_COMPARE_VALUE);
	cpl_test_rel(cpl_array_get(wlens_selected, 4, NULL),  7.5, HDRL_DELTA_COMPARE_VALUE);
	cpl_test_rel(cpl_array_get(wlens_selected, 5, NULL),  9.0, HDRL_DELTA_COMPARE_VALUE);
	cpl_test_rel(cpl_array_get(wlens_selected, 6, NULL), 11.0, HDRL_DELTA_COMPARE_VALUE);

	const hdrl_spectrum1D * final_resp = hdrl_response_result_get_final_response(r);

	const cpl_array * wlens_dest = hdrl_spectrum1D_get_wavelength(final_resp).wavelength;
	const cpl_array * wlens_obs = hdrl_spectrum1D_get_wavelength(obs_s).wavelength;

	cpl_test_eq(cpl_array_get_size(wlens_dest), cpl_array_get_size(wlens_obs));
	/*Must be defined on all the wavelengths (unless the models do not overlap, see test_truncation)*/
	for(cpl_size i = 0; i < cpl_array_get_size(wlens_dest); ++i){
		const double w_d = cpl_array_get(wlens_dest, i, NULL);
		const double w_o = cpl_array_get(wlens_obs, i, NULL);
		cpl_test_rel(w_d, w_o, 1e-10);
	}

	int rej = 0;
	/*wlen = 3.0 outside the fit points, rejected*/
	hdrl_spectrum1D_get_flux_value(final_resp, 0, & rej);
	cpl_test_eq(rej, 1);

	/*wlen = 7.0 is also the last fit point, NOT rejected*/
	hdrl_spectrum1D_get_flux_value(final_resp, cpl_array_get_size(wlens_dest) - 1, & rej);
	cpl_test_eq(rej, 0);

	hdrl_spectrum1D_delete(&obs_s);
	hdrl_spectrum1D_delete(&ref_s);
	hdrl_spectrum1D_delete(&E_x);
	hdrl_response_result_delete(r);
	hdrl_parameter_delete(calc_par);
	hdrl_parameter_delete(fit_par);
	cpl_array_delete(fit_points);
	cpl_bivector_delete(high_abs_regions);

}

void test_telluric(void){

   hdrl_spectrum1D * t1 = NULL;
   hdrl_spectrum1D * t2 = NULL;

   {
       double flx[] = {1, 2, 3, 4, 0, 1};
       double flx_e[] = {.1, .2, .1, .1, .05, .3, .83};
       double wlen[] = {3.1,5,7,9,10.9, 10.95};
       t1 = create_spectrum(wlen, flx, flx_e, 6);
   }

   {
          double flx[] = {2, 4, 6, 8, 0, 1};
          double flx_e[] = {.1, .2, .1, .1, .05, 3.3};
          double wlen[] = {3.1, 5, 7, 9, 10.9, 10.95};
          t2 = create_spectrum(wlen, flx, flx_e, 6);
   }

   hdrl_spectrum1Dlist * ts = hdrl_spectrum1Dlist_new();

   hdrl_spectrum1Dlist_set(ts, t2, 0);
   hdrl_spectrum1Dlist_set(ts, t1, 1);

   cpl_bivector * areas = cpl_bivector_new(3);

   cpl_vector_set(cpl_bivector_get_x(areas), 0, 3);
   cpl_vector_set(cpl_bivector_get_y(areas), 0, 5.1);

   cpl_vector_set(cpl_bivector_get_x(areas), 1, 6.9);
   cpl_vector_set(cpl_bivector_get_y(areas), 1, 7.1);

   cpl_vector_set(cpl_bivector_get_x(areas), 2, 10.0);
   cpl_vector_set(cpl_bivector_get_y(areas), 2, 11.0);

   hdrl_parameter * tell_par =
           hdrl_response_telluric_evaluation_parameter_create(ts, 1, 15, CPL_FALSE,
                   CPL_FALSE, areas , areas, 3, 11);

   hdrl_parameter * calc_par =
               hdrl_response_parameter_create((hdrl_value){1,.2},
                                              (hdrl_value){2,.3},
                                              (hdrl_value){5,.2},
                                              (hdrl_value){7,.1});

   cpl_array * fit_points = create_array((double[]){3.1, 6.8, 6.9, 7.0, 7.5, 9.0, 11.0}, 7);
   cpl_bivector * high_abs_regions = create_windows((double[]){8.9}, (double[]){9.1}, 1);

   hdrl_parameter * fit_par =
           hdrl_response_fit_parameter_create(11, fit_points, 1.0, high_abs_regions);

   hdrl_response_result * r = hdrl_response_compute(t1,
           t1, t1, tell_par, NULL, calc_par, fit_par);

   cpl_size idx = hdrl_response_result_get_best_telluric_model_idx(r);
   cpl_test_eq(idx, 1);

   double mean    = hdrl_response_result_get_avg_diff_from_1(r);
   double stddev  = hdrl_response_result_get_stddev(r);
   double shift   = hdrl_response_result_get_telluric_shift(r);
   double doppler = hdrl_response_result_get_doppler_shift(r);

   cpl_test_rel(mean, 0.0041, 1e-2);
   cpl_test_rel(stddev, 0.707, 1e-2);
   cpl_test((fabs(shift) < 1e-10));
   cpl_test(doppler >= 0.);

   const hdrl_spectrum1D * obs_corr =
           hdrl_response_result_get_corrected_obs_spectrum(r);

   cpl_test_eq(hdrl_spectrum1D_get_size(obs_corr), hdrl_spectrum1D_get_size(t1));

   hdrl_value v = hdrl_spectrum1D_get_flux_value(obs_corr, 2, NULL);
   cpl_test_rel(v.data, 1, 1e-5);

   cpl_ensure_no_error;
   cpl_test_nonnull(r);
   cpl_bivector_delete(areas);
   hdrl_spectrum1Dlist_delete(ts);
   hdrl_response_result_delete(r);
   hdrl_parameter_delete(calc_par);
   hdrl_parameter_delete(fit_par);
   hdrl_parameter_delete(tell_par);
   cpl_array_delete(fit_points);
   cpl_bivector_delete(high_abs_regions);
}

void  test_ignore_abs_regions(void){

	hdrl_spectrum1D * obs_s = NULL;
	hdrl_spectrum1D * ref_s = NULL;
	hdrl_spectrum1D * E_x = NULL;

	{
		double flx[] = {1,2,3,4,5};
		double flx_e[] = {.1, .2, .1, .1, .05};
		double wlen[] = {3,5,7,9,11};
		obs_s = create_spectrum(wlen, flx, flx_e, 5);
	}

	{
		double flx[] = {1,2,3,4,5};
		double flx_e[] = {.1, .2, .1, .1, .05};
		double wlen[] = {3,5,7,9,11};
		ref_s = create_spectrum(wlen, flx, flx_e, 5);
	}

	{
		double flx[] = {1,2,3,4,5};
		double flx_e[] = {.1, .2, .1, .1, .05};
		double wlen[] = {3,5,7,9,11};
		E_x = create_spectrum(wlen, flx, flx_e, 5);
	}

	hdrl_parameter * calc_par =
				hdrl_response_parameter_create((hdrl_value){1,.2},
											   (hdrl_value){2,.3},
											   (hdrl_value){5,.2},
											   (hdrl_value){7,.1});

	cpl_array * fit_points = create_array((double[]){3.1, 6.8, 6.9, 7.0, 7.5, 9.0, 11.0}, 7);
	cpl_bivector * high_abs_regions =  create_windows((double[]){8.9}, (double[]){9.1}, 1);


	hdrl_parameter * fit_par_no_abs =
			hdrl_response_fit_parameter_create(11, fit_points, 1.0, NULL);

	hdrl_parameter * fit_par_abs =
			hdrl_response_fit_parameter_create(11, fit_points, 1.0, high_abs_regions);

	hdrl_response_result * r_no_abs = hdrl_response_compute(obs_s,
			ref_s, E_x, NULL, NULL, calc_par, fit_par_no_abs);
	cpl_ensure_no_error;
	cpl_test_nonnull(r_no_abs);

	hdrl_response_result * r_abs = hdrl_response_compute(obs_s,
			ref_s, E_x, NULL, NULL, calc_par, fit_par_abs);
	cpl_ensure_no_error;
	cpl_test_nonnull(r_abs);

	const hdrl_spectrum1D * resp_selected_abs = hdrl_response_result_get_selected_response(r_abs);
	const hdrl_spectrum1D * resp_selected_no_abs = hdrl_response_result_get_selected_response(r_no_abs);

	cpl_test_eq(hdrl_spectrum1D_get_size(resp_selected_abs), hdrl_spectrum1D_get_size(resp_selected_no_abs) - 1);

	int rej = 0;
	cpl_test_rel(hdrl_spectrum1D_get_wavelength_value(resp_selected_abs, 0, &rej),
			hdrl_spectrum1D_get_wavelength_value(resp_selected_no_abs, 0, &rej), 1e-10);
	cpl_test_rel(hdrl_spectrum1D_get_wavelength_value(resp_selected_abs, 1, &rej),
			hdrl_spectrum1D_get_wavelength_value(resp_selected_no_abs, 1, &rej), 1e-10);
	cpl_test_rel(hdrl_spectrum1D_get_wavelength_value(resp_selected_abs, 2, &rej),
			hdrl_spectrum1D_get_wavelength_value(resp_selected_no_abs, 2, &rej), 1e-10);
	cpl_test_rel(hdrl_spectrum1D_get_wavelength_value(resp_selected_abs, 3, &rej),
			hdrl_spectrum1D_get_wavelength_value(resp_selected_no_abs, 3, &rej), 1e-10);
	cpl_test_rel(hdrl_spectrum1D_get_wavelength_value(resp_selected_abs, 4, &rej),
			hdrl_spectrum1D_get_wavelength_value(resp_selected_no_abs, 4, &rej), 1e-10);
	cpl_test_rel(hdrl_spectrum1D_get_wavelength_value(resp_selected_abs, 5, &rej),
			hdrl_spectrum1D_get_wavelength_value(resp_selected_no_abs, 6, &rej), 1e-10);

	hdrl_spectrum1D_delete(&obs_s);
	hdrl_spectrum1D_delete(&ref_s);
	hdrl_spectrum1D_delete(&E_x);
	hdrl_response_result_delete(r_no_abs);
	hdrl_response_result_delete(r_abs);
	hdrl_parameter_delete(calc_par);
	hdrl_parameter_delete(fit_par_no_abs);
	hdrl_parameter_delete(fit_par_abs);
	cpl_array_delete(fit_points);
	cpl_bivector_delete(high_abs_regions);
}

/*----------------------------------------------------------------------------*/
/**
  @brief   Unit tests of efficiency calculation module
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    test_response_basic();

    test_truncation();

    test_edges_of_response_outside_fit_points();

    test_ignore_abs_regions();

    test_telluric();

    cpl_test_error(CPL_ERROR_NONE);

    return cpl_test_end(0);
}

