/* 
 * This file is part of the HDRL
 * Copyright (C) 2012,2013 European Southern Observatory
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
#include "hdrl_utils.h"
#include "hdrl_test.h"
#include "hdrl_overscan_defs.h"

#include "hdrl_collapse.h" //TMP
#include <cpl.h>

#include <math.h>
#include <stdlib.h>

/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/

typedef enum {
    HDRL_OSCAN_COLLAPSE_TEST_MEAN,
    HDRL_OSCAN_COLLAPSE_TEST_MEDIAN,
	HDRL_OSCAN_COLLAPSE_TEST_WEIGHTED_MEAN,
    HDRL_OSCAN_COLLAPSE_TEST_SIGCLIP,
    HDRL_OSCAN_COLLAPSE_TEST_MINMAX
} hdrl_oscan_collapse_test ;

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_overscan_test  Testing of the HDRL Overscan module
 */
/*----------------------------------------------------------------------------*/

void test_parlist(void)
{
    /* parameter parsing smoketest */
    hdrl_parameter * hpar;
    hdrl_parameter * rect_region_def =
        hdrl_rect_region_parameter_create(1, 1, 20, 20);
    hdrl_parameter * sigclip_def =
        hdrl_collapse_sigclip_parameter_create(3., 3., 5);
    hdrl_parameter * minmax_def =
        hdrl_collapse_minmax_parameter_create(2., 3.);
    hdrl_parameter * mode_def =
        hdrl_collapse_mode_parameter_create(10., 1., 0., HDRL_MODE_MEDIAN, 0);

    cpl_parameterlist * pos = hdrl_overscan_parameter_create_parlist(
                "RECIPE", "oscan", "alongX", 10, 10., rect_region_def,
                "MINMAX", sigclip_def, minmax_def, mode_def);
    hdrl_parameter_delete(mode_def);
    hdrl_parameter_delete(sigclip_def);
    hdrl_parameter_delete(minmax_def);
    hdrl_parameter_delete(rect_region_def);
    cpl_test_error(CPL_ERROR_NONE);
    /*
        Before adding the mode method:
        cpl_test_eq(cpl_parameterlist_get_size(pos), 13);
    */

    cpl_test_eq(cpl_parameterlist_get_size(pos), 13 + 5);

    hpar = hdrl_overscan_parameter_parse_parlist(pos, "RECIPE.invalid");
    cpl_test_null(hpar);
    cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);

    hpar = hdrl_overscan_parameter_parse_parlist(pos, "RECIPE.oscan");
    cpl_parameterlist_delete(pos);
    cpl_test_error(CPL_ERROR_NONE);
    {
        hdrl_parameter * rect = hdrl_overscan_parameter_get_rect_region(hpar);
        cpl_test_eq(hdrl_rect_region_get_llx(rect), 1);
        cpl_test_eq(hdrl_rect_region_get_lly(rect), 1);
        cpl_test_eq(hdrl_rect_region_get_urx(rect), 20);
        cpl_test_eq(hdrl_rect_region_get_ury(rect), 20);
    }
    cpl_test_eq(hdrl_overscan_parameter_get_box_hsize(hpar), 10);
    cpl_test_eq(hdrl_overscan_parameter_get_ccd_ron(hpar), 10.);
    cpl_test_eq(hdrl_overscan_parameter_get_correction_direction(hpar),
                HDRL_X_AXIS);
    {
        hdrl_parameter * col = hdrl_overscan_parameter_get_collapse(hpar);
        cpl_test(hdrl_collapse_parameter_is_minmax(col));
        cpl_test(!hdrl_collapse_parameter_is_mean(col));
        cpl_test_eq(hdrl_collapse_minmax_parameter_get_nlow(col), 2);
        cpl_test_eq(hdrl_collapse_minmax_parameter_get_nhigh(col), 3);
    }

    hdrl_parameter_destroy(hpar);
}

