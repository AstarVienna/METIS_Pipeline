/*
 * This file is part of the irplib package
 * Copyright (C) 2002,2003,2014 European Southern Observatory
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "irplib_ppm.h"
#include "irplib_wlxcorr.h"
#include "irplib_spectrum.h"

#include <math.h>
#include <cpl.h>

/*-----------------------------------------------------------------------------
                                   Private functions
 -----------------------------------------------------------------------------*/
#ifdef IRPLIB_PPM_USE_METHOD2
static cpl_vector * irplib_ppm_convolve_line(const cpl_vector *, double,double);
static cpl_vector * irplib_ppm_detect_lines(const cpl_vector *, double) ;
#endif

/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_ppm     Point pattern matching
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*----------------------------------------------------------------------------*/
/**
  @brief    The Wavelength Calibration using PPM
  @param    spectrum        The spectrum vector
  @param    lines_catalog   The lines catalog
  @param    poly_init       Polynomial with the initial guess
  @param    slitw           The slit width
  @param    fwhm            The spectral FWHM [pixel]
  @param    thresh          The threshold for lines detection
  @param    degree          The polynomial degree
  @param    doplot          Plotting level (zero for none)
  @param    tab_infos       The computed solution table or NULL (computed) 
  @return   the polynomial solution or NULL in error case

  The returned table must be deallocated with cpl_table_delete().
  The returned polynomial must be deallocated with cpl_polynomial_delete().
 */
