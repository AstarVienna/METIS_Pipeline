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

#include "hdrl_response.h"
#include "hdrl_spectrum_resample.h"
#include "hdrl_efficiency.h"
#include "hdrl_utils.h"
#include <math.h>
#include <cpl.h>


/*-----------------------------------------------------------------------------
                          Private Functions
 -----------------------------------------------------------------------------*/
static inline  cpl_array *
get_uniform_wavs(const hdrl_spectrum1D * s, const hdrl_data_t w_step,
        const hdrl_data_t lmin, const hdrl_data_t lmax);

static inline hdrl_xcorrelation_result *
correlate_obs_with_telluric(const hdrl_spectrum1D * obs_s,
                                            const hdrl_spectrum1D * telluric_s,
                                            const hdrl_data_t w_step,
                                            cpl_size half_win,
                                            const cpl_boolean normalize,
                                            const hdrl_data_t lmin,
                                            const hdrl_data_t lmax);

static inline hdrl_spectrum1D *
shift_and_convolve_telluric_model(const hdrl_spectrum1D * obs,
                                              const hdrl_spectrum1D * telluric,
                                              const hdrl_data_t w_step,
                                              const cpl_size half_win,
                                              const cpl_boolean normalize,
                                              const hdrl_data_t lmin,
                                              const hdrl_data_t lmax,
                                              double * telluric_shift);

static inline cpl_size get_lower_odd(const cpl_size sz);

static inline hdrl_spectrum1D *
convolve_with_kernel_symmetrically(const hdrl_spectrum1D * s,
                                     const double sigma,
                                     const hdrl_data_t w_step);

static inline cpl_matrix *
create_symmetrical_gaussian_kernel(const double  slitw, const double  fwhm,
		const cpl_size max_sz);

static inline hdrl_spectrum1D *
convolve_spectrum_with_kernel(const hdrl_spectrum1D * s,
                         const cpl_matrix * kernel);

static inline double
erf_antideriv(const double x, const double sigma);


/*calculate ratio between obs spectrum and convolved and shifted telluric model*/
static inline hdrl_spectrum1D *
compute_corrected_obs_spectrum(
                              const hdrl_spectrum1D * obs_s_arg,
                              const hdrl_spectrum1D * telluric_s_arg,
                              const hdrl_data_t w_step,
                              const cpl_size half_win,
                              const cpl_boolean normalize,
                              const cpl_boolean shift_in_log_scale,
                              const hdrl_data_t lmin, const hdrl_data_t lmax,
                              double * telluric_shift);

static inline hdrl_spectrum1D *
compute_interpolated_spectrum(const hdrl_spectrum1D * wlength_source,
        const hdrl_spectrum1D * sampled_points,
		const hdrl_spectrum1D_interpolation_method method);

static inline void
compute_quality(const hdrl_spectrum1D * s,
        const cpl_bivector * quality_areas, double * mean_abs_difference_from_1,
		double * stddev);

static inline void
free_spectrum_array(hdrl_spectrum1D ** s, const cpl_size sz);

static inline cpl_error_code
get_first_error_code(const cpl_error_code * codes, const cpl_size sz);

static inline hdrl_spectrum1D *
select_win(const hdrl_spectrum1D * s, const hdrl_data_t wmin,
        const hdrl_data_t wmax);

static inline hdrl_spectrum1D *
select_obs_wlen( const hdrl_spectrum1D * s,
		const hdrl_spectrum1D * wlens_source);

static inline hdrl_spectrum1D *
correct_spectrum_for_doppler_shift(const hdrl_spectrum1D * s,
					const hdrl_data_t offset);

static inline hdrl_spectrum1D *
filter_spectrum_median(const hdrl_spectrum1D * resp, const cpl_size radius);

static inline hdrl_image *
compute_median_on_hdrl_image(const hdrl_image * img, const cpl_size radius);

static inline hdrl_spectrum1D *
resample_on_medians_skip_abs_regions(const hdrl_spectrum1D * s,
		const cpl_array * fit_points, const cpl_bivector * high_abs_regions,
		const hdrl_data_t wrange);

static inline cpl_array *
remove_regions_and_outliers_from_array(const cpl_array * fit_points,
		const cpl_bivector * high_abs_regions,
		const hdrl_data_t wmin, const hdrl_data_t wmax);

static inline cpl_boolean
contained_in_any_region(const hdrl_data_t w, const cpl_bivector * high_abs_regions);

static inline hdrl_spectrum1D *
get_median_on_fit_points(const hdrl_spectrum1D * s_input,
		const cpl_array * fit_points, const hdrl_data_t wrange);

static inline hdrl_spectrum1D *
remove_bad_data(const hdrl_spectrum1D * s);

static inline const hdrl_spectrum1Dlist *
hdrl_response_telluric_evaluation_parameter_get_telluric_models(
		const hdrl_parameter * par);

static inline hdrl_data_t
hdrl_response_telluric_evaluation_parameter_get_w_step(
		const hdrl_parameter * par);

static inline cpl_size
hdrl_response_telluric_evaluation_parameter_get_half_win(
		const hdrl_parameter * par);

static inline cpl_boolean
hdrl_response_telluric_evaluation_parameter_get_normalize(
		const hdrl_parameter * par);

static inline cpl_boolean
hdrl_response_telluric_evaluation_parameter_get_shift_in_log_scale(
		const hdrl_parameter * par);

static inline const cpl_bivector *
hdrl_response_telluric_evaluation_parameter_get_quality_areas(
		const hdrl_parameter * par);

static inline const cpl_bivector *
hdrl_response_telluric_evaluation_parameter_get_fit_areas(
		const hdrl_parameter * par);

static inline hdrl_data_t
hdrl_response_telluric_evaluation_parameter_get_lmin(
		const hdrl_parameter * par);

static inline hdrl_data_t
hdrl_response_telluric_evaluation_parameter_get_lmax(
		const hdrl_parameter * par);

static inline const cpl_array *
hdrl_response_parameter_get_fit_points(
		const hdrl_parameter * par);

static inline const cpl_bivector *
hdrl_response_parameter_get_high_abs_regions(
		const hdrl_parameter * par);

static inline cpl_size
hdrl_response_parameter_get_radius(
		const hdrl_parameter * par);

static inline hdrl_data_t
hdrl_response_parameter_get_wrange(
		const hdrl_parameter * par);

static inline hdrl_spectrum1D *
	hdrl_spectrum1D_extract_fit_regions(const hdrl_spectrum1D * s,
			const cpl_bivector * areas);

/*Private data structures*/

/*Parameter used for the telluric evaluation*/
typedef struct {
     HDRL_PARAMETER_HEAD;
	 hdrl_spectrum1Dlist * telluric_models;
	 hdrl_data_t w_step;
	 cpl_size half_win;
	 cpl_boolean normalize;
	 cpl_boolean shift_in_log_scale;
	 cpl_bivector * quality_areas;
	 cpl_bivector * fit_areas;
	 hdrl_data_t lmin;
	 hdrl_data_t lmax;
}hdrl_response_telluric_evaluation_parameter;

static inline void
hdrl_response_telluric_evaluation_parameter_delete(
		hdrl_parameter * ev){

	if(ev == NULL) return;

	if(hdrl_parameter_get_parameter_enum(ev)
			!= HDRL_PARAMETER_RESPONSE_TELLURIC_EVALUATION)
		return;

	hdrl_response_telluric_evaluation_parameter * par =
			(hdrl_response_telluric_evaluation_parameter *)ev;

	hdrl_spectrum1Dlist_delete(par->telluric_models);
	cpl_bivector_delete(par->quality_areas);
	cpl_bivector_delete(par->fit_areas);
	cpl_free(par);
}

static hdrl_parameter_typeobj
hdrl_telluric_eval_parameters_type = {
		HDRL_PARAMETER_RESPONSE_TELLURIC_EVALUATION,      			/* type */
    (hdrl_alloc *)&cpl_malloc,                							/* fp_alloc */
    (hdrl_free *)&hdrl_response_telluric_evaluation_parameter_delete, /* fp_free */
    NULL,                                     							/* fp_destroy */
    sizeof(hdrl_response_telluric_evaluation_parameter),  			/* obj_size */
};

/*Parameter used for the response*/
typedef struct {
     HDRL_PARAMETER_HEAD;
     cpl_size radius;
     cpl_array * fit_points;
     cpl_bivector * high_abs_regions;
     hdrl_data_t wrange;
}response_fit_parameter;

static inline void
hdrl_response_fit_parameter_delete(
		hdrl_parameter * ev){

	if(ev == NULL) return;

	if(hdrl_parameter_get_parameter_enum(ev)
			!= HDRL_PARAMETER_RESPONSE_FIT)
		return;

	response_fit_parameter * par =
			(response_fit_parameter *)ev;

	cpl_bivector_delete(par->high_abs_regions);
	cpl_array_delete(par->fit_points);
	cpl_free(par);
}

static hdrl_parameter_typeobj
hdrl_response_parameters_type = {
	HDRL_PARAMETER_RESPONSE_FIT,      			/* type */
    (hdrl_alloc *)&cpl_malloc,                			/* fp_alloc */
    (hdrl_free *)&hdrl_response_fit_parameter_delete, /* fp_free */
    NULL,                                     			/* fp_destroy */
    sizeof(response_fit_parameter),  		/* obj_size */
};