/*----------------------------------------------------------------------------*/
/**
  @brief Check hdrl_overscan_compute proper error on null input images
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_overscan_test_null_input(void)
{
    hdrl_parameter                  *   os_region;
    hdrl_parameter                  *   os_collapse;
    hdrl_parameter                  *   os_param;
    hdrl_overscan_compute_result    *   overscan_computation;

    /* Overscan Parameters */
    os_region = hdrl_rect_region_parameter_create(1, 1, 1, 1) ;
    os_collapse = hdrl_collapse_mean_parameter_create() ;
    os_param = hdrl_overscan_parameter_create(HDRL_Y_AXIS, 1., 1, 
            os_collapse, os_region) ;
    cpl_test(hdrl_overscan_parameter_check(os_param));

    /* test functionality */
    overscan_computation = hdrl_overscan_compute(NULL, os_param) ;
    hdrl_parameter_delete(os_region) ;
    hdrl_parameter_delete(os_collapse) ;
    hdrl_parameter_delete(os_param) ;

    cpl_test_null(overscan_computation);

    hdrl_overscan_compute_result_delete(overscan_computation);
    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Check hdrl_overscan_compute proper error on null input region
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_overscan_test_null_region(void)
{
    cpl_image                       *   image_data;
    hdrl_parameter                  *   os_region = NULL;
    hdrl_parameter                  *   os_collapse;
    hdrl_parameter                  *   os_param;
    hdrl_overscan_compute_result    *   overscan_computation;

    /* create input structures (on purpose not the input region) */
    image_data = cpl_image_new(1, 1, CPL_TYPE_DOUBLE);

    /* Overscan Parameters */
    os_collapse = hdrl_collapse_mean_parameter_create() ;
    os_param = hdrl_overscan_parameter_create(HDRL_Y_AXIS, 1., 1, 
            os_collapse, os_region) ;

    /* test functionality */
    overscan_computation = hdrl_overscan_compute(image_data, os_param);
    cpl_image_delete(image_data);
    hdrl_parameter_delete(os_collapse) ;
    hdrl_parameter_delete(os_param) ;
    cpl_test_null(overscan_computation);

    hdrl_overscan_compute_result_delete(overscan_computation);

    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Check hdrl_overscan_compute proper error on null input sigclip
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_overscan_test_null_sigclip(void)
{
    cpl_image                       *   image_data;
    hdrl_parameter                  *   os_region;
    hdrl_parameter                  *   os_collapse = NULL;
    hdrl_parameter                  *   os_param;
    hdrl_overscan_compute_result    *   overscan_computation;

    /* create input structures (on purpose not the input region) */
    image_data = cpl_image_new(1, 1, CPL_TYPE_DOUBLE);

    /* Overscan Parameters */
    os_region = hdrl_rect_region_parameter_create(1, 1, 1, 1) ;
    os_param = hdrl_overscan_parameter_create(HDRL_Y_AXIS, 1., 1, 
            os_collapse, os_region) ;

    /* test functionality */
    overscan_computation = hdrl_overscan_compute(image_data, os_param);
    cpl_image_delete(image_data);
    hdrl_parameter_delete(os_region) ;
    hdrl_parameter_delete(os_param) ;
    cpl_test_null(overscan_computation);

    hdrl_overscan_compute_result_delete(overscan_computation);

    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Check hdrl_overscan_compute proper error on null input compute control
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_overscan_test_null_params(void)
{
    cpl_image* image_data = cpl_image_new(1, 1, CPL_TYPE_DOUBLE);
    hdrl_overscan_compute_result * overscan_computation;

    /* test functionality */
    overscan_computation = hdrl_overscan_compute(image_data, NULL);
    cpl_test_null(overscan_computation);

    hdrl_overscan_compute_result_delete(overscan_computation);
    cpl_image_delete(image_data);

    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Check hdrl_overscan_compute proper error on wrong input region
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_overscan_test_wrong_region(void)
{
    hdrl_parameter                  *   os_region;
    hdrl_parameter                  *   os_collapse;
    hdrl_parameter                  *   os_param;
    cpl_image                       *   image_data;
    hdrl_overscan_compute_result    *   computation;

    /* create input image  */
    image_data = cpl_image_new(5, 10, CPL_TYPE_DOUBLE);

    /* Overscan Parameters */
    /* Initialize a region outside the bounds of the image */
    os_region = hdrl_rect_region_parameter_create(1, 1, 5, 10) ;
    os_collapse = hdrl_collapse_mean_parameter_create() ;
    os_param = hdrl_overscan_parameter_create(HDRL_Y_AXIS, 1., 1, 
            os_collapse, os_region) ;

    /* Run it and check */
    hdrl_rect_region_parameter_update(os_region, 0, 2, 4, 2) ;

    computation = hdrl_overscan_compute(image_data, os_param) ;
    cpl_test_null(computation);
    cpl_test_eq_error(cpl_error_get_code(), CPL_ERROR_ILLEGAL_INPUT);
        
    /* Initialize a region with size exceeding the X size of the image */
    hdrl_rect_region_parameter_update(os_region, 1, 5, 6, 10) ;

    // TODO yves fix and reenable
    //computation = hdrl_overscan_compute(image_data, os_param) ;
    //cpl_test_null(computation);
    //cpl_test_eq_error(cpl_error_get_code(), CPL_ERROR_ILLEGAL_INPUT);
    
    /* Destroy and return */
    hdrl_parameter_delete(os_region) ;
    hdrl_parameter_delete(os_collapse) ;
    hdrl_parameter_delete(os_param) ;
    cpl_image_delete(image_data);

    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Check hdrl_overscan_compute proper error on wrong input region
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static double hdrl_overscan_test_uniform_image(const double inp_value)
{
    double                              out_value = 0;
    cpl_image                       *   image_data;
    cpl_image                       *   image_errs;
    hdrl_image                      *   image;
    hdrl_overscan_compute_result    *   comp_res;
    hdrl_overscan_correct_result    *   overscan_correction;
    hdrl_parameter                  *   os_region;
    hdrl_parameter                  *   os_collapse;
    hdrl_parameter                  *   os_param;

    /* Create input structures (on purpose not the input region) */
    image_data = cpl_image_new(100, 100, HDRL_TYPE_DATA);
    cpl_image_add_scalar(image_data, inp_value);
    image_errs = cpl_image_new(100, 100, HDRL_TYPE_ERROR);
    cpl_image_add_scalar(image_errs, inp_value);
    cpl_image_power(image_errs, 0.5);
    image = hdrl_image_wrap(image_data, image_errs, NULL, CPL_FALSE);

    /* Overscan Parameters */
    os_region = hdrl_rect_region_parameter_create(1, 1, 100, 100) ;
    os_collapse = hdrl_collapse_median_parameter_create() ;
    os_param = hdrl_overscan_parameter_create(HDRL_Y_AXIS, 1., 5, 
            os_collapse, os_region) ;

    /* Compute the Overscan correction */
    comp_res = hdrl_overscan_compute(image_data, os_param);
    cpl_test_nonnull(comp_res);

    hdrl_parameter_delete(os_region) ;
    hdrl_parameter_delete(os_collapse) ;
    hdrl_parameter_delete(os_param) ;
    
    out_value = cpl_image_get_mean(
                           hdrl_image_get_image(comp_res->correction));
    cpl_test_abs(out_value, inp_value, HDRL_EPS_DATA);

    overscan_correction = hdrl_overscan_correct(image, NULL, comp_res);
    cpl_test_nonnull(overscan_correction);

    hdrl_overscan_compute_result_delete(comp_res);
    hdrl_overscan_correct_result_delete(overscan_correction);
    hdrl_image_delete(image);
    return out_value;
}

/*----------------------------------------------------------------------------*/
/**
  @brief Check overscan directions and shrinking window
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_overscan_test_dir(cpl_size Nx, cpl_size Ny, int hbox)
{
    const double                        error = 10;
    hdrl_image * image = hdrl_image_new(Nx, Ny);
    cpl_image * image_data = hdrl_image_get_image(image);
    cpl_image * image_errs = hdrl_image_get_error(image);
    hdrl_parameter                  *   os_region = NULL;
    hdrl_parameter                  *   os_collapse = NULL;
    hdrl_parameter                  *   os_param = NULL;
    hdrl_overscan_compute_result    *   res_os_comp = NULL;
    hdrl_overscan_compute_result    *   res_os_comp_turn = NULL;
    hdrl_overscan_correct_result    *   res_os_cor = NULL;
    hdrl_overscan_correct_result    *   res_os_cor_turn = NULL;
    int                                 rej;

    cpl_msg_info(cpl_func, "check mean hbox %d, Nx %ld, Ny %ld",
                 hbox, (long)Nx, (long)Ny);

    /* create image incrementing in x axis, constant in y axis */
    for (cpl_size x = 0; x < Nx; x++) {
        for (cpl_size y = 0; y < Ny; y++) {
            cpl_image_set(image_data, x + 1, y + 1, x);
            cpl_image_set(image_errs, x + 1, y + 1, x);
        }
    }
    cpl_image_power(image_errs, 0.5);

    /* Overscan Parameters */
    os_region = hdrl_rect_region_parameter_create(1, 1, Nx, Ny) ;
    os_collapse = hdrl_collapse_mean_parameter_create() ;
    os_param = hdrl_overscan_parameter_create(HDRL_Y_AXIS,
            error * sqrt(Ny * (1 + 2 * hbox)), hbox, os_collapse, os_region) ;

    /* test compute y direction */
    res_os_comp = hdrl_overscan_compute(image_data, os_param);
    cpl_test_error(CPL_ERROR_NONE);

    {
        /* in y direction overscan has same incremental pattern as input */
        cpl_image * ex = cpl_image_extract(image_data, 1, 1, Nx, 1);
        /* cpl mean implementation has a rather large error */
        cpl_test_image_abs(ex, hdrl_image_get_image(res_os_comp->correction),
                           2 * (1 + 2 * hbox) * Ny * HDRL_EPS_DATA);

        /* perfect match => zero chi2 = 0*/
        if (hbox == 0) {
            cpl_image_subtract(ex, ex);
            cpl_test_image_abs(ex, res_os_comp->red_chi2,
                               2 * (1 + 2 * hbox) * Ny * HDRL_EPS_DATA);
        }

        /* error constant in middle */
        cpl_image_subtract(ex, ex);
        cpl_image_add_scalar(ex, error);
        /* error larger on boundary */
        for (int i = 0; i < hbox; i++) {
            double cor = sqrt((1 + 2. * (hbox)) / (1 + 2 * i));
            cpl_image_set(ex, 1 + i, 1, error * cor);
            cpl_image_set(ex, Nx - i, 1, error * cor);
        }
        cpl_test_image_abs(ex, hdrl_image_get_error(res_os_comp->correction),
                           Ny * HDRL_EPS_ERROR);

        cpl_image_delete(ex);
    }

    /* test compute x direction */

    hdrl_parameter_delete(os_param) ;
    os_param = hdrl_overscan_parameter_create(HDRL_X_AXIS,
            error * sqrt(Nx * (1 + 2 * hbox)), hbox, os_collapse, os_region) ;
    res_os_comp_turn = hdrl_overscan_compute(image_data, os_param);

    hdrl_parameter_delete(os_region) ;
    hdrl_parameter_delete(os_collapse) ;
    hdrl_parameter_delete(os_param) ;

    cpl_test_error(CPL_ERROR_NONE);

    {
        /* in x direction overscan is the mean of the sum */
        cpl_image * ex = cpl_image_new(1, Ny, HDRL_TYPE_DATA);
        cpl_image_add_scalar(ex, (Nx - 1.) / 2.);
        cpl_test_image_abs(ex,
                           hdrl_image_get_image(res_os_comp_turn->correction),
                           2 * (1 + 2 * hbox) * Nx * HDRL_EPS_DATA);

        /* reduced chi2 so it should be constant with this pattern */
        cpl_image_subtract(ex, ex);
        cpl_image_add_scalar(ex, cpl_image_get(res_os_comp_turn->red_chi2,
                                                1, 1, &rej));
        cpl_test_image_abs(ex, res_os_comp_turn->red_chi2,
                           2 * (1 + 2 * hbox) * Nx * HDRL_EPS_DATA);

        /* error constant in middle */
        cpl_image_subtract(ex, ex);
        cpl_image_add_scalar(ex, error);
        /* error larger on boundary */
        for (int i = 0; i < hbox; i++) {
            double cor = sqrt((1 + 2. * (hbox)) / (1 + 2 * i));
            cpl_image_set(ex, 1, 1 + i, error * cor);
            cpl_image_set(ex, 1, Ny - i, error * cor);
        }
        cpl_test_image_abs(ex,
                           hdrl_image_get_error(res_os_comp_turn->correction),
                           3 * Nx * HDRL_EPS_ERROR);

        cpl_image_delete(ex);
    }


    /* test correct y direction */
    res_os_cor = hdrl_overscan_correct(image, NULL, res_os_comp);
    cpl_test_error(CPL_ERROR_NONE);

    {
        /* corrected y direction is just a zero image */
        cpl_image * ex = cpl_image_new(Nx, Ny, HDRL_TYPE_DATA);

        cpl_test_image_abs(ex, hdrl_image_get_image(res_os_cor->corrected),
                           2 * (1 + 2 * hbox) * Ny * HDRL_EPS_DATA);

        /* gaussian error */
        for (cpl_size y = 0; y < Ny; y++) {
            for (cpl_size x = 0; x < Nx; x++) {
                double val = cpl_image_get(image_errs, x + 1, y + 1, &rej);
                cpl_image_set(ex, x + 1, y + 1, hypot(error, val));
            }
            /* larger on boundaries */
            for (int i = 0; i < hbox; i++) {
                double cor = sqrt((1 + 2. * (hbox)) / (1 + 2 * i));
                double val = cpl_image_get(image_errs, 1 + i, y + 1, &rej);
                cpl_image_set(ex, 1 + i, y + 1, hypot(error * cor, val));
                val = cpl_image_get(image_errs, Nx - i, y + 1, &rej);
                cpl_image_set(ex, Nx - i, y + 1, hypot(error * cor, val));
            }
        }

        cpl_test_image_abs(ex, hdrl_image_get_error(res_os_cor->corrected),
                           Ny * HDRL_EPS_ERROR);

        cpl_image_delete(ex);
    }

    res_os_cor_turn = hdrl_overscan_correct(image, NULL, res_os_comp_turn);
    cpl_test_error(CPL_ERROR_NONE);

    {
        /* overscan in x direction constant over whole axis */
        cpl_image * ex = cpl_image_duplicate(image_data);
        cpl_image_subtract_scalar(ex,
                hdrl_image_get_pixel(res_os_comp_turn->correction, 1, 1,
                                     &rej).data);
        cpl_test_image_abs(ex, hdrl_image_get_image(res_os_cor_turn->corrected),
                2 * (1 + 2 * hbox) * Nx * HDRL_EPS_DATA);

        /* gaussian error */
        for (cpl_size x = 0; x < Nx; x++) {
            for (cpl_size y = 0; y < Ny; y++) {
                double val = cpl_image_get(image_errs, x + 1, y + 1, &rej);
                cpl_image_set(ex, x + 1, y + 1, hypot(error, val));
            }
            /* larger on boundaries */
            for (int i = 0; i < hbox; i++) {
                double cor = sqrt((1 + 2. * (hbox)) / (1 + 2 * i));
                double val = cpl_image_get(image_errs, 1 + x, 1 + i, &rej);
                cpl_image_set(ex, x + 1, 1 + i, hypot(error * cor, val));
                val = cpl_image_get(image_errs, x + 1, Ny - i, &rej);
                cpl_image_set(ex, x + 1, Ny - i, hypot(error * cor, val));
            }
        }

        cpl_test_image_abs(ex,
                           hdrl_image_get_error(res_os_cor_turn->corrected),
                           3 * Nx * HDRL_EPS_ERROR);

        cpl_image_delete(ex);
    }

    hdrl_overscan_compute_result_delete(res_os_comp);
    hdrl_overscan_compute_result_delete(res_os_comp_turn);
    hdrl_overscan_correct_result_delete(res_os_cor);
    hdrl_overscan_correct_result_delete(res_os_cor_turn);
    hdrl_image_delete(image);

    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Check single pixel overscan
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_overscan_test_full_hbox(
        cpl_size                    Nx, 
        cpl_size                    Ny)
{
    const double                        error = 10;
    hdrl_image * image = hdrl_image_new(Nx, Ny);
    cpl_image * image_data = hdrl_image_get_image(image);
    cpl_image * image_errs = hdrl_image_get_error(image);
    hdrl_parameter                  *   os_region = NULL;
    hdrl_parameter                  *   os_collapse = NULL;
    hdrl_parameter                  *   os_param = NULL;
    hdrl_overscan_compute_result    *   res_os_comp;
    hdrl_overscan_correct_result    *   res_os_cor = NULL;

    /* sets overscan to use full region and emit single pixel value */
    cpl_msg_info(cpl_func, "check mean full box, Nx %ld, Ny %ld",
                 (long)Nx, (long)Ny);

    /* create image incrementing in x axis, constant in y axis */
    for (cpl_size x = 0; x < Nx; x++) {
        for (cpl_size y = 0; y < Ny; y++) {
            cpl_image_set(image_data, x + 1, y + 1, x);
            cpl_image_set(image_errs, x + 1, y + 1, x);
        }
    }
    cpl_image_power(image_errs, 0.5);

    /* Overscan Parameters */
    os_region   = hdrl_rect_region_parameter_create(1, 1, Nx, Ny);
    os_collapse = hdrl_collapse_mean_parameter_create();

    /* invalid boxsize */
    os_param = hdrl_overscan_parameter_create(HDRL_Y_AXIS,
            error * sqrt(Nx * Ny), -2, os_collapse, os_region);

    hdrl_overscan_compute(image_data, os_param);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    /* test compute y direction */
    hdrl_parameter_delete(os_param);
    os_param = hdrl_overscan_parameter_create(HDRL_Y_AXIS,
											  error * sqrt(Nx * Ny),
											  HDRL_OVERSCAN_FULL_BOX,
											  os_collapse, os_region);
    res_os_comp = hdrl_overscan_compute(image_data, os_param);

    hdrl_parameter_delete(os_region);
    hdrl_parameter_delete(os_collapse);
    hdrl_parameter_delete(os_param);

    cpl_test_error(CPL_ERROR_NONE);

    {
        /* we expect a row with all the same overscan value */
        hdrl_image * ex = hdrl_image_new(Nx, 1);

        hdrl_image_add_scalar(ex, (hdrl_value){(Nx - 1.) / 2., error});

        /* cpl mean implementation has a rather large error */
        hdrl_test_image_abs(ex, res_os_comp->correction,
        						2 * Nx * Ny * HDRL_EPS_DATA);
        hdrl_test_image_abs(ex, res_os_comp->correction,
        						Nx * Ny * HDRL_EPS_DATA);

        hdrl_image_delete(ex);
    }

    hdrl_overscan_compute_result_delete(res_os_comp);
    hdrl_overscan_correct_result_delete(res_os_cor);

    hdrl_image_delete(image);

    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Check result of turned image when changing direction
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_overscan_test_turn_eq(
        cpl_size                    Nx, 
        cpl_size                    Ny, 
        int                         hbox,
        hdrl_oscan_collapse_test    method)
{
	cpl_ensure_code(method == HDRL_OSCAN_COLLAPSE_TEST_MEAN          ||
					method == HDRL_OSCAN_COLLAPSE_TEST_MEDIAN        ||
					method == HDRL_OSCAN_COLLAPSE_TEST_WEIGHTED_MEAN ||
					method == HDRL_OSCAN_COLLAPSE_TEST_SIGCLIP       ||
					method == HDRL_OSCAN_COLLAPSE_TEST_MINMAX,
				    CPL_ERROR_ILLEGAL_INPUT);

    hdrl_parameter *os_collapse = NULL;
    if (method == HDRL_OSCAN_COLLAPSE_TEST_MEAN) {
        cpl_msg_info(cpl_func, "Mean method hbox %d, Nx %ld, Ny %ld",
                     hbox, (long)Nx, (long)Ny);
        os_collapse = hdrl_collapse_mean_parameter_create();
    } else if (method == HDRL_OSCAN_COLLAPSE_TEST_MEDIAN) {
        cpl_msg_info(cpl_func, "Median method hbox %d, Nx %ld, Ny %ld",
                     hbox, (long)Nx, (long)Ny);
        os_collapse = hdrl_collapse_median_parameter_create();
    } else if (method == HDRL_OSCAN_COLLAPSE_TEST_WEIGHTED_MEAN) {
        cpl_msg_info(cpl_func, "Weighted Median method hbox %d, Nx %ld, Ny %ld",
                     hbox, (long)Nx, (long)Ny);
        os_collapse = hdrl_collapse_weighted_mean_parameter_create();
    } else if (method == HDRL_OSCAN_COLLAPSE_TEST_SIGCLIP) {
        cpl_msg_info(cpl_func, "Sigma-Clipping method hbox %d, Nx %ld, Ny %ld",
                     hbox, (long)Nx, (long)Ny);
        os_collapse = hdrl_collapse_sigclip_parameter_create(3., 3., 3) ;
    } else if (method == HDRL_OSCAN_COLLAPSE_TEST_MINMAX) {
        cpl_msg_info(cpl_func, "MinMax method hbox %d, Nx %ld, Ny %ld",
                     hbox, (long)Nx, (long)Ny);
        os_collapse = hdrl_collapse_minmax_parameter_create(3, 3);
    }


    const double error = 10;

    hdrl_image *image = hdrl_image_new(Nx, Ny);
    hdrl_image *image_t;

    cpl_image * image_data = hdrl_image_get_image(image);
    cpl_image * image_errs = hdrl_image_get_error(image);

    hdrl_parameter *os_region = NULL;
    hdrl_parameter *os_param  = NULL;

    hdrl_overscan_compute_result *res_os_comp      = NULL;
    hdrl_overscan_compute_result *res_os_comp_turn = NULL;
    hdrl_overscan_correct_result *res_os_cor       = NULL;
    hdrl_overscan_correct_result *res_os_cor_turn  = NULL;

    for (cpl_size x = 0; x < Nx; x++) {
        for (cpl_size y = 0; y < Ny; y++) {
            /* small range due to missing cpl_test_img_rel */
            double v = (50 - rand() % 100) / (double)((rand() % 50) + 1);
            cpl_image_set(image_data, x + 1, y + 1, v);
            cpl_image_set(image_errs, x + 1, y + 1, fabs(v));
        }
    }
    cpl_image_power(image_errs, 0.5);
    image_t = hdrl_image_duplicate(image);
    hdrl_image_turn(image_t, +1);
    


    /* Overscan Parameters */
    os_region = hdrl_rect_region_parameter_create(1, 1, Nx, Ny) ;
    os_param = hdrl_overscan_parameter_create(HDRL_Y_AXIS,
            error * sqrt(Ny * (1 + 2 * hbox)), hbox, os_collapse, os_region) ;

    /* test compute y direction */
    res_os_comp = hdrl_overscan_compute(image_data, os_param);
    cpl_test_error(CPL_ERROR_NONE);

    /* test compute x direction */
    hdrl_rect_region_parameter_update(os_region, 1, 1, Ny, Nx) ;
    hdrl_parameter_delete(os_param) ;
    os_param = hdrl_overscan_parameter_create(HDRL_X_AXIS,
            error * sqrt(Ny * (1 + 2 * hbox)), hbox, os_collapse, os_region) ;
    res_os_comp_turn = hdrl_overscan_compute(hdrl_image_get_image(image_t),
                                             os_param);
    cpl_test_error(CPL_ERROR_NONE);

    hdrl_parameter_delete(os_region) ;
    hdrl_parameter_delete(os_collapse) ;
    hdrl_parameter_delete(os_param) ;

    {
        hdrl_image * cor_t = hdrl_image_duplicate(res_os_comp_turn->correction);
        cpl_image * con_t = cpl_image_duplicate(res_os_comp_turn->contribution);
        cpl_image * chi_t = cpl_image_duplicate(res_os_comp_turn->red_chi2);
        hdrl_image_turn(cor_t, -1);
        cpl_image_turn(con_t, -1);
        cpl_image_turn(chi_t, -1);
        hdrl_test_image_abs(res_os_comp->correction,
                            cor_t, (1 + hbox) * Ny * HDRL_EPS_DATA);
        cpl_test_image_abs(res_os_comp->red_chi2, chi_t,
                           (1 + hbox) * Ny * HDRL_EPS_DATA);
        cpl_test_image_abs(res_os_comp->contribution, con_t, 0);
        hdrl_image_delete(cor_t);
        cpl_image_delete(con_t);
        cpl_image_delete(chi_t);
    }

    /* test correct y direction */
    res_os_cor = hdrl_overscan_correct(image, NULL, res_os_comp);
    cpl_test_error(CPL_ERROR_NONE);

    res_os_cor_turn = hdrl_overscan_correct(image_t, NULL, res_os_comp_turn);
    cpl_test_error(CPL_ERROR_NONE);

    hdrl_image_turn(res_os_cor_turn->corrected, -1);
    hdrl_test_image_abs(res_os_cor->corrected, res_os_cor_turn->corrected,
                       (1 + hbox) * Ny * HDRL_EPS_DATA);

	/* Get corrected */
	hdrl_image *imgCorrected1 = hdrl_overscan_correct_result_get_corrected(res_os_cor);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(imgCorrected1);
	hdrl_image *imgCorrected2 = hdrl_overscan_correct_result_unset_corrected(res_os_cor);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(imgCorrected2);
	hdrl_image_delete(imgCorrected2);

	/* Get badmask */
	cpl_image *cpImgBadMask_1 = hdrl_overscan_correct_result_get_badmask(res_os_cor);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(cpImgBadMask_1);
	cpl_image *cpImgBadMask_2 = hdrl_overscan_correct_result_unset_badmask(res_os_cor);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(cpImgBadMask_2);
	cpl_image_delete(cpImgBadMask_2);


    hdrl_overscan_compute_result_delete(res_os_comp);
    hdrl_overscan_compute_result_delete(res_os_comp_turn);
    hdrl_overscan_correct_result_delete(res_os_cor);
    hdrl_overscan_correct_result_delete(res_os_cor_turn);
    hdrl_image_delete(image);
    hdrl_image_delete(image_t);

    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Check case where a overscan box only contains bad pixels
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_overscan_test_empty_box(
        hdrl_oscan_collapse_test    method)
{

	cpl_ensure_code(method == HDRL_OSCAN_COLLAPSE_TEST_MEAN          ||
					method == HDRL_OSCAN_COLLAPSE_TEST_MEDIAN        ||
					method == HDRL_OSCAN_COLLAPSE_TEST_WEIGHTED_MEAN ||
					method == HDRL_OSCAN_COLLAPSE_TEST_SIGCLIP       ||
					method == HDRL_OSCAN_COLLAPSE_TEST_MINMAX,
					CPL_ERROR_ILLEGAL_INPUT);

    /* Overscan Parameters */
    hdrl_parameter *os_collapse = NULL;
    if(method == HDRL_OSCAN_COLLAPSE_TEST_MEAN) {
        cpl_msg_info(cpl_func, "check empty box MEAN");
        os_collapse = hdrl_collapse_mean_parameter_create();
    } else if (method == HDRL_OSCAN_COLLAPSE_TEST_MEDIAN) {
        cpl_msg_info(cpl_func, "check empty box MEDIAN");
        os_collapse = hdrl_collapse_median_parameter_create();
    } else if (method == HDRL_OSCAN_COLLAPSE_TEST_WEIGHTED_MEAN) {
        cpl_msg_info(cpl_func, "check empty box WEIGHTED_MEAN");
        os_collapse = hdrl_collapse_weighted_mean_parameter_create();
    } else if (method == HDRL_OSCAN_COLLAPSE_TEST_SIGCLIP) {
        cpl_msg_info(cpl_func, "check empty box SIGCLIP");
        os_collapse = hdrl_collapse_sigclip_parameter_create(3., 3., 3);
    } else if (method == HDRL_OSCAN_COLLAPSE_TEST_MINMAX) {
        cpl_msg_info(cpl_func, "check empty box MINMAX");
        os_collapse = hdrl_collapse_minmax_parameter_create(3, 3);
    }

    cpl_size Nx = 10;
    cpl_size Ny = 10;
    cpl_image *image_data = cpl_image_new(Nx, Ny, HDRL_TYPE_DATA);

    hdrl_parameter *os_region = hdrl_rect_region_parameter_create(1, 1, Nx, Ny);
    hdrl_parameter *os_param  = hdrl_overscan_parameter_create(
    								HDRL_Y_AXIS, 1., 0, os_collapse, os_region);

    /* reject one row (= one box */
    for (cpl_size i = 0; i < Ny; i++) cpl_image_reject(image_data, 2, i + 1);

    /* test compute y direction */
    hdrl_overscan_compute_result *res_os_comp;
    res_os_comp = hdrl_overscan_compute(image_data, os_param);
    cpl_test_error(CPL_ERROR_NONE);


    hdrl_parameter_delete(os_region);
    hdrl_parameter_delete(os_collapse);
    hdrl_parameter_delete(os_param);
    

	int rej;
	cpl_test_eq(cpl_image_get(res_os_comp->contribution, 2, 1, &rej), 0);
	cpl_test(hdrl_image_is_rejected(res_os_comp->correction, 2, 1));
	cpl_test(cpl_image_is_rejected(res_os_comp->red_chi2, 2, 1));

	/* Get correction */
	hdrl_image *img1 = hdrl_overscan_compute_result_get_correction(res_os_comp);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(img1);
	hdrl_image *img2 = hdrl_overscan_compute_result_unset_correction(res_os_comp);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(img2);
	hdrl_image_delete(img2);

	/* Get contribution */
	cpl_image *cpImg1 = hdrl_overscan_compute_result_get_contribution(res_os_comp);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(cpImg1);
	cpl_image *cpImg2 = hdrl_overscan_compute_result_unset_contribution(res_os_comp);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(cpImg2);
	cpl_image_delete(cpImg2);

	/* Get chi2 */
	cpl_image *cpImgCh2_1 = hdrl_overscan_compute_result_get_chi2(res_os_comp);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(cpImgCh2_1);
	cpl_image *cpImgCh2_2 = hdrl_overscan_compute_result_unset_chi2(res_os_comp);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(cpImgCh2_2);
	cpl_image_delete(cpImgCh2_2);

	/* Get red chi2 */
	cpl_image *cpImgRedCh2_1 = hdrl_overscan_compute_result_get_red_chi2(res_os_comp);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(cpImgRedCh2_1);
	cpl_image *cpImgRedCh2_2 = hdrl_overscan_compute_result_unset_red_chi2(res_os_comp);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(cpImgRedCh2_2);
	cpl_image_delete(cpImgRedCh2_2);


    /* Overscan Parameters - SigClip and MinMax*/

	if (method == HDRL_OSCAN_COLLAPSE_TEST_SIGCLIP) {

    	/* Get overscan SigClip Low */
    	cpl_image *cpImgSGLow_1 = hdrl_overscan_compute_result_get_sigclip_reject_low(res_os_comp);
    	cpl_test_error(CPL_ERROR_NONE);
    	cpl_test_nonnull(cpImgSGLow_1);
    	cpl_image *cpImgSGLow_2 = hdrl_overscan_compute_result_unset_sigclip_reject_low(res_os_comp);
    	cpl_test_error(CPL_ERROR_NONE);
    	cpl_test_nonnull(cpImgSGLow_2);
    	cpl_image_delete(cpImgSGLow_2);

    	/* Get overscan SigClip High */
    	cpl_image *cpImgSGHigh_1 = hdrl_overscan_compute_result_get_sigclip_reject_high(res_os_comp);
    	cpl_test_error(CPL_ERROR_NONE);
    	cpl_test_nonnull(cpImgSGHigh_1);
    	cpl_image *cpImgSGHigh_2 = hdrl_overscan_compute_result_unset_sigclip_reject_high(res_os_comp);
    	cpl_test_error(CPL_ERROR_NONE);
    	cpl_test_nonnull(cpImgSGHigh_2);
    	cpl_image_delete(cpImgSGHigh_2);

    } else if (method == HDRL_OSCAN_COLLAPSE_TEST_MINMAX) {

    	/* Get overscan MinMax Low */
    	cpl_image *cpImgMMLow_1 = hdrl_overscan_compute_result_get_minmax_reject_low(res_os_comp);
    	cpl_test_error(CPL_ERROR_NONE);
    	cpl_test_nonnull(cpImgMMLow_1);
    	cpl_image *cpImgMMLow_2 = hdrl_overscan_compute_result_unset_minmax_reject_low(res_os_comp);
    	cpl_test_error(CPL_ERROR_NONE);
    	cpl_test_nonnull(cpImgMMLow_2);
    	cpl_image_delete(cpImgMMLow_2);

    	/* Get overscan MinMax High */
    	cpl_image *cpImgMMHigh_1 = hdrl_overscan_compute_result_get_minmax_reject_high(res_os_comp);
    	cpl_test_error(CPL_ERROR_NONE);
    	cpl_test_nonnull(cpImgMMHigh_1);
    	cpl_image *cpImgMMHigh_2 = hdrl_overscan_compute_result_unset_minmax_reject_high(res_os_comp);
    	cpl_test_error(CPL_ERROR_NONE);
    	cpl_test_nonnull(cpImgMMHigh_2);
    	cpl_image_delete(cpImgMMHigh_2);
    }

    hdrl_overscan_compute_result_delete(res_os_comp);

    cpl_image_delete(image_data);


    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief   Unit tests of overscan functions module
 */
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);
    cpl_error_code code;
    double inp_value = 0;

    /* parameter parsing tests */
    test_parlist();

    /* Overscan with NULL input images */
    code = hdrl_overscan_test_null_input();
    cpl_test_eq_error(code, CPL_ERROR_NULL_INPUT);

    /* Overscan with NULL region */
    code = hdrl_overscan_test_null_region();
    cpl_test_eq_error(code, CPL_ERROR_ILLEGAL_INPUT);

    /* Overscan with NULL sigclip */
    code = hdrl_overscan_test_null_sigclip();
    cpl_test_eq_error(code, CPL_ERROR_ILLEGAL_INPUT);

    /* Overscan with NULL control */
    code = hdrl_overscan_test_null_params();
    cpl_test_eq_error(code, CPL_ERROR_NULL_INPUT);

    /* Overscan with wrong region */
    code = hdrl_overscan_test_wrong_region();
    cpl_test_eq_error(code, CPL_ERROR_NONE);

    /* Check expected results with uniform frame */
    hdrl_overscan_test_uniform_image(inp_value);
    
    cpl_size anx[] = {97,  45, 200};
    cpl_size any[] = {575, 34, 200};

    hdrl_overscan_test_turn_eq(0, 0, 0, -1);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    for (size_t i = 0; i < sizeof(anx) / sizeof(anx[0]); i++) {
        cpl_size nx = anx[i];
        cpl_size ny = any[i];

        for (int hbox = 0; hbox < 12; hbox++) {
            hdrl_overscan_test_dir(nx, ny, hbox);
        }

        for (int hbox = 0; hbox < 6; hbox+=2) {

            hdrl_overscan_test_turn_eq(nx, ny, hbox,
            					       HDRL_OSCAN_COLLAPSE_TEST_MEAN);
            cpl_test_error(CPL_ERROR_NONE);
            hdrl_overscan_test_turn_eq(nx, ny, hbox,
            		                   HDRL_OSCAN_COLLAPSE_TEST_MEDIAN);
            cpl_test_error(CPL_ERROR_NONE);
            hdrl_overscan_test_turn_eq(nx, ny, hbox,
            							HDRL_OSCAN_COLLAPSE_TEST_WEIGHTED_MEAN);
            cpl_test_error(CPL_ERROR_NONE);
            hdrl_overscan_test_turn_eq(nx, ny, hbox,
            		                   HDRL_OSCAN_COLLAPSE_TEST_SIGCLIP);
            cpl_test_error(CPL_ERROR_NONE);
            hdrl_overscan_test_turn_eq(nx, ny, hbox,
            		                   HDRL_OSCAN_COLLAPSE_TEST_MINMAX);
            cpl_test_error(CPL_ERROR_NONE);
        }

        hdrl_overscan_test_full_hbox(nx, ny);
    }

    hdrl_overscan_test_empty_box(-1);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    hdrl_overscan_test_empty_box(HDRL_OSCAN_COLLAPSE_TEST_MEAN);
    cpl_test_error(CPL_ERROR_NONE);
    hdrl_overscan_test_empty_box(HDRL_OSCAN_COLLAPSE_TEST_MEDIAN);
    cpl_test_error(CPL_ERROR_NONE);
    hdrl_overscan_test_empty_box(HDRL_OSCAN_COLLAPSE_TEST_WEIGHTED_MEAN);
    cpl_test_error(CPL_ERROR_NONE);
    hdrl_overscan_test_empty_box(HDRL_OSCAN_COLLAPSE_TEST_SIGCLIP);
    cpl_test_error(CPL_ERROR_NONE);
    hdrl_overscan_test_empty_box(HDRL_OSCAN_COLLAPSE_TEST_MINMAX);
    cpl_test_error(CPL_ERROR_NONE);



    /** \todo Test expected results with uniform frame + spikes */
    //cpl_test(hdrl_overscan_test_uniform_image_and_outliers(),value);

    /** \todo Test expected results with uniform frame + noise */
    //cpl_test(hdrl_overscan_test_uniform_image_and_noise(),value);

    return cpl_test_end(0);
}
