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
/* for j1 */
#if !defined(_XOPEN_SOURCE) || (_XOPEN_SOURCE - 0) < 600
#define _XOPEN_SOURCE 600
#endif

#include "hdrl_strehl.h"
#include "hdrl_image.h"
#include "hdrl_types.h"
#include "hdrl_utils.h"

#include <cpl.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>

/*-----------------------------------------------------------------------------
                                   Static
 -----------------------------------------------------------------------------*/


static hdrl_strehl_result bad_result = {
    {NAN, NAN},
    NAN, NAN,
    {NAN, NAN},
    {NAN, NAN},
    {NAN, NAN},
    NAN,
    0
};

/**
 *
 * @defgroup hdrl_strehl     Strehl Computation
 *
 * @brief
 *   Function to compute the Strehl ratio on an image.
 *
 * The most commonly used metrics for evaluating the AO correction is the Strehl
 * ratio. The Strehl ratio is defined as the ratio of the peak image intensity
 * from a point source compared to the maximum attainable intensity using an
 * ideal optical system limited only by diffraction over the telescope aperture.
 * The Strehl ratio is very frequently used to perform the quality control of
 * the scientific data obtained with the AO assisted instrumentation.
 *
 * The calculation is performed by calling the top-level function
 * hdrl_strehl_compute() and the parameters passed to this function can be
 * created by calling hdrl_strehl_parameter_create().
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/


/*-----------------------------------------------------------------------------
                        Strehl parameters Definition
 -----------------------------------------------------------------------------*/

/** @cond PRIVATE */

typedef struct {
    HDRL_PARAMETER_HEAD;
    double wavelength ;
    double m1 ;
    double m2 ;
    double pixel_scale_x;
    double pixel_scale_y;
    double flux_radius;
    double bkg_radius_low;
    double bkg_radius_high;
} hdrl_strehl_parameter;

/* parameter type */
static hdrl_parameter_typeobj hdrl_strehl_parameter_type = {
    HDRL_PARAMETER_STREHL,                /* type */
    (hdrl_alloc *)&cpl_malloc,            /* fp_alloc */
    (hdrl_free *)&cpl_free,               /* fp_free */
    NULL,                                 /* fp_destroy */
    sizeof(hdrl_strehl_parameter),        /* obj_size */
};



/*----------------------------------------------------------------------------*/
/**
 * @internal
 * @brief    Verify basic correctness of the Strehl parameters
 * @param    param      Strehl parameters
 * @return   CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
hdrl_strehl_parameter_verify(const hdrl_parameter    *   param)
{
    cpl_error_ensure(param != NULL, CPL_ERROR_NULL_INPUT,
                    return CPL_ERROR_NULL_INPUT, "NULL Input Parameters");
    cpl_error_ensure(hdrl_strehl_parameter_check(param),
                     CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                                     "Expected Strehl parameter") ;

    const hdrl_strehl_parameter * param_loc = (const hdrl_strehl_parameter *)param ;

    cpl_error_ensure(param_loc->wavelength >= 0, CPL_ERROR_ILLEGAL_INPUT,
                     return CPL_ERROR_ILLEGAL_INPUT, "wavelength must be >=0");
    cpl_error_ensure(param_loc->m1 >= 0, CPL_ERROR_ILLEGAL_INPUT,
                     return CPL_ERROR_ILLEGAL_INPUT, "m1 radius must be >=0");
    cpl_error_ensure(param_loc->m2 >= 0, CPL_ERROR_ILLEGAL_INPUT,
                     return CPL_ERROR_ILLEGAL_INPUT, "m2 radius must be >=0");
    cpl_error_ensure(param_loc->m1 > param_loc->m2, CPL_ERROR_ILLEGAL_INPUT,
                     return CPL_ERROR_ILLEGAL_INPUT,
                     "m1 radius must be larger than m2 radius");
    cpl_error_ensure(param_loc->pixel_scale_x >= 0, CPL_ERROR_ILLEGAL_INPUT,
                     return CPL_ERROR_ILLEGAL_INPUT, "pixel_scale_x must be >=0");
    cpl_error_ensure(param_loc->pixel_scale_y >= 0, CPL_ERROR_ILLEGAL_INPUT,
                     return CPL_ERROR_ILLEGAL_INPUT, "pixel_scale_y must be >=0");

    cpl_error_ensure(param_loc->flux_radius >= 0, CPL_ERROR_ILLEGAL_INPUT,
                     return CPL_ERROR_ILLEGAL_INPUT, "flux_radius must be >=0");

    cpl_error_ensure(param_loc->m1 >= param_loc->m2, CPL_ERROR_ILLEGAL_INPUT,
                     return CPL_ERROR_ILLEGAL_INPUT, "m1 must be >=m2");
    if (param_loc->bkg_radius_low > 0) {
        cpl_error_ensure(param_loc->bkg_radius_low >= param_loc->flux_radius,
                         CPL_ERROR_ILLEGAL_INPUT,return CPL_ERROR_ILLEGAL_INPUT,
                         "bkg_radius_low must be >=flux_radius");
        cpl_error_ensure(param_loc->bkg_radius_high > param_loc->bkg_radius_low,
                         CPL_ERROR_ILLEGAL_INPUT,return CPL_ERROR_ILLEGAL_INPUT,
                         "bkg_radius_high must be >bkg_radius_low");
    }
    else {
        cpl_error_ensure(param_loc->bkg_radius_high < 0,
                 CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                 "bkg_radius_high must be < 0 if bkg_radius_low is < 0");
    }

    return CPL_ERROR_NONE ;

}

/** @endcond */

/*----------------------------------------------------------------------------*/
/**
 * @brief    Creates Strehl Parameters object
 *
 * @param    wavelength      Nominal filter wavelength [m]
 * @param    m1_radius       primary mirror radius [m]
 * @param    m2_radius       obstruction radius [m]
 * @param    pixel_scale_x   image X pixel scale in [arcsec]
 * @param    pixel_scale_y   image Y pixel scale in [arcsec]
 * @param    flux_radius     radius used to sum the flux [arcsec]
 * @param    bkg_radius_low  radius used to determine the background [arcsec]
 * @param    bkg_radius_high radius used to determine the background [arcsec]
 *
 * @return   The Strehl parameters object.
 *           It needs to be deallocated with hdrl_parameter_delete()
 * @see      hdrl_parameter_delete()
 * @see      hdrl_strehl_compute()
 * The method creates a parameter to compute the Strehl
 */

