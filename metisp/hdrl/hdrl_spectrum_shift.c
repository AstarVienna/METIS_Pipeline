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
#include "hdrl_spectrum_shift.h"
#include "hdrl_parameter.h"
#include "hdrl_spectrum_resample.h"

#include <math.h>

/**
 *
 * @addtogroup hdrl_spectrum1D
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/
/*-----------------------------------------------------------------------------
                                   Private Functions
 -----------------------------------------------------------------------------*/

static inline hdrl_data_t
hdrl_spectrum1D_compute_min_fit(const hdrl_spectrum1D * s,
        						const hdrl_parameter * shift_par);

static inline cpl_bivector * get_win(hdrl_data_t wmin, hdrl_data_t wmax);


static inline cpl_array *
convert_to_sorted_array(const hdrl_spectrum1D * s1);

static inline hdrl_data_t
hdrl_shift_fit_parameter_get_fit_half_win(const hdrl_parameter * par);
static inline hdrl_data_t
hdrl_shift_fit_parameter_get_fit_wmax(const hdrl_parameter * par);
static inline hdrl_data_t
hdrl_shift_fit_parameter_get_fit_wmin(const hdrl_parameter * par);
static inline hdrl_data_t
hdrl_shift_fit_parameter_get_range_wmin(const hdrl_parameter * par);
static inline hdrl_data_t
hdrl_shift_fit_parameter_get_range_wmax(const hdrl_parameter * par);
static inline hdrl_data_t
hdrl_shift_fit_parameter_get_wguess(const hdrl_parameter * par);

static inline hdrl_spectrum1D *
get_polyfit_for_slope(const int degree, const hdrl_spectrum1D * s,
		const cpl_array * wlengths);

static inline hdrl_spectrum1D *
hdrl_spectrum1D_fit(const hdrl_spectrum1D * obs, const int degree,
		const hdrl_data_t wmin, const hdrl_data_t wmax);
/*-----------------------------------------------------------------------------
                                   Functions
 -----------------------------------------------------------------------------*/
/**
 * @brief The function computes the shift between the two spectra.
 *
 * @param s1        first spectrum
 * @param s2        second spectrum
 * @param half_win  half window where the correlation will be calculated
 * @param normalize normalize mean and stdev in cross-correlation calculation
 * @return estimation of the shift after gaussian fit.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: s1 or s2 is NULL;
 * - CPL_ERROR_INCOMPATIBLE_INPUT: if wavelengths are not uniformly sampled,
 * or if the two spectra are not compatible;
 */
