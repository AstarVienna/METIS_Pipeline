/*
 * This file is part of the HDRL
 * Copyright (C) 2017 European Southern Observatory
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

#include "hdrl_spectrum_resample.h"
#include "hdrl_spectrum_defs.h"
#include "hdrl_parameter.h"
#include "hdrl_utils.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <gsl/gsl_blas.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_bspline.h>
#include <gsl/gsl_multifit.h>

/*-----------------------------------------------------------------------------
                     Private Functions and Data Structures
 -----------------------------------------------------------------------------*/
/*Allocate gsl workspace*/
static inline
gsl_bspline_workspace * alloc_workspace(int k, int nCoeff,
        const double * x_dest, const cpl_size sample_len);

/*Filling gsl structs for fitting*/
static inline int
fit_matrixes(const double * x_raw, const double * y_raw,
        const cpl_size sample_len, const int nCoeff,
        gsl_bspline_workspace * ws, gsl_vector* B,
        gsl_vector* c, gsl_matrix * cov);

/*Private function used for the fit*/
static inline cpl_error_code
hdrl_spectrum1D_bspline_fit_internal
(const double* x_raw, const double* y_raw, const cpl_size sample_len,
const cpl_array * lambdas_dest, const cpl_size lambdas_dest_start,
const cpl_size lambdas_dest_stop, cpl_image * flux_dest,
const int k, const int nCoeff);

/*Private function used for the windowed fit*/
static inline cpl_error_code
hdrl_spectrum1D_fit_windowed_internal
(const double* x_raw, const double* y_raw, const cpl_size sample_len,
 const cpl_array * lambdas_dest, cpl_image * flux_dest, const int k,
 const int nCoeff, const long window, const double factor);

/* finds the closest element to l in arr*/
static inline cpl_size
get_closest_lambda(const double *arr, const cpl_size num_els, const double l);

/*See doc for hdrl_spectrum1D_resample*/
static inline hdrl_spectrum1D *
resample_internal(const hdrl_spectrum1D * self,
        const cpl_array * lambdas_dest, const hdrl_parameter * par);


static inline cpl_error_code
integrate_internal(double * x, const double * y, const double * y_var,
        const cpl_size sample_len, const cpl_array * arg_lambdas_dest,
        hdrl_image * flux_dest);

static inline cpl_boolean
is_destination_outside_source_spectrum(const double * source,
        const cpl_size source_length, const double dest_start,
        const double dest_stop);

static inline double
integrate(const double start_dest, const double stop_dest,
        cpl_size * source_idx, const double * x, const double * y,
        const cpl_size sample_len);

static inline double
get_stop(const double * v, const cpl_size length, const cpl_size i);

static inline double
get_start(const double * v, const cpl_size i);


/*Parameter used for the interpolation*/
typedef struct {
    HDRL_PARAMETER_HEAD;
    hdrl_spectrum1D_interpolation_method method;
} hdrl_spectrum1D_resample_interpolate_parameter;

static hdrl_parameter_typeobj
hdrl_spectrum1D_resample_interpolate_parameter_type = {
    HDRL_PARAMETER_SPECTRUM1D_RESAMPLE_INTERPOLATE,            /* type */
    (hdrl_alloc *)&cpl_malloc,                                 /* fp_alloc */
    (hdrl_free *)&cpl_free,                                    /* fp_free */
    NULL,                                                      /* fp_destroy */
    sizeof(hdrl_spectrum1D_resample_interpolate_parameter),    /* obj_size */
};

/*Parameter used for the integration*/
typedef struct {
    HDRL_PARAMETER_HEAD;
} hdrl_spectrum1D_resample_integrate_parameter;

static hdrl_parameter_typeobj
hdrl_spectrum1D_resample_integrate_parameter_type = {
    HDRL_PARAMETER_SPECTRUM1D_RESAMPLE_INTEGRATE,           /* type */
    (hdrl_alloc *)&cpl_malloc,                              /* fp_alloc */
    (hdrl_free *)&cpl_free,                                 /* fp_free */
    NULL,                                                   /* fp_destroy */
    sizeof(hdrl_spectrum1D_resample_integrate_parameter),   /* obj_size */
};


/*getter for the type of the interpolation*/
hdrl_spectrum1D_interpolation_method
hdrl_spectrum1D_resample_interpolate_parameter_get_method(const hdrl_parameter * par){
    cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, 0);
    cpl_ensure(hdrl_parameter_get_parameter_enum(par)
            == HDRL_PARAMETER_SPECTRUM1D_RESAMPLE_INTERPOLATE,
            CPL_ERROR_INCOMPATIBLE_INPUT, 0);

    const hdrl_spectrum1D_resample_interpolate_parameter * p =
             (const hdrl_spectrum1D_resample_interpolate_parameter *)par;

    return p->method;
}

/*Parameter used for the fit*/
typedef struct {
    HDRL_PARAMETER_HEAD;
    int k;
    int nCoeff;
    long window;
    double factor;
} hdrl_spectrum1D_resample_fit_parameter;

