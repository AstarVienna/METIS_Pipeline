/*
 * This file is part of the HDRL
 * Copyright (C) 2017 European Southern Observatory
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
#include "hdrl_DER_SNR.h"
#include "hdrl_image.h"
#include "hdrl_utils.h"

#include <math.h>
#include <cpl.h>

/* returns TRUE iff lambdas(i) < lambdas(i + 1) for every i */
static inline cpl_boolean is_strictly_monotonic(const cpl_array * lambdas);

static inline cpl_boolean
should_skip(const cpl_binary * msk, cpl_size i1, cpl_size i2, cpl_size i3);

/* DER_SNR estimation if the wavelengths are strictly monotonically increasing*/
static inline cpl_image *
estimate_noise_DER_SNR_on_sorted(const hdrl_data_t * flux,
        const cpl_binary * msk_in, const cpl_size length,
        const cpl_size half_window);

/* insert the arrays in the table and sorty according to wavelength. The
 * table is returned*/
static inline
cpl_table * conv_to_sorted_table(const hdrl_data_t * flux_in,
        const cpl_binary * msk_in, const cpl_array * wavelengths,
        const cpl_size length);

/* DER_SNR estimation if the wavelengths are NOT strictly monotonically
 * increasing*/
static inline cpl_image *
estimate_noise_DER_SNR_on_unsorted(const hdrl_data_t * flux_in,
        const cpl_binary * msk_in, const cpl_array * wavelengths,
        const cpl_size length, const cpl_size half_window);

/**
 * @addtogroup hdrl_spectrum1D
 * @{
 */
/*-----------------------------------------------------------------------------
                                   Functions
 -----------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief Estimate the noise in the pixels between [start, stop]. The noise
 * calculation is done using the formula from: Stoehr, F. et al.
 * DER SNR: A Simple & General Spectroscopic Signal-to-Noise Measurement Algorithm
 * @param flux      Input Flux
 * @param msk       Bad Pixel Mask
 * @param start     First Pixel
 * @param stop      Last Pixel
 * @param sz        Length of the flux (must be the same for msk)
 * @return estimated noise for the given window, NAN is returned in case of
 * error or if there are not enough non bad pixels to execute the calculation.
 *
 * Possible cpl-error-code set in this function (which also implies
 * that NAN is returned):
 * - CPL_ERROR_NULL_INPUT: if flux or msk are NULL
 * - CPL_ERROR_INCOMPATIBLE_INPUT: if start, stop and size are either not
 *   compatible with each other or if the resulting window is too small to
 *   calculate the noise (there must be at least 4 pixels between start and stop).
 */
/* ---------------------------------------------------------------------------*/
hdrl_error_t estimate_noise_window(const hdrl_data_t * flux,
        const cpl_binary * msk, cpl_size start, cpl_size stop,
        const cpl_size sz){

    const hdrl_data_t factor = CPL_MATH_STD_MAD / sqrt(6.0);

    cpl_ensure(flux != NULL, CPL_ERROR_NULL_INPUT, NAN);

    cpl_ensure(start >= 0, CPL_ERROR_INCOMPATIBLE_INPUT, NAN);
    cpl_ensure(stop > start, CPL_ERROR_INCOMPATIBLE_INPUT, NAN);


    cpl_ensure(stop < sz, CPL_ERROR_INCOMPATIBLE_INPUT, NAN);

    start += 2;
    stop -= 2;

    const cpl_size max_elems = stop - start + 1;

    cpl_ensure(max_elems > 0 , CPL_ERROR_INCOMPATIBLE_INPUT, NAN);

    cpl_array * data = cpl_array_new(max_elems, HDRL_TYPE_ERROR);
    cpl_array_fill_window_invalid(data, 0, max_elems - 1);

    for(cpl_size i = start; i <= stop; ++i){

        const cpl_size i_pre = i - 2;
        const cpl_size i_post = i + 2;

        if(should_skip(msk, i, i_pre, i_post)) continue;

        const hdrl_data_t curr = flux[i];
        const hdrl_data_t pre = flux[i_pre];
        const hdrl_data_t next = flux[i_post];

        const hdrl_error_t noise =
                (hdrl_error_t)fabs(factor * (2.0 * curr - pre - next));
        cpl_array_set(data, i - start, noise);
    }

    hdrl_error_t median = NAN;

    /* If no pixels were available for DER_SNR calculation, return NAN */
    if(cpl_array_count_invalid(data) < max_elems)
        median = cpl_array_get_median(data);

    cpl_array_delete(data);

    return median;

}

/* ---------------------------------------------------------------------------*/
/**
 * @brief For every pixel in position i in img_arg, the function estimates the
 * noise using the pixels in the window [i - half_window, i + half_window].
 * For details on the calculation inside the window, see estimate_noise_window()
 * @param flux_in       input flux
 * @param msk_in        bad pixels mask
 * @param wavelengths   wavelengths of the spectrum
 * @param length        length of the flux and msl
 * @param half_window   half window used to calculate the noise for each pixel.
 * @return the estimate standard deviation for each pixel or NULL in case of error.
 *
 * Possible cpl-error-code set in this function (which also implies
 * that NULL is returned):
 * - CPL_ERROR_NULL_INPUT: if any among flux_in, msk_in or wavelengths are NULL
 * - CPL_ERROR_INCOMPATIBLE_INPUT: if half_window < 2 or length > 4
 */
