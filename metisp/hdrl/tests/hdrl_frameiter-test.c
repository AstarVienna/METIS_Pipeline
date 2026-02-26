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

#include <cpl.h>

#ifndef ARRAY_LEN
#define ARRAY_LEN(a) sizeof((a))/sizeof((a)[0])
#endif

/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_frameiter_test
 */
/*----------------------------------------------------------------------------*/

static void check_strides(cpl_frameset * frames, intptr_t naxes,
                          intptr_t offsets[], intptr_t strides[],
                          intptr_t dims[], intptr_t values[], intptr_t len,
                          int swap)
{
    hdrl_iter * it;
    char * soffset = cpl_sprintf("%s", "(");
    char * sstride = cpl_sprintf("%s", "(");
    char * sdim = cpl_sprintf("%s", "(");
    for (intptr_t i = 0; i < naxes; i++) {
        char * t = cpl_sprintf("%s%zd%s", soffset, offsets[i],
                               i == naxes - 1 ? ")" : ",");
        cpl_free(soffset);
        soffset = t;
        t = cpl_sprintf("%s%zd%s", sstride, strides ? strides[i] : 0,
                        i == naxes - 1 ? ")" : ",");
        cpl_free(sstride);
        sstride = t;
        t = cpl_sprintf("%s%zd%s", sdim, dims ? dims[i] : -1,
                        i == naxes - 1 ? ")" : ",");
        cpl_free(sdim);
        sdim = t;
    }
    cpl_msg_info(cpl_func, "testing offset %s, stride %s, dim %s, len %zd, "
                 "swap %d", soffset, sstride, sdim, len, swap);
    cpl_free(soffset);
    cpl_free(sstride);
    cpl_free(sdim);

    intptr_t axes[] = {HDRL_FRAMEITER_AXIS_FRAME, HDRL_FRAMEITER_AXIS_EXT};
    if (swap) {
        intptr_t tmp;
        tmp = axes[0];
        axes[0] = axes[1];
        axes[1] = tmp;
    }
    it = hdrl_frameiter_new(frames, 0, naxes, axes, offsets, strides, dims);
    cpl_test_eq(hdrl_iter_length(it), len);
    int d;
    int cnt = 0;
    for (hdrl_frameiter_data * h = hdrl_iter_next(it); h != NULL; h = hdrl_iter_next(it)) {
        cpl_test_eq(cpl_image_get(h->image, 1, 1, &d), values[cnt]);
        cpl_image_delete(h->image);
        cpl_propertylist_delete(h->plist);
        cnt++;
    }
    cpl_test_eq(hdrl_iter_length(it), cnt);
    hdrl_iter_delete(it);
}

