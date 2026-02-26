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

/*
 * Originated from xshoop/xsh/xsh_remove_crh_single.c r154351
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* This should be defined in a more clever way, a parameter for example */
#define REGDEBUG_FULL 0

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_lacosmics.h"
#include "hdrl_utils.h"

#include <math.h>
#include <string.h>
#include <stdint.h>

/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                                   Static
 -----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_lacosmic   Bad Pixel Mask via edge detection
  @ingroup hdrl_bpm

  @brief
   Algorithm to detect bad-pixels and cosmic-rays hits on a single image like
   e.g. science frames

This routine determines bad-pixels on a single image via edge detection
following the algorithm (LA-Cosmic) describe in van Dokkum,
PASP,113,2001,p1420-27. The HDRL implementation does not use use error model
as described in the paper but the error image passed to the function. Moreover
we do several iterations and replace the detected bad pixels in each iteration
by the information of the surrounding pixels.

The calculation is performed by calling the top-level function
hdrl_lacosmic_edgedetect() and the parameters passed to this function can be
created by calling hdrl_lacosmic_parameter_create().


 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*-----------------------------------------------------------------------------
                        LaCosmic parameters Definition
 -----------------------------------------------------------------------------*/
typedef struct {
    HDRL_PARAMETER_HEAD;
    double              sigma_lim ;
    double              f_lim ;
    int                 max_iter ;
} hdrl_lacosmic_parameter;

/* parameter type */
static hdrl_parameter_typeobj hdrl_lacosmic_parameter_type = {
    HDRL_PARAMETER_LACOSMIC,                /* type */
    (hdrl_alloc *)&cpl_malloc,              /* fp_alloc */
    (hdrl_free *)&cpl_free,                 /* fp_free */
    NULL,                                   /* fp_destroy */
    sizeof(hdrl_lacosmic_parameter),             /* obj_size */
};

/*----------------------------------------------------------------------------*/
/**
  @brief   Creates LaCosmic parameters object
  @param   sigma_lim  Limiting sigma for detection on the sampling image
  @param   f_lim      Limiting f factor for detection on the modified Laplacian
                      image.
  @param    max_iter  Maximum number of iterations
  @return   The LaCosmic parameters object. It needs to be deallocated with
            hdrl_parameter_delete()
  @see      hdrl_lacosmic_edgedetect()

  @note For the algorithm see the paper of
  <a href="http://arxiv.org/abs/astro-ph/0108003"> van Dokkum,
  PASP,113,2001,p1420-27</a>.
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_lacosmic_parameter_create(
        double              sigma_lim,
        double              f_lim,
        int                 max_iter)
{
    hdrl_lacosmic_parameter * p = (hdrl_lacosmic_parameter *)
               hdrl_parameter_new(&hdrl_lacosmic_parameter_type);
    p->sigma_lim = sigma_lim ;
    p->f_lim = f_lim ;
    p->max_iter = max_iter ;
    return (hdrl_parameter *)p;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Verify basic correctness of the LaCosmic parameters
  @param    param       LaCosmic parameters
  @return   CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_lacosmic_parameter_verify(
                const hdrl_parameter    *   param)
{
    const hdrl_lacosmic_parameter * param_loc = (const hdrl_lacosmic_parameter *)param;

    cpl_error_ensure(param != NULL, CPL_ERROR_NULL_INPUT,
                     return CPL_ERROR_NULL_INPUT, "NULL Input Parameters");
    cpl_error_ensure(hdrl_lacosmic_parameter_check(param),
                     CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
                                     "Expected LaCosmic parameter") ;

    cpl_error_ensure(param_loc->max_iter > 0, CPL_ERROR_ILLEGAL_INPUT,
                    return CPL_ERROR_ILLEGAL_INPUT, "max_iter must be >0");
    cpl_error_ensure(param_loc->f_lim >= 0, CPL_ERROR_ILLEGAL_INPUT,
                    return CPL_ERROR_ILLEGAL_INPUT, "f_lim must be >=0");
    cpl_error_ensure(param_loc->sigma_lim >= 0, CPL_ERROR_ILLEGAL_INPUT,
                    return CPL_ERROR_ILLEGAL_INPUT, "sigma_lim must be >=0");

    return CPL_ERROR_NONE ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Check that the parameter is an LaCosmic parameter
  @param    self The parameter to check
  @return   True or False
 */
