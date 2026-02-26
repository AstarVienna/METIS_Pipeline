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
#include "hdrl_random.h"

#include "hdrl_mode.c"
#include "hdrl_utils.h"
#include <cpl.h>

#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <gsl/gsl_histogram.h>

#define HDRL_EPS_TEST   1.e-5

/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_mode_test   Testing of the HDRL mode module
 */
/*----------------------------------------------------------------------------*/

static cpl_error_code test_hdrl_mode(hdrl_random_state * state, int savetodisk)
{

  double expected = 10000.;
  cpl_size sx =  1000;
  cpl_size sy =  1000;
  cpl_image* ima = cpl_image_new(sx, sy, CPL_TYPE_INT);
  int* pdata = cpl_image_get_data(ima);
  cpl_size size = sx * sy;

  for(cpl_size i = 0; i < size; i++) {

      pdata[i] = (int)hdrl_random_poisson(state, expected);
  }
  if (savetodisk) {
      cpl_propertylist* plist = cpl_propertylist_new();
      cpl_propertylist_save(plist,"ima.fits",CPL_IO_DEFAULT);
      cpl_image_save(ima, "ima.fits", CPL_TYPE_INT, NULL, CPL_IO_EXTEND);
      cpl_propertylist_delete(plist);
  }

  hdrl_image* hima = hdrl_image_create(ima, NULL);
  cpl_vector* vec = NULL;
  vec = hdrl_image_to_vector(hdrl_image_get_image(hima), hdrl_image_get_mask(hima));
  double min = cpl_vector_get_min(vec);
  double max = cpl_vector_get_max(vec);
  cpl_msg_debug(cpl_func,"min=%g max=%g", min, max);

  double mode = 0;
  double mode_err = 0;
  cpl_size naccepted = 0;
  /* test invalid input */

  hdrl_mode_clip(NULL, 0, -1, 0, HDRL_MODE_MEDIAN, 0, &mode, &mode_err,
		 &naccepted);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_mode_clip_image(NULL, 0, -1, 0, HDRL_MODE_MEDIAN, 0, &mode,
		       &mode_err, &naccepted);
  cpl_test_error(CPL_ERROR_NULL_INPUT);



  cpl_msg_debug(cpl_func,"===============================================");
  cpl_msg_debug(cpl_func,"MODE METHOD MEDIAN                             ");
  cpl_msg_debug(cpl_func,"-----------------------------------------------");
  hdrl_mode_clip(vec, 0, -1, 0, HDRL_MODE_MEDIAN, 1, &mode, &mode_err, &naccepted);
  cpl_test_abs(mode, expected, 1.);
  cpl_test_abs(mode_err, 0., HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_mode_clip(vec, 0, -1, 0, HDRL_MODE_MEDIAN, 0, &mode, &mode_err, &naccepted);
  cpl_test_abs(mode, expected, 1.);
  cpl_test_abs(mode_err, 2.00072, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);


  hdrl_mode_clip_image(ima, 0, -1, 0, HDRL_MODE_MEDIAN, 0, &mode, &mode_err,
		       &naccepted);
  cpl_test_abs(mode, expected, 1.);
  cpl_test_abs(mode_err, 2.00072, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);

  /* case error_niter < 1 */

  hdrl_mode_clip_image(ima, 0, -1, 0, HDRL_MODE_MEDIAN, -1, &mode, &mode_err,
		       &naccepted);
  cpl_test_abs(mode, expected, 1.);
  cpl_test_zero(mode_err);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_msg_debug(cpl_func,"===============================================");
  cpl_msg_debug(cpl_func,"MODE METHOD WEIGHT                             ");
  cpl_msg_debug(cpl_func,"-----------------------------------------------");
  hdrl_mode_clip(vec, 0, -1, 0, HDRL_MODE_WEIGHTED, 0, &mode, &mode_err, &naccepted);
  cpl_test_abs(mode, expected, 4);
  cpl_test_abs(mode_err, 6.355987781, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_abs(mode_err, 6.355987781, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_mode_clip_image(ima, 0, -1, 0, HDRL_MODE_WEIGHTED, 0, &mode, &mode_err,
		       &naccepted);
  cpl_test_abs(mode, expected, 4);
  cpl_test_abs(mode_err, 6.355987781, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);

  /* case error_niter < 1 */

  hdrl_mode_clip_image(ima, 0, -1, 0, HDRL_MODE_WEIGHTED, -1, &mode, &mode_err,
		       &naccepted);
  cpl_test_abs(mode, expected, 4);
  cpl_test_zero(mode_err);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_msg_debug(cpl_func,"===============================================");
  cpl_msg_debug(cpl_func,"MODE METHOD FIT                                ");
  cpl_msg_debug(cpl_func,"-----------------------------------------------");
  hdrl_mode_clip(vec, 0, -1, 0, HDRL_MODE_FIT, 0, &mode, &mode_err, &naccepted);
  cpl_test_abs(mode, expected, 1);
  //cpl_test_abs(mode_err, 1.8927338794908046, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);

  //cpl_test_abs(mode_err, 1.8927338794908046, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_mode_clip_image(ima, 0, -1, 0, HDRL_MODE_FIT, 0, &mode, &mode_err,
		       &naccepted);
  cpl_test_abs(mode, expected, 1);
  //cpl_test_abs(mode_err, 1.8927338794908046, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);

  /* case error_niter < 1 */

  hdrl_mode_clip_image(ima, 0, -1, 0, HDRL_MODE_FIT, -1, &mode, &mode_err,
		       &naccepted);
  cpl_test_abs(mode, expected, 1);
  cpl_test_zero(mode_err);
  cpl_test_error(CPL_ERROR_NONE);

  if (savetodisk) {
      cpl_table* histo_tab = NULL;
      int nbins = 100;
      double bin_size = hdrl_mode_compute_binsize(vec);
      gsl_histogram * h = hdrl_mode_histogram(vec, min, max, nbins);
      histo_tab = hdrl_mode_histogram_to_table(h, min, bin_size, nbins);
      cpl_table_save(histo_tab, NULL, NULL, "histo_tab_sinfo.fits", CPL_IO_DEFAULT);

      cpl_table_delete(histo_tab);
      gsl_histogram_free(h);
  }

  /* clean memory */

  cpl_vector_delete(vec);
  cpl_image_delete(ima);
  hdrl_image_delete(hima);

  return cpl_error_get_code();
}

static cpl_error_code test_hdrl_mode_nogoodpixels(void)
{
  cpl_msg_debug(cpl_func,"test_hdrl_mode_nogoodpixels");
  cpl_size nx = 10;
  cpl_image* ima = cpl_image_new(nx, 1, CPL_TYPE_DOUBLE);
  cpl_mask* bpm = cpl_image_get_bpm(ima);
  cpl_image_set(ima, 1, 1, -1); cpl_mask_set(bpm, 1, 1, CPL_BINARY_1);
  cpl_image_set(ima, 2, 1, -1); cpl_mask_set(bpm, 2, 1, CPL_BINARY_1);
  cpl_image_set(ima, 3, 1, -1); cpl_mask_set(bpm, 3, 1, CPL_BINARY_1);
  cpl_image_set(ima, 4, 1, -1); cpl_mask_set(bpm, 4, 1, CPL_BINARY_1);
  cpl_image_set(ima, 5, 1, -1); cpl_mask_set(bpm, 5, 1, CPL_BINARY_1);
  cpl_image_set(ima, 6, 1, -1); cpl_mask_set(bpm, 6, 1, CPL_BINARY_1);
  cpl_image_set(ima, 7, 1, -1); cpl_mask_set(bpm, 7, 1, CPL_BINARY_1);
  cpl_image_set(ima, 8, 1, -1); cpl_mask_set(bpm, 8, 1, CPL_BINARY_1);
  cpl_image_set(ima, 9, 1, -1); cpl_mask_set(bpm, 9, 1, CPL_BINARY_1);
  cpl_image_set(ima, 10, 1, -1); cpl_mask_set(bpm, 10, 1, CPL_BINARY_1);

  cpl_image* err = cpl_image_duplicate(ima);
  cpl_image_power(err, 0.5);
  hdrl_image* hima = hdrl_image_create(ima, err);
  double mode = 0;
  double mode_err = 0;

  cpl_size naccepted = 0;


  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

  cpl_image_delete(err);

  cpl_image_delete(ima);
  hdrl_image_delete(hima);
  return cpl_error_get_code();
}

static cpl_error_code test_hdrl_mode_onevalue(void)
{
  cpl_msg_debug(cpl_func,"test_hdrl_mode_onevalue");
  cpl_size nx = 10;
  cpl_image* ima = cpl_image_new(nx, 1, CPL_TYPE_DOUBLE);
  cpl_image_set(ima, 1, 1, -1);
  cpl_image_set(ima, 2, 1, -1);
  cpl_image_set(ima, 3, 1, -1);
  cpl_image_set(ima, 4, 1, -1);
  cpl_image_set(ima, 5, 1, -1);
  cpl_image_set(ima, 6, 1, -1);
  cpl_image_set(ima, 7, 1, -1);
  cpl_image_set(ima, 8, 1, -1);
  cpl_image_set(ima, 9, 1, -1);
  cpl_image_set(ima, 10, 1, -1);

  cpl_image* err = cpl_image_duplicate(ima);
  cpl_image_power(err, 0.5);
  hdrl_image* hima = hdrl_image_create(ima, err);
  double mode = 0;
  double mode_err = 0;

  cpl_size naccepted = 0;


  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_image_delete(err);

  cpl_image_delete(ima);
  hdrl_image_delete(hima);
  return cpl_error_get_code();
}
static cpl_error_code test_hdrl_mode_image_one_value(void)
{
  cpl_msg_debug(cpl_func,"test_hdrl_mode_image_one_value");
  cpl_size sx = 5;
  cpl_size sy = 5;
  cpl_image* ima = cpl_image_new(sx,sy,CPL_TYPE_DOUBLE);
  cpl_image_add_scalar(ima,5);
  double mode = 0;
  double mode_err = 0;
  cpl_size naccepted = 0;
  hdrl_mode_clip_image(ima, 4.5, 5.5, 1, HDRL_MODE_FIT, -1, &mode, &mode_err,
		       &naccepted);

  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_abs(mode, 0, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0, HDRL_EPS_TEST);


  hdrl_mode_clip_image(ima, 4.5, 5.5, 1, HDRL_MODE_WEIGHTED, -1, &mode, &mode_err,
		       &naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 5, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0, HDRL_EPS_TEST);

  hdrl_mode_clip_image(ima, 4.5, 5.5, 1, HDRL_MODE_MEDIAN, -1, &mode, &mode_err,
		       &naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 5, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0, HDRL_EPS_TEST);


  hdrl_mode_clip_image(ima, 4, 6, 1, HDRL_MODE_FIT, -1, &mode, &mode_err,
		       &naccepted);

  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 5.5, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.0, HDRL_EPS_TEST);


  hdrl_mode_clip_image(ima, 4, 6, 1, HDRL_MODE_WEIGHTED, -1, &mode, &mode_err,
		       &naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 5.5, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0, HDRL_EPS_TEST);

  hdrl_mode_clip_image(ima, 4, 6, 1, HDRL_MODE_MEDIAN, -1, &mode, &mode_err,
		       &naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 5, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0, HDRL_EPS_TEST);

  cpl_image_delete(ima);
  return cpl_error_get_code();
}
static cpl_error_code test_hdrl_mode_vector_one_value(void)
{
  cpl_msg_debug(cpl_func,"test_hdrl_mode_median");

  double values[] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
      5, 5, 5, 5, 5, 5, 5};

  /*
  double values[] = {1., 2., 3., 4., 5., 6., 7., 8., 9., 10.,
                     11., 12., 13., 14., 15., 16., 17.};

   */
  cpl_vector* vec = cpl_vector_wrap(17, values);


  double mode = 0;
  double mode_err = 0;
  cpl_size naccepted = 0;

  hdrl_mode_clip(vec, 3.5, 6., 1, HDRL_MODE_FIT, 0, &mode, &mode_err,
		 &naccepted);
  cpl_test_error(CPL_ERROR_ILLEGAL_OUTPUT);



  hdrl_mode_clip(vec, 3.5, 6.5, 1, HDRL_MODE_WEIGHTED, 0, &mode, &mode_err,
		 &naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 5, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.08574929257125441, HDRL_EPS_TEST);


  // Fails HDRL_MODE_WEIGHTED
  hdrl_mode_clip(vec, 0.0, 20., 1, HDRL_MODE_WEIGHTED, 0, &mode, &mode_err,
		 &naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 5.5, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.08574929257125441, HDRL_EPS_TEST);


  cpl_vector_unwrap(vec);

  return cpl_error_get_code();
}

