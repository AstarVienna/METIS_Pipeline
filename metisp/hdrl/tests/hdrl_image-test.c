/* $Id: hdrl_image-test.c,v 1.5 2013-10-23 09:13:56 jtaylor Exp $
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
 * $Date: 2013-10-23 09:13:56 $
 * $Revision: 1.5 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                    Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_image.h"
#include "hdrl_test.h"
#include "hdrl_imagelist.h"
#include "hdrl_buffer.h"
#include "hdrl_types.h"

#include <cpl.h>

#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <stdio.h>

#include <limits.h>

#ifndef ARRAY_LEN
#define ARRAY_LEN(a) sizeof((a))/sizeof((a)[0])
#endif

/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_image_test   
            Testing of hdrl_image module
 */
/*----------------------------------------------------------------------------*/

static void test_basic(void)
{
    hdrl_image * img;
    cpl_image * cimg, * cerr;
    cpl_error_code error;

    hdrl_image_delete(NULL);

    /* empty creation */

    img = hdrl_image_new(5, 5);
    cpl_test_nonnull(img);
    cpl_test_error(CPL_ERROR_NONE);
    hdrl_image_delete(img);

    img = hdrl_image_new(0, 5);
    cpl_test_null(img);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    img = hdrl_image_new(5, 0);
    cpl_test_null(img);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    img = hdrl_image_new(0, 0);
    cpl_test_null(img);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    img = hdrl_image_create(NULL, NULL);
    cpl_test_null(img);
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    /* creation with cpl image */

    cimg = cpl_image_new(5, 6, CPL_TYPE_DOUBLE);
    cerr = cpl_image_new(5, 6, CPL_TYPE_DOUBLE);

    img = hdrl_image_create(NULL, cerr);
    cpl_test_null(img);
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    img = hdrl_image_create(cimg, NULL);
    cpl_test_nonnull(img);
    hdrl_image_delete(img);

    img = hdrl_image_create(cimg, cerr);
    cpl_test_nonnull(img);

    /* dump the structure */
    error = hdrl_image_dump_structure(img, stderr);
    cpl_test_eq_error(error, CPL_ERROR_NONE);

    error = hdrl_image_dump_window(img, 1, 1, 3, 3, stderr);
    cpl_test_eq_error(error, CPL_ERROR_NONE);

    hdrl_image_delete(img);

    /* with bpm */

    cpl_image_reject(cimg, 2, 3);
    img = hdrl_image_create(cimg, cerr);
    cpl_test_nonnull(img);
    cpl_test_eq(hdrl_image_count_rejected(img), 1);
    hdrl_image_delete(img);

    cpl_image_reject(cerr, 2, 3);
    img = hdrl_image_create(cimg, cerr);
    cpl_test_eq(hdrl_image_count_rejected(img), 1);
    cpl_test_nonnull(img);
    hdrl_image_delete(img);

    /* incompatible bpm (emits warning) */
    cpl_image_reject(cerr, 2, 4);
    img = hdrl_image_create(cimg, cerr);
    cpl_test_eq(hdrl_image_count_rejected(img), 1);
    cpl_test_nonnull(img);
    hdrl_image_delete(img);

    cpl_image_accept_all(cimg);
    img = hdrl_image_create(cimg, cerr);
    cpl_test_eq(hdrl_image_count_rejected(img), 0);
    cpl_test_nonnull(img);
    hdrl_image_delete(img);

    cpl_image_delete(cerr);
    cerr = cpl_image_new(2, 6, CPL_TYPE_DOUBLE);
    img = hdrl_image_create(cimg, cerr);
    cpl_test_null(img);
    cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

    cpl_image_delete(cerr);
    cerr = cpl_image_new(5, 2, CPL_TYPE_DOUBLE);
    img = hdrl_image_create(cimg, cerr);
    cpl_test_null(img);
    cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

    /* reject/accept */
    img = hdrl_image_new(5, 5);
    hdrl_image_reject(img, 4, 5);
    cpl_test(hdrl_image_is_rejected(img, 4, 5));
    hdrl_image_accept(img, 4, 5);
    cpl_test(!hdrl_image_is_rejected(img, 4, 5));

    hdrl_image_reject(img, 5, 6);
    cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);
    hdrl_image_reject(img, 0, 6);
    cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);
    hdrl_image_accept(img, 6, 5);
    cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);
    hdrl_image_accept(img, 0, 5);
    cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);

    hdrl_image_reject(img, 4, 5);
    hdrl_image_accept_all(img);
    cpl_test(!hdrl_image_is_rejected(img, 4, 5));
    hdrl_image_delete(img);

    cpl_image_delete(cimg);
    cpl_image_delete(cerr);
}