hdrl_parameter * hdrl_strehl_parameter_create(double wavelength,
        double m1_radius, double m2_radius,
        double pixel_scale_x, double pixel_scale_y, double flux_radius,
        double bkg_radius_low, double bkg_radius_high) {


    hdrl_strehl_parameter * p = (hdrl_strehl_parameter *)
    hdrl_parameter_new(&hdrl_strehl_parameter_type);

    p->wavelength = wavelength ;
    p->m1 = m1_radius;
    p->m2 = m2_radius;
    p->pixel_scale_x = pixel_scale_x;
    p->pixel_scale_y = pixel_scale_y;
    p->flux_radius = flux_radius;
    p->bkg_radius_low = bkg_radius_low;
    p->bkg_radius_high = bkg_radius_high;

    if (hdrl_strehl_parameter_verify((hdrl_parameter *)p)) {
        cpl_free(p);
        return NULL;
    }
    return (hdrl_parameter *)p;

}

/*----------------------------------------------------------------------------*/
/**
 * @brief    Check that the parameter is a Strehl parameter
 * @param    self The parameter to check
 * @return   True or False
 */
/*----------------------------------------------------------------------------*/
cpl_boolean hdrl_strehl_parameter_check(const hdrl_parameter * self)
{
    return hdrl_parameter_check_type(self, &hdrl_strehl_parameter_type);
}

/*----------------------------------------------------------------------------*/
/**
 * @brief    Access the wavelength in the Strehl parameter
 * @param    p   The Strehl parameter
 * @return   The wavelength value
 */
/*----------------------------------------------------------------------------*/
double hdrl_strehl_parameter_get_wavelength(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.0);
    return p != NULL ? ((const hdrl_strehl_parameter *)p)->wavelength : 0.;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief    Access the primary mirror radius in the Strehl parameter
 * @param    p   The Strehl parameter
 * @return   The primary mirror radius value
 */
/*----------------------------------------------------------------------------*/
double hdrl_strehl_parameter_get_m1(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.0);
    return p != NULL ? ((const hdrl_strehl_parameter *)p)->m1 : 0.;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief    Access the obstruction radius in the Strehl parameter
 * @param    p   The Strehl parameter
 * @return   The obstruction radius value
 */
/*----------------------------------------------------------------------------*/
double hdrl_strehl_parameter_get_m2(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.0);
    return p != NULL ? ((const hdrl_strehl_parameter *)p)->m2 : 0.;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief    Access the image X pixel scale in the Strehl parameter
 * @param    p   The Strehl parameter
 * @return   The image X pixel scale value
 */
/*----------------------------------------------------------------------------*/
double hdrl_strehl_parameter_get_pixel_scale_x(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.0);
    return p != NULL ? ((const hdrl_strehl_parameter *)p)->pixel_scale_x : 0.;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief    Access the image Y pixel scale in the Strehl parameter
 * @param    p   The Strehl parameter
 * @return   The image Y pixel scale value
 */
/*----------------------------------------------------------------------------*/
double hdrl_strehl_parameter_get_pixel_scale_y(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.0);
    return p != NULL ? ((const hdrl_strehl_parameter *)p)->pixel_scale_y : 0.;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief    Access the total flux radius in the Strehl parameter
 * @param    p   The Strehl parameter
 * @return   The total flux radius value
 */
/*----------------------------------------------------------------------------*/
double hdrl_strehl_parameter_get_flux_radius(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.0);
    return p != NULL ? ((const hdrl_strehl_parameter *)p)->flux_radius : 0.;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief    Access the background region internal radius in the Strehl parameter
 * @param    p   The Strehl parameter
 * @return   The background region internal radius value
 */
/*----------------------------------------------------------------------------*/
double hdrl_strehl_parameter_get_bkg_radius_low(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.0);
    return p != NULL ? ((const hdrl_strehl_parameter *)p)->bkg_radius_low : 0.;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief    Access the background region external radius in the Strehl parameter
 * @param    p   The Strehl parameter
 * @return   The background region external radius value
 */
/*----------------------------------------------------------------------------*/
double hdrl_strehl_parameter_get_bkg_radius_high(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.0);
    return p != NULL ? ((const hdrl_strehl_parameter *)p)->bkg_radius_high : 0.;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Create parameter list for the Strehl computation
 * @param base_context        base context of parameter (e.g. recipe name)
 * @param prefix              prefix of parameter, may be an empty string
 * @param par                 hdrl_parameter defining the defaults
 * @see hdrl_strehl_parameter_create()
 *
 *
 * Creates a parameter list with the Strehl parameters:
 *   - base_context.prefix.wavelength
 *   - base_context.prefix.m1
 *   - base_context.prefix.m2
 *   - base_context.prefix.pixel-scale-x
 *   - base_context.prefix.pixel-scale-y
 *   - base_context.prefix.flux-radius
 *   - base_context.prefix.bkg-radius-low
 *   - base_context.prefix.bkg-radius-high
 *
 * The CLI aliases omit the base_context.
 */