/* ---------------------------------------------------------------------------*/
hdrl_xcorrelation_result *
hdrl_spectrum1D_compute_shift_xcorrelation(const hdrl_spectrum1D * s1,
                              	  	  	   const hdrl_spectrum1D * s2,
                                           cpl_size half_win,
                                           const cpl_boolean normalize){


    cpl_ensure(s1 != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(s2 != NULL, CPL_ERROR_NULL_INPUT, NULL);

    {
        const hdrl_spectrum1D_wavelength wav1 = hdrl_spectrum1D_get_wavelength(s1);
        const hdrl_spectrum1D_wavelength wav2 = hdrl_spectrum1D_get_wavelength(s2);

        cpl_ensure(hdrl_spectrum1D_are_spectra_compatible(&wav1, &wav2),
                CPL_ERROR_INCOMPATIBLE_INPUT, NULL);
    }

    double bin = 0.0;
    cpl_ensure(hdrl_spectrum1D_is_uniformly_sampled(s1, &bin),
            CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    cpl_array * f1 = convert_to_sorted_array(s1);
    cpl_array * f2 = convert_to_sorted_array(s2);

    hdrl_xcorrelation_result * to_ret = hdrl_compute_offset_gaussian(f2, f1,
            half_win, normalize, bin, 0.0005);

    cpl_array_delete(f1);
    cpl_array_delete(f2);

    return to_ret;
}

/*Parameter used for the shift slope calculation*/
typedef struct {
    HDRL_PARAMETER_HEAD;
	hdrl_data_t wguess;
	hdrl_data_t range_wmin;
	hdrl_data_t range_wmax;
	hdrl_data_t fit_wmin;
	hdrl_data_t fit_wmax;
	hdrl_data_t fit_half_win;
} hdrl_spectrum1D_shift_parameter;

static hdrl_parameter_typeobj
hdrl_shift_fit_parameters_type = {
		HDRL_PARAMETER_SPECTRUM1D_SHIFT,      /* type */
    (hdrl_alloc *)&cpl_malloc,                /* fp_alloc */
    (hdrl_free *)&cpl_free,                   /* fp_free */
    NULL,                                     /* fp_destroy */
    sizeof(hdrl_spectrum1D_shift_parameter),  /* obj_size */
};

/* ---------------------------------------------------------------------------*/
/**
 * @brief The function create a hdrl_spectrum1D_shift_parameter to be used in
 * hdrl_spectrum1D_compute_shift_fit.
 *
 * @param wguess        Reference line wavelength position
 * @param range_wmin    Minimum of wavelength box for line fit
 * @param range_wmax    Maximum of wavelength box for line fit
 * @param fit_wmin 		Minimum wavelength value used to fit line slope
 * @param fit_wmax      Maximum wavelength value used to fit line slope
 * @param fit_half_win  Half box where polynomial fit is performed
 * @return the constructed hdrl_parameter.
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter *
hdrl_spectrum1D_shift_fit_parameter_create(const hdrl_data_t wguess,
		const hdrl_data_t range_wmin, const hdrl_data_t range_wmax,
		const hdrl_data_t fit_wmin, const hdrl_data_t fit_wmax,
		const hdrl_data_t fit_half_win){

	hdrl_spectrum1D_shift_parameter * p
    = (hdrl_spectrum1D_shift_parameter *)
       hdrl_parameter_new(&hdrl_shift_fit_parameters_type);

	p->fit_half_win = fit_half_win;
	p->fit_wmax = fit_wmax;
	p->fit_wmin = fit_wmin;
	p->range_wmax = range_wmax;
	p->range_wmin = range_wmin;
	p->wguess = wguess;

	return (hdrl_parameter*) p;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief The function compute the shift due to radial velocity. If wguess is
 * the reference line and wfound is its position in the obs spectrum, the function
 * returns (wfound - wguess) / wguess. The algorithm generate a smoothed fit of the
 * spectrum, between [range_wmin, range_wmax] but the wavelengths between [fit_wim,
 * fit_wmax] are ignored when fitting. obs is then divided by the fitted spectrum.
 * The ratio is then smoothed again via fitting inside the window [wguess - fit_half_win,
 * wguess + fit_half_win]. The wavelength corresponding to the minimum value is wfound.
 *
 * @param obs        The spectrum the shift has to be computed on
 * @param par		 The shift parameter, see hdrl_spectrum1D_shift_fit_parameter_create
 *
 * @return (wfound - wguess) / wguess.
 */
/* ---------------------------------------------------------------------------*/
hdrl_data_t
hdrl_spectrum1D_compute_shift_fit(const hdrl_spectrum1D * obs,
                              const hdrl_parameter * par){

	cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, 0.0);

	cpl_ensure(hdrl_parameter_get_parameter_enum(par)
			== HDRL_PARAMETER_SPECTRUM1D_SHIFT, CPL_ERROR_ILLEGAL_INPUT, 0.0);

	cpl_ensure(hdrl_shift_fit_parameter_get_range_wmin(par)
			< hdrl_shift_fit_parameter_get_range_wmax(par), CPL_ERROR_ILLEGAL_INPUT, 0.0);

	cpl_ensure(hdrl_shift_fit_parameter_get_fit_wmin(par)
			< hdrl_shift_fit_parameter_get_fit_wmax(par), CPL_ERROR_ILLEGAL_INPUT, 0.0);

	cpl_ensure(hdrl_shift_fit_parameter_get_range_wmin(par)
			< hdrl_shift_fit_parameter_get_fit_wmin(par), CPL_ERROR_ILLEGAL_INPUT, 0.0);

	cpl_ensure(hdrl_shift_fit_parameter_get_range_wmax(par)
			> hdrl_shift_fit_parameter_get_fit_wmax(par), CPL_ERROR_ILLEGAL_INPUT, 0.0);

	cpl_bivector * win = get_win(hdrl_shift_fit_parameter_get_range_wmin(par),
			hdrl_shift_fit_parameter_get_range_wmax(par));

	hdrl_spectrum1D * obs_sel = hdrl_spectrum1D_select_wavelengths(obs, win, CPL_TRUE);

	{
		const int degree = 4;

		hdrl_spectrum1D * obs_fitted = hdrl_spectrum1D_fit(obs_sel, degree,
				hdrl_shift_fit_parameter_get_fit_wmin(par),
				hdrl_shift_fit_parameter_get_fit_wmax(par));

		cpl_ensure(obs_fitted != NULL, CPL_ERROR_ILLEGAL_OUTPUT, 0.0);

		hdrl_spectrum1D_div_spectrum(obs_sel, obs_fitted);

		hdrl_spectrum1D_add_scalar(obs_sel, (hdrl_value){2.0, 0.0});

		hdrl_spectrum1D_delete(&obs_fitted);
	}

	hdrl_data_t min_wlen = hdrl_spectrum1D_compute_min_fit(obs_sel, par);

	hdrl_spectrum1D_delete(&obs_sel);
	cpl_bivector_delete(win);
	const hdrl_data_t wguess = hdrl_shift_fit_parameter_get_wguess(par);
	const hdrl_data_t offset = (min_wlen - wguess)/wguess;
	return offset;
}


/*-----------------------------------------------------------------------------
                    Private Functions Implementation
 -----------------------------------------------------------------------------*/

cpl_size convert_to_matrix_and_vector(cpl_matrix ** x, cpl_vector ** values,
		const hdrl_spectrum1D * s){

    *x = NULL;
    *values = NULL;

	const cpl_size sz = hdrl_spectrum1D_get_size(s);
	double * x_vals = cpl_calloc(sz, sizeof(double));
	double * p_values = cpl_calloc(sz, sizeof(double));

	cpl_size real_sz = 0;
	for(cpl_size i = 0; i < sz; ++i){
		int rej = 0;
		hdrl_data_t f = hdrl_spectrum1D_get_flux_value(s, i, &rej).data;
		if(rej) continue;
		hdrl_data_t w = hdrl_spectrum1D_get_wavelength_value(s, i, &rej);
		x_vals[real_sz] = w;
		p_values[real_sz] = f;
		real_sz++;
	}

	if(real_sz == 0){
		*values = NULL;
		*x = NULL;
		cpl_free(x_vals);
		cpl_free(p_values);
		return real_sz;
	}

	*values = cpl_vector_wrap(real_sz, p_values);
	*x = cpl_matrix_wrap(1, real_sz, x_vals);
	return real_sz;
}

static inline cpl_polynomial *
polynomial_fit_1d_create(
                   const hdrl_spectrum1D * s,
                   int                     degree)
{
    cpl_polynomial * fit1d = cpl_polynomial_new(1);
    double rechisq = 0;

    cpl_size loc_deg=(cpl_size)degree;
    cpl_matrix * samppos = NULL;
    cpl_vector * values = NULL;

    const cpl_size x_size = convert_to_matrix_and_vector(&samppos, &values, s);

    cpl_ensure(x_size > 0, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

    cpl_vector * fitresidual = cpl_vector_new(x_size);

    cpl_polynomial_fit(fit1d, samppos, NULL, values, NULL,
                       CPL_FALSE, NULL, &loc_deg);
    cpl_ensure(!cpl_error_get_code(), cpl_error_get_code(), NULL);

    if ( x_size > (degree + 1) )  {
      cpl_vector_fill_polynomial_fit_residual(fitresidual, values, NULL, fit1d,
                          samppos, &rechisq);
      cpl_ensure(!cpl_error_get_code(), cpl_error_get_code(), NULL);
    }

    cpl_matrix_delete(samppos);
    cpl_vector_delete(fitresidual);
    cpl_vector_delete(values);
    return fit1d;
}


static inline hdrl_spectrum1D *
get_polyfit_for_slope(const int degree, const hdrl_spectrum1D * s,
		const cpl_array * wlengths){

	const hdrl_spectrum1D_wave_scale scale = hdrl_spectrum1D_get_scale(s);

	cpl_polynomial* pfit = polynomial_fit_1d_create(s, degree);
	cpl_ensure(pfit != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

	const cpl_size sz = cpl_array_get_size(wlengths);
	cpl_image * new_flux = cpl_image_new(sz, 1, HDRL_TYPE_DATA);
	for(cpl_size i = 0; i < sz; ++i){
		double v = cpl_polynomial_eval_1d( pfit, cpl_array_get(wlengths, i, NULL), NULL);
		cpl_image_set(new_flux, i + 1, 1, v);
	}

	hdrl_spectrum1D * to_ret = hdrl_spectrum1D_create_error_free(new_flux, wlengths, scale);

	cpl_polynomial_delete(pfit);
	cpl_image_delete(new_flux);
	return to_ret;
}

/* Fits obs with a polynomial of degree "degree". The wavelengths between wmin and wmax
 * are ignored when building the fit. */
static inline hdrl_spectrum1D *
hdrl_spectrum1D_fit(
		const hdrl_spectrum1D * obs, const int degree, const hdrl_data_t wmin,
		const hdrl_data_t wmax){

	cpl_bivector * win = get_win(wmin, wmax);

	hdrl_spectrum1D * obs_sel = hdrl_spectrum1D_select_wavelengths(obs, win, CPL_FALSE);

	cpl_ensure(obs_sel != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

	hdrl_spectrum1D * obs_fitted = get_polyfit_for_slope(degree, obs_sel,
			hdrl_spectrum1D_get_wavelength(obs).wavelength);

	cpl_ensure(obs_fitted != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

	cpl_bivector_delete(win);
	hdrl_spectrum1D_delete(&obs_sel);
	return obs_fitted;
}

static inline hdrl_data_t
hdrl_spectrum1D_compute_min_fit(const hdrl_spectrum1D * s,
        						const hdrl_parameter * shift_par){

	const hdrl_data_t wguess = hdrl_shift_fit_parameter_get_wguess(shift_par);
	const hdrl_data_t fit_half_win =
			hdrl_shift_fit_parameter_get_fit_half_win(shift_par);
	cpl_bivector * win = get_win(wguess - fit_half_win, wguess + fit_half_win);

	hdrl_spectrum1D * s_core =
			hdrl_spectrum1D_select_wavelengths(s, win, CPL_TRUE);

	hdrl_spectrum1D_wavelength waves = hdrl_spectrum1D_get_wavelength(s_core);

	hdrl_spectrum1D * s_resampled =
			get_polyfit_for_slope(4, s_core, waves.wavelength);

	cpl_bivector_delete(win);
	hdrl_spectrum1D_delete(&s_core);

	const hdrl_image * flux = hdrl_spectrum1D_get_flux(s_resampled);
	cpl_size x = 0, y = 0;
	cpl_image_get_minpos(hdrl_image_get_image_const(flux), &x, &y);

	const hdrl_data_t real_min =
			hdrl_spectrum1D_get_wavelength_value(s_resampled, x - 1, NULL);

	hdrl_spectrum1D_delete(&s_resampled);
	return real_min;
}

static inline cpl_bivector * get_win(hdrl_data_t wmin, hdrl_data_t wmax){
	cpl_bivector * v = cpl_bivector_new(1);

	cpl_vector_set(cpl_bivector_get_x(v), 0, wmin);
	cpl_vector_set(cpl_bivector_get_y(v), 0, wmax);

	return v;
}

/* Converts the flux spectrum to an array of double. Rejected pixels are mapped
 * to NAN. */
static inline cpl_array *
convert_to_sorted_array(const hdrl_spectrum1D * s1){

    const cpl_size sz = hdrl_spectrum1D_get_size(s1);
    double * flx = cpl_calloc(sz, sizeof(double));
    double * wav = cpl_calloc(sz, sizeof(double));
    double * is_rej = cpl_calloc(sz, sizeof(double));
    for(cpl_size i = 0; i < sz; ++i){
        int rej = 0;
        flx[i] = hdrl_spectrum1D_get_flux_value(s1, i, &rej).data;
        is_rej[i] = rej;
        wav[i] = hdrl_spectrum1D_get_wavelength_value(s1, i, NULL);
    }

    hdrl_sort_on_x(wav, flx, is_rej, sz, CPL_FALSE);

    cpl_free(wav);

    cpl_array * to_ret = cpl_array_wrap_double(flx, sz);

    for(cpl_size i = 0; i < sz; ++i){
        if(fabs(is_rej[i]) < 1e-4) continue;

        cpl_array_set_invalid(to_ret, i);
    }

    cpl_free(is_rej);

    return to_ret;
}

static inline hdrl_data_t
hdrl_shift_fit_parameter_get_fit_half_win(const hdrl_parameter * par){
	cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, 0.0);
	cpl_ensure(hdrl_parameter_get_parameter_enum(par)
				== HDRL_PARAMETER_SPECTRUM1D_SHIFT, CPL_ERROR_ILLEGAL_INPUT, 0.0);

	const hdrl_spectrum1D_shift_parameter * p =
			(const hdrl_spectrum1D_shift_parameter *)par;
	return p->fit_half_win;
}

static inline hdrl_data_t
hdrl_shift_fit_parameter_get_fit_wmax(const hdrl_parameter * par){

	cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, 0.0);
	cpl_ensure(hdrl_parameter_get_parameter_enum(par)
				== HDRL_PARAMETER_SPECTRUM1D_SHIFT, CPL_ERROR_ILLEGAL_INPUT, 0.0);

	const hdrl_spectrum1D_shift_parameter * p =
			(const hdrl_spectrum1D_shift_parameter *)par;
	return p->fit_wmax;
}

static inline hdrl_data_t
hdrl_shift_fit_parameter_get_fit_wmin(const hdrl_parameter * par){

	cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, 0.0);
	cpl_ensure(hdrl_parameter_get_parameter_enum(par)
				== HDRL_PARAMETER_SPECTRUM1D_SHIFT, CPL_ERROR_ILLEGAL_INPUT, 0.0);

	const hdrl_spectrum1D_shift_parameter * p =
			(const hdrl_spectrum1D_shift_parameter *)par;
	return p->fit_wmin;
}

static inline hdrl_data_t
hdrl_shift_fit_parameter_get_range_wmin(const hdrl_parameter * par){

	cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, 0.0);
	cpl_ensure(hdrl_parameter_get_parameter_enum(par)
				== HDRL_PARAMETER_SPECTRUM1D_SHIFT, CPL_ERROR_ILLEGAL_INPUT, 0.0);

	const hdrl_spectrum1D_shift_parameter * p =
			(const hdrl_spectrum1D_shift_parameter *)par;
	return p->range_wmin;
}

static inline hdrl_data_t
hdrl_shift_fit_parameter_get_range_wmax(const hdrl_parameter * par){

	cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, 0.0);
	cpl_ensure(hdrl_parameter_get_parameter_enum(par)
				== HDRL_PARAMETER_SPECTRUM1D_SHIFT, CPL_ERROR_ILLEGAL_INPUT, 0.0);

	const hdrl_spectrum1D_shift_parameter * p =
			(const hdrl_spectrum1D_shift_parameter *)par;
	return p->range_wmax;
}

static inline hdrl_data_t
hdrl_shift_fit_parameter_get_wguess(const hdrl_parameter * par){

	cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, 0.0);
	cpl_ensure(hdrl_parameter_get_parameter_enum(par)
				== HDRL_PARAMETER_SPECTRUM1D_SHIFT, CPL_ERROR_ILLEGAL_INPUT, 0.0);

	const hdrl_spectrum1D_shift_parameter * p =
			(const hdrl_spectrum1D_shift_parameter *)par;
	return p->wguess;
}
/**@}*/
