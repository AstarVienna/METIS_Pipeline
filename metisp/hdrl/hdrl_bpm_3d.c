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

#include "hdrl_bpm_3d.h"
#include "hdrl_prototyping.h"

#include <cpl.h>
#include <string.h>
#include <math.h>

/*-----------------------------------------------------------------------------
                                   Static
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_bpm_3d     Bad Pixel Mask 3D Computation
  @ingroup hdrl_bpm

  @brief
   Algorithms to detect bad pixels on a stack of identical images like e.g.
   bias images

 The routines in this module can be used to detect bad pixels on a stack of
 identical images like bias or dark images. The algorithm first collapses
 the stack of images by using the median in order to generate a master-image.
 Then it subtracts the master image from each individual image and derives the
 bad pixels on the residual-images by thresholding, i.e. all pixels exceeding
 the threshold are considered as bad. Please note, that the algorithm assumes
 that the mean level of the different images is the same, if this is not the
 case, the master-image as described above will be biased.

 The calculation is performed by calling the top-level function
 hdrl_bpm_3d_compute() and the parameters passed to this function can be created
 by calling hdrl_bpm_3d_parameter_create().

 */
/*----------------------------------------------------------------------------*/

/**@{*/

/** @cond PRIVATE */


/*-----------------------------------------------------------------------------
                        BPM Parameters Definition
 -----------------------------------------------------------------------------*/
typedef struct {
    HDRL_PARAMETER_HEAD;
    double              kappa_low ;
    double              kappa_high ;
    hdrl_bpm_3d_method  method ;
} hdrl_bpm_3d_parameter;

/* Parameter type */
static hdrl_parameter_typeobj hdrl_bpm_3d_parameter_type = {
    HDRL_PARAMETER_BPM_3D,                  /* type */
    (hdrl_alloc *)&cpl_malloc,              /* fp_alloc */
    (hdrl_free *)&cpl_free,                 /* fp_free */
    NULL,                                   /* fp_destroy */
    sizeof(hdrl_bpm_3d_parameter),          /* obj_size */
};
/** @endcond */

/* ---------------------------------------------------------------------------*/
/**
  @brief    Creates BPM Parameters object for the imagelist method
  @param    kappa_low       Low kappa factor for thresholding algorithm
  @param    kappa_high      High kappa factor for thresholding algorithm
  @param    method          used method
  @return   The BPM_3D parameters object.

            It needs to be deallocated with hdrl_parameter_delete()

  @see      hdrl_parameter_delete()
  @see      hdrl_bpm_3d_compute()
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter * hdrl_bpm_3d_parameter_create(
        double              kappa_low,
        double              kappa_high,
        hdrl_bpm_3d_method  method)
{
    hdrl_bpm_3d_parameter * p = (hdrl_bpm_3d_parameter *)
               hdrl_parameter_new(&hdrl_bpm_3d_parameter_type);
    p->kappa_low = kappa_low ;
    p->kappa_high = kappa_high ;
    p->method = method ;
    return (hdrl_parameter *)p;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Verify basic correctness of the BPM_3D parameters
  @param    param   BPM image parameters
  @return   CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_bpm_3d_parameter_verify(
        const hdrl_parameter    *   param)
{
    const hdrl_bpm_3d_parameter * param_loc = (const hdrl_bpm_3d_parameter *)param ;

    cpl_error_ensure(param != NULL, CPL_ERROR_NULL_INPUT,
            return CPL_ERROR_NULL_INPUT, "NULL Input Parameters");
    cpl_error_ensure(hdrl_bpm_3d_parameter_check(param),
            CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
            "Expected BPM image parameter") ;

    cpl_error_ensure(param_loc->method == HDRL_BPM_3D_THRESHOLD_ABSOLUTE || 
            param_loc->method == HDRL_BPM_3D_THRESHOLD_RELATIVE ||
            param_loc->method == HDRL_BPM_3D_THRESHOLD_ERROR,
            CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT, 
            "Unsupported method");
    switch (param_loc->method) {
        case HDRL_BPM_3D_THRESHOLD_ABSOLUTE:
            cpl_error_ensure(param_loc->kappa_high >= param_loc->kappa_low, 
                    CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                    "kappa_high must be larger than kappa_low");
            break;
        case HDRL_BPM_3D_THRESHOLD_RELATIVE:
        case HDRL_BPM_3D_THRESHOLD_ERROR:
            cpl_error_ensure(param_loc->kappa_low >= 0, 
                    CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                    "kappa_low must be >=0");
            cpl_error_ensure(param_loc->kappa_high >= 0, 
                    CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                    "kappa_high must be >=0");
            break;
    }
    return CPL_ERROR_NONE ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Check that the parameter is a BPM_3D parameter
  @param    self The parameter to check
  @return   True or False
 */
