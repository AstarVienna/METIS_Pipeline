/*
 * This file is part of the irplib package
 * Copyright (C) 2002,2003,2014 European Southern Observatory
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* The IRPLIB-based application may have checked for the availability of
   memrchr() in which case the macro HAVE_DECL_MEMRCHR is defined as either
   0 or 1. Without checks it is assumed that the function is not available.
   With a suitable version of autoconf the macro can be defined with this
   entry in configure.ac:
   AC_CHECK_DECLS([memrchr])
*/

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "irplib_slitpos.h"
#include "irplib_flat.h"

#include <cpl.h>

#include <string.h>
#include <math.h>
#include <assert.h>

/*-----------------------------------------------------------------------------
                                Defines
 -----------------------------------------------------------------------------*/

#ifndef IRPLIB_SLITPOS_KERNEL_SIZE_Y
#define IRPLIB_SLITPOS_KERNEL_SIZE_Y      5
#endif

#ifndef IRPLIB_SLITPOS_MAX_EROSION
#define IRPLIB_SLITPOS_MAX_EROSION     1024
#endif

/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

static cpl_error_code irplib_slitpos_find_edges_one_line(const cpl_image *,
                                                         int, int *, int *);
static cpl_error_code irplib_slitpos_find_vert_slit_ends(const cpl_image *,
                                                         int, int *, int *);
static cpl_error_code irplib_slitpos_find_vert_pos(const cpl_image *, int,
                                                   cpl_size *);
static cpl_error_code irplib_image_filter_background_line(cpl_image *,
                                                   const cpl_image *,
                                                   int, cpl_boolean) ;

/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_slitpos     Functions for slit position
 */
/*----------------------------------------------------------------------------*/

/**@{*/
/*----------------------------------------------------------------------------*/
/**
  @brief    Detect the slit position, detect its ends, extract a thin image
            containing only the slit and find its edges
  @param    imslit          Input image with a vertical slit
  @param    slit_max_width  Maximum slit width
  @param    slit_flux       Some of the pixels values of the slit
  @return   the table with the slit position or NULL on error

  On success the created table contains rows of four columns labeled:
     "SLIT_Y"      (int)
     "SLIT_LEFT"   (double)
     "SLIT_CENTER" (double)
     "SLIT_RIGHT"  (double)

  This function returns a table with 3 columns:

  - Left or Lower edge of the slit
  - Center of the slit
  - Right or Upper edge of the slit

  Additionally, the slit flux is computed. The passed slit_flux pointer
  parameter can be NULL. In error case, its value is undefined.
  
  NB: Coordinates use FITS convention.
 */
