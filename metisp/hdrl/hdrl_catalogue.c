/*
 * This file is part of the HDRL
 * Copyright (C) 2016 European Southern Observatory
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

#include <assert.h>
#include <math.h>
#include <stdint.h>

#include "hdrl_catalogue.h"
#include "hdrl_image.h"
#include "hdrl_types.h"
#include "hdrl_utils.h"

/*-----------------------------------------------------------------------------
                                   Static
 -----------------------------------------------------------------------------*/




/**
 *
 * @defgroup hdrl_catalogue     Object catalogue
 *
 * @brief
 *   This module provides algorithms to build an objects catalogue.
 *   Depending on the value of the control parameter "resulttype", additional
 *   products may be generated: a full catalogue, the background map, the
 *   segmentation map.
 *
 * \par Brief algorithm description:
 *
 * A local sky background is estimated and removed
 *
 * Objects and blends are detected and the image pixels are assigned to each
 * object (or blend) they belong to.
 *
 * On the objects it is performed astrometry, photometry and shape analysis.
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*-----------------------------------------------------------------------------
                        Catalogue parameters Definition
 -----------------------------------------------------------------------------*/

/** @cond PRIVATE */

typedef struct {
    HDRL_PARAMETER_HEAD;
    int obj_min_pixels;
    double obj_threshold;
    cpl_boolean obj_deblending;
    double obj_core_radius;
    cpl_boolean bkg_estimate;
    int bkg_mesh_size;
    hdrl_catalogue_options resulttype;
    double bkg_smooth_fwhm;
    double det_eff_gain;
    double det_saturation;
} hdrl_catalogue_parameter;

/* parameter type */
static hdrl_parameter_typeobj hdrl_catalogue_parameter_type = {
    HDRL_PARAMETER_CATALOGUE,             /* type */
    (hdrl_alloc *)&cpl_malloc,            /* fp_alloc */
    (hdrl_free *)&cpl_free,               /* fp_free */
    NULL,                                 /* fp_destroy */
    sizeof(hdrl_catalogue_parameter),     /* obj_size */
};

static cpl_error_code hdrl_cleanup_qclist(cpl_propertylist * qclist);

/*----------------------------------------------------------------------------*/
/**
 * @internal
 * @brief    Verify basic correctness of the catalogue parameters
 * @param    param      catalogue parameters
 * @return   CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
hdrl_catalogue_parameter_verify(const hdrl_parameter    *   param)
{
    cpl_error_ensure(param != NULL, CPL_ERROR_NULL_INPUT,
                    return CPL_ERROR_NULL_INPUT, "NULL Input Parameters");
    cpl_error_ensure(hdrl_catalogue_parameter_check(param),
                     CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                                     "Expected catalogue parameter") ;

    const hdrl_catalogue_parameter * param_loc = (const hdrl_catalogue_parameter *)param ;

    cpl_error_ensure(param_loc->obj_min_pixels > 0, CPL_ERROR_ILLEGAL_INPUT,
                     return CPL_ERROR_ILLEGAL_INPUT, "obj.min-pixels > 0");

    cpl_error_ensure(param_loc->obj_threshold > 0., CPL_ERROR_ILLEGAL_INPUT,
                     return CPL_ERROR_ILLEGAL_INPUT, "obj_threshold > 0.");

    cpl_error_ensure(param_loc->obj_core_radius > 0., CPL_ERROR_ILLEGAL_INPUT,
                     return CPL_ERROR_ILLEGAL_INPUT, "obj_core_radius > 0.");

    if (param_loc->bkg_estimate) {
        cpl_error_ensure(param_loc->bkg_mesh_size > 2, CPL_ERROR_ILLEGAL_INPUT,
                         return CPL_ERROR_ILLEGAL_INPUT, "bkg_mesh_size > 2");

        cpl_error_ensure(param_loc->bkg_smooth_fwhm >= 0., CPL_ERROR_ILLEGAL_INPUT,
                         return CPL_ERROR_ILLEGAL_INPUT, "bkg_mesh_size >= 0.");
    }

    cpl_error_ensure(param_loc->det_eff_gain > 0., CPL_ERROR_ILLEGAL_INPUT,
                     return CPL_ERROR_ILLEGAL_INPUT, "det_eff_gain > 0.");

    cpl_error_ensure(param_loc->det_saturation > 0. ||
                     param_loc->det_saturation == HDRL_SATURATION_INIT,
                     CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                     "det_saturation > 0");

    return CPL_ERROR_NONE ;

}

/** @endcond */

