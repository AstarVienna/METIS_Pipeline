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

#include "hdrl_overscan.h"
#include "hdrl_overscan_defs.h"
#include "hdrl_sigclip.h"
#include "hdrl_utils.h"
#include "hdrl_collapse.h"

#include <cpl.h>
#include <math.h>
#include <string.h>

/*-----------------------------------------------------------------------------
                                   Static
 -----------------------------------------------------------------------------*/

static hdrl_overscan_compute_result * hdrl_overscan_compute_result_create(
        void);
static cpl_error_code hdrl_overscan_compute_result_verify(
        const hdrl_overscan_compute_result *);
static hdrl_overscan_correct_result * hdrl_overscan_correct_result_create(
        void);
static cpl_error_code  hdrl_overscan_compute_chi_square(const cpl_image *,
        const cpl_image *, const double, double *, double *);
static cpl_error_code hdrl_overscan_reduce_image_to_scalar(
        hdrl_collapse_imagelist_to_vector_t *, cpl_image *, cpl_image *, 
        double *, double *, cpl_size *, void **) ;
static void hdrl_overscan_parameter_destroy(void * param);

/*----------------------------------------------------------------------------*/
/* INCLUDED FROM doc/Func_Overscan.tex */
/**
  @defgroup hdrl_overscan   Overscan Computation and Correction
 
  @brief
  This module contains functionality to compute and correct the overscan level
  of CCD image.


The overscan (or prescan) region on a CCD can consist of physical
pixels on the detector that are not illuminated, or it can consist of a set
of virtual pixels created by reading out the serial register either before or
after transferring the charge from the CCD for each column/row.
A detector may have multiple regions (amplifiers) and each one usually has
an associated overscan region.

This module is intended to be applied to a single overscan region.

The key parameter is box_hsize. When box_hsize is zero, then the
overscan correction is calculated without smoothing over adjacent rows/columns.
However, this may lead to a noisy overscan correction vector.
Increasing box_hsize will increase the S/N of the values in the overscan
correction vector, but will decrease the ability of the overscan correction
to correct for rapid spatial changes in the bias level.
A balance must be reached which will likely depend on the detector under
consideration.

A typical overscan correction may be essentially independent of row/column,
or it may vary smoothly or with abrupt gradient changes. Hence the
implementation of a smoothing method as opposed to the fitting of an
analytical function.

When choosing the overscan region, note that the first few rows/columns
that are read out after the image area will have an artificially high bias
level due to the charge transfer efficiency not being 100%. It is
recommended therefore to not include the 2-3 rows/columns right next
to the image region in the overscan region.

An interesting phenomenon is that a very bright star/object may increase the
bias level for the few rows/columns that it covers. Hence it is not advisable
to set box_hsize to a value that is much larger than the image
point-spread-function FWHM.

An overscan correction is the correct way to correct for the detector bias
level. It is preferable to do an overscan correction instead of using
bias frames to determine a bias level.

This module allows the computation of the overscan correction for an image
from a predefined overscan region. It also provides a function that applies
the overscan correction to the image, using the result of the overscan
computation.
  
    \image html overscan_example.png
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*-----------------------------------------------------------------------------
                        Overscan Parameters Definition
 -----------------------------------------------------------------------------*/
typedef struct {
    HDRL_PARAMETER_HEAD;
    hdrl_direction              correction_direction; 
    double                      ccd_ron;
    int                         box_hsize;
    hdrl_parameter          *   collapse;
    hdrl_parameter          *   rect_region;
} hdrl_overscan_parameter;

/* Parameter type */
static hdrl_parameter_typeobj hdrl_overscan_parameter_type = {
    HDRL_PARAMETER_OVERSCAN,                        /* type */
    (hdrl_alloc *)&cpl_malloc,                      /* fp_alloc */
    (hdrl_free *)&cpl_free,                         /* fp_free */
    (hdrl_free *)&hdrl_overscan_parameter_destroy,  /* fp_destroy */
    sizeof(hdrl_overscan_parameter),                /* obj_size */
};

/* ---------------------------------------------------------------------------*/
/**
  @brief    Creates Overscan Parameters object
  @param    correction_direction    HDRL_X_AXIS or HDRL_Y_AXIS
  @param    ccd_ron                 The CCD read out noise
  @param    box_hsize               The running box half size
  @param    collapse                The collpase method parameters
  @param    rect_region             The overscan computation region
  @return   The overscan parameters object. It needs to be deallocated
            with hdrl_parameter_delete() or _destroy()
  @see      hdrl_parameter_delete()
  @see      hdrl_parameter_destroy()
  @see      hdrl_overscan_compute()
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter * hdrl_overscan_parameter_create(
        hdrl_direction      correction_direction,
        double              ccd_ron,
        int                 box_hsize,
        hdrl_parameter  *   collapse,
        hdrl_parameter  *   rect_region)
{
    hdrl_overscan_parameter * p = (hdrl_overscan_parameter *)
               hdrl_parameter_new(&hdrl_overscan_parameter_type);
    p->correction_direction = correction_direction ;
    p->ccd_ron = ccd_ron ;
    p->box_hsize = box_hsize ;
    p->collapse = collapse ;
    p->rect_region = rect_region ;
    return (hdrl_parameter *)p;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Verify basic correctness of the Overscan parameters
  @param    param   Overscan parameters
  @param    nx      required X region size, set to < 0 to skip check
  @param    ny      required Y region size, set to < 0 to skip check
  @return   CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_overscan_parameter_verify(
        const hdrl_parameter    *   param,
        cpl_size                    nx,
        cpl_size                    ny) 
{
    const hdrl_overscan_parameter * param_loc = 
        (const hdrl_overscan_parameter *)param ;

    cpl_error_ensure(param != NULL, CPL_ERROR_NULL_INPUT,
            return CPL_ERROR_NULL_INPUT, "NULL Input Parameters");
    cpl_error_ensure(hdrl_overscan_parameter_check(param),
            CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
            "Expected Overscan parameter") ;
    
    cpl_error_ensure(param_loc->ccd_ron >= 0, CPL_ERROR_ILLEGAL_INPUT,
            return CPL_ERROR_ILLEGAL_INPUT,
            "CCD read out noise (%g) must be >= 0", param_loc->ccd_ron);
    cpl_error_ensure(param_loc->box_hsize >= 0 ||
                     param_loc->box_hsize == HDRL_OVERSCAN_FULL_BOX,
                     CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                     "half box size (%d) must be >= 0 or -1", 
                     param_loc->box_hsize);
    cpl_error_ensure(param_loc->correction_direction == HDRL_X_AXIS ||
                     param_loc->correction_direction == HDRL_Y_AXIS,
                     CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                     "correction_direction must be HDRL_X_AXIS or HDRL_Y_AXIS");
    
    if (hdrl_collapse_parameter_is_sigclip(param_loc->collapse)) {
        cpl_error_ensure(
                hdrl_collapse_sigclip_parameter_verify(param_loc->collapse)
                == CPL_ERROR_NONE, CPL_ERROR_ILLEGAL_INPUT,
                return CPL_ERROR_ILLEGAL_INPUT,
                "Illegal Collapse Sigclip parameters");
    } 
    if (hdrl_collapse_parameter_is_minmax(param_loc->collapse)) {
        cpl_error_ensure(
                hdrl_collapse_minmax_parameter_verify(param_loc->collapse)
                == CPL_ERROR_NONE, CPL_ERROR_ILLEGAL_INPUT,
                return CPL_ERROR_ILLEGAL_INPUT,
                "Illegal Collapse Minmax parameters");
    } 
    if (hdrl_collapse_parameter_is_mode(param_loc->collapse)) {
        cpl_error_ensure(
                hdrl_collapse_mode_parameter_verify(param_loc->collapse)
                == CPL_ERROR_NONE, CPL_ERROR_ILLEGAL_INPUT,
                return CPL_ERROR_ILLEGAL_INPUT,
                "Illegal Collapse Mode parameters");
    } 
    cpl_error_ensure(
            hdrl_rect_region_parameter_verify(param_loc->rect_region, -1, -1)
            == CPL_ERROR_NONE, CPL_ERROR_ILLEGAL_INPUT,
            return CPL_ERROR_ILLEGAL_INPUT,
            "Illegal Rect Region parameters");

    cpl_error_ensure(hdrl_collapse_parameter_is_mean(param_loc->collapse) ||
            hdrl_collapse_parameter_is_weighted_mean(param_loc->collapse) ||
            hdrl_collapse_parameter_is_median(param_loc->collapse) ||
            hdrl_collapse_parameter_is_sigclip(param_loc->collapse) ||
            hdrl_collapse_parameter_is_minmax(param_loc->collapse) ||
            hdrl_collapse_parameter_is_mode(param_loc->collapse),
            CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
            "Only supported methods are MEAN, WEIGHTED_MEAN, MEDIAN, SIGCLIP,"
            " MINMAX and MODE");

    /* The region must be contained in the image */
    if (nx > 0) {
        cpl_size region_llx = hdrl_rect_region_get_llx(param_loc->rect_region);
        cpl_size region_urx = hdrl_rect_region_get_urx(param_loc->rect_region);
        cpl_error_ensure(
                region_llx>=1 && region_urx <= nx,
                CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT, 
                "Region (%d) exceeds source (%d) size in the X dir.",
                (int)region_urx, (int)nx);
    }
    if (ny > 0) {
        cpl_size region_lly = hdrl_rect_region_get_lly(param_loc->rect_region);
        cpl_size region_ury = hdrl_rect_region_get_ury(param_loc->rect_region);
        cpl_error_ensure(
                region_lly>=1 && region_ury <= ny,
                CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT, 
                "Region (%d) exceeds source (%d) size in the Y dir.",
                (int)region_ury, (int)ny);
    }
    return CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Check that the parameter is an Overscan parameter
  @param    self The parameter to check
  @return   True or False
 */