/*----------------------------------------------------------------------------*/
cpl_parameterlist * hdrl_strehl_parameter_create_parlist(
        const char     *base_context,
        const char     *prefix,
        hdrl_parameter *par)
{

    cpl_ensure(prefix && base_context && par,
    		CPL_ERROR_NULL_INPUT, NULL);

    cpl_ensure(hdrl_strehl_parameter_check(par),
    		CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    cpl_parameterlist *parlist = cpl_parameterlist_new();

    /* --prefix.wavelength */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "wavelength", base_context,
                          "Wavelength [m].", CPL_TYPE_DOUBLE,
                          hdrl_strehl_parameter_get_wavelength(par));

    /* --prefix.m1 */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "m1", base_context,
                          "Telescope radius [m].", CPL_TYPE_DOUBLE,
                          hdrl_strehl_parameter_get_m1(par));

    /* --prefix.m2 */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "m2", base_context,
                          "Telescope obstruction radius [m].", CPL_TYPE_DOUBLE,
                          hdrl_strehl_parameter_get_m2(par));

    /* --prefix.pixscale_x */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "pixel-scale-x", base_context,
                          "Detector X pixel scale on sky [arcsec].",
                          CPL_TYPE_DOUBLE,
                          hdrl_strehl_parameter_get_pixel_scale_x(par));

    /* --prefix.pixscale_y */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "pixel-scale-y", base_context,
                          "Detector Y pixel scale on sky [arcsec].",
                          CPL_TYPE_DOUBLE,
                          hdrl_strehl_parameter_get_pixel_scale_y(par));

    /* --prefix.flux_radius */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "flux-radius", base_context,
                           "PSF Flux integration radius [arcsec].",
                           CPL_TYPE_DOUBLE,
                           hdrl_strehl_parameter_get_flux_radius(par));

    /* --prefix.bkg_radius_low_def */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "bkg-radius-low", base_context,
                            "PSF background inner radii [arcsec].",
                            CPL_TYPE_DOUBLE,
                            hdrl_strehl_parameter_get_bkg_radius_low(par));

    /* --prefix.bkg_radius_high_def */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "bkg-radius-high", base_context,
                            "PSF background outer radius [arcsec].",
                            CPL_TYPE_DOUBLE,
                            hdrl_strehl_parameter_get_bkg_radius_high(par));

    if (cpl_error_get_code()) {
        cpl_parameterlist_delete(parlist);
        return NULL;
    }

    return parlist;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Parse parameter list to create input parameters for the Strehl
 * @param parlist        parameter list to parse
 * @param prefix         prefix of parameter name
 * @return   Input parameters for the Strehl computation
 *
 * Reads a parameter list in order to create Strehl parameters.
 *
 * Expects a parameter list containing:
 *   - base_context.prefix.wavelength
 *   - base_context.prefix.m1
 *   - base_context.prefix.m2
 *   - base_context.prefix.pixel-scale-x
 *   - base_context.prefix.pixel-scale-y
 *   - base_context.prefix.flux-radius
 *   - base_context.prefix.bkg-radius-low
 *   - base_context.prefix.bkg-radius-high
 *
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_strehl_parameter_parse_parlist(
        const cpl_parameterlist *   parlist,
        const char              *   prefix)
{
    cpl_ensure(prefix && parlist, CPL_ERROR_NULL_INPUT, NULL);
    char                *   name;
    const cpl_parameter *   par;
    double                  wavelength, m1, m2, pixel_scale_x, pixel_scale_y,
                            flux_radius, bkg_radius_low,bkg_radius_high;


    /* --wavelength */
    name = hdrl_join_string(".", 2, prefix, "wavelength");
    par=cpl_parameterlist_find_const(parlist, name);
    wavelength = cpl_parameter_get_double(par);
    cpl_free(name) ;

    /* --m1 */
    name = hdrl_join_string(".", 2, prefix, "m1");
    par=cpl_parameterlist_find_const(parlist, name);
    m1 = cpl_parameter_get_double(par);
    cpl_free(name) ;

    /* --m2 */
    name = hdrl_join_string(".", 2, prefix, "m2");
    par=cpl_parameterlist_find_const(parlist, name);
    m2 = cpl_parameter_get_double(par);
    cpl_free(name) ;

    /* --pixel_scale_x */
    name = hdrl_join_string(".", 2, prefix, "pixel-scale-x");
    par=cpl_parameterlist_find_const(parlist, name);
    pixel_scale_x = cpl_parameter_get_double(par);
    cpl_free(name) ;

    /* --pixel_scale_y */
    name = hdrl_join_string(".", 2, prefix, "pixel-scale-y");
    par=cpl_parameterlist_find_const(parlist, name);
    pixel_scale_y = cpl_parameter_get_double(par);
    cpl_free(name) ;

    /* --flux_radius */
    name = hdrl_join_string(".", 2, prefix, "flux-radius");
    par=cpl_parameterlist_find_const(parlist, name);
    flux_radius = cpl_parameter_get_double(par);
    cpl_free(name) ;

    /* --bkg_radius_low */
    name = hdrl_join_string(".", 2, prefix, "bkg-radius-low");
    par=cpl_parameterlist_find_const(parlist, name);
    bkg_radius_low = cpl_parameter_get_double(par);
    cpl_free(name) ;

    /* --bkg_radius_high */
    name = hdrl_join_string(".", 2, prefix, "bkg-radius-high");
    par=cpl_parameterlist_find_const(parlist, name);
    bkg_radius_high = cpl_parameter_get_double(par);
    cpl_free(name) ;


    if (cpl_error_get_code()) {
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                        "Error while parsing parameterlist with prefix %s", prefix);
        return NULL;
    } else {
        return hdrl_strehl_parameter_create(wavelength, m1,m2,
                        pixel_scale_x, pixel_scale_y,
                        flux_radius, bkg_radius_low, bkg_radius_high) ;
    }

}


/** @cond PRIVATE */

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief mask where mask is valid
 * @param himg  hdrl image
 * @param mask mask defining where to find max
 * @return max and its error
 */
/* ---------------------------------------------------------------------------*/
static hdrl_value
hdrl_image_max_where(const hdrl_image * himg, cpl_mask * mask)
{
    hdrl_image * tmpimg = hdrl_image_duplicate(himg);
    cpl_size px, py;
    hdrl_value mx;
    hdrl_image_reject_from_mask(tmpimg, mask);
    cpl_image_get_maxpos(hdrl_image_get_image(tmpimg), &px, &py);
    mx = hdrl_image_get_pixel(tmpimg, px, py, NULL);
    hdrl_image_delete(tmpimg);
    return mx;
}

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief sum where mask is valid
 * @param himg  hdrl image
 * @param mask mask defining where to sum
 * @return sum and its error
 */
/* ---------------------------------------------------------------------------*/
static hdrl_value
hdrl_image_sum_where(const hdrl_image * himg, cpl_mask * mask)
{
    hdrl_image * tmpimg = hdrl_image_duplicate(himg);
    hdrl_value flux;
    hdrl_image_reject_from_mask(tmpimg, mask);
    flux = hdrl_image_get_sum(tmpimg);
    hdrl_image_delete(tmpimg);
    return flux;
}

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief median where mask is valid
 * @param himg  hdrl image
 * @param mask mask defining where to sum
 * @return sum and its error
 */
