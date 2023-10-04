/* $Id: hdrl_elemop-test.c,v 1.4 2013-09-25 12:06:52 jtaylor Exp $
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
 * $Date: 2013-09-25 12:06:52 $
 * $Revision: 1.4 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                    Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_elemop.h"
#include "hdrl_types.h"
#include <cpl.h>

#include <math.h>
#include <stdlib.h>

#ifndef ARRAY_LEN
#define ARRAY_LEN(a) sizeof((a))/sizeof((a)[0])
#endif


/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_elemop_test   Testing of the HDRL variance module
 */
/*----------------------------------------------------------------------------*/

static cpl_error_code
hdrl_test_add(void)
{
    {
        hdrl_data_t a = 0., b = 0.;
        hdrl_error_t ea = 0., eb = 0.;
        hdrl_elemop_add(&a, &ea, 1, &b, &eb, 2, NULL);
        cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    }

    {
        hdrl_data_t a = 2;
        hdrl_error_t ea = 0.5;
        const hdrl_data_t b = 2;
        const hdrl_error_t eb = 0.5;

        hdrl_elemop_add(&a, &ea, 1, &b, &eb, 1, NULL);

        cpl_test_rel(a, 4., HDRL_EPS_DATA);
        cpl_test_rel(ea, sqrt(0.5), HDRL_EPS_ERROR);
    }

    {
        hdrl_data_t a = 2;
        hdrl_error_t ea = 0.5;

        hdrl_elemop_add(&a, &ea, 1, &a, &ea, 1, NULL);

        cpl_test_rel(a, 4., HDRL_EPS_DATA);
        cpl_test_rel(ea, sqrt(1.0), HDRL_EPS_ERROR);
    }

    {
        hdrl_data_t a[] = {2, 3};
        hdrl_error_t ea[] = {0.5, 1};
        const hdrl_data_t b[] = {-2, 6};
        const hdrl_error_t eb[] = {0.5, 3};

        hdrl_elemop_add(a, ea, ARRAY_LEN(a), b, eb, ARRAY_LEN(b), NULL);

        cpl_test_rel(a[0], 0., HDRL_EPS_DATA);
        cpl_test_rel(ea[0], sqrt(0.5), HDRL_EPS_ERROR);
        cpl_test_rel(a[1], 9., HDRL_EPS_DATA);
        cpl_test_rel(ea[1], sqrt(1 * 1 + 3 * 3), HDRL_EPS_ERROR);
    }

    {
        hdrl_data_t a[] = {2, 3};
        hdrl_error_t ea[] = {0.5, 1};

        hdrl_elemop_add(a, ea, ARRAY_LEN(a), a, ea, ARRAY_LEN(a), NULL);

        cpl_test_rel(a[0], 4., HDRL_EPS_DATA);
        cpl_test_rel(ea[0], 1, HDRL_EPS_ERROR);
        cpl_test_rel(a[1], 6., HDRL_EPS_DATA);
        cpl_test_rel(ea[1], 2., HDRL_EPS_ERROR);
    }

    {
        hdrl_data_t a[] = {2, 3};
        hdrl_error_t ea[] = {0.5, 1};
        const hdrl_data_t b = -2;
        const hdrl_error_t eb = 0.5;

        hdrl_elemop_add(a, ea, ARRAY_LEN(a), &b, &eb, 1, NULL);

        cpl_test_rel(a[0], 0., HDRL_EPS_DATA);
        cpl_test_rel(ea[0], sqrt(0.5), HDRL_EPS_ERROR);
        cpl_test_rel(a[1], 1., HDRL_EPS_DATA);
        cpl_test_rel(ea[1], sqrt(1 * 1 + 0.5 * 0.5), HDRL_EPS_ERROR);
    }

    {
        hdrl_data_t a[] = {2, 3};
        hdrl_error_t ea[] = {0.5, 1};
        const hdrl_data_t b = -2;
        const hdrl_error_t eb = 0.5;
        cpl_binary mask[] = {0, 1};

        hdrl_elemop_add(a, ea, ARRAY_LEN(a), &b, &eb, 1, mask);

        cpl_test_rel(a[0], 0., HDRL_EPS_DATA);
        cpl_test_rel(ea[0], sqrt(0.5), HDRL_EPS_ERROR);
        cpl_test_rel(a[1], 3., HDRL_EPS_DATA);
        cpl_test_rel(ea[1], 1., HDRL_EPS_ERROR);
    }

    /*{
        hdrl_data_t a = 2;
        hdrl_error_t ea = 0.7;
        hdrl_data_t a2 = a;
        hdrl_error_t ea2 = ea;
        const hdrl_data_t b = 2;
        const hdrl_error_t eb = 0.7;
        const double covab = 1;

        hdrl_elemop_add(&a, &ea, &a, &ea, 1);
        hdrl_elemop_add_cov(&a2, &ea2, &b, &eb, &covab, 1);

        cpl_test_rel(a, a2, HDRL_EPS_DATA);
        cpl_test_rel(ea, ea2, HDRL_EPS_ERROR);
        cpl_test_rel(ea, 1.4, HDRL_EPS_ERROR);
    }*/

    return cpl_error_get_code();
}