/*----------------------------------------------------------------------------*/
/**
 * @brief    Creates catalogue Parameters object
 *
 * @param obj_min_pixels  Minimum pixel area for each detected object.
 * @param obj_threshold   Detection threshold in sigma above sky.
 * @param obj_deblending  Use deblending?
 * @param obj_core_radius Value of Rcore in pixels.
 * @param bkg_estimate    Estimate background from input, if false it is assumed
 *                        input is already background corrected with median 0.
 * @param bkg_mesh_size   Background smoothing box size.
 * @param bkg_smooth_fwhm The FWHM of the Gaussian kernel used in convolution
 *                        for object detection.
 * @param det_eff_gain    Detector gain value to rescale convert intensity to
 *                        electrons.
 * @param det_saturation  Detector saturation value.
 * @param resulttype      Requested output: CAT  (table), BKG(image),
 *                        SEGMAP(image), QCLIST(propertylist).
 * @return   The catalogue parameters object.
 *           It needs to be deallocated with hdrl_parameter_delete()
 * @see      hdrl_parameter_delete()
 * @see      hdrl_catalogue_compute()
 * The method creates a parameter to compute the catalogue
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_catalogue_parameter_create(int obj_min_pixels,
                       double obj_threshold, cpl_boolean obj_deblending,
                       double obj_core_radius,
                       cpl_boolean bkg_estimate, int bkg_mesh_size,
                       double bkg_smooth_fwhm, double det_eff_gain,
                       double det_saturation,
                       hdrl_catalogue_options resulttype)
{
    hdrl_catalogue_parameter * p = (hdrl_catalogue_parameter *)
    hdrl_parameter_new(&hdrl_catalogue_parameter_type);

    p->obj_min_pixels  = obj_min_pixels;
    p->obj_threshold   = obj_threshold;
    p->obj_deblending  = obj_deblending;
    p->obj_core_radius = obj_core_radius;
    p->bkg_estimate    = bkg_estimate;
    p->bkg_mesh_size   = bkg_mesh_size;
    p->resulttype      = resulttype;
    p->bkg_smooth_fwhm = bkg_smooth_fwhm;
    p->det_eff_gain    = det_eff_gain;
    p->det_saturation  = det_saturation;

    if (!bkg_estimate) {
        p->resulttype &= ~(HDRL_CATALOGUE_BKG);
    }

    if (hdrl_catalogue_parameter_verify((hdrl_parameter *)p)) {
        cpl_free(p);
        return NULL;
    }
    return (hdrl_parameter *)p;

}

/*----------------------------------------------------------------------------*/
/**
 * @brief    Check that the parameter is a catalogue parameter
 * @param    self The parameter to check
 * @return   True or False
 */
