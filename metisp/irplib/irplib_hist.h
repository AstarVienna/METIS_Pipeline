/* $Id: irplib_hist.h,v 1.3 2007-09-07 14:23:50 lbilbao Exp $
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
 * $Author: lbilbao $
 * $Date: 2007-09-07 14:23:50 $
 * $Revision: 1.3 $
 * $Name: not supported by cvs2svn $
 */

#ifndef IRPLIB_HIST_H
#define IRPLIB_HIST_H

#include <cpl.h>

typedef struct _irplib_hist_ irplib_hist;

/* Creation/Destruction functions */

irplib_hist *
irplib_hist_new(void);

void
irplib_hist_delete(irplib_hist *);

/* Initialisation function */

cpl_error_code
irplib_hist_init(irplib_hist   *,
                 unsigned long  ,
                 double         ,
                 double         );

/* Accessor functions */

unsigned long
irplib_hist_get_value(const irplib_hist *,
                      const unsigned long);

unsigned long
irplib_hist_get_nbins(const irplib_hist *);

double
irplib_hist_get_bin_size(const irplib_hist *);

double
irplib_hist_get_range(const irplib_hist *);

double
irplib_hist_get_start(const irplib_hist *);

/* Histogram computing function */

cpl_error_code
irplib_hist_fill(irplib_hist     *,
                 const cpl_image *);

/* Statistics functions */

unsigned long
irplib_hist_get_max(const irplib_hist *,
                    unsigned long     *);

/* Casting function */

cpl_table *
irplib_hist_cast_table(const irplib_hist *);

/* Functions for operations on histograms */

cpl_error_code
irplib_hist_collapse(irplib_hist *,
                     unsigned long);

#endif /* IRPLIB_HIST_H */

