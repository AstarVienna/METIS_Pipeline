/*
 * This file is part of the HDRL
 * Copyright (C) 2014 European Southern Observatory
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
#include "hdrl_fit.h"
#include "hdrl_bpm_fit.h"

#include <cpl.h>
#include <string.h>
#include <math.h>

/*-----------------------------------------------------------------------------
                                   Static
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_bpm_fit     Bad Pixel Mask via fitting
  @ingroup hdrl_bpm

  @brief
   Algorithms to detect bad-pixels on a sequence of images like e.g. domeflats

 The routine in this module derives bad pixels on a sequence of images (e.g.
 domeflats with differend exposure time). The algorithm fits a polynomial to each
 pixel-sequence and determinates bad pixels based on this fit and various
 thresholding methods.

 The calculation is performed by calling the top-level function
 hdrl_bpm_fit_compute() and the parameters passed to this function can be created
 by calling
 hdrl_bpm_fit_parameter_create_rel_chi(), \b or
 hdrl_bpm_fit_parameter_create_rel_coef(), \b or
 hdrl_bpm_fit_parameter_create_pval(), depending on the method one would like
 to use.
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/** @cond PRIVATE */

double igamc(double, double);

/*-----------------------------------------------------------------------------
                        BPM Parameters Definition
 -----------------------------------------------------------------------------*/
typedef struct {
    HDRL_PARAMETER_HEAD;
    int        degree;
    double     pval;
    double     rel_chi_l;
    double     rel_chi_h;
    double     rel_coef_l;
    double     rel_coef_h;
} hdrl_bpm_fit_parameter;

/* Parameter type */
static hdrl_parameter_typeobj hdrl_bpm_fit_parameter_type = {
    HDRL_PARAMETER_BPM_FIT,                 /* type */
    (hdrl_alloc *)&cpl_malloc,              /* fp_alloc */
    (hdrl_free *)&cpl_free,                 /* fp_free */
    NULL,                                   /* fp_destroy */
    sizeof(hdrl_bpm_fit_parameter),         /* obj_size */
};


/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief create general bpm_fit parameter only one cut must be > 0
 * @param degree   degree of fit
 * @param pval     p-value bpm cutoff
 * @param rel_chi  relative chi bpm cutoff
 * @param rel_coef relative coefficient bpm cutoff
 *
 * @return  valud hdrl_parameter or NULL on error
 */
/* ---------------------------------------------------------------------------*/
static hdrl_parameter *
hdrl_bpm_fit_parameter_create_all(int degree, double pval,
                                  double rel_chi_l, double rel_chi_h,
                                  double rel_coef_l, double rel_coef_h)
{
    hdrl_bpm_fit_parameter * p = (hdrl_bpm_fit_parameter *)
               hdrl_parameter_new(&hdrl_bpm_fit_parameter_type);
    p->degree = degree;
    p->pval = pval;
    p->rel_chi_l = rel_chi_l;
    p->rel_chi_h = rel_chi_h;
    p->rel_coef_l = rel_coef_l;
    p->rel_coef_h = rel_coef_h;
    if (hdrl_bpm_fit_parameter_verify((const hdrl_parameter *)p) ==
        CPL_ERROR_NONE) {
        return (hdrl_parameter *)p;
    }
    else {
        hdrl_parameter_delete((hdrl_parameter*)p);
        return NULL;
    }
}

/** @endcond */

/* ---------------------------------------------------------------------------*/
/**
 * @brief create bpm_fit parameter with p-value bpm treshold
 * @param degree  degree of fit
 * @param pval    p-value of bpm threshold
 * @return hdrl_parameter or NULL on error
 * @see hdrl_bpm_fit_compute()
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter *
hdrl_bpm_fit_parameter_create_pval(int degree, double pval)
{
    return hdrl_bpm_fit_parameter_create_all(degree, pval, -1, -1, -1, -1);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief create bpm_fit parameter with relative chi bpm treshold
 * @param degree  degree of fit
 * @param rel_chi_low relative chi distribution bpm lower threshold
 * @param rel_chi_high relative chi distribution bpm upper threshold
 * @return hdrl_parameter or NULL on error
 * @see hdrl_bpm_fit_compute()
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter *
hdrl_bpm_fit_parameter_create_rel_chi(int degree,
                                      double rel_chi_low, double rel_chi_high)
{
    return hdrl_bpm_fit_parameter_create_all(degree, -1,
                                         rel_chi_low, rel_chi_high, -1, -1);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief create bpm_fit parameter with relative coefficient bpm treshold
 * @param degree   degree of fit
 * @param rel_coef_low relative fit coefficient distribution bpm lower threshold
 * @param rel_coef_high relative fit coefficient distribution bpm upper threshold
 * @return hdrl_parameter or NULL on error
 * @see hdrl_bpm_fit_compute()
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter *
hdrl_bpm_fit_parameter_create_rel_coef(int degree,
                                   double rel_coef_low, double rel_coef_high)
{
    return hdrl_bpm_fit_parameter_create_all(degree, -1,
                                         -1, -1, rel_coef_low, rel_coef_high);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Check that the parameter is a bpm_fit parameter
  @param    self The parameter to check
  @return   True or False
 */
