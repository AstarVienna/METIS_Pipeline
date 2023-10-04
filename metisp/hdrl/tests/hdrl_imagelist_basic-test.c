/* $Id: hdrl_imagelist_basic-test.c,v 1.6 2013-10-02 12:49:29 yjung Exp $
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
 * $Author: yjung $
 * $Date: 2013-10-02 12:49:29 $
 * $Revision: 1.6 $
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

#include <cpl.h>

#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef ARRAY_LEN
#define ARRAY_LEN(a) sizeof((a))/sizeof((a)[0])
#endif
#define    IMAGESZ     265
#define    IMAGENB     10

/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_imagelist_basic_test   
            Testing of hdrl_imagelist_basic module
 */
/*----------------------------------------------------------------------------*/

void test_image_basic_operations(void)
{
	/* Initialize values */
	hdrl_value value    = {100.,  10.};
	hdrl_value scalar   = {1000., 100.};
	hdrl_value exponent = {2.,    1.};


	/* Initialize images */
    hdrl_image *himg1 = hdrl_image_new(IMAGESZ, IMAGESZ);
    hdrl_image *himg2 = hdrl_image_new(IMAGESZ, IMAGESZ);
    hdrl_image *himg3 = hdrl_image_new(IMAGESZ, IMAGESZ);


    /* Operations with images */
    hdrl_image_add_scalar(himg1, scalar);
    hdrl_image_sub_scalar(himg1, value);
    cpl_test_error(CPL_ERROR_NONE);


    /* Initialize imageLists and add images */
    hdrl_imagelist *himlist1 = hdrl_imagelist_new();
    hdrl_imagelist_add_image(himlist1, himg1);
    cpl_test_error(CPL_ERROR_NONE);

    hdrl_imagelist *himlist2 = hdrl_imagelist_new();
    hdrl_imagelist_add_image(himlist2, himg2);
    hdrl_imagelist_sub_image(himlist2, himg2);
    hdrl_imagelist_add_image(himlist2, himg2);
    cpl_test_error(CPL_ERROR_NONE);

    hdrl_imagelist_add_imagelist(himlist1, himlist2);
    hdrl_imagelist_sub_imagelist(himlist1, himlist2);
    hdrl_imagelist_add_imagelist(himlist1, himlist2);
    cpl_test_error(CPL_ERROR_NONE);


    /* Complex operations with imageList: Div, Mul, Pow */
    hdrl_imagelist_div_scalar(himlist1, scalar);
    hdrl_imagelist_div_image(himlist1, himg1);
    hdrl_imagelist_div_imagelist(himlist1, himlist2);
    cpl_test_error(CPL_ERROR_NONE);

    hdrl_imagelist_mul_scalar(himlist1, scalar);
    hdrl_imagelist_mul_image(himlist1, himg1);
    hdrl_imagelist_mul_imagelist(himlist1, himlist2);
    cpl_test_error(CPL_ERROR_NONE);

    hdrl_imagelist_pow_scalar(himlist1, exponent);
    cpl_test_error(CPL_ERROR_NONE);

    hdrl_imagelist_add_image(himlist2, himg3);
    hdrl_imagelist_add_imagelist(himlist1, himlist2);
    hdrl_imagelist_pow_scalar(himlist1, exponent);
    cpl_test_error(CPL_ERROR_NONE);


    /* Clean up */
    hdrl_image_delete(himg1);
    hdrl_image_delete(himg2);
    hdrl_image_delete(himg3);
    hdrl_imagelist_delete(himlist1) ;
    hdrl_imagelist_delete(himlist2) ;
}

