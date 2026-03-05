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
#include "hdrl_spectrum_resample.h"

#include <math.h>


hdrl_spectrum1D * create_spectrum(double fx){
    cpl_image * flx = cpl_image_new(1, 1, HDRL_TYPE_DATA);
    cpl_array * wln = cpl_array_new(1, HDRL_TYPE_DATA);
    cpl_image_set(flx, 1, 1 , fx);
    cpl_array_set(wln, 0, 100);
    hdrl_spectrum1D * s = hdrl_spectrum1D_create_error_free(flx, wln,
            hdrl_spectrum1D_wave_scale_linear);

    cpl_array_delete(wln);
    cpl_image_delete(flx);

    return s;
}

void check_equal(const hdrl_spectrum1D * s1, const hdrl_spectrum1D * s2){

    const cpl_size sz = hdrl_spectrum1D_get_size(s1);
    cpl_test_eq(sz, hdrl_spectrum1D_get_size(s2));

    for(cpl_size i = 0; i < sz; ++i){
        int rej1 = 0, rej2 = 0;
        hdrl_value v1 = hdrl_spectrum1D_get_flux_value(s1, i, &rej1);
        hdrl_value v2 = hdrl_spectrum1D_get_flux_value(s2, i, &rej2);

        cpl_test_eq(rej1, rej2);
        cpl_test_rel(v1.data, v2.data, 1e-5);
        cpl_test_rel(v1.error, v2.error, 1e-5);
    }
}

void check_list_sequential(const hdrl_spectrum1Dlist * list,
        cpl_size idx, double mlx){

    for(cpl_size i = 0; i < hdrl_spectrum1Dlist_get_size(list); ++i){
        const hdrl_spectrum1D * s = hdrl_spectrum1Dlist_get_const(list, i);
        hdrl_value v  = hdrl_spectrum1D_get_flux_value(s, 0, NULL);
        cpl_test_rel(v.data, idx * mlx, 1e-10);
        idx++;
    }

}

void test_spectrum1dlist_wrap(void){
    hdrl_spectrum1D ** sl = cpl_calloc(6, sizeof(hdrl_spectrum1D*));

    sl[0] = create_spectrum(1);
    sl[1] = create_spectrum(2);
    sl[2] = create_spectrum(3);
    sl[3] = create_spectrum(4);
    sl[4] = create_spectrum(5);
    sl[5] = create_spectrum(6);

    hdrl_spectrum1Dlist * list = hdrl_spectrum1Dlist_wrap(sl, 6);

    cpl_test_eq(hdrl_spectrum1Dlist_get_size(list), 6);

    for(cpl_size i = 0; i < hdrl_spectrum1Dlist_get_size(list); ++i){
        const hdrl_spectrum1D * s = hdrl_spectrum1Dlist_get_const(list, i);
        cpl_test_eq_ptr(s, sl[i]);
    }

    hdrl_spectrum1Dlist_delete(list);
}

