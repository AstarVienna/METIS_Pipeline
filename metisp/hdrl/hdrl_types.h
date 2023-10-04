/* $Id: hdrl_types.h,v 1.2 2013-10-11 13:46:00 jtaylor Exp $
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
 * $Date: 2013-10-11 13:46:00 $
 * $Revision: 1.2 $
 * $Name: not supported by cvs2svn $
 */

#ifndef HDRL_TYPES_H
#define HDRL_TYPES_H

#include <float.h> /* for epsilon */
#include <stddef.h> /* for size_t */

/*-----------------------------------------------------------------------------
                                   Generic types
 -----------------------------------------------------------------------------*/

typedef void * (hdrl_alloc)(size_t);
typedef void (hdrl_free)(void *);

/* basic type for bit mask values, e.g. bad pixel classifications */
typedef unsigned int hdrl_bitmask_t;

#ifndef HDRL_SIZEOF_DATA
#define HDRL_SIZEOF_DATA 8
#endif
#ifndef HDRL_SIZEOF_ERROR
#define HDRL_SIZEOF_ERROR 8
#endif

/* makes little sense and causes problems when error propagation
 * mixes the two types (mul/div) */
#if HDRL_SIZEOF_ERROR > HDRL_SIZEOF_DATA
#error HDRL_SIZEOF_ERROR must not be larger than HDRL_SIZEOF_DATA
#endif

#if HDRL_SIZEOF_DATA == 4
typedef float hdrl_data_t;
#define HDRL_TYPE_DATA CPL_TYPE_FLOAT
#define HDRL_EPS_DATA FLT_EPSILON
#else
typedef double hdrl_data_t;
#define HDRL_TYPE_DATA CPL_TYPE_DOUBLE
#define HDRL_EPS_DATA DBL_EPSILON
#endif

#if HDRL_SIZEOF_ERROR == 4
typedef float hdrl_error_t;
#define HDRL_TYPE_ERROR CPL_TYPE_FLOAT
#define HDRL_EPS_ERROR FLT_EPSILON
#else
typedef double hdrl_error_t;
#define HDRL_TYPE_ERROR CPL_TYPE_DOUBLE
#define HDRL_EPS_ERROR DBL_EPSILON
#endif

typedef struct {
    hdrl_data_t data;
    hdrl_error_t error;
} hdrl_value;

#endif 