static void test_extract(void)
{
    cpl_size nx = 5;
    cpl_size ny = 13;
    hdrl_image * himg = hdrl_image_new(nx, ny);
    hdrl_image * ex;
    hdrl_image_add_scalar(himg, (hdrl_value){1., 1.});

    ex = hdrl_image_extract(himg, 1, 1, nx, ny);
    cpl_test_nonnull(ex);
    hdrl_test_image_abs(himg, ex, 0);
    hdrl_image_delete(ex);

    ex = hdrl_image_extract(himg, 1, 1, 0, 0);
    cpl_test_nonnull(ex);
    hdrl_test_image_abs(himg, ex, 0);
    hdrl_image_delete(ex);

    ex = hdrl_image_extract(himg, 0, 0, 0, 0);
    cpl_test_nonnull(ex);
    cpl_test_eq(hdrl_image_get_size_x(ex), 1);
    cpl_test_eq(hdrl_image_get_size_y(ex), 1);
    hdrl_image_delete(ex);

    ex = hdrl_image_extract(himg, 2, 2, -1, -1);
    cpl_test_nonnull(ex);
    cpl_test_eq(hdrl_image_get_size_x(ex), nx - 2);
    cpl_test_eq(hdrl_image_get_size_y(ex), ny - 2);
    hdrl_image_delete(ex);

    ex = hdrl_image_extract(himg, 2, 2, -1, 2 * ny);
    cpl_test_null(ex);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    ex = hdrl_image_extract(himg, 2, 2, -1, -2 * ny);
    cpl_test_null(ex);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    ex = hdrl_image_extract(himg, 2, 2, -2 * nx, -2);
    cpl_test_null(ex);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    ex = hdrl_image_extract(himg, 2, -2 * ny, -2, -2);
    cpl_test_null(ex);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    ex = hdrl_image_extract(himg, -2 * nx, -2, -2, -2);
    cpl_test_null(ex);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    hdrl_image_delete(himg);
}

static void test_power(void)
{
    hdrl_value v;
    hdrl_image * himg = hdrl_image_new(1, 1);
    hdrl_image_set_pixel(himg, 1, 1, (hdrl_value){2., 0.5});
    hdrl_image_pow_scalar(himg, (hdrl_value){2., 0.});
    v = hdrl_image_get_pixel(himg, 1, 1, NULL);
    cpl_test_rel(v.data, 4., HDRL_EPS_DATA);
    cpl_test_rel(v.error, 2.0, HDRL_EPS_ERROR);

    hdrl_image_set_pixel(himg, 1, 1, (hdrl_value){2., 0.5});
    hdrl_image_pow_scalar(himg, (hdrl_value){4., 0.});
    v = hdrl_image_get_pixel(himg, 1, 1, NULL);
    cpl_test_rel(v.data, 16, HDRL_EPS_DATA);
    cpl_test_rel(v.error, 16, HDRL_EPS_ERROR);

    hdrl_image_set_pixel(himg, 1, 1, (hdrl_value){2., 0.5});
    hdrl_image_pow_scalar(himg, (hdrl_value){-1., 0.});
    v = hdrl_image_get_pixel(himg, 1, 1, NULL);
    cpl_test_rel(v.data, 0.5, HDRL_EPS_DATA);
    cpl_test_rel(v.error, 0.125, HDRL_EPS_ERROR);

    hdrl_image_set_pixel(himg, 1, 1, (hdrl_value){2., 0.5});
    hdrl_image_pow_scalar(himg, (hdrl_value){-2., 0.});
    v = hdrl_image_get_pixel(himg, 1, 1, NULL);
    cpl_test_rel(v.data, 0.25, HDRL_EPS_DATA);
    cpl_test_rel(v.error, 0.125, HDRL_EPS_ERROR); /* yes the same as ^-1 */

    hdrl_image_set_pixel(himg, 1, 1, (hdrl_value){2., 0.5});
    hdrl_image_pow_scalar(himg, (hdrl_value){-4., 0.});
    v = hdrl_image_get_pixel(himg, 1, 1, NULL);
    cpl_test_rel(v.data, 0.0625, HDRL_EPS_DATA);
    cpl_test_rel(v.error, 0.0625, HDRL_EPS_ERROR);

    hdrl_image_set_pixel(himg, 1, 1, (hdrl_value){2., 0.3});
    hdrl_image_pow_scalar(himg, (hdrl_value){-3., 0.0});
    v = hdrl_image_get_pixel(himg, 1, 1, NULL);
    cpl_test_rel(v.data, 1.0/8.0, HDRL_EPS_DATA);
    cpl_test_rel(v.error, 0.05625, HDRL_EPS_ERROR);

    hdrl_image_delete(himg);
}

