/*
 * This file is part of the HDRL
 * Copyright (C) 2016 European Southern Observatory
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

#include "hdrl_frameiter.h"
#include "hdrl_iter.h"

#include <cpl.h>

/*-----------------------------------------------------------------------------
                            Static Prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
 * @addtogroup hdrl_frameiter
 * @{
 */
/*----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                            Function codes
 -----------------------------------------------------------------------------*/
#define MAX_DIM 32

typedef struct {
    /* frameset being iterated */
    const cpl_frameset * frames;
    /* number of axes being iterated */
    intptr_t naxes;
    /* global index/counter */
    intptr_t index;
    /* dimensions of iteration space */
    intptr_t dim[MAX_DIM];
    /* current position */
    intptr_t pos[MAX_DIM];
    /* current count in dimension */
    intptr_t cnt[MAX_DIM];
    /* offsets in data */
    intptr_t offsets[MAX_DIM];
    /* iteration strides */
    intptr_t strides[MAX_DIM];
    /* iteration axes */
    intptr_t axes[MAX_DIM];
    /* last iteration axis */
    intptr_t naxes_max;
    hdrl_frameiter_data data;
} hdrl_frameiter_state;

static cpl_size hdrl_frameiter_length(hdrl_iter * it);
static void hdrl_frameiter_delete(void * it);
static void * hdrl_frameiter_next(hdrl_iter * it);

/* ---------------------------------------------------------------------------*/
/**
 * @brief create iterator over cpl_frameset
 *
 * @param frames  frames to iterate
 * @param flags    flags of iterator
 * @param naxes   number axis to iterate
 * @param axes    axes iteration order
 * @param offsets offsets in axes (NULL for no offset)
 * @param strides strides in axes (NULL for stride 1)
 * @param dims    dimensions of axes (NULL for dimensions of data)
 *
 *  Create iterator over uniform frameset and extensions.
 *  Treats data as [nframes,next,nx,ny,nz] dimensional array and iterates over
 *  some axis of this data.
 *  Currently only supports iterating over frame and extension axis and
 *  returning 2d images.
 *  E.g. create iterator iterating first over the frames beginning at offset 0
 *  and stride 1, then over the extensions beginning at offset 2 (primary is
 *  offset 0) with stride 2. It returns a cpl_image in each iteration which is
 *  owned by the caller. The dimensions are defined by the data in the frames:
 *
 *  it = hdrl_frameiter_new(frames, 2,
 *                          (intptr_t[]){HDRL_FRAMEITER_AXIS_FRAME,
 *                                       HDRL_FRAMEITER_AXIS_EXT},
 *                          (intptr_t[]){0, 2}, (intptr_t[]){1, 2}, NULL),
 *  for (cpl_image * h = hdrl_iter_next(it); h != NULL;
 *       h = hdrl_iter_next(it)) {
 *      cpl_image_delete(h);
 *  }
 *  hdrl_iter_delete(it);
 */
/* ---------------------------------------------------------------------------*/
hdrl_iter *
hdrl_frameiter_new(const cpl_frameset * frames, hdrl_iter_flags flags,
                   intptr_t naxes,
                   intptr_t * axes, intptr_t * offsets, intptr_t * strides,
                   intptr_t * dims)
{
    hdrl_frameiter_state * state = cpl_calloc(sizeof(*state), 1);
    state->frames = frames;
    state->naxes = naxes;
    /* negative to indicate no iteration has happened yet */
    state->index = -1;
    /* setup dimensions from data, adapted for user parameters later */
    state->dim[HDRL_FRAMEITER_AXIS_FRAME] = cpl_frameset_get_size(frames);
    for (cpl_size i = 0; i < cpl_frameset_get_size(frames); i++) {
        const cpl_frame * frm = cpl_frameset_get_position_const(frames, i);
        const char * fn = cpl_frame_get_filename(frm);
        cpl_size next = cpl_frame_get_nextensions(frm);
        // todo check shape equality over all frames/ext
        state->dim[HDRL_FRAMEITER_AXIS_EXT] = next + 1;
        for (cpl_size j = 0; j < next + 1; j++) {
            cpl_propertylist * plist =
                cpl_propertylist_load_regexp(fn, j, "NAXIS.*", 0);
            if (!cpl_propertylist_has(plist, "NAXIS")) {
                cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT, "NO DATA");
            }
            state->naxes_max = HDRL_FRAMEITER_AXIS_EXT + cpl_propertylist_get_int(plist, "NAXIS");
            for (int k = 0; k < cpl_propertylist_get_int(plist, "NAXIS"); k++) {
                char * buf = cpl_sprintf("NAXIS%d", k + 1);
                state->dim[HDRL_FRAMEITER_AXIS_NAXIS1 + k] = cpl_propertylist_get_int(plist, buf);
                cpl_free(buf);
            }
            cpl_propertylist_delete(plist);
        }
    }
    if (state->naxes_max > HDRL_FRAMEITER_AXIS_NAXIS2 ||
        naxes > HDRL_FRAMEITER_AXIS_NAXIS2) {
        cpl_error_set_message(cpl_func, CPL_ERROR_UNSUPPORTED_MODE,
                              "UNSUPPORTED MODE");
    }
    if (cpl_error_get_code()) {
        cpl_free(state);
        return NULL;
    }

    /* setup iteration space in data */
    for (intptr_t i = 0; i < naxes; i++) {
        intptr_t offset = offsets == NULL ? 0 : offsets[i];
        intptr_t stride = strides == NULL ? 1 : strides[i];
        state->pos[axes[i]] = offset;
        state->offsets[axes[i]] = offset;
        state->strides[axes[i]] = stride;
        if (dims && dims[i] > 0) {
            /* TODO error checking? */
            state->dim[axes[i]] = dims[i];
        }
        else {
            state->dim[axes[i]] -= offset;
            if (stride != 0) {
                if (state->dim[axes[i]] % stride) {
                    state->dim[axes[i]] /= stride;
                    state->dim[axes[i]] += 1;
                }
                else {
                    state->dim[axes[i]] /= stride;
                }
            }
        }
        state->axes[i] = axes[i];
    }
    return hdrl_iter_init(&hdrl_frameiter_next, NULL,
                          &hdrl_frameiter_length,
                          &hdrl_frameiter_delete,
                          HDRL_ITER_INPUT | HDRL_ITER_IMAGE | flags, state);
}