static cpl_error_code
hdrl_test_sub(void)
{
    {
        hdrl_data_t a = 0., b = 0.;
        hdrl_error_t ea = 0., eb = 0.;
        hdrl_elemop_sub(&a, &ea, 1, &b, &eb, 2, NULL);
        cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    }

    {
        hdrl_data_t a = 2;
        hdrl_error_t ea = 0.5;
        const hdrl_data_t b = 2;
        const hdrl_error_t eb = 0.5;

        hdrl_elemop_sub(&a, &ea, 1, &b, &eb, 1, NULL);

        cpl_test_rel(a, 0., HDRL_EPS_DATA);
        cpl_test_rel(ea, sqrt(0.5), HDRL_EPS_ERROR);
    }

    {
        hdrl_data_t a = 2;
        hdrl_error_t ea = 0.5;

        hdrl_elemop_sub(&a, &ea, 1, &a, &ea, 1, NULL);

        cpl_test_rel(a, 0., HDRL_EPS_DATA);
        cpl_test_rel(ea, 0., HDRL_EPS_ERROR);
    }

    {
        hdrl_data_t a[] = {2, 3};
        hdrl_error_t ea[] = {0.5, 1};
        const hdrl_data_t b[] = {-2, 6};
        const hdrl_error_t eb[] = {0.5, 3};

        hdrl_elemop_sub(a, ea, ARRAY_LEN(a), b, eb, ARRAY_LEN(b), NULL);

        cpl_test_rel(a[0], 4., HDRL_EPS_DATA);
        cpl_test_rel(ea[0], sqrt(0.5), HDRL_EPS_ERROR);
        cpl_test_rel(a[1], -3., HDRL_EPS_DATA);
        cpl_test_rel(ea[1], sqrt(1 * 1 + 3 * 3), HDRL_EPS_ERROR);
    }

    {
        hdrl_data_t a[] = {2, 3};
        hdrl_error_t ea[] = {0.5, 1};

        hdrl_elemop_sub(a, ea, ARRAY_LEN(a), a, ea, ARRAY_LEN(a), NULL);

        cpl_test_rel(a[0], 0., HDRL_EPS_DATA);
        cpl_test_rel(ea[0], 0., HDRL_EPS_ERROR);
        cpl_test_rel(a[1], 0., HDRL_EPS_DATA);
        cpl_test_rel(ea[1], 0., HDRL_EPS_ERROR);
    }

    {
        hdrl_data_t a[] = {2, 3};
        hdrl_error_t ea[] = {0.5, 1};
        const hdrl_data_t b = -2;
        const hdrl_error_t eb = 0.5;

        hdrl_elemop_sub(a, ea, ARRAY_LEN(a), &b, &eb, 1, NULL);

        cpl_test_rel(a[0], 4., HDRL_EPS_DATA);
        cpl_test_rel(ea[0], sqrt(0.5), HDRL_EPS_ERROR);
        cpl_test_rel(a[1], 5., HDRL_EPS_DATA);
        cpl_test_rel(ea[1], sqrt(1 * 1 + 0.5 * 0.5), HDRL_EPS_ERROR);
    }

    {
        hdrl_data_t a[] = {2, 3};
        hdrl_error_t ea[] = {0.5, 1};
        const hdrl_data_t b = -2;
        const hdrl_error_t eb = 0.5;
        cpl_binary mask[] = {0, 1};

        hdrl_elemop_sub(a, ea, ARRAY_LEN(a), &b, &eb, 1, mask);

        cpl_test_rel(a[0], 4., HDRL_EPS_DATA);
        cpl_test_rel(ea[0], sqrt(0.5), HDRL_EPS_ERROR);
        cpl_test_rel(a[1], 3., HDRL_EPS_DATA);
        cpl_test_rel(ea[1], 1., HDRL_EPS_ERROR);
    }

    return cpl_error_get_code();
}


static cpl_error_code
hdrl_test_mul(void)
{
    {
        hdrl_data_t a = 0., b = 0.;
        hdrl_error_t ea = 0., eb = 0.;
        hdrl_elemop_mul(&a, &ea, 1, &b, &eb, 2, NULL);
        cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    }

    {
        hdrl_data_t a = 2;
        hdrl_error_t ea = 0.5;
        const hdrl_data_t b = 3;
        const hdrl_error_t eb = 0.5;

        hdrl_elemop_mul(&a, &ea, 1, &b, &eb, 1, NULL);

        cpl_test_rel(a, 6., HDRL_EPS_DATA);
        cpl_test_rel(ea, sqrt(3.25), HDRL_EPS_ERROR * 10);
    }

    {
        hdrl_data_t a = 2;
        hdrl_error_t ea = 0.5;

        hdrl_elemop_mul(&a, &ea, 1, &a, &ea, 1, NULL);

        cpl_test_rel(a, 4., HDRL_EPS_DATA);
        cpl_test_rel(ea, 2., HDRL_EPS_ERROR * 10);

        a = 1.7;
        ea = 2.8;

        hdrl_elemop_mul(&a, &ea, 1, &a, &ea, 1, NULL);

        cpl_test_rel(a, 1.7 * 1.7, HDRL_EPS_DATA);
        cpl_test_rel(ea, 9.52, HDRL_EPS_ERROR * 10);

        a = -1.7;
        ea = 2.8;

        hdrl_elemop_mul(&a, &ea, 1, &a, &ea, 1, NULL);

        cpl_test_rel(a, 1.7 * 1.7, HDRL_EPS_DATA);
        cpl_test_rel(ea, 9.52, HDRL_EPS_ERROR * 10);
    }

    {
        hdrl_data_t a[] = {2, 3};
        hdrl_error_t ea[] = {0.5, 1};
        const hdrl_data_t b[] = {-2, 6};
        const hdrl_error_t eb[] = {0.5, 3};

        hdrl_elemop_mul(a, ea, ARRAY_LEN(a), b, eb, ARRAY_LEN(b), NULL);

        cpl_test_rel(a[0], -4., HDRL_EPS_DATA);
        cpl_test_rel(ea[0], sqrt(2), HDRL_EPS_ERROR * 10);
        cpl_test_rel(a[1], 18., HDRL_EPS_DATA);
        cpl_test_rel(ea[1], 10.816653826391969, HDRL_EPS_ERROR * 10);
    }

    {
        hdrl_data_t a[] = {2, 3};
        hdrl_error_t ea[] = {0.5, 1};
        const hdrl_data_t b = -2;
        const hdrl_error_t eb = 0.5;

        hdrl_elemop_mul(a, ea, ARRAY_LEN(a), &b, &eb, 1, NULL);

        cpl_test_rel(a[0], -4., HDRL_EPS_DATA);
        cpl_test_rel(ea[0], sqrt(2.), HDRL_EPS_ERROR * 10);
        cpl_test_rel(a[1], -6., HDRL_EPS_DATA);
        cpl_test_rel(ea[1], 2.5, HDRL_EPS_ERROR * 10);
    }

    {
        hdrl_data_t a[] = {2, 3};
        hdrl_error_t ea[] = {0.5, 1};
        const hdrl_data_t b = -2;
        const hdrl_error_t eb = 0.5;
        cpl_binary mask[] = {0, 1};

        hdrl_elemop_mul(a, ea, ARRAY_LEN(a), &b, &eb, 1, mask);

        cpl_test_rel(a[0], -4., HDRL_EPS_DATA);
        cpl_test_rel(ea[0], sqrt(2.), HDRL_EPS_ERROR * 10);
        cpl_test_rel(a[1], 3., HDRL_EPS_DATA);
        cpl_test_rel(ea[1], 1., HDRL_EPS_ERROR);
    }

    /* {
        hdrl_data_t a = 2;
        hdrl_error_t ea = 0.7;
        hdrl_data_t a2 = a;
        hdrl_error_t ea2 = ea;
        const hdrl_data_t b = 2;
        const hdrl_error_t eb = 0.7;
        const double covab = 1;

        hdrl_elemop_mul(&a, &ea, &a, &ea, 1);
        hdrl_elemop_mul_cov(&a2, &ea2, &b, &eb, &covab, 1);

        cpl_test_rel(a, a2, HDRL_EPS_DATA);
        cpl_test_rel(ea, ea2, HDRL_EPS_ERROR);
        cpl_test_rel(ea, 2.8, HDRL_EPS_ERROR);
    } */

    return cpl_error_get_code();
}


