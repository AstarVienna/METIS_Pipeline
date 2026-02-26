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

#include "hdrl_strehl.c"
#include "hdrl_strehl.h"
#include "hdrl_test.h"

#include <cpl.h>

#include <math.h>
#include <string.h>

#ifndef ARRAY_LEN
#define ARRAY_LEN(a) sizeof((a))/sizeof((a)[0])
#endif


/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_strehl_test  Testing of the HDRL Strehl module
 */
/*----------------------------------------------------------------------------*/

void hdrl_strehl_test_parlist(void)
{
	/* Test null parameters */
	hdrl_parameter *pFake1 = hdrl_strehl_parameter_create(
			0., 0., 0., 0., 0., 0., 0., 0.);
	hdrl_strehl_parameter_verify(NULL);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
    hdrl_parameter_delete(pFake1);


    /* parameter parsing smoketest */
    double wavelength = 1.635e-6;
    double m1 = 5.08/2;
    double m2 = 5.08/2*0.36;
    double psx = 0.0331932/2.;
    double psy = 0.0331932/2.;
    double r1 = 1.5;
    double r2 = 1.5;
    double r3 = 2.0;


    /* Create parameter */
    hdrl_parameter * hpar;
    hdrl_parameter * strehl_def =
        hdrl_strehl_parameter_create(wavelength, m1, m2, psx, psy, r1, r2, r3);


    /* Check parameter */
    cpl_test(!hdrl_strehl_parameter_check(pFake1));
    cpl_test( hdrl_strehl_parameter_check(strehl_def));
    cpl_test_error(CPL_ERROR_NONE);


    /* Gets */

    cpl_test_eq(hdrl_strehl_parameter_get_wavelength(NULL), -1.);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_eq(hdrl_strehl_parameter_get_wavelength(strehl_def), wavelength);
    cpl_test_error(CPL_ERROR_NONE);

    cpl_test_eq(hdrl_strehl_parameter_get_m1(NULL), -1.);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_eq(hdrl_strehl_parameter_get_m1(strehl_def), m1);
	cpl_test_error(CPL_ERROR_NONE);

	cpl_test_eq(hdrl_strehl_parameter_get_m2(NULL), -1);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_eq(hdrl_strehl_parameter_get_m2(strehl_def), m2);
	cpl_test_error(CPL_ERROR_NONE);

	cpl_test_eq(hdrl_strehl_parameter_get_pixel_scale_x(NULL), -1);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_eq(hdrl_strehl_parameter_get_pixel_scale_x(strehl_def), psx);
	cpl_test_error(CPL_ERROR_NONE);

	cpl_test_eq(hdrl_strehl_parameter_get_pixel_scale_y(NULL), -1);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_eq(hdrl_strehl_parameter_get_pixel_scale_y(strehl_def), psy);
	cpl_test_error(CPL_ERROR_NONE);

	cpl_test_eq(hdrl_strehl_parameter_get_flux_radius(NULL), -1);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_eq(hdrl_strehl_parameter_get_flux_radius(strehl_def), r1);
	cpl_test_error(CPL_ERROR_NONE);

	cpl_test_eq(hdrl_strehl_parameter_get_bkg_radius_low(NULL), -1);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_eq(hdrl_strehl_parameter_get_bkg_radius_low(strehl_def), r2);
	cpl_test_error(CPL_ERROR_NONE);

	cpl_test_eq(hdrl_strehl_parameter_get_bkg_radius_high(NULL), -1);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_eq(hdrl_strehl_parameter_get_bkg_radius_high(strehl_def), r3);
	cpl_test_error(CPL_ERROR_NONE);


    cpl_parameterlist * strehl = hdrl_strehl_parameter_create_parlist(
                "RECIPE", "strehl", strehl_def);

    hdrl_parameter_delete(strehl_def);
    cpl_test_error(CPL_ERROR_NONE);

    cpl_test_eq(cpl_parameterlist_get_size(strehl), 8);

    hpar = hdrl_strehl_parameter_parse_parlist(strehl, "RECIPE.invalid");
    cpl_test_null(hpar);
    cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);

    hpar = hdrl_strehl_parameter_parse_parlist(strehl, "RECIPE.strehl");
    cpl_parameterlist_delete(strehl);
    cpl_test_error(CPL_ERROR_NONE);
    {
        cpl_test_eq(hdrl_strehl_parameter_get_wavelength(hpar), wavelength);
        cpl_test_eq(hdrl_strehl_parameter_get_m1(hpar), m1);
        cpl_test_eq(hdrl_strehl_parameter_get_m2(hpar), m2);
        cpl_test_eq(hdrl_strehl_parameter_get_pixel_scale_x(hpar), psx);
        cpl_test_eq(hdrl_strehl_parameter_get_pixel_scale_y(hpar), psy);
        cpl_test_eq(hdrl_strehl_parameter_get_flux_radius(hpar), r1);
        cpl_test_eq(hdrl_strehl_parameter_get_bkg_radius_low(hpar), r2);
        cpl_test_eq(hdrl_strehl_parameter_get_bkg_radius_high(hpar), r3);
    }

    hdrl_parameter_destroy(hpar);
}

