/*
 * hdrl_barycorr.h
 *
 *  Created on: Jan 31, 2022
 *      Author: agabasch
 */

/*
 * This file is part of the ESO Toolkit
 * Copyright (C) 2016 European Southern Observatory
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


#ifndef HDRL_BARYCORR_H_
#define HDRL_BARYCORR_H_

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "hdrl.h"
#include <cpl.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                                Functions prototypes
 -----------------------------------------------------------------------------*/

cpl_error_code
hdrl_barycorr_compute(double ra, double dec, const cpl_table * eop_table,
		       double mjdobs, double time_to_mid_exposure,
		       double longitude, double latitude, double elevation,
		       double pressure, double temperature, double humidity,
		       double  wavelength, double * barycorr);

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
 -----------------------------------------------------------------------------*/

#ifdef HDRL_USE_PRIVATE

/* Helper Functions */

cpl_error_code
hdrl_eop_interpolate(double mjd, const cpl_table * eop_table, hdrl_parameter *
		      resample_par, double *pmx, double *pmy, double *dut);
#endif

CPL_END_DECLS


#endif /* HDRL_BARYCORR_H_ */
