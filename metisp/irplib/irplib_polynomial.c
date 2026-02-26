/* $Id: irplib_polynomial.c,v 1.35 2013-01-29 08:43:33 jtaylor Exp $
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
 * $Revision: 1.35 $
 * $Name: not supported by cvs2svn $
 */

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "irplib_polynomial.h"

/* IRPLIB_SWAP_DOUBLE: */
#include "irplib_utils.h"

#include <assert.h>
#include <math.h>
/* DBL_MAX: */
#include <float.h>

/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_polynomial 1D-Polynomial roots 
 *
 *
 */
/*----------------------------------------------------------------------------*/
/**@{*/

/*-----------------------------------------------------------------------------
                                   Static functions
 -----------------------------------------------------------------------------*/

static double irplib_polynomial_eval_2_max(double, double, double, cpl_boolean,
                                           double, double);

static double irplib_polynomial_eval_3_max(double, double, double, double,
                                           cpl_boolean, double, double, double);


static cpl_boolean irplib_polynomial_solve_1d_2(double, double, double,
                                                double *, double *);
static cpl_boolean irplib_polynomial_solve_1d_3(double, double, double, double,
                                                double *, double *, double *,
                                                cpl_boolean *,
                                                cpl_boolean *);

static void irplib_polynomial_solve_1d_31(double, double, double *, double *,
                                          double *, cpl_boolean *);

static void irplib_polynomial_solve_1d_32(double, double, double, double *,
                                          double *, double *, cpl_boolean *);

static void irplib_polynomial_solve_1d_3r(double, double, double, double,
                                          double *, double *, double *);

static void irplib_polynomial_solve_1d_3c(double, double, double,
                                          double, double, double,
                                          double *, double *, double *,
                                          cpl_boolean *, cpl_boolean *);

static cpl_error_code irplib_polynomial_solve_1d_4(double, double, double,
                                                   double, double, cpl_size *,
                                                   double *, double *,
                                                   double *, double *);

static cpl_error_code irplib_polynomial_solve_1d_zero(cpl_polynomial *,
                                                      cpl_vector *,
                                                      cpl_size *)
    CPL_ATTR_NONNULL;

static cpl_error_code irplib_polynomial_solve_1d_nonzero(cpl_polynomial *,
                                                         cpl_vector *,
                                                         cpl_size *)
    CPL_ATTR_NONNULL;

static cpl_error_code irplib_polynomial_divide_1d_root(cpl_polynomial *, double,
                                                       double *);

static cpl_error_code irplib_polynomial_solve_1d_guess(const cpl_polynomial *,
                                                       double *)
    CPL_ATTR_NONNULL;

#ifdef IRPLIB_POLYNOMIAL_GUESS_ANASOL
static double irplib_polynomial_depress_1d(cpl_polynomial *);
#endif

/*-----------------------------------------------------------------------------
                              Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute all n roots of p(x) = 0, where p(x) is of degree n, n > 0.
  @param    self  The 1D-polynomial
  @param    roots A pre-allocated vector of length n to hold the roots
  @param    preal The number of real roots found, or undefined on error
  @return   CPL_ERROR_NONE or the relevant CPL error code
  @note     There is (currently) no support for more than 4 complex roots

  The *preal real roots are stored first in ascending order, then follows for
  each pair of complex conjugate roots, the real and imaginary parts of the
  root in the positive imaginary half-plane, for example for a 3rd degree
  polynomial with 1 real root, the roots are represented as:
  x0 = v0
  x1 = v1 + i v2
  x2 = v1 - i v2,
  where v0, v1, v2 are the elements of the roots vector.

  Possible CPL error code set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
  - CPL_ERROR_INVALID_TYPE if the polynomial has the wrong dimension
  - CPL_ERROR_DATA_NOT_FOUND if the polynomial has the degree 0.
  - CPL_ERROR_INCOMPATIBLE_INPUT if the roots vector does not have length n
  - CPL_ERROR_DIVISION_BY_ZERO if a division by zero occurs (n > 4)
  - CPL_ERROR_CONTINUE if the algorithm does not converge (n > 4)
 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_polynomial_solve_1d_all(const cpl_polynomial * self,
                                              cpl_vector * roots,
                                              cpl_size * preal)
{

    cpl_error_code error;
    const cpl_size degree = cpl_polynomial_get_degree(self);
    cpl_polynomial * p;

    cpl_ensure_code(self  != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(roots != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(preal != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(cpl_polynomial_get_dimension(self) == 1,
                    CPL_ERROR_INVALID_TYPE);
    cpl_ensure_code(degree > 0,    CPL_ERROR_DATA_NOT_FOUND);
    cpl_ensure_code(degree == cpl_vector_get_size(roots),
                    CPL_ERROR_INCOMPATIBLE_INPUT);

    *preal = 0;

    p = cpl_polynomial_duplicate(self);

    error = irplib_polynomial_solve_1d_zero(p, roots, preal);

    if (!error && *preal < degree) {
        /* There are non-zero roots */

        /* Whether roots need sorting (no need w. up to 4 non-zero roots) */
        const cpl_boolean dosort = *preal > 0 || degree - *preal > 4;

        assert(cpl_polynomial_get_degree(p) + *preal == degree);

        error = irplib_polynomial_solve_1d_nonzero(p, roots, preal);

        if (!error && dosort) {
            cpl_vector * reals = cpl_vector_wrap(*preal,
                                                 cpl_vector_get_data(roots));
            cpl_vector_sort(reals, CPL_SORT_ASCENDING);
            (void)cpl_vector_unwrap(reals);
        }
    }

    cpl_polynomial_delete(p);

    return error ? cpl_error_set_where(cpl_func) : CPL_ERROR_NONE;

}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Divide out all zero-valued roots from the 1D-polynomial
  @param    self  The 1D-polynomial
  @param    roots A pre-allocated vector of length n to hold the roots
  @param    preal The number of real roots found so far or undefined on error
  @return   CPL_ERROR_NONE or the relevant CPL error code
  @see      irplib_polynomial_solve_1d_all()
  @note The caller may pass a roots vector which already has *preal (other)
        roots which are only accessed when all roots are sorted at the end

