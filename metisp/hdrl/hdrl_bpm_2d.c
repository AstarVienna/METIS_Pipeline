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

#include "hdrl_types.h"
#include "hdrl_image.h"
#include "hdrl_imagelist.h"
#include "hdrl_utils.h"

#include "hdrl_bpm_2d.h"
#include "hdrl_prototyping.h"

#include <cpl.h>
#include <string.h>
#include <math.h>

/*-----------------------------------------------------------------------------
                                   Static
 -----------------------------------------------------------------------------*/


/**
 * @defgroup hdrl_bpm Bad Pixel Detection
 *
 * @brief
 *  This module contains functions to detect bad pixels on single images, on a
 *  stack of identical images and on a sequence of images.
 *
 */

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_bpm_2d     Bad Pixel Mask 2D Computation
  @ingroup hdrl_bpm

 @brief
   Algorithms to detect bad pixels on a single image

 The routines in this module can be used to detect bad pixels on a single image.
 The algorithm first smoothes the image by applying different methods.
 Then it subtracts the smoothed image and derives bad
 pixels by thresholding the residual image, i.e. all pixels exceeding
 the threshold are considered as bad.

 The calculation is performed by calling the top-level function
 hdrl_bpm_2d_compute() and the parameters passed to this function can be created
 by calling hdrl_bpm_2d_parameter_create_filtersmooth() \b or
 hdrl_bpm_2d_parameter_create_legendresmooth(), depending on the method one
 would like to use.

 */
/*----------------------------------------------------------------------------*/

/**@{*/


/** @cond PRIVATE */

static cpl_image * hdrl_get_residuals_filtersmooth(cpl_size, cpl_size,
        cpl_filter_mode, cpl_border_mode, cpl_image *, cpl_mask *);
static cpl_image * hdrl_get_residuals_legendresmooth(const cpl_image *, int,
        int, int, int, int, int) ;

/*-----------------------------------------------------------------------------
                        BPM Parameters Definition
 -----------------------------------------------------------------------------*/
typedef struct {
    HDRL_PARAMETER_HEAD;
    cpl_filter_mode     filter ;
    cpl_border_mode     border ;
    double              kappa_low ;
    double              kappa_high ;
    int                 maxiter ;
    int                 steps_x ;
    int                 steps_y ;
    int                 filter_size_x ;
    int                 filter_size_y ;
    int                 order_x ;
    int                 order_y ;
    int                 smooth_x ;
    int                 smooth_y ;
    hdrl_bpm_2d_method  method ;
} hdrl_bpm_2d_parameter;

/* Parameter type */
static hdrl_parameter_typeobj hdrl_bpm_2d_parameter_type = {
    HDRL_PARAMETER_BPM_2D,                  /* type */
    (hdrl_alloc *)&cpl_malloc,              /* fp_alloc */
    (hdrl_free *)&cpl_free,                 /* fp_free */
    NULL,                                   /* fp_destroy */
    sizeof(hdrl_bpm_2d_parameter),          /* obj_size */
};
/** @endcond */

