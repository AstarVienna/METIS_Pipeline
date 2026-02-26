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

#include "hdrl_spectrum.h"
#include "hdrl_spectrumlist.h"
#include "hdrl_spectrum_resample.h"
#include "hdrl_DER_SNR.h"

#include <cpl.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

/*-----------------------------------------------------------------------------
                                    Define
 -----------------------------------------------------------------------------*/

#define HDRL_DELTA_COMPARE_VALUE      CPL_MAX(HDRL_EPS_DATA, HDRL_EPS_ERROR) * 1.
#define HDRL_DELTA_COMPARE_VALUE_ABS  CPL_MAX(HDRL_EPS_DATA, HDRL_EPS_ERROR) * 4. 

#define LENGTH_ARRAY(a) ((cpl_size)(sizeof((a))/sizeof((a[0]))))

/*-----------------------------------------------------------------------------
                                    Function prototipes
 -----------------------------------------------------------------------------*/

static inline cpl_image * get_random_1d_img(cpl_size length,
        double min, double max, cpl_type type);

static inline void set_1d_bpm(cpl_image * img);

static inline cpl_array * get_wavelength(cpl_size length, cpl_type type);

static inline cpl_boolean are_hdrl_eq(const hdrl_image* flux_compound,
        const cpl_image * flux, const cpl_image * flux_e);

static inline cpl_error_code get_error_code_and_reset();

static inline double rand_0_to_1();

static inline hdrl_spectrum1D *
get_random_spectrum(int length, hdrl_spectrum1D_wave_scale scale);

typedef hdrl_spectrum1D * (* operate_spectra_create)
                                                (const hdrl_spectrum1D * self,
                                                 const hdrl_spectrum1D * other);

typedef cpl_error_code (* operate_spectra)(hdrl_spectrum1D * self,
                                           const hdrl_spectrum1D * other);

static inline void test_error_create_func(const hdrl_spectrum1D * s1,
                    const hdrl_spectrum1D * s2, operate_spectra_create f);

static inline void
test_calc_creat_error(operate_spectra_create f);

static inline void test_calc_error(operate_spectra f);

static inline hdrl_spectrum1D * get_spectrum1D_sin_shuffled
(cpl_size sz, int start, cpl_boolean add_peak, cpl_array ** unshuffled_lambda);

typedef struct{
    double lambda_min;
    double lambda_max;
    cpl_boolean is_internal;
}sel_window;

hdrl_spectrum1D * select_window(hdrl_spectrum1D * s, const sel_window w){

    cpl_bivector * vec = cpl_bivector_new(1);

    cpl_vector_set(cpl_bivector_get_x(vec), 0, w.lambda_min);
    cpl_vector_set(cpl_bivector_get_y(vec), 0, w.lambda_max);

    hdrl_spectrum1D * to_ret = hdrl_spectrum1D_select_wavelengths(s, vec, w.is_internal);
    cpl_bivector_delete(vec);

    return to_ret;
}

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Testing of the HDRL spectrum1D constructors
 */
/*----------------------------------------------------------------------------*/

void test_spectrum1D_constructor(cpl_type type){

    const cpl_size sz = 40;
    cpl_image * spectrum1d       = get_random_1d_img(sz, 0.0f, 128.0f, type);
    cpl_image * spectrum1d_error = get_random_1d_img(sz, 0.0f, 1.0f,type);
    set_1d_bpm(spectrum1d);
    cpl_array * wavelengths = get_wavelength(sz,type);

    /* test when passing error array */
    hdrl_spectrum1D * spec;

    spec = hdrl_spectrum1D_create(
            spectrum1d, spectrum1d_error,
            wavelengths, hdrl_spectrum1D_wave_scale_linear);

    cpl_test_eq(hdrl_spectrum1D_get_size(spec), 40);
    cpl_boolean are_equal = are_hdrl_eq(
            hdrl_spectrum1D_get_flux(spec), spectrum1d, spectrum1d_error
            );
    cpl_test_eq(are_equal, CPL_TRUE);

    hdrl_spectrum1D_delete(&spec);
    cpl_test_null(spec);

    const cpl_size wn = 10;

    /* test when using DER SNR error*/
    spec = hdrl_spectrum1D_create_error_DER_SNR(
            spectrum1d, wn,
            wavelengths, hdrl_spectrum1D_wave_scale_linear);
    cpl_test_nonnull(spec);

    /*some casting to make sure that we are always using hdrl_data_t types*/
    cpl_image * flux_casted = cpl_image_cast(spectrum1d, HDRL_TYPE_DATA);
    const hdrl_data_t * flux =
            (const hdrl_data_t *) cpl_image_get_data_const(flux_casted);
    const cpl_mask   *msk_in = cpl_image_get_bpm_const(flux_casted);
    const cpl_binary *msk_bn = cpl_mask_get_data_const(msk_in);

    cpl_image * noise = estimate_noise_DER_SNR(flux, msk_bn, wavelengths,
            sz, wn);

    cpl_image_delete(flux_casted);

    cpl_mask * msk = cpl_image_unset_bpm(noise);
    cpl_mask_delete(cpl_image_set_bpm(spectrum1d, msk));

    {
        hdrl_spectrum1D * spec2 = hdrl_spectrum1D_create(
                spectrum1d, noise,
                wavelengths, hdrl_spectrum1D_wave_scale_linear);

        const cpl_image * flux2 =
                hdrl_image_get_image_const(hdrl_spectrum1D_get_flux(spec2));

        const cpl_image * flux2_e =
                    hdrl_image_get_error_const(hdrl_spectrum1D_get_flux(spec2));

        cpl_boolean are_equal2 =
                are_hdrl_eq(hdrl_spectrum1D_get_flux(spec), flux2, flux2_e);
        cpl_test_eq(are_equal2, CPL_TRUE);

        hdrl_spectrum1D_delete(&spec2);
        cpl_test_null(spec2);
    }

    hdrl_spectrum1D_delete(&spec);
    cpl_test_null(spec);

    spec = hdrl_spectrum1D_create_error_free(
               spectrum1d, wavelengths, hdrl_spectrum1D_wave_scale_linear);
    cpl_test_nonnull(spec);

    cpl_size not_rej = 0;
    for(cpl_size i = 0; i < sz; ++i){
        int rej;
        const hdrl_value v = hdrl_spectrum1D_get_flux_value(spec, i, &rej);
        if(rej) continue;
        cpl_test_eq(v.error, 0.0);
        not_rej ++;
    }
    cpl_test(not_rej > 0);

    cpl_array_delete(wavelengths);
    cpl_image_delete(spectrum1d);
    cpl_image_delete(noise);
    cpl_image_delete(spectrum1d_error);

    hdrl_spectrum1D_delete(&spec);
    cpl_test_null(spec);

    /* deleting a nullptr should be NOOP */
    hdrl_spectrum1D_delete(&spec);
}

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Testing of the HDRL spectrum1D analytical
                               constructors
 */
/*----------------------------------------------------------------------------*/

hdrl_value test_val(hdrl_data_t lambda){
    return (hdrl_value){lambda * 2.0, lambda *3.0};
}

void test_spectrum1D_constructor_analytical(void){

    cpl_size sz = 10;
    cpl_array * wav = cpl_array_new(sz, CPL_TYPE_DOUBLE);

    for(cpl_size i = 0; i < sz; i++){
        cpl_array_set(wav, i, (1 + i) * 10.0);
    }

    hdrl_spectrum1D * spec = hdrl_spectrum1D_create_analytic
            (test_val, wav, hdrl_spectrum1D_wave_scale_linear);

    for(cpl_size i = 0; i < sz; i++){

        hdrl_value v = hdrl_spectrum1D_get_flux_value(spec, i, NULL);

        cpl_test_abs(v.data, (1 + i) * 20.0, 1e-3);
        cpl_test_abs(v.error, (1 + i) * 30.0, 1e-3);

        hdrl_data_t ws = hdrl_spectrum1D_get_wavelength_value(spec, i, NULL);
        hdrl_data_t ww = cpl_array_get(wav, i, NULL);
        cpl_test_abs(ws, ww, 1e-3);
    }

    cpl_test_eq(hdrl_spectrum1D_wave_scale_linear,
            hdrl_spectrum1D_get_scale(spec));

    cpl_array_delete(wav);
    hdrl_spectrum1D_delete(&spec);
}

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Testing of the HDRL spectrum1D constructors
                               errors
 */
/*----------------------------------------------------------------------------*/

void test_spectrum1D_constructor_error(void){

    cpl_image * spectrum1d_40 =
            get_random_1d_img(40, 0.0f, 128.0f, CPL_TYPE_DOUBLE);
    cpl_image * spectrum1d_error_40 =
            get_random_1d_img(40, 0.0f, 1.0f,CPL_TYPE_DOUBLE);
    set_1d_bpm(spectrum1d_40);
    cpl_array * wavelengths_40 = get_wavelength(40,CPL_TYPE_DOUBLE);

    cpl_image * spectrum1d_42 =
            get_random_1d_img(42, 0.0f, 128.0f, CPL_TYPE_DOUBLE);
    cpl_image * spectrum1d_error_42 =
            get_random_1d_img(42, 0.0f, 1.0f,CPL_TYPE_DOUBLE);
    set_1d_bpm(spectrum1d_42);
    cpl_array * wavelengths_42 = get_wavelength(42,CPL_TYPE_DOUBLE);

    hdrl_spectrum1D * spec = hdrl_spectrum1D_create(NULL, NULL,
                                 wavelengths_40,
                                 hdrl_spectrum1D_wave_scale_linear);

    cpl_test_null(spec);
    cpl_test_eq(get_error_code_and_reset(), CPL_ERROR_NULL_INPUT);

    hdrl_spectrum1D_delete(&spec);

    spec = hdrl_spectrum1D_create(spectrum1d_40, NULL,
                               NULL, hdrl_spectrum1D_wave_scale_linear);

    cpl_test_null(spec);
    cpl_test_eq(get_error_code_and_reset(), CPL_ERROR_NULL_INPUT);

    hdrl_spectrum1D_delete(&spec);

    spec = hdrl_spectrum1D_create(spectrum1d_40, spectrum1d_error_42,
                           wavelengths_40, hdrl_spectrum1D_wave_scale_linear);

    cpl_test_null(spec);
    cpl_test_eq(get_error_code_and_reset(), CPL_ERROR_INCOMPATIBLE_INPUT);

    spec = hdrl_spectrum1D_create(spectrum1d_42, spectrum1d_error_40,
                           wavelengths_40, hdrl_spectrum1D_wave_scale_linear);

    cpl_test_null(spec);
    cpl_test_eq(get_error_code_and_reset(), CPL_ERROR_INCOMPATIBLE_INPUT);

    spec = hdrl_spectrum1D_create(spectrum1d_40, spectrum1d_error_40,
                           wavelengths_42, hdrl_spectrum1D_wave_scale_linear);

    cpl_test_null(spec);
    cpl_test_eq(get_error_code_and_reset(), CPL_ERROR_INCOMPATIBLE_INPUT);

    double el0 = cpl_array_get(wavelengths_40, 0, NULL);
    cpl_array_set(wavelengths_40, 1, el0);

    spec = hdrl_spectrum1D_create(spectrum1d_40, spectrum1d_error_40,
                          wavelengths_40, hdrl_spectrum1D_wave_scale_linear);
    cpl_test_nonnull(spec);
    cpl_test_eq(get_error_code_and_reset(), CPL_ERROR_NONE);

    hdrl_spectrum1D_delete(&spec);

    cpl_array_delete(wavelengths_40);
    cpl_image_delete(spectrum1d_40);
    cpl_image_delete(spectrum1d_error_40);

    cpl_array_delete(wavelengths_42);
    cpl_image_delete(spectrum1d_42);
    cpl_image_delete(spectrum1d_error_42);
}


/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Testing of the HDRL spectrum1D constructors
 */
/*----------------------------------------------------------------------------*/

void test_spectrum1D_duplication(void){

    cpl_image * spectrum1d =
            get_random_1d_img(140, 0.0f, 128.0f, CPL_TYPE_DOUBLE);
    cpl_image * spectrum1d_error =
            get_random_1d_img(140, 0.0f, 1.0f,CPL_TYPE_DOUBLE);
    set_1d_bpm(spectrum1d);
    cpl_array * wavelengths = get_wavelength(140, CPL_TYPE_DOUBLE);

    hdrl_spectrum1D * spec = hdrl_spectrum1D_create(
                spectrum1d, NULL,
                wavelengths, hdrl_spectrum1D_wave_scale_linear);

    hdrl_spectrum1D * spec_copy =  hdrl_spectrum1D_duplicate(spec);

    hdrl_spectrum1D_delete(&spec);
    hdrl_spectrum1D_delete(&spec_copy);

    /*Issues with NULL?*/
    hdrl_spectrum1D * should_be_null = hdrl_spectrum1D_duplicate(NULL);
    cpl_test_null(should_be_null);

    cpl_array_delete(wavelengths);
    cpl_image_delete(spectrum1d);
    cpl_image_delete(spectrum1d_error);
}

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Testing of the HDRL spectrum1D constructors
 */
/*----------------------------------------------------------------------------*/

void test_spectrum1D_calculation_scalar(void){

    cpl_image * spectrum1d =
            get_random_1d_img(40, 0.0f, 128.0f, CPL_TYPE_DOUBLE);
    cpl_image * spectrum1d_error =
            get_random_1d_img(40, 0.0f, 1.0f, CPL_TYPE_DOUBLE);
    set_1d_bpm(spectrum1d);
    cpl_array * wavelengths = get_wavelength(40, CPL_TYPE_DOUBLE);

    cpl_image_set(spectrum1d, 3, 1, 5.0);
    cpl_image_set(spectrum1d_error, 3, 1, 2.1);

    hdrl_spectrum1D * spec = hdrl_spectrum1D_create(
                    spectrum1d, spectrum1d_error,
                    wavelengths, hdrl_spectrum1D_wave_scale_linear);

    hdrl_value vs = {1.5, 0.3};

    hdrl_spectrum1D * spec_copy = hdrl_spectrum1D_mul_scalar_create(spec, vs);
    hdrl_spectrum1D_mul_scalar(spec, vs);

    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec, 2, NULL).data,
            7.5, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec, 2, NULL).error,
            3.4889, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec_copy, 2, NULL).data,
            7.5, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec_copy, 2, NULL).error,
            3.4889, 1e-3);
    hdrl_spectrum1D_delete(&spec_copy);

    spec_copy = hdrl_spectrum1D_div_scalar_create(spec, vs);
    hdrl_spectrum1D_div_scalar(spec, vs);

    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec, 2, NULL).data,
            5.0, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec, 2, NULL).error,
            2.53179, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec_copy , 2, NULL).data,
            5.0, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec_copy , 2, NULL).error,
            2.53179, 1e-3);
    hdrl_spectrum1D_delete(&spec_copy);

    spec_copy = hdrl_spectrum1D_add_scalar_create(spec, vs);
    hdrl_spectrum1D_add_scalar(spec, vs);

    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec, 2, NULL).data,
            6.5, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec, 2, NULL).error,
            2.54951, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec_copy, 2, NULL).data,
            6.5, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec_copy, 2, NULL).error,
            2.54951, 1e-3);
    hdrl_spectrum1D_delete(&spec_copy);

    spec_copy = hdrl_spectrum1D_sub_scalar_create(spec, vs);
    hdrl_spectrum1D_sub_scalar(spec, vs);

    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec, 2, NULL).data,
            5.0, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec, 2, NULL).error,
            2.5671, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec_copy, 2, NULL).data,
            5.0, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec_copy, 2, NULL).error,
            2.5671, 1e-3);
    hdrl_spectrum1D_delete(&spec_copy);

    vs.data = 2.0;
    spec_copy = hdrl_spectrum1D_pow_scalar_create(spec, vs);
    hdrl_spectrum1D_pow_scalar(spec, vs);

    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec, 2, NULL).data,
            pow(5.0, 2.0), 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec, 2, NULL).error,
            28.3673, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec_copy, 2, NULL).data,
            pow(5.0, 2.0), 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec_copy, 2, NULL).error,
            28.3673, 1e-3);
    hdrl_spectrum1D_delete(&spec_copy);

    hdrl_spectrum1D_delete(&spec);
    spec = hdrl_spectrum1D_create(spectrum1d, spectrum1d_error,
                               wavelengths, hdrl_spectrum1D_wave_scale_linear);

    vs.data = 2.0;
    vs.error = 0.2;
    spec_copy = hdrl_spectrum1D_exp_scalar_create(spec, vs);
    hdrl_spectrum1D_exp_scalar(spec, vs);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec, 2, NULL).data,
            pow(2.0, 5.0), 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec, 2, NULL).error,
            49.25087754, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec_copy, 2, NULL).data,
            pow(2.0, 5.0), 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(spec_copy, 2, NULL).error,
            49.25087754, 1e-3);
    hdrl_spectrum1D_delete(&spec_copy);

    cpl_array_delete(wavelengths);
    cpl_image_delete(spectrum1d);
    cpl_image_delete(spectrum1d_error);
    hdrl_spectrum1D_delete(&spec);


    /* Issues with NULL */
    hdrl_value v = {100.0, 5.0};
    cpl_test_null(hdrl_spectrum1D_sub_scalar_create(NULL, v));
    cpl_test_null(hdrl_spectrum1D_add_scalar_create(NULL, v));
    cpl_test_null(hdrl_spectrum1D_div_scalar_create(NULL, v));
    cpl_test_null(hdrl_spectrum1D_mul_scalar_create(NULL, v));
    cpl_test_null(hdrl_spectrum1D_pow_scalar_create(NULL, v));
    cpl_test_null(hdrl_spectrum1D_exp_scalar_create(NULL, v));

    cpl_test_eq(hdrl_spectrum1D_sub_scalar(NULL, v), CPL_ERROR_NONE);
    cpl_test_eq(hdrl_spectrum1D_add_scalar(NULL, v), CPL_ERROR_NONE);
    cpl_test_eq(hdrl_spectrum1D_div_scalar(NULL, v), CPL_ERROR_NONE);
    cpl_test_eq(hdrl_spectrum1D_mul_scalar(NULL, v), CPL_ERROR_NONE);
    cpl_test_eq(hdrl_spectrum1D_pow_scalar(NULL, v), CPL_ERROR_NONE);
    cpl_test_eq(hdrl_spectrum1D_exp_scalar(NULL, v), CPL_ERROR_NONE);
}

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Testing of the HDRL spectrum1D calculation
 */