*/
/*----------------------------------------------------------------------------*/
static cpl_error_code irplib_polynomial_solve_1d_zero(cpl_polynomial * self,
                                                      cpl_vector * roots,
                                                      cpl_size * preal)
{
    cpl_size nzero;
    const cpl_size degree = cpl_polynomial_get_degree(self);

    /* Count number of zero-value roots */
    for (nzero = 0; nzero < degree; nzero++) {
        if (cpl_polynomial_get_coeff(self, &nzero) != 0.0) break;
    }

    if (nzero > 0) {
        cpl_size i = 0;
        for (; i <= degree - nzero; i++) {
            const cpl_size icopy = i + nzero;
            const double value = cpl_polynomial_get_coeff(self, &icopy);

            if (cpl_polynomial_set_coeff(self, &i, value))
                return cpl_error_set_where(cpl_func);
        }
        for (; i <= degree; i++) {
            if (cpl_polynomial_set_coeff(self, &i, 0.0))
                return cpl_error_set_where(cpl_func);
            cpl_vector_set(roots, (*preal)++, 0.0);
        }
    }
 
    return CPL_ERROR_NONE;

}

/**@}*/

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Compute all n roots of p(x) = 0, where p(x) is of degree n, n > 0
  @param    self  The 1D-polynomial, constant term is non-zero
  @param    roots A pre-allocated vector of length n to hold the roots
  @param    preal The number of real roots found so far or undefined on error
  @return   CPL_ERROR_NONE or the relevant CPL error code
  @see      irplib_polynomial_solve_1d_all()
  @note The caller may pass a roots vector which already has *preal (other)
        roots which are only accessed when all roots are sorted at the end

