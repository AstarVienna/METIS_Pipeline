/* $Id: irplib_strehl.c,v 1.43 2009-11-18 21:37:48 llundin Exp $
 *
 * This file is part of the irplib package
 * Copyright (C) 2002,2003 European Southern Observatory
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02111-1307  USA
 */

/*
 * $Author: llundin $
 * $Date: 2009-11-18 21:37:48 $
 * $Revision: 1.43 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "irplib_strehl.h"
#include "irplib_utils.h"

#include <assert.h>
#include <stdint.h>
#include <math.h>

/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_strehl   Functions to compute the Strehl
 */
/*----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                                   Define
 -----------------------------------------------------------------------------*/

#ifndef IRPLIB_STREHL_RAD_CENTRAL
#define IRPLIB_STREHL_RAD_CENTRAL 5
#endif

#ifndef IRPLIB_STREHL_DETECT_LEVEL
#define IRPLIB_STREHL_DETECT_LEVEL 5.0
#endif

#define IRPLIB_DISK_BG_MIN_PIX_NB    30
#define IRPLIB_DISK_BG_REJ_LOW       0.1
#define IRPLIB_DISK_BG_REJ_HIGH      0.1

#ifdef CPL_MIN
#define IRPLIB_MIN CPL_MIN
#else
#define IRPLIB_MIN(A,B) (((A) < (B)) ? (A) : (B))
#endif

#ifdef CPL_MAX
#define IRPLIB_MAX CPL_MAX
#else
#define IRPLIB_MAX(A,B) (((A) > (B)) ? (A) : (B))
#endif

/*-----------------------------------------------------------------------------
                                   Functions prototypes
 -----------------------------------------------------------------------------*/

static cpl_image * irplib_strehl_generate_otf(double, double, double, double,
                                              int, double);
static double PSF_H1(double, double, double);
static double PSF_H2(double, double);
static double PSF_G(double, double);
static double PSF_sinc_norm(double);
static double PSF_TelOTF(double, double);

#ifndef IRPLIB_NO_FIT_GAUSSIAN
#ifdef IRPLIB_STREHL_USE_CPL_IMAGE_FIT_GAUSSIAN
static double irplib_gaussian_2d(double, double, double, double, double);
#endif

#if defined CPL_VERSION_CODE && CPL_VERSION_CODE >= CPL_VERSION(6, 9, 1)
#define irplib_gaussian_eval_2d cpl_gaussian_eval_2d
#else
static double irplib_gaussian_eval_2d(const cpl_array *, double, double);
#endif

static uint32_t irplib_roundup_power2(uint32_t v) CPL_ATTR_CONST;

static
cpl_error_code irplib_gaussian_maxpos(const cpl_image *,
                                      double,
                                      double,
                                      double,
                                      double *,
                                      double *,
                                      double *);

static cpl_error_code
irplib_closeset_aperture(const cpl_apertures * self,
        const double x, const double y, int * ind);
#endif

/*-----------------------------------------------------------------------------
                                   Functions code
 -----------------------------------------------------------------------------*/
/**@{*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the strehl ratio in an image
  @param    im             Image with pixel-type float
  @param    m1             Diameter of the M1 mirror [m]
  @param    m2             Diameter of the M2 mirror [m]
  @param    lam            Central wavelength [micron]
  @param    dlam           Filter bandwidth [micron]
  @param    pscale         Positive pixel scale [Arcsecond/pixel]
  @param    size           Size of image to be used for internal PSF,
                           must be a power of two. [pixel]
  @param    xpos           The x position of the ring center [pixel]
  @param    ypos           The y position of the ring center [pixel]
  @param    r1             The Star Radius, r1 > 0 [Arcsecond]
  @param    r2             The Internal Radius, r2 > 0 [Arcsecond]
  @param    r3             The External Radius, r3 > r2 [Arcsecond]
  @param    noise_box_sz   Pass -1 for default values
  @param    noise_nsamples Pass -1 for default values
  @param    strehl         Pointer to the Strehl Error (positive on success)
  @param    strehl_err     Pointer to the Strehl Error (non-negative on success)
  @param    star_bg        Pointer to the Star Background
  @param    star_peak      Pointer to the Star Peak (positive on success)
  @param    star_flux      Pointer to the Star Flux (positive on success)
  @param    psf_peak       Pointer to the PSF Peak (positive on success)
  @param    psf_flux       Pointer to the PSF Flux (1 on success)
  @param    bg_noise       Pointer to the Background Noise
  @return   CPL_ERROR_NONE or the relevant CPL error code on error
  @note     The output is undefined on error. On success the Strehl Ratio may
            exceed 1. Any pixel flagged as bad is ignored.
 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_strehl_compute(const cpl_image *   im,
                                     double              m1,
                                     double              m2,
                                     double              lam,
                                     double              dlam,
                                     double              pscale,
                                     int                 size,
                                     double              xpos,
                                     double              ypos,
                                     double              r1,
                                     double              r2,
                                     double              r3,
                                     int                 noise_box_sz,
                                     int                 noise_nsamples,
                                     double          *   strehl,
                                     double          *   strehl_err,
                                     double          *   star_bg,
                                     double          *   star_peak,
                                     double          *   star_flux,
                                     double          *   psf_peak,
                                     double          *   psf_flux,
                                     double          *   bg_noise)
{
    cpl_image  * psf;
    double       star_radius, max_radius;

    /* FIXME: Arbitrary choice of image border */
    const double window_size = (double)(IRPLIB_STREHL_RAD_CENTRAL);

    /* Determined empirically by C. Lidman for Strehl error computation */
    const double strehl_error_coefficient = CPL_MATH_PI * 0.007 / 0.0271;
    double       ring[4];
    /* cpl_flux_get_noise_ring() must succeed with this many tries */
    int          ring_tries = 3;
