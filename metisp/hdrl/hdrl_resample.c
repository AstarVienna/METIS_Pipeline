/*
 * This file is part of the HDRLDEMO pipeline
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                    Includes
 -----------------------------------------------------------------------------*/
#include <cpl.h>
#include <string.h>
#include <math.h>

#include "hdrl_resample.h"
#include "hdrl_utils.h"

#ifndef _OPENMP
#define omp_get_max_threads() 1
#define omp_get_thread_num() 0
#else
#include <omp.h>
#endif

#include <sys/time.h>

/*-----------------------------------------------------------------------------
                                Defines
 -----------------------------------------------------------------------------*/

/* Not very elegant but it removes compiler warnings */
#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200112L
#elif (_POSIX_C_SOURCE - 0) < 200112L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif


/* maximum keyword length for FITS headers, including '\0' */
#define KEYWORD_LENGTH 81

/* Default field margin (in percent), if the user does not specify any. 5
 * percent is also used in the software package swarp */
#define FIELDMARGIN 5.

/* use bits 0-52 for the value (the pixel table row), this allows to convert
 * pixel tables with up to 9e15 pixels  into a pixel grid    */
#define PT_IDX_MASK 0x1FFFFFFFFFFFFFll

/* use bits 53-62 to store the thread ID, this allows parallelization with up to
 *  1024 cores */
#define XMAP_BITMASK 0x3FFll /* 1023 */
#define XMAP_LSHIFT 53ll

typedef struct {
  double crpix1, crpix2;         /* the reference point                       */
  double crval1, crval2;         /* the coordinate values at the ref. point   */
  double cd11, cd12, cd21, cd22; /* the four elements of the CDi_j matrix     */
  double cddet;                  /* the determinant of the CDi_j matrix       */
} hdrl_resample_smallwcs;

typedef struct {
  unsigned int npix; /* number of pixels in this grid point                   */
  cpl_size *pix;     /* the row number(s) in the pixel table                  */
} hdrl_resample_pixels_ext;

typedef struct {
   /* The pixel grid array, elements can be:
  0:        empty
  positive: row_number in the pixel table
  negative: -(i_ext+1) in the extension array,
  bits 53-62 contain the map index   */
  cpl_size *pix;
  cpl_size nx;             /* horizontal spatial size                         */
  cpl_size ny;             /* vertical spatial size                           */
  cpl_size nz;             /* size in dispersion direction                    */
  unsigned short nmaps;    /* number of extension maps                        */
  cpl_size *nxmap;         /* number of filled pixels in the extension maps   */
  cpl_size *nxalloc;       /* number of allocated pixels in the extension maps */
  hdrl_resample_pixels_ext **xmaps; /* the extension maps                              */
} hdrl_resample_pixgrid;

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_resample  Resample 2D/3D images/cubes

  @brief
   Algorithms to resample 2D images and 3D cubes onto a common grid based on the
   MUSE code.

 The routine currently implements the following algorithms:
  - Nearest neighbor resampling
  - Weighted resampling using Renka weighting function
  - Weighted resampling using inverse distance weighting function
  - Weighted resampling using quadratic inverse distance weighting function
  - Weighted resampling using a drizzle-like weighting scheme
  - Weighted resampling using a lanczos-like restricted sinc for weighting

 The 2D and 3D interpolation is done in 2 dimension and 3 dimension,
 respectively. Moreover, additional error based weights can be taken into
 account.

 The calculation is performed by calling the top-level function
 hdrl_resample_compute(). The latter does not directly work on images but on a
 cpl table. The table is created from a 2D image by calling the function
 hdrl_resample_image_to_table() or from a 3D cube by calling the function
 hdrl_resample_imagelist_to_table(). The advantage of this is that the user
 can combine many images/cubes into a single table and perform the interpolation
 based on all information in one step.

 Beside the world coordinate system (wcs), there are two
 hdrl_parameter passed to the function, one controlling the
 interpolation method and one defining the output grid.

 The parameter controlling the interpolation method can be created by
 - hdrl_resample_parameter_create_nearest(),  or
 - hdrl_resample_parameter_create_renka(),  or
 - hdrl_resample_parameter_create_linear(),  or
 - hdrl_resample_parameter_create_quadratic(),  or
 - hdrl_resample_parameter_create_drizzle(),  or
 - hdrl_resample_parameter_create_lanczos()

 that sets the interpolation method one would like to use.

 The parameter controlling the output grid can be created by
 - hdrl_resample_parameter_create_outgrid2D(),  or
 - hdrl_resample_parameter_create_outgrid3D(),  or
 - hdrl_resample_parameter_create_outgrid2D_userdef(),  or
 - hdrl_resample_parameter_create_outgrid3D_userdef()

 depending on a 2D or 3D interpolation and if the user wants to specify the
 final output grid on a high granularity or the hdrl routine should derive it
 from the input data.

 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_resample_parameter_method  HDRL parameter - interpolation method
  @ingroup  hdrl_resample

 @brief
   HDRL parameter controlling the interpolation method

 The parameter controlling the interpolation method is
 - hdrl_resample_parameter_create_nearest(),  or
 - hdrl_resample_parameter_create_renka(),  or
 - hdrl_resample_parameter_create_linear(),  or
 - hdrl_resample_parameter_create_quadratic(),  or
 - hdrl_resample_parameter_create_drizzle(),  or
 - hdrl_resample_parameter_create_lanczos()

 depending on the interpolation method one would like to use.

 */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_resample_parameter_outgrid  HDRL parameter - output grid
  @ingroup  hdrl_resample

 @brief
   Hdrl parameter defining the final output grid

 The parameter controlling the output grid is
 - hdrl_resample_parameter_create_outgrid2D(),  or
 - hdrl_resample_parameter_create_outgrid3D(),  or
 - hdrl_resample_parameter_create_outgrid2D_userdef(),  or
 - hdrl_resample_parameter_create_outgrid3D_userdef()

 depending on a 2D or 3D interpolation and if the user wants to specify the
 final output grid on a high granularity or the hdrl routine should derive it
 from the input data.

 */
/*----------------------------------------------------------------------------*/

/** @cond PRIVATE */

/*-----------------------------------------------------------------------------
                        RESAMPLE Parameter Definition
 -----------------------------------------------------------------------------*/

typedef struct {
  HDRL_PARAMETER_HEAD;
  hdrl_resample_outgrid method;
  double              delta_ra ;     /* step size in right ascension [deg] */
  double              delta_dec ;    /* step size in declination [deg] */
  double              delta_lambda ; /* step size in wavelength direction [m] */
  const cpl_wcs*      wcs ;          /* World Coordinate System */
  cpl_boolean         recalc_limits;
  double              ra_min;        /* Minimal Right ascension [deg] */
  double              ra_max;        /* Maximal Right ascension [deg] */
  double              dec_min;       /* Minimal Declination [deg] */
  double              dec_max;       /* Maximal Declination [deg] */
  double              lambda_min;    /* Minimal wavelength [m] */
  double              lambda_max;    /* Maximal wavelength [m] */
  double              fieldmargin;   /* Field margin to add [percent] */
} hdrl_resample_outgrid_parameter;

/* Parameter type */
static hdrl_parameter_typeobj hdrl_resample_outgrid_parameter_type = {
    HDRL_PARAMETER_RESAMPLE_OUTGRID,          /* type */
    (hdrl_alloc *)&cpl_malloc,                /* fp_alloc */
    (hdrl_free *)&cpl_free,                   /* fp_free */
    NULL,                                     /* fp_destroy */
    sizeof(hdrl_resample_outgrid_parameter),  /* obj_size */
};

typedef struct {
  HDRL_PARAMETER_HEAD;
  hdrl_resample_method method;
  int loop_distance;
  /** When interpolating use additional weights of 1/variance */
  cpl_boolean use_errorweights;
  /** the pixfrac parameter of the drizzle method: down-scaling factor  *
   *  of input pixel size before computing drizzling weights; different *
   *  values for x-, y-, and lambda directions are possible             */
  double drizzle_pix_frac_x;
  double drizzle_pix_frac_y;
  double drizzle_pix_frac_lambda;
  /** critical radius of the Renka-weighted method */
  double renka_critical_radius;
  /** kernel size of the lanczos-weighted method */
  int lanczos_kernel_size;
} hdrl_resample_method_parameter;

/* Parameter type */
static hdrl_parameter_typeobj hdrl_resample_method_parameter_type = {
    HDRL_PARAMETER_RESAMPLE_METHOD,           /* type */
    (hdrl_alloc *)&cpl_malloc,                /* fp_alloc */
    (hdrl_free *)&cpl_free,                   /* fp_free */
    NULL,                                     /* fp_destroy */
    sizeof(hdrl_resample_method_parameter),   /* obj_size */
};

/** @endcond */
/*-----------------------------------------------------------------------------
                            Functions
 -----------------------------------------------------------------------------*/

static hdrl_resample_result *
hdrl_resample_cube(const cpl_table *ResTable,
		     hdrl_resample_method_parameter *aParams_method,
		     hdrl_resample_outgrid_parameter *aParams_outputgrid,
		     hdrl_resample_pixgrid **aGrid);

static cpl_error_code
hdrl_resample_inputtable_verify(const cpl_table *ResTable);

/*----------------------------------------------------------------------------*/
/**
 @brief  print content of the wcs structure
 @param  wcs cpl_wcs to be printed
 @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/

static cpl_error_code
hdrl_resample_wcs_print(const cpl_wcs *wcs)
{
  cpl_ensure_code(wcs, CPL_ERROR_NULL_INPUT);

  const cpl_array *crval = cpl_wcs_get_crval(wcs);
  const cpl_array *crpix = cpl_wcs_get_crpix(wcs);
  const cpl_array *ctype = cpl_wcs_get_ctype(wcs);
  const cpl_array *cunit = cpl_wcs_get_cunit(wcs);

  const cpl_matrix *cd = cpl_wcs_get_cd(wcs);
  const cpl_array *dims = cpl_wcs_get_image_dims(wcs);
  int naxis = cpl_wcs_get_image_naxis(wcs);

  cpl_msg_debug(cpl_func, "NAXIS:  %d", naxis);

  int testerr = 0;

  cpl_msg_indent_more();
  /* Check NAXIS */
  for (cpl_size i = 0; i < naxis; i++) {
      cpl_msg_debug(cpl_func, "NAXIS%lld: %d", i + 1,
		   cpl_array_get_int(dims, i, &testerr));
  }
  cpl_msg_indent_less();

  double cd11 = cpl_matrix_get(cd, 0, 0);
  double cd12 = cpl_matrix_get(cd, 0, 1);
  double cd21 = cpl_matrix_get(cd, 1, 0);
  double cd22 = cpl_matrix_get(cd, 1, 1);
  double crpix1 = cpl_array_get_double(crpix, 0, &testerr);
  double crpix2 = cpl_array_get_double(crpix, 1, &testerr);
  double crval1 = cpl_array_get_double(crval, 0, &testerr);
  double crval2 = cpl_array_get_double(crval, 1, &testerr);

  cpl_msg_debug(cpl_func, "1st and 2nd dimension");
  cpl_msg_indent_more();
  cpl_msg_debug(cpl_func, "CD1_1:  %g", cd11);
  cpl_msg_debug(cpl_func, "CD1_2:  %g", cd12);
  cpl_msg_debug(cpl_func, "CD2_1:  %g", cd21);
  cpl_msg_debug(cpl_func, "CD2_2:  %g", cd22);

  cpl_msg_debug(cpl_func, "CRPIX1: %g", crpix1);
  cpl_msg_debug(cpl_func, "CRPIX2: %g", crpix2);
  cpl_msg_debug(cpl_func, "CRVAL1: %g", crval1);
  cpl_msg_debug(cpl_func, "CRVAL2: %g", crval2);
  if (ctype) {
      cpl_msg_debug(cpl_func, "CTYPE1: %s", cpl_array_get_string(ctype, 0));
      cpl_msg_debug(cpl_func, "CTYPE2: %s", cpl_array_get_string(ctype, 1));
  }

  if (cunit) {
      cpl_msg_debug(cpl_func, "CUNIT1: %s", cpl_array_get_string(cunit, 0));
      cpl_msg_debug(cpl_func, "CUNIT2: %s", cpl_array_get_string(cunit, 1));
  }
  cpl_msg_indent_less();

  /* Is it a 3D cube or a 2D image */
  cpl_size cdncol = cpl_matrix_get_ncol(cd);
  if (cdncol == 3) {

      double cd13 = cpl_matrix_get(cd, 0, 2);
      double cd23 = cpl_matrix_get(cd, 1, 2);
      double cd31 = cpl_matrix_get(cd, 2, 0);
      double cd32 = cpl_matrix_get(cd, 2, 1);
      double cd33 = cpl_matrix_get(cd, 2, 2);
      double crval3 = cpl_array_get_double(crval, 2, &testerr);
      double crpix3 = cpl_array_get_double(crpix, 2, &testerr);

      cpl_msg_debug(cpl_func, "3rd dimension");
      cpl_msg_indent_more();
      cpl_msg_debug(cpl_func, "CD1_3:  %g", cd13);
      cpl_msg_debug(cpl_func, "CD2_3:  %g", cd23);
      cpl_msg_debug(cpl_func, "CD3_1:  %g", cd31);
      cpl_msg_debug(cpl_func, "CD3_2:  %g", cd32);
      cpl_msg_debug(cpl_func, "CD3_3:  %g", cd33);

      cpl_msg_debug(cpl_func, "CRPIX3: %g", crpix3);
      cpl_msg_debug(cpl_func, "CRVAL3: %g", crval3);

      if (ctype) {
          cpl_msg_debug(cpl_func, "CTYPE3: %s", cpl_array_get_string(ctype, 2));
      }

      if (cunit) {
          cpl_msg_debug(cpl_func, "CUNIT3: %s", cpl_array_get_string(cunit, 2));
      }

      cpl_msg_indent_less();
  }

  return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
 @brief  print content of the outgrid parameter structure
 @param  p outgrid parameter structure to be printed
 @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
hdrl_resample_outgrid_parameter_print(hdrl_resample_outgrid_parameter* p)
{

  cpl_ensure_code(p, CPL_ERROR_NULL_INPUT);
  /* Content of the outgrid parameter structure */

  cpl_msg_indent_more();
  cpl_msg_debug(cpl_func, "delta_ra:       %g", p->delta_ra);
  cpl_msg_debug(cpl_func, "delta_dec:      %g", p->delta_dec);
  cpl_msg_debug(cpl_func, "delta_lambda:   %g", p->delta_lambda);
  cpl_msg_debug(cpl_func, "ra_min:         %g", p->ra_min);
  cpl_msg_debug(cpl_func, "ra_max:         %g", p->ra_max);
  cpl_msg_debug(cpl_func, "dec_min:        %g", p->dec_min);
  cpl_msg_debug(cpl_func, "dec_max:        %g", p->dec_max);
  cpl_msg_debug(cpl_func, "lambda_min:     %g", p->lambda_min);
  cpl_msg_debug(cpl_func, "lambda_max:     %g", p->lambda_max);
  cpl_msg_debug(cpl_func, "fieldmargin:    %g", p->fieldmargin);
  cpl_msg_debug(cpl_func, "recalc_limits:  %d", p->recalc_limits);

  /* World Coordinate System */
  hdrl_resample_wcs_print(p->wcs);
  cpl_msg_indent_less();
  return cpl_error_get_code();

}
/*----------------------------------------------------------------------------*/
/**
 @brief  print content of the resample method parameter structure
 @param  p resample method parameter structure to be printed
 @return cpl_error_code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
hdrl_resample_method_parameter_print(hdrl_resample_method_parameter* p)
{
    if (hdrl_resample_parameter_method_verify((hdrl_parameter*)p) != CPL_ERROR_NONE) {
        return cpl_error_get_code();
    }

    /* Content of the outgrid parameter structure */

    cpl_msg_indent_more();
    if(p->method == HDRL_RESAMPLE_METHOD_NEAREST) {
        cpl_msg_debug(cpl_func, "method:                   %s", "NEAREST" );
    } else if (p->method == HDRL_RESAMPLE_METHOD_RENKA) {
        cpl_msg_debug(cpl_func, "method:                   %s", "RENKA" );
        cpl_msg_debug(cpl_func, "loop_distance:            %d", p->loop_distance);
        cpl_msg_debug(cpl_func, "use_errorweights:         %s",
                     p->use_errorweights == CPL_TRUE ? "TRUE" : "FALSE");
        cpl_msg_debug(cpl_func, "renka_critical_radius:    %g", p->renka_critical_radius);
    } else if (p->method == HDRL_RESAMPLE_METHOD_LINEAR) {
        cpl_msg_debug(cpl_func, "method:                   %s", "LINEAR" );
        cpl_msg_debug(cpl_func, "loop_distance:            %d", p->loop_distance);
        cpl_msg_debug(cpl_func, "use_errorweights:         %s",
                     p->use_errorweights == CPL_TRUE ? "TRUE" : "FALSE");
    } else if (p->method == HDRL_RESAMPLE_METHOD_QUADRATIC) {
        cpl_msg_debug(cpl_func, "method:                   %s", "QUADRATIC)" );
        cpl_msg_debug(cpl_func, "loop_distance:            %d", p->loop_distance);
        cpl_msg_debug(cpl_func, "use_errorweights:         %s",
                     p->use_errorweights == CPL_TRUE ? "TRUE" : "FALSE");
    } else if (p->method == HDRL_RESAMPLE_METHOD_DRIZZLE) {
        cpl_msg_debug(cpl_func, "method:                   %s", "DRIZZLE" );
        cpl_msg_debug(cpl_func, "loop_distance:            %d", p->loop_distance);
        cpl_msg_debug(cpl_func, "use_errorweights:         %s",
                     p->use_errorweights == CPL_TRUE ? "TRUE" : "FALSE");
        cpl_msg_debug(cpl_func, "drizzle_pix_frac_x:       %g", p->drizzle_pix_frac_x);
        cpl_msg_debug(cpl_func, "drizzle_pix_frac_y:       %g", p->drizzle_pix_frac_y);
        cpl_msg_debug(cpl_func, "drizzle_pix_frac_lambda:  %g", p->drizzle_pix_frac_lambda);
    } else if (p->method == HDRL_RESAMPLE_METHOD_LANCZOS) {
        cpl_msg_debug(cpl_func, "method:                   %s", "LANCZOS" );
        cpl_msg_debug(cpl_func, "loop_distance:            %d", p->loop_distance);
        cpl_msg_debug(cpl_func, "use_errorweights:         %s",
                     p->use_errorweights == CPL_TRUE ? "TRUE" : "FALSE");
        cpl_msg_debug(cpl_func, "lanczos_kernel_size:      %d", p->lanczos_kernel_size);
    }

    cpl_msg_indent_less();
    return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
  @private
  @brief    A 2D WCS structure is used to convert input x,y coordinates to
            equatorial coordinates.

  @param    wcs    Input WCS structure
  @param    x      Input X position
  @param    y      Input Y position
  @param    ra     Output right ascension (RA)
  @param    dec    Output declination (DEC)
  @return   cpl_error_code
 */
