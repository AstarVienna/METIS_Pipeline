/*
 * This file is part of the HDRL
 * Copyright (C) 2013 European Southern Observatory
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

#if !defined(_POSIX_C_SOURCE) || (_POSIX_C_SOURCE - 0) < 200112L
#define _POSIX_C_SOURCE 200112L
#endif


/*---------------------------------------------------------------------------*
 *                                   Includes                                *
 *---------------------------------------------------------------------------*/
#include "hdrl_dar.h"

#include <math.h>

/*---------------------------------------------------------------------------*
 *                             Debugging/feature Macros                      *
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
 *                                  Static                                   *
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_dar 	DAR (Differential Atmospheric Refraction)
 *
 * @brief
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/


/** @cond PRIVATE */

/*-----------------------------------------------------------------------------
                        DAR Parameters Definition
 -----------------------------------------------------------------------------*/
typedef struct {
    HDRL_PARAMETER_HEAD;
    hdrl_value airmass;	/* Air mass		                     				 */
    hdrl_value parang;	/* Parallactic angle during exposure				 */
    hdrl_value posang;	/* Position angle on the sky from the angles we have */
    hdrl_value temp;	/* Temperature [Celsius]							 */
    hdrl_value rhum;	/* Relative humidity [%]							 */
    hdrl_value pres;	/* Pressure [mbar]									 */
	cpl_wcs    *wcs;	/* World Coordinate system (WCS) in degrees(CDi_j)	 */
} hdrl_dar_parameter;

/* Parameter type */
static hdrl_parameter_typeobj hdrl_dar_parameter_type = {
    HDRL_PARAMETER_DAR,		                        /* type 	  */
    (hdrl_alloc *)&cpl_malloc,                      /* fp_alloc   */
    (hdrl_free  *)&cpl_free,                        /* fp_free 	  */
    NULL,									 	    /* fp_destroy */
    sizeof(hdrl_dar_parameter),		                /* obj_size   */
};

/*----------------------------------------------------------------------------*/
 /**
  * @brief    Verify basic correctness of the DAR parameters
  *
  * @param    param   Contains all of DAR parameters
  *
  * @return   CPL_ERROR_NONE if everything is ok, an error code otherwise
  *
  **/
 /*----------------------------------------------------------------------------*/
cpl_error_code hdrl_dar_parameter_verify(const hdrl_parameter *param)
{
	cpl_error_ensure(param != NULL, CPL_ERROR_NULL_INPUT,
		return CPL_ERROR_NULL_INPUT, "NULL Input Parameters");

	cpl_error_ensure(hdrl_parameter_check_type(param, &hdrl_dar_parameter_type), CPL_ERROR_ILLEGAL_INPUT,
		return CPL_ERROR_ILLEGAL_INPUT, "Expected DAR parameter");

	const hdrl_dar_parameter *p_loc = (const hdrl_dar_parameter *)param;
	hdrl_value airmass = p_loc->airmass;
	hdrl_value parang  = p_loc->parang;		/* Degrees               */
	hdrl_value posang  = p_loc->posang;		/* Degrees               */
	hdrl_value temp    = p_loc->temp;		/* T [Celsius]           */
	hdrl_value rhum    = p_loc->rhum;		/* Relative humidity [%] */
	hdrl_value pres    = p_loc->pres;		/* Pressure [mbar]       */

	cpl_error_ensure(airmass.data >= 0. && airmass.error >= 0., CPL_ERROR_ILLEGAL_INPUT,
		return CPL_ERROR_ILLEGAL_INPUT, "Airmass parameter not valid");

	cpl_error_ensure(parang.data >= -180. && parang.data <= 180. && parang.error >= 0., CPL_ERROR_ILLEGAL_INPUT,
		return CPL_ERROR_ILLEGAL_INPUT, "Paralactic angle not valid");

	cpl_error_ensure(posang.data >= -360. && posang.data <= 360. && posang.error >= 0., CPL_ERROR_ILLEGAL_INPUT,
		return CPL_ERROR_ILLEGAL_INPUT, "Position angle not valid");

	cpl_error_ensure(temp.data >= -273.15 && temp.error >= 0., CPL_ERROR_ILLEGAL_INPUT,
		return CPL_ERROR_ILLEGAL_INPUT, "Temperature not valid");

	cpl_error_ensure(rhum.data >= 0. && rhum.data <=100 && rhum.error >= 0., CPL_ERROR_ILLEGAL_INPUT,
		return CPL_ERROR_ILLEGAL_INPUT, "Humidity percent value not valid");

	cpl_error_ensure( pres.data >= 0. && pres.error >= 0., CPL_ERROR_ILLEGAL_INPUT,
		return CPL_ERROR_ILLEGAL_INPUT, "Pressure not valid");

	cpl_wcs* wcs = p_loc->wcs;	/* Degrees */

	cpl_error_ensure(wcs != NULL, CPL_ERROR_NULL_INPUT,
		return CPL_ERROR_NULL_INPUT, "NULL WCS Input");

    return CPL_ERROR_NONE;
}

