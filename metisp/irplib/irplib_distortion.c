/* $Id: irplib_distortion.c,v 1.52 2013-01-29 08:43:33 jtaylor Exp $
 *
 * This file is part of the irplib package
 * Copyright (C) 2002,2003 European Southern Observatory
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02111-1307  USA
 */

/*
 * $Author: jtaylor $
 * $Date: 2013-01-29 08:43:33 $
 * $Revision: 1.52 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "irplib_distortion.h"

#include "irplib_flat.h"
#include "irplib_utils.h"
#include "irplib_polynomial.h"

#include <math.h>
#include <float.h>

/*-----------------------------------------------------------------------------
                                   Define
 -----------------------------------------------------------------------------*/

#define IRPLIB_MAX(A,B) ((A) > (B) ? (A) : (B))
#define IRPLIB_MIN(A,B) ((A) < (B) ? (A) : (B))

#define ARC_MINGOODPIX      100
#define ARC_MINARCLENFACT   2.0
#define ARC_MINNBARCS       4
#define ARC_RANGE_FACT      3.0
#define ARC_WINDOWSIZE      32

#define TRESH_MEDIAN_MIN    0.0
#define TRESH_SIGMA_MAX     200.0

/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_distortion       Distortion correction functions
 */
/*----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                                Functions prototypes
 -----------------------------------------------------------------------------*/

static cpl_apertures * irplib_distortion_detect_arcs(cpl_image *,
        cpl_image **, int, int, double, int, int, int, int);
static cpl_error_code irplib_distortion_fill_border(cpl_image *, int, int,
                                                    int, int, double);
static int irplib_distortion_threshold1d(cpl_image *, double, cpl_image *, 
        double);
static cpl_error_code irplib_distortion_purge_arcs(cpl_apertures **, cpl_image *,
                                                   const cpl_image *, int, int,
                                                   double);
static cpl_error_code irplib_distortion_fill_arc_positions(cpl_bivector *,
                                                          cpl_vector *,
                                                          const cpl_image *,
                                                          const cpl_image *,
                                                          const cpl_apertures *);

static double irplib_distortion_get_row_centroid(const cpl_image *,
                                                 const cpl_image *, int, int);

static int irplib_distortion_sub_hor_lowpass(cpl_image *, int);
static cpl_image * irplib_distortion_remove_ramp(const cpl_image *);

static cpl_error_code irplib_image_filter_background_line(cpl_image *,
        const cpl_image *, int, cpl_boolean) ;

static cpl_error_code irplib_polynomial_fit_2d(cpl_polynomial *,
                                               const cpl_bivector *,
                                               const cpl_vector *, int,
                                               double, double *);

static cpl_matrix * irplib_matrix_product_normal_create(const cpl_matrix *);

/*-----------------------------------------------------------------------------
                                Functions code
 -----------------------------------------------------------------------------*/

/**@{*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Estimate the distortion using vertical curved arc lines
  @param    org     the input image
  @param    xmin
  @param    ymin    Define the zone to take into account
  @param    xmax
  @param    ymax
  @param    auto_ramp_sub   To automatically clean the image before detection
  @param    arc_sat Saturation threshold for the arc lines
  @param    max_arc_width   Maximum arc width allowed in pixels
  @param    kappa   Kappa for arc detection (0.33 = good default)
  @param    degree  The positive degree of the fitted polynomial
  @param    arcs    The found arc lines, *arcs is NULL on error
  @return   The 2d polynomial describing the distortion or NULL in error case
  @see cpl_image_warp_polynomial()
  @note The arc lines are expected to be vertical.

  If (Xi, yi) define positions on the curved arc lines, and (xi, yi) the
  associated positions on the straight arc lines, the created polynomial is
  Xi = P(xi, yi).

  The polynomial has the property for all xi that xi = P(xi, (Ny+1)/2),
  where Ny is the Y-size, i.e. the polynomial transformation does not move
  the points on the detector center line, y = (Ny+1)/2.

  The returned polynomial can passed to cpl_image_warp_polynomial() to correct 
  the image.
 */
/*----------------------------------------------------------------------------*/
cpl_polynomial * irplib_distortion_estimate(
        const cpl_image *   org,
        int                 xmin,
        int                 ymin,
        int                 xmax,
        int                 ymax,
        int                 auto_ramp_sub,
        int                 arc_sat,
        int                 max_arc_width,
        double              kappa,
        int                 degree,
        cpl_apertures   **  arcs)
{
    cpl_image      * local_im;
    cpl_image      * filtered;
    cpl_image      * label_image;
    double           rightmost, leftmost;
    cpl_bivector   * grid;
    cpl_vector     * values_to_fit;
    int              n_arcs;
    cpl_polynomial * poly2d;
    double           mse = 0.0;
    const int        nx = cpl_image_get_size_x(org);
    const int        ny = cpl_image_get_size_y(org);
    const int        min_arc_range = (int)(nx / ARC_RANGE_FACT);
    int              i;

    /* Check entries */
    cpl_ensure(org           != NULL, CPL_ERROR_NULL_INPUT,    NULL);
    cpl_ensure(kappa         >= 0.0,  CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(max_arc_width > 0,     CPL_ERROR_ILLEGAL_INPUT, NULL);

    /* The background may vary strongly along the vertical line. */
    /* Detect and rm background with a 1+2*max_arc_width x 1 median filter */

    filtered = cpl_image_new(nx, ny, cpl_image_get_type(org));

    irplib_image_filter_background_line(filtered, org, max_arc_width, CPL_TRUE);

    if (auto_ramp_sub) {
        local_im = irplib_distortion_remove_ramp(filtered);
        cpl_image_delete(filtered);
    } else {
        local_im = filtered;
    }

    cpl_error_ensure(local_im != NULL, cpl_error_get_code(),
                     return(NULL), "Cannot clean the image");

    /* Detect the arcs in the input image */
    *arcs = irplib_distortion_detect_arcs(local_im, &label_image, arc_sat,
                                          max_arc_width, kappa, xmin, ymin,
                                          xmax, ymax);
    if (*arcs == NULL) {
        cpl_image_delete(local_im);
        cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                              "Cannot detect the arcs");
        return NULL;
    }
    n_arcs = cpl_apertures_get_size(*arcs);
    cpl_msg_info(cpl_func, "%d detected arcs", n_arcs);

    /* Check that the arcs are not concentrated in the same zone */
    rightmost = leftmost = cpl_apertures_get_pos_x(*arcs, 1);
    for (i=1; i<n_arcs; i++) {
        if (cpl_apertures_get_pos_x(*arcs, i+1) < leftmost)
            leftmost = cpl_apertures_get_pos_x(*arcs, i+1);
        if (cpl_apertures_get_pos_x(*arcs, i+1) > rightmost)
            rightmost = cpl_apertures_get_pos_x(*arcs, i+1);
    }
    if ((int)(rightmost-leftmost) < min_arc_range) {
#if defined CPL_HAVE_VA_ARGS && CPL_HAVE_VA_ARGS != 0
        cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                              "too narrow range (%g-%g)<%d",
                              rightmost, leftmost, min_arc_range);
#else
        cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                              "too narrow range");
