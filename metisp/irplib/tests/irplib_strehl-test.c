/* $Id: irplib_strehl-test.c,v 1.10 2013-01-29 08:43:33 jtaylor Exp $
 *
 * This file is part of the ESO Common Pipeline Library
 * Copyright (C) 2001-2008 European Southern Observatory
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
 * $Author: jtaylor $
 * $Date: 2013-01-29 08:43:33 $
 * $Revision: 1.10 $
 * $Name: not supported by cvs2svn $
 */

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "irplib_strehl.h"
#include "irplib_utils.h"

#include <string.h>

/*-----------------------------------------------------------------------------
                                   Defines
 -----------------------------------------------------------------------------*/

#ifndef IMAGESZ
#define IMAGESZ 1024
#endif

#ifndef CWLEN
#define CWLEN 1.6
#endif

#ifndef DWLEN
#define DWLEN 0.05
#endif

#ifndef PIXSCALE
#define PIXSCALE 12.25e-3
#endif

/*-----------------------------------------------------------------------------
                                   Static functions
 -----------------------------------------------------------------------------*/
static void irplib_strehl_test_one(void);
static void irplib_psf_test_one(int);
static void irplib_psf_test(void);
static cpl_image * irplib_strehl_create(cpl_size, cpl_size, cpl_type,
                                        double, double, double);
static void irplib_strehl_test(const cpl_image *, double, double,
                               double, double, double, double, double,
                               const char *);

static void irplib_strehl_test_fits(const char *, double, double, double,
                                    double, double, cpl_size);

static const char * irplib_get_base_name(const char *);

struct fitsinfo {
    const char *bname; /* filename with extension, but without path */
    double pixscale; /* Arcseconds per pixel */
    double cwlen;    /* micron */
    double dwlen;    /* micron */
    double m1;       /* The diameter of the primary mirror [m] */
    double m2;       /* The diameter of the secondary mirror [m] */
    cpl_size iplane;
};

/*-----------------------------------------------------------------------------
                                  Main
 -----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/**
  @brief  Test the strehl estimation, optionally with FITS data from disk
  @param  argc The number of FITS files + 1
  @param  argv The name of the exectuable and optionally any FITS-data to test
  @return Zero, iff successful

 */