/*----------------------------------------------------------------------------*/
cpl_error_code
hdrl_wcs_xy_to_radec(const cpl_wcs *wcs, double x, double y, double *ra,
		 double *dec) {
  cpl_ensure_code(wcs && ra && dec, CPL_ERROR_NULL_INPUT);
  double * xy = NULL;
  double * radec = NULL;
  cpl_matrix * from = NULL;
  cpl_matrix * to = NULL;
  cpl_array * status = NULL;

  /* Load up the information */
  int naxis = cpl_wcs_get_image_naxis(wcs);
  from = cpl_matrix_new(1, naxis);
  xy = cpl_matrix_get_data(from);
  xy[0] = x;
  xy[1] = y;

  /* Call the conversion routine */

  cpl_wcs_convert(wcs, from, &to, &status, CPL_WCS_PHYS2WORLD);

  /* Pass it back now */

  radec = cpl_matrix_get_data(to);
  *ra = radec[0];
  *dec = radec[1];

  /* Tidy and exit */

  cpl_matrix_delete(from);
  cpl_matrix_delete(to);
  cpl_array_delete(status);
  return cpl_error_get_code();
}


/*----------------------------------------------------------------------------*/
/**
  @brief    find out the WCS reference point
  @param    aHeaders       property list/headers to read from
  @param    aAxis          the axis (the "i" of CRPIXi)
  @return   the requested value or 0.0 on error

  Queries FITS header CRPIXi
 */
