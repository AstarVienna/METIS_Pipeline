/* $Id: hdrl_imagelist_io.h,v 1.8 2013-10-17 15:44:14 jtaylor Exp $
 *
 * This file is part of the HDRL
 * Copyright (C) 2013 European Southern Observatory
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
 * $Author: jtaylor $
 * $Date: 2013-10-17 15:44:14 $
 * $Revision: 1.8 $
 * $Name: not supported by cvs2svn $
 */

#ifndef HDRL_IMAGELIST_IO_H
#define HDRL_IMAGELIST_IO_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_image.h"
#include "hdrl_imagelist.h"
#include "hdrl_iter.h"

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/

/* Imagelist constructors */
hdrl_imagelist * hdrl_imagelist_new(void) CPL_ATTR_ALLOC;
hdrl_imagelist * hdrl_imagelist_create(cpl_imagelist *, 
        cpl_imagelist *) CPL_ATTR_ALLOC;

cpl_size hdrl_imagelist_get_size(const hdrl_imagelist *);
cpl_size hdrl_imagelist_get_size_x(const hdrl_imagelist * himlist);
cpl_size hdrl_imagelist_get_size_y(const hdrl_imagelist * himlist);
hdrl_image * hdrl_imagelist_get(const hdrl_imagelist *, cpl_size);
const hdrl_image * hdrl_imagelist_get_const(const hdrl_imagelist *, cpl_size);
cpl_error_code hdrl_imagelist_set(hdrl_imagelist *, hdrl_image *, cpl_size);
hdrl_image * hdrl_imagelist_unset(hdrl_imagelist *, cpl_size);

/* Imagelist destructor */
void hdrl_imagelist_delete(hdrl_imagelist *);
void hdrl_imagelist_empty(hdrl_imagelist *);

/* Others */
hdrl_imagelist * hdrl_imagelist_duplicate(
        const hdrl_imagelist *) CPL_ATTR_ALLOC;
int hdrl_imagelist_is_consistent(const hdrl_imagelist *) ;

cpl_error_code hdrl_imagelist_dump_structure(const hdrl_imagelist *, FILE *);
cpl_error_code hdrl_imagelist_dump_window(const hdrl_imagelist *,
                     cpl_size, cpl_size, cpl_size, cpl_size, FILE *);


/*-----------------------------------------------------------------------------
  Experimental declarations - can be used, but no guarantees on api stability
 -----------------------------------------------------------------------------*/
#if defined HDRL_USE_EXPERIMENTAL || defined HDRL_USE_PRIVATE
typedef struct {
    cpl_size ly;
    cpl_size uy;
} hdrl_il_rowsliceiter_data;
hdrl_il_rowsliceiter_data hdrl_imagelist_iter_row_slices_get_data(const hdrl_iter *);
hdrl_iter * hdrl_imagelist_get_iter_row_slices(const hdrl_imagelist * hlist,
                                                 cpl_size nrows,
                                                 cpl_size overlap,
                                                 hdrl_iter_flags flags);
#endif

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
 -----------------------------------------------------------------------------*/
#ifdef HDRL_USE_PRIVATE
void hdrl_imagelist_unwrap(hdrl_imagelist * himlist);
#endif

CPL_END_DECLS

#endif 