static cpl_error_code test_hdrl_mode_median(void)
{
  cpl_msg_debug(cpl_func,"test_hdrl_mode_median");
  double values[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  cpl_vector* vec = cpl_vector_wrap(10, values);

  double mode = 0;
  double mode_err = 0;

  hdrl_mode_median(vec, 0, 2, 3, 0, &mode, &mode_err);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 1, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0, HDRL_EPS_TEST);
  cpl_vector_unwrap(vec);
  return cpl_error_get_code();
}
/* ---------------------------------------------------------------------------*/
/**
 * @brief implements data as in asymm.fits from Lodo
 *
 * @return CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code test_hdrl_mode_asymm(void)
{
  cpl_msg_debug(cpl_func,"test_hdrl_mode_asymm");
  cpl_size nx = 17;
  cpl_image* ima = cpl_image_new(nx, 1, CPL_TYPE_DOUBLE);
  cpl_image_set(ima, 1, 1, 1.453698);
  cpl_image_set(ima, 2, 1, 1.526955);
  cpl_image_set(ima, 3, 1, 1.146273);
  cpl_image_set(ima, 4, 1, 0.9416522);
  cpl_image_set(ima, 5, 1, 1.059149);
  cpl_image_set(ima, 6, 1, 0.468435);
  cpl_image_set(ima, 7, 1, 0.4536197);
  cpl_image_set(ima, 8, 1, 0.469264);
  cpl_image_set(ima, 9, 1, 0.3145597);
  cpl_image_set(ima, 10, 1, -0.03258576);

  cpl_image_set(ima, 11, 1, -0.06351986);
  cpl_image_set(ima, 12, 1, -0.009271647);
  cpl_image_set(ima, 13, 1,  0.06780738);
  cpl_image_set(ima, 14, 1, -0.1385294);
  cpl_image_set(ima, 15, 1,  0.01233397);
  cpl_image_set(ima, 16, 1,  0.04090551);
  cpl_image_set(ima, 17, 1,  0.08584704);

  cpl_image* err = cpl_image_duplicate(ima);
  cpl_image_power(err, 0.5);
  hdrl_image* hima = hdrl_image_create(ima, err);
  double mode = 0;
  double mode_err = 0;

  cpl_size naccepted = 0;
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, 0, &mode, &mode_err,&naccepted);

  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_error_reset();
  /* this was making automatic switch to method weight
  cpl_test_abs(mode, 0.032959079472357655, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.19892636622913076, HDRL_EPS_TEST);
   */

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, -1, &mode, &mode_err,&naccepted);

  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_error_reset();
  /* this was making automatic switch to method weight
  cpl_test_abs(mode, 0.032959079472357655, HDRL_EPS_TEST);
   */
  cpl_test_zero(mode_err);
  cpl_test_error(CPL_ERROR_NONE);


  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_WEIGHTED, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.032959079472357655, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.19892636622913076, HDRL_EPS_TEST);


  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), -0.7901858605612435,
		       2.1786116577852885, 1.3033128217198686,
		       HDRL_MODE_WEIGHTED, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.032959079472357655, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.19892636622913076, HDRL_EPS_TEST);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), -0.7901858605612435,
		       2.1786116577852885, 1.,
		       HDRL_MODE_WEIGHTED, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.09870302832764533, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.384107387311211, HDRL_EPS_TEST);


  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), -1.,
		       2.1786116577852885, 1.,
		       HDRL_MODE_WEIGHTED, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.5, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.2549509756796392, HDRL_EPS_TEST);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), -1.,
		       1., 1.,
		       HDRL_MODE_WEIGHTED, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.35714285714285715, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.18239349930325996, HDRL_EPS_TEST);

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_WEIGHTED, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.032959079472357655, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.054356445, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.2236946, HDRL_EPS_TEST);


  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), -0.7901858605612435,
		       2.1786116577852885, 1.3033128217198686,
		       HDRL_MODE_MEDIAN, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.054356445, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.2236946, HDRL_EPS_TEST);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), -1,
		       2.1786116577852885, 1.3033128217198686,
		       HDRL_MODE_MEDIAN, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.94165224, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.46006256, HDRL_EPS_TEST);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), -1,
		       2.1786116577852885, 1.,
		       HDRL_MODE_MEDIAN, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.3145597, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.30404693, HDRL_EPS_TEST);


  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), -1,
		       3., 1.,
		       HDRL_MODE_MEDIAN, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.3145597, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.30404693, HDRL_EPS_TEST);


  /* error_niter < 0 */

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.054356445, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);


  cpl_image_delete(err);

  cpl_image_delete(ima);
  hdrl_image_delete(hima);
  return cpl_error_get_code();
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief implements data as in test4r.fits from Lodo
 *
 * @return CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code test_hdrl_mode_test4r(void)
{
  cpl_size nx = 51;
  cpl_image* ima = cpl_image_new(nx, 1, CPL_TYPE_DOUBLE);
  cpl_image_set(ima, 1, 1, 10.00458);
  cpl_image_set(ima, 2, 1, 9.010156);
  cpl_image_set(ima, 3, 1, 6.991111);
  cpl_image_set(ima, 4, 1, 7.01822);
  cpl_image_set(ima, 5, 1, 6.010726);
  cpl_image_set(ima, 6, 1, 6.003698);
  cpl_image_set(ima, 7, 1, 4.988562);
  cpl_image_set(ima, 8, 1, 5.005653);
  cpl_image_set(ima, 9, 1, 4.000302);

  cpl_image_set(ima, 10, 1, 3.987379);
  cpl_image_set(ima, 11, 1, 3.996887);
  cpl_image_set(ima, 12, 1, 4.01525);
  cpl_image_set(ima, 13, 1, 4.014528);
  cpl_image_set(ima, 14, 1, 3.012359);
  cpl_image_set(ima, 15, 1, 2.999663);
  cpl_image_set(ima, 16, 1, 2.991814);
  cpl_image_set(ima, 17, 1, 3.005553);
  cpl_image_set(ima, 18, 1, 2.989472);
  cpl_image_set(ima, 19, 1, 3.01091);
  cpl_image_set(ima, 20, 1, 1.99299);

  cpl_image_set(ima, 21, 1, 1.9974);
  cpl_image_set(ima, 22, 1, 1.992067);
  cpl_image_set(ima, 23, 1, 2.017085);
  cpl_image_set(ima, 24, 1, 1.980961);
  cpl_image_set(ima, 25, 1, 1.997591);
  cpl_image_set(ima, 26, 1, 1.992787);
  cpl_image_set(ima, 27, 1, 1.990686);
  cpl_image_set(ima, 28, 1, 0.9972966);
  cpl_image_set(ima, 29, 1, 1.015236);
  cpl_image_set(ima, 30, 1, 0.9991327);

  cpl_image_set(ima, 31, 1, 0.9961795);
  cpl_image_set(ima, 32, 1, 0.9818511);
  cpl_image_set(ima, 33, 1, 0.9957231);
  cpl_image_set(ima, 34, 1, 0.9882734);
  cpl_image_set(ima, 35, 1, 1.013345);
  cpl_image_set(ima, 36, 1, 0.008599423);
  cpl_image_set(ima, 37, 1, 0.0006760373);
  cpl_image_set(ima, 38, 1, -0.001155981);
  cpl_image_set(ima, 39, 1, -0.00481371);
  cpl_image_set(ima, 40, 1, -0.01359721);

  cpl_image_set(ima, 41, 1, 0.004918799);
  cpl_image_set(ima, 42, 1, 0.0004142628);
  cpl_image_set(ima, 43, 1, -0.01527452);
  cpl_image_set(ima, 44, 1, -1.006316);
  cpl_image_set(ima, 45, 1, -0.9906352);
  cpl_image_set(ima, 46, 1, -0.9964058);
  cpl_image_set(ima, 47, 1, -0.9859128);
  cpl_image_set(ima, 48, 1, -0.9941401);
  cpl_image_set(ima, 49, 1, -2.000185);
  cpl_image_set(ima, 50, 1, -1.997964);

  cpl_image_set(ima, 51, 1, -2.984013);
  cpl_image* err = NULL;
  //cpl_image* err = cpl_image_duplicate(ima);
  //cpl_image_power(err, 0.5);

  hdrl_image* hima = hdrl_image_create(ima, err);
  double mode = 0;
  double mode_err = 0;
  cpl_size naccepted = 0;

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 3.601380572859801, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 3.2971602106947917, HDRL_EPS_TEST);

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 3.601380572859801, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_WEIGHTED, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 2.4773676405741756, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.4347417015719646, HDRL_EPS_TEST);
  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_WEIGHTED, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 2.4773676405741756, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 1.9920673, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 1.5278777, HDRL_EPS_TEST);
  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 1.9920673, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  cpl_image_delete(err);
  cpl_image_delete(ima);
  hdrl_image_delete(hima);
  return cpl_error_get_code();

}
/* ---------------------------------------------------------------------------*/
/**
 * @brief implements data as in test4.fits from Lodo
 *
 * @return CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code test_hdrl_mode_test4(void)
{

  cpl_size nx = 51;
  cpl_image* ima = cpl_image_new(nx, 1, CPL_TYPE_DOUBLE);
  cpl_image_set(ima, 1, 1, 10);
  cpl_image_set(ima, 2, 1, 9);
  cpl_image_set(ima, 3, 1, 7);
  cpl_image_set(ima, 4, 1, 7);
  cpl_image_set(ima, 5, 1, 6);
  cpl_image_set(ima, 6, 1, 6);
  cpl_image_set(ima, 7, 1, 5);
  cpl_image_set(ima, 8, 1, 5);
  cpl_image_set(ima, 9, 1, 4);
  cpl_image_set(ima, 10, 1, 4);

  cpl_image_set(ima, 11, 1, 4);
  cpl_image_set(ima, 12, 1, 4);
  cpl_image_set(ima, 13, 1, 4);
  cpl_image_set(ima, 14, 1, 3);
  cpl_image_set(ima, 15, 1, 3);
  cpl_image_set(ima, 16, 1, 3);
  cpl_image_set(ima, 17, 1, 3);
  cpl_image_set(ima, 18, 1, 3);
  cpl_image_set(ima, 19, 1, 3);
  cpl_image_set(ima, 20, 1, 2);

  cpl_image_set(ima, 21, 1, 2);
  cpl_image_set(ima, 22, 1, 2);
  cpl_image_set(ima, 23, 1, 2);
  cpl_image_set(ima, 24, 1, 2);
  cpl_image_set(ima, 25, 1, 2);
  cpl_image_set(ima, 26, 1, 2);
  cpl_image_set(ima, 27, 1, 2);
  cpl_image_set(ima, 28, 1, 1);
  cpl_image_set(ima, 29, 1, 1);
  cpl_image_set(ima, 30, 1, 1);

  cpl_image_set(ima, 31, 1, 1);
  cpl_image_set(ima, 32, 1, 1);
  cpl_image_set(ima, 33, 1, 1);
  cpl_image_set(ima, 34, 1, 1);
  cpl_image_set(ima, 35, 1, 1);
  cpl_image_set(ima, 36, 1, 0);
  cpl_image_set(ima, 37, 1, 0);
  cpl_image_set(ima, 38, 1, 0);
  cpl_image_set(ima, 39, 1, 0);
  cpl_image_set(ima, 40, 1, 0);

  cpl_image_set(ima, 41, 1, 0);
  cpl_image_set(ima, 42, 1, 0);
  cpl_image_set(ima, 43, 1, 0);
  cpl_image_set(ima, 44, 1, -1);
  cpl_image_set(ima, 45, 1, -1);
  cpl_image_set(ima, 46, 1, -1);
  cpl_image_set(ima, 47, 1, -1);
  cpl_image_set(ima, 48, 1, -1);
  cpl_image_set(ima, 49, 1, -2);
  cpl_image_set(ima, 50, 1, -2);

  cpl_image_set(ima, 51, 1, -3);

  cpl_image* err = cpl_image_duplicate(ima);
  cpl_image_power(err, 0.5);

  double mode = 0;
  double mode_err = 0;
  cpl_size naccepted = 0;
  hdrl_image* hima = hdrl_image_create(ima, err);


  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_abs(mode, 3.6175831623895225, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 3.3132769022073894, HDRL_EPS_TEST);

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_abs(mode, 3.6175831623895225, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_WEIGHTED, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 2.488075996118219, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.43686674174112927, HDRL_EPS_TEST);
  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_WEIGHTED, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 2.488075996118219, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 2.0, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 1.5265421, HDRL_EPS_TEST);

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 2.0, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  cpl_image_delete(err);
  cpl_image_delete(ima);
  hdrl_image_delete(hima);
  return cpl_error_get_code();

}
/* ---------------------------------------------------------------------------*/
/**
 * @brief implements data as in test3r.fits from Lodo
 *
 * @return CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code test_hdrl_mode_test3r(void)
{
  cpl_size nx = 38;
  cpl_image* ima = cpl_image_new(nx, 1, CPL_TYPE_DOUBLE);
  cpl_image_set(ima, 1, 1, 10.01704);
  cpl_image_set(ima, 2, 1, 8.988381);
  cpl_image_set(ima, 3, 1, 7.006527);
  cpl_image_set(ima, 4, 1, 5.994881);
  cpl_image_set(ima, 5, 1, 6.005347);
  cpl_image_set(ima, 6, 1,  5.010838);
  cpl_image_set(ima, 7, 1, 4.996699);
  cpl_image_set(ima, 8, 1, 2.997336);
  cpl_image_set(ima, 9, 1, 2.983613);
  cpl_image_set(ima, 10, 1, 2.995615);

  cpl_image_set(ima, 11, 1, 2.003428);
  cpl_image_set(ima, 12, 1, 2.024438);
  cpl_image_set(ima, 13, 1, 2.000071);
  cpl_image_set(ima, 14, 1, 1.985059);
  cpl_image_set(ima, 15, 1, 1.97843);
  cpl_image_set(ima, 16, 1, 1.002653);
  cpl_image_set(ima, 17, 1, 0.9947691);
  cpl_image_set(ima, 18, 1, 1.000785);
  cpl_image_set(ima, 19, 1, 1.008936);
  cpl_image_set(ima, 20, 1, 0.9971861);

  cpl_image_set(ima, 21, 1, 0.9904818);
  cpl_image_set(ima, 22, 1, 0.9954762);
  cpl_image_set(ima, 23, 1, 1.001084);
  cpl_image_set(ima, 24, 1, -0.00915037);
  cpl_image_set(ima, 25, 1, -0.001580715);
  cpl_image_set(ima, 26, 1, -0.01343179);
  cpl_image_set(ima, 27, 1, 0.009741801);
  cpl_image_set(ima, 28, 1, -0.01869533);
  cpl_image_set(ima, 29, 1, -0.004127814);
  cpl_image_set(ima, 30, 1, 0.002222741);

  cpl_image_set(ima, 31, 1, 0.01353268);
  cpl_image_set(ima, 32, 1, -0.9949167);
  cpl_image_set(ima, 33, 1, -0.9913741);
  cpl_image_set(ima, 34, 1, -0.99041);
  cpl_image_set(ima, 35, 1, -1.003971);
  cpl_image_set(ima, 36, 1, -1.997081);
  cpl_image_set(ima, 37, 1, -2.006046);
  cpl_image_set(ima, 38, 1, -2.992429);


  cpl_image* err = cpl_image_duplicate(ima);
  cpl_image_power(err, 0.5);
  hdrl_image* hima = hdrl_image_create(ima, err);
  double mode = 0;
  double mode_err = 0;
  cpl_size naccepted = 0;

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_abs(mode, 1.5067609093941583, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 1.197924149788573, HDRL_EPS_TEST);

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_abs(mode, 1.5067609093941583, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_WEIGHTED, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.40135648757242137, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.3967385878378167, HDRL_EPS_TEST);

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_WEIGHTED, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.40135648757242137, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.0059822705, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.7660477, HDRL_EPS_TEST);

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.0059822705, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  cpl_image_delete(err);
  cpl_image_delete(ima);
  hdrl_image_delete(hima);
  return cpl_error_get_code();

}
/* ---------------------------------------------------------------------------*/
/**
 * @brief implements data as in test3.fits from Lodo
 *
 * @return CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code test_hdrl_mode_test3(void)
{

  cpl_size nx = 38;
  cpl_image* ima = cpl_image_new(nx, 1, CPL_TYPE_DOUBLE);
  cpl_image_set(ima, 1, 1, 10);
  cpl_image_set(ima, 2, 1, 9);
  cpl_image_set(ima, 3, 1, 7);
  cpl_image_set(ima, 4, 1, 6);
  cpl_image_set(ima, 5, 1, 6);
  cpl_image_set(ima, 6, 1, 5);
  cpl_image_set(ima, 7, 1, 5);
  cpl_image_set(ima, 8, 1, 3);
  cpl_image_set(ima, 9, 1, 3);
  cpl_image_set(ima, 10, 1, 3);

  cpl_image_set(ima, 11, 1, 2);
  cpl_image_set(ima, 12, 1, 2);
  cpl_image_set(ima, 13, 1, 2);
  cpl_image_set(ima, 14, 1, 2);
  cpl_image_set(ima, 15, 1, 2);
  cpl_image_set(ima, 16, 1, 1);
  cpl_image_set(ima, 17, 1, 1);
  cpl_image_set(ima, 18, 1, 1);
  cpl_image_set(ima, 19, 1, 1);
  cpl_image_set(ima, 20, 1, 1);

  cpl_image_set(ima, 21, 1, 1);
  cpl_image_set(ima, 22, 1, 1);
  cpl_image_set(ima, 23, 1, 1);
  cpl_image_set(ima, 24, 1, 0);
  cpl_image_set(ima, 25, 1, 0);
  cpl_image_set(ima, 26, 1, 0);
  cpl_image_set(ima, 27, 1, 0);
  cpl_image_set(ima, 28, 1, 0);
  cpl_image_set(ima, 29, 1, 0);
  cpl_image_set(ima, 30, 1, 0);

  cpl_image_set(ima, 31, 1, 0);
  cpl_image_set(ima, 32, 1, -1);
  cpl_image_set(ima, 33, 1, -1);
  cpl_image_set(ima, 34, 1, -1);
  cpl_image_set(ima, 35, 1, -1);
  cpl_image_set(ima, 36, 1, -2);
  cpl_image_set(ima, 37, 1, -2);
  cpl_image_set(ima, 38, 1, -3);

  cpl_image* err = cpl_image_duplicate(ima);
  cpl_image_power(err, 0.5);
  hdrl_image* hima = hdrl_image_create(ima, err);
  double mode = 0;
  double mode_err = 0;
  cpl_size naccepted = 0;

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_abs(mode, 1.4324860586022403, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 1.1801640248274137, HDRL_EPS_TEST);

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_abs(mode, 1.4324860586022403, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);


  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_WEIGHTED, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.3434700873077243, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.39085664038884216, HDRL_EPS_TEST);

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_WEIGHTED, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.3434700873077243, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.0, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.76777196, HDRL_EPS_TEST);

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.0, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  cpl_image_delete(err);
  cpl_image_delete(ima);
  hdrl_image_delete(hima);
  return cpl_error_get_code();

}

/* ---------------------------------------------------------------------------*/
/**
 * @brief implements data as in test2r.fits from Lodo
 *
 * @return CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code test_hdrl_mode_test2r(void)
{
  cpl_size nx = 32;
  cpl_image* ima = cpl_image_new(nx, 1, CPL_TYPE_DOUBLE);
  cpl_image_set(ima, 1, 1, 9.998852);
  cpl_image_set(ima, 2, 1, 9.005792);
  cpl_image_set(ima, 3, 1, 7.007078);
  cpl_image_set(ima, 4, 1, 5.99083);
  cpl_image_set(ima, 5, 1, 5.971509);
  cpl_image_set(ima, 6, 1, 4.996029);
  cpl_image_set(ima, 7, 1, 4.999716);
  cpl_image_set(ima, 8, 1, 2.984478);
  cpl_image_set(ima, 9, 1, 3.002251);
  cpl_image_set(ima, 10, 1, 2.991654);

  cpl_image_set(ima, 11, 1, 2.004251);
  cpl_image_set(ima, 12, 1, 2.009505);
  cpl_image_set(ima, 13, 1, 1.99897);
  cpl_image_set(ima, 14, 1, 1.013377);
  cpl_image_set(ima, 15, 1, 0.9919196);
  cpl_image_set(ima, 16, 1, 1.011431);
  cpl_image_set(ima, 17, 1, 0.9922368);
  cpl_image_set(ima, 18, 1, 0.9986056);
  cpl_image_set(ima, 19, 1, 0.00585786);
  cpl_image_set(ima, 20, 1, 0.01751465);

  cpl_image_set(ima, 21, 1, 0.003120024);
  cpl_image_set(ima, 22, 1, 0.007319644);
  cpl_image_set(ima, 23, 1, 0.01744251);
  cpl_image_set(ima, 24, 1, -0.01521505);
  cpl_image_set(ima, 25, 1, -0.001438193);
  cpl_image_set(ima, 26, 1, -0.02398127);
  cpl_image_set(ima, 27, 1, -0.9988093);
  cpl_image_set(ima, 28, 1, -0.9951036);
  cpl_image_set(ima, 29, 1, -1.011068);
  cpl_image_set(ima, 30, 1, -1.0117);

  cpl_image_set(ima, 31, 1, -4.004215);
  cpl_image_set(ima, 32, 1, -2.987547);


  cpl_image* err = cpl_image_duplicate(ima);
  cpl_image_power(err, 0.5);
  hdrl_image* hima = hdrl_image_create(ima, err);
  double mode = 0;
  double mode_err = 0;
  cpl_size naccepted = 0;


  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_abs(mode, 2.6150635557822244, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 2.3272304852994763, HDRL_EPS_TEST);

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_abs(mode, 2.6150635557822244, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_WEIGHTED, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 1.0875377897613978, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.46097044101825074, HDRL_EPS_TEST);

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_WEIGHTED, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 1.0875377897613978, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.017514654, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 1.2942566, HDRL_EPS_TEST);

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.017514654, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  cpl_image_delete(err);
  cpl_image_delete(ima);
  hdrl_image_delete(hima);
  return cpl_error_get_code();

}
/* ---------------------------------------------------------------------------*/
/**
 * @brief implements data as in test2.fits from Lodo
 *
 * @return CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code test_hdrl_mode_test2(void)
{

  cpl_size nx = 32;
  cpl_image* ima = cpl_image_new(nx, 1, CPL_TYPE_DOUBLE);
  cpl_image_set(ima, 1, 1, 10);
  cpl_image_set(ima, 2, 1, 9);
  cpl_image_set(ima, 3, 1, 7);
  cpl_image_set(ima, 4, 1, 6);
  cpl_image_set(ima, 5, 1, 6);
  cpl_image_set(ima, 6, 1, 5);
  cpl_image_set(ima, 7, 1, 5);
  cpl_image_set(ima, 8, 1, 3);
  cpl_image_set(ima, 9, 1, 3);
  cpl_image_set(ima, 10, 1, 3);

  cpl_image_set(ima, 11, 1, 2);
  cpl_image_set(ima, 12, 1, 2);
  cpl_image_set(ima, 13, 1, 2);
  cpl_image_set(ima, 14, 1, 1);
  cpl_image_set(ima, 15, 1, 1);
  cpl_image_set(ima, 16, 1, 1);
  cpl_image_set(ima, 17, 1, 1);
  cpl_image_set(ima, 18, 1, 1);
  cpl_image_set(ima, 19, 1, 0);
  cpl_image_set(ima, 20, 1, 0);

  cpl_image_set(ima, 21, 1, 0);
  cpl_image_set(ima, 22, 1, 0);
  cpl_image_set(ima, 23, 1, 0);
  cpl_image_set(ima, 24, 1, 0);
  cpl_image_set(ima, 25, 1, 0);
  cpl_image_set(ima, 26, 1, 0);
  cpl_image_set(ima, 27, 1, -1);
  cpl_image_set(ima, 28, 1, -1);
  cpl_image_set(ima, 29, 1, -1);
  cpl_image_set(ima, 20, 1, -1);

  cpl_image_set(ima, 31, 1, -4);
  cpl_image_set(ima, 32, 1, -3);

  cpl_image* err = cpl_image_duplicate(ima);
  cpl_image_power(err, 0.5);
  hdrl_image* hima = hdrl_image_create(ima, err);
  double mode = 0;
  double mode_err = 0;
  cpl_size naccepted = 0;

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 2.600666439655294, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 2.3206868514023493, HDRL_EPS_TEST);

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 2.600666439655294, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_WEIGHTED, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 1.0774357228117637, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.4596742987485125, HDRL_EPS_TEST);

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_WEIGHTED, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 1.0774357228117637, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.0, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 1.2945614, HDRL_EPS_TEST);

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.0, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  cpl_image_delete(err);
  cpl_image_delete(ima);
  hdrl_image_delete(hima);
  return cpl_error_get_code();

}


/* ---------------------------------------------------------------------------*/
/**
 * @brief implements data as in test1r.fits from Lodo
 *
 * @return CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code test_hdrl_mode_test1r(void)
{

  cpl_size nx = 27;
  cpl_image* ima = cpl_image_new(nx, 1, CPL_TYPE_DOUBLE);
  cpl_image_set(ima, 1, 1, 9.986042);
  cpl_image_set(ima, 2, 1, 9.011315);
  cpl_image_set(ima, 3, 1, 7.002415);
  cpl_image_set(ima, 4, 1, 5.996731);
  cpl_image_set(ima, 5, 1, 6.008002);
  cpl_image_set(ima, 6, 1, 5.004089);
  cpl_image_set(ima, 7, 1, 5.015304);
  cpl_image_set(ima, 8, 1, 3.016874);
  cpl_image_set(ima, 9, 1, 3.014342);
  cpl_image_set(ima, 10, 1, 2.993648);

  cpl_image_set(ima, 11, 1, 1.992332);
  cpl_image_set(ima, 12, 1, 1.990101);
  cpl_image_set(ima, 13, 1, 1.992047);
  cpl_image_set(ima, 14, 1, 0.9840552);
  cpl_image_set(ima, 15, 1, 1.012001);
  cpl_image_set(ima, 16, 1, 1.014128);
  cpl_image_set(ima, 17, 1, 0.9949675);
  cpl_image_set(ima, 18, 1, 1.001209);
  cpl_image_set(ima, 19, 1, -0.01172797);
  cpl_image_set(ima, 20, 1, 0.002719025);

  cpl_image_set(ima, 21, 1, 0.001201321);
  cpl_image_set(ima, 22, 1, 0.009442335);
  cpl_image_set(ima, 23, 1, 0.003206544);
  cpl_image_set(ima, 24, 1, -0.008347392);
  cpl_image_set(ima, 25, 1, -0.007489061);
  cpl_image_set(ima, 26, 1, -0.003378053);
  cpl_image_set(ima, 27, 1, 0.001325341);

  cpl_image* err = cpl_image_duplicate(ima);
  cpl_image_power(err, 0.5);
  hdrl_image* hima = hdrl_image_create(ima, err);
  double mode = 0;
  double mode_err = 0;
  cpl_size naccepted = 0;

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_error_reset();
  /* this was making automatic switch to method weight
  cpl_test_abs(mode, 0.6902527626942991, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.6089455241031, HDRL_EPS_TEST);
   */

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_error_reset();
  /* this was making automatic switch to method weight
  cpl_test_abs(mode, 0.6902527626942991, HDRL_EPS_TEST);
   */
  cpl_test_zero(mode_err);



  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_WEIGHTED, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.6902527626942991, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.6089455241031, HDRL_EPS_TEST);

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_WEIGHTED, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.6902527626942991, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.0029627844, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.49867436, HDRL_EPS_TEST);

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.0029627844, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);


  cpl_image_delete(err);
  cpl_image_delete(ima);
  hdrl_image_delete(hima);
  return cpl_error_get_code();
}
/* ---------------------------------------------------------------------------*/
/**
 * @brief implements data as in test1.fits from Lodo
 *
 * @return CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code test_hdrl_mode_test1(void)
{
  cpl_size nx = 27;
  cpl_image* ima = cpl_image_new(nx, 1, CPL_TYPE_DOUBLE);

  cpl_image_set(ima, 1, 1, 10);
  cpl_image_set(ima, 2, 1, 9);
  cpl_image_set(ima, 3, 1, 7);
  cpl_image_set(ima, 4, 1, 6);
  cpl_image_set(ima, 5, 1, 6);
  cpl_image_set(ima, 6, 1, 5);
  cpl_image_set(ima, 7, 1, 5);
  cpl_image_set(ima, 8, 1, 3);
  cpl_image_set(ima, 9, 1, 3);
  cpl_image_set(ima, 10, 1, 3);

  cpl_image_set(ima, 11, 1, 2);
  cpl_image_set(ima, 12, 1, 2);
  cpl_image_set(ima, 13, 1, 2);
  cpl_image_set(ima, 14, 1, 1);
  cpl_image_set(ima, 15, 1, 1);
  cpl_image_set(ima, 16, 1, 1);
  cpl_image_set(ima, 17, 1, 1);
  cpl_image_set(ima, 18, 1, 1);
  cpl_image_set(ima, 19, 1, 0);
  cpl_image_set(ima, 20, 1, 0);

  cpl_image_set(ima, 21, 1, 0);
  cpl_image_set(ima, 22, 1, 0);
  cpl_image_set(ima, 23, 1, 0);
  cpl_image_set(ima, 24, 1, 0);
  cpl_image_set(ima, 25, 1, 0);
  cpl_image_set(ima, 26, 1, 0);
  cpl_image_set(ima, 27, 1, 0);

  cpl_image* err = cpl_image_duplicate(ima);
  cpl_image_power(err, 0.5);
  hdrl_image* hima = hdrl_image_create(ima, err);
  double mode = 0;
  double mode_err = 0;
  cpl_size naccepted = 0;

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_error_reset();
  /* this was making automatic switch to method weight
  cpl_test_abs(mode, 0.6899031999999996, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.59846865840335, HDRL_EPS_TEST);
   */

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_error_reset();
  /* this was making automatic switch to method weight
  cpl_test_abs(mode, 0.6899031999999996, HDRL_EPS_TEST);
   */
  cpl_test_zero(mode_err);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_WEIGHTED, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.6899031999999996, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.59846865840335, HDRL_EPS_TEST);

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_WEIGHTED, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.6899031999999996, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, 0, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.0, HDRL_EPS_TEST);
  cpl_test_abs(mode_err, 0.49724513, HDRL_EPS_TEST);

  /* error_niter < 0 */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, -1, &mode, &mode_err,&naccepted);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(mode, 0.0, HDRL_EPS_TEST);
  cpl_test_zero(mode_err);

  cpl_image_delete(err);
  cpl_image_delete(ima);
  hdrl_image_delete(hima);
  return cpl_error_get_code();

}
static cpl_error_code test_hdrl_mode_parameter_create_parlist(void)
{
  /* creates a proper HDRL mode parameter */
  hdrl_parameter * mode_def =
      hdrl_collapse_mode_parameter_create(1., 100., 1., HDRL_MODE_MEDIAN,0);

  //char              *name;
  //name = hdrl_join_string(".", 2, prefix, "mode");
  const char     *prefix = "prefix";
  const char* base_context= "recipe";

  cpl_parameterlist * pmode = NULL;

  /* test improper inputs */
  pmode = hdrl_mode_parameter_create_parlist(NULL, prefix, mode_def);
  cpl_test_null(pmode);
  cpl_test_error(CPL_ERROR_NULL_INPUT);


  pmode = hdrl_mode_parameter_create_parlist(base_context, NULL, mode_def);
  cpl_test_null(pmode);
  cpl_test_error(CPL_ERROR_NULL_INPUT);


  pmode = hdrl_mode_parameter_create_parlist(base_context, prefix, NULL);
  cpl_test_null(pmode);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  /* test proper inputs */
  pmode = hdrl_mode_parameter_create_parlist(base_context, prefix,mode_def);
  cpl_test_nonnull(pmode);
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_parameter_delete(mode_def);
  cpl_parameterlist_delete(pmode);
  return cpl_error_get_code();
}

