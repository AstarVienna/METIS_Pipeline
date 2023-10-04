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

#include <cpl_test.h>

#include "../hdrl_cat_apio.h"
#include "../hdrl_cat_phopt.h"


int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    /* Initialize */
    double apertures[] = {2.5, 3.53553, 5.0, 7.07107, 10, 14, 20, 25, 30, 35, 40, 50, 60};


    /* Set up apm structure */
    ap_t ap;
    ap.lsiz     = 2048;
    ap.csiz     = 2048;
    ap.thresh   = 11.0936;
    ap.inframe  = cpl_image_new(2048, 2048, CPL_TYPE_DOUBLE);
    ap.conframe = cpl_image_new(2048, 2048, CPL_TYPE_DOUBLE);


    /* Initialize */
    hdrl_apinit(&ap);

    ap.indata   = cpl_image_get_data_double(ap.inframe);
    ap.confdata = cpl_image_get_data_double(ap.conframe);
    ap.mflag    = cpl_calloc(2048 * 2048, sizeof(*ap.mflag));

    /* Create a background */
    cpl_image_fill_noise_uniform(ap.inframe, -10., 10.);
    cpl_image_add_scalar(        ap.inframe,  5000.);
    cpl_image_fill_noise_uniform(ap.conframe, 99, 101);


    double parm[IMNUM][NPAR];
    for (cpl_size i = 0; i < IMNUM; i++) {
    	for (cpl_size j = 0; j < NPAR; j++) {
    		parm[i][j] = i + j;
    	}
    }

    /* Reserve memory for the fluxes */
    double cflux[NRADS * IMNUM];

    /* InitialiZe the badpix accumulator */
    double badpix[IMNUM];
    double avconf[IMNUM];

    cpl_size nrcore = 2;

    /*** TESTS ***/
    cpl_size       nbit;
    cpl_error_code e;

    /* Test 1 */
    nbit = 1;
    for (cpl_size i = 0; i < nbit; i++) {
        badpix[i] = 0.;
        avconf[i] = 0.;
    }
    e = hdrl_phopt(&ap, parm, nbit, NRADS, apertures, cflux, badpix, nrcore, avconf);
    cpl_test_eq(e, CPL_ERROR_NONE);

    /* Test 2 */
    nbit = 2;
    for (cpl_size i = 0; i < nbit; i++) {
        badpix[i] = 0.;
        avconf[i] = 0.;
    }
    e = hdrl_phopt(&ap, parm, nbit, NRADS, apertures, cflux, badpix, nrcore, avconf);
    cpl_test_eq(e, CPL_ERROR_NONE);


    /* Clean up */
    hdrl_apclose(&ap);
    cpl_free(ap.mflag);
    cpl_image_delete(ap.inframe);
    cpl_image_delete(ap.conframe);


    return cpl_test_end(0);
}