/*----------------------------------------------------------------------------*/
/**
  @brief    Creates BPM_2D Parameters object for HDRL_BPM_2D_FILTERSMOOTH
  @param    kappa_low       Low kappa factor for thresholding algorithm
  @param    kappa_high      High kappa factor for thresholding algorithm
  @param    maxiter         Maximum number of iterations
  @param    filter          filter mode
  @param    border          border mode
  @param    smooth_x        Smoothing kernel X size
  @param    smooth_y        Smoothing kernel Y size
  @return   The BPM_2D parameters object. It needs to be deallocated with
            hdrl_parameter_delete()

  The method creates a parameter for the method HDRL_BPM_2D_FILTERSMOOTH

  @see      hdrl_parameter_delete()
  @see      hdrl_bpm_2d_compute()
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_bpm_2d_parameter_create_filtersmooth(
        double              kappa_low,
        double              kappa_high,
        int                 maxiter,
        cpl_filter_mode     filter,
        cpl_border_mode     border,
        int                 smooth_x,
        int                 smooth_y)
{
    hdrl_bpm_2d_parameter * p = (hdrl_bpm_2d_parameter *)
               hdrl_parameter_new(&hdrl_bpm_2d_parameter_type);
    p->kappa_low = kappa_low ;
    p->kappa_high = kappa_high ;
    p->maxiter = maxiter ;
    p->filter = filter ;
    p->border = border ;
    p->smooth_x = smooth_x ;
    p->smooth_y = smooth_y ;
    p->steps_x = 0 ;
    p->steps_y = 0 ;
    p->filter_size_x = 0 ;
    p->filter_size_y = 0 ;
    p->order_x = 0 ;
    p->order_y = 0 ;
    p->method = HDRL_BPM_2D_FILTERSMOOTH ;

    if (hdrl_bpm_2d_parameter_verify((hdrl_parameter*)p) != CPL_ERROR_NONE) {
        cpl_free(p);
        return NULL;
    }
    return (hdrl_parameter *)p;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Creates BPM_2D Parameters object for HDRL_BPM_2D_LEGENDRESMOOTH
  @param    kappa_low       Low kappa factor for thresholding algorithm
  @param    kappa_high      High kappa factor for thresholding algorithm
  @param    maxiter         Maximum number of iterations
  @param    steps_x         Number of sampling coordinates in x-dir
  @param    steps_y         Number of sampling coordinates in y-dir
  @param    filter_size_x   size of the median box in x-dir
  @param    filter_size_y   size of the median box in y-dir
  @param    order_x         order of polynomial in x-dir
  @param    order_y         order of polynomial in y-dir
  @return   The BPM_2D parameters object. It needs to be deallocated with
            hdrl_parameter_delete().

  The method creates a hdrl parameter for the method HDRL_BPM_2D_LEGENDRESMOOTH

  @see      hdrl_parameter_delete()
  @see      hdrl_bpm_2d_compute()
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_bpm_2d_parameter_create_legendresmooth(
        double              kappa_low,
        double              kappa_high,
        int                 maxiter,
        int                 steps_x,
        int                 steps_y,
        int                 filter_size_x,
        int                 filter_size_y,
        int                 order_x,
        int                 order_y)
{
    hdrl_bpm_2d_parameter * p = (hdrl_bpm_2d_parameter *)
               hdrl_parameter_new(&hdrl_bpm_2d_parameter_type);
    p->kappa_low = kappa_low ;
    p->kappa_high = kappa_high ;
    p->maxiter = maxiter ;
    p->filter = CPL_FILTER_MEDIAN ;
    p->border = CPL_BORDER_FILTER ;
    p->smooth_x = 0 ;
    p->smooth_y = 0 ;
    p->steps_x = steps_x ;
    p->steps_y = steps_y ;
    p->filter_size_x = filter_size_x ;
    p->filter_size_y = filter_size_y ;
    p->order_x = order_x ;
    p->order_y = order_y ;
    p->method = HDRL_BPM_2D_LEGENDRESMOOTH ;
    if (hdrl_bpm_2d_parameter_verify((hdrl_parameter*)p) != CPL_ERROR_NONE) {
        cpl_free(p);
        return NULL;
    }
    return (hdrl_parameter *)p;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Verify basic correctness of the BPM_2D parameters
  @param    param   BPM_2D parameters
  @return   CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_bpm_2d_parameter_verify(
        const hdrl_parameter    *   param)
{
    const hdrl_bpm_2d_parameter * param_loc = (const hdrl_bpm_2d_parameter *)param ;

    cpl_error_ensure(param != NULL, CPL_ERROR_NULL_INPUT,
            return CPL_ERROR_NULL_INPUT, "NULL Input Parameters");
    cpl_error_ensure(hdrl_bpm_2d_parameter_check(param),
            CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
            "Expected BPM_2d parameter") ;

    cpl_error_ensure(param_loc->method == HDRL_BPM_2D_LEGENDRESMOOTH || 
            param_loc->method == HDRL_BPM_2D_FILTERSMOOTH, 
            CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT, 
            "Unsupported method");
    
    switch (param_loc->method) {
    case HDRL_BPM_2D_FILTERSMOOTH:
        cpl_error_ensure(param_loc->smooth_x >= 0, CPL_ERROR_ILLEGAL_INPUT,
                        return CPL_ERROR_ILLEGAL_INPUT, "smooth-x must be >=0");
        cpl_error_ensure(param_loc->smooth_y >= 0, CPL_ERROR_ILLEGAL_INPUT,
                         return CPL_ERROR_ILLEGAL_INPUT, "smooth-y must be >=0");
        /* Only odd-sized kernel are allowed */
        cpl_error_ensure(((param_loc->smooth_x)&1) == 1, CPL_ERROR_ILLEGAL_INPUT,
                         return CPL_ERROR_ILLEGAL_INPUT, "smooth-x must be odd");
        cpl_error_ensure(((param_loc->smooth_y)&1) == 1, CPL_ERROR_ILLEGAL_INPUT,
                         return CPL_ERROR_ILLEGAL_INPUT, "smooth-y must be odd");
        if (param_loc->filter != CPL_FILTER_AVERAGE &&
            param_loc->filter != CPL_FILTER_AVERAGE_FAST &&
            param_loc->filter != CPL_FILTER_MEDIAN) {
            cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                          "Function only supports filters: "
                          "CPL_FILTER_AVERAGE, CPL_FILTER_AVERAGE_FAST "
                          "and CPL_FILTER_MEDIAN");
            return CPL_ERROR_ILLEGAL_INPUT;
        }
        break ;
    case HDRL_BPM_2D_LEGENDRESMOOTH:
        cpl_error_ensure(param_loc->order_x >= 0, CPL_ERROR_ILLEGAL_INPUT,
                        return CPL_ERROR_ILLEGAL_INPUT, "order-x must be >= 0");
        cpl_error_ensure(param_loc->order_y >= 0, CPL_ERROR_ILLEGAL_INPUT,
                         return CPL_ERROR_ILLEGAL_INPUT, "order-y must be >= 0");
        cpl_error_ensure(param_loc->steps_x > param_loc->order_x, CPL_ERROR_ILLEGAL_INPUT,
                         return CPL_ERROR_ILLEGAL_INPUT, "stepx_x must be > order-x");
        cpl_error_ensure(param_loc->steps_y > param_loc->order_y, CPL_ERROR_ILLEGAL_INPUT,
                         return CPL_ERROR_ILLEGAL_INPUT, "stepx_y must be > order-y");
        cpl_error_ensure(param_loc->filter_size_x > 0, CPL_ERROR_ILLEGAL_INPUT,
                         return CPL_ERROR_ILLEGAL_INPUT, "filter-size-x must be > 0");
        cpl_error_ensure(param_loc->filter_size_y > 0, CPL_ERROR_ILLEGAL_INPUT,
                         return CPL_ERROR_ILLEGAL_INPUT, "filter-size-y must be > 0");
        break ;
    }

    cpl_error_ensure(param_loc->kappa_low >= 0, CPL_ERROR_ILLEGAL_INPUT,
            return CPL_ERROR_ILLEGAL_INPUT, "kappa-low must be >=0");
    cpl_error_ensure(param_loc->kappa_high >= 0, CPL_ERROR_ILLEGAL_INPUT,
            return CPL_ERROR_ILLEGAL_INPUT, "kappa-high must be >=0");
    cpl_error_ensure(param_loc->maxiter >= 0, CPL_ERROR_ILLEGAL_INPUT,
            return CPL_ERROR_ILLEGAL_INPUT, "maxiter must be >=0");
    return CPL_ERROR_NONE ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Check that the parameter is a BPM_2D parameter
  @param    self The parameter to check
  @return   True or False
 */
