/* $Id: irplib_plugin.c,v 1.40 2013-08-22 17:44:56 cgarcia Exp $
 *
 * This file is part of the irplib package 
 * Copyright (C) 2002,2003 European Southern Observatory
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02111-1307  USA
 */

/*
 * $Author: cgarcia $
 * $Date: 2013-08-22 17:44:56 $
 * $Revision: 1.40 $
 * $Name: not supported by cvs2svn $
 */

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "irplib_plugin.h"

#include "irplib_utils.h"  /* irplib_reset() */

#include <string.h>
#include <stdlib.h>
#include <assert.h>

/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_plugin Irplib plugin functionality
 *
 * This module provides a macro and a unit test function for @em irplib_plugin.
 *
 * @code
 *   #include "irplib_plugin.h"
 * @endcode
 */
/*----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                                Defines
 -----------------------------------------------------------------------------*/

/* Maximum line length in SOF-file */
#ifndef LINE_LEN_MAX
#define LINE_LEN_MAX 1023
#endif

#define LINE_SCAN_FMT                           \
    "%" CPL_STRINGIFY(LINE_LEN_MAX) "s "        \
    "%" CPL_STRINGIFY(LINE_LEN_MAX) "s "        \
    "%" CPL_STRINGIFY(LINE_LEN_MAX) "s"

/* This device provides quite-random data */
#define DEV_RANDOM "/dev/urandom"

