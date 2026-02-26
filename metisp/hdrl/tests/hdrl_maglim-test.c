/*
 * This file is part of the HDRL
 * Copyright (C) 2016 European Southern Observatory
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

#include "hdrl_maglim.h"

#include <cpl.h>
#include <stdint.h>
#include <math.h>



/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_maglim_test
            Testing of hdrl_maglim module
 */
/*----------------------------------------------------------------------------*/

/**
  @brief   Convert kernel to image (can be used for debugging purposes)

  @param   kernel the input kernel
  @param   normalise if TRUE normalise image
  @return   the output image

 */
static cpl_image*
hdrl_matrix_to_image_create(const cpl_matrix * kernel,
			    const cpl_boolean normalise) {

  cpl_ensure(kernel != NULL, CPL_ERROR_NULL_INPUT, NULL);
  cpl_size sx = cpl_matrix_get_ncol(kernel);
  cpl_size sy = cpl_matrix_get_nrow(kernel);

  cpl_image* image = cpl_image_new(sx, sy, CPL_TYPE_DOUBLE);
  double* pimage = cpl_image_get_data(image);
  const double* pkernel = cpl_matrix_get_data_const(kernel);
  cpl_size pix, piy;
  for(cpl_size j = 0; j < sy; j++) {
      piy = sx*j;
      for(cpl_size i = 0; i < sx; i++) {
	  pix = piy + i;
	  pimage[pix] = pkernel[pix];
      }
  }
  if(normalise) {
      double sum = cpl_matrix_get_mean(kernel);
      sum *= sx * sy;
      cpl_image_divide_scalar(image, sum);
  }
  return image;

}




cpl_error_code
test_hdrl_extend_image(void)
{
  cpl_image* ima;
  cpl_image* ima1;
  double* pinp;
  const cpl_size border_nx = 3;
  const cpl_size border_ny = 5;
  hdrl_image_extend_method extend_method = HDRL_IMAGE_EXTEND_NEAREST;

  cpl_size nx = 100;
  cpl_size ny = 100;
  ima = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);
  ima1 = cpl_image_new(nx, ny, CPL_TYPE_DOUBLE);

  for(cpl_size i = 1; i <= nx; i++) {
      cpl_image_fill_window(ima1, i, 1, i, ny, i-1);
  }
  cpl_image_add(ima,ima1);

  //cpl_image_save(ima, "ima1.fits", CPL_TYPE_DOUBLE, NULL, CPL_IO_DEFAULT);
  for(cpl_size j = 1; j <= ny; j++) {
      cpl_image_fill_window(ima1, 1, j, nx, j, (j-1) * 100);
  }
  //cpl_image_save(ima1, "ima2.fits", CPL_TYPE_DOUBLE, NULL, CPL_IO_DEFAULT);
  cpl_image_add(ima,ima1);
  //cpl_image_save(ima, "ima3.fits", CPL_TYPE_DOUBLE, NULL, CPL_IO_DEFAULT);
  cpl_image_delete(ima1);

  cpl_image* extended;

  extended = hdrl_extend_image(NULL, border_nx, border_ny, extend_method);
  cpl_test_error(CPL_ERROR_NULL_INPUT);
  cpl_test_null(extended);

  extended = hdrl_extend_image(ima, -1, border_ny, extend_method);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(extended);

  extended = hdrl_extend_image(ima, border_nx, -1, extend_method);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(extended);

  extended = hdrl_extend_image(ima, border_nx, border_ny, 3);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(extended);

  double* pext = cpl_image_get_data_double(ima);
  for(cpl_size j = border_ny; j <= ny-border_ny; j++) {
      for(cpl_size i = border_nx; i <= nx-border_nx; i++) {
	  cpl_test_abs(pext[j*nx+i], j*nx+i, HDRL_EPS_DATA);
      }
  }
  extended = hdrl_extend_image(ima, border_nx, border_ny, extend_method);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(extended);

  pext = cpl_image_get_data_double(extended);
  pinp = cpl_image_get_data_double(ima);
  /* the first (and last) border_nx X pixels and border_ny Y pixels of extended
   * image have same value as 1st X pixel and Y pixel of input image
   */
  cpl_size sx = cpl_image_get_size_x(extended);
  cpl_size sy = cpl_image_get_size_y(extended);
  /* check central Y range */
  for(cpl_size j = border_ny; j < sy-border_ny; j++) {
      //cpl_msg_info(cpl_func,"j=%lld",j);
      /* check early X range (i=0)*/
      for(cpl_size i = 0; i < border_nx; i++) {
	  cpl_test_abs(pext[j*sx+i], pinp[(j-border_ny)*nx], HDRL_EPS_DATA);
      }
      // check end X range (i=nx-1)
      for(cpl_size i = sx-border_nx; i < sx; i++) {
	  //cpl_msg_info(cpl_func,"i=%lld",i);
	  cpl_test_abs(pext[j*sx+i], pinp[(j-border_ny)*nx+(nx-1)], HDRL_EPS_DATA);
      }
  }


  /* check early Y range */
  for(cpl_size j = 0; j < border_ny; j++) {
      // check central X range
      for(cpl_size i = border_nx; i < sx-border_nx; i++) {
	  cpl_test_abs(pext[j*sx+i], pinp[0*nx+i-border_nx], HDRL_EPS_DATA);
      }                                /* 0 is first j index of input image */
  }


  /* check end Y range */
  for(cpl_size j = sy - border_ny; j < sy; j++) {
      //cpl_msg_info(cpl_func,"j=%lld",j);
      // check central X range
      for(cpl_size i = border_nx; i < sx-border_nx; i++) {
	  //cpl_msg_info(cpl_func,"i=%lld",i);
	  cpl_test_abs(pext[j*sx+i], pinp[(ny-1)*nx+i-border_nx], HDRL_EPS_DATA);
      }                                 /* ny-1 is last j index of input image */
  }

  cpl_image_delete(extended);
  cpl_image_delete(ima);
  return cpl_error_get_code();
}

