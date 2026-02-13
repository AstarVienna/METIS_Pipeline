/*
 * This file is part of the ESO Common Pipeline Library
 * Copyright (C) 2001-2004,2014 European Southern Observatory
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

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <irplib_wlxcorr.h>

#include <irplib_wavecal_impl.h>

#include <cpl_plot.h>

#include <math.h>
#include <float.h>


/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_wlcalib_test Testing of the CPL Wavelength calibration
 */
/*----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
                            Private Function prototypes
 -----------------------------------------------------------------------------*/

static void irplib_wlxcorr_best_poly_test(void);
static void irplib_wlxcorr_best_poly_test_one(int, int, cpl_boolean, int, int);
static void irplib_wlxcorr_convolve_create_kernel_test(void);
static void irplib_wlxcorr_convolve_create_kernel_test_one(double, double);
static double irplib_wlcalib_lss(double, double, double);
static void irplib_wavecal_profile_compare(int, double, double);


/*----------------------------------------------------------------------------*/
/**
   @brief   Unit tests of wlcalib module
**/
/*----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                                  Main
 -----------------------------------------------------------------------------*/
int main(void)
{
    /* Initialize CPL + IRPLIB */
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    irplib_wavecal_profile_compare(100, 4.0, 4.0);
    irplib_wlxcorr_convolve_create_kernel_test();
    irplib_wlxcorr_best_poly_test();

    return cpl_test_end(0);
}


