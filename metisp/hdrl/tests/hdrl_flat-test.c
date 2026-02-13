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

#include "hdrl_flat.h"
#include "hdrl_image.h"
#include "hdrl_parameter.h"

#include <cpl.h>
#include <math.h>
#include <string.h>

/*----------------------------------------------------------------------------*/
/**
  @defgroup test input parameter robustness on flat functionality
 */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/**
  @brief Test flat functions robustness to different kind of input parameters

 */
/*----------------------------------------------------------------------------*/
void hdrl_flat_test_parlist(void)
{
    /* parameter parsing smoketest */
    hdrl_parameter * hpar;
    cpl_size filter_size_x=5;
    cpl_size filter_size_y=5;
    hdrl_flat_method method=HDRL_FLAT_FREQ_LOW;

    hdrl_parameter * deflts = hdrl_flat_parameter_create(filter_size_x,
                    filter_size_y, method);
    cpl_test(hdrl_flat_parameter_check(deflts));
    cpl_test_error(CPL_ERROR_NONE);

    cpl_parameterlist * pflat = hdrl_flat_parameter_create_parlist("RECIPE",
                    "flat", deflts);
    hdrl_parameter_delete(deflts) ;
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(cpl_parameterlist_get_size(pflat), 3);

    hpar = hdrl_flat_parameter_parse_parlist(pflat, "RECIPE.invalid");
    cpl_test_null(hpar);
    cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);

    hpar = hdrl_flat_parameter_parse_parlist(pflat, "RECIPE.flat");
    cpl_parameterlist_delete(pflat);
    cpl_test_error(CPL_ERROR_NONE);

    cpl_test_eq(hdrl_flat_parameter_get_method(hpar),
                HDRL_FLAT_FREQ_LOW);
    cpl_test_eq(hdrl_flat_parameter_get_filter_size_x(hpar), filter_size_x);
    cpl_test_eq(hdrl_flat_parameter_get_filter_size_y(hpar), filter_size_x);
    hdrl_parameter_delete(hpar);

    /* filter size x < 0 : CPL_ERROR_ILLEGAL_INPUT */
    /* filter size y < 0 : CPL_ERROR_ILLEGAL_INPUT */
    /* method: HDRL_FLAT_FREQ_LOW || HDRL_FLAT_FREQ_HIGH =>
     * 2==> CPL_ERROR_ILLEGAL_INPUT */
    deflts = hdrl_flat_parameter_create(-1, filter_size_y, method);
    cpl_test_null(deflts);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    hdrl_parameter_delete(deflts) ;

    deflts = hdrl_flat_parameter_create(filter_size_x, -1, method);
    cpl_test_null(deflts);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    hdrl_parameter_delete(deflts) ;

    deflts = hdrl_flat_parameter_create(filter_size_x, filter_size_y, 2);
    cpl_test_null(deflts);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    hdrl_parameter_delete(deflts) ;

    deflts = hdrl_flat_parameter_create(2, filter_size_y, method);
    cpl_test_null(deflts);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    hdrl_parameter_delete(deflts) ;

    deflts = hdrl_flat_parameter_create(filter_size_x, 2, method);
    cpl_test_null(deflts);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    hdrl_parameter_delete(deflts) ;

    deflts = hdrl_flat_parameter_create(2, 2, method);
    cpl_test_null(deflts);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    hdrl_parameter_delete(deflts) ;
}

/* not yet used, comment it out (may be will be used later)
static cpl_error_code
hdrl_flat_flag_pixel_set(cpl_image * data, cpl_mask * data_bpm, const int var )
{
    cpl_mask_set(data_bpm, 10, 10, CPL_BINARY_1);
    if(var == 0) {
        // Negative outlier set and marked as bad
        cpl_image_set(data,    10, 10, 20.);
        cpl_mask_set(data_bpm, 10, 10, CPL_BINARY_1);

        // Positive outlier set and marked as bad
        cpl_image_set(data,    50, 50, 300.);
        cpl_mask_set(data_bpm, 50, 50, CPL_BINARY_1);

        // Positive outliers set
        cpl_image_set(data,   60, 60, 300.);
        cpl_image_set(data,   61, 61, 300.);
        cpl_image_set(data,   62, 62, 300.);

        // Negative outliers set
        cpl_image_set(data,   70, 70, 20.);
        cpl_image_set(data,   71, 71, 20.);
        cpl_image_set(data,   72, 72, 20.);

        // Some pixels marked as bad
        cpl_mask_set(data_bpm, 80,  80, CPL_BINARY_1);
        cpl_mask_set(data_bpm, 81,  80, CPL_BINARY_1);
        cpl_mask_set(data_bpm, 82,  80, CPL_BINARY_1);

    } else if (var == 3) {
        cpl_image_set(data, 150, 150, 300.);
        cpl_image_set(data, 110, 260, 300.);
        cpl_mask_set(data_bpm, 70, 70, CPL_BINARY_1);
        cpl_mask_set(data_bpm, 80, 80, CPL_BINARY_1);
    }
    return cpl_error_get_code();

}
*/

