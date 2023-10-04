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

#include "hdrl_iter.h"
#include "hdrl_types.h"
#include <cpl.h>

/*-----------------------------------------------------------------------------
                                   Static
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_iter   Iterator

 */
/*----------------------------------------------------------------------------*/

/**@{*/

/** @cond EXPERIMENTAL */

/** @cond PRIVATE */

struct _hdrl_iter_ {
    /* returns next value in sequence iterated on (e.g. image, imagelist) */
    hdrl_iter_next_f * next;
    /* optional, resets the iteration to the first element */
    hdrl_iter_reset_f * reset;
    /* optional, returns length of the iterator */
    hdrl_iter_length_f * length;
    /* state destructor */
    hdrl_free * destructor;
    /* iterator flags */
    hdrl_iter_flags flags;
    /* state structure of iterator */
    void * state;
};

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief initialize an iterator
 *
 * @param next   function pointer returning next value in sequence iterated on
 * @param reset  function pointer or NULL, reseting iteration to first element
 * @param length function pointer or NULL, returning length of sequence
 * @param flags  flags of iterator
 * @param state  state structure of iterator
 *
 * An iterator iterates on a sequence of data (e.g. imagelist, files on
 * disk, ...) and returns an element of that sequence on each call to the
 * next function. When the sequence is exhausted it returns NULL.
 *
 * Each member function is called with the iterator as first argument, from
 * this the state can be received with hdrl_iter_state(it), the state is a
 * user defined structure typically containing the current position in the
 * sequence.
 *
 * The iterator type defines what kind of data is returned by each next call.
 * The options are:
 * HDRL_ITER_INPUT: data provided is input data to be processed, it will be
                   freed by the caller of next. (FIXME: add flag to not free?)
 * HDRL_ITER_OUTPUT: data is an empty output buffer for the caller of next
 *                  to place its results into. Its memory is managed by the
 *                  iterator.
 * HDRL_ITER_IMAGE: the return of next is a cpl_image
 * HDRL_ITER_IMAGELIST: the return of next is a cpl_imagelist
 */
/* ---------------------------------------------------------------------------*/
hdrl_iter *
hdrl_iter_init(hdrl_iter_next_f * next,
                  hdrl_iter_reset_f * reset,
                  hdrl_iter_length_f * length,
                  hdrl_free * destructor,
                  hdrl_iter_flags flags,
                  void * state)
{
    /* exactly one each set */
    hdrl_iter_flags inout = (HDRL_ITER_INPUT | HDRL_ITER_OUTPUT);
    hdrl_iter_flags retflags = (HDRL_ITER_IMAGE | HDRL_ITER_IMAGELIST);
    cpl_ensure(((flags & inout) == HDRL_ITER_INPUT) ||
               ((flags & inout) == HDRL_ITER_OUTPUT),
               CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(((flags & retflags) == HDRL_ITER_IMAGE) ||
               ((flags & retflags) == HDRL_ITER_IMAGELIST),
               CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(state, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(next, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(flags != 0, CPL_ERROR_NULL_INPUT, NULL);
    hdrl_iter * it = cpl_malloc(sizeof(*it));
    it->next = next;
    it->length = length;
    it->reset = reset;
    it->destructor = destructor ? destructor : &cpl_free;
    it->flags = flags;
    it->state = state;
    return it;
}

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief return the state structure of the iterator
 * @param it iterator
 * @return state structure
 */
/* ---------------------------------------------------------------------------*/
void * hdrl_iter_state(const hdrl_iter * it)
{
    cpl_ensure(it, CPL_ERROR_NULL_INPUT, NULL);

    return it->state;
}

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief check if iterator has a certain flag
 * @param it    iterator
 * @param flags flag to check
 * @return true or false depending wheter iterator has flags set
 */
/* ---------------------------------------------------------------------------*/
cpl_boolean hdrl_iter_check(hdrl_iter * it, hdrl_iter_flags flags)
{
    cpl_ensure(it, CPL_ERROR_NULL_INPUT, CPL_FALSE);

    return (it->flags & flags) == flags;
}

/** @endcond */

/* ---------------------------------------------------------------------------*/
/**
 * @brief delete iterator structure
 * @param it iterator
 * @note will not delete the state structure, typically this is called in the
 *       user iterator destructor
 */
/* ---------------------------------------------------------------------------*/
void hdrl_iter_delete(hdrl_iter * it)
{
    if (it) {
        if (it->destructor) {
            it->destructor(it);
        }
        cpl_free(it);
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief return next element in sequence or NULL if done
 * @param it iterator
 * @return object or NULL, type of object depends on iterator type
 */
/* ---------------------------------------------------------------------------*/
void * hdrl_iter_next(hdrl_iter * it)
{
    cpl_ensure(it, CPL_ERROR_NULL_INPUT, NULL);

    return it->next(it);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief reset iterator to beginning of sequence
 * @param it iterator
 */
/* ---------------------------------------------------------------------------*/
void hdrl_iter_reset(hdrl_iter * it)
{
    if (!it) {
        cpl_error_set_message(cpl_func, CPL_ERROR_NULL_INPUT,
                              "Iterator Null");
    } else if (it->reset == NULL) {
        cpl_error_set_message(cpl_func, CPL_ERROR_UNSUPPORTED_MODE,
                                "Iterator has no reset method");
    } else {
        it->reset(it);
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief return length of sequence
 * @param it iterator
 * @return length of sequence
 */
/* ---------------------------------------------------------------------------*/
cpl_size hdrl_iter_length(hdrl_iter * it)
{
    if (!it) {
        cpl_error_set_message(cpl_func, CPL_ERROR_NULL_INPUT,
                              "Iterator Null");
        return -1;
    } else if (it->length == NULL) {
        cpl_error_set_message(cpl_func, CPL_ERROR_UNSUPPORTED_MODE,
                              "Iterator has no length method");
        return -1;
    }
    return it->length(it);
}

/**@}*/

/** @endcond */