/*----------------------------------------------------------------------------*/

void test_spectrum1D_calculation(void){

    cpl_image * spectrum1d1 =
                get_random_1d_img(40, 1.0f, 128.0f, CPL_TYPE_DOUBLE);
    cpl_image * spectrum1d2 =
                get_random_1d_img(40, 1.0f, 128.0f, CPL_TYPE_DOUBLE);

    cpl_image * spectrum1d_error1 =
                get_random_1d_img(40, 0.5f, 2.0f, CPL_TYPE_DOUBLE);
    cpl_image * spectrum1d_error2 =
                get_random_1d_img(40, 0.5f, 2.0f, CPL_TYPE_DOUBLE);
    set_1d_bpm(spectrum1d1);
    cpl_array * wavelengths = get_wavelength(40, CPL_TYPE_DOUBLE);

    cpl_image_set(spectrum1d1, 15, 1, 8.0);
    cpl_image_set(spectrum1d2, 15, 1, 4.0);
    cpl_image_set(spectrum1d_error1, 15, 1, 2.0);
    cpl_image_set(spectrum1d_error2, 15, 1, 1.0);

    hdrl_spectrum1D * s1 = hdrl_spectrum1D_create(
            spectrum1d1, spectrum1d_error1,
            wavelengths, hdrl_spectrum1D_wave_scale_linear);

    hdrl_spectrum1D * s2 = hdrl_spectrum1D_create(
            spectrum1d2, spectrum1d_error2,
            wavelengths, hdrl_spectrum1D_wave_scale_linear);

    cpl_test_abs(hdrl_spectrum1D_get_flux_value(s1, 14, NULL).data, 8.0, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(s2, 14, NULL).data, 4.0, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(s1, 14, NULL).error, 2.0, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(s2, 14, NULL).error, 1.0, 1e-3);

    hdrl_spectrum1D * s3 = hdrl_spectrum1D_div_spectrum_create(s1, s2);
    hdrl_spectrum1D * s4 = hdrl_spectrum1D_duplicate(s1);
    hdrl_spectrum1D_div_spectrum(s4, s2);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(s3, 14, NULL).data, 2.0, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(s3, 14, NULL).error,
            0.707107, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(s4, 14, NULL).data, 2.0,1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(s4, 14, NULL).error,
            0.707107, 1e-3);
    hdrl_spectrum1D_delete(&s3);
    hdrl_spectrum1D_delete(&s4);

    s3 = hdrl_spectrum1D_mul_spectrum_create(s1, s2);
    s4 =  hdrl_spectrum1D_duplicate(s1);
    hdrl_spectrum1D_mul_spectrum(s4, s2);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(s3, 14, NULL).data,
            32.0, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(s3, 14, NULL).error,
            11.3137, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(s4, 14, NULL).data,
            32.0, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(s4, 14, NULL).error,
            11.3137, 1e-3);
    hdrl_spectrum1D_delete(&s3);
    hdrl_spectrum1D_delete(&s4);


    s3 = hdrl_spectrum1D_sub_spectrum_create(s1, s2);
    s4 =  hdrl_spectrum1D_duplicate(s1);
    hdrl_spectrum1D_sub_spectrum(s4, s2);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(s3, 14, NULL).data, 4.0, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(s3, 14, NULL).error, 2.23607,
            1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(s4, 14, NULL).data, 4.0, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(s4, 14, NULL).error, 2.23607,
            1e-3);
    hdrl_spectrum1D_delete(&s3);
    hdrl_spectrum1D_delete(&s4);

    s3 = hdrl_spectrum1D_add_spectrum_create(s1, s2);
    s4 =  hdrl_spectrum1D_duplicate(s1);
    hdrl_spectrum1D_add_spectrum(s4, s2);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(s3, 14, NULL).data, 12.0, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(s3, 14, NULL).error, 2.23607,
            1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(s4, 14, NULL).data, 12.0, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(s4, 14, NULL).error, 2.23607,
            1e-3);
    hdrl_spectrum1D_delete(&s3);
    hdrl_spectrum1D_delete(&s4);

    hdrl_spectrum1D_delete(&s1);
    hdrl_spectrum1D_delete(&s2);

    cpl_array_delete(wavelengths);
    cpl_image_delete(spectrum1d1);
    cpl_image_delete(spectrum1d2);
    cpl_image_delete(spectrum1d_error1);
    cpl_image_delete(spectrum1d_error2);
}

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Testing of the HDRL spectrum1D calculation
                               error conditions
 */
/*----------------------------------------------------------------------------*/

void test_spectrum1D_calculation_error(void){

    test_calc_creat_error(hdrl_spectrum1D_div_spectrum_create);
    test_calc_creat_error(hdrl_spectrum1D_add_spectrum_create);
    test_calc_creat_error(hdrl_spectrum1D_sub_spectrum_create);
    test_calc_creat_error(hdrl_spectrum1D_mul_spectrum_create);

    test_calc_error(hdrl_spectrum1D_div_spectrum);
    test_calc_error(hdrl_spectrum1D_add_spectrum);
    test_calc_error(hdrl_spectrum1D_sub_spectrum);
    test_calc_error(hdrl_spectrum1D_mul_spectrum);
}

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Testing of the HDRL spectrum1D wavelength
                               calculation
 */
/*----------------------------------------------------------------------------*/

void test_spectrum1D_conversion_wavelength_scale(void){

    cpl_image * spectrum1d =
                get_random_1d_img(40, 1.0f, 128.0f, CPL_TYPE_DOUBLE);

    cpl_image * spectrum1d_error1 =
                get_random_1d_img(40, 0.5f, 2.0f, CPL_TYPE_DOUBLE);

    set_1d_bpm(spectrum1d);
    cpl_array * wavelengths = get_wavelength(40, CPL_TYPE_DOUBLE);

    hdrl_spectrum1D * sp = hdrl_spectrum1D_create(
                spectrum1d, spectrum1d_error1,
                wavelengths, hdrl_spectrum1D_wave_scale_linear);

    hdrl_data_t w1 = cpl_array_get(wavelengths, 4, NULL);
    hdrl_data_t w2 = cpl_array_get(wavelengths, 6, NULL);

    hdrl_spectrum1D * s_lg =
            hdrl_spectrum1D_wavelength_convert_to_log_create(sp);
    hdrl_spectrum1D_wavelength w_lg =
            hdrl_spectrum1D_get_wavelength(s_lg);

    cpl_test_eq(w_lg.scale, hdrl_spectrum1D_wave_scale_log);

    cpl_test_abs(log(w1), cpl_array_get(w_lg.wavelength, 4, NULL), 1e-3);
    cpl_test_abs(log(w2), cpl_array_get(w_lg.wavelength, 6, NULL), 1e-3);

    hdrl_spectrum1D * s_lin =
            hdrl_spectrum1D_wavelength_convert_to_linear_create(s_lg);
    hdrl_spectrum1D_wavelength w_lin = hdrl_spectrum1D_get_wavelength(s_lin);
    cpl_test_eq(w_lin.scale, hdrl_spectrum1D_wave_scale_linear);

    cpl_test_abs(w1, cpl_array_get(w_lin.wavelength, 4, NULL), 1e-3);
    cpl_test_abs(w2, cpl_array_get(w_lin.wavelength, 6, NULL), 1e-3);

   /*conversions on functions already in the right scale should behave like a
    * duplicate*/

    hdrl_spectrum1D * s_lg2 =
            hdrl_spectrum1D_wavelength_convert_to_log_create(s_lg);
    hdrl_spectrum1D_wavelength w_lg2 = hdrl_spectrum1D_get_wavelength(s_lg);
    cpl_test_eq(w_lg2.scale, hdrl_spectrum1D_wave_scale_log);
    cpl_test_noneq_ptr(s_lg2, s_lg);

    hdrl_spectrum1D * s_lin2 =
            hdrl_spectrum1D_wavelength_convert_to_log_create(s_lin);
    hdrl_spectrum1D_wavelength w_lin2 = hdrl_spectrum1D_get_wavelength(s_lin);
    cpl_test_eq(w_lin2.scale, hdrl_spectrum1D_wave_scale_linear);
    cpl_test_noneq_ptr(s_lin2, s_lin);

    hdrl_spectrum1D_delete(&s_lg);
    hdrl_spectrum1D_delete(&s_lg2);
    hdrl_spectrum1D_delete(&s_lin);
    hdrl_spectrum1D_delete(&s_lin2);

    /* Mutator functions */

    /* linear -> log */
    cpl_error_code e = hdrl_spectrum1D_wavelength_convert_to_log(sp);
    cpl_test_eq(e, CPL_ERROR_NONE);

    hdrl_spectrum1D_wavelength sp_w_lg = hdrl_spectrum1D_get_wavelength(sp);

    cpl_test_eq(sp_w_lg.scale, hdrl_spectrum1D_wave_scale_log);
    cpl_test_abs(log(w1), cpl_array_get(sp_w_lg.wavelength, 4, NULL), 1e-3);
    cpl_test_abs(log(w2), cpl_array_get(sp_w_lg.wavelength, 6, NULL), 1e-3);

    /* log -> log NOOP*/
    e = hdrl_spectrum1D_wavelength_convert_to_log(sp);
    cpl_test_eq(e, CPL_ERROR_NONE);

    sp_w_lg = hdrl_spectrum1D_get_wavelength(sp);

    cpl_test_eq(sp_w_lg.scale, hdrl_spectrum1D_wave_scale_log);
    cpl_test_abs(log(w1), cpl_array_get(sp_w_lg.wavelength, 4, NULL), 1e-3);
    cpl_test_abs(log(w2), cpl_array_get(sp_w_lg.wavelength, 6, NULL), 1e-3);

    /* log -> linear */
    e = hdrl_spectrum1D_wavelength_convert_to_linear(sp);
    cpl_test_eq(e, CPL_ERROR_NONE);

    hdrl_spectrum1D_wavelength sp_w_lin = hdrl_spectrum1D_get_wavelength(sp);
    cpl_test_eq(sp_w_lin.scale, hdrl_spectrum1D_wave_scale_linear);
    cpl_test_abs(w1, cpl_array_get(sp_w_lin.wavelength, 4, NULL), 1e-3);
    cpl_test_abs(w2, cpl_array_get(sp_w_lin.wavelength, 6, NULL), 1e-3);

    /* linear -> linear NOOP*/
    e = hdrl_spectrum1D_wavelength_convert_to_linear(sp);
    cpl_test_eq(e, CPL_ERROR_NONE);

    sp_w_lin = hdrl_spectrum1D_get_wavelength(sp);
    cpl_test_eq(sp_w_lin.scale, hdrl_spectrum1D_wave_scale_linear);
    cpl_test_abs(w1, cpl_array_get(sp_w_lin.wavelength, 4, NULL), 1e-3);
    cpl_test_abs(w2, cpl_array_get(sp_w_lin.wavelength, 6, NULL), 1e-3);

    hdrl_spectrum1D_delete(&sp);
    cpl_array_delete(wavelengths);
    cpl_image_delete(spectrum1d_error1);
    cpl_image_delete(spectrum1d);

    /* Test for null */

    cpl_test_eq(hdrl_spectrum1D_wavelength_convert_to_linear(NULL),
            CPL_ERROR_NONE);
    cpl_test_eq(hdrl_spectrum1D_wavelength_convert_to_log(NULL),
            CPL_ERROR_NONE);

    cpl_test_null(hdrl_spectrum1D_wavelength_convert_to_linear_create(NULL));
    cpl_test_null(hdrl_spectrum1D_wavelength_convert_to_log_create(NULL));
}

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Testing of the HDRL spectrum1D wavelength
                               calculation
 */
/*----------------------------------------------------------------------------*/

void test_spectrum1D_mul_wavelength(void){

    cpl_image * spectrum1d =
                get_random_1d_img(40, 1.0f, 128.0f, CPL_TYPE_DOUBLE);

    cpl_image * spectrum1d_error1 =
                get_random_1d_img(40, 0.5f, 2.0f, CPL_TYPE_DOUBLE);

    set_1d_bpm(spectrum1d);
    cpl_array * wavelengths = get_wavelength(40, CPL_TYPE_DOUBLE);

    hdrl_spectrum1D * sp = hdrl_spectrum1D_create(
                spectrum1d, spectrum1d_error1,
                wavelengths, hdrl_spectrum1D_wave_scale_linear);

    hdrl_data_t w1 = cpl_array_get(wavelengths, 4, NULL);
    hdrl_data_t w2 = cpl_array_get(wavelengths, 6, NULL);

    /* mutator functions */

    cpl_error_code e = hdrl_spectrum1D_wavelength_mult_scalar_linear(sp, 1e3);
    cpl_test_eq(e, CPL_ERROR_NONE);

    hdrl_spectrum1D_wavelength sp_mul_1e3 = hdrl_spectrum1D_get_wavelength(sp);

    cpl_test_abs(w1 * 1e3, cpl_array_get(sp_mul_1e3.wavelength, 4, NULL), 1e-3);
    cpl_test_abs(w2 * 1e3, cpl_array_get(sp_mul_1e3.wavelength, 6, NULL), 1e-3);

    e = hdrl_spectrum1D_wavelength_mult_scalar_linear(sp, 1e-3);
    cpl_test_eq(e, CPL_ERROR_NONE);

    hdrl_spectrum1D_wavelength_convert_to_log(sp);

    e = hdrl_spectrum1D_wavelength_mult_scalar_linear(sp, 1e3);
    cpl_test_eq(e, CPL_ERROR_NONE);

    hdrl_spectrum1D_wavelength sp_mul_log = hdrl_spectrum1D_get_wavelength(sp);

    cpl_test_abs(log(w1 * 1e3), cpl_array_get(sp_mul_log.wavelength, 4, NULL),
            1e-3);
    cpl_test_abs(log(w2 * 1e3), cpl_array_get(sp_mul_log.wavelength, 6, NULL),
            1e-3);

    /* non mutator functions */
    hdrl_spectrum1D_delete(&sp);

    sp = hdrl_spectrum1D_create(
                    spectrum1d, spectrum1d_error1,
                    wavelengths, hdrl_spectrum1D_wave_scale_linear);

    hdrl_spectrum1D * sp_lin =
            hdrl_spectrum1D_wavelength_mult_scalar_linear_create(sp, 1e-4);
    hdrl_spectrum1D_wavelength_convert_to_log(sp);
    hdrl_spectrum1D * sp_log =
            hdrl_spectrum1D_wavelength_mult_scalar_linear_create(sp, 1e-4);
    hdrl_spectrum1D_delete(&sp);

    sp_mul_1e3 = hdrl_spectrum1D_get_wavelength(sp_lin);
    sp_mul_log = hdrl_spectrum1D_get_wavelength(sp_log);

    cpl_test_abs(w1 * 1e-4, cpl_array_get(sp_mul_1e3.wavelength, 4, NULL), 1e-6);
    cpl_test_abs(w2 * 1e-4, cpl_array_get(sp_mul_1e3.wavelength, 6, NULL), 1e-6);

    cpl_test_abs(
            log(w1 * 1e-4), cpl_array_get(sp_mul_log.wavelength, 4, NULL), 1e-6);
    cpl_test_abs(
            log(w2 * 1e-4), cpl_array_get(sp_mul_log.wavelength, 6, NULL), 1e-6);

    cpl_test_noneq(hdrl_spectrum1D_wavelength_mult_scalar_linear(sp_log, -2.0),
            CPL_ERROR_NONE);
    cpl_test_noneq(get_error_code_and_reset(), CPL_ERROR_NONE);

    cpl_test_null(
            hdrl_spectrum1D_wavelength_mult_scalar_linear_create(sp_log, -2.0));
    cpl_test_noneq(get_error_code_and_reset(), CPL_ERROR_NONE);

    hdrl_spectrum1D_delete(&sp_log);
    hdrl_spectrum1D_delete(&sp_lin);
    cpl_array_delete(wavelengths);
    cpl_image_delete(spectrum1d_error1);
    cpl_image_delete(spectrum1d);

    cpl_test_eq(hdrl_spectrum1D_wavelength_mult_scalar_linear(NULL, 3.0),
            CPL_ERROR_NONE);
    cpl_test_null(hdrl_spectrum1D_wavelength_mult_scalar_linear_create(NULL,
            3.0));
}

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Testing of the HDRL spectrum1D wavelength shift
 */
