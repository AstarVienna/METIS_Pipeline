/*
 * This file is part of the IRPLIB package
 * Copyright (C) 2002,2003,2014 European Southern Observatory
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "irplib_wavecal_impl.h"

#include "irplib_wlxcorr.h"

#include <cpl.h>

#include <math.h>
#include <string.h>

/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_wlxcorr     Wavelength Cross correlation w. plotting
 *
 * @par Synopsis:
 * @code
 *   #include "irplib_wlxcorr.h"
 * @endcode
 *
 */
/*----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                           Defines
 -----------------------------------------------------------------------------*/

#ifndef inline
#define inline /* inline */
#endif

#define IRPLIB_MAX(A,B) ((A) > (B) ? (A) : (B))
#define IRPLIB_MIN(A,B) ((A) < (B) ? (A) : (B))

#define IRPLIB_PTR_SWAP(a,b)                                               \
    do { void * irplib_ptr_swap =(a);(a)=(b);(b)=irplib_ptr_swap; } while (0)

/*-----------------------------------------------------------------------------
                           Private functions
 -----------------------------------------------------------------------------*/

static void irplib_wlxcorr_estimate(cpl_vector *, cpl_vector *,
                                    const cpl_vector *,
                                    const cpl_bivector *,
                                    const cpl_vector *,
                                    const cpl_polynomial *,
                                    double, double);

static int irplib_wlxcorr_signal_resample(cpl_vector *, const cpl_vector *, 
        const cpl_bivector *) ;
static cpl_error_code cpl_vector_fill_lss_profile_symmetric(cpl_vector *,
                                                            double, double);
static cpl_error_code irplib_wlcalib_fill_spectrum(cpl_vector *,
                                                const cpl_bivector *,
                                                const cpl_vector *,
                                                const cpl_polynomial *, int);

static cpl_boolean irplib_wlcalib_is_lines(const cpl_vector *,
                                        const cpl_polynomial *,
                                        int, double);

/**@{*/


/*----------------------------------------------------------------------------*/
/**
  @brief    Find the best polynomial in a given range
  @param    spectrum        The spectrum vector
  @param    lines_catalog   The lines catalog
  @param    degree          The polynomial degree
  @param    guess_poly      Guess Dispersion Polynomial 
  @param    wl_error        Search range around the anchor points
  @param    nsamples        Number of samples around the anchor points
  @param    slitw           The slit width
  @param    fwhm            The spectral FWHM [pixel]
  @param    xc              Cross-correlation factor (returned)
  @param    wlres           The table with the calibration results or NULL 
  @param    xcorrs          The vector with the correlation values or NULL
  @return   the best polynomial or NULL in error case
  @note *wlres may be NULL also when no error code is set by the function

  wl_error must be of size degree+1.
  The returned polynomial must be deallocated with cpl_polynomial_delete().

  On success:
  If wlres is non-NULL, *wlres points to a table which must be deallocated
  with cpl_table_delete(), and if xcorrs is non-NULL, *xcorrs points to a
  vector which must be deallocated with cpl_vector_delete().

  The complexity in terms of model spectra creation is O(N^D), where N is
  nsamples and D is the length of wl_error.
 
  Possible #_cpl_error_code_ set in this function:
  - CPL_ERROR_NULL_INPUT if (one of) the input pointer(s) is NULL
  - CPL_ERROR_ILLEGAL_INPUT if the wl_error vector has not the proper
    size (with respect to the degree) of if the degree is outside the
    allowed range
  - CPL_ERROR_ILLEGAL_OUTPUT if the spectral table cannot be generated
    from the solution polynomial
 */
