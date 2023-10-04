/* $Id: irplib_cat.h,v 1.5 2009-12-16 14:49:52 cgarcia Exp $
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
 * $Author: cgarcia $
 * $Date: 2009-12-16 14:49:52 $
 * $Revision: 1.5 $
 * $Name: not supported by cvs2svn $
 */

#ifndef IRPLIB_CAT_H
#define IRPLIB_CAT_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>

/*-----------------------------------------------------------------------------
                               Function prototypes
 -----------------------------------------------------------------------------*/

int irplib_2mass_get_catpars
(const cpl_frame *master_index,
 char            **catpath,
 char            **catname);

cpl_table *  irplib_2mass_extract
(char *path,
 float ramin,
 float ramax,
 float decmin,
 float decmax);

cpl_error_code irplib_cat_get_image_limits
(const cpl_wcs    * wcs,
 float              ext_search,
 double           * ra1,
 double           * ra2,
 double           * dec1,
 double           * dec2);

#endif
