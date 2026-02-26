/* $Id: irplib_wcs.h,v 1.7 2010-10-07 14:10:55 llundin Exp $
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
 * $Author: llundin $
 * $Date: 2010-10-07 14:10:55 $
 * $Revision: 1.7 $
 * $Name: not supported by cvs2svn $
 */

#ifndef IRPLIB_WCS_H
#define IRPLIB_WCS_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>

/*-----------------------------------------------------------------------------
                               Function prototypes
 -----------------------------------------------------------------------------*/

cpl_error_code irplib_wcs_xytoradec(const cpl_wcs * wcs,
                                    double          x,
                                    double          y,
                                    double        * ra,
                                    double        * dec);

cpl_error_code irplib_wcs_radectoxy(const cpl_wcs * wcs,
                                    double          ra,
                                    double          dec,
                                    double        * x,
                                    double        * y);

double irplib_wcs_great_circle_dist(double ra1,
                                    double dec1,
                                    double ra2,
                                    double dec2);


cpl_error_code irplib_wcs_iso8601_from_string(int *, int *, int *, int *, int *,
                                              double *, const char *);

cpl_error_code irplib_wcs_mjd_from_iso8601(double *, int, int, int, int, int,
                                           double);

cpl_error_code irplib_wcs_mjd_from_string(double *, const char *);

cpl_error_code irplib_wcs_iso8601_from_mjd(int *, int *, int *, int *, int *,
                                           double *, double);

#endif
