/* $Id: irplib_match_cats.h,v 1.5 2009-12-18 10:44:48 cgarcia Exp $
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
 * $Date: 2009-12-18 10:44:48 $
 * $Revision: 1.5 $
 * $Name: not supported by cvs2svn $
 */

#ifndef IRPLIB_MATCH_CATS_H
#define IRPLIB_MATCH_CATS_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/

cpl_table * irplib_match_cat_pairs
(cpl_table ** catalogues,
 int          nCats,
 int (*binary_match_condition)
   (cpl_table * catalogue1,
    cpl_table * catalogue2,
    int         iobj1,
    int         iobj2)  ) CPL_ATTR_ALLOC;

#endif
