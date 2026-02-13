/* $Id: hdrl_elemop.c,v 1.4 2013-10-04 08:03:14 jtaylor Exp $
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

/*
 * $Author: jtaylor $
 * $Date: 2013-10-04 08:03:14 $
 * $Revision: 1.4 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_utils.h"
#include "hdrl_types.h"
#include "hdrl_elemop.h"

#include <cpl.h>
#include <math.h>

#ifndef SQR
#define SQR(a) ((a) * (a))
#endif

/*-----------------------------------------------------------------------------
                                   Functions
 -----------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief add two arrays of with error propagation in place
 *
 * @param a   array of values of size na
 * @param ea  array of errors of a
 * @param na  size of a arrays
 * @param b   array of values of size nb
 * @param eb  array of errors of b
 * @param nb  size of b array, must be equal to na or 1
 * @param mask  bad pixel mask, positions where it is true are not modified
 *
 * a = a + b
 * e = hypot(ea, eb)
 * error propagation of first order, correlations not considered
 * except for the a === b case (correlation = 1)
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_elemop_add(hdrl_data_t * a, hdrl_error_t * ea, size_t na,
                               const hdrl_data_t * b, const hdrl_error_t * eb, size_t nb,
                               const cpl_binary * mask)
{
    if (a == b && ea == eb) {
        for (size_t i = 0; i < na; i++) {
            if (mask == NULL || mask[i] == CPL_BINARY_0) {
                a[i] += a[i];
                ea[i] *= 2;
            }
        }
    }
    else {
        cpl_ensure_code(na == nb || nb == 1, CPL_ERROR_ILLEGAL_INPUT);

        if (nb == 1) {
            for (size_t i = 0; i < na; i++) {
                if (mask == NULL || mask[i] == CPL_BINARY_0) {
                    a[i] += b[0];
                    ea[i] = hypot(ea[i], eb[0]);
                }
            }
        }
        else {
            for (size_t i = 0; i < na; i++) {
                if (mask == NULL || mask[i] == CPL_BINARY_0) {
                    a[i] += b[i];
                    ea[i] = hypot(ea[i], eb[i]);
                }
            }
        }
    }

    return CPL_ERROR_NONE;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief subtract two arrays of with error propagation in place
 *
 * @param a   array of values of size na
 * @param ea  array of errors of a
 * @param na  size of a arrays
 * @param b   array of values of size nb
 * @param eb  array of errors of b
 * @param nb  size of b array, must be equal to na or 1
 * @param mask  bad pixel mask, positions where it is true are not modified
 *
 * a = a - b
 * e = hypot(ea, eb)
 * error propagation of first order, correlations not considered
 * except for the a === b case (correlation = 1)
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_elemop_sub(hdrl_data_t * a, hdrl_error_t * ea, size_t na,
                               const hdrl_data_t * b, const hdrl_error_t * eb, size_t nb,
                               const cpl_binary * mask)
{
    if (a == b && ea == eb) {
        for (size_t i = 0; i < na; i++) {
            if (mask == NULL || mask[i] == CPL_BINARY_0) {
                a[i] = 0.;
                ea[i] = 0.;
            }
        }
    }
    else {
        cpl_ensure_code(na == nb || nb == 1, CPL_ERROR_ILLEGAL_INPUT);

        if (nb == 1) {
            for (size_t i = 0; i < na; i++) {
                if (mask == NULL || mask[i] == CPL_BINARY_0) {
                    a[i] -= b[0];
                    ea[i] = hypot(ea[i], eb[0]);
                }
            }
        }
        else {
            for (size_t i = 0; i < na; i++) {
                if (mask == NULL || mask[i] == CPL_BINARY_0) {
                    a[i] -= b[i];
                    ea[i] = hypot(ea[i], eb[i]);
                }
            }
        }
    }

    return CPL_ERROR_NONE;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief multiply two arrays of with error propagation in place
 *
 * @param a   array of values of size na
 * @param ea  array of errors of a
 * @param na  size of a arrays
 * @param b   array of values of size nb
 * @param eb  array of errors of b
 * @param nb  size of b array, must be equal to na or 1
 * @param mask  bad pixel mask, positions where it is true are not modified
 *
 * a = a * b
 * e = hypot(a * eb, b * ea)
 * error propagation of first order, correlations not considered
 * except for the a === b case (correlation = 1)
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_elemop_mul(hdrl_data_t * a, hdrl_error_t * ea, size_t na,
                               const hdrl_data_t * b, const hdrl_error_t * eb, size_t nb,
                               const cpl_binary * mask)
{
    if (a == b && ea == eb) {
        for (size_t i = 0; i < na; i++) {
            if (mask == NULL || mask[i] == CPL_BINARY_0) {
                const hdrl_data_t a_ = a[i] * a[i];
                ea[i] = 2 * fabs(a[i]) * ea[i];
                a[i] = a_;
            }
        }
    }
    else {
        cpl_ensure_code(na == nb || nb == 1, CPL_ERROR_ILLEGAL_INPUT);

        if (nb == 1) {
            const hdrl_data_t b0 = b[0];
            const hdrl_error_t eb0 = eb[0];
            for (size_t i = 0; i < na; i++) {
                if (mask == NULL || mask[i] == CPL_BINARY_0) {
                    const hdrl_data_t a_ = a[i] * b0;
                    ea[i] = hypot(eb0 * a[i], ea[i] * b0);
                    a[i] = a_;
                }
            }
        }
        else {
            for (size_t i = 0; i < na; i++) {
                if (mask == NULL || mask[i] == CPL_BINARY_0) {
                    const hdrl_data_t a_ = a[i] * b[i];
                    ea[i] = hypot(eb[i] * a[i], ea[i] * b[i]);
                    a[i] = a_;
                }
            }
        }
    }

    return CPL_ERROR_NONE;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief divide two arrays of with error propagation in place
 *
 * @param a   array of values of size na
 * @param ea  array of errors of a
 * @param na  size of a arrays
 * @param b   array of values of size nb
 * @param eb  array of errors of b
 * @param nb  size of b array, must be equal to na or 1
 * @param mask  bad pixel mask, positions where it is true are not modified
 *
 * a = a / b
 * e = hypot(ea / b, eb * a / (b * b))
 * error propagation of first order, correlations not considered
 * except for the a === b case (correlation = 1)
 *
 * unlike cpl_image_divide pixels which are divided by zero are set to NAN and
 * not marked as bad.
 *
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_elemop_div(hdrl_data_t * a, hdrl_error_t * ea, size_t na,
                               const hdrl_data_t * b, const hdrl_error_t * eb, size_t nb,
                               const cpl_binary * mask)
{
    if (a == b && ea == eb) {
        for (size_t i = 0; i < na; i++) {
            if (mask == NULL || mask[i] == CPL_BINARY_0) {
                ea[i] = 0.;
                a[i] = 1.;
            }
        }
    }
    else {
        cpl_ensure_code(na == nb || nb == 1, CPL_ERROR_ILLEGAL_INPUT);

        if (nb == 1) {
            const hdrl_data_t rb0 = 1. / b[0];
            const hdrl_error_t eb0 = eb[0];
            for (size_t i = 0; i < na; i++) {
                if (mask == NULL || mask[i] == CPL_BINARY_0) {
                    const hdrl_data_t a_ = a[i] * rb0;
                    if (b[0] == 0.) {
                        a[i] = NAN;
                        ea[i] = NAN;
                    }
                    else {
                        /* sqrt((ea / b)^2 + (eb * a / b^2)^2) */
                        ea[i] = sqrt(SQR(ea[i] * rb0) +
                                     SQR(eb0 * a_ * rb0));
                        a[i] = a_;
                    }
                }
            }
        }
        else {
            for (size_t i = 0; i < na; i++) {
                if (mask == NULL || mask[i] == CPL_BINARY_0) {
                    const hdrl_data_t rb = 1. / b[i];
                    const hdrl_data_t a_ = a[i] * rb;
                    if (b[i] == 0.) {
                        a[i] = NAN;
                        ea[i] = NAN;
                    }
                    else {
                        /* sqrt((ea / b)^2 + (eb * a / b^2)^2) */
                        ea[i] = sqrt(SQR(ea[i] * rb) +
                                     SQR(eb[i] * a_ * rb));
                        a[i] = a_;
                    }
                }
            }
        }
    }

    return CPL_ERROR_NONE;
}

