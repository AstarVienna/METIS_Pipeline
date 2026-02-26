/* $Id: hdrl_efficiency.h,v 0.1 2017-03-15 11:55:00 msalmist Exp $
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
 * $Date: 2017-03-15 11:55:00 $
 * $Revision: 0.1 $
 * $Name: not supported by cvs2svn $
 */

#ifndef HDRL_EFFICIENCY_H
#define HDRL_EFFICIENCY_H

/*-----------------------------------------------------------------------------
                                   New types
 -----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_spectrum.h"

#include <cpl.h>

CPL_BEGIN_DECLS


/*-----------------------------------------------------------------------------
                                   Functions
 -----------------------------------------------------------------------------*/

hdrl_parameter* hdrl_efficiency_parameter_create(
        const hdrl_value Ap, const hdrl_value Am, const hdrl_value G,
        const hdrl_value Tex, const hdrl_value Atel);

hdrl_spectrum1D * hdrl_efficiency_compute(
        const hdrl_spectrum1D * I_std_arg, const hdrl_spectrum1D * I_std_ref,
        const hdrl_spectrum1D * E_x, const hdrl_parameter * pars);


hdrl_parameter* hdrl_response_parameter_create(
        const hdrl_value Ap, const hdrl_value Am, const hdrl_value G,
        const hdrl_value Tex);

#if defined HDRL_USE_EXPERIMENTAL || defined HDRL_USE_PRIVATE

    hdrl_value E_ph(hdrl_data_t lambda);

    hdrl_spectrum1D *
    hdrl_response_core_compute(
    const hdrl_spectrum1D * I_std_arg, const hdrl_spectrum1D * I_std_ref,
    const hdrl_spectrum1D * E_x, const hdrl_parameter * pars);

#endif

CPL_END_DECLS

#endif 
