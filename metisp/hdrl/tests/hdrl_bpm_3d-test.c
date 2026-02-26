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

#include "hdrl_bpm_3d.h"
#include "hdrl_image.h"

#include <cpl.h>
#include <math.h>
#include <string.h>

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_bpm_3d_test    Testing of the HDRL bpm_3d
 */
/*----------------------------------------------------------------------------*/

void test_parlist(void)
{
    /* parameter parsing smoketest */
    hdrl_parameter * hpar;
    hdrl_parameter * deflts;

    deflts = hdrl_bpm_3d_parameter_create(4, 5, HDRL_BPM_3D_THRESHOLD_ERROR);
    cpl_parameterlist *pos = hdrl_bpm_3d_parameter_create_parlist(
                								"RECIPE", "bpm", deflts);
    hdrl_parameter *parse = hdrl_bpm_3d_parameter_parse_parlist(pos, "RECIPE.bpm");
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_nonnull(parse);
    hdrl_parameter_delete(parse);
    hdrl_parameter_delete(deflts);
    cpl_parameterlist_delete(pos);

    deflts = hdrl_bpm_3d_parameter_create(4, 5, HDRL_BPM_3D_THRESHOLD_RELATIVE);
    pos = hdrl_bpm_3d_parameter_create_parlist("RECIPE", "bpm", deflts);
    hdrl_parameter_delete(deflts) ;
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(cpl_parameterlist_get_size(pos), 3);

    hpar = hdrl_bpm_3d_parameter_parse_parlist(pos, "RECIPE.invalid");
    cpl_test_null(hpar);
    cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);

    hpar = hdrl_bpm_3d_parameter_parse_parlist(pos, "RECIPE.bpm");
    cpl_parameterlist_delete(pos);
    cpl_test_error(CPL_ERROR_NONE);

    cpl_test_eq(hdrl_bpm_3d_parameter_get_method(hpar),
                HDRL_BPM_3D_THRESHOLD_RELATIVE);
    cpl_test_eq(hdrl_bpm_3d_parameter_get_kappa_low(hpar), 4);
    cpl_test_eq(hdrl_bpm_3d_parameter_get_kappa_high(hpar), 5);
    hdrl_parameter_delete(hpar);
}

