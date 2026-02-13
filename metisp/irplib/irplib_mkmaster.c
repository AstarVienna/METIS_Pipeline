/* $Id: irplib_mkmaster.c,v 1.6 2013-02-27 16:00:51 jtaylor Exp $
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
 * $Author: jtaylor $
 * $Date: 2013-02-27 16:00:51 $
 * $Revision: 1.6 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>
#include "irplib_mkmaster.h"

/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_calib   Functions for calibrations
 */
/*----------------------------------------------------------------------------*/

/**@{*/
/*---------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the DIT keyword
            in a propertylist
  @param    plist propertylist 
  @return   dit value
 */
/*---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief	Computes kappa-sigma clean mean (free bad pixels) for each input image of the input imagelist.
  @param	iml      input imagelist
  @param	kappa    value for kappa-sigma clip
  @param	nclip    Number of clipping iterations
  @param	tolerance tolerance on range between two successive clip iterations

  @return	vector with computed values for each image of the list

  The returned vector must be deallocated.
 */
/*--------------------------------------------------------------------------*/
static cpl_vector * 
irplib_imagelist_get_clean_mean_levels(const cpl_imagelist* iml, 
                                       const double kappa, 
                                       const int nclip,
                                       const double tolerance)
{

   int size=0;
   int i=0;
   cpl_vector* levels=NULL;
   double* pval=NULL;
   double mean=0;
   double stdev=0;


   cpl_error_ensure(iml != NULL, CPL_ERROR_NULL_INPUT, return(levels),
                    "Null input image list");
   cpl_error_ensure(kappa >= 0, CPL_ERROR_ILLEGAL_INPUT, return(levels), 
                    "Must be kappa>0");

   size=cpl_imagelist_get_size(iml);
   levels=cpl_vector_new(size);
   pval=cpl_vector_get_data(levels);

   for(i=0;i<size;i++) {
      const cpl_image* img=cpl_imagelist_get_const(iml,i);
      irplib_ksigma_clip(img,1,1,
                         cpl_image_get_size_x(img),
                         cpl_image_get_size_y(img),
                         nclip,kappa,tolerance,&mean,&stdev);
      cpl_msg_info(cpl_func,"Ima %d mean level: %g",i+1,mean);
      pval[i]=mean;
   }


   return levels;
}

/*-------------------------------------------------------------------------*/
/**
  @brief  Subtract from input imagelist values specified in input vector.
  @param  iml      input imagelist
  @param  values   value to be subtracted
  @return corrected imagelist

 */
/*--------------------------------------------------------------------------*/
static cpl_error_code
irplib_imagelist_subtract_values(cpl_imagelist** iml, cpl_vector* values)
{

   int size=0;
   int i=0;
   double* pval=NULL;
  
   size=cpl_imagelist_get_size(*iml);
   pval=cpl_vector_get_data(values);

   for(i=0;i<size;i++) {
      cpl_image* img=cpl_imagelist_get(*iml,i);
      cpl_image_subtract_scalar(img,pval[i]);
      cpl_imagelist_set(*iml,img,i);
   }

   return cpl_error_get_code();
}

/*---------------------------------------------------------------------------*/
/** 
 * @brief  Perform kappa-sigma clip.
   @author C. Izzo
   @param  values values to be checked
   @param  klow   kappa to clip too low level values
   @param  khigh  kappa to clip too high values
   @param  kiter  number of iterations

   @note   In first iteration a median is the reference value for robustness

 * @return 
 */
/*---------------------------------------------------------------------------*/
static double 
irplib_vector_ksigma(cpl_vector *values,
                     const double klow, const double khigh, int kiter)
{
   double  mean  /*= 0.0*/;  /* Comment out to suppress cppcheck warning. */
   double  sigma = 0.0;
   double *data  = cpl_vector_get_data(values);
   int     n     = cpl_vector_get_size(values);
   int     ngood = n;
   int     i;
 
   /*
    * At first iteration the mean is taken as the median, and the
    * standard deviation relative to this value is computed.
    */

   mean = cpl_vector_get_median(values);

   for (i = 0; i < n; i++) {
      sigma += (mean - data[i]) * (mean - data[i]);
   }
   sigma = sqrt(sigma / (n - 1));

   while (kiter) {
      cpl_vector *accepted;
      int count = 0;
      for (i = 0; i < ngood; i++) {
         if (data[i]-mean < khigh*sigma && mean-data[i] < klow*sigma) {
            data[count] = data[i];
            ++count;
         }
      }

      if (count == 0) // This cannot happen at first iteration.
         break;      // So we can break: we have already computed a mean.

      /*
       * The mean must be computed even if no element was rejected
       * (count == ngood), because at first iteration median instead
       * of mean was computed.
       */

      accepted = cpl_vector_wrap(count, data);
      mean = cpl_vector_get_mean(accepted);
      if(count>1) {
         sigma = cpl_vector_get_stdev(accepted);
      }
      cpl_vector_unwrap(accepted);

      if (count == ngood) {
         break;
      }
      ngood = count;
      --kiter;
   }

   return mean;
}


