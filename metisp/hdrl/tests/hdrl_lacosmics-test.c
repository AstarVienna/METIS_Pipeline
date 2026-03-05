/*
 * This file is part of the HDRL
 * Copyright (C) 2013 European Southern Observatory
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

#include "hdrl_lacosmics.h"
#include "hdrl_bpm_2d.h"

#include <cpl.h>

#include <math.h>

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_lacosmics_test   Testing of the HDRL cosmic ray rejection
 */
/*----------------------------------------------------------------------------*/

cpl_error_code test_lacosmic_inputs(void)
{
	/* Create parameters */
	hdrl_parameter *pFake  = hdrl_bpm_2d_parameter_create_legendresmooth(
											4., 5., 6, 20, 21, 11, 12, 2, 10);
    hdrl_parameter *pErr1  = hdrl_lacosmic_parameter_create(  0,   0, 0);
    hdrl_parameter *pErr2  = hdrl_lacosmic_parameter_create(  0, -1., 1);
    hdrl_parameter *pErr3  = hdrl_lacosmic_parameter_create(-1.,   0, 1);
    hdrl_parameter *params = hdrl_lacosmic_parameter_create(5, 2., 5);
    cpl_test_error(CPL_ERROR_NONE);


    /* Check parameter */
    cpl_test(!hdrl_lacosmic_parameter_check(pFake ));
    cpl_test( hdrl_lacosmic_parameter_check(params));


    /* Verify parameter */
    hdrl_lacosmic_parameter_verify(NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    hdrl_lacosmic_parameter_verify(pFake);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    hdrl_lacosmic_parameter_verify(pErr1);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    hdrl_lacosmic_parameter_verify(pErr2);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    hdrl_lacosmic_parameter_verify(pErr3);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    hdrl_lacosmic_parameter_verify(params);
    cpl_test_error(CPL_ERROR_NONE);


    /* Gets */

    cpl_test_eq(hdrl_lacosmic_parameter_get_sigma_lim(NULL), -1.);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_eq(hdrl_lacosmic_parameter_get_sigma_lim(params), 5);
    cpl_test_error(CPL_ERROR_NONE);

    cpl_test_eq(hdrl_lacosmic_parameter_get_f_lim(NULL), -1.);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_eq(hdrl_lacosmic_parameter_get_f_lim(params), 2.);
	cpl_test_error(CPL_ERROR_NONE);

	cpl_test_eq(hdrl_lacosmic_parameter_get_max_iter(NULL), -1);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_eq(hdrl_lacosmic_parameter_get_max_iter(params), 5);
	cpl_test_error(CPL_ERROR_NONE);


	/* Create ParameterList */
	cpl_parameterlist *pl;

	pl = hdrl_lacosmic_parameter_create_parlist(NULL, "lacosmic", params);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(pl);

	pl = hdrl_lacosmic_parameter_create_parlist("test", NULL, params);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(pl);

	pl = hdrl_lacosmic_parameter_create_parlist("test", "lacosmic", NULL);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(pl);

	cpl_parameterlist *plFake;
	plFake = hdrl_lacosmic_parameter_create_parlist("test", "lacosmic", pFake);
	cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);
	cpl_test_null(plFake);

	pl = hdrl_lacosmic_parameter_create_parlist("test", "lacosmic", params);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(pl);


	/* Parse ParameterList */
	hdrl_parameter *check;

	check = hdrl_lacosmic_parameter_parse_parlist(NULL, "test.lacosmic");
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(check);

	check = hdrl_lacosmic_parameter_parse_parlist(pl, NULL);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(check);

	check = hdrl_lacosmic_parameter_parse_parlist(pl, "test.lacosmic");
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(check);


    /* image smaller than 7x7 kernel */
    hdrl_image *img1 = hdrl_image_new(6, 1000);
    cpl_mask   *res1 = hdrl_lacosmic_edgedetect(img1, params);
    cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_test_null(res1);
    hdrl_image_delete(img1);


    /* image larger */
    hdrl_image *img2 = hdrl_image_new(1200, 4);
    cpl_mask   *res2 = hdrl_lacosmic_edgedetect(img2, params);
    cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);
    cpl_test_null(res2);
    hdrl_image_delete(img2);


    /* Clean up */
	cpl_parameterlist_delete(pl);
    hdrl_parameter_delete(pFake);
    hdrl_parameter_delete(pErr1);
    hdrl_parameter_delete(pErr2);
    hdrl_parameter_delete(pErr3);
    hdrl_parameter_delete(params);
    hdrl_parameter_delete(check);

    return cpl_error_get_code();
}