/*----------------------------------------------------------------------------*/
cpl_boolean hdrl_catalogue_parameter_check(const hdrl_parameter * self)
{
    return hdrl_parameter_check_type(self, &hdrl_catalogue_parameter_type);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief set result option of catalogue parameter
 *
 * @param par  hdrl catalogue parameter
 * @param opt  the options to set
 *
 * @return cpl_error_code
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_catalogue_parameter_set_option(hdrl_parameter * par,
                                                   hdrl_catalogue_options opt)
{
    cpl_ensure_code(par, CPL_ERROR_NULL_INPUT);
    cpl_error_code err = hdrl_catalogue_parameter_verify(par);
    if (err != CPL_ERROR_NONE) {
        return err;
    }
    hdrl_catalogue_parameter * par_ = (hdrl_catalogue_parameter*)par;
    par_->resulttype = opt;
    if (!par_->bkg_estimate) {
        par_->resulttype &= ~(HDRL_CATALOGUE_BKG);
    }
    return hdrl_catalogue_parameter_verify(par);
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Create parameter list for the catalogue computation
 * @param base_context        base context of parameter (e.g. recipe name)
 * @param prefix              prefix of parameter, may be an empty string
 * @param defaults            hdrl_parameter defining the defaults
 * @see hdrl_catalogue_parameter_create()
 *
 *
 * Creates a parameter list with the catalogue parameters:
 *   - base_context.prefix.obj.min-pixels
 *   - base_context.prefix.obj.threshold
 *   - base_context.prefix.obj.deblending
 *   - base_context.prefix.obj.core-radius
 *   - base_context.prefix.bkg.estimate
 *   - base_context.prefix.bkg.mesh-size
 *   - base_context.prefix.bkg.smooth-gauss-size
 *   - base_context.prefix.det.effective-gain
 *   - base_context.prefix.det.saturation
 *
 * The CLI aliases omit the base_context.
 */
/*----------------------------------------------------------------------------*/
cpl_parameterlist * hdrl_catalogue_parameter_create_parlist(
        const char     *base_context,
        const char     *prefix,
        hdrl_parameter *defaults)
{

    cpl_ensure(prefix && base_context && defaults,
    		CPL_ERROR_NULL_INPUT, NULL);

    cpl_ensure(hdrl_catalogue_parameter_check(defaults),
    		CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    hdrl_catalogue_parameter *par = (hdrl_catalogue_parameter*)defaults;

    cpl_parameterlist *parlist = cpl_parameterlist_new();

    hdrl_setup_vparameter(parlist, prefix, ".", "obj.", "min-pixels",
                          base_context, "Minimum pixel area for each detected "
                          "object.", CPL_TYPE_INT, par->obj_min_pixels);

    hdrl_setup_vparameter(parlist, prefix, ".", "obj.", "threshold",
                          base_context,
                          "Detection threshold in sigma above sky.",
                          CPL_TYPE_DOUBLE, par->obj_threshold);

    hdrl_setup_vparameter(parlist, prefix, ".", "obj.", "deblending",
                          base_context, "Use deblending?.",
                          CPL_TYPE_BOOL, par->obj_deblending);

    hdrl_setup_vparameter(parlist, prefix, ".", "obj.", "core-radius",
                          base_context, "Value of Rcore in pixels.",
                          CPL_TYPE_DOUBLE, par->obj_core_radius);

    hdrl_setup_vparameter(parlist, prefix, ".", "bkg.", "estimate",
                          base_context, "Estimate background from input, if "
                          "false it is assumed input is already background "
                          "corrected with median 0",
                          CPL_TYPE_BOOL, par->bkg_estimate);

    hdrl_setup_vparameter(parlist, prefix, ".", "bkg.", "mesh-size",
                          base_context, "Background smoothing box size.",
                          CPL_TYPE_INT, par->bkg_mesh_size);

    /* --prefix.result-type not an cpl option*/

    hdrl_setup_vparameter(parlist, prefix, ".", "bkg.", "smooth-gauss-fwhm",
                          base_context, "The FWHM of the Gaussian kernel used "
                          "in convolution for object detection.",
                          CPL_TYPE_DOUBLE, par->bkg_smooth_fwhm);

    hdrl_setup_vparameter(parlist, prefix, ".", "det.", "effective-gain",
                          base_context, "Detector gain value to rescale "
                          "convert intensity to electrons",
                          CPL_TYPE_DOUBLE, par->det_eff_gain);

    hdrl_setup_vparameter(parlist, prefix, ".", "det.", "saturation",
                          base_context, "Detector saturation value",
                          CPL_TYPE_DOUBLE, par->det_saturation);

    if (cpl_error_get_code()) {
        cpl_parameterlist_delete(parlist);
        return NULL;
    }

    return parlist;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Parse parameter list to create input parameters for the catalogue
 * @param parlist        parameter list to parse
 * @param prefix         prefix of parameter name
 * @return   Input parameters for the catalogue computation
 *
 * Reads a parameter list in order to create catalogue parameters.
 *
 * Expects a parameter list containing:
 *   - base_context.prefix.obj.min-pixels
 *   - base_context.prefix.obj.threshold
 *   - base_context.prefix.obj.deblending
 *   - base_context.prefix.obj.core-radius
 *   - base_context.prefix.bkg.estimate
 *   - base_context.prefix.bkg.mesh-size
 *   - base_context.prefix.bkg.smooth-gauss-fwhm
 *   - base_context.prefix.det.effective-gain
 *   - base_context.prefix.det.saturation
 *
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_catalogue_parameter_parse_parlist(
        const cpl_parameterlist *   parlist,
        const char              *   prefix)
{
    cpl_ensure(prefix && parlist, CPL_ERROR_NULL_INPUT, NULL);
    char                *   name ;
    const cpl_parameter *   par;
    int obj_min_pixels, bkg_mesh_size;
    cpl_boolean obj_deblending, bkg_estimate;
    double obj_threshold, obj_core_radius, bkg_smooth_fwhm, det_eff_gain,
           det_saturation;


    name = hdrl_join_string(".", 2, prefix, "obj.min-pixels");
    par = cpl_parameterlist_find_const(parlist, name);
    obj_min_pixels = cpl_parameter_get_int(par);
    cpl_free(name) ;

    name = hdrl_join_string(".", 2, prefix, "obj.threshold");
    par = cpl_parameterlist_find_const(parlist, name);
    obj_threshold = cpl_parameter_get_double(par);
    cpl_free(name) ;

    name = hdrl_join_string(".", 2, prefix, "obj.deblending");
    par = cpl_parameterlist_find_const(parlist, name);
    obj_deblending = cpl_parameter_get_bool(par);
    cpl_free(name) ;

    name = hdrl_join_string(".", 2, prefix, "obj.core-radius");
    par=cpl_parameterlist_find_const(parlist, name);
    obj_core_radius = cpl_parameter_get_double(par);
    cpl_free(name) ;

    name = hdrl_join_string(".", 2, prefix, "bkg.estimate");
    par = cpl_parameterlist_find_const(parlist, name);
    bkg_estimate = cpl_parameter_get_bool(par);
    cpl_free(name) ;

    name = hdrl_join_string(".", 2, prefix, "bkg.mesh-size");
    par = cpl_parameterlist_find_const(parlist, name);
    bkg_mesh_size = cpl_parameter_get_int(par);
    cpl_free(name) ;

    name = hdrl_join_string(".", 2, prefix, "bkg.smooth-gauss-fwhm");
    par = cpl_parameterlist_find_const(parlist, name);
    bkg_smooth_fwhm = cpl_parameter_get_double(par);
    cpl_free(name) ;

    name = hdrl_join_string(".", 2, prefix, "det.effective-gain");
    par = cpl_parameterlist_find_const(parlist, name);
    det_eff_gain = cpl_parameter_get_double(par);
    cpl_free(name) ;

    name = hdrl_join_string(".", 2, prefix, "det.saturation");
    par = cpl_parameterlist_find_const(parlist, name);
    det_saturation = cpl_parameter_get_double(par);
    cpl_free(name) ;

    if (cpl_error_get_code()) {
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                        "Error while parsing parameterlist with prefix %s", prefix);
        return NULL;
    } else {
        return hdrl_catalogue_parameter_create(obj_min_pixels, obj_threshold,
                                               obj_deblending, obj_core_radius,
                                               bkg_estimate,
                                               bkg_mesh_size, bkg_smooth_fwhm,
                                               det_eff_gain, det_saturation,
                                               HDRL_CATALOGUE_ALL);
    }

}

/*----------------------------------------------------------------------------*/
/**
 * @brief delete hdrl parameter result object
 * @param result hdrl parameter result object
 *
 * @return   void
 */
/*----------------------------------------------------------------------------*/
void hdrl_catalogue_result_delete(hdrl_catalogue_result * result)
{
    if (result == NULL) {
        return;
    }
    cpl_table_delete(result->catalogue);
    cpl_image_delete(result->background);
    cpl_image_delete(result->segmentation_map);
    cpl_propertylist_delete(result->qclist);
    cpl_free(result);
}
/*----------------------------------------------------------------------------*/
/**
 * @brief build object catalog
 * @param image_  input image
 * @param confidence_map   confidence map (optional input)
 * @param wcs     wcs information (optional input)
 * @param param_  parameter structure controlling catalog determination
 *
 * @return   void
 */
/*----------------------------------------------------------------------------*/
hdrl_catalogue_result *
hdrl_catalogue_compute(const cpl_image * image_, const cpl_image * confidence_map,
                       const cpl_wcs * wcs, hdrl_parameter * param_)
{
    hdrl_casu_fits *inf,*inconf = NULL;
    hdrl_casu_result * intres = NULL;
    cpl_ensure(image_, CPL_ERROR_NULL_INPUT, NULL);
    if (hdrl_catalogue_parameter_verify((hdrl_parameter *)param_)) {
        return NULL;
    }

    CPL_DIAG_PRAGMA_PUSH_IGN(-Wcast-qual);

    hdrl_catalogue_result * res = NULL;
    const hdrl_catalogue_parameter * param = (hdrl_catalogue_parameter *)param_;
    cpl_image * image = (cpl_image *)image_;
    if (cpl_image_get_type(image) != CPL_TYPE_DOUBLE) {
        image = cpl_image_cast(image, CPL_TYPE_DOUBLE);
    }

    inf = hdrl_casu_fits_wrap((cpl_image *)image);

    cpl_image * conf_ = (cpl_image*)confidence_map;

    CPL_DIAG_PRAGMA_POP;


    if (confidence_map) {
        if (cpl_image_get_min(confidence_map) < 0) {
            cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT,
                                  "confidence_map must only contain "
                                  "positive numbers");
            goto cleanup;
        }
    }

    if (cpl_image_get_bpm_const(image) != NULL) {
        if (confidence_map) {
            conf_ = cpl_image_cast(confidence_map, CPL_TYPE_DOUBLE);
        }
        else {
            conf_ = cpl_image_new(cpl_image_get_size_x(image),
                                  cpl_image_get_size_y(image), CPL_TYPE_DOUBLE);
            cpl_image_add_scalar(conf_, 100);
        }
        cpl_image_reject_from_mask(conf_, cpl_image_get_bpm_const(image));
        cpl_image_fill_rejected(conf_, 0);
        cpl_image_accept_all(conf_);
    }
    else if (confidence_map &&
             cpl_image_get_type(confidence_map) != CPL_TYPE_DOUBLE) {
        conf_ = cpl_image_cast(confidence_map, CPL_TYPE_DOUBLE);
    }
    inconf = hdrl_casu_fits_wrap((cpl_image *)conf_);

    /* Run catalogue */
    res = cpl_calloc(sizeof(hdrl_catalogue_result), 1);
    intres = cpl_calloc(sizeof(hdrl_casu_result), 1);
    hdrl_casu_catalogue(inf, inconf, wcs,
						param->obj_min_pixels, param->obj_threshold,
						param->obj_deblending, param->obj_core_radius,
						param->bkg_estimate,
						param->bkg_mesh_size, (int)param->resulttype,
						param->bkg_smooth_fwhm, param->det_eff_gain,
						param->det_saturation, intres);

    if (intres->catalogue) {
        res->catalogue = cpl_table_duplicate(       hdrl_casu_tfits_get_table(intres->catalogue));
        res->qclist    = cpl_propertylist_duplicate(hdrl_casu_tfits_get_ehu(  intres->catalogue));
        hdrl_cleanup_qclist(res->qclist);
    }
    res->segmentation_map = intres->segmentation_map;
    res->background       = intres->background;

cleanup:
    inf->image = NULL;
    if (image_ != image) {
        cpl_image_delete(image);
    }
    if (inconf && inconf->image == confidence_map) {
        inconf->image = NULL;
    }
    hdrl_casu_fits_delete(inf);
    if (intres) {
        hdrl_casu_tfits_delete(intres->catalogue);
    }
    hdrl_casu_fits_delete(inconf);
    cpl_free(intres);

    return res;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Cleaning of the quality control propertylist
 * @param qclist        propertylist to be cleaned
 * @return  cpl_error_code
 */

static cpl_error_code hdrl_cleanup_qclist(cpl_propertylist * qclist){

    cpl_propertylist * qclocal = cpl_propertylist_duplicate(qclist);
    cpl_propertylist_empty(qclist);

    if (cpl_propertylist_has(qclocal, "APCOR1"))
        cpl_propertylist_copy_property(qclist, qclocal, "APCOR1");
    if (cpl_propertylist_has(qclocal, "APCOR2"))
        cpl_propertylist_copy_property(qclist, qclocal, "APCOR2");
    if (cpl_propertylist_has(qclocal, "APCOR3"))
        cpl_propertylist_copy_property(qclist, qclocal, "APCOR3");
    if (cpl_propertylist_has(qclocal, "APCOR4"))
        cpl_propertylist_copy_property(qclist, qclocal, "APCOR4");
    if (cpl_propertylist_has(qclocal, "APCOR5"))
        cpl_propertylist_copy_property(qclist, qclocal, "APCOR5");
    if (cpl_propertylist_has(qclocal, "APCOR6"))
        cpl_propertylist_copy_property(qclist, qclocal, "APCOR6");
    if (cpl_propertylist_has(qclocal, "APCOR7"))
        cpl_propertylist_copy_property(qclist, qclocal, "APCOR7");
    if (cpl_propertylist_has(qclocal, "APCORPK"))
        cpl_propertylist_copy_property(qclist, qclocal, "APCORPK");
    if (cpl_propertylist_has(qclocal, "SYMBOL1"))
        cpl_propertylist_copy_property(qclist, qclocal, "SYMBOL1");
    if (cpl_propertylist_has(qclocal, "SYMBOL2"))
        cpl_propertylist_copy_property(qclist, qclocal, "SYMBOL2");
    if (cpl_propertylist_has(qclocal, "SYMBOL3"))
        cpl_propertylist_copy_property(qclist, qclocal, "SYMBOL3");
    if (cpl_propertylist_has(qclocal, "SYMBOL4"))
        cpl_propertylist_copy_property(qclist, qclocal, "SYMBOL4");
    if (cpl_propertylist_has(qclocal, "SYMBOL5"))
        cpl_propertylist_copy_property(qclist, qclocal, "SYMBOL5");
    if (cpl_propertylist_has(qclocal, "SYMBOL6"))
        cpl_propertylist_copy_property(qclist, qclocal, "SYMBOL6");
    if (cpl_propertylist_has(qclocal, "SYMBOL7"))
        cpl_propertylist_copy_property(qclist, qclocal, "SYMBOL7");
    if (cpl_propertylist_has(qclocal, "SYMBOL8"))
        cpl_propertylist_copy_property(qclist, qclocal, "SYMBOL8");
    if (cpl_propertylist_has(qclocal, "SYMBOL9"))
        cpl_propertylist_copy_property(qclist, qclocal, "SYMBOL9");

/*
    if (cpl_propertylist_has(qclocal, "ESO DRS CLASSIFD"))
        cpl_propertylist_copy_property(qclist, qclocal, "ESO DRS CLASSIFD");
    if (cpl_propertylist_has(qclocal, "ESO DRS CROWDED"))
        cpl_propertylist_copy_property(qclist, qclocal, "ESO DRS CROWDED");
    if (cpl_propertylist_has(qclocal, "ESO DRS FILTFWHM"))
        cpl_propertylist_copy_property(qclist, qclocal, "ESO DRS FILTFWHM");
    if (cpl_propertylist_has(qclocal, "ESO DRS MINPIX"))
        cpl_propertylist_copy_property(qclist, qclocal, "ESO DRS MINPIX");
    if (cpl_propertylist_has(qclocal, "ESO DRS NXOUT"))
        cpl_propertylist_copy_property(qclist, qclocal, "ESO DRS NXOUT");
    if (cpl_propertylist_has(qclocal, "ESO DRS NYOUT"))
        cpl_propertylist_copy_property(qclist, qclocal, "ESO DRS NYOUT");
    if (cpl_propertylist_has(qclocal, "ESO DRS RCORE"))
        cpl_propertylist_copy_property(qclist, qclocal, "ESO DRS RCORE");
    if (cpl_propertylist_has(qclocal, "ESO DRS SEEING"))
        cpl_propertylist_copy_property(qclist, qclocal, "ESO DRS SEEING");
    if (cpl_propertylist_has(qclocal, "ESO DRS THRESHOL"))
        cpl_propertylist_copy_property(qclist, qclocal, "ESO DRS THRESHOL");
    if (cpl_propertylist_has(qclocal, "ESO DRS XCOL"))
        cpl_propertylist_copy_property(qclist, qclocal, "ESO DRS XCOL");
    if (cpl_propertylist_has(qclocal, "ESO DRS YCOL"))
        cpl_propertylist_copy_property(qclist, qclocal, "ESO DRS YCOL");
    if (cpl_propertylist_has(qclocal, "ESO QC APERTURE_CORR"))
        cpl_propertylist_copy_property(qclist, qclocal, "ESO QC APERTURE_CORR");
    if (cpl_propertylist_has(qclocal, "ESO QC ELLIPTICITY"))
        cpl_propertylist_copy_property(qclist, qclocal, "ESO QC ELLIPTICITY");
    if (cpl_propertylist_has(qclocal, "ESO QC IMAGE_SIZE"))
        cpl_propertylist_copy_property(qclist, qclocal, "ESO QC IMAGE_SIZE");
    if (cpl_propertylist_has(qclocal, "ESO QC MEAN_SKY"))
        cpl_propertylist_copy_property(qclist, qclocal, "ESO QC MEAN_SKY");
    if (cpl_propertylist_has(qclocal, "ESO QC NOISE_OBJ"))
        cpl_propertylist_copy_property(qclist, qclocal, "ESO QC NOISE_OBJ");
    if (cpl_propertylist_has(qclocal, "ESO QC POSANG"))
        cpl_propertylist_copy_property(qclist, qclocal, "ESO QC POSANG");
    if (cpl_propertylist_has(qclocal, "ESO QC SATURATION"))
        cpl_propertylist_copy_property(qclist, qclocal, "ESO QC SATURATION");
    if (cpl_propertylist_has(qclocal, "ESO QC SKY_NOISE"))
        cpl_propertylist_copy_property(qclist, qclocal, "ESO QC SKY_NOISE");
    if (cpl_propertylist_has(qclocal, "HISTORY"))
        cpl_propertylist_copy_property(qclist, qclocal, "HISTORY");

*/

    cpl_propertylist_delete(qclocal);

    return cpl_error_get_code();
}
/**@}*/
