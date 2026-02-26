/*
 * This file is part of the HDRL
 * Copyright (C) 2013,2014 European Southern Observatory
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                    Includes
 -----------------------------------------------------------------------------*/

#include "hdrl.h"

#include <cpl.h>
#include <math.h>
#include <string.h>

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_catalogue_test    Testing of the HDRL catalogue
 */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief Check hdrl_catalogue_compute() in various conditions
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_catalogue_test_compute(void)
{
    cpl_image * img, * cnf, * flat_img, * peak_img;
    cpl_wcs * wcs;
    hdrl_parameter * par;
    hdrl_catalogue_result * r;
    cpl_size nx = 1001, ny = 753;

    img = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
    cnf = cpl_image_new(nx, ny, CPL_TYPE_INT);

    /* Create a completely flat image with all pixels set to 1.0 */
    flat_img = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
    cpl_test_nonnull(flat_img);
    cpl_image_add_scalar(flat_img, 1.0);
    cpl_test_error(CPL_ERROR_NONE);

    /* Create an image with all pixels set to 1.0 and a single sharp Gaussian
       peak in the image at coordinate (100, 100). */
    peak_img = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
    cpl_test_nonnull(peak_img);
    cpl_image_fill_gaussian(peak_img, 100, 100, 1000.0, 1.0, 1.0);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_image_add_scalar(peak_img, 1.0);
    cpl_test_error(CPL_ERROR_NONE);

    cpl_propertylist * pl = cpl_propertylist_new();
    cpl_propertylist_update_string(pl, "CTYPE1", "RA---TAN");
    cpl_propertylist_update_string(pl, "CTYPE2", "DEC--TAN");
    cpl_propertylist_update_double(pl, "CRVAL1", 30.0);
    cpl_propertylist_update_double(pl, "CRVAL2", 12.0);
    cpl_propertylist_update_double(pl, "CRPIX1", 512.0);
    cpl_propertylist_update_double(pl, "CRPIX2", 512.0);
    cpl_propertylist_update_double(pl, "CD1_1", -1.0 / 3600);
    cpl_propertylist_update_double(pl, "CD1_2", 0.0);
    cpl_propertylist_update_double(pl, "CD2_1", 0.0);
    cpl_propertylist_update_double(pl, "CD2_2", 1.0 / 3600);
    cpl_propertylist_update_int(pl, "NAXIS1", nx);
    cpl_propertylist_update_int(pl, "NAXIS2", ny);
    wcs = cpl_wcs_new_from_propertylist(pl);
    cpl_propertylist_delete(pl);

    par = hdrl_catalogue_parameter_create(3, 2.5, CPL_FALSE, 3., CPL_TRUE, 64,
                                          2., 2.0, HDRL_SATURATION_INIT,
                                          HDRL_CATALOGUE_ALL);

    /* check null input errors */
    for (size_t i = 0; i < 2; i++) {
       for (size_t j = 0; j < 2; j++) {
           for (size_t k = 0; k < 2; k++) {
               for (size_t l = 0; l < 2; l++) {
                   /* img and par are enough, rest optional */
                   if (i && l)
                       continue;
                   r = hdrl_catalogue_compute(i ? img  : NULL,
                                              j ? cnf  : NULL,
                                              k ? wcs  : NULL,
                                              l ? par  : NULL);
                   cpl_test_null(r);
                   cpl_test_error(CPL_ERROR_NULL_INPUT);
                   hdrl_catalogue_result_delete(r);
               }
           }
       } 
    }

    /* zero image */
    // TODO mem-leaks
    //r = hdrl_catalogue_compute(img , NULL, NULL, par);
    //cpl_test_null(r);
    //cpl_test_error(CPL_ERROR_TODO);

    /* check basic object detection and results */
    hdrl_parameter_destroy(par);
    par = hdrl_catalogue_parameter_create(5, 1.5, CPL_FALSE, 5., CPL_TRUE, 64,
                                          3., 1.0, HDRL_SATURATION_INIT,
                                          HDRL_CATALOGUE_ALL);
    cpl_image *bkg = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
    double sigma = 2., sky = 500.;
    double norm2 = 2.0 * CPL_MATH_PI * sigma * sigma;
    cpl_image_add_scalar(cnf, 100.);
    double xpos[] = {100.0,200.0,300.0,400.0,500.0,600.0,700.0,800.0,900.0,
                     950.0};
    double ypos[] = {100.0,200.0,300.0,400.0,550.0,600.0,650.0,700.0,230.0,
                     170.0};
    double norm[] = {1000.0,100.0,200.0,500.0,550.0,600.0,650.0,700.0,
                     750.0,800.0};
    size_t nobj = sizeof(xpos) / sizeof(xpos[0]);
    for (size_t i = 0; i < nobj; i++) {
        cpl_image_fill_gaussian(bkg, xpos[i], ypos[i], norm[i] * norm2, sigma,
                                sigma);
        cpl_image_add(img, bkg);
    }
    cpl_image_fill_noise_uniform(bkg, -10.0, 10.0);
    cpl_image_add_scalar(bkg, sky);
    cpl_image_add(img, bkg);
    cpl_image_delete(bkg);

    r = hdrl_catalogue_compute(img , NULL, NULL, par);
    cpl_test_nonnull(r);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_nonnull(r->catalogue);
    cpl_test_eq(cpl_table_get_nrow(r->catalogue), nobj);
    cpl_test_nonnull(r->segmentation_map);
    cpl_test_eq(cpl_image_get_max(r->segmentation_map), nobj);
    cpl_test_eq(cpl_image_get_min(r->segmentation_map), 0);
    cpl_test_nonnull(r->background);
    cpl_test_abs(cpl_image_get_mean(r->background), sky, 5);
    cpl_test_nonnull(r->qclist);
    // TODO check required keys
    hdrl_catalogue_result_delete(r);

    /* smoke test no background subtraction */
    hdrl_parameter * bpar =
        hdrl_catalogue_parameter_create(5, 1.5, CPL_FALSE, 5., CPL_FALSE, 64,
                                        3., 1.0, HDRL_SATURATION_INIT,
                                        HDRL_CATALOGUE_ALL);
    cpl_image * imgcor = cpl_image_subtract_scalar_create(img, sky);
    r = hdrl_catalogue_compute(imgcor, NULL, NULL, bpar);
    cpl_test_nonnull(r);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_nonnull(r->catalogue);
    cpl_test_eq(cpl_table_get_nrow(r->catalogue), nobj);
    cpl_test_nonnull(r->segmentation_map);
    cpl_test_eq(cpl_image_get_max(r->segmentation_map), nobj);
    cpl_test_eq(cpl_image_get_min(r->segmentation_map), 0);
    cpl_test_null(r->background);
    cpl_test_nonnull(r->qclist);
    cpl_image_delete(imgcor);
    hdrl_parameter_delete(bpar);
    hdrl_catalogue_result_delete(r);

    /* test too big nbsize */
    bpar = hdrl_catalogue_parameter_create(3, 2.5, CPL_FALSE, 3., CPL_TRUE,
                                           nx + 23, 2., 2.0,
                                           HDRL_SATURATION_INIT,
                                           HDRL_CATALOGUE_ALL);

    /* Check parameter */
    cpl_test(hdrl_catalogue_parameter_check(bpar));


    /* Change option */
    hdrl_catalogue_parameter_set_option(NULL, HDRL_CATALOGUE_ALL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    hdrl_catalogue_parameter_set_option(bpar, HDRL_CATALOGUE_BKG);
    cpl_test_error(CPL_ERROR_NONE);

    hdrl_catalogue_parameter_set_option(bpar, HDRL_CATALOGUE_ALL);
    cpl_test_error(CPL_ERROR_NONE);


	/* Create ParameterList */
	cpl_parameterlist *plCat;

	plCat = hdrl_catalogue_parameter_create_parlist(NULL, "catalogue", bpar);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(plCat);

	plCat = hdrl_catalogue_parameter_create_parlist("test", NULL, bpar);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(plCat);

	plCat = hdrl_catalogue_parameter_create_parlist("test", "catalogue", NULL);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(plCat);

	plCat = hdrl_catalogue_parameter_create_parlist("test", "catalogue", bpar);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(plCat);


	/* Parse ParameterList */
	hdrl_parameter *check;

	check = hdrl_catalogue_parameter_parse_parlist(NULL, "test.catalogue");
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(check);

	check = hdrl_catalogue_parameter_parse_parlist(plCat, NULL);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(check);

	check = hdrl_catalogue_parameter_parse_parlist(plCat, "test.catalogue");
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(check);


	cpl_parameterlist_delete(plCat);
	hdrl_parameter_delete(check);



	/* Compute */
    r = hdrl_catalogue_compute(img , NULL, NULL, bpar);
    cpl_test_nonnull(r);
    cpl_test_error(CPL_ERROR_NONE);
    hdrl_catalogue_result_delete(r);
    hdrl_parameter_delete(bpar);

    /* test bad confidence */
    cpl_image_subtract_scalar(cnf, 200);
    r = hdrl_catalogue_compute(img , cnf, NULL, par);
    cpl_test_null(r);
    cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_image_add_scalar(cnf, 200);

    /* test double confidence */
    cpl_image * dcnf = cpl_image_cast(cnf, CPL_TYPE_DOUBLE);
    r = hdrl_catalogue_compute(img , dcnf, NULL, par);
    cpl_test_nonnull(r);
    cpl_test_nonnull(r->catalogue);
    cpl_test_nonnull(r->segmentation_map);
    cpl_test_nonnull(r->background);
    cpl_test_error(CPL_ERROR_NONE);
    hdrl_catalogue_result_delete(r);

    /* smoketest image with bad pixels */
    cpl_image_reject(img, 60, 23);
    r = hdrl_catalogue_compute(img , NULL, NULL, par);
    cpl_test_nonnull(r);
    cpl_test_error(CPL_ERROR_NONE);
    hdrl_catalogue_result_delete(r);

    /* smoketest image with bad pixels and confidence */
    r = hdrl_catalogue_compute(img , cnf, NULL, par);
    cpl_test_nonnull(r);
    cpl_test_nonnull(r->catalogue);
    cpl_test_nonnull(r->segmentation_map);
    cpl_test_nonnull(r->background);
    cpl_test_error(CPL_ERROR_NONE);
    hdrl_catalogue_result_delete(r);

    /* smoketest image with bad pixels and double confidence */
    r = hdrl_catalogue_compute(img , dcnf, NULL, par);
    cpl_test_nonnull(r);
    cpl_test_nonnull(r->catalogue);
    cpl_test_nonnull(r->segmentation_map);
    cpl_test_nonnull(r->background);
    cpl_test_error(CPL_ERROR_NONE);
    hdrl_catalogue_result_delete(r);
    cpl_image_delete(dcnf);

    /* smoketest image and confidence */
    cpl_image_accept_all(img);
    r = hdrl_catalogue_compute(img , cnf, NULL, par);
    cpl_test_nonnull(r);
    cpl_test_error(CPL_ERROR_NONE);
    hdrl_catalogue_result_delete(r);

    /* smoketest image, confidence and wcs */
    cpl_image_accept_all(img);
    r = hdrl_catalogue_compute(img , cnf, wcs, par);
    cpl_test_nonnull(r);
    cpl_test_nonnull(r->catalogue);
    cpl_test_nonnull(r->segmentation_map);
    cpl_test_nonnull(r->background);
    cpl_test_error(CPL_ERROR_NONE);
    hdrl_catalogue_result_delete(r);

    /* smoketest double image, confidence and wcs */
    cpl_image * dimg = cpl_image_cast(img, CPL_TYPE_DOUBLE);
    cpl_image_accept_all(img);
    r = hdrl_catalogue_compute(dimg , cnf, wcs, par);
    cpl_test_nonnull(r);
    cpl_test_nonnull(r->catalogue);
    cpl_test_nonnull(r->segmentation_map);
    cpl_test_nonnull(r->background);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_image_delete(dimg);
    hdrl_catalogue_result_delete(r);

    /* test no segmap and background */
    hdrl_parameter_delete(par);
    par = hdrl_catalogue_parameter_create(5, 1.5, CPL_FALSE, 5., CPL_TRUE, 64,
                                          3., 1.0, HDRL_SATURATION_INIT,
                                          HDRL_CATALOGUE_CAT_COMPLETE);
    r = hdrl_catalogue_compute(img , cnf, wcs, par);
    cpl_test_nonnull(r);
    cpl_test_nonnull(r->catalogue);
    cpl_test_null(r->segmentation_map);
    cpl_test_null(r->background);
    cpl_test_error(CPL_ERROR_NONE);
    hdrl_catalogue_result_delete(r);

    /* test no segmap */
    hdrl_parameter_delete(par);
    par = hdrl_catalogue_parameter_create(5, 1.5, CPL_FALSE, 5., CPL_TRUE, 64,
                                          3., 1.0, HDRL_SATURATION_INIT,
                                          HDRL_CATALOGUE_CAT_COMPLETE |
                                          HDRL_CATALOGUE_BKG);
    r = hdrl_catalogue_compute(img , cnf, wcs, par);
    cpl_test_nonnull(r);
    cpl_test_nonnull(r->catalogue);
    cpl_test_null(r->segmentation_map);
    cpl_test_nonnull(r->background);
    cpl_test_error(CPL_ERROR_NONE);
    hdrl_catalogue_result_delete(r);

    /* test no background */
    hdrl_parameter_delete(par);
    par = hdrl_catalogue_parameter_create(5, 1.5, CPL_FALSE, 5., CPL_TRUE, 64,
                                          3., 1.0, HDRL_SATURATION_INIT,
                                          HDRL_CATALOGUE_CAT_COMPLETE |
                                          HDRL_CATALOGUE_SEGMAP);
    r = hdrl_catalogue_compute(img , cnf, wcs, par);
    cpl_test_nonnull(r);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_nonnull(r->catalogue);
    cpl_test_nonnull(r->segmentation_map);
    cpl_test_null(r->background);
    hdrl_catalogue_result_delete(r);

    /* test no catalogue */
    hdrl_parameter_delete(par);
    par = hdrl_catalogue_parameter_create(5, 1.5, CPL_FALSE, 5., CPL_TRUE, 64,
                                          3., 1.0, HDRL_SATURATION_INIT,
                                          HDRL_CATALOGUE_SEGMAP |
                                          HDRL_CATALOGUE_BKG);
    r = hdrl_catalogue_compute(img , cnf, wcs, par);
    cpl_test_nonnull(r);
    cpl_test_nonnull(r->catalogue);
    cpl_test_eq(cpl_table_get_nrow(r->catalogue), 0);
    cpl_test_nonnull(r->segmentation_map);
    cpl_test_nonnull(r->background);
    cpl_test_error(CPL_ERROR_NONE);
    hdrl_catalogue_result_delete(r);

    /* Test pathological case with a completely flat image. */
    hdrl_parameter_delete(par);
    par = hdrl_catalogue_parameter_create(5, 1.5, CPL_FALSE, 5., CPL_TRUE, 64,
                                          3., 1.0, HDRL_SATURATION_INIT,
                                          HDRL_CATALOGUE_CAT_COMPLETE |
                                          HDRL_CATALOGUE_SEGMAP |
                                          HDRL_CATALOGUE_BKG);
    cpl_test_nonnull(par);
    r = hdrl_catalogue_compute(flat_img , NULL, NULL, par);
    cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
    cpl_test_nonnull(r);
    hdrl_catalogue_result_delete(r);

    /* Test pathological case with a single sharp peak in the image. */
    hdrl_parameter_delete(par);
    par = hdrl_catalogue_parameter_create(5, 1.5, CPL_FALSE, 5., CPL_TRUE, 64,
                                          3., 1.0, HDRL_SATURATION_INIT,
                                          HDRL_CATALOGUE_CAT_COMPLETE |
                                          HDRL_CATALOGUE_SEGMAP |
                                          HDRL_CATALOGUE_BKG);
    cpl_test_nonnull(par);
    r = hdrl_catalogue_compute(peak_img , NULL, NULL, par);
    cpl_test_nonnull(r);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_nonnull(r->catalogue);
    cpl_test_nonnull(r->segmentation_map);
    cpl_test_nonnull(r->background);
    hdrl_catalogue_result_delete(r);

    cpl_image_delete(flat_img);
    cpl_image_delete(peak_img);
    cpl_image_delete(img);
    cpl_image_delete(cnf);
    cpl_wcs_delete(wcs);
    hdrl_parameter_delete(par);
    return cpl_error_get_code();
}


/*----------------------------------------------------------------------------*/
/**
  @brief   Unit tests of catalogue module
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    hdrl_catalogue_test_compute();
    cpl_test_error(CPL_ERROR_NONE);


    return cpl_test_end(0);
}
