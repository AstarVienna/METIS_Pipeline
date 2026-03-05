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
#include "hdrl_spectrumlist.h"
#include "hdrl_spectrum_resample.h"
#include "hdrl_imagelist.h"

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
static inline
cpl_error_code check_getter(const hdrl_spectrum1Dlist * s, const cpl_size idx);

static inline
void push_back(hdrl_spectrum1Dlist *self, hdrl_spectrum1D * s);

static inline hdrl_spectrum1D **
safe_realloc(hdrl_spectrum1D ** current, const cpl_size old_capacity,
        const cpl_size new_capacity);

static inline hdrl_imagelist *
create_list(hdrl_spectrum1D ** resampled_spectra,
        const hdrl_spectrum1Dlist * ori_spectra,
        const cpl_boolean mark_bpm_in_interpolation);

static inline void
delete_spectrum1D_array(hdrl_spectrum1D ** resampled_spectra,
        const cpl_size num_spectra);

static inline hdrl_image *
get_padded_flux(const hdrl_spectrum1D * resampled_spectrum,
        const hdrl_spectrum1D * ori_spectrum,
        const cpl_boolean mark_bpm_in_interpolation);

static inline cpl_boolean
are_spectra_valid(const hdrl_spectrum1Dlist * list);

static inline cpl_error_code
get_first_error_code(const cpl_error_code * cds, const cpl_size length);

static inline cpl_boolean
check_scales_are_same(const hdrl_spectrum1Dlist * list);

static inline double
get_wmin_valid(const hdrl_spectrum1D * s);

static inline double
get_wmax_valid(const hdrl_spectrum1D * s);

static inline void
remove_if_neighbors_are_rejected(hdrl_image * flx,
        const hdrl_spectrum1D * ori_spectrum, const cpl_array * wlens);

static inline hdrl_spectrum1D *
get_interp_bpm(const hdrl_spectrum1D * s, const cpl_array * wlens);

static inline cpl_boolean
contains(const hdrl_spectrum1Dlist * list, const hdrl_spectrum1D * s);
/* ---------------------------------------------------------------------------*/
/**
 * @brief hdrl_spectrum1Dlist default constructor
 * @return a newly allocated hdrl_spectrum1Dlist
 *
 * The constructor allocates memory of a hdrl_spectrum1Dlist structure. The struct
 * has zero capacity when allocated.
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1Dlist * hdrl_spectrum1Dlist_new(void){
    hdrl_spectrum1Dlist * to_ret = cpl_calloc(1, sizeof(*to_ret));

    to_ret->length = 0;
    to_ret->capacity= 0;
    to_ret->spectra = NULL;
    return to_ret;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief hdrl_spectrum1Dlist copy-constructor
 * @param l       input list
 * @return a copy of the input list
 *
 * The constructor allocates memory of a new hdrl_spectrum1Dlist structure. Each
 * element of the new list is a copy of the corresponding element in the source
 * list.
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1Dlist * hdrl_spectrum1Dlist_duplicate(const hdrl_spectrum1Dlist * l){

	if(l == NULL) return NULL;

	hdrl_spectrum1Dlist * to_ret = hdrl_spectrum1Dlist_new();

	for(cpl_size i = 0; i < hdrl_spectrum1Dlist_get_size(l); ++i){
		const hdrl_spectrum1D * s_old = hdrl_spectrum1Dlist_get_const(l, i);
		hdrl_spectrum1D * s = hdrl_spectrum1D_duplicate(s_old);
		hdrl_spectrum1Dlist_set(to_ret, s, i);
	}
	return to_ret;
}

/**
 * @brief hdrl_spectrum1Dlist wrapper
 * @param self       array of pointers to hdrl_spectrum1D
 * @param sz         number of elements in self
 * @return self wrapped in a hdrl_spectrum1Dlist. The returned structure takes
 * ownership of the self pointer.
 *
 * The constructor allocates memory of a new hdrl_spectrum1Dlist structure. The
 * structure takes ownership of self, and therefore it is responsible for its
 * de-allocation. Self must not contain duplicate or NULL pointers.
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1Dlist *
hdrl_spectrum1Dlist_wrap(hdrl_spectrum1D ** self, const cpl_size sz){
    hdrl_spectrum1Dlist * to_ret = hdrl_spectrum1Dlist_new();
    to_ret->spectra = self;
    to_ret->capacity = to_ret->length = sz;
    return to_ret;
}

/**
 * @brief hdrl_spectrum1Dlist getter of the i-th element
 * @param self       hdrl_spectrum1Dlist
 * @param idx        index of the element we want. 0 <= index < size
 * @return a pointer to the i-th element. NULL in case of error.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: if self is NULL
 * - CPL_ERROR_ACCESS_OUT_OF_RANGE: index is less than 0 or index >= size
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D *
hdrl_spectrum1Dlist_get(hdrl_spectrum1Dlist * self, const cpl_size idx){

    cpl_error_code fail = check_getter(self, idx);
    cpl_ensure(fail == CPL_ERROR_NONE, fail, NULL);

    return self->spectra[idx];

}

/**
 * @brief hdrl_spectrum1Dlist getter of the i-th element
 * @param self       hdrl_spectrum1Dlist
 * @param idx        index of the element we want. 0 <= index < size
 * @return a const pointer to the i-th element. NULL in case of error.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: if self is NULL
 * - CPL_ERROR_ACCESS_OUT_OF_RANGE: index is less than 0 or index >= size
 */
