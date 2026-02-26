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

#include "hdrl_sigclip.h"
#include "hdrl_utils.h"
#include "hdrl_collapse.h"

#include <cpl.h>
#include <string.h>
#include <math.h>


/*-----------------------------------------------------------------------------
                                   Static
 -----------------------------------------------------------------------------*/

static cpl_error_code hdrl_sort_double_pairs(cpl_vector *, cpl_vector *) ;
static long get_lower_bound_d(double * vec, long count, double val);
static long get_upper_bound_d(double * vec, long count, double val);
static long get_lower_bound(cpl_vector * vec, double val);
static long get_upper_bound(cpl_vector * vec, double val);

/** @cond PRIVATE */

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_sigclip   Clipping module
 
  This module provides parameters for iterative \f$\kappa-\sigma\f$ clipping and
  minmax rejection.
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
  @brief Create parameters for the sigma-clip collapse
  @param base_context    base context of parameter (e.g. recipe name)
  @param prefix          prefix of parameter, may be empty string
  @param defaults        default sigclip parameters
  @return The created parameter list
  Creates a parameterlist containing
      base_context.prefix.kappa-low
      base_context.prefix.kappa-high
      base_context.prefix.niter
 */
/* ---------------------------------------------------------------------------*/
cpl_parameterlist * hdrl_sigclip_parameter_create_parlist(
        const char           *base_context,
        const char           *prefix,
        const hdrl_parameter *defaults)
{
	cpl_ensure(base_context && prefix && defaults,
			CPL_ERROR_NULL_INPUT, NULL);

    cpl_ensure(hdrl_collapse_parameter_is_sigclip(defaults),
    		CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    cpl_parameterlist *parlist = cpl_parameterlist_new();

    /* --prefix.kappa-low */
    hdrl_setup_vparameter(parlist, prefix, ".", "",
            "kappa-low", base_context,
            "Low kappa factor for kappa-sigma clipping algorithm",
            CPL_TYPE_DOUBLE,
            hdrl_collapse_sigclip_parameter_get_kappa_low(defaults));
    
     /* --prefix.kappa-high */
    hdrl_setup_vparameter(parlist, prefix, ".", "",
            "kappa-high", base_context,
            "High kappa factor for kappa-sigma clipping algorithm",
            CPL_TYPE_DOUBLE,
            hdrl_collapse_sigclip_parameter_get_kappa_high(defaults));
 
    /* --prefix.niter */
    hdrl_setup_vparameter(parlist, prefix, ".", "",
            "niter", base_context,
            "Maximum number of clipping iterations for kappa-sigma clipping",
            CPL_TYPE_INT,
            hdrl_collapse_sigclip_parameter_get_niter(defaults));
    
    if (cpl_error_get_code()) {
        cpl_parameterlist_delete(parlist);
        return NULL;
    }

    return parlist;
}
/* ---------------------------------------------------------------------------*/
/**
  @brief Create parameters for the minmax-clip collapse
  @param base_context    base context of parameter (e.g. recipe name)
  @param prefix          prefix of parameter, may be empty string
  @param defaults        default minmax parameters
  @return The created parameter list
  Creates a parameterlist containing
      base_context.prefix.nlow
      base_context.prefix.nhigh
 */
/* ---------------------------------------------------------------------------*/
cpl_parameterlist * hdrl_minmax_parameter_create_parlist(
        const char           *base_context,
        const char           *prefix,
        const hdrl_parameter *defaults)
{
    cpl_ensure(base_context && prefix && defaults,
    		CPL_ERROR_NULL_INPUT, NULL);

    cpl_ensure(hdrl_collapse_parameter_is_minmax(defaults),
    		CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    cpl_parameterlist * parlist = cpl_parameterlist_new();

    /* --prefix.nlow */
    hdrl_setup_vparameter(parlist, prefix, ".", "",
            "nlow", base_context,
            "Low number of pixels to reject for the minmax clipping algorithm",
            CPL_TYPE_DOUBLE,
            hdrl_collapse_minmax_parameter_get_nlow(defaults));
    
     /* --prefix.nhigh */
    hdrl_setup_vparameter(parlist, prefix, ".", "",
            "nhigh", base_context,
            "High number of pixels to reject for the minmax clipping algorithm",
            CPL_TYPE_DOUBLE,
            hdrl_collapse_minmax_parameter_get_nhigh(defaults));
     
    if (cpl_error_get_code()) {
        cpl_parameterlist_delete(parlist);
        return NULL;
    }

    return parlist;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief parse parameterlist for sigclip parameters to init corresponding hdrl
 * structure parameters
 *
 * @param parlist    parameter list to parse
 * @param prefix     prefix of parameter name
 * @param kappa_low  pointer to storage to save kappa_low or NULL
 * @param kappa_high pointer to storage to save kappa_high or NULL
 * @param niter      pointer to storage to save niter or NULL
 * @see   hdrl_kappa_sigma_clip_get_parlist()
 * @return cpl_error_code
 *
 * parameterlist should have been created with
 * hdrl_kappa_sigma_clip_get_parlist or have the same name hierachy
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_sigclip_parameter_parse_parlist(
        const cpl_parameterlist *   parlist,
        const char              *   prefix,
        double                  *   kappa_low,
        double                  *   kappa_high,
        int                     *   niter)
{
    cpl_ensure_code(prefix && parlist, CPL_ERROR_NULL_INPUT);
    char * name;

    if (kappa_low) {
        name = hdrl_join_string(".", 2, prefix, "sigclip.kappa-low");
        const cpl_parameter * par = cpl_parameterlist_find_const(parlist, name);
        *kappa_low = cpl_parameter_get_double(par);
        cpl_free(name);
    }

    if (kappa_high) {
        name = hdrl_join_string(".", 2, prefix, "sigclip.kappa-high");
        const cpl_parameter * par = cpl_parameterlist_find_const(parlist, name);
        *kappa_high = cpl_parameter_get_double(par);
        cpl_free(name);
    }

    if (niter) {
        name = hdrl_join_string(".", 2, prefix, "sigclip.niter");
        const cpl_parameter * par = cpl_parameterlist_find_const(parlist, name);
        *niter = cpl_parameter_get_int(par);
        cpl_free(name);
    }

    if (cpl_error_get_code()) {
        return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                                     "Error while parsing parameterlist "
                                     "with prefix %s", prefix);
    }

    return CPL_ERROR_NONE;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief parse parameterlist for minmax parameters to init corresponding hdrl
 * structure parameters
 *
 * @param parlist    parameter list to parse
 * @param prefix     prefix of parameter name
 * @param nlow       pointer to storage to save nlow or NULL
 * @param nhigh      pointer to storage to save nhigh or NULL
 * @see   hdrl_minmax_clip_get_parlist()
 * @return cpl_error_code
 *
 * parameterlist should have been created with
 * hdrl_minmax_clip_get_parlist or have the same name hierachy
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_minmax_parameter_parse_parlist(
        const cpl_parameterlist *   parlist,
        const char              *   prefix,
        double                  *   nlow,
        double                  *   nhigh)
{
    cpl_ensure_code(prefix && parlist, CPL_ERROR_NULL_INPUT);
    char * name;

    if (nlow) {
        name = hdrl_join_string(".", 2, prefix, "minmax.nlow");
        const cpl_parameter * par = cpl_parameterlist_find_const(parlist, name);
        *nlow = cpl_parameter_get_double(par);
        cpl_free(name);
    }

    if (nhigh) {
        name = hdrl_join_string(".", 2, prefix, "minmax.nhigh");
        const cpl_parameter * par = cpl_parameterlist_find_const(parlist, name);
        *nhigh = cpl_parameter_get_double(par);
        cpl_free(name);
    }

    if (cpl_error_get_code()) {
        return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                                     "Error while parsing parameterlist "
                                     "with prefix %s", prefix);
    }

    return CPL_ERROR_NONE;
}

 
/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Compute mean image value using min-max rejection method
  @param source         Input image
  @param error          Input error image
  @param nlow         Absolute number of low pixels to reject
  @param nhigh         Absolute number of high pixels to reject
  @param mean_mm        The min-max clipped mean
  @param mean_mm_err    The propagated error of the min-max clipped mean
  @param naccepted      Number of accepted values
  @return   @c CPL_ERROR_NONE or the appropriate error code.
  @see hdrl_minmax_clip()

  This function converts the image inputs into the proper data types in
  order to call the hdrl_minmax_clip() function.
  If the error values at the rejection boundaries are ambigous, e.g. when you
  have multiple pixels with the same value but different error and the
  rejection boundary would only select a subset of these, the algorithm assigns
  the smallest error values of the equal value range to the selected pixels.
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_minmax_clip_image(
        const cpl_image   * source,
        const cpl_image   * error,
        const double        nlow,
        const double        nhigh,
        double            * mean_mm,
        double            * mean_mm_err,
        cpl_size          * naccepted,
        double            * reject_low,
        double            * reject_high)
{
    cpl_vector * vec_source = NULL;
    cpl_vector * vec_error = NULL;

    /* Check Entries */
    cpl_error_ensure(source != NULL, CPL_ERROR_NULL_INPUT,
            return CPL_ERROR_NULL_INPUT, "Null input source image!");
    cpl_error_ensure(error != NULL, CPL_ERROR_NULL_INPUT,
            return CPL_ERROR_NULL_INPUT, "Null input error image!");
    cpl_error_ensure(cpl_image_get_size_x(source)==cpl_image_get_size_x(error),
            CPL_ERROR_INCOMPATIBLE_INPUT, return CPL_ERROR_INCOMPATIBLE_INPUT,
            "source and error image musty have same X size");
    cpl_error_ensure(cpl_image_get_size_y(source)==cpl_image_get_size_y(error),
            CPL_ERROR_INCOMPATIBLE_INPUT, return CPL_ERROR_INCOMPATIBLE_INPUT,
            "source and error image musty have same Y size");

    /* compress images to vectors excluding the bad pixels */
    vec_source = hdrl_image_to_vector(source, NULL);
    vec_error = hdrl_image_to_vector(error, cpl_image_get_bpm_const(source));

    if (vec_source != NULL && vec_error != NULL) {
        /* Call here the real clipping function */
        hdrl_minmax_clip(vec_source, vec_error, nlow, nhigh, CPL_TRUE, mean_mm,
                         mean_mm_err, naccepted, reject_low, reject_high);
    }
    /* no good pixels */
    else {
        *mean_mm = NAN;
        *mean_mm_err = NAN;
        *naccepted = 0;
        *reject_low = NAN;
        *reject_high = NAN;
    }

    cpl_msg_debug(cpl_func, "mean_mm, mean_mm_err, naccepted:  %g, %g, %ld",
                  *mean_mm, *mean_mm_err, (long)*naccepted);

    cpl_vector_delete(vec_source);
    cpl_vector_delete(vec_error);
    return cpl_error_get_code();
}


/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief   Compute mean using min-max clipping.
  @param   vec         The vector for which the clipped mean is computed.
  @param   vec_err     The error of vec.
  @param   nlow        The number of low pixels to be rejected by the algorithm
  @param   nhigh       The number of high pixels to be rejected by the algorithm
  @param   inplace     If true the input vectors are modified
  @param   mean_mm     The min-max clipped mean.
  @param   mean_mm_err The propagated error of the min-max clipped mean.
  @param   naccepted   Number of accepted values.
  @param   reject_low  The lowest value that is rejected
  @param   reject_high The highest value that is rejected
  @return   @c CPL_ERROR_NONE or the appropriate error code.

  This function computes the mean after sorting the elements and rejecting nlow
  and nhigh values. The remaining pixels are then used to compute the mean and
  the associated error. Please note that, if multiple equal elements are
  present, the error propagation uses the value with the smallest error.


 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_minmax_clip(
                cpl_vector        * vec,
                cpl_vector        * vec_err,
                const double        nlow,
                const double        nhigh,
                cpl_boolean         inplace,
                double            * mean_mm,
                double            * mean_mm_err,
                cpl_size          * naccepted,
                double            * reject_low,
                double            * reject_high)
{
    /*    VARIABLES ON THE FUNCTION SCOPE:

          vec_image       a deep copy of the input vector vec.
          mean_mm         min-max clip mean (return variable).
     */

    cpl_vector   * vec_image = NULL;
    cpl_vector   * vec_image_err = NULL;

    cpl_size       vec_size;
    cpl_size nlow_int, nhigh_int;
    cpl_vector * vec_trunc;

    cpl_size trunc_size;
    double * d, * e;

    /*In the future minmax rejection could also use relative values therefore
     * we pass a double to the function - nevertheless the code as it is now
     * expects an integer - thus we need to do the  rounding */

    nlow_int = (cpl_size)round(nlow);
    nhigh_int = (cpl_size)round(nhigh);

    cpl_error_ensure(vec != NULL, CPL_ERROR_NULL_INPUT,
                     return CPL_ERROR_NULL_INPUT, "Null input vector data");
    cpl_error_ensure(vec_err != NULL, CPL_ERROR_NULL_INPUT,
                     return CPL_ERROR_NULL_INPUT, "Null input vector errors");
    cpl_error_ensure(cpl_vector_get_size(vec) == cpl_vector_get_size(vec_err),
                     CPL_ERROR_INCOMPATIBLE_INPUT,
                     return CPL_ERROR_INCOMPATIBLE_INPUT,
                                     "input data and error vectors must have same sizes");
    cpl_error_ensure(mean_mm != NULL, CPL_ERROR_NULL_INPUT,
                     return CPL_ERROR_NULL_INPUT, "Null input mean storage");

    cpl_error_ensure(nlow_int >=0 && nhigh_int>=0, CPL_ERROR_INCOMPATIBLE_INPUT,
                     return CPL_ERROR_INCOMPATIBLE_INPUT, "nlow and nhigh must "
                                     "be strictly positive");

    vec_size = cpl_vector_get_size(vec);

    /* Nothing to do if only one data point */
    if(vec_size <= (nlow_int+nhigh_int)) {
        *mean_mm=NAN;
        *mean_mm_err=NAN;
        *naccepted=0;
        return cpl_error_get_code();
    }

    if (inplace) {
        vec_image = vec;
        vec_image_err = vec_err;
    }
    else {
        vec_image = cpl_vector_duplicate(vec);
        vec_image_err = cpl_vector_duplicate(vec_err);
    }

    hdrl_sort_double_pairs(vec_image, vec_image_err);

    trunc_size = (vec_size - nhigh_int) - nlow_int;
    d = cpl_vector_get_data(vec_image);
    e = cpl_vector_get_data(vec_image_err);
    vec_trunc = cpl_vector_wrap(trunc_size, d + nlow_int);

    /*    COMPUTE THE MIN-MAX CLIP MEAN */
    *mean_mm  = cpl_vector_get_mean(vec_trunc);

    if (naccepted) {
        *naccepted = trunc_size;
    }

    if (reject_low) {
        *reject_low = d[nlow_int];
    }
    if (reject_high) {
        *reject_high = d[vec_size - nhigh_int - 1];
    }

    if (mean_mm_err) {
        cpl_vector * vec_trunc_err;
        /* if multiple equal elements use the one with the smallest error
         * get the equal range, sort the errors and write the smallest into the
         * valid array ends */
        intptr_t l = get_lower_bound(vec_image, d[nlow_int]);
        intptr_t h = get_upper_bound(vec_image, d[nlow_int]);
        if (h - l > 1 && h - l != vec_size) {
            cpl_vector * e_vec = cpl_vector_extract(vec_image_err, l, h - 1, 1);
            cpl_vector_sort(e_vec, CPL_SORT_ASCENDING);
            for (intptr_t i = nlow_int; i < h; i++) {
                cpl_vector_set(vec_image_err, i,
                               cpl_vector_get(e_vec, i - nlow_int));
            }
            cpl_vector_delete(e_vec);
        }

        l = get_lower_bound(vec_image, d[vec_size - nhigh_int - 1]);
        h = get_upper_bound(vec_image, d[vec_size - nhigh_int - 1]);
        if (h - l > 1 && h - l != vec_size) {
            cpl_vector * e_vec = cpl_vector_extract(vec_image_err, l, h - 1, 1);
            cpl_vector_sort(e_vec, CPL_SORT_ASCENDING);
            for (intptr_t i = l; i < vec_size - nhigh; i++) {
                cpl_vector_set(vec_image_err, i,
                               cpl_vector_get(e_vec, i - l));
            }
            cpl_vector_delete(e_vec);
        }

        vec_trunc_err = cpl_vector_wrap(trunc_size, e + nlow_int);
        /*Propagate the errors (cpl_vector_power is very slow PIPE-4330) */
        cpl_vector_multiply(vec_trunc_err, vec_trunc_err);
        *mean_mm_err = sqrt(cpl_vector_get_mean(vec_trunc_err) /
                            cpl_vector_get_size(vec_trunc_err));
        cpl_vector_unwrap(vec_trunc_err);
    }
    /* CLEAN, AND RETURN */
    cpl_vector_unwrap(vec_trunc);
    if (!inplace) {
        cpl_vector_delete(vec_image);
        cpl_vector_delete(vec_image_err);
    }
    return cpl_error_get_code();
}



/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Compute mean image value using kappa-sigma clipping method
  @param source         Input image 
  @param error          Input error image
  @param kappa_low      Number of sigmas for lower threshold
  @param kappa_high     Number of sigmas for upper threshold        
  @param iter           Number of iterations
  @param mean_ks        The kappa-sigma clipped mean
  @param mean_ks_err    The propagated error of the kappa-sigma clipped mean
  @param naccepted      Number of accepted values
  @param reject_low     Values lower than this have been rejected
  @param reject_high    Values higher than this have been rejected
  @return   @c CPL_ERROR_NONE or the appropriate error code.
  @see hdrl_kappa_sigma_clip()
  
  This function converts the image inputs into the proper data types in
  order to call the hdrl_kappa_sigma_clip() function.
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_kappa_sigma_clip_image(
        const cpl_image   * source,
        const cpl_image   * error,
        const double        kappa_low,
        const double        kappa_high,
        const int           iter,
        double            * mean_ks,
        double            * mean_ks_err,
        cpl_size          * naccepted,
        double            * reject_low,
        double            * reject_high)
{
    cpl_vector * vec_source = NULL;
    cpl_vector * vec_error = NULL;

    /* Check Entries */
    cpl_error_ensure(source != NULL, CPL_ERROR_NULL_INPUT,
            return CPL_ERROR_NULL_INPUT, "Null input source image!");
    cpl_error_ensure(error != NULL, CPL_ERROR_NULL_INPUT,
            return CPL_ERROR_NULL_INPUT, "Null input error image!");
    cpl_error_ensure(cpl_image_get_size_x(source)==cpl_image_get_size_x(error),
            CPL_ERROR_INCOMPATIBLE_INPUT, return CPL_ERROR_INCOMPATIBLE_INPUT,
            "source and error image musty have same X size");
    cpl_error_ensure(cpl_image_get_size_y(source)==cpl_image_get_size_y(error),
            CPL_ERROR_INCOMPATIBLE_INPUT, return CPL_ERROR_INCOMPATIBLE_INPUT,
            "source and error image musty have same Y size");

    /* compress images to vectors excluding the bad pixels */
    vec_source = hdrl_image_to_vector(source, NULL);
    vec_error = hdrl_image_to_vector(error, cpl_image_get_bpm_const(source));

    if (vec_source != NULL && vec_error != NULL) {
        /* Call here the real sigma-clipping function */
        hdrl_kappa_sigma_clip(vec_source, vec_error, kappa_low, kappa_high,
                             iter, CPL_TRUE, mean_ks, mean_ks_err, naccepted,
                             reject_low, reject_high);
    }
    /* no good pixels */
    else {
        *mean_ks = NAN;
        *mean_ks_err = NAN;
        *naccepted = 0;
        *reject_low = NAN;
        *reject_high = NAN;
    }

    cpl_msg_debug(cpl_func, "mean_ks, mean_ks_err, naccepted:  %g, %g, %ld",
                  *mean_ks, *mean_ks_err, (long)*naccepted);

    cpl_vector_delete(vec_source);
    cpl_vector_delete(vec_error);
    return cpl_error_get_code();
}

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief get first index that compares greater than value
 * @param vec vector to check
 * @param count size of vector
 * @param val upper bound to check
 */
/* ---------------------------------------------------------------------------*/
static long get_upper_bound_d(double * vec, long count, double val)
{
    long first = 0;
    while (count > 0)
    {
        long step = count / 2;
        long it = first + step;
        if (!(val < vec[it])) {
            first = it + 1;
            count -= step + 1;
        }
        else
            count = step;
    }
    return first;
}

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief get first index that compares greater than value
 * @param vec vector to check
 * @param val upper bound to check
 */
/* ---------------------------------------------------------------------------*/
static long get_upper_bound(cpl_vector * vec, double val)
{
    double * d = cpl_vector_get_data(vec);
    long count = cpl_vector_get_size(vec);
    return get_upper_bound_d(d, count, val);
}
/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief get index that compares does not compare less than value
 * @param vec vector to check
 * @param count size of vector
 * @param val upper bound to check
 */
/* ---------------------------------------------------------------------------*/
static long get_lower_bound_d(double * vec, long count, double val)
{
    long first = 0;
    while (count > 0)
    {
        long step = count / 2;
        long it = first + step;
        if (vec[it] < val) {
            first = it + 1;
            count -= step + 1;
        }
        else
            count = step;
    }
    return first;
}

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief get index that compares does not compare less than value
 * @param vec vector to check
 * @param val upper bound to check
 */
/* ---------------------------------------------------------------------------*/
static long get_lower_bound(cpl_vector * vec, double val)
{
    double * d = cpl_vector_get_data(vec);
    long count = cpl_vector_get_size(vec);
    return get_lower_bound_d(d, count, val);
}

/* compute mean and error without needing to wrap a vector, the allocation can
 * be very expensive for small stacks of images */
static void get_mean_err(const double * d, const double * e, long count,
                         double * rm, double * re)
{
    double m = 0.;
    for (long i = 0; i < count; i++) {
        m += (d[i] - m) / (double)(i + 1);
    }
    *rm = m;

    if (re) {
        double se = 0;
        for (long i = 0; i < count; i++) {
            se += e[i] * e[i];
        }
        *re = sqrt(se) / count;
    }
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief   Compute mean using kappa-sigma clipping.
  @param   vec         The vector for which mean is to be computed.
  @param   vec_err     The error of vec. 
  @param   kappa_low   Number of sigmas for lower threshold.
  @param   kappa_high  Number of sigmas for upper threshold.
  @param   iter        Number of iterations.
  @param   inplace     if true the vectors input are modified
  @param   mean_ks     The kappa-sigma clipped mean.
  @param   mean_ks_err The propagated error of the kappa-sigma clipped mean.
  @param   naccepted   Number of accepted values.
  @param   reject_low  Values lower than this have been rejected.
  @param   reject_high Values higher than this have been rejected
  @return   @c CPL_ERROR_NONE or the appropriate error code.

  The function computes the arithmetic mean of a vector after rejecting 
  outliers using kappa-sigma clipping. Robust estimates of the mean and 
  standard deviation are used to derive the interval within which values in 
  the vector are considered good.

  The sigma-clipping is applied on the vec vector data. The vec_err vector 
  is used for the error computation.

  An iterative process of rejection of the outlier elements of vec is
  applied. iter specifies the maximum number of iterations.

  At each iteration, the median and sigma values of the vector are computed and
  used to derive low and high thresholds (\f$median-kappa\_low \times sigma\f$
  and \f$median+kappa\_low \times sigma\f$). The values of vec outside those
  bounds are rejected and the remaining values are passed to the next
  iteration.

  The mean value of the remaining elements is stored into mean_ks.
  mean_ks_err contains \f$\frac{\sum_i{val_i^{2}}}{N}\f$ where \f$val_i\f$
  are the remaining elements of vec_err and N the number of those elements. 
  The N value is stored in naccepted.

  reject_low and reject_high are the final thresholds differenciating the 
  rejected pixels from the others.

  The iterative process is illustrated here:
  \image html sigclip_algorithm.png

  Note that the \f$\sigma\f$ used for the thresholding in the different
  iterations is not the standard deviation but the scaled
  Median Absolute Deviation (MAD). The scaling is
  \f$\sigma = MAD \times CPL_MATH_STD_MAD\f$.

  The MAD is a more robust estimate of the scale of the distribution than the
  standard deviation but only has ??% of the asymptotic statistical efficiency
  for normal distributed data. This higher error in scale parameter only has
  limited influence on the result as it is only used determination of clipping
  thresholds.

 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_kappa_sigma_clip(
        cpl_vector        * vec,
        cpl_vector        * vec_err,
        const double        kappa_low,
        const double        kappa_high,
        const int           iter,
        cpl_boolean         inplace,
        double            * mean_ks,
        double            * mean_ks_err,
        cpl_size          * naccepted,
        double            * reject_low,
        double            * reject_high)
{
    /*    VARIABLES ON THE FUNCTION SCOPE:

          vec_image       a deep copy of the input vector vec.
          mean_ks         kappa-sigma clip mean (return variable).
          mean_ks         kappa-sigma clip mean (return variable).
    */

    cpl_vector   * vec_image = NULL;
    cpl_vector   * vec_image_err = NULL;

    cpl_size       vec_size;
    double lower_bound = 0.;
    double upper_bound = 0.;

    cpl_error_ensure(vec != NULL, CPL_ERROR_NULL_INPUT,
                     return CPL_ERROR_NULL_INPUT, "Null input vector data");
    cpl_error_ensure(vec_err != NULL, CPL_ERROR_NULL_INPUT,
                     return CPL_ERROR_NULL_INPUT, "Null input vector errors");
    cpl_error_ensure(cpl_vector_get_size(vec) == cpl_vector_get_size(vec_err),
                     CPL_ERROR_INCOMPATIBLE_INPUT,
                     return CPL_ERROR_INCOMPATIBLE_INPUT,
                     "input data and error vectors must have same sizes");
    cpl_error_ensure(mean_ks != NULL, CPL_ERROR_NULL_INPUT,
                     return CPL_ERROR_NULL_INPUT, "Null input mean storage");
    cpl_error_ensure(iter > 0, CPL_ERROR_ILLEGAL_INPUT,
                     return CPL_ERROR_ILLEGAL_INPUT,
                     "iter must be larger than 0");

    if (inplace) {
        vec_image = vec;
        vec_image_err = vec_err;
    }
    else {
        vec_image = cpl_vector_duplicate(vec);
        vec_image_err = cpl_vector_duplicate(vec_err);
    }
    double * vec_data = cpl_vector_get_data(vec_image);
    double * vec_data_orig = vec_data;
    double * vec_data_err = cpl_vector_get_data(vec_image_err);
    vec_size = cpl_vector_get_size(vec_image);

    /* sort the two vectors by the data */
    hdrl_sort_double_pairs(vec_image, vec_image_err);

    for(int it = 0; it < iter; it++) {
        double median, sigma;
        cpl_size lower_index, upper_index;

        /* Nothing to do if only one data point */
        if(vec_size == 1) {
            lower_bound = vec_data[0];
            upper_bound = lower_bound;
            break;
        }

        /*  STEPS OF KAPPA SIGMA CLIP
            1. Sort the vector.
            2. Find mean, and standard deviation (sigma).
            3. Find lower, and upper bound after kappa-sigma clip.
            4. Find index which corresponds to lower and upper bound
            5. Extract the vector within the index bound.
        */

        /* Use median as a robust estimator of the mean */

        /* standard deviation from Median Absolute Deviation (MAD) as appropriate
           for a Gaussian distribution */

        /* offset index into original uncut vec_image */
        cpl_size orig_offset = (cpl_size)(vec_data - vec_data_orig) + 1;
        median = hcpl_vector_get_mad_window(vec_image, orig_offset,
                                            orig_offset + vec_size - 1, &sigma);

        if(sigma <= 0){
            sigma=nextafter(0,1.0);
        }
        sigma *= CPL_MATH_STD_MAD;

        lower_bound = median - kappa_low * sigma;
        upper_bound = median + kappa_high * sigma;

        lower_index = get_lower_bound_d(vec_data, vec_size, lower_bound);
        upper_index = get_upper_bound_d(vec_data, vec_size, upper_bound);
        upper_index = CX_MAX(upper_index - 1, 0);

        /* Stop if no outliers were found */
        if ((lower_index == 0) && (upper_index == vec_size - 1))
            break;

        /* truncate vector */
        vec_data = vec_data + lower_index;
        vec_data_err = vec_data_err + lower_index;
        vec_size = upper_index - lower_index + 1;
    }

    /*    COMPUTE THE KAPPA-SIGMA CLIP MEAN */
    get_mean_err(vec_data, vec_data_err, vec_size, mean_ks, mean_ks_err);

    if (naccepted) *naccepted = vec_size;

    if (reject_low) *reject_low = lower_bound;
    if (reject_high) *reject_high = upper_bound;

    /* CLEAN, AND RETURN */
    if (!inplace) {
        cpl_vector_delete(vec_image);
        cpl_vector_delete(vec_image_err);
    }
    return cpl_error_get_code();
}


/*---------------------------------------------------------------------------*/
/**
  @internal
  @brief   Sort an array @a u1 of doubles, and permute an array @a u2
           in the same way as @a u1 is permuted.
  @param   u1   Pointer to the first array.
  @param   u2   Pointer to the second array.
  @return   @c CPL_ERROR_NONE or the appropriate error code.
 */
/*---------------------------------------------------------------------------*/
static cpl_error_code hdrl_sort_double_pairs(cpl_vector *u1, cpl_vector *u2)
{
    cpl_bivector * bi_all = NULL;

    cpl_error_ensure(u1 != NULL, CPL_ERROR_NULL_INPUT,
                     return CPL_ERROR_NULL_INPUT, "NULL pointer to 1st array");
    cpl_error_ensure(u2 != NULL, CPL_ERROR_NULL_INPUT,
                     return CPL_ERROR_NULL_INPUT, "NULL pointer to 2nd array");

    bi_all = cpl_bivector_wrap_vectors(u1, u2);
    cpl_bivector_sort(bi_all, bi_all, CPL_SORT_ASCENDING, CPL_SORT_BY_X);

    /* cleaning up */
    cpl_bivector_unwrap_vectors(bi_all);

    return CPL_ERROR_NONE;
}


/**@}*/
/** @endcond */
