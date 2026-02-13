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

#include "hdrl_cat_filter.h"

#include "hdrl_cat_statistics.h"


/*** Prototypes ***/

static void filt1d( double ybuf[], cpl_size mpt, cpl_size mfilt);
static void hanning(double xbuf[], cpl_size npt);


/*---------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_filter  hdrl_filter
 * @ingroup  Catalogue
 *
 * @brief    Do bilinear median and linear filtering on background values
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Do bilinear median and linear filtering on background values
 *
 * @param    xbuf      The input map to be smoothed
 * @param    nx        The X dimension of the map
 * @param    ny        The Y dimension of the map
 *
 * @return   CPL_ERROR_NONE if all went well (Currently it's the only value).
 *
 * Description:
 * 	    A map is smoothed using sliding median and mean filters.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_bfilt(double **xbuf, cpl_size nx, cpl_size ny)
{
	/* Allocate temporary storage */
	double *ybuf = (double *)cpl_malloc(CPL_MAX(nx, ny) * sizeof(double));
	double *save = (double *)cpl_malloc((nx + 1) * ny   * sizeof(double));

	cpl_size mfilt = 5;

	/* median filter across */
	for (cpl_size k = 0; k < ny; k++) {

		for (cpl_size j = 0; j < nx; j++) {

			save[(nx + 1) * k + j] = xbuf[k][j];

			ybuf[j] = xbuf[k][j];
		}

		filt1d(ybuf, nx, mfilt);

		for (cpl_size j = 0; j < nx; j++) {
			xbuf[k][j] = ybuf[j];
		}
	}

	/* and now down */
	for (cpl_size k = 0; k < nx; k++) {

		for (cpl_size j = 0; j < ny; j++) {
			ybuf[j] = xbuf[j][k];
		}

		filt1d(ybuf, ny, mfilt);

		for (cpl_size j = 0; j < ny; j++) {

			/* make sure median filtered values are not large than original */
			if (save[(nx + 1) * j + k] > -1000.) {
				xbuf[j][k] = CPL_MIN(save[(nx + 1) * j + k], ybuf[j]);
			}
		}
	}

	/* now repeat with linear filters across */
	for (cpl_size k = 0; k < ny; k++) {

		for (cpl_size j = 0; j < nx; j++) {
			ybuf[j] = xbuf[k][j];
		}

		hanning(ybuf, nx);

		for (cpl_size j = 0; j < nx; j++) {
			xbuf[k][j] = ybuf[j];
		}
	}

	/* and now down */
	for (cpl_size k = 0; k < nx; k++) {

		for (cpl_size j = 0; j < ny; j++) {
			ybuf[j] = xbuf[j][k];
		}

		hanning(ybuf, ny);

		for (cpl_size j = 0; j < ny; j++) {
			xbuf[j][k] = ybuf[j];
		}
	}

	/* Free temporary storage */
	cpl_free((void *)ybuf);
	cpl_free((void *)save);

	return CPL_ERROR_NONE;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Pads out array with missing points and linearly extrapolates the ends
 *
 * @param    x         .
 * @param    n         .
 *
 * Description:
 * 	    .Pads out array with missing points and linearly extrapolates the ends
 */
