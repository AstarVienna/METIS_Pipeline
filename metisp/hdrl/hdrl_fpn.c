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

#include "hdrl_fpn.h"


/*---------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_fpn   Fixed pattern noise detection
 *
 * @brief
 *  Algorithms to compute fixed pattern noise on a single image
 *
 * The routine in this module can be used to detect fixed pattern noise in an
 * image. The algorithm first computes the power spectrum of the image using
 * the Fast Fourier Transform (FFT) as follows:
 *
 *   \f{eqnarray*}{
 *         fft         & = & FFT\_2D(\ img\ ) \\
 *         power\_spec & = & abs(\ fft\ )^2
 *  \f}
 *
 * Then it computes the standard deviation (std) and the mad-based std of the
 * power_spectrum excluding the masked region. For this the user can provide an
 * optional mask or use the dc_mask_x and dc_mask_y function parameter to create
 * one on the fly. The mask created on the fly will start at pixel (1,1) and
 * extend in both direction up to (dc_mask_x, dc_mask_y).
 *
 * \note The power spectrum contains the DC component (the DC term is the 0 Hz
 * term and is equivalent to the average of all the samples in the window)
 * in pixel (1,1)
 *
 * \note The mask created on the fly and the optional mask are combined and
 * are both taken into account.
 *
 *
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief Algorithms to compute fixed pattern noise on a single image
 *
 * @param img_in             input cpl image (bad pixels are not allowed)
 * @param mask_in            optional input cpl mask applied to the power spectrum (or NULL).
 * @param dc_mask_x          x-pixel window (>= 1) to discard DC component starting form pixel (1, 1)
 * @param dc_mask_y          y-pixel window (>= 1) to discard DC component starting from pixel (1, 1)
 * @param power_spectrum     output power spectrum image with associated mask.
 * @param std                output standard deviation of the power spectrum
 * @param std_mad            output standard deviation of the power spectrum based on the MAD
 *
 *
 *
 * The function detects fixed pattern noise on the image (img_in). The algorithm
 *  first computes the power spectrum (power_spectrum) of the image using the
 *  Fast Fourier Transform (FFT) as follows:
 *
 *   \f{eqnarray*}{
 *         fft         & = & FFT\_2D(\ img\ ) \\
 *         power\_spec & = & abs(\ fft\ )^2
 *  \f}
 *
 * Then it computes the standard deviation (std) and the mad-based std (std_mad)
 * of the power_spectrum excluding the masked region. For this the user can
 * provide an optional mask or use the dc_mask_x and dc_mask_y function
 * parameter to create one on the fly. The mask created on the fly will start at
 * pixel (1,1) and extend in both direction up to (dc_mask_x, dc_mask_y).
 *
 * \note The power spectrum contains the DC component (the DC term is the 0 Hz
 * term and is equivalent to the average of all the samples in the window)
 * in pixel (1,1)
 *
 * \note The mask created on the fly by setting dc_mask_x and dc_mask_y and the
 * optional mask (mask_in) are combined and are both taken into account when
 * calculating (std) and (mad_std).
 *
 * \note The final mask used to derive (std) and (mad_std) is attached to the
 * (power_spectrum) image as normal cpl_mask and can be retrieved by using the
 * cpl function cpl_image_get_bpm(power_spectrum)
 *
 * Possible cpl_error_code_ set in this function:
 * - CPL_ERROR_NULL_INPUT          If img_in is NULL
 * - CPL_ERROR_ILLEGAL_INPUT       If dc_mask_x < 1 or dc_mask_y < 1
 * - CPL_ERROR_ILLEGAL_INPUT       If the power_spectrum is NOT NULL
 * - CPL_ERROR_ILLEGAL_INPUT       If img_in contains bad pixels
 * - CPL_ERROR_INCOMPATIBLE_INPUT  If mask NOT NULL and size(mask) != size(img_in)
 *
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_fpn_compute(
    cpl_image      *img_in,
    const cpl_mask *mask_in,
    const cpl_size dc_mask_x,
    const cpl_size dc_mask_y,
    cpl_image      **power_spectrum,
    double         *std,
    double         *std_mad)
{
  /* Check Entries */
  cpl_ensure_code(img_in, CPL_ERROR_NULL_INPUT);

  cpl_ensure_code(   dc_mask_x >= 1 && dc_mask_y >= 1
                  && *power_spectrum == NULL, CPL_ERROR_ILLEGAL_INPUT);

  /* Check all of the pixels are good */
  if (cpl_image_count_rejected(img_in) != 0) {
      return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                                   "The image can't contain bad pixels");
  }

  /* Check if the size of the mask match with the image */
  cpl_size nx = cpl_image_get_size_x(img_in);
  cpl_size ny = cpl_image_get_size_y(img_in);
  if (mask_in) {
      cpl_ensure_code(   nx == cpl_mask_get_size_x(mask_in)
                      && ny == cpl_mask_get_size_y(mask_in),
                      CPL_ERROR_INCOMPATIBLE_INPUT);
  }

  /* Create ouput image */
  *power_spectrum = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);

  /* Calculate the fft of the input image
   * Converted previously to complex to get nx columns instead of (nx / 2) + 1 */
  cpl_image *img_in_complex = cpl_image_cast(img_in, CPL_TYPE_DOUBLE_COMPLEX);
  cpl_image *fft_image      = cpl_image_new(nx, ny,  CPL_TYPE_DOUBLE_COMPLEX);
  cpl_fft_image(fft_image, img_in_complex, CPL_FFT_FORWARD);
  cpl_image_delete(img_in_complex);

  /* Extract a double complex array */
  double complex *fft_image_array  = cpl_image_get_data_double_complex(fft_image);

  /* Calculate the power spectrum as: cabs(value)**2 == value x value_conjugate
   * Normalize image: cpl_fft_image scale the image in the number of elements */
  double norm_size = nx * ny;
  for (cpl_size y = 0; y < ny; y++) {
      for (cpl_size x = 0; x < nx ; x++) {

          /* Get position in the double complex array */
          cpl_size idx = (y * nx) + x;

          /* Get power spectrum of each cell and apply the normalization */
          double complex value_complex = fft_image_array[idx];
          double norm_value = creal(value_complex * conj(value_complex)) / norm_size;

          /* Save the normalized value in the output image */
          cpl_image_set(*power_spectrum, x + 1, y + 1, norm_value);
      }
  }
  cpl_image_delete(fft_image);


  /* If exist mask, use it and add region defined by dc_mask_x/y  */
  cpl_mask *out_mask = NULL;
  if (mask_in) {
      out_mask = cpl_mask_duplicate(mask_in);
  } else {
      out_mask = cpl_mask_new(nx, ny);
  }

  /* Check and apply the mask */
  for (cpl_size i = 1; i <= dc_mask_x; i++) {
      for (cpl_size j = 1; j <= dc_mask_y; j++) {
          cpl_mask_set(out_mask, i, j, CPL_BINARY_1);
      }
  }
  cpl_image_reject_from_mask(*power_spectrum, out_mask);

  /* Cleanup */
  cpl_mask_delete(out_mask);


  /* Get the STD */
  *std = cpl_image_get_stdev(*power_spectrum);


  /* Get the MAD */
  double mad = 0.;
  cpl_image_get_mad(*power_spectrum, &mad);
  *std_mad = CPL_MATH_STD_MAD * mad;


  return CPL_ERROR_NONE;
}


/** @cond PRIVATE */


/** @endcond */


/**@}*/

