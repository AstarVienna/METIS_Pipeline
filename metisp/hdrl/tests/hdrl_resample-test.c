/*
 * This file is part of the HDRL
 * Copyright (C) 2016 European Southern Observatory
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
#include <cpl.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <hdrl_utils.h>
#include <hdrl_resample.h>
#include <hdrl_resample.c>
#include <hdrl_random.h>
#ifndef _OPENMP
#define omp_get_max_threads() 1
#else
#include <omp.h>
#endif

/* The following code contains several unit tests to cover the hdrl_resample.c
 * library. As we would like that any function has its own unit test for easy
 * we used a naming convention as follow:
 *
 * 'test_function()' tests the function 'function()';
 *
 * There are a few exceptions: "in a few cases it was more convenient to have
 * a single function testing several library functions
 * Moreover we have some additional unit tests, testing special functionality
 * which follows a different naming convention.
 *
 */
/* -------------------------------- DEFINES --------------------------------- */
#define RECIPE_NAME "hdrldemo_resample"

/*Fine-grained accuracies*/
#define HDRL_DELTA_COMPARE_VALUE_ABS  CPL_MAX(HDRL_EPS_DATA, HDRL_EPS_ERROR) * 4.

#define HDRL_EPS_TEST       HDRL_EPS_DATA
/* For resampling method definition */
#define DRIZZLE_DOWN_SCALING_FACTOR_X 0.8
#define DRIZZLE_DOWN_SCALING_FACTOR_Y 0.8
#define DRIZZLE_DOWN_SCALING_FACTOR_Z 0.8
#define RENKA_CRITICAL_RADIUS         1.25
#define LANCZOS_KERNEL_SIZE 2
#define LOOP_DISTANCE                 1.0
/* Image value */
#define HDRL_FLUX_ADU 100
/* For WCS definition From SINFONI cube example 1:
 * SINFO.2005-08-22T07:47:54.305.fits */
#define HDRL_SCALE_Z 500
#define HDRL_CD11   -3.47222e-05
#define HDRL_CD12   0.0
#define HDRL_CD21   0.0
#define HDRL_CD22   3.47222e-05
#define HDRL_CD13   0.0
#define HDRL_CD31   0.0
#define HDRL_CD23   0.0
#define HDRL_CD32   0.0
#define HDRL_CD33   2.45e-10*HDRL_SCALE_Z

#define HDRL_CDELT1 fabs(HDRL_CD11)
#define HDRL_CDELT2 fabs(HDRL_CD22)
#define HDRL_CDELT3 fabs(HDRL_CD33)

#define HDRL_CRPIX1 33.5
#define HDRL_CRPIX2 33.5
#define HDRL_CRPIX3 1.

#define HDRL_CRVAL1 48.0706
#define HDRL_CRVAL2 -20.6219
#define HDRL_CRVAL3 1.9283e-06

#define HDRL_RA        48.070
#define HDRL_DEC         -20.621
#define HDRL_RA_MIN     48.069416667
#define HDRL_RA_MAX     48.0718125
#define HDRL_DEC_MIN    -20.6229925
#define HDRL_DEC_MAX    -20.620708611

#define HDRL_LAMBDA_MIN 1.9283e-06
#define HDRL_LAMBDA_MAX 2.47146e-06

/* Image sizes */
#define HDRL_SIZE_X 50
#define HDRL_SIZE_Y 50
#define HDRL_SIZE_Z 3


/*----------------------------------------------------------------------------*/
/**
    @brief creates hdrl_image from input data/error/bpm
    @return hdrl_image
 */
/*----------------------------------------------------------------------------*/
static hdrl_image*
hdrl_resample_util_hdrl_image_create(cpl_image* data, cpl_image* error,
				     cpl_image* bpm){
  if(bpm != NULL) {
      cpl_mask* mask = cpl_mask_threshold_image_create(bpm, 0, INT_MAX);
      cpl_image_reject_from_mask(data, mask);
      cpl_mask_delete(mask);
  }
  hdrl_image* hima = hdrl_image_create(data, error);

  return hima;
}

/*----------------------------------------------------------------------------*/
/**
    @brief Check hdrl_resample_compute() in various conditions for 2D case
    @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code test_hdrl_resample_compute2D_multiple(void)
{

  /* Data background always 1 and inner 9 pixel 49 - for error tracing*/

  cpl_image * img_data = cpl_image_new(9, 9, CPL_TYPE_DOUBLE);
  cpl_image * img_bpm  = cpl_image_new(9, 9, CPL_TYPE_INT);

  /* Set all data invalid */
  cpl_image_add_scalar(img_bpm, 1.);

  /* Set data in the center of the image  */
  cpl_image_set(img_data, 4, 4, 48.);
  cpl_image_set(img_data, 5, 4, 48.);
  cpl_image_set(img_data, 6, 4, 48.);
  cpl_image_set(img_data, 4, 5, 48.);
  cpl_image_set(img_data, 5, 5, 48.);
  cpl_image_set(img_data, 6, 5, 48.);
  cpl_image_set(img_data, 4, 6, 48.);
  cpl_image_set(img_data, 5, 6, 48.);
  cpl_image_set(img_data, 6, 6, 48.);
  /* Adding 1 and creating the errors */
  cpl_image_add_scalar(img_data, 1.0);
  cpl_image * img_error=cpl_image_power_create(img_data, 0.5);

  /* Set data in the center as valid */
  cpl_image_set(img_bpm, 4, 4, 0);
  cpl_image_set(img_bpm, 5, 4, 0);
  cpl_image_set(img_bpm, 6, 4, 0);
  cpl_image_set(img_bpm, 4, 5, 0);
  cpl_image_set(img_bpm, 5, 5, 0);
  cpl_image_set(img_bpm, 6, 5, 0);
  cpl_image_set(img_bpm, 4, 6, 0);
  cpl_image_set(img_bpm, 5, 6, 0);
  cpl_image_set(img_bpm, 6, 6, 0);

  /*Build the header for the wcs */
  cpl_propertylist* plist = cpl_propertylist_new();

  cpl_propertylist_append_int(plist, "NAXIS"  , 2);
  cpl_propertylist_append_int(plist, "NAXIS1" , 9);
  cpl_propertylist_append_int(plist, "NAXIS2" , 9);
  cpl_propertylist_append_double(plist, "CD1_1"  , -0.01      );
  cpl_propertylist_append_double(plist, "CD1_2"  , 0.        );
  cpl_propertylist_append_double(plist, "CD2_1"  , 0.        );
  cpl_propertylist_append_double(plist, "CD2_2"  , 0.01       );
  cpl_propertylist_append_double(plist, "CRPIX1" , 4.5       );
  cpl_propertylist_append_double(plist, "CRPIX2" , 4.5       );
  cpl_propertylist_append_double(plist, "CRVAL1" , 359.8     );
  cpl_propertylist_append_double(plist, "CRVAL2" , 10.0      );
  cpl_propertylist_append_string(plist, "CTYPE1" , "RA---TAN");
  cpl_propertylist_append_string(plist, "CTYPE2" , "DEC--TAN");
  cpl_propertylist_append_string(plist, "CUNIT1" , "deg"     );
  cpl_propertylist_append_string(plist, "CUNIT2" , "deg"     );

  hdrl_random_state * rastate = hdrl_random_state_new(1, NULL);
  hdrl_random_state * decstate = hdrl_random_state_new(1, NULL);

  /* Please be aware on the different accuracies set for the errors for the
   * different methods */

  /* generate hdrl image */
  hdrl_image*
  hima = hdrl_resample_util_hdrl_image_create(img_data, img_error, img_bpm);
  const cpl_size iterations = 500;
  for (cpl_size i = 0; i < iterations; i++) {
      double rarandom = hdrl_random_uniform_double(rastate, 0., 360.);
      double decrandom = hdrl_random_uniform_double(decstate, -89., 89.);
      cpl_msg_info(cpl_func,"ra-random: %g, dec-random: %g ", rarandom,
		   decrandom);
      cpl_propertylist_update_double(plist, "CRVAL1" , rarandom);
      cpl_propertylist_update_double(plist, "CRVAL2" , decrandom);
      cpl_wcs * wcs = cpl_wcs_new_from_propertylist(plist);

      /* Construct the table */
      cpl_table * table = hdrl_resample_image_to_table(hima, wcs);

      hdrl_parameter *aParams_outputgrid = NULL;

      /* Define the output grid */
      aParams_outputgrid = hdrl_resample_parameter_create_outgrid2D(0.01, 0.01);
      hdrl_resample_result *result = NULL;
      hdrl_parameter *aParams_method = NULL;
      int rej;

      /* ---------------------------lanczos------------------------------- */

      /* Define the method */
      aParams_method = hdrl_resample_parameter_create_lanczos(1, CPL_FALSE, 2);
      /* Do the resampling */
      result = hdrl_resample_compute(table, aParams_method, aParams_outputgrid, wcs);
      cpl_test_error(CPL_ERROR_NONE);
      cpl_test_rel(cpl_image_get(hdrl_image_get_image_const(hdrl_imagelist_get_const(result->himlist, 0)),
				 5, 5, &rej), 49., 1e-6);
      cpl_test_rel(cpl_image_get(hdrl_image_get_error_const(hdrl_imagelist_get_const(result->himlist, 0)),
				 5, 5, &rej), 7., 0.05);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 0)),
			       5, 5), 0);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 0)),
			       1, 1), 1);
      hdrl_parameter_delete(aParams_method);
      hdrl_resample_result_delete(result);

      /* -----------------------------drizzle----------------------------- */

      /* Define the method */
      aParams_method = hdrl_resample_parameter_create_drizzle(1, CPL_FALSE,
							      0.8, 0.8, 0.8);
      /* Do the resampling */

      result = hdrl_resample_compute(table, aParams_method, aParams_outputgrid, wcs);
      cpl_test_error(CPL_ERROR_NONE);
      cpl_test_rel(cpl_image_get(hdrl_image_get_image_const(hdrl_imagelist_get_const(result->himlist, 0)),
				 5, 5, &rej), 49., 1e-6);
      cpl_test_rel(cpl_image_get(hdrl_image_get_error_const(hdrl_imagelist_get_const(result->himlist, 0)),
				 5, 5, &rej), 7., 0.001);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 0)),
			       5, 5), 0);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 0)),
			       1, 1), 1);

      hdrl_parameter_delete(aParams_method);
      hdrl_resample_result_delete(result);

      /* -----------------------------linear------------------------------ */

      /* Define the method */
      aParams_method = hdrl_resample_parameter_create_linear(1, CPL_FALSE);
      /* Do the resampling */

      result = hdrl_resample_compute(table, aParams_method, aParams_outputgrid, wcs);
      cpl_test_error(CPL_ERROR_NONE);
      cpl_test_rel(cpl_image_get(hdrl_image_get_image_const(hdrl_imagelist_get_const(result->himlist, 0)),
				 5, 5, &rej), 49., 1e-6);
      cpl_test_rel(cpl_image_get(hdrl_image_get_error_const(hdrl_imagelist_get_const(result->himlist, 0)),
				 5, 5, &rej), 7., 0.3);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 0)),
			       5, 5), 0);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 0)),
			       1, 1), 1);

      hdrl_parameter_delete(aParams_method);
      hdrl_resample_result_delete(result);

      /* -----------------------------quadratic--------------------------- */

      /* Define the method */
      aParams_method = hdrl_resample_parameter_create_quadratic(1, CPL_FALSE);
      /* Do the resampling */

      result = hdrl_resample_compute(table, aParams_method, aParams_outputgrid, wcs);
      cpl_test_error(CPL_ERROR_NONE);
      cpl_test_rel(cpl_image_get(hdrl_image_get_image_const(hdrl_imagelist_get_const(result->himlist, 0)),
				 5, 5, &rej), 49., 1e-6);
      cpl_test_rel(cpl_image_get(hdrl_image_get_error_const(hdrl_imagelist_get_const(result->himlist, 0)),
				 5, 5, &rej), 7., 0.02);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 0)),
			       5, 5), 0);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 0)),
			       1, 1), 1);

      hdrl_parameter_delete(aParams_method);
      hdrl_resample_result_delete(result);

      /* -----------------------------renka------------------------------- */

      /* Define the method */
      aParams_method = hdrl_resample_parameter_create_renka(1, CPL_FALSE,
							    1.25);
      /* Do the resampling */

      result = hdrl_resample_compute(table, aParams_method, aParams_outputgrid, wcs);
      cpl_test_error(CPL_ERROR_NONE);
      cpl_test_rel(cpl_image_get(hdrl_image_get_image_const(hdrl_imagelist_get_const(result->himlist, 0)),
				 5, 5, &rej), 49., 1e-6);
      cpl_test_rel(cpl_image_get(hdrl_image_get_error_const(hdrl_imagelist_get_const(result->himlist, 0)),
				 5, 5, &rej), 7., 0.01);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 0)),
			       5, 5), 0);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 0)),
			       1, 1), 1);

      hdrl_parameter_delete(aParams_method);
      hdrl_resample_result_delete(result);

      /* -----------------------------nearest----------------------------- */

      /* Define the method */
      aParams_method = hdrl_resample_parameter_create_nearest();
      /* Do the resampling */
      result = hdrl_resample_compute(table, aParams_method, aParams_outputgrid, wcs);
      cpl_test_error(CPL_ERROR_NONE);
      cpl_test_rel(cpl_image_get(hdrl_image_get_image_const(hdrl_imagelist_get_const(result->himlist, 0)),
				 5, 5, &rej), 49., 1e-6);
      cpl_test_rel(cpl_image_get(hdrl_image_get_error_const(hdrl_imagelist_get_const(result->himlist, 0)),
				 5, 5, &rej), 7., 0.00001);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 0)),
			       5, 5), 0);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 0)),
			       1, 1), 1);

      hdrl_parameter_delete(aParams_method);
      hdrl_resample_result_delete(result);

      /* ----------------------------------------------------------------- */

      hdrl_parameter_delete(aParams_outputgrid);
      cpl_table_delete(table);
      cpl_wcs_delete(wcs);
  } /* Loop ends here */
  hdrl_image_delete(hima);

  /* ---------------- For manual updates and tests ----------------------- */

  /* Test image spanning over ra 360 degree */
  cpl_propertylist_update_double(plist, "CRVAL1" , 0.01);
  cpl_propertylist_update_double(plist, "CRVAL2" , 20.1);

  /* Test image rotated by 45 degree */
  cpl_propertylist_update_double(plist, "CD1_1"  , cos(45)        );
  cpl_propertylist_update_double(plist, "CD1_2"  , -sin(45)        );
  cpl_propertylist_update_double(plist, "CD2_1"  , sin(45)        );
  cpl_propertylist_update_double(plist, "CD2_2"  , cos(45)        );


  cpl_wcs * wcs = cpl_wcs_new_from_propertylist(plist);

  /* Construct the table */
  hima = hdrl_resample_util_hdrl_image_create(img_data, img_error, img_bpm);

  cpl_table * table = hdrl_resample_image_to_table(hima, wcs);

  hdrl_image_delete(hima);
  hdrl_parameter *aParams_outputgrid = NULL;

  /* Define the output grid */
  aParams_outputgrid = hdrl_resample_parameter_create_outgrid2D(cos(45) * 3., cos(45) * 3.);

  hdrl_resample_result *result = NULL;

  hdrl_parameter *aParams_method = NULL;
  int rej;

  /* Define the method */
  aParams_method = hdrl_resample_parameter_create_lanczos(2, CPL_FALSE, 2);
  /* Do the resampling */
  result = hdrl_resample_compute(table, aParams_method, aParams_outputgrid, wcs);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_rel(cpl_image_get(hdrl_image_get_image_const(hdrl_imagelist_get_const(result->himlist, 0)),
			     5, 5, &rej), 49., 1e-6);
  cpl_test_rel(cpl_image_get(hdrl_image_get_error_const(hdrl_imagelist_get_const(result->himlist, 0)),
			     5, 5, &rej), 7., 0.3);
  cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 0)),
			   5, 5), 0);
  cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 0)),
			   1, 1), 1);

  /*Save the original and resampled image/error/bpm*/
  cpl_image_save(img_data, "image_orig.fits", CPL_TYPE_FLOAT, plist,
		 CPL_IO_CREATE);

  cpl_image_save(img_error, "image_orig.fits", CPL_TYPE_FLOAT, plist,
		 CPL_IO_EXTEND);

  cpl_image_save(img_bpm, "image_orig.fits", CPL_TYPE_INT, plist,
		 CPL_IO_EXTEND);

  cpl_image_save(hdrl_image_get_image(hdrl_imagelist_get(result->himlist, 0)), "image_resampled.fits",
		 CPL_TYPE_FLOAT, result->header, CPL_IO_CREATE);
  cpl_image_save(hdrl_image_get_error(hdrl_imagelist_get(result->himlist, 0)), "image_resampled.fits",
		 CPL_TYPE_FLOAT, result->header, CPL_IO_EXTEND);
  cpl_mask_save(hdrl_image_get_mask(hdrl_imagelist_get(result->himlist, 0)), "image_resampled.fits",
		result->header, CPL_IO_EXTEND);

  /*Print the wcs from the original and resampled cube*/
  hdrl_resample_wcs_print(wcs);

  cpl_wcs_delete(wcs);
  wcs = cpl_wcs_new_from_propertylist(result->header);
  hdrl_resample_wcs_print(wcs);

  hdrl_parameter_delete(aParams_method);
  hdrl_resample_result_delete(result);
  hdrl_parameter_delete(aParams_outputgrid);
  cpl_table_delete(table);
  cpl_wcs_delete(wcs);

  /* --------------------------------------------------------------------- */

  /* free the remaining memory */
  cpl_propertylist_delete(plist);
  cpl_image_delete(img_data);
  cpl_image_delete(img_error);
  cpl_image_delete(img_bpm);
  hdrl_random_state_delete(rastate);
  hdrl_random_state_delete(decstate);

  cpl_test_error(CPL_ERROR_NONE);
  return cpl_error_get_code();
}

