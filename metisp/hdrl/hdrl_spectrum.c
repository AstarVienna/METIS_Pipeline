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
#include "hdrl_spectrum.h"

#include "hdrl_DER_SNR.h"
#include "hdrl_utils.h"

#include <math.h>

/**
 *
 * @defgroup hdrl_spectrum1D Spectrum 1D
 *
 * @brief
 *   This module defines the hdrl_spectrum1D data structure,
 *   provides basic functionalities for it (constructors, destructor, operators).
 */
/*----------------------------------------------------------------------------*/

/**@{*/
/*-----------------------------------------------------------------------------
                                   Private Functions
 -----------------------------------------------------------------------------*/
typedef cpl_error_code (* operate_image_mutable) (hdrl_image * self,
                                                  const hdrl_image * other);

typedef cpl_error_code (* operate_image_scalar_mutable) (hdrl_image * self,
                                                         hdrl_value scalar);

static inline cpl_error_code
operate_spectra_flux_mutate(hdrl_spectrum1D * self,
                            const hdrl_spectrum1D * other,
                            operate_image_mutable func);

static inline hdrl_spectrum1D *
operate_spectra_flux_create(const hdrl_spectrum1D * self,
                            const hdrl_spectrum1D * other,
                            operate_image_mutable func);

static inline cpl_error_code
operate_spectra_scalar_flux_mutate(hdrl_spectrum1D * self,
                                   hdrl_value scalar,
                                   operate_image_scalar_mutable func);

static inline hdrl_spectrum1D *
operate_spectra_scalar_flux_create(const hdrl_spectrum1D * self,
                                   hdrl_value scalar,
                                   operate_image_scalar_mutable func);

static inline
hdrl_spectrum1D * hdrl_spectrum1D_wrap(hdrl_image * arg_flux,
                                       cpl_array * wavelength,
                                       hdrl_spectrum1D_wave_scale scale);

static inline int compare_double(const void * a, const void * b);

static inline cpl_boolean is_uniformly_sampled(const double * v, cpl_size sz,
        double * bin);


static inline cpl_boolean
is_wlen_selected(const cpl_bivector * windows, const cpl_boolean is_internal,
        const hdrl_data_t w);

