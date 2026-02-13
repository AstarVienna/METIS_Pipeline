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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                    Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_sigclip.h"
#include "hdrl_utils.h"
#include <cpl.h>

#include <math.h>
#include <stdlib.h>

#ifndef ARRAY_LEN
#define ARRAY_LEN(a) sizeof((a))/sizeof((a)[0])
#endif

#ifndef SQR
#define SQR(a) ((a) * (a))
#endif


/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_sigclip_test   Testing of the HDRL Sigma Clipping module
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_clip_kappa_sigma_test(void)
{
    double omean, omean_err, rej_low, rej_high;
    cpl_size naccepted;

    {
        double * dpixels = cpl_calloc(9, sizeof(double));

        cpl_vector * data = cpl_vector_wrap(9, dpixels);
        cpl_vector * errors = cpl_vector_new(9);
        cpl_vector_fill(errors, 1);

        /* null optional out params */
        hdrl_kappa_sigma_clip(data, errors, 3, 3, 3, CPL_FALSE,
                              &omean, NULL, NULL, NULL, NULL);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_eq(omean, 0.);

        /* all out params NULL makes no sense */
        hdrl_kappa_sigma_clip(data, errors, 3, 3, 3, CPL_FALSE,
                              NULL, NULL, NULL, NULL, NULL);
        cpl_test_error(CPL_ERROR_NULL_INPUT);

        /* NULL data */
        hdrl_kappa_sigma_clip(NULL, errors, 3, 3, 3, CPL_FALSE,
                              &omean, NULL, NULL, NULL, NULL);
        cpl_test_error(CPL_ERROR_NULL_INPUT);

        /* wrong iter */
        hdrl_kappa_sigma_clip(data, errors, 3, 3, 0, CPL_TRUE,
                             &omean, NULL, NULL, NULL, NULL);
        cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

        cpl_vector_delete(data);
        cpl_vector_delete(errors);
    }

    {
        double * dpixels = cpl_calloc(9, sizeof(double));

        cpl_vector * data = cpl_vector_wrap(9, dpixels);
        cpl_vector * errors = cpl_vector_new(9);
        cpl_vector_fill(errors, 1);

        hdrl_kappa_sigma_clip(data, errors, 3, 3, 3, CPL_FALSE,
                              &omean, &omean_err, &naccepted,
                              NULL, NULL);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_eq(omean, 0.);
        cpl_test_rel(omean_err, 1 / sqrt(9), 0.001);
        cpl_test_eq(naccepted, 9);

        cpl_vector_delete(data);
        cpl_vector_delete(errors);
    }

    {
        /* MAD sigma ~3 median 6, check that 1.5 and 10.5 which are closer to
         * 6 -+ 3 * 1.5 than 4 and 6 but beyond the k*sig limit are not included */
        double values[] = {1.5, 6., 4., 6., 4., 6., 4., 6., 10.5};
        const int n = ARRAY_LEN(values);

        cpl_vector * data = cpl_vector_wrap(n, values);
        cpl_vector * errors = cpl_vector_new(n);
        cpl_vector_fill(errors, 1);

        hdrl_kappa_sigma_clip(data, errors, 1.5, 1.5, 1, CPL_FALSE,
                              &omean, &omean_err, &naccepted,
                              &rej_low, &rej_high);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_rel(omean, (4. * 3 + 6 * 4.) / 7., 0.001);
        cpl_test_rel(omean_err, 1 / sqrt(n - 2), 0.001);
        cpl_test_rel(rej_low, 1.55, 0.02);
        cpl_test_rel(rej_high, 10.44, 0.02);
        cpl_test_eq(naccepted, n - 2);

        cpl_vector_unwrap(data);
        cpl_vector_delete(errors);
    }

    {
        /* special case of one remaining pixels */
        double values[] = {10.};
        const int n = ARRAY_LEN(values);

        cpl_vector * data = cpl_vector_wrap(n, values);
        cpl_vector * errors = cpl_vector_new(n);
        cpl_vector_fill(errors, 1);

        hdrl_kappa_sigma_clip(data, errors, 2., 2., 1, CPL_FALSE,
                              &omean, &omean_err, &naccepted,
                              &rej_low, &rej_high);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_rel(omean, values[0], 0.001);
        cpl_test_rel(omean_err, 1, 0.001);
        cpl_test_rel(rej_low, values[0], 0.02);
        cpl_test_rel(rej_high, values[0], 0.02);
        cpl_test_eq(naccepted, n);

        cpl_vector_unwrap(data);
        cpl_vector_delete(errors);
    }

    {
        /* gaus mean 100 sigma 3.5 */
        double values[] = {92, 93, 94, 94, 95, 95, 96, 96, 96, 97,
            97, 97, 97, 98, 98, 98, 98, 99, 99, 99,
            99, 100, 100, 100, 100, 100, 101, 101, 101, 101,
            102, 102, 102, 102, 103, 103, 103, 103, 104, 104,
            104, 105, 105, 106, 106, 107, 108 };
        const int n = ARRAY_LEN(values);

        cpl_vector * data = cpl_vector_wrap(n, values);
        cpl_vector * errors = cpl_vector_new(n);
        cpl_vector_fill(errors, 1);

        /* kappa 2 included by iqr 92 and 108 */
        hdrl_kappa_sigma_clip(data, errors, 2., 2., 3, CPL_FALSE,
                              &omean, &omean_err, &naccepted,
                              &rej_low, &rej_high);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_rel(omean, 100., 0.001);
        cpl_test_rel(omean_err, 1 / sqrt(n), 0.001);
        /* sigma overestimated by iqr */
        cpl_test_rel(rej_low, 91., 0.005);
        cpl_test_rel(rej_high, 109, 0.005);
        cpl_test_eq(naccepted, n);

        cpl_vector_unwrap(data);
        cpl_vector_delete(errors);
    }

    {
        /* gaus mean 100 sigma 3.5, 2 sigma range, 2 outliers */
        double values[] = {1, 150, 94, 94, 95, 95, 96, 96, 96, 97,
            97, 97, 97, 98, 98, 98, 98, 99, 99, 99,
            99, 100, 100, 100, 100, 100, 101, 101, 101, 101,
            102, 102, 102, 102, 103, 103, 103, 103, 104, 104,
            104, 105, 105, 106, 106, 107, 108 };
        const int n = ARRAY_LEN(values);

        cpl_vector * data = cpl_vector_wrap(n, values);
        cpl_vector * errors = cpl_vector_new(n);
        cpl_vector_fill(errors, 1);

        hdrl_kappa_sigma_clip(data, errors, 3, 3, 3, CPL_FALSE,
                              &omean, &omean_err, &naccepted,
                              NULL, NULL);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_rel(omean, 100., 0.005);
        cpl_test_rel(omean_err, 1 / sqrt(n - 2), 0.001);
        cpl_test_eq(naccepted, n - 2);

        cpl_vector_unwrap(data);
        cpl_vector_delete(errors);
    }

    /* test inputs are not modified */
    {
        double values[] = {54, 234. ,5,2, 343, 23 , 2, 0.21, 0.1232 , 1.2e3};
        const int n = ARRAY_LEN(values);

        cpl_vector * data = cpl_vector_wrap(n, values);
        cpl_vector * errors = cpl_vector_duplicate(data);
        cpl_vector * odata = cpl_vector_duplicate(data);
        cpl_vector * oerrors = cpl_vector_duplicate(errors);

        hdrl_kappa_sigma_clip(data, errors, 3, 3, 3, CPL_FALSE,
                              &omean, &omean_err, &naccepted,
                              NULL, NULL);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_vector_abs(data, odata, FLT_EPSILON);
        cpl_test_vector_abs(errors, oerrors, FLT_EPSILON);

        cpl_vector_unwrap(data);
        cpl_vector_delete(errors);
        cpl_vector_delete(odata);
        cpl_vector_delete(oerrors);
    }


    /* image test */
    {
        /* gaus mean 100 sigma 3.5, 2 sigma range, 2 outliers */
        double values[] = {1, 150, 94, 94, 95, 95, 96, 96, 96, 97,
            97, 97, 97, 98, 98, 98, 98, 99, 99, 99,
            99, 100, 100, 100, 100, 100, 101, 101, 101, 101,
            102, 102, 102, 102, 103, 103, 103, 103, 104, 104,
            104, 105, 105, 106, 106, 107, 108, 100, 100};
        const int n = ARRAY_LEN(values);

        cpl_image * data = cpl_image_wrap(sqrt(n), sqrt(n), CPL_TYPE_DOUBLE, 
                values);
        cpl_image * errors = cpl_image_new(sqrt(n), sqrt(n), CPL_TYPE_DOUBLE);
        cpl_image_add_scalar(errors, 1);

        hdrl_kappa_sigma_clip_image(data, errors, 3, 3, 3,
                                   &omean, &omean_err, &naccepted,
                                   NULL, NULL);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_rel(omean, 100., 0.005);
        cpl_test_rel(omean_err, 1 / sqrt(n - 2), 0.001);
        cpl_test_eq(naccepted, n - 2);

        cpl_image_unwrap(data);
        cpl_image_delete(errors);
    }

    /* image test with bad pixels */
    {
        /* gaus mean 100 sigma 3.5, 2 sigma range, 2 outliers */
        float values[] = {1, 150, 94, 94, 95, 95, 96, 96, 96, 97,
            97, 97, 97, 98, 98, 98, 98, 99, 99, 99,
            99, 100, 100, 100, 100, 100, 101, 101, 101, 101,
            102, 102, 102, 102, 103, 103, 103, 103, 104, 104,
            104, 105, 105, 106, 106, 107, 108, 100, 100};
        const int n = sqrt(ARRAY_LEN(values));

        cpl_image * data = cpl_image_wrap(n, n, CPL_TYPE_FLOAT, values);
        cpl_image * errors = cpl_image_new(n, n, CPL_TYPE_FLOAT);
        cpl_image_add_scalar(errors, 1);
        /* set two bad pixels with really high error */
        cpl_image_reject(data, n, n);
        cpl_image_reject(data, n, n - 1);
        cpl_image_set(errors, n, n, 2343.e30);
        cpl_image_set(errors, n, n - 1, 2343.e30);
        cpl_image_reject_from_mask(errors, cpl_image_get_bpm(data));

        hdrl_kappa_sigma_clip_image(data, errors, 3, 3, 3,
                                   &omean, &omean_err, &naccepted,
                                   NULL, NULL);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_rel(omean, 100., 0.005);
        cpl_test_rel(omean_err, 1 / sqrt(n * n - 4), 0.001);
        cpl_test_eq(naccepted, (n * n) - 4);

        cpl_image_unwrap(data);
        cpl_image_delete(errors);
    }

    /* test unequal bpms */
    {
        const int n = 5;
        cpl_image * data = cpl_image_new(n, n, CPL_TYPE_FLOAT);
        cpl_image * errors = cpl_image_new(n, n, CPL_TYPE_FLOAT);
        cpl_image_reject(data, n, n);
        cpl_image_reject(data, n, n - 1);

        hdrl_kappa_sigma_clip_image(data, errors, 3, 3, 3,
                                   &omean, &omean_err, &naccepted,
                                   NULL, NULL);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_image_delete(data);
        cpl_image_delete(errors);
    }

    return cpl_error_get_code();
}