#endif
        cpl_apertures_delete(*arcs);
        cpl_image_delete(local_im);
        cpl_image_delete(label_image);
        *arcs = NULL;
        return NULL;
    }

    /* Create a 2-D deformation grid with detected arcs */
    cpl_msg_info(cpl_func, "Create deformation grid");
    grid = cpl_bivector_new(n_arcs * ny);
    values_to_fit = cpl_vector_new(n_arcs * ny);

    if (irplib_distortion_fill_arc_positions(grid, values_to_fit, local_im,
                                            label_image, *arcs)){
        cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                              "cannot get arcs positions");
        cpl_apertures_delete(*arcs);
        cpl_image_delete(local_im);
        cpl_image_delete(label_image);
        *arcs = NULL;
        return NULL;
    }
    cpl_image_delete(label_image);
    cpl_image_delete(local_im);

    /* Apply the fitting */
    poly2d = cpl_polynomial_new(2);
    if (irplib_polynomial_fit_2d(poly2d, grid, values_to_fit, degree,
                                 0.5*(ny+1), &mse)) {
        cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                              "cannot apply the 2d fit");
        cpl_bivector_delete(grid);
        cpl_vector_delete(values_to_fit);
        cpl_apertures_delete(*arcs);
        *arcs = NULL;
        return NULL;
    }

    cpl_msg_info(cpl_func, 
            "Fitted a %d. degree 2D-polynomial to %"CPL_SIZE_FORMAT" points "
            "with mean-square error: %g", degree,
            cpl_vector_get_size(values_to_fit), mse);

    /* Free and return */
    cpl_bivector_delete(grid);
    cpl_vector_delete(values_to_fit);
    return poly2d;
}

/**@}*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Detect the vertical arcs in an image
  @param    im          the input image
  @param    label_im    the output label image
  @param    arc_sat     Saturation threshold for the arcs
  @param    max_arc_width   Maximum arc width allowed
  @param    kappa           For arcs detection (0.33 is a good default)
  @param    xmin
  @param    ymin        Define the zone to take into account
  @param    xmax
  @param    ymax
  @return   The arcs or NULL in error case
  
  The arcs are expected to be vertical.
 */
/*----------------------------------------------------------------------------*/
static cpl_apertures * irplib_distortion_detect_arcs(
        cpl_image *   im,
        cpl_image **  label_im,
        int             arc_sat,
        int             max_arc_width,
        double          kappa,
        int             xmin,
        int             ymin,
        int             xmax,
        int             ymax)
{
    const int           ny = cpl_image_get_size_y(im);
    /* Set min_arclen */
    const int           min_arclen = (int)(ny / ARC_MINARCLENFACT);
    cpl_image       *   filt_im;
    cpl_mask        *   filter;
    cpl_image       *   collapsed;
    cpl_mask        *   bin_im;
    double              threshold, fillval, median_val, sigma;
    cpl_apertures   *   det;
    cpl_size            nobj;
    int                 ngoodpix;
    
    /* Default values for output parameters */
    *label_im = NULL;

    /* Clear zones to be ignored (to avoid false detections) */
    median_val = cpl_image_get_median_dev(im, &sigma);
    fillval = median_val-sigma/2.0;
    if (irplib_distortion_fill_border(im, xmin, ymin, xmax, ymax, fillval)) {
        cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                              "cannot fill bad zones");
        return NULL;
    }

    /* Subtract a low-pass */
    filt_im = cpl_image_duplicate(im);
    if (irplib_distortion_sub_hor_lowpass(filt_im, ARC_WINDOWSIZE) == -1) {
        cpl_image_delete(filt_im);
        return NULL;
    }
    
    /* Get relevant stats for thresholding */
    median_val = cpl_image_get_median_dev(filt_im, &sigma);

    /* Correct median_val and sigma if necessary */
    if (median_val < TRESH_MEDIAN_MIN) median_val = TRESH_MEDIAN_MIN;
    if (sigma > TRESH_SIGMA_MAX) sigma = TRESH_SIGMA_MAX;

    /* Set the threshold */
    threshold = median_val + sigma * kappa;

    /* Collapse the image */
    collapsed = cpl_image_collapse_median_create(filt_im, 0, 0, 0);

    /* Threshold to keep only the arcs - use of the collapsed image */
    if (irplib_distortion_threshold1d(filt_im, median_val, collapsed, 0.0)==-1) {
        cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                              "cannot threshold the filtered image");
        cpl_image_delete(filt_im);
        cpl_image_delete(collapsed);
        return NULL;
    }
    cpl_image_delete(collapsed);

    /* Binarize the image */
    bin_im = cpl_mask_threshold_image_create(filt_im, threshold, 
            DBL_MAX);
    cpl_image_delete(filt_im);
    if (bin_im == NULL) {
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                              "cannot binarise the image");
        return NULL;
    }

    /* Test if there are enough good pixels */
    ngoodpix = cpl_mask_count(bin_im);
    if (ngoodpix < ARC_MINGOODPIX) {
#if defined CPL_HAVE_VA_ARGS && CPL_HAVE_VA_ARGS != 0
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                              "Too few (%d) white pixels", ngoodpix);
#else
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                              "Too few white pixels");
#endif
        cpl_mask_delete(bin_im);
        return NULL;
    }

    /* Apply a morphological opening to clean the isolated pixels */
    filter = cpl_mask_new(3, 3);
    cpl_mask_not(filter);
    cpl_mask_filter(bin_im, bin_im, filter, CPL_FILTER_OPENING,
                    CPL_BORDER_ZERO);
    cpl_mask_delete(filter);

    /* Labelize pixel map to a label image */
    *label_im = cpl_image_labelise_mask_create(bin_im, &nobj);
    cpl_mask_delete(bin_im);

    /* Compute statistics on objects */
    if ((det = cpl_apertures_new_from_image(im, *label_im)) == NULL) {
        cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                              "Cannot compute arcs stats");
        cpl_image_delete(*label_im);
        *label_im = NULL;
        return NULL;
    }

    /* Purge non-relevant arcs */
    if (irplib_distortion_purge_arcs(&det, *label_im, im, min_arclen,
                                     max_arc_width, arc_sat)) {
        cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                              "Cannot purge the arcs");
        cpl_image_delete(*label_im);
        *label_im = NULL;
        cpl_apertures_delete(det);
        return NULL;
    }
    if (cpl_apertures_get_size(det) < ARC_MINNBARCS) {
#if defined CPL_HAVE_VA_ARGS && CPL_HAVE_VA_ARGS != 0
        cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                "Not enough valid arcs (%"CPL_SIZE_FORMAT" < %d)", 
                cpl_apertures_get_size(det), ARC_MINNBARCS);