/*----------------------------------------------------------------------------*/
cpl_boolean hdrl_bpm_2d_parameter_check(const hdrl_parameter * self)
{
    return hdrl_parameter_check_type(self, &hdrl_bpm_2d_parameter_type);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the filter in the BPM_2D parameter
  @param    p   The BPM_2D parameter
  @return   The filter
 */
/*----------------------------------------------------------------------------*/
cpl_filter_mode hdrl_bpm_2d_parameter_get_filter(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, CPL_FILTER_EROSION);
    return p != NULL ? ((const hdrl_bpm_2d_parameter *)p)->filter : CPL_FILTER_EROSION;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the border in the BPM_2D parameter
  @param    p   The BPM_2D parameter
  @return   The border
 */
/*----------------------------------------------------------------------------*/
cpl_border_mode hdrl_bpm_2d_parameter_get_border(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, CPL_BORDER_FILTER);
    return p != NULL ? ((const hdrl_bpm_2d_parameter *)p)->border : CPL_BORDER_FILTER;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the kappa_low in the BPM_2D parameter
  @param    p   The BPM_2D parameter
  @return   The kappa_low
 */
/*----------------------------------------------------------------------------*/
double hdrl_bpm_2d_parameter_get_kappa_low(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.0);
    return p != NULL ? ((const hdrl_bpm_2d_parameter *)p)->kappa_low : 0.;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the kappa_high in the BPM_2D parameter
  @param    p   The BPM_2D parameter
  @return   The kappa_high
 */
/*----------------------------------------------------------------------------*/
double hdrl_bpm_2d_parameter_get_kappa_high(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.0);
    return p != NULL ? ((const hdrl_bpm_2d_parameter *)p)->kappa_high : 0.;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the maxiter in the BPM_2D parameter
  @param    p   The BPM_2D parameter
  @return   The maxiter
 */
/*----------------------------------------------------------------------------*/
int hdrl_bpm_2d_parameter_get_maxiter(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    return p != NULL ? ((const hdrl_bpm_2d_parameter *)p)->maxiter : 0;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the steps_x in the BPM_2D parameter
  @param    p   The BPM_2D parameter
  @return   The steps_x
 */
/*----------------------------------------------------------------------------*/
int hdrl_bpm_2d_parameter_get_steps_x(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    return p != NULL ? ((const hdrl_bpm_2d_parameter *)p)->steps_x : 0;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the steps_y in the BPM_2D parameter
  @param    p   The BPM_2D parameter
  @return   The steps_y
 */
/*----------------------------------------------------------------------------*/
int hdrl_bpm_2d_parameter_get_steps_y(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    return p != NULL ? ((const hdrl_bpm_2d_parameter *)p)->steps_y : 0;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the filter_size_x in the BPM_2D parameter
  @param    p   The BPM_2D parameter
  @return   The filter_size_x
 */
/*----------------------------------------------------------------------------*/
int hdrl_bpm_2d_parameter_get_filter_size_x(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    return p != NULL ? ((const hdrl_bpm_2d_parameter *)p)->filter_size_x : 0;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the filter_size_y in the BPM_2D parameter
  @param    p   The BPM_2D parameter
  @return   The filter_size_y
 */
/*----------------------------------------------------------------------------*/
int hdrl_bpm_2d_parameter_get_filter_size_y(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    return p != NULL ? ((const hdrl_bpm_2d_parameter *)p)->filter_size_y : 0;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the order_x in the BPM_2D parameter
  @param    p   The BPM_2D parameter
  @return   The order_x
 */
/*----------------------------------------------------------------------------*/
int hdrl_bpm_2d_parameter_get_order_x(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    return p != NULL ? ((const hdrl_bpm_2d_parameter *)p)->order_x : 0;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the order_y in the BPM_2D parameter
  @param    p   The BPM_2D parameter
  @return   The order_y
 */
/*----------------------------------------------------------------------------*/
int hdrl_bpm_2d_parameter_get_order_y(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    return p != NULL ? ((const hdrl_bpm_2d_parameter *)p)->order_y : 0;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the smooth_y in the BPM_2D parameter
  @param    p   The BPM_2D parameter
  @return   The smooth_y
 */
/*----------------------------------------------------------------------------*/
int hdrl_bpm_2d_parameter_get_smooth_y(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    return p != NULL ? ((const hdrl_bpm_2d_parameter *)p)->smooth_y : 0;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the smooth_x in the BPM_2D parameter
  @param    p   The BPM_2D parameter
  @return   The smooth_x
 */
/*----------------------------------------------------------------------------*/
int hdrl_bpm_2d_parameter_get_smooth_x(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    return p != NULL ? ((const hdrl_bpm_2d_parameter *)p)->smooth_x : 0;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the method in the BPM_2D parameter
  @param    p   The BPM_2D parameter
  @return   The method
 */
/*----------------------------------------------------------------------------*/
hdrl_bpm_2d_method hdrl_bpm_2d_parameter_get_method(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, HDRL_BPM_2D_LEGENDRESMOOTH);
    return p != NULL ? ((const hdrl_bpm_2d_parameter *)p)->method : HDRL_BPM_2D_LEGENDRESMOOTH;
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief Create parameter list for the BPM_2D legendresmooth computation
  @param base_context    base context of parameter (e.g. recipe name)
  @param prefix          prefix of parameter, may be an empty string
  @param deflt           defaults
  Creates a parameter list with the BPM_2D parameters:
    - base_context.prefix.method
    - base_context.prefix.legendre.kappa_low
    - base_context.prefix.legendre.kappa_high
    - base_context.prefix.legendre.maxiter
    - base_context.prefix.legendre.steps_x
    - base_context.prefix.legendre.steps_y
    - base_context.prefix.legendre.filter_size_x
    - base_context.prefix.legendre.filter_size_y
    - base_context.prefix.legendre.order_x
    - base_context.prefix.legendre.order_y
  The CLI aliases omit the base_context.
 */
/*----------------------------------------------------------------------------*/
static cpl_parameterlist * hdrl_bpm_2d_legendresmooth_parameter_create_parlist(
        const char           *base_context,
        const char           *prefix,
        const hdrl_parameter *deflt)
{
    cpl_ensure(prefix && base_context && deflt,
    		CPL_ERROR_NULL_INPUT, NULL);

    cpl_ensure(hdrl_bpm_2d_parameter_check(deflt),
           	CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    cpl_parameterlist   *   parlist = cpl_parameterlist_new();
    char                *   context =
        hdrl_join_string(".", 2, base_context, prefix);

    double kappa_low_def = hdrl_bpm_2d_parameter_get_kappa_low(deflt);
    double kappa_high_def = hdrl_bpm_2d_parameter_get_kappa_high(deflt);
    int maxiter_def = hdrl_bpm_2d_parameter_get_maxiter(deflt);

    /* --prefix.kappa_low */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "kappa-low", base_context,
            "Low RMS scaling factor for image thresholding", CPL_TYPE_DOUBLE,
            kappa_low_def) ;

    /* --prefix.kappa_high */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "kappa-high", base_context,
            "High RMS scaling factor for image thresholding", CPL_TYPE_DOUBLE,
            kappa_high_def) ;

    /* --prefix.maxiter */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "maxiter", base_context,
            "Maximum number of algorithm iterations", CPL_TYPE_INT, maxiter_def);

    /* --prefix.steps_x */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "steps-x",
            base_context, "Number of image sampling points in x-dir for fitting",
            CPL_TYPE_INT, hdrl_bpm_2d_parameter_get_steps_x(deflt));

    /* --prefix.steps_y */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "steps-y",
            base_context, "Number of image sampling points in y-dir for fitting",
            CPL_TYPE_INT, hdrl_bpm_2d_parameter_get_steps_y(deflt)) ;

    /* --prefix.filter_size_x */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "filter-size-x",
            base_context, "X size of the median box around sampling points", CPL_TYPE_INT,
            hdrl_bpm_2d_parameter_get_filter_size_x(deflt)) ;

    /* --prefix.filter_size_y */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "filter-size-y",
            base_context, "Y size of the median box around sampling points", CPL_TYPE_INT,
            hdrl_bpm_2d_parameter_get_filter_size_y(deflt));

    /* --prefix.order_x */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "order-x",
            base_context, "Order of x polynomial for the fit", CPL_TYPE_INT,
            hdrl_bpm_2d_parameter_get_order_x(deflt));

    /* --prefix.order_y */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "order-y",
            base_context, "Order of y polynomial for the fit", CPL_TYPE_INT,
            hdrl_bpm_2d_parameter_get_order_y(deflt)) ;


    cpl_free(context);
    if (cpl_error_get_code()) {
        cpl_parameterlist_delete(parlist);
        return NULL;
    }
    return parlist;
}


static const char * filter_to_string(cpl_filter_mode filter)
{
    switch (filter) {
        case CPL_FILTER_EROSION:
            return "EROSION";
            break;
        case CPL_FILTER_DILATION:
            return "DILATION";
            break;
        case CPL_FILTER_OPENING:
            return "OPENING";
            break;
        case CPL_FILTER_CLOSING:
            return "CLOSING";
            break;
        case CPL_FILTER_LINEAR:
            return "LINEAR";
            break;
        case CPL_FILTER_LINEAR_SCALE:
            return "LINEAR_SCALE";
            break;
        case CPL_FILTER_AVERAGE:
            return "AVERAGE";
            break;
        case CPL_FILTER_AVERAGE_FAST:
            return "AVERAGE_FAST";
            break;
        case CPL_FILTER_MEDIAN:
            return "MEDIAN";
            break;
        case CPL_FILTER_STDEV:
            return "STDEV";
            break;
        case CPL_FILTER_STDEV_FAST:
            return "STDEV_FAST";
            break;
        case CPL_FILTER_MORPHO:
            return "MORPHO";
            break;
        case CPL_FILTER_MORPHO_SCALE:
            return "MORPHO_SCALE";
            break;
        default :
        	cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT, "Filter unknown");
        	return "";
        	break;
    }
}

static const char * border_to_string(cpl_border_mode border)
{
    switch (border) {
        case CPL_BORDER_FILTER:
            return "FILTER";
            break;
        case CPL_BORDER_ZERO:
            return "ZERO";
            break;
        case CPL_BORDER_CROP:
            return "CROP";
            break;
        case CPL_BORDER_NOP:
            return "NOP";
            break;
        case CPL_BORDER_COPY:
            return "COPY";
            break;
        default :
        	cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT, "border unknown");
        	return "";
        	break;
    }
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief Create parameter list for the BPM_2D filtersmooth computation
  @param base_context    base context of parameter (e.g. recipe name)
  @param prefix          prefix of parameter, may be an empty string
  @param deflt           defaults
  Creates a parameter list with the BPM_2D parameters:
    - base_context.prefix.method
    - base_context.prefix.filter.kappa-low
    - base_context.prefix.filter.kappa-high
    - base_context.prefix.filter.maxiter
    - base_context.prefix.filter.filter
    - base_context.prefix.filter.border
    - base_context.prefix.filter.smooth-x
    - base_context.prefix.filter.smooth-y
  The CLI aliases omit the base_context.
 */
/*----------------------------------------------------------------------------*/
static cpl_parameterlist * hdrl_bpm_2d_filtersmooth_parameter_create_parlist(
        const char           *base_context,
        const char           *prefix,
        const hdrl_parameter *deflt)
{
    cpl_ensure(prefix && base_context && deflt,
    		CPL_ERROR_NULL_INPUT, NULL);

    cpl_ensure(hdrl_bpm_2d_parameter_check(deflt),
           	CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    char                *   name ;
    cpl_parameterlist   *   parlist = cpl_parameterlist_new();
    cpl_parameter       *   par ;
    char                *   context =
        hdrl_join_string(".", 2, base_context, prefix);

    double kappa_low_def = hdrl_bpm_2d_parameter_get_kappa_low(deflt);
    double kappa_high_def = hdrl_bpm_2d_parameter_get_kappa_high(deflt);
    int maxiter_def = hdrl_bpm_2d_parameter_get_maxiter(deflt);

    /* --prefix.kappa_low */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "kappa-low", base_context,
            "Low RMS scaling factor for image thresholding", CPL_TYPE_DOUBLE, kappa_low_def) ;

    /* --prefix.kappa_high */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "kappa-high", base_context,
            "High RMS scaling factor for image thresholding", CPL_TYPE_DOUBLE, kappa_high_def) ;

    /* --prefix.maxiter */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "maxiter", base_context,
            "Maximum number of algorithm iterations", CPL_TYPE_INT, maxiter_def);

    /* --prefix.filter */
    cpl_filter_mode filter = hdrl_bpm_2d_parameter_get_filter(deflt);
    const char * filter_def = filter_to_string(filter);
    name = hdrl_join_string(".", 2, context, "filter");
    par = cpl_parameter_new_enum(name, CPL_TYPE_STRING, "Filter mode for image smooting",
            context, filter_def, 3, "AVERAGE", "AVERAGE_FAST", "MEDIAN");
    cpl_free(name);
    name = hdrl_join_string(".", 2, prefix, "filter");
    cpl_parameter_set_alias(par, CPL_PARAMETER_MODE_CLI, name);
    cpl_parameter_disable(par, CPL_PARAMETER_MODE_ENV);
    cpl_free(name);
    cpl_parameterlist_append(parlist, par);

    /* --prefix.border */
    cpl_border_mode border = hdrl_bpm_2d_parameter_get_border(deflt);
    const char * border_def = border_to_string(border);
    name = hdrl_join_string(".", 2, context, "border");
    par = cpl_parameter_new_enum(name, CPL_TYPE_STRING,
            "Border mode to use for the image smooting filter "
            "(only for MEDIAN filter)",
            context, border_def, 4, "FILTER", "CROP", "NOP", "COPY");
    cpl_free(name);
    name = hdrl_join_string(".", 2, prefix, "border");
    cpl_parameter_set_alias(par, CPL_PARAMETER_MODE_CLI, name);
    cpl_parameter_disable(par, CPL_PARAMETER_MODE_ENV);
    cpl_free(name);
    cpl_parameterlist_append(parlist, par);

    /* --prefix.smooth_x */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "smooth-x",
            base_context, "Kernel y size of the smoothing filter", CPL_TYPE_INT,
            hdrl_bpm_2d_parameter_get_smooth_x(deflt));

    /* --prefix.smooth_y */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "smooth-y",
            base_context, "Kernel y size of the image smoothing filter", CPL_TYPE_INT,
            hdrl_bpm_2d_parameter_get_smooth_y(deflt));

    cpl_free(context);
    if (cpl_error_get_code()) {
        cpl_parameterlist_delete(parlist);
        return NULL;
    }

    return parlist;
}

