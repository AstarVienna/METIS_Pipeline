/* $Id: irplib_wcs.c,v 1.8 2010-10-07 14:10:55 llundin Exp $
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
 * $Author: llundin $
 * $Date: 2010-10-07 14:10:55 $
 * $Revision: 1.8 $
 * $Name: not supported by cvs2svn $
 */

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "irplib_wcs.h"

#include <math.h>

/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_wcs   Functions related to WCS
 */
/*----------------------------------------------------------------------------*/

static cpl_error_code irplib_wcs_is_iso8601(int, int, int, int, int, double);

/* Standard year-2000 form: CCYY-MM-DD[Thh:mm:ss[.sss...]] */
#define IRPLIB_ISO8601_FORMAT "%4d-%2d-%2dT%2d:%2d:%lf"

/**@{*/

/*----------------------------------------------------------------------------*/
/**
  @brief Convert x, y coordinates (physical) to RA, DEC (World)
  @param wcs    The World Coordinate System solution
  @param x      The X coordinate (Physical)
  @param y      The Y coordinate (Physical)
  @param ra     The RA coordinate (World) (returned)
  @param dec    The DEC coordinate (World) (returned)
  @see cpl_wcs_convert()

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_wcs_xytoradec(const cpl_wcs *wcs,
                                    double         x,
                                    double         y,
                                    double        *ra,
                                    double        *dec)
{
    cpl_matrix   * xy;
    cpl_matrix   * radec  = NULL;
    cpl_array    * status = NULL;
    cpl_error_code error;

    cpl_ensure_code(ra  != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(dec != NULL, CPL_ERROR_NULL_INPUT);

    /* Load up the information */
    xy = cpl_matrix_new(1, 2);
    cpl_matrix_set(xy, 0, 0, x);
    cpl_matrix_set(xy, 0, 1, y);

    /* Call the conversion routine */
    error = cpl_wcs_convert(wcs, xy, &radec, &status, CPL_WCS_PHYS2WORLD);

    cpl_matrix_delete(xy);

    if (!error) {

        /* Pass it back now */
        *ra  = cpl_matrix_get(radec, 0, 0);
        *dec = cpl_matrix_get(radec, 0, 1);

    }

    /* Tidy and propagate error, if any */
    cpl_matrix_delete(radec);
    cpl_array_delete(status);

    return cpl_error_set_where(cpl_func);
}

/*----------------------------------------------------------------------------*/
/**
  @brief Convert RA, DEC (World) to x, y coordinates (physical)
  @param wcs    The World Coordinate System solution
  @param ra     The RA coordinate (World) (returned)
  @param dec    The DEC coordinate (World)
  @param x      The X coordinate (Physical) (returned)
  @param y      The Y coordinate (Physical) (returned)
  @see cpl_wcs_convert()

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_wcs_radectoxy(const cpl_wcs * wcs,
                                    double          ra,
                                    double          dec,
                                    double        * x,
                                    double        * y)
{
    cpl_matrix   * radec;
    cpl_matrix   * xy = NULL;
    cpl_array    * status   = NULL;
    cpl_error_code error;

    cpl_ensure_code(x != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(y != NULL, CPL_ERROR_NULL_INPUT);

    /* Feed the matrix with RA, DEC */
    radec = cpl_matrix_new(1, 2);
    cpl_matrix_set(radec, 0, 0, ra);
    cpl_matrix_set(radec, 0, 1, dec);

    error = cpl_wcs_convert(wcs, radec, &xy, &status, CPL_WCS_WORLD2PHYS);

    cpl_matrix_delete(radec);

    if (!error) {

        *x  = cpl_matrix_get(xy, 0, 0);
        *y  = cpl_matrix_get(xy, 0, 1);

    }

    /* Tidy and propagate error, if any */
    cpl_array_delete(status);
    cpl_matrix_delete(xy);

    return cpl_error_set_where(cpl_func);

}

/*----------------------------------------------------------------------------*/
/**
  @brief   Compute the great-circle distance between two points on a sphere
  @param   ra1    Right ascension of first point [degrees]
  @param   dec1   Declination of first point [degrees]
  @param   ra2    Right ascension of second point [degrees]
  @param   dec2   Declination of second point [degrees]
  @return  Non-negative distance [degrees].
  @see     http://en.wikipedia.org/wiki/Great-circle_distance (on 2005-10-23)
 */
