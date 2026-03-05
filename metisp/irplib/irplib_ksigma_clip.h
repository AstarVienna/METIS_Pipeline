/* $Id: irplib_ksigma_clip.h,v 1.1 2011-11-02 13:18:28 amodigli Exp $
 *
 * This file is part of the irplib package
 * Copyright (C) 2002, 2003 European Southern Observatory
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02111-1307 USA
 */

/*
 * $Author: amodigli $
 * $Date: 2011-11-02 13:18:28 $
 * $Revision: 1.1 $
 * $Name: not supported by cvs2svn $
 */

#ifndef IRPLIB_KSIGMA_CLIP_H
#define IRPLIB_KSIGMA_CLIP_H

/*----------------------------------------------------------------------------
                                   Includes
 ----------------------------------------------------------------------------*/

#include <cpl.h>

/*----------------------------------------------------------------------------
                                   Prototypes
 ----------------------------------------------------------------------------*/
cpl_error_code
irplib_ksigma_clip(const cpl_image *,
		   const int,
		   const int,
		   const int,
		   const int,
		   const double,
		   const int,
		   const double,
		   double *,
		   double *);

#endif
