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

#include "metis_utils.h"
#include "metis_pfits.h"
#include "metis_dfs.h"

#include <cpl.h>
#include <string.h>

/*----------------------------------------------------------------------------*/
/**
 *                              Defines
 */
/*----------------------------------------------------------------------------*/

#define RECIPE_NAME      "metis_calib"
#define CONTEXT          "metis."RECIPE_NAME

/*----------------------------------------------------------------------------*/
/**
 *                 Typedefs: Structs and enum types
 */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
 *                              Functions prototypes
 */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
 *                          Static variables
 */
/*----------------------------------------------------------------------------*/

static char metis_calib_description[] =
    "This example text is used to describe the recipe.\n"
    "The description should include the required FITS-files and\n"
    "their associated tags, e.g.\n"
    "METIS-METIS-CALIB-raw-file.fits " METIS_CALIB_RAW "\n"
    "\n"
    "Additionally, it should describe functionality of the expected output."
    "\n";

/* Standard CPL recipe definition */
cpl_recipe_define(	metis_calib,
                  	METIS_BINARY_VERSION,
                  	"Firstname Lastname",
                  	PACKAGE_BUGREPORT,
                  	"2021",
                  	"An example recipe.",
                  	metis_calib_description);

/*----------------------------------------------------------------------------*/
/**
 * @defgroup metis    Recipe explanation
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*----------------------------------------------------------------------------*/
/**
 *                              Functions code
 */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
 * @brief    Interpret the command line options and execute the data processing
 *
 * @param    frameset   the frames list
 * @param    parlist    the parameters list
 *
 * @return   0 if everything is ok
 *
 */
/*----------------------------------------------------------------------------*/
static int metis_calib(
    cpl_frameset            * frameset,
    const cpl_parameterlist * parlist)
{
  const cpl_parameter *param;
  const char          *str_option;
  int                 bool_option;
  cpl_frameset        *rawframes;
  const cpl_frame     *firstframe;
  double              qc_param;
  cpl_propertylist    *plist;
  cpl_propertylist    *applist;
  cpl_image           *image;
  int                 nraw;
  int                 i;


  if (metis_check_and_set_groups(frameset) != CPL_ERROR_NONE) {
    return cpl_error_get_code();
  }


  /* Use the errorstate to detect an error in a function that does not
       return an error code. */
  cpl_errorstate prestate = cpl_errorstate_get();


  /* HOW TO RETRIEVE INPUT PARAMETERS */

  /* --stropt */
  param = cpl_parameterlist_find_const(parlist, CONTEXT".str_option");
  str_option = cpl_parameter_get_string(param);

  /* --boolopt */
  param = cpl_parameterlist_find_const(parlist, CONTEXT".bool_option");
  bool_option = cpl_parameter_get_bool(param);

  if (!cpl_errorstate_is_equal(prestate)) {
      return cpl_error_set_message(cpl_func, cpl_error_get_code(),
                                   "Could not retrieve the input parameters");
  }

  /* HOW TO ACCESS INPUT DATA */

  /*  - A required file */
  rawframes = cpl_frameset_new();
  nraw = 0;
  for (i = 0; i<cpl_frameset_get_size(frameset); i++) {
      const cpl_frame * current_frame;
      current_frame = cpl_frameset_get_position_const(frameset, i);
      if(!strcmp(cpl_frame_get_tag(current_frame), METIS_CALIB_RAW)) {
          cpl_frame * new_frame = cpl_frame_duplicate(current_frame);
          cpl_frameset_insert(rawframes, new_frame);
          nraw++;
      }
  }
  if (nraw == 0) {
      return (int)cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                                        "SOF does not have any file tagged "
                                        "with %s", METIS_CALIB_RAW);
  }


  /* HOW TO GET THE FIRST FRAME OF A FRAME */
  firstframe = cpl_frameset_get_position_const(rawframes, 0);


  /* HOW TO GET THE VALUE OF A FITS KEYWORD */
  /*  - Load only DETector related keys */
  plist = cpl_propertylist_load_regexp(cpl_frame_get_filename(firstframe),
                                       0, "ESO DET ", 0);
  if (plist == NULL) {
      /* In this case an error message is added to the error propagation */
      return cpl_error_set_message(cpl_func, cpl_error_get_code(),
                                   "Could not read the FITS header");
  }

  if (bool_option == CPL_FALSE) {
      cpl_msg_info(cpl_func, "Bool option unset: String: %s", str_option);
  }

  qc_param = metis_pfits_get_dit(plist);
  cpl_propertylist_delete(plist);


  /* Check for a change in the CPL error state */
  /* - if it did change then propagate the error and return */
  cpl_ensure_code(cpl_errorstate_is_equal(prestate), cpl_error_get_code());


  /* NOW PERFORMING THE DATA REDUCTION */

  /* Let's just load an image for the example */
  image = cpl_image_load(cpl_frame_get_filename(firstframe), CPL_TYPE_FLOAT, 0,
                         0);
  if (image == NULL) {
      return cpl_error_set_message(cpl_func, cpl_error_get_code(),
                                   "Could not load the image");
  }

  applist = cpl_propertylist_new();

  /* Add the product category  */
  cpl_propertylist_append_string(applist, CPL_DFS_PRO_CATG,
                                 METIS_OUT_CALIB_PROCATG);

  /* Add a QC parameter  */
  cpl_propertylist_append_double(applist, "ESO QC QCPARAM", qc_param);

  /* HOW TO SAVE A DFS-COMPLIANT PRODUCT TO DISK  */
  if (cpl_dfs_save_image(frameset, NULL, parlist, frameset, NULL, image,
                         CPL_BPP_IEEE_FLOAT, RECIPE_NAME, applist,
                         NULL, PACKAGE "/" PACKAGE_VERSION,
                         "metis_calib.fits")) {
      /* Propagate the error */
      (void)cpl_error_set_where(cpl_func);
  }


  /* Cleanup */
  cpl_image_delete(image);
  cpl_propertylist_delete(applist);

  
  return cpl_error_get_code();
}

/**@}*/

/*----------------------------------------------------------------------------*/
/**
 * @brief Function needed by cpl_recipe_define to fill the input parameters
 *
 * @param  self   parameterlist where you need put parameters
 *
 * @return cpl_error_code
 *
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code metis_calib_fill_parameterlist(
    cpl_parameterlist *self)
{
  /* Add the different default parameters to the recipe */
  cpl_errorstate prestate = cpl_errorstate_get();

  /* Fill the parameters list */
  cpl_parameter *par;

  /* --stropt */
  par = cpl_parameter_new_value(CONTEXT".str_option",
                                CPL_TYPE_STRING, "the string option",
                                CONTEXT, "NONE");
  cpl_parameter_set_alias(par, CPL_PARAMETER_MODE_CLI, "stropt");
  cpl_parameter_disable(par, CPL_PARAMETER_MODE_ENV);
  cpl_parameterlist_append(self, par);

  /* --boolopt */
  par = cpl_parameter_new_value(CONTEXT".bool_option",
                                CPL_TYPE_BOOL, "a flag",
                                CONTEXT, TRUE);
  cpl_parameter_set_alias(par, CPL_PARAMETER_MODE_CLI, "boolopt");
  cpl_parameter_disable(par, CPL_PARAMETER_MODE_ENV);
  cpl_parameterlist_append(self, par);


  /* Check possible errors */
  if (!cpl_errorstate_is_equal(prestate)) {
      return cpl_error_set_message(cpl_func, cpl_error_get_code(),
                                   "metis_calib_fill_parameterlist failed!");
  }

  return CPL_ERROR_NONE;
}
