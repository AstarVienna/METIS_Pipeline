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

#ifndef HDRL_DAR_H_
#define HDRL_DAR_H_

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/
#include "hdrl_parameter.h"

#include <cpl.h>


CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                        DAR Parameters Definition
 -----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                                Defines
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
 *                          Special variable types                            *
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
 *                          Struct variable types                            *
 *----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
                            Parameters
  ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
                            Computation
  ----------------------------------------------------------------------------*/

hdrl_parameter * hdrl_dar_parameter_create(hdrl_value airmass, hdrl_value parang,
	hdrl_value posang, hdrl_value temp, hdrl_value rhum, hdrl_value pres, cpl_wcs *wcs);

cpl_error_code hdrl_dar_compute(const hdrl_parameter *params,
	const hdrl_value lambdaRef, const cpl_vector *lambdaIn,
	cpl_vector *xShift, cpl_vector *yShift, cpl_vector *xShiftErr, cpl_vector *yShiftErr);

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
 -----------------------------------------------------------------------------*/

#ifdef HDRL_USE_PRIVATE

	cpl_error_code hdrl_dar_parameter_verify(const hdrl_parameter *param);

    hdrl_value hdrl_dar_owens_saturation_pressure(hdrl_value hvT);

    hdrl_value hdrl_dar_filippenko_refractive_index(
    		hdrl_value hvL, hdrl_value hvP, hdrl_value hvT, hdrl_value hvF);

    cpl_error_code hdrl_dar_wcs_get_scales(
    		cpl_wcs *wcs, double *aXScale, double *aYScale);

#endif

CPL_END_DECLS

#endif /* HDRL_DAR_H_ */
