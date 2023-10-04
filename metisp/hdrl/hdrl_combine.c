/* $Id: hdrl_combine.c,v 1.15 2013-09-24 14:58:54 jtaylor Exp $
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
 * $Date: 2013-09-24 14:58:54 $
 * $Revision: 1.15 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
-----------------------------------------------------------------------------*/

#include "hdrl_combine.h"
#include "hdrl_iter.h"
#include "hdrl_utils.h"
#include "hdrl_sigclip.h"
#include <cpl.h>
#include <math.h>
#include <string.h>
#include <assert.h>

/*-----------------------------------------------------------------------------
                                   Static
 -----------------------------------------------------------------------------*/

/** @cond PRIVATE */

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_combine   COMBINE Module

  This module allows the combination of imagelists with error propagation.
  If input images size is large the user should use hdrl_imagelist_combine_it()
  for efficient RAM usage. Else can be used hdrl_imagelist_combine()
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief combine imagelist with error propagation
 *
 * @param data     input data imagelist
 * @param errors   input imagelist containing errors of data
 * @param method   reduction method applied to the imagelists
 * @param out      pointer to storage for a pointer to the allocated
 *                 combined data image
 * @param out      pointer to storage for a pointer to the allocated
 *                 combined error image
 * @param contrib  pointer to storage for a pointer to the allocated
 *                 contribution map
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code
hdrl_imagelist_combine(const cpl_imagelist * data,
                       const cpl_imagelist * errors,
                       hdrl_collapse_imagelist_to_image_t * method,
                       cpl_image ** out,
                       cpl_image ** err,
                       cpl_image ** contrib)
{
    cpl_ensure_code(data && errors, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(out && err && contrib, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(cpl_imagelist_get_size(data) > 0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(cpl_imagelist_get_size(data) ==
                    cpl_imagelist_get_size(errors), CPL_ERROR_ILLEGAL_INPUT);

    hdrl_collapse_imagelist_to_image_call(method, data, errors,
                                        out, err, contrib, NULL);

    return cpl_error_get_code();
}

/**@}*/

/** @endcond */