/*-----------------------------------------------------------------------------
                                   Functions
 -----------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief hdrl_spectrum1D default constructor
 * @param arg_flux       flux with bpm
 * @param arg_flux_e     error for the flux
 * @param wavelength     wavelengths
 * @param wave_scale     the scale of the wavelengths of the spectrum (logarithmic
 *                       or linear)
 * @return hdrl_spectrum1D or NULL on error
 *
 * The constructor allocates memory of a hdrl_spectrum1D structure. The
 * cpl_image(s) and the cpl_array are copied inside the newly allocated data
 * structure. The bpm of arg_flux becomes the bpm of the spectrum.
 *
 * @note providing a sorted, strictly monotonic increasing wavelength array will
 * improve performance of DER_SNR calculation and resampling since no sorting will
 * be required.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: if any of the images or array are NULL
 * - CPL_ERROR_INCOMPATIBLE_INPUT: if the sizes of images and array do not match,
 *   or if the height of the image is not 1.
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D * hdrl_spectrum1D_create(const cpl_image * arg_flux,
                                         const cpl_image * arg_flux_e,
                                         const cpl_array * wavelength,
                                         hdrl_spectrum1D_wave_scale wave_scale){

    cpl_ensure(arg_flux != NULL && wavelength != NULL && arg_flux_e != NULL,
    CPL_ERROR_NULL_INPUT, NULL);

    cpl_ensure(cpl_image_get_size_y(arg_flux) == 1
            && cpl_image_get_size_y(arg_flux_e) == 1,
    CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    cpl_ensure(cpl_image_get_size_x(arg_flux) == cpl_array_get_size(wavelength)
          && cpl_image_get_size_x(arg_flux_e) == cpl_array_get_size(wavelength),
          CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    cpl_image * flux_error = cpl_image_cast(arg_flux_e, HDRL_TYPE_ERROR);
    cpl_image * flux = cpl_image_cast(arg_flux, HDRL_TYPE_DATA);

    hdrl_image * flux_img = hdrl_image_wrap(flux, flux_error, NULL, CPL_TRUE);
    cpl_array * lambda = cpl_array_cast(wavelength, HDRL_TYPE_DATA);

    return hdrl_spectrum1D_wrap(flux_img, lambda, wave_scale);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief hdrl_spectrum1D constructor in the case of a spectrum defined by an
 * analytical function
 * @param func          analytical function defining the spectrum
 * @param wavelength    frequencies the flux is calculated on
 * @param scale         the scale of the spectrum (logarithmic or linear)
 * @return hdrl_spectrum1D or NULL on error
 *
 * The constructor allocates memory of a hdrl_spectrum1D structure.
 * The cpl_array is copied inside the newly allocated data structure.
 * For every wavelength inside the CPL array flux and error are calculated using
 * func.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: if any among the func or the array are NULL.
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D * hdrl_spectrum1D_create_analytic(
                                         calculate_analytic_spectrum_point func,
                                         const cpl_array * wavelength,
                                         hdrl_spectrum1D_wave_scale scale){

    cpl_ensure(wavelength != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(func != NULL, CPL_ERROR_NULL_INPUT, NULL);

    cpl_size sx = cpl_array_get_size(wavelength);
    cpl_image * flux = cpl_image_new(sx, 1, HDRL_TYPE_DATA);
    cpl_image * flux_e = cpl_image_new(sx, 1, HDRL_TYPE_ERROR);

    for(cpl_size i = 0; i < sx; ++i){
        hdrl_data_t lambda = cpl_array_get(wavelength, i, NULL);
        hdrl_value v = func(lambda);
        cpl_image_set(flux, i + 1, 1, v.data);
        cpl_image_set(flux_e, i + 1, 1, v.error);
    }

    hdrl_spectrum1D * to_ret =
            hdrl_spectrum1D_create(flux, flux_e, wavelength, scale);

    cpl_image_delete(flux);
    cpl_image_delete(flux_e);

    return to_ret;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief hdrl_spectrum1D constructor in the case of error-free spectrum
 * (i.e. the error on the flux is zero for every wavelengths).
 * @param arg_flux      flux with bpm
 * @param wavelength    frequencies
 * @param scale         the scale of the spectrum (logarithmic or linear)
 * @return hdrl_spectrum1D or NULL on error
 *
 * The constructor allocates memory of a hdrl_spectrum1D structure.
 * The cpl_array and the cpl_image are copied inside the newly allocated data
 * structure. The flux error is considered to be zero.
 *
 * Possible cpl-error-code set in this function: see hdrl_spectrum1D_create()
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D * hdrl_spectrum1D_create_error_free
                                      (const cpl_image * arg_flux,
                                      const cpl_array * wavelength,
                                      hdrl_spectrum1D_wave_scale scale){

    cpl_ensure(arg_flux != NULL, CPL_ERROR_NULL_INPUT, NULL);

    cpl_size sx = cpl_image_get_size_x(arg_flux);
    cpl_size sy = cpl_image_get_size_y(arg_flux);

    cpl_ensure(sy == 1 && sx > 0, CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    cpl_image * zero_errors = cpl_image_new(sx, sy, HDRL_TYPE_ERROR);
    cpl_image_fill_window(zero_errors, 1, 1, sx, sy, 0.0);

    hdrl_spectrum1D * to_ret = hdrl_spectrum1D_create
            (arg_flux, zero_errors, wavelength, scale);

    cpl_image_delete(zero_errors);

    return to_ret;

}

/* ---------------------------------------------------------------------------*/
/**
 * @brief hdrl_spectrum1D constructor when no error information is available, in
 * this case we use DER_SNR to esimate the error. Please refer to the
 * documentation of the function estimate_noise_DER_SNR().
 * @param arg_flux    flux with bpm
 * @param half_window half window the DER_SNR is calculated on
 * @param wavelength  frequencies
 * @param scale       the scale of the spectrum (logarithmic or linear)
 * @return hdrl_spectrum1D or NULL on error
 *
 * The constructor allocates memory of a hdrl_spectrum1D structure.
 * The cpl_array and the cpl_image are copied inside the newly allocated data
 * structure. The flux error is calculated creating a window of
 * 2 * half_window + 1 pixels around each flux pixel and then using the noise
 * estimation used for DER_SNR calculation. The use of DER_SNR can increase the
 * number of bad pixels, in the case of a good pixel surrounded by bad pixels.
 * See estimate_noise_DER_SNR() for more details.
 *
 * Possible cpl-error-code set in this function: see hdrl_spectrum1D_create()
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D * hdrl_spectrum1D_create_error_DER_SNR
                                      (const cpl_image * arg_flux,
                                      cpl_size half_window,
                                      const cpl_array * wavelength,
                                      hdrl_spectrum1D_wave_scale scale){
    cpl_ensure(arg_flux != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(wavelength != NULL, CPL_ERROR_NULL_INPUT, NULL);

    cpl_size sx = cpl_image_get_size_x(arg_flux);
    cpl_size sy = cpl_image_get_size_y(arg_flux);

    cpl_ensure(sy == 1 && sx > 0, CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    cpl_image * flux = cpl_image_cast(arg_flux, HDRL_TYPE_DATA);

    const hdrl_data_t * flux_data =
            (const hdrl_data_t*)cpl_image_get_data_const(flux);
    const cpl_mask * mask_in = cpl_image_get_bpm_const(flux);
    const cpl_binary * msk_in =
            mask_in == NULL ? NULL : cpl_mask_get_data_const(mask_in);

    cpl_image * DER_SNR_errors = estimate_noise_DER_SNR
            (flux_data, msk_in, wavelength, sx, half_window);

    if(!DER_SNR_errors || cpl_error_get_code() != CPL_ERROR_NONE){
        cpl_image_delete(flux);
        cpl_image_delete(DER_SNR_errors);
        return NULL;
    }

    /* DER_SNR_errors might contain more bad pixels than flux, e.g. 1 good pixel
     * in position i that is surrounded by bad pixels
    */
    cpl_mask * msk = cpl_image_unset_bpm(DER_SNR_errors);
    cpl_mask_delete(cpl_image_set_bpm(flux, msk));

    hdrl_spectrum1D * to_ret = hdrl_spectrum1D_create
            (flux , DER_SNR_errors, wavelength, scale);

    cpl_image_delete(DER_SNR_errors);
    cpl_image_delete(flux);

    return to_ret;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief hdrl_spectrum1D copy constructor
 * @param self spectrum that has to be duplicated
 * @return a copy of self, return NULL if self was NULL
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D * hdrl_spectrum1D_duplicate(const hdrl_spectrum1D* self)
{
    if(!self) return NULL;

    hdrl_image * flux = hdrl_image_duplicate(self->flux);
    cpl_array * lambdas = cpl_array_duplicate(self->wavelength);


    return hdrl_spectrum1D_wrap(flux, lambdas, self->wave_scale);
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief hdrl_spectrum1D destructor
 * @param p_self spectrum to delete.
 * @return nothing
 *
 * If p_self is NULL or *p_self is NULL nothing is done.
 */
/* ---------------------------------------------------------------------------*/
void hdrl_spectrum1D_delete(hdrl_spectrum1D ** p_self){

    if(!p_self) return;

    hdrl_spectrum1D * spectrum = *p_self;

    if(!spectrum) return;

    cpl_array_delete(spectrum->wavelength);
    hdrl_image_delete(spectrum->flux);
    cpl_free(spectrum);

    *p_self = NULL;
}

/**
 * @brief hdrl_spectrum1D getter for size
 * @param self the spectrum
 * @return the number of samples the 1D spectrum is made of
 */
/* ---------------------------------------------------------------------------*/
cpl_size hdrl_spectrum1D_get_size(const hdrl_spectrum1D * self){

    if(!self) return 0;

    return cpl_array_get_size(self->wavelength);
}

/**
 * @brief hdrl_spectrum1D getter flux
 * @param self the spectrum
 * @return hdrl image containing flux information (bpm, flux and error)
 */
/* ---------------------------------------------------------------------------*/
const hdrl_image*
hdrl_spectrum1D_get_flux(const hdrl_spectrum1D * self){

    if(!self) return NULL;

    return self->flux;
}

/**
 * @brief hdrl_spectrum1D getter for wavelengths
 * @param self the spectrum
 * @return the wavelengths the spectrum is defined on.
 * It returns also its scale and its bpm.
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D_wavelength
hdrl_spectrum1D_get_wavelength(const hdrl_spectrum1D * self){

    hdrl_spectrum1D_wavelength to_ret = {NULL, NULL,
            hdrl_spectrum1D_wave_scale_linear};

    cpl_ensure(self != NULL,
            CPL_ERROR_NULL_INPUT, to_ret);

    const hdrl_image * flux = hdrl_spectrum1D_get_flux(self);

    const cpl_array * lambdas = self->wavelength;

    const cpl_mask *
    mask = cpl_image_get_bpm_const(hdrl_image_get_image_const(flux));

    to_ret.bpm = mask;
    to_ret.wavelength = lambdas;
    to_ret.scale = hdrl_spectrum1D_get_scale(self);
    return to_ret;
}

/**
 * @brief hdrl_spectrum1D getter for scale
 * @param self the spectrum
 * @return the scale.
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D_wave_scale
hdrl_spectrum1D_get_scale(const hdrl_spectrum1D * self){
    cpl_ensure(self != NULL,
            CPL_ERROR_NULL_INPUT, hdrl_spectrum1D_wave_scale_linear);

    return self->wave_scale;
}

/**
 * @brief hdrl_spectrum1D getter for a flux value
 * @param self  the spectrum
 * @param idx   position of the sample. The samples are a 0-indexed sequence.
 * @param rej   output, set to 0 if the value is not a valid pixel.
 * @return the i-th pixel value.
 */
/* ---------------------------------------------------------------------------*/
hdrl_value
hdrl_spectrum1D_get_flux_value(const hdrl_spectrum1D * self, int idx, int * rej){

    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, ((hdrl_value){0.0, 0.0}));

    const hdrl_image * flx = hdrl_spectrum1D_get_flux(self);

    return hdrl_image_get_pixel(flx, idx + 1, 1, rej);
}