/* Utility function with error propagation logic for pow */
static inline void pow_scalar(hdrl_data_t base, hdrl_error_t base_e,
                hdrl_data_t exp, hdrl_error_t exp_e,
                hdrl_data_t* out, hdrl_error_t* out_e){

    *out_e = 0.0;

    if(base == 0. && exp < 0.){
        *out = NAN;
        *out_e = NAN;
        return;
    }

    if(exp_e == .0 && exp == 2.0){
        *out = base * base;
        *out_e = fabs(2.0 * base_e * base);
        return;
    }

    *out = pow(base, exp);

    /*No exponent error*/
    if(exp_e == 0.){
        *out_e = fabs(*out * (exp / base * base_e));
        return;
    }

    const hdrl_data_t log_base = log(fabs(base));
    *out_e = fabs(*out) * sqrt(SQR( exp / base * base_e)
            + SQR(log_base * exp_e));
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief power of two arrays of with error propagation in place
 *
 * @param a   array of values of size na
 * @param ea  array of errors of a
 * @param na  size of a arrays
 * @param b   array of values of size nb
 * @param eb  array of errors of b
 * @param nb  size of b array, must be equal to na or 1
 * @param mask  bad pixel mask, positions where it is true are not modified
 *
 * a = pow(a, b)
 * e = pow(a, b) * sqrt((a / b * ea)^2 + (ln(a) * eb)^2)
 * error propagation of first order, correlations not considered
 * except for the a === b case (correlation = 1)
 *
 * if b < 0 and a == 0 the result is NAN
 * for a < 0 and eb != 0 the error is not well defined
 *
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_elemop_pow(hdrl_data_t * a, hdrl_error_t * ea, size_t na,
                               const hdrl_data_t * b, const hdrl_error_t * eb, size_t nb,
                               const cpl_binary * mask)
{
    if (a == b && ea == eb) {
        for (size_t i = 0; i < na; i++) {
            if (mask == NULL || mask[i] == CPL_BINARY_0) {
                hdrl_data_t loga = log(fabs(a[i]));
                a[i] = pow(a[i], a[i]);
                ea[i] = fabs(a[i]) *
                    sqrt(SQR(ea[i]) * (1. + SQR(loga) + 2 * loga));
            }
        }
    }
    else {
        cpl_ensure_code(na == nb || nb == 1, CPL_ERROR_ILLEGAL_INPUT);

        if (nb == 1) {
            for (size_t i = 0; i < na; i++) {
                if (mask == NULL || mask[i] == CPL_BINARY_0) {
                    pow_scalar(a[i], ea[i], b[0], eb[0], &a[i], &ea[i]);
                }
            }
        }
        else {
            for (size_t i = 0; i < na; i++) {
                if (mask == NULL || mask[i] == CPL_BINARY_0) {
                    pow_scalar(a[i], ea[i], b[i], eb[i], &a[i], &ea[i]);
                }
            }
        }
    }

    return CPL_ERROR_NONE;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief power of two arrays of with error propagation in place
 *
 * @param a   array of values of size na
 * @param ea  array of errors of a
 * @param na  size of a arrays
 * @param b   array of values of size nb
 * @param eb  array of errors of b
 * @param nb  size of b array, must be equal to na or 1
 * @param mask  bad pixel mask, positions where it is true are not modified
 *
 * a = pow(b, a)
 * e = pow(b, a) * sqrt((b / a * eb)^2 + (ln(b) * ea)^2)
 * error propagation of first order, correlations not considered
 * except for the a === b case (correlation = 1)
 *
 * if a < 0 and b == 0 the result is NAN
 * for b < 0 and ea != 0 the error is not well defined
 *
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_elemop_pow_inverted(hdrl_data_t * a, hdrl_error_t * ea, size_t na,
                               const hdrl_data_t * b, const hdrl_error_t * eb, size_t nb,
                               const cpl_binary * mask)
{
    if (a == b && ea == eb) {
        /* a and b are the same image, order is irrelevant*/
        return hdrl_elemop_pow(a, ea, na, b, eb, nb, mask);
    }
    else {
        cpl_ensure_code(na == nb || nb == 1, CPL_ERROR_ILLEGAL_INPUT);

        if (nb == 1) {
            for (size_t i = 0; i < na; i++) {
                if (mask == NULL || mask[i] == CPL_BINARY_0) {
                    pow_scalar(b[0], eb[0], a[i], ea[i], &a[i], &ea[i]);
                }
            }
        }
        else {
            for (size_t i = 0; i < na; i++) {
                if (mask == NULL || mask[i] == CPL_BINARY_0) {
                    pow_scalar(b[i], eb[i], a[i], ea[i], &a[i], &ea[i]);
                }
            }
        }
    }

    return CPL_ERROR_NONE;
}


typedef cpl_error_code (hdrl_math_op_f)(
                                        hdrl_data_t * a, hdrl_error_t * ea, size_t na,
                                        const hdrl_data_t * b, const hdrl_error_t * eb, size_t nb,
                                        const cpl_binary * mask);

typedef cpl_error_code (hdrl_math_op_image_f)(
                                              cpl_image * a, cpl_image * ae,
                                              const cpl_image * b, const cpl_image * be);

typedef cpl_error_code (hdrl_math_op_image_scalar_f)(
                                                     cpl_image * a, cpl_image * ae,
                                                     const hdrl_data_t b, const hdrl_error_t be);

static cpl_error_code
check_input(cpl_image * a, cpl_image * ae,
            const cpl_image * b, const cpl_image * be)
{
    cpl_ensure_code(a, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(ae, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(b, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(be, CPL_ERROR_NULL_INPUT);

    cpl_ensure_code(cpl_image_get_size_x(a) == cpl_image_get_size_x(ae),
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(cpl_image_get_size_y(a) == cpl_image_get_size_y(ae),
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(cpl_image_get_size_x(b) == cpl_image_get_size_x(be),
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(cpl_image_get_size_y(b) == cpl_image_get_size_y(be),
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(cpl_image_get_size_x(a) == cpl_image_get_size_x(b),
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(cpl_image_get_size_y(a) == cpl_image_get_size_y(b),
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(cpl_image_get_type(a) == HDRL_TYPE_DATA,
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(cpl_image_get_type(ae) == HDRL_TYPE_ERROR,
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(cpl_image_get_type(b) == HDRL_TYPE_DATA,
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(cpl_image_get_type(be) == HDRL_TYPE_ERROR,
                    CPL_ERROR_INCOMPATIBLE_INPUT);

    return CPL_ERROR_NONE;
}


static cpl_error_code
hdrl_elemop_image_scalar(cpl_image * a, cpl_image * ae,
                         const hdrl_data_t b, const hdrl_error_t be,
                         hdrl_math_op_f * f)
{
    const cpl_binary * dmask = NULL;
    cpl_ensure_code(a, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(ae, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(cpl_image_get_type(a) == HDRL_TYPE_DATA,
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(cpl_image_get_type(ae) == HDRL_TYPE_ERROR,
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    const cpl_mask * bpm = cpl_image_get_bpm_const(a);

    if (bpm != NULL) {
        dmask = cpl_mask_get_data_const(bpm);
    }

    if (f == hdrl_elemop_div && b == 0.) {
        cpl_msg_warning(cpl_func, "dividing image by scalar zero");
        cpl_image_add_scalar(a, NAN);
        cpl_image_add_scalar(ae, NAN);
        cpl_image_reject_value(a, CPL_VALUE_NAN);
        cpl_image_reject_value(ae, CPL_VALUE_NAN);
        return cpl_error_get_code();
    }
    else {
        cpl_error_code err = f(hdrl_get_image_data(a),
                               hdrl_get_image_error(ae),
                               hdrl_get_image_npix(a),
                               &b, &be, 1,
                               dmask);
        if (f == hdrl_elemop_pow || f == hdrl_elemop_pow_inverted) {
            cpl_image_reject_value(a, CPL_VALUE_NAN);
            cpl_image_reject_from_mask(ae, cpl_image_get_bpm(a));
        }
        return err;
    }
}

/* doxygen in header */
cpl_error_code
hdrl_elemop_image_add_scalar(cpl_image * a, cpl_image * ae,
                             const hdrl_data_t b, const hdrl_error_t be)
{
    return hdrl_elemop_image_scalar(a, ae, b, be, hdrl_elemop_add);
}

cpl_error_code
hdrl_elemop_image_sub_scalar(cpl_image * a, cpl_image * ae,
                             const hdrl_data_t b, const hdrl_error_t be)
{
    return hdrl_elemop_image_scalar(a, ae, b, be, hdrl_elemop_sub);
}

cpl_error_code
hdrl_elemop_image_mul_scalar(cpl_image * a, cpl_image * ae,
                             const hdrl_data_t b, const hdrl_error_t be)
{
    return hdrl_elemop_image_scalar(a, ae, b, be, hdrl_elemop_mul);
}

cpl_error_code
hdrl_elemop_image_div_scalar(cpl_image * a, cpl_image * ae,
                             const hdrl_data_t b, const hdrl_error_t be)
{
    return hdrl_elemop_image_scalar(a, ae, b, be, hdrl_elemop_div);
}

cpl_error_code
hdrl_elemop_image_pow_scalar(cpl_image * a, cpl_image * ae,
                             const hdrl_data_t b, const hdrl_error_t be)
{
    return hdrl_elemop_image_scalar(a, ae, b, be, hdrl_elemop_pow);
}

cpl_error_code
hdrl_elemop_image_exp_scalar(cpl_image * a, cpl_image * ae,
                             const hdrl_data_t b, const hdrl_error_t be)
{
    return hdrl_elemop_image_scalar(a, ae, b, be, hdrl_elemop_pow_inverted);
}

static cpl_error_code
hdrl_elemop_image(cpl_image * a, cpl_image * ae,
                  const cpl_image * b, const cpl_image * be,
                  hdrl_math_op_f * f)
{
    cpl_error_code r = check_input(a, ae, b, be);
    const cpl_binary * dmask = NULL;
    cpl_ensure_code(r == CPL_ERROR_NONE, r);
    const cpl_mask * bbpm = cpl_image_get_bpm_const(b);

    if (bbpm != NULL) {
        if (cpl_image_get_bpm_const(a)) {
            /* a and b have bpm, merge them */
            cpl_mask * abpm = cpl_image_get_bpm(a);
            cpl_mask_or(abpm, bbpm);
            dmask = cpl_mask_get_data_const(abpm);
        }
        else {
            /* a has no bpm, copy from b */
            cpl_image_reject_from_mask(a, bbpm);
            dmask = cpl_mask_get_data_const(bbpm);
        }
    }
    else {
        const cpl_mask * abpm = cpl_image_get_bpm_const(a);
        if (abpm) {
            dmask = cpl_mask_get_data_const(abpm);
        }
    }

    r = f(hdrl_get_image_data(a),
          hdrl_get_image_error(ae),
          hdrl_get_image_npix(a),
          hdrl_get_image_data_const(b),
          hdrl_get_image_error_const(be),
          hdrl_get_image_npix(a),
          dmask);

    if (f == hdrl_elemop_div || f == hdrl_elemop_pow) {
        cpl_image_reject_value(a, CPL_VALUE_NAN);
        cpl_image_reject_from_mask(ae, cpl_image_get_bpm(a));
    }

    return r;
}

cpl_error_code
hdrl_elemop_image_add_image(cpl_image * a, cpl_image * ae,
                      const cpl_image * b, const cpl_image * be)
{
    return hdrl_elemop_image(a, ae, b, be, hdrl_elemop_add);
}

cpl_error_code
hdrl_elemop_image_sub_image(cpl_image * a, cpl_image * ae,
                      const cpl_image * b, const cpl_image * be)
{
    return hdrl_elemop_image(a, ae, b, be, hdrl_elemop_sub);
}

cpl_error_code
hdrl_elemop_image_mul_image(cpl_image * a, cpl_image * ae,
                      const cpl_image * b, const cpl_image * be)
{
    return hdrl_elemop_image(a, ae, b, be, hdrl_elemop_mul);
}

cpl_error_code
hdrl_elemop_image_div_image(cpl_image * a, cpl_image * ae,
                      const cpl_image * b, const cpl_image * be)
{
    return hdrl_elemop_image(a, ae, b, be, hdrl_elemop_div);
}

cpl_error_code
hdrl_elemop_image_pow_image(cpl_image * a, cpl_image * ae,
                      const cpl_image * b, const cpl_image * be)
{
    return hdrl_elemop_image(a, ae, b, be, hdrl_elemop_pow);
}

static cpl_error_code
hdrl_elemop_imagelist_vector(cpl_imagelist * self_d,
                             cpl_imagelist * self_e,
                             const cpl_vector * other_d,
                             const cpl_vector * other_e,
                             hdrl_math_op_image_scalar_f * f)
{
    cpl_ensure_code(self_d, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(self_e, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(other_d, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(other_e, CPL_ERROR_NULL_INPUT);
    const size_t nz = cpl_imagelist_get_size(self_d);
    cpl_ensure_code(nz == (size_t)cpl_imagelist_get_size(self_e),
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(nz == (size_t)cpl_vector_get_size(other_d),
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(nz == (size_t)cpl_vector_get_size(other_e),
                    CPL_ERROR_INCOMPATIBLE_INPUT);

    for (size_t i = 0; i < nz; i++) {
        cpl_image * ad = cpl_imagelist_get(self_d, i);
        cpl_image * ae = cpl_imagelist_get(self_e, i);
        const double bd = cpl_vector_get(other_d, i);
        const double be = cpl_vector_get(other_e, i);
        f(ad, ae, bd, be);
    }

    return cpl_error_get_code();
}

cpl_error_code
hdrl_elemop_imagelist_add_vector(cpl_imagelist * a,
                                 cpl_imagelist * ae,
                                 const cpl_vector * b,
                                 const cpl_vector * be)
{
    return hdrl_elemop_imagelist_vector(a, ae, b, be,
                                        hdrl_elemop_image_add_scalar);
}

cpl_error_code
hdrl_elemop_imagelist_sub_vector(cpl_imagelist * a,
                                 cpl_imagelist * ae,
                                 const cpl_vector * b,
                                 const cpl_vector * be)
{
    return hdrl_elemop_imagelist_vector(a, ae, b, be,
                                        hdrl_elemop_image_sub_scalar);
}

cpl_error_code
hdrl_elemop_imagelist_mul_vector(cpl_imagelist * a,
                                 cpl_imagelist * ae,
                                 const cpl_vector * b,
                                 const cpl_vector * be)
{
    return hdrl_elemop_imagelist_vector(a, ae, b, be,
                                        hdrl_elemop_image_mul_scalar);
}

cpl_error_code
hdrl_elemop_imagelist_div_vector(cpl_imagelist * a,
                                 cpl_imagelist * ae,
                                 const cpl_vector * b,
                                 const cpl_vector * be)
{
    return hdrl_elemop_imagelist_vector(a, ae, b, be,
                                        hdrl_elemop_image_div_scalar);
}

cpl_error_code
hdrl_elemop_imagelist_pow_vector(cpl_imagelist * a,
                                 cpl_imagelist * ae,
                                 const cpl_vector * b,
                                 const cpl_vector * be)
{
    return hdrl_elemop_imagelist_vector(a, ae, b, be,
                                        hdrl_elemop_image_pow_scalar);
}

static cpl_error_code
hdrl_elemop_imagelist_image(cpl_imagelist * self_d,
                            cpl_imagelist * self_e,
                            const cpl_image * other_d,
                            const cpl_image * other_e,
                            hdrl_math_op_image_f * f)
{
    cpl_ensure_code(self_d, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(self_e, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(other_d, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(other_e, CPL_ERROR_NULL_INPUT);
    const size_t nz = cpl_imagelist_get_size(self_d);
    cpl_ensure_code(nz == (size_t)cpl_imagelist_get_size(self_e),
                    CPL_ERROR_INCOMPATIBLE_INPUT);

    for (size_t i = 0; i < nz; i++) {
        cpl_image * ad = cpl_imagelist_get(self_d, i);
        cpl_image * ae = cpl_imagelist_get(self_e, i);
        f(ad, ae, other_d, other_e);
    }

    return cpl_error_get_code();
}

cpl_error_code
hdrl_elemop_imagelist_add_image(cpl_imagelist * a,
                                cpl_imagelist * ae,
                                const cpl_image * b,
                                const cpl_image * be)
{
    return hdrl_elemop_imagelist_image(a, ae, b, be,
                                       hdrl_elemop_image_add_image);
}

cpl_error_code
hdrl_elemop_imagelist_sub_image(cpl_imagelist * a,
                                cpl_imagelist * ae,
                                const cpl_image * b,
                                const cpl_image * be)
{
    return hdrl_elemop_imagelist_image(a, ae, b, be,
                                       hdrl_elemop_image_sub_image);
}

cpl_error_code
hdrl_elemop_imagelist_mul_image(cpl_imagelist * a,
                                cpl_imagelist * ae,
                                const cpl_image * b,
                                const cpl_image * be)
{
    return hdrl_elemop_imagelist_image(a, ae, b, be,
                                       hdrl_elemop_image_mul_image);
}

cpl_error_code
hdrl_elemop_imagelist_div_image(cpl_imagelist * a,
                                cpl_imagelist * ae,
                                const cpl_image * b,
                                const cpl_image * be)
{
    return hdrl_elemop_imagelist_image(a, ae, b, be,
                                       hdrl_elemop_image_div_image);
}

cpl_error_code
hdrl_elemop_imagelist_pow_image(cpl_imagelist * a,
                                cpl_imagelist * ae,
                                const cpl_image * b,
                                const cpl_image * be)
{
    return hdrl_elemop_imagelist_image(a, ae, b, be,
                                       hdrl_elemop_image_pow_image);
}

static cpl_error_code
hdrl_elemop_imagelist(cpl_imagelist * self_d,
                      cpl_imagelist * self_e,
                      const cpl_imagelist * other_d,
                      const cpl_imagelist * other_e,
                      hdrl_math_op_image_f * f)
{
    cpl_ensure_code(self_d, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(other_d, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(self_e, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(other_e, CPL_ERROR_NULL_INPUT);
    const size_t nz = cpl_imagelist_get_size(self_d);
    cpl_ensure_code(nz == (size_t)cpl_imagelist_get_size(self_e),
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(nz == (size_t)cpl_imagelist_get_size(other_d),
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_ensure_code(nz == (size_t)cpl_imagelist_get_size(other_e),
                    CPL_ERROR_INCOMPATIBLE_INPUT);

    for (size_t i = 0; i < nz; i++) {
        cpl_image * ad = cpl_imagelist_get(self_d, i);
        cpl_image * ae = cpl_imagelist_get(self_e, i);
        const cpl_image * bd = cpl_imagelist_get_const(other_d, i);
        const cpl_image * be = cpl_imagelist_get_const(other_e, i);
        f(ad, ae, bd, be);
    }

    return cpl_error_get_code();
}

cpl_error_code
hdrl_elemop_imagelist_add_imagelist(cpl_imagelist * a,
                                    cpl_imagelist * ae,
                                    const cpl_imagelist * b,
                                    const cpl_imagelist * be)
{
    return hdrl_elemop_imagelist(a, ae, b, be,
                                 hdrl_elemop_image_add_image);
}

cpl_error_code
hdrl_elemop_imagelist_sub_imagelist(cpl_imagelist * a,
                                    cpl_imagelist * ae,
                                    const cpl_imagelist * b,
                                    const cpl_imagelist * be)
{
    return hdrl_elemop_imagelist(a, ae, b, be,
                                 hdrl_elemop_image_sub_image);
}

cpl_error_code
hdrl_elemop_imagelist_mul_imagelist(cpl_imagelist * a,
                                    cpl_imagelist * ae,
                                    const cpl_imagelist * b,
                                    const cpl_imagelist * be)
{
    return hdrl_elemop_imagelist(a, ae, b, be,
                                 hdrl_elemop_image_mul_image);
}

cpl_error_code
hdrl_elemop_imagelist_div_imagelist(cpl_imagelist * a,
                                    cpl_imagelist * ae,
                                    const cpl_imagelist * b,
                                    const cpl_imagelist * be)
{
    return hdrl_elemop_imagelist(a, ae, b, be,
                                 hdrl_elemop_image_div_image);
}

cpl_error_code
hdrl_elemop_imagelist_pow_imagelist(cpl_imagelist * a,
                                    cpl_imagelist * ae,
                                    const cpl_imagelist * b,
                                    const cpl_imagelist * be)
{
    return hdrl_elemop_imagelist(a, ae, b, be,
                                 hdrl_elemop_image_pow_image);
}