/*----------------------------------------------------------------------------*/
cpl_polynomial * irplib_ppm_engine(
        const cpl_vector        *   spectrum,
        const cpl_bivector      *   lines_catalog,
        const cpl_polynomial    *   poly_init,
        double                      slitw,
        double                      fwhm,
        double                      thresh,
        int                         degree,
        int                         doplot,
        cpl_table               **  tab_infos)
{
#ifdef IRPLIB_PPM_USE_METHOD2
    cpl_vector      *   spec_conv ;
#endif
    int                 spec_sz ;
    cpl_vector      *   det_lines ;
    cpl_vector      *   cat_lines ;
    double          *   pcat_lines ;
    double              wmin, wmax ;
    double              disp_min, disp_max, disp ;
    int                 nlines_cat, nlines ;
    const double    *   plines_catalog_x ;
    const double    *   plines_catalog_y ;
    cpl_bivector    *   matched ;
    cpl_matrix      *   matchedx;
    int                 match_sz;
    cpl_polynomial  *   fitted ;
    cpl_table       *   spc_table ;
    const cpl_vector*   vectors_plot[3];
    cpl_vector      *   plot_y ;
    int                 start_ind, stop_ind ;
    double              fill_val ;
    cpl_size            deg_loc ;
    int                 i ;
    cpl_error_code      error;

    /* Check entries */
    if (spectrum == NULL) return NULL ;
    if (lines_catalog == NULL) return NULL ;
    if (poly_init == NULL) return NULL ;

    /* Initialise */
    spec_sz = cpl_vector_get_size(spectrum) ;
    deg_loc = (cpl_size)degree ;
   
#ifdef IRPLIB_PPM_USE_METHOD2
    /* METHOD 2 */
    /* Correlate the spectrum with the line profile */
    if ((spec_conv = irplib_ppm_convolve_line(spectrum, slitw, fwhm)) == NULL) {
        cpl_msg_error(cpl_func, "Cannot convolve the signal") ;
        return NULL ;
    }
   
    /* Apply the lines detection */
    if ((det_lines = irplib_ppm_detect_lines(spec_conv, 0.9)) == NULL) {
        cpl_msg_error(cpl_func, "Cannot detect lines") ;
        cpl_vector_delete(spec_conv) ;
        return NULL ;
    }
    cpl_vector_delete(spec_conv) ;
#else
    /* METHOD 1 */
    if ((det_lines = irplib_spectrum_detect_peaks(spectrum, fwhm,
                    thresh, 0, NULL, NULL)) == NULL) {
        cpl_msg_error(cpl_func, "Cannot convolve the signal") ;
        return NULL ;
    }
#endif
    cpl_msg_info(cpl_func, "Detected %"CPL_SIZE_FORMAT" lines", 
            cpl_vector_get_size(det_lines));
 
    /* Get the catalog lines */
    wmin = cpl_polynomial_eval_1d(poly_init, 1.0, NULL) ;
    wmax = cpl_polynomial_eval_1d(poly_init, spec_sz, NULL) ;
    plines_catalog_x = cpl_bivector_get_x_data_const(lines_catalog) ;
    plines_catalog_y = cpl_bivector_get_y_data_const(lines_catalog) ;
    nlines = cpl_bivector_get_size(lines_catalog) ;
    nlines_cat = 0 ;
    start_ind = stop_ind = -1 ;
    for (i=0 ; i<nlines ; i++) {
        if (plines_catalog_x[i] > wmin && plines_catalog_x[i] < wmax &&
                plines_catalog_y[i] > 0.0) {
            nlines_cat++ ;
            if (start_ind<0) start_ind = i ;
            stop_ind = i ;
        }
    }
    if (nlines_cat == 0) {
        cpl_msg_error(cpl_func, "No lines in catalog") ;
        cpl_vector_delete(det_lines) ;
        return NULL ;
    }
    cat_lines = cpl_vector_new(nlines_cat) ;
    pcat_lines = cpl_vector_get_data(cat_lines) ;
    nlines_cat = 0 ;
    for (i=0 ; i<nlines ; i++) {
        if (plines_catalog_x[i] > wmin && plines_catalog_x[i] < wmax &&
                plines_catalog_y[i] > 0.0) {
            pcat_lines[nlines_cat] = plines_catalog_x[i] ; 
            nlines_cat++ ;
        }
    }
 
    /* Plot inputs */
    if (doplot) {
        double * pdet_lines ;

        /* Catalog */
        irplib_wlxcorr_catalog_plot(lines_catalog, wmin, wmax) ;

        /* Spectrum with detected lines */
        fill_val = cpl_vector_get_max(spectrum) ;
        plot_y = cpl_vector_new(spec_sz);
        cpl_vector_fill(plot_y, 0.0) ;
        pdet_lines = cpl_vector_get_data(det_lines) ;
        for (i=0 ; i<cpl_vector_get_size(det_lines) ; i++) {
            cpl_vector_set(plot_y, (int)pdet_lines[i], fill_val) ;
        }
        vectors_plot[0] = NULL ;
        vectors_plot[1] = spectrum ;
        vectors_plot[2] = plot_y ;

        cpl_plot_vectors("set grid;set xlabel 'Position (Pixel)';set ylabel "
                            "'Intensity (ADU/sec)';",
                            "t 'Spectrum with detected lines' w lines", "",
                            vectors_plot, 3);
        cpl_vector_delete(plot_y) ;
    }
   
    /* Apply the point pattern matching */
    disp = (wmax-wmin) / spec_sz ;
    disp_min = disp - (disp/10) ;
    disp_max = disp + (disp/10) ;
    matched = cpl_ppm_match_positions(det_lines, cat_lines, disp_min,
                                      disp_max, 0.05, NULL, NULL);
    cpl_vector_delete(det_lines) ;
    cpl_vector_delete(cat_lines) ;

    if (matched == NULL) {
        cpl_msg_error(cpl_func, "Cannot apply the point pattern matching") ;
        return NULL ;
    }

    match_sz = cpl_bivector_get_size(matched);

    cpl_msg_info(cpl_func, "Matched %d lines", match_sz) ;

    if (match_sz <= deg_loc) {
        cpl_msg_error(cpl_func, "Not enough match for the fit") ;
        cpl_bivector_delete(matched) ;
        return NULL ;
    }
    
    /* Plot if requested */
    if (doplot) {
        const double    *   pmatched ;
        cpl_bivector    *   biplot ;
        cpl_vector      *   plot_cat_x ;
        cpl_vector      *   plot_cat_y ;
        /* Spectrum with matched lines */
        fill_val = cpl_vector_get_max(spectrum) ;
        plot_y = cpl_vector_new(spec_sz);
        cpl_vector_fill(plot_y, 0.0) ;
        pmatched = cpl_bivector_get_x_data_const(matched) ;
        for (i=0 ; i < match_sz; i++) {
            cpl_vector_set(plot_y, (int)pmatched[i], fill_val) ;
        }
        vectors_plot[0] = NULL ;
        vectors_plot[1] = spectrum ;
        vectors_plot[2] = plot_y ;

        cpl_plot_vectors("set grid;set xlabel 'Position (Pixel)';set ylabel "
                            "'Intensity (ADU/sec)';",
                            "t 'Spectrum with matched lines' w lines", "",
                            vectors_plot, 3);
        cpl_vector_delete(plot_y) ;

        /* Catalog with matched lines */
        plot_cat_x=cpl_vector_extract(cpl_bivector_get_x_const(lines_catalog), 
                start_ind, stop_ind, 1) ;
        plot_cat_y=cpl_vector_extract(cpl_bivector_get_y_const(lines_catalog), 
                start_ind, stop_ind, 1) ;
        biplot = cpl_bivector_wrap_vectors(plot_cat_x, plot_cat_y) ;
        cpl_plot_bivector("set grid;set xlabel 'Wavelength';set ylabel "
                             "'Emission';", "t 'Catalog' w impulses", "",
                             biplot);
        cpl_bivector_unwrap_vectors(biplot) ;

        plot_y = cpl_vector_duplicate(plot_cat_y) ;
        cpl_vector_fill(plot_y, 0.0) ;
        pmatched = cpl_bivector_get_y_data_const(matched) ;
        fill_val=cpl_vector_get_mean(plot_cat_y) ;
        for (i=0 ; i < match_sz; i++) {
            int wl_ind = 0 ;
            while (pmatched[i] > cpl_vector_get(plot_cat_x, wl_ind) 
                    && wl_ind < spec_sz) wl_ind++ ;
            if (wl_ind < spec_sz) cpl_vector_set(plot_y, wl_ind, fill_val) ;
        }
        biplot = cpl_bivector_wrap_vectors(plot_cat_x, plot_y) ;
        cpl_plot_bivector("set grid;set xlabel 'Wavelength';set ylabel "
                             "'Emission';", "t 'Catalog (matched lines)' w "
                             "impulses", "", biplot) ;
        cpl_bivector_unwrap_vectors(biplot) ;
        cpl_vector_delete(plot_cat_x) ;
        cpl_vector_delete(plot_cat_y) ;
        cpl_vector_delete(plot_y) ;
    }
    
    /* Apply the fit */
    matchedx = cpl_matrix_wrap(1, match_sz, cpl_bivector_get_x_data(matched));
    fitted = cpl_polynomial_new(1);
    error = cpl_polynomial_fit(fitted, matchedx, NULL,
                               cpl_bivector_get_y_const(matched), NULL,
                               CPL_FALSE, NULL, &deg_loc);
    cpl_bivector_delete(matched);
    (void)cpl_matrix_unwrap(matchedx);
    if (error) {
        cpl_msg_error(cpl_func, "Cannot fit the polynomial") ;
        cpl_polynomial_delete(fitted);
        return NULL ;
    }
   
    /* Create the infos table */
    if ((spc_table = irplib_wlxcorr_gen_spc_table(spectrum,
                    lines_catalog, slitw, fwhm, poly_init, fitted)) == NULL) {
        cpl_msg_error(cpl_func, "Cannot generate the infos table") ;
        cpl_polynomial_delete(fitted) ;
        return NULL ;
    }
    if (tab_infos != NULL) *tab_infos = spc_table ;
    else cpl_table_delete(spc_table) ;
    return fitted ;
}

