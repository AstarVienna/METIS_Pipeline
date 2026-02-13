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


#define NTEST           10
#define COMP_TOLERANCE  1e-2
#define CORNER_OFFSET   10
#define CORNER_REL_TOL  1e-2
#define X_OS_P1         60
#define X_OS_P2         95
#define STAR            7
#define PATCH_SIZE      100		/* even number (patch is a square) */
#define IMG_XSIZE       1100
#define IMG_YSIZE       1500
#define CELL_SIZE       32
#define MIN_RAMP        10.
#define MAX_RAMP        100.
#define APER_FLUX_NUM  "Aper_flux_3"

/*----------------------------------------------------------------------------*/
/**
 * @brief Check effect of bad pixel square patches in various conditions
 *
 * @note Magic numerical constants inherited from 'hdrl_casu_catalogue-test.c'
 *
 * @return cpl_error_code
 *
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_casubkg_badpatch_compute(void)
{
    double xpos[] = { 100., 200., 300., 400., 500., 600., 700., 800., 900., 1000.};
    double ypos[] = { 100., 200., 300., 400., 500., 600., 700., 800., 900., 1000.};
    double norm[] = {1000., 100., 200., 500., 550., 600., 650., 700., 750.,  800.};

    /* Check inherited status */
    hdrl_casu_result *res    = cpl_calloc(1, sizeof(hdrl_casu_result));
    hdrl_casu_result *res_p1 = cpl_calloc(1, sizeof(hdrl_casu_result));
    hdrl_casu_result *res_p2 = cpl_calloc(1, sizeof(hdrl_casu_result));
    cpl_test_eq(hdrl_casu_catalogue(NULL, NULL, NULL, 5, 1.5, 1, 3.5, 1, CELL_SIZE,
                   6, 1.0, 2.0, HDRL_SATURATION_INIT, res), CPL_ERROR_NULL_INPUT);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_null(res->catalogue);

    cpl_image_delete(res->segmentation_map);
    cpl_image_delete(res->background);
    hdrl_casu_tfits_delete(res->catalogue);
    cpl_free(res);

    /* Generate a field with some stars and a confidence map */
    cpl_image *bkg = cpl_image_new(IMG_XSIZE, IMG_YSIZE, CPL_TYPE_DOUBLE);
    cpl_image *im  = cpl_image_new(IMG_XSIZE, IMG_YSIZE, CPL_TYPE_DOUBLE);
    cpl_image *cnf = cpl_image_new(IMG_XSIZE, IMG_YSIZE, CPL_TYPE_DOUBLE);
    cpl_image *aux = cpl_image_new(IMG_XSIZE, IMG_YSIZE, CPL_TYPE_DOUBLE);

    double sigma = 2.;
    double norm2 = 2. * CPL_MATH_PI * sigma * sigma;
    cpl_image_fill_noise_uniform(bkg, -10., 10.);

    double sky = 500.;
    cpl_image_add_scalar(bkg, sky);

    cpl_image_fill_noise_uniform(cnf, 99.9, 100.1);
    cpl_image *cnf_p1 = cpl_image_duplicate(cnf);
    cpl_image *cnf_p2 = cpl_image_duplicate(cnf);

    double tot[NTEST];
    for (cpl_size i = 0; i < NTEST; i++) {
        cpl_image_fill_gaussian(im, xpos[i], ypos[i], norm[i] * norm2, sigma, sigma);
        tot[i] = cpl_image_get_flux(im);
        cpl_image_add(bkg, im);
    }
    cpl_image_delete(im);
    
    /* Add gradient */
    double dlt_ramp = MAX_RAMP - MIN_RAMP;
    for (cpl_size i = 1; i <= IMG_XSIZE; i++) {
        for (cpl_size j = 1; j <= IMG_YSIZE; j++) {
            cpl_image_set(aux, i, j, MIN_RAMP + dlt_ramp * (double)i / IMG_XSIZE);
        }
    }
    cpl_image_add(bkg, aux);
    cpl_image_delete(aux);

    cpl_size y_os = 0;
    
    /* Generation of confidence map "Patch Close" */
    cpl_size x_os = X_OS_P1;
    for (cpl_size i = 0; i <= PATCH_SIZE; i++) {
        for (cpl_size j = 0; j <= PATCH_SIZE; j++) {
            cpl_image_set(cnf_p1, xpos[STAR]-(PATCH_SIZE/2)+i +x_os, 
                                  ypos[STAR]-(PATCH_SIZE/2)+j +y_os, 0);
        }
    }
    
    /* Generation of confidence map "Patch Away" */
    x_os = X_OS_P2;
    for (cpl_size i = 0; i <= PATCH_SIZE; i++) {
        for (cpl_size j = 0; j <= PATCH_SIZE; j++) {
            cpl_image_set(cnf_p2, xpos[STAR]-(PATCH_SIZE/2)+i +x_os, 
                                  ypos[STAR]-(PATCH_SIZE/2)+j +y_os, 0);
        }
    }

    hdrl_casu_fits        *inf       = hdrl_casu_fits_wrap(   bkg);
    hdrl_casu_fits        *inconf    = hdrl_casu_fits_wrap(   cnf);
    hdrl_casu_fits        *inconf_p1 = hdrl_casu_fits_wrap(cnf_p1);
    hdrl_casu_fits        *inconf_p2 = hdrl_casu_fits_wrap(cnf_p2);
    
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

    /* Run casu catalogue */
    res = cpl_calloc(1, sizeof(hdrl_casu_result));
    cpl_test_eq(hdrl_casu_catalogue(inf, inconf, wcs, 5, 1.5, 0, 5., 1,
    		CELL_SIZE, HDRL_CATALOGUE_ALL, 3., 1., HDRL_SATURATION_INIT, res), CPL_ERROR_NONE);
    cpl_test_nonnull(res->catalogue);
    hdrl_casu_fits_delete(inconf);
    
    /* Run casu catalogue for PC */
    cpl_test_eq(hdrl_casu_catalogue(inf, inconf_p1, wcs, 5, 1.5, 0, 5., 1,
    		CELL_SIZE, HDRL_CATALOGUE_ALL, 3., 1., HDRL_SATURATION_INIT, res_p1), CPL_ERROR_NONE);
    cpl_test_nonnull(res_p1->catalogue);
    hdrl_casu_fits_delete(inconf_p1);

    /* Run casu catalogue for PA*/
    cpl_test_eq(hdrl_casu_catalogue(inf, inconf_p2, wcs, 5, 1.5, 0, 5., 1,
    		CELL_SIZE, HDRL_CATALOGUE_ALL, 3., 1., HDRL_SATURATION_INIT, res_p2), CPL_ERROR_NONE);
    cpl_test_nonnull(res_p2->catalogue);
    hdrl_casu_fits_delete(inconf_p2);
    hdrl_casu_fits_delete(inf);
    cpl_wcs_delete(wcs);

    /*** Tests ***/

    cpl_test_image_rel(res->background, res_p1->background, COMP_TOLERANCE);
    cpl_test_image_rel(res->background, res_p2->background, COMP_TOLERANCE);
    
    cpl_size corner_x = cpl_image_get_size_x(res->background) -CORNER_OFFSET;
    cpl_size corner_y = cpl_image_get_size_y(res->background) -CORNER_OFFSET;

    int rej;
    cpl_test_abs(cpl_image_get(res   ->background, corner_x, corner_y, &rej),
                 cpl_image_get(res_p1->background, corner_x, corner_y, &rej), CORNER_REL_TOL);
    cpl_test_abs(cpl_image_get(res   ->background, corner_x, corner_y, &rej),
    		     cpl_image_get(res_p2->background, corner_x, corner_y, &rej), CORNER_REL_TOL);

    cpl_test_image_rel(res->segmentation_map, res_p1->segmentation_map, COMP_TOLERANCE);
    cpl_test_image_rel(res->segmentation_map, res_p2->segmentation_map, COMP_TOLERANCE);
    

    /* Check the results. Start by checking the number of rows and columns. Sort the table by X */
    cpl_table *tab    = hdrl_casu_tfits_get_table(res   ->catalogue);
    cpl_table *tab_p1 = hdrl_casu_tfits_get_table(res_p1->catalogue);
    cpl_table *tab_p2 = hdrl_casu_tfits_get_table(res_p2->catalogue);
    cpl_test_nonnull(tab);
    cpl_test_nonnull(tab_p1);
    cpl_test_nonnull(tab_p2);
    
    int nl;
    for (cpl_size i = 0; i < NTEST; i++) {
        cpl_test_rel( cpl_table_get_double(tab,    APER_FLUX_NUM, i, &nl),
                      cpl_table_get_double(tab_p1, APER_FLUX_NUM, i, &nl), COMP_TOLERANCE );
        cpl_test_rel( cpl_table_get_double(tab,    APER_FLUX_NUM, i, &nl),
                      cpl_table_get_double(tab_p2, APER_FLUX_NUM, i, &nl), COMP_TOLERANCE );
    }
    
    cpl_test_eq(cpl_table_get_ncol(tab), 63);
    cpl_test_eq(cpl_table_get_nrow(tab), NTEST);
    pl = cpl_propertylist_new();
    cpl_propertylist_append_bool(pl, "X_coordinate", 0);
    cpl_table_sort(tab, pl);
    cpl_propertylist_delete(pl);

    /* Test the column content of the table */
    for (cpl_size i = 0; i < NTEST; i++) {
        cpl_test_abs(xpos[i], cpl_table_get_double(tab, "X_coordinate",    i, &nl), 0.2);
        cpl_test_abs(ypos[i], cpl_table_get_double(tab, "Y_coordinate",    i, &nl), 0.2);
        double diff =   fabs( cpl_table_get_double(tab, "Aper_flux_5",     i, &nl) -tot[i])
                      /       cpl_table_get_double(tab, "Aper_flux_5_err", i, &nl);
        cpl_test_lt(diff, 1.5);
        cpl_test_eq(          cpl_table_get_double(tab, "Classification",  i, &nl), -1.);
    }
    
    /* Clean up */
    cpl_image_delete( res->segmentation_map);
    cpl_image_delete( res->background);
    hdrl_casu_tfits_delete(res->catalogue);
    cpl_free(res);
    
    cpl_image_delete( res_p1->segmentation_map);
    cpl_image_delete( res_p1->background);
    hdrl_casu_tfits_delete(res_p1->catalogue);
    cpl_free(res_p1);
    
    cpl_image_delete( res_p2->segmentation_map);
    cpl_image_delete( res_p2->background);
    hdrl_casu_tfits_delete(res_p2->catalogue);
    cpl_free(res_p2);


    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
 * @brief   Unit tests of bad patches in background
 */
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    hdrl_casubkg_badpatch_compute();

    cpl_test_error(CPL_ERROR_NONE);

    return cpl_test_end(0);
}
