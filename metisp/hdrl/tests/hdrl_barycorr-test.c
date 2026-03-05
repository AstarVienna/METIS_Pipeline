/*
 * hdrl_barycorr-test.c
 *
 *  Created on: Jan 31, 2022
 *      Author: agabasch
 */

/*
 * This file is part of the HDRL Toolkit.
 * Copyright (C) 2022 European Southern Observatory
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*----------------------------------------------------------------------------*/
/**
 *                              Includes
 */
/*----------------------------------------------------------------------------*/

#include "hdrl_barycorr.h"

/*----------------------------------------------------------------------------*/
/**
 *                              Functions
 */
/*----------------------------------------------------------------------------*/


static cpl_table * hdrl_create_eoptable(void){

  cpl_table * eop_table = cpl_table_new(4);
  cpl_table_new_column(eop_table, "MJD", CPL_TYPE_DOUBLE);
  cpl_table_new_column(eop_table, "PMX", CPL_TYPE_DOUBLE);
  cpl_table_new_column(eop_table, "PMY", CPL_TYPE_DOUBLE);
  cpl_table_new_column(eop_table, "DUT", CPL_TYPE_DOUBLE);
  cpl_table_new_column(eop_table, "FLAG", CPL_TYPE_STRING);

  cpl_table_set_double(eop_table, "MJD", 0, 5.884300000000E+04);
  cpl_table_set_double(eop_table, "MJD", 1, 5.884400000000E+04);
  cpl_table_set_double(eop_table, "MJD", 2, 5.884500000000E+04);
  cpl_table_set_double(eop_table, "MJD", 3, 5.884600000000E+04);

  cpl_table_set_double(eop_table, "PMX", 0, 8.855900000000E-02);
  cpl_table_set_double(eop_table, "PMX", 1, 8.687500000000E-02);
  cpl_table_set_double(eop_table, "PMX", 2, 8.503000000000E-02);
  cpl_table_set_double(eop_table, "PMX", 3, 8.285400000000E-02);

  cpl_table_set_double(eop_table, "PMY", 0, 2.800810000000E-01);
  cpl_table_set_double(eop_table, "PMY", 1, 2.806870000000E-01);
  cpl_table_set_double(eop_table, "PMY", 2, 2.812910000000E-01);
  cpl_table_set_double(eop_table, "PMY", 3, 2.816470000000E-01);

  cpl_table_set_double(eop_table, "DUT", 0, -1.761465000000E-01);
  cpl_table_set_double(eop_table, "DUT", 1, -1.762087000000E-01);
  cpl_table_set_double(eop_table, "DUT", 2, -1.762508000000E-01);
  cpl_table_set_double(eop_table, "DUT", 3, -1.763259000000E-01);

  cpl_table_set_string(eop_table, "FLAG", 0, "I");
  cpl_table_set_string(eop_table, "FLAG", 1, "I");
  cpl_table_set_string(eop_table, "FLAG", 2, "I");
  cpl_table_set_string(eop_table, "FLAG", 3, "I");

  return eop_table;

}

static cpl_error_code hdrl_barycorr_compute_test(void) {

/*
 This unittest has been extracted from a regression tests using the following
 ESPRESSO file: ADP.2021-04-15T13:14:18.089.fits

 Results from espresso and hdrl read as follows:
 hdrl barycorr recipe: 22.814877482069
 ESPRESSO pipeline:     22.814548243970
*/
  cpl_table * eop_table = hdrl_create_eoptable();

  double ra = 149.823138;
  double dec = -27.39211;
  double mjdobs = 58844.22531243;
  double time_to_mid_exposure = 900.;
  double longitude = -70.4045;
  double latitude = -24.6268;
  double elevation = 2648.;
  double pressure = 0.;
  double temperature = 0.;
  double humidity = 0.;
  double wavelength = 0.;

  double barycorr = 0.;

  hdrl_barycorr_compute(ra, dec, eop_table, mjdobs, time_to_mid_exposure,
			 longitude, latitude, elevation, pressure, temperature,
			 humidity, wavelength, &barycorr);

  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_abs(barycorr, 22814.877482069, FLT_EPSILON);

  hdrl_barycorr_compute(-1., dec, eop_table, mjdobs, time_to_mid_exposure,
			 longitude, latitude, elevation, pressure, temperature,
			 humidity, wavelength, &barycorr);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

  hdrl_barycorr_compute(ra, 100., eop_table, mjdobs, time_to_mid_exposure,
			 longitude, latitude, elevation, pressure, temperature,
			 humidity, wavelength, &barycorr);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

  hdrl_barycorr_compute(ra, dec, eop_table, mjdobs, time_to_mid_exposure,
			 -200., latitude, elevation, pressure, temperature,
			 humidity, wavelength, &barycorr);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

  hdrl_barycorr_compute(ra, dec, eop_table, mjdobs, time_to_mid_exposure,
			 longitude, 100, elevation, pressure, temperature,
			 humidity, wavelength, &barycorr);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

  hdrl_barycorr_compute(ra, dec, NULL, mjdobs, time_to_mid_exposure,
			 longitude, latitude, elevation, pressure, temperature,
			 humidity, wavelength, &barycorr);
  cpl_test_error(CPL_ERROR_NULL_INPUT);


  cpl_table_delete(eop_table);
  return cpl_error_get_code();
}

