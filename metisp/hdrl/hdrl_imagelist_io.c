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

#include "hdrl_image.h"
#include "hdrl_imagelist.h"
#include "hdrl_imagelist_defs.h"
#include "hdrl_imagelist_view.h"
#include "hdrl_iter.h"

#include <cpl.h>
#include <assert.h>
#include <string.h>

/*-----------------------------------------------------------------------------
                                Defines
 -----------------------------------------------------------------------------*/

#define HDRL_MSG  "Imagelist with %d image(s)\n"
#define HDRL_IMSG "Image nb %d of %d in imagelist\n"

/*-----------------------------------------------------------------------------
                            Static Prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
 * @addtogroup hdrl_imagelist
 * @{
 */
/*----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                            Function codes
 -----------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief resize buffer to set size
 * @param h    imagelist to update
 * @param size capacity the buffer should have, minimum 128
 *
 * only changes the buffer size, not the number of contained images
 */
/* ---------------------------------------------------------------------------*/
static void hdrl_imagelist_set_capacity(hdrl_imagelist * h, cpl_size size)
{
    h->capacity = CX_MAX(h->ni, CX_MAX(128, size));
    h->images = cpl_realloc(h->images, h->capacity * sizeof(h->images[0]));
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Create an empty imagelist
  @return   1 newly allocated hdrl_imagelist
  @see      hdrl_imagelist_set()
  The returned hdrl_imagelist must be deallocated using hdrl_imagelist_delete()
 */
/*----------------------------------------------------------------------------*/
hdrl_imagelist * hdrl_imagelist_new(void)
{
    hdrl_imagelist * h = cpl_calloc(1, sizeof(hdrl_imagelist));
    hdrl_imagelist_set_capacity(h, 128);
    return h;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Create an hdrl_imagelist out of 2 cpl_imagelist 
  @param    imlist    the list of image
  @param    errlist   the list of errors
  @return   The new hdrl_imagelist

  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/*----------------------------------------------------------------------------*/
hdrl_imagelist * hdrl_imagelist_create(
        cpl_imagelist   *   imlist,
        cpl_imagelist   *   errlist)
{
    hdrl_imagelist  *   himlist ;
    cpl_image       *   error ;
    
    /* Check Entries */
    cpl_ensure(imlist != NULL, CPL_ERROR_NULL_INPUT, NULL);
    if (errlist) {
        cpl_ensure(cpl_imagelist_get_size(imlist) == 
                cpl_imagelist_get_size(errlist), CPL_ERROR_ILLEGAL_INPUT, NULL);
    }
    
    /* Create the new HDRL image list */
    himlist = hdrl_imagelist_new() ;
    
    /* Loop on the input image list */
    for (cpl_size i = 0; i < cpl_imagelist_get_size(imlist); i++) {
        /* Get the error image */
        hdrl_image * tmp ;
        if (errlist) {
            error = cpl_imagelist_get(errlist, i) ;
        } else {
            error = NULL ;
        }
        /* Create the HDRL image */
        tmp = hdrl_image_create(cpl_imagelist_get_const(imlist, i), error) ;

        /* Fill the HDRL image list */
        hdrl_imagelist_set(himlist, tmp, i);

    }
    return himlist ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Get the number of images in the imagelist
  @param    himlist     the list of images
  @return   The number of images or -1 on error

  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/*----------------------------------------------------------------------------*/
cpl_size hdrl_imagelist_get_size(const hdrl_imagelist * himlist)
{
    cpl_ensure(himlist != NULL, CPL_ERROR_NULL_INPUT, -1);
    assert( himlist->ni >= 0 );
    return himlist->ni;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Get number of colums of  images in the imagelist
  @param    himlist     the list of images
  @return   The number of columns of images or -1 on error

  assumes the imagelist is uniform (cpl_imagelist_is_uniform)

  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
  - CPL_ERROR_ILLEGAL_INPUT if the list is empty
 */
/*----------------------------------------------------------------------------*/
cpl_size hdrl_imagelist_get_size_x(const hdrl_imagelist * himlist)
{
    cpl_ensure(himlist != NULL, CPL_ERROR_NULL_INPUT, -1);
    cpl_ensure(himlist->ni > 0, CPL_ERROR_ILLEGAL_INPUT, -1);
    return hdrl_image_get_size_x(hdrl_imagelist_get_const(himlist, 0));
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Get number of rows of  images in the imagelist
  @param    himlist     the list of images
  @return   The number of rows of images or -1 on error

  assumes the imagelist is uniform (cpl_imagelist_is_uniform)

  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
  - CPL_ERROR_ILLEGAL_INPUT if the list is empty
 */
/*----------------------------------------------------------------------------*/
cpl_size hdrl_imagelist_get_size_y(const hdrl_imagelist * himlist)
{
    cpl_ensure(himlist != NULL, CPL_ERROR_NULL_INPUT, -1);
    cpl_ensure(himlist->ni > 0, CPL_ERROR_ILLEGAL_INPUT, -1);
    return hdrl_image_get_size_y(hdrl_imagelist_get_const(himlist, 0));
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Get an image from a list of images
  @param    himlist the image list
  @param    inum    the image id (from 0 to number of images-1)
  @return   A pointer to the image or NULL in error case.

  The returned pointer refers to already allocated data.

  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
  - CPL_ERROR_ACCESS_OUT_OF_RANGE if inum is bigger thant the list size
  - CPL_ERROR_ILLEGAL_INPUT if inum is negative
 */
/*----------------------------------------------------------------------------*/
hdrl_image * hdrl_imagelist_get(
        const hdrl_imagelist  *   himlist,
        cpl_size                  inum)
{
    cpl_ensure(himlist != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(inum >= 0, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(inum < himlist->ni, CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);
    return himlist->images[inum];
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Get an image from a list of images
  @param    himlist the image list
  @param    inum    the image id (from 0 to number of images-1)
  @return   A pointer to the image or NULL in error case.
  @see hdrl_imagelist_get
 */
/*----------------------------------------------------------------------------*/
const hdrl_image * hdrl_imagelist_get_const(
        const hdrl_imagelist    *   himlist,
        cpl_size                    inum)
{
    cpl_ensure(himlist != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(inum >= 0, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(inum < himlist->ni, CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);
    return himlist->images[inum];
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Insert an image into an imagelist
  @param    himlist  The imagelist
  @param    himg     The image to insert
  @param    pos      The list position (from 0 to number of images)
  @return   CPL_ERROR_NONE or the relevant cpl_error_code on error

  It is allowed to specify the position equal to the number of images in the
  list. This will increment the size of the imagelist.

  No action occurs if an image is inserted more than once into the same
  position. It is allowed to insert the same image into two different
  positions in a list.

  The image is inserted at the position pos in the image list. If the image
  already there is only present in that one location in the list, then the
  image is deallocated.

  It is not allowed to insert images of different size into a list.

  The added image is owned by the imagelist object, which deallocates it
  hdrl_imagelist_delete is called. Other option is to use 
  hdrl_imagelist_unset to recover ownership of the image, in which case 
  the hdrl_imagelist object is not longer responsible for deallocating it.

  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
  - CPL_ERROR_ILLEGAL_INPUT if pos is negative
  - CPL_ERROR_TYPE_MISMATCH if himg and himlist are of different types
  - CPL_ERROR_INCOMPATIBLE_INPUT if himg and himlist have different sizes
  - CPL_ERROR_ACCESS_OUT_OF_RANGE if pos is bigger than the number of
    images in himlist
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_set(
        hdrl_imagelist  *   himlist,
        hdrl_image      *   himg,
        cpl_size            pos)
{
    cpl_ensure_code(himlist,            CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(himg,                CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(pos >= 0,          CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(pos <= himlist->ni, CPL_ERROR_ACCESS_OUT_OF_RANGE);

    /* Do nothing if the image is already there */
    if (pos < himlist->ni && himg == himlist->images[pos]) 
        return CPL_ERROR_NONE;

    if (pos > 0 || himlist->ni > 1) {
        /* Require images to have the same size and type */
        cpl_ensure_code(hdrl_image_get_size_x(himg) ==
                        hdrl_image_get_size_x(himlist->images[0]),
                        CPL_ERROR_INCOMPATIBLE_INPUT);
        cpl_ensure_code(hdrl_image_get_size_y(himg) ==
                        hdrl_image_get_size_y(himlist->images[0]),
                        CPL_ERROR_INCOMPATIBLE_INPUT);
        // TODO : type of hdrl_image ??
        //cpl_ensure_code(hdrl_image_get_type(himg) ==
        //                hdrl_image_get_type(himlist->images[0]),
        //                CPL_ERROR_TYPE_MISMATCH);
    }

    if (pos == himlist->ni) {
        /* double buffer if required */
        if (pos >= himlist->capacity) {
            hdrl_imagelist_set_capacity(himlist, 2 * pos);
        }
        himlist->ni++;
    } else {
        /* Check if the image at the position to be overwritten
           is present in only one position */
        int i;

        for (i = 0; i < himlist->ni; i++) {
            if (i != pos && himlist->images[i] == himlist->images[pos]) break;
        }
        if (i == himlist->ni) {
            /* The image at the position to be overwritten
               is present in only one position, so delete it */
            hdrl_image_delete(himlist->images[pos]);
        }
    }

    himlist->images[pos] = himg;

    return CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Remove an image from an imagelist
  @param    himlist The imagelist
  @param    pos     The list position (from 0 to number of images-1)
  @return   The pointer to the removed image or NULL in error case

  The specified image is not deallocated, it is simply removed from the
  list. The pointer to the image is returned to let the user decide to
  deallocate it or not.
  Eventually, the image will have to be deallocated with hdrl_image_delete().

  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
  - CPL_ERROR_ILLEGAL_INPUT if pos is negative
  - CPL_ERROR_ACCESS_OUT_OF_RANGE if pos is bigger than the number of
    images in himlist
 */
/*----------------------------------------------------------------------------*/
hdrl_image * hdrl_imagelist_unset(
        hdrl_imagelist  *   himlist,
        cpl_size            pos)
{
    hdrl_image  *   out;
    cpl_size        i;

    cpl_ensure(himlist, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(pos >= 0, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(pos < himlist->ni, CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);

    /* Get pointer to image to be removed */
    out = himlist->images[pos];

    /* Move the following images one position towards zero */
    for (i=pos + 1; i < himlist->ni; i++) {
        himlist->images[i-1] = himlist->images[i];
    }

    /* Decrement of the size */
    himlist->ni--;

    /* shrink the buffer if its significantly too large */
    if (himlist->ni < himlist->capacity / 2) {
        hdrl_imagelist_set_capacity(himlist, himlist->ni / 2);
    }

    return out;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Free all memory used by a hdrl_imagelist object including the images
  @param    himlist    The image list or NULL
  @return   Nothing
  @see      hdrl_imagelist_empty(), hdrl_imagelist_unwrap()
 */
/*----------------------------------------------------------------------------*/
void hdrl_imagelist_delete(hdrl_imagelist * himlist)
{
    if (himlist != NULL) {
        hdrl_imagelist_empty(himlist);
        hdrl_imagelist_unwrap(himlist);
    }
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Empty an imagelist and deallocate all its images
  @param    himlist  The image list or NULL
  @return   Nothing
  @see  hdrl_imagelist_empty(), hdrl_imagelist_delete()
  @note If @em himlist is @c NULL nothing is done and no error is set.

  After the call the image list can be populated again. It must eventually
  be deallocated with a call to hdrl_imagelist_delete().
 */
/*----------------------------------------------------------------------------*/
void hdrl_imagelist_empty(hdrl_imagelist * himlist)
{
    if (himlist != NULL) {
        while (himlist->ni > 0) { /* An iteration may unset more than 1 image */

        	cpl_size i = himlist->ni - 1;
            hdrl_image *del = hdrl_imagelist_unset(himlist, i);

            /* If this image was inserted more than once into the list,
               the other insertions must be unset without a delete. */
            while (--i >= 0) {
                if (himlist->images[i] == del) {
                    /* This image was inserted more than once in the list */
                    del = hdrl_imagelist_unset(himlist, i);
                }
            }

            hdrl_image_delete(del);
        }
    }
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Duplicate an image list
  @param    himlist Source image list.
  @return   1 newly allocated image list, or NULL on error.

  Copy an image list into a new image list object.
  The returned image list must be deallocated using hdrl_imagelist_delete().

  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/*----------------------------------------------------------------------------*/
hdrl_imagelist * hdrl_imagelist_duplicate(const hdrl_imagelist * himlist)
{
    hdrl_imagelist *   out;
    cpl_size          i;

    cpl_ensure(himlist != NULL, CPL_ERROR_NULL_INPUT, NULL);

    /* Create the new imagelist */
    out = hdrl_imagelist_new();

    /* Duplicate the images */
    for (i=0; i<himlist->ni; i++) {
        hdrl_imagelist_set(out, hdrl_image_duplicate(himlist->images[i]), i);
    }

    return out;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Determine if an imagelist contains images of equal size and type
  @param    himlist The imagelist to check
  @return   Zero if ok, positive if not consistent and negative on error.

  The function returns 1 if the list is empty.

  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
 */
/*----------------------------------------------------------------------------*/
int hdrl_imagelist_is_consistent(const hdrl_imagelist * himlist)
{
    cpl_ensure(himlist != NULL, CPL_ERROR_NULL_INPUT, -1);
    if (himlist->ni == 0) return 1;

    /* Check the images */
    // TODO
    return 0;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Dump structural information of images in an imagelist
  @param    himlist Imagelist to dump
  @param    stream  Output stream, accepts @c stdout or @c stderr
  @return   CPL_ERROR_NONE or the relevant cpl_error_code on error

  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
  - CPL_ERROR_FILE_IO if a write operation fails
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_dump_structure(
        const hdrl_imagelist    *   himlist,
        FILE                    *   stream)
{
    const int    msgmin = (int)strlen(HDRL_MSG) - 5;

    int i;

    cpl_ensure_code(himlist   != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(stream != NULL, CPL_ERROR_NULL_INPUT);

    cpl_ensure_code( fprintf(stream,  HDRL_MSG, (int)himlist->ni) >= msgmin,
             CPL_ERROR_FILE_IO );

    for (i = 0; i < himlist -> ni; i++) {
    const hdrl_image    *   image   = hdrl_imagelist_get_const(himlist, i);
    const int               imsgmin = (int)strlen(HDRL_IMSG) - 5;

    cpl_ensure_code( fprintf(stream, HDRL_IMSG, i, (int)himlist->ni) >= imsgmin,
             CPL_ERROR_FILE_IO );

    cpl_ensure_code( !hdrl_image_dump_structure(image, stream),
             cpl_error_get_code() );
    }

    return CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Dump pixel values of images in a imagelist
  @param    himlist    Imagelist to dump
  @param    llx     Specifies the window position
  @param    lly     Specifies the window position
  @param    urx     Specifies the window position
  @param    ury     Specifies the window position
  @param    stream  Output stream, accepts @c stdout or @c stderr
  @return   CPL_ERROR_NONE or the relevant cpl_error_code on error

  Possible cpl_error_code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
  - CPL_ERROR_FILE_IO if a write operation fails
  - CPL_ERROR_ACCESS_OUT_OF_RANGE if the defined window is not in the image
  - CPL_ERROR_ILLEGAL_INPUT if the window definition is wrong (e.g llx > urx)
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_dump_window(
        const hdrl_imagelist    *   himlist,
        cpl_size                    llx,
        cpl_size                    lly,
        cpl_size                    urx, 
        cpl_size                    ury,
        FILE *                      stream)
{
    cpl_size i;

    cpl_ensure_code(himlist   != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(stream != NULL, CPL_ERROR_NULL_INPUT);

    for (i = 0; i < himlist -> ni; i++) {
    const hdrl_image    *   image   = hdrl_imagelist_get_const(himlist, i);
    const int               imsgmin = (int)strlen(HDRL_IMSG) - 5;

    cpl_ensure_code( fprintf(stream,  HDRL_IMSG, (int)i,
                             (int)himlist->ni) >= imsgmin, CPL_ERROR_FILE_IO );

    cpl_ensure_code( !hdrl_image_dump_window(image, llx, lly, urx, ury,
                           stream),
             cpl_error_get_code() );
    }
    return CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Free memory used by a hdrl_imagelist object, except the images
  @param    himlist    The image list or NULL
  @return   Nothing
  @see hdrl_imagelist_empty()
  @note The caller must have pointers to all images in the list and is
        reponsible for their deallocation. If @em himlist is @c NULL nothing is
        done and no error is set.
 */
/*----------------------------------------------------------------------------*/
void hdrl_imagelist_unwrap(hdrl_imagelist * himlist)
{
    if (himlist != NULL) {
        cpl_free(himlist->images);
        cpl_free(himlist);
    }
    return;
}

typedef struct {
    const hdrl_imagelist * hlist;
    cpl_size ny;
    cpl_size prev_pos;
    cpl_size pos;
    cpl_size nrows;
    cpl_size overlap;
    hdrl_imagelist * last_view;
} hdrl_imagelist_row_slices_iter;

static void * hdrl_imagelist_row_slices_next(hdrl_iter * it)
{
    hdrl_imagelist_row_slices_iter * s = hdrl_iter_state(it);
    if (s->pos > s->ny) {
        return NULL;
    }
    cpl_size lower = CX_MAX(s->pos - s->overlap, 1);
    cpl_size upper = CX_MIN(s->pos + s->nrows + s->overlap - 1, s->ny);
    /* const iterator means you cannot modify the data but the created views
     * can have a NULL bpm which can be faster e.g. when calling
     * cpl_image_new_from_accepted on the view */
    /* TODO could set memory readonly? */
    hdrl_imagelist * view;

    CPL_DIAG_PRAGMA_PUSH_IGN(-Wcast-qual);
    if (hdrl_iter_check(it, HDRL_ITER_CONST)) {
        view = (hdrl_imagelist*)
            hdrl_imagelist_const_row_view((hdrl_imagelist*)s->hlist,
                                          lower, upper);
    }
    else {
        view = hdrl_imagelist_row_view((hdrl_imagelist*)s->hlist,
                                       lower, upper);
    }
    CPL_DIAG_PRAGMA_POP;

    s->prev_pos = s->pos;
    s->pos = CX_MIN(s->pos + s->nrows - 1, s->ny) + 1;
    if (hdrl_iter_check(it, HDRL_ITER_OWNS_DATA)) {
        hdrl_imagelist_delete(s->last_view);
        s->last_view = view;
    }
    return view;
}

hdrl_il_rowsliceiter_data
hdrl_imagelist_iter_row_slices_get_data(const hdrl_iter * it)
{
    hdrl_imagelist_row_slices_iter * s = hdrl_iter_state(it);

    if (s->prev_pos == 1) {
        return (hdrl_il_rowsliceiter_data){1., CX_MIN(s->nrows, s->ny)};
    }
    else {
        return (hdrl_il_rowsliceiter_data){s->overlap + 1, s->overlap + s->pos - s->prev_pos};
    }
}

static cpl_size hdrl_imagelist_row_slices_length(hdrl_iter * it)
{
    hdrl_imagelist_row_slices_iter * s = hdrl_iter_state(it);
    return s->ny / s->nrows + ((s->ny % s->nrows) != 0);
}

static void hdrl_imagelist_iter_delete(void * it)
{
    if (!it)
        return;
    hdrl_imagelist_row_slices_iter * s = hdrl_iter_state(it);
    hdrl_imagelist_delete(s->last_view);
    cpl_free(s);
}

/* TODO, add offset and stride for parallel processing? or imagelist_split? */
hdrl_iter * hdrl_imagelist_get_iter_row_slices(const hdrl_imagelist * hlist,
                                                 cpl_size nrows,
                                                 cpl_size overlap,
                                                 hdrl_iter_flags flags)
{
    cpl_ensure(hlist, CPL_ERROR_NULL_INPUT, NULL);
    /* 0 accepted for now, could mean choosen by function */
    cpl_ensure(nrows >= 0, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(hdrl_imagelist_get_size(hlist) > 0,
               CPL_ERROR_ILLEGAL_INPUT, NULL);

    hdrl_imagelist_row_slices_iter * state = cpl_malloc(sizeof(*state));
    state->hlist = hlist;
    state->ny = hdrl_imagelist_get_size_y(hlist);
    state->prev_pos = 1;
    state->pos = 1;
    state->overlap = CX_MAX(overlap, 0);
    state->nrows = CX_MAX(nrows, 1);
    state->last_view = NULL;

    return hdrl_iter_init(hdrl_imagelist_row_slices_next, NULL,
                          hdrl_imagelist_row_slices_length,
                          hdrl_imagelist_iter_delete,
                          HDRL_ITER_INPUT | HDRL_ITER_IMAGELIST | flags,
                          state);
}


/**@}*/
