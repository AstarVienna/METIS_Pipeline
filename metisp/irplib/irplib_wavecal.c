/*
 * This file is part of the IRPLIB Pipeline
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

/* Needed for irplib_errorstate_dump_debug() */
#include "irplib_utils.h"

#include <string.h>
#include <math.h>

#ifdef HAVE_GSL
#include <gsl/gsl_multimin.h>
#endif

/*-----------------------------------------------------------------------------
                               Private types
 -----------------------------------------------------------------------------*/

#ifdef HAVE_GSL

typedef struct {

    const cpl_vector * observed;
    cpl_polynomial   * disp1d;
    cpl_vector       * spectrum;
    irplib_base_spectrum_model * param;
    cpl_error_code  (* filler)(cpl_vector *, const cpl_polynomial *,
                               irplib_base_spectrum_model *);
    cpl_vector       * vxc;
    double             xc;
    int                maxxc;
    double             mxc;
    cpl_polynomial   * mdisp;
    int                ishift;

} irplib_multimin;

#endif /* HAVE_GSL */

/*-----------------------------------------------------------------------------
                               Defines
 -----------------------------------------------------------------------------*/

#ifndef inline
#define inline /* inline */
#endif

#define IRPLIB_MAX(A,B) ((A) > (B) ? (A) : (B))
#define IRPLIB_MIN(A,B) ((A) < (B) ? (A) : (B))

/*-----------------------------------------------------------------------------
                                   Private functions
 -----------------------------------------------------------------------------*/

#ifdef HAVE_GSL
static double irplib_gsl_correlation(const gsl_vector *, void *);
#endif

static cpl_error_code
irplib_polynomial_find_1d_from_correlation_(cpl_polynomial *, int,
                                            const cpl_vector *,
                                            irplib_base_spectrum_model *,
                                            cpl_error_code (*)
                                            (cpl_vector *,
                                             const cpl_polynomial *,
                                             irplib_base_spectrum_model *),
                                            double, double, int, int,
                                            double *, cpl_boolean *);


/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_wavecal     Spectro functionality
 */
/*----------------------------------------------------------------------------*/

/**@{*/


/*----------------------------------------------------------------------------*/
/**
  @brief    Count the positive Y-entries in a given X-range
  @param    self     Bivector with increasing X-entries
  @param    x_min    minimum X-entry
  @param    x_max    maximum X-entry
  @return   the number of matching entries, or negative on error
 */
