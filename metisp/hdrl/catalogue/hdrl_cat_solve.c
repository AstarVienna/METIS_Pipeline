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

#include "hdrl_cat_solve.h"


/*---------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_solve       hdrl_solve
 * @ingroup  Catalogue
 *
 * @brief    Use Gauss-Jordan elimination to solve ax=b
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Use Gauss-Jordan elimination to solve ax=b
 *
 * @param    a         Input matrix
 * @param    b         Input vector
 * @param    m         rank of matrix
 *
 * @return   CPL_ERROR_NONE if all went well (Currently it's the only value).
 *
 * Description:
 * 	    Use Gauss-Jordan elimination to solve ax=b Standard algorithm
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_solve(double a[25][25], double b[25], cpl_size m)
{
	cpl_size l  = 0;
	cpl_size iu = m - 1;

	for (cpl_size i = 0; i < iu; i++) {

		/* find largest remaining term in i-th column for pivot */
		double big = 0.;
		for (cpl_size k = i; k < m; k++) {

			double rmax = fabs(a[i][k]);
			if (rmax > big) {
				big = rmax;
				l   = k;
			}
		}

		/* check for non-zero term */
		if (big == 0.) {
			for (cpl_size ib = 0; ib < m; ib++) {
				b[ib] = 0.;
			}
			return CPL_ERROR_NONE;
		}

		if (i != l) {

			/* switch rows */
			for (cpl_size j = 0; j < m; j++) {

				double temp = a[j][i];
				a[j][i]     = a[j][l];
				a[j][l]     = temp;
			}

			double temp = b[i];
			b[i]        = b[l];
			b[l]        = temp;
		}

		/* pivotal reduction */
		double pivot = a[i][i];

		cpl_size jl = i + 1;
		for (cpl_size j = jl; j < m; j++) {

			double temp = a[i][j] / pivot;

			b[j] -= temp * b[i];

			for (cpl_size k = i; k < m; k++) {
				a[k][j] -= temp * a[k][i];
			}
		}
	}

	/* back substitution for solution */
	for (cpl_size i = 0; i < m; i++) {

		cpl_size ir = m - 1 - i;
		if (a[ir][ir] != 0.) {

			double temp = b[ir];
			if (ir != m-1) {

				for (cpl_size j = 1; j <= i; j++) {
					cpl_size k = m - j;
					temp -= a[k][ir] * b[k];
				}
			}

			b[ir] = temp / a[ir][ir];

		} else {

			b[ir] = 0.;
		}
	}

	return CPL_ERROR_NONE;
}

/**@}*/

