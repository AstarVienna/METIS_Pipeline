/* $Id: hdrl_flat.c,v 1.29 2013-09-25 11:04:29 jtaylor Exp $
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
 * $Date: 2013-09-25 11:04:29 $
 * $Revision: 1.29 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
-----------------------------------------------------------------------------*/

#include "hdrl_flat.h"
#include <string.h>
#include <assert.h>

/*-----------------------------------------------------------------------------
                                   Static
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**

  @defgroup hdrl_flat   Master Flatfield

  @brief
  This module provides algorithms to compute a master flatfield.
  Several methods are available to deal with different flatfield
  characteristics.

  The routines in this module can be used to derive a high frequency or a low
  frequency master flatfield.

  \par HIGH frequency algorithm:
  The algorithm first smoothes the input images by a median filter and divides
  each input image through the smoothed image. The smoothed images is considered
  to be noiseless i.e. the relative error of the resulting images is the same as
  the one of the input image. Then all residual images are collapsed into a
  single master flatfield. The collapsing can be done with all methods
  currently implemented in hdrl. Moreover, it is also possible to give a static
  mask to the algorithm which e.g. allows the user to distinguish illuminated
  and not illuminated regions. In this case the smoothing procedure is done
  twice, once for the illuminated region and once for the blanked region. This
  ensures that the information of one region does not influence the other
  regions during the smoothing process.

  \par LOW frequency algorithm:
  The algorithm multiplicatively normalizes the input images by the median of
  the image to unity. A static mask can be provided to the algorithm in order
  to define the pixels that should be taken into account when computing the
  normalisation factor. This allows the user to normalize the flatfield e.g.
  only by the illuminated section.  Then all normalized images are collapsed
  into a single master flatfield. The collapsing can be done with all methods
  currently implemented in hdrl. Finally, the master flatfield is smoothed by a
  median filter. The associated error of the final masterframe is the error
  derived via error propagation of the previous steps, i.e. the smoothing itself
  is considered noiseless.


  The calculation is performed by calling the top-level function
  hdrl_flat_compute() and the parameters passed to this function
  can be created by calling hdrl_flat_parameter_create(). Additional one has
  to pass also the collapse parameter created e.g. via
  hdrl_collapse_mean_parameter_create(), ...
  Note that the function will overwrite the input imagelist in order to
  conserve memory. Its contents after the call are undefined and it must be
  deleted by the caller.

 */
/*----------------------------------------------------------------------------*/

/**@{*/

/** @cond PRIVATE */


/*-----------------------------------------------------------------------------
                        FLAT Parameters Definition
 -----------------------------------------------------------------------------*/
typedef struct {
    HDRL_PARAMETER_HEAD;
    cpl_size              filter_size_x ;
    cpl_size              filter_size_y ;
    hdrl_flat_method  method ;
} hdrl_flat_parameter;

/* Parameter type */
static hdrl_parameter_typeobj hdrl_flat_parameter_type = {
    HDRL_PARAMETER_FLAT,                  /* type */
    (hdrl_alloc *)&cpl_malloc,              /* fp_alloc */
    (hdrl_free *)&cpl_free,                 /* fp_free */
    NULL,                                   /* fp_destroy */
    sizeof(hdrl_flat_parameter),          /* obj_size */
};

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Verify basic correctness of the FLAT parameters
  @param    param   FLAT image parameters
  @return   CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_flat_parameter_verify(
                const hdrl_parameter    *   param)
{
    const hdrl_flat_parameter * param_loc = (const hdrl_flat_parameter *)param ;

    cpl_error_ensure(param != NULL, CPL_ERROR_NULL_INPUT,
                     return CPL_ERROR_NULL_INPUT, "NULL Input Parameters");
    cpl_error_ensure(hdrl_flat_parameter_check(param),
                     CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                                     "Expected FLAT image parameter") ;

    cpl_error_ensure(param_loc->method == HDRL_FLAT_FREQ_LOW || 
                     param_loc->method == HDRL_FLAT_FREQ_HIGH,
                     CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                                     "Unsupported method");
    cpl_error_ensure(param_loc->filter_size_x > 0,
                     CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                                     "filter_size_x must be > 0");
    cpl_error_ensure(param_loc->filter_size_y > 0,
                     CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                                     "filter_size_y must be > 0");
    cpl_error_ensure((param_loc->filter_size_x % 2) == 1,
                     CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                                     "filter_size_x must an odd number");
    cpl_error_ensure((param_loc->filter_size_y % 2) == 1,
                     CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                                     "filter_size_y must an odd number");
    return CPL_ERROR_NONE ;
}