/*----------------------------------------------------------------------------*/

void test_spectrum1D_shift_wavelength(void){

    cpl_image * spectrum1d =
                get_random_1d_img(40, 1.0f, 128.0f, CPL_TYPE_DOUBLE);

    cpl_image * spectrum1d_error1 =
                get_random_1d_img(40, 0.5f, 2.0f, CPL_TYPE_DOUBLE);

    set_1d_bpm(spectrum1d);
    cpl_array * wavelengths = get_wavelength(40, CPL_TYPE_DOUBLE);

    hdrl_data_t w1 = cpl_array_get(wavelengths, 4, NULL);
    hdrl_data_t w2 = cpl_array_get(wavelengths, 6, NULL);

    hdrl_spectrum1D * sp1 = hdrl_spectrum1D_create(
                spectrum1d, spectrum1d_error1,
                wavelengths, hdrl_spectrum1D_wave_scale_linear);

    hdrl_spectrum1D * sp2 = hdrl_spectrum1D_create(
                spectrum1d, spectrum1d_error1,
                wavelengths, hdrl_spectrum1D_wave_scale_log);

    /* shift works in the same way both for linear and log scale */
    cpl_error_code err = hdrl_spectrum1D_wavelength_shift(sp1, 3.0);
    cpl_test_eq(err, CPL_ERROR_NONE);

    err = hdrl_spectrum1D_wavelength_shift(sp2, -3.0);
    cpl_test_eq(err, CPL_ERROR_NONE);

    hdrl_spectrum1D_wavelength sp_p_3 = hdrl_spectrum1D_get_wavelength(sp1);
    hdrl_spectrum1D_wavelength sp_m_3 = hdrl_spectrum1D_get_wavelength(sp2);

    cpl_test_abs(w1 + 3.0, cpl_array_get(sp_p_3.wavelength, 4, NULL), 1e-3);
    cpl_test_abs(w2 + 3.0, cpl_array_get(sp_p_3.wavelength, 6, NULL), 1e-3);

    cpl_test_abs(w1 - 3.0, cpl_array_get(sp_m_3.wavelength, 4, NULL), 1e-3);
    cpl_test_abs(w2 - 3.0, cpl_array_get(sp_m_3.wavelength, 6, NULL), 1e-3);

    hdrl_spectrum1D* sp_new_1 =
            hdrl_spectrum1D_wavelength_shift_create(sp1, -3.0);
    hdrl_spectrum1D* sp_new_2 =
            hdrl_spectrum1D_wavelength_shift_create(sp2, 3.0);

    hdrl_spectrum1D_delete(&sp1);
    hdrl_spectrum1D_delete(&sp2);

    hdrl_spectrum1D_wavelength sp_n_1_w =
            hdrl_spectrum1D_get_wavelength(sp_new_1);
    hdrl_spectrum1D_wavelength sp_n_2_w =
            hdrl_spectrum1D_get_wavelength(sp_new_2);

    cpl_test_abs(w1, cpl_array_get(sp_n_1_w.wavelength, 4, NULL), 1e-3);
    cpl_test_abs(w2, cpl_array_get(sp_n_1_w.wavelength, 6, NULL), 1e-3);
    cpl_test_abs(w1, cpl_array_get(sp_n_2_w.wavelength, 4, NULL), 1e-3);
    cpl_test_abs(w2, cpl_array_get(sp_n_2_w.wavelength, 6, NULL), 1e-3);

    hdrl_spectrum1D_delete(&sp_new_1);
    hdrl_spectrum1D_delete(&sp_new_2);

    /* Issues with NULL? */
    err = hdrl_spectrum1D_wavelength_shift(NULL, 3.0);
    cpl_test_eq(err, CPL_ERROR_NONE);
    hdrl_spectrum1D * should_be_null =
            hdrl_spectrum1D_wavelength_shift_create(NULL, 3.0);
    cpl_test_null(should_be_null);

    cpl_array_delete(wavelengths);
    cpl_image_delete(spectrum1d_error1);
    cpl_image_delete(spectrum1d);
}

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Testing of the HDRL spectrum1D table conversions
 */
/*----------------------------------------------------------------------------*/
void test_spectrum1D_table_conversion(void){

    const cpl_size sz_ori = 17;
    cpl_array * unshuffled_lambda = NULL;
    hdrl_spectrum1D * sp1 = get_spectrum1D_sin_shuffled(sz_ori, 2, CPL_TRUE,
            &unshuffled_lambda);
    cpl_array_delete(unshuffled_lambda);

    cpl_table * tab = hdrl_spectrum1D_convert_to_table(sp1, "flux",
            "lambdas", "flux_e", "flux_bpm");

    /*ad rejected pixels*/
    cpl_table_set_int(tab, "flux_bpm", 0, 1);
    cpl_table_set_int(tab, "flux_bpm", sz_ori - 1, 1);

    hdrl_spectrum1D * sp2 = hdrl_spectrum1D_convert_from_table
            (tab, "flux", "lambdas", "flux_e", "flux_bpm",
                    hdrl_spectrum1D_wave_scale_linear);

    const cpl_image * flux1 =
            hdrl_image_get_image_const(hdrl_spectrum1D_get_flux(sp1));
    const cpl_image * flux1_e =
                hdrl_image_get_error_const(hdrl_spectrum1D_get_flux(sp1));

    const cpl_array * lambdas1 = hdrl_spectrum1D_get_wavelength(sp1).wavelength;

    const cpl_image * flux2 =
            hdrl_image_get_image_const(hdrl_spectrum1D_get_flux(sp2));
    const cpl_image * flux2_e =
            hdrl_image_get_error_const(hdrl_spectrum1D_get_flux(sp2));
    const cpl_array * lambdas2 = hdrl_spectrum1D_get_wavelength(sp2).wavelength;

    const cpl_size sz_x = cpl_image_get_size_x(flux1);
    const cpl_size sz_y = cpl_image_get_size_y(flux1);
    const cpl_size sz = cpl_array_get_size(lambdas1);

    cpl_test_eq(sz_x, cpl_image_get_size_x(flux2));
    cpl_test_eq(sz_y, cpl_image_get_size_y(flux2));
    cpl_test_eq(sz, cpl_array_get_size(lambdas2));

    for(cpl_size i = 1; i < sz - 1; i++){

        int rej1 = 0;
        int rej2 = 0;
        const double flx1 = cpl_image_get(flux1, i + 1, 1, &rej1);
        const double flx2 = cpl_image_get(flux2, i + 1, 1, &rej2);


        cpl_test_abs(flx1, flx2, 1e-3);
        cpl_test_eq(rej1, rej2);
        rej1 = rej2 = 0;

        const double flx1_e = cpl_image_get(flux1_e, i + 1, 1, &rej1);
        const double flx2_e = cpl_image_get(flux2_e, i + 1, 1, &rej2);

        cpl_test_abs(flx1_e, flx2_e, 1e-3);
        cpl_test_eq(rej1, rej2);
        rej1 = rej2 = 0;


        const double l1 = cpl_array_get(lambdas1, i, &rej1);
        const double l2 = cpl_array_get(lambdas2, i, &rej2);

        cpl_test_abs(l1, l2, 1e-3);
        cpl_test_eq(rej1, rej2);
    }

    cpl_test(cpl_image_is_rejected(flux2, 1, 1));
    cpl_test(cpl_image_is_rejected(flux2, sz_ori, 1));


    hdrl_spectrum1D_append_to_table(sp2, tab,
            "flux2", "lambdas2", NULL, "flux2_bpm");

    for(cpl_size i = 0; i < sz; i++){

        int rej = 0;

        const int bpm1 = cpl_table_get_int(tab, "flux_bpm", i, &rej);
        const int bpm2 = cpl_table_get_int(tab, "flux2_bpm", i, &rej);
        cpl_test_eq(bpm1, bpm2);

        if(bpm1) continue;

        const double flx1 = cpl_table_get(tab, "flux", i, &rej);
        const double flx2 = cpl_table_get(tab, "flux2", i, &rej);
        cpl_test_abs(flx1, flx2, 1e-3);

        const double l1 = cpl_table_get(tab, "lambdas", i, &rej);
        const double l2 = cpl_table_get(tab, "lambdas2", i, &rej);
        cpl_test_abs(l1, l2, 1e-3);
    }

    hdrl_spectrum1D_delete(&sp2);
    sp2 = hdrl_spectrum1D_convert_from_table
              (tab, "flux", "lambdas", NULL, NULL,
                      hdrl_spectrum1D_wave_scale_linear);

    /* Check save */
    const char* filename = "check_spectrum1D.fits";
    hdrl_spectrum1D_save(sp2, filename);
    remove(filename);

    for(cpl_size i = 0; i < hdrl_spectrum1D_get_size(sp2); ++i){
        int rej;

        const double fx1 = hdrl_spectrum1D_get_flux_value(sp1, i, &rej).data;

        const double fx2 = hdrl_spectrum1D_get_flux_value(sp2, i, &rej).data;
        const double fx2_e = hdrl_spectrum1D_get_flux_value(sp2, i, &rej).error;

        cpl_test_eq(fx2_e, 0.0);
        cpl_test_abs(fx1, fx2, 1e-3);
    }

    hdrl_spectrum1D_delete(&sp2);
    cpl_table_delete(tab);

    cpl_table * f_only =
            hdrl_spectrum1D_convert_to_table(sp1, "flux", NULL, NULL, NULL);
    cpl_table * l_only =
            hdrl_spectrum1D_convert_to_table(sp1, NULL, "wav", NULL, NULL);
    cpl_table * f_and_e =
            hdrl_spectrum1D_convert_to_table(sp1, "flux", NULL, "error", NULL);

    for(cpl_size i = 0; i < hdrl_spectrum1D_get_size(sp1); ++i){
        const double f = hdrl_spectrum1D_get_flux_value(sp1, i, NULL).data;
        const double e = hdrl_spectrum1D_get_flux_value(sp1, i, NULL).error;
        const double w = hdrl_spectrum1D_get_wavelength_value(sp1, i, NULL);

        const double f1 = cpl_table_get(f_only, "flux", i, NULL);
        const double f2 = cpl_table_get(f_and_e, "flux", i, NULL);
        cpl_test_eq(f, f1);
        cpl_test_eq(f, f2);

        const double w1 = cpl_table_get(l_only, "wav", i, NULL);
        cpl_test_eq(w, w1);

        const double e1 = cpl_table_get(f_and_e, "error", i, NULL);
        cpl_test_eq(e, e1);
    }


    hdrl_spectrum1D_delete(&sp1);
    cpl_table_delete(f_only);
    cpl_table_delete(l_only);
    cpl_table_delete(f_and_e);
}
/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Testing of the HDRL spectrum1D resample
 */
/*----------------------------------------------------------------------------*/

void test_spectrum1D_resample_spectrum(cpl_boolean add_peak){

	/* Initialize variables */
    cpl_array * unshuffled_lambda = NULL;
    hdrl_spectrum1D * sp = get_spectrum1D_sin_shuffled(17, 2, add_peak, &unshuffled_lambda);

    hdrl_spectrum1D_wavelength wl = hdrl_spectrum1D_get_wavelength(sp);
    cpl_size sz = hdrl_spectrum1D_get_size(sp);
    cpl_array * new_lambda = cpl_array_new(sz, HDRL_TYPE_DATA);
    for(cpl_size i = 0; i < sz; i++){
        double d = cpl_array_get(unshuffled_lambda, i, NULL)
                + cpl_array_get(unshuffled_lambda, CPL_MIN(sz - 1, i + 1), NULL);
        cpl_array_set(new_lambda, i, d/2);
    }

    wl.wavelength = new_lambda;


    /* Tests variables */
    int rej;
    hdrl_parameter  *pars;
    hdrl_spectrum1D *resampled_lambda;


    /* Test 1 */
    pars             = hdrl_spectrum1D_resample_fit_parameter_create(4, 17);
    resampled_lambda = hdrl_spectrum1D_resample(sp, &wl, pars);

    const double data2_fit = add_peak ? 116.368 : 209.577;
    const double data3_fit = add_peak ? 303.376 : 199.524;

    cpl_test_abs(hdrl_spectrum1D_get_flux_value(resampled_lambda, 2, &rej).data, data2_fit, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(resampled_lambda, 3, &rej).data, data3_fit, 1e-3);

    hdrl_parameter_delete(pars);
    hdrl_spectrum1D_delete(&resampled_lambda);


    /* Test 2 */
    pars             = hdrl_spectrum1D_resample_interpolate_parameter_create(hdrl_spectrum1D_interp_akima);
    resampled_lambda = hdrl_spectrum1D_resample(sp, &wl, pars);

    const double data2_interp = add_peak ? 208.699 : 209.65;
    const double data3_interp = add_peak ? 247.949 : 199.585;

    cpl_test_abs(hdrl_spectrum1D_get_flux_value(resampled_lambda, 2, &rej).data, data2_interp, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(resampled_lambda, 3, &rej).data, data3_interp, 1e-3);

    hdrl_parameter_delete(pars);
    hdrl_spectrum1D_delete(&resampled_lambda);


    /* Test 3 */
    pars             = hdrl_spectrum1D_resample_integrate_parameter_create();
    resampled_lambda = hdrl_spectrum1D_resample(sp, &wl, pars);

    const double data2_integrate = 207.878;
    const double data3_integrate = add_peak ? 245.443 : 197.992;

    cpl_test_abs(hdrl_spectrum1D_get_flux_value(resampled_lambda, 2, &rej).data, data2_integrate, 1e-3);
    cpl_test_abs(hdrl_spectrum1D_get_flux_value(resampled_lambda, 3, &rej).data, data3_integrate, 1e-3);

    hdrl_parameter_delete(pars);
    hdrl_spectrum1D_delete(&resampled_lambda);


    /* Clean up */
    hdrl_spectrum1D_delete(&sp);
    cpl_array_delete(new_lambda);
    cpl_array_delete(unshuffled_lambda);
}

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Testing of the HDRL spectrum1D resample private
 *                             functions
 */
/*----------------------------------------------------------------------------*/