/**@}*/

#ifdef IRPLIB_PPM_USE_METHOD2
/*----------------------------------------------------------------------------*/
/**
  @brief    Spectrum convolution with a line profile
  @param    spectrum        The spectrum vector
  @param    slitw           The slit width
  @param    fwhm            The spectral FWHM [pixel]
  @param    doplot          Plotting level (zero for none)
  @return   the convolved spectrum

  The returned vector must be deallocated with cpl_vector_delete().
 */
/*----------------------------------------------------------------------------*/
static cpl_vector * irplib_ppm_convolve_line(
        const cpl_vector        *   spectrum,
        double                      slitw,
        double                      fwhm)
{
    cpl_vector  *   conv_kernel ;
    cpl_vector  *   line_profile ;
    cpl_vector  *   xcorrs ;
    cpl_vector  *   xc_single ;
    int             hs, line_sz, sp_sz ;
    int             i ;

    /* Test entries */
    if (spectrum == NULL) return NULL ;

    /* Create the convolution kernel */
    if ((conv_kernel = irplib_wlxcorr_convolve_create_kernel(slitw,
                    fwhm)) == NULL) {
        cpl_msg_error(cpl_func, "Cannot create kernel") ;
        return NULL ;
    }
    hs = cpl_vector_get_size(conv_kernel) ;
    line_sz = 2 * hs + 1 ;
    
    /* Create the line profile */
    line_profile = cpl_vector_new(line_sz) ;
    cpl_vector_fill(line_profile, 0.0) ;
    cpl_vector_set(line_profile, hs, 1.0) ;
    if (irplib_wlxcorr_convolve(line_profile, conv_kernel) != 0) {
        cpl_msg_error(cpl_func, "Cannot create line profile") ;
        cpl_vector_delete(line_profile) ;
        cpl_vector_delete(conv_kernel) ;
        return NULL ;
    }
    cpl_vector_delete(conv_kernel) ;
    
    /* Create the correlations values vector */
    sp_sz = cpl_vector_get_size(spectrum) ;
    xcorrs = cpl_vector_new(sp_sz) ;
    cpl_vector_fill(xcorrs, 0.0) ;
    xc_single = cpl_vector_new(1) ;

    /* Loop on the pixels of the spectrum */
    for (i=hs ; i<sp_sz-hs ; i++) {
        cpl_vector * spec_ext ;
        /* Extract the current spectrum part */
        if ((spec_ext = cpl_vector_extract(spectrum, i-hs, i+hs, 1)) == NULL) {
            cpl_msg_error(cpl_func, "Cannot extract spectrum") ;
            cpl_vector_delete(xc_single) ;
            cpl_vector_delete(line_profile) ;
            return NULL ;
        }
        if (cpl_vector_correlate(xc_single, spec_ext, line_profile) < 0) {
            cpl_msg_error(cpl_func, "Cannot correlate") ;
            cpl_vector_delete(xc_single) ;
            cpl_vector_delete(line_profile) ;
            cpl_vector_delete(spec_ext) ;
            return NULL ;
        }
        cpl_vector_set(xcorrs, i, cpl_vector_get(xc_single, 0)) ;
        cpl_vector_delete(spec_ext) ;
    }
    cpl_vector_delete(xc_single) ;
    cpl_vector_delete(line_profile) ;

    return xcorrs ;
} 

