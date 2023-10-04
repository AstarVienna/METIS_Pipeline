/* $Id: hdrl_image_defs.h,v 1.0 2017-03-27 15:26:45 msalmist Exp $
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
 * $Date: 2017-03-27 15:26:45 $
 * $Revision: 1.0 $
 * $Name: not supported by cvs2svn $
 */

#ifndef HDRL_SPECTRUM_DEFS_H
#define HDRL_SPECTRUM_DEFS_H

#include "hdrl_types.h"
#include "hdrl_image.h"
#include <cpl.h>

CPL_BEGIN_DECLS


typedef enum{
    hdrl_spectrum1D_wave_scale_linear,
    hdrl_spectrum1D_wave_scale_log
}hdrl_spectrum1D_wave_scale;


typedef struct{
    hdrl_image * flux;
    cpl_array * wavelength;
    hdrl_spectrum1D_wave_scale wave_scale;
}hdrl_spectrum1D;

CPL_END_DECLS

#endif
