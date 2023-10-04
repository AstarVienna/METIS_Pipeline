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

#include "irplib_wlxcorr.h"
#include "irplib_spectrum.h"

#include <math.h>
#include <float.h>
#include <cpl.h>

/*-----------------------------------------------------------------------------
                                   Define
 -----------------------------------------------------------------------------*/

#define SPECTRUM_HW                     16
#define MIN_THRESH_FACT                 0.9
#define MAX_THRESH_FACT                 1.1
#define SPEC_SHADOW_FACT                30.0 /* Negative spectrum intensity*/
#define SPEC_MAXWIDTH                   48

/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

static int select_valid_spectra(cpl_image *, cpl_apertures *, int,
        spec_shadows, int, int *, int **) ;
static int valid_spectrum(cpl_image *, cpl_apertures *, int, spec_shadows, int,
        int) ;

/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_spectrum     Functions for LSS spectra
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Finds the brightest spectrum in an image 
  @param    in                  spectral image with spectra 
  @param    offset              the diff. between pos. and neg. spectra
  @param    shadows             the spectral shadows
  @param    min_bright          min. bright. required for a spectrum
  @param    orient              1 for vertical spec. 0 for horizontal ones
  @param    pos                 the computed spectrum position (1->npix)
  @return   int 0 if ok, -1 in error case 

  Finds the brightest spectrum in an image by collapsing the image orthogonally
  to the spectrum orientation.
  Spectra are assumed to be horizontal for orient==0, vertical for 1
