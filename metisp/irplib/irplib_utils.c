/*
 * This file is part of the irplib package
 * Copyright (C) 2002,2003,2014 European Southern Observatory
 *               2004  Free Software Foundation, Inc.
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

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/
#include "irplib_utils.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>

/*-----------------------------------------------------------------------------
                           Defines
 -----------------------------------------------------------------------------*/

#ifndef inline
#define inline /* inline */
#endif

/*-----------------------------------------------------------------------------
                           Missing Function Prototypes
 -----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                           Private Function Prototypes
 -----------------------------------------------------------------------------*/

inline static double irplib_data_get_double(const void *, cpl_type, int)
#ifdef CPL_HAVE_GNUC_NONNULL
     __attribute__((nonnull))
#endif
    ;

inline static void irplib_data_set_double(void *, cpl_type, int, double)
#ifdef CPL_HAVE_GNUC_NONNULL
     __attribute__((nonnull))
#endif
    ;


static
void irplib_errorstate_dump_one_level(void (*)(const char *,
                                               const char *, ...)
  #ifdef __GNUC__
      __attribute__((format (printf, 2, 3)))
  #endif
                                      , unsigned, unsigned, unsigned);
static double frame_get_exptime(const cpl_frame * pframe);
static void quicksort(int* index, double* exptime, int left, int right);

static cpl_error_code irplib_dfs_product_save(cpl_frameset *,
                                              cpl_propertylist *,
                                              const cpl_parameterlist *,
                                              const cpl_frameset *,
                                              const cpl_frame *,
                                              const cpl_imagelist *,
                                              const cpl_image *,
                                              cpl_type,
                                              const cpl_table *,
                                              const cpl_propertylist *,
                                              const char *,
                                              const cpl_propertylist *,
                                              const char *,
                                              const char *,
                                              const char *);

/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_utils     Miscellaneous Utilities
 */
/*----------------------------------------------------------------------------*/
/**@{*/


/*----------------------------------------------------------------------------*/
/**
  @brief    Dump a single CPL error at the CPL warning level
  @param    self      The number of the current error to be dumped
  @param    first     The number of the first error to be dumped
  @param    last      The number of the last error to be dumped
  @return   void
  @see cpl_errorstate_dump_one

  FIXME: Move this function to the CPL errorstate module.

 */
/*----------------------------------------------------------------------------*/
void irplib_errorstate_dump_warning(unsigned self, unsigned first,
                                    unsigned last)
{

    irplib_errorstate_dump_one_level(&cpl_msg_warning, self, first, last);

}

static cpl_polynomial * irplib_polynomial_fit_1d_create_common(
		const cpl_vector    *   x_pos,
        const cpl_vector    *   values,
        int                     degree,
        double              *   mse,
        double				*  rechisq
        );

/*----------------------------------------------------------------------------*/
/**
  @brief    Dump a single CPL error at the CPL info level
  @param    self      The number of the current error to be dumped
  @param    first     The number of the first error to be dumped
  @param    last      The number of the last error to be dumped
  @return   void
  @see cpl_errorstate_dump_one

 */
/*----------------------------------------------------------------------------*/
void irplib_errorstate_dump_info(unsigned self, unsigned first,
                                    unsigned last)
{

    irplib_errorstate_dump_one_level(&cpl_msg_info, self, first, last);

}


/*----------------------------------------------------------------------------*/
/**
  @brief    Dump a single CPL error at the CPL debug level
  @param    self      The number of the current error to be dumped
  @param    first     The number of the first error to be dumped
  @param    last      The number of the last error to be dumped
  @return   void
  @see cpl_errorstate_dump_one

 */
/*----------------------------------------------------------------------------*/
void irplib_errorstate_dump_debug(unsigned self, unsigned first,
                                    unsigned last)
{

    irplib_errorstate_dump_one_level(&cpl_msg_debug, self, first, last);

}


/*----------------------------------------------------------------------------*/
/**
  @brief  Save an image as a DFS-compliant pipeline product
  @param  allframes  The list of input frames for the recipe
  @param  parlist    The list of input parameters
  @param  usedframes The list of raw/calibration frames used for this product
  @param  image      The image to be saved
  @param  bpp        Bits per pixel
  @param  recipe     The recipe name
  @param  procat     The product category tag
  @param  applist    Optional propertylist to append to primary header or NULL
  @param  remregexp  Optional regexp of properties not to put in main header
  @param  pipe_id    PACKAGE "/" PACKAGE_VERSION
  @param  filename   Filename of created product
  @note The image may be NULL in which case only the header information is saved
        but passing a NULL image is deprecated, use cpl_dfs_save_propertylist().
  @note remregexp may be NULL
  @return CPL_ERROR_NONE or the relevant CPL error code on error
  @see cpl_dfs_save_image().

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_dfs_save_image(cpl_frameset            * allframes,
                                     const cpl_parameterlist * parlist,
                                     const cpl_frameset      * usedframes,
                                     const cpl_image         * image,
                                     cpl_type_bpp              bpp,
                                     const char              * recipe,
                                     const char              * procat,
                                     const cpl_propertylist  * applist,
                                     const char              * remregexp,
                                     const char              * pipe_id,
                                     const char              * filename)
{
    cpl_errorstate     prestate = cpl_errorstate_get();
    cpl_propertylist * prolist  = applist ? cpl_propertylist_duplicate(applist)
        : cpl_propertylist_new();

    cpl_propertylist_update_string(prolist, CPL_DFS_PRO_CATG, procat);

    irplib_dfs_save_image_(allframes, NULL, parlist, usedframes, NULL, image,
                           bpp, recipe, prolist, remregexp, pipe_id, filename);

    cpl_propertylist_delete(prolist);

    cpl_ensure_code(cpl_errorstate_is_equal(prestate), cpl_error_get_code());

    return CPL_ERROR_NONE;

}

/*----------------------------------------------------------------------------*/
/**
  @brief  Save a propertylist as a DFS-compliant pipeline product
  @param  allframes  The list of input frames for the recipe
  @param  parlist    The list of input parameters
  @param  usedframes The list of raw/calibration frames used for this product
  @param  recipe     The recipe name
  @param  procat     The product category tag
  @param  applist    Optional propertylist to append to primary header or NULL
  @param  remregexp  Optional regexp of properties not to put in main header
  @param  pipe_id    PACKAGE "/" PACKAGE_VERSION
  @param  filename   Filename of created product
  @note remregexp may be NULL
  @return CPL_ERROR_NONE or the relevant CPL error code on error
  @see cpl_dfs_save_propertylist().

 */
/*----------------------------------------------------------------------------*/
cpl_error_code
irplib_dfs_save_propertylist(cpl_frameset            * allframes,
                             const cpl_parameterlist * parlist,
                             const cpl_frameset      * usedframes,
                             const char              * recipe,
                             const char              * procat,
                             const cpl_propertylist  * applist,
                             const char              * remregexp,
                             const char              * pipe_id,
                             const char              * filename)
{
    cpl_errorstate     prestate = cpl_errorstate_get();
    cpl_propertylist * prolist  = applist ? cpl_propertylist_duplicate(applist)
        : cpl_propertylist_new();

    cpl_propertylist_update_string(prolist, CPL_DFS_PRO_CATG, procat);

    cpl_dfs_save_propertylist(allframes, NULL, parlist, usedframes, NULL,
                              recipe, prolist, remregexp, pipe_id, filename);

    cpl_propertylist_delete(prolist);

    cpl_ensure_code(cpl_errorstate_is_equal(prestate), cpl_error_get_code());

    return CPL_ERROR_NONE;

}

