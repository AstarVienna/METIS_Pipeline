/* $Id: hdrl_collapse.h,v 1.11 2013-10-17 15:44:14 jtaylor Exp $
 *
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

/*
 * $Author: jtaylor $
 * $Date: 2013-10-17 15:44:14 $
 * $Revision: 1.11 $
 * $Name: not supported by cvs2svn $
 */

#ifndef HDRL_COLLAPSE_H
#define HDRL_COLLAPSE_H

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_parameter.h"
#include "hdrl_mode.h"

#include <cpl.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

hdrl_parameter * hdrl_collapse_mean_parameter_create(void);
cpl_boolean hdrl_collapse_parameter_is_mean(const hdrl_parameter *);
extern hdrl_parameter * HDRL_COLLAPSE_MEAN;
hdrl_parameter * hdrl_collapse_median_parameter_create(void);
cpl_boolean hdrl_collapse_parameter_is_median(const hdrl_parameter *);
extern hdrl_parameter * HDRL_COLLAPSE_MEDIAN;
hdrl_parameter * hdrl_collapse_weighted_mean_parameter_create(void);
cpl_boolean hdrl_collapse_parameter_is_weighted_mean(const hdrl_parameter *);
extern hdrl_parameter * HDRL_COLLAPSE_WEIGHTED_MEAN;
hdrl_parameter * hdrl_collapse_sigclip_parameter_create(double, double, int);
cpl_boolean hdrl_collapse_parameter_is_sigclip(const hdrl_parameter *);
double hdrl_collapse_sigclip_parameter_get_kappa_high(const hdrl_parameter *);
double hdrl_collapse_sigclip_parameter_get_kappa_low(const hdrl_parameter *);
int hdrl_collapse_sigclip_parameter_get_niter(const hdrl_parameter *);

hdrl_parameter * hdrl_collapse_minmax_parameter_create(double, double);
cpl_boolean hdrl_collapse_parameter_is_minmax(const hdrl_parameter *);
double hdrl_collapse_minmax_parameter_get_nhigh(const hdrl_parameter *);
double hdrl_collapse_minmax_parameter_get_nlow(const hdrl_parameter *);

hdrl_parameter * hdrl_collapse_mode_parameter_create(double, double, double,
						     hdrl_mode_type, cpl_size);
cpl_boolean hdrl_collapse_parameter_is_mode(const hdrl_parameter *);
double hdrl_collapse_mode_parameter_get_histo_min(const hdrl_parameter *);
double hdrl_collapse_mode_parameter_get_histo_max(const hdrl_parameter *);
double hdrl_collapse_mode_parameter_get_bin_size(const hdrl_parameter *);
hdrl_mode_type hdrl_collapse_mode_parameter_get_method(const hdrl_parameter *);
cpl_size hdrl_collapse_mode_parameter_get_error_niter(const hdrl_parameter *);

hdrl_parameter * hdrl_collapse_parameter_parse_parlist(
        const cpl_parameterlist *, const char *);
cpl_parameterlist * hdrl_collapse_parameter_create_parlist(const char *,
        const char *, const char *, hdrl_parameter *, hdrl_parameter *,
	hdrl_parameter *) ;

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
 -----------------------------------------------------------------------------*/

#ifdef HDRL_USE_PRIVATE
cpl_error_code hdrl_collapse_mode_parameter_verify(const hdrl_parameter *);
cpl_error_code hdrl_collapse_minmax_parameter_verify(const hdrl_parameter *);
cpl_error_code hdrl_collapse_sigclip_parameter_verify(const hdrl_parameter *);

typedef struct hdrl_collapse_imagelist_to_image_s
    hdrl_collapse_imagelist_to_image_t;
typedef struct hdrl_collapse_imagelist_to_vector_s
    hdrl_collapse_imagelist_to_vector_t;

typedef struct {
    cpl_image * reject_low;
    cpl_image * reject_high;
} hdrl_sigclip_image_output;
typedef hdrl_sigclip_image_output hdrl_minmax_image_output;

