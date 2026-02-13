/* $Id: hdrl_imagelist_view.c,v 1.8 2013-10-23 09:13:56 jtaylor Exp $
 *
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

/*
 * $Author: jtaylor $
 * $Date: 2013-10-23 09:13:56 $
 * $Revision: 1.8 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_imagelist.h"
#include "hdrl_imagelist_defs.h"
#include "hdrl_imagelist_view.h"
#include "hdrl_image_defs.h"
#include "hdrl_image.h"

#include <cpl.h>
#include <assert.h>
#include <string.h>

/*-----------------------------------------------------------------------------
                                   Define
 -----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                            Static Prototypes
 -----------------------------------------------------------------------------*/

/* destructor unwrapping data and error */
static void hdrl_image_view_delete(hdrl_image * view)
{
    if (view) {
        hdrl_image * himg = (hdrl_image*)view;
        cpl_mask_unwrap(cpl_image_unset_bpm(hdrl_image_get_image(himg)));
        cpl_mask_unwrap(cpl_image_unset_bpm(hdrl_image_get_error(himg)));
        cpl_image_unwrap(hdrl_image_get_image(himg));
        cpl_image_unwrap(hdrl_image_get_error(himg));
        hdrl_image_unwrap(himg);
    }
}

/* destructor unwrapping data and deleting error */
static void hdrl_image_imgview_delete(hdrl_image * view)
{
    if (view) {
        hdrl_image * himg = (hdrl_image*)view;
        cpl_mask_unwrap(cpl_image_unset_bpm(hdrl_image_get_image(himg)));
        cpl_image_unwrap(hdrl_image_get_image(himg));
        cpl_image_delete(hdrl_image_get_error(himg));
        hdrl_image_unwrap(himg);
    }
}


/**@{*/

/*-----------------------------------------------------------------------------
                            Function codes
 -----------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief create a row view of an image
 *
 * @param himg input image
 * @param ly   lower row (FITS)
 * @param uy   upper row (FITS)
 *
 */
/* ---------------------------------------------------------------------------*/
static hdrl_image *
hdrl_image_row_view_create(hdrl_image * himg,
                           cpl_size ly,
                           cpl_size uy)
{
    cpl_image * oimg = hdrl_image_get_image(himg);
    cpl_image * oerr = hdrl_image_get_error(himg);
    const size_t dsz = cpl_type_get_sizeof(cpl_image_get_type(oimg));
    const size_t esz = cpl_type_get_sizeof(cpl_image_get_type(oerr));
    const cpl_size nx = hdrl_image_get_size_x(himg);
    char * d = cpl_image_get_data(oimg);
    char * e = cpl_image_get_data(oerr);
    size_t offset = (ly - 1) * nx;
    cpl_size nny = uy - ly + 1;
    cpl_image * img = cpl_image_wrap(nx, nny, cpl_image_get_type(oimg),
                                     d + offset * dsz);
    cpl_image * err = cpl_image_wrap(nx, nny, cpl_image_get_type(oerr),
                                     e + offset * esz);

    /* we must create a mask in the original in order for views to work */
    cpl_mask * omask = hdrl_image_get_mask(himg);
    cpl_mask * mask = cpl_mask_wrap(nx, nny, cpl_mask_get_data(omask) + offset);
    cpl_mask_delete(hcpl_image_set_bpm(img, mask));
    omask = cpl_image_get_bpm(himg->error);
    mask = cpl_mask_wrap(nx, nny, cpl_mask_get_data(omask) + offset);
    cpl_mask_delete(hcpl_image_set_bpm(err, mask));

    return hdrl_image_wrap(img, err, (hdrl_free*)&hdrl_image_view_delete,
                           CPL_FALSE);
}

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief create a constant row view of an image
 *
 * @param himg input image
 * @param ly   lower row (FITS)
 * @param uy   upper row (FITS)
 * @param uy   upper row (FITS)
 * @param destructor destructor function pointer
 *
 * compared to the non-const row version this does not force a bad pixel map
 * onto the image which can improve performance a lot when the data is only
 * read.
 * As bad pixel information cannot be propagated in this case the view must not
 * be modified.
 */