/**
 * @brief hdrl_spectrum1D getter for a wavelength value
 * @param self  the spectrum
 * @param idx   position of the sample. The samples are a 0-indexed sequence.
 * @param rej   output, set to 0 if the value is not a valid pixel.
 * @return the i-th wavelength value.
 */
/* ---------------------------------------------------------------------------*/
hdrl_data_t
hdrl_spectrum1D_get_wavelength_value(const hdrl_spectrum1D * self, int idx, int * rej){

    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, 0.0);

    const hdrl_spectrum1D_wavelength lambdas =
            hdrl_spectrum1D_get_wavelength(self);

    const cpl_array * wlengs = lambdas.wavelength;
    const hdrl_data_t to_ret = (hdrl_data_t) cpl_array_get(wlengs, idx, NULL);

    if(rej){
        const cpl_mask * msk = lambdas.bpm;
        if(msk)
            *rej = cpl_mask_get(msk, idx + 1, 1);
        else
            *rej = CPL_BINARY_0;
    }

    return to_ret;
}

/**
 * @brief divide one spectrum by another spectrum
 * @param num numerator
 * @param den denumerator
 * @return a newly allocated spectrum whose flux values are num/den,
 * with error propagation.
 *
 * Possible cpl-error-code set in this function: hdrl_image_div_image
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D *
hdrl_spectrum1D_div_spectrum_create(const hdrl_spectrum1D * num,
                                    const hdrl_spectrum1D * den){
    return operate_spectra_flux_create(num, den, hdrl_image_div_image);
}

/**
 * @brief multiply one spectrum by another spectrum
 * @param f1 factor 1
 * @param f2 factor 2
 * @return a newly allocated spectrum whose flux values are f1*f2,
 * with error propagation.
 *
 * Possible cpl-error-code set in this function: hdrl_image_mul_image
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D *
hdrl_spectrum1D_mul_spectrum_create(const hdrl_spectrum1D * f1,
                                     const hdrl_spectrum1D * f2){
    return operate_spectra_flux_create(f1, f2, hdrl_image_mul_image);
}

/**void
 * @brief sum one spectrum to another spectrum
 * @param f1 factor 1
 * @param f2 factor 2
 * @return a newly allocated spectrum whose flux values are f1+f2,
 * with error propagation.
 *
 * Possible cpl-error-code set in this function: hdrl_image_add_image
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D *
hdrl_spectrum1D_add_spectrum_create(const hdrl_spectrum1D * f1,
                                     const hdrl_spectrum1D * f2){
    return operate_spectra_flux_create(f1, f2, hdrl_image_add_image);
}


/**
 * @brief subtract two spectra
 * @param f1 factor 1
 * @param f2 factor 2
 * @return a newly allocated spectrum whose flux values are f1-f2,
 * with error propagation.
 *
 * Possible cpl-error-code set in this function: hdrl_image_sub_image
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D *
hdrl_spectrum1D_sub_spectrum_create(const hdrl_spectrum1D * f1,
                                     const hdrl_spectrum1D * f2){
    return operate_spectra_flux_create(f1, f2, hdrl_image_sub_image);
}


/**
 * @brief divide one spectrum by another spectrum
 * @param self  numerator, it is mutated to contain self/other
 * @param other denumerator
 * @return error code, for possible return codes see hdrl_image_div_image
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_spectrum1D_div_spectrum(hdrl_spectrum1D * self,
                                          const hdrl_spectrum1D * other){
    return operate_spectra_flux_mutate(self, other, hdrl_image_div_image);
}

/**
 * @brief multiply one spectrum by another spectrum
 * @param self  first factor, it is mutated to contain self*other
 * @param other second factor
 * @return error code, for possible return codes see hdrl_image_mul_image
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_spectrum1D_mul_spectrum(hdrl_spectrum1D * self,
                                          const hdrl_spectrum1D * other){
    return operate_spectra_flux_mutate(self, other, hdrl_image_mul_image);
}

/**
 * @brief sum two spectra
 * @param self  first factor, it is mutated to contain self+other
 * @param other second factor
 * @return error code, for possible return codes see hdrl_image_add_image
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_spectrum1D_add_spectrum(hdrl_spectrum1D * self,
                                          const hdrl_spectrum1D * other){
    return operate_spectra_flux_mutate(self, other, hdrl_image_add_image);
}


/**
 * @brief subtract two spectra
 * @param self  first factor, it is mutated to contain self-other
 * @param other second factor
 * @return error code, for possible return codes see hdrl_image_sub_image
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_spectrum1D_sub_spectrum(hdrl_spectrum1D * self,
                                          const hdrl_spectrum1D * other){
    return operate_spectra_flux_mutate(self, other, hdrl_image_sub_image);
}

/**
 * @brief divide a spectrum by a scalar
 * @param self      spectrum
 * @param scalar_operator  scalar factor
 * @return a newly created spectrum, whose flux is calculated dividing every
 * sample of self by scalar_operator. Returns NULL in case of error.
 *
 * Possible cpl-error-code set in this function: hdrl_image_div_scalar
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D *
hdrl_spectrum1D_div_scalar_create(const hdrl_spectrum1D * self,
                                   hdrl_value scalar_operator){
    return
    operate_spectra_scalar_flux_create(self, scalar_operator, hdrl_image_div_scalar);
}

/**
 * @brief multiply a spectrum by a scalar
 * @param self      spectrum
 * @param scalar_operator  scalar factor
 * @return a newly created spectrum, whose flux is calculated multiplying every
 * sample of self by scalar_operator. Returns NULL in case of error.
 *
 * Possible cpl-error-code set in this function: hdrl_image_mul_scalar
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D *
hdrl_spectrum1D_mul_scalar_create(const hdrl_spectrum1D * self,
                                   hdrl_value scalar_operator){
    return
    operate_spectra_scalar_flux_create(self, scalar_operator, hdrl_image_mul_scalar);
}

/**
 * @brief add a scalar to a spectrum
 * @param self      spectrum
 * @param scalar_operator  scalar factor
 * @return a newly created spectrum, whose flux is calculated adding every
 * sample of self to scalar_operator. Returns NULL in case of error.
 *
 * Possible cpl-error-code set in this function: hdrl_image_add_scalar
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D *
hdrl_spectrum1D_add_scalar_create(const hdrl_spectrum1D * self,
                                   hdrl_value scalar_operator){
    return
    operate_spectra_scalar_flux_create(self, scalar_operator, hdrl_image_add_scalar);
}

/**
 * @brief subtract a scalar from a spectrum
 * @param self      spectrum
 * @param scalar_operator  scalar factor
 * @return a newly created spectrum, whose flux is calculated subtracting from
 * every sample of self the value of scalar_operator. Returns NULL in case of error.
 *
 * Possible cpl-error-code set in this function: hdrl_image_sub_scalar
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D *
hdrl_spectrum1D_sub_scalar_create(const hdrl_spectrum1D * self,
                                   hdrl_value scalar_operator){
    return
    operate_spectra_scalar_flux_create(self, scalar_operator, hdrl_image_sub_scalar);
}

/**
 * @brief subtract a scalar from a spectrum
 * @param self      spectrum
 * @param scalar_operator  scalar factor
 * @return a newly created spectrum, whose flux is calculated elevating
 * every sample to the value of scalar_operator. Returns NULL in case of error.
 *
 * Possible cpl-error-code set in this function: hdrl_image_pow_scalar
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D *
hdrl_spectrum1D_pow_scalar_create(const hdrl_spectrum1D * self,
                                   hdrl_value scalar_operator){
    return
    operate_spectra_scalar_flux_create(self, scalar_operator, hdrl_image_pow_scalar);
}

/**
 * @brief subtract a scalar from a spectrum
 * @param self      spectrum
 * @param scalar_operator  scalar factor
 * @return a newly created spectrum, whose flux is calculated elevating
 * the value of scalar_operator to the samples. Returns NULL in case of error.
 *
 * Possible cpl-error-code set in this function: hdrl_image_exp_scalar
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D *
hdrl_spectrum1D_exp_scalar_create(const hdrl_spectrum1D * self,
                                   hdrl_value scalar_operator){
    return
    operate_spectra_scalar_flux_create(self, scalar_operator, hdrl_image_exp_scalar);
}

/**
 * @brief computes the elementwise division of a spectrum by a scalar,
 * the self parameter is modified
 * @param self      spectrum
 * @param scalar_operator  scalar factor
 * @return error code, for possible return codes see hdrl_image_div_scalar
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_spectrum1D_div_scalar(hdrl_spectrum1D * self,
                                           hdrl_value scalar_operator){
    return
    operate_spectra_scalar_flux_mutate(self, scalar_operator, hdrl_image_div_scalar);
}


/**
 * @brief computes the elementwise multiplication of a spectrum by a scalar,
 * the self parameter is modified
 * @param self      spectrum
 * @param scalar_operator  scalar factor
 * @return error code, for possible return codes see hdrl_image_mul_scalar
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_spectrum1D_mul_scalar(hdrl_spectrum1D * self,
                                           hdrl_value scalar_operator){
    return
    operate_spectra_scalar_flux_mutate(self, scalar_operator, hdrl_image_mul_scalar);
}

/**
 * @brief computes the elementwise addition of a spectrum by a scalar,
 * the self parameter is modified
 * @param self      spectrum
 * @param scalar_operator  scalar factor
 * @return error code, for possible return codes see hdrl_image_add_scalar
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_spectrum1D_add_scalar(hdrl_spectrum1D * self,
                                           hdrl_value scalar_operator){
    return
    operate_spectra_scalar_flux_mutate(self, scalar_operator, hdrl_image_add_scalar);
}

/**
 * @brief computes the elementwise subtraction of a spectrum by a scalar,
 * the self parameter is modified
 * @param self      spectrum
 * @param scalar_operator  scalar factor
 * @return error code, for possible return codes see hdrl_image_sub_scalar
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_spectrum1D_sub_scalar(hdrl_spectrum1D * self,
                                           hdrl_value scalar_operator){
    return
    operate_spectra_scalar_flux_mutate(self, scalar_operator, hdrl_image_sub_scalar);
}

/**
 * @brief computes the elementwise power of the flux to the scalar,
 * the self parameter is modified
 * @param self      spectrum
 * @param scalar_operator  scalar factor
 * @return error code, for possible return codes see hdrl_image_pow_scalar
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_spectrum1D_pow_scalar(hdrl_spectrum1D * self,
                                          hdrl_value scalar_operator){
    return
    operate_spectra_scalar_flux_mutate(self, scalar_operator, hdrl_image_pow_scalar);
}

/**
 * @brief computes the elementwise power of the scalar to the flux,
 * the self parameter is modified
 * @param self      spectrum
 * @param scalar_operator  scalar factor
 * @return error code, for possible return codes see hdrl_image_exp_scalar
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_spectrum1D_exp_scalar(hdrl_spectrum1D * self,
                                          hdrl_value scalar_operator){
    return
    operate_spectra_scalar_flux_mutate(self, scalar_operator, hdrl_image_exp_scalar);
}

/**
 * @brief computes the elementwise multiplication of the scalar for the
 * wavelength. The scalar is assumed to be expressed in linear units.
 * the self parameter is modified
 * @param self          spectrum
 * @param scale_linear  scalar factor
 * @return error code, for possible return codes see cpl_array_multiply_scalar
 * and cpl_array_add_scalar.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_spectrum1D_wavelength_mult_scalar_linear(hdrl_spectrum1D * self,
                                                   hdrl_data_t scale_linear)
{
    cpl_ensure_code(scale_linear > 0, CPL_ERROR_INCOMPATIBLE_INPUT);

    if(self == NULL) return CPL_ERROR_NONE;

    if(self->wave_scale == hdrl_spectrum1D_wave_scale_linear)
        return cpl_array_multiply_scalar(self->wavelength, scale_linear);

    return cpl_array_add_scalar(self->wavelength, log(scale_linear));
}

/**
 * @brief computes the elementwise multiplication of the scalar for the
 * wavelength. The scalar is assumed to be expressed in linear units. Self is
 * unchanged and the function returns a newly allocated spectrum.
 * @param self          spectrum
 * @param scale_linear  scalar factor
 * @return the modified spectrum
 *
 * Possible cpl-error-code set in this function: see
 * cpl_array_multiply_scalar and cpl_array_add_scalar.
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D *
hdrl_spectrum1D_wavelength_mult_scalar_linear_create(const hdrl_spectrum1D * self,
                                                   hdrl_data_t scale_linear)
{
    if(!self) return NULL;

    hdrl_spectrum1D * to_ret = hdrl_spectrum1D_duplicate(self);

    cpl_error_code fail =
            hdrl_spectrum1D_wavelength_mult_scalar_linear(to_ret, scale_linear);
    if(fail){
        hdrl_spectrum1D_delete(&to_ret);
    }
    return to_ret;
}

/**
 * @brief computes the elementwise shift of the wavelength by the shift parameter.
 * The self parameter is modified
 * @param self  spectrum
 * @param shift scalar factor
 * @return error code, for error codes see cpl_array_add_scalar
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_spectrum1D_wavelength_shift(hdrl_spectrum1D * self,
                                      hdrl_data_t shift)
{
    if(!self) return CPL_ERROR_NONE;

    return cpl_array_add_scalar(self->wavelength, shift);
}

/**
 * @brief computes the elementwise shift of the wavelength by the shift parameter.
 * @param self  spectrum
 * @param shift scalar factor
 * @return spectrum
 *
 * The self parameter is not modified, and a modified copy of self is returned.
 * Possible cpl-error-code set in this function: see cpl_array_add_scalar
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D *
hdrl_spectrum1D_wavelength_shift_create(const hdrl_spectrum1D * self,
                                      hdrl_data_t shift){

    if(!self) return NULL;

    hdrl_spectrum1D * to_ret = hdrl_spectrum1D_duplicate(self);

    cpl_error_code fail = hdrl_spectrum1D_wavelength_shift(to_ret, shift);

    if(fail){
        hdrl_spectrum1D_delete(&to_ret);
    }

    return to_ret;
}

/**
 * @brief converts the wavelength scale to linear.
 * @param self spectrum
 * @return error code, for error codes see cpl_array_exponential
 *
 * If the spectrum is already in linear scale nothing is done.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_spectrum1D_wavelength_convert_to_linear(hdrl_spectrum1D * self){

    if(self == NULL || self->wave_scale == hdrl_spectrum1D_wave_scale_linear)
        return CPL_ERROR_NONE;

    cpl_error_code fail = cpl_array_exponential(self->wavelength, CPL_MATH_E);
    self->wave_scale = hdrl_spectrum1D_wave_scale_linear;
    return fail;
}

/**
 * @brief converts the wavelength scale to linear.
 * @param self spectrum
 * @return the modified spectrum
 *
 * It returns a modified version of self. self is not modified.
 * If self is already in linear, the function is equivalent to a duplication.
 * Possible cpl-error-code set in this function: see cpl_array_exponential
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D *
hdrl_spectrum1D_wavelength_convert_to_linear_create(const hdrl_spectrum1D * self){

    hdrl_spectrum1D * to_ret =  hdrl_spectrum1D_duplicate(self);
    cpl_error_code fail = hdrl_spectrum1D_wavelength_convert_to_linear(to_ret);

    if(fail){
        hdrl_spectrum1D_delete(&to_ret);
    }
    return to_ret;
}

/**
 * @brief converts the wavelength scale to log. If the spectrum is already
 * in log scale nothing is done.
 * @param self spectrum
 * @return error code, for error codes see cpl_array_logarithm
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_spectrum1D_wavelength_convert_to_log(hdrl_spectrum1D * self){

    if(self == NULL || self->wave_scale == hdrl_spectrum1D_wave_scale_log)
        return CPL_ERROR_NONE;

    cpl_error_code fail = cpl_array_logarithm(self->wavelength, CPL_MATH_E);
    self->wave_scale = hdrl_spectrum1D_wave_scale_log;
    return fail;
}

/**
 * @brief converts the wavelength scale to log. It returns a modified version
 * of self. self is not modified. If self is already in log, the function is
 * equivalent to a duplication.
 * @param self spectrum
 * @return the modified spectrum
 *
 * It returns a modified version of self. self is not modified.
 * If self is already in log, the function is equivalent to a duplication.
 * Possible cpl-error-code set in this function: see cpl_array_logarithm
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D *
hdrl_spectrum1D_wavelength_convert_to_log_create(const hdrl_spectrum1D * self){

    hdrl_spectrum1D * to_ret =  hdrl_spectrum1D_duplicate(self);
    cpl_error_code fail = hdrl_spectrum1D_wavelength_convert_to_log(to_ret);

    if(fail){
        hdrl_spectrum1D_delete(&to_ret);
    }
    return to_ret;
}

/**
 * @brief the function selects or discards flux values according to whether the
 * value of the corresponding wavelength belongs to the interval
 * [min_lambda, max_lambda].
 * @param self           spectrum
 * @param windows        the intervals required for selection
 * @param is_internal    parallel array to windows, specifies if selection is
 * internal to the interval or external to the interval.
 * @return the selected subset of self or NULL in case of error
 *
 * @note: the complexity is O(kn) where k is the number of windows and n is the
 * number of samples in the spectrum. The assumption is that k << n, making the
 * complexity O(n).
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_ILLEGAL_OUTPUT: if no samples falls in the selected interval
 * - CPL_ERROR_ILLEGAL_INPUT: if windows and is_internal have different lengths,
 *   or if is_internal is not of type CPL_TYPE_INT
 * - CPL_ERROR_NULL_INPUT:  if any of the pointers is NULL
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D *
hdrl_spectrum1D_select_wavelengths(const hdrl_spectrum1D * self,
        const cpl_bivector * windows, const cpl_boolean is_internal){

    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(windows != NULL, CPL_ERROR_NULL_INPUT, NULL);

    const cpl_size sz = hdrl_spectrum1D_get_size(self);
    cpl_size num_selected = 0;

    for(cpl_size i = 0; i < sz; ++i){
        const hdrl_data_t w = hdrl_spectrum1D_get_wavelength_value(self, i, NULL);
        if(is_wlen_selected(windows, is_internal, w))
            num_selected++;
    }

    if(num_selected == sz)
        return hdrl_spectrum1D_duplicate(self);

    cpl_ensure(num_selected > 0, CPL_ERROR_ILLEGAL_OUTPUT, NULL);

    cpl_image * flux = cpl_image_new(num_selected, 1, HDRL_TYPE_DATA);
    cpl_image * flux_e = cpl_image_new(num_selected, 1, HDRL_TYPE_ERROR);
    cpl_array * wavs = cpl_array_new(num_selected, HDRL_TYPE_DATA);

    cpl_size idx_this = 0;
    for(cpl_size i = 0; i < sz; ++i){

        int rej = 0;
        const hdrl_data_t w = hdrl_spectrum1D_get_wavelength_value(self, i, NULL);
        if(!is_wlen_selected(windows, is_internal, w)) continue;

        hdrl_value v = hdrl_spectrum1D_get_flux_value(self, i, &rej);

        if(!rej){
            cpl_image_set(flux, idx_this + 1, 1, v.data);
            cpl_image_set(flux_e, idx_this + 1, 1, v.error);
        }
        else{
            cpl_image_reject(flux, idx_this + 1, 1);
            cpl_image_reject(flux_e, idx_this + 1, 1);
        }

        cpl_array_set(wavs, idx_this, w);

        idx_this++;
    }

    const hdrl_spectrum1D_wave_scale scale = hdrl_spectrum1D_get_scale(self);
    hdrl_spectrum1D * to_ret = hdrl_spectrum1D_create(flux,flux_e, wavs, scale);

    cpl_image_delete(flux);
    cpl_image_delete(flux_e);
    cpl_array_delete(wavs);

    return to_ret;
}

/**
 * @brief For every i-th element in bad_samples having value CPL_TRUE, the i-th
 * pixel in the 1D spectrum is marked as bad.
 * @param self        spectrum
 * @param bad_samples flags indicating whether the pixel is bad
 * @return the spectrum having the appropriate bad pixels selected.
 *
 * It returns a modified version of self. self is not modified.
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_ILLEGAL_INPUT: if min_lambda < max_lambda;
 * - CPL_ERROR_ILLEGAL_OUTPUT:  if the length of bad_samples and the length of
 *   self are different.
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D *
hdrl_spectrum1D_reject_pixels(const hdrl_spectrum1D * self,
        const cpl_array * bad_samples){

    const cpl_size sz = cpl_array_get_size(bad_samples);

    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(bad_samples != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(sz == hdrl_spectrum1D_get_size(self), CPL_ERROR_ILLEGAL_INPUT, NULL);

    hdrl_image * flux = hdrl_image_duplicate(hdrl_spectrum1D_get_flux(self));

    for(cpl_size i = 0; i < sz; ++i){

        if(cpl_array_get_int(bad_samples, i, NULL))
            hdrl_image_reject(flux, i + 1, 1);
    }

    const cpl_image * flx_cpl = hdrl_image_get_image(flux);
    const cpl_image * flx_e_cpl = hdrl_image_get_error(flux);
    hdrl_spectrum1D_wavelength wlen = hdrl_spectrum1D_get_wavelength(self);

    hdrl_spectrum1D * to_ret =
            hdrl_spectrum1D_create(flx_cpl, flx_e_cpl,
                    wlen.wavelength, wlen.scale);

    hdrl_image_delete(flux);
    return to_ret;
}

/**
 * @brief converts a spectrum in a table.
 * @param self                spectrum
 * @param flux_col_name       name of the column containing the flux
 * @param wavelength_col_name name of the column containing the wavelengths
 * @param flux_e_col_name     name of the column containing the flux error
 * @param flux_bpm_col_name   name of the column containing the flux bpm
 *
 * @return the newly allocated table. NULL if error occurs.
 *
 * If NULL is provided instead of a name, the corresponding column is not
 * inserted in the table. At least one between wavelength_col_name and
 * flux_col_name must be not NULL.
 *
 * Possible cpl-error-code set in this function: see cpl_table_wrap_double
 *
 */
