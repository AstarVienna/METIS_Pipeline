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


#define COMP_TOL_REL    1e-4
#define COMP_TOL_ABS    1e-2
#define IMG_XSIZE       110 * 2
#define IMG_YSIZE       150 * 2
#define CELL_SIZE       32
#define APER_FLUX_NUM  "Aper_flux_3"
#define NUM_FACTORS     5
#define NUM_BIASES      5
#define BIASES         {2., 100., 5000., 5.e4, 5.e5}
#define FACTORS        {2.,  10.,  100., 1.e4, 1.e6}


/*----------------------------------------------------------------------------*/
/**
 * @brief Check effect of multiplication on a given "Aper_flux",
 *        "Isophotal_flux", "FWHM", "Kron_radius", "Classification",
 *        "X_coordinate", "X_coordinate_err", "Y_coordinate" and "Y_coordinate_err".
 *
 * @note Magic numerical constants inherited from 'hdrl_casu_catalogue-test.c'
 *
 * @return cpl_error_code
 *
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_casumul_compute(double factor)
{
    /* Check inherited status */
    hdrl_casu_result *res1   = cpl_calloc(1, sizeof(hdrl_casu_result));
    hdrl_casu_result *res_p2 = cpl_calloc(1, sizeof(hdrl_casu_result));
    cpl_test_eq(hdrl_casu_catalogue(NULL, NULL, NULL, 5, 1.5, 1, 3.5, 1,
    		CELL_SIZE, 6, 1.0, 2.0, HDRL_SATURATION_INIT, res1), CPL_ERROR_NULL_INPUT);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_null(    res1->catalogue);
    hdrl_casu_tfits_delete(res1->catalogue);
    cpl_image_delete( res1->segmentation_map);
    cpl_image_delete( res1->background);
    cpl_free(res1);

    /* Generate a field with some stars and a confidence map */
    cpl_image *bkg    = cpl_image_new(IMG_XSIZE, IMG_YSIZE, CPL_TYPE_DOUBLE);
    cpl_image *im     = cpl_image_new(IMG_XSIZE, IMG_YSIZE, CPL_TYPE_DOUBLE);
    cpl_image *cnf    = cpl_image_new(IMG_XSIZE, IMG_YSIZE, CPL_TYPE_DOUBLE);

    double sigma = 2.;
    double norm2 = 2. * CPL_MATH_PI * sigma * sigma;
    cpl_image_fill_noise_uniform(bkg, -10., 10.);

    double sky = 500.;
    cpl_image_add_scalar(bkg, sky);
    cpl_image_fill_noise_uniform(cnf, 99.9, 100.1);

    double xpos =  100.;
    double ypos =  100.;
    double norm = 5000.;
    cpl_image_fill_gaussian(im, xpos, ypos, norm * norm2, sigma, sigma);
    cpl_image_add(bkg, im);
    cpl_image_delete(im);

    /* Mult */
    cpl_image *bkg2 = cpl_image_duplicate(bkg);
    cpl_image_multiply_scalar(bkg2, factor);

    hdrl_casu_fits        *inf    = hdrl_casu_fits_wrap( bkg);
    hdrl_casu_fits        *inf2   = hdrl_casu_fits_wrap(bkg2);
    hdrl_casu_fits        *inconf = hdrl_casu_fits_wrap( cnf);
    
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
    hdrl_casu_result * res2 = cpl_calloc(1, sizeof(hdrl_casu_result));
    cpl_test_eq(hdrl_casu_catalogue(inf, inconf, wcs, 5, 1.5, 0, 5.0, 1,
    		CELL_SIZE, HDRL_CATALOGUE_ALL, 3., 1., HDRL_SATURATION_INIT, res2), CPL_ERROR_NONE);
    cpl_test_nonnull(res2->catalogue);
    hdrl_casu_fits_delete(inf);
    
    /* Run casu catalogue for Mul*/
    cpl_test_eq(hdrl_casu_catalogue(inf2, inconf, wcs, 5, 1.5, 0, 5., 1,
    		CELL_SIZE, HDRL_CATALOGUE_ALL, 3., 1., HDRL_SATURATION_INIT, res_p2), CPL_ERROR_NONE);
    cpl_test_nonnull(res_p2->catalogue);
    hdrl_casu_fits_delete(inf2);
    hdrl_casu_fits_delete(inconf);
    cpl_wcs_delete(wcs);

    /*** Tests ***/
    int nl;

    cpl_test_image_rel(res2->segmentation_map, res_p2->segmentation_map, COMP_TOL_REL);

    cpl_table *tab = hdrl_casu_tfits_get_table(res2->catalogue);
    cpl_test_nonnull(tab);

    cpl_table *tab_p2 = hdrl_casu_tfits_get_table(res_p2->catalogue);
    cpl_test_nonnull(tab_p2);


    cpl_test_rel( cpl_table_get_double(tab,    APER_FLUX_NUM,      0, &nl),
                  cpl_table_get_double(tab_p2, APER_FLUX_NUM,      0, &nl) / factor, COMP_TOL_REL );
    cpl_test_rel( cpl_table_get_double(tab,    "Isophotal_flux",   0, &nl),
                  cpl_table_get_double(tab_p2, "Isophotal_flux",   0, &nl) / factor, COMP_TOL_REL );
    cpl_test_rel( cpl_table_get_double(tab,    "FWHM",             0, &nl),
                  cpl_table_get_double(tab_p2, "FWHM",             0, &nl),          COMP_TOL_REL );
    cpl_test_rel( cpl_table_get_double(tab,    "Kron_radius",      0, &nl),
                  cpl_table_get_double(tab_p2, "Kron_radius",      0, &nl),          COMP_TOL_REL );
    cpl_test_eq ( cpl_table_get_double(tab,    "Classification",   0, &nl),
                  cpl_table_get_double(tab_p2, "Classification",   0, &nl)                        );
    cpl_test_rel( cpl_table_get_double(tab,    "X_coordinate",     0, &nl),
                  cpl_table_get_double(tab_p2, "X_coordinate",     0, &nl),          COMP_TOL_REL );
    cpl_test_abs( cpl_table_get_double(tab,    "X_coordinate_err", 0, &nl),
                  cpl_table_get_double(tab_p2, "X_coordinate_err", 0, &nl),          COMP_TOL_ABS );
    cpl_test_rel( cpl_table_get_double(tab,    "Y_coordinate",     0, &nl),
                  cpl_table_get_double(tab_p2, "Y_coordinate",     0, &nl),          COMP_TOL_REL );
    cpl_test_abs( cpl_table_get_double(tab,    "Y_coordinate_err", 0, &nl),
                  cpl_table_get_double(tab_p2, "Y_coordinate_err", 0, &nl),          COMP_TOL_ABS );
                      
    /* Clean up */
    cpl_image_delete( res2->segmentation_map);
    cpl_image_delete( res2->background);
    hdrl_casu_tfits_delete(res2->catalogue);
    cpl_free(res2);

    cpl_image_delete( res_p2->segmentation_map);
    cpl_image_delete( res_p2->background);
    hdrl_casu_tfits_delete(res_p2->catalogue);
    cpl_free(res_p2);


    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Check effect of addition on a given "Aper_flux",
  "Isophotal_flux", "FWHM", "Kron_radius", "Classification", "X_coordinate", 
  "X_coordinate_err", "Y_coordinate" and "Y_coordinate_err".
  @note Magic numerical constants inherited from 'hdrl_casu_catalogue-test.c'
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_casuadd_compute(double bias)
{
    /* Check inherited status */
    hdrl_casu_result *res1   = cpl_calloc(1, sizeof(hdrl_casu_result));
    hdrl_casu_result *res_p2 = cpl_calloc(1, sizeof(hdrl_casu_result));
    cpl_test_eq(hdrl_casu_catalogue(NULL, NULL, NULL, 5, 1.5, 1, 3.5, 1,
    		CELL_SIZE, 6, 1., 2., HDRL_SATURATION_INIT, res1), CPL_ERROR_NULL_INPUT);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_null(res1->catalogue);
    hdrl_casu_tfits_delete(res1->catalogue);
    cpl_image_delete( res1->segmentation_map);
    cpl_image_delete( res1->background);
    cpl_free(res1);

    /* Generate a field with some stars and a confidence map */
    cpl_image *bkg = cpl_image_new(IMG_XSIZE, IMG_YSIZE, CPL_TYPE_DOUBLE);
    cpl_image *im  = cpl_image_new(IMG_XSIZE, IMG_YSIZE, CPL_TYPE_DOUBLE);
    cpl_image *cnf = cpl_image_new(IMG_XSIZE, IMG_YSIZE, CPL_TYPE_DOUBLE);

    double sigma = 2.;
    double norm2 = 2. * CPL_MATH_PI * sigma * sigma;
    cpl_image_fill_noise_uniform(bkg, -10., 10.);

    double sky = 500.;
    cpl_image_add_scalar(bkg, sky);

    cpl_image_fill_noise_uniform(cnf, 99.9, 100.1);

    double xpos =  100.;
    double ypos =  100.;
    double norm = 1000.;
    cpl_image_fill_gaussian(im, xpos, ypos, norm * norm2, sigma, sigma);
    cpl_image_add(bkg, im);
    cpl_image_delete(im);

    /* Add */
    cpl_image *bkg2 = cpl_image_duplicate(bkg);
    cpl_image_add_scalar(bkg2, bias);

    hdrl_casu_fits        *inf    = hdrl_casu_fits_wrap( bkg);
    hdrl_casu_fits        *inf2   = hdrl_casu_fits_wrap(bkg2);
    hdrl_casu_fits        *inconf = hdrl_casu_fits_wrap( cnf);
    
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
    hdrl_casu_result *res2 = cpl_calloc(1, sizeof(hdrl_casu_result));
    cpl_test_eq(hdrl_casu_catalogue(inf, inconf, wcs, 5, 1.5, 0, 5., 1,
    		CELL_SIZE, HDRL_CATALOGUE_ALL, 3., 1., HDRL_SATURATION_INIT, res2), CPL_ERROR_NONE);
    cpl_test_nonnull(res2->catalogue);
    hdrl_casu_fits_delete(inf);
    
    /* Run casu catalogue for ADD*/
    cpl_test_eq(hdrl_casu_catalogue(inf2, inconf, wcs, 5, 1.5, 0, 5., 1,
    		CELL_SIZE, HDRL_CATALOGUE_ALL, 3., 1., HDRL_SATURATION_INIT, res_p2), CPL_ERROR_NONE);
    cpl_test_nonnull(res_p2->catalogue);
    hdrl_casu_fits_delete(inf2);
    hdrl_casu_fits_delete(inconf);
    cpl_wcs_delete(wcs);

    /*** Tests ***/
    int nl;

    cpl_test_image_rel(res2->segmentation_map, res_p2->segmentation_map, COMP_TOL_REL);

    cpl_table *tab = hdrl_casu_tfits_get_table(res2->catalogue);
    cpl_test_nonnull(tab);

    cpl_table *tab_p2 = hdrl_casu_tfits_get_table(res_p2->catalogue);
    cpl_test_nonnull(tab_p2);

    cpl_test_rel( cpl_table_get_double(tab,    APER_FLUX_NUM,      0, &nl),
                  cpl_table_get_double(tab_p2, APER_FLUX_NUM,      0, &nl), COMP_TOL_REL );
    cpl_test_rel( cpl_table_get_double(tab,    "Isophotal_flux",   0, &nl),
                  cpl_table_get_double(tab_p2, "Isophotal_flux",   0, &nl), COMP_TOL_REL );
    cpl_test_rel( cpl_table_get_double(tab,    "FWHM",             0, &nl),
                  cpl_table_get_double(tab_p2, "FWHM",             0, &nl), COMP_TOL_REL );
    cpl_test_rel( cpl_table_get_double(tab,    "Kron_radius",      0, &nl),
                  cpl_table_get_double(tab_p2, "Kron_radius",      0, &nl), COMP_TOL_REL );
    cpl_test_eq ( cpl_table_get_double(tab,    "Classification",   0, &nl),
                  cpl_table_get_double(tab_p2, "Classification",   0, &nl)               );
    cpl_test_rel( cpl_table_get_double(tab,    "X_coordinate",     0, &nl),
                  cpl_table_get_double(tab_p2, "X_coordinate",     0, &nl), COMP_TOL_REL );
    cpl_test_abs( cpl_table_get_double(tab,    "X_coordinate_err", 0, &nl),
                  cpl_table_get_double(tab_p2, "X_coordinate_err", 0, &nl), COMP_TOL_ABS );
    cpl_test_rel( cpl_table_get_double(tab,    "Y_coordinate",     0, &nl),
                  cpl_table_get_double(tab_p2, "Y_coordinate",     0, &nl), COMP_TOL_REL );
    cpl_test_abs( cpl_table_get_double(tab,    "Y_coordinate_err", 0, &nl),
                  cpl_table_get_double(tab_p2, "Y_coordinate_err", 0, &nl), COMP_TOL_ABS );

    /* Clean up */
    cpl_image_delete( res2->segmentation_map);
    cpl_image_delete( res2->background);
    hdrl_casu_tfits_delete(res2->catalogue);
    cpl_free(res2);
    
    cpl_image_delete( res_p2->segmentation_map);
    cpl_image_delete( res_p2->background);
    hdrl_casu_tfits_delete(res_p2->catalogue);
    cpl_free(res_p2);


    return cpl_error_get_code();
}


/*----------------------------------------------------------------------------*/
/**
  @brief   Unit tests of addition or multiplication of some scalar value
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    double bias[] = BIASES;
    for (cpl_size i = 0; i < NUM_BIASES; i++) {
        hdrl_casuadd_compute(bias[i]);
    }

    double factor[] = FACTORS;
    for (cpl_size i = 0; i < NUM_FACTORS; i++) {
        hdrl_casumul_compute(factor[i]);
    }
    
    cpl_test_error(CPL_ERROR_NONE);

    return cpl_test_end(0);
}
