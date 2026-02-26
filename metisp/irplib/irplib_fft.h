/* $Id: irplib_fft.h,v 1.18 2012-08-03 21:05:33 llundin Exp $
 *
 * This file is part of the IRPLIB Pipeline
 * Copyright (C) 2002,2003 European Southern Observatory
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02111-1307  USA
 */

/*
 * $Author: llundin $
 * $Date: 2012-08-03 21:05:33 $
 * $Revision: 1.18 $
 * $Name: not supported by cvs2svn $
 */

#ifndef IRPLIB_FFT_H
#define IRPLIB_FFT_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>

/*-----------------------------------------------------------------------------
                                   Function Prototypes
 -----------------------------------------------------------------------------*/

cpl_error_code irplib_image_find_shift(const cpl_image *, const cpl_image *,
                                       double *, double *);
#endif