/*----------------------------------------------------------------------------*/
/**
  @brief  Save an imagelist as a DFS-compliant pipeline product
  @param  allframes  The list of input frames for the recipe
  @param  parlist    The list of input parameters
  @param  usedframes The list of raw/calibration frames used for this product
  @param  imagelist  The imagelist to be saved
  @param  bpp        Bits per pixel
  @param  recipe     The recipe name
  @param  procat     The product category tag
  @param  applist    Optional propertylist to append to primary header or NULL
  @param  remregexp  Optional regexp of properties not to put in main header
  @param  pipe_id    PACKAGE "/" PACKAGE_VERSION
  @param  filename   Filename of created product
  @note remregexp may be NULL
  @return CPL_ERROR_NONE or the relevant CPL error code on error
  @see cpl_dfs_save_imagelist().

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_dfs_save_imagelist(cpl_frameset            * allframes,
                                         const cpl_parameterlist * parlist,
                                         const cpl_frameset      * usedframes,
                                         const cpl_imagelist     * imagelist,
                                         cpl_type_bpp              bpp,
                                         const char              * recipe,
                                         const char              * procat,
                                         const cpl_propertylist  * applist,
                                         const char              * remregexp,
                                         const char              * pipe_id,
                                         const char              * filename)
{
    cpl_errorstate     prestate = cpl_errorstate_get();
    cpl_propertylist * prolist  = applist ? cpl_propertylist_duplicate(applist)
        : cpl_propertylist_new();

    cpl_propertylist_update_string(prolist, CPL_DFS_PRO_CATG, procat);

    cpl_dfs_save_imagelist(allframes, NULL, parlist, usedframes, NULL,
                           imagelist, bpp, recipe, prolist, remregexp, pipe_id,
                           filename);

    cpl_propertylist_delete(prolist);

    cpl_ensure_code(cpl_errorstate_is_equal(prestate), cpl_error_get_code());

    return CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @brief  Save a table as a DFS-compliant pipeline product
  @param  allframes  The list of input frames for the recipe
  @param  parlist    The list of input parameters
  @param  usedframes The list of raw/calibration frames used for this product
  @param  table      The table to be saved
  @param  tablelist  Optional propertylist to use in table extension or NULL
  @param  recipe     The recipe name
  @param  procat     The product category tag
  @param  applist    Optional propertylist to append to primary header or NULL
  @param  remregexp  Optional regexp of properties not to put in main header
  @param  pipe_id    PACKAGE "/" PACKAGE_VERSION
  @param  filename   Filename of created product
  @return CPL_ERROR_NONE or the relevant CPL error code on error
  @see cpl_dfs_save_table().

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_dfs_save_table(cpl_frameset            * allframes,
                                     const cpl_parameterlist * parlist,
                                     const cpl_frameset      * usedframes,
                                     const cpl_table         * table,
                                     const cpl_propertylist  * tablelist,
                                     const char              * recipe,
                                     const char              * procat,
                                     const cpl_propertylist  * applist,
                                     const char              * remregexp,
                                     const char              * pipe_id,
                                     const char              * filename)
{

    cpl_errorstate     prestate = cpl_errorstate_get();
    cpl_propertylist * prolist  = applist ? cpl_propertylist_duplicate(applist)
        : cpl_propertylist_new();

    cpl_propertylist_update_string(prolist, CPL_DFS_PRO_CATG, procat);

    cpl_dfs_save_table(allframes, NULL, parlist, usedframes, NULL,
                           table, tablelist, recipe, prolist, remregexp,
                           pipe_id, filename);

    cpl_propertylist_delete(prolist);

    cpl_ensure_code(cpl_errorstate_is_equal(prestate), cpl_error_get_code());

    return CPL_ERROR_NONE;
}



/*----------------------------------------------------------------------------*/
/**
  @brief  Save an image as a DFS-compliant pipeline product
  @param  allframes  The list of input frames for the recipe
  @param  header     NULL, or filled with properties written to product header
  @param  parlist    The list of input parameters
  @param  usedframes The list of raw/calibration frames used for this product
  @param  inherit    NULL or product frames inherit their header from this frame
  @param  image      The image to be saved
  @param  type       The type used to represent the data in the file
  @param  recipe     The recipe name
  @param  applist    Propertylist to append to primary header, w. PRO.CATG
  @param  remregexp  Optional regexp of properties not to put in main header
  @param  pipe_id    PACKAGE "/" PACKAGE_VERSION
  @param  filename   Filename of created product
  @note The image may be NULL in which case only the header information is saved
        but passing a NULL image is deprecated, use cpl_dfs_save_propertylist().
  @note remregexp may be NULL
  @note applist must contain a string-property with key CPL_DFS_PRO_CATG
  @note On success and iff header is non-NULL, it will be emptied and then
        filled with the properties written to the primary header of the product
  @return CPL_ERROR_NONE or the relevant CPL error code on error
  @see cpl_dfs_save_image()
  @note applist is copied with cpl_propertylist_copy_property_regexp() instead
        pf cpl_propertylist_append()

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_dfs_save_image_(cpl_frameset            * allframes,
                                      cpl_propertylist        * header,
                                      const cpl_parameterlist * parlist,
                                      const cpl_frameset      * usedframes,
                                      const cpl_frame         * inherit,
                                      const cpl_image         * image,
                                      cpl_type                  type,
                                      const char              * recipe,
                                      const cpl_propertylist  * applist,
                                      const char              * remregexp,
                                      const char              * pipe_id,
                                      const char              * filename)
{
    return
        irplib_dfs_product_save(allframes, header, parlist, usedframes, inherit,
                                NULL, image, type, NULL, NULL, recipe,
                                applist, remregexp, pipe_id, filename)
        ? cpl_error_set_where(cpl_func) : CPL_ERROR_NONE;

}


/*----------------------------------------------------------------------------*/
/**
  @brief  Save either an image or table as a pipeline product
  @param  allframes  The list of input frames for the recipe
  @param  header     NULL, or filled with properties written to product header
  @param  parlist    The list of input parameters
  @param  usedframes The list of raw/calibration frames used for this product
  @param  inherit    NULL, or frame from which header information is inherited
  @param  imagelist  The imagelist to be saved or NULL
  @param  image      The image to be saved or NULL
  @param  type       The type used to represent the data in the file
  @param  table      The table to be saved or NULL
  @param  tablelist  Optional propertylist to use in table extension or NULL
  @param  recipe     The recipe name
  @param  applist    Optional propertylist to append to main header or NULL
  @param  remregexp  Optional regexp of properties not to put in main header
  @param  pipe_id    PACKAGE "/" PACKAGE_VERSION
  @param  filename   Filename of created product
  @return CPL_ERROR_NONE or the relevant CPL error code on error
  @see cpl_dfs_product_save()
  @note applist is copied with cpl_propertylist_copy_property_regexp() instead
        pf cpl_propertylist_append()

 */
/*----------------------------------------------------------------------------*/

