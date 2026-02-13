/* $Id: hdrl_prototyping-test.c,v 1.0 2017-09-18 Exp $
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                    Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_prototyping.h"

#include <cpl.h>

/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_prototyping-test   Testing of the HDRL prototyping
 */
/*----------------------------------------------------------------------------*/

void test_prototyping_spatial_freq(void)
{
	int       dim_X = 64;
	int       dim_Y = 64;
	cpl_image *image = cpl_image_new(dim_X, dim_Y, CPL_TYPE_DOUBLE);


	double    gausfilt = 1.0;
	int       mirrorx  = 10;
	int       mirrory  = 10;


	/* Function to calculate the low spatial frequency */
	cpl_image *out1 = hdrl_get_spatial_freq(image, gausfilt, mirrorx, mirrory);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_image_delete(out1);
	cpl_image_delete(image);

	image = cpl_image_new(1, 1, CPL_TYPE_INT);
	cpl_image *out2 = hdrl_get_spatial_freq(image, gausfilt, 0, 0);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_image_delete(image);
	cpl_image_delete(out2);
}


void test_mime_image_polynomial_bkg(void)
{
	int        dim_X   = 10;
	int        dim_Y   = 10;
	cpl_image  *image  = NULL;
	cpl_matrix *coeffs = NULL;
	cpl_image *out = hdrl_mime_image_polynomial_bkg(image, dim_X, dim_Y, &coeffs);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(out);

	image = cpl_image_new(dim_X, dim_Y, CPL_TYPE_DOUBLE);
	out = hdrl_mime_image_polynomial_bkg(image, dim_X, dim_Y, &coeffs);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(out);
	cpl_test_nonnull(coeffs);

	cpl_image_delete(image);
	cpl_image_delete(out);
	cpl_matrix_delete(coeffs);
}

void test_mime_compute_polynomial_bkg(void){

	cpl_imagelist *images     = NULL;
	cpl_imagelist *bkg_images = NULL;
	int           dim_X       = 10;
	int           dim_Y       = 10;
	cpl_matrix    *coeffs     = NULL;


	/* Test input wrong */

	hdrl_mime_compute_polynomial_bkg(NULL, bkg_images, dim_X, dim_Y, &coeffs);
	cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);

	hdrl_mime_compute_polynomial_bkg(images, bkg_images, dim_X, dim_Y, &coeffs);
	cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);

	/* Normal execution */
	cpl_image *image = cpl_image_new(dim_X, dim_Y, CPL_TYPE_DOUBLE);
	    images = cpl_imagelist_new();
    cpl_imagelist_set(images, cpl_image_duplicate(image), 0);
    cpl_imagelist_set(images, cpl_image_duplicate(image), 1);
 /*	hdrl_mime_compute_polynomial_bkg(images, bkg_images, dim_X, dim_Y, &coeffs);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_matrix_delete(coeffs);
*/
    bkg_images = cpl_imagelist_new();
    cpl_imagelist_set(bkg_images, cpl_image_duplicate(image), 0);
    cpl_imagelist_set(bkg_images, cpl_image_duplicate(image), 1);
/* 	hdrl_mime_compute_polynomial_bkg(images, bkg_images, dim_X, dim_Y, &coeffs);
	cpl_test_error(CPL_ERROR_TYPE_MISMATCH);
	cpl_matrix_delete(coeffs);
*/
	/* Clean up */
	cpl_image_delete(image);
	cpl_imagelist_delete(images);
	cpl_imagelist_delete(bkg_images);
}

void test_mime_legendre_polynomials_create(void){

	int        npoly     = 0;
	double     a         = 2;
	double     b         = 2;
	cpl_matrix *x        = NULL;
	cpl_matrix *legendre;


	/* Test null inputs */

    legendre = hdrl_mime_legendre_polynomials_create(npoly, a, b, x);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_null(legendre);

    x = cpl_matrix_new(a, b);
    legendre = hdrl_mime_legendre_polynomials_create(npoly, a, b, x);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    cpl_test_null(legendre);
    cpl_matrix_delete(x);


    /* Normal execution */
    npoly = 2;
    a     = 3;
    b     = 5;
    x     = cpl_matrix_new(a, b);
    legendre = hdrl_mime_legendre_polynomials_create(npoly, a, b, x);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_nonnull(legendre);
    cpl_matrix_delete(legendre);
    cpl_matrix_delete(x);

}