void test_spectrum1D_resample_spectrum_private_funcs(void){

	double testValue1 = 2.1;
	double testValue2 = 3.5;
	double testValue3 = 5.5;

    {
        double x[]  = {3, 2.1, 5.5, 8.7, 3.3, 5.6, 2.1};
        double y1[] = {11, 88, -22,  56,   4, 22,   23};
        double y2[] = { 2, 55,   2,  27,  23,  1,   5};

        double x_sorted[]  = {2.1, 2.1, 3, 3.3, 5.5, 5.6, 8.7};
        double y1_sorted[] = {88,  23, 11,   4, -22,  22,  56};
        double y2_sorted[] = {55,  5,   2,  23,   2,   1,  27};

        const cpl_size l = LENGTH_ARRAY(x);

        hdrl_sort_on_x(x, y1, y2, l, CPL_FALSE);

        cpl_test_eq(x[0], testValue1);
        cpl_test_eq(x[1], testValue1);

        /*test the case were x is duplicated.*/
        cpl_test(y1[1] != y1[0]);
        cpl_test(y2[1] != y2[0]);

        for(cpl_size i = 2; i < l; ++i){
            cpl_test_eq(x[i], x_sorted[i]);
            cpl_test_eq(y1[i], y1_sorted[i]);
            cpl_test_eq(y2[i], y2_sorted[i]);
        }
    }

    {
        double x[]  = {3, 2.1, 5.5, 8.7, 3.3, 5.6, 2.1};
        double y2[] = { 2, 55,   2,  27,  23,  1,   5};

        double x_sorted[]  = {2.1, 2.1, 3, 3.3, 5.5, 5.6, 8.7};
        double y2_sorted[] = {55,  5,   2,  23,   2,   1,  27};

        const cpl_size l = LENGTH_ARRAY(x);

        hdrl_sort_on_x(x, NULL, y2, l, CPL_FALSE);

        cpl_test_eq(x[0], testValue1);
        cpl_test_eq(x[1], testValue1);

        cpl_test(y2[1] != y2[0]);

        for(cpl_size i = 2; i < l; ++i){
            cpl_test_eq(x[i], x_sorted[i]);
            cpl_test_eq(y2[i], y2_sorted[i]);
        }
    }

    {
            double x[]  = {3, 2.1, 5.5, 8.7, 3.3, 5.6, 2.1};
            double y1[] = {11, 88, -22,  56,   4, 22,   23};

            double x_sorted[]  = {2.1, 2.1, 3, 3.3, 5.5, 5.6, 8.7};
            double y1_sorted[] = {88,  23, 11,   4, -22,  22,  56};

            const cpl_size l = LENGTH_ARRAY(x);

            hdrl_sort_on_x(x, y1, NULL, l, CPL_FALSE);

            cpl_test_eq(x[0], testValue1);
            cpl_test_eq(x[1], testValue1);

            /*test the case were x is duplicated.*/
            cpl_test(y1[1] != y1[0]);

            for(cpl_size i = 2; i < l; ++i){
                cpl_test_eq(x[i], x_sorted[i]);
                cpl_test_eq(y1[i], y1_sorted[i]);
            }
        }

    /* test duplicates search and median */
    {
        /* edge case, all x are equal, even number of samples*/
        double x[]  = {1,1,1,1,1};
        double y1[] = {5,4,3,2,5};
        double y2[] = {8,7,5,2,6};

        cpl_size l = LENGTH_ARRAY(x);

        l = hdrl_spectrum1D_resample_filter_dups_and_substitute_with_median
                (x, y1, y2, l);

        cpl_test_eq(l, 1);
        cpl_test_eq(x[0],  1);
        cpl_test_eq(y1[0], 4);
        cpl_test_eq(y2[0], 6);
    }

    {
        /* edge case, all x are equal, odd number of samples*/
        double x[]  = {1,1,1,1,1,1};
        double y1[] = {5,4,3,2,5,2.5};
        double y2[] = {8,7,5,2,6,4.6};

        cpl_size l = LENGTH_ARRAY(x);

        l = hdrl_spectrum1D_resample_filter_dups_and_substitute_with_median
                (x, y1, y2, l);

        cpl_test_eq(l, 1);
        cpl_test_eq(x[0],  1);
        cpl_test_eq(y1[0], testValue2);
        cpl_test_eq(y2[0], testValue3);
    }

    /* test duplicates search and median */
    {
        /* edge case, all x are equal, except the first, even number of samples*/
        double x[]  = {1,2,2,2,2,2};
        double y1[] = {55,5,4,3,2,5};
        double y2[] = {88,8,7,5,2,6};

        cpl_size l = LENGTH_ARRAY(x);

        l = hdrl_spectrum1D_resample_filter_dups_and_substitute_with_median
                (x, y1, y2, l);

        cpl_test_eq(l, 2);
        cpl_test_eq(x[1],  2);
        cpl_test_eq(y1[1], 4);
        cpl_test_eq(y2[1], 6);
        cpl_test_eq(x[0],  1);
        cpl_test_eq(y1[0], 55);
        cpl_test_eq(y2[0], 88);
    }

    {
        /* edge case, all x are equal, except the last, odd number of samples*/
        double x[]  = {1, 1, 1, 1, 1,   1,  8};
        double y1[] = {5, 4, 3, 2, 5, 2.5, 77};
        double y2[] = {8, 7, 5, 2, 6, 4.6, 96};

        cpl_size l = LENGTH_ARRAY(x);

        l = hdrl_spectrum1D_resample_filter_dups_and_substitute_with_median
                (x, y1, y2, l);

        cpl_test_eq(l, 2);
        cpl_test_eq(x[0] ,  1);
        cpl_test_eq(y1[0], testValue2);
        cpl_test_eq(y2[0], testValue3);

        cpl_test_eq(x[1] ,  8);
        cpl_test_eq(y1[1], 77);
        cpl_test_eq(y2[1], 96);
    }

    /*more realist case with hunks of equal elements*/
    {
        {
            double x[]  = {1, 2, 2, 3, 3, 3, 5, 6, 7, 7, 8, 9, 10, 10, 10, 11};
            double y1[] = {4, 3, 7, 8, 9, 4, 3, 7, 2, 4, 5, 2,  8,  7,  1, 12};
            double y2[] = {3, 6, 7, 8, 4, 5, 8, 3, 5, 1, 3, 8, 44, 33, 55, 45};

            double x_f[]  = {1, 2,   3, 5, 6, 7, 8, 9, 10, 11};
            double y1_f[] = {4, 5,   8, 3, 7, 3, 5, 2,  7, 12};
            double y2_f[] = {3, 6.5, 5, 8, 3, 3, 3, 8, 44, 45};

            cpl_size l = LENGTH_ARRAY(x);

            l = hdrl_spectrum1D_resample_filter_dups_and_substitute_with_median
                    (x, y1, y2, l);

            cpl_test_eq(l, 10);

            for(cpl_size i = 0; i < l; ++i){
                cpl_test_eq(x[i],   x_f[i]);
                cpl_test_eq(y1[i], y1_f[i]);
                cpl_test_eq(y2[i], y2_f[i]);
            }
        }
    }

    /*more realist case with hunks of equal elements, one hunk at the end*/
    {
        {
            double x[]  = {1, 2, 2, 3, 3, 3, 5, 6, 7, 7, 8, 9, 10, 10, 10, 11, 11};
            double y1[] = {4, 3, 7, 8, 9, 4, 3, 7, 2, 4, 5, 2,  8,  7,  1, 12, 2};
            double y2[] = {3, 6, 7, 8, 4, 5, 8, 3, 5, 1, 3, 8, 44, 33, 55, 45, 5};

            double x_f[]  = {1, 2,   3, 5, 6, 7, 8, 9, 10, 11};
            double y1_f[] = {4, 5,   8, 3, 7, 3, 5, 2,  7,  7};
            double y2_f[] = {3, 6.5, 5, 8, 3, 3, 3, 8, 44, 25};

            cpl_size l = LENGTH_ARRAY(x);

            l = hdrl_spectrum1D_resample_filter_dups_and_substitute_with_median
                    (x, y1, y2, l);

            cpl_test_eq(l, 10);

            for(cpl_size i = 0; i < l; ++i){
                cpl_test_eq(x[i],   x_f[i]);
                cpl_test_eq(y1[i], y1_f[i]);
                cpl_test_eq(y2[i], y2_f[i]);
            }
        }
    }

    /*more realist case with hunks of equal elements, one hunk at the beginning*/
    {
        {
            double x[]  = {1, 1, 2, 2, 3, 3, 3, 5, 6, 7, 7, 8, 9, 10, 10, 10, 11};
            double y1[] = {5, 7, 3, 7, 8, 9, 4, 3, 7, 2, 4, 5, 2,  8,  7,  1, 12};
            double y2[] = {1, 3, 6, 7, 8, 4, 5, 8, 3, 5, 1, 3, 8, 44, 33, 55, 45};

            double x_f[]  = {1, 2,   3, 5, 6, 7, 8, 9, 10, 11};
            double y1_f[] = {6, 5,   8, 3, 7, 3, 5, 2,  7, 12};
            double y2_f[] = {2, 6.5, 5, 8, 3, 3, 3, 8, 44, 45};

            cpl_size l = LENGTH_ARRAY(x);

            l = hdrl_spectrum1D_resample_filter_dups_and_substitute_with_median
                    (x, y1, y2, l);

            cpl_test_eq(l, 10);

            for(cpl_size i = 0; i < l; ++i){
                cpl_test_eq(x[i],   x_f[i]);
                cpl_test_eq(y1[i], y1_f[i]);
                cpl_test_eq(y2[i], y2_f[i]);
            }
        }
    }
}

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Test error calculation for resample based on
 *                             interpolation
 */
/*----------------------------------------------------------------------------*/
void test_spectrum1D_resample_spectrum_interpolation_error_test(void){

    double y[]   = { 0,  1,  2,  1,  0, -1, -2, -1,  0,  1,   2,    1,   0,  -1};
    double y_e[] = {.1, .2, .3, .4, .5, .6, .7, .8, .9,  1, 1.1,  1.2, 1.3, 1.4};
    double x[]   = { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10,  11,   12,  13,  14};

    double x_r[] =
    {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.9, 11.1, 12.2, 13.9};

    cpl_size closer_idx[] =
    {0,     1,   2,   3,   4,   5,   6,   7,   8,   9,   10,   11,   12};

    const cpl_size l = LENGTH_ARRAY(y);
    const cpl_size l2 = LENGTH_ARRAY(x_r);
    cpl_test_eq(LENGTH_ARRAY(y), LENGTH_ARRAY(y_e));
    cpl_test_eq(LENGTH_ARRAY(y), LENGTH_ARRAY(x));

    cpl_image * flux = cpl_image_new(l, 1, CPL_TYPE_DOUBLE);
    cpl_image * flux_e = cpl_image_new(l, 1, CPL_TYPE_DOUBLE);
    cpl_array * wavelengths = cpl_array_new(l, CPL_TYPE_DOUBLE);
    cpl_array * wavelengths_resamp = cpl_array_new(l2, CPL_TYPE_DOUBLE);

    for(cpl_size i = 0; i < l; ++i){
        cpl_image_set(flux, i + 1, 1, y[i]);
        cpl_image_set(flux_e, i + 1, 1, y_e[i]);
        cpl_array_set(wavelengths, i, x[i]);
    }

    for(cpl_size i = 0; i < l2; ++i){
        cpl_array_set(wavelengths_resamp, i, x_r[i]);
    }

    hdrl_spectrum1D * sp1 = hdrl_spectrum1D_create(flux, flux_e, wavelengths,
            hdrl_spectrum1D_wave_scale_linear);

    cpl_array_delete(wavelengths);
    cpl_image_delete(flux);
    cpl_image_delete(flux_e);

    hdrl_spectrum1D_wavelength wl = hdrl_spectrum1D_get_wavelength(sp1);

    wl.wavelength = wavelengths_resamp;
    wl.bpm = NULL;

    hdrl_parameter * pars = hdrl_spectrum1D_resample_interpolate_parameter_create
            (hdrl_spectrum1D_interp_akima);

    hdrl_spectrum1D * sp2 = hdrl_spectrum1D_resample(sp1, &wl, pars);

    hdrl_parameter_delete(pars); pars = NULL;

    /*check that for each position in x_r[i] we linearly interpolate the noise*/
    for(cpl_size i = 0; i < l2; ++i){

        hdrl_error_t err = hdrl_spectrum1D_get_flux_value(sp2, i, NULL).error;
        const cpl_size idx = closer_idx[i];
        hdrl_error_t err_ori = pow(y_e[idx], 2.0) * fabs(x[idx + 1] - x_r[i]) +
                pow(y_e[idx + 1], 2.0) * fabs(x[idx] - x_r[i]);

        err_ori = sqrt(err_ori);

        cpl_test_abs(err, err_ori, HDRL_DELTA_COMPARE_VALUE_ABS);
        hdrl_data_t w = hdrl_spectrum1D_get_wavelength_value(sp2, i, NULL);
        cpl_test_abs(w, x_r[i], HDRL_DELTA_COMPARE_VALUE_ABS);
    }

    hdrl_spectrum1D_delete(&sp1);
    hdrl_spectrum1D_delete(&sp2);
    cpl_array_delete(wavelengths_resamp);
}

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Test error calculation for resample based on
 *                             interpolation
 */
/*----------------------------------------------------------------------------*/
void test_spectrum1D_resample_spectrum_fit_error_test_error_interpol(void){

    double y[]   = { 0,  1,  2,  1,  0, -1, -2, -1,  0,  1,   2,    1,   0,  -1};
    double y_e[] = {.1, .2, .3, .4, .5, .6, .7, .8, .9,  1, 1.1,  1.2, 1.3, 1.4};
    double x[]   = { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10,  11,   12,  13,  14};

    double x_r[] =
    {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.9, 11.1, 12.2, 13.9};

    cpl_size closer_idx[] =
    {0,     1,   2,   3,   4,   5,   6,   7,   8,   9,   10,   11,   12};

    const cpl_size l = LENGTH_ARRAY(y);
    const cpl_size l2 = LENGTH_ARRAY(x_r);
    cpl_test_eq(LENGTH_ARRAY(y), LENGTH_ARRAY(y_e));
    cpl_test_eq(LENGTH_ARRAY(y), LENGTH_ARRAY(x));

    cpl_image * flux = cpl_image_new(l, 1, CPL_TYPE_DOUBLE);
    cpl_image * flux_e = cpl_image_new(l, 1, CPL_TYPE_DOUBLE);
    cpl_array * wavelengths = cpl_array_new(l, CPL_TYPE_DOUBLE);
    cpl_array * wavelengths_resamp = cpl_array_new(l2, CPL_TYPE_DOUBLE);

    for(cpl_size i = 0; i < l; ++i){
        cpl_image_set(flux, i + 1, 1, y[i]);
        cpl_image_set(flux_e, i + 1, 1, y_e[i]);
        cpl_array_set(wavelengths, i, x[i]);
    }

    for(cpl_size i = 0; i < l2; ++i){
        cpl_array_set(wavelengths_resamp, i, x_r[i]);
    }

    hdrl_spectrum1D * sp1 = hdrl_spectrum1D_create(flux, flux_e, wavelengths,
            hdrl_spectrum1D_wave_scale_linear);

    cpl_array_delete(wavelengths);
    cpl_image_delete(flux);
    cpl_image_delete(flux_e);

    hdrl_spectrum1D_wavelength wl = hdrl_spectrum1D_get_wavelength(sp1);

    wl.wavelength = wavelengths_resamp;
    wl.bpm = NULL;

    hdrl_parameter * pars = hdrl_spectrum1D_resample_fit_parameter_create(2, 5);


    hdrl_spectrum1D * sp2 = hdrl_spectrum1D_resample(sp1, &wl, pars);

    hdrl_parameter_delete(pars); pars = NULL;

    /*check that for each position in x_r[i] we linearly interpolate the noise*/
    for(cpl_size i = 0; i < l2; ++i){

        hdrl_error_t err = hdrl_spectrum1D_get_flux_value(sp2, i, NULL).error;
        const cpl_size idx = closer_idx[i];
        hdrl_error_t err_ori = pow(y_e[idx], 2.0) * fabs(x[idx + 1] - x_r[i]) +
                pow(y_e[idx + 1], 2.0) * fabs(x[idx] - x_r[i]);

        err_ori = sqrt(err_ori);

        cpl_test_abs(err, err_ori, HDRL_DELTA_COMPARE_VALUE_ABS);
        hdrl_data_t w = hdrl_spectrum1D_get_wavelength_value(sp2, i, NULL);
        cpl_test_abs(w, x_r[i], HDRL_DELTA_COMPARE_VALUE_ABS);
    }

    hdrl_spectrum1D_delete(&sp1);
    hdrl_spectrum1D_delete(&sp2);
    cpl_array_delete(wavelengths_resamp);
}


/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Test that bad pixels are not taken into account
 *                             for interpolation
 */
/*----------------------------------------------------------------------------*/
void test_spectrum1D_resample_spectrum_bpm(cpl_boolean interpolate){

    double x[]   = { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10,  11,   12,  13,  14,
            15};

    double x_r[] =
    {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.9, 11.1, 12.2, 13.9, 14.1};

    const cpl_size l = LENGTH_ARRAY(x);
    const cpl_size l2 = LENGTH_ARRAY(x_r);

    cpl_image * flux = cpl_image_new(l, 1, CPL_TYPE_DOUBLE);
    cpl_array * wavelengths = cpl_array_new(l, CPL_TYPE_DOUBLE);
    cpl_array * wavelengths_resamp = cpl_array_new(l2, CPL_TYPE_DOUBLE);

    /* set all the bad pixels to 10 and all the others to 0. */
    for(cpl_size i = 0; i < l; ++i){
        if(i % 2 == 0)
            cpl_image_set(flux, i + 1, 1, 10);
        cpl_array_set(wavelengths, i, x[i]);
    }

    for(cpl_size i = 0; i < l; i+=2)
        cpl_image_reject(flux, i + 1, 1);

    for(cpl_size i = 0; i < l2; ++i){
        cpl_array_set(wavelengths_resamp, i, x_r[i]);
    }


    hdrl_spectrum1D * sp1 = hdrl_spectrum1D_create_error_free(flux, wavelengths,
            hdrl_spectrum1D_wave_scale_linear);

    cpl_array_delete(wavelengths);
    cpl_image_delete(flux);

    hdrl_spectrum1D_wavelength wl = hdrl_spectrum1D_get_wavelength(sp1);

    wl.wavelength = wavelengths_resamp;
    wl.bpm = NULL;

    hdrl_parameter * pars = NULL;
    if(interpolate)
        pars = hdrl_spectrum1D_resample_interpolate_parameter_create
            (hdrl_spectrum1D_interp_akima);
    else
        pars = hdrl_spectrum1D_resample_fit_parameter_create(2, 5);

    hdrl_spectrum1D * sp2 = hdrl_spectrum1D_resample(sp1, &wl, pars);

    hdrl_parameter_delete(pars); pars = NULL;

    /* the first element in the interpolated must be rejected since the element
     * a wlength 1 is bad and we mark as bad every pixel outside the interval of the
     * source spectra. I.e. we truncate*/
    int rej = 0;
    hdrl_spectrum1D_get_flux_value(sp2, 0, &rej);
    cpl_test_eq(rej, 1);

    rej = 0;
    hdrl_spectrum1D_get_flux_value(sp2, l2 - 1, &rej);
    cpl_test_eq(rej, 1);

    /*check that all the pixels are zero, the non zero pixels of sp1 should have
     * not contributed to the interpolation, because there were all bad*/
    for(cpl_size i = 1; i < l2 - 1; ++i){

        hdrl_data_t data = hdrl_spectrum1D_get_flux_value(sp2, i, NULL).data;

        cpl_test_rel(data, 0.0, 1e-6);
    }

    hdrl_spectrum1D_delete(&sp1);
    hdrl_spectrum1D_delete(&sp2);
    cpl_array_delete(wavelengths_resamp);
}

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Test error calculation for resample based on
 *                             fitting. We first resample .2 forward and then
 *                             resample .2 backward. We expect that the flux
 *                             of this 2nd spectrum has very similar errors to the
 *                             original one.
 */