/*----------------------------------------------------------------------------*/
cpl_table * irplib_slitpos_analysis(const cpl_image * imslit,
                                    int               slit_max_width,
                                    double          * slit_flux)
{
    const int       size_x = cpl_image_get_size_x(imslit);
    const int       size_y = cpl_image_get_size_y(imslit);
    int             slit_length;
    cpl_size        slit_pos;
    cpl_image   *   filtered;
    cpl_mask    *   mask;
    cpl_image   *   thin_im;
    int             slit_top_y = 0; /* Avoid (false) uninit warning */
    int             slit_bot_y = 0; /* Avoid (false) uninit warning */
    cpl_table   *   self;
    double      *   slit_y,
                *   slit_x_l,
                *   slit_x_r;
    double      *   coeff_r;
    double      *   coeff_l;
    int             i;
    cpl_error_code error = CPL_ERROR_NONE;

    /* Initialize */
    if (slit_flux != NULL) *slit_flux = 0.0 ;

    /* Median vertical filtering 3x3 */
    mask = cpl_mask_new(3, 3) ;
    cpl_mask_not(mask) ;
    filtered = cpl_image_new(size_x, size_y, cpl_image_get_type(imslit));
    error = cpl_image_filter_mask(filtered, imslit, mask,
                                  CPL_FILTER_MEDIAN, CPL_BORDER_FILTER);
    cpl_mask_delete(mask);

    if (error) {
        cpl_image_delete(filtered);
        cpl_ensure(0, cpl_error_get_code(), NULL);
    }

    /* The background may vary strongly along the vertical line. */
    /* Detect and remove background with a 1+2*Slit_max x 1 median filter */
    error = irplib_image_filter_background_line(filtered, NULL, slit_max_width,
                                                CPL_TRUE);

    if (error) {
        cpl_image_delete(filtered) ;
        cpl_ensure(0, cpl_error_get_code(), NULL);
    }

    /* Find the position of the slit */
    if (irplib_slitpos_find_vert_pos(filtered, slit_max_width/2, &slit_pos)) {
        cpl_image_delete(filtered);
        cpl_msg_error(cpl_func, "Could not find the slit position");
        cpl_ensure(0, cpl_error_get_code(), NULL);
    }

    /* Extract a thin image containing the slit */
    thin_im = cpl_image_extract(filtered, slit_pos-slit_max_width/2, 1,
                                slit_pos+slit_max_width/2, size_y);
    if (thin_im == NULL) {
        cpl_msg_error(cpl_func, "Could not extract the %d pixel thin image "
                      "around position %"CPL_SIZE_FORMAT, 
                      slit_max_width, slit_pos);
        cpl_image_delete(filtered);
        cpl_ensure(0, cpl_error_get_code(), NULL);
    }

    /* Find the ends of the slit */
    error = irplib_slitpos_find_vert_slit_ends(thin_im,
                                               IRPLIB_SLITPOS_KERNEL_SIZE_Y,
                                               &slit_bot_y,
                                               &slit_top_y);
    cpl_image_delete(thin_im);
    if (error) {
        cpl_image_delete(filtered);
        cpl_ensure(0, cpl_error_get_code(), NULL);
    }

    /* Extract an image with exactly the slit */
    thin_im = cpl_image_extract(filtered,
                                slit_pos-slit_max_width/2,
                                slit_bot_y,
                                slit_pos+slit_max_width/2,
                                slit_top_y);
    cpl_image_delete(filtered);

    cpl_ensure(thin_im != NULL, cpl_error_get_code(), NULL);

    slit_length = 1 + slit_top_y - slit_bot_y;

    /* Allocate some arrays */
    slit_y = cpl_malloc(slit_length * sizeof(double));
    slit_x_l = cpl_malloc(slit_length * sizeof(double));
    slit_x_r = cpl_malloc(slit_length * sizeof(double));
    
    /* Find the edges of the slit */
    for (i=0 ; i<slit_length ; i++) {
        int right_pos = 0; /* Avoid (false) uninit warning */
        int left_pos  = 0; /* Avoid (false) uninit warning */

        if (irplib_slitpos_find_edges_one_line(thin_im,
                                                i,
                                                &left_pos,
                                                &right_pos)) {
            cpl_msg_error(cpl_func, "cannot find the edges of the [%d]th line", 
                    i+1);
            cpl_image_delete(thin_im);
            cpl_free(slit_y);
            cpl_free(slit_x_l);
            cpl_free(slit_x_r);
            return NULL;
        }

        /* Update the slit_flux */
        if (slit_flux != NULL) {
            *slit_flux += cpl_image_get_flux_window(thin_im, left_pos+1,
                    i+1, right_pos+1, i+1) ;
        }
        
        /* Store the edges for the fit */
        slit_x_l[i] = (double)left_pos;
        slit_x_r[i] = (double)right_pos;
        slit_y[i]   = (double)(i+slit_bot_y-1);
    }
    cpl_image_delete(thin_im);

    /* Linear regression to find the edges */
    coeff_l = irplib_flat_fit_slope_robust(slit_y, slit_x_l, slit_length);
    coeff_r = irplib_flat_fit_slope_robust(slit_y, slit_x_r, slit_length);
    cpl_free(slit_y);
    cpl_free(slit_x_l);
    cpl_free(slit_x_r);

    /* Allocate the table containing the results */
    self = cpl_table_new(slit_length);
    error |= cpl_table_new_column(self, "SLIT_Y",      CPL_TYPE_INT);
    error |= cpl_table_new_column(self, "SLIT_LEFT",   CPL_TYPE_DOUBLE);
    error |= cpl_table_new_column(self, "SLIT_CENTER", CPL_TYPE_DOUBLE);
    error |= cpl_table_new_column(self, "SLIT_RIGHT",  CPL_TYPE_DOUBLE);

    error |= cpl_table_set_column_unit(self, "SLIT_Y", "pixel");
    error |= cpl_table_set_column_unit(self, "SLIT_LEFT", "pixel");
    error |= cpl_table_set_column_unit(self, "SLIT_CENTER", "pixel");
    error |= cpl_table_set_column_unit(self, "SLIT_RIGHT", "pixel");

    cpl_ensure(!error, cpl_error_get_code(), NULL);

    /* Rewrite the edges in the out table, and write the center */
    for (i=0 ; i < slit_length ; i++) {
        const int    islity = i + slit_bot_y;
        const double dslit  = slit_pos - slit_max_width / 2.0;
        const double dleft  = coeff_l[0] + coeff_l[1] * (double)islity + dslit;
        const double dright = coeff_r[0] + coeff_r[1] * (double)islity + dslit;
        const double dcent  = 0.5 * (dleft + dright);

        if (cpl_table_set_int(self,    "SLIT_Y",      i, islity)) break;
        if (cpl_table_set_double(self, "SLIT_LEFT",   i, dleft))  break;
        if (cpl_table_set_double(self, "SLIT_RIGHT",  i, dright)) break;
        if (cpl_table_set_double(self, "SLIT_CENTER", i, dcent))  break;
    }

    cpl_free(coeff_r);
    cpl_free(coeff_l);

    if (i != slit_length) {
        cpl_table_delete(self);
        cpl_ensure(0, cpl_error_get_code(), NULL);
    }

    return self;
}

