/*
 * This file is part of the HDRL 
 * Copyright (C) 2012,2013 European Southern Observatory
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
#if !defined(_XOPEN_SOURCE) || (_XOPEN_SOURCE - 0) < 500
#define _XOPEN_SOURCE 500 /* posix 2001, mkstemp */
#endif

#include "hdrl_utils.h"
#include "hdrl_types.h"
#include "hdrl_elemop.h"
#include "hdrl_prototyping.h"
#include "hdrl_imagelist.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>

/*-----------------------------------------------------------------------------
                                   Static
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_utils   General Utility Functions
  
  This module contain various utilities functions that might be used in several
  of the HDRL modules.
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Get the pipeline copyright and license
  @return   The copyright and license string

  The function returns a pointer to the statically allocated license string.
  This string should not be modified using the returned pointer.
 */
/*----------------------------------------------------------------------------*/
const char * hdrl_get_license(void)
{
    static const char  *   hdrl_license =
        "This file is part of the HDRL Instrument Pipeline\n"
        "Copyright (C) 2012,2013 European Southern Observatory\n"
        "\n"
        "This program is free software; you can redistribute it and/or modify\n"
        "it under the terms of the GNU General Public License as published by\n"
        "the Free Software Foundation; either version 2 of the License, or\n"
        "(at your option) any later version.\n"
        "\n"
        "This program is distributed in the hope that it will be useful,\n"
        "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
        "GNU General Public License for more details.\n"
        "\n"
        "You should have received a copy of the GNU General Public License\n"
        "along with this program; if not, write to the Free Software\n"
        "Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, \n"
        "MA  02110-1301  USA";

    return hdrl_license;
}

#define EOP_LINE_SIZE 188

/*----------------------------------------------------------------------------*/
/**
 * @brief    Export a raw string buffer containing EOP data to a CPL table
 *
 * @param    eop_data     The string buffer with the data
 * @param    data_length  The total size of the data buffer
 * @return   A string buffer with the full contents of the data
 *
 * This function convert the ascii file retrieve from the internet
 * and convert it to a CPL table.
 *
 * Possible cpl-error-code set in this function:
 * - CPL_ERROR_NULL_INPUT if eop_data is NULL
 * - CPL_ERROR_NULL_INPUT if data_length doesn't correspond with the expected
 *   EOP records length
 */
/*----------------------------------------------------------------------------*/

cpl_table * hdrl_eop_data_totable (const char * eop_data, cpl_size data_length)
{
  cpl_ensure (eop_data, CPL_ERROR_NULL_INPUT, NULL);

  if(!((data_length) % EOP_LINE_SIZE == 0))
    {
      cpl_error_set_message(cpl_func, CPL_ERROR_NULL_INPUT,
			    "Raw data doesn't have a fixed record width");
      return 0;
    }

  /* Create tables */
  cpl_size n_entries = (data_length) / EOP_LINE_SIZE;
  cpl_table * eop_table = cpl_table_new (n_entries);
  cpl_msg_info (cpl_func, " EOP data has a total of %"CPL_SIZE_FORMAT" entries", n_entries);

  /* Create columns */
  cpl_table_new_column (eop_table, "MJD",  CPL_TYPE_DOUBLE);
  cpl_table_new_column (eop_table, "PMX",  CPL_TYPE_DOUBLE);
  cpl_table_new_column (eop_table, "PMY",  CPL_TYPE_DOUBLE);
  cpl_table_new_column (eop_table, "DUT",  CPL_TYPE_DOUBLE);
  cpl_table_new_column (eop_table, "FLAG", CPL_TYPE_STRING);

  /* Set units */
  cpl_table_set_column_unit (eop_table, "MJD", "d");
  cpl_table_set_column_unit (eop_table, "PMX", "arcsec");
  cpl_table_set_column_unit (eop_table, "PMY", "arcsec");
  cpl_table_set_column_unit (eop_table, "DUT", "s");

  /* Fill the columns from the string buffer */
  for(cpl_size i=0; i<n_entries; i++)
    {
      char flag[2];
      strncpy(flag, eop_data+i*EOP_LINE_SIZE+16, 1);
      flag[1] = '\0';
      cpl_table_set_string(eop_table, "FLAG", i, flag);

      cpl_table_set_double(eop_table, "MJD", i, atof(eop_data+i*EOP_LINE_SIZE+7));
      if(!strncmp(flag, "I", 1) || !strncmp(flag, "P", 1))
	{
	  cpl_table_set_double(eop_table, "PMX", i, atof(eop_data+i*EOP_LINE_SIZE+18));
	  cpl_table_set_double(eop_table, "PMY", i, atof(eop_data+i*EOP_LINE_SIZE+37));
	  cpl_table_set_double(eop_table, "DUT", i, atof(eop_data+i*EOP_LINE_SIZE+58));
	}
    }

  /* Remove the NULL columns */
  cpl_table_unselect_all (eop_table);
  cpl_table_or_selected_invalid (eop_table, "PMX");
  cpl_table_or_selected_invalid (eop_table, "PMY");
  cpl_table_or_selected_invalid (eop_table, "DUT");
  cpl_msg_info (cpl_func,"Found %lld invalid", cpl_table_count_selected (eop_table));
  cpl_table_erase_selected (eop_table);

  return eop_table;
}


/*-----------------------------------------------------------------------------
                Rectangular Region Parameters Definition
 -----------------------------------------------------------------------------*/

/** @cond PRIVATE */
typedef struct {
    HDRL_PARAMETER_HEAD;
    cpl_size    llx ;
    cpl_size    lly ;
    cpl_size    urx ;
    cpl_size    ury ;
} hdrl_rect_region_parameter;

/* Parameter type */
static hdrl_parameter_typeobj hdrl_rect_region_parameter_type = {
    HDRL_PARAMETER_RECT_REGION,     /* type */
    (hdrl_alloc *)&cpl_malloc,      /* fp_alloc */
    (hdrl_free *)&cpl_free,         /* fp_free */
    NULL,                           /* fp_destroy */
    sizeof(hdrl_rect_region_parameter), /* obj_size */
};


/*----------------------------------------------------------------------------*/
/**
  @brief   Generates a 2D Gaussian Kernel of sizes kernel_sx, kernel_sy
  @param   kernel_sx   X kernel size
  @param   kernel_sy   Y kernel size
  @param   fwhm the FWHM seeing

  @return  The corresponding CPL matrix object or NULL in case of error
  The returned matrix must be deallocated using cpl_matrix_delete().
  The user must provide kernel_sx > 0, kernel_sy > 0, fwhm > 0
 */
/*----------------------------------------------------------------------------*/

cpl_matrix *
hdrl_maglim_kernel_create(const cpl_size kernel_sx, const cpl_size kernel_sy,
		          const double fwhm){

  cpl_ensure(kernel_sx > 0, CPL_ERROR_ILLEGAL_INPUT, NULL);
  cpl_ensure(kernel_sy > 0, CPL_ERROR_ILLEGAL_INPUT, NULL);
  cpl_ensure(fwhm > 0, CPL_ERROR_ILLEGAL_INPUT, NULL);

  cpl_matrix * kernel = cpl_matrix_new(kernel_sx, kernel_sy);

  double* pkernel = cpl_matrix_get_data(kernel);

  double arg_exp;
  double x;
  double y;

  double sigma_to_fwhm = sqrt( 4. * log(4) );  //log=natural log; log10 = bas 10

  double factor = (fwhm / sigma_to_fwhm);
  factor *= factor;
  factor = 2.0 * factor ;

  //double sum = 0.;
  double dx = 1;
  double dy = 1;

  for(cpl_size j = 0; j < kernel_sy; j++) {
      y =  j * dy - 0.5 * (kernel_sy - 1);

      for(cpl_size i = 0; i < kernel_sx; i++) {
	  x = i * dx - 0.5 * (kernel_sx - 1);

	  arg_exp = (x * x + y * y) / factor;
	  pkernel[kernel_sx * j + i] = exp( -arg_exp );

      }
  }

  return kernel;
}



/**
  @brief Increase the original image size by the kernel size propagating pixel
  quality

  @param image input image
  @param border_nx   X border size
  @param border_ny   Y border size
  @param image_extend_method      method used to extend the image
  @return the extended image or NULL in case of error

  The returned image must be deallocated using cpl_image_delete().
  The algorithm increases the image by the kernel size propagating pixel quality
  It applies on the extended image the operation and extract the original-size
  image at the very end.
  Possible method values are HDRL_IMAGE_EXTEND_NEAREST, HDRL_IMAGE_EXTEND_MIRROR
  The user must provide image != NULL, border_nx > 0, border_ny > 0.
 */
cpl_image*
hdrl_extend_image(const cpl_image* image, const cpl_size border_nx,
		  const cpl_size border_ny,
		  const hdrl_image_extend_method image_extend_method)
{
  cpl_ensure(image != NULL, CPL_ERROR_NULL_INPUT, NULL);
  cpl_ensure(border_nx > 0, CPL_ERROR_ILLEGAL_INPUT, NULL);
  cpl_ensure(border_ny > 0, CPL_ERROR_ILLEGAL_INPUT, NULL);

  cpl_ensure(image_extend_method == HDRL_IMAGE_EXTEND_NEAREST ||
	     image_extend_method == HDRL_IMAGE_EXTEND_MIRROR, CPL_ERROR_ILLEGAL_INPUT, NULL);

  cpl_type type = cpl_image_get_type(image);
  cpl_size input_sx = cpl_image_get_size_x(image);
  cpl_size input_sy = cpl_image_get_size_y(image);

  cpl_ensure(2 * border_nx <= input_sx, CPL_ERROR_ILLEGAL_INPUT, NULL);
  cpl_ensure(2 * border_ny <= input_sy, CPL_ERROR_ILLEGAL_INPUT, NULL);

  cpl_size output_sx = input_sx + 2 * border_nx;
  cpl_size output_sy = input_sy + 2 * border_ny;

  cpl_msg_debug(cpl_func, "Extend image:");
  cpl_msg_indent_more();
  cpl_msg_debug(cpl_func, "Border sizes (x, y): (%lld, %lld)", border_nx, border_ny);
  cpl_msg_debug(cpl_func, "Input  image (x, y): (%lld, %lld)", input_sx, input_sy);
  cpl_msg_debug(cpl_func, "Output image (x, y): (%lld, %lld)", output_sx, output_sy);
  cpl_msg_indent_less();

  cpl_image* output_image = cpl_image_new(output_sx, output_sy, type);
  double value = 0;
  int status = 0;

  if ( image_extend_method == HDRL_IMAGE_EXTEND_NEAREST ) {

      /* copy central image into the extended image*/
      cpl_image_copy(output_image, image, border_nx + 1, border_ny + 1 );

      /* fill left-right values at position of central image */
      for(cpl_size j = 1; j <= input_sy; j++) {
	  /* left most tail */
	  value = cpl_image_get(image, 1, j, &status);
	  cpl_image_fill_window(output_image, 1, border_ny +j,
				border_nx, border_ny + j, value);
	  /* right most tail */
	  value = cpl_image_get(image, input_sx, j, &status);
	  cpl_image_fill_window(output_image, input_sx + border_nx, border_ny +j,
				output_sx, border_ny + j, value);
      }

      /* fill bottom-up values at any position */
      for(cpl_size i = 1; i <= output_sx; i++) {
	  /* lower part */
	  value = cpl_image_get(output_image, i, border_ny + 1, &status);
	  cpl_image_fill_window(output_image, i, 1, i, border_ny, value);

	  /* upper part */
	  value = cpl_image_get(output_image, i, border_ny + input_sy, &status);
	  cpl_image_fill_window(output_image, i, border_ny + input_sy, i,
				output_sy, value);
      }

  } else if ( image_extend_method == HDRL_IMAGE_EXTEND_MIRROR ) {

      /* Insert input image into expanded image */
      cpl_image_copy(output_image, image, border_nx + 1, border_ny + 1);

      /* do left */
      cpl_image* image_ex = cpl_image_extract(image, 1, 1, border_nx, input_sy);
      cpl_image_flip(image_ex, 2);
      cpl_image_copy(output_image, image_ex, 1, border_ny + 1);
      cpl_image_delete(image_ex);

      /* do right */
      image_ex = cpl_image_extract(image, input_sx- border_nx + 1, 1, input_sx,
				   input_sy);
      cpl_image_flip(image_ex, 2);
      cpl_image_copy(output_image, image_ex, input_sx + border_nx + 1,
		     border_ny + 1);
      cpl_image_delete(image_ex);

      /* do upper */
      image_ex = cpl_image_extract(output_image, 1, input_sy, output_sx,
				   input_sy + border_ny);
      cpl_image_flip(image_ex, 0);
      cpl_image_copy(output_image, image_ex, 1, input_sy + border_ny + 1);
      cpl_image_delete(image_ex);

      /* do lower */
      image_ex = cpl_image_extract(output_image, 1, border_ny + 1, output_sx,
				   2 * border_ny);
      cpl_image_flip(image_ex, 0);
      cpl_image_copy(output_image, image_ex, 1, 1);
      cpl_image_delete(image_ex);

  }

  return output_image;

}