static cpl_error_code test_hdrl_mode_parameter_parse_parlist(void)
{
  /* creates a proper HDRL mode parameter */
  hdrl_parameter * mode_def =
      hdrl_collapse_mode_parameter_create(1., 100., 1., HDRL_MODE_MEDIAN,0);

  //char              *name;
  //name = hdrl_join_string(".", 2, prefix, "mode");
  const char     *prefix = "mode";
  const char* base_context= "recipe";
  cpl_parameterlist * parlist =
      hdrl_mode_parameter_create_parlist(base_context, prefix,mode_def);

  double histo_min = 0;
  double histo_max = 0;
  double bin_size = 0;
  cpl_size error_niter = 0;
  hdrl_mode_type method = HDRL_MODE_MEDIAN;
  hdrl_parameter* p = NULL;


  /* test invalid input */
  hdrl_mode_parameter_parse_parlist(NULL, prefix, &histo_min, &histo_max,
				    &bin_size, &method, &error_niter);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_mode_parameter_parse_parlist(parlist, NULL, &histo_min, &histo_max,
				    &bin_size, &method, &error_niter);
  cpl_test_error(CPL_ERROR_NULL_INPUT);


  /* test valid input */
  hdrl_mode_parameter_parse_parlist(parlist, base_context, &histo_min, &histo_max,
				    &bin_size, &method, &error_niter);
  cpl_test_error(CPL_ERROR_NONE);
  p = hdrl_collapse_mode_parameter_create(histo_min, histo_max, bin_size,
					  method, error_niter);
  cpl_test_error(CPL_ERROR_NONE);
  hdrl_parameter_delete(mode_def);
  hdrl_parameter_delete(p);
  cpl_parameterlist_delete(parlist);
  return cpl_error_get_code();
}