/** @endcond */

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Creates DAR parameters object with the values in the header
 *
 * @param	 airmass	Air mass
 * @param	 parang	 	Parallactic angle during exposure
 * @param	 posang	 	Position angle on the sky from the angles we have
 * @param	 temp	 	Temperature [Celsius]
 * @param	 rhum	    Relative humidity [%]
 * @param	 pres	    Pressure [mbar]
 * @param	 wcs	    World Coordinate system (WCS) in degrees(CDi_j)
 *
 * @return   The dar parameters object. It needs to be deallocated
 *           with hdrl_parameter_delete() or _destroy()
 *
 * @note References:
 * - based on public domain code of the IDL astro-lib procedure getrot.pro
 * - See <a href=http://idlastro.gsfc.nasa.gov/ftp/pro/astrom/getrot.pro>getrot
 * </a> for more info.
 *
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter * hdrl_dar_parameter_create(hdrl_value airmass, hdrl_value parang,
		hdrl_value posang, hdrl_value temp, hdrl_value rhum, hdrl_value pres, cpl_wcs *wcs)
{
	hdrl_dar_parameter *p =	(hdrl_dar_parameter *)hdrl_parameter_new(&hdrl_dar_parameter_type);

    p->airmass = airmass;
	p->parang  = parang;
	p->posang  = posang;
    p->temp    = temp;
	p->rhum    = rhum;
	p->pres    = pres;

	p->wcs = wcs;

	/* After add the parameters, verify if they are correct */
	if (hdrl_dar_parameter_verify((hdrl_parameter *)p) != CPL_ERROR_NONE) {
		hdrl_parameter_delete((hdrl_parameter*)p);
		return NULL;
	}

	return (hdrl_parameter *)p;
}


