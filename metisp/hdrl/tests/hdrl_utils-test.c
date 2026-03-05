/* $Id: hdrl_utils-test.c,v 1.11 2013-10-23 10:49:28 jtaylor Exp $
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
 * $Date: 2013-10-23 10:49:28 $
 * $Revision: 1.11 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                    Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_utils.h"
#include "hdrl_collapse.h"
#include "hdrl_bpm_utils.h"

#include <cpl.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef ARRAY_LEN
#define ARRAY_LEN(a) sizeof((a))/sizeof((a)[0])
#endif


/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_utils_test   Testing of the HDRL utility module
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_image_to_vector_test(void)
{

    {
        cpl_vector * v = hdrl_image_to_vector(NULL, NULL);
        cpl_test_error(CPL_ERROR_NULL_INPUT);
        cpl_test_null(v);
        v = hdrl_image_to_vector(NULL, (void*)1);
        cpl_test_error(CPL_ERROR_NULL_INPUT);
        cpl_test_null(v);
    }
    {
        cpl_image * img = cpl_image_new(5, 6, CPL_TYPE_INT);
        cpl_vector * v = hdrl_image_to_vector(img, NULL);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_eq(cpl_vector_get_size(v), 5 * 6);
        cpl_image_delete(img);
        cpl_vector_delete(v);
    }
    /* empty result */
    {
        cpl_image * img = cpl_image_new(1, 1, CPL_TYPE_INT);
        cpl_image_reject(img, 1, 1);
        cpl_vector * v = hdrl_image_to_vector(img, NULL);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_null(v);
        cpl_image_delete(img);
    }
    /* bad pixel in image bpm */
    {
        cpl_image * img = cpl_image_new(5, 6, CPL_TYPE_INT);
        cpl_image_set(img, 3, 5, 100);
        cpl_image_reject(img, 3, 5);
        cpl_vector * v = hdrl_image_to_vector(img, NULL);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_eq(cpl_vector_get_size(v), 5 * 6 - 1);
        cpl_test_eq(cpl_vector_get_sum(v), 0);
        cpl_image_delete(img);
        cpl_vector_delete(v);
    }
    /* bad pixel in external bpm */
    {
        cpl_image * img = cpl_image_new(5, 6, CPL_TYPE_INT);
        cpl_mask * m = cpl_mask_new(5, 6);
        cpl_image_set(img, 2, 5, 100);
        cpl_image_set(img, 3, 5, 100);
        cpl_mask_set(m, 3, 5, CPL_BINARY_1);
        cpl_vector * v = hdrl_image_to_vector(img, m);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_eq(cpl_vector_get_size(v), 5 * 6 - 1);
        cpl_test_eq(cpl_vector_get_sum(v), 100);
        cpl_image_delete(img);
        cpl_mask_delete(m);
        cpl_vector_delete(v);
    }
    /* no cast bad pixel in external bpm */
    {
        cpl_image * img = cpl_image_new(5, 6, CPL_TYPE_DOUBLE);
        cpl_mask * m = cpl_mask_new(5, 6);
        cpl_image_set(img, 2, 5, 100.);
        cpl_image_set(img, 3, 5, 100.);
        cpl_mask_set(m, 3, 5, CPL_BINARY_1);
        cpl_vector * v = hdrl_image_to_vector(img, m);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_eq(cpl_vector_get_size(v), 5 * 6 - 1);
        cpl_test_eq(cpl_vector_get_sum(v), 100);
        cpl_image_delete(img);
        cpl_mask_delete(m);
        cpl_vector_delete(v);
    }
    return cpl_error_get_code();
}