/*----------------------------------------------------------------------------*/
cpl_boolean hdrl_overscan_parameter_check(const hdrl_parameter * self)
{
    return hdrl_parameter_check_type(self, &hdrl_overscan_parameter_type);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the Correction Direction in the Overscan Parameter
  @param    p The Overscan parameter
  @return   The correction direction
 */
/*----------------------------------------------------------------------------*/
hdrl_direction hdrl_overscan_parameter_get_correction_direction(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, HDRL_UNDEFINED_AXIS);
    return ((const hdrl_overscan_parameter *)p)->correction_direction;
}
/*----------------------------------------------------------------------------*/
/**
  @brief    Access the CCD read out noise in the Overscan Parameter
  @param    p The Overscan parameter
  @return   The CCD read out noise
 */
/*----------------------------------------------------------------------------*/
double hdrl_overscan_parameter_get_ccd_ron(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.0) ;
    return ((const hdrl_overscan_parameter *)p)->ccd_ron ;
}
/*----------------------------------------------------------------------------*/
/**
  @brief    Access the Box Half Size in the Overscan Parameter
  @param    p The Overscan parameter
  @return   The box half size
 */
/*----------------------------------------------------------------------------*/
int hdrl_overscan_parameter_get_box_hsize(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.0) ;
    return ((const hdrl_overscan_parameter *)p)->box_hsize ;
}
/*----------------------------------------------------------------------------*/
/**
  @brief    Access the collapse method parameters in the Overscan Parameter
  @param    p The Overscan parameter
  @return   The collapse method parameters
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_overscan_parameter_get_collapse(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, NULL) ;
    return ((const hdrl_overscan_parameter *)p)->collapse;
}
/*----------------------------------------------------------------------------*/
/**
  @brief    Access the Overscan Region parameters in the Overscan Parameter
  @param    p The Overscan parameter
  @return   The Overscan region parameters
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_overscan_parameter_get_rect_region(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, NULL) ;
    return ((const hdrl_overscan_parameter *)p)->rect_region;
}

/* ---------------------------------------------------------------------------*/
/**
  @brief Create parameter list for the Overscan computation
  @param base_context    base context of parameter (e.g. recipe name)
  @param prefix          prefix of parameter, may be empty string
  @param corr_dir_def    default correction direction value
  @param box_hsize_def   default box hsize value
  @param ccd_ron_def     default ccd ron value
  @param rect_region_def default overscan region parameters
  @param method_def      default collapse method value
  @param sigclip_def     default sigma-clipping parameters 
  @param minmax_def      default minmax-clipping parameters 
  @param mode_def      default mode-clipping parameters 

  @see hdrl_rect_region_parameter_create_parlist()
  @see hdrl_collapse_parameter_create_parlist()
  Creates a parameterlist with the overscan computation parameters:

    - base_context.prefix.correction-direction
    - base_context.prefix.box-hsize
    - base_context.prefix.ccd-ron
    - base_context.prefix.calc-*
    - base_context.prefix.collapse.*

  The CLI aliases omit the base_context.
 */
/* ---------------------------------------------------------------------------*/
cpl_parameterlist * hdrl_overscan_parameter_create_parlist(
        const char      *base_context,
        const char      *prefix,
        const char      *corr_dir_def,
        int              box_hsize_def,
        double           ccd_ron_def,
        hdrl_parameter  *rect_region_def,
        const char      *method_def,
        hdrl_parameter  *sigclip_def,
        hdrl_parameter  *minmax_def,
	hdrl_parameter  *mode_def)
{
    cpl_ensure(prefix && base_context && rect_region_def && sigclip_def
	       && minmax_def && mode_def, CPL_ERROR_NULL_INPUT, NULL);

    cpl_ensure(hdrl_rect_region_parameter_check(rect_region_def)
    		    && hdrl_collapse_parameter_is_sigclip(sigclip_def)
				&& hdrl_collapse_parameter_is_minmax( minmax_def)
				&& hdrl_collapse_parameter_is_mode(mode_def),
       		CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    char                *   name ;
    cpl_parameterlist   *   parlist = cpl_parameterlist_new();
    cpl_parameter       *   par ;
    char                *   context = 
        hdrl_join_string(".", 2, base_context, prefix);

    /* --prefix.correction_direction */
    name = hdrl_join_string(".", 2, context, "correction-direction");
    par = cpl_parameter_new_enum(name, CPL_TYPE_STRING, "Correction Direction", 
            context, corr_dir_def, 2, "alongX", "alongY");
    cpl_free(name);
    name = hdrl_join_string(".", 2, prefix, "correction-direction");
    cpl_parameter_set_alias(par, CPL_PARAMETER_MODE_CLI, name);
    cpl_parameter_disable(par, CPL_PARAMETER_MODE_ENV);
    cpl_free(name);
    cpl_parameterlist_append(parlist, par);
 
    /* --prefix.box-hsize */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "box-hsize", base_context,
            "Half size of running box in pixel, -1 for full overscan region", 
            CPL_TYPE_INT, box_hsize_def) ;

    /* --prefix.ccd-ron */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "ccd-ron", base_context,
            "Readout noise in ADU", CPL_TYPE_DOUBLE, ccd_ron_def) ;

    /* Create Overscan Computation Region parameters */
    /* --prefix.overscan.calc-xxx */
    cpl_parameterlist * os_comp_reg = hdrl_rect_region_parameter_create_parlist(
                base_context, prefix, "calc-", rect_region_def);
    for (cpl_parameter * p = cpl_parameterlist_get_first(os_comp_reg) ;       
            p != NULL; p = cpl_parameterlist_get_next(os_comp_reg)) 
        cpl_parameterlist_append(parlist,cpl_parameter_duplicate(p));
    cpl_parameterlist_delete(os_comp_reg);
    
    /* Overscan Collapsing related parameters */
    /* --prefix.collapse.xxx */
    name = hdrl_join_string(".", 2, prefix, "collapse");
    cpl_parameterlist * pcollapse = hdrl_collapse_parameter_create_parlist(
            base_context, name, method_def, sigclip_def, minmax_def, mode_def) ;
    cpl_free(name);
    for (cpl_parameter * p = cpl_parameterlist_get_first(pcollapse) ;
            p != NULL; p = cpl_parameterlist_get_next(pcollapse))
        cpl_parameterlist_append(parlist,cpl_parameter_duplicate(p));
    cpl_parameterlist_delete(pcollapse);

    cpl_free(context);

    if (cpl_error_get_code()) {
        cpl_parameterlist_delete(parlist);
        return NULL;
    }

    return parlist;
}