/* ---------------------------------------------------------------------------*/
cpl_table * hdrl_spectrum1D_convert_to_table(
                const hdrl_spectrum1D * self, const char * flux_col_name,
                const char* wavelength_col_name, const char * flux_e_col_name,
                const char * flux_bpm_col_name){

    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(flux_col_name != NULL || wavelength_col_name != NULL,
            CPL_ERROR_NULL_INPUT, NULL);

    cpl_size sz = hdrl_spectrum1D_get_size(self);

    cpl_table * tb = cpl_table_new(sz);
    cpl_ensure(tb != NULL, CPL_ERROR_NULL_INPUT, NULL);

    cpl_error_code fail = hdrl_spectrum1D_append_to_table
            (self, tb, flux_col_name, wavelength_col_name, flux_e_col_name,
            flux_bpm_col_name);

    if(fail){
        cpl_table_delete(tb);
        tb = NULL;
    }
    return tb;
}

/**
 * @brief append a spectrum to a table.
 * @param self                spectrum
 * @param dest                table
 * @param flux_col_name       name of the column containing the flux
 * @param wavelength_col_name name of the column containing the wavelengths
 * @param flux_e_col_name     name of the column containing the flux error
 * @param flux_bpm_col_name   name of the column containing the flux bpm
 *
 * @return cpl_error_code, for error codes see cpl_table_wrap_double
 *
 * If NULL is provided instead of a name, the corresponding column is not
 * inserted in the table. At least one between wavelength_col_name and
 * flux_col_name must be not NULL.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_spectrum1D_append_to_table
(const hdrl_spectrum1D * self, cpl_table * dest,
const char * flux_col_name, const char* wavelength_col_name,
const char * flux_e_col_name, const char * flux_bpm_col_name){

    cpl_ensure_code(self != NULL && dest != NULL,CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(flux_col_name != NULL || wavelength_col_name != NULL,
            CPL_ERROR_NULL_INPUT);

    cpl_size cl_sz = cpl_table_get_nrow(dest);
    cpl_size sz = hdrl_spectrum1D_get_size(self);

    cpl_ensure_code(sz == cl_sz, CPL_ERROR_INCOMPATIBLE_INPUT);

    if(wavelength_col_name){
        double * lambdas = cpl_calloc(sz ,sizeof(double));
        for(cpl_size i = 0; i < sz; i++){
            lambdas[i] = hdrl_spectrum1D_get_wavelength_value(self, i, NULL);
        }
        cpl_error_code fail =
                cpl_table_wrap_double(dest, lambdas, wavelength_col_name);

        if(fail){
            cpl_free(lambdas);
            return fail;
        }
    }

    if(flux_col_name){
       double * flux = cpl_calloc(sz ,sizeof(double));
       for(cpl_size i = 0; i < sz; i++){
           flux[i] = hdrl_spectrum1D_get_flux_value(self, i, NULL).data;
       }

       cpl_error_code fail = cpl_table_wrap_double(dest, flux, flux_col_name);
       if(fail){
           cpl_free(flux);
           return fail;
       }
    }

    if(flux_e_col_name){
        double * e_flux = cpl_calloc(sz ,sizeof(double));
        for(cpl_size i = 0; i < sz; i++){
            e_flux[i] = hdrl_spectrum1D_get_flux_value(self, i, NULL).error;
        }
        cpl_error_code fail =
                cpl_table_wrap_double(dest, e_flux, flux_e_col_name);

        if(fail){
            cpl_free(e_flux);
            return fail;
        }
    }

    if(flux_bpm_col_name){
        int * bpm_flux = cpl_calloc(sz ,sizeof(int));
        for(cpl_size i = 0; i < sz; i++){
            hdrl_spectrum1D_get_flux_value(self, i, &bpm_flux[i]);
        }
        cpl_error_code fail =
                    cpl_table_wrap_int(dest, bpm_flux, flux_bpm_col_name);
        if(fail){
            cpl_free(bpm_flux);
            return fail;
        }
    }

    return CPL_ERROR_NONE;
}

/**
 * @brief convert a table to a spectrum
 * @param self                table
 * @param flux_col_name       name of the column containing the flux
 * @param wavelength_col_name name of the column containing the wavelengths
 * @param flux_e_col_name     name of the column containing the flux error
 * @param flux_bpm_col_name   name of the column containing the flux bpm
 * @param scale               scale of the spectrum
 * @return the newly allocated spectrum. NULL if error occurs. If NULL is provided
 * instead of flux_e_col_name, the spectrum is assumed error free.
 *
 * Possible cpl-error-code set in this function: see error free spectrum ctor
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D * hdrl_spectrum1D_convert_from_table
(const cpl_table * self, const char * flux_col_name,
const char* wavelength_col_name, const char * flux_e_col_name,
const char * flux_bpm_col_name, hdrl_spectrum1D_wave_scale scale){

    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(flux_col_name != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(wavelength_col_name != NULL, CPL_ERROR_NULL_INPUT, NULL);

    cpl_size sz = cpl_table_get_nrow(self);

    cpl_ensure(sz > 1, CPL_ERROR_ILLEGAL_INPUT, NULL);

    cpl_image * flux = cpl_image_new(sz, 1, HDRL_TYPE_DATA);
    cpl_image * flux_e = cpl_image_new(sz, 1, HDRL_TYPE_ERROR);
    cpl_array * lambdas = cpl_array_new(sz, HDRL_TYPE_DATA);


    for(cpl_size i = 0; i < sz; ++i){
        const double fx = cpl_table_get(self, flux_col_name, i, NULL);
        const double l = cpl_table_get(self, wavelength_col_name, i, NULL);

        double fx_e = 0;
        if(flux_e_col_name)
            fx_e = cpl_table_get(self, flux_e_col_name, i, NULL);

        int rej = 0;
        if(flux_bpm_col_name)
            rej = cpl_table_get_int(self, flux_bpm_col_name, i, NULL);

        cpl_image_set(flux, i + 1, 1, fx);

        if(rej)
            cpl_image_reject(flux, i + 1, 1);

        cpl_image_set(flux_e, i + 1, 1, fx_e);
        cpl_array_set(lambdas, i, l);
    }

    hdrl_spectrum1D * sp = hdrl_spectrum1D_create(flux, flux_e, lambdas, scale);

    cpl_image_delete(flux);
    cpl_image_delete(flux_e);
    cpl_array_delete(lambdas);
    return sp;
}

void  hdrl_spectrum1D_save(const hdrl_spectrum1D * s, const char * fname){
	if(s == NULL) return;
	cpl_table * tb = hdrl_spectrum1D_convert_to_table(s, "FLX", "WLN", "FLX_E",
			"FLX_BPM");
	cpl_table_save(tb, NULL, NULL, fname, CPL_IO_CREATE);
	cpl_table_delete(tb);
}

/**
 * @brief checks if two wavelengths array are defined on the same wavelengths.
 * @param w1                first wavelength array
 * @param w2                first wavelength array
 * @return CPL_TRUE if compatible, CPL_FALSE otherwise.
 */
