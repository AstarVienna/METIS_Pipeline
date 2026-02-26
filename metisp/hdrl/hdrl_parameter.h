/*
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

#ifndef HDRL_PARAMETER_H
#define HDRL_PARAMETER_H

/*-----------------------------------------------------------------------------
                                   New types
 -----------------------------------------------------------------------------*/

typedef struct _hdrl_parameter_ hdrl_parameter;

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_types.h"
#include "hdrl_utils.h"
#include <cpl.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                                   Functions
 -----------------------------------------------------------------------------*/

void hdrl_parameter_delete(hdrl_parameter * obj);
void hdrl_parameter_destroy(hdrl_parameter * obj);

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
 -----------------------------------------------------------------------------*/
#ifdef HDRL_USE_PRIVATE

/**
 * @brief parameter registry
 * required as typeobject pointer is not unique when doing static linking
 */
typedef enum {
  HDRL_PARAMETER_COLLAPSE_MEAN,
  HDRL_PARAMETER_COLLAPSE_MEDIAN,
  HDRL_PARAMETER_COLLAPSE_WEIGHTED_MEAN,
  HDRL_PARAMETER_COLLAPSE_SIGCLIP,
  HDRL_PARAMETER_COLLAPSE_MINMAX,
  HDRL_PARAMETER_COLLAPSE_MODE,
  HDRL_PARAMETER_RECT_REGION,
  HDRL_PARAMETER_OVERSCAN,
  HDRL_PARAMETER_BPM_2D,
  HDRL_PARAMETER_BPM_3D,
  HDRL_PARAMETER_BPM_FIT,
  HDRL_PARAMETER_LACOSMIC,
  HDRL_PARAMETER_STREHL,
  HDRL_PARAMETER_FLAT,
  HDRL_PARAMETER_CATALOGUE,
  HDRL_PARAMETER_SPECTRUM1D_RESAMPLE_INTERPOLATE,
  HDRL_PARAMETER_SPECTRUM1D_RESAMPLE_FIT,
  HDRL_PARAMETER_SPECTRUM1D_RESAMPLE_INTEGRATE,
  HDRL_PARAMETER_SPECTRUM1D_SHIFT,
  HDRL_PARAMETER_EFFICIENCY,
  HDRL_PARAMETER_RESPONSE_TELLURIC_EVALUATION,
  HDRL_PARAMETER_RESPONSE_FIT,
  HDRL_PARAMETER_DAR,
  HDRL_PARAMETER_RESAMPLE_OUTGRID,
  HDRL_PARAMETER_RESAMPLE_METHOD
} hdrl_parameter_enum;


/* ---------------------------------------------------------------------------*/
/**
 * @brief required first member of each parameter object
 *
 *   struct {
 *     HDRL_PARAMETER_HEAD;
 *     int val;
 *   } my_parameter;
 */
/* ---------------------------------------------------------------------------*/
#define HDRL_PARAMETER_HEAD void * base

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief Base type object / object metadata
 *
 * Defines required object meta data of the parameter.
 * Each object contains a reference to one of these so it can determine which
 * functions to call on certain operations.
 * Usually a single statically allocated type object shared between multiple
 * hdrl_parameter objects of the same type.
 * @see hdrl_parameter_new
 */
/* ---------------------------------------------------------------------------*/
typedef struct {
    /* enum defining type */
    hdrl_parameter_enum type;
    /* base parameter structure allocation, e.g. cpl_malloc */
    hdrl_alloc * fp_alloc;
    /* shallow destructor of the parameter */
    hdrl_free  * fp_free;
    /* deep destructor of the parameter, deletes all childs
     * may be NULL in which case fp_free is used */
    hdrl_free  * fp_destroy;
    /* size of the base parameter structure, argument to fp_alloc */
    size_t obj_size;
} hdrl_parameter_typeobj;


/* empty parameter, e.g. for collapse_mean */
typedef struct {
    HDRL_PARAMETER_HEAD;
} hdrl_parameter_empty;

hdrl_parameter *
hdrl_parameter_new(const hdrl_parameter_typeobj * typeobj);

const hdrl_parameter_typeobj *
hdrl_parameter_get_type(const hdrl_parameter * self);

hdrl_parameter_enum
hdrl_parameter_get_parameter_enum(const hdrl_parameter * self);

int
hdrl_parameter_check_type(const hdrl_parameter * self,
                              const hdrl_parameter_typeobj * type);

/* create a singleton parameter which does not need allocating or deleting a
 * comon use case are enum like parameters */
#define HDRL_PARAMETER_SINGLETON(name, type, alloc) \
hdrl_parameter * name = &(hdrl_parameter){ &type }; \
static void * alloc(size_t HDRL_UNUSED(n)) \
{ \
    return name; \
}

#endif

CPL_END_DECLS

#endif