/*----------------------------------------------------------------------------*/
/**
  @brief Create parameter list for the BPM_2D computation
  @param base_context    base context of parameter (e.g. recipe name)
  @param prefix          prefix of parameter, may be an empty string
  @param method_def         default used method
  @param filtersmooth_def   defaults for filtersmooth method
  @param legendresmooth_def defaults for legendresmooth method
  Creates a parameter list with the BPM_2D parameters:
    - base_context.prefix.method
    - base_context.prefix.legendre.kappa-low
    - base_context.prefix.legendre.kappa-high
    - base_context.prefix.legendre.maxiter
    - base_context.prefix.legendre.steps-x
    - base_context.prefix.legendre.steps-y
    - base_context.prefix.legendre.filter-size-x
    - base_context.prefix.legendre.filter-size-y
    - base_context.prefix.legendre.order-x
    - base_context.prefix.legendre.order-y
    - base_context.prefix.filter.kappa-low
    - base_context.prefix.filter.kappa-high
    - base_context.prefix.filter.maxiter
    - base_context.prefix.filter.filter
    - base_context.prefix.filter.border
    - base_context.prefix.filter.smooth-x
    - base_context.prefix.filter.smooth-y
  The CLI aliases omit the base_context.
 */
/*----------------------------------------------------------------------------*/
cpl_parameterlist * hdrl_bpm_2d_parameter_create_parlist(
        const char           *base_context,
        const char           *prefix,
        const char           *method_def,
        const hdrl_parameter *filtersmooth_def,
        const hdrl_parameter *legendresmooth_def)
{
    cpl_ensure(prefix && base_context && method_def,
    		CPL_ERROR_NULL_INPUT, NULL);

    cpl_ensure(filtersmooth_def || legendresmooth_def,
            CPL_ERROR_NULL_INPUT, NULL);

    if(filtersmooth_def){
        cpl_ensure(hdrl_bpm_2d_parameter_check(filtersmooth_def),
        	CPL_ERROR_INCOMPATIBLE_INPUT, NULL);
    }

    if(legendresmooth_def){
        cpl_ensure(hdrl_bpm_2d_parameter_check(legendresmooth_def),
        	CPL_ERROR_INCOMPATIBLE_INPUT, NULL);
    }

    char                *   name ;
    cpl_parameterlist   *   parlist = cpl_parameterlist_new();
    cpl_parameter       *   par ;
    char                *   context =
        hdrl_join_string(".", 2, base_context, prefix);

    /* --prefix.method */
    name = hdrl_join_string(".", 2, context, "method");
    par = cpl_parameter_new_enum(name, CPL_TYPE_STRING, "Method used", context,
            method_def, 2, "FILTER", "LEGENDRE");
    cpl_free(name);
    name = hdrl_join_string(".", 2, prefix, "method");
    cpl_parameter_set_alias(par, CPL_PARAMETER_MODE_CLI, name);
    cpl_free(name);
    cpl_parameter_disable(par, CPL_PARAMETER_MODE_ENV);
    cpl_parameterlist_append(parlist, par);

    /* --prefix.legendre */
    name = hdrl_join_string(".", 2, prefix, "legendre");
    cpl_parameterlist * pleg = hdrl_bpm_2d_legendresmooth_parameter_create_parlist(
            base_context, name, legendresmooth_def);
    cpl_free(name);
    for (cpl_parameter * p = cpl_parameterlist_get_first(pleg) ;
            p != NULL; p = cpl_parameterlist_get_next(pleg))
        cpl_parameterlist_append(parlist, cpl_parameter_duplicate(p));
    cpl_parameterlist_delete(pleg);

    /* --prefix.filter */
    name = hdrl_join_string(".", 2, prefix, "filter");
    cpl_parameterlist * pfil = hdrl_bpm_2d_filtersmooth_parameter_create_parlist(
            base_context, name, filtersmooth_def);
    cpl_free(name);
    for (cpl_parameter * p = cpl_parameterlist_get_first(pfil) ;
            p != NULL; p = cpl_parameterlist_get_next(pfil))
        cpl_parameterlist_append(parlist, cpl_parameter_duplicate(p));
    cpl_parameterlist_delete(pfil);

    cpl_free(context);
    if (cpl_error_get_code()) {
        cpl_parameterlist_delete(parlist);
        return NULL;
    }

    return parlist;
}

