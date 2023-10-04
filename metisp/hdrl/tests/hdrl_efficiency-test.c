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

#include "../hdrl_efficiency.h"

#include <math.h>
#include <cpl.h>

/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_efficiency_test
 */
/*----------------------------------------------------------------------------*/

void test_efficiency(void){

    cpl_size sz = 10;
    cpl_image * flux = cpl_image_new(sz,1, CPL_TYPE_DOUBLE);
    cpl_image * flux_e = cpl_image_new(sz,1, CPL_TYPE_DOUBLE);
    cpl_array * waves = cpl_array_new(sz, CPL_TYPE_DOUBLE);

    for(cpl_size i = 0; i < sz; ++i){
        cpl_image_set(flux, i + 1, 1, (i + 1) * 2.5);
        cpl_image_set(flux_e, i + 1, 1, (i + 1) * .02);
        cpl_array_set(waves, i, (i + 1) * 3.0);
    }

    hdrl_spectrum1D * sp_obs =
      hdrl_spectrum1D_create(flux, flux_e, waves, hdrl_spectrum1D_wave_scale_linear);

    for(cpl_size i = 0; i < sz; ++i){
        cpl_image_set(flux, i + 1, 1, (i + 1) * 0.5);
        cpl_image_set(flux_e, i + 1, 1, 0.0);
        cpl_array_set(waves, i, (i + 1) * 3.0);
    }

    hdrl_spectrum1D * sp_std =
      hdrl_spectrum1D_create(flux, flux_e, waves, hdrl_spectrum1D_wave_scale_linear);

    for(cpl_size i = 0; i < sz; ++i){
        cpl_image_set(flux, i + 1, 1, (i + 1) * 1.5);
        cpl_image_set(flux_e, i + 1, 1, 0.0);
        cpl_array_set(waves, i, (i + 1) * 3.0);
    }

    hdrl_spectrum1D * sp_ext =
      hdrl_spectrum1D_create(flux, flux_e, waves, hdrl_spectrum1D_wave_scale_linear);


    hdrl_parameter * pars = hdrl_efficiency_parameter_create(
            (hdrl_value){1.2, 0.0},
            (hdrl_value){0.4, 0.0},
            (hdrl_value){11.*12., 0.0},
            (hdrl_value){1.1, 0.0},
            (hdrl_value){2.2, 0.0});

    hdrl_spectrum1D * sp_eff =
            hdrl_efficiency_compute(sp_obs, sp_std, sp_ext, pars);

    hdrl_parameter_delete(pars); pars = NULL;

    hdrl_value v = hdrl_spectrum1D_get_flux_value(sp_eff, 3, NULL);
    cpl_test_abs(v.data, 3.75528e-06, 1e-5);
    cpl_test_abs(v.error, 3.00422e-08, 1e-5);

    cpl_array_delete(waves);
    cpl_image_delete(flux);
    cpl_image_delete(flux_e);
    hdrl_spectrum1D_delete(&sp_obs);
    hdrl_spectrum1D_delete(&sp_std);
    hdrl_spectrum1D_delete(&sp_eff);
    hdrl_spectrum1D_delete(&sp_ext);
}

/* Given the simplified formula: I_std * 10 ^(-0.4 * E_x)* E_ph we calculated the
 * analytical error propagation function using the Wolfram Alpha website and we
 * check this analytical model against the error propagation in the hdrl library.*/
hdrl_value get_error(const hdrl_value s, const hdrl_value x, const hdrl_data_t l){
    hdrl_error_t err = (hdrl_error_t)(exp(-1.84207 * x.data));
    err *= (hdrl_error_t)(0.848304 * pow(x.error, 2.0) * pow(s.data, 2.0) +
            pow(s.error, 2.0));

    hdrl_data_t ephot = (hdrl_data_t)fabs(E_ph(l).data);
    hdrl_data_t data = (hdrl_data_t)(s.data * pow(10.0, -0.4 * x.data));

    const hdrl_value to_ret = {data * ephot, sqrt(err) * (hdrl_error_t)ephot};
    return to_ret;
}

