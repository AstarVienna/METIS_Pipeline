/* $Id: irplib_wlxcorr.h,v 1.22 2013-01-29 08:43:33 jtaylor Exp $
 *
 * This file is part of the IRPLIB package
 * Copyright (C) 2002,2003 European Southern Observatory
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
 * $Revision: 1.22 $
 * $Name: not supported by cvs2svn $
 */

#ifndef IRPLIB_WLXCORR_H
#define IRPLIB_WLXCORR_H

/*-----------------------------------------------------------------------------
                                Include
 -----------------------------------------------------------------------------*/

#include <cpl.h>

/*-----------------------------------------------------------------------------
                                    Define
 -----------------------------------------------------------------------------*/

/* The 4 columns of the table */
#define IRPLIB_WLXCORR_COL_WAVELENGTH   "Wavelength"
#define IRPLIB_WLXCORR_COL_CAT_INIT     "Catalog Initial"
#define IRPLIB_WLXCORR_COL_CAT_FINAL    "Catalog Corrected"
#define IRPLIB_WLXCORR_COL_OBS          "Observed"

/*-----------------------------------------------------------------------------
                                Functions prototypes
 -----------------------------------------------------------------------------*/

int irplib_wlxcorr_plot_spc_table(const cpl_table *, const char *, int, int) ;
int irplib_wlxcorr_catalog_plot(const cpl_bivector *, double, double) ;
int irplib_wlxcorr_plot_solution(const cpl_polynomial *, const cpl_polynomial *,
        const cpl_polynomial *, int, int) ;

cpl_polynomial * irplib_wlxcorr_best_poly(const cpl_vector *, 
        const cpl_bivector *, int, const cpl_polynomial *, const cpl_vector *, 
        int, double, double, double *, cpl_table **, cpl_vector **) ;

/*
TODO :
    Merge irplib_wlxcorr_best_poly_prop() with irplib_wlxcorr_best_poly() by
    adding a new parameter.
    Need to coordinate with all pipelines.
*/
cpl_polynomial * irplib_wlxcorr_best_poly_prop(const cpl_vector     * spectrum,
                                          const cpl_bivector   * lines_catalog,
                                          int                    degree,
                                          const cpl_polynomial * guess_poly,
                                          const cpl_vector     * wl_error,
                                          int                    nsamples,
                                          double                 slitw,
                                          double                 fwhm,
                                          double               * xc,
                                          cpl_table           ** wlres,
                                          cpl_vector          ** xcorrs) ;

cpl_table * irplib_wlxcorr_gen_spc_table(const cpl_vector *, 
        const cpl_bivector *, double, double, const cpl_polynomial *,
        const cpl_polynomial *) ;
cpl_bivector * irplib_wlxcorr_cat_extract(const cpl_bivector *, double, double);
cpl_vector * irplib_wlxcorr_convolve_create_kernel(double, double) ;
int irplib_wlxcorr_convolve(cpl_vector *,const cpl_vector *) ;


cpl_error_code irplib_wlxcorr_vector_fill_line_spectrum(cpl_vector *,
                                                const cpl_polynomial *,
                                                const cpl_bivector *,
                                                double, double, double,
                                                int);

#endif