static void test_exp(void)
{
    hdrl_value v;
    hdrl_image * himg = hdrl_image_new(1, 1);
    hdrl_image_set_pixel(himg, 1, 1, (hdrl_value){2., 0.0});
    hdrl_image_exp_scalar(himg, (hdrl_value){2., 0.5});
    v = hdrl_image_get_pixel(himg, 1, 1, NULL);
    cpl_test_rel(v.data, 4., HDRL_EPS_DATA);
    cpl_test_rel(v.error, 2.0, HDRL_EPS_ERROR);


    hdrl_image_set_pixel(himg, 1, 1, (hdrl_value){4., 0.});
    hdrl_image_exp_scalar(himg, (hdrl_value){2., 0.5});
    v = hdrl_image_get_pixel(himg, 1, 1, NULL);
    cpl_test_rel(v.data, 16, HDRL_EPS_DATA);
    cpl_test_rel(v.error, 16, HDRL_EPS_ERROR);

    hdrl_image_set_pixel(himg, 1, 1, (hdrl_value){-1., 0.});
    hdrl_image_exp_scalar(himg, (hdrl_value){2., 0.5});
    v = hdrl_image_get_pixel(himg, 1, 1, NULL);
    cpl_test_rel(v.data, 0.5, HDRL_EPS_DATA);
    cpl_test_rel(v.error, 0.125, HDRL_EPS_ERROR);

    hdrl_image_set_pixel(himg, 1, 1, (hdrl_value){-2., 0.});
    hdrl_image_exp_scalar(himg, (hdrl_value){2., 0.5});
    v = hdrl_image_get_pixel(himg, 1, 1, NULL);
    cpl_test_rel(v.data, 0.25, HDRL_EPS_DATA);
    cpl_test_rel(v.error, 0.125, HDRL_EPS_ERROR);

    hdrl_image_set_pixel(himg, 1, 1, (hdrl_value){-4., 0.});
    hdrl_image_exp_scalar(himg, (hdrl_value){2., 0.5});
    v = hdrl_image_get_pixel(himg, 1, 1, NULL);
    cpl_test_rel(v.data, 0.0625, HDRL_EPS_DATA);
    cpl_test_rel(v.error, 0.0625, HDRL_EPS_ERROR);

    hdrl_image_delete(himg);
}