/* test error propagation using analytical model. We simplify the problem,
 * everything, except extinction and observed spectrum are considered error-free.
 * The values are set so that the formula becomes = I_std * 10 ^(0.4 * E_x)* E_ph.
 * We want to exercise the exponential which has been implemented for this feature.*/
void test_efficiency_error_propagation(void){

    const cpl_size len = 20;

    cpl_image * std_obs_flux = cpl_image_new(len, 1, CPL_TYPE_DOUBLE);
    cpl_image * std_obs_flux_e = cpl_image_new(len, 1, CPL_TYPE_DOUBLE);

    cpl_image * std_model_flux = cpl_image_new(len, 1, CPL_TYPE_DOUBLE);
    cpl_image * std_model_flux_e = cpl_image_new(len, 1, CPL_TYPE_DOUBLE);

    cpl_image * ext_flux = cpl_image_new(len, 1, CPL_TYPE_DOUBLE);
    cpl_image * ext_flux_e = cpl_image_new(len, 1, CPL_TYPE_DOUBLE);

    cpl_array * wave = cpl_array_new(len, CPL_TYPE_DOUBLE);

    hdrl_value Ap   = {3.0, 0.0};
    hdrl_value Am   = {2.0, 0.0};
    hdrl_value G    = {1.0, 0.0};
    hdrl_value Tex  = {1.0, 0.0};
    hdrl_value Atel = {1.0, 0.0};


    for(cpl_size i = 0; i < len; ++i){

        const double l = (i * .3 + 1.0) * 1e-4;
        cpl_array_set(wave, i, l);

        cpl_image_set(std_obs_flux, i + 1, 1, sin(l * CPL_MATH_PI));
        cpl_image_set(std_obs_flux_e, i + 1, 1, 0.2 * sin(l * CPL_MATH_PI));

        cpl_image_set(ext_flux, i + 1, 1, 1.7 * sin(l * CPL_MATH_PI));
        cpl_image_set(ext_flux_e, i + 1, 1, 0.02 * sin(l * CPL_MATH_PI));

        /* denominator must be always 1*/
        cpl_image_set(std_model_flux, i + 1, 1, 1.0);
        cpl_image_set(std_model_flux_e, i + 1, 1, 0.0);
    }

    hdrl_spectrum1D * I_std = hdrl_spectrum1D_create(std_obs_flux, std_obs_flux_e,
            wave, hdrl_spectrum1D_wave_scale_linear);

    hdrl_spectrum1D * I_ref_std = hdrl_spectrum1D_create(std_model_flux,
            std_model_flux_e, wave, hdrl_spectrum1D_wave_scale_linear);

    hdrl_spectrum1D * ext = hdrl_spectrum1D_create(ext_flux, ext_flux_e,
            wave, hdrl_spectrum1D_wave_scale_linear);

    cpl_image_delete(std_obs_flux);
    cpl_image_delete(std_obs_flux_e);

    cpl_image_delete(std_model_flux);
    cpl_image_delete(std_model_flux_e);

    cpl_image_delete(ext_flux);
    cpl_image_delete(ext_flux_e);


    hdrl_parameter * pars =
            hdrl_efficiency_parameter_create(Ap, Am, G, Tex, Atel);

    hdrl_spectrum1D * eff = hdrl_efficiency_compute(I_std,
            I_ref_std, ext, pars);

    for(cpl_size i = 0; i < len; ++i){
        int rej;
        const hdrl_value eff_i1 = hdrl_spectrum1D_get_flux_value(eff, i, &rej);

        const hdrl_value I_std_i = hdrl_spectrum1D_get_flux_value(I_std, i, &rej);
        const hdrl_value ext_i = hdrl_spectrum1D_get_flux_value(ext, i, &rej);

        const hdrl_data_t w = cpl_array_get(wave, i, &rej);

        const hdrl_value eff_i2 = get_error(I_std_i, ext_i, w);

        cpl_test_rel(eff_i1.data, eff_i2.data, 1e-5);
        cpl_test_rel(eff_i1.error, eff_i2.error, 1e-5);
    }

    cpl_array_delete(wave);

    hdrl_parameter_delete(pars);

    hdrl_spectrum1D_delete(&I_std);
    hdrl_spectrum1D_delete(&I_ref_std);
    hdrl_spectrum1D_delete(&ext);

    hdrl_spectrum1D_delete(&eff);
}

