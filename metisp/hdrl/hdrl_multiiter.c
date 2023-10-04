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

#include "hdrl_multiiter.h"
#include "hdrl_iter.h"

#include <cpl.h>
#include <assert.h>

/*-----------------------------------------------------------------------------
                            Static Prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
 * @addtogroup hdrl_multiiter
 * @{
 */
/*----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                            Function codes
 -----------------------------------------------------------------------------*/
#define MAX_ITERS 32

typedef struct {
    const cpl_frameset * frames;
    hdrl_iter * iters[MAX_ITERS];
    intptr_t niters;
    void * data[MAX_ITERS];
} hdrl_multiiter_state;

static cpl_size hdrl_multiiter_length(hdrl_iter * it);
static void hdrl_multiiter_delete(void * it);
static void * hdrl_multiiter_next(hdrl_iter * it);

/* ---------------------------------------------------------------------------*/
/**
 * @brief iterate over multiple iterators
 *
 * @param niters   number of iterators
 * @param iters    array of iterators
 * @param flags    flags of iterator
 *
 * Iterates over multiple iterators returning their results as an array of
 * pointers of the same length as number of iterators.
 * The length of all iterators must currently be equal unless
 * the HDRL_ITER_ALLOW_EMPTY flag is set iterators. Then the iterator will
 * return NULL for the exhausted iterator entries.
 * The multiiter may have HDRL_ITER_OWNS_DATA set in which case it will delete
 * the result values itself. To take ownership set the pointer in the multiiter
 * result to NULL.
 */
/* ---------------------------------------------------------------------------*/
hdrl_iter *
hdrl_multiiter_new(intptr_t niters, hdrl_iter * iters[], hdrl_iter_flags flags)
{
    cpl_ensure(niters > 0, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(iters, CPL_ERROR_NULL_INPUT, NULL);
    hdrl_multiiter_state * state = cpl_calloc(sizeof(*state), 1);
    state->niters = niters;
    cpl_size nlen = hdrl_iter_length(iters[0]);
    for (intptr_t i = 0; i < niters; i++) {
        state->iters[i] = iters[i];
        /* TODO add broadcasting */
        if (!(flags & HDRL_ITER_ALLOW_EMPTY) &&
            hdrl_iter_length(iters[i]) != nlen) {
            cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                                  "Iterator length must match");
        }
    }

    return hdrl_iter_init(&hdrl_multiiter_next, NULL,
                          &hdrl_multiiter_length,
                          &hdrl_multiiter_delete,
                          HDRL_ITER_INPUT | HDRL_ITER_IMAGE | flags,
                          state);
}

static cpl_size hdrl_multiiter_length(hdrl_iter * it)
{
    hdrl_multiiter_state * state = hdrl_iter_state(it);
    return hdrl_iter_length(state->iters[0]);
}

static void hdrl_multiiter_delete(void * it)
{
    if (!it)
        return;
    hdrl_multiiter_state * state = hdrl_iter_state(it);
    for (intptr_t i = 0; i < state->niters; i++) {
        hdrl_iter_delete(state->iters[i]);
    }
    cpl_free(state);
}


static void * hdrl_multiiter_next(hdrl_iter * it)
{
    hdrl_multiiter_state * state = hdrl_iter_state(it);
    int done = 0;
    for (intptr_t i = 0; i < state->niters; i++) {
        state->data[i] = hdrl_iter_next(state->iters[i]);
        if (state->data[i] == NULL) {
            done++;
        }
        assert(hdrl_iter_check(it, HDRL_ITER_ALLOW_EMPTY) ||
               (done && !state->data[i]) || (!done && state->data[i]));
    }
    /* if empties are allowed we are done when all iterators are done */
    if (hdrl_iter_check(it, HDRL_ITER_ALLOW_EMPTY)) {
        return done == state->niters ? NULL : state->data;
    }
    else {
        return done ? NULL : state->data;
    }
}

/**@}*/
