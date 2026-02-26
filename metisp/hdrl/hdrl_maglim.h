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

#ifndef HDRL_MAGLIM_H
#define HDRL_MAGLIM_H
#include <cpl.h>
#include <hdrl_parameter.h>
CPL_BEGIN_DECLS
/*-----------------------------------------------------------------------------
                                   New types
 -----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/
cpl_error_code
hdrl_maglim_compute(const cpl_image * image,
			  const double zeropoint,
			  const double fwhm,
			  const cpl_size kernel_size_x,
			  const cpl_size kernel_size_y,
			  const hdrl_image_extend_method image_extend_method,
			  const hdrl_parameter * mode_parameter,
			  double* limiting_magnitude);

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
 -----------------------------------------------------------------------------*/

CPL_END_DECLS

#endif