static void
hdrl_write_qc (cpl_propertylist *plist, cpl_table *tab)
{
  cpl_propertylist_append_double (
      plist, "ESO QC MODE_MEDIAN MEAN",
      cpl_table_get_column_mean (tab, "mode_median"));
  cpl_propertylist_append_double (
      plist, "ESO QC MODE_MEDIAN_ERR MEAN",
      cpl_table_get_column_mean (tab, "mode_median_error"));
  cpl_propertylist_append_double (
      plist, "ESO QC MODE_WEIGHT MEAN",
      cpl_table_get_column_mean (tab, "mode_weight"));
  cpl_propertylist_append_double (
      plist, "ESO QC MODE_WEIGHT_ERR MEAN",
      cpl_table_get_column_mean (tab, "mode_weight_error"));
  cpl_propertylist_append_double (plist, "ESO QC MODE_FIT MEAN",
				  cpl_table_get_column_mean (tab, "mode_fit"));
  cpl_propertylist_append_double (
      plist, "ESO QC MODE_FIT_ERR MEAN",
      cpl_table_get_column_mean (tab, "mode_fit_error"));
  cpl_propertylist_append_double (
      plist, "ESO QC MODE_MEDIAN MEDIAN",
      cpl_table_get_column_median (tab, "mode_median"));
  cpl_propertylist_append_double (
      plist, "ESO QC MODE_MEDIAN_ERR MEDIAN",
      cpl_table_get_column_median (tab, "mode_median_error"));
  cpl_propertylist_append_double (
      plist, "ESO QC MODE_WEIGHT MEDIAN",
      cpl_table_get_column_median (tab, "mode_weight"));
  cpl_propertylist_append_double (
      plist, "ESO QC MODE_WEIGHT_ERR MEDIAN",
      cpl_table_get_column_median (tab, "mode_weight_error"));
  cpl_propertylist_append_double (
      plist, "ESO QC MODE_FIT MEDIAN",
      cpl_table_get_column_median (tab, "mode_fit"));
  cpl_propertylist_append_double (
      plist, "ESO QC MODE_FIT_ERR MEDIAN",
      cpl_table_get_column_median (tab, "mode_fit_error"));
  cpl_propertylist_append_double (
      plist, "ESO QC MODE_MEDIAN STDEV",
      cpl_table_get_column_stdev (tab, "mode_median"));
  cpl_propertylist_append_double (
      plist, "ESO QC MODE_MEDIAN_ERR STDEV",
      cpl_table_get_column_stdev (tab, "mode_median_error"));
  cpl_propertylist_append_double (
      plist, "ESO QC MODE_WEIGHT STDEV",
      cpl_table_get_column_stdev (tab, "mode_weight"));
  cpl_propertylist_append_double (
      plist, "ESO QC MODE_WEIGHT_ERR STDEV",
      cpl_table_get_column_stdev (tab, "mode_weight_error"));
  cpl_propertylist_append_double (plist, "ESO QC MODE_FIT STDEV",
				  cpl_table_get_column_stdev (tab, "mode_fit"));
  cpl_propertylist_append_double (
      plist, "ESO QC MODE_FIT_ERR STDEV",
      cpl_table_get_column_stdev (tab, "mode_fit_error"));
}


