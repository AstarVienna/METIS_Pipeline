/*
 * This file is part of the HDRL
 * Copyright (C) 2013 European Southern Observatory
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

#include "hdrl_imagelist.h"
#include "hdrl_imagelist_view.h"
#include "hdrl_imagelist_defs.h"
#include "hdrl_image.h"
#include "hdrl_collapse.h"

#include <cpl.h>
#include <assert.h>


/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_imagelist   Imagelist object
 *
 * hdrl_imagelist is similiar to cpl_imagelist but for hdrl_image.
 * Its reduction methods (mean, median, ...) provide linear error propagation.
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*-----------------------------------------------------------------------------
                                   Define
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @enum hdrl_imagelist_basic_type
  @brief Define the kind of operation to apply
 */
/*----------------------------------------------------------------------------*/
typedef enum {
	HDRL_IMLIST_BASIC_IMLIST = 0,
	HDRL_IMLIST_BASIC_IMAGE,
	HDRL_IMLIST_BASIC_SCALAR
} hdrl_imagelist_basic_type;

typedef cpl_error_code (*hdrl_f_image)(hdrl_image *, const hdrl_image *);
typedef cpl_error_code (*hdrl_f_scalar)(hdrl_image *, hdrl_value);

/*-----------------------------------------------------------------------------
                            Static Prototypes
 -----------------------------------------------------------------------------*/

static cpl_error_code hdrl_imagelist_basic_operation(
		hdrl_imagelist_basic_type type,
		void                      *func,
		hdrl_imagelist            *himlist1,
		const hdrl_imagelist      *himlist2,
		const hdrl_image          *himg,
		hdrl_value                *value);

static cpl_error_code hdrl_imagelist_collapse_interface(
		const hdrl_imagelist                *himlist,
        hdrl_collapse_imagelist_to_image_t  *collapse_method,
        hdrl_image                          **out,
        cpl_image                           **contrib,
        void                                **eout);