/*----------------------------------------------------------------------------*/
/**
  @brief Check hdrl_strehl_compute proper error on null input images
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static hdrl_image* hdrl_strehl_test_gauss_create(void)
{

    double sig_x = 3.;
    double sig_y = 3.;
    cpl_size n = 5;
    double dmad;
    hdrl_image* hgauss;
    cpl_image * gauss_data;
    cpl_image * gauss_error;

    gauss_data = cpl_image_new(2 * n + 1, 2 * n + 1, CPL_TYPE_DOUBLE);
    cpl_image_fill_gaussian(gauss_data, n + 1, n + 1, (double)121.0, sig_x, sig_y);
    gauss_error = cpl_image_duplicate(gauss_data);
    cpl_image_multiply_scalar(gauss_error, 0);
    cpl_image_get_mad(gauss_data, &dmad);
    cpl_image_add_scalar(gauss_error, (dmad * CPL_MATH_STD_MAD));

    hgauss = hdrl_image_create(gauss_data, gauss_error);
    cpl_image_delete(gauss_data);
    cpl_image_delete(gauss_error);

    return hgauss;
}


/*----------------------------------------------------------------------------*/
/**
  @brief Check hdrl_strehl_compute proper error on null input images
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_strehl_test_null_input(void)
{
    double w;

    double m1 = 5.08/2;
    double m2 = 5.08/2*0.36;
    double psx = 0.0331932/2.;
    double psy = 0.0331932/2.;
    double r1 = 1.5;
    double r2 = 1.5;
    double r3 = 2.0;

    hdrl_parameter     *strehl_param;
    hdrl_strehl_result  strehl_result;

    /* test functionality - Image null*/
    hdrl_image *hima = NULL;
    w = 1.635e-6;
    strehl_param = hdrl_strehl_parameter_create(w, m1, m2, psx, psy, r1, r2, r3);
    cpl_test(hdrl_strehl_parameter_check(strehl_param));
    strehl_result = hdrl_strehl_compute(hima, strehl_param) ;
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    hdrl_parameter_delete(strehl_param) ;
    cpl_test(isnan(strehl_result.strehl_value.data));

    /* Strehl Parameter wrong */
    hima = hdrl_image_new(10, 10);
    w = -1.;
    strehl_param = hdrl_strehl_parameter_create(w, m1, m2, psx, psy, r1, r2, r3);
    cpl_test(!hdrl_strehl_parameter_check(strehl_param));
    strehl_result = hdrl_strehl_compute(hima, strehl_param);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    hdrl_parameter_delete(strehl_param);
    hdrl_image_delete(hima);
    cpl_test(isnan(strehl_result.strehl_value.data));

    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Check hdrl_strehl_compute proper error on illegal inputs
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_strehl_test_illegal_input(void)
{
    double w = 1.635e-6;
    double m1 = 5.08/2;
    double m2 = 5.08/2*0.36;
    double psx = 0.0331932/2.;
    double psy = 0.0331932/2.;
    double r1 = 1.5;
    double r2 = 1.5;
    double r3 = 2.0;

    hdrl_image * hima = hdrl_strehl_test_gauss_create();
    hdrl_parameter * strehl_param;

    strehl_param = hdrl_strehl_parameter_create(-1, m1, m2, psx, psy, r1, r2, r3);
    cpl_test_null(strehl_param);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    strehl_param = hdrl_strehl_parameter_create(w, -1, m2, psx, psy, r1, r2, r3);
    cpl_test_null(strehl_param);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    strehl_param = hdrl_strehl_parameter_create(w, m1, -1, psx, psy, r1, r2, r3);
    cpl_test_null(strehl_param);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    strehl_param = hdrl_strehl_parameter_create(w, m1, m2, -1, psy, r1, r2, r3);
    cpl_test_null(strehl_param);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    strehl_param = hdrl_strehl_parameter_create(w, m1, m2, psx, -1, r1, r2, r3);
    cpl_test_null(strehl_param);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    strehl_param = hdrl_strehl_parameter_create(w, m1, m2, psx, psy, -1, r2, r3);
    cpl_test_null(strehl_param);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    strehl_param = hdrl_strehl_parameter_create(w, m1, m2, psx, psy, r1, -1, r3);
    cpl_test_null(strehl_param);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    strehl_param = hdrl_strehl_parameter_create(w, m1, m2, psx, psy, r1, r2, -1);
    cpl_test_null(strehl_param);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    strehl_param = hdrl_strehl_parameter_create(w, m1, m1 + 1, psx, psy, r1, r2, r3);
    cpl_test_null(strehl_param);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    hdrl_image_delete(hima);

    return cpl_error_get_code();
}