/**@}*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Return the first pixel higher than avg starting from the
            left and from the right of the line 
  @param    self input image
  @param    line_pos    line position
  @param    left_pos    pointer to left position
  @param    right_pos   pointer to right position
  @return   0 iff successful
  
  Positions in C coordinates (first pixel position is 0)
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code irplib_slitpos_find_edges_one_line(const cpl_image * self,
                                                         int          line_pos,
                                                         int        * left_pos,
                                                         int        * right_pos)
{
    const int     size_x = cpl_image_get_size_x(self);
    const float * pself;
    double        threshold;
    int           i;

    cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(cpl_image_get_type(self) == CPL_TYPE_FLOAT,
                    CPL_ERROR_INVALID_TYPE);

    pself = cpl_image_get_data_float_const(self);

    /* Find the threshold */
    threshold = cpl_image_get_mean_window(self, 1, line_pos+1, size_x,
                                          line_pos+1);

    /* Detect the left edge */
    i = 0;
    while (i < size_x && pself[line_pos*size_x+i] < threshold) i++;
    *left_pos = i;

    /* Detect the right edge */
    i = size_x - 1;
    while (i >= 0 && pself[line_pos*size_x+i] < threshold) i--;
    *right_pos = i;

    return CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Find the ends of a vertical slit (y coordinates in FITS convention)
  @param    in  input image
  @param    kernel_size vertical kernel size
  @param    bot_slit_y  bottom slit y position
  @param    top_slit_y  top slit y position
  @return   0 iff successful
  
  The input image has to be as thin as possible to contain only the slit
 */