static cpl_error_code
hdrl_test_div(void)
{
    {
        hdrl_data_t a = 0., b = 0.;
        hdrl_error_t ea = 0., eb = 0.;
        hdrl_elemop_div(&a, &ea, 1, &b, &eb, 2, NULL);
        cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    }

    {
        hdrl_data_t a = 2;
        hdrl_error_t ea = 0.5;
        const hdrl_data_t b = 3;
        const hdrl_error_t eb = 0.5;

        hdrl_elemop_div(&a, &ea, 1, &b, &eb, 1, NULL);

        cpl_test_rel(a, 2. / 3., HDRL_EPS_DATA);
        cpl_test_rel(ea, 0.20030840419244383, HDRL_EPS_ERROR * 10);
    }

    {
        hdrl_data_t a = 2;
        hdrl_error_t ea = 0.5;

        hdrl_elemop_div(&a, &ea, 1, &a, &ea, 1, NULL);

        cpl_test_rel(a, 1., HDRL_EPS_DATA);
        cpl_test_rel(ea, 0., HDRL_EPS_ERROR * 10);

        a = 1.7;
        ea = 2.8;

        hdrl_elemop_div(&a, &ea, 1, &a, &ea, 1, NULL);

        cpl_test_rel(a, 1., HDRL_EPS_DATA);
        cpl_test_rel(ea, 0., HDRL_EPS_ERROR * 10);
    }

    {
        hdrl_data_t a[] = {2, 3};
        hdrl_error_t ea[] = {0.5, 1};
        const hdrl_data_t b[] = {-2, 6};
        const hdrl_error_t eb[] = {0.5, 3};

        hdrl_elemop_div(a, ea, ARRAY_LEN(a), b, eb, ARRAY_LEN(b), NULL);

        cpl_test_rel(a[0], -1., HDRL_EPS_DATA);
        cpl_test_rel(ea[0], 0.35355339059327379, HDRL_EPS_ERROR * 10);
        cpl_test_rel(a[1], 3./6., HDRL_EPS_DATA);
        cpl_test_rel(ea[1], 0.3004626062886658, HDRL_EPS_ERROR * 10);
    }

    /* division by 0. */
    {
        hdrl_data_t a[] = {2, 3};
        hdrl_error_t ea[] = {0.5, 1};
        hdrl_data_t b[] = {-2, 0.};
        const hdrl_error_t eb[] = {0.5, 3};

        hdrl_elemop_div(a, ea, ARRAY_LEN(a), b, eb, ARRAY_LEN(b), NULL);

        cpl_test_rel(a[0], -1., HDRL_EPS_DATA);
        cpl_test_rel(ea[0], 0.35355339059327379, HDRL_EPS_ERROR * 10);
        cpl_test(isnan(a[1]));
        cpl_test(isnan(ea[1]));

        b[0] = 0.;
        hdrl_elemop_div(a, ea, ARRAY_LEN(a), b, eb, 1, NULL);

        cpl_test(isnan(a[0]));
        cpl_test(isnan(ea[0]));
        cpl_test(isnan(a[1]));
        cpl_test(isnan(ea[1]));
    }

    {
        hdrl_data_t a[] = {2, 3};
        hdrl_error_t ea[] = {0.5, 1};
        const hdrl_data_t b = -2;
        const hdrl_error_t eb = 0.5;

        hdrl_elemop_div(a, ea, ARRAY_LEN(a), &b, &eb, 1, NULL);

        cpl_test_rel(a[0], -1., HDRL_EPS_DATA);
        cpl_test_rel(ea[0], 0.35355339059327379, HDRL_EPS_ERROR * 10);
        cpl_test_rel(a[1], -3. / 2., HDRL_EPS_DATA);
        cpl_test_rel(ea[1], 0.625, HDRL_EPS_ERROR * 10);
    }

    {
        hdrl_data_t a[] = {2, 3};
        hdrl_error_t ea[] = {0.5, 1};
        const hdrl_data_t b = -2;
        const hdrl_error_t eb = 0.5;
        cpl_binary mask[] = {0, 1};

        hdrl_elemop_div(a, ea, ARRAY_LEN(a), &b, &eb, 1, mask);

        cpl_test_rel(a[0], -1., HDRL_EPS_DATA);
        cpl_test_rel(ea[0], 0.35355339059327379, HDRL_EPS_ERROR * 10);
        cpl_test_rel(a[1], 3., HDRL_EPS_DATA);
        cpl_test_rel(ea[1], 1., HDRL_EPS_ERROR);
    }

    return cpl_error_get_code();
}

