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
#include "hdrl_correlation.h"

#include <math.h>


/*-----------------------------------------------------------------------------
                        Data structures used internally
 -----------------------------------------------------------------------------*/
typedef struct{
    double mean;
    double stdev;
}mean_and_stdev;

/*-----------------------------------------------------------------------------
                               Private Functions
 -----------------------------------------------------------------------------*/

static inline
double calculate_xcorr_sample(const cpl_size shift, const cpl_array * arr1,
        const cpl_array * arr2, const double mean1, const double mean2,
        const double stdev1, const double stdev2);

static inline
mean_and_stdev calculate_mean_and_stdev(const cpl_array * arr1);

static inline cpl_error_code
hdrl_compute_xcorrelation_refine(hdrl_xcorrelation_result* xcorr_res,
        const double bin, const double wrange);

/**
 * @addtogroup hdrl_correlation
 * @{
 */
/*-----------------------------------------------------------------------------
                                   Functions
 -----------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief Constructor for hdrl_xcorrelation_result
 * @param x_corr      Cross correlation. x_corr becomes owned by the returned value,
 *                    do not free x_corr after the wrapping.
 * @param max_idx     Index where the cross correlation reaches its maximum
 * @param half_window Half window used for the cross-correlation calculation
 *
 * @return the constructed object. NULL in case of error. Errors are triggered if
 * data are not self consistent, e.g. if max_idx is greated than the length of
 * x_corr.
 */
/* ---------------------------------------------------------------------------*/

