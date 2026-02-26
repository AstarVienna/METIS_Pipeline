/*                                                                            *
 *   This file is part of the ESO IRPLIB package                              *
 *   Copyright (C) 2004,2005 European Southern Observatory                    *
 *                                                                            *
 *   This library is free software; you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License as published by     *
 *   the Free Software Foundation; either version 2 of the License, or        *
 *   (at your option) any later version.                                      *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program; if not, write to the Free Software              *
 *   Foundation, 51 Franklin St, Fifth Floor, Boston, MA  02111-1307  USA     *
 *                                                                            */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <irplib_plugin.h>
#include <irplib_utils.h>
#include <string.h>
#include <float.h>

/*-----------------------------------------------------------------------------
                                   Function prototypes
 -----------------------------------------------------------------------------*/

/* Declare routines defining a dummy recipe to test the irplib_plugin_test
 * function. */
cpl_recipe_define(test_recipe, 123, "Some Author", "someone@local.org", "2014",
                  "For testing.", "Simple recipe for testing.");

static void test_irplib_recipe_test(void);

/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_plugin_test Testing of the IRPLIB utilities
 */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/**
   @brief   Unit tests of plugin module
**/
/*----------------------------------------------------------------------------*/

int main(void)
{



    cpl_parameterlist * parlist;

    /* Initialize CPL for unit testing */
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    parlist = cpl_parameterlist_new();


    (void)irplib_parameterlist_get_double(parlist, "INST", "RECIPE", NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    (void)irplib_parameterlist_get_double(parlist, "INST", NULL, "PAR");
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    (void)irplib_parameterlist_get_double(parlist, NULL, "RECIPE", "PAR");
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    (void)irplib_parameterlist_get_double(NULL, "INST", "RECIPE", "PAR");
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    (void)irplib_parameterlist_get_double(parlist, "INST", "RECIPE", "PAR");
    cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);


    (void)irplib_parameterlist_get_int(parlist, "INST", "RECIPE", NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    (void)irplib_parameterlist_get_int(parlist, "INST", NULL, "PAR");
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    (void)irplib_parameterlist_get_int(parlist, NULL, "RECIPE", "PAR");
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    (void)irplib_parameterlist_get_int(NULL, "INST", "RECIPE", "PAR");
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    (void)irplib_parameterlist_get_int(parlist, "INST", "RECIPE", "PAR");
    cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);


    (void)irplib_parameterlist_get_bool(parlist, "INST", "RECIPE", NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    (void)irplib_parameterlist_get_bool(parlist, "INST", NULL, "PAR");
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    (void)irplib_parameterlist_get_bool(parlist, NULL, "RECIPE", "PAR");
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    (void)irplib_parameterlist_get_bool(NULL, "INST", "RECIPE", "PAR");
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    (void)irplib_parameterlist_get_bool(parlist, "INST", "RECIPE", "PAR");
    cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);


    (void)irplib_parameterlist_get_string(parlist, "INST", "RECIPE", NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    (void)irplib_parameterlist_get_string(parlist, "INST", NULL, "PAR");
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    (void)irplib_parameterlist_get_string(parlist, NULL, "RECIPE", "PAR");
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    (void)irplib_parameterlist_get_string(NULL, "INST", "RECIPE", "PAR");
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    (void)irplib_parameterlist_get_string(parlist, "INST", "RECIPE", "PAR");
    cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);


    cpl_parameterlist_delete(parlist);

    test_irplib_recipe_test();

    return cpl_test_end(0);
}


static void test_irplib_recipe_test(void)
{
    /* Execute a simple test with the irplib_plugin_test to see that there are
     * no serious errors with that routine. */

    const char * tags[] = {
        "TEST_TAG",
        "ANOTHER_TAG"
    };
    cpl_pluginlist * pluginlist;
    const size_t ntags = sizeof(tags) / sizeof(char*);
    pluginlist = cpl_pluginlist_new();
    cpl_test_nonnull(pluginlist);

    cpl_test_zero(cpl_plugin_get_info(pluginlist));
    cpl_test_zero(irplib_plugin_test(pluginlist, ntags, tags));

    cpl_pluginlist_delete(pluginlist);
}

/**
 * @internal
 * @brief Dummy implementation of function for filling recipe parameters.
 */
static cpl_error_code test_recipe_fill_parameterlist(cpl_parameterlist *self)
{
    cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);
    return CPL_ERROR_NONE;
}

/**
 * @internal
 * @brief Dummy implementation of function to execute recipe.
 */
static int test_recipe(cpl_frameset *frames, const cpl_parameterlist *params)
{
    cpl_frameset_iterator * iterator = NULL;
    const cpl_frame * frame;

    cpl_ensure_code(frames != NULL && params != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(cpl_frameset_get_size(frames) > 0,
                    CPL_ERROR_DATA_NOT_FOUND);

    /* Assume all input files are FITS files and just try load the primary
     * header. If an error occurs then return it properly. This will exercise
     * the tests run by irplib_plugin_test. */
    for (frame = irplib_frameset_get_first_const(&iterator, frames);
         frame != NULL;
         frame = irplib_frameset_get_next_const(iterator))
    {
        const char * name = cpl_frame_get_filename(frame);
        cpl_propertylist * props = cpl_propertylist_load(name, 0);
        if (props == NULL) {
            cpl_frameset_iterator_delete(iterator);
            return cpl_error_get_code();
        }
        cpl_propertylist_delete(props);
    }

    cpl_frameset_iterator_delete(iterator);
    return CPL_ERROR_NONE;
}
