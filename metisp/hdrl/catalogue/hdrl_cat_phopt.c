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

#include "hdrl_cat_phopt.h"

#include "hdrl_cat_utils.h"


/*---------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_phopt     hdrl_phopt
 * @ingroup  Catalogue
 *
 * @brief    Does multiple profile fitting to determine intensities
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Does multiple profile fitting to determine intensities
 *
 * @param    ap        The current ap structure
 * @param    parm      The input/output object parameters
 * @param    nbit      The number of objects detected in the current Plessey structure
 * @param    naper     The number of apertures
 * @param    apertures Array of aperture radii
 * @param    cflux     Array of aperture fluxes
 * @param    badpix    Array saying how many bad pixels were included in the data for each object at each radius
 * @param    nrcore    The index of the apertures array that defines where the radius = Rcore
 * @param    avconf
 *
 * @return   CPL_ERROR_NONE if all went OK (Currently this is the only value).
 *
 * Description:
 * 	    Given a Plessey array and some parameters determined from a moments
 *      analysis for each of the objects detected in the array, this routine
 *      does multiple profile fitting for the given aperture set
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_phopt(
		ap_t *ap, double parm[IMNUM][NPAR], cpl_size nbit, cpl_size naper,
		double apertures[], double cflux[], double badpix[], cpl_size nrcore, double avconf[])
{
    /* Set up some local variables */
    double        *map   = ap->indata;
    double        *conf  = ap->confdata;
    unsigned char *mflag = ap->mflag;
    cpl_size      nx     = ap->lsiz;
    cpl_size      ny     = ap->csiz;

    /* Loop for each of the apertures */
    double aa[IMNUM+1][IMNUM+1];
	double bb[IMNUM+1];

    for (cpl_size iaper = 0; iaper < naper; iaper++) {

        double rcirc  = apertures[iaper];
        double parrad = rcirc + 0.5;

        /* profile normalising constant */
        double cn     = 1. / (CPL_MATH_PI * rcirc * rcirc);
        double cnsq   = cn * cn;
    
        /* set up covariance matrix - analytic special case for cores */
        for (cpl_size i = 0; i < nbit; i++) {

        	/* overlaps totally area=pi*r**2 */
            aa[i][i] = cn;

            if (nbit > 1) {

                double xi = parm[i][1];
                double yi = parm[i][2];

                for (cpl_size j = i + 1; j < nbit; j++) {

                    double d = sqrt(  (xi - parm[j][1]) * (xi - parm[j][1])
                                    + (yi - parm[j][2]) * (yi - parm[j][2]));

                    if (d >= 2. * rcirc) {
                        aa[j][i] = 0.;
                    } else {
                        double arg = d / (2. * rcirc);
                        aa[j][i] =  cnsq * 2. * rcirc * rcirc * (acos(arg)
                        		  - arg * (sqrt(1. - arg * arg)));
                    }

                    aa[i][j] = aa[j][i];
                }
            }
        }

        /* clear accumulators */
        for (cpl_size i = 0; i < nbit; i++) {
            bb[i] = 0.;
        }

        /* generate image-blend outer boundaries */
        double xmin = DBL_MAX;
        double xmax = DBL_MIN;
        double ymin = DBL_MAX;
        double ymax = DBL_MIN;

        for (cpl_size i = 0; i < nbit; i++) {

            double xi = parm[i][1];
            double yi = parm[i][2];

            xmin = CPL_MIN(xmin, xi);
            xmax = CPL_MAX(xmax, xi);
            ymin = CPL_MIN(ymin, yi);
            ymax = CPL_MAX(ymax, yi);
        }

        double ix1 = CPL_MAX(     0, (cpl_size)(xmin - parrad) - 1);
        double ix2 = CPL_MIN(nx - 1, (cpl_size)(xmax + parrad)    );

        double iy1 = CPL_MAX(     0, (cpl_size)(ymin - parrad) - 1);
        double iy2 = CPL_MIN(ny - 1, (cpl_size)(ymax + parrad)    );

        /* now go through pixel region */
        for (cpl_size ii = iy1; ii <= iy2; ii++) {

            cpl_size kk = ii * nx;

            for (cpl_size i = ix1; i <= ix2; i++) {

            	unsigned char mf = mflag[kk+i];

                if (mf == MF_ZEROCONF || mf == MF_STUPID_VALUE) {

                    for (cpl_size j = 0; j < nbit; j++) {

                        double xj =  i - parm[j][1] + 1.;
                        double yj = ii - parm[j][2] + 1.;
                        double tj = fraction(xj, yj, rcirc);

                        aa[j][j] -= tj * tj * cnsq;

                        for (cpl_size k = j + 1; k < nbit; k++) {
                            double tk = fraction( i - parm[k][1] + 1., ii - parm[k][2] + 1., rcirc);
                            aa[k][j] -= tk * tj * cnsq;
                            aa[j][k]  = aa[k][j];
                        }

                        if (iaper == nrcore) badpix[j] += tj;
                    }

                } else if (mf == MF_CLEANPIX || mf == MF_OBJPIX || mf == MF_SATURATED) {

                    double t = map[kk+i];

                    for (cpl_size j = 0; j < nbit; j++) {

                        double xj =  i - parm[j][1] + 1.;
                        double yj = ii - parm[j][2] + 1.;

                        double ff = fraction(xj, yj, rcirc);

                        bb[j] += ff * t;

                        if (iaper == nrcore) {
                            avconf[j] += ff * conf[kk + i];
                        }
                    }
                } 
            }
        }

        if (nbit == 1) {

        	/* Trivial solution for single object */
            cflux[iaper] = bb[0];

        } else {

            /* solve for profile intensities */
            for (cpl_size i = 0; i < nbit; i++) {
                aa[i][i] = CPL_MAX(aa[i][i], cnsq);
            }

            /* Could be better to call --> cpl_matrix_decomp_chol(...); */
            dchole(aa, bb, nbit);

            for (cpl_size i = 0; i < nbit; i++) {
                cflux[i * naper + iaper] = cn * bb[i];
            }
        }
    }

    return CPL_ERROR_NONE;
}

/**@}*/