/** @endcond PRIVATE */

/* ---------------------------------------------------------------------------*/
/**
  @brief    Creates FLAT Parameters object
  @param    filter_size_x      Smoothing filter size in x-direction
  @param    filter_size_y      Smoothing filter size in y-direction
  @param    method             used method
  @return   The FLAT parameters object.

            It needs to be deallocated with hdrl_parameter_delete()

  @see      hdrl_parameter_delete()
  @see      hdrl_flat_compute()
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter * hdrl_flat_parameter_create(
        cpl_size              filter_size_x,
        cpl_size              filter_size_y,
        hdrl_flat_method  method)
{
    hdrl_flat_parameter * p = (hdrl_flat_parameter *)
               hdrl_parameter_new(&hdrl_flat_parameter_type);
    p->filter_size_x = filter_size_x ;
    p->filter_size_y = filter_size_y ;
    p->method = method ;
    if (hdrl_flat_parameter_verify((hdrl_parameter *)p) != CPL_ERROR_NONE) {
        hdrl_parameter_delete((hdrl_parameter *)p);
        return NULL;
    }
    return (hdrl_parameter *)p;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Check that the parameter is a FLAT parameter
  @param    self The parameter to check
  @return   True or False
 */
/*----------------------------------------------------------------------------*/
cpl_boolean hdrl_flat_parameter_check(const hdrl_parameter * self)
{
    return hdrl_parameter_check_type(self, &hdrl_flat_parameter_type);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the filter_size_x in the FLAT parameter
  @param    p   The FLAT parameter
  @return   The filter_size_x
 */
/*----------------------------------------------------------------------------*/
cpl_size hdrl_flat_parameter_get_filter_size_x(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    return ((const hdrl_flat_parameter *)p)->filter_size_x;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the filter_size_y in the FLAT parameter
  @param    p   The FLAT parameter
  @return   The filter_size_y
 */
/*----------------------------------------------------------------------------*/
cpl_size hdrl_flat_parameter_get_filter_size_y(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    return ((const hdrl_flat_parameter *)p)->filter_size_y;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the method in the FLAT parameter
  @param    p   The FLAT parameter
  @return   The method
 */
/*----------------------------------------------------------------------------*/
hdrl_flat_method hdrl_flat_parameter_get_method(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    return ((const hdrl_flat_parameter *)p)->method;
}

/*----------------------------------------------------------------------------*/
/**
  @brief Create a parameter list for the FLAT computation
  @param base_context    base context of parameter (e.g. recipe name)
  @param prefix          prefix of parameter, may be empty string
  @param defaults        default values

  Creates a parameterlist with the FLAT parameters:
    - base_context.prefix.filter_size_x
    - base_context.prefix.filter_size_y
    - base_context.prefix.method
  The CLI aliases omit the base_context.
 */
/*----------------------------------------------------------------------------*/
cpl_parameterlist * hdrl_flat_parameter_create_parlist(
        const char              *   base_context,
        const char              *   prefix,
        const hdrl_parameter    *   defaults)
{
    cpl_ensure(prefix && base_context && defaults, CPL_ERROR_NULL_INPUT, NULL);
    char                *   name ;
    cpl_parameterlist   *   parlist = cpl_parameterlist_new();
    cpl_parameter       *   par ;
    char                *   context =
        hdrl_join_string(".", 2, base_context, prefix);

    cpl_size filter_size_x_def = hdrl_flat_parameter_get_filter_size_x(defaults) ;
    cpl_size filter_size_y_def = hdrl_flat_parameter_get_filter_size_y(defaults) ;

    hdrl_flat_method method_def =hdrl_flat_parameter_get_method(defaults);
    cpl_ensure( method_def == HDRL_FLAT_FREQ_LOW || method_def == HDRL_FLAT_FREQ_HIGH,
    			CPL_ERROR_ILLEGAL_INPUT, NULL) ;

    const char *method_str;
    if (method_def == HDRL_FLAT_FREQ_LOW) {
        method_str = "low";
    } else if (method_def == HDRL_FLAT_FREQ_HIGH) {
        method_str = "high";
    } else {
    	method_str = "";
    }

    /* --prefix.filter_size_x */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "filter-size-x", base_context,
            "Smoothing filter size in x-direction.", CPL_TYPE_INT,
            (int)filter_size_x_def) ;

    /* --prefix.filter_size_y */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "filter-size-y", base_context,
            "Smoothing filter size in y-direction.", CPL_TYPE_INT,
            (int)filter_size_y_def) ;

    /* --prefix.method */
    name = hdrl_join_string(".", 2, context, "method");
    par = cpl_parameter_new_enum(name, CPL_TYPE_STRING, 
            "Method to use for the master flatfield calculation", context, method_str,
            2, "low", "high");
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
  @brief Parse a parameterlist to create input parameters for the FLAT
  @param parlist        parameter list to parse
  @param prefix         prefix of parameter name
  @return   Input parameters for the FLAT computation

  Reads a parameterlist in order to create FLAT image parameters.
  Expects a parameterlist containing:
   - prefix.filter_size_x
   - prefix.filter_size_y
   - prefix.method
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_flat_parameter_parse_parlist(
        const cpl_parameterlist *   parlist,
        const char              *   prefix)
{
    cpl_ensure(prefix && parlist, CPL_ERROR_NULL_INPUT, NULL);
    char                *   name;
    const cpl_parameter *   par;
    const char          *   tmp_str ;
    cpl_size                filter_size_x;
    cpl_size                filter_size_y;
    hdrl_flat_method      method;
   
    /* --filter_size_x */
    name = hdrl_join_string(".", 2, prefix, "filter-size-x");
    par=cpl_parameterlist_find_const(parlist, name);
    filter_size_x = cpl_parameter_get_int(par);
    cpl_free(name) ;

    /* --filter_size_y */
    name = hdrl_join_string(".", 2, prefix, "filter-size-y");
    par=cpl_parameterlist_find_const(parlist, name);
    filter_size_y = cpl_parameter_get_int(par);
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
    if(!strcmp(tmp_str, "low")) {
        method = HDRL_FLAT_FREQ_LOW ;
    } else if(!strcmp(tmp_str, "high")) {
        method = HDRL_FLAT_FREQ_HIGH ;
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
        return hdrl_flat_parameter_create(filter_size_x, filter_size_y, method) ;
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief compute high or low frequency master flat with median filtering
 *
 * @param hdrl_data          input flats, will be overwritten!
 * @param stat_mask          input mask to select the regions for
 *                           statistics or smoothing
 * @param collapse_params    parameter controlling the collapse algorithm
 * @param flat_params        parameter controlling the flatfield algorithm
 * @param master             returned masterflat
 * @param contrib_map        returned contribution map
 *
 * \par The algorithms are described in the master flatfield module documentation
 *
 * @note the function will overwrite the input imagelist in order to
 * conserve memory. Its contents after the call are undefined and it must be
 * deleted by the caller.
 */
/* ---------------------------------------------------------------------------*/

cpl_error_code hdrl_flat_compute(
                hdrl_imagelist                       *  hdrl_data,
                const cpl_mask                       *  stat_mask,
                const hdrl_parameter                 *  collapse_params,
                hdrl_parameter                       *  flat_params,
                hdrl_image                           ** master,
                cpl_image                            ** contrib_map)
{
    hdrl_image * comb_img = NULL;
    cpl_image * comb_ctr = NULL;
    int lowf = 0;

    /* Check Entries */
    cpl_error_ensure(hdrl_data != NULL, CPL_ERROR_NULL_INPUT,
                     return CPL_ERROR_NULL_INPUT, "No flatfields found");
    cpl_error_ensure(collapse_params != NULL, CPL_ERROR_NULL_INPUT,
                     return CPL_ERROR_NULL_INPUT, "No collapsing parameter");
    cpl_error_ensure(flat_params != NULL, CPL_ERROR_NULL_INPUT,
                     return CPL_ERROR_NULL_INPUT, "No flatfield parameter");

    if (hdrl_flat_parameter_verify(flat_params) != CPL_ERROR_NONE)
        return cpl_error_get_code();

    /* Local Usage Parameters */
    const hdrl_flat_parameter * p_loc = (hdrl_flat_parameter *)flat_params ;
    cpl_size filter_size_x = p_loc->filter_size_x;
    cpl_size filter_size_y = p_loc->filter_size_y;

    if (p_loc->method == HDRL_FLAT_FREQ_LOW) {
        lowf = 1;
    } else if (p_loc->method == HDRL_FLAT_FREQ_HIGH) {
        lowf = 0;
    }

    cpl_mask *  kernel = cpl_mask_new(filter_size_x, filter_size_y) ;
    cpl_mask_not(kernel);

    /* NOTE: The filtered image is supposed to be noiseless */

    for (cpl_size i = 0; i < hdrl_imagelist_get_size(hdrl_data); i++) {

        /* TODO Only extract the mask if there is a mask and only combine
         * the masks if there are both masks  */

        cpl_image *img     = hdrl_image_get_image(hdrl_imagelist_get(hdrl_data, i));
        cpl_image *img_err = hdrl_image_get_error(hdrl_imagelist_get(hdrl_data, i));

        cpl_mask  *img_mask      = cpl_mask_duplicate(cpl_image_get_bpm(img));
        cpl_mask  *img_mask_orig = cpl_mask_duplicate(cpl_image_get_bpm(img));

        if (lowf == 1) {
            double median;
            /* no error propagation as the median error is small and also
             * consistency with the high freq variant which can't propagate
             * error */
            if (stat_mask != NULL) {
                cpl_mask_or(img_mask, stat_mask);
                /* Apply combined mask */
                cpl_image_reject_from_mask(img, img_mask);
                median = cpl_image_get_median(img);
                cpl_msg_debug(cpl_func, "Median of the flat: %g", median);
                /* restore the original mask */
                cpl_image_reject_from_mask(img, img_mask_orig);
            }
            else {
                median = cpl_image_get_median(img);
                cpl_msg_debug(cpl_func, "Median of the flat: %g", median);
            }
            cpl_image_divide_scalar(img, median);
            cpl_image_divide_scalar(img_err, median);
        }
        else {
            cpl_image * img_filtered;
            if (stat_mask != NULL) {
                cpl_image * img_filtered2;

                /* Algorithm takes into account border effects introduced by the
                 *  static mask, i.e it smoothes the good and bad region
                 *  seperately in order to make sure that they are not
                 *  correlated:
                 *  1) Smooth the part of the image declared to belong together
                 *  by the static mask, e.g. the iluminated part of the image
                 *  2) Smooth the remaining part of the image, e.g. the not
                 *  iluminated part of the image
                 *  3) combine 1) and 2) in a single image
                 *  */

                /*A) Filter the image declared as good by the static mask */

                cpl_mask_or(img_mask, stat_mask);
                cpl_image_reject_from_mask(img, img_mask);
                img_filtered = hdrl_parallel_filter_image(img, NULL, kernel,
                                                          CPL_FILTER_MEDIAN);

                /*The filtering extents the image into the bad pixel part, thus
                 * we must reapply the static mask to preserve the sharp
                 * cutoff */
                cpl_mask_or(cpl_image_get_bpm(img_filtered), stat_mask);

                /*B) Filter the image declared as bad by the static mask */

                /* Create an inverted mask */
                cpl_mask * stat_mask_inverted=cpl_mask_duplicate(stat_mask);
                cpl_mask_not(stat_mask_inverted);
                /* Restore the original bad pixel mask into img_mask */
                cpl_mask_delete(img_mask);
                img_mask = cpl_mask_duplicate(img_mask_orig);
                /* Filter the image */
                cpl_mask_or(img_mask, stat_mask_inverted);
                cpl_image_reject_from_mask(img, img_mask);
                img_filtered2 = hdrl_parallel_filter_image(img, NULL, kernel,
                                                           CPL_FILTER_MEDIAN);

                /*The filtering extents the image into the bad pixel part, thus
                 * we must reapply the static mask to preserve the sharp
                 * cutoff */
                cpl_mask_or(cpl_image_get_bpm(img_filtered2),
                            stat_mask_inverted);
                cpl_mask_delete(stat_mask_inverted);

                /*C) Combine the two images into one image */

                cpl_image_fill_rejected(img_filtered, 0.);
                cpl_image_fill_rejected(img_filtered2, 0.);
                /* remove the static masks to join images */
                cpl_mask * img_filtered1_mask =
                                cpl_image_unset_bpm(img_filtered);
                cpl_mask * img_filtered2_mask =
                                cpl_image_unset_bpm(img_filtered2);

                cpl_image_add(img_filtered, img_filtered2);
                cpl_image_delete(img_filtered2);

                /* & of static make should be input bpm */
                cpl_mask_and(img_filtered1_mask, img_filtered2_mask);
                assert(memcmp(cpl_mask_get_data(img_filtered1_mask),
                              cpl_mask_get_data(img_mask_orig),
                              hdrl_get_image_npix(img)) == 0);

                /* apply original bpm back onto filtered image */
                cpl_image_reject_from_mask(img_filtered, img_filtered1_mask);
                cpl_mask_delete(img_filtered1_mask);
                cpl_mask_delete(img_filtered2_mask);
            }
            else {
                /* Filter the image */
                cpl_image_reject_from_mask(img, img_mask);
                /* currently only tested for  CPL_BORDER_FILTER */
                img_filtered = hdrl_parallel_filter_image(img, NULL, kernel,
                                CPL_FILTER_MEDIAN);
            }

            /* restore the original mask */
            cpl_image_reject_from_mask(img, img_mask_orig);
            cpl_image_reject_from_mask(img_err, img_mask_orig);

            cpl_image_divide(img, img_filtered);
            cpl_image_divide(img_err, img_filtered);
            cpl_image_delete(img_filtered);
        }

        /* make sure that the error is positive */
        /* TODO Check if this is following the error propagation law */
        cpl_image_abs(img_err);

        cpl_mask_delete(img_mask);
        cpl_mask_delete(img_mask_orig);
   }

    cpl_msg_info(cpl_func, "Combining the normalized flatfields generating"
                    " the master-flatfield");

    hdrl_imagelist_collapse(hdrl_data, collapse_params, &comb_img, &comb_ctr);

    if (lowf == 1){
        cpl_image *img_filtered = hdrl_parallel_filter_image(
        	hdrl_image_get_image_const(comb_img), NULL, kernel, CPL_FILTER_MEDIAN);
        *master = hdrl_image_create(img_filtered, hdrl_image_get_error(comb_img));
        *contrib_map = comb_ctr;
        hdrl_image_delete(comb_img);
        cpl_image_delete(img_filtered);
    }
    else {
        *master = comb_img;
        *contrib_map = comb_ctr;
    }

    cpl_mask_delete(kernel);
    cpl_msg_indent_less();
    return cpl_error_get_code();
}


/**@}*/

