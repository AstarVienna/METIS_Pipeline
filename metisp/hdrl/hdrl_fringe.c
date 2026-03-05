/*
 * This file is part of the HDRL
 * Copyright (C) 2015 European Southern Observatory
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
 *
 * hdrl_fringe.c
 *
 *  Created on: May 11, 2015
 *      Author: agabasch
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <hdrl_fringe.h>
#include <hdrl_prototyping.h>
#include <math.h>




/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_fringe   Fringing
 *
 *  @brief
 *  This module contains functions to derive and subtract a master-fringe image
 *
 * \section Computation Master-fringe computation
 *
 * For the master-fringe estimation, the algorithm model the pixel intensity
 * distribution in a given image as a mixture of two Gaussian distributions,
 * whose means are the background and the fringe amplitudes, respectively:
 *
 * \image html gaussians_doxygen.png
 *
 * Thus the density function \f$f(x)\f$ of the
 * intensity of an individual pixel is modeled as follows
 * \f[
 * f(x) = c_1\,e^{ -\frac{(x-\mu_1)^2}{2\sigma_1^2} }
 * + c_2\,e^{ -\frac{(x-\mu_2)^2}{2\sigma_2^2} }.
 * \f]
 *
 * The means \f$\mu_1\f$ and \f$\mu_2\f$ ( \f$\mu_1 < \mu_2 = \mu_{1}+a\f$) are
 * proportional to the background amplitude and the fringe pattern
 * amplitude, respectively.  These values are used for normalization of
 * the background and fringe amplitudes before stacking.
 *
 * The parameters of the two Gaussian components are estimated from the
 * density function of the pixel intensities by a nonlinear least squares
 * fit algorithm.  The algorithm requires as its input an estimated
 * density function.  Such an estimate is calculated in a preprocessing
 * step as a truncated Hermite series:
 * \f[
 * f(x) \approx \sum_{n=0}^p \; c_n h_n\left(\frac{x - \mu}{\sigma}\right),
 * \f]
 * where \f$h_n\f$ is the normalized Hermite function
 * \f[
 *   h_n(x) = {\pi}^{-\frac14} \,
 *   2^{-\frac{n}{2}} (n!)^{-\frac12} \, (\-1)^n \, e^{\frac{x^2\!\!}{2}} \,
 *   \frac{d^n}{dx^n}\left( e^{-x^2} \right),
 * \f]
 * \f$\mu\f$ and \f$\sigma\f$ are, respectively, the sample mean and the
 * sample standard deviation of pixel intensities in the given image.
 * The truncation parameter \f$p\f$ is found experimentally, \f$p = 20\f$ has
 * been sufficient so far.   The Hermite coefficients \f$c_n\f$ are computed
 * as follows
 * \f[
 *   c_n = \frac1{\sigma N}\; \sum_{i=1}^N h_n\left(\frac{I_i - \mu}{\sigma}\right),
 * \f]
 * where the summation extends over all pixel intensities \f$I_1, \ldots,
 * I_{N}\f$, \f$N\f$ is the total number of pixels, and \f$n = 0, \ldots, p\f$.
 *
 * The following image shows a truncated Hermite series and its
 * approximation by a Gaussian mixture.
 *
 * \image html hermite.png
 *
 * \section Subtractoin Master-fringe subtraction
 *
 * For the master-fringe subtraction the algorithm computes fringe
 * amplitudes for each individual image by a least squares fit of a
 * linear combination of the estimated master-fringe and a constant background.
 * Specifically, the \f$i\f$th fringe \f$F_i\f$ is estimated as
 *
 * \f[
 * F_{i} = a_{i}F + b_{i},
 * \f]
 *
 * where \f$F\f$ is the estimated stacked master-fringe, and \f$b_i\f$ is a
 * constant representing the background.  The unknown constants \f$a_i\f$
 * and \f$b_i\f$ are computed by a standard least squares fit preformed
 * over the unmasked pixels.
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*---------------------------------------------------------------------------*/
/**
 * @brief    Calculates the master fringe and contribution map based on the
 * @brief    Gaussian mixture model.
 *
 * @param ilist_fringe    Image list from where to extract the fringes
 * @param ilist_obj       Masks with the objects of the single images (or NULL)
 * @param stat_mask       Static mask (or NULL)
 * @param collapse_params parameter controlling the collapse algorithm
 * @param master          returned master fringe map
 * @param contrib_map     returned contribution map of the master fringe map
 * @param qctable         returned table containing quality control information (or NULL)
 *
 * @return   the cpl error code in case of error or CPL_ERROR_NONE
 *
 * The function calculates the master fringe and contribution maps of a list of
 * (dithered) images. The background and fringe level are estimated as the mean
 * values of a Gaussian mixture model to the image histogram. The histogram is
 * approximated by a Hermite series before fitting the mixture model,
 * in order to avoid possible problems with bin sizes.
 *
 * The masks exclude the regions where the fringe is weak, and are essential
 * for an accurate estimation of noisy images.
 * The masks can be used to remove objects and bad regions from the
 * fit as well:
 * The algorithm combines the bad pixel map (from ilist_fringe),
 * the object mask (from ilist_obj), and static mask (stat_mask)
 * for the fringe computation itself, but uses only
 * the combined bad pixel map and object mask for the final collapsing. This
 * ensures that the master fringe is also calculated in regions excluded by the
 * static mask.
 *
 * @note Please note, that the function directly works on the passed hdrl
 * imagelist (ilist_fringe) in order to save memory thus modifying the imagelist
 *
 * @note Error propagation: Please note, that the scaling factor derived and
 * used in this function is considered to be noiseless, i.e. the associated
 * error is supposed to be zero
 *
 */
