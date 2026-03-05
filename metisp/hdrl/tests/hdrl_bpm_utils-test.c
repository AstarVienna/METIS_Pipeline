/*
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

#include "hdrl_bpm_utils.h"
#include "hdrl_image.h"

#include <cpl.h>
#include <math.h>

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_bpm_utils_test    Testing of the HDRL bpm_utils
 */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief Check hdrl_bpm_to_mask() in various conditions
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_bpm_test_bpm_to_mask(void)
{
    cpl_size nx = 20;
    cpl_size ny = 20;
    {
        cpl_mask * mask = hdrl_bpm_to_mask(NULL, 0);
        cpl_test_error(CPL_ERROR_NULL_INPUT);
        cpl_test_null(mask);
    }
    /* non int input */
    {
        cpl_image * bpm = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
        cpl_mask * mask = hdrl_bpm_to_mask(bpm, 0);
        cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
        cpl_test_null(mask);
        cpl_image_delete(bpm);

        /* too big mask */
        bpm = cpl_image_new(nx, ny, CPL_TYPE_INT);
        mask = hdrl_bpm_to_mask(bpm, ~0LLU);
        cpl_test_null(mask);
        cpl_test_error(CPL_ERROR_UNSUPPORTED_MODE);

        cpl_image_delete(bpm);
    }
    /* empty bpm */
    {
        cpl_image * bpm = cpl_image_new(nx, ny, CPL_TYPE_INT);
        cpl_mask * mask = hdrl_bpm_to_mask(bpm, 0);
        cpl_test_nonnull(mask);
        cpl_test_eq(cpl_mask_count(mask), 0);
        cpl_image_delete(bpm);
        cpl_mask_delete(mask);
    }
    /* range of codes */
    {
        cpl_image * bpm = cpl_image_new(nx, ny, CPL_TYPE_INT);
        cpl_image_set(bpm, 1, 1, 1);
        cpl_image_set(bpm, 1, 2, 2);
        cpl_image_set(bpm, 1, 3, 3);
        cpl_image_set(bpm, 1, 4, 4);

        cpl_mask * mask;

        mask = hdrl_bpm_to_mask(bpm, 1);
        cpl_test_nonnull(mask);
        cpl_test_eq(cpl_mask_count(mask), 2);
        cpl_mask_delete(mask);

        mask = hdrl_bpm_to_mask(bpm, ~0u);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_nonnull(mask);
        cpl_test_eq(cpl_mask_count(mask), 4);
        cpl_mask_delete(mask);

        cpl_image_delete(bpm);
    }
    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Check hdrl_mask_to_bpm() in various conditions
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_bpm_test_mask_to_bpm(void)
{
    cpl_size nx = 20;
    cpl_size ny = 20;
    {
        cpl_image * bpm = hdrl_mask_to_bpm(NULL, 0);
        cpl_test_error(CPL_ERROR_NULL_INPUT);
        cpl_test_null(bpm);
    }
    /* empty mask */
    {
        cpl_image * bpm;
        cpl_mask * mask = cpl_mask_new(nx, ny);
        bpm = hdrl_mask_to_bpm(mask, 0);
        cpl_test_nonnull(bpm);
        cpl_test_eq(cpl_image_get_flux(bpm), 0);
        cpl_image_delete(bpm);
        cpl_mask_delete(mask);
    }
    /* non-empty mask */
    {
        cpl_mask * mask = cpl_mask_new(nx, ny);
        cpl_mask_set(mask, 1, 1, CPL_BINARY_1);
        cpl_mask_set(mask, 1, 2, CPL_BINARY_1);
        cpl_mask_set(mask, 1, 3, CPL_BINARY_1);
        cpl_mask_set(mask, 1, 4, CPL_BINARY_1);

        cpl_image * bpm;

        bpm = hdrl_mask_to_bpm(mask, 1);
        cpl_test_nonnull(bpm);
        cpl_test_eq(cpl_image_get_flux(bpm), 4);
        cpl_image_delete(bpm);

        bpm = hdrl_mask_to_bpm(mask, 5);
        cpl_test_nonnull(bpm);
        cpl_test_eq(cpl_image_get_flux(bpm), 5 * 4);
        cpl_image_delete(bpm);

        cpl_mask_delete(mask);
    }
    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Check hdrl_bpm_filter() in various conditions
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_bpm_test_hdrl_bpm_filter(void)
{
    cpl_mask *img_mask = cpl_mask_new(200, 300);

    cpl_mask_set(img_mask,  50,  50, CPL_BINARY_1);
    cpl_mask_set(img_mask, 100, 100, CPL_BINARY_1);
    cpl_mask_set(img_mask, 150, 150, CPL_BINARY_1);
    cpl_mask_set(img_mask, 100, 250, CPL_BINARY_1);
    cpl_mask_set(img_mask, 100, 252, CPL_BINARY_1);
    cpl_mask_set(img_mask, 100, 254, CPL_BINARY_1);
    cpl_mask_set(img_mask, 100, 256, CPL_BINARY_1);
    cpl_mask_set(img_mask, 102, 252, CPL_BINARY_1);
    cpl_mask_set(img_mask, 102, 254, CPL_BINARY_1);
    cpl_mask_set(img_mask, 102, 256, CPL_BINARY_1);
    cpl_mask_set(img_mask, 198, 252, CPL_BINARY_1);
    cpl_mask_set(img_mask, 198, 254, CPL_BINARY_1);
    cpl_mask_set(img_mask, 198, 256, CPL_BINARY_1);
    cpl_mask_set(img_mask, 200, 252, CPL_BINARY_1);
    cpl_mask_set(img_mask, 200, 254, CPL_BINARY_1);
    cpl_mask_set(img_mask, 200, 256, CPL_BINARY_1);
    cpl_mask_set(img_mask, 199, 300, CPL_BINARY_1);
    cpl_mask_set(img_mask, 199, 299, CPL_BINARY_1);
    cpl_mask_set(img_mask, 199, 298, CPL_BINARY_1);
    cpl_mask_set(img_mask, 200, 300, CPL_BINARY_1);
    cpl_mask_set(img_mask, 200, 299, CPL_BINARY_1);
    cpl_mask_set(img_mask, 200, 298, CPL_BINARY_1);
    /*Test the border behaviour*/
    cpl_mask_set(img_mask, 199, 200, CPL_BINARY_1);
    cpl_mask_set(img_mask, 199, 198, CPL_BINARY_1);


    /*cpl_mask_save(img_mask, "img_mask.fits", NULL, CPL_IO_CREATE);*/

    {
        cpl_mask *filtered_mask = hdrl_bpm_filter(img_mask, 3, 3, CPL_FILTER_CLOSING);

        cpl_test_eq(cpl_mask_get(filtered_mask, 100, 255),  CPL_BINARY_1);
        cpl_test_eq(cpl_mask_get(filtered_mask, 101, 255),  CPL_BINARY_1);
        cpl_test_eq(cpl_mask_get(filtered_mask, 102, 255),  CPL_BINARY_1);
        cpl_test_eq(cpl_mask_get(filtered_mask, 103, 255),  CPL_BINARY_0);

        cpl_test_eq(cpl_mask_get(filtered_mask, 100, 251),  CPL_BINARY_1);

        cpl_test_eq(cpl_mask_get(filtered_mask, 198, 255),  CPL_BINARY_1);
        cpl_test_eq(cpl_mask_get(filtered_mask, 199, 255),  CPL_BINARY_1);
        cpl_test_eq(cpl_mask_get(filtered_mask, 200, 255),  CPL_BINARY_1);

        cpl_test_eq(cpl_mask_get(filtered_mask, 198, 254),  CPL_BINARY_1);
        cpl_test_eq(cpl_mask_get(filtered_mask, 199, 254),  CPL_BINARY_1);
        cpl_test_eq(cpl_mask_get(filtered_mask, 200, 254),  CPL_BINARY_1);

        /*Test the border behaviour*/
        cpl_test_eq(cpl_mask_get(filtered_mask, 200, 199),  CPL_BINARY_0);

         /*cpl_mask_save(filtered_mask, "filtered_closing_mask.fits", NULL,
                      CPL_IO_CREATE);*/
        cpl_mask_delete(filtered_mask);
    }

    /* free the memory */
    cpl_mask_delete(img_mask);

    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Can be used to test additional bad pixel growing algoritms 
  @return cpl_error_code
  NOT TRIGGERED
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_bpm_test_bpmgrow(void)
{
	const char* img_mask_name              = "img_mask.fits";
	const char* filtered_morpho_mask_name  = "filtered_morpho_mask.fits";
	const char* filtered_average_mask_name = "filtered_average_mask.fits";
	const char* file_gauss_name            = "gauss.fits";
	const char* filtered_gauss_data_name   = "filtered_gauss_data.fits";
	const char* filtered_gauss_mask_name   = "filtered_gauss_mask.fits";


    cpl_mask *img_mask = cpl_mask_new(200, 300);

    cpl_mask_set(img_mask,  50,  50, CPL_BINARY_1);
    cpl_mask_set(img_mask, 100, 100, CPL_BINARY_1);
    cpl_mask_set(img_mask, 150, 150, CPL_BINARY_1);
    cpl_mask_set(img_mask, 100, 250, CPL_BINARY_1);
    cpl_mask_set(img_mask, 100, 252, CPL_BINARY_1);
    cpl_mask_set(img_mask, 100, 254, CPL_BINARY_1);
    cpl_mask_set(img_mask, 100, 256, CPL_BINARY_1);
    cpl_mask_set(img_mask, 102, 252, CPL_BINARY_1);
    cpl_mask_set(img_mask, 102, 254, CPL_BINARY_1);
    cpl_mask_set(img_mask, 102, 256, CPL_BINARY_1);
    cpl_mask_set(img_mask, 198, 252, CPL_BINARY_1);
    cpl_mask_set(img_mask, 198, 254, CPL_BINARY_1);
    cpl_mask_set(img_mask, 198, 256, CPL_BINARY_1);
    cpl_mask_set(img_mask, 200, 252, CPL_BINARY_1);
    cpl_mask_set(img_mask, 200, 254, CPL_BINARY_1);
    cpl_mask_set(img_mask, 200, 256, CPL_BINARY_1);
    cpl_mask_set(img_mask, 199, 300, CPL_BINARY_1);
    cpl_mask_set(img_mask, 199, 299, CPL_BINARY_1);
    cpl_mask_set(img_mask, 199, 298, CPL_BINARY_1);
    cpl_mask_set(img_mask, 200, 300, CPL_BINARY_1);
    cpl_mask_set(img_mask, 200, 299, CPL_BINARY_1);
    cpl_mask_set(img_mask, 200, 298, CPL_BINARY_1);

    cpl_mask_save(img_mask, "img_mask.fits", NULL, CPL_IO_CREATE);

    if(1){
          /* Set all pixels to bad, if there are a predefined number of bad
           * pixels in the neighborhood - it uses the morpho filter which
           * is much slower than e.g. the CPL_FILTER_AVERAGE_FAST filter */

        cpl_matrix * kernel = cpl_matrix_new(3, 3);
        cpl_matrix_fill(kernel, 1.0);
        cpl_image * result_data = cpl_image_new_from_mask(img_mask);

        cpl_image * filtered_data = cpl_image_new(
                        cpl_image_get_size_x(result_data),
                        cpl_image_get_size_y(result_data),
                        CPL_TYPE_FLOAT);

        cpl_image_filter(filtered_data, result_data, kernel,
                         CPL_FILTER_MORPHO_SCALE, CPL_BORDER_FILTER);


        cpl_mask * filtered_mask = cpl_mask_threshold_image_create(
                        filtered_data, 3.-0.5, DBL_MAX);

        cpl_mask_save(filtered_mask, filtered_morpho_mask_name, NULL,
                      CPL_IO_CREATE);

        cpl_mask_delete(filtered_mask);
        cpl_image_delete(result_data);
        cpl_image_delete(filtered_data);
        cpl_matrix_delete(kernel);

    }

    if(1){
        /* Set all pixels to bad, if there are a predefined number of bad
         * pixels in the neighborhood - it uses the CPL_FILTER_AVERAGE_FAST
         * filter. This filter is fast but shrinks the window at the border.
         * Therefore a simple scaling to the number of bad pixels in the
         * neighborhood (nx * ny * average) can not be done at the image-border.
         * Nevertheless one can never detect less, but only more neighboring
         * bad pixels near the border (the bad pixel density increases at the
         * border as the windows shrinks) -
         * so it would be a conservative approach.
         *
         *  */

        cpl_mask * kernel = cpl_mask_new(3, 3);
        cpl_mask_not(kernel); /* All values set to unity*/


        cpl_image * result_data = cpl_image_new_from_mask(img_mask);

        cpl_image * filtered_data = cpl_image_new(
                        cpl_image_get_size_x(result_data),
                        cpl_image_get_size_y(result_data),
                        CPL_TYPE_FLOAT);



        cpl_image_filter_mask(filtered_data, result_data, kernel,
                              CPL_FILTER_AVERAGE_FAST, CPL_BORDER_FILTER);



        cpl_mask * filtered_mask = cpl_mask_threshold_image_create(
                        filtered_data, (3.-0.5)/(3.*3.), DBL_MAX);


        cpl_mask_save(filtered_mask, filtered_average_mask_name, NULL, CPL_IO_CREATE);

        cpl_mask_delete(filtered_mask);
        cpl_image_delete(result_data);
        cpl_image_delete(filtered_data);
        cpl_mask_delete(kernel);

    }

    if(1){
        /* This algo first smoothed the bad pixels by a gaussian kernel and
         * then one can use a threshold to detect new bad pixels on the
         * smoothed image. Here it is difficult to find good parameters for the
         * gaussian and the subsequent thresholding.
         *  */

        double sig_x = 3.;   /*Sigma in x for the gaussian distribution.*/
        double sig_y = 3.;   /*Sigma in y for the gaussian distribution.*/


        /* creating the Gaussian kernel */
        cpl_size n = 5;
        cpl_image * gauss = cpl_image_new(2 * n + 1, 2 * n + 1, CPL_TYPE_DOUBLE);

        cpl_image_fill_gaussian(gauss, n + 1, n + 1, (double)121.0, sig_x, sig_y);

        /* filtering the image */
        cpl_matrix * kernel = cpl_matrix_wrap(2 * n + 1, 2 * n + 1,
                                     cpl_image_get_data_double(gauss));
        /*cpl_image_filter(im1, im2, gauss_data, CPL_FILTER_LINEAR,
                         CPL_BORDER_FILTER);*/

        /*cpl_matrix * kernel = cpl_matrix_new(3, 3);
        cpl_matrix_fill(kernel, 1.0);*/

        cpl_image * result_data = cpl_image_new_from_mask(img_mask);

        cpl_image * filtered_data = cpl_image_new(
                        cpl_image_get_size_x(result_data),
                        cpl_image_get_size_y(result_data),
                        CPL_TYPE_DOUBLE);

        cpl_image_filter(filtered_data, result_data, kernel,
                         CPL_FILTER_LINEAR, CPL_BORDER_FILTER);


        cpl_mask * filtered_mask = cpl_mask_threshold_image_create(
                        filtered_data, 3.-0.5, DBL_MAX);

        cpl_image_save(filtered_data, filtered_gauss_data_name,
                       CPL_TYPE_DOUBLE, NULL, CPL_IO_CREATE);
        cpl_image_save(gauss, file_gauss_name,
                       CPL_TYPE_DOUBLE, NULL, CPL_IO_CREATE);

        cpl_mask_save(filtered_mask, filtered_gauss_mask_name, NULL, CPL_IO_CREATE);

        cpl_matrix_unwrap(kernel);
        cpl_image_delete(gauss);
        cpl_mask_delete(filtered_mask);
        cpl_image_delete(result_data);
        cpl_image_delete(filtered_data);
    }

    /* free the memory */
    cpl_mask_delete(img_mask);

    /* Remove to disk */
    remove(img_mask_name);
    remove(filtered_morpho_mask_name);
    remove(filtered_average_mask_name);
    remove(file_gauss_name);
    remove(filtered_gauss_data_name);
    remove(filtered_gauss_mask_name);

    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Can be used to test how to apply masks to imagelist
  @return cpl_error_code
  NOT TRIGGERED
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_bpm_test_apply_masks_to_imagelist(void)
{
	#define NUM_IMAGES 2

	int nx = 64;
	int ny = 64;


	/* Create a imagelist */
	cpl_image     *img  = cpl_image_fill_test_create(nx, ny);
	cpl_imagelist *list = cpl_imagelist_new();
    for (cpl_size i=0; i < NUM_IMAGES; i++) {
        cpl_imagelist_set(list, cpl_image_duplicate(img), i);
    }
    cpl_image_delete(img);


	/* join masks -> fill with a new allocated mask the vector*/
    cpl_mask *new_mask = cpl_mask_new(nx, ny);
	cpl_mask **orig_masks;
	hdrl_join_mask_on_imagelist(list, new_mask, &orig_masks);
	cpl_mask_delete(new_mask);


	/* restore original mask */
	hdrl_set_masks_on_imagelist(list, orig_masks);


	/* free memory */
	cpl_imagelist_delete(list);
	for (cpl_size i = 0; i < NUM_IMAGES; i++) {
		cpl_mask_delete(orig_masks[i]);
	}
	cpl_free(orig_masks);

	return CPL_ERROR_NONE;
}


/*----------------------------------------------------------------------------*/
/**
  @brief   Unit tests of BPM module
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    hdrl_bpm_test_bpm_to_mask();
    hdrl_bpm_test_mask_to_bpm();
    hdrl_bpm_test_hdrl_bpm_filter();
    hdrl_bpm_test_bpmgrow() ;
    hdrl_bpm_test_apply_masks_to_imagelist();

    cpl_test_error(CPL_ERROR_NONE);

    return cpl_test_end(0);
}