#else
        cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                              "Not enough valid arcs, min="
                              CPL_STRINGIFY(ARC_MINNBARCS));
#endif
        cpl_image_delete(*label_im);
        *label_im = NULL;
        cpl_apertures_delete(det);
        return NULL;
    }

    /* Return  */
    return det;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Fill the image border with the given constant
  @param    self  Image of pixel-type float to be modified
  @param    xmin  Fill xmin-1  leftmost  column(s) (No fill for xmin <= 1)
  @param    ymin  Fill ymin-1  first     row(s)    (No fill for ymin <= 1)
  @param    xmax  Fill nx-xmax rightmost column(s) (No fill for xmax >= nx)
  @param    ymax  Fill ny-ymax last      row(s)    (No fill for ymax >= ny)
  @return   CPL_ERROR_NONE on success, otherwise the relevant CPL error code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code irplib_distortion_fill_border(cpl_image * self,
                                                    int         xmin,
                                                    int         ymin,
                                                    int         xmax,
                                                    int         ymax,
                                                    double      fillval)
{
    const int   nx     = cpl_image_get_size_x(self);
    const int   ny     = cpl_image_get_size_y(self);
    float     * pfi    = cpl_image_get_data_float(self);
    const float fvalue = (float)fillval;
    int         i, j;


    cpl_ensure_code(pfi != NULL, cpl_error_get_code());

    /* Ensure validity of pixel buffer access */
    xmin = IRPLIB_MIN(xmin, nx+1);
    ymax = IRPLIB_MIN(ymax, ny);

    /* - and avoid double access */
    xmax = IRPLIB_MAX(xmax, xmin - 1);
    ymin = IRPLIB_MIN(ymin, ymax + 1);

    /* Fill the zone */

    for (j = 0; j < ymin-1; j++) {
        for (i = 0; i < nx; i++) {
            pfi[i+j*nx] = fvalue;
        }
    }
    /* assert( j == IRPLIB_MAX(0, ymin-1) ); */

    for (; j < ymax; j++) {
        for (i = 0; i < xmin-1; i++) {
            pfi[i+j*nx] = fvalue;
        }
        for (i = xmax; i < nx; i++) {
            pfi[i+j*nx] = fvalue;
        }
    }
    /* assert( j == IRPLIB_MAX(0, ymax) ); */

    for (; j < ny; j++) {
        for (i = 0; i < nx; i++) {
            pfi[i+j*nx] = fvalue;
        }
    }

    return CPL_ERROR_NONE;
}

static int irplib_distortion_threshold1d(
        cpl_image   *   im,
        double          threshold,
        cpl_image   *   im1d,
        double          newval)
{
    float       *   pim;
    float       *   pim1d;
    int             nx, ny;
    int             i, j;

    /* Check entries */
    if (im == NULL) return -1;
    if (im1d == NULL) return -1;
    if (cpl_image_get_type(im) != CPL_TYPE_FLOAT) return -1;
    if (cpl_image_get_type(im1d) != CPL_TYPE_FLOAT) return -1;

    /* Get access to the im / im1d data */
    pim = cpl_image_get_data_float(im);
    pim1d = cpl_image_get_data_float(im1d);
    nx = cpl_image_get_size_x(im);
    ny = cpl_image_get_size_y(im);

    /* Apply the thresholding */
    for (i=0; i<nx; i++)
        if (pim1d[i] < threshold) {
            for (j=0; j<ny; j++) pim[i+j*nx] = (float)newval;
        }

    /* Return */
    return 0;
}