/*-----------------------------------------------------------------------------
                            Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Add two image lists, the first one is replaced by the result.
  @param    himlist1   first input image list (modified)
  @param    himlist2   image list to add
  @return   the cpl_error_code or CPL_ERROR_NONE
  @see      hdrl_image_add_image()

  The two input lists must have the same size, the image number n in the
  list himlist2 is added to the image number n in the list himlist1.

  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
  - CPL_ERROR_ILLEGAL_INPUT if the input images have different sizes
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_add_imagelist(
        hdrl_imagelist        *    himlist1,
        const hdrl_imagelist  *    himlist2)
{
	return hdrl_imagelist_basic_operation(HDRL_IMLIST_BASIC_IMLIST,
			&hdrl_image_add_image, himlist1, himlist2, NULL, NULL);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Subtract two image lists, the first one is replaced by the result.
  @param    himlist1  first input image list (modified)
  @param    himlist2  image list to subtract
  @return   the cpl_error_code or CPL_ERROR_NONE
  @see      hdrl_image_sub_image()
  @see      hdrl_imagelist_add_imagelist
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_sub_imagelist(
        hdrl_imagelist        *    himlist1,
        const hdrl_imagelist  *    himlist2)
{
	return hdrl_imagelist_basic_operation(HDRL_IMLIST_BASIC_IMLIST,
			&hdrl_image_sub_image, himlist1, himlist2, NULL, NULL);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Multiply two image lists, the first one is replaced by the result.
  @param    himlist1  first input image list (modified)
  @param    himlist2  image list to multiply
  @return   the cpl_error_code or CPL_ERROR_NONE
  @see      hdrl_image_mul_image()
  @see      hdrl_imagelist_add_imagelist
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_mul_imagelist(
        hdrl_imagelist        *    himlist1,
        const hdrl_imagelist  *    himlist2)
{
	return hdrl_imagelist_basic_operation(HDRL_IMLIST_BASIC_IMLIST,
			&hdrl_image_mul_image, himlist1, himlist2, NULL, NULL);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Divide two image lists, the first one is replaced by the result.
  @param    himlist1  first input image list (modified)
  @param    himlist2  image list to divide
  @return   the cpl_error_code or CPL_ERROR_NONE
  @see      hdrl_image_div_image()
  @see      hdrl_imagelist_add_imagelist
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_div_imagelist(
        hdrl_imagelist        *    himlist1,
        const hdrl_imagelist  *    himlist2)
{
	return hdrl_imagelist_basic_operation(HDRL_IMLIST_BASIC_IMLIST,
			&hdrl_image_div_image, himlist1, himlist2, NULL, NULL);
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Add an image to an image list.
  @param    himlist input image list (modified)
  @param    himg    image to add
  @return   the cpl_error_code or CPL_ERROR_NONE
  @see      hdrl_image_add_image()

  The passed image is added to each image of the passed image list.

  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_add_image(
        hdrl_imagelist      *    himlist,
        const hdrl_image    *    himg)
{
	return hdrl_imagelist_basic_operation(HDRL_IMLIST_BASIC_IMAGE,
			&hdrl_image_add_image, himlist, NULL, himg, NULL);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Subtract an image from an image list.
  @param    himlist input image list (modified)
  @param    himg    image to subtract
  @return   the cpl_error_code or CPL_ERROR_NONE
  @see      hdrl_image_sub_image()
  @see      hdrl_imagelist_add_image()

  The passed image is subtracted from each image of the passed image list.

  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_sub_image(
        hdrl_imagelist      *    himlist,
        const hdrl_image    *    himg)
{
	return hdrl_imagelist_basic_operation(HDRL_IMLIST_BASIC_IMAGE,
			&hdrl_image_sub_image, himlist, NULL, himg, NULL);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Multiply an image by an image list.
  @param    himlist input image list (modified)
  @param    himg    image to multiply
  @return   the cpl_error_code or CPL_ERROR_NONE
  @see      hdrl_image_mul_image()
  @see      hdrl_imagelist_add_image()

  The passed image is multiplied by each image of the passed image list.

  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_mul_image(
        hdrl_imagelist      *    himlist,
        const hdrl_image    *    himg)
{
	return hdrl_imagelist_basic_operation(HDRL_IMLIST_BASIC_IMAGE,
			&hdrl_image_mul_image, himlist, NULL, himg, NULL);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Divide an image from an image list.
  @param    himlist input image list (modified)
  @param    himg    image to divide
  @return   the cpl_error_code or CPL_ERROR_NONE
  @see      hdrl_image_div_image()
  @see      hdrl_imagelist_add_image()

  The passed image is used to divide each image of the passed image list.

  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_div_image(
        hdrl_imagelist      *    himlist,
        const hdrl_image    *    himg)
{
	return hdrl_imagelist_basic_operation(HDRL_IMLIST_BASIC_IMAGE,
			&hdrl_image_div_image, himlist, NULL, himg, NULL);
}



/*----------------------------------------------------------------------------*/
/**
  @brief    Elementwise addition of a scalar to each image in the himlist
  @param    himlist Imagelist to be modified in place.
  @param    value   Value to add to the images
  @return   the cpl_error_code or CPL_ERROR_NONE
  @see      hdrl_image_add_scalar()
 
  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_add_scalar(
        hdrl_imagelist  *   himlist,
        hdrl_value          value)
{
    const cpl_error_code code = 
        hdrl_imagelist_basic_operation(HDRL_IMLIST_BASIC_SCALAR,
                                       &hdrl_image_add_scalar,
                                       himlist, NULL, NULL, &value);
    return code ? cpl_error_set_where(cpl_func) : CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Elementwise subtraction of a scalar to each image in the himlist
  @param    himlist Imagelist to be modified in place.
  @param    value   Value to subtract to the images
  @return   the cpl_error_code or CPL_ERROR_NONE
  @see      hdrl_image_sub_scalar()
  @see      hdrl_imagelist_add_scalar()
 
  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_sub_scalar(
        hdrl_imagelist  *   himlist,
        hdrl_value          value)
{
    const cpl_error_code code = 
        hdrl_imagelist_basic_operation(HDRL_IMLIST_BASIC_SCALAR,
                                       &hdrl_image_sub_scalar,
                                       himlist, NULL, NULL, &value);
    return code ? cpl_error_set_where(cpl_func) : CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Elementwise multiplication of a scalar to each image in the himlist
  @param    himlist Imagelist to be modified in place.
  @param    value   Value to multiply to the images
  @return   the cpl_error_code or CPL_ERROR_NONE
  @see      hdrl_image_mul_scalar()
  @see      hdrl_imagelist_add_scalar()
 
  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_mul_scalar(
        hdrl_imagelist  *   himlist,
        hdrl_value          value)
{
    const cpl_error_code code = 
        hdrl_imagelist_basic_operation(HDRL_IMLIST_BASIC_SCALAR,
                                       &hdrl_image_mul_scalar,
                                       himlist, NULL, NULL, &value);
    return code ? cpl_error_set_where(cpl_func) : CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Elementwise division by a scalar to each image in the himlist
  @param    himlist Imagelist to be modified in place.
  @param    value   Value to divide to the images
  @return   the cpl_error_code or CPL_ERROR_NONE
  @see      hdrl_image_div_scalar()
  @see      hdrl_imagelist_add_scalar()
 
  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_div_scalar(
        hdrl_imagelist  *   himlist,
        hdrl_value          value)
{
	return hdrl_imagelist_basic_operation(HDRL_IMLIST_BASIC_SCALAR,
			&hdrl_image_div_scalar, himlist,
                                              NULL, NULL, &value);
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the elementwise power of each image in the himlist
  @param    himlist     Imagelist to be modified in place.
  @param    exponent    Scalar exponent
  @return   CPL_ERROR_NONE or the relevant the cpl_error_code on error
  @see      hdrl_image_power()
 
  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_pow_scalar(
        hdrl_imagelist  *   himlist,
        hdrl_value          exponent)
{
    cpl_ensure_code(himlist != NULL, CPL_ERROR_NULL_INPUT);

    cpl_size nima = hdrl_imagelist_get_size(himlist) ;
    for (cpl_size i = 0 ; i<nima ; i++) {
        cpl_ensure_code(!hdrl_image_pow_scalar(hdrl_imagelist_get(himlist, i),
        		exponent), cpl_error_get_code());
    }
    return CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @brief collapsing of image list
  @param himlist    input image list
  @param param      collapse parameters defining the type of collapse
  @param out        output combined image
                    pointer filled with pointer to allocated result object
  @param contrib    output contribution mask
                    pointer filled with pointer to allocated result object
  @return   CPL_ERROR_NONE or the relevant the cpl_error_code on error
  @see hdrl_collapse_mean_parameter_create()

  Collapse an imagelist according to the type of collapse parameter used as
  input. It only supports collapse methods with the two outputs, the combined
  image and the contribution map.
  For collapse functions with additional output the specialized collapse
  functions must be used.

  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
  - CPL_ERROR_UNSUPPORTED_MODE parameter is not a know collapse parameter
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_collapse(
        const hdrl_imagelist    *   himlist,
        const hdrl_parameter    *   param,
        hdrl_image              **  out,
        cpl_image               **  contrib)
{
    cpl_ensure_code(himlist, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(param, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(out, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(contrib, CPL_ERROR_NULL_INPUT);

    if (hdrl_collapse_parameter_is_mean(param)) {
        hdrl_imagelist_collapse_mean(himlist, out, contrib);
    }
    else if (hdrl_collapse_parameter_is_weighted_mean(param)) {
        hdrl_imagelist_collapse_weighted_mean(himlist, out, contrib);
    }
    else if (hdrl_collapse_parameter_is_median(param)) {
        hdrl_imagelist_collapse_median(himlist, out, contrib);
    }
    else if (hdrl_collapse_parameter_is_sigclip(param)) {
        hdrl_imagelist_collapse_sigclip(himlist,
                hdrl_collapse_sigclip_parameter_get_kappa_low(param),
                hdrl_collapse_sigclip_parameter_get_kappa_high(param),
                hdrl_collapse_sigclip_parameter_get_niter(param),
                out, contrib, NULL, NULL);
    }
    else if (hdrl_collapse_parameter_is_minmax(param)) {
        hdrl_imagelist_collapse_minmax(himlist,
                hdrl_collapse_minmax_parameter_get_nlow(param),
                hdrl_collapse_minmax_parameter_get_nhigh(param),
                out, contrib, NULL, NULL);
    }
    else if (hdrl_collapse_parameter_is_mode(param)) {
        hdrl_imagelist_collapse_mode(himlist,
                hdrl_collapse_mode_parameter_get_histo_min(param),
                hdrl_collapse_mode_parameter_get_histo_max(param),
                hdrl_collapse_mode_parameter_get_bin_size(param),
                hdrl_collapse_mode_parameter_get_method(param),
                hdrl_collapse_mode_parameter_get_error_niter(param),
                out, contrib);
    }
    else {
        return cpl_error_set_message(cpl_func, CPL_ERROR_UNSUPPORTED_MODE,
                                     "Invalid parameter input for "
                                     "hdrl_imagelist_collapse");
    }

    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Mean collapsing of image list
  @param himlist    input image list
  @param out        output combined image
  @param contrib    output contribution mask
  @return   CPL_ERROR_NONE or the relevant the cpl_error_code on error
 
  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_collapse_mean(
        const hdrl_imagelist    *   himlist,
        hdrl_image              **  out,
        cpl_image               **  contrib) 
{
    hdrl_collapse_imagelist_to_image_t * method =
        hdrl_collapse_imagelist_to_image_mean();
    hdrl_imagelist_collapse_interface(himlist, method, out, contrib, NULL) ;
    hdrl_collapse_imagelist_to_image_delete(method) ;
    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Weighted Mean collapsing of image list
  @param himlist    input image list
  @param out        output combined image
  @param contrib    output contribution mask
  @return   CPL_ERROR_NONE or the relevant the cpl_error_code on error
 
  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_collapse_weighted_mean(
        const hdrl_imagelist    *   himlist,
        hdrl_image              **  out,
        cpl_image               **  contrib) 
{
    hdrl_collapse_imagelist_to_image_t * method =
        hdrl_collapse_imagelist_to_image_weighted_mean();
    hdrl_imagelist_collapse_interface(himlist, method, out, contrib, NULL) ;
    hdrl_collapse_imagelist_to_image_delete(method) ;
    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Median collapsing of image list
  @param himlist    input image list
  @param out        output combined image
  @param contrib    output contribution mask
  @return   CPL_ERROR_NONE or the relevant the cpl_error_code on error
 
  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_collapse_median(
        const hdrl_imagelist    *   himlist,
        hdrl_image              **  out,
        cpl_image               **  contrib) 
{
    hdrl_collapse_imagelist_to_image_t * method =
        hdrl_collapse_imagelist_to_image_median();
    hdrl_imagelist_collapse_interface(himlist, method, out, contrib, NULL) ;
    hdrl_collapse_imagelist_to_image_delete(method) ;
    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Sigma-clipped collapsing of image list
  @param himlist      input image list
  @param kappa_low    low sigma bound
  @param kappa_high   high sigma bound
  @param niter        number of clipping iterators
  @param out          output combined image
  @param contrib      output contribution mask
  @param reject_low   output low rejection thresholds, may be NULL
  @param reject_high  output high rejection thresholds, may be NULL
  @return   CPL_ERROR_NONE or the relevant the cpl_error_code on error
  @see hdrl_imagelist_collapse()
 
  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_collapse_sigclip(
        const hdrl_imagelist    *   himlist,
        double                      kappa_low,
        double                      kappa_high,
        int                         niter,
        hdrl_image              **  out,
        cpl_image               **  contrib,
        cpl_image               **  reject_low,
        cpl_image               **  reject_high)
{
    hdrl_sigclip_image_output * sigclipout;
    hdrl_collapse_imagelist_to_image_t * method =
        hdrl_collapse_imagelist_to_image_sigclip(kappa_low, kappa_high, niter);
    hdrl_imagelist_collapse_interface(himlist, method, out, contrib, (void**)&sigclipout);

    if (cpl_error_get_code() != CPL_ERROR_NONE) {

        hdrl_collapse_imagelist_to_image_delete(method);

        if (reject_low) {
            *reject_low = NULL;
        }

        if (reject_high) {
            *reject_high = NULL;
        }

        return cpl_error_get_code();
    }

    if (reject_low) {
        *reject_low = sigclipout->reject_low;
    } else {
        cpl_image_delete(sigclipout->reject_low);
    }

    if (reject_high) {
        *reject_high = sigclipout->reject_high;
    } else {
        cpl_image_delete(sigclipout->reject_high);
    }

    hdrl_collapse_imagelist_to_image_unwrap_eout(method, sigclipout);
    hdrl_collapse_imagelist_to_image_delete(method);

    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief Minmax-clipped collapsing of image list
  @param himlist      input image list
  @param nlow         low number of pixels to reject
  @param nhigh        high number of pixels to reject
  @param out          output combined image
  @param contrib      output contribution mask
  @param reject_low   output low rejection thresholds, may be NULL
  @param reject_high  output high rejection thresholds, may be NULL
  @return   CPL_ERROR_NONE or the relevant the cpl_error_code on error
  @see hdrl_imagelist_collapse()
 
  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_collapse_minmax(
        const hdrl_imagelist    *   himlist,
        double                      nlow,
        double                      nhigh,
        hdrl_image              **  out,
        cpl_image               **  contrib,
        cpl_image               **  reject_low,
        cpl_image               **  reject_high)
{
    hdrl_minmax_image_output * minmaxout;
    hdrl_collapse_imagelist_to_image_t * method =
        hdrl_collapse_imagelist_to_image_minmax(nlow, nhigh);
    hdrl_imagelist_collapse_interface(himlist, method, out, contrib, (void**)&minmaxout);

    if (cpl_error_get_code() != CPL_ERROR_NONE) {

        hdrl_collapse_imagelist_to_image_delete(method);

        if (reject_low) {
            *reject_low = NULL;
        }

        if (reject_high) {
            *reject_high = NULL;
        }

        return cpl_error_get_code();
    }

    if (reject_low) {
        *reject_low = minmaxout->reject_low;
    } else {
        cpl_image_delete(minmaxout->reject_low);
    }

    if (reject_high) {
        *reject_high = minmaxout->reject_high;
    } else {
        cpl_image_delete(minmaxout->reject_high);
    }

    hdrl_collapse_imagelist_to_image_unwrap_eout(method, minmaxout);
    hdrl_collapse_imagelist_to_image_delete(method);

    return cpl_error_get_code();
}




/*----------------------------------------------------------------------------*/
/**
  @brief Mode  collapsing of image list
  @param himlist     input image list
  @param histo_min   minimum value of low pixels to use
  @param histo_max   maximum value of high pixels to be use
  @param bin_size    size of the histogram bin
  @param mode_method mode_method to use for the mode computation
  @param error_niter number of iterations to compute the error of the mode
  @param out         output combined image
  @param contrib     output contribution mask
  @return   CPL_ERROR_NONE or the relevant the cpl_error_code on error

  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_collapse_mode(const hdrl_imagelist * himlist,
					    double                 histo_min,
					    double                 histo_max,
					    double                 bin_size,
					    hdrl_mode_type         mode_method,
					    cpl_size               error_niter,
					    hdrl_image              ** out,
					    cpl_image               ** contrib)
{
    hdrl_collapse_imagelist_to_image_t * method =
        hdrl_collapse_imagelist_to_image_mode(histo_min, histo_max, bin_size,
					      mode_method, error_niter );
    hdrl_imagelist_collapse_interface(himlist, method, out, contrib, NULL) ;
    hdrl_collapse_imagelist_to_image_delete(method) ;
    return cpl_error_get_code();
}










/*----------------------------------------------------------------------------*/
/**
  @brief Generic hdrl_imagelist_collapse interface to hdrl_collapse
  @param himlist    input image list
  @param niter      number of clipping iterators
  @param out        output combined image
  @param contrib    output contribution mask
  @param eout       storage for extra output, may be NULL
  @return   CPL_ERROR_NONE or the relevant the cpl_error_code on error
 
  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code hdrl_imagelist_collapse_interface(
        const hdrl_imagelist                *   himlist,
        hdrl_collapse_imagelist_to_image_t  *   collapse_method,
        hdrl_image                          **  out,
        cpl_image                           **  contrib,
        void                                **  eout)
{
    cpl_error_code fail = CPL_ERROR_NONE;

    /* Check inputs */
    cpl_ensure_code(himlist != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(out     != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(contrib != NULL, CPL_ERROR_NULL_INPUT);

    cpl_size nz = hdrl_imagelist_get_size(himlist);
    cpl_size nx = hdrl_imagelist_get_size_x(himlist);
    cpl_size ny = hdrl_imagelist_get_size_y(himlist);
    if (cpl_error_get_code() != CPL_ERROR_NONE) {
        return cpl_error_get_code();
    }

    *out     = hdrl_image_new(nx, ny);
    *contrib = cpl_image_new( nx, ny, CPL_TYPE_INT);

    /* make sure we have masks else the copy into these is not threadsafe */
    cpl_image_get_bpm(*contrib);
    hdrl_image_get_mask(*out);

    /* create full extra output */
    if (eout) {
        *eout = hdrl_collapse_imagelist_to_image_create_eout(
        	collapse_method, hdrl_image_get_image(hdrl_imagelist_get(himlist, 0)));
    }

    /* get blocks that can be processed in parallel
     * small blocksize better for cache but currently excessive malloc/free
     * prevents making it smaller */
    cpl_size blocksize = 16ul * (1ul<<20ul) / (nz * nx * sizeof(double));
    hdrl_iter *it = hdrl_imagelist_get_iter_row_slices(himlist, blocksize, 0, HDRL_ITER_CONST);

    cpl_size nit = hdrl_iter_length(it);
    hdrl_imagelist *vl[nit];
    cpl_size yl[nit];

    {
        cpl_size i = 0;
        cpl_size y = 1;
        for (hdrl_imagelist * v = hdrl_iter_next(it);
             v != NULL;
             v = hdrl_iter_next(it)) {
            vl[i] = v;
            yl[i++] = y;
            y += hdrl_imagelist_get_size_y(v);
        }
    }

    hdrl_iter_delete(it);

    if (cpl_error_get_code() != CPL_ERROR_NONE) {
        for (cpl_size i = 0; i < nit; i++) {
            hdrl_imagelist_delete(vl[i]);
        }
        return cpl_error_get_code();
    }

    HDRL_OMP(omp parallel for)
    for (cpl_size i = 0; i < nit; i++) {

        cpl_image     *out_data;
        cpl_image     *out_errors;
        cpl_image     *out_contrib;

        cpl_imagelist *data;
        cpl_imagelist *errors;

        void *out_eout;

        hdrl_imagelist *v = vl[i];
        cpl_size y = yl[i];

        /* Build the Inputs Interface */
        hdrl_imagelist_to_cplwrap(v, &data, &errors);

        /* Call the actual collapsing */
        hdrl_collapse_imagelist_to_image_call(collapse_method, data, errors,
                                              &out_data, &out_errors,
                                              &out_contrib, &out_eout);

        if (cpl_error_get_code() != CPL_ERROR_NONE) {

            fail = cpl_error_get_code();
            cpl_imagelist_unwrap(data) ;
            cpl_imagelist_unwrap(errors) ;
            hdrl_imagelist_delete(v);

        } else {

        	cpl_msg_debug(cpl_func, "Collapsed block %lld to %lld", y,
        			y + cpl_image_get_size_y(out_data) - 1);

        	/* TODO make use of output image views to avoid copying */
        	assert(hdrl_image_get_mask_const(*out));
        	assert(cpl_image_get_bpm_const(*contrib));
        	hdrl_image_insert(*out, out_data, out_errors, 1, y);
        	cpl_image_copy(*contrib, out_contrib, 1, y);

        	/* copy and delete slice extra out */
        	if (out_eout) {
        		hdrl_collapse_imagelist_to_image_move_eout(collapse_method,
        				*eout, out_eout, y);
        	}

        	/* Destroy the Inputs Interface */
        	cpl_image_delete(out_data);
        	cpl_image_delete(out_errors);
        	cpl_image_delete(out_contrib);
        	cpl_imagelist_unwrap(data) ;
        	cpl_imagelist_unwrap(errors) ;
        	hdrl_imagelist_delete(v);
        }
    }

    if (fail != CPL_ERROR_NONE) {

        if (eout) {
            hdrl_collapse_imagelist_to_image_delete_eout(collapse_method, *eout);
        }

        hdrl_image_delete(*out);
        cpl_image_delete(*contrib);

        return cpl_error_set_message(cpl_func, fail, "hdrl_imagelist_collapse failed");
    }

    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
 * @brief    Compute the elementwise power of each image in the himlist
 *
 * @param    type        type of operation
 * @param    func        ptr to operation what it need to apply
 * @param    himlist1    ptr to imagelist to be modified (not NULL).
 * @param    himlist2    ptr to imagelist to be apply over himlist1 or NULL
 * @param    himg        ptr to image     to be apply over himlist1 or NULL
 * @param    value       ptr to hvalue    to be apply over himlist1 or NULL
 *
 * @return   CPL_ERROR_NONE or the relevant the cpl_error_code on error
 *
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_imagelist_basic_operation(
    hdrl_imagelist_basic_type type,
    void                      *func,
    hdrl_imagelist            *himlist1,
    const hdrl_imagelist      *himlist2,
    const hdrl_image          *himg,
    hdrl_value                *value){

  /* Check input */
  cpl_ensure_code(func,     CPL_ERROR_NULL_INPUT);
  cpl_ensure_code(himlist1, CPL_ERROR_NULL_INPUT);

  switch (type) {
    case HDRL_IMLIST_BASIC_IMLIST:
      cpl_ensure_code(himlist2, CPL_ERROR_NULL_INPUT);
      cpl_ensure_code(himlist1->ni == himlist2->ni,
                      CPL_ERROR_INCOMPATIBLE_INPUT);
      break;
    case HDRL_IMLIST_BASIC_IMAGE:
      cpl_ensure_code(himg, CPL_ERROR_NULL_INPUT);
      break;
    case HDRL_IMLIST_BASIC_SCALAR:
      cpl_ensure_code(value, CPL_ERROR_NULL_INPUT);
      break;
  }

  /* Loop and apply the operation */
  for (cpl_size i = 0; i < himlist1->ni; i++) {
      cpl_error_code err = CPL_ERROR_NONE;
      switch (type) {
        case HDRL_IMLIST_BASIC_IMLIST:
        {
          err = ((hdrl_f_image)func)(himlist1->images[i], himlist2->images[i]);
          break;
        }
        case HDRL_IMLIST_BASIC_IMAGE:
        {
          err = ((hdrl_f_image)func)(himlist1->images[i], himg);
          break;
        }
        case HDRL_IMLIST_BASIC_SCALAR:
          err = ((hdrl_f_scalar)func)(himlist1->images[i], *value);
          break;
      }
      cpl_ensure_code(err == CPL_ERROR_NONE, err);
  }

  return CPL_ERROR_NONE;
}

/**@}*/
