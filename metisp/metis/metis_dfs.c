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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*----------------------------------------------------------------------------*/
/**
 *                              Includes
 */
/*----------------------------------------------------------------------------*/

#include "metis_dfs.h"

#include <string.h>

/*----------------------------------------------------------------------------*/
/**
 * @defgroup metis_dfs  DFS related functions
 *
 * TBD
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*----------------------------------------------------------------------------*/
/**
 * @brief    Set the group as RAW or CALIB in a frameset
 *
 * @param    set    The frameset to process
 *
 * @return   CPL_ERROR_NONE iff OK
 *
 */
/*----------------------------------------------------------------------------*/
cpl_error_code metis_dfs_set_groups(cpl_frameset * set)
{
  /* Check entries */
  cpl_ensure_code(set, CPL_ERROR_NULL_INPUT);

  /* Initialize */
  cpl_size nframes = cpl_frameset_get_size(set);

  /* Loop on frames */
  for (cpl_size i = 0; i < nframes; i++) {

      cpl_frame  *cur_frame = cpl_frameset_get_position(set, i);
      const char *tag       = cpl_frame_get_tag(cur_frame);

      if (tag == NULL) {

          /* tag not defined */
          cpl_msg_warning(cpl_func, "Frame %d of %d has no tag",
                          1 + (int)i, (int)nframes);
          cpl_frame_set_group(cur_frame, CPL_FRAME_GROUP_NONE);

      } else if (!strcmp(tag, METIS_RAW                ) ||
                 !strcmp(tag, METIS_OUT_PROCATG        ) ){

          /* RAW frames */
          cpl_frame_set_group(cur_frame, CPL_FRAME_GROUP_RAW);

      } else if (!strcmp(tag, METIS_CALIB_RAW          ) ||
                 !strcmp(tag, METIS_OUT_CALIB_PROCATG  ) ||
                 !strcmp(tag, METIS_CALIB_FLAT     ) ||
                 !strcmp(tag, LINE_INTMON_TABLE           ) ){

          /* CALIB frames */
          cpl_frame_set_group(cur_frame, CPL_FRAME_GROUP_CALIB);

      } else {

          /* unknown tag frame */
          cpl_msg_warning(cpl_func, "Frame:%lld with tag:<%s>, unknown!", i, tag);
          cpl_frame_set_group(cur_frame, CPL_FRAME_GROUP_NONE);
      }
  }

  return cpl_error_get_code();
}

/**@}*/
