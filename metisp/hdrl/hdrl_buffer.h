/* $Id: hdrl_buffer.h,v 1.2 2013-10-23 09:42:14 jtaylor Exp $
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
 * $Revision: 1.2 $
 * $Name: not supported by cvs2svn $
 */

#ifndef HDRL_BUFFER_H
#define HDRL_BUFFER_H

#include <cpl.h>
#include <stddef.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
  Experimental declarations - can be used, but no guarantees on api stability
 -----------------------------------------------------------------------------*/
#if defined HDRL_USE_EXPERIMENTAL || defined HDRL_USE_PRIVATE
typedef struct _hdrl_buffer_ hdrl_buffer;

hdrl_buffer * hdrl_buffer_new(void);
void hdrl_buffer_readonly(hdrl_buffer * buf, cpl_boolean ro);
size_t hdrl_buffer_set_malloc_threshold(hdrl_buffer * buf, size_t t);
char * hdrl_buffer_allocate(hdrl_buffer *, size_t);
void hdrl_buffer_free(hdrl_buffer *, char *);
void hdrl_buffer_delete(hdrl_buffer *);
#endif

CPL_END_DECLS

#endif 