/* ---------------------------------------------------------------------------*/
cpl_boolean
hdrl_spectrum1D_are_wavelengths_compatible(const cpl_array * w1,
                                           const cpl_array * w2){
    if(w1 == NULL && w2 == NULL) return CPL_TRUE;

    if(w1 == NULL) return CPL_FALSE;

    if(w2 == NULL) return CPL_FALSE;

    cpl_size sz = cpl_array_get_size(w1);
    if(sz != cpl_array_get_size(w2)) return CPL_FALSE;
    for(cpl_size i = 0; i < sz; i++){
        const double wa = cpl_array_get(w1, i, NULL);
        const double wb = cpl_array_get(w2, i, NULL);

        const double d =  wa - wb;
        if(fabs(d) > 1e-10 * CPL_MIN(wa, wb)){
            return CPL_FALSE;
        }
    }

    return CPL_TRUE;
}

/**
 * @brief checks if two spectrum wavelengths are equal.
 * @param s1                first spectrum wavelength
 * @param s2                second spectrum wavelength
 * @return CPL_TRUE if compatible, CPL_FALSE otherwise.
 */
/* ---------------------------------------------------------------------------*/
cpl_boolean
hdrl_spectrum1D_are_spectra_compatible(const hdrl_spectrum1D_wavelength* s1,
                                       const hdrl_spectrum1D_wavelength* s2){

    if(s1 == NULL && s2 == NULL) return CPL_TRUE;

    if(s1 == NULL) return CPL_FALSE;

    if(s2 == NULL) return CPL_FALSE;

    if(s1->scale != s2->scale) return CPL_FALSE;

   return hdrl_spectrum1D_are_wavelengths_compatible(s1->wavelength,
           s2->wavelength);
}

