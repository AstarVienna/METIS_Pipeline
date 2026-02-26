/*
 * hdrl_barycorr.c
 *
 *  Created on: Jan 31, 2022
 *      Author: agabasch
 */

/*
 * This file is part of the ESO Toolkit
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*-----------------------------------------------------------------------------
                                  Includes
 -----------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "hdrl_barycorr.h"
#include <erfa.h>

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_barycorr   Barycentric correction

  @brief This module contains a function to derives the barycentric
  correction of an observation, i.e. the wavelength shift to apply to
  a spectrum to compensate for the motion of the observer with respect
  to the barycenter of the solar system, by using the <a
  href="https://github.com/liberfa/erfa">ERFA</a> (Essential Routines
  for Fundamental Astronomy) library. ERFA is a C library containing
  key algorithms for astronomy, and is based on the <a
  href="http://www.iausofa.org">SOFA library</a> published by the
  International Astronomical Union (IAU).

 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
  @private
 * @brief Interpolate EOP parameter for a given MJD-OBS
 * @param mjd             MJD-OBS - Modified Julian Day
 * @param eop_table       Table containing the Earth Orientation Parameter
 * @param resample_par    Interpolation method
 * @param pmx             Output: x pole
 * @param pmy             Output: y pole
 * @param dut             Output: UT1-UTC
 *
 * @return  CPL_ERROR_NONE if everything is ok, an error code otherwise
 *
 * Please note that for the interpolation cpl_table_erase_invalid() is called
 * first - in order to remove all rows with at least one invalid element.
 *
 *
 */
/* ---------------------------------------------------------------------------*/