static hdrl_imagelist*
hdrl_resample_util_hdrl_imagelist_create(cpl_imagelist* dlist,
					 cpl_imagelist* elist, cpl_imagelist* qlist) {

  cpl_ensure(dlist != NULL, CPL_ERROR_NULL_INPUT, NULL);
  cpl_size size = cpl_imagelist_get_size(dlist);
  if(qlist != NULL) {
      for(cpl_size k = 0; k < size; k++) {
	  cpl_image* data = cpl_imagelist_get(dlist, k);

	  cpl_image* qual = cpl_imagelist_get(qlist, k);

	  /*we use INT_MAX instead of 1.1 as some pipeline
	   * may use pixel codes as qualifier */
	  cpl_mask* mask = cpl_mask_threshold_image_create(qual, 0, INT_MAX);

	  cpl_image_reject_from_mask(data, mask);
	  cpl_mask_delete(mask);
	  cpl_imagelist_set(dlist, data, k);
      }
  }
  hdrl_imagelist* hlist = hdrl_imagelist_create(dlist, elist);

  return hlist;

}
/*----------------------------------------------------------------------------*/
/**
  @brief Check hdrl_resample_compute() in various conditionsf or 3D case
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code test_hdrl_resample_compute3D_multiple(void)
{
  cpl_test_error(CPL_ERROR_NONE);
  cpl_imagelist *     imglist_data = cpl_imagelist_new();
  cpl_imagelist *     imglist_error = cpl_imagelist_new();
  cpl_imagelist *     imglist_bpm = cpl_imagelist_new();
  cpl_test_error(CPL_ERROR_NONE);

  /* Data background always 0 and inner 9 pixel ramp up from 1 to 50 and then
   * back to 1 */
  cpl_test_error(CPL_ERROR_NONE);
  for (cpl_size var = 0; var <99 ; ++var) {
      cpl_image * data = cpl_image_new(9, 9, CPL_TYPE_DOUBLE);
      cpl_image * bpm  = cpl_image_new(9, 9, CPL_TYPE_INT);
      /* Set all data invalid */
      cpl_image_add_scalar(bpm, 1.);

      if(var < 50) {
	  cpl_image_set(data, 4, 4, var + 1.);
	  cpl_image_set(data, 5, 4, var + 1.);
	  cpl_image_set(data, 6, 4, var + 1.);
	  cpl_image_set(data, 4, 5, var + 1.);
	  cpl_image_set(data, 5, 5, var + 1.);
	  cpl_image_set(data, 6, 5, var + 1.);
	  cpl_image_set(data, 4, 6, var + 1.);
	  cpl_image_set(data, 5, 6, var + 1.);
	  cpl_image_set(data, 6, 6, var + 1.);
	  cpl_image * errors=cpl_image_power_create(data, 0.5);

	  /* Set data in the center as valid */
	  cpl_image_set(bpm, 4, 4, 0);
	  cpl_image_set(bpm, 5, 4, 0);
	  cpl_image_set(bpm, 6, 4, 0);
	  cpl_image_set(bpm, 4, 5, 0);
	  cpl_image_set(bpm, 5, 5, 0);
	  cpl_image_set(bpm, 6, 5, 0);
	  cpl_image_set(bpm, 4, 6, 0);
	  cpl_image_set(bpm, 5, 6, 0);
	  cpl_image_set(bpm, 6, 6, 0);

	  cpl_imagelist_set(imglist_data, data, var);
	  cpl_imagelist_set(imglist_error, errors, var);
	  cpl_imagelist_set(imglist_bpm, bpm, var);
      }
      if(var >= 50) {
	  cpl_image_set(data, 4, 4, 99. - var);
	  cpl_image_set(data, 5, 4, 99. - var);
	  cpl_image_set(data, 6, 4, 99. - var);
	  cpl_image_set(data, 4, 5, 99. - var);
	  cpl_image_set(data, 5, 5, 99. - var);
	  cpl_image_set(data, 6, 5, 99. - var);
	  cpl_image_set(data, 4, 6, 99. - var);
	  cpl_image_set(data, 5, 6, 99. - var);
	  cpl_image_set(data, 6, 6, 99. - var);
	  cpl_image * errors=cpl_image_power_create(data, 0.5);

	  /* Set data in the center as valid */
	  cpl_image_set(bpm, 4, 4, 0);
	  cpl_image_set(bpm, 5, 4, 0);
	  cpl_image_set(bpm, 6, 4, 0);
	  cpl_image_set(bpm, 4, 5, 0);
	  cpl_image_set(bpm, 5, 5, 0);
	  cpl_image_set(bpm, 6, 5, 0);
	  cpl_image_set(bpm, 4, 6, 0);
	  cpl_image_set(bpm, 5, 6, 0);
	  cpl_image_set(bpm, 6, 6, 0);

	  cpl_imagelist_set(imglist_data, data, var);
	  cpl_imagelist_set(imglist_error, errors, var);
	  cpl_imagelist_set(imglist_bpm, bpm, var);
      }
  }
  cpl_test_error(CPL_ERROR_NONE);
  /*Build the header for the wcs */

  cpl_propertylist* plist = cpl_propertylist_new();

  cpl_propertylist_append_int(plist, "NAXIS"  , 3   );
  cpl_propertylist_append_int(plist, "NAXIS1" , 9   );
  cpl_propertylist_append_int(plist, "NAXIS2" , 9   );
  cpl_propertylist_append_int(plist, "NAXIS3" , 99  );
  cpl_propertylist_append_double(plist, "CD1_1"  , -0.01     );
  cpl_propertylist_append_double(plist, "CD1_2"  , 0.        );
  cpl_propertylist_append_double(plist, "CD2_1"  , 0.        );
  cpl_propertylist_append_double(plist, "CD2_2"  , 0.01      );
  cpl_propertylist_append_double(plist, "CRPIX1" , 4.5       );
  cpl_propertylist_append_double(plist, "CRPIX2" , 4.5       );
  cpl_propertylist_append_double(plist, "CRVAL1" , 48.0      );
  cpl_propertylist_append_double(plist, "CRVAL2" , -20.0     );
  cpl_propertylist_append_string(plist, "CTYPE1" , "RA---TAN");
  cpl_propertylist_append_string(plist, "CTYPE2" , "DEC--TAN");
  cpl_propertylist_append_string(plist, "CUNIT1" , "deg"     );
  cpl_propertylist_append_string(plist, "CUNIT2" , "deg"     );
  cpl_propertylist_append_double(plist, "CD1_3"  , 0.        );
  cpl_propertylist_append_double(plist, "CD2_3"  , 0.        );
  cpl_propertylist_append_double(plist, "CD3_1"  , 0.        );
  cpl_propertylist_append_double(plist, "CD3_2"  , 0.        );
  cpl_propertylist_append_double(plist, "CD3_3"  , 1.0       );
  cpl_propertylist_append_double(plist, "CRPIX3" , 1.0       );
  cpl_propertylist_append_double(plist, "CRVAL3" , 1.0       );
  cpl_propertylist_append_string(plist, "CTYPE3" , "WAVE"    );
  cpl_propertylist_append_string(plist, "CUNIT3" , "m"       );
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_random_state * rastate = hdrl_random_state_new(1, NULL);
  hdrl_random_state * decstate = hdrl_random_state_new(1, NULL);

  /* Please be aware on the different accuracies set for the errors for the
   * different methods */
  hdrl_imagelist* hlist =
      hdrl_resample_util_hdrl_imagelist_create(imglist_data,
					       imglist_error, imglist_bpm);
  const cpl_size iterations = 20;
  for (cpl_size i = 0; i < iterations; i++) {
      double rarandom = hdrl_random_uniform_double(rastate, 0., 360.);
      double decrandom = hdrl_random_uniform_double(decstate, -89., 89.);
      cpl_msg_info(cpl_func,"ra-random: %g, dec-random: %g ", rarandom,
		   decrandom);
      cpl_propertylist_update_double(plist, "CRVAL1" , rarandom);
      cpl_propertylist_update_double(plist, "CRVAL2" , decrandom);
      cpl_wcs * wcs = cpl_wcs_new_from_propertylist(plist);
      cpl_test_error(CPL_ERROR_NONE);
      /* Construct the table */
      cpl_table * table = hdrl_resample_imagelist_to_table(hlist, wcs);

      cpl_test_error(CPL_ERROR_NONE);
      hdrl_parameter *aParams_outputgrid = NULL;
      /* Define the output grid */
      aParams_outputgrid =
	  hdrl_resample_parameter_create_outgrid3D(0.01, 0.01, 1);
      cpl_test_nonnull(aParams_outputgrid);
      cpl_test_error(CPL_ERROR_NONE);

      hdrl_resample_result *result = NULL;
      hdrl_parameter *aParams_method = NULL;
      int rej;

      /* ---------------------------lanczos------------------------------- */

      /* Define the method */
      aParams_method = hdrl_resample_parameter_create_lanczos(1, CPL_FALSE, 2);
      /* Do the resampling */
      result = hdrl_resample_compute(table, aParams_method, aParams_outputgrid, wcs);
      cpl_test_error(CPL_ERROR_NONE);
      cpl_test_rel(cpl_image_get(hdrl_image_get_image_const(hdrl_imagelist_get_const(result->himlist, 48)),
				 5, 5, &rej), 49., 1e-6);
      cpl_test_rel(cpl_image_get(hdrl_image_get_error_const(hdrl_imagelist_get_const(result->himlist, 48)),
				 5, 5, &rej), 7., 0.05);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 48)),
			       5, 5), 0);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 48)),
			       1, 1), 1);
      hdrl_parameter_delete(aParams_method);
      hdrl_resample_result_delete(result);


      /* -----------------------------drizzle----------------------------- */

      /* Define the method */
      aParams_method = hdrl_resample_parameter_create_drizzle(1, CPL_FALSE,
							      0.8, 0.8, 0.8);
      /* Do the resampling */

      result = hdrl_resample_compute(table, aParams_method, aParams_outputgrid, wcs);
      cpl_test_error(CPL_ERROR_NONE);
      cpl_test_rel(cpl_image_get(hdrl_image_get_image_const(hdrl_imagelist_get_const(result->himlist, 48)),
				 5, 5, &rej), 49., 1e-6);
      cpl_test_rel(cpl_image_get(hdrl_image_get_error_const(hdrl_imagelist_get_const(result->himlist, 48)),
				 5, 5, &rej), 7., 0.001);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 48)),
			       5, 5), 0);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 48)),
			       1, 1), 1);

      hdrl_parameter_delete(aParams_method);
      hdrl_resample_result_delete(result);

      /* -----------------------------linear------------------------------ */

      /* Define the method */
      aParams_method = hdrl_resample_parameter_create_linear(1, CPL_FALSE);
      /* Do the resampling */

      result = hdrl_resample_compute(table, aParams_method, aParams_outputgrid, wcs);
      cpl_test_error(CPL_ERROR_NONE);
      cpl_test_rel(cpl_image_get(hdrl_image_get_image_const(hdrl_imagelist_get_const(result->himlist, 48)),
				 5, 5, &rej), 49., 1e-6);
      cpl_test_rel(cpl_image_get(hdrl_image_get_error_const(hdrl_imagelist_get_const(result->himlist, 48)),
				 5, 5, &rej), 7., 0.3);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 48)),
			       5, 5), 0);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 48)),
			       1, 1), 1);

      hdrl_parameter_delete(aParams_method);
      hdrl_resample_result_delete(result);

      /* -----------------------------quadratic--------------------------- */

      /* Define the method */
      aParams_method = hdrl_resample_parameter_create_quadratic(1, CPL_FALSE);
      /* Do the resampling */

      result = hdrl_resample_compute(table, aParams_method, aParams_outputgrid, wcs);
      cpl_test_error(CPL_ERROR_NONE);
      cpl_test_rel(cpl_image_get(hdrl_image_get_image_const(hdrl_imagelist_get_const(result->himlist, 48)),
				 5, 5, &rej), 49., 1e-6);
      cpl_test_rel(cpl_image_get(hdrl_image_get_error_const(hdrl_imagelist_get_const(result->himlist, 48)),
				 5, 5, &rej), 7., 0.02);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 48)),
			       5, 5), 0);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 48)),
			       1, 1), 1);

      hdrl_parameter_delete(aParams_method);
      hdrl_resample_result_delete(result);

      /* -----------------------------renka------------------------------- */

      /* Define the method */
      aParams_method = hdrl_resample_parameter_create_renka(1, CPL_FALSE,
							    1.25);
      /* Do the resampling */

      result = hdrl_resample_compute(table, aParams_method, aParams_outputgrid, wcs);
      cpl_test_error(CPL_ERROR_NONE);
      cpl_test_rel(cpl_image_get(hdrl_image_get_image_const(hdrl_imagelist_get_const(result->himlist, 48)),
				 5, 5, &rej), 49., 1e-6);
      cpl_test_rel(cpl_image_get(hdrl_image_get_error_const(hdrl_imagelist_get_const(result->himlist, 48)),
				 5, 5, &rej), 7., 0.01);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 48)),
			       5, 5), 0);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 48)),
			       1, 1), 1);

      hdrl_parameter_delete(aParams_method);
      hdrl_resample_result_delete(result);

      /* -----------------------------nearest----------------------------- */

      /* Define the method */
      aParams_method = hdrl_resample_parameter_create_nearest();
      /* Do the resampling */
      result = hdrl_resample_compute(table, aParams_method, aParams_outputgrid, wcs);
      cpl_test_error(CPL_ERROR_NONE);
      cpl_test_rel(cpl_image_get(hdrl_image_get_image_const(hdrl_imagelist_get_const(result->himlist, 48)),
				 5, 5, &rej), 49., 1e-6);
      cpl_test_rel(cpl_image_get(hdrl_image_get_error_const(hdrl_imagelist_get_const(result->himlist, 48)),
				 5, 5, &rej), 7., 0.00001);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 48)),
			       5, 5), 0);
      cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 48)),
			       1, 1), 1);

      hdrl_parameter_delete(aParams_method);
      hdrl_resample_result_delete(result);

      /* ----------------------------------------------------------------- */

      hdrl_parameter_delete(aParams_outputgrid);
      cpl_table_delete(table);
      cpl_wcs_delete(wcs);
  } /* Loop ends here */


  /* ---------------- For manual updates and tests ----------------------- */

  /* Test image spanning over ra 360 degree */
  cpl_propertylist_update_double(plist, "CRVAL1" , 0.03);
  cpl_propertylist_update_double(plist, "CRVAL2" , 0.1);
  cpl_wcs * wcs = cpl_wcs_new_from_propertylist(plist);

  /* Construct the table */
  cpl_table * table = hdrl_resample_imagelist_to_table(hlist, wcs);
  hdrl_imagelist_delete(hlist);
  hdrl_parameter *aParams_outputgrid = NULL;
  /* Define the output grid */
  aParams_outputgrid =
      hdrl_resample_parameter_create_outgrid3D(0.01, 0.01, 1);
  cpl_test_nonnull(aParams_outputgrid);
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_resample_result *result = NULL;
  hdrl_parameter *aParams_method = NULL;
  int rej;

  /* Define the method */
  aParams_method = hdrl_resample_parameter_create_lanczos(1, CPL_FALSE, 2);
  /* Do the resampling */
  result = hdrl_resample_compute(table, aParams_method, aParams_outputgrid, wcs);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_rel(cpl_image_get(hdrl_image_get_image_const(hdrl_imagelist_get_const(result->himlist, 48)),
			     5, 5, &rej), 49., 1e-6);
  cpl_test_rel(cpl_image_get(hdrl_image_get_error_const(hdrl_imagelist_get_const(result->himlist, 48)),
			     5, 5, &rej), 7., 0.05);
  cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 48)),
			   5, 5), 0);
  cpl_test_eq(cpl_mask_get(hdrl_image_get_mask_const(hdrl_imagelist_get_const(result->himlist, 48)),
			   1, 1), 1);

  cpl_test_error(CPL_ERROR_NONE);
  /*Save the original cube*/
  cpl_propertylist_save(plist, "cube_orig.fits", CPL_IO_CREATE);
  cpl_imagelist_save(imglist_data, "cube_orig.fits", CPL_TYPE_FLOAT, plist,
		     CPL_IO_EXTEND);
  cpl_imagelist_save(imglist_bpm, "cube_orig.fits", CPL_TYPE_INT, plist,
		     CPL_IO_EXTEND);
  cpl_imagelist_save(imglist_error, "cube_orig.fits", CPL_TYPE_FLOAT,
		     plist, CPL_IO_EXTEND);

  /*Save the resampled cube */

  cpl_propertylist_save(plist, "cube_resampled.fits", CPL_IO_CREATE);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_imagelist * ilistdata = cpl_imagelist_new();
  cpl_imagelist * ilisterrors = cpl_imagelist_new();
  cpl_imagelist * ilistbpm = cpl_imagelist_new();

  cpl_size planes = hdrl_imagelist_get_size(result->himlist);
  for (cpl_size i = 0; i < planes; i++) {
      /* The images are duplicated to avoid errors reported by cppcheck */
      cpl_imagelist_set(ilistdata, cpl_image_duplicate(hdrl_image_get_image(hdrl_imagelist_get(result->himlist, i))), i);
      cpl_imagelist_set(ilisterrors, cpl_image_duplicate(hdrl_image_get_error(hdrl_imagelist_get(result->himlist, i))), i);
      cpl_image * bpm = cpl_image_new_from_mask(hdrl_image_get_mask(hdrl_imagelist_get(result->himlist, i)));
      cpl_imagelist_set(ilistbpm, bpm, i);
  }

  /* Save the data/bpm/errors in the right extension as a cube */
  cpl_imagelist_save(ilistdata, "cube_resampled.fits", CPL_TYPE_FLOAT,
		     result->header, CPL_IO_EXTEND);
  cpl_imagelist_save(ilistbpm, "cube_resampled.fits", CPL_TYPE_INT,
		     result->header, CPL_IO_EXTEND);
  cpl_imagelist_save(ilisterrors, "cube_resampled.fits", CPL_TYPE_FLOAT,
		     result->header, CPL_IO_EXTEND);


  cpl_imagelist_delete(ilistdata);
  cpl_imagelist_delete(ilisterrors);
  cpl_imagelist_delete(ilistbpm);

  /*Print the wcs from the original and resampled cube*/
  hdrl_resample_wcs_print(wcs);
  cpl_wcs_delete(wcs);
  wcs = cpl_wcs_new_from_propertylist(result->header);
  hdrl_resample_wcs_print(wcs);

  /* free the memory */
  cpl_propertylist_delete(plist);
  cpl_imagelist_delete(imglist_data);
  cpl_imagelist_delete(imglist_error);
  cpl_imagelist_delete(imglist_bpm);
  hdrl_random_state_delete(rastate);
  hdrl_random_state_delete(decstate);

  hdrl_parameter_delete(aParams_method);
  hdrl_parameter_delete(aParams_outputgrid);
  hdrl_resample_result_delete(result);
  cpl_table_delete(table);
  cpl_wcs_delete(wcs);

  return cpl_error_get_code();
}
/* -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- */
/**
  @brief Utility to create a standard FITS header for an image. Used in several
         unit tests.

  @param naxis  NAXIS value
  @param sx     image X size
  @param sy     image Y size
  @param ra     RA value
  @param dec    DEC value
  @param cd11   CD1_1 value
  @param cd12   CD1_2 value
  @param cd21   CD2_1 value
  @param cd22   CD2_2 value
  @param crpix1 CRPIX1 value
  @param crpix2 CRPIX2 value
  @param crval1 CRVAL1 value
  @param crval2 CRVAL2 value
  @param cdelt1 CDELT1 value
  @param cdelt2 CDELT2 value
  @param ctype1 CTYPE1 value
  @param ctype2 CTYPE2 value
  @param cunit1 CUNIT1 value
  @param cunit2 CUNIT2 value

  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
/* unit tests utility */
static cpl_propertylist*
hdrl_resample_util_crea_header_image(const int naxis, const int sx, const int sy,
				     const double ra, const double dec,
				     const double cd11, const double cd12,
				     const double cd21, const double cd22,
				     const double crpix1, const double crpix2,
				     const double crval1, const double crval2,
				     const double cdelt1, const double cdelt2,
				     const char* ctype1, const char* ctype2,
				     const char* cunit1, const char* cunit2)
{
  cpl_propertylist* plist = cpl_propertylist_new();
  cpl_propertylist_append_int(plist,    "NAXIS", naxis);

  cpl_propertylist_append_int(plist,    "NAXIS1", sx);
  cpl_propertylist_append_int(plist,    "NAXIS2", sy);

  cpl_propertylist_append_double(plist, "RA", ra);
  cpl_propertylist_append_double(plist, "DEC", dec);

  cpl_propertylist_append_double(plist, "CRPIX1", crpix1);
  cpl_propertylist_append_double(plist, "CRPIX2", crpix2);

  cpl_propertylist_append_double(plist, "CRVAL1", crval1);
  cpl_propertylist_append_double(plist, "CRVAL2", crval2);

  cpl_propertylist_append_double(plist, "CDELT1", cdelt1);
  cpl_propertylist_append_double(plist, "CDELT2", cdelt2);

  cpl_propertylist_append_string(plist, "CTYPE1", ctype1);
  cpl_propertylist_append_string(plist, "CTYPE2", ctype2);

  cpl_propertylist_append_string(plist, "CUNIT1", cunit1);
  cpl_propertylist_append_string(plist, "CUNIT2", cunit2);

  cpl_propertylist_append_double(plist, "CD1_1", cd11);
  cpl_propertylist_append_double(plist, "CD1_2", cd12);
  cpl_propertylist_append_double(plist, "CD2_1", cd21);
  cpl_propertylist_append_double(plist, "CD2_2", cd22);

  /* To be sure to have a standard FITS header we save and reload the image */
  cpl_image* ima = cpl_image_new(sx, sy, CPL_TYPE_INT);
  cpl_image_add_scalar(ima, 1);

  cpl_image_save(ima, "ima.fits", CPL_TYPE_INT, plist, CPL_IO_DEFAULT);
  cpl_image_delete(ima);
  cpl_propertylist_delete(plist);
  plist = cpl_propertylist_load("ima.fits", 0);
  cpl_test_error(CPL_ERROR_NONE);
  return plist;
}


/**
  @brief Utility to create a standard FITS header for a cube. Used in several
         unit tests.

  @param naxis  NAXIS value
  @param sx     image X size
  @param sy     image Y size
  @param sz     image Z size
  @param ra     RA value
  @param dec    DEC value
  @param cd11   CD1_1 value
  @param cd12   CD1_2 value
  @param cd21   CD2_1 value
  @param cd22   CD2_2 value

  @param cd13   CD1_3 value
  @param cd31   CD3_1 value
  @param cd23   CD2_3 value
  @param cd32   CD3_2 value

  @param cd33   CD3_3 value

  @param crpix1 CRPIX1 value
  @param crpix2 CRPIX2 value
  @param crpix3 CRPIX3 value

  @param crval1 CRVAL1 value
  @param crval2 CRVAL2 value
  @param crval3 CRVAL3 value

  @param cdelt1 CDELT1 value
  @param cdelt2 CDELT2 value
  @param cdelt3 CDELT3 value

  @param ctype1 CTYPE1 value
  @param ctype2 CTYPE2 value
  @param ctype3 CTYPE3 value

  @param cunit1 CUNIT1 value
  @param cunit2 CUNIT2 value
  @param cunit3 CUNIT3 value

  @return cpl_error_code
 */