static cpl_error_code hdrl_imagelist_to_vector_test(void)
{
    {
        cpl_imagelist * list = cpl_imagelist_new();
        cpl_vector * v = hdrl_imagelist_to_vector(NULL, 1, 1);
        cpl_test_error(CPL_ERROR_NULL_INPUT);
        cpl_test_null(v);
        v = hdrl_imagelist_to_vector(list, 1, 1);
        cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
        cpl_test_null(v);
        cpl_imagelist_set(list, cpl_image_new(5, 5, CPL_TYPE_INT), 0);
        v = hdrl_imagelist_to_vector(list, 0, 0);
        cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);
        cpl_test_null(v);
        v = hdrl_imagelist_to_vector(list, 0, 1);
        cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);
        cpl_test_null(v);
        v = hdrl_imagelist_to_vector(list, 1, 0);
        cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);
        cpl_test_null(v);
        v = hdrl_imagelist_to_vector(list, 10, 3);
        cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);
        cpl_test_null(v);
        v = hdrl_imagelist_to_vector(list, 3, 10);
        cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);
        cpl_test_null(v);
        v = hdrl_imagelist_to_vector(list, 10, 10);
        cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);
        cpl_test_null(v);
        cpl_imagelist_delete(list);
    }
    {
        cpl_imagelist * list = cpl_imagelist_new();
        cpl_imagelist_set(list, cpl_image_new(5, 6, CPL_TYPE_INT), 0);
        cpl_vector * v = hdrl_imagelist_to_vector(list, 1, 1);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_eq(cpl_vector_get_size(v), 1);
        cpl_vector_delete(v);
        cpl_imagelist_set(list, cpl_image_new(5, 6, CPL_TYPE_INT), 1);
        v = hdrl_imagelist_to_vector(list, 1, 1);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_eq(cpl_vector_get_size(v), 2);
        cpl_imagelist_delete(list);
        cpl_vector_delete(v);
    }
    /* empty result */
    {
        cpl_imagelist * list = cpl_imagelist_new();
        cpl_image * img = cpl_image_new(5, 6, CPL_TYPE_INT);
        cpl_image_reject(img, 2, 1);
        cpl_imagelist_set(list, img, 0);
        cpl_imagelist_set(list, cpl_image_duplicate(img), 1);
        cpl_vector * v = hdrl_imagelist_to_vector(list, 2, 1);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_null(v);
        cpl_imagelist_delete(list);
        cpl_vector_delete(v);
    }
    /* bad pixel in image bpm */
    {
        cpl_imagelist * list = cpl_imagelist_new();
        cpl_image * img = cpl_image_new(5, 6, CPL_TYPE_INT);
        cpl_image_set(img, 2, 1, 100);
        cpl_imagelist_set(list, cpl_image_duplicate(img), 0);
        cpl_imagelist_set(list, cpl_image_duplicate(img), 1);
        cpl_image_reject(img, 2, 1);
        cpl_imagelist_set(list, img, 2);
        /* add a good image so potential uninitialized memory is included */
        cpl_imagelist_set(list, cpl_image_new(5, 6, CPL_TYPE_INT), 3);
        cpl_vector * v = hdrl_imagelist_to_vector(list, 2, 1);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_eq(cpl_vector_get_size(v), 3);
        cpl_test_eq(cpl_vector_get_sum(v), 200);
        cpl_imagelist_delete(list);
        cpl_vector_delete(v);
    }
    /* no cast bad pixel in image bpm */
    {
        cpl_imagelist * list = cpl_imagelist_new();
        cpl_image * img = cpl_image_new(5, 6, CPL_TYPE_DOUBLE);
        cpl_image_add_scalar(img, 37);
        cpl_image_set(img, 5, 6, 100);
        cpl_imagelist_set(list, cpl_image_duplicate(img), 0);
        cpl_imagelist_set(list, cpl_image_duplicate(img), 1);
        cpl_image_reject(img, 5, 6);
        cpl_imagelist_set(list, img, 2);
        /* add a good image so potential uninitialized memory is included */
        cpl_imagelist_set(list, cpl_image_new(5, 6, CPL_TYPE_DOUBLE), 3);
        cpl_vector * v = hdrl_imagelist_to_vector(list, 5, 6);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_eq(cpl_vector_get_size(v), 3);
        cpl_test_eq(cpl_vector_get_sum(v), 200);
        cpl_imagelist_delete(list);
        cpl_vector_delete(v);
    }
    /* no cast bad pixel in image bpm, full row test */
    {
        cpl_imagelist * list = cpl_imagelist_new();
        cpl_image * img = cpl_image_new(5, 6, CPL_TYPE_DOUBLE);
        cpl_image_add_scalar(img, 37);
        cpl_image_set(img, 5, 6, 100);
        cpl_imagelist_set(list, cpl_image_duplicate(img), 0);
        cpl_imagelist_set(list, cpl_image_duplicate(img), 1);
        cpl_image_reject(img, 5, 6);
        cpl_imagelist_set(list, img, 2);
        /* add a good image so potential uninitialized memory is included */
        cpl_imagelist_set(list, cpl_image_new(5, 6, CPL_TYPE_DOUBLE), 3);
        cpl_vector * o[5];
        hdrl_imagelist_to_vector_row(list, 6, o, NULL);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_eq(cpl_vector_get_size(o[4]), 3);
        cpl_test_eq(cpl_vector_get_sum(o[4]), 200);
        cpl_imagelist_delete(list);
        for (size_t i = 0; i < 5; i++) {
            cpl_vector_delete(o[i]);
        }
    }
    /* no cast bad pixel in image bpm, full row test non-double */
    {
        cpl_imagelist * list = cpl_imagelist_new();
        cpl_image * img = cpl_image_new(5, 6, CPL_TYPE_INT);
        cpl_image_add_scalar(img, 37);
        cpl_image_set(img, 5, 6, 100);
        cpl_imagelist_set(list, cpl_image_duplicate(img), 0);
        cpl_imagelist_set(list, cpl_image_duplicate(img), 1);
        cpl_image_reject(img, 5, 6);
        cpl_imagelist_set(list, img, 2);
        /* add a good image so potential uninitialized memory is included */
        cpl_imagelist_set(list, cpl_image_new(5, 6, CPL_TYPE_INT), 3);
        cpl_vector * o[5];
        hdrl_imagelist_to_vector_row(list, 6, o, NULL);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_eq(cpl_vector_get_size(o[4]), 3);
        cpl_test_eq(cpl_vector_get_sum(o[4]), 200);
        cpl_imagelist_delete(list);
        for (size_t i = 0; i < 5; i++) {
            cpl_vector_delete(o[i]);
        }
    }
    return cpl_error_get_code();
}

static cpl_error_code hdrl_imagelist_cplwrap(void)
{
	hdrl_imagelist *list = NULL;
	cpl_imagelist  *data = NULL;
	cpl_imagelist  *errs = NULL;

	/* Test list null */
	hdrl_imagelist_to_cplwrap(list, &data, &errs);
	cpl_test_error(CPL_ERROR_NULL_INPUT);

	/* new list and new data and errs */
	list = hdrl_imagelist_new();
	hdrl_imagelist_to_cplwrap(list, &data, &errs);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_imagelist_unwrap(data);
    cpl_imagelist_unwrap(errs);

    /* Not create new data and errs */
    hdrl_imagelist_to_cplwrap(list, &data, &errs);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_imagelist_unwrap(data);
    cpl_imagelist_unwrap(errs);

    hdrl_imagelist_delete(list);


    return cpl_error_get_code();
}

