/* $Id: irplib_plugin.h,v 1.23 2012-01-11 08:03:37 llundin Exp $
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
 * $Date: 2012-01-11 08:03:37 $
 * $Revision: 1.23 $
 * $Name: not supported by cvs2svn $
 */

#ifndef IRPLIB_PLUGIN_H
#define IRPLIB_PLUGIN_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>

/*-----------------------------------------------------------------------------
                                   Function prototypes
 -----------------------------------------------------------------------------*/

int irplib_plugin_test(cpl_pluginlist *, size_t, const char *[]);

cpl_error_code irplib_parameterlist_set_string(cpl_parameterlist *,
                                               const char *, const char *,
                                               const char *, const char *, 
                                               const char *, const char *,
                                               const char *);

cpl_error_code irplib_parameterlist_set_bool(cpl_parameterlist *,
                                             const char *, const char *,
                                             const char *, cpl_boolean,
                                             const char *, const char *,
                                             const char *);

cpl_error_code irplib_parameterlist_set_int(cpl_parameterlist *,
                                            const char *, const char *,
                                            const char *, int,
                                            const char *, const char *,
                                            const char *);

cpl_error_code irplib_parameterlist_set_double(cpl_parameterlist *,
                                               const char *, const char *,
                                               const char *, double, 
                                               const char *, const char *,
                                               const char *);

const char * irplib_parameterlist_get_string(const cpl_parameterlist *,
                                             const char *, const char *,
                                             const char *);

cpl_boolean irplib_parameterlist_get_bool(const cpl_parameterlist *,
                                          const char *, const char *,
                                          const char *);

int irplib_parameterlist_get_int(const cpl_parameterlist *,
                                 const char *, const char *,
                                 const char *);

double irplib_parameterlist_get_double(const cpl_parameterlist *,
                                       const char *, const char *,
                                       const char *);

#endif
