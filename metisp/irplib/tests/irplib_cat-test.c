/* $Id: irplib_cat-test.c,v 1.10 2013-01-29 08:43:33 jtaylor Exp $
 *
 * This file is part of the ESO Common Pipeline Library
 * Copyright (C) 2001-2008 European Southern Observatory
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
 * $Author: jtaylor $
 * $Date: 2013-01-29 08:43:33 $
 * $Revision: 1.10 $
 * $Name: not supported by cvs2svn $
 */

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cpl_test.h>

#include "irplib_cat.h"

/*-----------------------------------------------------------------------------
                                   Static functions
 -----------------------------------------------------------------------------*/
static void irplib_cat_all_test(void);

/*-----------------------------------------------------------------------------
                                  Main
 -----------------------------------------------------------------------------*/
int main (void)
{

    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    irplib_cat_all_test();

    return cpl_test_end(0);
}

static void irplib_cat_all_test(void)
{
    cpl_propertylist * prop_wcs;
    cpl_wcs          * wcs = NULL;
    double             ra1, ra2, dec1, dec2;

    /* Create WCS object */
    prop_wcs = cpl_propertylist_new();
    cpl_test_nonnull(prop_wcs);
    cpl_propertylist_append_double(prop_wcs, "CRVAL1", 0.);
    cpl_propertylist_append_double(prop_wcs, "CRVAL2", 0.);
    cpl_propertylist_append_int(prop_wcs, "CRPIX1", 1);
    cpl_propertylist_append_int(prop_wcs, "CRPIX2", 1);
    cpl_propertylist_append_double(prop_wcs, "CD1_1", .001);
    cpl_propertylist_append_double(prop_wcs, "CD1_2", 0.);
    cpl_propertylist_append_double(prop_wcs, "CD2_1", 00.);
    cpl_propertylist_append_double(prop_wcs, "CD2_2", .001);
    cpl_propertylist_append_int(prop_wcs, "NAXIS", 2);
    cpl_propertylist_append_int(prop_wcs, "NAXIS1", 1000);
    cpl_propertylist_append_int(prop_wcs, "NAXIS2", 1000);
    wcs = cpl_wcs_new_from_propertylist(prop_wcs);
    if(cpl_error_get_code() == CPL_ERROR_NO_WCS)
    {
        cpl_msg_warning(__func__,"No WCS present. Tests disabled");
        cpl_test_error(CPL_ERROR_NO_WCS);
        cpl_test_null(wcs);
        cpl_propertylist_delete(prop_wcs);
        return;
    }
    cpl_test_nonnull(wcs);

    irplib_cat_get_image_limits(wcs, 0., &ra1, &ra2, &dec1, &dec2);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_abs(ra1, 0.00, DBL_EPSILON);
    cpl_test_abs(ra2,  0.99,  DBL_EPSILON);
    cpl_test_abs(dec1,  0.00, DBL_EPSILON);
    cpl_test_abs(dec2,  0.99,  DBL_EPSILON);

    /* Free */
    cpl_wcs_delete(wcs);
    cpl_propertylist_delete(prop_wcs);
}
