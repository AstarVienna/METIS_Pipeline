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

#include "hdrl_cat_overlp.h"

#include "hdrl_cat_apio.h"
#include "hdrl_cat_apclust.h"
#include "hdrl_cat_polynm.h"
#include "hdrl_cat_terminate.h"
#include "hdrl_cat_utils_sort.h"


/*** Defines ***/

#define IDBLIM  10000  /* Maximum number of pixels to use in deblending */
#define NITER   6      /* Number of iterations */


/*** Internal global variables ***/

static double oldthr;
static double curthr;
static double nexthr;
static double lasthr;
static double xbar_start;
static double ybar_start;


/*** Prototypes ***/

static void moments_thr(ap_t *ap, double results[NPAR+1], cpl_size ipk[2]);
static void update_ov(  double iap[NAREAL], double t, double thresh, double fconst, double offset);
static void check_term( ap_t *ap, cpl_size *nobj, double parm[IMNUM][NPAR+1],
                        cpl_size peaks[IMNUM][2], cpl_size *toomany);

static int cmp_plstruct(const void *a, const void *b);


/*---------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_overlp     hdrl_overlp
 * @ingroup  Catalogue
 *
 * @brief    Deblend overlapping images
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Deblend overlapping images
 *
 * @param    ap        The current ap structure
 * @param    parm      The parameter array for the deblended objects
 * @param    nbit      The output number of objects found in the deblended object
 * @param    xbar      The X position of the input object
 * @param    ybar      The Y position of the input object
 * @param    total     The total flux of the input object
 * @param    npix      The number of pixels in the original object
 * @param    tmax      The peak flux of the original object
 *
 * @return   CPL_ERROR_NONE if all went OK (Currently this is the only value).
 *
 * Description:
 * 	    An array of pixels that are believed to be part of a single large
 *      object are analyzed with successively higher thresholds to see if
 *      they resolve into multiple objects.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_overlp(
		ap_t *ap, double parm[IMNUM][NPAR], cpl_size *nbit,
		double xbar, double ybar, double total, cpl_size npix, double tmax)
{
    /* Initialize a few variables */
    plstruct *pl    = ap->plarray;
    cpl_size npl    = ap->npl_pix;
    cpl_size ipix   = ap->ipnop;
    double   fconst = ap->fconst;
    double   offset = ap->areal_offset;

    oldthr     = ap->thresh;
    xbar_start = xbar;
    ybar_start = ybar;

    /* Initialize some constants that you might need later */
    double   tmul     = 1.2589678;                          /* 1/4 mag deblending contour increment */
    double   smul     = 2.5;                                /* starting contour increment */
    cpl_size ipixo2   = CPL_MAX(2, (ipix + 1) / 2);             /* ipix is the minimum image pixel size */
    double   xintmn   = oldthr * ipixo2;                    /* minimum intensity for fragments */
    double   itmaxlim = 0.9 * tmax;                         /* upper limit deblending 90% of peak */

    lasthr = itmaxlim;
    curthr = smul * oldthr;

    /* Get a maximum of IDBLIM points brighter than the new detection threshold
     * by reverse sorting the input array. If there are still more than IDBLIM
     * above the threshold, then revise the threshold until there aren't. Then
     * use the variable npl2 to stop the rest of the routine accessing any of
     * the fainter pixels in the list. This is to restrict processing time
     * for large extended objects */

	sort_array_f(pl, npl, sizeof(*pl), &cmp_plstruct);

    cpl_size npl2;
    while (1) {

        npl2 = 0;
        while (npl2 < npl - 1 && pl[npl2].zsm > curthr) {
            npl2++;    
        }

        if (npl2 > IDBLIM) curthr += oldthr;
        else               break;
    }

    /* If there are fewer pixels above the new threshold than the minimum
     * specified in the input parameters, then you have no reason to be here */
    if (npl2 < ipix) {
        *nbit = 1;
        return CPL_ERROR_NONE;
    }

    /* Get a new ap structure */
    ap_t ap2;
    ap2.lsiz         = ap->lsiz;
    ap2.csiz         = ap->csiz;
    ap2.multiply     = 1;
    ap2.ipnop        = ipixo2;
    ap2.areal_offset = offset;
    ap2.fconst       = fconst;
    ap2.mflag        = cpl_calloc((ap2.lsiz) * (ap2.csiz), sizeof(*ap2.mflag));

    hdrl_apinit(&ap2);

    /* Main analysis loop at new thresholds */
    *nbit = 0;

    /* TODO: it could be problems???, it's checked (in the while) before initialize */
    cpl_size ibitx[IMNUM];
    cpl_size ibity[IMNUM];

    cpl_size iupdate[IMNUM];
    double   parmnew[IMNUM][NPAR];

    cpl_size nbitprev = 0;
    while (1) {

        nexthr = CPL_MAX(curthr + oldthr, curthr * tmul);
        
        /* Locate objects in this cluster */
        ap2.thresh = curthr;
        hdrl_apclust(&ap2, npl2, pl);

        cpl_size nobj;
        double   results[IMNUM][NPAR+1];
        cpl_size ipks[   IMNUM][2];
        cpl_size toomany;
        check_term(&ap2, &nobj, results, ipks, &toomany);

        hdrl_apreinit(&ap2);

        if (nobj == 0) {
            break;
        }

        /* For each image check the number of points above the next threshold
         * and flag. Do a moments analysis of each object */
        for (cpl_size i = 0; i < nobj; i++) {

            /* Ok, has this object already been detected?  If so, then
             * load the new results into the parmnew array. We'll check
             * whether it's worthwhile updating the master parameters
             * list later */

            cpl_size isnew = 1;

            double xb  = results[i][1];
            double yb  = results[i][2];
            double sxx = CPL_MAX(1., results[i][4]);
            double syy = CPL_MAX(1., results[i][6]);

            for (cpl_size k = 0; k < nbitprev; k++) {

                double dx = xb - parm[k][1];
                double dy = yb - parm[k][2];

                double radius2 = dx * dx / sxx + dy * dy / syy;

                if ( (ibitx[k] == ipks[i][0] && ibity[k] == ipks[i][1]) || radius2 < 1.) {

                    isnew = 0;

                    for (cpl_size kk = 0; kk < NPAR; kk++) {
                        parmnew[k][kk] = results[i][kk];
                    }

                    break;
                }
            }

            /* If this is a new one and it's above the minimum threshold 
               then check to make sure you don't already have too many. 
               If you do, then flag this and break out to the next iteration.
               If there is room for another, then store the moments analysis
               profile */

            if (isnew && results[i][0] > xintmn) {

                if (*nbit >= IMNUM) {

                    *nbit   = IMNUM;
                    toomany = 1;

                    break;
                }

                ibitx[*nbit] = ipks[i][0];
                ibity[*nbit] = ipks[i][1];

                for (cpl_size kk = 0; kk < NPAR; kk++) {
                    parm[*nbit][kk] = results[i][kk];
                }

                (*nbit)++;
            }

        } /* End of object loop */


        /* If too many objects were found, then skip the next bit...otherwise
           go through and update parameters if necessary. This block of
           code is a bit of a place holder waiting for something better to
           be worked out...*/
        if (!toomany) {

            if (*nbit > nbitprev && nbitprev > 0) {

                for (cpl_size i = 0; i < nbitprev; i++) iupdate[i] = 0;

                for (cpl_size j = nbitprev; j < *nbit; j++) {

                    double   distmax = 0.;
                    cpl_size iwas    = 0;

                    for (cpl_size i = 0; i < nbitprev; i++) {

                        if (parmnew[i][0] > 0.) {

                            double radius2 = pow(parmnew[i][1] - parm[i][1], 2.) + pow(parmnew[i][2] - parm[i][2], 2.);

                            if (radius2 > distmax) {
                                iwas    = i;
                                distmax = radius2;
                            }
                        }
                    }
                    iupdate[iwas] = 1;
                }

                for (cpl_size i = 0; i < nbitprev; i++) {
                    if (iupdate[i] == 1 && parmnew[i][0] > 0.) {
                        for (cpl_size j = 0; j < NPAR; j++) parm[i][j] = parmnew[i][j];
                    }
                }
            }

            /* Reset the update flag and prepare for next iteration*/
            for (cpl_size i = 0; i <= *nbit; i++) {
                parmnew[i][0] = -1.;
            }
            nbitprev = *nbit;
        }

        /* Where do we cut in the list now? */
        cpl_size npl3 = 0;
        while (npl3 < npl2-1 && pl[npl3].zsm > nexthr) {
            npl3++;    
        }
        npl2 = npl3;

        /* Now, do we need to move onto the next threshold? */
        if (npl2 == 0 || toomany || nexthr >= itmaxlim) {
            break;
        }

        /* If so, then reset some variables and continue */

        curthr = nexthr;

    } /* End of main analysis loop */


    /* Free up some workspace */
    cpl_free(ap2.mflag);
    hdrl_apclose(&ap2);

    /* If there isn't only one --> continue program, in other case we can exit */
    if (*nbit != 1) return hdrl_overlp_2orMore( ap, parm, nbit, xbar, ybar, total,
                                                  npix, curthr, nexthr, lasthr);

    return CPL_ERROR_NONE;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Deblend overlapping images 2 or more.
 *           If you know that exist more than 1 (come to the previous version).
 *
 * @param    ap        The current ap structure
 * @param    parm      The parameter array for the deblended objects
 * @param    nbit      The output number of objects found in the deblended object
 * @param    xbar      The X position of the input object
 * @param    ybar      The Y position of the input object
 * @param    total     The total flux of the input object
 * @param    npix      The number of pixels in the original object
 * @param    curthr_prev  Threshold
 * @param    nexthr_prev   Threshold
 * @param    lasthr_prev   Threshold
 *
 * @return   CPL_ERROR_NONE if all went OK (Currently this is the only value).
 *
 * Description:
 * 	    An array of pixels that are believed to be part of a single large
 *      object are analyzed with successively higher thresholds to see if
 *      they resolve into multiple objects.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_overlp_2orMore(
		ap_t *ap, double parm[IMNUM][NPAR], cpl_size *nbit,
		double xbar, double ybar, double total, cpl_size npix,
	    double curthr_prev, double nexthr_prev, double lasthr_prev)
{
    /* Initialize a few variables */
    cpl_size ipix   = ap->ipnop;
    double   offset = ap->areal_offset;

    oldthr     = ap->thresh;
    curthr     = curthr_prev;
    nexthr     = nexthr_prev;
    lasthr     = lasthr_prev;
    xbar_start = xbar;
    ybar_start = ybar;

    /* Initialize some constants that you might need later */
    cpl_size ipixo2   = CPL_MAX(2, (ipix + 1) / 2);       /* ipix is the minimum image pixel size */
    double   xintmn   = oldthr * ipixo2;                  /* minimum intensity for fragments */
    double   algthr   = log(oldthr);                      /* Convenient bit of shorthand */
    double   radmax   = sqrt((double)npix / CPL_MATH_PI); /* Isophotal size of input data array */

    /* Find out which images terminated properly and remove those that didn't */
    cpl_size j = -1;
    for (cpl_size k = 0; k < *nbit; k++) {

        /* Commented this out as checking for terminations seems to miss some
         * if the total flux above the threshold for an object is negative */
        if (parm[k][0] > xintmn) {

            j++;
            if (j != k) {
                for (cpl_size i = 0; i < NPAR; i++) parm[j][i] = parm[k][i];
            }
        }
    }

    double bitx[IMNUM];
    double bitl[IMNUM];
    *nbit = j + 1;
    for (cpl_size jj = 0; jj < *nbit; jj++) {
        bitx[jj] = 0.;
        bitl[jj] = 0.;
    }

    /* For each image find true areal profile levels and iterate to find local continuum */
    double xdat[NAREAL + 1];
    double xcor[NAREAL + 1];
    cpl_size lastone = 0;
    cpl_size iter    = 0;
    double   sumint  = 0.;
    while (iter < NITER) {

        iter++;
        
        /* Loop for each of the objects and create a level vs radius array */
        for (cpl_size k = 0; k < *nbit; k++) {

            if (parm[k][0] >= 0.) {

				/* Pk + newthresh - cont */
				double   xlevol = log(parm[k][7] + parm[k][3] - bitl[k]);
				double   xlevel = xlevol;

				double   radold = 0.;
				double   radius = radold;
				double   slope  = 1.;
				cpl_size ic     = 0;

				for (cpl_size i = 1; i <= NAREAL; i++) {

					cpl_size jj = NPAR   - i;
					cpl_size ii = NAREAL - i;

					double xx = (double)ii + offset;

					if (parm[k][jj] > 0.5) {

						if (ii == 0) {
							xlevel = log(parm[k][3] - bitl[k] + 0.5);
						} else {
							xlevel = log(pow(2., xx) - oldthr + parm[k][3] - bitl[k] - 0.5);
						}

						radius     = sqrt(parm[k][jj] / CPL_MATH_PI);
						xdat[ic]   = xlevel;
						xcor[ic++] = radius;

						double dlbydr = (xlevol - xlevel) / CPL_MAX(0.01, radius - radold);

						double wt = CPL_MIN(1., CPL_MAX((radius - radold) * 5., 0.1));

						slope = (1. - 0.5 * wt) * slope + 0.5 * wt * CPL_MIN(5., dlbydr);

						radold = radius;
						xlevol = xlevel;
					}
				}

				/* If this is not the last iteration then work out the effect on the local continuum from each image */
				if (!lastone) {

					for (cpl_size i = 0; i < *nbit; i++) {

						if (parm[i][0] >= 0.0 && i != k) {

							double dist = sqrt(pow(parm[k][1] - parm[i][1], 2.) + pow(parm[k][2] - parm[i][2], 2.));

							double xeff = xlevel - CPL_MAX(0., CPL_MIN(50., slope * (dist - radius)));

							bitx[i] += exp(xeff);
						}
					}

				} else {

					/* If this is the last iteration loop, then update the parameters before exiting*/
					double ttt;
					if (ic > 2) {
						double polycf[3];
						hdrl_polynm(xdat, xcor, ic, polycf, 3, 0);
						ttt = polycf[1] + 2. * polycf[2] * radius;
					} else {
						ttt = 0.;
					}

					slope = CPL_MAX(0.1, CPL_MAX(-ttt, slope));

					double radthr = radius + (xlevel - algthr) / slope;
					if (radthr > radmax) {
						slope  = 1.;
						radthr = radmax;
					}

					/* Pixel area */
					double delb = parm[k][8] * (parm[k][3] - bitl[k]);
					parm[k][8] = CPL_MATH_PI * radthr * radthr;

					/* Peak height */
					parm[k][7] += (parm[k][3] - bitl[k]);

					/* Intensity */
					double deli = 2. * CPL_MATH_PI * ( (parm[k][3] - bitl[k]) * (1. + slope * radius)
													  - oldthr * (1. + slope*radthr)
													 ) / (slope * slope);

					parm[k][0] += delb + CPL_MAX(0., deli);

					for (cpl_size i = 0; i < 7; i++) {
						parm[k][i+9] = -1.;
					}

					if (parm[k][0] > xintmn) {
						sumint += parm[k][0];
					}
				}
            }
        }

        /* If this is not the last iteration then check and see how the
         * continuum estimates are converging. If they appear to be converging
         * then let the next iteration be the last one. */
        if (!lastone) {

            cpl_size conv = 1;

            for (cpl_size i = 0; i < *nbit; i++) {

                if (parm[i][0] >= 0.) {

                    if (fabs(bitx[i] - bitl[i]) > 3.) conv = 0;

                    bitl[i]  = bitx[i];
                    bitx[i]  = 0;

                    /* FIXME Possible font of problems in the cast. Is it necessary ? */
                    double a = parm[i][3] - oldthr;
                    bitl[i]  = CPL_MIN(bitl[i], (cpl_size)(a + (a < 0 ? -0.5 : 0.5)));

                    //bitl[i]  = CPL_MIN(bitl[i], parm[i][3] - oldthr);
                }
            }

            lastone = (conv || (iter == NITER - 1));

        } else {
            break;
        }
    }

    /* Find the scaling if needed */
    if (sumint == 0.) *nbit = 1;
    else {
        double ratio = total / sumint;
		for (cpl_size i = 0; i < *nbit; i++) {
			parm[i][0] = ratio * parm[i][0];
		}
    }

    return CPL_ERROR_NONE;
}

