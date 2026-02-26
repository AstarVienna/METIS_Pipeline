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

#include "hdrl_prototyping.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <complex.h>
/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_prototyping   Prototyping object (derived from MIME project)
 *
 * This module contains functions derived from MIME project, adapted to HDRL.
 * @internal
 * see hdrl_prototyping-test.c for an example usage
 */
/*----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                                Defines
 -----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
                                   Functions
 -----------------------------------------------------------------------------*/

/**@{*/


/*-----------------------------------------------------------------------------
                                   Static
 -----------------------------------------------------------------------------*/


static cpl_image *hdrl_mirror_edges(cpl_image *image, int dx, int dy);
static cpl_image *hdrl_gen_lowpass(int xs, int ys, double sigma_x,
                                   double sigma_y);



/**      Function to calculate the low spatial frequency
*
* @brief      Get low spatial frequency componenets from an image using the FFTW
* @param    ima         image
* @param    gausfilt   Gaussian Fourier filter size
* @param    mirrorx    for mirroring edges (ocfft continuity)
* @param    mirrory    for mirroring edges (ocfft continuity)
*
* @return       1 newly allocated image.
*/
cpl_image * hdrl_get_spatial_freq(cpl_image *ima, double gausfilt, int mirrorx,
                                  int mirrory){

    int xsize=0, ysize=0;

    double sigma_x = 0.;
    double sigma_y = 0.;

    cpl_image *clean_flat;
    cpl_image *eflat;
    cpl_image *eflat_complex=NULL;
    cpl_image *eflat_real=NULL;

    cpl_image *filter_image=NULL;
    cpl_image *filter_image_complex=NULL;
    cpl_image *flat_real;


    /*Clean flat from bad pixels if bpm exists*/
    //clean_flat = cpl_image_duplicate(flat);

    /*The algorithm uses float, so we have to cast here or change the algo...*/
    cpl_type ima_type = cpl_image_get_type(ima);

    clean_flat = cpl_image_cast(ima, CPL_TYPE_FLOAT);
        cpl_detector_interpolate_rejected(clean_flat);


    /* Expand the flat image using the mirror edges function */
    eflat = hdrl_mirror_edges(clean_flat, mirrorx, mirrory);

	if (clean_flat != NULL) {
		cpl_image_delete(clean_flat);
		clean_flat = NULL;
	}

    if(eflat == NULL){
        cpl_msg_error(cpl_func,"Filter image is NULL");
        return NULL;
    }

    xsize = cpl_image_get_size_x(eflat);
    ysize = cpl_image_get_size_y(eflat);

    sigma_x = gausfilt;
    sigma_y = (double)(sigma_x * ysize) / xsize;


    /* Generate a lowpass filter to be used in the FFT convolution */
    filter_image = hdrl_gen_lowpass(xsize, ysize, sigma_x, sigma_y);
    if(filter_image == NULL){
        cpl_msg_error(cpl_func,"Filter image is NULL");
        cpl_image_delete(eflat);
        return NULL;
    }

    eflat_complex = cpl_image_new(xsize,ysize, CPL_TYPE_FLOAT_COMPLEX);
    eflat_real = cpl_image_new(xsize,ysize, CPL_TYPE_FLOAT);
    filter_image_complex =cpl_image_cast(filter_image,CPL_TYPE_FLOAT_COMPLEX);

    /*Free memory*/
    cpl_image_delete(filter_image);


    /* Apply a forward FFT on the images  */
    cpl_fft_image(eflat_complex, eflat, CPL_FFT_FORWARD);
    /*Free memory*/
    cpl_image_delete(eflat);

    /*Multiply the filter with the the FFT image */
    cpl_image_multiply(eflat_complex,filter_image_complex);


    /* Apply a backward FFT on the images  */
    cpl_fft_image(eflat_real, eflat_complex,CPL_FFT_BACKWARD);
    /*Free memory*/
    cpl_image_delete(eflat_complex);
    cpl_image_delete(filter_image_complex);

    /*  Extract original image from the expanded image.  */
    flat_real = cpl_image_extract(eflat_real, mirrorx+1, mirrory+1,
            xsize-mirrorx, ysize-mirrory);

    if (flat_real == NULL) {
        cpl_msg_error (cpl_func,"Real extracted image is NULL. <%s>", cpl_error_get_message());
        return NULL;
    }
    cpl_image_delete(eflat_real);

    cpl_image * out_double = cpl_image_cast(flat_real, ima_type);
    cpl_image_delete(flat_real);
    return out_double;
}