/* the following function flag as bad data points over a given region */
static cpl_error_code
hdrl_flat_imlist_flag_region(hdrl_imagelist * imglist,
                             const hdrl_parameter * rect,
                             const double outlier,
                             const int mask_sw)
{
    const int nima=hdrl_imagelist_get_size(imglist);

    const cpl_size b_llx = hdrl_rect_region_get_llx(rect);
    const cpl_size b_lly = hdrl_rect_region_get_lly(rect);
    const cpl_size b_urx = hdrl_rect_region_get_urx(rect);
    const cpl_size b_ury = hdrl_rect_region_get_ury(rect);

    hdrl_image * hima = hdrl_imagelist_get(imglist , 0);

    cpl_size ima_sx = cpl_image_get_size_x(hdrl_image_get_image(hima));
    cpl_size ima_sy = cpl_image_get_size_y(hdrl_image_get_image(hima));

    for (int var = 0; var < nima ; ++var) {

        hima = hdrl_imagelist_get(imglist, var);
        cpl_image * data = hdrl_image_get_image(hima);
        cpl_mask * data_bpm = cpl_mask_new(ima_sx, ima_sy);
        // STDEV is about 10 for these images
        //cpl_image_fill_noise_uniform(data, 82, 118);

        //cpl_image_fill_window(data, b_llx, b_lly, b_urx, b_ury, outlier);
        for(int j = b_lly; j < b_ury; j++) {
            for(int i = b_llx; i < b_urx; i++) {
                cpl_image_set(data, i, j, outlier);
            }
        }

        if(mask_sw == 1) {
            for(int j = b_lly; j < b_ury; j++) {
                for(int i = b_llx; i < b_urx; i++) {
                    cpl_mask_set(data_bpm, i, j, CPL_BINARY_1);
                }
            }
        }
        /*
         hdrl_flat_flag_pixel_set(data, data_bpm, var);
         */
        cpl_image_reject_from_mask(data, data_bpm);

        cpl_image * errors=cpl_image_power_create(data, 0.5);
        hdrl_image * image = hdrl_image_create(data, errors);
        /*
        if(var == 0) {

            cpl_mask_save(cpl_image_get_bpm(data), "m_mask_flagged_region.fits",
                            NULL, CPL_IO_CREATE);
            cpl_mask_save(cpl_image_get_bpm(hdrl_image_get_image(image)),
                          "h_mask_flagged_region.fits", NULL, CPL_IO_CREATE);

            cpl_image_save(data, "data_flagged_region.fits",CPL_TYPE_FLOAT,
                            NULL, CPL_IO_DEFAULT);
            cpl_image_save(errors, "errs_flagged_region.fits",CPL_TYPE_FLOAT,
                           NULL, CPL_IO_DEFAULT);
            cpl_mask_save(data_bpm, "mask_flagged_region.fits", NULL,
                          CPL_IO_CREATE);

        }
        */
        hdrl_imagelist_set(imglist, image, var);
        // free the memory
        cpl_mask_delete(data_bpm);
        //cpl_image_delete(data);
        cpl_image_delete(errors);
    }


    cpl_imagelist* iml_data = NULL;
    cpl_imagelist* iml_errs = NULL;
    hdrl_imagelist_to_cplwrap(imglist, &iml_data, &iml_errs);
    /*
    cpl_imagelist_save(iml_data, "cube_data.fits", CPL_TYPE_FLOAT, NULL,
                       CPL_IO_DEFAULT);
    cpl_imagelist_save(iml_errs, "cube_errs.fits", CPL_TYPE_FLOAT, NULL,
                       CPL_IO_DEFAULT);
    */
    cpl_imagelist_unwrap(iml_data);
    cpl_imagelist_unwrap(iml_errs);

    return cpl_error_get_code();
}

/* the following function generates a list of uniform images of given size
 * and values  */
static hdrl_imagelist*
hdrl_flat_create_uniform_images(
                const cpl_size nima,
                const cpl_size ima_sx,
                const cpl_size ima_sy,
                const cpl_vector* values)
{
    hdrl_imagelist * imglist = hdrl_imagelist_new();

    for (int var = 0; var < nima ; ++var) {

        cpl_image * data = cpl_image_new(ima_sx, ima_sy, CPL_TYPE_DOUBLE);
        cpl_mask * data_bpm = cpl_mask_new(ima_sx, ima_sy);
        // STDEV is about 10 for these images
        //cpl_image_fill_noise_uniform(data, 82, 118);
        cpl_image_add_scalar(data, cpl_vector_get(values, var));
        cpl_image * errors = cpl_image_power_create(data, 0.5);
        /*
        if(var == 0) {
            cpl_image_save(data, "image_data.fits",CPL_TYPE_FLOAT, NULL,
                            CPL_IO_DEFAULT);
            cpl_image_save(errors, "image_errs.fits",CPL_TYPE_FLOAT, NULL,
                           CPL_IO_DEFAULT);
        }
        */
        hdrl_image * image = hdrl_image_create(data, errors);

        hdrl_imagelist_set(imglist, image, var);
        // free the memory
        cpl_mask_delete(data_bpm);
        cpl_image_delete(data);
        cpl_image_delete(errors);

    }

    cpl_imagelist* iml_data = NULL;
    cpl_imagelist* iml_errs = NULL;
    hdrl_imagelist_to_cplwrap(imglist, &iml_data, &iml_errs);
    /*
    cpl_imagelist_save(iml_data, "cube_data.fits", CPL_TYPE_FLOAT, NULL,
                       CPL_IO_DEFAULT);
    cpl_imagelist_save(iml_errs, "cube_errs.fits", CPL_TYPE_FLOAT, NULL,
                       CPL_IO_DEFAULT);
    */
    cpl_imagelist_unwrap(iml_data);
    cpl_imagelist_unwrap(iml_errs);
    return imglist;
}

