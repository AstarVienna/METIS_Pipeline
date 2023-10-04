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

#include "hdrl_cat_areals.h"


/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_areals  hdrl_areals
 * @ingroup  Catalogue
 *
 * @brief    Work out the areal profiles for an object
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Work out the areal profiles for an object
 *
 * @param    ap         The input ap structure
 * @param    iareal     areal profile array
 *
 * Purpose:
 *     Work out the areal profiles for an object
 *
 * Description:
 * 	    The pixel list for an object is used to define the areal profiles
 *      for that object and a given detection threshold.
 */
/* ---------------------------------------------------------------------------*/
void hdrl_areals(ap_t *ap, cpl_size iareal[NAREAL])
{

    /* Initialise some stuff */
    cpl_size np       = ap->npl_pix;
    plstruct *plarray = ap->plarray;
    double    thresh  = ap->thresh;
    double    fconst  = ap->fconst;
    double    offset  = ap->areal_offset;

    /* Zero the areal profile array */
    memset(iareal, 0, NAREAL * sizeof(cpl_size));

    /* Loop through the array and form the areal profiles */
    for (cpl_size i = 0; i < np; i++) {

        double t = plarray[i].z;

        if (t > thresh) {

        	cpl_size nup = CPL_MAX(1, CPL_MIN(NAREAL, (cpl_size)(log(t) * fconst - offset) + 1));

			for (cpl_size j = 0; j < nup; j++) {
				iareal[j]++;
			}
        }
    }
}

/**@}*/