/*----------------------------------------------------------------------------*/
cpl_boolean hdrl_bpm_fit_parameter_check(const hdrl_parameter * self)
{
    return hdrl_parameter_check_type(self, &hdrl_bpm_fit_parameter_type);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief verify that the parameter is a valid bpm_fit_parameter
 * @param p parameter
 * @return error code
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_bpm_fit_parameter_verify(const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, 0);
    cpl_ensure(hdrl_bpm_fit_parameter_check(p),
               CPL_ERROR_INCOMPATIBLE_INPUT, 0);
    const hdrl_bpm_fit_parameter * par = (const hdrl_bpm_fit_parameter *)p;
    int have_par = 0;
    if (par->degree < 0) {
        return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                                     "degree must be positive");
    }

    if (par->pval >= 0) {
        if (par->pval > 100.) {
            return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                                     "pval must be between 0 and 100%%");
        }
        have_par = 1;
    }

    if (par->rel_chi_l >= 0 || par->rel_chi_h >= 0) {
        if (have_par) {
            return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                                     "Only one rejection criteria is allowed, "
                                     "set the others to negative values");
        }
        if (!(par->rel_chi_l >= 0 && par->rel_chi_h >= 0)) {
            return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                         "Upper and lower rejection criteria must be >= 0");
        }
        have_par = 1;
    }

    if (par->rel_coef_l >= 0 || par->rel_coef_h >= 0) {
        if (have_par) {
            return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                                     "Only one rejection criteria is allowed, "
                                     "set the others to negative values");
        }
        if (!(par->rel_coef_l >= 0 && par->rel_coef_h >= 0)) {
            return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                         "Upper and lower rejection criteria must be >= 0");
        }
        have_par = 1;
    }

    if (!have_par) {
        return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                 "Only no bad pixel parameter given, the chosen threshold "
                 "must have a value larger than zero");
    }
    return CPL_ERROR_NONE;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get degree of polynomial fit of parameter
 * @param p parameter
 * @return degree of polynomial fit
 */
/* ---------------------------------------------------------------------------*/
int hdrl_bpm_fit_parameter_get_degree(const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, 0);
    cpl_ensure(hdrl_bpm_fit_parameter_check(p),
               CPL_ERROR_INCOMPATIBLE_INPUT, 0);
    return ((const hdrl_bpm_fit_parameter *)p)->degree;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get p-value bpm treshold
 * @param p parameter
 * @return p-value
 */
/* ---------------------------------------------------------------------------*/
double hdrl_bpm_fit_parameter_get_pval( const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.);
    cpl_ensure(hdrl_bpm_fit_parameter_check(p),
               CPL_ERROR_INCOMPATIBLE_INPUT, -1.);
    return ((const hdrl_bpm_fit_parameter *)p)->pval;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get relative chi distribution lower threshold
 * @param p parameter
 * @return relative chi distribution lower threshold
 */
/* ---------------------------------------------------------------------------*/
double hdrl_bpm_fit_parameter_get_rel_chi_low(const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.);
    cpl_ensure(hdrl_bpm_fit_parameter_check(p),
               CPL_ERROR_INCOMPATIBLE_INPUT, -1.);
    return ((const hdrl_bpm_fit_parameter *)p)->rel_chi_l;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get relative chi distribution upper threshold
 * @param p parameter
 * @return relative chi distribution upper threshold
 */
/* ---------------------------------------------------------------------------*/
double hdrl_bpm_fit_parameter_get_rel_chi_high(const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.);
    cpl_ensure(hdrl_bpm_fit_parameter_check(p),
               CPL_ERROR_INCOMPATIBLE_INPUT, -1.);
    return ((const hdrl_bpm_fit_parameter *)p)->rel_chi_h;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get relative fit coefficient distribution lower threshold
 * @param p parameter
 * @return relative fit coefficient distribution lower threshold
 */