static cpl_mask*
hdrl_flat_crea_static_mask(const cpl_size ima_sx,
                           const cpl_size ima_sy,
                           const hdrl_parameter * rect)
{
    const cpl_size b_llx = hdrl_rect_region_get_llx(rect);
    const cpl_size b_lly = hdrl_rect_region_get_lly(rect);
    const cpl_size b_urx = hdrl_rect_region_get_urx(rect);
    const cpl_size b_ury = hdrl_rect_region_get_ury(rect);

    cpl_mask * stat_mask = cpl_mask_new(ima_sx, ima_sy);
    for(int j = b_lly; j < b_ury; j++) {
        for(int i = b_llx; i < b_urx; i++) {
            cpl_mask_set(stat_mask, i, j, CPL_BINARY_1);
        }
    }
    return stat_mask;

}

/*
static cpl_error_code
hdrl_flat_save_results(hdrl_image* master, cpl_image* cmap, cpl_mask* stat_mask)
{

    cpl_image_save(hdrl_image_get_image(master), "master_data.fits",
                    CPL_TYPE_FLOAT, NULL, CPL_IO_DEFAULT);
    cpl_image_save(hdrl_image_get_error(master), "master_errs.fits",
                   CPL_TYPE_FLOAT, NULL, CPL_IO_DEFAULT);
    cpl_image_save(cmap, "master_map.fits", CPL_TYPE_FLOAT, NULL,
                   CPL_IO_DEFAULT);

    if (stat_mask != NULL) {
        cpl_mask_save(stat_mask, "static_mask.fits", NULL, CPL_IO_DEFAULT);
    }

    return cpl_error_get_code();
}
 */

/* this function is to verify that the image and error values in the
 * lower left corner and at a given data point are as expected
 */
static cpl_error_code
hdrl_flat_test_case(hdrl_imagelist* imglist,
                    const hdrl_parameter * rect,
                    const hdrl_flat_method method,
                    const cpl_size fsx,
                    const cpl_size fsy,
                    const hdrl_parameter* collapse_params,
                    const int mask_sw,
                    const double val1,
                    const double err1,
                    const double val2,
                    const double err2)

