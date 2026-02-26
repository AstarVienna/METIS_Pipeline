/*
 * hdrl_mode.c
 *
 *  Created on: Mar 1, 2021
 *      Author: amodigli, agabasch
 */

/*
 * This file is part of the HDRL
 * Copyright (C) 2021 European Southern Observatory
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
-----------------------------------------------------------------------------*/
#include "hdrl_mode.h"
#include "hdrl_sigclip.h"
#include "hdrl_utils.h"
#include "hdrl_collapse.h"
#include "hdrl_random.h"

#ifndef _OPENMP
#define omp_get_max_threads() 1
#define omp_get_thread_num() 0
#else
#include <omp.h>
#endif

#include <cpl.h>
//#include <cpl_vector.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <gsl/gsl_multifit.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_histogram.h>
#include <gsl/gsl_poly.h>

static const char * method_to_string(const hdrl_mode_type method)
{
  switch (method) {
    case HDRL_MODE_MEDIAN:
      return "MEDIAN";
      break;
    case HDRL_MODE_WEIGHTED:
      return "WEIGHTED";
      break;
    case HDRL_MODE_FIT:
      return "FIT";
      break;
    default :
      cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
			    "mode method unknown");
      return "";
      break;
  }
}

/* ---------------------------------------------------------------------------*/
/**
  @brief Create parameters for the mode collapse
  @param base_context    base context of parameter (e.g. recipe name)
  @param prefix          prefix of parameter, may be empty string
  @param defaults        default mode parameters
  @return The created parameter list
  Creates a parameterlist containing
      base_context.prefix.histo_min,
      base_context.prefix.histo_max,
      base_context.prefix.bin_size,
      base_context.prefix.method,
      base_context.prefix.error_niter

  @return the allocated parameter structure or NULL in case of error
  The user should make sure that
  base_context != NULL, prefix != NULL, defaults != NULL
 */
/* ---------------------------------------------------------------------------*/
cpl_parameterlist * hdrl_mode_parameter_create_parlist(
    const char           *base_context,
    const char           *prefix,
    const hdrl_parameter *defaults)
{
  cpl_ensure(base_context && prefix && defaults,
	     CPL_ERROR_NULL_INPUT, NULL);

  cpl_ensure(hdrl_collapse_parameter_is_mode(defaults),
	     CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

  cpl_parameterlist * parlist = cpl_parameterlist_new();

  /* --prefix.histo-min */
  hdrl_setup_vparameter(parlist, prefix, ".", "",
			"histo-min", base_context,
			"Minimum pixel value to accept for mode computation",
			CPL_TYPE_DOUBLE,
			hdrl_collapse_mode_parameter_get_histo_min(defaults));

  /* --prefix.histo-max */
  hdrl_setup_vparameter(parlist, prefix, ".", "",
			"histo-max", base_context,
			"Maximum pixel value to accept for mode computation",
			CPL_TYPE_DOUBLE,
			hdrl_collapse_mode_parameter_get_histo_max(defaults));

  /* --prefix.bin-size */
  hdrl_setup_vparameter(parlist, prefix, ".", "",
			"bin-size", base_context,
			"Binsize of the histogram",
			CPL_TYPE_DOUBLE,
			hdrl_collapse_mode_parameter_get_bin_size(defaults));

  /* --prefix.method */
  char  * name ;
  char  * context = hdrl_join_string(".", 2, base_context, prefix);

  hdrl_mode_type method = hdrl_collapse_mode_parameter_get_method(defaults);
  const char * method_def = method_to_string(method);
  name = hdrl_join_string(".", 2, context, "method");
  cpl_free(context);
  cpl_parameter * par = cpl_parameter_new_enum(name, CPL_TYPE_STRING,
          "Mode method (algorithm) to use",
          base_context, method_def, 3, "MEDIAN", "WEIGHTED", "FIT");
  cpl_free(name);
  name = hdrl_join_string(".", 2, prefix, "method");
  cpl_parameter_set_alias(par, CPL_PARAMETER_MODE_CLI, name);
  cpl_parameter_disable(par, CPL_PARAMETER_MODE_ENV);
  cpl_free(name);
  cpl_parameterlist_append(parlist, par);

/*
   --prefix.method
  hdrl_setup_vparameter(parlist, prefix, ".", "",
			"method", base_context,
			"Mode method",
			CPL_TYPE_INT,
			hdrl_collapse_mode_parameter_get_method(defaults));
*/

  /* --prefix.error-niter */
  hdrl_setup_vparameter(parlist, prefix, ".", "",
			"error-niter", base_context,
			"Iterations to compute the mode error",
			CPL_TYPE_INT,
			hdrl_collapse_mode_parameter_get_error_niter(defaults));

  if (cpl_error_get_code()) {
      cpl_parameterlist_delete(parlist);
      return NULL;
  }

  return parlist;
}
/* ---------------------------------------------------------------------------*/
/**
 * @brief parse parameterlist for mode parameters to init corresponding hdrl
 * structure parameters
 *
 * @param parlist    parameter list to parse
 * @param prefix     prefix of parameter name
 * @param histo_min  minimum value of low pixels to use
 * @param histo_max  maximum value of high pixels to be use
 * @param bin_size   size of the histogram bin
 * @param method     method to use for the mode computation
 * @param error_niter  number of iterations to compute the error of the mode
 *
 * @see   hdrl_mode_clip_get_parlist()
 * @return cpl_error_code
 *
 * parameterlist should have been created with
 * hdrl_mode_clip_get_parlist or have the same name hierarchy
 *
 * Developer should make sure that parlist != NULL, prefix != NULL
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_mode_parameter_parse_parlist(
    const cpl_parameterlist *   parlist,
    const char              *   prefix,
    double                  *   histo_min,
    double                  *   histo_max,
    double                  *   bin_size,
    hdrl_mode_type          *   method,
    cpl_size                *   error_niter)
{
  cpl_ensure_code(prefix && parlist, CPL_ERROR_NULL_INPUT);
  char * name;

  if (histo_min) {
      name = hdrl_join_string(".", 2, prefix, "mode.histo-min");
      const cpl_parameter * par = cpl_parameterlist_find_const(parlist, name);
      *histo_min = cpl_parameter_get_double(par);
      cpl_free(name);
  }
  if (histo_max) {
      name = hdrl_join_string(".", 2, prefix, "mode.histo-max");
      const cpl_parameter * par = cpl_parameterlist_find_const(parlist, name);
      *histo_max = cpl_parameter_get_double(par);
      cpl_free(name);
  }

  if (bin_size) {
      name = hdrl_join_string(".", 2, prefix, "mode.bin-size");
      const cpl_parameter * par = cpl_parameterlist_find_const(parlist, name);
      *bin_size = cpl_parameter_get_double(par);
      cpl_free(name);
  }


  /* --method */
  if (method) {
      name = hdrl_join_string(".", 2, prefix, "mode.method");
      const cpl_parameter * par = cpl_parameterlist_find_const(parlist, name) ;
      const char          * tmp_str = cpl_parameter_get_string(par);
      if (tmp_str == NULL) {
	  cpl_free(name);
	  return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
				       "Parameter mode.method not found");
      }
      if(!strcmp(tmp_str, "MEDIAN")) {
	  *method = HDRL_MODE_MEDIAN;
      } else if(!strcmp(tmp_str, "WEIGHTED")) {
	  *method = HDRL_MODE_WEIGHTED;
      } else if(!strcmp(tmp_str, "FIT")) {
	  *method = HDRL_MODE_FIT;
      }
      cpl_free(name) ;

  }

