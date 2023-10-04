/* $Id: irplib_stdstar.h,v 1.16 2013-02-27 10:37:52 llundin Exp $
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
 * $Date: 2013-02-27 10:37:52 $
 * $Revision: 1.16 $
 * $Name: not supported by cvs2svn $
 */

#ifndef IRPLIB_STDSTAR_H
#define IRPLIB_STDSTAR_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>

/*-----------------------------------------------------------------------------
                                   Defines
 -----------------------------------------------------------------------------*/

#define     IRPLIB_STDSTAR_STAR_COL     "STARS"
#define     IRPLIB_STDSTAR_TYPE_COL     "SP_TYPE"
#define     IRPLIB_STDSTAR_RA_COL       "RA"
#define     IRPLIB_STDSTAR_DEC_COL      "DEC"
#define     IRPLIB_STDSTAR_CAT_COL      "CATALOG"

/* Maximum allowed distance [arc minutes] between observation and
   catalogue coordinates  */
#define     IRPLIB_STDSTAR_MAXDIST       2.0

/* Magical value to indicate an invalid magnitude  */
#define     IRPLIB_STDSTAR_NOMAG         99.0

/* Use this limit in comparisons regarding an invalid magnitude  */
#define     IRPLIB_STDSTAR_LIMIT         (IRPLIB_STDSTAR_NOMAG-1.0)

/*-----------------------------------------------------------------------------
                                   Function prototypes
 -----------------------------------------------------------------------------*/

cpl_error_code
irplib_stdstar_write_catalogs(cpl_frameset *, const cpl_frameset *,
                              const char *, const char *, const char *,
                              const char *, const char *,
                              cpl_table * (*)(const char *));
cpl_table * irplib_stdstar_load_catalog(const char *, const char *);
cpl_error_code irplib_stdstar_check_columns_exist(const cpl_table  *);
int irplib_stdstar_select_stars_dist(cpl_table *, double, double, double);
int irplib_stdstar_select_stars_mag(cpl_table *, const char *);
int irplib_stdstar_find_closest(const cpl_table *, double, double);
cpl_error_code irplib_stdstar_find_star(const char *, double, double, const char *, 
        const char *, double *, char **, char **, char **, 
        double *, double  * , double);
cpl_vector * irplib_stdstar_get_conversion(const cpl_bivector *, double, double,
        double, double);
cpl_vector * irplib_stdstar_get_mag_zero(const cpl_bivector *,
        const cpl_vector *, double);
cpl_bivector * irplib_stdstar_get_sed(const char *, const char *);

#endif