cpl_error_code
test_hdrl_maglim_kernel_create(void)
{
  const cpl_size kernel_sx = 9;
  const cpl_size kernel_sy = 9;
  /* To simplify this unit test we choose
   * fwhm_seeing = sigma_to_fwhm=sqrt(4*log(4))
   * so that exponential: exp{-[(x^2+y^2)/2.0*(fwhm_seeing/sigma_to_fwhm)^2]}
   * simplifies to exp[-(x^2+y^2)/2.0)]
   * sqrt(4*log(4))=2,35482004503
   */
  const double fwhm_seeing = sqrt(4*log(4));
  cpl_matrix* kernel_mat;
  kernel_mat = hdrl_maglim_kernel_create(-1, kernel_sy, fwhm_seeing);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(kernel_mat);

  kernel_mat = hdrl_maglim_kernel_create(kernel_sx, -1, fwhm_seeing);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(kernel_mat);

  kernel_mat = hdrl_maglim_kernel_create(kernel_sx, kernel_sy, -1);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(kernel_mat);

  kernel_mat = hdrl_maglim_kernel_create(kernel_sx, kernel_sy, fwhm_seeing);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(kernel_mat);
  double arg;
  double x, y;

  for(cpl_size j = 0; j < kernel_sy; j++) {
      y =  j - 0.5 * (kernel_sy-1);

      for(cpl_size i = 0; i < kernel_sx; i++) {

	  x =  i - 0.5 * (kernel_sx-1);
	  arg = (x * x + y * y) / 2;
          cpl_test_abs(cpl_matrix_get(kernel_mat,j,i), exp(-arg), HDRL_EPS_DATA);

      }
  }

  cpl_image*
  kernel_ima = hdrl_matrix_to_image_create(kernel_mat, CPL_TRUE);
  //cpl_image_save(kernel_ima, "kernel_ima.fits", CPL_TYPE_DOUBLE, NULL, CPL_IO_DEFAULT);
  cpl_image_delete(kernel_ima);

  cpl_matrix_delete(kernel_mat);
  return cpl_error_get_code();
}