static cpl_size hdrl_frameiter_length(hdrl_iter * it)
{
    hdrl_frameiter_state * state = hdrl_iter_state(it);
    intptr_t sz = 1;
    for (intptr_t i = 0; i < state->naxes; i++) {
        sz *= state->dim[state->axes[i]];
    }
    return sz;
}

static void hdrl_frameiter_delete(void * it)
{
    if (!it)
        return;
    hdrl_frameiter_state * state = hdrl_iter_state(it);
    if (hdrl_iter_check(it, HDRL_ITER_OWNS_DATA)) {
        cpl_image_delete(state->data.image);
        cpl_propertylist_delete(state->data.plist);
    }
    cpl_free(state);
}

static cpl_boolean hdrl_frameiter_done(hdrl_iter * it)
{
    hdrl_frameiter_state * state = hdrl_iter_state(it);
    return state->index >= hdrl_frameiter_length(it);
}

static void get_data(hdrl_frameiter_state * state)
{
    if (state->naxes == 2 && state->naxes_max == HDRL_FRAMEITER_AXIS_NAXIS2) {

        if (state->axes[0] <= HDRL_FRAMEITER_AXIS_EXT &&
            state->axes[1] <= HDRL_FRAMEITER_AXIS_EXT ){

            cpl_msg_debug(cpl_func,  "Getting frame %zd, ext %zd",
									 state->pos[HDRL_FRAMEITER_AXIS_FRAME],
									 state->pos[HDRL_FRAMEITER_AXIS_EXT]);

            const cpl_frame *frm = cpl_frameset_get_position_const(
            	state->frames, state->pos[HDRL_FRAMEITER_AXIS_FRAME]);

            state->data.image = cpl_image_load(cpl_frame_get_filename(frm),
                CPL_TYPE_UNSPECIFIED, 0, state->pos[HDRL_FRAMEITER_AXIS_EXT]);

            state->data.plist = cpl_propertylist_load(
            	cpl_frame_get_filename(frm), state->pos[HDRL_FRAMEITER_AXIS_EXT]);

        } else {
            cpl_error_set_message(cpl_func, CPL_ERROR_UNSUPPORTED_MODE, "UNSUPPORTED MODE");
        }

    } else if (state->naxes   == 1                         &&
    		   state->axes[0] == HDRL_FRAMEITER_AXIS_FRAME ){
        /*
        cpl_msg_debug(cpl_func, "Getting frame %zd, ext 0",
                                state->pos[HDRL_FRAMEITER_AXIS_FRAME]);
        */
        const cpl_frame * frm = cpl_frameset_get_position_const(
        	state->frames, state->pos[HDRL_FRAMEITER_AXIS_FRAME]);

        state->data.image = cpl_image_load(
        	cpl_frame_get_filename(frm), CPL_TYPE_UNSPECIFIED, 0, 0);

        state->data.plist = cpl_propertylist_load(
        	cpl_frame_get_filename(frm), 0);

    } else if (state->naxes   == 1                       &&
    		   state->axes[0] == HDRL_FRAMEITER_AXIS_EXT ){

        cpl_msg_debug(cpl_func, "Getting frame 0, ext %zd",
                                state->pos[HDRL_FRAMEITER_AXIS_EXT]);

        const cpl_frame * frm = cpl_frameset_get_position_const(
        	state->frames, 0);

        state->data.image = cpl_image_load(cpl_frame_get_filename(frm),
        	CPL_TYPE_UNSPECIFIED, 0, state->pos[HDRL_FRAMEITER_AXIS_EXT]);

        state->data.plist = cpl_propertylist_load(cpl_frame_get_filename(frm),
            state->pos[HDRL_FRAMEITER_AXIS_EXT]);

    } else {
        cpl_error_set_message(cpl_func, CPL_ERROR_UNSUPPORTED_MODE, "UNSUPPORTED MODE");
    }
}

static void * hdrl_frameiter_next(hdrl_iter * it)
{
    hdrl_frameiter_state * state = hdrl_iter_state(it);
    /* iterate to next position  and return data, iterator starts at first
     * position */
    state->index++;
    for (intptr_t i = state->naxes - 1; state->index > 0 && i >= 0; i--) {
        intptr_t idx = state->axes[i];
        if (state->cnt[idx] + 1 < state->dim[idx]) {
            state->cnt[idx]++;
            state->pos[idx] += state->strides[idx];
            break;
        }
        else {
            state->cnt[idx] = 0;
            state->pos[idx] = state->offsets[idx];
        }
    }

    if (hdrl_iter_check(it, HDRL_ITER_OWNS_DATA)) {
        cpl_image_delete(state->data.image);
        cpl_propertylist_delete(state->data.plist);
        state->data.image = NULL;
        state->data.plist = NULL;
    }

    if (hdrl_frameiter_done(it)) {
        return NULL;
    }

    get_data(state);
    return &state->data;

}

/**@}*/
