/*
 * This file is part of the HDRL
 * Copyright (C) 2020 European Southern Observatory
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

#ifndef HDRL_RESAMPLE_H
#define HDRL_RESAMPLE_H

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>
#include <hdrl_parameter.h>
CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                                Defines
 -----------------------------------------------------------------------------*/

           /* Table columns of the resampling table. */

#define HDRL_RESAMPLE_TABLE_RA     "ra"     /* Right ascension column of the table*/
#define HDRL_RESAMPLE_TABLE_DEC    "dec"    /* Declination column of the table*/
#define HDRL_RESAMPLE_TABLE_LAMBDA "lambda" /* wavelength column of the table */
#define HDRL_RESAMPLE_TABLE_DATA   "data"   /* data column of the table       */
#define HDRL_RESAMPLE_TABLE_BPM    "bpm"    /* bpm column of the table        */
#define HDRL_RESAMPLE_TABLE_ERRORS "errors"  /* error column of the table      */

/* Right ascension column type of the table*/
#define HDRL_RESAMPLE_TABLE_RA_TYPE     CPL_TYPE_DOUBLE
/* Declination column type of the table*/
#define HDRL_RESAMPLE_TABLE_DEC_TYPE    CPL_TYPE_DOUBLE
/* wavelength column type of the table */
#define HDRL_RESAMPLE_TABLE_LAMBDA_TYPE CPL_TYPE_DOUBLE
/* data column type of the table       */
#define HDRL_RESAMPLE_TABLE_DATA_TYPE   CPL_TYPE_DOUBLE
/* bpm column type of the table        */
#define HDRL_RESAMPLE_TABLE_BPM_TYPE    CPL_TYPE_INT
/* error type column of the table      */
#define HDRL_RESAMPLE_TABLE_ERRORS_TYPE CPL_TYPE_DOUBLE

typedef enum {
  HDRL_RESAMPLE_METHOD_NEAREST = 0,
  HDRL_RESAMPLE_METHOD_RENKA = 1,
  HDRL_RESAMPLE_METHOD_LINEAR,
  HDRL_RESAMPLE_METHOD_QUADRATIC,
  HDRL_RESAMPLE_METHOD_DRIZZLE,
  HDRL_RESAMPLE_METHOD_LANCZOS,
  HDRL_RESAMPLE_METHOD_NONE  /* Can be used for range checking */
} hdrl_resample_method ;

typedef enum {
  HDRL_RESAMPLE_OUTGRID_2D,
  HDRL_RESAMPLE_OUTGRID_3D
} hdrl_resample_outgrid ;

typedef struct {
  cpl_propertylist * header; /* cpl propertylist containing the wcs informations */
  hdrl_imagelist * himlist; /* hdrl imagelist containing the data/errors/bpm */
} hdrl_resample_result;

/*----------------------------------------------------------------------------
                            RESAMPLING Computation
  ----------------------------------------------------------------------------*/

cpl_table*
hdrl_resample_imagelist_to_table(const hdrl_imagelist * himlist,
				 const cpl_wcs* wcs);
cpl_table*
hdrl_resample_image_to_table(const hdrl_image * hima, const cpl_wcs* wcs);

hdrl_resample_result *
hdrl_resample_compute(const cpl_table *restable,
		      hdrl_parameter *method,
		      hdrl_parameter *outputgrid,
		      const cpl_wcs* wcs);

void 
hdrl_resample_result_delete(hdrl_resample_result *resdata);

/*----------------------------------------------------------------------------
                 RESAMPLING Parameters for output definition
  ----------------------------------------------------------------------------*/

hdrl_parameter*
hdrl_resample_parameter_create_outgrid2D(const double delta_ra,
					 const double delta_dec);

hdrl_parameter*
hdrl_resample_parameter_create_outgrid3D(const double delta_ra,
					 const double delta_dec,
					 const double delta_lambda);

hdrl_parameter*
hdrl_resample_parameter_create_outgrid2D_userdef(const double delta_ra,
						 const double delta_dec,
						 const double ra_min,
						 const double ra_max,
						 const double dec_min,
						 const double dec_max,
						 const double fieldmargin);

hdrl_parameter*
hdrl_resample_parameter_create_outgrid3D_userdef(const double delta_ra,
						 const double delta_dec,
						 const double delta_lambda,
						 const double ra_min,
						 const double ra_max,
						 const double dec_min,
						 const double dec_max,
						 const double lambda_min,
						 const double lambda_max,
						 const double fieldmargin);

/*----------------------------------------------------------------------------
             RESAMPLING Parameters for method definition
  ----------------------------------------------------------------------------*/

hdrl_parameter*
hdrl_resample_parameter_create_renka(const int loop_distance,
				     cpl_boolean use_errorweights,
				     const double critical_radius);
hdrl_parameter*
hdrl_resample_parameter_create_drizzle(const int loop_distance,
				       cpl_boolean use_errorweights,
				       const double pix_frac_x,
				       const double pix_frac_y,
				       const double pix_frac_lambda);

hdrl_parameter*
hdrl_resample_parameter_create_nearest(void);

hdrl_parameter*
hdrl_resample_parameter_create_linear(const int loop_distance,
				      cpl_boolean use_errorweights);

hdrl_parameter*
hdrl_resample_parameter_create_quadratic(const int loop_distance,
					 cpl_boolean use_errorweights);

hdrl_parameter*
hdrl_resample_parameter_create_lanczos(const int loop_distance,
				       cpl_boolean use_errorweights,
				       const int kernel_size);

/*----------------------------------------------------------------------------
       Checks and verifications for the RESAMPLING Parameters
  ----------------------------------------------------------------------------*/

cpl_error_code
hdrl_resample_parameter_outgrid_verify(const hdrl_parameter * hp);
cpl_error_code
hdrl_resample_parameter_method_verify(const hdrl_parameter * hp);
int
hdrl_resample_parameter_outgrid_check(const hdrl_parameter * self);
int
hdrl_resample_parameter_method_check(const hdrl_parameter * hp);

/*-----------------------------------------------------------------------------
             Private declarations - must not be used outside of hdrl
 -----------------------------------------------------------------------------*/

#ifdef HDRL_USE_PRIVATE

/* Helper Functions */

cpl_error_code
hdrl_wcs_xy_to_radec(const cpl_wcs *wcs, double x, double y,
		 double *ra, double *dec);

cpl_error_code
hdrl_wcs_to_propertylist(const cpl_wcs * wcs, cpl_propertylist * header,
			 cpl_boolean only2d);

#endif
CPL_END_DECLS

#endif /* RECIPES_HDRL_RESAMPLE_H_ */