/* ---------------------------------------------------------------------------*/
static hdrl_value
hdrl_image_median_where(const hdrl_image * himg, cpl_mask * mask)
{
    hdrl_image * tmpimg = hdrl_image_duplicate(himg);
    hdrl_value flux;
    hdrl_image_reject_from_mask(tmpimg, mask);
    flux = hdrl_image_get_median(tmpimg);
    hdrl_image_delete(tmpimg);
    return flux;
}

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief stdev where mask is valid
 * @param himg  hdrl image
 * @param mask mask defining where to compute
 * @return computed standard deviation
 */
/* ---------------------------------------------------------------------------*/
static double
hdrl_image_stdev_where(const hdrl_image * himg, cpl_mask * mask)
{
    hdrl_image * tmpimg = hdrl_image_duplicate(himg);
    double mad;
    hdrl_image_reject_from_mask(tmpimg, mask);
    cpl_image_get_mad(hdrl_image_get_image_const(tmpimg), &mad);
    hdrl_image_delete(tmpimg);
    return mad * CPL_MATH_STD_MAD;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief compute obstructed Airy disk
 *
 * @param lam         wavelength [m]
 * @param m1          radius of primary mirror [m]
 * @param m2          radius of secondary mirror [m]
 * @param pixscale_x  pixel scale in x direction [arcseconds]
 * @param pixscale_y  pixel scale in y direction [arcseconds]
 * @param cx          position of center in x direction (FITS) [pixel]
 * @param cy          position of center in y direction (FITS) [pixel]
 * @param nx          size of image in x direction [pixel]
 * @param ny          size of image in y direction [pixel]
 *
 * Computes an obstructed Airy for one wavelength from following formula:
 *
 * \f$
 * I(r) = \frac{1}{1 - e^2} \left(\frac{2 j_1(r)}{r} - \frac{2 e j_1(er)}{r}\right)^2
 * \f$
 * with:
 * \f$
 * e = \frac{m_2}{m_2}
 * \;
 * r = k m_1 \sin{\theta} = \frac{2 \pi m_1}{\lambda} \sqrt{x^2 + y^2}
 * \f$
 * See Wikipedia.
 *
 * The parameters cx and cy define the center of the Airy disk.
 * E.g. a value of nx // 2 means the disk will be centered exactly at the
 * middle of the central pixel while a value of nx // 2 - 0.5 means the peak of
 * the disk will be at the origin and the flux evenly distributed among the
 * four neighboring pixels.
 */
/* ---------------------------------------------------------------------------*/
static cpl_image *
compute_psf(double lam, double m1, double m2,
            double pixscale_x, double pixscale_y,
            double cx, double cy,
            size_t nx, size_t ny)
{
    cpl_image * psf = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
    double * data = cpl_image_get_data(psf);
    double e = m2 / m1;
    double as_2_rad = CPL_MATH_2PI / (360. * 3600);
    /* inclusive linear space with pixel center at the middle. integer cx and cy
     * will result in a psf exactly centered around the middle of the central
     * pixel, cx/cy are in fits convention [1,nx/ny] with pixel origin in the
     * middle */
    double centerx = (-(nx / 2.) + cx - 1 + 0.5) * pixscale_x;
    double centery = (-(ny / 2.) + cy - 1 + 0.5) * pixscale_y;
    double xhigh = ((nx - 1) * pixscale_x / 2) - centerx;
    double yhigh = ((ny - 1) * pixscale_y / 2) - centery;
    double xlow = -((nx - 1) * pixscale_x / 2) - centerx;
    double ylow = -((ny - 1) * pixscale_y / 2) - centery;
    double step_x = (xhigh - xlow) / (nx - 1);
    double step_y = (yhigh - ylow) / (ny - 1);

    HDRL_OMP(omp parallel for)
    for (size_t iy = 0; iy < ny; iy++) {
        double y = iy == ny - 1 ? yhigh : ylow + iy * step_y;
        for (size_t ix = 0; ix < nx; ix++) {
            double x = ix == nx - 1 ? xhigh : xlow + ix * step_x;
            double r = sqrt(x*x + y*y) * as_2_rad * CPL_MATH_2PI * m1 / lam;
            if (r == 0.) {
                data[iy * nx + ix] = 1.;
            }
            else {
                double airy = (2 * j1(r) / r - 2 * e * j1(e * r) / r);
                double c = (1 - e * e);
                data[iy * nx + ix] = 1 / (c * c) * airy * airy;
            }
        }
    }
    return psf;
}

/*----------------------------------------------------------------------------*/
/**
 * @internal
 * @brief    Find the aperture(s) with the greatest flux
 * @param    self   The aperture object
 * @param    ind  The aperture-indices in order of decreasing flux
 * @param    nfind  Number of indices to find
 * @return   CPL_ERROR_NONE or the relevant _cpl_error_code_ on error
 *
 * nfind must be at least 1 and at most the size of the aperture object.
 *
 * The ind array must be able to hold (at least) nfind integers.
 * On success the first nfind elements of ind point to indices of the
 * aperture object.
 *
 * To find the single ind of the aperture with the maximum flux use simply:
 * int ind;
 * apertures_find_max_flux(self, &ind, 1);
 *
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code apertures_find_max_flux(const cpl_apertures * self,
                                              int * ind, int nfind)
{
    const int    nsize = cpl_apertures_get_size(self);
    int          ifind;


    cpl_ensure_code(nsize > 0,      cpl_error_get_code());
    cpl_ensure_code(ind,          CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(nfind > 0,      CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(nfind <= nsize, CPL_ERROR_ILLEGAL_INPUT);

    for (ifind=0; ifind < nfind; ifind++) {
        double maxflux = -1;
        int maxind = -1;
        int i;
        for (i=1; i <= nsize; i++) {
            int k;

            /* The flux has to be the highest among those not already found */
            for (k=0; k < ifind; k++) if (ind[k] == i) break;

            if (k == ifind) {
                /* i has not been inserted into ind */
                const double flux = cpl_apertures_get_flux(self, i);

                if (maxind < 0 || flux > maxflux) {
                    maxind = i;
                    maxflux = flux;
                }
            }
        }
        ind[ifind] = maxind;
    }

    return CPL_ERROR_NONE;

}