/*---------------------------------------------------------------------------*/
cpl_error_code
hdrl_fringe_compute(hdrl_imagelist* ilist_fringe, const cpl_imagelist * ilist_obj,
                    const cpl_mask* stat_mask, const hdrl_parameter* collapse_params,
                    hdrl_image** master, cpl_image** contrib_map,
                    cpl_table ** qctable)
{
    if (qctable) {
        *qctable = NULL;
    }
    /*Check that ilist_fringe and collapse_params are not pointer to NULL
     * and that there is at least one image in the hdrl imagelist */
    cpl_error_ensure(ilist_fringe && collapse_params , CPL_ERROR_NULL_INPUT,
                     goto cleanup, "NULL input imagelist or parameter");
    cpl_error_ensure(hdrl_imagelist_get_size(ilist_fringe) > 0 ,
                     CPL_ERROR_NULL_INPUT, goto cleanup,
                     "input imagelist is empty");

    cpl_size nx, ny, nx1, ny1;
    nx = hdrl_image_get_size_x(hdrl_imagelist_get_const(ilist_fringe, 0));
    ny = hdrl_image_get_size_y(hdrl_imagelist_get_const(ilist_fringe, 0));

    /* If there is a ilist_obj check the size and the dimensions: */
    if (ilist_obj != NULL) {
        cpl_error_ensure(hdrl_imagelist_get_size(ilist_fringe) ==
                         cpl_imagelist_get_size(ilist_obj),
                         CPL_ERROR_INCOMPATIBLE_INPUT, goto cleanup,
                         "size of fringe and object image list does "
                         "not match");

        nx1 = cpl_image_get_size_x(cpl_imagelist_get_const(ilist_obj, 0));
        ny1 = cpl_image_get_size_y(cpl_imagelist_get_const(ilist_obj, 0));
        cpl_error_ensure(nx == nx1, CPL_ERROR_INCOMPATIBLE_INPUT, goto cleanup,
                     "size of fringe image and object mask does not match");
        cpl_error_ensure(ny == ny1, CPL_ERROR_INCOMPATIBLE_INPUT, goto cleanup,
                     "size of fringe image and object mask does not match");
    }

    /* If there is a static mask check the dimensions: */
    if (stat_mask != NULL) {
        /* If there is a stat_mask check the dimensions: */
        cpl_error_ensure(nx == cpl_mask_get_size_x(stat_mask),
                        CPL_ERROR_INCOMPATIBLE_INPUT, goto cleanup,
                        "size of fringe image and fringe mask does not match");
        cpl_error_ensure(ny == cpl_mask_get_size_y(stat_mask),
                        CPL_ERROR_INCOMPATIBLE_INPUT, goto cleanup,
                        "size of fringe image and fringe mask does not match");
    }

/* This algorithm combines bpm (ilist_fringe), object list (from ilist_obj)
 * and static mask (stat_mask) for the fringe computation but uses only
 * the combined bpm and object list for the collapsing */

/* !! This algorithm directly works on the ilist_fringe object thus it
 * modifies ilist_fringe !! */

    double bkg_level = 0.;
    double fringe_level = 0.;

    cpl_size isize = hdrl_imagelist_get_size(ilist_fringe);
    cpl_msg_debug(cpl_func, "Measure fringe amplitudes");
    if (qctable != NULL) {
        *qctable = cpl_table_new(isize);
        cpl_table_new_column(*qctable, "Background_level", CPL_TYPE_DOUBLE);
        cpl_table_new_column(*qctable, "Fringe_amplitude", CPL_TYPE_DOUBLE);
    }
    for (cpl_size i = 0; i < isize; i++) {

        hdrl_image * this_himg = hdrl_imagelist_get(ilist_fringe, i);

        cpl_mask *this_fmsk = cpl_mask_duplicate(hdrl_image_get_mask(this_himg));
        if (ilist_obj != NULL) {
            const cpl_image * obj = cpl_imagelist_get_const(ilist_obj, i);
            cpl_mask * obj_mask = cpl_mask_threshold_image_create(obj,
                    -0.5, 0.5) ;
            cpl_mask_not(obj_mask) ;
            cpl_mask_or(this_fmsk, obj_mask);
            cpl_mask_delete(obj_mask) ;
        }

        /* Add the object mask to the bad pixel mask for the collapsing */
        hdrl_image_reject_from_mask(this_himg, this_fmsk);

        /* Add the static mask to the bad pixel mask and object mask for the
         * amplitude calculation */

        if (stat_mask != NULL) {
            cpl_mask_or(this_fmsk, stat_mask);
        }
        cpl_errorstate prestate = cpl_errorstate_get();
        cpl_matrix *cur_amplitudes = hdrl_mime_fringe_amplitudes(
                        hdrl_image_get_image_const(this_himg), this_fmsk);

        /* Handle situation where fit is not converging */
        if (!cpl_errorstate_is_equal(prestate)) {
            cpl_msg_warning(cpl_func, "Background level and fringe amplitude "
                            "could not be determined! Assuming a background"
                            " level of 0 and a fringe amplitude of 1");
            bkg_level = 0.;
            fringe_level = 1.;
            /* Reset error code */
            cpl_errorstate_set(prestate);
        }
        else {
            bkg_level = cpl_matrix_get(cur_amplitudes, 0, 0);
            fringe_level = cpl_matrix_get(cur_amplitudes, 1, 0);
        }


        double fringe_amplitude = fringe_level - bkg_level;
        if (qctable != NULL) {
            cpl_table_set_double(*qctable,"Background_level", i, bkg_level);
            cpl_table_set_double(*qctable,"Fringe_amplitude", i, fringe_amplitude);
        }
        cpl_msg_info(cpl_func, "img: %04d Bkg: %12.6g Amplitude: %12.6g",
                     (int)i+1, bkg_level, fringe_amplitude);

        cpl_msg_debug(cpl_func, "Rescaling image");

        hdrl_image_sub_scalar(this_himg, (hdrl_value){bkg_level, 0.});

        hdrl_image_div_scalar(this_himg, (hdrl_value){fringe_amplitude, 0.});

        cpl_matrix_delete(cur_amplitudes);
        cpl_mask_delete(this_fmsk);
    }

    cpl_msg_debug(cpl_func, "Combining the normalized fringes generating"
                 " the master-fringe");
    hdrl_imagelist_collapse(ilist_fringe, collapse_params, master,
                            contrib_map);

cleanup:
    if (cpl_error_get_code() != CPL_ERROR_NONE) {
        if (qctable) {
            cpl_table_delete(*qctable);
            *qctable = NULL;
        }
        if (master) {
            *master = NULL;
        }
        if (contrib_map) {
            *contrib_map = NULL;
        }
    }

    return cpl_error_get_code();

}

