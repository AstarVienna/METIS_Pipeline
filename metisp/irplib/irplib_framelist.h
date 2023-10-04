/* $Id: irplib_framelist.h,v 1.8 2008-11-20 10:24:47 llundin Exp $
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
 * $Date: 2008-11-20 10:24:47 $
 * $Revision: 1.8 $
 * $Name: not supported by cvs2svn $
 */

#ifndef IRPLIB_FRAMELIST_H
#define IRPLIB_FRAMELIST_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>

/*-----------------------------------------------------------------------------
                                   New type
 -----------------------------------------------------------------------------*/

typedef struct _irplib_framelist_ irplib_framelist;

/*-----------------------------------------------------------------------------
                                   Function prototypes
 -----------------------------------------------------------------------------*/

/* Constructors and destructor */

irplib_framelist * irplib_framelist_new(void);

void irplib_framelist_delete(irplib_framelist *);

irplib_framelist * irplib_framelist_cast(const cpl_frameset *);

irplib_framelist * irplib_framelist_extract(const irplib_framelist *,
                                            const char *);
irplib_framelist * irplib_framelist_extract_regexp(const irplib_framelist *,
                                                   const char *, cpl_boolean);

/* Accessors and element modifiers */

int irplib_framelist_get_size(const irplib_framelist *);

const cpl_frame * irplib_framelist_get_const(const irplib_framelist *, int);

cpl_frame * irplib_framelist_get(irplib_framelist *, int);

const cpl_propertylist * irplib_framelist_get_propertylist_const(
                                                       const irplib_framelist *,
                                                       int);

cpl_propertylist * irplib_framelist_get_propertylist(irplib_framelist *, int);

cpl_error_code irplib_framelist_set_propertylist(irplib_framelist *, int,
                                                 const cpl_propertylist *);

cpl_error_code irplib_framelist_load_propertylist(irplib_framelist *, int,
                                                  int, const char *,
                                                  cpl_boolean);

cpl_error_code irplib_framelist_load_propertylist_all(irplib_framelist *, int,
                                                      const char *,
                                                      cpl_boolean);

cpl_error_code irplib_framelist_set_tag_all(irplib_framelist *, const char *);


/* Inserting and removing elements */

cpl_error_code irplib_framelist_set(irplib_framelist *, cpl_frame *, int);

cpl_error_code irplib_framelist_erase(irplib_framelist *, int);

cpl_frame * irplib_framelist_unset(irplib_framelist *, int, cpl_propertylist **);

void irplib_framelist_empty(irplib_framelist *);

/* Others */
cpl_error_code irplib_framelist_contains(const irplib_framelist *, const char *,
                                         cpl_type, cpl_boolean, double);

cpl_imagelist * irplib_imagelist_load_framelist(const irplib_framelist *,
                                                cpl_type, int, int);

cpl_frameset * irplib_frameset_cast(const irplib_framelist *);

#endif
