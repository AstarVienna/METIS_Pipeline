/*                                                                              *
 *   This file is part of the ESO IRPLIB package                                *
 *   Copyright (C) 2004,2005 European Southern Observatory                      *
 *                                                                              *
 *   This library is free software; you can redistribute it and/or modify       *
 *   it under the terms of the GNU General Public License as published by       *
 *   the Free Software Foundation; either version 2 of the License, or          *
 *   (at your option) any later version.                                        *
 *                                                                              *
 *   This program is distributed in the hope that it will be useful,            *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 *   GNU General Public License for more details.                               *
 *                                                                              *
 *   You should have received a copy of the GNU General Public License          *
 *   along with this program; if not, write to the Free Software                *
 *   Foundation, 51 Franklin St, Fifth Floor, Boston, MA  02111-1307  USA       *
 *                                                                              */
 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*----------------------------------------------------------------------------
                                Includes
 ----------------------------------------------------------------------------*/


#include <irplib_hist.h>

#include <math.h>
/*---------------------------------------------------------------------------*/
/*
 * @defgroup irplib_hist_test   Testing of the IRPLIB utilities
 */
/*---------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
				Defines
 ----------------------------------------------------------------------------*/

#define NBINS 100

/*----------------------------------------------------------------------------
                            Private Function prototypes
 ----------------------------------------------------------------------------*/

static void irplib_hist_tests(void);

/*---------------------------------------------------------------------------*/
/*
 * @brief   Unit tests of fit module
 */
/*---------------------------------------------------------------------------*/

int main(void)
{
    /* Initialize CPL + IRPLIB */
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    irplib_hist_tests();

    return cpl_test_end(0);
}

static void irplib_hist_tests(void)
{
    irplib_hist * hist;
    cpl_image * image;
    cpl_error_code error;
    int i, j;
    float * data;

    unsigned long max_where;

    /* 1. trial: Create a right histogram */
    hist = irplib_hist_new();
    cpl_test_nonnull(hist);
    cpl_test_error(CPL_ERROR_NONE);
    irplib_hist_delete(hist);

    /* 3. trial: Histogram for a uniform image */
    image = cpl_image_new(100, 100, CPL_TYPE_FLOAT);
    cpl_image_add_scalar(image, 202);

    hist = irplib_hist_new();

    error = irplib_hist_init(hist, NBINS, 0, 500);
    cpl_test_zero(error);
    error = irplib_hist_fill(hist, image);
    cpl_test_zero(error);

    for(i = 0; i < 40; i++) {
	cpl_test_zero(irplib_hist_get_value(hist, i));
    }

    /* The following call retrieves the value of the 42-st bin */
    /* When i = 41, 42-th is retrieved. 500 - 0 / 100 = 5; 202/5=40,xx
       it should be in the 41-th bin but it is in the next one because
       there is one before left empty for possible values out of range
       0 (hinit) < 202 (image constant)
    */

    cpl_test_eq(irplib_hist_get_value(hist, 40), 10000);
    for(i = 42; i < NBINS; i++) {
	cpl_test_zero(irplib_hist_get_value(hist, i));
    }

    irplib_hist_delete(hist);
    cpl_image_delete(image);

    /* 4. trial: Histogram for a normal image: no checking of the output */
    image = cpl_image_new(100, 100, CPL_TYPE_FLOAT);
    cpl_image_fill_noise_uniform(image, 0, 200);

    hist = irplib_hist_new();
    error = irplib_hist_fill(hist,image);
    cpl_test_zero(error);

    irplib_hist_delete(hist);
    cpl_image_delete(image);

    /* 5. trial: Histogram */
    image = cpl_image_new(100, 100, CPL_TYPE_FLOAT);
    data = cpl_image_get_data_float(image);
    for (i = 0; i < 100; i++) {
	for (j = 0; j < 100; j++) {
	    *(data + 100*i + j) = i +j;
	}
    }

    hist = irplib_hist_new();
    error = irplib_hist_fill(hist, image);
    cpl_test_eq_error(error, CPL_ERROR_NONE);

    irplib_hist_get_max(hist, &max_where);

    /* The following call retrieves the value of the 41-st bin */
    /*  cpl_test_eq(irplib_hist_get_value(hist, 40), 10000);
	for(i = 42; i < NBINS; i++) {
	cpl_test_zero(irplib_hist_get_value(hist, i));
	}*/

    /* 6. trial: all by default ( we use the same image) */

    cpl_test_eq(max_where, irplib_hist_get_nbins(hist)/2);

    irplib_hist_delete(hist);
    cpl_image_delete(image);
}