static int irplib_distortion_sub_hor_lowpass(
        cpl_image   *   im, 
        int             filt_size)
{
    cpl_vector  *   linehi;
    cpl_vector  *   linelo;
    cpl_vector  *   avglinehi;
    cpl_vector  *   avglinelo;
    double      *   pavglinehi;
    float       *   pim;
    int             lopos, hipos, nx, ny;
    int             i, j;

    /* Test entries */
    if (im == NULL) return -1;
    if (filt_size <= 0) return -1;
    
    /* Initialise */
    nx = cpl_image_get_size_x(im);
    ny = cpl_image_get_size_y(im);
    lopos = (int)(ny/4);
    hipos = (int)(3*ny/4);

    /* Get the vectors out of the image */
    if ((linehi = cpl_vector_new_from_image_row(im, hipos)) == NULL) {
        return -1;
    }
    if ((linelo = cpl_vector_new_from_image_row(im, lopos)) == NULL) {
        cpl_vector_delete(linehi);
        return -1;
    }
    
    /* Filter the vectors */
    if ((avglinehi = cpl_vector_filter_median_create(linehi, 
                    filt_size)) == NULL) {
        cpl_vector_delete(linehi);
        cpl_vector_delete(linelo);
        return -1;
    }
    cpl_vector_delete(linehi);
    
    if ((avglinelo = cpl_vector_filter_median_create(linelo, 
                    filt_size)) == NULL) {
        cpl_vector_delete(linelo);
        cpl_vector_delete(avglinehi);
        return -1;
    }
    cpl_vector_delete(linelo);

    /* Average the filtered vectors to get the low freq signal */
    cpl_vector_add(avglinehi, avglinelo);
    cpl_vector_delete(avglinelo);
    cpl_vector_divide_scalar(avglinehi, 2.0);

    /* Subtract the low frequency signal */
    pavglinehi = cpl_vector_get_data(avglinehi);
    pim = cpl_image_get_data_float(im);
    for (i=0; i<nx; i++) {
        for (j=0; j<ny; j++) {
            pim[i+j*nx] -= pavglinehi[i];
        }
    }
    cpl_vector_delete(avglinehi);

    return 0;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Purge apertures that don't look like arc lines
  @param    self         Aperture object pointer
  @param    lab_im       Label image, to be relabelled
  @param    arc_im       Image with arc lines
  @param    min_arclen   Arc line criterion
  @param    max_arcwidth Arc line criterion
  @param    arc_sat      Arc line criterion
  @return   a newly allocated clean image
 */
/*----------------------------------------------------------------------------*/
static
cpl_error_code irplib_distortion_purge_arcs(cpl_apertures  ** self,
                                            cpl_image       * lab_im,
                                            const cpl_image * arc_im,
                                            int               min_arclen,
                                            int               max_arcwidth,
                                            double            arc_sat)
{
    const double ycenter = 0.5 * (1 + cpl_image_get_size_y(arc_im));
    int   narcs;
    int   nkeep  = 0;
    int   ifirst = 1;
    int * relabel;
    int   i;

    /* Check entries */
    cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);

    /* Get number of arcs */
    narcs = cpl_apertures_get_size(*self);

    cpl_ensure_code(narcs  > 0,     CPL_ERROR_DATA_NOT_FOUND);
    cpl_ensure_code(cpl_image_get_type(lab_im) == CPL_TYPE_INT,
                    CPL_ERROR_ILLEGAL_INPUT);

    /* Allocate relabel array with default relabelling to zero */
    relabel = cpl_calloc(narcs, sizeof(int));

    /* Loop on the different arcs candidates */
    for (i = 0; i < narcs; i++) {
        /* Test if the current object is a valid arc */
        const int arclen = 1
            + cpl_apertures_get_top(*self, i+1)
            - cpl_apertures_get_bottom(*self, i+1);

        if (cpl_apertures_get_top(*self,    i+1) < ycenter) continue;
        if (cpl_apertures_get_bottom(*self, i+1) > ycenter) continue;

        if (arclen > min_arclen) {
            const int arcwidth = 1
                + cpl_apertures_get_right(*self, i+1)
                - cpl_apertures_get_left(*self, i+1);
            if (arcwidth < max_arcwidth) {
                const int edge = cpl_apertures_get_left_y(*self, i+1);
                if (edge > 0) {
                    const double mean = cpl_apertures_get_mean(*self, i+1);
                    if (mean < arc_sat) {
                        relabel[i] = ++nkeep;
                        /* Relabeling, if any, starts with ifirst */
                        if (nkeep == i+1) ifirst = nkeep;
                    }
                }
            }
        }
    }

    if (nkeep < narcs) {
        /* Update the labelised image by erasing non valid arcs */
        int     * plabim = cpl_image_get_data_int(lab_im);
        const int npix   = cpl_image_get_size_x(lab_im)
            * cpl_image_get_size_y(lab_im);

        if (nkeep == 0) {
            cpl_free(relabel);
#if defined CPL_HAVE_VA_ARGS && CPL_HAVE_VA_ARGS != 0
            return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                                         "All %d arc(s) are invalid", narcs);
#else
            return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                                         "All arcs are invalid");
#endif
        }

        for (i = 0; i < npix; i++) {
            const int label = plabim[i];

            if (label < 0 || label > narcs) break;
            if (label >= ifirst) plabim[i] = relabel[label-1];
        }

        if (i < npix) {
            /* lab_im is not a valid label image */
            cpl_free(relabel);
            return cpl_error_set(cpl_func, plabim[i] < 0
                                         ? CPL_ERROR_ILLEGAL_INPUT
                                         : CPL_ERROR_INCOMPATIBLE_INPUT);
        }

        /* Purge the bad arcs */
        cpl_apertures_delete(*self);
        *self = cpl_apertures_new_from_image(arc_im, lab_im);

    }

    cpl_free(relabel);

    cpl_msg_info(cpl_func, "Purged %d of %d arcs (1st purged=%d)", narcs - nkeep,
                 narcs, ifirst);

    /* arc_im may be invalid */
    cpl_ensure_code(*self != NULL, cpl_error_get_code());

    return CPL_ERROR_NONE;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Find the fitting points for the 2D-polynomial
  @param    grid      Positions in the image (may be shortened), (1,1) for first
  @param    fitvalues Fitting values in the image (may be shortened)
  @param    in        2D-spectrum with curvature
  @param    label_im  Corresponding label image 
  @param    det       Corresponding apertures
  @return   CPL_ERROR_NONE on success, otherwise the relevant CPL error code

  Sample arcs along all rows.
  Fit to centroid of center row, i.e. center row is fix-points of transform.

 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
