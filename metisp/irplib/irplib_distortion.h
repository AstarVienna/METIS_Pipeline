/* $Id: irplib_distortion.h,v 1.6 2013-01-29 08:43:33 jtaylor Exp $
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
 * $Author: jtaylor $
 * $Date: 2013-01-29 08:43:33 $
 * $Revision: 1.6 $
 * $Name: not supported by cvs2svn $
 */

#ifndef IRPLIB_DISTORTION_H
#define IRPLIB_DISTORTION_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>

/*-----------------------------------------------------------------------------
                               Function prototypes
 -----------------------------------------------------------------------------*/

cpl_polynomial * irplib_distortion_estimate(const cpl_image *, int, int, int, 
        int, int, int, int, double, int, cpl_apertures **) ;

#endif