static void test_copy(void)
{
    hdrl_image * dst = hdrl_image_new(50, 50);
    hdrl_image * src = hdrl_image_new(30, 30);
    hdrl_image * expected = hdrl_image_new(50, 50);

    hdrl_image_copy(dst, src, 10, 10);
    hdrl_test_image_abs(dst, expected, 0);

    hdrl_image_reject(expected, 1, 1);
    /* bypass hdrl_image functions */
    cpl_image_reject(hdrl_image_get_image(src), 1, 1);

    hdrl_image_copy(dst, src, 10, 10);
    hdrl_test_image_abs(dst, expected, 0);

    hdrl_image_delete(dst);
    hdrl_image_delete(src);
    hdrl_image_delete(expected);
}

static void test_insert(void)
{
    hdrl_image * dst = hdrl_image_new(50, 50);
    hdrl_image * dst2 = hdrl_image_new(50, 50);
    cpl_image * im1 = cpl_image_new(50, 50, HDRL_TYPE_DATA);
    cpl_image * im2 = cpl_image_new(50, 50, HDRL_TYPE_ERROR);
    hdrl_image * him = hdrl_image_create(im1, im2);
    cpl_image_reject(im1, 1, 1);
    hdrl_image_reject(him, 1, 1);

    hdrl_image_copy(dst2, him, 1, 1);
    hdrl_image_insert(dst, im1, im2, 1, 1);
    hdrl_test_image_abs(dst, dst2, 0);

    hdrl_image_insert(dst, im1, NULL, 1, 1);
    hdrl_test_image_abs(dst, dst2, 0);

    hdrl_image_delete(dst);
    hdrl_image_delete(dst2);
    hdrl_image_delete(him);
    cpl_image_delete(im1);
    cpl_image_delete(im2);
}

