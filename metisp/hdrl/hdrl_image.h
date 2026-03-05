/* $Id: hdrl_image.h,v 1.18 2013-10-23 09:42:14 jtaylor Exp $
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
 * $Date: 2013-10-23 09:42:14 $
 * $Revision: 1.18 $
 * $Name: not supported by cvs2svn $
 */

#ifndef HDRL_IMAGE_H
#define HDRL_IMAGE_H

/*-----------------------------------------------------------------------------
                                   New types
 -----------------------------------------------------------------------------*/

typedef struct _hdrl_image_ hdrl_image;

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_image_math.h"
#include "hdrl_utils.h"
#include "hdrl_buffer.h"
#include "hdrl_types.h"

#include <cpl.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                                   Functions
 -----------------------------------------------------------------------------*/

hdrl_image * hdrl_image_new(cpl_size nx, cpl_size ny);

hdrl_image * hdrl_image_create(const cpl_image * image,
                               const cpl_image * error);

hdrl_image * hdrl_image_duplicate(const hdrl_image * himg);

void hdrl_image_delete(hdrl_image * himg);

cpl_image * hdrl_image_get_image(hdrl_image * himg);
const cpl_image * hdrl_image_get_image_const(const hdrl_image * himg);

cpl_image * hdrl_image_get_error(hdrl_image * himg);
const cpl_image * hdrl_image_get_error_const(const hdrl_image * himg);

cpl_mask * hdrl_image_get_mask(hdrl_image * himg);
const cpl_mask * hdrl_image_get_mask_const(const hdrl_image * himg);

hdrl_value hdrl_image_get_pixel(const hdrl_image * self,
                                cpl_size xpos, cpl_size ypos,
                                int * pis_rejected);

cpl_error_code hdrl_image_set_pixel(hdrl_image * self,
                                    cpl_size xpos, cpl_size ypos,
                                    hdrl_value value);

cpl_size hdrl_image_get_size_x(const hdrl_image * self) ;
cpl_size hdrl_image_get_size_y(const hdrl_image * self) ;

hdrl_image * hdrl_image_extract(const hdrl_image *,
                                cpl_size, cpl_size, cpl_size, cpl_size) ;

cpl_error_code hdrl_image_reject(hdrl_image * self,
                                 cpl_size xpos, cpl_size ypos);
cpl_error_code hdrl_image_reject_value(hdrl_image * self, cpl_value mode);
cpl_error_code hdrl_image_reject_from_mask(hdrl_image * self,
                                           const cpl_mask * map);
int hdrl_image_is_rejected(hdrl_image * self, cpl_size xpos, cpl_size ypos);
cpl_size hdrl_image_count_rejected(const hdrl_image * self);
cpl_error_code hdrl_image_accept(hdrl_image * self,
                                 cpl_size xpos, cpl_size ypos);
cpl_error_code hdrl_image_accept_all(hdrl_image * self);

cpl_error_code hdrl_image_turn(hdrl_image * self, int rot);

cpl_error_code hdrl_image_copy(hdrl_image * dst, const hdrl_image * src,
                               cpl_size xpos, cpl_size ypos);

cpl_error_code hdrl_image_insert(hdrl_image * self,
                                 const cpl_image * image,
                                 const cpl_image * error,
                                 cpl_size xpos, cpl_size ypos);

cpl_error_code hdrl_image_dump_structure(const hdrl_image *, FILE *) ;

cpl_error_code hdrl_image_dump_window(const hdrl_image *,
        cpl_size, cpl_size, cpl_size, cpl_size, FILE *) ;

/*-----------------------------------------------------------------------------
  Experimental declarations - can be used, but no guarantees on api stability
 -----------------------------------------------------------------------------*/
#if defined HDRL_USE_EXPERIMENTAL || defined HDRL_USE_PRIVATE
hdrl_image *
hdrl_image_new_from_buffer(cpl_size nx, cpl_size ny, hdrl_buffer * buf);
#endif

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
 -----------------------------------------------------------------------------*/
#ifdef HDRL_USE_PRIVATE
hdrl_image * hdrl_image_wrap(cpl_image *, cpl_image *, hdrl_free *,
                             cpl_boolean);
void hdrl_image_unwrap(hdrl_image * himg);
#endif

CPL_END_DECLS

#endif 