/*----------------------------------------------------------------------------*/
/**
 * @internal
 * @brief    Find the peak flux, peak sum and position of a Gaussian
 * @param    self        Image to process
 * @param    sigma       The initial detection level  [ADU]
 * @param    pxpos       On success, the refined X-position [pixel]
 * @param    pypos       On success, the refined Y-position [pixel]
 * @param    ppeak       On success, the refined peak flux  [ADU]
 * @return CPL_ERROR_NONE or the relevant CPL error code on error
 *
 * The routine initially determines the approximate position and flux value of
 * the PSF with a robust Gaussian fit: first are identified all sources that lie
 * 5 sigmas above the median of the image, then is determined the position of
 * the barycenter of the region with highest peak. Finally is performed the fit
 * of a Gaussian centered on the found barycenter position.
 *
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
gaussian_maxpos(const cpl_image * self,
                double sigma,
                double  * pxpos,
                double  * pypos,
                double  * ppeak)
{
    /* copied from irplib_strehl.c r163170 */
    const cpl_size  nx = cpl_image_get_size_x(self);
    const cpl_size  ny = cpl_image_get_size_y(self);
    int             iretry = 3; /* Number retries with decreasing sigma */
    int             ifluxapert = 0;
    double          med_dist;
    const double    median = cpl_image_get_median_dev(self, &med_dist);
    cpl_mask      * selection;
    cpl_size        nlabels = 0;
    cpl_image     * labels = NULL;
    cpl_apertures * aperts;
    cpl_size        npixobj;
    double          objradius;
    cpl_size        winsize;
    cpl_size        xposmax, yposmax;
    double          xposcen, yposcen;
    double          valmax, valfit = -1.0;
    cpl_array     * gauss_parameters = NULL;
    cpl_errorstate  prestate = cpl_errorstate_get();
    cpl_error_code  code = CPL_ERROR_NONE;


    cpl_ensure_code( sigma > 0.0, CPL_ERROR_ILLEGAL_INPUT);

    selection = cpl_mask_new(nx, ny);

    /* find aperture with signal larger than sigma * median deviation */
    for (; iretry > 0 && nlabels == 0; iretry--, sigma *= 0.5) {

        /* Compute the threshold */
        const double threshold = median + sigma * med_dist;


        /* Select the pixel above the threshold */
        code = cpl_mask_threshold_image(selection, self, threshold, DBL_MAX,
                                        CPL_BINARY_1);

        if (code) break;

        /* Labelise the thresholded selection */
        cpl_image_delete(labels);
        labels = cpl_image_labelise_mask_create(selection, &nlabels);
    }
    sigma *= 2.0; /* reverse last iteration that found no labels */

    cpl_mask_delete(selection);

    if (code) {
        cpl_image_delete(labels);
        return cpl_error_set_where(cpl_func);
    } else if (nlabels == 0) {
        cpl_image_delete(labels);
        return cpl_error_set(cpl_func, CPL_ERROR_DATA_NOT_FOUND);
    }

    aperts = cpl_apertures_new_from_image(self, labels);

    /* Find the aperture with the greatest flux */
    code = apertures_find_max_flux(aperts, &ifluxapert, 1);

    if (code) {
        cpl_apertures_delete(aperts);
        cpl_image_delete(labels);
        return cpl_error_set(cpl_func, CPL_ERROR_DATA_NOT_FOUND);
    }

    npixobj = cpl_apertures_get_npix(aperts, ifluxapert);
    objradius = sqrt((double)npixobj * CPL_MATH_1_PI);
    winsize = CX_MIN(CX_MIN(nx, ny), (3.0 * objradius));

    xposmax = cpl_apertures_get_maxpos_x(aperts, ifluxapert);
    yposmax = cpl_apertures_get_maxpos_y(aperts, ifluxapert);
    xposcen = cpl_apertures_get_centroid_x(aperts, ifluxapert);
    yposcen = cpl_apertures_get_centroid_y(aperts, ifluxapert);
    valmax  = cpl_apertures_get_max(aperts, ifluxapert);

    cpl_apertures_delete(aperts);
    cpl_image_delete(labels);

    cpl_msg_debug(cpl_func, "Object radius at S/R=%g: %g (window-size=%u)",
                  sigma, objradius, (unsigned)winsize);
    cpl_msg_debug(cpl_func, "Object-peak @ (%d, %d) = %g", (int)xposmax,
                  (int)yposmax, valmax);

    /* fit gaussian to get subpixel peak position */

    gauss_parameters = cpl_array_new(7, CPL_TYPE_DOUBLE);
    cpl_array_set_double(gauss_parameters, 0, median);

    code = cpl_fit_image_gaussian(self, NULL, xposmax, yposmax,
                                  winsize, winsize, gauss_parameters,
                                  NULL, NULL, NULL,
                                  NULL, NULL, NULL, 
                                  NULL, NULL, NULL);
    if (!code) {
        const double M_x = cpl_array_get_double(gauss_parameters, 3, NULL);
        const double M_y = cpl_array_get_double(gauss_parameters, 4, NULL);

        valfit = hcpl_gaussian_eval_2d(gauss_parameters, M_x, M_y);

        if (!cpl_errorstate_is_equal(prestate)) {
            code = cpl_error_get_code();
        } else {
            *pxpos        = M_x;
            *pypos        = M_y;
            *ppeak        = valfit;

            cpl_msg_debug(cpl_func, "Gauss-fit @ (%g, %g) = %g",
                          M_x, M_y, valfit);
        }
    }
    cpl_array_delete(gauss_parameters);

    if (code || valfit < valmax) {
        cpl_errorstate_set(prestate);
        *pxpos   = xposcen;
        *pypos   = yposcen;
        *ppeak   = valmax;
    }

    return code ? cpl_error_set_where(cpl_func) : CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    create a disk mask
  @param    im          Image to compute from
  @param    xpos        the x position of the disk center [pixel, C indexing]
  @param    ypos        the y position of the disk center [pixel, C indexing]
  @param    rad         the radius [pixel]
  @return   a mask with valid pixels inside the disk
  @note     (xpos, ypos) and may be outside the image, if so then a sufficiently
            small rad will cause no pixels to be encircled
 */
