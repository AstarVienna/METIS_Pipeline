/*
 * This file is part of the HDRL
 * Copyright (C) 2017 European Southern Observatory
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "hdrl_cat_seeing.h"

#include "hdrl_cat_utils_sort.h"


/*---------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_seeing   hdrl_seeing
 * @ingroup  Catalogue
 *
 * @brief    Work out the median seeing
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Work out the median seeing
 *
 * @param    ap        The current ap structure
 * @param    nrows     The number rows in the object catalogue
 * @param    ellipt    The array of ellipticities from the object catalogue
 * @param    pkht      The array of peak heights from the object catalogue
 * @param    areal     The array of areal profiles from the object catalogue
 * @param    work      A work array (should probably allocate this local at some stage)
 * @param    fwhm      The output FWHM estimate
 *
 * @return   CPL_ERROR_NONE if all went OK (Currently this is the only value).
 *
 * Description:
 * 	    The areal profiles for an array of objects is examined. The point
 *      where the areal profile falls to half its peak value is found for
 *      each object and the final seeing estimate is the median of these results
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_seeing(
		ap_t *ap, cpl_size nrows, double *ellipt, double *pkht,
        double **areal, double *work, double *fwhm)
{
    double log5t = log(0.5 / ap->thresh);
    double log2  = log(2.);

    /* Do the seeing calculation */
    cpl_size ii = 0;
    for (cpl_size i = 0; i < nrows; i++) {
        if (ellipt[i] < 0.2 && pkht[i] < 30000. && pkht[i] > 10. * ap->thresh) {

            double   aper    = (log5t + log(pkht[i])) / log2 + 1.;
            cpl_size iaper   = (cpl_size)aper;

            double   delaper = aper - iaper;

            if (iaper > 0 && iaper < NAREAL && areal[1][i] > 0.) {

                double area = (1. - delaper) * areal[iaper - 1][i] + delaper * areal[iaper][i];

                work[ii++] = CPL_MATH_2_SQRTPI * sqrt(area);
            }
        }
    }

    /* Sort the resulting array and choose a location that allows for contamination by galaxies */
    if (ii >= 3) {

        sort_array(work, ii, sizeof(*work), HDRL_SORT_DOUBLE, CPL_SORT_ASCENDING);

        *fwhm = work[ii / 3 - 1];

        /* Allow for finite pixel size */
        double arg = 0.25 * CPL_MATH_PI * pow(*fwhm, 2.) - 1;

        *fwhm = 2. * sqrt(CPL_MAX(0., arg / CPL_MATH_PI));

    } else {

        *fwhm = 0.;
    }

    return CPL_ERROR_NONE;
}

/**@}*/