*/
/*----------------------------------------------------------------------------*/
int irplib_spectrum_find_brightest(
        const cpl_image     *   in,
        int                     offset,
        spec_shadows            shadows,
        double                  min_bright,
        int                     orient,
        double              *   pos)
{
    cpl_image       *   loc_ima ;
    cpl_image       *   filt_image ;
    cpl_image       *   collapsed ;
    float           *   pcollapsed ;
    cpl_vector      *   line ;
    double          *   pline ;
    cpl_vector      *   line_filt ;
    double              threshold ;
    double              median, stdev, max, mean ;
    cpl_mask        *   mask ;
    cpl_image       *   labels ;
    cpl_size            nlabels ;
    cpl_apertures   *   aperts ;
    int                 n_valid_specs ;
    int             *   valid_specs ;
    double              brightness ;
    int                 i ;

    /* Test entries */
    if (in == NULL) return -1 ;
    if (orient!=0 && orient!=1) return -1 ;

    /* Flip the image if necessary */
    if (orient == 1) {
        loc_ima = cpl_image_duplicate(in) ;
        cpl_image_flip(loc_ima, 1) ;
    } else {
        loc_ima = cpl_image_duplicate(in) ;
    }

    /* Median vertical filtering 3x3 */
    mask = cpl_mask_new(3, 3) ;
    cpl_mask_not(mask) ;
    filt_image = cpl_image_new(
            cpl_image_get_size_x(loc_ima),
            cpl_image_get_size_y(loc_ima),
            cpl_image_get_type(loc_ima)) ;
    if (cpl_image_filter_mask(filt_image, loc_ima, mask,
                CPL_FILTER_MEDIAN, CPL_BORDER_FILTER) != CPL_ERROR_NONE) {
        cpl_msg_error(__func__, "Cannot filter the image") ;
        cpl_mask_delete(mask) ;
        cpl_image_delete(filt_image) ;
        return -1 ;
    }
    cpl_mask_delete(mask) ;
    cpl_image_delete(loc_ima) ;

    /* Collapse the image */
    if ((collapsed = cpl_image_collapse_median_create(filt_image, 1, 0,
                    0)) == NULL) {
        cpl_msg_error(cpl_func, "collapsing image: aborting spectrum detection");
        cpl_image_delete(filt_image) ;
        return -1 ;
    }
    cpl_image_delete(filt_image) ;

    /* Subtract low frequency signal */
    line = cpl_vector_new_from_image_column(collapsed, 1) ;
    cpl_image_delete(collapsed) ;
    line_filt = cpl_vector_filter_median_create(line, SPECTRUM_HW) ;
    cpl_vector_subtract(line, line_filt) ;
    cpl_vector_delete(line_filt) ;

    /* Get relevant stats for thresholding */
    median = cpl_vector_get_median_const(line) ;
    stdev = cpl_vector_get_stdev(line) ;
    max = cpl_vector_get_max(line) ;
    mean = cpl_vector_get_mean(line) ;

    /* Set the threshold */
    threshold = median + stdev ;
    if (threshold > MIN_THRESH_FACT * max)  threshold = MIN_THRESH_FACT * max ;
    if (threshold < MAX_THRESH_FACT * mean) threshold = MAX_THRESH_FACT * mean;

    /* Recreate the image */
    collapsed = cpl_image_new(1, cpl_vector_get_size(line), CPL_TYPE_FLOAT) ;
    pcollapsed = cpl_image_get_data_float(collapsed) ;
    pline = cpl_vector_get_data(line) ;
    for (i=0 ; i<cpl_vector_get_size(line) ; i++)
        pcollapsed[i] = (float)pline[i] ;
    cpl_vector_delete(line) ;

    /* Binarise the image */
    if ((mask = cpl_mask_threshold_image_create(collapsed, threshold,
            DBL_MAX)) == NULL) {
        cpl_msg_error(cpl_func, "cannot binarise") ;
        cpl_image_delete(collapsed) ;
        return -1 ;
    }
    if (cpl_mask_count(mask) < 1) {
        cpl_msg_error(cpl_func, "not enough signal to detect spectra") ;
        cpl_image_delete(collapsed) ;
        cpl_mask_delete(mask) ;
        return -1 ;
    }
    /* Labelise the different detected apertures */
    if ((labels = cpl_image_labelise_mask_create(mask, &nlabels))==NULL) {
        cpl_msg_error(cpl_func, "cannot labelise") ;
        cpl_image_delete(collapsed) ;
        cpl_mask_delete(mask) ;
        return -1 ;
    }
    cpl_mask_delete(mask) ;

    /* Create the detected apertures list */
    if ((aperts = cpl_apertures_new_from_image(collapsed, labels)) == NULL) {
        cpl_msg_error(cpl_func, "cannot compute apertures") ;
        cpl_image_delete(collapsed) ;
        cpl_image_delete(labels) ;
        return -1 ;
    }
    cpl_image_delete(labels) ;

    /* Select only relevant specs, create corresponding LUT's */
    if (select_valid_spectra(collapsed, aperts, offset, shadows, SPEC_MAXWIDTH,
                &n_valid_specs, &valid_specs) == -1) {
        cpl_msg_debug(cpl_func, 
                "Could not select valid spectra from the %"CPL_SIZE_FORMAT
                " apertures in %"CPL_SIZE_FORMAT"-col 1D-image, offset=%d"
                ", min_bright=%d",
                      cpl_apertures_get_size(aperts),
                      cpl_image_get_size_y(collapsed), offset, SPEC_MAXWIDTH);
        if (cpl_msg_get_level() <= CPL_MSG_DEBUG)
            cpl_apertures_dump(aperts, stderr);
        cpl_image_delete(collapsed);
        cpl_apertures_delete(aperts);
        return -1;
    }
    cpl_image_delete(collapsed) ;
    if (n_valid_specs < 1) {
        cpl_msg_error(cpl_func, "no valid spectrum detected") ;
        cpl_free(valid_specs) ;
        cpl_apertures_delete(aperts) ;
        return -1 ;
    }

    /* Look for the brightest, among the detected spectra */
    *pos = cpl_apertures_get_centroid_y(aperts, valid_specs[0]+1) ;
    brightness = cpl_apertures_get_flux(aperts, valid_specs[0]+1) ;
    for (i=0 ; i<n_valid_specs ; i++) {
        if (cpl_apertures_get_flux(aperts, valid_specs[i]+1) > brightness) {
            *pos = cpl_apertures_get_centroid_y(aperts, valid_specs[i]+1) ;
            brightness = cpl_apertures_get_flux(aperts, valid_specs[i]+1) ;
        }
    }
    cpl_apertures_delete(aperts) ;
    cpl_free(valid_specs) ;

    /* Minimum brightness required */
    if (brightness < min_bright) {
        cpl_msg_error(cpl_func, "brightness %f too low <%f", brightness,
                min_bright) ;
        return -1 ;
    }

    /* Return */
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Detect the brightest features in a spectrum
  @param    in      the spectrum
  @param    fwhm    the FWHM used for the lines convolution
  @param    display the flag to display
  @param    fwhms   the fwhms of the detected lines
  @param    areas   the areas under the detected lines
  @return   The bright lines positions or NULL in error case

  The lines positions are in pixels (first pixel is 1)
 */
/*----------------------------------------------------------------------------*/
cpl_vector * irplib_spectrum_detect_peaks(
        const cpl_vector    *   in,
        int                     fwhm,
        double                  sigma,
        int                     display,
        cpl_vector          **  fwhms_out,
        cpl_vector          **  areas_out)
{
    cpl_vector      *   filtered ;
    cpl_vector      *   spec_clean ;
    cpl_vector      *   spec_convolved ;
    double          *   pspec_convolved ;
    int                 filt_size ;
    cpl_vector      *   big_detected ;
    cpl_vector      *   big_fwhms ;
    cpl_vector      *   big_area ;
    double          *   pbig_detected ;
    double          *   pbig_fwhms ;
    double          *   pbig_area ;
    cpl_vector      *   detected ;
    cpl_vector      *   fwhms ;
    cpl_vector      *   area ;
    double              max, med, stdev ;
    double              x0, sig, norm, offset ;
    int                 nb_det, nb_samples, hwidth, start, stop ;
    int                 i, j ;

    /* Test entries */
    if (in == NULL) return NULL ;

    /* Initialise */
    nb_samples = cpl_vector_get_size(in) ;
    filt_size = 5 ;
    hwidth = 5 ;

    /* Subtract the low frequency part */
    cpl_msg_debug(__func__, "Low Frequency signal removal") ;
    if ((filtered=cpl_vector_filter_median_create(in, filt_size))==NULL){
        cpl_msg_error(__func__, "Cannot filter the spectrum") ;
        return NULL ;
    }
    spec_clean = cpl_vector_duplicate(in) ;
    cpl_vector_subtract(spec_clean, filtered) ;
    cpl_vector_delete(filtered) ;

    /* Display if requested */
    if (display) {
        cpl_plot_vector(
    "set grid;set xlabel 'Position (pixels)';set ylabel 'Intensity (ADU)';",
        "t 'Filtered extracted spectrum' w lines", "", spec_clean);
    }

    /* Convolve */
    spec_convolved = cpl_vector_duplicate(spec_clean) ;
    if (fwhm > 0) {
        cpl_vector * conv_kernel ;
        cpl_msg_debug(__func__, "Spectrum convolution") ;
        /* Create convolution kernel */
        if ((conv_kernel = irplib_wlxcorr_convolve_create_kernel(fwhm,
                        fwhm)) == NULL) {
            cpl_msg_error(cpl_func, "Cannot create convolution kernel") ;
            cpl_vector_delete(spec_clean) ;
            cpl_vector_delete(spec_convolved) ;
            return NULL ;
        }

        /* Smooth the instrument resolution */
        if (irplib_wlxcorr_convolve(spec_convolved, conv_kernel)) {
            cpl_msg_error(cpl_func, "Cannot smoothe the signal");
            cpl_vector_delete(spec_clean) ;
            cpl_vector_delete(spec_convolved) ;
            cpl_vector_delete(conv_kernel) ;
            return NULL ;
        }
        cpl_vector_delete(conv_kernel) ;

        /* Display if requested */
        if (display) {
            cpl_plot_vector(
        "set grid;set xlabel 'Position (pixels)';set ylabel 'Intensity (ADU)';",
            "t 'Convolved extracted spectrum' w lines", "", spec_convolved);
        }
    }

    /* Apply the detection */
    big_detected = cpl_vector_duplicate(spec_convolved) ;
    big_fwhms = cpl_vector_duplicate(spec_convolved) ;
    big_area = cpl_vector_duplicate(spec_convolved) ;
    pbig_detected = cpl_vector_get_data(big_detected) ;
    pbig_fwhms = cpl_vector_get_data(big_fwhms) ;
    pbig_area = cpl_vector_get_data(big_area) ;
    
    pspec_convolved = cpl_vector_get_data(spec_convolved) ;

    /* To avoid detection on the side */
    pspec_convolved[0] = pspec_convolved[nb_samples-1] = 0.0 ;

    /* Compute stats */
    max     =   cpl_vector_get_max(spec_convolved) ;
    stdev   =   cpl_vector_get_stdev(spec_convolved) ;
    med     =   cpl_vector_get_median_const(spec_convolved) ;

    /* Loop on the detected lines */
    nb_det = 0 ;
    while (max > med + stdev * sigma) {
        cpl_vector * extract ;
        cpl_vector * extract_x ;
        double       cur_val ;

        /* Compute the position */
        i=0 ;
        while (pspec_convolved[i] < max) i++ ;
        if (i<=0 || i>=nb_samples-1) break ;

        /* Extract the line */
        if (i - hwidth >= 0)                start = i - hwidth ;
        else                                start = 0 ;
        if (i + hwidth <= nb_samples-1)     stop = i + hwidth ;
        else                                stop = nb_samples-1 ;
        extract = cpl_vector_extract(spec_clean, start, stop, 1) ;
        extract_x = cpl_vector_duplicate(extract) ;
        for (j=0 ; j<cpl_vector_get_size(extract_x) ; j++) {
            cpl_vector_set(extract_x, j, (double)j+1) ;
        }
        /* Fit the gaussian */
        if (cpl_vector_fit_gaussian(extract_x, NULL, extract, NULL, 
                    CPL_FIT_ALL, &x0, &sig, &norm, &offset, NULL, NULL, 
                    NULL) != CPL_ERROR_NONE) {
            cpl_msg_debug(__func__, 
                    "Cannot fit a gaussian at [%d, %d]",
                    start, stop) ;
            cpl_error_reset() ;
        } else {
            pbig_detected[nb_det] = x0+start ;
            pbig_area[nb_det] = norm ;
            pbig_fwhms[nb_det] = 2*sig*sqrt(2*log(2)) ;
            cpl_msg_debug(__func__, "Line nb %d at position %g",
                    nb_det+1, pbig_detected[nb_det]) ;
            nb_det ++ ;
        }
        cpl_vector_delete(extract) ;
        cpl_vector_delete(extract_x) ;

        /* Cancel out the line on the left */
        j = i-1 ;
        cur_val = pspec_convolved[i] ;
        while (j>=0 && pspec_convolved[j] < cur_val) {
            cur_val = pspec_convolved[j] ;
            pspec_convolved[j] = 0.0 ;
            j-- ;
        }
        /* Cancel out the line on the right */
        j = i+1 ;
        cur_val = pspec_convolved[i] ;
        while (j<=nb_samples-1 && pspec_convolved[j] < cur_val) {
            cur_val = pspec_convolved[j] ;
            pspec_convolved[j] = 0.0 ;
            j++ ;
        }
        /* Cancel out the line on center */
        pspec_convolved[i] = 0.0 ;

        /* Recompute the stats */
        max     =   cpl_vector_get_max(spec_convolved) ;
        stdev   =   cpl_vector_get_stdev(spec_convolved) ;
        med     =   cpl_vector_get_median_const(spec_convolved) ;
    }
    cpl_vector_delete(spec_convolved) ;
    cpl_vector_delete(spec_clean) ;

    /* Create the output vector */
    if (nb_det == 0) {
        detected = NULL ;
        area = NULL ;
        fwhms = NULL ;
    } else {
        double * pdetected ;
        double * pfwhms ;
        double * parea ;
        detected = cpl_vector_new(nb_det) ;
        area = cpl_vector_new(nb_det) ;
        fwhms = cpl_vector_new(nb_det) ;
        pdetected = cpl_vector_get_data(detected) ;
        parea = cpl_vector_get_data(area) ;
        pfwhms = cpl_vector_get_data(fwhms) ;
        for (i=0 ; i<nb_det ; i++) {
            pdetected[i] = pbig_detected[i] ;
            parea[i] = pbig_area[i] ;
            pfwhms[i] = pbig_fwhms[i] ;
        }
    }
    cpl_vector_delete(big_detected) ;
    cpl_vector_delete(big_area) ;
    cpl_vector_delete(big_fwhms) ;

    /* Return  */
    if (fwhms_out == NULL)  cpl_vector_delete(fwhms) ;
    else                    *fwhms_out = fwhms ;
    if (areas_out == NULL)  cpl_vector_delete(area) ;
    else                    *areas_out = area ;
    return detected ;
}

/**@}*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Selects the valid spectra in a spectral image
  @param    in          the 1d image
  @param    aperts      detected objects
  @param    offset      the distance to the two shadows of the bright spectrum
  @param    shadows     shadows mode
  @param    max_spec_width  maximal spectrum width
  @param    n_valid_specs   number of valid spectra
  @param    valid_specs     lut giving the object number of a found spectrum
  @return   0 if ok, -1 in error case
*/
/*----------------------------------------------------------------------------*/
static int select_valid_spectra(
        cpl_image       *   in,
        cpl_apertures   *   aperts,
        int                 offset,
        spec_shadows        shadows,
        int                 max_spec_width,
        int             *   n_valid_specs,
        int             **  valid_specs)
{
    int                 nb_aperts ;
    int                 i, j ;

    /* Initialise */
    *valid_specs = NULL ;
    nb_aperts = cpl_apertures_get_size(aperts) ;
    *n_valid_specs = 0 ;

    /* Test entries */
    if (nb_aperts < 1) return -1 ;

    /* Count nb of valid specs */
    j = 0 ;
    for (i=0 ; i<nb_aperts ; i++)
        if (valid_spectrum(in, aperts, offset, shadows, max_spec_width,
                    i+1)) (*n_valid_specs)++ ;

    /* Associate to each spectrum, its object number */
    if (*n_valid_specs) {
        *valid_specs = cpl_calloc(*n_valid_specs, sizeof(int)) ;
        j = 0 ;
        for (i=0 ; i<nb_aperts ; i++)
            if (valid_spectrum(in, aperts, offset, shadows, max_spec_width,
                        i+1)) {
                (*valid_specs)[j] = i ;
                j++ ;
            }
    } else return -1 ;

    return 0 ;
}

/*---------------------------------------------------------------------------*/
/**
  @brief    Helper function to select_valid_spectra 
  @param    in          the 1d image
  @param    aperts      detected objects
  @param    offset      the distance to the two shadows of the bright spectrum
  @param    shadows     shadows mode
  @param    max_spec_width  maximal spectrum width
  @param    objnum      index of the object to test (1 for the first)
  @return   1 if valid 0 if not
*/
/*----------------------------------------------------------------------------*/
static int valid_spectrum(
        cpl_image       *   in,
        cpl_apertures   *   aperts,
        int                 offset,
        spec_shadows        shadows,
        int                 max_spec_width,
        int                 objnum)
{
    int                 objwidth ;
    double              valover, valunder, valcenter ;

    /* Find objwidth */
    objwidth = cpl_apertures_get_top(aperts, objnum) -
        cpl_apertures_get_bottom(aperts, objnum) + 1 ;
    if (objwidth > max_spec_width) {
        cpl_msg_error(cpl_func, "object is too wide") ;
        return 0 ;
    }

    /* Object is too small */
    if (cpl_apertures_get_npix(aperts, objnum) < 2) return 0 ;

    /* no shadow required */
    if (shadows == NO_SHADOW) return 1 ;

    /* Get the median of the object (valcenter) */
    valcenter = cpl_apertures_get_median(aperts, objnum) ;

    /* Get the black shadows medians (valunder and valover) */
    if (cpl_apertures_get_bottom(aperts, objnum) - offset < 1) valunder = 0.0 ;
    else valunder = cpl_image_get_median_window(in, 1,
            cpl_apertures_get_bottom(aperts, objnum) - offset, 1, 
            cpl_apertures_get_top(aperts, objnum) - offset) ;
    
    if (cpl_apertures_get_top(aperts, objnum) + offset > 1024) valover = 0.0 ;
    else valover = cpl_image_get_median_window(in, 1,
            cpl_apertures_get_bottom(aperts, objnum) + offset, 1, 
            cpl_apertures_get_top(aperts, objnum) + offset) ;

    switch (shadows) {
        case TWO_SHADOWS:
        if ((valunder < -fabs(valcenter/SPEC_SHADOW_FACT)) &&
            (valover < -fabs(valcenter/SPEC_SHADOW_FACT))    &&
            (valunder/valover > 0.5) &&
            (valunder/valover < 2.0)) return 1 ;
        break;

        case ONE_SHADOW:
        if ((valunder < -fabs(valcenter/SPEC_SHADOW_FACT)) ||
            (valover < -fabs(valcenter/SPEC_SHADOW_FACT))) return 1 ;
        break;

        case NO_SHADOW:
        return 1 ;

        default:
        cpl_msg_error(cpl_func, "unknown spec_detect_mode") ;
        break ;
    }

    cpl_msg_debug(cpl_func, "No spectrum(%d): under=%g, center=%g, over=%g",
                  shadows, valunder, valcenter, valover);

    return 0 ;
}
