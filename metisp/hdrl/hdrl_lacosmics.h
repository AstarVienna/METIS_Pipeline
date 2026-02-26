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

#ifndef HDRL_LACOSMICS_H_
#define HDRL_LACOSMICS_H_

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_types.h"
#include "hdrl_image.h"
#include <cpl.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
                            LaCosmic Parameters
  ----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_lacosmic_parameter_create(double, double, int);
cpl_error_code hdrl_lacosmic_parameter_verify(const hdrl_parameter *) ;
cpl_boolean hdrl_lacosmic_parameter_check(const hdrl_parameter *) ;

/* Accessors */
double hdrl_lacosmic_parameter_get_sigma_lim(const hdrl_parameter *) ;
double hdrl_lacosmic_parameter_get_f_lim(const hdrl_parameter *) ;
int hdrl_lacosmic_parameter_get_max_iter(const hdrl_parameter *) ;

/* Parameter Lists */
cpl_parameterlist * hdrl_lacosmic_parameter_create_parlist(const char *, 
        const char *, const hdrl_parameter *);
hdrl_parameter * hdrl_lacosmic_parameter_parse_parlist(
        const cpl_parameterlist *, const char *) ;

/*----------------------------------------------------------------------------
                            LaCosmic Computation
  ----------------------------------------------------------------------------*/

cpl_mask * hdrl_lacosmic_edgedetect(
        const hdrl_image        *   ima_in,
        const hdrl_parameter    *   params) ;

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
 -----------------------------------------------------------------------------*/

#ifdef HDRL_USE_PRIVATE

#endif

CPL_END_DECLS

#endif /* HDRL_LACOSMICS_H_ */