static cpl_error_code
test_hdrl_mode_montecarlo_exec(double expected,
			       cpl_size iteration,
			       double * mode_median,
			       double * mode_median_error,
			       double * mode_weight,
			       double * mode_weight_error,
			       double * mode_fit,
			       double * mode_fit_error,
			       hdrl_random_state * state)
{

  cpl_size sx =  1;
  cpl_size sy =  250000;
  cpl_image* ima = cpl_image_new(sx, sy, CPL_TYPE_INT);
  int* pdata = cpl_image_get_data_int(ima);
  cpl_size size = sx * sy;

  for(cpl_size i = 0; i < size; i++) {

      pdata[i] = (int)hdrl_random_poisson(state, expected);
  }

  double mean = cpl_image_get_mean(ima);
  double median = cpl_image_get_median(ima);
  double stdev = cpl_image_get_stdev(ima);

  cpl_msg_debug(cpl_func, "Montecarlo Simulated image:");
  cpl_msg_debug(cpl_func, "Montecarlo mean: %g, median: %g, stdev: %g",
		mean, median, stdev);

  hdrl_image* hima = hdrl_image_create(ima, NULL);
  double mode_loc = 0.;
  double mode_error_loc = 0.;
  cpl_size naccepted = 0;
  //--------------------------------------------------------------------
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, 0, &mode_loc, &mode_error_loc,&naccepted);

  cpl_test_error(CPL_ERROR_NONE);

  /* Fill the final mode and error */
  *mode_median = mode_loc;
  *mode_median_error = mode_error_loc;
  //--------------------------------------------------------------------

  //--------------------------------------------------------------------
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_WEIGHTED, 0, &mode_loc, &mode_error_loc,&naccepted);

  cpl_test_error(CPL_ERROR_NONE);

  /* Fill the final mode and error */
  *mode_weight = mode_loc;
  *mode_weight_error = mode_error_loc;
  //--------------------------------------------------------------------

  //--------------------------------------------------------------------
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, 0, &mode_loc, &mode_error_loc,&naccepted);
  if(cpl_error_get_code() != CPL_ERROR_NONE) {
      /* previous call may return error if fit method fail check conditions
       * in this case we reset error to CPL_ERROR_NONE */
      cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
      cpl_error_reset();
  }

  /* Fill the final mode and error */
  *mode_fit = mode_loc;
  *mode_fit_error = mode_error_loc;
  //--------------------------------------------------------------------

  cpl_propertylist * plist = cpl_propertylist_new();
  cpl_propertylist_append_double(plist, "ESO QC LAMBDA", expected);
  cpl_propertylist_append_double(plist, "ESO QC MEAN"  , mean);
  cpl_propertylist_append_double(plist, "ESO QC MEDIAN", median);
  cpl_propertylist_append_double(plist, "ESO QC STDEV" , stdev);

  cpl_propertylist_append_double(plist, "ESO QC MODE MEDIAN", *mode_median);
  cpl_propertylist_append_double(plist, "ESO QC MODE WEIGHT", *mode_weight);
  cpl_propertylist_append_double(plist, "ESO QC MODE FIT"   , *mode_fit);
  cpl_propertylist_append_double(plist, "ESO QC MODE MEDIAN ERR", *mode_median_error);
  cpl_propertylist_append_double(plist, "ESO QC MODE WEIGHT ERR", *mode_weight_error);
  cpl_propertylist_append_double(plist, "ESO QC MODE FIT ERR"   , *mode_fit_error);

  char * outname = cpl_sprintf("Simulation_Montecarlo_mode_%d_iter%04d.fits",
			       (int)expected, (int)iteration);

  /* Save the image */
  //cpl_propertylist_save(plist,outname,CPL_IO_CREATE);
  //cpl_image_save(ima, outname, CPL_TYPE_INT, plist, CPL_IO_EXTEND);
  //cpl_image_save(err, outname, CPL_TYPE_INT, plist, CPL_IO_EXTEND);

  cpl_free(outname);
  cpl_propertylist_delete(plist);

  /* clean memory */
  cpl_image_delete(ima);
  hdrl_image_delete(hima);
  return cpl_error_get_code();
}