/*----------------------------------------------------------------------------*/
/**
  @brief Parse parameter list to create input parameters for the BPM_2D
  @param parlist        parameter list to parse
  @param prefix         prefix of parameter name
  @return   Input parameters for the BPM_2D computation

  Reads a parameter list in order to create BPM_2D parameters.

  Expects a parameter list containing:
  - prefix.method
  - prefix.legendre.kappa-low
  - prefix.legendre.kappa-high
  - prefix.legendre.maxiter
  - prefix.legendre.steps-x
  - prefix.legendre.steps-y
  - prefix.legendre.filter-size-x
  - prefix.legendre.filter-size-y
  - prefix.legendre.order-x
  - prefix.legendre.order-y
  - prefix.filter.kappa-low
  - prefix.filter.kappa-high
  - prefix.filter.maxiter
  - prefix.filter.filter
  - prefix.filter.border
  - prefix.filter.smooth-x
  - prefix.filter.smooth-y
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_bpm_2d_parameter_parse_parlist(
        const cpl_parameterlist *   parlist,
        const char              *   prefix)
{
    cpl_ensure(prefix && parlist, CPL_ERROR_NULL_INPUT, NULL);
    char                *   name ;
    const cpl_parameter *   par;
    const char          *   tmp_str;
    cpl_filter_mode         filter = CPL_FILTER_EROSION ;
    cpl_border_mode         border = CPL_BORDER_FILTER ;
    double                  kappa_low = -1.0 ;
    double                  kappa_high = -1.0 ;
    int                     maxiter = -1;
    int                     steps_x = -1 ;
    int                     steps_y = -1  ;
    int                     filter_size_x = -1 ;
    int                     filter_size_y = -1 ;
    int                     order_x = -1 ;
    int                     order_y = -1 ;
    cpl_size                smooth_x = -1 ;
    cpl_size                smooth_y = -1 ;
    hdrl_bpm_2d_method      method ;
  
    /* --method */
    name = hdrl_join_string(".", 2, prefix, "method");
    par = cpl_parameterlist_find_const(parlist, name) ;
    tmp_str = cpl_parameter_get_string(par);
    if (tmp_str == NULL) {
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                "Parameter %s not found", name);
        cpl_free(name);
        return NULL;
    }
    cpl_free(name) ;
    if(!strcmp(tmp_str, "FILTER")) {
        method = HDRL_BPM_2D_FILTERSMOOTH ;
    } else if(!strcmp(tmp_str, "LEGENDRE")) {
        method = HDRL_BPM_2D_LEGENDRESMOOTH ;
    } else {
        cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                "Invalid method: %s", tmp_str);
        return NULL;
    }

    char * kappa_prefix = hdrl_join_string(".", 2, prefix,
                       method == HDRL_BPM_2D_FILTERSMOOTH ?
                       "filter" : "legendre" );

    /* --kappa_low */
    name = hdrl_join_string(".", 2, kappa_prefix, "kappa-low");
    par=cpl_parameterlist_find_const(parlist, name);
    kappa_low = cpl_parameter_get_double(par);
    cpl_free(name) ;

    /* --kappa_high */
    name = hdrl_join_string(".", 2, kappa_prefix, "kappa-high");
    par=cpl_parameterlist_find_const(parlist, name);
    kappa_high = cpl_parameter_get_double(par);
    cpl_free(name) ;

    /* --maxiter */
    name = hdrl_join_string(".", 2, kappa_prefix, "maxiter");
    par=cpl_parameterlist_find_const(parlist, name);
    maxiter = cpl_parameter_get_int(par);
    cpl_free(name) ;

    cpl_free(kappa_prefix);

    /* --steps_x */
    name = hdrl_join_string(".", 2, prefix, "legendre.steps-x");
    par=cpl_parameterlist_find_const(parlist, name);
    steps_x = cpl_parameter_get_int(par);
    cpl_free(name) ;

    /* --steps_y */
    name = hdrl_join_string(".", 2, prefix, "legendre.steps-y");
    par=cpl_parameterlist_find_const(parlist, name);
    steps_y = cpl_parameter_get_int(par);
    cpl_free(name) ;

    /* --filter_size_x */
    name = hdrl_join_string(".", 2, prefix, "legendre.filter-size-x");
    par=cpl_parameterlist_find_const(parlist, name);
    filter_size_x = cpl_parameter_get_int(par);
    cpl_free(name) ;

    /* --filter_size_y */
    name = hdrl_join_string(".", 2, prefix, "legendre.filter-size-y");
    par=cpl_parameterlist_find_const(parlist, name);
    filter_size_y = cpl_parameter_get_int(par);
    cpl_free(name) ;

    /* --order_x */
    name = hdrl_join_string(".", 2, prefix, "legendre.order-x");
    par=cpl_parameterlist_find_const(parlist, name);
    order_x = cpl_parameter_get_int(par);
    cpl_free(name) ;

    /* --order_y */
    name = hdrl_join_string(".", 2, prefix, "legendre.order-y");
    par=cpl_parameterlist_find_const(parlist, name);
    order_y = cpl_parameter_get_int(par);
    cpl_free(name) ;

    /* --filter */
    name = hdrl_join_string(".", 2, prefix, "filter.filter");
    par = cpl_parameterlist_find_const(parlist, name) ;
    tmp_str = cpl_parameter_get_string(par);
    if (tmp_str == NULL) {
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                "Parameter %s not found", name);
        cpl_free(name);
        return NULL;
    }
    if(!strcmp(tmp_str, "erosion")) {
        filter = CPL_FILTER_EROSION ;
    } else if(!strcmp(tmp_str, "DILATION")) {
        filter = CPL_FILTER_DILATION ;
    } else if(!strcmp(tmp_str, "OPENING")) {
        filter = CPL_FILTER_OPENING ;
    } else if(!strcmp(tmp_str, "CLOSING")) {
        filter = CPL_FILTER_CLOSING ;
    } else if(!strcmp(tmp_str, "LINEAR")) {
        filter = CPL_FILTER_LINEAR ;
    } else if(!strcmp(tmp_str, "LINEAR_SCALE")) {
        filter = CPL_FILTER_LINEAR_SCALE ;
    } else if(!strcmp(tmp_str, "AVERAGE")) {
        filter = CPL_FILTER_AVERAGE ;
    } else if(!strcmp(tmp_str, "AVERAGE_FAST")) {
        filter = CPL_FILTER_AVERAGE_FAST ;
    } else if(!strcmp(tmp_str, "MEDIAN")) {
        filter = CPL_FILTER_MEDIAN ;
    } else if(!strcmp(tmp_str, "STDEV")) {
        filter = CPL_FILTER_STDEV ;
    } else if(!strcmp(tmp_str, "STDEV_FAST")) {
        filter = CPL_FILTER_STDEV_FAST ;
    } else if(!strcmp(tmp_str, "MORPHO")) {
        filter = CPL_FILTER_MORPHO ;
    } else if(!strcmp(tmp_str, "MORPHO_SCALE")) {
        filter = CPL_FILTER_MORPHO_SCALE ;
    }
    cpl_free(name) ;
    
    /* --border */
    name = hdrl_join_string(".", 2, prefix, "filter.border");
    par = cpl_parameterlist_find_const(parlist, name) ;
    tmp_str = cpl_parameter_get_string(par);
    if (tmp_str == NULL) {
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                "Parameter %s not found", name);
        cpl_free(name);
        return NULL;
    }
    if(!strcmp(tmp_str, "filter")) {
        border = CPL_BORDER_FILTER ;
    } else if(!strcmp(tmp_str, "ZERO")) {
        border = CPL_BORDER_ZERO ;
    } else if(!strcmp(tmp_str, "CROP")) {
        border = CPL_BORDER_CROP ;
    } else if(!strcmp(tmp_str, "NOP")) {
        border = CPL_BORDER_NOP ;
    } else if(!strcmp(tmp_str, "COPY")) {
        border = CPL_BORDER_COPY ;
    } 
    cpl_free(name) ;

    /* --smooth_x */
    name = hdrl_join_string(".", 2, prefix, "filter.smooth-x");
    par=cpl_parameterlist_find_const(parlist, name);
    smooth_x = cpl_parameter_get_int(par);
    cpl_free(name) ;

    /* --smooth_y */
    name = hdrl_join_string(".", 2, prefix, "filter.smooth-y");
    par=cpl_parameterlist_find_const(parlist, name);
    smooth_y = cpl_parameter_get_int(par);
    cpl_free(name) ;

    /* Return */
    if (cpl_error_get_code()) {
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                "Error while parsing parameterlist with prefix %s", prefix);
        return NULL;
    } else {
        if (method == HDRL_BPM_2D_FILTERSMOOTH) {
            return hdrl_bpm_2d_parameter_create_filtersmooth(kappa_low, 
                    kappa_high, maxiter, filter, border, smooth_x, smooth_y) ;
        } else if (method == HDRL_BPM_2D_LEGENDRESMOOTH) {
            return hdrl_bpm_2d_parameter_create_legendresmooth(kappa_low, 
                    kappa_high, maxiter, steps_x, steps_y, filter_size_x, 
                    filter_size_y, order_x, order_y) ;
        } else {
            return NULL ;
        }
    }
}

