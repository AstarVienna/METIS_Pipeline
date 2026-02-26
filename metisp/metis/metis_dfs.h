/*
 * This file is part of the METIS Pipeline
 * Copyright (C) 2002-2017 European Southern Observatory
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

#ifndef METIS_DFS_H
#define METIS_DFS_H

/*----------------------------------------------------------------------------*/
/**
 *                              Includes
 */
/*----------------------------------------------------------------------------*/

#include <cpl.h>


/*----------------------------------------------------------------------------*/
/**
 *                              Defines
 */
/*----------------------------------------------------------------------------*/

/* Define here the RAW   tag keywords */
#define METIS_RAW                    "METIS_DOCATG_RAW"

/* Define here the CALIB tag keywords */
#define METIS_CALIB_RAW              "METIS_CALIB_DOCATG_RAW"
#define METIS_CALIB_FLAT         "FLAT"

/* Define here the static calibration tag keywords */
#define LINE_INTMON_TABLE               "LINE_INTMON_TABLE"

/* Define here the OUT   tag keywords */
#define METIS_OUT_PROCATG            "METIS_DOCATG_RESULT"
#define METIS_OUT_CALIB_PROCATG      "METIS_DOCATG_CALIB_RESULT"

/*----------------------------------------------------------------------------*/
/**
 *                              Functions prototypes
 */
/*----------------------------------------------------------------------------*/

cpl_error_code metis_dfs_set_groups(cpl_frameset *);

#endif
