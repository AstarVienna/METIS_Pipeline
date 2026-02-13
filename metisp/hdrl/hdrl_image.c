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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_elemop.h"
#include "hdrl_image.h"
#include "hdrl_image_defs.h"
#include "hdrl_utils.h"
#include "hdrl_buffer.h"

#include <cpl.h>
#include <string.h>
#include <math.h>

/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_image   Image object
 *
 * hdrl_image is a two dimensional array object containing data and its
 * associated errors. It provides a similar api to cpl_image and performs
 * linear error propagation where it makes sense.
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*-----------------------------------------------------------------------------
                                   Functions
 -----------------------------------------------------------------------------*/

static void _hdrl_image_delete(hdrl_image * himg);
static void _hdrl_image_delete_buffer(hdrl_image * himg);
static void _hdrl_image_sync_mask(hdrl_image * himg);

/* ---------------------------------------------------------------------------*/
/**
 * @brief check data and its errors for consistency
 *
 * will warn if error has a different bpm than image
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code
hdrl_image_check_consistent(const cpl_image * image,
                            const cpl_image * error)
{
    cpl_ensure_code(image, CPL_ERROR_NULL_INPUT);
    if (error) {
        cpl_size inx = cpl_image_get_size_x(image);
        cpl_size iny = cpl_image_get_size_y(image);
        cpl_size enx = cpl_image_get_size_x(error);
        cpl_size eny = cpl_image_get_size_y(error);
        const cpl_mask * ibpm = cpl_image_get_bpm_const(image);
        const cpl_mask * ebpm = cpl_image_get_bpm_const(error);
        cpl_ensure_code(inx == enx, CPL_ERROR_INCOMPATIBLE_INPUT);
        cpl_ensure_code(iny == eny, CPL_ERROR_INCOMPATIBLE_INPUT);

        if (ibpm && ebpm) {
            const cpl_binary * dibpm = cpl_mask_get_data_const(ibpm);
            const cpl_binary * debpm = cpl_mask_get_data_const(ebpm);
            if (memcmp(dibpm, debpm, inx * iny) != 0) {
                cpl_msg_warning(cpl_func, "Image and error bad pixel mask "
                                "not equal, ignoring mask of error image");
            }
        }
        else if (ibpm == NULL && ebpm) {
            cpl_msg_warning(cpl_func, "Image and error bad pixel mask "
                            "not equal, ignoring mask of error image");
        }
    }

    return CPL_ERROR_NONE;
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief get data as cpl image
 * @param himg  hdrl image
 * @return cpl_image
 */