/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_minmax_test   Testing of the HDRL MINMAX Clipping module
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_clip_minmax_test(void)
{
    double omean, omean_err, nlow, nhigh, rej_low, rej_high;

    cpl_size naccepted;

    {
        double * dpixels = cpl_calloc(9, sizeof(double));

        cpl_vector * data = cpl_vector_wrap(9, dpixels);
        cpl_vector * errors = cpl_vector_new(9);
        cpl_vector_fill(errors, 1);

        /* null optional out params */
        hdrl_minmax_clip(data, errors, 3, 3, CPL_FALSE,
                         &omean, NULL, NULL, NULL, NULL);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_eq(omean, 0.);

        /* all out params NULL makes no sense */
        hdrl_minmax_clip(data, errors, 3, 3, CPL_FALSE,
                         NULL, NULL, NULL, NULL, NULL);
        cpl_test_error(CPL_ERROR_NULL_INPUT);

        /* NULL data */
        hdrl_minmax_clip(NULL, errors, 3, 3, CPL_TRUE,
                         &omean, NULL, NULL, NULL, &rej_high);
        cpl_test_error(CPL_ERROR_NULL_INPUT);


        cpl_vector_delete(data);
        cpl_vector_delete(errors);
    }

    {
        double * dpixels = cpl_calloc(9, sizeof(double));
        cpl_vector * data = cpl_vector_wrap(9, dpixels);
        cpl_vector * errors = cpl_vector_new(9);
        cpl_vector_fill(errors, 1);
        nlow = 3;
        nhigh = 3;
        hdrl_minmax_clip(data, errors, nlow, nhigh, CPL_FALSE,
                         &omean, &omean_err, &naccepted,
                         &rej_low, &rej_high);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_eq(omean, 0.);
        cpl_test_rel(omean_err, 1 / sqrt(9 - (nlow + nhigh)), 0.001);
        cpl_test_eq(naccepted, 9 - (nlow + nhigh));
        cpl_test_eq(rej_low, 0.);
        cpl_test_eq(rej_high, 0.);

        cpl_vector_delete(data);
        cpl_vector_delete(errors);
    }


    {
        /* special case of one remaining pixels */
        double values[] = {10.};
        const int n = ARRAY_LEN(values);

        cpl_vector * data = cpl_vector_wrap(n, values);
        cpl_vector * errors = cpl_vector_new(n);
        cpl_vector_fill(errors, 1);
        nlow = 1;
        nhigh = 1;
        hdrl_minmax_clip(data, errors, nlow, nhigh, CPL_FALSE,
                         &omean, &omean_err, &naccepted,
                         &rej_low, NULL);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test(isnan(omean));
        cpl_test(isnan(omean_err));
        cpl_test_eq(naccepted, 0);
        cpl_test_eq(rej_low, 0); /* invalid */

        cpl_vector_unwrap(data);
        cpl_vector_delete(errors);
    }

    {
        /* gaus mean 100 sigma 3.5  - special case - don't reject anything */
        double values[] = {92, 93, 94, 94, 95, 95, 96, 96, 96, 97,
                        97, 97, 97, 98, 98, 98, 98, 99, 99, 99,
                        99, 100, 100, 100, 100, 100, 101, 101, 101, 101,
                        102, 102, 102, 102, 103, 103, 103, 103, 104, 104,
                        104, 105, 105, 106, 106, 107, 108 };
        const int n = ARRAY_LEN(values);

        cpl_vector * data = cpl_vector_wrap(n, values);
        cpl_vector * errors = cpl_vector_new(n);
        cpl_vector_fill(errors, 1);
        nlow = 0;
        nhigh = 0;

        hdrl_minmax_clip(data, errors, nlow, nhigh, CPL_FALSE,
                         &omean, &omean_err, &naccepted,
                         &rej_low, &rej_high);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_rel(omean, 100., n * HDRL_EPS_DATA);
        cpl_test_rel(omean_err, 1 / sqrt(n -(nlow + nhigh)), 0.001);
        cpl_test_eq(rej_low, 92.);
        cpl_test_eq(rej_high, 108);
        cpl_test_eq(naccepted, n - (nlow + nhigh));

        cpl_vector_unwrap(data);
        cpl_vector_delete(errors);
    }

    {
        /* gaus mean 100 sigma 3.5 */
        double values[] = {92, 93, 94, 94, 95, 95, 96, 96, 96, 97,
                        97, 97, 97, 98, 98, 98, 98, 99, 99, 99,
                        99, 100, 100, 100, 100, 100, 101, 101, 101, 101,
                        102, 102, 102, 102, 103, 103, 103, 103, 104, 104,
                        104, 105, 105, 106, 106, 107, 108 };
        const int n = ARRAY_LEN(values);

        cpl_vector * data = cpl_vector_wrap(n, values);
        cpl_vector * errors = cpl_vector_new(n);
        cpl_vector_fill(errors, 1);
        nlow = 2;
        nhigh = 2;

        hdrl_minmax_clip(data, errors, nlow, nhigh, CPL_FALSE,
                         &omean, &omean_err, &naccepted,
                         &rej_low, &rej_high);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_rel(omean, 100., 0.001);
        cpl_test_rel(omean_err, 1 / sqrt(n -(nlow + nhigh)), 0.001);
        /* sigma overestimated by iqr */
        cpl_test_eq(rej_low, 94.);
        cpl_test_eq(rej_high, 106);
        cpl_test_eq(naccepted, n - (nlow + nhigh));

        cpl_vector_unwrap(data);
        cpl_vector_delete(errors);
    }

    {
        /* gaus mean 100 sigma 3.5, 2 sigma range, 2 outliers */
        double values[] = {1, 150, 94, 94, 95, 95, 96, 96, 96, 97,
                        97, 97, 97, 98, 98, 98, 98, 99, 99, 99,
                        99, 100, 100, 100, 100, 100, 101, 101, 101, 101,
                        102, 102, 102, 102, 103, 103, 103, 103, 104, 104,
                        104, 105, 105, 106, 106, 107, 108 };
        const int n = ARRAY_LEN(values);

        cpl_vector * data = cpl_vector_wrap(n, values);
        cpl_vector * errors = cpl_vector_new(n);
        cpl_vector_fill(errors, 1);
        nlow = 3;
        nhigh = 3;
        hdrl_minmax_clip(data, errors, nlow, nhigh, CPL_FALSE,
                         &omean, &omean_err, &naccepted,
                         NULL, &rej_high);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_rel(omean, 100., 0.005);
        cpl_test_rel(omean_err, 1 / sqrt(n - (nlow + nhigh)), 0.001);
        cpl_test_eq(naccepted, n - (nlow + nhigh));

        cpl_vector_unwrap(data);
        cpl_vector_delete(errors);
    }

    /* test inputs are not modified */
    {
        double values[] = {54, 234. ,5,2, 343, 23 , 2, 0.21, 0.1232 , 1.2e3};
        const int n = ARRAY_LEN(values);

        cpl_vector * data = cpl_vector_wrap(n, values);
        cpl_vector * errors = cpl_vector_duplicate(data);
        cpl_vector * odata = cpl_vector_duplicate(data);
        cpl_vector * oerrors = cpl_vector_duplicate(errors);
        nlow = 3;
        nhigh = 3;

        hdrl_minmax_clip(data, errors, nlow, nhigh, CPL_FALSE,
                         &omean, &omean_err, &naccepted,
                         &rej_low, &rej_high);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_eq(rej_low, 2.);
        cpl_test_eq(rej_high, 54.);

        cpl_test_vector_abs(data, odata, 0.);
        cpl_test_vector_abs(errors, oerrors, 0.);

        cpl_vector_unwrap(data);
        cpl_vector_delete(errors);
        cpl_vector_delete(odata);
        cpl_vector_delete(oerrors);
    }


    /* image test */
    {
        /* gaus mean 100 sigma 3.5, 2 sigma range, 3 outliers */
        double values[] = {1, 150, 94, 94, 95, 95, 96, 96, 96, 97,
                        97, 97, 97, 98, 98, 98, 98, 99, 99, 99,
                        99, 100, 100, 100, 100, 100, 101, 101, 101, 101,
                        102, 102, 102, 102, 103, 103, 103, 103, 104, 104,
                        104, 105, 105, 106, 106, 107, 108, 100, -1000};
        const int n = ARRAY_LEN(values);

        cpl_image * data = cpl_image_wrap(sqrt(n), sqrt(n), CPL_TYPE_DOUBLE,
                        values);
        cpl_image * errors = cpl_image_new(sqrt(n), sqrt(n), CPL_TYPE_DOUBLE);
        cpl_image_add_scalar(errors, 1);
        nlow = 2;
        nhigh = 1;

        hdrl_minmax_clip_image(data, errors, nlow, nhigh,
                               &omean, &omean_err, &naccepted,
                               NULL, NULL);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_rel(omean, 100., 0.005);
        cpl_test_rel(omean_err, 1 / sqrt(n - (nlow + nhigh)), 0.001);
        cpl_test_eq(naccepted, n - (nlow + nhigh));

        cpl_image_unwrap(data);
        cpl_image_delete(errors);
    }

    /* image test with bad pixels */
    {
        /* gaus mean 100 sigma 3.5, 2 sigma range, 2 outliers */
        float values[] = {1, 150, 94, 94, 95, 95, 96, 96, 96, 97,
                        97, 97, 97, 98, 98, 98, 98, 99, 99, 99,
                        99, 100, 100, 100, 100, 100, 101, 101, 101, 101,
                        102, 102, 102, 102, 103, 103, 103, 103, 104, 104,
                        104, 105, 105, 106, 106, 107, 108, 100, 100};
        const int n = sqrt(ARRAY_LEN(values));

        cpl_image * data = cpl_image_wrap(n, n, CPL_TYPE_FLOAT, values);
        cpl_image * errors = cpl_image_new(n, n, CPL_TYPE_FLOAT);
        cpl_image_add_scalar(errors, 1);
        nlow = 1;
        nhigh = 1;

        /* set two bad pixels with really high error */
        cpl_image_reject(data, n, n);
        cpl_image_reject(data, n, n - 1);
        cpl_image_set(errors, n, n, 2343.e30);
        cpl_image_set(errors, n, n - 1, 2343.e30);
        cpl_image_reject_from_mask(errors, cpl_image_get_bpm(data));

        hdrl_minmax_clip_image(data, errors, nlow, nhigh,
                               &omean, &omean_err, &naccepted,
                               NULL, NULL);
        cpl_test_error(CPL_ERROR_NONE);


        cpl_test_rel(omean, 100., 0.005);
        cpl_test_rel(omean_err, 1 / sqrt(n * n - (nlow + nhigh + 2)), 0.001);
        cpl_test_eq(naccepted, (n * n) - (nlow + nhigh + 2));

        cpl_image_unwrap(data);
        cpl_image_delete(errors);
    }

    /* test unequal bpms */
    {
        const int n = 5;
        cpl_image * data = cpl_image_new(n, n, CPL_TYPE_FLOAT);
        cpl_image * errors = cpl_image_new(n, n, CPL_TYPE_FLOAT);
        cpl_image_reject(data, n, n);
        cpl_image_reject(data, n, n - 1);
        nlow = 3;
        nhigh = 3;

        hdrl_minmax_clip_image(data, errors, nlow, nhigh,
                               &omean, &omean_err, &naccepted,
                               NULL, NULL);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_image_delete(data);
        cpl_image_delete(errors);
    }
    /* image test equal range of rejected values */
    {
        float values[] = {-5., -5., -5., 1., 5., 1., 5., 1., 5., 5.};
        float errors[] = {50., 500., 100., 1., 5., 1., 500., 1., 200.};
        const int n = sqrt(ARRAY_LEN(values));

        cpl_image * data = cpl_image_wrap(n, n, CPL_TYPE_FLOAT, values);
        cpl_image * errs = cpl_image_wrap(n, n, CPL_TYPE_FLOAT, errors);
        cpl_image * odata = cpl_image_duplicate(data);
        cpl_image * oerrs = cpl_image_duplicate(errs);

        hdrl_minmax_clip_image(data, errs, 2, 2,
                               &omean, &omean_err, &naccepted,
                               NULL, NULL);
        cpl_test_error(CPL_ERROR_NONE);


        cpl_test_rel(omean, 0.6, 10 * HDRL_EPS_DATA);
        cpl_test_rel(omean_err, sqrt((SQR(50.) + 1. * 3. + SQR(5.)) /
                                     SQR(n * n - 4)),
                     20 * HDRL_EPS_ERROR);
        cpl_test_eq(naccepted, (n * n) - (4));

        /* select multiple smallest errors, low*/
        hdrl_minmax_clip_image(data, errs, 1, 2,
                               &omean, &omean_err, &naccepted,
                               NULL, NULL);
        cpl_test_error(CPL_ERROR_NONE);

        /* select multiple smallest errors, high */
        hdrl_minmax_clip_image(data, errs, 2, 1,
                               &omean, &omean_err, &naccepted,
                               NULL, NULL);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_rel(omean, 8. / 6., 10 * HDRL_EPS_DATA);
        cpl_test_rel(omean_err,
                     sqrt((SQR(50.) + SQR(200.) + 1. * 3. + SQR(5.)) /
                           SQR(n * n - (3))),
                     20 * HDRL_EPS_ERROR);
        cpl_test_eq(naccepted, (n * n) - (3));

        /* select multiple smallest errors, low + high */
        hdrl_minmax_clip_image(data, errs, 1, 1,
                               &omean, &omean_err, &naccepted,
                               NULL, NULL);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_rel(omean, 3. / 7., 10 * HDRL_EPS_DATA);
        cpl_test_rel(omean_err,
                     sqrt((SQR(50.) + SQR(100.) + SQR(200.) + 3. + SQR(5.)) /
                          SQR(n * n - 2)),
                     20 * HDRL_EPS_ERROR);
        cpl_test_eq(naccepted, (n * n) - (2));

        /* 50 error not in equal range so we should get the 100. error */
        values[0] = -5.0001;
        cpl_image_set(odata, 1, 1, values[0]);
        hdrl_minmax_clip_image(data, errs, 2, 2,
                               &omean, &omean_err, &naccepted,
                               NULL, NULL);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_rel(omean, 0.6, 10 * HDRL_EPS_DATA);
        cpl_test_rel(omean_err, sqrt((SQR(100.) + 1. * 3. + SQR(5.)) /
                                    SQR(n * n - 4)),
                     20 * HDRL_EPS_ERROR);
        cpl_test_eq(naccepted, (n * n) - (4));

        /* check original have not been overwritten */
        cpl_test_image_abs(data, odata, 0);
        cpl_test_image_abs(errs, oerrs, 0);

        cpl_image_delete(odata);
        cpl_image_delete(oerrs);
        cpl_image_unwrap(data);
        cpl_image_unwrap(errs);
    }

    return cpl_error_get_code();
}





/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of clipping
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    hdrl_clip_kappa_sigma_test();
    hdrl_clip_minmax_test();

    return cpl_test_end(0);
}