/*----------------------------------------------------------------------------*/
/**
  @brief filter image
  @param input_image input image
  @param kernel      input kernel matrix
  @param image_extend_method to   expand image
  @return the filtered image or NULL in case of error

  The returned image must be deallocated using cpl_image_delete().
  The algorithm enlarges the image by the kernel size propagating the image bad
  pixel map. It applies on the enlarged image the operation and extract the
  original-size image at the very end.
  Possible method values are HDRL_IMAGE_EXTEND_NEAREST, HDRL_IMAGE_EXTEND_MIRROR
  The user must also provide input_image != NULL, kernel != NULL.
 */
/*----------------------------------------------------------------------------*/
cpl_image *
hdrl_image_convolve(const cpl_image * input_image, const cpl_matrix * kernel,
		    const hdrl_image_extend_method image_extend_method)
{
  cpl_image * expanded_image;
  cpl_image * filtered_image;
  cpl_image * expanded_filtered_image;

  /* Check Entries */
  cpl_ensure(input_image != NULL, CPL_ERROR_NULL_INPUT, NULL);
  cpl_ensure(kernel != NULL, CPL_ERROR_NULL_INPUT, NULL);
  cpl_ensure(image_extend_method == HDRL_IMAGE_EXTEND_NEAREST ||
  	     image_extend_method == HDRL_IMAGE_EXTEND_MIRROR, CPL_ERROR_ILLEGAL_INPUT, NULL);

  cpl_size        kernel_nx = cpl_matrix_get_ncol(kernel);
  cpl_size        kernel_ny = cpl_matrix_get_nrow(kernel);
  cpl_ensure(kernel_nx >= 1, CPL_ERROR_ILLEGAL_INPUT, NULL);
  cpl_ensure(kernel_ny >= 1, CPL_ERROR_ILLEGAL_INPUT, NULL);

  /* Only odd-sized masks allowed */
  cpl_ensure((kernel_nx&1) == 1, CPL_ERROR_ILLEGAL_INPUT, NULL);
  cpl_ensure((kernel_ny&1) == 1, CPL_ERROR_ILLEGAL_INPUT, NULL);

  const int border_nx = (int)((kernel_nx - 1)/2.);
  const int border_ny = (int)((kernel_ny - 1)/2.);

  expanded_image =
  hdrl_extend_image(input_image, border_nx, border_ny, image_extend_method);


  /* Use parallel filter as faster */
    expanded_filtered_image =
    hdrl_parallel_filter_image(expanded_image, kernel, NULL,
			       CPL_FILTER_LINEAR);


  if(expanded_filtered_image == NULL) {
      cpl_image_delete(expanded_filtered_image);
       cpl_image_delete(expanded_image);
       return NULL;
  }

  /* Extract the original mask from the expanded mask */
  cpl_msg_debug(cpl_func, "Extract original image from expanded mask, region "\
	       "[llx: %lld, lly: %lld, urx: %lld, ury: %lld",
	       kernel_nx+1, kernel_ny + 1,
	       cpl_image_get_size_x(input_image) + kernel_nx,
	       cpl_image_get_size_y(input_image) + kernel_ny);

  filtered_image = cpl_image_extract(expanded_filtered_image, border_nx + 1,
				     border_ny + 1,
				     cpl_image_get_size_x(input_image) + border_nx,
				     cpl_image_get_size_y(input_image) + border_ny);

  /* Free memory */
  cpl_image_delete(expanded_filtered_image);
  cpl_image_delete(expanded_image);

  return filtered_image;
}



/** @endcond */

/*----------------------------------------------------------------------------*/
/**
  @brief    Creates Rect Region Parameters object
  @param    llx
  @param    lly
  @param    urx
  @param    ury
  @return   the Rect Region parameters
 */
/*----------------------------------------------------------------------------*/
hdrl_parameter * hdrl_rect_region_parameter_create(
        cpl_size    llx,
        cpl_size    lly,
        cpl_size    urx,
        cpl_size    ury)
{
    hdrl_rect_region_parameter * p = (hdrl_rect_region_parameter *)
               hdrl_parameter_new(&hdrl_rect_region_parameter_type);
    p->llx = llx ;
    p->lly = lly ;
    p->urx = urx ;
    p->ury = ury ;
    return (hdrl_parameter *)p;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Update Rect Region Parameters object
  @param    rect_region     The region to update
  @param    llx
  @param    lly
  @param    urx
  @param    ury
  @return   the error code in case of error or CPL_ERROR_NONE
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_rect_region_parameter_update(
        hdrl_parameter  *   rect_region,
        cpl_size            llx,
        cpl_size            lly,
        cpl_size            urx,
        cpl_size            ury)
{
    hdrl_rect_region_parameter * p = (hdrl_rect_region_parameter *)rect_region ;
    p->llx = llx ;
    p->lly = lly ;
    p->urx = urx ;
    p->ury = ury ;
    return hdrl_rect_region_parameter_verify(rect_region, -1, -1);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Check that the parameter is hdrl_rect_region parameter
  @return   the parameter to check
 */
/*----------------------------------------------------------------------------*/
cpl_boolean hdrl_rect_region_parameter_check(const hdrl_parameter * self)
{
    return hdrl_parameter_check_type(self, &hdrl_rect_region_parameter_type);
}

/*-----------------------------------------------------------------------------
                        Rect Region Parameters Accessors
 -----------------------------------------------------------------------------*/

/**
 * @brief get lower left x coordinate of rectangual region
 */
cpl_size hdrl_rect_region_get_llx(const hdrl_parameter * p) 
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1) ;
    return ((const hdrl_rect_region_parameter *)p)->llx;
}

/**
 * @brief get lower left y coordinate of rectangual region
 */
cpl_size hdrl_rect_region_get_lly(const hdrl_parameter * p) 
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1) ;
    return ((const hdrl_rect_region_parameter *)p)->lly;
}

/**
 * @brief get upper right x coordinate of rectangular region
 */
cpl_size hdrl_rect_region_get_urx(const hdrl_parameter * p) 
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1) ;
    return ((const hdrl_rect_region_parameter *)p)->urx;
}

/**
 * @brief get upper right y coordinate of rectangual region
 */
