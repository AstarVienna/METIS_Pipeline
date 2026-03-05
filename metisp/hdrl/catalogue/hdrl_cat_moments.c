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

#include "hdrl_cat_moments.h"


/*---------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_moments     hdrl_moments
 * @ingroup  Catalogue
 *
 * @brief    Do moments analysis on an object
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Do moments analysis on an object in a Plessey array
 *
 * @param    ap        The current ap structure
 * @param    results   The output array with the moments results.
 *
 * Description:
 * 	    The Plessey array is given from an ap structure for a single object.
 *      This routine does a basic zeroth, first and second moments analysis.
 */
/* ---------------------------------------------------------------------------*/
void hdrl_moments(ap_t *ap, double results[]) {

    /* Initialise a few things */
    double   xintmin  = ap->xintmin;
    plstruct *plarray = ap->plarray;
    cpl_size np       = ap->npl_pix;

    double xoff   = (double)(plarray[0].x);
    double yoff   = (double)(plarray[0].y);

    double tmax   = plarray[0].z;

    double xsum   = 0.;
    double ysum   = 0.;
    double tsum   = 0.;
    double xsum_w = 0.;
    double ysum_w = 0.;
    double wsum   = 0.;
    double xsumsq = 0.;
    double ysumsq = 0.;
    double xysum  = 0.;

    /* Do a moments analysis on an object */
    for (cpl_size i = 0; i < np; i++) {

        double t = plarray[i].z;
        if (t >= 0.) {

            double w = plarray[i].zsm;

            double x = (double)(plarray[i].x) - xoff;
            double y = (double)(plarray[i].y) - yoff;

			xsum    += t * x;
			ysum    += t * y;
			tsum    += t;

			xsum_w  += w * t * x;
			ysum_w  += w * t * y;
			wsum    += w * t;

			xsumsq  += x * x * t;
			ysumsq  += y * y * t;
			xysum   += x * y * t;

			tmax     = CPL_MAX(tmax, plarray[i].z);
        }
    }

    /* Check that the total intensity is enough and if it is, then do the final results */
    if (tsum >= xintmin) {

        double xbar = xsum / tsum;
        double ybar = ysum / tsum;

        double sxx = CPL_MAX(0., (xsumsq / tsum - xbar * xbar));
        double syy = CPL_MAX(0., (ysumsq / tsum - ybar * ybar));
        double sxy = xysum / tsum - xbar * ybar;

        xbar       = CPL_MAX(1., CPL_MIN(ap->lsiz, xoff + xsum_w / wsum));
        ybar       = CPL_MAX(1., CPL_MIN(ap->csiz, yoff + ysum_w / wsum));

        results[0] = 1.;
        results[1] = xbar;
        results[2] = ybar;
        results[3] = tsum;
        results[4] = sxx;
        results[5] = sxy;
        results[6] = syy;
        results[7] = tmax;

    } else {

        results[0] = -1.;
    }
}

/**@}*/