/*----------------------------------------------------------------------------*/
cpl_boolean hdrl_lacosmic_parameter_check(const hdrl_parameter * self)
{
    return hdrl_parameter_check_type(self, &hdrl_lacosmic_parameter_type);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the sigma_lim in the LaCosmic parameter
  @param    p   The LaCosmic parameter
  @return   The sigma_lim value
 */
/*----------------------------------------------------------------------------*/
double hdrl_lacosmic_parameter_get_sigma_lim(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.0);
    return ((const hdrl_lacosmic_parameter *)p)->sigma_lim;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the f_lim in the LaCosmic parameter
  @param    p   The LaCosmic parameter
  @return   The f_lim value
 */
/*----------------------------------------------------------------------------*/
double hdrl_lacosmic_parameter_get_f_lim(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1.0);
    return ((const hdrl_lacosmic_parameter *)p)->f_lim;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Access the max_iter in the LaCosmic parameter
  @param    p   The LaCosmic parameter
  @return   The max_iter value
 */
/*----------------------------------------------------------------------------*/
int hdrl_lacosmic_parameter_get_max_iter(
        const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    return ((const hdrl_lacosmic_parameter *)p)->max_iter;
}

/*----------------------------------------------------------------------------*/
/**
  @brief Create parameter list for the LaCosmic computation
  @param base_context   base context of parameter (e.g. recipe name)
  @param prefix         prefix of parameter, may be empty string
  @param defaults        default values

  @return  parameterlist controlling the LaCosmic algorithm

    base_context.prefix.sigma_lim
    base_context.prefix.f_lim
    base_context.prefix.max_iter
    The CLI aliases omit the base_context.
 */
/*----------------------------------------------------------------------------*/
cpl_parameterlist * hdrl_lacosmic_parameter_create_parlist(
        const char           *base_context,
        const char           *prefix,
        const hdrl_parameter *defaults)
{
    cpl_ensure(prefix && base_context && defaults,
    		CPL_ERROR_NULL_INPUT, NULL);

    cpl_ensure(hdrl_lacosmic_parameter_check(defaults),
    		   CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    cpl_parameterlist *parlist = cpl_parameterlist_new();

    double sigma_lim_def = hdrl_lacosmic_parameter_get_sigma_lim(defaults) ;
    double f_lim_def = hdrl_lacosmic_parameter_get_f_lim(defaults) ;
    int max_iter_def = hdrl_lacosmic_parameter_get_max_iter(defaults) ; 

    /* --prefix.sigma_lim */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "sigma_lim", base_context,
            "Poisson fluctuation threshold to flag cosmics"
            "(see van Dokkum, PASP,113,2001,p1420-27).",
            CPL_TYPE_DOUBLE, sigma_lim_def) ;

    /* --prefix.f_lim */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "f_lim", base_context,
            "Minimum contrast between the Laplacian image and the fine "
            "structure image that a point must have to be flagged as cosmics"
            , CPL_TYPE_DOUBLE, f_lim_def) ;

    /* --prefix.max_iter */
    hdrl_setup_vparameter(parlist, prefix, ".", "", "max_iter", base_context,
            "Maximum number of alghoritm iterations", CPL_TYPE_INT, max_iter_def) ;

    if (cpl_error_get_code()) {
        cpl_parameterlist_delete(parlist);
        return NULL;
    }
    return parlist;
}

