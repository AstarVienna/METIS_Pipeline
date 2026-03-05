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

#include <irplib_framelist.h>


/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_framelist_test Testing of the IRPLIB framelist object
 */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/**
   @brief   Unit tests of framelist module
**/
/*----------------------------------------------------------------------------*/

int main(void)
{

    irplib_framelist * flist;
    irplib_framelist * nulllist;
    cpl_frame        * frm;
    cpl_frameset     * fset;
    cpl_frameset     * nullset;
    int i;

    /* Initialize CPL for unit testing */
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    /* Some NULL tests */

    irplib_framelist_delete(NULL);
    cpl_test_error(CPL_ERROR_NONE);

    nulllist = irplib_framelist_cast(NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_null(nulllist);

    nullset = irplib_frameset_cast(NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_null(nullset);

    irplib_framelist_empty(NULL);
    cpl_test_error(CPL_ERROR_NONE);

    i = irplib_framelist_get_size(NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_leq(i, -1);


    /* Some tests on an empty list */
    flist = irplib_framelist_new();

    cpl_test_zero(irplib_framelist_get_size(flist));

    irplib_framelist_empty(flist);

    cpl_test_zero(irplib_framelist_get_size(flist));

    fset = irplib_frameset_cast(flist);

    irplib_framelist_delete(flist);

    flist = irplib_framelist_cast(fset);

    cpl_test_zero(irplib_framelist_get_size(flist));

    irplib_framelist_delete(flist);

    frm = cpl_frame_new();
    cpl_frame_set_filename(frm, "test.fits");
    cpl_frame_set_tag(frm, "TEST");
    cpl_frameset_insert(fset, frm);

    flist = irplib_framelist_cast(fset);

    cpl_test_eq(irplib_framelist_get_size(flist), 1);

    irplib_framelist_delete(flist);

    cpl_frameset_insert(fset, cpl_frame_duplicate(frm));
    flist = irplib_framelist_cast(fset);

    cpl_test_eq(irplib_framelist_get_size(flist), 2);

    cpl_frameset_delete(fset);
    irplib_framelist_delete(flist);

    return cpl_test_end(0);
}