/*----------------------------------------------------------------------------*/
static
cpl_error_code irplib_slitpos_find_vert_slit_ends(const cpl_image * self,
                                                  int        kernel_size,
                                                  int      * bot_slit_y,
                                                  int      * top_slit_y)
{
    cpl_mask         * binary;
    cpl_mask         * copy = NULL;
    cpl_mask         * kernel;
    cpl_image        * label_image;
    int                erosions_nb;
    cpl_size           nobj ;
    const int          size_x = cpl_image_get_size_x(self);
    const int          size_y = cpl_image_get_size_y(self);
    const int          npix = size_x * size_y;
    const cpl_binary * pbinary;
    const cpl_binary * pfind;
    int                i, itop, ibot;


    cpl_ensure_code(size_x      > 0, cpl_error_get_code());
    cpl_ensure_code(kernel_size > 0, cpl_error_get_code());

    /* Threshold to have a binary image */
    binary = cpl_mask_threshold_image_create(self, cpl_image_get_mean(self),
                                             cpl_image_get_max(self));
    cpl_ensure_code(binary != NULL, cpl_error_get_code());

    /* Erode until there is 1 object left in the image */
    label_image = cpl_image_labelise_mask_create(binary, &nobj);
    cpl_image_delete(label_image);

    if (label_image == NULL) {
        cpl_mask_delete(binary);
        cpl_ensure_code(0, cpl_error_get_code());
    }

    /* Define the kernel for morpho operations */
    kernel = cpl_mask_new(kernel_size, 1);
    cpl_mask_not(kernel);
    copy = cpl_mask_wrap(size_x, size_y, cpl_malloc(size_x * size_y *
                                                    sizeof(cpl_binary)));
    for (erosions_nb = 0; erosions_nb < IRPLIB_SLITPOS_MAX_EROSION && nobj > 1;
         erosions_nb++) {
        /* Should not be possible to break from this loop */
        cpl_mask_copy(copy, binary, 1, 1);
        if (cpl_mask_filter(binary, copy, kernel, CPL_FILTER_EROSION,
                            CPL_BORDER_ZERO)) break;

        label_image = cpl_image_labelise_mask_create(binary, &nobj);
        if (label_image == NULL) break; /* Assuming nobj was not set to 1 */
        cpl_image_delete(label_image);
    }

    if (nobj > 1) {
        cpl_mask_delete(binary);
        cpl_mask_delete(copy);
        cpl_mask_delete(kernel);
        if (erosions_nb >= IRPLIB_SLITPOS_MAX_EROSION) {
            cpl_msg_error(cpl_func, "Number of erosions reached a limit of %d "
                          "with %"CPL_SIZE_FORMAT" possible slits left",
                          IRPLIB_SLITPOS_MAX_EROSION, nobj);
            cpl_ensure_code(0, CPL_ERROR_CONTINUE);
        }
        cpl_ensure_code(0, cpl_error_get_code());
    } else if (nobj < 1) {
        cpl_mask_delete(binary);
        cpl_mask_delete(copy);
        cpl_mask_delete(kernel);
        if (erosions_nb == 0)
            cpl_msg_error(cpl_func, "No slit could be detected across %d "
                          "pixels", size_x);
        else 
            cpl_msg_error(cpl_func, "The last of %d erosions removed all the "
                          "possible slits", erosions_nb);
        cpl_ensure_code(0, CPL_ERROR_DATA_NOT_FOUND);
    }

    /* Reconstruct the slit with dilations */
    for (i=0 ; i < erosions_nb ; i++) {
        cpl_mask_copy(copy, binary, 1, 1);
        if (cpl_mask_filter(binary, copy, kernel, CPL_FILTER_DILATION,
                            CPL_BORDER_ZERO)) break;
    }
    cpl_mask_delete(copy);
    cpl_mask_delete(kernel);

    if (i != erosions_nb) {
        cpl_msg_error(cpl_func, "Dilation number %d out of %d failed",
                      i, erosions_nb);
        cpl_mask_delete(binary);
        cpl_ensure_code(0, cpl_error_get_code());
    }

    /* Find the ends of the slit */
    pbinary = cpl_mask_get_data(binary);
    assert( pbinary != NULL );

    pfind = memchr(pbinary, CPL_BINARY_1, (size_t)npix);
    assert( pfind != NULL );

    ibot = (int)(pfind - pbinary);

#if defined HAVE_DECL_MEMRCHR && HAVE_DECL_MEMRCHR == 1
    /* FIXME: Not tested */
    pfind = memrchr(pfind, CPL_BINARY_1, (size_t)(npix - ibot));
    assert( pfind != NULL );

    itop = (int)(pfind - pbinary);
#else

    itop = npix - 1;
    while (itop > ibot && pbinary[itop] == CPL_BINARY_0) itop--;

#endif

    *bot_slit_y = 1 + ibot / size_x;
    *top_slit_y = 1 + itop / size_x;

    cpl_msg_info(cpl_func, 
            "Detected %"CPL_SIZE_FORMAT"-pixel slit from pixel %d to %d "
            "using %d erosions/dilations", cpl_mask_count(binary),
            *bot_slit_y, *top_slit_y, erosions_nb);

    cpl_mask_delete(binary);

    /* Should really be an assert() */
    cpl_ensure_code(ibot <= itop, CPL_ERROR_DATA_NOT_FOUND);

    return CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Find a vertical slit position (x coordinate of the slit)
  @param    self      Filtered input image
  @param    xwidth    Width of the boundary to not search (in x)
  @param    slit_pos  Pointer to the searched position
  @return   0 iff successful
  
  Coordinate given in FITS convention (ll is (1,1))
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code irplib_slitpos_find_vert_pos(const cpl_image * self,
                                                   int               xwidth,
                                                   cpl_size        * slit_pos)
{
    const int       size_x = cpl_image_get_size_x(self);
    cpl_image   *   image1D;
    cpl_size        yone;
    cpl_error_code  error;


    /* Collapse the image to a horizontal 1D image */
    image1D = cpl_image_collapse_create(self, 0);

    cpl_ensure_code(image1D != NULL, cpl_error_get_code());

    /* Search the max of the 1D image to identify the slit position */
    error = cpl_image_get_maxpos_window(image1D, 1+xwidth, 1, size_x-xwidth,
                                        1, slit_pos, &yone);

    cpl_image_delete(image1D);

    cpl_ensure_code(!error, error);

    return CPL_ERROR_NONE;
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