/*----------------------------------------------------------------------------*/
static double
hdrl_resample_pfits_get_crpix(const cpl_propertylist *aHeaders, unsigned int aAxis)
{
  cpl_errorstate prestate = cpl_errorstate_get();
  cpl_ensure(aHeaders, CPL_ERROR_NULL_INPUT, 0);

  char keyword[KEYWORD_LENGTH];
  snprintf(keyword, KEYWORD_LENGTH, "CRPIX%u", aAxis);
  const double value = cpl_propertylist_get_double(aHeaders, keyword);
  /* default to 0.0 as per FITS Standard v3.0 */
  cpl_ensure(cpl_errorstate_is_equal(prestate), cpl_error_get_code(), 0.0);
  return value;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    find out the WCS coordinate at the reference point
  @param    aHeaders       property list/headers to read from
  @param    aAxis          the axis (the "i" of CRVALi)
  @return   the requested value or 0.0 on error

  Queries FITS header CRVALi
 */
/*----------------------------------------------------------------------------*/
static double
hdrl_resample_pfits_get_crval(const cpl_propertylist *aHeaders, unsigned int aAxis)
{
  cpl_errorstate prestate = cpl_errorstate_get();
  cpl_ensure(aHeaders, CPL_ERROR_NULL_INPUT, 0);

  char keyword[KEYWORD_LENGTH];
  snprintf(keyword, KEYWORD_LENGTH, "CRVAL%u", aAxis);
  const double value = cpl_propertylist_get_double(aHeaders, keyword);
  /* default to 0.0 as per FITS Standard v3.0 */
  cpl_ensure(cpl_errorstate_is_equal(prestate), cpl_error_get_code(), 0.0);
  return value;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    find out the WCS coordinate at the reference point
  @param    aHeaders       property list/headers to read from
  @param    aAxisI         the first axis (the "i" of CDi_j)
  @param    aAxisJ         the second axis (the "j" of CDi_j)
  @return   the requested value or 0.0 on error

  Queries FITS header CDi_j
 */
/*----------------------------------------------------------------------------*/
static double
hdrl_resample_pfits_get_cd(const cpl_propertylist *aHeaders, unsigned int aAxisI,
		  unsigned int aAxisJ)
{
  cpl_errorstate prestate = cpl_errorstate_get();
  cpl_ensure(aHeaders, CPL_ERROR_NULL_INPUT, 0);

  char keyword[KEYWORD_LENGTH];
  snprintf(keyword, KEYWORD_LENGTH, "CD%u_%u", aAxisI, aAxisJ);
  const double value = cpl_propertylist_get_double(aHeaders, keyword);
  /* default to 0.0 as per FITS Standard v3.0 */
  cpl_ensure(cpl_errorstate_is_equal(prestate), cpl_error_get_code(), 0.0);
  return value;
}

/*----------------------------------------------------------------------------*/
/**
  @brief   Create a new WCS structure from a given FITS header.
  @param   aHeader   the input header containing the input WCS
  @return  A newly allocated hdrl_resample_smallwcs *.

  The world coordinate system from aHeader, i.e. the CDi_j matrix, the CRPIXi,
  CRVALi, are used to fill the structure.

  The returned pointer has to be deallocated using cpl_free().

  @error{return the structure initialized with zeros\,
         except cd11 = cd22 = cddet = 1.,
         aHeader is NULL or does not contain the needed values}
  @error{set CPL_ERROR_SINGULAR_MATRIX\, but continue normally,
         the determinant of the CDi_j matrix (the cddet element) is zero}
 */
/*----------------------------------------------------------------------------*/
static hdrl_resample_smallwcs *
hdrl_resample_smallwcs_new(cpl_propertylist *aHeader)
{

  cpl_ensure(aHeader, CPL_ERROR_NULL_INPUT, NULL);
  hdrl_resample_smallwcs *wcs = cpl_calloc(1, sizeof(hdrl_resample_smallwcs));

  cpl_errorstate prestate = cpl_errorstate_get();
  wcs->crpix1 = hdrl_resample_pfits_get_crpix(aHeader, 1);
  wcs->crpix2 = hdrl_resample_pfits_get_crpix(aHeader, 2);
  wcs->crval1 = hdrl_resample_pfits_get_crval(aHeader, 1);
  wcs->crval2 = hdrl_resample_pfits_get_crval(aHeader, 2);
  if (!cpl_errorstate_is_equal(prestate)) {
      /* all these headers default to 0.0 following the FITS *
       * Standard, so we can ignore any errors set here      */
      cpl_errorstate_set(prestate);
  }

  prestate = cpl_errorstate_get();
  wcs->cd11 = hdrl_resample_pfits_get_cd(aHeader, 1, 1);
  wcs->cd22 = hdrl_resample_pfits_get_cd(aHeader, 2, 2);
  wcs->cd12 = hdrl_resample_pfits_get_cd(aHeader, 1, 2);
  wcs->cd21 = hdrl_resample_pfits_get_cd(aHeader, 2, 1);
  if (!cpl_errorstate_is_equal(prestate) &&
      wcs->cd11 == 0. && wcs->cd12 == 0. && wcs->cd21 == 0. && wcs->cd22 == 0.) {
      /* FITS Standard says to handle the CD matrix like the PC *
       * matrix in this case, with 1 for the diagonal elements  */
      wcs->cd11 = wcs->cd22 = wcs->cddet = 1.;
      cpl_errorstate_set(prestate); /* not a real error */
  }
  wcs->cddet = wcs->cd11 * wcs->cd22 - wcs->cd12 * wcs->cd21;
  if (wcs->cddet == 0.) {
      cpl_error_set(cpl_func, CPL_ERROR_SINGULAR_MATRIX);
  }

  return wcs;
} /* hdrl_resample_smallwcs_new() */

/*---------------------------------------------------------------------------*/
/**
  @brief   Delete a pixel grid and remove its memory.
  @param   aGrid   Pointer to pixel grid.
  @return void
 */
/*---------------------------------------------------------------------------*/
static void
hdrl_resample_pixgrid_delete(hdrl_resample_pixgrid *aGrid)
{
  if (!aGrid) {
      return;
  }
  cpl_free(aGrid->pix);
  aGrid->pix = NULL;
  /* clean up extension maps */
  unsigned short ix;
  for (ix = 0; ix < aGrid->nmaps; ix++) {
      cpl_size iext; /* index in this extension map */
      for (iext = 0; iext < aGrid->nxalloc[ix]; iext++) {
	  cpl_free(aGrid->xmaps[ix][iext].pix);
      } /* for iext (all allocated pixels in this extension map) */

      cpl_free(aGrid->xmaps[ix]);
  } /* for ix (all extension maps) */
  cpl_free(aGrid->xmaps);
  aGrid->xmaps = NULL;
  cpl_free(aGrid->nxmap);
  aGrid->nxmap = NULL;
  cpl_free(aGrid->nxalloc);
  aGrid->nxalloc = NULL;
  cpl_free(aGrid);
} /* hdrl_resample_pixgrid_delete() */


/*---------------------------------------------------------------------------*/
/**
  @brief   High level resampling function.

  @param   ResTable    The cpl table to be resampled. Should be derived by the
                       hdrl_resample_imagelist_to_table() or by the
                       hdrl_resample_image_to_table() function.
  @param   method      The hdrl parameter defining the resampling method
  @param   outputgrid  The hdrl parameter defining the output grid
  @param   wcs         The cpl wcs parameter do derive scalings/normalisations
  @return  The hdrl_resample_result structure containing all informations of the
           resampled output (data, error, bpm, wcs encoded in a
           cpl_propertylist). The returned hdrl_resample_result structure must
           be deleted with hdrl_resample_result_delete()


 The routine currently implements the following algorithms:
  - Nearest neighbor resampling
  - Weighted resampling using Renka weighting function
  - Weighted resampling using inverse distance weighting function
  - Weighted resampling using quadratic inverse distance weighting function
  - Weighted resampling using a drizzle-like weighting scheme
  - Weighted resampling using a lanczos-like restricted sinc for weighting

 The 2D and 3D interpolation is done in 2 dimension and 3 dimension,
 respectively. Moreover, additional error based weights can be taken into
 account.

 As can be seen from the interface, this function does not directly work on
 images but on the cpl table ResTable. The table is created from a 2D image by
 calling the function hdrl_resample_image_to_table() or from a 3D cube by
 calling the function hdrl_resample_imagelist_to_table(). In case you create the
 table on your own without the above mentioned functions, make sure to flag
 all(!) non normal pixels as bad (e.g. by using the isfinite() function).

 The parameter controlling the interpolation method (method) is created with
 - hdrl_resample_parameter_create_nearest(),  or
 - hdrl_resample_parameter_create_renka(),  or
 - hdrl_resample_parameter_create_linear(),  or
 - hdrl_resample_parameter_create_quadratic(),  or
 - hdrl_resample_parameter_create_drizzle(),  or
 - hdrl_resample_parameter_create_lanczos()

 depending on the interpolation method one would like to use.

 The parameter controlling the output grid (outputgrid) is created with
 - hdrl_resample_parameter_create_outgrid2D(),  or
 - hdrl_resample_parameter_create_outgrid3D(),  or
 - hdrl_resample_parameter_create_outgrid2D_userdef(),  or
 - hdrl_resample_parameter_create_outgrid3D_userdef()

 depending on a 2D or 3D interpolation and if the user wants to specify the
 final output grid on a high granularity or the hdrl routine should derive it
 from the input data.

  */
/*---------------------------------------------------------------------------*/
hdrl_resample_result *
hdrl_resample_compute(const cpl_table *ResTable,
		      hdrl_parameter *method,
		      hdrl_parameter *outputgrid,
		      const cpl_wcs* wcs)
{
  cpl_ensure(ResTable && method, CPL_ERROR_NULL_INPUT, NULL);
  cpl_ensure(wcs != NULL, CPL_ERROR_NULL_INPUT, NULL);

  if (hdrl_resample_inputtable_verify(ResTable)) {
        return NULL;
  }
  if (hdrl_resample_parameter_method_verify((hdrl_parameter *)method)) {
      return NULL;
  }
  if (hdrl_resample_parameter_outgrid_verify((hdrl_parameter *)outputgrid)) {
      return NULL;
  }

  if (hdrl_resample_inputtable_verify(ResTable)) {
      return NULL;
  }

  hdrl_resample_outgrid_parameter *aParams_outputgrid =
      (hdrl_resample_outgrid_parameter *) outputgrid;
  hdrl_resample_method_parameter *aParams_method =
      (hdrl_resample_method_parameter *) method;

  /*Assign the wcs*/
  aParams_outputgrid->wcs = wcs;

  /* Recalculate the limits if the user did not specify any */
  cpl_boolean recalc_limits = aParams_outputgrid->recalc_limits;

  if (recalc_limits == CPL_TRUE) {
      double ramin =  cpl_table_get_column_min(ResTable, HDRL_RESAMPLE_TABLE_RA);
      double ramax =  cpl_table_get_column_max(ResTable, HDRL_RESAMPLE_TABLE_RA);
      double decmin = cpl_table_get_column_min(ResTable, HDRL_RESAMPLE_TABLE_DEC);
      double decmax = cpl_table_get_column_max(ResTable, HDRL_RESAMPLE_TABLE_DEC);
      double lmin =   cpl_table_get_column_min(ResTable, HDRL_RESAMPLE_TABLE_LAMBDA);
      double lmax =   cpl_table_get_column_max(ResTable, HDRL_RESAMPLE_TABLE_LAMBDA);

      /* We have the rare case that the image spans over ra = 0.*/
      if(ramax - ramin > 180){
	  const double *ra = cpl_table_get_data_double_const(ResTable,
							     HDRL_RESAMPLE_TABLE_RA);

	  /* set to extreme values for a start */
	  ramin = 0.;
	  ramax = 360.;
	  cpl_size nrow = cpl_table_get_nrow(ResTable);

	  for (cpl_size i = 0; i < nrow; i++) {
	      if (ra[i] > ramin && ra[i] <= 180.) ramin = ra[i]; /* get the maximum */
	      if (ra[i] < ramax && ra[i] >  180.) ramax = ra[i]; /* get the minimum */
	  }
      }

      aParams_outputgrid->ra_min = ramin;
      aParams_outputgrid->ra_max = ramax;
      aParams_outputgrid->dec_min = decmin;
      aParams_outputgrid->dec_max = decmax;
      aParams_outputgrid->lambda_min = lmin;
      aParams_outputgrid->lambda_max = lmax;
  }

  cpl_msg_debug(cpl_func, "Content of the outgrid parameter structure "
      "hdrl_resample_outgrid_parameter when resampling starts:");
  hdrl_resample_outgrid_parameter_print(aParams_outputgrid);

  cpl_msg_debug(cpl_func, "Content of the method parameter structure "
      "hdrl_resample_method_parameter when resampling starts:");
  hdrl_resample_method_parameter_print(aParams_method);

  /* create cube and cast to generic pointer to save code duplication */
  hdrl_resample_result *cube = NULL;
  hdrl_resample_pixgrid *grid = NULL;

  cpl_msg_debug(cpl_func, "Resampling starts ...");
  cpl_msg_indent_more();
  cube = hdrl_resample_cube(ResTable,
			      (hdrl_resample_method_parameter *)aParams_method,
			      (hdrl_resample_outgrid_parameter *)aParams_outputgrid,
			      &grid);
  cpl_msg_indent_less();
  cpl_ensure(cube, CPL_ERROR_NULL_INPUT, NULL);
  cpl_ensure(cube->header, CPL_ERROR_NULL_INPUT, NULL);
  cpl_ensure(cube->himlist, CPL_ERROR_NULL_INPUT, NULL);

  /* Cleanup wcs for 2D / 3D case */

  if(hdrl_imagelist_get_size(cube->himlist) == 1) { /* 2D case */
      cpl_propertylist *header = cpl_propertylist_new();
      cpl_wcs* wcs_local = cpl_wcs_new_from_propertylist(cube->header);
      hdrl_wcs_to_propertylist(wcs_local, header, CPL_TRUE);
      cpl_wcs_delete(wcs_local);
      cpl_propertylist_delete(cube->header);
      cpl_propertylist_set_comment(header, "CTYPE1", "Gnomonic projection");
      cpl_propertylist_set_comment(header, "CTYPE2", "Gnomonic projection");
      cube->header = header;
  }


  hdrl_resample_pixgrid_delete(grid);
  return cube;
} /* hdrl_resample_compute() */

/*---------------------------------------------------------------------------*/
/**
  @brief    Deallocates the memory associated to a hdrl_resample_result object.
  @param    aCube   input hdrl_resample_result object
  @return   void
 */
/*---------------------------------------------------------------------------*/
void
hdrl_resample_result_delete(hdrl_resample_result *aCube)
{
  /* if the cube does not exists at all, we don't need to do anything */
  if (!aCube) {
      return;
  }

  /* checks for the existence of the sub-images *
   * are done in the CPL functions              */

  hdrl_imagelist_delete(aCube->himlist);
  aCube->himlist = NULL;
  /* delete the FITS header, too */
  cpl_propertylist_delete(aCube->header);
  aCube->header = NULL;

  cpl_free(aCube);
} /* hdrl_resample_result_delete() */

/*----------------------------------------------------------------------------*/
/**
  @brief   Convert from celestial spherical coordinates to projection plane
           coordinates.
  @param   aParams_outputgrid   the input outgrid parameter containing the WCS
                      of the exposure
  @param   aRA       the right-ascension in degrees
  @param   aDEC      the declination in degrees
  @param   aX        the output horizontal proj. plane position
  @param   aY        the output vertical proj. plane position
  @return  CPL_ERROR_NONE for success, any other value for failure

  The world coordinate system from aHeader is used to do the transformation,
  only the gnomonic (TAN) is supported.

  Uses Eqns (5), (12), (13), and (54) from Calabretta & Greisen 2002 A&A 395,
  1077 (Paper II). We use that phi_p = 180 deg for zenithal projections (like
  TAN).

  XXX this duplicates most of the code of hdrl_resample_wcs_pixel_from_celestial()

  @error{return CPL_ERROR_NULL_INPUT, aHeader\, aX\, and/or aY are NULL}
  @error{return CPL_ERROR_UNSUPPORTED_MODE,
         aHeader does not contain gnomonic (TAN) WCS}
  @error{return CPL_ERROR_ILLEGAL_INPUT,
         determinant of the CDi_j matrix in aHeader is zero}
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
hdrl_resample_wcs_projplane_from_celestial(hdrl_resample_outgrid_parameter *
				  aParams_outputgrid, double aRA,
				  double aDEC, double *aX, double *aY)
{
  cpl_ensure_code(aParams_outputgrid && aX && aY, CPL_ERROR_NULL_INPUT);
  /* make sure that the header represents TAN */
  //const char * type1 = aParams_outputgrid->limits.ctype1;
  //const char * type2 = aParams_outputgrid->limits.ctype2;
  /*
  cpl_ensure_code(type1 && type2 && !strncmp(type1, "RA---TAN", 9) &&
                  !strncmp(type2, "DEC--TAN", 9),
                  CPL_ERROR_UNSUPPORTED_MODE);
   */

  int err = 0;
  const cpl_array *crval = cpl_wcs_get_crval(aParams_outputgrid->wcs);
  double crval1 = cpl_array_get_double(crval, 0, &err);
  double crval2 = cpl_array_get_double(crval, 1, &err);

  /* spherical coordinate shift/translation */
  double a = aRA / CPL_MATH_DEG_RAD,  /* RA in radians */
      d = aDEC / CPL_MATH_DEG_RAD, /* DEC in radians */
      /* alpha_p and delta_p in Paper II (in radians) */
      //ap = aParams_outputgrid->limits.crval1 / CPL_MATH_DEG_RAD,
      //dp = aParams_outputgrid->limits.crval2 / CPL_MATH_DEG_RAD,
      ap = crval1 / CPL_MATH_DEG_RAD,
      dp = crval2 / CPL_MATH_DEG_RAD,
      phi = atan2(-cos(d) * sin(a - ap),
		  sin(d) * cos(dp) - cos(d) * sin(dp) * cos(a-ap))
		  + 180 / CPL_MATH_DEG_RAD,
		  theta = asin(sin(d) * sin(dp) + cos(d) * cos(dp) * cos(a-ap)),
		  R_theta = CPL_MATH_DEG_RAD / tan(theta);
  /* spherical deprojection */
  *aX = R_theta * sin(phi),
      *aY = -R_theta * cos(phi);

  return CPL_ERROR_NONE;
} /* hdrl_resample_wcs_projplane_from_celestial() */

/*---------------------------------------------------------------------------*/
/**
  @brief   Convert from celestial spherical coordinates to pixel coordinates.
  @param   aWCS   the input WCS structure,
                  <b>with the crpix components in radians!</b>
  @param   aRA    the right-ascension <b>in radians!</b>
  @param   aDEC   the declination <b>in radians!</b>
  @param   aX     the output horizontal pixel position
  @param   aY     the output vertical pixel position

  @warning This function does not do any safety checks. If you are not sure, if
           the WCS does not contain a proper gnomonic (TAN) setup, use
           hdrl_resample_wcs_pixel_from_celestial() instead!
  @note This function is defined as static inline, i.e. should get integrated
        into the caller by the compiler, to maximise execution speed.

  See hdrl_resample_wcs_pixel_from_celestial() for the origin of the equations.
 */
/*---------------------------------------------------------------------------*/
static inline void
hdrl_resample_wcs_pixel_from_celestial_fast(hdrl_resample_smallwcs *aWCS,
		double aRA, double aDEC, double *aX, double *aY)
{
  if(!aWCS) return;
  /* spherical coordinate shift/translation */
  /*Calabretta & Greisen 2002 A&A 395, 1077 (Paper II) */
  /* aRA is alpha in Paper II, aDEC is delta, aWCS->crval1 is alpha_p, *
   * aWCS->crval2 is delta_p, all of them in units of radians, eq (5), arg=atan2        */

  double phi = atan2(-cos(aDEC) * sin(aRA - aWCS->crval1),
		     sin(aDEC) * cos(aWCS->crval2)
		     - cos(aDEC) * sin(aWCS->crval2) * cos(aRA - aWCS->crval1))
        	     + 180 / CPL_MATH_DEG_RAD,
		     theta = asin(sin(aDEC) * sin(aWCS->crval2)
				  + cos(aDEC) * cos(aWCS->crval2) * cos(aRA - aWCS->crval1)),
				  R_theta = CPL_MATH_DEG_RAD / tan(theta);
  /* spherical deprojection */
  double x = R_theta * sin(phi),
      y = -R_theta * cos(phi);
  /* inverse linear transformation */
  *aX = (aWCS->cd22 * x - aWCS->cd12 * y) / aWCS->cddet + aWCS->crpix1;
  *aY = (aWCS->cd11 * y - aWCS->cd21 * x) / aWCS->cddet + aWCS->crpix2;
} /* hdrl_resample_wcs_pixel_from_celestial_fast() */


/*---------------------------------------------------------------------------*/
/**
  @private
  @brief   Compute "natural" cube size from the data.
  @param   aParams_outputgrid     the outputgrid parameters
  @param   aX          pointer to horizontal cube size to update
  @param   aY          pointer to vertical cube size to update
  @param   aZ          pointer to dispersion direction cube size to update
  @return  CPL_ERROR_NONE on non-critical failure or success, another CPL error
           code otherwise.
  @error{return CPL_ERROR_NULL_INPUT,
         ResTable\, aParams\, aX\, aY\, or aZ are NULL}
 */
/*---------------------------------------------------------------------------*/
static cpl_error_code
hdrl_resample_compute_size(hdrl_resample_outgrid_parameter *aParams_outputgrid,
			     int *aX, int *aY, int *aZ)
{
  cpl_ensure_code(aParams_outputgrid && aX && aY && aZ, CPL_ERROR_NULL_INPUT);
  const char func[] = "hdrl_resample_compute_size"; /* pretend to be in that fct... */
  double x1, y1, x2, y2;

  double ramin = aParams_outputgrid->ra_min;
  double ramax = aParams_outputgrid->ra_max;
  double decmin = aParams_outputgrid->dec_min;
  double decmax = aParams_outputgrid->dec_max;
  hdrl_resample_wcs_projplane_from_celestial(aParams_outputgrid, ramin, decmin,
		  &x1, &y1);
  hdrl_resample_wcs_projplane_from_celestial(aParams_outputgrid, ramax, decmax,
		  &x2, &y2);
  *aX = lround(fabs(x2 - x1) / aParams_outputgrid->delta_ra) + 1;
  *aY = lround(fabs(y2 - y1) / aParams_outputgrid->delta_dec) + 1;

  double lmin = aParams_outputgrid->lambda_min;
  double lmax = aParams_outputgrid->lambda_max;

  *aZ = (int)ceil((lmax - lmin) / aParams_outputgrid->delta_lambda) + 1;

  cpl_msg_debug(func, "Output cube size %d x %d x %d (fit to data)",
	       *aX, *aY, *aZ);
  return CPL_ERROR_NONE;
} /* hdrl_resample_compute_size() */

/*---------------------------------------------------------------------------*/
/**
  @private
  @brief   Add a table row to the pixel grid.
  @param   aGrid    Pointer to pixel grid.
  @param   aIndex   Pixel index, as computed by hdrl_resample_pixgrid_get_index().
  @param   aRow     Row number to be added.
  @param   aXIdx    Index of the extension map to use.

  This function adds a new entry into the grid, either directly in the grid or
  in the extension maps (aGrid->xmaps). To do the latter, it allocates space
  for storage in one extension map, doubling the amount of allocated memory
  every time an enlargement is needed. The number of real (filled) extension map
  entries is stored in aGrid->nxmap, while the current number of allocated
  entries is tracked with aGrid->nxalloc.
 */
/*---------------------------------------------------------------------------*/
static void
hdrl_resample_pixgrid_add(hdrl_resample_pixgrid *aGrid, cpl_size aIndex,
		cpl_size aRow, unsigned short aXIdx)
{

  if (aIndex < 0 || aGrid == NULL) {
      return;
  }

  if (aGrid->pix[aIndex] == 0 && aRow > 0) {
      /* First pixel is stored directly. */
      aGrid->pix[aIndex] = aRow;
  } else if (aGrid->pix[aIndex] == 0 && aRow == 0) {
      /* Special case: we cannot put "0" into the main map. */
      cpl_size iext = aGrid->nxmap[aXIdx]++;
      if (aGrid->nxmap[aXIdx] > aGrid->nxalloc[aXIdx]) {
	  /* double the number of allocated entries */
	  aGrid->nxalloc[aXIdx] = 2 * aGrid->nxmap[aXIdx];
	  aGrid->xmaps[aXIdx] = cpl_realloc(aGrid->xmaps[aXIdx],
					    aGrid->nxalloc[aXIdx]
							   * sizeof(hdrl_resample_pixels_ext));
      }
      aGrid->xmaps[aXIdx][iext].npix = 1;
      aGrid->xmaps[aXIdx][iext].pix = cpl_malloc(sizeof(cpl_size));
      aGrid->xmaps[aXIdx][iext].pix[0] = aRow;
      aGrid->pix[aIndex] = -(iext + 1 + ((cpl_size)aXIdx << XMAP_LSHIFT));
  } else if (aGrid->pix[aIndex] > 0) {
      /* When a second pixel is added, put both to the extension map. */
      cpl_size iext = aGrid->nxmap[aXIdx]++;
      if (aGrid->nxmap[aXIdx] > aGrid->nxalloc[aXIdx]) {
	  /* double the number of allocated entries */
	  aGrid->nxalloc[aXIdx] = 2 * aGrid->nxmap[aXIdx];
	  aGrid->xmaps[aXIdx] = cpl_realloc(aGrid->xmaps[aXIdx],
					    aGrid->nxalloc[aXIdx]
							   * sizeof(hdrl_resample_pixels_ext));
      }
      aGrid->xmaps[aXIdx][iext].npix = 2;
      aGrid->xmaps[aXIdx][iext].pix = cpl_malloc(2 * sizeof(cpl_size));
      aGrid->xmaps[aXIdx][iext].pix[0] = aGrid->pix[aIndex];
      aGrid->xmaps[aXIdx][iext].pix[1] = aRow;
      aGrid->pix[aIndex] = -(iext + 1 + ((cpl_size)aXIdx << XMAP_LSHIFT));
  } else {
      /* Append additional pixels to the extension map. */
      cpl_size iext = (-aGrid->pix[aIndex] - 1) & PT_IDX_MASK;
      /* index of the new entry in this grid point */
      unsigned int ipix = aGrid->xmaps[aXIdx][iext].npix;
      aGrid->xmaps[aXIdx][iext].npix++;
      aGrid->xmaps[aXIdx][iext].pix = cpl_realloc(aGrid->xmaps[aXIdx][iext].pix,
						  (ipix + 1) * sizeof(cpl_size));
      aGrid->xmaps[aXIdx][iext].pix[ipix] = aRow;
  }
} /* hdrl_resample_pixgrid_add() */

/*---------------------------------------------------------------------------*/
/**
  @brief   Return the number of rows stored in one pixel.
  @param   aGrid    Pointer to pixel grid.
  @param   aIndex   Pixel index, as computed by hdrl_resample_pixgrid_get_index().

  @note This function is defined as static inline, i.e. should get integrated
        into the caller by the compiler, to maximise execution speed.
 */
/*---------------------------------------------------------------------------*/
static inline cpl_size
hdrl_resample_pixgrid_get_count(hdrl_resample_pixgrid *aGrid, cpl_size aIndex)
{
  if (aIndex < 0 || aGrid == NULL) {
      return 0;
  }
  /* get entry in pixel grid */
  cpl_size p = aGrid->pix[aIndex];
  if (p == 0) { /* points nowhere --> no pixels */
      return 0;
  }
  if (p > 0) { /* points to pixel table --> 1 pixel */
      return 1;
  }
  /* p is negative, so points to an extension map, get its index */
  unsigned short ix = (-p >> XMAP_LSHIFT) & XMAP_BITMASK;
  p = (-p - 1) & PT_IDX_MASK;
  /* the npix component now gives the number of pixels in this grid index */
  return aGrid->xmaps[ix][p].npix;
} /* hdrl_resample_pixgrid_get_count() */

/*---------------------------------------------------------------------------*/
/**
  @brief   Get the grid index determined from all three coordinates.
  @param   aGrid           Pointer to pixel grid.
  @param   aX              x index
  @param   aY              y index
  @param   aZ              z index
  @param   aAllowOutside   if true, return a positive value for pixels nominally
                           outside the grid
  @return  Index to be used in the other pixel grid functions, or -1 on error.

  @note This function is defined as static inline, i.e. should get integrated
        into the caller by the compiler, to maximise execution speed.
 */
/*---------------------------------------------------------------------------*/
static inline cpl_size
hdrl_resample_pixgrid_get_index(hdrl_resample_pixgrid *aGrid, cpl_size aX,
		cpl_size aY, cpl_size aZ, cpl_boolean aAllowOutside)
{
  if (aGrid == NULL)  {
	  return -1;
  }
  if (!aAllowOutside &&
      (aX < 0 || aX >= aGrid->nx || aY < 0 || aY >= aGrid->ny ||
	  aZ < 0 || aZ >= aGrid->nz)) {
      return -1;
  }
  if (aX < 0) {
      aX = 0;
  }
  if (aX >= aGrid->nx) {
      aX = aGrid->nx - 1;
  }
  if (aY < 0) {
      aY = 0;
  }
  if (aY >= aGrid->ny) {
      aY = aGrid->ny - 1;
  }
  if (aZ < 0) {
      aZ = 0;
  }
  if (aZ >= aGrid->nz) {
      aZ = aGrid->nz - 1;
  }
  return aX + aGrid->nx * (aY + aGrid->ny * aZ);
} /* hdrl_resample_pixgrid_get_index() */

/*---------------------------------------------------------------------------*/
/**
  @private
  @brief   Create a new pixel grid.
  @param   aSizeX   X size of the grid.
  @param   aSizeY   Y size of the grid.
  @param   aSizeZ   Z size of the grid.
  @param   aNMaps   number of extensions maps.
  @return  Pointer to the newly created pixel grid.
 */
/*---------------------------------------------------------------------------*/
static hdrl_resample_pixgrid *
hdrl_resample_pixgrid_new(cpl_size aSizeX, cpl_size aSizeY, cpl_size aSizeZ,
		 unsigned short aNMaps)
{
  cpl_ensure(aSizeX > 0 && aSizeY > 0 && aSizeZ > 0 && aNMaps > 0,
		  CPL_ERROR_ILLEGAL_INPUT, NULL);
  hdrl_resample_pixgrid *pixels = cpl_calloc(1, sizeof(hdrl_resample_pixgrid));
  pixels->nx = aSizeX;
  pixels->ny = aSizeY;
  pixels->nz = aSizeZ;
  pixels->pix = cpl_calloc(aSizeX * aSizeY * aSizeZ, sizeof(cpl_size));
  /* extension maps for possibly multiple threads */
  pixels->nmaps = aNMaps;
  pixels->nxalloc = cpl_calloc(aNMaps, sizeof(cpl_size));
  pixels->xmaps = cpl_calloc(aNMaps, sizeof(hdrl_resample_pixels_ext *));
  pixels->nxmap = cpl_calloc(aNMaps, sizeof(cpl_size));
  return pixels;
} /* hdrl_resample_pixgrid_new() */

/*---------------------------------------------------------------------------*/
/**
  @brief   Convert selected rows of a pixel table into pixel grid, linking the
           grid points to entries (=rows) in the pixel table.
  @param   ResTable   the input pixel table
  @param   aHeader     the FITS header of the hdrl_resample datacube to be created
  @param   aXSize      x size of the output grid
  @param   aYSize      y size of the output grid
  @param   aZSize      z size of the output grid (wavelength direction)
  @return  A hdrl_resample_pixels * buffer for the output pixel grid or NULL on error.
  @remark  The returned pixel grid has to be deallocated after use with
           hdrl_resample_pixgrid_delete().

  Construct a standard C array, where the array indices representing the 3D grid
  in the sense the the x-coordinate is varying fastest, the lambda-coordinate
  varying slowest (like in FITS buffers), i.e.
    index = [i + nx * j + nx*ny * l]
    (i: x-axis index, j: y-axis index, l: lambda-axis index,
    nx: x-axis length, ny: y-axis length).
  For each pixel table row search for the closest grid point. Store the pixel
  table row number in that grid point.

  @error{set CPL_ERROR_NULL_INPUT\, return NULL,
         ResTable is NULL or it contains zero rows}
  @error{set CPL_ERROR_ILLEGAL_INPUT\, return NULL,
         one of the sizes is not positive}
  @error{set CPL_ERROR_UNSUPPORTED_MODE\, return NULL,
         the WCS in the pixel table is neither in pixels nor degrees}
  @error{set CPL_ERROR_DATA_NOT_FOUND\, return NULL,
         ResTable is missing one of the coordinate columns}
  @error{set CPL_ERROR_ILLEGAL_OUTPUT and ERROR message\, but return created grid,
         ResTable contains different number of pixels than the output pixel grid}
 */
/*---------------------------------------------------------------------------*/
static hdrl_resample_pixgrid *
hdrl_resample_pixgrid_create(const cpl_table *ResTable, cpl_propertylist *aHeader,
		    cpl_size aXSize, cpl_size aYSize, cpl_size aZSize)
{
  cpl_ensure(ResTable, CPL_ERROR_NULL_INPUT, NULL);
  cpl_ensure(aHeader, CPL_ERROR_NULL_INPUT, NULL);
  cpl_size nrow = cpl_table_get_nrow(ResTable);
  if (nrow == 0) {
      cpl_msg_error(cpl_func, "Invalid pixel table (no entries?)");
      cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
      return NULL;
  }
  cpl_ensure(aXSize > 0 && aYSize > 0 && aZSize > 0, CPL_ERROR_ILLEGAL_INPUT,
	     NULL);


  double crval3 = hdrl_resample_pfits_get_crval(aHeader, 3),
      crpix3 = hdrl_resample_pfits_get_crpix(aHeader, 3),
      cd33 = hdrl_resample_pfits_get_cd(aHeader, 3, 3);

  hdrl_resample_smallwcs *wcs = hdrl_resample_smallwcs_new(aHeader);

  /* get all (relevant) table columns for easy pointer access */

  const double *xpos = cpl_table_get_data_double_const(ResTable, HDRL_RESAMPLE_TABLE_RA),
      *ypos = cpl_table_get_data_double_const(ResTable, HDRL_RESAMPLE_TABLE_DEC),
      *lbda = cpl_table_get_data_double_const(ResTable, HDRL_RESAMPLE_TABLE_LAMBDA);
  if (!xpos || !ypos || !lbda) {
      cpl_msg_error(cpl_func, "Missing pixel table column (%p %p %p): %s",
		    (void *)xpos, (void *)ypos, (void *)lbda,
		    cpl_error_get_message());
      cpl_error_set(cpl_func, CPL_ERROR_DATA_NOT_FOUND);
      cpl_free(wcs);
      return NULL;
  }

  wcs->crval1 /= CPL_MATH_DEG_RAD; /* convert to radians before calling...    */
  wcs->crval2 /= CPL_MATH_DEG_RAD; /* ...hdrl_resample_wcs_pixel_from_celestial_fast() */

  double timeinit = cpl_test_get_walltime(),
      timeprogress = timeinit,
      cpuinit = cpl_test_get_cputime();
  cpl_boolean showprogress = cpl_msg_get_level() == CPL_MSG_DEBUG
      || cpl_msg_get_log_level() == CPL_MSG_DEBUG;

  /* check for the selected pixels in the pixel table, only those *
   * are used to construct the pixel grid; since constructing the *
   * array of selected pixels costs significant amounts of time,  *
   * do that only when not all pixels are selected!               */
  cpl_array *asel = NULL;
  const cpl_size *sel = NULL;
  cpl_size nsel = cpl_table_count_selected(ResTable);
  if (nsel < nrow) {
      asel = cpl_table_where_selected(ResTable);
      sel = cpl_array_get_data_cplsize_const(asel);
  }

  /* can use at most XMAP_BITMASK threads so that the bitmask does not       *
   * overflow, but ensure that we are not using more cores than available... */
  int nth = omp_get_max_threads() > XMAP_BITMASK ? XMAP_BITMASK
      : omp_get_max_threads();
  /* prepare the ranges for the different threads, store them in arrays */
  cpl_array *az1 = cpl_array_new(nth, CPL_TYPE_INT),
      *az2 = cpl_array_new(nth, CPL_TYPE_INT);
  if (aZSize < nth) {
      /* pre-fill arrays with values that cause the threads to do nothing */
      cpl_array_fill_window_int(az1, aZSize, nth, -1);
      cpl_array_fill_window_int(az2, aZSize, nth, -2);
  }
  /* now fill the (first) ones with real ranges */
  double base = nth > aZSize ? 1. : (double)aZSize / nth;
  cpl_size ith;
  for (ith = 0; ith < nth && ith < aZSize; ith++) {
      cpl_array_set_int(az1, ith, lround(base * ith));
      cpl_array_set_int(az2, ith, lround(base * (ith + 1) - 1));
  } /* for */
  /* make sure that we don't lose pixels at the edges of the wavelength      *
   * range, put them into the extreme threads by making their ranges larger; *
   * set the relevant array entries to something close to the largest value, *
   * that we can still add as an integer (to compute the z-range)            */
  cpl_array_set_int(az1, 0, -INT_MAX / 2 + 1);
  cpl_array_set_int(az2, ith - 1, INT_MAX / 2 - 1);

  /* create the pixel grid with extension maps for threads */
  hdrl_resample_pixgrid *grid = hdrl_resample_pixgrid_new(aXSize, aYSize, aZSize, nth);

  /* parallel region to fill the pixel grid */

  struct timeval tv1, tv2;

  cpl_msg_debug(cpl_func,"Starting parallel loop in hdrl_resample_pixgrid_create");
  gettimeofday(&tv1, NULL);

#pragma omp parallel num_threads(nth) /* default(none)    as req. by Ralf */ \
    shared(aXSize, aYSize, aZSize, az1, az2, cd33, crpix3, crval3, grid, \
	   lbda, nsel, sel, showprogress,     \
	   timeinit, timeprogress, wcs, xpos, ypos)

  {
    /* split the work up into threads, for non-overlapping wavelength ranges */
    unsigned short ithread = omp_get_thread_num(); /* index of this thread */
    int z1 = cpl_array_get_int(az1, ithread, NULL),
	z2 = cpl_array_get_int(az2, ithread, NULL),
	zrange = z2 - z1 + 1;

    /* check if we actually need to enter the (parallel) loop, i.e. *
     * if the current thread is handling any wavelength planes      */

    /* now the actual parallel loop */
    cpl_size isel;
    for (isel = 0 ; zrange > 0 && isel < nsel; isel++) {
#pragma omp master /* only output progress from the master thread */
	if (showprogress && !((isel+1) % 1000000ll)) { /* output before every millionth entry */
	    double timenow = cpl_test_get_walltime();
	    if (timenow - timeprogress > 30.) { /* and more than half a minute passed */
		timeprogress = timenow;
		double percent = 100. * (isel + 1.) / nsel,
		    elapsed = timeprogress - timeinit,
		    remaining = (100. - percent) * elapsed / percent;
		/* overwritable only exists for INFO mode, but we check  *
		 * above that we want this only for DEBUG mode output... */
		cpl_msg_info_overwritable(cpl_func, "pixel grid creation is %.1f%% "
					  "complete, %.1fs elapsed, ~%.1fs remaining",
					  percent, elapsed, remaining);
	    } /* if: 1/2 min passed */
	} /* if: want debug output */

	/* either use the index from the array of selected rows   *
	 * or the row numberdirectly (for a fully selected table) */
	cpl_size n = sel ? sel[isel] : isel;
	int z = -1;

	z = lround((lbda[n] - crval3) / cd33 + crpix3) - 1;

	if (z < z1 || z > z2) {
	    continue; /* skip this entry, one of the other threads handles it */
	}

	/* determine the pixel coordinates in the grid (indices, starting at 0) */
	double xpx = 0., ypx = 0.;

	hdrl_resample_wcs_pixel_from_celestial_fast(wcs, (xpos[n]) / CPL_MATH_DEG_RAD,
					   (ypos[n]) / CPL_MATH_DEG_RAD,
					   &xpx, &ypx);

	int x = lround(xpx) - 1,
	    y = lround(ypx) - 1;
	cpl_size idx = hdrl_resample_pixgrid_get_index(grid, x, y, z, CPL_TRUE);


	/* write the pixel values to the correct place in the grid */
	hdrl_resample_pixgrid_add(grid, idx, n, ithread);
    } /* for isel (all selected pixel table rows) */

    /* Clean up the possibly too many allocations; this is not strictly *
     * needed but nice to only consume as much memory as we need.       */
    grid->xmaps[ithread] = cpl_realloc(grid->xmaps[ithread],
				       grid->nxmap[ithread]
						   * sizeof(hdrl_resample_pixels_ext));
    grid->nxalloc[ithread] = grid->nxmap[ithread];
  } /* omp parallel */

  gettimeofday(&tv2, NULL);
  cpl_msg_debug(cpl_func, "Wall time for hdrl_resample_pixgrid_create was %f seconds\n",
	       (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
	       (double) (tv2.tv_sec - tv1.tv_sec));

  cpl_array_delete(asel);
  cpl_free(wcs);
  cpl_array_delete(az1);
  cpl_array_delete(az2);


  cpl_size idx, npix = 0;
  for (idx = 0; idx < aXSize * aYSize * aZSize; idx++) {
      npix += hdrl_resample_pixgrid_get_count(grid, idx);
  }
  cpl_size nxmap = 0;
  for (idx = 0; idx < (cpl_size)grid->nmaps; idx++) {
      nxmap += grid->nxmap[idx];
  }
  if (npix != nsel) {
      char *msg = cpl_sprintf("Pixels got lost while creating the cube (input "
	  "pixel table: %"CPL_SIZE_FORMAT", output pixel grid"
	  ": %"CPL_SIZE_FORMAT")", nsel, npix);
      cpl_msg_error(cpl_func, "%s:", msg);
      cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_OUTPUT, "%s!", msg);
      cpl_free(msg);
  }
  double timefini = cpl_test_get_walltime(),
      cpufini = cpl_test_get_cputime();
  cpl_msg_debug(cpl_func, "pixel grid: %dx%dx%d, %"CPL_SIZE_FORMAT" pixels "
		"total, %"CPL_SIZE_FORMAT" (%.1f%%) in %hu extension maps; took"
		" %gs (wall-clock) and %gs (CPU) to create", (int)grid->nx,
		(int)grid->ny, (int)grid->nz, npix, nxmap,
		(double)nxmap / npix * 100., grid->nmaps, timefini - timeinit,
		cpufini - cpuinit);

  return grid;
} /* hdrl_resample_pixgrid_create() */

/*----------------------------------------------------------------------------*/
/**
  @brief   Compute the spatial scales (in degrees) from the FITS header WCS.
  @param   aParams_outputgrid   the input outgrid parameter containing the WCS
                                of the exposure
  @param   aXScale   the output scale in x-direction
  @param   aYScale   the output scale in y-direction
  @return  CPL_ERROR_NONE for success, any other value for failure

  The world coordinate system from aHeader, i.e. the CDi_j matrix, is used to
  compute the scales.

  References:
  - based on public domain code of the IDL astro-lib procedure getrot.pro
  - http://idlastro.gsfc.nasa.gov/ftp/pro/astrom/getrot.pro

  @error{return CPL_ERROR_NULL_INPUT, aHeader\, aX\, and/or aY are NULL}
  @error{propagate the error of cpl_propertylist_get_double(),
         some elements of the CDi_j matrix don't exist in aHeader}
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
hdrl_resample_wcs_get_scales(hdrl_resample_outgrid_parameter *aParams_outputgrid,
		    double *aXScale, double *aYScale)
{
  cpl_ensure_code(aParams_outputgrid && aXScale && aYScale, CPL_ERROR_NULL_INPUT);

  cpl_errorstate prestate = cpl_errorstate_get();

  const cpl_matrix *cd = cpl_wcs_get_cd(aParams_outputgrid->wcs);
  //double cd11 = aParams_outputgrid->limits.cd11;
  //double cd22 = aParams_outputgrid->limits.cd22;
  //double cd12 = aParams_outputgrid->limits.cd12;
  //double cd21 = aParams_outputgrid->limits.cd21;
  double cd11 = cpl_matrix_get(cd, 0, 0);
  double cd12 = cpl_matrix_get(cd, 0, 1);
  double cd21 = cpl_matrix_get(cd, 1, 0);
  double cd22 = cpl_matrix_get(cd, 1, 1);


  double  det = cd11 * cd22 - cd12 * cd21;
  cpl_ensure_code(cpl_errorstate_is_equal(prestate), cpl_error_get_code());

  if (det < 0.) {
      cd12 *= -1;
      cd11 *= -1;
  }
  if (cd12 == 0. && cd21 == 0.) { /* matrix without rotation */
      *aXScale = cd11;
      *aYScale = cd22;
      return CPL_ERROR_NONE;
  }
  *aXScale = sqrt(cd11*cd11 + cd12*cd12); /* only the absolute value */
  *aYScale = sqrt(cd22*cd22 + cd21*cd21);
  return CPL_ERROR_NONE;
} /* hdrl_resample_wcs_get_scales() */
/*---------------------------------------------------------------------------*/
/**
  @brief   Return a pointer to the rows stored in one pixel.
  @param   aGrid    Pointer to pixel grid.
  @param   aIndex   Pixel index, as computed by hdrl_resample_pixgrid_get_index().

  @note This function is defined as static inline, i.e. should get integrated
        into the caller by the compiler, to maximise execution speed.
 */
