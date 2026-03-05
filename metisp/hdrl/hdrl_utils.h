/* $Id: hdrl_utils.h,v 1.32 2013-10-23 09:13:12 jtaylor Exp $
 *
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

/*
 * $Author: jtaylor $
 * $Date: 2013-10-23 09:13:12 $
 * $Revision: 1.32 $
 * $Name: not supported by cvs2svn $
 */

#ifndef HDRL_UTILS_H
#define HDRL_UTILS_H

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_parameter.h"
#include "hdrl_image.h"
#include "hdrl_imagelist.h"
#include <cpl.h>

#include <stdlib.h>
#include <stdarg.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @enum hdrl_direction
  @brief Define an image direction e.g along X or Y
 */
/*----------------------------------------------------------------------------*/
typedef enum {

    /** X axis, equivalent to NAXIS1 in FITS convention */ 
    HDRL_X_AXIS,
    /** Y axis, equivalent to NAXIS2 in FITS convention */ 
    HDRL_Y_AXIS,
    /** Reserved value for undefined direction */ 
    HDRL_UNDEFINED_AXIS
} hdrl_direction;

/*----------------------------------------------------------------------------*/
/**
  @enum hdrl_airmass_approx
  @brief Define the kind of airmass approximation
 */
/*----------------------------------------------------------------------------*/
typedef enum {
  HDRL_AIRMASS_APPROX_HARDIE = 1,		/* Hardie(1962)          */
  HDRL_AIRMASS_APPROX_YOUNG_IRVINE,		/* Young & Irvine (1967) */
  HDRL_AIRMASS_APPROX_YOUNG				/* Young(1994)           */
} hdrl_airmass_approx;

typedef enum {
  HDRL_IMAGE_EXTEND_NEAREST = 0,
  HDRL_IMAGE_EXTEND_MIRROR = 1,
} hdrl_image_extend_method ;

/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

const char * hdrl_get_license(void);

cpl_table * hdrl_eop_data_totable(const char * eop_data, cpl_size data_length);

/*----------------------------------------------------------------------------
                           Rect region Parameters
  ----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_rect_region_parameter_create(cpl_size, cpl_size,
        cpl_size, cpl_size) ;
cpl_error_code hdrl_rect_region_parameter_update(hdrl_parameter *, cpl_size, 
        cpl_size, cpl_size, cpl_size) ;
cpl_boolean hdrl_rect_region_parameter_check(const hdrl_parameter *) ;
cpl_size hdrl_rect_region_get_llx(const hdrl_parameter *) ;
cpl_size hdrl_rect_region_get_lly(const hdrl_parameter *) ;
cpl_size hdrl_rect_region_get_urx(const hdrl_parameter *) ;
cpl_size hdrl_rect_region_get_ury(const hdrl_parameter *) ;
cpl_error_code hdrl_rect_region_parameter_verify(const hdrl_parameter *,
        const cpl_size, const cpl_size) ;
cpl_parameterlist * hdrl_rect_region_parameter_create_parlist(const char *, 
        const char *, const char *, const hdrl_parameter *) ;
hdrl_parameter * hdrl_rect_region_parameter_parse_parlist(
        const cpl_parameterlist *, const char *, const char *) ;
cpl_boolean hdrl_is_strictly_monotonic_increasing(const double * x, cpl_size l);
void hdrl_sort_on_x(double * x, double * y1, double * y2,
        const cpl_size sample_len, const cpl_boolean sort_decreasing);

/*-----------------------------------------------------------------------------
  Experimental declarations - can be used, but no guarantees on api stability
 -----------------------------------------------------------------------------*/
#if defined HDRL_USE_EXPERIMENTAL || defined HDRL_USE_PRIVATE
typedef enum {
    HDRL_SCALE_ADDITIVE,
    HDRL_SCALE_MULTIPLICATIVE
} hdrl_scale_type;

cpl_error_code
hdrl_normalize_imagelist_by_vector(const cpl_vector      * scale,
                                   const cpl_vector      * scale_e,
								   const hdrl_scale_type   scale_type,
                                   cpl_imagelist         * data,
                                   cpl_imagelist         * errors);

cpl_error_code
hdrl_normalize_imagelist_by_imagelist(const cpl_imagelist * scale,
                                      const cpl_imagelist * scale_e,
                                      const hdrl_scale_type scale_type,
                                      cpl_imagelist * data,
                                      cpl_imagelist * errors);

