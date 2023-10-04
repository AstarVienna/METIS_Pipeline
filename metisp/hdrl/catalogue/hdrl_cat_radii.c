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

#include "hdrl_cat_radii.h"

#include "hdrl_cat_utils.h"


/*---------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_radii  hdrl_radii
 * @ingroup  Catalogue
 *
 * @brief    Work out the fluxes for special radii
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Work out the half-light radius for an object
 *
 * @param    rcores    The list of aperture radii used
 * @param    cflux     The list of fluxes through the aperture radii
 * @param    halflight An estimate of half the light of the object
 * @param    peak      The peak flux of the object
 * @param    naper     The number of radii used
 *
 * @return   The half-light radius of the object
 *
 * Description:
 * 	    Given the the array of core apertures and core fluxes, work out
 *      the half light radius
 */
/* ---------------------------------------------------------------------------*/
double hdrl_halflight(double rcores[], double cflux[], double halflight, double peak, cpl_size naper)
{
    /* Work out the half-light value from either isophotal flux or the flux at
     * Rcore. The find out roughly where the curve of growth exceeds this */

    cpl_size gotone = 0;
    cpl_size i;
    for (i = 0; i < naper; i++) {
        if (cflux[i] > halflight) {
            gotone = 1;
            break;
        }
    }

    if (gotone == 0) i = naper - 1;

    /* Now work out what the radius of half light is */
    double halfrad;
    if (i == 0) {

        double delr = (cflux[i] - halflight) / CPL_MAX(1., cflux[i] - peak);

        halfrad = rcores[0] * (1. - delr) + delr * sqrt(1. / CPL_MATH_PI);

    } else {

        double delr = (cflux[i] - halflight) / CPL_MAX(1., (cflux[i] - cflux[i - 1]));

        halfrad = rcores[i - 1] * delr + rcores[i] * (1. - delr);
    }

    return halfrad;
}   

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Work out the exponential radius for an object
 *
 * @param    thresh    The detection threshold
 * @param    peak      The peak flux of the object
 * @param    areal0    The lowest level areal profile for the object
 * @param    rcores    The list of aperture radii used
 * @param    naper     The number of radii used
 *
 * @return   The exponential radius of the object
 *
 * Description:
 * 	    Given the detection threshold, peak flux and lowest areal
 *      profile, work out the exponential radius for an object
 */
/* ---------------------------------------------------------------------------*/
double hdrl_exprad(double thresh, double peak, double areal0, double rcores[], cpl_size naper)
{
    double pk  = CPL_MAX(1.5 * thresh, peak);

    double r_t = sqrt(areal0 / CPL_MATH_PI);

    double rad = CPL_MAX(r_t, CPL_MIN(5. * r_t, CPL_MIN(5. * r_t / log(pk / thresh), rcores[naper - 1])));

    return rad;
}
    
/* ---------------------------------------------------------------------------*/
/**
 * @brief    Work out the Kron radius for an object
 *
 * @param    areal0    The lowest level areal profile for the object
 * @param    rcores    The list of aperture radii used
 * @param    cflux     The aperture fluxes for each radius
 * @param    naper     The number of radii used
 *
 * @return   The Kron radius of the object
 *
 * Description:
 * 	    Given the lowest areal profile and the circular aperture fluxes
 *      already done, calculate the Kron radius
 */