/* ---------------------------------------------------------------------------*/
cpl_image * hdrl_image_get_image(hdrl_image * himg)
{
    cpl_ensure(himg, CPL_ERROR_NULL_INPUT, NULL);
    return himg->image;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get data as cpl image
 * @param himg  hdrl image
 * @return const cpl_image
 */
/* ---------------------------------------------------------------------------*/
const cpl_image * hdrl_image_get_image_const(const hdrl_image * himg)
{
    cpl_ensure(himg, CPL_ERROR_NULL_INPUT, NULL);
    return himg->image;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get error as cpl image
 * @param himg  hdrl image
 * @return cpl_image
 */
/* ---------------------------------------------------------------------------*/
cpl_image * hdrl_image_get_error(hdrl_image * himg)
{
    cpl_ensure(himg, CPL_ERROR_NULL_INPUT, NULL);
    return himg->error;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get error as cpl image
 * @param himg  hdrl image
 * @return cpl_image
 */
/* ---------------------------------------------------------------------------*/
const cpl_image * hdrl_image_get_error_const(const hdrl_image * himg)
{
    cpl_ensure(himg, CPL_ERROR_NULL_INPUT, NULL);
    return himg->error;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get cpl bad pixel mask from image
 * @param himg  hdrl image
 * @return cpl_mask, created when it does not exist
 */
/* ---------------------------------------------------------------------------*/
cpl_mask * hdrl_image_get_mask(hdrl_image * himg)
{
    cpl_ensure(himg, CPL_ERROR_NULL_INPUT, NULL);
    if (cpl_image_get_bpm_const(hdrl_image_get_image(himg)) == NULL) {
        /* call adds mask, also add a mask to the errors */
        cpl_image_get_bpm(hdrl_image_get_error(himg));
    }

    return cpl_image_get_bpm(hdrl_image_get_image(himg));
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get cpl bad pixel mask from image
 * @param himg  hdrl image
 * @return cpl_mask or NULL if no mask exists
 */
/* ---------------------------------------------------------------------------*/
const cpl_mask * hdrl_image_get_mask_const(const hdrl_image * himg)
{
    cpl_ensure(himg, CPL_ERROR_NULL_INPUT, NULL);
    return cpl_image_get_bpm_const(hdrl_image_get_image_const(himg));
}



/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief create hdrl image by wrapping two cpl images
 * @note beside typecheck no consistency checks on the two images
 * @see hdrl_image_check_consistent
 */
/* ---------------------------------------------------------------------------*/
hdrl_image *
hdrl_image_wrap(cpl_image * img, cpl_image * err, hdrl_free * destructor,
                cpl_boolean sync_mask)
{
    hdrl_image * himg;
    cpl_ensure(img, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(err, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(cpl_image_get_type(img) == HDRL_TYPE_DATA,
               CPL_ERROR_INCOMPATIBLE_INPUT, NULL);
    cpl_ensure(cpl_image_get_type(err) == HDRL_TYPE_ERROR,
               CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    himg = cpl_malloc(sizeof(*himg));
    himg->image = img;
    himg->error = err;

    if (destructor) {
        himg->fp_free = destructor;
    }
    else {
        himg->fp_free = (hdrl_free*)_hdrl_image_delete;
    }

    if (sync_mask) {
        _hdrl_image_sync_mask(himg);
    }

    return himg;
}


/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief unwrap hdrl image, will not delete wrapped cpl images
 */
/* ---------------------------------------------------------------------------*/
void hdrl_image_unwrap(hdrl_image * himg)
{
    cpl_free(himg);
}


/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief create a new hdrl_image from to existing images by copying them
 *
 * @param image  data to copy
 * @param error  errors to copy
 * @param check_consistent check consistency
 *
 * @return hdrl_image
 * @see hdrl_image_create
 */
/* ---------------------------------------------------------------------------*/
static hdrl_image * _hdrl_image_create(const cpl_image * image,
                                       const cpl_image * error,
                                       cpl_boolean check_consistent)
{
    cpl_image * himage = NULL, * herror = NULL;

    if (check_consistent && hdrl_image_check_consistent(image, error)) {
        return NULL;
    }

    himage = cpl_image_cast(image, HDRL_TYPE_DATA);
    if (error) {
        herror = cpl_image_cast(error, HDRL_TYPE_ERROR);
    }
    else {
        /* set error to zero */
        herror = cpl_image_new(cpl_image_get_size_x(image),
                               cpl_image_get_size_y(image),
                               HDRL_TYPE_ERROR);
    }

    /* sync image and error bpm ignoring what is in error before */
    if (cpl_image_get_bpm_const(image)) {
        cpl_image_reject_from_mask(herror,
                                   cpl_image_get_bpm_const(image));
    }
    else {
        cpl_image_accept_all(herror);
    }

    return hdrl_image_wrap(himage, herror, NULL, CPL_FALSE);
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief create a new hdrl_image from to existing images by copying them
 *
 * @param image  data to copy
 * @param error  errors to copy
 *
 * @return hdrl_image
 *
 * @note The bad pixel mask of the error-image (\a error) is completely
 * ignored. The bad pixel mask associated with the passed image (\a image)
 * becomes the only relevant bad pixel mask.
 */
/* ---------------------------------------------------------------------------*/
hdrl_image * hdrl_image_create(const cpl_image * image,
                               const cpl_image * error)
{
    return _hdrl_image_create(image, error, CPL_TRUE);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief create new zero filled hdrl image
 *
 * @param nx  size in x
 * @param ny  size in y
 *
 * @return hdrl_image or NULL on error
 */
/* ---------------------------------------------------------------------------*/
hdrl_image * hdrl_image_new(cpl_size nx, cpl_size ny)
{
    cpl_image * himage = cpl_image_new(nx, ny, HDRL_TYPE_DATA);
    cpl_image * herror = cpl_image_new(nx, ny, HDRL_TYPE_ERROR);

    if (cpl_error_get_code()) {
        cpl_image_delete(himage);
        cpl_image_delete(herror);
        return NULL;
    }

    return hdrl_image_wrap(himage, herror, NULL, CPL_FALSE);
}

hdrl_image *
hdrl_image_new_from_buffer(cpl_size nx, cpl_size ny, hdrl_buffer * buf)
{
    char * p = hdrl_buffer_allocate(buf, nx * ny * (sizeof(hdrl_data_t) +
                                                    sizeof(hdrl_error_t)));
    cpl_image * himage = cpl_image_wrap(nx, ny, HDRL_TYPE_DATA, p);
    cpl_image * herror = cpl_image_wrap(nx, ny, HDRL_TYPE_ERROR,
                                        p + nx * ny * sizeof(hdrl_data_t));

    if (cpl_error_get_code()) {
        cpl_image_delete(himage);
        cpl_image_delete(herror);
        return NULL;
    }

    return hdrl_image_wrap(himage, herror,
                           (hdrl_free*)&_hdrl_image_delete_buffer,
                           CPL_FALSE);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief delete hdrl_image
 */
/* ---------------------------------------------------------------------------*/
static void _hdrl_image_delete(hdrl_image * himg)
{
    if (himg) {
        cpl_image_delete(himg->image);
        cpl_image_delete(himg->error);
        cpl_free(himg);
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief delete buffered hdrl_image
 */
/* ---------------------------------------------------------------------------*/
static void _hdrl_image_delete_buffer(hdrl_image * himg)
{
    if (himg) {
        cpl_image_unwrap(himg->image);
        cpl_image_unwrap(himg->error);
        cpl_free(himg);
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief delete hdrl_image
 * @note may be used on views in which case the original memory is kept
 */
/* ---------------------------------------------------------------------------*/
void hdrl_image_delete(hdrl_image * himg)
{
    if (himg) {
        himg->fp_free(himg);
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief copy hdrl_image
 */
/* ---------------------------------------------------------------------------*/
hdrl_image * hdrl_image_duplicate(const hdrl_image * himg)
{
    return _hdrl_image_create(hdrl_image_get_image_const(himg),
                              hdrl_image_get_error_const(himg), CPL_FALSE);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief set bpm of hdrl_image
 *
 * @param self  image on which to set bpm
 * @param map   bpm to set
 * @see cpl_image_reject_from_mask
 *
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_image_reject_from_mask(hdrl_image * self,
                                           const cpl_mask * map)
{
    if (hdrl_image_get_mask_const(self) != map) {
        cpl_image_reject_from_mask(hdrl_image_get_image(self), map);
    }
    return cpl_image_reject_from_mask(hdrl_image_get_error(self), map);
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief mark pixel as bad
 *
 * @param self          image
 * @param xpos          x coordinate (FITS CONVENTION)
 * @param ypos          y coordinate (FITS CONVENTION)
 * @see cpl_image_reject
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_image_reject(hdrl_image * self,
                                 cpl_size xpos, cpl_size ypos)
{
    cpl_image_reject(hdrl_image_get_image(self), xpos, ypos);
    return cpl_image_reject(hdrl_image_get_error(self), xpos, ypos);
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief return if pixel is marked bad
 *
 * @param self          image
 * @param xpos          x coordinate (FITS CONVENTION)
 * @param ypos          y coordinate (FITS CONVENTION)
 * @see cpl_image_is_rejected
 */
/* ---------------------------------------------------------------------------*/
int hdrl_image_is_rejected(hdrl_image * self, cpl_size xpos, cpl_size ypos)
{
    cpl_ensure(self, CPL_ERROR_NULL_INPUT, -1);
    return cpl_image_is_rejected(hdrl_image_get_image(self), xpos, ypos);
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief return number of rejected pixels
 *
 * @param self          image
 * @return number of rejected pixels
 * @see cpl_image_count_rejected
 */
/* ---------------------------------------------------------------------------*/
cpl_size hdrl_image_count_rejected(const hdrl_image * self)
{
    cpl_ensure(self, CPL_ERROR_NULL_INPUT, -1);
    return cpl_image_count_rejected(hdrl_image_get_image_const(self));
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief    Reject pixels with the specified special value(s)
 *
 * @param self   Input image to modify
 * @param mode   Bit field specifying which special value(s) to reject
 * @see cpl_image_reject_value
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_image_reject_value(hdrl_image * self, cpl_value mode)
{
    return cpl_image_reject_value(hdrl_image_get_image(self), mode);
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief mark pixel as good
 *
 * @param self          image
 * @param xpos          x coordinate (FITS CONVENTION)
 * @param ypos          y coordinate (FITS CONVENTION)
 * @see cpl_image_accept
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_image_accept(hdrl_image * self,
                                 cpl_size xpos, cpl_size ypos)
{
    cpl_image_accept(hdrl_image_get_image(self), xpos, ypos);
    return cpl_image_accept(hdrl_image_get_error(self), xpos, ypos);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Accept all pixels in an image
 *
 * @param self input image
 * @see cpl_image_accept_all
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_image_accept_all(hdrl_image * self)
{
    cpl_image_accept_all(hdrl_image_get_image(self));
    cpl_image_accept_all(hdrl_image_get_error(self));
    return cpl_error_get_code();
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief return size of X dimension of image
 *
 * @param self  image
 * @return size of X dimension of image
 * @see cpl_image_get_size_x
 */
/* ---------------------------------------------------------------------------*/
cpl_size hdrl_image_get_size_x(const hdrl_image * self)
{
    return cpl_image_get_size_x(hdrl_image_get_image_const(self));
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief return size of Y dimension of image
 *
 * @param self  image
 * @return size of Y dimension of image
 * @see cpl_image_get_size_y
 */
/* ---------------------------------------------------------------------------*/
cpl_size hdrl_image_get_size_y(const hdrl_image * self)
{
    return cpl_image_get_size_y(hdrl_image_get_image_const(self));
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief get pixel values of hdrl_image
 *
 * @param self          image
 * @param xpos          x coordinate (FITS CONVENTION)
 * @param ypos          y coordinate (FITS CONVENTION)
 * @param pis_rejected  int pointer to store if pixel is bad, may be NULL
 *
 * @return data value of the pixel
 * @see cpl_image_get
 */
/* ---------------------------------------------------------------------------*/
hdrl_value hdrl_image_get_pixel(const hdrl_image * self,
                                cpl_size xpos, cpl_size ypos,
                                int * pis_rejected)
{
    int d;
    hdrl_value v;
    v.data = cpl_image_get(hdrl_image_get_image_const(self), xpos, ypos, &d);

    /* NULL allowed, different than CPL */
    if (pis_rejected) {
        *pis_rejected = d;
    }
    if (d) {
        v.data = NAN;
        v.error = NAN;
    }
    else {
        v.error = cpl_image_get(hdrl_image_get_error_const(self),
                                xpos, ypos, &d);
    }

    return v;
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief set pixel values of hdrl_image
 *
 * @param self          image
 * @param xpos          x coordinate (FITS CONVENTION)
 * @param ypos          y coordinate (FITS CONVENTION)
 * @param value         data value to set
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_image_set_pixel(hdrl_image * self,
                                    cpl_size xpos, cpl_size ypos,
                                    hdrl_value value)
{
    /* nan may be used as a bad pixel value */
    cpl_ensure_code(value.error >= 0 || isnan(value.error), CPL_ERROR_ILLEGAL_INPUT);

    if (cpl_image_set(hdrl_image_get_image(self), xpos, ypos, value.data)) {
        return cpl_error_get_code();
    }

    return cpl_image_set(hdrl_image_get_error(self), xpos, ypos, value.error);
}


/* ---------------------------------------------------------------------------*/
/**
 @brief extract copy of window from image
 @param self  image to extract from
 @param llx   lower left x coordinate of window (FITS convention)
 @param lly   lower left y coordinate of window (FITS convention)
 @param urx   upper right x coordinate of window (FITS convention)
 @param ury   upper right y coordinate of window (FITS convention)
 
 @return newly allocated hdrl_image containing the window
 @see cpl_image_extract

 If the coordinates are < 1 the dimension of the image is added to them.
 So llx = 0 wraps to hdrl_image_get_size_x(self).
 */
/* ---------------------------------------------------------------------------*/
hdrl_image * hdrl_image_extract(
        const hdrl_image        *   self,
        cpl_size                    llx,
        cpl_size                    lly,
        cpl_size                    urx,
        cpl_size                    ury)
{
    const cpl_size nx = hdrl_image_get_size_x(self);
    const cpl_size ny = hdrl_image_get_size_y(self);
    if (llx < 1) llx += nx;
    if (lly < 1) lly += ny;
    if (urx < 1) urx += nx;
    if (ury < 1) ury += ny;

    cpl_image * img = cpl_image_extract(hdrl_image_get_image_const(self),
                                        llx, lly, urx, ury);
    cpl_image * err = cpl_image_extract(hdrl_image_get_error_const(self),
                                        llx, lly, urx, ury);

    if (cpl_error_get_code()) {
        cpl_image_delete(img);
        cpl_image_delete(err);
        return NULL;
    }

    return hdrl_image_wrap(img, err, NULL, CPL_FALSE);
}


/* ---------------------------------------------------------------------------*/
/**
  @brief    Rotate an image by a multiple of 90 degrees clockwise.
  @param    self    The image to rotate in place.
  @param    rot     The multiple: -1 is a rotation of 90 deg counterclockwise.
  @return   CPL_ERROR_NONE on success, otherwise the relevant cpl_error_code
  @see cpl_image_turn
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_image_turn(hdrl_image * self, int rot)
{
    cpl_ensure_code(self, CPL_ERROR_NULL_INPUT);
    cpl_image_turn(hdrl_image_get_image(self), rot);
    cpl_image_turn(hdrl_image_get_error(self), rot);
    return cpl_error_get_code();
}


/* ---------------------------------------------------------------------------*/
/**
  @brief    Copy one image into another
  @param    dst     the image in which im2 is inserted
  @param    src     the inserted image
  @param    xpos    the x pixel position in im1 where the lower left pixel of
                    im2 should go (from 1 to the x size of im1)
  @param    ypos    the y pixel position in im1 where the lower left pixel of
                    im2 should go (from 1 to the y size of im1)
  @return   CPL_ERROR_NONE or the relevant cpl_error_code on error.
  @see cpl_image_copy
  @note The two pixel buffers may not overlap
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_image_copy(hdrl_image * dst, const hdrl_image * src,
                               cpl_size xpos, cpl_size ypos)
{
    cpl_ensure_code(dst, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(src, CPL_ERROR_NULL_INPUT);
    cpl_image_copy(hdrl_image_get_image(dst), hdrl_image_get_image_const(src),
                   xpos, ypos);
    cpl_image_copy(hdrl_image_get_error(dst), hdrl_image_get_error_const(src),
                   xpos, ypos);

    return cpl_error_get_code();
}


/* ---------------------------------------------------------------------------*/
/**
  @brief    Copy cpl images into an hdrl image
  @param    self    the image in which the cpl images are inserted
  @param    image   the inserted image
  @param    error   the inserted error, may be NULL
  @param    xpos    the x pixel position in im1 where the lower left pixel of
                    im2 should go (from 1 to the x size of im1)
  @param    ypos    the y pixel position in im1 where the lower left pixel of
                    im2 should go (from 1 to the y size of im1)
  @return   CPL_ERROR_NONE or the relevant cpl_error_code on error.
  @see hdrl_image_copy
  @note The pixel buffers may not overlap
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_image_insert(hdrl_image * self,
                                 const cpl_image * image,
                                 const cpl_image * error,
                                 cpl_size xpos, cpl_size ypos)
{
    cpl_ensure_code(self, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(image, CPL_ERROR_NULL_INPUT);
    cpl_image_copy(hdrl_image_get_image(self), image, xpos, ypos);
    if (error) {
        cpl_image_copy(hdrl_image_get_error(self), error, xpos, ypos);
    }
    /* sync error mask */
    if (cpl_image_get_bpm_const(image)) {
        const cpl_mask * msrc = cpl_image_get_bpm_const(image);
        cpl_mask * mdst = cpl_image_get_bpm(hdrl_image_get_error(self));
        cpl_mask_copy(mdst, msrc, xpos, ypos);
    }

    return cpl_error_get_code();
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief synchronize mask
 */
/* ---------------------------------------------------------------------------*/

static void _hdrl_image_sync_mask(hdrl_image * himg)
{
    const cpl_mask * m = hdrl_image_get_mask_const(himg);
    if (m) {
        hdrl_image_reject_from_mask(himg, m);
    }
    else {
        cpl_image_accept_all(hdrl_image_get_error(himg));
    }
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    Dump structural information of a HDRL image
  @param    himg    Image to dump
  @param    stream  Output stream, accepts @c stdout or @c stderr
  @return   CPL_ERROR_NONE or the relevant cpl-error-code on error

  Possible cpl-error-code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
  - CPL_ERROR_FILE_IO if a write operation fails
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_image_dump_structure(
        const hdrl_image    *   himg,
        FILE                *   stream)
{
    return cpl_image_dump_structure(hdrl_image_get_image_const(himg), stream);
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_image
  @brief    Dump pixel values in a HDRL image
  @param    himg    Image to dump
  @param    llx     Lower left x position (FITS convention, 1 for leftmost)
  @param    lly     Lower left y position (FITS convention, 1 for lowest)
  @param    urx     Specifies the window position
  @param    ury     Specifies the window position
  @param    stream  Output stream, accepts @c stdout or @c stderr
  @return   CPL_ERROR_NONE or the relevant cpl-error-code on error

  Possible cpl-error-code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
  - CPL_ERROR_FILE_IO if a write operation fails
  - CPL_ERROR_ACCESS_OUT_OF_RANGE if the defined window is not in the image
  - CPL_ERROR_ILLEGAL_INPUT if the window definition is wrong (e.g llx > urx)
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_image_dump_window(
        const hdrl_image    *   himg,
        cpl_size                llx,
        cpl_size                lly,
        cpl_size                urx,
        cpl_size                ury,
        FILE *                  stream)
{
    return cpl_image_dump_window(hdrl_image_get_image_const(himg),
                    llx, lly, urx, ury, stream);

}

/**@}*/
