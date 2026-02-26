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

#include "hdrl_cat_terminate.h"

#include "hdrl_cat_table.h"


/*---------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_terminate      hdrl_terminate
 * @ingroup  Catalogue
 *
 * @brief    Check for objects that have terminated
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Check for objects that have terminated
 *
 * @param    ap        The current ap structure
 * @param    gain      The header keyword with the gain in e-/ADU
 * @param    nobjects  Number of detected objects
 * @param    tab       Output catalogue table
 * @param    res
 *
 * Description:
 * 	    The parents in the current ap structure are examined to see which
 *      have not grown since the last pass. Any that have not grown are
 *      sent to the processing routine.
 */
/* ---------------------------------------------------------------------------*/
void hdrl_terminate(ap_t *ap, double gain, cpl_size *nobjects,
					  cpl_table *tab, hdrl_casu_result *res)
{

    /* Search through all possible parents!  */
    for (cpl_size ip = 1; ip <= ap->maxip; ip++) {

        if (ap->parent[ip].pnop != -1) {

            if (ap->parent[ip].pnop == ap->parent[ip].growing) {

                /* That's a termination: */
                if (( ap->parent[ip].pnop  >=  ap->ipnop               &&
                      ap->parent[ip].touch ==  0)                      &&
                     (ap->parent[ip].pnbp  <  (ap->parent[ip].pnop)/2) ){

                    /* Call the processing routine */
                    hdrl_extract_data(ap, ip);
                    hdrl_process_results(ap, gain, nobjects, tab, res);
                }

                hdrl_restack(ap,ip);

            } else {

                /* This parent still active: */
                ap->parent[ip].growing = ap->parent[ip].pnop;
            }
        }
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Free information for an object from the ap structure
 *
 * @param    ap        The current ap structure
 * @param    ip        The parent number for the object
 *
 * Description:
 * 	    The starting address for an object in the ap structure is given.
 *      Information relating to that object is erased and the space made available
 */
/* ---------------------------------------------------------------------------*/
void hdrl_restack(ap_t *ap, cpl_size ip)
{
    /* Reset the mflag */
    unsigned char *mflag = ap->mflag;

    cpl_size np = ap->parent[ip].pnop;
    cpl_size ib = ap->parent[ip].first;

    for (cpl_size i = 0; i < np; i++) {
    	cpl_size nn = ap->plessey[ib].y*ap->lsiz + ap->plessey[ib].x;
        mflag[nn] = MF_POSSIBLEOBJ;
        ib = ap->blink[ib];
    }

    /* Stash all blocks back in a burst */
    ib = ap->parent[ip].first;
    for (cpl_size i = ap->ibstack - ap->parent[ip].pnop; i < ap->ibstack-1;  i++) {
        ap->bstack[i] = ib;
        ib = ap->blink[ib];
    }

    /* and the last one */
    ap->bstack[ap->ibstack-1] = ib;
    ap->ibstack -= ap->parent[ip].pnop;

    /* Put parent name back on stack */
    ap->pstack[--ap->ipstack] = ip;

    /* Mark that parent inactive */
    ap->parent[ip].pnop = -1;
    ap->parent[ip].pnbp = -1;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Get rid of the largest contributor in an ap structure
 *
 * @param    ap        The current ap structure
 *
 * Description:
 * 	    The parents in the current ap structure are examined to see which
 *      has the largest number of pixels. That parent is junked.
 */
/* ---------------------------------------------------------------------------*/
void hdrl_apfu(ap_t *ap)
{
	/* Search through all possible parents and just junk the biggest one to free space:  */
    cpl_size big   = 0;
    cpl_size ipbig = 0;

    for (cpl_size ip = 1; ip <= ap->maxip; ip++) {

        if (ap->parent[ip].pnop != -1) {

            if (ap->parent[ip].pnop > big) {
                big   = ap->parent[ip].pnop;
                ipbig = ip;
            }
        }
    }

    if (big > 0) {

        hdrl_restack(ap, ipbig);

        /* clearout lastline references to this parent: */
        for (cpl_size ip = 0; ip <= ap->lsiz; ip++) {

            if (ap->lastline[ip] == ipbig) ap->lastline[ip] = 0;
        }
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Put data into the Plessey array for an object
 *
 * @param    ap        The current ap structure
 * @param    ip        The parent in question
 *
 * Description:
 * 	    The information for the object from a given parent is extracted from
 *      the link list in the ap structure and put into the Plessey array in
 *      preparation for analysis.
 */
/* ---------------------------------------------------------------------------*/
void hdrl_extract_data(ap_t *ap, cpl_size ip)
{
    /* Check the size of the workspace and see if it's big enough. If it
       isn't then increase the size until it is */

    unsigned char *mflag = ap->mflag;

    cpl_size np = ap->parent[ip].pnop;
    if (ap->npl < np) {
        ap->plarray = cpl_realloc(ap->plarray, np * sizeof(*ap->plarray));
        ap->npl = np;
    }

    /* Pull the info out now */
    cpl_size ib = ap->parent[ip].first;
    ap->npl_pix = np;
    for (cpl_size i = 0; i < np; i++) {

        ap->plarray[i].x   = ap->plessey[ib].x + 1;
        ap->plarray[i].y   = ap->plessey[ib].y + 1;

        ap->plarray[i].z   = ap->plessey[ib].z;
        ap->plarray[i].zsm = ap->plessey[ib].zsm;

        cpl_size nn = ap->plessey[ib].y*ap->lsiz + ap->plessey[ib].x;

        mflag[nn] = MF_OBJPIX;

        ib = ap->blink[ib];
    }
}

/**@}*/
