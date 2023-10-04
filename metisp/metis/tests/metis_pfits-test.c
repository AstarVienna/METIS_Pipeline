/* $Id: metis_dfs-test.c,v 1.6 2013-03-25 11:46:49 cgarcia Exp $
 *
 * This file is part of the METIS Pipeline
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
 * $Author: cgarcia $
 * $Date: 2013-03-25 11:46:49 $
 * $Revision: 1.6 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#define _POSIX_C_SOURCE   200809L   /* For mkdtemp()       */
#define _DARWIN_C_SOURCE

#include <unistd.h>

#include <stdlib.h>
#include <string.h>

#include <cpl.h>

#include <metis_dfs.h>

/*----------------------------------------------------------------------------*/
/**
 * @defgroup metis_dfs_test  Unit test of metis_dfs
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Unit test of metis_pfits_get_arcfile
 */
/*----------------------------------------------------------------------------*/
static void test_pfits_arcfile(void)
{
    const char *const fctid = "test_pfits";
//    const char *const test_subject = "metis_pfits_get_arcfile";
    cpl_errorstate prestate = cpl_errorstate_get();

    /* Create a property list with relevant keywords */
    const char *const arcfile_content = "arcfile.fits";
    cpl_propertylist * plist;
    plist = cpl_propertylist_new();
    cpl_propertylist_append_string(plist, "ARCFILE", arcfile_content);

    /* Create a unique temporary directory to store FITS files.
       This prevents race conditions if other tests use the same
       filenames or even if this test is executed more than once
       in parallel */
    char testdir[50];
    strncpy(testdir, "test-metis_pfits_XXXXXX\0", 31);
    if(mkdtemp(testdir) == NULL) {
        cpl_msg_error(fctid, "Failed to create temporary directory %s",testdir);
        cpl_errorstate_dump(prestate, CPL_FALSE, NULL);
        cpl_propertylist_delete(plist);
        cpl_end();
        exit(EXIT_FAILURE);
    }

    /* Save header */
    char filename[100];
    char * filepos = stpncpy(filename, testdir, 30);
    strncpy(filepos, "/proplist.fits\0", 15);
    cpl_propertylist_save(plist, filename, CPL_IO_CREATE);
    cpl_propertylist_delete(plist);

    /* Read keyword from header */
    cpl_propertylist * plist_read;
    plist_read = cpl_propertylist_load(filename, 0);
    cpl_test_nonnull(plist_read);

    /* Compare the original and read values */
    const char * arcfile_content_read =
         cpl_propertylist_get_string(plist_read, "ARCFILE");
    cpl_test_nonnull(arcfile_content_read);
    cpl_test_eq_string(arcfile_content, arcfile_content_read);
    cpl_propertylist_delete(plist_read);

    /* Remove temporary files (recursive remove can be done using nftw() */
    if(remove(filename)) {
      cpl_msg_error(fctid, "Cannot remove file %s", filename);
      cpl_end();
      exit(EXIT_FAILURE);
    }
    if(remove(testdir)) {
      cpl_msg_error(fctid, "Cannot remove temporary directory %s", testdir);
      cpl_end();
      exit(EXIT_FAILURE);
    }

    return;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Unit tests of metis_dfs module
 */
/*----------------------------------------------------------------------------*/

int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT,CPL_MSG_WARNING);

    test_pfits_arcfile();

    return cpl_test_end(0);
}

/**@}*/