/* ---------------------------------------------------------------------------*/
double hdrl_bpm_fit_parameter_get_rel_coef_low(const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.);
    cpl_ensure(hdrl_bpm_fit_parameter_check(p),
               CPL_ERROR_INCOMPATIBLE_INPUT, -1.);
    return ((const hdrl_bpm_fit_parameter *)p)->rel_coef_l;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get relative fit coefficient distribution upper threshold
 * @param p parameter
 * @return relative fit coefficient distribution upper threshold
 */
/* ---------------------------------------------------------------------------*/
double hdrl_bpm_fit_parameter_get_rel_coef_high(const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.);
    cpl_ensure(hdrl_bpm_fit_parameter_check(p),
               CPL_ERROR_INCOMPATIBLE_INPUT, -1.);
    return ((const hdrl_bpm_fit_parameter *)p)->rel_coef_h;
}

/*----------------------------------------------------------------------------*/
/**
  @brief Create a parameter list for the BPM_FIT computation
  @param base_context    base context of parameter (e.g. recipe name)
  @param prefix          prefix of parameter, may be empty string
  @param defaults        default values

  Creates a parameterlist with the BPM_FIT parameters:
    - base_context.prefix.degree
    - base_context.prefix.pval
    - base_context.prefix.rel-chi-low
    - base_context.prefix.rel-chi-high
    - base_context.prefix.rel-coef-low
    - base_context.prefix.rel-coef-high
  The CLI aliases omit the base_context.
 */
/*----------------------------------------------------------------------------*/
cpl_parameterlist * hdrl_bpm_fit_parameter_create_parlist(
        const char           *base_context,
        const char           *prefix,
        const hdrl_parameter *defaults)
{
    cpl_ensure(prefix && base_context && defaults,
    		CPL_ERROR_NULL_INPUT, NULL);

    cpl_ensure(hdrl_bpm_fit_parameter_check(defaults),
    		   CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    cpl_parameterlist *parlist = cpl_parameterlist_new();
        
    int  degree_default = hdrl_bpm_fit_parameter_get_degree(defaults);
    double pval_default = hdrl_bpm_fit_parameter_get_pval(defaults) ;
    double rel_chi_low_default = 
        hdrl_bpm_fit_parameter_get_rel_chi_low(defaults) ;
    double rel_chi_high_default =
        hdrl_bpm_fit_parameter_get_rel_chi_high(defaults) ;
    double rel_coef_low_default =
        hdrl_bpm_fit_parameter_get_rel_coef_low(defaults) ;
    double rel_coef_high_default =
        hdrl_bpm_fit_parameter_get_rel_coef_high(defaults);

    hdrl_setup_vparameter(parlist, prefix, ".", "", "degree", base_context,
            "Degree of polynomial to fit.", CPL_TYPE_INT, degree_default);

    hdrl_setup_vparameter(parlist, prefix, ".", "", "pval", base_context,
          "p-value threshold (in percent). Fits with a p-value below "
          "this threshold are considered bad pixels.",
          CPL_TYPE_DOUBLE, pval_default);

    hdrl_setup_vparameter(parlist, prefix, ".", "", "rel-chi-low", base_context,
        "Relative chi threshold. Pixels with with a chi value "
        "smaller than mean - rel-threshold * stdev-of-chi are "
        "considered bad pixels.", CPL_TYPE_DOUBLE, rel_chi_low_default);

    hdrl_setup_vparameter(parlist, prefix, ".", "", "rel-chi-high", base_context,
        "Relative chi threshold. Pixels with with a chi value larger "
        "than mean + rel-threshold * stdev-of-chi are "
        "considered bad pixels.", CPL_TYPE_DOUBLE, rel_chi_high_default);

    hdrl_setup_vparameter(parlist, prefix, ".", "", "rel-coef-low", base_context,
              "Relative fit coefficient threshold. Pixels with with a "
              "coefficient value smaller than "
              "mean +- rel-threshold * stdev-of-coeff are "
              "considered bad pixels.", CPL_TYPE_DOUBLE, rel_coef_low_default);

    hdrl_setup_vparameter(parlist, prefix, ".", "", "rel-coef-high", base_context,
              "Relative fit coefficient threshold. Pixels with with a "
              "coefficient value larger than "
              "mean +- rel-threshold * stdev-of-coeff are "
              "considered bad pixels.", CPL_TYPE_DOUBLE, rel_coef_high_default);

    if (cpl_error_get_code()) {
        cpl_parameterlist_delete(parlist);
        return NULL;
    }

    return parlist;
}

/*----------------------------------------------------------------------------*/
/**
  @brief Parse a parameterlist to create input parameters for the BPM_FIT
  @param parlist        parameter list to parse
  @param prefix         prefix of parameter name
  @return   Input parameter for the BPM_FIT computation

  Reads a parameterlist in order to create BPM image parameters.
  Expects a parameterlist containing:
   - prefix.degree
   - prefix.pval
   - prefix.rel-chi-low
   - prefix.rel-chi-high
   - prefix.rel-coef-low
   - prefix.rel-coef-high
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_bpm_fit_parameter_parse_parlist(
        const cpl_parameterlist *   parlist,
        const char              *   prefix)
{
    cpl_ensure(prefix && parlist, CPL_ERROR_NULL_INPUT, NULL);
    char                *   name;
    const cpl_parameter *   par;
    double                  pval = -1;
    double                  rel_chi_l = -1;
    double                  rel_chi_h = -1;
    double                  rel_coef_l = -1;
    double                  rel_coef_h = -1;
    int                     degree;

    name = hdrl_join_string(".", 2, prefix, "degree");
    par = cpl_parameterlist_find_const(parlist, name);
    if (par == NULL) {
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                              "Parameter %s not found", name);
        cpl_free(name);
        return NULL;
    }
    degree = cpl_parameter_get_int(par);
    cpl_free(name);

    name = hdrl_join_string(".", 2, prefix, "pval");
    par = cpl_parameterlist_find_const(parlist, name);
    if (par) {
        pval = cpl_parameter_get_double(par);
    }
    cpl_free(name) ;

    name = hdrl_join_string(".", 2, prefix, "rel-chi-low");
    par = cpl_parameterlist_find_const(parlist, name);
    if (par) {
        rel_chi_l = cpl_parameter_get_double(par);
    }
    cpl_free(name) ;

    name = hdrl_join_string(".", 2, prefix, "rel-chi-high");
    par = cpl_parameterlist_find_const(parlist, name);
    if (par) {
        rel_chi_h = cpl_parameter_get_double(par);
    }
    cpl_free(name) ;

    name = hdrl_join_string(".", 2, prefix, "rel-coef-low");
    par = cpl_parameterlist_find_const(parlist, name);
    if (par) {
        rel_coef_l = cpl_parameter_get_double(par);
    }
    cpl_free(name);

    name = hdrl_join_string(".", 2, prefix, "rel-coef-high");
    par = cpl_parameterlist_find_const(parlist, name);
    if (par) {
        rel_coef_h = cpl_parameter_get_double(par);
    }
    cpl_free(name);

    if (cpl_error_get_code()) {
        return NULL;
    }
    /* does verification */
    return hdrl_bpm_fit_parameter_create_all(degree, pval,
                                 rel_chi_l, rel_chi_h, rel_coef_l, rel_coef_h);
}

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief Utility to get a integer mask from a (stdev) relative threshold
 * @param img    image to threshold
 * @param kappa_low (stdev) relative threshold
 * @param kappa_high  (stdev) relative threshold
 * @param mad   if true estimate stdev via mad
 * @return integer mask where threshold is exceeded
 */