/* ---------------------------------------------------------------------------*/
double hdrl_kronrad(double areal0, double rcores[], double cflux[], cpl_size naper)
{
    double r_t = sqrt(areal0 / CPL_MATH_PI);
    double rad = 0.5 * rcores[0] * cflux[0];
    double sum = cflux[0];

    cpl_size imax = CPL_MIN(naper, 7);
    for (cpl_size i = 1; i < imax; i++) {

        double wt = CPL_MAX(0., cflux[i] - cflux[i - 1]);

        rad += 0.5 * (rcores[i] + rcores[i - 1]) * wt;
        sum += wt;
    }
    rad /= sum;

    rad  = CPL_MAX(r_t, CPL_MIN(5. * r_t, CPL_MIN(2. * rad, rcores[naper - 1])));

    return rad;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Work out the Petrosian radius for an object
 *
 * @param    areal0    The lowest level areal profile for the object
 * @param    rcores    The list of aperture radii used
 * @param    cflux     The aperture fluxes for each radius
 * @param    naper     The number of radii used
 *
 * @return   The Petrosian radius of the object
 *
 * Description:
 * 	    Given the lowest areal profile and the circular aperture fluxes
 *      already done, calculate the Petrosian radius
 */
/* ---------------------------------------------------------------------------*/
double hdrl_petrad(double areal0, double rcores[], double cflux[], cpl_size naper)
{
    double r_t = sqrt(areal0 / CPL_MATH_PI);

    double eta    = 1.;
    double etaold = eta;

    cpl_size j = 1;
    while (eta > 0.2 && j < naper) {

        etaold = eta;

        double r1 = rcores[j] * rcores[j] / (rcores[j - 1] * rcores[j - 1]) - 1.;
        double r2 = cflux[j] / cflux[j - 1] - 1.;

        eta = r2 / r1;

        j++;
    }

    double r_petr = rcores[naper - 1];
    if (j != naper) {

        double r1 = rcores[j] * rcores[j];
        double r2 = rcores[j - 1] * rcores[j - 1];
        double r3 = rcores[j - 2] * rcores[j - 2];
        double r4 = (etaold - 0.2) / (etaold - eta);
        double r5 = (0.2 - eta) / (etaold - eta);

        r_petr = r4 * sqrt(0.5 * (r1 + r2)) + r5 * sqrt(0.5 * (r2 + r3));
    }

    r_petr = CPL_MAX(r_t, CPL_MIN(5. * r_t, CPL_MIN(2. * r_petr, rcores[naper-1])));

    return r_petr;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Work out the fluxes for special radii
 *
 * @param    ap        The current ap structure
 * @param    parm      The parameters for each object already detected
 * @param    nbit      The number of detected objects in the current Plessey structure.
 * @param    apers     The radii of the standard apertures
 * @param    fluxes    The fluxes computed through the standard apertures
 * @param    nr        The number of special apertures
 * @param    rcores    The radii the special apertures
 * @param    rfluxes   The fluxes computed through the special apertures.
 *
 * Description:
 * 	    The fluxes for the 'special' radii (Kron etc) are worked out
 *      by an interpolation of the pre-existing aperture photometry to the
 *      new radius.
 */
/* ---------------------------------------------------------------------------*/
void hdrl_flux(ap_t *ap, double parm[IMNUM][NPAR], cpl_size nbit, double apers[],
                 double fluxes[], cpl_size nr, double rcores[], double rfluxes[])
{
    /* Set up some local variables */
    double        *map   = ap->indata;
    unsigned char *mflag = ap->mflag;

    cpl_size nx = ap->lsiz;
    cpl_size ny = ap->csiz;

    /* Section for nbit == 1 */
    if (nbit == 1) {

        /* Generate image-blend outer boundaries */

    	double   xmin = parm[0][1] - apers[0] - 0.5;
        double   xmax = parm[0][1] + apers[0] + 0.5;
        double   ymin = parm[0][2] - apers[0] - 0.5;
        double   ymax = parm[0][2] + apers[0] + 0.5;

        cpl_size ix1  = CPL_MAX(     0, (cpl_size)xmin - 1);
        cpl_size ix2  = CPL_MIN(nx - 1, (cpl_size)xmax    );
        cpl_size iy1  = CPL_MAX(     0, (cpl_size)ymin - 1);
        cpl_size iy2  = CPL_MIN(ny - 1, (cpl_size)ymax    );

        /* Now go through pixel region and add up the contributions inside the aperture */
        fluxes[0] = 0.;
        for (cpl_size j = iy1; j <= iy2; j++) {

        	cpl_size kk = j * nx;

            for (cpl_size i = ix1; i <= ix2; i++) {

            	unsigned char mf = mflag[kk + i];

                if (mf == MF_CLEANPIX || mf == MF_OBJPIX || mf == MF_SATURATED) {

                    double t   = map[kk+i];
                    double xj  = (double)i - parm[0][1] + 1.;
                    double yj  = (double)j - parm[0][2] + 1.;
                    fluxes[0] += fraction(xj, yj, apers[0]) * t;
                } 
            }
        }

        if (fluxes[0] <= 0) fluxes[0] = parm[0][0];
    
    } else {	/* Section for blended images */
    
        /* Interpolate circular aperture fluxes */
        double sumiso = 0.;
        double sumcf  = 0.;

        for (cpl_size j = 0; j < nbit; j++) {

            sumiso += parm[j][0];

            cpl_size n = 1;
            while (n < nr - 1 && rcores[n] < apers[j]) n++;

            double delr = (rcores[n] - apers[j]) / (rcores[n] - rcores[n - 1]);

            fluxes[j] = rfluxes[j * nr + n]*(1. - delr) + rfluxes[j * nr + n - 1] * delr;

            sumcf += fluxes[j];
        }

        /* Constrain the result so that the ratios are the same as for the isophotal fluxes */
        for (cpl_size j = 0; j < nbit; j++) {

            fluxes[j] = sumcf * parm[j][0] / CPL_MAX(1., sumiso);

            if (fluxes[j] < 0.) fluxes[j] = parm[j][0];
        }
    }
}

/**@}*/
