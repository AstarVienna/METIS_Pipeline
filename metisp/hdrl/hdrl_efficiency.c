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

#include <hdrl_efficiency.h>

#include "hdrl_spectrum.h"
#include "hdrl_spectrum_resample.h"
#include "hdrl_parameter.h"
#include <math.h>

/*-----------------------------------------------------------------------------
                     Private Functions and Data Structures
 -----------------------------------------------------------------------------*/

/*Parameter used for the efficiency calculation*/
typedef struct {
    HDRL_PARAMETER_HEAD;
    hdrl_value Ap;
    hdrl_value Am;
    hdrl_value G;
    hdrl_value Tex;
    hdrl_value Atel;
} hdrl_efficiency_parameters;

static hdrl_parameter_typeobj
hdrl_efficiency_parameters_type = {
        HDRL_PARAMETER_EFFICIENCY,                  /* type */
    (hdrl_alloc *)&cpl_malloc,                                 /* fp_alloc */
    (hdrl_free *)&cpl_free,                                    /* fp_free */
    NULL,                                                      /* fp_destroy */
    sizeof(hdrl_efficiency_parameters),             /* obj_size */
};

static inline cpl_error_code
hdrl_efficiency_parameter_check(const hdrl_parameter * pars);

static inline hdrl_value
hdrl_efficiency_parameter_get_Am(const hdrl_parameter * pars);
static inline hdrl_value
hdrl_efficiency_parameter_get_Ap(const hdrl_parameter * pars);
static inline hdrl_value
hdrl_efficiency_parameter_get_G(const hdrl_parameter * pars);
static inline hdrl_value
hdrl_efficiency_parameter_get_Tex(const hdrl_parameter * pars);
static inline hdrl_value
hdrl_efficiency_parameter_get_Atel(const hdrl_parameter * pars);

static inline hdrl_spectrum1D *
select_obs_spectrum(const hdrl_spectrum1D * I_std,
                      const hdrl_spectrum1D * I_std_ref,
                      const hdrl_spectrum1D * E_x);

static inline hdrl_data_t
lowest_w_max(const cpl_array * a1, const cpl_array * a2);

static inline hdrl_data_t
highest_w_min(const cpl_array * a1, const cpl_array * a2);

/**
 * @addtogroup hdrl_efficiency
 * @{
 */
/*-----------------------------------------------------------------------------
                                   Functions
 -----------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief ctor for the hdrl_parameter for response
 * @param Ap     Parameter to indicate if the efficiency is computed at
 *               airmass = 0, or at a given non zero value
 * @param Am     Airmass at which the std star was observed
 * @param G      Gain [ADU/e]
 * @param Tex    Exposure time [s]
 *
 * @return hdrl_parameter
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter* hdrl_response_parameter_create(
        const hdrl_value Ap, const hdrl_value Am, const hdrl_value G,
        const hdrl_value Tex){

    hdrl_efficiency_parameters * p
    = (hdrl_efficiency_parameters *)
       hdrl_parameter_new(&hdrl_efficiency_parameters_type);

    p->Am = Am;
    p->Ap = Ap;
    p->G = G;
    p->Tex = Tex;
    p->Atel = (hdrl_value){0.0, 0.0};
    return (hdrl_parameter*) p;
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief ctor for the hdrl_parameter for efficiency
 * @param Ap     Parameter to indicate if the efficiency is computed at
 *               airmass = 0, or at a given non zero value
 * @param Am     Airmass at which the std star was observed
 * @param G      Gain [e/ADU]
 * @param Tex    Exposure time [s]
 * @param Atel   Collecting area of the telescope [cm2]
 *
 * @return hdrl_parameter
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter* hdrl_efficiency_parameter_create(
        const hdrl_value Ap, const hdrl_value Am, const hdrl_value G,
        const hdrl_value Tex, const hdrl_value Atel){

    hdrl_efficiency_parameters * p
    = (hdrl_efficiency_parameters *)
       hdrl_parameter_new(&hdrl_efficiency_parameters_type);

    p->Am = Am;
    p->Ap = Ap;
    p->G = G;
    p->Tex = Tex;
    p->Atel = Atel;
    return (hdrl_parameter*) p;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief core response calculation
 * @param I_std_arg     std star observed spectrum, wavelength in [nm]
 * @param I_std_ref     std start model spectrum, wavelength in [nm]
 * @param E_x           atm. extinction model spectrum, wavelength in [nm]
 * @param pars          parameters, see the constructor for
 *                      hdrl_efficiency_parameter_create
 * @return response, NULL in case of error
 *
 * This function implements the efficiency calculation. The formula used is:
 *
 *             	  I_std_ref(l)* G * Tex * 10^(0.4 * (Ap-Am) * E_x(l))
 *  Res(l)= -----------------------------------------------------------
 *                           		I_std(l)
 *
 * Where I_std_ref and E_x spectra are resampled by this function to match
 * the wavelengths where I_std is defined on. If E_x and I_std_ref already match
 * the wavelengths of I_std resampling is not executed. If E_x or I_std_ref do not
 * completely cover the wavelength interval where I_std is defined, I_std is
 * truncated to avoid extrapolation of the models.
 * For the other parameters see hdrl_response_parameter_create().
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: if any of the spectra or parameter are NULL
 * - For other errors see hdrl_spectrum1D_resample(), and arithmetic functions
 *   for spectrum processing (e.g. hdrl_spectrum1D_div_spectrum_create())
 */