/**
 * @brief
 *   Stack images using k-sigma clipping
 *
 * @param imlist      List of images to stack
 * @param klow        Number of sigmas for rejection of lowest values
 * @param khigh       Number of sigmas for rejection of highest values
 * @param kiter       Max number of iterations
 *
 * @return Stacked image.
 *
 * At the first iteration the value of sigma is computed relatively to
 * the median value of all pixels at a given image position. For the
 * next iterations the sigma is computed in the standard way. If
 * at some iteration all points would be rejected, the mean computed
 * at the previous iteration is returned.
 */

static cpl_image *
irplib_imagelist_ksigma_stack(const cpl_imagelist *imlist, 
                              double klow, double khigh, int kiter)
{
   int         ni, nx, ny, npix;
   cpl_image  *out_ima=NULL;
   cpl_imagelist  *loc_iml=NULL;
   double      *pout_ima=NULL;
   cpl_image  *image=NULL;
   const double     **data=NULL;
   double     *med=NULL;
   cpl_vector *time_line=NULL;
  
   double     *ptime_line=NULL;
   int         i, j;
   double mean_of_medians=0;

   cpl_error_ensure(imlist != NULL, CPL_ERROR_NULL_INPUT, return(out_ima),
                    "Null input image list");

   ni         = cpl_imagelist_get_size(imlist);
   loc_iml        = cpl_imagelist_duplicate(imlist);
   image      = cpl_imagelist_get(loc_iml, 0);
   nx         = cpl_image_get_size_x(image);
   ny         = cpl_image_get_size_y(image);
   npix       = nx * ny;

   out_ima    = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
   pout_ima   = cpl_image_get_data_double(out_ima);

   time_line  = cpl_vector_new(ni);
   
   ptime_line = cpl_vector_get_data(time_line);

   data = cpl_calloc(sizeof(double *), ni);
   med  = cpl_calloc(sizeof(double), ni);

   for (i = 0; i < ni; i++) {
      image = cpl_imagelist_get(loc_iml, i);
      med[i]=cpl_image_get_median(image);
      cpl_image_subtract_scalar(image,med[i]);
      data[i] = cpl_image_get_data_double(image);
      mean_of_medians+=med[i];
   }
   mean_of_medians/=ni;

   for (i = 0; i < npix; i++) {
      for (j = 0; j < ni; j++) {
         ptime_line[j] = data[j][i];
      }
      pout_ima[i] = irplib_vector_ksigma(time_line, klow, khigh, kiter); 
   }
 
   cpl_image_add_scalar(out_ima,mean_of_medians);

 
   cpl_free(data);
   cpl_free(med);
   cpl_vector_delete(time_line);
   cpl_imagelist_delete(loc_iml);

   return out_ima;

} 




/*-------------------------------------------------------------------------*/
/**
  @brief  Computes master frame by clean stack mean of the input imagelist.
  @param  images   input imagelist
  @param  kappa    value for kappa-sigma clip
  @param  nclip    Number of clipping iterations
  @param  tolerance tolerance on range between two successive clip iterations

  @return master image 

  The returned image must be deallocated.
 */
/*--------------------------------------------------------------------------*/
cpl_image*
irplib_mkmaster_mean(cpl_imagelist* images,const double kappa, const int nclip, const double tolerance,const double klow,const double khigh,const int niter)
{

   cpl_image* master=NULL;
   cpl_vector* levels=NULL;
   double mean=0;
   cpl_imagelist* iml=NULL;

   cpl_msg_info(cpl_func,"method mean");
   iml=cpl_imagelist_duplicate(images);
   levels=irplib_imagelist_get_clean_mean_levels(iml,kappa,nclip,tolerance);
   mean=cpl_vector_get_mean(levels);
   cpl_msg_info(cpl_func,"Master mean level: %g",mean);

   irplib_imagelist_subtract_values(&iml,levels);

   master = irplib_imagelist_ksigma_stack(iml,klow,khigh,niter);
   cpl_image_add_scalar(master,mean);

   cpl_vector_delete(levels);
   cpl_imagelist_delete(iml);
   return master;

}