/*
  if (method) {
      name = hdrl_join_string(".", 2, prefix, "mode.method");
      const cpl_parameter * par = cpl_parameterlist_find_const(parlist, name);
      *method = cpl_parameter_get_int(par);
      cpl_free(name);
  }
*/

  if (error_niter) {
      name = hdrl_join_string(".", 2, prefix, "mode.error-niter");
      const cpl_parameter * par = cpl_parameterlist_find_const(parlist, name);
      *error_niter = cpl_parameter_get_int(par);
      cpl_free(name);
  }

  if (cpl_error_get_code()) {
      return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
				   "Error while parsing parameterlist "
				   "with prefix %s", prefix);
  }

  return CPL_ERROR_NONE;
}

/**
  @name  hdrl_mode_compute_binsize
  @memo  determines 'optimal' histogram bin size
  @param vec [input]   data vector
  @doc we adopt Scott's rule for determining the bin size of an histogram,
  multiplied by 2 and with STDDEV replaced by MAD

  see Wikipedia:
  https://en.wikipedia.org/wiki/Histogram#Scott's_normal_reference_rule

  @return histogram bin size
 */
static double
hdrl_mode_compute_binsize(cpl_vector* vec)
{

  cpl_size size = cpl_vector_get_size(vec);
  double mad = 0;
  hcpl_vector_get_mad_window(vec, 1, size, &mad);
  double std_deviation_from_mad = mad * CPL_MATH_STD_MAD;
  /*
  cpl_msg_debug(cpl_func, "mad: %g standard deviation from mad: %16.10f",
  mad, std_deviation_from_mad);
  */

  double binsize = 2. * 3.49 * std_deviation_from_mad / pow(size, 1. / 3. );

  if(binsize <= 0){
        binsize = nextafter(0,1.0);
   }

  return  binsize ;

}
/**
  @name  hdrl_mode_get_nbin
  @memo  computes the number of histogram bins
  @param min  histogram min value
  @param max  histogram max value
  @param size  bin size
 */
static cpl_size
hdrl_mode_get_nbin(const double min, const double max, const double size)
{

  return (cpl_size)floor( (max - min) / size ) + 1;

}

/**
  @name  hdrl_mode_histogram
  @memo  creates histogram
  @param vec [input]   data vector
  @param histo_min [input] min data to be considered in histogram
  @param histo_max [input] max data to be considered in histogram
  @param nbin      [input] number of bins in histogram

  @return GSL histogram or NULL in case of error
  Developer should make sure that nbin > 0

 */
static gsl_histogram *
hdrl_mode_histogram(const cpl_vector* vec, const double histogram_min,
		    const double histogram_max, const cpl_size nbin)
{

  cpl_error_ensure(nbin > 0, CPL_ERROR_ILLEGAL_INPUT,
  		   return NULL, "Number of bins must be > 0");

  cpl_error_ensure(histogram_max > histogram_min, CPL_ERROR_ILLEGAL_INPUT,
  		   return NULL, "histo_max must be larger than histo_min");

  gsl_histogram * histogram = gsl_histogram_alloc(nbin);

  gsl_histogram_set_ranges_uniform (histogram, histogram_min, histogram_max);

  const cpl_size size = cpl_vector_get_size(vec);
  const double* data = cpl_vector_get_data_const(vec);
  for(cpl_size i = 0; i < size; i++) {

      gsl_histogram_increment(histogram, data[i]);

  }

  return histogram;
}
/*-------------------------------------------------------------------------*/
/**
  @name  hdrl_mode_histogram_to_table
  @memo  computes histogram
  @param histogram [input]   histogram created by GSL
  @param histo_min [input] min data to be considered in histogram
  @param histo_step [input] histogram sampling step
  @param nbin      [input] number of bins in histogram

  @return output histogram: stored in a table with results in columns:
        # BIN: data values corresponding to each bin number
        # INTERVAL_LOWER: data values corresponding to the (lower) intensity of
                          each bin
        # INTERVAL_UPPER: data values corresponding to the (upper) intensity of
                          each bin
        # COUNTS: column counting how many data have intensity values
            corresponding to each bin intensity range

  @note this routine is useful for debugging (and visualization) purposes
 */