void test_efficiency_spectrum_external_to_models(void){

    cpl_size sz = 10;
    cpl_image * flux = cpl_image_new(sz,1, CPL_TYPE_DOUBLE);
    cpl_image * flux_e = cpl_image_new(sz,1, CPL_TYPE_DOUBLE);
    cpl_array * waves = cpl_array_new(sz, CPL_TYPE_DOUBLE);

    for(cpl_size i = 0; i < sz; ++i){
        cpl_image_set(flux, i + 1, 1, (i + 1) * 2.5);
        cpl_image_set(flux_e, i + 1, 1, (i + 1) * .02);

        if(i == sz - 1)
            cpl_array_set(waves, i, 3.0 * (sz + 5));
        else
            cpl_array_set(waves, i, (i - 1) * 3.0);
    }

    cpl_array * waves_obs = cpl_array_duplicate(waves);

    hdrl_spectrum1D * sp_obs =
      hdrl_spectrum1D_create(flux, flux_e, waves, hdrl_spectrum1D_wave_scale_linear);

    for(cpl_size i = 0; i < sz; ++i){
        cpl_image_set(flux, i + 1, 1, (i + 1) * 0.5);
        cpl_image_set(flux_e, i + 1, 1, 0.0);
        cpl_array_set(waves, i, (i + 1) * 3.0);
    }

    hdrl_spectrum1D * sp_std =
      hdrl_spectrum1D_create(flux, flux_e, waves, hdrl_spectrum1D_wave_scale_linear);

    for(cpl_size i = 0; i < sz; ++i){
        cpl_image_set(flux, i + 1, 1, (i + 1) * 1.5);
        cpl_image_set(flux_e, i + 1, 1, 0.0);
        cpl_array_set(waves, i, (i + 2) * 3.0);
    }

    hdrl_spectrum1D * sp_ext =
      hdrl_spectrum1D_create(flux, flux_e, waves, hdrl_spectrum1D_wave_scale_linear);


    hdrl_parameter * pars = hdrl_efficiency_parameter_create(
            (hdrl_value){1.2,       0.0},
            (hdrl_value){0.4,       0.0},
            (hdrl_value){11. * 12., 0.0},
            (hdrl_value){1.1,       0.0},
            (hdrl_value){2.2,       0.0});

    hdrl_spectrum1D * sp_eff =
            hdrl_efficiency_compute(sp_obs, sp_std, sp_ext, pars);

    hdrl_parameter_delete(pars); pars = NULL;

    cpl_test_eq(hdrl_spectrum1D_get_size(sp_eff), sz - 4);

    const cpl_array * wavs_eff =
            hdrl_spectrum1D_get_wavelength(sp_eff).wavelength;

    for(cpl_size i = 3; i < sz - 1; ++i){

        double w_f = cpl_array_get(wavs_eff, i - 3, NULL);
        double w_obs = cpl_array_get(waves_obs, i, NULL);

        cpl_test_rel(w_f, w_obs, 1e-16);
    }

    cpl_array_delete(waves);
    cpl_array_delete(waves_obs);
    cpl_image_delete(flux);
    cpl_image_delete(flux_e);
    hdrl_spectrum1D_delete(&sp_obs);
    hdrl_spectrum1D_delete(&sp_std);
    hdrl_spectrum1D_delete(&sp_eff);
    hdrl_spectrum1D_delete(&sp_ext);
}


/*----------------------------------------------------------------------------*/
/**
  @brief   Unit tests of efficiency calculation module
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    test_efficiency();
    test_efficiency_error_propagation();
    test_efficiency_spectrum_external_to_models();

    cpl_test_error(CPL_ERROR_NONE);

    return cpl_test_end(0);
}