/*----------------------------------------------------------------------------*/
cpl_boolean hdrl_bpm_3d_parameter_check(const hdrl_parameter * self)
{
    return hdrl_parameter_check_type(self, &hdrl_bpm_3d_parameter_type);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the kappa_low in the BPM_3D parameter
  @param    p   The BPM_3D parameter
  @return   The kappa_low
 */
/*----------------------------------------------------------------------------*/
double hdrl_bpm_3d_parameter_get_kappa_low(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.0);
    return ((const hdrl_bpm_3d_parameter *)p)->kappa_low;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the kappa_high in the BPM_3D parameter
  @param    p   The BPM_3D parameter
  @return   The kappa_high
 */
/*----------------------------------------------------------------------------*/
double hdrl_bpm_3d_parameter_get_kappa_high(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.0);
    return ((const hdrl_bpm_3d_parameter *)p)->kappa_high;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the method in the BPM_3D parameter
  @param    p   The BPM_3D parameter
  @return   The method
 */
/*----------------------------------------------------------------------------*/
hdrl_bpm_3d_method hdrl_bpm_3d_parameter_get_method(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    return ((const hdrl_bpm_3d_parameter *)p)->method;
}

/*----------------------------------------------------------------------------*/
/**
  @brief Create a parameter list for the BPM_3D computation
  @param base_context    base context of parameter (e.g. recipe name)
  @param prefix          prefix of parameter, may be empty string
  @param defaults        default values

  Creates a parameterlist with the BPM_3D parameters:
    - base_context.prefix.kappa_low
    - base_context.prefix.kappa_high
    - base_context.prefix.method
  The CLI aliases omit the base_context.
 */
/*----------------------------------------------------------------------------*/
cpl_parameterlist * hdrl_bpm_3d_parameter_create_parlist(
        const char           *base_context,
        const char           *prefix,
        const hdrl_parameter *defaults)
{
    cpl_ensure(prefix && base_context && defaults,
    		CPL_ERROR_NULL_INPUT, NULL);

    cpl_ensure(hdrl_bpm_3d_parameter_check(defaults),
    		   CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    char              *name;
    cpl_parameterlist *parlist = cpl_parameterlist_new();
    cpl_parameter     *par;
    char              *context =
        hdrl_join_string(".", 2, base_context, prefix);

    double kappa_low_def  = hdrl_bpm_3d_parameter_get_kappa_low(defaults);
    double kappa_high_def = hdrl_bpm_3d_parameter_get_kappa_high(defaults);

    hdrl_bpm_3d_method method_def = hdrl_bpm_3d_parameter_get_method(defaults);
    cpl_ensure(   method_def == HDRL_BPM_3D_THRESHOLD_ABSOLUTE
    		   || method_def == HDRL_BPM_3D_THRESHOLD_RELATIVE
			   || method_def == HDRL_BPM_3D_THRESHOLD_ERROR,
			   CPL_ERROR_ILLEGAL_INPUT, NULL) ;

    const char *method_str;
    if (method_def == HDRL_BPM_3D_THRESHOLD_ABSOLUTE) {
        method_str = "absolute" ;
    } else if (method_def == HDRL_BPM_3D_THRESHOLD_RELATIVE) {
        method_str = "relative" ;
    } else if (method_def == HDRL_BPM_3D_THRESHOLD_ERROR) {
        method_str = "error" ;
    } else {
    	method_str = "";
    }

    /* --prefix.kappa_low */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "kappa-low", base_context,
            "Low RMS scaling factor for image thresholding.", CPL_TYPE_DOUBLE, 
            kappa_low_def) ;

    /* --prefix.kappa_high */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "kappa-high", base_context,
            "High RMS scaling factor for image thresholding.", CPL_TYPE_DOUBLE,
            kappa_high_def) ;

    /* --prefix.method */
    name = hdrl_join_string(".", 2, context, "method");
    par = cpl_parameter_new_enum(name, CPL_TYPE_STRING, 
            "Thresholdig method to use for bpm detection", context, method_str,
            3, "absolute", "relative", "error");
    cpl_free(name);
    name = hdrl_join_string(".", 2, prefix, "method");
    cpl_parameter_set_alias(par, CPL_PARAMETER_MODE_CLI, name);
    cpl_parameter_disable(par, CPL_PARAMETER_MODE_ENV);
    cpl_free(name);
    cpl_parameterlist_append(parlist, par);

    cpl_free(context);

    if (cpl_error_get_code()) {
        cpl_parameterlist_delete(parlist);
        return NULL;
    }

    return parlist;
}