/*-------------------------------------------------------------------------*/
/**
@brief	Generate a low pass filter for FFT convolution .
@param	xs	x size of the generated image.
@param	ys	y size of the generated image.
@param	sigma_x	Sigma for the gaussian distribution.
@param	sigma_y      Sigma for the gaussian distribution.
@return	1 newly allocated image.

This function generates an image of a 2d gaussian, modified in such
a way that the different quadrants have a quadrants of the gaussian
in the corner. This image is suitable for FFT convolution.
Copied from eclipse, src/iproc/generate.c

The returned image must be deallocated.
*/
/*--------------------------------------------------------------------------*/
static cpl_image * hdrl_gen_lowpass(int xs, int ys, double sigma_x,
                                    double sigma_y)
{

	int i= 0.0;
	int j= 0.0;
	int hlx= 0.0;
	int hly = 0.0;
	double x= 0.0;
	double gaussval= 0.0;
	float *data;

	cpl_image 	*lowpass_image;


	lowpass_image = cpl_image_new (xs, ys, CPL_TYPE_FLOAT);
	if (lowpass_image == NULL) {
		cpl_msg_error (cpl_func, "Cannot generate lowpass filter <%s>",cpl_error_get_message());
		return NULL;
	}

	hlx = xs/2;
	hly = ys/2;

	data = cpl_image_get_data_float(lowpass_image);

	/* Given an image with pixels 0<=i<N, 0<=j<M then the convolution image
   has the following properties:

   ima[0][0] = 1
   ima[i][0] = ima[N-i][0] = exp (-0.5 * (i/sig_i)^2)   1<=i<N/2
   ima[0][j] = ima[0][M-j] = exp (-0.5 * (j/sig_j)^2)   1<=j<M/2
   ima[i][j] = ima[N-i][j] = ima[i][M-j] = ima[N-i][M-j]
             = exp (-0.5 * ((i/sig_i)^2 + (j/sig_j)^2))
	 */

	data[0] = (float)1.0;

	/* first row */
	for (i=1 ; i<=hlx ; i++) {
		x = (double)i / sigma_x;
		gaussval = (double)exp(-0.5*x*x);
		data[i] = gaussval;
		data[xs-i] = gaussval;
	}

	for (j=1; j<=hly ; j++) {
		double y = (double)j / sigma_y;
		/* first column */
		data[j*xs] = (double)exp(-0.5*y*y);
		data[(ys-j)*xs] = (double)exp(-0.5*y*y);

		for (i=1 ; i<=hlx ; i++) {
			/* Use internal symetries */
			x = (double) i / sigma_x;
			gaussval = (double)exp (-0.5*(x*x+y*y));
			data[j*xs+i] = gaussval;
			data[(j+1)*xs-i] = gaussval;
			data[(ys-j)*xs+i] = gaussval;
			data[(ys+1-j)*xs-i] = gaussval;

		}
	}

	/* FIXME: for the moment, reset errno which is coming from exp()
            in first for-loop at i=348. This is causing cfitsio to
            fail when loading an extension image (bug in cfitsio too).
	 */
	if(errno != 0)
		errno = 0;

	return lowpass_image;
}



/**
@brief    expand image by mirroring edges
@param    image_in      Image.
@param    x_size        the number of pixels in x to expand
@param    y_size        the number of pixels in y to expand

*/
/*--------------------------------------------------------------------------*/
cpl_image *hdrl_mirror_edges(cpl_image *image, int dx, int dy)
{

	intptr_t xs = cpl_image_get_size_x(image);
	intptr_t ys = cpl_image_get_size_y(image);
	intptr_t xx = xs+(2*dx);
	intptr_t yy = ys+(2*dy);
	float * data = cpl_image_get_data_float(image);
	cpl_image * big_image = cpl_image_new(xx, yy, CPL_TYPE_FLOAT);
	float * out_data = cpl_image_get_data_float(big_image);

	for (intptr_t j=0; j<ys ; j++){
		intptr_t inrow = j*xs;
		intptr_t outrow = (j+dy)*xx;

		for (intptr_t i=0; i<xs ; i++){
			out_data[outrow+dx+i] = data[inrow+i];
		}

		for (intptr_t i=0; i<dx; i++){
			out_data[outrow+i] = data[inrow+dx-i-1];
			out_data[outrow+xs+dx+i] = data[inrow+xs-i-1];

		}
	}

	for (intptr_t j=0; j<dy ; j++) {

		for (intptr_t i=0; i<xx; i++) {
			out_data[j*xx+i] = out_data[(2*dy-j-1)*xx+i];
			out_data[(yy-j-1)*xx+i] = out_data[(yy-2*dy+j)*xx+i];
		}
	}

	return big_image;
}



cpl_image * hdrl_mime_image_polynomial_bkg(cpl_image * image,
                                             int dim_X, int dim_Y,
                                             cpl_matrix ** coeffs){
    cpl_imagelist * imlist;
    cpl_imagelist * bkg_imlist;
    cpl_image * bkg_image;
    cpl_image * bkg_image_origtype;

    cpl_type ima_type;
    if (image == NULL){
        cpl_error_set_message(cpl_func,
                              CPL_ERROR_NULL_INPUT,
                              "Null input image provided");
        return NULL;
    }

    ima_type = cpl_image_get_type(image);

    imlist = cpl_imagelist_new();
    bkg_imlist = cpl_imagelist_new();
    cpl_imagelist_set(imlist, image, 0);

    hdrl_mime_compute_polynomial_bkg(imlist, bkg_imlist, dim_X, dim_Y, coeffs);

    cpl_imagelist_unwrap(imlist);
    bkg_image = cpl_imagelist_unset(bkg_imlist, 0);
    cpl_imagelist_delete(bkg_imlist);

    bkg_image_origtype=cpl_image_cast(bkg_image, ima_type);
    cpl_image_delete(bkg_image);

    return bkg_image_origtype;

}