static
cpl_error_code irplib_dfs_product_save(cpl_frameset            * allframes,
                                       cpl_propertylist        * header,
                                       const cpl_parameterlist * parlist,
                                       const cpl_frameset      * usedframes,
                                       const cpl_frame         * inherit,
                                       const cpl_imagelist     * imagelist,
                                       const cpl_image         * image,
                                       cpl_type                  type,
                                       const cpl_table         * table,
                                       const cpl_propertylist  * tablelist,
                                       const char              * recipe,
                                       const cpl_propertylist  * applist,
                                       const char              * remregexp,
                                       const char              * pipe_id,
                                       const char              * filename) {

    const char       * procat;
    cpl_propertylist * plist;
    cpl_frame        * product_frame;
    /* Inside this function the product-types are numbered:
       0: imagelist
       1: table
       2: image
       3: propertylist only
    */
    const unsigned     pronum
        = imagelist != NULL ? 0 : table != NULL ? 1 :  (image != NULL ? 2 : 3);
    const char       * proname[] = {"imagelist", "table", "image",
                                    "propertylist"};
    /* FIXME: Define a frame type for an imagelist and when data-less */
    const int          protype[] = {CPL_FRAME_TYPE_ANY, CPL_FRAME_TYPE_TABLE,
                                    CPL_FRAME_TYPE_IMAGE, CPL_FRAME_TYPE_ANY};
    cpl_error_code     error = CPL_ERROR_NONE;


    /* No more than one of imagelist, table and image may be non-NULL */
    /* tablelist may only be non-NULL when table is non-NULL */
    if (imagelist != NULL) {
        assert(pronum == 0);
        assert(image == NULL);
        assert(table == NULL);
        assert(tablelist == NULL);
    } else if (table != NULL) {
        assert(pronum == 1);
        assert(imagelist == NULL);
        assert(image == NULL);
    } else if (image != NULL) {
        assert(pronum == 2);
        assert(imagelist == NULL);
        assert(table == NULL);
        assert(tablelist == NULL);
    } else {
        assert(pronum == 3);
        assert(imagelist == NULL);
        assert(table == NULL);
        assert(tablelist == NULL);
        assert(image == NULL);
    }

    cpl_ensure_code(allframes  != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(parlist    != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(usedframes != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(recipe     != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(applist    != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(pipe_id    != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(filename   != NULL, CPL_ERROR_NULL_INPUT);

    procat = cpl_propertylist_get_string(applist, CPL_DFS_PRO_CATG);

    cpl_ensure_code(procat     != NULL, cpl_error_get_code());

    cpl_msg_info(cpl_func, "Writing FITS %s product(%s): %s", proname[pronum],
                 procat, filename);

    product_frame = cpl_frame_new();

    /* Create product frame */
    error |= cpl_frame_set_filename(product_frame, filename);
    error |= cpl_frame_set_tag(product_frame, procat);
    error |= cpl_frame_set_type(product_frame, protype[pronum]);
    error |= cpl_frame_set_group(product_frame, CPL_FRAME_GROUP_PRODUCT);
    error |= cpl_frame_set_level(product_frame, CPL_FRAME_LEVEL_FINAL);

    if (error) {
        cpl_frame_delete(product_frame);
        return cpl_error_set_where(cpl_func);
    }

    if (header != NULL) {
        cpl_propertylist_empty(header);
        plist = header;
    } else {
        plist = cpl_propertylist_new();
    }

    /* Add any QC parameters here */
    if (applist != NULL) error = cpl_propertylist_copy_property_regexp(plist,
                                                                       applist,
                                                                       ".", 0);

    /* Add DataFlow keywords */
    if (!error)
        error = cpl_dfs_setup_product_header(plist, product_frame, usedframes,
                                             parlist, recipe, pipe_id,
                                             "PRO-1.16", inherit);

    if (remregexp != NULL && !error) {
        cpl_errorstate prestate = cpl_errorstate_get();
        (void)cpl_propertylist_erase_regexp(plist, remregexp, 0);
        if (!cpl_errorstate_is_equal(prestate)) error = cpl_error_get_code();
    }

    if (!error) {
        switch (pronum) {
            case 0:
                error = cpl_imagelist_save(imagelist, filename, type, plist,
                                           CPL_IO_CREATE);
                break;
            case 1:
                error = cpl_table_save(table, plist, tablelist, filename,
                                       CPL_IO_CREATE);
                break;
            case 2:
                error = cpl_image_save(image, filename, type, plist,
                                       CPL_IO_CREATE);
                break;
            default:
                /* case 3: */
                error = cpl_propertylist_save(plist, filename, CPL_IO_CREATE);
        }
    }

    if (!error) {
        /* Insert the frame of the saved file in the input frameset */
        error = cpl_frameset_insert(allframes, product_frame);

    } else {
        cpl_frame_delete(product_frame);
    }

    if (plist != header) cpl_propertylist_delete(plist);

    cpl_ensure_code(!error, error);

    return CPL_ERROR_NONE;

}    


/*----------------------------------------------------------------------------*/
/**
  @brief Split the values in an image in three according to two thresholds
  @param self       The image to split
  @param im_low     If non-NULL low-valued pixels are assigned to this image
  @param im_mid     If non-NULL middle-valued pixels are assigned to this image
  @param im_high    If non-NULL high-valued pixels are assigned to this image
  @param th_low     The lower threshold
  @param isleq_low  Ift true use less than or equal
  @param th_high    The upper threshold, must be at least th_low
  @param isgeq_high Iff true use greater than or equal
  @param alt_low    Assign this value when the pixel value is not low
  @param alt_high   Assign this value, when the pixel value is not high
  @param isbad_low  Flag non-low pixels as bad
  @param isbad_mid  Flag non-mid pixels as bad
  @param isbad_high Flag non-high pixels as bad
  @return CPL_ERROR_NONE or the relevant CPL error code on error
  @note At least one output image must be non-NULL; all non-NULL images must
        be of identical size, but may be of any pixel-type.
        self may be passed as one of the output images for an in-place split.


   FIXME: This function is way too slow and perhaps over-engineered...

  A split in two is achieved with th_low equal th_high (in this case there
  is little reason for im_mid to be non-NULL).

  All pixel values in the output images are reset,
  as well as their bad pixels maps.

  If an input pixel-value is flagged as bad, then the receiving pixel in the
  output image is flagged as well.

   @par The same image may be passed more than once which allows a split
        into one image with the mid-valued pixels and another with both the
        low and high-valued pixels, i.e.
   @code
       irplib_image_split(source, dest, im_mid, dest,
                          th_low,  isleq_low, th_high, isgeq_high,
                          alt_low, alt_high,
                          isbad_low, isbad_mid, isbad_high);
   @endcode

   @par These two calls are equivalent:
   @code
     cpl_image_threshold(img, th_low, th_high, alt_low, alt_high);
   @endcode

   @code
     irplib_image_split(img, NULL, img, NULL,
                        th_low,  CPL_TRUE, th_high, CPL_TRUE,
                        alt_low, alt_high, dontcare, CPL_FALSE, dontcare);
   @endcode

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_image_split(const cpl_image * self,
                                  cpl_image * im_low,
                                  cpl_image * im_mid,
                                  cpl_image * im_high,
                                  double th_low,
                                  cpl_boolean isleq_low,
                                  double th_high,
                                  cpl_boolean isgeq_high,
                                  double alt_low,
                                  double alt_high,
                                  cpl_boolean isbad_low,
                                  cpl_boolean isbad_mid,
                                  cpl_boolean isbad_high)
{

    const void       * selfdata = cpl_image_get_data_const(self);
    /* hasbpm reduces check-overhead if self does not have a bpm, and if
       self is also passed as an output image, that ends up with bad pixels */
    /* FIXME: Need a proper way to know if a bpm has been allocated :-((((((( */
    const cpl_boolean  hasbpm
        = cpl_image_count_rejected(self) ? CPL_TRUE : CPL_FALSE;
    const cpl_binary * selfbpm = hasbpm
        ? cpl_mask_get_data_const(cpl_image_get_bpm_const(self)) : NULL;
    const cpl_type     selftype = cpl_image_get_type(self);
    const int          nx = cpl_image_get_size_x(self);
    const int          ny = cpl_image_get_size_y(self);
    const int          npix = nx * ny;
    const cpl_boolean  do_low   = im_low  != NULL;
    const cpl_boolean  do_mid   = im_mid  != NULL;
    const cpl_boolean  do_high  = im_high != NULL;
    void             * lowdata  = NULL;
    void             * middata  = NULL;
    void             * highdata = NULL;
    cpl_binary       * lowbpm   = NULL;
    cpl_binary       * midbpm   = NULL;
    cpl_binary       * highbpm  = NULL;
    const cpl_type     lowtype
        = do_low ? cpl_image_get_type(im_low) : CPL_TYPE_INVALID;
    const cpl_type     midtype
        = do_mid ? cpl_image_get_type(im_mid) : CPL_TYPE_INVALID;
    const cpl_type     hightype
        = do_high ? cpl_image_get_type(im_high) : CPL_TYPE_INVALID;
    int i;


    cpl_ensure_code(self != NULL,                CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(do_low || do_mid || do_high, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(th_low <= th_high,           CPL_ERROR_ILLEGAL_INPUT);

    if (do_low) {
        cpl_ensure_code(cpl_image_get_size_x(im_low) == nx,
                        CPL_ERROR_INCOMPATIBLE_INPUT);
        cpl_ensure_code(cpl_image_get_size_y(im_low) == ny,
                        CPL_ERROR_INCOMPATIBLE_INPUT);
        lowdata = cpl_image_get_data(im_low);
    }

    if (do_mid) {
        cpl_ensure_code(cpl_image_get_size_x(im_mid) == nx,
                        CPL_ERROR_INCOMPATIBLE_INPUT);
        cpl_ensure_code(cpl_image_get_size_y(im_mid) == ny,
                        CPL_ERROR_INCOMPATIBLE_INPUT);
        middata = cpl_image_get_data(im_mid);
    }

    if (do_high) {
        cpl_ensure_code(cpl_image_get_size_x(im_high) == nx,
                        CPL_ERROR_INCOMPATIBLE_INPUT);
        cpl_ensure_code(cpl_image_get_size_y(im_high) == ny,
                        CPL_ERROR_INCOMPATIBLE_INPUT);
        highdata = cpl_image_get_data(im_high);
    }

    /* From this point a failure would indicate a serious bug in CPL */

    for (i = 0; i < npix; i++) {
        const double value = irplib_data_get_double(selfdata, selftype, i);
        cpl_boolean  isalt_low   = do_low;
        cpl_boolean  isalt_mid   = do_mid;
        cpl_boolean  isalt_high  = do_high;
        cpl_boolean  setbad_low  = do_low;
        cpl_boolean  setbad_mid  = do_mid;
        cpl_boolean  setbad_high = do_high;
        const void * setdata     = NULL;
        double       alt_mid     = 0.0; /* Avoid (false) uninit warning */

        if (isleq_low ? value <= th_low : value < th_low) {
            if (do_low) {
                isalt_low = CPL_FALSE;
                irplib_data_set_double(lowdata, lowtype, i, value);
                setbad_low = hasbpm && selfbpm[i];
                setdata = lowdata;
            }
            alt_mid = alt_low;
        } else if (isgeq_high ? value >= th_high : value > th_high) {
            if (do_high) {
                isalt_high = CPL_FALSE;
                irplib_data_set_double(highdata, hightype, i, value);
                setbad_high = hasbpm && selfbpm[i];
                setdata = highdata;
            }
            alt_mid = alt_high;
        } else if (do_mid) {
            isalt_mid = CPL_FALSE;
            irplib_data_set_double(middata, midtype, i, value);
            setbad_mid = hasbpm && selfbpm[i];
            setdata = middata;
        }

        if (isalt_low && lowdata != setdata) {
            irplib_data_set_double(lowdata, lowtype, i, alt_low);
            setbad_low = isbad_low;
        }
        if (isalt_mid && middata != setdata) {
            irplib_data_set_double(middata, midtype, i, alt_mid);
            setbad_mid = isbad_mid;
        }
        if (isalt_high && highdata != setdata) {
            irplib_data_set_double(highdata, hightype, i, alt_high);
            setbad_high = isbad_high;
        }

        if (setbad_low) {
            if (lowbpm == NULL) lowbpm
                = cpl_mask_get_data(cpl_image_get_bpm(im_low));
            lowbpm[i] = CPL_BINARY_1;
        }
        if (setbad_mid) {
            if (midbpm == NULL) midbpm
                = cpl_mask_get_data(cpl_image_get_bpm(im_mid));
            midbpm[i] = CPL_BINARY_1;
        }
        if (setbad_high) {
            if (highbpm == NULL) highbpm
                = cpl_mask_get_data(cpl_image_get_bpm(im_high));
            highbpm[i] = CPL_BINARY_1;
        }
    }

    return CPL_ERROR_NONE;

}


/*----------------------------------------------------------------------------*/
/**
  @brief  Create a DFS product with one table from one or more (ASCII) file(s)
  @param  self         Table with labels (and units) but no row data
  @param  allframes    The list of input frames for the recipe
  @param  useframes    The frames to process for the product
  @param  maxlinelen   The maximum line length in the input file(s)
  @param  commentchar  Skip lines that start with this character, e.g. '#'
  @param  product_name The name of the created FITS table product or NULL
  @param  procatg      The PROCATG of the created FITS table product
  @param  parlist      The list of input parameters
  @param  recipe_name  The name of the calling recipe
  @param  mainlist     Optional propertylist to append to main header or NULL
  @param  extlist      Optional propertylist to append to ext. header or NULL
  @param  remregexp    Optional regexp of properties not to put in main header
  @param  instrume     The value to use for the INSTRUME key, uppercase PACKAGE
  @param  pipe_id      PACKAGE "/" PACKAGE_VERSION
  @param  table_set_row Caller-defined function to insert one row in the table
  @param  table_check  Optional caller-defined function to check table or NULL
  @return CPL_ERROR_NONE or the relevant CPL error code on error
  @see irplib_table_read_from_frameset(), cpl_dfs_save_table()
  @note If product_name is NULL, the product will be named <recipe_name>.fits.

   @par Example (error handling omitted for brevity):
   @code
      extern cpl_boolean my_table_set_row(cpl_table *, const char *, int,
                                          const cpl_frame *,
                                          const cpl_parameterlist *);
      extern cpl_error_code my_table_check(cpl_table *,
                                           const cpl_frameset *,
                                           const cpl_parameterlist *);
      const int expected_rows = 42;
      cpl_table * self = cpl_table_new(expected_rows);

      cpl_table_new_column(self, "MYLABEL1", CPL_TYPE_STRING);
      cpl_table_new_column(self, "MYLABEL2", CPL_TYPE_DOUBLE);
      cpl_table_set_column_unit(self, "MYLABEL2", "Some_SI_Unit");

      irplib_dfs_table_convert(self, allframes, useframes, 1024, '#', NULL,
                               "MYPROCATG", parlist, "myrecipe", NULL, NULL,
                               NULL, "MYINSTRUME", PACKAGE "/" PACKAGE_VERSION,
                               my_table_set_row, my_table_check);

      cpl_table_delete(self);

   @endcode

 */
/*----------------------------------------------------------------------------*/

cpl_error_code
irplib_dfs_table_convert(cpl_table               * self,
                         cpl_frameset            * allframes,
                         const cpl_frameset      * useframes,
                         int                       maxlinelen,
                         char                      commentchar,
                         const char              * product_name,
                         const char              * procatg,
                         const cpl_parameterlist * parlist,
                         const char              * recipe_name,
                         const cpl_propertylist  * mainlist,
                         const cpl_propertylist  * extlist,
                         const char              * remregexp,
                         const char              * instrume,
                         const char              * pipe_id,
                         cpl_boolean (*table_set_row)
                         (cpl_table *, const char *, int,
                          const cpl_frame *,
                          const cpl_parameterlist *),
                         cpl_error_code (*table_check)
                         (cpl_table *,
                          const cpl_frameset *,
                          const cpl_parameterlist *))
{

    const char       * filename;
    cpl_propertylist * applist    = NULL;
    cpl_errorstate     prestate   = cpl_errorstate_get();
    cpl_error_code     error;
    char             * fallback_filename = NULL;

    cpl_ensure_code(self         != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(allframes    != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(useframes    != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(procatg      != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(parlist      != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(recipe_name  != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(instrume     != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(pipe_id      != NULL, CPL_ERROR_NULL_INPUT);

    cpl_ensure_code(!irplib_table_read_from_frameset(self, useframes,
                                                     maxlinelen,
                                                     commentchar,
                                                     parlist,
                                                     table_set_row),
                    cpl_error_get_code());

    if (table_check != NULL && (table_check(self, useframes, parlist) ||
                                !cpl_errorstate_is_equal(prestate))) {
        return cpl_error_set_message(cpl_func, cpl_error_get_code(),
                                     "Consistency check of table failed");
    }

    fallback_filename = cpl_sprintf("%s" CPL_DFS_FITS, recipe_name);
    filename = product_name != NULL ? product_name : fallback_filename;

    applist = mainlist == NULL
        ? cpl_propertylist_new() : cpl_propertylist_duplicate(mainlist);

    error = cpl_propertylist_update_string(applist, "INSTRUME", instrume);

    if (!error)
        error = irplib_dfs_save_table(allframes, parlist, useframes, self,
                                      extlist, recipe_name, procatg, applist,
                                      remregexp, pipe_id, filename);

    cpl_propertylist_delete(applist);
    cpl_free(fallback_filename);

    /* Propagate the error, if any */
    cpl_ensure_code(!error, error);

    return CPL_ERROR_NONE;

}



/*----------------------------------------------------------------------------*/
/**
  @brief  Set the rows of a table with data from one or more (ASCII) files
  @param  self           Table with labels (and units) but no row data
  @param  useframes      The frames to process for the table
  @param  maxlinelen     The maximum line length in the input file(s)
  @param  commentchar    Skip lines that start with this character, e.g. '#'
  @param  parlist        The list of input parameters
  @param  table_set_row  Caller-defined function to insert one row in the table
  @return CPL_ERROR_NONE or the relevant CPL error code on error

  table_set_row() is a function that sets the specified row in a table
  - it may optionally include a check of the line for consistency.
  An integer is passed to table_set_row() to indicate which row
  to set. Instead of setting the row table_set_row() may decide to discard
  the data. Iff the row was set, table_set_row() should return CPL_TRUE.

  It needs to know:
    1) How to parse the lines - each line is read with fgets().
    2) For each column: type/format (%lg/%s/%d) + label

  During a successful call self will have rows added or removed to exactly
  match the number of lines converted. Any a priori knowledge about the
  expected number of converted rows can be used in the creation of the
  table (to reduce memory reallocation overhead).
  On error the number of rows in self is undefined.

   @par Example (error handling omitted for brevity):
   @code
      extern cpl_boolean my_table_set_row(cpl_table *, const char *, int,
                                          const cpl_frame *,
                                          const cpl_parameterlist *);
      const int expected_rows = 42;
      cpl_table * self = cpl_table_new(expected_rows);

      cpl_table_new_column(self, "MYLABEL1", CPL_TYPE_STRING);
      cpl_table_new_column(self, "MYLABEL2", CPL_TYPE_DOUBLE);
      cpl_table_set_column_unit(self, "MYLABEL2", "Some_SI_Unit");

      irplib_table_read_from_frameset(self, useframes, 1024, '#', parlist,
                                      my_table_set_row);

      // Use self...

      cpl_table_delete(self);

   @endcode

 */
/*----------------------------------------------------------------------------*/

cpl_error_code
irplib_table_read_from_frameset(cpl_table               * self,
                                const cpl_frameset      * useframes,
                                int                       maxlinelen,
                                char                      commentchar,
                                const cpl_parameterlist * parlist,
                                cpl_boolean (*table_set_row)
                                (cpl_table *, const char *, int,
                                 const cpl_frame *,
                                 const cpl_parameterlist *))
{

    const cpl_frame  * rawframe;
    char             * linebuffer = NULL;
    FILE             * stream     = NULL;
    int                nfiles     = 0;
    int                nrow       = cpl_table_get_nrow(self);
    int                irow       = 0;
    cpl_errorstate     prestate   = cpl_errorstate_get();
    cpl_frameset_iterator * iterator = NULL;

    cpl_ensure_code(self         != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(useframes    != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(maxlinelen > 0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(parlist      != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(table_set_row != NULL, CPL_ERROR_NULL_INPUT);

    linebuffer = cpl_malloc(maxlinelen);

    for (rawframe = irplib_frameset_get_first_const(&iterator, useframes);
         rawframe != NULL;
         rawframe = irplib_frameset_get_next_const(iterator), nfiles++) {

        const char * rawfile = cpl_frame_get_filename(rawframe);
        const char * done; /* Indicate when the reading is done */
        const int irowpre = irow;
        int iirow = 0;
        int ierror;

        if (rawfile == NULL) break; /* Should not happen... */

        stream = fopen(rawfile, "r");

        if (stream == NULL) {
#if defined CPL_HAVE_VA_ARGS && CPL_HAVE_VA_ARGS != 0
            cpl_error_set_message(cpl_func, CPL_ERROR_FILE_IO, "Could not "
                                  "open %s for reading", rawfile);
#else
            cpl_error_set_message(cpl_func, CPL_ERROR_FILE_IO, "Could not "
                                  "open file for reading");
#endif
            break;
        }

        for (;(done = fgets(linebuffer, maxlinelen, stream)) != NULL; iirow++) {

            if (linebuffer[0] != commentchar) {
                cpl_boolean didset;
#if defined CPL_HAVE_VA_ARGS && CPL_HAVE_VA_ARGS != 0
                const int prerow = irow;
#endif

                if (irow == nrow) {
                    nrow += nrow ? nrow : 1;
                    if (cpl_table_set_size(self, nrow)) break;
                }

                didset = table_set_row(self, linebuffer, irow, rawframe,
                                       parlist);
                if (didset) irow++;

                if (!cpl_errorstate_is_equal(prestate)) {
                    if (didset)
#if defined CPL_HAVE_VA_ARGS && CPL_HAVE_VA_ARGS != 0
                        cpl_error_set_message(cpl_func, cpl_error_get_code(),
                                              "Failed to set table row %d "
                                              "using line %d from %d. file %s",
                                              1+prerow, iirow+1,
                                              nfiles+1, rawfile);
                    else
                        cpl_error_set_message(cpl_func, cpl_error_get_code(),
                                              "Failure with line %d from %d. "
                                              "file %s", iirow+1,
                                              nfiles+1, rawfile);
#else
                        cpl_error_set_message(cpl_func, cpl_error_get_code(),
                                              "Failed to set table row"
                                              "using catalogue line");
                    else
                        cpl_error_set_message(cpl_func, cpl_error_get_code(),
                                              "Failure with catalogue line");
#endif

                    break;
                }
            }
        }
        if (done != NULL) break;

        ierror = fclose(stream);
        stream = NULL;
        if (ierror) break;


        if (irow == irowpre)
            cpl_msg_warning(cpl_func, "No usable lines in the %d. file: %s",
                            1+nfiles, rawfile);
    }

    cpl_frameset_iterator_delete(iterator);
    cpl_free(linebuffer);
    if (stream != NULL) fclose(stream);

    /* Check for premature end */
    cpl_ensure_code(rawframe == NULL, cpl_error_get_code());

    if (irow == 0) {
#if defined CPL_HAVE_VA_ARGS && CPL_HAVE_VA_ARGS != 0
        return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                                     "No usable lines in the %d input "
                                     "frame(s)", nfiles);
#else
        return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                                     "No usable lines in the input frame(s)");
#endif
    }

    /* Resize the table to the actual number of rows set */
    cpl_ensure_code(!cpl_table_set_size(self, irow), cpl_error_get_code());

    return CPL_ERROR_NONE;
}



/*----------------------------------------------------------------------------*/
/**
  @brief    Reset IRPLIB state

  This function resets all static memory used by IRPLIB to a well-defined,
  initial state.

  The function should be called (during initialization) by any application
  using static memory facilities in IRPLIB.

  Currently, this function does nothing.
 */
/*----------------------------------------------------------------------------*/
void irplib_reset(void)
{
    return;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Comparison function to identify different input frames
  @param    frame1  first frame
  @param    frame2  second frame
  @return   0 if frame1!=frame2, 1 if frame1==frame2, -1 in error case
 */
/*----------------------------------------------------------------------------*/
int irplib_compare_tags(
        cpl_frame   *   frame1,
        cpl_frame   *   frame2)
{
    const char            *   v1 ;
    const char            *   v2 ;

    /* Test entries */
    if (frame1==NULL || frame2==NULL) return -1 ;

    /* Get the tags */
    if ((v1 = cpl_frame_get_tag(frame1)) == NULL) return -1 ;
    if ((v2 = cpl_frame_get_tag(frame2)) == NULL) return -1 ;

    /* Compare the tags */
    if (strcmp(v1, v2)) return 0 ;
    else return 1 ;
}

/*----------------------------------------------------------------------------*/
/**
   @brief Find the filename with the given tag in a frame set.
   @param self  A frame set.
   @param tag   The frame tag to search for.
   @return The filename or NULL if none found and on error.
   @see cpl_frameset_find
   @note If called with a CPL error code, the location will be updated and NULL
     returned.

   NULL is returned and no error code set if the tag is not found.

   If the file is not unique, the name of the first one is returned and with
   a warning.

 */
/*----------------------------------------------------------------------------*/
const char * irplib_frameset_find_file(const cpl_frameset * self,
                                      const char * tag)
{
    const cpl_frame * frame = cpl_frameset_find_const(self, tag);


    cpl_ensure(!cpl_error_get_code(), cpl_error_get_code(), NULL);

    if (frame == NULL) return NULL;

    if (cpl_frameset_find_const(self, NULL))
        cpl_msg_warning(cpl_func,
			"Frameset has more than one file with tag: %s",
                        tag);

    return cpl_frame_get_filename(frame);

}

/*----------------------------------------------------------------------------*/
/**
  @brief    Find the first frame belonging to the given group
  @param    self   The frameset
  @param    group  The group attribute
  @return   The first frame belonging to the given group, or @c NULL if no
            such frame was found. The function returns @c NULL if an error
            occurs and sets the appropriate error code.

 */
/*----------------------------------------------------------------------------*/
const
cpl_frame * irplib_frameset_get_first_from_group(const cpl_frameset * self,
                                                 cpl_frame_group      group)
{
    const cpl_frame * frame;
    cpl_frameset_iterator * iterator = NULL;

    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, NULL);

    for (frame = irplib_frameset_get_first_const(&iterator, self);
         frame != NULL ;
         frame = irplib_frameset_get_next_const(iterator)) {
        if (cpl_frame_get_group(frame) == group) break;
    }
    cpl_frameset_iterator_delete(iterator);
    return frame;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Find the aperture(s) with the greatest flux
  @param    self   The aperture object
  @param    ind  The aperture-indices in order of decreasing flux
  @param    nfind  Number of indices to find
  @return   CPL_ERROR_NONE or the relevant _cpl_error_code_ on error

  nfind must be at least 1 and at most the size of the aperture object.

  The ind array must be able to hold (at least) nfind integers.
  On success the first nfind elements of ind point to indices of the
  aperture object.

  To find the single ind of the aperture with the maximum flux use simply:
  int ind;
  irplib_apertures_find_max_flux(self, &ind, 1);

 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_apertures_find_max_flux(const cpl_apertures * self,
                                              int * ind, int nfind)
{
    const int    nsize = cpl_apertures_get_size(self);
    int          ifind;


    cpl_ensure_code(nsize > 0,      cpl_error_get_code());
    cpl_ensure_code(ind,          CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(nfind > 0,      CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(nfind <= nsize, CPL_ERROR_ILLEGAL_INPUT);

    for (ifind=0; ifind < nfind; ifind++) {
        double maxflux = -1;
        int maxind = -1;
        int i;
        for (i=1; i <= nsize; i++) {
            int k;

            /* The flux has to be the highest among those not already found */
            for (k=0; k < ifind; k++) if (ind[k] == i) break;

            if (k == ifind) {
                /* i has not been inserted into ind */
                const double flux = cpl_apertures_get_flux(self, i);

                if (maxind < 0 || flux > maxflux) {
                    maxind = i;
                    maxflux = flux;
                }
            }
        }
        ind[ifind] = maxind;
    }

    return CPL_ERROR_NONE;

}

/**@}*/



/*----------------------------------------------------------------------------*/
/**
  @brief  Optimized version of cpl_image_get()
  @param  self  A void pointer, e.g. from cpl_image_get_data_const()
  @param  type  The data type, e.g. from  cpl_image_get_type()
  @param  i     The index into the buffer after it has been cast to type
  @return The value cast to double
  @note This function has no error checking for speed
  @see cpl_image_get()

 */
/*----------------------------------------------------------------------------*/
inline static
double irplib_data_get_double(const void * self, cpl_type type, int i)
{

    double value;


    switch (type) {
    case CPL_TYPE_FLOAT:
        {
            const float * pself = (const float*)self;
            value = (double)pself[i];
            break;
        }
    case CPL_TYPE_INT:
        {
            const int * pself = (const int*)self;
            value = (double)pself[i];
            break;
        }
    default: /* case CPL_TYPE_DOUBLE */
        {
            const double * pself = (const double*)self;
            value = pself[i];
            break;
        }
    }

    return value;

}


/*----------------------------------------------------------------------------*/
/**
  @brief  Optimized version of cpl_image_set()
  @param  self   A void pointer, e.g. from cpl_image_get_data_const()
  @param  type   The data type, e.g. from  cpl_image_get_type()
  @param  i      The index into the buffer after it has been cast to type
  @param  value  The value to write
  @note This function has no error checking for speed
  @see cpl_image_set()

 */
/*----------------------------------------------------------------------------*/
inline static
void irplib_data_set_double(void * self, cpl_type type, int i, double value)
{

    switch (type) {
    case CPL_TYPE_FLOAT:
        {
            float * pself = (float*)self;
            pself[i] = (float)value;
            break;
        }
    case CPL_TYPE_INT:
        {
            int * pself = (int*)self;
            pself[i] = (int)value;
            break;
        }
    default: /* case CPL_TYPE_DOUBLE */
        {
            double * pself = (double*)self;
            pself[i] = value;
            break;
        }
    }
}





/*----------------------------------------------------------------------------*/
/**
  @brief    Dump a single CPL error
  @param    Pointer to one of cpl_msg_info(), cpl_msg_warning(), ...
  @param    self      The number of the current error to be dumped
  @param    first     The number of the first error to be dumped
  @param    last      The number of the last error to be dumped
  @return   void
  @see irplib_errorstate_dump_one

 */
/*----------------------------------------------------------------------------*/
static
void irplib_errorstate_dump_one_level(void (*messenger)(const char *,
                                                        const char *, ...),
                                      unsigned self, unsigned first,
                                      unsigned last)
{

    const cpl_boolean is_reverse = first > last ? CPL_TRUE : CPL_FALSE;
    const unsigned    newest     = is_reverse ? first : last;
    const unsigned    oldest     = is_reverse ? last : first;
    const char      * revmsg     = is_reverse ? " in reverse order" : "";


    /*
    cx_assert( messenger != NULL );
    cx_assert( oldest <= self );
    cx_assert( newest >= self );
    */

    if (newest == 0) {
        messenger(cpl_func, "No error(s) to dump");
        /* cx_assert( oldest == 0); */
    } else {
        /*
          cx_assert( oldest > 0);
          cx_assert( newest >= oldest);
        */
        if (self == first) {
            if (oldest == 1) {
                messenger(cpl_func, "Dumping all %u error(s)%s:", newest,
                          revmsg);
            } else {
                messenger(cpl_func, "Dumping the %u most recent error(s) "
                          "out of a total of %u errors%s:",
                          newest - oldest + 1, newest, revmsg);
            }
            cpl_msg_indent_more();
        }

        messenger(cpl_func, "[%u/%u] '%s' (%u) at %s", self, newest,
                  cpl_error_get_message(), cpl_error_get_code(),
                  cpl_error_get_where());

        if (self == last) cpl_msg_indent_less();
    }
}

cpl_polynomial * irplib_polynomial_fit_1d_create_chiq(
		const cpl_vector    *   x_pos,
        const cpl_vector    *   values,
        int                     degree,
        double              *   rechisq
        )
 {
    return irplib_polynomial_fit_1d_create_common(x_pos, values, degree, NULL, rechisq);
 }
cpl_polynomial * irplib_polynomial_fit_1d_create(
		const cpl_vector    *   x_pos,
        const cpl_vector    *   values,
        int                     degree,
        double              *   mse
        )
{

    return irplib_polynomial_fit_1d_create_common(x_pos, values, degree, mse, NULL);
}
static cpl_polynomial * irplib_polynomial_fit_1d_create_common(
		const cpl_vector    *   x_pos,
        const cpl_vector    *   values,
        int                     degree,
        double              *   mse,
        double				*  rechisq
        )
{
    cpl_polynomial * fit1d = NULL;
    cpl_size loc_degree = (cpl_size)degree ;
    int x_size = 0;
    fit1d = cpl_polynomial_new(1);
    x_size = cpl_vector_get_size(x_pos);    
    if(fit1d != NULL && x_size > 1)
    {
        cpl_matrix     * samppos = NULL;
        cpl_vector     * fitresidual = NULL;
        cpl_ensure(!cpl_error_get_code(), cpl_error_get_code(), NULL);
        samppos = cpl_matrix_wrap(1, x_size,
                                  (double*)cpl_vector_get_data_const(x_pos));
        cpl_ensure(!cpl_error_get_code(), cpl_error_get_code(), NULL);
        fitresidual = cpl_vector_new(x_size);
        cpl_ensure(!cpl_error_get_code(), cpl_error_get_code(), NULL);
        cpl_polynomial_fit(fit1d, samppos, NULL, values, NULL,
                           CPL_FALSE, NULL, &loc_degree);
        cpl_ensure(!cpl_error_get_code(), cpl_error_get_code(), NULL);
        cpl_vector_fill_polynomial_fit_residual(fitresidual, values, NULL,
                                                fit1d, samppos, rechisq);
        cpl_ensure(!cpl_error_get_code(), cpl_error_get_code(), NULL);
        if (mse)
        {
            *mse = cpl_vector_product(fitresidual, fitresidual)
                / cpl_vector_get_size(fitresidual);
        }
        cpl_matrix_unwrap(samppos);
        cpl_vector_delete(fitresidual);
    }
    return fit1d;
}

static void quicksort(int* iindex, double* exptime, int left, int right)
{
	int i = left;
	int j = right;
	int pivot = (i + j) / 2;
	double index_value = exptime[pivot];
	do
	{
		while(exptime[i] < index_value) i++;
		while(exptime[j] > index_value) j--;
		if (i <= j)
		{
			if(i < j)
			{
				int tmp = iindex[i];
				double dtmp = exptime[i];
				iindex[i]=iindex[j];
				iindex[j]=tmp;
				exptime[i] = exptime[j];
				exptime[j] = dtmp;
			}
			i++;
			j--;
		}
	} while (i <= j);

	if (i < right)
	{
		quicksort(iindex, exptime, i, right);
	}
	if (left < j)
	{
		quicksort(iindex, exptime,left, j);
	}
}
cpl_error_code irplib_frameset_sort(const cpl_frameset *  self, int* iindex, double* exptime)
{
	int i = 0;
	const cpl_frame* tmp_frame = 0;
	cpl_error_code error = CPL_ERROR_NONE;
	int sz = cpl_frameset_get_size(self);
	cpl_frameset_iterator* iterator = NULL;

	/* 1. get an array of frames */
	tmp_frame = irplib_frameset_get_first_const(&iterator, self);
	while(tmp_frame)
	{
		exptime[i] = frame_get_exptime(tmp_frame);
		iindex[i] = i;
		tmp_frame = irplib_frameset_get_next_const(iterator);
		i++;
	}
	cpl_frameset_iterator_delete(iterator);
	/* 2.sort */
	quicksort(iindex, exptime, 0, sz - 1);

	return error;
}

static double frame_get_exptime(const cpl_frame * pframe)
{
    double dval = 0;
    cpl_propertylist * plist =
        cpl_propertylist_load_regexp(cpl_frame_get_filename(pframe), 0,
                                     "EXPTIME", CPL_FALSE);
    if(plist) {
        dval = cpl_propertylist_get_double(plist, "EXPTIME");
        if (cpl_error_get_code() != CPL_ERROR_NONE) {
            cpl_msg_error(cpl_func, "error during reading EXPTIME key from "
                          "the frame [%s]", cpl_frame_get_filename(pframe));
        }
    }
    /* Free and return */
    cpl_propertylist_delete(plist);
    return dval;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    allocate uninitialized aligned memory
  @param    alignment alignment of the data, must be a power of two
  @param    size      size of the memory block to be allocated in bytes
  @return   pointer to aligned memory or on failure returns NULL and sets errno
  @see irplib_aligned_free
  @note memory MUST be free'd with irplib_aligned_free and cannot be realloc'd
        memory leaks will not be detected by cpl
        It is recommended to build with posix 2001 to get posix_memalign and add
        AC_CHECK_FUNCS([posix_memalign]) and AC_CHECK_DECLS([posix_memalign]) to
        configure.ac

 */
/*----------------------------------------------------------------------------*/
void * irplib_aligned_malloc(size_t alignment, size_t size)
{
    if (alignment == 0)
        alignment = 1;
    /* Error if align is not a power of two.  */
    if (alignment & (alignment - 1)) {
        errno = EINVAL;
        return NULL;
    }
    /* make size a multiple of alignment (required by C11) */
    if ((size % alignment) != 0) {
        size += alignment - (size % alignment);
    }

#if defined HAVE_DECL_ALIGNED_ALLOC && defined HAVE_ALIGNED_ALLOC
    return aligned_alloc(alignment, size);
#elif defined HAVE_POSIX_MEMALIGN && defined HAVE_DECL_POSIX_MEMALIGN
    {
        void *ptr;
        if (alignment == 1)
            return malloc (size);
        if (alignment == 2 || (sizeof (void *) == 8 && alignment == 4))
            alignment = sizeof (void *);
        if (posix_memalign (&ptr, alignment, size) == 0)
            return ptr;
        else
            return NULL;
    }
#else
    /* copied from gmm_malloc.h in gcc-4.8 */
    {
        void * malloc_ptr;
        void * aligned_ptr;

        if (size == 0)
            return NULL;

        /* Assume malloc'd pointer is aligned at least to sizeof (void*).
           If necessary, add another sizeof (void*) to store the value
           returned by malloc. Effectively this enforces a minimum alignment
           of sizeof double. */
        if (alignment < 2 * sizeof (void *))
            alignment = 2 * sizeof (void *);

        malloc_ptr = malloc (size + alignment);
        if (!malloc_ptr)
            return NULL;

        /* Align  We have at least sizeof (void *) space below malloc'd ptr. */
        aligned_ptr = (void *) (((size_t) malloc_ptr + alignment)
                                & ~((size_t) (alignment) - 1));

        /* Store the original pointer just before p.  */
        *(((void **) aligned_ptr) - 1) = malloc_ptr;

        return aligned_ptr;
    }
#endif
}


/*----------------------------------------------------------------------------*/
/**
  @brief    allocate aligned memory initialized to zero
  @param    alignment alignment of the data, must be a power of two
  @param    nelem     number of elements in buffer
  @param    nbytes    size of single element in bytes
  @return   pointer to aligned memory or on failure returns NULL and sets errno
  @see irplib_aligned_malloc

 */
/*----------------------------------------------------------------------------*/
void * irplib_aligned_calloc(size_t alignment, size_t nelem, size_t nbytes)
{
    void * buffer = irplib_aligned_malloc(alignment, nelem * nbytes);
    if (buffer == NULL)
        return NULL;
    /* cast to aligned pointer helps compilers to emit better (builtin) code */
    memset((size_t *)buffer, 0, nelem * nbytes);
    return buffer;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    free aligned memory
  @param    aligned_ptr    pointer to memory to be free'd
  @see irplib_aligned_malloc
  @note memory MUST be allocated with irplib_aligned_[cm]alloc

 */
/*----------------------------------------------------------------------------*/
void irplib_aligned_free (void * aligned_ptr)
{
#if defined HAVE_DECL_ALIGNED_ALLOC && defined HAVE_ALIGNED_ALLOC
  free(aligned_ptr);
#elif defined HAVE_POSIX_MEMALIGN && defined HAVE_DECL_POSIX_MEMALIGN
  free(aligned_ptr);
#else
  if (aligned_ptr)
      free (*(((void **) aligned_ptr) - 1));
#endif
}


/*----------------------------------------------------------------------------*/
/**
  @brief Return the first frame in a frameset using the iterator API.
  @param[out]  iterator  Location of frameset iterator pointer that will be
        filled with a new iterator instance for subsequent calls to the function
        @c irplib_frameset_get_next_const.
  @param[in]   frameset  The frameset to iterate over.
  @return The first frame in the frameset or NULL on error.
  @note The iterator instance returned in the @a iterator pointer must be
        cleaned up with a call to @c cpl_frameset_iterator_delete. If an error
        occurred and a new iterator instance was not created then @c *iterator
        will be set to NULL and need not be cleaned up.
 */
/*----------------------------------------------------------------------------*/
const cpl_frame *
irplib_frameset_get_first_const(cpl_frameset_iterator **iterator,
                                const cpl_frameset *frameset)
{
    cpl_ensure(iterator != NULL, CPL_ERROR_NULL_INPUT, NULL);
    *iterator = cpl_frameset_iterator_new(frameset);
    return cpl_frameset_iterator_get_const(*iterator);
}

/*----------------------------------------------------------------------------*/
/**
  @brief Return the next frame in a frameset using the iterator API.
  @param[in]  iterator  Iterator instance that was returned in an initial call
                        to @c _irplib_frameset_get_first_const.
  @return The next frame in the frameset or NULL if already at the end of the
          list. NULL is also returned if an error occurred which can be checked
          with a call to @c cpl_error_get_code.
 */
/*----------------------------------------------------------------------------*/
const cpl_frame *
irplib_frameset_get_next_const(cpl_frameset_iterator *iterator)
{
    cpl_errorstate prestate = cpl_errorstate_get();
    cpl_error_code error = cpl_frameset_iterator_advance(iterator, 1);
    if (error == CPL_ERROR_ACCESS_OUT_OF_RANGE) {
        cpl_errorstate_set(prestate);
        return NULL;
    } else if (error != CPL_ERROR_NONE) {
        return NULL;
    }
    return cpl_frameset_iterator_get_const(iterator);
}


/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief  Find the kth smallest value in a cpl_vector
  @param  self The vector to permute and search
  @param  k    The requested value position in the sorted array, zero for 1st

  After a successful call, self is permuted so elements less than the kth have
  lower indices, while elements greater than the kth have higher indices. If
  the call fails, self is not modified.

  Reference:

  Author: Wirth, Niklaus 
  Title: Algorithms + data structures = programs 
  Publisher: Englewood Cliffs: Prentice-Hall, 1976 
  Physical description: 366 p. 
  Series: Prentice-Hall Series in Automatic Computation 

  See also: http://ndevilla.free.fr/median/median/

 */
/*----------------------------------------------------------------------------*/

void irplib_vector_get_kth(cpl_vector * self, cpl_size k)
{

    cpl_size l = 0;
    cpl_size m = cpl_vector_get_size(self) - 1;
    cpl_size i = l;
    cpl_size j = m;
    double*  pself = cpl_vector_get_data(self);

    if (pself == NULL) {
        (void)cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
        return;
    } else if (k < 0) {
        (void)cpl_error_set(cpl_func, CPL_ERROR_ILLEGAL_INPUT);
        return;
    } else if (k > m) {
        (void)cpl_error_set(cpl_func, CPL_ERROR_ACCESS_OUT_OF_RANGE);
        return;
    }

    while (l < m) {
        const double x = pself[k];

        do {
            while (pself[i] < x) i++;
            while (x < pself[j]) j--;
            if (i <= j) {
                IRPLIB_SWAP_DOUBLE(pself[i], pself[j]);
                i++; j--;
            }
        } while (i <= j);

        assert( j < i );

        /* The original implementation has two index comparisons and
           two, three or four index assignments. This has been reduced
           to one or two index comparisons and two index assignments.
        */

        if (k <= j) {
            assert( k < i );
            m = j;
            i = l;
        } else {
            if (k < i) {
                m = j;
            } else {
                j = m;
            }
            l = i;
        }
    }
}
