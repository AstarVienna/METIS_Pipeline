/*
 * This file is part of the HDRL
 * Copyright (C) 2013,2014 European Southern Observatory
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

#include "hdrl_combine.h"
#include "hdrl_types.h"

#include <cpl.h>

#include <math.h>
#include <stdlib.h>

#ifndef ARRAY_LEN
#define ARRAY_LEN(a) sizeof((a))/sizeof((a)[0])
#endif


/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_collapse_test   Testing of the HDRL reduce module
 */
/*----------------------------------------------------------------------------*/
typedef struct {
    double v1;
    double v2;
} pair;

/* cpl_test_abs that handles nan, rej is the rej variable of cpl_image_get to
 * test if NAN is marked as a bad pixel or -1 if not applicable */
#define hdrl_test_abs(a, b, tol, rej) \
    if (isnan(a)) { \
        cpl_test_eq(isnan(a), isnan(b)); \
        cpl_test(rej == -1 || rej != 0); \
    } \
    else { \
        cpl_test_abs(a, b, tol); \
    }


/* cpl_image_get that really returns the value regardless of bad or not */
static double
_hdrl_image_get(cpl_image * img, cpl_size x, cpl_size y, int * rej)
{
    double v = cpl_image_get(img, x, y, rej);
    if (*rej) {
        size_t nx = cpl_image_get_size_x(img);
        if (cpl_image_get_type(img) == CPL_TYPE_DOUBLE) {
            v = cpl_image_get_data_double(img)[(y - 1) * nx + (x - 1)];
        }
        else {
            v = cpl_image_get_data_float(img)[(y - 1) * nx + (x - 1)];
        }
    }
    return v;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief convert list to image format to list to vector format
 *
 * @param data  image
 * @param errs  errors
 * @param x     x coordinate to extract (FITS)
 * @param y     y coordinate to extract (FITS)
 * @param vl    imagelist to put new data list into
 * @param el    imagelist to put new error list into
 */
/* ---------------------------------------------------------------------------*/
static void
prep_l2v_input(cpl_imagelist * data,
               cpl_imagelist * errs,
               cpl_size x,
               cpl_size y,
               cpl_imagelist * vl,
               cpl_imagelist * el)
{
    int d;
    size_t n = cpl_imagelist_get_size(data);
    cpl_image * vimg = cpl_image_new(1, n, HDRL_TYPE_DATA);
    cpl_image * verr = cpl_image_new(1, n, HDRL_TYPE_ERROR);
    for (size_t i = 0; i < n; i++) {
        cpl_image * img = cpl_imagelist_get(data, i);
        cpl_image_set(vimg, 1, i + 1, cpl_image_get(img, x, y, &d));
        if (d) {
            cpl_image_reject(vimg, 1, i + 1);
        }
        img = cpl_imagelist_get(errs, i);
        cpl_image_set(verr, 1, i + 1, cpl_image_get(img, x, y, &d));
        if (d) {
            cpl_image_reject(verr, 1, i + 1);
        }
    }
    cpl_imagelist_set(vl, vimg, 0);
    cpl_imagelist_set(el, verr, 0);
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief test list to image and list to vector collapses
 *
 * @param data  data for list to image
 * @param errs  errors for list to image
 * @param l2im  list to image reduction method
 * @param l2vm  list to vector reduction method
 * @param x     x coordinate to extract for l2v (FITS)
 * @param y     y coordinate to extract for l2v (FITS)
 * @param v     value tolerance pair for expected data result at x, y
 * @param e     value tolerance pair for expected error result at x, y
 * @param expcontrib  expected contribution map
 */
/* ---------------------------------------------------------------------------*/
void test_l2i_and_l2v(cpl_imagelist * data,
                      cpl_imagelist * errs,
                      hdrl_collapse_imagelist_to_image_t * l2im,
                      hdrl_collapse_imagelist_to_vector_t * l2vm,
                      cpl_size x,
                      cpl_size y,
                      pair v,
                      pair e,
                      cpl_image * expcontrib)
{
    int d;
    /* test list to image */
    cpl_image * outimg, *outerr, *contrib;
    hdrl_imagelist_combine(data, errs, l2im, &outimg, &outerr, &contrib);

    hdrl_test_abs(_hdrl_image_get(outimg, x, y, &d), v.v1, v.v2, d);
    hdrl_test_abs(_hdrl_image_get(outerr, x, y, &d), e.v1, e.v2, d);
    cpl_test_image_abs(contrib, expcontrib, 0);
    cpl_image_delete(outimg);
    cpl_image_delete(outerr);
    cpl_image_delete(contrib);

    /* map the list to image input into a list to vector input that should give
     * the same result */
    cpl_imagelist * vl = cpl_imagelist_new();
    cpl_imagelist * el = cpl_imagelist_new();
    prep_l2v_input(data, errs, x, y, vl, el);

    /* test list to vector */
    cpl_vector * voutimg, *vouterr;
    cpl_array * acontrib;
    hdrl_collapse_imagelist_to_vector_call(l2vm, vl, el,
                                           &voutimg, &vouterr, &acontrib,
                                           NULL);
    cpl_imagelist_delete(vl);
    cpl_imagelist_delete(el);

	hdrl_test_abs(cpl_vector_get(voutimg, 0), v.v1, v.v2, -1);
	hdrl_test_abs(cpl_vector_get(vouterr, 0), e.v1, e.v2, -1);

	cpl_test_abs(cpl_array_get_int(acontrib, 0, NULL),
				 cpl_image_get(expcontrib, x, y, &d), 0);

    cpl_vector_delete(voutimg);
    cpl_vector_delete(vouterr);
    cpl_array_delete(acontrib);
    hdrl_collapse_imagelist_to_image_delete(l2im);
    hdrl_collapse_imagelist_to_vector_delete(l2vm);
}

void test_parameters(void)
{
    hdrl_parameter * hpar = hdrl_collapse_mean_parameter_create();
    cpl_test(hdrl_collapse_parameter_is_mean(hpar));
    hdrl_parameter_delete(hpar);
    hpar = hdrl_collapse_median_parameter_create();
    cpl_test(hdrl_collapse_parameter_is_median(hpar));
    hdrl_parameter_delete(hpar);
    hpar = hdrl_collapse_weighted_mean_parameter_create();
    cpl_test(hdrl_collapse_parameter_is_weighted_mean(hpar));
    hdrl_parameter_delete(hpar);
    hpar = hdrl_collapse_sigclip_parameter_create(3., 3., 5);
    cpl_test(hdrl_collapse_parameter_is_sigclip(hpar));
    hdrl_parameter_delete(hpar);
    hpar = hdrl_collapse_minmax_parameter_create(2., 2.);
    cpl_test(hdrl_collapse_parameter_is_minmax(hpar));
    hdrl_parameter_delete(hpar);
    hpar = hdrl_collapse_mode_parameter_create(100., 200., 1., HDRL_MODE_FIT, 100);
    cpl_test(hdrl_collapse_parameter_is_mode(hpar));
    hdrl_parameter_delete(hpar);

    cpl_test(hdrl_collapse_parameter_is_mean(HDRL_COLLAPSE_MEAN));
    cpl_test(!hdrl_collapse_parameter_is_mean(HDRL_COLLAPSE_MEDIAN));
    hdrl_parameter_delete((hdrl_parameter*)HDRL_COLLAPSE_MEAN);

    cpl_test(hdrl_collapse_parameter_is_median(HDRL_COLLAPSE_MEDIAN));
    hdrl_parameter_delete((hdrl_parameter*)HDRL_COLLAPSE_MEDIAN);

    cpl_test(hdrl_collapse_parameter_is_weighted_mean(
                                              HDRL_COLLAPSE_WEIGHTED_MEAN));
    hdrl_parameter_delete((hdrl_parameter*)HDRL_COLLAPSE_WEIGHTED_MEAN);
}


void test_parlist(void)
{
    /* parameter parsing smoketest */
    hdrl_parameter * hpar;
    hdrl_parameter * sigclip_def =
        hdrl_collapse_sigclip_parameter_create(1., 2., 5);
    hdrl_parameter * minmax_def =
        hdrl_collapse_minmax_parameter_create(1., 2.);
    hdrl_parameter * mode_def =
        hdrl_collapse_mode_parameter_create(10., 1., 0., HDRL_MODE_MEDIAN, 0);

    cpl_test_error(CPL_ERROR_NONE);
    cpl_parameterlist *parlist = hdrl_collapse_parameter_create_parlist(
                 "RECIPE", "collapse", "UNKNOWN", sigclip_def, minmax_def, mode_def) ;
     cpl_test_error(CPL_ERROR_NONE);

     cpl_parameterlist *parlist1 = hdrl_collapse_parameter_create_parlist(
                  "RECIPE", "collapse", "MEAN", sigclip_def, minmax_def, mode_def) ;
      cpl_test_error(CPL_ERROR_NONE);

      cpl_parameterlist *parlist2 = hdrl_collapse_parameter_create_parlist(
                "RECIPE", "collapse", "WEIGHTED_MEAN", sigclip_def, minmax_def, mode_def) ;
    cpl_test_error(CPL_ERROR_NONE);

    cpl_parameterlist *parlist3 = hdrl_collapse_parameter_create_parlist(
                "RECIPE", "collapse", "MEDIAN", sigclip_def, minmax_def, mode_def) ;
    cpl_test_error(CPL_ERROR_NONE);

    cpl_parameterlist *parlist4 = hdrl_collapse_parameter_create_parlist(
                "RECIPE", "collapse", "SIGCLIP", sigclip_def, minmax_def, mode_def) ;
    cpl_test_error(CPL_ERROR_NONE);

    cpl_parameterlist *parlist5 = hdrl_collapse_parameter_create_parlist(
                "RECIPE", "collapse", "MINMAX", sigclip_def, minmax_def, mode_def) ;
    cpl_test_error(CPL_ERROR_NONE);

    cpl_parameterlist *parlist6 = hdrl_collapse_parameter_create_parlist(
                "RECIPE", "collapse", "MODE", sigclip_def, minmax_def, mode_def) ;
    cpl_test_error(CPL_ERROR_NONE);


/*
    Befor adding the mode method:
    cpl_test_eq(cpl_parameterlist_get_size(parlist4), 6);
*/
    cpl_test_eq(cpl_parameterlist_get_size(parlist5), 6 + 5);

    hdrl_parameter_delete(sigclip_def);
    hdrl_parameter_delete(minmax_def);
    hdrl_parameter_delete(mode_def);


    /* UNKNOWN */
/*    hpar = hdrl_collapse_parameter_parse_parlist(parlist, "RECIPE.invalid");
    cpl_test_null(hpar);
    cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
    hpar = hdrl_collapse_parameter_parse_parlist(parlist, "RECIPE.collapse");
    cpl_test_error(CPL_ERROR_NONE);
    hdrl_parameter_delete(hpar);
*/
    /* MEAN */
    hpar = hdrl_collapse_parameter_parse_parlist(parlist1, "RECIPE.invalid");
    cpl_test_null(hpar);
    cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
    hpar = hdrl_collapse_parameter_parse_parlist(parlist1, "RECIPE.collapse");
    cpl_test_error(CPL_ERROR_NONE);
    hdrl_parameter_delete(hpar);


    /* WEIGHTED_MEAN */
    hpar = hdrl_collapse_parameter_parse_parlist(parlist2, "RECIPE.invalid");
    cpl_test_null(hpar);
    cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
    hpar = hdrl_collapse_parameter_parse_parlist(parlist2, "RECIPE.collapse");
    cpl_test_error(CPL_ERROR_NONE);
    hdrl_parameter_delete(hpar);


    /* MEDIAN */
    hpar = hdrl_collapse_parameter_parse_parlist(parlist3, "RECIPE.invalid");
    cpl_test_null(hpar);
    cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
    hpar = hdrl_collapse_parameter_parse_parlist(parlist3, "RECIPE.collapse");
    cpl_test_error(CPL_ERROR_NONE);
    hdrl_parameter_delete(hpar);


    /* SIGCLIP */
    hpar = hdrl_collapse_parameter_parse_parlist(parlist4, "RECIPE.invalid");
    cpl_test_null(hpar);
    cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
    hpar = hdrl_collapse_parameter_parse_parlist(parlist4, "RECIPE.collapse");
    cpl_test_error(CPL_ERROR_NONE);

    cpl_test(hdrl_collapse_parameter_is_sigclip(hpar));
    cpl_test(!hdrl_collapse_parameter_is_median(hpar));
    cpl_test_eq(hdrl_collapse_sigclip_parameter_get_kappa_high(hpar), 2.);
    cpl_test_eq(hdrl_collapse_sigclip_parameter_get_kappa_low(hpar), 1.);
    cpl_test_eq(hdrl_collapse_sigclip_parameter_get_niter(hpar), 5);
    hdrl_parameter_delete(hpar);


    /* MINMAX */
    hpar = hdrl_collapse_parameter_parse_parlist(parlist5, "RECIPE.invalid");
    cpl_test_null(hpar);
    cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
    hpar = hdrl_collapse_parameter_parse_parlist(parlist5, "RECIPE.collapse");
    cpl_test_error(CPL_ERROR_NONE);

    cpl_test(hdrl_collapse_parameter_is_minmax(hpar));
    cpl_test(!hdrl_collapse_parameter_is_median(hpar));
    cpl_test_eq(hdrl_collapse_minmax_parameter_get_nlow(hpar), 1.);
    cpl_test_eq(hdrl_collapse_minmax_parameter_get_nhigh(hpar), 2.);
    hdrl_parameter_delete(hpar);

    /* MODE */
    hpar = hdrl_collapse_parameter_parse_parlist(parlist6, "RECIPE.invalid");
    cpl_test_null(hpar);
    cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
    hpar = hdrl_collapse_parameter_parse_parlist(parlist6, "RECIPE.collapse");
    cpl_test_error(CPL_ERROR_NONE);

    cpl_test(hdrl_collapse_parameter_is_mode(hpar));
    cpl_test(!hdrl_collapse_parameter_is_median(hpar));
    cpl_test_eq(hdrl_collapse_mode_parameter_get_histo_min(hpar), 10.);
    cpl_test_eq(hdrl_collapse_mode_parameter_get_histo_max(hpar), 1.);
    cpl_test_eq(hdrl_collapse_mode_parameter_get_bin_size(hpar), 0);
    cpl_test_eq(hdrl_collapse_mode_parameter_get_method(hpar), HDRL_MODE_MEDIAN);
    cpl_test_eq(hdrl_collapse_mode_parameter_get_error_niter(hpar), 0);
    hdrl_parameter_delete(hpar);


    /* Clean up */
    cpl_parameterlist_delete(parlist );
    cpl_parameterlist_delete(parlist1);
    cpl_parameterlist_delete(parlist2);
    cpl_parameterlist_delete(parlist3);
    cpl_parameterlist_delete(parlist4);
    cpl_parameterlist_delete(parlist5);
    cpl_parameterlist_delete(parlist6);


    hpar = hdrl_collapse_mean_parameter_create();
    cpl_test(hdrl_collapse_parameter_is_mean(hpar));
    hdrl_parameter_delete(hpar);
    hpar = hdrl_collapse_median_parameter_create();
    cpl_test(hdrl_collapse_parameter_is_median(hpar));
    hdrl_parameter_delete(hpar);
    hpar = hdrl_collapse_weighted_mean_parameter_create();
    cpl_test(hdrl_collapse_parameter_is_weighted_mean(hpar));
    hdrl_parameter_delete(hpar);
}


void test_eout(void)
{
    cpl_size n = 40;


	/* Method with vector mean */
    hdrl_collapse_imagelist_to_vector_t * vMethod1;
    vMethod1 = hdrl_collapse_imagelist_to_vector_mean();

    void *eout1 = hdrl_collapse_imagelist_to_vector_create_eout(vMethod1, n);

    void *eout2 = NULL;
    hdrl_collapse_imagelist_to_vector_move_eout(vMethod1, eout1, eout2, n);

    hdrl_collapse_imagelist_to_vector_delete_eout(vMethod1, eout2);
	hdrl_collapse_imagelist_to_vector_delete_eout(vMethod1, eout1);
    hdrl_collapse_imagelist_to_vector_delete(vMethod1);


	/* Method with vector minmax */
	hdrl_collapse_imagelist_to_vector_t *vMethod2;
	vMethod2 = hdrl_collapse_imagelist_to_vector_minmax(3., 3.);

    void *eout3 = hdrl_collapse_imagelist_to_vector_create_eout(vMethod2, n);

    hdrl_collapse_imagelist_to_vector_delete_eout(vMethod2, eout3);
	hdrl_collapse_imagelist_to_vector_delete(vMethod2);


	/* Method with vector sigclip */
	hdrl_collapse_imagelist_to_vector_t *vMethod3;
	vMethod3 = hdrl_collapse_imagelist_to_vector_sigclip(3., 3., 3);

    void *eout4 = hdrl_collapse_imagelist_to_vector_create_eout(vMethod3, n);
    void *eout5 = hdrl_collapse_imagelist_to_vector_create_eout(vMethod3, n);

    hdrl_collapse_imagelist_to_vector_move_eout(vMethod3, NULL, eout5, n);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    hdrl_collapse_imagelist_to_vector_move_eout(vMethod3, eout4, NULL, n);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    hdrl_collapse_imagelist_to_vector_move_eout(vMethod3, eout4, eout4, n);
    cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);
    hdrl_collapse_imagelist_to_vector_move_eout(vMethod3, eout4, eout5, n);
    cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);
    hdrl_collapse_imagelist_to_vector_move_eout(vMethod3, eout5, eout4, n);
    cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);
    hdrl_collapse_imagelist_to_vector_move_eout(vMethod3, eout5, eout5, n);
    cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);

    hdrl_collapse_imagelist_to_vector_delete_eout(vMethod3, eout4);
	hdrl_collapse_imagelist_to_vector_delete_eout(vMethod3, eout5);
	hdrl_collapse_imagelist_to_vector_delete(vMethod3);


	/* Method with image */
    hdrl_collapse_imagelist_to_image_t *vMethod4;
    vMethod4 = hdrl_collapse_imagelist_to_image_sigclip(3., 3., 3);

    cpl_image *img = cpl_image_new(10, 10, CPL_TYPE_DOUBLE);
    void *eout6 = hdrl_collapse_imagelist_to_image_create_eout(vMethod4, img);
	cpl_image_delete(img);

	hdrl_collapse_imagelist_to_image_delete_eout(vMethod4, eout6);
	hdrl_collapse_imagelist_to_image_delete(vMethod4);
}