/*----------------------------------------------------------------------------*/
/**
  @brief Detect bad pixels on a single image with an iterative process
  @param img_in     input hdrl image
  @param params     BPM_2D computation parameters - see below
  @return Bad pixel mask with the newly found bad pixels


 The algorithm first smoothes the image by applying the methods
 described below. Then it subtracts the smoothed image and derives bad
 pixels by thresholding the residual image, i.e. all pixels exceeding
 the threshold are considered as bad. To compute the upper and lower
 threshold, it measures a robust rms (a properly scaled Median Absolute
 Deviation), which is then scaled by the parameter \e kappa_low and \e
 kappa_high. Furthermore, the algorithm is applied iteratively
 controlled by \e maxiter. During each iteration the newly found bad
 pixels are ignored. Please note, that the thresholding values are
 applied as median(residual-image) \f$\pm\f$ thresholds. This makes the
 algorithm more robust in the case that the methods listed below are
 not able to completely remove the background level, e.g due to an
 exceeding number of bad pixels in the first iteration.

Two methods are currently available to derive a smoothed version of
the image:

\li \c Applying a filter like e.g. a median filter to the image. The
  filtering can be done by all modes currently supported by cpl and is
  controlled by the filter-type \e filter, the border-type \e border
  and by the kernel size in x and y, i.e.  \e smooth_x and \e
  smooth_y. The corresponding BPM_2D parameter is created by
  hdrl_bpm_2d_parameter_create_filtersmooth()

\li \c Fitting a Legendre polynomial to the image of order \e order_x,
  in x and \e order_y in y direction.  This method allows you to
  define \e steps_x \f$\times\f$ \e steps_y sampling points (the
  latter are computed as the median within a box of \e filter_size_x
  and \e filter_size_y) where the polynomial is fitted. This
  substantially decreases the fitting time for the Legendre
  polynomial. The corresponding BPM_2D parameter is created by
  hdrl_bpm_2d_parameter_create_legendresmooth()

 */
