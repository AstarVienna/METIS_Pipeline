/* $Id: hdrl_imagelist_view-test.c,v 1.3 2013-10-22 08:26:11 jtaylor Exp $
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
 * $Date: 2013-10-22 08:26:11 $
 * $Revision: 1.3 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                    Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_imagelist.h"
#include "hdrl_test.h"
#include "hdrl_imagelist_view.h" /* TODO put in public api? */

#include <cpl.h>

#define IMG_PTR(a) \
    cpl_image_get_data(hdrl_image_get_image(a))
#define ERR_PTR(a) \
    cpl_image_get_data(hdrl_image_get_image(a))
#define MSK_PTR(a) \
    cpl_mask_get_data(hdrl_image_get_mask(a))
#define CIMG_PTR(a) \
    cpl_image_get_data_const(hdrl_image_get_image_const(a))
#define CERR_PTR(a) \
    cpl_image_get_data_const(hdrl_image_get_image_const(a))
#define CMSK_PTR(a) \
    cpl_mask_get_data_const(hdrl_image_get_mask_const(a))

/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_imagelist_view_test   
            Testing of hdrl_imagelist_view module
 */
/*----------------------------------------------------------------------------*/

#define YSIZE 73
#define XSIZE 50

cpl_error_code test_row_view(void)
{
    hdrl_imagelist  *   hlist;
    hdrl_image      *   himg;
    hdrl_image      *   himg2;
    cpl_image       *   contrib;
    cpl_image       *   contrib2;

    cpl_size            nimages = 20;
    hdrl_imagelist * view;
    const hdrl_imagelist * cview;
    cpl_imagelist * clist;

    /* Create an image list */
    hlist = hdrl_imagelist_new() ;
    clist = cpl_imagelist_new() ;
    for (cpl_size i = 0 ; i < nimages ; i++) {

    	cpl_image *ima     = cpl_image_new(XSIZE, YSIZE, HDRL_TYPE_DATA);
        cpl_image *ima_err = cpl_image_new(XSIZE, YSIZE, HDRL_TYPE_ERROR);

        cpl_image_add_scalar(ima_err, 1.);
        cpl_image_reject(ima, 1, 5);
        if ((i % 5) == 0) {
            cpl_image_reject(ima, 2, 5);
        }
        himg = hdrl_image_create(ima, ima_err);
        cpl_image_delete(ima_err);
        hdrl_imagelist_set(hlist, himg, i);
        cpl_imagelist_set(clist, ima, i);
    }

    view = hdrl_imagelist_row_view(NULL, 10, 20);
    cpl_test_null(view);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    view = hdrl_imagelist_row_view(hlist, 20, 10);
    cpl_test_null(view);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    view = hdrl_imagelist_row_view(hlist, 0, 10);
    cpl_test_null(view);
    cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);
    view = hdrl_imagelist_row_view(hlist, 1, YSIZE + 11);
    cpl_test_null(view);
    cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);

    /* const variant */
    cview = hdrl_imagelist_const_row_view(NULL, 10, 20);
    cpl_test_null(cview);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cview = hdrl_imagelist_const_row_view(hlist, 20, 10);
    cpl_test_null(cview);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    cview = hdrl_imagelist_const_row_view(hlist, 0, 10);
    cpl_test_null(cview);
    cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);
    cview = hdrl_imagelist_const_row_view(hlist, 1, YSIZE + 11);
    cpl_test_null(cview);
    cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);
   
    // Mean collapse
    hdrl_imagelist_collapse_mean(hlist, &himg, &contrib);

    view = hdrl_imagelist_row_view(hlist, 1, YSIZE);
    hdrl_imagelist_collapse_mean(view, &himg2, &contrib2);

    hdrl_test_image_abs(himg, himg2, 0);
    cpl_test_image_abs(contrib, contrib2, 0);
    cpl_image * ccontrib = cpl_image_new_from_accepted(clist);
    cpl_test_image_abs(ccontrib, contrib2, 0);
    cpl_image * cerror = cpl_image_cast(ccontrib, HDRL_TYPE_ERROR);
    cpl_image_power(cerror, -0.5);
    cpl_test_image_abs(hdrl_image_get_error(himg), cerror, HDRL_EPS_ERROR);
    cpl_image_delete(contrib2);
    hdrl_image_delete(himg2);
    cpl_image_delete(cerror);

    /* const variant */
    cview = hdrl_imagelist_const_row_view(hlist, 1, YSIZE);
    hdrl_imagelist_collapse_mean(view, &himg2, &contrib2);

    hdrl_test_image_abs(himg, himg2, 0);
    cpl_test_image_abs(contrib, contrib2, 0);
    cpl_test_image_abs(ccontrib, contrib2, 0);
    cerror = cpl_image_cast(ccontrib, HDRL_TYPE_ERROR);
    cpl_image_power(cerror, -0.5);
    cpl_test_image_abs(hdrl_image_get_error(himg), cerror, HDRL_EPS_ERROR);
    hdrl_image_delete(himg);
    cpl_image_delete(contrib2);
    hdrl_image_delete(himg2);
    cpl_image_delete(ccontrib);
    cpl_image_delete(cerror);
    cpl_image_delete(contrib);

    /* one has no bpm */
    hdrl_image * iorig = hdrl_imagelist_get(hlist, 1);
    hdrl_image * iview = hdrl_imagelist_get(view, 1);
    cpl_test_eq_ptr(IMG_PTR(iorig), IMG_PTR(iview));
    cpl_test_eq_ptr(ERR_PTR(iorig), ERR_PTR(iview));
    cpl_test_eq_ptr(MSK_PTR(iorig), MSK_PTR(iview));

    hdrl_image * ciview = hdrl_imagelist_get(cview, 1);
    cpl_test_eq_ptr(IMG_PTR(iorig), CIMG_PTR(ciview));
    cpl_test_eq_ptr(ERR_PTR(iorig), CERR_PTR(ciview));
    cpl_test_eq_ptr(MSK_PTR(iorig), CMSK_PTR(ciview));

    /* zero has bpm */
    iorig = hdrl_imagelist_get(hlist, 0);
    iview = hdrl_imagelist_get(view, 0);
    cpl_test_eq_ptr(IMG_PTR(iorig), IMG_PTR(iview));
    cpl_test_eq_ptr(ERR_PTR(iorig), ERR_PTR(iview));
    cpl_test_eq_ptr(MSK_PTR(iorig), MSK_PTR(iview));

    ciview = hdrl_imagelist_get(cview, 0);
    cpl_test_eq_ptr(IMG_PTR(iorig), CIMG_PTR(ciview));
    cpl_test_eq_ptr(ERR_PTR(iorig), CERR_PTR(ciview));
    cpl_test_eq_ptr(MSK_PTR(iorig), CMSK_PTR(ciview));

    hdrl_image * icopy = hdrl_image_duplicate(iview);
    cpl_test_eq(hdrl_image_get_size_y(icopy), YSIZE);
    cpl_test_noneq_ptr(IMG_PTR(icopy), IMG_PTR(iview));
    cpl_test_noneq_ptr(ERR_PTR(icopy), ERR_PTR(iview));
    cpl_test_noneq_ptr(MSK_PTR(icopy), MSK_PTR(iview));
    hdrl_image_delete(icopy);

    hdrl_imagelist * lcopy = hdrl_imagelist_duplicate(view);
    hdrl_imagelist_delete(view);
    cpl_test_eq(hdrl_imagelist_get_size(lcopy), nimages);
    cpl_test_eq(hdrl_image_get_size_y(hdrl_imagelist_get(lcopy, 0)), YSIZE);
    hdrl_imagelist_delete(lcopy);

    cpl_size nsizey = YSIZE - 17 - 5 + 1;
    view = hdrl_imagelist_row_view(hlist, 5, YSIZE - 17);
    hdrl_image * vimg = hdrl_imagelist_get(view, 0);
    hdrl_image_add_scalar(vimg, (hdrl_value){5, 1});

    cpl_test_eq(hdrl_image_get_size_x(vimg), XSIZE);
    cpl_test_eq(hdrl_image_get_size_y(vimg), nsizey);

    hdrl_image * orig = hdrl_imagelist_get(hlist, 0);
    /* mean of original image (mix of 5s and zeros) */
    cpl_test_noneq(hdrl_image_get_mean(orig).data, 5);

    vimg = hdrl_imagelist_get(view, 0);
    cpl_test_rel(hdrl_image_get_mean(vimg).data, 5,
                 HDRL_EPS_DATA * XSIZE * YSIZE);

    hdrl_imagelist_delete(view);

    CPL_DIAG_PRAGMA_PUSH_IGN(-Wcast-qual);
    hdrl_imagelist_delete((hdrl_imagelist*)cview);
    CPL_DIAG_PRAGMA_POP;

    hdrl_imagelist_delete(hlist);
    cpl_imagelist_delete(clist);

    return cpl_error_get_code();
}

