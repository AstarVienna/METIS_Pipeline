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

#include "hdrl_cat_polynm.h"

#include "hdrl_cat_solve.h"


/*---------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_polynm      hdrl_polynm
 * @ingroup  Catalogue
 *
 * @brief    Work out the median seeing
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Work out the median seeing
 *
 * @param    xdat      x data points
 * @param    xcor
 * @param    n         Number of data
 * @param    polycf    polynomial coefficients
 * @param    m         number of coefficients
 * @param    ilim
 *
 * @return   CPL_ERROR_NONE if all went well (Currently it's the only value).
 *
 * Description:
 * 	    Determine polynomial coefficients
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_polynm(
		double xdat[], double xcor[], cpl_size n, double polycf[], cpl_size m, cpl_size ilim)
{
	/* Clear arrays */
    #define sizeAray 25
    double a[sizeAray][sizeAray];
    double b[sizeAray];
	for (cpl_size i = 0; i < sizeAray; i++) {

		b[i] = 0.;

		for (cpl_size j = 0; j < sizeAray; j++) {
			a[i][j] = 0.;
		}
	}

	/* Acumulate sums */
	for (cpl_size i = 0; i < n; i++) {

		for (cpl_size k = 0; k < m; k++) {

			double temp = 1.;

			if (k+ilim != 0) {
				temp = pow(xcor[i], (double)(k + ilim));
			}

			b[k] += xdat[i] * temp;

			for (cpl_size j = 0; j <= k; j++) {

				temp = 1.;

				if (k+j+2*ilim != 0) {
					temp = pow(xcor[i], (double)(k + j + 2 * ilim));
				}

				a[j][k] += temp;
			}
		}
	}

	for (cpl_size k = 1; k < m; k++) {
		for (cpl_size j = 0; j < k; j++) {
			a[k][j] = a[j][k];
		}
	}

	/* solve linear equations */
	hdrl_solve(a, b, m);

	for (cpl_size i = 0; i < m; i++) {
		polycf[i] = b[i];
	}

	return CPL_ERROR_NONE;
}

/**@}*/
