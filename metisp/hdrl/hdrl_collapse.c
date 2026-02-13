/*
 * This file is part of the HDRL
 * Copyright (C) 2013 European Southern Observatory
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

#include "hdrl_collapse.h"
#include "hdrl_parameter.h"
#include "hdrl_parameter_defs.h"
#include "hdrl_sigclip.h"
#include "hdrl_mode.h"
#include "hdrl_utils.h"
#include "hdrl_types.h"
#include <cpl.h>

#include <string.h>
#include <math.h>
/*-----------------------------------------------------------------------------
                                   Types
-----------------------------------------------------------------------------*/

/* function performing imglist -> image reduction */
typedef cpl_error_code (hdrl_collapse_imagelist_to_image_f)(
    const cpl_imagelist * data,
    const cpl_imagelist * errors,
    cpl_image ** out,
    cpl_image ** err,
    cpl_image ** contrib,
    void * parameters,
    void * extra_out);

struct hdrl_collapse_imagelist_to_image_s {
    /* function performing imglist -> image reduction */
    hdrl_collapse_imagelist_to_image_f * func;
    /* create extra out storage */
    void * (*create_eout)(const cpl_image *);
    /* move extra out storage */
    cpl_error_code (*move_eout)(void *, void *, const cpl_size);
    /* unwrap extra out storage */
    hdrl_free * unwrap_eout;
    /* delete extra out storage */
    void (*delete_eout)(void *);
    /* parameters the reduction function requires */
    hdrl_parameter * parameters;
};

/* function performing imglist -> image reduction */
typedef cpl_error_code (hdrl_collapse_imagelist_to_vector_f)(
    const cpl_imagelist * data,
    const cpl_imagelist * errors,
    cpl_vector ** out,
    cpl_vector ** err,
    cpl_array ** contrib,
    void * parameters,
    void * extra_out);

struct hdrl_collapse_imagelist_to_vector_s {
    /* function performing imglist -> vector reduction */
    hdrl_collapse_imagelist_to_vector_f * func;
    /* create extra out storage */
    void * (*create_eout)(cpl_size);
    /* move extra out storage */
    cpl_error_code (*move_eout)(void *, void *, const cpl_size);
    /* unwrap extra out storage */
    hdrl_free * unwrap_eout;
    /* delete extra out storage */
    void (*delete_eout)(void *);
    /* parameters the reduction function requires */
    hdrl_parameter * parameters;
};

static cpl_error_code
hdrl_collapse_mean(const cpl_imagelist * data,
                   const cpl_imagelist * errors,
                   cpl_image ** out, cpl_image ** err,
                   cpl_image ** contrib, void *, void *);
static cpl_error_code
hdrl_collapse_median(const cpl_imagelist * data,
                     const cpl_imagelist * errors,
                     cpl_image ** out, cpl_image ** err,
                     cpl_image ** contrib, void *, void *);
static cpl_error_code
hdrl_collapse_sigclip(const cpl_imagelist * data,
                      const cpl_imagelist * errors,
                      cpl_image ** out, cpl_image ** err,
                      cpl_image ** contrib,
					  void * parameters, void * extra_out);

static cpl_error_code
hdrl_collapse_minmax(const cpl_imagelist * data,
                      const cpl_imagelist * errors,
                      cpl_image ** out, cpl_image ** err,
                      cpl_image ** contrib,
					  void * parameters, void * extra_out);
static cpl_error_code
hdrl_collapse_mode(const cpl_imagelist * data,
                     const cpl_imagelist * errors,
                     cpl_image ** out, cpl_image ** err,
                     cpl_image ** contrib, void *, void *);

static cpl_error_code
hdrl_collapse_weighted_mean(const cpl_imagelist * data_,
                            const cpl_imagelist * errors_,
                            cpl_image ** out, cpl_image ** err,
                            cpl_image ** contrib, void *, void *);

/*-----------------------------------------------------------------------------
                                   Static
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_collapse Collapse Parameters

  This module provides collapse parameters to be used by hdrl_image and
  hdrl_imagelist objects.
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/** @cond PRIVATE */


/*-----------------------------------------------------------------------------
                            Collapse Parameters
 -----------------------------------------------------------------------------*/
static void hdrl_nop_free(void * HDRL_UNUSED(x))
{
    return;
}

static void * hdrl_mean_alloc(size_t n);
static void * hdrl_median_alloc(size_t n);
static void * hdrl_weighted_mean_alloc(size_t n);

/* Mean COLLAPSE */
typedef hdrl_parameter_empty hdrl_collapse_mean_parameter;
static hdrl_parameter_typeobj hdrl_collapse_mean_parameter_type = {
    HDRL_PARAMETER_COLLAPSE_MEAN,           /* type */
    (hdrl_alloc *)&hdrl_mean_alloc,         /* fp_alloc */
    (hdrl_free *)&hdrl_nop_free,            /* fp_free */
    NULL,                                   /* fp_destroy */
    sizeof(hdrl_collapse_mean_parameter),   /* obj_size */
};

HDRL_PARAMETER_SINGLETON(HDRL_COLLAPSE_MEAN,
						 hdrl_collapse_mean_parameter_type,
                         hdrl_mean_alloc);

/* Median COLLAPSE */
typedef hdrl_parameter_empty hdrl_collapse_median_parameter;
static hdrl_parameter_typeobj hdrl_collapse_median_parameter_type = {
    HDRL_PARAMETER_COLLAPSE_MEDIAN,         /* type */
    (hdrl_alloc *)&hdrl_median_alloc,       /* fp_alloc */
    (hdrl_free *)&hdrl_nop_free,            /* fp_free */
    NULL,                                   /* fp_destroy */
    sizeof(hdrl_collapse_median_parameter), /* obj_size */
};

HDRL_PARAMETER_SINGLETON(HDRL_COLLAPSE_MEDIAN,
                         hdrl_collapse_median_parameter_type,
                         hdrl_median_alloc);

/* Weighted Mean COLLAPSE */
typedef hdrl_parameter_empty hdrl_collapse_weighted_mean_parameter;
static hdrl_parameter_typeobj hdrl_collapse_weighted_mean_parameter_type = {
    HDRL_PARAMETER_COLLAPSE_WEIGHTED_MEAN,          /* type */
    (hdrl_alloc *)&hdrl_weighted_mean_alloc,        /* fp_alloc */
    (hdrl_free *)&hdrl_nop_free,                    /* fp_free */
    NULL,                                           /* fp_destroy */
    sizeof(hdrl_collapse_weighted_mean_parameter),  /* obj_size */
};

HDRL_PARAMETER_SINGLETON(HDRL_COLLAPSE_WEIGHTED_MEAN,
                         hdrl_collapse_weighted_mean_parameter_type,
                         hdrl_weighted_mean_alloc);

/* Sigma-Clipping COLLAPSE */
typedef struct {
    HDRL_PARAMETER_HEAD;
    double kappa_low;
    double kappa_high;
    int niter;
} hdrl_collapse_sigclip_parameter;
static hdrl_parameter_typeobj hdrl_collapse_sigclip_parameter_type = {
    HDRL_PARAMETER_COLLAPSE_SIGCLIP,            /* type */
    (hdrl_alloc *)&cpl_malloc,                  /* fp_alloc */
    (hdrl_free *)&cpl_free,                     /* fp_free */
    NULL,                                       /* fp_destroy */
    sizeof(hdrl_collapse_sigclip_parameter),    /* obj_size */
};


/* Minmax-Clipping COLLAPSE */
typedef struct {
    HDRL_PARAMETER_HEAD;
    double nlow;
    double nhigh;
} hdrl_collapse_minmax_parameter;
static hdrl_parameter_typeobj hdrl_collapse_minmax_parameter_type = {
    HDRL_PARAMETER_COLLAPSE_MINMAX,            /* type */
    (hdrl_alloc *)&cpl_malloc,                 /* fp_alloc */
    (hdrl_free *)&cpl_free,                    /* fp_free */
    NULL,                                      /* fp_destroy */
    sizeof(hdrl_collapse_minmax_parameter),    /* obj_size */
};

/* Mode COLLAPSE */
typedef struct {
    HDRL_PARAMETER_HEAD;
    double histo_min;
    double histo_max;
    double bin_size;
    hdrl_mode_type method;
    cpl_size error_niter;
} hdrl_collapse_mode_parameter;
static hdrl_parameter_typeobj hdrl_collapse_mode_parameter_type = {
    HDRL_PARAMETER_COLLAPSE_MODE,            /* type */
    (hdrl_alloc *)&cpl_malloc,                 /* fp_alloc */
    (hdrl_free *)&cpl_free,                    /* fp_free */
    NULL,                                      /* fp_destroy */
    sizeof(hdrl_collapse_mode_parameter),    /* obj_size */
};

/** @endcond */

/* ---------------------------------------------------------------------------*/
/**
 * @brief create a parameter object for mean
 * @return hdrl_parameter
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter * hdrl_collapse_mean_parameter_create(void)
{
    hdrl_parameter_empty * p = (hdrl_parameter_empty *)
       hdrl_parameter_new(&hdrl_collapse_mean_parameter_type);
    return (hdrl_parameter *)p;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief check if parameter is a mean parameter
 * @return boolean
 */
