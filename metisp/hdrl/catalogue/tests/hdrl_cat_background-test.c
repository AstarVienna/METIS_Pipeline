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
#include "../hdrl_cat_background.h"


int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT,CPL_MSG_WARNING);

    /* Create an input apm structure */
    ap_t ap;
    ap.lsiz     = 2048;
    ap.csiz     = 2048;
    ap.inframe  = cpl_image_new(2048, 2048, CPL_TYPE_DOUBLE);
    ap.conframe = cpl_image_new(2048, 2048, CPL_TYPE_DOUBLE);

    /* Initialize */
    hdrl_apinit(&ap);

    ap.indata   = cpl_image_get_data_double(ap.inframe);
    ap.confdata = cpl_image_get_data_double(ap.conframe);
    ap.mflag    = cpl_malloc(ap.lsiz * ap.csiz * sizeof(*ap.mflag));

    double null_value = -100.;
    for (cpl_size i = 0; i < ap.lsiz * ap.csiz; i++) {
    	ap.mflag[i] = null_value;
    }

    /* Create a background */
    cpl_image_fill_noise_uniform(ap.inframe, -10., 10.);
    cpl_image_add_scalar(        ap.inframe,  5000.);
    cpl_image_fill_noise_uniform(ap.conframe, 99, 101);

    /* Get the background value */
    double skymed;
    double skysig;
    cpl_test_eq(hdrl_backstats(&ap, &skymed, &skysig), CPL_ERROR_NONE);
    cpl_test_rel(skymed, 5000., 0.01);
    cpl_test_rel(skysig, 20 / sqrt(12), 0.1);

    /* Create a background map */
    hdrl_casu_result *res = cpl_malloc(sizeof(hdrl_casu_result));
    res->background = cpl_image_new(ap.lsiz, ap.csiz, CPL_TYPE_DOUBLE);
    cpl_test_eq(hdrl_background(&ap, 64, 1, res), CPL_ERROR_NONE);

    for (cpl_size j = 0; j < ap.backmap.nby; j++) {
        for (cpl_size i = 0; i < ap.backmap.nbx; i++) {
            cpl_test_rel((ap.backmap.bvals)[i][j], 5000., 0.01);
        }
    }
    cpl_test_rel(cpl_image_get_median(ap.inframe), 5000., 0.1);

    /* TODO: Check results */
    hdrl_backest(&ap, 1000., 1000., &skymed, &skysig);
    cpl_test_rel(skymed, 5000., 0.01);
    cpl_test_lt(0., skysig);
    cpl_image_delete(res->background);
    cpl_free(res);

    /* Clean up */
    hdrl_apclose(&ap);
    cpl_free(ap.mflag);
    cpl_image_delete(ap.inframe);
    cpl_image_delete(ap.conframe);


    return cpl_test_end(0);
}