/*----------------------------------------------------------------------------*/
cpl_mask * hdrl_bpm_2d_compute(
        const hdrl_image        *   img_in,
        const hdrl_parameter    *   params)
{
    cpl_image   *   img;
    cpl_mask    *   mask_iter, * img_mask;

    /* Check Entries */
    cpl_error_ensure(img_in && params, CPL_ERROR_NULL_INPUT,
            return NULL, "NULL input");
   if (hdrl_bpm_2d_parameter_verify(params) != CPL_ERROR_NONE) return NULL;

    /* Local Usage Parameters */
    const hdrl_bpm_2d_parameter * p_loc = (const hdrl_bpm_2d_parameter *)params ;

    img = cpl_image_duplicate(hdrl_image_get_image_const(img_in));
    img_mask = cpl_mask_duplicate(cpl_image_get_bpm(img));

    /* The first iteration contains the passed mask */
    mask_iter = cpl_mask_duplicate(img_mask);


    for (int var = 0; var < p_loc->maxiter; ++var) {
        cpl_image   * img_res = NULL;
        cpl_mask * mask_iter_startloop = cpl_mask_duplicate(mask_iter);
        double median, mad, std_mad, std_mad_low, std_mad_high ;

        /*Add original bad pixels to previous iteration*/
        cpl_mask_or(mask_iter, img_mask);

        /* Filter the image */
        if (p_loc->method == HDRL_BPM_2D_FILTERSMOOTH){
            img_res = hdrl_get_residuals_filtersmooth(p_loc->smooth_x, 
                    p_loc->smooth_y, p_loc->filter, p_loc->border, img, 
                    mask_iter);
        } else if (p_loc->method == HDRL_BPM_2D_LEGENDRESMOOTH) {
            img_res = hdrl_get_residuals_legendresmooth(img, p_loc->steps_x,
                    p_loc->steps_y, p_loc->filter_size_x, p_loc->filter_size_y,
                    p_loc->order_x, p_loc->order_y);
        }

         /*
         For a Gaussian distribution the Median Absolute Deviation (MAD) is a
         robust and consistent estimate of the Standard Deviation (STD) in the
         sense that the STD is approximately K * MAD, where K is a constant
         equal to approximately 1.4826 == CPL_MATH_STD_MAD
         */

        /* Calculating the mad and assuming that after subtraction image has a
           mean of Zero */

        median = cpl_image_get_mad(img_res, &mad);
        //mad = cpl_image_get_stdev(img_res);
        if(mad <= 0){
            mad=nextafter(0,1.0);
        }
        std_mad = CPL_MATH_STD_MAD * mad;
        std_mad_low = median -(std_mad * p_loc->kappa_low);
        std_mad_high = median + (std_mad * p_loc->kappa_high);

        /*restore the original mask as we only want to add the new bad pixels
         * to the originally passed mask - done by the threshold function*/

        cpl_image_reject_from_mask(img_res, img_mask);

        /*Reset all pixels to good in the mask as we only want the new bad
         * pixels  */
        cpl_mask_xor(mask_iter, mask_iter);

        cpl_mask_threshold_image(mask_iter, img_res, std_mad_low, std_mad_high,
                CPL_BINARY_0);

        /* Currently the cpl function assigns the "outside" value
         * also to the bad pixels - thus if one only wants to have the new bad
         * pixel, one has to do the bitwise or */
        cpl_mask_xor(mask_iter, img_mask);
        cpl_image_delete(img_res);

        if (!hdrl_check_maskequality(mask_iter, mask_iter_startloop)) {
            cpl_mask_delete(mask_iter_startloop);
            cpl_msg_debug(cpl_func, "iter: %d", var);
            break;
        }
        cpl_mask_delete(mask_iter_startloop);
        cpl_msg_debug(cpl_func, "iter: %d", var);
    }
    cpl_mask_delete(img_mask);
    cpl_image_delete(img);

    return mask_iter ;
}