/*----------------------------------------------------------------------------*/
double irplib_wcs_great_circle_dist(double ra1,
                                    double dec1,
                                    double ra2,
                                    double dec2)
{

    /* Convert all input from degrees to radian - and back for the result */
    const double dra  = sin( CPL_MATH_RAD_DEG * (ra2  - ra1 )/2.0 );
    const double ddec = sin( CPL_MATH_RAD_DEG * (dec2 - dec1)/2.0 );

    dec1 *= CPL_MATH_RAD_DEG;
    dec2 *= CPL_MATH_RAD_DEG;

    return 2.0 * asin(sqrt( ddec*ddec + cos(dec1)*cos(dec2)*dra*dra))
        * CPL_MATH_DEG_RAD;
}


/*----------------------------------------------------------------------------*/
/**
  @brief   Convert a date from ISO-8601 to Modified Julian Date (MJD)
  @param   pmjd   On success, the MJD
  @param   year   The ISO-8601 Year
  @param   month  The ISO-8601 Month  (1 for first)
  @param   day    The ISO-8601 Day    (1 for first)
  @param   hour   The ISO-8601 Hour   (0 for first)
  @param   minute The ISO-8601 Minute (0 for first)
  @param   second The ISO-8601 Second (0 for first)
  @return  CPL_ERROR_NONE on success, otherwise the error
  @see The conversion code in wcslib version 4.4.4

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_wcs_mjd_from_iso8601(double * pmjd, int year, int month,
                                           int day, int hour, int minute,
                                           double second)
{

    cpl_ensure_code(pmjd  != NULL,        CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(!irplib_wcs_is_iso8601(year, month, day, hour, minute,
                                           second), cpl_error_get_code());

    /* Compute MJD. */
    *pmjd = (double)((1461*(year - (12-month)/10 + 4712))/4
                     + (306*((month+9)%12) + 5)/10
                     - (3*((year - (12-month)/10 + 4900)/100))/4
                     + day - 2399904)
        + (hour + (minute + second/60.0)/60.0)/24.0;

    return CPL_ERROR_NONE;

}


