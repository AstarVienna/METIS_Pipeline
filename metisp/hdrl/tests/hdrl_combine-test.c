/* $Id: hdrl_combine-test.c,v 1.7 2013-09-24 14:58:54 jtaylor Exp $
 *
 * This file is part of the HDRL
 * Copyright (C) 2013 European Southern Observatory
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
 * $Author: jtaylor $
 * $Date: 2013-09-24 14:58:54 $
 * $Revision: 1.7 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                    Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_combine.h"
#include <cpl.h>

#include <math.h>
#include <stdlib.h>

#ifndef ARRAY_LEN
#define ARRAY_LEN(a) sizeof((a))/sizeof((a)[0])
#endif


/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_combine_test   Testing of the HDRL combine module
 */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of combine module
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

#define TST_FREE \
        cpl_image_delete(outimg); \
        cpl_image_delete(outerr); \
        cpl_image_delete(contrib); \
        hdrl_collapse_imagelist_to_image_delete(method);

    /* create data, value 5., error +-1 */
    cpl_imagelist * data = cpl_imagelist_new();
    cpl_imagelist * errs = cpl_imagelist_new();
    cpl_image * img = cpl_image_new(40, 40, CPL_TYPE_DOUBLE);
    cpl_image * err = cpl_image_new(40, 40, CPL_TYPE_DOUBLE);
    cpl_image_add_scalar(img, 5.);
    cpl_image_add_scalar(err, 2.);
    size_t nz = 5;

    /* create expected results (err / sqrt(nz) for mean) */
    cpl_image * expect_err = cpl_image_duplicate(err);
    cpl_image_divide_scalar(expect_err, sqrt(nz));
    cpl_image * expect_contrib = cpl_image_new(40, 40, CPL_TYPE_INT);
    cpl_image_add_scalar(expect_contrib, 5);
    for (size_t i = 0; i < nz; i++) {
        cpl_imagelist_set(data, cpl_image_duplicate(img), cpl_imagelist_get_size(data));
        cpl_imagelist_set(errs, cpl_image_duplicate(err), cpl_imagelist_get_size(errs));
    }
    cpl_image * outimg, * outerr, * contrib;

    /* test inputs */
    {
        hdrl_collapse_imagelist_to_image_t * method =
            hdrl_collapse_imagelist_to_image_mean();
        hdrl_imagelist_combine(data, errs, method, &outimg, &outerr, NULL);
        cpl_test_error(CPL_ERROR_NULL_INPUT);
        hdrl_imagelist_combine(data, errs, method, &outimg, NULL, &contrib);
        cpl_test_error(CPL_ERROR_NULL_INPUT);
        hdrl_imagelist_combine(data, errs, method, NULL, &outerr, &contrib);
        cpl_test_error(CPL_ERROR_NULL_INPUT);
        hdrl_imagelist_combine(data, errs, NULL, &outimg, &outerr, &contrib);
        cpl_test_error(CPL_ERROR_NULL_INPUT);
        hdrl_imagelist_combine(data, NULL, method, &outimg, &outerr, &contrib);
        cpl_test_error(CPL_ERROR_NULL_INPUT);
        hdrl_imagelist_combine(NULL, errs, method, &outimg, &outerr, &contrib);
        cpl_test_error(CPL_ERROR_NULL_INPUT);
        hdrl_collapse_imagelist_to_image_delete(method);
    }
    /* wrong imagelist sizes */
    {
        hdrl_collapse_imagelist_to_image_t * method =
            hdrl_collapse_imagelist_to_image_mean();
        cpl_imagelist * data2 = cpl_imagelist_duplicate(data);
        cpl_imagelist * errs2;
        cpl_image_delete(cpl_imagelist_unset(data2, 0));
        hdrl_imagelist_combine(data2, errs, method, &outimg, &outerr, &contrib);
        cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
        cpl_imagelist_delete(data2);

        data2 = cpl_imagelist_new();
        errs2 = cpl_imagelist_new();
        hdrl_imagelist_combine(data2, errs2, method, &outimg, &outerr, &contrib);
        cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
        cpl_imagelist_delete(data2);
        cpl_imagelist_delete(errs2);
        hdrl_collapse_imagelist_to_image_delete(method);
    }
    /* test reductions on uniform error cases */
    {
        /* mean */
        hdrl_collapse_imagelist_to_image_t * method =
            hdrl_collapse_imagelist_to_image_mean();
        hdrl_imagelist_combine(data, errs, method, &outimg, &outerr, &contrib);

        cpl_test_image_abs(outimg, img, HDRL_EPS_DATA);
        cpl_test_image_abs(outerr, expect_err, HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);
        TST_FREE;

        /* sigclip */
        method = hdrl_collapse_imagelist_to_image_sigclip(3., 3., 3);
        hdrl_imagelist_combine(data, errs, method, &outimg, &outerr, &contrib);

        cpl_test_image_abs(outimg, img, HDRL_EPS_DATA);
        cpl_test_image_abs(outerr, expect_err, HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);
        TST_FREE;

        /* minmax */
        method = hdrl_collapse_imagelist_to_image_minmax(0, 0);
        hdrl_imagelist_combine(data, errs, method, &outimg, &outerr, &contrib);

        cpl_test_image_abs(outimg, img, HDRL_EPS_DATA);
        cpl_test_image_abs(outerr, expect_err, HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);
        TST_FREE;

        /* weighted mean */
        method = hdrl_collapse_imagelist_to_image_weighted_mean();
        hdrl_imagelist_combine(data, errs, method, &outimg, &outerr, &contrib);

        cpl_test_image_abs(outimg, img, HDRL_EPS_DATA);
        cpl_test_image_abs(outerr, expect_err, HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);
        TST_FREE;

        /* median */
        cpl_image_multiply_scalar(expect_err, sqrt(CPL_MATH_PI_2));
        method = hdrl_collapse_imagelist_to_image_median();
        hdrl_imagelist_combine(data, errs, method, &outimg, &outerr, &contrib);

        cpl_test_image_abs(outimg, img, HDRL_EPS_DATA);
        cpl_test_image_abs(outerr, expect_err, HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);
        TST_FREE;
    }
    /* test non uniform error cases */
    {
        double v[] = {1, 2, 1, 3, 2};
        double e[] = {0.5, 0.7, 0.1, 1.0, 0.01};
        int d;
        for (size_t i = 0; i < nz; i++) {
            cpl_image * tmp = cpl_imagelist_get(data, i);
            cpl_image_set(tmp, 1, 1, v[i]);
            tmp = cpl_imagelist_get(errs, i);
            cpl_image_set(tmp, 1, 1, e[i]);
        }
        hdrl_collapse_imagelist_to_image_t * method =
            hdrl_collapse_imagelist_to_image_mean();
        hdrl_imagelist_combine(data, errs, method, &outimg, &outerr, &contrib);
        cpl_test_abs(cpl_image_get(outimg, 1, 1, &d), 1.8, HDRL_EPS_DATA);
        cpl_test_abs(cpl_image_get(outerr, 1, 1, &d), 0.26458269028793246, HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);
        TST_FREE;

        method = hdrl_collapse_imagelist_to_image_sigclip(3., 3., 3);
        hdrl_imagelist_combine(data, errs, method, &outimg, &outerr, &contrib);

        cpl_test_abs(cpl_image_get(outimg, 1, 1, &d), 1.8, HDRL_EPS_DATA);
        cpl_test_abs(cpl_image_get(outerr, 1, 1, &d), 0.26458269028793246, HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);
        TST_FREE;

        method = hdrl_collapse_imagelist_to_image_minmax(1, 1);
        hdrl_imagelist_combine(data, errs, method, &outimg, &outerr, &contrib);

        cpl_test_abs(cpl_image_get(outimg, 1, 1, &d), 5./3., HDRL_EPS_DATA);

        /* The minmax rejection algorithm has a
         * intrinsic problem with same values but different errors. If for
         * example the two lowest values are 1+-0.5 and 1+-500 what "1" should
         * be kept? It doesn't matter for the mean, but for the propagated
         * error  - the implemented algorithm propagates the smaller error */

        cpl_test_abs(cpl_image_get(outerr, 1, 1, &d),
                     sqrt(0.1 * 0.1 + 0.7 * 0.7 + 0.01 * 0.01) / 3.,
                     HDRL_EPS_ERROR);
        cpl_image * expect_contrib_minmax =
                        cpl_image_subtract_scalar_create(expect_contrib, 2);
        cpl_test_image_abs(contrib, expect_contrib_minmax, 0);
        cpl_image_delete(expect_contrib_minmax);
        TST_FREE;

        method = hdrl_collapse_imagelist_to_image_weighted_mean();
        hdrl_imagelist_combine(data, errs, method, &outimg, &outerr, &contrib);
        cpl_test_abs(cpl_image_get(outimg, 1, 1, &d), 1.9898090843925733, HDRL_EPS_DATA);
        cpl_test_abs(cpl_image_get(outerr, 1, 1, &d), 0.0099469054598625289, HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);
        TST_FREE;
    }
    /* test non uniform error cases with rejected values */
    {
        double v[] = {1, 2, 1, 3, 2};
        double e[] = {0.5, 0.7, 0.1, 1.0, 0.01};
        int d;
        for (size_t i = 0; i < nz; i++) {
            cpl_image * tmp = cpl_imagelist_get(data, i);
            cpl_image_set(tmp, 1, 1, v[i]);
            if (i == 3) {
                cpl_image_reject(tmp, 1, 1);
            }
            tmp = cpl_imagelist_get(errs, i);
            cpl_image_set(tmp, 1, 1, e[i]);
            if (i == 3) {
                cpl_image_reject(tmp, 1, 1);
            }
        }
        cpl_image_delete(expect_contrib);
        expect_contrib = cpl_image_new_from_accepted(data);

        hdrl_collapse_imagelist_to_image_t * method =
            hdrl_collapse_imagelist_to_image_mean();
        hdrl_imagelist_combine(data, errs, method, &outimg, &outerr, &contrib);

        cpl_test_abs(cpl_image_get(outimg, 1, 1, &d), 1.5, HDRL_EPS_DATA);
        cpl_test_abs(cpl_image_get(outerr, 1, 1, &d), 0.21652078422174625, HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);
        TST_FREE;

        method = hdrl_collapse_imagelist_to_image_sigclip(3., 3., 3);
        hdrl_imagelist_combine(data, errs, method, &outimg, &outerr, &contrib);

        cpl_test_abs(cpl_image_get(outimg, 1, 1, &d), 1.5, HDRL_EPS_DATA);
        cpl_test_abs(cpl_image_get(outerr, 1, 1, &d), 0.21652078422174625, HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);
        TST_FREE;

        method = hdrl_collapse_imagelist_to_image_minmax(1, 1);
        hdrl_imagelist_combine(data, errs, method, &outimg, &outerr, &contrib);

        cpl_test_abs(cpl_image_get(outimg, 1, 1, &d), 3./2., HDRL_EPS_DATA);
        cpl_test_abs(cpl_image_get(outerr, 1, 1, &d),
                      sqrt(0.01 * 0.01 + 0.1 * 0.1) / 2., HDRL_EPS_ERROR);
        cpl_image * expect_contrib_minmax =
                         cpl_image_subtract_scalar_create(expect_contrib, 2);
        cpl_test_image_abs(contrib, expect_contrib_minmax, 0);
        cpl_image_delete(expect_contrib_minmax);
        TST_FREE;

        method = hdrl_collapse_imagelist_to_image_weighted_mean();
        hdrl_imagelist_combine(data, errs, method, &outimg, &outerr, &contrib);

        cpl_test_abs(cpl_image_get(outimg, 1, 1, &d), 1.9897091252756485, HDRL_EPS_DATA);
        cpl_test_abs(cpl_image_get(outerr, 1, 1, &d), 0.0099473975744101273, HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);
        TST_FREE;
    }
    /* test median error propagation with rejects, only makes sense on uniform
     * errors as the scaling relies on gaussian errors */
    {
        double v[] = {1, 2, 1, 3, 2};
        double e[] = {1., 1., 1., 1., 1.};
        int d;
        for (size_t i = 0; i < nz; i++) {
            cpl_image * tmp = cpl_imagelist_get(data, i);
            cpl_image_set(tmp, 1, 1, v[i]);
            cpl_image_set(tmp, 2, 2, v[i]);
            if (i > 1) {
                cpl_image_reject(tmp, 1, 1);
            }
            tmp = cpl_imagelist_get(errs, i);
            cpl_image_set(tmp, 1, 1, e[i]);
            cpl_image_set(tmp, 2, 2, e[i]);
            if (i > 1) {
                cpl_image_reject(tmp, 1, 1);
            }
        }
        cpl_image_delete(expect_contrib);
        expect_contrib = cpl_image_new_from_accepted(data);

        hdrl_collapse_imagelist_to_image_t * method =
            hdrl_collapse_imagelist_to_image_median();
        hdrl_imagelist_combine(data, errs, method, &outimg, &outerr, &contrib);

        /* contrib > 2 -> sqrt(nz * pi / 2) error scaling */
        cpl_test_abs(cpl_image_get(outimg, 2, 2, &d), 2., HDRL_EPS_DATA);
        cpl_test_abs(cpl_image_get(outerr, 2, 2, &d),
                     1. / sqrt(nz) * sqrt(CPL_MATH_PI_2), HDRL_EPS_ERROR);
        /* contrib <= 2 -> median is a mean, no scaling */
        cpl_test_abs(cpl_image_get(outimg, 1, 1, &d), 1.5, HDRL_EPS_DATA);
        cpl_test_abs(cpl_image_get(outerr, 1, 1, &d), 1. / sqrt(2.), HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);
        TST_FREE;
    }

    cpl_imagelist_delete(data);
    cpl_imagelist_delete(errs);
    cpl_image_delete(expect_err);
    cpl_image_delete(expect_contrib);
    cpl_image_delete(img);
    cpl_image_delete(err);

    return cpl_test_end(0);
#undef TST_FREE
}