/**
 * @brief checks if the spectrum is defined on uniformly sampled wavelengths.
 * @param self              the input spectrum
 * @param bin               bin width - output parameter
 *
 * @return CPL_TRUE if if the spectrum is defined on uniformly sampled wavelengths,
 * false otherwise.
 */
/* ---------------------------------------------------------------------------*/
cpl_boolean hdrl_spectrum1D_is_uniformly_sampled(const hdrl_spectrum1D * self,
        double * bin){

    *bin = 0.0;

    if(self == NULL) return CPL_FALSE;

    const cpl_size sz = hdrl_spectrum1D_get_size(self);

    if(sz <= 2) return CPL_TRUE;

    double * vd = cpl_calloc(sz, sizeof(double));

    for(cpl_size i = 0; i < sz; ++i){
        vd[i] = hdrl_spectrum1D_get_wavelength_value(self, i, NULL);
    }

    qsort(vd, sz, sizeof(double), compare_double);

    cpl_boolean to_ret = is_uniformly_sampled(vd, sz, bin);

    cpl_free(vd);

    return to_ret;

}

/*-----------------------------------------------------------------------------
                    Private Functions Implementation
 -----------------------------------------------------------------------------*/
/**
 * @brief execute the passed function on self and other. Used to centralize the
 * compatibility checks. Mutate self.
 * @param self              first spectrum, the one mutated
 * @param other             second spectrum, will not be mutated
 * @param func              function that implements the operation between
 *                          self and other
 * @return error code
 */
