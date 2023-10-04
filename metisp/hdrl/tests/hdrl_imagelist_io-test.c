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

#include "hdrl_imagelist.h"
#include "hdrl_utils.h"
#include "hdrl_image.h"
#include "hdrl_test.h"


#include <cpl.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_imagelist_io-test 
        Testing of functions working on hdrl_imagelist
 */
/*----------------------------------------------------------------------------*/

void test_create(void)
{

    /* create data initial data */
    cpl_imagelist * data = cpl_imagelist_new();
    cpl_imagelist * errs = cpl_imagelist_new();
    cpl_image * img = cpl_image_new(64, 64, CPL_TYPE_DOUBLE);
    cpl_image * err = cpl_image_new(64, 64, CPL_TYPE_DOUBLE);
    cpl_image_add_scalar(img, 1.  );
    cpl_image_add_scalar(err, 0.05);

    /* create expected results (err / sqrt(nz) for mean) */
    size_t n = 5;
    for (size_t i = 0; i < n; i++) {
        cpl_imagelist_set(data, cpl_image_duplicate(img), cpl_imagelist_get_size(data));
        cpl_imagelist_set(errs, cpl_image_duplicate(err), cpl_imagelist_get_size(errs));
    }

    /* new hdrl_imagelist */
	hdrl_imagelist * hl = hdrl_imagelist_create(data, errs);
    cpl_test_eq(hdrl_imagelist_get_size(hl), n);
    cpl_test_error(CPL_ERROR_NONE);

	/* Clean up */
	cpl_image_delete(img);
	cpl_image_delete(err);
	cpl_imagelist_delete(data);
	cpl_imagelist_delete(errs);
	hdrl_imagelist_delete(hl);
}

void test_get(void)
{
    hdrl_imagelist * hl = NULL;
    cpl_test_eq(hdrl_imagelist_get_size_x(hl), -1);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_eq(hdrl_imagelist_get_size_y(hl), -1);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_eq(hdrl_imagelist_get_size(hl), -1);
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    hl = hdrl_imagelist_new();
    cpl_test_eq(hdrl_imagelist_get_size(hl), 0);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(hdrl_imagelist_get_size_x(hl), -1);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    cpl_test_eq(hdrl_imagelist_get_size_y(hl), -1);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    hdrl_imagelist_set(hl, hdrl_image_new(5, 6), 0);
    cpl_test_eq(hdrl_imagelist_get_size(hl), 1);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(hdrl_imagelist_get_size_x(hl), 5);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(hdrl_imagelist_get_size_y(hl), 6);
    cpl_test_error(CPL_ERROR_NONE);

    hdrl_imagelist_delete(hl);
}

cpl_error_code test_interface(void)
{
    hdrl_imagelist * hl = hdrl_imagelist_new();
    hdrl_imagelist * h;
    int c = 0;
    hdrl_iter * it = hdrl_imagelist_get_iter_row_slices(hl, 1, 0,
                                                      HDRL_ITER_OWNS_DATA);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    cpl_test_null(it);

    it = hdrl_imagelist_get_iter_row_slices(NULL, 1, 0, HDRL_ITER_OWNS_DATA);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_test_null(it);

    hdrl_imagelist_set(hl, hdrl_image_new(5, 5), 0);
    it = hdrl_imagelist_get_iter_row_slices(hl, 0, 0, HDRL_ITER_OWNS_DATA);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_nonnull(it);
    for (h = hdrl_iter_next(it); h != NULL; h = hdrl_iter_next(it)) {
        c++;
    }
    cpl_test_eq(c, 5);
    hdrl_iter_delete(it);

    hdrl_imagelist_set(hl, hdrl_image_new(5, 5), 0);
    it = hdrl_imagelist_get_iter_row_slices(hl, 3, 0, 0);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_nonnull(it);
    c = 0;
    for (h = hdrl_iter_next(it); h != NULL; h = hdrl_iter_next(it)) {
        c++;
        hdrl_imagelist_delete(h);
    }
    cpl_test_eq(c, 2);
    hdrl_iter_delete(it);

    hdrl_imagelist_set(hl, hdrl_image_new(5, 5), 0);
    it = hdrl_imagelist_get_iter_row_slices(hl, 3000, 0, HDRL_ITER_OWNS_DATA);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_nonnull(it);
    c = 0;
    for (h = hdrl_iter_next(it); h != NULL; h = hdrl_iter_next(it)) {
        c++;
    }
    cpl_test_eq(c, 1);
    hdrl_iter_delete(it);

    hdrl_imagelist_delete(hl);

    return cpl_error_get_code();
}


