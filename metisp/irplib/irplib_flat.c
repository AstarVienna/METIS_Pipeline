/* $Id: irplib_flat.c,v 1.15 2007-08-07 12:15:41 llundin Exp $
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
 * $Author: llundin $
 * $Date: 2007-08-07 12:15:41 $
 * $Revision: 1.15 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "irplib_flat.h"
#include <math.h>

/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

static double * irplib_flat_fit_proportional(double *, double *, int) ;

/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_flat    Functions for flatfielding 
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute a flat-field out of a set of exposures.
  @param    raw     Input image set 
  @param    mode    0 for proportional, 1 for robust fit
  @return   1 newly allocated set of 2 or 3 images 

  The input is assumed to be a cube containing planes of different intensities 
  (usually increasing or decreasing). Typical inputs are: twilight data sets, 
  halogen lamp, or skies of different airmasses in the thermal regime.

  The input image list must be of type float.
  
  In robust mode, the output is a set of 3 images. 
  The first image contains a regression map, i.e. for each pixel position on 
  the detector, a curve is plotted of the pixel intensity in each plane 
  against the median intensity of the plane. A slope is fit, and the gain 
  factor is stored into this first image.

  The second image contains the y-intercepts of the slope fit. It is usually 
  good to check it out in case of failures.

  The third image contains the sum of squared errors for each fit.
  The fit is using a robust least-squares criterion rejecting outliers. This 
  is the algorithm to use with big telescopes like the VLT, which collect so 
  much light that objects are actually seen in the twilight sky.  

  In proportional mode, the output is a set of 2 images.
  The first image contains a regression map.
  The second image contains the sum of squared errors for each fit.
 */
/*----------------------------------------------------------------------------*/
cpl_imagelist * irplib_flat_fit_set(
        cpl_imagelist   *   raw,
        int                 mode)
{
    double          *   plane_med    = NULL ;
    double          *   slope        = NULL ;
    cpl_image       *   gain         = NULL ;
    double          *   pgain        = NULL ;
    cpl_image       *   intercept    = NULL ;
    double          *   pintercept   = NULL ;
    cpl_image       *   sq_err       = NULL ;
    double          *   psq_err      = NULL ;
    double          *   timeline     = NULL ;
    float           *   raw_im_data  = NULL ;
    cpl_imagelist   *   result       = NULL ;
    const int           nx = cpl_image_get_size_x(cpl_imagelist_get(raw, 0));
    const int           ny = cpl_image_get_size_y(cpl_imagelist_get(raw, 0));
    const int           ni = cpl_imagelist_get_size(raw);
    int                 i, j ;

    /* Check entries */
    if (raw==NULL) return NULL ;
    if ((mode != 0) && (mode != 1)) return NULL ;
    if (cpl_image_get_type(cpl_imagelist_get(raw, 0)) != CPL_TYPE_FLOAT)
        return NULL ;
    if (cpl_imagelist_get_size(raw) <= 1) return NULL ;

    /* Compute median for all planes */
    plane_med = cpl_malloc(ni * sizeof(double)) ;
    for (i=0 ; i<ni ; i++)
        plane_med[i] = cpl_image_get_median(cpl_imagelist_get(raw, i));

    /* Create result images */
    gain = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE) ;
    pgain = cpl_image_get_data_double(gain) ;
    if (mode == 1) {
        intercept = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE) ;
        pintercept = cpl_image_get_data_double(intercept) ;
    }
    sq_err = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE) ;
    psq_err = cpl_image_get_data_double(sq_err) ;
    timeline = cpl_malloc(ni * sizeof(double)) ;

    /* Loop on all pixel positions */
    cpl_msg_info(cpl_func, "Computing gains for all positions (long)...") ;
    for (i=0 ; i<nx * ny ; i++) {
        /* extract time line */
        for (j=0 ; j<ni ; j++) {
            raw_im_data = cpl_image_get_data_float(cpl_imagelist_get(raw, j)) ;
            timeline[j] = (double)raw_im_data[i] ;
        }
        /* Fit slope to this time line */
        if (mode == 1) {
            slope = irplib_flat_fit_slope_robust(plane_med, timeline, ni) ;
            pintercept[i] = slope[0] ;
            pgain[i]      = slope[1] ;
            psq_err[i]    = slope[2] ;
            /* Set results in output images */
        } else {
            slope = irplib_flat_fit_proportional(plane_med, timeline, ni) ;
            /* Set results in output images */
            pgain[i]      = slope[0] ;
            psq_err[i]    = slope[1] ;
        }
        cpl_free(slope);
    }
    cpl_free(plane_med) ;
    cpl_free(timeline) ;

    /* Return */
    result = cpl_imagelist_new() ;
    if (mode == 1) {
        cpl_imagelist_set(result, gain, 0) ;
        cpl_imagelist_set(result, intercept, 1) ;
        cpl_imagelist_set(result, sq_err, 2) ;
    } else {
        cpl_imagelist_set(result, gain, 0) ;
        cpl_imagelist_set(result, sq_err, 1) ;
    }
    return result ;
}

