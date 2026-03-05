/* $Id: hdrl_image_defs.h,v 1.4 2013-10-17 15:44:14 jtaylor Exp $
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
 * $Revision: 1.4 $
 * $Name: not supported by cvs2svn $
 */

#ifndef HDRL_IMAGE_DEFS_H
#define HDRL_IMAGE_DEFS_H

#ifndef HDRL_USE_PRIVATE
#error This file is not allowed to be included outside of hdrl
#endif

#include "hdrl_types.h"
#include <cpl.h>

CPL_BEGIN_DECLS

struct _hdrl_image_ {
    cpl_image * image;
    cpl_image * error;
    hdrl_free * fp_free;
};

CPL_END_DECLS

#endif