static cpl_error_code hdrl_normalize_test(void)
{
    cpl_imagelist * data = cpl_imagelist_new();
    cpl_imagelist * errs = cpl_imagelist_new();
    cpl_vector * vnorm_d, * vnorm_e;
    cpl_array * acontrib;

    const size_t nz = 23;
    const size_t nx = 41;
    const size_t ny = 17;
    for (size_t i = 0; i < nz; i++) {
        cpl_image * img = cpl_image_new(nx, ny, HDRL_TYPE_DATA);
        cpl_image_add_scalar(img, i + 1);
        cpl_image * err = cpl_image_new(nx, ny, HDRL_TYPE_ERROR);
        cpl_image_add_scalar(err, i + 1);
        cpl_image_divide_scalar(err, 3.);
        cpl_imagelist_set(data, img, i);
        cpl_imagelist_set(errs, err, i);
    }


    hdrl_collapse_imagelist_to_vector_t * vmethod =
        hdrl_collapse_imagelist_to_vector_mean();
    hdrl_collapse_imagelist_to_vector_call(vmethod, data, errs,
                                           &vnorm_d, &vnorm_e, &acontrib,
                                           NULL);
    cpl_array_delete(acontrib);

    /* Unknown mode */
    {
    	cpl_imagelist * ndata = cpl_imagelist_duplicate(data);
        cpl_imagelist * nerrs = cpl_imagelist_duplicate(errs);
        hdrl_normalize_imagelist_by_vector(vnorm_d, vnorm_e,
                                           -1, ndata, nerrs);
        cpl_test_error(CPL_ERROR_UNSUPPORTED_MODE);
        cpl_imagelist_delete(ndata);
        cpl_imagelist_delete(nerrs);
    }

    /* additive */
    {
        cpl_imagelist * ndata = cpl_imagelist_duplicate(data);
        cpl_imagelist * nerrs = cpl_imagelist_duplicate(errs);
        hdrl_normalize_imagelist_by_vector(vnorm_d, vnorm_e,
                                           HDRL_SCALE_ADDITIVE,
                                           ndata, nerrs);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_image_abs(cpl_imagelist_get_const(data, 0),
                           cpl_imagelist_get_const(ndata, 0), HDRL_EPS_DATA);
        cpl_test_image_abs(cpl_imagelist_get_const(errs, 0),
                           cpl_imagelist_get_const(nerrs, 0), HDRL_EPS_ERROR);
        for (size_t i = 1; i < nz; i++) {
            cpl_image * expect_d = cpl_image_new(nx, ny, HDRL_TYPE_DATA);
            cpl_image * expect_e = cpl_image_new(nx, ny, HDRL_TYPE_ERROR);
            double escale = hypot(((i + 1) / 3.) / sqrt(nx * ny),
                                  (1. / 3.) / sqrt(nx * ny));
            cpl_image_add_scalar(expect_e, hypot(escale, (i + 1) / 3.));
            cpl_image_add_scalar(expect_d, 1);
            cpl_test_image_abs(expect_d, cpl_imagelist_get_const(ndata, i),
                               HDRL_EPS_DATA);
            cpl_test_image_abs(expect_e, cpl_imagelist_get_const(nerrs, i),
                               HDRL_EPS_ERROR * 4);
            cpl_image_delete(expect_d);
            cpl_image_delete(expect_e);
        }
        cpl_imagelist_delete(ndata);
        cpl_imagelist_delete(nerrs);
    }

    /* multiplicative */
    {
        cpl_imagelist *ndata = cpl_imagelist_duplicate(data);
        cpl_imagelist *nerrs = cpl_imagelist_duplicate(errs);


        /* Simulate scale error (emit warnings) */
        cpl_vector    *fake1 = cpl_vector_duplicate(vnorm_d);
        cpl_vector    *fake2 = cpl_vector_duplicate(vnorm_e);
        cpl_imagelist *fake3 = cpl_imagelist_duplicate(ndata);
        cpl_imagelist *fake4 = cpl_imagelist_duplicate(nerrs);
       	cpl_vector_set(fake1, 1, 0.);
        cpl_vector_set(fake2, 1, 0.);
        hdrl_normalize_imagelist_by_vector(fake1, fake2,
        		HDRL_SCALE_MULTIPLICATIVE, fake3, fake4);
        cpl_vector_delete(fake1);
        cpl_vector_delete(fake2);
        cpl_imagelist_delete(fake3);
        cpl_imagelist_delete(fake4);


        /* Normal work */
        hdrl_normalize_imagelist_by_vector(vnorm_d, vnorm_e,
                                           HDRL_SCALE_MULTIPLICATIVE,
                                           ndata, nerrs);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_image_abs(cpl_imagelist_get_const(data, 0),
                           cpl_imagelist_get_const(ndata, 0), HDRL_EPS_DATA);
        cpl_test_image_abs(cpl_imagelist_get_const(errs, 0),
                           cpl_imagelist_get_const(nerrs, 0), HDRL_EPS_ERROR);
        for (size_t i = 1; i < nz; i++) {
            cpl_image * expect_d = cpl_image_new(nx, ny, HDRL_TYPE_DATA);
            cpl_image * expect_e = cpl_image_new(nx, ny, HDRL_TYPE_ERROR);
            /* off by about 10e-4 due to missing correlation */
            cpl_image_add_scalar(expect_e, 0.3338112308308246);
            cpl_image_add_scalar(expect_d, 1.);
            cpl_test_image_abs(expect_d, cpl_imagelist_get_const(ndata, i),
                               HDRL_EPS_DATA);
            cpl_test_image_abs(expect_e, cpl_imagelist_get_const(nerrs, i),
                               HDRL_EPS_ERROR * 4);
            cpl_image_delete(expect_d);
            cpl_image_delete(expect_e);
        }
        cpl_imagelist_delete(ndata);
        cpl_imagelist_delete(nerrs);
    }

    cpl_imagelist_delete(data);
    cpl_imagelist_delete(errs);
    cpl_vector_delete(vnorm_d);
    cpl_vector_delete(vnorm_e);
    hdrl_collapse_imagelist_to_vector_delete(vmethod);

    return cpl_error_get_code();
}