int hdrl_get_tempfile(const char * dir, cpl_boolean unlink);
char * hdrl_get_cwd(void);
cpl_error_code hdrl_rect_region_fix_negatives(hdrl_parameter *, const cpl_size,
                                              const cpl_size) ;

cpl_error_code
hdrl_wcs_convert(const cpl_wcs *wcs, const cpl_matrix *from,
                 cpl_matrix **to, cpl_array **status,
                 cpl_wcs_trans_mode transform);
#endif

/* --------------- *
 * Airmass funcion *
 * --------------- */
hdrl_value hdrl_utils_airmass(
	hdrl_value aRA, hdrl_value aDEC, hdrl_value aLST,
	hdrl_value aExptime, hdrl_value aLatitude,
	hdrl_airmass_approx type);

/* must not be followed by a semicolon!
 * gcc on mac has omp but it doesn't work for nontrivial cases as libc lacks
 * alloca */
#if defined (_OPENMP) && !defined( __APPLE__)
  #define HDRL_OMP(x) _Pragma (#x)
#else
  #define HDRL_OMP(x)
#endif

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
 -----------------------------------------------------------------------------*/
#ifdef HDRL_USE_PRIVATE
cpl_matrix *
hdrl_maglim_kernel_create(const cpl_size kernel_sx, const cpl_size kernel_sy,
		          const double fwhm);
cpl_image*
hdrl_extend_image(const cpl_image* image, const cpl_size border_nx,
		  const cpl_size border_ny,
		  const hdrl_image_extend_method image_extend_method);

cpl_image *
hdrl_image_convolve(const cpl_image * input_image, const cpl_matrix * kernel,
		    const hdrl_image_extend_method image_extend_method);

/* Private functions for airmass calculations */
hdrl_value hdrl_get_zenith_distance(
	hdrl_value aHourAngle, hdrl_value aDelta, hdrl_value aLatitude);

hdrl_value hdrl_get_airmass_hardie(      hdrl_value hvaSecZ);
hdrl_value hdrl_get_airmass_youngirvine( hdrl_value hvaSecZ);
hdrl_value hdrl_get_airmass_young(       hdrl_value hvaCosZt);
/* ---------------------- */


cpl_image *
hdrl_medianfilter_image_grid(const cpl_image * ima, cpl_matrix * x, cpl_matrix * y,
                             cpl_size  filtersize_x, cpl_size filtersize_y);

cpl_matrix * hdrl_matrix_linspace(cpl_size start, cpl_size stop, cpl_size step);

cpl_matrix * hdrl_fit_legendre(cpl_image * img, int order_x, int order_y,
                               cpl_matrix * grid_x, cpl_matrix * grid_y,
                               cpl_size orig_nx, cpl_size  orig_ny);

cpl_image * hdrl_legendre_to_image(cpl_matrix * coeffs, int order_x,
                                   int order_y, cpl_size nx, cpl_size ny);

int hdrl_check_maskequality(const cpl_mask * mask1, const cpl_mask * mask2);

typedef struct hdrl_vector_cache_ hdrl_vector_cache;

hdrl_vector_cache * hdrl_vector_cache_new(cpl_size max_cached_size,
		                                  cpl_size ncached_entries);
void hdrl_vector_cache_delete(hdrl_vector_cache * cache);
cpl_vector * hdrl_cplvector_new_from_cache(hdrl_vector_cache * cache, cpl_size sz);
void hdrl_cplvector_delete_to_cache(hdrl_vector_cache * cache, cpl_vector * v);

/* setup a value parameter and append it into parlist */
#define hdrl_setup_vparameter(parlist, \
                              prefix, \
                              sep, \
                              name_prefix, \
                              pname, \
                              context, \
                              descr, \
                              type, \
                              pdefault) \
do { \
        char * fname = cpl_sprintf("%s%s", name_prefix, pname); \
        char * setup_name = hdrl_join_string(sep, 3, context, prefix, fname); \
        cpl_parameter * setup_p = cpl_parameter_new_value(setup_name, type, \
                descr, context, pdefault) ; \
        cpl_free(setup_name); \
        setup_name = hdrl_join_string(sep, 2, prefix, fname); \
        cpl_parameter_set_alias(setup_p, CPL_PARAMETER_MODE_CLI, setup_name); \
        cpl_parameter_disable(setup_p, CPL_PARAMETER_MODE_ENV); \
        cpl_free(setup_name); \
        cpl_free(fname); \
        cpl_parameterlist_append(parlist, setup_p); \
    } while (0)