/*----------------------------------------------------------------------------*/
/**
 * @brief    Fit smooth background for a list of images.
 *
 * @param      images      List of images.
 * @param[out] bkg_images  Smooth background images.
 * @param      dim_X,dim_Y dimensions of polynomials in @f$x@f$ and @f$y@f$.
 * @param[out] coeffs      Polynomial coefficients.
 *
 * @return   @c CPL_ERROR_NONE or the appropriate error code.
 *
 * This function computes smooth background images by fitting
 * polynomial surfaces to the input images. Bad-pixel masks for the
 * images are taken into account.
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_mime_compute_polynomial_bkg(
    const cpl_imagelist       * images,
          cpl_imagelist       * bkg_images,
          int                   dim_X,
          int                   dim_Y,
          cpl_matrix         ** coeffs
    )
{

    int         n_images;
    int         n_x;
    int         n_y;
    cpl_matrix * poly_tensors;
    int          n_tensor;
    int             im;
    cpl_matrix    * weights;

    cpl_msg_debug(cpl_func, "Polynomial with X, Y dimensions %2d, %2d.",
                  dim_X, dim_Y);

    /* Sanity Check of input data, and parameters */
    cpl_error_ensure(images != NULL, CPL_ERROR_DATA_NOT_FOUND,
        	return CPL_ERROR_DATA_NOT_FOUND, "list of dithered images is empty");

    cpl_error_ensure(cpl_imagelist_is_uniform(images) == 0, CPL_ERROR_INCOMPATIBLE_INPUT,
        	return CPL_ERROR_INCOMPATIBLE_INPUT, "input image list have non uniform data");

    /*
       Compute Dimensions
    */
    n_images = cpl_imagelist_get_size(images);
    n_x      = cpl_image_get_size_x(cpl_imagelist_get_const(images, 0));
    n_y      = cpl_image_get_size_y(cpl_imagelist_get_const(images, 0));

    /*
      Create tensor products of polynomials
    */
    poly_tensors  = hdrl_mime_legendre_tensors_create(n_x, n_y, dim_X, dim_Y);
    n_tensor      = cpl_matrix_get_ncol(poly_tensors);
    *coeffs        = cpl_matrix_new(n_tensor, n_images);

    weights = hdrl_mime_tensor_weights_create(n_x, n_y);

    /*
       Loop over each image to find the corresponding
       sky background
    */

    for (im = 0; im < n_images; im++){
        cpl_matrix    * image_data;
        cpl_matrix    * bkg_image_data;
        cpl_matrix    * masked_tensors;
        cpl_matrix    * masked_image;
        cpl_matrix    * image_double_wrap;
        cpl_matrix    * coeff;
        cpl_image     * image;
        cpl_image     * bkg_image;
        cpl_image     * image_double;
        cpl_image     * bkg_image_float;
        cpl_mask      * mask_bin;
        double          alpha = 1.0e-10;

        image_data     = cpl_matrix_new(n_x*n_y, 1);
        bkg_image_data = cpl_matrix_new(n_x*n_y, 1);
        masked_image   = cpl_matrix_new(n_x * n_y, 1);
        masked_tensors = cpl_matrix_new(n_x * n_y, n_tensor);


        /*
          Load image, and mask
        */

        image      = cpl_image_duplicate(cpl_imagelist_get_const(images, im));
        mask_bin   = cpl_image_get_bpm(image);
        if (mask_bin == NULL) {
            cpl_msg_info(cpl_func, "mask not available");
            cpl_matrix_delete(poly_tensors);
            cpl_matrix_delete(image_data);
            cpl_matrix_delete(bkg_image_data);
            cpl_matrix_delete(masked_image);
            cpl_matrix_delete(masked_tensors);
            cpl_image_delete(image);
            return cpl_error_set(cpl_func, CPL_ERROR_DATA_NOT_FOUND);
        }

        image_double = cpl_image_cast(image, CPL_TYPE_DOUBLE);
        image_double_wrap = cpl_matrix_wrap(n_x*n_y, 1,
                             cpl_image_get_data_double(image_double));
        cpl_matrix_copy(image_data, image_double_wrap, 0, 0);

        /*
          Loop over all pixels, to reject from mask.
        */
        cpl_matrix_copy(masked_tensors, poly_tensors, 0, 0);
        hdrl_mime_matrix_mask_rows(masked_tensors, mask_bin);
        hdrl_mime_matrix_rescale_rows(masked_tensors, weights, masked_tensors);

        cpl_matrix_copy(masked_image, image_data, 0, 0);
        hdrl_mime_matrix_mask_rows(masked_image, mask_bin);
        hdrl_mime_matrix_rescale_rows(masked_image, weights, masked_image);

        /*
          Find coefficients, agument the matrix of coefficients
        */
        coeff = hdrl_mime_linalg_solve_tikhonov(masked_tensors, masked_image,
                                              alpha);
        cpl_matrix_copy(*coeffs, coeff, 0, im);


        /* hdrl_mime_matrix_formatted_dump(coeff, NULL); */

        /*
          Synthesize background, copy to image, and then to return imagelist
        */
        hdrl_mime_matrix_product(poly_tensors, coeff, bkg_image_data);
        bkg_image = cpl_image_wrap_double(n_x, n_y,
                    cpl_matrix_get_data(bkg_image_data));
        bkg_image_float = cpl_image_cast(bkg_image, CPL_TYPE_FLOAT);
        cpl_imagelist_set(bkg_images, bkg_image_float, im);

        cpl_matrix_delete(image_data);
        cpl_matrix_delete(bkg_image_data);
        cpl_matrix_delete(masked_image);
        cpl_matrix_delete(masked_tensors);
        cpl_matrix_delete(coeff);
        cpl_image_delete(image);
        cpl_image_delete(image_double);
        cpl_matrix_unwrap(image_double_wrap);
        cpl_image_unwrap(bkg_image);

    }

    /* save results, destroy whatever is not necessary, return */

    cpl_matrix_delete(weights);
    cpl_matrix_delete(poly_tensors);
    return CPL_ERROR_NONE;
}
/*---------------------------------------------------------------------------*/
/**
 * @brief Create tensor products of Legendre polynomials.
 *
 * @param nx    Number of nodes in the x-direction.
 * @param ny    Number of x nodes in the y-direction.
 * @param npx   Number of tensor products of functions of x.
 * @param npy   Number of tensor products of functions of y.
 *
 * @return The tensor products of Legendre polynomials.
 *
 * The returned matrix must be deallocated using cpl_matrix_delete().
 */