/*---------------------------------------------------------------------------*/
/**
 * @brief    Scales and subtracts the master fringe from the images.
 *
 * @param ilist_fringe     Image list from where to subtract the master fringe
 * @param ilist_obj        Masks with the objects of the single images (or NULL)
 * @param stat_mask        Static mask (or NULL)
 * @param masterfringe     master fringe to scale and subtract
 * @param qctable          table containing quality control information (or NULL)
 *
 * @return   the cpl error code in case of error or CPL_ERROR_NONE
 *
 * The function subtracts a fringe correction image (master) from a set of
 * input images (ilist_fringe). The amplitude of the fringes is computed
 * for each input image and used to properly rescale the correction image
 * before subtraction.
 *
 * The masks exclude the regions where the fringe is weak, and are essential
 * for an accurate scaling estimation of noisy images.
 * The algorithm combines the bad pixel map (from ilist_fringe),
 * the object mask (from ilist_obj), and static mask (stat_mask)
 * for the scaling computation of the master fringe, but only uses the bad pixel
 * map when subtracting the master-fringe. The object mask and static mask
 * are ignored in this step. This ensures that the master fringe is properly
 * subtracted (with error propagation) in all regions not affected by the bad
 * pixel mask.
 *
 * @note Please note, that the function directly works on the passed hdrl
 * imagelist (ilist_fringe) in order to save memory thus modifying the imagelist
 * i.e. removing the fringes directly from the original imagelist (ilist_fringe)
 *
 * @note Error propagation: Please note, that the scaling factor derived and
 * used in this function is considered to be noiseless, i.e. the associated
 * error is supposed to be zero
 *
 */
