/* $Id: hdrl_elemop.h,v 1.4 2013-10-17 15:44:14 jtaylor Exp $
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
 * $Date: 2013-10-17 15:44:14 $
 * $Revision: 1.4 $
 */

#ifndef HDRL_ELEMOP_H
#define HDRL_ELEMOP_H

#ifndef HDRL_USE_PRIVATE
#error This file is not allowed to be included outside of hdrl
#endif

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_types.h"
#include <cpl.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief add/subtract/multiply/divide two images with error propagation
 *
 * @param a   image, modified in place
 * @param ae  errors of image a, modified in place
 * @param b   image
 * @param be  errors of image b
 *
 * Gaussian error propagation of first order, no accounting correlation
 * besides a == b.
 * Divisions by zero will be marked as bad pixels and set to NAN
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code
hdrl_elemop_image_add_image(cpl_image * a, cpl_image * ae,
                            const cpl_image * b, const cpl_image * be);
cpl_error_code
hdrl_elemop_image_sub_image(cpl_image * a, cpl_image * ae,
                            const cpl_image * b, const cpl_image * be);
cpl_error_code
hdrl_elemop_image_mul_image(cpl_image * a, cpl_image * ae,
                            const cpl_image * b, const cpl_image * be);
cpl_error_code
hdrl_elemop_image_div_image(cpl_image * a, cpl_image * ae,
                            const cpl_image * b, const cpl_image * be);
cpl_error_code
hdrl_elemop_image_pow_image(cpl_image * a, cpl_image * ae,
                            const cpl_image * b, const cpl_image * be);

