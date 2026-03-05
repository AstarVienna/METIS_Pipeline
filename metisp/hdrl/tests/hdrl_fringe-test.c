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
 *
 * hdrl_fringe-test.c
 *
 *  Created on: May 11, 2015
 *      Author: agabasch
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef HDRL_USE_PRIVATE
/* Make sure we pull in private functions like hdrl_mime_fringe_amplitudes. */
#define HDRL_USE_PRIVATE
#endif

#include "hdrl_fringe.h"
#include "hdrl_prototyping.h"
#include "hdrl_random.h"

#include <cpl.h>
#include <math.h>

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_fringe_test  Testing of the Fringe module
 */
/*----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/
void test_hermite_sum(void);
void hdrl_fringe_compute_test_input(void);
void hdrl_fringe_correct_test_input(void);
void hdrl_fringe_hermite_test(void);
void hdrl_mime_fringe_amplitudes_test(void);
void hdrl_mime_fringe_amplitudes_ls_test(void);

static cpl_matrix * hdrl_mime_hermite_functions_create(int n, double center,
          double scale, const cpl_matrix * x);


void hdrl_fringe_hermite_test(void)
{
    cpl_matrix *x;
    cpl_matrix *f;
    cpl_matrix *funs;
    cpl_matrix *coeffs;
    cpl_matrix *hseries;

    double    w, a, b;
    double    center, scale;
    int       i, n;
    int       nfun;


    /* setting parameters:
       n          number of equispaced nodes
       a, b       endpoints of the interval of integration
       nfun      number of the Hermite functions
       center      center of the Hermite functions
       scale      scaling of the Hermite functions
     */
    n = 10000;
    a = -50.0;
    b = 50.0;

    nfun = 20;
    center = 0.1;
    scale = 1.3;

    /* creating the equispaced nodes and weights */
    x = hdrl_mime_matrix_linspace_create(n, a, b);
    cpl_test_nonnull(x);
    w = (b - a) / (n - 1);

    /* creating the values of a test function */
    f = cpl_matrix_new(n, 1);
    cpl_test_nonnull(f);
    for (i = 0; i < n; i++)
    {
        double xi = (cpl_matrix_get_data(x))[i];
        (cpl_matrix_get_data(f))[i] = (1.0 + xi) * exp(-0.5 * xi * xi);
    }

    /* test improper inputs are properly handled */
    funs = hdrl_mime_hermite_functions_create(nfun, center, scale, NULL);
    cpl_test_null(funs);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    funs = hdrl_mime_hermite_functions_create(0, center, scale, x);
    cpl_test_null(funs);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    funs = hdrl_mime_hermite_functions_create(nfun, center, 0.0, x);
    cpl_test_null(funs);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    funs = hdrl_mime_hermite_functions_create(nfun, center, -1.0, x);
    cpl_test_null(funs);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    /* test functionality is working */
    funs = hdrl_mime_hermite_functions_create(nfun, center, scale, x);
    cpl_test_nonnull(funs);
    cpl_test_error(CPL_ERROR_NONE);

    /* computing the Hermite coefficients of the function */
    coeffs = hdrl_mime_matrix_product_left_transpose_create(funs, f);
    cpl_test_nonnull(coeffs);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(cpl_matrix_multiply_scalar(coeffs, w), CPL_ERROR_NONE);


    /* Test error handling of hdrl_mime_hermite_series_create. */
    hseries = hdrl_mime_hermite_series_create(nfun, center, scale, NULL, x);
    cpl_test_null(hseries);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    hseries = hdrl_mime_hermite_series_create(nfun, center, scale, coeffs, NULL);
    cpl_test_null(hseries);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    hseries = hdrl_mime_hermite_series_create(nfun, center, scale, NULL, NULL);
    cpl_test_null(hseries);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    hseries = hdrl_mime_hermite_series_create(0, center, scale, coeffs, x);
    cpl_test_null(hseries);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    hseries = hdrl_mime_hermite_series_create(nfun, center, -1.0, coeffs, x);
    cpl_test_null(hseries);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    hseries = hdrl_mime_hermite_series_create(nfun, center, 0.0, coeffs, x);
    cpl_test_null(hseries);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    /* comparing with the last partial sum */
    hseries = hdrl_mime_hermite_series_create(nfun, center, scale, coeffs, x);
    cpl_test_nonnull(hseries);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(cpl_matrix_subtract(hseries, f), CPL_ERROR_NONE);

    /* cleaning up */
    cpl_matrix_delete(x);
    cpl_matrix_delete(f);
    cpl_matrix_delete(funs);
    cpl_matrix_delete(coeffs);
    cpl_matrix_delete(hseries);
}