/*----------------------------------------------------------------------------*/
/**
  @brief Parse parameterlist to create input parameters for the LaCosmic
  @param parlist        parameter list to parse
  @param prefix         prefix of parameter name
  @return   Input parameters for the LaCosmic Computation

  Reads a parameterlist in order to create LaCosmic parameters.
  Expects a parameterlist containing:
  - prefix.sigma_lim
  - prefix.f_lim
  - prefix.max_iter
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_lacosmic_parameter_parse_parlist(
        const cpl_parameterlist *   parlist,
        const char              *   prefix)
{
    cpl_ensure(prefix && parlist, CPL_ERROR_NULL_INPUT, NULL);
    char                *   name ;
    const cpl_parameter *   par;
    double                  sigma_lim, f_lim;
    int                     max_iter;

    /* --sigma_lim */
    name = hdrl_join_string(".", 2, prefix, "sigma_lim");
    par=cpl_parameterlist_find_const(parlist, name);
    sigma_lim = cpl_parameter_get_double(par);
    cpl_free(name) ;

    /* --f_lim */
    name = hdrl_join_string(".", 2, prefix, "f_lim");
    par=cpl_parameterlist_find_const(parlist, name);
    f_lim = cpl_parameter_get_double(par);
    cpl_free(name) ;

    /* --max_iter */
    name = hdrl_join_string(".", 2, prefix, "max_iter");
    par=cpl_parameterlist_find_const(parlist, name);
    max_iter = cpl_parameter_get_int(par);
    cpl_free(name) ;

    if (cpl_error_get_code()) {
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                "Error while parsing parameterlist with prefix %s", prefix);
        return NULL;
    } else {
        return hdrl_lacosmic_parameter_create(sigma_lim, f_lim, max_iter) ;
    }
}



