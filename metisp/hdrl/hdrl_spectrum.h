/* $Id: hdrl_spectrum.h,v 0.1 2017-02-28 16:20:00 msalmist Exp $
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
 * $Date: 2017-02-28 16:20:00 $
 * $Revision: 0.1 $
 * $Name: not supported by cvs2svn $
 */

#ifndef HDRL_SPECTRUM_H
#define HDRL_SPECTRUM_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_image.h"
#include "hdrl_correlation.h"
#include "hdrl_spectrum_defs.h"

#include <cpl.h>

CPL_BEGIN_DECLS

#if defined HDRL_USE_EXPERIMENTAL || defined HDRL_USE_PRIVATE

cpl_boolean
hdrl_spectrum1D_is_uniformly_sampled(const hdrl_spectrum1D * self, double * bin);

void  hdrl_spectrum1D_save(const hdrl_spectrum1D * s, const char * fname);

#endif

/*-----------------------------------------------------------------------------
                              Functions - Constructors
 -----------------------------------------------------------------------------*/
hdrl_spectrum1D * hdrl_spectrum1D_create(const cpl_image * arg_flux,
                                         const cpl_image * arg_flux_e,
                                         const cpl_array * wavelength,
                                         hdrl_spectrum1D_wave_scale wave_scale);

hdrl_spectrum1D * hdrl_spectrum1D_create_error_free
                                      (const cpl_image * arg_flux,
                                      const cpl_array * wavelength,
                                      hdrl_spectrum1D_wave_scale scale);

hdrl_spectrum1D * hdrl_spectrum1D_create_error_DER_SNR
                                      (const cpl_image * arg_flux,
                                      cpl_size half_window,
                                      const cpl_array * wavelength,
                                      hdrl_spectrum1D_wave_scale scale);

typedef hdrl_value (* calculate_analytic_spectrum_point) (hdrl_data_t lambda);

hdrl_spectrum1D * hdrl_spectrum1D_create_analytic(
                                         calculate_analytic_spectrum_point func,
                                         const cpl_array * wavelength,
                                         hdrl_spectrum1D_wave_scale scale);

hdrl_spectrum1D * hdrl_spectrum1D_duplicate(const hdrl_spectrum1D* self);

void hdrl_spectrum1D_delete(hdrl_spectrum1D ** p_self);


/*-----------------------------------------------------------------------------
                            Functions - Getter operations
 -----------------------------------------------------------------------------*/

cpl_size hdrl_spectrum1D_get_size(const hdrl_spectrum1D * self);

const hdrl_image*
hdrl_spectrum1D_get_flux(const hdrl_spectrum1D * self);


typedef struct{
    const cpl_array * wavelength;
    const cpl_mask * bpm;
    hdrl_spectrum1D_wave_scale scale;
}hdrl_spectrum1D_wavelength;

hdrl_spectrum1D_wavelength
hdrl_spectrum1D_get_wavelength(const hdrl_spectrum1D * self);

hdrl_spectrum1D_wave_scale
hdrl_spectrum1D_get_scale(const hdrl_spectrum1D * self);


hdrl_value
hdrl_spectrum1D_get_flux_value(const hdrl_spectrum1D * self, int idx, int* rej);

hdrl_data_t
hdrl_spectrum1D_get_wavelength_value(const hdrl_spectrum1D * self, int idx, int* rej);


/*-----------------------------------------------------------------------------
                            Functions - Vectorial flux operations
 -----------------------------------------------------------------------------*/

cpl_boolean
hdrl_spectrum1D_are_spectra_compatible(const hdrl_spectrum1D_wavelength* s1,
                                       const hdrl_spectrum1D_wavelength* s2);
cpl_boolean
hdrl_spectrum1D_are_wavelengths_compatible(const cpl_array* w1,
                                           const cpl_array* w2);
hdrl_spectrum1D *
hdrl_spectrum1D_div_spectrum_create(const hdrl_spectrum1D * num,
                                    const hdrl_spectrum1D * den);

hdrl_spectrum1D *
hdrl_spectrum1D_mul_spectrum_create(const hdrl_spectrum1D * f1,
                                    const hdrl_spectrum1D * f2);

hdrl_spectrum1D *
hdrl_spectrum1D_add_spectrum_create(const hdrl_spectrum1D * f1,
                                    const hdrl_spectrum1D * f2);

hdrl_spectrum1D *
hdrl_spectrum1D_sub_spectrum_create(const hdrl_spectrum1D * f1,
                                    const hdrl_spectrum1D * f2);


cpl_error_code hdrl_spectrum1D_div_spectrum(      hdrl_spectrum1D * self,
                                            const hdrl_spectrum1D * other);

cpl_error_code hdrl_spectrum1D_mul_spectrum(      hdrl_spectrum1D * self,
                                            const hdrl_spectrum1D * other);

cpl_error_code hdrl_spectrum1D_add_spectrum(      hdrl_spectrum1D * self,
                                            const hdrl_spectrum1D * other);