static cpl_error_code
hdrl_test_pow(void)
{
    {
        hdrl_data_t a = 0., b = 0.;
        hdrl_error_t ea = 0., eb = 0.;
        hdrl_elemop_pow(&a, &ea, 1, &b, &eb, 2, NULL);
        cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    }

    {
        hdrl_data_t a = 1.2;
        hdrl_error_t ea = 0.5;

        hdrl_elemop_pow(&a, &ea, 1, &a, &ea, 1, NULL);

        cpl_test_rel(a, pow(1.2, 1.2), HDRL_EPS_DATA);
        cpl_test_rel(ea, 0.7357378647225408, HDRL_EPS_ERROR * 10);
    }

    /* square root test */
    {
        hdrl_data_t a = 1.2;
        hdrl_error_t ea = 0.6;
        hdrl_data_t b = 0.5;
        hdrl_error_t eb = 0.;

        hdrl_elemop_pow(&a, &ea, 1, &b, &eb, 1, NULL);

        cpl_test_rel(a, sqrt(1.2), HDRL_EPS_DATA);
        cpl_test_rel(ea, 0.27386127875258304, HDRL_EPS_ERROR * 10);

        a = -1.2;
        hdrl_elemop_pow(&a, &ea, 1, &b, &eb, 1, NULL);

        cpl_test(isnan(a));
        cpl_test(isnan(ea));

        /* negative value test */
        a = -1.2;
        ea = 0.6;
        b = 3.;
        hdrl_elemop_pow(&a, &ea, 1, &b, &eb, 1, NULL);

        cpl_test_rel(a, pow(-1.2, 3.), HDRL_EPS_DATA);
        cpl_test_rel(ea, 2.592, HDRL_EPS_ERROR * 10);
    }

    /* pow with error free power 2 exponent == repeated multiplication */
    {
        hdrl_data_t a = -1.2;
        hdrl_error_t ea = 0.5;
        hdrl_data_t b = 2.0;
        hdrl_error_t eb = 0.0;

        hdrl_elemop_pow(&a, &ea, 1, &b, &eb, 1, NULL);

        hdrl_data_t a2 = -1.2;
        hdrl_error_t ea2 = 0.5;
        hdrl_elemop_mul(&a2, &ea2, 1, &a2, &ea2, 1, NULL);

        cpl_test_rel(a, a2, HDRL_EPS_DATA);
        cpl_test_rel(ea, ea2, HDRL_EPS_ERROR * 10);

        a = -1.2;
        ea = 0.5;
        b = 4.0;
        eb = 0.0;
        hdrl_elemop_pow(&a, &ea, 1, &b, &eb, 1, NULL);
        hdrl_elemop_mul(&a2, &ea2, 1, &a2, &ea2, 1, NULL);
        cpl_test_rel(a, a2, HDRL_EPS_DATA);
        cpl_test_rel(ea, ea2, HDRL_EPS_ERROR * 10);
    }
    /* array pow with error free power 2 exponent == repeated multiplication */
    {
        hdrl_data_t a[] = {0.3, 10.};
        hdrl_error_t ea[] = {0.5, 2.};
        hdrl_data_t b[] = {2.0, 2.0};
        hdrl_error_t eb[] = {0., 0.};

        hdrl_elemop_pow(a, ea, 2, b, eb, 2, NULL);

        hdrl_data_t a2[] = {0.3, 10.};
        hdrl_error_t ea2[] = {0.5, 2.};
        hdrl_elemop_mul(a2, ea2, 2, a2, ea2, 2, NULL);

        cpl_test_rel(a[0], a2[0], HDRL_EPS_DATA);
        cpl_test_rel(ea[0], ea2[0], HDRL_EPS_ERROR * 10);
        cpl_test_rel(a[1], a2[1], HDRL_EPS_DATA);
        cpl_test_rel(ea[1], ea2[1], HDRL_EPS_ERROR * 10);
    }

    /* pow with error free -1 exponent == reciprocal */
    {
        hdrl_data_t a = 1.2;
        hdrl_error_t ea = 0.5;
        hdrl_data_t b = -1.0;
        hdrl_error_t eb = 0.0;

        hdrl_elemop_pow(&a, &ea, 1, &b, &eb, 1, NULL);

        hdrl_data_t a2 = 1.0;
        hdrl_error_t ea2 = 0.;
        hdrl_data_t b2 = 1.2;
        hdrl_error_t eb2 = 0.5;
        hdrl_elemop_div(&a2, &ea2, 1, &b2, &eb2, 1, NULL);

        cpl_test_rel(a, a2, HDRL_EPS_DATA);
        cpl_test_rel(ea, ea2, HDRL_EPS_ERROR * 10);
    }

    /* array pow with error free -1 exponent == reciprocal*/
    {
        hdrl_data_t a[] = {0.3, 10.};
        hdrl_error_t ea[] = {0.5, 2.};
        hdrl_data_t b[] = {-1.0, -1.0};
        hdrl_error_t eb[] = {0., 0.};

        hdrl_elemop_pow(a, ea, 2, b, eb, 2, NULL);

        hdrl_data_t a2[] = {1.0, 1.0};
        hdrl_error_t ea2[] = {0., 0.};
        hdrl_data_t b2[] = {0.3, 10.};
        hdrl_error_t eb2[] = {0.5, 2.};
        hdrl_elemop_div(a2, ea2, 2, b2, eb2, 2, NULL);

        cpl_test_rel(a[0], a2[0], HDRL_EPS_DATA);
        cpl_test_rel(ea[0], ea2[0], HDRL_EPS_ERROR * 10);
        cpl_test_rel(a[1], a2[1], HDRL_EPS_DATA);
        cpl_test_rel(ea[1], ea2[1], HDRL_EPS_ERROR * 10);
    }

    {
        hdrl_data_t a = 1.2;
        hdrl_error_t ea = 0.5;
        hdrl_data_t b = 2.0;
        hdrl_error_t eb = 2.0;

        hdrl_elemop_pow(&a, &ea, 1, &b, &eb, 1, NULL);

        cpl_test_rel(a, pow(1.2, b), HDRL_EPS_DATA);
        cpl_test_rel(ea, 1.3098531960320208, HDRL_EPS_ERROR * 10);
    }

    {
        hdrl_data_t a = 0.;
        hdrl_error_t ea = 0.5;
        hdrl_data_t b = -1.;
        hdrl_error_t eb = 0.;

        hdrl_elemop_pow(&a, &ea, 1, &b, &eb, 1, NULL);

        cpl_test(isnan(a));
        cpl_test(isnan(ea));
    }

    {
        hdrl_data_t a[] = {0., 0.};
        hdrl_error_t ea[] = {0.5, 0.5};
        hdrl_data_t b = -1.;
        hdrl_error_t eb = 0.;

        hdrl_elemop_pow(a, ea, 2, &b, &eb, 1, NULL);

        cpl_test(isnan(a[0]));
        cpl_test(isnan(ea[0]));
        cpl_test(isnan(a[1]));
        cpl_test(isnan(ea[1]));
    }

    {
        hdrl_data_t a[] = {2, 3};
        hdrl_error_t ea[] = {0.5, 1};
        const hdrl_data_t b[] = {-2, 6};
        const hdrl_error_t eb[] = {0.5, 3};

        hdrl_elemop_pow(a, ea, ARRAY_LEN(a), b, eb, ARRAY_LEN(b), NULL);

        cpl_test_rel(a[0], 0.25, HDRL_EPS_DATA);
        cpl_test_rel(ea[0], 0.15209233492346647, HDRL_EPS_ERROR * 10);
        cpl_test_rel(a[1], 729, HDRL_EPS_DATA);
        cpl_test_rel(ea[1], 2810.438304633068, HDRL_EPS_ERROR * 10);
    }

    {
        hdrl_data_t a[] = {2, 3};
        hdrl_error_t ea[] = {0.5, 1};
        const hdrl_data_t b = -2;
        const hdrl_error_t eb = 0.5;

        hdrl_elemop_pow(a, ea, ARRAY_LEN(a), &b, &eb, 1, NULL);

        cpl_test_rel(a[0], 0.25, HDRL_EPS_DATA);
        cpl_test_rel(ea[0], 0.15209233492346647, HDRL_EPS_ERROR * 10);
        cpl_test_rel(a[1], 0.1111111111111111, HDRL_EPS_DATA);
        cpl_test_rel(ea[1], 0.09597978726560344, HDRL_EPS_ERROR * 10);
    }

    /* masked test */
    {
        hdrl_data_t a[] = {2, 3};
        hdrl_error_t ea[] = {0.5, 1};
        const hdrl_data_t b[] = {-2, 6};
        const hdrl_error_t eb[] = {0.5, 3};
        cpl_binary mask[] = {0, 1};

        hdrl_elemop_pow(a, ea, ARRAY_LEN(a), b, eb, ARRAY_LEN(b), mask);

        cpl_test_rel(a[0], 0.25, HDRL_EPS_DATA);
        cpl_test_rel(ea[0], 0.15209233492346647, HDRL_EPS_ERROR * 10);
        cpl_test_rel(a[1], 3., HDRL_EPS_DATA);
        cpl_test_rel(ea[1], 1., HDRL_EPS_ERROR * 10);
    }

    return cpl_error_get_code();
}