/* Copied from cpl_tools.h */
#define recipe_assert(bool) \
  ((bool) ? (cpl_msg_debug(cpl_func, \
     "OK in " __FILE__ " line %d (CPL-error state: '%s' in %s): %s",__LINE__, \
       cpl_error_get_message(), cpl_error_get_where(), #bool), 0) \
          : (cpl_msg_error(cpl_func, \
     "Failure in " __FILE__ " line %d (CPL-error state: '%s' in %s): %s", \
      __LINE__, cpl_error_get_message(), cpl_error_get_where(), #bool), 1))



/*-----------------------------------------------------------------------------
                            Private Function prototypes
 -----------------------------------------------------------------------------*/

static const cpl_parameter * irplib_parameterlist_get(const cpl_parameterlist *,
                                                      const char *,
                                                      const char *,
                                                      const char *);

static void recipe_parameterlist_set(cpl_parameterlist *);
static cpl_boolean irplib_plugin_has_sof_from_env(const cpl_plugin *,
                                                  const char *);

static void recipe_frameset_load(cpl_frameset *, const char *);

static void recipe_sof_test_devfile(cpl_plugin *, const char *, size_t,
                                   const char *[]);
static void recipe_sof_test_image_empty(cpl_plugin *, size_t, const char *[]);
static void recipe_sof_test_local(cpl_plugin *);
static void recipe_sof_test_from_env(cpl_plugin *);
static void recipe_frameset_empty(cpl_frameset *);
static void recipe_frameset_test_frame(const cpl_frame *);
static void recipe_frameset_test_frameset_diff(const cpl_frameset *,
                                               const cpl_frameset *);

static cpl_errorstate inistate;

/**@{*/

/*-----------------------------------------------------------------------------
                            Function definitions
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Retrieve the value of a plugin parameter of type string
  @param    self      The parameterlist to get from
  @param    instrume  The recipe name, e.g. PACKAGE from config.h
  @param    recipe    The recipe name, e.g. "rrrecipe"
  @param    parameter The parameter name, e.g. "stropt"
  @return   The string, or NULL on error

 */
/*----------------------------------------------------------------------------*/
const char * irplib_parameterlist_get_string(const cpl_parameterlist * self,
                                             const char * instrume,
                                             const char * recipe,
                                             const char * parameter)
{
    const cpl_parameter * par = irplib_parameterlist_get(self, instrume,
                                                         recipe, parameter);

    if (par == NULL) {

        (void)cpl_error_set_where(cpl_func);

        return NULL;
    } else {
        const char * value = cpl_parameter_get_string(par);

        if (value == NULL) (void)cpl_error_set_where(cpl_func);

        return value;
    }
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Retrieve the value of a plugin parameter of type int
  @param    self      The parameterlist to get from
  @param    instrume  The recipe name, e.g. PACKAGE from config.h
  @param    recipe    The recipe name, e.g. "rrrecipe"
  @param    parameter The parameter name, e.g. "boolopt"
  @return   The cpl_boolean (CPL_FALSE or CPL_TRUE), or undefined on error

 */
/*----------------------------------------------------------------------------*/
cpl_boolean irplib_parameterlist_get_bool(const cpl_parameterlist * self,
                                          const char * instrume,
                                          const char * recipe,
                                          const char * parameter)
{
    const cpl_parameter * par = irplib_parameterlist_get(self, instrume,
                                                         recipe, parameter);

    if (par == NULL) {

        (void)cpl_error_set_where(cpl_func);

        return CPL_FALSE;
    } else {
        cpl_errorstate    prestate = cpl_errorstate_get();
        const cpl_boolean value    = cpl_parameter_get_bool(par);

        if (!cpl_errorstate_is_equal(prestate))
            (void)cpl_error_set_where(cpl_func);

        return value;
    }
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Retrieve the value of a plugin parameter of type int
  @param    self      The parameterlist to get from
  @param    instrume  The recipe name, e.g. PACKAGE from config.h
  @param    recipe    The recipe name, e.g. "rrrecipe"
  @param    parameter The parameter name
  @return   The int, or undefined on error

 */
/*----------------------------------------------------------------------------*/
int irplib_parameterlist_get_int(const cpl_parameterlist * self,
                                 const char * instrume,
                                 const char * recipe,
                                 const char * parameter)
{
    const cpl_parameter * par = irplib_parameterlist_get(self, instrume,
                                                         recipe, parameter);

    if (par == NULL) {

        (void)cpl_error_set_where(cpl_func);

        return 0;
    } else {
        cpl_errorstate prestate = cpl_errorstate_get();
        const int      value    = cpl_parameter_get_int(par);

        if (!cpl_errorstate_is_equal(prestate))
            (void)cpl_error_set_where(cpl_func);

        return value;
    }
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Retrieve the value of a plugin parameter of type double
  @param    self      The parameterlist to get from
  @param    instrume  The recipe name, e.g. PACKAGE from config.h
  @param    recipe    The recipe name, e.g. "rrrecipe"
  @param    parameter The parameter name
  @return   The double, or undefined on error

 */
/*----------------------------------------------------------------------------*/
double irplib_parameterlist_get_double(const cpl_parameterlist * self,
                                       const char * instrume,
                                       const char * recipe,
                                       const char * parameter)
{
    const cpl_parameter * par = irplib_parameterlist_get(self, instrume,
                                                         recipe, parameter);

    if (par == NULL) {

        (void)cpl_error_set_where(cpl_func);

        return 0.0;
    } else {
        cpl_errorstate prestate = cpl_errorstate_get();
        const double   value    = cpl_parameter_get_double(par);

        if (!cpl_errorstate_is_equal(prestate))
            (void)cpl_error_set_where(cpl_func);

        return value;
    }
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Add a parameter of type string to a plugin parameterlist
  @param    self      The parameterlist to set
  @param    instrume  The recipe name, e.g. PACKAGE from config.h
  @param    recipe    The recipe name, e.g. "rrrecipe"
  @param    parameter The parameter name
  @param    defvalue  The default value of the parameter
  @param    alias     The alias of the parameter or NULL to alias to parameter
  @param    context   The parameter context
  @param    man       The man-page of the parameter
  @return   CPL_ERROR_NONE or the relevant CPL error code

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_parameterlist_set_string(cpl_parameterlist * self,
                                               const char * instrume,
                                               const char * recipe,
                                               const char * parameter,
                                               const char * defvalue,
                                               const char * alias,
                                               const char * context,
                                               const char * man)
{

    cpl_error_code  error;
    cpl_parameter * par;
    char          * paramname = cpl_sprintf("%s.%s.%s", instrume, recipe,
                                            parameter);

    cpl_ensure_code(paramname != NULL, cpl_error_get_code());
    
    par = cpl_parameter_new_value(paramname, CPL_TYPE_STRING, man, context,
                                  defvalue);
    cpl_free(paramname);

    cpl_ensure_code(par != NULL, cpl_error_get_code());
    
    error = cpl_parameter_set_alias(par, CPL_PARAMETER_MODE_CLI,
                                    alias ? alias : parameter);
    cpl_ensure_code(!error, error);

    error = cpl_parameter_disable(par, CPL_PARAMETER_MODE_ENV);
    cpl_ensure_code(!error, error);

    error = cpl_parameterlist_append(self, par);
    cpl_ensure_code(!error, error);
    
    return CPL_ERROR_NONE;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Add a parameter of type cpl_boolean to a plugin parameterlist
  @param    self      The parameterlist to set
  @param    instrume  The recipe name, e.g. PACKAGE from config.h
  @param    recipe    The recipe name, e.g. "rrrecipe"
  @param    parameter The parameter name
  @param    defvalue  The default value of the parameter
  @param    alias     The alias of the parameter or NULL to alias to parameter
  @param    context   The parameter context
  @param    man       The man-page of the parameter
  @return   CPL_ERROR_NONE or the relevant CPL error code

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_parameterlist_set_bool(cpl_parameterlist * self,
                                             const char * instrume,
                                             const char * recipe,
                                             const char * parameter,
                                             cpl_boolean  defvalue,
                                             const char * alias,
                                             const char * context,
                                             const char * man)
{

    cpl_error_code  error;
    cpl_parameter * par;
    char          * paramname = cpl_sprintf("%s.%s.%s", instrume, recipe,
                                            parameter);

    cpl_ensure_code(paramname != NULL, cpl_error_get_code());
    
    par = cpl_parameter_new_value(paramname, CPL_TYPE_BOOL, man, context,
                                  defvalue);
    cpl_free(paramname);

    cpl_ensure_code(par != NULL, cpl_error_get_code());
    
    error = cpl_parameter_set_alias(par, CPL_PARAMETER_MODE_CLI,
                                    alias ? alias : parameter);
    cpl_ensure_code(!error, error);
    
    error = cpl_parameter_disable(par, CPL_PARAMETER_MODE_ENV);
    cpl_ensure_code(!error, error);

    error = cpl_parameterlist_append(self, par);
    cpl_ensure_code(!error, error);
    
    return CPL_ERROR_NONE;
}



/*----------------------------------------------------------------------------*/
/**
  @brief    Add a parameter of type int to a plugin parameterlist
  @param    self      The parameterlist to set
  @param    instrume  The recipe name, e.g. PACKAGE from config.h
  @param    recipe    The recipe name, e.g. "rrrecipe"
  @param    parameter The parameter name
  @param    defvalue  The default value of the parameter
  @param    alias     The alias of the parameter or NULL to alias to parameter
  @param    context   The parameter context
  @param    man       The man-page of the parameter
  @return   CPL_ERROR_NONE or the relevant CPL error code

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_parameterlist_set_int(cpl_parameterlist * self,
                                            const char * instrume,
                                            const char * recipe,
                                            const char * parameter,
                                            int         defvalue,
                                            const char * alias,
                                            const char * context,
                                            const char * man)
{

    cpl_error_code  error;
    cpl_parameter * par;
    char          * paramname = cpl_sprintf("%s.%s.%s", instrume, recipe,
                                            parameter);

    cpl_ensure_code(paramname != NULL, cpl_error_get_code());
    
    par = cpl_parameter_new_value(paramname, CPL_TYPE_INT, man, context,
                                  defvalue);
    cpl_free(paramname);

    cpl_ensure_code(par != NULL, cpl_error_get_code());
    
    error = cpl_parameter_set_alias(par, CPL_PARAMETER_MODE_CLI,
                                    alias ? alias : parameter);
    cpl_ensure_code(!error, error);
    
    error = cpl_parameter_disable(par, CPL_PARAMETER_MODE_ENV);
    cpl_ensure_code(!error, error);

    error = cpl_parameterlist_append(self, par);
    cpl_ensure_code(!error, error);
    
    return CPL_ERROR_NONE;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Add a parameter of type double to a plugin parameterlist
  @param    self      The parameterlist to set
  @param    instrume  The recipe name, e.g. PACKAGE from config.h
  @param    recipe    The recipe name, e.g. "rrrecipe"
  @param    parameter The parameter name
  @param    defvalue  The default value of the parameter
  @param    alias     The alias of the parameter or NULL to alias to parameter
  @param    context   The parameter context
  @param    man       The man-page of the parameter
  @return   CPL_ERROR_NONE or the relevant CPL error code

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_parameterlist_set_double(cpl_parameterlist * self,
                                               const char * instrume,
                                               const char * recipe,
                                               const char * parameter,
                                               double       defvalue,
                                               const char * alias,
                                               const char * context,
                                               const char * man)
{

    cpl_error_code  error;
    cpl_parameter * par;
    char          * paramname = cpl_sprintf("%s.%s.%s", instrume, recipe,
                                            parameter);

    cpl_ensure_code(paramname != NULL, cpl_error_get_code());
    
    par = cpl_parameter_new_value(paramname, CPL_TYPE_DOUBLE, man, context,
                                  defvalue);
    cpl_free(paramname);

    cpl_ensure_code(par != NULL, cpl_error_get_code());
    
    error = cpl_parameter_set_alias(par, CPL_PARAMETER_MODE_CLI,
                                    alias ? alias : parameter);
    cpl_ensure_code(!error, error);
    
    error = cpl_parameter_disable(par, CPL_PARAMETER_MODE_ENV);
    cpl_ensure_code(!error, error);

    error = cpl_parameterlist_append(self, par);
    cpl_ensure_code(!error, error);
    
    return CPL_ERROR_NONE;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Find a plugin and submit it to some tests
  @param    self   A non-empty pluginlist
  @param    nstr   The counter of parameter strings, may be zero
  @param    astr   The array of nstr parameter strings, may be NULL
  @return   0
  @note astr may be NULL iff nstr is zero.
        If nstr is non-zero, the strings are used as tags in SOFs
        created to test the plugin.

  FIXME: Redeclare to void

 */
/*----------------------------------------------------------------------------*/
int irplib_plugin_test(cpl_pluginlist * self, size_t nstr, const char *astr[]) {

    cpl_plugin     * plugin;
    int            (*recipe_create) (cpl_plugin *);
    int            (*recipe_exec  ) (cpl_plugin *);
    int            (*recipe_deinit) (cpl_plugin *);
    FILE         * stream;
    cpl_boolean    is_debug;


    is_debug = cpl_msg_get_level() <= CPL_MSG_DEBUG ? CPL_TRUE : CPL_FALSE;

    /* Modified from CPL unit tests */
    stream = is_debug ? stdout : fopen("/dev/null", "a");

    inistate = cpl_errorstate_get();

    assert( nstr == 0 || astr != NULL );

    plugin = cpl_pluginlist_get_first(self);

    if (plugin == NULL) {
        cpl_msg_warning(cpl_func, "With an empty pluginlist, "
                        "no tests can be made");
        return 0;
    }

    cpl_plugin_dump(plugin, stream);

    recipe_create = cpl_plugin_get_init(plugin);
    cpl_test( recipe_create != NULL);

    recipe_exec   = cpl_plugin_get_exec(plugin);
    cpl_test( recipe_exec != NULL);

    recipe_deinit = cpl_plugin_get_deinit(plugin);
    cpl_test( recipe_deinit != NULL);

    /* Only plugins of type recipe are tested (further)  */
    if (cpl_plugin_get_type(plugin) != CPL_PLUGIN_TYPE_RECIPE) {
        cpl_msg_warning(cpl_func, "This plugin is not of type recipe, "
                      "cannot test further");
        return 0;
    }

    if (recipe_create != NULL && recipe_exec != NULL && recipe_deinit != NULL) {

        cpl_error_code error;
        cpl_recipe     * recipe;

        cpl_test_zero(recipe_create(plugin));

        recipe = (cpl_recipe *) plugin;

        cpl_test_nonnull( recipe->parameters );

        recipe_parameterlist_set(recipe->parameters);

        cpl_parameterlist_dump(recipe->parameters, stream);

        recipe->frames = cpl_frameset_new();

        if (irplib_plugin_has_sof_from_env(plugin, "RECIPE_SOF_PATH")) {

            recipe_sof_test_from_env(plugin);

        } else {

            const cpl_msg_severity msg_level = cpl_msg_get_level();

            /* Unless the CPL_MSG_LEVEL has been explicitly set, turn off
               terminal messaging completely while inside this function */
            if (getenv("CPL_MSG_LEVEL") == NULL) cpl_msg_set_level(CPL_MSG_OFF);

            cpl_msg_info(cpl_func,"Checking handling of pre-existing CPL error "
                         "state - may produce warning(s)/error(s):");
            cpl_error_set(cpl_func, CPL_ERROR_EOL);
            /* Call recipe and expect non-zero return code */
            cpl_test( recipe_exec(plugin) );
            /* Expect also the CPL error code to be preserved */
            cpl_test_error( CPL_ERROR_EOL );

            cpl_msg_info(cpl_func,"Checking handling of empty frameset - "
                         "may produce warning(s)/error(s):");
            /* Call recipe and expect non-zero return code */
            cpl_test( recipe_exec(plugin) );
            error = cpl_error_get_code();
            /* Expect also the CPL error code to be set */
            cpl_test_error( error );
            cpl_test( error );

            cpl_msg_info(cpl_func,"Checking handling of dummy frameset - "
                         "may produce warning(s)/error(s):");
            do {
                cpl_frame * f = cpl_frame_new();
                error = cpl_frame_set_filename(f, "/dev/null");
                cpl_test_eq_error(error, CPL_ERROR_NONE);
                error = cpl_frame_set_tag(f, "RECIPE_DUMMY_TAG");
                cpl_test_eq_error(error, CPL_ERROR_NONE);
                error = cpl_frameset_insert(recipe->frames, f);
                cpl_test_eq_error(error, CPL_ERROR_NONE);

                /* Call recipe and expect non-zero return code */
                cpl_test( recipe_exec(plugin) );
                error = cpl_error_get_code();
                /* Expect also the CPL error code to be set */
                cpl_test_error( error );
                cpl_test( error );

                error = cpl_frameset_erase_frame(recipe->frames, f);
                cpl_test_eq_error(error, CPL_ERROR_NONE);

            } while (0);

#ifdef IRPLIB_TEST_RANDOM_SOF
            recipe_sof_test_devfile(plugin, DEV_RANDOM, nstr, astr);
#endif

            recipe_sof_test_devfile(plugin, "/dev/null", nstr, astr);

            recipe_sof_test_devfile(plugin, ".", nstr, astr);

            recipe_sof_test_image_empty(plugin, nstr, astr);

            recipe_sof_test_local(plugin);

            cpl_msg_set_level(msg_level);

        }

        cpl_frameset_delete(recipe->frames);

        error = recipe_deinit(plugin);
        cpl_test_eq_error(error, CPL_ERROR_NONE);
    }

    if (stream != stdout) fclose(stream);

    return 0;
}

/**@}*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Set unset parameters to default value or via the env
  @param    self    The parameter list
  @return   void
  @note This is a modification of params_parse_config_postprocess() from esorex

  The function initializes the provided parameter list by setting the current
  parameter values to the default parameter values.
 */
/*----------------------------------------------------------------------------*/
static void recipe_parameterlist_set(cpl_parameterlist * self)
{

    cpl_parameter * p = cpl_parameterlist_get_first(self);

    for (; p != NULL; p = cpl_parameterlist_get_next(self)) {

        const char * envvar;
        const char * svalue;

        /* FIXME: Needed ? */
        if (cpl_parameter_get_default_flag(p)) continue;

        cpl_msg_debug(cpl_func, __FILE__ " line %u: OK", __LINE__);

        envvar = cpl_parameter_get_alias(p, CPL_PARAMETER_MODE_ENV);
        svalue = envvar ? getenv(envvar) : NULL;

        switch (cpl_parameter_get_type(p)) {
        case CPL_TYPE_BOOL: {
            const int value
                = svalue ? atoi(svalue) : cpl_parameter_get_default_bool(p);
            cpl_parameter_set_bool(p, value);
            break;
        }
        case CPL_TYPE_INT: {
            const int value
                = svalue ? atoi(svalue) : cpl_parameter_get_default_int(p);
            cpl_parameter_set_int(p, value);
            break;
        }
        case CPL_TYPE_DOUBLE: {
            const double value
                = svalue ? atof(svalue) : cpl_parameter_get_default_double(p);
            cpl_parameter_set_double(p, value);
            break;
        }
        case CPL_TYPE_STRING:
            {
                const char * s_default = cpl_parameter_get_default_string(p);
                /* Replace NULL with "" */
                const char * value
                    = svalue ? svalue : (s_default ? s_default : "");
                cpl_parameter_set_string(p, value);
                break;
            }

        default:
            assert( 0 ); /* It is a testing error to reach this point */
        }
    }
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Invoke the recipe with the supplied TAGS using the supplied file
  @param    plugin   The plugin
  @param    filename The filename to use
  @param    nstr   The number of strings
  @param    astr   The array of tags
  @return   void

 */
/*----------------------------------------------------------------------------*/
static void recipe_sof_test_devfile(cpl_plugin * plugin, const char * filename,
                                    size_t nstr, const char *astr[])
{
    cpl_recipe * recipe  = (cpl_recipe*)plugin;
    int       (*recipe_exec) (cpl_plugin *);
    cpl_frameset * copy;
    cpl_error_code error;
    size_t i;


    if (nstr < 1) return;
    if (filename == NULL) return;

    cpl_msg_info(cpl_func, "Testing recipe with %u %s as input ",
                 (unsigned)nstr, filename);

    for (i = 0; i < nstr; i++) {
        cpl_frame * f = cpl_frame_new();

        error = cpl_frame_set_filename(f, filename);
        cpl_test_eq_error(error, CPL_ERROR_NONE);

        error = cpl_frame_set_tag(f, astr[i]);
        cpl_test_eq_error(error, CPL_ERROR_NONE);

        error = cpl_frameset_insert(recipe->frames, f);
        cpl_test_eq_error(error, CPL_ERROR_NONE);
    }

    copy = cpl_frameset_duplicate(recipe->frames);

    recipe_exec = cpl_plugin_get_exec(plugin);
    cpl_test( recipe_exec != NULL);

    if (recipe_exec != NULL) {

        /* Call recipe and expect non-zero return code */
        cpl_test( recipe_exec(plugin) );
        error = cpl_error_get_code();
        /* Expect also the CPL error code to be set */
        cpl_test_error( error );
        cpl_test( error );

        recipe_frameset_test_frameset_diff(recipe->frames, copy);

        recipe_frameset_empty(recipe->frames);
    }

    cpl_frameset_delete(copy);

    return;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Invoke the recipe with the supplied TAGS using empty images
  @param    plugin The plugin
  @return   void

 */
/*----------------------------------------------------------------------------*/
static void recipe_sof_test_image_empty(cpl_plugin * plugin, size_t nstr,
                                        const char *astr[])
{
    cpl_recipe * recipe  = (cpl_recipe*)plugin;
    int       (*recipe_exec) (cpl_plugin *);
    cpl_frameset * copy;
    cpl_error_code error;
    size_t i;
    cpl_image * iempty;


    if (nstr < 1) return;

    cpl_msg_info(cpl_func, "Testing recipe with %u empty images as input ",
                 (unsigned)nstr);

    iempty = cpl_image_new(13, 17, CPL_TYPE_FLOAT);
    cpl_test_nonnull(iempty);

    for (i = 0; i < nstr; i++) {
        cpl_frame * f = cpl_frame_new();
        char * rawname = cpl_sprintf("%s-raw%05u.fits",
                                     cpl_plugin_get_name(plugin),
                                     (unsigned)(i+1));

        error = cpl_image_save(iempty, rawname,CPL_BPP_IEEE_FLOAT, NULL,
                               CPL_IO_DEFAULT);
        cpl_test_eq_error(error, CPL_ERROR_NONE);

        error = cpl_frame_set_filename(f, rawname);
        cpl_test_eq_error(error, CPL_ERROR_NONE);

        error = cpl_frame_set_tag(f, astr[i]);
        cpl_test_eq_error(error, CPL_ERROR_NONE);

        error = cpl_frameset_insert(recipe->frames, f);
        cpl_test_eq_error(error, CPL_ERROR_NONE);

        cpl_free(rawname);
    }
    cpl_image_delete(iempty);

    copy = cpl_frameset_duplicate(recipe->frames);

    recipe_exec = cpl_plugin_get_exec(plugin);
    cpl_test(recipe_exec != NULL);

    if (recipe_exec != NULL) {
        const cpl_frame * frame;
        cpl_frameset_iterator * iterator = NULL;
        int retstat;

        /* Call recipe and expect consistency between return code and
           CPL error */

        retstat = recipe_exec(plugin);
        error = cpl_error_get_code();
        /* Expect also the CPL error code to be set */
        if (error == 0) {
            cpl_test_zero(retstat);
        } else {
            cpl_test(retstat);
        }
        cpl_test_error( error );

        recipe_frameset_test_frameset_diff(recipe->frames, copy);

        for (frame = irplib_frameset_get_first_const(&iterator, recipe->frames);
             frame != NULL;
             frame = irplib_frameset_get_next_const(iterator))
            {
                cpl_test_zero( remove(cpl_frame_get_filename(frame)) );
            }
        cpl_frameset_iterator_delete(iterator);

        recipe_frameset_empty(recipe->frames);
    }

    cpl_frameset_delete(copy);

    return;
}


/*----------------------------------------------------------------------------*/
/**
  @brief  Check if the the env-variable has an SOF from the plugin
  @param  plugin   The plugin
  @param  envname  The environment variable with the directory to load from
  @return True iff there is an SOF in the directory of the env

 */
/*----------------------------------------------------------------------------*/
cpl_boolean irplib_plugin_has_sof_from_env(const cpl_plugin * plugin,
                                           const char * envname)
{
    const char      * recipename = cpl_plugin_get_name(plugin);
    const char      * sof_path   = envname ? getenv(envname) : NULL;
    cpl_frameset    * frames;
    char            * sof_name;
    const cpl_frame * ffirst;

    cpl_ensure(plugin  != NULL, CPL_ERROR_NULL_INPUT, CPL_FALSE);
    cpl_ensure(envname != NULL, CPL_ERROR_NULL_INPUT, CPL_FALSE);
    cpl_ensure(recipename != NULL, CPL_ERROR_DATA_NOT_FOUND, CPL_FALSE);
    cpl_ensure(!cpl_error_get_code(), cpl_error_get_code(), CPL_FALSE);

    if (sof_path == NULL) return CPL_FALSE;

    sof_name = cpl_sprintf("%s/%s.sof", sof_path, recipename);

    frames = cpl_frameset_new();
    recipe_frameset_load(frames, sof_name);

    ffirst = cpl_frameset_get_position_const(frames, 0);

    cpl_free(sof_name);
    cpl_frameset_delete(frames);

    cpl_ensure(!cpl_error_get_code(), cpl_error_get_code(), CPL_FALSE);

    return ffirst ? CPL_TRUE : CPL_FALSE;

}

/*----------------------------------------------------------------------------*/
/**
  @brief    Use getenv() to look for SOFs
  @param    plugin The plugin
  @return   void

 */
/*----------------------------------------------------------------------------*/
static void recipe_sof_test_from_env(cpl_plugin * plugin)
{
    cpl_recipe * recipe  = (cpl_recipe*)plugin;
    const char * recipename = cpl_plugin_get_name(plugin);
    const char * var_name = "RECIPE_SOF_PATH";
    const char * sof_path = getenv(var_name);

    char * sof_name;

    if (sof_path == NULL) {
        cpl_msg_warning(cpl_func, "Environment variable %s is unset: "
                        "No SOFs to check", var_name);
        return;
    }

    cpl_msg_debug(cpl_func, "Checking for SOFs in %s", sof_path);

    cpl_test_nonnull( recipename );
    if (recipename == NULL) return;

    sof_name = cpl_sprintf("%s/%s.sof", sof_path, recipename);

    cpl_msg_debug(cpl_func, "Checking for SOF %s", sof_name);
    
    recipe_frameset_load(recipe->frames, sof_name);

    if (!cpl_frameset_is_empty(recipe->frames)) {

        int          (*recipe_exec  ) (cpl_plugin *);
        cpl_frameset * copy = cpl_frameset_duplicate(recipe->frames);

        recipe_exec   = cpl_plugin_get_exec(plugin);
        cpl_test(recipe_exec != NULL);

        if (recipe_exec != NULL) {
            cpl_error_code error;
            cpl_msg_info(cpl_func,"Checking handling of SOF: %s", sof_name);

            /* Call recipe and expect zero return code */
            cpl_test_zero( recipe_exec(plugin) );
            /* Expect also the CPL error code to be clear */
            cpl_test_error(CPL_ERROR_NONE);

            error = cpl_dfs_update_product_header(recipe->frames);
            cpl_test_eq_error(error, CPL_ERROR_NONE);

            recipe_frameset_test_frameset_diff(recipe->frames, copy);

            recipe_frameset_empty(recipe->frames);
        }

        cpl_frameset_delete(copy);

    }

    cpl_free(sof_name);

    return;
}



/*----------------------------------------------------------------------------*/
/**
  @brief    Look for SOF an SOF in ./<recipename>.sof
  @param    plugin The plugin
  @return   void

 */
/*----------------------------------------------------------------------------*/
static void recipe_sof_test_local(cpl_plugin * plugin)
{
    cpl_recipe * recipe  = (cpl_recipe*)plugin;
    const char * recipename = cpl_plugin_get_name(plugin);
    char * sof_name = cpl_sprintf("%s.sof", recipename);

    cpl_msg_debug(cpl_func, "Checking for SOF %s", sof_name);
    
    recipe_frameset_load(recipe->frames, sof_name);

    if (!cpl_frameset_is_empty(recipe->frames)) {

        int          (*recipe_exec  ) (cpl_plugin *);
        cpl_frameset * copy = cpl_frameset_duplicate(recipe->frames);

        recipe_exec   = cpl_plugin_get_exec(plugin);
        cpl_test(recipe_exec != NULL);

        if (recipe_exec != NULL) {
            cpl_error_code error;

            cpl_msg_info(cpl_func,"Checking handling of SOF: %s", sof_name);

            /* Call recipe and expect zero return code */
            cpl_test_zero( recipe_exec(plugin) );
            /* Expect also the CPL error code to be clear */
            cpl_test_error(CPL_ERROR_NONE);

            error = cpl_dfs_update_product_header(recipe->frames);
            cpl_test_eq_error( error, CPL_ERROR_NONE );

            recipe_frameset_test_frameset_diff(recipe->frames, copy);

            recipe_frameset_empty(recipe->frames);
        }

        cpl_frameset_delete(copy);
    }

    cpl_free(sof_name);

    return;
}




/**********************************************************************/
/**  
 * @brief
 *   Create a new frame set from a @em set @em of @em frames file.
 * 
 * @param set   Frame set to be updated with the contents of @em name.
 * @param name  Input file path.
 * 
 * @return Pointer to the newly created frame set if @em set was @c NULL,
 *   or the updated set @em set. In case an error occurred the return value
 *   is @c NULL.
 *
 * @note This is a simplification of er_frameset_load() from esorex
 */   
/**********************************************************************/

static void recipe_frameset_load(cpl_frameset * set, const char *name)
{

    FILE *fp;
    char line[LINE_LEN_MAX + 1];
    char path[LINE_LEN_MAX + 1], group[LINE_LEN_MAX + 1], tag[LINE_LEN_MAX + 1];
    int line_number;

    assert( set != NULL );
    assert( name != NULL );

    fp = fopen(name, "r");
    if (fp == NULL) {
        cpl_msg_debug(cpl_func, "Unable to open SOF file '%s'", name);
        return;
    }

    /* Loop over all the lines in the set-of-frames file */
    for (line_number = 0; fgets(line, LINE_LEN_MAX, fp); line_number++) {

        cpl_frame_group grp;
        cpl_frame * frame;
        int n;

        if (line[0] == '#') continue;

        n = sscanf(line, LINE_SCAN_FMT, path, tag, group);

        if (n < 1) {
            cpl_msg_warning(cpl_func, "Spurious line no. %d in %s: %s",
                            line_number, name, line);
            break;
        }

        /* Allocate a new frame */
        frame = cpl_frame_new();

        /* Set the filename component of the frame */
        cpl_frame_set_filename(frame, path);

        /* Set the tag component of the frame (or set a default) */
        cpl_frame_set_tag(frame, n == 1 ? "" : tag);

        cpl_frameset_insert(set, frame);

        /* Set the group component of the frame (or set a default) */
        if (n < 3) continue;

        if (!strcmp(group, CPL_FRAME_GROUP_RAW_ID))
            grp = CPL_FRAME_GROUP_RAW;
        else if (!strcmp(group, CPL_FRAME_GROUP_CALIB_ID))
            grp = CPL_FRAME_GROUP_CALIB;
        else if (!strcmp(group, CPL_FRAME_GROUP_PRODUCT_ID))
            grp = CPL_FRAME_GROUP_PRODUCT;
        else
            grp = CPL_FRAME_GROUP_NONE;

        cpl_frame_set_group(frame, grp);
    }

    fclose(fp);

    return;

}


/*----------------------------------------------------------------------------*/
/**
  @brief    Retrieve a parameter from a plugin parameterlist
  @param    self      The parameterlist to get from
  @param    instrume  The recipe name, e.g. PACKAGE from config.h
  @param    recipe    The recipe name, e.g. "rrrecipe"
  @param    parameter The parameter name, e.g. "stropt"
  @return   The parameter, or NULL on error

 */
/*----------------------------------------------------------------------------*/
static
const cpl_parameter * irplib_parameterlist_get(const cpl_parameterlist * self,
                                               const char * instrume,
                                               const char * recipe,
                                               const char * parameter)
{

    char                * paramname;
    const cpl_parameter * par;


    cpl_ensure(instrume  != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(recipe    != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(parameter != NULL, CPL_ERROR_NULL_INPUT, NULL);

    paramname = cpl_sprintf("%s.%s.%s", instrume, recipe, parameter);

    par = cpl_parameterlist_find_const(self, paramname);

    if (par == NULL) (void)cpl_error_set_message(cpl_func,
                                                 cpl_error_get_code()
                                                 ? cpl_error_get_code()
                                                 : CPL_ERROR_DATA_NOT_FOUND,
                                                 "%s", paramname);

    cpl_free(paramname);
    
    return par;

}


/*----------------------------------------------------------------------------*/
/**
 * @brief
 *   Remove all frames from a frameset
 *
 * @param self  A frameset.
 *
 * @return Nothing.
 *
 * @note FIXME: Move to CPL
 *
 * @error
 *   <table class="ec" align="center">
 *     <tr>
 *       <td class="ecl">CPL_ERROR_NULL_INPUT</td>
 *       <td class="ecr">
 *         The parameter <i>self</i> is a <tt>NULL</tt> pointer.
 *       </td>
 *     </tr>
 *   </table>
 * @enderror
 *
 * The function removes all frames from @em self. Each frame
 * is properly deallocated. After calling this function @em self is
 * empty.
*/
/*----------------------------------------------------------------------------*/
static void recipe_frameset_empty(cpl_frameset * self)
{
    cpl_size i, n;

    if (self == NULL) {
        cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
        return;
    }

    n = cpl_frameset_get_size(self);
    for (i = 0; i < n; ++i)
        {
            cpl_frame * f = cpl_frameset_get_position(self, n-1-i);
            cpl_frameset_erase_frame(self, f);
        }
}


/*----------------------------------------------------------------------------*/
/**
 * @brief
 *   Test a frame for product conformance
 * @param self  A frame.
 * @return Nothing.
 * @note FIXME: Move to CPL test module
 *
 * @error
 *   <table class="ec" align="center">
 *     <tr>
 *       <td class="ecl">CPL_ERROR_NULL_INPUT</td>
 *       <td class="ecr">
 *         The parameter <i>self</i> is a <tt>NULL</tt> pointer.
 *       </td>
 *     </tr>
 *   </table>
 * @enderror
 *
*/
/*----------------------------------------------------------------------------*/
static void recipe_frameset_test_frame(const cpl_frame * self)
{

    cpl_msg_info(cpl_func, "Validating new frame: %s",
                 cpl_frame_get_filename(self));

    cpl_test_nonnull(self);

    /* Frame must be tagged */
    cpl_test_nonnull(cpl_frame_get_tag(self));

    /* New frames must be products */
    cpl_test_eq(cpl_frame_get_group(self), CPL_FRAME_GROUP_PRODUCT);

    if (cpl_frame_get_type(self) != CPL_FRAME_TYPE_PAF) {
        /* All but PAF (?) must be FITS */
        cpl_test_fits(cpl_frame_get_filename(self));
    } else {
        /* Frame must at least have a filename */
        cpl_test_nonnull(cpl_frame_get_filename(self));
    }
}

/*----------------------------------------------------------------------------*/
/**
 * @brief
 *   Test the new frames for product conformance
 * @param self  The frameset in which to test
 * @param other The reference frameset (with recipe input frames)
 * @return Nothing.
 * @note FIXME: Move to CPL test module ?
 *
 * @error
 *   <table class="ec" align="center">
 *     <tr>
 *       <td class="ecl">CPL_ERROR_NULL_INPUT</td>
 *       <td class="ecr">
 *         The parameter <i>self</i> is a <tt>NULL</tt> pointer.
 *       </td>
 *     </tr>
 *   </table>
 * @enderror
 *
*/
/*----------------------------------------------------------------------------*/
static void recipe_frameset_test_frameset_diff(const cpl_frameset * self,
                                               const cpl_frameset * other)
{

    cpl_frameset_iterator * it1 = NULL;
    cpl_frameset_iterator * it2 = NULL;
    const cpl_frame * frame = irplib_frameset_get_first_const(&it2, other);

    /* First verify that filenames in other are non-NULL */
    for (;frame != NULL; frame = irplib_frameset_get_next_const(it2)) {
        const char * file = cpl_frame_get_filename(frame);

        if (file == NULL) {
            cpl_test_nonnull(cpl_frame_get_filename(frame));
            break;
        }
    }
    cpl_frameset_iterator_delete(it2);
    it2 = NULL;
    if (frame != NULL) return;

    frame = irplib_frameset_get_first_const(&it1, self);

    for (;frame != NULL; frame = irplib_frameset_get_next_const(it1)) {
        const cpl_frame * cmp;
        const char * file = cpl_frame_get_filename(frame);

        if (file == NULL) {
            cpl_test_nonnull(cpl_frame_get_filename(frame));
            continue;
        }

        cmp = irplib_frameset_get_first_const(&it2, other);
        for (;cmp != NULL; cmp = irplib_frameset_get_next_const(it2)) {
            const char * cfile = cpl_frame_get_filename(cmp);

            if (!strcmp(file, cfile)) break;

        }
        cpl_frameset_iterator_delete(it2);
        it2 = NULL;
        if (cmp == NULL) {
            /* frame is new */

            cpl_test_eq(cpl_frame_get_group(frame), CPL_FRAME_GROUP_PRODUCT);
            recipe_frameset_test_frame(frame);
        }
    }

    cpl_frameset_iterator_delete(it1);
}