static cpl_error_code test_hdrl_mode_bootstrap_exec(cpl_image * ima_in,
						    cpl_size iteration,
						    double * mode_median,
						    double * mode_median_error,
						    double * mode_weight,
						    double * mode_weight_error,
						    double * mode_fit,
						    double * mode_fit_error,
						    hdrl_random_state * state)
{

  cpl_size sx =  cpl_image_get_size_x(ima_in);
  cpl_size sy =  cpl_image_get_size_x(ima_in);
  //cpl_type type = cpl_image_get_type(ima_in);
  cpl_image* ima_simul = cpl_image_new(sx, sy, CPL_TYPE_INT);
  int* pima_in = cpl_image_get_data_int(ima_in);
  int* pima_simul = cpl_image_get_data_int(ima_simul);

  //hdrl_random_state * state = hdrl_random_state_new(1, NULL);
  const cpl_size N = sx * sy;

  for (cpl_size i = 0; i < N; i++) {
      pima_simul[i] = pima_in[(cpl_size)hdrl_random_uniform_int64(state, 0, N - 1)];
      //cpl_size index = (cpl_size)(rand_0_to_1() * ((N-1) - 0) + 0);
      //pima_simul[i] = pima_in[index];
      //cpl_size upper = N - 1 ;
      //cpl_size lower = 0;
      //cpl_size index = (rand() % (upper - lower + 1)) + lower;
      //pima_simul[i] = pima_in[index];
  }

  double mean = cpl_image_get_mean(ima_simul);
  double median = cpl_image_get_median(ima_simul);
  double stdev = cpl_image_get_stdev(ima_simul);

  cpl_msg_debug(cpl_func,"Bootstrap Simulated image:");
  cpl_msg_debug(cpl_func,"Bootstrap mean: %g, median: %g, stdev: %g",
		mean, median, stdev);

  hdrl_image* hima = hdrl_image_create(ima_simul, NULL);
  double mode_loc = 0.;
  double mode_error_loc = 0.;
  cpl_size naccepted = 0;
  //--------------------------------------------------------------------
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_MEDIAN, 0, &mode_loc, &mode_error_loc,&naccepted);

  cpl_test_error(CPL_ERROR_NONE);

  /* Fill the final mode and error */
  *mode_median = mode_loc;
  *mode_median_error = mode_error_loc;
  //--------------------------------------------------------------------

  //--------------------------------------------------------------------
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_WEIGHTED, 0, &mode_loc, &mode_error_loc,&naccepted);


  cpl_test_error(CPL_ERROR_NONE);

  /* Fill the final mode and error */
  *mode_weight = mode_loc;
  *mode_weight_error = mode_error_loc;
  //--------------------------------------------------------------------

  //--------------------------------------------------------------------
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 0,
		       HDRL_MODE_FIT, 0, &mode_loc, &mode_error_loc,&naccepted);
  if(cpl_error_get_code() != CPL_ERROR_NONE) {
      /* previous call may return error if fit method fail check conditions
       * in this case we reset error to CPL_ERROR_NONE */
      cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
      cpl_error_reset();
  }
  /* Fill the final mode and error */
  *mode_fit = mode_loc;
  *mode_fit_error = mode_error_loc;
  //--------------------------------------------------------------------

  cpl_propertylist * plist = cpl_propertylist_new();
  cpl_propertylist_append_double(plist, "ESO QC MEAN"  , mean);
  cpl_propertylist_append_double(plist, "ESO QC MEDIAN", median);
  cpl_propertylist_append_double(plist, "ESO QC STDEV" , stdev);

  cpl_propertylist_append_double(plist, "ESO QC MODE MEDIAN", *mode_median);
  cpl_propertylist_append_double(plist, "ESO QC MODE WEIGHT", *mode_weight);
  cpl_propertylist_append_double(plist, "ESO QC MODE FIT"   , *mode_fit);
  cpl_propertylist_append_double(plist, "ESO QC MODE MEDIAN ERR", *mode_median_error);
  cpl_propertylist_append_double(plist, "ESO QC MODE WEIGHT ERR", *mode_weight_error);
  cpl_propertylist_append_double(plist, "ESO QC MODE FIT ERR"   , *mode_fit_error);

  char * outname = cpl_sprintf("Simulation_Bootstrap_mode_iter%04d.fits",
			       (int)iteration);

  /* Save the image */
  //cpl_propertylist_save(plist,outname,CPL_IO_CREATE);
  //cpl_image_save(ima_simul, outname, CPL_TYPE_INT, plist, CPL_IO_EXTEND);
  //cpl_image_save(err, outname, CPL_TYPE_INT, plist, CPL_IO_EXTEND);

  cpl_free(outname);
  cpl_propertylist_delete(plist);

  /* clean memory */
  //hdrl_random_state_delete(state);
  cpl_image_delete(ima_simul);
  hdrl_image_delete(hima);
  return cpl_error_get_code();
}