cpl_error_code test_iter(void)
{
    const cpl_size nx = 500;
    const cpl_size ny = 571;
    const cpl_size nz = 121;
    const cpl_size yslice = 23;
    hdrl_imagelist * hl = hdrl_imagelist_new();
    for (cpl_size i = 0; i < nz; i++) {
        hdrl_image * img = hdrl_image_new(nx, ny);
        hdrl_image_add_scalar(img, (hdrl_value){i, i});
        hdrl_image_reject(img, 1, 5);
        if ((i % 5) == 0) {
            hdrl_image_reject(img, 2, 5);
        }
        hdrl_imagelist_set(hl, img, i);
    }

    for (cpl_size y = 1; y < ny + 1; y++) {

        hdrl_imagelist * lview = hdrl_imagelist_row_view(hl, y, y);

        hdrl_imagelist_add_scalar(lview, (hdrl_value){y, y});
        hdrl_imagelist_sub_scalar(lview, (hdrl_value){y, y});

        int isConsistent = hdrl_imagelist_is_consistent(lview);
        cpl_msg_debug(cpl_func, "Imagelist consistent? %d", isConsistent);

        /* Show the structure, the first time */
        if(y == 1){

        	hdrl_imagelist_dump_structure(lview, stdout);

        	cpl_size llx = 1;
        	cpl_size lly = 1;
        	cpl_size urx = y;
        	cpl_size ury = y;
			hdrl_imagelist_dump_window(lview, llx, lly, urx, ury, stdout);
        }

        hdrl_imagelist_delete(lview);
    }

    cpl_size ysum = 1;
    hdrl_image * mean;
    cpl_image * contrib;
    hdrl_imagelist_collapse_mean(hl, &mean, &contrib);
    hdrl_image * slicemean = hdrl_image_new(nx, ny);
    cpl_image * slicecontrib = cpl_image_new(nx, ny, CPL_TYPE_INT);

    hdrl_iter * it = hdrl_imagelist_get_iter_row_slices(hl, yslice, 0,
                                                          HDRL_ITER_OWNS_DATA);
    for (hdrl_imagelist * h = hdrl_iter_next(it); h != NULL;
         h = hdrl_iter_next(it)) {
        cpl_test_eq(hdrl_imagelist_get_size(h), nz);
        hdrl_image * img = hdrl_imagelist_get(h, 0);
        cpl_test_eq(hdrl_image_get_size_x(img), nx);
        cpl_test_leq(hdrl_image_get_size_y(img), yslice);
        hdrl_image * m;
        cpl_image * c;
        hdrl_imagelist_collapse_mean(h, &m, &c);
        hdrl_image_copy(slicemean, m, 1, ysum);
        cpl_image_copy(slicecontrib, c, 1, ysum);
        hdrl_image_delete(m);
        cpl_image_delete(c);
        ysum += hdrl_image_get_size_y(img);
    }

    cpl_test_eq(ysum - 1, ny);
    hdrl_test_image_abs(slicemean, mean, nx * ny * DBL_EPSILON);
    cpl_test_image_abs(slicecontrib, contrib, 0);
    hdrl_iter_delete(it);

    /* test overlap iterator */
    ysum = 1;
    cpl_size overlap = 5;
    cpl_image_multiply_scalar(slicecontrib, 0);
    hdrl_image_mul_scalar(slicemean, (hdrl_value){0., 0.});
    it = hdrl_imagelist_get_iter_row_slices(hl, yslice, overlap,
                                            HDRL_ITER_OWNS_DATA);
    for (hdrl_imagelist * h = hdrl_iter_next(it); h != NULL;
         h = hdrl_iter_next(it)) {
        cpl_test_eq(hdrl_imagelist_get_size(h), nz);
        hdrl_image * img = hdrl_imagelist_get(h, 0);
        cpl_test_eq(hdrl_image_get_size_x(img), nx);

        if (ysum == 1) {
            cpl_test_leq(hdrl_image_get_size_y(img), yslice + overlap);
        } else {
            cpl_test_leq(hdrl_image_get_size_y(img), yslice + overlap * 2);
        }

        hdrl_image * m;
        cpl_image * c;
        hdrl_imagelist_collapse_mean(h, &m, &c);
        hdrl_il_rowsliceiter_data pdata =
            hdrl_imagelist_iter_row_slices_get_data(it);

        hdrl_image * rm = hdrl_image_extract(m, 1, pdata.ly, nx, pdata.uy);
        cpl_image * rc = cpl_image_extract(c, 1, pdata.ly, nx, pdata.uy);
        hdrl_image_delete(m);

        hdrl_image_copy(slicemean, rm, 1, ysum);
        cpl_image_copy(slicecontrib, rc, 1, ysum);
        ysum += hdrl_image_get_size_y(rm);
        hdrl_image_delete(rm);
        cpl_image_delete(rc);
        cpl_image_delete(c);
    }

    cpl_test_eq(ysum - 1, ny);
    hdrl_test_image_abs(slicemean, mean, nx * ny * DBL_EPSILON);
    cpl_test_image_abs(slicecontrib, contrib, 0);

    hdrl_iter_delete(it);
    hdrl_imagelist_delete(hl);
    hdrl_image_delete(slicemean);
    cpl_image_delete(slicecontrib);
    hdrl_image_delete(mean);
    cpl_image_delete(contrib);

    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of hdrl_image module
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    test_create();
    test_get();
    test_interface();
    test_iter();

    return cpl_test_end(0);
}