static cpl_error_code
hdrl_test_pow_inverted(void)
{
    {
        hdrl_data_t a = 0., b = 0.;
        hdrl_error_t ea = 0., eb = 0.;
        hdrl_elemop_pow_inverted(&a, &ea, 1, &b, &eb, 2, NULL);
        cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    }

    {
        hdrl_data_t a = 1.2;
        hdrl_error_t ea = 0.5;

        hdrl_elemop_pow_inverted(&a, &ea, 1, &a, &ea, 1, NULL);

        cpl_test_rel(a, pow(1.2, 1.2), HDRL_EPS_DATA);
        cpl_test_rel(ea, 0.7357378647225408, HDRL_EPS_ERROR * 10);
    }

    /* square root test */
    {
        hdrl_data_t a = 1.2;
        hdrl_error_t ea = 0.6;
        hdrl_data_t b = 0.5;
        hdrl_error_t eb = 0.;

        hdrl_elemop_pow_inverted(&b, &eb, 1, &a, &ea, 1, NULL);

        cpl_test_rel(b, sqrt(1.2), HDRL_EPS_DATA);
        cpl_test_rel(eb, 0.27386127875258304, HDRL_EPS_ERROR * 10);

        a = -1.2;
        hdrl_elemop_pow_inverted(&b, &eb, 1, &a, &ea, 1, NULL);

        cpl_test(isnan(b));
        cpl_test(isnan(eb));

        /* negative value test */
        a = -1.2;
        ea = 0.6;
        b = 3.;
        eb = 0.0;
        hdrl_elemop_pow_inverted(&b, &eb, 1, &a, &ea, 1, NULL);

        cpl_test_rel(b, pow(-1.2, 3.), HDRL_EPS_DATA);
        cpl_test_rel(eb, 2.592, HDRL_EPS_ERROR * 10);
    }

    /* pow with error free exper 2 exponent == repeated multiplication */
    {
        hdrl_data_t a = -1.2;
        hdrl_error_t ea = 0.5;
        hdrl_data_t b = 2.0;
        hdrl_error_t eb = 0.0;

        hdrl_elemop_pow_inverted(&b, &eb, 1, &a, &ea, 1, NULL);

        hdrl_data_t a2 = -1.2;
        hdrl_error_t ea2 = 0.5;
        hdrl_elemop_mul(&a2, &ea2, 1, &a2, &ea2, 1, NULL);

        cpl_test_rel(b, a2, HDRL_EPS_DATA);
        cpl_test_rel(eb, ea2, HDRL_EPS_ERROR * 10);

        a = -1.2;
        ea = 0.5;
        b = 4.0;
        eb = 0.0;
        hdrl_elemop_pow_inverted(&b, &eb, 1, &a, &ea, 1, NULL);
        hdrl_elemop_mul(&a2, &ea2, 1, &a2, &ea2, 1, NULL);
        cpl_test_rel(b, a2, HDRL_EPS_DATA);
        cpl_test_rel(eb, ea2, HDRL_EPS_ERROR * 10);
    }
    /* array pow with error free exper 2 exponent == repeated multiplication */
    {
        hdrl_data_t a[] = {0.3, 10.};
        hdrl_error_t ea[] = {0.5, 2.};
        hdrl_data_t b[] = {2.0, 2.0};
        hdrl_error_t eb[] = {0., 0.};

        hdrl_elemop_pow_inverted(b, eb, 2, a, ea, 2, NULL);

        hdrl_data_t a2[] = {0.3, 10.};
        hdrl_error_t ea2[] = {0.5, 2.};
        hdrl_elemop_mul(a2, ea2, 2, a2, ea2, 2, NULL);

        cpl_test_rel(b[0], a2[0], HDRL_EPS_DATA);
        cpl_test_rel(eb[0], ea2[0], HDRL_EPS_ERROR * 10);
        cpl_test_rel(b[1], a2[1], HDRL_EPS_DATA);
        cpl_test_rel(eb[1], ea2[1], HDRL_EPS_ERROR * 10);
    }

    /* pow with error free -1 exponent == reciprocal */
    {
        hdrl_data_t a = 1.2;
        hdrl_error_t ea = 0.5;
        hdrl_data_t b = -1.0;
        hdrl_error_t eb = 0.0;

        hdrl_elemop_pow_inverted(&b, &eb, 1, &a, &ea, 1, NULL);

        hdrl_data_t a2 = 1.0;
        hdrl_error_t ea2 = 0.;
        hdrl_data_t b2 = 1.2;
        hdrl_error_t eb2 = 0.5;
        hdrl_elemop_div(&a2, &ea2, 1, &b2, &eb2, 1, NULL);

        cpl_test_rel(b, a2, HDRL_EPS_DATA);
        cpl_test_rel(eb, ea2, HDRL_EPS_ERROR * 10);
    }

    /* array pow with error free -1 exponent == reciprocal*/
    {
        hdrl_data_t a[] = {0.3, 10.};
        hdrl_error_t ea[] = {0.5, 2.};
        hdrl_data_t b[] = {-1.0, -1.0};
        hdrl_error_t eb[] = {0., 0.};

        hdrl_elemop_pow_inverted(b, eb, 2, a, ea, 2, NULL);

        hdrl_data_t a2[] = {1.0, 1.0};
        hdrl_error_t ea2[] = {0., 0.};
        hdrl_data_t b2[] = {0.3, 10.};
        hdrl_error_t eb2[] = {0.5, 2.};
        hdrl_elemop_div(a2, ea2, 2, b2, eb2, 2, NULL);

        cpl_test_rel(b[0], a2[0], HDRL_EPS_DATA);
        cpl_test_rel(eb[0], ea2[0], HDRL_EPS_ERROR * 10);
        cpl_test_rel(b[1], a2[1], HDRL_EPS_DATA);
        cpl_test_rel(eb[1], ea2[1], HDRL_EPS_ERROR * 10);
    }

    {
        hdrl_data_t a = 1.2;
        hdrl_error_t ea = 0.5;
        hdrl_data_t b = 2.0;
        hdrl_error_t eb = 2.0;

        hdrl_elemop_pow_inverted(&b, &eb, 1, &a, &ea, 1, NULL);

        cpl_test_rel(b, pow(a, 2.0), HDRL_EPS_DATA);
        cpl_test_rel(eb, 1.3098531960320208, HDRL_EPS_ERROR * 10);
    }

    {
        hdrl_data_t a = 0.;
        hdrl_error_t ea = 0.5;
        hdrl_data_t b = -1.;
        hdrl_error_t eb = 0.;

        hdrl_elemop_pow_inverted(&b, &eb, 1, &a, &ea, 1, NULL);

        cpl_test(isnan(b));
        cpl_test(isnan(eb));
    }

    {
        hdrl_data_t a[] = {-1., 3.};
        hdrl_error_t ea[] = {0.0, 0.1};
        hdrl_data_t b = 0.;
        hdrl_error_t eb = 2.;

        hdrl_elemop_pow_inverted(a, ea, 2, &b, &eb, 1, NULL);

        cpl_test(isnan(a[0]));
        cpl_test(isnan(ea[0]));
        cpl_test(!isnan(a[1]));
        cpl_test_rel(a[1], 0.0, HDRL_EPS_ERROR);
        /* it is null because in the error calculation we divide by the base (0) */
        cpl_test(isnan(ea[1]));
    }

    {
        const hdrl_data_t a[] = {2, 3};
        const hdrl_error_t ea[] = {0.5, 1};
        hdrl_data_t b[] = {-2, 6};
        hdrl_error_t eb[] = {0.5, 3};

        hdrl_elemop_pow_inverted(b, eb, ARRAY_LEN(a), a, ea, ARRAY_LEN(b), NULL);

        cpl_test_rel(b[0], 0.25, HDRL_EPS_DATA);
        cpl_test_rel(eb[0], 0.15209233492346647, HDRL_EPS_ERROR * 10);
        cpl_test_rel(b[1], 729, HDRL_EPS_DATA);
        cpl_test_rel(eb[1], 2810.438304633068, HDRL_EPS_ERROR * 10);
    }

    {
        hdrl_data_t a[] = {-2, 3};
        hdrl_error_t ea[] = {0.5, .2};
        const hdrl_data_t b = 2;
        const hdrl_error_t eb = 0.5;

        hdrl_elemop_pow_inverted(a, ea, ARRAY_LEN(a), &b, &eb, 1, NULL);

        cpl_test_rel(a[0], 0.25, HDRL_EPS_DATA);
        cpl_test_rel(ea[0], 0.15209233492346647, HDRL_EPS_ERROR * 10);
        cpl_test_rel(a[1], 8., HDRL_EPS_DATA);
        cpl_test_rel(ea[1], 6.10163582292737, HDRL_EPS_ERROR * 10);
    }

    /* masked test */
    {
        const hdrl_data_t a[] = {2, 3};
        const hdrl_error_t ea[] = {0.5, 1};
        hdrl_data_t b[] = {-2, 6};
        hdrl_error_t eb[] = {0.5, 3};
        cpl_binary mask[] = {0, 1};

        hdrl_elemop_pow_inverted(b, eb, ARRAY_LEN(b), a, ea, ARRAY_LEN(a), mask);

        cpl_test_rel(b[0], 0.25, HDRL_EPS_DATA);
        cpl_test_rel(eb[0], 0.15209233492346647, HDRL_EPS_ERROR * 10);
        cpl_test_rel(b[1], 6., HDRL_EPS_DATA);
        cpl_test_rel(eb[1], 3., HDRL_EPS_ERROR * 10);
    }

    return cpl_error_get_code();
}