irplib_distortion_fill_arc_positions(cpl_bivector        * grid,
                                     cpl_vector          * fitvalues,
                                     const cpl_image     * in,
                                     const cpl_image     * label_im,
                                     const cpl_apertures * det)
{
    const int    narcs = cpl_apertures_get_size(det);
    int          nfitvals = cpl_vector_get_size(fitvalues);
    const int    nx = cpl_image_get_size_x(label_im);
    const int    ny = cpl_image_get_size_y(label_im);
    cpl_image  * filt_img;
    cpl_mask   * kernel;
    cpl_vector * gridx = cpl_bivector_get_x(grid);
    cpl_vector * gridy = cpl_bivector_get_y(grid);
    cpl_polynomial* dist1d;
    cpl_matrix * dist1dx = NULL;
    cpl_vector * dist1dy = NULL;
    double     * dgridx;
    double     * dgridy;
    double     * dfitv;
    int          ndone = 0;
    int          i, obj;

    cpl_ensure_code(nfitvals > 0,      CPL_ERROR_DATA_NOT_FOUND);
    cpl_ensure_code(narcs    > 0,      CPL_ERROR_DATA_NOT_FOUND);
    cpl_ensure_code(cpl_image_get_type(label_im) == CPL_TYPE_INT,
                    CPL_ERROR_TYPE_MISMATCH);

    /* Ensure space for output */
    if (nfitvals < narcs * ny) {
        nfitvals = narcs * ny;
        cpl_vector_set_size(fitvalues, nfitvals);
    }
    if (cpl_vector_get_size(gridx) < nfitvals ||
        cpl_vector_get_size(gridy) < nfitvals) {
        cpl_vector_set_size(gridx, nfitvals);
        cpl_vector_set_size(gridy, nfitvals);
    }

    /* Get data after resizing */
    dgridx = cpl_vector_get_data(gridx);
    dgridy = cpl_vector_get_data(gridy);
    dfitv  = cpl_vector_get_data(fitvalues);

    /* Median filter on input image */
    kernel = cpl_mask_new(3, 3);
    cpl_mask_not(kernel);
    filt_img = cpl_image_new(nx, ny, cpl_image_get_type(in));
    cpl_image_filter_mask(filt_img, in, kernel, CPL_FILTER_MEDIAN,
                                                CPL_BORDER_FILTER);
    cpl_mask_delete(kernel);

    dist1d = cpl_polynomial_new(1);

    for (obj = 0; obj < narcs; obj++) {
        /* Find the reference X-coordinate for the arc */
        const int  * plabel_im = cpl_image_get_data_int_const(label_im);
        const int    ndist1d = cpl_apertures_get_top(det, obj+1)
            - cpl_apertures_get_bottom(det, obj+1) + 1;
        cpl_boolean sampsym = CPL_TRUE;
        int         j, prevj = 0;
        int         k = 0;

        (void)cpl_matrix_unwrap(dist1dx);
        (void)cpl_vector_unwrap(dist1dy);
        dist1dx = cpl_matrix_wrap(1, ndist1d, dgridy + ndone);
        dist1dy = cpl_vector_wrap(ndist1d, dfitv  + ndone);

        /* Find out the X coord. at all Y positions on the arc */

        for (j = cpl_apertures_get_bottom(det, obj+1)-1;
             j < cpl_apertures_get_top(det, obj+1); j++) {

            for (i = 0; i < nx; i++) {
                if (plabel_im[i + j * nx] == obj + 1) break;
            }
            if (i < nx) {
                /* Found 1st pixel of aperture obj+1 in row j+1 */
                cpl_errorstate prestate = cpl_errorstate_get();

                const double x_finepos
                    = irplib_distortion_get_row_centroid(filt_img, label_im,
                                                         i, j);
                if (!cpl_errorstate_is_equal(prestate)) {
                    irplib_error_recover(prestate, "Could not find X-position "
                                         "for line %d at y=%d (x=%d)",
                                         obj+1, j+1, i+1);
                } else if (x_finepos >= 0.0) {
                    cpl_matrix_set(dist1dx, 0, k, 1.0 + j);
                    cpl_vector_set(dist1dy, k, 1.0 + x_finepos);
                    if (k > 0 && j != 1 + prevj) sampsym = CPL_FALSE;
                    prevj = j;
                    k++;
                }
            }
        }
        if (k > 0) {
            double ref_xpos, grad;
            cpl_error_code error;
            const cpl_boolean did_drop = k != ndist1d;
            const cpl_size mindeg = 0;
            const cpl_size maxdeg = 2;

            if (did_drop) {
                /* Set correct size */
                dist1dx = cpl_matrix_wrap(1, k, cpl_matrix_unwrap(dist1dx));
                dist1dy = cpl_vector_wrap(k, cpl_vector_unwrap(dist1dy));
            }

            error = cpl_polynomial_fit(dist1d, dist1dx, &sampsym, dist1dy, NULL,
                             CPL_FALSE, &mindeg, &maxdeg);
            if (error) {
                cpl_msg_error(cpl_func, "1D-fit failed");
                break;
            }

            ref_xpos = cpl_polynomial_eval_1d(dist1d, 0.5 * (ny + 1), &grad);

            for (j = cpl_apertures_get_bottom(det, obj+1)-1;
                 j < cpl_apertures_get_top(det, obj+1); j++) {
                const double xpos = cpl_polynomial_eval_1d(dist1d, j+1.0, NULL);

                dfitv [ndone] = xpos;
                dgridx[ndone] = ref_xpos;
                /* Wrapping dist1dx does _not_ take care of dgridy,
                   in case of "Could not find X-position " */
                if (did_drop)
                    dgridy[ndone] = 1.0 + j;
                ndone++;
            }
            cpl_msg_info(cpl_func, "Line %d has center gradient %g", obj+1,
                         grad);
        }
    }

    cpl_image_delete(filt_img);
    cpl_polynomial_delete(dist1d);
    (void)cpl_matrix_unwrap(dist1dx);
    (void)cpl_vector_unwrap(dist1dy);

    cpl_msg_info(cpl_func, "Found %d fitting points ("
                 "expected up to %d points)", ndone, nfitvals);

    cpl_ensure_code(obj == narcs, cpl_error_get_code());

    cpl_ensure_code(ndone > 0, CPL_ERROR_DATA_NOT_FOUND);

    cpl_vector_set_size(fitvalues, ndone);
    cpl_vector_set_size(gridx, ndone);
    cpl_vector_set_size(gridy, ndone);

    return CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Get the X-centroid of the row y (y=0 for first)
  @param    im        Input image 
  @param    label_im  Label image 
  @param    x         Start column of the line profile (0 for first)
  @param    y         Start row (0 for first)
  @return   a newly allocated clean image
  @note All object pixels must have column positions of at least x
 */
/*----------------------------------------------------------------------------*/
static double irplib_distortion_get_row_centroid(const cpl_image * im,
                                                 const cpl_image * label_im,
                                                 int               x,
                                                 int               y)
{
    const int     nx        = cpl_image_get_size_x(im);
    const int     ny        = cpl_image_get_size_y(im);
    const int     ynx       = y * nx;
    const float * pim       = cpl_image_get_data_float_const(im);
    const int   * plabel_im = cpl_image_get_data_int_const(label_im);
    int           firstpos = -1;
    int           lastpos  = -1;
    int           maxpos   = x;
    int           objnum;
    double        wsum = 0.0;
    double        sum  = 0.0;
    double        max  = 0.0;

    cpl_ensure(pim       != NULL, cpl_error_get_code(),    -1.0);
    cpl_ensure(plabel_im != NULL, cpl_error_get_code(),    -2.0);
    cpl_ensure(x         >= 0,    CPL_ERROR_ILLEGAL_INPUT, -3.0);
    cpl_ensure(y         >= 0,    CPL_ERROR_ILLEGAL_INPUT, -4.0);
    cpl_ensure(x         <  nx,   CPL_ERROR_ILLEGAL_INPUT, -5.0);
    cpl_ensure(y         <  ny,   CPL_ERROR_ILLEGAL_INPUT, -6.0);

    max    = (double)pim[x + ynx];
    objnum = plabel_im[x + ynx];

    /* While we stay in the same object... */
    do {
        const double val = (double)pim[x + ynx];

        if (val > 0.0) { /* FIXME: Handle this exception better */
            wsum += x * val;
            sum += val;

            if (firstpos < 0) firstpos = x;
            lastpos = x;

            if (val > max) {
                max = val;
                maxpos = x;
            }
        }


        /* Next point */
        x++;

    } while (x < nx && objnum == plabel_im[x + ynx]);

    cpl_ensure(sum > 0.0, CPL_ERROR_DATA_NOT_FOUND, -7.0);

    /*
       assert( 0 <= maxpos && maxpos < nx );
       assert( objnum == plabel_im[maxpos + ynx] );
       assert( wsum >= 0.0 );
    */

    return (wsum < sum * firstpos || wsum > sum * lastpos)
        ? maxpos : wsum / sum;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Detect and remove a dark ramp in an image
  @param    in  input image 
  @return   a newly allocated clean image
 */
/*----------------------------------------------------------------------------*/
#define IS_NB_TESTPOINTS    8
#define IS_MIN_SLOPE        0.01
#define IS_MAX_SLOPE_DIF    0.075
#define IS_MAX_FIT_EDGE_DIF 0.05
#define IS_MIN_RAMP         10.0
#define IS_MAX_MNERR        13.0
#define IS_MAX_MNERR_DIF    8.0
#define IS_MAX_INTER_DIF    20.0
#define IS_SKIPZONE         2.5
#define SQR(x) ((x)*(x))
static cpl_image * irplib_distortion_remove_ramp(const cpl_image * in) 
{
    int                 ramp_present;
    const int           nx = cpl_image_get_size_x(in);
    const int           ny = cpl_image_get_size_y(in);
    const int           yhi = (int)(ny/2);
    const int           ylo = yhi - 1;
    cpl_bivector    *   testpointlo;
    double          *   testpointlo_x;
    double          *   testpointlo_y;
    cpl_bivector    *   testpointhi;
    double          *   testpointhi_x;
    double          *   testpointhi_y;
    const int           spacing = ny / (IS_SKIPZONE*IS_NB_TESTPOINTS);
    double              rampdif, fitslope;
    double          *   pol_coefhi,
                    *   pol_coeflo;
    cpl_vector      *   median;
    double          *   median_data;
    double              medianerrlo, medianerrhi;
    double              slope;
    cpl_image       *   out;
    float           *   pout;
    int                 i;

    cpl_ensure(cpl_image_get_type(in) == CPL_TYPE_FLOAT,
               CPL_ERROR_UNSUPPORTED_MODE, NULL);
                    
    if (ny < IS_SKIPZONE * IS_NB_TESTPOINTS){
#if defined CPL_HAVE_VA_ARGS && CPL_HAVE_VA_ARGS != 0
        cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                              "image has %d lines, min="
                              CPL_STRINGIFY(IS_SKIPZONE) "*"
                              CPL_STRINGIFY(IS_NB_TESTPOINTS), ny);
#else
        cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                              "image has too few lines, min="
                              CPL_STRINGIFY(IS_SKIPZONE) "*"
                              CPL_STRINGIFY(IS_NB_TESTPOINTS));
#endif
        return NULL;
    }
    
    /* Fill the vectors */
    testpointhi = cpl_bivector_new(IS_NB_TESTPOINTS);
    testpointhi_x = cpl_bivector_get_x_data(testpointhi);
    testpointhi_y = cpl_bivector_get_y_data(testpointhi);
    testpointlo = cpl_bivector_new(IS_NB_TESTPOINTS);
    testpointlo_x = cpl_bivector_get_x_data(testpointlo);
    testpointlo_y = cpl_bivector_get_y_data(testpointlo);
    for (i=0; i<IS_NB_TESTPOINTS; i++) {
        int          y;
        cpl_vector * tmp_vector;
        y = yhi + i * spacing;
        tmp_vector = cpl_vector_new_from_image_row(in, y+1);
        testpointhi_x[i] = y - ny / 2;
        testpointhi_y[i] = cpl_vector_get_median_const(tmp_vector);
        cpl_vector_delete(tmp_vector);
        y = ylo - i * spacing;
        tmp_vector = cpl_vector_new_from_image_row(in, y+1);
        testpointlo_x[IS_NB_TESTPOINTS-i-1] = y;
        testpointlo_y[IS_NB_TESTPOINTS-i-1]=cpl_vector_get_median_const(tmp_vector);
        cpl_vector_delete(tmp_vector);
    }

    /* Apply the fit */
    pol_coefhi = irplib_flat_fit_slope_robust(testpointhi_x,
            testpointhi_y, IS_NB_TESTPOINTS);
    pol_coeflo = irplib_flat_fit_slope_robust(testpointlo_x, 
            testpointlo_y, IS_NB_TESTPOINTS);

    /* Compute the errors */
    median = cpl_vector_new(IS_NB_TESTPOINTS);
    median_data = cpl_vector_get_data(median);
    for (i=0; i<IS_NB_TESTPOINTS; i++) {
        median_data[i]=SQR(testpointhi_y[i]
                - pol_coefhi[0] - pol_coefhi[1] * testpointhi_x[i]);
    }
    medianerrhi = cpl_vector_get_median(median);
    for (i=0; i<IS_NB_TESTPOINTS; i++) {
        median_data[i]=SQR(testpointlo_y[i]
                - pol_coeflo[0] - pol_coeflo[1] * testpointlo_x[i]);
    }
    medianerrlo = cpl_vector_get_median(median);
    cpl_vector_delete(median);
    rampdif = testpointlo_y[IS_NB_TESTPOINTS-1] - testpointhi_y[0];
    slope = rampdif / (ny/2.0);
    fitslope = (pol_coefhi[1] + pol_coeflo[1]) / 2.0;

    cpl_bivector_delete(testpointlo);
    cpl_bivector_delete(testpointhi);

    /* Decide if there is a ramp or not  */
    if (fabs(rampdif)<IS_MIN_RAMP ||
            fabs(pol_coefhi[1]) < IS_MIN_SLOPE ||
            fabs(pol_coeflo[1]) < IS_MIN_SLOPE ||
            pol_coefhi[1]/pol_coeflo[1]<0.5 ||
            pol_coefhi[1]/pol_coeflo[1]>2.0 ||
            fabs(pol_coefhi[1]-pol_coeflo[1])>IS_MAX_SLOPE_DIF ||
            fabs(pol_coefhi[0]-pol_coeflo[0]) > IS_MAX_INTER_DIF ||
            medianerrlo> IS_MAX_MNERR ||
            medianerrhi> IS_MAX_MNERR ||
            fabs(medianerrlo-medianerrhi) >IS_MAX_MNERR_DIF ||
            fabs(slope-fitslope) > IS_MAX_FIT_EDGE_DIF ||
            slope/fitslope<0.5 ||
            slope/fitslope>2.0) ramp_present = 0;
    else ramp_present = 1;

    cpl_free(pol_coeflo);
    cpl_free(pol_coefhi);

    /* Correct the ramp if it is there */
    out = cpl_image_duplicate(in);
    pout = cpl_image_get_data_float(out);
    if (ramp_present == 1) {
        float val;
        int   j;
        for (j=0; j<ny/2; j++) {
            val = slope * (j-ny/2);
            for (i=0; i<nx; i++)
                pout[i+j*nx] -= val;
        }
        for (j=ny/2; j<ny; j++) {
            val = slope * (j-ny);
            for (i=0; i<nx; i++)
                pout[i+j*nx] -= val;
        }

    }

    return out;
}