static cpl_image*
test_util_crea_9x9_annular_image(void) {
  /* We test with an image 9x9 with
    * center at 100, then
    * 1st annular at 90,
    * 2nd annular at 80
    */
  cpl_image* image = cpl_image_new(9, 9, CPL_TYPE_DOUBLE);
  cpl_image_add_scalar(image, 60);
  cpl_image_fill_window(image, 2, 2, 8, 8, 70);
  cpl_image_fill_window(image, 3, 3, 7, 7, 80);
  cpl_image_fill_window(image, 4, 4, 6, 6, 90);
  cpl_image_fill_window(image, 5, 5, 5, 5, 100);

  return image;
}
cpl_error_code
test_hdrl_image_convolve(void)
{
  cpl_image    *   input_image ;
  cpl_matrix   *   kernel;
  cpl_image   *   kernel_image;
  hdrl_image_extend_method method = HDRL_IMAGE_EXTEND_MIRROR;
  cpl_image* convolved;
  const cpl_size kernel_sx = 9;
  const cpl_size kernel_sy = 9;
  const double fwhm_seeing = sqrt(4*log(4));
  const int sx = 100;
  const int sy = 100;

  input_image = cpl_image_new(sx, sy, CPL_TYPE_DOUBLE);
  cpl_image_add_scalar(input_image,1);

  kernel = hdrl_maglim_kernel_create(kernel_sx, kernel_sy, fwhm_seeing);
  kernel_image = hdrl_matrix_to_image_create(kernel, CPL_TRUE);
  //cpl_image_save(kernel_image, "kernel_image.fits", CPL_TYPE_DOUBLE, NULL, CPL_IO_DEFAULT);
  cpl_image_delete(kernel_image);

  convolved = hdrl_image_convolve(NULL, kernel, method);
  cpl_test_error(CPL_ERROR_NULL_INPUT);
  cpl_test_null(convolved);

  convolved = hdrl_image_convolve(input_image, NULL, method);
  cpl_test_error(CPL_ERROR_NULL_INPUT);
  cpl_test_null(convolved);

  convolved = hdrl_image_convolve(input_image, kernel, 2);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
  cpl_test_null(convolved);

  //cpl_image_save(input_image, "input_image.fits", CPL_TYPE_DOUBLE, NULL, CPL_IO_DEFAULT);
  convolved = hdrl_image_convolve(input_image, kernel, method);
  //cpl_image_save(convolved, "convolved.fits", CPL_TYPE_DOUBLE, NULL, CPL_IO_DEFAULT);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_nonnull(convolved);
  cpl_image_delete(input_image);
  cpl_matrix_delete(kernel);
  cpl_image_delete(convolved);

   /*
   * Kernel 3x3, central pixel 1.
   */

  input_image = test_util_crea_9x9_annular_image();
  //cpl_image_save(input_image, "convolved.fits", CPL_TYPE_DOUBLE, NULL, CPL_IO_DEFAULT);

  kernel = cpl_matrix_new(3, 3);
  cpl_matrix_set(kernel, 1, 1, 1);
  cpl_matrix_set(kernel, 1, 0, 0.7);
  cpl_matrix_set(kernel, 0, 1, 0.7);
  cpl_matrix_set(kernel, 2, 1, 0.7);
  cpl_matrix_set(kernel, 1, 2, 0.7);
  kernel_image = hdrl_matrix_to_image_create(kernel, CPL_FALSE);
  //cpl_image_save(kernel_image, "kernel_image.fits", CPL_TYPE_DOUBLE, NULL, CPL_IO_DEFAULT);
  cpl_image_delete(kernel_image);

  convolved = hdrl_image_convolve(input_image, kernel, method);
  //cpl_image_save(convolved, "convolved.fits", CPL_TYPE_DOUBLE, NULL, CPL_IO_DEFAULT);

  int status = 0;
  //cpl_msg_info(cpl_func,"value: %16.16g",cpl_image_get(convolved, 4, 4, &status));
  /* test some values */
  cpl_test_abs(cpl_image_get(convolved, 5, 5, &status),92.63157894736842, HDRL_EPS_DATA);
  cpl_test_abs(cpl_image_get(convolved, 4, 5, &status),90.0, HDRL_EPS_DATA);
  cpl_test_abs(cpl_image_get(convolved, 3, 5, &status),80.0, HDRL_EPS_DATA);
  cpl_test_abs(cpl_image_get(convolved, 2, 5, &status),70.0, HDRL_EPS_DATA);
  cpl_test_abs(cpl_image_get(convolved, 1, 5, &status),61.8421052631579, HDRL_EPS_DATA);
  cpl_test_abs(cpl_image_get(convolved, 1, 1, &status),60.0, HDRL_EPS_DATA);
  cpl_test_abs(cpl_image_get(convolved, 2, 2, &status),66.31578947368422, HDRL_EPS_DATA);
  cpl_test_abs(cpl_image_get(convolved, 3, 3, &status),76.31578947368422, HDRL_EPS_DATA);
  cpl_test_abs(cpl_image_get(convolved, 4, 4, &status),86.31578947368422, HDRL_EPS_DATA);
  cpl_image_delete(convolved);
  cpl_image_delete(input_image);
  cpl_matrix_delete(kernel);
  return cpl_error_get_code();
}