typedef struct {
    cpl_vector * reject_low;
    cpl_vector * reject_high;
} hdrl_sigclip_vector_output;
typedef hdrl_sigclip_vector_output hdrl_minmax_vector_output;

hdrl_collapse_imagelist_to_image_t *
    hdrl_collapse_imagelist_to_image_mean(void);
hdrl_collapse_imagelist_to_image_t *
    hdrl_collapse_imagelist_to_image_weighted_mean(void);
hdrl_collapse_imagelist_to_image_t * 
    hdrl_collapse_imagelist_to_image_median(void);
hdrl_collapse_imagelist_to_image_t *
    hdrl_collapse_imagelist_to_image_sigclip(double, double, int);
hdrl_collapse_imagelist_to_image_t *
    hdrl_collapse_imagelist_to_image_minmax(double, double);
hdrl_collapse_imagelist_to_image_t *
    hdrl_collapse_imagelist_to_image_mode(double, double, double,
					  hdrl_mode_type, cpl_size);
cpl_error_code hdrl_collapse_imagelist_to_image_call(
    hdrl_collapse_imagelist_to_image_t * f,
    const cpl_imagelist * data,
    const cpl_imagelist * errors,
    cpl_image ** out,
    cpl_image ** err,
    cpl_image ** contrib,
    void ** eout);
void *
hdrl_collapse_imagelist_to_image_create_eout(
                         hdrl_collapse_imagelist_to_image_t * f,
                         const cpl_image * data);
void
hdrl_collapse_imagelist_to_image_delete_eout(
                         hdrl_collapse_imagelist_to_image_t * f,
                         void * eout);
void
hdrl_collapse_imagelist_to_image_unwrap_eout(
                         hdrl_collapse_imagelist_to_image_t * f,
                         void * eout);
cpl_error_code
hdrl_collapse_imagelist_to_image_move_eout(
                       hdrl_collapse_imagelist_to_image_t * f,
                       void * dst,
                       void * src,
                       cpl_size y);
void hdrl_collapse_imagelist_to_image_delete(hdrl_collapse_imagelist_to_image_t *);

hdrl_collapse_imagelist_to_vector_t * 
    hdrl_collapse_imagelist_to_vector_mean(void);
hdrl_collapse_imagelist_to_vector_t *
    hdrl_collapse_imagelist_to_vector_weighted_mean(void);
hdrl_collapse_imagelist_to_vector_t * 
    hdrl_collapse_imagelist_to_vector_median(void);
hdrl_collapse_imagelist_to_vector_t *
    hdrl_collapse_imagelist_to_vector_sigclip(double, double, int);
hdrl_collapse_imagelist_to_vector_t *
hdrl_collapse_imagelist_to_vector_minmax(double, double);
hdrl_collapse_imagelist_to_vector_t *
hdrl_collapse_imagelist_to_vector_mode(double, double, double, hdrl_mode_type, cpl_size);

cpl_error_code hdrl_collapse_imagelist_to_vector_call(
    hdrl_collapse_imagelist_to_vector_t * f,
    const cpl_imagelist * data,
    const cpl_imagelist * errors,
    cpl_vector ** out,
    cpl_vector ** err,
    cpl_array ** contrib,
    void ** eout);

void * hdrl_collapse_imagelist_to_vector_create_eout(
                         hdrl_collapse_imagelist_to_vector_t * f,
                         cpl_size);
void hdrl_collapse_imagelist_to_vector_delete_eout(
                         hdrl_collapse_imagelist_to_vector_t * f,
                         void * eout);
void hdrl_collapse_imagelist_to_vector_unwrap_eout(
                         hdrl_collapse_imagelist_to_vector_t * f,
                         void * eout);
cpl_error_code hdrl_collapse_imagelist_to_vector_move_eout(
                       hdrl_collapse_imagelist_to_vector_t * f,
                       void * dst,
                       void * src,
                       cpl_size y);
void hdrl_collapse_imagelist_to_vector_delete(
    hdrl_collapse_imagelist_to_vector_t *);

#endif

CPL_END_DECLS

#endif