cpl_error_code test_cpl_row_view_invalid(void)
{
    cpl_imagelist * imglist = cpl_imagelist_new();
    cpl_imagelist * errlist = cpl_imagelist_new();
    const hdrl_imagelist * view;

    view = hdrl_imagelist_const_cpl_row_view(NULL, errlist, 1, 2);
    cpl_test_null(view);
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    view = hdrl_imagelist_const_cpl_row_view(imglist, errlist, 1, 0);
    cpl_test_null(view);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    view = hdrl_imagelist_const_cpl_row_view(imglist, errlist, 1, 2);
    cpl_test_null(view);
    cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);

    cpl_image * img = cpl_image_new(5, 5, HDRL_TYPE_DATA);
    cpl_imagelist_set(imglist, img, 0);
    cpl_image * err = cpl_image_new(6, 5, HDRL_TYPE_ERROR);
    cpl_imagelist_set(errlist, err, 0);

    view = hdrl_imagelist_const_cpl_row_view(imglist, errlist, 1, 2);
    cpl_test_null(view);
    cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

    cpl_image_delete(cpl_imagelist_unset(imglist, 0));
    cpl_image_delete(cpl_imagelist_unset(errlist, 0));

    img = cpl_image_new(5, 5, CPL_TYPE_INT);
    cpl_imagelist_set(imglist, img, 0);
    err = cpl_image_new(5, 5, CPL_TYPE_INT);
    cpl_imagelist_set(errlist, err, 0);

    view = hdrl_imagelist_const_cpl_row_view(imglist, errlist, 1, 2);
    cpl_test_null(view);
    cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

    cpl_image_delete(cpl_imagelist_unset(imglist, 0));
    cpl_image_delete(cpl_imagelist_unset(errlist, 0));

    img = cpl_image_new(5, 5, HDRL_TYPE_DATA);
    cpl_imagelist_set(imglist, img, 0);
    err = cpl_image_new(5, 5, HDRL_TYPE_ERROR);
    cpl_imagelist_set(errlist, err, 0);
    cpl_image_reject(img, 1, 1);

    view = hdrl_imagelist_const_cpl_row_view(imglist, errlist, 1, 2);
    cpl_test_null(view);
    cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

    cpl_image_reject(img, 1, 1);
    cpl_image_reject(err, 1, 2);

    view = hdrl_imagelist_const_cpl_row_view(imglist, errlist, 1, 2);
    cpl_test_null(view);
    cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

    cpl_image_accept_all(img);

    view = hdrl_imagelist_const_cpl_row_view(imglist, errlist, 1, 2);
    cpl_test_null(view);
    cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);

    cpl_imagelist_delete(imglist);
    cpl_imagelist_delete(errlist);

    return cpl_error_get_code();
}