void test_mime_legendre_tensors_create(void)
{
	int nx  = 2;
	int ny  = 2;
	int npx = 1;
	int npy = 1;

	cpl_matrix *out;

	/* Test wrong input */

	out = hdrl_mime_legendre_tensors_create(0, ny, npx, npy);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
	cpl_test_null(out);

	out = hdrl_mime_legendre_tensors_create(nx, 0, npx, npy);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
	cpl_test_null(out);

	out = hdrl_mime_legendre_tensors_create(nx, ny, 0,   npy);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
	cpl_test_null(out);

	out = hdrl_mime_legendre_tensors_create(nx, ny, npx, 0  );
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
	cpl_test_null(out);


	/* Normal work */
	out = hdrl_mime_legendre_tensors_create(nx, ny, npx, npy);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(out);
	cpl_matrix_delete(out);
}

void test_mime_matrix_linspace_create(void)
{
	int    n = 1;
	double a = 2;
	double b = 4;

	/* Test wrong input */
	cpl_matrix *out = hdrl_mime_matrix_linspace_create(n, a, b);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
	cpl_test_null(out);

	/* Normal work */
	n = 2;
	out = hdrl_mime_matrix_linspace_create(n, a, b);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(out);
	cpl_matrix_delete(out);
}

void test_mime_matrix_copy_column(void)
{
	int        j_1   = 1;
	int        j_2   = 1;
	int        nx    = 2;
	int        ny    = 4;
	cpl_matrix *mat1 = cpl_matrix_new(nx, ny);
	cpl_matrix *mat2 = cpl_matrix_new(nx, ny);

	cpl_matrix *mat  = cpl_matrix_new(ny, nx);


	/* Test wrong input */

	hdrl_mime_matrix_copy_column(NULL, j_1, mat2, j_2);
	cpl_test_error(CPL_ERROR_NULL_INPUT);

	hdrl_mime_matrix_copy_column(mat1, j_1, NULL, j_2);
	cpl_test_error(CPL_ERROR_NULL_INPUT);

	hdrl_mime_matrix_copy_column(mat1, j_1, mat, j_2);
	cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

	hdrl_mime_matrix_copy_column(mat1, -1, mat2, j_2);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

	hdrl_mime_matrix_copy_column(mat1, 10, mat2, j_2);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

	hdrl_mime_matrix_copy_column(mat1, -1, mat2, j_2);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

	hdrl_mime_matrix_copy_column(mat1, j_1, mat2, -1);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

	hdrl_mime_matrix_copy_column(mat1, j_1, mat2, 10);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);


	/* Normal work */
	hdrl_mime_matrix_copy_column(mat1, j_1, mat2, j_2);
	cpl_test_error(CPL_ERROR_NONE);


	cpl_matrix_delete(mat );
	cpl_matrix_delete(mat1);
	cpl_matrix_delete(mat2);
}


void test_mime_linalg_pairwise_column_tensor_products_create(void)
{
	int        nx    = 4;
	int        ny    = 4;

	cpl_matrix *mat1 = NULL;
	cpl_matrix *mat2 = NULL;

	cpl_matrix *out;

	/* Test wrong inputs */
	out = hdrl_mime_linalg_pairwise_column_tensor_products_create(mat1, mat2);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(out);

	mat1 = cpl_matrix_new(nx, ny);
	out = hdrl_mime_linalg_pairwise_column_tensor_products_create(mat1, mat2);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(out);

	/* Normal work */

	mat2 = cpl_matrix_new(nx, ny);
	out = hdrl_mime_linalg_pairwise_column_tensor_products_create(mat1, mat2);
	cpl_test_nonnull(out);
	cpl_matrix_delete(out);

	/* Clean up */
	cpl_matrix_delete(mat1);
	cpl_matrix_delete(mat2);
}