/* ---------------------------------------------------------------------------*/
cpl_image *
estimate_noise_DER_SNR(const hdrl_data_t * flux_in, const cpl_binary * msk_in,
        const cpl_array * wavelengths, const cpl_size length,
        const cpl_size half_window){

    cpl_ensure(half_window >= 2, CPL_ERROR_INCOMPATIBLE_INPUT, NULL);
    cpl_ensure(flux_in != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(wavelengths != NULL, CPL_ERROR_NULL_INPUT, NULL);

    cpl_ensure(length > 4, CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    /* simple case */
    if(is_strictly_monotonic(wavelengths))
        return estimate_noise_DER_SNR_on_sorted
                (flux_in, msk_in, length, half_window);

    /* complex case, we need to sort copies of the data calculate DER_SNR and
     * put the correct noise sample in the correct spot, following the positioning
     * provided as input*/
    return estimate_noise_DER_SNR_on_unsorted(flux_in, msk_in, wavelengths,
            length, half_window);
}

/*-----------------------------------------------------------------------------
                           Private Functions
 -----------------------------------------------------------------------------*/

static inline cpl_image *
estimate_noise_DER_SNR_on_sorted(const hdrl_data_t * flux,
        const cpl_binary * msk_in, const cpl_size length,
        const cpl_size half_window){

    cpl_image * to_ret = cpl_image_new(length, 1, HDRL_TYPE_ERROR);
    cpl_mask * msk = cpl_mask_new(length, 1);

    for(cpl_size i = 0; i < length; ++i){

        cpl_boolean rej = msk_in != NULL && msk_in[i];

        double d = NAN;
        /* skip if bad pixel */
        if(!rej)
        {
            const cpl_size start = CPL_MAX(0, i - half_window);
            const cpl_size stop = CPL_MIN(length - 1, i + half_window);
            d = estimate_noise_window(flux, msk_in, start, stop, length);
        }

        cpl_image_set(to_ret, i + 1, 1, d);

        if(isnan(d)){
            cpl_mask_set(msk, i + 1, 1, CPL_BINARY_1);
        }
    }

    cpl_mask_delete(cpl_image_set_bpm(to_ret, msk));
    return to_ret;
}

static inline
cpl_table * conv_to_sorted_table(const hdrl_data_t * flux_in,
        const cpl_binary * msk_in, const cpl_array * wavelengths,
        const cpl_size length){

    cpl_table * tb = cpl_table_new(length);
    int * map = cpl_calloc(length, sizeof(int));
    int * pmask = cpl_calloc(length, sizeof(int));
    hdrl_data_t * flux = cpl_calloc(length, sizeof(hdrl_data_t));
    double * pwlen = cpl_calloc(length, sizeof(double));

    for(cpl_size i = 0; i < length; ++i){
     map[i] = i;
     pwlen[i] = cpl_array_get(wavelengths, i, NULL);
     pmask[i] = msk_in == NULL ? 0 : (int)msk_in[i];
     flux[i] = flux_in[i];
    }

    cpl_table_wrap_int(tb, map, "map");
    cpl_table_wrap_int(tb, pmask, "bad_pixel_mask");
    cpl_table_wrap_double(tb, pwlen, "lambda");
    hdrl_wrap_table(tb, flux, "flux");

    cpl_propertylist * ls = cpl_propertylist_new();
    cpl_propertylist_append_bool(ls, "lambda", CPL_FALSE);
    cpl_table_sort(tb, ls);
    cpl_propertylist_delete(ls);
    return tb;
}

static inline cpl_image *
estimate_noise_DER_SNR_on_unsorted(const hdrl_data_t * flux_in,
        const cpl_binary * msk_in, const cpl_array * wavelengths,
        const cpl_size length, const cpl_size half_window){

    cpl_binary * msk_sorted = cpl_calloc(length, sizeof(cpl_binary));

    cpl_table * tb = conv_to_sorted_table(flux_in, msk_in, wavelengths, length);

    /* extract columns */
    int * map = (int*)cpl_table_unwrap(tb, "map");
    hdrl_data_t * flux = (hdrl_data_t*)cpl_table_unwrap(tb, "flux");
    int * pmask = (int*)cpl_table_unwrap(tb, "bad_pixel_mask");

    cpl_table_delete(tb);

    /* convert to a cpl_binary array the sorted masks */
    for(cpl_size i = 0; i < length; ++i){
      msk_sorted[i] = pmask[i];
    }
    cpl_free(pmask);

    cpl_image * img_sorted = estimate_noise_DER_SNR_on_sorted
          (flux, msk_sorted, length, half_window);

    cpl_free(flux);
    cpl_free(msk_sorted);

    cpl_image * to_ret = cpl_image_new(length, 1, HDRL_TYPE_DATA);

    for(cpl_size i = 0; i < length; i++){
      const cpl_size dest_idx = map[i] + 1;
      const cpl_size source_idx = i + 1;

      int rej;
      double data = cpl_image_get(img_sorted, source_idx, 1, &rej);
      if(rej){
          cpl_image_reject(to_ret, dest_idx, 1);
      }
      else{
          cpl_image_set(to_ret, dest_idx, 1, data);
      }
    }

    cpl_free(map);
    cpl_image_delete(img_sorted);
    return to_ret;
}

static inline cpl_boolean is_strictly_monotonic(const cpl_array * lambdas){
    for(cpl_size i = 0; i < cpl_array_get_size(lambdas) - 1; ++i){
        if(cpl_array_get(lambdas, i, NULL) >= cpl_array_get(lambdas, i + 1, NULL)){
            return CPL_FALSE;
        }
    }
    return CPL_TRUE;
}

static inline cpl_boolean
should_skip(const cpl_binary * msk, cpl_size i1, cpl_size i2, cpl_size i3){

    /*no mask means all the pixel are good*/
    if(!msk) return CPL_FALSE;

    return msk[i1] || msk[i2] || msk[i3];
}

/**@}*/
