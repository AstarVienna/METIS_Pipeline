/*
 * This file is part of the HDRL
 * Copyright (C) 2014 European Southern Observatory
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

#include "hdrl_fit.c"
#include "hdrl_imagelist.h"
#include "hdrl_test.h"

#include <cpl.h>

#include <math.h>

#ifndef ARRAY_LEN
#define ARRAY_LEN(a) sizeof((a))/sizeof((a)[0])
#endif

/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_fit_test
            Testing of hdrl_fit module
 */
/*----------------------------------------------------------------------------*/

#define matrix_eq(m, exp, eps) \
    do { \
        for (size_t i = 0; i < (size_t)cpl_matrix_get_nrow(m); i++) { \
            for (size_t j = 0; j < (size_t)cpl_matrix_get_ncol(m); j++) { \
                cpl_test_abs(cpl_matrix_get(m, i, j), exp[i * \
                             cpl_matrix_get_ncol(m) + j], eps); \
            } \
        } \
    } while (0)

static void test_vander1d(void)
{
    {
        double p[] = {1, 2, 3, 4};
        double exp[] = {1., 1., 1.,
                        1., 2., 4.,
                        1., 3., 9.,
                        1., 4., 16.};
        cpl_vector * pv = cpl_vector_wrap(ARRAY_LEN(p), p);
        cpl_matrix * v = polyvander1d(pv, 2);
        cpl_test_error(CPL_ERROR_NONE);
        matrix_eq(v, exp, DBL_EPSILON * 5);
        cpl_matrix_delete(v);

        v = polyvander1d(pv, 2);
        cpl_test_error(CPL_ERROR_NONE);
        matrix_eq(v, exp, DBL_EPSILON * 5);
        cpl_matrix_delete(v);

        cpl_vector_unwrap(pv);
    }
    /* could be used for weight_design function */
    if (0) {
        double p[] = {1, 2, 3, 4};
        double e[] = {1, 1, 4, 1};
        double exp[] = {1.,   1.,   1.,
                        1.,   2.,   4.,
                        0.25, 0.75, 2.25,
                        1.,   4.,   16.};
        cpl_vector * pv = cpl_vector_wrap(ARRAY_LEN(p), p);
        cpl_vector * ev = cpl_vector_wrap(ARRAY_LEN(e), e);
        cpl_matrix * v = polyvander1d(pv, 2);
        cpl_test_error(CPL_ERROR_NONE);

        matrix_eq(v, exp, DBL_EPSILON * 5);
        cpl_vector_unwrap(pv);
        cpl_vector_unwrap(ev);
        cpl_matrix_delete(v);
    }
}