cpl_error_code test_cpl_row_view(void)
{
    hdrl_imagelist  *   hlist;
    hdrl_imagelist  *   hlist_noerr;
    hdrl_image      *   himg;
    hdrl_image      *   vhimg;
    cpl_image       *   contrib;
    cpl_image       *   vcontrib;

    cpl_size            nimages = 20;
    const hdrl_imagelist * cview;
    cpl_imagelist * cimglist, * cerrlist;

    /* Create an image list */
    hlist = hdrl_imagelist_new() ;
    hlist_noerr = hdrl_imagelist_new() ;
    cimglist = cpl_imagelist_new() ;
    cerrlist = cpl_imagelist_new() ;
    for (cpl_size i = 0 ; i < nimages ; i++) {

    	cpl_image *ima     = cpl_image_new(XSIZE, YSIZE, HDRL_TYPE_DATA);
    	cpl_image *ima_err = cpl_image_new(XSIZE, YSIZE, HDRL_TYPE_ERROR);

        cpl_image_add_scalar(ima_err, 1.);
        cpl_image_reject(ima, 1, 5);
        cpl_image_reject(ima_err, 1, 5);
        if ((i % 5) == 0) {
            cpl_image_reject(ima, 2, 5);
            cpl_image_reject(ima_err, 2, 5);
        }
        himg = hdrl_image_create(ima, ima_err);
        hdrl_imagelist_set(hlist, himg, i);
        hdrl_imagelist_set(hlist_noerr, hdrl_image_create(ima, NULL), i);
        cpl_imagelist_set(cimglist, ima, i);
        cpl_imagelist_set(cerrlist, ima_err, i);
    }

    cview = hdrl_imagelist_const_cpl_row_view(cimglist, cerrlist, 1, YSIZE);
    cpl_test_nonnull(cview);
    cpl_test_error(CPL_ERROR_NONE);
    hdrl_imagelist_collapse_weighted_mean(hlist, &himg, &contrib);
    hdrl_imagelist_collapse_weighted_mean(cview, &vhimg, &vcontrib);
    hdrl_test_image_abs(himg, vhimg, 0);
    cpl_test_image_abs(contrib, vcontrib, 0);
    hdrl_image_delete(himg);
    hdrl_image_delete(vhimg);
    cpl_image_delete(contrib);
    cpl_image_delete(vcontrib);

    hdrl_imagelist_collapse_mean(hlist, &himg, &contrib);
    hdrl_imagelist_collapse_mean(cview, &vhimg, &vcontrib);
    hdrl_test_image_abs(himg, vhimg, 0);
    cpl_test_image_abs(contrib, vcontrib, 0);
    hdrl_image_delete(himg);
    hdrl_image_delete(vhimg);
    cpl_image_delete(contrib);
    cpl_image_delete(vcontrib);
    CPL_DIAG_PRAGMA_PUSH_IGN(-Wcast-qual);
    hdrl_imagelist_delete((hdrl_imagelist*)cview);
    CPL_DIAG_PRAGMA_POP;

    /* no error test */
    cview = hdrl_imagelist_const_cpl_row_view(cimglist, NULL, 1, YSIZE);
    hdrl_imagelist_collapse_mean(hlist_noerr, &himg, &contrib);
    hdrl_imagelist_collapse_mean(cview, &vhimg, &vcontrib);
    hdrl_test_image_abs(himg, vhimg, 0);
    cpl_test_image_abs(contrib, vcontrib, 0);
    hdrl_image_delete(himg);
    hdrl_image_delete(vhimg);
    cpl_image_delete(contrib);
    cpl_image_delete(vcontrib);

    hdrl_imagelist_collapse_median(hlist_noerr, &himg, &contrib);
    hdrl_imagelist_collapse_median(cview, &vhimg, &vcontrib);
    hdrl_test_image_abs(himg, vhimg, 0);
    cpl_test_image_abs(contrib, vcontrib, 0);
    hdrl_image_delete(himg);
    hdrl_image_delete(vhimg);
    cpl_image_delete(contrib);
    cpl_image_delete(vcontrib);

    hdrl_imagelist_collapse_sigclip(hlist_noerr, 3., 3., 3, &himg, &contrib,
                                    NULL, NULL);
    hdrl_imagelist_collapse_sigclip(cview, 3., 3., 3, &vhimg, &vcontrib,
                                    NULL, NULL);
    hdrl_test_image_abs(himg, vhimg, 0);
    cpl_test_image_abs(contrib, vcontrib, 0);
    hdrl_image_delete(himg);
    hdrl_image_delete(vhimg);
    cpl_image_delete(contrib);
    cpl_image_delete(vcontrib);

    hdrl_imagelist_collapse_minmax(hlist_noerr, 3., 3., &himg, &contrib,
                                    NULL, NULL);
    hdrl_imagelist_collapse_minmax(cview, 3., 3., &vhimg, &vcontrib,
                                    NULL, NULL);
    hdrl_test_image_abs(himg, vhimg, 0);
    cpl_test_image_abs(contrib, vcontrib, 0);
    hdrl_image_delete(himg);
    hdrl_image_delete(vhimg);
    cpl_image_delete(contrib);
    cpl_image_delete(vcontrib);

    CPL_DIAG_PRAGMA_PUSH_IGN(-Wcast-qual);
    hdrl_imagelist_delete((hdrl_imagelist*)cview);
    CPL_DIAG_PRAGMA_POP;

    hdrl_imagelist_delete(hlist);
    hdrl_imagelist_delete(hlist_noerr);
    cpl_imagelist_delete(cimglist);
    cpl_imagelist_delete(cerrlist);

    return cpl_error_get_code();
}