/*----------------------------------------------------------------------------*/
void test_spectrum1D_resample_spectrum_fit_error_test_shift(cpl_boolean is_error_free){

    const hdrl_spectrum1D_wave_scale scale = hdrl_spectrum1D_wave_scale_linear;
    double y[]   = { 0,  1,  2,  1,  0, -1, -2, -1,  0,  1,   2,    1,   0, -1};
    double y_e[] = {.1, .2, .3, .2, .1, .2, .3, .2, .1, .2,  .3,   .2,  .1, .2};
    double x[]   = { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10,  11,   12,  13, 14};

    const cpl_size l = LENGTH_ARRAY(x);

    cpl_image * flux = cpl_image_new(l, 1, CPL_TYPE_DOUBLE);
    cpl_image * flux_e = cpl_image_new(l, 1, CPL_TYPE_DOUBLE);
    cpl_array * wavelengths = cpl_array_new(l, CPL_TYPE_DOUBLE);


    for(cpl_size i = 0; i < l; ++i){
        cpl_image_set(flux, i + 1, 1, y[i]);
        if(!is_error_free)
            cpl_image_set(flux_e, i + 1, 1, y_e[i]);
        cpl_array_set(wavelengths, i, x[i]);
    }

    cpl_array * wavelengths_resampled1 = cpl_array_new(l - 1, CPL_TYPE_DOUBLE);
    cpl_array * wavelengths_resampled2 = cpl_array_new(l - 2, CPL_TYPE_DOUBLE);

    for(cpl_size i = 0; i < l - 1; ++i){
        const double d = cpl_array_get(wavelengths, i, NULL) + .2;
        cpl_array_set(wavelengths_resampled1, i, d);
    }

    for(cpl_size i = 1; i < l - 1; ++i){
        const double d = cpl_array_get(wavelengths, i, NULL);
        cpl_array_set(wavelengths_resampled2, i - 1, d);
    }

    hdrl_spectrum1D * sp1 = hdrl_spectrum1D_create(flux, flux_e,
            wavelengths, scale);
    cpl_test_nonnull(sp1);

    const hdrl_spectrum1D_wavelength wl1 = {wavelengths_resampled1, NULL, scale};
    const hdrl_spectrum1D_wavelength wl2 = {wavelengths_resampled2, NULL, scale};

    hdrl_parameter * pars =
            hdrl_spectrum1D_resample_fit_parameter_create(4, l - 3);

    hdrl_spectrum1D * sp2 = hdrl_spectrum1D_resample(sp1, &wl1, pars);
    cpl_test_nonnull(sp2);

    hdrl_spectrum1D * sp3 = hdrl_spectrum1D_resample(sp2, &wl2, pars);
    cpl_test_nonnull(sp3);

    for(cpl_size i = 1; i < l - 2; ++i){
        const hdrl_value v1 =
                hdrl_spectrum1D_get_flux_value(sp1, i, NULL);
        const hdrl_value v3 =
                hdrl_spectrum1D_get_flux_value(sp3, i - 1, NULL);

        cpl_test_eq(v1.error, v3.error);
        cpl_test_abs(v1.data, v3.data, .5);
    }

    hdrl_parameter_delete(pars);
    cpl_image_delete(flux);
    cpl_image_delete(flux_e);
    cpl_array_delete(wavelengths);
    cpl_array_delete(wavelengths_resampled1);
    cpl_array_delete(wavelengths_resampled2);

    hdrl_spectrum1D_delete(&sp1);
    hdrl_spectrum1D_delete(&sp2);
    hdrl_spectrum1D_delete(&sp3);
}

static inline double
func(const double t)
{
  double x = sin(10.0 * t);
  return exp(x*x*x);
}

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Test for fitting when using windows
 */
/*----------------------------------------------------------------------------*/
void test_spectrum1D_resample_spectrum_fit_windowed(void){

    const cpl_size nblocks = 5e2;
    const cpl_size length = 100e3;
    const cpl_size window = length / nblocks;

    const double dt = 1.0  / (length - 1);

    const hdrl_spectrum1D_wave_scale scale = hdrl_spectrum1D_wave_scale_linear;

    hdrl_parameter * pars_no_win =
               hdrl_spectrum1D_resample_fit_parameter_create(4, 28);

    hdrl_parameter * pars_win =
               hdrl_spectrum1D_resample_fit_windowed_parameter_create
               (4, 6, window, 1.2);

    cpl_array * lambdas           = cpl_array_new(length,     CPL_TYPE_DOUBLE);
    cpl_array * lambdas_resampled = cpl_array_new(length - 2, CPL_TYPE_DOUBLE);

    cpl_image * flux = cpl_image_new(length, 1, CPL_TYPE_DOUBLE);
    cpl_image * flux_real = cpl_image_new(length - 2, 1, CPL_TYPE_DOUBLE);

    for(cpl_size i = 0; i < length; ++i){
        const double d = i * dt;
        cpl_array_set(lambdas, i, d);
        cpl_image_set(flux, i + 1, 1,  func(d));
    }

    for(cpl_size i = 0; i < length - 2; ++i){
        const double d = (i + .5) * dt;
        cpl_array_set(lambdas_resampled, i, d);
        cpl_image_set(flux_real, i + 1, 1, func(d));
    }

    hdrl_spectrum1D_wavelength wav = {lambdas_resampled, NULL, scale};

    hdrl_spectrum1D * sp_source =
            hdrl_spectrum1D_create_error_free(flux, lambdas, scale);

    hdrl_spectrum1D * sp_ideal_resampled =
            hdrl_spectrum1D_create_error_free
            (flux_real, lambdas_resampled, scale);

    hdrl_spectrum1D * sp_win_resampled = hdrl_spectrum1D_resample
            (sp_source, &wav, pars_win);

    hdrl_spectrum1D * sp_resampled = hdrl_spectrum1D_resample
            (sp_source, &wav, pars_no_win);

    for(cpl_size i = 0; i < length - 2; ++i){
        hdrl_data_t v_ideal = hdrl_spectrum1D_get_flux_value
                (sp_ideal_resampled, i, NULL).data;
        hdrl_data_t v_win = hdrl_spectrum1D_get_flux_value
                (sp_win_resampled, i, NULL).data;
        hdrl_data_t v_no_win = hdrl_spectrum1D_get_flux_value
                (sp_resampled, i, NULL).data;

        cpl_test_rel(v_ideal, v_win, 1.5e-1);
        cpl_test_rel(v_ideal, v_no_win, 1.5e-1);
    }

    hdrl_parameter_delete(pars_no_win);
    hdrl_parameter_delete(pars_win);

    cpl_array_delete(lambdas);
    cpl_array_delete(lambdas_resampled);

    cpl_image_delete(flux);
    cpl_image_delete(flux_real);
    hdrl_spectrum1D_delete(&sp_ideal_resampled);
    hdrl_spectrum1D_delete(&sp_win_resampled);
    hdrl_spectrum1D_delete(&sp_resampled);
    hdrl_spectrum1D_delete(&sp_source);
}

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Test for the selection function.
 */
/*----------------------------------------------------------------------------*/
void test_spectrum1D_wavelength_select(void){

    const hdrl_spectrum1D_wave_scale scale = hdrl_spectrum1D_wave_scale_linear;
    double y[]   = { 0,  1,  2,  1,  0, -1, -2, -1,  0,  1,   2,    1,   0, -1};
    double y_e[] = {.1, .2, .3, .2, .1, .2, .3, .2, .1, .2,  .3,   .2,  .1, .2};
    double x[]   = { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10,  11,   12,  13, 14};

    const cpl_size l = LENGTH_ARRAY(x);

    cpl_image * flux = cpl_image_new(l, 1, CPL_TYPE_DOUBLE);
    cpl_image * flux_e = cpl_image_new(l, 1, CPL_TYPE_DOUBLE);
    cpl_array * wavelengths = cpl_array_new(l, CPL_TYPE_DOUBLE);


    for(cpl_size i = 0; i < l; ++i){
       cpl_image_set(flux, i + 1, 1, y[i]);
       cpl_image_set(flux_e, i + 1, 1, y_e[i]);
       cpl_array_set(wavelengths, i, x[i]);
   }

   cpl_image_reject(flux, 5, 1);

   hdrl_spectrum1D * sp1 =
           hdrl_spectrum1D_create(flux, flux_e, wavelengths, scale);

   hdrl_spectrum1D * sp2 = select_window(sp1, (sel_window){3, 10, CPL_TRUE});

   cpl_test_eq(8, hdrl_spectrum1D_get_size(sp2));
   {
       int rej = 0;
       hdrl_spectrum1D_get_flux_value(sp2, 2, &rej);
       cpl_test_eq(rej, 1);
    }
   const cpl_size l2 = hdrl_spectrum1D_get_size(sp2);
   for(cpl_size i = 0; i < l2; ++i){

       if(i == 2) continue;

       int rej1_1 = 0;
       int rej2_1 = 0;

       const hdrl_data_t w1 =
               hdrl_spectrum1D_get_wavelength_value(sp1, i + 2, &rej1_1);
       const hdrl_data_t w2 =
               hdrl_spectrum1D_get_wavelength_value(sp2, i, &rej2_1);

       cpl_test_eq(rej1_1, 0);
       cpl_test_eq(rej2_1, 0);

       cpl_test_eq(w1, w2);

       int rej1_2 = 0;
       int rej2_2 = 0;
       const hdrl_value s1 = hdrl_spectrum1D_get_flux_value(sp1, i + 2, &rej1_2);
       const hdrl_value s2 = hdrl_spectrum1D_get_flux_value(sp2, i, &rej2_2);

       cpl_test_eq(rej1_2, 0);
       cpl_test_eq(rej2_2, 0);

       cpl_test_rel(s1.data, s2.data, HDRL_DELTA_COMPARE_VALUE);
       cpl_test_rel(s1.error, s2.error, HDRL_DELTA_COMPARE_VALUE);
   }

   hdrl_spectrum1D_delete(&sp2);

   hdrl_spectrum1D * sp3 = select_window(sp1, (sel_window){3, 10, CPL_FALSE});

   cpl_test_eq(6, hdrl_spectrum1D_get_size(sp3));

   cpl_size idxes[] = {0, 1, 10, 11, 12, 13};

   for(cpl_size i = 0; i < 6; ++i){

          int rej1_1 = 0;
          int rej2_1 = 0;

          const hdrl_data_t w1 =
                  hdrl_spectrum1D_get_wavelength_value(sp1, idxes[i], &rej1_1);
          const hdrl_data_t w2 =
                  hdrl_spectrum1D_get_wavelength_value(sp3, i, &rej2_1);

          cpl_test_eq(rej1_1, 0);
          cpl_test_eq(rej2_1, 0);

          cpl_test_eq(w1, w2);

          int rej1_2 = 0;
          int rej2_2 = 0;
          const hdrl_value s1 =
                  hdrl_spectrum1D_get_flux_value(sp1, idxes[i], &rej1_2);
          const hdrl_value s2 = hdrl_spectrum1D_get_flux_value(sp3, i, &rej2_2);

          cpl_test_eq(rej1_2, 0);
          cpl_test_eq(rej2_2, 0);

          cpl_test_rel(s1.data, s2.data, HDRL_DELTA_COMPARE_VALUE);
          cpl_test_rel(s1.error, s2.error, HDRL_DELTA_COMPARE_VALUE);
      }

   hdrl_spectrum1D_delete(&sp1);
   hdrl_spectrum1D_delete(&sp3);
   cpl_image_delete(flux);
   cpl_image_delete(flux_e);
   cpl_array_delete(wavelengths);

   /* Check for the function that rejects some elements inside the spectrum */
   const cpl_size sz = 10;

   hdrl_spectrum1D * sp =
           get_spectrum1D_sin_shuffled(sz, 3, CPL_FALSE, NULL);

   cpl_array * arr = cpl_array_new(sz, CPL_TYPE_INT);

   for(cpl_size i = 0; i < sz; i++){
       cpl_array_set(arr, i, i % 2);
   }

   hdrl_spectrum1D * sp_r1 =
           hdrl_spectrum1D_reject_pixels(sp, arr);

   for(cpl_size i = 0; i < sz; i++){
       int rej = 0;
       hdrl_value d = hdrl_spectrum1D_get_flux_value(sp_r1, i, &rej);

       if(i % 2 == 1){
           cpl_test(rej);
       }
       else{
           hdrl_value d1 = hdrl_spectrum1D_get_flux_value(sp, i, NULL);
           cpl_test_rel(d.data, d1.data, HDRL_EPS_DATA);
           cpl_test_rel(d.error, d1.error, HDRL_EPS_DATA);
       }
   }

   cpl_array_delete(arr);
   hdrl_spectrum1D_delete(&sp);
   hdrl_spectrum1D_delete(&sp_r1);
}

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Test for the uniformly sampled function
 */
/*----------------------------------------------------------------------------*/
void test_spectrum1D_test_uniformly_sampled(void){

    const hdrl_spectrum1D_wave_scale scale = hdrl_spectrum1D_wave_scale_linear;

    const cpl_size sz = 100;
    hdrl_spectrum1D * sp =
            get_spectrum1D_sin_shuffled(sz, 3, CPL_FALSE, NULL);

    double delta = 0.0;
    cpl_boolean is_OK = hdrl_spectrum1D_is_uniformly_sampled(sp, &delta);

    cpl_test(is_OK);
    cpl_test_abs(delta, 2 * CPL_MATH_PI / sz, HDRL_EPS_DATA);

    hdrl_spectrum1D_delete(&sp);

    cpl_array * arr = cpl_array_new(sz, CPL_TYPE_DOUBLE);
    cpl_image * flx = cpl_image_new(sz, 1, CPL_TYPE_DOUBLE);
    for(cpl_size i = 0; i < sz; ++i){
        cpl_array_set(arr, i, i + 1);
        cpl_image_set(flx, i + 1, 1, .1);
    }

    cpl_array_set(arr, 4, 5.1);
    sp = hdrl_spectrum1D_create_error_free(flx, arr, scale);
    is_OK = hdrl_spectrum1D_is_uniformly_sampled(sp, &delta);
    cpl_test(!is_OK);
    hdrl_spectrum1D_delete(&sp);

    cpl_array_set(arr, 4, 5);
    sp = hdrl_spectrum1D_create_error_free(flx, arr, scale);
    is_OK = hdrl_spectrum1D_is_uniformly_sampled(sp, &delta);
    cpl_test(is_OK);
    cpl_test_rel(delta, 1.0, HDRL_EPS_DATA);
    hdrl_spectrum1D_delete(&sp);

    cpl_image_delete(flx);
    cpl_array_delete(arr);
}

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Test for the hdrl_spectrum1Dlist
 */