/* ---------------------------------------------------------------------------*/
cpl_boolean hdrl_collapse_parameter_is_mean(const hdrl_parameter * self)
{
    return hdrl_parameter_check_type(self, &hdrl_collapse_mean_parameter_type);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief create a parameter object for median
 * @return hdrl_parameter
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter * hdrl_collapse_median_parameter_create(void)
{
    hdrl_parameter_empty * p = (hdrl_parameter_empty *)
       hdrl_parameter_new(&hdrl_collapse_median_parameter_type);
    return (hdrl_parameter *)p;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief check if parameter is a median parameter
 * @return boolean
 */
/* ---------------------------------------------------------------------------*/
cpl_boolean hdrl_collapse_parameter_is_median(const hdrl_parameter * self)
{
    return hdrl_parameter_check_type(self, &hdrl_collapse_median_parameter_type);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief create a parameter object for weighted mean
 * @return hdrl_parameter
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter * hdrl_collapse_weighted_mean_parameter_create(void)
{
    hdrl_parameter_empty * p = (hdrl_parameter_empty *)
       hdrl_parameter_new(&hdrl_collapse_weighted_mean_parameter_type);
    return (hdrl_parameter *)p;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief check if parameter is a weighted mean parameter
 * @return boolean
 */
/* ---------------------------------------------------------------------------*/
cpl_boolean hdrl_collapse_parameter_is_weighted_mean(
        const hdrl_parameter * self)
{
    return hdrl_parameter_check_type(self, 
            &hdrl_collapse_weighted_mean_parameter_type);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief create a parameter object for sigclipped mean
 *
 * @param kappa_low   low kappa multiplier
 * @param kappa_high  high kappa multiplier
 * @param niter       maximum number of clipping iterations
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter *
hdrl_collapse_sigclip_parameter_create(double kappa_low, double kappa_high, int niter)
{
    hdrl_collapse_sigclip_parameter * p = (hdrl_collapse_sigclip_parameter *)
       hdrl_parameter_new(&hdrl_collapse_sigclip_parameter_type);
    p->kappa_low = kappa_low;
    p->kappa_high = kappa_high;
    p->niter = niter;

    if (hdrl_collapse_sigclip_parameter_verify((hdrl_parameter*)p) !=
        CPL_ERROR_NONE) {
        hdrl_parameter_delete((hdrl_parameter*)p);
        return NULL;
    }
    return (hdrl_parameter *)p;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief check if parameter is a sigclip mean parameter
 * @return boolean
 */
/* ---------------------------------------------------------------------------*/
cpl_boolean hdrl_collapse_parameter_is_sigclip(const hdrl_parameter * self)
{
    return hdrl_parameter_check_type(self, 
            &hdrl_collapse_sigclip_parameter_type);
}

/** @cond PRIVATE */

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Verify basic correctness of the parameters
  @param    param   Collapse siglip parameters
  @return   CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_collapse_sigclip_parameter_verify(
        const hdrl_parameter    *   param) 
{
    cpl_error_ensure(param != NULL, CPL_ERROR_NULL_INPUT,
            return CPL_ERROR_NULL_INPUT,
            "NULL Collapse Sigclip Parameters");

    cpl_error_ensure(hdrl_collapse_parameter_is_sigclip(param),
                     CPL_ERROR_INCOMPATIBLE_INPUT, return
                     CPL_ERROR_INCOMPATIBLE_INPUT,
                     "Not a Sigclip parameter");

    const hdrl_collapse_sigclip_parameter * param_loc =
            (const hdrl_collapse_sigclip_parameter *)param ;

    cpl_error_ensure(param_loc->niter > 0, CPL_ERROR_ILLEGAL_INPUT,
            return CPL_ERROR_ILLEGAL_INPUT,
            "sigma-clipping iter (%d) value must be > 0",
            param_loc->niter);

    return CPL_ERROR_NONE ;
}

/** @endcond */

/* ---------------------------------------------------------------------------*/
/**
 * @brief get high kappa
 * @param p  parameter
 * @return kappa_low if p is of sigclip type
 */
/* ---------------------------------------------------------------------------*/
double hdrl_collapse_sigclip_parameter_get_kappa_high(const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    cpl_ensure(hdrl_collapse_parameter_is_sigclip(p), 
            CPL_ERROR_INCOMPATIBLE_INPUT, -1);

    return ((const hdrl_collapse_sigclip_parameter *)p)->kappa_high;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get low kappa
 * @param p  parameter
 * @return kappa_high if p is of sigclip type
 */
/* ---------------------------------------------------------------------------*/
double hdrl_collapse_sigclip_parameter_get_kappa_low(const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    cpl_ensure(hdrl_collapse_parameter_is_sigclip(p), 
            CPL_ERROR_INCOMPATIBLE_INPUT, -1);

    return ((const hdrl_collapse_sigclip_parameter *)p)->kappa_low;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get maximum number of clipping iterations
 * @param p  parameter
 * @return n if p is of sigclip type
 */
/* ---------------------------------------------------------------------------*/
int hdrl_collapse_sigclip_parameter_get_niter(const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    cpl_ensure(hdrl_collapse_parameter_is_sigclip(p), 
            CPL_ERROR_INCOMPATIBLE_INPUT, -1);

    return ((const hdrl_collapse_sigclip_parameter *)p)->niter;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief create a parameter object for min-max rejected mean
 *
 * @param nlow  number of low pixels to be rejected
 * @param nhigh  number of high pixels to be rejected
 * @return minmax collapse parameter or NULL on error
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter *
hdrl_collapse_minmax_parameter_create(double nlow, double nhigh)
{
    hdrl_collapse_minmax_parameter * p = (hdrl_collapse_minmax_parameter *)
       hdrl_parameter_new(&hdrl_collapse_minmax_parameter_type);
    p->nlow = nlow;
    p->nhigh = nhigh;

    if (hdrl_collapse_minmax_parameter_verify((hdrl_parameter*)p) !=
        CPL_ERROR_NONE) {
        hdrl_parameter_delete((hdrl_parameter*)p);
        return NULL;
    }
    return (hdrl_parameter *)p;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief check if parameter is a minmax mean parameter
 * @return boolean
 */
/* ---------------------------------------------------------------------------*/
cpl_boolean hdrl_collapse_parameter_is_minmax(const hdrl_parameter * self)
{
    return hdrl_parameter_check_type(self,
            &hdrl_collapse_minmax_parameter_type);
}

/** @cond PRIVATE */

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Verify basic correctness of the parameters
  @param    param   Collapse minmax parameters
  @return   CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_collapse_minmax_parameter_verify(
        const hdrl_parameter    *   param)
{

    cpl_error_ensure(param != NULL, CPL_ERROR_NULL_INPUT,
                     return CPL_ERROR_NULL_INPUT,
                     "NULL Collapse Minmax Parameters");

    cpl_error_ensure(hdrl_collapse_parameter_is_minmax(param),
                     CPL_ERROR_INCOMPATIBLE_INPUT, return
                     CPL_ERROR_INCOMPATIBLE_INPUT,
                     "Not a minmax parameter");

    const hdrl_collapse_minmax_parameter * param_loc =
            (const hdrl_collapse_minmax_parameter *)param ;

    cpl_error_ensure(param_loc->nlow >= 0, CPL_ERROR_ILLEGAL_INPUT,
            return CPL_ERROR_ILLEGAL_INPUT,
            "nlow value (%g) must be >= 0",
            param_loc->nlow);

    cpl_error_ensure(param_loc->nhigh >= 0, CPL_ERROR_ILLEGAL_INPUT,
            return CPL_ERROR_ILLEGAL_INPUT,
            "nhigh value (%g) must be >= 0",
            param_loc->nlow);

    return CPL_ERROR_NONE ;
}

/** @endcond */

/* ---------------------------------------------------------------------------*/
/**
 * @brief get high value
 * @param p  parameter
 * @return nlow if p is of minmax type
 */
/* ---------------------------------------------------------------------------*/
double hdrl_collapse_minmax_parameter_get_nhigh(const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    cpl_ensure(hdrl_collapse_parameter_is_minmax(p),
            CPL_ERROR_INCOMPATIBLE_INPUT, -1);

    return ((const hdrl_collapse_minmax_parameter *)p)->nhigh;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get low value
 * @param p  parameter
 * @return nhigh if p is of minmax type
 */
/* ---------------------------------------------------------------------------*/
double hdrl_collapse_minmax_parameter_get_nlow(const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    cpl_ensure(hdrl_collapse_parameter_is_minmax(p),
            CPL_ERROR_INCOMPATIBLE_INPUT, -1);

    return ((const hdrl_collapse_minmax_parameter *)p)->nlow;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief create a parameter object for the mode
 *
 * @param histo_min      minimum value of low pixels to use
 * @param histo_max      maximum value of high pixels to be use
 * @param bin_size       size of the histogram bin
 * @param mode_method    method to use for the mode computation
 * @param error_niter    number of iterations to compute the error of the mode
 * @return mode collapse parameter or NULL on error
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter *
hdrl_collapse_mode_parameter_create(double histo_min,
				    double histo_max,
				    double bin_size,
				    hdrl_mode_type mode_method,
				    cpl_size error_niter)
{
    hdrl_collapse_mode_parameter * p = (hdrl_collapse_mode_parameter *)
       hdrl_parameter_new(&hdrl_collapse_mode_parameter_type);
    p->histo_min = histo_min;
    p->histo_max = histo_max;
    p->bin_size = bin_size;
    p->method = mode_method;
    p->error_niter = error_niter;
    if (hdrl_collapse_mode_parameter_verify((hdrl_parameter*)p) !=
        CPL_ERROR_NONE) {
        hdrl_parameter_delete((hdrl_parameter*)p);
        return NULL;
    }
    return (hdrl_parameter *)p;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief check if parameter is a mode parameter
 * @return boolean
 */
/* ---------------------------------------------------------------------------*/
cpl_boolean hdrl_collapse_parameter_is_mode(const hdrl_parameter * self)
{
    return hdrl_parameter_check_type(self,
            &hdrl_collapse_mode_parameter_type);
}

/** @cond PRIVATE */

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    Verify basic correctness of the parameters
  @param    param   Collapse mode parameters
  @return   CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_collapse_mode_parameter_verify(
        const hdrl_parameter    *   param)
{

    cpl_error_ensure(param != NULL, CPL_ERROR_NULL_INPUT,
                     return CPL_ERROR_NULL_INPUT,
                     "NULL Collapse Mode Parameters");

    cpl_error_ensure(hdrl_collapse_parameter_is_mode(param),
                     CPL_ERROR_INCOMPATIBLE_INPUT, return
                     CPL_ERROR_INCOMPATIBLE_INPUT,
                     "Not a mode parameter");

    const hdrl_collapse_mode_parameter * param_loc =
            (const hdrl_collapse_mode_parameter *)param ;


    cpl_error_ensure(param_loc->bin_size >= 0, CPL_ERROR_ILLEGAL_INPUT,
            return CPL_ERROR_ILLEGAL_INPUT,
            "bin_size value (%g) must be >= 0",
            param_loc->bin_size);

    cpl_error_ensure(param_loc->error_niter >= 0, CPL_ERROR_ILLEGAL_INPUT,
            return CPL_ERROR_ILLEGAL_INPUT,
            "error_niter value (%lld) must be >= 0",
            param_loc->error_niter);

    cpl_error_ensure(param_loc->method == HDRL_MODE_MEDIAN ||
		     param_loc->method == HDRL_MODE_FIT ||
		     param_loc->method == HDRL_MODE_WEIGHTED ,
		     CPL_ERROR_ILLEGAL_INPUT,
            return CPL_ERROR_ILLEGAL_INPUT,
            "Please check the computation method of the mode. It has to be "
            "%d, or %d, or %d", HDRL_MODE_MEDIAN, HDRL_MODE_WEIGHTED,
	    HDRL_MODE_FIT);

    return CPL_ERROR_NONE ;
}

/** @endcond */

/* ---------------------------------------------------------------------------*/
/**
 * @brief get min value
 * @param p  parameter
 * @return histo_min if p is of mode type
 */
/* ---------------------------------------------------------------------------*/
double hdrl_collapse_mode_parameter_get_histo_min(const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    cpl_ensure(hdrl_collapse_parameter_is_mode(p),
            CPL_ERROR_INCOMPATIBLE_INPUT, -1);

    return ((const hdrl_collapse_mode_parameter *)p)->histo_min;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get high value
 * @param p  parameter
 * @return histo_max if p is of mode type
 */
/* ---------------------------------------------------------------------------*/
double hdrl_collapse_mode_parameter_get_histo_max(const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    cpl_ensure(hdrl_collapse_parameter_is_mode(p),
            CPL_ERROR_INCOMPATIBLE_INPUT, -1);

    return ((const hdrl_collapse_mode_parameter *)p)->histo_max;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get size of the histogram bins
 * @param p  parameter
 * @return bin_size if p is of mode type
 */
/* ---------------------------------------------------------------------------*/
double hdrl_collapse_mode_parameter_get_bin_size(const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    cpl_ensure(hdrl_collapse_parameter_is_mode(p),
            CPL_ERROR_INCOMPATIBLE_INPUT, -1);

    return ((const hdrl_collapse_mode_parameter *)p)->bin_size;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get the mode determination method
 * @param p  parameter
 * @return mode method if p is of mode type
 */
/* ---------------------------------------------------------------------------*/
hdrl_mode_type hdrl_collapse_mode_parameter_get_method(const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    cpl_ensure(hdrl_collapse_parameter_is_mode(p),
            CPL_ERROR_INCOMPATIBLE_INPUT, -1);

    return ((const hdrl_collapse_mode_parameter *)p)->method;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief get the error type of the mode
 * @param p  parameter
 * @return error type if p is of mode type
 */
/* ---------------------------------------------------------------------------*/
cpl_size hdrl_collapse_mode_parameter_get_error_niter(const hdrl_parameter * p)
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1);
    cpl_ensure(hdrl_collapse_parameter_is_mode(p),
            CPL_ERROR_INCOMPATIBLE_INPUT, -1);

    return ((const hdrl_collapse_mode_parameter *)p)->error_niter;
}


/* ---------------------------------------------------------------------------*/
/**
  @brief Create parameters for the collapse
  @param base_context    base context of parameter (e.g. recipe name)
  @param prefix          prefix of parameter, may be empty string
  @param method_def      default collapse method
  @param sigclip_def     default sigclip parameters
  @param minmax_def      default minmax parameters
  @param mode_def        default collapsing mode computation parameters
  @return The created parameter list
  Creates a parameterlist containing
      base_context.prefix.method
      base_context.prefix.sigclip.*
  The CLI aliases omit the base_context.
 */
/* ---------------------------------------------------------------------------*/
cpl_parameterlist * hdrl_collapse_parameter_create_parlist(
        const char     *base_context,
        const char     *prefix,
        const char     *method_def,
        hdrl_parameter *sigclip_def,
        hdrl_parameter *minmax_def,
	hdrl_parameter *mode_def)
{
    cpl_ensure(base_context && prefix, CPL_ERROR_NULL_INPUT, NULL);

    cpl_ensure(hdrl_collapse_parameter_is_sigclip(sigclip_def)
	       && hdrl_collapse_parameter_is_minmax( minmax_def )
	       && hdrl_collapse_parameter_is_mode( mode_def ),
       		CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    char              *name;
    cpl_parameterlist *parlist = cpl_parameterlist_new();
    cpl_parameter     *p;
    char              *context = hdrl_join_string(".", 2, base_context, prefix);

    /* --prefix.method */
    name = hdrl_join_string(".", 2, context, "method");
    p = cpl_parameter_new_enum(name, CPL_TYPE_STRING, 
            "Method used for collapsing the data", context,
            method_def, 6, "MEAN", "WEIGHTED_MEAN", "MEDIAN", "SIGCLIP",
            "MINMAX", "MODE");
    cpl_free(name) ;
    name = hdrl_join_string(".", 2, prefix, "method");
    cpl_parameter_set_alias(p, CPL_PARAMETER_MODE_CLI, name);
    cpl_parameter_disable(p, CPL_PARAMETER_MODE_ENV);
    cpl_free(name) ;
    cpl_parameterlist_append(parlist, p);

    /* --prefix.sigclip.xxx */
    name = hdrl_join_string(".", 2, prefix, "sigclip");
    cpl_parameterlist * psigclip = hdrl_sigclip_parameter_create_parlist(
            base_context, name, sigclip_def);
    cpl_free(name) ;

    for (cpl_parameter * par = cpl_parameterlist_get_first(psigclip);
         par != NULL;
         par = cpl_parameterlist_get_next(psigclip)) {
        cpl_parameterlist_append(parlist, cpl_parameter_duplicate(par));
    }
    cpl_parameterlist_delete(psigclip);

    /* --prefix.minmax.xxx */
    name = hdrl_join_string(".", 2, prefix, "minmax");
    cpl_parameterlist * pminmax = hdrl_minmax_parameter_create_parlist(
            base_context, name, minmax_def);
    cpl_free(name) ;


    for (cpl_parameter * par = cpl_parameterlist_get_first(pminmax);
         par != NULL;
         par = cpl_parameterlist_get_next(pminmax)) {
        cpl_parameterlist_append(parlist, cpl_parameter_duplicate(par));
    }
    cpl_parameterlist_delete(pminmax);


    /* --prefix.mode.xxx */
    name = hdrl_join_string(".", 2, prefix, "mode");
    cpl_parameterlist * pmode = hdrl_mode_parameter_create_parlist(
            base_context, name, mode_def);
    cpl_free(name) ;


    for (cpl_parameter * par = cpl_parameterlist_get_first(pmode);
         par != NULL;
         par = cpl_parameterlist_get_next(pmode)) {
        cpl_parameterlist_append(parlist, cpl_parameter_duplicate(par));
    }
    cpl_parameterlist_delete(pmode);

    cpl_free(context);

    if (cpl_error_get_code()) {
        cpl_parameterlist_delete(parlist);
        return NULL;
    }

    return parlist;
}

/* ---------------------------------------------------------------------------*/
/**
  @brief parse parameterlist for imagelist reduction method
  @param parlist    parameter list to parse
  @param prefix     prefix of parameter name
  @return hdrl_parameter 
  Reads a Parameterlist in order to create collapse parameters.
  Expects a parameterlist containing
      prefix.method
      prefix.sigclip.*
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter * hdrl_collapse_parameter_parse_parlist(
        const cpl_parameterlist     *   parlist,
        const char                  *   prefix)
{
    cpl_ensure(prefix && parlist, CPL_ERROR_NULL_INPUT, NULL);
    hdrl_parameter * p = NULL;

    /* Get the Method parameter */
    char * name = hdrl_join_string(".", 2, prefix, "method");
    const cpl_parameter * par = cpl_parameterlist_find_const(parlist, name);
    const char          * value = cpl_parameter_get_string(par);
    if (value == NULL) {
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                              "Parameter %s not found", name);
        cpl_free(name);
        return NULL;
    }
    
    /* Switch on the methods */
    if(!strcmp(value, "MEDIAN")) {
        p = hdrl_collapse_median_parameter_create();
    } else if (!strcmp(value, "WEIGHTED_MEAN")) {
        p = hdrl_collapse_weighted_mean_parameter_create();
    } else if (!strcmp(value, "MEAN")) {
        p = hdrl_collapse_mean_parameter_create();
    } else if (!strcmp(value, "SIGCLIP")) {
        double kappa_low, kappa_high;
        int niter;
        hdrl_sigclip_parameter_parse_parlist(parlist, prefix, &kappa_low, 
                &kappa_high, &niter);
        p = hdrl_collapse_sigclip_parameter_create(kappa_low,kappa_high,niter);
    } else if (!strcmp(value, "MINMAX")) {
        double nlow, nhigh;
        hdrl_minmax_parameter_parse_parlist(parlist, prefix, &nlow,
                &nhigh);
        p = hdrl_collapse_minmax_parameter_create(nlow,nhigh);

    } else if (!strcmp(value, "MODE")) {
      double histo_min, histo_max, bin_size;
	cpl_size error_niter;
	hdrl_mode_type method;
        hdrl_mode_parameter_parse_parlist(parlist, prefix, &histo_min,
					  &histo_max,
					  &bin_size,
					  &method,
					  &error_niter);
        p = hdrl_collapse_mode_parameter_create(histo_min, histo_max, bin_size,
						method, error_niter);
    } else {
        cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                "%s not a valid method for %s", value, name);
        cpl_free(name);
        return NULL;
    }
    cpl_free(name);
    return p;
}

/** @cond PRIVATE */

/* ---------------------------------------------------------------------------*/
/**
 * @brief create a new imagelist wrapping errors with same bpm as data
 *
 * @param data   data list
 * @param errors error list
 * @returns cpl_imagelist which needs deleted with unwrap_synced_errlist
 *
 * Creates a new imagelist containing the wrapped data from the error list but
 * the bad pixel map from the data list.
 * This avoids issues with desynchronized bad pixel maps and avoids modifying
 * the inputs for collapse operations.
 */
/* ---------------------------------------------------------------------------*/
static cpl_imagelist *
wrap_synced_errlist(const cpl_imagelist * data, const cpl_imagelist * errors)
{
    cpl_imagelist * nerrors = cpl_imagelist_new();
    for (size_t i = 0; i < (size_t)cpl_imagelist_get_size(errors); i++) {
        const cpl_image * img = cpl_imagelist_get_const(data, i);
        const cpl_image * err = cpl_imagelist_get_const(errors, i);

        CPL_DIAG_PRAGMA_PUSH_IGN(-Wcast-qual);

        cpl_image * nerr = cpl_image_wrap(cpl_image_get_size_x(err),
                                          cpl_image_get_size_y(err),
                                          cpl_image_get_type(err),
                                          (void*)cpl_image_get_data_const(err));
        cpl_mask_delete(hcpl_image_set_bpm(nerr,
                                  (cpl_mask*)cpl_image_get_bpm_const(img)));

        CPL_DIAG_PRAGMA_POP;

        cpl_imagelist_set(nerrors, nerr, i);
    }
    return nerrors;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief unwrap imagelist created with wrap_synced_errlist
 *
 * @param nerrors  imagelist from wrap_synced_errlist
 */
/* ---------------------------------------------------------------------------*/
static void
unwrap_synced_errlist(cpl_imagelist * nerrors)
{
    for (size_t i = 0; i < (size_t)cpl_imagelist_get_size(nerrors); i++) {

        CPL_DIAG_PRAGMA_PUSH_IGN(-Wcast-qual);

        cpl_image * err = (cpl_image*)cpl_imagelist_get_const(nerrors, i);

        CPL_DIAG_PRAGMA_POP;

        cpl_image_unset_bpm(err);
        cpl_image_unwrap(err);
    }
    cpl_imagelist_unwrap(nerrors);
}

/* ---------------------------------------------------------------------------*/
/**
  @internal
  @brief calculate sum of squares of an imagelist
  @param data     imagelist
  @param contrib  if not NULL contribution map of reduction is stored
  @return image containing the sum of squares along the imagelist axis
  equivalent to:
    cpl_imagelist_power(data, 2.)
    sqlist = cpl_imagelist_collapse_create(data)
    cpl_image_multiply(sqlist, contrib);
 */
/* ---------------------------------------------------------------------------*/
static cpl_image * imagelist_sqsum(
        const cpl_imagelist     *   data, 
        cpl_image               **  pcontrib)
{
    cpl_image * contrib = cpl_image_new_from_accepted(data);
    cpl_image * res = NULL;

    for (cpl_size i = 0; i < cpl_imagelist_get_size(data); i++) {
        const cpl_image * img = cpl_imagelist_get_const(data, i);
        cpl_image * sqerr = cpl_image_multiply_create(img, img);
        if (cpl_image_get_bpm_const(sqerr)) {
            cpl_image_fill_rejected(sqerr, 0.0);
            cpl_image_accept_all(sqerr);
        }

        if (i == 0) {
            res = sqerr;
        } else {
            cpl_image_add(res, sqerr);
            cpl_image_delete(sqerr);
        }
    }
    cpl_mask * allbad = cpl_mask_threshold_image_create(contrib, 0, 0);
    cpl_image_reject_from_mask(res, allbad);
    cpl_mask_delete(allbad);

    if (pcontrib)
        *pcontrib = contrib;
    else
        cpl_image_delete(contrib);

    return res;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief implements mean combination on input image list
 *
 * @param data    input data images
 * @param errors  input errors images
 * @param out     output combined images
 * @param err     output combined errors
 * @param contrib output contribution mask
 * @param parameters parameters to control, not used
 * @param extra_out  optional extra output, not used
 *
 *
 * @return cpl_error_code
 *
 * @doc
 * Mean and associated error are computed with standard formulae
 *
 * \f$
 *   x_{mean}=\frac{(\sum_{i}^{n} x_{i})} { n }
 * \f$
 *
 * \f$
 *   \sigma_{x}=\sqrt{ \frac{ \sum_{i}^{n} x_{i}^{2} }{ n } }
 * \f$
 *
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code
hdrl_collapse_mean(const cpl_imagelist * data,
                             const cpl_imagelist * errors,
                             cpl_image ** out, cpl_image ** err,
                             cpl_image ** contrib,
                             void * HDRL_UNUSED(parameters),
                             void * HDRL_UNUSED(extra_out))
{
    /* (\Sum_i^n x_i) / n */
    /* \sqrt(\Sum_i^n x_i^2) / n */
    cpl_errorstate prestate = cpl_errorstate_get();
    *out = cpl_imagelist_collapse_create(data);
    /* ignore division by 0 on all pixels zero error */
    if (*out == NULL) {
        cpl_errorstate_set(prestate);
        *out = cpl_image_duplicate(cpl_imagelist_get_const(data, 0));
        cpl_image_accept_all(*out);
        cpl_mask_not(cpl_image_get_bpm(*out));
        *err = cpl_image_duplicate(cpl_imagelist_get_const(errors, 0));
        cpl_image_accept_all(*err);
        cpl_mask_not(cpl_image_get_bpm(*err));
        *contrib = cpl_image_new(cpl_image_get_size_x(*err),
                                 cpl_image_get_size_y(*err), CPL_TYPE_INT);
        cpl_image_fill_rejected(*out, NAN);
        cpl_image_fill_rejected(*err, NAN);
    }
    else {
        *err = imagelist_sqsum(errors, contrib);
        cpl_image_power(*err, 0.5);
        cpl_image_divide(*err, *contrib);
        cpl_image_fill_rejected(*out, NAN);
        cpl_image_fill_rejected(*err, NAN);
    }

    return cpl_error_get_code();
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief implements weighted mean combination on input image list
 *
 * @param data    input data images
 * @param errors_ input errors images
 * @param out     output combined images
 * @param err     output combined errors
 * @param contrib output contribution mask
 * @param parameters parameters to control, not used
 * @param extra_out  optional extra output, not used
 *
 * @return cpl_error_code
 *
 * @doc
 * weighted mean and associated error are computed with standard formulae
 *
 * \f$
 *   x_{mean}=\frac{(\sum_{i}^{n} w_{i} \cdot x_{i})} { \sum_{i}^{n} w_{i} }
 * \f$
 *
 * \f$
 *   \sigma_{x}=\frac{ 1 } { \sqrt{  \sum_{i}^{n} w_{i}^{2} } }
 * \f$
 *
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code
hdrl_collapse_weighted_mean(const cpl_imagelist * data_,
                                      const cpl_imagelist * errors_,
                                      cpl_image ** out, cpl_image ** err,
                                      cpl_image ** contrib,
                                      void * HDRL_UNUSED(parameters),
                                      void * HDRL_UNUSED(extra_out))
{
    /* (\Sum_i^n w_i * x_i) / (\Sum_i^n w_i) */
    /* 1 / \sqrt(\Sum_i^n w_i^2) */
    cpl_errorstate prestate = cpl_errorstate_get();
    cpl_imagelist * data = cpl_imagelist_duplicate(data_);
    cpl_imagelist * errors = cpl_imagelist_new();
    cpl_image * tmperr;
    cpl_imagelist_cast(errors, errors_,
                       cpl_image_get_type(cpl_imagelist_get(data, 0)));
    cpl_imagelist_power(errors, -2);
    cpl_imagelist_multiply(data, errors);
    *contrib = cpl_image_new_from_accepted(data);
    *out = cpl_imagelist_collapse_create(data);
    if (*out == NULL) {
        cpl_errorstate_set(prestate);
        *out = cpl_image_duplicate(cpl_imagelist_get_const(data, 0));
        cpl_image_accept_all(*out);
        cpl_mask_not(cpl_image_get_bpm(*out));
        *err = cpl_image_duplicate(cpl_imagelist_get_const(errors, 0));
        cpl_image_accept_all(*err);
        cpl_mask_not(cpl_image_get_bpm(*err));
        cpl_image_fill_rejected(*out, NAN);
        cpl_image_fill_rejected(*err, NAN);
        cpl_imagelist_delete(errors);
        cpl_imagelist_delete(data);
        return cpl_error_get_code();
    }
    cpl_imagelist_delete(data);
    tmperr = cpl_imagelist_collapse_create(errors);
    cpl_imagelist_delete(errors);
    cpl_image_multiply(*out, *contrib);
    cpl_image_multiply(tmperr, *contrib);
    cpl_image_divide(*out, tmperr);
    cpl_image_power(tmperr, -0.5);
    if (cpl_image_get_type(cpl_imagelist_get_const(errors_, 0)) ==
        cpl_image_get_type(cpl_imagelist_get_const(data_, 0))) {
        *err = tmperr;
    }
    else {
        *err = cpl_image_cast(tmperr,
                  cpl_image_get_type(cpl_imagelist_get_const(errors_, 0)));
        cpl_image_delete(tmperr);
    }

    cpl_image_fill_rejected(*out, NAN);
    cpl_image_fill_rejected(*err, NAN);

    return cpl_error_get_code();
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief implements median combination on input image list
 *
 * @param data    input data images
 * @param errors  input errors images
 * @param out     output combined images
 * @param err     output combined errors
 * @param contrib output contribution mask
 * @param parameters parameters to control, not used
 * @param extra_out  optional extra output, not used
 *
 * @return cpl_error_code
 *
 * @doc
 * Median and associated error are computed similarly as for mean but
 * scaling by \f$ \sqrt{ \frac{ \pi } { 2 } } \f$
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code
hdrl_collapse_median(const cpl_imagelist * data,
                               const cpl_imagelist * errors,
                               cpl_image ** out, cpl_image ** err,
                               cpl_image ** contrib,
                               void * HDRL_UNUSED(parameters),
                               void * HDRL_UNUSED(extra_out))
{
    cpl_errorstate prestate = cpl_errorstate_get();
    /* same as mean with scaling by \sqrt(\pi / 2) */
    *out = cpl_imagelist_collapse_median_create(data);
    *err = imagelist_sqsum(errors, contrib);
    cpl_image_power(*err, 0.5);
    cpl_image_divide(*err, *contrib);
    if (cpl_error_get_code() == CPL_ERROR_DIVISION_BY_ZERO) {
        cpl_errorstate_set(prestate);
        cpl_image_accept_all(*out);
        cpl_mask_not(cpl_image_get_bpm(*out));
        cpl_image_accept_all(*err);
        cpl_mask_not(cpl_image_get_bpm(*err));
        cpl_image_fill_rejected(*out, NAN);
        cpl_image_fill_rejected(*err, NAN);
        return cpl_error_get_code();
    }
    /* scale error so it estimates stdev of normal distribution */
    cpl_image_multiply_scalar(*err, sqrt(CPL_MATH_PI_2));
    /* revert scaling for contrib <= 2 as median == mean in this case */
    cpl_image * tmp = cpl_image_cast(*contrib, CPL_TYPE_DOUBLE);
    cpl_image_threshold(tmp, 2.1, 2.1,  1. / sqrt(CPL_MATH_PI_2), 1.);
    cpl_image_multiply(*err, tmp);
    cpl_image_delete(tmp);
    cpl_image_fill_rejected(*out, NAN);
    cpl_image_fill_rejected(*err, NAN);

    return cpl_error_get_code();
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief implements sigma-clipped combination on input image list
 *
 * @param data    input data images
 * @param errors  input errors images
 * @param out     output combined images
 * @param err     output combined errors
 * @param contrib output contribution mask
 * @param parameters parameters to control, not used
 * @param extra_out  optional extra output, not used
 *
 * @return cpl_error_code
 *
 * @doc
 * sigma-clipped mean and associated error, computed similarly as for mean but
 * without taking the clipped values into account.
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code
hdrl_collapse_sigclip(const cpl_imagelist * data,
                      const cpl_imagelist * errors,
                      cpl_image ** out, cpl_image ** err,
                      cpl_image ** contrib,
					  void * parameters, void * extra_out)
{
    /* same as mean but working on the not-clipped values only */
    hdrl_collapse_sigclip_parameter * par = parameters;
    hdrl_sigclip_image_output * eout =
        (hdrl_sigclip_image_output *)extra_out;
    cpl_ensure_code(par, CPL_ERROR_NULL_INPUT);
    const cpl_image * img = cpl_imagelist_get_const(data, 0);
    size_t nx = cpl_image_get_size_x(img);
    size_t ny = cpl_image_get_size_y(img);
    *out = cpl_image_new(nx, ny, HDRL_TYPE_DATA);
    *err = cpl_image_new(nx, ny, HDRL_TYPE_ERROR);
    *contrib = cpl_image_new(nx, ny, CPL_TYPE_INT);
    hdrl_vector_cache * cache =
        hdrl_vector_cache_new(cpl_imagelist_get_size(data), nx * 2);

    /* sigmaclip along imagelist axis */
    for (size_t y = 1; y < ny + 1; y++) {
        cpl_vector * vdv[nx];
        cpl_vector * vev[nx];
        hdrl_imagelist_to_vector_row(data, y, vdv, cache);
        hdrl_imagelist_to_vector_row(errors, y, vev, cache);
        for (size_t x = 1; x < nx + 1; x++) {
            cpl_vector * vd = vdv[x - 1];
            cpl_vector * ve = vev[x - 1];
            if (vd && ve) {
                double m, e, rej_low, rej_high;
                cpl_size naccepted;
                hdrl_kappa_sigma_clip(vd, ve,
                                     par->kappa_low, par->kappa_high,
                                     par->niter, CPL_TRUE,
                                     &m, &e, &naccepted,
                                     &rej_low, &rej_high);
                cpl_image_set(*out, x, y, m);
                cpl_image_set(*err, x, y, e);
                cpl_image_set(*contrib, x, y, naccepted);
                if (eout) {
                    cpl_image_set(eout->reject_low, x, y, rej_low);
                    cpl_image_set(eout->reject_high, x, y, rej_high);
                }
            }
            else {
                cpl_image_set(*out, x, y, NAN);
                cpl_image_set(*err, x, y, NAN);
                cpl_image_reject(*out, x, y);
                cpl_image_reject(*err, x, y);
                cpl_image_set(*contrib, x, y, 0);
                if (eout) {
                    cpl_image_set(eout->reject_low, x, y, 0.);
                    cpl_image_set(eout->reject_high, x, y, 0.);
                }
            }
            hdrl_cplvector_delete_to_cache(cache, vd);
            hdrl_cplvector_delete_to_cache(cache, ve);
        }
    }
    hdrl_vector_cache_delete(cache);

    return cpl_error_get_code();
}




/* ---------------------------------------------------------------------------*/
/**
 * @brief implements minmax-clipped combination on input image list
 *
 * @param data    input data images
 * @param errors  input errors images
 * @param out     output combined images
 * @param err     output combined errors
 * @param contrib output contribution mask
 * @param parameters parameters to control, not used
 * @param extra_out  optional extra output, not used
 *
 * @return cpl_error_code
 *
 * @doc minmax-clipped mean and associated error, computed similarly
 * as for mean but without taking the clipped values into account
 * 
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code
hdrl_collapse_minmax(const cpl_imagelist * data,
                     const cpl_imagelist * errors,
                     cpl_image ** out, cpl_image ** err,
                     cpl_image ** contrib,
					 void * parameters, void * extra_out)
{
    /* same as mean but working on the not-clipped values only */
    hdrl_collapse_minmax_parameter * par = parameters;
    cpl_ensure_code(par, CPL_ERROR_NULL_INPUT);
    hdrl_minmax_image_output * eout =
        (hdrl_minmax_image_output *)extra_out;
    const cpl_image * img = cpl_imagelist_get_const(data, 0);
    size_t nx = cpl_image_get_size_x(img);
    size_t ny = cpl_image_get_size_y(img);
    *out = cpl_image_new(nx, ny, HDRL_TYPE_DATA);
    *err = cpl_image_new(nx, ny, HDRL_TYPE_ERROR);
    *contrib = cpl_image_new(nx, ny, CPL_TYPE_INT);
    hdrl_vector_cache * cache =
        hdrl_vector_cache_new(cpl_imagelist_get_size(data), nx * 2);

    /* minmaxclip along imagelist axis */
    for (size_t y = 1; y < ny + 1; y++) {
        cpl_vector * vdv[nx];
        cpl_vector * vev[nx];
        hdrl_imagelist_to_vector_row(data, y, vdv, cache);
        hdrl_imagelist_to_vector_row(errors, y, vev, cache);
        for (size_t x = 1; x < nx + 1; x++) {
            cpl_vector * vd = vdv[x - 1];
            cpl_vector * ve = vev[x - 1];
            if (vd && ve) {
                double m, e, rej_low, rej_high;
                cpl_size naccepted;
                hdrl_minmax_clip(vd, ve,
                                 par->nlow, par->nhigh, CPL_TRUE,
                                 &m, &e, &naccepted,
                                 &rej_low, &rej_high);
                cpl_image_set(*out, x, y, m);
                cpl_image_set(*err, x, y, e);
                cpl_image_set(*contrib, x, y, naccepted);
                if (eout) {
                    cpl_image_set(eout->reject_low, x, y, rej_low);
                    cpl_image_set(eout->reject_high, x, y, rej_high);
                }
            }
            else {
                cpl_image_set(*out, x, y, NAN);
                cpl_image_set(*err, x, y, NAN);
                cpl_image_reject(*out, x, y);
                cpl_image_reject(*err, x, y);
                cpl_image_set(*contrib, x, y, 0);
                if (eout) {
                    cpl_image_set(eout->reject_low, x, y, 0.);
                    cpl_image_set(eout->reject_high, x, y, 0.);
                }
            }
            hdrl_cplvector_delete_to_cache(cache, vd);
            hdrl_cplvector_delete_to_cache(cache, ve);
        }
    }
    hdrl_vector_cache_delete(cache);

    return cpl_error_get_code();
}



/* ---------------------------------------------------------------------------*/
/**
 * @brief implements mode combination on input image list
 *
 * @param data    input data images
 * @param errors  input errors images
 * @param out     output combined images
 * @param err     output combined errors
 * @param contrib output contribution mask
 * @param parameters parameters to control, not used
 * @param extra_out  optional extra output, not used
 *
 * @return cpl_error_code
 *
 * @doc compute mode and associated error
 *
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code
hdrl_collapse_mode(const cpl_imagelist * data,
		   const cpl_imagelist * errors,
		   cpl_image ** out,
		   cpl_image ** err,
		   cpl_image ** contrib,
		   void * parameters,
		   void * HDRL_UNUSED(extra_out))
{

  hdrl_collapse_mode_parameter * par = parameters;
  cpl_ensure_code(par, CPL_ERROR_NULL_INPUT);
  const cpl_image * img = cpl_imagelist_get_const(data, 0);
  size_t nx = cpl_image_get_size_x(img);
  size_t ny = cpl_image_get_size_y(img);
  *out = cpl_image_new(nx, ny, HDRL_TYPE_DATA);
  *err = cpl_image_new(nx, ny, HDRL_TYPE_ERROR);
  *contrib = cpl_image_new(nx, ny, CPL_TYPE_INT);
  hdrl_vector_cache * cache =
      hdrl_vector_cache_new(cpl_imagelist_get_size(data), nx * 2);

  /* mode along imagelist axis */
  for (size_t y = 1; y < ny + 1; y++) {
      cpl_vector * vdv[nx];
      cpl_vector * vev[nx];
      hdrl_imagelist_to_vector_row(data, y, vdv, cache);
      hdrl_imagelist_to_vector_row(errors, y, vev, cache);
      for (size_t x = 1; x < nx + 1; x++) {
	  cpl_vector * vd = vdv[x - 1];
	  cpl_vector * ve = vev[x - 1];
	  cpl_errorstate prestate = cpl_errorstate_get();
	  if (vd && ve) {
	      double m, e;
	      cpl_size naccepted;
	      cpl_errorstate prestate1 = cpl_errorstate_get();
	      if (hdrl_mode_clip(vd,
				 par->histo_min,
				 par->histo_max,
				 par->bin_size,
				 par->method,
				 par->error_niter,
				 &m, &e, &naccepted) == CPL_ERROR_NONE) {
		  cpl_image_set(*out, x, y, m);
		  cpl_image_set(*err, x, y, e);
		  cpl_image_set(*contrib, x, y, naccepted);
	      } else {
		  cpl_image_set(*out, x, y, NAN);
		  cpl_image_set(*err, x, y, NAN);
		  cpl_image_reject(*out, x, y);
		  cpl_image_reject(*err, x, y);
		  cpl_image_set(*contrib, x, y, 0);
		  cpl_errorstate_set(prestate1);
	      }
	  }
	  else {
	      cpl_image_set(*out, x, y, NAN);
	      cpl_image_set(*err, x, y, NAN);
	      cpl_image_reject(*out, x, y);
	      cpl_image_reject(*err, x, y);
	      cpl_image_set(*contrib, x, y, 0);
	      cpl_errorstate_set(prestate);
	  }
	  hdrl_cplvector_delete_to_cache(cache, vd);
	  hdrl_cplvector_delete_to_cache(cache, ve);
      }
  }
  hdrl_vector_cache_delete(cache);

  return cpl_error_get_code();}


static void *
hdrl_nop_create_eout_vec(cpl_size HDRL_UNUSED(size))
{
    return NULL;
}

static void *
hdrl_nop_create_eout_img(const cpl_image * HDRL_UNUSED(img))
{
    return NULL;
}

static cpl_error_code
hdrl_nop_move_eout(void * HDRL_UNUSED(dst_),
                   void * HDRL_UNUSED(src_), const cpl_size HDRL_UNUSED(y))
{
    return CPL_ERROR_NONE;
}

static void
hdrl_nop_unwrap_eout(void * HDRL_UNUSED(dst))
{
    return;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief reduction object to reduce imagelist via mean
 * @return mean reduction object
 */
/* ---------------------------------------------------------------------------*/
hdrl_collapse_imagelist_to_image_t * 
    hdrl_collapse_imagelist_to_image_mean(void)
{
    hdrl_collapse_imagelist_to_image_t * s = cpl_calloc(1, sizeof(*s));
    s->func = &hdrl_collapse_mean;
    s->create_eout = &hdrl_nop_create_eout_img;
    s->move_eout = &hdrl_nop_move_eout;
    s->unwrap_eout = &hdrl_nop_unwrap_eout;
    s->delete_eout = &hdrl_nop_unwrap_eout;
    return s;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief reduction object to reduce imagelist via weighted mean
 * @return weighted mean reduction object
 */
/* ---------------------------------------------------------------------------*/
hdrl_collapse_imagelist_to_image_t *
    hdrl_collapse_imagelist_to_image_weighted_mean(void)
{
    hdrl_collapse_imagelist_to_image_t * s = cpl_calloc(1, sizeof(*s));
    s->func = &hdrl_collapse_weighted_mean;
    s->create_eout = &hdrl_nop_create_eout_img;
    s->move_eout = &hdrl_nop_move_eout;
    s->unwrap_eout = &hdrl_nop_unwrap_eout;
    s->delete_eout = &hdrl_nop_unwrap_eout;
    return s;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief reduction object to reduce imagelist via median
 * @return median reduction object
 */
/* ---------------------------------------------------------------------------*/
hdrl_collapse_imagelist_to_image_t *
    hdrl_collapse_imagelist_to_image_median(void)
{
    hdrl_collapse_imagelist_to_image_t * s = cpl_calloc(1, sizeof(*s));
    s->func = &hdrl_collapse_median;
    s->create_eout = &hdrl_nop_create_eout_img;
    s->move_eout = &hdrl_nop_move_eout;
    s->unwrap_eout = &hdrl_nop_unwrap_eout;
    s->delete_eout = &hdrl_nop_unwrap_eout;
    return s;
}

static void *
hdrl_sigclip_create_eout_img(const cpl_image * img)
{
    cpl_ensure(img, CPL_ERROR_NULL_INPUT, NULL);
    hdrl_sigclip_image_output * eout = cpl_calloc(sizeof(*eout), 1);
    eout->reject_low = cpl_image_new(cpl_image_get_size_x(img),
                                     cpl_image_get_size_y(img),
                                     cpl_image_get_type(img));
    eout->reject_high = cpl_image_new(cpl_image_get_size_x(img),
                                      cpl_image_get_size_y(img),
                                      cpl_image_get_type(img));
    /* add masks for thread safety on move */
    cpl_image_get_bpm(eout->reject_low);
    cpl_image_get_bpm(eout->reject_high);
    return eout;
}

static void
hdrl_sigclip_delete_eout_img(void * eout_)
{
    if (eout_ == NULL) {
        return;
    }
    hdrl_sigclip_image_output * eout = (hdrl_sigclip_image_output*)eout_;
    cpl_image_delete(eout->reject_low);
    cpl_image_delete(eout->reject_high);
    cpl_free(eout);
}

static cpl_error_code
hdrl_sigclip_move_eout_img(void * dst_, void * src_, const cpl_size y)
{
    hdrl_sigclip_image_output * dst = dst_;
    hdrl_sigclip_image_output * src = src_;
    cpl_ensure_code(dst, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(src, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(y > 0, CPL_ERROR_ACCESS_OUT_OF_RANGE);
    cpl_ensure_code(y <= cpl_image_get_size_y(dst->reject_low),
                    CPL_ERROR_ACCESS_OUT_OF_RANGE);

    cpl_image_copy(dst->reject_low, src->reject_low, 1, y);
    cpl_image_copy(dst->reject_high, src->reject_high, 1, y);
    cpl_image_delete(src->reject_low);
    cpl_image_delete(src->reject_high);
    cpl_free(src);
    return cpl_error_get_code();
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief reduction object to reduce imagelist via kappa sigma clipped mean
 *
 * @param kappa_low  low sigma bound
 * @param kappa_high high sigma bound
 * @param niter      number of clipping iterators
 *
 * @return sigma clip reduction object
 * @see  hdrl_kappa_sigma_clip()
 *
 * the high and low reject values are stored in extra_out if applicable
 */
/* ---------------------------------------------------------------------------*/
hdrl_collapse_imagelist_to_image_t *
hdrl_collapse_imagelist_to_image_sigclip(double kappa_low,
                                       double kappa_high,
                                       int niter)
{
    hdrl_collapse_imagelist_to_image_t * s = cpl_calloc(1, sizeof(*s));
    hdrl_parameter * sp =
        hdrl_collapse_sigclip_parameter_create(kappa_low, kappa_high, niter);
    s->func = &hdrl_collapse_sigclip;
    s->create_eout = &hdrl_sigclip_create_eout_img;
    s->move_eout = &hdrl_sigclip_move_eout_img;
    s->unwrap_eout = &cpl_free;
    s->delete_eout = &hdrl_sigclip_delete_eout_img;
    s->parameters = sp;
    return s;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief reduction object to reduce imagelist via minmax clipped mean
 *
 * @param nlow  number of low pixel to reject
 * @param nhigh number of high pixel to reject
 *
 * @return minmax cliped reduction object
 * @see  hdrl_minmax_clip()
 *
 *
 */
/* ---------------------------------------------------------------------------*/
hdrl_collapse_imagelist_to_image_t *
hdrl_collapse_imagelist_to_image_minmax(double nlow,
                                       double nhigh)
{
    hdrl_collapse_imagelist_to_image_t * s = cpl_calloc(1, sizeof(*s));
    hdrl_parameter * sp =
        hdrl_collapse_minmax_parameter_create(nlow, nhigh);
    s->func = &hdrl_collapse_minmax;
    s->create_eout = &hdrl_sigclip_create_eout_img;
    s->move_eout = &hdrl_sigclip_move_eout_img;
    s->unwrap_eout = &cpl_free;
    s->delete_eout = &hdrl_sigclip_delete_eout_img;
    s->parameters = sp;
    return s;
}
/* ---------------------------------------------------------------------------*/
/**
 * @brief reduction object to reduce imagelist via mode
 *
 * @param histo_min  minimum value of low pixels to use
 * @param histo_max  maximum value of high pixels to be use
 * @param bin_size   size of the histogram bin
 * @param method     method to use for the mode computation
 * @param error_niter  number of iterations to compute the error of the mode
 *
 * @return mode reduction object
 * @see  hdrl_mode_clip()
 *
 *
 */
/* ---------------------------------------------------------------------------*/
hdrl_collapse_imagelist_to_image_t *
hdrl_collapse_imagelist_to_image_mode(	double histo_min,
					double histo_max,
					double bin_size,
					hdrl_mode_type method,
					cpl_size error_niter)
{
    hdrl_collapse_imagelist_to_image_t * s = cpl_calloc(1, sizeof(*s));
    hdrl_parameter * sp =
        hdrl_collapse_mode_parameter_create(histo_min, histo_max, bin_size,
					    method, error_niter);
    s->func = &hdrl_collapse_mode;
    s->create_eout = &hdrl_nop_create_eout_img;
    s->move_eout = &hdrl_nop_move_eout;
    s->unwrap_eout = &hdrl_nop_unwrap_eout;
    s->delete_eout = &hdrl_nop_unwrap_eout;
    s->parameters = sp;
    return s;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Call the associated reduction function
 *
 * @param f       reduction function object
 * @param data    data to apply function on
 * @param errors  errors to use for propagation
 * @param out     pointer which will contain reduced data image, type double
 * @param err     pointer which will contain reduced error image, type double
 * @param contrib pointer which will contain contribution map, type integer
 * @param eout    storage for extra output, may be NULL
 *
 * @return cpl_error_code
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code
hdrl_collapse_imagelist_to_image_call(hdrl_collapse_imagelist_to_image_t * f,
                                    const cpl_imagelist * data,
                                    const cpl_imagelist * errors,
                                    cpl_image ** out,
                                    cpl_image ** err,
                                    cpl_image ** contrib,
                                    void ** eout)
{
    cpl_ensure_code(f, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(data, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(errors, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(out, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(err, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(contrib, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(cpl_imagelist_get_size(data) ==
                    cpl_imagelist_get_size(errors),
                    CPL_ERROR_INCOMPATIBLE_INPUT);
    if (eout) {
        *eout = f->create_eout(cpl_imagelist_get_const(data, 0));
    }

    cpl_imagelist * nerrors = wrap_synced_errlist(data, errors);
    if (nerrors == NULL) {
        return cpl_error_get_code();
    }

    cpl_error_code errcode = f->func(data, nerrors, out, err, contrib,
                                     f->parameters, eout ? *eout :  NULL);

    unwrap_synced_errlist(nerrors);

    return errcode;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief call extra output creation function
 *
 * @param f     reduction function object
 * @param data  an image of same size and type as the intended reductee
 *
 * @return reduction objects extra output function, its entries must be deleted
 *         by the caller and the structure unwraped with the unwrap_eout
 *         function
 */
/* ---------------------------------------------------------------------------*/
void *
hdrl_collapse_imagelist_to_image_create_eout(
                         hdrl_collapse_imagelist_to_image_t * f,
                         const cpl_image * data)
{
    cpl_ensure(f, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(data, CPL_ERROR_NULL_INPUT, NULL);

    return f->create_eout(data);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief call extra output unwrap function
 *
 * @param f     reduction function object
 *
 * @note does not delete the members
 *
 */
/* ---------------------------------------------------------------------------*/
void
hdrl_collapse_imagelist_to_image_unwrap_eout(
                         hdrl_collapse_imagelist_to_image_t * f,
                         void * eout)
{
    if (f != NULL) {
        f->unwrap_eout(eout);
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief call extra output delete function
 *
 * @param f     reduction function object
 *
 * @note does delete the members
 *
 */
/* ---------------------------------------------------------------------------*/
void
hdrl_collapse_imagelist_to_image_delete_eout(
                         hdrl_collapse_imagelist_to_image_t * f,
                         void * eout)
{
    if (f != NULL) {
        f->delete_eout(eout);
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief call function to move extra output do destination with offset
 *
 * @param f     reduction function object
 * @param dst   destination output
 * @param src   source output
 * @param y     offset in y, see cpl_image_copy
 *
 * @return cpl_error_code
 *
 * @note deletes the source after the content is copied
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code
hdrl_collapse_imagelist_to_image_move_eout(
                       hdrl_collapse_imagelist_to_image_t * f,
                       void * dst,
                       void * src,
                       cpl_size y)
{
    cpl_ensure_code(f, CPL_ERROR_NULL_INPUT);

    return f->move_eout(dst, src, y);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief delete imagelist reduction object
 *
 * @param p  imagelist reduction object or NULL
 */
/* ---------------------------------------------------------------------------*/
void hdrl_collapse_imagelist_to_image_delete(hdrl_collapse_imagelist_to_image_t * p)
{
    if (p) {
        hdrl_parameter_delete(p->parameters);
    }
    cpl_free(p);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief call extra output creation function
 *
 * @param f     reduction function object
 * @param size  size of the imagelist which should be reduced
 *
 * @return reduction objects extra output function, its entries must be deleted
 *         by the caller and the structure unwraped with the unwrap_eout
 *         function
 */
/* ---------------------------------------------------------------------------*/
void * hdrl_collapse_imagelist_to_vector_create_eout(
                         hdrl_collapse_imagelist_to_vector_t * f,
                         const cpl_size size)
{
    cpl_ensure(f, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(size > 0, CPL_ERROR_ILLEGAL_INPUT, NULL);

    return f->create_eout(size);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief call extra output unwrap function
 *
 * @param f     reduction function object
 *
 * @note does not delete the members
 *
 */
/* ---------------------------------------------------------------------------*/
void
hdrl_collapse_imagelist_to_vector_unwrap_eout(
                         hdrl_collapse_imagelist_to_vector_t * f,
                         void * eout)
{
    if (f != NULL) {
        f->unwrap_eout(eout);
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief call extra output delete function
 *
 * @param f     reduction function object
 *
 * @note does delete the members
 *
 */
/* ---------------------------------------------------------------------------*/
void hdrl_collapse_imagelist_to_vector_delete_eout(
                         hdrl_collapse_imagelist_to_vector_t * f,
                         void * eout)
{
    if (f != NULL) {
        f->delete_eout(eout);
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief call function to move extra output do destination with offset
 *
 * @param f     reduction function object
 * @param dst   destination output
 * @param src   source output
 * @param y     offset in y, see cpl_image_copy
 *
 * @return cpl_error_code
 *
 * @note deletes the source after the content is copied
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_collapse_imagelist_to_vector_move_eout(
                       hdrl_collapse_imagelist_to_vector_t * f,
                       void * dst,
                       void * src,
                       cpl_size y)
{
    cpl_ensure_code(f, CPL_ERROR_NULL_INPUT);

    return f->move_eout(dst, src, y);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief implements mean reduction on each image of an imagelist
 *
 * @param data    input data images
 * @param errors  input errors images
 * @param out     vector of median of each image
 * @param err     vector of errors of median of each image
 * @param contrib array of contributions
 * @param parameters parameters to control, not used
 * @param extra_out  optional extra output, not used
 *
 * @return cpl_error_code
 *
 * @doc
 * The mean value on all good pixels of each image of an imagelist, the
 * associated error and the number of good pixels are stored as elements
 * of the corresponding output vectors.
 * If all pixels of an image in the list are bad the contribution is 0 and the
 * out and err are set to NAN.
 *
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code
reduce_imagelist_to_vector_mean(const cpl_imagelist * data,
                                const cpl_imagelist * errors,
                                cpl_vector ** out, cpl_vector ** err,
                                cpl_array ** contrib,
                                void * HDRL_UNUSED(parameters),
                                void * HDRL_UNUSED(extra_out))
{
    size_t nz = cpl_imagelist_get_size(data);
    *out = cpl_vector_new(nz);
    *err = cpl_vector_new(nz);
    *contrib = cpl_array_new(nz, CPL_TYPE_INT);

    for (size_t i = 0; i < nz; i++) {
        const cpl_image * img = cpl_imagelist_get_const(data, i);
        const cpl_image * ierr = cpl_imagelist_get_const(errors, i);
        size_t naccepted = hdrl_get_image_good_npix(img);

        if (naccepted != 0) {
            double error = sqrt(cpl_image_get_sqflux(ierr)) / naccepted;

            cpl_vector_set(*out, i, cpl_image_get_mean(img));
            cpl_vector_set(*err, i, error);
        }
        else {
            cpl_vector_set(*out, i, NAN);
            cpl_vector_set(*err, i, NAN);
        }
        cpl_array_set_int(*contrib, i, naccepted);
    }

    return cpl_error_get_code();
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief reduction object to reduce imagelist via mean
 * @return mean reduction object
 */
/* ---------------------------------------------------------------------------*/
hdrl_collapse_imagelist_to_vector_t *
    hdrl_collapse_imagelist_to_vector_mean(void)
{
    hdrl_collapse_imagelist_to_vector_t * s = cpl_calloc(1, sizeof(*s));
    s->create_eout = &hdrl_nop_create_eout_vec;
    s->move_eout = &hdrl_nop_move_eout;
    s->unwrap_eout = &hdrl_nop_unwrap_eout;
    s->delete_eout = &hdrl_nop_unwrap_eout;
    s->func = &reduce_imagelist_to_vector_mean;
    return s;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief implements weighted mean reduction on each image of an imagelist
 *
 * @param data    input data images
 * @param errors  input errors images
 * @param out     vector of median of each image
 * @param err     vector of errors of median of each image
 * @param contrib array of contributions
 * @param parameters parameters to control, not used
 * @param extra_out  optional extra output, not used
 *
 * @return cpl_error_code
 *
 * @doc
 * weighted mean and associated error are computed with standard formulae
 *
 * \f$
 *   x_{mean}=\frac{(\sum_{i}^{n} w_{i} \cdot x_{i})} { \sum_{i}^{n} w_{i} }
 * \f$
 *
 * \f$
 *   \sigma_{x}=\frac{ 1 } { \sqrt{  \sum_{i}^{n} w_{i}^{2} } }
 * \f$
 *
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code
reduce_imagelist_to_vector_weighted_mean(const cpl_imagelist * data,
                                         const cpl_imagelist * errors,
                                         cpl_vector ** out, cpl_vector ** err,
                                         cpl_array ** contrib,
                                         void * HDRL_UNUSED(parameters),
                                         void * HDRL_UNUSED(extra_out))
{
    size_t nz = cpl_imagelist_get_size(data);
    *out = cpl_vector_new(nz);
    *err = cpl_vector_new(nz);
    *contrib = cpl_array_new(nz, CPL_TYPE_INT);

    for (size_t i = 0; i < nz; i++) {
        cpl_image * img =
            cpl_image_duplicate(cpl_imagelist_get_const(data, i));
        cpl_image * ierr =
            cpl_image_duplicate(cpl_imagelist_get_const(errors, i));
        size_t naccepted = hdrl_get_image_good_npix(img);

        if (naccepted != 0) {
            /* (\Sum_i^n w_i * x_i) / (\Sum_i^n w_i) */
            /* 1 / \sqrt(\Sum_i^n w_i^2) */
            cpl_image_power(ierr, -2);
            /* ierr = weights now */
            cpl_image_multiply(img, ierr);
            double sum_v = cpl_image_get_mean(img) * naccepted;
            double sum_w = cpl_image_get_mean(ierr) * naccepted;
            double wmean = sum_v / sum_w;
            double error = 1. / sqrt(sum_w);

            cpl_vector_set(*out, i, wmean);
            cpl_vector_set(*err, i, error);
        }
        else {
            cpl_vector_set(*out, i, NAN);
            cpl_vector_set(*err, i, NAN);
        }
        cpl_array_set_int(*contrib, i, naccepted);
        cpl_image_delete(img);
        cpl_image_delete(ierr);
    }

    return cpl_error_get_code();
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief reduction object to reduce imagelist via weighted mean
 * @return weighted mean reduction object
 */
/* ---------------------------------------------------------------------------*/
hdrl_collapse_imagelist_to_vector_t *
    hdrl_collapse_imagelist_to_vector_weighted_mean(void)
{
    hdrl_collapse_imagelist_to_vector_t * s = cpl_calloc(1, sizeof(*s));
    s->create_eout = &hdrl_nop_create_eout_vec;
    s->move_eout = &hdrl_nop_move_eout;
    s->unwrap_eout = &hdrl_nop_unwrap_eout;
    s->delete_eout = &hdrl_nop_unwrap_eout;
    s->func = &reduce_imagelist_to_vector_weighted_mean;
    return s;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief implements median reduction on each image of an imagelist
 *
 * @param data    input data images
 * @param errors  input errors images
 * @param out     vector of median of each image
 * @param err     vector of errors of median of each image
 * @param contrib array of contributions
 * @param parameters parameters to control, not used
 * @param extra_out  optional extra output, not used
 * @return cpl_error_code
 *
 * @doc
 * The median value on all good pixels of each image of an imagelist, the
 * associated error and the number of good pixels are stored as elements
 * of the corresponding output vectors.
 * If all pixels of an image in the list are bad the contribution is 0 and the
 * out and err are set to NAN.
 * The errors are scaled by the sqrt of the statistical efficiency of the
 * median on normal distributed data which is \f$ \frac{ \pi }{ 2 } \f$
 *
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code
reduce_imagelist_to_vector_median(const cpl_imagelist * data,
                                  const cpl_imagelist * errors,
                                  cpl_vector ** out, cpl_vector ** err,
                                  cpl_array ** contrib,
                                  void * HDRL_UNUSED(parameters),
                                  void * HDRL_UNUSED(extra_out))
{
    size_t nz = cpl_imagelist_get_size(data);
    *out = cpl_vector_new(nz);
    *err = cpl_vector_new(nz);
    *contrib = cpl_array_new(nz, CPL_TYPE_INT);

    for (size_t i = 0; i < nz; i++) {
        const cpl_image * img = cpl_imagelist_get_const(data, i);
        const cpl_image * ierr = cpl_imagelist_get_const(errors, i);
        size_t naccepted = hdrl_get_image_good_npix(img);

        if (naccepted != 0) {
            double error = sqrt(cpl_image_get_sqflux(ierr)) / naccepted;
            /* sqrt(statistical efficiency on normal data)*/
            if (naccepted > 2) {
                error *= sqrt(CPL_MATH_PI_2);
            }

            cpl_vector_set(*out, i, cpl_image_get_median(img));
            cpl_vector_set(*err, i, error);
        }
        else {
            cpl_vector_set(*out, i, NAN);
            cpl_vector_set(*err, i, NAN);
        }
        cpl_array_set_int(*contrib, i, naccepted);
    }

    return cpl_error_get_code();
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief reduction object to reduce imagelist via median
 * @return median reduction object
 */
/* ---------------------------------------------------------------------------*/
hdrl_collapse_imagelist_to_vector_t *
    hdrl_collapse_imagelist_to_vector_median(void)
{
    hdrl_collapse_imagelist_to_vector_t * s = cpl_calloc(1, sizeof(*s));
    s->create_eout = &hdrl_nop_create_eout_vec;
    s->move_eout = &hdrl_nop_move_eout;
    s->unwrap_eout = &hdrl_nop_unwrap_eout;
    s->delete_eout = &hdrl_nop_unwrap_eout;
    s->func = &reduce_imagelist_to_vector_median;
    return s;
}

static void *
hdrl_sigclip_create_eout_vec(cpl_size size)
{
    hdrl_sigclip_vector_output * eout = cpl_calloc(sizeof(*eout), 1);
    eout->reject_low = cpl_vector_new(size);
    eout->reject_high = cpl_vector_new(size);
    return eout;
}

static void
hdrl_sigclip_delete_eout_vec(void * eout_)
{
    if (eout_ == NULL) {
        return;
    }
    hdrl_sigclip_vector_output * eout = (hdrl_sigclip_vector_output*)eout_;
    cpl_vector_delete(eout->reject_low);
    cpl_vector_delete(eout->reject_high);
    cpl_free(eout);
}

static cpl_error_code
hdrl_sigclip_move_eout_vec(void * dst_, void * src_, const cpl_size y)
{
    hdrl_sigclip_vector_output * dst = dst_;
    hdrl_sigclip_vector_output * src = src_;
    cpl_ensure_code(dst, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(src, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(y >= 0, CPL_ERROR_ACCESS_OUT_OF_RANGE);
    cpl_ensure_code(y < cpl_vector_get_size(dst->reject_low),
                    CPL_ERROR_ACCESS_OUT_OF_RANGE);

    double * ddst = cpl_vector_get_data(dst->reject_low);
    double * dsrc = cpl_vector_get_data(src->reject_low);
    memcpy(ddst + y, dsrc, cpl_vector_get_size(src->reject_low));
    ddst = cpl_vector_get_data(dst->reject_high);
    dsrc = cpl_vector_get_data(src->reject_high);
    memcpy(ddst + y, dsrc, cpl_vector_get_size(src->reject_high));
    cpl_vector_delete(src->reject_low);
    cpl_vector_delete(src->reject_high);
    cpl_free(src);
    return cpl_error_get_code();
}

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief implements sigma-clipped combination on input image list into a vector
 *
 * @param data    input data imagelist
 * @param errors  input errors imagelist
 * @param out     output vector
 * @param err     output vector errors
 * @param contrib output contribution vector
 *
 * @return cpl_error_code
 *
 * If all pixels of an image in the list are rejected the contribution is 0 and
 * the out and err are set to NAN.
 *
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code
reduce_imagelist_to_vector_sigclip(const cpl_imagelist * data,
                                   const cpl_imagelist * errors,
                                   cpl_vector ** out, cpl_vector ** err,
                                   cpl_array ** contrib, void * parameters,
                                   void * extra_out)
{
    hdrl_collapse_sigclip_parameter * par = parameters;
    hdrl_minmax_vector_output * eout =
        (hdrl_minmax_vector_output *)extra_out;

    cpl_size nz = cpl_imagelist_get_size(data);
    *out = cpl_vector_new(nz);
    *err = cpl_vector_new(nz);
    *contrib = cpl_array_new(nz, CPL_TYPE_INT);

    /* sigmaclip on each image of the imagelist */
    for (cpl_size z = 0; z < nz ; z++) {
        double corr, error, low, high;
        cpl_size contribution;
        if (hdrl_kappa_sigma_clip_image(cpl_imagelist_get_const(data, z),
                                        cpl_imagelist_get_const(errors, z),
                                        par->kappa_low,
                                        par->kappa_high,
                                        par->niter,
                                        &corr,
                                        &error,
                                        &contribution,
                                        &low,
                                        &high) != CPL_ERROR_NONE) {
            break;
        }
        cpl_vector_set(*out, z, corr);
        cpl_vector_set(*err, z, error);
        cpl_array_set_int(*contrib, z, contribution);

        if (eout) {
            cpl_vector_set(eout->reject_low, z, low);
            cpl_vector_set(eout->reject_high, z, high);
        }
    }

    return cpl_error_get_code();
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief reduction object to reduce imagelist to a vector via kappa sigma
 * clipped mean
 *
 * @param kappa_low  low sigma bound
 * @param kappa_high high sigma bound
 * @param niter      maximum number of clipping iterations
 *
 * @return sigma clip reduction object
 * @see  hdrl_kappa_sigma_clip()
 *
 * the high and low reject values are stored in extra_out if applicable
 */
/* ---------------------------------------------------------------------------*/
hdrl_collapse_imagelist_to_vector_t *
hdrl_collapse_imagelist_to_vector_sigclip(double kappa_low, double kappa_high,
                                          int niter)
{
    hdrl_collapse_imagelist_to_vector_t * s = cpl_calloc(1, sizeof(*s));
    hdrl_parameter * sp =
        hdrl_collapse_sigclip_parameter_create(kappa_low, kappa_high, niter);
    s->func = &reduce_imagelist_to_vector_sigclip;
    s->create_eout = &hdrl_sigclip_create_eout_vec;
    s->move_eout = &hdrl_sigclip_move_eout_vec;
    s->unwrap_eout = &cpl_free;
    s->delete_eout = &hdrl_sigclip_delete_eout_vec;
    s->parameters = sp;
    return s;
}

/* ---------------------------------------------------------------------------*/
/**
 * @internal
 * @brief implements minmax-clipped combination on input image list into a vector
 *
 * @param data    input data imagelist
 * @param errors  input errors imagelist
 * @param out     output vector
 * @param err     output vector errors
 *
 * @return cpl_error_code
 *
 * If all pixels of an image in the list are rejected the contribution is 0 and
 * the out and err are set to NAN.
 *
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code
reduce_imagelist_to_vector_minmax(const cpl_imagelist * data,
                                   const cpl_imagelist * errors,
                                   cpl_vector ** out, cpl_vector ** err,
                                   cpl_array ** contrib, void * parameters,
                                   void * extra_out)
{
    hdrl_collapse_minmax_parameter * par = parameters;
    hdrl_sigclip_vector_output * eout =
        (hdrl_sigclip_vector_output *)extra_out;
    cpl_size nz = cpl_imagelist_get_size(data);
    *out = cpl_vector_new(nz);
    *err = cpl_vector_new(nz);
    *contrib = cpl_array_new(nz, CPL_TYPE_INT);

    /* minmax on each image of the imagelist */
    for (cpl_size z = 0; z < nz ; z++) {
        double corr, error, low, high;
        cpl_size contribution;
        if (hdrl_minmax_clip_image(cpl_imagelist_get_const(data, z),
                                        cpl_imagelist_get_const(errors, z),
                                        par->nlow,
                                        par->nhigh,
                                        &corr,
                                        &error,
                                        &contribution,
                                        &low, &high) != CPL_ERROR_NONE) {
            break;
        }
        cpl_vector_set(*out, z, corr);
        cpl_vector_set(*err, z, error);
        cpl_array_set_int(*contrib, z, contribution);

        if (eout) {
            cpl_vector_set(eout->reject_low, z, low);
            cpl_vector_set(eout->reject_high, z, high);
        }
    }

    return cpl_error_get_code();
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief reduction object to reduce imagelist to a vector via min-max rejection
 *
 * @param nlow  low bound
 * @param nhigh  high bound
 *
 * @return minmax rejected reduction object
 * @see  hdrl_minmax()
 *
 */
/* ---------------------------------------------------------------------------*/
hdrl_collapse_imagelist_to_vector_t *
hdrl_collapse_imagelist_to_vector_minmax(double nlow, double nhigh)
{
    hdrl_collapse_imagelist_to_vector_t * s = cpl_calloc(1, sizeof(*s));
    hdrl_parameter * sp =
        hdrl_collapse_minmax_parameter_create(nlow, nhigh);
    s->func = &reduce_imagelist_to_vector_minmax;
    s->create_eout = &hdrl_sigclip_create_eout_vec;
    s->move_eout = &hdrl_sigclip_move_eout_vec;
    s->unwrap_eout = &cpl_free;
    s->delete_eout = &hdrl_sigclip_delete_eout_vec;
    s->parameters = sp;
    return s;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief implements mode reduction on each image of an imagelist
 *
 * @param data    input data images
 * @param errors  input errors images
 * @param out     vector of mode of each image
 * @param err     vector of errors of mode of each image
 * @param contrib array of contributions
 * @param parameters parameters to control the mode computation
 * @param extra_out  optional extra output, not used
 * @return cpl_error_code
 *
 * @doc
 * The mode value on all good pixels of each image of an imagelist, the
 * associated error and the number of good pixels are stored as elements
 * of the corresponding output vectors.
 * If all pixels of an image in the list are bad the contribution is 0 and the
 * out and err are set to NAN.
 *
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code
reduce_imagelist_to_vector_mode(const cpl_imagelist * data,
				const cpl_imagelist * HDRL_UNUSED(errors),
				cpl_vector ** out, cpl_vector ** err,
				cpl_array ** contrib, void * parameters,
				void * HDRL_UNUSED(extra_out))
{

    hdrl_collapse_mode_parameter * par = parameters;
    cpl_size nz = cpl_imagelist_get_size(data);
    *out = cpl_vector_new(nz);
    *err = cpl_vector_new(nz);
    *contrib = cpl_array_new(nz, CPL_TYPE_INT);

    /* mode on each image of the imagelist */
    for (cpl_size z = 0; z < nz ; z++) {
      double corr, error;
        cpl_size contribution;
        if (hdrl_mode_clip_image(cpl_imagelist_get_const(data, z),
                                        par->histo_min,
                                        par->histo_max,
					par->bin_size,
					par->method,
					par->error_niter,
                                        &corr,
                                        &error,
                                        &contribution) != CPL_ERROR_NONE) {
            break;
        }
        cpl_vector_set(*out, z, corr);
        cpl_vector_set(*err, z, error);
        cpl_array_set_int(*contrib, z, contribution);

    }

    return cpl_error_get_code();
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief reduction object to reduce imagelist via mode
 * @return mode reduction object
 */
/* ---------------------------------------------------------------------------*/
hdrl_collapse_imagelist_to_vector_t *
    hdrl_collapse_imagelist_to_vector_mode(double histo_min, double histo_max,
				       double bin_size, hdrl_mode_type method,
				       cpl_size error_niter)
{

    hdrl_collapse_imagelist_to_vector_t * s = cpl_calloc(1, sizeof(*s));
    hdrl_parameter * sp =
        hdrl_collapse_mode_parameter_create(histo_min, histo_max, bin_size, method, error_niter);

    s->func = &reduce_imagelist_to_vector_mode;
    s->create_eout = &hdrl_nop_create_eout_vec;
    s->move_eout =   &hdrl_nop_move_eout;
    s->unwrap_eout = &hdrl_nop_unwrap_eout;
    s->delete_eout = &hdrl_nop_unwrap_eout;
    s->parameters = sp;
    return s;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Call the associated reduction function
 *
 * @param f       reduction function object
 * @param data    data to apply function on
 * @param errors  errors to use for propagation
 * @param out     pointer which will contain reduced data image, type double
 * @param err     pointer which will contain reduced error image, type double
 * @param contrib pointer which will contain contribution map, type integer
 * @param eout    storage for extra output, may be NULL
 *
 * @return cpl_error_code
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code
hdrl_collapse_imagelist_to_vector_call(hdrl_collapse_imagelist_to_vector_t * f,
                                     const cpl_imagelist * data,
                                     const cpl_imagelist * errors,
                                     cpl_vector ** out,
                                     cpl_vector ** err,
                                     cpl_array ** contrib,
                                     void ** eout)
{
    cpl_ensure_code(f, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(data, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(errors, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(out, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(err, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(contrib, CPL_ERROR_NULL_INPUT);
    if (eout) {
        *eout = f->create_eout(cpl_imagelist_get_size(data));
    }

    cpl_imagelist * nerrors = wrap_synced_errlist(data, errors);
    if (nerrors == NULL) {
        return cpl_error_get_code();
    }

    cpl_error_code errcode = f->func(data, errors, out, err, contrib,
                                     f->parameters, eout ? *eout :  NULL);

    unwrap_synced_errlist(nerrors);

    return errcode;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief delete imagelist reduction object
 *
 * @param p  imagelist reduction object or NULL
 */
/* ---------------------------------------------------------------------------*/
void
hdrl_collapse_imagelist_to_vector_delete(hdrl_collapse_imagelist_to_vector_t * p)
{
    if (p) {
        cpl_free(p->parameters);
    }
    cpl_free(p);
}

/** @endcond */

/**@}*/