cpl_error_code test_image_view(void)
{
    hdrl_imagelist * hl = hdrl_imagelist_new();
    hdrl_imagelist * view;

    view = hdrl_imagelist_image_view(hl, 5, 1);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    cpl_test_null(view);

    view = hdrl_imagelist_image_view(hl, 1, 1);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    cpl_test_null(view);

    view = hdrl_imagelist_image_view(hl, 0, 1);
    cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);
    cpl_test_null(view);

    hdrl_imagelist_set(hl, hdrl_image_new(5, 7), 0);

    view = hdrl_imagelist_image_view(hl, 1, 2);
    cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);
    cpl_test_null(view);

    view = hdrl_imagelist_image_view(hl, -1, 1);
    cpl_test_error(CPL_ERROR_ACCESS_OUT_OF_RANGE);
    cpl_test_null(view);

    view = hdrl_imagelist_image_view(hl, 0, 1);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_nonnull(view);

    cpl_test_eq(hdrl_imagelist_get_size(view), hdrl_imagelist_get_size(hl));

    hdrl_imagelist_add_scalar(hl, (hdrl_value){1, 1});

    hdrl_test_image_abs(hdrl_imagelist_get(view, 0),
                        hdrl_imagelist_get(hl, 0), 0);

    hdrl_image * n = hdrl_image_new(5, 7);
    hdrl_imagelist_set(hl, hdrl_image_new(5, 7), 1);

    hdrl_test_image_abs(hdrl_imagelist_get(view, 0),
                        hdrl_imagelist_get(hl, 0), 0);
    hdrl_test_image_abs(hdrl_imagelist_get(hl, 1), n, 0);
    hdrl_imagelist_delete(view);
    hdrl_image_delete(n);

    hdrl_imagelist_set(hl, hdrl_image_new(5, 7), 2);
    hdrl_imagelist_set(hl, hdrl_image_new(5, 7), 3);
    view = hdrl_imagelist_image_view(hl, 1, 2);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(hdrl_imagelist_get_size(view), 1);
    cpl_test_eq_ptr(IMG_PTR(hdrl_imagelist_get(view, 0)),
                    IMG_PTR(hdrl_imagelist_get(hl, 1)));
    cpl_test_eq_ptr(ERR_PTR(hdrl_imagelist_get(view, 0)),
                    ERR_PTR(hdrl_imagelist_get(hl, 1)));
    cpl_test_eq_ptr(MSK_PTR(hdrl_imagelist_get(view, 0)),
                    MSK_PTR(hdrl_imagelist_get(hl, 1)));
    hdrl_imagelist_delete(view);

    view = hdrl_imagelist_image_view(hl, 1, 3);
    cpl_test_eq(hdrl_imagelist_get_size(view), 2);
    cpl_test_eq_ptr(IMG_PTR(hdrl_imagelist_get(view, 0)),
                    IMG_PTR(hdrl_imagelist_get(hl, 1)));
    cpl_test_eq_ptr(ERR_PTR(hdrl_imagelist_get(view, 0)),
                    ERR_PTR(hdrl_imagelist_get(hl, 1)));
    cpl_test_eq_ptr(MSK_PTR(hdrl_imagelist_get(view, 0)),
                    MSK_PTR(hdrl_imagelist_get(hl, 1)));
    cpl_test_eq_ptr(IMG_PTR(hdrl_imagelist_get(view, 1)),
                    IMG_PTR(hdrl_imagelist_get(hl, 2)));
    cpl_test_eq_ptr(ERR_PTR(hdrl_imagelist_get(view, 1)),
                    ERR_PTR(hdrl_imagelist_get(hl, 2)));
    cpl_test_eq_ptr(MSK_PTR(hdrl_imagelist_get(view, 1)),
                    MSK_PTR(hdrl_imagelist_get(hl, 2)));

    /* test view of view */
    hdrl_imagelist * rview = hdrl_imagelist_row_view(view, 2, 7);
    cpl_test_eq(hdrl_imagelist_get_size(rview), hdrl_imagelist_get_size(view));
    hdrl_image * tmp = hdrl_imagelist_get(rview, 0);
    cpl_test_eq(hdrl_image_get_size_x(tmp), 5);
    cpl_test_eq(hdrl_image_get_size_y(tmp), 6);
    hdrl_imagelist_delete(rview);

    hdrl_imagelist_delete(hl);
    hdrl_imagelist_delete(view);

    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of hdrl_image_view
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    test_row_view();
    test_image_view();
    test_cpl_row_view_invalid();
    test_cpl_row_view();

    return cpl_test_end(0);
}