cpl_error_code
hdrl_eop_interpolate(double mjd, const cpl_table * eop_table, hdrl_parameter *
		      resample_par, double *pmx, double *pmy, double *dut)
{
  cpl_ensure_code (eop_table,    CPL_ERROR_NULL_INPUT);
  cpl_ensure_code (resample_par, CPL_ERROR_NULL_INPUT);
  cpl_ensure_code (pmx, CPL_ERROR_NULL_INPUT);
  cpl_ensure_code (pmy, CPL_ERROR_NULL_INPUT);
  cpl_ensure_code (dut, CPL_ERROR_NULL_INPUT);

  cpl_table * eop_table_loc = cpl_table_duplicate(eop_table);

  /* Doing some baisc checks on the local eop table */

  if (!cpl_table_has_column(eop_table_loc, "MJD") ||
      !cpl_table_has_column(eop_table_loc, "PMX") ||
      !cpl_table_has_column(eop_table_loc, "PMY") ||
      !cpl_table_has_column(eop_table_loc, "DUT")){
      cpl_table_delete(eop_table_loc);
      return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
				   "The EOP table does not have all required "
				   "columns, i.e. MJD, PMX, PMY, DUT");
  }

  cpl_table_unselect_all (eop_table_loc);
  cpl_table_or_selected_invalid (eop_table_loc, "MJD");
  cpl_table_or_selected_invalid (eop_table_loc, "PMX");
  cpl_table_or_selected_invalid (eop_table_loc, "PMY");
  cpl_table_or_selected_invalid (eop_table_loc, "DUT");
  cpl_table_erase_selected (eop_table_loc);

  if (cpl_table_get_nrow(eop_table_loc) < 1){
      cpl_table_delete(eop_table_loc);
      return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
				   "The EOP table does not have entries after "
				   "removing all invalid elements");
  }

  cpl_size mjd_max = cpl_table_get_column_max(eop_table_loc, "MJD");
  cpl_size mjd_min = cpl_table_get_column_min(eop_table_loc, "MJD");

  if (mjd < mjd_min || mjd > mjd_max) {
      *pmx = cpl_table_get_column_median(eop_table_loc, "PMX");
      *pmy = cpl_table_get_column_median(eop_table_loc, "PMY");
      *dut = cpl_table_get_column_median(eop_table_loc, "DUT");
      cpl_msg_warning(cpl_func,"The exposure MJD-OBS is outside the validity "
	  "range of the EOP calibration. Using median values instead of "
	  "interpolated values. Please provide a more up to date EOP file for "
	  "a higher accuracy.");
      cpl_table_delete(eop_table_loc);
      return cpl_error_get_code();
  }

  hdrl_spectrum1D * pmx_spec =
      hdrl_spectrum1D_convert_from_table(eop_table_loc, "PMX", "MJD", NULL, NULL,
					 hdrl_spectrum1D_wave_scale_linear);
  hdrl_spectrum1D * pmy_spec =
      hdrl_spectrum1D_convert_from_table(eop_table_loc, "PMY", "MJD", NULL, NULL,
					 hdrl_spectrum1D_wave_scale_linear);
  hdrl_spectrum1D * dut_spec =
      hdrl_spectrum1D_convert_from_table(eop_table_loc, "DUT", "MJD", NULL, NULL,
					 hdrl_spectrum1D_wave_scale_linear);

  cpl_array * mjd_out = cpl_array_new(1, CPL_TYPE_DOUBLE);
  cpl_array_set_double(mjd_out, 0, mjd);

  hdrl_spectrum1D * pmx_spec_resampled =
      hdrl_spectrum1D_resample_on_array(pmx_spec, mjd_out, resample_par);
  hdrl_spectrum1D * pmy_spec_resampled =
      hdrl_spectrum1D_resample_on_array(pmy_spec, mjd_out, resample_par);
  hdrl_spectrum1D * dut_spec_resampled =
      hdrl_spectrum1D_resample_on_array(dut_spec, mjd_out, resample_par);

  cpl_array_delete(mjd_out);
  hdrl_spectrum1D_delete(&pmx_spec);
  hdrl_spectrum1D_delete(&pmy_spec);
  hdrl_spectrum1D_delete(&dut_spec);

  int rej_pmx = 0, rej_pmy = 0, rej_dut = 0;

  hdrl_value hvpmx = hdrl_spectrum1D_get_flux_value(pmx_spec_resampled, 0, &rej_pmx);
  hdrl_value hvpmy = hdrl_spectrum1D_get_flux_value(pmy_spec_resampled, 0, &rej_pmy);
  hdrl_value hvdut = hdrl_spectrum1D_get_flux_value(dut_spec_resampled, 0, &rej_dut);

  hdrl_spectrum1D_delete(&pmx_spec_resampled);
  hdrl_spectrum1D_delete(&pmy_spec_resampled);
  hdrl_spectrum1D_delete(&dut_spec_resampled);

  *pmx = hvpmx.data;
  *pmy = hvpmy.data;
  *dut = hvdut.data;

  cpl_table_delete(eop_table_loc);
  return cpl_error_get_code();
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Derives the barycentric correction using the erfa function
 *  eraApco13(). The latter For a terrestrial observer, prepare
 *  star-independent astrometry parameters for transformations between
 *  ICRS and observed coordinates. ERFA models are used to obtain the
 *  Earth ephemeris, CIP/CIO and refraction constants.
 *
 * @param ra                   Target right ascension (J2000) [deg]
 * @param dec                  Target declination (J2000) [deg]
 * @param eop_table            Earth orientation parameter
 * @param mjdobs               Start of observation [days]
 * @param time_to_mid_exposure Time to mid exposure, e.g. EXPTIME/2. [s]
 * @param longitude            Telescope geodetic longitude (+ = East ) [deg]
 * @param latitude             Telescope geodetic latitude  (+ = North) [deg]
 * @param elevation            Telescope elevation above sea level [m]
 * @param pressure             Pressure at the observer [hPa == mbar]
 * @param temperature          Ambient temperature at the observer [deg C]
 * @param humidity             Relative humidity at the observer [range 0 - 1]
 * @param wavelength           Observing wavelength [micrometer]
 * @param barycorr             Output: Computed barycentric correction [m/s]
 *
 * @return  CPL_ERROR_NONE if everything is ok, an error code otherwise
 *
 * @note Please check the notes in the <a
 * href="https://github.com/liberfa/erfa/blob/master/src/apco13.c">eraApco13()</a>
 * function.


 */