static cpl_table *
hdrl_mode_histogram_to_table(gsl_histogram* histogram,
			     const double histogram_min,
			     const double histogram_step, const cpl_size nbin){

  cpl_table* table = cpl_table_new(nbin);

  cpl_table_new_column(table, "BIN", CPL_TYPE_DOUBLE);
  cpl_table_new_column(table, "INTERVAL_LOWER", CPL_TYPE_DOUBLE);
  cpl_table_new_column(table, "INTERVAL_UPPER", CPL_TYPE_DOUBLE);
  cpl_table_new_column(table, "COUNTS", CPL_TYPE_DOUBLE);

  cpl_table_fill_column_window(table, "BIN", 0, nbin, 0);
  cpl_table_fill_column_window(table, "INTERVAL_LOWER", 0, nbin, 0);
  cpl_table_fill_column_window(table, "INTERVAL_UPPER", 0, nbin, 0);
  cpl_table_fill_column_window(table, "COUNTS", 0, nbin, 0);

  double* phb = cpl_table_get_data_double(table, "BIN");
  double* phl = cpl_table_get_data_double(table, "INTERVAL_LOWER");
  double* phu = cpl_table_get_data_double(table, "INTERVAL_UPPER");
  double* phc = cpl_table_get_data_double(table, "COUNTS");

  //cpl_msg_debug(cpl_func,"histogram bin step: %16.10f", histo_step);
  for(cpl_size i = 0; i < nbin; i++) {
      phb[i] = (double)i;
      phl[i] = histogram_min + phb[i] * histogram_step;
      phu[i] = phl[i] + histogram_step;
      phc[i] = histogram->bin[i];
  }
  return table;
}
/*----------------------------------------------------------------------------*/
/**
 @brief     polynomial fit (wrap for the gsl functions)
 @param         data_x            X data to be fitted
 @param         data_y            Y data to be fitted
 @param         errs_y            errors on Y
 @param         n_sampling_points number of sampling points
 @param         poly_degree       polynomial degree
 @param[out]    fit               fit result
 @param[out]    coeffs_val        coeffs of the fit
 @param[out]    coeffs_err        errors on coeffs of the fit
 @param[out]    chisq             CHI2 of the fit
 @return   CPL_ERROR_NONE iff OK

 @note: HDRL scales covar by chisq_loc to get results as reference python code
 by mode project requester

 coefficients, errors (including covariance) are provided by GSL
 */
