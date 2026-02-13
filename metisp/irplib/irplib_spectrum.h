/* $Id: irplib_spectrum.h,v 1.7 2009-07-30 12:38:37 yjung Exp $
 *
 * This file is part of the irplib package
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
 * $Author: yjung $
 * $Date: 2009-07-30 12:38:37 $
 * $Revision: 1.7 $
 * $Name: not supported by cvs2svn $
 */

#ifndef IRPLIB_SPECTRUM_H
#define IRPLIB_SPECTRUM_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>

/*-----------------------------------------------------------------------------
                                New types
 -----------------------------------------------------------------------------*/

typedef enum SPEC_SHADOWS {
    /* 2 shadows above and below true spectrum */
    TWO_SHADOWS,
    /* 1 shadow at specified distance from spectrum */
    ONE_SHADOW,
    /* Do not search for shadow */
    NO_SHADOW
} spec_shadows ;

/*-----------------------------------------------------------------------------
                               Function prototypes
 -----------------------------------------------------------------------------*/

int irplib_spectrum_find_brightest(const cpl_image *, int, spec_shadows, 
        double, int, double *) ;
cpl_vector * irplib_spectrum_detect_peaks(const cpl_vector *, int,
        double, int, cpl_vector **, cpl_vector **) ;

#endif