/* ---------------------------------------------------------------------------*/
const hdrl_spectrum1D *
hdrl_spectrum1Dlist_get_const(const hdrl_spectrum1Dlist *self , const cpl_size idx){

    cpl_error_code fail = check_getter(self, idx);
    cpl_ensure(fail == CPL_ERROR_NONE, fail, NULL);

    return self->spectra[idx];
}

/**
 * @brief hdrl_spectrum1Dlist setter of the i-th element
 * @param self       hdrl_spectrum1Dlist
 * @param s       	 hdrl_spectrum1D we want to insert
 * @param idx        position the spectrum is inserted in. 0 <= index <= size
 * @return a cpl_error_code.
 *
 * The function insert s inside self. if index == size, the capacity of the list
 * is increased to accommodate the new element. If a spectrum was already contained
 * in the i-th position, it is de-allocated. The list becomes responsible for the
 * memory management of s.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: if self is NULL
 * - CPL_ERROR_ACCESS_OUT_OF_RANGE: index < 0 or index > size
 * - CPL_ERROR_ILLEGAL_INPUT: if s is already contained inside self
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code
hdrl_spectrum1Dlist_set(hdrl_spectrum1Dlist * self,
        hdrl_spectrum1D * s, const cpl_size idx){

    cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(self->length >= 0, CPL_ERROR_ACCESS_OUT_OF_RANGE);
    cpl_ensure_code(idx <= self->length, CPL_ERROR_ACCESS_OUT_OF_RANGE);
    cpl_ensure_code(!contains(self, s), CPL_ERROR_ILLEGAL_INPUT);

    if(idx == self->length){
        push_back(self, s);
        return CPL_ERROR_NONE;
    }

    hdrl_spectrum1D * old = self->spectra[idx];
    hdrl_spectrum1D_delete(&old);

    self->spectra[idx] = s;

    return CPL_ERROR_NONE;
}

/**
 * @brief hdrl_spectrum1Dlist remove of the i-th element
 * @param self       hdrl_spectrum1Dlist
 * @param idx        position of element we want to remove from the list.
 * 					 0 <= index < size
 * @return the removed element. NULL in case of error of if the idx-th element was NULL
 *
 * The function extract the idx-th element from the list. It shifts all the elements
 * from idx + 1 to size - 1 down of one position. If the new size is less than half
 * the capacity, the capacity is reduced.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: if self is NULL
 * - CPL_ERROR_ACCESS_OUT_OF_RANGE: index < 0 or index >= size
 */