/* unit tests utility */
static cpl_propertylist*
hdrl_resample_crea_header_cube(const int naxis,
			       const int sx, const int sy, const int sz,
			       const double ra, const double dec,
			       const double cd11, const double cd12,
			       const double cd21, const double cd22,
			       const double cd13, const double cd31,
			       const double cd23, const double cd32,
			       const double cd33,
			       const double crpix1, const double crpix2, const double crpix3,
			       const double crval1, const double crval2, const double crval3,
			       const double cdelt1, const double cdelt2, const double cdelt3,
			       const char* ctype1, const char* ctype2, const char* ctype3,
			       const char* cunit1, const char* cunit2, const char* cunit3)
{
  /* crea first the FITS header for a 2D example */
  cpl_propertylist* plist =
      hdrl_resample_util_crea_header_image(naxis, sx, sy, ra, dec,
					   cd11, cd12, cd21, cd22, crpix1, crpix2,
					   crval1, crval2, cdelt1, cdelt2, ctype1,
					   ctype2, cunit1, cunit2);

  /* then add information for a 3D example */
  cpl_propertylist_update_int(plist,    "NAXIS", naxis);

  cpl_propertylist_append_int(plist,    "NAXIS3", sz);

  cpl_propertylist_append_double(plist, "CRVAL3", crval3);
  cpl_propertylist_append_double(plist, "CRPIX3", crpix3);
  cpl_propertylist_append_double(plist, "CDELT3", cdelt3);
  cpl_propertylist_append_string(plist, "CTYPE3", ctype3);
  cpl_propertylist_append_string(plist, "CUNIT3", cunit3);

  cpl_propertylist_append_double(plist, "CD1_3", cd13);
  cpl_propertylist_append_double(plist, "CD3_1", cd31);
  cpl_propertylist_append_double(plist, "CD2_3", cd23);
  cpl_propertylist_append_double(plist, "CD3_2", cd32);
  cpl_propertylist_append_double(plist, "CD3_3", cd33);


  /* To be sure to have a standard FITS header we save & reload the imagelist */
  cpl_image* ima = cpl_image_new(sx, sy, CPL_TYPE_INT);
  cpl_image_add_scalar(ima, 1);
  cpl_imagelist* iml = cpl_imagelist_new();
  for(int i = 0; i< sz;i++) {
      cpl_imagelist_set(iml, ima, i);
  }

  cpl_imagelist_save(iml, "iml.fits", CPL_TYPE_INT, plist, CPL_IO_DEFAULT);
  cpl_imagelist_delete(iml);
  cpl_propertylist_delete(plist);
  plist = cpl_propertylist_load("iml.fits", 0);
  cpl_test_error(CPL_ERROR_NONE);
  return plist;
}

/**
  @brief this unit test verify  hdrl_resample_weight_function_renka()
 */
static cpl_error_code
test_invalid_input_hdrl_resample_weight_function_renka(void){
  double r;
  double r_c;

  /* test invalid input */
  /* case r = 0 */
  r = 0;
  r_c = 1;
  cpl_test_abs(hdrl_resample_weight_function_renka(r, r_c), FLT_MAX, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);

  /* case r > r_c */
  r = 2;
  r_c = 1;
  cpl_test_abs(hdrl_resample_weight_function_renka(r, r_c), DBL_MIN, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);

  /* case r == r_c */
  r = 2;
  r_c = 2;
  cpl_test_abs(hdrl_resample_weight_function_renka(r, r_c), DBL_MIN, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);
  return cpl_error_get_code();
}
static cpl_error_code
test_hdrl_resample_weight_function_renka(void)
{
  double r;
  double r_c;
  double res;

  /* test invalid input */
  test_invalid_input_hdrl_resample_weight_function_renka();

  /* test valid input */
  /* case r < r_c */
  r = 2.;
  r_c = 3.;

  res = 0.0277777777777777762; /* value from print out on fc32 (laptop)*/
  cpl_test_abs(hdrl_resample_weight_function_renka(r, r_c), res, HDRL_EPS_TEST);

  cpl_test_error(CPL_ERROR_NONE);

  return cpl_error_get_code();
}

/**
  @brief this unit test verify  hdrl_resample_weight_function_drizzle()
 */

static cpl_error_code
test_hdrl_resample_weight_function_drizzle(void)
{
  double x_in, y_in, z_in;
  double x_out, y_out, z_out;
  double dx, dy, dz;

  /* 1st test case */
  x_in = 2, y_in = 2, z_in = 2;
  dx = 1, dy = 1, dz = 1;
  x_out = 1, y_out = 1, z_out = 1;
  double res = 0.015625;/* value from computation in code */
  hdrl_resample_weight_function_drizzle(x_in, y_in, z_in, x_out, y_out, z_out,
					dx, dy, dz);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_abs(hdrl_resample_weight_function_drizzle(x_in, y_in, z_in,
						     x_out, y_out, z_out, dx, dy, dz),res,HDRL_EPS_TEST);

  /* 2nd test case */
  x_in = 3, y_in = 3, z_in = 3;
  dx = 1, dy = 1, dz = 1;
  x_out = 2, y_out = 2, z_out = 2;
  res = 0.125;/* value from computation in code */
  hdrl_resample_weight_function_drizzle(x_in, y_in, z_in, x_out, y_out, z_out,
					dx, dy, dz);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(hdrl_resample_weight_function_drizzle(x_in, y_in, z_in,
						     x_out, y_out, z_out, dx, dy, dz),res,HDRL_EPS_TEST);

  /* 3rd test case */
  x_in = 2, y_in = 2, z_in = 2;
  dx = 10, dy = 10, dz = 10;
  x_out = 1, y_out = 1, z_out = 1;
  res = 0; /* value from computation in code */
  cpl_test_abs(hdrl_resample_weight_function_drizzle(x_in, y_in, z_in,
						     x_out, y_out, z_out, dx, dy, dz),res,HDRL_EPS_TEST);

  cpl_test_error(CPL_ERROR_NONE);

  return cpl_error_get_code();
}

/**
  @brief this unit test verify  hdrl_resample_weight_function_linear()
 */
static cpl_error_code
test_hdrl_resample_weight_function_linear(void)
{
  /* test invalid input */
  /* r = 0 */
  cpl_test_abs(hdrl_resample_weight_function_linear(0), FLT_MAX, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);

  /* test valid input */
  /* r = 2 */
  cpl_test_abs(hdrl_resample_weight_function_linear(2), 0.5, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);


  return cpl_error_get_code();
}

/**
  @brief this unit test verify  hdrl_resample_weight_function_quadratic()
 */
static cpl_error_code
test_hdrl_resample_weight_function_quadratic(void)
{
  /* test invalid input */
  /* r = 0 */
  cpl_test_abs(hdrl_resample_weight_function_quadratic(0), FLT_MAX, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);

  /* test valid input */
  /* r = 4 */
  cpl_test_abs(hdrl_resample_weight_function_quadratic(4), 0.25, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);


  return cpl_error_get_code();
}

/**
  @brief this unit test verify  hdrl_resample_weight_function_sinc()
 */
static cpl_error_code
test_hdrl_resample_weight_function_sinc(void)
{

  double r;
  double res;
  /* test invalid input */
  /* r = 0 */
  cpl_test_abs(hdrl_resample_weight_function_sinc(DBL_EPSILON), 1., HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);

  /* test valid input */
  r = 0.25;

  res = 0.900316316157106056;/* value from computation in code */
  cpl_test_abs(hdrl_resample_weight_function_sinc(r), res, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);

  return cpl_error_get_code();
}

/**
  @brief this unit test verify  hdrl_resample_weight_function_lanczos()
 */
static cpl_error_code
test_hdrl_resample_weight_function_lanczos(void)
{

  double dx = 1.;
  double dy = 1.;
  double dz = 1.;
  unsigned int ld = 4; /* loop distance */
  unsigned int lks = 2; /* lanczos kernel size */

  /* test invalid input */
  /* dx = 0 || dy =0 || dz = 0 */
  cpl_test_abs(hdrl_resample_weight_function_lanczos(5., dy, dz, ld, lks), 0., HDRL_EPS_TEST);
  cpl_test_abs(hdrl_resample_weight_function_lanczos(dx, 5., dz, ld, lks), 0., HDRL_EPS_TEST);
  cpl_test_abs(hdrl_resample_weight_function_lanczos(dx, dy, 5., ld, lks), 0., HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);

  /* test valid input */
  double res = 4.32283142061004719e-50;/* value from computation in code */
  cpl_test_abs(hdrl_resample_weight_function_lanczos(dx, dy, dz, ld, lks), res, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);

  return cpl_error_get_code();
}

/**
 * Utility to create an image of a #5 of a dice
 */
static cpl_image*
hdrl_resample_util_crea_image_dice_5(const cpl_size sx, const cpl_size sy,
				     const double value)
{
  assert(value > 0);
  cpl_image * image = cpl_image_new(sx, sy, CPL_TYPE_DOUBLE);
  cpl_image * dice = cpl_image_new(sx, sy, CPL_TYPE_DOUBLE);
  cpl_image_add_scalar(image,value);

  cpl_size xc = 0.5 * sx;
  cpl_size yc = 0.5 * sy;
  cpl_size dx = 2;
  cpl_size dy = 2;
  cpl_size xl = 0.25 * sx;
  cpl_size yl = 0.25 * sy;
  cpl_size xh = 0.75 * sx;
  cpl_size yh = 0.75 * sy;
  double flux_point=2.*value;
  cpl_image_fill_window(dice, xc - dx, yc - dy, xc + dx, yc + dy, flux_point);
  cpl_image_fill_window(dice, xl - dx, yl - dy, xl + dx, yl + dy, flux_point);
  cpl_image_fill_window(dice, xl - dx, yh - dy, xl + dx, yh + dy, flux_point);
  cpl_image_fill_window(dice, xh - dx, yh - dy, xh + dx, yh + dy, flux_point);
  cpl_image_fill_window(dice, xh - dx, yl - dy, xh + dx, yl + dy, flux_point);
  cpl_image_add(image,dice);

  cpl_image_delete(dice);
  cpl_test_error(CPL_ERROR_NONE);
  return image;
}


/*----------------------------------------------------------------------------*/
/**
  @brief   creates an HDRL method parameter
  @param loop_distance     loop distance
  @param critical_radius_renka critical radius for renka resampling method
  @param pix_frac_drizzle_x     pixel fraction x for drizzle resampling method
  @param pix_frac_drizzle_y     pixel fraction x for drizzle resampling method
  @param pix_frac_drizzle_l     pixel fraction x for drizzle resampling method
  @param resample_method     pixel resampling method
  @param use_errorweights    use error weights ?
  @return  hdrl_parameter*
 */
/*----------------------------------------------------------------------------*/
static hdrl_parameter *
hdrl_resample_util_methodparam_create (
    const int loop_distance,
    const double critical_radius_renka,
    const int kernel_size_lanczos,
    const double pix_frac_drizzle_x,
    const double pix_frac_drizzle_y,
    const double pix_frac_drizzle_l,
    const int resample_method,
    const cpl_boolean use_errorweights
)
{
  hdrl_parameter      *aParams_method = NULL;

  /* Create the right re-sampling parameter */
  switch(resample_method) {
    case HDRL_RESAMPLE_METHOD_NEAREST:
      aParams_method =
	  hdrl_resample_parameter_create_nearest();
      break;
    case HDRL_RESAMPLE_METHOD_RENKA:
      aParams_method =
	  hdrl_resample_parameter_create_renka(loop_distance, use_errorweights,
					       critical_radius_renka);
      break;
    case HDRL_RESAMPLE_METHOD_LINEAR:
      aParams_method =
	  hdrl_resample_parameter_create_linear(loop_distance, use_errorweights);
      break;
    case HDRL_RESAMPLE_METHOD_QUADRATIC:
      aParams_method =
	  hdrl_resample_parameter_create_quadratic(loop_distance, use_errorweights);
      break;
    case HDRL_RESAMPLE_METHOD_DRIZZLE:
      aParams_method =
	  hdrl_resample_parameter_create_drizzle(loop_distance, use_errorweights,
						 pix_frac_drizzle_x,
						 pix_frac_drizzle_y,
						 pix_frac_drizzle_l);
      break;
    case HDRL_RESAMPLE_METHOD_LANCZOS:
      aParams_method =
	  hdrl_resample_parameter_create_lanczos(loop_distance, use_errorweights,
						 kernel_size_lanczos);
      break;
    default:
      aParams_method =
	  hdrl_resample_parameter_create_lanczos(loop_distance, use_errorweights,
						 kernel_size_lanczos);
      cpl_msg_warning (cpl_func,
		       "You set an unsupported method! Default to LANCZOS");
      break;
  }

  cpl_test_error(CPL_ERROR_NONE);
  return aParams_method;
}


/*----------------------------------------------------------------------------*/
/**
  @brief   creates a FITS header with an example taken by a paper
  @return  cpl_wcs *
  @doc create header for part of example 1 (Table 5) of
  Greisen & Calabretta 2002 A&A 395, 1077 (Paper II)
 */
/*----------------------------------------------------------------------------*/
static cpl_wcs *
hdrl_resample_util_wcs_create_example_params(void)
{

  cpl_propertylist* p = cpl_propertylist_new();
  /* leave out the velocity and stokes axes */
  cpl_propertylist_append_int(p,    "NAXIS",  2);
  cpl_propertylist_append_int(p,    "NAXIS1", 512);
  cpl_propertylist_append_int(p,    "NAXIS2", 512);
  cpl_propertylist_append_double(p, "CRPIX1", 256.);

  /* use the CDi_j matrix instead of CDELT */
  cpl_propertylist_append_double(p, "CD1_1", -0.003);
  cpl_propertylist_append_string(p, "CTYPE1", "RA---TAN");
  cpl_propertylist_append_double(p, "CRVAL1", 45.83);
  cpl_propertylist_append_string(p, "CUNIT1", "deg");

  cpl_propertylist_append_double(p, "CRPIX2", 257.);
  cpl_propertylist_append_double(p, "CD2_2",  0.003);
  cpl_propertylist_append_string(p, "CTYPE2", "DEC--TAN");
  cpl_propertylist_append_double(p, "CRVAL2", 63.57);
  cpl_propertylist_append_string(p, "CUNIT2", "deg");

  /* no cross terms */
  cpl_propertylist_append_double(p, "CD1_2", 0.);
  cpl_propertylist_append_double(p, "CD2_1", 0.);

  /* XXX RA,DEC identical to the CRVALi for our functions */
  cpl_propertylist_append_double(p, "RA", 45.83);
  cpl_propertylist_append_double(p, "DEC", 63.57);

  cpl_wcs* wcs = cpl_wcs_new_from_propertylist(p);
  cpl_propertylist_delete(p);
  cpl_test_error(CPL_ERROR_NONE);
  return wcs;
} /* hdrl_resample_util_wcs_create_example_params() */

/*----------------------------------------------------------------------------*/
/**
   @brief   creates wcs example for a 2D frame
   @return  cpl_wcs *
 */
/*----------------------------------------------------------------------------*/
static cpl_wcs*
hdrl_resample_util_crea_wcs_2d(void)
{
  cpl_propertylist* plist = NULL;
  /* test valid input */
  int sx = HDRL_SIZE_X;
  int sy = HDRL_SIZE_Y;

  int naxis = 2;
  double ra = 10.;
  double dec = 20.;
  double cd11 = HDRL_CD11;
  double cd22 = HDRL_CD22;
  double cd12 = HDRL_CD12;
  double cd21 = HDRL_CD21;
  const double crpix1 = HDRL_CRPIX1;
  const double crpix2 = HDRL_CRPIX2;
  const double crval1 = HDRL_CRVAL1;
  const double crval2 = HDRL_CRVAL2;
  const double cdelt1 = HDRL_CDELT1;
  const double cdelt2 = HDRL_CDELT2;
  const char* cunit1 = "";
  const char* cunit2 = "";
  const char* ctype1 = "pix";
  const char* ctype2 = "pix";

  plist = hdrl_resample_util_crea_header_image(naxis, sx, sy, ra, dec,
					       cd11, cd12, cd21, cd22, crpix1,
					       crpix2, crval1, crval2,
					       cdelt1, cdelt2, ctype1, ctype2,
					       cunit1, cunit2);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_wcs* wcs = cpl_wcs_new_from_propertylist(plist);
  cpl_test_nonnull(wcs);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_propertylist_delete(plist);
  return wcs;

}
/*----------------------------------------------------------------------------*/
/**
  @brief   creates wcs example for a 2D frame
  @return  cpl_wcs *
 */
/*----------------------------------------------------------------------------*/
static cpl_wcs*
hdrl_resample_util_crea_wcs_3d(void)
{
  /* test 3D case */
  cpl_propertylist* plist = NULL;
  int sx = HDRL_SIZE_X;
  int sy = HDRL_SIZE_Y;

  int naxis = 2;
  double ra = 10.;
  double dec = 20.;
  double cd11 = HDRL_CD11;
  double cd22 = HDRL_CD22;
  double cd12 = HDRL_CD12;
  double cd21 = HDRL_CD21;
  const double crpix1 = HDRL_CRPIX1;
  const double crpix2 = HDRL_CRPIX2;
  const double crval1 = HDRL_CRVAL1;
  const double crval2 = HDRL_CRVAL2;
  const double cdelt1 = HDRL_CDELT1;
  const double cdelt2 = HDRL_CDELT2;
  const char* cunit1 = "deg";
  const char* cunit2 = "deg";
  const char* ctype1 = "RA---TAN";
  const char* ctype2 = "DEC--TAN";

  int sz = HDRL_SIZE_Z;
  double cd13 = HDRL_CD13;
  double cd31 = HDRL_CD31;
  double cd23 = HDRL_CD23;
  double cd32 = HDRL_CD32;
  double cd33 = HDRL_CD33;
  const double crpix3 = HDRL_CRPIX3;
  const double crval3 = HDRL_CRVAL3;
  const double cdelt3 = HDRL_CDELT3;
  const char* cunit3 = "m";
  const char* ctype3 = "WAV";
  naxis = 3;
  cpl_test_error(CPL_ERROR_NONE);
  cpl_propertylist_delete(plist);
  cpl_test_error(CPL_ERROR_NONE);
  plist = hdrl_resample_crea_header_cube(naxis, sx, sy, sz, ra, dec,
					 cd11, cd12, cd21, cd22, cd13, cd31,
					 cd23, cd32, cd33,
					 crpix1, crpix2, crpix3,
					 crval1, crval2, crval3,
					 cdelt1, cdelt2, cdelt3,
					 ctype1, ctype2, ctype3,
					 cunit1, cunit2, cunit3);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_wcs* wcs = cpl_wcs_new_from_propertylist(plist);
  cpl_test_nonnull(wcs);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_propertylist_delete(plist);
  return wcs;

}