static inline hdrl_spectrum1D *
hdrl_spectrum1D_create_from_buffers(double * flux, double * wlens, cpl_size sz,
		hdrl_spectrum1D_wave_scale scale);

static inline hdrl_response_result *
hdrl_response_result_wrap(hdrl_spectrum1D * final_response,
		  	  	  	      hdrl_spectrum1D * selected_response,
						  hdrl_spectrum1D * raw_response,

						  hdrl_spectrum1D * corrected_observed_spectrum,
						  cpl_size best_telluric_model_idx,
                          hdrl_data_t telluric_shift,
						  hdrl_data_t avg_diff_from_1,
						  hdrl_data_t stddev,

						  hdrl_data_t doppler_shift);

/**
 *
 * @addtogroup hdrl_response
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*-----------------------------------------------------------------------------
                                   Functions
 -----------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief ctor for the hdrl_parameter for the telluric evaluation
 * @param telluric_models		The available telluric models
 * @param w_step				Sampling step to use when upsampling model and
 * 								observed spectrum to calculate the cross
 * 								correlations
 * @param half_win				Half the search window to be used to find the
 * 								peak of the cross correlation
 * @param normalize				CPL_TRUE if the cross correlation should be
 * 								normalized, CPL_FALSE otherwise
 * @param shift_in_log_scale	CPL_TRUE if the cross correlation has to be
 * 								calculated	in logarithmic scale.
 * 								CPL_FALSE otherwise.
 * @param quality_areas			Areas where the quality of the fit of the
 * 								telluric model has to be evaluated.
 * @param fit_areas				Areas where the median points are extracted from,
 * 								in order to generate the final quality parameters
 * 								of the telluric model
 * @param lmin					Minimum wavelength used to calculate the
 * 								cross-correlation (in log scale if
 * 								shift_in_log_scale = TRUE)
 * @param lmax					Maximum wavelength used to calculate the
 * 								cross-correlation (in log scale if
 * 								shift_in_log_scale = TRUE)
 * @return hdrl_parameter or NULL in case of error, see below.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: if any of the pointers are NULL
 * - CPL_ERROR_ILLEGAL_INPUT: if w_step <= 0 or half_win <= 0 or lmin >= lmax
 */
/* ---------------------------------------------------------------------------*/