/*----------------------------------------------------------------------------*/
/**
  @brief    Detect lines in a vector
  @param    spectrum        The spectrum vector
  @param    threshold       The threshold for line detection
  @return   the detected lines or NULL on error

  The returned vector must be deallocated with cpl_vector_delete().
 */
/*----------------------------------------------------------------------------*/
static cpl_vector * irplib_ppm_detect_lines(
        const cpl_vector    *   spec,
        double                  threshold)
{
    cpl_vector  *   spec_loc ;
    double      *   pspec_loc ;
    cpl_vector  *   lines ;
    double      *   plines ;
    int             spec_loc_sz, nlines ;
    double          max ;
    int             i ;

    /* Test inputs */
    if (spec == NULL) return NULL ;

    /* Local spectrum */
    spec_loc = cpl_vector_duplicate(spec) ;
    pspec_loc = cpl_vector_get_data(spec_loc) ;
    spec_loc_sz = cpl_vector_get_size(spec_loc) ;

    /* Threshold the local spectrum */
    for (i=0 ; i<spec_loc_sz ; i++) 
        if (pspec_loc[i] < threshold) pspec_loc[i] = 0.0 ;
    
    /* Allocate lines container */
    lines = cpl_vector_new(spec_loc_sz) ;
    plines = cpl_vector_get_data(lines) ;
    nlines = 0 ;
    
    /* Loop as long as there are lines */
    while ((max = cpl_vector_get_max(spec_loc)) > threshold) {
        /* Find the max position */
        int max_ind = 0 ;
        while (max_ind < spec_loc_sz && pspec_loc[max_ind] < max) max_ind++ ;
        if (max_ind == spec_loc_sz) {
            cpl_msg_error(cpl_func, "Cannot find maximum") ;
            cpl_vector_delete(spec_loc) ;
            cpl_vector_delete(lines) ;
            return NULL ;
        }
        if (max_ind == 0 || max_ind == spec_loc_sz-1) {
            pspec_loc[max_ind] = 0 ;
            continue ;
        }

        /* Get the precise position from the neighbours values */
        plines[nlines] =    pspec_loc[max_ind] * max_ind + 
                            pspec_loc[max_ind-1] * (max_ind-1) +
                            pspec_loc[max_ind+1] * (max_ind+1) ; 
        plines[nlines] /= pspec_loc[max_ind] + pspec_loc[max_ind+1] +
            pspec_loc[max_ind-1] ;
        plines[nlines] ++ ;
        nlines ++ ;

        /* Clean the line */
        i = max_ind ;
        while (i>=0 && pspec_loc[i] > threshold) {
            pspec_loc[i] = 0.0 ;
            i-- ;
        }
        i = max_ind+1 ;
        while (i<spec_loc_sz && pspec_loc[i] > threshold) {
            pspec_loc[i] = 0.0 ;
            i++ ;
        }
    }
    cpl_vector_delete(spec_loc) ;
   
    /* Check if there are lines */
    if (nlines == 0) {
        cpl_msg_error(cpl_func, "Cannot detect any line") ;
        cpl_vector_delete(lines) ;
        return NULL ;
    }
    
    /* Resize the vector */
    cpl_vector_set_size(lines, nlines) ;

    /* Sort the lines */
    cpl_vector_sort(lines, 1) ;
    
    return lines ;
}

#endif