/*----------------------------------------------------------------------------*/
void test_spectrum1Dlist(void){

    {
        hdrl_spectrum1Dlist * list = hdrl_spectrum1Dlist_new();
        hdrl_spectrum1Dlist_delete(list);
    }

    const cpl_size sz = 40;
    {
        hdrl_spectrum1Dlist * list = hdrl_spectrum1Dlist_new();

        hdrl_spectrum1D * s1 =
                get_spectrum1D_sin_shuffled(sz, 4, CPL_TRUE, NULL);
        hdrl_spectrum1D * s2 =
                get_spectrum1D_sin_shuffled(sz, 4, CPL_TRUE, NULL);
        hdrl_spectrum1D * s3 =
                get_spectrum1D_sin_shuffled(sz, 4, CPL_TRUE, NULL);

        hdrl_spectrum1Dlist_set(list, s1, 0);
        hdrl_spectrum1Dlist_set(list, s2, 1);
        hdrl_spectrum1Dlist_set(list, s3, 2);

        cpl_test_eq(hdrl_spectrum1Dlist_get_size(list), 3);
        cpl_test_eq(list->capacity, 4);

        hdrl_spectrum1D * s22 = hdrl_spectrum1Dlist_get(list, 1);
        const hdrl_spectrum1D * s22_c = hdrl_spectrum1Dlist_get_const(list, 1);

        cpl_test_eq_ptr(s2, s22);
        cpl_test_eq_ptr(s2, s22_c);

        hdrl_spectrum1D * s22_u = hdrl_spectrum1Dlist_unset(list, 1);
        cpl_test_eq_ptr(s22_u, s22_c);

        cpl_test_eq(hdrl_spectrum1Dlist_get_size(list), 2);

        hdrl_spectrum1D_delete(&s22_u);

        cpl_size i = 0;
        while(hdrl_spectrum1Dlist_get_size(list) > 0) {
            hdrl_spectrum1D * s = hdrl_spectrum1Dlist_unset(list, 0);

            if(i == 0)
                cpl_test_eq_ptr(s, s1);
            else
                cpl_test_eq_ptr(s, s3);

            hdrl_spectrum1D_delete(&s);
            i++;
        }

        cpl_test_eq(hdrl_spectrum1Dlist_get_size(list), 0);
        cpl_test_eq(list->capacity, 0);
        cpl_test_eq_ptr(list->spectra, NULL);

        hdrl_spectrum1Dlist_delete(list);
    }
}

static inline cpl_array *
get_waves(double start_wave, cpl_size sz, double step){

    cpl_array * to_ret = cpl_array_new(sz, HDRL_TYPE_DATA);

    for(cpl_size i = 0; i < sz; ++i){
        cpl_array_set(to_ret, i, start_wave + step * i);
    }

    return to_ret;
}

static inline hdrl_spectrum1D *
generate_stair_spectrum(int start, int stop, double start_wave, double step_wave){

    const cpl_size sz = stop - start + 1;
    cpl_array * wave = get_waves(start_wave, sz, step_wave);
    cpl_image * flx = cpl_image_new(sz, 1, CPL_TYPE_DOUBLE);
    cpl_image * flx_e = cpl_image_new(sz, 1, CPL_TYPE_DOUBLE);


    for(cpl_size i = 0; i < sz; ++i){
        const double f = start + i;
        cpl_image_set(flx, i + 1, 1, f);
        cpl_image_set(flx_e, i + 1, 1, f / 10.0);
    }

    hdrl_spectrum1D * to_ret = hdrl_spectrum1D_create(flx, flx_e, wave,
            hdrl_spectrum1D_wave_scale_linear);

    cpl_image_delete(flx);
    cpl_image_delete(flx_e);
    cpl_array_delete(wave);


    return to_ret;
}

cpl_boolean contains(const cpl_array * arr, cpl_size idx){
    for(cpl_size i = 0; i < cpl_array_get_size(arr); ++i){
        const cpl_size this_idx = (cpl_size)cpl_array_get(arr, i, NULL);
        if(this_idx == idx) return CPL_TRUE;
    }
    return CPL_FALSE;
}

static inline hdrl_spectrum1D *
generate_bad_stair_spectrum(int start, int stop, double start_wave,
        double step_wave, cpl_array * bad_idxes){

    const cpl_size sz = stop - start + 1;
    cpl_array * wave = get_waves(start_wave, sz, step_wave);
    cpl_image * flx = cpl_image_new(sz, 1, CPL_TYPE_DOUBLE);
    cpl_image * flx_e = cpl_image_new(sz, 1, CPL_TYPE_DOUBLE);


    for(cpl_size i = 0; i < sz; ++i){

        if(contains(bad_idxes, i))
        {
            cpl_image_reject(flx, i + 1, 1);
            cpl_image_reject(flx_e, i + 1, 1);
            continue;
        }

        const double f = start + i;
        cpl_image_set(flx, i + 1, 1, f);
        cpl_image_set(flx_e, i + 1, 1, f / 10.0);
    }

    hdrl_spectrum1D * to_ret = hdrl_spectrum1D_create(flx, flx_e, wave,
            hdrl_spectrum1D_wave_scale_linear);

    cpl_image_delete(flx);
    cpl_image_delete(flx_e);
    cpl_array_delete(wave);


    return to_ret;
}


static inline hdrl_spectrum1D *
generate_stair_spectrum_shuffled(int start, int stop, double start_wave,
        double step_wave, cpl_size * idxes){

    const cpl_size sz = stop - start + 1;
    cpl_array * wave = get_waves(start_wave, sz, step_wave);
    cpl_array * wave_s = cpl_array_duplicate(wave);

    cpl_image * flx = cpl_image_new(sz, 1, CPL_TYPE_DOUBLE);
    cpl_image * flx_e = cpl_image_new(sz, 1, CPL_TYPE_DOUBLE);

    for(cpl_size i = 0; i < sz; ++i){
        const double f = start + i;
        const cpl_size dest_idx = idxes[i];
        cpl_image_set(flx, dest_idx + 1, 1, f);
        cpl_image_set(flx_e, dest_idx + 1, 1, f / 10.0);

        const double w = cpl_array_get(wave, i, NULL);
        cpl_array_set(wave_s, dest_idx, w);
    }

    hdrl_spectrum1D * to_ret = hdrl_spectrum1D_create(flx, flx_e, wave_s,
            hdrl_spectrum1D_wave_scale_linear);

    cpl_image_delete(flx);
    cpl_image_delete(flx_e);
    cpl_array_delete(wave);
    cpl_array_delete(wave_s);

    return to_ret;
}

void test1(void){
    hdrl_parameter * par = hdrl_spectrum1D_resample_integrate_parameter_create();

    hdrl_spectrum1D * ori_s = generate_stair_spectrum(1, 8, 20.0, 2.0);

    cpl_array * wavs_integrate = get_waves(21, 9, 1);
    hdrl_spectrum1D * integrated_s =
            hdrl_spectrum1D_resample_on_array(ori_s, wavs_integrate, par);

    /* first element: the initial bin is asimmetric, tested as
     * special case*/
    {
        int rej = 0;
        hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, 0, &rej);
        cpl_test_eq(rej, 0);
        cpl_test_rel(f.data, 2.0, HDRL_DELTA_COMPARE_VALUE);
        cpl_test_rel(f.error, f.data / 10.0, HDRL_DELTA_COMPARE_VALUE);
    }

    const cpl_size int_size = cpl_array_get_size(wavs_integrate);

    /* last element: the final bin is asimmetric, tested as
     * special case*/
    {
        int rej = 0;
        hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, int_size - 1, &rej);
        cpl_test_eq(rej, 0);
        cpl_test_rel(f.data, 5.0, HDRL_DELTA_COMPARE_VALUE);
        cpl_test_rel(f.error, f.data / 10.0, HDRL_DELTA_COMPARE_VALUE);
    }

    double src_flx = 2.0;
    for(cpl_size i = 1; i < int_size - 1; ++i){

        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);

        cpl_test_eq(rej, 0);

        if((i % 2) == 1){
            /*odd elements: they are in the center of the source bin*/
            cpl_test_rel(f.data, src_flx, HDRL_DELTA_COMPARE_VALUE);
            cpl_test_rel(f.error, src_flx / 10.0, HDRL_DELTA_COMPARE_VALUE);

        }
        else{
            /*even elements: they are between bins*/
            const double el = src_flx + .5;
            cpl_test_rel(f.data, el, HDRL_DELTA_COMPARE_VALUE);

            const double el_e = pow(src_flx, 2.0) + pow((src_flx + 1.0), 2.0);
            cpl_test_rel(f.error, sqrt(el_e) / (10.0 * sqrt(2.0)), HDRL_DELTA_COMPARE_VALUE);
            src_flx++;
        }
    }

    hdrl_parameter_delete(par);
    hdrl_spectrum1D_delete(&ori_s);
    hdrl_spectrum1D_delete(&integrated_s);
    cpl_array_delete(wavs_integrate);
}

void test2(void){

    hdrl_parameter * par = hdrl_spectrum1D_resample_integrate_parameter_create();

    hdrl_spectrum1D * ori_s = generate_stair_spectrum(1, 8, 20.0, 2.0);

    cpl_array * wavs_integrate = get_waves(20, 15, 1);
    hdrl_spectrum1D * integrated_s =
            hdrl_spectrum1D_resample_on_array(ori_s, wavs_integrate, par);

    const cpl_size int_size = cpl_array_get_size(wavs_integrate);

    double src_flx = 1.0;
    for(cpl_size i = 0; i < int_size; ++i){

        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);

        cpl_test_eq(rej, 0);

        if((i % 2) == 0){
            /*odd elements: they are in the center of the source bin*/
            cpl_test_rel(f.data, src_flx, HDRL_DELTA_COMPARE_VALUE);
            cpl_test_rel(f.error, src_flx / 10.0, HDRL_DELTA_COMPARE_VALUE);

        }
        else{
            /*even elements: they are between bins*/
            const double el = src_flx + .5;
            cpl_test_rel(f.data, el, HDRL_DELTA_COMPARE_VALUE);

            const double el_e = pow(src_flx, 2.0) + pow((src_flx + 1.0), 2.0);
            cpl_test_rel(f.error, sqrt(el_e) / (10.0 * sqrt(2.0)), HDRL_DELTA_COMPARE_VALUE);
            src_flx++;
        }
    }

    hdrl_parameter_delete(par);
    hdrl_spectrum1D_delete(&ori_s);
    hdrl_spectrum1D_delete(&integrated_s);
    cpl_array_delete(wavs_integrate);
}

void test3(void){

    hdrl_parameter * par = hdrl_spectrum1D_resample_integrate_parameter_create();

    hdrl_spectrum1D * ori_s = generate_stair_spectrum(1, 8, 20.0, 2.0);

    cpl_array * wavs_integrate = get_waves(19, 17, 1);
    hdrl_spectrum1D * integrated_s =
            hdrl_spectrum1D_resample_on_array(ori_s, wavs_integrate, par);

    const cpl_size int_size = cpl_array_get_size(wavs_integrate);

    /*2 bins are rejected, 1 because is completely outside, the other half outside*/
    for(cpl_size i = 0; i < 2; ++i){
       int rej = 0;
       const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);
       cpl_test_eq(rej, 1);
       cpl_test(isnan(f.data));
       cpl_test(isnan(f.error));
    }

    /*2 bins are rejected, 1 because is completely outside, the other half outside*/
    for(cpl_size i = int_size - 2; i < int_size; ++i){
       int rej = 0;
       const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);
       cpl_test_eq(rej, 1);
       cpl_test(isnan(f.data));
       cpl_test(isnan(f.error));
    }

    double src_flx = 1.0;
    for(cpl_size i = 2; i < int_size - 2; ++i){

        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);

        cpl_test_eq(rej, 0);

        if((i % 2) == 1){
            /*odd elements: they are in the center of the source bin*/
            cpl_test_rel(f.data, src_flx, HDRL_DELTA_COMPARE_VALUE);
            cpl_test_rel(f.error, src_flx / 10.0, HDRL_DELTA_COMPARE_VALUE);

        }
        else{
            /*even elements: they are between bins*/
            const double el = src_flx + .5;
            cpl_test_rel(f.data, el, HDRL_DELTA_COMPARE_VALUE);

            const double el_e = pow(src_flx, 2.0) + pow((src_flx + 1.0), 2.0);
            cpl_test_rel(f.error, sqrt(el_e) / (10.0 * sqrt(2.0)), HDRL_DELTA_COMPARE_VALUE);
            src_flx++;
        }
    }

    hdrl_parameter_delete(par);
    hdrl_spectrum1D_delete(&ori_s);
    hdrl_spectrum1D_delete(&integrated_s);
    cpl_array_delete(wavs_integrate);
}

static inline cpl_array *
shuffle(const cpl_array * ori, const cpl_size * idxes){
    cpl_array * to_ret = cpl_array_duplicate(ori);
    cpl_size sz = cpl_array_get_size(ori);
    for(cpl_size i = 0; i < sz; ++i){
        const double w = cpl_array_get(ori, i, NULL);
        cpl_array_set(to_ret, idxes[i], w);
    }
    return to_ret;
}

void test4(void){
    hdrl_parameter * par = hdrl_spectrum1D_resample_integrate_parameter_create();

    hdrl_spectrum1D * ori_s = generate_stair_spectrum(1, 8, 20.0, 2.0);

    hdrl_spectrum1D * ori_s_shuffled = generate_stair_spectrum_shuffled(1, 8, 20.0, 2.0,
            (cpl_size[]){3, 2, 1, 4, 7, 6, 0, 5});

    cpl_array * wavs_integrate = get_waves(21, 9, 1);

    cpl_size shuffles[] = {1, 3, 5, 0, 8, 7, 6, 2, 4};
    cpl_array * wavs_integrate_shuffle = shuffle(wavs_integrate,
            shuffles);

    hdrl_spectrum1D * integrated_s_shuffled =
            hdrl_spectrum1D_resample_on_array(ori_s_shuffled, wavs_integrate_shuffle, par);

    hdrl_spectrum1D * integrated_s=
            hdrl_spectrum1D_resample_on_array(ori_s, wavs_integrate, par);

    const cpl_size sz = hdrl_spectrum1D_get_size(integrated_s_shuffled);
    cpl_test_eq(sz, cpl_array_get_size(wavs_integrate));
    cpl_test_eq(sz, hdrl_spectrum1D_get_size(integrated_s));

    for(cpl_size i = 0; i < sz; ++i){
        const double wav_s = hdrl_spectrum1D_get_wavelength_value(integrated_s,
                i, NULL);
        const double wav = cpl_array_get(wavs_integrate, i, NULL);
        cpl_test_rel(wav_s, wav, HDRL_DELTA_COMPARE_VALUE);
    }

    for(cpl_size i = 0; i < sz; ++i){
        const double wav_s = hdrl_spectrum1D_get_wavelength_value(integrated_s_shuffled,
                i, NULL);
        const double wav = cpl_array_get(wavs_integrate_shuffle, i, NULL);
        cpl_test_rel(wav_s, wav, HDRL_DELTA_COMPARE_VALUE);
    }

    for(cpl_size i = 0; i < sz; ++i){
        int rej = 0, rej_shuffled = 0;

        const hdrl_value flx = hdrl_spectrum1D_get_flux_value(integrated_s,
                i, &rej);
        const hdrl_value flx_shuffled = hdrl_spectrum1D_get_flux_value(integrated_s_shuffled,
                shuffles[i], &rej_shuffled);

        const hdrl_data_t wav = hdrl_spectrum1D_get_wavelength_value(integrated_s,
                i, NULL);

        const hdrl_data_t wav_shuffled = hdrl_spectrum1D_get_wavelength_value(integrated_s_shuffled,
                shuffles[i], NULL);

        cpl_test_rel(wav, wav_shuffled, HDRL_DELTA_COMPARE_VALUE);
        cpl_test_eq(rej, rej_shuffled);
        cpl_test_rel(flx.data, flx_shuffled.data, HDRL_DELTA_COMPARE_VALUE);
        cpl_test_rel(flx.error, flx_shuffled.error, HDRL_DELTA_COMPARE_VALUE);
    }

    hdrl_parameter_delete(par);
    hdrl_spectrum1D_delete(&ori_s);
    hdrl_spectrum1D_delete(&ori_s_shuffled);
    hdrl_spectrum1D_delete(&integrated_s);
    hdrl_spectrum1D_delete(&integrated_s_shuffled);
    cpl_array_delete(wavs_integrate);
    cpl_array_delete(wavs_integrate_shuffle);
}

void test5(void){

    hdrl_parameter * par = hdrl_spectrum1D_resample_integrate_parameter_create();

    cpl_array * bad_idxes = cpl_array_new(4, CPL_TYPE_INT);

    cpl_array_set(bad_idxes, 0, 0);
    cpl_array_set(bad_idxes, 1, 7);
    cpl_array_set(bad_idxes, 2, 2);
    cpl_array_set(bad_idxes, 3, 5);

    hdrl_spectrum1D * ori_s = generate_bad_stair_spectrum(1, 8, 20.0, 2.0 , bad_idxes);

    cpl_array * wavs_integrate = get_waves(19, 17, 1);
    hdrl_spectrum1D * integrated_s =
            hdrl_spectrum1D_resample_on_array(ori_s, wavs_integrate, par);

    const cpl_size int_size = cpl_array_get_size(wavs_integrate);

    /*3 bins are rejected, 1 because is completely outside, the other half outside,
     * the third in a bad pixel*/
    for(cpl_size i = 0; i < 3; ++i){
       int rej = 0;
       const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);
       cpl_test_eq(rej, 1);
       cpl_test(isnan(f.data));
       cpl_test(isnan(f.error));
    }

    /*3 bins are rejected, 1 because is completely outside, the other half outside,
     * the third in a bad pixel*/
    for(cpl_size i = int_size - 3; i < int_size; ++i){
       int rej = 0;
       const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);
       cpl_test_eq(rej, 1);
       cpl_test(isnan(f.data));
       cpl_test(isnan(f.error));
    }

    double src_flx = 2.0;
    for(cpl_size i = 3; i < int_size - 3; ++i){

        /*rejected because inside a bad pixel*/
        if((i >=4 && i <= 6) || (i >= 10 && i <= 12)){
            int rej = 0;
            const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);
            cpl_test_eq(rej, 1);
            cpl_test(isnan(f.data));
            cpl_test(isnan(f.error));

            if((i % 2) == 0) src_flx++;

            continue;
        }

        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);

        cpl_test_eq(rej, 0);

        if((i % 2) == 1){
            /*odd elements: they are in the center of the source bin*/
            cpl_test_rel(f.data, src_flx, HDRL_DELTA_COMPARE_VALUE);
            cpl_test_rel(f.error, src_flx / 10.0, HDRL_DELTA_COMPARE_VALUE);

        }
        else{
            /*even elements: they are between bins*/
            const double el = src_flx + .5;
            cpl_test_rel(f.data, el, HDRL_DELTA_COMPARE_VALUE);

            const double el_e = pow(src_flx, 2.0) + pow((src_flx + 1.0), 2.0);
            cpl_test_rel(f.error, sqrt(el_e) / (10.0 * sqrt(2.0)), HDRL_DELTA_COMPARE_VALUE);
            src_flx++;
        }
    }

    hdrl_parameter_delete(par);
    hdrl_spectrum1D_delete(&ori_s);
    hdrl_spectrum1D_delete(&integrated_s);
    cpl_array_delete(wavs_integrate);
    cpl_array_delete(bad_idxes);
}