/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of hdrl_image_basic
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    test_image_basic_operations();

    hdrl_imagelist  *   himlist ;
    hdrl_image      *   himg ;
    hdrl_image      *   himg2 ;
    cpl_image       *   contrib ;
    cpl_image       *   contrib2 ;
    cpl_size            nimages = IMAGENB ;

    /* Create an image list */
    himlist = hdrl_imagelist_new() ;
    for (cpl_size i = 0 ; i < nimages ; i++) {
        /*
        What about having an

        hdrl_image_load_image(hdrl_image * hdrlimage
                           const char * filename,
                           cpl_type     im_type,
                           cpl_size     pnum,
                           cpl_size     xtnum);

        hdrl_image_load_error(hdrl_image * hdrlimage
                           const char * filename,
                           cpl_type     im_type,
                           cpl_size     pnum,
                           cpl_size     xtnum);

        with the same API like cpl?
        two functions are better in the case the error is in another fitsfile
        */

        himg = hdrl_image_new(IMAGESZ, IMAGESZ);
        if (i == nimages / 2)
            hdrl_image_add_scalar(himg, (hdrl_value){1000, 100});
        else
            hdrl_image_add_scalar(himg, (hdrl_value){i, 1});
        hdrl_imagelist_set(himlist, himg, i);
    }
   
    /* Mean collapse */
    hdrl_imagelist_collapse_mean(himlist, &himg, &contrib);
    hdrl_imagelist_collapse(himlist, HDRL_COLLAPSE_MEAN, &himg2, &contrib2);
    hdrl_test_image_abs(himg, himg2, 0);
    cpl_test_image_abs(contrib, contrib2, 0);
    cpl_image_delete(contrib) ;
    hdrl_image_delete(himg) ;
    cpl_image_delete(contrib2) ;
    hdrl_image_delete(himg2) ;

    /* Weighted Mean collapse */
    hdrl_imagelist_collapse_weighted_mean(himlist, &himg, &contrib);
    hdrl_imagelist_collapse(himlist, HDRL_COLLAPSE_WEIGHTED_MEAN, &himg2, &contrib2);
    hdrl_test_image_abs(himg, himg2, 0);
    cpl_test_image_abs(contrib, contrib2, 0);
    cpl_image_delete(contrib) ;
    hdrl_image_delete(himg) ;
    cpl_image_delete(contrib2) ;
    hdrl_image_delete(himg2) ;

    /* Median collapse */
    hdrl_imagelist_collapse_median(himlist, &himg, &contrib);
    hdrl_imagelist_collapse(himlist, HDRL_COLLAPSE_MEDIAN, &himg2, &contrib2);
    hdrl_test_image_abs(himg, himg2, 0);
    cpl_test_image_abs(contrib, contrib2, 0);
    cpl_image_delete(contrib) ;
    hdrl_image_delete(himg) ;
    cpl_image_delete(contrib2) ;
    hdrl_image_delete(himg2) ;

    /* Sigclip collapse */
    hdrl_parameter *psc = hdrl_collapse_sigclip_parameter_create(1., 3., 10);
	hdrl_imagelist_collapse_sigclip(himlist, 1., 3., 10, &himg, &contrib, NULL, NULL);
    hdrl_imagelist_collapse(himlist, psc, &himg2, &contrib2);
    hdrl_test_image_abs(himg, himg2, 0);
    cpl_test_image_abs(contrib, contrib2, 0);
    cpl_image_delete(contrib);
    hdrl_image_delete(himg);
    cpl_image_delete(contrib2);
    hdrl_image_delete(himg2);
    hdrl_parameter_delete(psc);

    /* MinMax collapse */
    hdrl_parameter *pmm = hdrl_collapse_minmax_parameter_create(1., 3.);
    hdrl_imagelist_collapse_minmax(himlist, 1., 3., &himg, &contrib, NULL, NULL);
    hdrl_imagelist_collapse(himlist, pmm, &himg2, &contrib2);
    hdrl_test_image_abs(himg, himg2, 0);
    cpl_image_delete(contrib) ;
    hdrl_image_delete(himg) ;
    cpl_image_delete(contrib2) ;
    hdrl_image_delete(himg2) ;
    hdrl_parameter_delete(pmm);

    /* Mode collapse */
    hdrl_parameter *pmode = hdrl_collapse_mode_parameter_create(10., 1., 0., HDRL_MODE_MEDIAN, 0);
    hdrl_imagelist_collapse_mode(himlist, 10., 1., 0., HDRL_MODE_MEDIAN, 0, &himg, &contrib);
    hdrl_imagelist_collapse(himlist, pmode, &himg2, &contrib2);
    hdrl_test_image_abs(himg, himg2, 0);
    cpl_test_image_abs(contrib, contrib2, 0);
    cpl_image_delete(contrib) ;
    hdrl_image_delete(himg) ;
    cpl_image_delete(contrib2) ;
    hdrl_image_delete(himg2) ;
    hdrl_parameter_delete(pmode);


    /* Unknown collapse mode */
    typedef hdrl_parameter_empty hdrl_collapse_unknown_parameter;
    static hdrl_parameter_typeobj hdrl_collapse_unknown_parameter_type = {
        -1,                                        /* type */
        (hdrl_alloc *)&cpl_malloc,                 /* fp_alloc */
        (hdrl_free *)&cpl_free,                    /* fp_free */
        NULL,                                      /* fp_destroy */
        sizeof(hdrl_collapse_unknown_parameter),   /* obj_size */
    };
    hdrl_collapse_unknown_parameter *unk = (hdrl_collapse_unknown_parameter *)
       hdrl_parameter_new(&hdrl_collapse_unknown_parameter_type);
    hdrl_parameter *punk = (hdrl_parameter *)unk;
    hdrl_imagelist_collapse(himlist, punk, &himg2, &contrib2);
    hdrl_parameter_delete(punk);


    /* Collapse Direct */
    cpl_image *rej_low;
	cpl_image *rej_high;

    /* Sigclip */
    hdrl_imagelist_collapse_sigclip(NULL,    1.0, 3.0, 10, &himg, &contrib, &rej_low, &rej_high);
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    hdrl_imagelist_collapse_sigclip(himlist, 1.0, 3.0, 10, &himg, &contrib, NULL,     NULL     );
    cpl_test_error(CPL_ERROR_NONE);
    cpl_image_delete(contrib );
    hdrl_image_delete(himg);

    hdrl_imagelist_collapse_sigclip(himlist, 1.0, 3.0, 10, &himg, &contrib, &rej_low, NULL     );
    cpl_test_error(CPL_ERROR_NONE);
    cpl_image_delete(contrib );
    cpl_image_delete(rej_low );
    hdrl_image_delete(himg);

    hdrl_imagelist_collapse_sigclip(himlist, 1.0, 3.0, 10, &himg, &contrib, NULL,     &rej_high);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_image_delete(contrib );
    cpl_image_delete(rej_high);
    hdrl_image_delete(himg);

    hdrl_imagelist_collapse_sigclip(himlist, 1.0, 3.0, 10, &himg, &contrib, &rej_low, &rej_high);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_image_delete(contrib );
    cpl_image_delete(rej_low );
    cpl_image_delete(rej_high);
    hdrl_image_delete(himg);

    /* MinMax */
	hdrl_imagelist_collapse_minmax(NULL,    1.0, 3.0, &himg, &contrib, &rej_low, &rej_high);
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    hdrl_imagelist_collapse_minmax(himlist, 1.0, 3.0, &himg, &contrib, NULL,     NULL     );
    cpl_test_error(CPL_ERROR_NONE);
    cpl_image_delete(contrib );
    hdrl_image_delete(himg);

    hdrl_imagelist_collapse_minmax(himlist, 1.0, 3.0, &himg, &contrib, &rej_low, NULL     );
    cpl_test_error(CPL_ERROR_NONE);
    cpl_image_delete(contrib );
    cpl_image_delete(rej_low );
    hdrl_image_delete(himg);

    hdrl_imagelist_collapse_minmax(himlist, 1.0, 3.0, &himg, &contrib, NULL,     &rej_high);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_image_delete(contrib );
    cpl_image_delete(rej_high);
    hdrl_image_delete(himg);

    hdrl_imagelist_collapse_minmax(himlist, 1.0, 3.0, &himg, &contrib, &rej_low, &rej_high);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_image_delete(contrib );
    cpl_image_delete(rej_low );
    cpl_image_delete(rej_high);
    hdrl_image_delete(himg);




    

    /* Mode */
    hdrl_imagelist_collapse_mode(NULL,    10., 1., 0., HDRL_MODE_MEDIAN, 10, &himg, &contrib);
    cpl_test_error(CPL_ERROR_NULL_INPUT);

    /* auto determine histogram */
    hdrl_imagelist_collapse_mode(himlist, 10., 1., 0., HDRL_MODE_MEDIAN, 10, &himg, &contrib);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(cpl_image_count_rejected(hdrl_image_get_image(himg)), 0);
    cpl_test_eq(cpl_image_count_rejected(hdrl_image_get_error(himg)), 0);
    cpl_test_eq(cpl_image_get_sqflux(contrib), (IMAGENB * IMAGENB) * IMAGESZ * IMAGESZ);
    cpl_image_delete(contrib );
    hdrl_image_delete(himg);

    /* determine histogram by user */
    hdrl_imagelist_collapse_mode(himlist, 1., 20., 1., HDRL_MODE_MEDIAN, 10, &himg, &contrib);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(cpl_image_count_rejected(hdrl_image_get_image(himg)), 0);
    cpl_test_eq(cpl_image_count_rejected(hdrl_image_get_error(himg)), 0);
    cpl_test_eq(cpl_image_get_sqflux(contrib), (IMAGENB * IMAGENB) * IMAGESZ * IMAGESZ);
    cpl_image_delete(contrib );
    hdrl_image_delete(himg);

    /* data outside histogram -> no error but all pixels rejected */
    hdrl_imagelist_collapse_mode(himlist, -1000., -100., 0., HDRL_MODE_MEDIAN, 0, &himg, &contrib);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(cpl_image_count_rejected(hdrl_image_get_image(himg)), IMAGESZ * IMAGESZ);
    cpl_test_eq(cpl_image_count_rejected(hdrl_image_get_error(himg)), IMAGESZ * IMAGESZ);
    cpl_test_eq(cpl_image_get_sqflux(contrib), 0.);
    cpl_image_delete(contrib );
    hdrl_image_delete(himg);

    /* Clean up */
    hdrl_imagelist_delete(himlist);

    return cpl_test_end(0);
}