static void test_reduce(void)
{
    {
        size_t nx = 53, ny = 2310;
        hdrl_value m;
        hdrl_image * a = hdrl_image_new(nx, ny);
        hdrl_image * b = hdrl_image_new(nx, ny);
        hdrl_image * c = hdrl_image_new(nx, ny);
        hdrl_imagelist * hl = hdrl_imagelist_new();
        hdrl_image_add_scalar(a, (hdrl_value){5., 3.2});
        hdrl_image_add_scalar(b, (hdrl_value){7., 1.2});
        hdrl_image_add_scalar(b, (hdrl_value){-3., .2});

        m = hdrl_image_get_mean(a);
        cpl_test_abs(m.data, 5., HDRL_EPS_DATA);
        cpl_test_abs(m.error, 3.2 / sqrt(nx * ny), HDRL_EPS_ERROR * nx * ny);

        m = hdrl_image_get_weighted_mean(a);
        cpl_test_abs(m.data, 5., HDRL_EPS_DATA);
        cpl_test_abs(m.error, 3.2 / sqrt(nx * ny), HDRL_EPS_ERROR * nx * ny);

        m = hdrl_image_get_sigclip_mean(a, 3., 3., 100);
        cpl_test_abs(m.data, 5., HDRL_EPS_DATA);
        cpl_test_abs(m.error, 3.2 / sqrt(nx * ny), HDRL_EPS_ERROR * nx * ny);

        hdrl_imagelist_set(hl, a, 0);
        hdrl_imagelist_set(hl, b, 1);
        hdrl_imagelist_set(hl, c, 2);
        hdrl_image * r;
        cpl_image * con;

        /* must be equivalent */
        hdrl_imagelist_collapse_mean(hl, &r, &con);
        hdrl_image_add_image(a, b);
        hdrl_image_add_image(a, c);
        hdrl_image_div_scalar(a, (hdrl_value){3., 0.});
        hdrl_test_image_abs(a, r, HDRL_EPS_DATA);

        hdrl_image_delete(r);
        cpl_image_delete(con);
        hdrl_imagelist_delete(hl);
    }
    { /* Test sigmaclipped mean */
        hdrl_value m;
        /* gauss mean 100 sigma 3.5 and 2 outliers */
        double values[] = {92, 93, 94, 94, 95, 95, 96, 96, 96, 97,
            97, 97, 97, 98, 98, 98, 98, 99, 99, 99,
            99, 100, 100, 100, 100, 100, 101, 101, 101, 101,
            102, 102, 102, 102, 103, 103, 103, 103, 104, 104,
            104, 105, 105, 106, 106, 107, 108, 500, 600 };

        cpl_image * data = cpl_image_wrap(7, 7, CPL_TYPE_DOUBLE, values);
        cpl_image * errors = cpl_image_new(7, 7, CPL_TYPE_DOUBLE);
        cpl_image_add_scalar(errors, 1);

        cpl_image_set(errors, 7, 7, 100000.);
        cpl_image_set(errors, 6, 7, 10000.);

        hdrl_image * sigimage = hdrl_image_create(data, errors);

        m = hdrl_image_get_sigclip_mean(sigimage, 3., 3., 100);
        cpl_test_rel(m.data, 100., HDRL_EPS_DATA * 49);
        cpl_test_rel(m.error, 1 / sqrt(7 * 7 - 2), HDRL_EPS_ERROR * 49);
        cpl_image_unwrap(data);
        cpl_image_delete(errors);
        hdrl_image_delete(sigimage);
    }
    { /* Test minmax rejected mean */
        hdrl_value m;
        /* gauss mean 100 sigma 3.5 and 2 outliers */
        double values[] = {-100000, 93, 94, 94, 95, 95, 96, 96, 96, 97,
            97, 97, 97, 98, 98, 98, 98, 99, 99, 99,
            99, 100, 100, 100, 100, 100, 101, 101, 101, 101,
            102, 102, 102, 102, 103, 103, 103, 103, 104, 104,
            104, 105, 105, 106, 106, 107, 108, 100000, 500000 };

        cpl_image * data = cpl_image_wrap(7, 7, CPL_TYPE_DOUBLE, values);
        cpl_image * errors = cpl_image_new(7, 7, CPL_TYPE_DOUBLE);
        cpl_image_add_scalar(errors, 1);

        cpl_image_set(errors, 7, 7,100000.);
        cpl_image_set(errors, 6, 7,10000.);
        cpl_image_set(errors, 1, 1,1000.);

        /*cpl_image_save(data,"data.fits", CPL_TYPE_FLOAT, NULL,
                       CPL_IO_CREATE);
        cpl_image_save(errors,"errors.fits", CPL_TYPE_FLOAT, NULL,
                       CPL_IO_CREATE);*/

        hdrl_image * minmaximage = hdrl_image_create(data, errors);

        m = hdrl_image_get_minmax_mean(minmaximage, 0, 0);
        cpl_test_rel(m.data, 10298.122448979591, HDRL_EPS_DATA * 49);
        m = hdrl_image_get_minmax_mean(minmaximage, 0, 1);
        cpl_test_rel(m.data, 96.0, 0.005);
        m = hdrl_image_get_minmax_mean(minmaximage, 0, 2);
        cpl_test_rel(m.data, -2029.6170212765958, HDRL_EPS_DATA * 49);
        m = hdrl_image_get_minmax_mean(minmaximage, 1, 2);
        cpl_test_rel(m.data, 100.17391304347827, HDRL_EPS_DATA * 49);

        hdrl_image_delete(minmaximage);
        cpl_image_delete(errors);
        cpl_image_unwrap(data);
    }
    /* test sum, sqsum */
    {
        size_t nx = 3, ny = 1;
        hdrl_value m;
        hdrl_image * a = hdrl_image_new(nx, ny);
        hdrl_image_set_pixel(a, 1, 1, (hdrl_value){1 , 0.5});
        hdrl_image_set_pixel(a, 2, 1, (hdrl_value){2., 1.5});
        hdrl_image_set_pixel(a, 3, 1, (hdrl_value){3., 2.5});
        hdrl_image_reject(a, 1, 1);

        m = hdrl_image_get_sum(NULL);
        cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
        cpl_test(isnan(m.data) && isnan(m.error));

        m = hdrl_image_get_sum(a);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test(!isnan(m.data) && !isnan(m.error));

        cpl_test_abs(m.data, 5., HDRL_EPS_DATA);
        cpl_test_abs(m.error, sqrt(1.5 * 1.5 + 2.5 * 2.5), HDRL_EPS_ERROR * 3);

        m = hdrl_image_get_sqsum(a);
        cpl_test_abs(m.data, 4. + 9., HDRL_EPS_DATA);
        cpl_test_abs(m.error, 16.15549442140351, HDRL_EPS_ERROR * 6);
        hdrl_image_delete(a);
    }
}