/*---------------------------------------------------------------------------*/
cpl_matrix *hdrl_mime_legendre_tensors_create(int nx, int ny, int npx, int npy)
{
    cpl_matrix *x;
    cpl_matrix *y;
    cpl_matrix *xpolys;
    cpl_matrix *ypolys;
    cpl_matrix *tensors;

    double    ax, ay, bx, by;

/* testing input */
    if (nx < 2 || ny < 2 || npx < 1 || npy < 1)
    {
        cpl_error_set(cpl_func, CPL_ERROR_ILLEGAL_INPUT);
        return NULL;
    }

/* setting parameters:
   nx, ny          numbers of the nodes in the x- and y-direction
   ax, ay, bx, by  endpoints of the intervals with nodes
 */
    ax = 0.0;
    bx = nx - 1.0;

    ay = 0.0;
    by = ny - 1.0;

/* creating equally spaced nodes */
    x = hdrl_mime_matrix_linspace_create(nx, ax, bx);
    y = hdrl_mime_matrix_linspace_create(ny, ay, by);

/* creating the tensor products */
    xpolys = hdrl_mime_legendre_polynomials_create(npx, ax, bx, x);
    ypolys = hdrl_mime_legendre_polynomials_create(npy, ay, by, y);
    tensors = hdrl_mime_linalg_pairwise_column_tensor_products_create(ypolys,
              xpolys);

/* cleaning up */
    cpl_matrix_delete(x);
    cpl_matrix_delete(y);
    cpl_matrix_delete(xpolys);
    cpl_matrix_delete(ypolys);

    return tensors;
}
/*---------------------------------------------------------------------------*/
/**
 * @brief Create equally spaced nodes.
 *
 * @param n   The number of nodes,
 * @param a   The leftmost node,
 * @param b   The rightmost node.
 *
 * @return A column matrix with the equally spaced nodes.
 *
 * The number of nodes @a n must be at least 2.
 */
/*---------------------------------------------------------------------------*/
cpl_matrix *hdrl_mime_matrix_linspace_create(int n, double a, double b)
{
    cpl_matrix *nodes;
    double   *data;
    double    h;
    int       i;

/* testing input */
    if (n < 2)
    {
        cpl_error_set(cpl_func, CPL_ERROR_ILLEGAL_INPUT);
        return NULL;
    }

/* allocating memory */
    nodes = cpl_matrix_new(n, 1);
    data = cpl_matrix_get_data(nodes);

/* creating the nodes */
    h = (b - a) / (n - 1);
    for (i = 0; i < n; i++)
        data[i] = a + i * h;
    data[n - 1] = b;

    return nodes;
}
/*---------------------------------------------------------------------------*/
/**
 * @brief    Create the Legendre polynomial basis on the interval (@a a,@a b).
 *
 * @param    npoly Number of polynomials.
 * @param    a     Left endpoint of the interval.
 * @param    b     Right endpoint of the interval.
 * @param    x     Nodes, at which the polynomials are evaluated.
 *
 * @return   A matrix with the values of the polynomials at @a x.
 *
 * The i-th column contains the values of the i-th polynomial at the
 * given nodes.  The polynomials have degrees 0, 1, ..., npoly-1.  The
 * nodes must lie on the interval [@a a, @a b].  The specific dimensions of
 * the matrix @a x are not used, only its size.
 *
 * The returned matrix must be deallocated using cpl_matrix_delete().
 */