/*---------------------------------------------------------------------------*/
static inline const cpl_size *
hdrl_resample_pixgrid_get_rows(hdrl_resample_pixgrid *aGrid, cpl_size aIndex)
{

  cpl_ensure(aGrid , CPL_ERROR_NULL_INPUT, 0);
  cpl_ensure(aIndex >= 0 , CPL_ERROR_ILLEGAL_INPUT, 0);
  cpl_ensure(aIndex < aGrid->nx * aGrid->ny * aGrid->nz, CPL_ERROR_ILLEGAL_INPUT, 0);
  /* get entry in pixel grid */
  cpl_size p = aGrid->pix[aIndex];
  if (p == 0) { /* points nowhere --> no array */
      return NULL;
  }
  if (p > 0) { /* points to pixel table */
      return aGrid->pix + aIndex;
  }
  /* p is negative, so points to an extension map, get its array */
  unsigned short ix = (-p >> XMAP_LSHIFT) & XMAP_BITMASK;
  p = (-p - 1) & PT_IDX_MASK;
  /* the pix component provides the array of pixel table rows in this index */
  return aGrid->xmaps[ix][p].pix;
} /* hdrl_resample_pixgrid_get_rows() */


/*---------------------------------------------------------------------------*/
/**
  @private
  @brief   Do the resampling from pixel grid into 3D cube using nearest
           neighbor.
  @param   aCube       the hdrl_resample datacube to fill
  @param   ResTable   the input pixel table
  @param   aGrid       the input pixel grid
  @param   aParams_outputgrid     resampling outgrid parameters
  @return  CPL_ERROR_NONE on success, another cpl_error_code on failure

  Simply loop through all grid points wavelength by wavelength and set the data,
  dq, and stat values of the one or closest data point.

  @error{set CPL_ERROR_NULL_INPUT\, return NULL,
         aCube\, ResTable\, or aGrid are NULL}
 */
