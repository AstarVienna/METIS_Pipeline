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

#include "../hdrl_cat_classify.h"
#include "../hdrl_cat_conf.h"
#include "../hdrl_cat_table.h"


#define NTEST 10


int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT,CPL_MSG_WARNING);

    /* Initialize */
    double xpos[] = { 100., 200., 300., 400., 500., 600., 700., 800., 900., 1000.};
    double ypos[] = { 100., 200., 300., 400., 500., 600., 700., 800., 900., 1000.};
    double norm[] = {1000., 100., 200., 500., 550., 600., 650., 700., 750.,  800.};

    /* Generate a field with some stars and a confidence map */
    cpl_image *bkg = cpl_image_new(1024, 1024, CPL_TYPE_DOUBLE);
    cpl_image *im  = cpl_image_new(1024, 1024, CPL_TYPE_DOUBLE);
    cpl_image *cnf = cpl_image_new(1024, 1024, CPL_TYPE_DOUBLE);

	double sigma = 2.;
    double norm2 = 2. * CPL_MATH_PI * sigma * sigma;
    cpl_image_fill_noise_uniform(bkg, -10., 10.);

	double sky = 500.;
    cpl_image_add_scalar(bkg, sky);

    cpl_image_fill_noise_uniform(cnf, 99.9, 100.1);

    double tot[NTEST];
    for (cpl_size i = 0; i < NTEST; i++) {
        cpl_image_fill_gaussian(im, xpos[i], ypos[i], norm[i] * norm2, sigma, sigma);
        tot[i] = cpl_image_get_flux(im);
        cpl_image_add(bkg, im);
    }
    cpl_image_delete(im);

    hdrl_casu_fits        *inf    = hdrl_casu_fits_wrap(bkg);
    hdrl_casu_fits        *inconf = hdrl_casu_fits_wrap(cnf);

    /* Give it a WCS */
    cpl_propertylist *pl = hdrl_casu_fits_get_ehu(inf);
    cpl_propertylist_update_string(pl, "CTYPE1", "RA---TAN" );
    cpl_propertylist_update_string(pl, "CTYPE2", "DEC--TAN" );
    cpl_propertylist_update_double(pl, "CRVAL1",  30.       );
    cpl_propertylist_update_double(pl, "CRVAL2",  12.       );
    cpl_propertylist_update_double(pl, "CRPIX1",  512.      );
    cpl_propertylist_update_double(pl, "CRPIX2",  512.      );
    cpl_propertylist_update_double(pl, "CD1_1",  -1. / 3600.);
    cpl_propertylist_update_double(pl, "CD1_2",   0.        );
    cpl_propertylist_update_double(pl, "CD2_1",   0.        );
    cpl_propertylist_update_double(pl, "CD2_2",   1. / 3600.);

    /* Run casu catalogue, TODO: check results */
    hdrl_casu_result *res = cpl_calloc(1, sizeof(hdrl_casu_result));
    cpl_test_eq(hdrl_catalogue_conf(inf, inconf, 5, 1.5, 0, 5., 1, 64, 6, 3., 1.,
    		HDRL_SATURATION_INIT, res), CPL_ERROR_NONE);
    cpl_test_nonnull(res->catalogue);
    cpl_image_delete(res->segmentation_map);
    cpl_image_delete(res->background);

    /* Check the results. Start by checking the number of rows and columns. Sort the table by X */
    cpl_table *tab = hdrl_casu_tfits_get_table(res->catalogue);
    cpl_test_nonnull(tab);
    cpl_test_eq(cpl_table_get_ncol(tab), NCOLS);
    cpl_test_eq(cpl_table_get_nrow(tab), NTEST);

    pl = cpl_propertylist_new();
    cpl_propertylist_append_bool(pl, "X_coordinate", 0);
    cpl_table_sort(tab, pl);
    cpl_propertylist_delete(pl);

    /* Test the column content of the table */
    int nl;
    for (cpl_size i = 0; i < NTEST; i++) {

        cpl_test_abs(xpos[i], cpl_table_get_double(tab, "X_coordinate",    i, &nl), 0.2);
        cpl_test_abs(ypos[i], cpl_table_get_double(tab, "Y_coordinate",    i, &nl), 0.2);

        double diff =    fabs(cpl_table_get_double(tab, "Aper_flux_5",     i, &nl) - tot[i])
                      /       cpl_table_get_double(tab, "Aper_flux_5_err", i, &nl);
        cpl_test_lt(diff, 1.6);
    }

    /*** Run classify and test the values of the classification ***/

    /* Test 1 */
    cpl_test_eq(hdrl_classify(res->catalogue, 5), CPL_ERROR_NONE);

    pl = hdrl_casu_tfits_get_ehu(res->catalogue);
    cpl_test_rel(cpl_propertylist_get_double(pl, "ESO QC IMAGE_SIZE"), 4.47,  0.02);
    cpl_test_eq( cpl_propertylist_get_bool(  pl, "ESO DRS CLASSIFD" ), 1          );
    cpl_test_rel(cpl_propertylist_get_double(pl, "APCOR3"           ), 0.132, 0.01);

    for (cpl_size i = 0; i < NTEST; i++) {
    	double val = cpl_table_get(hdrl_casu_tfits_get_table(res->catalogue), "Classification", i, NULL);
        cpl_test_rel(val, -1., 0.001);
    }

    /* Test 2 */
    cpl_test_eq(hdrl_classify(res->catalogue, 10), CPL_ERROR_NONE);

    /* Clean up */
    hdrl_casu_fits_delete(inf);
    hdrl_casu_fits_delete(inconf);
    hdrl_casu_tfits_delete(res->catalogue);
    cpl_free(res);


    return cpl_test_end(0);
}
