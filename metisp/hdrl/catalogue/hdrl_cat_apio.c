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

#include "hdrl_cat_apio.h"


/*** DEFINES ***/
#define MAXBL 250000


/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_apio  hdrl_apio
 * @ingroup  Catalogue
 *
 * @brief    Initialize the ap structure
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Initialize the ap structure
 *
 * @param    ap      The input ap structure
 *
 * Purpose:
 *     Initialize the ap structure given some pre-existing information
 *
 * Description:
 * 	    The ap structure is initialized. In order for this to be done
 * 	    properly the value of ap->lsiz (the length of the image rows)
 * 	    must be set before calling this routine.
 */
/* ---------------------------------------------------------------------------*/
void hdrl_apinit(ap_t *ap) {

    /* max possible parents */
	cpl_size maxpa = ap->lsiz / 2;

    ap->lastline = cpl_calloc(ap->lsiz + 1, sizeof(*ap->lastline));
    ap->maxip    = 0;
    ap->maxpa    = maxpa;
    ap->pstack   = cpl_malloc(maxpa * sizeof(*ap->pstack));
    ap->parent   = cpl_malloc(maxpa * sizeof(*ap->parent));

    for (cpl_size i = 0; i < maxpa; i++) {
        ap->pstack[i]      =  i;
        ap->parent[i].pnop = -1;        /* mark all parents inactive */
        ap->parent[i].pnbp = -1;        /* mark all parents inactive */
    }

    ap->ipstack = 1;
    ap->maxbl   = MAXBL;
    ap->bstack  = cpl_malloc(ap->maxbl * sizeof(*ap->bstack));
    ap->blink   = cpl_malloc(ap->maxbl * sizeof(*ap->blink));
    ap->plessey = cpl_malloc(ap->maxbl * sizeof(*ap->plessey));

    for (cpl_size i = 0; i < MAXBL; i++) {
        ap->bstack[i] = i;
    }

    ap->ibstack = 2;    /* block 1 will get overwritten; don't use it */
    ap->nimages = 0;

    /* set up exponential areal-profile levels: */
    ap->areal[0]      = 1;
    cpl_size maxAreal = 8;
    for (cpl_size i = 1; i < maxAreal; i++) {
        ap->areal[i] = ap->areal[i - 1] * 2;
    }

    /* allocate some space for a processing array */
    ap->npl     = ap->lsiz;
    ap->npl_pix = 0;
    ap->plarray = cpl_malloc(ap->npl * sizeof(*ap->plarray));

    /* set these to null values as you may not need the background structure */
    ap->backmap.nby   = -1;
    ap->backmap.bvals = NULL;

    /* Initialise some info about the input images */
    ap->indata   = NULL;
    ap->confdata = NULL;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Re-initialize the ap structure
 *
 * @param    ap      The input ap structure
 *
 * Purpose:
 *     Re-initialize the ap structure
 *
 * Description:
 * 	    The ap structure is reinitialized to the state it was in before
 *      we started detecting objects in it. All information about detected
 *      objects is erased.
 */
/* ---------------------------------------------------------------------------*/
void hdrl_apreinit(ap_t *ap) {

    for (cpl_size i = 0; i < ap->lsiz + 1; i++) {
        ap->lastline[i] = 0;
    }

    ap->maxip = 0;

    for (cpl_size i = 0; i < ap->maxpa; i++) {
        ap->pstack[i]      =  i;
        ap->parent[i].pnop = -1;        /* mark all parents inactive */
        ap->parent[i].pnbp = -1;        /* mark all parents inactive */
    }

    ap->ipstack = 1;
    ap->ibstack = 2;    /* block 1 will get overwritten; don't use it */
    ap->nimages = 0;
    ap->npl_pix = 0;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Close ap structure
 *
 * @param    ap      The input ap structure
 *
 * Purpose:
 *     Close the ap structure
 *
 * Description:
 * 	    The ap structure has all of its allocated memory freed.
 */
/* ---------------------------------------------------------------------------*/
void hdrl_apclose(ap_t *ap) {

    if (ap->lastline) {cpl_free(ap->lastline);  ap->lastline = NULL;}
    if (ap->pstack  ) {cpl_free(ap->pstack  );  ap->pstack   = NULL;}
    if (ap->parent  ) {cpl_free(ap->parent  );  ap->parent   = NULL;}
    if (ap->bstack  ) {cpl_free(ap->bstack  );  ap->bstack   = NULL;}
    if (ap->blink   ) {cpl_free(ap->blink   );  ap->blink    = NULL;}
    if (ap->plessey ) {cpl_free(ap->plessey );  ap->plessey  = NULL;}
    if (ap->plarray ) {cpl_free(ap->plarray );  ap->plarray  = NULL;}

    if (ap->backmap.bvals) {

        for (cpl_size i = 0; i < ap->backmap.nby; i++) {

            if (ap->backmap.bvals[i]) {
            	cpl_free(ap->backmap.bvals[i]);
            	ap->backmap.bvals[i] = NULL;
            }
        }

       	cpl_free(ap->backmap.bvals);
       	ap->backmap.bvals = NULL;
    }
}

/**@}*/
