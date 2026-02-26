/* $Id: hdrl_spectrum_shift.h,v 0.1 2017-06-06 13:23:28 msalmist Exp $
 *
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

/*
 * $Author: msalmist $
 * $Date: 2017-06-06 13:23:28 $
 * $Revision: 0.1 $
 * $Name: not supported by cvs2svn $
 */

#ifndef HDRL_SPECTRUM_SHIFT_H
#define HDRL_SPECTRUM_SHIFT_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_spectrum.h"

#include <cpl.h>

CPL_BEGIN_DECLS
/*-----------------------------------------------------------------------------
                            		Functions
 -----------------------------------------------------------------------------*/
hdrl_xcorrelation_result *
hdrl_spectrum1D_compute_shift_xcorrelation(const hdrl_spectrum1D * s1,
                              const hdrl_spectrum1D * s2,
                              const cpl_size half_win,
                              const cpl_boolean normalize);
hdrl_parameter *
hdrl_spectrum1D_shift_fit_parameter_create(const hdrl_data_t wguess, const hdrl_data_t range_wmin,
		const hdrl_data_t range_wmax, const hdrl_data_t fit_wmin,
		const hdrl_data_t fit_wmax, const hdrl_data_t fit_half_win);

hdrl_data_t
hdrl_spectrum1D_compute_shift_fit(const hdrl_spectrum1D * obs,
                                  const hdrl_parameter  * par);

CPL_END_DECLS

#endif 