static cpl_error_code
test_hdrl_mode_general_montecarlo(double expected, cpl_size iterations,
				  hdrl_random_state * state, double sigfactor,
				  double relsigfactor, int savetodisk)
{
  double mode_median = 0.;
  double mode_median_error = 0.;
  double mode_weight = 0.;
  double mode_weight_error = 0.;
  double mode_fit = 0.;
  double mode_fit_error = 0.;

  /* Simulate images with a poissonian flux distribution and save the results
   * into a table */

  cpl_table * tab = cpl_table_new(iterations);
  cpl_table_new_column(tab, "lambda",            CPL_TYPE_DOUBLE);
  cpl_table_new_column(tab, "mode_median",       CPL_TYPE_DOUBLE);
  cpl_table_new_column(tab, "mode_median_error", CPL_TYPE_DOUBLE);
  cpl_table_new_column(tab, "mode_weight",       CPL_TYPE_DOUBLE);
  cpl_table_new_column(tab, "mode_weight_error", CPL_TYPE_DOUBLE);
  cpl_table_new_column(tab, "mode_fit",          CPL_TYPE_DOUBLE);
  cpl_table_new_column(tab, "mode_fit_error",    CPL_TYPE_DOUBLE);

  for (cpl_size i = 0; i < iterations; i++) {
      test_hdrl_mode_montecarlo_exec(expected,
				     i,
				     &mode_median,
				     &mode_median_error,
				     &mode_weight,
				     &mode_weight_error,
				     &mode_fit,
				     &mode_fit_error,
				     state);
      cpl_table_set_double(tab, "lambda",            i, expected);
      cpl_table_set_double(tab, "mode_median",       i, mode_median);
      cpl_table_set_double(tab, "mode_median_error", i, mode_median_error);
      cpl_table_set_double(tab, "mode_weight",       i, mode_weight);
      cpl_table_set_double(tab, "mode_weight_error", i, mode_weight_error);
      cpl_table_set_double(tab, "mode_fit",          i, mode_fit);
      cpl_table_set_double(tab, "mode_fit_error",    i, mode_fit_error);

  }

  /* Check if the calculated values are compatible with the expectations
   * within sigfactor * standard-deviation of the the calculated errorbars */

  cpl_test_abs(
      cpl_table_get_column_median(tab, "mode_median"), expected,
      cpl_table_get_column_stdev (tab, "mode_median") * sigfactor);
  cpl_test_abs(
      cpl_table_get_column_median(tab, "mode_weight"), expected,
      cpl_table_get_column_stdev (tab, "mode_weight") * sigfactor);
  cpl_test_abs(
      cpl_table_get_column_median(tab, "mode_fit"), expected,
      cpl_table_get_column_stdev (tab, "mode_fit") * sigfactor);

  /* Check if the calculated standard deviation is not to large and in the
   * order of 1 percent */
  cpl_msg_debug(cpl_func, "lambda: %g", expected);
  double relerr_median = cpl_table_get_column_stdev (tab, "mode_median")
		  / expected;
  double relerr_weight = cpl_table_get_column_stdev (tab, "mode_weight")
		  / expected;
  double relerr_fit = cpl_table_get_column_stdev (tab, "mode_fit")
		  / expected;

  cpl_test(relerr_median < relsigfactor);
  cpl_test(relerr_weight < relsigfactor);
  //TODO: the following fails occasionally
  cpl_test(relerr_fit < relsigfactor);

  if(savetodisk) {
      cpl_propertylist * plist = cpl_propertylist_new();
      hdrl_write_qc (plist, tab);
      char * outname = cpl_sprintf("Simultable_mode_%08d_montecarlo.fits",
				   (int)expected);
      cpl_table_save(tab, plist, NULL, outname, CPL_IO_CREATE);
      cpl_free(outname);
      cpl_propertylist_delete(plist);
  }

  cpl_table_delete(tab);

  return cpl_error_get_code();
}

static cpl_error_code
test_hdrl_mode_general_bootstrap(double expected, cpl_size iterations,
				 hdrl_random_state * state, double sigfactor,
				 double relsigfactor, int savetodisk)
{

  double mode_median = 0.;
  double mode_median_error = 0.;
  double mode_weight = 0.;
  double mode_weight_error = 0.;
  double mode_fit = 0.;
  double mode_fit_error = 0.;

  cpl_size sx =  500;
  cpl_size sy =  500;
  cpl_image* ima = cpl_image_new(sx, sy, CPL_TYPE_INT);
  int* pdata = cpl_image_get_data(ima);
  cpl_size size = sx * sy;

  /* Simulate an image with a poissonian flux distribution */
  for(cpl_size i = 0; i < size; i++) {
      pdata[i] = (int)hdrl_random_poisson(state, expected);
  }

  /* Derive new images by using the bootstrap method and save the results into
   * a table */

  cpl_table * tab = cpl_table_new(iterations);
  cpl_table_new_column(tab, "lambda",            CPL_TYPE_DOUBLE);
  cpl_table_new_column(tab, "mode_median",       CPL_TYPE_DOUBLE);
  cpl_table_new_column(tab, "mode_median_error", CPL_TYPE_DOUBLE);
  cpl_table_new_column(tab, "mode_weight",       CPL_TYPE_DOUBLE);
  cpl_table_new_column(tab, "mode_weight_error", CPL_TYPE_DOUBLE);
  cpl_table_new_column(tab, "mode_fit",          CPL_TYPE_DOUBLE);
  cpl_table_new_column(tab, "mode_fit_error",    CPL_TYPE_DOUBLE);

  for (cpl_size i = 0; i < iterations; i++) {
      test_hdrl_mode_bootstrap_exec(ima, i,
				    &mode_median, &mode_median_error,
				    &mode_weight, &mode_weight_error,
				    &mode_fit, &mode_fit_error,
				    state);

      cpl_table_set_double(tab, "lambda",            i, expected);
      cpl_table_set_double(tab, "mode_median",       i, mode_median);
      cpl_table_set_double(tab, "mode_median_error", i, mode_median_error);
      cpl_table_set_double(tab, "mode_weight",       i, mode_weight);
      cpl_table_set_double(tab, "mode_weight_error", i, mode_weight_error);
      cpl_table_set_double(tab, "mode_fit",          i, mode_fit);
      cpl_table_set_double(tab, "mode_fit_error",    i, mode_fit_error);
  }
  cpl_test_error(CPL_ERROR_NONE);

  /* Check if the calculated values are compatible with the expectations
   * within sigfactor * standard-deviation of the the calculated errorbars */

  cpl_test_abs(
      cpl_table_get_column_median(tab, "mode_median"), expected,
      cpl_table_get_column_stdev (tab, "mode_median") * sigfactor);
  cpl_test_abs(
      cpl_table_get_column_median(tab, "mode_weight"), expected,
      cpl_table_get_column_stdev (tab, "mode_weight") * sigfactor);
  cpl_test_abs(
      cpl_table_get_column_median(tab, "mode_fit"), expected,
      cpl_table_get_column_stdev (tab, "mode_fit") * sigfactor);

  /* Check if the calculated standard deviation is not to large and in the
   * order of 1 percent */

  cpl_msg_debug(cpl_func, "lambda: %g", expected);
  double relerr_median = cpl_table_get_column_stdev (tab, "mode_median")
		  / expected;
  double relerr_weight = cpl_table_get_column_stdev (tab, "mode_weight")
		  / expected;
  double relerr_fit = cpl_table_get_column_stdev (tab, "mode_fit")
		  / expected;

  cpl_test(relerr_median < relsigfactor);
  cpl_test(relerr_weight < relsigfactor);
  cpl_test(relerr_fit < relsigfactor);

  if(savetodisk) {
      cpl_propertylist * plist = cpl_propertylist_new();
      hdrl_write_qc (plist, tab);
      char * outname = cpl_sprintf("Simultable_mode_%08d_bootstrap.fits",
				   (int)expected);
      cpl_table_save(tab, plist, NULL, outname, CPL_IO_CREATE);
      cpl_free(outname);
      cpl_propertylist_delete(plist);
  }

  cpl_image_delete(ima);
  cpl_table_delete(tab);
  return cpl_error_get_code();
}


