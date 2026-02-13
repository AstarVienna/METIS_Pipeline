/* $Id: irplib_ksigma_clip.c,v 1.1 2011-11-02 13:18:28 amodigli Exp $
 *
 * This file is part of the irplib package
 * Copyright (C) 2002, 2003 European Southern Observatory
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02111-1307 USA
 */

/*
 * $Author: amodigli $
 * $Date: 2011-11-02 13:18:28 $
 * $Revision: 1.1 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <complex.h>

/*---------------------------------------------------------------------------
                                  Includes
 ---------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>
#include <assert.h>
#include <float.h>

#include <cpl.h>

#include "irplib_ksigma_clip.h"

#include "irplib_hist.h"
#include "irplib_utils.h"

/*--------------------------------------------------------------------------*/

/*
 * @defgroup ksigmaclip        kappa sigma clip functions
 */

/*--------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                  Defines
 ---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                  Private function prototypes
 ---------------------------------------------------------------------------*/

static cpl_error_code
irplib_ksigma_clip_double(const double  * pi,
			  int               llx,
			  int               lly,
			  int               urx,
			  int               ury,
			  int               nx,
			  double            var_sum,
			  int               npixs,
			  double            kappa,
			  int               nclip,
			  double            tolerance,
			  double          * mean,
			  double          * stdev);

static cpl_error_code
irplib_ksigma_clip_float(const float     * pi,
			 int               llx,
			 int               lly,
			 int               urx,
			 int               ury,
			 int               nx,
			 double            var_sum,
			 int               npixs,
			 double            kappa,
			 int               nclip,
			 double            tolerance,
			 double          * mean,
			 double          * stdev);

static cpl_error_code
irplib_ksigma_clip_int(const int       * pi,
		       int               llx,
		       int               lly,
		       int               urx,
		       int               ury,
		       int               nx,
		       double            var_sum,
		       int               npixs,
		       double            kappa,
		       int               nclip,
		       double            tolerance,
		       double          * mean,
		       double          * stdev);


/*---------------------------------------------------------------------------*/
/**
  @brief    Apply kappa-sigma clipping on input image
  @param    img      Input image
  @param    llx      Lower left x position (FITS convention)
  @param    lly      Lower left y position (FITS convention)
  @param    urx      Upper right x position (FITS convention)
  @param    ury      Upper right y position (FITS convention)
  @param    kappa    Kappa value for the clipping
  @param    nclip    Number of clipping iterations
  @param    tolerance tolerance on range between two successive clip iterations
  @param    kmean    Mean after clipping (output)
  @param    kstdev   Stdev after clipping (output)
  @return   CPL_ERROR_NONE or the relevant #_cpl_error_code_ on error

  This function applies an iterative kappa-sigma clipping on the image and
  returns mean and stdev after the clipping.

  The function takes as a starting point the "standard" values of mean and
  stdev from cpl_stats.

  On each iteration, the contribution of pixels outside the range
  [mean - kappa * stdev, mean + kappa * stdev] is removed, the values of
  mean and stdev are updated, and so are the limits of the range to be used
  in the next iteration as well.

  The algorithm stops after nclip iterations or when the variation of the
  range between two consecutive iterations is smaller (absolute value) than
  the tolerance.

  The effectiveness of this function resides on the way the update of the
  values of mean and stdev is done.

  The contribution of a single pixel in variance can be removed as follows:

  \sum_{i=1}^{N-1} (x_i - \overline{x}_{n-1})^2 =
  \sum_{i=1}^ N    (x_i - \overline{x}_n    )^2 -
  \frac{N}{N-1} \,( \, \overline{x}_n - x_{n} )^2

  For further details on the mathematical aspects, please refer to DFS05126.

  Possible #_cpl_error_code_ set in this function:
   - CPL_ERROR_NULL_INPUT if img or kmean is NULL
   - CPL_ERROR_ILLEGAL_INPUT if
       a) the window specification is illegal (llx > urx or lly > ury)
       b) the window specification is outside the image
       c) the tolerance is negative
       d) kappa is <= 1.0
       e) nclip is <= 0.

  The values of kmean and kstdev is undefined on error.
*/
/*---------------------------------------------------------------------------*/
cpl_error_code
irplib_ksigma_clip(const cpl_image * img,
		   int               llx,
		   int               lly,
		   int               urx,
		   int               ury,
		   double            kappa,
		   int               nclip,
		   double            tolerance,
		   double          * kmean,
		   double          * kstdev)
{
    cpl_errorstate inistate = cpl_errorstate_get();

    int nx, ny;

    cpl_stats * stats;
    double      mean, stdev, var_sum;
    int         npixs;

    cpl_ensure_code(img != NULL, CPL_ERROR_NULL_INPUT);

    nx = cpl_image_get_size_x(img);
    ny = cpl_image_get_size_y(img);

    cpl_ensure_code(llx > 0 && urx > llx && urx <= nx &&
		    lly > 0 && ury > lly && ury <= ny,
		    CPL_ERROR_ILLEGAL_INPUT);

    cpl_ensure_code(tolerance >= 0.0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(kappa     >  1.0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(nclip     >    0, CPL_ERROR_ILLEGAL_INPUT);

    stats = cpl_stats_new_from_image_window(img,
					    CPL_STATS_MEAN | CPL_STATS_STDEV,
					    llx, lly, urx, ury);

    npixs   = cpl_stats_get_npix(stats); /* Non-bad pixels in window */
    mean    = cpl_stats_get_mean(stats);
    stdev   = cpl_stats_get_stdev(stats);
    var_sum = stdev * stdev * (npixs - 1);

    cpl_stats_delete(stats);

    /* img, llx etc. may cause errors: Check and propagate */
    cpl_ensure_code(cpl_errorstate_is_equal(inistate), cpl_error_get_code());

    switch (cpl_image_get_type(img)) {
    case CPL_TYPE_DOUBLE:
	skip_if(irplib_ksigma_clip_double(cpl_image_get_data_double_const(img),
					  llx, lly, urx, ury, nx, var_sum,
					  npixs, kappa, nclip, tolerance,
					  &mean, &stdev));
	break;
    case CPL_TYPE_FLOAT:
	skip_if(irplib_ksigma_clip_float(cpl_image_get_data_float_const(img),
					 llx, lly, urx, ury, nx, var_sum,
					 npixs, kappa, nclip, tolerance,
					 &mean, &stdev));
	break;
    case CPL_TYPE_INT:
	skip_if(irplib_ksigma_clip_int(cpl_image_get_data_int_const(img),
				       llx, lly, urx, ury, nx, var_sum,
				       npixs, kappa, nclip, tolerance,
				       &mean, &stdev));
	break;
    default:
	/* It is an error in CPL to reach this point */
	assert( 0 );
    }

    *kmean = mean;
    if (kstdev != NULL) *kstdev = stdev; /* Optional */

    end_skip;

    return cpl_error_get_code();
}

#define CONCAT(a,b) a ## _ ## b
#define CONCAT2X(a,b) CONCAT(a,b)

#define CPL_TYPE double
#include "irplib_ksigma_clip_body.h"
#undef CPL_TYPE

#define CPL_TYPE float
#include "irplib_ksigma_clip_body.h"
#undef CPL_TYPE

#define CPL_TYPE int
#include "irplib_ksigma_clip_body.h"
#undef CPL_TYPE