static cpl_error_code hdrl_eop_interpolate_test(void) {

  /*
   This unittest has been extracted from a regression tests using the following
   ESPRESSO file: ADP.2021-04-15T13:14:18.089.fits
   See also unit test above
   */

  cpl_table * eop_table = hdrl_create_eoptable();

  /* Compute Earth Orientation Parameters for the mean MJD */
  double mean_mjd = 58844.235729096661089;
  double dut1 = 0., pmx = 0., pmy = 0.;

  hdrl_parameter * resample_par =
      hdrl_spectrum1D_resample_interpolate_parameter_create(hdrl_spectrum1D_interp_linear);

  hdrl_eop_interpolate(mean_mjd, eop_table, resample_par, &pmx, &pmy, &dut1) ;
  cpl_test_error(CPL_ERROR_NONE);

  /* Interpolated Earth Orientation Parameter for MJD-OBS 58844.235729096661089:
   * pmx: 0.086440079816660284062,
   * pmy: 0.28082938037438331946,
   * dut1: -0.17621862419496941987*/
  cpl_test_abs(pmx,   0.08644007981666028406, FLT_EPSILON);
  cpl_test_abs(pmy,   0.28082938037438331946, FLT_EPSILON);
  cpl_test_abs(dut1, -0.17621862419496941987, FLT_EPSILON);

  hdrl_eop_interpolate(mean_mjd, NULL, resample_par, &pmx, &pmy, &dut1) ;
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_eop_interpolate(mean_mjd, eop_table, NULL, &pmx, &pmy, &dut1) ;
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_eop_interpolate(mean_mjd, NULL, resample_par, NULL, &pmy, &dut1) ;
  cpl_test_error(CPL_ERROR_NULL_INPUT);
  hdrl_eop_interpolate(mean_mjd, NULL, resample_par, &pmx, NULL, &dut1) ;
  cpl_test_error(CPL_ERROR_NULL_INPUT);
  hdrl_eop_interpolate(mean_mjd, NULL, resample_par, &pmx, &pmy, NULL) ;
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  hdrl_eop_interpolate(-100., eop_table, resample_par, &pmx, &pmy, &dut1) ;
  cpl_test_error(CPL_ERROR_NONE);


  cpl_table * eop_table_with_invalid = cpl_table_duplicate(eop_table);

  cpl_table_set_column_invalid(eop_table_with_invalid,"DUT", 0, 2);
  hdrl_eop_interpolate(1., eop_table_with_invalid, resample_par, &pmx, &pmy, &dut1) ;
  cpl_test_error(CPL_ERROR_NONE);
  cpl_table_delete(eop_table_with_invalid);

  eop_table_with_invalid = cpl_table_duplicate(eop_table);
  cpl_table_set_column_invalid(eop_table_with_invalid,"DUT", 0, 4);
  hdrl_eop_interpolate(1., eop_table_with_invalid, resample_par, &pmx, &pmy, &dut1) ;
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_table_delete(eop_table_with_invalid);

  cpl_table_erase_column(eop_table, "PMY");
  hdrl_eop_interpolate(mean_mjd, eop_table, resample_par, &pmx, &pmy, &dut1) ;
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

  cpl_table_delete(eop_table);
  hdrl_parameter_delete(resample_par);
  return cpl_error_get_code();
}



/*----------------------------------------------------------------------------*/
/**
 * @brief   Main function
 *
 * @return (int)cpl_error_code if error, CPL_ERROR_NONE (== 0) in other case
 *
 */
/*----------------------------------------------------------------------------*/

int main(void)
{
  cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

  hdrl_barycorr_compute_test();
  hdrl_eop_interpolate_test();

  return cpl_test_end(0);
}