/*---------------------------------------------------------------------------*/
cpl_error_code
hdrl_fringe_correct(hdrl_imagelist * ilist_fringe, const cpl_imagelist * ilist_obj,
                    const cpl_mask * stat_mask, const hdrl_image * masterfringe,
                    cpl_table ** qctable)
{
    if (qctable) {
        *qctable = NULL;
    }
    /*Check that ilist_fringe and masterfringe are not NULL pointers
     * and that there is at least one image in the hdrl imagelist */
    cpl_ensure_code(ilist_fringe && masterfringe, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(hdrl_imagelist_get_size(ilist_fringe) > 0 ,
                    CPL_ERROR_NULL_INPUT);

    cpl_size nx, ny, nx1, ny1;
    nx = hdrl_image_get_size_x(hdrl_imagelist_get_const(ilist_fringe, 0));
    ny = hdrl_image_get_size_y(hdrl_imagelist_get_const(ilist_fringe, 0));

    /*Check the dimension of the masterfringe image*/
    nx1 = hdrl_image_get_size_x(masterfringe);
    ny1 = hdrl_image_get_size_y(masterfringe);
    cpl_ensure_code(nx == nx1, CPL_ERROR_INCOMPATIBLE_INPUT );
    cpl_ensure_code(ny == ny1, CPL_ERROR_INCOMPATIBLE_INPUT );


    /* If there is a ilist_obj check the size and the dimensions: */
    if (ilist_obj != NULL) {
        cpl_ensure_code(hdrl_imagelist_get_size(ilist_fringe) ==
                        cpl_imagelist_get_size(ilist_obj),
                        CPL_ERROR_INCOMPATIBLE_INPUT);

        nx1 = cpl_image_get_size_x(cpl_imagelist_get_const(ilist_obj, 0));
        ny1 = cpl_image_get_size_y(cpl_imagelist_get_const(ilist_obj, 0));
        cpl_ensure_code(nx == nx1, CPL_ERROR_INCOMPATIBLE_INPUT );
        cpl_ensure_code(ny == ny1, CPL_ERROR_INCOMPATIBLE_INPUT );
    }

    /* If there is a static mask check the dimensions: */
    if (stat_mask != NULL) {
        /* If there is a stat_mask check the dimensions: */
        cpl_ensure_code(nx == cpl_mask_get_size_x(stat_mask),
                        CPL_ERROR_INCOMPATIBLE_INPUT );
        cpl_ensure_code(ny == cpl_mask_get_size_y(stat_mask),
                        CPL_ERROR_INCOMPATIBLE_INPUT );
    }
    /* !! This algorithm directly works on the ilist_fringe object thus it
     * modifies ilist_fringe !! */

    /* Do the actual fringemap correction */
    double bkg_level = 0.;
    double fringe_level = 0.;

    cpl_size isize = hdrl_imagelist_get_size(ilist_fringe);
    cpl_msg_debug(cpl_func, "Measure fringe amplitudes");

    if (qctable != NULL) {
        *qctable = cpl_table_new(isize);
        cpl_table_new_column(*qctable, "Background_level", CPL_TYPE_DOUBLE);
        cpl_table_new_column(*qctable, "Fringe_amplitude", CPL_TYPE_DOUBLE);
    }
    for (cpl_size i = 0; i < isize; i++) {

        hdrl_image * this_himg = hdrl_imagelist_get(ilist_fringe, i);
        hdrl_image * this_masterfringe = hdrl_image_duplicate(masterfringe);

        cpl_mask *this_fmsk = cpl_mask_duplicate(hdrl_image_get_mask(this_himg));
        if (stat_mask != NULL) {
            cpl_mask_or(this_fmsk, stat_mask);
        }

        if (ilist_obj != NULL) {
            const cpl_image * obj = cpl_imagelist_get_const(ilist_obj, i);
            cpl_mask * obj_mask = cpl_mask_threshold_image_create(obj,
                    -0.5, 0.5) ;
            cpl_mask_not(obj_mask) ;
            cpl_mask_or(this_fmsk, obj_mask);
            cpl_mask_delete(obj_mask) ;
        }

        cpl_errorstate prestate = cpl_errorstate_get();

        cpl_matrix * cur_amplitudes = hdrl_mime_fringe_amplitudes_ls(
                            hdrl_image_get_image_const(this_himg), this_fmsk,
                            hdrl_image_get_image_const(this_masterfringe));

        /* Handle situation where fit is not converging */
        if (!cpl_errorstate_is_equal(prestate)) {
            cpl_msg_warning(cpl_func, "Background level and fringe amplitude "
                            "could not be determined! Assuming a background"
                            " level of 0 and a fringe amplitude of 0, i.e. "
                            "no correction will be applied to this image");
            bkg_level = 0.;
            fringe_level = 0.;
            /* Reset error code */
            cpl_errorstate_set(prestate);
        }
        else {
            bkg_level = cpl_matrix_get(cur_amplitudes, 0, 0);
            fringe_level = cpl_matrix_get(cur_amplitudes, 1, 0);
        }

        double fringe_amplitude = fringe_level - bkg_level;

        if (qctable != NULL) {
            cpl_table_set_double(*qctable,"Background_level", i, bkg_level);
            cpl_table_set_double(*qctable,"Fringe_amplitude", i, fringe_amplitude);
        }
        cpl_msg_info(cpl_func, "img: %04d Bkg: %12.6g Amplitude: %12.6g",
                     (int)i+1, bkg_level, fringe_amplitude);

        cpl_msg_debug(cpl_func, "Rescaling masterfringe");
        hdrl_image_mul_scalar(this_masterfringe, (hdrl_value){fringe_amplitude, 0.});

        cpl_msg_debug(cpl_func, "Subtract rescaled masterfringe");
        hdrl_image_sub_image(this_himg, this_masterfringe);

        hdrl_image_delete(this_masterfringe);
        cpl_matrix_delete(cur_amplitudes);
        cpl_mask_delete(this_fmsk);
    }

    if (cpl_error_get_code() != CPL_ERROR_NONE && qctable != NULL) {
        cpl_table_delete(*qctable);
        *qctable = NULL;
    }

    return cpl_error_get_code();

}

/** @cond PRIVATE */
/*---------------------------------------------------------------------------*/
/**
 * @brief    Estimate background and fringe levels in an image from a
 * @brief    Gaussian mixture model.
 *
 * @param    img0    Image,
 * @param    mask0   Mask,
 *
 * @return   A 2-by-1 matrix with the bkg and fringe amplitudes.
 *
 * Background and fringe level are estimated as the mean values of a
 * Gaussian mixture model to the image histogram. The histogram is
 * approximated by a Hermite series before fitting the mixture model,
 * in order to avoid possible problems with bin sizes.
 *
 * The mask exclude the regions where the fringe is weak, and is essential
 * for an accurate estimation of noisy images.
 * The mask can be used to remove objects and bad regions from the
 * fit.
 *
 * The returned matrix must be deallocated using cpl_matrix_delete().
 */
/*---------------------------------------------------------------------------*/
cpl_matrix *hdrl_mime_fringe_amplitudes(const cpl_image * img0,
          const cpl_mask * mask0)
{
    cpl_matrix *mdata;
    cpl_matrix *coeffs;
    cpl_matrix *x;
    cpl_matrix *hseries;
    cpl_matrix *amplitudes;
    cpl_vector *vals;
    cpl_vector *params;
    const double *img_data;
    const cpl_binary *mask_data;
    double   *md;
    double   *par;
    double    mean, stdev, bkg_amp, fringe_amp;
    int       nx, ny, n, size, ns;
    int       msize, i;

    cpl_ensure(img0 != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(mask0 != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(cpl_image_get_type(img0) == CPL_TYPE_DOUBLE,
               CPL_ERROR_INVALID_TYPE, NULL);

/* setting parameters
   n       number of the Hermite functions
 */
    n = 20;

/* getting image parameters and stats
   nx, ny          dimensions of images
 */
    nx = cpl_image_get_size_x(img0);
    ny = cpl_image_get_size_y(img0);
    size = nx * ny;
    msize = size - cpl_mask_count(mask0);
    /* check that at least some region have been flagged */
    cpl_ensure(msize > 0 , CPL_ERROR_ILLEGAL_INPUT,NULL);

/* creating masked image */
    mdata = cpl_matrix_new(msize, 1);
    md = cpl_matrix_get_data(mdata);

    img_data = cpl_image_get_data_double_const(img0);
    mask_data = cpl_mask_get_data_const(mask0);
    for (i = 0; i < size; i++, mask_data++, img_data++)
    {
        if (*mask_data == CPL_BINARY_0)
        {
            *md = (double) *img_data;
            md++;
        }
    }
    mean = cpl_matrix_get_mean(mdata);
    stdev = cpl_matrix_get_stdev(mdata);

/* computing the Hermite coefficients */
    coeffs = hdrl_mime_hermite_functions_sums_create(n, mean, stdev, mdata);
    cpl_matrix_multiply_scalar(coeffs, 1.0 / msize);

/* reconstructing the density function as the Hermite series */
    ns = 1000;
    x = hdrl_mime_matrix_linspace_create(ns, mean - 4.0 * stdev,
              mean + 4.0 * stdev);
    hseries = hdrl_mime_hermite_series_create(n, mean, stdev, coeffs, x);

/* computing the parameters of the Gaussian mixture */

    params = cpl_vector_new(6);
    par = cpl_vector_get_data(params);

    par[0] = 0.62 / (sqrt(CPL_MATH_PI) * stdev);
    par[1] = mean - 0.4 * stdev;
    par[2] = 0.58 * stdev;

    par[3] = 0.57 / (sqrt(CPL_MATH_PI) * stdev);
    par[4] = mean + 0.3 * stdev;
    par[5] = 0.61 * stdev;

    vals = cpl_vector_wrap(ns, cpl_matrix_get_data(hseries));
    cpl_fit_lvmq(x, NULL, vals, NULL, params, NULL, hdrl_mime_gmix1,
              hdrl_mime_gmix_derivs1, CPL_FIT_LVMQ_TOLERANCE, CPL_FIT_LVMQ_COUNT,
              CPL_FIT_LVMQ_MAXITER, NULL, NULL, NULL);

    bkg_amp = (par[1] > par[4] ? par[4] : par[1]);
    fringe_amp = (par[1] > par[4] ? par[1] : par[4]);
    amplitudes = cpl_matrix_new(2, 1);
    cpl_matrix_set(amplitudes, 0, 0, bkg_amp);
    cpl_matrix_set(amplitudes, 1, 0, fringe_amp);

/* cleaning up */
    cpl_matrix_delete(mdata);
    cpl_matrix_delete(coeffs);
    cpl_matrix_delete(x);
    cpl_matrix_delete(hseries);
    cpl_vector_unwrap(vals);
    cpl_vector_delete(params);

    return amplitudes;
}


/*---------------------------------------------------------------------------*/
/**
 * @brief    Estimate background and fringe levels in an image from a
 * @brief    least-squares fit.
 *
 * @param    img0     Image,
 * @param    mask0    Mask,
 * @param    fringe0  Fringe image
 *
 * @return   A 2-by-1 matrix with the bkg and fringe amplitudes.
 *
 * The function determines the background and fringe levels in the
 * image @a img0 by fitting the image with the fringe image @a fringe0
 * and a constant background in the least-squares sense.  The fit
 * ignores the masked parts of the image and the fringe, where the
 * fringe is weak.
 *
 * The returned matrix must be deallocated using cpl_matrix_delete().
 */
/*---------------------------------------------------------------------------*/
cpl_matrix *hdrl_mime_fringe_amplitudes_ls(const cpl_image * img0,
          const cpl_mask * mask0, const cpl_image * fringe0)
{
    cpl_matrix *mdata;
    cpl_matrix *fdata;
    cpl_matrix *cols12;
    cpl_matrix *coeffs;
    cpl_matrix *amplitudes;
    const double *img_data;
    const cpl_binary *mask_data;
    const double *fringe_data;
    double   *md;
    double   *fd;
    int       nx, ny, size;
    int       msize, i;

    cpl_ensure(img0 != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(mask0 != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(fringe0 != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(cpl_image_get_type(img0) == CPL_TYPE_DOUBLE,
               CPL_ERROR_INVALID_TYPE, NULL);
    cpl_ensure(cpl_image_get_type(fringe0) == CPL_TYPE_DOUBLE,
               CPL_ERROR_INVALID_TYPE, NULL);

/* getting image parameters and stats
   nx, ny          dimensions of images
 */
    nx = cpl_image_get_size_x(img0);
    ny = cpl_image_get_size_y(img0);
    size = nx * ny;
    msize = size - cpl_mask_count(mask0);
    /* check that at least some region have been flagged */
    cpl_ensure(msize > 0 , CPL_ERROR_ILLEGAL_INPUT,NULL);

/* creating masked image and masked fringe */
    mdata = cpl_matrix_new(msize, 1);
    md = cpl_matrix_get_data(mdata);

    fdata = cpl_matrix_new(msize, 1);
    fd = cpl_matrix_get_data(fdata);

    img_data = cpl_image_get_data_double_const(img0);
    mask_data = cpl_mask_get_data_const(mask0);
    fringe_data = cpl_image_get_data_double_const(fringe0);
    for (i = 0; i < size; i++, img_data++, mask_data++, fringe_data++)
    {
        if (*mask_data == CPL_BINARY_0)
        {
            *md = (double) *img_data;
            *fd = (double) *fringe_data;
            md++;
            fd++;
        }
    }

/* computing the least squares coefficients */
    cols12 = cpl_matrix_new(msize, 2);
    cpl_matrix_fill(cols12, 1.0);
    cpl_matrix_copy(cols12, fdata, 0, 0);
    coeffs = hdrl_mime_linalg_solve_tikhonov(cols12, mdata, 1.0e-10);

/* computing the amplitudes */
    amplitudes = cpl_matrix_new(2, 1);
    cpl_matrix_set(amplitudes, 0, 0, cpl_matrix_get(coeffs, 1, 0));
    cpl_matrix_set(amplitudes, 1, 0, cpl_matrix_get(coeffs, 0, 0) +
              cpl_matrix_get(coeffs, 1, 0));

/* cleaning up */
    cpl_matrix_delete(mdata);
    cpl_matrix_delete(fdata);
    cpl_matrix_delete(cols12);
    cpl_matrix_delete(coeffs);

    return amplitudes;
}


/*---------------------------------------------------------------------------*/
/**
 * @brief Evaluate the partial derivatives of the Gaussian mixture
 *
 * @param x[]        Argument
 * @param params[]   Array of length 6 with the means, sigmas, and factors,
 * @param result[]   Derivatives of the GM at x[0]
 *
 * @return zero on success
 *
 */
/*---------------------------------------------------------------------------*/
int hdrl_mime_gmix_derivs1(const double x[], const double params[],
                           double result[])
{
    double    a1, m1, sigma1;
    double    a2, m2, sigma2;
    double    tmp;

/* initializing */
    a1 = params[0];
    m1 = params[1];
    sigma1 = params[2];

    a2 = params[3];
    m2 = params[4];
    sigma2 = params[5];

/* evaluating */
    tmp = (x[0] - m1) / sigma1;
    result[0] = exp(-0.5 * tmp * tmp);
    result[1] = a1 * exp(-0.5 * tmp * tmp);
    result[1] *= tmp / sigma1;
    result[2] = a1 * exp(-0.5 * tmp * tmp);
    result[2] *= tmp * tmp / sigma1;

    tmp = (x[0] - m2) / sigma2;
    result[3] = exp(-0.5 * tmp * tmp);
    result[4] = a2 * exp(-0.5 * tmp * tmp);
    result[4] *= tmp / sigma2;
    result[5] = a2 * exp(-0.5 * tmp * tmp);
    result[5] *= tmp * tmp / sigma2;

    return 0;
}

/*---------------------------------------------------------------------------*/
/**
 * @brief Evaluate Gaussian mixture
 *
 * @param x[]        Argument
 * @param params[]   Array of length 6 with the means, sigmas, and factors,
 * @param result     Value of the GM at x[0]
 *
 * @return zero on success
 *
 */
/*---------------------------------------------------------------------------*/
int hdrl_mime_gmix1(const double x[], const double params[], double *result)
{
    double    a1, m1, sigma1;
    double    a2, m2, sigma2;
    double    tmp;

/* initializing */
    a1 = params[0];
    m1 = params[1];
    sigma1 = params[2];

    a2 = params[3];
    m2 = params[4];
    sigma2 = params[5];

/* evaluating */
    tmp = (x[0] - m1) / sigma1;
    result[0] = a1 * exp(-0.5 * tmp * tmp);
    tmp = (x[0] - m2) / sigma2;
    result[0] += a2 * exp(-0.5 * tmp * tmp);

    return 0;
}

/*---------------------------------------------------------------------------*/
/**
 * @brief   Evaluate the Hermite series at given arguments.
 *
 * @param   n       Number of the Hermite functions,
 * @param   center  Center,
 * @param   scale   Scale factor,
 * @param   coeffs  Hermite coefficients,
 * @param   x       Nodes, at which the functions are evaluated.
 *
 * @return  The Hermite series evaluated at given arguments.
 *
 * The Hermite functions are normalized in the L2-sense.
 *
 * The returned matrix must be deallocated using cpl_matrix_delete().
 */
/*---------------------------------------------------------------------------*/
cpl_matrix *hdrl_mime_hermite_series_create(int n, double center,
          double scale, const cpl_matrix * coeffs, const cpl_matrix * x)
{
    cpl_matrix *series;
    double   *ms;
    const double *mc;
    const double *mx;
    double    rt;
    int       i, k, size;

/* testing input */
    if (x == NULL || coeffs == NULL)
    {
        cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
        return NULL;
    }

    if (n < 1 || scale <= 0.0)
    {
        cpl_error_set(cpl_func, CPL_ERROR_ILLEGAL_INPUT);
        return NULL;
    }

/* The specific dimensions of the matrix x are not used, only its size. */
    size = cpl_matrix_get_nrow(x) * cpl_matrix_get_ncol(x);
    mx = cpl_matrix_get_data_const(x);
    mc = cpl_matrix_get_data_const(coeffs);

/* allocating memory */
    series = cpl_matrix_new(size, 1);
    ms = cpl_matrix_get_data(series);

/* computing the normalization constant */
    rt = 1.0 / sqrt(sqrt(CPL_MATH_PI));

/* evaluating the Hermite functions at the samples */
    for (k = 0; k < size; k++)
    {
        double xk = (mx[k] - center) / scale;
        double tmp1 = rt * exp(-0.5 * xk * xk);
        double tmp2 = rt * sqrt(2.0) * xk * exp(-0.5 * xk * xk);
        for (i = 2; i < n + 2; i++)
        {
            double tmp3 = sqrt(2) * xk * tmp2 - sqrt(i - 1) * tmp1;
            tmp3 = tmp3 / sqrt(i);

            ms[k] += tmp1 * mc[i - 2];
            tmp1 = tmp2;
            tmp2 = tmp3;
        }
    }

/* normalizing */
    cpl_matrix_multiply_scalar(series, 1 / sqrt(scale));

    return series;
}



/*---------------------------------------------------------------------------*/
/**
 * @brief   Create the sum of values of the k-th Hermite function at
 * @brief   given arguments, for k = 0, ..., n-1.
 *
 * @param   n        Number of the Hermite functions.
 * @param   center   Center.
 * @param   scale    Scale factor.
 * @param   x        Nodes, at which the functions are evaluated.
 *
 * @return  The sums of the Hermite functions at given arguments.
 *
 * The Hermite functions are normalized in the L2-sense.
 *
 * The returned matrix must be deallocated using cpl_matrix_delete().
 */
/*---------------------------------------------------------------------------*/
cpl_matrix *hdrl_mime_hermite_functions_sums_create(int n, double center,
          double scale, const cpl_matrix * x)
{
    cpl_matrix *sums;
    double   *ms;
    const double *mx;
    double    rt;
    int       i, k, size;
    double    tmp3;
    double    sqrt_i[n + 2];
    double    rsqrt_i[n + 2];

/* testing input */
    if (x == NULL)
    {
        cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
        return NULL;
    }

    if (n < 1 || scale <= 0.0)
    {
        cpl_error_set(cpl_func, CPL_ERROR_ILLEGAL_INPUT);
        return NULL;
    }

/* The specific dimensions of the matrix x are not used, only its size. */
    size = cpl_matrix_get_nrow(x) * cpl_matrix_get_ncol(x);
    mx = cpl_matrix_get_data_const(x);

/* allocating memory */
    sums = cpl_matrix_new(n, 1);
    ms = cpl_matrix_get_data(sums);

/* computing the normalization constant */
    rt = 1.0 / sqrt(sqrt(CPL_MATH_PI));
    for (i = 1; i < n + 2; i++) {
        sqrt_i[i] = sqrt(i);
        rsqrt_i[i] = 1. / sqrt_i[i];
    }

/* evaluating the Hermite functions at the samples */
    for (k = 0; k < size; k++)
    {
        double xk = (mx[k] - center) / scale;
        double tmp1 = rt * exp(-0.5 * xk * xk);
        double tmp2 = rt * sqrt(2.0) * xk * exp(-0.5 * xk * xk);
        for (i = 2; i < n + 2; i++)
        {
            tmp3 = sqrt(2) * xk * tmp2 - sqrt_i[i - 1] * tmp1;
            tmp3 = tmp3 * rsqrt_i[i];

            ms[i - 2] += tmp1;
            tmp1 = tmp2;
            tmp2 = tmp3;
        }
    }

/* normalizing */
    cpl_matrix_multiply_scalar(sums, 1 / sqrt(scale));

    return sums;
}
/** @endcond */


/**@}*/