/*----------------------------------------------------------------------------*/
/**
  @brief Detect bad-pixels  / cosmic-rays on a single image.
  @param ima_in    The input image
  @param params    LaCosmic computation parameters

  @return Mask where all detected bad-pixels/cosmics-rays are marked
          as bad or NULL on error

This routine determines bad-pixels on a single image via edge
detection following the algorithm (LA-Cosmic) describe in van Dokkum,
PASP,113,2001,p1420-27. It was originally developed to detect cosmic
ray hits but can also be used in the more general context of detecting
bad pixels.  The HDRL implementation does not use use error model as
described in the paper but the error image passed to the
function. Moreover we do several iterations with \e max_iter defining
an upper limit for the number of iterations, i.e. the iteration stops
if no new bad pixels are found or \e max_iter is reached. In each
iteration we replace the detected cosmic ray hits by the median of the
surroundings 5x5 pixels taking into account the pixel quality
information. The input parameter \e sigma_lim and \e f_lim refer to
\f$\sigma_{lim}\f$ and \f$f_{lim}\f$ as described in the paper
mentioned above. The hdrl parameter passed to this routine is created
by hdrl_lacosmic_parameter_create().

@note Be aware that the implementation only detects positive bad
pixels / cosmic ray hits, i.e. no "holes" in the image are detected,
but in such a case the pixels surrounding the whole are marked as
bad. Holes in the image can be introduced, if e.g. one subtracts a not
cosmic-ray-cleaned image from another image.

*/
/*----------------------------------------------------------------------------*/
cpl_mask * hdrl_lacosmic_edgedetect(
        const hdrl_image        *   ima_in,
        const hdrl_parameter    *   params)
{
    cpl_image * sci_data;
    cpl_image * sci_error = NULL;
    cpl_mask  * sci_mask = NULL;
    cpl_image * laplacian_redu_data = NULL; /* re-binned Laplacian */
    cpl_image * subs2_data = NULL; /* Sub-sampled image by a factor of 2 */
    cpl_image * s_data = NULL;
    cpl_image * f_data = NULL;
    cpl_image * r_data = NULL;


    intptr_t subs2_nx = 0;
    intptr_t subs2_ny = 0;

    cpl_binary * psci_mask = NULL;
    cpl_binary * pout_mask = NULL;

    cpl_matrix * laplacian_kernel = NULL;

    cpl_mask * median3_kernel = NULL;
    cpl_mask * median5_kernel = NULL;
    cpl_mask * median7_kernel = NULL;
    cpl_mask * lastiter_mask = NULL;
    cpl_mask * out_mask = NULL;

    double * psci_data = NULL;
    double * psci_error = NULL;
    double * psubs2_data = NULL;
    double * plaplacian_data = NULL;
    double * plaplacian_redu_data = NULL;
    double * psci_median3_data = NULL;
    double * ps_data = NULL;
    double * pf_data = NULL;
    double * pr_data = NULL;

    int nbiter = 1;

    intptr_t nx = 0;
    intptr_t ny = 0;

    /* Check Entries */
    cpl_ensure(ima_in, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(hdrl_lacosmic_parameter_verify(params) == CPL_ERROR_NONE,
            CPL_ERROR_ILLEGAL_INPUT, NULL) ;

    cpl_ensure(hdrl_image_get_size_x(ima_in) >= 7,
               CPL_ERROR_INCOMPATIBLE_INPUT, NULL);
    cpl_ensure(hdrl_image_get_size_y(ima_in) >= 7,
               CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    /* Local Usage parameters */
    const hdrl_lacosmic_parameter * p_loc = (const hdrl_lacosmic_parameter *)params ;

    sci_data = cpl_image_cast(hdrl_image_get_image_const(ima_in),
            CPL_TYPE_DOUBLE);
    if (hdrl_image_get_mask_const(ima_in)) {
        sci_mask = cpl_mask_duplicate(hdrl_image_get_mask_const(ima_in));
    } else {
        sci_mask = cpl_mask_new(cpl_image_get_size_x(sci_data),
                cpl_image_get_size_y(sci_data));
    }
    sci_error = cpl_image_cast(hdrl_image_get_error_const(ima_in),
            CPL_TYPE_DOUBLE);

    /* Laplacian */
    laplacian_kernel = cpl_matrix_new(3, 3);
    cpl_matrix_set(laplacian_kernel, 0, 0, 0.0);
    cpl_matrix_set(laplacian_kernel, 0, 1, -1.0);
    cpl_matrix_set(laplacian_kernel, 0, 2, 0.0);
    cpl_matrix_set(laplacian_kernel, 1, 0, -1.0);
    cpl_matrix_set(laplacian_kernel, 1, 1, 4.0);
    cpl_matrix_set(laplacian_kernel, 1, 2, -1.0);
    cpl_matrix_set(laplacian_kernel, 2, 0, 0.0);
    cpl_matrix_set(laplacian_kernel, 2, 1, -1.0);
    cpl_matrix_set(laplacian_kernel, 2, 2, 0.0);

    /* Median 3x3*/
    median3_kernel = cpl_mask_new(3, 3);
    cpl_mask_not(median3_kernel); /* All values set to unity */

    /* Median 5x5 */
    median5_kernel = cpl_mask_new(5, 5);
    cpl_mask_not(median5_kernel); /* All values set to unity */

    /* Median 7x7 */
    median7_kernel = cpl_mask_new(7, 7);
    cpl_mask_not(median7_kernel); /* All values set to unity */

    out_mask = cpl_mask_new(cpl_mask_get_size_x(sci_mask),
                    cpl_mask_get_size_y(sci_mask));

    nx = cpl_image_get_size_x(sci_data);
    ny = cpl_image_get_size_y(sci_data);

    /* Preparing different kernels */
    psci_data = cpl_image_get_data_double(sci_data);
    psci_error = cpl_image_get_data_double(sci_error);

    psci_mask = cpl_mask_get_data(sci_mask);
    pout_mask = cpl_mask_get_data(out_mask);

    subs2_nx = nx * 2;
    subs2_ny = ny * 2;

    subs2_data = cpl_image_new(subs2_nx, subs2_ny, CPL_TYPE_DOUBLE);
    psubs2_data = cpl_image_get_data_double( subs2_data);

    laplacian_redu_data = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
    plaplacian_redu_data = cpl_image_get_data_double( laplacian_redu_data);

    s_data = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
    ps_data = cpl_image_get_data_double( s_data);

    f_data = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
    pf_data = cpl_image_get_data_double( f_data);

    r_data = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
    pr_data = cpl_image_get_data_double( r_data);

    /* mask needed to stop the loop if the pixel-replacements is not propperly
     * working */
    lastiter_mask = cpl_mask_duplicate(out_mask);

    /* The actual cosmic ray search is done here.
     * stop if no new cosmic are found or max iter is reached */
    while (nbiter <= p_loc->max_iter) {
        cpl_vector * median;
        double * psci_median3_7_data;
        double * ps_median_data;

        /* Stopping if the detections of the previus run are identical to
         *  the last run */
        if (nbiter > 1 && !hdrl_check_maskequality(lastiter_mask, out_mask)) {
            cpl_msg_debug(cpl_func, "Detections of iteration %d and %d are "
                            "identical - stopping here", nbiter-1, nbiter );
            break;
        }
        cpl_mask_delete(lastiter_mask);
        lastiter_mask = cpl_mask_duplicate(out_mask);

        /* Super sample data:
           Create a 2n x 2n images like this
         | 1 | 2 |  =>  | 1 | 1 | 2 | 2 |
         | 3 | 4 |      | 1 | 1 | 2 | 2 |
                        | 3 | 3 | 4 | 4 |
                        | 3 | 3 | 4 | 4 | */

        for (intptr_t j = 0; j < ny; j++) {
            intptr_t j_2_subs2_nx = j * 2 * subs2_nx;
            intptr_t j_nx = j * nx;
            for (intptr_t i = 0; i < nx; i++) {
                double val = psci_data[i + j_nx];
                intptr_t pix = i * 2 + j_2_subs2_nx;
                psubs2_data[pix] = val;
                psubs2_data[pix + subs2_nx] = val;
                pix++;
                psubs2_data[pix] = val;
                psubs2_data[pix + subs2_nx] = val;
            }
        }

        /* Doing the Laplacian convolution and set negative pixels to
           0 in the Laplacian image
          0  -1   0
         -1   4  -1
          0  -1   0 */

        cpl_image * laplacian_data = hdrl_parallel_filter_image(subs2_data,
                                                    laplacian_kernel, NULL,
                                                    CPL_FILTER_LINEAR);

        plaplacian_data = cpl_image_get_data_double(laplacian_data);
        for (intptr_t i = 0; i < subs2_nx * subs2_ny; i++) {
            if (plaplacian_data[i] < 0.0) {
                plaplacian_data[i] = 0.0;
            } else {
                /*TODO Where does this factor of 8 come from???*/
                plaplacian_data[i] *= 8.0;
            }
        }

        /* fix discontinuity at edges due to incomplete laplace kernel by
         * mirroring the neighbouring pixel */
        for (intptr_t i = 0; i < subs2_ny; i++) {
            plaplacian_data[i * subs2_nx + 0] = plaplacian_data[i * subs2_nx + 1];
            plaplacian_data[i * subs2_nx + subs2_nx - 1] =
                plaplacian_data[i * subs2_nx +  + subs2_nx - 2];
        }
        for (intptr_t j = 0; j < subs2_nx; j++) {
            plaplacian_data[0 * subs2_nx + j] = plaplacian_data[1 * subs2_nx + j];
            plaplacian_data[(subs2_ny - 1) * subs2_nx + j] =
                plaplacian_data[(subs2_ny - 2) * subs2_nx + j];
        }

#if REGDEBUG_FULL
        cpl_image_save(laplacian_data, "Lpositive.fits", CPL_BPP_IEEE_DOUBLE, 
                NULL, CPL_IO_DEFAULT);
#endif

        /* resample to the original size
         | 1 | 1 | 2 | 2 |    | 1 | 2 |
         | 1 | 1 | 2 | 2 |    | 3 | 4 |
         | 3 | 3 | 4 | 4 | =>
         | 3 | 3 | 4 | 4 |               */

HDRL_OMP(omp parallel for)
        for (intptr_t j = 0; j < ny; j++) {
            intptr_t j_2_subs2_nx = j * 2 * subs2_nx;
            intptr_t j_nx = j * nx;
            for (intptr_t i = 0; i < nx; i++) {
                intptr_t pix = i * 2 + j_2_subs2_nx;
                plaplacian_redu_data[i + j_nx] =
                    (plaplacian_data[pix] +
                     plaplacian_data[pix + 1] +
                     plaplacian_data[pix + subs2_nx] +
                     plaplacian_data[pix + subs2_nx + 1]) * 0.25;
                /* A) Compute S image */
                ps_data[i + j_nx] =
                   0.5 * plaplacian_redu_data[i + j_nx] / psci_error[i + j_nx];
            }
        }

#if REGDEBUG_FULL
        cpl_image_save(laplacian_redu_data, "Lplus.fits", CPL_BPP_IEEE_DOUBLE,
                       NULL, CPL_IO_DEFAULT);
#endif

        /* A) compute S median image */
        cpl_image * s_median_data = hdrl_parallel_filter_image(s_data, NULL,
                                                   median5_kernel,
                                                   CPL_FILTER_MEDIAN);
        ps_median_data = cpl_image_get_data_double( s_median_data);


        /* B) Compute s2 -> denoted S' in the original paper */
        for (intptr_t i = 0; i < nx * ny; i++) {
            ps_data[i] -= ps_median_data[i];
        }

#if REGDEBUG_FULL
        cpl_image_save( s_data, "S2.fits", CPL_BPP_IEEE_DOUBLE, NULL,
                CPL_IO_DEFAULT);
#endif

        /* Apply 3x3 median filter on data */
        cpl_image * sci_median3_data =
            hdrl_parallel_filter_image(sci_data, NULL,
                                       median3_kernel,
                                       CPL_FILTER_MEDIAN);
        psci_median3_data = cpl_image_get_data_double(sci_median3_data);

        /* Apply 7x7 median filter */

        cpl_image * sci_median3_7_data =
            hdrl_parallel_filter_image(sci_median3_data, NULL,
                                       median7_kernel,
                                       CPL_FILTER_MEDIAN);
        psci_median3_7_data = cpl_image_get_data_double(sci_median3_7_data);

        /* C) Compute F, i.e. the fine structure image */
        for (intptr_t i = 0; i < nx * ny; i++) {
            pf_data[i] = psci_median3_data[i] - psci_median3_7_data[i];
            /* TODO: why this setting?
             * why one use an absolute number?
             * pf_data may span on different ranges as it depends on the value
             * of the difference psci_median3_data[i] - psci_median3_7_data[i]
             */
            if (pf_data[i] < 0.01) {
                pf_data[i] = 0.01;
            }
        }

#if REGDEBUG_FULL
        cpl_image_save( f_data, "F.fits", CPL_BPP_IEEE_DOUBLE, NULL,
                       CPL_IO_DEFAULT);
#endif

        /* D) Compute R, i.e. the ratio of Laplacian and fine structure image */
        for (intptr_t i = 0; i < nx * ny; i++) {
            pr_data[i] = plaplacian_redu_data[i] / pf_data[i];
        }

#if REGDEBUG_FULL
        cpl_image_save( r_data, "R.fits", CPL_BPP_IEEE_DOUBLE, NULL,
                       CPL_IO_DEFAULT);
#endif

        /* E) Search for cosmics */
        median = cpl_vector_new(24);
        for (intptr_t j = 0; j < ny - 1; j++) {
            intptr_t j_nx = j * nx;
            for (intptr_t i = 0; i < nx - 1; i++) {
                intptr_t i_plus_j_nx = i + j_nx;
                if (ps_data[i_plus_j_nx] > p_loc->sigma_lim &&
                    pr_data[i_plus_j_nx] > p_loc->f_lim &&
                    /* check for bad pixels added here */
                    psci_mask[i_plus_j_nx] == CPL_BINARY_0) {
                    double *data = NULL;
                    cpl_vector* med_vect = NULL;
                    intptr_t li, lj, ui, uj, m;
                    /* we flag the CRH in the science frame */
                    pout_mask[i_plus_j_nx] = CPL_BINARY_1;
                    cpl_msg_debug(cpl_func, "Detection found at x=%zd y=%zd with "
                                    "value=%g", i+1, j+1, psci_data[i_plus_j_nx]);

                    /* we replace the CRH with median of the surroundings 
                       pixels in a box 5x5 pixels */
                    m = 0;
                    li = CX_MAX(i - 2, 0);
                    lj = CX_MAX(j - 2, 0);
                    ui = CX_MIN(nx, i + 3);
                    uj = CX_MIN(ny, j + 3);

                    for (intptr_t k = lj; k < uj; k++) {
                        intptr_t k_nx = k * nx;
                        for (intptr_t l = li; l < ui; l++) {
                            intptr_t l_plus_k_nx = l + k_nx;
                            if (ps_data[l_plus_k_nx] <= p_loc->sigma_lim &&
                                    /* check for bad pixels added here */
                                    psci_mask[l_plus_k_nx] == CPL_BINARY_0) {
                                cpl_vector_set(median,m,psci_data[l_plus_k_nx]);
                                m++;
                                continue;
                            }
                            if (pr_data[l_plus_k_nx] <= p_loc->f_lim &&
                                    /* check for bad pixels added here */
                                    psci_mask[l_plus_k_nx] == CPL_BINARY_0) {
                                cpl_vector_set(median,m,psci_data[l_plus_k_nx]);
                                m++;
                            }
                        }
                    }
                    /* if no good pixel has been found skip pixel value
                       replacement */
                    if(m == 0) continue;
                    /* Replace CRH value with median of computed values on
                       good pixels */
                    data = cpl_vector_get_data( median);
                    med_vect = cpl_vector_wrap( m, data);
                    psci_data[i_plus_j_nx] = cpl_vector_get_median(med_vect);
                    cpl_msg_debug(cpl_func, "Detection replaced with value=%g",
                                  psci_data[i_plus_j_nx]);
                    cpl_vector_unwrap(med_vect);
                } /* end check on thresholds */
            } /* end loop over image columns */
        } /* end loop over image rows */

        cpl_vector_delete(median); median = NULL;

        nbiter++;
        cpl_image_delete(laplacian_data);
        cpl_image_delete(sci_median3_7_data);
        cpl_image_delete(sci_median3_data);
        cpl_image_delete(s_median_data);
    }

#if REGDEBUG_FULL
    cpl_mask_save( *out_mask, "CRH_SINGLE.fits", NULL, CPL_IO_DEFAULT);
#endif

    /* Free memory */
    cpl_matrix_delete(laplacian_kernel);
    cpl_mask_delete(median3_kernel);
    cpl_mask_delete(median5_kernel);
    cpl_mask_delete(median7_kernel);
    cpl_mask_delete(lastiter_mask);
    cpl_image_delete(laplacian_redu_data);
    cpl_image_delete(subs2_data);
    cpl_image_delete(s_data);
    cpl_image_delete(f_data);
    cpl_image_delete(r_data);
    cpl_image_delete(sci_data);
    cpl_image_delete(sci_error);
    cpl_mask_delete(sci_mask);

    return out_mask;
}

/**@}*/