/* ---------------------------------------------------------------------------*/
static cpl_image *
bpm_from_rel(cpl_image * img, double kappa_low, double kappa_high, int mad)
{
    cpl_image * r;
    double m, s;
    if (mad) {
        m = cpl_image_get_mad(img, &s);
        s *= CPL_MATH_STD_MAD;
        s = CX_MAX(DBL_EPSILON, s);
    }
    else {
        m = cpl_image_get_mean(img);
        s = cpl_image_get_stdev(img);
    }
    cpl_mask * bpm = cpl_mask_threshold_image_create(img, m - s * kappa_low,
                                                     m + s * kappa_high);
    cpl_mask_not(bpm);
    r = cpl_image_new_from_mask(bpm);
    cpl_mask_delete(bpm);
    return r;
}



/* ---------------------------------------------------------------------------*/
/**
  @brief compute bad pixel map based on fitting a stack of images
  @param par         input bpm_fit_parameter
  @param data        hdrl_imagelist to fit
  @param sample_pos  cpl_vector of sampling position of the images in data,
                     e.g. exposure time
  @param out_mask    output bad pixel mask as integer image
  @return CPL_ERROR_NONE if everything is ok, an error code otherwise

The function fits a polynomial of degree \e degree to
the imagelist at the sampling positions defined in \e sample_pos.

Three methods are available to convert the information from the fit
into a bad pixel map:


\li \c Relative cutoff on the chi distribution of all fits. Pixels
       with chi values strictly outside the interval \f$ mean(\chi) -
       stdev(\chi) \times rel\_chi\_low \f$ and \f$ mean(\chi) +
       stdev(\chi) \times rel\_chi\_high \f$ are considered bad. The
       corresponding hdrl parameter is created by
       hdrl_bpm_fit_parameter_create_rel_chi()

\li \c Relative cutoff on the distribution of the fit
       coefficients. Pixels with fit coefficients strictly outside the
       interval \f$ mean(coef) - stdev(coef) \times rel\_coef\_low
       \f$ and \f$ mean(coef) + stdev(coef) \times rel\_coef\_high
       \f$ are considered bad.  The coefficient numbers that caused
       the pixel to be marked as bad are encoded as powers of two of
       their degree (starting from 0). The corresponding hdrl
       parameter is created by
       hdrl_bpm_fit_parameter_create_rel_coef()

\li \c Pixels with low \e p-value. When the errors of the pixels are
       correct the \e p-value can be interpreted as the probability
       with which the pixel response fits the chosen model. The
       corresponding hdrl parameter is created by
       hdrl_bpm_fit_parameter_create_pval()
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code
hdrl_bpm_fit_compute(const hdrl_parameter * par,
                     const hdrl_imagelist * data,
                     const cpl_vector * sample_pos,
                     cpl_image ** out_mask)
{
    cpl_image * out_chi2 = NULL, * out_dof = NULL;
    hdrl_imagelist * out_coef = NULL;
    if (hdrl_bpm_fit_parameter_verify(par) != CPL_ERROR_NONE)
        return cpl_error_get_code();

    int degree = hdrl_bpm_fit_parameter_get_degree(par);

    /* TODO check for 0 error,
     * in that case set to 1 and do not allow pval */

    cpl_error_code err = hdrl_fit_polynomial_imagelist(data, sample_pos, degree,
                                               &out_coef, &out_chi2, &out_dof);
    if (err) {
        return cpl_error_set_message(cpl_func, CPL_ERROR_FILE_NOT_FOUND,
                                     "Fit failed");
    }

    if (cpl_image_count_rejected(out_chi2) ==
        (cpl_image_get_size_x(out_chi2) * cpl_image_get_size_y(out_chi2))) {
        cpl_msg_error(cpl_func, "Too few good pixels to fit polynomial of "
                      "degree %d in all pixels", degree);
        goto end;
    }

    {
        cpl_image * bpm = NULL;
        double pval = hdrl_bpm_fit_parameter_get_pval(par);
        double rel_chi_l = hdrl_bpm_fit_parameter_get_rel_chi_low(par);
        double rel_chi_h = hdrl_bpm_fit_parameter_get_rel_chi_high(par);
        double rel_coef_l = hdrl_bpm_fit_parameter_get_rel_coef_low(par);
        double rel_coef_h = hdrl_bpm_fit_parameter_get_rel_coef_high(par);

        if (rel_chi_l >= 0) {
            /* chi is symmetric */
            cpl_image_power(out_chi2, 0.5);
            bpm = bpm_from_rel(out_chi2, rel_chi_l, rel_chi_h, 1);
        }
        else if (rel_coef_l >= 0) {
            for (cpl_size i = 0; i < hdrl_imagelist_get_size(out_coef); i++) {
                hdrl_image * coef = hdrl_imagelist_get(out_coef, i);
                cpl_image * b = bpm_from_rel(hdrl_image_get_image(coef),
                                             rel_coef_l, rel_coef_h, 0);

                /* bits of bpm defines which coefficient "bad" */
                if (bpm) {
                    cpl_image_multiply_scalar(b, pow(2, i));
                    cpl_image_add(bpm, b);
                    cpl_image_delete(b);
                }
                else {
                    bpm = b;
                }
            }
        }
        else if (pval >= 0) {
            pval /= 100.;
            bpm = cpl_image_new(cpl_image_get_size_x(out_chi2),
                                cpl_image_get_size_y(out_chi2),
                                CPL_TYPE_INT);
            int * md = cpl_image_get_data_int(bpm);
            hdrl_data_t * cd = cpl_image_get_data(out_chi2);
            hdrl_data_t * dd = cpl_image_get_data(out_dof);
            for (size_t i = 0; i < (size_t)cpl_image_get_size_x(out_chi2) *
                 cpl_image_get_size_y(out_chi2); i++) {
                double pv = igamc(dd[i] / 2., cd[i] / 2.);
                md[i] = (pv < pval);
            }
        }
        *out_mask = bpm;
    }

end:
    hdrl_imagelist_delete(out_coef);
    cpl_image_delete(out_chi2);
    cpl_image_delete(out_dof);

    return cpl_error_get_code();
}

/**@}*/
