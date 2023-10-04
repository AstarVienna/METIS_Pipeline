/* $Id: irplib_calib.c,v 1.19 2013-03-01 10:26:22 llundin Exp $
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
 * $Date: 2013-03-01 10:26:22 $
 * $Revision: 1.19 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "irplib_calib.h"

#include <math.h>

/*-----------------------------------------------------------------------------
                            Static Function Prototypes
 -----------------------------------------------------------------------------*/

static int  
irplib_get_clean_mean_window(cpl_image* img, 
                             const int llx, 
                             const int lly, 
                             const int urx, int ury, 
                             const int kappa, 
                             const int nclip, 
                             double* clean_mean, 
                             double* clean_stdev);

static double irplib_pfits_get_dit(const cpl_propertylist * plist);
static double irplib_pfits_get_exp_time(const cpl_propertylist* plist);
/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_calib   Functions for calibrations
 */
/*----------------------------------------------------------------------------*/

/**@{*/
/*---------------------------------------------------------------------------*/
/**
  @brief    Read the DIT from the DIT keyword, ESO.DET.DIT or ESO.DET.SEQ1.DIT
            in a propertylist
  @param    plist propertylist 
  @return   dit value, or zero on error
  @see cpl_propertylist_get_double()

 */
/*---------------------------------------------------------------------------*/
static double irplib_pfits_get_dit(const cpl_propertylist * plist)
{
    cpl_errorstate prestate = cpl_errorstate_get();
    double         dit      = cpl_propertylist_get_double(plist, "ESO DET DIT");

    if (!cpl_errorstate_is_equal(prestate)) {
        /* Key not present (or with wrong type, or something) */
        cpl_errorstate prestate2 = cpl_errorstate_get();

        dit = cpl_propertylist_get_double(plist, "ESO DET SEQ1 DIT");

        if (cpl_errorstate_is_equal(prestate2)) {
            /* Key present (with expected type): recover */
            cpl_errorstate_set(prestate);
        } else {
            cpl_error_set_where(cpl_func); /* Propagate error */
        }
    }

    return dit;
}

/*---------------------------------------------------------------------------*/
/**
  @brief    Read the EXPTIME from the EXPTIME keyword in a propertylist
  @param    plist propertylist
  @return   keyword value
  @see cpl_propertylist_get_double()

 */
/*---------------------------------------------------------------------------*/
static double irplib_pfits_get_exp_time(const cpl_propertylist* plist)
{
  
    return cpl_propertylist_get_double(plist,"EXPTIME");

}


/**
  @brief    Get clean mean and stdev of an image over a window
  @param    img input image
  @param    llx input lower left x image's window coordinate
  @param    lly input lower left y image's window coordinate
  @param    urx input upper right y image's window coordinate
  @param    ury input upper right y image's window coordinate
  @param    kappa  input kappa of kappa-sigma clip
  @param    nclip input max no of kappa-sigma clip iterations
  @param    clean_mean output upper right y image's window coordinate
  @param    clean_stdev output upper right y image's window coordinate
  @return   pixel scale
 */


static int  
irplib_get_clean_mean_window(cpl_image* img, 
                             const int llx, 
                             const int lly, 
                             const int urx, int ury, 
                             const int kappa, 
                             const int nclip, 
                             double* clean_mean, 
                             double* clean_stdev)
{


  double mean=0;
  double stdev=0;
  cpl_image* tmp=NULL;
  cpl_stats* stats=NULL;
  int i=0;
  
  tmp=cpl_image_extract(img,llx,lly,urx,ury);
  cpl_image_accept_all(tmp);
  for(i=0;i<nclip;i++) {

    double threshold=0;
    double lo_cut=0;
    double hi_cut=0;
    cpl_mask* mask=NULL;

    cpl_stats_delete(stats);
    stats = cpl_stats_new_from_image(tmp, CPL_STATS_MEAN | CPL_STATS_STDEV);
    mean = cpl_stats_get_mean(stats);
    stdev = cpl_stats_get_stdev(stats);

    threshold=kappa*stdev;
    lo_cut=mean-threshold;
    hi_cut=mean+threshold;

    cpl_image_accept_all(tmp);
    mask=cpl_mask_threshold_image_create(tmp,lo_cut,hi_cut);

    cpl_mask_not(mask);
    cpl_image_reject_from_mask(tmp,mask);
    cpl_mask_delete(mask);


  }
  *clean_mean=mean;
  *clean_stdev=stdev;
  cpl_image_delete(tmp);
  cpl_stats_delete(stats);
 
  return 0;


}



