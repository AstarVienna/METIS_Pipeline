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

#include <cpl.h>

#include "metis_utils.h"
#include "metis_dfs.h"

/*----------------------------------------------------------------------------*/
/**
 * @defgroup metis_utils     Miscellaneous Utilities
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*----------------------------------------------------------------------------*/
/**
 * @brief    Get the pipeline copyright and license
 *
 * @return   The copyright and license string
 *
 * The function returns a pointer to the statically allocated license string.
 * This string should not be modified using the returned pointer.
 *
 */
/*----------------------------------------------------------------------------*/
const char * metis_get_license(void)
{
  const char *metis_license = cpl_get_license("METIS", "2002,2018");

  return metis_license ;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief   check the entries in the recipe and classify the frameset with the tags
 *
 * @param   frameset      input set of frames
 *
 * @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/
cpl_error_code metis_check_and_set_groups(
    cpl_frameset *frameset)
{
  /* Check size of frameset for to know if the sof file is not empty */
  cpl_size nframes = cpl_frameset_get_size(frameset);
  for (cpl_size i = 0; i < nframes; i++) {

      cpl_frame  *frame    = cpl_frameset_get_position(frameset, i);
      const char *filename = cpl_frame_get_filename(frame);

      /* Check if the FITS file exist and have correct data,
       * return 0 if the fits file is valid without extensions */
      if (cpl_fits_count_extensions(filename) < 0){

          return cpl_error_set_message(cpl_func, cpl_error_get_code(),
                     "Problem with the file '%s' (%s --> Code %d)",
                     filename, cpl_error_get_message(), cpl_error_get_code());
      }
  }

  /* Identify the RAW, CONF and CALIB frames in the input frameset */
  if (metis_dfs_set_groups(frameset)) {

      /* Error classify frames */
      return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                 "Cannot classify RAW and/or CALIB frames");
  } else {

      /* Check classification */
      for (cpl_size i = 0; i < nframes; i++) {

          cpl_frame       *frame = cpl_frameset_get_position(frameset, i);
          const char      *tag   = cpl_frame_get_tag(frame);
          cpl_frame_group group  = cpl_frame_get_group(frame);

          /* The tag is invalid */
          if (group == CPL_FRAME_GROUP_NONE) {
              return cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT,
                         "Frame:%lld with tag:%s is invalid", i, tag);
          }
      }
  }

  return CPL_ERROR_NONE;
}

/**@}*/