/*---------------------------------------------------------------------------*/
cpl_matrix *hdrl_mime_legendre_polynomials_create(int npoly, double a, double b,
          const cpl_matrix * x)
{
    cpl_matrix *polys;
    double   *m;
    const double *mx;
    double    midpoint, scale;
    int       i, j, nr;

/* testing input */
    if (x == NULL)
    {
        cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
        return NULL;
    }

    if (npoly < 1 || a == b)
    {
        cpl_error_set(cpl_func, CPL_ERROR_ILLEGAL_INPUT);
        return NULL;
    }

/* The specific dimensions of the matrix x are not used, only its size. */
    nr = cpl_matrix_get_nrow(x) * cpl_matrix_get_ncol(x);

/* allocating memory */
    polys = cpl_matrix_new(nr, npoly);

/* initializing */
    midpoint = 0.5 * (a + b);
    scale = 2.0 / (b - a);

/* filling the first column */
    m = cpl_matrix_get_data(polys);
    for (i = 0; i < nr; i++, m += npoly)
        m[0] = 1.0;

/* filling the second column */
    m = cpl_matrix_get_data(polys);
    mx = cpl_matrix_get_data_const(x);
    if (npoly >= 2)
        for (i = 0; i < nr; i++, m += npoly)
            m[1] = scale * (mx[i] - midpoint);

/* filling the remaining columns by recursion
 * j P'_j(x) = (2j-1)xP'_{j-1}(x) - (j-1)P'_{j-2}(x)
 * with P'(x) = P((2x - b - a)/(b - a)) for orthogonality in [-1, 1]->[a, b] */
    m = cpl_matrix_get_data(polys);
    for (i = 0; i < nr; i++, m += npoly)
    {
        double xi = scale * (mx[i] - midpoint);
        for (j = 2; j < npoly; j++)
        {
            double alpha = (2.0 * j - 1.0) / j;
            double beta = (j - 1.0) / j;
            m[j] = alpha * xi * m[j - 1] - beta * m[j - 2];
        }
    }

    return polys;
}
/*---------------------------------------------------------------------------*/
 /**
  * @brief    Create selected pairwise tensor products of the columns of two
  *           matrices.
  *
  * @param    mat1        A matrix,
  * @param    mat2        A matrix.
  *
  * @return   The tensor product of pairs the columns of the two matrices.
  *
  * The tensor product of the j1-th and j2-th columns is created iff
  * j1*(nc2-1) + j2*(nc1-1) <= (nc1-1)*(nc2-1).  The two matrices may
  * have different dimensions.
  */
/*---------------------------------------------------------------------------*/
cpl_matrix *hdrl_mime_linalg_pairwise_column_tensor_products_create(const
          cpl_matrix * mat1, const cpl_matrix * mat2)
{
    cpl_matrix *tensor;
    cpl_matrix *repl1;
    cpl_matrix *repl2;

    int       nc1, nc2;
    int       j1, j2, nc, col_count;

/* testing input */
    if (mat1 == NULL || mat2 == NULL)
    {
        cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
        return NULL;
    }

/* initializing */
    nc1 = cpl_matrix_get_ncol(mat1);
    nc2 = cpl_matrix_get_ncol(mat2);

/* counting the admissible pairs */
    nc = 0;
    for (j1 = 0; j1 < nc1; j1++)
    {
        for (j2 = 0; j2 < nc2; j2++)
            nc += (j1 * (nc2 - 1) + j2 * (nc1 - 1) <=
                      (nc1 - 1) * (nc2 - 1) ? 1 : 0);
    }

/* replicating the columns */
    repl1 = cpl_matrix_new(cpl_matrix_get_nrow(mat1), nc);
    repl2 = cpl_matrix_new(cpl_matrix_get_nrow(mat2), nc);
    col_count = 0;

    for (j1 = 0; j1 < nc1; j1++)
    {
        for (j2 = 0; j2 < nc2; j2++)
        {
            if (j1 * (nc2 - 1) + j2 * (nc1 - 1) <= (nc1 - 1) * (nc2 - 1))
            {
                hdrl_mime_matrix_copy_column(mat1, j1, repl1, col_count);
                hdrl_mime_matrix_copy_column(mat2, j2, repl2, col_count);
                col_count++;
            }
        }
    }

/* filling the matrix with the tensor products */
    tensor = hdrl_mime_linalg_tensor_products_columns_create(repl1, repl2);
    cpl_matrix_delete(repl1);
    cpl_matrix_delete(repl2);

    return tensor;
}
/*---------------------------------------------------------------------------*/
/**
 * @brief    Copy a column from one matrix to another matrix.
 *
 * @param    mat1        The matrix whose column is copied,
 * @param    j_1         The index of the column to be copied,
 * @param[in,out]  mat2        The matrix whose column is overwritten,
 * @param[in,out]  j_2         The index of the column to be overwritten.
 *
 * @return   @c CPL_ERROR_NONE or the appropriate error code.
 *
 * Both matrices must have the same number of rows.
 */
/*---------------------------------------------------------------------------*/
cpl_error_code hdrl_mime_matrix_copy_column(const cpl_matrix * mat1, int j_1,
          cpl_matrix * mat2, int j_2)
{
    const double *m1;
    double   *m2;
    int       i, nr, nc1, nc2;

/* testing input */
    if (mat1 == NULL || mat2 == NULL)
        return cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);

    if (cpl_matrix_get_nrow(mat1) != cpl_matrix_get_nrow(mat2))
        return cpl_error_set(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT);

    if (j_1 < 0 || j_1 >= cpl_matrix_get_ncol(mat1) || j_2 < 0 ||
              j_2 >= cpl_matrix_get_ncol(mat2))
        return cpl_error_set(cpl_func, CPL_ERROR_ILLEGAL_INPUT);

