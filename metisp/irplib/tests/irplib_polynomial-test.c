/* $Id: irplib_polynomial-test.c,v 1.37 2013-01-29 08:43:33 jtaylor Exp $
 *
 * This file is part of the ESO Common Pipeline Library
 * Copyright (C) 2001-2004 European Southern Observatory
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02111-1307  USA
 */

/*
 * $Author: jtaylor $
 * $Date: 2013-01-29 08:43:33 $
 * $Revision: 1.37 $
 * $Name: not supported by cvs2svn $
 */

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <irplib_polynomial.h>
#include <math.h>
#include <float.h>
#include <stdint.h>

/*-----------------------------------------------------------------------------
                                   Defines
 -----------------------------------------------------------------------------*/

#define MAXDEGREE 14

#define irplib_polynomial_test_root_all(A, B, C, D, E)                  \
    irplib_polynomial_test_root_all_macro(A, B, C, D, E, __LINE__)

/*-----------------------------------------------------------------------------
                                   Static functions
 -----------------------------------------------------------------------------*/

static cpl_error_code irplib_polynomial_multiply_1d_factor(cpl_polynomial *,
                                                        const cpl_vector *,
                                                        cpl_size);
static void irplib_polynomial_solve_1d_all_test(void);

static void irplib_polynomial_test_root_all_macro(const cpl_vector *, cpl_size,
                                                  double, double, double,
                                                  unsigned);

/*-----------------------------------------------------------------------------
                                  Main
 -----------------------------------------------------------------------------*/
int main(void)
{
    /* Initialize CPL */
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    irplib_polynomial_solve_1d_all_test();

    return cpl_test_end(0);
}


/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief Test irplib_polynomial_solve_1d_all()
  @see irplib_polynomial_solve_1d_all()
  
 */