/* ---------------------------------------------------------------------------*/
void padext(double x[], cpl_size n)
{
	/* elements <= -1000.0 are treated as missing */
	cpl_size ii = 0;
	while (ii < n && x[ii] <= -1000.) {
		ii++;
	}
	cpl_size ilow = ii;
	cpl_size ihih = 0;

	for (cpl_size i = ilow + 1; i < n; i++){

		if (x[i] <= -1000.) {

			cpl_size ic = 1;
			if (i < n - 1) {

				while (x[i + ic] <= -1000.) {

					ic++;

					if (i + ic >= n - 1) {
						break;
					}
				}
			}

			if (i + ic < n - 1){

				double xlow = x[i - 1];
				double xhih = x[i + ic];

				for (cpl_size j = 0; j < ic; j++){

					double t2 = ((double)j + 1.) / ((double)ic + 1.);
					double t1 = 1. - t2;

					x[i + j] = t1 * xlow + t2 * xhih;
				}
			}

		} else {

			ihih = i;
		}
	}


	/* linear extrapolation of ends */
	if (ilow > 0 && ilow < n){

		double slope;
		if (ilow < n - 1) {
			slope = x[ilow + 1] - x[ilow];
	    } else {
			slope = 0.;
		}

		for (cpl_size i = 0; i < ilow; i++) {
			x[i] = x[ilow] - slope * (ilow - i);
		}
	}

	if (ihih < n - 1) {

		double slope;
		if (ihih > 0) {
			slope = x[ihih] - x[ihih - 1];
		} else {
			slope = 0.;
		}

		for (cpl_size i = ihih + 1; i < n; i++) {
			x[i] = x[ihih] + slope * (i - ihih);
		}
	}
}

/**@}*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Does median filtering allowing for unmeasured entries
 *
 * @param    ybuf      .
 * @param    mpt       .
 * @param    mfilt     .
 *
 * Description:
 * 	    .
 */
/* ---------------------------------------------------------------------------*/
static void filt1d(double ybuf[], cpl_size mpt, cpl_size mfilt)
{
	/* Allocate temporary storage */
	double *wbuf = (double *)cpl_malloc(mpt * sizeof(double));

	cpl_size irc = 0;
	for (cpl_size i = 0; i < mpt; i++){
		if (ybuf[i] > -1000.){
			wbuf[irc] = ybuf[i];
			irc++;
		}
	}

	if (irc != 0) {

		hdrl_median(wbuf, irc, mfilt);

		irc = 0;
		for (cpl_size i = 0; i < mpt; i++){
			if (ybuf[i] > -1000.){
				ybuf[i] = wbuf[irc];
				irc++;
			}
		}

		padext(ybuf, mpt);
	}

	/* Free temporary storage */
	cpl_free((void *)wbuf);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Performs linear filtering on array xbuf
 *
 * @param    xbuf      .
 * @param    npt       .
 *
 * Description:
 * 	    .
 */
/* ---------------------------------------------------------------------------*/
static void hanning(double xbuf[], cpl_size npt)
{
	cpl_size nfilt = 3;
	if (npt <= nfilt) {
		return;
	}

	/* set first and last edges equal */
	cpl_size il = nfilt / 2;

	cpl_size ilow = CPL_MAX(3, nfilt / 4);
	ilow = (ilow / 2) * 2 + 1;

	double sum = 0.;
	for (cpl_size i = 0; i < ilow; i++) {
		sum += xbuf[i];
	}
	double xmns = sum / (double)ilow;

	sum = 0.;
	for (cpl_size i = 0; i < ilow; i++) {
		sum += xbuf[npt - 1 - i];
	}
	double xmnf = sum / (double)ilow;

	/* allocate ybuf array */
	cpl_size nelem = npt + nfilt;  /* Max. number of elements req'd */
	double   *ybuf = (double *)cpl_malloc(nelem * sizeof(double));

	/* reflect edges before filtering */
	for (cpl_size i = 0; i < il; i++) {
		ybuf[i]            = 2. * xmns - xbuf[il + ilow - 1 - i];
		ybuf[npt + i + il] = 2. * xmnf - xbuf[npt - i - ilow - 1];
	}

	for (cpl_size i = 0; i < npt; i++) {
		ybuf[i + il] = xbuf[i];
	}

	/* do linear filtering on rest */
	for (cpl_size i = 0; i < npt; i++) {
		/* 1-2-1 Hanning weighting */
		xbuf[i] = 0.25 * (ybuf[i] + 2. * ybuf[i + 1] + ybuf[i + 2]);
	}

	/* clean up */
	cpl_free((void *) ybuf);
}