void test_results(void)
{

#define TST_FREE \
        cpl_image_delete(outimg); outimg = NULL; \
        cpl_image_delete(outerr); outerr = NULL; \
        cpl_image_delete(contrib); contrib = NULL; \
        cpl_vector_delete(voutimg); voutimg = NULL; \
        cpl_vector_delete(vouterr); vouterr = NULL; \
        cpl_array_delete(acontrib); acontrib = NULL; \
        hdrl_collapse_imagelist_to_image_delete(method);

    /* create data, value 5., error +-1 */
    cpl_imagelist * data = cpl_imagelist_new();
    cpl_imagelist * errs = cpl_imagelist_new();
    cpl_size nz = 5;
    cpl_size nx = 40;
    cpl_size ny = 37;
    cpl_image * img = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
    cpl_image * err = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
    cpl_image_add_scalar(img, 5.);
    cpl_image_add_scalar(err, 2.);

    /* create expected results (err / sqrt(nz) for mean) */
    cpl_image * expect_err = cpl_image_duplicate(err);
    cpl_image_divide_scalar(expect_err, sqrt(nz));
    cpl_image * expect_contrib = cpl_image_new(nx, ny, CPL_TYPE_INT);
    cpl_image_add_scalar(expect_contrib, nz);
    for (cpl_size i = 0; i < nz; i++) {
        cpl_imagelist_set(data, cpl_image_duplicate(img),
                          cpl_imagelist_get_size(data));
        cpl_imagelist_set(errs, cpl_image_duplicate(err),
                          cpl_imagelist_get_size(errs));
    }
    cpl_image * outimg, * outerr, * contrib;
    
    cpl_vector * expect_vimg = cpl_vector_new(nz);
    cpl_vector * expect_verr = cpl_vector_new(nz);
    cpl_array * expect_acontrib = cpl_array_new(nz, CPL_TYPE_INT);
    cpl_vector * voutimg, * vouterr;
    cpl_array * acontrib;
    cpl_vector_fill(expect_vimg, 5.);
    cpl_vector_fill(expect_verr, 2. / sqrt(nx * ny));
    cpl_array_fill_window_int(expect_acontrib, 0, nz, nx * ny);

    /* test reductions on uniform error cases */
    {
        /* mean */
        hdrl_collapse_imagelist_to_image_t * method =
            hdrl_collapse_imagelist_to_image_mean();
        hdrl_collapse_imagelist_to_image_call(method, data, errs,
                                              &outimg, &outerr, &contrib,
                                              NULL);

        cpl_test_image_abs(outimg, img, HDRL_EPS_DATA);
        cpl_test_image_abs(outerr, expect_err, HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);

        hdrl_collapse_imagelist_to_vector_t * vmethod =
            hdrl_collapse_imagelist_to_vector_mean();
        hdrl_collapse_imagelist_to_vector_call(vmethod, data, errs,
                                               &voutimg, &vouterr, &acontrib,
                                               NULL);
        cpl_test_vector_abs(voutimg, expect_vimg, HDRL_EPS_DATA);
        cpl_test_vector_abs(vouterr, expect_verr, HDRL_EPS_ERROR);
        cpl_test_array_abs(acontrib, expect_acontrib, 0);
        hdrl_collapse_imagelist_to_vector_delete(vmethod);
        TST_FREE;

        /* sigclip */
        method = hdrl_collapse_imagelist_to_image_sigclip(3., 3., 3);
        hdrl_collapse_imagelist_to_image_call(method, data, errs,
                                              &outimg, &outerr, &contrib,
                                              NULL);

        cpl_test_image_abs(outimg, img, HDRL_EPS_DATA);
        cpl_test_image_abs(outerr, expect_err, HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);


        vmethod = hdrl_collapse_imagelist_to_vector_sigclip(3., 3., 3);
        hdrl_collapse_imagelist_to_vector_call(vmethod, data, errs,
                                               &voutimg, &vouterr, &acontrib,
                                               NULL);
        cpl_test_vector_abs(voutimg, expect_vimg, HDRL_EPS_DATA);
        cpl_test_vector_abs(vouterr, expect_verr, HDRL_EPS_ERROR);
        cpl_test_array_abs(acontrib, expect_acontrib, 0);
        hdrl_collapse_imagelist_to_vector_delete(vmethod);
        TST_FREE;

        /* minmax */
        method = hdrl_collapse_imagelist_to_image_minmax(0., 0.);
        hdrl_collapse_imagelist_to_image_call(method, data, errs,
                                              &outimg, &outerr, &contrib,
                                              NULL);

        cpl_test_image_abs(outimg, img, HDRL_EPS_DATA);
        cpl_test_image_abs(outerr, expect_err, HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);


        vmethod = hdrl_collapse_imagelist_to_vector_minmax(0., 0.);
        hdrl_collapse_imagelist_to_vector_call(vmethod, data, errs,
                                               &voutimg, &vouterr, &acontrib,
                                               NULL);
        cpl_test_vector_abs(voutimg, expect_vimg, HDRL_EPS_DATA);
        cpl_test_vector_abs(vouterr, expect_verr, HDRL_EPS_ERROR);
        cpl_test_array_abs(acontrib, expect_acontrib, 0);
        hdrl_collapse_imagelist_to_vector_delete(vmethod);
        TST_FREE;

        /* weighted mean */
        method = hdrl_collapse_imagelist_to_image_weighted_mean();
        hdrl_collapse_imagelist_to_image_call(method, data, errs,
                                              &outimg, &outerr, &contrib,
                                              NULL);

        cpl_test_image_abs(outimg, img, HDRL_EPS_DATA);
        cpl_test_image_abs(outerr, expect_err, HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);
        TST_FREE;

        /* median */
        cpl_image_multiply_scalar(expect_err, sqrt(CPL_MATH_PI_2));
        method = hdrl_collapse_imagelist_to_image_median();
        hdrl_collapse_imagelist_to_image_call(method, data, errs,
                                              &outimg, &outerr, &contrib,
                                              NULL);

        cpl_test_image_abs(outimg, img, HDRL_EPS_DATA);
        cpl_test_image_abs(outerr, expect_err, HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);

        cpl_vector_multiply_scalar(expect_verr, sqrt(CPL_MATH_PI_2));
        vmethod = hdrl_collapse_imagelist_to_vector_median();
        hdrl_collapse_imagelist_to_vector_call(vmethod, data, errs,
                                               &voutimg, &vouterr, &acontrib,
                                               NULL);
        cpl_test_vector_abs(voutimg, expect_vimg, HDRL_EPS_DATA);
        cpl_test_vector_abs(vouterr, expect_verr, HDRL_EPS_ERROR);
        cpl_test_array_abs(acontrib, expect_acontrib, 0);
        hdrl_collapse_imagelist_to_vector_delete(vmethod);
        TST_FREE;
    }
    /* test non uniform error cases */
    {
        double v[] = {1, 2, 1, 3, 2};
        double e[] = {0.5, 0.7, 0.1, 1.0, 0.01};
        for (cpl_size i = 0; i < nz; i++) {
            cpl_image * tmp = cpl_imagelist_get(data, i);
            cpl_image_set(tmp, 1, 1, v[i]);
            tmp = cpl_imagelist_get(errs, i);
            cpl_image_set(tmp, 1, 1, e[i]);
        }
        hdrl_collapse_imagelist_to_image_t * method =
            hdrl_collapse_imagelist_to_image_mean();
        hdrl_collapse_imagelist_to_vector_t * vmethod =
            hdrl_collapse_imagelist_to_vector_mean();
        test_l2i_and_l2v(data, errs, method, vmethod, 1, 1,
                         (pair){1.8, HDRL_EPS_DATA},
                         (pair){0.26458269028793246, HDRL_EPS_ERROR},
                         expect_contrib);

        method = hdrl_collapse_imagelist_to_image_sigclip(3., 3., 3);
        vmethod = hdrl_collapse_imagelist_to_vector_sigclip(3., 3., 3);
        test_l2i_and_l2v(data, errs, method, vmethod, 1, 1,
                         (pair){1.8, HDRL_EPS_DATA},
                         (pair){0.26458269028793246, HDRL_EPS_ERROR},
                         expect_contrib);

        method = hdrl_collapse_imagelist_to_image_minmax(1, 1);
        vmethod = hdrl_collapse_imagelist_to_vector_minmax(1, 1);
        cpl_image * expect_contrib_minmax =
                        cpl_image_subtract_scalar_create(expect_contrib, 2);
        test_l2i_and_l2v(data, errs, method, vmethod, 1, 1,
                         (pair){5. / 3., HDRL_EPS_DATA},
                         (pair){sqrt(0.1 * 0.1 + 0.7 * 0.7 + 0.01 * 0.01) / 3.,
                                HDRL_EPS_ERROR},
                         expect_contrib_minmax);
        cpl_image_delete(expect_contrib_minmax);

        method = hdrl_collapse_imagelist_to_image_weighted_mean();
        vmethod = hdrl_collapse_imagelist_to_vector_weighted_mean();
        test_l2i_and_l2v(data, errs, method, vmethod, 1, 1,
                         (pair){1.9898090843925733, HDRL_EPS_ERROR},
                         (pair){0.0099469054598625289, HDRL_EPS_ERROR},
                         expect_contrib);
    }
    /* test non uniform error cases with rejected values */
    {
        double v[] = {1,   2,   1,   3,   2};
        double e[] = {0.5, 0.7, 0.1, 1.0, 0.01};
        for (cpl_size i = 0; i < nz; i++) {
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
        hdrl_collapse_imagelist_to_vector_t * vmethod =
            hdrl_collapse_imagelist_to_vector_mean();
        test_l2i_and_l2v(data, errs, method, vmethod, 1, 1,
                         (pair){1.5, HDRL_EPS_DATA},
                         (pair){0.21652078422174625, HDRL_EPS_ERROR},
                         expect_contrib);

        method = hdrl_collapse_imagelist_to_image_sigclip(3., 3., 3);
        vmethod = hdrl_collapse_imagelist_to_vector_sigclip(3., 3., 3);
        test_l2i_and_l2v(data, errs, method, vmethod, 1, 1,
                         (pair){1.5, HDRL_EPS_DATA},
                         (pair){0.21652078422174625, HDRL_EPS_ERROR},
                         expect_contrib);

        method = hdrl_collapse_imagelist_to_image_minmax(1, 1);
        vmethod = hdrl_collapse_imagelist_to_vector_minmax(1, 1);
        cpl_image * expect_contrib_minmax =
                        cpl_image_subtract_scalar_create(expect_contrib, 2);
        test_l2i_and_l2v(data, errs, method, vmethod, 1, 1,
                         (pair){3. / 2., HDRL_EPS_DATA},
                         (pair){sqrt(0.1 * 0.1 + 0.01 * 0.01) / 2.,
                                HDRL_EPS_ERROR},
                         expect_contrib_minmax);
        cpl_image_delete(expect_contrib_minmax);

        method = hdrl_collapse_imagelist_to_image_weighted_mean();
        vmethod = hdrl_collapse_imagelist_to_vector_weighted_mean();
        test_l2i_and_l2v(data, errs, method, vmethod, 1, 1,
                         (pair){1.9897091252756485, HDRL_EPS_ERROR},
                         (pair){0.0099473975744101273, HDRL_EPS_ERROR},
                         expect_contrib);
    }
    /* test all values rejected in line, list to image */
    {
        double v[] = {1, 2, 1, 3, 2};
        double e[] = {0.5, 0.7, 0.1, 1.0, 0.01};
        for (cpl_size i = 0; i < nz; i++) {
            cpl_image * tmp = cpl_imagelist_get(data, i);
            cpl_image_set(tmp, 1, 1, v[i]);
            cpl_image_reject(tmp, 1, 1);
            tmp = cpl_imagelist_get(errs, i);
            cpl_image_set(tmp, 1, 1, e[i]);
            cpl_image_reject(tmp, 1, 1);
        }
        cpl_image_delete(expect_contrib);
        expect_contrib = cpl_image_new_from_accepted(data);

        hdrl_collapse_imagelist_to_image_t * img_meth[] = {
            hdrl_collapse_imagelist_to_image_mean(),
            hdrl_collapse_imagelist_to_image_sigclip(3., 3., 3),
            hdrl_collapse_imagelist_to_image_weighted_mean(),
            hdrl_collapse_imagelist_to_image_median(),
        };
        hdrl_collapse_imagelist_to_vector_t * vec_meth[] = {
            hdrl_collapse_imagelist_to_vector_mean(),
            hdrl_collapse_imagelist_to_vector_sigclip(3., 3., 3),
            hdrl_collapse_imagelist_to_vector_weighted_mean(),
            hdrl_collapse_imagelist_to_vector_median(),
        };

        for (size_t i = 0; i < ARRAY_LEN(img_meth); i++) {
            test_l2i_and_l2v(data, errs, img_meth[i], vec_meth[i], 1, 1,
                             (pair){NAN, HDRL_EPS_DATA},
                             (pair){NAN, HDRL_EPS_ERROR},
                             expect_contrib);
        }

        hdrl_collapse_imagelist_to_image_t * method =
            hdrl_collapse_imagelist_to_image_minmax(1., 1.);
        hdrl_collapse_imagelist_to_vector_t * vmethod =
            hdrl_collapse_imagelist_to_vector_minmax(1., 1.);
        cpl_image * expect_contrib_minmax =
            cpl_image_subtract_scalar_create(expect_contrib, 2);
        cpl_image_set(expect_contrib_minmax, 1, 1, 0);
        test_l2i_and_l2v(data, errs, method, vmethod, 1, 1,
                         (pair){NAN, HDRL_EPS_DATA},
                         (pair){NAN, HDRL_EPS_ERROR},
                         expect_contrib_minmax);
        cpl_image_delete(expect_contrib_minmax);
    }
    /* test all values rejected, list to image */
    {
        for (cpl_size i = 0; i < nz; i++) {
            cpl_image * tmp = cpl_imagelist_get(data, i);
            cpl_image_accept_all(tmp);
            cpl_mask_not(cpl_image_get_bpm(tmp));
        }
        cpl_image_delete(expect_contrib);
        expect_contrib = cpl_image_new_from_accepted(data);

        hdrl_collapse_imagelist_to_image_t * img_meth[] = {
            hdrl_collapse_imagelist_to_image_mean(),
            hdrl_collapse_imagelist_to_image_sigclip(3., 3., 3),
            hdrl_collapse_imagelist_to_image_weighted_mean(),
            hdrl_collapse_imagelist_to_image_median(),
            hdrl_collapse_imagelist_to_image_minmax(1., 1.),
        };
        hdrl_collapse_imagelist_to_vector_t * vec_meth[] = {
            hdrl_collapse_imagelist_to_vector_mean(),
            hdrl_collapse_imagelist_to_vector_sigclip(3., 3., 3),
            hdrl_collapse_imagelist_to_vector_weighted_mean(),
            hdrl_collapse_imagelist_to_vector_median(),
            hdrl_collapse_imagelist_to_vector_minmax(1., 1.),
        };

        for (size_t i = 0; i < ARRAY_LEN(img_meth); i++) {
            cpl_image * outimg_loc, *outerr_loc, *contrib_loc;
            hdrl_imagelist_combine(data, errs, img_meth[i], &outimg_loc, &outerr_loc, &contrib_loc);
            /* unlike cpl, don't emit error */
            cpl_test_error(CPL_ERROR_NONE);

            cpl_test_image_abs(contrib_loc, expect_contrib, 0);
            cpl_test_eq(cpl_image_count_rejected(outimg_loc), nx *ny);
            cpl_test_eq(cpl_image_count_rejected(outerr_loc), nx *ny);
            cpl_image_delete(outimg_loc);
            cpl_image_delete(outerr_loc);
            cpl_image_delete(contrib_loc);

            /* also check vector variant */
            test_l2i_and_l2v(data, errs, img_meth[i], vec_meth[i], 1, 1,
                             (pair){NAN, HDRL_EPS_DATA},
                             (pair){NAN, HDRL_EPS_ERROR},
                             expect_contrib);
            cpl_test_error(CPL_ERROR_NONE);
        }
    }
    /* test median error propagation with rejects, only makes sense on uniform
     * errors as the scaling relies on gaussian errors */
    {
        double v[] = {1, 2, 1, 3, 2};
        double e[] = {1., 1., 1., 1., 1.};
        int d;
        for (cpl_size i = 0; i < nz; i++) {
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

        cpl_imagelist * vl = cpl_imagelist_new();
        cpl_imagelist * el = cpl_imagelist_new();
        prep_l2v_input(data, errs, 1, 1, vl, el);
        hdrl_collapse_imagelist_to_vector_t * vmethod =
            hdrl_collapse_imagelist_to_vector_median();
        hdrl_collapse_imagelist_to_vector_call(vmethod, vl, el,
                                               &voutimg, &vouterr, &acontrib,
                                               NULL);

        cpl_test_abs(cpl_vector_get(voutimg, 0), 1.5, HDRL_EPS_DATA);
        cpl_test_abs(cpl_vector_get(vouterr, 0), 1. / sqrt(2.), HDRL_EPS_ERROR);

        cpl_test_abs(cpl_array_get_int(acontrib, 0, NULL), 2, 0);
        cpl_vector_delete(voutimg);
        cpl_vector_delete(vouterr);
        cpl_array_delete(acontrib);
        cpl_imagelist_empty(vl);
        cpl_imagelist_empty(el);

        prep_l2v_input(data, errs, 2, 2, vl, el);
        hdrl_collapse_imagelist_to_vector_call(vmethod, vl, el,
                                               &voutimg, &vouterr, &acontrib,
                                               NULL);

        cpl_test_abs(cpl_vector_get(voutimg, 0), 2., HDRL_EPS_DATA);
        cpl_test_abs(cpl_vector_get(vouterr, 0),
        		1. / sqrt(nz) * sqrt(CPL_MATH_PI_2), HDRL_EPS_ERROR);

        cpl_test_abs(cpl_array_get_int(acontrib, 0, NULL), 5, 0);
        cpl_imagelist_delete(vl);
        cpl_imagelist_delete(el);
        hdrl_collapse_imagelist_to_vector_delete(vmethod);

        TST_FREE;
    }

    cpl_imagelist_delete(data);
    cpl_imagelist_delete(errs);
    cpl_image_delete(expect_err);
    cpl_image_delete(expect_contrib);
    cpl_image_delete(img);
    cpl_image_delete(err);
    cpl_vector_delete(expect_vimg);
    cpl_vector_delete(expect_verr);
    cpl_array_delete(expect_acontrib);

#undef TST_FREE
}

void test_results_mode(void)
{
#define TST_FREE_MODE \
        cpl_image_delete(outimg); outimg = NULL; \
        cpl_image_delete(outerr); outerr = NULL; \
        cpl_image_delete(contrib); contrib = NULL; \
        cpl_vector_delete(voutimg); voutimg = NULL; \
        cpl_vector_delete(vouterr); vouterr = NULL; \
        cpl_array_delete(acontrib); acontrib = NULL; \
        hdrl_collapse_imagelist_to_image_delete(method);

    /* create data, value 5., error +-1 */
    cpl_imagelist * data = cpl_imagelist_new();
    cpl_imagelist * errs = cpl_imagelist_new();
    cpl_size nz = 10;
    cpl_size nx = 2;
    cpl_size ny = 2;
    cpl_image * img = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
    cpl_image * err = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
    cpl_image_add_scalar(img, 3.5);
    cpl_image_add_scalar(err, 1.5);

    /* create expected results (err / sqrt(nz) for mean) */
    cpl_image * expect_err = cpl_image_duplicate(err);
    cpl_image_divide_scalar(expect_err, sqrt(nz));
    cpl_image * expect_contrib = cpl_image_new(nx, ny, CPL_TYPE_INT);
    cpl_image_add_scalar(expect_contrib, nz);
    for (cpl_size i = 0; i < nz; i++) {
        cpl_imagelist_set(data, cpl_image_duplicate(img),
                          cpl_imagelist_get_size(data));
        cpl_imagelist_set(errs, cpl_image_duplicate(err),
                          cpl_imagelist_get_size(errs));
    }
    cpl_image * outimg, * outerr, * contrib;

    cpl_vector * expect_vimg = cpl_vector_new(nz);
    cpl_vector * expect_verr = cpl_vector_new(nz);
    cpl_array * expect_acontrib = cpl_array_new(nz, CPL_TYPE_INT);
    cpl_vector * voutimg = NULL, * vouterr = NULL;
    cpl_array * acontrib = NULL;
    cpl_vector_fill(expect_vimg, 3.5);
    cpl_vector_fill(expect_verr, 1.5 / sqrt(nx * ny));
    cpl_array_fill_window_int(expect_acontrib, 0, nz, nx * ny);

    /* test reductions on uniform error cases */
    {
        /* mode MEDIAN */

        cpl_image * mode_expect_err = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
        cpl_vector * mode_expect_verr = cpl_vector_new(nz);
        cpl_vector_fill(mode_expect_verr, 0.);

        hdrl_collapse_imagelist_to_image_t * method =
            hdrl_collapse_imagelist_to_image_mode(5, 5, 0, HDRL_MODE_MEDIAN, 20);
        hdrl_collapse_imagelist_to_image_call(method, data, errs,
                                              &outimg, &outerr, &contrib,
                                              NULL);

        cpl_test_image_abs(outimg, img, HDRL_EPS_DATA);
        cpl_test_image_abs(outerr, mode_expect_err, HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);
        TST_FREE_MODE;

        method = hdrl_collapse_imagelist_to_image_mode(3, 4, 1, HDRL_MODE_MEDIAN, 20);
        hdrl_collapse_imagelist_to_image_call(method, data, errs,
                                              &outimg, &outerr, &contrib,
                                              NULL);

        cpl_test_image_abs(outimg, img, HDRL_EPS_DATA);
        cpl_test_image_abs(outerr, mode_expect_err, HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);
        cpl_image_delete(mode_expect_err);


        hdrl_collapse_imagelist_to_vector_t * vmethod =
            hdrl_collapse_imagelist_to_vector_mode(3., 4. ,1., HDRL_MODE_MEDIAN, 20);
        hdrl_collapse_imagelist_to_vector_call(vmethod, data, errs,
                                               &voutimg, &vouterr, &acontrib,
                                               NULL);
        cpl_test_vector_abs(voutimg, expect_vimg, HDRL_EPS_DATA);
        cpl_test_vector_abs(vouterr, mode_expect_verr, HDRL_EPS_ERROR);
        cpl_test_array_abs(acontrib, expect_acontrib, 0);
        hdrl_collapse_imagelist_to_vector_delete(vmethod);
        cpl_vector_delete(mode_expect_verr);
        TST_FREE_MODE;

        /* mode WEIGHTED */

        mode_expect_err = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
        mode_expect_verr = cpl_vector_new(nz);
        cpl_vector_fill(mode_expect_verr, 0.);
        method = hdrl_collapse_imagelist_to_image_mode(5, 5, 0, HDRL_MODE_WEIGHTED, 20);
        hdrl_collapse_imagelist_to_image_call(method, data, errs,
                                              &outimg, &outerr, &contrib,
                                              NULL);

        cpl_test_image_abs(outimg, img, HDRL_EPS_DATA * 2);
        cpl_test_image_abs(outerr, mode_expect_err, HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);
        TST_FREE_MODE;

        method = hdrl_collapse_imagelist_to_image_mode(3., 4., 1, HDRL_MODE_WEIGHTED, 20);
        hdrl_collapse_imagelist_to_image_call(method, data, errs,
                                              &outimg, &outerr, &contrib,
                                              NULL);

        //cpl_image_add_scalar(img, 0.5);
        cpl_test_image_abs(outimg, img, HDRL_EPS_DATA);
        //cpl_image_subtract_scalar(img, 0.5);

        cpl_test_image_abs(outerr, mode_expect_err, HDRL_EPS_ERROR);
        cpl_test_image_abs(contrib, expect_contrib, 0);
        cpl_image_delete(mode_expect_err);


        vmethod = hdrl_collapse_imagelist_to_vector_mode(3., 4. ,1., HDRL_MODE_WEIGHTED, 20);
        hdrl_collapse_imagelist_to_vector_call(vmethod, data, errs,
                                               &voutimg, &vouterr, &acontrib,
                                               NULL);
        //cpl_vector_add_scalar(expect_vimg, 0.5);
        cpl_test_vector_abs(voutimg, expect_vimg, HDRL_EPS_DATA);
        //cpl_vector_subtract_scalar(expect_vimg, 0.5);
        cpl_test_vector_abs(vouterr, mode_expect_verr, HDRL_EPS_ERROR);
        cpl_test_array_abs(acontrib, expect_acontrib, 0);
        hdrl_collapse_imagelist_to_vector_delete(vmethod);
        cpl_vector_delete(mode_expect_verr);
        TST_FREE_MODE;

        /* mode FIT */

        method = hdrl_collapse_imagelist_to_image_mode(5, 5, 0, HDRL_MODE_FIT, 20);
        hdrl_collapse_imagelist_to_image_call(method, data, errs,
                                              &outimg, &outerr, &contrib,
                                              NULL);

        /* You can not make a fit with one bin only */
        cpl_test_eq(cpl_image_count_rejected(outimg), nx * ny); /* All bad */
        cpl_test_eq(cpl_image_count_rejected(outerr), nx * ny); /* All bad */
        cpl_test_eq(cpl_image_get_sqflux(contrib), 0);  /* All Zero */
        TST_FREE_MODE;

        method = hdrl_collapse_imagelist_to_image_mode(2, 5, 1, HDRL_MODE_FIT, 20);
        hdrl_collapse_imagelist_to_image_call(method, data, errs,
                                              &outimg, &outerr, &contrib,
                                              NULL);

        cpl_test_eq(cpl_image_count_rejected(outimg), 0); /* All good */
        cpl_test_eq(cpl_image_count_rejected(outerr), 0); /* All good */
        cpl_test_eq(cpl_image_get_sqflux(contrib), 400.);  /* All 4 */

        vmethod = hdrl_collapse_imagelist_to_vector_mode(2., 5. ,1., HDRL_MODE_FIT, 20);
        hdrl_collapse_imagelist_to_vector_call(vmethod, data, errs,
                                               &voutimg, &vouterr, &acontrib,
                                               NULL);

        /* You can not make a fit with one bin only */
        cpl_test_eq(cpl_image_count_rejected(outimg), 0); /* All good */
        cpl_test_eq(cpl_image_count_rejected(outerr), 0); /* All good */
        cpl_test_eq(cpl_image_get_sqflux(contrib), 400.);  /* 4 * 10*10 */
        hdrl_collapse_imagelist_to_vector_delete(vmethod);
        TST_FREE_MODE;

    }
    /* test non uniform error cases */
    {
      double v[] = {1.5, 2.5, 2.5, 3.5, 3.5, 3.5, 3.5, 4.5, 4.5, 5.5};
      double e[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; /* Not used */
      for (cpl_size i = 0; i < nz; i++) {
	  cpl_image * tmp = cpl_imagelist_get(data, i);
	  cpl_image_set(tmp, 1, 1, v[i]);
	  tmp = cpl_imagelist_get(errs, i);
	  cpl_image_set(tmp, 1, 1, e[i]);
      }

      /* MODE MEDIAN */

      /* histo_max, histo_min and binsize is determined internally by the
       * algorithm */
      hdrl_collapse_imagelist_to_image_t * method =
	  hdrl_collapse_imagelist_to_image_mode(0, 0, 0, HDRL_MODE_MEDIAN, 0);
      hdrl_collapse_imagelist_to_vector_t * vmethod =
	  hdrl_collapse_imagelist_to_vector_mode(0, 0, 0, HDRL_MODE_MEDIAN, 0);

      /* The code that deives automatically the bines gives:
	Histogram bin size: 4.80337 min: -0.901685 max: 8.70506 number of bins: 2
        Therefore the final vector in the innermost bin is:
        [1.5, 2.5, 2.5, 3.5, 3.5, 3.5, 3.5,]
        The error is the standard deviation of the final vector (using cpl) */


      test_l2i_and_l2v(data, errs, method, vmethod, 1, 1,
		       (pair){3.5, HDRL_EPS_DATA},
		       (pair){0.7867957924694431, HDRL_EPS_ERROR},
		       expect_contrib);

      /* histo_max, histo_min and binsize is given by the user */
      method = hdrl_collapse_imagelist_to_image_mode(3, 4, 1, HDRL_MODE_MEDIAN, 0);
      vmethod = hdrl_collapse_imagelist_to_vector_mode(3, 4, 1, HDRL_MODE_MEDIAN, 0);

      test_l2i_and_l2v(data, errs, method, vmethod, 1, 1,
		       (pair){3.5, HDRL_EPS_DATA},
		       (pair){0., HDRL_EPS_ERROR},
		       expect_contrib);

      /* MODE WEIGHTED */

      /* histo_max, and histo_min are determined internally by the
       * algorithm */
      method =
	  hdrl_collapse_imagelist_to_image_mode(0, 0, 1, HDRL_MODE_WEIGHTED, 0);
      vmethod =
	  hdrl_collapse_imagelist_to_vector_mode(0, 0, 1, HDRL_MODE_WEIGHTED, 0);

      /* The code that deives automatically the bines gives:
	Histogram bin size: 4.80337 min: -0.901685 max: 8.70506 number of bins: 2
     Therefore the final vector in the innermost bin is:
     [1.5, 2.5, 2.5, 3.5, 3.5, 3.5, 3.5,]
     The error is the standard deviation of the final vector (using cpl) */

      test_l2i_and_l2v(data, errs, method, vmethod, 1, 1,
		       (pair){3.5, HDRL_EPS_DATA},
		       (pair){0.4330127018922193, HDRL_EPS_ERROR},
		       expect_contrib);



       /* histo_max, histo_min and binsize is given by the user */
      method = hdrl_collapse_imagelist_to_image_mode(3, 4, 1, HDRL_MODE_WEIGHTED, 0);
      vmethod = hdrl_collapse_imagelist_to_vector_mode(3, 4, 1, HDRL_MODE_WEIGHTED, 0);

      test_l2i_and_l2v(data, errs, method, vmethod, 1, 1,
                       (pair){3.5, HDRL_EPS_DATA},
                       (pair){0.1767766952966369, HDRL_EPS_ERROR},
                       expect_contrib);


      /* MODE FIT */

        /* histo_max, histo_min and binsize is given by the user */
       method = hdrl_collapse_imagelist_to_image_mode(2, 5, 1, HDRL_MODE_FIT, 0);
       vmethod = hdrl_collapse_imagelist_to_vector_mode(2, 5, 1, HDRL_MODE_FIT, 0);

       test_l2i_and_l2v(data, errs, method, vmethod, 1, 1,
                        (pair){3.6, HDRL_EPS_DATA * 10},
                        (pair){0.2683281572999749, HDRL_EPS_ERROR * 10.},
                        expect_contrib);

    }


//    /* test non uniform error cases with rejected values */
//    if (0){
//        double v[] = {1,   2,   1,   3,   2};
//        double e[] = {0.5, 0.7, 0.1, 1.0, 0.01};
//        for (cpl_size i = 0; i < nz; i++) {
//            cpl_image * tmp = cpl_imagelist_get(data, i);
//            cpl_image_set(tmp, 1, 1, v[i]);
//            if (i == 3) {
//                cpl_image_reject(tmp, 1, 1);
//            }
//            tmp = cpl_imagelist_get(errs, i);
//            cpl_image_set(tmp, 1, 1, e[i]);
//            if (i == 3) {
//                cpl_image_reject(tmp, 1, 1);
//            }
//        }
//        cpl_image_delete(expect_contrib);
//        expect_contrib = cpl_image_new_from_accepted(data);
//
//        hdrl_collapse_imagelist_to_image_t * method =
//            hdrl_collapse_imagelist_to_image_mean();
//        hdrl_collapse_imagelist_to_vector_t * vmethod =
//            hdrl_collapse_imagelist_to_vector_mean();
//        test_l2i_and_l2v(data, errs, method, vmethod, 1, 1,
//                         (pair){1.5, HDRL_EPS_DATA},
//                         (pair){0.21652078422174625, HDRL_EPS_ERROR},
//                         expect_contrib);
//
//        method = hdrl_collapse_imagelist_to_image_sigclip(3., 3., 3);
//        vmethod = hdrl_collapse_imagelist_to_vector_sigclip(3., 3., 3);
//        test_l2i_and_l2v(data, errs, method, vmethod, 1, 1,
//                         (pair){1.5, HDRL_EPS_DATA},
//                         (pair){0.21652078422174625, HDRL_EPS_ERROR},
//                         expect_contrib);
//
//        method = hdrl_collapse_imagelist_to_image_minmax(1, 1);
//        vmethod = hdrl_collapse_imagelist_to_vector_minmax(1, 1);
//        cpl_image * expect_contrib_minmax =
//                        cpl_image_subtract_scalar_create(expect_contrib, 2);
//        test_l2i_and_l2v(data, errs, method, vmethod, 1, 1,
//                         (pair){3. / 2., HDRL_EPS_DATA},
//                         (pair){sqrt(0.1 * 0.1 + 0.01 * 0.01) / 2.,
//                                HDRL_EPS_ERROR},
//                         expect_contrib_minmax);
//        cpl_image_delete(expect_contrib_minmax);
//
//        method = hdrl_collapse_imagelist_to_image_weighted_mean();
//        vmethod = hdrl_collapse_imagelist_to_vector_weighted_mean();
//        test_l2i_and_l2v(data, errs, method, vmethod, 1, 1,
//                         (pair){1.9897091252756485, HDRL_EPS_ERROR},
//                         (pair){0.0099473975744101273, HDRL_EPS_ERROR},
//                         expect_contrib);
//    }
//    /* test all values rejected in line, list to image */
//    if (0){
//        double v[] = {1, 2, 1, 3, 2};
//        double e[] = {0.5, 0.7, 0.1, 1.0, 0.01};
//        for (cpl_size i = 0; i < nz; i++) {
//            cpl_image * tmp = cpl_imagelist_get(data, i);
//            cpl_image_set(tmp, 1, 1, v[i]);
//            cpl_image_reject(tmp, 1, 1);
//            tmp = cpl_imagelist_get(errs, i);
//            cpl_image_set(tmp, 1, 1, e[i]);
//            cpl_image_reject(tmp, 1, 1);
//        }
//        cpl_image_delete(expect_contrib);
//        expect_contrib = cpl_image_new_from_accepted(data);
//
//        hdrl_collapse_imagelist_to_image_t * img_meth[] = {
//            hdrl_collapse_imagelist_to_image_mean(),
//            hdrl_collapse_imagelist_to_image_sigclip(3., 3., 3),
//            hdrl_collapse_imagelist_to_image_weighted_mean(),
//            hdrl_collapse_imagelist_to_image_median(),
//        };
//        hdrl_collapse_imagelist_to_vector_t * vec_meth[] = {
//            hdrl_collapse_imagelist_to_vector_mean(),
//            hdrl_collapse_imagelist_to_vector_sigclip(3., 3., 3),
//            hdrl_collapse_imagelist_to_vector_weighted_mean(),
//            hdrl_collapse_imagelist_to_vector_median(),
//        };
//
//        for (size_t i = 0; i < ARRAY_LEN(img_meth); i++) {
//            test_l2i_and_l2v(data, errs, img_meth[i], vec_meth[i], 1, 1,
//                             (pair){NAN, HDRL_EPS_DATA},
//                             (pair){NAN, HDRL_EPS_ERROR},
//                             expect_contrib);
//        }
//
//        hdrl_collapse_imagelist_to_image_t * method =
//            hdrl_collapse_imagelist_to_image_minmax(1., 1.);
//        hdrl_collapse_imagelist_to_vector_t * vmethod =
//            hdrl_collapse_imagelist_to_vector_minmax(1., 1.);
//        cpl_image * expect_contrib_minmax =
//            cpl_image_subtract_scalar_create(expect_contrib, 2);
//        cpl_image_set(expect_contrib_minmax, 1, 1, 0);
//        test_l2i_and_l2v(data, errs, method, vmethod, 1, 1,
//                         (pair){NAN, HDRL_EPS_DATA},
//                         (pair){NAN, HDRL_EPS_ERROR},
//                         expect_contrib_minmax);
//        cpl_image_delete(expect_contrib_minmax);
//    }
//    /* test all values rejected, list to image */
//    if (0){
//        for (cpl_size i = 0; i < nz; i++) {
//            cpl_image * tmp = cpl_imagelist_get(data, i);
//            cpl_image_accept_all(tmp);
//            cpl_mask_not(cpl_image_get_bpm(tmp));
//        }
//        cpl_image_delete(expect_contrib);
//        expect_contrib = cpl_image_new_from_accepted(data);
//
//        hdrl_collapse_imagelist_to_image_t * img_meth[] = {
//            hdrl_collapse_imagelist_to_image_mean(),
//            hdrl_collapse_imagelist_to_image_sigclip(3., 3., 3),
//            hdrl_collapse_imagelist_to_image_weighted_mean(),
//            hdrl_collapse_imagelist_to_image_median(),
//            hdrl_collapse_imagelist_to_image_minmax(1., 1.),
//        };
//        hdrl_collapse_imagelist_to_vector_t * vec_meth[] = {
//            hdrl_collapse_imagelist_to_vector_mean(),
//            hdrl_collapse_imagelist_to_vector_sigclip(3., 3., 3),
//            hdrl_collapse_imagelist_to_vector_weighted_mean(),
//            hdrl_collapse_imagelist_to_vector_median(),
//            hdrl_collapse_imagelist_to_vector_minmax(1., 1.),
//        };
//
//        for (size_t i = 0; i < ARRAY_LEN(img_meth); i++) {
//            cpl_image * outimg_loc, *outerr_loc, *contrib_loc;
//            hdrl_imagelist_combine(data, errs, img_meth[i], &outimg_loc, &outerr_loc, &contrib_loc);
//            /* unlike cpl, don't emit error */
//            cpl_test_error(CPL_ERROR_NONE);
//
//            cpl_test_image_abs(contrib_loc, expect_contrib, 0);
//            cpl_test_eq(cpl_image_count_rejected(outimg_loc), nx *ny);
//            cpl_test_eq(cpl_image_count_rejected(outerr_loc), nx *ny);
//            cpl_image_delete(outimg_loc);
//            cpl_image_delete(outerr_loc);
//            cpl_image_delete(contrib_loc);
//
//            /* also check vector variant */
//            test_l2i_and_l2v(data, errs, img_meth[i], vec_meth[i], 1, 1,
//                             (pair){NAN, HDRL_EPS_DATA},
//                             (pair){NAN, HDRL_EPS_ERROR},
//                             expect_contrib);
//            cpl_test_error(CPL_ERROR_NONE);
//        }
//    }
//    /* test median error propagation with rejects, only makes sense on uniform
//     * errors as the scaling relies on gaussian errors */
//    if (0){
//        double v[] = {1, 2, 1, 3, 2};
//        double e[] = {1., 1., 1., 1., 1.};
//        int d;
//        for (cpl_size i = 0; i < nz; i++) {
//            cpl_image * tmp = cpl_imagelist_get(data, i);
//            cpl_image_set(tmp, 1, 1, v[i]);
//            cpl_image_set(tmp, 2, 2, v[i]);
//            if (i > 1) {
//                cpl_image_reject(tmp, 1, 1);
//            }
//            tmp = cpl_imagelist_get(errs, i);
//            cpl_image_set(tmp, 1, 1, e[i]);
//            cpl_image_set(tmp, 2, 2, e[i]);
//            if (i > 1) {
//                cpl_image_reject(tmp, 1, 1);
//            }
//        }
//        cpl_image_delete(expect_contrib);
//        expect_contrib = cpl_image_new_from_accepted(data);
//
//        hdrl_collapse_imagelist_to_image_t * method =
//            hdrl_collapse_imagelist_to_image_median();
//        hdrl_imagelist_combine(data, errs, method, &outimg, &outerr, &contrib);
//
//        /* contrib > 2 -> sqrt(nz * pi / 2) error scaling */
//        cpl_test_abs(cpl_image_get(outimg, 2, 2, &d), 2., HDRL_EPS_DATA);
//        cpl_test_abs(cpl_image_get(outerr, 2, 2, &d),
//                     1. / sqrt(nz) * sqrt(CPL_MATH_PI_2), HDRL_EPS_ERROR);
//        /* contrib <= 2 -> median is a mean, no scaling */
//        cpl_test_abs(cpl_image_get(outimg, 1, 1, &d), 1.5, HDRL_EPS_DATA);
//        cpl_test_abs(cpl_image_get(outerr, 1, 1, &d), 1. / sqrt(2.), HDRL_EPS_ERROR);
//        cpl_test_image_abs(contrib, expect_contrib, 0);
//
//        cpl_imagelist * vl = cpl_imagelist_new();
//        cpl_imagelist * el = cpl_imagelist_new();
//        prep_l2v_input(data, errs, 1, 1, vl, el);
//        hdrl_collapse_imagelist_to_vector_t * vmethod =
//            hdrl_collapse_imagelist_to_vector_median();
//        hdrl_collapse_imagelist_to_vector_call(vmethod, vl, el,
//                                               &voutimg, &vouterr, &acontrib,
//                                               NULL);
//
//        cpl_test_abs(cpl_vector_get(voutimg, 0), 1.5, HDRL_EPS_DATA);
//        cpl_test_abs(cpl_vector_get(vouterr, 0), 1. / sqrt(2.), HDRL_EPS_ERROR);
//
//        cpl_test_abs(cpl_array_get_int(acontrib, 0, NULL), 2, 0);
//        cpl_vector_delete(voutimg);
//        cpl_vector_delete(vouterr);
//        cpl_array_delete(acontrib);
//        cpl_imagelist_empty(vl);
//        cpl_imagelist_empty(el);
//
//        prep_l2v_input(data, errs, 2, 2, vl, el);
//        hdrl_collapse_imagelist_to_vector_call(vmethod, vl, el,
//                                               &voutimg, &vouterr, &acontrib,
//                                               NULL);
//
//        cpl_test_abs(cpl_vector_get(voutimg, 0), 2., HDRL_EPS_DATA);
//        cpl_test_abs(cpl_vector_get(vouterr, 0),
//        		1. / sqrt(nz) * sqrt(CPL_MATH_PI_2), HDRL_EPS_ERROR);
//
//        cpl_test_abs(cpl_array_get_int(acontrib, 0, NULL), 5, 0);
//        cpl_imagelist_delete(vl);
//        cpl_imagelist_delete(el);
//        hdrl_collapse_imagelist_to_vector_delete(vmethod);
//
//        TST_FREE_MODE_MODE;
//    }

    cpl_imagelist_delete(data);
    cpl_imagelist_delete(errs);
    cpl_image_delete(expect_err);
    cpl_image_delete(expect_contrib);
    cpl_image_delete(img);
    cpl_image_delete(err);
    cpl_vector_delete(expect_vimg);
    cpl_vector_delete(expect_verr);
    cpl_array_delete(expect_acontrib);

#undef TST_FREE_MODE
}


/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of reduce module
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    test_parameters();
    test_parlist();
    test_eout();
    test_results();
    test_results_mode();
    return cpl_test_end(0);
}



