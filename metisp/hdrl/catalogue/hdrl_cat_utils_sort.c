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

#include <string.h>

#include "hdrl_cat_utils_sort.h"

#include "hdrl_types.h"


/*** Datatypes ***/

typedef struct {
	double data;
	int    index;
} sort_index;


/*** Prototypes ***/

static int cmp_index_asc(     const void *a, const void *b);
static int cmp_int_asc(       const void *a, const void *b);
static int cmp_double_asc(    const void *a, const void *b);
static int cmp_cpl_size_asc(  const void *a, const void *b);
static int cmp_hdrl_value_asc(const void *a, const void *b);

static int cmp_index_des(     const void *a, const void *b);
static int cmp_int_des(       const void *a, const void *b);
static int cmp_double_des(    const void *a, const void *b);
static int cmp_cpl_size_des(  const void *a, const void *b);
static int cmp_hdrl_value_des(const void *a, const void *b);

static cpl_error_code sort_and_gen_index(double *a, cpl_size nE, sort_index *a_index, cpl_sort_direction dir);
static cpl_error_code sort_array_using_index(sort_index *a_index, cpl_size nE, void *b, hdrl_sort_type type);


/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_utils_sort         hdrl_utils_sort
 * @ingroup  Catalogue
 *
 * @brief    Sort functions for differents data types
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    sort_array_f      Core sort algorith that it's called with the
 *                             other sort function. If you need to changed the
 *                             the method of sort this is the function that
 *                             you need modified.
 *
 * @param    a         generic pointer array  with one unknown datatype
 * @param    nE        Number of elements in the array
 * @param    sE        Size of element in the array
 * @param    f         Function for evaluate the comparison between elements
 *
 * @return   cpl_error_code.   Whit qsort() always return CPL_ERROR_NONE because
 *                             qsort only return a void. If it change maybe it
 *                             is possible to check it.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code sort_array_f(void *a, cpl_size nE, cpl_size sE, f_compare f)
{
	/* Generic funtion in C99 STD standard.
	 * All the rest of the functions call, at the end, this function.
	 * If you need to change the method of sort and affect all of hdrl library,
	 * you need to change only that part. */
	qsort(a, nE, sE, f);

	return CPL_ERROR_NONE;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    sort_array        hdrl function for order arrays with know types.
 *                             Using the type parameter for select the comparation function
 *
 * @param    a         generic pointer array  with one unknown datatype
 * @param    nE        Number of elements in the array
 * @param    sE        Size of element in the array
 * @param    type      Datatype of elements in the 'a' generic array. Standard type in hdrl_sort_type
 * @param    dir       Direction of sort array (Ascending or Descending)
 *
 * @return   cpl_error_code.   Return the error code provided by the sort function.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code sort_array(void *a, cpl_size nE, cpl_size sE, hdrl_sort_type type, cpl_sort_direction dir)
{
	f_compare f;

	if (dir == CPL_SORT_ASCENDING) {

		/* Ascending order hdrl_sort_type functions */
		switch(type) {
			case HDRL_SORT_INT :        f = &cmp_int_asc;        break;
			case HDRL_SORT_DOUBLE :     f = &cmp_double_asc;     break;
			case HDRL_SORT_CPL_SIZE :   f = &cmp_cpl_size_asc;   break;
			case HDRL_SORT_HDRL_VALUE : f = &cmp_hdrl_value_asc; break;
			default : return CPL_ERROR_ILLEGAL_INPUT;
		}

	} else {

		/* Descending order hdrl_sort_type functions */
		switch(type) {
			case HDRL_SORT_INT :        f = &cmp_int_des;        break;
			case HDRL_SORT_DOUBLE :     f = &cmp_double_des;     break;
			case HDRL_SORT_CPL_SIZE :   f = &cmp_cpl_size_des;   break;
			case HDRL_SORT_HDRL_VALUE : f = &cmp_hdrl_value_des; break;
			default : return CPL_ERROR_ILLEGAL_INPUT;
		}
	}

	/* Sort the generic vector */
	return sort_array_f(a, nE, sE, f);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    sort_array_index  hdrl function for sort two arrays
 *                             The alghorithm sort 'a' and in the same way
 *                             sort the other generic array by the index of 'a'.
 *
 * @param    a         generic pointer array  with doubles
 * @param    nE        Number of elements in the array of doubles
 * @param    b         generic array that you need to order by index of the first array
 * @param    type      Datatype of elements in the 'b' generic array. Standard type in hdrl_sort_type
 * @param    dir       Direction of sort the array (Ascending or Descending)
 *
 * @return   cpl_error_code.   Return the error code provided by the sort functions.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code sort_array_index(double *a, cpl_size nE, void *b, hdrl_sort_type type, cpl_sort_direction dir)
{
	/* Reserve memory for the index */
	sort_index *a_index = cpl_malloc(nE * sizeof(sort_index));

	/* Sort 'a' double array and generate the index sort */
	cpl_error_code e = sort_and_gen_index(a, nE, a_index, dir);
	if (e == CPL_ERROR_NONE) {

		/* Sort the 'b' generic array with the index sort in 'a' */
		e = sort_array_using_index(a_index, nE, b, type);
	}

	/* Cleanup */
	cpl_free(a_index);

	return e;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    sort_arrays_index hdrl function for sort several arrays
 *                             The alghorithm sort 'a' and in the same way
 *                             sort the others generic arrays by the index of 'a'.
 *
 * @param    a         generic pointer array  with doubles
 * @param    nE        Number of elements in the array of doubles
 * @param    bs        generic arrays that you need to order by index of the first array
 * @param    nA        Number of generic arrays
 * @param    types     Datatypes of elements in the 'bs' generic arrays. Standard type in hdrl_sort_type
 * @param    dir       Direction of sort the arrays (Ascending or Descending)
 *
 * @return   cpl_error_code.   Return the error code provided by the sort functions.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code sort_arrays_index(double *a, cpl_size nE, void **bs, cpl_size nA, hdrl_sort_type *types, cpl_sort_direction dir)
{
	/* Reserve memory for the index */
	sort_index *a_index = cpl_malloc(nE * sizeof(sort_index));

	/* Sort 'a' double array and generate the index sort */
	cpl_error_code e = sort_and_gen_index(a, nE, a_index, dir);
	for (cpl_size i = 0; i < nA && e == CPL_ERROR_NONE; i++) {

		/* Sort the 'bs[i]' generic array with the index sort in 'a' */
		 e = sort_array_using_index(a_index, nE, bs[i], types[i]);
	}

	/* Cleanup */
	cpl_free(a_index);

	return e;
}

/**@}*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    cmp_index_asc     Function for sort in ascending order elements of
 *                             the type sort_index (data field).
 *
 * @param    a         generic pointer to an element of the array (first  in the comparison)
 * @param    b         generic pointer to an element of the array (second in the comparison)
 *
 * @return   result    -1: If 'a' element is less    than 'b'
 *                      1: If 'a' element is greater than 'b'
 *                      0: If 'a' element is equal   than 'b'
 */
/* ---------------------------------------------------------------------------*/
static int cmp_index_asc(const void *a, const void *b) {

	const sort_index *a_d = (const sort_index*)a;
	const sort_index *b_d = (const sort_index*)b;

	return a_d->data < b_d->data ? -1 : (a_d->data > b_d->data ? 1 : 0);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    cmp_int_asc       Function for sort in ascending order elements of
 *                             the type int
 *
 * @param    a         generic pointer to an element of the array (first  in the comparison)
 * @param    b         generic pointer to an element of the array (second in the comparison)
 *
 * @return   result    -1: If 'a' element is less    than 'b'
 *                      1: If 'a' element is greater than 'b'
 *                      0: If 'a' element is equal   than 'b'
 */
/* ---------------------------------------------------------------------------*/
static int cmp_int_asc(const void * a, const void * b)
{
	const int a_d = *(const int *)a;
    const int b_d = *(const int *)b;

	return a_d < b_d ? -1 : (a_d > b_d ? 1 : 0);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    cmp_double_asc    Function for sort in ascending order elements of
 *                             the type double
 *
 * @param    a         generic pointer to an element of the array (first  in the comparison)
 * @param    b         generic pointer to an element of the array (second in the comparison)
 *
 * @return   result    -1: If 'a' element is less    than 'b'
 *                      1: If 'a' element is greater than 'b'
 *                      0: If 'a' element is equal   than 'b'
 */
/* ---------------------------------------------------------------------------*/
static int cmp_double_asc(const void * a, const void * b)
{
	const double a_d = *(const double *)a;
    const double b_d = *(const double *)b;

	return a_d < b_d ? -1 : (a_d > b_d ? 1 : 0);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    cmp_cpl_size_asc  Function for sort in ascending order elements of
 *                             the type cpl_size
 *
 * @param    a         generic pointer to an element of the array (first  in the comparison)
 * @param    b         generic pointer to an element of the array (second in the comparison)
 *
 * @return   result    -1: If 'a' element is less    than 'b'
 *                      1: If 'a' element is greater than 'b'
 *                      0: If 'a' element is equal   than 'b'
 */
/* ---------------------------------------------------------------------------*/
static int cmp_cpl_size_asc(const void *a, const void *b)
{
	const cpl_size a_cs = *(const cpl_size *)a;
    const cpl_size b_cs = *(const cpl_size *)b;

	return a_cs < b_cs ? -1 : (a_cs > b_cs ? 1 : 0);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    cmp_hdrl_value_asc Function for sort in ascending order elements of
 *                              the type hdrl_value (data field).
 *
 * @param    a         generic pointer to an element of the array (first  in the comparison)
 * @param    b         generic pointer to an element of the array (second in the comparison)
 *
 * @return   result    -1: If 'a' element is less    than 'b'
 *                      1: If 'a' element is greater than 'b'
 *                      0: If 'a' element is equal   than 'b'
 */
/* ---------------------------------------------------------------------------*/
static int cmp_hdrl_value_asc(const void *a, const void *b)
{
	const hdrl_value *a_hv = (const hdrl_value*)a;
	const hdrl_value *b_hv = (const hdrl_value*)b;

	return a_hv->data < b_hv->data ? -1 : (a_hv->data > b_hv->data ? 1 : 0);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    cmp_index_des     Function for sort in descending order elements of
 *                             the type sort_index (data field).
 *
 * @param    a         generic pointer to an element of the array (first  in the comparison)
 * @param    b         generic pointer to an element of the array (second in the comparison)
 *
 * @return   result    -1: If 'a' element is greater than 'b'
 *                      1: If 'a' element is less    than 'b'
 *                      0: If 'a' element is equal   than 'b'
 */
/* ---------------------------------------------------------------------------*/
static int cmp_index_des(const void *a, const void *b) {

	const sort_index *a_d = (const sort_index*)a;
	const sort_index *b_d = (const sort_index*)b;

	return a_d->data > b_d->data ? -1 : (a_d->data < b_d->data ? 1 : 0);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    cmp_int_des       Function for sort in descending order elements of
 *                             the type int
 *
 * @param    a         generic pointer to an element of the array (first  in the comparison)
 * @param    b         generic pointer to an element of the array (second in the comparison)
 *
 * @return   result    -1: If 'a' element is greater than 'b'
 *                      1: If 'a' element is less    than 'b'
 *                      0: If 'a' element is equal   than 'b'
 */
/* ---------------------------------------------------------------------------*/
static int cmp_int_des(const void * a, const void * b)
{
	const int a_d = *(const int *)a;
    const int b_d = *(const int *)b;

	return a_d > b_d ? -1 : (a_d < b_d ? 1 : 0);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    cmp_double_des    Function for sort in descending order elements of
 *                             the type double
 *
 * @param    a         generic pointer to an element of the array (first  in the comparison)
 * @param    b         generic pointer to an element of the array (second in the comparison)
 *
 * @return   result    -1: If 'a' element is greater than 'b'
 *                      1: If 'a' element is less    than 'b'
 *                      0: If 'a' element is equal   than 'b'
 */
/* ---------------------------------------------------------------------------*/
static int cmp_double_des(const void * a, const void * b)
{
	const double a_d = *(const double *)a;
    const double b_d = *(const double *)b;

	return a_d > b_d ? -1 : (a_d < b_d ? 1 : 0);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    cmp_cpl_size_des  Function for sort in descending order elements of
 *                             the type cpl_size
 *
 * @param    a         generic pointer to an element of the array (first  in the comparison)
 * @param    b         generic pointer to an element of the array (second in the comparison)
 *
 * @return   result    -1: If 'a' element is greater than 'b'
 *                      1: If 'a' element is less    than 'b'
 *                      0: If 'a' element is equal   than 'b'
 */
/* ---------------------------------------------------------------------------*/
static int cmp_cpl_size_des(const void *a, const void *b)
{
	const cpl_size a_cs = *(const cpl_size *)a;
    const cpl_size b_cs = *(const cpl_size *)b;

	return a_cs > b_cs ? -1 : (a_cs < b_cs ? 1 : 0);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    cmp_hdrl_value_des Function for sort in descending order elements of
 *                              the type hdrl_value (data field).
 *
 * @param    a         generic pointer to an element of the array (first  in the comparison)
 * @param    b         generic pointer to an element of the array (second in the comparison)
 *
 * @return   result    -1: If 'a' element is greater than 'b'
 *                      1: If 'a' element is less    than 'b'
 *                      0: If 'a' element is equal   than 'b'
 */
/* ---------------------------------------------------------------------------*/
static int cmp_hdrl_value_des(const void *a, const void *b)
{
	const hdrl_value *a_hv = (const hdrl_value*)a;
	const hdrl_value *b_hv = (const hdrl_value*)b;

	return a_hv->data > b_hv->data ? -1 : (a_hv->data < b_hv->data ? 1 : 0);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    sort_and_gen_index hdrl function for sort the 'a' double array
 *                              and fill the in/out array with index (type sort_index[])
 *
 * @param    a         pointer to an double array
 * @param    nE        Number of elements in the array of doubles
 * @param    a_index   Output vector that it's necessary to fill with the index order.
 * @param    dir       Direction of sort the arrays (Ascending or Descending)
 *
 * @return   cpl_error_code.   Return the error code provided by the sort functions.
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code sort_and_gen_index(double *a, cpl_size nE, sort_index *a_index, cpl_sort_direction dir){

	/* Update the a_index vector with the values in 'a' array and your position
	 * in the array for sort with index */
	for (cpl_size i = 0; i < nE; i++) {
		a_index[i].data  = a[i];
		a_index[i].index = i;
	}

	/* Sort array in ascending/descending order the array with data and index */
	cpl_error_code e;
	if (dir == CPL_SORT_ASCENDING) {
		e = sort_array_f(a_index, nE, sizeof(*a_index), &cmp_index_asc);
	} else {
		e = sort_array_f(a_index, nE, sizeof(*a_index), &cmp_index_des);
	}

	/* Update the original 'a' double array and keep the sort apply in a_index array */
	if (e == CPL_ERROR_NONE ) {
		for (cpl_size i = 0; i < nE; i++) {
			a[i] = a_index[i].data;
		}
	}

	return e;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    sort_array_using_index
 *                     hdrl function for sort several arrays following the input index
 *
 * @param    a_index   pointer to an sort_index array with the index to order the 'b' array.
 * @param    nE        Number of elements in the array 'a_index' and 'b'
 * @param    type      Datatype of elements in the 'b' generic array. Standard type in hdrl_sort_type
 *
 * @return   cpl_error_code.   Return the error code provided by the sort functions.
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code sort_array_using_index(sort_index *a_index, cpl_size nE, void *b, hdrl_sort_type type)
{
	if (type == HDRL_SORT_INT) {

		/* Cast generic 'b' array to int
		 * Copy the values in a auxiliary vector
		 * and apply the sort in the a_index array using the copy values */
		int *b_cast = (int *)b;
		int b_aux[nE];
		for (cpl_size i = 0; i < nE; i++) {
			b_aux[i] = b_cast[i];
		}
		for (cpl_size i = 0; i < nE; i++) {
			b_cast[i] = b_aux[a_index[i].index];
		}

	} else if (type == HDRL_SORT_DOUBLE) {

		/* Cast generic 'b' array to double
		 * Copy the values in a auxiliary vector
		 * and apply the sort in the a_index array using the copy values */
		double *b_cast = (double *)b;
		double b_aux[nE];
		for (cpl_size i = 0; i < nE; i++) {
			b_aux[i] = b_cast[i];
		}
		for (cpl_size i = 0; i < nE; i++) {
			b_cast[i] = b_aux[a_index[i].index];
		}

	} else if (type == HDRL_SORT_CPL_SIZE) {

		/* Cast generic 'b' array to cpl_size
		 * Copy the values in a auxiliary vector
		 * and apply the sort in the a_index array using the copy values */
		cpl_size *b_cast = (cpl_size *)b;
		cpl_size b_aux[nE];
		for (cpl_size i = 0; i < nE; i++) {
			b_aux[i] = b_cast[i];
		}
		for (cpl_size i = 0; i < nE; i++) {
			b_cast[i] = b_aux[a_index[i].index];
		}

	} else if (type == HDRL_SORT_HDRL_VALUE) {

		/* Cast generic 'b' array to hdrl_value
		 * Copy the values in a auxiliary vector
		 * and apply the sort in the a_index array using the copy values */
		hdrl_value *b_cast = (hdrl_value *)b;
		hdrl_value b_aux[nE];
		for (cpl_size i = 0; i < nE; i++) {
			b_aux[i] = b_cast[i];
		}
		for (cpl_size i = 0; i < nE; i++) {
			b_cast[i] = b_aux[a_index[i].index];
		}

	} else {
		return CPL_ERROR_ILLEGAL_INPUT;
	}

	return CPL_ERROR_NONE;
}