void test_mime_linalg_tensor_products_columns_create(void)
{
	int        nx    = 2;
	int        ny    = 4;
	cpl_matrix *mat1 = cpl_matrix_new(nx, ny);
	cpl_matrix *mat2 = cpl_matrix_new(nx, ny);

	cpl_matrix *mat  = cpl_matrix_new(ny, nx);


    cpl_matrix *out;


	/* Test wrong input */

	out = hdrl_mime_linalg_tensor_products_columns_create(NULL, mat2);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(out);

	out = hdrl_mime_linalg_tensor_products_columns_create(mat1, NULL);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(out);

	out = hdrl_mime_linalg_tensor_products_columns_create(mat1, mat);
	cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);
	cpl_test_null(out);


	/* Normal work */
	out = hdrl_mime_linalg_tensor_products_columns_create(mat1, mat2);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(out);
	cpl_matrix_delete(out);


	/* Clean up */
	cpl_matrix_delete(mat );
	cpl_matrix_delete(mat1);
	cpl_matrix_delete(mat2);
}

void test_mime_tensor_weights_create(void)
{
	int nx = 2;
	int ny = 2;

	cpl_matrix *out;


	/* Test wrong input */

	out = hdrl_mime_tensor_weights_create(1, ny);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
	cpl_test_null(out);

	out = hdrl_mime_tensor_weights_create(nx, 1);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
	cpl_test_null(out);


	/* Normal work */
	out = hdrl_mime_tensor_weights_create(nx, ny);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(out);
	cpl_matrix_delete(out);
}

void test_mime_matrix_mask_rows(void){

	int        nx     = 2;
	int        ny     = 8;
	cpl_matrix *mat   = cpl_matrix_new(nx, ny);
	cpl_mask   *mask  = cpl_mask_new(  nx, ny);
	cpl_mask   *mask1 = cpl_mask_new(  ny, nx);


	/* Test wrong input */

	hdrl_mime_matrix_mask_rows(NULL, mask);
	cpl_test_error(CPL_ERROR_NULL_INPUT);

	hdrl_mime_matrix_mask_rows(mat, NULL);
	cpl_test_error(CPL_ERROR_NULL_INPUT);

	hdrl_mime_matrix_mask_rows(mat, mask1);
	cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);


	/* Normal work */
	hdrl_mime_matrix_mask_rows(mat, mask);
	cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);


	/* Clean up */
	cpl_matrix_delete(mat);
	cpl_mask_delete(mask);
	cpl_mask_delete(mask1);
}

void test_mime_matrix_rescale_rows(void){

	cpl_matrix *mat   = cpl_matrix_new(2, 1);
	cpl_matrix *d     = cpl_matrix_new(2, 1);
	cpl_matrix *dmat  = cpl_matrix_new(2, 1);
	cpl_matrix *d1    = cpl_matrix_new(2, 2);
	cpl_matrix *dmat1 = cpl_matrix_new(1, 2);


	/* Test wrong input */

	hdrl_mime_matrix_rescale_rows(NULL, d,   dmat);
	cpl_test_error(CPL_ERROR_NULL_INPUT);

	hdrl_mime_matrix_rescale_rows(mat, NULL, dmat);
	cpl_test_error(CPL_ERROR_NULL_INPUT);

	hdrl_mime_matrix_rescale_rows(mat, d,    NULL);
	cpl_test_error(CPL_ERROR_NULL_INPUT);

	hdrl_mime_matrix_rescale_rows(mat, d1, dmat);
	cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

	hdrl_mime_matrix_rescale_rows(mat, d, dmat1);
	cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);


	/* Normal work */
	hdrl_mime_matrix_rescale_rows(mat, d, dmat);
	cpl_test_error(CPL_ERROR_NONE);

	cpl_matrix_delete(mat);
	cpl_matrix_delete(d);
	cpl_matrix_delete(d1);
	cpl_matrix_delete(dmat);
	cpl_matrix_delete(dmat1);
}

void test_mime_linalg_solve_tikhonov(void)
{
	cpl_matrix *mat  = cpl_matrix_new(1,2);
	cpl_matrix *rhs  = cpl_matrix_new(1,2);
	cpl_matrix *rhs1 = cpl_matrix_new(2,2);
	double     alpha = 2.;

	cpl_matrix *out;

    /* Test wrong input */

	out = hdrl_mime_linalg_solve_tikhonov(NULL, rhs, alpha);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(out);

	out = hdrl_mime_linalg_solve_tikhonov(mat, NULL, alpha);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(out);

	out = hdrl_mime_linalg_solve_tikhonov(mat, rhs1, alpha);
	cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);
	cpl_test_null(out);


	/* Normal work */
	out = hdrl_mime_linalg_solve_tikhonov(mat, rhs, alpha);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(out);
	cpl_matrix_delete(out);


	/* Clean up */
	cpl_matrix_delete(mat);
	cpl_matrix_delete(rhs);
	cpl_matrix_delete(rhs1);
}

