/* $Id: hdrl_spectrum1Dlist.h,v 0.1 2017-04-26 18:30:28 msalmist Exp $
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
 * $Date: 2017-04-26 18:30:28 $
 * $Revision: 0.1 $
 * $Name: not supported by cvs2svn $
 */

#ifndef HDRL_SPECTRUMLIST_H
#define HDRL_SPECTRUMLIST_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_spectrum.h"

#include <cpl.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                              Data Structures
 -----------------------------------------------------------------------------*/
typedef struct{
    cpl_size length; /*Number of valid spectra*/
    cpl_size  capacity; /*Number of spectra that can fit in the internal buffer*/
    hdrl_spectrum1D ** spectra; /*Internal buffer*/
}hdrl_spectrum1Dlist;

/*-----------------------------------------------------------------------------
                              Functions
 -----------------------------------------------------------------------------*/

hdrl_spectrum1Dlist * hdrl_spectrum1Dlist_new(void);

hdrl_spectrum1Dlist * hdrl_spectrum1Dlist_duplicate(const hdrl_spectrum1Dlist * l);

hdrl_spectrum1D *
hdrl_spectrum1Dlist_get(hdrl_spectrum1Dlist *, const cpl_size);

const hdrl_spectrum1D *
hdrl_spectrum1Dlist_get_const(const hdrl_spectrum1Dlist *, const cpl_size);

cpl_error_code
hdrl_spectrum1Dlist_set(hdrl_spectrum1Dlist *, hdrl_spectrum1D *, const cpl_size);

hdrl_spectrum1D *
hdrl_spectrum1Dlist_unset(hdrl_spectrum1Dlist *, const cpl_size);

void hdrl_spectrum1Dlist_delete(hdrl_spectrum1Dlist *);

cpl_size
hdrl_spectrum1Dlist_get_size(const hdrl_spectrum1Dlist *);

#if defined HDRL_USE_EXPERIMENTAL || defined HDRL_USE_PRIVATE

hdrl_spectrum1Dlist *
hdrl_spectrum1Dlist_wrap(hdrl_spectrum1D ** self, const cpl_size sz);

cpl_error_code
hdrl_spectrum1Dlist_collapse(const hdrl_spectrum1Dlist *list,
         const hdrl_parameter * stacking_par,
         const cpl_array * wlengths, const hdrl_parameter * resample_par,
         const cpl_boolean mark_bpm_in_interpolation,
         hdrl_spectrum1D ** result,  cpl_image ** contrib,
         hdrl_imagelist ** resampled_and_aligned_fluxes);

#endif

CPL_END_DECLS

#endif 