static inline void test_buffer(void)
{
    hdrl_buffer    *buf = hdrl_buffer_new();
    hdrl_imagelist *hl  = hdrl_imagelist_new();

#if (__WORDSIZE == 64)

    cpl_size iMax    = 100;     /* Previously = 900 */
    cpl_size sizeImg =  64;		/* Previously = 500 */

    for (cpl_size i = 0; i < iMax; i++) {
        hdrl_image * img = hdrl_image_new_from_buffer(sizeImg, sizeImg, buf);
        hdrl_imagelist_set(hl, img, i);
        hdrl_image_add_scalar(img, (hdrl_value){1, 1});
    }
    cpl_msg_warning(cpl_func, "alloc done");

    hdrl_image *m;
    cpl_image  *c;
    hdrl_imagelist_collapse_mean(hl, &m, &c);
    hdrl_image_delete(m);
    cpl_image_delete(c);
    cpl_msg_warning(cpl_func, "collapse done");


#endif


    hdrl_imagelist_delete(hl);

    hdrl_buffer_delete(buf);
}

static void test_create(void)
{
	/* Create reference images */
	size_t nx = 10;
	size_t ny = 100;
	hdrl_image *a = hdrl_image_new(nx, ny);
	hdrl_image *b = hdrl_image_new(nx, ny);

	/* Reject value */
	hdrl_image_reject_value(a, CPL_VALUE_NAN);
	hdrl_image_reject_value(b, CPL_VALUE_NAN);

	/* Add information */
	hdrl_value hValue = {2., 0.5};
	hdrl_image_add_scalar(a, hValue);
	hdrl_image_add_scalar(b, hValue);

	hdrl_image *new1;
	hdrl_image *new2;
	hdrl_image *new3;
	hdrl_image *new4;
	hdrl_image *new5;
	hdrl_image *new6;

	/* Basic operations */

	new1 = hdrl_image_add_image_create(NULL, b);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(new1);
	new1 = hdrl_image_add_image_create(a, NULL);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(new1);
	new1 = hdrl_image_add_image_create(a, b);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(new1);

	new2 = hdrl_image_sub_image_create(NULL, b);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(new2);
	new2 = hdrl_image_sub_image_create(a, NULL);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(new2);
	new2 = hdrl_image_sub_image_create(a, b);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(new2);


	new3 = hdrl_image_mul_image_create(NULL, b);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(new3);
	new3 = hdrl_image_mul_image_create(a, NULL);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(new3);
	new3 = hdrl_image_mul_image_create(a, b);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(new3);


	new4 = hdrl_image_div_image_create(NULL, b);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(new4);
	new4 = hdrl_image_div_image_create(a, NULL);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(new4);
	new4 = hdrl_image_div_image_create(a, b);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(new4);


	/* Pow and Exp operations */

	new5 = hdrl_image_pow_scalar_create(NULL, hValue);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(new5);
	new5 = hdrl_image_pow_scalar_create(a, hValue);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(new5);

	new6 = hdrl_image_exp_scalar_create(NULL, hValue);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(new6);
	new6 = hdrl_image_exp_scalar_create(a, hValue);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(new6);


	/* Clean up */
    hdrl_image_delete(a);
    hdrl_image_delete(b);

    hdrl_image_delete(new1);
    hdrl_image_delete(new2);
    hdrl_image_delete(new3);
    hdrl_image_delete(new4);

    hdrl_image_delete(new5);
    hdrl_image_delete(new6);
}


/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of hdrl_image
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    test_basic();
    test_power();
    test_exp();
    test_copy();
    test_insert();
    test_extract();
    test_reduce();

    test_create();

    test_buffer();

    return cpl_test_end(0);
}
