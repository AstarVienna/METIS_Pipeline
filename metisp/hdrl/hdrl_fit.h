/*
 * This file is part of the HDRL
 * Copyright (C) 2014 European Southern Observatory
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

#ifndef HDRL_FIT_H
#define HDRL_FIT_H

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/
#include "hdrl_imagelist.h"
#include <cpl.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

cpl_error_code
hdrl_fit_polynomial_imagelist(const hdrl_imagelist * list,
                              const cpl_vector * samplepos,
                              const int degree,
                              hdrl_imagelist ** coef,
                              cpl_image ** chi2,
                              cpl_image ** dof);

#if defined HDRL_USE_EXPERIMENTAL || defined HDRL_USE_PRIVATE
/** @cond EXPERIMENTAL */

cpl_error_code
hdrl_fit_polynomial_imagelist2(const hdrl_imagelist * list,
                               const cpl_imagelist * samplepos,
                               const int degree,
                               hdrl_imagelist ** coef,
                               cpl_image ** chi2,
                               cpl_image ** dof);

/** @endcond */
#endif

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
 -----------------------------------------------------------------------------*/

#ifdef HDRL_USE_PRIVATE

#endif

CPL_END_DECLS

#endif