/* initializing */
    nr = cpl_matrix_get_nrow(mat1);
    nc1 = cpl_matrix_get_ncol(mat1);
    nc2 = cpl_matrix_get_ncol(mat2);
    m1 = cpl_matrix_get_data_const(mat1) + j_1;
    m2 = cpl_matrix_get_data(mat2) + j_2;

/* filling the column */
    for (i = 0; i < nr; i++, m1 += nc1, m2 += nc2)
        *m2 = *m1;

    return CPL_ERROR_NONE;
}
/*---------------------------------------------------------------------------*/
 /**
  * @brief    Create the tensor products of the columns of two matrices.
  *
  * @param    mat1        A matrix,
  * @param    mat2        A matrix.
  *
  * @return   The tensor product of the columns of the two matrices.
  *
  * The two matrices must have the same number of columns.  The result
  * has dimensions (nr1*nr2) x nc.
  */
/*---------------------------------------------------------------------------*/
cpl_matrix *hdrl_mime_linalg_tensor_products_columns_create(const cpl_matrix *
          mat1, const cpl_matrix * mat2)
{
    cpl_matrix *tensor;

    const double *m1;
    double   *t;
    int       nr1, nr2, nc;
    int       i1;

/* testing input */
    if (mat1 == NULL || mat2 == NULL)
    {
        cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
        return NULL;
    }

    if (cpl_matrix_get_ncol(mat1) != cpl_matrix_get_ncol(mat2))
    {
        cpl_error_set(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT);
        return NULL;
    }

/* initializing */
    nr1 = cpl_matrix_get_nrow(mat1);
    nr2 = cpl_matrix_get_nrow(mat2);
    nc = cpl_matrix_get_ncol(mat1);     /* nc = cpl_matrix_get_ncol(mat2); */

/* allocating memory */
    tensor = cpl_matrix_new(nr1 * nr2, nc);

/* filling the matrix with the tensor products */
    m1 = cpl_matrix_get_data_const(mat1);
    t = cpl_matrix_get_data(tensor);
    for (i1 = 0; i1 < nr1; i1++, m1 += nc)
    {
        int           i2;
        const double *m2;
        m2 = cpl_matrix_get_data_const(mat2);
        for (i2 = 0; i2 < nr2; i2++, m2 += nc, t += nc)
        {
            int j;
            for (j = 0; j < nc; j++)
                t[j] = m1[j] * m2[j];
        }
    }

    return tensor;
}

/*---------------------------------------------------------------------------*/
/**
 * @brief Create tensor product weights.
 *
 * @param nx    Number of nodes in the x-direction,
 * @param ny    Number of x nodes in the y-direction,
 *
 * @return The tensor weights
 *
 * The returned matrix must be deallocated using cpl_matrix_delete().
 */
/*---------------------------------------------------------------------------*/
cpl_matrix *hdrl_mime_tensor_weights_create(int nx, int ny)
{
    cpl_matrix *x;
    cpl_matrix *y;
    cpl_matrix *weights;

    double   *m;
    double    ax, ay, bx, by, v;
    int       i;

/* testing input */
    if (nx < 2 || ny < 2)
    {
        cpl_error_set(cpl_func, CPL_ERROR_ILLEGAL_INPUT);
        return NULL;
    }

/* setting parameters:
   nx, ny          numbers of the nodes in the x- and y-direction
 */
    bx = 1.0 - 1.0 / nx;
    ax = -bx;

    by = 1.0 - 1.0 / ny;
    ay = -by;

/* creating equally spaced nodes */
    x = hdrl_mime_matrix_linspace_create(nx, ax, bx);
    y = hdrl_mime_matrix_linspace_create(ny, ay, by);

/* creating the weights as the tensor product */
    m = cpl_matrix_get_data(x);
    for (i = 0; i < nx; i++)
    {
        v = m[i];
        v = 1.0 / sqrt(1.0 - v * v);
        m[i] = sqrt(v);
    }

    m = cpl_matrix_get_data(y);
    for (i = 0; i < ny; i++)
    {
        v = m[i];
        v = 1.0 / sqrt(1.0 - v * v);
        m[i] = sqrt(v);
    }

/* switch
     no weights: 1
     with weights: 0  */
    if (1)
    {
        cpl_matrix_fill(x, 1.0);
        cpl_matrix_fill(y, 1.0);
    }

    weights = hdrl_mime_linalg_pairwise_column_tensor_products_create(y, x);

/* cleaning up */
    cpl_matrix_delete(x);
    cpl_matrix_delete(y);

    return weights;
}
/*---------------------------------------------------------------------------*/
/**
 * @brief    Fill matrix rows with zeros as indicated by a mask.
 *
 * @param    mat        A matrix,
 * @param    mask        A mask flagging rows to be filled with 0.0s,
 *
 * @return   @c CPL_ERROR_NONE or the appropriate error code.
 *
 * The size of @a mask must be equal to the number of rows of @a
 * mat.  The rows corresponding to @c CPL_BINARY_1 are set to 0.0.
 */