/*----------------------------------------------------------------------------*/
static gsl_matrix *
hdrl_gsl_fit_poly (const double* data_x,
		   const double* data_y,
		   const double* errs_y,
		   const size_t n_sampling_points,
		   const cpl_size poly_degree,
		   double *fit,
		   double *coeffs_val,
		   double *coeffs_err,
		   double *chisq) {

  cpl_size i, j;
  double err, chisq_loc;
  gsl_vector *x = NULL, *y = NULL, *w = NULL, *p = NULL;
  gsl_matrix *X = NULL, *covar = NULL;

  //double *fit;
  gsl_multifit_linear_workspace *work = NULL;

  x = gsl_vector_alloc (n_sampling_points);
  y = gsl_vector_alloc (n_sampling_points);
  w = gsl_vector_alloc (n_sampling_points);
  p = gsl_vector_alloc (poly_degree);
  X = gsl_matrix_alloc (n_sampling_points, poly_degree);
  covar = gsl_matrix_alloc (poly_degree, poly_degree);

  for (i = 0; i < (cpl_size) n_sampling_points; i++) {
      gsl_vector_set (x, i, data_x[i]);
      gsl_vector_set (y, i, data_y[i]);
      err = errs_y[i];

      gsl_vector_set (w, i, 1.0 / err / err);

      for (j = 0; j < poly_degree; j++)
	gsl_matrix_set (X, i, j, gsl_pow_int(gsl_vector_get(x, i), j));
  }

  work = gsl_multifit_linear_alloc (n_sampling_points, poly_degree);
  gsl_multifit_wlinear (X, w, y, p, covar, &chisq_loc, work);
  gsl_multifit_linear_free (work);

  for (i = 0; i < (cpl_size) n_sampling_points; i++) {

      fit[i] = 0.;
      for (j = 0; j < poly_degree; j++)
	fit[i] += gsl_matrix_get (X, i, j) * gsl_vector_get (p, j);
  }

  *chisq = chisq_loc/(n_sampling_points - poly_degree);
  /*
   * NOTE we scale covar arbitrarily by chisq_loc to get results as
   * python code by Lodo
   */
  for (j = 0; j < poly_degree; j++) {
      gsl_matrix_set (covar, j, j, gsl_matrix_get (covar, j, j)*chisq_loc);
      coeffs_val[j] = gsl_vector_get (p, j);
      coeffs_err[j] = sqrt(gsl_matrix_get (covar, j, j));
  }

  /*
  cpl_msg_debug(cpl_func,"GSL poly fit c0: %16.8f c1: %16.8f, c2: %16.8f",
		    coeffs_val[0], coeffs_val[1], coeffs_val[2]);
  cpl_msg_debug(cpl_func,"GSL poly fit e0: %16.8f e1: %16.8f, e2: %16.8f",
		    coeffs_err[0], coeffs_err[1], coeffs_err[2]);
  cpl_msg_debug(cpl_func,"GSL poly fit chi2: %16.8f", *chisq);
  */

  gsl_vector_free(x);
  gsl_vector_free(y);
  gsl_vector_free(w);
  gsl_vector_free(p);
  gsl_matrix_free(X);


  return covar;
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief   Trim vector keeping values in interval [min,max]
  @param   vec         The vector for which mean is to be computed.
  @param   min         Min value to keep
  @param   max         Max value to keep

  @return   cpl_vector* the trimmed vector or NULL in case of error
  The input vec shall have size > 0

 */

static cpl_vector*
hdrl_mode_vector_trim(const cpl_vector* vec, const double min,
		       const double max) {
  cpl_size size = cpl_vector_get_size(vec);
  cpl_error_ensure(size > 0, CPL_ERROR_ILLEGAL_INPUT,
		   return NULL, "vector size must be > 0");
  cpl_vector * vec_trim = cpl_vector_new(size);
  const double     * pvec = cpl_vector_get_data_const(vec);
  double     * pvec_trim = cpl_vector_get_data(vec_trim);

  cpl_size index = 0;

  for (cpl_size i = 0; i < size; i++) {
      if (pvec[i] >= min && pvec[i] <= max) {
	  pvec_trim[index] = pvec[i];
	  index++;
      } else {
	  continue;
      }
  }

  if (index > 0) {
      /* return resized vector */
      cpl_vector_set_size(vec_trim, index);
      return vec_trim;
  } else {
      /* return NULL */
      cpl_vector_delete(vec_trim);
      return NULL;
  }
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief   Compute mode using median method.
  @param   vec         The vector for which mean is to be computed.
  @param   histo_min   Min of histogram
  @param   histo_max   Max of histogram
  @param   nbin        number of histogram bins
  @param   error_niter number of iterations if error is computed with Montecarlo
  @param [out]  mode_median     The mode
  @param [out]  mode_median_err The error associated to the mode
  @return   @c CPL_ERROR_NONE or the appropriate error code.

  The input vector shall have size > 0

  @note: method to be used for very asymmetric (for example Gamma) functions

 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_mode_median(
    const cpl_vector        * vec,
    const double        histo_min,
    const double        histo_max,
    const cpl_size      nbin,
    const cpl_size      error_niter,
    double            * mode_median,
    double            * mode_median_err)
{


  cpl_error_ensure(vec != NULL, CPL_ERROR_NULL_INPUT,
		   return CPL_ERROR_NULL_INPUT, "Null input vector data");

  gsl_histogram * histogram = hdrl_mode_histogram(vec, histo_min, histo_max,
						  nbin);

  cpl_error_ensure(histogram != NULL, CPL_ERROR_NULL_INPUT,
		   return CPL_ERROR_NULL_INPUT, "Histogram can not be created");

  /* for debug purposes
  cpl_table* t = hdrl_mode_histogram_to_table(h, histo_min, nbin);
  cpl_table_save(t,NULL,NULL,"debug.fits",CPL_IO_CREATE);
  */

  cpl_size histogram_bin_max = gsl_histogram_max_bin(histogram);
  //cpl_msg_debug(cpl_func, "histogram bin max: %lld", histogram_bin_max);

  /* get median and error associated to the values in the histogram max bin */

  /* to compute the mode we need to determine the median of the values in the
   * histogram bin max. The error is the standard deviation of these values.
   * */
  double lower = 0.;
  double upper = 0.;
  gsl_histogram_get_range(histogram, histogram_bin_max, &lower, &upper);

  cpl_vector * vec_final = hdrl_mode_vector_trim(vec, lower, upper);
  *mode_median = cpl_vector_get_median(vec_final);

  if(error_niter == 0) {
      *mode_median_err = cpl_vector_get_stdev(vec_final);
      cpl_msg_debug(cpl_func, "(method median) computed mode: %g, associated error: %g",
		    *mode_median, *mode_median_err);
  } else {
      /* Derive only the analytical error, if requested */
      *mode_median_err = 0;
  }

  /* free memory */
  gsl_histogram_free(histogram);
  cpl_vector_delete(vec_final);

  return cpl_error_get_code();
}
/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief   Compute mode using weight method.
  @param   vec         The vector for which mean is to be computed.
  @param   histo_min   Min of histogram
  @param   histo_max   Max of histogram
  @param   bin_size    histogram bin size
  @param   nbin        number of histogram bins
  @param   error_niter number of iterations if error is computed with Montecarlo
  @param [out]  mode_weight     The mode
  @param [out]  mode_weight_err The error associated to the mode

  @return   @c CPL_ERROR_NONE or the appropriate error code.

  The input vector shall have size > 0

  @note: method appropriate for distribution with moderate asymmetry

 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_mode_weight(
    const cpl_vector        * vec,
    const double        histo_min,
    const double        histo_max,
    const double        bin_size,
    const cpl_size      nbin,
    const cpl_size      error_niter,
    double            * mode_weight,
    double            * mode_weight_err)
{

  cpl_error_ensure(vec != NULL, CPL_ERROR_NULL_INPUT,
		   return CPL_ERROR_NULL_INPUT, "Null input vector data");

  gsl_histogram * histogram = hdrl_mode_histogram(vec, histo_min, histo_max,
						  nbin);

  cpl_error_ensure(histogram != NULL, CPL_ERROR_NULL_INPUT,
		   return CPL_ERROR_NULL_INPUT, "Histogram can not be created");

  cpl_table* table = hdrl_mode_histogram_to_table(histogram, histo_min,
						  bin_size, nbin);
  //cpl_table_save(table, NULL, NULL, "histo_tab.fits", CPL_IO_DEFAULT);

  double histogram_max = gsl_histogram_max_val(histogram);
  //cpl_msg_debug(cpl_func, "histogram value at max: %g", histogram_max);
  cpl_size histogram_bin_max = gsl_histogram_max_bin(histogram);
  //cpl_msg_debug(cpl_func, "histogram bin max: %lld", histogram_bin_max);
  //cpl_msg_debug(cpl_func, "histogram bin size: %ld", gsl_histogram_bins(histogram));
  if(histogram_bin_max > 0 &&
      (histogram_bin_max < ((cpl_size)gsl_histogram_bins(histogram) - 1))) {
      cpl_msg_debug(cpl_func, "histogram (bin_max-1) value: %16.8g",
		    gsl_histogram_get(histogram,histogram_bin_max-1));
      cpl_msg_debug(cpl_func, "histogram (bin_max+1) value: %16.8g",
		    gsl_histogram_get(histogram,histogram_bin_max+1));
  }

  /* determine the mean of the values that lead to the histogram bin max */
  double lower = 0;
  double upper = 0;
  gsl_histogram_get_range(histogram, histogram_bin_max, &lower, &upper);
  //cpl_msg_debug(cpl_func, "histogram range lower: %16.8g upper: %16.8g", lower, upper);
  cpl_table_and_selected_double(table,"COUNTS",CPL_EQUAL_TO, histogram_max);
  cpl_table* extract = cpl_table_extract_selected(table);

  //cpl_table_save(extract, NULL, NULL, "histo_tab_extr1.fits", CPL_IO_DEFAULT);

  double mean = cpl_table_get_column_mean(extract, "INTERVAL_LOWER");

  cpl_table_delete(extract);

  cpl_size max_pos = 0;
  cpl_table_get_column_maxpos(table, "INTERVAL_LOWER", &max_pos);

  cpl_table_delete(table);

  double freq1 = histogram_max;
  double level1 = mean;

  double freq0, freq2;
  //cpl_msg_debug(cpl_func, "nbin: %lld histogram_bin_max: %lld", nbin,histogram_bin_max);
  if(histogram_bin_max + 1 <= ((cpl_size)nbin - 1)) {
      freq2 = gsl_histogram_get(histogram, histogram_bin_max + 1);
  } else {
      freq2 = 0;
  }


  if(histogram_bin_max > 0) {
      freq0 = gsl_histogram_get(histogram, histogram_bin_max - 1);
  } else {
      freq0 = 0;
  }

  /*
  cpl_msg_debug(cpl_func, "Frequencies: f0: %16.8g f1: %16.10g f2: %16.8g l1: %16.10g",
		freq0, freq1, freq2, level1);
		*/

  double diff1 = freq1 - freq0;
  double diff2 = freq1 - freq2;
  double factor = diff1 / (diff1 + diff2);
  if( factor == 0 || isnan(factor) ) {
      factor = 0.5;
  }
  /* compute mode */
  *mode_weight = level1 + bin_size * factor;

  /* compute error associated to mode */
  if (error_niter == 0) {
       double dd1 = sqrt(freq0 + freq1);
       double dd2 = sqrt(freq1 + freq2);
       double dw2=1;

       double term1 = diff2 * dd1 / pow((diff1 + diff2), 2);
       double term2 = diff1 * dd2 / pow((diff1 + diff2), 2);
       dw2 = pow(term1, 2.) + pow(term2, 2.);
       *mode_weight_err = bin_size * sqrt(dw2);
       /*
       cpl_msg_debug(cpl_func,"fct: %16.8g m: %16.8g, dd1: %16.8g dd2: %16.8g",
     		factor,*mode_weight,dd1,dd2);
       cpl_msg_debug(cpl_func,"dw2: %16.8g e: %16.8g", dw2, *mode_weight_err);
       */

   } else {
      /* Derive only the analytical error, if requested */
      *mode_weight_err = 0;
   }

  cpl_msg_debug(cpl_func, "(method weight) computed mode: %16.10g error:  %16.10g",
		*mode_weight, *mode_weight_err);

  /* free memory */
  gsl_histogram_free(histogram);

  return cpl_error_get_code();
}
/**
  @internal
  @brief   compute analytical error associated to fit method
  @param   coeffs_val    fit coefficient values.
  @param   coeffs_err    error on fit coefficient values.
  @param   covar         covariance
  @param   scale_factor  scaling factor to align GSL results to python ones
    */
static double
hdrl_mode_fit_analytical_error(const double* coeffs_val,
			       const double* coeffs_err, gsl_matrix* covar,
			       const double scale_factor){


  double a2 = coeffs_val[2];
  double a1 = coeffs_val[1];
  //double a0 = coeffs_val[0];

  /*NOTE we scale arbirtarily by scale_factor = chi2 / dof, where dof is the
   * degree of freedom to get results as python code by Lodo
   */
  double cvar_matrix = gsl_matrix_get (covar, 2, 1) * scale_factor;

  double da2 = coeffs_err[2];
  double da1 = coeffs_err[1];
  //double da0 = coeffs_err[0];

  double cvr_term = 2 * (-1. / (2 * a2)) * (a1 / (2 * a2 * a2)) * cvar_matrix;

  double add1 = (da1 / (2 * a2));
  add1 *= add1;
  double a_square = a2 * a2;
  double add2 = (a1 * da2 / (2 * a_square ));
  add2 *= add2;
  double err2 = add1 + add2;

  err2 +=  cvr_term;

  return sqrt(err2);;
}

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief   Compute mode using fit method.
  @param   vec         The vector for which mean is to be computed.
  @param   histo_min   Min of histogram
  @param   histo_max   Max of histogram
  @param   bin_size    histogram bin size
  @param   nbin        number of histogram bins
  @param   error_niter number of iterations if error is computed with Montecarlo
  @param [out]  mode_fit     The mode
  @param [out]  mode_fit_err The error associated to the mode

  @return   @c CPL_ERROR_NONE or the appropriate error code.

  The developer provide vector != NULL

  @note: method to be used with almost symmetric distributions
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code hdrl_mode_fit(
    const cpl_vector        * vec,
    const double        histo_min,
    const double        histo_max,
    const double        bin_size,
    const cpl_size      nbin,
    const cpl_size      error_niter,
    double            * mode_fit,
    double            * mode_fit_err)
{

  cpl_error_ensure(vec != NULL, CPL_ERROR_NULL_INPUT,
		   return CPL_ERROR_NULL_INPUT, "Null input vector data");

  /* We  use semi_fit_window = 2 as the fit polynomial degree used is 2
   * We cannot use a larger value to prevent asymmetries (for asymmetric
   * distributions) that cannot be fit well by a parabola */
  cpl_size semi_fit_window = 2;

  double hmin = histo_min;
  double hmax = histo_max;

  gsl_histogram * h = hdrl_mode_histogram(vec, hmin, hmax, nbin);

  cpl_error_ensure(h != NULL, CPL_ERROR_NULL_INPUT,
		   return CPL_ERROR_NULL_INPUT, "Histogram can not be created");

  cpl_size histogram_bin_max = (cpl_size) gsl_histogram_max_bin(h);
  cpl_size histo_nbins = (cpl_size) gsl_histogram_bins(h);

  if(histogram_bin_max > 0) {
      cpl_msg_debug(cpl_func, "histogram (bin_max-1) value: %16.8g",
		    gsl_histogram_get(h, histogram_bin_max - 1));
  }
  if(histogram_bin_max < (histo_nbins-1)) {
      cpl_msg_debug(cpl_func, "histogram (bin_max+1) value: %16.8g",
		    gsl_histogram_get(h, histogram_bin_max + 1));
  }

  double lower = 0.;
  double upper = 0.;
  gsl_histogram_get_range(h, gsl_histogram_max_bin(h), &lower, &upper);
  double value_at_max = lower;

  /* check if one has enough points (at least 3) to be able to do a polynomial
   * fit. If too less points, switch to weight mode
   */
  if( histo_nbins < 3 ) {
      cpl_error_set(cpl_func, CPL_ERROR_ILLEGAL_INPUT);
      cpl_msg_info(cpl_func,"Cannot do polynomial fit with less than 3 points.");
      gsl_histogram_free(h);
      return cpl_error_get_code();
  } else {
      cpl_size mn = histogram_bin_max - semi_fit_window;
      cpl_size mx = histogram_bin_max + semi_fit_window;
      mn = histogram_bin_max - semi_fit_window;
      mn = (mn > 0) ? mn : 0;
      mx = histogram_bin_max + semi_fit_window;
      mx = (mx < histo_nbins) ? mx : histo_nbins-1; // Cannot go till histo_nbins else after for loop seg fault

      /* fit a parabola */
      const cpl_size good_points = (mx - mn + 1);
      const cpl_size win_size = 2 * semi_fit_window + 1;
      const cpl_size sz = (good_points < win_size) ? good_points : win_size;

      double* x_vals = cpl_calloc(sz, sizeof(double));
      double* y_vals = cpl_calloc(sz, sizeof(double));
      double* y_errs = cpl_calloc(sz, sizeof(double));
      cpl_size j = 0;
      /* we assume uniform error as we do not know how to compute the error
       * associated to each bin size
       */
      for(cpl_size i = mn; i <= mx; i++) {
	  double lower_local = 0.;
	  double upper_local = 0.;
	  gsl_histogram_get_range(h, i, &lower_local, &upper_local);
	  x_vals[j] = lower_local;
	  y_vals[j] = gsl_histogram_get(h, i);
	  y_errs[j] = 1;
	  j++;
      }

      /* do the polynomial fit, using GSL */
      cpl_size deg = 2;
      double chi2;
      double* coeffs_val = (double *) cpl_calloc (sz, sizeof(double));
      double* coeffs_err = (double *) cpl_calloc (sz, sizeof(double));
      double* fit = (double *) cpl_calloc (sz, sizeof(double));
      gsl_matrix* covar =
	  hdrl_gsl_fit_poly (x_vals, y_vals, y_errs, sz, deg+1, fit, coeffs_val,
			     coeffs_err, &chi2);

      double m = -coeffs_val[1] / 2. / coeffs_val[2];
      double value_at_mode = gsl_poly_eval(coeffs_val, sz, m);

      *mode_fit = m + bin_size / 2.;

      /* check that we are not at an edge and that we found a true maximum
       * (that means the point distribution had a proper shape)
       */
      double y_min_val = gsl_poly_eval(coeffs_val, sz,x_vals[0]);
      double y_max_val = gsl_poly_eval(coeffs_val, sz,x_vals[sz - 1]);
      double max_parab = (y_max_val > y_min_val ) ? y_max_val : y_min_val;
      cpl_boolean is_not_supported_fit = CPL_FALSE;
      if( (fabs(value_at_max - m) > bin_size / 2.) ||
	  (value_at_mode < max_parab ) ) {
	  /* if we are near an edge point or the point distribution is not
	   * proper it is safer to use the weight method to get the mode
	   */
	  if( fabs(value_at_max - m) > bin_size / 2. ) {
	      cpl_error_set(cpl_func, CPL_ERROR_ILLEGAL_INPUT);
	      cpl_msg_info(cpl_func, "Max too close to point distribution edge: abs(value_at_max+bin_size/2-m) > bin_size");
	      is_not_supported_fit = CPL_TRUE;
	  }

	  if(value_at_mode < max_parab) {
	      cpl_error_set(cpl_func, CPL_ERROR_ILLEGAL_INPUT);
	      cpl_msg_info(cpl_func, "Value at mode is not a valid maximum: value_at_mode < np.max(parab)");
	      is_not_supported_fit = CPL_TRUE;
	  }
	  if(is_not_supported_fit == CPL_TRUE) {
	      gsl_matrix_free(covar);
	      gsl_histogram_free(h);
	      cpl_free(fit);
	      cpl_free(coeffs_val);
	      cpl_free(coeffs_err);
	      cpl_free(y_errs);
	      cpl_free(x_vals);
	      cpl_free(y_vals);
	      return cpl_error_get_code();
	  }

      } else {
	  if (error_niter == 0) {
	      /* As requested, compute the analytical error, */
	      double dof = sz - (deg +1);
	      double scale_factor = (chi2 / dof);

	      *mode_fit_err =
		  hdrl_mode_fit_analytical_error(coeffs_val, coeffs_err, covar,
						 scale_factor);
	  } else {
	      /* As requested, do not compute the analytical error */
	      *mode_fit_err = 0;
	  }

      }
      /* check if problem occurred during poly fit or in error computation*/
      if(!isfinite(*mode_fit_err) || !isfinite(*mode_fit))  {
	  cpl_error_set(cpl_func, CPL_ERROR_ILLEGAL_OUTPUT);
	  *mode_fit_err = NAN;
	  *mode_fit = NAN;
      }
      cpl_msg_debug(cpl_func, "(method fit) computed mode: %16.10g err: %16.10g ",
			*mode_fit, *mode_fit_err);

      gsl_matrix_free(covar);
      cpl_free(fit);
      cpl_free(coeffs_val);
      cpl_free(coeffs_err);
      cpl_free(x_vals);
      cpl_free(y_vals);
      cpl_free(y_errs);

  }

  /* free memory */
  gsl_histogram_free(h);

  return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @brief   Compute mode of data.
  @param   source      The data for which mode is to be computed.
  @param   histo_min   Min of histogram
  @param   histo_max   Max of histogram
  @param   bin_size    the histogram bin size
  @param   method      the mode computation method
  @param   error_niter the number of iterations to determine error model
  @param [out]  mode      The mode
  @param [out]  mode_error The error associated to the mode
  @param [out]  naccepted    the number of bins used to determine mode

  @return   @c CPL_ERROR_NONE or the appropriate error code.

  The developer provide source != NULL

  @doc
  The mode (or modal value) of a distribution of values is defined as the value
  of the distribution that appears more often (the maximum of the function
  distribution).
  This function computes the mode and the associated error for the case of a
  discrete distribution of points as defined by the input source image.

  In  the  case  of  a  discrete  distribution,  one  can  define
  the ``modal class'' (the position of the highest peak of the histogram), and
  the ``modal frequency" (the value of the highest peak of the histogram), which
  are related to the mode of the underlying continuous distribution.
  The values of histo_min/max can be changed to remove outliers or optimise
  computation time.
  The value
  of the modal frequency depends from the chosen bin size used to create the
  histogram and from statistical noise.  Large bin size helps in finding an high
  and unique modal frequency, but decreases the resolution because the bin is
  large.  A small bin increases the resolution of the modal class but decreases
  the modal frequency and therefore increases the chance that, because of the
  statistical noise, the highest peak in the histogram is not related to the
  mode of the underlying distribution.

  The mode can be computed with three different methods:

  fit:   the distribution function maximum is obtained via a parabolic fit after
         making some check (that the distribution is sufficiently symmetric)

  weight: the distribution function maximum is obtained by an ad hoc weight of
          the histogram bins adjacent to the maximum. This method is appropriate
          for point distributions that are not perfectly symmetric.

  median: the distribution function maximum is obtained by computing the median
          of the data points.

  The error computation can be computed analytically (if error_miter=0) or
  using a Montecarlo simulation (if error_miter > 0)

  Results depend additionally on following parameters:

  bin_size:
     if bin_size <=0
        bin_size is automatically determined. as
        bin_size = 2. * 3.49 * stdev / pow(size, 1. / 3. ) ;
        where stdev is a robust estimation of the standard deviation obtained
        from the MAD of the image

  histo_min:
     if histo_min < histo_max
        histo_min if histo_min < histo_max
     else
        histo_min = is equal min(distribution) - 0.5 * bin_size

  histo_max:
     if histo_min < histo_max
        histo_max = histo_min + nbin * bin_size
     else
        histo_min = is equal max(distribution) + 0.5 * bin_size
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_mode_clip_image(
    const cpl_image         * source,
    const double              histo_min,
    const double              histo_max,
    const double              bin_size,
    const hdrl_mode_type      method,
    const cpl_size            error_niter,
    double                  * mode,
    double                  * mode_error,
    cpl_size                * naccepted)
{
  cpl_vector * vec_source = NULL;

  /* Check Entries */
  cpl_error_ensure(source != NULL, CPL_ERROR_NULL_INPUT,
		   return CPL_ERROR_NULL_INPUT, "Null input source image!");

  cpl_image* data =(cpl_image*) source;

  /* compress images to vectors excluding the bad pixels */
  vec_source = hdrl_image_to_vector(data, cpl_image_get_bpm_const(source));

  if (vec_source != NULL) {
      /* compute mode */
      hdrl_mode_clip(vec_source, histo_min, histo_max, bin_size, method,
		     error_niter, mode, mode_error, naccepted);

      if(error_niter > 0) {
        /* Calculate the error using the bootstrap technique */
        hdrl_mode_bootstrap(vec_source, histo_min, histo_max, bin_size,
			      method, error_niter, mode_error);
      }

      if (CPL_ERROR_NONE != cpl_error_get_code()) {
	  /* if error occurred exit after cleaning memory */
   	  cpl_vector_delete(vec_source);
   	  return cpl_error_get_code();
      }

  } else {
      /* no good pixels */
      *mode = NAN;
      *mode_error = NAN;
      cpl_error_set(cpl_func, CPL_ERROR_ILLEGAL_INPUT);
  }

  cpl_vector_delete(vec_source);
  return cpl_error_get_code();
}


/*----------------------------------------------------------------------------*/
/**
  @brief   Compute mode of data.
  @param   vec         The data for which mode is to be computed.
  @param   histo_min   Min of histogram
  @param   histo_max   Max of histogram
  @param   bin_size    the histogram bin size
  @param   method      the mode computation method
  @param   error_niter the number of iterations to determine error model
  @param [out]  mode     The mode
  @param [out]  mode_error The error associated to the mode
  @param [out]  naccepted    the number of good data points

  @return   @c CPL_ERROR_NONE or the appropriate error code.

  The developer provide vec != NULL
  @doc: see hdrl_mode_clip_image

 */
/*----------------------------------------------------------------------------*/

cpl_error_code hdrl_mode_clip(
    cpl_vector              * vec,
    const double              histo_min,
    const double              histo_max,
    const double              bin_size,
    const hdrl_mode_type      method,
    const cpl_size            error_niter,
    double                  * mode,
    double                  * mode_error,
    cpl_size                * naccepted)
{
  cpl_error_ensure(vec != NULL, CPL_ERROR_NULL_INPUT,
 		   return CPL_ERROR_NULL_INPUT, "Null input source image!");

  *naccepted = 0; /* In case an error happens - if not, re-set it later */

  cpl_vector   * loc_vec = NULL;
  double bsize = bin_size;

  /* if binsize <=0 : the routine derives the value from a formula */
  if (bsize <= DBL_EPSILON) {
      bsize = hdrl_mode_compute_binsize(vec);
  }

  double hmin = histo_min;
  double hmax = histo_max;
  cpl_size nbin = 0;
  cpl_vector* trim_vec = NULL;
  /* if histo_min >= histo_max: the routine derives the two values from the
   * data (and also the number of histogram bins, nbin)
   */
  if ( histo_min >= histo_max) {

      trim_vec = cpl_vector_duplicate(vec);
      hmin = cpl_vector_get_min(vec) - bsize / 2;
      hmax = cpl_vector_get_max(vec) + bsize / 2;
      /* the following two are required to have same results as Lodo's code
       * as implemented in the unit tests
       */
      nbin = hdrl_mode_get_nbin(hmin, hmax, bsize);
      hmax = hmin + (nbin * bsize);
      /* Fallback solution for the case there is only one single value */
      if(hmin == hmax) {
	  hmin = nextafter(hmin, hmin - FLT_EPSILON);
	  hmax = nextafter(hmax, hmax + FLT_EPSILON);
	  bsize = nextafter(0.0, 1.0);
	  nbin = 1 ;
      }


  } else {
      nbin = hdrl_mode_get_nbin(hmin, hmax, bsize);
      trim_vec = hdrl_mode_vector_trim(vec, hmin, hmax);
      if( hmin + (nbin * bsize)  >= hmax) {
	  /* adjust hmax to use gsl_histogram without changing bsize */
	  //cpl_msg_warning(cpl_func,"Strange");
	  hmax = hmin + (nbin * bsize);
      }
  }
  cpl_msg_debug(cpl_func,"Histogram bin size: %g min: %g max: %g number of bins: %lld",
		bsize, hmin, hmax, nbin);

  cpl_error_ensure(trim_vec != NULL, CPL_ERROR_NULL_INPUT,
		   return CPL_ERROR_NULL_INPUT, "No data for mode "
		       "computation. Try to change mode parameters ... ");

  loc_vec = trim_vec;


  /* method == enum with mode_fit, mode_weight, mode_median */
  switch(method) {

    case HDRL_MODE_FIT:

      if (CPL_ERROR_NONE != hdrl_mode_fit(loc_vec, hmin, hmax, bsize, nbin, error_niter, mode,
		    mode_error) ) {
	  //cpl_error_set(cpl_func,CPL_ERROR_ILLEGAL_INPUT);
          cpl_msg_info(cpl_func,"Mode computation failed using method fit. Try method weight or median.");
      }
      break;

    case HDRL_MODE_WEIGHTED:

      if (CPL_ERROR_NONE != hdrl_mode_weight(loc_vec, hmin, hmax, bsize, nbin,
					     error_niter, mode, mode_error) ) {
	  //cpl_error_set(cpl_func,CPL_ERROR_UNSUPPORTED_MODE);
	  cpl_msg_info(cpl_func,"Mode computation failed using method weight. Try method fit or median.");

      }
      break;

    case HDRL_MODE_MEDIAN:

      if (CPL_ERROR_NONE != hdrl_mode_median(loc_vec, hmin, hmax, nbin,
					     error_niter, mode, mode_error) ) {
	  //cpl_error_set(cpl_func,CPL_ERROR_UNSUPPORTED_MODE);
	  cpl_msg_info(cpl_func,"Mode computation failed using method median. Try method fit or weight");

      }

      break;

    default:
      cpl_msg_info(cpl_func,"Unsupported mode method. Supported methods are: fit, weight, median");
      return CPL_ERROR_UNSUPPORTED_MODE;

  }

  *naccepted = cpl_vector_get_size(vec);
  cpl_vector_delete(trim_vec);

  return cpl_error_get_code();
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief uses Montecarlo simulations based on the bootstrap technique to
 * determine the error of the mode of the underlying distribution
 *
 * @param vec        input vector for the bootstrap simulations
 * @param histo_min  minimum value of low pixels to use in the histogram
 * @param histo_max  maximum value of high pixels to be use in the histogram
 * @param bin_size   size of the histogram bin
 * @param method     method to use for the mode computation
 * @param error_niter  number of iterations to compute the error of the mode
 * @param mode_error   returned error of the mode
 *
 * @return cpl_error_code
 *
 * The returned mode error is the mad-based standard deviation of the
 * simulations where simulations that did not converge where excluded a priori.
 *
 */
/* ---------------------------------------------------------------------------*/

cpl_error_code
hdrl_mode_bootstrap(
    const cpl_vector        * vec,
    const double              histo_min,
    const double              histo_max,
    const double              bin_size,
    const hdrl_mode_type      method,
    const cpl_size            error_niter,
    double                  * mode_error)
{


  /* To ensure new random numbers each time the compiled program is called,
   * call srand(), e.g. srand(time(NULL)) */

  /* Predefine the state for the threads as they get manipulated in the thread*/
  hdrl_random_state ** pstate = cpl_calloc(omp_get_max_threads(),
					  sizeof(hdrl_random_state *));

  for(cpl_size i = 0 ; i < (cpl_size)omp_get_max_threads(); i++){
      uint64_t seed[2] = {(uint64_t)rand(), (uint64_t)rand()};
      pstate[i] = hdrl_random_state_new(1, seed);
  }

  cpl_size       vec_size = cpl_vector_get_size(vec);
  const double * pvec     = cpl_vector_get_data_const(vec);
  cpl_image    * ima_mode = cpl_image_new(1, error_niter, CPL_TYPE_DOUBLE);
  double       * pima_mode = cpl_image_get_data_double(ima_mode);

  /* In case the loop gets parallelized use cpl_image_get_bpm outside the loop
   * once to get a single bad pixel mask for the image */
  cpl_binary * pima_mode_bpm = cpl_mask_get_data(cpl_image_get_bpm(ima_mode));

  HDRL_OMP(omp parallel for)
  for (cpl_size i = 0; i < error_niter; i++) {
      cpl_error_code err = CPL_ERROR_NONE;
      cpl_vector * vec_simul = cpl_vector_new(vec_size);
      double   mode = 0.;
      double   mode_err = 0.;
      cpl_size naccepted = 0;
      double * pvec_simul = cpl_vector_get_data(vec_simul);

      /* simulate the new vector by bootstraping */
      for (cpl_size j = 0; j < vec_size; j++) {
	  pvec_simul[j] = pvec[(cpl_size)hdrl_random_uniform_int64(pstate[omp_get_thread_num()], 0, vec_size - 1)];
      }

      err = hdrl_mode_clip(vec_simul, histo_min, histo_max, bin_size, method,
			   -1, &mode, &mode_err, &naccepted);
      cpl_vector_delete(vec_simul);

      if (err == CPL_ERROR_NONE) {
	  pima_mode[i] = mode;
	  pima_mode_bpm[i] = CPL_BINARY_0; // may be overdoing ....
      } else {
	  pima_mode[i] = NAN;
	  pima_mode_bpm[i] = CPL_BINARY_1;
	  cpl_error_reset();
      }
  }

  /* Return the mad scaled standard deviation */
  //double mad = 0;
  //cpl_image_get_mad(ima_mode, &mad);
  //*mode_error = mad * CPL_MATH_STD_MAD;

  /* Return the pure standard deviation as the mode on the various simulations
   * is not very smoothly distributed - the MAD based mode depends thus more on
   * the number of iterations */
  *mode_error = cpl_image_get_stdev(ima_mode);

  /* clean memory */
  cpl_image_delete(ima_mode);

  for(cpl_size i = 0 ; i < (cpl_size)omp_get_max_threads(); i++){
      hdrl_random_state_delete(pstate[i]);
  }
  cpl_free(pstate);

  return cpl_error_get_code();
}

