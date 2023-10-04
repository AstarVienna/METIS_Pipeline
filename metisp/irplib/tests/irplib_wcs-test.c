/* $Id: irplib_wcs-test.c,v 1.9 2013-01-29 08:43:33 jtaylor Exp $
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
 * $Revision: 1.9 $
 * $Name: not supported by cvs2svn $
 */

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cpl_test.h>

#include "irplib_wcs.h"
#include "math.h"

/*-----------------------------------------------------------------------------
                                   Static functions
 -----------------------------------------------------------------------------*/
static void irplib_wcs_all_test(int);

static void irplib_wcs_mjd_test(void);

static void irplib_wcs_great_circle_dist_test(void);

/*-----------------------------------------------------------------------------
                                  Main
 -----------------------------------------------------------------------------*/
int main (void)
{

    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    irplib_wcs_all_test(-1);
    irplib_wcs_all_test(0);
    irplib_wcs_all_test(1);

    irplib_wcs_mjd_test();

    irplib_wcs_great_circle_dist_test();

    return cpl_test_end(0);
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Test the WCS functions
  @param    naxismode  Negative for too small, zero for missing, positive for OK
  @return   void
 */
/*----------------------------------------------------------------------------*/
static void irplib_wcs_all_test(int naxismode)
{
    const double       xorig = 1.0;
    const double       yorig = 2.0;
    double             xnew,ynew;
    double             ra, dec;
    cpl_propertylist * prop_wcs; 
    cpl_wcs *          wcs = NULL;
    cpl_error_code     error;

    
    /* Create WCS object */
    prop_wcs = cpl_propertylist_new();
    cpl_test_nonnull(prop_wcs);

    if (naxismode < 0) { /* NAXIS inconsistent with WCS keys */
        cpl_propertylist_append_int(prop_wcs, "NAXIS", 1);
        cpl_propertylist_append_int(prop_wcs, "NAXIS1", 42);
    }
    else if (naxismode > 0) {
        cpl_propertylist_append_int(prop_wcs, "NAXIS", 2);
        cpl_propertylist_append_int(prop_wcs, "NAXIS1", 42);
        cpl_propertylist_append_int(prop_wcs, "NAXIS2", 42);
    }
    cpl_propertylist_append_double(prop_wcs, "CRVAL1", 10.);
    cpl_propertylist_append_double(prop_wcs, "CRVAL2", 20.);
    cpl_propertylist_append_int(prop_wcs, "CRPIX1", 1);
    cpl_propertylist_append_int(prop_wcs, "CRPIX2", 2);
    cpl_propertylist_append_double(prop_wcs, "CD1_1", 10.);
    cpl_propertylist_append_double(prop_wcs, "CD1_2", 11.);
    cpl_propertylist_append_double(prop_wcs, "CD2_1", 13.);
    cpl_propertylist_append_double(prop_wcs, "CD2_2", 14.);

    cpl_test_error(CPL_ERROR_NONE);

    wcs = cpl_wcs_new_from_propertylist(prop_wcs);
    cpl_propertylist_delete(prop_wcs);

    if (cpl_error_get_code() == CPL_ERROR_NO_WCS) {

        cpl_msg_warning(cpl_func, "No WCS present. Tests disabled");
        cpl_test_error(CPL_ERROR_NO_WCS);
        cpl_test_null(wcs);

    } else {

        cpl_test_nonnull(wcs);
    
        /* Test that a simple call to xytoradec does not fail*/
        error = irplib_wcs_xytoradec(wcs, xorig, yorig, &ra, &dec);
        cpl_test_eq_error(error, CPL_ERROR_NONE);
    
        /* Get the transformation back and compare */
        error = irplib_wcs_radectoxy(wcs, ra, dec, &xnew, &ynew);
        cpl_test_eq_error(error, CPL_ERROR_NONE);

        cpl_test_abs(xnew, xorig, 2.0 * DBL_EPSILON);
        cpl_test_abs(ynew, yorig, 2.0 * DBL_EPSILON);

        /* Error testing */

        error = irplib_wcs_xytoradec(wcs, xorig, yorig, NULL, &dec);
        cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);
    
        error = irplib_wcs_radectoxy(wcs, ra, dec, NULL, &ynew);
        cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

        error = irplib_wcs_xytoradec(wcs, xorig, yorig, &ra, NULL);
        cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);
    
        error = irplib_wcs_radectoxy(wcs, ra, dec, &xnew, NULL);
        cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

        cpl_wcs_delete(wcs);
    
    }    

    /* Error testing */

    error = irplib_wcs_xytoradec(NULL, xorig, yorig, &ra, &dec);
    cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);
    
    error = irplib_wcs_radectoxy(NULL, ra, dec, &xnew, &ynew);
    cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);


}