/*----------------------------------------------------------------------------*/
cpl_polynomial * irplib_wlxcorr_best_poly(const cpl_vector     * spectrum,
                                          const cpl_bivector   * lines_catalog,
                                          int                    degree,
                                          const cpl_polynomial * guess_poly,
                                          const cpl_vector     * wl_error,
                                          int                    nsamples,
                                          double                 slitw,
                                          double                 fwhm,
                                          double               * xc,
                                          cpl_table           ** wlres,
                                          cpl_vector          ** xcorrs)
{
    const int         spec_sz = cpl_vector_get_size(spectrum);
    const int         nfree   = cpl_vector_get_size(wl_error);
    int               ntests  = 1;
    cpl_vector      * model;
    cpl_vector      * vxc;
    cpl_vector      * init_pts_wl;
    cpl_matrix      * init_pts_x;
    cpl_vector      * pts_wl;
    cpl_vector      * vxcorrs;
    cpl_vector      * conv_kernel = NULL;    
    cpl_polynomial  * poly_sol;
    cpl_polynomial  * poly_candi;
    const double    * pwl_error = cpl_vector_get_data_const(wl_error); 
    const double    * dxc;
    cpl_size          degree_loc ;
    const cpl_boolean symsamp = CPL_TRUE; /* init_pts_x is symmetric */
    const cpl_boolean is_lines
        = irplib_wlcalib_is_lines(cpl_bivector_get_x_const(lines_catalog),
                               guess_poly, spec_sz, 1.0);
    int               i;

    /* FIXME: Need mode parameter for catalogue type (lines <=> profile) */

    /* In case of failure */
    if (wlres  != NULL) *wlres  = NULL;
    if (xcorrs != NULL) *xcorrs = NULL;

    /* Useful for knowing if resampling is used */
    cpl_msg_debug(cpl_func, "Checking %d^%d dispersion polynomials (slitw=%g, "
                  "fwhm=%g) against %d-point observed spectrum with%s "
                  "catalog resampling", nsamples, nfree, slitw, fwhm, spec_sz,
                  is_lines ? "out" : "");

    cpl_ensure(xc            != NULL, CPL_ERROR_NULL_INPUT,    NULL);
    *xc = -1.0;
    cpl_ensure(spectrum      != NULL,  CPL_ERROR_NULL_INPUT,    NULL);
    cpl_ensure(lines_catalog != NULL, CPL_ERROR_NULL_INPUT,    NULL);
    cpl_ensure(guess_poly    != NULL, CPL_ERROR_NULL_INPUT,    NULL);
    cpl_ensure(wl_error      != NULL, CPL_ERROR_NULL_INPUT,    NULL);
    cpl_ensure(nfree         >= 2,    CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(nsamples      >  0,    CPL_ERROR_ILLEGAL_INPUT, NULL);
    /* FIXME: degree is redundant */
    cpl_ensure(1 + degree   == nfree, CPL_ERROR_ILLEGAL_INPUT, NULL);

    cpl_ensure(cpl_polynomial_get_dimension(guess_poly) == 1,
               CPL_ERROR_ILLEGAL_INPUT, NULL);

    if (nsamples > 1) {
        /* Search place must consist of more than one point */
        /* FIXME: The bounds should probably not be negative */
        for (i = 0; i < nfree; i++) {
            if (pwl_error[i] != 0.0) break;
        }
        cpl_ensure(i < nfree, CPL_ERROR_ILLEGAL_INPUT, NULL);
    }
 
    if (!is_lines) {
        /* Create the convolution kernel */
        conv_kernel = irplib_wlxcorr_convolve_create_kernel(slitw, fwhm);
        cpl_ensure(conv_kernel   != NULL, CPL_ERROR_ILLEGAL_INPUT, NULL);
    }

    /* Create initial test points */
    init_pts_x  = cpl_matrix_new(1, nfree);
    init_pts_wl = cpl_vector_new(nfree);
    pts_wl      = cpl_vector_new(nfree);
    for (i = 0; i < nfree; i++) {
        const double xpos  = spec_sz * i / (double)degree;
        const double wlpos = cpl_polynomial_eval_1d(guess_poly, xpos, NULL)
            - 0.5 * pwl_error[i];

        cpl_matrix_set(init_pts_x, 0, i, xpos);
        cpl_vector_set(init_pts_wl,   i, wlpos);

        ntests *= nsamples; /* Count number of tests */

    }

    vxcorrs = xcorrs != NULL ? cpl_vector_new(ntests) : NULL;

    poly_sol   = cpl_polynomial_new(1);
    poly_candi = cpl_polynomial_new(1);
    model = cpl_vector_new(spec_sz);
    vxc = cpl_vector_new(1);
    dxc = cpl_vector_get_data_const(vxc);
   
    /* Create the polynomial candidates and estimate them */
    for (i=0; i < ntests; i++) {
        int    idiv = i;
        int    deg;

        /* Update wavelength at one anchor point - and reset wavelengths
           to their default for any anchor point(s) at higher wavelengths */
        for (deg = degree; deg >= 0; deg--, idiv /= nsamples) {
            const int imod = idiv % nsamples;
            const double wlpos = cpl_vector_get(init_pts_wl, deg)
                               + imod * pwl_error[deg] / nsamples;

            /* FIXME: If wlpos causes pts_wl to be non-increasing, the
               solution will be non-physical with no need for evaluation.
               (*xc could be set to -1 in this case). */
            cpl_vector_set(pts_wl, deg, wlpos);

            if (imod > 0) break;
        }

        /* Generate */
        degree_loc = (cpl_size)degree ;
        cpl_polynomial_fit(poly_candi, init_pts_x, &symsamp, pts_wl,
                           NULL, CPL_FALSE, NULL, &degree_loc);
        /* *** Estimate *** */
        irplib_wlxcorr_estimate(vxc, model, spectrum, lines_catalog,
                                conv_kernel, poly_candi, slitw, fwhm);
        if (vxcorrs != NULL) cpl_vector_set(vxcorrs, i, *dxc);
        if (*dxc > *xc) {
            /* Found a better solution */
            *xc = *dxc;
            IRPLIB_PTR_SWAP(poly_sol, poly_candi);
        }
    }

    cpl_vector_delete(model);
    cpl_vector_delete(vxc);
    cpl_vector_delete(conv_kernel);
    cpl_vector_delete(pts_wl);
    cpl_matrix_delete(init_pts_x);
    cpl_vector_delete(init_pts_wl);
    cpl_polynomial_delete(poly_candi);

#ifdef CPL_WLCALIB_FAIL_ON_CONSTANT
    /* FIXME: */
    if (cpl_polynomial_get_degree(poly_sol) == 0) {
        cpl_polynomial_delete(poly_sol);
        cpl_vector_delete(vxcorrs);
        *xc = 0.0;
        cpl_error_set_message_macro(cpl_func, CPL_ERROR_ILLEGAL_OUTPUT,
                                    __FILE__, __LINE__, "Found a constant "
                                    "dispersion");
            cpl_errorstate_dump(prestate, CPL_FALSE, NULL);
        return NULL;
    }
#endif
    
    if (wlres != NULL) {
        /* FIXME: A failure in the table creation is not considered a failure
           of the whole function call (although all outputs may be useless) */

        cpl_errorstate prestate = cpl_errorstate_get();
        /* Create the spc_table  */
        *wlres = irplib_wlxcorr_gen_spc_table(spectrum, lines_catalog, slitw,
                                              fwhm, guess_poly, poly_sol);
        if (*wlres == NULL) {
            cpl_polynomial_delete(poly_sol);
            cpl_vector_delete(vxcorrs);
            *xc = -1.0;
            cpl_error_set_message_macro(cpl_func, CPL_ERROR_ILLEGAL_OUTPUT,
                                        __FILE__, __LINE__, "Cannot generate "
                                        "infos table");
            /* cpl_errorstate_dump(prestate, CPL_FALSE, NULL); */
            cpl_errorstate_set(prestate);
            return NULL;
        }
    } 
    
    if (xcorrs != NULL) {
        *xcorrs = vxcorrs;
    } else {
        /* assert(vxcorrs == NULL); */
    }

    return poly_sol;
}

/*
TODO :
    Merge irplib_wlxcorr_best_poly_prop() with irplib_wlxcorr_best_poly() by
    adding a new parameter.
    Need to coordinate with all pipelines.
*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Find the best polynomial in a given range, propagating input
  @param    spectrum        The spectrum vector
  @param    lines_catalog   The lines catalog
  @param    degree          The polynomial degree
  @param    guess_poly      Guess Dispersion Polynomial 
  @param    wl_error        Search range around the anchor points
  @param    nsamples        Number of samples around the anchor points
  @param    slitw           The slit width
  @param    fwhm            The spectral FWHM [pixel]
  @param    xc              Cross-correlation factor (returned)
  @param    wlres           The table with the calibration results or NULL 
  @param    xcorrs          The vector with the correlation values or NULL
  @return   the best polynomial or NULL in error case
  @note *wlres may be NULL also when no error code is set by the function

  Same as irplib_wlxcorr_best_poly() above, except:
  The degree can be 0.
  The degree from the guess_poly can be higher than the degree that is
  asked for. In that case the output degree is the same as of guess_poly
  and the caller needs to accept that.
  Also in that case this function evaluates the full guess_poly for the
  guess and it keeps the coefficients >degree in the return.

 */
/*----------------------------------------------------------------------------*/
cpl_polynomial * irplib_wlxcorr_best_poly_prop(const cpl_vector     * spectrum,
                                          const cpl_bivector   * lines_catalog,
                                          int                    degree,
                                          const cpl_polynomial * guess_poly,
                                          const cpl_vector     * wl_error,
                                          int                    nsamples,
                                          double                 slitw,
                                          double                 fwhm,
                                          double               * xc,
                                          cpl_table           ** wlres,
                                          cpl_vector          ** xcorrs)
{
    const int         spec_sz = cpl_vector_get_size(spectrum);
    const int         nfree   = cpl_vector_get_size(wl_error);
    int               ntests  = 1;
    cpl_vector      * model;
    cpl_vector      * vxc;
    cpl_vector      * init_pts_wl;
    cpl_matrix      * init_pts_x;
    cpl_vector      * pts_wl;
    cpl_vector      * vxcorrs;
    cpl_vector      * conv_kernel = NULL;    
    cpl_polynomial  * poly_sol;
    cpl_polynomial  * poly_candi;
    const double    * pwl_error = cpl_vector_get_data_const(wl_error); 
    const double    * dxc;
    cpl_size          degree_loc ;
    const cpl_boolean symsamp = CPL_TRUE; /* init_pts_x is symmetric */
    const cpl_boolean is_lines
        = irplib_wlcalib_is_lines(cpl_bivector_get_x_const(lines_catalog),
                               guess_poly, spec_sz, 1.0);
    int               i;

    /* FIXME: Need mode parameter for catalogue type (lines <=> profile) */

    /* In case of failure */
    if (wlres  != NULL) *wlres  = NULL;
    if (xcorrs != NULL) *xcorrs = NULL;

    /* Useful for knowing if resampling is used */
    cpl_msg_debug(cpl_func, "Checking %d^%d dispersion polynomials (slitw=%g, "
                  "fwhm=%g) against %d-point observed spectrum with%s "
                  "catalog resampling", nsamples, nfree, slitw, fwhm, spec_sz,
                  is_lines ? "out" : "");

    cpl_ensure(xc            != NULL, CPL_ERROR_NULL_INPUT,    NULL);
    *xc = -1.0;
    cpl_ensure(spectrum      != NULL,  CPL_ERROR_NULL_INPUT,    NULL);
    cpl_ensure(lines_catalog != NULL, CPL_ERROR_NULL_INPUT,    NULL);
    cpl_ensure(guess_poly    != NULL, CPL_ERROR_NULL_INPUT,    NULL);
    cpl_ensure(wl_error      != NULL, CPL_ERROR_NULL_INPUT,    NULL);
    cpl_ensure(nfree         >= 1,    CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(nsamples      >  0,    CPL_ERROR_ILLEGAL_INPUT, NULL);
    /* FIXME: degree is redundant */
    cpl_ensure(1 + degree   == nfree, CPL_ERROR_ILLEGAL_INPUT, NULL);

    cpl_ensure(cpl_polynomial_get_dimension(guess_poly) == 1,
               CPL_ERROR_ILLEGAL_INPUT, NULL);

    if (nsamples > 1) {
        /* Search place must consist of more than one point */
        /* FIXME: The bounds should probably not be negative */
        for (i = 0; i < nfree; i++) {
            if (pwl_error[i] != 0.0) break;
        }
        cpl_ensure(i < nfree, CPL_ERROR_ILLEGAL_INPUT, NULL);
    }
 
    if (!is_lines) {
        /* Create the convolution kernel */
        conv_kernel = irplib_wlxcorr_convolve_create_kernel(slitw, fwhm);
        cpl_ensure(conv_kernel   != NULL, CPL_ERROR_ILLEGAL_INPUT, NULL);
    }

    /* Create initial test points */
    init_pts_x  = cpl_matrix_new(1, nfree);
    init_pts_wl = cpl_vector_new(nfree);
    pts_wl      = cpl_vector_new(nfree);
    const double degree_loc2 = degree == 0 ? 1 : (double)degree ;
    for (i = 0; i < nfree; i++) {
        const double xpos  = spec_sz * i / degree_loc2;
        const double wlpos = cpl_polynomial_eval_1d(guess_poly, xpos, NULL)
            - 0.5 * pwl_error[i];

        cpl_matrix_set(init_pts_x, 0, i, xpos);
        cpl_vector_set(init_pts_wl,   i, wlpos);

        ntests *= nsamples; /* Count number of tests */

    }

    vxcorrs = xcorrs != NULL ? cpl_vector_new(ntests) : NULL;

    poly_sol   = cpl_polynomial_new(1);
    poly_candi = cpl_polynomial_new(1);
    model = cpl_vector_new(spec_sz);
    vxc = cpl_vector_new(1);
    dxc = cpl_vector_get_data_const(vxc);
   
    /* Create the polynomial candidates and estimate them */
    for (i=0; i < ntests; i++) {
        int    idiv = i;
        int    deg;

        /* Update wavelength at one anchor point - and reset wavelengths
           to their default for any anchor point(s) at higher wavelengths */
        for (deg = degree; deg >= 0; deg--, idiv /= nsamples) {
            const int imod = idiv % nsamples;
            const double wlpos = cpl_vector_get(init_pts_wl, deg)
                               + imod * pwl_error[deg] / nsamples;

            /* FIXME: If wlpos causes pts_wl to be non-increasing, the
               solution will be non-physical with no need for evaluation.
               (*xc could be set to -1 in this case). */
            cpl_vector_set(pts_wl, deg, wlpos);

            if (imod > 0) break;
        }

        /* Generate */
        degree_loc = (cpl_size)degree ;
        cpl_polynomial_fit(poly_candi, init_pts_x, &symsamp, pts_wl,
                           NULL, CPL_FALSE, NULL, &degree_loc);

        /* Use the degrees of the input guess */
        degree_loc = cpl_polynomial_get_degree(guess_poly);
        for (cpl_size power = degree + 1; power < degree_loc + 1; power++){
            cpl_polynomial_set_coeff(poly_candi, &power, 
                cpl_polynomial_get_coeff(guess_poly, &power));
        }

        /* *** Estimate *** */
        irplib_wlxcorr_estimate(vxc, model, spectrum, lines_catalog,
                                conv_kernel, poly_candi, slitw, fwhm);
        if (vxcorrs != NULL) cpl_vector_set(vxcorrs, i, *dxc);
        if (*dxc > *xc) {
            /* Found a better solution */
            *xc = *dxc;
            IRPLIB_PTR_SWAP(poly_sol, poly_candi);
        }
    }

    /* This trunctates poly_sol to degree, not wanted for propagation */
    /* degree_loc = cpl_polynomial_get_degree(guess_poly);
    for (cpl_size power = degree_loc; power > degree; power--){
        cpl_polynomial_set_coeff(poly_sol, &power, 0);
    } 
    */
   
    cpl_vector_delete(model);
    cpl_vector_delete(vxc);
    cpl_vector_delete(conv_kernel);
    cpl_vector_delete(pts_wl);
    cpl_matrix_delete(init_pts_x);
    cpl_vector_delete(init_pts_wl);
    cpl_polynomial_delete(poly_candi);

#ifdef CPL_WLCALIB_FAIL_ON_CONSTANT
    /* FIXME: */
    if (cpl_polynomial_get_degree(poly_sol) == 0) {
        cpl_polynomial_delete(poly_sol);
        cpl_vector_delete(vxcorrs);
        *xc = 0.0;
        cpl_error_set_message_macro(cpl_func, CPL_ERROR_ILLEGAL_OUTPUT,
                                    __FILE__, __LINE__, "Found a constant "
                                    "dispersion");
            cpl_errorstate_dump(prestate, CPL_FALSE, NULL);
        return NULL;
    }
#endif
    
    if (wlres != NULL) {
        /* FIXME: A failure in the table creation is not considered a failure
           of the whole function call (although all outputs may be useless) */

        cpl_errorstate prestate = cpl_errorstate_get();
        /* Create the spc_table  */
        *wlres = irplib_wlxcorr_gen_spc_table(spectrum, lines_catalog, slitw,
                                              fwhm, guess_poly, poly_sol);
        if (*wlres == NULL) {
            cpl_polynomial_delete(poly_sol);
            cpl_vector_delete(vxcorrs);
            *xc = -1.0;
            cpl_error_set_message_macro(cpl_func, CPL_ERROR_ILLEGAL_OUTPUT,
                                        __FILE__, __LINE__, "Cannot generate "
                                        "infos table");
            /* cpl_errorstate_dump(prestate, CPL_FALSE, NULL); */
            cpl_errorstate_set(prestate);
            return NULL;
        }
    } 
    
    if (xcorrs != NULL) {
        *xcorrs = vxcorrs;
    } else {
        /* assert(vxcorrs == NULL); */
    }

    return poly_sol;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Generate the infos table
  @param    spectrum        The spectrum vector
  @param    lines_catalog   The lines catalog
  @param    slitw           The slit width
  @param    fwhm            The spectral FWHM [pixel]
  @param    guess_poly      Guess Dispersion Polynomial 
  @param    corr_poly       Corrected Dispersion Polynomial
  @return   the table with the results of the calibration or NULL in error case

  The returned table must be deallocated with cpl_table_delete().
  
  Possible #_cpl_error_code_ set in this function:
  - CPL_ERROR_NULL_INPUT if (one of) the input pointer(s) is NULL
  - CPL_ERROR_ILLEGAL_INPUT if the spectrum cannot be generated from the
    catalog or if the convolution kernel cannot be created
 */
/*----------------------------------------------------------------------------*/
cpl_table * irplib_wlxcorr_gen_spc_table(
        const cpl_vector        *   spectrum,
        const cpl_bivector      *   lines_catalog,
        double                      slitw,
        double                      fwhm,
        const cpl_polynomial    *   guess_poly,
        const cpl_polynomial    *   corr_poly)
{

    cpl_vector      *   conv_kernel = NULL;
    cpl_bivector    *   gen_init ;
    cpl_bivector    *   gen_corr ;
    cpl_table       *   spc_table ;
    const double    *   pgen ;
    const double        xtrunc = 0.5 * slitw + 5.0 * fwhm * CPL_MATH_SIG_FWHM;
    const int           spec_sz = cpl_vector_get_size(spectrum);
    const cpl_boolean   guess_resamp
        = !irplib_wlcalib_is_lines(cpl_bivector_get_x_const(lines_catalog),
                                guess_poly, spec_sz, 1.0);
    const cpl_boolean   corr_resamp
        = !irplib_wlcalib_is_lines(cpl_bivector_get_x_const(lines_catalog),
                                corr_poly, spec_sz, 1.0);
    cpl_error_code      error;

    cpl_msg_debug(cpl_func, "Table for guess dispersion polynomial (slitw=%g, "
                  "fwhm=%g) with %d-point observed spectrum with%s catalog re"
                  "sampling", slitw, fwhm, spec_sz, guess_resamp ? "out" : "");
    cpl_msg_debug(cpl_func, "Table for corr. dispersion polynomial (slitw=%g, "
                  "fwhm=%g) with %d-point observed spectrum with%s catalog re"
                  "sampling", slitw, fwhm, spec_sz, corr_resamp ? "out" : "");

    /* Test inputs */
    cpl_ensure(spectrum, CPL_ERROR_NULL_INPUT, NULL) ;
    cpl_ensure(lines_catalog, CPL_ERROR_NULL_INPUT, NULL) ;
    cpl_ensure(guess_poly, CPL_ERROR_NULL_INPUT, NULL) ;
    cpl_ensure(corr_poly, CPL_ERROR_NULL_INPUT, NULL) ;

    /* Create the convolution kernel */
    if (guess_resamp || corr_resamp) {
        conv_kernel = irplib_wlxcorr_convolve_create_kernel(slitw, fwhm);

        if (conv_kernel == NULL) {
            cpl_error_set_message_macro(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                                        __FILE__, __LINE__, "Cannot create "
                                        "convolution kernel") ;
            return NULL ;
        }
    }

    /* Get the emission at initial wavelengths */
    gen_init = cpl_bivector_new(spec_sz);
    if (guess_resamp) {
        error = irplib_wlcalib_fill_spectrum(cpl_bivector_get_y(gen_init),
                                          lines_catalog, conv_kernel,
                                          guess_poly, 0);
    } else {
        error = irplib_vector_fill_line_spectrum_model
            (cpl_bivector_get_y(gen_init), NULL, NULL,
             guess_poly, lines_catalog,
             slitw, fwhm, xtrunc, 0, CPL_FALSE, CPL_FALSE, NULL);
    }

    if (error || cpl_vector_fill_polynomial(cpl_bivector_get_x(gen_init),
                                            guess_poly, 1, 1)) {
        cpl_vector_delete(conv_kernel);
        cpl_bivector_delete(gen_init);
        cpl_error_set_message_macro(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                                    __FILE__, __LINE__, "Cannot get the "
                                    "emission spectrum");
        return NULL;
    }
 
    /* Get the emission at corrected wavelengths */
    gen_corr = cpl_bivector_new(spec_sz);
    if (corr_resamp) {
        error = irplib_wlcalib_fill_spectrum(cpl_bivector_get_y(gen_corr),
                                          lines_catalog, conv_kernel,
                                          corr_poly, 0);
    } else {
        error = irplib_vector_fill_line_spectrum_model
            (cpl_bivector_get_y(gen_corr), NULL, NULL,
             corr_poly, lines_catalog,
             slitw, fwhm, xtrunc, 0, CPL_FALSE, CPL_FALSE, NULL);
    }

    if (error || cpl_vector_fill_polynomial(cpl_bivector_get_x(gen_corr),
                                            corr_poly, 1, 1)) {
        cpl_vector_delete(conv_kernel);
        cpl_bivector_delete(gen_init);
        cpl_bivector_delete(gen_corr) ;
        cpl_error_set_message_macro(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                                    __FILE__, __LINE__, "Cannot get the "
                                    "emission spectrum");
        return NULL;
    }
    cpl_vector_delete(conv_kernel) ;

    /* Create the ouput table */
    spc_table = cpl_table_new(spec_sz);
    cpl_table_new_column(spc_table, IRPLIB_WLXCORR_COL_WAVELENGTH, 
            CPL_TYPE_DOUBLE);
    cpl_table_new_column(spc_table, IRPLIB_WLXCORR_COL_CAT_INIT, 
            CPL_TYPE_DOUBLE);
    cpl_table_new_column(spc_table, IRPLIB_WLXCORR_COL_CAT_FINAL, 
            CPL_TYPE_DOUBLE);
    cpl_table_new_column(spc_table, IRPLIB_WLXCORR_COL_OBS, CPL_TYPE_DOUBLE);
    
    /* Update table */
    pgen = cpl_bivector_get_x_data_const(gen_corr) ;
    cpl_table_copy_data_double(spc_table, IRPLIB_WLXCORR_COL_WAVELENGTH, pgen) ;
    pgen = cpl_bivector_get_y_data_const(gen_corr) ;
    cpl_table_copy_data_double(spc_table, IRPLIB_WLXCORR_COL_CAT_FINAL, pgen) ;
    pgen = cpl_vector_get_data_const(spectrum) ;
    cpl_table_copy_data_double(spc_table, IRPLIB_WLXCORR_COL_OBS, pgen) ;
    pgen = cpl_bivector_get_y_data_const(gen_init) ;
    cpl_table_copy_data_double(spc_table, IRPLIB_WLXCORR_COL_CAT_INIT, pgen);
    cpl_bivector_delete(gen_init);
    cpl_bivector_delete(gen_corr);

    return spc_table ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Extract a sub catalog
  @param    lines_catalog   Bivector with the lines
  @param    wave_min        The wavelength min
  @param    wave_max        The wavelength max
  @return   the extracted bivector or NULL in error case
  
  Possible #_cpl_error_code_ set in this function:
  - CPL_ERROR_NULL_INPUT if (one of) the input pointer(s) is NULL
  - CPL_ERROR_ILLEGAL_INPUT if the specified wavelength range is invalid
 */
/*----------------------------------------------------------------------------*/
cpl_bivector * irplib_wlxcorr_cat_extract(
        const cpl_bivector  *   lines_catalog,
        double                  wave_min,
        double                  wave_max)
{
    const int           nlines = cpl_bivector_get_size(lines_catalog);
    int                 wave_min_id, wave_max_id ;
    cpl_vector       *  sub_cat_wl ;
    cpl_vector       *  sub_cat_int ;
    const cpl_vector *  xlines  = cpl_bivector_get_x_const(lines_catalog);
    const double     *  dxlines = cpl_vector_get_data_const(xlines);

    cpl_ensure(lines_catalog != NULL, CPL_ERROR_NULL_INPUT,    NULL);

    /* Find the 1st line */
    wave_min_id = (int)cpl_vector_find(xlines, wave_min);
    if (wave_min_id < 0) {
        cpl_error_set_message_macro(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                                    __FILE__, __LINE__, 
                                    "The starting wavelength cannot be found") ;
        return NULL ;
    }

    /* The first line must be greater than (at least?) wave_min */
    if (dxlines[wave_min_id] <= wave_min) wave_min_id++;

    /* Find the last line */
    wave_max_id = (int)cpl_vector_find(xlines, wave_max);
    if (wave_max_id < 0) {
        cpl_error_set_message_macro(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                                    __FILE__, __LINE__, 
                                    "The ending wavelength cannot be found") ;
        return NULL ;
    }
    /* The last line must be less than wave_max */
    if (dxlines[wave_max_id] >= wave_max) wave_max_id--;

    /* Checking the wavelength range at this point via the indices also
       verifies that they were not found using non-increasing wavelengths */
    cpl_ensure(wave_min_id <= wave_max_id, CPL_ERROR_ILLEGAL_INPUT, NULL);

    if (wave_min_id < 0 || wave_max_id == nlines) {
        cpl_error_set_message_macro(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                                    __FILE__, __LINE__, "The %d-line catalogue "
                                    "has no lines in the range %g -> %g",
                                    nlines, wave_min, wave_max);
        return NULL ;
    }

    sub_cat_wl = cpl_vector_extract(xlines, wave_min_id, wave_max_id, 1);
    sub_cat_int = cpl_vector_extract(cpl_bivector_get_y_const(lines_catalog), 
                                     wave_min_id, wave_max_id, 1);

    return cpl_bivector_wrap_vectors(sub_cat_wl, sub_cat_int);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Create Right Half of a symmetric smoothing kernel
  @param    slitw  The slit width [pixel]
  @param    fwhm   The spectral FWHM [pixel]
  @return   Right Half of (symmetric) smoothing vector

  The smoothing function is the right half of the convolution of a Gaussian with
  sigma =  fwhm / (2 * sqrt(2*log(2))) and a top-hat with a width equal to the
  slit width, and area 1.
  Since this function is symmetric only the central, maximum value and the
  right half is returned. The length of the resulting vector is
  1 + 5 * sigma + slitw/2 
  
  Possible #_cpl_error_code_ set in this function:
  - CPL_ERROR_ILLEGAL_INPUT if the slit width or fwhm is non-positive
 */
/*----------------------------------------------------------------------------*/
cpl_vector * irplib_wlxcorr_convolve_create_kernel(double  slitw,
                                                   double  fwhm)
{
    const double  sigma  = fwhm * CPL_MATH_SIG_FWHM;
    const int     size   = 1 + (int)(5.0 * sigma + 0.5*slitw);
    cpl_vector  * kernel = cpl_vector_new(size);


    if (cpl_vector_fill_lss_profile_symmetric(kernel, slitw, fwhm)) {
        cpl_vector_delete(kernel);
        cpl_ensure(0, cpl_error_get_code(), NULL);
    }

    return kernel;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Convolve a 1d-signal with a symmetric 1D-signal
  @param    smoothed  Preallocated vector to be smoothed in place
  @param    conv_kernel     Vector with symmetric convolution function
  @return   0 or -1 in error case

  The length of conv_kernel must be smaller than that of smoothed.
  
  Possible #_cpl_error_code_ set in this function:
  - CPL_ERROR_NULL_INPUT if (one of) the input pointer(s) is NULL
  - CPL_ERROR_ILLEGAL_INPUT if conv_kernel is longer than smoothed
 */
/*----------------------------------------------------------------------------*/
int irplib_wlxcorr_convolve(
        cpl_vector          *   smoothed,
        const cpl_vector    *   conv_kernel)
{
    int             nsamples ;
    int             ihwidth ;
    cpl_vector  *   raw ;
    double      *   psmoothe ;
    double      *   praw ;
    const double*   psymm ;
    int             i, j ;

    /* Test entries */
    cpl_ensure(smoothed, CPL_ERROR_NULL_INPUT, -1) ;
    cpl_ensure(conv_kernel, CPL_ERROR_NULL_INPUT, -1) ;

    /* Initialise */
    nsamples = cpl_vector_get_size(smoothed) ;
    ihwidth = cpl_vector_get_size(conv_kernel) - 1 ;
    cpl_ensure(ihwidth<nsamples, CPL_ERROR_ILLEGAL_INPUT, -1) ;
    psymm = cpl_vector_get_data_const(conv_kernel) ;
    psmoothe = cpl_vector_get_data(smoothed) ;
    
    /* Create raw vector */
    raw = cpl_vector_duplicate(smoothed) ;
    praw = cpl_vector_get_data(raw) ;

    /* Convolve with the symmetric function */
    for (i=0 ; i<ihwidth ; i++) {
        psmoothe[i] = praw[i] * psymm[0];
        for (j=1 ; j <= ihwidth ; j++) {
            const int k = i-j < 0 ? 0 : i-j;
            psmoothe[i] += (praw[k]+praw[i+j]) * psymm[j];
        }
    }

    for (i=ihwidth ; i<nsamples-ihwidth ; i++) {
        psmoothe[i] = praw[i] * psymm[0];
        for (j=1 ; j<=ihwidth ; j++)
            psmoothe[i] += (praw[i-j]+praw[i+j]) * psymm[j];
    }
    for (i=nsamples-ihwidth ; i<nsamples ; i++) {
        psmoothe[i] = praw[i] * psymm[0];
        for (j=1 ; j<=ihwidth ; j++) {
            const int k = i+j > nsamples-1 ? nsamples - 1 : i+j;
            psmoothe[i] += (praw[k]+praw[i-j]) * psymm[j];
        }
    }
    cpl_vector_delete(raw) ;
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Plot the wavelength solution
  @param    init        The initial guess
  @param    comp        The computed solution
  @param    sol         The real solution or NULL if not available
  @param    pix_start   The first pixel
  @param    pix_stop    The last pixel
  @return   0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int irplib_wlxcorr_plot_solution(
        const cpl_polynomial    *   init,
        const cpl_polynomial    *   comp,
        const cpl_polynomial    *   sol,
        int                         pix_start,
        int                         pix_stop) 
{
    int                 nsamples, nplots ;
    cpl_vector      **  vectors ;
    int                 i ;
    
    /* Test entries */
    if (init == NULL || comp == NULL) return -1 ;
    
    /* Initialise */
    nsamples = pix_stop - pix_start + 1 ;
    if (sol != NULL)    nplots = 3 ;
    else                nplots = 2 ;
    
    /* Create vectors */
    vectors = cpl_malloc((nplots+1)*sizeof(cpl_vector*)) ;
    for (i=0 ; i<nplots+1 ; i++) vectors[i] = cpl_vector_new(nsamples) ;

    /* First plot with the lambda/pixel relation */
    /* Fill vectors */
    for (i=0 ; i<nsamples ; i++) {
        cpl_vector_set(vectors[0], i, pix_start+i) ;
        cpl_vector_set(vectors[1], i, 
                cpl_polynomial_eval_1d(init, (double)(pix_start+i), NULL)) ;
        cpl_vector_set(vectors[2], i, 
                cpl_polynomial_eval_1d(comp, (double)(pix_start+i), NULL)) ;
        if (sol != NULL) 
            cpl_vector_set(vectors[3], i, 
                    cpl_polynomial_eval_1d(sol, (double)(pix_start+i), NULL)) ;
    }

    /* Plot */
    cpl_plot_vectors("set grid;set xlabel 'Position (pixels)';", 
        "t '1-Initial / 2-Computed / 3-Solution' w lines", 
        "", (const cpl_vector **)vectors, nplots+1);

    /* Free vectors */
    for (i=0 ; i<nplots+1 ; i++) cpl_vector_delete(vectors[i]) ;
    cpl_free(vectors) ;

    /* Allocate vectors */
    nplots -- ;
    vectors = cpl_malloc((nplots+1)*sizeof(cpl_vector*)) ;
    for (i=0 ; i<nplots+1 ; i++) vectors[i] = cpl_vector_new(nsamples) ;
    
    /* Second plot with the delta-lambda/pixel relation */
    /* Fill vectors */
    for (i=0 ; i<nsamples ; i++) {
        double              diff ;
        cpl_vector_set(vectors[0], i, pix_start+i) ;
        diff = cpl_polynomial_eval_1d(comp, (double)(pix_start+i), NULL) -
            cpl_polynomial_eval_1d(init, (double)(pix_start+i), NULL) ;
        cpl_vector_set(vectors[1], i, diff) ;
        if (sol != NULL) {
            diff = cpl_polynomial_eval_1d(sol, (double)(pix_start+i), NULL) -
                cpl_polynomial_eval_1d(init, (double)(pix_start+i), NULL) ;
            cpl_vector_set(vectors[2], i, diff) ;
        }
    }

    /* Plot */
    if (sol == NULL) {
        cpl_bivector    *   bivector ;
        bivector = cpl_bivector_wrap_vectors(vectors[0], vectors[1]) ;
        cpl_plot_bivector(
"set grid;set xlabel 'Position (pixels)';set ylabel 'Wavelength difference';", 
            "t 'Computed-Initial wavelenth' w lines", "", bivector);
        cpl_bivector_unwrap_vectors(bivector) ;
    } else {
        cpl_plot_vectors("set grid;set xlabel 'Position (pixels)';", 
            "t '1-Computed - Initial / 2--Solution - Initial' w lines", 
            "", (const cpl_vector **)vectors, nplots+1);
    }
    
    /* Free vectors */
    for (i=0 ; i<nplots+1 ; i++) cpl_vector_delete(vectors[i]) ;
    cpl_free(vectors) ;

    /* Return */
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Plot the spectral table
  @param    spc_table       The spectral table
  @param    title           A title
  @param    first_plotted_line  idx of the first line to plot (strongest to
                weakest, 1 is the strongest)
  @param    last_plotted_line   idx of the last line to plot (strongest to
                weakest, 1 is the strongest). If 0, no line is plotted.
  @return   0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int irplib_wlxcorr_plot_spc_table(
        const cpl_table     *   spc_table, 
        const char          *   title,
        int                     first_plotted_line,
        int                     last_plotted_line) 
{
    char                title_loc[1024] ;
    cpl_vector      **  vectors ;
    cpl_vector      **  sub_vectors ;
    cpl_vector      *   tmp_vec ;
    int                 nsamples ;
    double              mean1, mean3 ;
    int                 start_ind, stop_ind, hsize_pix ;
    int                 i, j ;

    /* Test entries */
    if (first_plotted_line > last_plotted_line) return -1 ;
    if (spc_table == NULL) return -1 ;
    
    /* Initialise */
    nsamples = cpl_table_get_nrow(spc_table) ;
    hsize_pix = 10 ;
    
    sprintf(title_loc, 
        "t '%s - 1-Initial catalog/2-Corrected catalog/3-Observed' w lines",
        title) ;
    title_loc[1023] = (char)0 ;
    
    vectors = cpl_malloc(4*sizeof(cpl_vector*)) ;
    vectors[0] = cpl_vector_wrap(nsamples, 
            cpl_table_get_data_double((cpl_table*)spc_table,
                IRPLIB_WLXCORR_COL_WAVELENGTH));
    vectors[1] = cpl_vector_wrap(nsamples, 
            cpl_table_get_data_double((cpl_table*)spc_table, 
                IRPLIB_WLXCORR_COL_CAT_INIT));
    vectors[2] = cpl_vector_wrap(nsamples, 
            cpl_table_get_data_double((cpl_table*)spc_table, 
                IRPLIB_WLXCORR_COL_CAT_FINAL));
    vectors[3] = cpl_vector_wrap(nsamples, 
            cpl_table_get_data_double((cpl_table*)spc_table, 
                IRPLIB_WLXCORR_COL_OBS)) ;

    /* Scale the signal for a bettre display */
    mean1 = cpl_vector_get_mean(vectors[1]) ;
    mean3 = cpl_vector_get_mean(vectors[3]) ;
    if (fabs(mean3) > 1)
        cpl_vector_multiply_scalar(vectors[3], fabs(mean1/mean3)) ;

    cpl_plot_vectors("set grid;set xlabel 'Wavelength (nm)';", title_loc,
        "", (const cpl_vector **)vectors, 4);

    /* Unscale the signal */
    if (fabs(mean3) > 1)
        cpl_vector_multiply_scalar(vectors[3], mean3/mean1) ;

    /* Loop on the brightest lines and zoom on them */
    sprintf(title_loc, 
"t '%s - 1-Initial catalog/2-Corrected catalog/3-Observed (ZOOMED)' w lines",
        title) ;
    title_loc[1023] = (char)0 ;
    tmp_vec = cpl_vector_duplicate(vectors[2]) ;
    for (i=0 ; i<last_plotted_line ; i++) {
        double max;
        /* Find the brightest line */
        if ((max = cpl_vector_get_max(tmp_vec)) <= 0.0) break ;
        for (j=0 ; j<nsamples ; j++) {
            if (cpl_vector_get(tmp_vec, j) == max) break ;
        }
        if (j-hsize_pix < 0) start_ind = 0 ;
        else start_ind = j-hsize_pix ;
        if (j+hsize_pix > nsamples-1) stop_ind = nsamples-1 ;
        else stop_ind = j+hsize_pix ;
        for (j=start_ind ; j<=stop_ind ; j++) cpl_vector_set(tmp_vec, j, 0.0) ;

        if (i+1 >= first_plotted_line) {
            sub_vectors = cpl_malloc(4*sizeof(cpl_vector*)) ;
            sub_vectors[0]=cpl_vector_extract(vectors[0],start_ind,stop_ind,1);
            sub_vectors[1]=cpl_vector_extract(vectors[1],start_ind,stop_ind,1);
            sub_vectors[2]=cpl_vector_extract(vectors[2],start_ind,stop_ind,1);
            sub_vectors[3]=cpl_vector_extract(vectors[3],start_ind,stop_ind,1);

            cpl_plot_vectors("set grid;set xlabel 'Wavelength (nm)';", 
                    title_loc, "", (const cpl_vector **)sub_vectors, 4);

            cpl_vector_delete(sub_vectors[0]) ;
            cpl_vector_delete(sub_vectors[1]) ;
            cpl_vector_delete(sub_vectors[2]) ;
            cpl_vector_delete(sub_vectors[3]) ;
            cpl_free(sub_vectors) ;
        }
    }
    cpl_vector_delete(tmp_vec) ;
    
    cpl_vector_unwrap(vectors[0]) ;
    cpl_vector_unwrap(vectors[1]) ;
    cpl_vector_unwrap(vectors[2]) ;
    cpl_vector_unwrap(vectors[3]) ;
    cpl_free(vectors) ;

    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Plot a part of the catalog
  @param    cat         The catalog
  @param    wmin        The minimum wavelength
  @param    wmax        The maximum wavelength
  @return   0 if ok, -1 in error case
 */
/*----------------------------------------------------------------------------*/
int irplib_wlxcorr_catalog_plot(
        const cpl_bivector      *   cat,
        double                      wmin,
        double                      wmax) 
{
    int                 start, stop ;
    cpl_bivector    *   subcat ;
    cpl_vector      *   subcat_x ;
    cpl_vector      *   subcat_y ;
    const double    *   pwave ;
    int                 nvals, nvals_tot ;
    int                 i ;

    /* Test entries */
    if (cat == NULL) return -1 ;
    if (wmax <= wmin) return -1 ;

    /* Initialise */
    nvals_tot = cpl_bivector_get_size(cat) ;

    /* Count the nb of values */
    pwave = cpl_bivector_get_x_data_const(cat) ;
    if (pwave[0] >= wmin) start = 0 ;
    else start = -1 ;
    if (pwave[nvals_tot-1] <= wmax) stop = nvals_tot-1 ;
    else stop = -1 ;
    i=0 ;
    while ((i<nvals_tot-1) && (pwave[i] < wmin)) i++ ;
    start = i ;
    i= nvals_tot-1 ;
    while ((i>0)           && (pwave[i] > wmax)) i-- ;
    stop = i ;

    if (start>=stop) {
        cpl_msg_error(cpl_func, "Cannot plot the catalog") ;
        return -1 ;
    }
    nvals = stop - start + 1 ;

    /* Create the bivector to plot */
    subcat_x = cpl_vector_extract(cpl_bivector_get_x_const(cat),start,stop, 1) ;
    subcat_y = cpl_vector_extract(cpl_bivector_get_y_const(cat),start,stop, 1) ;
    subcat = cpl_bivector_wrap_vectors(subcat_x, subcat_y) ;

    /* Plot */
    if (nvals > 500) {
        cpl_plot_bivector(
                "set grid;set xlabel 'Wavelength (nm)';set ylabel 'Emission';",
                "t 'Catalog Spectrum' w lines", "", subcat);
    } else {
        cpl_plot_bivector(
                "set grid;set xlabel 'Wavelength (nm)';set ylabel 'Emission';",
                "t 'Catalog Spectrum' w impulses", "", subcat);
    }
    cpl_bivector_unwrap_vectors(subcat) ;
    cpl_vector_delete(subcat_x) ;
    cpl_vector_delete(subcat_y) ;

    return 0 ;
}
   
/**@}*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Estimate a possible solution
  @param    vxc             The vector of cross-correlation(s) to be filled
  @param    model           The model spectrum
  @param    spectrum        The observed spectrum
  @param    lines_catalog   The lines catalog
  @param    conv_kernel     The convolution kernel, or NULL for line spectrum
  @param    poly_candi      Candidate Polynomial 
  @param    slitw           The slit width [pixel]
  @param    fwhm            The spectral FWHM [pixel]
  @return   void
  @note vxc is filled with 0.0 on failure, errors are reset

 */
/*----------------------------------------------------------------------------*/
static void irplib_wlxcorr_estimate(cpl_vector           * vxc,
                                    cpl_vector           * model,
                                    const cpl_vector     * spectrum,
                                    const cpl_bivector   * lines_catalog,
                                    const cpl_vector     * conv_kernel,
                                    const cpl_polynomial * poly_candi,
                                    double                 slitw,
                                    double                 fwhm)
{
    cpl_errorstate prestate = cpl_errorstate_get();
    const int hsize = cpl_vector_get_size(vxc) / 2;

    if (conv_kernel != NULL) {
        irplib_wlcalib_fill_spectrum(model, lines_catalog, conv_kernel,
                                  poly_candi, hsize);
    } else {
        const double xtrunc = 0.5 * slitw + 5.0 * fwhm * CPL_MATH_SIG_FWHM;

        irplib_vector_fill_line_spectrum_model(model, NULL, NULL, poly_candi,
                                               lines_catalog, slitw, fwhm,
                                               xtrunc, 0, CPL_FALSE, CPL_FALSE,
                                               NULL);
    }

    if (cpl_errorstate_is_equal(prestate))
        cpl_vector_correlate(vxc, model, spectrum);

    if (!cpl_errorstate_is_equal(prestate)) {
        cpl_vector_fill(vxc, 0.0);

        /* cpl_errorstate_dump(prestate, CPL_FALSE, NULL); */
        cpl_errorstate_set(prestate);

    }

    return;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Try to guess if a catalogue is a sampled profile or (arc) lines
  @param    wavelengths     Cector with the wavelengths of the catalogue
  @param    disp1d          The 1D-polynomial for the wavelengths
  @param    spec_sz         The spectrum size
  @param    tol             The line tolerance [lines/pixel]
  @return   CPL_TRUE iff the catalogue is a line catalogue

 */
/*----------------------------------------------------------------------------*/
static cpl_boolean irplib_wlcalib_is_lines(const cpl_vector * wavelengths,
                                        const cpl_polynomial * disp1d,
                                        int spec_sz,
                                        double tol)
{
    const int nlines = cpl_vector_get_size(wavelengths);
    /* The dispersion on the detector center */
    const double dispersion = cpl_polynomial_eval_1d_diff(disp1d,
                                                          0.5 * spec_sz + 1.0,
                                                          0.5 * spec_sz,
                                                          NULL);
    const double range = cpl_vector_get(wavelengths, nlines-1)
        - cpl_vector_get(wavelengths, 0);

    cpl_ensure(wavelengths != NULL, CPL_ERROR_NULL_INPUT,    CPL_FALSE);
    cpl_ensure(disp1d      != NULL, CPL_ERROR_NULL_INPUT,    CPL_FALSE);
    cpl_ensure(cpl_polynomial_get_dimension(disp1d) == 1,
               CPL_ERROR_ILLEGAL_INPUT, CPL_FALSE);
    cpl_ensure(range > 0.0,      CPL_ERROR_ILLEGAL_INPUT, CPL_FALSE);

    return nlines * fabs(dispersion) <= tol * fabs(range) ? CPL_TRUE
        : CPL_FALSE;

}

/*----------------------------------------------------------------------------*/
/**
  @brief    Construct the model spectrum at the given wavelengths
  @param    self            Vector to fill
  @param    lines_catalog   Bivector with the lines
  @param    slitw           The slit width
  @param    fwhm            The spectral FWHM [pixel]
  @param    poly            the polynomial for the wavelengths
  @param    search_hs       the half size of the searching zone
  @return   CPL_ERROR_NONE or the relevant CPL eror on failure

  The expected emission is a model spectrum used to cross-correlate
  against an actual observed spectrum. 
  Its size is nsamples + 2 * search_hs
 */
/*----------------------------------------------------------------------------*/
static
cpl_error_code irplib_wlcalib_fill_spectrum(cpl_vector           * self,
                                         const cpl_bivector   * lines_catalog,
                                         const cpl_vector     * conv_kernel,
                                         const cpl_polynomial * poly,
                                         int                    search_hs)
{


    const int          size = cpl_vector_get_size(self);
    const int          nlines = cpl_bivector_get_size(lines_catalog);
    const cpl_vector * xlines  = cpl_bivector_get_x_const(lines_catalog);
    const double     * dxlines = cpl_vector_get_data_const(xlines);
    cpl_bivector     * sub_cat ;
    cpl_vector       * sub_cat_x;
    cpl_vector       * sub_cat_y;
    cpl_vector       * wl_limits;
    double             wave_min, wave_max;
    int                wave_min_id, wave_max_id;
    int                nsub;
    int                error;

    cpl_ensure_code(self          != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(lines_catalog != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(conv_kernel   != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(poly          != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(size > 0,              CPL_ERROR_ILLEGAL_INPUT);


    /* Resample the spectrum */
    wl_limits = cpl_vector_new(size + 1);
    cpl_vector_fill_polynomial(wl_limits, poly, 0.5 - search_hs, 1);

    /* The spectrum wavelength bounds */
    wave_min = cpl_vector_get(wl_limits, 0);
    wave_max = cpl_vector_get(wl_limits, size);

    /* Find the 1st line */
    wave_min_id = cpl_vector_find(xlines, wave_min);
    /* The first line must be less than or equal to wave_min */
    if (dxlines[wave_min_id] > wave_min) wave_min_id--;

    if (wave_min_id < 0) {
        cpl_vector_delete(wl_limits);
        return cpl_error_set_message_macro(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                                           __FILE__, __LINE__, "The %d-line "
                                           "catalogue only has lines above %g",
                                           nlines, wave_min);
    }

    /* Find the last line */
    wave_max_id = cpl_vector_find(xlines, wave_max);
    /* The last line must be greater than or equal to wave_max */
    if (dxlines[wave_max_id] < wave_max) wave_max_id++;

    if (wave_max_id == nlines) {
        cpl_vector_delete(wl_limits);
        return cpl_error_set_message_macro(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                                           __FILE__, __LINE__, "The %d-line "
                                           "catalogue only has lines below %g",
                                           nlines, wave_max);
    }

    /* Checking the wavelength range at this point via the indices also
       verifies that they were not found using non-increasing wavelengths */
    nsub = 1 + wave_max_id - wave_min_id;
    cpl_ensure_code(nsub > 1, CPL_ERROR_ILLEGAL_INPUT);

    /* Wrap a new bivector around the relevant part of the catalog */
    /* The data is _not_ modified */
    sub_cat_x = cpl_vector_wrap(nsub, wave_min_id + (double*)dxlines);
    sub_cat_y = cpl_vector_wrap(nsub, wave_min_id + (double*)
                                cpl_bivector_get_y_data_const(lines_catalog));
    sub_cat = cpl_bivector_wrap_vectors(sub_cat_x, sub_cat_y);

    /* High resolution catalog */
    error = irplib_wlxcorr_signal_resample(self, wl_limits, sub_cat);

    cpl_vector_delete(wl_limits);
    cpl_bivector_unwrap_vectors(sub_cat);
    (void)cpl_vector_unwrap(sub_cat_x);
    (void)cpl_vector_unwrap(sub_cat_y);

    cpl_ensure_code(!error, CPL_ERROR_ILLEGAL_INPUT);

    /* Smooth the instrument resolution */
    cpl_ensure_code(!irplib_wlxcorr_convolve(self, conv_kernel),
                    cpl_error_get_code());

    return CPL_ERROR_NONE;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Smoothe a 1d-signal by averaging it
  @param    resampled Preallocated vector to hold resampled signal
  @param    xbounds   Vector with the wavelengths boundaries
  @param    hires     Bivector with the high resolution signal
  @return   0 if ok, -1 otherwise

  The length of xbouns must be one higher than that of the resampled signal.
 */
/*----------------------------------------------------------------------------*/
static int irplib_wlxcorr_signal_resample(
        cpl_vector          *   resampled, 
        const cpl_vector    *   xbounds,
        const cpl_bivector  *   hires)
{
    const int           hrsize = cpl_bivector_get_size(hires);
    const cpl_vector*   xhires ;
    const cpl_vector*   yhires ;
    const double    *   pxhires ;
    const double    *   pyhires ;
    const double    *   pxbounds ;
    cpl_vector      *   ybounds ;
    cpl_bivector    *   boundary ;
    double          *   pybounds ;
    double          *   presampled ;
    int                 nsamples ;
    int                 i, itt ;
   
    /* Test entries */
    if ((!resampled) || (!xbounds) || (!hires)) return -1 ;

    /* Initialise */
    nsamples = cpl_vector_get_size(resampled) ;

    /* Initialise */
    presampled = cpl_vector_get_data(resampled) ;
    pxbounds = cpl_vector_get_data_const(xbounds) ;
    xhires = cpl_bivector_get_x_const(hires) ;
    yhires = cpl_bivector_get_y_const(hires) ;
    pxhires = cpl_vector_get_data_const(xhires) ;
    pyhires = cpl_vector_get_data_const(yhires) ;
    
    /* Create a new vector */
    ybounds = cpl_vector_new(cpl_vector_get_size(xbounds)) ;
    boundary = cpl_bivector_wrap_vectors((cpl_vector*)xbounds,ybounds) ;
    pybounds = cpl_vector_get_data(ybounds) ;

    /* Test entries */
    if (cpl_bivector_get_size(boundary) != nsamples + 1) {
        cpl_bivector_unwrap_vectors(boundary) ;
        cpl_vector_delete(ybounds) ;
        return -1 ;
    }

    /* Get the ind  */
    itt = cpl_vector_find(xhires, pxbounds[0]);

    /* Interpolate the signal */
    if (cpl_bivector_interpolate_linear(boundary, hires)) {
        cpl_bivector_unwrap_vectors(boundary) ;
        cpl_vector_delete(ybounds) ;
        return -1 ;
    }

    /* At this point itt most likely points to element just below
       pxbounds[0] */
    while (pxhires[itt] < pxbounds[0]) itt++;

    for (i=0; i < nsamples; i++) {
        /* The i'th signal is the weighted average of the two interpolated
           signals at the pixel boundaries and those table signals in
           between */

        double xlow  = pxbounds[i];
        double x     = pxhires[itt];

        if (x > pxbounds[i+1]) x = pxbounds[i+1];
        /* Contribution from interpolated value at wavelength at lower pixel
           boundary */
        presampled[i] = pybounds[i] * (x - xlow);

        /* Contribution from table values in between pixel boundaries */
        while ((pxhires[itt] < pxbounds[i+1]) && (itt < hrsize)) {
            const double xprev = x;
            x = pxhires[itt+1];
            if (x > pxbounds[i+1]) x = pxbounds[i+1];
            presampled[i] += pyhires[itt] * (x - xlow);
            xlow = xprev;
            itt++;
        }

        /* Contribution from interpolated value at wavelength at upper pixel
           boundary */
        presampled[i] += pybounds[i+1] * (pxbounds[i+1] - xlow);

        /* Compute average by dividing integral by length of pixel range
           (the factor 2 comes from the contributions) */
        presampled[i] /= 2 * (pxbounds[i+1] - pxbounds[i]);
    }
    cpl_bivector_unwrap_vectors(boundary) ;
    cpl_vector_delete(ybounds) ;
    return 0 ;
}



/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Fill right half of a symmetric long-slit spectroscopy line profile
  @param    self   The pre-allocated vector to be filled
  @param    slitw  The slit width [pixel]
  @param    fwhm   The spectral FWHM [pixel]
  @return   CPL_ERROR_NONE or the relevant error code on failure

  The smoothing function is the right half of the convolution of a Gaussian with
  sigma =  fwhm / (2 * sqrt(2*log(2))) and a top-hat with a width equal to the
  slit width, and area 1, and a tophat with unit width and area.
  (the convolution with a tophat with unit width and area equals integration
   from  i-1/2 to 1+1/2).
  Since this function is symmetric only the central, maximum value and the
  right half is computed, 
  
  Possible #_cpl_error_code_ set in this function:
  - CPL_ERROR_NULL_INPUT if a pointer is NULL
  - CPL_ERROR_ILLEGAL_INPUT if the slit width or fwhm is non-positive
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code cpl_vector_fill_lss_profile_symmetric(cpl_vector * self,
                                                            double  slitw,
                                                            double  fwhm)
{

    const double sigma = fwhm * CPL_MATH_SIG_FWHM;
    const int    n     = cpl_vector_get_size(self);
    int          i;


    cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(slitw > 0.0,  CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(fwhm  > 0.0,  CPL_ERROR_ILLEGAL_INPUT);

    /* Cannot fail now */

    /* Special case for i = 0 */
    (void)cpl_vector_set(self, 0,
                         (irplib_erf_antideriv(0.5*slitw + 0.5, sigma) -
                          irplib_erf_antideriv(0.5*slitw - 0.5, sigma)) / slitw);

    for (i = 1; i < n; i++) {
        /* FIXME: Reuse two irplib_erf_antideriv() calls from previous value */
        const double x1p = i + 0.5*slitw + 0.5;
        const double x1n = i - 0.5*slitw + 0.5;
        const double x0p = i + 0.5*slitw - 0.5;
        const double x0n = i - 0.5*slitw - 0.5;
        const double val = 0.5/slitw *
            (irplib_erf_antideriv(x1p, sigma) - irplib_erf_antideriv(x1n, sigma) -
             irplib_erf_antideriv(x0p, sigma) + irplib_erf_antideriv(x0n, sigma));
        (void)cpl_vector_set(self, i, val);
    }

    return CPL_ERROR_NONE;
}