/*----------------------------------------------------------------------------*/
/**
 * @brief   Correct the pixel coordinates of all pixels of a given pixel table
 *          for differential atmospheric refraction (DAR).
 *
 * @param   params      In:  h_parameter with all of params. in the observation
 * @param   lambdaRef   In:  Reference wavelength (in Angstroms)
 * @param   lambdaIn	In:  One lambda for each plane (in Angstroms)
 * @param   xShift		Out: Correction for each plane in x-axis (pixels)
 * @param   yShift		Out: Correction for each plane in y-axis (pixels)
 * @param   xShiftErr	Out: Error in correction for each plane in x-axis (pix)
 * @param   yShiftErr	Out: Error in correction for each plane in x-axis (pix)
 *
 * @return  CPL_ERROR_NONE on success another CPL error code on failure
 *
 * @remark  The resulting correction can be directly applied to the pixel table.
 *
 * Loop that compute the DAR offset for the wavelength difference with respect
 * to the reference wavelength, and storage the shift in the coordinates,
 * taking into account the instrument rotation angle on the sky and the
 * parallactic angle at the time of the observations.
 *
 * The algorithm from Filippenko (1982, PASP, 94, 715). This only uses the formula
 * from Owens which converts relative humidity to water vapor pressure.
 *
 * @note The calculation is performed by calling the top-level function
 * hdrl_dar_compute() and the parameters passed to this function can be created
 * by calling hdrl_dar_parameter_create().
 *
 * @note  This module contains routines to calculate the refractive index of air.
 * See <a href="http://emtoolbox.nist.gov/Wavelength/Documentation.asp#AppendixA">
 * here</a> for the formulae used.
 *
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_dar_compute(const hdrl_parameter *params,
		const hdrl_value lambdaRef, const cpl_vector *lambdaIn,
		cpl_vector *xShift, cpl_vector *yShift, cpl_vector *xShiftErr, cpl_vector *yShiftErr)
{

	cpl_error_ensure(params && lambdaIn && xShift && yShift != NULL, CPL_ERROR_NULL_INPUT,
		return CPL_ERROR_NULL_INPUT, "NULL Input Parameters");

	/* Check entry values in hdrl_parameter */
	if (hdrl_dar_parameter_verify(params) != CPL_ERROR_NONE) {
		return CPL_ERROR_UNSPECIFIED;
	}

	cpl_error_ensure(lambdaRef.data >= 0, CPL_ERROR_ILLEGAL_INPUT,
		return CPL_ERROR_ILLEGAL_INPUT, "Reference wavelength must be >=0");

	 /* Local Usage Parameters */
	const hdrl_dar_parameter *p_loc = (const hdrl_dar_parameter *)params;
	hdrl_value airmass  = p_loc->airmass;
	hdrl_value parang   = p_loc->parang;		/* Degrees               */
	hdrl_value posang   = p_loc->posang;		/* Degrees               */
	hdrl_value temp     = p_loc->temp;			/* T [Celsius]           */
	hdrl_value rhum     = p_loc->rhum;			/* Relative humidity [%] */
	hdrl_value pres     = p_loc->pres;			/* Pressure [mbar]       */
	cpl_wcs *wcs        = p_loc->wcs;			/* Degrees               */

	/* Check if the airmass is at less 1. */
	cpl_ensure_code(airmass.data >= 1., cpl_error_get_code());

	/* simple zenith distance in radians */
	hdrl_value z = {acos(1. / airmass.data),
	                airmass.error * fabs( (-1. / pow(airmass.data, 2)) / sqrt(1. - pow(1. / airmass.data, 2)) )};

	/* ----------------------------------------------------------------- *
	 * Compute the refractive index at lambdaRef with FILIPPENKO method  *
	 * in um and output properties in "natural" (for the formulae) units *
	 * ----------------------------------------------------------------- */

	/* Calculate temperature and error in Kelvin */
	double temp_kel_data = temp.data + 273.15;
	double temp_kel_err  = (temp.error / fabs(temp.data)) * fabs(temp_kel_data);
	hdrl_value temp_kel  = {temp_kel_data, temp_kel_err};

	/* Use the Owens formula to derive saturation pressure. Needs T[K] */
	hdrl_value sp = hdrl_dar_owens_saturation_pressure(temp_kel);

	/* Conversion from hPa (or mbar) to mmHg, needed for Filippenko *
	 * using that, derive the water vapor pressure in mmHg          */
	double HDRL_PHYS_hPa_TO_mmHg = 0.75006158;

	/* Convert relative humidity [%] to fraction */
	rhum.data  /= 100.;
	rhum.error /= 100.;

	/* water vapor pressure in mmHg */
	hdrl_value fp = {rhum.data *sp.data *HDRL_PHYS_hPa_TO_mmHg,
	                  fabs(HDRL_PHYS_hPa_TO_mmHg * sp.data  ) * rhum.error
			        + fabs(HDRL_PHYS_hPa_TO_mmHg * rhum.data) * sp.error};

	/* need the pressure in mmHg */
	pres.data  *= HDRL_PHYS_hPa_TO_mmHg;
	pres.error *= HDRL_PHYS_hPa_TO_mmHg;

	/* refractive index of air at reference wavelength. Needs lambda[um] */
	hdrl_value lambdaRef_um = {lambdaRef.data * 1e-4,lambdaRef.error * 1e-4};
	hdrl_value nr0 = hdrl_dar_filippenko_refractive_index(lambdaRef_um, pres, temp, fp);

	/* Obtain shift with scale: Absolute Shift for a lambdaRef, xshift is in *
	 * E-W direction for posang = 0, yshift is N-S Shift units --> Degrees   */
	hdrl_value x_shift = {-sin( (parang.data + posang.data) * CPL_MATH_RAD_DEG),
	                       parang.error * fabs(-CPL_MATH_RAD_DEG * cos(parang.data + posang.data))
	                     + posang.error * fabs(-CPL_MATH_RAD_DEG * cos(parang.data + posang.data))};

	hdrl_value y_shift = { cos( (parang.data + posang.data) * CPL_MATH_RAD_DEG),
	                       parang.error * fabs(-CPL_MATH_RAD_DEG * sin(parang.data + posang.data))
	                     + posang.error * fabs(-CPL_MATH_RAD_DEG * sin(parang.data + posang.data))};

	/* Get scale in the world cordinate system (wcs) and apply them */
	double xscale, yscale;
	hdrl_dar_wcs_get_scales(wcs, &xscale, &yscale);

	x_shift.data  /= xscale;
	x_shift.error /= xscale;

	y_shift.data  /= yscale;
	y_shift.error /= yscale;

	/* Diff. refr. base in arcsec converted from radians (Filippenko does *
	 * the conversion using x206265 which converts radians to arcsec)     */
	hdrl_value dr0 = {tan(z.data) * CPL_MATH_DEG_RAD,
	                  z.error * fabs( (1. + pow(tan(z.data), 2)) * CPL_MATH_DEG_RAD)};

	/* ------------------------------------------------------------------ *
	 * Calculate the relative lambda of in array (in)                     *
	 * apply the absolute shift  (x_shift, y_shift) for lamdaRef          *
	 * for obtain the out arrays (xShift,  yShift )                       *
	 * ------------------------------------------------------------------ */
	cpl_size i;
	cpl_size nmax = cpl_vector_get_size(lambdaIn);

	HDRL_OMP(omp parallel for                               		\
				 default(none)                                      \
				 shared(  nmax, lambdaIn, 						    \
						  xShift, yShift, xShiftErr, yShiftErr, 	\
						  lambdaRef_um, pres, temp, fp, dr0, nr0,   \
						  x_shift, y_shift                          ))
	for (i = 0; i < nmax; i++) {

		double lambda = cpl_vector_get(lambdaIn, i);
		if (isfinite(lambda) != 0) {

			hdrl_value lambda_um = {lambda * 1e-4, lambdaRef_um.error};
			hdrl_value nr = hdrl_dar_filippenko_refractive_index(lambda_um, pres, temp, fp);

			hdrl_value dr = {dr0.data  *     (nr0.data - nr.data),
			                 dr0.error * fabs(nr0.data - nr.data)
			               + nr0.error * fabs( dr0.data)
			               + nr.error  * fabs(-dr0.data)};

			hdrl_value shiftPlaneX = {x_shift.data * dr.data,
			                          x_shift.error * fabs(dr.data)
			                        + dr.error      * fabs(x_shift.data)};
			cpl_vector_set(xShift,    i, shiftPlaneX.data );
			cpl_vector_set(xShiftErr, i, shiftPlaneX.error);

			hdrl_value shiftPlaneY = {y_shift.data * dr.data,
			                          y_shift.error * fabs(dr.data)
			                        + dr.error      * fabs(y_shift.data)};
			cpl_vector_set(yShift,    i, shiftPlaneY.data );
			cpl_vector_set(yShiftErr, i, shiftPlaneY.error);

		} else {

			cpl_vector_set(xShift,    i, NAN);
			cpl_vector_set(xShiftErr, i, NAN);

			cpl_vector_set(yShift,    i, NAN);
			cpl_vector_set(yShiftErr, i, NAN);
		}
	}

	return CPL_ERROR_NONE;
}