static void irplib_wlxcorr_best_poly_test(void)
{
    cpl_polynomial  *   poly;
    const cpl_boolean   do_bench = cpl_msg_get_level() <= CPL_MSG_INFO
        ? CPL_TRUE : CPL_FALSE;
    const int           spec_size = do_bench ? 1024 : 256;
    const int           nreps     = do_bench ? 3 : 1;
    const int           nsamples  = do_bench ? 30 : 10;


    /* 1st test: NULL input */
    poly = irplib_wlxcorr_best_poly(NULL, NULL, 1, NULL, NULL, 1, 1.0, 1.0,
                                    NULL, NULL, NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_null( poly );

#if 1
    /* 2nd test: Resampling of catalog lines */
    irplib_wlxcorr_best_poly_test_one(spec_size, spec_size*10, CPL_TRUE,
                                      nsamples, nreps);
#endif

    /* 3rd test: No resampling of catalog lines */
    irplib_wlxcorr_best_poly_test_one(spec_size, spec_size/50,  CPL_FALSE,
                                      nsamples, nreps);
}

static void irplib_wlxcorr_best_poly_test_one(int spec_size, int cat_size,
                                              cpl_boolean do_resample,
                                              int nsamples, int nreps)
{
    const int           degree     = 2;
    cpl_vector      *   spectrum   = cpl_vector_new(spec_size);
    cpl_bivector    *   catalog    = cpl_bivector_new(cat_size);
    cpl_polynomial  *   true_poly  = cpl_polynomial_new(1);
    cpl_polynomial  *   guess_poly = cpl_polynomial_new(1);
    cpl_vector      *   wl_err     = cpl_vector_new(degree+1);
    double              xc;
    const double        slitw = 2.0;
    const double        fwhm = 2.0;
    const double        xtrunc = 0.5 * slitw + 5.0 * fwhm * CPL_MATH_SIG_FWHM;
    const double        rel_error = 0.05; /* Introduce error */

    /* A black-body with T=253K should emit mostly in the range [2;50] micron */
    const double        b_true = 2e-6;
    const double        a_true = 48e-6 / spec_size;

    const double        a_error = a_true * rel_error;
    const double        b_error = b_true * rel_error;
    const double        a = a_true + a_error;
    const double        b = b_true + b_error;
    double              wl_errmax;
    cpl_size            pow_ind;
    int                 i;
    FILE              * stream = cpl_msg_get_level() > CPL_MSG_INFO
        ? fopen("/dev/null", "a") : stdout;


    cpl_test_nonnull( stream );

    /* First guess P(x) = ax + b */
    /* The true and distorted polynomials */
    pow_ind = 1;
    cpl_polynomial_set_coeff(true_poly,  &pow_ind, a_true);
    cpl_polynomial_set_coeff(guess_poly, &pow_ind, a);
    pow_ind = 0;
    cpl_polynomial_set_coeff(true_poly,  &pow_ind, b_true);
    cpl_polynomial_set_coeff(guess_poly, &pow_ind, b);

    cpl_msg_info(cpl_func, "First guess polynomial:");
    cpl_polynomial_dump(guess_poly, stream);

    /* Try also to shift the guess of the solution */
    cpl_test_zero(cpl_polynomial_shift_1d(guess_poly, 0, 25.0));

    cpl_msg_info(cpl_func, "True polynomial:");
    cpl_polynomial_dump(true_poly, stream);


    if (do_resample) {
        const double    temp_bb = 253.0;
        cpl_vector * evalpoints = cpl_vector_new(spec_size);

        /* Wavelengths of the spectrum */
        cpl_vector_fill_polynomial(evalpoints, true_poly, 1.0, 1.0);

        /* Catalog */
        /* The sampled profile is a black body radiation */
        cpl_vector_fill_polynomial(cpl_bivector_get_x(catalog), true_poly,
                                   -1.0, 1.5 * spec_size / cat_size);

        cpl_photom_fill_blackbody(cpl_bivector_get_y(catalog), CPL_UNIT_LESS,
                                  cpl_bivector_get_x_const(catalog),
                                  CPL_UNIT_LENGTH, temp_bb);

        cpl_photom_fill_blackbody(spectrum, CPL_UNIT_LESS,
                                  evalpoints, CPL_UNIT_LENGTH, temp_bb);

        cpl_vector_delete(evalpoints);

    } else {
        /* Place some lines with different intensities */
        double * dx = cpl_bivector_get_x_data(catalog);
        double * dy = cpl_bivector_get_y_data(catalog);

        for (i = 0; i < cat_size; i++) {
            const double wli = cpl_polynomial_eval_1d(true_poly, 3.0 * i * i
                                                      -10.0, NULL);

            dx[i] = wli;
            dy[i] = sin(i * CPL_MATH_PI / cat_size);

        }

        irplib_vector_fill_line_spectrum_model(spectrum, NULL, NULL, true_poly,
                                               catalog, slitw, fwhm, xtrunc,
                                               0, CPL_FALSE, CPL_FALSE, NULL);
        cpl_test_error(CPL_ERROR_NONE);
    }

    /* FIXME: Add some random noise to the spectrum */
    
    if (cpl_msg_get_level() <= CPL_MSG_DEBUG) {
        cpl_plot_bivector( "", "t 'Catalog' w lines", "", catalog);
        cpl_plot_vector( "", "t 'Spectrum' w lines", "", spectrum);
    }


    /* Error */
    /* Compute an error bound certain to include to true solution */
    wl_errmax = cpl_polynomial_eval_1d(guess_poly, spec_size, NULL)
        - cpl_polynomial_eval_1d(true_poly, spec_size, NULL);
    cpl_vector_fill(wl_err, 2.0 * wl_errmax);

    /* Multiple calls for bench-marking */

    for (i=0; i < nreps; i++) {
        cpl_table      * wl_res;
        cpl_vector     * xcorrs;
        cpl_polynomial * poly
            = irplib_wlxcorr_best_poly(spectrum, catalog, degree,
                                       guess_poly, wl_err, nsamples,
                                       slitw, fwhm, &xc, &wl_res, &xcorrs);
        cpl_test_nonnull(poly);
        cpl_test_error(CPL_ERROR_NONE);

        if (i == 0 && poly != NULL) {
            if (cpl_msg_get_level() <= CPL_MSG_DEBUG) {
                const char * labels[] = {IRPLIB_WLXCORR_COL_WAVELENGTH,
                                         IRPLIB_WLXCORR_COL_CAT_INIT,
                                         IRPLIB_WLXCORR_COL_CAT_FINAL,
                                         IRPLIB_WLXCORR_COL_OBS};

                cpl_plot_vector( "", "t 'X corr values' w lines", "", xcorrs);

                cpl_test_zero(cpl_plot_columns("", "", "", wl_res, labels, 4));
            }

            cpl_msg_info(cpl_func, "Corrected polynomial:");
            cpl_polynomial_dump(poly, stream);

            /* Corrected polynomial must be monotone, with same sign
               as a_true. */ 
            cpl_test_zero(cpl_polynomial_derivative(poly, 0));
            cpl_test_leq(0.0, a_true * cpl_polynomial_eval_1d(poly, 1.0, NULL));
            cpl_test_leq(0.0, a_true
                         * cpl_polynomial_eval_1d(poly, 0.5 * spec_size, NULL));
            cpl_test_leq(0.0, a_true
                         * cpl_polynomial_eval_1d(poly, spec_size, NULL));

            cpl_test_error(CPL_ERROR_NONE);

        }

        cpl_table_delete(wl_res);
        cpl_vector_delete(xcorrs);
        cpl_polynomial_delete(poly);
    }

    cpl_vector_delete(wl_err);
    cpl_vector_delete(spectrum);
    cpl_bivector_delete(catalog);
    cpl_polynomial_delete(true_poly);
    cpl_polynomial_delete(guess_poly);
    cpl_test_error(CPL_ERROR_NONE);

    if (stream != stdout) cpl_test_zero( fclose(stream) );

    return;
}


static void irplib_wlxcorr_convolve_create_kernel_test_one(double slitw,
                                                           double fwhm)
{

    cpl_vector * kernel;
    double       sum = 0.0;
    /* Maximum value of profile */
    const double maxval = irplib_wlcalib_lss(0.0, slitw, fwhm);
    double       prev = maxval;
    int          n, i;

    cpl_msg_info(cpl_func, "Slit-width=%g, FWHM=%g", slitw, fwhm);

    kernel = irplib_wlxcorr_convolve_create_kernel(0.0, fwhm);

    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    cpl_test_null(kernel);

    kernel = irplib_wlxcorr_convolve_create_kernel(slitw, 0.0);

    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    cpl_test_null(kernel);

    kernel = irplib_wlxcorr_convolve_create_kernel(slitw, fwhm);

    cpl_test_nonnull(kernel);

    n = cpl_vector_get_size(kernel);

    for (i = 0; i < n; i++) {
        const double val = cpl_vector_get(kernel, i);
        sum += i ? 2.0*val : val; /* Non-central elements twice */

        /* Profile consists of non-negative values */
        cpl_test_leq(0.0, val);

        /* The max of the profile is less than maxval and decreases */
        cpl_test_leq(val, prev);

        if (i > 0) {
            /* The profile at i is less than the continuous profile at
               i - 0.5, and greater than that at i + 0.5 */
            cpl_test_leq(val, irplib_wlcalib_lss(i - 0.5, slitw, fwhm));
            cpl_test_leq(irplib_wlcalib_lss(i + 0.5, slitw, fwhm), val);
        }

        /* The profile has a FWHM (sligthly) greater than slitw */
        if ((double)i < 0.5 * slitw) {
            /* Thus if x is less than half the slit width, then
               the value has to be greater than half the maximum */
            cpl_test_leq(0.5 * maxval, val);
        } else if (val < 0.5 * maxval) {
            /* On the other hand, if the value is less than the maximum,
               then x must exceed half the slitw */
            cpl_test_leq(0.5*slitw, (double)i);
        }

        prev = val;
    }

    /* Integral is supposed to be 1 */
    cpl_test_abs(sum, 1.0, 1e-5); /* FIXME: Improve tolerance */

    if (cpl_msg_get_level() <= CPL_MSG_DEBUG) {
        char * title = cpl_sprintf("t 'LSS profile, slitw=%g, fwhm=%g' "
                                   "w linespoints", slitw, fwhm);
        cpl_plot_vector("set grid;", title, "", kernel);
        cpl_free(title);
    }

    cpl_vector_delete(kernel);
}

static void irplib_wlxcorr_convolve_create_kernel_test(void)
{

    irplib_wlxcorr_convolve_create_kernel_test_one(0.86, 2.0);
    irplib_wlxcorr_convolve_create_kernel_test_one(1.72, 3.0);
    irplib_wlxcorr_convolve_create_kernel_test_one(40.0, 2.0);
    irplib_wlxcorr_convolve_create_kernel_test_one(3.0, 40.0);

}


/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    The intensity of the line profile at distance x
  @param    x      x
  @param    slitw  The slit width
  @param    fwhm   The FWHM of the (Gaussian) transfer function
  @return   The intensity of the line profile at distance x

 */
/*----------------------------------------------------------------------------*/
static double irplib_wlcalib_lss(double x, double slitw, double fwhm)
{
  const double sigmasqrt2 = fwhm * CPL_MATH_SIG_FWHM * CPL_MATH_SQRT2;
  const double result = 0.5 / slitw *
      (erf((x+0.5*slitw)/sigmasqrt2) - erf((x-0.5*slitw)/sigmasqrt2));

  cpl_test_lt(0.0, slitw);
  cpl_test_lt(0.0, sigmasqrt2);

  /* Protect against round-off (on SunOS 5.8) */
  return result < 0.0 ? 0.0 : result;

}


/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Compare the standard and fast profiles
  @param  spec_size Number of points in the spectrum
  @param  slitw     The slit width
  @param  fwhm      The FWHM of the (Gaussian) transfer function

 */
/*----------------------------------------------------------------------------*/
static void irplib_wavecal_profile_compare(int spec_size, double slitw,
                                           double fwhm)
{

    cpl_vector     * spectrum1  = cpl_vector_new(spec_size);
    cpl_vector     * spectrum2  = cpl_vector_new(spec_size);
    cpl_bivector   * catalog    = cpl_bivector_new(2);
    cpl_polynomial * dispersion = cpl_polynomial_new(1);
    const double     a = 1.0;
    const double     b = 100.0;
    const double     xtrunc = 0.5 * slitw + 2.0 * fwhm * CPL_MATH_SIG_FWHM;
    double           mean;
    cpl_error_code   error;
    cpl_size         pow_ind;


    pow_ind = 1;
    cpl_polynomial_set_coeff(dispersion, &pow_ind, a);
    pow_ind = 0;
    cpl_polynomial_set_coeff(dispersion, &pow_ind, b);

    cpl_vector_set(cpl_bivector_get_x(catalog), 0, b + spec_size / 3.0);
    cpl_vector_set(cpl_bivector_get_y(catalog), 0, 100);

    cpl_vector_set(cpl_bivector_get_x(catalog), 1, b + spec_size / 1.5);
    cpl_vector_set(cpl_bivector_get_y(catalog), 1, 100);

    cpl_test_error(CPL_ERROR_NONE);

    error = irplib_vector_fill_line_spectrum_model(spectrum1, NULL, NULL,
                                                   dispersion, catalog, slitw,
                                                   fwhm, xtrunc, 0, CPL_FALSE,
                                                   CPL_FALSE, NULL);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(error, CPL_ERROR_NONE);


    error = irplib_vector_fill_line_spectrum_model(spectrum2, NULL, NULL,
                                                   dispersion, catalog, slitw,
                                                   fwhm, xtrunc, 0, CPL_TRUE,
                                                   CPL_FALSE, NULL);
    
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(error, CPL_ERROR_NONE);

    if (cpl_msg_get_level() <= CPL_MSG_DEBUG) {
        error = cpl_plot_vector("set grid;", "t 'Spectrum' w lines", "",
                                spectrum1);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_eq(error, CPL_ERROR_NONE);
        error = cpl_plot_vector("set grid;", "t 'Spectrum' w lines", "",
                                spectrum2);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_eq(error, CPL_ERROR_NONE);
    }

    cpl_vector_subtract(spectrum1, spectrum2);
    mean = cpl_vector_get_mean(spectrum1);
    if (mean != 0.0) {
        cpl_msg_info(cpl_func, "Error: %g", mean);
        if (cpl_msg_get_level() <= CPL_MSG_DEBUG) {
            error = cpl_plot_vector("set grid;", "t 'Spectrum error' w lines",
                                    "", spectrum1);
            cpl_test_error(CPL_ERROR_NONE);
            cpl_test_eq(error, CPL_ERROR_NONE);
        }
    }

    cpl_polynomial_delete(dispersion);
    cpl_vector_delete(spectrum1);
    cpl_vector_delete(spectrum2);
    cpl_bivector_delete(catalog);

    cpl_test_error(CPL_ERROR_NONE);

}