hdrl_parameter *
hdrl_response_telluric_evaluation_parameter_create(
										const hdrl_spectrum1Dlist * telluric_models,
										hdrl_data_t w_step,
										cpl_size half_win,
										cpl_boolean normalize,
										cpl_boolean shift_in_log_scale,
										const cpl_bivector * quality_areas,
										const cpl_bivector * fit_areas,
										hdrl_data_t lmin,
										hdrl_data_t lmax){

    cpl_ensure(quality_areas != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(telluric_models != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(fit_areas != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(w_step > 0.0, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(half_win > 0, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(lmin < lmax, CPL_ERROR_ILLEGAL_INPUT, NULL);

    hdrl_response_telluric_evaluation_parameter * ev
        = (hdrl_response_telluric_evaluation_parameter *)
           hdrl_parameter_new(&hdrl_telluric_eval_parameters_type);

    ev->telluric_models = hdrl_spectrum1Dlist_duplicate(telluric_models);
    ev->w_step = w_step;
    ev->half_win = half_win;
    ev->normalize = normalize;
    ev->shift_in_log_scale = shift_in_log_scale;
    ev->quality_areas = cpl_bivector_duplicate(quality_areas);
    ev->fit_areas = cpl_bivector_duplicate(fit_areas);
    ev->lmin = lmin;
    ev->lmax = lmax;

    return (hdrl_parameter *)ev;
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief ctor for the hdrl_parameter for the final interpolation of the response
 * @param radius				Radius of the median filter used to smooth the
 * 								response
 * 								before the final interpolation
 * @param fit_points			Median points where the fit will be calculated
 * @param wrange				Range around the median point where the median
 * 								is calculated
 * @param high_abs_regions		High absorption regions that should be skipped
 * 								when calculating the fit. If NULL no skipping is done.
 *
 * @return hdrl_parameter or NULL in case of error, see below.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: if any of the required pointers are NULL
 * - CPL_ERROR_ILLEGAL_INPUT: radius or wrange is less or equal 0
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter *
hdrl_response_fit_parameter_create(
		const cpl_size radius, const cpl_array * fit_points,
		const hdrl_data_t wrange, const cpl_bivector * high_abs_regions){

	cpl_ensure(radius > 0, CPL_ERROR_ILLEGAL_INPUT, NULL);
	cpl_ensure(wrange > 0.0, CPL_ERROR_ILLEGAL_INPUT, NULL);
	cpl_ensure(fit_points != NULL, CPL_ERROR_NULL_INPUT, NULL);

	response_fit_parameter * p
	        = (response_fit_parameter *)
	           hdrl_parameter_new(&hdrl_response_parameters_type);

	p->fit_points = cpl_array_duplicate(fit_points);
	p->high_abs_regions =  NULL;

	if(high_abs_regions)
		p->high_abs_regions = cpl_bivector_duplicate(high_abs_regions);

	p->radius = radius;
	p->wrange = wrange;

	return (hdrl_parameter *) p;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Computation of the response
 * @param obs_s				Observed spectrum
 * @param ref_s				Reference std star spectrum
 * @param E_x				Atmospheric Extinction
 * @param telluric_par		Telluric correction parameter,
 * 							NULL if telluric correction is skipped.
 * 							See hdrl_response_telluric_evaluation_parameter_create
 * 							for details.
 * @param velocity_par		Doppler shift estimation and compensation. NULL if
 * 							compensation has to be skipped.
 * 							See hdrl_spectrum1D_shift_fit_parameter_create for
 * 							more details.
 * @param calc_par			Parameter for the core computation of the response,
 * 							e.g. exposure time.
 * 							See hdrl_response_parameter_create for more details.
 * @param fit_par			Parameter for the final interpolation of the response,
 * 							see hdrl_response_fit_parameter_create for more details.
 *
 * @return hdrl_response_result or NULL in case of error, see below.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: if any among calc_par, fit_par  or the spectra
 * 						   are NULL
 * - CPL_ERROR_ILLEGAL_OUTPUT: Any of the algorithmical steps can fail.
 * 							   This usually means that the output  of that step
 * 							   can be NULL, if that is the case we abort triggering
 * 							   a CPL_ERROR_ILLEGAL_OUTPUT
 */
/* ---------------------------------------------------------------------------*/
hdrl_response_result *
hdrl_response_compute(
        const hdrl_spectrum1D * obs_s,
		const hdrl_spectrum1D * ref_s,
		const hdrl_spectrum1D * E_x,
		const hdrl_parameter * telluric_par,
		const hdrl_parameter * velocity_par,
		const hdrl_parameter * calc_par,
		const hdrl_parameter * fit_par){

	cpl_ensure(calc_par != NULL, CPL_ERROR_NULL_INPUT, NULL);
	cpl_ensure(hdrl_parameter_get_parameter_enum(calc_par) ==
			HDRL_PARAMETER_EFFICIENCY, CPL_ERROR_ILLEGAL_INPUT, NULL);

	if(telluric_par)
		cpl_ensure(hdrl_parameter_get_parameter_enum(telluric_par) ==
				HDRL_PARAMETER_RESPONSE_TELLURIC_EVALUATION,
				CPL_ERROR_ILLEGAL_INPUT, NULL);

	if(velocity_par)
		cpl_ensure(hdrl_parameter_get_parameter_enum(velocity_par) ==
				HDRL_PARAMETER_SPECTRUM1D_SHIFT,
				CPL_ERROR_ILLEGAL_INPUT, NULL);

	cpl_ensure(fit_par != NULL, CPL_ERROR_NULL_INPUT, NULL);
	cpl_ensure(hdrl_parameter_get_parameter_enum(fit_par) ==
				HDRL_PARAMETER_RESPONSE_FIT, CPL_ERROR_ILLEGAL_INPUT, NULL);

	cpl_ensure(obs_s != NULL, CPL_ERROR_NULL_INPUT, NULL);
	cpl_ensure(ref_s != NULL, CPL_ERROR_NULL_INPUT, NULL);
	cpl_ensure(E_x != NULL, CPL_ERROR_NULL_INPUT, NULL);

	hdrl_data_t best_mean_minus1 = 0, best_stddev = 0;
	hdrl_data_t best_telluric_shift = 0;
	cpl_size best_idx = -1;

	hdrl_spectrum1D * corrected_obs =
			hdrl_response_evaluate_telluric_models(obs_s, telluric_par,
			        &best_telluric_shift, &best_mean_minus1, &best_stddev, &best_idx);

	cpl_ensure(best_idx >= 0, CPL_ERROR_ILLEGAL_OUTPUT, NULL);
	cpl_ensure(corrected_obs != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

	cpl_ensure(cpl_error_get_code() == CPL_ERROR_NONE,
			CPL_ERROR_ILLEGAL_OUTPUT, NULL);

	hdrl_data_t velocity_shift = 0.0;
	if(velocity_par != NULL){
		velocity_shift =
				hdrl_spectrum1D_compute_shift_fit(corrected_obs, velocity_par);
	}

	cpl_ensure(cpl_error_get_code() == CPL_ERROR_NONE,
			CPL_ERROR_ILLEGAL_OUTPUT, NULL);

	hdrl_spectrum1D * ref_shifted =
			correct_spectrum_for_doppler_shift(ref_s,
					velocity_shift);

	cpl_ensure(ref_shifted != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);
	cpl_ensure(cpl_error_get_code() == CPL_ERROR_NONE,
			CPL_ERROR_ILLEGAL_OUTPUT, NULL);

	hdrl_spectrum1D * resp_raw =
			hdrl_response_core_compute(corrected_obs, ref_shifted, E_x, calc_par);

	cpl_ensure(resp_raw != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);
	cpl_ensure(cpl_error_get_code() == CPL_ERROR_NONE,
			CPL_ERROR_ILLEGAL_OUTPUT, NULL);

	const cpl_size radius = hdrl_response_parameter_get_radius(fit_par);
	const cpl_bivector * high_abs_regions =
			hdrl_response_parameter_get_high_abs_regions(fit_par);
	const cpl_array * fit_points =
				hdrl_response_parameter_get_fit_points(fit_par);
	const hdrl_data_t wrange =
			hdrl_response_parameter_get_wrange(fit_par);

	cpl_ensure(cpl_error_get_code() == CPL_ERROR_NONE,
				CPL_ERROR_ILLEGAL_OUTPUT, NULL);

	hdrl_spectrum1D * resp_smoothed =
			filter_spectrum_median(resp_raw, radius);

	cpl_ensure(resp_smoothed != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);
	cpl_ensure(cpl_error_get_code() == CPL_ERROR_NONE, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

	hdrl_spectrum1D * resp_on_fit_points =
			resample_on_medians_skip_abs_regions(resp_smoothed, fit_points,
					high_abs_regions, wrange);

	cpl_ensure(resp_on_fit_points != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);
	cpl_ensure(cpl_error_get_code() == CPL_ERROR_NONE,
			CPL_ERROR_ILLEGAL_OUTPUT, NULL);


	hdrl_parameter * par =
			hdrl_spectrum1D_resample_interpolate_parameter_create(hdrl_spectrum1D_interp_akima);

	hdrl_spectrum1D * resp_final =
			hdrl_spectrum1D_resample_on_array(resp_on_fit_points,
					hdrl_spectrum1D_get_wavelength(resp_smoothed).wavelength,
					par);

	hdrl_parameter_delete(par);
	hdrl_spectrum1D_delete(&resp_smoothed);
	hdrl_spectrum1D_delete(&ref_shifted);


	return hdrl_response_result_wrap(resp_final, resp_on_fit_points,
			resp_raw, corrected_obs, best_idx, best_telluric_shift,
			best_mean_minus1, best_stddev, velocity_shift);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Getter for the final response contained inside the hdrl_response_result
 * @param res			 hdrl_response_result
 *
 * @return response or NULL in case of error. The final response is the final
 * product of the algorithm.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: res is NULL
 */
/* ---------------------------------------------------------------------------*/

const hdrl_spectrum1D *
hdrl_response_result_get_final_response(const hdrl_response_result * res){
	cpl_ensure(res != NULL, CPL_ERROR_NULL_INPUT, NULL);
	return res->final_response;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Getter for the selected response contained inside the hdrl_response_result
 * @param res			 hdrl_response_result
 *
 * @return  selected response or NULL in case of error. The selected response is
 * the raw response sampled in the fit points. This response is going then to be
 * interpolated, creating the final response.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: res is NULL
 */
/* ---------------------------------------------------------------------------*/

const hdrl_spectrum1D *
hdrl_response_result_get_selected_response(const hdrl_response_result * res){
	cpl_ensure(res != NULL, CPL_ERROR_NULL_INPUT, NULL);
	return res->selected_response;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Getter for the raw response contained inside the hdrl_response_result
 * @param res			 hdrl_response_result
 *
 * @return the raw response or NULL in case of error. The raw response is
 * the ratio between the observed spectrum and the reference one, corrected for
 * e.g. gain, atmospheric extinction, etc.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: res is NULL
 */
/* ---------------------------------------------------------------------------*/
const hdrl_spectrum1D *
hdrl_response_result_get_raw_response(const hdrl_response_result * res){
	cpl_ensure(res != NULL, CPL_ERROR_NULL_INPUT, NULL);
	return res->raw_response;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Getter for the corrected observed spectrum contained in hdrl_response_result
 * @param res			 hdrl_response_result
 *
 * @return the observed spectrum corrected by the telluric model. If telluric
 * correction was disabled the output of this function is not defined.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: res is NULL
 */
/* ---------------------------------------------------------------------------*/
const hdrl_spectrum1D *
hdrl_response_result_get_corrected_obs_spectrum(const hdrl_response_result * res){
	cpl_ensure(res != NULL, CPL_ERROR_NULL_INPUT, NULL);
	return res->corrected_observed_spectrum;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Getter of the index of the telluric model used for telluric correction
 * contained in hdrl_response_result
 * @param res			 hdrl_response_result
 *
 * @return the index (0-based) of the telluric model used for telluric correction.
 * If telluric correction was disabled the output of this function is not defined.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: res is NULL
 */
/* ---------------------------------------------------------------------------*/
cpl_size
hdrl_response_result_get_best_telluric_model_idx(const hdrl_response_result * res){
	cpl_ensure(res != NULL, CPL_ERROR_NULL_INPUT, -1);
	return res->best_telluric_model_idx;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Getter of the value |mean - 1|, where mean is the average of the ratio
 * between the corrected observed spectrum and its smoothed fit.
 * @param res			 hdrl_response_result
 *
 * @return the value |mean - 1|, where mean is the average of the ratio
 * between the corrected observed spectrum and its smoothed fit.
 * This value can be used to assess the quality of the match of the telluric model
 * with the provided observed spectrum.
 * If telluric correction was disabled the output of this function is not defined.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: res is NULL
 */
/* ---------------------------------------------------------------------------*/
hdrl_data_t
hdrl_response_result_get_avg_diff_from_1(const hdrl_response_result * res){
	cpl_ensure(res != NULL, CPL_ERROR_NULL_INPUT, NAN);
	return  res->avg_diff_from_1;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Getter of the standard deviation of the ratio between the corrected
 * observed spectrum and its smoothed fit.
 * @param res			 hdrl_response_result
 *
 * @return the standard deviation of the ratio between the corrected observed
 * spectrum and its smoothed fit.
 * This value can be used to assess the quality of the match of the telluric model
 * with the provided observed spectrum.
 * If telluric correction was disabled the output of this function is not defined.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: res is NULL
 */
/* ---------------------------------------------------------------------------*/
hdrl_data_t
hdrl_response_result_get_stddev(const hdrl_response_result * res){
	cpl_ensure(res != NULL, CPL_ERROR_NULL_INPUT, NAN);
	return res->stddev;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Getter of the shift applied to the telluric model.
 * @param res            hdrl_response_result
 *
 * @return shift applied to the selected telluric model.
 * This value can be used to assess the quality of the match of the telluric model
 * with the provided observed spectrum.
 * If telluric correction was disabled the output of this function is not defined.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: res is NULL
 */
/* ---------------------------------------------------------------------------*/
hdrl_data_t
hdrl_response_result_get_telluric_shift(const hdrl_response_result * res){
    cpl_ensure(res != NULL, CPL_ERROR_NULL_INPUT, NAN);
    return res->telluric_shift;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Getter of the doppler shift used to correct the model.
 * @param res			 hdrl_response_result
 *
 * @return the doppler shift used to correct the model.
 * If doppler corection was disabled the output of this function is not defined.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: res is NULL
 */
/* ---------------------------------------------------------------------------*/
hdrl_data_t
hdrl_response_result_get_doppler_shift(const hdrl_response_result * res){
	cpl_ensure(res != NULL, CPL_ERROR_NULL_INPUT, NAN);
	return res->doppler_shift;
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief Destructor for hdrl_response_result
 * @param res			 hdrl_response_result to be destroyed
 */
/* ---------------------------------------------------------------------------*/
void
hdrl_response_result_delete(hdrl_response_result * res){

	if(!res) return;

	hdrl_spectrum1D_delete(&(res->final_response));
	hdrl_spectrum1D_delete(&(res->selected_response));
	hdrl_spectrum1D_delete(&(res->raw_response));

	hdrl_spectrum1D_delete(&(res->corrected_observed_spectrum));

	cpl_free(res);
}

/* This function evaluates all the telluric models inside the hdrl_parameter ev,
 * picks the best model, returns its index (best_model_index), some quality parameter
 * (mean_minus_1 and stddev) and obs_s corrected with the best model.
 * If ev == NULL the function returns a copy of obs_s.
 * ev is created using hdrl_response_telluric_evaluation_parameter_create. */
hdrl_spectrum1D *
hdrl_response_evaluate_telluric_models(
                                     const hdrl_spectrum1D * obs_s,
                                     const hdrl_parameter * ev,
                                     hdrl_data_t * telluric_shift,
                                     hdrl_data_t * mean_minus_1, hdrl_data_t * stddev,
                                     cpl_size * best_model_index){

	cpl_ensure(mean_minus_1 != NULL, CPL_ERROR_NULL_INPUT, NULL);
	cpl_ensure(stddev != NULL, CPL_ERROR_NULL_INPUT, NULL);
	cpl_ensure(best_model_index != NULL, CPL_ERROR_NULL_INPUT, NULL);
	cpl_ensure(obs_s != NULL, CPL_ERROR_NULL_INPUT, NULL);

	*mean_minus_1 = (hdrl_data_t)0.0;
    *stddev = (hdrl_data_t)0.0;
    *best_model_index = -1;

	if(ev == NULL){
		*best_model_index  = 0;
		*mean_minus_1 = NAN;
		*stddev = NAN;
		*telluric_shift = NAN;
		return hdrl_spectrum1D_duplicate(obs_s);
	}

	cpl_ensure(hdrl_parameter_get_parameter_enum(ev)
			== HDRL_PARAMETER_RESPONSE_TELLURIC_EVALUATION,
			CPL_ERROR_ILLEGAL_INPUT, NULL);

	 const hdrl_spectrum1Dlist * telluric_models =
			 hdrl_response_telluric_evaluation_parameter_get_telluric_models(ev);
	 const hdrl_data_t w_step =
			 hdrl_response_telluric_evaluation_parameter_get_w_step(ev);
	 const cpl_size half_win =
			 hdrl_response_telluric_evaluation_parameter_get_half_win(ev);
	 const cpl_boolean normalize =
			 hdrl_response_telluric_evaluation_parameter_get_normalize(ev);
	 const cpl_boolean shift_in_log_scale =
			 hdrl_response_telluric_evaluation_parameter_get_shift_in_log_scale(ev);
	 const cpl_bivector * quality_areas =
			 hdrl_response_telluric_evaluation_parameter_get_quality_areas(ev);
	 const cpl_bivector * fit_areas =
			 hdrl_response_telluric_evaluation_parameter_get_fit_areas(ev);
	 const hdrl_data_t lmin =
			 hdrl_response_telluric_evaluation_parameter_get_lmin(ev);
	 const hdrl_data_t lmax =
			 hdrl_response_telluric_evaluation_parameter_get_lmax(ev);

    hdrl_spectrum1D * to_ret = NULL;

    const cpl_size sz = hdrl_spectrum1Dlist_get_size(telluric_models);
    cpl_ensure(sz > 0, CPL_ERROR_ILLEGAL_INPUT, NULL);

    cpl_array * calc_std_devs = cpl_array_new(sz, CPL_TYPE_DOUBLE);
    cpl_array * calc_means_minus_1 = cpl_array_new(sz, CPL_TYPE_DOUBLE);
    cpl_array * calc_telluric_shift = cpl_array_new(sz, CPL_TYPE_DOUBLE);

    cpl_array_fill_window(calc_std_devs, 0, sz, 0);
    cpl_array_fill_window(calc_means_minus_1, 0, sz, 0);
    cpl_array_fill_window(calc_telluric_shift, 0, sz, 0);

    double * p_stddevs = cpl_array_get_data_double(calc_std_devs);
    double * p_means = cpl_array_get_data_double(calc_means_minus_1);
    double * p_telluric_shift = cpl_array_get_data_double(calc_telluric_shift);

    hdrl_spectrum1D ** l = cpl_calloc(sz, sizeof(hdrl_spectrum1D*));
    cpl_error_code * codes = cpl_calloc(sz, sizeof(cpl_error_code));

    HDRL_OMP(omp parallel for)
    for(cpl_size i = 0; i < sz; ++i){
        const hdrl_spectrum1D * this_model =
                hdrl_spectrum1Dlist_get_const(telluric_models, i);

        l[i] = hdrl_response_evaluate_telluric_model(obs_s, this_model, w_step,
                half_win, normalize, shift_in_log_scale, quality_areas, fit_areas,
                lmin, lmax, p_means + i, p_stddevs + i,p_telluric_shift + i);
        codes[i] = cpl_error_get_code();

        if(l[i] == NULL && codes[i] == CPL_ERROR_NONE)
            codes[i] = CPL_ERROR_ILLEGAL_OUTPUT;
    }

    cpl_error_code err = get_first_error_code(codes, sz);

    if(!err){
    	cpl_size best_idx = 0;
        err = cpl_array_get_minpos(calc_means_minus_1, &best_idx);
        if(!err){
        	*stddev = cpl_array_get(calc_std_devs, best_idx, NULL);
        	*mean_minus_1 = cpl_array_get(calc_means_minus_1, best_idx, NULL);
        	*telluric_shift = cpl_array_get(calc_telluric_shift, best_idx, NULL);
        	to_ret = l[best_idx];
        	l[best_idx] = NULL;
        	*best_model_index = best_idx;
        }
    }

    cpl_array_delete(calc_std_devs);
    cpl_array_delete(calc_means_minus_1);
    cpl_array_delete(calc_telluric_shift);
    cpl_free(codes);
    free_spectrum_array(l, sz);

    cpl_ensure(err == CPL_ERROR_NONE, err, NULL);

    return to_ret;
}



/*This function evaluates how well a telluric model corrects an observed spectrum.
 * For the parameters see hdrl_response_telluric_evaluation_parameter_create.*/
hdrl_spectrum1D *
hdrl_response_evaluate_telluric_model(const hdrl_spectrum1D * obs_s_arg,
                                        const hdrl_spectrum1D * telluric_s_arg,
                                        const hdrl_data_t w_step,
                                        const cpl_size half_win,
                                        const cpl_boolean normalize,
                                        const cpl_boolean shift_in_log_scale,
                                        const cpl_bivector * quality_areas,
                                        const cpl_bivector * fit_areas,
                                        const hdrl_data_t lmin, const hdrl_data_t lmax,
                                        double * mean_minus_1, double * stddev,
                                        double * telluric_shift){

    cpl_ensure(obs_s_arg != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(telluric_s_arg != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(quality_areas != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(fit_areas != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(mean_minus_1 != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(stddev != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(w_step > 0, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(half_win > 0, CPL_ERROR_ILLEGAL_INPUT, NULL);

    *mean_minus_1 = 0.0;
    *stddev  = 0.0;
    *telluric_shift = 0.0;

    hdrl_spectrum1D * corrected_spectrum =
            compute_corrected_obs_spectrum(obs_s_arg,
                    telluric_s_arg, w_step, half_win, normalize,
                    shift_in_log_scale, lmin, lmax, telluric_shift);

    cpl_ensure(corrected_spectrum != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

    hdrl_spectrum1D * corr_spectrum_extracted =
    		hdrl_spectrum1D_extract_fit_regions(corrected_spectrum, fit_areas);

    if(corr_spectrum_extracted == NULL) {
        hdrl_spectrum1D_delete(&corrected_spectrum);
        cpl_ensure(CPL_FALSE, CPL_ERROR_ILLEGAL_OUTPUT, NULL);
    }

    hdrl_spectrum1D * smoothed_fit =
            compute_interpolated_spectrum(corrected_spectrum,
            		corr_spectrum_extracted, hdrl_spectrum1D_interp_akima);

    hdrl_spectrum1D * quality_ratio =
            hdrl_spectrum1D_div_spectrum_create(corrected_spectrum, smoothed_fit);

    compute_quality(quality_ratio, quality_areas, mean_minus_1, stddev);

    hdrl_spectrum1D_delete(&corr_spectrum_extracted);
    hdrl_spectrum1D_delete(&smoothed_fit);
    hdrl_spectrum1D_delete(&quality_ratio);

    return corrected_spectrum;
}

/*-----------------------------------------------------------------------------
                          Private Functions
 -----------------------------------------------------------------------------*/

/* Given a spectrum the function calculates:
 * mean_abs_difference_from_1 = |mean - 1| and standard deviation.
 * The flux defined on wavelengths outside the quality_areas are ignored.*/
static inline void
compute_quality(const hdrl_spectrum1D * s,
        const cpl_bivector * quality_areas, double * mean_abs_difference_from_1,
		double * stddev){

    hdrl_spectrum1D * s_new =
            hdrl_spectrum1D_select_wavelengths(s, quality_areas, CPL_TRUE);

    const hdrl_image * flux = hdrl_spectrum1D_get_flux(s_new);

    *mean_abs_difference_from_1 = fabs(hdrl_image_get_mean(flux).data - 1.0);
    *stddev = hdrl_image_get_stdev(flux);

    hdrl_spectrum1D_delete(&s_new);
}

/* Interpolates using a cspline the points in sampled_points to obtain a spectrum
 * defined on the wavelengths of wlength_source*/
static inline hdrl_spectrum1D *
compute_interpolated_spectrum(const hdrl_spectrum1D * wlength_source,
        const hdrl_spectrum1D * sampled_points,
		const hdrl_spectrum1D_interpolation_method method){

    hdrl_parameter * par =
            hdrl_spectrum1D_resample_interpolate_parameter_create(method);

    hdrl_spectrum1D_wavelength waves = hdrl_spectrum1D_get_wavelength(wlength_source);

    hdrl_spectrum1D * continuum_fit =
            hdrl_spectrum1D_resample(sampled_points, &waves, par);

    hdrl_parameter_delete(par);
    return continuum_fit;
}

/* This function correct the observed spectrum by the telluric spectrum. In order
 * to do so, the function aligns the two spectra, convolves the telluric model by
 * a gaussian kernel. The observed spectrum is then divided by the shifted and
 * convolved telluric model.
 * NOTE: the output spectrum is defined on the wavelengths of obs_s_arg.*/
static inline hdrl_spectrum1D *
compute_corrected_obs_spectrum(
                              const hdrl_spectrum1D * obs_s_arg,
                              const hdrl_spectrum1D * telluric_s_arg,
                              const hdrl_data_t w_step, const cpl_size half_win,
                              const cpl_boolean normalize,
                              const cpl_boolean shift_in_log_scale,
                              const hdrl_data_t lmin, const hdrl_data_t lmax,
                              double * telluric_shift){

    cpl_ensure(obs_s_arg != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(telluric_s_arg != NULL, CPL_ERROR_NULL_INPUT, NULL);

    hdrl_spectrum1D_wavelength obs_wavs = hdrl_spectrum1D_get_wavelength(obs_s_arg);

    hdrl_spectrum1D * obs_s = hdrl_spectrum1D_duplicate(obs_s_arg);
    hdrl_spectrum1D * telluric_s_cp = hdrl_spectrum1D_duplicate(telluric_s_arg);

    if(shift_in_log_scale){
        hdrl_spectrum1D_wavelength_convert_to_log(obs_s);
        hdrl_spectrum1D_wavelength_convert_to_log(telluric_s_cp);
    }

    hdrl_spectrum1D * telluric_s_shifted_convolved=
            shift_and_convolve_telluric_model(obs_s, telluric_s_cp, w_step,
                    half_win, normalize, lmin, lmax, telluric_shift);


    if(telluric_s_shifted_convolved != NULL){
        hdrl_spectrum1D_wavelength_convert_to_linear(telluric_s_shifted_convolved);
    }

    hdrl_parameter * pars = hdrl_spectrum1D_resample_integrate_parameter_create();
    hdrl_spectrum1D * telluric_s_shifted_convolved_downsampled =
                hdrl_spectrum1D_resample(telluric_s_shifted_convolved, &obs_wavs, pars);
    hdrl_spectrum1D * corrected =
            hdrl_spectrum1D_div_spectrum_create(obs_s_arg , telluric_s_shifted_convolved_downsampled);

    hdrl_spectrum1D_delete(&obs_s);
    hdrl_spectrum1D_delete(&telluric_s_cp);
    hdrl_spectrum1D_delete(&telluric_s_shifted_convolved);
    hdrl_spectrum1D_delete(&telluric_s_shifted_convolved_downsampled);
    hdrl_parameter_delete(pars);

    return corrected;
}

/*Trims s so that its minimum and maximum wavelenghts do not exceed wlens_source*/
static inline hdrl_spectrum1D *
select_obs_wlen( const hdrl_spectrum1D * s,
		const hdrl_spectrum1D * wlens_source){

	const cpl_array * wlens = hdrl_spectrum1D_get_wavelength(wlens_source).wavelength;

	const hdrl_data_t wmin = cpl_array_get_min(wlens);
	const hdrl_data_t wmax = cpl_array_get_max(wlens);

	return select_win(s, wmin, wmax);
}

/*The telluric model is correlated with the observed spectrum to compute the
 * relative shift between the two. Then the telluric model is shifted to match the
 * observed spectrum. Then the shifted model is convolved with a gaussian kernel. */
static inline hdrl_spectrum1D *
shift_and_convolve_telluric_model(const hdrl_spectrum1D * obs,
								  const hdrl_spectrum1D * telluric,
								  const hdrl_data_t w_step,
								  const cpl_size half_win,
								  const cpl_boolean normalize,
								  const hdrl_data_t lmin,
 								  const hdrl_data_t lmax,
 								  double * telluric_shift){

    hdrl_spectrum1D * telluric_s = select_win(telluric, lmin, lmax);

	cpl_ensure(telluric_s != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

    hdrl_xcorrelation_result * res =
            correlate_obs_with_telluric(obs, telluric_s,
                    w_step, half_win,  normalize, lmin, lmax);
    hdrl_spectrum1D_delete(&telluric_s);

    cpl_ensure(res != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

    hdrl_data_t shift = (hdrl_data_t)
            (hdrl_xcorrelation_result_get_peak_subpixel(res) -
             hdrl_xcorrelation_result_get_half_window(res) * w_step);

    *telluric_shift = shift;

    hdrl_spectrum1D * telluric_selected_obs =
    		select_obs_wlen(telluric, obs);

    hdrl_spectrum1D * telluric_s_shifted =
            hdrl_spectrum1D_wavelength_shift_create(telluric_selected_obs, shift);
    const double sigma = hdrl_xcorrelation_result_get_sigma(res);

    hdrl_xcorrelation_result_delete(res);

    cpl_ensure(telluric_s_shifted != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

    hdrl_spectrum1D * telluric_s_shifted_convolved =
            convolve_with_kernel_symmetrically(telluric_s_shifted, sigma, w_step);

    hdrl_spectrum1D_delete(&telluric_s_shifted);
    hdrl_spectrum1D_delete(&telluric_selected_obs);

    return telluric_s_shifted_convolved;
}

/*The function: 1. resamples uniformly obs_s and telluric_s, so they're defined
 * on the same wavelengths. 2. correlates the two spectra to find out the relative
 * shift of one wrt the other.
 *
 * w_step, lmin and lmax are used for the resampling, all the other inputs
 * are used for the cross correlation.*/
static inline hdrl_xcorrelation_result *
correlate_obs_with_telluric(const hdrl_spectrum1D * obs_s,
                                            const hdrl_spectrum1D * telluric_s,
                                            const hdrl_data_t w_step,
                                            const cpl_size half_win,
                                            const cpl_boolean normalize,
                                            const hdrl_data_t lmin,
                                            const hdrl_data_t lmax){

    cpl_ensure(obs_s != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(telluric_s != NULL, CPL_ERROR_NULL_INPUT, NULL);

    hdrl_data_t wmin = cpl_array_get_min(hdrl_spectrum1D_get_wavelength(obs_s).wavelength);
    hdrl_data_t wmax = cpl_array_get_max(hdrl_spectrum1D_get_wavelength(obs_s).wavelength);

    hdrl_spectrum1D * tell_for_sel = select_win(telluric_s, wmin, wmax);

    hdrl_spectrum1D * telluric_s_res = NULL;
    hdrl_spectrum1D * obs_s_res = NULL;
    {
        cpl_array * new_lambdas = get_uniform_wavs(tell_for_sel, w_step, lmin, lmax);

        /*
         * We need to make sure that telluric_s and obs_s are
         * sampled uniformly.
        */
        hdrl_parameter * par =
                hdrl_spectrum1D_resample_interpolate_parameter_create
                (hdrl_spectrum1D_interp_akima);

        telluric_s_res =
                hdrl_spectrum1D_resample_on_array(telluric_s, new_lambdas, par);

        obs_s_res =
                hdrl_spectrum1D_resample_on_array(obs_s, new_lambdas, par);

        hdrl_parameter_delete(par);
        cpl_array_delete(new_lambdas);
    }

    cpl_ensure(obs_s_res != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);
    cpl_ensure(telluric_s_res != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

    hdrl_xcorrelation_result * gshift =
            hdrl_spectrum1D_compute_shift_xcorrelation(telluric_s_res, obs_s_res,
                    half_win, normalize);

    hdrl_spectrum1D_delete(&telluric_s_res);
    hdrl_spectrum1D_delete(&obs_s_res);
    hdrl_spectrum1D_delete(&tell_for_sel);
    return gshift;
}


/*This function returns a uniformly sampled sequence of wavelengths. The distance
 * between 2 elements is w_step, the starting point is max(lmin, min_wavelengths_s)
 * the end point is min(lmax, max_wavelegths_s)*/
static inline  cpl_array *
get_uniform_wavs(const hdrl_spectrum1D * s, const hdrl_data_t w_step,
        const hdrl_data_t lmin, const hdrl_data_t lmax){

    const hdrl_data_t w_min = CPL_MAX(lmin,
            cpl_array_get_min(hdrl_spectrum1D_get_wavelength(s).wavelength));
    const hdrl_data_t w_max = CPL_MIN(lmax,
            cpl_array_get_max(hdrl_spectrum1D_get_wavelength(s).wavelength));

    const cpl_size sz_new_spectrum = (w_max - w_min)/w_step;
    cpl_array * new_w_lengths = cpl_array_new(sz_new_spectrum, HDRL_TYPE_DATA);

    for(cpl_size i = 0; i < sz_new_spectrum; ++i){
        cpl_array_set(new_w_lengths, i, i * w_step + w_min);
    }

   return new_w_lengths;
}

/*This function convolves a kernel with the flux of a spectrum. The output
 * spectrum is without error and it is defined on the same wavelengths of the input
 * spectrum. The convolution one the borders is done using a reduced number of samples.*/
static inline hdrl_spectrum1D *
convolve_spectrum_with_kernel(const hdrl_spectrum1D * s,
                         const cpl_matrix * kernel){

    const cpl_size sz = hdrl_spectrum1D_get_size(s);
    const hdrl_image * h_img = hdrl_spectrum1D_get_flux(s);
    const cpl_image * img = hdrl_image_get_image_const(h_img);
    cpl_image * dest = cpl_image_new(sz, 1, HDRL_TYPE_DATA);

    const cpl_error_code cd =
            cpl_image_filter(dest, img, kernel, CPL_FILTER_LINEAR, CPL_BORDER_FILTER);


    hdrl_spectrum1D * to_ret = NULL;
    if(cd == CPL_ERROR_NONE){
        hdrl_spectrum1D_wavelength s_wav = hdrl_spectrum1D_get_wavelength(s);

        to_ret =
                hdrl_spectrum1D_create_error_free(dest, s_wav.wavelength, s_wav.scale);
    }

    cpl_image_delete(dest);

    cpl_ensure(cd == CPL_ERROR_NONE, cd, NULL);

    return to_ret;
}

static inline cpl_size get_lower_odd(const cpl_size sz){

	if(sz == 0) return 0;

	if(sz % 2 == 1) return sz;
	return sz - 1;
}

/*This function convolves the spectrum s with a symmetrical gaussian kernel having
 * std deviation sigma. The wavelength step of the kernel is w_step.*/
static inline hdrl_spectrum1D *
convolve_with_kernel_symmetrically(const hdrl_spectrum1D * s,
                                   const double sigma,
                                   const hdrl_data_t w_step){

	const double fwhm = CPL_MATH_FWHM_SIG * sigma;
    int fwhm_pix = (int) (fwhm / w_step + 0.5);

    cpl_matrix * kernel =
            create_symmetrical_gaussian_kernel(fwhm_pix / CPL_MATH_FWHM_SIG,
                                        fwhm_pix / CPL_MATH_FWHM_SIG,
										get_lower_odd(hdrl_spectrum1D_get_size(s)));

    hdrl_spectrum1D * convolved =
    		convolve_spectrum_with_kernel(s, kernel);
    cpl_matrix_delete(kernel);

    cpl_ensure(convolved != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

    return convolved;
}

/* Creates a gaussian symmetrical kernel for a given slit width (slitw) and a given
 * fwhm. The function ALWAYS returns a kernel with a odd number of elements.*/
static inline cpl_matrix *
create_symmetrical_gaussian_kernel(const double  slitw, const double  fwhm,
		const cpl_size max_sz){

    cpl_ensure(slitw > 0.0,  CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(fwhm  > 0.0,  CPL_ERROR_ILLEGAL_INPUT, NULL);

    const double   sigma  = fwhm * CPL_MATH_SIG_FWHM;
    cpl_size size   = 1 + (cpl_size)(5.0 * sigma + 0.5*slitw);

    size *= 2;
    /* filter need an odd number of elements */
    size ++;

    size = CPL_MIN(size, max_sz);

    cpl_matrix * kernel = cpl_matrix_new(1, size);

    /* Special case for i = 0 */
    cpl_matrix_set(kernel, 0, size/2,
                         (erf_antideriv(0.5*slitw + 0.5, sigma) -
                          erf_antideriv(0.5*slitw - 0.5, sigma)) / slitw);

    for (cpl_size i = 1; i < size / 2; i++) {

        const double x1p = (double)i + 0.5 * slitw + 0.5;
        const double x1n = (double)i - 0.5 * slitw + 0.5;
        const double x0p = (double)i + 0.5 * slitw - 0.5;
        const double x0n = (double)i - 0.5 * slitw - 0.5;
        const double val = 0.5 / slitw *
            (erf_antideriv(x1p, sigma) - erf_antideriv(x1n, sigma) -
             erf_antideriv(x0p, sigma) + erf_antideriv(x0n, sigma));

        cpl_matrix_set(kernel, 0, size/2 + i, val);
        cpl_matrix_set(kernel, 0, size/2 - i, val);
    }

    return kernel;
}

/* The antiderivative of erx(x/sigma/sqrt(2)) with respect to x */
static inline double
erf_antideriv(const double x, const double sigma)
{
    return x * erf( x / (sigma * CPL_MATH_SQRT2))
       + 2.0 * sigma/CPL_MATH_SQRT2PI * exp(-0.5 * x * x / (sigma * sigma));
}

/* Given an array of spectra the function takes care of freeing each spectra
 * and the buffer containing the pointers.*/
static inline void
free_spectrum_array(hdrl_spectrum1D ** s, const cpl_size sz){
    hdrl_spectrum1Dlist * l = hdrl_spectrum1Dlist_wrap(s, sz);
    hdrl_spectrum1Dlist_delete(l);
}

/*Get the first element of the array that is not CPL_ERROR_NONE. If all the
 * elements are CPL_ERROR_NONE the function returns CPL_ERROR_NONE.*/
static inline cpl_error_code
get_first_error_code(const cpl_error_code * codes, const cpl_size sz){
    for(cpl_size i = 0; i < sz; ++i){
        if(codes[i]) return codes[i];
    }
    return CPL_ERROR_NONE;
}

/*Selects all the wavelengths between wmin and wmax*/
static inline hdrl_spectrum1D *
select_win(const hdrl_spectrum1D * s, const hdrl_data_t wmin,
        const hdrl_data_t wmax){

    cpl_bivector * bv = cpl_bivector_new(1);

    cpl_vector_set(cpl_bivector_get_x(bv), 0, wmin);
    cpl_vector_set(cpl_bivector_get_y(bv), 0, wmax);

    hdrl_spectrum1D * to_ret = hdrl_spectrum1D_select_wavelengths(s, bv, CPL_TRUE);

    cpl_bivector_delete(bv);

    return to_ret;
}
/*Corrects the spectrum s by the doppler offset offset.*/
static inline hdrl_spectrum1D *
correct_spectrum_for_doppler_shift(const hdrl_spectrum1D * s,
					const hdrl_data_t offset){

	if(offset == 0.0)
		return hdrl_spectrum1D_duplicate(s);

	const hdrl_image * flux = hdrl_spectrum1D_get_flux(s);
	cpl_array * wavs =
			cpl_array_duplicate(hdrl_spectrum1D_get_wavelength(s).wavelength);

	for(cpl_size i = 0; i < cpl_array_get_size(wavs); ++i){
		const double d = cpl_array_get(wavs, i, NULL) * (1. + offset);
		cpl_array_set(wavs, i, d);
	}

	hdrl_spectrum1D * to_ret = hdrl_spectrum1D_create(
		hdrl_image_get_image_const(flux),
		hdrl_image_get_error_const(flux),
		wavs,
		hdrl_spectrum1D_get_scale(s));
	cpl_array_delete(wavs);
	return to_ret;
}
/*Median filters the flux, with error propagation*/
static inline hdrl_spectrum1D *
filter_spectrum_median(const hdrl_spectrum1D * resp, const cpl_size radius){
	const hdrl_image  * flx_total = hdrl_spectrum1D_get_flux(resp);
	hdrl_image * flx_smoothed = compute_median_on_hdrl_image(flx_total, radius);

	hdrl_spectrum1D * to_ret = hdrl_spectrum1D_create(
			hdrl_image_get_image(flx_smoothed),
			hdrl_image_get_error(flx_smoothed),
			hdrl_spectrum1D_get_wavelength(resp).wavelength,
			hdrl_spectrum1D_get_scale(resp));

	hdrl_image_delete(flx_smoothed);

	return to_ret;
}

/*Median filters an HDRL image, with error propagation*/
static inline hdrl_image *
compute_median_on_hdrl_image(const hdrl_image * img, const cpl_size radius){

	hdrl_image * to_ret = hdrl_image_duplicate(img);
	cpl_size sz = hdrl_image_get_size_x(img);
	for(cpl_size i = 1; i <= sz; ++i){
		cpl_size start = CPL_MAX(i - radius, 1);
		cpl_size stop = CPL_MIN(i + radius, sz);
		hdrl_image * ex_img = hdrl_image_extract(img, start, 1, stop, 1);
		hdrl_value m = hdrl_image_get_median(ex_img);
		hdrl_image_delete(ex_img);
		hdrl_image_set_pixel(to_ret, i, 1, m);
	}
	return to_ret;
}

/*Remove rejected values or values being NAN or INF*/
static inline hdrl_spectrum1D *
remove_bad_data(const hdrl_spectrum1D * s){

	const cpl_size sz = hdrl_spectrum1D_get_size(s);
	double * flx = cpl_calloc(sz, sizeof(double));
	double * flx_e = cpl_calloc(sz, sizeof(double));
	double * wlen = cpl_calloc(sz, sizeof(double));

	cpl_size true_size = 0;
	for(cpl_size i = 0; i < sz; ++i){
		int rej = 0;
		hdrl_value v = hdrl_spectrum1D_get_flux_value(s, i, &rej);
		if(rej || isnan(v.data) || isinf(v.data)) continue;

		flx[true_size] = v.data;
		flx_e[true_size] = v.error;
		wlen[true_size] = hdrl_spectrum1D_get_wavelength_value(s, i, &rej);
		true_size++;
	}

	if(true_size == 0){
		cpl_free(flx);
		cpl_free(flx_e);
		cpl_free(wlen);
		return NULL;
	}

	hdrl_spectrum1D_wave_scale scale = hdrl_spectrum1D_get_scale(s);
	cpl_image * img_flx = cpl_image_wrap_double(true_size, 1, flx);
	cpl_image * img_flx_e = cpl_image_wrap_double(true_size, 1, flx_e);
	cpl_array * arr_wlens= cpl_array_wrap_double(wlen, true_size);
	hdrl_spectrum1D * good_spectrum = hdrl_spectrum1D_create(img_flx, img_flx_e,
			arr_wlens, scale);

	cpl_image_delete(img_flx);
	cpl_image_delete(img_flx_e);
	cpl_array_delete(arr_wlens);
	return good_spectrum;
}

/* For each point p in fit_points generates a new spectrum whose wavelengths are
 * fit_points and whose flux are the medians taken in the range
 * [p - wrange, p + wrange]. */
static inline hdrl_spectrum1D *
get_median_on_fit_points(const hdrl_spectrum1D * s_input,
		const cpl_array * fit_points, const hdrl_data_t wrange){

	cpl_size final_fit_points_size = cpl_array_get_size(fit_points);

	cpl_array * wlens_fit = cpl_array_new(final_fit_points_size,
			HDRL_TYPE_DATA);
	hdrl_image * flux_fit = hdrl_image_new(final_fit_points_size, 1);

	for(cpl_size i = 0; i < final_fit_points_size; ++i){
		const double w_fit = cpl_array_get(fit_points, i, NULL);
		cpl_array_set(wlens_fit, i, w_fit);
		hdrl_spectrum1D * f_s =
				select_win(s_input, w_fit - wrange, w_fit + wrange);

		if(f_s == NULL){
			cpl_error_reset();
			hdrl_image_reject(flux_fit, i + 1, 1);
			continue;
		}

		const hdrl_value v = hdrl_image_get_median(hdrl_spectrum1D_get_flux(f_s));
		hdrl_image_set_pixel(flux_fit, i + 1, 1, v);
		hdrl_spectrum1D_delete(&f_s);
	}

	const hdrl_spectrum1D_wave_scale scale = hdrl_spectrum1D_get_scale(s_input);

	hdrl_spectrum1D * to_ret = hdrl_spectrum1D_create(
			hdrl_image_get_image(flux_fit),
			hdrl_image_get_error(flux_fit),
			wlens_fit,
			scale
			);
	cpl_array_delete(wlens_fit);
	hdrl_image_delete(flux_fit);
	return to_ret;
}
/*removes high abs regions and values that are NAN or INF or rejected*/
static inline hdrl_spectrum1D *
select_regions_and_good_value(const hdrl_spectrum1D * s, const cpl_bivector * areas){

	hdrl_spectrum1D * filted = NULL;
	if(areas != NULL)
			filted = hdrl_spectrum1D_select_wavelengths(s, areas, CPL_FALSE);
	else
			filted = hdrl_spectrum1D_duplicate(s);

	hdrl_spectrum1D * to_ret = remove_bad_data(filted);
	hdrl_spectrum1D_delete(&filted);
	return to_ret;
}

/*This function:
 * 1. Removes all the wavelengths contained inside high_abs_regions, from both s
 * 	  and fit_points
 * 2. For each surviving point in fit_points take the median flux on the filtered s
*/
static inline hdrl_spectrum1D *
resample_on_medians_skip_abs_regions(const hdrl_spectrum1D * s,
		const cpl_array * fit_points, const cpl_bivector * high_abs_regions,
		const hdrl_data_t wrange){

	cpl_ensure(s != NULL, CPL_ERROR_NULL_INPUT, NULL);
	cpl_ensure(fit_points != NULL, CPL_ERROR_NULL_INPUT, NULL);

	hdrl_spectrum1D * filter_s =
			select_regions_and_good_value(s, high_abs_regions);

	cpl_ensure(filter_s != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

	const hdrl_data_t wmin=
				cpl_array_get_min(hdrl_spectrum1D_get_wavelength(filter_s).wavelength);
	const hdrl_data_t wmax=
				cpl_array_get_max(hdrl_spectrum1D_get_wavelength(filter_s).wavelength);

	cpl_array * filter_fit_points =
			remove_regions_and_outliers_from_array(fit_points, high_abs_regions, wmin, wmax);

	if(filter_fit_points == NULL || cpl_array_get_size(filter_fit_points) == 0){
		hdrl_spectrum1D_delete(&filter_s);
		cpl_array_delete(filter_fit_points);
		cpl_ensure(CPL_FALSE, CPL_ERROR_ILLEGAL_OUTPUT, NULL);
	}

	hdrl_spectrum1D * selected_s_median_points =
			get_median_on_fit_points(filter_s, filter_fit_points, wrange);

	cpl_array_delete(filter_fit_points);
	hdrl_spectrum1D_delete(&filter_s);

	return selected_s_median_points;
}

/*Checks if w is contained in any of the windows in high_abs_regions.*/
static inline cpl_boolean
contained_in_any_region(const hdrl_data_t w, const cpl_bivector * high_abs_regions){

	if(!high_abs_regions) return CPL_FALSE;

	const cpl_size sz = cpl_bivector_get_size(high_abs_regions);

	for(cpl_size i = 0; i < sz; ++i){
		const double wmin =
				cpl_vector_get(cpl_bivector_get_x_const(high_abs_regions), i);
		const double wmax =
				cpl_vector_get(cpl_bivector_get_y_const(high_abs_regions), i);
		if(w >= wmin && w <= wmax) return CPL_TRUE;
	}
	return CPL_FALSE;
}

/*Removes each element in the array that is outside the range [wmin, wmax] or that
 * is contained inside the high_abs_regions*/
static inline cpl_array *
remove_regions_and_outliers_from_array(const cpl_array * fit_points,
		const cpl_bivector * high_abs_regions,
		const hdrl_data_t wmin, const hdrl_data_t wmax){

	const cpl_size sz = cpl_array_get_size(fit_points);
	double * filter = cpl_calloc(sz, sizeof(*filter));
	cpl_size k = 0;
	for(cpl_size i = 0; i < sz; ++i){
		const double w = cpl_array_get(fit_points, i, NULL);
		if(w > wmax) continue;
		if(w < wmin) continue;
		if(contained_in_any_region(w, high_abs_regions)) continue;

		filter[k] = w;
		k++;
	}

	if(k == 0){
		cpl_free(filter);
		return NULL;
	}

	return cpl_array_wrap_double(filter, k);
}

static inline const hdrl_spectrum1Dlist *
hdrl_response_telluric_evaluation_parameter_get_telluric_models(
		const hdrl_parameter * par){

	cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, NULL);
	cpl_ensure(hdrl_parameter_get_parameter_enum(par)
				== HDRL_PARAMETER_RESPONSE_TELLURIC_EVALUATION,
				CPL_ERROR_ILLEGAL_INPUT, NULL);

	const hdrl_response_telluric_evaluation_parameter * p =
			(const hdrl_response_telluric_evaluation_parameter *)par;
	return p->telluric_models;
}

static inline hdrl_data_t
hdrl_response_telluric_evaluation_parameter_get_w_step(
		const hdrl_parameter * par){

	cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, 0.0);
	cpl_ensure(hdrl_parameter_get_parameter_enum(par)
				== HDRL_PARAMETER_RESPONSE_TELLURIC_EVALUATION,
				CPL_ERROR_ILLEGAL_INPUT, 0.0);

	const hdrl_response_telluric_evaluation_parameter * p =
			(const hdrl_response_telluric_evaluation_parameter *)par;
	return p->w_step;
}

static inline cpl_size
hdrl_response_telluric_evaluation_parameter_get_half_win(
		const hdrl_parameter * par){

	cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, 0);
	cpl_ensure(hdrl_parameter_get_parameter_enum(par)
				== HDRL_PARAMETER_RESPONSE_TELLURIC_EVALUATION,
				CPL_ERROR_ILLEGAL_INPUT, 0);

	const hdrl_response_telluric_evaluation_parameter * p =
			(const hdrl_response_telluric_evaluation_parameter *)par;
	return p->half_win;
}

static inline cpl_boolean
hdrl_response_telluric_evaluation_parameter_get_normalize(
		const hdrl_parameter * par){

	cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, CPL_FALSE);
	cpl_ensure(hdrl_parameter_get_parameter_enum(par)
				== HDRL_PARAMETER_RESPONSE_TELLURIC_EVALUATION,
				CPL_ERROR_ILLEGAL_INPUT, CPL_FALSE);

	const hdrl_response_telluric_evaluation_parameter * p =
			(const hdrl_response_telluric_evaluation_parameter *)par;
	return p->normalize;
}

static inline cpl_boolean
hdrl_response_telluric_evaluation_parameter_get_shift_in_log_scale(
		const hdrl_parameter * par){

	cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, CPL_FALSE);
	cpl_ensure(hdrl_parameter_get_parameter_enum(par)
				== HDRL_PARAMETER_RESPONSE_TELLURIC_EVALUATION,
				CPL_ERROR_ILLEGAL_INPUT, CPL_FALSE);

	const hdrl_response_telluric_evaluation_parameter * p =
			(const hdrl_response_telluric_evaluation_parameter *)par;
	return p->shift_in_log_scale;
}

static inline const cpl_bivector *
hdrl_response_telluric_evaluation_parameter_get_quality_areas(
		const hdrl_parameter * par){

	cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, NULL);
	cpl_ensure(hdrl_parameter_get_parameter_enum(par)
				== HDRL_PARAMETER_RESPONSE_TELLURIC_EVALUATION,
				CPL_ERROR_ILLEGAL_INPUT, NULL);

	const hdrl_response_telluric_evaluation_parameter * p =
			(const hdrl_response_telluric_evaluation_parameter *)par;
	return p->quality_areas;
}

static inline const cpl_bivector *
hdrl_response_telluric_evaluation_parameter_get_fit_areas(
		const hdrl_parameter * par){

	cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, NULL);
	cpl_ensure(hdrl_parameter_get_parameter_enum(par)
				== HDRL_PARAMETER_RESPONSE_TELLURIC_EVALUATION,
				CPL_ERROR_ILLEGAL_INPUT, NULL);

	const hdrl_response_telluric_evaluation_parameter * p =
			(const hdrl_response_telluric_evaluation_parameter *)par;
	return p->fit_areas;
}

static inline hdrl_data_t
hdrl_response_telluric_evaluation_parameter_get_lmin(
		const hdrl_parameter * par){

	cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, 0.0);
	cpl_ensure(hdrl_parameter_get_parameter_enum(par)
				== HDRL_PARAMETER_RESPONSE_TELLURIC_EVALUATION,
				CPL_ERROR_ILLEGAL_INPUT, 0.0);

	const hdrl_response_telluric_evaluation_parameter * p =
			(const hdrl_response_telluric_evaluation_parameter *)par;
	return p->lmin;
}

static inline hdrl_data_t
hdrl_response_telluric_evaluation_parameter_get_lmax(
		const hdrl_parameter * par){

	cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, 0.0);
	cpl_ensure(hdrl_parameter_get_parameter_enum(par)
				== HDRL_PARAMETER_RESPONSE_TELLURIC_EVALUATION,
				CPL_ERROR_ILLEGAL_INPUT, 0.0);

	const hdrl_response_telluric_evaluation_parameter * p =
			(const hdrl_response_telluric_evaluation_parameter *)par;
	return p->lmax;
}

static inline const cpl_array *
hdrl_response_parameter_get_fit_points(
		const hdrl_parameter * par){

	cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, NULL);
	cpl_ensure(hdrl_parameter_get_parameter_enum(par)
				== HDRL_PARAMETER_RESPONSE_FIT,
				CPL_ERROR_ILLEGAL_INPUT, NULL);

	const response_fit_parameter * p =
			(const response_fit_parameter *)par;
	return p->fit_points;
}

static inline const cpl_bivector *
hdrl_response_parameter_get_high_abs_regions(
		const hdrl_parameter * par){

	cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, NULL);
	cpl_ensure(hdrl_parameter_get_parameter_enum(par)
				== HDRL_PARAMETER_RESPONSE_FIT,
				CPL_ERROR_ILLEGAL_INPUT, NULL);

	const response_fit_parameter * p =
			(const response_fit_parameter *)par;
	return p->high_abs_regions;
}

static inline cpl_size
hdrl_response_parameter_get_radius(
		const hdrl_parameter * par){

	cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, 0);
	cpl_ensure(hdrl_parameter_get_parameter_enum(par)
				== HDRL_PARAMETER_RESPONSE_FIT,
				CPL_ERROR_ILLEGAL_INPUT, 0);

	const response_fit_parameter * p =
			(const response_fit_parameter *)par;
	return p->radius;
}

static inline hdrl_data_t
hdrl_response_parameter_get_wrange(
		const hdrl_parameter * par){

	cpl_ensure(par != NULL, CPL_ERROR_NULL_INPUT, 0.0);
	cpl_ensure(hdrl_parameter_get_parameter_enum(par)
				== HDRL_PARAMETER_RESPONSE_FIT,
				CPL_ERROR_ILLEGAL_INPUT, 0.0);

	const response_fit_parameter * p =
			(const response_fit_parameter *)par;
	return p->wrange;
}

/*wrapper around the hdrl_spectrum1D constructor that accepts double arrays*/
static inline hdrl_spectrum1D *
hdrl_spectrum1D_create_from_buffers(double * flux, double * wlens, cpl_size sz,
		hdrl_spectrum1D_wave_scale scale){

	cpl_array * w = cpl_array_wrap_double(wlens, sz);
	cpl_image * fl = cpl_image_wrap_double(sz, 1, flux);

	hdrl_spectrum1D * to_ret = hdrl_spectrum1D_create_error_free(fl, w, scale);

	cpl_array_unwrap(w);
	cpl_image_unwrap(fl);
	return to_ret;
}
/* For every window in "areas", extract a flux point having as wavelength the
 * middle point of the window and as flux the median of the flux value defined
 * on the window.*/
static inline hdrl_spectrum1D *
	hdrl_spectrum1D_extract_fit_regions(const hdrl_spectrum1D * s,
			const cpl_bivector * areas){

	const double step = 1.0;

	cpl_size sz = cpl_bivector_get_size(areas);
	const cpl_vector * l_min = cpl_bivector_get_x_const(areas);
	const cpl_vector * l_max = cpl_bivector_get_y_const(areas);

	double * flux = cpl_calloc(sz + 2, sizeof(double));
	double * wlens = cpl_calloc(sz + 2, sizeof(double));

	const hdrl_data_t wmin =
			cpl_array_get_min(hdrl_spectrum1D_get_wavelength(s).wavelength);
	const hdrl_data_t wmax =
			cpl_array_get_max(hdrl_spectrum1D_get_wavelength(s).wavelength);
	{
		hdrl_spectrum1D * s_sel =
				select_win(s, wmin - step, wmin + step);

		const hdrl_image * sel_flux = hdrl_spectrum1D_get_flux(s_sel);
		flux[0] = hdrl_image_get_median(sel_flux).data;
		wlens[0] = wmin;

		hdrl_spectrum1D_delete(&s_sel);
	}

	cpl_size size_sel = 1;
	for(cpl_size i = 0; i < sz; ++i){
		const double lambda_min = cpl_vector_get(l_min, i);
		const double lambda_max = cpl_vector_get(l_max, i);

		hdrl_spectrum1D * s_sel =
				select_win(s, lambda_min, lambda_max);

		if(s_sel == NULL){
			cpl_error_reset();
			continue;
		}
		wlens[size_sel] = .5 * (lambda_max + lambda_min);
		const hdrl_image * sel_flux = hdrl_spectrum1D_get_flux(s_sel);
		flux[size_sel] = hdrl_image_get_median(sel_flux).data;

		hdrl_spectrum1D_delete(&s_sel);
		size_sel++;
	}

	{
		hdrl_spectrum1D * s_sel =
				select_win(s, wmax - step, wmax + step);
		const hdrl_image * sel_flux = hdrl_spectrum1D_get_flux(s_sel);
		flux[size_sel] = hdrl_image_get_median(sel_flux).data;
		wlens[size_sel] = wmax;
		hdrl_spectrum1D_delete(&s_sel);
		size_sel++;
	}

	hdrl_spectrum1D * sel_s = NULL;
	if(size_sel > 0)
	{
		const hdrl_spectrum1D_wave_scale scale = hdrl_spectrum1D_get_scale(s);
		sel_s = hdrl_spectrum1D_create_from_buffers(flux, wlens, size_sel, scale);
	}

	cpl_free(flux);
	cpl_free(wlens);

	return sel_s;
}
/*ctor for hdrl_response_result.
 * NOTE: hdrl_response_result gets ownership of all the pointers provided.
 * DO NOT deallocate them, they will be de-allocated by hdrl_response_result dtor.
 * */
static inline hdrl_response_result *
hdrl_response_result_wrap(hdrl_spectrum1D * final_response,
                          hdrl_spectrum1D * selected_response,
                          hdrl_spectrum1D * raw_response,

                          hdrl_spectrum1D * corrected_observed_spectrum,
                          cpl_size best_telluric_model_idx,
                          hdrl_data_t telluric_shift,
                          hdrl_data_t avg_diff_from_1,
                          hdrl_data_t stddev,

                          hdrl_data_t doppler_shift){

	hdrl_response_result * to_ret = cpl_calloc(1, sizeof(*to_ret));

	to_ret->final_response = final_response;
	to_ret->raw_response = raw_response;
	to_ret->selected_response = selected_response;

	to_ret->corrected_observed_spectrum = corrected_observed_spectrum;
	to_ret->best_telluric_model_idx = best_telluric_model_idx;
	to_ret->telluric_shift = telluric_shift;
	to_ret->avg_diff_from_1 = avg_diff_from_1;
	to_ret->stddev = stddev;

	to_ret->doppler_shift = doppler_shift;

	return to_ret;
}


/**@}*/
