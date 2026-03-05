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

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_elemop.h"
#include "hdrl_image.h"
#include "hdrl_collapse.h"

#include <cpl.h>
#include <math.h>

/*-----------------------------------------------------------------------------
                                   Functions
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    Add two images, store the result in the first image.
  @param    self      first operand.
  @param    other     second operand.
  @return   the cpl-error-code or CPL_ERROR_NONE
  
  The first input image is modified to contain the result of the operation.

  The bad pixel map of the first image becomes the union of the bad pixel 
  maps of the input images.
  
  Possible cpl-error-code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
  - CPL_ERROR_INCOMPATIBLE_INPUT if the input images have different sizes

 */
/*----------------------------------------------------------------------------*/
cpl_error_code
hdrl_image_add_image(hdrl_image * self, const hdrl_image * other)
{
    cpl_ensure_code(self, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(other, CPL_ERROR_NULL_INPUT);
    /* size check included in elemop */
    return hdrl_elemop_image_add_image(hdrl_image_get_image(self), hdrl_image_get_error(self),
                                         hdrl_image_get_image_const(other),
                                         hdrl_image_get_error_const(other));
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    Add two images.
  @param    self  first operand
  @param    other  second operand
  @return   1 newly allocated image or NULL on error

  Creates a new image, being the result of the operation, and returns it to
  the caller. The returned image must be deallocated using hdrl_image_delete().

  The bad pixels map of the result is the union of the bad pixels maps of
  the input images.
  
  Possible cpl-error-code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
  - CPL_ERROR_INCOMPATIBLE_INPUT if the input images have different sizes
 */
/*----------------------------------------------------------------------------*/
hdrl_image *
hdrl_image_add_image_create(const hdrl_image * self, const hdrl_image * other)
{
    hdrl_image * n = hdrl_image_duplicate(self);

    if (hdrl_image_add_image(n, other)) {
        hdrl_image_delete(n);
        return NULL;
    }

    return n;
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    Elementwise addition of a scalar to an image
  @param    self    Image to be modified in place.
  @param    value   Number to add
  @return   CPL_ERROR_NONE or the relevant the cpl-error-code on error
 
  Modifies the image by adding a number to each of its pixels with error 
  propagation.
  
  Possible cpl-error-code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/*----------------------------------------------------------------------------*/
cpl_error_code
hdrl_image_add_scalar(hdrl_image * self, hdrl_value value)
{
    cpl_ensure_code(self, CPL_ERROR_NULL_INPUT);
    /* size check included in elemop */
    return hdrl_elemop_image_add_scalar(hdrl_image_get_image(self),
                                        hdrl_image_get_error(self),
                                        value.data, value.error);
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    Subtract two images, store the result in the first image.
  @param    self     first operand.
  @param    other    second operand.
  @return   the cpl-error-code or CPL_ERROR_NONE
  @see      hdrl_image_add_image()
 */
/*----------------------------------------------------------------------------*/
cpl_error_code
hdrl_image_sub_image(hdrl_image * self, const hdrl_image * other)
{
    cpl_ensure_code(self, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(other, CPL_ERROR_NULL_INPUT);
    /* size check included in elemop */
    return hdrl_elemop_image_sub_image(hdrl_image_get_image(self), hdrl_image_get_error(self),
                                         hdrl_image_get_image_const(other),
                                         hdrl_image_get_error_const(other));
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    Subtract two images.
  @param    self   first operand
  @param    other  second operand
  @return   1 newly allocated image or NULL on error
  @see      hdrl_image_add_image_create()
 */
/*----------------------------------------------------------------------------*/
hdrl_image *
hdrl_image_sub_image_create(const hdrl_image * self, const hdrl_image * other)
{
    hdrl_image * n = hdrl_image_duplicate(self);

    if (hdrl_image_sub_image(n, other)) {
        hdrl_image_delete(n);
        return NULL;
    }

    return n;
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    Elementwise subtraction of a scalar from an image
  @param    self    Image to be modified in place.
  @param    value  Number to subtract
  @return   CPL_ERROR_NONE or the relevant the cpl-error-code on error
  @see      hdrl_image_add_scalar()
 */
/*----------------------------------------------------------------------------*/
cpl_error_code
hdrl_image_sub_scalar(hdrl_image * self, hdrl_value value)
{
    cpl_ensure_code(self, CPL_ERROR_NULL_INPUT);
    /* size check included in elemop */
    return hdrl_elemop_image_sub_scalar(hdrl_image_get_image(self),
                                        hdrl_image_get_error(self),
                                        value.data, value.error);
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    Multiply two images, store the result in the first image.
  @param    self      first operand.
  @param    other     second operand.
  @return   the cpl-error-code or CPL_ERROR_NONE
  @see      hdrl_image_add_image()
 */
/*----------------------------------------------------------------------------*/
cpl_error_code
hdrl_image_mul_image(hdrl_image * self, const hdrl_image * other)
{
    cpl_ensure_code(self, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(other, CPL_ERROR_NULL_INPUT);
    /* size check included in elemop */
    return hdrl_elemop_image_mul_image(hdrl_image_get_image(self), hdrl_image_get_error(self),
                                         hdrl_image_get_image_const(other),
                                         hdrl_image_get_error_const(other));
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    Multiply two images.
  @param    self   first operand
  @param    other  second operand
  @return   1 newly allocated image or NULL on error
  @see      hdrl_image_add_image_create()
 */
/*----------------------------------------------------------------------------*/
hdrl_image *
hdrl_image_mul_image_create(const hdrl_image * self, const hdrl_image * other)
{
    hdrl_image * n = hdrl_image_duplicate(self);

    if (hdrl_image_mul_image(n, other)) {
        hdrl_image_delete(n);
        return NULL;
    }

    return n;
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    Elementwise multiplication of an image with a scalar
  @param    self    Image to be modified in place.
  @param    value   Number to multiply with
  @return   CPL_ERROR_NONE or the relevant the cpl-error-code on error
  @see      hdrl_image_add_scalar()
 */
/*----------------------------------------------------------------------------*/
cpl_error_code
hdrl_image_mul_scalar(hdrl_image * self, hdrl_value value)
{
    cpl_ensure_code(self, CPL_ERROR_NULL_INPUT);
    /* size check included in elemop */
    return hdrl_elemop_image_mul_scalar(hdrl_image_get_image(self),
                                        hdrl_image_get_error(self),
                                        value.data, value.error);
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    Divide two images, store the result in the first image.
  @param    self   first operand
  @param    other  second operand
  @return   the cpl-error-code or CPL_ERROR_NONE
  @see      hdrl_image_add_image()

  @note The result of division with a zero-valued pixel is marked as a bad
  pixel. Additionally, the value and the error are set to NAN


  Possible cpl-error-code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
  - CPL_ERROR_INCOMPATIBLE_INPUT if the input images have different sizes
 */
/*----------------------------------------------------------------------------*/
cpl_error_code
hdrl_image_div_image(hdrl_image * self, const hdrl_image * other)
{
    cpl_ensure_code(self, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(other, CPL_ERROR_NULL_INPUT);
    /* size check included in elemop */
    return hdrl_elemop_image_div_image(hdrl_image_get_image(self), hdrl_image_get_error(self),
                                         hdrl_image_get_image_const(other),
                                         hdrl_image_get_error_const(other));
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    Divide two images and return the resulting image
  @param    self   first operand
  @param    other  second operand
  @return   1 newly allocated image or NULL on error
  @see      hdrl_image_add_image_create()

  @note The result of division with a zero-valued pixel is marked as a bad
  pixel. Additionally, the value and the error are set to NAN


  Possible cpl-error-code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
  - CPL_ERROR_INCOMPATIBLE_INPUT if the input images have different sizes
 */
/*----------------------------------------------------------------------------*/
hdrl_image *
hdrl_image_div_image_create(const hdrl_image * self, const hdrl_image * other)
{
    hdrl_image * n = hdrl_image_duplicate(self);

    if (hdrl_image_div_image(n, other)) {
        hdrl_image_delete(n);
        return NULL;
    }

    return n;
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    Elementwise division of an image with a scalar
  @param    self   Image to be modified in place.
  @param    value  Non-zero number to divide with
  @return   CPL_ERROR_NONE or the relevant the cpl-error-code on error
  @see      cpl_image_add_image_scalar()

  Possible cpl-error-code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL

 */
/*----------------------------------------------------------------------------*/
cpl_error_code
hdrl_image_div_scalar(hdrl_image * self, hdrl_value value)
{
    cpl_ensure_code(self, CPL_ERROR_NULL_INPUT);
    /* size check included in elemop */
    return hdrl_elemop_image_div_scalar(hdrl_image_get_image(self),
                                        hdrl_image_get_error(self),
                                        value.data, value.error);
}
/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    Collapse the image
\todo copy from overscan
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
hdrl_image_collapse(hdrl_collapse_imagelist_to_vector_t * red,
                    const hdrl_image * self,
                    hdrl_data_t * result, hdrl_error_t * error,
                    int * contrib, void * eout)
{
    cpl_imagelist * ld = cpl_imagelist_new();
    cpl_imagelist * le = cpl_imagelist_new();
    cpl_vector * od = NULL, * oe = NULL;
    cpl_array * oc = NULL;
    cpl_error_code fail;

    CPL_DIAG_PRAGMA_PUSH_IGN(-Wcast-qual);
    cpl_imagelist_set(ld, hdrl_image_get_image((hdrl_image * )self), 0);
    cpl_imagelist_set(le, hdrl_image_get_error((hdrl_image * )self), 0);
    CPL_DIAG_PRAGMA_POP;

    fail = hdrl_collapse_imagelist_to_vector_call(red, ld, le, &od, &oe, &oc,
                                                  eout);
    cpl_imagelist_unwrap(ld);
    cpl_imagelist_unwrap(le);

    if (fail == CPL_ERROR_NONE) {
        if (result) {
            *result = cpl_vector_get(od, 0);
        }
        if (error) {
            *error = cpl_vector_get(oe, 0);
        }
        if (contrib) {
            *contrib = cpl_array_get_int(oc, 0, NULL);
        }
    }
    else {
        if (result) {
            *result = NAN;
        }
        if (error) {
            *error = NAN;
        }
    }

    cpl_vector_delete(od);
    cpl_vector_delete(oe);
    cpl_array_delete(oc);

    return fail;
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    computes mean pixel value and associated error of an image.
  @param    self       input image
  @return   mean and its error in a hdrl_value structure
 */ 
/*----------------------------------------------------------------------------*/
hdrl_value
hdrl_image_get_mean(const hdrl_image * self)
{
    hdrl_collapse_imagelist_to_vector_t * red =
        hdrl_collapse_imagelist_to_vector_mean();
    hdrl_value res;
    hdrl_image_collapse(red, self, &res.data, &res.error, NULL, NULL);
    hdrl_collapse_imagelist_to_vector_delete(red);
    return res;
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief computes the sigma-clipped mean and associated error of an image.
  @param self         input image
  @param kappa_low    low sigma bound
  @param kappa_high   high sigma bound
  @param niter        maximum number of clipping iterators
  @return   clipped mean and its error in a hdrl_value structure
  @see hdrl_kappa_sigma_clip() for the algorithm
 */
/*----------------------------------------------------------------------------*/
hdrl_value
hdrl_image_get_sigclip_mean(const hdrl_image * self, double kappa_low,
                            double kappa_high, int niter)
{
    hdrl_collapse_imagelist_to_vector_t * red =
        hdrl_collapse_imagelist_to_vector_sigclip(kappa_low, kappa_high, niter);
    hdrl_value res;
    hdrl_image_collapse(red, self, &res.data, &res.error, NULL, NULL);
    hdrl_collapse_imagelist_to_vector_delete(red);
    return res;
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief computes the minmax rejected mean and the associated error of an image.
  @param self         input image
  @param nlow         number of low pixels to reject
  @param nhigh        number of high pixels to reject
  @return   minmax rejected mean and its error in a hdrl_value structure
  @see hdrl_minmax_clip() for the algorithm
 */
/*----------------------------------------------------------------------------*/
hdrl_value
hdrl_image_get_minmax_mean(const hdrl_image * self, double nlow,
                           double nhigh)
{
    hdrl_collapse_imagelist_to_vector_t * red =
        hdrl_collapse_imagelist_to_vector_minmax(nlow, nhigh);
    hdrl_value res;
    hdrl_image_collapse(red, self, &res.data, &res.error, NULL, NULL);
    hdrl_collapse_imagelist_to_vector_delete(red);
    return res;
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief computes the mode and the associated error of an image.
  @param self        input image
  @param histo_min   minimum value of low pixels to use
  @param histo_max   maximum value of high pixels to be use
  @param bin_size    size of the histogram bin
  @param method      method to use for the mode computation
  @param error_niter number of iterations to compute the error of the mode
  @return            the mode and its error in a hdrl_value structure
  @see hdrl_mode_clip() for the algorithm
 */
/*----------------------------------------------------------------------------*/
hdrl_value
hdrl_image_get_mode(const hdrl_image * self, double histo_min,
			 double histo_max, double bin_size,
			 hdrl_mode_type method, cpl_size error_niter)
{
    hdrl_collapse_imagelist_to_vector_t * red =
        hdrl_collapse_imagelist_to_vector_mode(histo_min, histo_max, bin_size,
					       method, error_niter);
    hdrl_value res;
    hdrl_image_collapse(red, self, &res.data, &res.error, NULL, NULL);
    hdrl_collapse_imagelist_to_vector_delete(red);
    return res;
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief computes the median and associated error of an image.
  @param self         input image
  @return   median and its error in a hdrl_value structure

  This function computes the median and the associated error of the image. For
  the error propagation the error is scaled by the sqrt of the statistical
  efficiency of the median on normal distributed data which is
  \f$ \frac{\pi}{ 2 } \f$
 */
/*----------------------------------------------------------------------------*/
hdrl_value
hdrl_image_get_median(const hdrl_image * self)
{
    hdrl_collapse_imagelist_to_vector_t * red =
        hdrl_collapse_imagelist_to_vector_median();
    hdrl_value res;
    hdrl_image_collapse(red, self, &res.data, &res.error, NULL, NULL);
    hdrl_collapse_imagelist_to_vector_delete(red);
    return res;
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief computes the weighted mean and associated error of an image.
  @param self         input image
  @return   weighted mean and its error in a hdrl_value structure
 */
/*----------------------------------------------------------------------------*/
hdrl_value
hdrl_image_get_weighted_mean(const hdrl_image * self)
{
    hdrl_collapse_imagelist_to_vector_t * red =
        hdrl_collapse_imagelist_to_vector_weighted_mean();
    hdrl_value res;
    hdrl_image_collapse(red, self, &res.data, &res.error, NULL, NULL);
    hdrl_collapse_imagelist_to_vector_delete(red);
    return res;
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief computes the standard deviation of the data of an image
  @param self         input image
  @return   CPL_ERROR_NONE or the relevant the cpl-error-code on error
  @see cpl_image_get_stdev
 */
/*----------------------------------------------------------------------------*/
double
hdrl_image_get_stdev(const hdrl_image * self)
{
    return cpl_image_get_stdev(hdrl_image_get_image_const(self));
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    computes the sum of all pixel values and the associated error of an
            image.
  @param    self       input image
  @return   sum and its error in a hdrl_value structure
 */
/*----------------------------------------------------------------------------*/
hdrl_value
hdrl_image_get_sum(const hdrl_image * self)
{
    int contrib;
    hdrl_value res;
    hdrl_collapse_imagelist_to_vector_t * red =
        hdrl_collapse_imagelist_to_vector_mean();
    cpl_error_code err = hdrl_image_collapse(red, self, &res.data, &res.error,
                                             &contrib, NULL);
    if (err == CPL_ERROR_NONE) {
        res.data *= contrib;
        res.error *= contrib;
    }
    else {
        res.data = NAN;
        res.error = NAN;
    }
    hdrl_collapse_imagelist_to_vector_delete(red);
    return res;
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    computes the sum of all pixel values and the error of a squared
            image.
  @param    self       input image
  @return   squared sum and its error in a hdrl_value structure
 */
/*----------------------------------------------------------------------------*/
hdrl_value
hdrl_image_get_sqsum(const hdrl_image * self)
{
    /* TODO could be a specialized collapse method for better performance */
    hdrl_image * tmp = hdrl_image_pow_scalar_create(self, (hdrl_value){2, 0.});
    hdrl_value res = hdrl_image_get_sum(tmp);
    hdrl_image_delete(tmp);
    return res;
}


/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    computes the power of an image by a scalar
  @param    self       input image
  @param    exponent   exponent of the power
  @return   CPL_ERROR_NONE or the relevant the cpl-error-code on error

 */
/*----------------------------------------------------------------------------*/
cpl_error_code
hdrl_image_pow_scalar(hdrl_image * self, const hdrl_value exponent)
{
    cpl_ensure_code(self, CPL_ERROR_NULL_INPUT);
    /* size check included in elemop */
    return hdrl_elemop_image_pow_scalar(hdrl_image_get_image(self),
                                        hdrl_image_get_error(self),
                                        exponent.data, exponent.error);
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    computes the power of an image by a scalar creating a new image
  @param    self       input image
  @param    exponent   exponent of the power
  @return   new image containing the powered data or NULL on error
  @see hdrl_image_pow_scalar
 */
/*----------------------------------------------------------------------------*/
hdrl_image *
hdrl_image_pow_scalar_create(const hdrl_image * self, const hdrl_value exponent)
{
    hdrl_image * n = hdrl_image_duplicate(self);

    if (hdrl_image_pow_scalar(n, exponent)) {
        hdrl_image_delete(n);
        return NULL;
    }

    return n;
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    computes the exponential of an image by a scalar
  @param    self       input image
  @param    base       base of the power
  @return   CPL_ERROR_NONE or the relevant the cpl-error-code on error

 */
/*----------------------------------------------------------------------------*/
cpl_error_code
hdrl_image_exp_scalar(hdrl_image * self, const hdrl_value base)
{
    cpl_ensure_code(self, CPL_ERROR_NULL_INPUT);
    /* size check included in elemop */
    return hdrl_elemop_image_exp_scalar(hdrl_image_get_image(self),
                                        hdrl_image_get_error(self),
                                        base.data, base.error);
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    computes the exponential of an image by a scalar creating a new image
  @param    self       input image
  @param    base       base of the power
  @return   new image containing the powered data or NULL on error
  @see hdrl_image_pow_scalar
 */
/*----------------------------------------------------------------------------*/
hdrl_image *
hdrl_image_exp_scalar_create(const hdrl_image * self, const hdrl_value base)
{
    hdrl_image * n = hdrl_image_duplicate(self);

    if (hdrl_image_exp_scalar(n, base)) {
        hdrl_image_delete(n);
        return NULL;
    }

    return n;
}