/*---------------------------------------------------------------------------*/
cpl_error_code hdrl_mime_matrix_mask_rows(cpl_matrix * mat, const cpl_mask * mask)
{
    double   *m;
    const cpl_binary *mk;
    int       i, j, nr, nc;

/* testing input */
    if (mat == NULL || mask == NULL)
        return cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);

    if (cpl_matrix_get_nrow(mat) !=
              cpl_mask_get_size_x(mask) * cpl_mask_get_size_y(mask))
        return cpl_error_set(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT);

/* initializing */
    nr = cpl_matrix_get_nrow(mat);
    nc = cpl_matrix_get_ncol(mat);
    m = cpl_matrix_get_data(mat);
    mk = cpl_mask_get_data_const(mask);

/* updating the rows */
    for (i = 0; i < nr; i++, mk++, m += nc)
    {
        if (*mk == CPL_BINARY_1)
        {
            for (j = 0; j < nc; j++)
                m[j] = 0.0;
        }
    }

    return CPL_ERROR_NONE;
}
/*---------------------------------------------------------------------------*/
/**
 * @brief    Multiply the rows of a matrix by given factors.
 *
 * @param    mat        A matrix,
 * @param    d          The factors.
 * @param[out]   dmat   The  matrix with rescaled rows.
 *
 * @return @c CPL_ERROR_NONE or the appropriate error code.
 *
 * The number of rows must be equal to the size of @a d. The matrix
 * @a dmat must be allocated before calling this function.
 */
/*---------------------------------------------------------------------------*/
cpl_error_code hdrl_mime_matrix_rescale_rows(const cpl_matrix * mat,
          const cpl_matrix * d, cpl_matrix * dmat)
{
    const double *m;
    const double *di;
    double *dm;
    int       i, j, nr, nc;

/* testing input */
    if (mat == NULL || d == NULL || dmat == NULL)
    {
        return cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
    }

    if (cpl_matrix_get_nrow(mat) !=
              cpl_matrix_get_nrow(d) * cpl_matrix_get_ncol(d))
    {
        return cpl_error_set(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT);
    }

    if (cpl_matrix_get_ncol(mat) != cpl_matrix_get_ncol(dmat) ||
              cpl_matrix_get_nrow(mat) != cpl_matrix_get_nrow(dmat))
    {
        return cpl_error_set(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT);
    }

/* initializing */
    nr = cpl_matrix_get_nrow(mat);
    nc = cpl_matrix_get_ncol(mat);

    m = cpl_matrix_get_data_const(mat);
    di = cpl_matrix_get_data_const(d);
    dm = cpl_matrix_get_data(dmat);

/* multiplying the rows by the d[i]'s */
    for (i = 0; i < nr; i++, m += nc, dm += nc)
    {
        for (j = 0; j < nc; j++)
            dm[j] = di[i] * m[j];
    }

    return CPL_ERROR_NONE;
}
/*---------------------------------------------------------------------------*/
/**
 * @brief Solve an overdetermined linear system in the least-squares sense.
 *
 * @param mat   A matrix.
 * @param rhs   A matrix containing right-hand-side vectors.
 * @param alpha The regularization parameter of the Tikhonov method.
 *
 * @return A matrix with solutions of the least-squares problem.
 *
 * Typically, this method is used for overdetermined systems, where
 * the matrix has more rows than columns, but it can also be used for
 * square and underdetermined systems.  Several right-hand-sides can
 * be provided.  The regularization parameter increases with the noise
 * level.
 */
/*---------------------------------------------------------------------------*/
cpl_matrix *hdrl_mime_linalg_solve_tikhonov(const cpl_matrix * mat,
          const cpl_matrix * rhs, double alpha)
{
    cpl_matrix *normal;
    cpl_matrix *solution;
    cpl_error_code error;

/* testing input */
    if (mat == NULL || rhs == NULL)
    {
        cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
        return NULL;
    }

    if (cpl_matrix_get_nrow(mat) != cpl_matrix_get_nrow(rhs))
    {
        cpl_error_set(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT);
        return NULL;
    }

/* creating the normal equations and computing the Cholesky decomposition */
    normal = hdrl_mime_linalg_normal_equations_create(mat, alpha);
    error = cpl_matrix_decomp_chol(normal);

    if (error != CPL_ERROR_NONE)
    {
        cpl_matrix_delete(normal);
        return NULL;
    }

/* solving the normal equations and cleaning up */
    solution = hdrl_mime_matrix_product_left_transpose_create(mat, rhs);
    error = cpl_matrix_solve_chol(normal, solution);
    cpl_matrix_delete(normal);

    if (error != CPL_ERROR_NONE)
    {
        cpl_matrix_delete(solution);
        return NULL;
    }

    return solution;
}
/*---------------------------------------------------------------------------*/
/**
 * @brief Create the matrix transpose(A) * A  + alpha for given A and alpha.
 *
 * @param mat     Matrix,
 * @param alpha   The regularization parameter.
 *
 * @return The matrix transpose(@a mat) * @a mat  + @a alpha.
 *
 * @note Only the upper triangle is computed, since cpl_matrix_decomp_chol()
 *  only requires the upper triangle.
 */
