/* 
 * This file is part of the METIS Pipeline
 * Copyright (C) 2002-2019 European Southern Observatory
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

/*----------------------------------------------------------------------------*/
/**
 *                              Defines
 */
/*----------------------------------------------------------------------------*/

#define RECIPE_NAME      "metis"
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

static const char metis_description[] =
    "This example text is used to describe the recipe.\n"
    "The description should include the required FITS-files and\n"
    "their associated tags, e.g.\n"
    "METIS-METIS-raw-file.fits " METIS_RAW "\n"
    "and any optional files, e.g.\n"
    "METIS-METIS-flat-file.fits " METIS_CALIB_FLAT "\n"
    "\n"
    "Additionally, it should describe functionality of the expected output."
    "\n";

/* Standard CPL recipe definition */
cpl_recipe_define(	metis,
                  	METIS_BINARY_VERSION,
                  	"Firstname Lastname",
                  	PACKAGE_BUGREPORT,
                  	"2021",
                  	"An example recipe.",
                  	metis_description);

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
 * @return   CPL_ERROR_NONE(0) if everything is ok
 *
 */
/*----------------------------------------------------------------------------*/
static int metis(
    cpl_frameset            * frameset,
    const cpl_parameterlist * parlist)
{
  const cpl_parameter *param;
  const char          *str_option;
  int                 bool_option;
  const cpl_frame     *rawframe;
  const cpl_frame     *flat;
  double              qc_param;
  cpl_propertylist    *plist;
  cpl_propertylist    *applist;
  cpl_image           *image;


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
  rawframe = cpl_frameset_find_const(frameset, METIS_RAW);
  if (rawframe == NULL) {
      /* cpl_frameset_find_const() does not set an error code, when a frame
           is not found, so we will set one here. */
      return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                                   "SOF does not have any file tagged with %s", METIS_RAW);
  }

  /* - A recommended file */
  flat = cpl_frameset_find(frameset, METIS_CALIB_FLAT);
  if (flat == NULL) {
      cpl_msg_warning(cpl_func, "SOF does not have any file tagged with %s",
                      METIS_CALIB_FLAT);
  }


  /* HOW TO GET THE VALUE OF A FITS KEYWORD */
  /*  - Load only DETector related keys */
  plist = cpl_propertylist_load_regexp(cpl_frame_get_filename(rawframe),
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
  image = cpl_image_load(cpl_frame_get_filename(rawframe), CPL_TYPE_FLOAT, 0,
                         0);
  if (image == NULL) {
      return cpl_error_set_message(cpl_func, cpl_error_get_code(),
                                   "Could not load the image");
  }

  applist = cpl_propertylist_new();

  /* Add the product category  */
  cpl_propertylist_append_string(applist, CPL_DFS_PRO_CATG,
                                 METIS_OUT_PROCATG);

  /* Add a QC parameter  */
  cpl_propertylist_append_double(applist, "ESO QC QCPARAM", qc_param);


  /* HOW TO SAVE A DFS-COMPLIANT PRODUCT TO DISK  */
  if (cpl_dfs_save_image(frameset, NULL, parlist, frameset, NULL, image,
                         CPL_BPP_IEEE_FLOAT, RECIPE_NAME, applist,
                         NULL, PACKAGE "/" PACKAGE_VERSION,
                         "metis.fits")) {
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
static cpl_error_code metis_fill_parameterlist(
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

  /* --fileopt */
  par = cpl_parameter_new_value(CONTEXT".file_option",
                                CPL_TYPE_STRING, "the file option",
                                CONTEXT, "NONE");
  cpl_parameter_set_alias(par, CPL_PARAMETER_MODE_CLI, "fileopt");
  cpl_parameter_disable(par, CPL_PARAMETER_MODE_ENV);
  cpl_parameterlist_append(self, par);

  /* --boolopt */
  par = cpl_parameter_new_value(CONTEXT".bool_option",
                                CPL_TYPE_BOOL, "a flag",
                                CONTEXT, TRUE);
  cpl_parameter_set_alias(par, CPL_PARAMETER_MODE_CLI, "boolopt");
  cpl_parameter_disable(par, CPL_PARAMETER_MODE_ENV);
  cpl_parameterlist_append(self, par);

  /* --intopt */
  par = cpl_parameter_new_value(CONTEXT".int_option",
                                CPL_TYPE_INT, "an integer",
                                CONTEXT, 3);
  cpl_parameter_set_alias(par, CPL_PARAMETER_MODE_CLI, "intopt");
  cpl_parameter_disable(par, CPL_PARAMETER_MODE_ENV);
  cpl_parameterlist_append(self, par);

  /* --floatopt */
  par = cpl_parameter_new_value(CONTEXT".float_option", CPL_TYPE_DOUBLE,
                                "A float",
                                CONTEXT, 0.5);
  cpl_parameter_set_alias(par, CPL_PARAMETER_MODE_CLI, "floatopt");
  cpl_parameter_disable(par, CPL_PARAMETER_MODE_ENV);
  cpl_parameterlist_append(self, par);

  /* --rangeopt */
  par = cpl_parameter_new_range(CONTEXT".range_option", CPL_TYPE_INT,
                                "This is a value range of type int",
                                CONTEXT, 3, 0, 10);
  cpl_parameter_set_alias(par, CPL_PARAMETER_MODE_CLI, "rangeopt");
  cpl_parameter_disable(par, CPL_PARAMETER_MODE_ENV);
  cpl_parameterlist_append(self, par);

  /* --enumopt */
  par = cpl_parameter_new_enum(CONTEXT".enum_option", CPL_TYPE_STRING,
                               "This is an enumeration of type string",
                               CONTEXT, "first", 3, "first", "second", "third");
  cpl_parameter_set_alias(par, CPL_PARAMETER_MODE_CLI, "enumopt");
  cpl_parameter_disable(par, CPL_PARAMETER_MODE_ENV);
  cpl_parameterlist_append(self, par);

  /* --floatrangeopt */
  par = cpl_parameter_new_range(CONTEXT".float_range_option", CPL_TYPE_DOUBLE,
                                "This is a value range of type float."
                                " Valid ragne is [-5.5, 5.5]",
                                CONTEXT, 3.5, -5.5, 5.5);
  cpl_parameter_set_alias(par, CPL_PARAMETER_MODE_CLI, "floatrangeopt");
  cpl_parameter_disable(par, CPL_PARAMETER_MODE_ENV);
  cpl_parameterlist_append(self, par);


  /* Check possible errors */
  if (!cpl_errorstate_is_equal(prestate)) {
      return cpl_error_set_message(cpl_func, cpl_error_get_code(),
                                   "metis_fill_parameterlist failed!");
  }

  return CPL_ERROR_NONE;
}