static cpl_error_code hdrl_normalize_image_test(void)
{
    cpl_imagelist * data = cpl_imagelist_new();
    cpl_imagelist * errs = cpl_imagelist_new();
    cpl_imagelist * vnorm_d = cpl_imagelist_new();
    cpl_imagelist * vnorm_e = cpl_imagelist_new();
    cpl_vector * vnorm_d_, * vnorm_e_;
    cpl_array * acontrib;

    const size_t nz = 23;
    const size_t nx = 41;
    const size_t ny = 17;
    for (size_t i = 0; i < nz; i++) {
        cpl_image * img = cpl_image_new(nx, ny, HDRL_TYPE_DATA);
        cpl_image_add_scalar(img, i + 1);
        cpl_image * err = cpl_image_new(nx, ny, HDRL_TYPE_ERROR);
        cpl_image_add_scalar(err, i + 1);
        cpl_image_divide_scalar(err, 3.);
        cpl_imagelist_set(data, img, i);
        cpl_imagelist_set(errs, err, i);
    }


    hdrl_collapse_imagelist_to_vector_t * vmethod =
        hdrl_collapse_imagelist_to_vector_mean();
    hdrl_collapse_imagelist_to_vector_call(vmethod, data, errs,
                                           &vnorm_d_, &vnorm_e_, &acontrib,
                                           NULL);
    cpl_array_delete(acontrib);
    for (cpl_size i = 0; i < cpl_vector_get_size(vnorm_d_); i++) {
        cpl_image * dimg = cpl_image_new(nx, ny, HDRL_TYPE_DATA);
        cpl_image * eimg = cpl_image_new(nx, ny, HDRL_TYPE_ERROR);
        cpl_image_add_scalar(dimg, cpl_vector_get(vnorm_d_, i));
        cpl_image_add_scalar(eimg, cpl_vector_get(vnorm_e_, i));
        cpl_imagelist_set(vnorm_d, dimg, i);
        cpl_imagelist_set(vnorm_e, eimg, i);
    }
    cpl_vector_delete(vnorm_d_);
    cpl_vector_delete(vnorm_e_);


    /* Unknown mode */
    {
    	cpl_imagelist * ndata = cpl_imagelist_duplicate(data);
        cpl_imagelist * nerrs = cpl_imagelist_duplicate(errs);
        hdrl_normalize_imagelist_by_imagelist(vnorm_d, vnorm_e,
        		                              -1, ndata, nerrs);
        cpl_test_error(CPL_ERROR_UNSUPPORTED_MODE);
        cpl_imagelist_delete(ndata);
        cpl_imagelist_delete(nerrs);
    }

    /* additive */
    {
        cpl_imagelist * ndata = cpl_imagelist_duplicate(data);
        cpl_imagelist * nerrs = cpl_imagelist_duplicate(errs);
        hdrl_normalize_imagelist_by_imagelist(vnorm_d, vnorm_e,
                                              HDRL_SCALE_ADDITIVE,
                                              ndata, nerrs);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_image_abs(cpl_imagelist_get_const(data, 0),
                           cpl_imagelist_get_const(ndata, 0), HDRL_EPS_DATA);
        cpl_test_image_abs(cpl_imagelist_get_const(errs, 0),
                           cpl_imagelist_get_const(nerrs, 0), HDRL_EPS_ERROR);
        for (size_t i = 1; i < nz; i++) {
            cpl_image * expect_d = cpl_image_new(nx, ny, HDRL_TYPE_DATA);
            cpl_image * expect_e = cpl_image_new(nx, ny, HDRL_TYPE_ERROR);
            double escale = hypot(((i + 1) / 3.) / sqrt(nx * ny),
                                  (1. / 3.) / sqrt(nx * ny));
            cpl_image_add_scalar(expect_e, hypot(escale, (i + 1) / 3.));
            cpl_image_add_scalar(expect_d, 1);
            cpl_test_image_abs(expect_d, cpl_imagelist_get_const(ndata, i),
                               HDRL_EPS_DATA);
            cpl_test_image_abs(expect_e, cpl_imagelist_get_const(nerrs, i),
                               HDRL_EPS_ERROR * 4);
            cpl_image_delete(expect_d);
            cpl_image_delete(expect_e);
        }
        cpl_imagelist_delete(ndata);
        cpl_imagelist_delete(nerrs);
    }

    /* multiplicative */
    {
        cpl_imagelist * ndata = cpl_imagelist_duplicate(data);
        cpl_imagelist * nerrs = cpl_imagelist_duplicate(errs);
        hdrl_normalize_imagelist_by_imagelist(vnorm_d, vnorm_e,
                                              HDRL_SCALE_MULTIPLICATIVE,
                                              ndata, nerrs);
        cpl_test_error(CPL_ERROR_NONE);

        cpl_test_image_abs(cpl_imagelist_get_const(data, 0),
                           cpl_imagelist_get_const(ndata, 0), HDRL_EPS_DATA);
        cpl_test_image_abs(cpl_imagelist_get_const(errs, 0),
                           cpl_imagelist_get_const(nerrs, 0), HDRL_EPS_ERROR);
        for (size_t i = 1; i < nz; i++) {
            cpl_image * expect_d = cpl_image_new(nx, ny, HDRL_TYPE_DATA);
            cpl_image * expect_e = cpl_image_new(nx, ny, HDRL_TYPE_ERROR);
            /* off by about 10e-4 due to missing correlation */
            cpl_image_add_scalar(expect_e, 0.3338112308308246);
            cpl_image_add_scalar(expect_d, 1.);
            cpl_test_image_abs(expect_d, cpl_imagelist_get_const(ndata, i),
                               HDRL_EPS_DATA);
            cpl_test_image_abs(expect_e, cpl_imagelist_get_const(nerrs, i),
                               HDRL_EPS_ERROR * 4);
            cpl_image_delete(expect_d);
            cpl_image_delete(expect_e);
        }
        cpl_imagelist_delete(ndata);
        cpl_imagelist_delete(nerrs);
    }

    cpl_imagelist_delete(data);
    cpl_imagelist_delete(errs);
    cpl_imagelist_delete(vnorm_d);
    cpl_imagelist_delete(vnorm_e);
    hdrl_collapse_imagelist_to_vector_delete(vmethod);

    return cpl_error_get_code();
}



/* path related smoke tests */
void hdrl_path_test(void)
{
    char * cwd;
    int fd;

    cwd = hdrl_get_cwd();
    cpl_test_nonnull(cwd);

    fd = hdrl_get_tempfile(NULL, CPL_TRUE);
    cpl_test(fd >= 0);
    close(fd);

/*
    fd = hdrl_get_tempfile(NULL, CPL_FALSE);
    cpl_test(fd >= 0);
*/

    fd = hdrl_get_tempfile(cwd, CPL_TRUE);
    cpl_test(fd >= 0);

/*
    fd = hdrl_get_tempfile(cwd, CPL_FALSE);
    cpl_test(fd >= 0);
*/

    cpl_free(cwd);
}

