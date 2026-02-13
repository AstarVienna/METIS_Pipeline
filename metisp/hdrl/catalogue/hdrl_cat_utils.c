/*
 * This file is part of the HDRL
 * Copyright (C) 2017 European Southern Observatory
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "hdrl_cat_utils_sort.h"

/*---------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_utils_catalogue  hdrl_utils_catalogue
 * @ingroup  Catalogue
 *
 * @brief    Common functions
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Fraction of pixel bounded
 *
 * @param    x         X Cordinate relative to center
 * @param    y         Y Cordinate relative to center
 * @param    r_out
 *
 * @return   fraction of pixel bounded by 0 - r_out
 *
 * Description:
 * 	    Returns fraction of pixel bounded by 0 -  r_out x,y coordinates relative to centre.
 *      Uses linear approximation ok if pixel located >>1 away from centre.
 */
/* ---------------------------------------------------------------------------*/
double fraction(double x, double y, double r_out)
{
    double r       = sqrt(x * x + y * y);
    double sqrt2o2 = 0.5 * CPL_MATH_SQRT2;

    if (r > r_out + sqrt2o2) {	        /* Is it worth bothering?  */
        return 0.;
    } else if (r < r_out - sqrt2o2) {	/* Is it trivially all in? */
        return(1.);
    }

    /* bugger - have to do some work then ... ok first ...
     * use 8-fold symmetry to convert to 0-45 degree range */
    x = fabs(x);
    y = fabs(y);
    if (y > x) {
        double t = x;
        x = y;
        y = t;
    }

    /* If the angles are too close to cardinal points, then fudge something */
    double tanao2;
    double tanp2a;
    double cosa;
    if (x > 0. && y > 0.) {
        tanao2 = 0.5 * y / x;
        tanp2a = x / y;
        cosa   = x / sqrt(x * x + y * y);
    } else {
        tanao2 = 0.00005;
        tanp2a = 10000.;
        cosa   = 1.;
    }

    /* only outer radius - compute linear intersections top and bot of pixel */
    double frac;

    double x_a = x - tanao2 + (r_out - r) / cosa;
    if (x_a < x + 0.5) {

        /* intersects */
        double x_b = x + tanao2 + (r_out - r) / cosa;

        /* three cases to consider */
        if (x_a < x - 0.5) {
            frac = 0.5 * CPL_MAX(0., x_b - (x - 0.5)) * CPL_MAX(0., x_b - (x - 0.5)) * tanp2a;
        } else if (x_b > x + 0.5) {
            frac = 1. - 0.5 * (x + 0.5 - x_a) * (x + 0.5 - x_a) * tanp2a;
        } else {
            frac = 0.5 - (x - x_a) + 0.5 * (x_b - x_a);
        }

    } else {  /* missed entirely */
        frac = 1.;
    }

    return frac;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Cholesky decomposition of definite symmetric matrix to solve Ax = b
 *
 * @param    a         .
 * @param    b         .
 * @param    n         .
 *
 * Description:
 * 	    Cholesky decomposition of definite symmetric matrix to solve Ax = b
 */
/* ---------------------------------------------------------------------------*/
void dchole(double a[IMNUM+1][IMNUM+1], double b[IMNUM+1], cpl_size n)
{
	double l[IMNUM + 1][IMNUM + 1];

	cpl_boolean restart = CPL_FALSE;
	do {

		l[0][0] = sqrt(a[0][0]);

		for (cpl_size k = 1; k < n && !restart; k++) {

			for (cpl_size j = 0; j <= k - 1; j++) {

				double sum = a[j][k];
				if (j != 0) {
					for (cpl_size i = 0; i <= j - 1; i++) sum -= l[i][k] * l[i][j];
				}

				l[j][k] = sum / l[j][j];
			}

			double sum = a[k][k];
			for (cpl_size i = 0; i <= k - 1; i++) {
				sum -= l[i][k] * l[i][k];
			}

			/* if true -> warning: matrix ill-conditioned. max eigenvalue < trace. Offset added to diagonal */
			if (sum <= 0.) {

				double aveigv = a[0][0];
				for (cpl_size i = 1; i < n; i++) aveigv += a[i][i];

				double offset = 0.1 * aveigv / (double)n;
				for (cpl_size i = 0; i < n; i++) a[i][i] += offset;

				restart = CPL_TRUE;
			}

			if (!restart) {
				l[k][k] = sqrt(sum);
			}
		}

	} while (restart);


	double y[IMNUM + 1];

	/* solve Ly = b */
	y[0] = b[0] / l[0][0];
	for (cpl_size i = 1; i < n; i++) {

		double sum = b[i];
		for (cpl_size k = 0; k <= i - 1; k++) {
			sum -= l[k][i] * y[k];
		}

		y[i] = sum / l[i][i];
	}

	/* solve L(T)x = y */
	b[n - 1] = y[n - 1] / l[n - 1][n - 1];
	for (cpl_size i = n - 2; i >= 0; i--) {

		double sum = y[i];
		for (cpl_size k = i+1; k < n; k++) {
			sum -= l[i][k] * b[k];
		}

		b[i] = sum / l[i][i];
	}
}

/**@}*/