/*----------------------------------------------------------------------------*/
/**
  @brief   Extract an ISO-8601 date from a string
  @param   pyear   The ISO-8601 Year
  @param   pmonth  The ISO-8601 Month  (1 for first)
  @param   pday    The ISO-8601 Day    (1 for first)
  @param   phour   The ISO-8601 Hour   (0 for first)
  @param   pminute The ISO-8601 Minute (0 for first)
  @param   psecond The ISO-8601 Second (0 for first)
  @param   iso8601 The ISO-8601 formatted string
  @return  CPL_ERROR_NONE on success, otherwise the error
  @see irplib_wcs_mjd_from_iso8601()
  @note The format must be the
        standard year-2000 form: CCYY-MM-DD[Thh:mm:ss[.sss...]]

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_wcs_iso8601_from_string(int * pyear, int * pmonth,
                                              int * pday, int * phour,
                                              int * pminute, double * psecond,
                                              const char * iso8601)
{

    int nret;

    cpl_ensure_code(pyear   != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(pmonth  != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(pday    != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(phour   != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(pminute != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(psecond != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(iso8601 != NULL, CPL_ERROR_NULL_INPUT);

    nret = sscanf(iso8601, IRPLIB_ISO8601_FORMAT, pyear, pmonth,
                  pday, phour, pminute, psecond);

    if (nret != 6) {
        return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT, "Parsed"
                                     " %d != 6: input %s is not in format %s",
                                     nret, iso8601, IRPLIB_ISO8601_FORMAT);
    }

    return irplib_wcs_is_iso8601(*pyear, *pmonth, *pday, *phour, *pminute,
                                 *psecond)
        ? cpl_error_set_where(cpl_func) : CPL_ERROR_NONE;
}


/*----------------------------------------------------------------------------*/
/**
  @brief   Convert a date from a ISO-8601 string to Modified Julian Date (MJD)
  @param   pmjd    On success, the MJD
  @param   iso8601 The ISO-8601 formatted string
  @return  CPL_ERROR_NONE on success, otherwise the error
  @see irplib_wcs_iso8601_from_string(), irplib_wcs_mjd_from_iso8601()

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_wcs_mjd_from_string(double * pmjd, const char * iso8601)
{


    int year, day, month, hour, minute;
    double second;

    return irplib_wcs_iso8601_from_string(&year, &month, &day, &hour,
                                          &minute, &second, iso8601)
        || irplib_wcs_mjd_from_iso8601(pmjd, year, month, day, hour, minute,
                                       second)
        ? cpl_error_set_where(cpl_func) : CPL_ERROR_NONE;
}



/*----------------------------------------------------------------------------*/
/**
  @brief   Convert a date from Modified Julian Date (MJD) to ISO-8601
  @param   pyear   The ISO-8601 Year
  @param   pmonth  The ISO-8601 Month  (1 for first)
  @param   pday    The ISO-8601 Day    (1 for first)
  @param   phour   The ISO-8601 Hour   (0 for first)
  @param   pminute The ISO-8601 Minute (0 for first)
  @param   psecond The ISO-8601 Second (0 for first)
  @param   mjd     The MJD
  @return  CPL_ERROR_NONE on success, otherwise the error
  @see irplib_wcs_mjd_from_iso8601()

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_wcs_iso8601_from_mjd(int * pyear, int * pmonth,
                                           int * pday, int * phour,
                                           int * pminute, double * psecond,
                                           double mjd)
{

    int jd, n4, dd;
    double t;

    cpl_ensure_code(pyear   != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(pmonth  != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(pday    != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(phour   != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(pminute != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(psecond != NULL, CPL_ERROR_NULL_INPUT);

    /* Copied from datfix() in wcslib (v. 4.4.4) */

    jd = 2400001 + (int)mjd;

    n4 =  4*(jd + ((2*((4*jd - 17918)/146097)*3)/4 + 1)/2 - 37);
    dd = 10*(((n4-237)%1461)/4) + 5;

    *pyear  = n4/1461 - 4712;
    *pmonth = (2 + dd/306)%12 + 1;
    *pday   = (dd%306)/10 + 1;

    t = mjd - (int)mjd; /* t is now days */

    t *= 24.0; /* t is now hours */
    *phour = (int)t;
    t = 60.0 * (t - *phour); /* t is now minutes */
    *pminute = (int)t;
    *psecond = 60.0 * (t - *pminute);

    /* A failure here implies that this code has a bug */
    cpl_ensure_code(!irplib_wcs_is_iso8601(*pyear, *pmonth, *pday, *phour,
                                           *pminute, *psecond),
                    CPL_ERROR_UNSPECIFIED);

    return CPL_ERROR_NONE;
}


/**@}*/

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief   Verify that the six numbers comprise a valid ISO-8601 date
  @param   year   The Year
  @param   month  The Month  (1 for first)
  @param   day    The Day    (1 for first)
  @param   hour   The Hour   (0 for first)
  @param   minute The Minute (0 for first)
  @param   second The Second (0 for first)
  @return  CPL_ERROR_NONE on valid input, otherwise CPL_ERROR_ILLEGAL_INPUT
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code irplib_wcs_is_iso8601(int year, int month,
                                            int day, int hour,
                                            int minute, double second)
{

    const cpl_boolean is_leap = (year % 4) ? CPL_FALSE : CPL_TRUE;
    const int mlen[] = {0, 31, is_leap ? 29 : 28, 31, 30, 31, 30, 31, 31, 30,
                        31, 30, 31};

    cpl_ensure_code(month > 0,            CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(month <= 12,          CPL_ERROR_ILLEGAL_INPUT);

    cpl_ensure_code(day   > 0,            CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(day   <= mlen[month], CPL_ERROR_ILLEGAL_INPUT);

    cpl_ensure_code(minute  < 60,         CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(minute  >= 0,         CPL_ERROR_ILLEGAL_INPUT);

    cpl_ensure_code(second  < 60.0,       CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(second  >= 0.0,       CPL_ERROR_ILLEGAL_INPUT);

    cpl_ensure_code(hour  >= 0,           CPL_ERROR_ILLEGAL_INPUT);
    /* 24:00:00 is valid ISO-8601 */
    cpl_ensure_code(hour  <= (minute > 0 || second > 0.0 ? 23 : 24),
                    CPL_ERROR_ILLEGAL_INPUT);

    return CPL_ERROR_NONE;
}