/*----------------------------------------------------------------------------*/
/**
  @brief  Filter the background of an image with horizontal or vertical lines
  @param  self    Filtered image
  @param  other   Image to filter, use NULL for in-place filtering
  @param  hsize   Filtering half-size, total size is 1 + 2 * hsize
  @param  vertical True iff the lines are vertical
  @return CPL_ERROR_NONE or the relevant CPL error code

  If the background varies strongly along the line, it can be detected
  and removed with a unit width 1+2*hsize median filter, where hsize is an
  upper bound on the line width.

 */
/*----------------------------------------------------------------------------*/
static cpl_error_code irplib_image_filter_background_line(cpl_image * self,
                                                   const cpl_image * other,
                                                   int hsize,
                                                   cpl_boolean vertical)
{
    const int      nx = cpl_image_get_size_x(self);
    const int      ny = cpl_image_get_size_y(self);
    const int      msize = 1 + 2 * hsize;
    cpl_mask     * mask;
    cpl_image    * background;
    cpl_error_code error = CPL_ERROR_NONE;

    cpl_ensure_code(self  != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(hsize >= 0,    CPL_ERROR_ILLEGAL_INPUT);

    if (other == NULL) other = self;

    mask = vertical ? cpl_mask_new(msize, 1) : cpl_mask_new(1, msize);

    error |= cpl_mask_not(mask);

    background = cpl_image_new(nx, ny, cpl_image_get_type(other));

    error |= cpl_image_filter_mask(background, other, mask, CPL_FILTER_MEDIAN,
                                   CPL_BORDER_FILTER);
    cpl_mask_delete(mask);

    if (self != other) {
        error |= cpl_image_copy(self, other, 1, 1);
    }

    error |= cpl_image_subtract(self, background);
    cpl_image_delete(background);

    return error ? cpl_error_set_where(cpl_func) : CPL_ERROR_NONE;
}



/**
 * @internal
 * @brief Create and compute A = B * transpose(B)
 *
 * @param self     M x N Matrix
 * @return Pointer to created M x M product matrix, or @c NULL on error.
 * @note Only the upper triangle of A is computed, while the elements
 *       below the main diagonal have undefined values.
 * @see cpl_matrix_product_normal_create()
 *
 * @error
 *   <table class="ec" align="center">
 *     <tr>
 *       <td class="ecl">CPL_ERROR_NULL_INPUT</td>
 *       <td class="ecr">
 *         Any input matrix is a <tt>NULL</tt> pointer.
 *       </td>
 *     </tr>
 *   </table>
 * @enderror
 *
 * To destroy the new matrix the function @c cpl_matrix_delete() should
 * be used.
 */

static cpl_matrix * irplib_matrix_product_normal_create(const cpl_matrix * self)
{

    double         sum;
    cpl_matrix   * product;
    const double * ai = cpl_matrix_get_data_const(self);
    double       * bwrite;
    const int      m = cpl_matrix_get_nrow(self);
    const int      n = cpl_matrix_get_ncol(self);
    int            i, j, k;


    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, NULL);

#if 0
    /* Initialize all values to zero.
       This is done to avoid access of uninitilized memory,  in case
       someone passes the matrix to for example cpl_matrix_dump(). */
    product = cpl_matrix_new(m, m);
    bwrite = cpl_matrix_get_data(product);
#else
    bwrite = (double *) cpl_malloc(m * m * sizeof(double));
    product = cpl_matrix_wrap(m, m, bwrite);
#endif

    /* The result at (i,j) is the dot-product of i'th and j'th row */
    for (i = 0; i < m; i++, bwrite += m, ai += n) {
        const double * aj;
        aj = ai; /* aj points to first entry in j'th row */
        for (j = i; j < m; j++, aj += n) {
            sum = 0.0;
            for (k = 0; k < n; k++) {
                sum += ai[k] * aj[k];
            }
            bwrite[j] = sum;
        }
    }

    return product;

}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Fit a 2D-polynomial to a 2D-surface in a least squares sense
  @param    xy_pos  Bivector  positions of the surface to fit.
  @param    values  Vector of values of the surface to fit.
  @param    degree  Positive polynomial degree
  @param    fixy    Fix-point in Y
  @param    mse     Iff mse is not null, the mean squared error on success
  @return   The fitted polynomial or NULL on error
  @see cpl_polynomial_fit_2d_createe
  @note The fix-point is not supported by cpl_polynomial_fit_2d_create()

 */
