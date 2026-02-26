/* $Id: hdrl_multiiter-test.c,v 1.0 2017 Exp $
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                    Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_multiiter.h"
#include "hdrl_frameiter.h"

#include <cpl.h>


/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_buffer_test
            Testing of hdrl_buffer module
 */
/*----------------------------------------------------------------------------*/

cpl_frameset * create_frames(void){

	size_t nframes = 5;
	size_t next = 4;
	cpl_frameset * frames = cpl_frameset_new();
	for (size_t i = 0; i < nframes; i++) {
		char * fn = cpl_sprintf("hdrl_multiiter-test_%zu_%zd.fits", i, (intptr_t)getpid());
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

	return frames;
}

void test_invalid(cpl_frameset *frames){

	hdrl_iter *subiters[] = {
		hdrl_frameiter_new(
			frames, 0, 2,
			(intptr_t[]){HDRL_FRAMEITER_AXIS_FRAME, HDRL_FRAMEITER_AXIS_EXT},
			(intptr_t[]){0, 1},
			(intptr_t[]){1, 2},
			NULL),
		hdrl_frameiter_new(
			frames, 0, 2,
			(intptr_t[]){HDRL_FRAMEITER_AXIS_FRAME,	HDRL_FRAMEITER_AXIS_EXT},
			(intptr_t[]){0, 2},
			(intptr_t[]){1, 2},
			NULL),
	};
	cpl_test_error(CPL_ERROR_NONE);

	/* empty and fail tests */
	hdrl_iter *it;

	it = hdrl_multiiter_new(0, subiters, HDRL_ITER_ALLOW_EMPTY);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
	cpl_test_null(it);

	it = hdrl_multiiter_new(2, NULL, HDRL_ITER_ALLOW_EMPTY);
	cpl_test_error(CPL_ERROR_NULL_INPUT);
	cpl_test_null(it);
/*
	it = hdrl_multiiter_new(2, subiters, HDRL_ITER_IMAGELIST);
	cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
	cpl_test_null(it);
*/
	it = hdrl_multiiter_new(2, subiters, HDRL_ITER_ALLOW_EMPTY);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_nonnull(it);

	cpl_size size = hdrl_iter_length(it);
	cpl_test_error(CPL_ERROR_NONE);
	cpl_test_eq(size, 10);


	hdrl_iter_delete(it);
}

void test_basic(cpl_frameset *frames)
{
	hdrl_iter *subiters[] = {
		hdrl_frameiter_new(
			frames, 0, 2,
			(intptr_t[]){HDRL_FRAMEITER_AXIS_FRAME, HDRL_FRAMEITER_AXIS_EXT},
			(intptr_t[]){0, 1},
			(intptr_t[]){1, 2},
			NULL),
		hdrl_frameiter_new(
			frames, 0, 2,
			(intptr_t[]){HDRL_FRAMEITER_AXIS_FRAME,	HDRL_FRAMEITER_AXIS_EXT},
			(intptr_t[]){0, 2},
			(intptr_t[]){1, 2},
			NULL),
		NULL,
	};
	cpl_test_error(CPL_ERROR_NONE);

    /* ugly need to know length of other iterators
     * TODO add broadcasting do avoid need for dim override */
    subiters[2] =
        hdrl_frameiter_new(frames, 0, 2,
            (intptr_t[]){HDRL_FRAMEITER_AXIS_FRAME, HDRL_FRAMEITER_AXIS_EXT},
            (intptr_t[]){0, 1},
			(intptr_t[]){0, 0},
            (intptr_t[]){1, hdrl_iter_length(subiters[0])}
		);
    cpl_test_error(CPL_ERROR_NONE);

	hdrl_iter *it = hdrl_multiiter_new(3, subiters, 0);
	cpl_test_nonnull(it);

	int cnt = 0;
	for (hdrl_frameiter_data ** h = hdrl_iter_next(it); h != NULL;
			h = hdrl_iter_next(it)) {
		for (size_t i = 0; i < 3; i++) {
			int d;
			cpl_test_eq(cpl_image_get_size_x(h[i]->image), 50);
			if (i < 2) {
				cpl_test_eq(cpl_image_get(h[i]->image, 1, 1, &d), cnt + 1);
				cnt++;
			}
			else {
				cpl_test_eq(cpl_image_get(h[i]->image, 1, 1, &d), 1);
			}
			cpl_image_delete(h[i]->image);
			cpl_propertylist_delete(h[i]->plist);
		}
	}

	hdrl_iter_delete(it);
}


void test_empty(cpl_frameset *frames)
{
	hdrl_iter *subiters[] = {
		hdrl_frameiter_new(
			frames, HDRL_ITER_OWNS_DATA, 2,
			(intptr_t[]){HDRL_FRAMEITER_AXIS_FRAME,	HDRL_FRAMEITER_AXIS_EXT},
			(intptr_t[]){0, 1},
			(intptr_t[]){1, 1},
			NULL),
		hdrl_frameiter_new(
			frames, HDRL_ITER_OWNS_DATA, 1,
			(intptr_t[]){HDRL_FRAMEITER_AXIS_EXT},
			(intptr_t[]){1},
			NULL,
			NULL)
	};
	cpl_test_error(CPL_ERROR_NONE);

	hdrl_iter *it = hdrl_multiiter_new(2, subiters,	HDRL_ITER_ALLOW_EMPTY);
	cpl_test_nonnull(it);

	int cnt = 0;
	for (hdrl_frameiter_data ** h = hdrl_iter_next(it); h != NULL;
			h = hdrl_iter_next(it)) {
		int d;
		cpl_test_eq(cpl_image_get_size_x(h[0]->image), 50);
		cpl_test_eq(cpl_image_get(h[0]->image, 1, 1, &d), cnt + 1);
		if (cnt < hdrl_iter_length(subiters[1])) {
			cpl_test_eq(cpl_image_get(h[1]->image, 1, 1, &d), cnt + 1);
			/* take ownership test */
			cpl_image_delete(h[1]->image);
			h[1]->image = NULL;
		}
		else {
			cpl_test_null(h[1]);
		}
		cnt++;
	}

	cpl_test_eq(hdrl_iter_length(subiters[0]), cnt);

	hdrl_iter_delete(it);
}

/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of hdrl_multiiter
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    cpl_frameset *frames = create_frames();

    test_invalid(frames);
    cpl_test_error(CPL_ERROR_NONE);

    test_basic(  frames);
    cpl_test_error(CPL_ERROR_NONE);

    test_empty(  frames);
    cpl_test_error(CPL_ERROR_NONE);

	  cpl_frameset_delete(frames);
    cpl_test_zero(system("rm -f hdrl_multiiter-test_*fits"));

    cpl_test_error(CPL_ERROR_NONE);

    return cpl_test_end(0);
}
