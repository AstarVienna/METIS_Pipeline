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

#include <string.h>

#include "hdrl_cat_extend.h"

#include "hdrl_cat_statistics.h"
#include "hdrl_cat_polynm.h"


/*** Defines ***/

#define NACC  10    /*  */
#define NCOEF 4		/* Number of coeficients using in the call to the polynm */


/*---------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_extend      hdrl_extend
 * @ingroup  Catalogue
 *
 * @brief    Do aperture integration
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Do aperture integration
 *
 * @param    ap        The current ap structure
 * @param    xniso     The isophotal flux
 * @param    xbar      The X position of the object
 * @param    ybar      The Y position of the object
 * @param    sxx       Second moment in X
 * @param    syy       Second moment in Y
 * @param    sxy       Second moment cross term
 * @param    areal0    The first areal profile
 * @param    tmax      The peak flux of the object
 * @param    ttotal    The output total integrated flux
 *
 * @return   CPL_ERROR_NONE if all went OK (Currently this is the only value).
 *
 * Description:
 * 	    The integrated flux of an object is calculated using matched.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_extend(
		ap_t *ap, double xniso, double xbar, double ybar,
		double sxx, double sxy, double syy, double areal0, double tmax, double *ttotal)
{
    /* Initialise a few things */
    double        *map   = ap->indata;
    cpl_size      nx     = ap->lsiz;
    cpl_size      ny     = ap->csiz;
    double        skysig = ap->sigma;
    double        thresh = ap->thresh;
    unsigned char *mflag = ap->mflag;

    /* Calculate the eccentricity and position angle of the object
     * CONSTANTS: 0.5, 4, 16, 0.9, 5, 3, 2 */
    double srr = CPL_MAX(0.5, sxx + syy);
    double ecc = CPL_MIN(0.9, sqrt((syy - sxx) * (syy - sxx) + 4. * sxy * sxy) / srr);
    double xx  = 0.5 * (1. + ecc) * srr - sxx;

    double theta;
    if (sxy == 0) {
        theta = 0.;
    } else if (xx == 0.) {
        theta = CPL_MATH_PI_2;
    } else {
        theta = atan(sxy / xx);
    }

    double ctheta = cos(theta);
    double stheta = sin(theta);

    /* Eccentricity modified by noise effect.  NB: 50 == 16*pi */
    ecc = CPL_MIN(0.9, sqrt( CPL_MAX(  (syy - sxx) * (syy - sxx)
                               - 16. * CPL_MATH_PI * skysig * srr * srr * srr / (xniso * xniso)
                               +  4. * sxy * sxy,
							   0.) ) / srr);
    
    /* Set initial aperture to be isophotal area */
    double a       = sqrt(srr * (1. + ecc));
    double b       = sqrt(srr * (1. - ecc));
    double stretch = sqrt(areal0 / (CPL_MATH_PI * a * b));

    /* Number of isophotal radii to extend */
    double rad     = CPL_MAX(1.1, (tmax - skysig) / thresh);
    double sfac    = CPL_MIN(5., CPL_MAX(2., 3. / sqrt(log(rad))));

    a *= sfac * stretch;
    b *= sfac * stretch;

    /* Clear accumulator */
    double accum[NACC];
    memset(accum, 0, NACC * sizeof(double));
    
    /* Generate image boundaries. First for y */
    double climsq = CPL_MAX(1., (a * ctheta) * (a * ctheta) + (b * stheta) * (b * stheta));
    double clim   = sqrt(climsq);

    double pt1 = sin(2. * theta) * (b * b - a * a);
    double pt2 = (b * ctheta) * (b * ctheta) + (a * stheta) * (a * stheta);
    double pt3 = (a * b) * (a * b);

    cpl_size jmin = CPL_MAX( 1, (cpl_size)(ybar - clim));
    cpl_size jmax = CPL_MIN(ny, (cpl_size)(ybar + clim + 1.));
    for (cpl_size jj = jmin; jj <= jmax; jj++) {

        /* Now for x */
    	cpl_size kk = (jj-1)*nx;

        double c  = (double)jj - ybar;

        double pa = climsq;
        double pb = pt1 * c;
        double pc = pt2 * c * c - pt3;

        double arg1 = sqrt(CPL_MAX(0., pb * pb - 4. * pa * pc));

        double xliml = (-pb - arg1) / (2. * pa);
        double xlimu = (-pb + arg1) / (2. * pa);

        cpl_size imin = CPL_MAX( 1, (cpl_size)(xbar + xliml     ));
        cpl_size imax = CPL_MIN(nx, (cpl_size)(xbar + xlimu + 1.));

        double y = c;

        for (cpl_size ii = imin; ii <= imax; ii++) {

            if (   mflag[kk + ii - 1] == MF_CLEANPIX
            	|| mflag[kk + ii - 1] == MF_OBJPIX
				|| mflag[kk + ii - 1] == MF_SATURATED)
            {
                double t = map[kk + ii - 1];
                double x = (double)ii - xbar;

                /* Accumulate elliptical isophotal areas */
                double xnew = x * ctheta - y * stheta;
                double ynew = x * stheta + y * ctheta;

                double ellrad = 2. * sqrt(  (ynew / a) * (ynew / a)
                		                  + (xnew / b) * (xnew / b));

                cpl_size iupd = CPL_MIN(NACC, CPL_MAX(1, (cpl_size)((2. - ellrad) * (double)NACC) + 1));

                for (cpl_size idx = 1; idx <= iupd; idx++) {
                    accum[NACC - idx] += t;
                }
            }
        }
    }

    /* Now find limiting intensity */
    if (xniso < 0.) {
        for (cpl_size i = 0; i < NACC; i++) {
            accum[i] = -accum[i];
        }
    }

    hdrl_median(accum, NACC, 3);

    double xmax  =  0.;
    double xlim1 = -1.;
    double xlim2 = -1.;

    double xcord[NACC];
    double xdat[NACC];
    for (cpl_size i = 0; i < NACC; i++) {
        xcord[i] = i + 1;
        xmax     = CPL_MAX(xmax, accum[i]);
        xdat[i]  = accum[i];
    }

	double polycf[NCOEF];
    hdrl_polynm(xdat, xcord, NACC, polycf, NCOEF, 0);

    double pa = polycf[1];
    double pb = polycf[2] * 2.;
    double pc = polycf[3] * 3.;

    double arg1 = sqrt(CPL_MAX(0., pb * pb - 4. * pa * pc));

    if (pc != 0.) {

        double rt1 = (-pb + arg1) / (2. * pc);
        double rt2 = (-pb - arg1) / (2. * pc);

        if (rt1 < (double)NACC && rt1 > 1.) {

            cpl_size ir = (cpl_size)rt1;
            double   t1 = rt1 - (double)ir;

            xlim1 = (1. - t1) * accum[ir - 1] + t1 * accum[ir];
        }

        if (rt2 < (double)NACC && rt2 > 1.) {

            cpl_size ir = (cpl_size)rt2;
            double   t1 = rt2 - ir;

            xlim2 = (1. - t1) * accum[ir - 1] + t1 * accum[ir];
        }
    }

    double xlimit = CPL_MAX(xlim1, xlim2);
    if (xlimit < 0.) {
        xlimit = xmax;
    }

    /* Update total intensity */
    if (xniso < 0.) {
        xlimit = -xlimit;
    }
    *ttotal = xlimit;

    return CPL_ERROR_NONE;
}

/**@}*/
