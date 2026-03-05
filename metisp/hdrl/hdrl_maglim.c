/* $Id: hdrl_maglim.c,v 1.15 2013-09-24 14:58:54 jtaylor Exp $
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

#include "hdrl_maglim.h"
#include <cpl.h>
#include <math.h>

/*-----------------------------------------------------------------------------
                                   Static
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_maglim   Limiting Magnitude Module

  @brief
  This module contains a function to compute the limiting magnitude of an image.
  Several methods are available to deal with different data points distributions


  The limiting magnitude is a quantity that characterizes the depth of an
  observation.
  It is one of the mandatory header keywords in the Phase3 archival standard for
  imaging and datacubes.  HDRL implements a general algorithm to compute the
  magnitude limit of an image (or can be used to compute it for each plan of a
  data cube).
  The limiting magnitude, according to the Phase3 archive standard, is defined
  as the magnitude of a unresolved source whose flux is 5 times the noise
  background (e.g., the magnitude of a point like source detected with
  S/N = 5).
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/** @cond PRIVATE */


/** @endcond PRIVATE */

/*----------------------------------------------------------------------------*/
/**
  @brief   Computes the limiting magnitude of an image
  @ingroup hdrl_maglim
  @param   image          input image
  @param   zeropoint      the zero point
  @param   fwhm    the FWHM seeing [pix]
  @param   kernel_size_x  X kernel size [pix]
  @param   kernel_size_y  Y kernel size [pix]
  @param   image_extend_method         method used to extend the image
  @param   mode_parameter HDRL parameter controlling the mode computation
  @param   limiting_magnitude  the computed limiting magnitude

  @return   @c CPL_ERROR_NONE or the appropriate error code.

  Please make sure that:
  - image != NULL,
  - kernel_size_x, kernel_size_y are even numbers,
  - mode_parameter is an hdrl_parameter created by the appropriate function
  (hdrl_collapse_mode_parameter_create).


  A 2D Gaussian kernel of (X,Y) FWHM kernel_size_x, kernel_size_y is created.

  The input image is convolved with the kernel as specified by
  image_extend_method. If the image has bad pixels the developer is supposed
  to flag them in its bad pixel extension.

  Then is computed the mode of the convolved image according to the parameters
  specified by the HDRL parameter mode_parameter.

  Next are flagged all data points with intensity below the mode.

  Finally it is determined the limiting magnitude as:

  limiting_magnitude = -2.5 * log10(5. * noise * norm) + zeropoint

  where noise is the noise = mad * 1.4826 / sqrt(1. -2. / 3.141592)

  mad is the Median Absolute Deviation determined on the flagged image,

  norm = 4.0 * 3.141592 * (fwhm / 2.354820045)


 */
/*----------------------------------------------------------------------------*/

