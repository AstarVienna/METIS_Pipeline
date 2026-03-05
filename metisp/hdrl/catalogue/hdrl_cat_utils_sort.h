/*
 * This file is part of the HDRL
 * Copyright (C) 2017 European Southern Observatory
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef HDRL_UTILS_SORT_H
#define HDRL_UTILS_SORT_H


#include "hdrl_cat_def.h"


typedef enum {
	HDRL_SORT_INT = 1,
	HDRL_SORT_DOUBLE,
	HDRL_SORT_CPL_SIZE,
	HDRL_SORT_HDRL_VALUE,
} hdrl_sort_type;

typedef int (*f_compare)(const void *, const void *);

cpl_error_code sort_array_f(void   *a, cpl_size nE, cpl_size sE, f_compare f);
cpl_error_code sort_array(  void   *a, cpl_size nE, cpl_size sE, hdrl_sort_type type, cpl_sort_direction dir);

cpl_error_code sort_array_index(  double *a, cpl_size nE, void  *b,               hdrl_sort_type  type,  cpl_sort_direction dir);
cpl_error_code sort_arrays_index( double *a, cpl_size nE, void **bs, cpl_size nA, hdrl_sort_type *types, cpl_sort_direction dir);


 #endif /* HDRL_UTILS_SORT_H */
