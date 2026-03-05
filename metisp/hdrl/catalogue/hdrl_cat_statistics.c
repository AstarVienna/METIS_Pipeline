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

#include "hdrl_cat_statistics.h"

#include "hdrl_cat_utils_sort.h"


/*---------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_statistics  hdrl_statistics
 * @ingroup  Catalogue
 *
 * @brief    Compute statistics
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    compute median
 *
 * @param    xbuf      buffer
 * @param    npt       number of points
 * @param    nfilt     size of median filter
 *
 * @return   CPL_ERROR_NONE if all went well, CPL_ERROR_INCOMPATIBLE_INPUT if there isn't enough size.
 *
 * Description:
 * 	    performs median filtering on array xbuf
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_median(double xbuf[], cpl_size npt, cpl_size nfilt)
{
	if ((nfilt / 2) * 2 == nfilt) {
		nfilt++;
	}

	if (npt <= nfilt) {
		return CPL_ERROR_INCOMPATIBLE_INPUT;
	}

	cpl_size nfo2p1 = nfilt / 2;

	/* allocate ybuf, array, point */
	cpl_size nelem  = npt + nfilt;  /* Max. number of elements req'd */
	double   *ybuf  = (double *  )cpl_malloc(nelem * sizeof(double  ));
	double   *array = (double *  )cpl_malloc(nfilt * sizeof(double  ));
	cpl_size *point = (cpl_size *)cpl_malloc(nfilt * sizeof(cpl_size));

	/* set first and last edges equal */
	cpl_size il   = nfilt / 2;
	cpl_size ilow = (CPL_MAX(3, nfilt / 4) / 2) * 2 + 1;


	/* Calculation of xmns */
	for (cpl_size i = 0; i < ilow; i++) {
		array[i] = xbuf[i];
	}
	sort_array_index(array, ilow, point, HDRL_SORT_CPL_SIZE, CPL_SORT_ASCENDING);
	double xmns = array[ilow / 2];


	/* Calculation of xmnf */
	for (cpl_size i = 0; i < ilow; i++) {
		array[i] = xbuf[npt - 1 - i];
	}
	sort_array_index(array, ilow, point, HDRL_SORT_CPL_SIZE, CPL_SORT_ASCENDING);
	double xmnf = array[ilow / 2];


	/* reflect edges before filtering */
	for (cpl_size i = 0; i < il; i++) {
		ybuf[i]            = 2. * xmns - xbuf[il  - i - 1 + ilow];
		ybuf[npt + i + il] = 2. * xmnf - xbuf[npt - i - 1 - ilow];
	}

	for (cpl_size i = 0; i < npt; i++) {
		ybuf[i + il] = xbuf[i];
	}

	/* do median filtering on rest */
	for (cpl_size i = 0; i < nfilt; i++) {
		array[i] = ybuf[i];
		point[i] = i + 1;
	}

	sort_array_index(array, nfilt, point, HDRL_SORT_CPL_SIZE, CPL_SORT_ASCENDING);
	xbuf[0] = array[nfo2p1];

	cpl_size jl = nfilt;
	cpl_size jh = nfilt + npt - 1;

	cpl_size l = 0;
	for (cpl_size j = jl; j < jh; j++) {

		for (cpl_size i = 0; i < nfilt; i++) {
			if (point[i] != 1) {
				point[i]--;
			} else {
				point[i] = nfilt;
				array[i] = ybuf[j];
				l = i;
			}
		}

		cpl_size jj = nfilt;
		for (cpl_size i = 0; i < nfilt; i++) {
			if (i != l && array[l] <= array[i]) {
				jj = i;
				break;
			}
		}

		if (jj - 1 != l) {

			double   temp = array[l];
			cpl_size it   = point[l];

			if (jj < l) {

				for (cpl_size i = 0; i < l - jj; i++) {
					cpl_size ii = l - i - 1;
					array[ii + 1] = array[ii];
					point[ii + 1] = point[ii];
				}

			} else if (jj > l) {

				jj--;
				if (npt != 0) {
					for (cpl_size i = 0; i < jj - l; i++) {
						cpl_size ii = l + i + 1;
						array[ii - 1] = array[ii];
						point[ii - 1] = point[ii];
					}
				}
			}

			array[jj] = temp;
			point[jj] = it;
		}

		xbuf[j - jl + 1] = array[nfo2p1];
	}

	/* Free temporary arrays */
	cpl_free((void *) point);
	cpl_free((void *) array);
	cpl_free((void *) ybuf);

	return CPL_ERROR_NONE;
}

/**@}*/
