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

#include "hdrl_cat_apclust.h"

#include "hdrl_cat_terminate.h"


/*** Prototypes ***/
static void minmax_xy( cpl_size np, plstruct *plstr,
					   cpl_size *ix1, cpl_size *ix2,
					   cpl_size *iy1, cpl_size *iy2);


/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_apclust  hdrl_apclust
 * @ingroup  Catalogue
 *
 * @brief    Detect multiple objects from a given Plessey array
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Detect multiple objects from a given Plessey array
 *
 * @param    ap      The new input ap structure
 * @param    np      The number of pixels within the input Plessey array
 * @param    plstr   The Plessey array from the original structure with the
 *                   lower detection threshold.
 * Purpose:
 *     Detect multiple peaks in a Plessey array and write to a new ap structure.
 *
 * Description:
 * 	   The Plessey array is given from an ap structure for a single object.
 *     Given a second ap structure with a revised threshold this routine
 *     will attempt to detect multiple objects within that Plessey array.
 */
/* ---------------------------------------------------------------------------*/
void hdrl_apclust(ap_t *ap, cpl_size np, plstruct *plstr)
{
    /* A couple of useful things */
	double i2compare = ap->thresh;
	double icompare  = i2compare * (double)ap->multiply;

    /* Get the min and max positions. Create a raster with the IDs of the
     * pixels in the pixel list (algorithm prefers data to be in a raster) */
	cpl_size ix1, ix2, iy1, iy2;
    minmax_xy(np, plstr, &ix1, &ix2, &iy1, &iy2);

    cpl_size nx = ix2 - ix1 + 1;
    cpl_size ny = iy2 - iy1 + 1;

    cpl_size nwork = nx * ny;
    cpl_size *work  = cpl_malloc(nwork * sizeof(cpl_size));

    for (cpl_size i = 0; i < nwork; i++) {
        work[i] = -1;
    }

    for (cpl_size k = 0; k < np; k++) {

    	cpl_size i = plstr[k].x - 1;
    	cpl_size j = plstr[k].y - 1;

        cpl_size kk = (j - iy1) * nx + i - ix1;
        work[kk] = k;
    }

    /* Now do the job */
    for (cpl_size j = iy1; j <= iy2; j++) {

        for (cpl_size i = ix1; i <= ix2; i++) {

        	cpl_size kk = (j - iy1) * nx + i - ix1;
        	cpl_size k  = work[kk];

            if (k < 0) {

                ap->lastline[i + 1] = 0;

            } else {

                if (plstr[k].zsm > icompare) {

                    /* Pixel is above threshold, find which parent it belongs to. */
                	cpl_size is = ap->lastline[i];      /* Parent last pixel this line */
                	cpl_size ip = ap->lastline[i + 1];  /* Guess belongs to above line */

                    if (ip == 0) {

                        /* New parent, or, horizontal slice: */
                        if (is == 0) {

                            /* Ah - new parent. */
                            if (ap->ipstack > ap->maxpa * 3 / 4) {
                                for (cpl_size ik = 0; ik < ap->maxpa * 3 / 8; ik++) {
                                    hdrl_apfu(ap);
                                }
                            }

                            ip = ap->pstack[ap->ipstack++];

                            ap->parent[ip].first   = ap->bstack[ap->ibstack];
                            ap->parent[ip].pnop    = 0;
                            ap->parent[ip].pnbp    = 0;
                            ap->parent[ip].growing = 0;

                            if (j == 0) {
                                ap->parent[ip].touch = 1; /* It touches first line */
                            } else {
                                ap->parent[ip].touch = 0;
                            }

                            /* For hunt thru list for terminates: */
                            if (ip > ap->maxip) {
                                ap->maxip = ip;
                            }

                        } else {

                            /* Slice with no vertical join: */
                            ip = is;
                        }

                    } else if ((ip > 0 && is > 0) && (ip != is)) {

                        /* Merge: Join linked lists: */
                        ap->blink[ap->parent[ip].last] = ap->parent[is].first;

                        /* Copy `last block': */
                        ap->parent[ip].last  = ap->parent[is].last;
                        ap->parent[ip].pnop += ap->parent[is].pnop;
                        ap->parent[ip].pnbp += ap->parent[is].pnbp;

                        /* Fix `lastline' correlator array: */
                        cpl_size ib = ap->parent[is].first;

                        cpl_size loop = 1;
                        while (loop) {

                        	cpl_size i1 = ap->plessey[ib].x;

                            if (ap->lastline[i1 + 1] == is) {
                                ap->lastline[i1 + 1] = ip;
                            }

                            if (ap->parent[is].last == ib) {
                                loop = 0;
                            } else {
                                ib = ap->blink[ib];
                            }
                        }

                        /* Mark parent inactive: */
                        ap->parent[is].pnop = -1;
                        ap->parent[is].pnbp = -1;

                        /* return name to stack: */
                        ap->pstack[--ap->ipstack] = is;
                    }

                    /* Add in pixel to linked list: */
                    cpl_size ib = ap->bstack[ap->ibstack++];

                    /* Patch forward link into last data block: */
                    if (ap->parent[ip].pnop > 0) {
                        ap->blink[ap->parent[ip].last] = ib;
                    }

                    /* Remember last block in chain: */
                    ap->parent[ip].last = ib;

                    /* Store the data: */
                    ap->plessey[ib].x   = i;
                    ap->plessey[ib].y   = j;
                    ap->plessey[ib].z   = plstr[k].z;
                    ap->plessey[ib].zsm = plstr[k].zsm;

                    /* increment active count: */
                    ap->parent[ip].pnop++;

                    /* remember which parent this pixel was for next line: */
                    ap->lastline[i + 1] = ip;

                } else {

                    /* Pixel was below threshold, mark lastline: */
                    ap->lastline[i + 1] = 0;
                }
            }
        }
    }

    /* Check for images touching left & right edges:
     * OR the touch flag with 2 for left, 4 for right: */

    if (ap->lastline[1] > 0) {
        ap->parent[ap->lastline[1]].touch |= 2;
    }

    if (ap->lastline[ap->lsiz] > 0) {
        ap->parent[ap->lastline[ap->lsiz]].touch |= 4;
    }

    cpl_free(work);
}