void test_mime_linalg_normal_equations_create(void)
{
	cpl_matrix *mat  = cpl_matrix_new(1,2);
	double     alpha = 2.;

	cpl_matrix *out;

    /* Test wrong input */

	out = hdrl_mime_linalg_normal_equations_create(NULL, alpha);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(out);

	out = hdrl_mime_linalg_normal_equations_create(mat, -1.);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
	cpl_test_null(out);


	/* Normal work */
	out = hdrl_mime_linalg_normal_equations_create(mat, alpha);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(out);
	cpl_matrix_delete(out);


	/* Clean up */
	cpl_matrix_delete(mat);
}

void test_mime_matrix_product_left_transpose_create(void)
{
	int        nx    = 2;
	int        ny    = 4;
	cpl_matrix *mat1 = cpl_matrix_new(nx, ny);
	cpl_matrix *mat2 = cpl_matrix_new(nx, ny);

	cpl_matrix *mat  = cpl_matrix_new(ny, nx);


    cpl_matrix *out;


	/* Test wrong input */

	out = hdrl_mime_matrix_product_left_transpose_create(NULL, mat2);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(out);

	out = hdrl_mime_matrix_product_left_transpose_create(mat1, NULL);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(out);

	out = hdrl_mime_matrix_product_left_transpose_create(mat1, mat);
	cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);
	cpl_test_null(out);


	/* Normal work */
	out = hdrl_mime_matrix_product_left_transpose_create(mat1, mat2);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(out);
	cpl_matrix_delete(out);


	/* Clean up */
	cpl_matrix_delete(mat );
	cpl_matrix_delete(mat1);
	cpl_matrix_delete(mat2);
}

void test_mime_matrix_product(void)
{
	cpl_matrix *mat1      = cpl_matrix_new(2, 3);
	cpl_matrix *mat2      = cpl_matrix_new(3, 2);
	cpl_matrix *mat       = cpl_matrix_new(2, 3);

	cpl_matrix *product   = cpl_matrix_new(2, 2);
	cpl_matrix *product_1 = cpl_matrix_new(3, 2);
	cpl_matrix *product_2 = cpl_matrix_new(3, 2);


	/* Test wrong input */

	hdrl_mime_matrix_product(NULL, mat2, product);
	cpl_test_error(CPL_ERROR_NULL_INPUT);

	hdrl_mime_matrix_product(mat1, NULL, product);
	cpl_test_error(CPL_ERROR_NULL_INPUT);

	hdrl_mime_matrix_product(mat1, mat2, NULL);
	cpl_test_error(CPL_ERROR_NULL_INPUT);

	hdrl_mime_matrix_product(mat1, mat, product);
	cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

	hdrl_mime_matrix_product(mat1, mat2, product_1);
	cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

	hdrl_mime_matrix_product(mat1, mat2, product_2);
	cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

	/* Normal work */
	hdrl_mime_matrix_product(mat1, mat2, product);
	cpl_test_error(CPL_ERROR_NONE);


	/* Clean up */
	cpl_matrix_delete(mat1);
	cpl_matrix_delete(mat2);
	cpl_matrix_delete(mat);
	cpl_matrix_delete(product);
	cpl_matrix_delete(product_1);
	cpl_matrix_delete(product_2);
}


/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of parameter module
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    test_prototyping_spatial_freq();

    test_mime_image_polynomial_bkg();
    test_mime_compute_polynomial_bkg();

    test_mime_legendre_polynomials_create();
    test_mime_legendre_tensors_create();
    test_mime_matrix_linspace_create();
    test_mime_matrix_copy_column();
    test_mime_linalg_pairwise_column_tensor_products_create();
    test_mime_linalg_tensor_products_columns_create();
    test_mime_tensor_weights_create();

    test_mime_matrix_mask_rows();
    test_mime_matrix_rescale_rows();

    test_mime_linalg_solve_tikhonov();
    test_mime_linalg_normal_equations_create();

    test_mime_matrix_product_left_transpose_create();
    test_mime_matrix_product();


    return cpl_test_end(0);
}