static void test_fit(void)
{
    {
        double s[] = {1, 2, 3, 4};
        double p[] = {2, 2.5, 3, 3.5};
        double exp[] = {1.5, 0.5, 0.};
        double eres[] = {0., 0., 0., 0.};
        cpl_vector * sv = cpl_vector_wrap(ARRAY_LEN(s), s);
        cpl_vector * pv = cpl_vector_wrap(ARRAY_LEN(p), p);
        cpl_vector * vre = cpl_vector_wrap(ARRAY_LEN(eres), eres);
        cpl_matrix * v = polyvander1d(sv, 2);
        hdrl_ls_fit_result * r = fit(v, pv, NULL);
        cpl_test_error(CPL_ERROR_NONE);
        matrix_eq(r->coef, exp, DBL_EPSILON * 10);
        cpl_vector * res = hdrl_ls_fit_result_get_residuals(r, pv);
        cpl_test_vector_abs(res, vre, DBL_EPSILON * 5);
        cpl_vector_unwrap(vre);
        cpl_vector_delete(res);
        cpl_matrix_delete(v);
        cpl_vector_unwrap(pv);
        cpl_vector_unwrap(sv);
        hdrl_ls_fit_result_delete(r);
    }
    {
        double s[] = {1, 2, 3, 4, 5};
        double p[] = {1.1, 2.5, 3.4, 3.8, 7};
        double e[] = {0.3, 0.2, 0.2, 0.1, 0.5};
        double exp[] = {0.54529, 0.858981};
        double cexp[] = {0.0756216, -0.0206541, -0.0206541, 0.00613226};
        cpl_vector * vfit = cpl_vector_new(ARRAY_LEN(s));
        cpl_vector * vres = cpl_vector_new(ARRAY_LEN(s));
        for (size_t i = 0; i < ARRAY_LEN(s); i++) {
            cpl_vector_set(vfit, i, exp[0] + exp[1] * s[i]);
            cpl_vector_set(vres, i,  p[i] - (exp[0] + exp[1] * s[i]));
        }

        cpl_vector * sv = cpl_vector_wrap(ARRAY_LEN(s), s);
        cpl_vector * pv = cpl_vector_wrap(ARRAY_LEN(p), p);
        cpl_vector * ev = cpl_vector_wrap(ARRAY_LEN(e), e);
        cpl_matrix * v = polyvander1d(sv, 1);

        hdrl_ls_fit_result * r = fit(v, pv, ev);
        cpl_test_error(CPL_ERROR_NONE);
        matrix_eq(r->coef, exp, DBL_EPSILON * 1e10);
        matrix_eq(r->cov, cexp, DBL_EPSILON * 1e10);

        cpl_vector * values = hdrl_ls_fit_result_get_fitted_values(r);
        cpl_test_vector_abs(values, vfit, DBL_EPSILON * 1e10);
        cpl_vector_delete(vfit);
        cpl_vector_delete(values);
        cpl_vector * resi = hdrl_ls_fit_result_get_residuals(r, pv);
        cpl_test_vector_abs(resi, vres, DBL_EPSILON * 1e10);
        cpl_vector_delete(vres);
        cpl_vector_delete(resi);
        cpl_matrix_delete(v);

        hdrl_ls_fit_result_delete(r);

        r = polyfit1d(sv, pv, ev, 1);
        cpl_test_error(CPL_ERROR_NONE);
        matrix_eq(r->coef, exp, DBL_EPSILON * 1e10);
        hdrl_ls_fit_result_delete(r);

        cpl_vector_unwrap(pv);
        cpl_vector_unwrap(ev);
        cpl_vector_unwrap(sv);
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief return prediction interval for the data
 *
 * @param r       fit result
 * @param data    data
 * @param errors  errors
 * @return        vector of symmetric prediction interval for data
 *
 * The prediction interval is the one sigma area in which new measurements from
 * the same setup are going to lie. It is not the error of the fitted
 * coefficients which is much smaller due to the use of all values from data to
 * compute them.
 */
/* ---------------------------------------------------------------------------*/
static inline cpl_vector * hdrl_ls_fit_result_get_fit_interval(
      const hdrl_ls_fit_result * r,
      const cpl_vector * data,
      cpl_vector * errors)
{
    /* mse = sum(sqrt(weights) * residuals ** 2) / df */
    double mse = hdrl_ls_fit_result_get_chi2(r, data, errors) /
        hdrl_ls_fit_result_get_residual_dof(r);
    /* var = mse / weights */
    cpl_vector  * serror = cpl_vector_duplicate(errors);
    cpl_vector_multiply(serror, serror);
    cpl_vector_multiply_scalar(serror, mse);
    cpl_vector_power(serror, 0.5);

    /* TODO: accounting for covariance missing
     * + (exog * np.dot(covb, exog.T).T).sum(axis=1) */

    return serror;
}


void test_poisson(void)
{
    double x[] = { 10.  , 62.1 , 114.2, 166.3, 218.4, 270.5, 322.6,
                   374.7, 426.8, 478.9, 531.1, 583.2, 635.3, 687.4, 739.5,
                   791.6, 843.7, 895.8, 947.9, 1000. };
    /* poisson data with lambda: x / 10 (== variance) and 100 offset */
    double y[] = { 103, 107, 111, 112, 117, 127, 126, 125, 139, 150, 157, 162,
                   153, 158, 162, 184, 191, 195, 182, 196 };
    cpl_vector * vx = cpl_vector_wrap(ARRAY_LEN(x), x);
    cpl_vector * vy = cpl_vector_wrap(ARRAY_LEN(y), y);
    /* relative model errors  (poisson model ~ sqrt(x)) */
    cpl_vector * ve_model = cpl_vector_duplicate(vx);
    cpl_vector_power(ve_model, 0.5);
    /* real absolute error of the population != relative model errors */
    cpl_vector * ve_real = cpl_vector_duplicate(vx);
    cpl_vector_divide_scalar(ve_real, 10);
    cpl_vector_power(ve_real, 0.5);


    double exp_c[] = {101.4164, 0.0919476};
    cpl_matrix * design = polyvander1d(vx, 1);
    hdrl_ls_fit_result * res = polyfit1d(vx, vy, ve_model, 1);
    matrix_eq(res->coef, exp_c, DBL_EPSILON * 2e12);
    /* get sample error */
    cpl_vector * pred_e =
        hdrl_ls_fit_result_get_fit_interval(res, vy, ve_model);
    /* should be less than < 20% deviation to population error */
    cpl_test_vector_abs(pred_e, ve_real, 0.7);
    cpl_vector_delete(pred_e);

    cpl_vector_unwrap(vx);
    cpl_vector_unwrap(vy);
    cpl_vector_delete(ve_model);
    cpl_vector_delete(ve_real);
    cpl_matrix_delete(design);
    hdrl_ls_fit_result_delete(res);
}

void test_imglistfit(void)
{
    {
        size_t n = 5;
        hdrl_imagelist * input = hdrl_imagelist_new();
        cpl_vector * samp = cpl_vector_new(n);
        hdrl_imagelist * out_coef = NULL;
        cpl_image * out_chi2 = NULL;
        cpl_image * out_dof = NULL;
        hdrl_fit_polynomial_imagelist(NULL, NULL, 0, NULL, NULL, NULL);
        cpl_test_error(CPL_ERROR_NULL_INPUT);

        hdrl_fit_polynomial_imagelist(input, NULL, 0, NULL, NULL, NULL);
        cpl_test_error(CPL_ERROR_NULL_INPUT);

        hdrl_fit_polynomial_imagelist(NULL, samp, 0, NULL, NULL, NULL);
        cpl_test_error(CPL_ERROR_NULL_INPUT);

        hdrl_fit_polynomial_imagelist(input, samp, 0, NULL, NULL, NULL);
        cpl_test_error(CPL_ERROR_NULL_INPUT);

        hdrl_fit_polynomial_imagelist(input, samp, -1, &out_coef, &out_chi2, &out_dof);
        cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

        hdrl_fit_polynomial_imagelist(input, samp, n + 2, &out_coef, &out_chi2, &out_dof);
        cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

        for (size_t i = 0; i < n; i++) {
            double t = (i + 1) * 100;
            hdrl_image * img = hdrl_image_new(10, 10);
            /* slight deviation from linear to get nonzero scale */
            hdrl_image_add_scalar(img,
                              (hdrl_value){0.5 * t + 100 - i, sqrt(0.5 * t)});
            if (i == 3) {
                hdrl_image_reject(img, 3, 4);
            }
            hdrl_imagelist_set(input, img, i);
            cpl_vector_set(samp, i, t);
        }

        hdrl_fit_polynomial_imagelist(input, samp, 1, &out_coef, &out_chi2, &out_dof);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_eq(cpl_image_get_type(out_dof), HDRL_TYPE_DATA);
        cpl_test_eq(cpl_image_get_type(out_chi2), HDRL_TYPE_DATA);
        cpl_test_eq(hdrl_imagelist_get_size(out_coef), 2);

        hdrl_image * coef0 = hdrl_imagelist_get(out_coef, 0);
        hdrl_image * expect = hdrl_image_new(10, 10);
        hdrl_image_add_scalar(expect, (hdrl_value){101, 9.0045});
        hdrl_image_set_pixel(expect, 3, 4, (hdrl_value){101, 9.29448});
        hdrl_test_image_abs(coef0, expect, HDRL_EPS_DATA * 1e11);
        hdrl_image_delete(expect);

        hdrl_image * coef1 = hdrl_imagelist_get(out_coef, 1);
        expect = hdrl_image_new(10, 10);
        hdrl_image_add_scalar(expect, (hdrl_value){0.49, 0.0351317});
        hdrl_image_set_pixel(expect, 3, 4, (hdrl_value){0.49, 0.0399607});
        hdrl_test_image_abs(coef1, expect, HDRL_EPS_DATA * 1e11);
        hdrl_image_delete(expect);

        cpl_image * cexpect = cpl_image_new(10, 10, HDRL_TYPE_DATA);
        /* dof*scaling between weights and measured error */
        cpl_image_add_scalar(cexpect, 1.831e-29);
        cpl_test_image_abs(out_chi2, cexpect, DBL_EPSILON * 1e9);
        cpl_image_delete(cexpect);

        cexpect = cpl_image_new(10, 10, HDRL_TYPE_DATA);
        cpl_image_add_scalar(cexpect, 3);
        cpl_image_set(cexpect, 3, 4, 2);
        cpl_test_image_abs(out_dof, cexpect, 0);
        cpl_image_delete(cexpect);

        hdrl_imagelist_delete(out_coef);
        cpl_image_delete(out_chi2);
        cpl_image_delete(out_dof);


        /* smoke test less good pixels than fit degree */
        hdrl_image_reject(hdrl_imagelist_get(input, 0), 2, 2);
        hdrl_image_reject(hdrl_imagelist_get(input, 1), 2, 2);
        hdrl_image_reject(hdrl_imagelist_get(input, 2), 2, 2);
        hdrl_image_reject(hdrl_imagelist_get(input, 3), 2, 2);
        hdrl_fit_polynomial_imagelist(input, samp, 3, &out_coef, &out_chi2, &out_dof);

        hdrl_imagelist_delete(input);
        cpl_vector_delete(samp);
        hdrl_imagelist_delete(out_coef);
        cpl_image_delete(out_chi2);
        cpl_image_delete(out_dof);
    }
}

void test_imglistfit2(void)
{
    {
        size_t n = 5;
        hdrl_imagelist * input = hdrl_imagelist_new();
        cpl_imagelist * samp = cpl_imagelist_new();
        hdrl_imagelist * out_coef = NULL;
        cpl_image * out_chi2 = NULL;
        cpl_image * out_dof = NULL;
        hdrl_fit_polynomial_imagelist2(NULL, NULL, 0, NULL, NULL, NULL);
        cpl_test_error(CPL_ERROR_NULL_INPUT);

        hdrl_fit_polynomial_imagelist2(input, NULL, 0, NULL, NULL, NULL);
        cpl_test_error(CPL_ERROR_NULL_INPUT);

        hdrl_fit_polynomial_imagelist2(NULL, samp, 0, NULL, NULL, NULL);
        cpl_test_error(CPL_ERROR_NULL_INPUT);

        hdrl_fit_polynomial_imagelist2(input, samp, 0, NULL, NULL, NULL);
        cpl_test_error(CPL_ERROR_NULL_INPUT);

        hdrl_fit_polynomial_imagelist2(input, samp, -1, &out_coef, &out_chi2, &out_dof);
        cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

        hdrl_fit_polynomial_imagelist2(input, samp, n + 2, &out_coef, &out_chi2, &out_dof);
        cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

        for (size_t i = 0; i < n; i++) {
            double t = (i + 1) * 100;
            hdrl_image * img = hdrl_image_new(10, 10);
            /* slight deviation from linear to get nonzero scale */
            hdrl_image_add_scalar(img,
                              (hdrl_value){0.5 * t + 100 - i, sqrt(0.5 * t)});
            hdrl_imagelist_set(input, img, i);
            cpl_image * sampi = cpl_image_new(10, 10, HDRL_TYPE_DATA);
            cpl_image_add_scalar(sampi, t);
            if (i == 3) {
                cpl_image_reject(sampi, 3, 4);
            }
            cpl_imagelist_set(samp, sampi, i);
        }

        hdrl_fit_polynomial_imagelist2(input, samp, 1, &out_coef, &out_chi2, &out_dof);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_eq(cpl_image_get_type(out_dof), HDRL_TYPE_DATA);
        cpl_test_eq(cpl_image_get_type(out_chi2), HDRL_TYPE_DATA);
        cpl_test_eq(hdrl_imagelist_get_size(out_coef), 2);

        hdrl_image * coef0 = hdrl_imagelist_get(out_coef, 0);
        hdrl_image * expect = hdrl_image_new(10, 10);
        hdrl_image_add_scalar(expect, (hdrl_value){101, 9.0045});
        hdrl_image_set_pixel(expect, 3, 4, (hdrl_value){101, 9.29448});
        hdrl_test_image_abs(coef0, expect, HDRL_EPS_DATA * 1e11);
        hdrl_image_delete(expect);

        hdrl_image * coef1 = hdrl_imagelist_get(out_coef, 1);
        expect = hdrl_image_new(10, 10);
        hdrl_image_add_scalar(expect, (hdrl_value){0.49, 0.0351317});
        hdrl_image_set_pixel(expect, 3, 4, (hdrl_value){0.49, 0.0399607});
        hdrl_test_image_abs(coef1, expect, HDRL_EPS_DATA * 1e11);
        hdrl_image_delete(expect);

        cpl_image * cexpect = cpl_image_new(10, 10, HDRL_TYPE_DATA);
        /* dof*scaling between weights and measured error */
        cpl_image_add_scalar(cexpect, 1.831e-29);
        cpl_test_image_abs(out_chi2, cexpect, DBL_EPSILON * 1e9);
        cpl_image_delete(cexpect);

        cexpect = cpl_image_new(10, 10, HDRL_TYPE_DATA);
        cpl_image_add_scalar(cexpect, 3);
        cpl_image_set(cexpect, 3, 4, 2);
        cpl_test_image_abs(out_dof, cexpect, 0);
        cpl_image_delete(cexpect);

        hdrl_imagelist_delete(out_coef);
        cpl_image_delete(out_chi2);
        cpl_image_delete(out_dof);


        /* smoke test less good pixels than fit degree */
        hdrl_image_reject(hdrl_imagelist_get(input, 0), 2, 2);
        hdrl_image_reject(hdrl_imagelist_get(input, 1), 2, 2);
        hdrl_image_reject(hdrl_imagelist_get(input, 2), 2, 2);
        hdrl_image_reject(hdrl_imagelist_get(input, 3), 2, 2);
        hdrl_fit_polynomial_imagelist2(input, samp, 3, &out_coef, &out_chi2, &out_dof);

        hdrl_imagelist_delete(input);
        cpl_imagelist_delete(samp);
        hdrl_imagelist_delete(out_coef);
        cpl_image_delete(out_chi2);
        cpl_image_delete(out_dof);
    }
}

void test_real_data(void)
{
    /* pixel from vcam ramp data with poisson model error (gain 2.4, ron 10) */
    double x[] = {  3.,   3.,   5.,   5.,   7.,   7.,  10.,  10.,  12.,  12.,
                    15., 15.,  20.,  20. };
    double y[] = { 3441,  3420,  5606,  5586,  7814,  7815, 11003, 10970,
                   13292, 13198, 16347, 16175, 21267, 21318 };
    double e[] = { 39.16312027,  39.05124664,  49.35416031,  49.26966476,
                   57.92955399,  57.93315125,  68.4440155 ,  68.34349823,
                   75.08883667, 74.82757568,  83.13392639,  82.7017746 ,
                   94.66387939,  94.77605438 };
    cpl_vector * vx = cpl_vector_wrap(ARRAY_LEN(x), x);
    cpl_vector * vy = cpl_vector_wrap(ARRAY_LEN(y), y);
    cpl_vector * ve = cpl_vector_wrap(ARRAY_LEN(e), e);

    double exp_c[] = {296.10245659,  1063.12005477};
    cpl_matrix * design = polyvander1d(vx, 1);
    hdrl_ls_fit_result * res = polyfit1d(vx, vy, ve, 1);
    matrix_eq(res->coef, exp_c, DBL_EPSILON * 2e10);

    cpl_vector_unwrap(vx);
    cpl_vector_unwrap(vy);
    cpl_vector_unwrap(ve);
    cpl_matrix_delete(design);
    hdrl_ls_fit_result_delete(res);
}

/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of hdrl_image
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    test_vander1d();
    test_fit();
    test_poisson();
    test_imglistfit();
    test_imglistfit2();
    test_real_data();

    return cpl_test_end(0);
}