/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_strehl_test
            Testing of hdrl_strehl module
 */
/*----------------------------------------------------------------------------*/

void test_psf(void)
{
    double m1 = 8.3 / 2; /* telescope mirror radii [m] */
    double m2 = 1.1 / 2; /* telescope obstruction radii [m] */
    size_t nx = 256; /* PSF image X size */
    size_t ny = 256; /* PSF image Y size */
    size_t hx = nx / 2; /* pixel position of peak (FITS) */
    size_t hy = ny / 2; /* pixel position of peak (FITS) */
    double peak; /* expected PSF peak value */
    int rej;


    double wavelength = 7.7e-6; /* observing wavelength in [m] */
    double pscale_x = 0.075; /* pixel scale in x [as] */
    double pscale_y = 0.075; /* pixel scale in y [as] */

    /* exactly centered psf (symmetric) */
    cpl_image * psf = compute_psf(wavelength, m1, m2, pscale_x, pscale_y,
                                  hx, hy,
                                  nx, ny);
    peak = 0.670695;
    cpl_test_abs(cpl_image_get(psf, hx, hy, &rej), 1., 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx - 1, hy, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx, hy - 1, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx + 1, hy, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx, hy + 1, &rej), peak, 1e-4);
    cpl_image_delete(psf);

    /* exactly centered psf (symmetric) one pixel lower in x */
    psf = compute_psf(wavelength, m1, m2, pscale_x, pscale_y,
                      hx - 1, hy,
                      nx, ny);
    cpl_test_abs(cpl_image_get(psf, hx - 1, hy, &rej), 1., 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx - 2, hy, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx - 1, hy - 1, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx, hy, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx - 1, hy + 1, &rej), peak, 1e-4);
    cpl_image_delete(psf);

    /* centered at origin of pixel -> square block */
    psf = compute_psf(wavelength, m1, m2, pscale_x, pscale_y,
                      hx - 0.5, hy - 0.5,
                      nx, ny);
    peak = 0.821877;
    cpl_test_abs(cpl_image_get(psf, hx, hy, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx - 1, hy - 1, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx, hy - 1, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx - 1, hy, &rej), peak, 1e-4);
    cpl_image_delete(psf);

    /* + .75 / 0.25  */
    psf = compute_psf(wavelength, m1, m2, pscale_x, pscale_y,
                      hx + .75, hy + .25,
                      nx, ny);
    cpl_test_abs(cpl_image_get(psf, hx, hy, &rej), 0.781698, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx - 1, hy, &rej), 0.255305, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx, hy - 1, &rej), 0.411749, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx + 1, hy, &rej), 0.952739, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx, hy + 1, &rej), 0.636695, 1e-4);
    cpl_image_delete(psf);

    /* centered at origin of pixel -> square block
     * asymmetric pixel scale */
    psf = compute_psf(wavelength, m1, m2, pscale_x, 0.025,
                      hx - 0.5, hy - 0.5,
                      nx, ny);
    peak = 0.897496;
    cpl_test_abs(cpl_image_get(psf, hx, hy, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx - 1, hy - 1, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx, hy - 1, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx - 1, hy, &rej), peak, 1e-4);
    /* x direction tails */
    peak = 0.383906;
    cpl_test_abs(cpl_image_get(psf, hx + 1, hy, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx + 1, hy - 1, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx - 2, hy, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx - 2, hy - 1, &rej), peak, 1e-4);
    /* y direction tails */
    peak = 0.821877;
    cpl_test_abs(cpl_image_get(psf, hx, hy + 1, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx - 1, hy + 1, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx, hy - 2, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx - 1, hy - 2, &rej), peak, 1e-4);
    cpl_image_delete(psf);

    /* exactly centered psf (symmetric) double sampled*/
    hx = nx; /* fits */
    hy = ny; /* fits */
    psf = compute_psf(wavelength, m1, m2, pscale_x / 2., pscale_y / 2.,
                      hx, hx,
                      nx * 2, ny * 2);
    peak = 0.907339;
    cpl_test_abs(cpl_image_get(psf, hx, hy, &rej), 1., 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx - 1, hy, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx, hy - 1, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx + 1, hy, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx, hy + 1, &rej), peak, 1e-4);
    cpl_image_delete(psf);

    /* centered at origin of pixel -> square block
     * asymmetric pixel scale and image size*/
    ny = 2 * nx;
    hx = nx / 2; /* fits */
    hy = ny / 2; /* fits */
    psf = compute_psf(wavelength, m1, m2, pscale_x, 0.025,
                      hx - 0.5, hy - 0.5,
                      nx, ny);
    peak = 0.897496;
    cpl_test_abs(cpl_image_get(psf, hx, hy, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx - 1, hy - 1, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx, hy - 1, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx - 1, hy, &rej), peak, 1e-4);
    /* x direction tails */
    peak = 0.383906;
    cpl_test_abs(cpl_image_get(psf, hx + 1, hy, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx + 1, hy - 1, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx - 2, hy, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx - 2, hy - 1, &rej), peak, 1e-4);
    /* y direction tails */
    peak = 0.821877;
    cpl_test_abs(cpl_image_get(psf, hx, hy + 1, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx - 1, hy + 1, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx, hy - 2, &rej), peak, 1e-4);
    cpl_test_abs(cpl_image_get(psf, hx - 1, hy - 2, &rej), peak, 1e-4);
    cpl_image_delete(psf);
}


