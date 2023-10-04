/* $Id: hdrl_DER_SNR.h,v 0.1 2017-03-02 15:38:44 msalmist Exp $
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
 * $Date: 2017-03-02 15:38:44 $
 * $Revision: 0.1 $
 * $Name: not supported by cvs2svn $
 */

#ifndef HDRL_DER_SNR_H
#define HDRL_DER_SNR_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>
#include "hdrl_types.h"

CPL_BEGIN_DECLS

hdrl_error_t estimate_noise_window(const hdrl_data_t * flux, const cpl_binary * msk,
        cpl_size start, cpl_size stop, const cpl_size sz);

cpl_image *
estimate_noise_DER_SNR(const hdrl_data_t * flux_in, const cpl_binary * msk_in,
        const cpl_array * wavelengths, const cpl_size length,
        const cpl_size half_window);

CPL_END_DECLS

#endif 
