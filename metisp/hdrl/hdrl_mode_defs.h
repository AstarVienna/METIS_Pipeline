/*
 * hdrl_mode.h
 *
 *  Created on: Mar 1, 2021
 *      Author: agabasch
 */

/*
 * This file is part of the HDRL
 * Copyright (C) 2021 European Southern Observatory
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

#ifndef HDRL_MODE_DEFS_H
#define HDRL_MODE_DEFS_H

#include <cpl.h>

CPL_BEGIN_DECLS

/*----------------------------------------------------------------------------*/
/**
  @enum hdrl_mode_type
  @brief Define the type of the mode that should be computed.
 */
/*----------------------------------------------------------------------------*/
typedef enum {

    /** Mode computed mostly by the usage of the median */
    HDRL_MODE_MEDIAN,
    /** Mode computed bei the usage of weights */
    HDRL_MODE_WEIGHTED,
    /** Mode computed by a fit */
    HDRL_MODE_FIT
} hdrl_mode_type;

CPL_END_DECLS

#endif /* HDRL_MODE_DEFS_H */