void test_strehl_with_bkg(void)
{
    hdrl_image * himg;
    cpl_image * img;
    hdrl_strehl_result r;
    /* TODO add more unit tests */
    double m1 = 8.3 / 2;
    double m2 = 1.1 / 2;
    size_t nx = 256;
    size_t ny = 256;
    double lam = 7.7e-6;
    /* oversampled image, 0.075 would be about 2*nyquist */
    double pscale = 0.03;

    /* realistic bkg slope */
    double slope_x = 1. / nx * 100;

    img = compute_psf(lam, m1, m2, pscale, pscale, nx / 2., ny / 2., nx, ny);

    /* we multiply for a large factor to make sure that the psf has high S/N */
    cpl_image_multiply_scalar(img, 2000.);

    /* create a background image with a simple slope along X direction */
    cpl_image *bkg = cpl_image_duplicate(img);
    cpl_image_multiply_scalar(bkg, 0);
    double *pbkg=cpl_image_get_data_double(bkg);


    for(size_t j = 0; j < ny; j++) {
        for(size_t i = 0; i < nx; i++) {
            pbkg[i+nx*j] = i * slope_x;
        }
    }
    cpl_image_add(img, bkg);
    /*
      cpl_image_save(img, "data.fits", CPL_TYPE_FLOAT, NULL, CPL_IO_DEFAULT);
      cpl_image_save(bkg, "bkg.fits", CPL_TYPE_FLOAT, NULL, CPL_IO_DEFAULT);
     */

    himg = hdrl_image_create(img, NULL);

    /* upsampling/downsampling introduces an error */
    double rel = 0.015;

    /* real test starts here: we need to subtract bkg from proper region */
    r = compute_strehl(himg, lam, m1, m2, pscale, pscale, .5, 2.5, 3.0);
    cpl_test_abs(r.strehl_value.data, 1.0, rel);

    hdrl_image_delete(himg);
    cpl_image_delete(img);
    cpl_image_delete(bkg);

}