/*---------------------------------------------------------------------------*/
static cpl_error_code
hdrl_resample_cube_nearest(hdrl_resample_result *aCube, const cpl_table *ResTable,
			     hdrl_resample_pixgrid *aGrid,
			     hdrl_resample_outgrid_parameter *aParams_outputgrid)
{
  cpl_ensure_code(aCube && ResTable && aGrid && aParams_outputgrid,
		  CPL_ERROR_NULL_INPUT);
  cpl_ensure_code(cpl_propertylist_has(aCube->header,"CRVAL3") == 1,
		  CPL_ERROR_ILLEGAL_INPUT);
  cpl_ensure_code(cpl_propertylist_has(aCube->header,"CRPIX3") == 1,
  		  CPL_ERROR_ILLEGAL_INPUT);
  cpl_ensure_code(cpl_propertylist_has(aCube->header,"CD3_3") == 1,
  		  CPL_ERROR_ILLEGAL_INPUT);

  double crval3 = hdrl_resample_pfits_get_crval(aCube->header, 3);
  double crpix3 = hdrl_resample_pfits_get_crpix(aCube->header, 3);
  double cd33 = hdrl_resample_pfits_get_cd(aCube->header, 3, 3);
  cpl_ensure_code(cd33 != 0, CPL_ERROR_ILLEGAL_INPUT);

  cpl_wcs *wcscpl = cpl_wcs_new_from_propertylist(aCube->header);

  if (hdrl_resample_inputtable_verify(ResTable)) {
       return CPL_ERROR_ILLEGAL_INPUT;
  }


  const double *xpos = cpl_table_get_data_double_const(ResTable, HDRL_RESAMPLE_TABLE_RA),
      *ypos = cpl_table_get_data_double_const(ResTable, HDRL_RESAMPLE_TABLE_DEC),
      *lbda = cpl_table_get_data_double_const(ResTable, HDRL_RESAMPLE_TABLE_LAMBDA),
      *data = cpl_table_get_data_double_const(ResTable, HDRL_RESAMPLE_TABLE_DATA),
      *stat = cpl_table_get_data_double_const(ResTable, HDRL_RESAMPLE_TABLE_ERRORS);
  const int *dq = cpl_table_get_data_int_const(ResTable, HDRL_RESAMPLE_TABLE_BPM);

  /* If our data was astrometrically calibrated, we need to scale the *
   * data units to the pixel size in all three dimensions so that the *
   * radius computation works again.                                  *
   * Otherwise dx~5.6e-5deg won't contribute to the weighting at all. */

  double xnorm = 1., ynorm = 1., znorm = 1. ;


  hdrl_resample_wcs_get_scales(aParams_outputgrid, &xnorm, &ynorm);
  //TODO: we should check that xnorm, ynorm, znorm are not zero
  xnorm = 1. / xnorm;
  ynorm = 1. / ynorm;
  //znorm = 1. / aParams_outputgrid->limits.cd33;
  const cpl_matrix *cd = cpl_wcs_get_cd(aParams_outputgrid->wcs);
  if (cpl_matrix_get_ncol(cd) == 3) {
      znorm = 1. / cpl_matrix_get(cd, 2, 2);
  }
/*
  cpl_msg_debug(cpl_func, "Norm factors from wcs - xnorm: %g ynorm: %g znorm: %g",
		xnorm, ynorm, znorm);
   need to use the real coordinate offset for celestial spherical
  // We are now working with the full astrometric solution
  //ptxoff = hdrl_resample_pfits_get_crval(ResTable->header, 1);
  //ptyoff = hdrl_resample_pfits_get_crval(ResTable->header, 2);
*/

  struct timeval tv1, tv2;
  cpl_msg_debug(cpl_func,"Starting parallel loop in hdrl_resample_cube_nearest");
  gettimeofday(&tv1, NULL);

#pragma omp parallel for default(none)                 /* as req. by Ralf */ \
    shared(aCube, aGrid, cd33, crpix3, crval3, data, dq, lbda,           \
	   stat, stdout, wcscpl, \
	   xnorm, xpos, ynorm, ypos, znorm) collapse(2)

  for (cpl_size l = 0; l < aGrid->nz; l++) {
      for (cpl_size i = 0; i < aGrid->nx; i++) {
	  double *pdata   = cpl_image_get_data_double(hdrl_image_get_image(hdrl_imagelist_get(aCube->himlist, l)));
	  double *pstat   = cpl_image_get_data_double(hdrl_image_get_error(hdrl_imagelist_get(aCube->himlist, l)));
	  cpl_binary *pdq = cpl_mask_get_data(hdrl_image_get_mask(hdrl_imagelist_get(aCube->himlist, l)));
	  /* wavelength of center of current grid cell (l is index starting at 0) */
	  double lambda = (l + 1. - crpix3) * cd33 + crval3;

	  cpl_size j;
	  for (j = 0; j < aGrid->ny; j++) {
	      cpl_size idx = hdrl_resample_pixgrid_get_index(aGrid, i, j, l, CPL_FALSE),
		  n_rows = hdrl_resample_pixgrid_get_count(aGrid, idx);
	      const cpl_size *rows = hdrl_resample_pixgrid_get_rows(aGrid, idx);

	      /* x and y position of center of current grid cell (i, j start at 0) */
	      double x = 0., y = 0.;

	      // We are now working with the full astrometric solution
	      // hdrl_resample_wcs_celestial_from_pixel_fast(wcs, i + 1, j + 1, &x, &y);
	      hdrl_wcs_xy_to_radec(wcscpl, i + 1, j + 1, &x, &y);

	      if (n_rows == 1) {
		  if ((cpl_binary)dq[rows[0]] == CPL_BINARY_0) {
		      /* if there is only one pixel in the cell, just use it */
		      pdata[i + j * aGrid->nx] = data[rows[0]];
		      pstat[i + j * aGrid->nx] = stat[rows[0]];
		      pdq[i + j * aGrid->nx] = (cpl_binary)dq[rows[0]];
		  } else {
		      pdq[i + j * aGrid->nx] = CPL_BINARY_1;
		  }
	      } else if (n_rows >= 2) {
		  /* loop through all available values and take the closest one */
		  cpl_size n, nbest = -1;
		  double dbest = FLT_MAX; /* some unlikely large value to start with*/
		  for (n = 0; n < n_rows; n++) {
		      /* do not use bad pixels */
		      if((cpl_binary)dq[rows[n]] != CPL_BINARY_0) {
			  continue;
		      }
		      /* the differences for the cell center and the current pixel */
		      double dx = fabs(x - xpos[rows[n]]) * xnorm,
			  dy = fabs(y - ypos[rows[n]]) * ynorm,
			  dlambda = fabs(lambda - lbda[rows[n]]) * znorm,
			  dthis = sqrt(dx*dx + dy*dy + dlambda*dlambda);

		      /* Not strictly necessary for NN, but still scale the RA   *
		       * distance properly, see hdrl_resample_cube_weighted(). */
		      dx *= cos(y * CPL_MATH_RAD_DEG);

		      if (dthis < dbest) {
			  nbest = n;
			  dbest = dthis;
		      }
		  }
		  if (nbest >= 0) { /* We found a good nearest neighbor */
		      pdata[i + j * aGrid->nx] = data[rows[nbest]];
		      pstat[i + j * aGrid->nx] = stat[rows[nbest]];
		      pdq[i + j * aGrid->nx] = (cpl_binary)dq[rows[nbest]];
		  }
	      } else {
		  /* npix == 0: do nothing, pixel stays zero */
		  pdq[i + j * aGrid->nx] = CPL_BINARY_1;
	      }
	  } /* for j (y direction) */
      } /* for i (x direction) */
  } /* for l (wavelength planes) */

  gettimeofday(&tv2, NULL);
  cpl_msg_debug(cpl_func, "Wall time for hdrl_resample_cube_nearest was %f seconds\n",
	       (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
	       (double) (tv2.tv_sec - tv1.tv_sec));

  /* Make sure that the bpm of the image and the error are in sinc as we are
   * working with pointers */
  cpl_size size = hdrl_imagelist_get_size(aCube->himlist);
  for(cpl_size i = 0; i < size; i++){
      /* sync image and error bpm ignoring what is in error before */
      cpl_image_reject_from_mask(hdrl_image_get_error(hdrl_imagelist_get(aCube->himlist, i)),
				 hdrl_image_get_mask(hdrl_imagelist_get(aCube->himlist, i) ));
  }

  cpl_wcs_delete(wcscpl);

  return CPL_ERROR_NONE;
} /* hdrl_resample_cube_nearest() */

/*---------------------------------------------------------------------------*/
/**
  @private
  @brief   Modified Shepard-like distance weighting function following Renka.
  @param   r     linear distance r
  @param   r_c   critical radius r_c beyond which the function returns 0
  @return  the weight

  Set weight to high number in case of perfect match (using FLT_MAX for this
  double variable should allow enough space for other pixel's weights to be
  added on top of this) and to a very low number in case this value is outside
  the critical radius r_c (using DBL_MIN avoids divide-by-zero).
 */
/*---------------------------------------------------------------------------*/
static inline double
hdrl_resample_weight_function_renka(double r, double r_c)
{
  if (r == 0) {
      return FLT_MAX;
  } else if (r >= r_c) {
      return DBL_MIN;
  } else {
      double p = (r_c - r) / (r_c * r);
      return p*p;
  }
} /* hdrl_resample_weight_function_renka() */

/*---------------------------------------------------------------------------*/
/**
  @private
  @brief   Drizzle-like distance weighting function.
  @param   xin    pixfrac-scaled input pixel size in x direction (spatial)
  @param   yin    pixfrac-scaled input pixel size in y direction (spatial)
  @param   zin    pixfrac-scaled input pixel size in z direction (wavelength)
  @param   xout   target voxel size in x direction (spatial)
  @param   yout   target voxel size in y direction (spatial)
  @param   zout   target voxel size in z direction (wavelength)
  @param   dx     offset from target voxel in x direction (spatial)
  @param   dy     offset from target voxel in y direction (spatial)
  @param   dz     offset from target voxel in z direction (wavelength)
  @return  the weight

  This function computes the percentage of flux of the original pixel that
  "drizzles" into the target voxel. It therefore has to be given a good estimate
  of the input size and the target size as well as the offset in all three axes.
 */
/*---------------------------------------------------------------------------*/
static inline double
hdrl_resample_weight_function_drizzle(double xin, double yin, double zin,
					double xout, double yout, double zout,
					double dx, double dy, double dz)
{
  /* compute the three terms in the numerator: if the offset   *
   * plus the output halfsize is less than the input halfsize, *
   * then that side is fully contained in the input pixel      */
  double x = (dx + xout / 2.) <= xin / 2. ? xout : (xin + xout) / 2. - dx,
      y = (dy + yout / 2.) <= yin / 2. ? yout : (yin + yout) / 2. - dy,
	  z = (dz + zout / 2.) <= zin / 2. ? zout : (zin + zout) / 2. - dz;
  /* any negative value means that the input pixel is completely *
   * outside the target voxel, so it doesn't contribute          */
  if (x <= 0 || y <= 0 || z <= 0) {
      return 0.;
  }
  /* any value > input size means this dimension of the input pixel *
   * is completely inside the target voxel, so that's the limit!    */
  return (x > xin ? xin : x) * (y > yin ? yin : y) * (z > zin ? zin : z)
      / (xin * yin * zin);
} /* hdrl_resample_weight_function_drizzle() */


/*---------------------------------------------------------------------------*/
/**
  @private
  @brief   Linear inverse distance weighting function.
  @param   r   linear distance r
  @return  the weight

  Set weight to high number in case of perfect match (using FLT_MAX for this
  double variable should allow enough space for other pixel's weights to be
  added on top of this).
 */
/*---------------------------------------------------------------------------*/
static inline double
hdrl_resample_weight_function_linear(double r)
{
  return r == 0 ? FLT_MAX : 1. / r;
} /* hdrl_resample_weight_function_linear() */

/*---------------------------------------------------------------------------*/
/**
  @private
  @brief   Quadratic inverse distance weighting function.
  @param   r2   squared distance r^2
  @return  the weight

  Set weight to high number in case of perfect match (using FLT_MAX for this
  double variable should allow enough space for other pixel's weights to be
  added on top of this).
 */
/*---------------------------------------------------------------------------*/
static inline double
hdrl_resample_weight_function_quadratic(double r2)
{
  return r2 == 0 ? FLT_MAX : 1. / r2;
} /* hdrl_resample_weight_function_quadratic() */

/*---------------------------------------------------------------------------*/
/**
  @private
  @brief   Normalized sinc distance weighting function.
  @param   r   linear distance r
  @return  the weight
 */
/*---------------------------------------------------------------------------*/
static inline double
hdrl_resample_weight_function_sinc(double r)
{
  return fabs(r) < DBL_EPSILON ? 1. : sin(CPL_MATH_PI * r) / (CPL_MATH_PI * r);
} /* hdrl_resample_weight_function_sinc() */

/*---------------------------------------------------------------------------*/
/**
  @private
  @brief   Lanczos distance weighting function (restricted sinc).
  @param   dx   linear distance in x direction (spatial)
  @param   dy   linear distance in y direction (spatial)
  @param   dz   linear distance in z direction (wavelength)
  @param   ld   maximum radius in pixels
  @param   lks  lanczos kernel size
  @return  the weight
 */
/*---------------------------------------------------------------------------*/
static inline double
hdrl_resample_weight_function_lanczos(double dx, double dy, double dz,
					unsigned int ld, unsigned int lks)
{
  /* Adding 0.5 as for a loop distance of 0 the weight should only drop to 0 if
   * the distance is larger then half the pixel */
  return (fabs(dx) >= (ld + 0.5) || fabs(dy) >= (ld + 0.5) || fabs(dz) > (ld + 0.5)) ? 0.
      : hdrl_resample_weight_function_sinc(dx) * hdrl_resample_weight_function_sinc(dx / lks)
	* hdrl_resample_weight_function_sinc(dy) * hdrl_resample_weight_function_sinc(dy / lks)
	* hdrl_resample_weight_function_sinc(dz) * hdrl_resample_weight_function_sinc(dz / lks);
} /* hdrl_resample_weight_function_lanczos() */

/*---------------------------------------------------------------------------*/
/**
  @private
  @brief   Do the resampling from pixel grid into 3D cube using a weighting
           scheme.
  @param   aCube       the hdrl_resample datacube to fill
  @param   ResTable   the input pixel table
  @param   aGrid       the input pixel grid
  @param   aParams_method     the resampling method parameters
  @param   aParams_outputgrid     the resampling outgrid parameters
  @return  CPL_ERROR_NONE on success, another cpl_error_code on failure

  Loop through all grid points wavelength by wavelength and compute the final
  values (of data/flux, dq, and stat) using a suitable weighting scheme on the
  surrounding grid points.

  All parameters are set in the aParams_outputgrid/method structures.

  @error{set and return CPL_ERROR_NULL_INPUT,
         aCube\, ResTable\, aGrid\, or aParams are NULL}
  @error{override it and output warning, loop distance (aParams->ld) is <= 0}
 */
/*---------------------------------------------------------------------------*/
static cpl_error_code
hdrl_resample_cube_weighted(hdrl_resample_result *aCube,
		const cpl_table *ResTable,
		hdrl_resample_pixgrid *aGrid,
		hdrl_resample_method_parameter *aParams_method,
		hdrl_resample_outgrid_parameter *aParams_outputgrid)
{

  cpl_ensure_code(aCube && ResTable && aGrid && aParams_method &&
		  aParams_outputgrid, CPL_ERROR_NULL_INPUT);
  cpl_ensure_code(cpl_propertylist_has(aCube->header,"CRVAL3") == 1,
		  CPL_ERROR_ILLEGAL_INPUT);
  cpl_ensure_code(cpl_propertylist_has(aCube->header,"CRPIX3") == 1,
  		  CPL_ERROR_ILLEGAL_INPUT);
  cpl_ensure_code(cpl_propertylist_has(aCube->header,"CD3_3") == 1,
  		  CPL_ERROR_ILLEGAL_INPUT);

  double crval3 = hdrl_resample_pfits_get_crval(aCube->header, 3),
      crpix3 = hdrl_resample_pfits_get_crpix(aCube->header, 3),
      cd33 = hdrl_resample_pfits_get_cd(aCube->header, 3, 3);

  hdrl_resample_smallwcs *wcs = hdrl_resample_smallwcs_new(aCube->header);
  cpl_wcs *wcscpl = cpl_wcs_new_from_propertylist(aCube->header);

  const double *xpos = cpl_table_get_data_double_const(ResTable, HDRL_RESAMPLE_TABLE_RA),
      *ypos = cpl_table_get_data_double_const(ResTable, HDRL_RESAMPLE_TABLE_DEC),
      *lbda = cpl_table_get_data_double_const(ResTable, HDRL_RESAMPLE_TABLE_LAMBDA),
      *data = cpl_table_get_data_double_const(ResTable, HDRL_RESAMPLE_TABLE_DATA),
      *stat = cpl_table_get_data_double_const(ResTable, HDRL_RESAMPLE_TABLE_ERRORS);
  const int *dq = cpl_table_get_data_int_const(ResTable, HDRL_RESAMPLE_TABLE_BPM);

  /* If our data was astrometrically calibrated, we need to scale the *
   * data units to the pixel size in all three dimensions so that the *
   * radius computation works again.                                  *
   * Otherwise dx~5.6e-5deg won't contribute to the weighting at all. */

  double xnorm = 1., ynorm = 1., znorm = 1. ;

  hdrl_resample_wcs_get_scales(aParams_outputgrid, &xnorm, &ynorm);
  xnorm = 1. / xnorm;
  ynorm = 1. / ynorm;
  //znorm = 1. / aParams_outputgrid->limits.cd33;
  const cpl_matrix *cd = cpl_wcs_get_cd(aParams_outputgrid->wcs);
  if (cpl_matrix_get_ncol(cd) == 3) {
      znorm = 1. / cpl_matrix_get(cd, 2, 2);
  }
/*
  cpl_msg_debug(cpl_func, "Norm factors from wcs - xnorm: %g ynorm: %g znorm: %g",
		xnorm, ynorm, znorm);
   need to use the real coordinate offset for celestial spherical
  // We are now working with the full astrometric solution
  //ptxoff = hdrl_resample_pfits_get_crval(ResTable->header, 1);
  //ptyoff = hdrl_resample_pfits_get_crval(ResTable->header, 2);
*/


  /* scale the input critical radius by the voxel radius */
  double renka_rc = aParams_method->renka_critical_radius /*beware of rotation! */
      * sqrt((wcs->cd11*xnorm)*(wcs->cd11*xnorm)
	     + (wcs->cd22*ynorm)*(wcs->cd22*ynorm)
	     + (cd33*znorm)*(cd33*znorm));

  /* loop distance (to take into account surrounding pixels) verification */
  int ld = aParams_method->loop_distance;
  if (ld < 0) {
      ld = 0;
      cpl_msg_debug(cpl_func, "Overriding loop distance ld=%d", ld);
  }

  /* Lankzos kernel size (lks) verification */
  int lks = aParams_method->lanczos_kernel_size;
  if (lks <= 0) {
      lks = 1;
      cpl_msg_debug(cpl_func, "Overriding lanczos kernel size lks=%d", lks);
  }

  /* Should 1/variance be used as an additional weight */
  cpl_boolean wght = aParams_method->use_errorweights;

  /* pixel sizes in all three directions, scaled by pixfrac, and *
   * output pixel sizes (absolute values), as needed for drizzle */
  double xsz = aParams_method->drizzle_pix_frac_x / xnorm,
      ysz = aParams_method->drizzle_pix_frac_y / ynorm,
      zsz = aParams_method->drizzle_pix_frac_lambda / znorm,
      xout = fabs(wcs->cd11), yout = fabs(wcs->cd22), zout = fabs(cd33);

  struct timeval tv1, tv2;
  cpl_msg_debug(cpl_func,"Starting parallel loop in hdrl_resample_cube_weighted");
  gettimeofday(&tv1, NULL);

#pragma omp parallel for default(none)                 /* as req. by Ralf */ \
    shared(aCube, aParams_method, aParams_outputgrid, aGrid, ResTable, cd33, crpix3, crval3, data, \
	   dq, lbda, ld, lks, renka_rc, stat,      \
	   stdout, wcs, wcscpl, wght, xnorm, xout, xpos, xsz, ynorm, yout,\
	   ypos, ysz, znorm, zout, zsz) collapse(2)

  for (cpl_size l = 0; l < aGrid->nz; l++) {
      for (cpl_size i = 0; i < aGrid->nx; i++) {
      double *pdata   = cpl_image_get_data_double(hdrl_image_get_image(hdrl_imagelist_get(aCube->himlist, l)));
      double *pstat   = cpl_image_get_data_double(hdrl_image_get_error(hdrl_imagelist_get(aCube->himlist, l)));
      cpl_binary *pdq = cpl_mask_get_data(hdrl_image_get_mask(hdrl_imagelist_get(aCube->himlist, l)));

	  /* wavelength of center of current grid cell (l is index starting at 0) */
	  double lambda = (l + 1. - crpix3) * cd33 + crval3;
	  double zout2 = zout; /* correct the output pixel size for log-lambda */

	  for (cpl_size j = 0; j < aGrid->ny; j++) {
	      /* x and y position of center of current grid cell (i, j start at 0) */
	      double x, y;

	      // We are now working with the full astrometric solution
	      // hdrl_resample_wcs_celestial_from_pixel_fast(wcs, i + 1, j + 1, &x, &y);
	      hdrl_wcs_xy_to_radec(wcscpl, i + 1, j + 1, &x, &y);
	      //cpl_msg_debug(cpl_func,"case1 i %d j %d x %10.7f y %10.7f ",i+1,j+1,x,y);

	      double sumdata = 0, sumstat = 0, sumweight = 0;
	      double flux=0;
	      cpl_size npoints = 0;

	      /* loop through surrounding cells and their contained pixels */
	      cpl_size i2;
	      for (i2 = i - ld; i2 <= i + ld; i2++) {
		  cpl_size j2;
		  for (j2 = j - ld; j2 <= j + ld; j2++) {
		      cpl_size l2;
		      for (l2 = l - ld; l2 <= l + ld; l2++) {
			  cpl_size idx2 = hdrl_resample_pixgrid_get_index(aGrid, i2, j2, l2, CPL_FALSE);
			  if (idx2 < 0) continue;
			  cpl_size n_rows2 = hdrl_resample_pixgrid_get_count(aGrid, idx2);

			  const cpl_size *rows2 = hdrl_resample_pixgrid_get_rows(aGrid, idx2);
			  cpl_size n;
			  for (n = 0; n < n_rows2; n++) {
			      if (dq[rows2[n]]) { /* exclude all bad pixels */

				  continue;
			      }

			      double dx = fabs(x - (xpos[rows2[n]])),
				  dy = fabs(y - (ypos[rows2[n]])),
				  dlambda = fabs(lambda - lbda[rows2[n]]),
				  r2 = 0;

			      /* Since the distances of RA in degrees get larger the *
			       * closer we get to the celestial pole, we have to     *
			       * compensate for that by multiplying the distance in  *
			       * RA by cos(delta), to make it comparable to the      *
			       * distances in pixels for the different kernels below. */

			      // We are now working with the full astrometric solution
			      dx *= cos(y * CPL_MATH_RAD_DEG);

			      if (aParams_method->method != HDRL_RESAMPLE_METHOD_DRIZZLE) {
				  dx *= xnorm;
				  dy *= ynorm;
				  dlambda *= znorm;
				  r2 = dx * dx + dy * dy + dlambda * dlambda;
				  //cpl_msg_debug(cpl_func,"n %lld dx %10.7f dy %10.7f dlambda %10.7f",n,dx,dy,dlambda);
			      }
			      double weight = 0.;
			      if (aParams_method->method == HDRL_RESAMPLE_METHOD_RENKA) {
				  weight = hdrl_resample_weight_function_renka(sqrt(r2), renka_rc);
			      } else if (aParams_method->method == HDRL_RESAMPLE_METHOD_DRIZZLE) {
				  weight = hdrl_resample_weight_function_drizzle(xsz, ysz, zsz,
										   xout, yout, zout2,
										   dx, dy, dlambda);
			      } else if (aParams_method->method == HDRL_RESAMPLE_METHOD_LINEAR) {

				  weight = hdrl_resample_weight_function_linear(sqrt(r2));
				  //cpl_msg_debug(cpl_func,"r2=%10.7f weight=%10.7f",r2,weight);
			      } else if (aParams_method->method == HDRL_RESAMPLE_METHOD_QUADRATIC) {
				  weight = hdrl_resample_weight_function_quadratic(r2);
			      } else if (aParams_method->method == HDRL_RESAMPLE_METHOD_LANCZOS) {
				  weight = hdrl_resample_weight_function_lanczos(dx, dy,
										   dlambda, ld, lks);
			      }

			      if (wght && stat[rows2[n]] > 0.) { /* User wants to weight by 1/variance */
				  /* apply it on top of the weight computed here */
				  weight /= (stat[rows2[n]] * stat[rows2[n]]);
			      }

			      sumweight += weight;
			      sumdata += data[rows2[n]] * weight;
			      flux += data[rows2[n]];
			      sumstat += stat[rows2[n]] * stat[rows2[n]] * weight * weight;
			      npoints++;

			  } /* for n (all pixels in grid cell) */
		      } /* for l2 (lambda direction) */
		  } /* for j2 (y direction) */
	      } /* for i2 (x direction) */


	      /* if no points were found, we cannot divide by the summed weight *
	       * and don't need to set the output pixel value (it's 0 already), *
	       * only set the relevant Euro3D bad pixel flag
	       * In some cases only sumweight * sumweight is really zero so this
	       * check was additionally added for the error propagation part*/

	      if (!npoints || !isnormal(sumweight) || !isnormal(sumweight * sumweight)) {
		  pdq[i + j * aGrid->nx] = CPL_BINARY_1;
		  continue;
	      }

	      /* divide results by weight of summed pixels */
	      sumdata /= sumweight;
	      sumstat /= sumweight * sumweight;

	      pdata[i + j * aGrid->nx] = sumdata;
	      /* Going back from variance to errors */
	      pstat[i + j * aGrid->nx] = sqrt(sumstat);
	      pdq[i + j * aGrid->nx] = CPL_BINARY_0; /* now we can mark it as good */

	  } /* for j (y direction) */
      } /* for i (x direction) */
  } /* for l (wavelength planes) */

  gettimeofday(&tv2, NULL);
  cpl_msg_debug(cpl_func, "Wall time for hdrl_resample_cube_weighted was %f seconds\n",
	       (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
	       (double) (tv2.tv_sec - tv1.tv_sec));

  /* Make sure that the bpm of the image and the error are in sinc as we are
   * working with pointers */
  cpl_size size = hdrl_imagelist_get_size(aCube->himlist);
  for(cpl_size i = 0; i < size; i++){
      /* sync image and error bpm ignoring what is in error before */
      cpl_image_reject_from_mask(hdrl_image_get_error(hdrl_imagelist_get(aCube->himlist, i)),
				 hdrl_image_get_mask(hdrl_imagelist_get(aCube->himlist, i) ));
  }

  cpl_free(wcs);
  cpl_wcs_delete(wcscpl);

  return CPL_ERROR_NONE;
} /* hdrl_resample_cube_weighted() */
/*---------------------------------------------------------------------------*/
/**
  @private
  @brief   set the output grid parameters

  @param   xsize   the output grid X size
  @param   ysize   the output grid Y size
  @param   zsize   the output grid Z size
  @param   cube   the input pixel table
  @param   aParams_outputgrid     the resampling outgrid parameters
  @return  CPL_ERROR_NONE on success, another cpl_error_code on failure

  Set relevant FITS header WCS FITS keywords based on the information provided
  by the aParams_outputgrid structure.
  CRPIX1 = (xsize + 1) / 2.
  CRPIX2 = (ysize + 1) / 2.
  CRPIX3 = 1.

 */
/*---------------------------------------------------------------------------*/
static cpl_error_code hdrl_resampling_set_outputgrid (
    int xsize, int ysize, int zsize, hdrl_resample_result *cube,
    hdrl_resample_outgrid_parameter *aParams_outputgrid)
{
  cpl_ensure_code(xsize > 0, CPL_ERROR_ILLEGAL_INPUT);
  cpl_ensure_code(ysize > 0,CPL_ERROR_ILLEGAL_INPUT);
  cpl_ensure_code(zsize >= 0, CPL_ERROR_ILLEGAL_INPUT);

  cpl_ensure_code(cube && aParams_outputgrid, CPL_ERROR_NULL_INPUT);
  /* TODO: cube->header may be already allocated. In this case why a new and
  	 * not an over-write of the content?
  	 */
  cube->header = cpl_propertylist_new ();
  hdrl_wcs_to_propertylist (aParams_outputgrid->wcs, cube->header,
			    FALSE);

  cpl_propertylist_update_string (cube->header, "CTYPE1", "RA---TAN");
  cpl_propertylist_update_string (cube->header, "CTYPE2", "DEC--TAN");
  cpl_propertylist_set_comment(cube->header, "CTYPE1", "Gnomonic projection");
  cpl_propertylist_set_comment(cube->header, "CTYPE2", "Gnomonic projection");



  /* set NAXIS for later handling of the WCS */
  cpl_propertylist_update_int (cube->header, "NAXIS", 3);
  cpl_propertylist_update_int (cube->header, "NAXIS1", xsize);
  cpl_propertylist_update_int (cube->header, "NAXIS2", ysize);
  cpl_propertylist_update_int (cube->header, "NAXIS3", zsize);
  /* if pixel table was astrometrically calibrated, use its WCS headers *
   * Axis 1: x or RA, axis 2: y or DEC, axis 3: lambda
   *
   *                 */
  cpl_propertylist_update_double (cube->header, "CD1_1",
				  -aParams_outputgrid->delta_ra);
  cpl_propertylist_update_double (cube->header, "CD2_2",
				  aParams_outputgrid->delta_dec);
  cpl_propertylist_update_double (cube->header, "CD1_2", 0.);
  cpl_propertylist_update_double (cube->header, "CD2_1", 0.);

  double ramin = aParams_outputgrid->ra_min;
  double ramax = aParams_outputgrid->ra_max;
  double decmin = aParams_outputgrid->dec_min;
  double decmax = aParams_outputgrid->dec_max;

  /* Following swarp we put CRPIX and CRVAL to the center of the field */
  cpl_propertylist_update_double(cube->header, "CRPIX1", (double)((xsize + 1) / 2.));
  cpl_propertylist_update_double(cube->header, "CRPIX2", (double)((ysize + 1) / 2.));

  if(ramax - ramin < 180) {
      /* To be checked: Both values are in 0 - 180 or 180 - 360 */
      cpl_propertylist_update_double(cube->header, "CRVAL1", (ramin + ramax) / 2.);
  } else {
      double diff1 = 360. - ramax;
      double diff2 = ramin - 0.;
      if (diff1 < diff2) {
	  cpl_propertylist_update_double(cube->header, "CRVAL1", ramin - (diff1 + diff2) / 2);
      } else {
	  cpl_propertylist_update_double(cube->header, "CRVAL1", ramax + (diff1 + diff2) / 2.);
      }
  }
  cpl_propertylist_update_double(cube->header, "CRVAL2", (decmin + decmax) / 2.);
  cpl_propertylist_update_double (cube->header, "CD3_3",
				  aParams_outputgrid->delta_lambda);
  cpl_propertylist_update_double (cube->header, "CRPIX3", 1.);
  cpl_propertylist_update_double (cube->header, "CRVAL3",
				  aParams_outputgrid->lambda_min);
  /* fill in empty cross-terms of the CDi_j matrix */
  cpl_propertylist_update_double (cube->header, "CD1_3", 0.);
  cpl_propertylist_update_double (cube->header, "CD2_3", 0.);
  cpl_propertylist_update_double (cube->header, "CD3_1", 0.);
  cpl_propertylist_update_double (cube->header, "CD3_2", 0.);

  return cpl_error_get_code();
}

/*---------------------------------------------------------------------------*/
/**
  @brief   Resample a pixel table onto a regular grid structure representing
           a FITS NAXIS=3 datacube.
  @param   ResTable   the hdrl_resample pixel table to resample
  @param   aParams_method     the resampling method parameters
  @param   aParams_outputgrid     the resampling output grid parameters
  @param   aGrid       if not NULL, use it to store the pixel grid pointer
  @return  A hdrl_resample_datacube * for the output FITS datacube (and its bad pixels,
           error and headers) or NULL on error.

  This function implements the resampling scheme discussed in Sect. 2.2 of the
  DRLDesign document:
  First, convert the input pixel table into a regular grid of cells, storing
  the input pixels in their nearest cell. To resample, then loop through all
  cells, sampling surrounding pixels depending on the requested method. Values
  that rise aHSigma above the surrounding values, i.e. cosmic rays, are removed
  at this stage. Store the output pixels (their values, bad pixel status, and
  error) in a hdrl_resample_datacube structure to be used to store 3xFITS_NAXIS=3
  files.

  @note This function changes some of the components of aParams. Specifically,
        dx, dy, and dlambda may be changed after calling this function!

  @qa Using the INM, different astronomical scenes can be created to be
      used for quality checks.
      When creating a scene with a few well separated point sources, one knows
      the flux of each source at each wavelength. If this function works
      correctly, the flux should be conserved and can be compared to the
      expected value (using some tolerance).
      If using a scene with sky emission lines, one can measure the wavelengths
      and dispersions in a few of the sky lines. They should be at the same
      wavelength for all spaxels and this position should agree with the
      position determined from the INM-created data.
      Additionally, a (visual) comparison of monochromatic maps created from
      INM input frames can be compared to maps derived from output cubes created
      by this routine. If all intermediate calibrations were derived correctly,
      both should look similar.

  @error{set CPL_ERROR_NULL_INPUT\, return NULL,
         the input pixel table or input param structure are NULL}
  @error{set CPL_ERROR_ILLEGAL_INPUT\, return NULL,
         the input pixel table does not contain full geometry information}
  @error{set CPL_ERROR_UNSUPPORTED_MODE\, return NULL,
         the WCS in the pixel table is neither in pixels nor degrees}
  @error{set CPL_ERROR_ILLEGAL_OUTPUT\, return NULL,
         computed output size in at least one coordinate is not positive}
  @error{set CPL_ERROR_DATA_NOT_FOUND\, fill aGrid with NULL\, and return NULL,
         could not create pixel grid for the cube}
  @error{just create empty datacube and possibly return pixel grid,
         given method is HDRL_RESAMPLE_METHOD_NONE}
  @error{set CPL_ERROR_UNSUPPORTED_MODE\, return NULL, given method is unknown}
  @error{return NULL\, propagate error code, resampling fails}
 */
/*---------------------------------------------------------------------------*/
static hdrl_resample_result *
hdrl_resample_cube(const cpl_table *ResTable,
		     hdrl_resample_method_parameter *aParams_method,
		     hdrl_resample_outgrid_parameter *aParams_outputgrid,
		     hdrl_resample_pixgrid **aGrid)
{
  cpl_ensure(ResTable && aParams_method && aParams_outputgrid && aGrid,
		  CPL_ERROR_NULL_INPUT, NULL);

  /* compute or set the size of the output grid depending on *
   * the inputs and the data available in the pixel table    */


  /* compute output sizes; wavelength is different in that it is *
   * more useful to contain partly empty areas within the field  *
   * for the extreme ends, so use ceil()                         */
  int xsize = 0, ysize = 0, zsize = 0;

  hdrl_resample_compute_size(aParams_outputgrid, &xsize, &ysize, &zsize);

  /* Following swarp for x and y : Add a margin in field size */

  xsize *= (100. + aParams_outputgrid->fieldmargin)/100.;
  ysize *= (100. + aParams_outputgrid->fieldmargin)/100.;

  cpl_ensure(xsize > 0 && ysize > 0 && zsize > 0, CPL_ERROR_ILLEGAL_OUTPUT,
	     NULL);

  double time = cpl_test_get_walltime();

  /* create the structure for the output datacube */
  hdrl_resample_result *cube = cpl_calloc(1, sizeof(hdrl_resample_result));

  hdrl_resampling_set_outputgrid (xsize, ysize, zsize, cube, aParams_outputgrid);

  if (aParams_method->method < HDRL_RESAMPLE_METHOD_NONE) {
      /* fill the cube for the data */
      cube->himlist = hdrl_imagelist_new();

      for (cpl_size i = 0; i < zsize; i++) {
	  hdrl_image * image = hdrl_image_new(xsize, ysize);

	  /* Set all pixels a priori to bad - do not use pointers to keep the
	   * bpm of the data and error image in sync*/
	  for (cpl_size j = 1; j <= xsize; j++) {
	      for (cpl_size k = 1; k <= ysize; k++) {
		  hdrl_image_reject(image, j, k);
	      }
	  }

          hdrl_imagelist_set(cube->himlist, image, i);
      } /* for i (all wavelength planes) */
  } /* if method not HDRL_RESAMPLE_METHOD_NONE */

  /* convert the pixel table into a pixel grid */
  hdrl_resample_pixgrid *grid = hdrl_resample_pixgrid_create(ResTable, cube->header,
					   xsize, ysize, zsize);
  if (!grid) {
      hdrl_resample_result_delete(cube);
      cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND, "Could not create"
			    " pixel grid!");
      if (aGrid) {
	  *aGrid = NULL;
      }
      return NULL;
  } /* if not grid */

  double timeinit = cpl_test_get_walltime(),
      cpuinit = cpl_test_get_cputime();

  /* do the resampling */
  cpl_error_code rc = CPL_ERROR_NONE;
  switch (aParams_method->method) {
    case HDRL_RESAMPLE_METHOD_NEAREST:
      cpl_msg_debug(cpl_func, "Starting resampling, using method \"nearest\"");
      rc = hdrl_resample_cube_nearest(cube, ResTable, grid, aParams_outputgrid);
      break;
    case HDRL_RESAMPLE_METHOD_RENKA:
      cpl_msg_debug(cpl_func, "Starting resampling, using method \"renka\" "
		   "(critical radius rc=%f, loop distance ld=%d)",
		   aParams_method->renka_critical_radius,
		   aParams_method->loop_distance);
      rc = hdrl_resample_cube_weighted(cube, ResTable, grid, aParams_method,
					 aParams_outputgrid);
      break;
    case HDRL_RESAMPLE_METHOD_LINEAR:
    case HDRL_RESAMPLE_METHOD_QUADRATIC:
    case HDRL_RESAMPLE_METHOD_LANCZOS:
      cpl_msg_debug(cpl_func, "Starting resampling, using method \"%s\" (loop "
		   "distance ld=%d)",
		   aParams_method->method == HDRL_RESAMPLE_METHOD_LINEAR
		   ? "linear"
		       : (aParams_method->method == HDRL_RESAMPLE_METHOD_QUADRATIC
			   ? "quadratic"
			       : "lanczos"),
				 aParams_method->loop_distance);
      rc = hdrl_resample_cube_weighted(cube, ResTable, grid, aParams_method,
					 aParams_outputgrid);
      break;
    case HDRL_RESAMPLE_METHOD_DRIZZLE:
      cpl_msg_debug(cpl_func, "Starting resampling, using method \"drizzle\" "
		   "(pixfrac f=%.3f,%.3f,%.3f, loop distance ld=%d)",
		   aParams_method->drizzle_pix_frac_x,
		   aParams_method->drizzle_pix_frac_y,
		   aParams_method->drizzle_pix_frac_lambda,
		   aParams_method->loop_distance);
      rc = hdrl_resample_cube_weighted(cube, ResTable, grid,
					 aParams_method,
					 aParams_outputgrid);
      break;
    case HDRL_RESAMPLE_METHOD_NONE:
      /* cpl_msg_debug(cpl_func, "Method %d (no resampling)", aParams_method->method); */
      break;
    default:
      cpl_msg_error(cpl_func, "Don't know this resampling method: %d",
		    aParams_method->method);
      cpl_error_set(cpl_func, CPL_ERROR_UNSUPPORTED_MODE);
      rc = CPL_ERROR_UNSUPPORTED_MODE;
  }

  double timefini = cpl_test_get_walltime(),
      cpufini = cpl_test_get_cputime();

  /* now that we have resampled we can either remove the pixel grid or save it */
  if (aGrid) {
      *aGrid = grid;
  } else {
      hdrl_resample_pixgrid_delete(grid);
  }

  cpl_msg_debug(cpl_func, "resampling took %.3fs (wall-clock) and %.3fs "
		"(%.3fs CPU, %d CPUs) for hdrl_resample_cube*() alone",
		timefini - time, timefini - timeinit, cpufini - cpuinit,
		omp_get_max_threads());
  if (rc != CPL_ERROR_NONE) {
      cpl_msg_error(cpl_func, "resampling failed: %s", cpl_error_get_message());
      hdrl_resample_result_delete(cube);
      return NULL;
  }

  return cube;
} /* hdrl_resample_cube() */

/*---------------------------------------------------------------------------*/
/**
  @private
  @brief   Write WCS properties in a cpl propertylist.
  @param   wcs      cpl_wcs structure containing the wcs information
  @param   header   output header informations
  @param   only2d   if TRUE save only the 2D part of the wcs structure
  @return  CPL_ERROR_NONE on non-critical failure or success, another CPL error
           code otherwise.
 */
/*---------------------------------------------------------------------------*/
cpl_error_code
hdrl_wcs_to_propertylist(const cpl_wcs * wcs, cpl_propertylist * header,
			 cpl_boolean only2d)
{
  cpl_ensure_code(wcs && header, CPL_ERROR_NULL_INPUT);
  int err = 0;
  const cpl_array *crval = cpl_wcs_get_crval(wcs);
  const cpl_array *crpix = cpl_wcs_get_crpix(wcs);
  const cpl_array *ctype = cpl_wcs_get_ctype(wcs);
  const cpl_array *cunit = cpl_wcs_get_cunit(wcs);

  const cpl_matrix *cd = cpl_wcs_get_cd(wcs);

  const cpl_array *dims = cpl_wcs_get_image_dims(wcs);
  int naxis = cpl_wcs_get_image_naxis(wcs);


  /* Check NAXIS */
  for (cpl_size i = 0; i < naxis; i++) {
      if (i == 0) {
	  cpl_propertylist_update_int(header, "NAXIS", naxis);
      }
      char * buf = cpl_sprintf("NAXIS%lld", i + 1);
      cpl_propertylist_update_int(header, buf,
				  cpl_array_get_int(dims, i, &err));
      cpl_free(buf);
  }

/* Make sure to have the right NAXIS keywords if 2D is forced */
  if (only2d == TRUE) {
      cpl_propertylist_update_int(header, "NAXIS", 2);

      if(cpl_propertylist_has(header, "NAXIS3")){
	  cpl_propertylist_erase(header, "NAXIS3");
      }
  }

  /* for 2D images */
  if (crval) {
      cpl_propertylist_update_double(header, "CRVAL1",
				     cpl_array_get_double(crval, 0, &err));
      cpl_propertylist_update_double(header, "CRVAL2",
				     cpl_array_get_double(crval, 1, &err));
  }

  if (crpix) {
      cpl_propertylist_update_double(header, "CRPIX1",
				     cpl_array_get_double(crpix, 0, &err));
      cpl_propertylist_update_double(header, "CRPIX2",
				     cpl_array_get_double(crpix, 1, &err));
  }

  if (ctype) {
      cpl_propertylist_update_string(header, "CTYPE1",
				     cpl_array_get_string(ctype, 0));
      cpl_propertylist_update_string(header, "CTYPE2",
				     cpl_array_get_string(ctype, 1));
  }

  if (cunit) {
      cpl_propertylist_update_string(header, "CUNIT1",
				     cpl_array_get_string(cunit, 0));
      cpl_propertylist_update_string(header, "CUNIT2",
				     cpl_array_get_string(cunit, 1));
  }

  if (cd) {
      double cd11 = cpl_matrix_get(cd, 0, 0);
      double cd12 = cpl_matrix_get(cd, 0, 1);
      double cd21 = cpl_matrix_get(cd, 1, 0);
      double cd22 = cpl_matrix_get(cd, 1, 1);
      cpl_propertylist_update_double(header, "CD1_1", cd11);
      cpl_propertylist_update_double(header, "CD1_2", cd12);
      cpl_propertylist_update_double(header, "CD2_1", cd21);
      cpl_propertylist_update_double(header, "CD2_2", cd22);
  }

  /* for 3D cubes */
  if (only2d == FALSE && cpl_array_get_size(crval) > 2) {

      if (crval) {
	  cpl_propertylist_update_double(header, "CRVAL3",
					 cpl_array_get_double(crval, 2, &err));
      }

      if (crpix) {
	  cpl_propertylist_update_double(header, "CRPIX3",
					 cpl_array_get_double(crpix, 2, &err));
      }

      if (ctype) {
	  cpl_propertylist_update_string(header, "CTYPE3",
					 cpl_array_get_string(ctype, 2));
      }

      if (cunit) {
	  cpl_propertylist_update_string(header, "CUNIT3",
					 cpl_array_get_string(cunit, 2));
      }

      if (cd) {
	  double cd13 = cpl_matrix_get(cd, 0, 2);
	  double cd23 = cpl_matrix_get(cd, 1, 2);
	  double cd31 = cpl_matrix_get(cd, 2, 0);
	  double cd32 = cpl_matrix_get(cd, 2, 1);
	  double cd33 = cpl_matrix_get(cd, 2, 2);
	  cpl_propertylist_update_double(header, "CD1_3", cd13);
	  cpl_propertylist_update_double(header, "CD2_3", cd23);
	  cpl_propertylist_update_double(header, "CD3_1", cd31);
	  cpl_propertylist_update_double(header, "CD3_2", cd32);
	  cpl_propertylist_update_double(header, "CD3_3", cd33);
      }
  } /* if 3D */
  return CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
 @brief   generates a table collecting data to be resampled changed
 @param   tab   table to be filled
 @parm    size  size of table
 @return  cpl_error_code
 */
/*----------------------------------------------------------------------------*/

static cpl_error_code
hdrl_resample_create_table(cpl_table** tab, const cpl_size size)
{

	cpl_ensure_code(tab, CPL_ERROR_NULL_INPUT);
	cpl_ensure_code(size > 0 , CPL_ERROR_ILLEGAL_INPUT);

  *tab = cpl_table_new(size);
  /*
   * MUSE_PIXTABLE_XPOS    "xpos" *< x-position column of a MUSE pixel table
   * MUSE_PIXTABLE_YPOS    "ypos" *< y-position column of a MUSE pixel table
   * MUSE_PIXTABLE_LAMBDA  "lambda" *< wavelength column of a MUSE pixel table
   * MUSE_PIXTABLE_DATA    "data" *< data column of a MUSE pixel table
   * MUSE_PIXTABLE_DQ      "dq" *< data quality column of a MUSE pixel table
   * MUSE_PIXTABLE_STAT    "stat" *< error column of a MUSE pixel table
   */
  cpl_table_new_column(*tab,
                       HDRL_RESAMPLE_TABLE_RA,     HDRL_RESAMPLE_TABLE_RA_TYPE);
  cpl_table_new_column(*tab,
                       HDRL_RESAMPLE_TABLE_DEC,    HDRL_RESAMPLE_TABLE_DEC_TYPE);
  cpl_table_new_column(*tab,
                       HDRL_RESAMPLE_TABLE_LAMBDA, HDRL_RESAMPLE_TABLE_LAMBDA_TYPE);
  cpl_table_new_column(*tab,
                       HDRL_RESAMPLE_TABLE_DATA,   HDRL_RESAMPLE_TABLE_DATA_TYPE);
  cpl_table_new_column(*tab,
                       HDRL_RESAMPLE_TABLE_BPM,    HDRL_RESAMPLE_TABLE_BPM_TYPE);
  cpl_table_new_column(*tab,
                       HDRL_RESAMPLE_TABLE_ERRORS, HDRL_RESAMPLE_TABLE_ERRORS_TYPE);

  /* init column values */
  cpl_table_fill_column_window_double(*tab, HDRL_RESAMPLE_TABLE_RA,    0, size, 0.);
  cpl_table_fill_column_window_double(*tab, HDRL_RESAMPLE_TABLE_DEC,   0, size, 0.);
  cpl_table_fill_column_window_double(*tab, HDRL_RESAMPLE_TABLE_LAMBDA,0, size, 0.);
  cpl_table_fill_column_window_double(*tab, HDRL_RESAMPLE_TABLE_DATA,  0, size, 0.);
  cpl_table_fill_column_window_int(*tab,    HDRL_RESAMPLE_TABLE_BPM,   0, size, 0);
  cpl_table_fill_column_window_double(*tab, HDRL_RESAMPLE_TABLE_ERRORS,0, size, 0.);

  return cpl_error_get_code();
}

/*----------------------------------------------------------------------------*/
/**
 @brief Convert a hdrl image into a cpl table that can be given as input to
        hdrl_resample_compute()
 @param hima  hdrl_image containing the data/error/bpm
 @param wcs   The world coordinate system used for the conversion
 @return cpl_table that is used by the hdrl_resample_compute() function
 */
/*----------------------------------------------------------------------------*/
cpl_table*
hdrl_resample_image_to_table(const hdrl_image * hima, const cpl_wcs* wcs)
{
  cpl_ensure(hima, CPL_ERROR_NULL_INPUT, NULL);
  cpl_ensure(wcs, CPL_ERROR_NULL_INPUT, NULL);
  cpl_msg_debug(cpl_func, "Converting Data to table");
  hdrl_imagelist* ilist = NULL;
  cpl_table* tab;
  ilist = hdrl_imagelist_new();
  hdrl_imagelist_set(ilist, (hdrl_image *)hima, 0);

  tab = hdrl_resample_imagelist_to_table(ilist, wcs);

  /* cleanup memory */
  hdrl_imagelist_unset(ilist, 0);
  hdrl_imagelist_delete(ilist);


  return tab;
}

/*----------------------------------------------------------------------------*/
/**
 @brief Convert a hdrl imagelist into a cpl table that can be given as input to
        hdrl_resample_compute().
 @param himlist  hdrl_imagelist containing the data/error/bpm
 @param wcs   The world coordinate system used for the conversion
 @return cpl_table that is used by the hdrl_resample_compute() function
 */
/*----------------------------------------------------------------------------*/
cpl_table*
hdrl_resample_imagelist_to_table(const hdrl_imagelist* himlist,
				 const cpl_wcs* wcs)
{

  cpl_ensure(himlist, CPL_ERROR_NULL_INPUT, NULL);
  cpl_ensure(wcs, CPL_ERROR_NULL_INPUT, NULL);

  cpl_msg_debug(cpl_func, "Converting Dataset to table");

  cpl_size naxis1 = hdrl_imagelist_get_size_x(himlist);
  cpl_size naxis2 = hdrl_imagelist_get_size_y(himlist);
  cpl_size naxis3 = hdrl_imagelist_get_size(himlist);

  cpl_msg_debug(cpl_func,"Dataset dimentions (x, y, l): (%lld, %lld, %lld)",
	       naxis1, naxis2, naxis3);

  const cpl_array *crval = cpl_wcs_get_crval(wcs);
  const cpl_array *crpix = cpl_wcs_get_crpix(wcs);
  const cpl_matrix *cd = cpl_wcs_get_cd(wcs);

  int testerr = 0;
  double crpix3 = 0.;
  double crval3 = 0.;
  double cdelt3 = 0.;

  if (naxis3 > 1) { /* We have a cube */
      crpix3 = cpl_array_get_double(crpix, 2, &testerr);
      crval3 = cpl_array_get_double(crval, 2, &testerr);
      cdelt3 = cpl_matrix_get(cd, 2, 2); /* CD3_3 */
/*
      cpl_msg_debug(cpl_func,"crpix3: %g crval3: %g cdelt3: %g", crpix3,
		    crval3, cdelt3);
*/
  }

  cpl_size tab_size = naxis1 * naxis2 * naxis3;
  cpl_table* tab = NULL;
  /* Prefill the full table */
  hdrl_resample_create_table(&tab, tab_size);

  double* ptabxpos = cpl_table_get_data_double(tab, HDRL_RESAMPLE_TABLE_RA);
  double* ptabypos = cpl_table_get_data_double(tab, HDRL_RESAMPLE_TABLE_DEC);
  double* ptablambda = cpl_table_get_data_double(tab, HDRL_RESAMPLE_TABLE_LAMBDA);
  double* ptabdata = cpl_table_get_data_double(tab, HDRL_RESAMPLE_TABLE_DATA);
  int*   ptabbpm = cpl_table_get_data_int(tab, HDRL_RESAMPLE_TABLE_BPM);
  double* ptaberr = cpl_table_get_data_double(tab, HDRL_RESAMPLE_TABLE_ERRORS);

  struct timeval tv1, tv2;
  cpl_msg_debug(cpl_func,"Starting parallel loop in hdrl_imagelist_to_table");
  gettimeofday(&tv1, NULL);

  HDRL_OMP(omp parallel for collapse(2))
  for(cpl_size k = 0; k < naxis3; k++) {
      for(cpl_size j = 0; j < naxis2; j++) {
	  const double* pimaerr = NULL;
	  const cpl_binary *   pimabpm = NULL;

	  /* Fill the data */
	  const hdrl_image* hima = hdrl_imagelist_get_const(himlist, k);
	  const cpl_image* imadata = hdrl_image_get_image_const(hima);
	  const cpl_image* imaerrs = hdrl_image_get_error_const(hima);
	  const cpl_mask*  imamask  = hdrl_image_get_mask_const(hima);
	  const double* pimadata = cpl_image_get_data_double_const(imadata);

	  if (imaerrs) { /* Fill the errors */
	      pimaerr = cpl_image_get_data_double_const(imaerrs);
	  }
	  if (imamask) { /* Fill the bpm */
	      pimabpm = cpl_mask_get_data_const(imamask);
	  }

	  cpl_size k_naxis1_naxis2 = naxis1 * naxis2 * k;
	  cpl_size j_naxis1 = j * naxis1;
	  for(cpl_size i = 0; i < naxis1; i++) {
	      cpl_size raw = k_naxis1_naxis2 + j_naxis1 + i;
	      hdrl_wcs_xy_to_radec(wcs, i+1 , j+1 , &ptabxpos[raw],
			       &ptabypos[raw]);
	      ptabdata[raw] = pimadata[j_naxis1 + i];
	      if (naxis3 > 1) ptablambda[raw] = crval3 + cdelt3 * (k - crpix3 + 1.);
	      if (imaerrs) ptaberr[raw] = pimaerr[j_naxis1 + i];
	      if (imamask) ptabbpm[raw] = pimabpm[j_naxis1 + i];
	      /* Insert only good pixels */
	      if (!isfinite(pimadata[j_naxis1 + i]) ||
		  ptabbpm[raw] != CPL_BINARY_0 ){
		  ptabbpm[raw] = CPL_BINARY_1;

	      }
	  }
      }
  }

  gettimeofday(&tv2, NULL);
  cpl_msg_debug(cpl_func, "Wall time for hdrl_imagelist_to_table was %f seconds\n",
	       (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
	       (double) (tv2.tv_sec - tv1.tv_sec));

  return tab;
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup  hdrl_resample_parameter_outgrid

  @brief    Creates a resample_outgrid hdrl parameter object for a 2 dimensional
            interpolation, i.e HDRL_RESAMPLE_OUTGRID_2D. Only two values can be
            set by the caller. The remaining values (see
            hdrl_resample_parameter_create_outgrid2D_userdef() for all values)
            are derived from the data itself by the hdrl_resample_compute()
            function.
  @param    delta_ra    step size in right ascension [deg]
  @param    delta_dec   step size in declination [deg]
  @return   The HDRL_RESAMPLE_OUTGRID_2D parameter object or NULL on error.
            It needs to be deallocated with hdrl_parameter_delete().

  This function creates a hdrl_resample_parameter_outgrid object
  HDRL_RESAMPLE_OUTGRID_2D

  @see      hdrl_parameter_delete()
  @see      hdrl_resample_compute()
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter*
hdrl_resample_parameter_create_outgrid2D(const double delta_ra,
		const double delta_dec)
{

  hdrl_resample_outgrid_parameter * p = (hdrl_resample_outgrid_parameter *)
	      hdrl_parameter_new(&hdrl_resample_outgrid_parameter_type);
  p->method = HDRL_RESAMPLE_OUTGRID_2D;
  p->delta_ra = delta_ra;
  p->delta_dec = delta_dec;

  p->recalc_limits = CPL_TRUE;

  /* This function asks to recalculate the limits in the hdrl_resample_compute
   * function - therefore we put dummy values for the moment. */

  p->dec_min = 0.1;
  p->dec_max = 0.2;
  p->ra_min = 0.1;
  p->ra_max = 0.2;

  /* in case of 2D sets some defaults dummy values for 3rd dimension */
  p->lambda_min = 0;
  p->lambda_max = 0;
  p->delta_lambda = 1;

  /* Default field margin in percent taken from swarp. */
  p->fieldmargin = FIELDMARGIN;

  p->wcs = NULL;

  if (hdrl_resample_parameter_outgrid_verify((hdrl_parameter*)p) != CPL_ERROR_NONE) {
      cpl_free(p);
      return NULL;
  }
  return (hdrl_parameter *)p;

}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_resample_parameter_outgrid

  @brief    Creates a resample_outgrid hdrl parameter object for a 3 dimensional
            interpolation, i.e HDRL_RESAMPLE_OUTGRID_3D. Only three values can
            be set by the caller. The remaining values (see
            hdrl_resample_parameter_create_outgrid3D_userdef() for all values)
            are derived from the data itself by the hdrl_resample_compute()
            function.
  @param    delta_ra     step size in right ascension [deg]
  @param    delta_dec    step size in declination [deg]
  @param    delta_lambda step size in wavelength direction [m]
  @return   The HDRL_RESAMPLE_OUTGRID_3D parameters object or NULL on error.
            It needs to be deallocated with hdrl_parameter_delete().

  This function creates a hdrl_resample_parameter_outgrid object
  HDRL_RESAMPLE_OUTGRID_3D

  @see      hdrl_parameter_delete()
  @see      hdrl_resample_compute()
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter*
hdrl_resample_parameter_create_outgrid3D(const double delta_ra,
		const double delta_dec, const double delta_lambda)
{

  hdrl_resample_outgrid_parameter * p = (hdrl_resample_outgrid_parameter *)
	  hdrl_parameter_new(&hdrl_resample_outgrid_parameter_type);
  p->method = HDRL_RESAMPLE_OUTGRID_3D;
  p->delta_ra = delta_ra;
  p->delta_dec = delta_dec;
  p->delta_lambda = delta_lambda;

  p->recalc_limits = CPL_TRUE;
  /* This function asks to recalculate the limits in the hdrl_resample_compute
   * function - therefore we put dummy values for the moment. */

  p->dec_min = 0.1;
  p->dec_max = 0.2;
  p->ra_min = 0.1;
  p->ra_max = 0.2;
  p->lambda_min = 0.;
  p->lambda_max = 0.;

  /* Default field margin in percent taken from swarp. */
  p->fieldmargin = FIELDMARGIN;

  p->wcs = NULL;

  if (hdrl_resample_parameter_outgrid_verify((hdrl_parameter*)p) != CPL_ERROR_NONE) {
      cpl_free(p);
      return NULL;
  }
  return (hdrl_parameter *)p;

}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_resample_parameter_outgrid

  @brief    Creates a resample_outgrid hdrl parameter object for a 2 dimensional
            interpolation, i.e HDRL_RESAMPLE_OUTGRID_2D. All values must be set
            by the caller (see also hdrl_resample_parameter_create_outgrid2D()) .
  @param    delta_ra     step size in right ascension [deg]
  @param    delta_dec    step size in declination [deg]
  @param    ra_min       minimum right ascension [deg]
  @param    ra_max       maximum right ascension [deg]
  @param    dec_min      minimum declination [deg]
  @param    dec_max      maximum declination [deg]
  @param    fieldmargin additional field margin [percent]
  @return   The HDRL_RESAMPLE_OUTGRID_2D parameter object or NULL on error.
            It needs to be deallocated with hdrl_parameter_delete().

  This function creates a hdrl_resample_parameter_outgrid object
  HDRL_RESAMPLE_OUTGRID_2D

  @see      hdrl_parameter_delete()
  @see      hdrl_resample_compute()
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter*
hdrl_resample_parameter_create_outgrid2D_userdef(const double delta_ra,
						 const double delta_dec,
						 const double ra_min,
						 const double ra_max,
						 const double dec_min,
						 const double dec_max,
						 const double fieldmargin)
{

  hdrl_resample_outgrid_parameter * p = (hdrl_resample_outgrid_parameter *)
	  hdrl_parameter_new(&hdrl_resample_outgrid_parameter_type);
  p->method = HDRL_RESAMPLE_OUTGRID_2D;
  p->delta_ra = delta_ra;
  p->delta_dec = delta_dec;


  p->recalc_limits = CPL_FALSE; /*This function takes the limits from the user*/
  p->dec_min = dec_min;
  p->dec_max = dec_max;
  p->ra_min = ra_min;
  p->ra_max = ra_max;

  /* in case of 2D sets some defaults dummy values for 3rd dimension */
  p->lambda_min = 0.;
  p->lambda_max = 0.;
  p->delta_lambda = 1.;

  p->fieldmargin = fieldmargin;

  p->wcs = NULL;

  if (hdrl_resample_parameter_outgrid_verify((hdrl_parameter*)p) != CPL_ERROR_NONE) {
      cpl_free(p);
      return NULL;
  }
  return (hdrl_parameter *)p;

}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_resample_parameter_outgrid

   @brief   Creates a resample_outgrid hdrl parameter object for a 3 dimensional
            interpolation, i.e HDRL_RESAMPLE_OUTGRID_3D. All values must be set
            by the caller (see also hdrl_resample_parameter_create_outgrid3D()).
  @param    delta_ra     step size in right ascension [deg]
  @param    delta_dec    step size in declination [deg]
  @param    delta_lambda step size in wavelength direction [m]
  @param    ra_min       minimum right ascension [deg]
  @param    ra_max       maximum right ascension [deg]
  @param    dec_min      minimum declination [deg]
  @param    dec_max      maximum declination [deg]
  @param    lambda_min   step size in right ascension [m]
  @param    lambda_max   step size in declination [m]
  @param    fieldmargin  additional field margin [percent]


  @return   The HDRL_RESAMPLE_OUTGRID_3D parameter object or NULL on error.
            It needs to be deallocated with hdrl_parameter_delete().

  This function creates a hdrl_resample_parameter_outgrid object
  HDRL_RESAMPLE_OUTGRID_3D

  @see      hdrl_parameter_delete()
  @see      hdrl_resample_compute()
 */
/*----------------------------------------------------------------------------*/
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
						 const double fieldmargin)
{

  hdrl_resample_outgrid_parameter * p = (hdrl_resample_outgrid_parameter *)
      hdrl_parameter_new(&hdrl_resample_outgrid_parameter_type);
  p->method = HDRL_RESAMPLE_OUTGRID_3D;
  p->delta_ra = delta_ra;
  p->delta_dec = delta_dec;
  p->delta_lambda = delta_lambda;

  p->recalc_limits = CPL_FALSE; /*This function takes the limits from the user*/

  p->dec_min = dec_min;
  p->dec_max = dec_max;
  p->ra_min = ra_min;
  p->ra_max = ra_max;

  p->lambda_min = lambda_min;
  p->lambda_max = lambda_max;

  p->fieldmargin = fieldmargin;

  p->wcs = NULL;

  if (hdrl_resample_parameter_outgrid_verify((hdrl_parameter*)p) != CPL_ERROR_NONE) {
      cpl_free(p);
      return NULL;
  }
  return (hdrl_parameter *)p;

}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_resample_parameter_method

  @brief    Creates a resample renka hdrl parameter object. The algorithm uses a
            modified Shepard-like distance weighting function following Renka
            for the interpolation.
  @param    loop_distance         loop distance to take into account surrounding
                                  pixels when interpolating on the final
                                  grid  [pixel]
  @param    use_errorweights      use additional weights based on 1/err^2 when
                                  interpolating [TRUE/FALSE]
  @param    critical_radius       critical radius beyond which the weight
                                  function returns 0      [pixel]

  @note in general a pixel is a voxel
  @return   The hdrl parameter object or NULL on error.
            It needs to be deallocated with hdrl_parameter_delete().

  @see      hdrl_parameter_delete()
  @see      hdrl_resample_compute()
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter*
hdrl_resample_parameter_create_renka(const int loop_distance,
				     cpl_boolean use_errorweights,
				     const double critical_radius)
{

  hdrl_resample_method_parameter * p = (hdrl_resample_method_parameter *)
      hdrl_parameter_new(&hdrl_resample_method_parameter_type);

  p->method = HDRL_RESAMPLE_METHOD_RENKA;
  p->loop_distance = loop_distance;
  p->use_errorweights = use_errorweights;
  p->renka_critical_radius = critical_radius;

  /* fill rest with dummy input */
  p->drizzle_pix_frac_x = 0.1;
  p->drizzle_pix_frac_y = 0.1;
  p->drizzle_pix_frac_lambda = 0.1;
  p->lanczos_kernel_size = 2;

  if (hdrl_resample_parameter_method_verify((hdrl_parameter*)p) != CPL_ERROR_NONE) {
      cpl_free(p);
      return NULL;
  }
  return (hdrl_parameter*) p;
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_resample_parameter_method

  @brief    Creates a resample linear hdrl parameter object. The algorithm uses
            a linear inverse distance weighting function for the interpolation.
  @param    loop_distance         loop distance to take into account surrounding
                                  pixels when interpolating on the final
                                  grid  [pixel]
  @param    use_errorweights      use additional weights based on 1/err^2 when
                                  interpolating [TRUE/FALSE]

  @note in general a pixel is a voxel
  @return   The hdrl parameter object or NULL on error.
            It needs to be deallocated with hdrl_parameter_delete().

  @see      hdrl_parameter_delete()
  @see      hdrl_resample_compute()
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter*
hdrl_resample_parameter_create_linear(const int loop_distance,
				      cpl_boolean use_errorweights)
{

  hdrl_resample_method_parameter * p = (hdrl_resample_method_parameter *)
      hdrl_parameter_new(&hdrl_resample_method_parameter_type);


  p->method = HDRL_RESAMPLE_METHOD_LINEAR;
  p->loop_distance = loop_distance;
  p->use_errorweights = use_errorweights;

  /* fill rest with dummy input */
  p->renka_critical_radius = 0.1;
  p->drizzle_pix_frac_x = 0.1;
  p->drizzle_pix_frac_y = 0.1;
  p->drizzle_pix_frac_lambda = 0.1;
  p->lanczos_kernel_size = 2;

  if (hdrl_resample_parameter_method_verify((hdrl_parameter*)p) != CPL_ERROR_NONE) {
      cpl_free(p);
      return NULL;
  }
  return (hdrl_parameter*) p;
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_resample_parameter_method

  @brief    Creates a resample quadratic hdrl parameter object. The algorithm
            uses a quadratic inverse distance weighting function for the
            interpolation.
  @param    loop_distance         loop distance to take into account surrounding
                                  pixels when interpolating on the final
                                  grid  [pixel]
  @param    use_errorweights      use additional weights based on 1/err^2 when
                                  interpolating [TRUE/FALSE]

  @note in general a pixel is a voxel
  @return   The hdrl parameter object or NULL on error.
            It needs to be deallocated with hdrl_parameter_delete().

  @see      hdrl_parameter_delete()
  @see      hdrl_resample_compute()
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter*
hdrl_resample_parameter_create_quadratic(const int loop_distance,
					 cpl_boolean use_errorweights)
{

  hdrl_resample_method_parameter * p = (hdrl_resample_method_parameter *)
      hdrl_parameter_new(&hdrl_resample_method_parameter_type);

  p->method = HDRL_RESAMPLE_METHOD_QUADRATIC;
  p->loop_distance = loop_distance;
  p->use_errorweights = use_errorweights;

  /* fill rest with dummy input */
  p->renka_critical_radius = 0.1;
  p->drizzle_pix_frac_x = 0.1;
  p->drizzle_pix_frac_y = 0.1;
  p->drizzle_pix_frac_lambda = 0.1;
  p->lanczos_kernel_size = 2;

  if (hdrl_resample_parameter_method_verify((hdrl_parameter*)p) != CPL_ERROR_NONE) {
      cpl_free(p);
      return NULL;
  }
  return (hdrl_parameter*) p;
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_resample_parameter_method

  @brief Creates a resample nearest neighbor hdrl parameter object.
         The algorithm does not use any weighting functions but the nearest
         neighbor inside a voxel for the "interpolation". If there is no nearest
         neighbor inside the voxel but only outside, the voxel is marked as bad.

  @return  The hdrl parameter object or NULL on error.
           It needs to be deallocated with hdrl_parameter_delete().

  @see     hdrl_parameter_delete()
  @see     hdrl_resample_compute()
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter*
hdrl_resample_parameter_create_nearest(void)
{

  hdrl_resample_method_parameter * p = (hdrl_resample_method_parameter *)
      hdrl_parameter_new(&hdrl_resample_method_parameter_type);


  p->method = HDRL_RESAMPLE_METHOD_NEAREST;
  p->loop_distance = 0;
  p->use_errorweights = CPL_FALSE;

  /* fill rest with dummy input */
  p->renka_critical_radius = 0.1;
  p->drizzle_pix_frac_x = 0.1;
  p->drizzle_pix_frac_y = 0.1;
  p->drizzle_pix_frac_lambda = 0.1;
  p->lanczos_kernel_size = 2;

  if (hdrl_resample_parameter_method_verify((hdrl_parameter*)p) != CPL_ERROR_NONE) {
      cpl_free(p);
      return NULL;
  }
  return (hdrl_parameter*) p;
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_resample_parameter_method

  @brief    Creates a resample Lanczos hdrl parameter object. The algorithm
            uses a restricted SINC distance weighting function for the
            interpolation.
  @param    loop_distance         loop distance to take into account surrounding
                                  pixels when interpolating on the final
                                  grid  [pixel]
  @param    use_errorweights      use additional weights based on 1/err^2 when
                                  interpolating [TRUE/FALSE]
  @param    kernel_size           kernel size of the sinc() function [pixel]

  @note in general a pixel is a voxel
  @return   The hdrl parameter object or NULL on error.
            It needs to be deallocated with hdrl_parameter_delete().

  @see      hdrl_parameter_delete()
  @see      hdrl_resample_compute()
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter*
hdrl_resample_parameter_create_lanczos(const int loop_distance,
				       cpl_boolean use_errorweights,
				       const int kernel_size)
{

  hdrl_resample_method_parameter * p = (hdrl_resample_method_parameter *)
      hdrl_parameter_new(&hdrl_resample_method_parameter_type);

  p->method = HDRL_RESAMPLE_METHOD_LANCZOS;
  p->loop_distance = loop_distance;
  p->use_errorweights = use_errorweights;
  p->lanczos_kernel_size = kernel_size;
  /* fill rest with dummy input */
  p->renka_critical_radius = 0.1;
  p->drizzle_pix_frac_x = 0.1;
  p->drizzle_pix_frac_y = 0.1;
  p->drizzle_pix_frac_lambda = 0.1;

  if (hdrl_resample_parameter_method_verify((hdrl_parameter*)p) != CPL_ERROR_NONE) {
      cpl_free(p);
      return NULL;
  }
  return (hdrl_parameter*) p;
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_resample_parameter_method

  @brief    Creates a resample drizzle hdrl parameter object. The algorithm
            uses a drizzle-like distance weighting function for the
            interpolation.
  @param    loop_distance         loop distance to take into account surrounding
                                  pixels when interpolating on the final
                                  grid  [pixel]
  @param    use_errorweights      use additional weights based on 1/err^2 when
                                  interpolating [TRUE/FALSE]

  @param    pix_frac_x            pixfrac-scaled input pixel size in x direction
                                  [pixel]
  @param    pix_frac_y            pixfrac-scaled input pixel size in y direction
                                  [pixel]
  @param    pix_frac_lambda       pixfrac-scaled input pixel size in
                                  wavelength direction [pixel]

  @note in general a pixel is a voxel
  @return   The hdrl parameter object or NULL on error.
            It needs to be deallocated with hdrl_parameter_delete().


  @see      hdrl_parameter_delete()
  @see      hdrl_resample_compute()
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter*
hdrl_resample_parameter_create_drizzle(const int loop_distance,
				       cpl_boolean use_errorweights,
				       const double pix_frac_x,
				       const double pix_frac_y,
				       const double pix_frac_lambda)
{

  hdrl_resample_method_parameter * p = (hdrl_resample_method_parameter *)
      hdrl_parameter_new(&hdrl_resample_method_parameter_type);

  p->method = HDRL_RESAMPLE_METHOD_DRIZZLE;
  p->loop_distance = loop_distance;
  p->use_errorweights = use_errorweights;
  p->drizzle_pix_frac_x = pix_frac_x;
  p->drizzle_pix_frac_y = pix_frac_y;
  p->drizzle_pix_frac_lambda = pix_frac_lambda;

  /* fill rest with dummy input */
  p->renka_critical_radius = 0.1;
  p->lanczos_kernel_size = 2;

  if (hdrl_resample_parameter_method_verify((hdrl_parameter*)p) != CPL_ERROR_NONE) {
      cpl_free(p);
      return NULL;
  }
  return (hdrl_parameter*) p;

}
/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_resample_parameter_outgrid

  @brief   verify parameters have proper values
  @param    hp       hdrl_parameter
  @return   cpl_error_code

 */
/*----------------------------------------------------------------------------*/
cpl_error_code
hdrl_resample_parameter_outgrid_verify(const hdrl_parameter * hp){

  const hdrl_resample_outgrid_parameter * param_loc =
      (const hdrl_resample_outgrid_parameter *)hp ;

  cpl_error_ensure(hp != NULL, CPL_ERROR_NULL_INPUT,
		   return CPL_ERROR_NULL_INPUT, "NULL Input Parameters");

  cpl_error_ensure(hdrl_resample_parameter_outgrid_check(hp),
		   CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
		       "Here we expect a resample outgrid parameter") ;

  cpl_error_ensure(
      param_loc->recalc_limits == CPL_TRUE ||
      param_loc->recalc_limits == CPL_FALSE,
      CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
	  "Unsupported resample recalc_limits value");

/*
 * The wcs is filled later on by the compute function so it can not be checked
 * at this stage
 *
  cpl_error_ensure(param_loc->wcs != NULL, CPL_ERROR_NULL_INPUT,
		   return CPL_ERROR_NULL_INPUT, "WCS must be defined");
*/


  cpl_error_ensure(param_loc->delta_ra > 0, CPL_ERROR_ILLEGAL_INPUT,
		   return CPL_ERROR_ILLEGAL_INPUT, "right ascension "
		       "stepsize must be > 0");

  cpl_error_ensure(param_loc->delta_dec > 0, CPL_ERROR_ILLEGAL_INPUT,
		   return CPL_ERROR_ILLEGAL_INPUT, "declination stepsize "
		       "must be > 0");

  cpl_error_ensure(param_loc->delta_lambda > 0, CPL_ERROR_ILLEGAL_INPUT,
		   return CPL_ERROR_ILLEGAL_INPUT, "wavelength stepsize "
		       "must be > 0");


  cpl_error_ensure(param_loc->ra_min >= 0,
		   CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
		       "Minimum right ascension must be >= 0");

  cpl_error_ensure(param_loc->ra_max >= 0,
		   CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
		       "Maximum right ascension must be >= 0");


  cpl_error_ensure(param_loc->lambda_min >= 0,
		   CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
		       "Minimum wavelength must be >= 0");

  cpl_error_ensure(param_loc->lambda_max >= 0,
		   CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
		       "Maximum wavelength must be >= 0");

  cpl_error_ensure(param_loc->fieldmargin >= 0., CPL_ERROR_ILLEGAL_INPUT,
		   return CPL_ERROR_ILLEGAL_INPUT, "The field margin must"
		       " be >= 0.");

  cpl_error_ensure(param_loc->ra_max >= param_loc->ra_min ,
		   CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
		       "The maximum right ascension must be >= "
		       "the minimum right ascension");
  cpl_error_ensure(param_loc->dec_max >= param_loc->dec_min ,
		   CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
		       "The maximum declination must be >= "
		       "the minimum declination");

  cpl_error_ensure(param_loc->lambda_max >= param_loc->lambda_min ,
 		   CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
 		       "The maximum wavelength must be >= "
 		       "the minimum wavelength");

  return CPL_ERROR_NONE ;
}

/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_resample_parameter_method

  @brief   verify parameters have proper values
  @param    hp       hdrl_parameter
  @return   cpl_error_code

 */
/*----------------------------------------------------------------------------*/
cpl_error_code
hdrl_resample_parameter_method_verify(const hdrl_parameter * hp){

  const hdrl_resample_method_parameter * param_loc =
      (const hdrl_resample_method_parameter *)hp ;
  cpl_error_ensure(hp != NULL, CPL_ERROR_NULL_INPUT,
		   return CPL_ERROR_NULL_INPUT, "NULL Input Parameters");

  cpl_error_ensure(hdrl_resample_parameter_method_check(hp),
		   CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
		       "Here we expect a resample method parameter") ;

  /* checks on parameter methods */
  cpl_error_ensure(
      param_loc->method == HDRL_RESAMPLE_METHOD_NEAREST ||
      param_loc->method == HDRL_RESAMPLE_METHOD_LINEAR ||
      param_loc->method == HDRL_RESAMPLE_METHOD_QUADRATIC ||
      param_loc->method == HDRL_RESAMPLE_METHOD_LANCZOS ||
      param_loc->method == HDRL_RESAMPLE_METHOD_DRIZZLE ||
      param_loc->method == HDRL_RESAMPLE_METHOD_RENKA,
      CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
	  "Unsupported resample method");

  /* checks on common parameter elements */
  cpl_error_ensure(param_loc->loop_distance >= 0, CPL_ERROR_ILLEGAL_INPUT,
		   return CPL_ERROR_ILLEGAL_INPUT, "The loop distance must "
		       "be >=0");

  cpl_error_ensure(param_loc->use_errorweights == CPL_TRUE ||
		   param_loc->use_errorweights == CPL_FALSE,
		   CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
		       "Unsupported resample use_errorweights value");

  switch (param_loc->method) {
    case HDRL_RESAMPLE_METHOD_NEAREST:
    case HDRL_RESAMPLE_METHOD_LINEAR:
    case HDRL_RESAMPLE_METHOD_QUADRATIC:
    case HDRL_RESAMPLE_METHOD_NONE:
      break;
    case HDRL_RESAMPLE_METHOD_RENKA:
      /* checks on peculiar parameter elements */
      cpl_error_ensure(param_loc->renka_critical_radius > 0.,
		       CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
			   "Critical radius of the Renka method must be > 0");
      break ;

    case HDRL_RESAMPLE_METHOD_DRIZZLE:
      /* checks on peculiar parameter elements */
      cpl_error_ensure(param_loc->drizzle_pix_frac_x > 0.,
		       CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
			   "Drizzle down-scaling factor in x direction "
			   "must be > 0");

      cpl_error_ensure(param_loc->drizzle_pix_frac_y > 0.,
		       CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
			   "Drizzle down-scaling factor in y direction "
			   "must be > 0");

      cpl_error_ensure(param_loc->drizzle_pix_frac_lambda > 0.,
		       CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
			   "Drizzle down-scaling factor in z/lambda direction "
			   "must be > 0");
      break;

    case HDRL_RESAMPLE_METHOD_LANCZOS:
      /* checks on peculiar parameter elements */
      cpl_error_ensure(param_loc->lanczos_kernel_size > 0,
		       CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
			   "The kernel size of the Lanczos method must be > 0");
      break;
  }

  return CPL_ERROR_NONE ;
}
/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_resample_parameter_outgrid

  @brief   check method is of proper type
  @param   self       hdrl_parameter
  @return   cpl_error_code

 */
/*----------------------------------------------------------------------------*/
int
hdrl_resample_parameter_outgrid_check(const hdrl_parameter * self){
  /* Check if the method is of proper type */
  cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);

  return hdrl_parameter_check_type(self, &hdrl_resample_outgrid_parameter_type);
}
/*----------------------------------------------------------------------------*/
/**
  @ingroup hdrl_resample_parameter_method

  @brief   check method is of proper type
  @param   self       hdrl_parameter
  @return   cpl_error_code

 */
/*----------------------------------------------------------------------------*/
int
hdrl_resample_parameter_method_check(const hdrl_parameter * self){
  /* Check if the method is of proper type */
  cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);
  return hdrl_parameter_check_type(self, &hdrl_resample_method_parameter_type);
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Verifies the existence and format of the resampling table columns
  @param    ResTable    Resampling table passed to hdrl_resample_compute()

  @return   The cpl error code
 */
/*----------------------------------------------------------------------------*/
static cpl_error_code
hdrl_resample_inputtable_verify(const cpl_table *ResTable){

  cpl_error_ensure(ResTable != NULL, CPL_ERROR_NULL_INPUT,
		   return CPL_ERROR_NULL_INPUT, "No Table as input");

  /* Check the existence of all columns */
  cpl_error_ensure(cpl_table_has_column(ResTable, HDRL_RESAMPLE_TABLE_DATA  )
		   == 1, CPL_ERROR_INCOMPATIBLE_INPUT,
		   return CPL_ERROR_INCOMPATIBLE_INPUT,
		       "Missing data table column");
  cpl_error_ensure(cpl_table_has_column(ResTable, HDRL_RESAMPLE_TABLE_BPM   )
		   == 1, CPL_ERROR_INCOMPATIBLE_INPUT,
		   return CPL_ERROR_INCOMPATIBLE_INPUT,
		       "Missing bpm table column");
  cpl_error_ensure(cpl_table_has_column(ResTable, HDRL_RESAMPLE_TABLE_ERRORS)
		   == 1, CPL_ERROR_INCOMPATIBLE_INPUT,
		   return CPL_ERROR_INCOMPATIBLE_INPUT,
		       "Missing error table column");
  cpl_error_ensure(cpl_table_has_column(ResTable, HDRL_RESAMPLE_TABLE_RA    )
		   == 1, CPL_ERROR_INCOMPATIBLE_INPUT,
		   return CPL_ERROR_INCOMPATIBLE_INPUT,
		       "Missing right ascension table column");
  cpl_error_ensure(cpl_table_has_column(ResTable, HDRL_RESAMPLE_TABLE_DEC   )
		   == 1, CPL_ERROR_INCOMPATIBLE_INPUT,
		   return CPL_ERROR_INCOMPATIBLE_INPUT,
		       "Missing declination table column");
  cpl_error_ensure(cpl_table_has_column(ResTable, HDRL_RESAMPLE_TABLE_LAMBDA)
		   == 1, CPL_ERROR_INCOMPATIBLE_INPUT,
		   return CPL_ERROR_INCOMPATIBLE_INPUT,
		       "Missing wavelength table column");

  /* Check the format of all columns */
  cpl_error_ensure(cpl_table_get_column_type(ResTable, HDRL_RESAMPLE_TABLE_DATA  )
		   == HDRL_RESAMPLE_TABLE_DATA_TYPE, CPL_ERROR_INCOMPATIBLE_INPUT,
		   return CPL_ERROR_INCOMPATIBLE_INPUT,
		       "Data table column has wrong format");
  cpl_error_ensure(cpl_table_get_column_type(ResTable, HDRL_RESAMPLE_TABLE_BPM   )
		   == HDRL_RESAMPLE_TABLE_BPM_TYPE,    CPL_ERROR_INCOMPATIBLE_INPUT,
		   return CPL_ERROR_INCOMPATIBLE_INPUT,
		       "Bpm table column has wrong format");
  cpl_error_ensure(cpl_table_get_column_type(ResTable, HDRL_RESAMPLE_TABLE_ERRORS)
		   == HDRL_RESAMPLE_TABLE_ERRORS_TYPE, CPL_ERROR_INCOMPATIBLE_INPUT,
		   return CPL_ERROR_INCOMPATIBLE_INPUT,
		       "Error table column has wrong format");
  cpl_error_ensure(cpl_table_get_column_type(ResTable, HDRL_RESAMPLE_TABLE_RA    )
		   == HDRL_RESAMPLE_TABLE_RA_TYPE, CPL_ERROR_INCOMPATIBLE_INPUT,
		   return CPL_ERROR_INCOMPATIBLE_INPUT,
		       "Right ascension table column has wrong format");
  cpl_error_ensure(cpl_table_get_column_type(ResTable, HDRL_RESAMPLE_TABLE_DEC   )
		   == HDRL_RESAMPLE_TABLE_DEC_TYPE, CPL_ERROR_INCOMPATIBLE_INPUT,
		   return CPL_ERROR_INCOMPATIBLE_INPUT,
		       "Declination table column has wrong format");
  cpl_error_ensure(cpl_table_get_column_type(ResTable, HDRL_RESAMPLE_TABLE_LAMBDA)
		   == HDRL_RESAMPLE_TABLE_LAMBDA_TYPE, CPL_ERROR_INCOMPATIBLE_INPUT,
		   return CPL_ERROR_INCOMPATIBLE_INPUT,
		       "Wavelength table column has wrong format");

  return cpl_error_get_code();
}
/**@} */
/* EOF */

