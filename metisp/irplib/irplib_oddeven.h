/* $Id: irplib_oddeven.h,v 1.4 2006-10-06 20:46:04 yjung Exp $
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
 * $Date: 2006-10-06 20:46:04 $
 * $Revision: 1.4 $
 * $Name: not supported by cvs2svn $
 */

#ifndef IRPLIB_ODDEVEN_H
#define IRPLIB_ODDEVEN_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>

int irplib_oddeven_monitor(const cpl_image *, int, double *) ;
cpl_image * irplib_oddeven_correct(const cpl_image *) ;

#endif