/*---------------------------------------------------------------------------*/
/**
  @brief    Computes the detector's gain
  @param    son   the input frameset of linearity on-flat fields 
  @param    sof   the input frameset of linearity off-flat fields 
  @param    zone  pointer to an integer array with locations (llx,lly,urx,ury)
                   of region where a clean mean and noise are computed 
  @param    kappa  value of kappa in kappa-sigma clipping 
  @param    nclip  number of kappa-sigma clipping iterations

  @return   pointer to a table containing single gain evaluations
  @note: 
  #1 input frames need to have defined FITS keyword EXPTIME
  #2 input frames need to have defined FITS keyword DIT
 */
/*---------------------------------------------------------------------------*/


cpl_table* 
irplib_compute_gain(
                cpl_frameset* son, 
                cpl_frameset* sof, 
                int* zone,   
                const int kappa,
                const int nclip)
{

  cpl_frame*    frm=NULL;

  cpl_table* res_tbl=NULL;
  cpl_vector* dit_on=NULL;
  cpl_vector* dit_of=NULL;
  cpl_vector* exptime_on=NULL;
  cpl_vector* exptime_of=NULL;

  int non=0;
  int nof=0;
  int nfr=0;
  int llx;
  int lly;
  int urx;
  int ury;

  const char* name=NULL;
  int i=0;

  double dit_ref=0;
  double exptime_ref=0;

 
  non = cpl_frameset_get_size(son);
  nof = cpl_frameset_get_size(sof);
  nfr = (non <= nof) ? non : nof;

  dit_on=cpl_vector_new(nfr);
  dit_of=cpl_vector_new(nfr);  
  exptime_on=cpl_vector_new(nfr);
  exptime_of=cpl_vector_new(nfr);

  for(i=0;i<nfr;i++) {
    cpl_propertylist* plist=NULL;

    frm=cpl_frameset_get_position(son,i);
    name=cpl_frame_get_filename(frm);
    plist=cpl_propertylist_load(name,0);
    dit_ref=irplib_pfits_get_dit(plist);
    exptime_ref=(double)irplib_pfits_get_exp_time(plist);
    cpl_propertylist_delete(plist);
    cpl_vector_set(dit_on,i,dit_ref);
    cpl_vector_set(exptime_on,i,exptime_ref);

    frm=cpl_frameset_get_position(sof,i);
    name=cpl_frame_get_filename(frm);
    plist=cpl_propertylist_load(name,0);
    dit_ref=irplib_pfits_get_dit(plist);
    exptime_ref=(double)irplib_pfits_get_exp_time(plist);
    cpl_propertylist_delete(plist);
    cpl_vector_set(dit_of,i,dit_ref);
    cpl_vector_set(exptime_of,i,exptime_ref);

  }


  llx=zone[0];
  lly=zone[1];
  urx=zone[2];
  ury=zone[3];



  res_tbl=cpl_table_new(nfr);
  cpl_table_new_column(res_tbl,"adu", CPL_TYPE_DOUBLE);
  cpl_table_new_column(res_tbl,"gain", CPL_TYPE_DOUBLE);
 
  for(i=0;i<nfr;i++) {
    cpl_image* img_on1=NULL;
    cpl_image* img_of1=NULL;
    int m=0;

    frm=cpl_frameset_get_position(son,i);
    name=cpl_frame_get_filename(frm);
    img_on1=cpl_image_load(name,CPL_TYPE_FLOAT,0,0);

    frm=cpl_frameset_get_position(sof,i);
    name=cpl_frame_get_filename(frm);
    img_of1=cpl_image_load(name,CPL_TYPE_FLOAT,0,0);


    dit_ref=cpl_vector_get(dit_on,i);
    exptime_ref=cpl_vector_get(exptime_on,i);

   
    for(m=0;m<nfr; m++) {
      if(m != i) {
        double dit_tmp=0;
        double exptime_tmp=0;

	frm=cpl_frameset_get_position(son,m);
	name=cpl_frame_get_filename(frm);
	dit_tmp=cpl_vector_get(dit_on,m);
	exptime_tmp=cpl_vector_get(exptime_on,m);
	if(dit_tmp == dit_ref && exptime_tmp == exptime_ref) {
          cpl_image* img_on2=NULL;
          cpl_image* img_on_dif=NULL;

          cpl_image* img_of2=NULL;
          cpl_image* img_of_dif=NULL;

          double avg_on1=0;
          double avg_on2=0;
          double avg_of1=0;
          double avg_of2=0;
          double avg_on_dif=0;
          double avg_of_dif=0;
          double std=0;
          double sig_on_dif=0;
          double sig_of_dif=0;
          double gain=0;

	  img_on2=cpl_image_load(name,CPL_TYPE_FLOAT,0,0);
	  frm=cpl_frameset_get_position(sof,m);
	  name=cpl_frame_get_filename(frm);
	  img_of2=cpl_image_load(name,CPL_TYPE_FLOAT,0,0);

	  img_on_dif=cpl_image_subtract_create(img_on1,img_on2);
	  img_of_dif=cpl_image_subtract_create(img_of1,img_of2);
	  
	  irplib_get_clean_mean_window(img_on1,llx,lly,urx,ury,kappa,
                                      nclip,&avg_on1,&std);
	  irplib_get_clean_mean_window(img_on2,llx,lly,urx,ury,kappa,
                                      nclip,&avg_on2,&std);
	  irplib_get_clean_mean_window(img_of1,llx,lly,urx,ury,kappa,
                                      nclip,&avg_of1,&std);
	  irplib_get_clean_mean_window(img_of2,llx,lly,urx,ury,kappa,
                                      nclip,&avg_of2,&std);
	  irplib_get_clean_mean_window(img_on_dif,llx,lly,urx,ury,kappa,
                                      nclip,&avg_on_dif,&sig_on_dif);
	  irplib_get_clean_mean_window(img_of_dif,llx,lly,urx,ury,kappa,
                                      nclip,&avg_of_dif,&sig_of_dif);

	  cpl_image_delete(img_on2);
	  cpl_image_delete(img_of2);
	  cpl_image_delete(img_on_dif);
	  cpl_image_delete(img_of_dif);

          gain=((avg_on1+avg_on2)-(avg_of1+avg_of2))/
               ((sig_on_dif*sig_on_dif)-(sig_of_dif*sig_of_dif));

          cpl_table_set_double(res_tbl,"gain",m,gain);
          cpl_table_set_double(res_tbl,"adu",m,
                               ((avg_on1+avg_on2)/2-(avg_of1+avg_of2)/2));

	}
      }
    }
    cpl_image_delete(img_on1);
    cpl_image_delete(img_of1);
  }
  

  cpl_vector_delete(dit_on);
  cpl_vector_delete(dit_of);
  cpl_vector_delete(exptime_on);
  cpl_vector_delete(exptime_of);

  return res_tbl;

}