/*----------------------------------------------------------------------------*/
static cpl_error_code irplib_polynomial_fit_2d(cpl_polynomial * self,
                                               const cpl_bivector * xy_pos,
                                               const cpl_vector * values,
                                               int degree, double fixy,
                                               double * mse)
{

    const int        np = cpl_bivector_get_size(xy_pos);
    /* Number of unknowns to determine in one dimension */
    const int        nc1 = 1+degree;
    /* Number of unknowns to determine */
    /* P_{i,0} = 0, except P_{1,0} = 1 */
    const int        nc = nc1 * (1 + nc1) / 2 - nc1;
    cpl_matrix     * mv;   /* The transpose of the Vandermonde matrix */
    cpl_matrix     * mh;   /* Block-Hankel matrix, V'*V */
    cpl_matrix     * mb;
    cpl_matrix     * mx;
#ifdef IRPLIB_DISTORTION_ASSERT
    /*const double   * coeffs1d;*/
#endif
    double         * dmv;
    cpl_vector     * xhat;
    cpl_vector     * yhat;
    cpl_vector     * zhat;
    cpl_size         powers[2];
    int              degx, degy;
    int              i, j;
    cpl_error_code   error;
   

    cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(cpl_polynomial_get_dimension(self) == 2,
                    CPL_ERROR_INVALID_TYPE);
    cpl_ensure_code(np > 0,         cpl_error_get_code());
    cpl_ensure_code(values != NULL, CPL_ERROR_NULL_INPUT);

    cpl_ensure_code(cpl_vector_get_size(values) == np,
                    CPL_ERROR_INCOMPATIBLE_INPUT);

    cpl_ensure_code(degree > 0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(np >= nc,   CPL_ERROR_DATA_NOT_FOUND);

    /* transform zero-point to fixy */
    yhat = cpl_vector_duplicate(cpl_bivector_get_y_const(xy_pos));
    cpl_vector_subtract_scalar(yhat, fixy);

    /* - and ensure P(y) = y on center line */
    xhat = cpl_vector_duplicate(cpl_bivector_get_x_const(xy_pos));
    zhat = cpl_vector_duplicate(values);
    cpl_vector_subtract(zhat, xhat);

    /* Initialize matrices */
    /* mv contains the polynomial terms in the order described */
    /* above in each row, for each input point. */
    dmv = (double*)cpl_malloc(nc*np*sizeof(double));
    mv = cpl_matrix_wrap(nc, np, dmv);

    /* Has redundant FLOPs, appears to improve accuracy */
    for (i=0; i < np; i++) {
        const double x = cpl_vector_get(xhat, i);
        const double y = cpl_vector_get(yhat, i);
        double yvalue = y;
        j = 0;
        for (degy = 1; degy <= degree; degy++) {
            double xvalue = 1;
            for (degx = 0; degx <= degree-degy; degx++, j++) {
                dmv[np * j + i] = xvalue * yvalue;
                xvalue *= x;
            }
            yvalue *= y;
        }
        /* cx_assert( j == nc ); */
    }
    cpl_vector_delete(xhat);
    cpl_vector_delete(yhat);

    /* mb contains the values, it is not modified */
    mb = cpl_matrix_wrap(np, 1, cpl_vector_get_data(zhat));

    /* Form the right hand side of the normal equations */
    mx = cpl_matrix_product_create(mv, mb);

    cpl_matrix_unwrap(mb);
    cpl_vector_delete(zhat);

    /* Form the matrix of the normal equations */
    mh = irplib_matrix_product_normal_create(mv);
    cpl_matrix_delete(mv);

    /* Solve XA=B by a least-square solution (aka pseudo-inverse). */
    error = cpl_matrix_decomp_chol(mh) || cpl_matrix_solve_chol(mh, mx);

    cpl_matrix_delete(mh);

    if (error) {
        cpl_matrix_delete(mx);
        cpl_ensure_code(0, error);
    }

    /* Store coefficients for output */

#ifdef IRPLIB_DISTORTION_ASSERT
    /*coeffs1d = cpl_matrix_get_data(mx);*/
#endif

    j = 0;
    for (degy = 1; degy <= degree; degy++) {
        powers[1] = degy;
        for (degx = 0; degx <= degree-degy; degx++, j++) {
            powers[0] = degx;
            /* cx_assert( coeffs1d[j] == cpl_matrix_get(mx, j, 0) ); */
            cpl_polynomial_set_coeff(self, powers, cpl_matrix_get(mx, j, 0));
        }
    }
    /* cx_assert( j == nc ); */

    cpl_matrix_delete(mx);

    /* P_{1,0} = 1 */
    powers[0] = 1;
    powers[1] = 0;
    cpl_polynomial_set_coeff(self, powers, 1.0);

    /* Transform the polynomial back in Y */
    cpl_polynomial_shift_1d(self, 1, -fixy);

    /* If requested, compute mean squared error */
    if (mse != NULL) {
        const cpl_vector * x_pos = cpl_bivector_get_x_const(xy_pos);
        const cpl_vector * y_pos = cpl_bivector_get_y_const(xy_pos);
        cpl_vector * x_val = cpl_vector_new(2);

        *mse = 0;
        for (i=0; i<np; i++) {
            double residue;
            cpl_vector_set(x_val, 0, cpl_vector_get(x_pos, i));
            cpl_vector_set(x_val, 1, cpl_vector_get(y_pos, i));
            /* Subtract from the true value, square, accumulate */
            residue = cpl_vector_get(values, i)
                - cpl_polynomial_eval(self, x_val);
            *mse += residue * residue;
        }
        cpl_vector_delete(x_val);
        /* Average the error term */
        *mse /= np;
    }

    return CPL_ERROR_NONE;
}