cpl_error_code test_lacosmic_edgedetect(void)
{
    cpl_image * img_data  = NULL;
    cpl_mask  * img_mask = NULL;
    cpl_image * img_error  = NULL;
    cpl_mask *  result_mask = NULL;
    /* detect single pixel cosmics  */
    {
        img_data = cpl_image_new(200, 300, CPL_TYPE_DOUBLE);
        img_mask = cpl_mask_new(200, 300);
        cpl_image_fill_noise_uniform(img_data, 90, 110);
        double error = (110 - 90) / sqrt(12);
        cpl_image_set(img_data,  50,  50, 300.);
        cpl_image_set(img_data, 100, 100, 300.);
        cpl_image_set(img_data, 150, 150, 300.);
        cpl_image_set(img_data, 100, 250, 300.);

        img_error = cpl_image_new(cpl_image_get_size_x(img_data),
                                  cpl_image_get_size_y(img_data),
                                  CPL_TYPE_DOUBLE);
        cpl_image_add_scalar(img_error, error);

        cpl_mask_set(img_mask, 120,120, CPL_BINARY_1);
        cpl_mask_set(img_mask, 120,121, CPL_BINARY_1);
        cpl_mask_set(img_mask, 120,122, CPL_BINARY_1);
        cpl_mask_set(img_mask, 121,120, CPL_BINARY_1);
        cpl_mask_set(img_mask, 121,121, CPL_BINARY_1);
        cpl_mask_set(img_mask, 121,122, CPL_BINARY_1);
        cpl_mask_set(img_mask, 122,120, CPL_BINARY_1);
        cpl_mask_set(img_mask, 122,121, CPL_BINARY_1);
        cpl_mask_set(img_mask, 122,122, CPL_BINARY_1);
        /*set one outlier on a bad pixel*/
        cpl_image_set(img_data, 122, 122, 300.);

        cpl_image_reject_from_mask(img_data, img_mask);
        hdrl_image * image = hdrl_image_create(img_data, img_error);
        hdrl_parameter * params =
            hdrl_lacosmic_parameter_create(error * 2, 2.0, 5);
        result_mask = hdrl_lacosmic_edgedetect(image, params);
        hdrl_parameter_delete(params) ;

        /*
        cpl_image_save(data, "test_image.fits", CPL_TYPE_DOUBLE, NULL,
                       CPL_IO_CREATE);
        cpl_image_save(data_error, "test_error.fits", CPL_TYPE_DOUBLE, NULL,
                       CPL_IO_CREATE);
        cpl_mask_save(data_result, "test_cr.fits", NULL, CPL_IO_CREATE);
        */

        cpl_test_eq(cpl_mask_get(result_mask,  50,  50), CPL_BINARY_1);
        cpl_test_eq(cpl_mask_get(result_mask, 100, 100), CPL_BINARY_1);
        cpl_test_eq(cpl_mask_get(result_mask, 150, 150), CPL_BINARY_1);
        cpl_test_eq(cpl_mask_get(result_mask, 100, 250), CPL_BINARY_1);
        cpl_test_eq(cpl_mask_get(result_mask, 122, 122), CPL_BINARY_0);

        cpl_test_eq(cpl_mask_get(result_mask, 110, 260), CPL_BINARY_0);

        /* free the memory */
        cpl_image_delete(img_data);
        cpl_mask_delete(img_mask);
        cpl_image_delete(img_error);
        cpl_mask_delete(result_mask);
        hdrl_image_delete(image);
    }

    /* detect a very big rectangular cosmic  */
    {
        img_data = cpl_image_new(150, 200, CPL_TYPE_DOUBLE);
        img_mask = cpl_mask_new(150, 200);
        cpl_image_fill_noise_uniform(img_data, 90, 110);
        double error = (110 - 90) / sqrt(12);

        for (int varx = 50; varx < 75; ++varx) {
            for (int vary = 60; vary < 130; ++vary) {
                cpl_image_set(img_data, varx, vary, 5000);
            }
        }
        for (int varx = 20; varx < 120; ++varx) {
            for (int vary = 20; vary < 40; ++vary) {
                cpl_image_set(img_data, varx, vary, 5000);
            }
        }
        img_error = cpl_image_new(cpl_image_get_size_x(img_data),
                                  cpl_image_get_size_y(img_data),
                                  CPL_TYPE_DOUBLE);
        cpl_image_add_scalar(img_error, error);

        hdrl_image * image = hdrl_image_create(img_data, img_error);
        /* In order to detect the full rectangle, f_lim has to be set to a low
         * value - if not, the central part is not detected */
        hdrl_parameter * params =
            hdrl_lacosmic_parameter_create(error * 2, 0.5, 65);
        result_mask = hdrl_lacosmic_edgedetect(image, params);
        hdrl_parameter_delete(params) ;

        /*
        cpl_image_save(data, "input_data.fits", CPL_TYPE_DOUBLE, NULL,
                       CPL_IO_CREATE);
        cpl_image_save(data_error, "input_error.fits", CPL_TYPE_DOUBLE, NULL,
                       CPL_IO_CREATE);
        cpl_mask_save(data_result, "output_mask.fits", NULL, CPL_IO_CREATE);
        */

        cpl_test_eq(cpl_mask_count(result_mask), 100*20+25*70);
        /* free the memory */
        cpl_image_delete(img_data);
        cpl_mask_delete(img_mask);
        cpl_image_delete(img_error);
        cpl_mask_delete(result_mask);
        hdrl_image_delete(image);
    }
    /* detect a very big rectangular cosmic with bad pixels */
    {
        img_data = cpl_image_new(150, 200, CPL_TYPE_DOUBLE);
        img_mask = cpl_mask_new(150, 200);
        cpl_image_fill_noise_uniform(img_data, 90, 110);
        double error = (110 - 90) / sqrt(12);

        for (int varx = 50; varx < 75; ++varx) {
            for (int vary = 60; vary < 130; ++vary) {
                cpl_image_set(img_data, varx, vary, 5000);
            }
        }
        for (int varx = 20; varx < 120; ++varx) {
            for (int vary = 20; vary < 40; ++vary) {
                cpl_image_set(img_data, varx, vary, 5000);
            }
        }

        /*mark bad pixels*/
        for (int varx = 65; varx < 68; ++varx) {
            for (int vary = 1; vary < 150; ++vary) {
                cpl_mask_set(img_mask, varx, vary, CPL_BINARY_1);
            }
        }
        img_error = cpl_image_new(cpl_image_get_size_x(img_data),
                                  cpl_image_get_size_y(img_data),
                                  CPL_TYPE_DOUBLE);
        cpl_image_add_scalar(img_error, error);
        cpl_image_reject_from_mask(img_data, img_mask);

        hdrl_image * image = hdrl_image_create(img_data, img_error);

        /* In order to detect the full rectangle, f_lim has to be set to a low
         * value - if not, the central part is not detected */
        hdrl_parameter * params =
            hdrl_lacosmic_parameter_create(error * 2, 0.5, 80);
        result_mask = hdrl_lacosmic_edgedetect(image, params);
        hdrl_parameter_delete(params) ;
        /*
        cpl_image_save(data, "input_data.fits", CPL_TYPE_DOUBLE, NULL,
                       CPL_IO_CREATE);
        cpl_image_save(data_error, "input_error.fits", CPL_TYPE_DOUBLE, NULL,
                       CPL_IO_CREATE);
        cpl_mask_save(data_bpm, "input_mask.fits", NULL, CPL_IO_CREATE);

        cpl_mask_save(data_result, "output_mask.fits", NULL, CPL_IO_CREATE);
         */

        cpl_test_eq(cpl_mask_count(result_mask), 100*20 + 25*70 - 3*70 - 3*20);
        /* free the memory */
        cpl_image_delete(img_data);
        cpl_mask_delete(img_mask);
        cpl_image_delete(img_error);
        cpl_mask_delete(result_mask);
        hdrl_image_delete(image);
    }
    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of Cosmic module
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    test_lacosmic_inputs();

    test_lacosmic_edgedetect();

    return cpl_test_end(0);
}
