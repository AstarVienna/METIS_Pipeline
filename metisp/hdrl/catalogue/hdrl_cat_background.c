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

#include "hdrl_cat_background.h"

#include "hdrl_cat_filter.h"
#include "hdrl_cat_utils_sort.h"

#include "hdrl_image.h"


/*** Prototypes ***/

static cpl_image * hdrl_sigclipfilter_image_grid(
	const cpl_image *ima, cpl_matrix *x, cpl_matrix *y,
	cpl_size filtersize_x, cpl_size filtersize_y);

static cpl_matrix * matrix_linspace(cpl_size start, cpl_size stop, cpl_size step);


/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_background    hdrl_background
 * @ingroup  Catalogue
 *
 * @brief    Model and create background map
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Model and create background map
 *
 * @param    ap        The current ap structure
 * @param    nbsize    The size of the cells in pixels
 * @param    bkg_subtr switch: if bkg_subtr !=0 fill background map
 * @param    res
 *
 * @return   CPL_ERROR_CODE if all went well (this is currently the only value).
 *
 * Description:
 * 	    The image data array is split into cells. In each cell a robust
 *      background estimate is obtained. The cell raster is gently smoothed
 *      and then used to create a full background map with a bi-linear
 *      interpolation scheme.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_background(
		ap_t *ap, cpl_size nbsize, cpl_size bkg_subtr, hdrl_casu_result *res)
{
    /* Set up some variables */
    double        *map   = ap->indata;
    cpl_size      nx     = ap->lsiz;
    cpl_size      ny     = ap->csiz;
    unsigned char *mflag = ap->mflag;

    /* check to see if nbsize is close to exact divisor */
    nbsize = CPL_MIN(CPL_MIN(nx, ny), nbsize);

    double    fracx = (double)nx / (double)nbsize;
    double    fracy = (double)ny / (double)nbsize;

    cpl_size ifracx = (cpl_size)(fracx + 0.1);
    cpl_size ifracy = (cpl_size)(fracy + 0.1);

    cpl_size nbsizx = nx / ifracx;
    cpl_size nbsizy = ny / ifracy;

    /* trap for small maps */
    double a = 0.9 * nbsize;
    cpl_size aux = (cpl_size)(a + (a < 0 ? -0.5 : 0.5));
    nbsize = CPL_MAX(aux, CPL_MIN(nbsize, CPL_MIN(nbsizx, nbsizy)));
    nbsize = CPL_MIN( nx, CPL_MIN(ny,     nbsize                 ));

    /* Divide the map into partitions */
    cpl_size nbx = nx / nbsize;
    cpl_size nby = ny / nbsize;

    /* Same for background values array */
    double **bvals = cpl_malloc(nby * sizeof(double *));
    for (cpl_size l = 0; l < nby; l++) {
        bvals[l] = cpl_malloc(nbx * sizeof(double));
    }

    /* Store some of this away for use later */
    ap->backmap.nbx    = nbx;
    ap->backmap.nby    = nby;
    ap->backmap.nbsize = nbsize;
    ap->backmap.bvals  = bvals;

    /* create cpl image with mask from raw data */
    cpl_image  *image           = cpl_image_wrap_double(nx, ny, map);
    cpl_mask   *image_mask      = cpl_image_get_bpm(image);
    cpl_binary *image_mask_data = cpl_mask_get_data(image_mask);

    /* Add additional bad pixels if present*/
    for (cpl_size i = 0; i < nx * ny ; i++){
        if (   mflag[i] == MF_ZEROCONF
			|| mflag[i] == MF_STUPID_VALUE
			|| mflag[i] == MF_SATURATED )
        {
            image_mask_data[i] = CPL_BINARY_1;
        }
    }

    /* sigclip stepped grid */
    cpl_size   steps_x       = nbx;
    cpl_size   steps_y       = nby;
    cpl_size   sx            = CPL_MAX(nx / steps_x, 1);
    cpl_size   sy            = CPL_MAX(ny / steps_y, 1);
    cpl_matrix *x            = matrix_linspace(sx / 2, nx, sx);
    cpl_matrix *y            = matrix_linspace(sy / 2, ny, sy);
    cpl_size   filter_size_x = nbsize / 2;
    cpl_size   filter_size_y = nbsize / 2;
    cpl_image  *imgtmp_mod   = hdrl_sigclipfilter_image_grid(image, x, y, filter_size_x, filter_size_y);

    /* interpolate remaining bad pixels */
    cpl_detector_interpolate_rejected(imgtmp_mod);

    cpl_matrix_delete(x);
    cpl_matrix_delete(y);

    for (cpl_size l = 0; l < nby; l++) {
        for (cpl_size j = 0; j < nbx; j++) {
            int rej;
            bvals[l][j] = cpl_image_get(imgtmp_mod, j + 1, l + 1, &rej);
        }
    }

    cpl_image_delete(imgtmp_mod);
    cpl_image_unwrap(image);

    /* filter raw background values */
    hdrl_bfilt(bvals, nbx, nby);

    /* compute average sky level */
    double *work = cpl_malloc(nbx * nby * sizeof(double));

    cpl_size k = 0;
    for (cpl_size l = 0; l < nby; l++) {
        for (cpl_size j = 0; j < nbx; j++) {
            work[k++] = bvals[l][j];
        }
    }

    sort_array((void *)work, k, sizeof(*work), HDRL_SORT_DOUBLE, CPL_SORT_ASCENDING);
    double avsky = work[(k) / 2];
    cpl_free(work);

    /* ok now correct map for background variations and put avsky back on */
    cpl_size nbsizo2 = nbsize / 2;
    double   fnbsize = 1. / (double)nbsize;
    for (k = 0; k < ny; k++) {

    	cpl_size kk = k * nx;

        /* Nearest background pixel vertically */
        cpl_size iby   = (k + 1 + nbsizo2) / nbsize;
        cpl_size ibyp1 = iby + 1;
        iby            = CPL_MIN(nby, CPL_MAX(1, iby));
        ibyp1          = CPL_MIN(nby, ibyp1);

        double dely = (k + 1. - nbsize*iby + nbsizo2) * fnbsize;

        for (cpl_size j = 0; j < nx; j++) {

            /* nearest background pixel across */
            cpl_size ibx   = (j + 1 + nbsizo2) / nbsize;
            cpl_size ibxp1 = ibx + 1;
            ibx            = CPL_MIN(nbx, CPL_MAX(1, ibx));
            ibxp1          = CPL_MIN(nbx, ibxp1);

            double delx = (j + 1. - nbsize * ibx + nbsizo2) * fnbsize;

            /* bilinear interpolation to find background */
            double t1   = (1. - dely) * bvals[iby - 1][ibx   - 1] + dely * bvals[ibyp1 - 1][ibx   - 1];
            double t2   = (1. - dely) * bvals[iby - 1][ibxp1 - 1] + dely * bvals[ibyp1 - 1][ibxp1 - 1];
            double dsky = avsky - (1. - delx) * t1 - delx * t2;

            if (bkg_subtr) {

                map[kk+j] += dsky;

                /* Fill the background map */
                if (res->background) {
                    cpl_image_set(res->background, j + 1, k + 1, avsky - dsky);
                }
            }
        }
    }

    return CPL_ERROR_NONE;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Work out robust background estimate over a whole input image
 *
 * @param    ap        The current ap structure
 * @param    skymed    Output sky median
 * @param    skysig    Output sky noise
 *
 * @return   CPL_ERROR_NONE if all went well and CPL_ERROR_ILLEGAL_INPUT
 *           if there aren't enough good values to do the calculation.
 *
 * Description:
 * 	    The image is analysed to work out a robust estimate of the background
 *      median, and sigma.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_backstats(
		ap_t *ap, double *skymed, double *skysig)
{
    /* Get some info from the ap structure */
    double        *map   = ap->indata;
    cpl_size      nx     = ap->lsiz;
    cpl_size      ny     = ap->csiz;
    unsigned char *mflag = ap->mflag;

    cpl_image  *ima_wrp         = cpl_image_wrap_double(nx, ny, map);
    cpl_mask   *image_mask      = cpl_image_get_bpm(ima_wrp);
    cpl_binary *image_mask_data = cpl_mask_get_data(image_mask);

    /* Add additional bad pixels if present*/
    for (cpl_size i = 0; i < nx * ny ; i++){
        if (   mflag[i] == MF_ZEROCONF
			|| mflag[i] == MF_STUPID_VALUE
			|| mflag[i] == MF_SATURATED )
        {
            image_mask_data[i] = CPL_BINARY_1;
        }
    }

    /* do kappa-sigma clipping to thresh out basic outliers */
    cpl_size rej_new;

    cpl_size niter = 30;
    for (cpl_size i = 0; i < niter; i++ ) {

        double mad;
        double median     = cpl_image_get_mad(ima_wrp, &mad);
        double stdev      = mad * CPL_MATH_STD_MAD;

		double kappa_low  = 2.5;
		double lo_cut     = median - kappa_low * stdev;

		double kappa_high = 2.5;
        double hi_cut     = median + kappa_high * stdev;

        cpl_size rej_orig = cpl_image_count_rejected(ima_wrp);

        if (lo_cut < hi_cut) {
            cpl_mask_threshold_image(image_mask, ima_wrp, lo_cut, hi_cut, CPL_BINARY_0);
        }
        rej_new = cpl_image_count_rejected(ima_wrp);

        if (rej_orig == rej_new){
        	break;
        }
    }

    /* Set the final answer. Here the normal mean and standard deviation is used
     * as all outliers should be masked and thus the mean is more accurate */
    cpl_error_code e;
    if (rej_new ==  nx * ny) {
        *skymed = 0.;
        *skysig = 0.;
        e       =  CPL_ERROR_ILLEGAL_INPUT;
    } else {
        *skymed = cpl_image_get_mean( ima_wrp);
        *skysig = cpl_image_get_stdev(ima_wrp);
        e       = CPL_ERROR_NONE;
    }

    cpl_image_unwrap(ima_wrp);

    return e;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Work out estimated sky for a pixel position
 *
 * @param    ap        The current ap structure
 * @param    x         The X position in question
 * @param    y         The Y position in question
 * @param    skylev    Output sky level at x, y
 * @param    skyrms    Output sky noise at x, y
 *
 * Description:
 * 	    Given the coarse background grid, calculate the background at a
 *      given image pixel position by doing a bi-linear interpolation of
 *      it's position within the grid.
 */
/* ---------------------------------------------------------------------------*/
void hdrl_backest(
		ap_t *ap, double x, double y, double *skylev, double *skyrms)
{
    /* Define some local variables */
	cpl_size nbx     = ap->backmap.nbx;
	cpl_size nby     = ap->backmap.nby;
	cpl_size nbsize  = ap->backmap.nbsize;
    double   **bvals = ap->backmap.bvals;

    /* Get closest pixel to the input location */
    cpl_size i = (cpl_size)(x + (x < 0 ? -0.5 : 0.5));
    cpl_size j = (cpl_size)(y + (y < 0 ? -0.5 : 0.5));

    /* Now, work out where in the map to do the interpolation */
    cpl_size nbsizo2 = nbsize / 2;

    cpl_size ibx     = (i + nbsizo2) / nbsize;
    cpl_size ibxp1   = ibx + 1;
    ibx              = CPL_MIN(nbx, CPL_MAX(1, ibx));
    ibxp1            = CPL_MIN(nbx, ibxp1);

    cpl_size iby     = (j + nbsizo2) / nbsize;
    cpl_size ibyp1   = iby + 1;
    iby              = CPL_MIN(nby, CPL_MAX(1, iby));
    ibyp1            = CPL_MIN(nby, ibyp1);

    double fnbsize   = 1. / (double)nbsize;
    double delx      = (i - nbsize * ibx + nbsizo2) * fnbsize;
    double dely      = (j - nbsize * iby + nbsizo2) * fnbsize;

    /* Now do a linear interpolation to find the background. Calculate MAD of
     * the four adjacent background cells as an estimate of the RMS */

    double t1 = (1. - dely) * bvals[iby - 1][ibx   - 1] + dely * bvals[ibyp1 - 1][ibx   - 1];
    double t2 = (1. - dely) * bvals[iby - 1][ibxp1 - 1] + dely * bvals[ibyp1 - 1][ibxp1 - 1];

    *skylev  = (1. - delx) * t1 + delx * t2;

    *skyrms  = 0.25 * (  fabs(bvals[iby   - 1][ibx   - 1] - *skylev)
                       + fabs(bvals[ibyp1 - 1][ibx   - 1] - *skylev)
                       + fabs(bvals[iby   - 1][ibxp1 - 1] - *skylev)
                       + fabs(bvals[ibyp1 - 1][ibxp1 - 1] - *skylev)
					  );
}

/**@}*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief filter image on a grid
 *
 * @param ima           image to filter
 * @param x             row vector of coordinates to filter on
 * @param y             row vector of coordinates to filter on
 * @param filtersize_x  size of the median filter
 * @param filtersize_y  size of the median filter
 *
 * @return filtered image of size of the grid
 *
 */
/* ---------------------------------------------------------------------------*/
static cpl_image * hdrl_sigclipfilter_image_grid(
	const cpl_image *ima, cpl_matrix *x, cpl_matrix *y,
	cpl_size filtersize_x, cpl_size filtersize_y)
{
    cpl_error_ensure(ima != NULL, CPL_ERROR_NULL_INPUT,
    		return NULL, "NULL input image");

    cpl_error_ensure(filtersize_x > 0 && filtersize_y > 0 , CPL_ERROR_INCOMPATIBLE_INPUT,
    		return NULL,  "All function parameters must be greater then Zero");

    const cpl_size nx      = cpl_image_get_size_x(ima);
    const cpl_size ny      = cpl_image_get_size_y(ima);

    const cpl_size steps_x = cpl_matrix_get_nrow(x);
    const cpl_size steps_y = cpl_matrix_get_nrow(y);

    cpl_image *ima_local = cpl_image_new(steps_x, steps_y, CPL_TYPE_DOUBLE);
    cpl_image_get_bpm(ima_local);

    HDRL_OMP(omp parallel for)
    for (cpl_size iy = 0; iy < steps_y; iy++) {

        cpl_size middlep_y = cpl_matrix_get(y, iy, 0);

        for (cpl_size ix = 0; ix < steps_x; ix++) {

            cpl_size middlep_x = cpl_matrix_get(x, ix, 0);

            cpl_size lowerlimit_x = CPL_MAX(middlep_x - filtersize_x, 1 );
            cpl_size lowerlimit_y = CPL_MAX(middlep_y - filtersize_y, 1 );
            cpl_size upperlimit_x = CPL_MIN(middlep_x + filtersize_x, nx);
            cpl_size upperlimit_y = CPL_MIN(middlep_y + filtersize_y, ny);

            cpl_image  *ima_cut      = cpl_image_extract(ima, lowerlimit_x, lowerlimit_y, upperlimit_x, upperlimit_y);
            hdrl_image *ima_cut_hdrl = hdrl_image_create(ima_cut, NULL);
            hdrl_value median        = hdrl_image_get_sigclip_mean(ima_cut_hdrl, 2.5 ,2.5 ,3);

            cpl_image_set(ima_local, ix + 1, iy + 1, median.data);

            /* reject also if less than one 4th bad pixels as in original catalogue code */
            if (   isnan(median.data)
                || cpl_image_count_rejected(ima_cut) >= 0.25 * 2 * (filtersize_x * filtersize_y))
            {
                cpl_image_reject(ima_local, ix + 1, iy + 1);
            }

            cpl_image_delete(ima_cut);
            hdrl_image_delete(ima_cut_hdrl);
        }
    }

    return ima_local;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief create linear space row vector
 * @param start  starting point
 * @param stop   end point, exclusive
 * @param step   step size
 *
 * @returns matrix with one row filled equally spaced points from start to end
 *
 */
/* ---------------------------------------------------------------------------*/
static cpl_matrix * matrix_linspace(cpl_size start, cpl_size stop, cpl_size step)
{
    cpl_matrix * x = cpl_matrix_new(stop / step, 1);
    for (intptr_t i = 0; start + i * step < stop && i < stop / step; i++) {
        cpl_matrix_set(x, i, 0, start + i * step);
    }
    return x;
}