void hdrl_string_test(void)
{
    char * dummy = NULL;
	char * s;
	int  result;

    s = hdrl_join_string(NULL, 0, "test");
    cpl_test_null(s);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    s = hdrl_join_string(NULL, 1, "test");
    result = strcmp(s, "test");
    cpl_test_zero(result);
    cpl_free(s);

    s = hdrl_join_string(NULL, 2, "test", "bla");
    result = strcmp(s, "testbla");
    cpl_test_zero(result);
    cpl_free(s);

    s = hdrl_join_string(".", 1, "test");
    result = strcmp(s, "test");
    cpl_test_zero(result);
    cpl_free(s);

    s = hdrl_join_string(".", 2, "test", "bla");
    result = strcmp(s, "test.bla");
    cpl_test_zero(result);
    cpl_free(s);

    s = hdrl_join_string("--", 3, "test", "bla", "val");
    result = strcmp(s, "test--bla--val");
    cpl_test_zero(result);
    cpl_free(s);

    s = hdrl_join_string("--", 3, "", "bla", "val");
    result = strcmp(s, "bla--val");
    cpl_test_zero(result);
    cpl_free(s);

    s = hdrl_join_string("--", 3, "test", "", "val");
    result = strcmp(s, "test--val");
    cpl_test_zero(result);
    cpl_free(s);

    s = hdrl_join_string(",", 3, "test", dummy, "val");
    result = strcmp(s, "test,val");
    cpl_test_zero(result);
    cpl_free(s);

    s = hdrl_join_string("--", 4, "", dummy, "val", "test");
    result = strcmp(s, "val--test");
    cpl_test_zero(result);
    cpl_free(s);

    s = hdrl_join_string("--", 3, dummy, "bla", "val");
    result = strcmp(s, "bla--val");
    cpl_test_zero(result);
    cpl_free(s);

    s = hdrl_join_string("--", 3, "test", "bla", "");
    result = strcmp(s, "test--bla");
    cpl_test_zero(result);
    cpl_free(s);

    s = hdrl_join_string("--", 3, "test", "bla", dummy);
    result = strcmp(s, "test--bla");
    cpl_test_zero(result);
    cpl_free(s);
}


void hdrl_pfilter_test(void)
{
    cpl_imagelist * list1 = cpl_imagelist_new();
    cpl_imagelist_set(list1, cpl_image_new(5, 5, CPL_TYPE_INT), 0);
    cpl_imagelist_set(list1, cpl_image_new(5, 5, CPL_TYPE_INT), 1);

    cpl_imagelist *list2;

    list2 = hdrl_bpm_filter_list(NULL, 5, 5, CPL_FILTER_MEDIAN);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_null(list2);

    list2 = hdrl_bpm_filter_list(list1, 5, 5, CPL_FILTER_MEDIAN);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    cpl_test_null(list2);

    cpl_imagelist_delete(list2);
    cpl_imagelist_delete(list1);

    size_t nx = 100;
    size_t any[] = {13, 22, 47, 100, 103, 104, 202, 203, 204,
                    542, 1034, 1239};
    size_t amx[] = {1, 1, 3, 3, 5, 7, 13};
    size_t amy[] = {1, 3, 1, 3, 5, 7, 13};
    for (size_t j = 0; j < ARRAY_LEN(amx); j++) {
        cpl_mask * m = cpl_mask_new(amx[j], amy[j]);
        cpl_matrix * k = cpl_matrix_new(amx[j], amy[j]);
        cpl_mask_not(m);
        cpl_matrix_add_scalar(k, 1);
        for (size_t i = 0; i < ARRAY_LEN(any); i++) {
            size_t ny = any[i];
            cpl_msg_info(cpl_func, "Testing ny %zu mask %zu,%zu", ny,
                         amx[j], amy[j]);
            cpl_image * img = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
            double * d = cpl_image_get_data_double(img);
            for (size_t kindex = 0; kindex < nx * ny; kindex++) {
                d[kindex] = kindex + rand() % 100;
                if (i % 2 == 0 && ((int)d[kindex] % 20) == 0) {
                    cpl_image_reject(img, (rand() % nx) + 1, (rand() % ny) + 1);
                }
            }
            cpl_image * orig_inp = cpl_image_duplicate(img);
            cpl_image * ro = cpl_image_duplicate(img);
            cpl_image * rp;

            cpl_image_filter_mask(ro, img, m, CPL_FILTER_MEDIAN, CPL_BORDER_FILTER);
            rp = hdrl_parallel_filter_image(img, NULL, m, CPL_FILTER_MEDIAN);
            cpl_test_image_abs(rp, ro, 0);
            /* input unchanged */
            cpl_test_image_abs(img, orig_inp, 0);
            cpl_image_delete(rp);

            cpl_image_filter(ro, img, k, CPL_FILTER_LINEAR, CPL_BORDER_FILTER);
            rp = hdrl_parallel_filter_image(img, k, NULL, CPL_FILTER_LINEAR);
            cpl_test_image_abs(rp, ro, 0);
            /* input unchanged */
            cpl_test_image_abs(img, orig_inp, 0);
            cpl_image_delete(rp);

            cpl_image_delete(ro);
            cpl_image_delete(img);
            cpl_image_delete(orig_inp);
        }
        cpl_mask_delete(m);
        cpl_matrix_delete(k);
    }
}