/*-------------------------------------------------------------------------*/
/**
  @brief  Computes master frame by clean stack median of the input imagelist.
  @param  images   input imagelist
  @param  kappa    value for kappa-sigma clip
  @param  nclip    Number of clipping iterations
  @param  tolerance tolerance on range between two successive clip iterations

  @return master image 

  The returned image must be deallocated.
 */
/*--------------------------------------------------------------------------*/
cpl_image* 
irplib_mkmaster_median(cpl_imagelist* images,const double kappa, const int nclip, const double tolerance)
{

   cpl_image* master=NULL;
   cpl_vector* levels=NULL;
   double mean=0;
   cpl_imagelist* iml=NULL;

   cpl_msg_info(cpl_func,"method median");
   iml=cpl_imagelist_duplicate(images);
   levels=irplib_imagelist_get_clean_mean_levels(iml,kappa,nclip,tolerance);

   mean=cpl_vector_get_mean(levels);
   cpl_msg_info(cpl_func,"Master mean level: %g",mean);
   irplib_imagelist_subtract_values(&iml,levels);
       
   master = cpl_imagelist_collapse_median_create(iml);

   cpl_image_add_scalar(master,mean);

   cpl_vector_delete(levels);
   cpl_imagelist_delete(iml);

   return master;

}

/* Work in progress */
static cpl_error_code
irplib_mkmaster_dark_qc(const cpl_imagelist* raw_images,
    cpl_imagelist* preproc_images,
    const cpl_parameterlist* parameters,
     const int pr_num_x, const int pr_num_y,
    const int pr_box_sx, const int pr_box_sy, const char* recipe_id,
    cpl_table* qclog) {

   cpl_ensure_code(qclog !=NULL, CPL_ERROR_NULL_INPUT);
   cpl_ensure_code(recipe_id !=NULL, CPL_ERROR_NULL_INPUT);
   cpl_ensure_code(parameters !=NULL, CPL_ERROR_NULL_INPUT);

  if (pr_num_x != 0 && pr_num_y != 0 && pr_box_sx != 0 && pr_box_sy != 0) {
    int i;
    for (i = 0; i < cpl_imagelist_get_size(raw_images); i++) {
      cpl_image* current_dark = cpl_image_duplicate(
          cpl_imagelist_get_const(preproc_images, i));
      cpl_msg_info(cpl_func, "Calculating QC parameters on raw dark frame %d",
          i);
      /* Here To be defined more general way to qc-log */
       /* UVES specific stuff: may be this function should not be put in irplib
      irplib_mdark_region_qc(current_dark, parameters, raw_images, recipe_id,qclog);
       */    
      /* FIXME: still safe if irplib_mdark_region_qc is commented in? */
      cpl_image_delete(current_dark);
    }
  }
  return cpl_error_get_code();
}

/*-------------------------------------------------------------------------*/
/**
 @brief    Find out the exposure time in seconds
 @param    plist       Header to read from

 @return   The requested value, or undefined on error

 An error is set if the exposure time is negative. In that case this negative
 value is returned.
 */