void test6(void){

    hdrl_parameter * par = hdrl_spectrum1D_resample_integrate_parameter_create();

    hdrl_spectrum1D * ori_s = generate_stair_spectrum(1, 8, 20.0, 2.0);

    cpl_array * wavs_integrate = get_waves(20.5, 15, 1);
    hdrl_spectrum1D * integrated_s =
            hdrl_spectrum1D_resample_on_array(ori_s, wavs_integrate, par);

    double src_flx = 1.0;

    for(cpl_size i = 0; i < cpl_array_get_size(wavs_integrate) - 1; ++i){
        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);
        cpl_test_eq(rej, 0);
        cpl_test_rel(f.data, src_flx, HDRL_DELTA_COMPARE_VALUE);
        cpl_test_rel(f.error, src_flx / 10.0, HDRL_DELTA_COMPARE_VALUE);
        src_flx = src_flx + ((i+1) % 2);
    }

    int rej = 0;
    const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s,
            cpl_array_get_size(wavs_integrate) - 1, &rej);
    cpl_test_eq(rej, 1);
    cpl_test(isnan(f.data));
    cpl_test(isnan(f.error));

    hdrl_parameter_delete(par);
    hdrl_spectrum1D_delete(&ori_s);
    hdrl_spectrum1D_delete(&integrated_s);
    cpl_array_delete(wavs_integrate);
}

void test7(void){

    hdrl_parameter * par = hdrl_spectrum1D_resample_integrate_parameter_create();

    cpl_array * bads = cpl_array_new(3, CPL_TYPE_INT);

    cpl_array_set(bads, 0, 0);
    cpl_array_set(bads, 1, 7);
    cpl_array_set(bads, 2, 4);

    hdrl_spectrum1D * ori_s = generate_bad_stair_spectrum(1, 8, 20.0, 2.0, bads);

    cpl_array * wavs_integrate = get_waves(20.5, 15, 1);
    hdrl_spectrum1D * integrated_s =
            hdrl_spectrum1D_resample_on_array(ori_s, wavs_integrate, par);

    double src_flx = 1.0;

    for(cpl_size i = 0; i < cpl_array_get_size(wavs_integrate); ++i){
        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);

        if(i == 0 || i == 7 || i == 8 || i >= 13){
            cpl_test_eq(rej, 1);
            cpl_test(isnan(f.data));
            cpl_test(isnan(f.error));
        }
        else{
            cpl_test_eq(rej, 0);
            cpl_test_rel(f.data, src_flx, HDRL_DELTA_COMPARE_VALUE);
            cpl_test_rel(f.error, src_flx / 10.0, HDRL_DELTA_COMPARE_VALUE);
        }
        src_flx = src_flx + ((i+1) % 2);
    }

    hdrl_parameter_delete(par);
    hdrl_spectrum1D_delete(&ori_s);
    hdrl_spectrum1D_delete(&integrated_s);
    cpl_array_delete(wavs_integrate);
    cpl_array_delete(bads);
}


void test8(void){
    hdrl_parameter * par = hdrl_spectrum1D_resample_integrate_parameter_create();

    hdrl_spectrum1D * ori_s = generate_stair_spectrum(1, 8, 20.0, 1.0);

    cpl_array * wavs_integrate = get_waves(21, 3, 2);
    hdrl_spectrum1D * integrated_s =
            hdrl_spectrum1D_resample_on_array(ori_s, wavs_integrate, par);

    double res_flx[] = {2.5, 4, 5.5};
    double res_flx_e[] = {sqrt(6.5), sqrt(16.5), sqrt(30.5)};

    for(cpl_size i = 0; i < 3; ++i){

        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);

        cpl_test_eq(rej, 0);
        cpl_test_rel(f.data, res_flx[i], HDRL_DELTA_COMPARE_VALUE);
        cpl_test_rel(f.error, res_flx_e[i] / 10.0, HDRL_DELTA_COMPARE_VALUE);
    }

    hdrl_parameter_delete(par);
    hdrl_spectrum1D_delete(&ori_s);
    hdrl_spectrum1D_delete(&integrated_s);
    cpl_array_delete(wavs_integrate);
}

void test9(void){
    hdrl_parameter * par = hdrl_spectrum1D_resample_integrate_parameter_create();

    hdrl_spectrum1D * ori_s = generate_stair_spectrum(1, 8, 20.0, 1.0);

    cpl_array * wavs_integrate = get_waves(19, 5, 2);
    hdrl_spectrum1D * integrated_s =
            hdrl_spectrum1D_resample_on_array(ori_s, wavs_integrate, par);

    {
        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, 0, &rej);
        cpl_test(rej);
        cpl_test(isnan(f.data));
        cpl_test(isnan(f.error));
    }

    double res_flx[] = {2, 4, 6, 7.5};
    for(cpl_size i = 1; i <= 4; ++i){

        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);

        cpl_test_eq(rej, 0);
        cpl_test_rel(f.data, res_flx[i - 1], HDRL_DELTA_COMPARE_VALUE);
    }

    hdrl_parameter_delete(par);
    hdrl_spectrum1D_delete(&ori_s);
    hdrl_spectrum1D_delete(&integrated_s);
    cpl_array_delete(wavs_integrate);
}


void test10(void){
    hdrl_parameter * par = hdrl_spectrum1D_resample_integrate_parameter_create();

    hdrl_spectrum1D * ori_s = generate_stair_spectrum(1, 8, 20.0, 1.0);

    cpl_array * wavs_integrate = get_waves(19, 6, 2);
    hdrl_spectrum1D * integrated_s =
            hdrl_spectrum1D_resample_on_array(ori_s, wavs_integrate, par);
    {
        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, 0, &rej);
        cpl_test(rej);
        cpl_test(isnan(f.data));
        cpl_test(isnan(f.error));
    }

    double res_flx[] = {2, 4, 6};
    for(cpl_size i = 1; i <= 3; ++i){

        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);

        cpl_test_eq(rej, 0);
        cpl_test_rel(f.data, res_flx[i - 1], HDRL_DELTA_COMPARE_VALUE);
    }

    for(cpl_size i = 4; i < 6; ++i){
        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);
        cpl_test(rej);
        cpl_test(isnan(f.data));
        cpl_test(isnan(f.error));
    }

    hdrl_parameter_delete(par);
    hdrl_spectrum1D_delete(&ori_s);
    hdrl_spectrum1D_delete(&integrated_s);
    cpl_array_delete(wavs_integrate);
}


void test11(void){
    hdrl_parameter * par = hdrl_spectrum1D_resample_integrate_parameter_create();

    cpl_array * bads = cpl_array_new(1, CPL_TYPE_INT);

    cpl_array_set(bads, 0, 3);

    hdrl_spectrum1D * ori_s = generate_bad_stair_spectrum(1, 8, 20.0, 1.0, bads);

    cpl_array * wavs_integrate = get_waves(19, 6, 2);
    hdrl_spectrum1D * integrated_s =
            hdrl_spectrum1D_resample_on_array(ori_s, wavs_integrate, par);
    {
        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, 0, &rej);
        cpl_test(rej);
        cpl_test(isnan(f.data));
        cpl_test(isnan(f.error));
    }

    double res_flx[] = {2, 4, 6};
    for(cpl_size i = 1; i <= 3; ++i){

        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);

        if(i == 2){
            cpl_test(rej);
            cpl_test(isnan(f.data));
            cpl_test(isnan(f.error));
            continue;
        }

        cpl_test_eq(rej, 0);
        cpl_test_rel(f.data, res_flx[i - 1], HDRL_DELTA_COMPARE_VALUE);
    }

    for(cpl_size i = 4; i < 6; ++i){
        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);
        cpl_test(rej);
        cpl_test(isnan(f.data));
        cpl_test(isnan(f.error));
    }

    hdrl_parameter_delete(par);
    hdrl_spectrum1D_delete(&ori_s);
    hdrl_spectrum1D_delete(&integrated_s);
    cpl_array_delete(wavs_integrate);
    cpl_array_delete(bads);
}

void test12(void){
    hdrl_parameter * par = hdrl_spectrum1D_resample_integrate_parameter_create();

    cpl_array * bads = cpl_array_new(1, CPL_TYPE_INT);

    cpl_array_set(bads, 0, 4);

    hdrl_spectrum1D * ori_s = generate_bad_stair_spectrum(1, 8, 20.0, 1.0, bads);

    cpl_array * wavs_integrate = get_waves(19, 6, 2);
    hdrl_spectrum1D * integrated_s =
            hdrl_spectrum1D_resample_on_array(ori_s, wavs_integrate, par);

    {
        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, 0, &rej);
        cpl_test(rej);
        cpl_test(isnan(f.data));
        cpl_test(isnan(f.error));
    }

    double res_flx[] = {2, 4, 6};
    for(cpl_size i = 1; i <= 3; ++i){

        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);

        if(i == 2 || i == 3){
            cpl_test(rej);
            cpl_test(isnan(f.data));
            cpl_test(isnan(f.error));
            continue;
        }

        cpl_test_eq(rej, 0);
        cpl_test_rel(f.data, res_flx[i - 1], HDRL_DELTA_COMPARE_VALUE);
    }

    for(cpl_size i = 4; i < 6; ++i){
        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);
        cpl_test(rej);
        cpl_test(isnan(f.data));
        cpl_test(isnan(f.error));
    }

    hdrl_parameter_delete(par);
    hdrl_spectrum1D_delete(&ori_s);
    hdrl_spectrum1D_delete(&integrated_s);
    cpl_array_delete(wavs_integrate);
    cpl_array_delete(bads);
}


void test13(void){
    hdrl_parameter * par = hdrl_spectrum1D_resample_integrate_parameter_create();

    cpl_array * bads = cpl_array_new(2, CPL_TYPE_INT);

    cpl_array_set(bads, 0, 0);
    cpl_array_set(bads, 1, 7);

    hdrl_spectrum1D * ori_s = generate_bad_stair_spectrum(1, 8, 20.0, 1.0, bads);

    cpl_array * wavs_integrate = get_waves(19, 6, 2);
    hdrl_spectrum1D * integrated_s =
            hdrl_spectrum1D_resample_on_array(ori_s, wavs_integrate, par);
    {
        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, 0, &rej);
        cpl_test(rej);
        cpl_test(isnan(f.data));
        cpl_test(isnan(f.error));
    }

    double res_flx[] = {2, 4, 6};
    for(cpl_size i = 0; i <= 3; ++i){

        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);

        if(i == 0 || i == 1){
            cpl_test(rej);
            cpl_test(isnan(f.data));
            cpl_test(isnan(f.error));
            continue;
        }

        cpl_test_eq(rej, 0);
        cpl_test_rel(f.data, res_flx[i - 1], HDRL_DELTA_COMPARE_VALUE);
    }

    for(cpl_size i = 4; i < 6; ++i){
        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);
        cpl_test(rej);
        cpl_test(isnan(f.data));
        cpl_test(isnan(f.error));
    }

    hdrl_parameter_delete(par);
    hdrl_spectrum1D_delete(&ori_s);
    hdrl_spectrum1D_delete(&integrated_s);
    cpl_array_delete(wavs_integrate);
    cpl_array_delete(bads);
}

void test14(void){

    hdrl_parameter * par = hdrl_spectrum1D_resample_integrate_parameter_create();

    cpl_array * bads = cpl_array_new(2, CPL_TYPE_INT);

    cpl_array_set(bads, 0, 0);
    cpl_array_set(bads, 1, 4);

    hdrl_spectrum1D * ori_s = generate_bad_stair_spectrum(1, 9, 20.5, 1.0, bads);

    cpl_array * wavs_integrate = get_waves(21, 5, 2);
    hdrl_spectrum1D * integrated_s =
            hdrl_spectrum1D_resample_on_array(ori_s, wavs_integrate, par);


    double res_flx[] = {2, 3.5, 4.5, 7.5};

    for(cpl_size i = 0; i < cpl_array_get_size(wavs_integrate) - 1; ++i){
        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);

        if(i == 2){
            cpl_test(rej);
            cpl_test(isnan(f.data));
            cpl_test(isnan(f.error));
            continue;
        }

        cpl_test_eq(rej, 0);
        cpl_test_rel(f.data, res_flx[i], HDRL_DELTA_COMPARE_VALUE);
    }

    int rej = 0;
    const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s,
            cpl_array_get_size(wavs_integrate) - 1, &rej);
    cpl_test(rej);
    cpl_test(isnan(f.data));
    cpl_test(isnan(f.error));

    hdrl_parameter_delete(par);
    hdrl_spectrum1D_delete(&ori_s);
    hdrl_spectrum1D_delete(&integrated_s);
    cpl_array_delete(wavs_integrate);
    cpl_array_delete(bads);
}

void test15(void){

    hdrl_parameter * par = hdrl_spectrum1D_resample_integrate_parameter_create();

    hdrl_spectrum1D * ori_s = generate_stair_spectrum(1, 3, 20.5, 1.0);

    double wa_mx = hdrl_spectrum1D_get_wavelength_value(ori_s, 2, NULL);
    cpl_test_rel(wa_mx, 22.5, HDRL_DELTA_COMPARE_VALUE);

    cpl_array * wavs_integrate = get_waves(22.5, 5, 2);
    hdrl_spectrum1D * integrated_s =
           hdrl_spectrum1D_resample_on_array(ori_s, wavs_integrate, par);

    for(cpl_size i = 0; i < cpl_array_get_size(wavs_integrate); ++i){
        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);
        cpl_test(rej);
        cpl_test(isnan(f.data));
        cpl_test(isnan(f.error));
    }

    hdrl_parameter_delete(par);
    hdrl_spectrum1D_delete(&ori_s);
    hdrl_spectrum1D_delete(&integrated_s);
    cpl_array_delete(wavs_integrate);
}

void test16(void){

    hdrl_parameter * par = hdrl_spectrum1D_resample_integrate_parameter_create();

    hdrl_spectrum1D * ori_s = generate_stair_spectrum(1, 3, 20.5, 1.0);

    cpl_array * wavs_integrate = get_waves(14.5, 4, 2);

    double wa_mx = cpl_array_get(wavs_integrate, 3, NULL);
    cpl_test_rel(wa_mx, 20.5, HDRL_DELTA_COMPARE_VALUE);

    hdrl_spectrum1D * integrated_s =
           hdrl_spectrum1D_resample_on_array(ori_s, wavs_integrate, par);

    for(cpl_size i = 0; i < cpl_array_get_size(wavs_integrate); ++i){
        int rej = 0;
        const hdrl_value f = hdrl_spectrum1D_get_flux_value(integrated_s, i, &rej);
        cpl_test(rej);
        cpl_test(isnan(f.data));
        cpl_test(isnan(f.error));
    }

    hdrl_parameter_delete(par);
    hdrl_spectrum1D_delete(&ori_s);
    hdrl_spectrum1D_delete(&integrated_s);
    cpl_array_delete(wavs_integrate);
}