void test_strehl(void)
{
    hdrl_image * himg;
    cpl_image * img;
    hdrl_strehl_result r;
    /* TODO add more unit tests */
    double m1 = 8.3 / 2;
    double m2 = 1.1 / 2;
    size_t nx = 256;
    size_t ny = 256;
    double lam = 7.7e-6;
    /* oversampled image, 0.075 would be about 2*nyquist */
    double pscale = 0.03;
    img = compute_psf(lam, m1, m2, pscale, pscale,
                      nx / 2, ny / 2, nx, ny);
    himg = hdrl_image_create(img, NULL);

    /* add unmasked larger maximum */
    hdrl_image_set_pixel(himg, 28, 231, (hdrl_value){1.5, 1.5});

    /* bad background */
    compute_strehl(himg, lam, m1, m2, pscale, pscale, 1.5, -1, 4.);
    cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

    compute_strehl(himg, lam, m1, m2, pscale, pscale, 1.5, 4., -1);
    cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

    compute_strehl(himg, lam, m1, m2, pscale, pscale, 1.5, 5, 5);
    cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_mask_not(hdrl_image_get_mask(himg));

    compute_strehl(himg, lam, m1, m2, pscale, pscale, 1.5, 5, 6);
    cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
    cpl_mask_not(hdrl_image_get_mask(himg));

    /* upsampling/downsampling introduces an error */
    double rel = 0.015;

    r = compute_strehl(himg, lam, m1, m2, pscale, pscale, .5, -1, -1);
    cpl_test_abs(r.strehl_value.data, 1.0, rel);
    cpl_test_abs(r.star_x, nx / 2., rel);
    cpl_test_abs(r.star_y, nx / 2., rel);

    /* test with background */
    hdrl_image_add_scalar(himg, (hdrl_value){5., 0.});
    r = compute_strehl(himg, lam, m1, m2, pscale, pscale, .5, 2, 3);
    cpl_test_abs(r.strehl_value.data, 1.0, rel);
    cpl_test_abs(r.star_background.data, 5.0, rel);
    cpl_test_abs(r.star_background.error, 0., rel);

    /* test with bad pixel in background */
    hdrl_image_set_pixel(himg, 128, 161, (hdrl_value){1e20, 1e20});
    hdrl_image_reject(himg, 128, 161);
    r = compute_strehl(himg, lam, m1, m2, pscale, pscale, .5, 2, 3);
    cpl_test_abs(r.strehl_value.data, 1.0, rel);
    cpl_test_abs(r.star_background.data, 5.0, rel);
    cpl_test_abs(r.star_background.error, 0., rel);

    /* test zero background */
    compute_strehl(himg, lam, m1, m2, pscale, pscale, .5, 50, 55);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    hdrl_image_delete(himg);
    cpl_image_delete(img);


    /* test other psf offsets */
    for (size_t i = 0; i < 10; i++) {
        img = compute_psf(lam, m1, m2, pscale, pscale,
                          nx / 2 + i / 10., ny / 2 + i / 10., nx, ny);
        himg = hdrl_image_create(img, NULL);
        r = compute_strehl(himg, lam, m1, m2, pscale, pscale, .5, -1, -1);
        cpl_test_abs(r.strehl_value.data, 1.0, rel);
        hdrl_image_delete(himg);
        cpl_image_delete(img);
    }

    /* failing fit */
    himg = hdrl_image_new(nx, ny);
    compute_strehl(himg, lam, m1, m2, pscale, pscale, .5, -1, -1);
    cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);

    hdrl_image_delete(himg);
}