void hdrl_pconvert_test(void)
{
    cpl_size nx = 223;
    cpl_size ny = 223;
    cpl_error_code error_cpl;
    cpl_error_code error_hdrl;
    cpl_propertylist * plist = cpl_propertylist_new();
    cpl_propertylist_update_int(plist, "NAXIS", 2);
    cpl_propertylist_update_int(plist, "NAXIS1", nx);
    cpl_propertylist_update_int(plist, "NAXIS2", ny);
    cpl_propertylist_update_string(plist, "CTYPE1", "RA---ZPN");
    cpl_propertylist_update_string(plist, "CTYPE2", "DEC--ZPN");
    cpl_propertylist_update_double(plist, "CRVAL1", 149.947);
    cpl_propertylist_update_double(plist, "CRVAL2", 2.205);
    cpl_propertylist_update_double(plist, "CRPIX1", 5401.42);
    cpl_propertylist_update_double(plist, "CRPIX2", 6834.89);
    cpl_propertylist_update_double(plist, "CD1_1", 1.8072e-07);
    cpl_propertylist_update_double(plist, "CD1_2", 9.4796e-05);
    cpl_propertylist_update_double(plist, "CD2_1", -9.4820e-05);
    cpl_propertylist_update_double(plist, "CD2_2", 2.0167e-07);
    cpl_propertylist_update_double(plist, "PV2_1", 1.);
    cpl_propertylist_update_double(plist, "PV2_2", 0.);
    cpl_propertylist_update_double(plist, "PV2_3", 44.);
    cpl_propertylist_update_double(plist, "PV2_4", 0.);
    cpl_propertylist_update_double(plist, "PV2_5", -10300.);
    cpl_propertylist_update_string(plist, "CUNIT1", "deg");
    cpl_propertylist_update_string(plist, "CUNIT2", "deg");

    cpl_wcs * wcs = cpl_wcs_new_from_propertylist(plist);
    cpl_matrix * from = cpl_matrix_new(nx * ny, 2);
    for (cpl_size y = 0; y < ny; y++) {
        for (cpl_size x = 0; x < nx; x++) {
            cpl_matrix_set(from, (y * nx) + x, 0, x);
            cpl_matrix_set(from, (y * nx) + x, 1, y);
        }
    }
    cpl_matrix * to, *to2, *toc, *toc2;
    cpl_array * status, *statusc;
    hdrl_wcs_convert(wcs, from, &to, &status, CPL_WCS_PHYS2WORLD);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_wcs_convert(wcs, from, &toc, &statusc, CPL_WCS_PHYS2WORLD);

    cpl_test_matrix_abs(to, toc, 0.);
    cpl_test_array_abs(status, statusc, 0.);
    cpl_array_delete(status);
    cpl_array_delete(statusc);

    hdrl_wcs_convert(wcs, to, &to2, &status, CPL_WCS_WORLD2PHYS);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_wcs_convert(wcs, to, &toc2, &statusc, CPL_WCS_WORLD2PHYS);

    cpl_test_matrix_abs(to2, toc2, 0.);
    cpl_test_array_abs(status, statusc, 0.);

    cpl_matrix_delete(to);
    cpl_matrix_delete(toc);
    cpl_matrix_delete(to2);
    cpl_matrix_delete(toc2);
    cpl_array_delete(status);
    cpl_array_delete(statusc);

    /* invalid phys as world input */

    error_cpl = cpl_wcs_convert(wcs, from, &toc, &statusc, CPL_WCS_WORLD2PHYS);
    cpl_error_reset();
    error_hdrl = hdrl_wcs_convert(wcs, from, &to, &status, CPL_WCS_WORLD2PHYS);
    cpl_test_eq_error(error_hdrl, error_cpl);
    cpl_test_array_abs(status, statusc, 0.);

    cpl_matrix_delete(to);
    cpl_matrix_delete(toc);
    cpl_array_delete(status);
    cpl_array_delete(statusc);

    /* check error code - make sure that the hdrl function returns the same
     * error code than the cpl function does */

    error_cpl = cpl_wcs_convert(NULL, NULL, NULL, NULL, CPL_WCS_PHYS2WORLD);
    cpl_error_reset();
    error_hdrl = hdrl_wcs_convert(NULL, NULL, NULL, NULL, CPL_WCS_PHYS2WORLD);
    cpl_test_eq_error(error_hdrl, error_cpl);

    error_cpl = cpl_wcs_convert(wcs, from, &to, &status, (cpl_wcs_trans_mode)1421);
    cpl_error_reset();
    error_hdrl = hdrl_wcs_convert(wcs, from, &to, &status, (cpl_wcs_trans_mode)1421);
    cpl_test_eq_error(error_hdrl, error_cpl);

    to = NULL;
    status = NULL;
    cpl_propertylist_erase(plist, "NAXIS2");
    cpl_wcs_delete(wcs);
    wcs = cpl_wcs_new_from_propertylist(plist);
    error_cpl = cpl_wcs_convert(wcs, from, &to, &status, CPL_WCS_PHYS2WORLD);
    cpl_error_reset();
    cpl_matrix_delete(to);
    cpl_array_delete(status);
    error_hdrl = hdrl_wcs_convert(wcs, from, &to, &status, CPL_WCS_PHYS2WORLD);
    cpl_test_eq_error(error_hdrl, error_cpl);
    cpl_matrix_delete(to);
    cpl_array_delete(status);

    cpl_propertylist_delete(plist);
    cpl_wcs_delete(wcs);
    cpl_matrix_delete(from);
}