static cpl_error_code
hdrl_test_image(void)
{
    int is_rej;

    hdrl_data_t  ad[] = {2,   2};
    hdrl_error_t ae[] = {0.5, 0.5};
    cpl_image    *iad = cpl_image_wrap(1, 2, HDRL_TYPE_DATA,  ad);
    cpl_image    *iae = cpl_image_wrap(1, 2, HDRL_TYPE_ERROR, ae);

    hdrl_data_t  bd[] = {3,   3};
    hdrl_error_t be[] = {0.5, 0.5};
    cpl_image    *ibd = cpl_image_wrap(1, 2, HDRL_TYPE_DATA,  bd);
    cpl_image    *ibe = cpl_image_wrap(1, 2, HDRL_TYPE_ERROR, be);

    hdrl_elemop_image_add_image(iad, iae, ibd, ibe);

    cpl_test_rel(ad[0], 5., HDRL_EPS_DATA);
    cpl_test_rel(ae[0], sqrt(0.5), HDRL_EPS_ERROR);

    hdrl_elemop_image_sub_image(iad, iae, ibd, ibe);
    cpl_test_rel(ad[0], 5. - 3., HDRL_EPS_DATA);

    hdrl_elemop_image_mul_image(iad, iae, ibd, ibe);
    cpl_test_rel(ad[0], (5. - 3.) * 3., HDRL_EPS_DATA);

    hdrl_elemop_image_div_image(iad, iae, ibd, ibe);
    cpl_test_rel(ad[0], ((5. - 3.) * 3.) / 3., HDRL_EPS_DATA);

    hdrl_elemop_image_pow_image(iad, iae, ibd, ibe);
    cpl_test_rel(ad[0], pow(((5. - 3.) * 3.) / 3., 3.), HDRL_EPS_DATA);

    cpl_image_set(iad, 1, 1,  0.);
    cpl_image_set(ibd, 1, 1, -1.);
    hdrl_elemop_image_pow_image(iad, iae, ibd, ibe);
    cpl_test(cpl_image_is_rejected(iad, 1, 1));

    cpl_image_set(iad, 1, 1, 2.);
    cpl_image_set(ibd, 1, 1, 0.);
    cpl_image_accept_all(iad);
    cpl_image_accept_all(iae);
    hdrl_elemop_image_div_image(iad, iae, ibd, ibe);
    cpl_test(cpl_image_is_rejected(iad, 1, 1));

    /* test a has bpm */
    cpl_image_set(iad, 1, 1, 2. );
    cpl_image_set(iae, 1, 1, 0.5);
    cpl_image_set(ibd, 1, 1, 3. );
    cpl_image_reject(iad, 1, 1);
    hdrl_elemop_image_div_image(iad, iae, ibd, ibe);
    cpl_test(cpl_image_is_rejected(iad, 1, 1));
    cpl_test_rel(ad[0], 2., 0);

    cpl_image_accept_all(iad);
    cpl_test_rel(cpl_image_get(iad, 1, 1, &is_rej), 2., 0);

    /* test a and b have bpm */
    cpl_image_set(iad, 1, 1, 2. );
    cpl_image_set(iae, 1, 1, 0.5);
    cpl_image_reject(iad, 1, 1);
    cpl_image_reject(ibd, 1, 2);
    hdrl_elemop_image_div_image(iad, iae, ibd, ibe);
    cpl_test(cpl_image_is_rejected(iad, 1, 1));
    cpl_test(cpl_image_is_rejected(iad, 1, 2));
    cpl_test_rel(ad[0], 2.,  0);
    cpl_test_rel(ae[0], 0.5, 0);

    /* test b has bpm */
    cpl_image_set(iad, 1, 2, 2. );
    cpl_image_set(iae, 1, 2, 0.5);
    cpl_image_set(ibd, 1, 2, 2. );
    cpl_image_set(ibe, 1, 2, 0.5);
    cpl_image_accept_all(iad);
    cpl_image_reject(ibd, 1, 2);
    hdrl_elemop_image_div_image(iad, iae, ibd, ibe);
    cpl_test(cpl_image_is_rejected(iad, 1, 2));
    cpl_test_rel(ad[1], 2.,  0);
    cpl_test_rel(ae[1], 0.5, 0);

    cpl_image_unwrap(iad);
    cpl_image_unwrap(iae);
    cpl_image_unwrap(ibd);
    cpl_image_unwrap(ibe);

    return cpl_error_get_code();
}


