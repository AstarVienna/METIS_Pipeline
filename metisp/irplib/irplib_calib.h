/* $Id: irplib_calib.h,v 1.8 2007-02-12 10:34:51 amodigli Exp $
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
 * $Author: amodigli $
 * $Date: 2007-02-12 10:34:51 $
 * $Revision: 1.8 $
 * $Name: not supported by cvs2svn $
 */

#ifndef IRPLIB_CALIB_H
#define IRPLIB_CALIB_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>

/*-----------------------------------------------------------------------------
                               Function prototypes
 -----------------------------------------------------------------------------*/

cpl_table*
irplib_compute_gain(
                cpl_frameset* son, 
                cpl_frameset* sof, 
                int* zone1, 
                const int kappa1,
                const int nclip1) CPL_ATTR_ALLOC;

cpl_table* irplib_compute_linearity(cpl_frameset* son, cpl_frameset* sof)
    CPL_ATTR_ALLOC;

int irplib_flat_dark_bpm_calib(cpl_imagelist *, const char *, const char *,
                               const char *);
int irplib_detlin_correct(cpl_imagelist *, const char *, const char *,
                          const char *);

#endif