/* ---------------------------------------------------------------------------*/
hdrl_spectrum1D *
hdrl_spectrum1Dlist_unset(hdrl_spectrum1Dlist * self, const cpl_size idx){

    cpl_error_code fail = check_getter(self, idx);
    cpl_ensure(fail == CPL_ERROR_NONE, fail, NULL);

    hdrl_spectrum1D * to_ret = self->spectra[idx];

    const cpl_size sz = hdrl_spectrum1Dlist_get_size(self);

    for(cpl_size i = idx; i < sz - 1; ++i){
        self->spectra[i] = self->spectra[i + 1];
    }
    self->length--;

    const cpl_size new_capacity = self->capacity / 2;

    if(hdrl_spectrum1Dlist_get_size(self) <= new_capacity){
        self->spectra = safe_realloc(self->spectra, self->capacity, new_capacity);
        self->capacity = new_capacity;
    }

    return to_ret;
}

/**
 * @brief hdrl_spectrum1Dlist destructor
 * @param l       hdrl_spectrum1Dlist
 *
 * The function deallocates the list and all the spectra it was containing. If l
 * is NULL no operation is performed.
 */
/* ---------------------------------------------------------------------------*/
void hdrl_spectrum1Dlist_delete(hdrl_spectrum1Dlist * l){

    if(l == NULL) return;

    for(cpl_size i = 0; i < hdrl_spectrum1Dlist_get_size(l); ++i){
        hdrl_spectrum1D_delete(&l->spectra[i]);
    }
    cpl_free(l->spectra);
    cpl_free(l);
}

/**
 * @brief hdrl_spectrum1Dlist getter for size
 * @param l       hdrl_spectrum1Dlist
 *
 * @return the size of the list. The size can be less or equal than the capacity
 * of the list. The size refers to the number of valid elements in the list.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: if l is NULL
 */