static cpl_error_code
hdrl_test_image_scalar(void)
{
    int is_rej;
    hdrl_data_t ad = 2;
    hdrl_error_t ae = 0.5;
    const hdrl_data_t bd = 3;
    const hdrl_error_t be = 0.5;
    cpl_image * iad = cpl_image_wrap(1, 1, HDRL_TYPE_DATA, &ad);
    cpl_image * iae = cpl_image_wrap(1, 1, HDRL_TYPE_ERROR, &ae);

    hdrl_elemop_image_add_scalar(iad, iae, bd, be);

    cpl_test_rel(ad, 5., HDRL_EPS_DATA);
    cpl_test_rel(ae, sqrt(0.5), HDRL_EPS_ERROR);

    hdrl_elemop_image_sub_scalar(iad, iae, bd, be);
    cpl_test_rel(ad, 5. - 3., HDRL_EPS_DATA);

    hdrl_elemop_image_mul_scalar(iad, iae, bd, be);
    cpl_test_rel(ad, (5. - 3.) * 3., HDRL_EPS_DATA);

    hdrl_elemop_image_div_scalar(iad, iae, bd, be);
    cpl_test_rel(ad, ((5. - 3.) * 3.) / 3., HDRL_EPS_DATA);

    hdrl_data_t ad_old = ad;
    hdrl_elemop_image_exp_scalar(iad, iae, bd, be);
    cpl_test_rel(ad, pow(3., ((5. - 3.) * 3.) / 3.), HDRL_EPS_DATA);

    ad = ad_old;
    hdrl_elemop_image_pow_scalar(iad, iae, bd, be);
    cpl_test_rel(ad, pow(((5. - 3.) * 3.) / 3., 3.), HDRL_EPS_DATA);

    /* divide image by scalar zero (emits warning) */
    hdrl_elemop_image_div_scalar(iad, iae, 0., be);
    cpl_test(cpl_image_is_rejected(iad, 1, 1));

    cpl_image_accept_all(iad);
    cpl_image_set(iad, 1, 1, 0.);
    hdrl_elemop_image_pow_scalar(iad, iae, -1., be);
    cpl_test(cpl_image_is_rejected(iad, 1, 1));

    cpl_image_set(iad, 1, 1, -1);
    cpl_image_reject(iad, 1, 1);
    hdrl_elemop_image_exp_scalar(iad, iae, 0., be);
    cpl_test(cpl_image_is_rejected(iad, 1, 1));

    cpl_image_set(iad, 1, 1, 2.);
    cpl_image_reject(iad, 1, 1);
    hdrl_elemop_image_div_scalar(iad, iae, bd, be);
    cpl_test(cpl_image_is_rejected(iad, 1, 1));
    cpl_image_accept_all(iad);
    cpl_test_rel(cpl_image_get(iad, 1, 1, &is_rej), 2., 0);

    cpl_image_unwrap(iad);
    cpl_image_unwrap(iae);

    return cpl_error_get_code();
}


static cpl_error_code
hdrl_test_imagelist(void)
{
    int is_rej;
    hdrl_data_t ad = 2;
    hdrl_error_t ae = 0.5;
    hdrl_data_t bd = 3;
    hdrl_error_t be = 0.5;
    cpl_image * _iad = cpl_image_wrap(1, 1, HDRL_TYPE_DATA, &ad);
    cpl_image * _iae = cpl_image_wrap(1, 1, HDRL_TYPE_ERROR, &ae);
    cpl_image * _ibd = cpl_image_wrap(1, 1, HDRL_TYPE_DATA, &bd);
    cpl_image * _ibe = cpl_image_wrap(1, 1, HDRL_TYPE_ERROR, &be);
    cpl_imagelist * iad = cpl_imagelist_new();
    cpl_imagelist * iae = cpl_imagelist_new();
    cpl_imagelist * ibd = cpl_imagelist_new();
    cpl_imagelist * ibe = cpl_imagelist_new();
    cpl_imagelist_set(iad, _iad, 0);
    cpl_imagelist_set(ibd, _ibd, 0);
    cpl_imagelist_set(iae, _iae, 0);
    cpl_imagelist_set(ibe, _ibe, 0);

    hdrl_elemop_imagelist_add_imagelist(iad, iae, ibd, ibe);
    cpl_test_rel(ad, 5., HDRL_EPS_DATA);
    cpl_test_rel(ae, sqrt(0.5), HDRL_EPS_ERROR);

    hdrl_elemop_imagelist_sub_imagelist(iad, iae, ibd, ibe);
    cpl_test_rel(ad, 5. - 3., HDRL_EPS_DATA);

    hdrl_elemop_imagelist_mul_imagelist(iad, iae, ibd, ibe);
    cpl_test_rel(ad, (5. - 3.) * 3., HDRL_EPS_DATA);

    hdrl_elemop_imagelist_div_imagelist(iad, iae, ibd, ibe);
    cpl_test_rel(ad, ((5. - 3.) * 3.) / 3., HDRL_EPS_DATA);

    hdrl_elemop_imagelist_pow_imagelist(iad, iae, ibd, ibe);
    cpl_test_rel(ad, pow(((5. - 3.) * 3.) / 3., 3.), HDRL_EPS_DATA);

    cpl_image_set(_iad, 1, 1, 2.);
    cpl_image_reject(_iad, 1, 1);
    hdrl_elemop_imagelist_div_imagelist(iad, iae, ibd, ibe);
    cpl_test(cpl_image_is_rejected(_iad, 1, 1));
    cpl_image_accept_all(_iad);
    cpl_test_rel(cpl_image_get(_iad, 1, 1, &is_rej), 2., 0);
    cpl_imagelist_unwrap(iad);
    cpl_imagelist_unwrap(iae);
    cpl_imagelist_unwrap(ibd);
    cpl_imagelist_unwrap(ibe);
    cpl_image_unwrap(_iad);
    cpl_image_unwrap(_iae);
    cpl_image_unwrap(_ibd);
    cpl_image_unwrap(_ibe);

    return cpl_error_get_code();
}