*/
/*----------------------------------------------------------------------------*/
static cpl_error_code irplib_polynomial_solve_1d_nonzero(cpl_polynomial * self,
                                                         cpl_vector * roots,
                                                         cpl_size * preal)
{
    cpl_error_code error   = CPL_ERROR_NONE;
    const cpl_size ncoeffs = 1 + cpl_polynomial_get_degree(self);


    cpl_ensure_code(ncoeffs   > 1,  CPL_ERROR_DATA_NOT_FOUND);
    cpl_ensure_code(*preal >= 0,    CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(ncoeffs + *preal == 1+cpl_vector_get_size(roots),
                    CPL_ERROR_INCOMPATIBLE_INPUT);

    switch (ncoeffs) {

    case 2 : {
        const cpl_size i1 = 1;
        const cpl_size i0 = 0;
        const double   p1 = cpl_polynomial_get_coeff(self, &i1);
        const double   p0 = cpl_polynomial_get_coeff(self, &i0);

        assert( p1 != 0.0 );

        cpl_vector_set(roots, (*preal)++, -p0/p1);
        break;
    }
    case 3 : {
        const cpl_size i2 = 2;
        const cpl_size i1 = 1;
        const cpl_size i0 = 0;
        const double   p2 = cpl_polynomial_get_coeff(self, &i2);
        const double   p1 = cpl_polynomial_get_coeff(self, &i1);
        const double   p0 = cpl_polynomial_get_coeff(self, &i0);
        double         x1, x2;

        assert( p2 != 0.0 );

        if (irplib_polynomial_solve_1d_2(p2, p1, p0, &x1, &x2)) {
            /* This is the complex root in the upper imaginary half-plane */
            cpl_vector_set(roots, (*preal)  , x1);
            cpl_vector_set(roots, (*preal)+1, x2);
        } else {
            cpl_vector_set(roots, (*preal)++, x1);
            cpl_vector_set(roots, (*preal)++, x2);
        }
        break;
    }
    case 4 : {
        const cpl_size i3 = 3;
        const cpl_size i2 = 2;
        const cpl_size i1 = 1;
        const cpl_size i0 = 0;
        const double   p3 = cpl_polynomial_get_coeff(self, &i3);
        const double   p2 = cpl_polynomial_get_coeff(self, &i2);
        const double   p1 = cpl_polynomial_get_coeff(self, &i1);
        const double   p0 = cpl_polynomial_get_coeff(self, &i0);
        double         x1, x2, x3;

        assert( p3 != 0.0 );

        if (irplib_polynomial_solve_1d_3(p3, p2, p1, p0, &x1, &x2, &x3,
                                         NULL, NULL)) {
            cpl_vector_set(roots, (*preal)++, x1);
            /* This is the complex root in the upper imaginary half-plane */
            cpl_vector_set(roots, (*preal)  , x2);
            cpl_vector_set(roots, (*preal)+1, x3);
        } else {
            cpl_vector_set(roots, (*preal)++, x1);
            cpl_vector_set(roots, (*preal)++, x2);
            cpl_vector_set(roots, (*preal)++, x3);
        }
        break;
    }
    case 5 : {
        const cpl_size i4 = 4;
        const cpl_size i3 = 3;
        const cpl_size i2 = 2;
        const cpl_size i1 = 1;
        const cpl_size i0 = 0;
        const double   p4 = cpl_polynomial_get_coeff(self, &i4);
        const double   p3 = cpl_polynomial_get_coeff(self, &i3);
        const double   p2 = cpl_polynomial_get_coeff(self, &i2);
        const double   p1 = cpl_polynomial_get_coeff(self, &i1);
        const double   p0 = cpl_polynomial_get_coeff(self, &i0);
        double         x1, x2, x3, x4;
        cpl_size       nreal;

        assert( p4 != 0.0 );

        error = irplib_polynomial_solve_1d_4(p4, p3, p2, p1, p0, &nreal,
                                             &x1, &x2, &x3, &x4);
        if (!error) {
            cpl_vector_set(roots, (*preal)  , x1);
            cpl_vector_set(roots, (*preal)+1, x2);
            cpl_vector_set(roots, (*preal)+2, x3);
            cpl_vector_set(roots, (*preal)+3, x4);

            *preal += nreal;
        }
        break;
    }

    default: {

        /* Try to reduce the problem by finding a single root */
        double         root = 0.0;

        error = irplib_polynomial_solve_1d_guess(self, &root);

        if (!error) {

            cpl_vector_set(roots, (*preal)++, root);

            irplib_polynomial_divide_1d_root(self, root, NULL);

            error = irplib_polynomial_solve_1d_nonzero(self, roots, preal);

        }

        break;
    }
    }

    return error ? cpl_error_set_where(cpl_func) : CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Try to find a single, real root of the provided 1D-polynomial
  @param    self  The 1D-polynomial, constant term is non-zero
  @param    proot The root or undefined on error
  @return   CPL_ERROR_NONE or the relevant CPL error code
  @see      irplib_polynomial_solve_1d_all()

*/
/*----------------------------------------------------------------------------*/
static
cpl_error_code irplib_polynomial_solve_1d_guess(const cpl_polynomial * self,
                                                double * proot)
{
    cpl_errorstate prestate = cpl_errorstate_get();
    cpl_error_code error = CPL_ERROR_NONE;
    const cpl_size degree = cpl_polynomial_get_degree(self);
    const cpl_size ncand = 5; /* 2 is enough for current guessing strategy */
    double rcand[ncand];
    cpl_size icand = 0;
    size_t ipos = 0, ineg = 0;
    double rpos[1], rneg[1];
    cpl_boolean do_bisect = CPL_FALSE;


    /* If the derivative at the first guess happens to be zero, then
       the first guess is no good, so try a few different ones. */

    for (cpl_size itry = 0; ; itry++) {
        switch (itry) {
          case 0: {
              /* Try the arithmetic mean of the roots */
              const double   pn0   = cpl_polynomial_get_coeff(self, &degree);
              const cpl_size n1    = degree-1;
              const double   pn1   = cpl_polynomial_get_coeff(self, &n1);
              double         rmean;

              assert( pn0 != 0.0 );

              rmean = -pn1 / (pn0 * (double)degree);

              rcand[icand++] = rmean;

              break;
          }

          case 1: {
              /* Try the geometric mean of the roots */
              const cpl_size i0 = 0;
              const double   c0 = cpl_polynomial_get_coeff(self, &i0);
              double         rmean;

              assert( c0 != 0.0 );

              rmean = pow(fabs(c0), 1.0/(double)degree);

              rcand[icand++] = rmean;

              break;
          }

          case 2: {
              /* Try to get starting guesses with opposite sign residuals */
              const cpl_size i0 = 0;
              const double   c0 = cpl_polynomial_get_coeff(self, &i0);
              const double   pn0   = cpl_polynomial_get_coeff(self, &degree);
              const cpl_size n1    = degree-1;
              const double   pn1   = cpl_polynomial_get_coeff(self, &n1);
              double         rmean;

              assert( pn0 != 0.0 );

              rmean = -pn1 / (pn0 * (double)degree);

              rcand[icand++] = rmean + c0;
              rcand[icand++] = rmean - c0;

              break;
          }

#ifdef IRPLIB_POLYNOMIAL_GUESS_ANASOL
          case 3: {

              /* Try an analytical solution to a (shifted) monomial */
              cpl_polynomial * copy = cpl_polynomial_duplicate(self);
              const cpl_size   i0   = 0;
              const double rmean = irplib_polynomial_depress_1d(copy);
              const double c0 = cpl_polynomial_get_coeff(copy, &i0);
              const double radius = pow(fabs(c0), 1.0/(double)degree);

              rcand[icand++] = rmean + radius;
              if (radius != 0.0) /* Should always be true */
                  rcand[icand++] = rmean - radius;

              cpl_polynomial_delete(copy);

              break;
          }
#endif

        default:
            /* From here on only first guesses increasingly refined via
               bisection are tried */
            if (ipos > 0 && ineg > 0) {
                rcand[icand++] = 0.5 * (rpos[0] + rneg[0]);
                do_bisect = CPL_TRUE;
            }
            break;
        }

        if (icand > 0) {
            double grad;
            double root = rcand[--icand];
            const double resid = cpl_polynomial_eval_1d(self, root, &grad);

#ifdef IRPLIB_POLYNOMIAL_DEBUG
            if (itry > 0)
                cpl_msg_warning(cpl_func, "RETRY(%d)=%g, degree=%d, r=%g, d=%g",
                                (int)itry, root, (int)degree, resid, grad);
#endif

            error = cpl_polynomial_solve_1d(self, root, proot, 1);
            if (!error) {
                cpl_errorstate_set(prestate);
                break;
            }

            if (do_bisect) {
                *(resid > 0.0 ? rpos : rneg) = root;
            } else {
                /* Try to collect first guess with residuals with opposite signs */
                if (resid > 0.0) {
                    if (ipos == 0) rpos[ipos++] = root;
                } else if (ineg == 0) {
                    rneg[ineg++] = root;
                }

                if (ipos == 0 || ineg == 0) {
                    const double resid2 = cpl_polynomial_eval_1d(self, *proot,
                                                                 &grad);
                    if (resid2 > 0.0) {
                        if (ipos == 0) rpos[ipos++] = root;
                    } else if (ineg == 0) {
                        rneg[ineg++] = root;
                    }
                }
            }

        } else {
            break;
        }
    }

    return error ? cpl_error_set_where(cpl_func) : CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Solve the quadratic equation p2 * x^2 + p1 * x + p0 = 0, p2 != 0
  @param    p2    The non-zero coefficient to x^2
  @param    p1    The coefficient to x
  @param    p0    The constant term
  @param    px1   The 1st real root or the real part of the complex, conjugate
                  roots
  @param    px2   The 2nd real root or the positive, imaginary part of the
                  complex roots
  @return   CPL_TRUE iff the roots are complex conjugate

 */
/*----------------------------------------------------------------------------*/
static cpl_boolean irplib_polynomial_solve_1d_2(double p2, double p1, double p0,
                                                double * px1,
                                                double * px2) {

    const double sqrtD = sqrt(p1 * p1 < 4.0 * p2 * p0
                              ? 4.0 * p2 * p0 - p1 * p1
                              : p1 * p1 - 4.0 * p2 * p0);
    cpl_boolean is_complex = CPL_FALSE;
    double x1 = -0.5 * p1 / p2; /* Double root */
    double x2;

    /* Compute residual, assuming D == 0 */
    double res0 = irplib_polynomial_eval_2_max(p2, p1, p0, CPL_FALSE, x1, x1);
    double res;

    assert(px1 != NULL );
    assert(px2 != NULL );

    *px2 = *px1 = x1;

    /* Compute residual, assuming D > 0 */

    /* x1 is the root with largest absolute value */
    if (p1 > 0.0) {
        x1 = -0.5 * (p1 + sqrtD);
    } else {
        x1 = -0.5 * (p1 - sqrtD);
    }
    /* Compute smaller root via division to avoid
       loss of precision due to cancellation */
    x2 = p0 / x1;
    x1 /= p2; /* Scale x1 with leading coefficient */

    res = irplib_polynomial_eval_2_max(p2, p1, p0, CPL_FALSE, x1, x2);

    if (res < res0) {
        res0 = res;
        if (x2 > x1) {
            *px1 = x1;
            *px2 = x2;
        } else {
            *px1 = x2;
            *px2 = x1;
        }
    }

    /* Compute residual, assuming D < 0 */

    x1 = -0.5 * p1 / p2;          /* Real part of complex root */
    x2 =  0.5 * sqrtD / fabs(p2); /* Positive, imaginary part of root */

    res  = irplib_polynomial_eval_2_max(p2, p1, p0, CPL_TRUE,  x1, x2);

    if (res < res0) {
        *px1 = x1;
        *px2 = x2;
        is_complex = CPL_TRUE;
    }

    return is_complex;

}


/*----------------------------------------------------------------------------*/
/**
  @brief    Find the max residual on a 2nd degree 1D-polynomial on the roots
  @param    p2   p2
  @param    p1   p1
  @param    p0   p0
  @param    is_c CPL_TRUE iff the two roots are complex
  @param    x1   The 1st point of evaluation (or real part on complex)
  @param    x2   The 2nd point of evaluation (or imaginary part on complex)
  @return   The result


 */
/*----------------------------------------------------------------------------*/
static double irplib_polynomial_eval_2_max(double p2, double p1, double p0,
                                           cpl_boolean is_c,
                                           double x1, double x2)
{
    double res;

    if (is_c) {
        res = fabs(p0 + x1 * (p1 + x1 * p2) - p2 * x2 * x2);
    } else {
        const double r1 = fabs(p0 + x1 * (p1 + x1 * p2));
        const double r2 = fabs(p0 + x2 * (p1 + x2 * p2));

        res = r1 > r2 ? r1 : r2;
    }

    return res;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Find the max residual on a 3rd degree 1D-polynomial on the roots
  @param    p3    p3
  @param    p2    p2
  @param    p1    p1
  @param    p0    p0
  @param    is_c  CPL_TRUE iff two roots are complex
  @param    x1    The 1st point of evaluation (real)
  @param    x2    The 2nd point of evaluation (or real part on complex)
  @param    x3    The 3rd point of evaluation (or imaginary part on complex)
  @return   The result


 */
/*----------------------------------------------------------------------------*/
static double irplib_polynomial_eval_3_max(double p3, double p2,
                                           double p1, double p0,
                                           cpl_boolean is_c,
                                           double x1, double x2, double x3)
{
    const double r1 = fabs(p0 + x1 * (p1 + x1 * (p2 + x1 * p3)));
    double res;

    if (is_c) {
        const double r2 = fabs(p0 + x2 * (p1 + x2 * (p2 + x2 * p3))
                               - x3 * x3 * ( 3.0 * p3 * x2 + p2));

        res = r1 > r2 ? r1 : r2;
    } else {
        const double r2 = fabs(p0 + x2 * (p1 + x2 * (p2 + x2 * p3)));
        const double r3 = fabs(p0 + x3 * (p1 + x3 * (p2 + x3 * p3)));
        res = r1 > r2 ? (r1 > r3 ? r1 : r3) : (r2 > r3 ? r2 : r3);
    }

    /* cpl_msg_info(cpl_func, "%d: %g (%g)", __LINE__, res, r1); */

    return res;
}


/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Solve the cubic equation p3 * x^3 + p2 * x^2 + p1 * x + p0 = 0
  @param    p3    The non-zero coefficient to x^3
  @param    p2    The coefficient to x^2
  @param    p1    The coefficient to x
  @param    p0    The constant term
  @param    px1   The 1st root (real)
  @param    px2   The 2nd root or the real part of the complex roots
  @param    px3   The 3rd root or the imaginary part of the complex roots
  @param    pdbl1 CPL_TRUE iff *px1 == *px2 (only for all real)
  @param    pdbl2 CPL_TRUE iff *px2 == *px3 (only for all real)
  @return   CPL_TRUE iff two of the roots are complex
  @see gsl_poly_complex_solve_cubic() of GSL v. 1.9
  @note px2 and px3 may be NULL, if in this case all three roots are real *px1
        will be set to the largest root.

 */
/*----------------------------------------------------------------------------*/
static cpl_boolean irplib_polynomial_solve_1d_3(double p3, double p2, double p1,
                                                double p0,
                                                double * px1,
                                                double * px2,
                                                double * px3,
                                                cpl_boolean * pdbl1,
                                                cpl_boolean * pdbl2) {
    cpl_boolean is_complex = CPL_FALSE;
    const double a = p2/p3;
    const double b = p1/p3;
    const double c = p0/p3;

    const double q = (a * a - 3.0 * b);
    const double r = (a * (2.0 * a * a - 9.0 * b) + 27.0 * c);

    const double Q = q / 9.0;
    const double R = r / 54.0;

    const double Q3 = Q * Q * Q;
    const double R2 = R * R;

    double x1 = DBL_MAX; /* Fix (false) uninit warning */
    double x2 = DBL_MAX; /* Fix (false) uninit warning */
    double x3 = DBL_MAX; /* Fix (false) uninit warning */
    double xx1 = DBL_MAX; /* Fix (false) uninit warning */
    double xx2 = DBL_MAX; /* Fix (false) uninit warning */
    double xx3 = DBL_MAX; /* Fix (false) uninit warning */

    double resx = DBL_MAX;
    double res  = DBL_MAX;
    cpl_boolean is_first = CPL_TRUE;

    cpl_boolean dbl2;


    assert(px1 != NULL );

    if (pdbl1 != NULL) *pdbl1 = CPL_FALSE;
    if (pdbl2 != NULL) *pdbl2 = CPL_FALSE;

    dbl2 = CPL_FALSE;

    /*
      All branches (for which the roots are defined) are evaluated, and
      the branch with the smallest maximum-residual is chosen.
      When two maximum-residual are identical, preference is given to
      the purely real solution and if necessary to the solution with a
      double root.
    */

    if ((R2 >= Q3 && R != 0.0) || R2 > Q3) {

        cpl_boolean is_c = CPL_FALSE;

        irplib_polynomial_solve_1d_3c(a, c, Q, Q3, R, R2, &x1, &x2, &x3,
                                      &is_c, &dbl2);


        res = resx = irplib_polynomial_eval_3_max(p3, p2, p1, p0, is_c,
                                            x1, x2, x3);

        is_first = CPL_FALSE;

        if (pdbl1 != NULL) *pdbl1 = CPL_FALSE;
        if (!is_c && pdbl2 != NULL) *pdbl2 = dbl2;
        is_complex = is_c;
   
    }

    if (Q > 0.0 && fabs(R / (Q * sqrt(Q))) <= 1.0) {

        /* this test is actually R2 < Q3, written in a form suitable
           for exact computation with integers */

        /* assert( Q > 0.0 ); */

        irplib_polynomial_solve_1d_3r(a, c, Q, R, &xx1, &xx2, &xx3);

        resx = irplib_polynomial_eval_3_max(p3, p2, p1, p0, CPL_FALSE,
                                            xx1, xx2, xx3);

        if (is_first || (dbl2 ? resx < res : resx <= res)) {
            is_first = CPL_FALSE;
            res = resx;
            x1 = xx1;
            x2 = xx2;
            x3 = xx3;
            if (pdbl1 != NULL) *pdbl1 = CPL_FALSE;
            if (pdbl2 != NULL) *pdbl2 = CPL_FALSE;
            is_complex = CPL_FALSE;
        }
    }

    if (Q >= 0) {
        cpl_boolean dbl1 = CPL_FALSE;


        irplib_polynomial_solve_1d_32(a, c, Q, &xx1, &xx2, &xx3, &dbl2);

        resx = irplib_polynomial_eval_3_max(p3, p2, p1, p0, CPL_FALSE,
                                            xx1, xx2, xx3);
        /*
        cpl_msg_info(cpl_func, "%d: %g = %g - %g (%u)", __LINE__,
                     res - resx, res, resx, is_complex);
        */

        if (is_first || resx <= res) {
            is_first = CPL_FALSE;
            res = resx;
            x1 = xx1;
            x2 = xx2;
            x3 = xx3;
            if (pdbl1 != NULL) *pdbl1 = CPL_FALSE;
            if (pdbl2 != NULL) *pdbl2 = dbl2;
            is_complex = CPL_FALSE;
        }


        /* This branch also covers the case where the depressed cubic
           polynomial has zero as triple root (i.e. Q == R == 0) */

        irplib_polynomial_solve_1d_31(a, Q, &xx1, &xx2, &xx3, &dbl1);

        resx = irplib_polynomial_eval_3_max(p3, p2, p1, p0, CPL_FALSE,
                                            xx1, xx2, xx3);

        if (resx <= res) {
            is_first = CPL_FALSE;
            /*res = resx;*/
            x1 = xx1;
            x2 = xx2;
            x3 = xx3;
            if (pdbl1 != NULL) *pdbl1 = dbl1;
            if (pdbl2 != NULL) *pdbl2 = CPL_FALSE;
            is_complex = CPL_FALSE;
        }

    }

    if (px2 != NULL && px3 != NULL) {
        *px1 = x1;
        *px2 = x2;
        *px3 = x3;
    } else if (is_complex) {
        *px1 = x1;
    } else {
        *px1 = x3;
    }

    return is_complex;
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Solve the monic, depressed cubic with 1st and 2nd root as double
  @param    a     p2/p3 for back-transform
  @param    Q     The linear term, must be positive
  @param    px1   The 1st root
  @param    px2   The 2nd root
  @param    px3   The 3rd root
  @param    pdbl1 CPL_TRUE iff *px1 == *px2
  @return   void
  @see irplib_polynomial_solve_1d_3()

 */
/*----------------------------------------------------------------------------*/
static void irplib_polynomial_solve_1d_31(double a, double Q,
                                          double * px1, double * px2,
                                          double * px3, cpl_boolean * pdbl1)
{

    const double sqrtQ = sqrt (Q);

    double x1, x2, x3;

    x2 = x1 = -sqrtQ - a / 3.0;
    x3 = 2.0 * sqrtQ - a / 3.0;
    if (pdbl1 != NULL) *pdbl1 = CPL_TRUE;

    *px1 = x1;
    *px2 = x2;
    *px3 = x3;

    return;
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Solve the monic, depressed cubic with 2nd and 3rd root as double
  @param    a     p2/p3 for back-transform
  @param    c     p0/p3 for finding root via division of constant term
  @param    Q     The linear term, must be positive
  @param    px1   The 1st root
  @param    px2   The 2nd root
  @param    px3   The 3rd root
  @param    pdbl2 CPL_TRUE iff *px2 == *px3
  @return   void
  @see irplib_polynomial_solve_1d_3()

 */
/*----------------------------------------------------------------------------*/
static void irplib_polynomial_solve_1d_32(double a, double c, double Q,
                                          double * px1, double * px2,
                                          double * px3, cpl_boolean * pdbl2)
{

    const double sqrtQ = sqrt (Q);

    double x1 = DBL_MAX;
    double x2 = DBL_MAX;
    double x3 = DBL_MAX;

    if (a > 0.0) {
        /* a and sqrt(Q) have same sign - or Q is zero */
        x1 = -2.0 * sqrtQ - a / 3.0;
        /* FIXME: Two small roots with opposite signs may
           end up here, with the sign lost for one of them */
        x3 = x2 = -a < x1 ? -sqrt(fabs(c / x1)) : sqrt(fabs(c / x1));
        if (pdbl2 != NULL) *pdbl2 = CPL_TRUE;
    } else if (a < 0.0) {
        /* a and sqrt(Q) have opposite signs - or Q is zero */
        x3 = x2 = sqrtQ - a / 3.0;
        x1 = -c / (x2 * x2);
        if (pdbl2 != NULL) *pdbl2 = CPL_TRUE;
    } else {
        x1 = -2.0 * sqrtQ;
        x3 = x2 = sqrtQ;
        if (pdbl2 != NULL) *pdbl2 = CPL_TRUE;
    }

    *px1 = x1;
    *px2 = x2;
    *px3 = x3;

    return;
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Solve the monic, depressed cubic with complex roots
  @param    a     p2/p3 for back-transform
  @param    c     p0/p3 for finding root via division of constant term
  @param    Q     The linear term
  @param    Q3    Q^3
  @param    R     The constant term
  @param    R2    R^2
  @param    px1   The 1st root (real)
  @param    px2   The 2nd root or the real part of the complex roots
  @param    px3   The 3rd root or the imaginary part of the complex roots
  @param    pis_c CPL_TRUE iff complex
  @param    pdbl2 CPL_TRUE iff *px2 == *px3 (only for all real)
  @return   void
  @see irplib_polynomial_solve_1d_3()
  @note If all roots are real, then two of them are a double root.

 */
/*----------------------------------------------------------------------------*/
static void irplib_polynomial_solve_1d_3c(double a, double c,
                                          double Q, double Q3,
                                          double R, double R2,
                                          double * px1,
                                          double * px2, double * px3,
                                          cpl_boolean * pis_c,
                                          cpl_boolean * pdbl2)
{

    /* Due to finite precision some double roots may be missed, and
       will be considered to be a pair of complex roots z = x +/-
       epsilon i close to the real axis. */

    /* Another case: A double root, which is small relative to the
       last root, may cause this branch to be taken - with the
       imaginary part eventually being truncated to zero. */

    const double sgnR = (R >= 0 ? 1.0 : -1.0);
    const double A = -sgnR * pow (fabs (R) + sqrt (R2 - Q3), 1.0 / 3.0);
    const double B = Q / A;

    double x1 = DBL_MAX;
    double x2 = DBL_MAX;
    double x3 = DBL_MAX;
    cpl_boolean is_complex = CPL_FALSE;

    if (( A > -B && a > 0.0) || (A < -B && a < 0.0)) {
        /* A+B has same sign as a */

        /* Real part of complex conjugate */
        x2 = -0.5 * (A + B) - a / 3.0; /* No cancellation */
        /* Positive, imaginary part of complex conjugate */
        x3 = 0.5 * CPL_MATH_SQRT3 * fabs(A - B);

        x1 = -c / (x2 * x2 + x3 * x3);
    } else {
        /* A+B and a have opposite signs - or exactly one is zero */
        x1 = A + B - a / 3.0;
        /* Positive, imaginary part of complex conjugate */
        x3 = 0.5 * CPL_MATH_SQRT3 * fabs(A - B);

        if (x3 > 0.0) {
            /* Real part of complex conjugate */
            x2 = -0.5 * (A + B) - a / 3.0; /* FIXME: Cancellation */
        } else {

            x2 = -a < x1 ? -sqrt(fabs(c / x1)) : sqrt(fabs(c / x1));
            x3 = 0.0;
        }
    }

    if (x3 > 0.0) {
        is_complex = CPL_TRUE;
    } else {
        /* Whoaa, the imaginary part was truncated to zero
           - return a real, double root */
        x3 = x2;
        if (pdbl2 != NULL) *pdbl2 = CPL_TRUE;
    }

    *px1 = x1;
    *px2 = x2;
    *px3 = x3;
    *pis_c = is_complex;

    return;
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Solve the monic, depressed cubic with 3 distinct, real roots
  @param    a     p2/p3 for back-transform
  @param    c     p0/p3 for finding root via division of constant term
  @param    Q     The linear term, must be positive
  @param    R     The constant term
  @param    px1   The 1st root
  @param    px2   The 2nd root
  @param    px3   The 3rd root
  @return   void
  @see irplib_polynomial_solve_1d_3()

 */
/*----------------------------------------------------------------------------*/
static void irplib_polynomial_solve_1d_3r(double a, double c,
                                          double Q, double R,
                                          double * px1,
                                          double * px2, double * px3)
{

    const double sqrtQ = sqrt(Q);
    const double theta = acos (R / (Q * sqrtQ)); /* theta in range [0; pi] */

    /* -1.0 <= cos((theta + CPL_MATH_2PI) / 3.0) <= -0.5
       -0.5 <= cos((theta - CPL_MATH_2PI) / 3.0) <=  0.5
        0.5 <= cos((theta               ) / 3.0) <=  1.0 */

#define TR1 (-2.0 * sqrtQ * cos( theta                 / 3.0))
#define TR2 (-2.0 * sqrtQ * cos((theta - CPL_MATH_2PI) / 3.0))
#define TR3 (-2.0 * sqrtQ * cos((theta + CPL_MATH_2PI) / 3.0))

    /* TR1 < TR2 < TR3, except when theta == 0, then TR2 == TR3 */

    /* The three roots must be transformed back via subtraction with a/3.
       To prevent loss of precision due to cancellation, the root which
       is closest to a/3 is computed using the relation
       p3 * x1 * x2 * x3 = -p0 */

    double x1 = DBL_MAX;
    double x2 = DBL_MAX;
    double x3 = DBL_MAX;

    if (a > 0.0) {
        x1 = TR1 - a / 3.0;
        if (TR2 > 0.0 && (TR2 + TR3) > 2.0 * a) {
            /* FIXME: Cancellation may still effect x3 ? */
            x3 = TR3 - a / 3.0;
            x2 = -c / ( x1 * x3 );
        } else {
            /* FIXME: Cancellation may still effect x2, especially
               if x2, x3 is (almost) a double root, i.e.
               if theta is close to zero. */
            x2 = TR2 - a / 3.0;
 
            x3 = -c / ( x1 * x2 );
        }
    } else if (a < 0.0) {
        x3 = TR3 - a / 3.0;
        if (TR2 < 0.0 && (TR1 + TR2) > 2.0 * a) {
            x1 = TR1 - a / 3.0;
            x2 = -c / ( x1 * x3 );
        } else {
            x2 = TR2 - a / 3.0;
            x1 = -c / ( x2 * x3 );
        }
    } else {
        x1 = TR1;
        x2 = TR2;
        x3 = TR3;
    }

    assert(x1 < x3);

    if (x1 > x2) {
        /* In absence of round-off:
           theta == PI: x1 == x2,
           theta  < PI: x1 <  x2,

           The only way x1 could exceed x2 would be due to round-off when
           theta is close to PI */
     
        x1 = x2 = 0.5 * ( x1 + x2 );
    } else if (x2 > x3) {
        /* In absence of round-off:
           theta == 0: x2 == x3,
           theta  > 0: x2 <  x3,

           For small theta:
           Round-off can cause x2 to become greater than x3 */
     
        x3 = x2 = 0.5 * ( x2 + x3 );
    }

    *px1 = x1;
    *px2 = x2;
    *px3 = x3;

    return;
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Solve the quartic equation
            p4 * x^4 + p3 * x^3 + p2 * x^2 + p1 * x + p0 = 0
  @param    p4    The non-zero coefficient to x^4
  @param    p3    The coefficient to x^3
  @param    p2    The coefficient to x^2
  @param    p1    The coefficient to x
  @param    p0    The constant term
  @param    preal *preal is the number of real roots, or undefined on error
  @param    px1   The 1st root or the real part of the 1st complex roots-pair
  @param    px2   The 2nd root or the imaginary part of the 1st complex roots
  @param    px3   The 3rd root or the real part of the 2nd complex roots-pair
  @param    px4   The 4th root or the imaginary part of the 2nd complex roots
  @return   CPL_ERROR_NONE or the relevant CPL error code

 */
/*----------------------------------------------------------------------------*/
static cpl_error_code irplib_polynomial_solve_1d_4(double p4, double p3,
                                                   double p2, double p1,
                                                   double p0, cpl_size * preal,
                                                   double * px1, double * px2,
                                                   double * px3, double * px4)
{

    /* Construct the monic, depressed quartic using Horners scheme on 1 / p4 */
    const double a = (p2 - 0.375 * p3 * p3 / p4) / p4;
    const double b = (p1 - 0.5 * (p2 - 0.25 * p3 * p3 / p4 ) * p3 / p4 ) / p4;
    const double c =
        (p0 - 0.25 * (p1 - 0.25 * (p2 - 0.1875 * p3 * p3 / p4 ) * p3 / p4
                      ) * p3 / p4 ) / p4;

    double x1 = DBL_MAX; /* Fix (false) uninit warning */
    double x2 = DBL_MAX; /* Fix (false) uninit warning */
    double x3 = DBL_MAX; /* Fix (false) uninit warning */
    double x4 = DBL_MAX; /* Fix (false) uninit warning */

    assert(preal != NULL );
    assert(px1   != NULL );
    assert(px2   != NULL );
    assert(px3   != NULL );
    assert(px4   != NULL );

    *preal = 4;

    if (c == 0.0) {
        /* The depressed quartic has zero as root */
        /* Since the sum of the roots is zero, at least one is negative
           and at least one is positive - unless they are all zero */
        cpl_boolean dbl1, dbl2;
        const cpl_boolean is_real =
            !irplib_polynomial_solve_1d_3(1.0, 0.0, a, b, &x1, &x3, &x4,
                                          &dbl1, &dbl2);

        x1 -= 0.25 * p3 / p4;
        x2 = -0.25 * p3 / p4;
        x3 -= 0.25 * p3 / p4;
        if (is_real) {

            if (dbl2) {
                x4 = x3;
                assert( x1 <= x2);
                assert( x2 <= x3);
            } else {
                x4 -= 0.25 * p3 / p4;
                /* Need (only) a guarded swap of x2, x3 */
                if (x2 > x3) {
                    IRPLIB_SWAP_DOUBLE(x2, x3);
                }
                if (dbl1) {
                    assert( x1 <= x2); /* The cubic may have 0 as triple root */
                    assert( x2 <= x3);
                    assert( x2 <= x4);
                } else {
                    assert( x1 < x2);
                    assert( x2 < x4);
                }
            }
        } else {
            *preal = 2;

            if (x1 > x2) {
                assert( x3 <= x2 ); /* Don't swap a complex root */

                IRPLIB_SWAP_DOUBLE(x1, x2);
            } else {
                assert( x3 >= x2 );
            }
        }

    } else if (b == 0.0) {
        /* The monic, depressed quartic is a monic, biquadratic equation */
        double u1, u2;
        const cpl_boolean is_complex = irplib_polynomial_solve_1d_2(1.0, a, c,
                                                                    &u1, &u2);

        if (is_complex) {
            /* All four roots are conjugate, complex */
            const double norm = sqrt(u1*u1 + u2*u2);
            const double   v1 = sqrt(0.5*(norm+u1));
            const double   v2 = u2 / sqrt(2.0*(norm+u1));


            x1 = -0.25 * p3 / p4 - v1;
            x3 = -0.25 * p3 / p4 + v1;

            x4 = x2 = v2;

            *preal = 0;

        } else if (u1 >= 0.0) {
            /* All four roots are real */
            const double sv1 = sqrt(u1);
            const double sv2 = sqrt(u2);


            *preal = 4;

            x1 = -0.25 * p3 / p4 - sv2;
            x2 = -0.25 * p3 / p4 - sv1;
            x3 = -0.25 * p3 / p4 + sv1;
            x4 = -0.25 * p3 / p4 + sv2;
        } else if (u2 < 0.0) {
            /* All four roots are conjugate, complex */
            const double sv1 = sqrt(-u2);
            const double sv2 = sqrt(-u1);


            *preal = 0;

            x1 = x3 = -0.25 * p3 / p4;

            x2 = sv1;
            x4 = sv2;
        } else {
            /* Two roots are real, two roots are conjugate, complex */
            const double sv1 = sqrt(-u1);
            const double sv2 = sqrt(u2);


            *preal = 2;

            x1 = -0.25 * p3 / p4 - sv2;
            x2 = -0.25 * p3 / p4 + sv2;

            x3 = -0.25 * p3 / p4;
            x4 = sv1;
        }
    } else {
        /* Need a root from the nested, monic cubic */
        const double q2 = -a;
        const double q1 = -4.0 * c;
        const double q0 = 4.0 * a * c - b * b;
        double u1, sqrtd, sqrtrd;
        double z1, z2, z3, z4;

        cpl_boolean is_complex1, is_complex2;

        /* Largest cubic root ensures real square roots when solving the
           quartic equation */
        (void)irplib_polynomial_solve_1d_3(1.0, q2, q1, q0, &u1, NULL, NULL,
                                           NULL, NULL);


        assert( u1 > a );

        sqrtd = sqrt(u1 - a);

        sqrtrd = 0.5 * b/sqrtd;

        is_complex1 = irplib_polynomial_solve_1d_2(1.0,  sqrtd, 0.5*u1 - sqrtrd,
                                                   &z1, &z2);

        is_complex2 = irplib_polynomial_solve_1d_2(1.0, -sqrtd, 0.5*u1 + sqrtrd,
                                                   &z3, &z4);

        z1 -= 0.25 * p3 / p4;
        z3 -= 0.25 * p3 / p4;
        if (!is_complex1) z2 -= 0.25 * p3 / p4;
        if (!is_complex2) z4 -= 0.25 * p3 / p4;

        if (!is_complex1 && is_complex2) {
            *preal = 2;
            x1 = z1;
            x2 = z2;
            x3 = z3;
            x4 = z4;
        } else if (is_complex1 && !is_complex2) {
            *preal = 2;
            x1 = z3;
            x2 = z4;
            x3 = z1;
            x4 = z2;
        } else if (is_complex1 && is_complex2) {
            *preal = 0;

            if (z1 < z3 || (z1 == z3 && z2 <= z4)) {
                x1 = z1;
                x2 = z2;
                x3 = z3;
                x4 = z4;
            } else {
                x1 = z3;
                x2 = z4;
                x3 = z1;
                x4 = z2;
            }
        } else {
            *preal = 4;

            if (z3 >= z2) {
                x1 = z1;
                x2 = z2;
                x3 = z3;
                x4 = z4;
            } else if (z4 <= z1) {
                x1 = z3;
                x2 = z4;
                x3 = z1;
                x4 = z2;
            } else if (z2 > z4) {
                x1 = z3;
                x2 = z1;
                x3 = z4;
                x4 = z2;
            } else {
                x1 = z1;
                x2 = z3;
                x3 = z2;
                x4 = z4;
            }
        }
    }

    *px1 = x1;
    *px2 = x2;
    *px3 = x3;
    *px4 = x4;

    return CPL_ERROR_NONE;
}

#ifdef IRPLIB_POLYNOMIAL_GUESS_ANASOL
/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Depress a 1D-polynomial of degree at least 1
  @param    self   The 1D-polynomial to modify
  @return   The mean of the roots of the input polynomial

 */
/*----------------------------------------------------------------------------*/
static double irplib_polynomial_depress_1d(cpl_polynomial * self)
{

    const cpl_size degree = cpl_polynomial_get_degree(self);
    const cpl_size nc1    = degree - 1;
    const double   an     = cpl_polynomial_get_coeff(self, &degree);
    const double   an1    = cpl_polynomial_get_coeff(self, &nc1);
    const double   rmean  = an != 0.0 ? -an1/(an * (double)degree) : 0.0;


    cpl_ensure(degree > 0,   CPL_ERROR_DATA_NOT_FOUND, 0.0);

    assert( an != 0.0 );

    if (rmean != 0.0) {

        cpl_polynomial_shift_1d(self, 0, rmean);

    }

    /* Divide polynomial by leading coefficient */
    for (cpl_size i = 0; i < nc1; i++) {
        const double ai = cpl_polynomial_get_coeff(self, &i) / an;
        cpl_polynomial_set_coeff(self, &i, ai);
    }

    cpl_polynomial_set_coeff(self, &nc1,    0.0); /* Ensure exact values */
    cpl_polynomial_set_coeff(self, &degree, 1.0); /* Ensure exact values */

    return rmean;
}
#endif

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief  Modify p, p(x) := p(x)/(x-r), where p(r) = 0.
  @param  p    The polynomial to be modified in place
  @param  r    The root
  @param  pres If non-NULL, *pres is the residual of the original p, p(r).
  @return CPL_ERROR_NONE or the relevant CPL error code

  Possible #_cpl_error_code_ set in this function:
  - CPL_ERROR_NULL_INPUT if an input pointer is NULL
  - CPL_ERROR_INVALID_TYPE if the polynomial has the wrong dimension
  - CPL_ERROR_DATA_NOT_FOUND if the polynomial does not have a degree of at
                             least 1.
 */
/*----------------------------------------------------------------------------*/
static
cpl_error_code irplib_polynomial_divide_1d_root(cpl_polynomial * p, double r,
                                                double * pres)
{

    const cpl_size n = cpl_polynomial_get_degree(p);
    double         sum;
    cpl_size       i;


    cpl_ensure_code(p != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(n > 0, CPL_ERROR_DATA_NOT_FOUND);

    sum = cpl_polynomial_get_coeff(p, &n);
    cpl_polynomial_set_coeff(p, &n, 0.0);

    for (i = n-1; i >= 0; i--) {
        const double coeff = cpl_polynomial_get_coeff(p, &i);

        cpl_polynomial_set_coeff(p, &i, sum);

        sum = coeff + r * sum;

    }

    if (pres != NULL) *pres = sum;

    return CPL_ERROR_NONE;
}