/* ---------------------------------------------------------------------------*/
/**
  @brief Parse parameterlist to create input parameters for the Overscan method
  @param parlist    parameter list to parse
  @param prefix     prefix of parameter name
  @return   Input parameters for the Overscan Computation
  Reads a Parameterlist in order to create overscan parameters.
  Expects a parameterlist containing:
  - prefix.correction-direction
  - prefix.box-hsize
  - prefix.ccd-ron
  - prefix.calc-llx
  - prefix.calc-lly
  - prefix.calc-urx
  - prefix.calc-ury
  - prefix.collapse.method
  - prefix.collapse.sigclip.kappa-low
  - prefix.collapse.sigclip.kappa-high
  - prefix.collapse.sigclip.niter
  - prefix.collapse.minmax.nlow
  - prefix.collapse.minmax.nhigh
  - prefix.collapse.mode.histo-min
  - prefix.collapse.mode.histo-max
  - prefix.collapse.mode.bin-size
  - prefix.collapse.mode.method
  - prefix.collapse.mode.error_niter

 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter * hdrl_overscan_parameter_parse_parlist(
        const cpl_parameterlist *   parlist,
        const char              *   prefix)
{
    cpl_ensure(prefix && parlist, CPL_ERROR_NULL_INPUT, NULL);
    hdrl_direction          corr_dir_param = HDRL_UNDEFINED_AXIS ;
    int                     box_hsize = 0 ;
    double                  ccd_ron = 0. ;
    hdrl_parameter      *   os_collapse_params = NULL;
    hdrl_parameter      *   os_region_params = NULL;
    char                *   name ;

    /* --correction-direction */
    name = hdrl_join_string(".", 2, prefix, "correction-direction");
    const cpl_parameter *par = cpl_parameterlist_find_const(parlist, name);
    const char *correction_direction = cpl_parameter_get_string(par);
    if (correction_direction == NULL) {
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                              "Parameter %s not found", name);
        cpl_free(name);
        return NULL;
    }
    if(!strcmp(correction_direction, "alongX")) {
        corr_dir_param = HDRL_X_AXIS;
    } else if(!strcmp(correction_direction, "alongY")) {
        corr_dir_param = HDRL_Y_AXIS;
    }
    cpl_free(name) ;
 
    /* --box-hsize */
    name = hdrl_join_string(".", 2, prefix, "box-hsize");
    par=cpl_parameterlist_find_const(parlist, name);
    box_hsize = cpl_parameter_get_int(par);
    cpl_free(name) ;

    /* --ccd-ron */
    name = hdrl_join_string(".", 2, prefix, "ccd-ron");
    par=cpl_parameterlist_find_const(parlist, name);
    ccd_ron = cpl_parameter_get_double(par);
    cpl_free(name) ;

    if (cpl_error_get_code()) {
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                              "Error while parsing parameterlist "
                              "with prefix %s", prefix);
        return NULL;
    }

    /* --calc-* */
    os_region_params = hdrl_rect_region_parameter_parse_parlist(parlist,
            prefix, "calc-");

    /* --collapse.* */
    name = hdrl_join_string(".", 2, prefix, "collapse");
    os_collapse_params = hdrl_collapse_parameter_parse_parlist(parlist, name) ;
    cpl_free(name) ;

    if (cpl_error_get_code()) {
        hdrl_parameter_destroy(os_region_params);
        hdrl_parameter_destroy(os_collapse_params);
        return NULL;
    }
    else {
        return hdrl_overscan_parameter_create(corr_dir_param, ccd_ron,
                                              box_hsize, os_collapse_params,
                                              os_region_params);
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get imagelist to vector reduction object
 *
 * @param cpse              parameter that determines the reduction type
 * @param overscan_sub_ima  overscan image for default parameters
 * @param reduce            pointer to reduction object storage
 * @return cpl_error_code
 *
 * pointer stored in reduce must be deleted with
 * hdrl_collapse_imagelist_to_vector_delete()
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code
get_reduction(const hdrl_parameter * cpse,
              const cpl_image * overscan_sub_ima,
              hdrl_collapse_imagelist_to_vector_t ** reduce)
{
    if (hdrl_collapse_parameter_is_mean(cpse)) {
        *reduce = hdrl_collapse_imagelist_to_vector_mean();
    } else if (hdrl_collapse_parameter_is_weighted_mean(cpse)) {
        *reduce = hdrl_collapse_imagelist_to_vector_weighted_mean();
    } else if (hdrl_collapse_parameter_is_median(cpse)) {
        *reduce = hdrl_collapse_imagelist_to_vector_median();
    } else if (hdrl_collapse_parameter_is_sigclip(cpse)) {
        double kappa =
            sqrt(log(CX_MAX(hdrl_get_image_good_npix(overscan_sub_ima), 1)));
        double kappa_low = hdrl_collapse_sigclip_parameter_get_kappa_low(cpse);
        double kappa_high =
            hdrl_collapse_sigclip_parameter_get_kappa_high(cpse);
        int niter = hdrl_collapse_sigclip_parameter_get_niter(cpse);
        if (kappa_low <= 0)     kappa_low = kappa;
        if (kappa_high <= 0)    kappa_high = kappa;
        *reduce = hdrl_collapse_imagelist_to_vector_sigclip(kappa_low,
                                                            kappa_high, niter);
    } else if (hdrl_collapse_parameter_is_minmax(cpse)) {
        double nlow = hdrl_collapse_minmax_parameter_get_nlow(cpse);
        double nhigh = hdrl_collapse_minmax_parameter_get_nhigh(cpse);
        if (nlow <= 0)     nlow = 0;
        if (nhigh <= 0)    nhigh = 0;
        *reduce = hdrl_collapse_imagelist_to_vector_minmax(nlow, nhigh);
	
    } else if (hdrl_collapse_parameter_is_mode(cpse)) {
	double histo_min = hdrl_collapse_mode_parameter_get_histo_min(cpse);
	double histo_max = hdrl_collapse_mode_parameter_get_histo_max(cpse);
	double bin_size = hdrl_collapse_mode_parameter_get_bin_size(cpse);
	hdrl_mode_type method = hdrl_collapse_mode_parameter_get_method(cpse);
	cpl_size error_niter = hdrl_collapse_mode_parameter_get_error_niter(cpse);
	*reduce = hdrl_collapse_imagelist_to_vector_mode(histo_min, histo_max,
							 bin_size, method,
							 error_niter);
    }

    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Overscan correction computation
  @param    source  input image containing the Overscan region
  @param    params  Overscan computation parameters
  @return   hdrl_overscan_compute_result structure containing the results
  of the computation. It must be de-allocated by the caller using 
  hdrl_overscan_compute_result_delete().
  In case of error, NULL is returned and an error code is set.
  @see      hdrl_overscan_compute_result_delete()

  The source image may contain more than the overscan region that is
  actually needed by the computation.

  params->rect_region defines the overscan region in the source image. The 
  bad pixels that might be present in the image are taken into account (i.e.
  excluded from the computations).
  
  params->correction_direction is HDRL_X_AXIS (resp. HDRL_Y_AXIS) if the
  overscan region has to be collapsed along the X (resp. Y) axis in
  order to create the 1D resulting images (correction, error, contribution, the
  \f$\chi^{2}\f$ (chi2) and
  the reduced \f$\chi^{2}\f$ (red_chi2), additionally sigclip_reject_low and 
  _high if the sigma-clipping collapsing is used).

  Each pixel of the resulting 1D images are computed from a running
  sub-window of the overscan region. params->box_hsize defines the half size
  of the sub-window used for the computation (in the direction orthogonal to 
  params->correction_direction). If params->box_hsize value is 
  HDRL_OVERSCAN_FULL_BOX, the calculation is done on the whole overscan 
  region instead of a running sub-window. In this case, all pixels of the 
  resulting 1D images will be identical.

  params->collapse methods can be mean, weighted mean, median or sigclip, 
  i.e. the
  collapse parameter can be created with
  hdrl_collapse_mean_parameter_create(),
  hdrl_collapse_weighted_mean_parameter_create(),
  hdrl_collapse_median_parameter_create() or 
  hdrl_collapse_sigclip_parameter_create().

  params->ccd_ron is the CCD read-out noise. The parameter is mandatory,
  must be strictly non-negative. It is used for the error, the chi2 and the
  red_chi2 computation.

  In case the sigma-clipping collapsing method is used, a sigma clipping
  iterative rejection is applied in the overscan sub-window before the 
  computation of the results.

  The output hdrl_overscan_compute_result structure (in the following
  named 'out') contains the following members:

  out.correction is a 1D HDRL image of type double.
  Its image contains the overscan correction values computed from the good
  pixels of the running sub-window (mean, median or mean after rejection, 
  depending on the used collapsing method)

  Its error contains \f$\frac{ccd\_ron}{\sqrt{contribution}}\f$ for the
  mean, weighted mean and sigma-cliping methods.
  In case the method is the median one, it would contain 
  \f$\sqrt{\frac{\pi}{2}}\times\frac{ccd\_ron}{\sqrt{contribution}}\f$
  if contribution is strictly greater than 2 pixels and 
  \f$\frac{ccd\_ron}{\sqrt{contribution}}\f$ when the contribution is 1 or 
  2 pixels. 

  out.contribution is a 1D CPL image of type integer.
  It contains the number of good pixels of the input running sub-window
  for the mean and median methods, and the remaining good pixels after the 
  rejection for the sigma-clipping method.

  out.chi2 and out.red_chi2 are 1D CPL images of type double.

  out.chi2 contains the sum over the good pixels of the region to correct, i.e.
  of \f$\frac{source - out.correction}{ccd\_ron}^2\f$

  out.red_chi2 contains the reduced chi2, i.e. the chi2 divided by the number 
  of contributing pixels.

  out.sigclip_reject_low and _high are 1D CPL images of type double.
  They are only returned for the sigma-clipping method. They
  indicate the final thresholds of the sigma-clipping rejection.

  \image html overscan_computation_algorithm.png

  @see hdrl_kappa_sigma_clip
  @see hdrl_overscan_correct
 */
/*----------------------------------------------------------------------------*/
hdrl_overscan_compute_result * hdrl_overscan_compute(
        const cpl_image         *   source,
        const hdrl_parameter    *   params)
{
    cpl_image   *   correction_img = NULL;
    cpl_image   *   correction_err_img = NULL;
    cpl_image   *   contribution_img = NULL;
    cpl_image   *   chi2_img = NULL;
    cpl_image   *   red_chi2_img = NULL;
    cpl_image   *   reject_low = NULL;
    cpl_image   *   reject_high = NULL;
    cpl_size        llx = 0;
    cpl_size        lly = 0;
    cpl_size        urx = 0;
    cpl_size        ury = 0;
    cpl_image   *   overscan_ima = NULL;

     /*TODO add minmax description to the doxygen */

    /* Check Entries */
    cpl_error_ensure(source != NULL, CPL_ERROR_NULL_INPUT, 
            return NULL, "NULL input image");
    cpl_error_ensure(params != NULL, CPL_ERROR_NULL_INPUT, 
            return NULL, "NULL input parameters");

	int d1 = sizeof(hdrl_sigclip_vector_output);
	int d2 = sizeof(hdrl_minmax_vector_output );
    cpl_error_ensure(d1 == d2, CPL_ERROR_INVALID_TYPE, return NULL,
		"Invalid check type between hdrl_sigclip_vector_output and hdrl_minmax_vector_output");

    if (hdrl_overscan_parameter_verify(
                params,
                cpl_image_get_size_x(source),
                cpl_image_get_size_y(source)) != CPL_ERROR_NONE)
        return NULL;

    /* Local Usage Parameters */
    const hdrl_overscan_parameter * p_loc = 
        (const hdrl_overscan_parameter *)params ;
    const hdrl_parameter * cpse = p_loc->collapse ;
    const hdrl_parameter * rr = p_loc->rect_region ;

    /* Extract Overscan region */
    overscan_ima = cpl_image_extract(source, 
            hdrl_rect_region_get_llx(rr),
            hdrl_rect_region_get_lly(rr),
            hdrl_rect_region_get_urx(rr),
            hdrl_rect_region_get_ury(rr)) ;

    /* Handle Orientation */
    if(p_loc->correction_direction == HDRL_Y_AXIS) {
        /* rotate the image 90 degree counter-clockwise */
        cpl_image_turn(overscan_ima, -1);
    }

    /* Redefine the boundaries to extracted image */
    llx = 1;
    lly = 1;
    urx = cpl_image_get_size_x(overscan_ima);
    ury = cpl_image_get_size_y(overscan_ima);

    /* Create output images  */
    correction_img = cpl_image_new(1, ury, HDRL_TYPE_DATA);
    correction_err_img = cpl_image_new(1, ury, HDRL_TYPE_ERROR);
    contribution_img = cpl_image_new(1, ury, CPL_TYPE_INT);
    chi2_img = cpl_image_new(1, ury, CPL_TYPE_DOUBLE);
    red_chi2_img = cpl_image_new(1, ury, CPL_TYPE_DOUBLE);

    if (hdrl_collapse_parameter_is_sigclip(cpse) ||
                    hdrl_collapse_parameter_is_minmax(cpse)) {
        reject_low = cpl_image_new(1, ury, CPL_TYPE_DOUBLE);
        reject_high = cpl_image_new(1, ury, CPL_TYPE_DOUBLE);
    }

    /* Loop along Y direction */
    /* only 1 iteration if hbox == HDRL_OVERSCAN_FULL_BOX */
HDRL_OMP(omp parallel for)
    for (long ipixel = 1;
         ipixel <= (p_loc->box_hsize == HDRL_OVERSCAN_FULL_BOX ? 1 : ury);
         ipixel++) {
        cpl_size contribution;
        double corr, error, chi2, red_chi2;
        const int box_hsize = p_loc->box_hsize;
        const double ccd_ron = p_loc->ccd_ron;
        cpl_size upperlimit;
        cpl_size lowerlimit;
        cpl_image * overscan_sub_ima, * ccd_ron_ima;
        /* Switch on the different methods */
        hdrl_collapse_imagelist_to_vector_t * reduce = NULL;
        void * collapse_eout = NULL;

        /* define proper extraction limits */
        if (box_hsize == HDRL_OVERSCAN_FULL_BOX) {
            /* take full region as box */
            lowerlimit = lly;
            upperlimit = ury;
        }
        else if (ipixel + box_hsize > ury) {
            /* Shrink the window if you are approaching the image boundaries */
            upperlimit = CX_MIN(ipixel + box_hsize, ury);
            lowerlimit = 2 * ipixel - upperlimit;
        }
        else {
            /* Shrink the window if you are approaching the image boundaries */
            lowerlimit = CX_MAX(ipixel - box_hsize, 1);
            upperlimit = 2 * ipixel - lowerlimit;
        }

        /* Extract the current running sub-window */
        overscan_sub_ima = cpl_image_extract(overscan_ima, llx, lowerlimit, 
                urx, upperlimit);

        /* Fill an image with the ccd_ron constant */
        ccd_ron_ima = cpl_image_duplicate(overscan_sub_ima);
        cpl_image_multiply_scalar(ccd_ron_ima, 0.0);
        cpl_image_add_scalar(ccd_ron_ima, ccd_ron);

        get_reduction(cpse, overscan_sub_ima, &reduce);

        /* Compute the over-scan correction, error, and contribution  */
        hdrl_overscan_reduce_image_to_scalar(reduce, overscan_sub_ima, 
                ccd_ron_ima, &corr, &error, &contribution, &collapse_eout);

        /* handle additional sigclip and minmax output */
        if (collapse_eout && (hdrl_collapse_parameter_is_sigclip(cpse) ||
            hdrl_collapse_parameter_is_minmax(cpse))) {

            hdrl_sigclip_vector_output * eout = collapse_eout;

            double low  = cpl_vector_get(eout->reject_low, 0);
            double high = cpl_vector_get(eout->reject_high, 0);

            cpl_image_set(reject_low, 1, ipixel, low);
            cpl_image_set(reject_high, 1, ipixel, high);

            cpl_vector_delete(eout->reject_low);
            cpl_vector_delete(eout->reject_high);
        }

        hdrl_collapse_imagelist_to_vector_unwrap_eout(reduce, collapse_eout);

        /* Compute the chi2 - Independent of the method */
        if (contribution == 0) {
            chi2 = NAN;
            red_chi2 = NAN;
        } else if (p_loc->box_hsize == HDRL_OVERSCAN_FULL_BOX) {
            hdrl_overscan_compute_chi_square(overscan_sub_ima,
                    ccd_ron_ima, corr, &chi2, &red_chi2);
        } else {
            /*Calculate the chi2 only in the central slice of the image
             * TODO very inefficient - restructure */
            cpl_size nx = cpl_image_get_size_x(overscan_sub_ima);
            cpl_size ny = cpl_image_get_size_y(overscan_sub_ima);

            cpl_image * overscan_sub_ima_slice =
                cpl_image_extract(overscan_sub_ima, 1, (cpl_size)((ny+1)/2), 
                        nx, (cpl_size)((ny+1)/2));
            cpl_image * ccd_ron_ima_slice = 
                cpl_image_extract(ccd_ron_ima, 1, (cpl_size)((ny+1)/2), nx,
                        (cpl_size)((ny+1)/2));

            hdrl_overscan_compute_chi_square(overscan_sub_ima_slice, 
                    ccd_ron_ima_slice, corr, &chi2, &red_chi2);

            cpl_image_delete(overscan_sub_ima_slice);
            cpl_image_delete(ccd_ron_ima_slice);
        }

        /* Fill the result images with the current result */
        cpl_image_set(correction_img,     1, ipixel, corr);
        cpl_image_set(correction_err_img, 1, ipixel, error);
        cpl_image_set(contribution_img, 1, ipixel, contribution);
        cpl_image_set(chi2_img, 1, ipixel, chi2);
        cpl_image_set(red_chi2_img, 1, ipixel, red_chi2);

        cpl_image_delete(overscan_sub_ima);
        cpl_image_delete(ccd_ron_ima);

        hdrl_collapse_imagelist_to_vector_delete(reduce);
    }

    /* broadcast the full box result to the full result row/col */
    if (p_loc->box_hsize == HDRL_OVERSCAN_FULL_BOX) {
        int rej;
        const double ccd_ron = p_loc->ccd_ron;
        const double correction_value =
                        cpl_image_get(correction_img, 1, 1, &rej);
        const double correction_err_value =
                        cpl_image_get(correction_err_img, 1, 1, &rej);
        const cpl_size contribution_value =
                        cpl_image_get(contribution_img, 1, 1, &rej);
        cpl_size loopmax = cpl_image_get_size_y(correction_img);
HDRL_OMP(omp parallel for private(rej))
        for (cpl_size i = 1; i <= loopmax; i++) {
            double chi2, red_chi2;
            cpl_image * overscan_sub_ima, * ccd_ron_ima;

            if (i < loopmax) {
                /*Here we broadcast*/
                cpl_image_set(correction_img, 1, i + 1, correction_value);
                cpl_image_set(correction_err_img, 1, i + 1, 
                        correction_err_value);
                cpl_image_set(contribution_img, 1, i + 1,contribution_value);
                if (hdrl_collapse_parameter_is_sigclip(cpse) ||
                                hdrl_collapse_parameter_is_minmax(cpse)) {
                    cpl_image_set(reject_low, 1, i + 1,
                            cpl_image_get(reject_low, 1, 1, &rej));
                    cpl_image_set(reject_high, 1, i + 1,
                            cpl_image_get(reject_high, 1, 1, &rej));
                }
            }
            /* Here we do additional chi2 calculation */

            /* Extract the current running sub-window */
            overscan_sub_ima = cpl_image_extract(overscan_ima, llx, i, urx, i);

            /* Fill an image with the ccd_ron constant */
            ccd_ron_ima = cpl_image_duplicate(overscan_sub_ima);
            cpl_image_multiply_scalar(ccd_ron_ima, 0.0);
            cpl_image_add_scalar(ccd_ron_ima, ccd_ron);

            hdrl_overscan_compute_chi_square(overscan_sub_ima,
                                             ccd_ron_ima, correction_value,
                                             &chi2, &red_chi2);

            cpl_image_set(chi2_img, 1, i, chi2);
            cpl_image_set(red_chi2_img, 1, i, red_chi2);
            cpl_image_delete(overscan_sub_ima);
            cpl_image_delete(ccd_ron_ima);
        }
    }

    cpl_image_delete(overscan_ima);
    /* flag bad pixels */
    cpl_image_reject_value(correction_img, CPL_VALUE_NAN);
    cpl_image_reject_value(correction_err_img, CPL_VALUE_NAN);
    cpl_image_reject_value(chi2_img, CPL_VALUE_NAN);
    cpl_image_reject_value(red_chi2_img, CPL_VALUE_NAN);
    if (hdrl_collapse_parameter_is_sigclip(cpse) ||
                    hdrl_collapse_parameter_is_minmax(cpse)) {
        cpl_image_reject_value(reject_low, CPL_VALUE_NAN);
        cpl_image_reject_value(reject_high, CPL_VALUE_NAN);
    }

    /* Handle Orientation */
    if(p_loc->correction_direction == HDRL_Y_AXIS) {
        cpl_image_turn(correction_img, +1);
        cpl_image_turn(correction_err_img, +1);
        cpl_image_turn(contribution_img, +1);
        cpl_image_turn(chi2_img, +1);
        cpl_image_turn(red_chi2_img, +1);
        if (hdrl_collapse_parameter_is_sigclip(cpse) ||
                        hdrl_collapse_parameter_is_minmax(cpse)) {
            cpl_image_turn(reject_low, +1);
            cpl_image_turn(reject_high, +1);
        }
    }

    /* Create the Overscan resulting structure */
    {
        hdrl_overscan_compute_result * result =
            hdrl_overscan_compute_result_create();
        hdrl_image * res = hdrl_image_create(correction_img,
                                             correction_err_img);
        cpl_image_delete(correction_img);
        cpl_image_delete(correction_err_img);
        result->correction_direction = p_loc->correction_direction;
        result->correction = res;
        result->contribution = contribution_img;
        result->chi2 = chi2_img;
        result->red_chi2 = red_chi2_img;
        result->sigclip_reject_low = reject_low;
        result->sigclip_reject_high = reject_high;
        return result;
    }
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the correction in the Overscan Computation result object
  @param    res The Overscan Computation result object
  @return   The correction HDRL image
 */
/*----------------------------------------------------------------------------*/
hdrl_image * hdrl_overscan_compute_result_get_correction(
        const hdrl_overscan_compute_result * res)
{
    cpl_ensure(res, CPL_ERROR_NULL_INPUT, NULL);
    return res->correction;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Unset the correction in the Overscan Computation result object
  @param    res The Overscan Computation result object
  @return   The correction HDRL image that has been unset
 */
/*----------------------------------------------------------------------------*/
hdrl_image * hdrl_overscan_compute_result_unset_correction(
        hdrl_overscan_compute_result * res)
{
    cpl_ensure(res, CPL_ERROR_NULL_INPUT, NULL);
    hdrl_image * r = res->correction;
    res->correction = NULL;
    return r;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the contribution in the Overscan Computation result object
  @param    res The Overscan Computation result object
  @return   The contribution CPL image
 */
/*----------------------------------------------------------------------------*/
cpl_image * hdrl_overscan_compute_result_get_contribution(
        const hdrl_overscan_compute_result * res)
{
    cpl_ensure(res, CPL_ERROR_NULL_INPUT, NULL);
    return res->contribution;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Unset the contribution in the Overscan Computation result object
  @param    res The Overscan Computation result object
  @return   The contribution CPL image that has been unset
 */
/*----------------------------------------------------------------------------*/
cpl_image * hdrl_overscan_compute_result_unset_contribution(
        hdrl_overscan_compute_result * res)
{
    cpl_ensure(res, CPL_ERROR_NULL_INPUT, NULL);
    cpl_image * r = res->contribution;
    res->contribution = NULL;
    return r;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the CHI2 in the Overscan Computation result object
  @param    res The Overscan Computation result object
  @return   The CHI2 CPL image
 */
/*----------------------------------------------------------------------------*/
cpl_image * hdrl_overscan_compute_result_get_chi2(
        const hdrl_overscan_compute_result * res)
{
    cpl_ensure(res, CPL_ERROR_NULL_INPUT, NULL);
    return res->chi2;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Unset the CHI2 in the Overscan Computation result object
  @param    res The Overscan Computation result object
  @return   The CHI2 CPL image that has been unset
 */
/*----------------------------------------------------------------------------*/
cpl_image * hdrl_overscan_compute_result_unset_chi2(
        hdrl_overscan_compute_result * res)
{
    cpl_ensure(res, CPL_ERROR_NULL_INPUT, NULL);
    cpl_image * r = res->chi2;
    res->chi2 = NULL;
    return r;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the reduced CHI2 in the Overscan Computation result object
  @param    res The Overscan Computation result object
  @return   The reduced CHI2 CPL image
 */
/*----------------------------------------------------------------------------*/
cpl_image * hdrl_overscan_compute_result_get_red_chi2(
        const hdrl_overscan_compute_result * res)
{
    cpl_ensure(res, CPL_ERROR_NULL_INPUT, NULL);
    return res->red_chi2;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Unset the reduced CHI2 in the Overscan Computation result object
  @param    res The Overscan Computation result object
  @return   The reduced CHI2 CPL image that has been unset
 */
/*----------------------------------------------------------------------------*/
cpl_image * hdrl_overscan_compute_result_unset_red_chi2(
        hdrl_overscan_compute_result * res)
{
    cpl_ensure(res, CPL_ERROR_NULL_INPUT, NULL);
    cpl_image * r = res->red_chi2;
    res->red_chi2 = NULL;
    return r;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the low threshold in the Overscan Computation result object
  @param    res The Overscan Computation result object
  @return   The low threshold CPL image
 */
/*----------------------------------------------------------------------------*/
cpl_image * hdrl_overscan_compute_result_get_sigclip_reject_low(
        const hdrl_overscan_compute_result * res)
{
    cpl_ensure(res, CPL_ERROR_NULL_INPUT, NULL);
    if (!res->sigclip_reject_low) {
        cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT,
                              "rejection parameters are only "
                              "available if collapse mode of overscan is set "
                              "to sigclip or minmax");
    }
    return res->sigclip_reject_low;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Unset the low threshold in the Overscan Computation result object
  @param    res The Overscan Computation result object
  @return   The low threshold CPL image that has been unset
 */
/*----------------------------------------------------------------------------*/
cpl_image * hdrl_overscan_compute_result_unset_sigclip_reject_low(
        hdrl_overscan_compute_result * res)
{
    cpl_ensure(res, CPL_ERROR_NULL_INPUT, NULL);
    cpl_image * r = res->sigclip_reject_low;
    if (!res->sigclip_reject_low) {
        cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT,
                              "rejection parameters are only "
                              "available if collapse mode of overscan is set "
                              "to sigclip or minmax");
    }
    res->sigclip_reject_low = NULL;
    return r;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the high threshold in the Overscan Computation result object
  @param    res The Overscan Computation result object
  @return   The high threshold CPL image
 */
/*----------------------------------------------------------------------------*/
cpl_image * hdrl_overscan_compute_result_get_sigclip_reject_high(
        const hdrl_overscan_compute_result * res)
{
    cpl_ensure(res, CPL_ERROR_NULL_INPUT, NULL);
    if (!res->sigclip_reject_high) {
        cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT,
                              "rejection parameters are only "
                              "available if collapse mode of overscan is set "
                              "to sigclip or minmax");
    }
    return res->sigclip_reject_high;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Unset the high threshold in the Overscan Computation result object
  @param    res The Overscan Computation result object
  @return   The high threshold CPL image that has been unset
 */
/*----------------------------------------------------------------------------*/
cpl_image * hdrl_overscan_compute_result_unset_sigclip_reject_high(
        hdrl_overscan_compute_result * res)
{
    cpl_ensure(res, CPL_ERROR_NULL_INPUT, NULL);
    cpl_image * r = res->sigclip_reject_high;
    res->sigclip_reject_high = NULL;
    return r;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Access the low threshold in the Overscan Computation result object
  @param    res The Overscan Computation result object
  @return   The low threshold CPL image
 */
/*----------------------------------------------------------------------------*/
cpl_image * hdrl_overscan_compute_result_get_minmax_reject_low(
        const hdrl_overscan_compute_result * res)
{
   return hdrl_overscan_compute_result_get_sigclip_reject_low(res);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Unset the low threshold in the Overscan Computation result object
  @param    res The Overscan Computation result object
  @return   The low threshold CPL image that has been unset
 */
/*----------------------------------------------------------------------------*/
cpl_image * hdrl_overscan_compute_result_unset_minmax_reject_low(
        hdrl_overscan_compute_result * res)
{
    return hdrl_overscan_compute_result_unset_sigclip_reject_low(res);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the high threshold in the Overscan Computation result object
  @param    res The Overscan Computation result object
  @return   The high threshold CPL image
 */
/*----------------------------------------------------------------------------*/
cpl_image * hdrl_overscan_compute_result_get_minmax_reject_high(
        const hdrl_overscan_compute_result * res)
{
    return hdrl_overscan_compute_result_get_sigclip_reject_high(res);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Unset the high threshold in the Overscan Computation result object
  @param    res The Overscan Computation result object
  @return   The high threshold CPL image that has been unset
 */
/*----------------------------------------------------------------------------*/
cpl_image * hdrl_overscan_compute_result_unset_minmax_reject_high(
        hdrl_overscan_compute_result * res)
{
    return hdrl_overscan_compute_result_unset_sigclip_reject_high(res);
}



/*----------------------------------------------------------------------------*/
/**
  @brief    Deletes the Overscan Computation Result Structure
  @param    result  The computation result structure to delete
 */
/*----------------------------------------------------------------------------*/
void hdrl_overscan_compute_result_delete(
        hdrl_overscan_compute_result   *   result)
{
    if (result == NULL) return;
    hdrl_image_delete(result->correction);
    cpl_image_delete(result->contribution);
    cpl_image_delete(result->chi2);
    cpl_image_delete(result->red_chi2);
    cpl_image_delete(result->sigclip_reject_low);
    cpl_image_delete(result->sigclip_reject_high);
    cpl_free(result);
    return;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Overscan correction 
  @param    source          Input HDRL image that needs to be corrected
  @param    region          Region from source that needs correction
  @param    os_computation  Results of the Overscan computation
  @return  hdrl_overscan_correct_result structure containing the
  results of the correction. It must be freed with 
  hdrl_overscan_correct_result_delete()
  On error, NULL is returned and an error code is set.

  source is the input CPL image that needs to be corrected. It is a required
  input.
  Usually the same image is passed to hdrl_overscan_compute() to compute 
  the overscan correction parameters. 

  source_error is the error used for error propagation. It is a required input.

  region specifies which region of the source image must be corrected.
  If NULL or invalid, the whole image is corrected.
  The region size must fit the sixe of the os_computation.

  os_computation contains all the parameters for the overscan
  correction. It has been produced by hdrl_overscan_compute().

  Following results are available from the return value:

  hdrl_overscan_correct_result_get_corrected(result):
  returns the HDRL image of type double of the same size as source.
  
  Its image part contains the corrected values where all the pixels
  within the specified region were subtracted using the proper correction 
  obtained in os_computation.correction.
  The pixels outside the specified region remain unchanged.
  Pixels for which a overscan value could not be determined (e.g. because all
  pixels in the overscan region are bad) are set to zero.

  Its error part contains the error where all the pixels within the specified 
  region are set to: \f$\sqrt{(os\_computation.error^2 + source\_error^2)}\f$, which is
  the standard Gaussian error propagation.
  The pixels outside the specified region remain unchanged.
  Pixels for which a overscan value could not be determined (e.g. because all
  pixels in the overscan region are bad) are set to zero.

  hdrl_overscan_correct_result_get_badmask(result):
  returns an integer cpl_image of the same size as the input that contains a
  value of 1 for pixels that have not been corrected due to the overscan data
  being bad in that row/column.
 */
/*----------------------------------------------------------------------------*/
hdrl_overscan_correct_result * hdrl_overscan_correct(
        const hdrl_image                    *   source,
        const hdrl_parameter                *   region,
        const hdrl_overscan_compute_result  *   os_computation)
{
    cpl_size            llx = 0, lly = 0, urx = 0, ury = 0;
    /* local pointers to source image and its error */
    cpl_image       *   source_loc = NULL;
    cpl_image       *   source_error_loc = NULL;
    long                xsize_overscan = 0;
    long                ysize_overscan = 0;
    size_t              nx;
    cpl_mask        *   orig_mask = NULL;
    hdrl_image      *   hoverscan;
    /* value to use in mask for pixels rejected by algorithm */
    hdrl_bitmask_t      reject_code = 1;

    /* Check Entries */
    cpl_error_ensure(source != NULL, CPL_ERROR_NULL_INPUT,
            return NULL, "NULL input source image");
    cpl_error_ensure(os_computation != NULL, CPL_ERROR_NULL_INPUT,
            return NULL, "NULL overscan computation result");

    cpl_error_ensure(hdrl_int_is_power_of_two(reject_code),
                     CPL_ERROR_ILLEGAL_INPUT, return NULL,
                     "reject_code must be a power of two");

    if (hdrl_overscan_compute_result_verify(os_computation) != CPL_ERROR_NONE)
        return NULL;

    hoverscan = os_computation->correction;

    /* Initialise Region parameters */
    if (region != NULL) {
        if (hdrl_rect_region_parameter_verify(region, 
                    hdrl_image_get_size_x(source),
                    hdrl_image_get_size_y(source)) != CPL_ERROR_NONE)
            return NULL;
        llx = hdrl_rect_region_get_llx(region);
        lly = hdrl_rect_region_get_lly(region);
        urx = hdrl_rect_region_get_urx(region);
        ury = hdrl_rect_region_get_ury(region);
    } else {
        llx = lly = 1;
        urx = hdrl_image_get_size_x(source);
        ury = hdrl_image_get_size_y(source);
    }

    source_loc =
        cpl_image_cast(hdrl_image_get_image_const(source), HDRL_TYPE_DATA);
    source_error_loc =
        cpl_image_cast(hdrl_image_get_error_const(source), HDRL_TYPE_ERROR);
    nx = cpl_image_get_size_x(source_loc);

    xsize_overscan = hdrl_image_get_size_x(hoverscan);
    ysize_overscan = hdrl_image_get_size_y(hoverscan);
    
    /* Check if the overscan image has an appropriate size */
    if (os_computation->correction_direction == HDRL_X_AXIS &&
            ury-lly+1 != ysize_overscan) {
        cpl_image_delete(source_loc);
        cpl_image_delete(source_error_loc);
        cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT,
                "Correction region Y size does not match overscan Y size");
        return NULL;
    }
    if (os_computation->correction_direction == HDRL_Y_AXIS &&
            urx-llx+1 != xsize_overscan) {
        cpl_image_delete(source_loc);
        cpl_image_delete(source_error_loc);
        cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT,
                "Correction region X size does not match overscan X size");
        return NULL;
    }

    if (xsize_overscan != 1 && ysize_overscan != 1) {
        cpl_image_delete(source_loc);
        cpl_image_delete(source_error_loc);
        cpl_error_set(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT);
        return NULL;
    }

    /* store original mask to later separate it from newly created bpms */
    orig_mask = hdrl_copy_image_mask(source_loc);

    {
        hdrl_data_t * psource_loc = cpl_image_get_data(source_loc);
        hdrl_error_t * psource_err_loc = cpl_image_get_data(source_error_loc);
        const cpl_mask * bpm = hdrl_image_get_mask_const(hoverscan);
        /* os image is one dimensional, so just get the data */
        const cpl_binary * rej = bpm ?  cpl_mask_get_data_const(bpm) : NULL;
        const hdrl_data_t * pos_val = hdrl_get_image_data_const(
                                     hdrl_image_get_image_const(hoverscan));
        const hdrl_error_t * pos_e = hdrl_get_image_error_const(
                                     hdrl_image_get_error_const(hoverscan));
        /* make sure we have a bpm before the parallel loop */
        cpl_image_get_bpm(source_loc);

        /* corrects extracted image for over-scan value, compute associated
         * error, flag bad pixels */
HDRL_OMP(omp parallel for)
        for (long j = lly - 1; j < ury; j++) {
            for (long i = llx - 1; i < urx; i++) {
                const size_t idx = 
                    os_computation->correction_direction == HDRL_X_AXIS ? 
                    j-lly+1 : i-llx+1;
                double ima_e = psource_err_loc[j * nx + i];

                if (rej && rej[idx]) {
                    /* set to zero as per requirements */
                    cpl_image_reject(source_loc, i + 1, j + 1);
                    psource_loc[j * nx + i] = 0;
                    psource_err_loc[j * nx + i] = 0;
                } else {
                    psource_loc[j * nx + i] -= pos_val[idx];
                    psource_err_loc[j * nx + i] =
                        sqrt(pos_e[idx] * pos_e[idx] + ima_e * ima_e);
                }
            }
        }
    }

    {
        hdrl_overscan_correct_result * res;
        cpl_mask * new_mask = hdrl_copy_image_mask(source_loc);
        cpl_image * badmask = cpl_image_new(cpl_image_get_size_x(source_loc),
                                            cpl_image_get_size_y(source_loc),
                                            CPL_TYPE_INT);
        /* get the new bad pixels */
        cpl_mask_xor(new_mask, orig_mask);
        /* fill them with the code */
        cpl_image_reject_from_mask(badmask, new_mask);
        cpl_image_fill_rejected(badmask, reject_code);

        cpl_mask_delete(new_mask);
        cpl_mask_delete(orig_mask);

        /* Create, fill and return the returned structure */
        res = hdrl_overscan_correct_result_create();
        res->corrected = hdrl_image_wrap(source_loc, source_error_loc, NULL,
                                         CPL_TRUE);
        res->badmask = badmask;
        return res;
    }
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Delete the Overscan Correction Result Structure
  @param    result  The correction result structure to delete
 */
/*----------------------------------------------------------------------------*/
void hdrl_overscan_correct_result_delete(
        hdrl_overscan_correct_result   *   result)
{
    if (result == NULL) return; 
    hdrl_image_delete(result->corrected);
    cpl_image_delete(result->badmask);
    cpl_free(result);
    return;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the corrected image in the Overscan Correction result object
  @param    res The Overscan Correction result object
  @return   The HDLR corrected image
 */
/*----------------------------------------------------------------------------*/
hdrl_image * hdrl_overscan_correct_result_get_corrected(
        const hdrl_overscan_correct_result * res)
{
    cpl_ensure(res, CPL_ERROR_NULL_INPUT, NULL);
    return res->corrected;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Unset the corrected image in the Overscan Correction result object
  @param    res The Overscan Correction result object
  @return   The HDLR corrected image that was unset
 */
/*----------------------------------------------------------------------------*/
hdrl_image * hdrl_overscan_correct_result_unset_corrected(
        hdrl_overscan_correct_result * res)
{
    cpl_ensure(res, CPL_ERROR_NULL_INPUT, NULL);
    hdrl_image * r = res->corrected;
    res->corrected = NULL;
    return r;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the bad pixels mask in the Overscan Correction result object
  @param    res The Overscan Correction result object
  @return   The bad pixels mask as a CPL image
 */
/*----------------------------------------------------------------------------*/
cpl_image * hdrl_overscan_correct_result_get_badmask(
        const hdrl_overscan_correct_result * res)
{
    cpl_ensure(res, CPL_ERROR_NULL_INPUT, NULL);
    return res->badmask;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Unset the bad pixels mask in the Overscan Correction result object
  @param    res The Overscan Correction result object
  @return   The bad pixels mask as a CPL image that was unset
 */
/*----------------------------------------------------------------------------*/
cpl_image * hdrl_overscan_correct_result_unset_badmask(
        hdrl_overscan_correct_result * res)
{
    cpl_ensure(res, CPL_ERROR_NULL_INPUT, NULL);
    cpl_image * r = res->badmask;
    res->badmask = NULL;
    return r;
}

/**@}*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Destroys Overscan Parameters objects
  @param    param   The parameter to destroy
 */
/*----------------------------------------------------------------------------*/
static void hdrl_overscan_parameter_destroy(void * param)
{
    hdrl_overscan_parameter * p = (hdrl_overscan_parameter *)param ;
    hdrl_parameter_destroy(p->collapse) ;
    hdrl_parameter_destroy(p->rect_region) ;
    hdrl_parameter_delete((hdrl_parameter*)param) ;
    return ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Creates Overscan Computation Result Structure 
  @return   A structure that needs to be de-allocated with
  hdrl_overscan_compute_result_delete()
  @see hdrl_overscan_compute_result_delete()
 */
/*----------------------------------------------------------------------------*/
static hdrl_overscan_compute_result * hdrl_overscan_compute_result_create(
        void)
{
    hdrl_overscan_compute_result * self = 
        cpl_malloc(sizeof(hdrl_overscan_compute_result));

    self->correction_direction = HDRL_UNDEFINED_AXIS;
    self->correction = NULL;
    self->contribution = NULL;
    self->chi2 = NULL;
    self->red_chi2 = NULL;
    self->sigclip_reject_low = NULL;
    self->sigclip_reject_high = NULL;
    return self;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Verify basic correctness of the result structure
  @param    result      Overscan result structure
  @return   CPL_ERROR_NONE if eveything is ok, an error code otherwise
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_overscan_compute_result_verify(
        const hdrl_overscan_compute_result * result)
{
    cpl_error_ensure(result != NULL, CPL_ERROR_NULL_INPUT,
            return CPL_ERROR_NULL_INPUT,
            "NULL input overscan result structure");
    cpl_error_ensure(result->correction_direction == HDRL_X_AXIS ||
            result->correction_direction == HDRL_Y_AXIS, 
            CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
            "The specified collapse direction is unknown");

    if (result->correction_direction == HDRL_X_AXIS) {
        cpl_error_ensure(hdrl_image_get_size_x(result->correction) == 1, 
                CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                "The Correction image X size should be 1");
        cpl_error_ensure(cpl_image_get_size_x(result->contribution) == 1, 
                CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                "The Contribution image X size should be 1");
        cpl_error_ensure(cpl_image_get_size_x(result->chi2) == 1,
                CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                "The Chi Square image X size should be 1");
        cpl_error_ensure(cpl_image_get_size_x(result->red_chi2) == 1, 
                CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                "The reduced Chi Square image X size should be 1");
        if (result->sigclip_reject_low != NULL) {
            cpl_error_ensure(cpl_image_get_size_x(result->sigclip_reject_low) 
                    == 1, 
                    CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                    "The SIGCLIP low rejection image X size should be 1");

        }
        if (result->sigclip_reject_high != NULL) {
            cpl_error_ensure(cpl_image_get_size_x(result->sigclip_reject_high) 
                    == 1, 
                    CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                    "The SIGCLIP high rejection image X size should be 1");
        }
    }
    else if (result->correction_direction == HDRL_Y_AXIS) {
        cpl_error_ensure(hdrl_image_get_size_y(result->correction) == 1, 
                CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                "The Correction image Y size should be 1");
        cpl_error_ensure(cpl_image_get_size_y(result->contribution) == 1, 
                CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                "The Contribution image Y size should be 1");
        cpl_error_ensure(cpl_image_get_size_y(result->chi2) == 1,
                CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                "The Chi Square image Y size should be 1");
        cpl_error_ensure(cpl_image_get_size_y(result->red_chi2) == 1, 
                CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                "The reduced Chi Square image Y size should be 1");
        if (result->sigclip_reject_low != NULL) {
            cpl_error_ensure(cpl_image_get_size_y(result->sigclip_reject_low) 
                    == 1, 
                    CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                    "The SIGCLIP low rejection image Y size should be 1");

        }
        if (result->sigclip_reject_high != NULL) {
            cpl_error_ensure(cpl_image_get_size_y(result->sigclip_reject_high) 
                    == 1, 
                    CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                    "The SIGCLIP high rejection image Y size should be 1");
        }
    }
    else return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
            "correction_direction must be HDRL_X_AXIS or HDRL_Y_AXIS");
    return CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Creates Overscan Correction result structure
  @return   A structure holding the Overscan Correction Results, that
  needs to be de-allocated with hdrl_overscan_correct_result_delete()
  @see hdrl_overscan_correct_result_delete() 
 */
/*----------------------------------------------------------------------------*/
static hdrl_overscan_correct_result * hdrl_overscan_correct_result_create(
        void)
{
    hdrl_overscan_correct_result * self = cpl_malloc(sizeof(*self));
    self->corrected = NULL;
    self->badmask = NULL;
    return self;
}

/*----------------------------------------------------------------------------*/
/**
  @brief Calculate chi square as well as the reduced chi square
  @param data          Measured data image
  @param error         Error of measured data image
  @param expect        Expected value of data
  @param[out] chi2     Weighted sum of squared deviations
  @param[out] red_chi2 Reduced weighted sum of squared deviations
  @return CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code  hdrl_overscan_compute_chi_square(
        const cpl_image *   data,
        const cpl_image *   error,
        const double        expect,
        double          *   chi2,
        double          *   red_chi2)
{
    cpl_image * s;
    cpl_size nrej, nerej;
    cpl_image * e = NULL;
    cpl_size nepix = 0;
    cpl_size npix = 0;

    nrej = cpl_image_count_rejected(data);
    npix = cpl_image_get_size_x(data) * cpl_image_get_size_y(data);

    if (nrej == npix) {
        *chi2 = NAN;
        *red_chi2 = NAN;
        return CPL_ERROR_NONE;
    }

    e = cpl_image_duplicate(error);
    nepix = cpl_image_get_size_x(e) * cpl_image_get_size_y(e);

    /*TODO handle pre-existing Zero's marked as bad */
    /* check if error image contains zeros */
    cpl_image_accept_all(e);
    cpl_image_reject_value(e, CPL_VALUE_ZERO);
    nerej = cpl_image_count_rejected(e);
    /* all errors zero allowed */
    if (nerej == nepix) {
        cpl_image_delete(e);
        *chi2 = NAN;
        *red_chi2 = NAN;
        return CPL_ERROR_NONE;
    }
    /* partial zero errors make no sense */
    else if (nerej != 0) {
        cpl_image_delete(e);
        cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                              "Error image can't contain zeros");
        *chi2 = NAN;
        *red_chi2 = NAN;
        return CPL_ERROR_ILLEGAL_INPUT;
    }

    s = cpl_image_duplicate(data);
    /* computes chi squared defined as:
     * \Sum_i[ (x_i - expect)^2 / sigma_i^2 ] */
    cpl_image_subtract_scalar(s, expect);
    cpl_image_divide(s, e);
    *chi2 = cpl_image_get_sqflux(s); /* = squared sum */
    *red_chi2 = *chi2 / npix; /* reduced chi2 */
    cpl_image_delete(s);
    cpl_image_delete(e);
    return CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief reduce image to scalar
  @param red          imagelist to vector reduction method
  @param data         image to reduce
  @param data_error   error of image
  @param result       data reduction result
  @param error        propagated error
  @param contribution number of pixels that contributed
  @param eout         extra output for sigclip
  @return A CPL error code

  this is just a single image wrapper over hdrl_collapse_imagelist_to_vector
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code hdrl_overscan_reduce_image_to_scalar(
        hdrl_collapse_imagelist_to_vector_t *   red,
        cpl_image                           *   data,
        cpl_image                           *   data_error,
        double                              *   result, 
        double                              *   error,
        cpl_size                            *   contribution,
        void                                **  eout)
{
    cpl_imagelist * ld = cpl_imagelist_new();
    cpl_imagelist * le = cpl_imagelist_new();
    cpl_vector * od = NULL, * oe = NULL;
    cpl_array * oc = NULL;
    cpl_error_code fail;
    cpl_imagelist_set(ld, data, 0);
    cpl_imagelist_set(le, data_error, 0);


    fail = hdrl_collapse_imagelist_to_vector_call(red, ld, le, &od, &oe, &oc,
                                                  eout);
    cpl_imagelist_unwrap(ld);
    cpl_imagelist_unwrap(le);

    if (fail == CPL_ERROR_NONE) {
        *result = cpl_vector_get(od, 0);
        *error = cpl_vector_get(oe, 0);
        *contribution = cpl_array_get_int(oc, 0, NULL);
    }
    else {
        *result = NAN;
        *error = NAN;
        *contribution = 0;
    }

    cpl_vector_delete(od);
    cpl_vector_delete(oe);
    cpl_array_delete(oc);

    return fail;
}
