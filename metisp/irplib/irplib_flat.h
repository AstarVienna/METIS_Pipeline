/* $Id: irplib_flat.h,v 1.4 2005-09-15 11:47:16 llundin Exp $
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
 * $Date: 2005-09-15 11:47:16 $
 * $Revision: 1.4 $
 * $Name: not supported by cvs2svn $
 */

#ifndef IRPLIB_FLAT_H
#define IRPLIB_FLAT_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>

cpl_imagelist * irplib_flat_fit_set(cpl_imagelist *, int) ;
double * irplib_flat_fit_slope_robust(double *, double *, int) ;

#endif