/* ---------------------------------------------------------------------------*/
cpl_error_code
hdrl_barycorr_compute(double ra, double dec, const cpl_table * eop_table,
		       double mjdobs, double time_to_mid_exposure,
		       double longitude, double latitude, double elevation,
		       double pressure, double temperature,
		       double humidity, double  wavelength , double *barycorr){

  cpl_ensure_code(ra  >= 0.    && ra  <   360., CPL_ERROR_ILLEGAL_INPUT);
  cpl_ensure_code(dec >= -90.  && dec <=  90.,  CPL_ERROR_ILLEGAL_INPUT);
  /* Todo check range for longitude and latidude !*/
  cpl_ensure_code(longitude >= -180.  && longitude <=  180.,  CPL_ERROR_ILLEGAL_INPUT);
  cpl_ensure_code(latitude >= -90.  && latitude <=  90.,  CPL_ERROR_ILLEGAL_INPUT);
  cpl_ensure_code(eop_table != NULL,  CPL_ERROR_NULL_INPUT);


  cpl_error_code err = CPL_ERROR_NONE;
  ra *= CPL_MATH_RAD_DEG;
  dec *= CPL_MATH_RAD_DEG;
  longitude *= CPL_MATH_RAD_DEG; // Lon in [rad], East positive
  latitude *= CPL_MATH_RAD_DEG;  // Lat in [rad]

  /* Mean mjdobs from the middle of the exposure time */
  double mean_mjd = mjdobs + (time_to_mid_exposure/3600./24.);
  cpl_msg_info(cpl_func, "Mean MJD-OBS used to derive barycorr: %g", mean_mjd);


  /* Compute Earth Orientation Parameters for the mean MJD */
  double dut1 = 0, pmx = 0, pmy = 0;

  // ToDo Expose it in the API?
  // hdrl_parameter * resample_par = hdrl_spectrum1D_resample_interpolate_parameter_create(hdrl_spectrum1D_interp_akima);
  // hdrl_parameter * resample_par = hdrl_spectrum1D_resample_interpolate_parameter_create(hdrl_spectrum1D_interp_cspline);

  hdrl_parameter * resample_par =
      hdrl_spectrum1D_resample_interpolate_parameter_create(hdrl_spectrum1D_interp_linear);
  err =
      hdrl_eop_interpolate(mean_mjd, eop_table, resample_par, &pmx, &pmy, &dut1) ;
  hdrl_parameter_delete(resample_par);

  /* Check output result */
  if (err) {
      return cpl_error_set_message(cpl_func, err, "Could not interpolate the "
	  "Earth Orientation Parameter table");
  }

  cpl_msg_debug(cpl_func, "Using the following Earth Orientation Parameter for "
      "MJD-OBS %g: pmx: %g, pmy: %g, dut1: %g", mean_mjd, pmx, pmy, dut1);

  pmx = pmx / 3600.0 * CPL_MATH_RAD_DEG;
  pmy = pmy / 3600.0 * CPL_MATH_RAD_DEG;

  cpl_msg_debug(cpl_func, "Input to the erfa function eraApco13():");
  cpl_msg_indent_more();
  cpl_msg_debug (cpl_func, "%12s %50s: %20.20g", "utc1  :", "UTC as a 2-part...                              ", 2400000.5);
  cpl_msg_debug (cpl_func, "%12s %50s: %20.20g", "utc2  :", "...quasi Julian Date (Notes 1,2)                ", mean_mjd);
  cpl_msg_debug (cpl_func, "%12s %50s: %20.20g", "dut1  :", "UT1-UTC (seconds, Note 3)                       ", dut1);
  cpl_msg_debug (cpl_func, "%12s %50s: %20.20g", "elong :", "longitude (radians, east +ve, Note 4)           ", longitude);
  cpl_msg_debug (cpl_func, "%12s %50s: %20.20g", "phi   :", "latitude (geodetic, radians, Note 4)            ", latitude);
  cpl_msg_debug (cpl_func, "%12s %50s: %20.20g", "hm    :", "height above ellipsoid (m, geodetic, Notes 4,6) ", elevation);
  cpl_msg_debug (cpl_func, "%12s %50s: %20.20g", "xp    :", "polar motion coordinates (radians, Note 5)      ", pmx);
  cpl_msg_debug (cpl_func, "%12s %50s: %20.20g", "yp    :", "polar motion coordinates (radians, Note 5)      ", pmy);
  cpl_msg_debug (cpl_func, "%12s %50s: %20.20g", "phpa  :", "pressure at the observer (hPa = mB, Note 6)     ", pressure);
  cpl_msg_debug (cpl_func, "%12s %50s: %20.20g", "tc    :", "ambient temperature at the observer (deg C)     ", temperature);
  cpl_msg_debug (cpl_func, "%12s %50s: %20.20g", "rh    :", "relative humidity at the observer (range 0-1)   ", humidity);
  cpl_msg_debug (cpl_func, "%12s %50s: %20.20g", "wl    :", "wavelength (micrometers, Note 7)                ", wavelength);
  cpl_msg_indent_less();
  /* Allocate memory for the tmp computations */
  eraASTROM astrom;
  double eo;
  int eraApco13_status = 0;
  cpl_msg_info(cpl_func, "Calling erfa function eraApco13() ...");
  eraApco13_status = eraApco13 (2400000.5, mean_mjd, dut1,
				longitude, latitude, elevation,
				pmx, pmy,
				pressure, temperature, humidity, wavelength,
				&astrom, &eo);
  if (eraApco13_status < 0) {
      *barycorr = NAN;
      return cpl_error_set_message(cpl_func, CPL_ERROR_UNSPECIFIED,
				   "Erfa function eraApco13() did not succeed in"
				   " computing the barycentric correction");
  }

  cpl_msg_debug(cpl_func, "Output of the erfa function eraApco13():");
  cpl_msg_indent_more();

  cpl_msg_debug (cpl_func, " pmt       : /* PM time interval (SSB, Julian years) */              : %20.20g ", astrom.pmt);
  cpl_msg_debug (cpl_func, " eb[0]     : /* SSB to observer (vector, au) */                      : %20.20g ", astrom.eb[0]);
  cpl_msg_debug (cpl_func, " eb[1]     : /* SSB to observer (vector, au) */                      : %20.20g ", astrom.eb[1]);
  cpl_msg_debug (cpl_func, " eb[2]     : /* SSB to observer (vector, au) */                      : %20.20g ", astrom.eb[2]);
  cpl_msg_debug (cpl_func, " eh[0]     : /* Sun to observer (unit vector) */                     : %20.20g ", astrom.eh[0]);
  cpl_msg_debug (cpl_func, " eh[1]     : /* Sun to observer (unit vector) */                     : %20.20g ", astrom.eh[1]);
  cpl_msg_debug (cpl_func, " eh[2]     : /* Sun to observer (unit vector) */                     : %20.20g ", astrom.eh[2]);
  cpl_msg_debug (cpl_func, " em        : /* distance from Sun to observer (au) */                : %20.20g ", astrom.em);
  cpl_msg_debug (cpl_func, " v[0]      : /* barycentric observer velocity (vector, c) */         : %20.20g ", astrom.v[0]);
  cpl_msg_debug (cpl_func, " v[1]      : /* barycentric observer velocity (vector, c) */         : %20.20g ", astrom.v[1]);
  cpl_msg_debug (cpl_func, " v[2]      : /* barycentric observer velocity (vector, c) */         : %20.20g ", astrom.v[2]);
  cpl_msg_debug (cpl_func, " bm1       : /* sqrt(1-|v|^2): reciprocal of Lorenz factor */        : %20.20g ", astrom.bm1);
  cpl_msg_debug (cpl_func, " bpn[0][0] : /* bias-precession-nutation matrix */                   : %20.20g ", astrom.bpn[0][0]);
  cpl_msg_debug (cpl_func, " along:    : /* longitude + s' + dERA(DUT) (radians) */              : %20.20g ", astrom.along);
  //  phi may be un-initialized according to valgrind - not printing it at this stage
  //  cpl_msg_debug (cpl_func, " phi:      : /* geodetic latitude (radians) */                       : %20.20g ", astrom.phi);
  cpl_msg_debug (cpl_func, " xpl       : /* polar motion xp wrt local meridian (radians) */      : %20.20g ", astrom.xpl);
  cpl_msg_debug (cpl_func, " ypl       : /* polar motion yp wrt local meridian (radians) */      : %20.20g ", astrom.ypl);
  cpl_msg_debug (cpl_func, " sphi      : /* sine of geodetic latitude */                         : %20.20g ", astrom.sphi);
  cpl_msg_debug (cpl_func, " cphi      : /* cosine of geodetic latitude */                       : %20.20g ", astrom.cphi);
  cpl_msg_debug (cpl_func, " diurab    : /* magnitude of diurnal aberration vector */            : %20.20g ", astrom.diurab);
  cpl_msg_debug (cpl_func, " eral      : /* local Earth rotation angle (radians) */              : %20.20g ", astrom.eral);
  cpl_msg_debug (cpl_func, " refa      : /* refraction constant A (radians) */                   : %20.20g ", astrom.refa);
  cpl_msg_debug (cpl_func, " refb      : /* refraction constant B (radians) */                   : %20.20g ", astrom.refb);
  cpl_msg_debug (cpl_func, " eo        : /* equation of the origins (ERA-GST) */                 : %20.20g ", astrom.refb);
  cpl_msg_indent_less();


  /* From UVES Pipeline:
   * ... REFERENCE: THE ASTRONOMICAL ALMANAC 1982 PAGE:B17 */

  *barycorr =
      astrom.v[0]*cos(ra)*cos(dec)+
      astrom.v[1]*sin(ra)*cos(dec)+
      astrom.v[2]*sin(dec);

  *barycorr *= CPL_PHYS_C; // m/s

  return cpl_error_get_code();

}

/**@} */