{
    hdrl_parameter  *   fparam ;
    cpl_mask  * stat_mask = NULL;
    hdrl_image * hima;
    cpl_size ima_sx;
    cpl_size ima_sy;

    const cpl_size b_llx = hdrl_rect_region_get_llx(rect);
    const cpl_size b_lly = hdrl_rect_region_get_lly(rect);
    const cpl_size b_urx = hdrl_rect_region_get_urx(rect);
    const cpl_size b_ury = hdrl_rect_region_get_ury(rect);

    cpl_size b_cx = (int)(0.5 * (b_llx + b_urx) + 0.5);
    cpl_size b_cy = (int)(0.5 * (b_lly + b_ury) + 0.5);
    hima = hdrl_imagelist_get(imglist,0);
    ima_sx = hdrl_image_get_size_x(hima);
    ima_sy = hdrl_image_get_size_y(hima);
    int bad;
    hdrl_image * master = NULL;
    cpl_image * contrib_map = NULL;

    fparam = hdrl_flat_parameter_create(fsx, fsy, method);

    if(mask_sw) {
        stat_mask=hdrl_flat_crea_static_mask(ima_sx, ima_sy, rect);
    }

    hdrl_flat_compute(imglist, stat_mask, collapse_params,
                      fparam, &master, &contrib_map);
    //hdrl_flat_save_results(master, contrib_map, stat_mask);

    cpl_test_eq(cpl_image_get(hdrl_image_get_image(master), 1, 1, &bad), val1);
    cpl_test_eq(cpl_image_get(hdrl_image_get_error(master), 1, 1, &bad), err1);

    cpl_test_eq(cpl_image_get(hdrl_image_get_image(master), b_cx, b_cy, &bad),
                val2);

    cpl_msg_warning(cpl_func,"Check for err2 = %g still to be implemented",
                    err2);

    /*
    cpl_msg_warning(cpl_func,"check sub data=%g %g",
                    cpl_image_get(hdrl_image_get_image(master), 1, 1, &bad), val1); //ok
    cpl_msg_warning(cpl_func,"check2 sub data=%g %g",
                    cpl_image_get(hdrl_image_get_image(master),
                    b_cx, b_cy, &bad),val2);//ok

    cpl_msg_warning(cpl_func,"check sub error=%g %g",
                    cpl_image_get(hdrl_image_get_error(master), 1, 1, &bad), err1);//nok
    cpl_msg_warning(cpl_func,"check2 sub error=%g %g",
                    cpl_image_get(hdrl_image_get_error(master),
                    b_cx, b_cy, &bad),err2);
    */

    /* Note: HDRL fully propagates errors. The error on the mean is not per
        * pixel but for uniform error image it is divided by sqrt(npix).
        * Thus one cannot compare the pixel values of the image with the error
        * computed on the image.
        */


    hdrl_parameter_delete(fparam);
    hdrl_image_delete(master);
    cpl_mask_delete(stat_mask);
    cpl_image_delete(contrib_map);

    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Check flat algorithm for various collapsing/smoothing conditions
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_flat_test_multi_options(void)
{
    /* input data */
    const cpl_size ima_sx = 200;
    const cpl_size ima_sy = 300;
    const cpl_size b_llx = 100;
    const cpl_size b_lly = 100;
    const cpl_size b_urx = 200;
    const cpl_size b_ury = 200;
    const cpl_size nima = 5;
    const double outlier = 100000;
    hdrl_parameter * rect_region_bad_area_def =
                    hdrl_rect_region_parameter_create(b_llx, b_lly, b_urx,
                                    b_ury);

    /* image intensity values distributed as 2^n */
    cpl_vector * vals = cpl_vector_new(nima);
    cpl_vector * valserr_rel = cpl_vector_new(nima); //relative error
    const double base = 2;

    for (int i = 0; i < nima ; ++i) {
    	double intensity = pow(base, i);
        cpl_vector_set(vals, i, intensity);
        cpl_vector_set(valserr_rel, i, sqrt(intensity)/intensity);
    }

    hdrl_imagelist * imglist = hdrl_flat_create_uniform_images(nima, ima_sx,
                    ima_sy, vals);
    hdrl_flat_imlist_flag_region(imglist, rect_region_bad_area_def, outlier, 0);

    /* flat parameters */
    cpl_size filter_size_x = 1;
    cpl_size filter_size_y = 1;
    const double r_median = 25000;
    const double r_mean = 38750;
    /* case 1:
     *
     * flat method=HDRL_FLAT_FREQ_LOW,
     * stat_mask=NULL,
     * colapse_method: median
     */

    /* Error propagation for pixel 1,1 */
    cpl_vector_power(valserr_rel, 2);

    double error_expected_pix1_mean   = sqrt(cpl_vector_get_sum(valserr_rel))/nima;
    double error_expected_pix1_median = error_expected_pix1_mean* sqrt(CPL_MATH_PI_2);


    hdrl_parameter *collapse_pMean   = hdrl_collapse_mean_parameter_create();
    hdrl_parameter *collapse_pMedian = hdrl_collapse_median_parameter_create();


    /* in the following cases (2-8) it is difficult to verify the error value at
     * the image centre because the image intensities are on purpose distributed
     * with complex values.
     */
    hdrl_flat_test_case(imglist, rect_region_bad_area_def, HDRL_FLAT_FREQ_LOW,
                        filter_size_x, filter_size_y, collapse_pMedian, 0,
                        1, error_expected_pix1_median, r_median, 91.4844);

    /* case 2:
     *
     * flat method=HDRL_FLAT_FREQ_LOW,
     * stat_mask!=NULL,
     * colapse_method: median
     */

    hdrl_flat_test_case(imglist, rect_region_bad_area_def, HDRL_FLAT_FREQ_LOW,
                        filter_size_x, filter_size_y, collapse_pMedian, 1,
                        1, error_expected_pix1_median, r_median, 91.4844);

    /* case 3:
     *
     * flat method=HDRL_FLAT_FREQ_LOW,
     * stat_mask=NULL,
     * colapse_method: mean
     */

    hdrl_flat_test_case(imglist, rect_region_bad_area_def, HDRL_FLAT_FREQ_LOW,
                        filter_size_x, filter_size_y, collapse_pMean, 0,
                        1, error_expected_pix1_mean, r_mean, 72.994);

    /* case 4:
     *
     * flat method=HDRL_FLAT_FREQ_LOW,
     * stat_mask!=NULL,
     * colapse_method: mean
     */

    hdrl_flat_test_case(imglist, rect_region_bad_area_def, HDRL_FLAT_FREQ_LOW,
                        filter_size_x, filter_size_y, collapse_pMean, 1,
                        1, error_expected_pix1_mean,r_mean,72.994);// error TBV

    /* case 5:
     *
     * flat method=HDRL_FLAT_FREQ_HIGH,
     * stat_mask=NULL,
     * colapse_method: median
     */

    hdrl_flat_test_case(imglist, rect_region_bad_area_def, HDRL_FLAT_FREQ_HIGH,
                        filter_size_x, filter_size_y, collapse_pMedian, 0,
                        1, error_expected_pix1_median, 1, 0.00177245);

    /* case 6:
     *
     * flat method=HDRL_FLAT_FREQ_HIGH,
     * stat_mask!=NULL,
     * colapse_method: median
     */

    hdrl_flat_test_case(imglist, rect_region_bad_area_def, HDRL_FLAT_FREQ_HIGH,
                        filter_size_x, filter_size_y, collapse_pMedian, 1,
                        1, error_expected_pix1_median, 1, 0.00177245);

    /* case 7:
     *
     * flat method=HDRL_FLAT_FREQ_HIGH,
     * stat_mask=NULL,
     * colapse_method: mean
     */

    hdrl_flat_test_case(imglist, rect_region_bad_area_def, HDRL_FLAT_FREQ_HIGH,
                        filter_size_x, filter_size_y, collapse_pMean, 0,
                        1, error_expected_pix1_mean, 1, 0.00141421);

    /* case 8:
     *
     * flat method=HDRL_FLAT_FREQ_HIGH,
     * stat_mask!=NULL,
     * colapse_method: mean
     */

    hdrl_flat_test_case(imglist, rect_region_bad_area_def, HDRL_FLAT_FREQ_HIGH,
                        filter_size_x, filter_size_y, collapse_pMean, 1,
                        1, error_expected_pix1_mean, 1, 0.00141421);

    /* free memory */
    hdrl_parameter_delete(collapse_pMean);
    hdrl_parameter_delete(collapse_pMedian);

    hdrl_parameter_delete(rect_region_bad_area_def);
    hdrl_imagelist_delete(imglist);
    cpl_vector_delete(vals);
    cpl_vector_delete(valserr_rel);
    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Check flat results in case of a static mask on three regions
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_flat_test_static_mask(void)
{
    /* input data */
    const cpl_size ima_sx = 200;
    const cpl_size ima_sy = 300;

    const cpl_size r1_llx = 50;
    const cpl_size r1_lly = 50;
    const cpl_size r1_urx = 80;
    const cpl_size r1_ury = 250;
    const double outlier1 = 100000;

    const cpl_size r2_llx = 100;
    const cpl_size r2_lly = 90;
    const cpl_size r2_urx = 130;
    const cpl_size r2_ury = 260;
    const double outlier2 = 200000;

    const cpl_size r3_llx = 150;
    const cpl_size r3_lly = 80;
    const cpl_size r3_urx = 180;
    const cpl_size r3_ury = 270;
    const double outlier3 = 300000;

    const cpl_size nima = 5;
    hdrl_parameter * rect1 = hdrl_rect_region_parameter_create(r1_llx, r1_lly,
                    r1_urx, r1_ury);

    hdrl_parameter * rect2 = hdrl_rect_region_parameter_create(r2_llx, r2_lly,
                    r2_urx, r2_ury);

    hdrl_parameter * rect3 = hdrl_rect_region_parameter_create(r3_llx, r3_lly,
                    r3_urx, r3_ury);

    /* image intensity values distributed as 2^n */
    cpl_vector * vals = cpl_vector_new(nima);

    const double base = 2;
    for (int i = 0; i < nima ; ++i) {
        double intensity = pow(base, i);
        cpl_vector_set(vals, i, intensity);
    }

    hdrl_imagelist* imglist = hdrl_flat_create_uniform_images(nima, ima_sx,
                    ima_sy, vals);
    hdrl_flat_imlist_flag_region(imglist, rect1, outlier1, 1);
    hdrl_flat_imlist_flag_region(imglist, rect2, outlier2, 1);
    hdrl_flat_imlist_flag_region(imglist, rect3, outlier3, 1);

    /* flat parameters */
    cpl_size filter_size_x = 1;
    cpl_size filter_size_y = 1;
    const double r_mean = 38750;
    const double e_mean = 72.994; //TBV
    //const double r_mean=25000;




    hdrl_parameter *collapse_pMean   = hdrl_collapse_mean_parameter_create();
    hdrl_parameter *collapse_pMedian = hdrl_collapse_median_parameter_create();

    /* case 1:
     *
     * flat method=HDRL_FLAT_FREQ_LOW,
     * stat_mask=NULL,
     * colapse_method: mean
     */

    hdrl_flat_test_case(imglist, rect1, HDRL_FLAT_FREQ_LOW,
                        filter_size_x, filter_size_y, collapse_pMean, 0,
                        1, 0.278388, r_mean, e_mean);




    /* case 2:
     *
     * flat method=HDRL_FLAT_FREQ_HIGH,
     * stat_mask=NULL,
     * colapse_method: mean
     */

    hdrl_flat_test_case(imglist, rect1, HDRL_FLAT_FREQ_HIGH,
                        filter_size_x, filter_size_y, collapse_pMean, 0,
                        1, 0.278388, 1, 0.00141421); // error TBV

    /* case 3:
     *
     * flat method=HDRL_FLAT_FREQ_HIGH,
     * stat_mask=NULL,
     * colapse_method: median
     */

    hdrl_flat_test_case(imglist, rect1, HDRL_FLAT_FREQ_HIGH,
                        filter_size_x, filter_size_y, collapse_pMedian, 0,
                        1, 0.278388, 1, 0.00141421); // error TBV


    /* free memory */
    hdrl_parameter_delete(collapse_pMean);
    hdrl_parameter_delete(collapse_pMedian);

    hdrl_parameter_delete(rect1);
    hdrl_parameter_delete(rect2);
    hdrl_parameter_delete(rect3);
    hdrl_imagelist_delete(imglist);
    cpl_vector_delete(vals);

    return cpl_error_get_code();
}


/*----------------------------------------------------------------------------*/
/**
  @brief Check flat results in case of a uniform input
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_flat_test_data_value_basic(void)
{
    /* input data */
    const cpl_size ima_sx = 51;
    const cpl_size ima_sy = 31;
    int npix = ima_sx * ima_sy;
    const cpl_size filter_size_x = 1;
    const cpl_size filter_size_y = 1;
    const cpl_size nima = 9;

    /* image intensity values distributed as 2^n */
    cpl_vector * vals = cpl_vector_new(nima);
    double value = 9;
    double error = sqrt(value);
    cpl_vector_fill(vals, value);

    hdrl_imagelist * imglist = hdrl_flat_create_uniform_images(nima, ima_sx,
                    ima_sy, vals);
    cpl_vector_delete(vals);

    hdrl_parameter * flat_params;
    hdrl_image * master = NULL;
    cpl_image * contrib_map = NULL;
    hdrl_value res;
    double expected_error;
    /* case 1:
     * 9 images each of value 9,
     * flat method=HDRL_FLAT_FREQ_LOW,
     * stat_mask=NULL,
     * colapse_method: mean
     *
     * expected results:
     * master should have value 1 (master is normalised),
     * error 3./9./math.sqrt(9) / math.sqrt(51*31)
     * contribution map should be NULL.
     */
    flat_params = hdrl_flat_parameter_create(filter_size_x, filter_size_y,
                    HDRL_FLAT_FREQ_LOW);

    hdrl_parameter * collapse_params = hdrl_collapse_mean_parameter_create();
    hdrl_flat_compute(imglist, NULL, collapse_params, flat_params, &master,
                      &contrib_map);
    hdrl_parameter_delete(flat_params);

    //hdrl_flat_save_results(master, contrib_map, NULL);

    expected_error = error / value / sqrt(nima) / sqrt(ima_sx * ima_sy);

    res = hdrl_image_get_mean(master);
    cpl_test_abs( res.data, 1, HDRL_EPS_ERROR);
    cpl_test_abs( res.error, expected_error, HDRL_EPS_ERROR);
    /* Note: HDRL fully propagates errors. The error on the mean is not per
     * pixel but for uniform error image it is divided by sqrt(npix).
     * Thus one cannot compare the pixel values of the image with the error
     * computed on the image.
     */

    res = hdrl_image_get_median(master);
    cpl_test_abs( res.data, 1, HDRL_EPS_ERROR);
    /* here error changes with respect to mean case by a
     * factor sqrt(CPL_MATH_PI_2), see HDRL documentation, section on
     * Statistical estimators
     */
    cpl_test_abs( res.error, expected_error * sqrt(CPL_MATH_PI_2),
                  HDRL_EPS_ERROR);

    /* TODO: why not stdev does not return hdrl_value (with stdev of error) */
    cpl_test_abs( hdrl_image_get_stdev(master), 0, HDRL_EPS_ERROR);

    res = hdrl_image_get_sum(master);
    cpl_test_abs( res.data, npix, HDRL_EPS_ERROR);
    /* Error associated to not normalised image is: sqrt (npix*err^2).
     * As the data are normalised error is sqrt (npix*err_norm^2)/ ndata
     * error = sqrt(npix) / ndata
     */
    cpl_test_abs( res.error, sqrt(npix) / nima, npix * HDRL_EPS_ERROR);

    cpl_test_abs( cpl_image_get_mean(contrib_map), nima, HDRL_EPS_ERROR );
    hdrl_image_delete(master);
    cpl_image_delete(contrib_map);


    /* case 2:
     * 9 images each of value 9,
     * flat method=HDRL_FLAT_FREQ_HIGH,
     * stat_mask=NULL,
     * colapse_method: mean
     *
     * expected results:
     * master should have value 1 (master is normalised),
     * error 3,
     * contribution map should be NULL.
     */
    flat_params = hdrl_flat_parameter_create(filter_size_x, filter_size_y,
                    HDRL_FLAT_FREQ_HIGH);

    hdrl_flat_compute(imglist, NULL, collapse_params, flat_params, &master,
                      &contrib_map);
    hdrl_parameter_delete(flat_params);

    //hdrl_flat_save_results(master, contrib_map, NULL);

    expected_error = error / value / sqrt(nima) / sqrt(ima_sx * ima_sy);
    res = hdrl_image_get_mean(master);
    cpl_test_abs( res.data, 1, HDRL_EPS_ERROR);
    cpl_test_abs( res.error, expected_error, HDRL_EPS_ERROR);

    res = hdrl_image_get_median(master);
    cpl_test_abs( res.data, 1, HDRL_EPS_ERROR);
    cpl_test_abs( res.error, expected_error * sqrt(CPL_MATH_PI_2),
                  HDRL_EPS_ERROR);

    cpl_test_abs( hdrl_image_get_stdev(master), 0, HDRL_EPS_ERROR);

    res = hdrl_image_get_sum(master);
    cpl_test_abs( res.data, npix, HDRL_EPS_ERROR);
    /* Error associated to not normalised image is: sqrt (npix*err^2).
     * As the data are normalised error is sqrt (npix*err_norm^2)/ ndata
     * error = sqrt(npix) / ndata
     */
    cpl_test_abs( res.error, sqrt(npix) / nima, npix * HDRL_EPS_ERROR);

    cpl_test_abs( cpl_image_get_mean(contrib_map), nima, HDRL_EPS_ERROR);
    hdrl_image_delete(master);
    cpl_image_delete(contrib_map);

    /* free memory */
    hdrl_imagelist_delete(imglist);
    hdrl_parameter_delete(collapse_params);

    return cpl_error_get_code();
}


/*----------------------------------------------------------------------------*/
/**
  @brief Check flat results in case of a uniform input and a static mask
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_flat_test_data_value_bpm(void)
{
    /* input data */
    const cpl_size ima_sx = 51;
    const cpl_size ima_sy = 31;

    const cpl_size filter_size_x = 1;
    const cpl_size filter_size_y = 1;

    const cpl_size nima = 9;

    /* image intensity values distributed as 2^n */
    cpl_vector * vals = cpl_vector_new(nima);
    double value = 9;
    double error = sqrt(value);
    cpl_vector_fill(vals, value);

    hdrl_imagelist * imglist = hdrl_flat_create_uniform_images(nima, ima_sx,
                    ima_sy, vals);
    cpl_vector_delete(vals);
    const cpl_size r1_llx = 11;
    const cpl_size r1_lly = 11;
    const cpl_size r1_urx = 31;
    const cpl_size r1_ury = 23;
    cpl_size xsam=0;
    cpl_size ysam=0;
    const double outlier1 = 10000;

    hdrl_parameter * rect1 =
                    hdrl_rect_region_parameter_create(r1_llx, r1_lly, r1_urx,
                                    r1_ury);
    hdrl_flat_imlist_flag_region(imglist, rect1, outlier1, 1);
    hdrl_parameter_delete(rect1);

    hdrl_parameter * flat_params;
    hdrl_image * master = NULL;
    cpl_image * contrib_map = NULL;
    hdrl_value res;
    double expected_error;
    double expected_value;
    /* case 1:
     * 9 images each of value 9,
     * flat method=HDRL_FLAT_FREQ_LOW,
     * stat_mask=NULL,
     * colapse_method: mean
     *
     * expected results:
     * master should have value 1 (master is normalised),
     * error 3./9./math.sqrt(9) / math.sqrt(51*31)
     * contribution map should be NULL.
     */
    flat_params = hdrl_flat_parameter_create(filter_size_x, filter_size_y,
                    HDRL_FLAT_FREQ_LOW);

    hdrl_parameter * collapse_params = hdrl_collapse_mean_parameter_create();
    hdrl_flat_compute(imglist, NULL, collapse_params, flat_params, &master,
                      &contrib_map);
    hdrl_parameter_delete(flat_params);
    //hdrl_flat_save_results(master, contrib_map, NULL);

    xsam = 0.5 * (r1_llx+r1_urx);
    ysam = 0.5 * (r1_lly+r1_ury);
    int status;
    expected_value = cpl_image_get(contrib_map, xsam, ysam, &status);
    res = hdrl_image_get_pixel(master, xsam, ysam, NULL);

    cpl_test( isnan(res.data));
    cpl_test( isnan(res.error));
    cpl_test_abs( expected_value, 0, HDRL_EPS_ERROR );

    /* 3./9./math.sqrt(9) */

    xsam = 0.5 * (ima_sx+r1_urx);
    ysam = 0.5 * (ima_sy+r1_ury);
    expected_error = error / value / sqrt(nima);
    expected_value = cpl_image_get(contrib_map, xsam, ysam, &status);

    res = hdrl_image_get_pixel(master, xsam, ysam, NULL);

    cpl_test_abs(res.data, 1, HDRL_EPS_ERROR);
    cpl_test_abs(res.error, expected_error, HDRL_EPS_ERROR);
    cpl_test_abs( expected_value, nima, HDRL_EPS_ERROR );

    hdrl_image_delete(master);
    cpl_image_delete(contrib_map);

    /* case 2:
     * 9 images each of value 9,
     * flat method=HDRL_FLAT_FREQ_HIGH,
     * stat_mask=NULL,
     * colapse_method: mean
     *
     * expected results:
     * master should have value 1 (master is normalised),
     * error 3,
     * contribution map should be NULL.
     */
    flat_params = hdrl_flat_parameter_create(filter_size_x, filter_size_y,
                    HDRL_FLAT_FREQ_HIGH);

    hdrl_flat_compute(imglist, NULL, collapse_params, flat_params, &master,
                      &contrib_map);
    hdrl_parameter_delete(flat_params);
    //hdrl_flat_save_results(master, contrib_map, NULL);

    xsam = 0.5 * (r1_llx+r1_urx);
    ysam = 0.5 * (r1_lly+r1_ury);

    expected_value = cpl_image_get(contrib_map, xsam, ysam, &status);
    res = hdrl_image_get_pixel(master, xsam, ysam, NULL);

    cpl_test( isnan(res.data));
    cpl_test( isnan(res.error));

    cpl_test_abs( expected_value, 0, HDRL_EPS_ERROR );
    /* 3./9./math.sqrt(9) */

    xsam = 0.5 * (ima_sx+r1_urx);
    ysam = 0.5 * (ima_sy+r1_ury);
    expected_error = error / value / sqrt(nima);
    expected_value = cpl_image_get(contrib_map, xsam, ysam, &status);

    res = hdrl_image_get_pixel(master, xsam, ysam, NULL);

    cpl_test_abs(res.data, 1, HDRL_EPS_ERROR);
    cpl_test_abs(res.error, expected_error, HDRL_EPS_ERROR);
    cpl_test_abs( expected_value, nima, HDRL_EPS_ERROR );


    hdrl_image_delete(master);
    cpl_image_delete(contrib_map);

    /* free memory */
    hdrl_imagelist_delete(imglist);
    hdrl_parameter_delete(collapse_params);

    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Check flat results in case of a uniform input and a static mask
  @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_flat_test_data_value_bpm_static(void)
{
    /* input data */
    const cpl_size ima_sx = 51;
    const cpl_size ima_sy = 31;

    const cpl_size filter_size_x = 1;
    const cpl_size filter_size_y = 1;
    const cpl_size nima = 9;

    /* image intensity values distributed as 2^n */
    cpl_vector * vals = cpl_vector_new(nima);
    double value = 9;
    double error = sqrt(value);
    cpl_vector_fill(vals, value);

    hdrl_imagelist * imglist = hdrl_flat_create_uniform_images(nima, ima_sx,
                    ima_sy, vals);
    cpl_vector_delete(vals);
    const cpl_size r1_llx = 11;
    const cpl_size r1_lly = 11;
    const cpl_size r1_urx = 31;
    const cpl_size r1_ury = 23;
    cpl_size xsam=0;
    cpl_size ysam=0;
    const double outlier1 = 10000;

    hdrl_parameter * rect1 =
                    hdrl_rect_region_parameter_create(r1_llx, r1_lly, r1_urx,
                                    r1_ury);
    hdrl_flat_imlist_flag_region(imglist, rect1, outlier1, 0);
    hdrl_parameter_delete(rect1);

    hdrl_parameter * flat_params;
    hdrl_image * master = NULL;
    cpl_image * contrib_map = NULL;
    hdrl_value res;
    double expected_error;
    double expected_value;
    /* case 1:
     * 9 images each of value 9,
     * flat method=HDRL_FLAT_FREQ_LOW,
     * stat_mask=NULL,
     * colapse_method: mean
     *
     * expected results:
     * master has value 1 (master is normalised) where points are not masked,
     * and outlier/nima=111.11 where points are masked
     * error 3./9./math.sqrt(9) / math.sqrt(51*31)
     * contribution map should be NULL.
     */
    flat_params = hdrl_flat_parameter_create(filter_size_x, filter_size_y,
                    HDRL_FLAT_FREQ_LOW);

    hdrl_parameter * collapse_params = hdrl_collapse_mean_parameter_create();

    /* creates and fill static mask */
    cpl_mask* static_bpm=cpl_mask_new(ima_sx,ima_sy);
    for(int j = r1_lly; j < r1_ury; j++) {
        for(int i = r1_llx; i < r1_urx; i++) {
            cpl_mask_set(static_bpm, i, j, CPL_BINARY_1);
        }
    }

    hdrl_flat_compute(imglist, static_bpm, collapse_params, flat_params,
                      &master, &contrib_map);
    hdrl_parameter_delete(flat_params);
    //hdrl_flat_save_results(master, contrib_map, NULL);

    xsam = 0.5 * (r1_llx+r1_urx);
    ysam = 0.5 * (r1_lly+r1_ury);
    int status;
    expected_value = cpl_image_get(contrib_map, xsam, ysam, &status);
    res = hdrl_image_get_pixel(master, xsam, ysam, NULL);
    //expected_error = error / value / sqrt(nima);
    expected_error = sqrt(outlier1) / value / sqrt(nima);

    cpl_test_abs( res.data, outlier1/nima, 13 * 21 * nima * HDRL_EPS_DATA);
    cpl_test_abs( res.error, expected_error, 3 * HDRL_EPS_ERROR);
    cpl_test_abs( expected_value, nima, HDRL_EPS_ERROR );


    /* 3./9./math.sqrt(9) */

    xsam = 0.5 * (ima_sx+r1_urx);
    ysam = 0.5 * (ima_sy+r1_ury);
    expected_error = error / value / sqrt(nima);
    expected_value = cpl_image_get(contrib_map, xsam, ysam, &status);

    res = hdrl_image_get_pixel(master, xsam, ysam, NULL);

    cpl_test_abs(res.data, 1, HDRL_EPS_ERROR);
    cpl_test_abs(res.error, expected_error, HDRL_EPS_ERROR);
    cpl_test_abs( expected_value, nima, HDRL_EPS_ERROR );

    hdrl_image_delete(master);
    cpl_image_delete(contrib_map);

    /* case 2:
     * 9 images each of value 9,
     * flat method=HDRL_FLAT_FREQ_HIGH,
     * stat_mask=NULL,
     * colapse_method: mean
     *
     * expected results:
     * master should have value 1 (master is normalised),
     * error 3,
     * contribution map should be NULL.
     */
    flat_params = hdrl_flat_parameter_create(filter_size_x, filter_size_y,
                    HDRL_FLAT_FREQ_HIGH);

    hdrl_flat_compute(imglist, static_bpm, collapse_params, flat_params,
                      &master, &contrib_map);
    hdrl_parameter_delete(flat_params);
    //hdrl_flat_save_results(master, contrib_map, NULL);

    xsam = 0.5 * (r1_llx+r1_urx);
    ysam = 0.5 * (r1_lly+r1_ury);

    expected_value = cpl_image_get(contrib_map, xsam, ysam, &status);
    res = hdrl_image_get_pixel(master, xsam, ysam, NULL);

    expected_error = sqrt(outlier1) / outlier1 / sqrt(nima);

    cpl_test_abs( res.data, 1, HDRL_EPS_ERROR);
    cpl_test_abs( res.error, expected_error, 3 * HDRL_EPS_ERROR);
    cpl_test_abs( expected_value, nima, HDRL_EPS_ERROR );



    /* 3./9./math.sqrt(9) */

    xsam = 0.5 * (ima_sx+r1_urx);
    ysam = 0.5 * (ima_sy+r1_ury);
    expected_error = error / value / sqrt(nima);
    expected_value = cpl_image_get(contrib_map, xsam, ysam, &status);

    res = hdrl_image_get_pixel(master, xsam, ysam, NULL);

    cpl_test_abs(res.data, 1, HDRL_EPS_ERROR);
    cpl_test_abs(res.error, expected_error, HDRL_EPS_ERROR);
    cpl_test_abs( expected_value, nima, HDRL_EPS_ERROR );

    hdrl_image_delete(master);
    cpl_image_delete(contrib_map);

    /* free memory */
    hdrl_imagelist_delete(imglist);
    cpl_mask_delete(static_bpm);
    hdrl_parameter_delete(collapse_params);

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

    hdrl_flat_test_data_value_bpm();
    hdrl_flat_test_data_value_bpm_static();
    hdrl_flat_test_data_value_basic();
    hdrl_flat_test_multi_options();
    hdrl_flat_test_static_mask();
    hdrl_flat_test_parlist();

    cpl_test_error(CPL_ERROR_NONE);
    return cpl_test_end(0);
}