/*----------------------------------------------------------------------------*/
/**
 * @brief   Compute the saturation pressure using the Owens calibration.
 *
 * @param   hvT   temperature (in Kelvin) with it error associated (in Celsius)
 *
 * @return  the saturation pressure for the given temperature with it error propagation
 *
 * \f[
 * s_p = -10474 +116.43\ T -0.43284\ T^2 +0.00053840\ T^3
 * \f]
 * where T is the temperature.
 *
 * @note This function is used for the Filippenko formulae.
 *
 */
/*----------------------------------------------------------------------------*/
hdrl_value hdrl_dar_owens_saturation_pressure(hdrl_value hvT)
{
  double T      = hvT.data;
  double errorT = hvT.error;

  return (hdrl_value){-10474.0 + 116.43 * T - 0.43284 * T * T + 0.00053840 * pow(T, 3),
      	  	  	  	  errorT * fabs(0.0016152 * T * T - 0.86568 * T + 116.43)};
}

/*----------------------------------------------------------------------------*/
/**
 * @brief   Compute the refractive index for the given wavelength following
 *          Filippenko formulae. This function is called by hdrl_dar_compute().
 *
 * @param   hvL    	the wavelength (in um) with it error associated
 * @param   hvP    	atmospheric pressure (in mmHg) with it error associated
 * @param   hvT    	temperature (in degrees Celsius) with it error associated
 * @param   hvF    	water vapor pressure (in mmHg) with it error associated
 *
 * @return  The refractive index with it error propagation
 *
 * At sea level (P=760 mm Hg, T = 15 \f$^oC\f$) the refractive index of dry air is given
 * by (Edlen 1953; Coleman, Bozman, and Meggers 1960):
 * \f[
 * (n( \lambda )_{15,760}-1)10^6 = 64.328 + \frac{29498.1}{146-(1/ \lambda )^2} +\frac{255.4}{41-(1/ \lambda )^2}
 * \f]
 * where \f$\lambda\f$ is the wavelength of light in vacue (microns). Since observatories
 * are usually located at high altitudes, the index of refraction must be corrected
 * for the lower ambient temperature and pressure (Barrell 1951):
 * \f[
 * (n(\lambda)_{T,P} -1) = (n(\lambda)_{15,760} - 1) \cdot
 * \frac{P[1+(1.049-0.0157\ T) 10^{-6}\ P]}{720.883 (1+0.003661\ T)}
 * \f]
 * In addition, the presence of water vapor in the atmosphere reduces \f$(n-1)10^6\f$ by:
 * \f[
 * \frac{0.0624-0.000680/\lambda^2}{1 + 0.003661\ T} f
 * \f]
 * here \f$f\f$ is the water vapor pressure in mm of Hg and T is the air temperature
 * in \f$^oC\f$ (Barrell 1951).
 * \f[
 * f = 0.75006158 \cdot s_p \cdot h
 * \f]
 * where \f$s_p\f$ is the saturation pressure with Owens calibration and h is the fraction
 * of humidity in [%].
 */
