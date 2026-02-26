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

#include "../hdrl_cat_casu.h"

#include "hdrl_random.h"


#define COMP_TOL_REL  1. / 3.
#define COMP_TOL_ABS  1e-2
#define IMG_XSIZE     120
#define IMG_YSIZE     180


void test_basic(void)
{
    /* Generate a field with some stars and a confidence map */
    cpl_image *bkg = cpl_image_new(IMG_XSIZE, IMG_YSIZE, CPL_TYPE_DOUBLE);
    cpl_image *im  = cpl_image_new(IMG_XSIZE, IMG_YSIZE, CPL_TYPE_DOUBLE);
    cpl_image *cnf = cpl_image_new(IMG_XSIZE, IMG_YSIZE, CPL_TYPE_DOUBLE);

    /* Generate confidence uniform confidence map */
    cpl_image_add_scalar(cnf, 100.);

    /* Generate sky and a gaussian source without errors */
    double sigma = 2.;
    double norm2 = 2. * CPL_MATH_PI * sigma * sigma;

    double sky = 500.;
    cpl_image_add_scalar(bkg, sky);

    double xpos =   80.;
    double ypos =  100.;
    double norm = 3000.;
    cpl_image_fill_gaussian(im, xpos, ypos, norm * norm2, sigma, sigma);
    cpl_image_add(bkg, im);

    hdrl_casu_fits        *inf    = hdrl_casu_fits_wrap( im);
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
    cpl_propertylist_update_int(   pl, "NAXIS1",  IMG_XSIZE );
    cpl_propertylist_update_int(   pl, "NAXIS2",  IMG_YSIZE );
    cpl_wcs *wcs = cpl_wcs_new_from_propertylist(pl);

    /* Perform the montecarlo simulation */
    hdrl_casu_result *res = cpl_calloc(1, sizeof(hdrl_casu_result));
    cpl_table *tabfinal = NULL;
    for (cpl_size iterate = 0; iterate < 100; iterate++) {

        /* Add realistic poisonnian noise */
        hdrl_random_state * rng = hdrl_random_state_new(1, NULL);
        cpl_size size = cpl_image_get_size_x(bkg) * cpl_image_get_size_y(bkg);
        double   *pim = cpl_image_get_data_double(im);
        double   *pbkg= cpl_image_get_data_double(bkg);

        for (cpl_size i = 0; i < size; i++) {
            pim[i] = hdrl_random_poisson(rng, pbkg[i]);
        }
        hdrl_random_state_delete(rng);

        /* Run casu catalogue */
        hdrl_casu_catalogue(inf, inconf, wcs, 5, 2.5, 0, 3., 1, 32,
                HDRL_CATALOGUE_ALL, 3.0, 1.0, HDRL_SATURATION_INIT, res);

        cpl_table *tab = hdrl_casu_tfits_get_table(res->catalogue);

        if (iterate == 0) {
            tabfinal = cpl_table_duplicate(tab);
        } else {
            cpl_table_insert(tabfinal, tab, iterate);
        }

        cpl_image_delete( res->segmentation_map);
        cpl_image_delete( res->background);
        hdrl_casu_tfits_delete(res->catalogue);
    }

    /* Do the checks */
    cpl_test_rel(cpl_table_get_column_mean( tabfinal, "X_coordinate_err"),
    		     cpl_table_get_column_stdev(tabfinal, "X_coordinate"    ), COMP_TOL_REL);
    cpl_test_rel(cpl_table_get_column_mean( tabfinal, "Y_coordinate_err"),
    		     cpl_table_get_column_stdev(tabfinal, "Y_coordinate"    ), COMP_TOL_REL);
    cpl_test_rel(cpl_table_get_column_mean( tabfinal, "Peak_height_err" ),
    		     cpl_table_get_column_stdev(tabfinal, "Peak_height"     ), COMP_TOL_REL);
    cpl_test_rel(cpl_table_get_column_mean( tabfinal, "Aper_flux_1_err" ),
    		     cpl_table_get_column_stdev(tabfinal, "Aper_flux_1"     ), COMP_TOL_REL);
    cpl_test_rel(cpl_table_get_column_mean( tabfinal, "Aper_flux_2_err" ),
    		     cpl_table_get_column_stdev(tabfinal, "Aper_flux_2"     ), COMP_TOL_REL);
    cpl_test_rel(cpl_table_get_column_mean( tabfinal, "Aper_flux_3_err" ),
    		     cpl_table_get_column_stdev(tabfinal, "Aper_flux_3"     ), COMP_TOL_REL);
    cpl_test_rel(cpl_table_get_column_mean( tabfinal, "Aper_flux_4_err" ),
    		     cpl_table_get_column_stdev(tabfinal, "Aper_flux_4"     ), COMP_TOL_REL);
    cpl_test_rel(cpl_table_get_column_mean( tabfinal, "Aper_flux_5_err" ),
    		     cpl_table_get_column_stdev(tabfinal, "Aper_flux_5"     ), COMP_TOL_REL);
    cpl_test_rel(cpl_table_get_column_mean( tabfinal, "Aper_flux_6_err" ),
    		     cpl_table_get_column_stdev(tabfinal, "Aper_flux_6"     ), COMP_TOL_REL);
    cpl_test_rel(cpl_table_get_column_mean( tabfinal, "Aper_flux_7_err" ),
    		     cpl_table_get_column_stdev(tabfinal, "Aper_flux_7"     ), COMP_TOL_REL);
    cpl_test_rel(cpl_table_get_column_mean( tabfinal, "Aper_flux_8_err" ),
    		     cpl_table_get_column_stdev(tabfinal, "Aper_flux_8"     ), COMP_TOL_REL);
    cpl_test_rel(cpl_table_get_column_mean( tabfinal, "Aper_flux_9_err" ),
    		     cpl_table_get_column_stdev(tabfinal, "Aper_flux_9"     ), COMP_TOL_REL);
    cpl_test_rel(cpl_table_get_column_mean( tabfinal, "Aper_flux_10_err"),
    		     cpl_table_get_column_stdev(tabfinal, "Aper_flux_10"    ), COMP_TOL_REL);
    cpl_test_rel(cpl_table_get_column_mean( tabfinal, "Aper_flux_11_err"),
    		     cpl_table_get_column_stdev(tabfinal, "Aper_flux_11"    ), COMP_TOL_REL);
    cpl_test_rel(cpl_table_get_column_mean( tabfinal, "Aper_flux_12_err"),
    		     cpl_table_get_column_stdev(tabfinal, "Aper_flux_12"    ), COMP_TOL_REL);
    cpl_test_rel(cpl_table_get_column_mean( tabfinal, "Aper_flux_13_err"),
    		     cpl_table_get_column_stdev(tabfinal, "Aper_flux_13"    ), COMP_TOL_REL);
    cpl_test_rel(cpl_table_get_column_mean( tabfinal, "Petr_flux_err"   ),
    		     cpl_table_get_column_stdev(tabfinal, "Petr_flux"       ), COMP_TOL_REL);
    cpl_test_rel(cpl_table_get_column_mean( tabfinal, "Kron_flux_err"   ),
    		     cpl_table_get_column_stdev(tabfinal, "Kron_flux"       ), COMP_TOL_REL);
    cpl_test_rel(cpl_table_get_column_mean( tabfinal, "Half_flux_err"   ),
    		     cpl_table_get_column_stdev(tabfinal, "Half_flux"       ), COMP_TOL_REL);

    /* Clean up */
    hdrl_casu_fits_delete(inf);
    hdrl_casu_fits_delete(inconf);
    cpl_wcs_delete(wcs);
    cpl_table_delete(tabfinal);
    cpl_image_delete(bkg);
    cpl_free(res);
}

/*----------------------------------------------------------------------------*/
/**
 * @brief   Unit tests of hdrl_simulerror_montecarlo
 */
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    test_basic();

    return cpl_test_end(0);
}