/**@}*/

/*----------------------------------------------------------------------------*/
/**
  @brief calculates the residual of an image by fitting a Legendre polynomial
  @param img             Input image where the residuals are calculated
  @param steps_x         Number of sampling points in x for the polyfit
  @param steps_y         Number of sampling points in y for the polyfit
  @param filter_size_x   x-size of the median filter to calc the sampling points
  @param filter_size_y   y-size of the median filter to calc the sampling points
  @param order_x         x-degree of the fitted Legendre polynomial
  @param order_y         y-degree of the fitted Legendre polynomial
  @return                The residual image derived as (data - fit)

  The function interpolates the image onto a grid steps_x, steps_y using the
  median. The size of the median box can by controlled by filter_size_x,
  filter_size_y. On this image a Legendre polynomial of degree order_x, order_y
  is fitted and subtracted from the original image
 */
/*----------------------------------------------------------------------------*/
static cpl_image * hdrl_get_residuals_legendresmooth(
        const cpl_image *   img, 
        int                 steps_x,
        int                 steps_y,
        int                 filter_size_x, 
        int                 filter_size_y,
        int                 order_x, 
        int                 order_y)
{
    cpl_image * img_res;
    cpl_image * img_filtered;
    cpl_size nx = cpl_image_get_size_x(img);
    cpl_size ny = cpl_image_get_size_y(img);
    cpl_size sx = CX_MAX(nx / steps_x, 1);
    cpl_size sy = CX_MAX(ny / steps_y, 1);

    /* fit to stepped grid */
    cpl_matrix * x = hdrl_matrix_linspace(sx / 2, nx, sx);
    cpl_matrix * y = hdrl_matrix_linspace(sy / 2, ny, sy);
    cpl_image * imgtmp_mod =
        hdrl_medianfilter_image_grid(img, x, y,
                                     filter_size_x,
                                     filter_size_y);
    cpl_matrix * coeffs = hdrl_fit_legendre(imgtmp_mod,
                                            order_x, order_y,
                                            x, y, nx, ny);
    /* TODO: naming conventions: hdrl_matrix_legendre_to_image() */
    img_filtered = hdrl_legendre_to_image(coeffs, order_x, order_y, nx, ny);
    img_res = cpl_image_subtract_create(img, img_filtered);
    if (cpl_msg_get_level() == CPL_MSG_DEBUG)
        cpl_matrix_dump(coeffs, stdout);
    cpl_matrix_delete(coeffs);
    cpl_matrix_delete(x);
    cpl_matrix_delete(y);
    cpl_image_delete(imgtmp_mod);
    cpl_image_delete(img_filtered);
    return img_res;
}

/*----------------------------------------------------------------------------*/
/**
  @brief calculates the residual of an image by smoothing the image
  @param kernel_size_x x-size of the smoothing kernel
  @param kernel_size_y y-size of the smoothing kernel
  @param filter,       Filter type to be applied (all cpl filters)
  @param border,       Border-mode to be applied (all cpl border)
  @param img,          Input image where to apply the filtering
  @param mask_iter,    Input mask do determine the bad pixels
  @return              The residual image derived as (data - data_smoothed)
 */
/*----------------------------------------------------------------------------*/
static cpl_image * hdrl_get_residuals_filtersmooth(
        cpl_size            kernel_size_x, 
        cpl_size            kernel_size_y, 
        cpl_filter_mode     filter,
        cpl_border_mode     border, 
        cpl_image       *   img,
        cpl_mask        *   mask_iter)
{
    cpl_mask    *   kernel ;
    cpl_image   *   img_res = NULL;
    cpl_image   *   img_filtered = NULL;

    cpl_size nx = cpl_image_get_size_x(img);
    cpl_size ny = cpl_image_get_size_y(img);


    /* Create the kernel */
    kernel = cpl_mask_new(kernel_size_x, kernel_size_y) ;
    cpl_mask_not(kernel); 
    if (kernel == NULL) return NULL ;

    /* Filter the image */
    cpl_image_reject_from_mask(img, mask_iter);
    if (border == CPL_BORDER_FILTER) {
        img_filtered = hdrl_parallel_filter_image(img, NULL, kernel, filter);
    }
    else {
        img_filtered = cpl_image_new(nx, ny, HDRL_TYPE_DATA);
        cpl_image_filter_mask(img_filtered, img, kernel, filter, border);
    }
    cpl_mask_delete(kernel) ;
    img_res = cpl_image_subtract_create(img, img_filtered);
    cpl_image_delete(img_filtered);

    return img_res;
}

