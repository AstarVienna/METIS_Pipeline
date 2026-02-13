/* $Id: hdrl_parameter.c,v 1.3 2013-10-10 13:47:35 jtaylor Exp $
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
 * $Date: 2013-10-10 13:47:35 $
 * $Revision: 1.3 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_parameter.h"
#include "hdrl_parameter_defs.h"
#include <cpl.h>

#include <stddef.h>
#include <assert.h>

/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_parameter   Parameter object
 *
 * The hdrl_parameter is the base object to store various hierarchical
 * parameters. It only provides the deletion functions, the parameters
 * themselves are implemented in their respective modules.
 *
 * @internal
 * see hdrl_parameter-test.c for an example usage
 */
/*----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                                   Functions
 -----------------------------------------------------------------------------*/

/**@{*/

/** @cond PRIVATE */

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief create new parameter of certain type
 *
 * @param type  type object which defines the type of the parameter
 *
 * @return hdrl_parameter of type 'typeobj'
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter * hdrl_parameter_new(const hdrl_parameter_typeobj * typeobj)
{
    hdrl_parameter * p = typeobj->fp_alloc(typeobj->obj_size);
    p->base = typeobj;
    return p;
}

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief check if a parameter is of a certain type
 *
 * @param self  parameter to check
 * @param type  type required
 *
 * @return true iff parameter is of type "type"
 */
/* ---------------------------------------------------------------------------*/
int hdrl_parameter_check_type(const hdrl_parameter * self,
                              const hdrl_parameter_typeobj * type)
{
    /* we can't compare the base pointers as they may be different in recipes
     * and libraries when static linking */
    if (self) {
        return ((const hdrl_parameter_typeobj *)self->base)->type == type->type;
    }
    else {
        return 0;
    }
}


/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief get typeobject of parameter
 *
 * @param self  parameter to check
 * @return pointer to type object
 *
 */
/* ---------------------------------------------------------------------------*/
const hdrl_parameter_typeobj * hdrl_parameter_get_type(const hdrl_parameter * self)
{
    return self->base;
}

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief get enum of parameter
 *
 * @param self  parameter to check
 * @return pointer to type object
 *
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter_enum
hdrl_parameter_get_parameter_enum(const hdrl_parameter * self){
    return hdrl_parameter_get_type(self)->type;
}

/** @endcond */

/* ---------------------------------------------------------------------------*/
/**
 * @brief shallow delete of a parameter
 *
 * @param obj parameter to delete, may be NULL
 *
 * will not delete sub parameters
 * @see hdrl_parameter_destroy
 */
/* ---------------------------------------------------------------------------*/
void hdrl_parameter_delete(hdrl_parameter * obj)
{
    if (obj) {
        obj->base->fp_free(obj);
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief deep delete of a parameter
 *
 * @param obj parameter to delete, may be NULL
 *
 * deletes all sub parameters via the registered deep destructor
 */
/* ---------------------------------------------------------------------------*/
void hdrl_parameter_destroy(hdrl_parameter * obj)
{
    if (obj == NULL) {
        return;
    }

    if (obj->base->fp_destroy) {
        obj->base->fp_destroy(obj);
    }
    else {
        obj->base->fp_free(obj);
    }
}

/**@}*/