/*----------------------------------------------------------------------------*/
/**
  @brief Parse a parameterlist to create input parameters for the BPM_3D
  @param parlist        parameter list to parse
  @param prefix         prefix of parameter name
  @return   Input parameters for the BPM_3D computation

  Reads a parameterlist in order to create BPM image parameters.
  Expects a parameterlist containing:
   - prefix.kappa_low
   - prefix.kappa_high
   - prefix.method
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_bpm_3d_parameter_parse_parlist(
        const cpl_parameterlist *   parlist,
        const char              *   prefix)
{
    cpl_ensure(prefix && parlist, CPL_ERROR_NULL_INPUT, NULL);
    char                *   name;
    const cpl_parameter *   par;
    const char          *   tmp_str ;
    double                  kappa_low;
    double                  kappa_high;
    hdrl_bpm_3d_method      method;
   
    /* --kappa_low */
    name = hdrl_join_string(".", 2, prefix, "kappa-low");
    par=cpl_parameterlist_find_const(parlist, name);
    kappa_low = cpl_parameter_get_double(par);
    cpl_free(name) ;

    /* --kappa_high */
    name = hdrl_join_string(".", 2, prefix, "kappa-high");
    par=cpl_parameterlist_find_const(parlist, name);
    kappa_high = cpl_parameter_get_double(par);
    cpl_free(name) ;

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
    if(!strcmp(tmp_str, "absolute")) {
        method = HDRL_BPM_3D_THRESHOLD_ABSOLUTE ;
    } else if(!strcmp(tmp_str, "relative")) {
        method = HDRL_BPM_3D_THRESHOLD_RELATIVE ;
    } else if(!strcmp(tmp_str, "error")) {
        method = HDRL_BPM_3D_THRESHOLD_ERROR ;
    } else {
        cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                "Invalid method: %s", tmp_str);
        return NULL;
    }

    if (cpl_error_get_code()) {
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                "Error while parsing parameterlist with prefix %s", prefix);
        return NULL;
    } else {
        return hdrl_bpm_3d_parameter_create(kappa_low, kappa_high, method) ;
    }
}

/*----------------------------------------------------------------------------*/
/**
  @brief detect bad pixels on a stack of identical images
  @param imglist   input hdrl imagelist
  @param params    BPM_3D computation parameters
  @return cpl_imagelist where the newly rejected pixels are marked as unity.

The algorithm first collapses the stack of images by using the median to
generate a master-image. Then it subtracts the master image from each individual
image and derives the bad pixels on the residual-images by thresholding,
i.e. all pixels strictly exceeding the threshold are considered as bad.

Three methods are currently available to derive the bad pixels on the
residual images and can be set when creating the BPM_3D parameter via
hdrl_bpm_3d_parameter_create():

\li \c method = HDRL_BPM_3D_THRESHOLD_ABSOLUTE: It uses
  \e kappa_low and \e kappa_high as absolute threshold

\li \c method = HDRL_BPM_3D_THRESHOLD_RELATIVE: It scales the
  measured rms on the residual-image with \e kappa_low and
  \e kappa_high and uses it as threshold. For the rms a properly
  scaled Median Absolute Deviation (MAD) is used.

\li \c method = HDRL_BPM_3D_THRESHOLD_ERROR: It scales the
  propagated error of each individual pixel with \e kappa_low and
  \e kappa_high and uses it as threshold.


  @note We assume that the images are already scaled outside this routine, i.e.
  their absolute levels match.

  @details For a Gaussian distribution the Median Absolute Deviation (MAD) is a
  robust and consistent estimate of the Standard Deviation (STD) in the sense
  that the STD is approximately K * MAD, where K is a constant equal to
  approximately 1.4826

 */