void test17(void){

    hdrl_parameter * par = hdrl_spectrum1D_resample_integrate_parameter_create();
    cpl_array * bads = cpl_array_new(3, CPL_TYPE_INT);

    cpl_array_set(bads, 0, 0);
    cpl_array_set(bads, 1, 4);
    cpl_array_set(bads, 2, 7);

    hdrl_spectrum1D * ori_s = generate_bad_stair_spectrum(1, 8, 20.5, 1.0, bads);

    const cpl_array * wavs_integrate =
                    hdrl_spectrum1D_get_wavelength(ori_s).wavelength;

    hdrl_spectrum1D * integrated_s =
           hdrl_spectrum1D_resample_on_array(ori_s, wavs_integrate, par);

    for(cpl_size i = 0; i < cpl_array_get_size(wavs_integrate); ++i){
        int ori_rej = 0;
        const hdrl_value ori_flx = hdrl_spectrum1D_get_flux_value(ori_s, i, &ori_rej);

        int int_rej = 0;
        const hdrl_value int_flx = hdrl_spectrum1D_get_flux_value(integrated_s, i, &int_rej);

        cpl_test_eq(ori_rej, int_rej);
        if(ori_rej) continue;

        cpl_test_rel(ori_flx.data, int_flx.data, HDRL_DELTA_COMPARE_VALUE);
        cpl_test_rel(ori_flx.error, int_flx.error, HDRL_DELTA_COMPARE_VALUE);
    }

    hdrl_parameter_delete(par);
    hdrl_spectrum1D_delete(&ori_s);
    hdrl_spectrum1D_delete(&integrated_s);
    cpl_array_delete(bads);
}

double get_bin_size(const hdrl_spectrum1D * s, const cpl_size i){

    if(i == 0){
        const double w0 = hdrl_spectrum1D_get_wavelength_value(s, i,  NULL);
        const double w1 = hdrl_spectrum1D_get_wavelength_value(s, i + 1,  NULL);
        return (w0 + w1) / 2 - w0;
    }

    const cpl_size sz = hdrl_spectrum1D_get_size(s);
    if(i == sz - 1){
        const double w0 = hdrl_spectrum1D_get_wavelength_value(s, i - 1,  NULL);
        const double w1 = hdrl_spectrum1D_get_wavelength_value(s, i,  NULL);
        return (w0 + w1) / 2 - w0;
    }

    const double w0 = hdrl_spectrum1D_get_wavelength_value(s, i - 1,  NULL);
    const double w1 = hdrl_spectrum1D_get_wavelength_value(s, i + 1,  NULL);
    return (w0 + w1) / 2 - w0;
}

double calc_total_flux(const hdrl_spectrum1D * s){
    double flux = 0;
    const cpl_size sz = hdrl_spectrum1D_get_size(s);
    for(cpl_size i = 0; i < sz; ++i)
        flux += hdrl_spectrum1D_get_flux_value(s, i, NULL).data * get_bin_size(s, i);

    return flux;
}

void test18(void){


    hdrl_parameter * par = hdrl_spectrum1D_resample_integrate_parameter_create();

    hdrl_spectrum1D * ori_s = generate_stair_spectrum(1, 4, 20.0, 2.0);

    cpl_array * wavs_integrate = get_waves(20, 3, 3);

    hdrl_spectrum1D * integrated_s =
              hdrl_spectrum1D_resample_on_array(ori_s, wavs_integrate, par);

    cpl_test_rel(calc_total_flux(ori_s), 15.0, HDRL_DELTA_COMPARE_VALUE);

    cpl_test_rel(calc_total_flux(ori_s), calc_total_flux(integrated_s), HDRL_DELTA_COMPARE_VALUE);

    hdrl_parameter_delete(par);
    hdrl_spectrum1D_delete(&ori_s);
    hdrl_spectrum1D_delete(&integrated_s);
    cpl_array_delete(wavs_integrate);
}

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_spectrum1D    Test for resampling done using integration
 */
/*----------------------------------------------------------------------------*/
void test_spectrum1D_resample_spectrum_integrate(void){

    /*tests 1 to 6 test the case of upsampling doing integration*/
    /*test when the destination spectrum starts after the source and ends before
     * the source end.*/
    test1();

    /*test when source and destination points start and stop at the same bin*/
    test2();

    /*test when destination points cover a wider range of the source*/
    test3();

    /*like test1 but the output and input values are shuffled*/
    test4();

    /*like test3 but we have bad pixels inside (and the edges) of the spectrum*/
    test5();

    /* Test for rebinning, the dest bins split exactly source bin in half,
     * except the last one which is outside*/
    test6();

    /*Test like test6 but with bad pixels*/
    test7();

    /*The following test test the case of dest bins bigger than source*/

    /*Test when the destination spectrum starts after the source and ends before
     * the source end.*/
    test8();

    /*test when source and destination points stop at the same bin*/
    test9();

    /*test when destination points cover a wider range of the source*/
    test10();

    /*like test 10 but we have a bad pixel*/
    test11();
    test12();
    test13();

    /* Test for rebinning, the dest bins split exactly source bin in half,
     * except the last one which is outside, with bad pixels*/
    test14();

    /*Source and destination have empty intersections*/
    test15();
    test16();

    /*Source and destination with same bins*/
    test17();

    /*Flux conservation*/
    test18();

    cpl_test_eq(cpl_error_get_code(), CPL_ERROR_NONE);
}

void test_parlist(void)
{
    /* parameter parsing smoketest */
	const char *base_context = "RECIPE";
	const char *prefix       = "test";
	const char *method_def   = "LINEAR";

    cpl_parameterlist *pos;
    pos = hdrl_spectrum1D_resample_interpolate_parameter_create_parlist(base_context, prefix, method_def);
    cpl_test_error(CPL_ERROR_NONE);

    hdrl_parameter *hpar;
    hpar = hdrl_spectrum1D_resample_interpolate_parameter_parse_parlist(
    												pos, "RECIPE.test");
    cpl_test_error(CPL_ERROR_NONE);

    hdrl_parameter_delete(hpar);

    cpl_parameterlist_delete(pos);
}

/*----------------------------------------------------------------------------*/
/**
  @brief   Unit tests of Spectrum1D module
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    srand(500);

    test_parlist();

    test_spectrum1D_constructor(CPL_TYPE_DOUBLE);
    test_spectrum1D_constructor(CPL_TYPE_FLOAT);
    test_spectrum1D_constructor_error();
    test_spectrum1D_constructor_analytical();
    test_spectrum1D_duplication();


    test_spectrum1D_calculation_scalar();

    test_spectrum1D_calculation();
    test_spectrum1D_calculation_error();

    test_spectrum1D_conversion_wavelength_scale();
    test_spectrum1D_mul_wavelength();
    test_spectrum1D_shift_wavelength();

    test_spectrum1D_wavelength_select();

    test_spectrum1D_resample_spectrum(CPL_TRUE);
    test_spectrum1D_resample_spectrum(CPL_FALSE);
    test_spectrum1D_resample_spectrum_private_funcs();
    test_spectrum1D_resample_spectrum_bpm(CPL_TRUE);
    test_spectrum1D_resample_spectrum_bpm(CPL_FALSE);
    test_spectrum1D_resample_spectrum_interpolation_error_test();
    test_spectrum1D_resample_spectrum_fit_error_test_shift(CPL_TRUE);
    test_spectrum1D_resample_spectrum_fit_error_test_shift(CPL_FALSE);
    test_spectrum1D_resample_spectrum_fit_error_test_error_interpol();
    test_spectrum1D_resample_spectrum_fit_windowed();

    test_spectrum1D_resample_spectrum_integrate();

    test_spectrum1D_table_conversion();

    test_spectrum1D_test_uniformly_sampled();

    test_spectrum1Dlist();

    return cpl_test_end(0);
}

/*----------------------------------------------------------------------------*/
/**
  @brief   Helper functions
 **/
/*----------------------------------------------------------------------------*/

static inline cpl_image * get_random_1d_img(cpl_size length,
        double min, double max, cpl_type type){
    cpl_image * to_ret = cpl_image_new(length, 1, type);

    for(cpl_size i = 0; i < length; i++){
        double d = rand_0_to_1() * (max - min) + min;
        cpl_image_set(to_ret, i + 1, 1, d);
    }
    return to_ret;
}

static inline void set_1d_bpm(cpl_image * img){

    int sz_x = cpl_image_get_size_x(img);
    int sz_y = cpl_image_get_size_y(img);

    cpl_mask * msk = cpl_image_get_bpm(img);

    for(cpl_size x = 0; x < sz_x; x++){
        for(cpl_size y = 0; y < sz_y; y++){
            cpl_mask_set(msk, x + 1, y + 1, rand_0_to_1() > .5);
        }
    }

}

static inline cpl_array * get_wavelength(cpl_size length, cpl_type type){

    cpl_array * to_ret = cpl_array_new(length, type);
    double d = rand_0_to_1();

    for(cpl_size i = 0; i < length; i++){
        cpl_array_set(to_ret, i, d);
        d += 1.0 + rand_0_to_1();
    }
    return to_ret;
}

static inline cpl_boolean are_cpl_img_eq(
        const cpl_image *im1, const cpl_image *im2){

	if (cpl_image_get_size_x(im1) != cpl_image_get_size_x(im2) ||
	    cpl_image_get_size_y(im1) != cpl_image_get_size_y(im2) ){
		return CPL_FALSE;
	}

    cpl_size sz_x = cpl_image_get_size_x(im1);
    cpl_size sz_y = cpl_image_get_size_y(im1);

    int rej1 = 0;
    int rej2 = 0;

    for(cpl_size x = 0; x < sz_x; x++){
        for(cpl_size y = 0; y < sz_y; y++){
            double px1 = cpl_image_get(im1, x + 1, y + 1, &rej1);
            double px2 = cpl_image_get(im2, x + 1, y + 1, &rej2);

            if(px1 != px2 || rej1 != rej2) {
            	return CPL_FALSE;
            }
        }
    }

    return CPL_TRUE;
}

static inline cpl_boolean are_hdrl_eq(const hdrl_image* flux_compound,
        const cpl_image * flux, const cpl_image * flux_e){

    if (cpl_image_get_size_x(flux) != cpl_image_get_size_x(flux_e) ||
    	cpl_image_get_size_y(flux) != cpl_image_get_size_y(flux_e) ||
		hdrl_image_get_size_x(flux_compound) != cpl_image_get_size_x(flux) ||
		hdrl_image_get_size_y(flux_compound) != cpl_image_get_size_y(flux) ){
        return CPL_FALSE;
    }

    const cpl_image * flux_hdrl = hdrl_image_get_image_const(flux_compound);
    const cpl_image * flux_e_hdrl = hdrl_image_get_error_const(flux_compound);

    hdrl_image * hdrl_img = hdrl_image_create(flux, flux_e);

    cpl_boolean is_success =
            are_cpl_img_eq(hdrl_image_get_image_const(hdrl_img), flux_hdrl);

    is_success  &= are_cpl_img_eq(hdrl_image_get_error_const(hdrl_img),
                    flux_e_hdrl);

    hdrl_image_delete(hdrl_img); hdrl_img = NULL;

    return is_success;
}

static inline cpl_error_code get_error_code_and_reset(void){

    cpl_error_code err = cpl_error_get_code();
    cpl_error_reset();
    return err;
}

static inline double rand_0_to_1(void){
    double r = rand();
    r /= RAND_MAX;
    return r;
}

static inline hdrl_spectrum1D *
get_random_spectrum(int length, hdrl_spectrum1D_wave_scale scale){

    cpl_image * spectrum1d =
                get_random_1d_img(length, 1.0f, 128.0f, CPL_TYPE_DOUBLE);
    cpl_image * spectrum1d_error =
                    get_random_1d_img(length, 0.5f, 2.0f, CPL_TYPE_DOUBLE);

    set_1d_bpm(spectrum1d);
    cpl_array * wavelengths = get_wavelength(length, CPL_TYPE_DOUBLE);

    hdrl_spectrum1D * s1 = hdrl_spectrum1D_create(
            spectrum1d, spectrum1d_error,
            wavelengths, scale);

    cpl_array_delete(wavelengths);
    cpl_image_delete(spectrum1d);
    cpl_image_delete(spectrum1d_error);

    return s1;
}


static inline void
test_error_create_func(const hdrl_spectrum1D * s1, const hdrl_spectrum1D * s2,
        operate_spectra_create f){

    hdrl_spectrum1D * res = f(s1, s2);
    cpl_test_null(res);
    const cpl_error_code cd = get_error_code_and_reset();
    cpl_test_noneq(cd, CPL_ERROR_NONE);
}

static inline void
test_error_mutate_func(hdrl_spectrum1D * s1, const hdrl_spectrum1D * s2,
        operate_spectra f){

    const cpl_error_code res = f(s1, s2);
    cpl_test_noneq(res, CPL_ERROR_NONE);

    const cpl_error_code cd = get_error_code_and_reset();
    cpl_test_noneq(cd, CPL_ERROR_NONE);
}

static inline void
test_calc_creat_error(operate_spectra_create f){

    hdrl_spectrum1D * spec_l40_linear =
            get_random_spectrum(40, hdrl_spectrum1D_wave_scale_linear);
    hdrl_spectrum1D * spec_l40_log =
            get_random_spectrum(40, hdrl_spectrum1D_wave_scale_log);

    hdrl_spectrum1D * spec_l41_linear =
            get_random_spectrum(41, hdrl_spectrum1D_wave_scale_linear);
    hdrl_spectrum1D * spec_l41_log =
            get_random_spectrum(41, hdrl_spectrum1D_wave_scale_log);

    test_error_create_func
    (spec_l40_linear, spec_l40_log, f);
    test_error_create_func
    (spec_l41_linear, spec_l40_linear, f);
    test_error_create_func
    (spec_l40_log, spec_l40_linear, f);
    test_error_create_func
    (spec_l40_linear, spec_l41_linear, f);

    test_error_create_func
    (NULL, spec_l40_log, f);
    test_error_create_func
    (spec_l41_linear, NULL, f);
    test_error_create_func(NULL, NULL, f);


    hdrl_spectrum1D_delete(&spec_l40_linear);
    hdrl_spectrum1D_delete(&spec_l41_linear);

    hdrl_spectrum1D_delete(&spec_l40_log);
    hdrl_spectrum1D_delete(&spec_l41_log);
}


static inline void test_calc_error(operate_spectra f){

    hdrl_spectrum1D * spec_l40_linear =
               get_random_spectrum(40, hdrl_spectrum1D_wave_scale_linear);
    hdrl_spectrum1D * spec_l40_log =
               get_random_spectrum(40, hdrl_spectrum1D_wave_scale_log);

    hdrl_spectrum1D * spec_l41_linear =
               get_random_spectrum(41, hdrl_spectrum1D_wave_scale_linear);
    hdrl_spectrum1D * spec_l41_log =
               get_random_spectrum(41, hdrl_spectrum1D_wave_scale_log);

    test_error_mutate_func
    (spec_l40_linear, spec_l40_log, f);
    test_error_mutate_func
    (spec_l41_linear, spec_l40_linear, f);
    test_error_mutate_func
    (spec_l40_log, spec_l40_linear, f);
    test_error_mutate_func
    (spec_l40_linear, spec_l41_linear, f);

    test_error_mutate_func
    (NULL, spec_l40_log, f);
    test_error_mutate_func
    (spec_l41_linear, NULL, f);
    test_error_mutate_func(NULL, NULL, f);

    hdrl_spectrum1D_delete(&spec_l40_linear);
    hdrl_spectrum1D_delete(&spec_l41_linear);

    hdrl_spectrum1D_delete(&spec_l40_log);
    hdrl_spectrum1D_delete(&spec_l41_log);
}

static inline hdrl_spectrum1D * get_spectrum1D_sin_shuffled(cpl_size sz, int start,
        cpl_boolean add_peak, cpl_array ** unshuffled_lambda){

    static const double peak = 100.0;
    const double delta = 2 * CPL_MATH_PI / sz;

    cpl_array * lambda = cpl_array_new(sz, HDRL_TYPE_DATA);
    cpl_image * flux = cpl_image_new(sz, 1, HDRL_TYPE_DATA);

    for(cpl_size i = 0; i < sz; i++){
        double l = delta * (i + start);
        double f = fabs(peak * (sin(l) + 1.1));

        if(i == 4 && add_peak) f *= 1.5;

        cpl_array_set(lambda, i, l);
        cpl_image_set(flux, i + 1, 1, f);
    }

    if(unshuffled_lambda)
        *unshuffled_lambda = cpl_array_duplicate(lambda);

    /* scramble array */
    for(cpl_size i1 = 0; i1 < sz; i1++){
        int rej;
        double l1 = cpl_array_get(lambda, i1, &rej);
        double f1 = cpl_image_get(flux, i1 + 1, 1, &rej);

        cpl_size i2 = (cpl_size)(rand_0_to_1() * (sz - 1));

        double l2 = cpl_array_get(lambda, i2, &rej);
        double f2 = cpl_image_get(flux, i2 + 1, 1, &rej);

        cpl_array_set(lambda, i1, l2);
        cpl_image_set(flux, i1 + 1, 1, f2);

        cpl_array_set(lambda, i2, l1);
        cpl_image_set(flux, i2 + 1, 1, f1);
    }


    hdrl_spectrum1D * sp =
        hdrl_spectrum1D_create_error_DER_SNR
        (flux, 10, lambda, hdrl_spectrum1D_wave_scale_linear);

    cpl_test_nonnull(sp);
    cpl_test_eq(get_error_code_and_reset(), CPL_ERROR_NONE);


    cpl_array_delete(lambda);
    cpl_image_delete(flux);

    return sp;
}
