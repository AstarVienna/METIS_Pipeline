/* $Id: hdrl_combine.h,v 1.6 2013-10-17 15:44:14 jtaylor Exp $
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
 * $Revision: 1.6 $
 * $Name: not supported by cvs2svn $
 */

#ifndef HDRL_COMBINE_H
#define HDRL_COMBINE_H

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_iter.h"
#include "hdrl_collapse.h"
#include <cpl.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
 -----------------------------------------------------------------------------*/

#ifdef HDRL_USE_PRIVATE

cpl_error_code
hdrl_imagelist_combine(const cpl_imagelist * data, const cpl_imagelist * errors,
                      hdrl_collapse_imagelist_to_image_t * method,
                      cpl_image ** out, cpl_image ** err, cpl_image ** contrib);

#endif

CPL_END_DECLS

#endif