/*---------------------------------------------------------------------------*/
cpl_matrix *hdrl_mime_linalg_normal_equations_create(const cpl_matrix * mat,
          double alpha)
{
    cpl_matrix *normal;
    const double *m;
    double   *p;
    double    sum;
    int       nr, nc;
    int       i, j, k;

/* testing input */
    if (mat == NULL)
    {
        cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
        return NULL;
    }

    if (alpha < 0.0)
    {
        cpl_error_set(cpl_func, CPL_ERROR_ILLEGAL_INPUT);
        return NULL;
    }

/* initializing */
    nr = cpl_matrix_get_nrow(mat);
    nc = cpl_matrix_get_ncol(mat);

/* allocating memory */
    normal = cpl_matrix_new(nc, nc);

/* filling the normal matrix */
    p = cpl_matrix_get_data(normal);
    for (i = 0; i < nc; i++, p += nc)
    {
        for (j = i; j < nc; j++)
        {
            m = cpl_matrix_get_data_const(mat);
            sum = 0.0;
            for (k = 0; k < nr; k++, m += nc)
                sum += m[i] * m[j];
            p[j] = sum;
        }
    }

/* updating the diagonal */
    p = cpl_matrix_get_data(normal);
    for (i = 0; i < nc; i++)
        p[nc * i + i] += alpha;

    return normal;
}
/*---------------------------------------------------------------------------*/
/**
 * @brief    Create the product of the transpose of a matrix with
 * another matrix.
 *
 * @param    mat1       A matrix,
 * @param    mat2       A matrix.
 *
 * @return   The product of the transpose of the first matrix with the second
 *           matrix.
 *
 * The two matrices must have the same number of rows. The product
 * matrix must be deallocated with cpl_matrix_delete().
 */
/*---------------------------------------------------------------------------*/
cpl_matrix *hdrl_mime_matrix_product_left_transpose_create(const cpl_matrix *
          mat1, const cpl_matrix * mat2)
{
    cpl_matrix *product;

    const double *m1;
    const double *m2;
    double   *p;
    double    sum;

    int       nr, nc, common;
    int       i, j, k;

/* testing input */
    if (mat1 == NULL || mat2 == NULL)
    {
        cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
        return NULL;
    }

    if (cpl_matrix_get_nrow(mat1) != cpl_matrix_get_nrow(mat2))
    {
        cpl_error_set(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT);
        return NULL;
    }

/* initializing */
    nr = cpl_matrix_get_ncol(mat1);
    nc = cpl_matrix_get_ncol(mat2);
    common = cpl_matrix_get_nrow(mat1);
    /* common = cpl_matrix_get_nrow(mat2); */

/* allocating memory */
    product = cpl_matrix_new(nr, nc);
    p = cpl_matrix_get_data(product);

/* filling the product matrix */
    for (i = 0; i < nr; i++, p += nc)
    {
        for (j = 0; j < nc; j++)
        {
            m1 = cpl_matrix_get_data_const(mat1);
            m2 = cpl_matrix_get_data_const(mat2);
            sum = 0.0;
            for (k = 0; k < common; k++, m1 += nr, m2 += nc)
                sum += m1[i] * m2[j];
            p[j] = sum;
        }
    }

    return product;
}
/*---------------------------------------------------------------------------*/
/**
 * @brief    Fill a matrix with the product of two given matrices.
 *
 * @param    mat1        A matrix,
 * @param    mat2        A matrix,
 * @param[out]    product    The product of the matrices.
 *
 * @return   @c CPL_ERROR_NONE or the appropriate error code.
 *
 * The number of rows of @a mat1 must be equal to the number of rows
 * of @a product.  The number of columns of @a mat2 must be equal to
 * the number of columns of @a product.  The number of columns of @a
 * mat1 must be equal to the number of rows of @a mat2.
 */
/*---------------------------------------------------------------------------*/
cpl_error_code hdrl_mime_matrix_product(const cpl_matrix * mat1,
          const cpl_matrix * mat2, cpl_matrix * product)
{
    const double *m1;
    const double *m2;
    double   *p;
    double    sum;

    int       nr, nc, common;
    int       i, j, k;

/* testing input */
    if (mat1 == NULL || mat2 == NULL || product == NULL)
    {
        cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
        return CPL_ERROR_NONE;
    }

    if (cpl_matrix_get_ncol(mat1) != cpl_matrix_get_nrow(mat2) ||
              cpl_matrix_get_nrow(mat1) != cpl_matrix_get_nrow(product) ||
              cpl_matrix_get_ncol(mat2) != cpl_matrix_get_ncol(product))
    {
        cpl_error_set(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT);
        return CPL_ERROR_NONE;
    }

/* initializing */
    nr = cpl_matrix_get_nrow(mat1);
    nc = cpl_matrix_get_ncol(mat2);
    common = cpl_matrix_get_ncol(mat1); /* common = cpl_matrix_get_nrow(mat2); */

/* filling the product matrix */
    m1 = cpl_matrix_get_data_const(mat1);
    p = cpl_matrix_get_data(product);
    for (i = 0; i < nr; i++, m1 += cpl_matrix_get_ncol(mat1), p += nc)
    {
        for (j = 0; j < nc; j++)
        {
            m2 = cpl_matrix_get_data_const(mat2);
            sum = 0.0;
            for (k = 0; k < common; k++, m2 += cpl_matrix_get_ncol(mat2))
                sum += m1[k] * m2[j];
            p[j] = sum;
        }
    }

    return CPL_ERROR_NONE;
}



/**@}*/
