/* $Id: hdrl_test.h,v 1.1 2013-10-22 08:26:11 jtaylor Exp $
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
 * $Date: 2013-10-22 08:26:11 $
 * $Revision: 1.1 $
 * $Name: not supported by cvs2svn $
 */

#ifndef HDRL_TEST_H
#define HDRL_TEST_H

#include "hdrl_image.h"
#include "hdrl_types.h"

#include <cpl.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                                   Macros
 -----------------------------------------------------------------------------*/

#define hdrl_test_image_abs(himg1, himg2, tol) \
{ \
    cpl_test_image_abs(hdrl_image_get_image_const(himg1), \
                       hdrl_image_get_image_const(himg2), tol * \
                       HDRL_EPS_ERROR / HDRL_EPS_DATA); \
    cpl_test_image_abs(hdrl_image_get_error_const(himg1), \
                       hdrl_image_get_error_const(himg2), tol * \
                       HDRL_EPS_ERROR / HDRL_EPS_DATA); \
}

CPL_END_DECLS

#endif