cpl_size hdrl_rect_region_get_ury(const hdrl_parameter * p) 
{
    cpl_ensure(p, CPL_ERROR_NULL_INPUT, -1) ;
    return ((const hdrl_rect_region_parameter *)p)->ury;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Verify basic correctness of the parameters
  @param    param   rect region parameters
  @param    max_x   max value for upper x bound, set to < 0 to skip check
  @param    max_y   max value for upper y bound, set to < 0 to skip check
  @return   CPL_ERROR_NONE if everything is ok, an error code otherwise
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_rect_region_parameter_verify(
        const hdrl_parameter    *   param,
        const cpl_size              max_x,
        const cpl_size              max_y)
{
    const hdrl_rect_region_parameter * param_loc =
        (const hdrl_rect_region_parameter *)param ;

    cpl_error_ensure(param != NULL, CPL_ERROR_NULL_INPUT,
            return CPL_ERROR_NULL_INPUT, "NULL Input Parameters");
    cpl_error_ensure(hdrl_rect_region_parameter_check(param),
            CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT, 
            "Expected Rect Region parameter") ;
    cpl_error_ensure(param_loc->llx >= 1 && param_loc->lly >= 1 &&
            param_loc->urx >= 1 && param_loc->ury >= 1, CPL_ERROR_ILLEGAL_INPUT,
            return CPL_ERROR_ILLEGAL_INPUT,
            "Coordinates must be strictly positive");

    cpl_error_ensure(param_loc->urx >= param_loc->llx, CPL_ERROR_ILLEGAL_INPUT,
                     return CPL_ERROR_ILLEGAL_INPUT,
                     "urx (%ld) must be larger equal than llx (%ld)",
                     (long)param_loc->urx, (long)param_loc->llx);
    cpl_error_ensure(param_loc->ury >= param_loc->lly, CPL_ERROR_ILLEGAL_INPUT,
                     return CPL_ERROR_ILLEGAL_INPUT,
                     "ury (%ld) must be larger equal than lly (%ld)",
                     (long)param_loc->ury, (long)param_loc->lly);
    if (max_x > 0)
        cpl_error_ensure(param_loc->urx <= max_x, CPL_ERROR_ILLEGAL_INPUT,
                         return CPL_ERROR_ILLEGAL_INPUT,
                         "urx %zu larger than maximum %zu",
                         (size_t)param_loc->urx, (size_t)max_x);
    if (max_y > 0)
        cpl_error_ensure(param_loc->ury <= max_y, CPL_ERROR_ILLEGAL_INPUT,
                         return CPL_ERROR_ILLEGAL_INPUT,
                         "ury %zu larger than maximum %zu",
                         (size_t)param_loc->ury, (size_t)max_y);
    return CPL_ERROR_NONE;
}

/* ---------------------------------------------------------------------------*/
/**
  @brief Create parameter list for hdrl_rect_region
  @param base_context   base context of parameter (e.g. recipe name)
  @param prefix         prefix of parameter, may be empty string
  @param name_prefix    prefix for the parameter names, may be empty string
  @param defaults       default parameters values
  @return cpl_parameterlist
  Creates a parameterlist containing
      base_context.prefix.name-prefixllx
      base_context.prefix.name-prefixlly
      base_context.prefix.name-prefixurx
      base_context.prefix.name-prefixury
  The CLI aliases omit the base_context.
 */
/* ---------------------------------------------------------------------------*/
cpl_parameterlist * hdrl_rect_region_parameter_create_parlist(
        const char           *base_context,
        const char           *prefix,
        const char           *name_prefix,
        const hdrl_parameter *defaults)
{
    cpl_ensure(base_context && prefix && name_prefix && defaults,
               CPL_ERROR_NULL_INPUT, NULL);

    cpl_ensure(hdrl_rect_region_parameter_check(defaults),
    		   CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    cpl_parameterlist *parlist = cpl_parameterlist_new();

    /* --prefix.llx */
    hdrl_setup_vparameter(parlist, prefix, ".", name_prefix,
                         "llx", base_context,
                         "Lower left x pos. (FITS) defining the region",
                         CPL_TYPE_INT, (int)hdrl_rect_region_get_llx(defaults));
    /* --prefix.lly */
    hdrl_setup_vparameter(parlist, prefix, ".", name_prefix,
                         "lly", base_context,
                         "Lower left y pos. (FITS) defining the region",
                         CPL_TYPE_INT, (int)hdrl_rect_region_get_lly(defaults));

    /* --prefix.urx */
    hdrl_setup_vparameter(parlist, prefix, ".", name_prefix,
                         "urx", base_context,
                         "Upper right x pos. (FITS) defining the region",
                         CPL_TYPE_INT, (int)hdrl_rect_region_get_urx(defaults));

    /* --prefix.ury */
    hdrl_setup_vparameter(parlist, prefix, ".", name_prefix,
                         "ury", base_context,
                         "Upper right y pos. (FITS) defining the region",
                         CPL_TYPE_INT, (int)hdrl_rect_region_get_ury(defaults));

    if (cpl_error_get_code()) {
        cpl_parameterlist_delete(parlist);
        return NULL;
    }

    return parlist;
}

/* ---------------------------------------------------------------------------*/
/**
 @brief parse parameterlist for rectangle parameters
 @param parlist      parameter list to parse
 @param base_context prefix of parameter
 @param name_prefix  prefix of parameter name, may be empty string
 @see   hdrl_rect_get_parlist()
 @return A newly allocated hdrl_rect_region parameter or NULL in error case
    
 The returned object must be deallocated with hdrl_parameter_delete()
 parameterlist should have been created with hdrl_rect_get_parlist or have the 
 same name hierachy
 */
/* ---------------------------------------------------------------------------*/
hdrl_parameter * hdrl_rect_region_parameter_parse_parlist(
        const cpl_parameterlist *   parlist,
        const char              *   base_context,
        const char              *   name_prefix)
{
    cpl_size llx, lly, urx, ury ;
    cpl_error_ensure(base_context && parlist, CPL_ERROR_NULL_INPUT,
            return NULL, "NULL Input Parameters");
    const char * sep = strlen(base_context) > 0 ? "." : "";
    const char * points[] = {"llx", "lly", "urx", "ury"};
    cpl_size * dest[] = {&llx, &lly, &urx, &ury};
    for (size_t i = 0; i < 4; i++) {
        char * name = cpl_sprintf("%s%s%s%s", base_context, sep,
                                  name_prefix, points[i]);
        const cpl_parameter * par = cpl_parameterlist_find_const(parlist, name);
        *(dest[i]) = cpl_parameter_get_int(par);
        cpl_free(name);
    }

    if (cpl_error_get_code()) {
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                              "Error while parsing parameterlist "
                              "with base_context %s", base_context);
        return NULL;
    }

    return hdrl_rect_region_parameter_create(llx, lly, urx, ury) ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    wrap negative or zero coordinates around full image size
  @param    rect_region  rect region to wrap
  @param    nx   image size in x, nx is added to entries < 1
  @param    ny   image size in y, ny is added to entries < 1
  @return  cpl_error_code

  allows reverse indexing: 0 would be nx, -2 would be nx - 2 etc
  Wrapping is based in FITS convention: 1 first pixel, nx last pixel inclusive
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_rect_region_fix_negatives(
        hdrl_parameter    *     rect_region,
        const cpl_size          nx,
        const cpl_size          ny)
{
    hdrl_rect_region_parameter * rr_loc =
        (hdrl_rect_region_parameter *)rect_region ;

    cpl_error_ensure(rect_region != 0, CPL_ERROR_NULL_INPUT,
            return CPL_ERROR_NULL_INPUT, "region input must not be NULL");
    cpl_error_ensure(hdrl_rect_region_parameter_check(rect_region),
            CPL_ERROR_ILLEGAL_INPUT, return CPL_ERROR_ILLEGAL_INPUT,
            "Expected Rect Region parameter") ;

    if (nx > 0 && rr_loc->llx < 1) rr_loc->llx += nx;
    if (ny > 0 && rr_loc->lly < 1) rr_loc->lly += ny;
    if (nx > 0 && rr_loc->urx < 1) rr_loc->urx += nx;
    if (ny > 0 && rr_loc->ury < 1) rr_loc->ury += ny;

    return hdrl_rect_region_parameter_verify(rect_region, nx, ny);
}

/* ---------------------------------------------------------------------------*/
/**
 @brief returns CPL_TRUE if x is strictly monotonic increasing
 @param x      array to check for being strictly monotonic increasing
 @param l      length of the array
 @return CPL_TRUE if x is strictly monotonic increasing, CPL_FALSE otherwise
 */
/* ---------------------------------------------------------------------------*/
cpl_boolean hdrl_is_strictly_monotonic_increasing(const double * x, cpl_size l){

    if(x == NULL || l <= 1) return CPL_TRUE;

    for(cpl_size i = 0; i < l - 1; i++){
        if(x[i] >= x[i + 1]) return CPL_FALSE;
    }
    return CPL_TRUE;
}

/* ---------------------------------------------------------------------------*/
/**
 @brief sort in increasing or decreasing order x. Keep aligned with y1 and y2.
 @param x               x  array
 @param y1              y1 array
 @param y2              y2 array
 @param sample_len      length of the arrays
 @param sort_decreasing if CPL_TRUE sort as decreasing, otherwise increasing

 @return nothing
 */
/* ---------------------------------------------------------------------------*/
void hdrl_sort_on_x(double * x, double * y1, double * y2,
        const cpl_size sample_len, const cpl_boolean sort_decreasing){

    cpl_propertylist * col_list = cpl_propertylist_new();

    cpl_propertylist_append_bool(col_list, "x", sort_decreasing);

    cpl_table * tb = cpl_table_new(sample_len);

    cpl_table_wrap_double(tb, x,  "x");

    if(y1)
        cpl_table_wrap_double(tb, y1, "y1");

    if(y2)
        cpl_table_wrap_double(tb, y2, "y2");

    cpl_table_sort(tb, col_list);

    cpl_table_unwrap(tb, "x");

    if(y1)
        cpl_table_unwrap(tb, "y1");

    if(y2)
        cpl_table_unwrap(tb, "y2");

    cpl_table_delete(tb);
    cpl_propertylist_delete(col_list);
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief join strings together
 *
 * @param sep_ separator to place between strings, NULL equals empty string
 * @param n    number of strings to join
 *
 * The input strings may be empty or NULL in which case it skips the entry
 * adding no extra separator.
 *
 * @return joined string, must be deallocated by user with cpl_free
 */
/* ---------------------------------------------------------------------------*/
char * hdrl_join_string(const char * sep_, int n, ...)
{
    va_list vl;
    char * res = NULL;
    const char * sep = sep_ ? sep_ : "";
    cpl_ensure(n > 0, CPL_ERROR_ILLEGAL_INPUT, NULL);

    va_start(vl, n);
    for (int i = 0; i < n; i++) {
        char * tmp = res;
        char * val = va_arg(vl, char *);
        if (val == NULL || strlen(val) == 0) {
            continue;
        }
        if (!res) {
            res = cpl_strdup(val);
        }
        else {
            res = cpl_sprintf("%s%s%s", res, sep, val);
        }
        cpl_free(tmp);
    }
    va_end(vl);

    return res;
}


/** @cond EXPERIMENTAL */

/* ---------------------------------------------------------------------------*/
/**
 * @brief return file descriptor of a temporary file
 * @param dir     directory prefix to place the file in, may be NULL
 * @param unlink  unlink the file from the filesystem immediately
 * @return file descriptor or -1 on error
 * @note if dir is NULL or not writable it tries to create a file in following
 *       directories in decreasing order of preference:
 *       $TMPDIR, /var/tmp, /tmp, $PWD
 */
/* ---------------------------------------------------------------------------*/
int hdrl_get_tempfile(const char * dir, cpl_boolean unlink)
{
    /* options in decreasing preference */
    const char * tmpdirs[] = {
        /* user override */
        getenv("TMPDIR"),
        /* large file temp */
        "/var/tmp/",
        /* small file temp, may be tmpfs */
        "/tmp/"
    };
    const size_t ndirs = sizeof(tmpdirs) / sizeof(tmpdirs[0]);
    const char * tmpdir = NULL;

    if (dir && access(dir, W_OK) == 0) {
        tmpdir = dir;
    }
    else {
        for (size_t i = 0; i < ndirs; i++) {
            if (tmpdirs[i] && access(tmpdirs[i], W_OK) == 0) {
                tmpdir = tmpdirs[i];
                break;
            }
        }
    }

    {
        /* try $PWD if no tmpdir found */
        char * template = hdrl_join_string("/", 2, tmpdir, "hdrl_tmp_XXXXXX");
        int fd = mkstemp(template);
        if (fd == -1) {
            cpl_error_set_message(cpl_func, CPL_ERROR_FILE_IO,
                                  "Temporary file creation failed: %s",
                                  strerror(errno));
            cpl_free(template);
            return -1;
        }

        cpl_msg_debug(cpl_func, "Created tempfile %s", template);

        if (unlink) {
            remove(template);
        }
        cpl_free(template);

        return fd;
    }
}


/* ---------------------------------------------------------------------------*/
/**
 @brief get the absoluet current working directory
 @return char string, must be deleted by the user with cpl_free
 */
/* ---------------------------------------------------------------------------*/
char * hdrl_get_cwd(void)
{
    size_t n = 4096;
    char * buf;
    errno = 0;
    /* if only we could use sane GNU functions instead of this posix crap */
    while (1) {
        buf = cpl_malloc(n);
        if (getcwd(buf, n) != 0) {
            break;
        }
        else if (errno == ERANGE) {
            /* increase buffer, repeat */
            errno = 0;
            n *= 2;
            cpl_free(buf);
        }
        else {
            cpl_free(buf);
            cpl_error_set_message(cpl_func, CPL_ERROR_FILE_IO,
                                  "Could not determine current working "
                                  "directory: %s", strerror(errno));
            return NULL;
        }
    }

    return buf;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief scale a imagelist using scaling factors in a vector
 *
 * @param scale      vector containing scaling factors, the first element
 *                   defines the value to scale to
 * @param scale_e    errors of the scaling factors
 * @param scale_type scale type, additive or multiplicative
 * @param data       imagelist to be scaled
 * @param errors     errors of imagelist to be scaled
 *
 * If scale_type is HDRL_SCALE_ADDITIVE the difference of each value in the
 * scaling vector is subtracted from the first element of the vector and added
 * to each image in the imagelist.
 *
 * for i in vector.size:
 *   data[i] += vector[0] - vector[i]
 *
 * If scale_type is HDRL_SCALE_MULTIPLICATIVE the quotient of each value in
 * the scaling vector to the first element of the vector is multiplied
 * to each image in the imagelist.
 * If an element of the scaling vector is zero the image will have all its
 * pixels marked as bad in its bad pixel map.
 *
 * for i in vector.size:
 *   data[i] *= vector[0] / vector[i]
 *
 * The errors are propagated using error propagation of first order
 * and assuming no correlations between the values.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code
hdrl_normalize_imagelist_by_vector(const cpl_vector      * scale,
                                   const cpl_vector      * scale_e,
                                   const hdrl_scale_type   scale_type,
                                   cpl_imagelist         * data,
                                   cpl_imagelist         * errors)
{
    cpl_ensure_code(scale, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(scale_e, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(data, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(errors, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(cpl_vector_get_size(scale) == cpl_imagelist_get_size(data),
                    CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(cpl_vector_get_size(scale_e) == cpl_vector_get_size(scale),
                    CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(cpl_imagelist_get_size(errors) ==
                    cpl_imagelist_get_size(data), CPL_ERROR_ILLEGAL_INPUT);

    for (size_t i = 1; i < (size_t)cpl_imagelist_get_size(data); i++) {
        const hdrl_data_t dfirst = cpl_vector_get(scale, 0);
        const hdrl_error_t efirst = cpl_vector_get(scale_e, 0);
        cpl_image * dimg = cpl_imagelist_get(data, i);
        cpl_image * eimg = cpl_imagelist_get(errors, i);
        if (scale_type == HDRL_SCALE_ADDITIVE) {
            hdrl_data_t dscale_o = cpl_vector_get(scale, i);
            hdrl_error_t escale_o =  cpl_vector_get(scale_e, i);

            /* dscale = dfirst - dscale_o */
            hdrl_data_t dscale = dfirst;
            hdrl_error_t escale = efirst;
            hdrl_elemop_sub(&dscale, &escale, 1, &dscale_o, &escale_o, 1, NULL);

            /* dimg = dimg + dscale */
            hdrl_elemop_image_add_scalar(dimg, eimg, dscale, escale);
        }
        else if (scale_type == HDRL_SCALE_MULTIPLICATIVE) {
            hdrl_data_t dscale_o = cpl_vector_get(scale, i);
            hdrl_error_t escale_o = cpl_vector_get(scale_e, i);

            if (dscale_o == 0.) {
                cpl_msg_warning(cpl_func, "scale factor of image %zu is "
                                "not a number", i);
                cpl_image_add_scalar(dimg, NAN);
                cpl_image_add_scalar(eimg, NAN);
                cpl_image_reject_value(dimg, CPL_VALUE_NAN);
                cpl_image_reject_value(eimg, CPL_VALUE_NAN);
                continue;
            }

            /* dscale = dfirst / dscale_o */
            hdrl_data_t dscale = dfirst;
            hdrl_error_t escale = efirst;
            hdrl_elemop_div(&dscale, &escale, 1, &dscale_o, &escale_o, 1, NULL);

            /* dimg = dimg * dscale */
            hdrl_elemop_image_mul_scalar(dimg, eimg, dscale, escale);
        }
        else {
            return cpl_error_set_message(cpl_func, CPL_ERROR_UNSUPPORTED_MODE,
                                         "Unsupported scale type");
        }

        if (cpl_error_get_code())
            break;
    }

    return cpl_error_get_code();
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief scale a imagelist using scaling images
 *
 * @param scale      imagelist containing scaling factors, the first element
 *                   defines the value to scale to
 * @param scale_e    errors of the scaling factors
 * @param scale_type scale type, additive or multiplicative
 * @param data       imagelist to be scaled
 * @param errors     errors of imagelist to be scaled
 *
 * If scale_type is HDRL_SCALE_ADDITIVE the difference of each value in the
 * scaling imagelist is subtracted from the first element of the vector and
 * added to each image in the imagelist.
 *
 * for i in vector.size:
 *   data[i] += scale[0] - scale[i]
 *
 * If scale_type is HDRL_SCALE_MULTIPLICATIVE the quotient of each value in
 * the scaling vector to the first element of the imagelist is multiplied
 * to each image in the imagelist.
 * If an element of the scaling vector is zero the image will have all its
 * pixels marked as bad in its bad pixel map.
 *
 * for i in vector.size:
 *   data[i] *= scale[0] / scale[i]
 *
 * The errors are propagated using error propagation of first order
 * and assuming no correlations between the values.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code
hdrl_normalize_imagelist_by_imagelist(const cpl_imagelist * scale,
                                      const cpl_imagelist * scale_e,
                                      const hdrl_scale_type scale_type,
                                      cpl_imagelist * data,
                                      cpl_imagelist * errors)
{
    cpl_ensure_code(scale, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(scale_e, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(data, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(errors, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(cpl_imagelist_get_size(scale) ==
                    cpl_imagelist_get_size(data), CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(cpl_imagelist_get_size(scale_e) ==
                    cpl_imagelist_get_size(scale), CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(cpl_imagelist_get_size(errors) ==
                    cpl_imagelist_get_size(data), CPL_ERROR_ILLEGAL_INPUT);

    for (size_t i = 1; i < (size_t)cpl_imagelist_get_size(data); i++) {
        cpl_image * dscale =
            cpl_image_duplicate(cpl_imagelist_get_const(scale, 0));
        cpl_image * escale =
            cpl_image_duplicate(cpl_imagelist_get_const(scale_e, 0));
        cpl_image * dimg = cpl_imagelist_get(data, i);
        cpl_image * eimg = cpl_imagelist_get(errors, i);
        const cpl_image * dscale_o = cpl_imagelist_get_const(scale, i);
        const cpl_image * escale_o = cpl_imagelist_get_const(scale_e, i);

        if (scale_type == HDRL_SCALE_ADDITIVE) {
            /* dscale = dfirst - dscale_o */
            hdrl_elemop_image_sub_image(dscale, escale, dscale_o, escale_o);

            /* dimg = dimg + dscale */
            hdrl_elemop_image_add_image(dimg, eimg, dscale, escale);
        }
        else if (scale_type == HDRL_SCALE_MULTIPLICATIVE) {
            /* dscale = dfirst / dscale_o */
            hdrl_elemop_image_div_image(dscale, escale, dscale_o, escale_o);

            /* dimg = dimg * dscale */
            hdrl_elemop_image_mul_image(dimg, eimg, dscale, escale);
        }
        else {
            cpl_image_delete(dscale);
            cpl_image_delete(escale);
            return cpl_error_set_message(cpl_func, CPL_ERROR_UNSUPPORTED_MODE,
                                         "Unsupported scale type");
        }

        cpl_image_delete(dscale);
        cpl_image_delete(escale);

        if (cpl_error_get_code())
            break;
    }

    return cpl_error_get_code();
}

/** @endcond */


/** @cond PRIVATE */

/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    compress an image to a vector removing bad pixels
  @param    source     image to compress
  @param    bpm        optional bpm to use instead of the one in the image
  @return  cpl_vector or NULL if no good pixels or error
  @note    vector can't have size 0 so NULL is returned if all pixels are bad
 */
/*----------------------------------------------------------------------------*/
cpl_vector * hdrl_image_to_vector(
        const cpl_image     *   source, 
        const cpl_mask      *   bpm)
{
    cpl_ensure(source != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_vector * vec_source = NULL;
    /* only cast if required, extra copying of non double data is still
     * faster than a loop with cpl_image_get */

    CPL_DIAG_PRAGMA_PUSH_IGN(-Wcast-qual);
    cpl_image * d_img = cpl_image_get_type(source) == CPL_TYPE_DOUBLE ?
        (cpl_image *)source : cpl_image_cast(source, CPL_TYPE_DOUBLE);
    CPL_DIAG_PRAGMA_POP;

    const cpl_size naxis1 = cpl_image_get_size_x(source);
    const cpl_size naxis2 = cpl_image_get_size_y(source);

    /* get raw data */
    const double * restrict sdata = cpl_image_get_data_double_const(d_img);
    const cpl_binary * restrict bpmd = NULL;

    /* allocate output buffer */
    double * restrict ddata = cpl_malloc(naxis1 * naxis2 *
                                         sizeof(ddata[0]));
    long j = 0;

    if (bpm)
        bpmd = cpl_mask_get_data_const(bpm);
    else if (cpl_image_get_bpm_const(source) != NULL)
        bpmd = cpl_mask_get_data_const(cpl_image_get_bpm_const(source));

    if (bpmd == NULL) {
        memcpy(ddata, sdata, naxis1 * naxis2 * sizeof(ddata[0]));
        j = naxis1 * naxis2;
    }
    else {
        /* copy only good pixels */
        for (long i = 0; i < naxis1 * naxis2; i++) {
            if (bpmd[i] == CPL_BINARY_0) {
                ddata[j] = sdata[i];
                j++;
            }
        }
    }

    assert(j == naxis1 * naxis2 -
           (bpm ? cpl_mask_count(bpm) : cpl_image_count_rejected(source)));

    if (j > 0) {
        vec_source = cpl_vector_wrap(j, ddata);
    }
    else {
        cpl_free(ddata);
    }

    if (d_img != source) cpl_image_delete(d_img);

    return vec_source;
}

typedef struct {
    size_t available; /* number of cached pointers */
    size_t nspace;    /* max cache pointers */
    void ** ptrs;
} cache_bucket;

struct hdrl_vector_cache_ {
    cpl_size nbuckets;
    cache_bucket buckets[];
};

/* ---------------------------------------------------------------------------*/
/**
 * @brief create a cpl_vector cache
 *
 * @param max_cached_size  maximum size of vectors that will be cached
 * @param ncached_entries  number of vectors of each size that can be cached
 *
 * Allows storing allocated vectors in a temporary structure for fast retrieval later.
 * Useful when one needs to allocate and delete many small vectors with high
 * frequency, this avoids two calls to malloc that would cause high overheads
 * in this case.
 *
 * Vectors can be retrieved from the cache with hdrl_vector_new_from_cache()
 * and returned to the cache with hdrl_vector_delete_to_cache().
 *
 * The cache can store max_cached_size * ncached_entries vectors.
 *
 * May return NULL which is an no-opt cache and is a valid value for functions
 * working on it.
 *
 * @note the cache has no global state but it is not thread-safe
 *
 * @return cache structure that must be deleted with hdrl_vector_cache_delete()
 */
/* ---------------------------------------------------------------------------*/
hdrl_vector_cache * hdrl_vector_cache_new(cpl_size max_cached_size,
                                          cpl_size ncached_entries)
{
    /* for largish vectors a cache is not worthwhile, save the memory */
    if (max_cached_size > 50)
        return NULL;
    hdrl_vector_cache * c = cpl_malloc(sizeof(hdrl_vector_cache) +
                               sizeof(cache_bucket) * (max_cached_size + 1));
    c->nbuckets = max_cached_size + 1;
    for (cpl_size i = 0; i < c->nbuckets; i++) {
        c->buckets[i].available = 0;
        c->buckets[i].nspace = ncached_entries;
        c->buckets[i].ptrs = cpl_calloc(sizeof(cpl_vector*), ncached_entries);
    }
    return c;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Delete cpl_vector cache and all its entries
 *
 * @param cache cache to delete
 */
/* ---------------------------------------------------------------------------*/
void hdrl_vector_cache_delete(hdrl_vector_cache * cache)
{
    if (!cache)
        return;
    for (cpl_size i = 0; i < cache->nbuckets; i++) {
        for (size_t j = 0; j < cache->buckets[i].available; j++) {
            cpl_vector_delete(cache->buckets[i].ptrs[j]);
        }
        cpl_free(cache->buckets[i].ptrs);
    }
    cpl_free(cache);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Allocate cpl_vector
 *
 * @param cache pointer vector cache structure, may be NULL
 * @param sz    size of vector to allocate
 *
 * returns a cpl_vector as cpl_vector_new() would but if a matching vector is in
 * the cache it returns that instead of allocating a new one.
 * The returned vector is a normal cpl_vector with no restrictions on usage.
 *
 * @see cpl_vector_new()
 *
 * @return  cpl_vector that must be deleted with cpl_vector_delete or
 *          hdrl_cplvector_delete_to_cache()
 */
/* ---------------------------------------------------------------------------*/
cpl_vector * hdrl_cplvector_new_from_cache(hdrl_vector_cache * cache, cpl_size sz)
{
    if (!cache) {
        return cpl_vector_new(sz);
    }
    if (sz < cache->nbuckets) {
        if (cache->buckets[sz].available > 0) {
            return cache->buckets[sz].ptrs[--(cache->buckets[sz].available)];
        }
    }
    return cpl_vector_new(sz);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Return cpl_vector to a cache
 *
 * @param cache  cache to place the vector in, may be NULL
 * @param v      vector to be deleted or cached
 *
 * Returns a vector to the cache for later reuse, if the cache is full it
 * will be deleted.
 */
/* ---------------------------------------------------------------------------*/
void hdrl_cplvector_delete_to_cache(hdrl_vector_cache * cache, cpl_vector * v)
{
    if (!v) {
        return;
    }
    if (cache) {
        const cpl_size sz = cpl_vector_get_size(v);
        if (sz < cache->nbuckets) {
            if (cache->buckets[sz].available < cache->buckets[sz].nspace) {
                cache->buckets[sz].ptrs[(cache->buckets[sz].available)++] = v;
                return;
            }
        }
    }
    cpl_vector_delete(v);
}


/* internal if imgdatabuf and maskbuf are not NULL they are assumed to hold the
 * data buffers of the image in the list in order to save on cpl_image_get
 * calls, error and type checking must be done by caller */
static cpl_vector * imagelist_to_vector(const cpl_imagelist * list,
                                        const cpl_size nx,
                                        const cpl_size x,
                                        const cpl_size y,
                                        const double ** imgdatabuf,
                                        const cpl_binary ** maskbuf, hdrl_vector_cache * cache)
{
    const long nz  = list ? cpl_imagelist_get_size(list) : -1;
    unsigned long j = 0;
    cpl_vector * vec        = hdrl_cplvector_new_from_cache(cache, nz);
    double * restrict ddata = cpl_vector_get_data(vec);

    if (imgdatabuf && maskbuf) {
        for (long k = 0; k < nz; k++) {
            double v = imgdatabuf[k][(y - 1) * nx + (x  - 1)];
            cpl_binary rej = maskbuf[k] ?
                maskbuf[k][(y - 1) * nx + (x  - 1)] : 0;

            if (!rej) {
                ddata[j] = v;
                j++;
            }
        }
    }
    else {
        for (long k = 0; k < nz; k++) {
            int rej;
            const cpl_image * img = cpl_imagelist_get_const(list, k);
            double v = cpl_image_get(img, x, y, &rej);

            if (!rej) {
                ddata[j] = v;
                j++;
            }
        }
    }

    if (j > 0) {
        if ((cpl_size)j != nz) {
            cpl_vector_set_size(vec, j);
        }
    }
    else {
        hdrl_cplvector_delete_to_cache(cache, vec);
        vec = NULL;
    }

    return vec;
}


/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    compress an imagelist to a vector via z axis, removing bad pixels
  @param    list     imagelist to compress
  @param    x        x coordinate to compress over (FITS convention)
  @param    y        y coordinate to compress over (FITS convention)
  @return  cpl_vector or NULL if no good pixels or error
  @note    vector can't have size 0 so NULL is returned if all pixels are bad
 */
/*----------------------------------------------------------------------------*/
cpl_vector * hdrl_imagelist_to_vector(const cpl_imagelist * list,
                                      const cpl_size x,
                                      const cpl_size y)
{
    cpl_size nx;
    const cpl_size nz  = list ? cpl_imagelist_get_size(list) : -1;
    cpl_ensure(list != NULL, CPL_ERROR_NULL_INPUT, NULL);
    cpl_ensure(nz > 0, CPL_ERROR_ILLEGAL_INPUT, NULL);
    cpl_ensure(x > 0, CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);
    cpl_ensure(y > 0, CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);

    {
        const cpl_image  * img = cpl_imagelist_get_const(list, 0);
        const cpl_size ny = cpl_image_get_size_y(img);
        nx = cpl_image_get_size_x(img);
        cpl_ensure(x <= nx, CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);
        cpl_ensure(y <= ny, CPL_ERROR_ACCESS_OUT_OF_RANGE, NULL);
    }
    return imagelist_to_vector(list, nx, x, y, NULL, NULL, NULL);
}


/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief    compress an imagelist to a vector via z axis for a row
  @param    list     imagelist to compress
  @param    y        y coordinate to compress over (FITS convention)
  @param    out      output array of vector pointers of size nx
  @param    cache    vector cache to avoid unnecessary memory allocations
                     may be NULL
  @return  cpl_error_code
  @note    vector can't have size 0 so NULL is returned if all pixels are bad

  more efficient than hdrl_imagelist_to_vector for double images as it will
  call cpl_image_get less often by caching the data buffers
 */
/*----------------------------------------------------------------------------*/
cpl_error_code hdrl_imagelist_to_vector_row(const cpl_imagelist * list,
                                            const cpl_size y,
                                            cpl_vector ** out,
                                            hdrl_vector_cache * cache)
{
    long nx;
    int isdouble;
    const long nz  = list ? (long)cpl_imagelist_get_size(list) : 0;
    cpl_ensure_code(list != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(nz > 0, CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code(y > 0, CPL_ERROR_ACCESS_OUT_OF_RANGE);

    {
        const cpl_image  * img = cpl_imagelist_get_const(list, 0);
        const cpl_size ny = cpl_image_get_size_y(img);
        cpl_ensure_code(y <= ny, CPL_ERROR_ACCESS_OUT_OF_RANGE);
        nx = (long)cpl_image_get_size_x(img);
        isdouble = cpl_image_get_type(img) == CPL_TYPE_DOUBLE;
    }

    const double * imgdatabuf[nz];
    const cpl_binary * maskbuf[nz];
    for (long i = 0; i < nz && isdouble; i++) {
        const cpl_image * img = cpl_imagelist_get_const(list, i);
        const cpl_mask * bpm = cpl_image_get_bpm_const(img);
        imgdatabuf[i] = cpl_image_get_data_double_const(img);
        if (bpm) {
            maskbuf[i] = cpl_mask_get_data_const(bpm);
        }
        else {
            maskbuf[i] = NULL;
        }
    }
    for (long x = 0; x < nx; x++) {
        if (isdouble) {
            out[x] = imagelist_to_vector(list, nx, x + 1, y,
                                         imgdatabuf, maskbuf, cache);
        }
        else {
            out[x] = imagelist_to_vector(list, nx, x + 1, y, NULL, NULL, cache);
        }
    }
    return cpl_error_get_code();
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief wrap two cpl_imagelist around data and errors in hdrl_imagelist
 *
 * @param list input hdrl_imagelist
 * @param data output data cpl_imagelist (may be NULL)
 * @param errs output error cpl_imagelist (may be NULL)
 * @return cpl_error_code
 *
 * @note the imagelists only wrap the images and must be deleted with
 * cpl_imagelist_unwrap
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code
hdrl_imagelist_to_cplwrap(const hdrl_imagelist * list,
                          cpl_imagelist ** data,
                          cpl_imagelist ** errs)
{
    cpl_ensure_code(list, CPL_ERROR_NULL_INPUT);
    if (data) {
        *data = cpl_imagelist_new();
    }
    if (errs) {
        *errs = cpl_imagelist_new();
    }
    for (cpl_size i = 0; i < hdrl_imagelist_get_size(list); i++) {
        hdrl_image * img = hdrl_imagelist_get(list, i);
        if (data) {
            cpl_imagelist_set(*data, hdrl_image_get_image(img), i);
        }
        if (errs) {
            cpl_imagelist_set(*errs, hdrl_image_get_error(img), i);
        }
    }

    if (cpl_error_get_code()) {
        if (data) {
            cpl_imagelist_unwrap(*data);
            *data = NULL;
        }
        if (errs) {
            cpl_imagelist_unwrap(*errs);
            *errs = NULL;
        }
    }
    return cpl_error_get_code();
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief filter image on a grid
 *
 * @param ima           image to filter
 * @param x             row vector of coordinates to filter on
 * @param y             row vector of coordinates to filter on
 * @param filtersize_x  size of the median filter
 * @param filtersize_y  size of the median filter
 * @return filtered image of size of the grid
 *
 */
/* ---------------------------------------------------------------------------*/
cpl_image *
hdrl_medianfilter_image_grid(const cpl_image * ima, cpl_matrix * x, cpl_matrix * y,
                             cpl_size  filtersize_x, cpl_size filtersize_y)
{
    cpl_error_ensure(ima != NULL, CPL_ERROR_NULL_INPUT, return NULL,
            "NULL input image");
    cpl_error_ensure(filtersize_x > 0 && filtersize_y > 0 ,
            CPL_ERROR_INCOMPATIBLE_INPUT, return NULL,
            "All function parameters must be greater then Zero");

    const cpl_size nx = cpl_image_get_size_x(ima);
    const cpl_size ny = cpl_image_get_size_y(ima);
    const cpl_size steps_x = cpl_matrix_get_nrow(x);
    const cpl_size steps_y = cpl_matrix_get_nrow(y);

    cpl_image * ima_local = cpl_image_new(steps_x, steps_y, CPL_TYPE_DOUBLE);

    for (cpl_size iy = 0; iy < steps_y; iy++) {
        cpl_size middlep_y = cpl_matrix_get(y, iy, 0);
        for (cpl_size ix = 0; ix < steps_x; ix++) {
            cpl_size middlep_x = cpl_matrix_get(x, ix, 0);

            cpl_size lowerlimit_x = CX_MAX(middlep_x - filtersize_x, 1);
            cpl_size lowerlimit_y = CX_MAX(middlep_y - filtersize_y, 1);
            cpl_size upperlimit_x = CX_MIN(middlep_x + filtersize_x, nx);
            cpl_size upperlimit_y = CX_MIN(middlep_y + filtersize_y, ny);

            double median = cpl_image_get_median_window(ima, lowerlimit_x,
                            lowerlimit_y, upperlimit_x, upperlimit_y);

            cpl_image_set(ima_local, ix + 1, iy + 1, median);

            cpl_msg_debug(cpl_func, "middlep_x: %lld, middlep_y: %lld, median: "
                          "%g", middlep_x, middlep_y, median);
        }
    }
    return ima_local;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief create linear space row vector
 * @param start  starting point
 * @param stop   end point, exclusive
 * @param step   step size
 *
 * @returns matrix with one row filled equally spaced points from start to end
 *
 */
/* ---------------------------------------------------------------------------*/
cpl_matrix * hdrl_matrix_linspace(
        cpl_size    start,
        cpl_size    stop,
        cpl_size    step)
{
    cpl_matrix * x = cpl_matrix_new(stop / step, 1);
    for (intptr_t i = 0; start + i * step < stop && i < stop / step; i++) {
        cpl_matrix_set(x, i, 0, start + i * step);
    }
    return x;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief fit 2D legendre polynomials to img
 *
 * @param img      image to be fitted
 * @param order_x  order of x polynomial
 * @param order_y  order of y polynomial
 * @param grid_x   x grid to evaluate on
 * @param grid_y   y grid to evaluate on
 * @param orig_nx  upper limit in x, exclusive
 * @param orig_ny  upper limit in y, exclusive
 *
 * @return coefficient matrix of the fitted 2-D polynomial
 * @see hdrl_mime_linalg_pairwise_column_tensor_products_create,
 *      hdrl_legendre_to_image
 */
/* ---------------------------------------------------------------------------*/
cpl_matrix * hdrl_fit_legendre(
        cpl_image   *   img,
        int             order_x,
        int             order_y,
        cpl_matrix  *   grid_x,
        cpl_matrix  *   grid_y,
        cpl_size        orig_nx,
        cpl_size        orig_ny)
{
    cpl_size nx2 = cpl_matrix_get_nrow(grid_x);
    cpl_size ny2 = cpl_matrix_get_nrow(grid_y);
    cpl_matrix * xpolys =
        hdrl_mime_legendre_polynomials_create(order_x + 1, 0, orig_nx - 1, grid_x);
    cpl_matrix * ypolys =
        hdrl_mime_legendre_polynomials_create(order_y + 1, 0, orig_ny - 1, grid_y);
    cpl_matrix * tensors =
        hdrl_mime_linalg_pairwise_column_tensor_products_create(ypolys,
                                                                xpolys);
    cpl_matrix * mimage = cpl_matrix_wrap(nx2 * ny2, 1, cpl_image_get_data(img));
    cpl_matrix * coeffs = cpl_matrix_solve_normal(tensors, mimage);
    cpl_matrix_unwrap(mimage);
    cpl_matrix_delete(xpolys);
    cpl_matrix_delete(ypolys);
    cpl_matrix_delete(tensors);

    return coeffs;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief evaluate 2D legendre polynomials on image
 *
 * @param coeffs   legendre coefficients
 * @param order_x  order of x polynomial
 * @param order_y  order of y polynomial
 * @param nx       x size of image
 * @param ny       y size of image
 * @return legendre polynomial evaluated on an image
 *
 */
/* ---------------------------------------------------------------------------*/
cpl_image * hdrl_legendre_to_image(
        cpl_matrix  *   coeffs,
        int             order_x,
        int             order_y,
        cpl_size        nx,
        cpl_size        ny)
{
    /* evaluate on full image */
    /* TODO need grid of original fit here? */
    cpl_matrix * x = hdrl_matrix_linspace(0, nx, 1);
    cpl_matrix * y = hdrl_matrix_linspace(0, ny, 1);
    cpl_matrix * xpolys =
        hdrl_mime_legendre_polynomials_create(order_x + 1, 0, nx - 1, x);
    cpl_matrix  * ypolys =
        hdrl_mime_legendre_polynomials_create(order_y + 1, 0, ny - 1, y);
    cpl_matrix * tensors =
        hdrl_mime_linalg_pairwise_column_tensor_products_create(ypolys,
                                                                xpolys);
    cpl_matrix * result = cpl_matrix_product_create(tensors, coeffs);
    cpl_image * iresult = cpl_image_wrap(nx, ny, CPL_TYPE_DOUBLE,
                                         cpl_matrix_get_data(result));
    cpl_matrix_delete(x);
    cpl_matrix_delete(y);
    cpl_matrix_delete(xpolys);
    cpl_matrix_delete(ypolys);
    cpl_matrix_delete(tensors);
    cpl_matrix_unwrap(result);

    return iresult;
}
/* ---------------------------------------------------------------------------*/
/**
 * @brief check if bad pixel masks are identical.
 * @param mask1   input mask 1 to be compared with input mask 2
 * @param mask2   input mask 2 to be compared with input mask 1
 *
 * @return  0 if the masks are identical - in all other cases 1
 */
/* ---------------------------------------------------------------------------*/
int hdrl_check_maskequality(const cpl_mask * mask1, const cpl_mask * mask2)
{
    cpl_ensure(mask1, CPL_ERROR_NULL_INPUT, 1);
    cpl_ensure(mask2, CPL_ERROR_NULL_INPUT, 1);

    cpl_size m1nx = cpl_mask_get_size_x(mask1);
    cpl_size m1ny = cpl_mask_get_size_y(mask1);
    cpl_size m2nx = cpl_mask_get_size_x(mask2);
    cpl_size m2ny = cpl_mask_get_size_y(mask2);
    cpl_ensure(m1nx == m2nx, CPL_ERROR_NONE, 1);
    cpl_ensure(m1ny == m2ny, CPL_ERROR_NONE, 1);

    const cpl_binary * dm1bpm = cpl_mask_get_data_const(mask1);
    const cpl_binary * dm2bpm = cpl_mask_get_data_const(mask2);
    if (memcmp(dm1bpm, dm2bpm, m1nx * m1ny) != 0) {
        return 1;
    }
    else {
        return 0;
    }
}

static const cpl_image *
image_const_row_view_create(const cpl_image * img,
                            cpl_size ly,
                            cpl_size uy)
{
    const size_t dsz = cpl_type_get_sizeof(cpl_image_get_type(img));
    const cpl_size nx = cpl_image_get_size_x(img);
    const char * d = cpl_image_get_data_const(img);
    size_t offset = (ly - 1) * nx;
    cpl_size nny = uy - ly + 1;

    CPL_DIAG_PRAGMA_PUSH_IGN(-Wcast-qual);

    cpl_image * wimg = cpl_image_wrap(nx, nny, cpl_image_get_type(img),
                                     (char*)d + offset * dsz);

    const cpl_mask * omask = cpl_image_get_bpm_const(img);
    if (omask) {
        cpl_mask * mask = cpl_mask_wrap(nx, nny,
                        (cpl_binary*)cpl_mask_get_data_const(omask) + offset);
        cpl_mask_delete(hcpl_image_set_bpm(wimg, mask));
    }

    CPL_DIAG_PRAGMA_POP;

    return wimg;
}

static void
image_const_row_view_delete(const cpl_image * img)
{
    CPL_DIAG_PRAGMA_PUSH_IGN(-Wcast-qual);

    cpl_mask_unwrap(cpl_image_unset_bpm((cpl_image*)(img)));
    cpl_image_unwrap((cpl_image *)img);

    CPL_DIAG_PRAGMA_POP;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief compute a filter in parallel
 *
 * @param img     image to filter
 * @param kernel  filter kernel (mutually exclusive with mask)
 * @param mask    filter mask (mutually exclusive with kernel)
 * @param mode   filter mode
 * @return filtered image, must be deleted with cpl_image_delete
 *
 * if kernel is not NULL and mask is NULL cpl_image_filter is used
 * if mask is not NULL and kernel is NULL cpl_image_filter_mask is used
 *
 * the kernel or mask size must be odd and less equal to the image size
 *
 */
/* ---------------------------------------------------------------------------*/
cpl_image *
hdrl_parallel_filter_image(const cpl_image * img,
                           const cpl_matrix * kernel,
                           const cpl_mask * mask,
                           const cpl_filter_mode mode)
{
    cpl_ensure(img, CPL_ERROR_NULL_INPUT, NULL);
    intptr_t nx = cpl_image_get_size_x(img);
    intptr_t ny = cpl_image_get_size_y(img);
    intptr_t ky, kx;
    /* TODO probably one can use all except COPY */
    const cpl_border_mode border = CPL_BORDER_FILTER;
    cpl_ensure((kernel && !mask) || (!kernel && mask),
               CPL_ERROR_INCOMPATIBLE_INPUT, NULL);
    if (kernel) {
        ky = cpl_matrix_get_nrow(kernel);
        kx = cpl_matrix_get_ncol(kernel);
    }
    else {
        ky = cpl_mask_get_size_y(mask);
        kx = cpl_mask_get_size_x(mask);
    }
    cpl_ensure(ky % 2 == 1, CPL_ERROR_INCOMPATIBLE_INPUT, NULL);
    cpl_ensure(ny >= ky, CPL_ERROR_INCOMPATIBLE_INPUT, NULL);
    cpl_ensure(nx >= kx, CPL_ERROR_INCOMPATIBLE_INPUT, NULL);

    intptr_t hk = (ky / 2);
    cpl_image * res = cpl_image_new(nx, ny, cpl_image_get_type(img));
    /* make sure image has a bpm to avoid creation races later */
    cpl_image_get_bpm(res);

    /* filter first half-kernel rows (needs full kernel image) */
    if (hk) {
        const cpl_image * slice = image_const_row_view_create(img, 1, ky);
        cpl_image * slres = cpl_image_duplicate(slice);
        if (kernel) {
            cpl_image_filter(slres, slice, kernel, mode, border);
        }
        else {
            cpl_image_filter_mask(slres, slice, mask, mode, border);
        }

        const cpl_image * slice2 = image_const_row_view_create(slres, 1, hk);
        cpl_image_copy(res, slice2, 1, 1);
        image_const_row_view_delete(slice2);
        image_const_row_view_delete(slice);
        cpl_image_delete(slres);
    }
    intptr_t y = hk;
    const intptr_t s = 200;
    /* filter s sized row chunks needs one kernel overlap */
HDRL_OMP(omp parallel for lastprivate(y) if (ny > s + ky))
    for (y = hk; y < ny - ky - (ny - ky) % s; y+=s) {
        intptr_t l = (y + 1) - hk;
        intptr_t u = (y + 1 + s) + hk - 1;
        const cpl_image * slice = image_const_row_view_create(img, l, u);
        cpl_image * slres = cpl_image_new(nx, u - l + 1,
                                          cpl_image_get_type(slice));
        if (kernel) {
            cpl_image_filter(slres, slice, kernel, mode, border);
        }
        else {
            cpl_image_filter_mask(slres, slice, mask, mode, border);
        }
        const cpl_image * slice2 =
            image_const_row_view_create(slres, hk + 1, hk + s);
        cpl_image_copy(res, slice2, 1, y + 1);
        image_const_row_view_delete(slice);
        image_const_row_view_delete(slice2);
        cpl_image_delete(slres);
    }
    /* filter remainder, needs half kernel overlap */
    if (y + 1 - hk < ny) {
        const cpl_image * slice = image_const_row_view_create(img, y + 1 - hk, ny);
        cpl_image * slres = cpl_image_duplicate(slice);
        if (kernel) {
            cpl_image_filter(slres, slice, kernel, mode, border);
        }
        else {
            cpl_image_filter_mask(slres, slice, mask, mode, border);
        }
        const cpl_image * slice2 =
            image_const_row_view_create(slres, hk + 1, cpl_image_get_size_y(slice));
        cpl_image_copy(res, slice2, 1, y + 1);
        image_const_row_view_delete(slice);
        image_const_row_view_delete(slice2);
        cpl_image_delete(slres);
    }
    return res;
}


/**
 * @brief
 *    Convert between physical and world coordinates using multiple threads
 *
 * @param wcs       The input cpl_wcs structure
 * @param from      The input coordinate matrix
 * @param to        The output coordinate matrix
 * @param status    The output status array
 * @param transform The transformation mode
 *
 * @return  An appropriate error code
 *
 * @see cpl_wcs_convert()
 *
 * This function is a wrapper around cpl_wcs_convert using OpenMP for
 * parallelization, see the cpl documentation for details on the functionality.
 */
    /* ---------------------------------------------------------------------------*/
cpl_error_code
hdrl_wcs_convert(const cpl_wcs *wcs, const cpl_matrix *from,
                 cpl_matrix **to, cpl_array **status,
                 cpl_wcs_trans_mode transform)
{
    size_t nr = cpl_matrix_get_nrow(from);
    size_t nc = cpl_matrix_get_ncol(from);
    const size_t s = 4000;
    size_t i;
    int * dstatus;
    cpl_ensure_code(to, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(status, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(wcs, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(from, CPL_ERROR_NULL_INPUT);

    *status = cpl_array_new(nr, CPL_TYPE_INT);
    cpl_ensure_code(*status, CPL_ERROR_NULL_INPUT);
    dstatus = cpl_array_get_data_int(*status);
    *to = cpl_matrix_new(nr, nc);
    cpl_error_code err = CPL_ERROR_NONE;

HDRL_OMP(omp parallel for if (nr > s))
    for (i = 0; i < nr; i+=s) {
        cpl_matrix * lfrom = cpl_matrix_extract(from, i, 0, 1, 1, CPL_MIN(s, nr - i), nc);
        cpl_matrix * lto = NULL;
        cpl_array * lstatus = NULL;
        cpl_error_code lerr = cpl_wcs_convert(wcs, lfrom, &lto, &lstatus, transform);
        if (lto != NULL) {
            cpl_matrix_copy(*to, lto, i, 0);
        }
        if (lstatus != NULL) {
            memcpy(&dstatus[i], cpl_array_get_data_int(lstatus),
                   cpl_array_get_size(lstatus) * sizeof(*dstatus));
        }
        cpl_array_delete(lstatus);
        cpl_matrix_delete(lfrom);
        cpl_matrix_delete(lto);
        if (lerr != CPL_ERROR_NONE) {
HDRL_OMP(omp critical(hdrl_hdrlwcserror))
            err = lerr;
        }
    }

    if (err == CPL_ERROR_UNSUPPORTED_MODE) {
        cpl_matrix_delete(*to);
        *to = NULL;
        cpl_array_delete(*status);
        *status = NULL;
    }

    return cpl_error_set(cpl_func, err);
}

/* cpl_image_set_bpm from cpl 6.4 which we can't require yet for VLTSW 2014 */
struct _cpl_image_ {
    /* Size of the image in x and y */
    cpl_size            nx, ny;
    /* Type of the pixels used for the cpl_image */
    cpl_type            type;
    /* Pointer to pixel buffer as a 1d buffer */
    void            *   pixels;
    /* Bad Pixels mask */
    cpl_mask        *   bpm;
};

cpl_mask * hcpl_image_set_bpm(cpl_image * self, cpl_mask * bpm) 
{
#if CPL_VERSION_CODE < CPL_VERSION(6, 4, 0)
    cpl_mask * oldbpm;

    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, NULL);

    oldbpm = self->bpm;
    self->bpm = bpm; 
 
    return oldbpm;
#else
    return cpl_image_set_bpm(self, bpm);
#endif
}

double hcpl_vector_get_mad_window(cpl_vector * vec,
                                  cpl_size llx,
                                  cpl_size urx,
                                  double * sigma)
{
    /* TODO use cpl directly when PIPE-4792 is fixed */
    struct _cpl_image_ img;
    img.pixels = cpl_vector_get_data(vec);
    img.nx = cpl_vector_get_size(vec);
    img.ny = 1;
    img.bpm = NULL;
    img.type = CPL_TYPE_DOUBLE;
    return cpl_image_get_mad_window(&img, llx, 1, urx, 1, sigma);
}

double hcpl_gaussian_eval_2d(const cpl_array * self, double x, double y)
{
#if CPL_VERSION_CODE < CPL_VERSION(6, 4, 0)
    cpl_errorstate prestate = cpl_errorstate_get();
    const double B    = cpl_array_get_double(self, 0, NULL);
    const double A    = cpl_array_get_double(self, 1, NULL);
    const double R    = cpl_array_get_double(self, 2, NULL);
    const double M_x  = cpl_array_get_double(self, 3, NULL);
    const double M_y  = cpl_array_get_double(self, 4, NULL);
    const double S_x  = cpl_array_get_double(self, 5, NULL);
    const double S_y  = cpl_array_get_double(self, 6, NULL);

    double value = 0.0; 

    if (!cpl_errorstate_is_equal(prestate)) {
        (void)cpl_error_set_where(cpl_func);
    } else if (cpl_array_get_size(self) != 7) { 
        (void)cpl_error_set(cpl_func, CPL_ERROR_ILLEGAL_INPUT);
    } else if (fabs(R) < 1.0 && S_x != 0.0 && S_y != 0.0) {
        const double x_n  = (x - M_x) / S_x; 
        const double y_n  = (y - M_y) / S_y; 

        value = B + A / (CPL_MATH_2PI * S_x * S_y * sqrt(1 - R * R)) *
            exp(-0.5 / (1 - R * R) * ( x_n * x_n + y_n * y_n
                                       - 2.0 * R * x_n * y_n));
    } else if (fabs(R) > 1.0) {
        (void)cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_OUTPUT,
                                     "fabs(R=%g) > 1", R);
    } else {
        (void)cpl_error_set_message(cpl_func, CPL_ERROR_DIVISION_BY_ZERO,
                                     "R=%g. Sigma=(%g, %g)", R, S_x, S_y);
    }    

    return value;
#else
    return cpl_gaussian_eval_2d(self, x, y);
#endif
}


/** @endcond */


/*----------------------------------------------------------------------------*/
/**
 *
 * @brief   Compute the effective airmass of an observation. Takes in count the
 * error propagation if you enter the relative error of the input parameters
 * in a hdrl_value structure {data,error}
 *
 * The function calculates the average airmass for the line of sight given by the
 * right ascension (ra) and the declination (dec). The latitude (geolat) in
 * degrees of the observatory site and the local siderial time (lst) at
 * observation start has to be given, as well as the duration of the observation,
 * i.e. the exposure time (exptime). If the exposure time is zero then only one
 * value of airmass is computed, instead of weighting beginning, middle, and
 * end of exposure according to Stetson (Stetson P., 1987, PASP 99, 191).
 *
 * @param   aRA          right ascension in degrees
 * @param   aDEC         declination in degrees
 * @param   aLST         local sideral time (s.) elapsed since siderial midnight
 * @param   aExptime     integration time in seconds
 * @param   aLatitude    latitude of the observatory site in degrees
 * @param   type         kind of approximation (see hdrl_airmass_approx enumtype)
 *
 * @return  The computed average airmass or {-1,0.} on error.
 *
 * @note This function can be calcule different kinds of approximation specified
 * in the hdrl_airmass_approx enum type, explain as follow. 1) The formula given
 * by Hardie (Hardie 1962, In: "Astronomical Techniques", ed. Hiltner, p. 184)
 * to compute the airmass as a function of zenith distance. 2) The formula of
 * Young and Irvine (Young A. T., Irvine W. M., 1967, Astron. J. 72, 945).
 * The range of trustworthy airmass outputs is only between 1 and 4. 3)
 * The formula of Young (Young A. T., 1994 ApOpt, 33, 1108).
 *
 * @note You can to find in <a href="http://en.wikipedia.org/w/index.php
 * ?title=Airmass&oldid=358226579#Interpolative_formulas">Interpolative formulas</a>
 * an interesting collection of approximations.
 *
 */
/*----------------------------------------------------------------------------*/
hdrl_value hdrl_utils_airmass(
    hdrl_value aRA, hdrl_value aDEC, hdrl_value aLST,
	hdrl_value aExptime, hdrl_value aLatitude,
	hdrl_airmass_approx type)
{

	hdrl_value retErr = {-1.,0.};
    cpl_ensure(   aRA.data       >=   0. && aRA.data       <  360.   && aRA.error       >= 0.
     		   && aDEC.data      >= -90. && aDEC.data      <= 90.    && aDEC.error      >= 0.
			   && aLST.data      >=   0. && aLST.data      <  86400. && aLST.error      >= 0.
			   && aExptime.data  >=   0.                             && aExptime.error  >= 0.
			   && aLatitude.data >= -90. && aLatitude.data <= 90.    && aLatitude.error >= 0.
			   && (type == HDRL_AIRMASS_APPROX_HARDIE || type == HDRL_AIRMASS_APPROX_YOUNG_IRVINE || type == HDRL_AIRMASS_APPROX_YOUNG),
			   CPL_ERROR_ILLEGAL_INPUT,
			   retErr);

    /* Compute hour angle of the observation in degrees. */
    hdrl_value HA = {aLST.data * 15. / 3600. - aRA.data,
    				 aLST.error * fabs(15. / 3600.) + aRA.error * fabs(-1.)};

    /* Range adjustments.Angle between line of sight and meridian is needed. */
    if (HA.data < -180.) HA.data += 360.;
    if (HA.data >  180.) HA.data -= 360.;

    /* Convert angles from degrees to radians. */

    hdrl_value delta     = {aDEC.data       *      CPL_MATH_RAD_DEG,
                            aDEC.error      * fabs(CPL_MATH_RAD_DEG)};

    hdrl_value latitude  = {aLatitude.data  *      CPL_MATH_RAD_DEG,
                            aLatitude.error * fabs(CPL_MATH_RAD_DEG)};

    hdrl_value hourangle = {HA.data         *      CPL_MATH_RAD_DEG,
                            HA.error        * fabs(CPL_MATH_RAD_DEG)};


    /* Calculate airmass of the observation using the approximation given    *
     * by Young (1994). For non-zero exposure times these airmass values are *
     * averaged using the weights given by Stetson.                          */

    hdrl_value cosz = hdrl_get_zenith_distance(hourangle, delta, latitude);
    hdrl_value z    = {0., cosz.error};
    double zlimit   = 80.;

	if (type == HDRL_AIRMASS_APPROX_HARDIE) {

		z.data  = acos(cosz.data) * CPL_MATH_DEG_RAD;
		z.error = cosz.error * fabs(-CPL_MATH_DEG_RAD / sqrt(1. + pow(cosz.data, 2)));

		cpl_error_ensure(z.data <= zlimit, CPL_ERROR_ILLEGAL_OUTPUT, return retErr,
			"Zenith angle %f+-[%f] > %f!", z.data, z.error, zlimit);
	}

	cpl_error_ensure(
		cosz.data != 0. && fabs(1./cosz.data) >= FLT_EPSILON && acos(cosz.data) <= CPL_MATH_PI_2,
		CPL_ERROR_ILLEGAL_OUTPUT,
		return retErr,
		"Airmass computation unsuccessful. Object is below the horizon at start (z = %f). Return the error",
		cosz.error * fabs(-CPL_MATH_DEG_RAD / sqrt(1. + pow(cosz.data, 2))) );

    hdrl_value airmass     = {0.,0.};
	hdrl_value secansZdist = {1. / cosz.data, cosz.error * fabs(-1. / pow(cosz.data, 2))};

    switch(type){
		case HDRL_AIRMASS_APPROX_HARDIE:
			airmass = hdrl_get_airmass_hardie(secansZdist);
			break;
        case HDRL_AIRMASS_APPROX_YOUNG_IRVINE:
        	airmass = hdrl_get_airmass_youngirvine(secansZdist);
        	break;
        case HDRL_AIRMASS_APPROX_YOUNG:
        	airmass = hdrl_get_airmass_young(cosz);
        	break;
    }

    /* if the exposure time is larger than zero, weight in airmass   *
     * at mid and end exposure according to Stetson's weights (which *
     * are also used in IRAF/astcalc for the airmass function        */
	if (aExptime.data > 0.) {

		const double weights[]  = {1./6., 2./3., 1./6.};
		const cpl_size nweights = sizeof(weights) / sizeof(double);

		hdrl_value timeStep = {aExptime.data / (nweights - 1.) * 15. / 3600. * CPL_MATH_RAD_DEG, 0.};
		timeStep.error      =  aExptime.error * fabs( 1. / (nweights - 1.) * 15. / 3600. * CPL_MATH_RAD_DEG);

		airmass.data  *= weights[0];
		airmass.error *= fabs(weights[0]);

		for (cpl_size i = 1; i < nweights; i++) {

			hdrl_value aux_hourangle = { hourangle.data  + i * timeStep.data,
										 hourangle.error + i * timeStep.error };

			cosz = hdrl_get_zenith_distance(aux_hourangle, delta, latitude);

			if (type == HDRL_AIRMASS_APPROX_HARDIE) {

				z = (hdrl_value){acos(cosz.data) * CPL_MATH_DEG_RAD, 0.};
				z.error = cosz.error * fabs(-CPL_MATH_DEG_RAD / sqrt(1. + pow(cosz.data, 2)));

				cpl_error_ensure(z.data <= zlimit, CPL_ERROR_ILLEGAL_OUTPUT, return retErr,
					"Zenith angle %f+-[%f] > %f!", z.data, z.error, zlimit);
			}

			cpl_error_ensure(
				cosz.data != 0. && fabs(1./cosz.data) >= FLT_EPSILON && acos(cosz.data) <= CPL_MATH_PI_2,
				CPL_ERROR_ILLEGAL_OUTPUT,
				return retErr,
				"timeStep. Object is below the horizon at %s exposure (z=%f).",
				i==1 ? "mid. Return the error" : "end. Return the error",
				cosz.error * fabs(-CPL_MATH_DEG_RAD / sqrt(1. + pow(cosz.data, 2))) );

			hdrl_value weight = {0., 0.};
			secansZdist = (hdrl_value){1. / cosz.data, cosz.error * fabs(-1. / pow(cosz.data, 2))};

			switch(type){
				case HDRL_AIRMASS_APPROX_HARDIE:
					weight = hdrl_get_airmass_hardie(secansZdist);
					break;
				case HDRL_AIRMASS_APPROX_YOUNG_IRVINE:
					weight = hdrl_get_airmass_youngirvine(secansZdist);
					break;
				case HDRL_AIRMASS_APPROX_YOUNG:
					weight = hdrl_get_airmass_young(cosz);
					break;
			 }
			 airmass.data  += weights[i] *weight.data;
			 airmass.error += weights[i] *weight.error;
		}
	}

	if (type == HDRL_AIRMASS_APPROX_YOUNG_IRVINE) {

		/* Accuracy limit for airmass approximation of Young & Irvine */
		const double airmasslimit = 4.;

		cpl_error_ensure(airmass.data <= airmasslimit, CPL_ERROR_ILLEGAL_OUTPUT,
			return retErr, "Airmass larger than %f", airmasslimit);
	}

	return airmass;
}

/*----------------------------------------------------------------------------*/
/**
  @private
  @brief   Compute the zenith distance for an observation.
  @param   aHourAngle   Hour angle in radians.
  @param   aDelta       Declination in radians.
  @param   aLatitude    Latitude of the observatory in radians.
  @return  cos(z) on success or 0. on error.

  The function computes the cosine of the zenith distance for an observation
  taken at an angle aHourAngle from the meridian, which can take values in the
  range extending from @f$-\pi@f$ to @f$\pi@f$, and the declination aDelta with
  possible values between @f$-0.5\pi@f$ and @f$0.5\pi@f$. The latitude
  aLatitude of the observing site may take values in the range @f$0@f$ to
  @f$2\pi@f$.
 */
/*----------------------------------------------------------------------------*/
hdrl_value hdrl_get_zenith_distance(
	hdrl_value aHourAngle, hdrl_value aDelta, hdrl_value aLatitude)
{

	hdrl_value p0 = {sin(aLatitude.data) * sin(aDelta.data),
	                  aLatitude.error * fabs( cos(aLatitude.data) * sin(aDelta.data)
			        + aDelta.error    * fabs( sin(aLatitude.data) * cos(aDelta.data)))};

	hdrl_value p1 = {cos(aLatitude.data) * cos(aDelta.data),
	                  aLatitude.error * fabs(-sin(aLatitude.data) * cos(aDelta.data)
			        + aDelta.error    * fabs(-cos(aLatitude.data) * sin(aDelta.data)))};

	hdrl_value z  = {p0.data + cos(aHourAngle.data) * p1.data,
	                  p0.error         * fabs(1.)
	                + aHourAngle.error * fabs(-sin(aHourAngle.data) * p1.data)
	                + p1.error         * fabs( cos(aHourAngle.data))};

	return fabs(z.data) < FLT_EPSILON ? (hdrl_value){0.,0.} : z;
}

/*----------------------------------------------------------------------------*/
/**
  @private
  @brief   Compute airmass with Hardie (1962) approximation.
  @param   aSecZ   The secans of the zenith distance.
  @return  The function returns the airmass.

  The function uses the approximation given by Hardie (Hardie 1962, In:
  "Astronomical Techniques", ed. Hiltner, p. 184) to compute the airmass as a
  function of zenith angle which is given in terms of its secons aSecZ.
  It is supposedy more accurate than Young & Irvine (1967) and usable for zenith
  angles below 85 degrees.
 */
/*----------------------------------------------------------------------------*/
hdrl_value hdrl_get_airmass_hardie(hdrl_value	hvaSecZ)
{
	double aSecZ    = hvaSecZ.data,
		   aSecZErr = hvaSecZ.error;

	hdrl_value secm1 = {aSecZ - 1, aSecZErr};

	hdrl_value airmass = {aSecZ - 0.0018167 * secm1.data
							    - 0.002875  * secm1.data * secm1.data
							    - 0.0008083 * secm1.data * secm1.data * secm1.data,
                          aSecZErr +secm1.error * fabs( - 0.0018167
									                    - 2. * 0.002875  * secm1.data
									                    - 3. * 0.0008083 * secm1.data * secm1.data)};

	return airmass;
}

/*----------------------------------------------------------------------------*/
/**
  @private
  @brief   Compute airmass with Young and Irvine (1997) approximation.
  @param   hvaSecZ   Secans of the zenith distance.
  @return  The function returns the airmass.

  The function uses the approximation given by Young and Irvine (Young A. T.,
  Irvine W. M., 1967, Astron. J. 72, 945) to compute the airmass for a given
  sec(z) aSecZ. This approximation takes into account atmosphere refraction and
  curvature, but is in principle only valid at sea level.
 */
/*----------------------------------------------------------------------------*/
hdrl_value hdrl_get_airmass_youngirvine(hdrl_value hvaSecZ)
{
	double aSecZ    = hvaSecZ.data,
	       aSecZErr = hvaSecZ.error;

	hdrl_value airmass = {aSecZ * (1. - 0.0012 * (pow(aSecZ, 2) - 1.)),
	                      aSecZErr * fabs( (1. - 0.0012 * (pow(aSecZ, 2) - 1.)) - 2. * 0.0012 * pow(aSecZ, 2))};

	return airmass;
}

/*----------------------------------------------------------------------------*/
/**
  @private
  @brief   Compute airmass with Young (1994) approximation.
  @param   aCosZt   The cosine of the true zenith distance.
  @return  The function returns the airmass.

  The function uses the approximation given by Young (Young A. T., 1994 ApOpt,
  33, 1108) to compute the relative optical air mass as a function of true,
  rather than refracted, zenith angle which is given in terms of its cosine
  aCosZt.
  It is supposedy more accurate than Young & Irvine (1967) but restrictions are
  not known.
 */
/*----------------------------------------------------------------------------*/
hdrl_value hdrl_get_airmass_young(hdrl_value hvaCosZt)
{
	double aCosZt    = hvaCosZt.data,
		   aCosZtErr = hvaCosZt.error;

	hdrl_value airmass = {(1.002432 * aCosZt * aCosZt + 0.148386 * aCosZt + 0.0096467)
						/ (aCosZt * aCosZt * aCosZt + 0.149864 * aCosZt * aCosZt + 0.0102963 * aCosZt
						   + 0.000303978),
                 	    aCosZtErr * fabs( ( (2. * 1.002432 * aCosZt + 0.148386) * (aCosZt * aCosZt * aCosZt + 0.149864 * aCosZt * aCosZt + 0.0102963 * aCosZt + 0.000303978)
									    - (3. * aCosZt * aCosZt + 2. * 0.149864 * aCosZt + 0.0102963) * (1.002432 * aCosZt * aCosZt + 0.148386 * aCosZt + 0.0096467)
								     	) / pow(aCosZt * aCosZt * aCosZt + 0.149864 * aCosZt * aCosZt + 0.0102963 * aCosZt + 0.000303978, 2) )};

	return airmass;
}


/**@}*/
