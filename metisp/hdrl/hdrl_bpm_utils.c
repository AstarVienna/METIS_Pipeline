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

#include "hdrl_types.h"
#include "hdrl_image.h"
#include "hdrl_imagelist.h"
#include "hdrl_utils.h"

#include "hdrl_bpm_utils.h"
#include "hdrl_prototyping.h"

#include <cpl.h>
#include <string.h>
#include <math.h>

/*-----------------------------------------------------------------------------
                                   Static
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_bpm_utils     Bad Pixel Utilities
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*----------------------------------------------------------------------------*/
/**
  @brief Wrapper around hdrl_bpm_filter() to filter list of images
  @param inlist     input image list
  @param kernel_nx  size in x-direction of the filtering kernel
  @param kernel_ny  size in y-direction of the filtering kernel
  @param filter     filter modes as defined in cpl
  @return   The filtered image list
  @see hdrl_bpm_filter()
 */
/*----------------------------------------------------------------------------*/
cpl_imagelist * hdrl_bpm_filter_list(
        const cpl_imagelist   *   inlist,
        cpl_size            kernel_nx,
        cpl_size            kernel_ny,
        cpl_filter_mode     filter)
{
    cpl_imagelist   *   out ;
    cpl_size            nima;
    
    /* Check Entries */
    cpl_ensure(inlist != NULL, CPL_ERROR_NULL_INPUT, NULL);
    nima = cpl_imagelist_get_size(inlist) ;

    /* Create Output list */
    out = cpl_imagelist_new() ;

    /* Loop over the imagelist */
    for (cpl_size i = 0 ; i < nima ; i++) {
        cpl_mask * mask_in, * mask_out;
        /* Convert input image to mask */
        mask_in = cpl_mask_threshold_image_create(
                      cpl_imagelist_get_const(inlist, i), -0.5, 0.5);
        cpl_mask_not(mask_in);

        /* Filter mask */
        mask_out = hdrl_bpm_filter(mask_in, kernel_nx, kernel_ny, filter) ;
        cpl_mask_delete(mask_in) ;
        if (mask_out == NULL) {
            cpl_imagelist_delete(out) ;
            return NULL ;
        }

        /* Convert mask to image and store it in the list */
        cpl_imagelist_set(out, cpl_image_new_from_mask(mask_out), i);
        cpl_mask_delete(mask_out) ;
    }
    return out ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief Allows the growing and shrinking of bad pixel masks. It can be used to
  e.g. set pixels to bad if the pixel is surrounded by other bad pixels.
  @param input_mask input mask
  @param kernel_nx  size in x-direction of the filtering kernel
  @param kernel_ny  size in y-direction of the filtering kernel
  @param filter     filter modes as defined in cpl - see below
  Supported modes:
  CPL_FILTER_EROSION, CPL_FILTER_DILATION, CPL_FILTER_OPENING,
  CPL_FILTER_CLOSING
 
  The returned mask must be deallocated using cpl_mask_delete().
  The algorithm assumes, that all pixels outside the mask are good, i.e. it
  enlarges the mask by the kernel size and marks this border as good. It
  applies on the enlarged mask the operation and extract the original-size
  mask at the very end.
 */
/*----------------------------------------------------------------------------*/
cpl_mask * hdrl_bpm_filter(
        const cpl_mask    *   input_mask,
        cpl_size        kernel_nx,
        cpl_size        kernel_ny, 
        cpl_filter_mode filter)
{
    cpl_mask * kernel;
    cpl_mask * filtered_mask;
    cpl_mask * expanded_mask;
    cpl_mask * expanded_filtered_mask;
    
    /* Check Entries */
    cpl_ensure(input_mask != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(kernel_nx >= 1, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(kernel_ny >= 1, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(filter == CPL_FILTER_EROSION || filter == CPL_FILTER_DILATION ||
            filter == CPL_FILTER_OPENING || filter == CPL_FILTER_CLOSING, 
            CPL_ERROR_ILLEGAL_INPUT, NULL);

    /* Only odd-sized masks allowed */
    cpl_ensure((kernel_nx&1) == 1, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure((kernel_ny&1) == 1, CPL_ERROR_ILLEGAL_INPUT, NULL);

    kernel = cpl_mask_new(kernel_nx, kernel_ny);
    cpl_mask_not(kernel); /* All values set to unity*/

    /* Enlarge the original mask with the kernel size and assume that outside
     * all pixels are good */
    expanded_mask = cpl_mask_new(
            cpl_mask_get_size_x(input_mask) + 2 * kernel_nx,
            cpl_mask_get_size_y(input_mask) + 2 * kernel_ny);

    cpl_mask_copy(expanded_mask, input_mask, kernel_nx + 1, kernel_ny +1 );

    expanded_filtered_mask = cpl_mask_new(cpl_mask_get_size_x(expanded_mask),
            cpl_mask_get_size_y(expanded_mask));

    if(cpl_mask_filter(expanded_filtered_mask, expanded_mask, kernel, filter,
                    CPL_BORDER_ZERO) != CPL_ERROR_NONE) {

        cpl_mask_delete(kernel);
        cpl_mask_delete(expanded_filtered_mask);
        cpl_mask_delete(expanded_mask);
        return NULL;
    }

    /* Extract the original mask from the expanded mask */
    filtered_mask = cpl_mask_extract(expanded_filtered_mask,
            kernel_nx+1, kernel_ny + 1,
            cpl_mask_get_size_x(input_mask) + kernel_nx,
            cpl_mask_get_size_y(input_mask) + kernel_ny);


    /* Free memory */
    cpl_mask_delete(kernel);
    cpl_mask_delete(expanded_filtered_mask);
    cpl_mask_delete(expanded_mask);

    return filtered_mask;
}

/*----------------------------------------------------------------------------*/
/**
  @brief convert bad pixel information mask to a cpl_mask
  @param bpm        integer image containing the bad pixel information
  @param selection  bit-mask selecting which values to set to bad
  @return cpl_mask
  @note as cpl only supports 32 bit integer images the top 32 bit of the
        selection mask must be zero
 */
/*----------------------------------------------------------------------------*/
cpl_mask * hdrl_bpm_to_mask(const cpl_image * bpm, uint64_t selection)
{
    cpl_ensure(bpm, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(cpl_image_get_type(bpm) == CPL_TYPE_INT,
               CPL_ERROR_ILLEGAL_INPUT, NULL);
    /* cpl currently only has int images, but use 64 for forward compat */
    cpl_ensure(selection <= CX_MAXUINT, CPL_ERROR_UNSUPPORTED_MODE, NULL);
    unsigned int iselection = (unsigned int)selection;
    const unsigned int * data=(const unsigned int *)cpl_image_get_data_int_const(bpm);
    const size_t nx = cpl_image_get_size_x(bpm);
    const size_t ny = cpl_image_get_size_y(bpm);
    cpl_mask * msk = cpl_mask_new(nx, ny);
    cpl_binary * dmsk = cpl_mask_get_data(msk);
    for (size_t i = 0; i < ny * nx; i++) {
        dmsk[i] = (data[i] & iselection) ? 1 : 0;
    }
    return msk;
}

/*----------------------------------------------------------------------------*/
/**
  @brief convert cpl_mask to bad pixel information mask
  @param mask       cpl_mask to be converted
  @param flag       bit-mask selecting which values to set the bad pixels to
  @return integer cpl_image
  @note as cpl only supports 32 bit integer images the top 32 bit of the
        selection mask must be zero
 */
/*----------------------------------------------------------------------------*/
cpl_image * hdrl_mask_to_bpm(const cpl_mask * mask, uint64_t flag)
{
    cpl_ensure(mask, CPL_ERROR_NULL_INPUT, NULL);
    /* cpl currently only has int images, but use 64 for forward compatibility */
    cpl_ensure(flag <= CX_MAXUINT, CPL_ERROR_UNSUPPORTED_MODE, NULL);
    unsigned int iflag = (unsigned int)flag;
    const size_t nx = cpl_mask_get_size_x(mask);
    const size_t ny = cpl_mask_get_size_y(mask);
    cpl_image * bpm = cpl_image_new(nx, ny, CPL_TYPE_INT);
    const cpl_binary * dmsk = cpl_mask_get_data_const(mask);
    unsigned int * data = (unsigned int *)cpl_image_get_data_int(bpm);
    for (size_t i = 0; i < ny * nx; i++) {
        data[i] = dmsk[i] ? iflag : 0;
    }
    return bpm;
}

/*----------------------------------------------------------------------------*/
/**
  @brief apply array of masks to an image list
  @param list  image list where the masks should be applied
  @param masks array of masks, must have same length as the image list
 
  already existing masks will be overwritten
  can be used to re-apply the original mask array returned by
  hdrl_join_mask_on_imagelist

  Example:\code{.c}
  cpl_mask ** orig_masks;
  // change mask to get partial statistics from the list
  hdrl_join_mask_on_imagelist(list, echelle_mask, &orig_masks);
  stats = get_stats_from_echelle_orders();
  // restore original mask
  hdrl_set_masks_on_imagelist(list, orig_masks);
  // free memory
  for (i = 0; i < cpl_imagelist_get_size(list); i++) {
    cpl_mask_free(orig_masks[i]);
  }
  cpl_free(orig_masks);
  \endcode
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_set_masks_on_imagelist(
        cpl_imagelist   *   list, 
        cpl_mask        **  masks)
{
    cpl_ensure_code(list, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(masks, CPL_ERROR_NULL_INPUT);

    for (size_t i = 0; i < (size_t)cpl_imagelist_get_size(list); i++) {
        cpl_image * img = cpl_imagelist_get(list, i);
        cpl_mask * img_mask = cpl_image_get_bpm(img);
        /* zero mask */
        cpl_mask_xor(img_mask, img_mask);
        /* add new mask */
        cpl_mask_or(img_mask, masks[i]);
    }
    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief join mask with existing masks in an imagelist
  @param list      imagelist where the new mask should be joined on
  @param new_mask  new mask joined with the masks in the images
  @param pold_mask pointer to array pointer to store a copy of the original
                   masks, array and masks contained in it must be deleted by
                   the user, may be NULL in which case no copy is stored
  @see hdrl_set_masks_on_imagelist
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_join_mask_on_imagelist(
        cpl_imagelist   *   list, 
        cpl_mask        *   new_mask,
        cpl_mask        *** pold_mask)
{
    cpl_ensure_code(list, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(new_mask, CPL_ERROR_NULL_INPUT);

    if (pold_mask) {
        *pold_mask = cpl_malloc(sizeof(*pold_mask) *
                                cpl_imagelist_get_size(list));
    }

    for (size_t i = 0; i < (size_t)cpl_imagelist_get_size(list); i++) {
        cpl_image * img = cpl_imagelist_get(list, i);
        cpl_mask * img_mask = cpl_image_get_bpm(img);
        if (pold_mask) {
            (*pold_mask)[i] = cpl_mask_duplicate(img_mask);
        }

        cpl_mask_or(img_mask, new_mask);
    }
    return cpl_error_get_code();
}

/**@}*/