cpl_error_code
test_hdrl_matrix_to_image_create(void)
{
  cpl_matrix* kernel;
  cpl_image* image;
  const cpl_size kernel_sx = 9;
  const cpl_size kernel_sy = 9;
  const double fwhm_seeing = sqrt(4*log(4));


  kernel = hdrl_maglim_kernel_create(kernel_sx, kernel_sy, fwhm_seeing);

  image = hdrl_matrix_to_image_create(NULL, CPL_TRUE);
  cpl_test_error(CPL_ERROR_NULL_INPUT);
  cpl_test_null(image);

  image = hdrl_matrix_to_image_create(kernel, CPL_TRUE);
  cpl_test_nonnull(image);

  cpl_test_abs(cpl_image_get_size_x(image), kernel_sx, HDRL_EPS_DATA);
  cpl_test_abs(cpl_image_get_size_y(image), kernel_sy, HDRL_EPS_DATA);

  cpl_image_delete(image);
  cpl_matrix_delete(kernel);
  return cpl_error_get_code();
}

cpl_error_code
test_hdrl_maglim_compute(void) {
  cpl_image* image;
  double zeropoint = 0;
  double fwhm_seeing = sqrt(4*log(4));;
  const cpl_size kernel_sx = 9;
  const cpl_size kernel_sy = 9;
  double limiting_magnitude = 0;

  hdrl_image_extend_method method = HDRL_IMAGE_EXTEND_MIRROR;
  double histo_min = 0.;
  double histo_max = 0.;
  double bin_size = 0.;
  cpl_size error_niter = 0;
  hdrl_mode_type mode_method = HDRL_MODE_MEDIAN;
  hdrl_parameter * mode_parameter = hdrl_collapse_mode_parameter_create(histo_min,
  							    histo_max,
  							    bin_size,
  							    mode_method,
  							    error_niter);
  /* testinvalid input */
  hdrl_maglim_compute(NULL, zeropoint, fwhm_seeing, kernel_sx, kernel_sy,
  	            method, mode_parameter, &limiting_magnitude);
  cpl_test_error(CPL_ERROR_NULL_INPUT);

  image = test_util_crea_9x9_annular_image();
  hdrl_maglim_compute(image, zeropoint, -1, kernel_sx, kernel_sy,
  	            method, mode_parameter, &limiting_magnitude);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);


  hdrl_maglim_compute(image, zeropoint, fwhm_seeing, -1, kernel_sy,
  	            method, mode_parameter, &limiting_magnitude);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

  hdrl_maglim_compute(image, zeropoint, fwhm_seeing, kernel_sx, -1,
  	            method, mode_parameter, &limiting_magnitude);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);


  hdrl_maglim_compute(image, zeropoint, fwhm_seeing, kernel_sx, kernel_sy,
  	            -1, mode_parameter, &limiting_magnitude);
  cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

  hdrl_maglim_compute(image, zeropoint, fwhm_seeing, kernel_sx, kernel_sy,
    	            method, NULL, &limiting_magnitude);
  cpl_test_error(CPL_ERROR_INCOMPATIBLE_INPUT);


  hdrl_maglim_compute(image, zeropoint, fwhm_seeing, kernel_sx, kernel_sy,
    	            method, mode_parameter, &limiting_magnitude);


  cpl_test_abs(limiting_magnitude,-5.591854160255954,HDRL_EPS_DATA);


  cpl_image_delete(image);
  hdrl_parameter_delete(mode_parameter);
  return cpl_error_get_code();
}


/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of hdrl_image
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_INFO);

    test_hdrl_extend_image();
    test_hdrl_maglim_kernel_create();
    test_hdrl_image_convolve();
    test_hdrl_matrix_to_image_create();
    test_hdrl_maglim_compute();

    return cpl_test_end(0);
}