/* --------------------------------------------------------------------------*/
/**
  @brief    Computes the detector's linearity
  @param    son    the input frameset of linearity on flat fields 
  @param    sof    the input frameset of linearity off flat fields 
  @return   pointer to a table containing linearity evaluations
  @note: 
  #2 input frames need to have defined FITS keyword EXPTIME
  #3 input frames need to have defined FITS keyword DIT
 */
/*---------------------------------------------------------------------------*/


cpl_table* irplib_compute_linearity(cpl_frameset* son, cpl_frameset* sof)
{

  int non=0;
  int nof=0;
  int nfr=0;
  int i=0;
  double med_dit=0;
  /*double avg_dit=0;*/

  cpl_vector* vec_adl=NULL;
  cpl_vector* vec_dit=NULL;
  cpl_vector* vec_avg=NULL;
  cpl_vector* vec_med=NULL;
  cpl_vector* vec_avg_dit=NULL;
  cpl_vector* vec_med_dit=NULL;

  double dit=0;
  cpl_table* lin_tbl=NULL;

 
  non = cpl_frameset_get_size(son);
  nof = cpl_frameset_get_size(sof);
  nfr = (non <= nof) ? non : nof;

  lin_tbl=cpl_table_new(nfr);
  cpl_table_new_column(lin_tbl,"med", CPL_TYPE_DOUBLE);
  cpl_table_new_column(lin_tbl,"avg", CPL_TYPE_DOUBLE);
  cpl_table_new_column(lin_tbl,"med_dit", CPL_TYPE_DOUBLE);
  cpl_table_new_column(lin_tbl,"avg_dit", CPL_TYPE_DOUBLE);
  cpl_table_new_column(lin_tbl,"dit", CPL_TYPE_DOUBLE);
  vec_med=cpl_vector_new(nfr);
  vec_avg=cpl_vector_new(nfr);
  vec_med_dit=cpl_vector_new(nfr);
  vec_avg_dit=cpl_vector_new(nfr);
  vec_dit=cpl_vector_new(nfr);
  vec_adl=cpl_vector_new(nfr);
  for(i=0;i<nfr;i++) {
    cpl_frame*    frm=NULL;

    double med_on=0;
    double avg_on=0;
    double med_of=0;
    double avg_of=0;

    double med=0;
    double avg=0;
    double avg_dit=0;

    const char* name=NULL;
    cpl_image* img=NULL;
    cpl_propertylist* plist=NULL;

    frm=cpl_frameset_get_position(son,i);
    name=cpl_frame_get_filename(frm);
    img=cpl_image_load(name,CPL_TYPE_FLOAT,0,0);
    med_on=cpl_image_get_median(img);
    avg_on=cpl_image_get_mean(img);
    cpl_image_delete(img);

    frm=cpl_frameset_get_position(sof,i);
    name=cpl_frame_get_filename(frm);
    img=cpl_image_load(name,CPL_TYPE_FLOAT,0,0);
    med_of=cpl_image_get_median(img);
    avg_of=cpl_image_get_mean(img);
    cpl_image_delete(img);
    med=med_on-med_of;
    avg=avg_on-avg_of;
    plist=cpl_propertylist_load(name,0);
    dit=(double)irplib_pfits_get_dit(plist);
    cpl_propertylist_delete(plist);
    avg_dit=avg/dit;
    med_dit=med/dit;

    cpl_vector_set(vec_dit,i,dit);
    cpl_vector_set(vec_avg,i,avg);
    cpl_vector_set(vec_med,i,med);
    cpl_vector_set(vec_avg_dit,i,avg_dit);
    cpl_vector_set(vec_med_dit,i,med_dit);


    cpl_table_set_double(lin_tbl,"dit",i,dit);
    cpl_table_set_double(lin_tbl,"med",i,med);
    cpl_table_set_double(lin_tbl,"avg",i,avg);
    cpl_table_set_double(lin_tbl,"med_dit",i,med_dit);
    cpl_table_set_double(lin_tbl,"avg_dit",i,avg_dit);

  }
  cpl_table_new_column(lin_tbl,"adl", CPL_TYPE_DOUBLE);
  med_dit=cpl_vector_get_mean(vec_med_dit);
  /*avg_dit=cpl_vector_get_mean(vec_avg_dit);*/

  for(i=0;i<nfr;i++) {
    int* status=0;
    dit = cpl_table_get_double(lin_tbl,"dit",i,status);
    cpl_vector_set(vec_adl,i,dit*med_dit);
    cpl_table_set_double(lin_tbl,"adl",i,dit*med_dit);
  }
 
  
  cpl_vector_delete(vec_dit);
  cpl_vector_delete(vec_adl);
  cpl_vector_delete(vec_avg);
  cpl_vector_delete(vec_med);
  cpl_vector_delete(vec_avg_dit);
  cpl_vector_delete(vec_med_dit);


  return lin_tbl;

}