/* ---------------------------------------------------------------------------*/
static inline cpl_error_code operate_spectra_flux_mutate(hdrl_spectrum1D * self,
                                                  const hdrl_spectrum1D * other,
                                                  operate_image_mutable func){
    cpl_ensure_code(self != NULL && other != NULL,
    CPL_ERROR_NULL_INPUT);

    hdrl_spectrum1D_wavelength w_self = hdrl_spectrum1D_get_wavelength(self);
    hdrl_spectrum1D_wavelength w_other = hdrl_spectrum1D_get_wavelength(other);
    cpl_ensure_code(hdrl_spectrum1D_are_spectra_compatible(&w_self, &w_other),
            CPL_ERROR_INCOMPATIBLE_INPUT);

    hdrl_image * f1 = self->flux;
    const hdrl_image * f2 = other->flux;

    cpl_ensure_code(f1 != NULL && f2 != NULL,
    CPL_ERROR_NULL_INPUT);

    func(f1, f2);

    return CPL_ERROR_NONE;
}

/**
 * @brief execute the passed function on self and other. Return the newly
 * calculated spectrum.
 * @param self              first spectrum, will not be mutated
 * @param other             second spectrum, will not be mutated
 * @param func              function that implements the operation between
 *                          self and other
 * @return the new spectrum. NULL in case of error.
 */