/*----------------------------------------------------------------------------*/
static cpl_mask *
strehl_disk_mask(const cpl_image * im,
                 double            xpos,
                 double            ypos,
                 double            rad)
{
    const intptr_t       nx = cpl_image_get_size_x(im);
    const intptr_t       ny = cpl_image_get_size_y(im);
    /* Round down */
    const intptr_t       lx = (intptr_t)(xpos - rad);
    const intptr_t       ly = (intptr_t)(ypos - rad);
    /* Round up */
    const intptr_t       ux = (intptr_t)(xpos + rad) + 1;
    const intptr_t       uy = (intptr_t)(ypos + rad) + 1;

    const double    sqr = rad * rad;
    cpl_mask * m;


    /* Check entries */
    cpl_ensure(im != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(rad > 0.0, CPL_ERROR_ILLEGAL_INPUT, NULL);

    m = cpl_mask_new(nx, ny);

    for (intptr_t j = CX_MAX(ly, 0); j < CX_MIN(uy, ny); j++) {
        const double yj = (double)j - ypos;
        for (intptr_t i = CX_MAX(lx, 0); i < CX_MIN(ux, nx); i++) {
            const double xi   = (double)i - xpos;
            const double dist = yj * yj + xi * xi;
            if (dist <= sqr) {
                if (!cpl_image_is_rejected(im, i + 1, j + 1)) {
                    cpl_mask_set(m, i + 1, j + 1, CPL_BINARY_1);
                }
            }
        }
    }
    cpl_mask_not(m);

    return m;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief rebin image
 * @param img       image to rebin, size must be a multiple of sampling
 * @param sampling  rebin factor
 * @return rebinned image
 */
/* ---------------------------------------------------------------------------*/
static inline cpl_image * hdrl_rebin(cpl_image * img, size_t sampling)
{
    cpl_size lnx = cpl_image_get_size_x(img);
    cpl_size lny = cpl_image_get_size_y(img);
    cpl_size nx = lnx / sampling;
    cpl_size ny = lny / sampling;
    cpl_image * n = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
    double * ld = cpl_image_get_data_double(img);
    double * nd = cpl_image_get_data_double(n);
    for (size_t iy = 0; iy < (size_t)ny; iy++) {
        for (size_t ix = 0; ix < (size_t)nx; ix++) {
            for (size_t ly = 0; ly < sampling; ly++) {
                for (size_t lx = 0; lx < sampling; lx++) {
                    nd[iy * nx + ix] += ld[((iy*sampling) + ly) * lnx + lx + (ix*sampling)];
                }
            }
        }
    }
    return n;
}
#if 0
/* ---------------------------------------------------------------------------*/
/**
 * @brief compute encircled energy of psf
 * @param lam  wavelength
 * @param m1   primary mirror radius
 * @param m2   obstruction radius
 * @param r    integration radius
 *
 *
 * \f[
 * EE(v_{0}) = \frac{1}{1 - \epsilon^2} \left(1 - J_{0}^{2}(v_{0}) - J_{1}^{2}(v_{0})
 * + \epsilon^{2} (1- J_{0}^{2}(v_{0}) - J_{1}^{2}(\epsilon v_{0}))
 * -2\epsilon\int_{0}^{v_{0}} J_{1}(\epsilon v) \frac{2J_{1}(v)}{v}dv \right)
 * \f]
 *
 * where
 * \f$J_{1}\f$ is the Bessel function or order \f$0\f$,
 * \f$J_{1}\f$ is the Bessel function or order \f$1\f$,
 * \f$\epsilon = \frac{m_2}{m_2}\f$ is the telescope central obstruction
 * (fraction of telescope diameter),
 * v is the radial distance from Airy function center (2 * Nyquist/ units), and
 * \f$v_{0}\f$ is the maximum radial distance from Airy function center for EE
 * calculation (2 * Nyquist/ units). During \f$EE\f$ computation we neglect
 * the integral term.
 *
 */
/* ---------------------------------------------------------------------------*/
static inline double
compute_psf_flux(double lam, double m1, double m2, double r)
{
    /* currently unused */
    double e = m2 / m1;
    r *= CPL_MATH_2PI * m1 / lam;
    double E = 1 - j0(r) * j0(r) - j1(r) * j1(r) + e*e *
        (1 - j0(e*r)*j0(e*r) - j1(e*r)*j1(e*r));
    E *= 1 / (1 - e * e);
    /* Note compared to analytical formula iof the encircled energy, here we 
       neglect the contribute from integral term: -2*e*integral(j1*j1*2/v dv) */
    return E;
}
#endif

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief compute Strehl parameter
 * @param himg background corrected image
 * @param lam  wavelength [m]
 * @param m1_radius   primary mirror radius [m]
 * @param m2_radius   obstruction radius [m]
 * @param pixscale_x    image X pixel scale in [arcsec]
 * @param pixscale_y    image Y pixel scale in [arcsec]
 * @param flux_radius   radius used to sum the flux [arcsec]
 *
 * @return an hdrl_value containing the computed Strehl value and its error
 * or NAN in case of error.
 *
 * @note  strehl function, assumes background corrected image with peak at
 * peak_x/peak_y
 *
 * @note
 * This function assumes that the input image is background corrected.
 *
 * Initially a synthetic PSF is generated with parameters:
 *  - base_context.prefix.wavelength
 *  - base_context.prefix.m1
 *  - base_context.prefix.m2
 *  - base_context.prefix.pixel-scale-x
 *  - base_context.prefix.pixel-scale-y
 *  and pixel scale upsampling factor 16.
 * @see compute_psf()
 *
 * The synthetic PSF is shifted of an arbitrary value equal to 7 pixels
 * (determined ad hoc) and then the synthetic PSF is down-sampled to the
 * original PSF sampling. Then synthetic PSF is normalized dividing it by a
 * value equal to the ratio of the synthetic PSF peak and the observed PSF peak
 * values.
 *
 * Then the position and the peak of the PSF are determined by a robust fit of
 * a 2D Gaussian. @see gaussian_maxpos()
 *
 * Then an approximate position of the maximum on the observed PSF standard
 * is determined, later refined using the associated error (and bad pixel)
 * information. This define the value of PSFo_peak
 *
 * Then is determined the observed PSF flux, PSFo_flux, within the ring centered
 * on the peak and user set radius. This is used to compute the ratio
 * Ro=PSFo_peak/PSFo_flux
 *
 * The same ratio is also determined similarly for the synthetic PSF.
 * Rt=PSFt_peak/PSFt_flux
 *
 * Finally the strehl is computed as
 * strehl=Ro/Rt
 *
 * along with its associated error:
 * strehl_err=strehl*sqrt((peak_err/peak_val^2)+(flux_err/flux_val^2))
 *
 */

static hdrl_strehl_result
compute_strehl2(const hdrl_image * himg, double lam,
                double m1_radius, double m2_radius,
                double pixscale_x, double pixscale_y,
                double peak_x, double peak_y,
                double radius_arcsec)
{
    const cpl_image * img = hdrl_image_get_image_const(himg);
    double min_pscale = CX_MIN(pixscale_x, pixscale_y);
    double radius_pix = (radius_arcsec / min_pscale);
    /* could be shrunk for better performance as flux beyond a few rings is negligible
     * using analytic encircled energy would probably also work */
    cpl_size wins = 2 * radius_pix;
    cpl_msg_debug(cpl_func, "strehl psf window size %d", (int)wins);
    double smallx = peak_x - (floor(peak_x) - wins/2);
    double smally = peak_y - (floor(peak_y) - wins/2);
    /* sample psf on larger grid for a primitive integration of the flux */
    size_t sampling = 16;
    intptr_t nnx = wins * sampling;
    intptr_t nny = wins * sampling;
    cpl_image * lpsf = compute_psf(lam, m1_radius, m2_radius,
                                   pixscale_x / sampling, pixscale_y / sampling,
                                   smallx*sampling,smally*sampling,
                                   nnx, nny);
    /* Note: o is a hard-coded offset to get the peak in same position as in data 
     * after down-sampling, 7 seems to work reasonably well for sampling 16 */
    int o = 7;
    cpl_image * epsf = cpl_image_extract(lpsf, 1 + o, 1 + o, nnx - o, nny - o);
    cpl_image * psf = hdrl_rebin(epsf, sampling);
    cpl_image_delete(epsf);
    cpl_image_delete(lpsf);
    /* normalize for easier comparison, not required */
    cpl_image_divide_scalar(psf, cpl_image_get_max(psf)/cpl_image_get_max(img));
    cpl_msg_debug(cpl_func, "position/peak of data: %g %g", peak_x, peak_y);
    {
        /* fit not required, just for debugging */
        double xposfit, yposfit, peak;
        gaussian_maxpos(psf, 5, &xposfit, &yposfit, &peak);
        cpl_msg_debug(cpl_func, "position/peak of psf: %g %g", xposfit, yposfit);
    }
    
    /* computes ratio peak/flux over the observed PSF standard */
    cpl_mask  * im = strehl_disk_mask(img, peak_x, peak_y, radius_pix);
    hdrl_value ipeak = hdrl_image_max_where(himg, im);
    cpl_msg_debug(cpl_func, "Computing flux on %d pixel radius, total pixels %ld",
                  (int)(radius_pix),
                  (long)(cpl_mask_get_size_x(im) * cpl_mask_get_size_y(im) -
                         cpl_mask_count(im)));
    hdrl_value iflux = hdrl_image_sum_where(himg, im);
    cpl_msg_debug(cpl_func, "flux ring/total data: %g (%g) %g", iflux.data,
                  iflux.error, cpl_image_get_flux(img));
    cpl_mask_delete(im);
    
    double ratio_img = ipeak.data / iflux.data;

    /* computes ratio peak/flux over the synthetic PSF */
    double ppeak = cpl_image_get_max(psf);
    cpl_mask * pm =
        strehl_disk_mask(psf, wins / 2 - 1, wins / 2 - 1, radius_pix);
    hdrl_image * tmpimg = hdrl_image_create(psf, NULL);
    hdrl_value pflux = hdrl_image_sum_where(tmpimg, pm);
    hdrl_image_delete(tmpimg);
    cpl_msg_debug(cpl_func, "flux ring/total psf: %g %g", pflux.data,
                 cpl_image_get_flux(psf));
    cpl_mask_delete(pm);

    double ratio_psf = ppeak / pflux.data;

    cpl_msg_debug(cpl_func, "data peak,flux,ratio: %g %g: %g",
                  ipeak.data, iflux.data, ratio_img);
    cpl_msg_debug(cpl_func, "psf peak,flux,ratio:  %g %g: %g",
                  ppeak, pflux.data, ratio_psf);

    double strehl = ratio_img / ratio_psf;
    double strehl_err = strehl *
        sqrt(ipeak.error * ipeak.error / (ipeak.data * ipeak.data) +
             iflux.error * iflux.error / (iflux.data * iflux.data));

    cpl_msg_debug(cpl_func, "Strehl ratio %g +/- %g", strehl, strehl_err);
    cpl_image_delete(psf);

    hdrl_strehl_result r;
    r.strehl_value = (hdrl_value){strehl, strehl_err};
    r.star_peak = ipeak;
    r.star_flux = iflux;
    r.star_background = (hdrl_value){0., 0.};

    /* Initialize the remaining variables with dummy values to prevent warnings.
     * The derived values will be filled later outside this function */
    r.star_x = 0.;
    r.star_y = 0.;
    r.computed_background_error = -1.;
    r.nbackground_pixels = 0;
    /* TODO:
     * error on negative flux, error on disk max position != fit position */
    return r;
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief compute image Strehl
 * @param himg background corrected image
 * @param lam  wavelength [m]
 * @param m1_radius   primary mirror radius [m]
 * @param m2_radius   obstruction radius [m]
 * @param pixscale_x    image X pixel scale in [arcsec]
 * @param pixscale_y    image Y pixel scale in [arcsec]
 * @param flux_radius   radius used to sum the flux [arcsec]
 * @param bkg_radius_low_def  radius used to determine the background [arcsec]
 * @param bkg_radius_high_def radius used to determine the background [arcsec]
 *
 * @return an hdrl_strehl_result containing the computed Strehl value and its error
 * or NAN in case of error.
 *
 * @doc
 * This function assumes that the input image is pre-processed to remove
 * instrument signatures (flag bad pixels, etc.) and correct for the natural
 * noise sources (sky background, etc.). Bad pixels are interpolated.
 *
 * The routine initially determines the approximate position and flux value of
 * the PSF with a Gaussian fit.
 * @see gaussian_maxpos()
 *
 * If the user sets the parameters bkg_radius_low, bkg_radius_high to -1, the
 * routine assumes that the input image has zero background level.
 * Else if the user set the parameters bkg_radius_low, bkg_radius_high to proper
 * values the routine measure the background in the corresponding annular
 * region and subtract it. This second option is recommended.
 *
 * Then the strehl ratio and its associated error are computed.
 * @see compute_streh2()
 *
 */
/* ---------------------------------------------------------------------------*/
static hdrl_strehl_result
compute_strehl(const hdrl_image * himg_, double lam,
               double m1_radius, double m2_radius,
               double pixscale_x, double pixscale_y, double flux_radius,
               double bkg_radius_low, double bkg_radius_high)
{
    hdrl_image * himg = hdrl_image_duplicate(himg_);
    double xposfit, yposfit, peak;
    double min_pscale = CX_MIN(pixscale_x, pixscale_y);
    const cpl_image * img = hdrl_image_get_image_const(himg);
    /* interpolate bad pixels, */
    if (hdrl_image_count_rejected(himg) != 0) {
        cpl_msg_warning(cpl_func,
                        "%zu bad pixels in strehl input, interpolating.",
                        (size_t)hdrl_image_count_rejected(himg) );
        cpl_detector_interpolate_rejected(hdrl_image_get_image(himg));
        /* should have error propagation here .. */
        cpl_detector_interpolate_rejected(hdrl_image_get_error(himg));
    }

    if (gaussian_maxpos(img, 5, &xposfit, &yposfit, &peak) != CPL_ERROR_NONE) {
        goto error;
    }
    if (peak <= 0) {
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                              "detected peak of star smaller than zero, "
                              "gaussian fit likely failed to fit the star");
        goto error;
    }
    hdrl_value bkg = {0, 0};
    double comp_bkg_error = -1;
    size_t nbkg_pix = 0;
    if ((bkg_radius_low < 0 && bkg_radius_high >= 0) ||
        (bkg_radius_low >= 0 && bkg_radius_high < 0)) {
        cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT,
                              "background radius parameters must be larger "
                              "zero or both negative");
        goto error;
    }
    else if (bkg_radius_low >= 0 && bkg_radius_high >= 0) {
        if (bkg_radius_low >= bkg_radius_high) {
            cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT,
                          "low background radius parameters must be smaller "
                          "than large background radius");
            goto error;
        }
        /* compute mask of background ring */
        cpl_mask * high = strehl_disk_mask(img, xposfit, yposfit,
                                           bkg_radius_high / min_pscale);
        cpl_mask * low = strehl_disk_mask(img, xposfit, yposfit,
                                          bkg_radius_low / min_pscale);
        cpl_mask_xor(low, high);
        nbkg_pix = cpl_mask_count(low);
        if (nbkg_pix == 0) {
            cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                          "No valid pixels in background");
            cpl_mask_delete(low);
            cpl_mask_delete(high);
            goto error;
        }

        cpl_mask_not(low);
        bkg = hdrl_image_median_where(himg, low);
        comp_bkg_error = hdrl_image_stdev_where(himg, low) / sqrt(nbkg_pix);
        /* expected difference sqrt(pi / 2)  due to median */
        cpl_msg_debug(cpl_func, "Median estimated background: %g +- %g "
                      "(computed error %g)", bkg.data, bkg.error,
                      comp_bkg_error);
        cpl_mask_delete(low);
        cpl_mask_delete(high);

        hdrl_image_sub_scalar(himg, bkg);
    }

    hdrl_strehl_result r = compute_strehl2(himg, lam, m1_radius, m2_radius,
                                   pixscale_x, pixscale_y, xposfit, yposfit,
                                   flux_radius);
    hdrl_image_delete(himg);
    r.star_background = bkg;
    r.star_x = xposfit;
    r.star_y = yposfit;
    r.computed_background_error = comp_bkg_error;
    r.nbackground_pixels = nbkg_pix;
    return r;

