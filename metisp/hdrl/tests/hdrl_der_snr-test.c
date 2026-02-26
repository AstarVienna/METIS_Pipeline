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

#include "hdrl_DER_SNR.h"
#include "hdrl_spectrum.h"
#include "hdrl_random.h"

#include <cpl.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>



/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_der_snr_test
 */
/*----------------------------------------------------------------------------*/

static inline cpl_image * get_noisy_flux(const cpl_image * img);

#define ARR_SIZE(a) ((cpl_size)((sizeof((a)) / sizeof((a[0])))))

static inline
cpl_image * get_error(const int* data, const int* map, cpl_size length){
    /* map[j] is the position in unsorted  where the j-th element is going to
     * be inserted*/
    cpl_array * wav = cpl_array_new(length, HDRL_TYPE_DATA);
    cpl_image * flux = cpl_image_new(length, 1, HDRL_TYPE_DATA);

    for(cpl_size j = 0; j < length; ++j){
       cpl_image_set(flux, map[j] + 1, 1, data[j]);
       cpl_array_set(wav, map[j], j + 1);
    }

    const hdrl_data_t *flux_p =
           (const hdrl_data_t *)cpl_image_get_data_const(flux);

    const cpl_binary *msk  = NULL;
    cpl_image *flux_e = estimate_noise_DER_SNR(flux_p, msk, wav, length, 5);

    cpl_image_delete(flux);
    cpl_array_delete(wav);

    return flux_e;
}

void test_DER_SNR_sort(void){

    int data[] = {
            1, 5, 10, 5, 23, 1, 8, 17, 21, 7, 11, 13, 5, 99, 12, 4,
            1, 5, 10, 5, 23, 1, 8, 17, 21, 7, 11, 13, 5, 99, 12, 4
    };

    int map1[ARR_SIZE(data)] = {0};
    int map2[ARR_SIZE(data)] = {0};

    const cpl_size length = ARR_SIZE(data);

    for(cpl_size i = 0; i < length; ++i){
        map1[i] = i;
        map2[i] = i;
    }

    for(cpl_size i = 0; i < length; ++i){

          const cpl_size source = i;
          const cpl_size dest = ((double)rand() / (double)RAND_MAX) * (length - 1);

          cpl_test(dest >= 0 && dest < length);

          const int data_s = map1[source];
          const int data_d = map1[dest];

          map1[source] = data_d;
          map1[dest] = data_s;
      }

    cpl_image * flux_e_unsorted = get_error(data, map1, length);
    cpl_image * flux_e_sorted = get_error(data, map2, length);

    /* map1[i] is the position in unsorted  where the i-th element of sorted
     * is inserted.*/
    for(cpl_size i = 0; i < length; ++i){
        int rej1;
        const double data1 = cpl_image_get(flux_e_sorted, i + 1, 1, &rej1);

        int rej2;
        const double data2 = cpl_image_get(flux_e_unsorted, map1[i] + 1, 1, &rej2);

        cpl_test_eq(rej1, rej2);
        cpl_test_abs(data1, data2, 1e-3);
    }

    cpl_image_delete(flux_e_sorted);
    cpl_image_delete(flux_e_unsorted);
}