/*-------------------------------------------------------------------------*/
static double
irplib_head_get_exptime(const cpl_propertylist * plist) {
  double result = 0; /* Conversion from electrons to ADUs */

  result=cpl_propertylist_get_double(plist, "EXPTIME");
  cpl_ensure_code(result >= 0, CPL_ERROR_ILLEGAL_OUTPUT);

  return result;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Write the exposure time
  @param    plist        Property list to write to
  @param    exptime      The value to write

  @return   CPL_ERROR_NONE iff okay.
 */
/*-------------------------------------------------------------------------*/
static cpl_error_code
irplib_head_set_exptime(cpl_propertylist *plist, double exptime)
{
   cpl_propertylist_update_double(plist, "EXPTIME", exptime);
   cpl_propertylist_set_comment(plist, "EXPTIME", "Total integration time");

    return cpl_error_get_code();
}

static cpl_imagelist*
irplib_mkmaster_dark_fill_imagelist(const cpl_imagelist* raw_images,
    cpl_propertylist** raw_headers, const cpl_image* master_bias,
    double* mean_exptime) {
  /* First process each input image and store the results in a
   new image list */

  cpl_imagelist* preproc_images = NULL;
  int i = 0;
  double min_exptime = 0;
  double max_exptime = 0;

  preproc_images = cpl_imagelist_new();
  for (i = 0; i < cpl_imagelist_get_size(raw_images); i++) {
    double exposure_time = 0.0;
    cpl_image* current_dark = NULL;
    const cpl_propertylist *current_header;

    current_dark = cpl_image_duplicate(cpl_imagelist_get_const(raw_images, i));
    current_header = raw_headers[i];

    /* Subtract master bias */
    if (master_bias != NULL) {
      cpl_msg_info(cpl_func, "Subtracting master bias");
      cpl_image_subtract(current_dark, master_bias);
    } else {
      cpl_msg_info(cpl_func, "Skipping bias subtraction");
    }

    exposure_time = irplib_head_get_exptime(current_header);

    /* Initialize/update min/max exposure time*/
    if (i == 0 || exposure_time < min_exptime) {
      min_exptime = exposure_time;
    }
    if (i == 0 || exposure_time > max_exptime) {
      max_exptime = exposure_time;
    }

    /* Do not normalize to unit exposure time */
    /*        If this is uncommented, then remember to also calculate the
     correct master dark exposure time below.
     irplib_msg("Normalizing from %f s to unit exposure time", exposure_time);
     check( cpl_image_divide_scalar(current_dark, exposure_time),
     "Error normalizing dark frame");   */

    /* Append to imagelist */
    cpl_imagelist_set(preproc_images, current_dark, i);

    /* Don't deallocate the image. It will be deallocated when
     the image list is deallocated */
    current_dark = NULL;
  }


  /* Check exposure times */
   cpl_msg_info(cpl_func,
       "Exposure times range from %e s to %e s (%e %% variation)", min_exptime,
       max_exptime, 100 * (max_exptime - min_exptime) / min_exptime);

   if ((max_exptime - min_exptime) / min_exptime > .001) {
     cpl_msg_warning(cpl_func, "Exposure times differ by %e %%",
         100 * (max_exptime - min_exptime) / min_exptime);
   }

   /* compute correct exposure time */
   *mean_exptime=0.5 * (max_exptime + min_exptime);
  return preproc_images;
}


cpl_image *
irplib_mdark_process_chip(const cpl_imagelist *raw_images,
    cpl_propertylist **raw_headers, const cpl_image *master_bias,
    cpl_propertylist *mdark_header, const cpl_parameterlist *parameters,
    const char* recipe_id, cpl_table* qclog, const int do_qc,
    const char* STACK_METHOD, const double STACK_KLOW, const double STACK_KHIGH,
    const int STACK_NITER,
    const int pr_num_x, const int pr_num_y,
    const int pr_box_sx, const int pr_box_sy) {
  cpl_image *master_dark = NULL; /* Result */
  cpl_image *current_dark = NULL;
  cpl_imagelist *preproc_images = NULL;
  double mean_exptime = 0;

  /* First process each input image and store the results in a
   new image list */
  preproc_images = irplib_mkmaster_dark_fill_imagelist(raw_images, raw_headers,
      master_bias, &mean_exptime);
  if (do_qc) {
     /* Here we should compute QC but a a better way to log it is TBD  */
    irplib_mkmaster_dark_qc(raw_images, preproc_images, parameters, pr_num_x,
        pr_num_y, pr_box_sx, pr_box_sy, recipe_id, qclog);
     
  }
  /* Get median stack of input darks */
  if (strcmp(STACK_METHOD, "MEDIAN") == 0) {
    cpl_msg_info(cpl_func, "Calculating stack median");
    master_dark = cpl_imagelist_collapse_median_create(preproc_images);
  } else {
    cpl_msg_info(cpl_func, "Calculating stack mean");
    master_dark = irplib_imagelist_ksigma_stack(preproc_images, STACK_KLOW,
        STACK_KHIGH, STACK_NITER);

  }
  irplib_head_set_exptime(mdark_header, mean_exptime );

  cpl_image_delete(current_dark);
  cpl_imagelist_delete(preproc_images);
  if (cpl_error_get_code() != CPL_ERROR_NONE) {
    cpl_image_delete(master_dark);
  }

  return master_dark;
}

/**@}*/