void test_fringe_mime_gmx1(void)
{
    double    x[1];
    double    params[6];
    double    result[6];
    double    y;

/* setting parameters */
    params[0] = 1.0;
    params[1] = 2.0;
    params[2] = sqrt(0.5);

    params[3] = 0.0;
    params[4] = 2.0;
    params[5] = sqrt(0.5);

/* testing the values */
    x[0] = 1.0;
    hdrl_mime_gmix1(x, params, &y);
    cpl_test_leq(fabs(y - 3.678794411714423e-01), 1e-15);

    x[0] = 2.0;
    hdrl_mime_gmix1(x, params, &y);
    cpl_test_leq(fabs(y - 1.0), 1e-15);

    x[0] = 3.0;
    hdrl_mime_gmix1(x, params, &y);
    cpl_test_leq(fabs(y - 3.678794411714423e-01), 1e-15);

    hdrl_mime_gmix_derivs1(x, params, result);
    cpl_test_leq(fabs(result[0] - 3.678794411714423e-01), 1e-15);
    cpl_test_leq(fabs(result[1] - 7.357588823428847e-01), 1e-15);
    cpl_test_leq(fabs(result[2] - 1.040520190045778e+00), 1e-15);
    cpl_test_leq(fabs(result[3] - 3.678794411714423e-01), 1e-15);
    cpl_test_zero(result[4]);
    cpl_test_zero(result[5]);

/* setting another set of parameters */
    params[0] = 0.0;
    params[1] = 2.0;
    params[2] = sqrt(0.5);

    params[3] = 1.0;
    params[4] = 2.0;
    params[5] = sqrt(0.5);

/* testing the values */
    x[0] = 1.0;
    hdrl_mime_gmix1(x, params, &y);
    cpl_test_leq(fabs(y - 3.678794411714423e-01), 1e-15);

    x[0] = 2.0;
    hdrl_mime_gmix1(x, params, &y);
    cpl_test_leq(fabs(y - 1.0), 1e-15);

    x[0] = 3.0;
    hdrl_mime_gmix1(x, params, &y);
    cpl_test_leq(fabs(y - 3.678794411714423e-01), 1e-15);

    hdrl_mime_gmix_derivs1(x, params, result);
    cpl_test_leq(fabs(result[0] - 3.678794411714423e-01), 1e-15);
    cpl_test_zero(result[1]);
    cpl_test_zero(result[2]);
    cpl_test_leq(fabs(result[3] - 3.678794411714423e-01), 1e-15);
    cpl_test_leq(fabs(result[4] - 7.357588823428847e-01), 1e-15);
    cpl_test_leq(fabs(result[5] - 1.040520190045778e+00), 1e-15);

}

/*---------------------------------------------------------------------------*/
/**
 * @brief    Create the Hermite functions.
 *
 * @param    n       Number of functions,
 * @param    center  Center,
 * @param    scale   Scale factor,
 * @param    x       Nodes, at which the functions are evaluated.
 *
 * @return   A matrix with the values of the functions.
 *
 * The i-th column contains the values of the i-th function at the
 * given nodes.  The functions have degrees 0, 1, ..., @a n-1.  The
 * specific dimensions of the matrix @a x are not used, only its size.
 * The functions are normalized in the L2-sense.
 *
 * The returned matrix must be deallocated using cpl_matrix_delete().
  */