/* ---------------------------------------------------------------------------*/
static const hdrl_image *
hdrl_image_const_row_view_create(const hdrl_image * himg,
                                 cpl_size ly,
                                 cpl_size uy,
                                 hdrl_free * destructor)
{
    const cpl_image * oimg = hdrl_image_get_image_const(himg);
    const cpl_image * oerr = hdrl_image_get_error_const(himg);
    const size_t dsz = cpl_type_get_sizeof(cpl_image_get_type(oimg));
    const size_t esz = cpl_type_get_sizeof(cpl_image_get_type(oerr));
    const cpl_size nx = hdrl_image_get_size_x(himg);
    const char * d = cpl_image_get_data_const(oimg);
    const char * e = cpl_image_get_data_const(oerr);
    size_t offset = (ly - 1) * nx;
    cpl_size nny = uy - ly + 1;

    CPL_DIAG_PRAGMA_PUSH_IGN(-Wcast-qual);

    cpl_image * img = cpl_image_wrap(nx, nny, cpl_image_get_type(oimg),
                                     (char*)d + offset * dsz);
    cpl_image * err = cpl_image_wrap(nx, nny, cpl_image_get_type(oerr),
                                     (char *)e + offset * esz);

    /* we must create a mask in the original in order for views to work */
    const cpl_mask * omask = hdrl_image_get_mask_const(himg);
    if (omask) {
        cpl_mask * mask = cpl_mask_wrap(nx, nny,
                        (cpl_binary*)cpl_mask_get_data_const(omask) + offset);
        cpl_mask_delete(hcpl_image_set_bpm(img, mask));
    }
    else {
        if (cpl_image_get_bpm_const(himg->error) != NULL) {
            cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                                  "Inconsistent HDRL image, data image has no"
                                  " BPM but error image does");
            cpl_image_unwrap(img);
            cpl_image_unwrap(err);
            return NULL;
        }
    }
    omask = cpl_image_get_bpm_const(himg->error);
    if (omask) {
        cpl_mask * mask = cpl_mask_wrap(nx, nny,
                        (cpl_binary*)cpl_mask_get_data_const(omask) + offset);
        cpl_mask_delete(hcpl_image_set_bpm(err, mask));
    }

    CPL_DIAG_PRAGMA_POP;

    return hdrl_image_wrap(img, err, destructor, CPL_FALSE);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief create view of subset of image rows in an imagelist
 *
 * @param hl source imagelist to create view of
 * @param ly lower y row to view (FITS convention)
 * @param uy upper y row to view (FITS convention)
 *
 * @return imagelist view
 *
 * The imagelist will contain the same number of images but they will have
 * uy - ly + 1 rows. Modifiying the view will modify the selected rows of the
 * original imagelist.
 * The view can be deleted with hdrl_imagelist_delete.
 * Deleting the original imagelist will invalidate all views, they must not be
 * used anymore besides being deleted.
 */