/*----------------------------------------------------------------------------*/
cpl_imagelist * hdrl_bpm_3d_compute(
        const hdrl_imagelist    *   imglist,
        const hdrl_parameter    *   params)
{

/*     This routine assumes that the images are already scaled outside ! */

    cpl_imagelist * imglist_out =  NULL;
    hdrl_imagelist * masterlist = NULL;
    hdrl_parameter *  collapse_params = NULL;

    /* Check Entries */
    cpl_error_ensure(imglist && params, CPL_ERROR_NULL_INPUT,
            return NULL, "NULL input");
   if (hdrl_bpm_3d_parameter_verify(params) != CPL_ERROR_NONE) return NULL;

    /* Local Usage Parameters */
    const hdrl_bpm_3d_parameter * p_loc = (const hdrl_bpm_3d_parameter *)params ;

    collapse_params = hdrl_collapse_median_parameter_create();

    imglist_out = cpl_imagelist_new();
    masterlist = hdrl_imagelist_new();


    /* Here we have correlated errors */
    hdrl_image * master;
    cpl_image * contrib_map;

    /* Get the proper collapse function and perform frames combination */
    hdrl_imagelist_collapse(imglist, collapse_params, &master,&contrib_map);
    /* broadcasted same master over list */
    for (int var = 0; var < hdrl_imagelist_get_size(imglist); ++var) {
        hdrl_imagelist_set(masterlist, master,
                        hdrl_imagelist_get_size(masterlist));
    }
    cpl_image_delete(contrib_map);


    for (int var = 0; var < hdrl_imagelist_get_size(imglist); ++var) {
        /* subtract master */
        hdrl_image * tmp_hdrlimg = hdrl_image_sub_image_create(
                hdrl_imagelist_get_const(imglist, var),
                hdrl_imagelist_get(masterlist, var));
        cpl_mask * tmp_cplimg_mask = hdrl_image_get_mask(tmp_hdrlimg);
        cpl_mask * mask_out = NULL;

        if (p_loc->method == HDRL_BPM_3D_THRESHOLD_ABSOLUTE) { 
            /* Absolute values */
            mask_out = cpl_mask_threshold_image_create(
                    hdrl_image_get_image(tmp_hdrlimg), p_loc->kappa_low,
                    p_loc->kappa_high);
            cpl_mask_not(mask_out);
            /*Here we use the bpm from the image after master frame subtraction
             *  - this is not 100 percent clean but a good approximation*/
            cpl_mask_xor(mask_out, tmp_cplimg_mask);
        } else if (p_loc->method == HDRL_BPM_3D_THRESHOLD_RELATIVE) { 
            /* Scaled residual from image using scaled mad*/
            double mad, std_mad;
            cpl_image_get_mad(hdrl_image_get_image(tmp_hdrlimg), &mad);
            if (mad <= 0){
                mad = nextafter(0, 1.0);
            }
            std_mad = CPL_MATH_STD_MAD * mad;
            mask_out = cpl_mask_threshold_image_create(
                    hdrl_image_get_image(tmp_hdrlimg),
                    -(p_loc->kappa_low * std_mad), 
                    (p_loc->kappa_high * std_mad));
            cpl_mask_not(mask_out);
            /*Here we use the bpm from the image after master frame subtraction
             *  - this is not 100 percent clean but a good approximation*/
            cpl_mask_xor(mask_out, tmp_cplimg_mask);
        } else if (p_loc->method == HDRL_BPM_3D_THRESHOLD_ERROR) {
            /* Using scaled pixel error bars to detect bp -
             * we use the propagated error */
            cpl_size nx = hdrl_image_get_size_x(tmp_hdrlimg);
            cpl_size ny = hdrl_image_get_size_y(tmp_hdrlimg);
            mask_out = cpl_mask_new(nx, ny); /*All pixels are now good*/

            for (cpl_size x = 1; x <= nx; ++x) {
                for (cpl_size y = 1; y <= ny; ++y) {
                    int bad_data;
                    hdrl_value v = hdrl_image_get_pixel(tmp_hdrlimg, x, y,
                                                        &bad_data);

                    if (v.data < -(v.error * p_loc->kappa_low) ||
                        v.data > (v.error * p_loc->kappa_high) || bad_data) {
                        cpl_mask_set(mask_out, x, y, CPL_BINARY_1);
                    }
                }
            }
            /*Here we use the bpm from the image after master frame subtraction
             *  - this is not 100 percent clean but a good approximation */
            cpl_mask_xor(mask_out, tmp_cplimg_mask);
        }
        cpl_imagelist_set(imglist_out, cpl_image_new_from_mask(mask_out), var);
        cpl_mask_delete(mask_out);
        hdrl_image_delete(tmp_hdrlimg);
    }

    if (cpl_error_get_code() != CPL_ERROR_NONE) {
        cpl_imagelist_delete(imglist_out);
        imglist_out = NULL;
    }

    hdrl_parameter_delete(collapse_params);

    /* single master broadcasted over list */
    hdrl_image_delete(hdrl_imagelist_unset(masterlist, 0));
    hdrl_imagelist_unwrap(masterlist);

    return imglist_out;
}

/**@}*/