/**@}*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Work out moments for an object with the current threshold.
 *
 * @param    ap        The current ap structure
 * @param    results   The moments results
 * @param    ipk       The x,y coordinates of the peak pixel
 *
 * Description:
 * 	    The moments an object in the current Plessey array are calculated
 *      for pixels above the current threshold.
 */
/* ---------------------------------------------------------------------------*/
static void moments_thr(ap_t *ap, double results[NPAR + 1], cpl_size ipk[2])
{
    /* Copy some stuff to local variables */
    double   fconst   = ap->fconst;
    double   offset   = ap->areal_offset;
    plstruct *plarray = ap->plarray;
    cpl_size np       = ap->npl_pix;

    /* Initialize a few things */
    double   xoff     = xbar_start;
    double   yoff     = ybar_start;
    double   xsum     = 0.;
    double   ysum     = 0.;
    double   xsum_w   = 0.;
    double   ysum_w   = 0.;
    double   wsum     = 0.;
    double   xsumsq   = 0.;
    double   ysumsq   = 0.;
    double   tsum     = 0.;
    double   xysum    = 0.;
    double   tmax     = plarray[0].z - curthr;

    ipk[0] = plarray[0].x;
    ipk[1] = plarray[0].y;

    for (cpl_size i = 8; i < NPAR; i++) {
        results[i] = 0.;
    }

    /* Do a moments analysis on an object */
    cpl_size nnext = 0;
    for (cpl_size i = 0; i < np; i++) {

        double x = (double)(plarray[i].x) - xoff;
        double y = (double)(plarray[i].y) - yoff;

        double t = plarray[i].z   - curthr;
        double w = plarray[i].zsm - curthr;

        if (w > nexthr) {
            nnext++;
        }

        xsum   += t * x;
        ysum   += t * y;
        tsum   += t;

        xsum_w += w * t * x;
        ysum_w += w * t * y;

        wsum   += w * t;

        xsumsq += (x * x) * t;
        ysumsq += (y * y) * t;

        xysum  += x * y * t;

        update_ov(results + 8, t, oldthr, fconst, offset);

        if (t > tmax) {

            ipk[0] = plarray[i].x;
            ipk[1] = plarray[i].y;

            tmax = t;
        }
    }

    /* Check that the total intensity is enough and if it is, then do
     *  the final results. Use negative total counts to signal an error */
    if (tsum > 0.) {
        results[0] = tsum;
    } else {
        results[0] = -1.;
        tsum = 1.;
    }

    double xbar  = xsum / tsum;
    double ybar  = ysum / tsum;

    double sxx   = CPL_MAX(0., (xsumsq / tsum - xbar * xbar));
    double syy   = CPL_MAX(0., (ysumsq / tsum - ybar * ybar));
    double sxy   = xysum / tsum - xbar * ybar;

    wsum  = CPL_MAX(1., wsum);

    xbar  = xsum_w / wsum;
    ybar  = ysum_w / wsum;

    xbar += xoff;
    ybar += yoff;

    xbar  = CPL_MAX(1., CPL_MIN(xbar, ap->lsiz));
    ybar  = CPL_MAX(1., CPL_MIN(ybar, ap->csiz));

    /* Store the results now */
    results[1]    = xbar;
    results[2]    = ybar;
    results[3]    = curthr;
    results[4]    = sxx;
    results[5]    = sxy;
    results[6]    = syy;
    results[7]    = tmax;
    results[NPAR] = ((nnext > ap->ipnop && nexthr < lasthr) ? 0 : 1);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Update areal profiles for a given threshold
 *
 * @param    iap       The areal profile array
 * @param    t         The input flux
 * @param    thresh    The current detection threshold
 * @param    fconst    Scaling parameter (log_2(e))
 * @param    offset    The offset between areal levels
 *
 * Description:
 * 	    The areal profiles are updated for a given flux and threshold.
 */
/* ---------------------------------------------------------------------------*/
static void update_ov(double iap[NAREAL], double t, double thresh, double fconst, double offset)
{
    /* if the intensity is positive */
    if (t > 0.) {

		/* update the relevant profile counts */
		cpl_size nup = CPL_MAX(1, CPL_MIN(NAREAL, (cpl_size)(log(t + thresh) * fconst - offset) + 1));

		for (cpl_size i = 0; i < nup; i++) {
			iap[i] += 1.;
		}
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Check for terminations in ap structure
 *
 * @param    ap        The current ap structure
 * @param    nobj      The output number of objects detected
 * @param    parm      The parameter array for all objects
 * @param    peaks     The location of the peaks for each of the objects
 * @param    toomany   If set, then too many objects have been detected
 *
 * Description:
 * 	    The ap structure is analyzed to see if there are any objects that can be terminated.
 */
/* ---------------------------------------------------------------------------*/
static void check_term( ap_t *ap, cpl_size *nobj, double parm[IMNUM][NPAR+1],
		                cpl_size peaks[IMNUM][2], cpl_size *toomany)
{
	/* Search through all possible parents */
	*nobj    = 0;
	*toomany = 0;

	double   momresults[NPAR + 1];
	cpl_size ipks[2];

	for (cpl_size ip = 1; ip <= ap->maxip; ip++) {

		if (ap->parent[ip].pnop != -1) {

			if (   ap->parent[ip].pnop  >= ap->ipnop
				&& ap->parent[ip].touch == 0)
			{
				hdrl_extract_data(ap, ip);

				moments_thr(ap, momresults, ipks);
				if (momresults[0] > 0.) {

					if (*nobj == IMNUM - 1) {
						*toomany = 1;
						break;
					}

					for (cpl_size i = 0; i <= NPAR; i++) {
						parm[*nobj][i] = momresults[i];
					}

					for (cpl_size i = 0; i < 2; i++) {
						peaks[*nobj][i] = ipks[i];
					}

					(*nobj)++;
				}
			}

			hdrl_restack(ap, ip);
		}
	}
}

static int cmp_plstruct(const void *a, const void *b) {

	const plstruct *a_d = (const plstruct*)a;
	const plstruct *b_d = (const plstruct*)b;

	/* Descending order */
	return -(a_d->zsm < b_d->zsm ? -1 : (a_d->zsm > b_d->zsm ? 1 : 0));
}