void test_DER_SNR(void){

    int data[] = {
            1, 5, 10, 5, 23, 1, 8, 17, 21, 7, 11, 13, 5, 99, 12, 4,
            1, 5, 10, 5, 23, 1, 8, 17, 21, 7, 11, 13, 5, 99, 12, 4
    };

    cpl_array *  wav = cpl_array_new(ARR_SIZE(data), HDRL_TYPE_DATA);
    cpl_image * flux = cpl_image_new(ARR_SIZE(data), 1, HDRL_TYPE_DATA);

    for(cpl_size i = 0; i < ARR_SIZE(data); ++i){
        cpl_image_set(flux, i +1, 1, data[i]);
        cpl_array_set(wav, i, i + 1);
    }

    const hdrl_data_t * flux_p =
            (const hdrl_data_t *)cpl_image_get_data_const(flux);

    const cpl_binary *msk = NULL;
    hdrl_data_t err =
            estimate_noise_window(flux_p, msk, 0,
                    ARR_SIZE(data) - 1, ARR_SIZE(data));
    cpl_test_abs(err, 12.105, 1e-3);

    cpl_image * flux_e = estimate_noise_DER_SNR(flux_p, msk, wav,
            ARR_SIZE(data), 5);

    cpl_test_eq(cpl_image_get_size_x(flux), cpl_image_get_size_x(flux_e));
    cpl_test_eq(cpl_image_get_size_y(flux), cpl_image_get_size_y(flux_e));

    int rej = 0;
    err = cpl_image_get(flux_e, 7, 1, &rej);
    cpl_test_abs(err, 13.921, 1e-3);

    err = cpl_image_get(flux_e, 6, 1, &rej);
    cpl_test_abs(err, 13.921, 1e-3);

    err = cpl_image_get(flux_e, 1, 1, &rej);
    cpl_test_abs(err, 2.421, 1e-3);

    cpl_image_delete(flux_e);


    /*bad pixel is bad also in error*/
    cpl_image_reject(flux, 2, 1);

    flux_p = (const hdrl_data_t *)cpl_image_get_data_const(flux);
    const cpl_mask *mask = cpl_image_get_bpm_const(flux);
    msk  = cpl_mask_get_data_const(mask);

    flux_e = estimate_noise_DER_SNR(flux_p, msk, wav, ARR_SIZE(data), 5);

    cpl_test(cpl_image_is_rejected(flux_e, 2, 1));

    /*bad pixels in window are skipped*/
    err = cpl_image_get(flux_e, 6, 1, &rej);
    cpl_test_abs(err, 14.829, 1e-3);
    cpl_test_eq(rej, 0);

    /*good pixel surrounded by bad pixels so that there is no pixel to calculate
     * error, becomes bad*/
    cpl_image_delete(flux_e);

    for(cpl_size i = 1; i <= 11; i++)
    {
        if(i == 6) continue;
        cpl_image_reject(flux, i, 1);
    }

    flux_p = (const hdrl_data_t *)cpl_image_get_data_const(flux);
    mask = cpl_image_get_bpm_const(flux);
    msk  = cpl_mask_get_data_const(mask);

    flux_e = estimate_noise_DER_SNR(flux_p, msk, wav, ARR_SIZE(data), 5);
    cpl_test(cpl_image_is_rejected(flux_e, 6, 1));
    cpl_test(!cpl_image_is_rejected(flux, 6, 1));

    cpl_image_delete(flux_e);
    cpl_image_delete(flux);
    cpl_array_delete(wav);
}

void test_DER_SNR_performance(void){

    static const int sz = 2000;
    static const double delta = 3.14 / 2000.0;
    static const double peak = 1e3;
    static const int n_iter = 100;

    cpl_image * flux = cpl_image_new(sz, 1, HDRL_TYPE_DATA);
    cpl_array * lambdas = cpl_array_new(sz, HDRL_TYPE_DATA);

    for(cpl_size i = 0; i < sz; i++){
        double x = delta * (i+1);
        cpl_array_set(lambdas, i, x);
        cpl_image_set(flux, i + 1, 1, peak * sin(x));
    }

    cpl_image * std_dev_theo = cpl_image_power_create(flux, 1./2.);
    cpl_image * DER_SNR_avg = cpl_image_new(sz, 1,HDRL_TYPE_DATA);

    for(cpl_size i = 0; i < n_iter; ++i){
        cpl_image * noisy_flux_i = get_noisy_flux(flux);

        const hdrl_data_t * noisy_flux =
                (const hdrl_data_t *)cpl_image_get_data_const(noisy_flux_i);

        const cpl_binary *msk = NULL;
        cpl_image * e = estimate_noise_DER_SNR(noisy_flux, msk, lambdas, sz, 5);

        cpl_image_divide_scalar(e, n_iter);
        cpl_image_add(DER_SNR_avg, e);
        cpl_image_delete(e);

        cpl_image_delete(noisy_flux_i);
    }

    cpl_image * ratio = cpl_image_divide_create(std_dev_theo, DER_SNR_avg);
    double avg_ratio = cpl_image_get_absflux(ratio) / sz;

    cpl_test_leq(avg_ratio, 1.1);
    cpl_test_leq(.9, avg_ratio);

    cpl_image_delete(flux);
    cpl_image_delete(DER_SNR_avg);
    cpl_array_delete(lambdas);
    cpl_image_delete(ratio);
    cpl_image_delete(std_dev_theo);
}

/*----------------------------------------------------------------------------*/
/**
  @brief   Unit tests of DER_SNR calculation module
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    srand (time(NULL));

    test_DER_SNR();
    test_DER_SNR_sort();
    test_DER_SNR_performance();

    return cpl_test_end(0);
}

/*----------------------------------------------------------------------------*/
/**
  @brief   Private functions
 **/
/*----------------------------------------------------------------------------*/
static inline
cpl_image * get_noisy_flux(const cpl_image * img){

    cpl_size sx = cpl_image_get_size_x(img);
    cpl_size sy = cpl_image_get_size_y(img);

    cpl_image * to_ret = cpl_image_new(sx, sy, HDRL_TYPE_DATA);

    hdrl_random_state * rng = hdrl_random_state_new(1, NULL);

    for(cpl_size x = 1; x <= sx; x++){
        for(cpl_size y = 1; y <= sy; y++){
            int rej = 0;
            double clean_value = cpl_image_get(img, x, y, &rej);
            double noisy_value = hdrl_random_poisson(rng, clean_value);
            cpl_image_set(to_ret, x, y, noisy_value);
        }
    }

    hdrl_random_state_delete(rng);
    return to_ret;
}