CPL_DIAG_PRAGMA_PUSH_IGN(-Wunused-function);
/*----------------------------------------------------------------------------*/
/**
  @brief   creates wcs example. Taken from MUSE pipeline
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_resample_wcs_as_muse(void)
{

  //  const double kLimitPos = 0.05; /* [pix] object position detection accuracy */
  /* accuracy limits for coordinate transformation tests */
  //	const double kLimitDeg = DBL_EPSILON * 115; /* ~10 nano-arcsec for trafo to deg */
  //	const double kLimitDegF = FLT_EPSILON * 13.51; /* ~5.8 milli-arcsec for trafo to  *
  //	                                                * deg, with value stored in float */
  //	const double kLimitPix = FLT_EPSILON * 205; /* ~1/40000th pixel for trafo to pix */
  //	const double kLimitRot = FLT_EPSILON * 10; /* ~4.3 milli-arcsec for rotations */
  //	const double kLimitSca = DBL_EPSILON * 7461; /* pixel scales to ~5.5 nano-arcsec */

  const double kLimitPPl = FLT_EPSILON * 2.88; /* ~1.24 milli-arcsec in proj. plane */


  double v1, v2;
  /***************************************************************************
   * test transformations between projection plane and celestial coordinates *
   ***************************************************************************/
  /* use values from WCS Paper II, example 1 as references */

  cpl_test_error(CPL_ERROR_NONE);
  cpl_wcs* wcs = hdrl_resample_util_wcs_create_example_params();
  cpl_test_nonnull(wcs);
  /* NB: for the following we just want to test the WCS==> other coordinates
   * are casual.
   */
  hdrl_parameter* hpar =
      hdrl_resample_parameter_create_outgrid2D_userdef(1, 1, 10, 10.1, 10,
						       10.1, 5.);
  cpl_test_nonnull(hpar);
  hdrl_resample_outgrid_parameter * par = (hdrl_resample_outgrid_parameter *) hpar;
  cpl_test_nonnull(par);
  par->wcs = wcs;
  double x, y;
  cpl_errorstate state = cpl_errorstate_get();
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test(hdrl_resample_wcs_projplane_from_celestial(par, 47.503264, 62.795111,
						      &x, &y) == CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test(fabs(x - 0.765000) < kLimitPPl && fabs(y - (-0.765000)) < kLimitPPl);
  cpl_msg_debug(__func__, "SE corner: %f,%f (%e,%e <? %e", x, y, x - 0.765000,
		y - (-0.765000), kLimitPPl);
  cpl_test(hdrl_resample_wcs_projplane_from_celestial(par, 47.595581, 64.324332,
						      &x, &y) == CPL_ERROR_NONE);
  cpl_test(fabs(x - 0.765000) < kLimitPPl && fabs(y - 0.765000) < kLimitPPl);
  cpl_msg_debug(__func__, "NE corner: %f,%f (%e,%e <? %e", x, y, x - 0.765000,
		y - 0.765000, kLimitPPl);
  cpl_test(hdrl_resample_wcs_projplane_from_celestial(par, 44.064419, 64.324332,
						      &x, &y) == CPL_ERROR_NONE);
  cpl_test(fabs(x - (-0.765000)) < kLimitPPl && fabs(y - 0.765000) < kLimitPPl);
  cpl_msg_debug(__func__, "NW corner: %f,%f (%e,%e <? %e", x, y, x - (-0.765000),
		y - 0.765000, kLimitPPl);
  state = cpl_errorstate_get();
  cpl_test(hdrl_resample_wcs_projplane_from_celestial(NULL, 1., 1., &v1, &v2)
	   == CPL_ERROR_NULL_INPUT);
  cpl_test(hdrl_resample_wcs_projplane_from_celestial(par, 1., 1., NULL, &v2)
	   == CPL_ERROR_NULL_INPUT);
  cpl_test(hdrl_resample_wcs_projplane_from_celestial(par, 1., 1., &v1, NULL)
	   == CPL_ERROR_NULL_INPUT);
  cpl_errorstate_set(state);

  cpl_test_error(CPL_ERROR_NONE);
  cpl_test(hdrl_resample_wcs_projplane_from_celestial(par, 1., 1., &v1, &v2)
	   == CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_errorstate_set(state);

  cpl_wcs_delete(wcs);
  hdrl_parameter_destroy(hpar);
  cpl_test_error(CPL_ERROR_NONE);
  return cpl_error_get_code();
}

CPL_DIAG_PRAGMA_POP;
/*----------------------------------------------------------------------------*/
/**
  @brief   unit test to check hdrl_resample_image_to_table()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_image_to_table(void)
{

  cpl_table* tab = NULL;
  cpl_image* data = NULL;
  cpl_image* error = NULL;
  cpl_image* quality = NULL;
  cpl_wcs* wcs = NULL;

  /* test invalid input */
  tab = hdrl_resample_image_to_table(NULL, wcs);
  cpl_test_error(CPL_ERROR_NULL_INPUT);
  cpl_test_null(tab);

  /* test valid input */
  int sx = HDRL_SIZE_X;
  int sy = HDRL_SIZE_Y;
  double value = HDRL_FLUX_ADU;

  data = cpl_image_new(sx, sy, CPL_TYPE_DOUBLE);
  cpl_image_add_scalar(data, value);

  error = cpl_image_power_create(data, 0.5);
  quality = cpl_image_new(sx, sy, CPL_TYPE_INT);
  int naxis = 2;
  double ra = 10.;
  double dec = 20.;
  const double cd11 = -3.47222e-05;
  const double cd22 =  3.47222e-05;
  const double cd12 = 0;
  const double cd21 = 0;
  const double crpix1 = 33.5;
  const double crpix2 = 33.5;
  const double crval1 = 48.0718057375143246;
  const double crval2 = -20.6230284673176705;
  const double cdelt1 = 0;
  const double cdelt2 = 0;
  const char* cunit1 = "deg";
  const char* cunit2 = "deg";
  const char* ctype1 = "RA---TAN";
  const char* ctype2 = "DEC--TAN";

  cpl_propertylist* plist =
      hdrl_resample_util_crea_header_image(naxis, sx, sy, ra, dec,
					   cd11, cd12, cd21, cd22,
					   crpix1, crpix2, crval1, crval2,
					   cdelt1, cdelt2, ctype1, ctype2,
					   cunit1, cunit2);

  wcs = cpl_wcs_new_from_propertylist(plist);
  cpl_test_nonnull(wcs);
  hdrl_image* hima = hdrl_resample_util_hdrl_image_create(data, error, quality);
  tab = hdrl_resample_image_to_table(hima, wcs);
  cpl_test_error(CPL_ERROR_NONE);

  /* test expected table columns */
  cpl_test_eq(1, cpl_table_has_column(tab, HDRL_RESAMPLE_TABLE_RA));
  cpl_test_eq(1, cpl_table_has_column(tab, HDRL_RESAMPLE_TABLE_DEC));
  cpl_test_eq(1, cpl_table_has_column(tab, HDRL_RESAMPLE_TABLE_LAMBDA));
  cpl_test_eq(1, cpl_table_has_column(tab, HDRL_RESAMPLE_TABLE_BPM));
  cpl_test_eq(1, cpl_table_has_column(tab, HDRL_RESAMPLE_TABLE_ERRORS));

  double* ptablambda = cpl_table_get_data_double(tab, HDRL_RESAMPLE_TABLE_LAMBDA);
  double* ptabdata = cpl_table_get_data_double(tab, HDRL_RESAMPLE_TABLE_DATA);
  int*   ptabbpm = cpl_table_get_data_int(tab, HDRL_RESAMPLE_TABLE_BPM);
  double* ptaberr = cpl_table_get_data_double(tab, HDRL_RESAMPLE_TABLE_ERRORS);

  cpl_test_abs(0, ptablambda[0], HDRL_EPS_TEST);
  cpl_test_abs(HDRL_FLUX_ADU, ptabdata[0], HDRL_EPS_TEST);
  cpl_test_abs(0, ptabbpm[0], HDRL_EPS_TEST);
  cpl_test_abs(10, ptaberr[0], HDRL_EPS_TEST);

  /* free memory */
  cpl_table_delete(tab);
  cpl_image_delete(data);
  cpl_image_delete(error);
  cpl_image_delete(quality);
  hdrl_image_delete(hima);
  cpl_propertylist_delete(plist);
  cpl_wcs_delete(wcs);
  cpl_test_error(CPL_ERROR_NONE);
  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief   unit test to check hdrl_resample_imagelist_to_table()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_imagelist_to_table(void)
{
  /* test invalid input */
  cpl_table* tab = NULL;
  cpl_imagelist* ilist = NULL;
  cpl_imagelist* elist = NULL;
  cpl_imagelist* qlist = NULL;
  cpl_image* data = NULL;
  cpl_image* error = NULL;
  cpl_image* quality = NULL;
  cpl_wcs* wcs = NULL;

  /* test invalid input */
  tab = hdrl_resample_imagelist_to_table(NULL, wcs);
  cpl_test_error(CPL_ERROR_NULL_INPUT);
  cpl_test_null(tab);

  /* test valid input */
  int sx = 67;
  int sy = 67;
  double value = HDRL_FLUX_ADU;

  data = cpl_image_new(sx, sy, CPL_TYPE_DOUBLE);
  cpl_image_add_scalar(data, value);

  error = cpl_image_power_create(data, 0.5);
  quality = cpl_image_new(sx, sy, CPL_TYPE_INT);
  int naxis = 3;
  double ra = 10.;
  double dec = 20.;
  const double cd11 = -3.47222e-05;
  const double cd22 =  3.47222e-05;
  const double cd12 = 0;
  const double cd21 = 0;
  const double crpix1 = 33.5;
  const double crpix2 = 33.5;
  const double crval1 = 48.0706;
  const double crval2 = -20.6219;
  const double cdelt1 = 0;
  const double cdelt2 = 0;
  const char* cunit1 = "deg";
  const char* cunit2 = "deg";
  const char* ctype1 = "RA---TAN";
  const char* ctype2 = "DEC--TAN";
  int sz = 2218;
  double cd13 = 0;
  double cd31 = 0;
  double cd23 = 0;
  double cd32 = 0;
  double cd33 = 2.45e-10;
  const double crpix3 = 1;
  const double crval3 = 1.9283e-06;
  const double cdelt3 = 0.1;
  const char* cunit3 = "m";
  const char* ctype3 = "WAVE";

  cpl_test_error(CPL_ERROR_NONE);
  cpl_propertylist* plist =
      hdrl_resample_crea_header_cube(naxis, sx, sy, sz, ra, dec,
				     cd11, cd12, cd21, cd22,
				     cd13, cd31, cd23, cd32, cd33,
				     crpix1, crpix2, crpix3,
				     crval1, crval2, crval3,
				     cdelt1, cdelt2, cdelt3,
				     ctype1, ctype2, ctype3,
				     cunit1, cunit2, cunit3);

  ilist = cpl_imagelist_new();
  elist = cpl_imagelist_new();
  qlist = cpl_imagelist_new();

  cpl_imagelist_set(ilist, data, 0);
  cpl_imagelist_set(elist, error, 0);
  cpl_imagelist_set(qlist, quality, 0);

  wcs = cpl_wcs_new_from_propertylist(plist);
  cpl_test_nonnull(wcs);
  cpl_test_error(CPL_ERROR_NONE);
  hdrl_imagelist* hlist = hdrl_resample_util_hdrl_imagelist_create(ilist, elist,
								   qlist);
  tab = hdrl_resample_imagelist_to_table(hlist, wcs);
  cpl_test_error(CPL_ERROR_NONE);

  /* test expected table columns */
  cpl_test_eq(1, cpl_table_has_column(tab, HDRL_RESAMPLE_TABLE_RA));
  cpl_test_eq(1, cpl_table_has_column(tab, HDRL_RESAMPLE_TABLE_DEC));
  cpl_test_eq(1, cpl_table_has_column(tab, HDRL_RESAMPLE_TABLE_LAMBDA));
  cpl_test_eq(1, cpl_table_has_column(tab, HDRL_RESAMPLE_TABLE_BPM));
  cpl_test_eq(1, cpl_table_has_column(tab, HDRL_RESAMPLE_TABLE_ERRORS));

  //double* ptabxpos = cpl_table_get_data_double(tab, HDRL_RESAMPLE_TABLE_RA);
  //double* ptabypos = cpl_table_get_data_double(tab, HDRL_RESAMPLE_TABLE_DEC);
  double* ptablambda = cpl_table_get_data_double(tab, HDRL_RESAMPLE_TABLE_LAMBDA);
  double* ptabdata = cpl_table_get_data_double(tab, HDRL_RESAMPLE_TABLE_DATA);
  int*   ptabbpm = cpl_table_get_data_int(tab, HDRL_RESAMPLE_TABLE_BPM);
  double* ptaberr = cpl_table_get_data_double(tab, HDRL_RESAMPLE_TABLE_ERRORS);

  /* test expected values */
  cpl_test_abs(0, ptablambda[0], HDRL_EPS_TEST);
  cpl_test_abs(HDRL_FLUX_ADU, ptabdata[0], HDRL_EPS_TEST);
  cpl_test_abs(0, ptabbpm[0], HDRL_EPS_TEST);
  cpl_test_abs(10, ptaberr[0], HDRL_EPS_TEST);

  /* free memory */
  cpl_imagelist_delete(ilist);
  cpl_imagelist_delete(elist);
  cpl_imagelist_delete(qlist);
  hdrl_imagelist_delete(hlist);
  cpl_table_delete(tab);
  cpl_propertylist_delete(plist);
  cpl_wcs_delete(wcs);
  cpl_test_error(CPL_ERROR_NONE);
  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief   unit test to check hdrl_wcs_to_propertylist()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_wcs_to_propertylist(void) {

  cpl_propertylist* plist = NULL;
  cpl_wcs* wcs = NULL;
  cpl_boolean only2d = CPL_TRUE;

  /* test improper input */
  hdrl_wcs_to_propertylist(NULL, plist, only2d);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_wcs_to_propertylist(wcs, NULL, only2d);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  /* test proper input */
  wcs = hdrl_resample_util_crea_wcs_2d();
  cpl_test_error(CPL_ERROR_NONE);
  plist = cpl_propertylist_new();
  hdrl_wcs_to_propertylist(wcs, plist, only2d);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_wcs_delete(wcs);
  wcs = hdrl_resample_util_crea_wcs_3d();
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_wcs_to_propertylist(wcs, plist, !only2d);
  cpl_test_error(CPL_ERROR_NONE);

  /* start detailed checks on expected values */
  const cpl_array *crval = cpl_wcs_get_crval(wcs);
  const cpl_array *crpix = cpl_wcs_get_crpix(wcs);
  const cpl_array *ctype = cpl_wcs_get_ctype(wcs);
  const cpl_array *cunit = cpl_wcs_get_cunit(wcs);
  const cpl_matrix *cd = cpl_wcs_get_cd(wcs);
  int naxis_out = cpl_wcs_get_image_naxis(wcs);

  int naxis = 3;
  int sx = HDRL_SIZE_X;
  int sz = HDRL_SIZE_Z;
  cpl_test_eq(naxis, naxis_out);

  /* Check NAXIS */
  for (int i = 0; i < naxis_out; i++) {

      char * buf = cpl_sprintf("NAXIS%d", i + 1);
      /* sx = sy = HDRL_SIZE_X */
      if(i<2) {
	  cpl_test_eq(sx, cpl_propertylist_get_int(plist, buf));
      } else {
	  cpl_test_eq(sz, cpl_propertylist_get_int(plist, buf));
      }
      cpl_free(buf);
  }

  int err = 0;
  /* CRVAL */
  cpl_test_abs(cpl_propertylist_get_double(plist, "CRVAL1"),
	       cpl_array_get_double(crval, 0, &err), HDRL_EPS_TEST);

  cpl_test_abs(cpl_propertylist_get_double(plist, "CRVAL2"),
	       cpl_array_get_double(crval, 1, &err), HDRL_EPS_TEST);

  cpl_test_abs(cpl_propertylist_get_double(plist, "CRVAL3"),
	       cpl_array_get_double(crval, 2, &err), HDRL_EPS_TEST);

  /* CRPIX */
  cpl_test_abs(cpl_propertylist_get_double(plist, "CRPIX1"),
	       cpl_array_get_double(crpix, 0, &err), HDRL_EPS_TEST);

  cpl_test_abs(cpl_propertylist_get_double(plist, "CRPIX2"),
	       cpl_array_get_double(crpix, 1, &err), HDRL_EPS_TEST);

  cpl_test_abs(cpl_propertylist_get_double(plist, "CRPIX3"),
	       cpl_array_get_double(crpix, 2, &err), HDRL_EPS_TEST);

  /* CTYPE */
  const char* string_val = NULL;
  const char* string_chk = NULL;

  string_val = cpl_propertylist_get_string(plist, "CTYPE1");
  string_chk = cpl_array_get_string(ctype, 0);
  cpl_test_eq(0, strcmp(string_val,string_chk));

  string_val = cpl_propertylist_get_string(plist, "CTYPE2");
  string_chk = cpl_array_get_string(ctype, 1);
  cpl_test_eq(0, strcmp(string_val,string_chk));

  string_val = cpl_propertylist_get_string(plist, "CTYPE3");
  string_chk = cpl_array_get_string(ctype, 2);
  cpl_test_eq(0, strcmp(string_val,string_chk));

  /* CUNIT */
  string_val = cpl_propertylist_get_string(plist, "CUNIT1");
  string_chk = cpl_array_get_string(cunit, 0);
  cpl_test_eq(0, strcmp(string_val,string_chk));

  string_val = cpl_propertylist_get_string(plist, "CUNIT2");
  string_chk = cpl_array_get_string(cunit, 1);
  cpl_test_eq(0, strcmp(string_val,string_chk));

  string_val = cpl_propertylist_get_string(plist, "CUNIT3");
  string_chk = cpl_array_get_string(cunit, 2);
  cpl_test_eq(0, strcmp(string_val,string_chk));

  /* CD */
  double cd11 = 1, cd22 = 1, cd33 = 0.1;
  double cd12 = 0, cd21 = 0, cd13 = 0, cd31 = 0, cd23 = 0, cd32 = 0;

  cd11 = cpl_matrix_get(cd, 0, 0);
  cd12 = cpl_matrix_get(cd, 0, 1);
  cd21 = cpl_matrix_get(cd, 1, 0);
  cd22 = cpl_matrix_get(cd, 1, 1);
  cd13 = cpl_matrix_get(cd, 0, 2);
  cd31 = cpl_matrix_get(cd, 2, 0);
  cd23 = cpl_matrix_get(cd, 1, 2);
  cd32 = cpl_matrix_get(cd, 2, 1);
  cd33 = cpl_matrix_get(cd, 2, 2);

  cpl_test_abs(cd11, cpl_propertylist_get_double(plist, "CD1_1"), HDRL_EPS_TEST);
  cpl_test_abs(cd12, cpl_propertylist_get_double(plist, "CD1_2"), HDRL_EPS_TEST);
  cpl_test_abs(cd21, cpl_propertylist_get_double(plist, "CD2_1"), HDRL_EPS_TEST);
  cpl_test_abs(cd22, cpl_propertylist_get_double(plist, "CD2_2"), HDRL_EPS_TEST);
  cpl_test_abs(cd13, cpl_propertylist_get_double(plist, "CD1_3"), HDRL_EPS_TEST);
  cpl_test_abs(cd31, cpl_propertylist_get_double(plist, "CD3_1"), HDRL_EPS_TEST);
  cpl_test_abs(cd23, cpl_propertylist_get_double(plist, "CD2_3"), HDRL_EPS_TEST);
  cpl_test_abs(cd32, cpl_propertylist_get_double(plist, "CD3_2"), HDRL_EPS_TEST);
  cpl_test_abs(cd33, cpl_propertylist_get_double(plist, "CD3_3"), HDRL_EPS_TEST);

  cpl_propertylist_delete(plist);
  cpl_wcs_delete(wcs);
  cpl_test_error(CPL_ERROR_NONE);
  return cpl_error_get_code();
}

static cpl_error_code
test_resample_invalid_input_outgrid_param(
    const double delta_ra,
    const double delta_dec,
    const double delta_lambda,
    const double ra_min,
    const  double ra_max,
    const  double dec_min,
    const  double dec_max,
    const  double lambda_min,
    const  double lambda_max,
    const  double field_margin) {

  hdrl_parameter* pErr = NULL;

  pErr = hdrl_resample_parameter_create_outgrid2D(0, delta_dec);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_outgrid2D(delta_ra, 0);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_outgrid3D(0, delta_dec, delta_lambda);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_outgrid3D(delta_ra, 0, delta_lambda);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_outgrid3D(delta_ra, delta_dec, 0);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr =
      hdrl_resample_parameter_create_outgrid2D_userdef(0, delta_dec, ra_min, ra_max,
						       dec_min, dec_max, field_margin);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr =
      hdrl_resample_parameter_create_outgrid2D_userdef(delta_ra, 0, ra_min, ra_max,
						       dec_min, dec_max, field_margin);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_outgrid2D_userdef(delta_ra, 0, -1,
							  ra_max, dec_min, dec_max, field_margin);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_outgrid2D_userdef(delta_ra, 0, ra_min,
							  -1, dec_min, dec_max, field_margin);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_outgrid2D_userdef(delta_ra, delta_dec,
							  2, 1, dec_min, dec_max, field_margin);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_outgrid2D_userdef(delta_ra, delta_dec,
							  ra_min, ra_max, 2, 1, field_margin);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_outgrid2D_userdef(delta_ra, delta_dec,
							  ra_min, ra_max, dec_min, dec_max, -1);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_outgrid3D_userdef(delta_ra, delta_dec,
							  delta_lambda, -1, ra_max,
							  dec_min, dec_max, lambda_min,
							  lambda_max,field_margin);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_outgrid3D_userdef(delta_ra, delta_dec,
							  delta_lambda, ra_min, -1,
							  dec_min, dec_max, lambda_min,
							  lambda_max,field_margin);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_outgrid3D_userdef(delta_ra, delta_dec,
							  delta_lambda, 2, 1, dec_min,
							  dec_max, lambda_min,
							  lambda_max,field_margin);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_outgrid3D_userdef(delta_ra, delta_dec,
							  delta_lambda, ra_min, ra_max,
							  2, 1, lambda_min,
							  lambda_max,field_margin);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_outgrid3D_userdef(delta_ra, delta_dec,
							  delta_lambda, ra_min, ra_max,
							  dec_min, dec_max, -1,
							  lambda_max,field_margin);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_outgrid3D_userdef(delta_ra, delta_dec,
							  delta_lambda, ra_min, ra_max,
							  dec_min, dec_max, lambda_min,
							  -1,field_margin);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_outgrid3D_userdef(delta_ra, delta_dec,
							  delta_lambda, ra_min, ra_max,
							  dec_min, dec_max, 2,
							  1,field_margin);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_outgrid3D_userdef(delta_ra, delta_dec,
							  delta_lambda, ra_min, ra_max,
							  dec_min, dec_max, lambda_min,
							  lambda_max,-1);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check hdrl_resample_parameter_create_outgrid2D/3D_userdef
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_resample_outgrid_param(void) {
  /* test invalid input */
  const double delta_ra = 0.1;
  const double delta_dec = 0.2;
  const double delta_lambda = 0.001;

  hdrl_resample_outgrid_parameter * aParams_outputgrid =
      cpl_calloc (1, sizeof(hdrl_resample_outgrid_parameter));

  aParams_outputgrid->delta_ra = delta_ra;
  aParams_outputgrid->delta_dec = delta_dec;
  aParams_outputgrid->delta_lambda = delta_lambda;
  aParams_outputgrid->wcs = hdrl_resample_util_crea_wcs_2d();

  double ra_min = HDRL_RA_MIN;         /* Minimal Right ascension [deg] */
  double ra_max = HDRL_RA_MIN;         /* Maximal Right ascension [deg] */
  double dec_min = HDRL_DEC_MIN;       /* Minimal Declination [deg] */
  double dec_max = HDRL_DEC_MIN;       /* Maximal Declination [deg] */
  double lambda_min = HDRL_LAMBDA_MIN; /* Minimal wavelength [m] */
  double lambda_max = HDRL_LAMBDA_MIN; /* Maximal wavelength [m] */
  double field_margin = 5;
  aParams_outputgrid->dec_min = dec_min;
  aParams_outputgrid->dec_max = dec_max;
  aParams_outputgrid->ra_min = ra_min;
  aParams_outputgrid->ra_max = ra_max;
  aParams_outputgrid->lambda_min = lambda_min;
  aParams_outputgrid->lambda_max = lambda_max;
  hdrl_parameter* pErr = NULL;
  cpl_wcs_delete((cpl_wcs *)aParams_outputgrid->wcs);

  /* test invalid input */
  test_resample_invalid_input_outgrid_param(delta_ra, delta_dec, delta_lambda,
					    ra_min, ra_max, dec_min, dec_max, lambda_min, lambda_max, field_margin);

  /* test valid input */
  pErr = hdrl_resample_parameter_create_outgrid2D(delta_ra, delta_dec);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(pErr);
  cpl_test(hdrl_resample_parameter_outgrid_check(pErr));
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_eq(CPL_ERROR_NONE, hdrl_resample_parameter_outgrid_verify(pErr));
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_parameter_delete(pErr);
  pErr = hdrl_resample_parameter_create_outgrid3D(delta_ra, delta_dec, delta_lambda);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(pErr);
  cpl_test(hdrl_resample_parameter_outgrid_check(pErr));
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_eq(CPL_ERROR_NONE, hdrl_resample_parameter_outgrid_verify(pErr));
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_parameter_delete(pErr);
  pErr =
      hdrl_resample_parameter_create_outgrid2D_userdef(delta_ra, delta_dec,
						       ra_min, ra_max,
						       dec_min, dec_max, field_margin);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(pErr);
  cpl_test(hdrl_resample_parameter_outgrid_check(pErr));
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_eq(CPL_ERROR_NONE, hdrl_resample_parameter_outgrid_verify(pErr));
  cpl_test_error(CPL_ERROR_NONE);


  hdrl_parameter_delete(pErr);
  pErr = hdrl_resample_parameter_create_outgrid3D_userdef(delta_ra, delta_dec,
							  delta_lambda, ra_min, ra_max,
							  dec_min, dec_max, lambda_min,
							  lambda_max,field_margin);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(pErr);

  cpl_test(hdrl_resample_parameter_outgrid_check(pErr));
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_eq(CPL_ERROR_NONE, hdrl_resample_parameter_outgrid_verify(pErr));
  cpl_test_error(CPL_ERROR_NONE);
  hdrl_parameter_delete(pErr);

  cpl_free(aParams_outputgrid);

  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check hdrl_resample_parameter_create_nearest()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_parameter_create_nearest(void) {
  /* test invalid input */
  hdrl_parameter *pErr;

  pErr = 	hdrl_resample_parameter_create_nearest();
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(pErr);
  hdrl_parameter_delete(pErr);

  hdrl_parameter* p = hdrl_resample_parameter_create_nearest();
  cpl_test_nonnull(p);

  cpl_test(hdrl_resample_parameter_method_check(p));
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_eq(CPL_ERROR_NONE, hdrl_resample_parameter_method_verify(p));
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_parameter_delete(p);

  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check hdrl_resample_parameter_create_lanczos()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_parameter_create_lanczos(void) {
  /* test invalid input */
  const double loop_distance = 2;
  const int kernel_size = 2;
  cpl_boolean use_errorweights= CPL_TRUE;
  hdrl_parameter *pErr;
  /* test invalid input */
  pErr = 	hdrl_resample_parameter_create_lanczos(1, use_errorweights, 0);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  pErr = 	hdrl_resample_parameter_create_lanczos(-1, use_errorweights,
						       kernel_size);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);
  cpl_test_error(CPL_ERROR_NONE);

  /* test valid input */
  hdrl_parameter* p = hdrl_resample_parameter_create_lanczos(loop_distance,
							     use_errorweights,
							     kernel_size);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(p);

  cpl_test(hdrl_resample_parameter_method_check(p));
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_eq(CPL_ERROR_NONE, hdrl_resample_parameter_method_verify(p));
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_parameter_delete(p);

  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check hdrl_resample_parameter_create_linear()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_parameter_create_linear(void) {
  /* test invalid input */
  const double loop_distance = 2;
  cpl_boolean use_errorweights= CPL_TRUE;

  hdrl_parameter *pErr;
  /* test invalid input */
  pErr = 	hdrl_resample_parameter_create_linear(-1, use_errorweights);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  /* test valid input */
  hdrl_parameter* p = hdrl_resample_parameter_create_linear(loop_distance,
							    use_errorweights);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(p);

  cpl_test(hdrl_resample_parameter_method_check(p));
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_eq(CPL_ERROR_NONE, hdrl_resample_parameter_method_verify(p));
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_parameter_delete(p);

  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check hdrl_resample_parameter_create_quadratic()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_parameter_create_quadratic(void) {
  /* test invalid input */
  const double loop_distance = 2;
  cpl_boolean use_errorweights= CPL_TRUE;
  hdrl_parameter *pErr;

  /* test invalid input */
  pErr = 	hdrl_resample_parameter_create_quadratic(-1, use_errorweights);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  /* test valid input */
  hdrl_parameter* p = hdrl_resample_parameter_create_quadratic(loop_distance,
							       use_errorweights);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(p);

  cpl_test(hdrl_resample_parameter_method_check(p));
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_eq(CPL_ERROR_NONE, hdrl_resample_parameter_method_verify(p));
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_parameter_delete(p);

  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check hdrl_resample_parameter_create_renka()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_parameter_create_renka(void) {
  /* test invalid input */
  const double loop_distance = 2;
  const double critical_radius_renka = 3;
  cpl_boolean use_errorweights= CPL_TRUE;
  hdrl_parameter *pErr;

  /* test invalid input */
  pErr = hdrl_resample_parameter_create_renka(-1, use_errorweights,
					      critical_radius_renka);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_renka(loop_distance,
					      use_errorweights, -1);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  /* test valid input */
  hdrl_parameter* p =
      hdrl_resample_parameter_create_renka(loop_distance, use_errorweights,
					   critical_radius_renka);

  cpl_test(hdrl_resample_parameter_method_check(p));
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_eq(CPL_ERROR_NONE, hdrl_resample_parameter_method_verify(p));
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_parameter_delete(p);

  return cpl_error_get_code();
}
static cpl_error_code
test_invalid_input_hdrl_resample_parameter_create_drizzle(
    const double loop_distance,
    const double pix_frac_drizzle_x,
    const double pix_frac_drizzle_y,
    const double pix_frac_drizzle_lambda,
    cpl_boolean use_errorweights
)
{
  hdrl_parameter *pErr;

  /* test invalid input */
  pErr = hdrl_resample_parameter_create_drizzle(-1, use_errorweights,
						pix_frac_drizzle_x,
						pix_frac_drizzle_y,
						pix_frac_drizzle_lambda);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_drizzle(loop_distance, use_errorweights,
						-1,
						pix_frac_drizzle_y,
						pix_frac_drizzle_lambda);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_drizzle(loop_distance, use_errorweights,
						pix_frac_drizzle_x,
						-1,
						pix_frac_drizzle_lambda);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_drizzle(loop_distance, use_errorweights,
						pix_frac_drizzle_x,
						pix_frac_drizzle_y,
						-1);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check hdrl_resample_parameter_create_drizzle()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_parameter_create_drizzle(void) {
  /* test invalid input */
  const double loop_distance = 2;
  const double pix_frac_drizzle_x = 0.8;
  const double pix_frac_drizzle_y = 0.8;
  const double pix_frac_drizzle_lambda = 1;
  cpl_boolean use_errorweights= CPL_TRUE;

  /* test valid input */
  test_invalid_input_hdrl_resample_parameter_create_drizzle(loop_distance,
							    pix_frac_drizzle_x, pix_frac_drizzle_y, pix_frac_drizzle_lambda,
							    use_errorweights);

  /* test valid input */
  hdrl_parameter* p =
      hdrl_resample_parameter_create_drizzle(loop_distance, use_errorweights,
					     pix_frac_drizzle_x,
					     pix_frac_drizzle_y,
					     pix_frac_drizzle_lambda);

  cpl_test(hdrl_resample_parameter_method_check(p));
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_eq(CPL_ERROR_NONE, hdrl_resample_parameter_method_verify(p));
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_parameter_delete(p);

  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check all functions for all supported resampled methods
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_resample_parameters(void) {

  test_resample_outgrid_param();
  cpl_test_error(CPL_ERROR_NONE);

  test_hdrl_resample_parameter_create_nearest();
  cpl_test_error(CPL_ERROR_NONE);

  test_hdrl_resample_parameter_create_lanczos();
  cpl_test_error(CPL_ERROR_NONE);

  test_hdrl_resample_parameter_create_linear();
  cpl_test_error(CPL_ERROR_NONE);

  test_hdrl_resample_parameter_create_quadratic();
  cpl_test_error(CPL_ERROR_NONE);

  test_hdrl_resample_parameter_create_renka();
  cpl_test_error(CPL_ERROR_NONE);

  test_hdrl_resample_parameter_create_drizzle();
  cpl_test_error(CPL_ERROR_NONE);

  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check hdrl_resample_parameter_create_outgrid2D
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_parameter_create_outgrid2D(void){
  const double delta_ra = 0.1;
  const double delta_dec = 0.1;
  hdrl_parameter *pErr;
  /* test invalid input: nothing all values are allowed */
  /* test valid input */
  pErr = hdrl_resample_parameter_create_outgrid2D(delta_ra, delta_dec);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(pErr);

  hdrl_resample_parameter_outgrid_verify(pErr);
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_resample_parameter_outgrid_check(pErr);
  cpl_test_error(CPL_ERROR_NONE);
  hdrl_parameter_delete(pErr);
  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check hdrl_resample_parameter_create_outgrid3D
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_parameter_create_outgrid3D(void){
  const double delta_ra = 0.1;
  const double delta_dec = 0.1;
  const double delta_lambda = 0.1;
  hdrl_parameter *pErr;

  /* test invalid input: nothing all values are allowed */

  /* test valid input */
  pErr = hdrl_resample_parameter_create_outgrid3D(delta_ra, delta_dec, delta_lambda);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(pErr);

  hdrl_resample_parameter_outgrid_verify(pErr);
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_resample_parameter_outgrid_check(pErr);
  cpl_test_error(CPL_ERROR_NONE);
  hdrl_parameter_delete(pErr);
  return cpl_error_get_code();
}

static cpl_error_code
test_invalid_input_hdrl_resample_parameter_create_outgrid2D_userdef(
    const double delta_ra,
    const double delta_dec,
    const double ra_min,
    const double ra_max,
    const double dec_min,
    const double dec_max,
    const double fieldmargin
){

  hdrl_parameter *pErr;

  /* test invalid input */
  pErr = hdrl_resample_parameter_create_outgrid2D_userdef(delta_ra, delta_dec,
							  1, 0,
							  dec_min, dec_max,
							  fieldmargin);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_outgrid2D_userdef(delta_ra, delta_dec,
							  ra_min, ra_max,
							  1, 0,
							  fieldmargin);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr = hdrl_resample_parameter_create_outgrid2D_userdef(delta_ra, delta_dec,
							  ra_min, ra_max,
							  dec_min, dec_max,
							  -1);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check hdrl_resample_parameter_create_outgrid2D_userdef
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_parameter_create_outgrid2D_userdef(void){
  const double delta_ra = 0.1;
  const double delta_dec = 0.1;
  const double ra_min = 1;
  const double ra_max = 2;
  const double dec_min = 0;
  const double dec_max = 1;
  double fieldmargin = 5;
  hdrl_parameter *pErr;

  /* test invalid input */
  test_invalid_input_hdrl_resample_parameter_create_outgrid2D_userdef(delta_ra,
								      delta_dec,
								      ra_min, ra_max, dec_min, dec_max, fieldmargin);
  /* test valid input */
  pErr = hdrl_resample_parameter_create_outgrid2D_userdef(delta_ra, delta_dec,
							  ra_min, ra_max,
							  dec_min, dec_max,
							  fieldmargin);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(pErr);

  hdrl_resample_parameter_outgrid_verify(pErr);
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_resample_parameter_outgrid_check(pErr);
  cpl_test_error(CPL_ERROR_NONE);
  hdrl_parameter_delete(pErr);
  return cpl_error_get_code();
}
static cpl_error_code
test_invalid_input_hdrl_resample_parameter_create_outgrid3D_userdef(
    const double delta_ra,
    const double delta_dec,
    const double delta_lambda,
    const double ra_min,
    const double ra_max,
    const double dec_min,
    const double dec_max,
    const double lambda_min,
    const double lambda_max,
    const double fieldmargin
){
  hdrl_parameter *pErr;

  /* test invalid input */
  pErr =
      hdrl_resample_parameter_create_outgrid3D_userdef(delta_ra, delta_dec, delta_lambda,
						       2, 1,
						       dec_min, dec_max,
						       lambda_min, lambda_max,
						       fieldmargin);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr =
      hdrl_resample_parameter_create_outgrid3D_userdef(delta_ra, delta_dec, delta_lambda,
						       ra_min, ra_max,
						       2, 1,
						       lambda_min, lambda_max,
						       fieldmargin);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr =
      hdrl_resample_parameter_create_outgrid3D_userdef(delta_ra, delta_dec, delta_lambda,
						       ra_min, ra_max,
						       dec_min, dec_max,
						       550, 500,
						       fieldmargin);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  pErr =
      hdrl_resample_parameter_create_outgrid3D_userdef(delta_ra, delta_dec, delta_lambda,
						       ra_min, ra_max,
						       dec_min, dec_max,
						       lambda_min, lambda_max,
						       -1);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pErr);

  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check hdrl_resample_parameter_create_outgrid3D_userdef
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_parameter_create_outgrid3D_userdef(void){
  const double delta_ra = 0.1;
  const double delta_dec = 0.1;
  const double delta_lambda = 0.1;
  const double ra_min = 1;
  const double ra_max = 2;
  const double dec_min = 0;
  const double dec_max = 1;
  const double lambda_min = 500;
  const double lambda_max = 550;

  double fieldmargin = 5;
  hdrl_parameter *pErr;

  /* test invalid input */
  test_invalid_input_hdrl_resample_parameter_create_outgrid3D_userdef(delta_ra,
								      delta_dec, delta_lambda, ra_min, ra_max, dec_min, dec_max, lambda_min,
								      lambda_max, fieldmargin);
  /* test valid input */
  pErr =
      hdrl_resample_parameter_create_outgrid3D_userdef(delta_ra, delta_dec, delta_lambda,
						       ra_min, ra_max,
						       dec_min, dec_max,
						       lambda_min, lambda_max,
						       fieldmargin);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(pErr);

  hdrl_resample_parameter_outgrid_verify(pErr);
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_resample_parameter_outgrid_check(pErr);
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_parameter_delete(pErr);
  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check hdrl_resample_wcs_print()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_wcs_print(void){

  cpl_wcs* wcs = NULL;

  /* verify invalid input */
  hdrl_resample_wcs_print(NULL);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  /* verify valid input: 3D case */
  wcs = hdrl_resample_util_crea_wcs_3d();
  hdrl_resample_wcs_print(wcs);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_wcs_delete(wcs);

  /* verify valid input: 2D case */
  wcs = hdrl_resample_util_crea_wcs_2d();
  hdrl_resample_wcs_print(wcs);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_wcs_delete(wcs);

  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check hdrl_resample_wcs_print()
  @param aParams_outputgrid outputgrid parameter
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
hdrl_resample_util_fill_outputgrid(hdrl_resample_outgrid_parameter * aParams_outputgrid)
{
  /*Assign the wcs*/
  cpl_wcs* wcs = hdrl_resample_util_crea_wcs_2d();
  cpl_test_nonnull(wcs);

  aParams_outputgrid->wcs = wcs;

  /* Recalculate the limits if the user did not specify any */
  cpl_boolean recalc_limits = aParams_outputgrid->recalc_limits;

  if (recalc_limits == CPL_TRUE) {

      aParams_outputgrid->ra_min = HDRL_RA_MIN;
      aParams_outputgrid->ra_max = HDRL_RA_MAX;
      aParams_outputgrid->dec_min = HDRL_DEC_MIN;
      aParams_outputgrid->dec_max = HDRL_DEC_MAX;
      aParams_outputgrid->lambda_min = HDRL_LAMBDA_MIN;
      aParams_outputgrid->lambda_max = HDRL_LAMBDA_MAX;
  }
  cpl_test_error(CPL_ERROR_NONE);
  return cpl_error_get_code();

}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check hdrl_resample_outgrid_parameter_print()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_outgrid_parameter_print(void){

  /* verify invalid input */
  hdrl_resample_outgrid_parameter_print(NULL);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_parameter *  outputgrid =
      hdrl_resample_parameter_create_outgrid2D(1e-5, 1e-5);

  hdrl_resample_outgrid_parameter *aParams_outputgrid =
      (hdrl_resample_outgrid_parameter *) outputgrid;

  cpl_test_nonnull(aParams_outputgrid);

  /*Assign the wcs*/
  hdrl_resample_util_fill_outputgrid(aParams_outputgrid);
  cpl_test_error(CPL_ERROR_NONE);

  /* verify valid input */
  hdrl_resample_outgrid_parameter_print(aParams_outputgrid);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_wcs_delete((cpl_wcs *)aParams_outputgrid->wcs);
  cpl_free(aParams_outputgrid);

  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check hdrl_resample_method_parameter_print()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_method_parameter_print(void){

  /* verify invalid input */
  hdrl_resample_method_parameter_print(NULL);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_parameter * wrong = hdrl_resample_parameter_create_outgrid2D(0.1, 0.1);
  hdrl_resample_method_parameter * pwrong =
      (hdrl_resample_method_parameter *) wrong;
  cpl_test_nonnull(pwrong);
  hdrl_resample_method_parameter_print(pwrong);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_free(pwrong);

  /* verify valid input */
  hdrl_parameter * method = hdrl_resample_parameter_create_nearest();
  hdrl_resample_method_parameter * p =
      (hdrl_resample_method_parameter *) method;
  cpl_test_nonnull(p);

  hdrl_resample_method_parameter_print(p);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_free(p);
  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check hdrl_wcs_xy_to_radec()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_wcs_xy_to_radec(void) {

  double x = 2;
  double y = 2;
  double ra = 2;
  double dec = 2;

  /* verify invalid input */
  hdrl_wcs_xy_to_radec(NULL, x, y, &ra, &dec);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  cpl_wcs* wcs = hdrl_resample_util_crea_wcs_2d();
  cpl_test_nonnull(wcs);

  hdrl_wcs_xy_to_radec(wcs, x, y, NULL, &dec);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_wcs_xy_to_radec(wcs, x, y, &ra, NULL);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  /* verify valid input */
  hdrl_wcs_xy_to_radec(wcs, x, y, &ra, &dec);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_msg_warning(cpl_func,"ra: %20.18g",ra);
  cpl_msg_warning(cpl_func,"dec: %20.18g",dec);
  cpl_test_abs(48.0716937492999961, ra, HDRL_EPS_TEST);
  cpl_test_abs(-20.6229937493000008, dec, HDRL_EPS_TEST);

  cpl_wcs_delete(wcs);
  cpl_test_error(CPL_ERROR_NONE);
  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check hdrl_resample_pfits_get()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_pfits_get(void) {

  cpl_wcs* wcs = hdrl_resample_util_crea_wcs_3d();
  cpl_test_nonnull(wcs);
  cpl_propertylist* header = cpl_propertylist_new();
  hdrl_wcs_to_propertylist(wcs, header, CPL_FALSE);
  cpl_test_nonnull(header);
  double crpix = 0;
  double crval = 0;
  double cd = 0;

  /* test invalid input */
  crpix = hdrl_resample_pfits_get_crpix(NULL, 1);
  cpl_test_error(CPL_ERROR_NULL_INPUT);
  cpl_test_abs(crpix, 0, HDRL_EPS_TEST);

  crval = hdrl_resample_pfits_get_crval(NULL, 1);
  cpl_test_error(CPL_ERROR_NULL_INPUT);
  cpl_test_abs(crval, 0, HDRL_EPS_TEST);

  cd = hdrl_resample_pfits_get_cd(NULL, 1, 1);
  cpl_test_error(CPL_ERROR_NULL_INPUT);
  cpl_test_abs(cd, 0, HDRL_EPS_TEST);

  /* test valid input */
  cpl_test_abs(HDRL_CRPIX1,hdrl_resample_pfits_get_crpix(header, 1), HDRL_EPS_TEST);
  cpl_test_abs(HDRL_CRPIX2,hdrl_resample_pfits_get_crpix(header, 2), HDRL_EPS_TEST);
  cpl_test_abs(HDRL_CRPIX3,hdrl_resample_pfits_get_crpix(header, 3), HDRL_EPS_TEST);

  cpl_test_abs(HDRL_CRVAL1,hdrl_resample_pfits_get_crval(header, 1), HDRL_EPS_TEST);
  cpl_test_abs(HDRL_CRVAL2,hdrl_resample_pfits_get_crval(header, 2), HDRL_EPS_TEST);
  cpl_test_abs(HDRL_CRVAL3,hdrl_resample_pfits_get_crval(header, 3), HDRL_EPS_TEST);

  cpl_test_abs(HDRL_CD11,hdrl_resample_pfits_get_cd(header, 1, 1), HDRL_EPS_TEST);
  cpl_test_abs(HDRL_CD12,hdrl_resample_pfits_get_cd(header, 1, 2), HDRL_EPS_TEST);
  cpl_test_abs(HDRL_CD21,hdrl_resample_pfits_get_cd(header, 2, 1), HDRL_EPS_TEST);
  cpl_test_abs(HDRL_CD22,hdrl_resample_pfits_get_cd(header, 2, 2), HDRL_EPS_TEST);

  cpl_test_abs(HDRL_CD13,hdrl_resample_pfits_get_cd(header, 1, 3), HDRL_EPS_TEST);
  cpl_test_abs(HDRL_CD31,hdrl_resample_pfits_get_cd(header, 3, 1), HDRL_EPS_TEST);
  cpl_test_abs(HDRL_CD23,hdrl_resample_pfits_get_cd(header, 2, 3), HDRL_EPS_TEST);
  cpl_test_abs(HDRL_CD32,hdrl_resample_pfits_get_cd(header, 3, 1), HDRL_EPS_TEST);
  cpl_test_abs(HDRL_CD33,hdrl_resample_pfits_get_cd(header, 3, 3), HDRL_EPS_TEST);

  cpl_propertylist_delete(header);
  cpl_wcs_delete(wcs);
  cpl_test_error(CPL_ERROR_NONE);
  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check hdrl_resample_smallwcs_new()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_smallwcs_new(void){

  cpl_wcs* wcs = hdrl_resample_util_crea_wcs_3d();
  cpl_test_nonnull(wcs);
  cpl_propertylist* header = cpl_propertylist_new();
  hdrl_wcs_to_propertylist(wcs, header, CPL_FALSE);
  cpl_test_nonnull(header);

  /* test invalid input */
  hdrl_resample_smallwcs* swcs = NULL;
  swcs = hdrl_resample_smallwcs_new(NULL);
  cpl_test_error(CPL_ERROR_NULL_INPUT);
  cpl_test_null(swcs);

  /* test valid input */
  swcs = hdrl_resample_smallwcs_new(header);
  cpl_test_nonnull(swcs);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(swcs);

  cpl_test_rel(swcs->cd11, HDRL_CD11, HDRL_EPS_TEST);
  cpl_test_rel(swcs->cd22, HDRL_CD22, HDRL_EPS_TEST);
  cpl_test_rel(swcs->cd12, HDRL_CD12, HDRL_EPS_TEST);
  cpl_test_rel(swcs->cd21, HDRL_CD21, HDRL_EPS_TEST);
  cpl_test_rel(swcs->crpix1, HDRL_CRPIX1, HDRL_EPS_TEST);
  cpl_test_rel(swcs->crpix2, HDRL_CRPIX2, HDRL_EPS_TEST);
  cpl_test_rel(swcs->crval1, HDRL_CRVAL1, HDRL_EPS_TEST);
  cpl_test_rel(swcs->crval2, HDRL_CRVAL2, HDRL_EPS_TEST);

  cpl_free(swcs);
  cpl_propertylist_erase(header, "CRPIX1");
  swcs = hdrl_resample_smallwcs_new(header);
  cpl_test_nonnull(swcs);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(swcs);
  cpl_test_rel(swcs->crpix1, 0., HDRL_EPS_TEST);

  cpl_free(swcs);
  cpl_propertylist_erase(header, "CD1_1");
  swcs = hdrl_resample_smallwcs_new(header);
  cpl_test_nonnull(swcs);
  cpl_test_error(CPL_ERROR_SINGULAR_MATRIX);

  cpl_free(swcs);
  cpl_propertylist_erase_regexp(header, "CD?_?", 0);
  swcs = hdrl_resample_smallwcs_new(header);
  cpl_test_nonnull(swcs);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(swcs);
  cpl_test_rel(swcs->crpix1, 0., HDRL_EPS_TEST);

  cpl_free(swcs);
  cpl_wcs_delete(wcs);
  cpl_propertylist_delete(header);
  cpl_test_error(CPL_ERROR_NONE);
  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief utility to generate a pixel grid
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static hdrl_resample_pixgrid*
hdrl_resample_util_pixgrid_new(void) {

  cpl_size aSizeX = 10;
  cpl_size aSizeY = 10;
  cpl_size aSizeZ = 10;
  unsigned short aNMaps = 10;

  /* test valid input */
  hdrl_resample_pixgrid *aGrid =
      hdrl_resample_pixgrid_new(aSizeX, aSizeY, aSizeZ, aNMaps);

  return aGrid;
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check hdrl_resample_pixgrid_delete()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_pixgrid_delete(void) {

  /* test invalid input */
  hdrl_resample_pixgrid_delete(NULL);
  /* Nothing to check on error code as in this case function returns void */

  /* test valid input */
  hdrl_resample_pixgrid *aGrid = hdrl_resample_util_pixgrid_new();
  cpl_test_nonnull(aGrid);

  hdrl_resample_pixgrid_delete(aGrid);
  cpl_test_error(CPL_ERROR_NONE);

  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief utility to generate a pixel table
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_table*
hdrl_resample_util_crea_pixel_table(void)
{
  cpl_wcs* wcs = hdrl_resample_util_crea_wcs_3d();
  cpl_imagelist* ilist = NULL;
  cpl_imagelist* elist = NULL;

  ilist = cpl_imagelist_new();
  elist = cpl_imagelist_new();
  cpl_image* simul = hdrl_resample_util_crea_image_dice_5(HDRL_SIZE_X, HDRL_SIZE_X,
							  HDRL_FLUX_ADU);
  cpl_image* errs = cpl_image_duplicate(simul);
  cpl_image_power(errs, 0.5);
  cpl_imagelist_set(ilist, simul, 0);
  cpl_imagelist_set(elist, errs, 0);
  hdrl_imagelist* hlist = hdrl_resample_util_hdrl_imagelist_create(ilist,
								   elist, NULL);
  cpl_table* pixel_table = hdrl_resample_imagelist_to_table(hlist, wcs);

  cpl_imagelist_delete(ilist);
  cpl_imagelist_delete(elist);
  hdrl_imagelist_delete(hlist);

  cpl_wcs_delete(wcs);
  cpl_test_error(CPL_ERROR_NONE);

  return pixel_table;
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test to check hdrl_resample_compute_method()
  @param aMethod a resample method
  @param use_errorweights use error weights?
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/

static cpl_error_code
test_hdrl_resample_compute_method(hdrl_resample_method aMethod,
				  cpl_boolean use_errorweights) {

  const double delta_ra = HDRL_CDELT1;
  const double delta_dec = HDRL_CDELT2;
  const double delta_lambda = HDRL_CDELT3;
  const double ramin = HDRL_RA_MIN;
  const double ramax = HDRL_RA_MAX;
  const double decmin = HDRL_DEC_MIN;
  const double decmax = HDRL_DEC_MAX;
  const double lambmin = HDRL_LAMBDA_MIN;
  const double lambmax = HDRL_LAMBDA_MAX;

  cpl_wcs* wcs = hdrl_resample_util_crea_wcs_3d();

  cpl_table* pixel_table = hdrl_resample_util_crea_pixel_table();

  hdrl_parameter * aParams_method =
      hdrl_resample_util_methodparam_create (
	  LOOP_DISTANCE,
	  RENKA_CRITICAL_RADIUS,
	  LANCZOS_KERNEL_SIZE,
	  DRIZZLE_DOWN_SCALING_FACTOR_X,
	  DRIZZLE_DOWN_SCALING_FACTOR_Y,
	  DRIZZLE_DOWN_SCALING_FACTOR_Z,
	  aMethod, use_errorweights);

  hdrl_parameter* aParams_outputgrid =
      hdrl_resample_parameter_create_outgrid3D_userdef(delta_ra, delta_dec,
						       delta_lambda, ramin, ramax, decmin, decmax, lambmin, lambmax, 0.);
  /* we use 0 field margin to check later NAXISi values vs cube expected size */

  /* Create resampling table starting from a data cube */
  int sx = HDRL_SIZE_X, sy = HDRL_SIZE_Z, sz = HDRL_SIZE_Z;
  cpl_image* ima = cpl_image_new(sx, sy, CPL_TYPE_INT);
  cpl_image_add_scalar(ima, 1);
  cpl_imagelist* iml = cpl_imagelist_new();
  for(int i = 0; i< sz;i++) {
      cpl_imagelist_set(iml, ima, i);
  }
  hdrl_imagelist* hlist = hdrl_resample_util_hdrl_imagelist_create(iml,
								   NULL, NULL);

  cpl_table * ResTable = hdrl_resample_imagelist_to_table(hlist, wcs);
  cpl_imagelist_delete(iml);
  hdrl_imagelist_delete(hlist);

  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(ResTable);
  cpl_test_nonnull(aParams_method);
  cpl_test_nonnull(aParams_outputgrid);
  cpl_test_nonnull(wcs);
  hdrl_resample_result *cube =
      hdrl_resample_compute(ResTable, aParams_method, aParams_outputgrid,
			    wcs);
  cpl_test_nonnull(cube);
  cpl_test_error(CPL_ERROR_NONE);

  /* test valid input */
  /* test 3D case */
  hdrl_parameter_delete(aParams_outputgrid);

  aParams_outputgrid =
      hdrl_resample_parameter_create_outgrid3D(delta_ra, delta_dec, delta_lambda);

  /* test case: recalc_limits == CPL_TRUE */
  hdrl_resample_result_delete(cube);
  cube = hdrl_resample_compute(ResTable, aParams_method, aParams_outputgrid,
			       wcs);
  cpl_test_nonnull(cube);
  cpl_test_error(CPL_ERROR_NONE);

  /* test 2D case */
  hdrl_parameter_delete(aParams_outputgrid);
  aParams_outputgrid =
      hdrl_resample_parameter_create_outgrid2D_userdef(delta_ra, delta_dec,
						       ramin, ramax,
						       decmin, decmax,
						       5.);

  hdrl_resample_result_delete(cube);
  cube = hdrl_resample_compute(ResTable, aParams_method, aParams_outputgrid,
			       wcs);
  cpl_test_nonnull(cube);
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_parameter_delete(aParams_outputgrid);
  hdrl_parameter_delete(aParams_method);
  cpl_table_delete(ResTable);
  cpl_wcs_delete(wcs);
  cpl_table_delete(pixel_table);
  hdrl_resample_result_delete(cube);

  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test all functions that implement the supported resampling methods
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_compute(void) {

  test_hdrl_resample_compute_method(HDRL_RESAMPLE_METHOD_NEAREST, CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);
  test_hdrl_resample_compute_method(HDRL_RESAMPLE_METHOD_RENKA, CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);
  test_hdrl_resample_compute_method(HDRL_RESAMPLE_METHOD_LINEAR, CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);
  test_hdrl_resample_compute_method(HDRL_RESAMPLE_METHOD_QUADRATIC, CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);
  test_hdrl_resample_compute_method(HDRL_RESAMPLE_METHOD_DRIZZLE, CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);
  test_hdrl_resample_compute_method(HDRL_RESAMPLE_METHOD_LANCZOS, CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);
  test_hdrl_resample_compute_method(HDRL_RESAMPLE_METHOD_NONE, CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);
  test_hdrl_resample_compute_method(HDRL_RESAMPLE_METHOD_LINEAR, CPL_FALSE);
  cpl_test_error(CPL_ERROR_NONE);

  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test hdrl_resample_wcs_projplane_from_celestial()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_wcs_projplane_from_celestial(void) {

  hdrl_parameter *  outputgrid =
      hdrl_resample_parameter_create_outgrid2D(1e-5, 1e-5);

  hdrl_resample_outgrid_parameter *pogrid =
      (hdrl_resample_outgrid_parameter *) outputgrid;

  double ra = HDRL_RA;
  double dec = HDRL_DEC;
  double x_out = HDRL_RA;
  double y_out = HDRL_DEC;

  /* test invalid input */
  hdrl_resample_wcs_projplane_from_celestial(NULL, ra, dec, &x_out, &y_out);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  /* test valid input */
  hdrl_resample_util_fill_outputgrid(pogrid);
  hdrl_resample_wcs_projplane_from_celestial(pogrid, ra, dec, &x_out, &y_out);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(x_out,  -0.000561558354797, HDRL_DELTA_COMPARE_VALUE_ABS);
  cpl_test_abs(y_out,   0.000899999036676, HDRL_DELTA_COMPARE_VALUE_ABS);

  cpl_wcs_delete((cpl_wcs *)pogrid->wcs);
  cpl_free(pogrid);
  cpl_test_error(CPL_ERROR_NONE);
  return cpl_error_get_code();

}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test hdrl_resample_wcs_pixel_from_celestial_fast()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_wcs_pixel_from_celestial_fast(void){

  double ra = HDRL_RA;
  double dec = HDRL_DEC;
  double x_out = HDRL_RA;
  double y_out = HDRL_DEC;
  /* test invalid input */
  hdrl_resample_wcs_pixel_from_celestial_fast(NULL, ra, dec, &x_out, &y_out);
  cpl_wcs* wcs = hdrl_resample_util_crea_wcs_3d();
  cpl_test_nonnull(wcs);
  cpl_propertylist* header = cpl_propertylist_new();
  hdrl_wcs_to_propertylist(wcs, header, CPL_FALSE);
  cpl_test_nonnull(header);

  hdrl_resample_smallwcs * swcs = hdrl_resample_smallwcs_new(header);
  cpl_test_nonnull(swcs);

  hdrl_resample_wcs_pixel_from_celestial_fast(swcs, ra, dec, &x_out, &y_out);
  cpl_test_error(CPL_ERROR_NONE);

  /*
  This function uses many trigonometric functions and thus accumulates differences between
  e.g. linux 64 bit, linux 32 bit and macosx
   */
  cpl_test_rel(x_out,  -163.82544787203, 1.e-11);
  cpl_test_rel(y_out,  1518.66596649235, 1.e-11);

  cpl_free(swcs);
  cpl_wcs_delete(wcs);
  cpl_propertylist_delete(header);
  cpl_test_error(CPL_ERROR_NONE);
  return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief unit test hdrl_resample_compute_size()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_compute_size(void){

  hdrl_parameter *  outputgrid =
      hdrl_resample_parameter_create_outgrid2D(1e-5, 1e-5);

  hdrl_resample_outgrid_parameter *pogrid =
      (hdrl_resample_outgrid_parameter *) outputgrid;

  int x, y, z;

  /* test invalid input */
  hdrl_resample_compute_size(NULL, &x, &y, &z);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  /* test valid input */
  hdrl_resample_util_fill_outputgrid(pogrid);
  hdrl_resample_compute_size(pogrid, &x, &y, &z);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_eq(x, 225); /* values determined on fc32 */
  cpl_test_eq(y, 229); /* values determined on fc32 */
  cpl_test_eq(z, 2);   /* values determined on fc32 */

  cpl_wcs_delete((cpl_wcs *)pogrid->wcs);
  cpl_free(pogrid);
  cpl_test_error(CPL_ERROR_NONE);
  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test hdrl_resample_pixgrid_add()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_pixgrid_add(void){

  hdrl_resample_pixgrid *aGrid = hdrl_resample_util_pixgrid_new();
  cpl_test_nonnull(aGrid);

  cpl_size aIndex = 1;
  cpl_size aRow = 1;
  unsigned short aXIdx = 1;

  /* test invalid input */
  hdrl_resample_pixgrid_add(NULL, aIndex, aRow, aXIdx);
  hdrl_resample_pixgrid_add(aGrid, -1, aRow, aXIdx);

  /* test valid input */
  hdrl_resample_pixgrid_add(aGrid, aIndex, aRow, aXIdx);
  /* As hdrl_resample_pixgrid_add returns void we just test error code */
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_resample_pixgrid_delete(aGrid);
  cpl_test_error(CPL_ERROR_NONE);

  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test hdrl_resample_pixgrid_get_count()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_pixgrid_get_count(void){
  cpl_size aIndex = 1;
  /* test invalid input */
  hdrl_resample_pixgrid_get_count(NULL, aIndex);

  hdrl_resample_pixgrid *aGrid = hdrl_resample_util_pixgrid_new();
  cpl_test_nonnull(aGrid);

  hdrl_resample_pixgrid_get_count(aGrid, -1);

  /* test valid input */
  cpl_size n = hdrl_resample_pixgrid_get_count(aGrid, aIndex);

  cpl_test_rel(n, 0, HDRL_EPS_TEST);

  hdrl_resample_pixgrid_delete(aGrid);
  cpl_test_error(CPL_ERROR_NONE);
  return cpl_error_get_code();

}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test hdrl_resample_pixgrid_get_index()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_pixgrid_get_index(void)
{
  cpl_size x = 1;
  cpl_size y = 1;
  cpl_size z = 1;
  cpl_boolean aAllowOutside = CPL_TRUE;

  hdrl_resample_pixgrid *aGrid = hdrl_resample_util_pixgrid_new();
  cpl_test_nonnull(aGrid);
  cpl_msg_warning(cpl_func,"x: %lld",aGrid->nx);
  cpl_msg_warning(cpl_func,"y: %lld",aGrid->ny);
  cpl_msg_warning(cpl_func,"z: %lld",aGrid->nz);
  cpl_size index = 0;

  /* test invalid input */
  index = hdrl_resample_pixgrid_get_index(NULL, x, y, z, aAllowOutside);
  cpl_test_eq(index, -1);
  index = hdrl_resample_pixgrid_get_index(aGrid, -1, y, z, CPL_FALSE);
  cpl_test_eq(index, -1);
  index = hdrl_resample_pixgrid_get_index(aGrid, x, -1, z, CPL_FALSE);
  cpl_test_eq(index, -1);
  index = hdrl_resample_pixgrid_get_index(aGrid, x, y, -1, CPL_FALSE);
  cpl_test_eq(index, -1);

  index = hdrl_resample_pixgrid_get_index(aGrid, aGrid->nx+1, y, z, CPL_FALSE);
  cpl_test_eq(index, -1);
  index = hdrl_resample_pixgrid_get_index(aGrid, x, aGrid->ny+1, z, CPL_FALSE);
  cpl_test_eq(index, -1);
  index = hdrl_resample_pixgrid_get_index(aGrid, x, y, aGrid->nz+1, CPL_FALSE);
  cpl_test_eq(index, -1);

  index = hdrl_resample_pixgrid_get_index(aGrid, aGrid->nx + 1, y, z, CPL_TRUE);
  cpl_test_eq(index, 119);
  index = hdrl_resample_pixgrid_get_index(aGrid, x, aGrid->ny + 1, z, CPL_TRUE);
  cpl_test_eq(index, 191);
  index = hdrl_resample_pixgrid_get_index(aGrid, x, y, aGrid->nz + 1, CPL_TRUE);
  cpl_test_eq(index, 911);

  /* test valid input */
  cpl_size index_res = 111;
  cpl_msg_warning(cpl_func,"index: %lld",
		  hdrl_resample_pixgrid_get_index(aGrid, x, y, z, aAllowOutside));
  index = hdrl_resample_pixgrid_get_index(aGrid, x, y, z, aAllowOutside);
  cpl_test_eq(index_res, index);

  hdrl_resample_pixgrid_delete(aGrid);
  cpl_test_error(CPL_ERROR_NONE);
  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test hdrl_resample_pixgrid_new()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_pixgrid_new(void) {
  cpl_size x = 1;
  cpl_size y = 1;
  cpl_size z = 1;
  unsigned short aNMaps = 1;
  hdrl_resample_pixgrid * pGrid = NULL;

  /* test invalid input */
  pGrid = hdrl_resample_pixgrid_new(-1, y, z, aNMaps);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pGrid);

  pGrid = hdrl_resample_pixgrid_new(x, -1, z, aNMaps);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pGrid);

  pGrid = hdrl_resample_pixgrid_new(x, y, -1, aNMaps);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pGrid);

  pGrid = hdrl_resample_pixgrid_new(x, y, z, 0);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pGrid);

  /* test valid input */
  pGrid = hdrl_resample_pixgrid_new(x, y, z, aNMaps);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(pGrid);

  hdrl_resample_pixgrid_delete(pGrid);
  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test hdrl_resample_pixgrid_create()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_pixgrid_create(void) {

  cpl_size x = 1;
  cpl_size y = 1;
  cpl_size z = 1;
  cpl_table* pixel_table = NULL;
  cpl_wcs* wcs = hdrl_resample_util_crea_wcs_3d();
  cpl_test_nonnull(wcs);
  cpl_propertylist* header = cpl_propertylist_new();
  hdrl_wcs_to_propertylist(wcs, header, CPL_FALSE);
  hdrl_resample_pixgrid * pGrid= NULL;

  /* test invalid input */
  pixel_table = cpl_table_new(0);
  pGrid = hdrl_resample_pixgrid_create(pixel_table, header, x, y, z);
  cpl_test_error(CPL_ERROR_NULL_INPUT);
  cpl_test_null(pGrid);
  cpl_table_delete(pixel_table);

  pixel_table = cpl_table_new(1);
  pGrid = hdrl_resample_pixgrid_create(pixel_table, header, x, y, z);
  cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
  cpl_test_null(pGrid);
  cpl_table_delete(pixel_table);

  pixel_table = hdrl_resample_util_crea_pixel_table();

  /* test invalid input */
  pGrid = hdrl_resample_pixgrid_create(pixel_table, NULL, x, y, z);
  cpl_test_error(CPL_ERROR_NULL_INPUT);
  cpl_test_null(pGrid);

  pGrid = hdrl_resample_pixgrid_create(pixel_table, header, -1, y, z);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pGrid);

  pGrid = hdrl_resample_pixgrid_create(pixel_table, header, x, -1, z);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pGrid);

  pGrid = hdrl_resample_pixgrid_create(pixel_table, header, x, y, -1);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(pGrid);

  /* test valid input */
  pGrid = hdrl_resample_pixgrid_create(pixel_table, header, x, y, z);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(pGrid);
  hdrl_resample_pixgrid_delete(pGrid);

  cpl_table_unselect_row(pixel_table, 1);
  pGrid = hdrl_resample_pixgrid_create(pixel_table, header, x, y, z);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(pGrid);

  cpl_wcs_delete(wcs);
  cpl_table_delete(pixel_table);
  cpl_propertylist_delete(header);
  hdrl_resample_pixgrid_delete(pGrid);
  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test hdrl_resample_wcs_get_scales()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_wcs_get_scales(void) {

  double aXScale = 1;
  double aYScale = 1;
  hdrl_parameter *  outputgrid =
      hdrl_resample_parameter_create_outgrid2D(1e-5, 1e-5);

  hdrl_resample_outgrid_parameter *pogrid =
      (hdrl_resample_outgrid_parameter *) outputgrid;
  hdrl_resample_util_fill_outputgrid(pogrid);
  hdrl_parameter * method = hdrl_resample_parameter_create_nearest();

  hdrl_resample_method_parameter * p =
      (hdrl_resample_method_parameter *) method;
  cpl_test_nonnull(p);

  /* test invalid input */
  hdrl_resample_wcs_get_scales(NULL, &aXScale, &aYScale);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_resample_wcs_get_scales(pogrid, NULL, &aYScale);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_resample_wcs_get_scales(pogrid, &aXScale, NULL);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  /* test valid input */
  hdrl_resample_wcs_get_scales(pogrid, &aXScale, &aYScale);
  cpl_msg_warning(cpl_func,"x: %16.10g",aXScale);
  cpl_msg_warning(cpl_func,"y: %16.10g",aYScale);
  cpl_test_error(CPL_ERROR_NONE);
  /* clean memory */
  cpl_wcs_delete((cpl_wcs *)pogrid->wcs);
  hdrl_parameter_delete(outputgrid);
  hdrl_parameter_delete(method);
  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test hdrl_resample_create_table()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_create_table(void){

  const cpl_size size = 10;
  cpl_table* tab = NULL;

  /* test invalid input */
  hdrl_resample_create_table(NULL, size);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_resample_create_table(&tab, -1);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

  /* test valid input */
  hdrl_resample_create_table(&tab, size);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_table_delete(tab);
  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test hdrl_resample_pixgrid_get_rows()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_pixgrid_get_rows(void) {

  cpl_size aIndex = 1;
  hdrl_resample_pixgrid *aGrid = NULL;
  cpl_size x = 2;
  cpl_size y = 1;
  cpl_size z = 1;
  unsigned short aNMaps = 1;

  /* test invalid input */
  hdrl_resample_pixgrid_get_rows(NULL, aIndex);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  aGrid = hdrl_resample_pixgrid_new(x, y, z, aNMaps);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(aGrid);

  hdrl_resample_pixgrid_get_rows(aGrid, -1);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

  /* test valid input */
  hdrl_resample_pixgrid_get_rows(aGrid, aIndex);
  cpl_test_error(CPL_ERROR_NONE);
  hdrl_resample_pixgrid_delete(aGrid);

  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test hdrl_resample_cube_nearest()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_cube_nearest(void){

  cpl_table *ResTable = NULL;
  const cpl_size size = 10;
  cpl_size x = 2;
  cpl_size y = 2;
  cpl_size z = 2;
  unsigned short aNMaps = 1;

  /* prepare re-sample table */
  hdrl_resample_create_table(&ResTable, size);

  /* prepare a Grid */
  hdrl_resample_pixgrid *aGrid = hdrl_resample_pixgrid_new(x, y, z, aNMaps);

  /* prepare output-grid parameter */
  hdrl_parameter *  outputgrid =
      hdrl_resample_parameter_create_outgrid3D_userdef(HDRL_CDELT1,
						       HDRL_CDELT2, HDRL_CDELT3,
						       HDRL_RA_MIN, HDRL_RA_MAX, HDRL_DEC_MIN, HDRL_DEC_MAX,
						       HDRL_LAMBDA_MIN, HDRL_LAMBDA_MAX,5.);

  /* creates the cube */
  hdrl_resample_method aMethod = HDRL_RESAMPLE_METHOD_DRIZZLE;
  cpl_boolean use_errorweights = CPL_TRUE;
  hdrl_parameter * aParams_method =
      hdrl_resample_util_methodparam_create (
	  LOOP_DISTANCE,
	  RENKA_CRITICAL_RADIUS,
	  LANCZOS_KERNEL_SIZE,
	  DRIZZLE_DOWN_SCALING_FACTOR_X,
	  DRIZZLE_DOWN_SCALING_FACTOR_Y,
	  DRIZZLE_DOWN_SCALING_FACTOR_Z,
	  aMethod, use_errorweights);
  hdrl_resample_outgrid_parameter *aParams_outputgrid =
      (hdrl_resample_outgrid_parameter *) outputgrid;
  hdrl_resample_util_fill_outputgrid(aParams_outputgrid);
  cpl_wcs_delete((cpl_wcs *)aParams_outputgrid->wcs);
  aParams_outputgrid->wcs = hdrl_resample_util_crea_wcs_3d();

  /* generates a cube to be resampled */
  hdrl_resample_result * cube = hdrl_resample_compute(ResTable, aParams_method, outputgrid,
						      aParams_outputgrid->wcs);

  /* test invalid input */
  hdrl_resample_cube_nearest(NULL, ResTable, aGrid, aParams_outputgrid);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_resample_cube_nearest(cube, NULL, aGrid, aParams_outputgrid);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_resample_cube_nearest(cube, ResTable, NULL, aParams_outputgrid);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_resample_cube_nearest(cube, ResTable, aGrid, NULL);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  /* test invalid input */
  hdrl_resample_cube_nearest(cube, ResTable, aGrid,aParams_outputgrid);
  cpl_test_error(CPL_ERROR_NONE);

  /* clean memory */
  cpl_wcs_delete((cpl_wcs *)aParams_outputgrid->wcs);
  hdrl_resample_pixgrid_delete(aGrid);
  cpl_table_delete(ResTable);
  hdrl_parameter_delete(outputgrid);
  hdrl_parameter_delete(aParams_method);
  hdrl_resample_result_delete(cube);
  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test hdrl_resample_cube_weighted()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_cube_weighted(void) {

  cpl_table *ResTable = NULL;
  const cpl_size size = 10;
  cpl_size x = 2;
  cpl_size y = 2;
  cpl_size z = 2;
  unsigned short aNMaps = 1;

  /* prepare re-sample table */
  hdrl_resample_create_table(&ResTable, size);

  /* prepare a Grid */
  hdrl_resample_pixgrid *aGrid = hdrl_resample_pixgrid_new(x, y, z, aNMaps);

  /* prepare output-grid parameter */
  hdrl_parameter *  outputgrid =
      hdrl_resample_parameter_create_outgrid3D_userdef(HDRL_CDELT1,
						       HDRL_CDELT2, HDRL_CDELT3,
						       HDRL_RA_MIN, HDRL_RA_MAX, HDRL_DEC_MIN, HDRL_DEC_MAX,
						       HDRL_LAMBDA_MIN, HDRL_LAMBDA_MAX,5.);

  /* creates the cube */
  hdrl_resample_method aMethod = HDRL_RESAMPLE_METHOD_DRIZZLE;
  cpl_boolean use_errorweights = CPL_TRUE;
  hdrl_parameter * aParams_method =
      hdrl_resample_util_methodparam_create (
	  LOOP_DISTANCE,
	  RENKA_CRITICAL_RADIUS,
	  LANCZOS_KERNEL_SIZE,
	  DRIZZLE_DOWN_SCALING_FACTOR_X,
	  DRIZZLE_DOWN_SCALING_FACTOR_Y,
	  DRIZZLE_DOWN_SCALING_FACTOR_Z,
	  aMethod, use_errorweights);
  hdrl_resample_outgrid_parameter *aParams_outputgrid =
      (hdrl_resample_outgrid_parameter *) outputgrid;
  hdrl_resample_util_fill_outputgrid(aParams_outputgrid);
  cpl_wcs_delete((cpl_wcs *)aParams_outputgrid->wcs);
  aParams_outputgrid->wcs = hdrl_resample_util_crea_wcs_3d();

  hdrl_resample_method_parameter * mp =
      (hdrl_resample_method_parameter *) aParams_method;
  cpl_test_nonnull(mp);
  /* generates a cube to be resampled */
  hdrl_resample_result * cube = hdrl_resample_compute(ResTable, aParams_method, outputgrid,
						      aParams_outputgrid->wcs);

  /* test invalid input */
  hdrl_resample_cube_weighted(NULL, ResTable, aGrid, mp,
			      aParams_outputgrid);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_resample_cube_weighted(cube, NULL, aGrid, mp,
			      aParams_outputgrid);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_resample_cube_weighted(cube, ResTable, NULL, mp,
			      aParams_outputgrid);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_resample_cube_weighted(cube, ResTable, aGrid, NULL,
			      aParams_outputgrid);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_resample_cube_weighted(cube, ResTable, aGrid, mp,
			      NULL);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  /* test valid input */
  hdrl_resample_cube_weighted(cube, ResTable, aGrid, mp,
			      aParams_outputgrid);
  cpl_test_error(CPL_ERROR_NONE);

  mp->loop_distance = -1;
  mp->lanczos_kernel_size = 0;
  hdrl_resample_cube_weighted(cube, ResTable, aGrid, mp,
			      aParams_outputgrid);
  cpl_test_error(CPL_ERROR_NONE);

  /* clean memory */
  cpl_wcs_delete((cpl_wcs *)aParams_outputgrid->wcs);
  hdrl_resample_pixgrid_delete(aGrid);
  cpl_table_delete(ResTable);
  hdrl_parameter_delete(outputgrid);
  hdrl_parameter_delete(aParams_method);
  hdrl_resample_result_delete(cube);

  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test hdrl_resample_cube()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resample_cube(void) {
  cpl_table *ResTable = NULL;
  const cpl_size size = 10;

  /* prepare re-sample table */
  hdrl_resample_create_table(&ResTable, size);

  /* prepare a Grid */
  hdrl_resample_pixgrid *aGrid = NULL;
  /* prepare output-grid parameter */
  hdrl_parameter *  outputgrid =
      hdrl_resample_parameter_create_outgrid3D_userdef(HDRL_CDELT1,
						       HDRL_CDELT2, HDRL_CDELT3,
						       HDRL_RA_MIN, HDRL_RA_MAX, HDRL_DEC_MIN, HDRL_DEC_MAX,
						       HDRL_LAMBDA_MIN, HDRL_LAMBDA_MAX,5.);

  /* creates the cube */
  hdrl_resample_method aMethod = HDRL_RESAMPLE_METHOD_DRIZZLE;
  cpl_boolean use_errorweights = CPL_TRUE;
  hdrl_parameter * aParams_method =
      hdrl_resample_util_methodparam_create (
	  LOOP_DISTANCE,
	  RENKA_CRITICAL_RADIUS,
	  LANCZOS_KERNEL_SIZE,
	  DRIZZLE_DOWN_SCALING_FACTOR_X,
	  DRIZZLE_DOWN_SCALING_FACTOR_Y,
	  DRIZZLE_DOWN_SCALING_FACTOR_Z,
	  aMethod, use_errorweights);
  hdrl_resample_outgrid_parameter *aParams_outputgrid =
      (hdrl_resample_outgrid_parameter *) outputgrid;
  hdrl_resample_util_fill_outputgrid(aParams_outputgrid);
  cpl_wcs_delete((cpl_wcs *)aParams_outputgrid->wcs);
  aParams_outputgrid->wcs = hdrl_resample_util_crea_wcs_3d();

  hdrl_resample_method_parameter * mp =
      (hdrl_resample_method_parameter *) aParams_method;
  cpl_test_nonnull(mp);
  /* test invalid input */
  hdrl_resample_cube(NULL, mp, aParams_outputgrid, &aGrid);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_resample_cube(ResTable, NULL, aParams_outputgrid, &aGrid);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_resample_cube(ResTable, mp, NULL, &aGrid);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_resample_cube(ResTable, mp, aParams_outputgrid, NULL);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  /* test valid input */
  hdrl_resample_result * cube = hdrl_resample_cube(ResTable, mp, aParams_outputgrid, &aGrid);
  cpl_test_error(CPL_ERROR_NONE);
  hdrl_resample_result_delete(cube);
  hdrl_resample_pixgrid_delete(aGrid);

  /* test wrong method input */
  mp->method = 10;
  cube = hdrl_resample_cube(ResTable, mp, aParams_outputgrid, &aGrid);
  cpl_test_error(CPL_ERROR_UNSUPPORTED_MODE);
  hdrl_resample_result_delete(cube);
  cpl_table_delete(ResTable);
  hdrl_resample_pixgrid_delete(aGrid);

  /* test invalid grid creation input */
  ResTable = cpl_table_new(0);
  cube = hdrl_resample_cube(ResTable, mp, aParams_outputgrid, &aGrid);
  cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);

  /* clean memory */
  cpl_wcs_delete((cpl_wcs *)aParams_outputgrid->wcs);
  hdrl_resample_pixgrid_delete(aGrid);
  cpl_table_delete(ResTable);
  hdrl_parameter_delete(outputgrid);
  hdrl_parameter_delete(aParams_method);
  hdrl_resample_result_delete(cube);

  cpl_test_error(CPL_ERROR_NONE);
  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @brief unit test hdrl_resampling_set_outputgrid()
  @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
test_hdrl_resampling_set_outputgrid(void)
{
  int xsize = HDRL_SIZE_X;
  int ysize = HDRL_SIZE_Y;
  int zsize = HDRL_SIZE_Z;

  cpl_table *ResTable = NULL;
  const cpl_size size = 10;

  /* prepare re-sample table */
  hdrl_resample_create_table(&ResTable, size);

  /* prepare a Grid */
  hdrl_resample_pixgrid *aGrid = NULL;

  /* prepare output-grid parameter */
  hdrl_parameter *  outputgrid =
      hdrl_resample_parameter_create_outgrid3D_userdef(HDRL_CDELT1,
						       HDRL_CDELT2, HDRL_CDELT3,
						       HDRL_RA_MIN, HDRL_RA_MAX, HDRL_DEC_MIN, HDRL_DEC_MAX,
						       HDRL_LAMBDA_MIN, HDRL_LAMBDA_MAX,5.);

  /* creates the cube */
  hdrl_resample_method aMethod = HDRL_RESAMPLE_METHOD_DRIZZLE;
  cpl_boolean use_errorweights = CPL_TRUE;
  hdrl_parameter * aParams_method =
      hdrl_resample_util_methodparam_create (
	  LOOP_DISTANCE,
	  RENKA_CRITICAL_RADIUS,
	  LANCZOS_KERNEL_SIZE,
	  DRIZZLE_DOWN_SCALING_FACTOR_X,
	  DRIZZLE_DOWN_SCALING_FACTOR_Y,
	  DRIZZLE_DOWN_SCALING_FACTOR_Z,
	  aMethod, use_errorweights);
  hdrl_resample_outgrid_parameter *aParams_outputgrid =
      (hdrl_resample_outgrid_parameter *) outputgrid;
  hdrl_resample_util_fill_outputgrid(aParams_outputgrid);
  cpl_wcs_delete((cpl_wcs *)aParams_outputgrid->wcs);
  aParams_outputgrid->wcs = hdrl_resample_util_crea_wcs_3d();

  hdrl_resample_method_parameter * mp =
      (hdrl_resample_method_parameter *) aParams_method;
  cpl_test_nonnull(mp);

  hdrl_resample_result * cube = hdrl_resample_cube(ResTable, mp,
						   aParams_outputgrid, &aGrid);

  /* test invalid input */
  hdrl_resampling_set_outputgrid (-1, ysize, zsize, cube, aParams_outputgrid);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

  hdrl_resampling_set_outputgrid (xsize, -1, zsize, cube, aParams_outputgrid);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

  hdrl_resampling_set_outputgrid (xsize, ysize, -1, cube, aParams_outputgrid);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

  hdrl_resampling_set_outputgrid (xsize, ysize, zsize, NULL, aParams_outputgrid);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_resampling_set_outputgrid (xsize, ysize, zsize, cube, NULL);
  cpl_test_error(CPL_ERROR_NULL_INPUT);
  /* test valid input */
  /* We need to do delete the header cube->header, as
   * hdrl_resampling_set_outputgrid allocates mem for cube->header
   * even if cube->header might already exists
   */
  cpl_propertylist_delete(cube->header);

  hdrl_resampling_set_outputgrid (xsize, ysize, zsize, cube, aParams_outputgrid);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_wcs_delete((cpl_wcs *)aParams_outputgrid->wcs);
  hdrl_resample_pixgrid_delete(aGrid);
  cpl_table_delete(ResTable);
  hdrl_parameter_delete(outputgrid);
  hdrl_parameter_delete(aParams_method);
  hdrl_resample_result_delete(cube);
  cpl_test_error(CPL_ERROR_NONE);
  return cpl_error_get_code();
}
static cpl_error_code
test_hdrl_resample_inputtable_verify(void)
{
  cpl_table* tab = NULL;
  hdrl_resample_inputtable_verify(NULL);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  /* verify column existence */
  tab= cpl_table_new(1);
  cpl_table_new_column(tab, HDRL_RESAMPLE_TABLE_RA, CPL_TYPE_DOUBLE);
  hdrl_resample_inputtable_verify(tab);
  cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

  cpl_table_new_column(tab, HDRL_RESAMPLE_TABLE_DEC, CPL_TYPE_DOUBLE);
  hdrl_resample_inputtable_verify(tab);
  cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

  cpl_table_new_column(tab, HDRL_RESAMPLE_TABLE_LAMBDA, CPL_TYPE_DOUBLE);
  hdrl_resample_inputtable_verify(tab);
  cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

  cpl_table_new_column(tab, HDRL_RESAMPLE_TABLE_BPM, CPL_TYPE_INT);
  hdrl_resample_inputtable_verify(tab);
  cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

  cpl_table_new_column(tab, HDRL_RESAMPLE_TABLE_ERRORS, CPL_TYPE_DOUBLE);
  hdrl_resample_inputtable_verify(tab);
  cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

  cpl_table_new_column(tab, HDRL_RESAMPLE_TABLE_DATA, CPL_TYPE_DOUBLE);
  hdrl_resample_inputtable_verify(tab);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_table_delete(tab);

  /* verify column type */
  tab= cpl_table_new(1);
  cpl_table_new_column(tab, HDRL_RESAMPLE_TABLE_RA, CPL_TYPE_INT);
  hdrl_resample_inputtable_verify(tab);
  cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);
  cpl_table_erase_column(tab, HDRL_RESAMPLE_TABLE_RA);
  cpl_table_new_column(tab, HDRL_RESAMPLE_TABLE_RA, CPL_TYPE_DOUBLE);

  cpl_table_new_column(tab, HDRL_RESAMPLE_TABLE_DEC, CPL_TYPE_INT);
  hdrl_resample_inputtable_verify(tab);
  cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);
  cpl_table_erase_column(tab, HDRL_RESAMPLE_TABLE_DEC);
  cpl_table_new_column(tab, HDRL_RESAMPLE_TABLE_DEC, CPL_TYPE_DOUBLE);

  cpl_table_new_column(tab, HDRL_RESAMPLE_TABLE_LAMBDA, CPL_TYPE_INT);
  hdrl_resample_inputtable_verify(tab);
  cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);
  cpl_table_erase_column(tab, HDRL_RESAMPLE_TABLE_LAMBDA);
  cpl_table_new_column(tab, HDRL_RESAMPLE_TABLE_LAMBDA, CPL_TYPE_DOUBLE);

  cpl_table_new_column(tab, HDRL_RESAMPLE_TABLE_BPM, CPL_TYPE_DOUBLE);
  hdrl_resample_inputtable_verify(tab);
  cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);
  cpl_table_erase_column(tab, HDRL_RESAMPLE_TABLE_BPM);
  cpl_table_new_column(tab, HDRL_RESAMPLE_TABLE_BPM, CPL_TYPE_INT);

  cpl_table_new_column(tab, HDRL_RESAMPLE_TABLE_ERRORS, CPL_TYPE_INT);
  hdrl_resample_inputtable_verify(tab);
  cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);
  cpl_table_erase_column(tab, HDRL_RESAMPLE_TABLE_ERRORS);
  cpl_table_new_column(tab, HDRL_RESAMPLE_TABLE_ERRORS, CPL_TYPE_DOUBLE);

  cpl_table_new_column(tab, HDRL_RESAMPLE_TABLE_DATA, CPL_TYPE_INT);
  hdrl_resample_inputtable_verify(tab);
  cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);
  cpl_table_erase_column(tab, HDRL_RESAMPLE_TABLE_DATA);
  cpl_table_new_column(tab, HDRL_RESAMPLE_TABLE_DATA, CPL_TYPE_DOUBLE);
  hdrl_resample_inputtable_verify(tab);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_table_delete(tab);
  return cpl_error_get_code();
}

/**
 * Utility to create a FITS header of an image
 */
static cpl_propertylist*
hdrl_resample_util_header_create(const int sx, const int sy,
				 const double dx, const double dy)
{

  cpl_propertylist* plist = cpl_propertylist_new();
  cpl_propertylist_append_bool(plist,"SIMPLE", CPL_TRUE);
  cpl_propertylist_append_int(plist,"NAXIS", 2);
  cpl_propertylist_append_int(plist,"NAXIS1", sx);
  cpl_propertylist_append_int(plist,"NAXIS2", sy);
  cpl_propertylist_append_bool(plist,"EXTEND", CPL_TRUE);
  cpl_propertylist_append_double(plist,"CRPIX1", 0.5 * sx);
  cpl_propertylist_append_double(plist,"CRPIX2", 0.5 * sy);
  cpl_propertylist_append_double(plist,"CRVAL1", 0.5 * sx);
  cpl_propertylist_append_double(plist,"CRVAL2", 0.5 * sy);
  cpl_propertylist_append_double(plist,"CDELT1", dx );
  cpl_propertylist_append_double(plist,"CDELT2", dy );
  cpl_propertylist_append_double(plist,"CD1_1", -dx );
  cpl_propertylist_append_double(plist,"CD1_2", 0. );
  cpl_propertylist_append_double(plist,"CD2_1", 0. );
  cpl_propertylist_append_double(plist,"CD2_2", dy );
  cpl_propertylist_append_string(plist,"CTYPE1","PIXEL");
  cpl_propertylist_append_string(plist,"CTYPE2","PIXEL");
  cpl_propertylist_append_string(plist,"CUNIT1","PIXEL");
  cpl_propertylist_append_string(plist,"CUNIT2","PIXEL");

  return plist;
}

/**
 * Utility to create a uniform image
 */
static cpl_image*
hdrl_resample_util_crea_image_uniform(const cpl_size sx, const cpl_size sy,
				      const double value)
{
  assert(value > 0);
  cpl_image * image = cpl_image_new(sx, sy, CPL_TYPE_DOUBLE);
  cpl_image_add_scalar(image,value);
  return image;
}

static double
hdrldemo_get_resampled_pix_value(const char* type, const double pix_value,
				 const double outlier, cpl_boolean is_bad,
				 hdrl_resample_method aMethod)
{
  cpl_size sx = HDRL_SIZE_X;
  cpl_size sy = HDRL_SIZE_X;
  cpl_size xc = 0.5 * sx;
  cpl_size yc = 0.5 * sy;

  const double value = pix_value;
  const double dx = 0.01;
  const double dy = 0.01;
  cpl_image* simul = NULL;
  cpl_image* errs = NULL;

  cpl_test_error(CPL_ERROR_NONE);
  if(strcmp(type,"uniform") == 0) {
      simul = hdrl_resample_util_crea_image_uniform(sx, sy, value);
  } else {
      simul = hdrl_resample_util_crea_image_dice_5(sx, sy, value);
  }
  cpl_test_error(CPL_ERROR_NONE);

  char* fname;
  if( is_bad ) {
      cpl_image_set(simul, xc, yc, outlier);
      fname = cpl_sprintf("cube_and_bp.fits");
  } else {
      fname = cpl_sprintf("cube_not_bp.fits");
  }
  cpl_free(fname);
  cpl_test_error(CPL_ERROR_NONE);
  errs = cpl_image_duplicate(simul);
  cpl_image_power(errs, 0.5);

  /* We need to add wcs */
  cpl_propertylist* plist = NULL;
  plist = hdrl_resample_util_header_create(sx, sy, dx, dy);
  cpl_image_save(simul, "image.fits", CPL_TYPE_DOUBLE,plist,CPL_IO_DEFAULT);
  cpl_propertylist_delete(plist);
  plist = cpl_propertylist_load("image.fits",0);
  cpl_wcs *wcs = cpl_wcs_new_from_propertylist(plist);
  cpl_test_nonnull(wcs);
  cpl_test_error(CPL_ERROR_NONE);

  /* we now create the MUSE table */
  cpl_imagelist* ilist = NULL;
  cpl_imagelist* elist = NULL;
  ilist = cpl_imagelist_new();
  elist = cpl_imagelist_new();

  cpl_imagelist_set(ilist, simul, 0);
  cpl_imagelist_set(elist, errs, 0);
  cpl_table* pixel_table = NULL;
  hdrl_imagelist* hlist = NULL;
  if( is_bad == CPL_TRUE) {
      cpl_image* qual = cpl_image_new(sx, sy, CPL_TYPE_INT);
      cpl_image_set(qual, xc, yc, 1);
      cpl_imagelist* qlist = cpl_imagelist_new();
      cpl_imagelist_set(qlist, qual, 0);
      hlist = hdrl_imagelist_create(ilist, elist);
      pixel_table = hdrl_resample_imagelist_to_table(hlist, wcs);
      cpl_imagelist_delete(qlist);
  } else {
      hlist = hdrl_imagelist_create(ilist, elist);
      pixel_table = hdrl_resample_imagelist_to_table(hlist, wcs);
  }
  cpl_imagelist_delete(ilist);
  cpl_imagelist_delete(elist);
  hdrl_imagelist_delete(hlist);

  cpl_frameset* frameset = cpl_frameset_new();

  /* now we resample */
  cpl_msg_info(cpl_func,"start resample");

  cpl_test_error(CPL_ERROR_NONE);
  cpl_wcs_delete(wcs);
  cpl_propertylist* p = cpl_propertylist_new();
  cpl_propertylist_append_string(p,"CTYPE1", "RA---TAN");
  cpl_propertylist_append_string(p,"CTYPE2", "DEC--TAN");
  cpl_propertylist_append_double(p,"CRVAL1", 0.);
  cpl_propertylist_append_double(p,"CRVAL2", 0.);
  cpl_propertylist_append_double(p,"CRPIX1", xc);
  cpl_propertylist_append_double(p,"CRPIX2", yc);
  cpl_propertylist_append_double(p,"CD1_1", -dx);
  cpl_propertylist_append_double(p,"CD1_2", 0.1); /* add rotation if not 0 */
  cpl_propertylist_append_double(p,"CD2_1", 0.1); /* add rotation if not 0 */
  cpl_propertylist_append_double(p,"CD2_2", dy);
  wcs = cpl_wcs_new_from_propertylist(p);
  cpl_test_nonnull(wcs);
  cpl_propertylist_delete(p);

  const double ramin = xc - 0.5*sx*dx;
  const double ramax = xc + 0.5*sx*dx;
  const double decmin = yc - 0.5*sy*dy;
  const double decmax = yc + 0.5*sy*dy;
  const double lambmin = 550;
  const double lambmax = 551;
  const double dlambda = 1;

  hdrl_parameter* aParams_outputgrid =
      hdrl_resample_parameter_create_outgrid3D_userdef(dx, dy, dlambda,
						       ramin, ramax, decmin, decmax, lambmin, lambmax, 5.);

  cpl_boolean use_errorweights = CPL_FALSE;
  hdrl_parameter * aParams_method =
      hdrl_resample_util_methodparam_create (
	  LOOP_DISTANCE,
	  RENKA_CRITICAL_RADIUS,
	  LANCZOS_KERNEL_SIZE,
	  DRIZZLE_DOWN_SCALING_FACTOR_X,
	  DRIZZLE_DOWN_SCALING_FACTOR_Y,
	  DRIZZLE_DOWN_SCALING_FACTOR_Z,
	  aMethod, use_errorweights);

  hdrl_resample_result *cube = NULL;
  cube = hdrl_resample_compute(pixel_table, aParams_method, aParams_outputgrid, wcs);
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_imagelist* himlist = cube->himlist;
  hdrl_image* hima = hdrl_imagelist_get(himlist, 0);
  double*  data = cpl_image_get_data(hdrl_image_get_image(hima));
  cpl_size rsx = hdrl_imagelist_get_size_x(himlist);
  cpl_size rsy = hdrl_imagelist_get_size_y(himlist);
  cpl_size rpix = 0.5 * rsy * rsx + rsx;

  double pix_ref = data[rpix];
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);

  /* clean memory */
  cpl_free(aParams_method);
  cpl_wcs_delete(wcs);
  hdrl_parameter_destroy(aParams_outputgrid);
  cpl_table_delete(pixel_table);
  hdrl_resample_result_delete(cube);
  cpl_frameset_delete(frameset);
  cpl_propertylist_delete(plist);
  return pix_ref;
}

static cpl_error_code
test_resample_image_dice(hdrl_resample_method aMethod)
{

  /* Create an image with the number 5 of a dice.
	   Central point is round
	   Other points are elliptical with major axis oriented along different
	   directions
   */
  cpl_test_error(CPL_ERROR_NONE);
  double ref_value =
      hdrldemo_get_resampled_pix_value("uniform", HDRL_FLUX_ADU, 1000., CPL_FALSE,
				       aMethod );

  double check_value =
      hdrldemo_get_resampled_pix_value("uniform", HDRL_FLUX_ADU, 100000., CPL_TRUE,
				       aMethod);

  cpl_test_abs(check_value, ref_value, HDRL_EPS_TEST);
  /* possible further tests:
	   move the image horizontally
	   move the image vertically
	   move the image diagonally
	   rotate the image around a point: around the center, at the edge
	   over-sample the image
	   sub-sample the image
   */
  return cpl_error_get_code();
}

static hdrl_resample_result *
hdrl_resample_util_get_resampled_pix_cube(const char* type, const double pix_value,
					  const double outlier, cpl_boolean is_bad, const char* suffix)
{
  cpl_size sx = HDRL_SIZE_X;
  cpl_size sy = HDRL_SIZE_Y;
  cpl_size xc = 0.5 * sx;
  cpl_size yc = 0.5 * sy;
  const double value = pix_value;
  const double dx = 0.01;
  const double dy = 0.01;
  cpl_image* simul = NULL;
  cpl_image* errs = NULL;

  cpl_test_error(CPL_ERROR_NONE);
  if(strcmp(type,"uniform") == 0) {
      simul = hdrl_resample_util_crea_image_uniform(sx, sy, value);
  } else {
      simul = hdrl_resample_util_crea_image_dice_5(sx, sy, value);
  }
  cpl_test_error(CPL_ERROR_NONE);

  char* bname;
  if( is_bad ) {
      cpl_msg_warning(cpl_func,"is bad");
      cpl_image_set(simul, xc, yc, outlier);
      bname = cpl_sprintf("%s_%s.fits","cube_and_bp",suffix);
  } else {
      cpl_msg_warning(cpl_func,"is not bad");
      bname = cpl_sprintf("%s_%s.fits","cube_not_bp",suffix);
  }
  cpl_test_error(CPL_ERROR_NONE);

  errs = cpl_image_duplicate(simul);
  cpl_image_power(errs, 0.5);

  /* Add wcs */
  cpl_propertylist* plist = NULL;
  plist = hdrl_resample_util_header_create(sx, sy, dx, dy);
  cpl_image_save(simul, "image.fits", CPL_TYPE_DOUBLE,plist,CPL_IO_DEFAULT);
  cpl_propertylist_delete(plist);
  plist = cpl_propertylist_load("image.fits",0);
  cpl_wcs *wcs = cpl_wcs_new_from_propertylist(plist);
  cpl_test_nonnull(wcs);

  /* we now create the pixel table */
  cpl_imagelist* ilist = NULL;
  cpl_imagelist* elist = NULL;
  cpl_table* pixel_table = NULL;
  ilist = cpl_imagelist_new();
  elist = cpl_imagelist_new();
  cpl_imagelist_set(ilist, simul, 0);
  cpl_imagelist_set(elist, errs, 0);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_imagelist_save(ilist,bname, CPL_TYPE_DOUBLE, NULL, CPL_IO_DEFAULT);
  cpl_imagelist_save(elist,bname, CPL_TYPE_DOUBLE, NULL, CPL_IO_EXTEND);
  cpl_test_error(CPL_ERROR_NONE);
  hdrl_imagelist* hlist;
  if( is_bad == CPL_TRUE) {
      cpl_image* qual = cpl_image_new(sx, sy, CPL_TYPE_INT);
      cpl_image_set(qual, xc, yc, 1);
      cpl_imagelist* qlist = cpl_imagelist_new();
      cpl_imagelist_set(qlist, qual, 0);
      cpl_imagelist_save(qlist,bname, CPL_TYPE_INT, NULL, CPL_IO_EXTEND);
      hlist = hdrl_imagelist_create(ilist, elist);
      pixel_table = hdrl_resample_imagelist_to_table(hlist, wcs);
      cpl_imagelist_delete(qlist);
  } else {
      hlist = hdrl_imagelist_create(ilist, elist);
      pixel_table = hdrl_resample_imagelist_to_table(hlist, wcs);
  }
  cpl_test_error(CPL_ERROR_NONE);

  cpl_imagelist_delete(ilist);
  cpl_imagelist_delete(elist);
  hdrl_imagelist_delete(hlist);
  cpl_free(bname);
  cpl_frameset* frameset = cpl_frameset_new();

  /* now we resample */
  cpl_msg_info(cpl_func,"start resample");
  const double ramin = xc - 0.5*sx*dx;
  const double ramax = xc + 0.5*sx*dx;
  const double decmin = yc - 0.5*sy*dy;
  const double decmax = yc + 0.5*sy*dy;
  const double lambmin = 550;
  const double lambmax = 551;
  const double dlambda = 1;
  cpl_wcs_delete(wcs);
  cpl_propertylist* p = cpl_propertylist_new();
  cpl_propertylist_append_string(p,"CTYPE1", "RA---TAN");
  cpl_propertylist_append_string(p,"CTYPE2", "DEC--TAN");
  cpl_propertylist_append_double(p,"CRVAL1", 0.);
  cpl_propertylist_append_double(p,"CRVAL2", 0.);
  cpl_propertylist_append_double(p,"CRPIX1", xc);
  cpl_propertylist_append_double(p,"CRPIX2", yc);
  cpl_propertylist_append_double(p,"CD1_1", -dx);
  cpl_propertylist_append_double(p,"CD1_2", 0.);
  cpl_propertylist_append_double(p,"CD2_1", 0.);
  cpl_propertylist_append_double(p,"CD2_2", dy);
  wcs = cpl_wcs_new_from_propertylist(p);
  cpl_propertylist_delete(p);

  hdrl_parameter* aParams_outputgrid =
      hdrl_resample_parameter_create_outgrid3D_userdef(dx, dy, dlambda,
						       ramin, ramax, decmin, decmax, lambmin, lambmax, 5.);

  cpl_boolean use_errorweights = CPL_FALSE;
  /* RENKA, LANCZOS, DRIZZLE generate a cube with all flagged pixels */
  hdrl_resample_method aMethod = HDRL_RESAMPLE_METHOD_QUADRATIC;
  hdrl_parameter * aParams_method =
      hdrl_resample_util_methodparam_create (
	  LOOP_DISTANCE,
	  RENKA_CRITICAL_RADIUS,
	  LANCZOS_KERNEL_SIZE,
	  DRIZZLE_DOWN_SCALING_FACTOR_X,
	  DRIZZLE_DOWN_SCALING_FACTOR_Y,
	  DRIZZLE_DOWN_SCALING_FACTOR_Z,
	  aMethod, use_errorweights);

  hdrl_resample_result *cube = NULL;
  cube = hdrl_resample_compute(pixel_table, aParams_method,
			       aParams_outputgrid, wcs);

  cpl_msg_info(cpl_func,"end resample");
  /* clean memory */
  cpl_table_delete(pixel_table);
  cpl_wcs_delete(wcs);
  hdrl_parameter_destroy(aParams_outputgrid);
  hdrl_parameter_destroy(aParams_method);
  cpl_frameset_delete(frameset);
  cpl_propertylist_delete(plist);

  return cube;
}

static cpl_error_code
test_resample_image_with_outlier(void)
{
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_resample_result *cube1 =
      hdrl_resample_util_get_resampled_pix_cube("uniform", HDRL_FLUX_ADU, 1000., CPL_FALSE,"one");
  hdrl_image* hima1 = hdrl_imagelist_get(cube1->himlist,0);

  hdrl_resample_result *cube2 =
      hdrl_resample_util_get_resampled_pix_cube("uniform", HDRL_FLUX_ADU, 100000., CPL_FALSE,"two");

  hdrl_image* hima2 = hdrl_imagelist_get(cube2->himlist,0);
  hdrl_image* hdiff = hdrl_image_sub_image_create(hima1, hima2);
  cpl_image* diff = hdrl_image_get_image(hdiff);

  cpl_image_save(diff,"diff.fits",CPL_TYPE_DOUBLE,NULL,CPL_IO_DEFAULT);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_abs(0,cpl_image_get_mean(diff), HDRL_EPS_TEST);
  cpl_test_abs(0,cpl_image_get_stdev(diff), HDRL_EPS_TEST);
  cpl_test_abs(0,cpl_image_get_min(diff), HDRL_EPS_TEST);
  cpl_test_abs(0,cpl_image_get_max(diff), HDRL_EPS_TEST);

  hdrl_image_delete(hdiff);
  hdrl_resample_result_delete(cube1);
  hdrl_resample_result_delete(cube2);
  cpl_test_error(CPL_ERROR_NONE);
  return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of hdrl_image
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
  cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

  test_resample_wcs_as_muse();
  test_hdrl_resample_wcs_print();
  test_hdrl_resample_outgrid_parameter_print();
  test_hdrl_resample_method_parameter_print();
  test_hdrl_wcs_xy_to_radec();
  test_hdrl_resample_pfits_get();
  test_hdrl_resample_smallwcs_new();
  test_hdrl_resample_pixgrid_delete();
  test_hdrl_resample_compute();

  test_resample_parameters();
  test_hdrl_wcs_to_propertylist();
  test_hdrl_resample_image_to_table();
  test_hdrl_resample_imagelist_to_table();
  test_hdrl_resample_parameter_create_outgrid2D();
  test_hdrl_resample_parameter_create_outgrid3D();
  test_hdrl_resample_parameter_create_outgrid2D_userdef();
  test_hdrl_resample_parameter_create_outgrid3D_userdef();

  test_hdrl_resample_weight_function_renka();
  test_hdrl_resample_weight_function_linear();
  test_hdrl_resample_weight_function_quadratic();
  test_hdrl_resample_weight_function_sinc();
  test_hdrl_resample_weight_function_lanczos();
  test_hdrl_resample_weight_function_drizzle();

  test_hdrl_resample_wcs_projplane_from_celestial();
  test_hdrl_resample_wcs_pixel_from_celestial_fast();
  test_hdrl_resample_compute_size();
  test_hdrl_resample_pixgrid_add();
  test_hdrl_resample_pixgrid_get_count();
  test_hdrl_resample_pixgrid_get_index();
  test_hdrl_resample_pixgrid_new();

  test_hdrl_resample_wcs_get_scales();
  test_hdrl_resample_create_table();
  test_hdrl_resample_pixgrid_get_rows();
  test_hdrl_resample_pixgrid_create();
  test_hdrl_resample_cube_nearest();
  test_hdrl_resample_cube_weighted();
  test_hdrl_resample_cube();

  test_hdrl_resampling_set_outputgrid();
  test_hdrl_resample_compute2D_multiple();
  test_hdrl_resample_compute3D_multiple();
  test_hdrl_resample_inputtable_verify();

  test_resample_image_dice(HDRL_RESAMPLE_METHOD_LINEAR);
  test_resample_image_dice(HDRL_RESAMPLE_METHOD_QUADRATIC);
  test_resample_image_dice(HDRL_RESAMPLE_METHOD_NEAREST);
  test_resample_image_dice(HDRL_RESAMPLE_METHOD_RENKA);
  test_resample_image_dice(HDRL_RESAMPLE_METHOD_DRIZZLE);
  test_resample_image_dice(HDRL_RESAMPLE_METHOD_LANCZOS);

  test_resample_image_with_outlier();
  return cpl_test_end(0);
}