/*----------------------------------------------------------------------------*/
int irplib_bivector_count_positive(const cpl_bivector * self,
                                  double               x_min,
                                  double               x_max)
{

    const int      nself = cpl_bivector_get_size(self);
    const double * px    = cpl_bivector_get_x_data_const(self);
    const double * py    = cpl_bivector_get_y_data_const(self);
    int            npos  = 0;
    int            i     = 0;

    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, -1);
    cpl_ensure(x_min <= x_max, CPL_ERROR_ILLEGAL_INPUT, -2);
    
    /* FIXME: Use cpl_vector_find() */
    while (i < nself && px[i] < x_min) i++;
    while (i < nself && px[i] < x_max)
        if (py[i++] > 0) npos++;

    return npos;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Fit a 2D-dispersion from an image of wavelengths
  @param    self    2D-polynomial to hold fit
  @param    imgwave Image map of wavelengths, any pixeltype
  @param    fitdeg  Degree of fit
  @param    presid  On success, points to fitting residual
  @return   CPL_ERROR_NONE on success, otherwise the relevant CPL error code

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_polynomial_fit_2d_dispersion(cpl_polynomial * self,
                                                   const cpl_image * imgwave,
                                                   int fitdeg, double * presid)
{

    const int        nx = cpl_image_get_size_x(imgwave);
    const int        ny = cpl_image_get_size_y(imgwave);
    const int        nbad = cpl_image_count_rejected(imgwave);
    const int        nsamp = nx * ny - nbad;
    cpl_matrix     * xy_pos;
    double         * xdata;
    double         * ydata;
    cpl_vector     * wlen;
    double         * dwlen;
    const cpl_size   nfitdeg = (cpl_size)fitdeg;
    int i, j;
    int k = 0;

    cpl_ensure_code(self    != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(imgwave != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(presid  != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(fitdeg > 0,      CPL_ERROR_ILLEGAL_INPUT);

    cpl_ensure_code(cpl_polynomial_get_dimension(self) == 2,
                    CPL_ERROR_ILLEGAL_INPUT);

    xy_pos = cpl_matrix_new(2, nsamp);
    xdata = cpl_matrix_get_data(xy_pos);
    ydata = xdata + nsamp;

    dwlen = (double*)cpl_malloc(nsamp * sizeof(double));
    wlen = cpl_vector_wrap(nsamp, dwlen);

    for (i=1; i <= nx; i++) {
        for (j=1; j <= ny; j++) {
            int is_bad;
            const double value = cpl_image_get(imgwave, i, j, &is_bad);
            if (!is_bad) {
                xdata[k] = i;
                ydata[k] = j;
                dwlen[k] = value;
                k++;
            }
        }
    }

    cpl_msg_info(cpl_func, "Fitting 2D polynomial to %d X %d image, ignoring "
                 "%d poorly calibrated pixels", nx, ny, nbad);

    if (cpl_polynomial_fit(self, xy_pos, NULL, wlen, NULL, CPL_FALSE, NULL,
                           &nfitdeg) == CPL_ERROR_NONE && presid != NULL) {
        cpl_vector_fill_polynomial_fit_residual(wlen, wlen, NULL, self, xy_pos,
                                                NULL);
        *presid = cpl_vector_product(wlen, wlen)/nsamp;
    }
    cpl_matrix_delete(xy_pos);
    cpl_vector_delete(wlen);

    cpl_ensure_code(k == nsamp, CPL_ERROR_UNSPECIFIED);

    return CPL_ERROR_NONE;
}


/*----------------------------------------------------------------------------*/
/**
  @brief Modify self by maximizing the cross-correlation
  @param self    1D-Dispersion relation to modify, at least of degree 1
  @param maxdeg  Maximize the cross-correlation by modifying maxdeg degree
  @param obs     The observed spectrum to correlate against
  @param model   The model of the lines/OTF etc.
  @param filler  The function to fill the model spectrum
  @param pixtol  The (positive) dispersion tolerance, e.g. 1e-6
  @param pixstep The step length used in the maximization, e.g. 0.5 [pixel]
  @param hsize   Half the search-distance to ensure a global-maximum, hsize >= 0
  @param maxite  Maximum number of iterations, e.g. 100 * maxdeg
  @param pxc     On sucess, *pxc is the cross-correlation
  @return CPL_ERROR_NONE on success, otherwise the relevant CPL error code
  @note Fails with CPL_ERROR_UNSUPPORTED_MODE if compiled without GSL.
        self must be increasing in the interval from 1 to the length of obs.

 */
/*----------------------------------------------------------------------------*/
cpl_error_code
irplib_polynomial_find_1d_from_correlation(cpl_polynomial * self,
                                           int maxdeg,
                                           const cpl_vector * obs,
                                           irplib_base_spectrum_model * model,
                                           cpl_error_code (* filler)
                                           (cpl_vector *,
                                            const cpl_polynomial *,
                                            irplib_base_spectrum_model *),
                                           double pixtol,
                                           double pixstep,
                                           int hsize,
                                           int maxite,
                                           double * pxc)
{
    cpl_boolean restart = CPL_FALSE;
    const cpl_error_code error = irplib_polynomial_find_1d_from_correlation_
        (self, maxdeg, obs, model, filler, pixtol, pixstep, hsize, maxite, pxc,
         &restart);

    return error ? cpl_error_set_where(cpl_func) :
        (restart ? cpl_error_set(cpl_func, CPL_ERROR_CONTINUE)
         : CPL_ERROR_NONE);
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief Modify self by maximizing the cross-correlation
  @param self    1D-Dispersion relation to modify, at least of degree 1
  @param maxdeg  Maximize the cross-correlation by modifying maxdeg degree
  @param obs     The observed spectrum to correlate against
  @param model   The model of the lines/OTF etc.
  @param filler  The function to fill the model spectrum
  @param pixtol  The (positive) dispersion tolerance, e.g. 1e-6
  @param pixstep The step length used in the maximization, e.g. 0.5 [pixel]
  @param hsize   Half the search-distance to ensure a global-maximum, hsize >= 0
  @param maxite  Maximum number of iterations, e.g. 100 * maxdeg
  @param pxc     On sucess, *pxc is the cross-correlation
  @param prestart CPL_TRUE, iff the call can be redone
  @return CPL_ERROR_NONE on success, otherwise the relevant CPL error code
  @see irplib_polynomial_find_1d_from_correlation
  @note Fails with CPL_ERROR_UNSUPPORTED_MODE if compiled without GSL.
        self must be increasing in the interval from 1 to the length of obs.

 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
irplib_polynomial_find_1d_from_correlation_(cpl_polynomial * self,
                                            int maxdeg,
                                            const cpl_vector * obs,
                                            irplib_base_spectrum_model * model,
                                            cpl_error_code (* filler)
                                            (cpl_vector *,
                                             const cpl_polynomial *,
                                             irplib_base_spectrum_model *),
                                            double pixtol,
                                            double pixstep,
                                            int hsize,
                                            int maxite,
                                            double * pxc,
                                            cpl_boolean * prestart)
{

#ifdef HAVE_GSL
    const gsl_multimin_fminimizer_type * T = gsl_multimin_fminimizer_nmsimplex;
    gsl_multimin_fminimizer * minimizer;
    gsl_multimin_function my_func;
    irplib_multimin data;
    gsl_vector * dispgsl;
    gsl_vector * stepsize;
    gsl_vector * dispprev;
    int status = GSL_CONTINUE;
    const int nobs = cpl_vector_get_size(obs);
    const cpl_size nfit = maxdeg + 1;
    cpl_errorstate prestate = cpl_errorstate_get();
    /* Convert pixel step to wavelength step on detector center */
    const double wlstep =
        cpl_polynomial_eval_1d_diff(self, 0.5 * (nobs + pixstep),
                                    0.5 * (nobs - pixstep), NULL);
    double wlstepi = wlstep;
    int iter;
    cpl_size i;

#endif

    cpl_ensure_code(prestart != NULL, CPL_ERROR_NULL_INPUT);
    *prestart = CPL_FALSE;
    cpl_ensure_code(self     != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(obs      != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(model    != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(filler   != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(pxc      != NULL, CPL_ERROR_NULL_INPUT);

    cpl_ensure_code(cpl_polynomial_get_dimension(self) == 1,
                    CPL_ERROR_ILLEGAL_INPUT);

    cpl_ensure_code(cpl_polynomial_get_degree(self) > 0,
                    CPL_ERROR_ILLEGAL_INPUT);

    cpl_ensure_code(maxdeg  >=  0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(pixtol  > 0.0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(pixstep > 0.0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(hsize   >=  0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(maxite  >=  0, CPL_ERROR_ILLEGAL_INPUT);

#ifndef HAVE_GSL
    return cpl_error_set_message(cpl_func, CPL_ERROR_UNSUPPORTED_MODE,
                                 "GSL is not available");
#else

    minimizer = gsl_multimin_fminimizer_alloc(T, (size_t)nfit);

    cpl_ensure_code(minimizer != NULL, CPL_ERROR_ILLEGAL_OUTPUT);
       
    dispgsl  = gsl_vector_alloc((size_t)nfit);
    stepsize = gsl_vector_alloc((size_t)nfit);
    dispprev = gsl_vector_alloc((size_t)nfit);

    for (i=0; i < nfit; i++) {
        const double value = cpl_polynomial_get_coeff(self, &i);
        gsl_vector_set(dispgsl, (size_t)i, value);
        gsl_vector_set(stepsize, (size_t)i, wlstepi);
        wlstepi /= (double)nobs;
    }

    my_func.n = nfit;
    my_func.f = &irplib_gsl_correlation;
    my_func.params = (void *)(&data);

    data.observed = obs;
    data.disp1d   = self;
    data.spectrum = cpl_vector_new(nobs + 2 * hsize);
    data.vxc      = cpl_vector_new(1 + 2 * hsize);
    data.xc       = 0;
    data.param    = model;
    data.filler   = filler;
    data.maxxc    = 0; /* Output */
    data.ishift   = 0; /* Output */
    data.mxc      = -1.0; /* Output */
    data.mdisp    = NULL; /* Output */

    gsl_multimin_fminimizer_set (minimizer, &my_func, dispgsl, stepsize);

    for (iter = 0; status == GSL_CONTINUE && iter < maxite; iter++) {

        double size;
        const double fprev = minimizer->fval;

        gsl_vector_memcpy(dispprev, minimizer->x);
        status = gsl_multimin_fminimizer_iterate(minimizer);

        if (status || !cpl_errorstate_is_equal(prestate)) break;

        size = gsl_multimin_fminimizer_size (minimizer);
        status = gsl_multimin_test_size (size, pixtol);
     
        if (status == GSL_SUCCESS) {
            cpl_msg_debug(cpl_func, "converged to minimum at");

            if (nfit == 0) {
                cpl_msg_debug(cpl_func, "%5d %g df() = %g size = %g", 
                              iter,
                              gsl_vector_get (minimizer->x, 0)
                              - gsl_vector_get (dispprev, 0), 
                              minimizer->fval - fprev, size);
            } else if (nfit == 1) {
                cpl_msg_debug(cpl_func, "%5d %g %g df() = %g size = %g", 
                              iter,
                              gsl_vector_get (minimizer->x, 0)
                              - gsl_vector_get (dispprev, 0), 
                              gsl_vector_get (minimizer->x, 1)
                              - gsl_vector_get (dispprev, 1), 
                              minimizer->fval - fprev, size);
            } else {
                cpl_msg_debug(cpl_func, "%5d %g %g %g df() = %g size = %g", 
                              iter,
                              gsl_vector_get (minimizer->x, 0)
                              - gsl_vector_get (dispprev, 0), 
                              gsl_vector_get (minimizer->x, 1)
                              - gsl_vector_get (dispprev, 1), 
                              gsl_vector_get (minimizer->x, 2)
                              - gsl_vector_get (dispprev, 2), 
                              minimizer->fval - fprev, size);
            }
        }
    }

    if (status == GSL_SUCCESS && cpl_errorstate_is_equal(prestate)) {
        if (data.mxc > -minimizer->fval) {
            *pxc = data.mxc;
            cpl_msg_warning(cpl_func, "Local maximum: %g(%d) > %g",
                            data.mxc, data.ishift, -minimizer->fval);
            cpl_polynomial_shift_1d(data.mdisp, 0, (double)data.ishift);
            cpl_polynomial_copy(self, data.mdisp);
            *prestart = CPL_TRUE;
        } else {
            *pxc = -minimizer->fval;
            for (i=0; i < nfit; i++) {
                const double value = gsl_vector_get(minimizer->x, i);
                cpl_polynomial_set_coeff(self, &i, value);
            }
        }
    }

    cpl_vector_delete(data.spectrum);
    cpl_vector_delete(data.vxc);
    cpl_polynomial_delete(data.mdisp);
    gsl_multimin_fminimizer_free(minimizer);
    gsl_vector_free(dispgsl);
    gsl_vector_free(dispprev);
    gsl_vector_free(stepsize);

    cpl_ensure_code(status != GSL_CONTINUE, CPL_ERROR_CONTINUE);
    cpl_ensure_code(status == GSL_SUCCESS, CPL_ERROR_DATA_NOT_FOUND);
    cpl_ensure_code(cpl_errorstate_is_equal(prestate), cpl_error_get_code());

    return CPL_ERROR_NONE;
#endif
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Generate a 1D spectrum from a model and a dispersion relation
  @param    self    Vector to fill with spectrum
  @param    disp    1D-Dispersion relation, at least of degree 1
  @param    lsslamp Pointer to irplib_line_spectrum_model struct
  @return   CPL_ERROR_NONE on success, otherwise the relevant CPL error code

  The model comprises these elements:
  @code
    double wslit;  // Slit Width
    double wfwhm;  // FWHM of transfer function
    double xtrunc; // Truncate transfer function beyond xtrunc, xtrunc > 0
    const cpl_bivector * lines;    // Catalogue of intensities, with
                                   //   increasing X-vector elements
    cpl_vector         * linepix;  // NULL, or temporary work-space of size
                                   // equal to the lines bivector
                                   // - should be uninitialized to zero
    cpl_size             cost;     // Will be incremented for each call
    cpl_size             xcost;    // Will be incremented for each OK call
  @endcode

  The units of the X-values of the lines is assumed to be the same as
  that of disp, the units of wslit, wfwhm and xtrunc are assumed to be the same
  as the input unit of disp(), the units of self will be that of the Y-values
  of the lines.

 */
/*----------------------------------------------------------------------------*/
cpl_error_code
irplib_vector_fill_line_spectrum(cpl_vector * self,
                                 const cpl_polynomial * disp,
                                 irplib_base_spectrum_model * lsslamp)
{

    irplib_line_spectrum_model * arclamp
        = (irplib_line_spectrum_model *)lsslamp;
    cpl_error_code error;

    cpl_ensure_code(arclamp != NULL, CPL_ERROR_NULL_INPUT);

    arclamp->cost++;

    error = irplib_vector_fill_line_spectrum_model(self,
                                                   arclamp->linepix,
                                                   arclamp->erftmp,
                                                   disp,
                                                   arclamp->lines,
                                                   arclamp->wslit,
                                                   arclamp->wfwhm,
                                                   arclamp->xtrunc,
                                                   0, CPL_FALSE, CPL_FALSE,
                                                   &(arclamp->ulines));
    cpl_ensure_code(!error, error);

    arclamp->xcost++;

    return CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Generate a 1D spectrum from a model and a dispersion relation
  @param    self    Vector to fill with spectrum
  @param    disp    1D-Dispersion relation, at least of degree 1
  @param    lsslamp Pointer to irplib_line_spectrum_model struct
  @return   CPL_ERROR_NONE on success, otherwise the relevant CPL error code
  @note The logarithm is taken on the intensities
  @see irplib_vector_fill_line_spectrum

  log(1+I) is used for the (positive) intensities

 */
/*----------------------------------------------------------------------------*/
cpl_error_code
irplib_vector_fill_logline_spectrum(cpl_vector * self,
                                    const cpl_polynomial * disp,
                                    irplib_base_spectrum_model * lsslamp)
{

    irplib_line_spectrum_model * arclamp
        = (irplib_line_spectrum_model *)lsslamp;
    cpl_error_code error;

    cpl_ensure_code(arclamp != NULL, CPL_ERROR_NULL_INPUT);

    arclamp->cost++;

    error = irplib_vector_fill_line_spectrum_model(self,
                                                   arclamp->linepix,
                                                   arclamp->erftmp,
                                                   disp,
                                                   arclamp->lines,
                                                   arclamp->wslit,
                                                   arclamp->wfwhm,
                                                   arclamp->xtrunc,
                                                   0, CPL_FALSE, CPL_TRUE,
                                                   &(arclamp->ulines));
    cpl_ensure_code(!error, error);

    arclamp->xcost++;

    return CPL_ERROR_NONE;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Generate a 1D spectrum from a model and a dispersion relation
  @param    self    Vector to fill with spectrum
  @param    disp    1D-Dispersion relation, at least of degree 1
  @param    lsslamp Pointer to irplib_line_spectrum_model struct
  @return   CPL_ERROR_NONE on success, otherwise the relevant CPL error code
  @see irplib_vector_fill_line_spectrum()

  Complexity reduced from O(nw) to O(n + w), where n is number of lines and
  truncation width [pixel] of the line.

 */
/*----------------------------------------------------------------------------*/
cpl_error_code
irplib_vector_fill_line_spectrum_fast(cpl_vector * self,
                                      const cpl_polynomial * disp,
                                      irplib_base_spectrum_model * lsslamp)
{

    irplib_line_spectrum_model * arclamp
        = (irplib_line_spectrum_model *)lsslamp;
    cpl_error_code error;

    cpl_ensure_code(arclamp != NULL, CPL_ERROR_NULL_INPUT);

    arclamp->cost++;

    error = irplib_vector_fill_line_spectrum_model(self,
                                                   arclamp->linepix,
                                                   arclamp->erftmp,
                                                   disp,
                                                   arclamp->lines,
                                                   arclamp->wslit,
                                                   arclamp->wfwhm,
                                                   arclamp->xtrunc,
                                                   0, CPL_TRUE, CPL_FALSE,
                                                   &(arclamp->ulines));
    cpl_ensure_code(!error, error);

    arclamp->xcost++;

    return CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Generate a 1D spectrum from a model and a dispersion relation
  @param    self    Vector to fill with spectrum
  @param    disp    1D-Dispersion relation, at least of degree 1
  @param    lsslamp Pointer to irplib_line_spectrum_model struct
  @return   CPL_ERROR_NONE on success, otherwise the relevant CPL error code
  @note The logarithm is taken on the intensities
  @see irplib_vector_fill_line_spectrum_fast()

  log(1+I) is used for the (positive) intensities

 */
/*----------------------------------------------------------------------------*/
cpl_error_code
irplib_vector_fill_logline_spectrum_fast(cpl_vector * self,
                                         const cpl_polynomial * disp,
                                         irplib_base_spectrum_model * lsslamp)
{

    irplib_line_spectrum_model * arclamp
        = (irplib_line_spectrum_model *)lsslamp;
    cpl_error_code error;

    cpl_ensure_code(arclamp != NULL, CPL_ERROR_NULL_INPUT);

    arclamp->cost++;

    error = irplib_vector_fill_line_spectrum_model(self,
                                                   arclamp->linepix,
                                                   arclamp->erftmp,
                                                   disp,
                                                   arclamp->lines,
                                                   arclamp->wslit,
                                                   arclamp->wfwhm,
                                                   arclamp->xtrunc,
                                                   0, CPL_TRUE, CPL_TRUE,
                                                   &(arclamp->ulines));
    cpl_ensure_code(!error, error);

    arclamp->xcost++;

    return CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Plot a 1D spectrum and one from a model
  @param    self    Vector with observed spectrum
  @param    disp1d  1D-Dispersion relation, at least of degree 1
  @param    model   Pointer to model parameters
  @param    filler  The function to fill the model spectrum
  @return   CPL_ERROR_NONE on success, otherwise the relevant CPL error code
  @see irplib_vector_fill_line_spectrum()

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_plot_spectrum_and_model(const cpl_vector * self,
                                              const cpl_polynomial * disp1d,
                                              irplib_base_spectrum_model * model,
                                              cpl_error_code (* filler)
                                              (cpl_vector *,
                                               const cpl_polynomial *,
                                               irplib_base_spectrum_model *))
{

    cpl_errorstate prestate = cpl_errorstate_get();
    cpl_vector * wl;
    cpl_vector * spectrum;
    cpl_vector * vxc;
    const int len = cpl_vector_get_size(self);
    double maxval, xc;
    int ixc;
    int error = 0;

    cpl_ensure_code(self   != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(disp1d != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(model  != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(filler != NULL, CPL_ERROR_NULL_INPUT);

    cpl_ensure_code(cpl_polynomial_get_dimension(disp1d) == 1,
                    CPL_ERROR_ILLEGAL_INPUT);

    cpl_ensure_code(cpl_polynomial_get_degree(disp1d) > 0,
                    CPL_ERROR_ILLEGAL_INPUT);

    wl = cpl_vector_new(len);
    spectrum = cpl_vector_new(len);
    vxc = cpl_vector_new(1);

    error |= (int)cpl_vector_fill_polynomial(wl, disp1d, 1.0, 1.0);
    error |= filler(spectrum, disp1d, model);

    ixc = cpl_vector_correlate(vxc, self, spectrum);
    xc = cpl_vector_get(vxc, ixc);

    maxval = cpl_vector_get_max(spectrum);
    if (maxval != 0.0) 
        error |= cpl_vector_multiply_scalar(spectrum,
                                             cpl_vector_get_max(self)/maxval);
    if (!error) {
        const cpl_vector * spair[] = {wl, self, spectrum};
        char * pre = cpl_sprintf("set grid;set xlabel 'Wavelength (%g -> %g)'; "
                                 "set ylabel 'Intensity';", cpl_vector_get(wl, 0),
                                 cpl_vector_get(wl, len-1));
        char * title = cpl_sprintf("t 'Observed and modelled spectra (%d pixel "
                                   "XC=%g) ' w linespoints", len, xc);

        (void)cpl_plot_vectors(pre, title, "", spair, 3);
        cpl_free(pre);
        cpl_free(title);
    }

    cpl_vector_delete(wl);
    cpl_vector_delete(spectrum);
    cpl_vector_delete(vxc);

    cpl_errorstate_set(prestate);

    return CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Find shift(s) that maximizes (locally) the cross-correlation
  @param    self   list of shifts that maximizes the cross-correlation (locally)
  @param    disp   1D-Dispersion relation, at least of degree 1
  @param    obs    The observed spectrum to correlate against
  @param    model  Pointer to model parameters
  @param    filler The function to fill the model spectrum
  @param    hsize  Half the search-distance, hsize > 0 [pixel]
  @param    doplot Plot the cross-correlation as a function of pixel shift
  @param    pxc    Iff non-NULL, set *pxc to cross-correlation on success
  @return   CPL_ERROR_NONE on success, otherwise the relevant CPL error code
  @note On success, self will be resized to fit the number of shifts.

  The shifts are listed in order of decreasing cross-correlation. If pxc is
  non-NULL, *pxc will be set to the cross-correlation at shift 0.

  The shifts may include the extrema -hsize and hsize.

 */
/*----------------------------------------------------------------------------*/
cpl_error_code
irplib_bivector_find_shift_from_correlation(cpl_bivector * self,
                                            const cpl_polynomial * disp,
                                            const cpl_vector * obs,
                                            irplib_base_spectrum_model * model,
                                            cpl_error_code (*filler)
                                            (cpl_vector *,
                                             const cpl_polynomial *,
                                             irplib_base_spectrum_model *),
                                            int hsize,
                                            cpl_boolean doplot,
                                            double *pxc)
{

    const int        nobs   = cpl_vector_get_size(obs);
    const int        nmodel = 2 * hsize + nobs;
    cpl_polynomial * shdisp;
    cpl_vector     * xself = cpl_bivector_get_x(self);
    cpl_vector     * yself = cpl_bivector_get_y(self);
    cpl_vector     * mspec1d;
    cpl_vector     * xcorr;
    cpl_error_code   error = CPL_ERROR_NONE;
    double           xcprev, xcnext;
    int              ixc, imax = 0;
    int              i;

    cpl_ensure_code(self   != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(disp   != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(obs    != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(model  != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(filler != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(hsize  >  0,    CPL_ERROR_ILLEGAL_INPUT);

    shdisp = cpl_polynomial_duplicate(disp);

    /* Shift reference by -hsize so filler can be used without offset */
    if (cpl_polynomial_shift_1d(shdisp, 0, -hsize)) {
        cpl_polynomial_delete(shdisp);
        return cpl_error_set_where(cpl_func);
    }

    mspec1d = cpl_vector_new(nmodel);

    if (filler(mspec1d, shdisp, model)) {
        cpl_vector_delete(mspec1d);
        return cpl_error_set_where(cpl_func);
    }

    /* Should not be able to fail now */
    xcorr = cpl_vector_new(1 + 2 * hsize);
    ixc = cpl_vector_correlate(xcorr, mspec1d, obs);

#ifdef IRPLIB_SPC_DUMP
    /* Need irplib_wavecal.c rev. 1.12 through 1.15 */
    irplib_polynomial_dump_corr_step(shdisp, xcorr, "Shift");
#endif

    cpl_vector_delete(mspec1d);
    cpl_polynomial_delete(shdisp);

    /* Find local maxima. */
    /* FIXME(?): Also include stationary points */
    i = 0;
    xcprev = cpl_vector_get(xcorr, i);
    xcnext = cpl_vector_get(xcorr, i+1);

    if (xcprev >= xcnext) {
        /* 1st data point is an extreme */
        /* FIXME: This could also be an error, recoverable by caller by
           increasing hsize */
        imax++;

        cpl_vector_set(xself, 0, i - hsize);
        cpl_vector_set(yself, 0, xcprev);

    }

    for (i = 1; i < 2 * hsize; i++) {
        const double xc = xcnext;
        xcnext = cpl_vector_get(xcorr, i+1);
        if (xc >= xcprev && xc >= xcnext) {
            /* Found (local) maximum at shift i - hsize */
            int j;

            imax++;

            if (cpl_bivector_get_size(self) < imax) {
                cpl_vector_set_size(xself, imax);
                cpl_vector_set_size(yself, imax);
            }

            for (j = imax-1; j > 0; j--) {
                if (xc <= cpl_vector_get(yself, j-1)) break;
                cpl_vector_set(xself, j, cpl_vector_get(xself, j-1));
                cpl_vector_set(yself, j, cpl_vector_get(yself, j-1));
            }
            cpl_vector_set(xself, j, i - hsize);
            cpl_vector_set(yself, j, xc);
        }
        xcprev = xc;
    }

    /* assert( i == 2 * hsize ); */

    if (xcnext >= xcprev) {
        /* Last data point is an extreme */
        /* FIXME: This could also be an error, recoverable by caller by
           increasing hsize */
        int j;

        imax++;

        if (cpl_bivector_get_size(self) < imax) {
            cpl_vector_set_size(xself, imax);
            cpl_vector_set_size(yself, imax);
        }

        for (j = imax-1; j > 0; j--) {
            if (xcnext <= cpl_vector_get(yself, j-1)) break;
            cpl_vector_set(xself, j, cpl_vector_get(xself, j-1));
            cpl_vector_set(yself, j, cpl_vector_get(yself, j-1));
        }
        cpl_vector_set(xself, j, i - hsize);
        cpl_vector_set(yself, j, xcnext);

    }

    if (doplot) {
        /* Vector of -hsize, 1-hsize, 2-hsize, ..., 0, ..., hsize */
        cpl_vector * xvals = cpl_vector_new(1 + 2 * hsize);
        cpl_bivector * bcorr = cpl_bivector_wrap_vectors(xvals, xcorr);
        double x = (double)-hsize;
        char * title = cpl_sprintf("t 'Cross-correlation of shifted %d-pixel "
                                   "spectrum (XCmax=%g at %d)' w linespoints",
                                   nobs, cpl_vector_get(xcorr, ixc),
                                   ixc - hsize);

        for (i = 0; i < 1 + 2 * hsize; i++, x += 1.0) {
            cpl_vector_set(xvals, i, x);
        }

        cpl_plot_bivector("set grid;set xlabel 'Offset [pixel]';", title,
                             "", bcorr);
        cpl_bivector_unwrap_vectors(bcorr);
        cpl_vector_delete(xvals);
        cpl_free(title);
    }

    if (pxc != NULL) *pxc = cpl_vector_get(xcorr, hsize);

    cpl_vector_delete(xcorr);

    if (imax < 1) {
        error = CPL_ERROR_DATA_NOT_FOUND;
    } else if (cpl_bivector_get_size(self) > imax) {
        cpl_vector_set_size(xself, imax);
        cpl_vector_set_size(yself, imax);
    }

    /* Propagate error, if any */
    return cpl_error_set(cpl_func, error);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Shift self by the amount that maximizes the cross-correlation
  @param    self   1D-Dispersion relation to shift, at least of degree 1
  @param    obs    The observed spectrum to correlate against
  @param    model  Pointer to model parameters
  @param    filler The function to fill the model spectrum
  @param    hsize  Half the search-distance, hsize > 0 [pixel]
  @param    doplot Plot the cross-correlation as a function of pixel shift
  @param    pxc    Iff non-NULL, set *pxc to cross-correlation on success
  @return   CPL_ERROR_NONE on success, otherwise the relevant CPL error code

 */
/*----------------------------------------------------------------------------*/
cpl_error_code
irplib_polynomial_shift_1d_from_correlation(cpl_polynomial * self,
                                            const cpl_vector * obs,
                                            irplib_base_spectrum_model * model,
                                            cpl_error_code (*filler)
                                            (cpl_vector *,
                                             const cpl_polynomial *,
                                             irplib_base_spectrum_model *),
                                            int hsize,
                                            cpl_boolean doplot,
                                            double * pxc)
{

    const int      nobs   = cpl_vector_get_size(obs);
    const int      nmodel = 2 * hsize + nobs;
    cpl_vector   * mspec1d;
    cpl_vector   * xcorr;
    cpl_error_code error;
    int            ixc, xxc;
    double         xc;

    cpl_ensure_code(self   != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(obs    != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(model  != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(filler != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(hsize  >  0,    CPL_ERROR_ILLEGAL_INPUT);

    /* Shift reference by -hsize so filler can be used without offset */
    cpl_ensure_code(!cpl_polynomial_shift_1d(self, 0, -hsize),
                    cpl_error_get_code());

    mspec1d = cpl_vector_new(nmodel);

    if (filler(mspec1d, self, model)) {
        cpl_vector_delete(mspec1d);
        cpl_ensure_code(0, cpl_error_get_code());
    }

    /* Should not be able to fail now */
    xcorr = cpl_vector_new(1 + 2 * hsize);
    ixc = cpl_vector_correlate(xcorr, mspec1d, obs);

#ifdef IRPLIB_SPC_DUMP
    /* Need irplib_wavecal.c rev. 1.12 through 1.15 */
    irplib_polynomial_dump_corr_step(self, xcorr, "Shift");
#endif

    cpl_vector_delete(mspec1d);

    error = cpl_polynomial_shift_1d(self, 0, (double)ixc);

    xc = cpl_vector_get(xcorr, ixc);

    xxc = ixc - hsize; /* The effect of the two shifts */

    cpl_msg_info(cpl_func, "Shifting %d pixels (%g < %g)", xxc,
                 cpl_vector_get(xcorr, hsize), xc);

    if (doplot) {
        cpl_vector * xvals = cpl_vector_new(1 + 2 * hsize);
        cpl_bivector * bcorr = cpl_bivector_wrap_vectors(xvals, xcorr);
        int i;
        double x = (double)-hsize;
        char * title = cpl_sprintf("t 'Cross-correlation of shifted %d-pixel "
                                   "spectrum (XCmax=%g at %d)' w linespoints",
                                   nobs, cpl_vector_get(xcorr, ixc), xxc);

        for (i = 0; i < 1 + 2 * hsize; i++, x += 1.0) {
            cpl_vector_set(xvals, i, x);
        }

        cpl_plot_bivector("set grid;set xlabel 'Offset [pixel]';", title,
                             "", bcorr);
        cpl_bivector_unwrap_vectors(bcorr);
        cpl_vector_delete(xvals);
        cpl_free(title);
    }

    cpl_vector_delete(xcorr);

    cpl_ensure_code(!error, error);

    if (pxc != NULL) *pxc = xc;

    return CPL_ERROR_NONE;

}


/*----------------------------------------------------------------------------*/
/**
  @brief   Generate a 1D spectrum from (arc) lines and a dispersion relation
  @param   self    Vector to fill with spectrum
  @param   linepix Vector to update with best guess of line pixel position
  @param   erftmp  Vector with temporary buffer for erf() values
  @param   disp    1D-Dispersion relation, at least of degree 1
  @param   lines   Catalogue of lines, with increasing wavelengths 
  @param   wslit   Positive width of the slit
  @param   wfwhm   Positive FWHM of the transfer function
  @param   xtrunc  Truncate the line profile beyond distance xtrunc, xtrunc > 0
  @param   hsize   The 1st intensity in self will be disp(1-hsize), hsize >= 0
  @param   dofast  Iff true compose profile from pairs of two integer-placed
  @param   dolog   Iff true log(1+I) is used for the (positive) intensities
  @param   pulines Iff non-NULL, number of lines used, on success
  @return  CPL_ERROR_NONE on success, otherwise the relevant CPL error code
  @see irplib_vector_fill_line_spectrum()
  @note This function is supposed to be called via
          irplib_vector_fill_line_spectrum().

 */
/*----------------------------------------------------------------------------*/
cpl_error_code
irplib_vector_fill_line_spectrum_model(cpl_vector * self,
                                       cpl_vector * linepix,
                                       cpl_vector * erftmp,
                                       const cpl_polynomial * disp,
                                       const cpl_bivector * lines,
                                       double wslit,
                                       double wfwhm,
                                       double xtrunc,
                                       int hsize,
                                       cpl_boolean dofast,
                                       cpl_boolean dolog,
                                       cpl_size * pulines)
{

    cpl_errorstate     prestate;
    const double       sigma = wfwhm * CPL_MATH_SIG_FWHM;
    const cpl_vector * xlines  = cpl_bivector_get_x_const(lines);
    const double     * dxlines = cpl_vector_get_data_const(xlines);
    const double     * dylines = cpl_bivector_get_y_data_const(lines);
    double           * plinepix
        = linepix ? cpl_vector_get_data(linepix) : NULL;
    const int          nlines  = cpl_vector_get_size(xlines);
    const int          nself   = cpl_vector_get_size(self);
    double           * dself   = cpl_vector_get_data(self);
    cpl_polynomial   * dispi;
    double           * profile = NULL;
    const cpl_size     i0 = 0;
    const double       p0 = cpl_polynomial_get_coeff(disp, &i0);
    double             wl;
    double             xpos = (double)(1-hsize)-xtrunc;
    const double       xmax = (double)(nself-hsize)+xtrunc;
    double             xderiv, xextreme;
    cpl_error_code     error = CPL_ERROR_NONE;
    int                iline;
    cpl_size           ulines = 0;

    cpl_ensure_code(self    != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(disp    != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(lines   != NULL, CPL_ERROR_NULL_INPUT);

    cpl_ensure_code(wslit  >  0.0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(wfwhm  >  0.0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(hsize  >= 0,   CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(xtrunc >  0.0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(nself  > 2 * hsize, CPL_ERROR_ILLEGAL_INPUT);

    cpl_ensure_code(cpl_polynomial_get_dimension(disp) == 1,
                    CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(cpl_polynomial_get_degree(disp) > 0,
                    CPL_ERROR_ILLEGAL_INPUT);

    /* The smallest wavelength contributing to the spectrum. */
    wl = cpl_polynomial_eval_1d(disp, xpos, &xderiv);

    if (wl <= 0.0) return
        cpl_error_set_message_macro(cpl_func, CPL_ERROR_ILLEGAL_INPUT, __FILE__,
                                    __LINE__, "Non-positive wavelength at x=%g: "
                                    "P(x)=%g, P'(x)=%g", xpos, wl, xderiv);

    if (xderiv <= 0.0) return
        cpl_error_set_message_macro(cpl_func, CPL_ERROR_ILLEGAL_INPUT, __FILE__,
                                    __LINE__, "Non-increasing dispersion at "
                                    "x=%g: P'(x)=%g, P(x)=%g", xpos, xderiv, wl);

    /* Find the 1st line */
    iline = cpl_vector_find(xlines, wl);

    /* The first line must be at least at wl */
    if (dxlines[iline] < wl) iline++;

    if (iline >= nlines) return
        cpl_error_set_message_macro(cpl_func, CPL_ERROR_DATA_NOT_FOUND, __FILE__,
                                    __LINE__, "The %d-line catalogue has only "
                                    "lines below P(%g)=%g > %g", nlines, xpos,
                                    wl, dxlines[nlines-1]);

    memset(dself, 0, nself * sizeof(double));

    dispi = cpl_polynomial_duplicate(disp);

    /* Verify monotony of dispersion */
    cpl_polynomial_derivative(dispi, 0);

    prestate = cpl_errorstate_get();

    if (cpl_polynomial_solve_1d(dispi, 0.5*(nlines+1), &xextreme, 1)) {
        cpl_errorstate_set(prestate);
    } else if (xpos < xextreme && xextreme < xmax) {
        cpl_polynomial_delete(dispi);
        return cpl_error_set_message_macro(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                                           __FILE__, __LINE__, "Non-monotone "
                                           "dispersion at x=%g: P'(x)=0, "
                                           "P(x)=%g", xextreme,
                                           cpl_polynomial_eval_1d(disp, xextreme,
                                                                  NULL));
    }

    if (dofast) {
        const int npix = 1+(int)xtrunc;

        if (erftmp != NULL && cpl_vector_get_size(erftmp) == npix &&
            cpl_vector_get(erftmp, 0) > 0.0) {
            profile = cpl_vector_get_data(erftmp);
        } else {

            const double yval =  0.5 / wslit;
            const double x0p  =  0.5 * wslit + 0.5;
            const double x0n  = -0.5 * wslit + 0.5;
            double       x1diff
                = irplib_erf_antideriv(x0p, sigma)
                - irplib_erf_antideriv(x0n, sigma);
            int          ipix;

            if (erftmp == NULL) {
                profile = (double*)cpl_malloc(sizeof(double)*(size_t)npix);
            } else {
                cpl_vector_set_size(erftmp, npix);
                profile = cpl_vector_get_data(erftmp);
            }

            profile[0] = 2.0 * yval * x1diff;

            for (ipix = 1; ipix < npix; ipix++) {
                const double x1 = (double)ipix;
                const double x1p = x1 + 0.5 * wslit + 0.5;
                const double x1n = x1 - 0.5 * wslit + 0.5;
                const double x0diff = x1diff;

                x1diff = irplib_erf_antideriv(x1p, sigma)
                    - irplib_erf_antideriv(x1n, sigma);

                profile[ipix] = yval * (x1diff - x0diff);

            }
        }
    }

    cpl_polynomial_copy(dispi, disp);

    /* FIXME: A custom version of cpl_polynomial_solve_1d() which returns
       P'(xpos) can be used for the 1st NR-iteration. */
    /* Further, the sign of P'(xpos) could be checked for all lines. */
    /* Perform 1st NR-iteration in solving for P(xpos) = dxlines[iline] */
    xpos -= (wl - dxlines[iline]) / xderiv;

    /* Iterate through the lines */
    for (; !error && iline < nlines; iline++) {

        /* Lines may have a non-physical intensity (e.g. zero) to indicate some
           property of the line, e.g. unknown intensity due to blending */
        if (dylines[iline] <= 0.0) continue;

        /* Use 1st guess, if available (Use 0.0 to flag unavailable) */
        if (plinepix != NULL && plinepix[iline] > 0.0) xpos = plinepix[iline];

        if (xpos > xmax) xpos = xmax; /* FIXME: Better to limit xpos ? */

        /* Find the (sub-) pixel position of the line */
        error = cpl_polynomial_set_coeff(dispi, &i0, p0 - dxlines[iline]) ||
            cpl_polynomial_solve_1d(dispi, xpos, &xpos, 1);

        if (xpos > xmax) {
            if (error) {
                error = 0;
                cpl_msg_debug(cpl_func, "Stopping spectrum fill at line %d/%d "
                             "at xpos=%g > xmax=%g",
                             iline, nlines, xpos, xmax);
                cpl_errorstate_dump(prestate, CPL_FALSE,
                                    irplib_errorstate_dump_debug);
                cpl_errorstate_set(prestate);
            }
            break;
        } else if (error) {
            if (linepix != NULL && ulines) (void)cpl_vector_fill(linepix, 0.0);
            (void)cpl_error_set_message_macro(cpl_func, cpl_error_get_code(),
                                              __FILE__, __LINE__,
                                              "Could not find pixel-position "
                                              "of line %d/%d at wavelength=%g."
                                              " xpos=%g, xmax=%g",
                                              iline, nlines, dxlines[iline],
                                              xpos, xmax);
            break;
        } else if (dofast) {
            const double frac  = fabs(xpos - floor(xpos));
#ifdef IRPLIB_WAVECAL_FAST_FAST
            const double frac0 = 1.0 - frac; /* Weight opposite of distance */
#else
            /* Center intensity correctly */
            const double ep1pw = irplib_erf_antideriv(frac + 0.5 * wslit, sigma);
            const double en1pw = irplib_erf_antideriv(frac + 0.5 * wslit - 1.0,
                                                      sigma);
            const double ep1nw = irplib_erf_antideriv(frac - 0.5 * wslit, sigma);
            const double en1nw = irplib_erf_antideriv(frac - 0.5 * wslit - 1.0,
                                                      sigma);
            const double frac0
                = (en1nw - en1pw) / (ep1pw - en1pw - ep1nw + en1nw);

#endif
            const double frac1 = 1.0 - frac0;
            const double yval0 = frac0 * dylines[iline];
            const double yval1 = frac1 * dylines[iline];
            const int    npix  = 1+(int)xtrunc;
            int          ipix;
            int          i0n    = hsize - 1 + floor(xpos);
            int          i0p    = i0n;
            int          i1n    = i0n + 1;
            int          i1p    = i1n;
            cpl_boolean  didline = CPL_FALSE;


            /* Update 1st guess for next time, if available */
            if (plinepix != NULL) plinepix[iline] =  xpos;

            if (frac0 < 0.0) {
                (void)cpl_error_set_message_macro(cpl_func,
                                                  CPL_ERROR_UNSPECIFIED,
                                                  __FILE__, __LINE__,
                                                  "Illegal split at x=%g: %g + "
                                                  "%g = 1", xpos, frac0, frac1);
#ifdef IRPLIB_WAVEVAL_DEBUG
            } else {
                cpl_msg_warning(cpl_func,"profile split at x=%g: %g + %g = 1",
                                xpos, frac0, frac1);
#endif
            }

            for (ipix = 0; ipix < npix; ipix++, i0n--, i0p++, i1n--, i1p++) {

                if (i0n >= 0 && i0n < nself) {
                    dself[i0n] += yval0 * profile[ipix];
                    didline = CPL_TRUE;
                }
                if (i1n >= 0 && i1n < nself && ipix + 1 < npix) {
                    dself[i1n] += yval1 * profile[ipix+1];
                    didline = CPL_TRUE;
                }

                if (ipix == 0) continue;

                if (i0p >= 0 && i0p < nself) {
                    dself[i0p] += yval0 * profile[ipix];
                    didline = CPL_TRUE;
                }
                if (i1p >= 0 && i1p < nself && ipix + 1 < npix) {
                    dself[i1p] += yval1 * profile[ipix+1];
                    didline = CPL_TRUE;
                }
            }

            if (didline) ulines++;

        } else {
            const double yval = 0.5 * dylines[iline] / wslit;
            const int    ifirst = IRPLIB_MAX((int)(xpos-xtrunc+0.5), 1-hsize);
            const int    ilast  = IRPLIB_MIN((int)(xpos+xtrunc), nself-hsize);
            int          ipix;
            const double x0 = (double)ifirst - xpos;
            const double x0p = x0 + 0.5*wslit - 0.5;
            const double x0n = x0 - 0.5*wslit - 0.5;
            double       x1diff
                = irplib_erf_antideriv(x0p, sigma)
                - irplib_erf_antideriv(x0n, sigma);

            /* Update 1st guess for next time, if available */
            if (plinepix != NULL) plinepix[iline] =  xpos;

            if (ilast >= ifirst) ulines++;

            for (ipix = ifirst; ipix <= ilast; ipix++) {
                const double x1 = (double)ipix - xpos;
                const double x1p = x1 + 0.5*wslit + 0.5;
                const double x1n = x1 - 0.5*wslit + 0.5;
                const double x0diff = x1diff;

                x1diff = irplib_erf_antideriv(x1p, sigma)
                    - irplib_erf_antideriv(x1n, sigma);

                dself[ipix+hsize-1] += yval * (x1diff - x0diff);

            }
        }
    }

    cpl_polynomial_delete(dispi);
    if (erftmp == NULL) cpl_free(profile);

    cpl_ensure_code(!error, cpl_error_get_code());

    if (dolog) {
        int i;
        for (i = 0; i < nself; i++) {
            dself[i] = dself[i] > 0.0 ? log(1.0 + dself[i]) : 0.0;
        }
    }

    if (!ulines) return
        cpl_error_set_message_macro(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                                    __FILE__, __LINE__, "The %d-line "
                                    "catalogue has no lines in the range "
                                    "%g -> P(%g)=%g", nlines, wl, xmax,
                                    cpl_polynomial_eval_1d(disp, xmax, NULL));

    if (pulines != NULL) *pulines = ulines;

    return CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    The antiderivative of erx(x/sigma/sqrt(2)) with respect to x
  @param    x      x
  @param    sigma  sigma
  @return   The antiderivative
  @note This function is even.

 */
/*----------------------------------------------------------------------------*/
inline double irplib_erf_antideriv(double x, double sigma)
{
    return x * erf( x / (sigma * CPL_MATH_SQRT2))
       + 2.0 * sigma/CPL_MATH_SQRT2PI * exp(-0.5 * x * x / (sigma * sigma));
}


#ifdef HAVE_GSL

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute minus the cross-correlation
  @param    self  The 1D-dispersion relation
  @param    data  Pointer to a irplib_multimin struct
  @return   Minus the cross-correlation or GSL_NAN on error
 */
/*----------------------------------------------------------------------------*/
static double irplib_gsl_correlation(const gsl_vector * self, void * data)
{

    irplib_multimin * mindata = (irplib_multimin *)data;
    cpl_errorstate prestate = cpl_errorstate_get();
    int nobs, nmodel, ndiff;
    cpl_size i;

    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, GSL_NAN);
    cpl_ensure(data != NULL, CPL_ERROR_NULL_INPUT, GSL_NAN);

    cpl_ensure(mindata->filler   != NULL, CPL_ERROR_NULL_INPUT, GSL_NAN);
    cpl_ensure(mindata->observed != NULL, CPL_ERROR_NULL_INPUT, GSL_NAN);
    cpl_ensure(mindata->spectrum != NULL, CPL_ERROR_NULL_INPUT, GSL_NAN);

    nobs   = cpl_vector_get_size(mindata->observed);
    nmodel = cpl_vector_get_size(mindata->spectrum);
    ndiff  = nmodel - nobs;

    cpl_ensure((ndiff & 1) == 0, CPL_ERROR_ILLEGAL_INPUT, GSL_NAN);

    cpl_ensure(cpl_vector_get_size(mindata->vxc) == 1 + ndiff,
               CPL_ERROR_ILLEGAL_INPUT, GSL_NAN);

    ndiff /= 2;

    for (i=0; i < (cpl_size)self->size; i++) {
        const double value = gsl_vector_get(self, (size_t)i);
        cpl_polynomial_set_coeff(mindata->disp1d, &i, value);
    }

    /* Shift reference by -ndiff so filler can be used without offset.
       The subsequent polynomial shift is reduced by -ndiff. */
    cpl_ensure_code(!cpl_polynomial_shift_1d(mindata->disp1d, 0, -ndiff),
                    cpl_error_get_code());

    if (mindata->filler(mindata->spectrum, mindata->disp1d,
                        mindata->param)
        || !cpl_errorstate_is_equal(prestate)) {

        /* The fill failed. Ensure the discarding of this candidate by
           setting the cross-correlation to its minimum possible value. */

        (void)cpl_vector_fill(mindata->vxc, -1.0);

        mindata->maxxc = ndiff;

        if (!cpl_errorstate_is_equal(prestate)) {
                cpl_msg_debug(cpl_func, "Spectrum fill failed:");
                cpl_errorstate_dump(prestate, CPL_FALSE,
                                    irplib_errorstate_dump_debug);
                cpl_errorstate_set(prestate);
        }
    } else {

        mindata->maxxc = cpl_vector_correlate(mindata->vxc,
                                              mindata->spectrum,
                                              mindata->observed);
    }

#ifdef IRPLIB_SPC_DUMP
    /* Need irplib_wavecal.c rev. 1.12 through 1.15 */
    irplib_polynomial_dump_corr_step(mindata->disp1d, mindata->vxc,
                                     "Optimize");
#endif

    mindata->xc = cpl_vector_get(mindata->vxc, ndiff);

    if (mindata->maxxc != ndiff &&
        cpl_vector_get(mindata->vxc, mindata->maxxc) > mindata->mxc) {
        const irplib_base_spectrum_model * arclamp
            = (const irplib_base_spectrum_model *)mindata->param;

        if (mindata->mdisp == NULL) {
            mindata->mdisp = cpl_polynomial_duplicate(mindata->disp1d);
        } else {
            cpl_polynomial_copy(mindata->mdisp, mindata->disp1d);
        }
        mindata->mxc = cpl_vector_get(mindata->vxc, mindata->maxxc);
        mindata->ishift = mindata->maxxc; /* Offset -ndiff pre-shifted above */
        cpl_msg_debug(cpl_func, "Local maximum: %g(%d) > %g(%d) (cost=%u:%u. "
                      "lines=%u)", mindata->mxc, mindata->maxxc, mindata->xc,
                      ndiff, (unsigned)arclamp->cost, (unsigned)arclamp->xcost,
                      (unsigned)arclamp->ulines);
    }

    return -mindata->xc;
}

#endif

/*----------------------------------------------------------------------------*/
/**
  @brief Modify self by maximizing the cross-correlation across all maxima
  @param self    1D-Dispersion relation to modify, at least of degree 1
  @param maxdeg  Maximize the cross-correlation by modifying maxdeg degree
  @param obs     The observed spectrum to correlate against
  @param nmaxima Number of local maxima to try (0 for all, 1 for global only)
  @param linelim Maximum number of lines allowed in iterative refinement
  @param model   The model of the lines/OTF etc.
  @param filler  The function to fill the model spectrum
  @param pixtol  The (positive) dispersion tolerance, e.g. 1e-6
  @param pixstep The step length used in the maximization, e.g. 0.5 [pixel]
  @param hsize   Half the search-distance to ensure a global-maximum, hsize >= 0
  @param maxite  Maximum number of iterations, e.g. 100 * maxdeg
  @param maxfail Number of retries on failure
  @param maxcont Number of retries on non-convergence
  @param doplot  Plot the cross-correlation as a function of pixel shift
  @param pxc     On success, *pxc is the cross-correlation
  @return CPL_ERROR_NONE on success, otherwise the relevant CPL error code
  @note Fails with CPL_ERROR_UNSUPPORTED_MODE if compiled without GSL.
        self must be increasing in the interval from 1 to the length of obs.

 */
/*----------------------------------------------------------------------------*/
cpl_error_code
irplib_polynomial_find_1d_from_correlation_all(cpl_polynomial * self,
                                               int maxdeg,
                                               const cpl_vector * obs,
                                               int nmaxima,
                                               int linelim,
                                               irplib_base_spectrum_model* model,
                                               cpl_error_code (* filler)
                                               (cpl_vector *,
                                                const cpl_polynomial *,
                                                irplib_base_spectrum_model *),
                                               double pixtol,
                                               double pixstep,
                                               int hsize,
                                               int maxite,
                                               int maxfail,
                                               int maxcont,
                                               cpl_boolean doplot,
                                               double * pxc)
{

#ifdef HAVE_GSL

    cpl_errorstate     prestate = cpl_errorstate_get();
    cpl_polynomial   * start;
    cpl_polynomial   * cand;
    cpl_polynomial   * backup;
    cpl_error_code     error = CPL_ERROR_NONE;
    double             xc;
    cpl_bivector     * xtshift = cpl_bivector_new(nmaxima ? nmaxima : 1);
    const cpl_vector * xtshiftx = cpl_bivector_get_x_const(xtshift);
    const cpl_vector * xtshifty = cpl_bivector_get_y_const(xtshift);
    int                nshift;
    int                imaximum = -1;
    int                imaxima;

#endif

    cpl_ensure_code(self   != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(obs    != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(model  != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(filler != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(pxc    != NULL, CPL_ERROR_NULL_INPUT);

    cpl_ensure_code(cpl_polynomial_get_dimension(self) == 1,
                    CPL_ERROR_ILLEGAL_INPUT);

    cpl_ensure_code(cpl_polynomial_get_degree(self) > 0,
                    CPL_ERROR_ILLEGAL_INPUT);

    cpl_ensure_code(maxdeg  >=  0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(pixtol  > 0.0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(pixstep > 0.0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(hsize   >=  0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(maxite  >=  0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(nmaxima >=  0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(maxfail >   0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(maxcont >   0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(linelim >=  0, CPL_ERROR_ILLEGAL_INPUT);

#ifndef HAVE_GSL
    /* Avoid unused variable warning */
    cpl_ensure_code(doplot == CPL_TRUE || doplot == CPL_FALSE,
                    CPL_ERROR_ILLEGAL_INPUT);
    return cpl_error_set_message(cpl_func, CPL_ERROR_UNSUPPORTED_MODE,
                                 "GSL is not available");
#else

    if (irplib_bivector_find_shift_from_correlation(xtshift, self, obs,
                                                    model, filler,
                                                    hsize, doplot, &xc)) {
        cpl_bivector_delete(xtshift);
        return cpl_error_set_where(cpl_func);
    }

    if (model->ulines > (cpl_size)linelim) {
        /* The initial, optimal (integer) shift */
        const double xxc = cpl_vector_get(xtshiftx, 0);
        const double xc0 = cpl_vector_get(xtshifty, 0);

        cpl_msg_warning(cpl_func, "Doing only shift=%g pixels with lines=%u > "
                        "%d and XC=%g", xxc, (unsigned)model->ulines, linelim,
                        xc0);

        cpl_polynomial_shift_1d(self, 0, xxc);

        *pxc = xc0;

        cpl_bivector_delete(xtshift);

        return CPL_ERROR_NONE;
    }

    start  = cpl_polynomial_duplicate(self);
    cand   = cpl_polynomial_new(1);
    backup = cpl_polynomial_new(1);

    /* Number of (local) maxima to use as starting point of the optimization */
    nshift = cpl_bivector_get_size(xtshift);
    if (nmaxima == 0 || nmaxima > nshift) nmaxima = nshift;

    cpl_msg_info(cpl_func, "Optimizing %d/%d local shift-maxima "
                 "(no-shift xc=%g. linelim=%d)", nmaxima, nshift, xc, linelim);
    if (cpl_msg_get_level() <= CPL_MSG_DEBUG)
        cpl_bivector_dump(xtshift, stdout);

    for (imaxima = 0; imaxima < nmaxima; imaxima++) {
        /* The initial, optimal (integer) shift */
        const double xxc = cpl_vector_get(xtshiftx, imaxima);
        double xtpixstep = pixstep;
        double xtpixtol  = pixtol;
        double xtxc;
        cpl_boolean ok = CPL_FALSE;
        int nfail;


        cpl_polynomial_copy(cand, start);
        cpl_polynomial_shift_1d(cand, 0, xxc);
        cpl_polynomial_copy(backup, cand);

        /* Increase tolerance until convergence */
        for (nfail = 0; nfail < maxfail; nfail++, xtpixtol *= 2.0,
                 xtpixstep *= 2.0) {
            int restart = maxcont;
            cpl_boolean redo;

            do {
                if (error) {
                    cpl_errorstate_dump(prestate, CPL_FALSE,
                                        irplib_errorstate_dump_debug);
                    cpl_errorstate_set(prestate);
                }
                error = irplib_polynomial_find_1d_from_correlation_
                    (cand, maxdeg, obs, model,
                     filler, xtpixtol, xtpixstep, 2,
                     maxite, &xtxc, &redo);
                if (redo && !error) error = CPL_ERROR_CONTINUE;
            } while (((!error && redo) || error == CPL_ERROR_CONTINUE)
                     && --restart);

            if (!error && !redo) {
                cpl_msg_debug(cpl_func, "XC(imax=%d/%d:xtpixtol=%g): %g "
                              "(cost=%u:%u)", 1+imaxima, nmaxima, xtpixtol,
                              xtxc, (unsigned)model->cost,
                              (unsigned)model->xcost);
                break;
            }
            cpl_msg_warning(cpl_func, "Increasing xtpixtol from %g (%g, imax="
                            "%d/%d)", xtpixtol, xtpixstep, 1+imaxima, nmaxima);
            if (model->ulines > (cpl_size)linelim) {
                cpl_msg_warning(cpl_func, "Stopping search-refinement via "
                                "catalogue with %u lines > %d",
                                (unsigned)model->ulines, linelim);
                break;
            }
            cpl_polynomial_copy(cand, start);
        }

        /* Decrease tolerance until divergence, keep previous */
        for (; !error && xtpixtol > 0.0; xtpixtol *= 0.25, xtpixstep *= 0.5) {
            int restart = maxcont;
            cpl_boolean redo;

            cpl_polynomial_copy(backup, cand);
            do {
                if (error) {
                    cpl_errorstate_dump(prestate, CPL_FALSE,
                                        irplib_errorstate_dump_debug);
                    cpl_errorstate_set(prestate);
                }
                error = irplib_polynomial_find_1d_from_correlation_
                    (cand, maxdeg, obs, model, filler,
                     xtpixtol, xtpixstep, 2, maxite, &xtxc, &redo);
                if (redo && !error) error = CPL_ERROR_CONTINUE;
            } while (((!error && redo) || error == CPL_ERROR_CONTINUE)
                     && --restart);
            if (error) break;
            ok = CPL_TRUE;
            if (redo) break;
            cpl_msg_debug(cpl_func, "XC(imax=%d/%d:xtpixtol=%g): %g (cost=%u:%u"
                          ". ulines=%u)", 1+imaxima, nmaxima, xtpixtol, xtxc,
                          (unsigned)model->cost, (unsigned)model->xcost,
                          (unsigned)model->ulines);
            if (model->ulines > (cpl_size)linelim) {
                cpl_msg_info(cpl_func, "Stopping search-refinement via "
                             "catalogue with %u lines > %u",
                             (unsigned)model->ulines, linelim);
                break;
            }
        }

        if (error) {
            error = 0;
            cpl_errorstate_dump(prestate, CPL_FALSE,
                                irplib_errorstate_dump_debug);
            cpl_errorstate_set(prestate);
            cpl_polynomial_copy(cand, backup);
        }
        if (ok && xtxc > xc) {
            imaximum = imaxima;
            cpl_polynomial_copy(self, cand);
            xc = xtxc;

            cpl_msg_info(cpl_func, "XC(imax=%d/%d): %g -> %g (initial-shift=%g. "
                         "cost=%u:%u. lines=%u)", 1+imaxima, nmaxima,
                         cpl_vector_get(xtshifty, imaxima), xtxc,
                         cpl_vector_get(xtshiftx, imaxima),
                         (unsigned)model->cost, (unsigned)model->xcost,
                         (unsigned)model->ulines);
        } else {
            cpl_msg_info(cpl_func, "xc(imax=%d/%d): %g -> %g (initial-shift=%g. "
                         "cost=%u:%u. lines=%u)", 1+imaxima, nmaxima,
                         cpl_vector_get(xtshifty, imaxima), xtxc,
                         cpl_vector_get(xtshiftx, imaxima),
                         (unsigned)model->cost, (unsigned)model->xcost,
                         (unsigned)model->ulines);
        }
    }

    cpl_polynomial_delete(start);
    cpl_polynomial_delete(backup);
    cpl_polynomial_delete(cand);

    if (imaximum < 0) {
      /* The initial, optimal (integer) shift */
        const double xxc = cpl_vector_get(xtshiftx, 0);
        const double xc0 = cpl_vector_get(xtshifty, 0);

        error = cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                                      "Could not improve XC=%g over %d "
                                      "local shift-maxima, best at shift %g",
                                      xc0, nmaxima, xxc);
    } else {
        cpl_msg_info(cpl_func, "Maximal XC=%g (up from %g, with initial pixel-"
                     "shift of %g) at %d/%d local shift-maximi", xc,
                     cpl_vector_get(xtshifty, imaximum),
                     cpl_vector_get(xtshiftx, imaximum),
                     1+imaximum, nmaxima);

        if (doplot) {
            irplib_plot_spectrum_and_model(obs, self, model, filler);
        }

        *pxc = xc;
    }

    cpl_bivector_delete(xtshift);

    return error;

#endif

}
/**@}*/