static hdrl_parameter_typeobj
hdrl_spectrum1D_resample_fit_parameter_type = {
    HDRL_PARAMETER_SPECTRUM1D_RESAMPLE_FIT,         /* type */
    (hdrl_alloc *)&cpl_malloc,                      /* fp_alloc */
    (hdrl_free *)&cpl_free,                         /* fp_free */
    NULL,                                           /* fp_destroy */
    sizeof(hdrl_spectrum1D_resample_fit_parameter), /* obj_size */
};

/*getter for the k parameter in fit*/
int hdrl_spectrum1D_resample_fit_parameter_get_k(const hdrl_parameter * par){

    cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, 0);
    cpl_ensure(hdrl_parameter_get_parameter_enum(par)
            == HDRL_PARAMETER_SPECTRUM1D_RESAMPLE_FIT,
            CPL_ERROR_INCOMPATIBLE_INPUT, 0);

    const hdrl_spectrum1D_resample_fit_parameter * p =
            (const hdrl_spectrum1D_resample_fit_parameter *)par;
    return p->k;
}

/*getter for the nCoeff parameter in fit*/
int hdrl_spectrum1D_resample_fit_parameter_get_nCoeff
(const hdrl_parameter * par){

    cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, 0);
    cpl_ensure(hdrl_parameter_get_parameter_enum(par)
            == HDRL_PARAMETER_SPECTRUM1D_RESAMPLE_FIT,
            CPL_ERROR_INCOMPATIBLE_INPUT, 0);

    const hdrl_spectrum1D_resample_fit_parameter * p =
            (const hdrl_spectrum1D_resample_fit_parameter *)par;
    return p->nCoeff;
}

/*getter for the window parameter in fit*/
long hdrl_spectrum1D_resample_fit_parameter_get_window
(const hdrl_parameter * par){

    cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, 0);
    cpl_ensure(hdrl_parameter_get_parameter_enum(par)
            == HDRL_PARAMETER_SPECTRUM1D_RESAMPLE_FIT,
            CPL_ERROR_INCOMPATIBLE_INPUT, 0);

    const hdrl_spectrum1D_resample_fit_parameter * p =
            (const hdrl_spectrum1D_resample_fit_parameter *)par;
    return p->window;
}

/*getter for the window parameter in fit*/
long hdrl_spectrum1D_resample_fit_parameter_get_factor
(const hdrl_parameter * par){

    cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, 0);
    cpl_ensure(hdrl_parameter_get_parameter_enum(par)
            == HDRL_PARAMETER_SPECTRUM1D_RESAMPLE_FIT,
            CPL_ERROR_INCOMPATIBLE_INPUT, 0);

    const hdrl_spectrum1D_resample_fit_parameter * p =
            (const hdrl_spectrum1D_resample_fit_parameter *)par;
    return p->factor;
}

/**
 * @addtogroup hdrl_spectrum1D
 * @{
 */
/*-----------------------------------------------------------------------------
                               Functions
 -----------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief constructor for the hdrl_parameter in the case of interpolation
 * @param method        interpolation methods, see the enum
 *                      hdrl_spectrum1D_interpolation_method
 *
 * @return the constructed parameter
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter* hdrl_spectrum1D_resample_interpolate_parameter_create
(const hdrl_spectrum1D_interpolation_method method){
    hdrl_spectrum1D_resample_interpolate_parameter * p
    = (hdrl_spectrum1D_resample_interpolate_parameter *)
       hdrl_parameter_new(&hdrl_spectrum1D_resample_interpolate_parameter_type);
    p->method = method;
    return (hdrl_parameter*) p;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief constructor for the hdrl_parameter in the case of integration
 *
 * @return the constructed parameter
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter* hdrl_spectrum1D_resample_integrate_parameter_create(void){
    hdrl_spectrum1D_resample_integrate_parameter * p
      = (hdrl_spectrum1D_resample_integrate_parameter *)
         hdrl_parameter_new(&hdrl_spectrum1D_resample_integrate_parameter_type);
    return (hdrl_parameter*) p;
}


hdrl_parameter * hdrl_spectrum1D_resample_interpolate_parameter_parse_parlist(
        const cpl_parameterlist * parlist, const char * prefix){

    cpl_ensure(prefix && parlist, CPL_ERROR_NULL_INPUT, NULL);

    /* Get the Method parameter */
    char * name = hdrl_join_string(".", 2, prefix, "method");

    const cpl_parameter * par = cpl_parameterlist_find_const(parlist, name);
    const char          * value = cpl_parameter_get_string(par);
    if (value == NULL) {
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                              "Parameter %s not found", name);
        cpl_free(name);
        return NULL;
    }

    hdrl_spectrum1D_interpolation_method met = hdrl_spectrum1D_interp_linear;
    /* Switch on the methods */
    if(!strcmp(value, "LINEAR"))
       met = hdrl_spectrum1D_interp_linear;

    else if(!strcmp(value, "CSPLINE"))
       met = hdrl_spectrum1D_interp_cspline;

    else if (!strcmp(value, "AKIMA"))
       met = hdrl_spectrum1D_interp_akima;

    else{
      cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                                    "Interpolation method %s not found", value);
      cpl_free(name);
      return NULL;
    }

    cpl_free(name);
    return hdrl_spectrum1D_resample_interpolate_parameter_create(met);
}