cpl_error_code hdrl_spectrum1D_sub_spectrum(      hdrl_spectrum1D * self,
                                            const hdrl_spectrum1D * other);

/*-----------------------------------------------------------------------------
                            Functions - Scalar flux operations
 -----------------------------------------------------------------------------*/

hdrl_spectrum1D *
hdrl_spectrum1D_div_scalar_create(const hdrl_spectrum1D * self,
                                  hdrl_value scalar_operator);

hdrl_spectrum1D *
hdrl_spectrum1D_mul_scalar_create(const hdrl_spectrum1D * self,
                                  hdrl_value scalar_operator);

hdrl_spectrum1D *
hdrl_spectrum1D_add_scalar_create(const hdrl_spectrum1D * self,
                                  hdrl_value scalar_operator);

hdrl_spectrum1D *
hdrl_spectrum1D_sub_scalar_create(const hdrl_spectrum1D * self,
                                  hdrl_value scalar_operator);

hdrl_spectrum1D *
hdrl_spectrum1D_pow_scalar_create(const hdrl_spectrum1D * self,
                                  hdrl_value scalar_operator);

hdrl_spectrum1D *
hdrl_spectrum1D_exp_scalar_create(const hdrl_spectrum1D * self,
                                  hdrl_value scalar_operator);

cpl_error_code hdrl_spectrum1D_div_scalar(hdrl_spectrum1D * self,
                                          hdrl_value scalar_operator);

cpl_error_code hdrl_spectrum1D_mul_scalar(hdrl_spectrum1D * self,
                                          hdrl_value scalar_operator);

cpl_error_code hdrl_spectrum1D_add_scalar(hdrl_spectrum1D * self,
                                          hdrl_value scalar_operator);

cpl_error_code hdrl_spectrum1D_sub_scalar(hdrl_spectrum1D * self,
                                          hdrl_value scalar_operator);

cpl_error_code hdrl_spectrum1D_pow_scalar(hdrl_spectrum1D * self,
                                          hdrl_value scalar_operator);

cpl_error_code hdrl_spectrum1D_exp_scalar(hdrl_spectrum1D * self,
                                          hdrl_value scalar_operator);

/*-----------------------------------------------------------------------------
                            Functions - Wavelength operations
 -----------------------------------------------------------------------------*/

cpl_error_code hdrl_spectrum1D_wavelength_mult_scalar_linear(hdrl_spectrum1D * self,
                                                   hdrl_data_t scale_linear);

hdrl_spectrum1D *
hdrl_spectrum1D_wavelength_mult_scalar_linear_create(const hdrl_spectrum1D * self,
                                                   hdrl_data_t scale_linear);


cpl_error_code hdrl_spectrum1D_wavelength_shift(hdrl_spectrum1D * self,
                                      hdrl_data_t shift);

hdrl_spectrum1D *
hdrl_spectrum1D_wavelength_shift_create(const hdrl_spectrum1D * self,
                                      hdrl_data_t shift);


cpl_error_code hdrl_spectrum1D_wavelength_convert_to_linear(hdrl_spectrum1D * self);

hdrl_spectrum1D *
hdrl_spectrum1D_wavelength_convert_to_linear_create(const hdrl_spectrum1D * self);

cpl_error_code hdrl_spectrum1D_wavelength_convert_to_log(hdrl_spectrum1D * self);


hdrl_spectrum1D *
hdrl_spectrum1D_wavelength_convert_to_log_create(const hdrl_spectrum1D * self);

/*-----------------------------------------------------------------------------
                            Functions - Selectors
 -----------------------------------------------------------------------------*/

hdrl_spectrum1D *
hdrl_spectrum1D_select_wavelengths(const hdrl_spectrum1D * self,
        const cpl_bivector * windows, const cpl_boolean is_internal);

hdrl_spectrum1D *
hdrl_spectrum1D_reject_pixels(const hdrl_spectrum1D * self,
        const cpl_array * bad_samples);


/*-----------------------------------------------------------------------------
                            Functions - Table conversions
 -----------------------------------------------------------------------------*/

cpl_table * hdrl_spectrum1D_convert_to_table
(const hdrl_spectrum1D * self, const char * flux_col_name,
const char* wavelength_col_name, const char * flux_e_col_name,
const char * flux_bpm_col_name);

hdrl_spectrum1D * hdrl_spectrum1D_convert_from_table
(const cpl_table * self, const char * flux_col_name,
const char* wavelength_col_name, const char * flux_e_col_name,
const char * flux_bpm_col_name, hdrl_spectrum1D_wave_scale scale);

cpl_error_code hdrl_spectrum1D_append_to_table
(const hdrl_spectrum1D * self, cpl_table * dest,
const char * flux_col_name, const char* wavelength_col_name,
const char * flux_e_col_name, const char * flux_bpm_col_name);


CPL_END_DECLS

#endif 
