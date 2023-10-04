/* $Id: hdrl_image_math.h,v 1.6 2013-10-17 15:44:14 jtaylor Exp $
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
 * $Revision: 1.6 $
 * $Name: not supported by cvs2svn $
 */

#ifndef HDRL_IMAGE_MATH_H
#define HDRL_IMAGE_MATH_H
#include "hdrl_image.h"
#include "hdrl_types.h"
#include "hdrl_mode.h"
#include <cpl.h>

CPL_BEGIN_DECLS

hdrl_value hdrl_image_get_sqsum(const hdrl_image *self);
hdrl_value hdrl_image_get_sum(const hdrl_image *self);
double hdrl_image_get_stdev(const hdrl_image *self);
hdrl_value hdrl_image_get_median(const hdrl_image *self);
hdrl_value hdrl_image_get_mean(const hdrl_image *self);
hdrl_value hdrl_image_get_weighted_mean(const hdrl_image *self);
hdrl_value hdrl_image_get_sigclip_mean(const hdrl_image * self,
                                       double kappa_low, double kappa_high,
                                       int niter);
hdrl_value hdrl_image_get_minmax_mean(const hdrl_image * self, double nlow,
                                      double nhigh);
hdrl_value hdrl_image_get_mode(const hdrl_image * self, double histo_min,
			       double histo_max, double bin_size,
			       hdrl_mode_type method, cpl_size error_niter);


hdrl_image *   hdrl_image_div_image_create(const hdrl_image *self, const hdrl_image *other);
cpl_error_code hdrl_image_div_image(             hdrl_image *self, const hdrl_image *other);
cpl_error_code hdrl_image_div_scalar(            hdrl_image *self,       hdrl_value value);

hdrl_image *   hdrl_image_mul_image_create(const hdrl_image *self, const hdrl_image *other);
cpl_error_code hdrl_image_mul_image(             hdrl_image *self, const hdrl_image *other);
cpl_error_code hdrl_image_mul_scalar(            hdrl_image *self,       hdrl_value value);

hdrl_image *   hdrl_image_sub_image_create(const hdrl_image *self, const hdrl_image *other);
cpl_error_code hdrl_image_sub_image(             hdrl_image *self, const hdrl_image *other);
cpl_error_code hdrl_image_sub_scalar(            hdrl_image *self,       hdrl_value value);

hdrl_image *   hdrl_image_add_image_create(const hdrl_image *self, const hdrl_image *other);
cpl_error_code hdrl_image_add_image(             hdrl_image *self, const hdrl_image *other);
cpl_error_code hdrl_image_add_scalar(            hdrl_image *self,       hdrl_value value);

hdrl_image *   hdrl_image_pow_scalar_create(const hdrl_image *self, const hdrl_value exponent);
cpl_error_code hdrl_image_pow_scalar(             hdrl_image *self, const hdrl_value exponent);

hdrl_image *   hdrl_image_exp_scalar_create(const hdrl_image *self, const hdrl_value base);
cpl_error_code hdrl_image_exp_scalar(             hdrl_image *self, const hdrl_value base);

CPL_END_DECLS

#endif