static cpl_error_code hdrl_airmass_test(void){

	cpl_errorstate prestate = cpl_errorstate_get();

	/* Temporary variables */
	hdrl_value airmass, airmass1, airmass2, airmass3;

	/* Aproximation methods */
	hdrl_airmass_approx typeAirmassAprox1 = HDRL_AIRMASS_APPROX_HARDIE;
	hdrl_airmass_approx typeAirmassAprox2 = HDRL_AIRMASS_APPROX_YOUNG_IRVINE;
	hdrl_airmass_approx typeAirmassAprox3 = HDRL_AIRMASS_APPROX_YOUNG;


	/* Input parameters based in MUSE data */

	hdrl_value ra1           = { 122.994945, 		0.};
	hdrl_value dec1          = {74.95304, 			0.};
	hdrl_value lst1          = {25407.072748, 		0.};
	hdrl_value exptime1      = {120., 				0.};
	hdrl_value geolat1       = {37.2236, 			0.};

	hdrl_value ra2           = {238.071555, 		0.};
	hdrl_value dec2          = {32.92533,			0.};
	hdrl_value lst2          = {60515.584209, 		0.};
	hdrl_value exptime2      = {300.,				0.};
	hdrl_value geolat2       = {37.2236,			0.};

	hdrl_value ra3           = {0.125, 				0.};
	hdrl_value dec3          = {-30.,				0.};
	hdrl_value lst3          = {69446.2765265328, 	0.};
	hdrl_value exptime3      = {3600.,				0.};
	hdrl_value geolat3       = {-24.625278,			0.};


	/******** Tests of failure cases (HARDIE approx method) ****************/

	/* Not valid Ra */
	airmass = hdrl_utils_airmass((hdrl_value){-1.,0.}, dec1, lst1, exptime1, geolat1, typeAirmassAprox1);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
	cpl_test(airmass.data == -1.);

	/* Not valid Dec */
	airmass = hdrl_utils_airmass(ra1, (hdrl_value){180.,0.}, lst1, exptime1, geolat1, typeAirmassAprox1);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
	cpl_test(airmass.data == -1.);

	/* Not valid Lst */
	airmass = hdrl_utils_airmass(ra1, dec1, (hdrl_value){-1.,0.}, exptime1, geolat1, typeAirmassAprox1);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
	cpl_test(airmass.data == -1.);

	/* Not valid Exptime */
	airmass = hdrl_utils_airmass(ra1, dec1, lst1, (hdrl_value){-1.,0.}, geolat1, typeAirmassAprox1);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
	cpl_test(airmass.data == -1.);

	/* Not valid Geolat */
	airmass = hdrl_utils_airmass(ra1, dec1, lst1, exptime1, (hdrl_value){180.,0.}, typeAirmassAprox1);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
	cpl_test(airmass.data == -1.);

	/* Not valid Airmass Approximation method */
	airmass = hdrl_utils_airmass(ra1, dec1, lst1, exptime1, geolat1, 0);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
	cpl_test(airmass.data == -1.);


	/***** Test without error propagation (HARDIE approx method) ***********/

	airmass1 = hdrl_utils_airmass(ra1, dec1, lst1, exptime1, geolat1, typeAirmassAprox1);
	cpl_test(cpl_errorstate_is_equal(prestate) == CPL_TRUE);

	airmass2 = hdrl_utils_airmass(ra2, dec2, lst2, exptime2, geolat2, typeAirmassAprox1);
	cpl_test(cpl_errorstate_is_equal(prestate) == CPL_TRUE);

	airmass3 = hdrl_utils_airmass(ra3, dec3, lst3, exptime3, geolat3, typeAirmassAprox1);
	cpl_test(cpl_errorstate_is_equal(prestate) == CPL_TRUE);

	/* check the result without error propagation */

	cpl_test(airmass1.data > 1.27803 - 0.001);
	cpl_test(airmass1.data < 1.27803 + 0.001);

	cpl_test(airmass2.data > 1.02529 - 0.001);
	cpl_test(airmass2.data < 1.02529 + 0.001);

	cpl_test(airmass3.data > 1.79364 - 0.001);
	cpl_test(airmass3.data < 1.79364 + 0.001);


	/****** Tests with error propagation (HARDIE approx method) ************/

	double delta        	= 1e-2;
	double deltaRa      	= delta;
	double deltaDec     	= delta;
	double deltaLst     	= delta;
	double deltaExptime 	= delta;
	double deltaGeolat  	= delta;

	ra1.error           	= deltaRa      * fabs(ra1.data);
	dec1.error          	= deltaDec     * fabs(dec1.data);
	lst1.error          	= deltaLst     * fabs(lst1.data);
	exptime1.error      	= deltaExptime * fabs(exptime1.data);
	geolat1.error      	    = deltaGeolat  * fabs(geolat1.data);

	ra2.error           	= deltaRa      * fabs(ra2.data);
	dec2.error          	= deltaDec     * fabs(dec2.data);
	lst2.error          	= deltaLst     * fabs(lst2.data);
	exptime2.error      	= deltaExptime * fabs(exptime2.data);
	geolat2.error      	    = deltaGeolat  * fabs(geolat2.data);

	ra3.error           	= deltaRa      * fabs(ra3.data);
	dec3.error          	= deltaDec     * fabs(dec3.data);
	lst3.error          	= deltaLst     * fabs(lst3.data);
	exptime3.error      	= deltaExptime * fabs(exptime3.data);
	geolat3.error      	    = deltaGeolat  * fabs(geolat3.data);

	airmass1 = hdrl_utils_airmass(ra1, dec1, lst1, exptime1, geolat1, typeAirmassAprox1);
	cpl_test(cpl_errorstate_is_equal(prestate) == CPL_TRUE);

	airmass2 = hdrl_utils_airmass(ra2, dec2, lst2, exptime2, geolat2, typeAirmassAprox1);
	cpl_test(cpl_errorstate_is_equal(prestate) == CPL_TRUE);

	airmass3 = hdrl_utils_airmass(ra3, dec3, lst3, exptime3, geolat3, typeAirmassAprox1);
	cpl_test(cpl_errorstate_is_equal(prestate) == CPL_TRUE);

	/* check the result with error propagation */

	cpl_test(airmass1.data  > 1.27803   - 0.001 );
	cpl_test(airmass1.data  < 1.27803   + 0.001 );
	cpl_test(airmass1.error > 0.0136602 - 0.0001);
	cpl_test(airmass1.error < 0.0136602 + 0.0001);

	cpl_test(airmass2.data  > 1.02529   - 0.001 );
	cpl_test(airmass2.data  < 1.02529   + 0.001 );
	cpl_test(airmass2.error > 0.0216347 - 0.0001);
	cpl_test(airmass2.error < 0.0216347 + 0.0001);

	cpl_test(airmass3.data  > 1.79364   - 0.001 );
	cpl_test(airmass3.data  < 1.79364   + 0.001 );
	cpl_test(airmass3.error > 0.128632  - 0.0001);
	cpl_test(airmass3.error < 0.128631  + 0.0001);


	/*** Test with different kinds of approximation and error propagation ***/

	airmass1 = hdrl_utils_airmass(ra1, dec1, lst1, exptime1, geolat1, typeAirmassAprox1);
	cpl_test(cpl_errorstate_is_equal(prestate) == CPL_TRUE);

	airmass2 = hdrl_utils_airmass(ra1, dec1, lst1, exptime1, geolat1, typeAirmassAprox2);
	cpl_test(cpl_errorstate_is_equal(prestate) == CPL_TRUE);

	airmass3 = hdrl_utils_airmass(ra1, dec1, lst1, exptime1, geolat1, typeAirmassAprox3);
	cpl_test(cpl_errorstate_is_equal(prestate) == CPL_TRUE);


	/* check the result with error propagation and different approximations */

	cpl_test(airmass1.data  > 1.27803   - 0.001 );
	cpl_test(airmass1.data  < 1.27803   + 0.001 );
	cpl_test(airmass1.error > 0.0136602 - 0.0001);
	cpl_test(airmass1.error < 0.0136602 + 0.0001);

	cpl_test(airmass2.data  > 1.2778    - 0.001 );
	cpl_test(airmass2.data  < 1.2778    + 0.001 );
	cpl_test(airmass2.error > 0.0135473 - 0.0001);
	cpl_test(airmass2.error < 0.0135473 + 0.0001);

	cpl_test(airmass3.data  > 1.27755   - 0.001 );
	cpl_test(airmass3.data  < 1.27755   + 0.001 );
	cpl_test(airmass3.error > 0.0135339 - 0.0001);
	cpl_test(airmass3.error < 0.0135339 + 0.0001);


	return cpl_error_get_code();
}

static cpl_error_code hdrl_license_test(void)
{
	const char* str = hdrl_get_license();
	cpl_test_nonnull(str);

	return cpl_error_get_code();
}