/*----------------------------------------------------------------------------*/
/**
  @brief    Apply the detector linearity correction
  @param    ilist   the input image list
  @param    detlin_a    the a coeffs
  @param    detlin_b    the b coeffs
  @param    detlin_c    the c coeffs
  @return   0 if everything is ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int irplib_detlin_correct(
        cpl_imagelist       *   ilist,
        const char          *   detlin_a,
        const char          *   detlin_b,
        const char          *   detlin_c)
{
    cpl_image       *   ima ;
    cpl_image       *   imb ;
    cpl_image       *   imc ;
    float           *   pima ;
    float           *   pimb ;
    float           *   pimc ;
    float           *   pdata ;
    int                 nx, ny, ni ; 
    double              coeff_1, coeff_2, val ;
    int                 i, j ;

    /* Test entries */
    if (!ilist || !detlin_a || !detlin_b || !detlin_c) return -1 ;
    
    /* Load the 3 coeffs images */
    ima = cpl_image_load(detlin_a, CPL_TYPE_FLOAT, 0, 0) ;
    imb = cpl_image_load(detlin_b, CPL_TYPE_FLOAT, 0, 0) ;
    imc = cpl_image_load(detlin_c, CPL_TYPE_FLOAT, 0, 0) ;
    if (!ima || !imb || !imc) {
        cpl_msg_error(cpl_func, "Cannot load the detlin images") ;
        if (ima) cpl_image_delete(ima) ;
        if (imb) cpl_image_delete(imb) ;
        if (imc) cpl_image_delete(imc) ;
        return -1 ;
    }
    pima = cpl_image_get_data_float(ima) ;
    pimb = cpl_image_get_data_float(imb) ;
    pimc = cpl_image_get_data_float(imc) ;
    
    /* Test sizes */
    nx = cpl_image_get_size_x(cpl_imagelist_get(ilist, 0)) ;
    ny = cpl_image_get_size_y(cpl_imagelist_get(ilist, 0)) ;
    ni = cpl_imagelist_get_size(ilist) ;
    if ((cpl_image_get_size_x(ima) != nx) ||
            (cpl_image_get_size_x(imb) != nx) ||
            (cpl_image_get_size_x(imc) != nx) ||
            (cpl_image_get_size_y(ima) != ny) ||
            (cpl_image_get_size_y(imb) != ny) ||
            (cpl_image_get_size_y(imc) != ny)) {
        cpl_msg_error(cpl_func, "Incompatible sizes") ;
        cpl_image_delete(ima) ;
        cpl_image_delete(imb) ;
        cpl_image_delete(imc) ;
        return -1 ;
    }
    
    /* Loop on pixels */
    for (i=0 ; i<nx*ny ; i++) {
        /* Compute the coefficients */
        if (fabs(pima[i]) < 1e-30) {
            coeff_1 = coeff_2 = (double)0.0 ;
        } else {
            coeff_1 = (double)pimb[i] / (double)pima[i] ;
            coeff_2 = (double)pimc[i] / (double)pima[i] ;
        }
        /* Correct this pixel in each plane */
        for (j=0 ; j<ni ; j++) {
            pdata = cpl_image_get_data_float(cpl_imagelist_get(ilist, j)) ;
            val = (double)pdata[i] ;
            pdata[i]=(float)(val+coeff_1*val*val+coeff_2*val*val*val) ;
        }
    }
    /* Free and return */
    cpl_image_delete(ima) ;
    cpl_image_delete(imb) ;
    cpl_image_delete(imc) ;
    return 0 ;
}
 