/* ---------------------------------------------------------------------------*/
/**
 * @brief add/subtract/multiply/divide image and scalar with error propagation
 *
 * @param a   image, modified in place
 * @param ae  errors of image a, modified in place
 * @param b   scalar
 * @param be  error of scalar b
 *
 * Gaussian error propagation of first order, no accounting correlation
 * besides a == b.
 * Divisions by zero will be marked as bad pixels and set to NAN
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code
hdrl_elemop_image_add_scalar(cpl_image * a, cpl_image * ae,
                          const hdrl_data_t b, const hdrl_error_t be);
cpl_error_code
hdrl_elemop_image_sub_scalar(cpl_image * a, cpl_image * ae,
                          const hdrl_data_t b, const hdrl_error_t be);
cpl_error_code
hdrl_elemop_image_mul_scalar(cpl_image * a, cpl_image * ae,
                          const hdrl_data_t b, const hdrl_error_t be);
cpl_error_code
hdrl_elemop_image_div_scalar(cpl_image * a, cpl_image * ae,
                          const hdrl_data_t b, const hdrl_error_t be);
cpl_error_code
hdrl_elemop_image_pow_scalar(cpl_image * a, cpl_image * ae,
                          const hdrl_data_t b, const hdrl_error_t be);
cpl_error_code
hdrl_elemop_image_exp_scalar(cpl_image * a, cpl_image * ae,
                          const hdrl_data_t b, const hdrl_error_t be);
/* ---------------------------------------------------------------------------*/
/**
 * @brief add/subtract/multiply/divide two imagelists and scalar with error
 *        propagation
 *
 * @param a   imagelist, modified in place
 * @param ae  errors of imagelist a, modified in place
 * @param b   imagelist
 * @param be  error of imagelist b
 *
 * Gaussian error propagation of first order, no accounting correlation
 * besides a == b.
 * Divisions by zero will be marked as bad pixels and set to NAN
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code
hdrl_elemop_imagelist_add_imagelist(cpl_imagelist * a, cpl_imagelist * ae,
                   const cpl_imagelist * b, const cpl_imagelist * be);
cpl_error_code
hdrl_elemop_imagelist_sub_imagelist(cpl_imagelist * a, cpl_imagelist * ae,
                   const cpl_imagelist * b, const cpl_imagelist * be);
cpl_error_code
hdrl_elemop_imagelist_mul_imagelist(cpl_imagelist * a, cpl_imagelist * ae,
                   const cpl_imagelist * b, const cpl_imagelist * be);
cpl_error_code
hdrl_elemop_imagelist_div_imagelist(cpl_imagelist * a, cpl_imagelist * ae,
                   const cpl_imagelist * b, const cpl_imagelist * be);
cpl_error_code
hdrl_elemop_imagelist_pow_imagelist(cpl_imagelist * a, cpl_imagelist * ae,
                   const cpl_imagelist * b, const cpl_imagelist * be);

/* ---------------------------------------------------------------------------*/
/**
 * @brief add/subtract/multiply/divide each image of an imagelist and a
 *        scalar from a vector with error propagation
 *
 * @param a   imagelist, modified in place
 * @param ae  errors of imagelist a, modified in place
 * @param b   vector
 * @param be  error of vector b
 *
 * Gaussian error propagation of first order, no accounting correlation
 * besides a == b.
 * Divisions by zero will be marked as bad pixels and set to NAN
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code
hdrl_elemop_imagelist_add_vector(cpl_imagelist * a, cpl_imagelist * ae,
                              const cpl_vector * b, const cpl_vector * be);
cpl_error_code
hdrl_elemop_imagelist_sub_vector(cpl_imagelist * a, cpl_imagelist * ae,
                              const cpl_vector * b, const cpl_vector * be);
cpl_error_code
hdrl_elemop_imagelist_mul_vector(cpl_imagelist * a, cpl_imagelist * ae,
                              const cpl_vector * b, const cpl_vector * be);
cpl_error_code
hdrl_elemop_imagelist_div_vector(cpl_imagelist * a, cpl_imagelist * ae,
                              const cpl_vector * b, const cpl_vector * be);
cpl_error_code
hdrl_elemop_imagelist_pow_vector(cpl_imagelist * a, cpl_imagelist * ae,
                              const cpl_vector * b, const cpl_vector * be);

/* ---------------------------------------------------------------------------*/
/**
 * @brief add/subtract/multiply/divide each image of an imagelist and an image
 *        with error propagation
 *
 * @param a   imagelist, modified in place
 * @param ae  errors of imagelist a, modified in place
 * @param b   image
 * @param be  error of image b
 *
 * Gaussian error propagation of first order, no accounting correlation
 * besides a == b.
 * Divisions by zero will be marked as bad pixels and set to NAN
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code
hdrl_elemop_imagelist_add_image(cpl_imagelist * a, cpl_imagelist * ae,
                             const cpl_image * b, const cpl_image * be);
cpl_error_code
hdrl_elemop_imagelist_sub_image(cpl_imagelist * a, cpl_imagelist * ae,
                             const cpl_image * b, const cpl_image * be);
cpl_error_code
hdrl_elemop_imagelist_mul_image(cpl_imagelist * a, cpl_imagelist * ae,
                             const cpl_image * b, const cpl_image * be);
cpl_error_code
hdrl_elemop_imagelist_div_image(cpl_imagelist * a, cpl_imagelist * ae,
                             const cpl_image * b, const cpl_image * be);
cpl_error_code
hdrl_elemop_imagelist_pow_image(cpl_imagelist * a, cpl_imagelist * ae,
                             const cpl_image * b, const cpl_image * be);

/* see hdrl_elemop.c for doc strings */
cpl_error_code hdrl_elemop_add(hdrl_data_t * a, hdrl_error_t * ea, size_t na,
                            const hdrl_data_t * b, const hdrl_error_t * eb, size_t nb,
                            const cpl_binary * mask);
cpl_error_code hdrl_elemop_sub(hdrl_data_t * a, hdrl_error_t * ea, size_t na,
                            const hdrl_data_t * b, const hdrl_error_t * eb, size_t nb,
                            const cpl_binary * mask);
cpl_error_code hdrl_elemop_mul(hdrl_data_t * a, hdrl_error_t * ea, size_t na,
                            const hdrl_data_t * b, const hdrl_error_t * eb, size_t nb,
                            const cpl_binary * mask);
cpl_error_code hdrl_elemop_div(hdrl_data_t * a, hdrl_error_t * ea, size_t na,
                            const hdrl_data_t * b, const hdrl_error_t * eb, size_t nb,
                            const cpl_binary * mask);
cpl_error_code hdrl_elemop_pow(hdrl_data_t * a, hdrl_error_t * ea, size_t na,
                            const hdrl_data_t * b, const hdrl_error_t * eb, size_t nb,
                            const cpl_binary * mask);
cpl_error_code hdrl_elemop_pow_inverted(
                                hdrl_data_t * a, hdrl_error_t * ea, size_t na,
                            const hdrl_data_t * b, const hdrl_error_t * eb, size_t nb,
                            const cpl_binary * mask);

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
 -----------------------------------------------------------------------------*/

#ifdef HDRL_USE_PRIVATE

#endif

CPL_END_DECLS

#endif