cpl_error_code
hdrl_maglim_compute(const cpl_image * image, const double zeropoint,
		    const double fwhm, const cpl_size kernel_size_x,
		    const cpl_size kernel_size_y,
	            const hdrl_image_extend_method image_extend_method,
		    const hdrl_parameter * mode_parameter,
		    double* limiting_magnitude)
{
  cpl_error_ensure(fwhm > 0, CPL_ERROR_ILLEGAL_INPUT, return
                   CPL_ERROR_ILLEGAL_INPUT, "fwhm must be > 0");

  cpl_error_ensure(kernel_size_x > 0, CPL_ERROR_ILLEGAL_INPUT, return
                     CPL_ERROR_ILLEGAL_INPUT, "kernel_size_x must be > 0");

  cpl_error_ensure(kernel_size_y > 0, CPL_ERROR_ILLEGAL_INPUT, return
                     CPL_ERROR_ILLEGAL_INPUT, "kernel_size_y must be > 0");

  cpl_error_ensure(image_extend_method == HDRL_IMAGE_EXTEND_MIRROR ||
		   image_extend_method == HDRL_IMAGE_EXTEND_NEAREST,
		   CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
		       "image extension method can be 'HDRL_IMAGE_EXTEND_MIRROR'"
		       "or 'HDRL_IMAGE_EXTEND_NEAREST' only");

  cpl_error_ensure(hdrl_collapse_parameter_is_mode(mode_parameter),
                    CPL_ERROR_INCOMPATIBLE_INPUT, return
                    CPL_ERROR_INCOMPATIBLE_INPUT,
                    "Not a mode parameter");

  cpl_error_ensure((kernel_size_x % 2 != 0) && (kernel_size_y % 2 != 0),
		   CPL_ERROR_INCOMPATIBLE_INPUT,
		   return CPL_ERROR_INCOMPATIBLE_INPUT,
                   "The size of the convolution kernel must be odd in x and y");

  cpl_msg_debug(cpl_func, "Convolution kernel: X size: %lld Y size: %lld, "
		"FWHM: %16.14g", kernel_size_x, kernel_size_y, fwhm);

  /* build convolution kernel */
  cpl_matrix * kernel = hdrl_maglim_kernel_create(kernel_size_x, kernel_size_y,
						  fwhm);
  /* Convolve the image */
  cpl_image * ima_convolved = hdrl_image_convolve(image, kernel,
						  image_extend_method);

  cpl_matrix_delete(kernel);

  // compute the mode of the image
  hdrl_image* hima = hdrl_image_create(ima_convolved, NULL);
  cpl_image_delete(ima_convolved);
  double histo_min = hdrl_collapse_mode_parameter_get_histo_min(mode_parameter);
  double histo_max = hdrl_collapse_mode_parameter_get_histo_max(mode_parameter);
  double bin_size = hdrl_collapse_mode_parameter_get_bin_size(mode_parameter);
  hdrl_mode_type mode_method =
      hdrl_collapse_mode_parameter_get_method(mode_parameter);

  /*
   * We do not need the error here as the limiting magnitude algorithm does not
   * deliver any error
   * cpl_size error_niter = hdrl_collapse_mode_parameter_get_error_niter(mode_parameter);
   * */

  cpl_size error_niter = 0;

  hdrl_value hmode = hdrl_image_get_mode(hima, histo_min, histo_max, bin_size,
					 mode_method, error_niter);

  double mode = hmode.data;

  double norm = 1;
  cpl_msg_debug(cpl_func,"Computing noise and limiting magnitude ...");


 /*
  *      The value CPL_BINARY_1 is assigned where the pixel value is not marked
  *      as rejected and is strictly inside the provided interval. The other
  *      positions are assigned CPL_BINARY_0
*/
  cpl_mask* bpm = cpl_mask_threshold_image_create(hdrl_image_get_image(hima),
						  mode, DBL_MAX);

  /* Adding the original bad pixels if present */
  cpl_mask * hbpm = hdrl_image_get_mask(hima);
  cpl_mask_or(bpm, hbpm);
  hdrl_image_reject_from_mask(hima, bpm);
  cpl_mask_delete(bpm);

  double arg = (1. -2. / CPL_MATH_PI);
  double correction_factor =  1. / sqrt(arg);
  double mad = 0.;
  cpl_image_get_mad(hdrl_image_get_image_const(hima), &mad);
  if (mad <= 0) {
      mad = nextafter(0,1.0);
  }
  double noise = mad * CPL_MATH_STD_MAD * correction_factor;

  double factor = (fwhm / CPL_MATH_FWHM_SIG );
  factor *= factor;
  norm = 4. * CPL_MATH_PI * factor;

  *limiting_magnitude = -2.5 * log10(5. * noise * norm) + zeropoint;

  cpl_msg_debug(cpl_func,"Computed values: M.A.D. %g std (from M.A.D.) %g "
      "correction_factor %g norm %g", mad, mad * CPL_MATH_STD_MAD,
    	       correction_factor, norm);
  cpl_msg_debug(cpl_func,"Computed values: mode %16.14g stdev %16.14g "
      "correction_factor %16.14g noise %16.14g Limiting Magnitude %10.7g", mode,
	       hdrl_image_get_stdev(hima), correction_factor, noise,
	       *limiting_magnitude);

  hdrl_image_delete(hima);

  return cpl_error_get_code();
}


/**@}*/