/* ---------------------------------------------------------------------------*/
static inline hdrl_spectrum1D *
operate_spectra_flux_create(const hdrl_spectrum1D * self,
                            const hdrl_spectrum1D * other,
                            operate_image_mutable func){

    hdrl_spectrum1D * to_ret = hdrl_spectrum1D_duplicate(self);

    cpl_error_code fail = operate_spectra_flux_mutate(to_ret, other, func);

    if(fail){
        hdrl_spectrum1D_delete(&to_ret);
    }
    return to_ret;
}

/**
 * @brief execute the passed function on self and scalar. Used to centralize the
 * compatibility checks. Mutate self.
 * @param self              spectrum, will be mutated
 * @param scalar            scalar value
 * @param func              function that implements the operation between
 *                          self and scalar
 * @return error code
 */
/* ---------------------------------------------------------------------------*/
static inline cpl_error_code
operate_spectra_scalar_flux_mutate(hdrl_spectrum1D * self,
                                   hdrl_value scalar,
                                   operate_image_scalar_mutable func){

    if(!self) return CPL_ERROR_NONE;

    hdrl_image * f1 = self->flux;

    cpl_ensure_code(f1 != NULL, CPL_ERROR_NULL_INPUT);

    func(f1, scalar);

    return CPL_ERROR_NONE;
}

/**
 * @brief execute the passed function on self and scalar. Return the newly
 * calculated spectrum.
 * @param self              first spectrum, will not be mutated
 * @param scalar            scalar value
 * @param func              function that implements the operation between
 *                          self and scalar
 * @return the new spectrum. NULL in case of error.
 */
/* ---------------------------------------------------------------------------*/
static inline hdrl_spectrum1D *
operate_spectra_scalar_flux_create(const hdrl_spectrum1D * self,
                                   hdrl_value scalar,
                                   operate_image_scalar_mutable func){

    hdrl_spectrum1D * to_ret = hdrl_spectrum1D_duplicate(self);

    cpl_error_code fail =
            operate_spectra_scalar_flux_mutate(to_ret, scalar, func);

    if(fail){
        hdrl_spectrum1D_delete(&to_ret);
    }

    return to_ret;
}

static inline
hdrl_spectrum1D * hdrl_spectrum1D_wrap(hdrl_image * arg_flux,
                                       cpl_array * wavelength,
                                       hdrl_spectrum1D_wave_scale scale){

    hdrl_spectrum1D * to_ret = cpl_calloc(1, sizeof(*to_ret));

    to_ret->flux = arg_flux;
    to_ret->wavelength = wavelength;
    to_ret->wave_scale = scale;
    return to_ret;
}

static inline
int compare_double(const void * a, const void * b){
    const double ad = *((const double *)a);
    const double bd = *((const double *)b);
    const double delta = ad - bd;
    if(delta > 0.0) return 1;
    if(delta < 0.0) return -1;
    return 0;
}

#if HDRL_SIZEOF_DATA == 4
	const double wave_delta = 1.e-5;
#else
	const double wave_delta = 1.e-6;
#endif

static inline
cpl_boolean is_uniformly_sampled(const double * v, cpl_size sz, double * bin){

    const double d = v[1] - v[0];
    *bin = d;
    for(cpl_size i = 1; i < sz - 1; ++i){
        const double d2 = (v[i + 1] - v[i]);
        const double eps = (fabs(d2 - d) / d);
        if(eps > wave_delta){
            return CPL_FALSE;
        }
    }

    return CPL_TRUE;
}


static inline cpl_boolean
is_contained_in_at_least_one_window(const cpl_bivector * windows, const hdrl_data_t w){
    const cpl_size sz = cpl_bivector_get_size(windows);

    for(cpl_size i = 0; i < sz; ++i){
        const double wmin = cpl_vector_get(cpl_bivector_get_x_const(windows), i);
        const double wmax = cpl_vector_get(cpl_bivector_get_y_const(windows), i);

        if(w >= wmin && w <= wmax) return CPL_TRUE;
    }

    return CPL_FALSE;
}

static inline cpl_boolean
is_wlen_selected(const cpl_bivector * windows, const cpl_boolean is_internal,
        const hdrl_data_t w){

    if(is_internal) return is_contained_in_at_least_one_window(windows, w);

    return !is_contained_in_at_least_one_window(windows, w);

}

/**@}*/