void test_spectrum1dlist(void){

    hdrl_spectrum1Dlist * list1 = hdrl_spectrum1Dlist_new();

    cpl_test_eq(0, hdrl_spectrum1Dlist_get_size(list1));

    hdrl_spectrum1D * s4 = create_spectrum(4);

    {
        hdrl_spectrum1D * s1 = create_spectrum(1);
        hdrl_spectrum1D * s11 = create_spectrum(1);
        hdrl_spectrum1D * s2 = create_spectrum(2);
        hdrl_spectrum1D * s3 = create_spectrum(3);
        hdrl_spectrum1D * s5 = create_spectrum(5);
        hdrl_spectrum1D * s6 = create_spectrum(6);

        hdrl_spectrum1Dlist_set(list1, s1, 0);
        /*Removed s1 when setting*/
        hdrl_spectrum1Dlist_set(list1, s11, 0);
        hdrl_spectrum1Dlist_set(list1, s2, 1);
        hdrl_spectrum1Dlist_set(list1, s3, 2);
        hdrl_spectrum1Dlist_set(list1, s4, 3);
        hdrl_spectrum1Dlist_set(list1, s5, 4);
        hdrl_spectrum1Dlist_set(list1, s6, 5);
    }

    cpl_test_eq(6, hdrl_spectrum1Dlist_get_size(list1));

    /* Check duplication */
    hdrl_spectrum1Dlist * list2 = hdrl_spectrum1Dlist_duplicate(list1);

    cpl_test_eq(hdrl_spectrum1Dlist_get_size(list1),
            hdrl_spectrum1Dlist_get_size(list2));

    for(cpl_size i = 0; i < hdrl_spectrum1Dlist_get_size(list1); ++i){
        const hdrl_spectrum1D * s1 = hdrl_spectrum1Dlist_get_const(list1, i);
        const hdrl_spectrum1D * s2 = hdrl_spectrum1Dlist_get_const(list2, i);

        check_equal(s1, s2);

        /*check that list1 and list2 point to different memory spaces*/
        cpl_test_noneq_ptr(s1, s2);
    }

    /*Check getters (mutability)*/
    for(cpl_size i = 0; i < hdrl_spectrum1Dlist_get_size(list1); ++i){
        hdrl_spectrum1D * s = hdrl_spectrum1Dlist_get(list2, i);
        hdrl_value v  = hdrl_spectrum1D_get_flux_value(s, 0, NULL);
        cpl_test_rel(v.data, (double)i + 1.0 , 1e-5);
        hdrl_spectrum1D_mul_scalar(s, (hdrl_value){5.0, 0.0});
    }

    for(cpl_size i = 0; i < hdrl_spectrum1Dlist_get_size(list1); ++i){
        hdrl_spectrum1D * s = hdrl_spectrum1Dlist_get(list2, i);
        hdrl_value v  = hdrl_spectrum1D_get_flux_value(s, 0, NULL);
        cpl_test_rel(v.data, ((double)i + 1.0) * 5.0 , 1e-5);
    }

    hdrl_spectrum1D * new_s4 = hdrl_spectrum1Dlist_unset(list1, 3);

    cpl_test_eq_ptr(new_s4, s4);

    cpl_test_eq(hdrl_spectrum1Dlist_get_size(list1), 5);

    const double flx_value[] = {1, 2, 3, 5, 6};
    for(cpl_size i = 0; i < hdrl_spectrum1Dlist_get_size(list1); ++i){

        const hdrl_spectrum1D * s = hdrl_spectrum1Dlist_get_const(list1, i);
        hdrl_value v  = hdrl_spectrum1D_get_flux_value(s, 0, NULL);
        cpl_test_rel(v.data, flx_value[i], 1e-5);
    }

    cpl_size i = 1;
    while(hdrl_spectrum1Dlist_get_size(list2)){
        hdrl_spectrum1D * s = hdrl_spectrum1Dlist_unset(list2, 0);
        check_list_sequential(list2, i + 1, 5.0);
        hdrl_value v  = hdrl_spectrum1D_get_flux_value(s, 0, NULL);
        cpl_test_rel(v.data, i * 5.0, 1e-5);
        hdrl_spectrum1D_delete(&s);
        i++;
    }

    hdrl_spectrum1Dlist_delete(list2);
    hdrl_spectrum1Dlist_delete(list1);
    hdrl_spectrum1D_delete(&s4);
}
 /*data[i] is both wavelength and flux. if data[i] the i-th pixel is rejected*/
hdrl_spectrum1D *
create_spectrum_long(const double * data, const cpl_size length){
    cpl_array * wlens = cpl_array_new(length, HDRL_TYPE_DATA);
    cpl_image * flx = cpl_image_new(length, 1, HDRL_TYPE_DATA);

    for(cpl_size i = 0; i < length; ++i){
        cpl_array_set(wlens, i, fabs(data[i]));

        if(data[i] >= 0)
            cpl_image_set(flx, i + 1, 1, data[i]);
        else
            cpl_image_reject(flx, i + 1, 1);
    }

    hdrl_spectrum1D * s = hdrl_spectrum1D_create_error_free(flx, wlens,
            hdrl_spectrum1D_wave_scale_linear);

    cpl_image_delete(flx);
    cpl_array_delete(wlens);
    return s;
}