/* ---------------------------------------------------------------------------*/
hdrl_imagelist * hdrl_imagelist_row_view(
    hdrl_imagelist * hl,
    cpl_size ly,
    cpl_size uy)
{
    cpl_ensure(hl, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(uy >= ly, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(ly > 0, CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);
    size_t n = hdrl_imagelist_get_size(hl);
    cpl_ensure(n > 0, CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);
    cpl_ensure(uy <= hdrl_imagelist_get_size_y(hl),
               CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);

    hdrl_imagelist * viewlist = hdrl_imagelist_new();
    for (size_t i = 0; i < n; i++) {
        hdrl_image * img = hdrl_imagelist_get(hl, i);
        hdrl_image * view = hdrl_image_row_view_create(img, ly, uy);
        if (view == NULL) {
            hdrl_imagelist_delete(viewlist);
            return NULL;
        }
        hdrl_imagelist_set(viewlist, view, i);
    }

    return viewlist;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief create const view of subset of image rows in an imagelist
 *
 * @param hl source imagelist to create view of
 * @param ly lower y row to view (FITS convention)
 * @param uy upper y row to view (FITS convention)
 *
 * @return constant imagelist view
 *
 * The imagelist will contain the same number of images but they will have
 * uy - ly + 1 rows. Modifiying the view will modify the selected rows of the
 * original imagelist.
 * The view can be deleted with hdrl_imagelist_delete.
 * Deleting the original imagelist will invalidate all views, they must not be
 * used anymore besides being deleted.
 * The view must not be modified as bad pixel information cannot be propagated
 * to the original image if it did not have a bad pixel map to begin with.
 */
/* ---------------------------------------------------------------------------*/
const hdrl_imagelist * hdrl_imagelist_const_row_view(
    const hdrl_imagelist * hl,
    cpl_size ly,
    cpl_size uy)
{
    cpl_ensure(hl, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(uy >= ly, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(ly > 0, CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);
    size_t n = hdrl_imagelist_get_size(hl);
    cpl_ensure(n > 0, CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);
    cpl_ensure(uy <= hdrl_imagelist_get_size_y(hl),
               CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);

    hdrl_imagelist * viewlist = hdrl_imagelist_new();
    for (size_t i = 0; i < n; i++) {
        hdrl_image * img = hdrl_imagelist_get(hl, i);
        const hdrl_image * view = hdrl_image_const_row_view_create(img, ly, uy,
                                       (hdrl_free*)&hdrl_image_view_delete);
        if (view == NULL) {
            hdrl_imagelist_delete(viewlist);
            return NULL;
        }
        CPL_DIAG_PRAGMA_PUSH_IGN(-Wcast-qual);
        hdrl_imagelist_set(viewlist, (hdrl_image*)view, i);
        CPL_DIAG_PRAGMA_POP;
    }

    return viewlist;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief create view of subset of images in an imagelist
 *
 * @param hl source imagelist to create view of
 * @param lz lower image index to view, starting from 0
 * @param uz upper bound exclusive
 *
 * @return imagelist view
 *
 * The imagelist will contain uz - lz images of the same size as the original
 * imagelist. Modifiying the view is not allowed
 * The view must be deleted with hdrl_imagelist_delete.
 * Deleting the original imagelist will invalidate all views, they must not be
 * used anymore besides being deleted.
 */
/* ---------------------------------------------------------------------------*/
hdrl_imagelist * hdrl_imagelist_image_view(
    hdrl_imagelist * hl,
    cpl_size lz,
    cpl_size uz)
{
    cpl_ensure(hl, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(uz > lz, CPL_ERROR_ILLEGAL_INPUT, NULL);
    size_t n = hdrl_imagelist_get_size(hl);
    cpl_ensure(lz >= 0, CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);
    cpl_ensure(uz <= (cpl_size)n, CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);

    hdrl_imagelist * viewlist = hdrl_imagelist_new();
    for (size_t i = lz; i < (size_t)uz; i++) {
        hdrl_image * img = hdrl_imagelist_get(hl, i);
        size_t ny = hdrl_image_get_size_y(img);
        hdrl_image * view = hdrl_image_row_view_create(img, 1, ny);
        if (view == NULL) {
            hdrl_imagelist_delete(viewlist);
            return NULL;
        }
        hdrl_imagelist_set(viewlist, view, i - lz);
    }

    return viewlist;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief create const view of subset of image rows in cpl imagelists
 *
 * @param imglist source imagelist to create view of
 * @param errlist source error imagelist to create view of, may be NULL
 * @param ly lower y row to view (FITS convention)
 * @param uy upper y row to view (FITS convention)
 *
 * @return constant imagelist view
 *
 * The imagelist will contain the same number of images but they will have
 * uy - ly + 1 rows. Modifiying the view is not allowed
 * The view can be deleted with hdrl_imagelist_delete.
 * Deleting the original imagelist will invalidate all views, they must not be
 * used anymore besides being deleted.
 * The images and errors in the lists must contain equal bad pixel maps.
 */
/* ---------------------------------------------------------------------------*/
const hdrl_imagelist * hdrl_imagelist_const_cpl_row_view(
    const cpl_imagelist * imglist,
    const cpl_imagelist * errlist,
    cpl_size ly,
    cpl_size uy)
{
    cpl_ensure(imglist, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(uy >= ly, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(ly > 0, CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);
    size_t n = cpl_imagelist_get_size(imglist);
    cpl_ensure(n > 0, CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);
    cpl_ensure(uy <= cpl_image_get_size_y(cpl_imagelist_get_const(imglist, 0)),
               CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);
    cpl_ensure(errlist == NULL || n <= (size_t)cpl_imagelist_get_size(errlist),
               CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    hdrl_imagelist * viewlist = hdrl_imagelist_new();
    if (errlist) {
        const cpl_image * img = cpl_imagelist_get_const(imglist, 0);
        const cpl_image * err = cpl_imagelist_get_const(errlist, 0);

        if (cpl_image_get_type(img) != HDRL_TYPE_DATA ||
            cpl_image_get_type(err) != HDRL_TYPE_ERROR) {
            hdrl_imagelist_delete(viewlist);
            cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT,
                                  "Can only view image pairs with type "
                                  "HDRL_TYPE_DATA and HDRL_TYPE_ERROR");
            return NULL;
        }
        if (cpl_image_get_size_x(img) != cpl_image_get_size_x(err) ||
            cpl_image_get_size_y(img) != cpl_image_get_size_y(err)) {
            hdrl_imagelist_delete(viewlist);
            cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT,
                                  "Image and Error not consistent");
            return NULL;
        }
        for (size_t i = 0; i < n; i++) {
            img = cpl_imagelist_get_const(imglist, i);
            err = cpl_imagelist_get_const(errlist, i);
            if ((cpl_image_get_bpm_const(img) && !cpl_image_get_bpm_const(err)) ||
                (!cpl_image_get_bpm_const(img) && cpl_image_get_bpm_const(err))) {
                    hdrl_imagelist_delete(viewlist);
                    cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT,
                                          "Image and error bad pixel mask "
                                          "not equal");
                    return NULL;
            }
            if (cpl_image_get_bpm_const(img) && cpl_image_get_bpm_const(err)) {
                const cpl_binary * dibpm =
                    cpl_mask_get_data_const(cpl_image_get_bpm_const(img));
                const cpl_binary * debpm =
                    cpl_mask_get_data_const(cpl_image_get_bpm_const(err));
                if (memcmp(dibpm, debpm, hdrl_get_image_npix(img)) != 0) {
                    hdrl_imagelist_delete(viewlist);
                    cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT,
                                          "Image and error bad pixel mask "
                                          "not equal");
                    return NULL;
                }
            }
            /* TODO could wrap directly */
            CPL_DIAG_PRAGMA_PUSH_IGN(-Wcast-qual);
            hdrl_image * himg = hdrl_image_wrap((cpl_image*)img, (cpl_image*)err,
                                            (hdrl_free*)&hdrl_image_view_delete,
                                            CPL_FALSE);
            const hdrl_image * view =
                hdrl_image_const_row_view_create(himg, ly, uy,
                                         (hdrl_free*)&hdrl_image_view_delete);
            if (view == NULL) {
                hdrl_imagelist_delete(viewlist);
                return NULL;
            }
            hdrl_image_unwrap(himg);
            hdrl_imagelist_set(viewlist, (hdrl_image*)view, i);
            CPL_DIAG_PRAGMA_POP;
        }
    }
    else {
        const cpl_image * img = cpl_imagelist_get_const(imglist, 0);
        if (cpl_image_get_type(img) != HDRL_TYPE_DATA) {
            hdrl_imagelist_delete(viewlist);
            cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT,
                                  "Can only view images with type "
                                  "HDRL_TYPE_DATA");
            return NULL;
        }
         for (size_t i = 0; i < n; i++) {
            img = cpl_imagelist_get_const(imglist, i);
            /* can't share a single image as it needs the same bpm as img
             * TODO the bpm of img could probably be shared */
            cpl_image * err = cpl_image_new(cpl_image_get_size_x(img),
                                            cpl_image_get_size_y(img),
                                            HDRL_TYPE_ERROR);
            if (cpl_image_get_bpm_const(img)) {
                cpl_image_reject_from_mask(err, cpl_image_get_bpm_const(img));
            }
            /* TODO could wrap directly */
            CPL_DIAG_PRAGMA_PUSH_IGN(-Wcast-qual);
            hdrl_image * himg = hdrl_image_wrap((cpl_image*)img, err,
                                            (hdrl_free*)&hdrl_image_imgview_delete,
                                            CPL_FALSE);
            const hdrl_image * view = hdrl_image_const_row_view_create(himg, ly, uy,
                                           (hdrl_free*)&hdrl_image_imgview_delete);
            if (view == NULL) {
                hdrl_imagelist_delete(viewlist);
                return NULL;
            }
            hdrl_image_unwrap(himg);
            cpl_mask_unwrap(cpl_image_unset_bpm(err));
            cpl_image_unwrap(err);
            hdrl_imagelist_set(viewlist, (hdrl_image*)view, i);
            CPL_DIAG_PRAGMA_POP;
         }
    }

    return viewlist;
}

/**@}*/