/*----------------------------------------------------------------------------*/
int main (int argc, char * argv[])
{
    /* Support the Strehl-method evaluation effort by Enrico Marchetti */
    const struct fitsinfo suite[] = {
        {"Berlin.fits",     12.25e-3, 1.6, 0.060, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 0},
        {"Frankfurt.fits",  12.25e-3, 1.6, 0.060, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 0},
        {"Hamburg.fits",    12.25e-3, 1.6, 0.060, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 0},
        {"Koeln.fits",      12.25e-3, 1.6, 0.060, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 0},
        {"Muenchen.fits",   12.25e-3, 1.6, 0.060, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 0},
        {"Stuttgart.fits",  12.25e-3, 1.6, 0.060, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 0},

        {"Torino_2.fits",   0.0331932, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Trieste_2.fits",  0.0331932, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Bologna_2.fits",  0.0331932, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Cagliari_2.fits", 0.0331932, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Catania_2.fits",  0.0331932, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Firenze_2.fits",  0.0331932, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Lapalma_2.fits",  0.0331932, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Milano_2.fits",   0.0331932, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Napoli_2.fits",   0.0331932, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Padova_2.fits",   0.0331932, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Palermo_2.fits",  0.0331932, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Roma_2.fits",     0.0331932, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Teramo_2.fits",   0.0331932, 1.635, 0.0001, 5.0800, 1.8288, 0},

        {"Bologna_4.fits",  0.0165966, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Cagliari_4.fits", 0.0165966, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Catania_4.fits",  0.0165966, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Firenze_4.fits",  0.0165966, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Lapalma.fits",    0.0165966, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Milano_4.fits",   0.0165966, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Naoli_4.fits",    0.0165966, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Padova_4.fits",   0.0165966, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Palermo_4.fits",  0.0165966, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Roma_4.fits",     0.0165966, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Teramo_4.fits",   0.0165966, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Torino_4.fits",   0.0165966, 1.635, 0.0001, 5.0800, 1.8288, 0},
        {"Trieste_4.fits",  0.0165966, 1.635, 0.0001, 5.0800, 1.8288, 0},

        {"Antofagasta.fits", 0.17678, 2.2  , 0.05, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 0},
        {"Bordeaux.fits",    0.01327, 2.166, 0.05, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 0},
        {"Concepcion.fits",  0.01768, 2.2  , 0.05, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 0},
        {"Grenoble.fits",    0.02715, 2.15 , 0.05, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 0},
        {"LeHavre.fits",     0.01327, 1.65 , 0.05, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 0},
        {"Lille.fits",       0.01327, 1.04 , 0.05, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 8},
        {"Lyon.fits",        0.01327, 2.15 , 0.05, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 0},
        {"Marseille.fits",   0.02715, 4.05 , 0.05, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 0},
        {"Nantes.fits",      0.0546 , 2.15 , 0.05, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 0},
        {"Nice.fits",        0.02715, 4.78 , 0.05, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 0},
        {"Paris.fits",       0.0033 , 2.18 , 0.05, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 0},
        {"Santiago.fits",    0.01768, 2.2  , 0.05, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 0},
        {"Strasbourg.fits",  0.01327, 2.166, 0.05, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 0},
        {"Toulouse.fits",    0.02715, 2.15 , 0.05, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 0},
        {"Valdivia.fits",    0.07071, 2.2  , 0.05, IRPLIB_STREHL_M1,
         IRPLIB_STREHL_M2, 0}};

    const size_t suitesz = sizeof(suite) / sizeof(*suite);
    int iarg;

    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    cpl_msg_info("Strehl-testing with star with ",
                 CPL_XSTRINGIFY(IRPLIB_STREHL_STAR_RADIUS) " = "
                 CPL_STRINGIFY(IRPLIB_STREHL_STAR_RADIUS) ". "
                 CPL_XSTRINGIFY(IRPLIB_STREHL_BACKGROUND_R1) " = "
                 CPL_STRINGIFY(IRPLIB_STREHL_BACKGROUND_R1) ". "
                 CPL_XSTRINGIFY(IRPLIB_STREHL_BACKGROUND_R2) " = "
                 CPL_STRINGIFY(IRPLIB_STREHL_BACKGROUND_R2) ".");

    irplib_strehl_test_one();

    for (iarg = 1; iarg < argc; iarg++) {
        const char * basename = irplib_get_base_name(argv[iarg]);
        double       pixscale = PIXSCALE;
        double       cwlen    = CWLEN;
        double       dwlen    = DWLEN;
        double       m1       = IRPLIB_STREHL_M1;
        double       m2       = IRPLIB_STREHL_M2;
        cpl_size     iplane   = 0;
        size_t       i;

        cpl_test_assert(basename != NULL);

        for (i = 0; i < suitesz; i++) {
            if (!strcmp(basename, suite[i].bname)) {
                pixscale = suite[i].pixscale;
                cwlen    = suite[i].cwlen;
                dwlen    = suite[i].dwlen;
                m1       = suite[i].m1;
                m2       = suite[i].m2;
                iplane   = suite[i].iplane;
                break;
            }
        }

        irplib_strehl_test_fits(argv[iarg], pixscale, cwlen, dwlen, m1, m2,
                                iplane);
    }

    irplib_psf_test();

    return cpl_test_end(0);
}


/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Test the strehl with a FITS-file and specific settings
  @param  file The FITS file to load
  @param  pixscale The pixel scale [Arcsecond]
  @param  lam      The central wavelength [micron]
  @param  dlam     The filter bandwidth [micron]
  @param  m1       The M1 diameter [m]
  @param  m2       The M2 diameter [m]
  @param  iplane   The plane number

 */
/*----------------------------------------------------------------------------*/
static void irplib_strehl_test_fits(const char * file, double pixscale,
                                    double lam, double dlam, double m1,
                                    double m2, cpl_size iplane)
{

    cpl_type type = CPL_TYPE_DOUBLE;
    int i;

    for (i = 0; i < 1; i++, type = CPL_TYPE_FLOAT) {

        cpl_image * img = cpl_image_load(file, type, iplane, 0);

        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_nonnull(img);

        irplib_strehl_test(img, 0.0, 10.0, pixscale, lam, dlam, m1, m2, file);

        cpl_image_delete(img);
    }
}


/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Test the strehl with specific settings

 */
/*----------------------------------------------------------------------------*/
static void irplib_psf_test(void)
{

    const int is_bench = cpl_msg_get_level() <= CPL_MSG_INFO
        ? CPL_TRUE : CPL_FALSE;

    double tstop;
    const double tstart = cpl_test_get_cputime();

    int szstart = 4;
    int szstop = is_bench ? IMAGESZ : IRPLIB_STREHL_BOX_SIZE * 2;

    int isz, irep;

    for (irep = 0; irep < (is_bench ? 3 : 1); irep++) {

        for (isz = szstart; isz <= szstop; isz *= 2) {
            irplib_psf_test_one(isz);
        }
    }

    tstop = cpl_test_get_cputime() - tstart;

    cpl_msg_info(cpl_func, "Time to generate %d set(s) of PSFs up to size "
                 "%d X %d [s]: %g", irep, szstop, szstop, tstop);
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Test the strehl with specific settings

 */
/*----------------------------------------------------------------------------*/
static void irplib_psf_test_one(int size)
{
    cpl_image * imgpsf = irplib_strehl_generate_psf(IRPLIB_STREHL_M1,
                                                    IRPLIB_STREHL_M2,
                                                    CWLEN, DWLEN, PIXSCALE,
                                                    size);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_nonnull(imgpsf);
    cpl_test_eq(cpl_image_get_size_x(imgpsf),
                cpl_image_get_size_y(imgpsf));

    cpl_test_eq(cpl_image_get_size_x(imgpsf), size);

    cpl_image_delete(imgpsf);
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Test the strehl with specific settings

 */
/*----------------------------------------------------------------------------*/
static void irplib_strehl_test_one(void)
{
    cpl_type type = CPL_TYPE_DOUBLE;
    int i;

    for (i = 0; i < 2; i++, type = CPL_TYPE_FLOAT) {

        cpl_image * img = irplib_strehl_create(IMAGESZ, IMAGESZ, type,
                                               1000.0, 1.0, 1.0);

        irplib_strehl_test(img, 1000.0, 1.0, 0.03,
                           2.18, 0.35, IRPLIB_STREHL_M1, IRPLIB_STREHL_M2,
                           "Synthetic image"); /* NACO values */

        cpl_image_delete(img);
    }
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Fill image for strehl testing
  @param    nx          Image X-size
  @param    ny          Image Y-size
  @param    type        Image pixel type
  @param    norm        norm of the gaussian.
  @param    sig_x       Sigma in x for the gaussian distribution.
  @param    sig_y       Sigma in y for the gaussian distribution.
  @return   The created image

 */
/*----------------------------------------------------------------------------*/
static cpl_image * irplib_strehl_create(cpl_size nx, cpl_size ny,
                                        cpl_type type,
                                        double norm,
                                        double sig_x,
                                        double sig_y)
{

    const double noise = FLT_EPSILON;
    cpl_size     mx, my;
    cpl_image * im1 = cpl_image_new(nx, ny, type);
    cpl_error_code code;

    code = cpl_image_fill_gaussian(im1, nx/2, ny/2, norm, sig_x, sig_y);
    cpl_test_eq_error(code, CPL_ERROR_NONE);

    if (noise != 0.0) {
        cpl_image * im0 = cpl_image_new(nx, ny, CPL_TYPE_FLOAT);
        code = cpl_image_fill_noise_uniform(im0, -noise, noise);
        cpl_test_eq_error(code, CPL_ERROR_NONE);
        code = cpl_image_subtract(im1, im0);
        cpl_test_eq_error(code, CPL_ERROR_NONE);
        cpl_image_delete(im0);
    }

    code = cpl_image_get_maxpos(im1, &mx, &my);
    cpl_test_eq_error(code, CPL_ERROR_NONE);
    cpl_test_eq(mx, nx/2);
    cpl_test_eq(my, ny/2);

    return im1;
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Test Strehl computation
  @param    norm        norm of the gaussian, or zero
  @param    sigma       The detection level
  @param    pixscale    The pixel scale (arcsecs/pixel)
  @param    lam         Central wavelength [micron]
  @param    dlam        Filter bandwidth [micron]
  @param    m1          The M1 diameter [m]
  @param    m2          The M2 diameter [m]
  @param    label       Image label for messaging
  @return   void

 */
/*----------------------------------------------------------------------------*/
static void irplib_strehl_test(const cpl_image * im1, double norm,
                               double sigma,
                               double pixscale, double lam, double dlam,
                               double m1, double m2,
                               const char * label)
{

    const cpl_size nx = cpl_image_get_size_x(im1);
    const cpl_size ny = cpl_image_get_size_y(im1);
    cpl_error_code code;

    /* Sigma-levels for detection of a bright star, copied from NACO */
    const double    psigmas[] = {sigma, sigma/2.0, sigma/4.0};
    const size_t    nsigmas = sizeof(psigmas)/sizeof(*psigmas);
    cpl_size        isigma;
    cpl_vector    * sigmas = cpl_vector_wrap(nsigmas, (double*)psigmas);
    cpl_apertures * apert = NULL;
    double          fwhm_x, fwhm_y;
    cpl_size        mx, my;
    cpl_image     * imgpsf;

    double strehl = 0.0, strehl_err = 0.0;
    double star_bg = 0.0, star_peak = 0.0, star_flux = 0.0;
    double psf_peak = 0.0, psf_flux = 0.0, bg_noise = 0.0;

    const double star_radius  = IRPLIB_STREHL_STAR_RADIUS;
    const double background_1 = IRPLIB_STREHL_BACKGROUND_R1;
    const double background_2 = IRPLIB_STREHL_BACKGROUND_R2;

    code = cpl_image_get_maxpos(im1, &mx, &my);
    cpl_test_eq_error(code, CPL_ERROR_NONE);
    cpl_test_leq(mx - nx/4, mx);
    cpl_test_leq(my - ny/4, my);
    cpl_test_leq(mx, mx + nx/4);
    cpl_test_leq(my, my + ny/4);

    apert = cpl_apertures_extract_window(im1, sigmas,
                                         mx - nx/4, my - ny/4,
                                         mx + nx/4, my + ny/4,
                                         &isigma);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_nonnull(apert);
    cpl_test_zero(isigma);

    cpl_apertures_delete(apert);
    cpl_test_eq_ptr(cpl_vector_unwrap(sigmas), psigmas);

    cpl_test_lt(0.0, pixscale);

    code = cpl_image_get_fwhm(im1, mx, my, &fwhm_x, &fwhm_y);
    cpl_test_eq_error(code, CPL_ERROR_NONE);

    cpl_msg_info(cpl_func, "Expected star-radius vs. actual FWHM [pixel]: "
                 "%g <=> (%g, %g)", star_radius / pixscale,
                 fwhm_x, fwhm_y);

    cpl_msg_debug(cpl_func, "Inner and outer radius of ring-zone for noise "
                  "estimate [pixel]: %g < %g", background_1 / pixscale,
                  background_2 / pixscale);


    imgpsf = irplib_strehl_generate_psf(m1, m2,
                                        lam, dlam, pixscale,
                                        IRPLIB_STREHL_BOX_SIZE);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_nonnull(imgpsf);
    cpl_test_eq(cpl_image_get_size_x(imgpsf),
                cpl_image_get_size_y(imgpsf));

    cpl_test_eq(cpl_image_get_size_x(imgpsf),
                IRPLIB_STREHL_BOX_SIZE);

    if (cpl_msg_get_level() <= CPL_MSG_DEBUG) {
        cpl_plot_image("", "", "", imgpsf);
    }
    cpl_image_delete(imgpsf);

    code = irplib_strehl_compute(im1, m1, m2,
                                 lam, dlam, pixscale,
                                 IRPLIB_STREHL_BOX_SIZE,
                                 mx, my,
                                 star_radius,
                                 background_1,
                                 background_2,
                                 -1, -1,
                                 &strehl,
                                 &strehl_err,
                                 &star_bg,
                                 &star_peak,
                                 &star_flux,
                                 &psf_peak,
                                 &psf_flux,
                                 &bg_noise);
    cpl_test_eq_error(code, CPL_ERROR_NONE);
    cpl_test_leq(0.0, strehl);
    cpl_test_leq(strehl, 1.0);
    cpl_test_leq(0.0, strehl_err);
    if (norm > 0.0) {
        cpl_test_rel(norm, star_flux, 0.01);
    }

    cpl_msg_info(cpl_func, "Strehl: ratio=%g, error=%g; Background: flux=%g, "
                 "noise=%g; Star: peak=%g, flux=%g; PSF: peak=%g, flux=%g",
                 strehl, strehl_err, star_bg, bg_noise, star_peak, star_flux,
                 psf_peak, psf_flux);

    cpl_msg_info(cpl_func, "%-16s %-10.9g %-8.6g %-8.6g %-8.6g %-8.6g "
                 "%-8.4g %-8.4g", label, pixscale, lam, dlam, m1, m2,
                 strehl, strehl_err);

    /* Test for various errors */

    /* M2 < 0*/
    code = irplib_strehl_compute(im1, m1, 0.0,
                                 lam, dlam, pixscale,
                                 IRPLIB_STREHL_BOX_SIZE, 
                                 nx/2, ny/2,
                                 star_radius,
                                 background_1,
                                 background_2,
                                 -1, -1,
                                 &strehl,
                                 &strehl_err,
                                 &star_bg,
                                 &star_peak,
                                 &star_flux,
                                 &psf_peak,
                                 &psf_flux,
                                 &bg_noise);
    cpl_test_eq_error(code, CPL_ERROR_ILLEGAL_INPUT);


    /* M1 < M2 */
    code = irplib_strehl_compute(im1, m2, m1,
                                 lam, dlam, pixscale,
                                 IRPLIB_STREHL_BOX_SIZE, 
                                 nx/2, ny/2,
                                 star_radius,
                                 background_1,
                                 background_2,
                                 -1, -1,
                                 &strehl,
                                 &strehl_err,
                                 &star_bg,
                                 &star_peak,
                                 &star_flux,
                                 &psf_peak,
                                 &psf_flux,
                                 &bg_noise);
    cpl_test_eq_error(code, CPL_ERROR_ILLEGAL_INPUT);


    /* lam = 0 */
    code = irplib_strehl_compute(im1, m1, m2,
                                 0.0, dlam, pixscale,
                                 IRPLIB_STREHL_BOX_SIZE, 
                                 nx/2, ny/2,
                                 star_radius,
                                 background_1,
                                 background_2,
                                 -1, -1,
                                 &strehl,
                                 &strehl_err,
                                 &star_bg,
                                 &star_peak,
                                 &star_flux,
                                 &psf_peak,
                                 &psf_flux,
                                 &bg_noise);
    cpl_test_eq_error(code, CPL_ERROR_ILLEGAL_INPUT);

    /* dlam = 0 */
    code = irplib_strehl_compute(im1, m1, m2,
                                 lam, 0.0, pixscale,
                                 IRPLIB_STREHL_BOX_SIZE, 
                                 nx/2, ny/2,
                                 star_radius,
                                 background_1,
                                 background_2,
                                 -1, -1,
                                 &strehl,
                                 &strehl_err,
                                 &star_bg,
                                 &star_peak,
                                 &star_flux,
                                 &psf_peak,
                                 &psf_flux,
                                 &bg_noise);
    cpl_test_eq_error(code, CPL_ERROR_ILLEGAL_INPUT);

    /* pixscale = 0 */
    code = irplib_strehl_compute(im1, m1, m2,
                                 lam, dlam, 0.0,
                                 IRPLIB_STREHL_BOX_SIZE, 
                                 nx/2, ny/2,
                                 star_radius,
                                 background_1,
                                 background_2,
                                 -1, -1,
                                 &strehl,
                                 &strehl_err,
                                 &star_bg,
                                 &star_peak,
                                 &star_flux,
                                 &psf_peak,
                                 &psf_flux,
                                 &bg_noise);
    cpl_test_eq_error(code, CPL_ERROR_ILLEGAL_INPUT);

    /* IRPLIB_STREHL_BOX_SIZE = 0 */
    code = irplib_strehl_compute(im1, m1, m2,
                                 lam, dlam, pixscale,
                                 0, 
                                 nx/2, ny/2,
                                 star_radius,
                                 background_1,
                                 background_2,
                                 -1, -1,
                                 &strehl,
                                 &strehl_err,
                                 &star_bg,
                                 &star_peak,
                                 &star_flux,
                                 &psf_peak,
                                 &psf_flux,
                                 &bg_noise);
    cpl_test_eq_error(code, CPL_ERROR_ILLEGAL_INPUT);

    /* star_radius = 0 */
    code = irplib_strehl_compute(im1, m1, m2,
                                 lam, dlam, pixscale,
                                 IRPLIB_STREHL_BOX_SIZE, 
                                 nx/2, ny/2,
                                 0.0,
                                 background_1,
                                 background_2,
                                 -1, -1,
                                 &strehl,
                                 &strehl_err,
                                 &star_bg,
                                 &star_peak,
                                 &star_flux,
                                 &psf_peak,
                                 &psf_flux,
                                 &bg_noise);
    cpl_test_eq_error(code, CPL_ERROR_ILLEGAL_INPUT);

    /* background_1 = 0 */
    code = irplib_strehl_compute(im1, m1, m2,
                                 lam, dlam, pixscale,
                                 IRPLIB_STREHL_BOX_SIZE, 
                                 nx/2, ny/2,
                                 star_radius,
                                 0.0,
                                 background_2,
                                 -1, -1,
                                 &strehl,
                                 &strehl_err,
                                 &star_bg,
                                 &star_peak,
                                 &star_flux,
                                 &psf_peak,
                                 &psf_flux,
                                 &bg_noise);
    cpl_test_eq_error(code, CPL_ERROR_ILLEGAL_INPUT);

    /* background_2 too small */
    code = irplib_strehl_compute(im1, m1, m2,
                                 lam, dlam, pixscale,
                                 IRPLIB_STREHL_BOX_SIZE, 
                                 nx/2, ny/2,
                                 star_radius,
                                 background_1,
                                 background_1,
                                 -1, -1,
                                 &strehl,
                                 &strehl_err,
                                 &star_bg,
                                 &star_peak,
                                 &star_flux,
                                 &psf_peak,
                                 &psf_flux,
                                 &bg_noise);
    cpl_test_eq_error(code, CPL_ERROR_ILLEGAL_INPUT);

    /* strehl pointer is NULL */
    code = irplib_strehl_compute(im1, m1, m2,
                                 lam, dlam, pixscale,
                                 IRPLIB_STREHL_BOX_SIZE, 
                                 nx/2, ny/2,
                                 star_radius,
                                 background_1,
                                 background_2,
                                 -1, -1,
                                 NULL,
                                 &strehl_err,
                                 &star_bg,
                                 &star_peak,
                                 &star_flux,
                                 &psf_peak,
                                 &psf_flux,
                                 &bg_noise);
    cpl_test_eq_error(code, CPL_ERROR_NULL_INPUT);

    /* strehl_err pointer is NULL */
    code = irplib_strehl_compute(im1, m1, m2,
                                 lam, dlam, pixscale,
                                 IRPLIB_STREHL_BOX_SIZE, 
                                 nx/2, ny/2,
                                 star_radius,
                                 background_1,
                                 background_2,
                                 -1, -1,
                                 &strehl,
                                 NULL,
                                 &star_bg,
                                 &star_peak,
                                 &star_flux,
                                 &psf_peak,
                                 &psf_flux,
                                 &bg_noise);
    cpl_test_eq_error(code, CPL_ERROR_NULL_INPUT);

    /* star_bg pointer is NULL */
    code = irplib_strehl_compute(im1, m1, m2,
                                 lam, dlam, pixscale,
                                 IRPLIB_STREHL_BOX_SIZE, 
                                 nx/2, ny/2,
                                 star_radius,
                                 background_1,
                                 background_2,
                                 -1, -1,
                                 &strehl,
                                 &strehl_err,
                                 NULL,
                                 &star_peak,
                                 &star_flux,
                                 &psf_peak,
                                 &psf_flux,
                                 &bg_noise);
    cpl_test_eq_error(code, CPL_ERROR_NULL_INPUT);

    /* star_peak pointer is NULL */
    code = irplib_strehl_compute(im1, m1, m2,
                                 lam, dlam, pixscale,
                                 IRPLIB_STREHL_BOX_SIZE, 
                                 nx/2, ny/2,
                                 star_radius,
                                 background_1,
                                 background_2,
                                 -1, -1,
                                 &strehl,
                                 &strehl_err,
                                 &star_bg,
                                 NULL,
                                 &star_flux,
                                 &psf_peak,
                                 &psf_flux,
                                 &bg_noise);
    cpl_test_eq_error(code, CPL_ERROR_NULL_INPUT);

    /* psf_peak pointer is NULL */
    code = irplib_strehl_compute(im1, m1, m2,
                                 lam, dlam, pixscale,
                                 IRPLIB_STREHL_BOX_SIZE, 
                                 nx/2, ny/2,
                                 star_radius,
                                 background_1,
                                 background_2,
                                 -1, -1,
                                 &strehl,
                                 &strehl_err,
                                 &star_bg,
                                 &star_peak,
                                 &star_flux,
                                 NULL,
                                 &psf_flux,
                                 &bg_noise);
    cpl_test_eq_error(code, CPL_ERROR_NULL_INPUT);

    /* psf_flux pointer is NULL */
    code = irplib_strehl_compute(im1, m1, m2,
                                 lam, dlam, pixscale,
                                 IRPLIB_STREHL_BOX_SIZE, 
                                 nx/2, ny/2,
                                 star_radius,
                                 background_1,
                                 background_2,
                                 -1, -1,
                                 &strehl,
                                 &strehl_err,
                                 &star_bg,
                                 &star_peak,
                                 &star_flux,
                                 &psf_peak,
                                 NULL,
                                 &bg_noise);
    cpl_test_eq_error(code, CPL_ERROR_NULL_INPUT);


    /* bg_noise pointer is NULL */
    code = irplib_strehl_compute(im1, m1, m2,
                                 lam, dlam, pixscale,
                                 IRPLIB_STREHL_BOX_SIZE, 
                                 nx/2, ny/2,
                                 star_radius,
                                 background_1,
                                 background_2,
                                 -1, -1,
                                 &strehl,
                                 &strehl_err,
                                 &star_bg,
                                 &star_peak,
                                 &star_flux,
                                 &psf_peak,
                                 &psf_flux,
                                 NULL);
    cpl_test_eq_error(code, CPL_ERROR_NULL_INPUT);
}


/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief  Return a pointer to the basename of a full-path filename
  @param  self   The filename
  @return The pointer (possibly self, which may be NULL)
  @note   The pointer returned by this function may not be used after self is
          de/re-allocated, nor after a call of the function with another string.
  @see cpl_get_base_name()
 */
/*----------------------------------------------------------------------------*/
static const char * irplib_get_base_name(const char * self)
{
    const char * p = self ? strrchr(self, '/') : NULL;

    return p ? p + 1 : self;
}