/*check that bad pixels at the beginning or at the end of spectrum
 * do not contribute to the collapsed spectrum*/
void test_spectrum1dlist_collapse_badpix(void){

    hdrl_spectrum1Dlist * l = hdrl_spectrum1Dlist_new();

    double aValues1[4] = {1, 2, 3, 4};
    hdrl_spectrum1Dlist_set(l, create_spectrum_long(aValues1, 4), 0);

    double aValues2[3] = {-1, 2, 4};
    hdrl_spectrum1Dlist_set(l, create_spectrum_long(aValues2, 3), 1);

    double aValues3[3] = {1, 3, -4};
    hdrl_spectrum1Dlist_set(l, create_spectrum_long(aValues3, 3), 2);

    cpl_array * wlenghts = cpl_array_new(6, HDRL_TYPE_DATA);
    for(cpl_size i = 0; i < cpl_array_get_size(wlenghts); ++i){
        cpl_array_set(wlenghts, i, i);
    }

    hdrl_parameter * stacking_par = hdrl_collapse_mean_parameter_create();
    hdrl_parameter * resampling_par =
            hdrl_spectrum1D_resample_interpolate_parameter_create(hdrl_spectrum1D_interp_linear);

    hdrl_spectrum1D * res = NULL;
    cpl_image * contrib = NULL;
    hdrl_imagelist * aligned_fluxes = NULL;

    hdrl_spectrum1Dlist_collapse(l, stacking_par, wlenghts, resampling_par, CPL_FALSE,
            &res, &contrib, &aligned_fluxes);

    int contribs[] = {2, 3, 3, 2};
    for(cpl_size i = 1; i < 5; ++i){
        int rej = 0;
        const double el = cpl_image_get(contrib, i + 1, 1, &rej);
        cpl_test_eq(rej, 0);
        cpl_test_eq(el, contribs[i - 1]);
    }

    int reject = 0;
    double el = cpl_image_get(contrib, 6, 1, &reject);
    cpl_test_eq(reject, 0);
    cpl_test_eq(el, 0);

    reject = 0;
    el = cpl_image_get(contrib, 1, 1, &reject);
    cpl_test_eq(reject, 0);
    cpl_test_eq(el, 0);

    const cpl_size sz = hdrl_spectrum1D_get_size(res);
    cpl_test_eq(cpl_array_get_size(wlenghts), sz);

    for(cpl_size i = 0; i < sz; ++i){
        int rej = 0;
        const hdrl_value v = hdrl_spectrum1D_get_flux_value(res, i, &rej);
        if(i == 0 || i == (sz - 1)){
            cpl_test_eq(rej, 1);
            continue;
        }
        cpl_test_rel(v.data, i, 1e-5);
        cpl_test_eq(rej, 0);
    }

    hdrl_spectrum1Dlist_delete(l);
    hdrl_spectrum1D_delete(&res);
    cpl_image_delete(contrib);
    hdrl_parameter_delete(stacking_par);
    hdrl_parameter_delete(resampling_par);
    hdrl_imagelist_delete(aligned_fluxes);
    cpl_array_delete(wlenghts);
}

/*Check that resamplking pixels having rejected neighbors in the original spectrum
 * do not contribute to the stacking*/