/* ---------------------------------------------------------------------------*/
cpl_size
hdrl_spectrum1Dlist_get_size(const hdrl_spectrum1Dlist * l){

    cpl_ensure(l != NULL, CPL_ERROR_NULL_INPUT, 0);

    return l->length;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief collapsing a hdrl_spectrum1Dlist. The spectra in list are first
 * resampled on the wavelengths wlengths. The resampling method is regulated
 * by the resample_par. For more information on samplig_par see
 * hdrl_spectrum1D_resample_on_array.
 * The resampled fluxes are collapsed as in hdrl_imagelist_collapse and the
 * collapsing is regulated by the parameter stacking_par.
 * For more information on stacking_par please see the documentation of
 * hdrl_imagelist_collapse.
 *
 * @param list                          hdrl_spectrum1D list
 * @param stacking_par                  parameter regulating the stacking
 * @param wlengths                      wavelengths the resulting spectrum is
 *                                      defined on
 * @param resample_par                  parameter regulating the resampling
 * @param mark_bpm_in_interpolation     if true interpolated pixels whose
 *                                      neighbors (in the original spectrum)
 *                                      are rejected, are not considered
 *                                      during collapsing
 * @param result                        the resulting spectrum
 * @param contrib		                output contribution mask
 * @param resampled_and_aligned_fluxes  resampled and aligned fluxes to be
 *                                      collapsed
 * @return cpl_error_code
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT: if any among input pointers in NULL of if any of the
 * 	spectra inside the list are NULL.
 * 	- CPL_ERROR_ILLEGAL_INPUT: if all the elements inside list are not defined on
 * 	the same scale.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code
hdrl_spectrum1Dlist_collapse (const hdrl_spectrum1Dlist *list,
         const hdrl_parameter * stacking_par,
         const cpl_array * wlengths, const hdrl_parameter * resample_par,
         const cpl_boolean mark_bpm_in_interpolation,
         hdrl_spectrum1D ** result,  cpl_image ** contrib,
         hdrl_imagelist ** resampled_and_aligned_fluxes){

    cpl_ensure_code(are_spectra_valid(list), CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(wlengths != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(check_scales_are_same(list), CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(result != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(resampled_and_aligned_fluxes != NULL, CPL_ERROR_NULL_INPUT);
    *result = NULL;
    *contrib = NULL;

    const cpl_size num_spectra =  hdrl_spectrum1Dlist_get_size(list);
    hdrl_spectrum1D ** resampled_spectra =
            cpl_calloc(num_spectra, sizeof(hdrl_spectrum1D*));

    cpl_ensure_code(num_spectra > 0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_error_code * errors = cpl_calloc(num_spectra, sizeof(cpl_error_code));

    HDRL_OMP(omp parallel for)
    for(cpl_size i = 0; i < num_spectra; ++i){
        const hdrl_spectrum1D * this_s = hdrl_spectrum1Dlist_get_const(list, i);
        resampled_spectra[i] =
                hdrl_spectrum1D_resample_on_array(this_s, wlengths, resample_par);
        errors[i] = cpl_error_get_code();
    }


    cpl_error_code err = get_first_error_code(errors, num_spectra);
    cpl_free(errors);

    if(!err){
        hdrl_imagelist * stack_list =
                create_list(resampled_spectra, list, mark_bpm_in_interpolation);

        hdrl_image * stacked_img = NULL;

        err = hdrl_imagelist_collapse(stack_list, stacking_par,
                        &stacked_img, contrib);

        *resampled_and_aligned_fluxes = stack_list;

        if(!err){
            const hdrl_spectrum1D_wave_scale scale =
                            hdrl_spectrum1D_get_scale(hdrl_spectrum1Dlist_get_const(list, 0));

            *result = hdrl_spectrum1D_create(hdrl_image_get_image(stacked_img),
                                            hdrl_image_get_error(stacked_img),
                                            wlengths,
                                            scale);
        }

        hdrl_image_delete(stacked_img);
    }

    delete_spectrum1D_array(resampled_spectra, num_spectra);
    return err;
}

/*-----------------------------------------------------------------------------
                    Private Functions Implementation
 -----------------------------------------------------------------------------*/
/*Gets the minimum wavelengths ignoring bad pixels*/
static inline double
get_wmin_valid(const hdrl_spectrum1D * s){
    const cpl_size sz = hdrl_spectrum1D_get_size(s);

    double wmin = INFINITY;

    for(cpl_size i = 0; i < sz; ++i){
        int rej = 0;
        double w = hdrl_spectrum1D_get_wavelength_value(s, i, &rej);
        if(rej) continue;
        if(w < wmin) wmin = w;
    }
    return wmin;
}
/*Gets the maximum wavelengths ignoring bad pixels*/
static inline double
get_wmax_valid(const hdrl_spectrum1D * s){
    const cpl_size sz = hdrl_spectrum1D_get_size(s);

    double wmax = -INFINITY;

    for(cpl_size i = 0; i < sz; ++i){
        int rej = 0;
        double w = hdrl_spectrum1D_get_wavelength_value(s, i, &rej);
        if(rej) continue;
        if(w > wmax) wmax = w;
    }
    return wmax;
}

static inline cpl_image *
get_img_from_bpm(hdrl_spectrum1D_wavelength * w){
    if(w->bpm)
        return cpl_image_new_from_mask(w->bpm);

    const cpl_size sz = cpl_array_get_size(w->wavelength);
    return cpl_image_new(sz, 1, CPL_TYPE_INT);
}

static inline hdrl_spectrum1D *
get_interp_bpm(const hdrl_spectrum1D * s, const cpl_array * wlens){

    hdrl_spectrum1D_wavelength wlen_ori = hdrl_spectrum1D_get_wavelength(s);
    cpl_image * flx = get_img_from_bpm(&wlen_ori);
    hdrl_spectrum1D * bpm = hdrl_spectrum1D_create_error_free(flx,
            wlen_ori.wavelength, wlen_ori.scale);
    cpl_image_delete(flx);

    hdrl_parameter * par =
            hdrl_spectrum1D_resample_interpolate_parameter_create(
                    hdrl_spectrum1D_interp_linear);
    hdrl_spectrum1D * interp_bpm = hdrl_spectrum1D_resample_on_array(bpm, wlens,
            par);
    hdrl_spectrum1D_delete(&bpm);
    hdrl_parameter_delete(par);
    return interp_bpm;
}

static inline void
remove_if_neighbors_are_rejected(hdrl_image * flx,
        const hdrl_spectrum1D * ori_spectrum, const cpl_array * wlens){
    /*We interpolate the bpm to see if the elements in wlens have a bad pixel
     * close by. If that is the case we reject them.*/
    hdrl_spectrum1D * interp_bpm = get_interp_bpm(ori_spectrum, wlens);

    for(cpl_size i = 0; i < hdrl_spectrum1D_get_size(interp_bpm); ++i){
        const double v = hdrl_spectrum1D_get_flux_value(interp_bpm, i, NULL).data;
        if(v > HDRL_EPS_DATA){
            hdrl_image_reject(flx, i + 1, 1);
        }
    }
    hdrl_spectrum1D_delete(&interp_bpm);
}

static inline hdrl_image *
get_padded_flux(const hdrl_spectrum1D * resampled_spectrum,
        const hdrl_spectrum1D * ori_spectrum,
        const cpl_boolean mark_bpm_in_interpolation){

    if(resampled_spectrum == NULL) return NULL;

    const double wmin = get_wmin_valid(ori_spectrum);

    const double wmax = get_wmax_valid(ori_spectrum);

    if(isinf(wmin) || isinf(wmax)) return NULL;

    hdrl_image * flx =
            hdrl_image_duplicate(hdrl_spectrum1D_get_flux(resampled_spectrum));

    const cpl_array * wlens =
            hdrl_spectrum1D_get_wavelength(resampled_spectrum).wavelength;

    for(cpl_size i = 0; i < hdrl_spectrum1D_get_size(resampled_spectrum); ++i){
        const double wlen = cpl_array_get(wlens, i, NULL);
        if(wlen < wmin || wlen > wmax) {
            hdrl_image_reject(flx, i + 1, 1);
        }
    }

    if(mark_bpm_in_interpolation)
        remove_if_neighbors_are_rejected(flx, ori_spectrum, wlens);

    return flx;
}

static inline cpl_boolean
check_scales_are_same(const hdrl_spectrum1Dlist * list){
    if(list == NULL) return CPL_TRUE;
    const cpl_size sz = hdrl_spectrum1Dlist_get_size(list);

    if(sz <= 1) return CPL_TRUE;

    const hdrl_spectrum1D_wave_scale scale =
            hdrl_spectrum1D_get_scale(hdrl_spectrum1Dlist_get_const(list, 0));

    for(cpl_size i = 1; i < sz; ++i){
        const hdrl_spectrum1D_wave_scale scale_this =
                hdrl_spectrum1D_get_scale(hdrl_spectrum1Dlist_get_const(list, 0));

        if(scale_this != scale) return CPL_FALSE;
    }

    return CPL_TRUE;
}
/*Given a array of hdrl_spectrum1D this function creates an hdrl_imagelist containing
 * the fluxes of the spectra. The current implementation of resample extrapolates
 * points outside the interval where values are available. We reject those points.*/
static inline hdrl_imagelist *
create_list(hdrl_spectrum1D ** resampled_spectra,
        const hdrl_spectrum1Dlist * ori_spectra,
        const cpl_boolean mark_bpm_in_interpolation){

    const cpl_size num_spectra = hdrl_spectrum1Dlist_get_size(ori_spectra);
    hdrl_image ** images = cpl_calloc(num_spectra, sizeof(hdrl_image*));
    cpl_error_code * errs = cpl_calloc(num_spectra, sizeof(cpl_error_code));
    HDRL_OMP(omp parallel for)
    for(cpl_size i = 0; i < num_spectra; i++){
            hdrl_image * img = get_padded_flux(resampled_spectra[i],
                    hdrl_spectrum1Dlist_get_const(ori_spectra, i),
                    mark_bpm_in_interpolation);
            images[i] = img;
            errs[i] = cpl_error_get_code();
    }

    cpl_error_code fail = get_first_error_code(errs, num_spectra);
    cpl_free(errs);

    hdrl_imagelist * list = NULL;
    if(!fail){
        list = hdrl_imagelist_new();

        /* hdrl_imagelist_set has a race condition when growing the internal buffer
         * it cannot be parallelized */
        for(cpl_size i = 0; i < num_spectra; i++){
            if(images[i] != NULL)
                hdrl_imagelist_set(list, images[i], i);
        }
    }
    cpl_free(images);
    return list;
}
/*Function used in the getters: check that idx is in the correct ranges.*/
static inline
cpl_error_code check_getter(const hdrl_spectrum1Dlist *s, const cpl_size idx){

    if(s == NULL) return CPL_ERROR_NULL_INPUT;

    if(idx < 0) return CPL_ERROR_ACCESS_OUT_OF_RANGE;

    if(idx >= s->length) return CPL_ERROR_ACCESS_OUT_OF_RANGE;

    return CPL_ERROR_NONE;
}
/*wrapper around realloc, works are realloc, except if old_capacity  == 0 (in this
 * case it works like calloc) and if new_capacity == 0 (in this case it works like free)*/
static inline hdrl_spectrum1D **
safe_realloc(hdrl_spectrum1D ** current, const cpl_size old_capacity,
        const cpl_size new_capacity){

    /* Going from empty list to list with elements */
    if(old_capacity == 0 && new_capacity > 0){
        return cpl_calloc(new_capacity, sizeof(hdrl_spectrum1D * ));
    }

    /* Clean capacity */
    if(new_capacity == 0){
        cpl_free(current);
        return NULL;
    }

    hdrl_spectrum1D ** to_ret =
            cpl_realloc(current, sizeof(hdrl_spectrum1D * ) * new_capacity);

    for(cpl_size i = old_capacity + 1; i < new_capacity; ++i){
        to_ret[i] = NULL;
    }

    return to_ret;
}

/*increase the capacity of the list if length >= capacity*/
static inline
void resize_if_needed(hdrl_spectrum1Dlist *self){

    if(self->length < self->capacity) return;

    const cpl_size new_capacity = self->capacity  == 0 ? 1 : 2 * self->capacity;

    self->spectra = safe_realloc(self->spectra, self->capacity,
            new_capacity);
    self->capacity = new_capacity;
}

/*add an element in the back of a list, it can increase the capacity if needed*/
static inline
void push_back(hdrl_spectrum1Dlist *self, hdrl_spectrum1D * s){

    resize_if_needed(self);
    self->spectra[self->length] = s;
    self->length++;
}

/*frees an array of spectrum1D and frees every spectrum1D*/
static inline void
delete_spectrum1D_array(hdrl_spectrum1D ** resampled_spectra,
        const cpl_size num_spectra){

    hdrl_spectrum1Dlist * l =
            hdrl_spectrum1Dlist_wrap(resampled_spectra, num_spectra);
    hdrl_spectrum1Dlist_delete(l);
}

/*check that all the spectra in the list are != NULL*/
static inline cpl_boolean
are_spectra_valid(const hdrl_spectrum1Dlist * list){
    if(list == NULL) return CPL_FALSE;

    const cpl_size length = hdrl_spectrum1Dlist_get_size(list);
    for(cpl_size i = 0; i < length; ++i){
        const hdrl_spectrum1D * s = hdrl_spectrum1Dlist_get_const(list, i);
        if(s == NULL) return CPL_FALSE;
    }
    return CPL_TRUE;
}
/*get the first element in the array different from CPL_ERROR_NONE*/
static inline cpl_error_code
get_first_error_code(const cpl_error_code * cds, const cpl_size length){
    for(cpl_size i = 0; i < length; ++i){
        if(cds[i]) return cds[i];
    }
    return CPL_ERROR_NONE;
}

static inline cpl_boolean
contains(const hdrl_spectrum1Dlist * list, const hdrl_spectrum1D * s){
    const cpl_size sz = hdrl_spectrum1Dlist_get_size(list);
    for(cpl_size i = 0; i < sz; ++i){
        const hdrl_spectrum1D * s_in = hdrl_spectrum1Dlist_get_const(list, i);
        if(s_in == s) return CPL_TRUE;
    }
    return CPL_FALSE;
}

/**@}*/