/*----------------------------------------------------------------------------*/
hdrl_value hdrl_dar_filippenko_refractive_index(
		hdrl_value hvL, hdrl_value hvP, hdrl_value hvT, hdrl_value hvF)
{
	 double l      = hvL.data,
	        P      = hvP.data,
	        T      = hvT.data,
	        f      = hvF.data;

	 double errorL = hvL.error,
			errorP = hvP.error,
			errorT = hvT.error,
			errorF = hvF.error;

	/* inverse square of the wavelength */
    double lisq = 1. / (l * l);
    double errorLisq  = errorL * fabs(-2. / pow(l, 3) );

    /* 10^6 [n(lambda) - 1] at standard environmental conditions, Eq. (1) */
    double nl1        = 64.328 + 29498.1 / (146. - lisq) + 255.4 / (41. - lisq);
    double errorNl1   = errorLisq * fabs( 29498.1 / pow(146. - lisq, 2) + 255.4 / pow(41. - lisq, 2) );

    /* correction for non-standard conditions, Eq. (2) */
    double factor     = 1.e-6;
    double nl2A       = nl1 * ( P / 720.883 * (1. + (1.049 -0.0157 * T) * 1e-6 * P) / (1. + 0.003661 * T) );
    double errorNl2A1 = errorNl1 * fabs( factor *(P / 720.883 * (1. + (1.049 - 0.0157 * T) * 1e-6 * P) / (1. + 0.003661 * T) ) );
    double errorNl2A2 = errorP   * fabs( factor *(nl1 / (720.883 * (1. + 0.003661 * T)) *( (1. + (1.049 - 0.0157 * T) * 1e-6 * P) + P * (1.049 - 0.0157 * T) * 1e-6) ) );
    double errorNl2A3 = errorT   * fabs( factor *(nl1 * P / 720.883 * ( ( -0.0157 * 1e-6 * P * (1. + 0.003661 * T) - 0.003661 * (1. + (1.049 - 0.0157 * T) * 1e-6 * P) )/pow(1. + 0.003661 * T, 2) ) ) );
    double errorNl2A  = errorNl2A1 + errorNl2A2 + errorNl2A3;

    /* Calcule correction for water vapor, Eq. (3) */
    double nl2B       = (0.0624 - 0.000680 * lisq) / (1. + 0.003661 * T) * f;
    double errorNl2B1 = errorLisq * fabs( -0.000680 * f / (1. + 0.003661 * T) );
    double errorNl2B2 = errorT    * fabs( -0.003661 * (0.0624 - 0.000680 * lisq) * f / pow(1. + 0.003661 * T, 2) );
    double errorNl2B3 = errorF    * fabs( (0.0624 - 0.000680 * lisq) / (1. + 0.003661 * T)  );
    double errorNl2B  = errorNl2B1 + errorNl2B2 + errorNl2B3;

    /* Apply correction for water vapor, Eq. (3) */
	double nl2        = nl2A - nl2B;
    double errorNl2   = errorNl2A + errorNl2B;

    /* convert to refractive index n(lambda) */
    double nl3        = nl2 * 1e-6 + 1.;
    double errorNl3   = fabs(errorNl2 * 1e-6);

	return (hdrl_value){nl3,errorNl3};
}