/* @cond */
#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))
#define MAX_ITERATE     30
/* @endcond */
/*----------------------------------------------------------------------------*/
/**
  @brief    Fit a slope to a list of points (robust fit).
  @param    x   x coordinates 
  @param    y   y coordinates 
  @param    np  number of points 
  @return   Pointer to newly allocated array of 3 doubles.

  The slope to fit has the following kind of equation:
  y = c[0] + c[1] * x

  The returned coefficients are defined as:
  c[0] is the y-intercept.
  c[1] is the slope.
  c[2] is the median squared error of the fit.
  This is a very robust slope fit. It tolerates up to 50% of outliers in input.
 */
/*----------------------------------------------------------------------------*/
double * irplib_flat_fit_slope_robust(
        double  *   x,
        double  *   y,
        int         np)
{
    double      *   c ;
    double          aa, bb, bcomp, b1, b2, del, abdevt, f1, f2, sigb,
                    d, sum ;
    double          sx, sy, sxy, sxx, chisq ;
    cpl_vector  *   arr ;
    double      *   parr ;
    double          aa_ls, bb_ls ;
    int             iter ;
    int             i ;

    /* Check entries */
    if (x==NULL || y==NULL) return NULL ;

    c = cpl_malloc(3 * sizeof(double)) ;

    sx = sy = sxx = sxy = 0.00 ;
    for (i=0 ; i<np ; i++) {
        sx  += x[i];
        sy  += y[i];
        sxy += x[i] * y[i];
        sxx += x[i] * x[i];
    }

    del = np * sxx - sx * sx;
    aa_ls = aa  = (sxx * sy - sx * sxy) / del;
    bb_ls = bb  = (np * sxy - sx * sy) / del;

    chisq = 0.00 ;
    for (i=0;i<np;i++) {
        double temp = y[i] - (aa+bb*x[i]) ;
        temp *= temp ;
        chisq += temp ;
    }

    arr = cpl_vector_new(np) ;
    parr = cpl_vector_get_data(arr) ;
    sigb = sqrt(chisq/del);
    b1   = bb ;

    bcomp = b1 ;
    sum = 0.00 ;
    for (i=0 ; i<np ; i++) {
            parr[i] = y[i] - bcomp * x[i];
        }
    aa = cpl_vector_get_median(arr); /* arr permuted */
    abdevt = 0.0;
    for (i=0 ; i<np ; i++) {
        d = y[i] - (bcomp * x[i] + aa);
        abdevt += fabs(d);
        if (fabs(y[i]) > 1e-7) d /= fabs(y[i]);
        if (fabs(d) > 1e-7) sum += (d >= 0.0 ? x[i] : -x[i]);
    }
    f1 = sum ;
    b2   = bb + SIGN(3.0 * sigb, f1);
    bcomp = b2 ;
    sum = 0.00 ;
    for (i=0 ; i<np ; i++) parr[i] = y[i] - bcomp * x[i];
    aa = cpl_vector_get_median(arr);  /* arr permuted */
    abdevt = 0.0;
    for (i=0 ; i<np ; i++) {
        d = y[i] - (bcomp * x[i] + aa);
        abdevt += fabs(d);
        if (fabs(y[i]) > 1e-7) d /= fabs(y[i]);
        if (fabs(d) > 1e-7) sum += (d >= 0.0 ? x[i] : -x[i]);
    }
    f2 = sum ;

    if (fabs(b2-b1)<1e-7) {
        c[0] = aa ;
        c[1] = bb ;
        c[2] = abdevt / (double)np;
        cpl_vector_delete(arr);
        return c ;
    }

    iter = 0 ;
    while (f1*f2 > 0.0) {
        bb = 2.0*b2-b1;
        b1 = b2;
        f1 = f2;
        b2 = bb;

        bcomp = b2 ;
        sum = 0.00 ;
        for (i=0 ; i<np ; i++) parr[i] = y[i] - bcomp * x[i];
        aa = cpl_vector_get_median(arr); /* arr permuted */
        abdevt = 0.0;
        for (i=0 ; i<np ; i++) {
            d = y[i] - (bcomp * x[i] + aa);
            abdevt += fabs(d);
            if (fabs(y[i]) > 1e-7) d /= fabs(y[i]);
            if (fabs(d) > 1e-7) sum += (d >= 0.0 ? x[i] : -x[i]);
        }
        f2 = sum ;
        iter++;
        if (iter>=MAX_ITERATE) break ;
    }
    if (iter>=MAX_ITERATE) {
        c[0] = aa_ls ;
        c[1] = bb_ls ;
        c[2] = -1.0 ;
        cpl_vector_delete(arr);
        return c ;
    }

    sigb = 0.01 * sigb;
    while (fabs(b2-b1) > sigb) {
        double f;
        bb = 0.5 * (b1 + b2) ;
        if ((fabs(bb-b1)<1e-7) || (fabs(bb-b2)<1e-7)) break;
        bcomp = bb ;
        sum = 0.00 ;
        for (i=0 ; i<np ; i++) parr[i] = y[i] - bcomp * x[i];
        aa = cpl_vector_get_median(arr); /* arr permuted */
        abdevt = 0.0;
        for (i=0 ; i<np ; i++) {
            d = y[i] - (bcomp * x[i] + aa);
            abdevt += fabs(d);
            if (fabs(y[i]) > 1e-7) d /= fabs(y[i]);
            if (fabs(d) > 1e-7) sum += (d >= 0.0 ? x[i] : -x[i]);
        }
        f = sum ;

        if (f*f1 >= 0.0) {
            f1=f;
            b1=bb;
        } else {
            /*f2=f;*/
            b2=bb;
        }
    }
    cpl_vector_delete(arr) ;
    c[0]=aa;
    c[1]=bb;
    c[2]=abdevt/np;
    return c ;
}
#undef MAX_ITERATE
#undef SIGN