cpl_parameterlist *
hdrl_spectrum1D_resample_interpolate_parameter_create_parlist(const char * base_context,
        const char * prefix, const char * method_def) {

    cpl_ensure(base_context && prefix, CPL_ERROR_NULL_INPUT, NULL);
    char                *   name ;
    cpl_parameterlist   *   parlist = cpl_parameterlist_new();
    cpl_parameter       *   p ;
    char                *   context = hdrl_join_string(".", 2, base_context, prefix);

    /* --prefix.method */
    name = hdrl_join_string(".", 2, context, "method");
    p = cpl_parameter_new_enum(name, CPL_TYPE_STRING,
       "Method used for Spectrum1D interpolation", context,
       method_def, 3, "LINEAR", "CSPLINE", "AKIMA");
    cpl_free(name) ;
    name = hdrl_join_string(".", 2, prefix, "method");
    cpl_parameter_set_alias(p, CPL_PARAMETER_MODE_CLI, name);
    cpl_parameter_disable(p, CPL_PARAMETER_MODE_ENV);
    cpl_free(name) ;
    cpl_parameterlist_append(parlist, p);

    cpl_free(context);
    return parlist;
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief constructor for the hdrl_parameter in the case of interpolation
 * @param k             order of the B-spline
 * @param nCoeff        number of coefficients used for the fit
 *
 * @return the constructed parameter
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter* hdrl_spectrum1D_resample_fit_parameter_create(
        const int k, const int nCoeff){

    hdrl_spectrum1D_resample_fit_parameter * p
    = (hdrl_spectrum1D_resample_fit_parameter *)
       hdrl_parameter_new(&hdrl_spectrum1D_resample_fit_parameter_type);
    p->k = k;
    p->nCoeff = nCoeff;
    p->window = 0;
    p->factor = 1.0;
    return (hdrl_parameter*) p;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief constructor for the hdrl_parameter in the case of interpolation
 * @param k             order of the B-spline
 * @param nCoeff        number of coefficients used for the fit
 * @param window        number of destination wavelengths whose flux values are
 *                      computed using the same model
 * @param factor        Given window2 = window * factor. window2 is the number of
 *                      source wavelengths used to compute the fit model
 *
 * @return the constructed parameter, NULL in case of error
 *
 * @note window must be greater than 0 and factor greater or equal than 1.0
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter* hdrl_spectrum1D_resample_fit_windowed_parameter_create(
        const int k, const int nCoeff, const long window, const double factor){

    cpl_ensure(window > 0, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(factor >= 1.0,  CPL_ERROR_ILLEGAL_INPUT, NULL);

    hdrl_spectrum1D_resample_fit_parameter * p
    = (hdrl_spectrum1D_resample_fit_parameter *)
       hdrl_parameter_new(&hdrl_spectrum1D_resample_fit_parameter_type);
    p->k = k;
    p->nCoeff = nCoeff;
    p->window = window;
    p->factor = factor;
    return (hdrl_parameter*) p;
}

cpl_error_code
hdrl_resample_parameter_verify(const hdrl_parameter * par){

    cpl_ensure_code(par != NULL, CPL_ERROR_NULL_INPUT);

    hdrl_parameter_enum method = hdrl_parameter_get_parameter_enum(par);

    cpl_ensure_code(method == HDRL_PARAMETER_SPECTRUM1D_RESAMPLE_INTERPOLATE ||
                    method == HDRL_PARAMETER_SPECTRUM1D_RESAMPLE_FIT ||
                    method == HDRL_PARAMETER_SPECTRUM1D_RESAMPLE_INTEGRATE,
                    CPL_ERROR_INCOMPATIBLE_INPUT);

    return CPL_ERROR_NONE;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief resample a hdrl_spectrum1D on the wavelengths contained in waves
 * @param self          the spectrum to be resampled
 * @param waves         the waves the spectrum has to be resampled on
 * @param par           hdrl_parameter, see
 * hdrl_spectrum1D_resample_fit_parameter_create(),
 * hdrl_spectrum1D_resample_interpolate_parameter_create() or
 * hdrl_spectrum1D_resample_integrate_parameter_create().
 *
 * @return the resampled spectrum, NULL in case of error (see below).
 *
 * @note providing a spectrum with sorted, strictly monotonic increasing wavelength
 * values will provide the best performance. If the spectrum is not sorted, the
 * routine will create a copy of it and sort the copy. In case of duplicated
 * wavelengths they are collapsed in a single wavelength and the corresponding
 * flux is calculated as the median of the fluxes. Error propagation is performed
 * through linear interpolation of the variance (error^2) of the spectrum in case
 * of fit or interpolation. In case of integration we integrate the variance using
 * the same weights used for the flux. The integration is done assuming that the
 * flux is constant inside the bin and that the bin is centered on the sample.
 * The only exceptions are the first bin (it starts at the sample) and the last
 * bin (it ends at the sample).
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: if any of the pointers are NULL
 * - CPL_ERROR_INCOMPATIBLE_INPUT: if the scale of self and waves are different
 *  or if par cannot be casted to any of the resample parameters (fit,
 *  interpolation, integration)
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D *
hdrl_spectrum1D_resample(const hdrl_spectrum1D * self,
                         const hdrl_spectrum1D_wavelength* waves,
                         const hdrl_parameter* par){

    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(self-> flux != NULL, CPL_ERROR_NULL_INPUT, NULL);

    cpl_ensure(waves != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(waves->wavelength != NULL, CPL_ERROR_NULL_INPUT, NULL);

    cpl_ensure(self->wave_scale == waves->scale, CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    if(hdrl_resample_parameter_verify(par)) return NULL;

    /* if the two spectra are already compatible and we not fitting,
     * simply return a copy of self */
    hdrl_spectrum1D_wavelength self_w = hdrl_spectrum1D_get_wavelength(self);
    if(hdrl_spectrum1D_are_spectra_compatible(&self_w, waves) &&
    		hdrl_parameter_get_parameter_enum(par) != HDRL_PARAMETER_SPECTRUM1D_RESAMPLE_FIT)
        return hdrl_spectrum1D_duplicate(self);

    return resample_internal(self, waves->wavelength, par);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief resample a hdrl_spectrum1D on the wavelengths contained in waves
 * @param self          the spectrum to be resampled
 * @param waves         the waves the spectrum has to be resampled on
 * @param par           hdrl_parameter, see
 * hdrl_spectrum1D_resample_fit_parameter_create(),
 * hdrl_spectrum1D_resample_interpolate_parameter_create() or
 * hdrl_spectrum1D_resample_integrate_parameter_create().
 *
 * @return the resampled spectrum, NULL in case of error (see below).
 *
 * @note providing a spectrum with sorted, strictly monotonic increasing wavelength
 * values will provide the best performance. If the spectrum is not sorted, the
 * routine will create a copy of it and sort the copy. In case of duplicated
 * wavelengths they are collapsed in a single wavelength and the corresponding
 * flux is calculated as the median of the fluxes. Error propagation is performed
 * through linear interpolation of the variance (error^2) of the spectrum in case
 * of fit or interpolation. In case of integration we integrate the variance using
 * the same weights used for the flux. The integration is done assuming that the
 * flux is constant inside the bin and that the bin is centered on the sample.
 * The only exceptions are the first bin (it starts at the sample) and the last
 * bin (it ends at the sample).
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: if any of the pointers are NULL
 * - CPL_ERROR_INCOMPATIBLE_INPUT: if par cannot be casted to any of the resample
 *   parameters (fit, interpolation, integration)
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D *
hdrl_spectrum1D_resample_on_array(const hdrl_spectrum1D * self,
                                  const cpl_array * waves,
                                  const hdrl_parameter* par){

    cpl_ensure(waves != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(self-> flux != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, NULL);

    if(hdrl_resample_parameter_verify(par)) return NULL;

    /* if the two spectra are already compatible and we NOT fitting,
     * simply return a copy of self */
    hdrl_spectrum1D_wavelength self_w = hdrl_spectrum1D_get_wavelength(self);
    if(hdrl_parameter_get_parameter_enum(par) == HDRL_PARAMETER_SPECTRUM1D_RESAMPLE_INTERPOLATE &&
    		hdrl_spectrum1D_are_wavelengths_compatible(self_w.wavelength, waves))
        return hdrl_spectrum1D_duplicate(self);

    hdrl_spectrum1D * to_ret = resample_internal(self, waves, par);

    return to_ret;
}

/*-----------------------------------------------------------------------------
                    Private Functions Implementation
 -----------------------------------------------------------------------------*/

/*wrapper around the allocation function provided by gsl. It maps our enum in
 * gls types.*/
static inline gsl_spline * get_interp_spline
(const hdrl_spectrum1D_interpolation_method method, const int sample_len){

    switch(method){
    case hdrl_spectrum1D_interp_linear:
        return gsl_spline_alloc (gsl_interp_linear, sample_len);
    case hdrl_spectrum1D_interp_cspline:
        return gsl_spline_alloc (gsl_interp_cspline, sample_len);
    case hdrl_spectrum1D_interp_akima:
        return gsl_spline_alloc (gsl_interp_akima, sample_len);
    default:
        cpl_ensure(0, CPL_ERROR_ILLEGAL_INPUT, NULL);
    }
    /*To avoid warning*/
    return NULL;
}

/* cleanup for the gsl interpolation structs*/
static inline void
cleanup_gsl_interpolate(gsl_interp_accel ** acc_flux, gsl_spline ** spline_flux){
    if(acc_flux != NULL && *acc_flux != NULL){
        gsl_interp_accel_free (*acc_flux);
	*acc_flux = NULL;
    }

    if(spline_flux != NULL && *spline_flux != NULL){
        gsl_spline_free (*spline_flux);
	*spline_flux = NULL;
    }
}
/*Init of the gls interpolation data structures.
 * If fails, the memory is cleaned-up */
static inline cpl_error_code
init_gsl_interpolate(
        const double *x, const double *y, const cpl_size sample_len,
        const hdrl_spectrum1D_interpolation_method method,
        gsl_interp_accel ** acc_flux, gsl_spline ** spline_flux){

    *acc_flux = gsl_interp_accel_alloc ();

    cpl_ensure_code(*acc_flux, CPL_ERROR_UNSPECIFIED);

    *spline_flux = get_interp_spline(method, sample_len);

    if(!*spline_flux){
        cleanup_gsl_interpolate(acc_flux, spline_flux);
        cpl_ensure_code(0, CPL_ERROR_UNSPECIFIED);
    }

    int fail = gsl_spline_init(*spline_flux, x, y, sample_len);

    if(fail){
        cleanup_gsl_interpolate(acc_flux, spline_flux);
        cpl_ensure_code(0, CPL_ERROR_UNSPECIFIED);
    }

    return CPL_ERROR_NONE;
}

/*Wrapper around gsl_spline_eval. If x is outside the interval provided during
 * init phase we return NAN. GSL would abort the program.*/
static inline
double spline_eval_internal
(const gsl_spline * spline, double x, gsl_interp_accel * a, cpl_boolean * is_rej){

    *is_rej = x < spline->x[0] || x > spline->x[spline->size - 1];

    if (*is_rej){
        return NAN;
    }

    return gsl_spline_eval(spline, x, a);
}

static inline cpl_size
get_closest_lambda(const double *arr, const cpl_size num_els, const double l){

    cpl_size best_idx = 0;
    double smallest_diff = fabs(arr[0] - l);

    for(cpl_size i = 1; i < num_els; i++){
        const double diff = fabs(arr[i] - l);

        if(diff < smallest_diff){
            smallest_diff = diff;
            best_idx = i;
        }

        /*we assume arr sorted*/
        if(arr[i] >= l) break;
    }
    return best_idx;
}

static inline cpl_error_code
fill_cpl_image_with_interpolation(const double *x, const double *y,
        const cpl_size sample_len, const hdrl_spectrum1D_interpolation_method method,
        const cpl_array * lambdas_dest, cpl_image * dest){

    cpl_size dest_len = cpl_array_get_size(lambdas_dest);

    gsl_interp_accel * acc_flux = NULL;
    gsl_spline * spline_flux = NULL;

    cpl_error_code fail =
            init_gsl_interpolate(x, y, sample_len, method, &acc_flux, &spline_flux);

    cpl_ensure_code(fail == CPL_ERROR_NONE, fail);

    for(cpl_size i = 0; i < dest_len; i++){
        const double lambda = cpl_array_get(lambdas_dest, i, NULL);
        cpl_boolean is_rejected = CPL_FALSE;
        const double val = spline_eval_internal(spline_flux, lambda,
                acc_flux, &is_rejected);
        if(is_rejected){
            cpl_image_reject(dest, i + 1, 1);
        }
        else{
            cpl_image_set(dest, i + 1, 1, val);
        }
    }
    cleanup_gsl_interpolate(&acc_flux, &spline_flux);

    return CPL_ERROR_NONE;
}

static inline double
get_min(const double * x, cpl_size length){
	double best = x[0];
	for(cpl_size i = 1; i < length; ++i){
		if(x[i] < best) best = x[i];
	}
	return best;
}

static inline double
get_max(const double * x, cpl_size length){
	double best = x[0];
	for(cpl_size i = 1; i < length; ++i){
		if(x[i] > best) best = x[i];
	}
	return best;
}

static inline
gsl_bspline_workspace * alloc_workspace(int k, int nCoeff,
        const double * x_dest, const cpl_size sample_len){

    const int nKnots = nCoeff + 2 - k;

    gsl_bspline_workspace  * ws = gsl_bspline_alloc(k, nKnots);

    double lambda_min = get_min(x_dest, sample_len);
    double lambda_max = get_max(x_dest, sample_len);

    gsl_bspline_knots_uniform(lambda_min, lambda_max, ws);
    return ws;
}

static inline cpl_error_code
hdrl_spectrum1D_fit_windowed_internal
(const double* x_raw, const double* y_raw, const cpl_size sample_len,
    const cpl_array * lambdas_dest, cpl_image * flux_dest, const int k,
    const int nCoeff, const long window, const double factor){

    const cpl_size dest_size = cpl_array_get_size(lambdas_dest);

    const cpl_size fit_win = (cpl_size)((double)window * factor);
    const cpl_size extra_samples_for_fit = (fit_win - window) / 2;

    for(cpl_size dest_start = 0; dest_start < dest_size; dest_start += window){

        const cpl_size dest_stop = CPL_MIN(dest_size - 1, dest_start + window - 1);

        const double min_dest_lambda =
                cpl_array_get(lambdas_dest, dest_start, NULL);
        const double max_dest_lambda =
                cpl_array_get(lambdas_dest, dest_stop, NULL);

        /*get corresponding indexes in x_raw (we need to be sure that
         * min_dest_lambda and max_dest_lambda are inside the interval,
         * hence the +1 and -1)*/
        cpl_size raw_start =
                get_closest_lambda(x_raw, sample_len, min_dest_lambda) - 1;
        cpl_size raw_end =
                get_closest_lambda(x_raw, sample_len, max_dest_lambda) + 1;

        /*expand the fit window*/
        raw_end += extra_samples_for_fit;
        raw_start -= extra_samples_for_fit;

        raw_end = CPL_MIN(sample_len - 1, raw_end);
        raw_start = CPL_MAX(0, raw_start);

        const cpl_size win_len = raw_end - raw_start + 1;

        cpl_error_code fail = hdrl_spectrum1D_bspline_fit_internal(
        x_raw + raw_start, y_raw + raw_start, win_len,
        lambdas_dest, dest_start, dest_stop, flux_dest, k, nCoeff);

        if(fail) return fail;
    }

   return  CPL_ERROR_NONE;
}

static inline cpl_error_code
hdrl_spectrum1D_bspline_fit_internal
(const double* x_raw, const double* y_raw,
const cpl_size sample_len, const cpl_array * lambdas_dest,
const cpl_size lambdas_dest_start, const cpl_size lambdas_dest_stop,
cpl_image * flux_dest, const int k, const int nCoeff){

    cpl_ensure_code(sample_len >= nCoeff, CPL_ERROR_INCOMPATIBLE_INPUT);

    gsl_vector* B = gsl_vector_alloc(nCoeff);
    gsl_vector* c = gsl_vector_alloc(nCoeff);
    gsl_matrix * cov = gsl_matrix_alloc(nCoeff, nCoeff);
    gsl_bspline_workspace * ws = alloc_workspace(k, nCoeff, x_raw, sample_len);

    int fail = fit_matrixes(x_raw, y_raw, sample_len, nCoeff, ws, B, c, cov);

    cpl_error_code error = !fail ? CPL_ERROR_NONE : CPL_ERROR_UNSPECIFIED;
    const double x_raw_min = x_raw[0];
    const double x_raw_max = x_raw[sample_len - 1];

    if(!error)
    {
        cpl_size dest_len = cpl_array_get_size(lambdas_dest);

           for (cpl_size i = CPL_MAX(0, lambdas_dest_start);
                   i <= CPL_MIN(dest_len - 1, lambdas_dest_stop); i++)
           {
               const double x_dest = cpl_array_get(lambdas_dest, i, NULL);

               /*If outside boundaries reject*/
               if(x_dest < x_raw_min || x_dest > x_raw_max){
                   cpl_image_reject(flux_dest, i + 1, 1);
                   continue;
               }

               gsl_bspline_eval(x_dest, B, ws);
               double y_dest = 0.0, irrelevant = 0.0;
               gsl_multifit_linear_est(B, c, cov, &y_dest, &irrelevant);

               cpl_image_set(flux_dest, i + 1, 1, y_dest);
           }
    }

    gsl_matrix_free(cov);
    gsl_vector_free(B);
    gsl_vector_free(c);
    gsl_bspline_free(ws);

    return error;
}

static inline
cpl_size count_equals_from_i
(const double * x, const cpl_size i, const cpl_size sample_len){

    cpl_size idx = i;
    while( (idx < sample_len - 1) && x[idx] == x[idx + 1])
                idx++;

    return idx - i + 1;
}

static inline int compare_d(const void * a, const void * b){
    const double * a_d = (const double*)a;
    const double * b_d = (const double*)b;

    const double delta = *a_d - *b_d;
    if(delta > 0.0) return 1;
    if(delta < 0.0) return -1;
    return 0;
}

static inline
double get_median(double * x, const cpl_size first, const cpl_size n){

    double * v = x + first;
    qsort(v, n, sizeof(x[0]), compare_d);

    if(n % 2 != 0) return v[n / 2];

    return (v[n / 2] + v[(n - 1)/2]) / 2.0;
}

cpl_size
hdrl_spectrum1D_resample_filter_dups_and_substitute_with_median(double * x,
        double * y1, double * y2, cpl_size sample_len){

    for(cpl_size i = 0; i < sample_len - 1; i++){

        const cpl_size n_equals = count_equals_from_i(x, i, sample_len);

        if(n_equals <= 1) continue;

        y1[i] = get_median(y1, i, n_equals);
        y2[i] = get_median(y2, i, n_equals);

        const cpl_size num_els_to_copy = sample_len - (i + n_equals);
        const cpl_size num_bytes = sizeof(double) * num_els_to_copy;

        if(num_bytes > 0){
            /* using memmove because memory overlaps*/
            memmove( x + i + 1,  x + i + n_equals, (size_t)num_bytes);
            memmove(y1 + i + 1, y1 + i + n_equals, (size_t)num_bytes);
            memmove(y2 + i + 1, y2 + i + n_equals, (size_t)num_bytes);
        }
        /* one of the n_equals elements is survived (it is median now), hence the
         * -1*/
        sample_len -= n_equals - 1;
    }

    return sample_len;
}


static inline int
fit_matrixes(const double * x_raw, const double * y_raw,
        const cpl_size sample_len, const int nCoeff,
        gsl_bspline_workspace * ws, gsl_vector* B,
        gsl_vector* c, gsl_matrix * cov){

    gsl_matrix* X = gsl_matrix_alloc(sample_len, nCoeff);

    for(cpl_size i = 0; i < sample_len; ++i){
        int fail = gsl_bspline_eval(x_raw[i], B, ws);
        if(fail) continue;
        for(cpl_size j = 0; j < nCoeff; j++){
            gsl_matrix_set(X, i, j, gsl_vector_get(B, j));
        }
    }

    double chisq = 0.0;
    gsl_vector_const_view y = gsl_vector_const_view_array(y_raw, sample_len);
    gsl_multifit_linear_workspace* mw =
            gsl_multifit_linear_alloc(sample_len, nCoeff);

    const int error = gsl_multifit_linear(X, &y.vector, c, cov, &chisq, mw);

    gsl_multifit_linear_free(mw);
    gsl_matrix_free(X);

    return error;
}

static inline cpl_error_code
resample_with_interpol_on_variance(const hdrl_parameter * par,
        const cpl_boolean interpolate, const double * x, const double * y,
        const double * y_var, const cpl_size sample_len,
        const cpl_array * lambdas_dest, hdrl_image * flux_dest){

    cpl_error_code fail = CPL_ERROR_NONE;

    if(interpolate){
        hdrl_spectrum1D_interpolation_method int_met =
                hdrl_spectrum1D_resample_interpolate_parameter_get_method(par);

        fail = fill_cpl_image_with_interpolation(x, y, sample_len, int_met,
                        lambdas_dest, hdrl_image_get_image(flux_dest));
    } else {

        int k = hdrl_spectrum1D_resample_fit_parameter_get_k(par);
        int nCoeff = hdrl_spectrum1D_resample_fit_parameter_get_nCoeff(par);
        long window1 = hdrl_spectrum1D_resample_fit_parameter_get_window(par);
        double factor= hdrl_spectrum1D_resample_fit_parameter_get_factor(par);

        if(window1 == 0){
            fail = hdrl_spectrum1D_bspline_fit_internal(x, y,
                    sample_len, lambdas_dest, 0,
                    cpl_array_get_size(lambdas_dest) - 1,
                    hdrl_image_get_image(flux_dest), k, nCoeff);
        } else{
            fail = hdrl_spectrum1D_fit_windowed_internal(x, y,
                            sample_len, lambdas_dest,
                            hdrl_image_get_image(flux_dest), k,
                            nCoeff, window1, factor);
        }
    }

    if(!fail){
        /* linear interpolate variance */
        fill_cpl_image_with_interpolation(x, y_var, sample_len,
                hdrl_spectrum1D_interp_linear,
                lambdas_dest, hdrl_image_get_error(flux_dest));
        /* convert variance into error taking the square root*/
        cpl_image_power(hdrl_image_get_error(flux_dest), 0.5);
    }

    return fail;
}

static inline double
get_start(const double * v, const cpl_size i){

    if(i <= 0){
        return v[0];
    }

    const double post = v[i];
    const double pre = v[i - 1];

    return (post + pre) / 2.0;
}

static inline double
get_stop(const double * v, const cpl_size length, const cpl_size i){

    if(i >= length - 1){
        return v[length - 1];
    }

    const double post = v[i + 1];
    const double pre = v[i];

    return (post + pre) / 2.0;
}


static inline cpl_boolean
is_destination_outside_source_spectrum(const double * source,
        const cpl_size source_length, const double dest_start,
        const double dest_stop){
    /*edge case, source cover a smaller interval than dest, hence starting and/or
     * ending bins in destination might be NAN*/
    const double source_lower_bound = get_start(source, 0);
    const double source_upper_bound = get_stop(source, source_length, source_length - 1);

    /* destination starts before the start of the first bin*/
    if(dest_start < source_lower_bound) return CPL_TRUE;
    /* destination ends after the end of the last bin*/
    if(dest_stop > source_upper_bound) return CPL_TRUE;

    return CPL_FALSE;
}


static inline double
integrate(const double start_dest, const double stop_dest,
        cpl_size * source_idx, const double * x, const double * y,
        const cpl_size sample_len){

    double start_source = NAN;
    double stop_source = NAN;
    double val = 0.0;

    if(is_destination_outside_source_spectrum(x, sample_len, start_dest, stop_dest))
        return NAN;

    /*area of destination bin*/
    const double den = stop_dest - start_dest;
    *source_idx = CPL_MIN(*source_idx, sample_len - 1);

    do{
        /*start and stop of source bin*/
        start_source = get_start(x, *source_idx);
        stop_source = get_stop(x, sample_len, *source_idx);

        /* There is no intersection, source starts after the end of dest.
         * We are done with this bin, go back one step so that the next
         * destination can use the source bin. */
        if(start_source >= stop_dest) {
            *source_idx = CPL_MAX(*source_idx - 1, 0);
            break;
        }

        /* If stop_source <= start_dest, there is no intersection,
         * source ends before the start of dest. We can skip this element of source
         * increase the index so that the next element can be used. We are sure
         * that there will be an acceptable element because we already checked boundaries
         * before starting the loop.*/
        if(stop_source > start_dest){
            /*Get common area*/
            const double common_slice_start = CPL_MAX(start_source, start_dest);
            const double common_slice_stop = CPL_MIN(stop_source, stop_dest);
            const double num = common_slice_stop - common_slice_start;

            val += y[*source_idx] * num / den;
        }

        *source_idx = *source_idx + 1;

    }while(*source_idx < sample_len);

    return val;
}

static inline cpl_error_code
integrate_internal(double * x, const double * y, const double * y_var,
        const cpl_size sample_len, const cpl_array * arg_lambdas_dest,
        hdrl_image * flux_dest){

    const cpl_size size_dest = cpl_array_get_size(arg_lambdas_dest);
    cpl_bivector * lamb_dest = cpl_bivector_new(size_dest);

    for(cpl_size i = 0; i < size_dest ; ++i){
        const double val = cpl_array_get(arg_lambdas_dest, i, NULL);
        cpl_vector_set(cpl_bivector_get_x(lamb_dest), i, val);
        cpl_vector_set(cpl_bivector_get_y(lamb_dest), i, i);
    }
    /*Sort dest lambdas keeping track of the original position*/
    cpl_bivector_sort(lamb_dest, lamb_dest, CPL_SORT_ASCENDING, CPL_SORT_BY_X);
    cpl_size source_idx = 0;
    const double * lambdas_dest_sorted =
            cpl_vector_get_data_const(cpl_bivector_get_x(lamb_dest));

    for(cpl_size i = 0; i < size_dest; ++i){

        const double start_destination = get_start(lambdas_dest_sorted, i);
        const double stop_destination = get_stop(lambdas_dest_sorted, size_dest , i);

        /*copy index before increase for error computation*/
        cpl_size old_idx = source_idx;
        const double val = integrate(start_destination, stop_destination,
                &source_idx, x, y, sample_len);

        const double val_e = sqrt(integrate(start_destination, stop_destination,
                        &old_idx, x, y_var, sample_len));

        const cpl_size dest_idx =
                        (cpl_size) cpl_vector_get(cpl_bivector_get_y(lamb_dest), i);

        if(!isfinite(val) || !isfinite(val_e)) {
            hdrl_image_reject(flux_dest, dest_idx + 1, 1);
            continue;
        }

        hdrl_image_set_pixel(flux_dest, dest_idx + 1, 1, (hdrl_value){val, val_e});
    }

    cpl_bivector_delete(lamb_dest);

    return CPL_ERROR_NONE;
}

static inline hdrl_spectrum1D *
resample_internal(const hdrl_spectrum1D * self,
        const cpl_array * lambdas_dest, const hdrl_parameter * par){

    const cpl_size f_len = hdrl_spectrum1D_get_size(self);
    cpl_size sample_len = 0;

    double * y = cpl_calloc(f_len, sizeof(double));
    double * y_var = cpl_calloc(f_len, sizeof(double));
    double * x = cpl_calloc(f_len, sizeof(double));

    const cpl_boolean reject_bad_pix =
            hdrl_parameter_get_parameter_enum(par) != HDRL_PARAMETER_SPECTRUM1D_RESAMPLE_INTEGRATE;

    for(cpl_size i = 0; i < f_len; i++){

        int rej = 0;
        const hdrl_value v =  hdrl_spectrum1D_get_flux_value(self, i, &rej);
        rej = rej || !isfinite((double)v.data) || !isfinite((double)v.error);

        if(reject_bad_pix && rej) continue;

        y[sample_len] = rej ? NAN : v.data;
        /*We interpolate the VARIANCE*/
        y_var[sample_len] = rej ? NAN : pow(v.error, 2.0);
        x[sample_len] = hdrl_spectrum1D_get_wavelength_value(self, i, NULL);
        sample_len++;
    }

    if(sample_len == 0){
        cpl_free(x);
        cpl_free(y);
        cpl_free(y_var);
        cpl_ensure(CPL_FALSE, CPL_ERROR_INCOMPATIBLE_INPUT, NULL);
    }

    /* If wavelengths are not strictly increasing, sort and collapse in case of
     * duplicate wavelengths. */
    if(!hdrl_is_strictly_monotonic_increasing(x, sample_len)){
        /* we need to sort the three arrays so x is strictly increasing*/
        hdrl_sort_on_x(x, y, y_var, sample_len, CPL_FALSE);
        /* remove duplicated lambdas and substitute flux with median*/
        sample_len =
                hdrl_spectrum1D_resample_filter_dups_and_substitute_with_median
                (x, y, y_var, sample_len);
    }

    if(sample_len == 0){
        cpl_free(x);
        cpl_free(y);
        cpl_free(y_var);
        cpl_ensure(CPL_FALSE, CPL_ERROR_INCOMPATIBLE_INPUT, NULL);
    }

    cpl_size dest_len = cpl_array_get_size(lambdas_dest);
    hdrl_image * flux_dest = hdrl_image_new(dest_len, 1);

    cpl_error_code fail = CPL_ERROR_NONE;

    hdrl_parameter_enum method = hdrl_parameter_get_parameter_enum(par);

    if(method != HDRL_PARAMETER_SPECTRUM1D_RESAMPLE_INTEGRATE){
        const cpl_boolean is_interpolation =
                method == HDRL_PARAMETER_SPECTRUM1D_RESAMPLE_INTERPOLATE;
        fail = resample_with_interpol_on_variance(par, is_interpolation, x, y,
                y_var, sample_len, lambdas_dest, flux_dest);
    }
    else{
        fail = integrate_internal(x, y, y_var, sample_len, lambdas_dest,
                flux_dest);
    }
    cpl_free(x);
    cpl_free(y);
    cpl_free(y_var);

    hdrl_spectrum1D * to_ret = NULL;

    if(fail == CPL_ERROR_NONE)
    {
        const cpl_image * img = hdrl_image_get_image_const(flux_dest);
        const cpl_image * img_e = hdrl_image_get_error_const(flux_dest);
        to_ret = hdrl_spectrum1D_create(img, img_e, lambdas_dest,
                self->wave_scale);
    }
    hdrl_image_delete(flux_dest);

    cpl_ensure(fail == CPL_ERROR_NONE, fail, NULL);

    return to_ret;
}
/**@}*/