static cpl_error_code hdrl_region_test(void)
{
	/* Test wrong hdrl_parameter */
	hdrl_parameter* pFake = hdrl_collapse_mean_parameter_create();
	hdrl_rect_region_parameter_verify(pFake, 10, 10);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

	/* Test error verify */
	hdrl_parameter* pErr1 = hdrl_rect_region_parameter_create( 0,  1, 10, 10);
	hdrl_parameter* pErr2 = hdrl_rect_region_parameter_create( 1,  0, 10, 10);
	hdrl_parameter* pErr3 = hdrl_rect_region_parameter_create( 1,  1,  0, 10);
	hdrl_parameter* pErr4 = hdrl_rect_region_parameter_create( 1,  1, 10,  0);
	hdrl_parameter* pErr5 = hdrl_rect_region_parameter_create(10,  1,  1, 10);
	hdrl_parameter* pErr6 = hdrl_rect_region_parameter_create( 1, 10, 10,  1);
	cpl_test_error(CPL_ERROR_NONE);

	hdrl_rect_region_parameter_verify(NULL, 10, 10);
	cpl_test_error(CPL_ERROR_NULL_INPUT);

	hdrl_rect_region_parameter_verify(pErr1, 10, 10);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

	hdrl_rect_region_parameter_verify(pErr2, 10, 10);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

	hdrl_rect_region_parameter_verify(pErr3, 10, 10);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

	hdrl_rect_region_parameter_verify(pErr4, 10, 10);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

	hdrl_rect_region_parameter_verify(pErr5, 10, 10);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

	hdrl_rect_region_parameter_verify(pErr6, 10, 10);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);


	/* Correct verify */
	hdrl_parameter* p = hdrl_rect_region_parameter_create(1, 1, 10, 10);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(p);

	hdrl_rect_region_parameter_check(p);
	cpl_test_error(CPL_ERROR_NONE);

	hdrl_rect_region_parameter_update(p, 1, 1, 20, 20);
	cpl_test_error(CPL_ERROR_NONE);

	hdrl_rect_region_parameter_verify(p, 10, 20);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

	hdrl_rect_region_parameter_verify(p, 20, 10);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

	hdrl_rect_region_get_llx(NULL);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_size llx = hdrl_rect_region_get_llx(p);
	cpl_test_eq(llx, 1);

	hdrl_rect_region_get_lly(NULL);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_size lly = hdrl_rect_region_get_lly(p);
	cpl_test_eq(lly, 1);

	hdrl_rect_region_get_urx(NULL);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_size urx = hdrl_rect_region_get_urx(p);
	cpl_test_eq(urx, 20);

	hdrl_rect_region_get_ury(NULL);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_size ury = hdrl_rect_region_get_ury(p);
	cpl_test_eq(ury, 20);



	/* Test null parameters */

	cpl_parameterlist *plist = hdrl_rect_region_parameter_create_parlist(
			NULL, "test", "region-", p);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(plist);

	plist = hdrl_rect_region_parameter_create_parlist(
			"RECIPE", NULL, "region-", p);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(plist);

	plist = hdrl_rect_region_parameter_create_parlist(
			"RECIPE", "test", NULL, p);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(plist);

	plist = hdrl_rect_region_parameter_create_parlist(
			"RECIPE", "test", "region-", NULL);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(plist);


	/* Wrong parse */
	cpl_parameterlist *plFake = hdrl_rect_region_parameter_create_parlist(
			"RECIPE", "test", "region-", pFake);
	cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);
	cpl_test_null(plFake);
	hdrl_parameter_delete(pFake);


	cpl_parameterlist *plist1 = hdrl_rect_region_parameter_create_parlist(
			"RECIPE", "test", "region-", p);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(plist1);

	hdrl_parameter *out1 = hdrl_rect_region_parameter_parse_parlist(
			NULL, "test", "region-");
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(out1);

	out1 = hdrl_rect_region_parameter_parse_parlist(
			plist1, NULL, "region-");
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(out1);

	out1 = hdrl_rect_region_parameter_parse_parlist(
			plist1, "test", NULL);
	cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
	cpl_test_null(out1);

	out1 = hdrl_rect_region_parameter_parse_parlist(
			plist1, "test", "region-");
	cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
	cpl_test_null(out1);


	/* Right parse */
	cpl_parameterlist *plist2 = hdrl_rect_region_parameter_create_parlist(
			"test", "", "region-", p);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(plist2);

	hdrl_parameter *out2 = hdrl_rect_region_parameter_parse_parlist(
			plist2, "test", "region-");
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(out2);


	/* Region Fix negatives */

	hdrl_rect_region_fix_negatives(NULL, 10, 10);
	cpl_test_error(CPL_ERROR_NULL_INPUT);

	hdrl_parameter* pFake2 = hdrl_collapse_mean_parameter_create();
	hdrl_rect_region_fix_negatives(pFake2, 10, 10);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
	hdrl_parameter_delete(pFake2);

	hdrl_rect_region_fix_negatives(p, 0, 0);
	cpl_test_error(CPL_ERROR_NONE);

    hdrl_parameter* pErr = hdrl_rect_region_parameter_create(-1, -1, -1, -1);

	hdrl_rect_region_fix_negatives(pErr, 0, 0);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

	hdrl_rect_region_fix_negatives(pErr, 2, 2);
	cpl_test_error(CPL_ERROR_NONE);


	/* Clean up */
	cpl_parameterlist_delete(plist1);
	cpl_parameterlist_delete(plist2);

	hdrl_parameter_delete(pErr1);
	hdrl_parameter_delete(pErr2);
	hdrl_parameter_delete(pErr3);
	hdrl_parameter_delete(pErr4);
	hdrl_parameter_delete(pErr5);
	hdrl_parameter_delete(pErr6);
	hdrl_parameter_delete(pErr);

	hdrl_parameter_delete(p);
	hdrl_parameter_delete(out1);
	hdrl_parameter_delete(out2);

	return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of utility module
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    hdrl_image_to_vector_test();
    hdrl_imagelist_to_vector_test();
    hdrl_imagelist_cplwrap();
    hdrl_normalize_test();
    hdrl_normalize_image_test();
    hdrl_path_test();
    hdrl_string_test();
    hdrl_pfilter_test();
    hdrl_pconvert_test();
    hdrl_airmass_test();
    hdrl_license_test();
    hdrl_region_test();

    return cpl_test_end(0);
}
