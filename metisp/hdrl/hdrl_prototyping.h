/*
 * This file is part of the HDRL
 * Copyright (C) 2014 European Southern Observatory
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

#ifndef HDRL_PROTOTYPING_H
#define HDRL_PROTOTYPING_H

#include <cpl.h>

cpl_image * hdrl_get_spatial_freq(cpl_image *ima, double gausfilt, int mirrorx,
                                  int mirrory);

cpl_image * hdrl_mime_image_polynomial_bkg(cpl_image * image,
                                            int dim_X, int dim_Y,
                                            cpl_matrix ** coeffs);


cpl_error_code hdrl_mime_compute_polynomial_bkg(
    const cpl_imagelist       * images,
          cpl_imagelist       * bkg_images,
          int                   dim_X,
          int                   dim_Y,
          cpl_matrix         ** coeffs
   );
cpl_matrix *hdrl_mime_legendre_tensors_create(int nx, int ny, int npx, int npy);

cpl_matrix *hdrl_mime_matrix_linspace_create(int n, double a, double b);

cpl_matrix *hdrl_mime_legendre_polynomials_create(int npoly, double a, double b,
                                             const cpl_matrix * x);

cpl_matrix *hdrl_mime_linalg_pairwise_column_tensor_products_create(const
          cpl_matrix * mat1, const cpl_matrix * mat2);

cpl_error_code hdrl_mime_matrix_copy_column(const cpl_matrix * mat1, int j_1,
          cpl_matrix * mat2, int j_2);

cpl_matrix *hdrl_mime_linalg_tensor_products_columns_create(const cpl_matrix *
                                                       mat1, const cpl_matrix * mat2);
cpl_matrix *hdrl_mime_tensor_weights_create(int nx, int ny);

cpl_error_code hdrl_mime_matrix_rescale_rows(const cpl_matrix * mat,
          const cpl_matrix * d, cpl_matrix * dmat);

cpl_error_code hdrl_mime_matrix_mask_rows(cpl_matrix * mat, const cpl_mask * mask);

cpl_matrix *hdrl_mime_linalg_solve_tikhonov(const cpl_matrix * mat,
          const cpl_matrix * rhs, double alpha);

cpl_matrix *hdrl_mime_linalg_normal_equations_create(const cpl_matrix * mat,
          double alpha);
cpl_matrix *hdrl_mime_matrix_product_left_transpose_create(const cpl_matrix *
          mat1, const cpl_matrix * mat2);

cpl_error_code hdrl_mime_matrix_product(const cpl_matrix * mat1,
          const cpl_matrix * mat2, cpl_matrix * product);


#endif /* HDRL_PROTOTYPING_H */