/**@}*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute a=y/x for all given points
  @param    x   x coordinates 
  @param    y   y coordinates 
  @param    np  number of points 
  @return   Pointer to newly allocated array of two doubles.

  This function takes in input a list of points supposed all aligned
  on a slope going through the origin (of equation y=ax). It computes
  the slope a = y/x for all points, and returns a pointer to two
  doubles:

  \begin{itemize}
  \item The median slope.
  \item The mean squared error.
  \end{itemize}

  Returning the median of all slopes makes it very robust to outliers.
  A more precise method would be to make a histogram of all slopes and
  take the maximum (i.e. the mode of the distribution). It can be
  shown that the median approximates the mode quite well for a large
  number of points.
 */
/*----------------------------------------------------------------------------*/
#define FITPROP_BIG_SLOPE   1e30
static double * irplib_flat_fit_proportional(
        double  *   x,
        double  *   y,
        int         np)
{
    cpl_vector  *   slopes ;
    double      *   pslopes ;
    double      *   med_slope ;
    double          sq_err ;
    int             i ;

    /* Check entries */
    if (x==NULL || y==NULL) return NULL ;

    slopes = cpl_vector_new(np) ;
    pslopes = cpl_vector_get_data(slopes) ;
    for (i=0 ; i<np ; i++) {
        if (fabs(x[i])>1e-30)  pslopes[i] = y[i] / x[i] ;
        else                   pslopes[i] = FITPROP_BIG_SLOPE ;
    }
    med_slope = cpl_malloc(2 * sizeof(double));
    med_slope[0] = cpl_vector_get_median(slopes); /* slopes permuted */
    cpl_vector_delete(slopes);

    sq_err = 0.00 ;
    for (i=0 ; i<np ; i++) {
        double val = med_slope[0] * x[i] ;
        sq_err += (val-y[i])*(val-y[i]) ;
    }
    sq_err /= (double)np ;
    med_slope[1] = sq_err ;

    return med_slope ;
#undef FITPROP_BIG_SLOPE
}