static cpl_error_code
test_hdrl_mode_bootstrap_results (double lambda, cpl_size iterations,
				  hdrl_mode_type method, double binsize,
				  double expected_error,
				  double sigfactor)
{

  cpl_size vec_size =  250000;
  cpl_vector * vec = cpl_vector_new(vec_size);
  double * pvec = cpl_vector_get_data(vec);

  uint64_t seed[2] = {(uint64_t)1804289383, (uint64_t)846930886};
  hdrl_random_state * state = hdrl_random_state_new(1, seed);

  /* Simulate a vector with a poissonian flux distribution */
  for(cpl_size i = 0; i < vec_size; i++) {
      pvec[i] = (double)hdrl_random_poisson(state, lambda);
  }
  hdrl_random_state_delete(state);

  double mode_error = 0.;

  hdrl_mode_bootstrap(vec, 10, 1, binsize, method, iterations,
		      &mode_error);
  cpl_test_rel(expected_error, mode_error, sigfactor);

  /*
  if(method == HDRL_MODE_MEDIAN){
      cpl_msg_debug(cpl_func, "MEDIAN: vector-mean: %.2g, mode-error: %g",
		    cpl_vector_get_mean(vec), mode_error);
  } else if (method == HDRL_MODE_WEIGHTED) {
      cpl_msg_debug(cpl_func, "WEIGHTED: vector-mean: %.2g, mode-error: %g",
		    cpl_vector_get_mean(vec), mode_error);
  } else {
      cpl_msg_debug(cpl_func, "FIT: vector-mean: %.2g, mode-error: %g",
		    cpl_vector_get_mean(vec), mode_error);
  }
   */

  cpl_vector_delete(vec);

  return cpl_error_get_code();
}
static cpl_error_code
test_hdrl_mode_bootstrap_stability (double lambda, cpl_size iterations1,
				    cpl_size iterations2,
				    hdrl_mode_type method,
				    double binsize,
				    double difference)
{

  cpl_size vec_size =  250000;
  cpl_vector * vec = cpl_vector_new(vec_size);
  double * pvec = cpl_vector_get_data(vec);

  uint64_t seed[2] = {(uint64_t)1804289383, (uint64_t)846930886};
  hdrl_random_state * state = hdrl_random_state_new(1, seed);

  /* Simulate a vector with a poissonian flux distribution */
  for(cpl_size i = 0; i < vec_size; i++) {
      pvec[i] = (double)hdrl_random_poisson(state, lambda);
  }
  hdrl_random_state_delete(state);

  double mode_error1 = 0.;
  double mode_error2 = 0.;

  hdrl_mode_bootstrap(vec, 10, 1, binsize, method, iterations1,
		      &mode_error1);
  hdrl_mode_bootstrap(vec, 10, 1, binsize, method, iterations2,
		      &mode_error2);
  cpl_test_rel(mode_error1, mode_error2, difference);

  /*
  if(method == HDRL_MODE_MEDIAN){
      cpl_msg_debug(cpl_func, "MEDIAN: vector-mean: %.2g, mode-error1: %g,"
		    " mode-error2: %g, ratio: %g",
		    cpl_vector_get_mean(vec), mode_error1, mode_error2, mode_error1/mode_error2);
  } else if (method == HDRL_MODE_WEIGHTED) {
      cpl_msg_debug(cpl_func, "WEIGHTED: vector-mean: %.2g, mode-error1: %g, "
		    "mode-error2: %g, ratio: %g",
		    cpl_vector_get_mean(vec), mode_error1, mode_error2, mode_error1/mode_error2);
  } else if (method == HDRL_MODE_FIT) {
      cpl_msg_debug(cpl_func, "FIT: vector-mean: %.2g, mode-error1: %g, "
		    "mode-error2: %g, ratio: %g",
		    cpl_vector_get_mean(vec), mode_error1, mode_error2, mode_error1/mode_error2);
  } else {
      cpl_msg_debug(cpl_func,"wrong");
  }
   */

  cpl_vector_delete(vec);

  return cpl_error_get_code();
}

static cpl_error_code test_hdrl_mode_image_threevalues(void)
{
  cpl_size nx = 12;
  cpl_image* ima = cpl_image_new(nx, 1, CPL_TYPE_DOUBLE);
  cpl_image_set(ima, 1,  1, 1);
  cpl_image_set(ima, 2,  1, 2);
  cpl_image_set(ima, 3,  1, 2);
  cpl_image_set(ima, 4,  1, 3);
  cpl_image_set(ima, 5,  1, 3);
  cpl_image_set(ima, 6,  1, 3);
  cpl_image_set(ima, 7,  1, 3);
  cpl_image_set(ima, 8,  1, 3);
  cpl_image_set(ima, 9,  1, 3);
  cpl_image_set(ima, 10, 1, 4);
  cpl_image_set(ima, 11, 1, 4);
  cpl_image_set(ima, 12, 1, 5);

  cpl_image* err = cpl_image_duplicate(ima);
  cpl_image_power(err, 0.5);
  hdrl_image* hima = hdrl_image_create(ima, err);
  double mode = 0;
  double mode_err = 0;

  cpl_size naccepted = 0;

  /* Symmetric histogram */
  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 1,
		       HDRL_MODE_FIT, 0, &mode, &mode_err,&naccepted);
  cpl_msg_warning(cpl_func, "HDRL_MODE_FIT: %g, %g", mode, mode_err);
  cpl_test_abs(mode, 3.0, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 1,
		       HDRL_MODE_MEDIAN, 0, &mode, &mode_err,&naccepted);
  cpl_msg_warning(cpl_func, "HDRL_MODE_MEDIAN: %g, %g", mode, mode_err);
  cpl_test_abs(mode, 3.0, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 1,
		       HDRL_MODE_WEIGHTED, 0, &mode, &mode_err,&naccepted);
  cpl_msg_warning(cpl_func, "HDRL_MODE_WEIGHTED: %g, %g", mode, mode_err);
  cpl_test_abs(mode, 3.0, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);


  /* Asymmetric histogram */

  /* Was cpl_image_set(ima, 10, 1, 4); */
  hdrl_image_set_pixel(hima, 10, 1, (hdrl_value){2.0, sqrt(2.)});

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 1,
		       HDRL_MODE_FIT, 0, &mode, &mode_err,&naccepted);
  cpl_msg_warning(cpl_func, "HDRL_MODE_FIT: %g, %g", mode, mode_err);
  cpl_test(mode < 3.);
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 1,
		       HDRL_MODE_MEDIAN, 0, &mode, &mode_err,&naccepted);
  cpl_msg_warning(cpl_func, "HDRL_MODE_MEDIAN: %g, %g", mode, mode_err);
  cpl_test_abs(mode, 3.0, HDRL_EPS_TEST);
  cpl_test_error(CPL_ERROR_NONE);

  hdrl_mode_clip_image(hdrl_image_get_image_const(hima), 0, -1, 1,
		       HDRL_MODE_WEIGHTED, 0, &mode, &mode_err,&naccepted);
  cpl_msg_warning(cpl_func, "HDRL_MODE_WEIGHTED: %g, %g", mode, mode_err);
  cpl_test(mode < 3.);
  cpl_test_error(CPL_ERROR_NONE);

  cpl_image_delete(err);
  cpl_image_delete(ima);
  hdrl_image_delete(hima);
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
  test_hdrl_mode_vector_one_value();
  test_hdrl_mode_image_one_value();
  test_hdrl_mode_image_threevalues();

  test_hdrl_mode_parameter_create_parlist();
  test_hdrl_mode_parameter_parse_parlist();
  test_hdrl_mode_nogoodpixels();
  test_hdrl_mode_onevalue();
  test_hdrl_mode_median();
  test_hdrl_mode_asymm();
  test_hdrl_mode_test1();
  test_hdrl_mode_test1r();
  test_hdrl_mode_test2();
  test_hdrl_mode_test2r();
  test_hdrl_mode_test3();
  test_hdrl_mode_test3r();
  test_hdrl_mode_test4();
  test_hdrl_mode_test4r();


  /* To ensure new random numbers each time the compiled program is called,
   * call srand(), e.g.
   * srand(time(NULL)) */

  hdrl_random_state * state = NULL;
  uint64_t seed[2] = {(uint64_t)1804289383, (uint64_t)846930886};


  state = hdrl_random_state_new(1, seed);
  test_hdrl_mode(state, 0);
  hdrl_random_state_delete(state);

  //lambda = 10000

  state = hdrl_random_state_new(1, seed);
  test_hdrl_mode_general_montecarlo(10000.,  100, state, 1.0, 0.01, 0);
  hdrl_random_state_delete(state);

  state = hdrl_random_state_new(1, seed);
  test_hdrl_mode_general_bootstrap (10000.,  100, state, 1.0, 0.01, 0);
  hdrl_random_state_delete(state);


  // lambda = 10.000

  test_hdrl_mode_bootstrap_results (10000., 200, HDRL_MODE_MEDIAN,   25.,  6.0, 0.50);
  test_hdrl_mode_bootstrap_results (10000., 200, HDRL_MODE_FIT,      100., 3.0, 0.50);
  test_hdrl_mode_bootstrap_results (10000., 200, HDRL_MODE_WEIGHTED, 20.,  5.0, 0.50);

#if defined HDRL_UNITTESTS_SLOW
  //lambda = 1000
  test_hdrl_mode_bootstrap_stability (1000, 800, 200, HDRL_MODE_MEDIAN,   3., 0.35);
  test_hdrl_mode_bootstrap_stability (1000, 800, 200, HDRL_MODE_FIT,      3., 0.35);
  test_hdrl_mode_bootstrap_stability (1000, 800, 200, HDRL_MODE_WEIGHTED, 3., 0.35);

  //lambda = 10.000
  test_hdrl_mode_bootstrap_stability (10000, 800, 200, HDRL_MODE_MEDIAN,   10., 0.35);
  test_hdrl_mode_bootstrap_stability (10000, 800, 200, HDRL_MODE_FIT,      10., 0.35);
  test_hdrl_mode_bootstrap_stability (10000, 800, 200, HDRL_MODE_WEIGHTED, 10., 0.35);
#endif

  //lambda = 10.000
  test_hdrl_mode_bootstrap_stability (10000, 400, 100, HDRL_MODE_MEDIAN,   10., 0.35);
  test_hdrl_mode_bootstrap_stability (10000, 400, 100, HDRL_MODE_FIT,      10., 0.35);
  test_hdrl_mode_bootstrap_stability (10000, 400, 100, HDRL_MODE_WEIGHTED, 10., 0.35);

#if defined HDRL_UNITTESTS_SLOW
  //lambda = 100.000
  test_hdrl_mode_bootstrap_stability (100000, 800, 200, HDRL_MODE_MEDIAN,   30., 0.35);
  test_hdrl_mode_bootstrap_stability (100000, 800, 200, HDRL_MODE_FIT,      30., 0.35);
  test_hdrl_mode_bootstrap_stability (100000, 800, 200, HDRL_MODE_WEIGHTED, 30., 0.35);
#endif


  return cpl_test_end(0);
}
