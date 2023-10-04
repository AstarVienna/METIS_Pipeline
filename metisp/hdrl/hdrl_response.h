/* $Id: hdrl_spectrum_response.h,v 0.1 2017-04-25 09:02:08 msalmist Exp $
 *
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

/*
 * $Author: msalmist $
 * $Date: 2017-04-25 09:02:08 $
 * $Revision: 0.1 $
 * $Name: not supported by cvs2svn $
 */

#ifndef HDRL_RESPONSE_H_
#define HDRL_RESPONSE_H_

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>
#include "hdrl_spectrum.h"
#include "hdrl_spectrumlist.h"
#include "hdrl_spectrum_shift.h"

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                            		Functions
 -----------------------------------------------------------------------------*/
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
									hdrl_data_t lmax);

hdrl_parameter *
hdrl_response_fit_parameter_create(
		const cpl_size radius, const cpl_array * fit_points,
		const hdrl_data_t wrange, const cpl_bivector * high_abs_regions);

typedef struct{

	hdrl_spectrum1D * final_response;
	hdrl_spectrum1D * selected_response;
	hdrl_spectrum1D * raw_response;

	hdrl_spectrum1D * corrected_observed_spectrum;
	cpl_size best_telluric_model_idx;
    hdrl_data_t telluric_shift;
	hdrl_data_t avg_diff_from_1;
	hdrl_data_t stddev;

	hdrl_data_t doppler_shift;

}hdrl_response_result;

const hdrl_spectrum1D *
hdrl_response_result_get_final_response(const hdrl_response_result * res);
const hdrl_spectrum1D *
hdrl_response_result_get_selected_response(const hdrl_response_result * res);
const hdrl_spectrum1D *
hdrl_response_result_get_raw_response(const hdrl_response_result * res);
const hdrl_spectrum1D *
hdrl_response_result_get_corrected_obs_spectrum(const hdrl_response_result * res);
cpl_size
hdrl_response_result_get_best_telluric_model_idx(const hdrl_response_result * res);
hdrl_data_t
hdrl_response_result_get_avg_diff_from_1(const hdrl_response_result * res);
hdrl_data_t
hdrl_response_result_get_stddev(const hdrl_response_result * res);
hdrl_data_t
hdrl_response_result_get_telluric_shift(const hdrl_response_result * res);
hdrl_data_t
hdrl_response_result_get_doppler_shift(const hdrl_response_result * res);

void
hdrl_response_result_delete(hdrl_response_result *);

hdrl_response_result *
hdrl_response_compute(
        const hdrl_spectrum1D * obs_s,
		const hdrl_spectrum1D * ref_s,
		const hdrl_spectrum1D * E_x,
		const hdrl_parameter * telluric_par,
		const hdrl_parameter * velocity_par,
		const hdrl_parameter * calc_par,
		const hdrl_parameter * fit_par);


#if defined HDRL_USE_PRIVATE

hdrl_spectrum1D *
hdrl_response_evaluate_telluric_models(
								 const hdrl_spectrum1D * obs_s,
								 const hdrl_parameter * ev,
                                 hdrl_data_t * telluric_shift,
								 hdrl_data_t * mean_minus_1, hdrl_data_t * stddev,
								 cpl_size * best_model_index);

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
                                        double * telluric_shift);
#endif

CPL_END_DECLS

#endif /* HDRL_RESPONSE_H_ */