cpl_vector * hdrl_image_to_vector(const cpl_image * source, const
        cpl_mask * bpm);
cpl_vector * hdrl_imagelist_to_vector(const cpl_imagelist * list,
        const cpl_size x, const cpl_size y);
cpl_error_code hdrl_imagelist_to_vector_row(const cpl_imagelist * list,
                                            const cpl_size y,
                                            cpl_vector ** out,
                                            hdrl_vector_cache * cache);

cpl_error_code
hdrl_imagelist_to_cplwrap(const hdrl_imagelist * list,
                          cpl_imagelist ** data,
                          cpl_imagelist ** errs);

cpl_image *
hdrl_parallel_filter_image(const cpl_image * img,
                           const cpl_matrix * kernel,
                           const cpl_mask * mask,
                           const cpl_filter_mode mode);

cpl_mask * hcpl_image_set_bpm(cpl_image * self, cpl_mask * bpm) ;
double hcpl_vector_get_mad_window(cpl_vector * vec,
                                  cpl_size llx,
                                  cpl_size urx,
                                  double * sigma);
double hcpl_gaussian_eval_2d(const cpl_array * self, double x, double y);

static inline int hdrl_int_is_power_of_two(unsigned long long x)
{
    return (x & (x - 1)) == 0;
}

static inline size_t hdrl_get_image_npix(const cpl_image * img)
{
    return cpl_image_get_size_x(img) * cpl_image_get_size_y(img);
}

static inline size_t hdrl_get_image_good_npix(const cpl_image * img)
{
    return (cpl_image_get_size_x(img) * cpl_image_get_size_y(img)) -
        cpl_image_count_rejected(img);
}

static inline cpl_mask * hdrl_copy_image_mask(const cpl_image * img)
{
    /* always returns a mask even it image has none */
    const cpl_mask * bpm = cpl_image_get_bpm_const(img);

    if (bpm) {
        return cpl_mask_duplicate(bpm);
    }
    else {
        return cpl_mask_new(cpl_image_get_size_x(img),
                            cpl_image_get_size_y(img));
    }
}

char * hdrl_join_string(const char * sep_, int n, ...);

static inline hdrl_data_t * hdrl_get_image_data(cpl_image * image)
{
#if HDRL_SIZEOF_DATA == 8
    return cpl_image_get_data_double(image);
#else
    return cpl_image_get_data_float(image);
#endif
}

static inline cpl_error_code
hdrl_wrap_table(cpl_table * tb, hdrl_data_t * data, const char * name)
{
#if HDRL_SIZEOF_DATA == 8
    return cpl_table_wrap_double(tb, data, name);
#else
    return cpl_table_wrap_float(tb, data, name);
#endif
}

static inline hdrl_error_t * hdrl_get_image_error(cpl_image * image)
{
#if HDRL_SIZEOF_ERROR == 8
    return cpl_image_get_data_double(image);
#else
    return cpl_image_get_data_float(image);
#endif
}


static inline const
hdrl_data_t * hdrl_get_image_data_const(const cpl_image * image)
{
#if HDRL_SIZEOF_DATA == 8
    return cpl_image_get_data_double_const(image);
#else
    return cpl_image_get_data_float_const(image);
#endif
}

static inline const
hdrl_error_t * hdrl_get_image_error_const(const cpl_image * image)
{
#if HDRL_SIZEOF_ERROR == 8
    return cpl_image_get_data_double_const(image);
#else
    return cpl_image_get_data_float_const(image);
#endif
}

#ifndef HDRL_ATTR_UNUSED
    #if defined(__GNUC__) || defined(__ICC) || defined(__clang__)
        #define HDRL_ATTR_UNUSED __attribute__ ((__unused__))
    #else
        #define HDRL_ATTR_UNUSED
    #endif
#endif

/* Use this to tag a variable as not used. It will remove unused variable
 * warning and mangle the variable to avoid accidental use */
#define HDRL_UNUSED(x) (__HDRL_UNUSED_TAGGED ## x) HDRL_ATTR_UNUSED

#endif



CPL_END_DECLS

#endif