static void test_basic(void)
{
    size_t nframes = 5;
    size_t next = 4;
    cpl_frameset * frames = cpl_frameset_new();
    for (size_t i = 0; i < nframes; i++) {
        char * fn = cpl_sprintf("hdrl_frameiter-test_%zu_%zd.fits", i, (intptr_t)getpid());
        cpl_propertylist * plist = cpl_propertylist_new();
        cpl_propertylist_update_string(plist, "TAG", fn);
        cpl_propertylist_save(plist, fn, CPL_IO_CREATE);
        cpl_propertylist_delete(plist);
        for (size_t j = 1; j < next + 1; j++) {
            cpl_image * img = cpl_image_new(50, 70, CPL_TYPE_INT);
            cpl_image_add_scalar(img, i * next + j);
            cpl_image_save(img, fn, CPL_TYPE_INT, NULL, CPL_IO_EXTEND);
            cpl_image_delete(img);
        }
        cpl_frame * frm = cpl_frame_new();
        cpl_frame_set_filename(frm, fn);
        cpl_frame_set_tag(frm, "RAW");
        cpl_frameset_insert(frames, frm);
        cpl_free(fn);
    }

    /* empty iterator test */
    {
        cpl_frameset * empty = cpl_frameset_new();
        hdrl_iter * it =
            hdrl_frameiter_new(empty, 0, 1, (intptr_t[]){HDRL_FRAMEITER_AXIS_EXT},
                               NULL, NULL, NULL);
        cpl_test_eq(hdrl_iter_length(it), 0);
        cpl_test_null(hdrl_iter_next(it));
        cpl_test_null(hdrl_iter_next(it));
        cpl_test_null(hdrl_iter_next(it));
        cpl_frameset_delete(empty);
        hdrl_iter_delete(it);
    }
    /* one iteration axis */
    {
        intptr_t values[next];
        for (size_t i = 0; i < ARRAY_LEN(values); i++) {
            values[i] = i + 1;
        }
        intptr_t offsets[] = {1};
        check_strides(frames, 1, offsets, NULL, NULL, values, ARRAY_LEN(values), 1);
    }
    /* two iteration axis basic */
    {
        intptr_t values[nframes * next];
        for (size_t i = 0; i < ARRAY_LEN(values); i++) {
            values[i] = i + 1;
        }
        intptr_t offsets[] = {0, 1};
        intptr_t strides[] = {1, 1};
        check_strides(frames, 2, offsets, strides, NULL, values,
                      ARRAY_LEN(values), 0);
    }
    /* two iteration axis offset (1,1) */
    {
        intptr_t values[(nframes - 1) * next];
        for (size_t i = 0; i < ARRAY_LEN(values); i++) {
            values[i] = i + 1 + next;
        }
        intptr_t offsets[] = {1, 1};
        intptr_t strides[] = {1, 1};
        check_strides(frames, 2, offsets, strides, NULL, values,
                      ARRAY_LEN(values), 0);
    }
    /* two iteration axis stride (1,2) */
    {
        intptr_t values[nframes * (next / 2 + next % 2)];
        intptr_t c = 0;
        for (size_t i = 0; i < nframes; i++) {
            for (size_t j = 1; j < next + 1; j+=2) {
                values[c++] = i * next + j;
            }
        }
        cpl_test_eq(ARRAY_LEN(values), c);
        intptr_t offsets[] = {0, 1};
        intptr_t strides[] = {1, 2};
        check_strides(frames, 2, offsets, strides, NULL, values,
                      ARRAY_LEN(values), 0);
    }
    /* two iteration axis stride (2,1) */
    {
        intptr_t values[(nframes / 2 + nframes % 2) * (next)];
        intptr_t c = 0;
        for (size_t i = 0; i < nframes; i+=2) {
            for (size_t j = 1; j < next + 1; j++) {
                values[c++] = i * next + j;
            }
        }
        cpl_test_eq(ARRAY_LEN(values), c);
        intptr_t offsets[] = {0, 1};
        intptr_t strides[] = {2, 1};
        check_strides(frames, 2, offsets, strides, NULL, values,
                      ARRAY_LEN(values), 0);
    }
    /* two iteration axis stride (2,2) */
    {
        intptr_t values[(nframes / 2 + nframes % 2) * (next / 2 + next % 2)];
        intptr_t c = 0;
        for (size_t i = 0; i < nframes; i+=2) {
            for (size_t j = 1; j < next + 1; j+=2) {
                values[c++] = i * next + j;
            }
        }
        cpl_test_eq(ARRAY_LEN(values), c);
        intptr_t offsets[] = {0, 1};
        intptr_t strides[] = {2, 2};
        check_strides(frames, 2, offsets, strides, NULL, values,
                      ARRAY_LEN(values), 0);
    }
    /* stride 1,0 */
    {
        intptr_t values[nframes * next];
        for (size_t i = 0; i < ARRAY_LEN(values); i++) {
            values[i] = (i / next) * next + 1;
        }
        intptr_t offsets[] = {0, 1};
        intptr_t strides[] = {1, 0};
        check_strides(frames, 2, offsets, strides, NULL, values,
                      ARRAY_LEN(values), 0);
    }
    /* stride 0,1 */
    {
        intptr_t values[nframes * next];
        for (size_t i = 0; i < ARRAY_LEN(values); i++) {
            values[i] = (i % next) + 1;
        }
        intptr_t offsets[] = {0, 1};
        intptr_t strides[] = {0, 1};
        check_strides(frames, 2, offsets, strides, NULL, values,
                      ARRAY_LEN(values), 0);
    }
    /* stride 0,0 */
    {
        intptr_t values[nframes * next];
        for (size_t i = 0; i < ARRAY_LEN(values); i++) {
            values[i] = 1;
        }
        intptr_t offsets[] = {0, 1};
        intptr_t strides[] = {0, 0};
        check_strides(frames, 2, offsets, strides, NULL, values,
                      ARRAY_LEN(values), 0);
    }
    /* swapped axes */
    {
        intptr_t values[nframes * next];
        intptr_t c = 0;
        for (size_t j = 1; j < next + 1; j++) {
            for (size_t i = 0; i < nframes; i++) {
                values[c++] = i * next + j;
            }
        }
        cpl_test_eq(ARRAY_LEN(values), c);
        intptr_t offsets[] = {1, 0};
        check_strides(frames, 2, offsets, NULL, NULL, values,
                      ARRAY_LEN(values), 1);
    }
    /* dim -1, -1 */
    {
        intptr_t values[nframes * next];
        cpl_msg_debug(cpl_func,"The related valgrind error present if compiled"
                        " with O3 optimisation is most probably a  false "
                        "positive - adding this message suppresses the error.");
        for (size_t i = 0; i < ARRAY_LEN(values); i++) {
            values[i] = i + 1;
        }
        intptr_t offsets[] = {0, 1};
        intptr_t dims[] = {-1, -1};
        check_strides(frames, 2, offsets, NULL, dims, values,
                      ARRAY_LEN(values), 0);
    }
    /* dim -1, x*/
    {
        intptr_t values[nframes * 3];
        intptr_t c = 0;
        for (size_t i = 0; i < nframes; i++) {
            for (size_t j = 1; j < 3 + 1; j++) {
                values[c++] = i * next + j;
            }
        }
        cpl_test_eq(ARRAY_LEN(values), c);
        intptr_t offsets[] = {0, 1};
        intptr_t dims[] = {-1, 3};
        check_strides(frames, 2, offsets, NULL, dims, values,
                      ARRAY_LEN(values), 0);
    }
    /* dim x,-1*/
    {
        intptr_t values[4 * next];
        intptr_t c = 0;
        for (size_t i = 0; i < 4; i++) {
            for (size_t j = 1; j < next + 1; j++) {
                values[c++] = i * next + j;
            }
        }
        cpl_test_eq(ARRAY_LEN(values), c);
        intptr_t offsets[] = {0, 1};
        intptr_t dims[] = {4, -1};
        check_strides(frames, 2, offsets, NULL, dims, values,
                      ARRAY_LEN(values), 0);
    }

    cpl_frameset_delete(frames);
    cpl_test_zero(system("rm -f hdrl_frameiter-test_*fits"));
}

/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of hdrl_image
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    test_basic();

    return cpl_test_end(0);
}