error:
    hdrl_image_delete(himg);
    return bad_result;
}
/** @endcond */

/* TODO: missing doxygen, add more comments */
/* ---------------------------------------------------------------------------*/
/**
 * @brief This function computes the Strehl ratio.
 * @param himg input hdrl image
 * @param params input hdrl parameters
 *
 * @see hdrl_strehl_parameter_create()
 *
 * The raw image is assumed to be pre-processed to remove the instrument
 * signatures (bad pixels, etc.) and the natural noise sources (sky background,
 * etc.). Nethertheless this function allows also the user to correct a residual
 * background by setting the parameters \e bkg_radius_low, \e bkg_radius_high.
 * The PSF is identified and its integrated flux (controlled by the parameter
 * \e flux_radius) is normalized to 1. The
 * PSF baricenter is computed and used to generate the ideal PSF (with
 * integrated flux normalized to 1) which takes into account the telescope pupil
 * characteristics (radius \e m1, central obstruction, \e m2, ...), the
 * wavelength \e wavelength, at which the image has been obtained and the
 * related pixel scale (\e pixel_scale_x, \e pixel_scale_y,). Finally the Strehl
 * ratio is computed dividing the maximum intensity of the image PSF by the
 * maximum intensity of the ideal PSF and the associated error is also computed.
 *
 */
/* ---------------------------------------------------------------------------*/
hdrl_strehl_result
hdrl_strehl_compute(const hdrl_image * himg, hdrl_parameter* params)
{
    hdrl_strehl_result r;
    /* Check Entries */

    cpl_error_ensure(himg && params, CPL_ERROR_NULL_INPUT,
                     return bad_result,
                                     "NULL input");
    if (hdrl_strehl_parameter_verify(params) != CPL_ERROR_NONE) {
        return bad_result;
    }

    /* Local Usage Parameters */
    const hdrl_strehl_parameter * p_loc = (hdrl_strehl_parameter *)params ;

    r = compute_strehl(himg,p_loc->wavelength, p_loc->m1, p_loc->m2,
                    p_loc->pixel_scale_x, p_loc->pixel_scale_y,
                    p_loc->flux_radius, p_loc->bkg_radius_low,
                    p_loc->bkg_radius_high);
    return r;
}

/**@}*/