hdrl_xcorrelation_result *
hdrl_xcorrelation_result_wrap(cpl_array * x_corr,  const cpl_size max_idx,
        const cpl_size half_window){

    cpl_ensure(x_corr != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(max_idx >= 0, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(max_idx < cpl_array_get_size(x_corr),
            CPL_ERROR_ILLEGAL_INPUT, NULL);

    hdrl_xcorrelation_result *  to_ret = cpl_calloc(1, sizeof(*to_ret));

    to_ret->xcorr = x_corr;
    to_ret->pix_peakpos = max_idx;
    to_ret->half_window = half_window;
    return to_ret;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Destructor for hdrl_xcorrelation_result
 * @param self      hdrl_xcorrelation_result to be deleted
 *
 * @return nothing
 */
/* ---------------------------------------------------------------------------*/
void hdrl_xcorrelation_result_delete(hdrl_xcorrelation_result * self){
    if(self == NULL) return;

    cpl_array_delete(self->xcorr);
    cpl_free(self);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Get the index where the cross correlation reaches its maximum
 * @param self          hdrl_xcorrelation_result the getter will extract the
 *                      data from
 *
 * @return the index where the cross correlation reaches its maximum
 */
/* ---------------------------------------------------------------------------*/
cpl_size
hdrl_xcorrelation_result_get_peak_pixel(const hdrl_xcorrelation_result * self){

    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, 0);

    return self->pix_peakpos;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Get the index where the cross correlation reaches its maximum, with
 * sub-pixel precision
 * @param self          hdrl_xcorrelation_result the getter will extract the
 *                      data from
 *
 * @return the index where the cross correlation reaches its maximum, with
 * sub-pixel precision
 */
/* ---------------------------------------------------------------------------*/
double  hdrl_xcorrelation_result_get_peak_subpixel
                                        (const hdrl_xcorrelation_result * self){

    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, 0);

    return self->peakpos;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Get the half_window used to calculate the cross-correlation.
 * @param self          hdrl_xcorrelation_result the getter will extract the
 *                      data from
 *
 * @return the half_window used to calculate the cross-correlation.
 */
/* ---------------------------------------------------------------------------*/
cpl_size hdrl_xcorrelation_result_get_half_window
                                        (const hdrl_xcorrelation_result * self){

    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, 0);

    return self->half_window;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Get the estimated standard deviation of the correlation
 * @param self          hdrl_xcorrelation_result the getter will extract the
 *                      data from
 *
 * @return the estimated standard deviation of the correlation
 */
/* ---------------------------------------------------------------------------*/
double
hdrl_xcorrelation_result_get_sigma(const hdrl_xcorrelation_result * self){

    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, 0);

    return self->sigma;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Getter for the cross correlation
 * @param self          hdrl_xcorrelation_result the getter will extract the
 *                      data from
 *
 * @return the cross correlation
 */
/* ---------------------------------------------------------------------------*/
const cpl_array *
hdrl_xcorrelation_result_get_correlation(const hdrl_xcorrelation_result * self){

    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, NULL);

    return self->xcorr;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Calculate cross-correlation
 * @param arr1        First array
 * @param arr2        Second array
 * @param half_window half search window where the correlation is calculated
 * @param normalize   CPL_TRUE normalize correlation in mean and rms
 *
 * @return cross correlation and index where the peak is. NULL in case of error.
 *
 * @note: elements marked as invalid in arr1 or arr2 will be treated as they were
 * out-of-boudary pixels.
 */
/* ---------------------------------------------------------------------------*/
hdrl_xcorrelation_result * hdrl_compute_xcorrelation(
                    const cpl_array * arr1, const cpl_array * arr2,
                    const cpl_size half_window, const cpl_boolean normalize){

    cpl_ensure(half_window > 1, CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    const mean_and_stdev not_normalized = {0.0, 1.0};

    cpl_ensure(arr1 != NULL && arr2 != NULL, CPL_ERROR_NULL_INPUT, NULL);

    cpl_size xcorr_length = 2 * half_window + 1;
    cpl_array * corr = cpl_array_new(xcorr_length, HDRL_TYPE_DATA);

    const mean_and_stdev d1 = normalize ?
            calculate_mean_and_stdev(arr1) : not_normalized;
    const mean_and_stdev d2 = normalize ?
            calculate_mean_and_stdev(arr2) : not_normalized;

    cpl_size max_idx = -1;
    double max_corr = 0.0;

    for(cpl_size i = -half_window; i <= half_window; ++i){
        const double cr = calculate_xcorr_sample(i, arr1, arr2, d1.mean, d2.mean,
                d1.stdev, d2.stdev);

        const cpl_size idx = i + half_window;
        cpl_array_set(corr, idx, cr);

        if(isnan(cr)) continue;

        if(cr >= max_corr || max_idx < 0){
            max_corr = cr;
            max_idx = idx;
        }
    }

    return hdrl_xcorrelation_result_wrap(corr, max_idx, half_window);
}

static inline
cpl_error_code check_if_bad(const hdrl_xcorrelation_result * gfit,
        const cpl_boolean check_refine){

    cpl_ensure_code(gfit != NULL, CPL_ERROR_ILLEGAL_OUTPUT);
    cpl_ensure_code(hdrl_xcorrelation_result_get_peak_pixel(gfit) >=0,
            CPL_ERROR_ILLEGAL_OUTPUT);

    if(check_refine){

        const double px = hdrl_xcorrelation_result_get_peak_subpixel(gfit);
        cpl_ensure_code(px >= 0.0 && !isnan(px), CPL_ERROR_ILLEGAL_OUTPUT);

        const double sigma = hdrl_xcorrelation_result_get_sigma(gfit);
        cpl_ensure_code( sigma > 0.0 && !isnan(sigma),CPL_ERROR_ILLEGAL_OUTPUT);
    }

    return CPL_ERROR_NONE;
}

static inline
cpl_boolean check_and_delete_if_bad(hdrl_xcorrelation_result ** gfit_arg,
        const cpl_boolean check_refine){

    cpl_error_code fail = check_if_bad(*gfit_arg, check_refine);

    if(fail){
        hdrl_xcorrelation_result_delete(*gfit_arg);
        *gfit_arg = NULL;
    }

    return fail ? CPL_FALSE : CPL_TRUE;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Calculate gaussian fit on cross-correlation, does a second fitting for
 * refinement.
 * @param arr1        First array
 * @param arr2        Second array
 * @param half_win    half search window where the correlation is calculated
 * @param normalize   CPL_TRUE normalize correlation in mean and rms
 * @param bin         wavelength bin
 * @param wrange      half window wavelength range where the fit is going to be
 *                    done
 *
 * @return gaussian fit for cross correlation.
 */
/* ---------------------------------------------------------------------------*/
hdrl_xcorrelation_result * hdrl_compute_offset_gaussian(
        const cpl_array * arr1,
        const cpl_array * arr2,
        const cpl_size  half_win, const cpl_boolean normalize,
        const double bin, const double wrange){

    cpl_ensure(half_win > 1, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(arr1 != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(arr2 != NULL, CPL_ERROR_NULL_INPUT, NULL);

    hdrl_xcorrelation_result * gfit =
            hdrl_compute_offset_gaussian_internal(arr1, arr2, half_win,
                    normalize, bin, wrange);

    cpl_ensure(gfit != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

    const cpl_size half_win2 =
            (cpl_size)(3. * CPL_MATH_FWHM_SIG * gfit->sigma / bin);

    hdrl_xcorrelation_result_delete(gfit);

    gfit = hdrl_compute_offset_gaussian_internal(arr1, arr2, half_win2,
                    normalize, bin, wrange);

    return gfit;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Calculate gaussian fit on cross-correlation
 * @param arr1        First array
 * @param arr2        Second array
 * @param half_win    half search window where the correlation is calculated
 * @param normalize   CPL_TRUE normalize correlation in mean and rms
 * @param bin         wavelength bin
 * @param wrange      half window wavelength range where the fit is going to be
 *                    done
 *
 * @return gaussian fit for cross correlation.
 */
/* ---------------------------------------------------------------------------*/
hdrl_xcorrelation_result * hdrl_compute_offset_gaussian_internal(
        const cpl_array * arr1, const cpl_array * arr2,
        const cpl_size half_win, const cpl_boolean normalize,
        const double bin, const double wrange){

    hdrl_xcorrelation_result * res =
            hdrl_compute_xcorrelation(arr1, arr2, half_win, normalize);

    if(!check_and_delete_if_bad(&res, CPL_FALSE)) return  NULL;

    cpl_error_code fail =
            hdrl_compute_xcorrelation_refine(res, bin, wrange);

    if(fail){
        hdrl_xcorrelation_result_delete(res);
        return NULL;
    }

    if(!check_and_delete_if_bad(&res, CPL_TRUE)) return  NULL;

    return  res;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Refine a hdrl_xcorrelation_result using gaussian fit
 * @param xcorr_res   hdrl_xcorrelation_result calculated with pixel precision
 * @param bin         wavelength bin
 * @param wrange      half window wavelength range where the fit is going to be
 *                    done
 *
 * @return gaussian fit for cross correlation.
 */
/* ---------------------------------------------------------------------------*/
static inline cpl_error_code
hdrl_compute_xcorrelation_refine(hdrl_xcorrelation_result* xcorr_res,
        const double bin, const double wrange){

       const cpl_array * xcorr = hdrl_xcorrelation_result_get_correlation(xcorr_res);
       const cpl_size maxpos = hdrl_xcorrelation_result_get_peak_pixel(xcorr_res);

       const cpl_size xcorr_size = cpl_array_get_size(xcorr);
       const cpl_size pre_idx = CPL_MAX(0, maxpos - 1);
       const cpl_size post_idx =  CPL_MIN(maxpos + 1, xcorr_size - 1);

       const double a = cpl_array_get(xcorr, pre_idx, NULL);
       const double b = cpl_array_get(xcorr, post_idx, NULL);
       const double c = cpl_array_get(xcorr, maxpos, NULL);
       /* Find sub-pixel peak initial estimation. Use parabolic assumption */
       const double fraction = (b - a) / ( 4. * c - 2. * a - 2. * b );
       const double subpix_offset = maxpos - fraction;

       xcorr_res->peakpos = subpix_offset * bin;
       xcorr_res->sigma = bin * 10;
       xcorr_res->area = 1.0;

       cpl_size num_elems = 0;

       cpl_vector * wavs_windowed = cpl_vector_new(xcorr_size);
       cpl_vector * corr_windowed = cpl_vector_new(xcorr_size);

       for(cpl_size i = 0; i < xcorr_size; ++i){
           const double w = i * bin;
           int rej = 0;
           const double xcorr_data = cpl_array_get(xcorr, i, &rej);

           if(rej || isnan(xcorr_data)) continue;
           if(w <  xcorr_res->peakpos - wrange
                   || w > xcorr_res->peakpos + wrange) continue;

           cpl_vector_set(corr_windowed, num_elems, xcorr_data);
           cpl_vector_set(wavs_windowed, num_elems, w);

           num_elems++;
       }

       if(num_elems <= 0){
           cpl_vector_delete(wavs_windowed);
           cpl_vector_delete(corr_windowed);
           cpl_ensure_code(CPL_FALSE, CPL_ERROR_ILLEGAL_OUTPUT);
       }

       cpl_vector_set_size(corr_windowed, num_elems);
       cpl_vector_set_size(wavs_windowed, num_elems);

       cpl_error_code code = cpl_vector_fit_gaussian(wavs_windowed, NULL,
               corr_windowed, NULL, CPL_FIT_ALL, &xcorr_res->peakpos, &xcorr_res->sigma,
               &xcorr_res->area, &xcorr_res->offset, &xcorr_res->mse, NULL, NULL);

       /* if the fitting does not converge, CPL_ERROR_CONTINUE is set, the
        * output parameters are calculated using a best-effort approach.
        * See the documentation of cpl_vector_fit_gaussian .*/
       if(code == CPL_ERROR_CONTINUE){
           cpl_error_reset();
       }

       cpl_vector_delete(wavs_windowed);
       cpl_vector_delete(corr_windowed);

       return cpl_error_get_code();
}

static inline
double calculate_xcorr_sample(const cpl_size shift,
        const cpl_array * arr1, const cpl_array * arr2,
        const double mean1, const double mean2,
        const double stdev1, const double stdev2){

    double d = 0.0;
    cpl_size num_el = 0;

    const double norm = 1.00 / sqrt(stdev1 * stdev2);

    const cpl_size l1 = cpl_array_get_size(arr1);
    const cpl_size l2 = cpl_array_get_size(arr2);

    for(cpl_size i = 0; i < l2; i++){
        const cpl_size j = i + shift;
        int rej1 = 0, rej2 = 0;
        if(j < 0) continue;
        if(j >= l1) continue;

        const double v1 = cpl_array_get(arr1, j, &rej1);
        const double v2 = cpl_array_get(arr2, i, &rej2);

        if(rej1 || rej2) continue;

        const double val = norm * (v1 - mean1) * (v2 - mean2);

        d += val;
        num_el++;
    }
    return d / (double)num_el;
}

static inline
mean_and_stdev calculate_mean_and_stdev(const cpl_array * arr1){

    const double mean = cpl_array_get_mean(arr1);
    const double stdev = cpl_array_get_stdev(arr1);

    return (mean_and_stdev){mean, stdev};
}
/**@}*/