void test_spectrum1dlist_collapse_mark_rej_in_interpolation(void){

    hdrl_spectrum1Dlist * l = hdrl_spectrum1Dlist_new();

    double aValues1[4] = {1, 2, 3, 4};
    hdrl_spectrum1Dlist_set(l, create_spectrum_long(aValues1, 4), 0);

    double aValues2[4] = {-1, 2, -3, 4};
    hdrl_spectrum1Dlist_set(l, create_spectrum_long(aValues2, 4), 1);

    double aValues3[4] = {1,-2, 3, -4};
    hdrl_spectrum1Dlist_set(l, create_spectrum_long(aValues3, 4), 2);

    cpl_array * wlenghts = cpl_array_new(6, HDRL_TYPE_DATA);
    for(cpl_size i = 0; i < cpl_array_get_size(wlenghts); ++i){
        cpl_array_set(wlenghts, i, i);
    }

    hdrl_parameter * stacking_par = hdrl_collapse_mean_parameter_create();
    hdrl_parameter * resampling_par =
            hdrl_spectrum1D_resample_interpolate_parameter_create(hdrl_spectrum1D_interp_linear);

    hdrl_spectrum1D * res = NULL;
    cpl_image * contrib = NULL;
    hdrl_imagelist * aligned_fluxes = NULL;

    hdrl_spectrum1Dlist_collapse(l, stacking_par, wlenghts, resampling_par, CPL_TRUE,
            &res, &contrib, &aligned_fluxes);

    int contribs[] = {2, 2, 2, 2};
    for(cpl_size i = 1; i < 5; ++i){
        int reject = 0;
        const double el = cpl_image_get(contrib, i + 1, 1, &reject);
        cpl_test_eq(reject, 0);
        cpl_test_eq(el, contribs[i - 1]);
    }

    int rej_contr = 0;
    double el = cpl_image_get(contrib, 6, 1, &rej_contr);
    cpl_test_eq(rej_contr, 0);
    cpl_test_eq(el, 0);

    rej_contr = 0;
    el = cpl_image_get(contrib, 1, 1, &rej_contr);
    cpl_test_eq(rej_contr, 0);
    cpl_test_eq(el, 0);

    const cpl_size sz = hdrl_spectrum1D_get_size(res);
    cpl_test_eq(cpl_array_get_size(wlenghts), sz);

    for(cpl_size i = 0; i < sz; ++i){
        int rej = 0;
        const hdrl_value v = hdrl_spectrum1D_get_flux_value(res, i, &rej);
        if(i == 0 || i == (sz - 1)){
            cpl_test_eq(rej, 1);
            continue;
        }
        cpl_test_rel(v.data, i, 1e-5);
        cpl_test_eq(rej, 0);
    }

    hdrl_spectrum1Dlist_delete(l);
    hdrl_spectrum1D_delete(&res);
    cpl_image_delete(contrib);
    hdrl_parameter_delete(stacking_par);
    hdrl_parameter_delete(resampling_par);
    hdrl_imagelist_delete(aligned_fluxes);
    cpl_array_delete(wlenghts);
}

/*check that shorted spectra are handled correctly*/
void test_spectrum1dlist_collapse_holes(void){

    hdrl_spectrum1Dlist * l = hdrl_spectrum1Dlist_new();

    double aValues1[4] = {1, 2, 3, 4};
    hdrl_spectrum1Dlist_set(l, create_spectrum_long(aValues1, 4), 0);

    double aValues2[2] = {2, 4};
    hdrl_spectrum1Dlist_set(l, create_spectrum_long(aValues2, 2), 1);

    double aValues3[2] = {1, 3};
    hdrl_spectrum1Dlist_set(l, create_spectrum_long(aValues3, 2), 2);

    cpl_array * wlenghts = cpl_array_new(6, HDRL_TYPE_DATA);
    for(cpl_size i = 0; i < cpl_array_get_size(wlenghts); ++i){
        cpl_array_set(wlenghts, i, i);
    }

    hdrl_parameter * stacking_par = hdrl_collapse_mean_parameter_create();
    hdrl_parameter * resampling_par =
            hdrl_spectrum1D_resample_interpolate_parameter_create(hdrl_spectrum1D_interp_linear);

    hdrl_spectrum1D * res = NULL;
    cpl_image * contrib = NULL;
    hdrl_imagelist * aligned_fluxes = NULL;
    hdrl_spectrum1Dlist_collapse(l, stacking_par, wlenghts, resampling_par,
            CPL_FALSE, &res, &contrib, &aligned_fluxes);

    int contribs[] = {2, 3, 3, 2};
    for(cpl_size i = 1; i < 5; ++i){
        int rej_contr = 0;
        const double el = cpl_image_get(contrib, i + 1, 1, &rej_contr);
        cpl_test_eq(rej_contr, 0);
        cpl_test_eq(el, contribs[i - 1]);
    }

    int rej_contrib = 0;
    double el = cpl_image_get(contrib, 6, 1, &rej_contrib);
    cpl_test_eq(rej_contrib, 0);
    cpl_test_eq(el, 0);

    rej_contrib = 0;
    el = cpl_image_get(contrib, 1, 1, &rej_contrib);
    cpl_test_eq(rej_contrib, 0);
    cpl_test_eq(el, 0);

    const cpl_size sz = hdrl_spectrum1D_get_size(res);
    cpl_test_eq(cpl_array_get_size(wlenghts), sz);

    for(cpl_size i = 0; i < sz; ++i){
        int rej = 0;
        const hdrl_value v = hdrl_spectrum1D_get_flux_value(res, i, &rej);
        if(i == 0 || i == (sz - 1)){
            cpl_test_eq(rej, 1);
            continue;
        }
        cpl_test_rel(v.data, i, 1e-5);
        cpl_test_eq(rej, 0);
    }

    hdrl_spectrum1Dlist_delete(l);
    hdrl_spectrum1D_delete(&res);
    cpl_image_delete(contrib);
    hdrl_imagelist_delete(aligned_fluxes);
    hdrl_parameter_delete(stacking_par);
    hdrl_parameter_delete(resampling_par);
    cpl_array_delete(wlenghts);
}