#ifndef IRPLIB_NO_FIT_GAUSSIAN
    double xposfit = 0.0, yposfit = 0.0, peak = 0.0;
    cpl_error_code code;
#endif
    cpl_errorstate prestate = cpl_errorstate_get();

    /* Check compile-time constant */
    cpl_ensure_code(window_size > 0.0,  CPL_ERROR_ILLEGAL_INPUT);

    /* Test inputs */
    cpl_ensure_code(im != NULL,         CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(strehl != NULL,     CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(strehl_err != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(star_bg != NULL,    CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(star_peak != NULL,  CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(star_flux != NULL,  CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(psf_peak != NULL,   CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(psf_flux != NULL,   CPL_ERROR_NULL_INPUT);

    cpl_ensure_code(pscale > 0.0,      CPL_ERROR_ILLEGAL_INPUT);

    cpl_ensure_code(r1 > 0.0,      CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(r2 > 0.0,      CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(r3 > r2,       CPL_ERROR_ILLEGAL_INPUT);

    /* Computing a Strehl ratio is a story between an ideal PSF */
    /* and a candidate image supposed to approximate this ideal PSF. */

    /* Generate first appropriate PSF to find max peak */
    psf = irplib_strehl_generate_psf(m1, m2, lam, dlam, pscale, size);
    if (psf == NULL) {
        return cpl_error_set_where(cpl_func);
    }

    /* Compute flux in PSF and find max peak */
    *psf_peak = cpl_image_get_max(psf);
    cpl_image_delete(psf);

    assert( *psf_peak > 0.0); /* The ideal PSF has a positive maximum */
    *psf_flux = 1.0; /* The psf flux, cpl_image_get_flux(psf), is always 1 */

#ifndef IRPLIB_NO_FIT_GAUSSIAN
    code = irplib_gaussian_maxpos(im, IRPLIB_STREHL_DETECT_LEVEL, xpos, ypos,
                                  &xposfit, &yposfit, &peak);
    if (code) {
        cpl_errorstate_set(prestate);
    } else {
        xpos = xposfit;
        ypos = yposfit;
    }
#endif

    /* Measure the background in the candidate image */
    *star_bg = irplib_strehl_ring_background(im, xpos, ypos,
                                             r2/pscale, r3/pscale,
                                             IRPLIB_BG_METHOD_AVER_REJ);
    if (!cpl_errorstate_is_equal(prestate)) {
        return cpl_error_set_where(cpl_func);
    }

    /* Compute star_radius in pixels */
    star_radius = r1/pscale;

    /* Measure the flux on the candidate image */
    *star_flux = irplib_strehl_disk_flux(im, xpos, ypos, star_radius, *star_bg);

    if (*star_flux <= 0.0) {
        return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_OUTPUT,
                                     "Non-positive star flux=%g (Star "
                                     "background=%g)", *star_flux, *star_bg);
    }

    /* Find the peak value on the central part of the candidate image */
    max_radius = window_size < star_radius ? window_size : star_radius;
    cpl_ensure_code(!irplib_strehl_disk_max(im, xpos, ypos, max_radius,
                                            star_peak), cpl_error_get_code());
    *star_peak -= *star_bg;

    if (*star_flux <= 0.0) {
        return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_OUTPUT,
                                     "Non-positive star peak=%g (Star "
                                     "background=%g, Star flux=%g)",
                                     *star_flux, *star_bg, *star_flux);
    }

    /* Compute Strehl */
    /* (StarPeak / StarFlux) / (PsfPeak / PsfFlux) */
    *strehl = (*star_peak * *psf_flux ) / ( *star_flux * *psf_peak);

#ifndef IRPLIB_NO_FIT_GAUSSIAN
    if (code == CPL_ERROR_NONE && peak > *star_peak && *star_peak > 0.0 &&
        *strehl * peak / *star_peak <= 1.0) {
        cpl_msg_debug(cpl_func, "Increasing Strehl from %g: %g (%g)",
                      *strehl, *strehl * peak / *star_peak,
                      peak / *star_peak);
        *strehl *= peak / *star_peak;
        *star_peak = peak;
    }
#endif

    /* Compute Strehl error */
    ring[0] = xpos;
    ring[1] = ypos;
    ring[2] = r2/pscale;
    ring[3] = r3/pscale;

    while (cpl_flux_get_noise_ring(im, ring, noise_box_sz, noise_nsamples,
                                   bg_noise, NULL) && --ring_tries > 0);
    if (ring_tries > 0) {
        cpl_errorstate_set(prestate); /* Recover, if an error happened */
    } else {
        return cpl_error_set_where(cpl_func);
    }

    *strehl_err = strehl_error_coefficient * (*bg_noise) * pscale *
        star_radius * star_radius / *star_flux;

    if (*strehl > 1.0) {
        cpl_msg_warning(cpl_func, "Extreme Strehl-ratio=%g (strehl-error=%g, "
                        "star_peak=%g, star_flux=%g, psf_peak=%g, psf_flux=%g)",
                        *strehl, *strehl_err, *star_peak, *star_flux, *psf_peak,
                        *psf_flux);
    }

    /* This check should not be able to fail, but just to be sure */
    return *strehl_err >= 0.0
        ? CPL_ERROR_NONE
        : cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_OUTPUT,
                                 "Negative strehl-error=%g (Strehl-ratio=%g, "
                                 "star_peak=%g, star_flux=%g, psf_peak=%g, "
                                 "psf_flux=%g", *strehl_err, *strehl,
                                 *star_peak, *star_flux, *psf_peak, *psf_flux);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the flux from a disk
  @param    im          Image to compute from
  @param    xpos        the x position of the disk center
  @param    ypos        the y position of the disk center
  @param    rad         the radius
  @param    bg          the star background
  @return   The computed flux or 0.0 on error
  @note     (xpos, ypos) and may be outside the image, if so then a sufficiently
            small rad will cause no pixels to be encircled, in which case the
            returned flux is zero.
 */
/*----------------------------------------------------------------------------*/
double irplib_strehl_disk_flux(const cpl_image * im,
                               double            xpos,
                               double            ypos,
                               double            rad,
                               double            bg)
{
    const int       nx = cpl_image_get_size_x(im);
    const int       ny = cpl_image_get_size_y(im);
    /* Round down */
    const int       lx = (int)(xpos - rad);
    const int       ly = (int)(ypos - rad);
    /* Round up */
    const int       ux = (int)(xpos + rad) + 1;
    const int       uy = (int)(ypos + rad) + 1;

    const double    sqr = rad * rad;
    double          flux = 0.0;
    int             i, j;


    /* Check entries */
    cpl_ensure(im != NULL, CPL_ERROR_NULL_INPUT, 0.0);
    cpl_ensure(rad > 0.0, CPL_ERROR_ILLEGAL_INPUT, 0.0);

    for (j = IRPLIB_MAX(ly, 0); j < IRPLIB_MIN(uy, ny); j++) {
        const double yj = (double)j - ypos;
        for (i = IRPLIB_MAX(lx, 0); i < IRPLIB_MIN(ux, nx); i++) {
            const double xi   = (double)i - xpos;
            const double dist = yj * yj + xi * xi;
            if (dist <= sqr) {
                int isbad;
                const double value = cpl_image_get(im, i+1, j+1, &isbad);

                if (!isbad ) {

                    flux += value - bg;

                }
            }
        }
    }

    return flux;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the background in the image from a specified disk
  @param    im          Image to compute from
  @param    xpos        the x position of the ring center
  @param    ypos        the y position of the ring center
  @param    rad_int     the internal radius
  @param    rad_ext     the external radius
  @param    mode        IRPLIB_BG_METHOD_AVER_REJ or IRPLIB_BG_METHOD_MEDIAN
  @return   The computed background or 0.0 on error.

 */
/*----------------------------------------------------------------------------*/
double irplib_strehl_ring_background(const cpl_image *       im,
                                     double                  xpos,
                                     double                  ypos,
                                     double                  rad_int,
                                     double                  rad_ext,
                                     irplib_strehl_bg_method mode)
{
    const int       nx = cpl_image_get_size_x(im);
    const int       ny = cpl_image_get_size_y(im);
    /* Round down */
    const int       lx = (int)(xpos - rad_ext);
    const int       ly = (int)(ypos - rad_ext);
    /* Round up */
    const int       ux = (int)(xpos + rad_ext) + 1;
    const int       uy = (int)(ypos + rad_ext) + 1;
    int             mpix, npix;
    const double    sqr_int = rad_int * rad_int;
    const double    sqr_ext = rad_ext * rad_ext;
    cpl_vector  *   pix_arr;
    double          flux = 0.0;
    int             i, j;

    /* Check entries */
    cpl_ensure(im != NULL, CPL_ERROR_NULL_INPUT, 0.0);
    cpl_ensure(rad_int > 0.0, CPL_ERROR_ILLEGAL_INPUT, 0.0);
    cpl_ensure(rad_ext > rad_int, CPL_ERROR_ILLEGAL_INPUT, 0.0);

    cpl_ensure(mode == IRPLIB_BG_METHOD_AVER_REJ ||
               mode == IRPLIB_BG_METHOD_MEDIAN,
               CPL_ERROR_UNSUPPORTED_MODE, 0.0);

    mpix = (int)((2.0 * rad_ext + 1.0) * (2.0 * rad_ext + 1.0));

    /* Allocate pixel array to hold values in the ring */
    pix_arr = cpl_vector_new(mpix);

    /* Count number of pixels in the ring */
    /* Retrieve all pixels which belong to the ring */
    npix = 0;
    for (j = IRPLIB_MAX(ly, 0); j < IRPLIB_MIN(uy, ny); j++) {
        const double yj = (double)j - ypos;
        for (i = IRPLIB_MAX(lx, 0); i < IRPLIB_MIN(ux, nx); i++) {
            const double xi   = (double)i - xpos;
            const double dist = yj * yj + xi * xi;
            if (sqr_int <= dist && dist <= sqr_ext) {
                int isbad;
                const double value = cpl_image_get(im, i+1, j+1, &isbad);

                if (!isbad) {
                    cpl_vector_set(pix_arr, npix, value);
                    npix++;
                }
            }
        }
    }

    assert(npix <= mpix);

    if (npix < IRPLIB_DISK_BG_MIN_PIX_NB) {
        cpl_vector_delete(pix_arr);
        (void)cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND, "Need "
                                    "at least %d (not %d <= %d) samples to "
                                    "compute noise", IRPLIB_DISK_BG_MIN_PIX_NB,
                                    npix, mpix);
        return 0.0;
    }

    /* Should not be able to fail now */

    /* Resize pixel array to actual number of values within the ring */
    pix_arr = cpl_vector_wrap(npix, (double*)cpl_vector_unwrap(pix_arr));

    if (mode == IRPLIB_BG_METHOD_AVER_REJ) {
        const int low_ind  = (int)((double)npix * IRPLIB_DISK_BG_REJ_LOW);
        const int high_ind = (int)((double)npix
                                   * (1.0 - IRPLIB_DISK_BG_REJ_HIGH));

        /* Sort the array */
        cpl_vector_sort(pix_arr, CPL_SORT_ASCENDING);

        for (i=low_ind; i<high_ind; i++) {
            flux += cpl_vector_get(pix_arr, i);
        }
        if (high_ind - low_ind > 1) flux /= (double)(high_ind - low_ind);
    } else /* if (mode == IRPLIB_BG_METHOD_MEDIAN) */ {
        flux = cpl_vector_get_median(pix_arr);
    }

    cpl_vector_delete(pix_arr);

    return flux;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the ideal PSF for a given telescope+instrument.
  @param    m1          Diameter of the M1 mirror [meter]
  @param    m2          Diameter of the M2 mirror [meter]
  @param    lam         Central wavelength [micron]
  @param    dlam        Filter bandwidth [micron]
  @param    size        Generated image size (image will be square) [pixel]
  @param    pscale      Pixel scale on the sky [Arcsecond/pixel]
  @return   1 newly generated image.

  This function computes the ideal PSF for a given telescope and instrument.
  The PSF is computed by first generated the ideal OTF for the provided
  conditions, and applying a Fourier transform to it to bring it back to real
  space. The returned PSF is normalized to unity flux, to help Strehl ratio
  computations.

  The image halves of the returned PSF are swapped in both directions.

 */
/*----------------------------------------------------------------------------*/
cpl_image * irplib_strehl_generate_psf(double   m1,
                                       double   m2,
                                       double   lam,
                                       double   dlam,
                                       double   pscale,
                                       int      size)
{
    cpl_image * otf_image = irplib_strehl_generate_otf(m1, m2, lam, dlam,
                                                       size, pscale);

    if (otf_image == NULL || 

        /* Transform back to real space
           - Normalization is unnecessary, due to the subsequent normalisation.
           - An OTF is point symmetric about its center, i.e. it is even,
           i.e. the real space image is real.
           - Because of this a forward FFT works as well.
           - If the PSF ever needs to have its images halves swapped add
           CPL_FFT_SWAP_HALVES to the FFT call.
        */

        cpl_image_fft(otf_image, NULL, CPL_FFT_UNNORMALIZED) ||

        /* Compute absolute values of PSF */
        cpl_image_abs(otf_image) ||

        /* Normalize PSF to get flux=1  */
        cpl_image_normalise(otf_image, CPL_NORM_FLUX)) {

        (void)cpl_error_set_where(cpl_func);
        cpl_image_delete(otf_image);
        otf_image = NULL;
    }

    return otf_image;
}

/**@}*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Generate an image of an ideal Optical Transfer Function.
  @param    m1          Diameter of the M1 mirror [m]
  @param    m2          Diameter of the M2 mirror [m], m2 < m1
  @param    lam         Central wavelength [micron]
  @param    dlam        Filter bandwidth [micron]
  @param    size        The generated (square) image will be size X size pixels
  @param    pscale      Pixel scale on the sky [Arcsecond/pixel]
  @return   1 newly generated cpl_image

  Based on the paper "Amplitude estimation from speckle interferometry" by
  Christian Perrier in "Diffraction-limited imaging with very large telescopes",
  NATO ASI Series C, Vol. 274, edited by D. Alloin and J.-M. Mariotti, 1989
  (p. 99).
 */
/*----------------------------------------------------------------------------*/
static cpl_image * irplib_strehl_generate_otf(double  m1,
                                              double  m2,
                                              double  lam,
                                              double  dlam,
                                              int     size,
                                              double  pscale)
{
    double     * otf_data;
    /* Obscuration ratio, m1 / m2 */
    const double obs_ratio = m1 != 0.0 ? m2 / m1 : 0.0;
    /* pixel scale converted from Arsecond to radian  */
    const double rpscale = pscale * CPL_MATH_2PI / (double)(360 * 60 * 60);
    /* Cut-off frequency in pixels per central wavelength (in m) */
    const double f_max = m1 * rpscale * (double)size;

    /* Pixel corresponding to the zero frequency */
    const int    pix0 = size / 2;
    int i, j;


    cpl_ensure(m2       > 0.0, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(m1       > m2,  CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(dlam     > 0.0, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(pscale   > 0.0, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(size     > 0,   CPL_ERROR_ILLEGAL_INPUT, NULL);
    /* Due the the FFT, size is actually required to be a power of two */
    cpl_ensure(size % 2 == 0,  CPL_ERROR_ILLEGAL_INPUT, NULL);

    /* Ensure positive lambda */
    cpl_ensure(2.0 * lam > dlam, CPL_ERROR_ILLEGAL_INPUT, NULL);

    /* Convert wavelengths from micron to meter */
    lam /= 1.0e6;
    dlam /= 1.0e6;

    /* Allocate the output pixel buffer */
    otf_data = (double*)cpl_malloc(size * size * sizeof(*otf_data));

    /* Convolution with the detector pixels */
    /* The OTF is point symmetric so the whole image can be computed from the
       values of a single octant. */
    /* The image could be created with calloc() and j limited by
       f_max / (mlam - mdlam * 0.5) but this is not faster */
    for (j = 0; j <= pix0; j++) {
        double sinc_y_9 = 0.0; /* Avoid uninit warning */
        for (i = 0; i <= j; i++) {
            if (i == 0 && j == 0) {
                otf_data[size * pix0 + pix0] = 1.0;
            } else {
                const double x = (double)i;
                const double y = (double)j;
                const double sqdist = x * x + y * y;
                double f_lambda = 0.0;
                double sinc_xy_9 = 0.0; /* Zero if OTF is zero */
                double otfxy = 0.0;
                int k;

                assert( j > 0 );

                /* 9 iterations on the wavelength  */
                /* Unrolling the loop is not faster (due to the break?) */
                for (k = 4; k >= -4; k--) {
                    /* Compute intermediate cut-off frequency   */
                    const double lambda = lam - dlam * (double)k / 8.0;

                    /* A decreasing k ensures that we either enter on the first
                       iteration or not at all */
                    if (sqdist * lambda * lambda >= f_max * f_max) break;

                    if (k == 4) {
                        f_lambda = sqrt(sqdist) / f_max;
                        if (i == 0) {
                            /* Sinc(x = 0) == 1 */
                            sinc_xy_9 = sinc_y_9 =
                                PSF_sinc_norm(y / (double)size) / 9.0;
                        } else {
                            sinc_xy_9 = sinc_y_9 *
                                PSF_sinc_norm(x / (double)size);
                        }
                    }

                    otfxy += PSF_TelOTF(f_lambda * lambda, obs_ratio);
                }
                otfxy *= sinc_xy_9;

                /* When i == j the same value is written to the same
                   position twice. That's probably faster than a guard */
                otf_data[size * (pix0 - j) + pix0 - i] = otfxy;
                otf_data[size * (pix0 - i) + pix0 - j] = otfxy;
                if (i < pix0) {
                    otf_data[size * (pix0 - j) + pix0 + i] = otfxy;
                    otf_data[size * (pix0 + i) + pix0 - j] = otfxy;
                    if (j < pix0) {
                        otf_data[size * (pix0 + j) + pix0 - i] = otfxy;
                        otf_data[size * (pix0 - i) + pix0 + j] = otfxy;
                        otf_data[size * (pix0 + j) + pix0 + i] = otfxy;
                        otf_data[size * (pix0 + i) + pix0 + j] = otfxy;
                    }
                }
            }
        }
    }

    return cpl_image_wrap_double(size, size, otf_data);
}

/*----------------------------------------------------------------------------*
 * H1 function
 *----------------------------------------------------------------------------*/
static double PSF_H1(
        double  f,
        double  u,
        double  v)
{
    const double e = fabs(1.0-v) > 0.0 ? -1.0 : 1.0; /* e = 1.0 iff v = 1.0 */

    return((v*v/CPL_MATH_PI)*acos((f/v)*(1.0+e*(1.0-u*u)/(4.0*f*f))));
}

/*----------------------------------------------------------------------------*
 * H2 function
 *----------------------------------------------------------------------------*/
static double PSF_H2(double  f,
                     double  u)
{
    const double tmp1 = (2.0 * f) / (1.0 + u);
    const double tmp2 = (1.0 - u) / (2.0 * f);

    return -1.0 * (f/CPL_MATH_PI) * (1.0+u)
        * sqrt((1.0-tmp1*tmp1)*(1.0-tmp2*tmp2));
}

/*----------------------------------------------------------------------------*
 * G function
 *----------------------------------------------------------------------------*/
static double PSF_G(double  f,
                    double  u)
{
    if (f <= (1.0-u)/2.0) return(u*u);
    if (f >= (1.0+u)/2.0) return(0.0);
    else return(PSF_H1(f,u,1.0) + PSF_H1(f,u,u) + PSF_H2(f,u));
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    The normalized sinc function
  @param    x  The non-zero argument
  @return   sin(x * pi)/ (x * pi)

 */
/*----------------------------------------------------------------------------*/
static double PSF_sinc_norm(double x)
{
    /* This static function should not be called with zero, but handle it
       anyway. */
    return x != 0.0 ? sin(x * CPL_MATH_PI) / (x * CPL_MATH_PI) : 1.0;
}

/*----------------------------------------------------------------------------*
 * Telescope OTF function
 *----------------------------------------------------------------------------*/
static double PSF_TelOTF(double  f,
                         double  u)
{
    return((PSF_G(f,1.0)+u*u*PSF_G(f/u,1.0)-2.0*PSF_G(f,u))/(1.0-u*u));
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Find the peak flux inside a disk
  @param    self        Image with pixel-type float
  @param    xpos        The x position of the disk center
  @param    ypos        The y position of the disk center
  @param    radius      The positive radius
  @param    ppeak       On success, *ppeak is the peak intensity
  @return   CPL_ERROR_NONE or the relevant CPL error code on error

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_strehl_disk_max(const cpl_image * self,
                                      double            xpos,
                                      double            ypos,
                                      double            radius,
                                      double          * ppeak)
{

    const int       nx = cpl_image_get_size_x(self);
    const int       ny = cpl_image_get_size_y(self);
    /* Round down */
    const int       lx = (int)(xpos - radius);
    const int       ly = (int)(ypos - radius);
    /* Round up */
    const int       ux = (int)(xpos + radius) + 1;
    const int       uy = (int)(ypos + radius) + 1;

    const double    sqr = radius * radius;
    cpl_boolean     first = CPL_TRUE;
    int             i, j;


    /* Check entries */
    cpl_ensure_code(self  != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(ppeak != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(radius > 0.0,  CPL_ERROR_ILLEGAL_INPUT);


    for (j = IRPLIB_MAX(ly, 0); j < IRPLIB_MIN(uy, ny); j++) {
        const double yj = (double)j - ypos;
        for (i = IRPLIB_MAX(lx, 0); i < IRPLIB_MIN(ux, nx); i++) {
            const double xi   = (double)i - xpos;
            const double dist = yj * yj + xi * xi;
            if (dist <= sqr) {
                int isbad;
                const double value = cpl_image_get(self, i+1, j+1, &isbad);

                if (!isbad &&
                    (first || value > *ppeak)) {
                    first = CPL_FALSE;
                    *ppeak = value;
                }
            }
        }
    }

    return first
        ? cpl_error_set(cpl_func, CPL_ERROR_DATA_NOT_FOUND)
        : CPL_ERROR_NONE;
}

#ifndef IRPLIB_NO_FIT_GAUSSIAN
#ifdef IRPLIB_STREHL_USE_CPL_IMAGE_FIT_GAUSSIAN
/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Compute the value of a Gaussian function at a given point.
  @param    x       x coordinate where to compute the function.
  @param    y       y coordinate where to compute the function.
  @param    norm    The norm of the gaussian.
  @param    sig_x   Sigma in x for the Gauss distribution.
  @param    sig_y   Sigma in y for the Gauss distribution.
  @return   the gaussian value

  Compute the value of a 2d Gaussian function at a given point:

  f(x, y) = (norm/(2*pi*sig_x*sig_y)) * exp(-x^2/(2*sig_x^2)) * 
            exp(-y^2/(2*sig_y^2))
 */
/*----------------------------------------------------------------------------*/
static double irplib_gaussian_2d(double  x,
                                 double  y,
                                 double  norm,
                                 double  sig_x,
                                 double  sig_y)
{

    /* Copied from CPL */
    return norm / (sig_x * sig_y * CPL_MATH_2PI *
                   exp(x * x / (2.0 * sig_x * sig_x) +
                       y * y / (2.0 * sig_y * sig_y)));
}
#endif

#if defined CPL_VERSION_CODE && CPL_VERSION_CODE >= CPL_VERSION(6, 9, 1)
#else
/*----------------------------------------------------------------------------*/
/**
  @brief    Evaluate the Gaussian in a 2D-point
  @param    self  The seven Gaussian parameters
  @param    x     The X-coordinate to evaluate
  @param    y     The Y-coordinate to evaluate
  @return   The gaussian value or zero on error
  @see cpl_fit_image_gaussian()
  @note The function should not be able to fail if the parameters come from
        a succesful call to cpl_fit_image_gaussian()

   Possible #_cpl_error_code_ set in this function:
   - CPL_ERROR_NULL_INPUT if a pointer is NULL.
   - CPL_ERROR_TYPE_MISMATCH if the array is not of type double
   - CPL_ERROR_ILLEGAL_INPUT if the array has a length different from 7
   - CPL_ERROR_ILLEGAL_OUTPUT if the (absolute value of the) radius exceeds 1
   - CPL_ERROR_DIVISION_BY_ZERO if a sigma is 0, or the radius is 1

 */
/*----------------------------------------------------------------------------*/
static
double irplib_gaussian_eval_2d(const cpl_array * self, double x, double y)
{
    cpl_errorstate prestate = cpl_errorstate_get();
    const double B    = cpl_array_get_double(self, 0, NULL);
    const double A    = cpl_array_get_double(self, 1, NULL);
    const double R    = cpl_array_get_double(self, 2, NULL);
    const double M_x  = cpl_array_get_double(self, 3, NULL);
    const double M_y  = cpl_array_get_double(self, 4, NULL);
    const double S_x  = cpl_array_get_double(self, 5, NULL);
    const double S_y  = cpl_array_get_double(self, 6, NULL);

    double value = 0.0;

    if (!cpl_errorstate_is_equal(prestate)) {
        (void)cpl_error_set_where(cpl_func);
    } else if (cpl_array_get_size(self) != 7) {
        (void)cpl_error_set(cpl_func, CPL_ERROR_ILLEGAL_INPUT);
    } else if (fabs(R) < 1.0 && S_x != 0.0 && S_y != 0.0) {
        const double x_n  = (x - M_x) / S_x;
        const double y_n  = (y - M_y) / S_y;

        value = B + A / (CPL_MATH_2PI * S_x * S_y * sqrt(1 - R * R)) *
            exp(-0.5 / (1 - R * R) * ( x_n * x_n + y_n * y_n
                                       - 2.0 * R * x_n * y_n));
    } else if (fabs(R) > 1.0) {
        (void)cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_OUTPUT,
                                     "fabs(R=%g) > 1", R);
    } else {
        (void)cpl_error_set_message(cpl_func, CPL_ERROR_DIVISION_BY_ZERO,
                                     "R=%g. Sigma=(%g, %g)", R, S_x, S_y);
    }

    return value;
}
#endif

/*----------------------------------------------------------------------------*/
/**
  @brief    Increase a non-zero, unsigned 32-bit integer to the next power of 2
  @param    v  The non-zero number to increase
  @return   The power of two
  @see http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
 */
/*----------------------------------------------------------------------------*/
static uint32_t irplib_roundup_power2(uint32_t v)
{
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;

    return v + 1;
}


static cpl_error_code
irplib_closeset_aperture(const cpl_apertures * self,
        const double x, const double y, int * ind){

    double dist = INFINITY;

    const int nsize = cpl_apertures_get_size(self);

    cpl_ensure_code(nsize > 0,    cpl_error_get_code());
    cpl_ensure_code(ind,          CPL_ERROR_NULL_INPUT);

    *ind = -1;

    for(int i = 1; i <= nsize; ++i){
        const double this_x = cpl_apertures_get_centroid_x(self, i);
        const double this_y = cpl_apertures_get_centroid_y(self, i);
        const double this_d_sq = pow(this_x - x, 2.0) + pow(this_y - y, 2.0);

        if(this_d_sq < dist){
            dist = this_d_sq;
            *ind = i;
        }

    }
    return CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Find the peak flux, peak sum and position of a Gaussian
  @param    self        Image to process
  @param    sigma       The initial detection level
  @param    pxpos       On success, the refined X-position
  @param    pypos       On success, the refined Y-position
  @param    ppeak       On success, the refined peak flux
  @return CPL_ERROR_NONE or the relevant CPL error code on error

 */
/*----------------------------------------------------------------------------*/
static
cpl_error_code irplib_gaussian_maxpos(const cpl_image * self,
                                      double    sigma,
                                      double    x_initial,
                                      double    y_initial,
                                      double  * pxpos,
                                      double  * pypos,
                                      double  * ppeak)
{

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
#ifdef IRPLIB_STREHL_USE_CPL_IMAGE_FIT_GAUSSIAN
    double          norm, xcen, ycen, sig_x, sig_y, fwhm_x, fwhm_y;
#endif
    cpl_array     * gauss_parameters = NULL;
    cpl_errorstate  prestate = cpl_errorstate_get();
    cpl_error_code  code = CPL_ERROR_NONE;


    cpl_ensure_code( sigma > 0.0, CPL_ERROR_ILLEGAL_INPUT);

    selection = cpl_mask_new(nx, ny);

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
    sigma *= 2.0; /* FIXME: unelegant */

    cpl_mask_delete(selection);

    if (code) {
        cpl_image_delete(labels);
        return cpl_error_set_where(cpl_func);
    } else if (nlabels == 0) {
        cpl_image_delete(labels);
        return cpl_error_set(cpl_func, CPL_ERROR_DATA_NOT_FOUND);
    }

    aperts = cpl_apertures_new_from_image(self, labels);

    /* Find the aperture closest to the provided coordinates */
    code = irplib_closeset_aperture(aperts, x_initial, y_initial, &ifluxapert);

    if (code) {
        cpl_apertures_delete(aperts);
        cpl_image_delete(labels);
        return cpl_error_set(cpl_func, CPL_ERROR_DATA_NOT_FOUND);
    }

    npixobj = cpl_apertures_get_npix(aperts, ifluxapert);
    objradius = sqrt((double)npixobj * CPL_MATH_1_PI);
    /* Size is power of two for future noise filtering w. fft */
    winsize = IRPLIB_MIN(IRPLIB_MIN(nx, ny), irplib_roundup_power2
                         ((uint32_t)(3.0 * objradius + 0.5)));

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

    gauss_parameters = cpl_array_new(7, CPL_TYPE_DOUBLE);
    cpl_array_set_double(gauss_parameters, 0, median);

    code = cpl_fit_image_gaussian(self, NULL, xposcen, yposcen,
                                  winsize, winsize, gauss_parameters,
                                  NULL, NULL, NULL,
                                  NULL, NULL, NULL, 
                                  NULL, NULL, NULL);
    if (!code) {
        const double M_x = cpl_array_get_double(gauss_parameters, 3, NULL);
        const double M_y = cpl_array_get_double(gauss_parameters, 4, NULL);

        valfit = irplib_gaussian_eval_2d(gauss_parameters, M_x, M_y);

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

#ifdef IRPLIB_STREHL_USE_CPL_IMAGE_FIT_GAUSSIAN
    if (code || valfit < valmax) {
        cpl_errorstate_set(prestate);

        code = cpl_image_fit_gaussian(self, xposcen, yposcen,
                                      (int)(2.0 * objradius),
                                      &norm,
                                      &xcen,
                                      &ycen,
                                      &sig_x,
                                      &sig_y,
                                      &fwhm_x,
                                      &fwhm_y);

        if (!code) {
            valfit = irplib_gaussian_2d(0.0, 0.0, norm, sig_x, sig_y);

            cpl_msg_debug(cpl_func, "Gauss-Fit @ (%g, %g) = %g. norm=%g, "
                          "sigma=(%g, %g)", xcen, ycen, valfit, norm,
                          sig_x, sig_y);

            if (valfit > valmax) {
                *pxpos   = xcen;
                *pypos   = ycen;
                *ppeak   = valfit;
            }
        }
    }
#endif

    if (code || valfit < valmax) {
        cpl_errorstate_set(prestate);
        *pxpos   = xposcen;
        *pypos   = yposcen;
        *ppeak   = valmax;
    }

    return code ? cpl_error_set_where(cpl_func) : CPL_ERROR_NONE;
}
#endif