/*---------------------------------------------------------------------------*/
static cpl_matrix * hdrl_mime_hermite_functions_create(int n, double center,
          double scale, const cpl_matrix * x)
{
    cpl_matrix *funs;
    double   *m;
    const double *mx;
    double    rt, xi;
    int       i, j, nr;


    /* testing input */
    cpl_ensure(x, CPL_ERROR_NULL_INPUT, NULL);

    if (n < 1 || scale <= 0.0)
    {
        cpl_error_set(cpl_func, CPL_ERROR_ILLEGAL_INPUT);
        return NULL;
    }

/* The specific dimensions of the matrix x are not used, only its size. */
    nr = cpl_matrix_get_nrow(x) * cpl_matrix_get_ncol(x);

/* allocating memory */
    funs = cpl_matrix_new(nr, n);

/* computing the normalization constant */
    rt = 1.0 / sqrt(sqrt(CPL_MATH_PI));

/* filling the first column */
    m = cpl_matrix_get_data(funs);
    mx = cpl_matrix_get_data_const(x);
    for (i = 0; i < nr; i++, m += n)
    {
        xi = (mx[i] - center) / scale;
        m[0] = rt * exp(-0.5 * xi * xi);
    }

/* filling the second column */
    m = cpl_matrix_get_data(funs);
    mx = cpl_matrix_get_data_const(x);

    for (i = 0; i < nr; i++, m += n)
    {
		xi = (mx[i] - center) / scale;
		m[1] = rt * sqrt(2.0) * xi * exp(-0.5 * xi * xi);
    }

/* filling the remaining columns by recursion */
    m = cpl_matrix_get_data(funs);
    for (i = 0; i < nr; i++, m += n)
    {
        xi = (mx[i] - center) / scale;
        for (j = 2; j < n; j++)
        {
            m[j] = sqrt(2.0) * xi * m[j - 1] - sqrt(j - 1) * m[j - 2];
            m[j] = m[j] / sqrt(j);
        }
    }

/* normalizing */
    cpl_matrix_multiply_scalar(funs, 1 / sqrt(scale));

    return funs;
}


void test_hermite_sum(void)
{
    cpl_matrix *x;
    cpl_matrix *sums;

    double    a, b;
    double    center, scale;
    int       n, nfun;

    /* setting parameters:
       n          number of equispaced nodes
       a, b       endpoints of the sampled interval
       nfun      number of the Hermite functions
       center      center of the Hermite functions
       scale      scaling of the Hermite functions
     */
    n = 6;
    a = 0.0;
    b = 5.0;

    nfun = 5;
    center = 0.5;
    scale = 1.3;

    /* creating the equispaced nodes and weights */
    x = hdrl_mime_matrix_linspace_create(n, a, b);
    cpl_test_nonnull(x);

    /* Test error handling. */
    sums = hdrl_mime_hermite_functions_sums_create(nfun, center, scale, NULL);
    cpl_test_null(sums);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    sums = hdrl_mime_hermite_functions_sums_create(0, center, scale, x);
    cpl_test_null(sums);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    sums = hdrl_mime_hermite_functions_sums_create(nfun, center, 0.0, x);
    cpl_test_null(sums);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    sums = hdrl_mime_hermite_functions_sums_create(nfun, center, -1.0, x);
    cpl_test_null(sums);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    /* creating sums of the Hermite functions */
    sums = hdrl_mime_hermite_functions_sums_create(nfun, center, scale, x);
    cpl_test_nonnull(sums);
    cpl_test_error(CPL_ERROR_NONE);

    cpl_test_leq(fabs(cpl_matrix_get(sums, 0, 0) - 1.685081590066050e+00),
                 1e-15);
    cpl_test_leq(fabs(cpl_matrix_get(sums, 1, 0) - 9.093843925414908e-01),
                 1e-15);
    cpl_test_leq(fabs(cpl_matrix_get(sums, 2, 0) - 4.521636055506448e-01),
                 1e-15);
    cpl_test_leq(fabs(cpl_matrix_get(sums, 3, 0) - 8.130124958110769e-01),
                 1e-15);
    cpl_test_leq(fabs(cpl_matrix_get(sums, 4, 0) - 8.013868548156017e-01),
                 1e-15);

    /* cleaning up */
    cpl_matrix_delete(x);
    cpl_matrix_delete(sums);


}