/* ---------------------------------------------------------------------------*/


hdrl_spectrum1D *
hdrl_response_core_compute(
const hdrl_spectrum1D * I_std_arg,
const hdrl_spectrum1D * I_std_ref,
const hdrl_spectrum1D * E_x,
const hdrl_parameter * pars){

    cpl_ensure(I_std_arg != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(I_std_ref != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(E_x != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(pars != NULL, CPL_ERROR_NULL_INPUT, NULL);

    const hdrl_value Ap = hdrl_efficiency_parameter_get_Ap(pars);
    const hdrl_value Am = hdrl_efficiency_parameter_get_Am(pars);
    const hdrl_value G = hdrl_efficiency_parameter_get_G(pars);
    const hdrl_value Tex = hdrl_efficiency_parameter_get_Tex(pars);

    cpl_ensure(cpl_error_get_code() == CPL_ERROR_NONE,
    		CPL_ERROR_ILLEGAL_OUTPUT, NULL);

    hdrl_spectrum1D * I_std = select_obs_spectrum(I_std_arg, I_std_ref, E_x);

    cpl_ensure(I_std != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

    const hdrl_spectrum1D_wavelength spec_wav =
                    hdrl_spectrum1D_get_wavelength(I_std);

    hdrl_parameter *params = hdrl_spectrum1D_resample_interpolate_parameter_create(
    							hdrl_spectrum1D_interp_akima);
    hdrl_spectrum1D *exponential = hdrl_spectrum1D_resample(E_x, &spec_wav, params);
    hdrl_parameter_delete(params);
    cpl_ensure(exponential != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

    params = hdrl_spectrum1D_resample_interpolate_parameter_create(
    							hdrl_spectrum1D_interp_akima);
    hdrl_spectrum1D * I_std_ref_resampled = hdrl_spectrum1D_resample(I_std_ref,
    							&spec_wav, params);
    hdrl_parameter_delete(params);
    cpl_ensure(I_std_ref_resampled != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);


    /* exponent of 10.0*/
    {
		hdrl_spectrum1D * exponential2 = hdrl_spectrum1D_duplicate(exponential);

		/*0.4A_pE_x(f) */
		hdrl_spectrum1D_mul_scalar(exponential,	(hdrl_value){0.4, 0.0});
		hdrl_spectrum1D_mul_scalar(exponential, Ap);

		/*0.4A_mE_x(f) */
		hdrl_spectrum1D_mul_scalar(exponential2, (hdrl_value){0.4, 0.0});
		hdrl_spectrum1D_mul_scalar(exponential2, Am);

		/*0.4A_pE_x(f) - 0.4A_mE_x(f) */
		hdrl_spectrum1D_sub_spectrum(exponential, exponential2);

		hdrl_spectrum1D_delete(&exponential2);
    }

    hdrl_spectrum1D_exp_scalar(exponential, (hdrl_value){10.0, 0.0});
    hdrl_spectrum1D_mul_scalar(exponential, G);
    hdrl_spectrum1D_mul_spectrum(exponential, I_std_ref_resampled);
    hdrl_spectrum1D_mul_scalar(exponential, Tex);
    hdrl_spectrum1D_div_spectrum(exponential, I_std);

    hdrl_spectrum1D_delete(&I_std_ref_resampled);
    hdrl_spectrum1D_delete(&I_std);

    return exponential;
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief efficiency calculation
 * @param I_std_arg     std star observed spectrum, wavelength in [nm]
 * @param I_std_ref     std start model spectrum, wavelength in [nm]
 * @param E_x           atm. extinction model spectrum, wavelength in [nm]
 * @param pars          parameters, see the constructor for
 *                      hdrl_spectrum1D_efficiency_parameter_create
 * @return efficiency, NULL in case of error
 *
 * This function implements the efficiency calculation. The formula used is:
 *
 *             I_std(l) * 10^(0.4 * E_x(l) * (Am - Ap)) * G * E_phot(l)
 *  Eff(l)= -----------------------------------------------------------
 *                          Tex * Atel * I_std_ref(l)
 *
 * Where I_std_ref and E_x spectra are resampled by this function to match
 * the wavelengths where I_std is defined on. If E_x and I_std_ref already match
 * the wavelengths of I_std resampling is not executed.
 * E_phot is the energy of one photon. For the other parameters see
 * hdrl_spectrum1D_efficiency_parameter_create(). If E_x or I_std_ref do not
 * completely cover the wavelength interval where I_std is defined, I_std is
 * truncated to avoid extrapolation of the models.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: if any of the spectra or parameter are NULL
 * - For other errors see hdrl_spectrum1D_resample(), and arithmetic functions
 *   for spectrum processing (e.g. hdrl_spectrum1D_div_spectrum_create())
 */
/* ---------------------------------------------------------------------------*/

hdrl_spectrum1D * hdrl_efficiency_compute(
	const hdrl_spectrum1D * I_std_arg,
	const hdrl_spectrum1D * I_std_ref,
	const hdrl_spectrum1D * E_x,
	const hdrl_parameter * pars)
{

    cpl_ensure(I_std_arg != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(I_std_ref != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(E_x       != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(pars      != NULL, CPL_ERROR_NULL_INPUT, NULL);

    const hdrl_value Ap   = hdrl_efficiency_parameter_get_Ap(pars);
    const hdrl_value Am   = hdrl_efficiency_parameter_get_Am(pars);
    const hdrl_value G    = hdrl_efficiency_parameter_get_G(pars);
    const hdrl_value Tex  = hdrl_efficiency_parameter_get_Tex(pars);
    const hdrl_value Atel = hdrl_efficiency_parameter_get_Atel(pars);

    cpl_ensure(cpl_error_get_code() == CPL_ERROR_NONE, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

    hdrl_spectrum1D * I_std = select_obs_spectrum(I_std_arg, I_std_ref, E_x);
    cpl_ensure(I_std != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

    const hdrl_spectrum1D_wavelength spec_wav = hdrl_spectrum1D_get_wavelength(I_std);

    hdrl_parameter * params = hdrl_spectrum1D_resample_interpolate_parameter_create(
            									hdrl_spectrum1D_interp_akima);
    hdrl_spectrum1D * exponential = hdrl_spectrum1D_resample(E_x,
    											&spec_wav, params);
    hdrl_parameter_delete(params);
    cpl_ensure(exponential != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

    params = hdrl_spectrum1D_resample_interpolate_parameter_create(
    											hdrl_spectrum1D_interp_akima);
    hdrl_spectrum1D * I_std_ref_resampled = hdrl_spectrum1D_resample(I_std_ref,
    											&spec_wav, params);
    hdrl_parameter_delete(params);
    cpl_ensure(I_std_ref_resampled != NULL, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

    /* exponent of 10.0*/
    {
        hdrl_spectrum1D * exponential2 = hdrl_spectrum1D_duplicate(exponential);

        /*0.4A_mE_x(f) */
        hdrl_spectrum1D_mul_scalar(exponential, (hdrl_value){0.4, 0.0});
        hdrl_spectrum1D_mul_scalar(exponential, Am);
        /*0.4A_pE_x(f) */
        hdrl_spectrum1D_mul_scalar(exponential2, (hdrl_value){0.4, 0.0});
        hdrl_spectrum1D_mul_scalar(exponential2, Ap);

        /*0.4A_mE_x(f) - 0.4A_pE_x(f) */
        hdrl_spectrum1D_sub_spectrum(exponential, exponential2);

        hdrl_spectrum1D_delete(&exponential2);
    }

    hdrl_spectrum1D * eph_spec = hdrl_spectrum1D_create_analytic(
    								E_ph, spec_wav.wavelength, spec_wav.scale);

    hdrl_spectrum1D_exp_scalar(exponential, (hdrl_value){10.0, 0.0});
    hdrl_spectrum1D_mul_scalar(exponential, G);
    hdrl_spectrum1D_mul_spectrum(exponential, I_std);
    hdrl_spectrum1D_mul_spectrum(exponential, eph_spec);
    hdrl_spectrum1D_div_scalar(exponential, Tex);
    hdrl_spectrum1D_div_scalar(exponential, Atel);
    hdrl_spectrum1D_div_spectrum(exponential, I_std_ref_resampled);

    hdrl_spectrum1D_delete(&eph_spec);
    hdrl_spectrum1D_delete(&I_std_ref_resampled);
    hdrl_spectrum1D_delete(&I_std);
    return exponential;

}

/* ---------------------------------------------------------------------------*/
/**
 * @brief energy of the photon calculation
 * @param lambda  wavelength, in nm
 * @return energy of the photon for the given wavelength
 */
/* ---------------------------------------------------------------------------*/
hdrl_value E_ph(hdrl_data_t lambda){
    const double nm2um = 0.001;
    const double hc = 1.e7*1.986e-19 / nm2um;

    return (hdrl_value){ hc / lambda, 0.0};
}

/*-----------------------------------------------------------------------------
                     Private Functions Implementation
 -----------------------------------------------------------------------------*/
/* check that hdrl_parameter is an compatible with the efficiency computation
 * routine.*/
static inline cpl_error_code
hdrl_efficiency_parameter_check(const hdrl_parameter * pars){


    cpl_ensure_code(pars != NULL, CPL_ERROR_NULL_INPUT);

    cpl_boolean is_compatible = hdrl_parameter_get_parameter_enum(pars) ==
            HDRL_PARAMETER_EFFICIENCY;
    cpl_ensure_code(is_compatible , CPL_ERROR_INCOMPATIBLE_INPUT);

    return CPL_ERROR_NONE;
}

static inline hdrl_value
hdrl_efficiency_parameter_get_Am(const hdrl_parameter * pars){

    if(hdrl_efficiency_parameter_check(pars))
        return (hdrl_value){0.0,0.0};

    const hdrl_efficiency_parameters * p
    = (const hdrl_efficiency_parameters*)pars;

    return p->Am;
}

static inline hdrl_value
hdrl_efficiency_parameter_get_Ap(const hdrl_parameter * pars){

    if(hdrl_efficiency_parameter_check(pars))
        return (hdrl_value){0.0,0.0};

    const hdrl_efficiency_parameters * p
    = (const hdrl_efficiency_parameters*)pars;

    return p->Ap;
}

static inline hdrl_value
hdrl_efficiency_parameter_get_G(const hdrl_parameter * pars){

    if(hdrl_efficiency_parameter_check(pars))
        return (hdrl_value){0.0,0.0};

    const hdrl_efficiency_parameters * p
    = (const hdrl_efficiency_parameters*)pars;

    return p->G;
}

static inline hdrl_value
hdrl_efficiency_parameter_get_Tex(const hdrl_parameter * pars){

    if(hdrl_efficiency_parameter_check(pars))
        return (hdrl_value){0.0,0.0};

    const hdrl_efficiency_parameters * p
    = (const hdrl_efficiency_parameters*)pars;

    return p->Tex;
}

static inline hdrl_value
hdrl_efficiency_parameter_get_Atel(const hdrl_parameter * pars){

    if(hdrl_efficiency_parameter_check(pars))
        return (hdrl_value){0.0,0.0};

    const hdrl_efficiency_parameters * p
    = (const hdrl_efficiency_parameters*)pars;

    return p->Atel;
}
/*get the maximum between the two minimum values of a1 and a2*/
static inline hdrl_data_t
highest_w_min(const cpl_array * a1, const cpl_array * a2){
    const hdrl_data_t w1 = cpl_array_get_min(a1);
    const hdrl_data_t w2 = cpl_array_get_min(a2);

    return CPL_MAX(w2, w1);
}

/*get the minimum between the two maximum values of a1 and a2*/
static inline hdrl_data_t
lowest_w_max(const cpl_array * a1, const cpl_array * a2){
    const hdrl_data_t w1 = cpl_array_get_max(a1);
    const hdrl_data_t w2 = cpl_array_get_max(a2);

    return CPL_MIN(w2, w1);
}

/*Removes lines inside I_std whose wavelengths are not contained inside I_std_ref
 * or E_x*/
static inline hdrl_spectrum1D *
select_obs_spectrum(const hdrl_spectrum1D * I_std,
                      const hdrl_spectrum1D * I_std_ref,
                      const hdrl_spectrum1D * E_x){

    const cpl_array * w_std_ref =
            hdrl_spectrum1D_get_wavelength(I_std_ref).wavelength;

    const cpl_array * E_x_ref =
            hdrl_spectrum1D_get_wavelength(E_x).wavelength;

    const hdrl_data_t w_min = highest_w_min(w_std_ref, E_x_ref);
    const hdrl_data_t w_max = lowest_w_max(w_std_ref, E_x_ref);

    cpl_ensure(w_min < w_max, CPL_ERROR_ILLEGAL_INPUT, NULL);

    cpl_bivector * wavs = cpl_bivector_new(1);
    cpl_vector_set(cpl_bivector_get_x(wavs), 0, w_min);
    cpl_vector_set(cpl_bivector_get_y(wavs), 0, w_max);

    hdrl_spectrum1D * to_ret = hdrl_spectrum1D_select_wavelengths(I_std,
            wavs, CPL_TRUE);

    cpl_bivector_delete(wavs);

    return to_ret;
}

/**@}*/