/*----------------------------------------------------------------------------*/
/**
  @brief    Apply the calibration to the frames
  @param    ilist   the input image list
  @param    flat    the flat field
  @param    dark    the dark
  @param    bpm     the bad pixels map
  @return   0 if everything is ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int irplib_flat_dark_bpm_calib(
        cpl_imagelist       *   ilist,
        const char          *   flat,
        const char          *   dark,
        const char          *   bpm)
{
    /* Test entries */
    if (ilist == NULL) return -1 ;

    /* Dark correction */
    if (dark != NULL) {
        cpl_image       *   dark_image ;
        cpl_msg_info(cpl_func, "Subtract the dark to the images") ;
        /* Load the dark image */
        if ((dark_image = cpl_image_load(dark, CPL_TYPE_FLOAT, 0, 0)) == NULL) {
            cpl_msg_error(cpl_func, "Cannot load the dark %s", dark) ;
            return -1 ;
        }
        /* Apply the dark correction to the images */
        if (cpl_imagelist_subtract_image(ilist, dark_image)!=CPL_ERROR_NONE) {
            cpl_msg_error(cpl_func, "Cannot apply the dark to the images") ;
            cpl_image_delete(dark_image) ;
            return -1 ;
        }
        cpl_image_delete(dark_image) ;
    }

    /* Flat-field correction */
    if (flat != NULL) {
        cpl_image       *   flat_image ;
        cpl_msg_info(cpl_func, "Divide the images by the flatfield") ;
        /* Load the flat image */
        if ((flat_image = cpl_image_load(flat, CPL_TYPE_FLOAT, 0, 0)) == NULL) {
            cpl_msg_error(cpl_func, "Cannot load the flat field %s", flat) ;
            return -1 ;
        }
        /* Apply the flatfield correction to the images */
        if (cpl_imagelist_divide_image(ilist, flat_image)!=CPL_ERROR_NONE) {
            cpl_msg_error(cpl_func, "Cannot apply the flatfield to the images") ;
            cpl_image_delete(flat_image) ;
            return -1 ;
        }
        cpl_image_delete(flat_image) ;
    }

    /* Correct the bad pixels if requested */
    if (bpm != NULL) {
        cpl_mask        *   bpm_im_bin ;
        cpl_image       *   bpm_im_int ;
        int                 i ;
        cpl_msg_info(cpl_func, "Correct the bad pixels in the images") ;
         /* Load the bad pixels image */
        if ((bpm_im_int = cpl_image_load(bpm, CPL_TYPE_INT, 0, 0)) == NULL) {
            cpl_msg_error(cpl_func, "Cannot load the bad pixel map %s", bpm) ;
            return -1 ;
        }
        /* Convert the map from integer to binary */
        bpm_im_bin = cpl_mask_threshold_image_create(bpm_im_int, -0.5, 0.5) ;
        cpl_mask_not(bpm_im_bin) ;
        cpl_image_delete(bpm_im_int) ;
        /* Apply the bad pixels cleaning */
        for (i=0 ; i<cpl_imagelist_get_size(ilist) ; i++) {
            cpl_image_reject_from_mask(cpl_imagelist_get(ilist, i), bpm_im_bin);
            if (cpl_detector_interpolate_rejected(
                        cpl_imagelist_get(ilist, i)) != CPL_ERROR_NONE) {
                cpl_msg_error(cpl_func, "Cannot clean the bad pixels in obj %d",
                        i+1);
                cpl_mask_delete(bpm_im_bin) ;
                return -1 ;
            }
        }
        cpl_mask_delete(bpm_im_bin) ;
    }

    /* Return */
    return 0 ;
}

/**@}*/