/*----------------------------------------------------------------------------*/
/**
  @brief Check hdrl_bpm_3d_compute() in various conditions
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_bpm_3d_test_compute(void)
{
    cpl_image       *   img_tmp = NULL;
    cpl_imagelist   *   result_data = NULL;
    hdrl_imagelist * imglist = hdrl_imagelist_new();
    int                 bad = 0;
    hdrl_parameter  *   bpm_param ;
    for (int var = 0; var <5 ; ++var) {
        cpl_image * data = cpl_image_new(200, 300, CPL_TYPE_DOUBLE);
        cpl_mask * data_bpm = cpl_mask_new(200, 300);
        /* STDEV is about 10 for these images */
        cpl_image_fill_noise_uniform(data, 82, 118);
        if(var == 0) {
            /* Negative outlier set and marked as bad */
            cpl_image_set(data,    10, 10, 20.);
            cpl_mask_set(data_bpm, 10, 10, CPL_BINARY_1);

            /* Positive outlier set and marked as bad */
            cpl_image_set(data,    50, 50, 300.);
            cpl_mask_set(data_bpm, 50, 50, CPL_BINARY_1);

            /* Positive outliers set */
            cpl_image_set(data,   60, 60, 300.);
            cpl_image_set(data,   61, 61, 300.);
            cpl_image_set(data,   62, 62, 300.);

            /* Negative outliers set */
            cpl_image_set(data,   70, 70, 20.);
            cpl_image_set(data,   71, 71, 20.);
            cpl_image_set(data,   72, 72, 20.);

            /*Some pixels marked as bad*/
            cpl_mask_set(data_bpm, 80,  80, CPL_BINARY_1);
            cpl_mask_set(data_bpm, 81,  80, CPL_BINARY_1);
            cpl_mask_set(data_bpm, 82,  80, CPL_BINARY_1);

        } else if (var == 3) {
            cpl_image_set(data, 150, 150, 300.);
            cpl_image_set(data, 110, 260, 300.);
            cpl_mask_set(data_bpm, 70, 70, CPL_BINARY_1);
            cpl_mask_set(data_bpm, 80, 80, CPL_BINARY_1);
        }

        cpl_image_reject_from_mask(data, data_bpm);
        cpl_image * errors=cpl_image_power_create(data, 0.5);
        hdrl_image * image = hdrl_image_create(data, errors);

        hdrl_imagelist_set(imglist, image, var);
        /* free the memory */
        cpl_mask_delete(data_bpm);
        cpl_image_delete(data);
        cpl_image_delete(errors);

    }
    //----------------------------------------------------------------------
    bpm_param = hdrl_bpm_3d_parameter_create(-50.0, 50.0,
            HDRL_BPM_3D_THRESHOLD_ABSOLUTE) ;
    cpl_test(hdrl_bpm_3d_parameter_check(bpm_param));

    result_data = hdrl_bpm_3d_compute(imglist, bpm_param);
    hdrl_parameter_delete(bpm_param) ;
    img_tmp = cpl_imagelist_get(result_data, 0);
    cpl_test_eq(cpl_image_get(img_tmp, 10, 10, &bad), 0);
    cpl_test_eq(cpl_image_get(img_tmp, 50, 50, &bad), 0);
    cpl_test_eq(cpl_image_get(img_tmp, 60, 60, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 61, 61, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 62, 62, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 70, 70, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 71, 71, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 72, 72, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 80, 80, &bad), 0);
    cpl_test_eq(cpl_image_get(img_tmp, 81, 80, &bad), 0);
    cpl_test_eq(cpl_image_get(img_tmp, 82, 80, &bad), 0);
    cpl_imagelist_delete(result_data);
    //----------------------------------------------------------------------

    //----------------------------------------------------------------------
    bpm_param = hdrl_bpm_3d_parameter_create(5., 5.,
            HDRL_BPM_3D_THRESHOLD_RELATIVE) ;
    result_data = hdrl_bpm_3d_compute(imglist, bpm_param);
    hdrl_parameter_delete(bpm_param) ;
    img_tmp = cpl_imagelist_get(result_data, 0);
    cpl_test_eq(cpl_image_get(img_tmp, 10, 10, &bad), 0);
    cpl_test_eq(cpl_image_get(img_tmp, 50, 50, &bad), 0);
    cpl_test_eq(cpl_image_get(img_tmp, 60, 60, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 61, 61, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 62, 62, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 70, 70, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 71, 71, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 72, 72, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 80, 80, &bad), 0);
    cpl_test_eq(cpl_image_get(img_tmp, 81, 80, &bad), 0);
    cpl_test_eq(cpl_image_get(img_tmp, 82, 80, &bad), 0);
    cpl_imagelist_delete(result_data);
    //----------------------------------------------------------------------

    //----------------------------------------------------------------------
    bpm_param = hdrl_bpm_3d_parameter_create(5., 5.,
            HDRL_BPM_3D_THRESHOLD_ERROR) ;
    result_data = hdrl_bpm_3d_compute(imglist, bpm_param);
    hdrl_parameter_delete(bpm_param) ;
    cpl_test_error(CPL_ERROR_NONE);

    img_tmp = cpl_imagelist_get(result_data, 0);

    cpl_test_eq(cpl_image_get(img_tmp, 10, 10, &bad), 0);
    cpl_test_eq(cpl_image_get(img_tmp, 50, 50, &bad), 0);
    cpl_test_eq(cpl_image_get(img_tmp, 60, 60, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 61, 61, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 62, 62, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 70, 70, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 71, 71, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 72, 72, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 80, 80, &bad), 0);
    cpl_test_eq(cpl_image_get(img_tmp, 81, 80, &bad), 0);
    cpl_test_eq(cpl_image_get(img_tmp, 82, 80, &bad), 0);
    /*cpl_imagelist_save(result_data, "result.fits", CPL_TYPE_INT, NULL,
      CPL_IO_CREATE);*/
    cpl_imagelist_delete(result_data);
    //----------------------------------------------------------------------

    //----------------------------------------------------------------------
    bpm_param = hdrl_bpm_3d_parameter_create(500., 5.,
            HDRL_BPM_3D_THRESHOLD_ERROR) ;
    result_data = hdrl_bpm_3d_compute(imglist, bpm_param);
    hdrl_parameter_delete(bpm_param) ;
    cpl_test_error(CPL_ERROR_NONE);

    img_tmp = cpl_imagelist_get(result_data, 0);

    cpl_test_eq(cpl_image_get(img_tmp, 10, 10, &bad), 0);
    cpl_test_eq(cpl_image_get(img_tmp, 50, 50, &bad), 0);
    cpl_test_eq(cpl_image_get(img_tmp, 60, 60, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 61, 61, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 62, 62, &bad), 1);
    cpl_test_eq(cpl_image_get(img_tmp, 70, 70, &bad), 0);
    cpl_test_eq(cpl_image_get(img_tmp, 71, 71, &bad), 0);
    cpl_test_eq(cpl_image_get(img_tmp, 72, 72, &bad), 0);
    cpl_test_eq(cpl_image_get(img_tmp, 80, 80, &bad), 0);
    cpl_test_eq(cpl_image_get(img_tmp, 81, 80, &bad), 0);
    cpl_test_eq(cpl_image_get(img_tmp, 82, 80, &bad), 0);
    cpl_imagelist_delete(result_data);
    //----------------------------------------------------------------------

    bpm_param = hdrl_bpm_3d_parameter_create(5., 5.,
            HDRL_BPM_3D_THRESHOLD_ERROR) ;
    result_data = hdrl_bpm_3d_compute(NULL, bpm_param);
    hdrl_parameter_delete(bpm_param) ;
    cpl_test_null(result_data);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    bpm_param = hdrl_bpm_3d_parameter_create(5.1, 5.,
            HDRL_BPM_3D_THRESHOLD_ABSOLUTE) ;
    result_data = hdrl_bpm_3d_compute(imglist, bpm_param);
    hdrl_parameter_delete(bpm_param) ;
    cpl_test_null(result_data);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    bpm_param = hdrl_bpm_3d_parameter_create(-5., 5.,
            HDRL_BPM_3D_THRESHOLD_RELATIVE) ;
    result_data = hdrl_bpm_3d_compute(imglist, bpm_param);
    hdrl_parameter_delete(bpm_param) ;
    cpl_test_null(result_data);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    bpm_param = hdrl_bpm_3d_parameter_create(-5., 5.,
            HDRL_BPM_3D_THRESHOLD_ERROR) ;
    result_data = hdrl_bpm_3d_compute(imglist, bpm_param);
    cpl_test_null(result_data);
    hdrl_parameter_delete(bpm_param) ;
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    bpm_param = hdrl_bpm_3d_parameter_create(5., -5.,
            HDRL_BPM_3D_THRESHOLD_ERROR) ;
    result_data = hdrl_bpm_3d_compute(imglist, bpm_param);
    hdrl_parameter_delete(bpm_param) ;
    cpl_test_null(result_data);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    hdrl_imagelist_delete(imglist);

    return cpl_error_get_code();
}


/*----------------------------------------------------------------------------*/
/**
  @brief   Unit tests of BPM module
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);
    hdrl_bpm_3d_test_compute();
    test_parlist();
    cpl_test_error(CPL_ERROR_NONE);
    return cpl_test_end(0);
}