/**@}*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    minmax_xy
 *
 * @param    np      The number of pixels in the Plessey array
 * @param    plstr   The Plessey array from the original structure with the
 *                   lower detection threshold.
 * @param    ix1 	 The min x-coordinate
 * @param    ix2 	 The max x-coordinate
 * @param    iy1 	 The min y-coordinate
 * @param    iy2 	 The max y-coordinate
 *
 * Purpose:
 *     Work out the min and max x,y positions within a Plessey array
 * Description:
 *     All the pixels within the given Plessey array are searched and the
 *     min and max positions are returned. These values have 1 subtracted
 *     off so they can be used as array indices.
 */
/* ---------------------------------------------------------------------------*/
static void minmax_xy( cpl_size np, plstruct *plstr,
					   cpl_size *ix1, cpl_size *ix2,
					   cpl_size *iy1, cpl_size *iy2)
{

	/* Get the minmax of the positions of the pixels in a plstruct.
	 * Take 1 away from each position so that it runs from 0 rather than 1 */

    *ix1 = plstr[0].x - 1;
    *ix2 = plstr[0].x - 1;

    *iy1 = plstr[0].y - 1;
    *iy2 = plstr[0].y - 1; 

    for (cpl_size i = 1; i < np; i++) {

        *ix1 = CPL_MIN(*ix1, plstr[i].x - 1);
        *ix2 = CPL_MAX(*ix2, plstr[i].x - 1);
        *iy1 = CPL_MIN(*iy1, plstr[i].y - 1);
        *iy2 = CPL_MAX(*iy2, plstr[i].y - 1);
    }
}