/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Test the MJD functions
  @return   void
 */
/*----------------------------------------------------------------------------*/
static void irplib_wcs_mjd_test(void)
{

    /* Matching example from some VLT header */
    const char * iso8601  = "2010-07-13T23:24:39.284";
    const double mjd  = 55390.97545467;

    /* Two equal dates */
    const char * iso8601a = "2010-07-13T24:00:00";
    const char * iso8601b = "2010-07-14T00:00:00.000";

    const double mstol = 1e-3/86400.0; /* 1ms tolerance in MJD */
    int year, day, month, hour, minute;
    double second;
    double tmjd, tmjd2;
    cpl_error_code error;

    /* The MJD counts the number of days since November 17, 1858 */
    /* Test 1a: Conversion of MJD == 0 */
    error = irplib_wcs_iso8601_from_mjd(&year, &month, &day, &hour,
                                        &minute, &second, 0.0);
    cpl_test_eq_error(error, CPL_ERROR_NONE);

    cpl_test_eq(year,  1858);
    cpl_test_eq(month,   11);
    cpl_test_eq(day,     17);
    cpl_test_eq(hour,     0);
    cpl_test_eq(minute,   0);
    cpl_test_abs(second, 0.0, 2.0 * DBL_EPSILON);

    /* Test 1b: - and convert back */
    error = irplib_wcs_mjd_from_iso8601(&tmjd, year, month, day, hour, minute,
                                        second);
    cpl_test_eq_error(error, CPL_ERROR_NONE);

    cpl_test_abs(tmjd, 0.0, 2.0 * DBL_EPSILON);

    /* Test 2: Conversion back and forth of some recent date */
    error = irplib_wcs_iso8601_from_string(&year, &month, &day, &hour,
                                           &minute, &second, iso8601);
    cpl_test_eq_error(error, CPL_ERROR_NONE);

    error = irplib_wcs_mjd_from_iso8601(&tmjd, year, month, day, hour, minute,
                                        second);
    cpl_test_eq_error(error, CPL_ERROR_NONE);

    cpl_test_abs(mjd, tmjd, mstol);

    error = irplib_wcs_mjd_from_string(&tmjd, iso8601);
    cpl_test_eq_error(error, CPL_ERROR_NONE);

    cpl_test_abs(mjd, tmjd, mstol);

    error = irplib_wcs_iso8601_from_mjd(&year, &month, &day, &hour,
                                        &minute, &second, mjd);
    cpl_test_eq_error(error, CPL_ERROR_NONE);

    error = irplib_wcs_mjd_from_iso8601(&tmjd, year, month, day, hour, minute,
                                        second);
    cpl_test_eq_error(error, CPL_ERROR_NONE);

    cpl_test_abs(mjd, tmjd, 2.0 * DBL_EPSILON);

    /* Test 3: 24:00:00 == 00.00.00 + 1 day */
    error = irplib_wcs_iso8601_from_string(&year, &month, &day, &hour,
                                           &minute, &second, iso8601a);
    cpl_test_eq_error(error, CPL_ERROR_NONE);

    error = irplib_wcs_mjd_from_iso8601(&tmjd, year, month, day, hour, minute,
                                        second);
    cpl_test_eq_error(error, CPL_ERROR_NONE);

    error = irplib_wcs_iso8601_from_string(&year, &month, &day, &hour,
                                           &minute, &second, iso8601b);
    cpl_test_eq_error(error, CPL_ERROR_NONE);

    error = irplib_wcs_mjd_from_iso8601(&tmjd2, year, month, day, hour, minute,
                                        second);
    cpl_test_eq_error(error, CPL_ERROR_NONE);

    cpl_test_abs(tmjd, tmjd2, 2.0 * DBL_EPSILON);

    /* Test 4: Do not allow days from y10k */
    error = irplib_wcs_iso8601_from_string(&year, &month, &day, &hour,
                                           &minute, &second,
                                           "10000-07-13T23:24:39.284");
    cpl_test_eq_error(error, CPL_ERROR_ILLEGAL_INPUT);

    /* Test 5: Verify validation of length of a non-leap year month */
    error = irplib_wcs_iso8601_from_string(&year, &month, &day, &hour,
                                           &minute, &second,
                                           "2010-02-29T23:24:39.284");
    cpl_test_eq_error(error, CPL_ERROR_ILLEGAL_INPUT);

    /* Test 6: NULL pointer checking */
    error = irplib_wcs_mjd_from_iso8601(NULL, year, month, day, hour, minute,
                                        second);
    cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

    error = irplib_wcs_iso8601_from_string(NULL, &month, &day, &hour,
                                           &minute, &second, iso8601);
    cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

    error = irplib_wcs_iso8601_from_string(&year, NULL, &day, &hour,
                                           &minute, &second, iso8601);
    cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

    error = irplib_wcs_iso8601_from_string(&year, &month, NULL, &hour,
                                           &minute, &second, iso8601);
    cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

    error = irplib_wcs_iso8601_from_string(&year, &month, &day, NULL,
                                           &minute, &second, iso8601);
    cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

    error = irplib_wcs_iso8601_from_string(&year, &month, &day, &hour,
                                           NULL, &second, iso8601);
    cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

    error = irplib_wcs_iso8601_from_string(&year, &month, &day, &hour,
                                           &minute, NULL, iso8601);
    cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

    error = irplib_wcs_iso8601_from_string(&year, &month, &day, &hour,
                                           &minute, &second, NULL);
    cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

    error = irplib_wcs_iso8601_from_mjd(NULL, &month, &day, &hour,
                                        &minute, &second, mjd);
    cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

    error = irplib_wcs_iso8601_from_mjd(&year, NULL, &day, &hour,
                                        &minute, &second, mjd);
    cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

    error = irplib_wcs_iso8601_from_mjd(&year, &month, NULL, &hour,
                                        &minute, &second, mjd);
    cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

    error = irplib_wcs_iso8601_from_mjd(&year, &month, &day, NULL,
                                        &minute, &second, mjd);
    cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

    error = irplib_wcs_iso8601_from_mjd(&year, &month, &day, &hour,
                                        NULL, &second, mjd);
    cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

    error = irplib_wcs_iso8601_from_mjd(&year, &month, &day, &hour,
                                        &minute, NULL, mjd);
    cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Test the function, using trivial cases
  @return   void
 */
/*----------------------------------------------------------------------------*/
static void irplib_wcs_great_circle_dist_test(void)
{
    double dist1, dist2;

    /* Commutative */
    dist1 = irplib_wcs_great_circle_dist(12.0, 34.0, 56.0, 78.0);
    dist2 = irplib_wcs_great_circle_dist(56.0, 78.0, 12.0, 34.0);
    cpl_test_abs(dist1, dist2, 0.0);

    for (int j = 0; j <= 360; j += 4) {
        const double ra2 = CPL_MATH_E + (double)j;

        for (int i = 0; i < 180; i++) {
            const double ra1  = (double)i;
            const double dec1 = (double)i;

            /* Poles Apart */
            dist1 = irplib_wcs_great_circle_dist(ra2, 90.0, ra2 + ra1, -90.0);
            cpl_test_abs(dist1, 180.0, 0.0);

            /* Equatorial */
            dist1 = irplib_wcs_great_circle_dist(ra2, 0.0, ra2 + ra1, 0.0);
            cpl_test_abs(dist1, ra1, 2560.0 * DBL_EPSILON);

            /* "I will go on the slightest errand now to the Antipodes..." */
            dist1 = irplib_wcs_great_circle_dist(ra2, dec1, ra2 + 180.0, -dec1);
            cpl_test_abs(dist1, 180.0, 30.0 * FLT_EPSILON);

        }
        /* Meridional */
        for (int i = -90; i <= 90; i += 3) {
            const double dec1 = (double)i;
            for (int k = 0; k <= 90; k += 3) {
                const double dec2 = (double)k;

                dist1 = irplib_wcs_great_circle_dist(ra2, dec1, ra2, dec2);
                cpl_test_abs(dist1, fabs(dec1 - dec2), 1024.0 * DBL_EPSILON);
            }

            dist1 = irplib_wcs_great_circle_dist(ra2, dec1, ra2 + 180.0,
                                                 90.0 - dec1);
            cpl_test_abs(dist1, 90.0, 512.0 * DBL_EPSILON);
        }
    }
}