void test_error_and_reset(cpl_error_code expected){
    cpl_test_eq_error(expected, cpl_error_get_code());
    cpl_error_reset();
}

void test_spectrum1dlist_insert_duplication(void){

    hdrl_spectrum1Dlist * list1 = hdrl_spectrum1Dlist_new();

    cpl_test_eq(0, hdrl_spectrum1Dlist_get_size(list1));

    hdrl_spectrum1D * s1 = create_spectrum(1);
    hdrl_spectrum1D * s2 = create_spectrum(2);
    hdrl_spectrum1D * s3 = create_spectrum(3);
    hdrl_spectrum1D * s4 = create_spectrum(4);
    hdrl_spectrum1D * s5 = create_spectrum(5);
    hdrl_spectrum1D * s6 = create_spectrum(6);

    hdrl_spectrum1Dlist_set(list1, s1, 0);
    hdrl_spectrum1Dlist_set(list1, s2, 1);
    hdrl_spectrum1Dlist_set(list1, s3, 2);
    hdrl_spectrum1Dlist_set(list1, s4, 3);
    hdrl_spectrum1Dlist_set(list1, s5, 4);
    hdrl_spectrum1Dlist_set(list1, s6, 5);

    hdrl_spectrum1Dlist_set(list1, s1, 4);
    test_error_and_reset(CPL_ERROR_ILLEGAL_INPUT);

    hdrl_spectrum1Dlist_set(list1, s2, 3);
    test_error_and_reset(CPL_ERROR_ILLEGAL_INPUT);

    hdrl_spectrum1Dlist_set(list1, s3, 4);
    test_error_and_reset(CPL_ERROR_ILLEGAL_INPUT);

    hdrl_spectrum1Dlist_set(list1, s4, 5);
    test_error_and_reset(CPL_ERROR_ILLEGAL_INPUT);

    hdrl_spectrum1Dlist_set(list1, s5, 0);
    test_error_and_reset(CPL_ERROR_ILLEGAL_INPUT);

    hdrl_spectrum1Dlist_set(list1, s6, 2);
    test_error_and_reset(CPL_ERROR_ILLEGAL_INPUT);

    hdrl_spectrum1Dlist_delete(list1);
}

/*----------------------------------------------------------------------------*/
/**
  @brief   Unit tests of efficiency calculation module
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    test_spectrum1dlist();
    test_spectrum1dlist_wrap();
    test_spectrum1dlist_collapse_holes();
    test_spectrum1dlist_collapse_badpix();
    test_spectrum1dlist_collapse_mark_rej_in_interpolation();
    test_spectrum1dlist_insert_duplication();

    cpl_test_error(CPL_ERROR_NONE);

    return cpl_test_end(0);
}