static cpl_error_code
hdrl_test_imagelist_image(void)
{
    int is_rej;
    hdrl_data_t ad = 2;
    hdrl_error_t ae = 0.5;
    hdrl_data_t bd = 3;
    hdrl_error_t be = 0.5;
    cpl_image * _iad = cpl_image_wrap(1, 1, HDRL_TYPE_DATA, &ad);
    cpl_image * _iae = cpl_image_wrap(1, 1, HDRL_TYPE_ERROR, &ae);
    cpl_image * ibd = cpl_image_wrap(1, 1, HDRL_TYPE_DATA, &bd);
    cpl_image * ibe = cpl_image_wrap(1, 1, HDRL_TYPE_ERROR, &be);
    cpl_imagelist * iad = cpl_imagelist_new();
    cpl_imagelist * iae = cpl_imagelist_new();
    cpl_imagelist_set(iad, _iad, 0);
    cpl_imagelist_set(iae, _iae, 0);

    hdrl_elemop_imagelist_add_image(iad, iae, ibd, ibe);
    cpl_test_rel(ad, 5., HDRL_EPS_DATA);
    cpl_test_rel(ae, sqrt(0.5), HDRL_EPS_ERROR);

    hdrl_elemop_imagelist_sub_image(iad, iae, ibd, ibe);
    cpl_test_rel(ad, 5. - 3., HDRL_EPS_DATA);

    hdrl_elemop_imagelist_mul_image(iad, iae, ibd, ibe);
    cpl_test_rel(ad, (5. - 3.) * 3., HDRL_EPS_DATA);

    hdrl_elemop_imagelist_div_image(iad, iae, ibd, ibe);
    cpl_test_rel(ad, ((5. - 3.) * 3.) / 3., HDRL_EPS_DATA);

    hdrl_elemop_imagelist_pow_image(iad, iae, ibd, ibe);
    cpl_test_rel(ad, pow(((5. - 3.) * 3.) / 3., 3.), HDRL_EPS_DATA);

    cpl_image_set(_iad, 1, 1, 2.);
    cpl_image_reject(_iad, 1, 1);
    hdrl_elemop_imagelist_div_image(iad, iae, ibd, ibe);
    cpl_test(cpl_image_is_rejected(_iad, 1, 1));
    cpl_image_accept_all(_iad);
    cpl_test_rel(cpl_image_get(_iad, 1, 1, &is_rej), 2., 0);
    cpl_imagelist_unwrap(iad);
    cpl_imagelist_unwrap(iae);
    cpl_image_unwrap(_iad);
    cpl_image_unwrap(_iae);
    cpl_image_unwrap(ibd);
    cpl_image_unwrap(ibe);

    return cpl_error_get_code();
}


static cpl_error_code
hdrl_test_imagelist_vector(void)
{
    int is_rej;

    hdrl_data_t  ad = 2;
    hdrl_error_t ae = 0.5;

    double bd[1] = {3};
    double be[1] = {0.5};

    cpl_image * _iad = cpl_image_wrap(1, 1, HDRL_TYPE_DATA,  &ad);
    cpl_image * _iae = cpl_image_wrap(1, 1, HDRL_TYPE_ERROR, &ae);

    cpl_vector * ibd = cpl_vector_wrap(ARRAY_LEN(bd), bd);
    cpl_vector * ibe = cpl_vector_wrap(ARRAY_LEN(be), be);

    cpl_imagelist * iad = cpl_imagelist_new();
    cpl_imagelist * iae = cpl_imagelist_new();
    cpl_imagelist_set(iad, _iad, 0);
    cpl_imagelist_set(iae, _iae, 0);

    hdrl_elemop_imagelist_add_vector(iad, iae, ibd, ibe);
    cpl_test_rel(ad, 5., HDRL_EPS_DATA);
    cpl_test_rel(ae, sqrt(0.5), HDRL_EPS_ERROR);

    hdrl_elemop_imagelist_sub_vector(iad, iae, ibd, ibe);
    cpl_test_rel(ad, 5. - 3., HDRL_EPS_DATA);

    hdrl_elemop_imagelist_mul_vector(iad, iae, ibd, ibe);
    cpl_test_rel(ad, (5. - 3.) * 3., HDRL_EPS_DATA);

    hdrl_elemop_imagelist_div_vector(iad, iae, ibd, ibe);
    cpl_test_rel(ad, ((5. - 3.) * 3.) / 3., HDRL_EPS_DATA);

    hdrl_elemop_imagelist_pow_vector(iad, iae, ibd, ibe);
    cpl_test_rel(ad, pow(((5. - 3.) * 3.) / 3., 3.), HDRL_EPS_DATA);

    cpl_image_set(_iad, 1, 1, 2.);
    cpl_image_reject(_iad, 1, 1);
    hdrl_elemop_imagelist_div_vector(iad, iae, ibd, ibe);
    cpl_test(cpl_image_is_rejected(_iad, 1, 1));
    cpl_image_accept_all(_iad);
    cpl_test_rel(cpl_image_get(_iad, 1, 1, &is_rej), 2., 0);

    cpl_imagelist_unwrap(iad);
    cpl_imagelist_unwrap(iae);
    cpl_image_unwrap(_iad);
    cpl_image_unwrap(_iae);
    cpl_vector_unwrap(ibd);
    cpl_vector_unwrap(ibe);

    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of variance module
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    hdrl_test_add();
    hdrl_test_sub();
    hdrl_test_mul();
    hdrl_test_div();
    hdrl_test_pow();
    hdrl_test_pow_inverted();

    hdrl_test_image();
    hdrl_test_image_scalar();

    hdrl_test_imagelist();
    hdrl_test_imagelist_image();
    hdrl_test_imagelist_vector();

    return cpl_test_end(0);
}