/*----------------------------------------------------------------------------*/
static void irplib_polynomial_solve_1d_all_test(void)
{

    cpl_polynomial * p2d   = cpl_polynomial_new(2);
    cpl_polynomial * p1d   = cpl_polynomial_new(1);
    cpl_vector     * xtrue = cpl_vector_new(2);
    const cpl_size   maxdegree = 4; /* Largest robustly handled degree */
    cpl_size         nreal = 0;
    cpl_size         i;
    cpl_error_code   code;
#if MAXDEGREE > 8
    double           stol, rtol;
#endif

    code = irplib_polynomial_solve_1d_all(NULL, xtrue, &nreal);
    cpl_test_eq_error(code, CPL_ERROR_NULL_INPUT);

    code = irplib_polynomial_solve_1d_all(p1d, NULL, &nreal);
    cpl_test_eq_error(code, CPL_ERROR_NULL_INPUT);

    code = irplib_polynomial_solve_1d_all(p1d, xtrue, NULL);
    cpl_test_eq_error(code, CPL_ERROR_NULL_INPUT);

    code = irplib_polynomial_solve_1d_all(p2d, xtrue, &nreal);
    cpl_test_eq_error(code, CPL_ERROR_INVALID_TYPE);

    code = irplib_polynomial_solve_1d_all(p1d, xtrue, &nreal);
    cpl_test_eq_error(code, CPL_ERROR_DATA_NOT_FOUND);

    /* Create a 1st degree polynomial, x = 0 */
    i = 1;
    code = cpl_polynomial_set_coeff(p1d, &i, 1.0);
    cpl_test_eq_error(code, CPL_ERROR_NONE);
    code = irplib_polynomial_solve_1d_all(p1d, xtrue, &nreal);
    cpl_test_eq_error(code, CPL_ERROR_INCOMPATIBLE_INPUT);

    cpl_polynomial_delete(p1d);
    cpl_polynomial_delete(p2d);

    for (nreal = 1; nreal <= maxdegree; nreal++) {
        /* A single, zero-valued root with multiplicity equal to degree */
        double xreal = 0.0;


        cpl_vector_set_size(xtrue, nreal);

        (void)cpl_vector_fill(xtrue, xreal);

        irplib_polynomial_test_root_all(xtrue, nreal, CPL_MATH_PI,
                                        2.0 * DBL_EPSILON, 2.0 * DBL_EPSILON);

        /* A single, non-zero integer root with multiplicity equal to degree */
        xreal = 1.0;

        (void)cpl_vector_fill(xtrue, xreal);

        irplib_polynomial_test_root_all(xtrue, nreal, 1.0,
                                        2.0 * DBL_EPSILON, 2.0 * DBL_EPSILON);

        /* degree distinct real roots - with rounding */
        for (i = 0; i < nreal; i++) {
            (void)cpl_vector_set(xtrue, i, 2.0 * (double)i - CPL_MATH_E);
        }

        irplib_polynomial_test_root_all(xtrue, nreal, CPL_MATH_PI,
                                        20.0 * DBL_EPSILON,
                                        300.0 * DBL_EPSILON);

        /* All real, one zero, one positive, rest negative, sum zero */
        for (i = 0; i < nreal-1; i++) {
            (void)cpl_vector_set(xtrue, nreal-i-2, (double)(-i));
        }
        (void)cpl_vector_set(xtrue, nreal-1, (double)(nreal-1)); /* FIXME: ? */

        irplib_polynomial_test_root_all(xtrue, nreal, CPL_MATH_PI,
                                        16.0*DBL_EPSILON, 600.0*DBL_EPSILON);

        if (nreal < 2) continue;
        /* Two complex, conjugate roots, the rest is real
           with multiplicity degree-2 */

        (void)cpl_vector_fill(xtrue, 2.0);
        (void)cpl_vector_set(xtrue, nreal-2, -1.0);
        (void)cpl_vector_set(xtrue, nreal-1, 1.0);

        irplib_polynomial_test_root_all(xtrue, nreal-2, CPL_MATH_PI,
                                        30.0*DBL_EPSILON, 25.0*DBL_EPSILON);

        if (nreal < 3) continue;
        if (nreal > 4) {
            /* Two real roots, the smaller with multiplicity degree-1 */
            (void)cpl_vector_fill(xtrue, 1.0);
            (void)cpl_vector_set(xtrue, nreal - 1 , 2.0);

            irplib_polynomial_test_root_all(xtrue, nreal, CPL_MATH_PI,
                                            DBL_EPSILON, DBL_EPSILON);
            /* Same with negative roots */
            (void)cpl_vector_fill(xtrue, -1.0);
            (void)cpl_vector_set(xtrue, 0 , -2.0);

            irplib_polynomial_test_root_all(xtrue, nreal, CPL_MATH_PI,
                                            DBL_EPSILON, DBL_EPSILON);
            /* Two real roots, the larger with multiplicity degree-1 */
            (void)cpl_vector_fill(xtrue, 2.0);
            (void)cpl_vector_set(xtrue, 0, 1.0);

            irplib_polynomial_test_root_all(xtrue, nreal, CPL_MATH_PI,
                                            DBL_EPSILON, DBL_EPSILON);
        }

        if (nreal > 3) continue;

        /* Same with negative roots */
        (void)cpl_vector_fill(xtrue, -2.0 * FLT_EPSILON);
        (void)cpl_vector_set(xtrue, 0, -1.0);

        irplib_polynomial_test_root_all(xtrue, nreal, CPL_MATH_PI,
                                        DBL_EPSILON, 2.0*DBL_EPSILON);

        /* A more extreme case: Same with negative roots */
#if defined SIZE_MAX && SIZE_MAX <= 4294967295
        /* Fails on 32-bit - also w. -0.1 * FLT_EPSILON */
#else
        (void)cpl_vector_fill(xtrue, -0.2 * FLT_EPSILON);

        (void)cpl_vector_set(xtrue, 0, -1.0);

        irplib_polynomial_test_root_all(xtrue, nreal, CPL_MATH_PI,
                                        FLT_EPSILON, 3.0*DBL_EPSILON);
#endif


        if (nreal != 3) {
            /* The most extreme case: Same with negative roots */
            (void)cpl_vector_fill(xtrue, -2.0 * DBL_EPSILON);
            (void)cpl_vector_set(xtrue, 0, -1.0);

            irplib_polynomial_test_root_all(xtrue, nreal, CPL_MATH_PI,
                                            DBL_EPSILON, 2.0*DBL_EPSILON);


            (void)cpl_vector_set(xtrue, 0, -1.0);
            (void)cpl_vector_set(xtrue, 1, -2.0e-4 * FLT_EPSILON);
            (void)cpl_vector_set(xtrue, 2,  2.0e-4 * FLT_EPSILON);

            irplib_polynomial_test_root_all(xtrue, nreal, CPL_MATH_PI,
                                            FLT_EPSILON, 2.0*DBL_EPSILON);
        }

        /* Two complex conjugate roots, remaining:
           small, with multiplicity degree-2 */
        (void)cpl_vector_fill(xtrue, 2.0*DBL_EPSILON);
        (void)cpl_vector_set(xtrue, nreal - 2 , 3.0);
        (void)cpl_vector_set(xtrue, nreal - 1 , 2.0);

        irplib_polynomial_test_root_all(xtrue, nreal - 2, CPL_MATH_PI,
                                        4.0 * DBL_EPSILON, DBL_EPSILON);

        /* Two complex conjugate roots with small real part, remaining:
           with multiplicity degree-2 */
        (void)cpl_vector_fill(xtrue, 3.0);
        (void)cpl_vector_set(xtrue, nreal - 2 , -1.0);
        (void)cpl_vector_set(xtrue, nreal - 1 , 2.0);

        irplib_polynomial_test_root_all(xtrue, nreal - 2, CPL_MATH_PI,
                                        6.0*DBL_EPSILON, 220.0*DBL_EPSILON);


    }

#if MAXDEGREE > 2
    /* Cover branch fixing cancellation with one negative,
         one positive near-zero and one positive root. */
    nreal = 3;

    cpl_vector_set_size(xtrue, nreal);

    /* -2, epsilon, 1.5  */
    (void)cpl_vector_set(xtrue, 0, -2.0);
    (void)cpl_vector_set(xtrue, 1,  2.0 * DBL_EPSILON);
    (void)cpl_vector_set(xtrue, 2,  1.5);

    irplib_polynomial_test_root_all(xtrue, nreal, 1.0,
                                    4.0*DBL_EPSILON, 30.0*DBL_EPSILON);

    (void)cpl_vector_set(xtrue, 0,  1.0);
    (void)cpl_vector_set(xtrue, 1,  2.0);
    (void)cpl_vector_set(xtrue, 2,  1.0);

    irplib_polynomial_test_root_all(xtrue, nreal-2, 1.0,
                                    4.0*DBL_EPSILON, 30.0*DBL_EPSILON);

#if MAXDEGREE > 3
    nreal = 4;

    cpl_vector_set_size(xtrue, nreal);

    /* Depressed has zero as root */
    (void)cpl_vector_set(xtrue, 0, -1.0);
    (void)cpl_vector_set(xtrue, 1,  1.0);
    (void)cpl_vector_set(xtrue, 2,  2.0);
    (void)cpl_vector_set(xtrue, 3,  2.0);

    irplib_polynomial_test_root_all(xtrue, nreal, CPL_MATH_PI,
                                    2.0 * DBL_EPSILON, 2.0 * DBL_EPSILON);

    /* Depressed has zero as root, and two complex roots*/
    irplib_polynomial_test_root_all(xtrue, 2, CPL_MATH_PI,
                                    2.0 * DBL_EPSILON, 2.0 * DBL_EPSILON);


    /* Depressed is biquadratic, with 4 real roots */
    (void)cpl_vector_set(xtrue, 0, -2.0);
    (void)cpl_vector_set(xtrue, 1, -1.0);
    (void)cpl_vector_set(xtrue, 2,  1.0);
    (void)cpl_vector_set(xtrue, 3,  2.0);

    irplib_polynomial_test_root_all(xtrue, nreal, CPL_MATH_PI,
                                    2.0 * DBL_EPSILON, 2.0 * DBL_EPSILON);

    /* Depressed is biquadratic, with 2 real roots */
    (void)cpl_vector_set(xtrue, 0, -1.0);
    (void)cpl_vector_set(xtrue, 1, 1.0);
    (void)cpl_vector_set(xtrue, 2, 0.0);
    (void)cpl_vector_set(xtrue, 3, 2.0);

    irplib_polynomial_test_root_all(xtrue, 2, CPL_MATH_PI,
                                    2.0 * DBL_EPSILON, 2.0 * DBL_EPSILON);

    /* Depressed is biquadratic (the quadratic has real, negative roots),
       with 0 real roots */
    (void)cpl_vector_set(xtrue, 0, 1.0);
    (void)cpl_vector_set(xtrue, 1, 2.0);
    (void)cpl_vector_set(xtrue, 2, 1.0);
    (void)cpl_vector_set(xtrue, 3, 3.0);

    irplib_polynomial_test_root_all(xtrue, 0, CPL_MATH_PI,
                                    10.0 * DBL_EPSILON, 10.0 * DBL_EPSILON);

    /* roots: 0, 0, ai, -ai */
    (void)cpl_vector_set(xtrue, 0,  0.0);
    (void)cpl_vector_set(xtrue, 1,  0.0);
    (void)cpl_vector_set(xtrue, 2,  0.0);
    (void)cpl_vector_set(xtrue, 3,  2.0);

    irplib_polynomial_test_root_all(xtrue, 2, CPL_MATH_PI,
                                    2.0 * DBL_EPSILON, 2.0 * DBL_EPSILON);

    p1d = cpl_polynomial_new(1);

    i = 0;
    cpl_polynomial_set_coeff(p1d, &i, -5.0);
    i = 1;
    cpl_polynomial_set_coeff(p1d, &i, -1.0);
    i = 2;
    cpl_polynomial_set_coeff(p1d, &i, -2.0);
    i = 4;
    cpl_polynomial_set_coeff(p1d, &i,  1.0);

    code = irplib_polynomial_solve_1d_all(p1d, xtrue, &nreal);
    cpl_test_eq_error(code, CPL_ERROR_NONE);

    cpl_msg_info(cpl_func, "Computed roots (%" CPL_SIZE_FORMAT " real): ",
                 nreal);
    if (cpl_msg_get_level() <= CPL_MSG_INFO)
        cpl_vector_dump(xtrue, stderr);
    cpl_msg_info(cpl_func, "Residual: %g -> %g ", cpl_vector_get(xtrue, 0),
                 cpl_polynomial_eval_1d(p1d, cpl_vector_get(xtrue, 0), NULL) );
    cpl_msg_info(cpl_func, "Residual: %g -> %g ", cpl_vector_get(xtrue, 1),
                 cpl_polynomial_eval_1d(p1d, cpl_vector_get(xtrue, 1), NULL) );

    cpl_polynomial_delete(p1d);

    (void)cpl_vector_set(xtrue, 0, 0.0);
    (void)cpl_vector_set(xtrue, 1, 2.0);
    (void)cpl_vector_set(xtrue, 2, 1.0);
    (void)cpl_vector_set(xtrue, 3, 1.0);

    irplib_polynomial_test_root_all(xtrue, 0, CPL_MATH_PI,
                                    2.0 * DBL_EPSILON, 2.0 * DBL_EPSILON);

    (void)cpl_vector_set(xtrue, 0, -1.0);
    (void)cpl_vector_set(xtrue, 1, 2.0);
    (void)cpl_vector_set(xtrue, 2, 1.0);
    (void)cpl_vector_set(xtrue, 3, 3.0);

    irplib_polynomial_test_root_all(xtrue, 0, CPL_MATH_PI,
                                    3.0 * DBL_EPSILON, 3.0 * DBL_EPSILON);
#if MAXDEGREE > 4
    nreal = 5;

    cpl_vector_set_size(xtrue, nreal);

    /* Depressed has zero as root */
    (void)cpl_vector_set(xtrue, 0, -1.0);
    (void)cpl_vector_set(xtrue, 1,  1.0);
    (void)cpl_vector_set(xtrue, 2,  2.0);
    (void)cpl_vector_set(xtrue, 3,  3.0);
    (void)cpl_vector_set(xtrue, 4,  4.0);

    irplib_polynomial_test_root_all(xtrue, nreal, CPL_MATH_PI,
                                    48.0 * DBL_EPSILON, 2800.0 * DBL_EPSILON);

    irplib_polynomial_test_root_all(xtrue, nreal-2, CPL_MATH_PI,
                                    8.0 * DBL_EPSILON, 4000.0 * DBL_EPSILON);

    irplib_polynomial_test_root_all(xtrue, nreal-4, CPL_MATH_PI,
                                    4.0 * DBL_EPSILON, 600.0 * DBL_EPSILON);

    (void)cpl_vector_set(xtrue, 0, -1.0);
    (void)cpl_vector_set(xtrue, 1, 10.0);
    (void)cpl_vector_set(xtrue, 2,  1.0);
    (void)cpl_vector_set(xtrue, 3, 20.0);
    (void)cpl_vector_set(xtrue, 4,  1.0);

    irplib_polynomial_test_root_all(xtrue, 1, 1.0,
                                    DBL_EPSILON, DBL_EPSILON);

    (void)cpl_vector_set(xtrue, 0,   4.0);
    (void)cpl_vector_set(xtrue, 1, -10.0);
    (void)cpl_vector_set(xtrue, 2,   4.0);
    (void)cpl_vector_set(xtrue, 3,  10.0);
    (void)cpl_vector_set(xtrue, 4,   4.0);

    irplib_polynomial_test_root_all(xtrue, 1, 1.0,
                                    DBL_EPSILON, DBL_EPSILON);
    irplib_polynomial_test_root_all(xtrue, 1, -1.0,
                                    DBL_EPSILON, DBL_EPSILON);

#if MAXDEGREE > 5
    nreal = 6;

    cpl_vector_set_size(xtrue, nreal);

    /* Depressed has zero as root */
    (void)cpl_vector_set(xtrue, 0, -1.0);
    (void)cpl_vector_set(xtrue, 1,  1.0);
    (void)cpl_vector_set(xtrue, 2,  2.0);
    (void)cpl_vector_set(xtrue, 3,  3.0);
    (void)cpl_vector_set(xtrue, 4,  4.0);
    (void)cpl_vector_set(xtrue, 5,  5.0);

    irplib_polynomial_test_root_all(xtrue, nreal, CPL_MATH_PI,
                                    240.0 * DBL_EPSILON, 50.0e3 * DBL_EPSILON);

    irplib_polynomial_test_root_all(xtrue, nreal-2, CPL_MATH_PI,
                                    10.0 * DBL_EPSILON, 25.0e3 * DBL_EPSILON);

    irplib_polynomial_test_root_all(xtrue, nreal-4, CPL_MATH_PI,
                                    12.0 * DBL_EPSILON, 1600.0 * DBL_EPSILON);

    /* These two pairs of double roots are not handled well */
    (void)cpl_vector_set(xtrue, 0,  1.0);
    (void)cpl_vector_set(xtrue, 1,  1.0);
    (void)cpl_vector_set(xtrue, 2,  3.0);
    (void)cpl_vector_set(xtrue, 3,  3.0);
    (void)cpl_vector_set(xtrue, 4,  2.0);
    (void)cpl_vector_set(xtrue, 5,  1.0);

    irplib_polynomial_test_root_all(xtrue, nreal-2, CPL_MATH_PI,
                                    0.05, 0.02);

    /* Single pair of double roots - somewhat better */
    (void)cpl_vector_set(xtrue, 0,  1.0);
    (void)cpl_vector_set(xtrue, 1,  1.0);
    (void)cpl_vector_set(xtrue, 2,  2.0);
    (void)cpl_vector_set(xtrue, 3,  1.0);
    (void)cpl_vector_set(xtrue, 4,  3.0);
    (void)cpl_vector_set(xtrue, 5,  3.0);

    irplib_polynomial_test_root_all(xtrue, nreal-4, CPL_MATH_PI,
                                    FLT_EPSILON, 1600.0 * DBL_EPSILON);

    /* These three pairs of double roots are handled only without scaling */
    (void)cpl_vector_set(xtrue, 0,  0.0);
    (void)cpl_vector_set(xtrue, 1,  0.0);
    (void)cpl_vector_set(xtrue, 2,  1.0);
    (void)cpl_vector_set(xtrue, 3,  1.0);
    (void)cpl_vector_set(xtrue, 4,  2.0);
    (void)cpl_vector_set(xtrue, 5,  2.0);

    irplib_polynomial_test_root_all(xtrue, nreal, 1.0,
                                    DBL_EPSILON, DBL_EPSILON);

    /* These three pairs of double roots are handled only without scaling */
    (void)cpl_vector_set(xtrue, 0,  1.0);
    (void)cpl_vector_set(xtrue, 1,  1.0);
    (void)cpl_vector_set(xtrue, 2,  2.0);
    (void)cpl_vector_set(xtrue, 3,  2.0);
    (void)cpl_vector_set(xtrue, 4,  3.0);
    (void)cpl_vector_set(xtrue, 5,  3.0);

    irplib_polynomial_test_root_all(xtrue, nreal, 1.0,
                                    10.0 * FLT_EPSILON, 1500.0 * DBL_EPSILON);

    /* These three pairs of double roots are easy ... */
    (void)cpl_vector_set(xtrue, 0,  0.0);
    (void)cpl_vector_set(xtrue, 1,  0.0);
    (void)cpl_vector_set(xtrue, 2,  0.0);
    (void)cpl_vector_set(xtrue, 3,  0.0);
    (void)cpl_vector_set(xtrue, 4,  1.0);
    (void)cpl_vector_set(xtrue, 5,  1.0);

    irplib_polynomial_test_root_all(xtrue, nreal, CPL_MATH_PI,
                                    DBL_EPSILON, DBL_EPSILON);

    /* A triple-root */
    (void)cpl_vector_set(xtrue, 0, -1.0);
    (void)cpl_vector_set(xtrue, 1,  1.0);
    (void)cpl_vector_set(xtrue, 2,  1.0);
    (void)cpl_vector_set(xtrue, 3,  1.0);
    (void)cpl_vector_set(xtrue, 4,  2.0);
    (void)cpl_vector_set(xtrue, 5,  3.0);

    irplib_polynomial_test_root_all(xtrue, nreal, CPL_MATH_PI,
                                    124.0 * FLT_EPSILON, 960e4 * DBL_EPSILON);

#if MAXDEGREE > 6
    nreal = 7;

    cpl_vector_set_size(xtrue, nreal);

    /* Effectively a triple root */
    (void)cpl_vector_set(xtrue, 0,  0.0);
    (void)cpl_vector_set(xtrue, 1,  0.0);
    (void)cpl_vector_set(xtrue, 2,  0.0);
    (void)cpl_vector_set(xtrue, 3,  0.0);
    (void)cpl_vector_set(xtrue, 4,  1.0);
    (void)cpl_vector_set(xtrue, 5,  1.0);
    (void)cpl_vector_set(xtrue, 6,  1.0);

    irplib_polynomial_test_root_all(xtrue, nreal, CPL_MATH_PI,
                                    DBL_EPSILON, DBL_EPSILON);

#if MAXDEGREE > 7
    nreal = 8;

    cpl_vector_set_size(xtrue, nreal);

    (void)cpl_vector_set(xtrue, 0, -3.0);
    (void)cpl_vector_set(xtrue, 1, -2.0);
    (void)cpl_vector_set(xtrue, 2, -1.0);
    (void)cpl_vector_set(xtrue, 3,  0.0);
    (void)cpl_vector_set(xtrue, 4,  1.0);
    (void)cpl_vector_set(xtrue, 5,  2.0);
    (void)cpl_vector_set(xtrue, 6,  3.0);
    (void)cpl_vector_set(xtrue, 7,  4.0);

    irplib_polynomial_test_root_all(xtrue, nreal, CPL_MATH_PI,
                                    50.0 * DBL_EPSILON, 350e3 * DBL_EPSILON);

    (void)cpl_vector_set(xtrue, 0,  1.0);
    (void)cpl_vector_set(xtrue, 1,  2.0);
    (void)cpl_vector_set(xtrue, 2,  3.0);
    (void)cpl_vector_set(xtrue, 3,  4.0);
    (void)cpl_vector_set(xtrue, 4,  5.0);
    (void)cpl_vector_set(xtrue, 5,  6.0);
    (void)cpl_vector_set(xtrue, 6,  7.0);
    (void)cpl_vector_set(xtrue, 7,  8.0);

    irplib_polynomial_test_root_all(xtrue, nreal, CPL_MATH_PI,
                                    5e5 * DBL_EPSILON, FLT_EPSILON);

#if MAXDEGREE > 8

    nreal = 0;
    stol = DBL_EPSILON;
    rtol = DBL_EPSILON * 1000.0;

    do {
        nreal++;
        cpl_vector_set_size(xtrue, nreal);

        for (i = 0; i < nreal; i++) {
            (void)cpl_vector_set(xtrue, i, (double)i);
        }

        irplib_polynomial_test_root_all(xtrue, nreal, 1.0,
                                        stol, rtol);
        stol *= 6.0;
        rtol *= 12.0;
    } while (nreal < MAXDEGREE);

#endif
#endif
#endif
#endif
#endif
#endif
#endif

    cpl_vector_delete(xtrue);

    return;
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Multiply a polynomial by (x-v1)(x-v2)...(x-vn)
  @param    self  The 1D-polynomial to modify
  @param    roots The roots to use for the extension
  @param    nreal The number of real roots
  @return   CPL_ERROR_NONE or the relevant CPL error code.
  @note roots must be sorted (for subsequent comparison)
  
 */
/*----------------------------------------------------------------------------*/
static
cpl_error_code irplib_polynomial_multiply_1d_factor(cpl_polynomial * self,
                                                    const cpl_vector * roots,
                                                    cpl_size nreal)
{

    const cpl_size nroots = cpl_vector_get_size(roots);
    cpl_size       i, degree;
    double         prevroot = 0.0;  /* Avoid (false) uninit warning */

    cpl_ensure_code(self  != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(roots != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(cpl_polynomial_get_dimension(self) == 1,
                     CPL_ERROR_ILLEGAL_INPUT);

    cpl_ensure_code(nreal >= 0,    CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(nreal <= nroots,
                     CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code((cpl_vector_get_size(roots) - nreal) % 2 == 0,
                     CPL_ERROR_ILLEGAL_INPUT);

    i = 0;
    degree = cpl_polynomial_get_degree(self);
    cpl_ensure_code(degree > 0 || cpl_polynomial_get_coeff(self, &i) != 0.0,
                    CPL_ERROR_DATA_NOT_FOUND);

    for (i = 0; i < nreal; i++) {
        const double root = cpl_vector_get(roots, i);
        double prev = 0.0;
        cpl_size j;

        degree++;

        for (j = degree; j >= 0; j--) {
            double value = 0.0;
            double newval;

            if (j > 0) {
                const cpl_size jj = j - 1;
                newval = value = cpl_polynomial_get_coeff(self, &jj);
            } else {
                newval = 0.0;
            }

            if (j < degree) {
                newval -= root * prev;
            }

            cpl_polynomial_set_coeff(self, &j, newval);

            prev = value;

        }

        if (i > 0)
            cpl_test_leq(prevroot, root);
        prevroot = root;
    }

    /* Multiplication with the complex conjugate root
       (x-a-ib) (x-a+ib) p(x) = (x-a)^2 p(x) + b^2 p(x) */
    for (; i < nroots; i += 2) {
        const double a = cpl_vector_get(roots, i);
        const double b = cpl_vector_get(roots, i+1);
        cpl_vector * aroot = cpl_vector_new(2);
        cpl_polynomial * copy = cpl_polynomial_duplicate(self);

        cpl_vector_fill(aroot, a);

        irplib_polynomial_multiply_1d_factor(self, aroot, 2);

        cpl_test_lt(0.0, fabs(b)); /* Complex root must be complex ... */

        cpl_polynomial_multiply_scalar(copy, copy, b * b);

        cpl_polynomial_add(self, self, copy);

        cpl_vector_delete(aroot);
        cpl_polynomial_delete(copy);

    }
    cpl_test_assert(i == nroots);

    for (i = 0; i < nreal; i++) {
        const double root = cpl_vector_get(roots, i);
        double d = 0.0;
        const double resid = cpl_polynomial_eval_1d(self, root, &d);
        if (resid != 0.0) {
            cpl_msg_info(cpl_func, "Real, true root %d/%d of %d degree 1D-"
                         "polynomial at %g has non-zero residual: %g "
                         "(gradient=%g)",
                         1+(int)i, (int)nreal, (int)degree, root, resid, d);
        }
    }

    return CPL_ERROR_NONE;

}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Test the roots of a 1D-polynomial
  @param    self      The roots to use for the extension
  @param    nreal     The number of real roots in self
  @param    factor    The factor of the leading polynomial term
  @param    tolerance The acceptable absolute tolerance on each root
  @param    resitol   The acceptable absolute residual of each root
  @param    line      __LINE__
  @return   void 
  
 */
/*----------------------------------------------------------------------------*/
static void
irplib_polynomial_test_root_all_macro(const cpl_vector * self, cpl_size nreal,
                                      double factor, double tolerance,
                                      double resitol, unsigned line)
{

    const cpl_size degree = cpl_vector_get_size(self);
    cpl_polynomial * p1d = cpl_polynomial_new(1);
    cpl_vector * roots = cpl_vector_new(degree);
    cpl_size i = 0;
    cpl_size jreal;
    cpl_error_code code;

    code = cpl_polynomial_set_coeff(p1d, &i, factor);
    cpl_test_eq_error(code, CPL_ERROR_NONE);

    code = irplib_polynomial_multiply_1d_factor(p1d, self, nreal);
    cpl_test_eq_error(code, CPL_ERROR_NONE);

    code = irplib_polynomial_solve_1d_all(p1d, roots, &jreal);
    cpl_test_eq_error(code, CPL_ERROR_NONE);

    cpl_test_eq(jreal, nreal);
    if (jreal != nreal) {
        cpl_vector * jroots = (jreal == 0 && code) ? NULL :
            cpl_vector_wrap(code ? jreal : CPL_MAX(nreal, jreal),
                            cpl_vector_get_data(roots));

        cpl_msg_info(cpl_func, "1D-polynomial of degree %d:", (int)degree);
        cpl_polynomial_dump(p1d, stderr);
        cpl_msg_error(cpl_func, "True roots (%" CPL_SIZE_FORMAT
                      " real): (line=%u)", nreal, line);
        cpl_vector_dump(self, stderr);
        cpl_msg_error(cpl_func, "Computed roots (%" CPL_SIZE_FORMAT " real): ",
                      jreal);
        cpl_vector_dump(jroots, stderr);
        (void)cpl_vector_unwrap(jroots);
    } else {
        if (cpl_msg_get_level() < CPL_MSG_WARNING) {
            CPL_DIAG_PRAGMA_PUSH_IGN(-Wcast-qual)
                cpl_bivector * dump =
                cpl_bivector_wrap_vectors((cpl_vector*)self, roots);
            CPL_DIAG_PRAGMA_POP;

            cpl_msg_warning(cpl_func, "Comparing %" CPL_SIZE_FORMAT " roots (%"
                            CPL_SIZE_FORMAT " real): (line=%u)",
                            degree, nreal, line);
            cpl_bivector_dump(dump, stderr);
            cpl_bivector_unwrap_vectors(dump);
        }

        for (i = 0; i < jreal; i++) {
            const double root = cpl_vector_get(roots, i);
            const double residual = cpl_polynomial_eval_1d(p1d, root, NULL);

            cpl_test_abs(root, cpl_vector_get(self, i), tolerance);

            cpl_test_abs(residual, 0.0, resitol);

        }

        for (i = nreal; i < degree; i++) {
            const double root = cpl_vector_get(roots, i);

            cpl_test_abs(root, cpl_vector_get(self, i), tolerance);

            /* FIXME: Verify residual as well */

        }
    }

    cpl_vector_delete(roots);
    cpl_polynomial_delete(p1d);

    return;
}