/*
void test_strehl_data(int argc, char * argv[])
{
    for (size_t i = 2; i < (size_t)argc; i++) {
        hdrl_image * himg;
        cpl_image * img;
        hdrl_strehl_result r;
        cpl_frame * frm = cpl_frame_new();
        cpl_frame_set_filename(frm, argv[i]);
        cpl_size next = cpl_frame_get_nextensions(frm);
        cpl_frame_delete(frm);
        img = cpl_image_load(argv[i], CPL_TYPE_DOUBLE, 0, 0);
        double radius = atof(argv[1]); // arcsec
        if (next == 0) {

            double dmad;
            cpl_image_get_mad(img, &dmad);
			// signal/noise
            cpl_image * err = cpl_image_duplicate(img);
            cpl_image_multiply_scalar(err, 0);
            cpl_image_add_scalar(err, (dmad * CPL_MATH_STD_MAD));
            himg = hdrl_image_create(img, err);
            cpl_msg_info(cpl_func, "image error %g", (dmad * CPL_MATH_STD_MAD));
        }
        else {
            cpl_image * err = cpl_image_load(argv[i], CPL_TYPE_DOUBLE, 0, 1);
            himg = hdrl_image_create(img, err);
            cpl_msg_info(cpl_func, "using ext 1 as error");
        }
        if (strstr(argv[i], "_2.fits")) {
            r = compute_strehl(himg, 1.635e-6, 5.08/2, 5.08/2*0.36, 0.0331932,
                               0.0331932, radius, -1, -1);
        }
        else if (strstr(argv[i], "_4.fits")) {
            r = compute_strehl(himg, 1.635e-6, 5.08/2, 5.08/2*0.36,
                               0.0331932/2., 0.0331932/2., radius, -1, -1);
        }
        else if (strstr(argv[i], ".fits")) {
            cpl_msg_warning(cpl_func,"case .fits");
            cpl_image_save(hdrl_image_get_image(himg), "data.fits",
                           CPL_TYPE_FLOAT, NULL, CPL_IO_DEFAULT);
            cpl_image_save(hdrl_image_get_error(himg), "errs.fits",
                           CPL_TYPE_FLOAT, NULL, CPL_IO_DEFAULT);
            cpl_msg_info(cpl_func,"r1=%g r2=%g r3=%g",radius,-1.,-1.);
            cpl_msg_info(cpl_func,"m1=%g m2=%g pscale_x=%g pscale_y=%g",
                         8.0,1.12,0.01225,0.01225);
            r = compute_strehl(himg, 1.6e-6, 8.0/2, 1.12/2,
                               0.01225, 0.01225, radius, 1.5, 2.0);
        }
        else {
            cpl_msg_error(cpl_func, "Unknown pixelscale for %s", argv[i]);
            continue;
        }
        cpl_msg_info(cpl_func, "Strehl for %s: %g", argv[i], r.strehl_value.data);
        cpl_test_abs(r.strehl_value.data, 1.0, 0);
        cpl_image_delete(img);
    }
    

}
*/

/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of hdrl_image
 **/
/*----------------------------------------------------------------------------*/
int main(int argc, char * argv[])
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    hdrl_strehl_test_parlist();
    hdrl_strehl_test_null_input();
    hdrl_strehl_test_illegal_input();

    test_psf();
    test_strehl();
    test_strehl_with_bkg();

	cpl_msg_debug(cpl_func, "test_strehl_data only for command line "
			"(argc=%d, argv=%s). If you need to test images images "
			"via command line uncomment the function.",	argc, argv[0]);
/*
     test_strehl_data(argc, argv);
*/
    return cpl_test_end(0);
}