/** @cond PRIVATE */

/*----------------------------------------------------------------------------*/
/**
 * @brief   Compute the spatial scales (in degrees) from the FITS header WCS.
 *
 * @param   header   the input header containing the WCS of the exposure
 * @param   aXScale   the output scale in x-direction
 * @param   aYScale   the output scale in y-direction
 *
 * @return  CPL_ERROR_NONE for success, any other value for failure
 *
 * The world coordinate system from header, i.e. the CDi_j matrix, is used to
 * compute the scales.
 *
 * @note References:
 * - based on public domain code of the IDL astro-lib procedure getrot.pro
 * - http://idlastro.gsfc.nasa.gov/ftp/pro/astrom/getrot.pro
 *
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_dar_wcs_get_scales(
	cpl_wcs *wcs,
	double  *aXScale,
	double  *aYScale)
{
  cpl_ensure_code(aXScale && aYScale, CPL_ERROR_NULL_INPUT);

  cpl_errorstate prestate = cpl_errorstate_get();

  const cpl_matrix *cd = cpl_wcs_get_cd(wcs);

  /* take the absolute and scale by 3600 to get positive arcseconds */
  double cd11 = cpl_matrix_get(cd, 0, 0),
         cd12 = cpl_matrix_get(cd, 0, 1),
         cd21 = cpl_matrix_get(cd, 1, 0),
         cd22 = cpl_matrix_get(cd, 1, 1);

  double det = cd11 * cd22 - cd12 * cd21;
  cpl_ensure_code(cpl_errorstate_is_equal(prestate), cpl_error_get_code());

  if (det < 0.) {
    cd12 *= -1.;
    cd11 *= -1.;
  }

  /* matrix without rotation */
  if (cd12 == 0. && cd21 == 0.) {
    *aXScale = cd11;
    *aYScale = cd22;
    return CPL_ERROR_NONE;
  }

  /* Only the absolute value */
  *aXScale = sqrt(cd11 * cd11 + cd12 * cd12);
  *aYScale = sqrt(cd22 * cd22 + cd21 * cd21);

  return CPL_ERROR_NONE;
}

/** @endcond */

/**@}*/
