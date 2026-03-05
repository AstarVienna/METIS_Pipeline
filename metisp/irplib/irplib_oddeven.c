/* $Id: irplib_oddeven.c,v 1.9 2012-01-12 11:50:41 llundin Exp $
 *
 * This file is part of the irplib package
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
 * $Author: llundin $
 * $Date: 2012-01-12 11:50:41 $
 * $Revision: 1.9 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <math.h>
#include <cpl.h>

#include "irplib_oddeven.h"

/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

static cpl_imagelist * irplib_oddeven_cube_conv_xy_rtheta(cpl_imagelist *) ;
static cpl_imagelist * irplib_oddeven_cube_conv_rtheta_xy(cpl_imagelist *) ;
 
/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_oddeven     Odd/Even column effect correction
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Estimate the odd/even rate in an image quadrant
  @param    in      the inpute image
  @param    iquad    the quadrant (ll=1, lr=2, ul=3, ur=4, all=0)  
  @param    r_even  the median of even columns / median of all columns
  @return   0 if ok, -1 otherwise
*/
/*----------------------------------------------------------------------------*/
int irplib_oddeven_monitor(
        const cpl_image     *   in,
        int                     iquad,
        double              *   r_even) 
{
    cpl_image       *   extracted ;        
    cpl_image       *   labels ;        
    int             *   plabels ;
    int                 llx, lly, urx, ury ;
    int                 nx, ny ;
    double              f_even, f_tot ;
    cpl_apertures   *   aperts ;
    int                 i, j ;

    /* Test entries */
    if (in == NULL || r_even == NULL) return -1 ;
    nx = cpl_image_get_size_x(in) ;
    ny = cpl_image_get_size_y(in) ;
    
    switch (iquad){
        case 1:
            llx = 1 ; lly = 1 ; urx = nx/2 ; ury = ny/2 ; break ;
        case 2:
            llx = (nx/2)+1 ; lly = 1 ; urx = nx ; ury = ny/2 ; break ;
        case 3:
            llx = 1 ; lly = (ny/2)+1 ; urx = nx/2 ; ury = ny ; break ;
        case 4:
            llx = (nx/2)+1 ; lly = (ny/2)+1 ; urx = nx ; ury = ny ; break ;
        case 0:
            llx = 1 ; lly = 1 ; urx = nx ; ury = ny ; break ;
        default:
            cpl_msg_error(cpl_func, "Unsupported mode") ;
            *r_even = 0.0 ;
            return -1 ;
    }
   
    /* Extract quadrant */
    if ((extracted = cpl_image_extract(in, llx, lly, urx, ury)) == NULL) {
        cpl_msg_error(cpl_func, "Cannot extract quadrant") ;
        *r_even = 0.0 ;
        return -1 ;
    }
    nx = cpl_image_get_size_x(extracted) ;
    ny = cpl_image_get_size_y(extracted) ;
            
    /* Get f_tot */
    f_tot = cpl_image_get_median(extracted) ;
    if (fabs(f_tot) < 1e-6) {
        cpl_msg_warning(cpl_func, "Quadrant median is 0.0") ;
        cpl_image_delete(extracted) ;
        *r_even = 0.0 ;
        return -1 ;
    }

    /* Create the label image to define the even columns */
    labels = cpl_image_new(nx, ny, CPL_TYPE_INT) ;
    plabels = cpl_image_get_data_int(labels) ;
    for (i=0 ; i<nx ; i++) {
        if (i % 2) for (j=0 ; j<ny ; j++) plabels[i+j*nx] = 0 ;
        else for (j=0 ; j<ny ; j++) plabels[i+j*nx] = 1 ;
    }
    
    /* Get the median of even columns */
    if ((aperts = cpl_apertures_new_from_image(extracted, labels)) == NULL) {
        cpl_msg_error(cpl_func, "Cannot compute the even columns median") ;
        cpl_image_delete(extracted) ;
        cpl_image_delete(labels) ;
        *r_even = 0.0 ;
        return -1 ;
    }
    cpl_image_delete(extracted) ;
    cpl_image_delete(labels) ;
    f_even = cpl_apertures_get_median(aperts, 1) ;
    cpl_apertures_delete(aperts) ;
    
    /* Compute the even rate and return */
    *r_even = f_even / f_tot ;
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Correct the odd/even in an image 
  @param    in      the inpute image
  @return   the corrected image or NULL on error case
*/
/*----------------------------------------------------------------------------*/
cpl_image * irplib_oddeven_correct(const cpl_image * in)
{
    cpl_image       *   in_real ;
    cpl_image       *   in_imag ;
    cpl_imagelist   *   freq_i ;
    cpl_imagelist   *   freq_i_amp ;
    cpl_image       *   cur_im ;
    double          *   pcur_im ;
    cpl_image       *   cleaned ;
    int                 nx ;
    cpl_vector      *   hf_med ;

    /* Test entries */
    if (in==NULL) return NULL ;

    nx = cpl_image_get_size_x(in) ;

    /* Local copy of the input image in DOUBLE */
    in_real = cpl_image_cast(in, CPL_TYPE_DOUBLE) ;
    in_imag = cpl_image_duplicate(in_real) ;
    cpl_image_multiply_scalar(in_imag, 0.0) ;
    
    /* Apply FFT to input image */
    cpl_image_fft(in_real, in_imag, CPL_FFT_DEFAULT) ;

    /* Put the result in an image list */
    freq_i = cpl_imagelist_new() ;
    cpl_imagelist_set(freq_i, in_real, 0) ;
    cpl_imagelist_set(freq_i, in_imag, 1) ;
    
    /* Convert to amplitude/phase */
    freq_i_amp = irplib_oddeven_cube_conv_xy_rtheta(freq_i);
    cpl_imagelist_delete(freq_i) ;

    /* Correct the odd-even frequency */
    cur_im = cpl_imagelist_get(freq_i_amp, 0) ;
    pcur_im = cpl_image_get_data_double(cur_im) ;
    /* Odd-even frequency will be replaced by 
       the median of the 5 values around */
    hf_med = cpl_vector_new(5); 

    cpl_vector_set(hf_med, 0, pcur_im[nx/2 + 1]); 
    cpl_vector_set(hf_med, 1, pcur_im[nx/2 + 2]);
    cpl_vector_set(hf_med, 2, pcur_im[nx/2 + 3]);
    cpl_vector_set(hf_med, 3, pcur_im[nx/2    ]);
    cpl_vector_set(hf_med, 4, pcur_im[nx/2  -1]);

    pcur_im[nx / 2 + 1] = cpl_vector_get_median(hf_med);
    cpl_vector_delete(hf_med);

    /* Convert to X/Y */
    freq_i = irplib_oddeven_cube_conv_rtheta_xy(freq_i_amp) ;
    cpl_imagelist_delete(freq_i_amp) ;
    /* FFT back to image space */
    cpl_image_fft(cpl_imagelist_get(freq_i, 0), cpl_imagelist_get(freq_i, 1), 
            CPL_FFT_INVERSE) ;
    cleaned = cpl_image_cast(cpl_imagelist_get(freq_i, 0), CPL_TYPE_FLOAT) ;
    cpl_imagelist_delete(freq_i) ;
    return cleaned ;
}

/**@}*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Convert a 2-plane cube from (real,imag) to (ampl,phase).
  @param    cube_in        Input cube (containing 2 planes)
  @return    1 newly allocated cube containing 2 planes.

  The input cube is expected to contain two planes: first one is the real part 
  of a complex image, second one is the imaginary part of the same image. The 
  returned cube contains two planes: first one is the complex amplitude of the 
  image, second one is the phase.
 */
/*----------------------------------------------------------------------------*/
static cpl_imagelist * irplib_oddeven_cube_conv_xy_rtheta(
        cpl_imagelist   *   cube_in)
{
    cpl_imagelist       *   cube_out ;
    double                  re, im ;
    double                  mod, phase ;
    int                     nx, ny, np ;
    cpl_image           *   tmp_im ;
    double              *   pim1 ;
    double              *   pim2 ;
    double              *   pim3 ;
    double              *   pim4 ;
    int                     i, j ;

    /* Error handling : test entries    */
    if (cube_in == NULL) return NULL ;
    np = cpl_imagelist_get_size(cube_in) ;
    if (np != 2) return NULL ;

    /* Initialise */
    tmp_im = cpl_imagelist_get(cube_in, 0) ;
    pim1 = cpl_image_get_data_double(tmp_im) ;
    nx = cpl_image_get_size_x(tmp_im) ;
    ny = cpl_image_get_size_y(tmp_im) ;
    tmp_im = cpl_imagelist_get(cube_in, 1) ;
    pim2 = cpl_image_get_data_double(tmp_im) ;

    /* Allocate cube_out */
    cube_out = cpl_imagelist_duplicate(cube_in) ;

    tmp_im = cpl_imagelist_get(cube_out, 0) ;
    pim3 = cpl_image_get_data_double(tmp_im) ;
    tmp_im = cpl_imagelist_get(cube_out, 1) ;
    pim4 = cpl_image_get_data_double(tmp_im) ;
    /* Convert */
    for (j=0 ; j<ny ; j++) {
        for (i=0 ; i<nx ; i++) {
            re = (double)pim1[i+j*nx] ;
            im = (double)pim2[i+j*nx] ;
            mod = (double)(sqrt(re*re + im*im)) ;
            if (re != 0.0)
                phase = (double)atan2(im, re) ;
            else 
                phase = 0.0 ;
            pim3[i+j*nx] = mod ; 
            pim4[i+j*nx] = phase ; 
        }
    }
    return cube_out ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Convert a 2-plane cube from (ampl,phase) to (real,imag).
  @param    cube_in        Input cube (containing 2 planes)
  @return    1 newly allocated cube containing 2 planes.

  The input cube is expected to contain two planes: first one is the
  amplitude of a complex image, second one is the phase. The returned cube
  contains two planes: first one is the real part of the image, second one
  is the imaginary part.

  The returned cube must be deallocated using cube_del().
 */
/*----------------------------------------------------------------------------*/
static cpl_imagelist * irplib_oddeven_cube_conv_rtheta_xy(
        cpl_imagelist   *   cube_in)
{
    cpl_imagelist       *   cube_out ;
    double                  re, im ;
    double                  mod, phase ;
    int                     nx, ny, np ;
    cpl_image           *   tmp_im ;
    double              *   pim1 ;
    double              *   pim2 ;
    double              *   pim3 ;
    double              *   pim4 ;
    int                     i, j ;

    /* Error handling : test entries    */
    if (cube_in == NULL) return NULL ;
    np = cpl_imagelist_get_size(cube_in) ;
    if (np != 2) return NULL ;

    /* Initialise */
    tmp_im = cpl_imagelist_get(cube_in, 0) ;
    pim1 = cpl_image_get_data_double(tmp_im) ;
    nx = cpl_image_get_size_x(tmp_im) ;
    ny = cpl_image_get_size_y(tmp_im) ;
    tmp_im = cpl_imagelist_get(cube_in, 1) ;
    pim2 = cpl_image_get_data_double(tmp_im) ;

    /* Allocate cube_out */
    cube_out = cpl_imagelist_duplicate(cube_in) ;

    tmp_im = cpl_imagelist_get(cube_out, 0) ;
    pim3 = cpl_image_get_data_double(tmp_im) ;
    tmp_im = cpl_imagelist_get(cube_out, 1) ;
    pim4 = cpl_image_get_data_double(tmp_im) ;
    /* Convert */
    for (j=0 ; j<ny ; j++) {
        for (i=0 ; i<nx ; i++) {
            mod = (double)pim1[i+j*nx] ;
            phase = (double)pim2[i+j*nx] ;
            re = (double)(mod * cos(phase));
            im = (double)(mod * sin(phase));
            pim3[i+j*nx] = re ; 
            pim4[i+j*nx] = im ; 
        }
    }
    return cube_out ;
}