void hdrl_fringe_compute_test_input(void)
{
    cpl_size nx = 21;
    cpl_size ny= 32;
    hdrl_parameter      *   collapse_params;
    collapse_params = hdrl_collapse_mean_parameter_create();

    hdrl_imagelist * ilist_fringe = hdrl_imagelist_new();
    cpl_imagelist *  ilist_obj = cpl_imagelist_new();
    cpl_mask * stat_mask = cpl_mask_new(nx, ny);

    hdrl_image * master;
    cpl_image * contrib_map;

    hdrl_image * hima1 = hdrl_image_new(nx, ny);
    hdrl_image * hima2 = hdrl_image_new(nx, ny);
    hdrl_image * hima_dimen1 = hdrl_image_new(nx+5, ny+10);
    hdrl_image * hima_dimen2 = hdrl_image_new(nx+5, ny+10);

    cpl_image * cima1 = cpl_image_new(nx, ny, CPL_TYPE_FLOAT);
    cpl_image * cima2 = cpl_image_new(nx, ny, CPL_TYPE_FLOAT);

    hdrl_image_add_scalar(hima1, (hdrl_value){1., 0.1});
    hdrl_image_add_scalar(hima2, (hdrl_value){10., 1.});
    cpl_image_add_scalar(cima1, 1.);
    cpl_image_add_scalar(cima2, 10.);


    hdrl_fringe_compute(NULL, ilist_obj, stat_mask, collapse_params,
                        &master, &contrib_map, NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_null(master);
    cpl_test_null(contrib_map);

    cpl_table *qctable = (cpl_table*)0xbadbeef;
    hdrl_fringe_compute(NULL, ilist_obj, stat_mask, collapse_params,
                        &master, &contrib_map, &qctable);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_null(master);
    cpl_test_null(contrib_map);

    hdrl_fringe_compute(ilist_fringe, ilist_obj, stat_mask, NULL,
                        &master, &contrib_map, NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_null(master);
    cpl_test_null(contrib_map);

    hdrl_imagelist_set(ilist_fringe, hima1, 0);
    hdrl_imagelist_set(ilist_fringe, hima2, 1);
    cpl_imagelist_set(ilist_obj, cima1, 0);
    hdrl_fringe_compute(ilist_fringe, ilist_obj, stat_mask, collapse_params,
                         &master, &contrib_map, NULL);
    cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_test_null(master);
    cpl_test_null(contrib_map);
    cpl_imagelist_set(ilist_obj, cima2, 1);


    hima2 = hdrl_imagelist_unset(ilist_fringe, 1);
    hima1 = hdrl_imagelist_unset(ilist_fringe, 0);
    hdrl_fringe_compute(ilist_fringe, ilist_obj, stat_mask, collapse_params,
                         &master, &contrib_map, NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_null(master);
    cpl_test_null(contrib_map);

    /* Images in ilist_fringe and ilist_obj have now different dimensions */
    hdrl_imagelist_set(ilist_fringe, hima_dimen1, 0);
    hdrl_imagelist_set(ilist_fringe, hima_dimen2, 1);

    hdrl_fringe_compute(ilist_fringe, ilist_obj, stat_mask, collapse_params,
                         &master, &contrib_map, NULL);
    cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_test_null(master);
    cpl_test_null(contrib_map);

    hima_dimen2 = hdrl_imagelist_unset(ilist_fringe, 1);
    hima_dimen1 = hdrl_imagelist_unset(ilist_fringe, 0);

    /*
    cpl_msg_warning(cpl_func,"ilist_fringe=%p",ilist_fringe);
    cpl_msg_warning(cpl_func,"ilist_obj=%p",ilist_obj);
    cpl_msg_warning(cpl_func,"stat mask=%p",stat_mask);
    cpl_msg_warning(cpl_func,"collapse params=%p",collapse_params);
    cpl_msg_warning(cpl_func,"master=%p",master);
    cpl_msg_warning(cpl_func,"contrib map=%p",contrib_map);
    hdrl_fringe_compute(ilist_fringe, ilist_obj, stat_mask, collapse_params,
                           &master, &contrib_map);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_nonnull(master);
    cpl_test_nonnull(contrib_map);
    hdrl_image_delete(master);
    cpl_image_delete(contrib_map);
    */

    /* Test for success when computing a master fringe just from two frames
       without any object or static masks. */
    hdrl_imagelist_set(ilist_fringe, hima1, 0);
    hdrl_imagelist_set(ilist_fringe, hima2, 1);
    hdrl_fringe_compute(ilist_fringe, NULL, NULL, collapse_params,
                        &master, &contrib_map, NULL);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_nonnull(master);
    cpl_test_nonnull(contrib_map);
    hdrl_image_delete(master);
    cpl_image_delete(contrib_map);

    /* Final cleanup */
    hdrl_image_delete(hima_dimen1);
    hdrl_image_delete(hima_dimen2);
    hdrl_imagelist_delete(ilist_fringe);
    cpl_imagelist_delete(ilist_obj);
    hdrl_parameter_delete(collapse_params);
    cpl_mask_delete(stat_mask);
}


void hdrl_fringe_correct_test_input(void)
{
    cpl_size nx = 21;
    cpl_size ny= 32;

    hdrl_imagelist * ilist_fringe = hdrl_imagelist_new();
    cpl_imagelist *  ilist_obj = cpl_imagelist_new();
    cpl_mask * stat_mask = cpl_mask_new(nx, ny);

    hdrl_image * masterfringe = hdrl_image_new(nx, ny);;


    hdrl_image * hima1 = hdrl_image_new(nx, ny);
    hdrl_image * hima2 = hdrl_image_new(nx, ny);

    hdrl_image * hima_dimen1 = hdrl_image_new(nx+5, ny+10);
    hdrl_image * hima_dimen2 = hdrl_image_new(nx+5, ny+10);

    cpl_image * cima1 = cpl_image_new(nx, ny, CPL_TYPE_FLOAT);
    cpl_image * cima2 = cpl_image_new(nx, ny, CPL_TYPE_FLOAT);

    hdrl_image_add_scalar(hima1, (hdrl_value){1., 0.1});
    hdrl_image_add_scalar(hima2, (hdrl_value){10., 1.});
    cpl_image_add_scalar(cima1, 1.);
    cpl_image_add_scalar(cima2, 10.);


    hdrl_fringe_correct(NULL, ilist_obj, stat_mask, masterfringe, NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    hdrl_imagelist_set(ilist_fringe, hima1, 0);
    hdrl_imagelist_set(ilist_fringe, hima2, 1);
    cpl_imagelist_set(ilist_obj, cima1, 0);
    hdrl_fringe_correct(ilist_fringe, ilist_obj, stat_mask, masterfringe, NULL);
    cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_imagelist_set(ilist_obj, cima2, 1);


    hima2 = hdrl_imagelist_unset(ilist_fringe, 1);
    hima1 = hdrl_imagelist_unset(ilist_fringe, 0);
    hdrl_fringe_correct(ilist_fringe, ilist_obj, stat_mask, masterfringe, NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    /* Images in ilist_fringe and ilist_obj have now different dimensions */
    hdrl_imagelist_set(ilist_fringe, hima_dimen1, 0);
    hdrl_imagelist_set(ilist_fringe, hima_dimen2, 1);

    hdrl_fringe_correct(ilist_fringe, ilist_obj, stat_mask, masterfringe, NULL);
    cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

    hima_dimen2 = hdrl_imagelist_unset(ilist_fringe, 1);
    hima_dimen1 = hdrl_imagelist_unset(ilist_fringe, 0);

    hdrl_imagelist_set(ilist_fringe, hima1, 0);
    hdrl_imagelist_set(ilist_fringe, hima2, 1);

    hdrl_fringe_correct(ilist_fringe, ilist_obj, stat_mask, masterfringe, NULL);
    cpl_test_error(CPL_ERROR_NONE);

    hdrl_fringe_correct(ilist_fringe, ilist_obj, NULL, masterfringe, NULL);
    cpl_test_error(CPL_ERROR_NONE);

    hdrl_fringe_correct(ilist_fringe, NULL, NULL, masterfringe, NULL);
    cpl_test_error(CPL_ERROR_NONE);


    /* now we add real data: two frames with Poisson noise centred at different levels */
	cpl_size sizex = hdrl_image_get_size_x(hima1);
	cpl_size sizey = hdrl_image_get_size_y(hima1);
	hdrl_random_state * rng = hdrl_random_state_new(1, NULL);

	cpl_image *cplima1 = hdrl_image_get_image(hima1);
	cpl_image *cplima2 = hdrl_image_get_image(hima2);

	for(cpl_size i = 1; i <= sizex; i++) {
		for(cpl_size j = 1; j <= sizey; j++) {
			cpl_image_set(cplima1, i, j, (double)hdrl_random_poisson(rng, 100.));
			cpl_image_set(cplima2, i, j, (double)hdrl_random_poisson(rng, 200.));
		}
	}

    hdrl_random_state_delete(rng);
    float* pobj = cpl_image_get_data_float(cima1);
    int sx = cpl_image_get_size_x(cima1);
    int sy = cpl_image_get_size_y(cima1);
    int i_min = 0.25 * sx;
    int i_max = 0.75 * sx;
    int j_min = 0.25 * sy;
    int j_max = 0.75 * sy;

    for(int j = j_min; j < j_max; j++) {
        for(int i = i_min; i < i_max; i++) {
           pobj[j*sx+i] = 0;
        }
    }

    pobj = cpl_image_get_data_float(cima2);
    for(int j = j_min; j < j_max; j++) {
          for(int i = i_min; i < i_max; i++) {
             pobj[j*sx+i] = 0;
          }
    }
    i_min = 0.1 * sx;
    i_max = 0.2 * sx;
    j_min = 0.1 * sy;
    j_max = 0.2 * sy;

    cpl_binary* pmask = cpl_mask_get_data(stat_mask);

    for(int j = j_min; j < j_max; j++) {
              for(int i = i_min; i < i_max; i++) {
                 pmask[j*sx+i] = 0;
              }
    }


    cpl_image* contrib_map=NULL;
    hdrl_parameter      *   collapse_params;
    collapse_params = hdrl_collapse_mean_parameter_create();
    hdrl_image_delete(masterfringe);
    hdrl_fringe_compute(ilist_fringe, ilist_obj, stat_mask, collapse_params,
                             &masterfringe, &contrib_map, NULL);
    hdrl_fringe_correct(ilist_fringe, NULL, NULL, masterfringe, NULL);
    cpl_test_error(CPL_ERROR_NONE);

    /* Final cleanup */
    hdrl_image_delete(hima_dimen1);
    hdrl_image_delete(hima_dimen2);
    hdrl_imagelist_delete(ilist_fringe);
    cpl_imagelist_delete(ilist_obj);
    hdrl_parameter_delete(collapse_params);
    cpl_image_delete(contrib_map);

    cpl_mask_delete(stat_mask);
    hdrl_image_delete(masterfringe);

}


void hdrl_mime_fringe_amplitudes_test(void)
{
    cpl_matrix * matrix;
    cpl_image * image_double = cpl_image_new(10, 10, CPL_TYPE_DOUBLE);
    cpl_image * image_float = cpl_image_new(10, 10, CPL_TYPE_FLOAT);
    cpl_mask * mask = cpl_mask_new(10, 10);

    /* The following test are to check correct error handling for invalid
       input. */
    matrix = hdrl_mime_fringe_amplitudes(NULL, NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_null(matrix);

    matrix = hdrl_mime_fringe_amplitudes(image_double, NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_null(matrix);

    matrix = hdrl_mime_fringe_amplitudes(NULL, mask);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_null(matrix);

    /* Check handling of input images that are not doubles. */
    matrix = hdrl_mime_fringe_amplitudes(image_float, mask);
    cpl_test_error(CPL_ERROR_INVALID_TYPE);
    cpl_test_null(matrix);

    /* Check handling of input when nothing is marked in the mask. */
    cpl_mask_not(mask);
    cpl_test_error(CPL_ERROR_NONE);
    matrix = hdrl_mime_fringe_amplitudes(image_double, mask);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    cpl_test_null(matrix);

    cpl_mask_delete(mask);
    cpl_image_delete(image_float);
    cpl_image_delete(image_double);
}


void hdrl_mime_fringe_amplitudes_ls_test(void)
{
    cpl_matrix * matrix;
    cpl_image * image_double1 = cpl_image_new(10, 10, CPL_TYPE_DOUBLE);
    cpl_image * image_double2 = cpl_image_new(10, 10, CPL_TYPE_DOUBLE);
    cpl_image * image_float = cpl_image_new(10, 10, CPL_TYPE_FLOAT);
    cpl_mask * mask = cpl_mask_new(10, 10);

    /* The following test are to check correct error handling for invalid
       input. */
    matrix = hdrl_mime_fringe_amplitudes_ls(NULL, NULL, NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_null(matrix);

    matrix = hdrl_mime_fringe_amplitudes_ls(NULL, mask, image_double2);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_null(matrix);

    matrix = hdrl_mime_fringe_amplitudes_ls(image_double1, NULL, image_double2);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_null(matrix);

    matrix = hdrl_mime_fringe_amplitudes_ls(image_double1, mask, NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_null(matrix);

    /* Check handling of input images that are not doubles. */
    matrix = hdrl_mime_fringe_amplitudes_ls(image_float, mask, image_double2);
    cpl_test_error(CPL_ERROR_INVALID_TYPE);
    cpl_test_null(matrix);

    matrix = hdrl_mime_fringe_amplitudes_ls(image_double1, mask, image_float);
    cpl_test_error(CPL_ERROR_INVALID_TYPE);
    cpl_test_null(matrix);

    /* Check handling of input when nothing is marked in the mask. */
    cpl_mask_not(mask);
    cpl_test_error(CPL_ERROR_NONE);
    matrix = hdrl_mime_fringe_amplitudes_ls(image_double1, mask, image_double2);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    cpl_test_null(matrix);

    cpl_mask_delete(mask);
    cpl_image_delete(image_float);
    cpl_image_delete(image_double2);
    cpl_image_delete(image_double1);
}



/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of hdrl_fringe module.
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    hdrl_fringe_hermite_test();
    test_fringe_mime_gmx1();
    test_hermite_sum();
    hdrl_fringe_compute_test_input(); //problem testing proper inputs
    hdrl_fringe_correct_test_input(); //problem testing proper inputs
    hdrl_mime_fringe_amplitudes_test();
    hdrl_mime_fringe_amplitudes_ls_test();

    test_hermite_sum();
    return cpl_test_end(0);

}
