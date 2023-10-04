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

#include <cpl_test.h>

#include "../hdrl_cat_utils_sort.h"

#include "hdrl_random.h"
#include "hdrl_types.h"


/* Number of elements of arrays in the random test */
#define N                     10000

/* The different methods to fill with numbers the random arrays */
#define RANDOM_INIT           0
#define RANDOM_UNIFORM_DOBULE 1
#define RANDOM_NORMAL         2
#define RANDOM_POISSON        3
#define RANDOM_END            4


/* Comparing function with ascending sort direction */
int comparation_asc(const void * a, const void * b)
{
	const double a_d = *(const double *)a;
    const double b_d = *(const double *)b;

	return a_d < b_d ? -1 : (a_d > b_d ? 1 : 0);
}

/* Compraring Function with descending sort direction */
int comparation_des(const void * a, const void * b)
{
	const double a_d = *(const double *)a;
    const double b_d = *(const double *)b;

	return a_d > b_d ? -1 : (a_d < b_d ? 1 : 0);
}

void test_sort(double *a, double *b, cpl_vector *c_vec, cpl_size random_type, cpl_sort_direction dir) {

	/* Initialize the seed for to have repeteability with the results
	 * and to be sure of test the same numbers */
	double rd;
	hdrl_random_state *state = hdrl_random_state_new(1, NULL);

	/* Depending of the ramdom_type fill the arrays with different kind of random numbers */
    for (cpl_size i = 0; i < N; i++) {

    	switch(random_type) {
    	case RANDOM_UNIFORM_DOBULE : rd = hdrl_random_uniform_double(state, 10., 20.); break;
    	case RANDOM_NORMAL :         rd = hdrl_random_normal(state, 3.5, 1.5);         break;
    	case RANDOM_POISSON :        rd = hdrl_random_poisson(state, 100.);            break;
    	default : exit(1);
    	}

        a[i] = rd;
        b[i] = rd;
        cpl_vector_set(c_vec, i, rd);
    }
    hdrl_random_state_delete(state);

    /* Call the functions depending of the ascending/descending sort direction */
    if(dir == CPL_SORT_ASCENDING) {
		qsort(a, N, sizeof(*a), comparation_asc);
    } else {
		qsort(a, N, sizeof(*a), comparation_des);
    }
	sort_array(b, N, sizeof(*b), HDRL_SORT_DOUBLE, dir);
	cpl_vector_sort(c_vec, dir);

	/* Check the results */
    for (cpl_size i = 0; i < N; i++) {
    	cpl_test_eq(a[i], b[i]);
    	cpl_test_eq(a[i], cpl_vector_get(c_vec, i));
    	cpl_test_eq(b[i], cpl_vector_get(c_vec, i));
    }
}

void test_cmp_sort_func(cpl_sort_direction dir)
{
	double *a = cpl_malloc(N * sizeof(double));
	double *b = cpl_malloc(N * sizeof(double));
	double *c = cpl_malloc(N * sizeof(double));
    cpl_vector *c_vec = cpl_vector_wrap(N, c);

    /* Sort vector with diferent fill random numbers */
    for(cpl_size i = RANDOM_INIT + 1; i < RANDOM_END; i++) {
    	test_sort(a, b, c_vec, i, dir);
    }

    cpl_vector_unwrap(c_vec);
    cpl_free(a);
    cpl_free(b);
    cpl_free(c);
}

void test_sort_arrays_random(void)
{
	double     *a_asc = cpl_malloc(N * sizeof(double));
	double     *a_des = cpl_malloc(N * sizeof(double));
	double     *a1    = cpl_malloc(N * sizeof(double));
	double     *a2    = cpl_malloc(N * sizeof(double));
	double     *a3    = cpl_malloc(N * sizeof(double));
	double     *a4    = cpl_malloc(N * sizeof(double));
	double     *a6    = cpl_malloc(N * sizeof(double));
	double     *a7    = cpl_malloc(N * sizeof(double));
	double     *a8    = cpl_malloc(N * sizeof(double));
	double     *a9    = cpl_malloc(N * sizeof(double));

	int        *b_asc = cpl_malloc(N * sizeof(int));
	int        *b_des = cpl_malloc(N * sizeof(int));
	int        *b1    = cpl_malloc(N * sizeof(int));
	int        *b6    = cpl_malloc(N * sizeof(int));

	double     *c_asc = cpl_malloc(N * sizeof(double));
	double     *c_des = cpl_malloc(N * sizeof(double));
	double     *c2    = cpl_malloc(N * sizeof(double));
	double     *c7    = cpl_malloc(N * sizeof(double));

	cpl_size   *d_asc = cpl_malloc(N * sizeof(cpl_size));
	cpl_size   *d_des = cpl_malloc(N * sizeof(cpl_size));
	cpl_size   *d3    = cpl_malloc(N * sizeof(cpl_size));
	cpl_size   *d8    = cpl_malloc(N * sizeof(cpl_size));

	hdrl_value *e_asc = cpl_malloc(N * sizeof(hdrl_value));
	hdrl_value *e_des = cpl_malloc(N * sizeof(hdrl_value));
	hdrl_value *e4    = cpl_malloc(N * sizeof(hdrl_value));
	hdrl_value *e9    = cpl_malloc(N * sizeof(hdrl_value));


	/* Fill arrays with random values */
	hdrl_random_state *state = hdrl_random_state_new(1, NULL);
    for (cpl_size i = 0; i < N; i++) {

    	a_asc[i] = hdrl_random_normal(state, 5.5, 0.5);
    	a1[i]    = a_asc[i];
    	a2[i]    = a_asc[i];
    	a3[i]    = a_asc[i];
    	a4[i]    = a_asc[i];
    	a_des[i] = a_asc[i];
    	a6[i]    = a_des[i];
    	a7[i]    = a_des[i];
    	a8[i]    = a_des[i];
    	a9[i]    = a_des[i];

    	b_asc[i] = hdrl_random_poisson(state, 100.);
    	b1[i]    = b_asc[i];
    	b_des[i] = b_asc[i];
    	b6[i]    = b_des[i];

    	c_asc[i] = hdrl_random_uniform_double(state, 10., 20.);
    	c2[i]    = c_asc[i];
    	c_des[i] = c_asc[i];
    	c7[i]    = c_des[i];

    	d_asc[i] = hdrl_random_poisson(state, 300.);
    	d3[i]    = d_asc[i];
    	d_des[i] = d_asc[i];
    	d8[i]    = d_des[i];

    	e_asc[i] = (hdrl_value){hdrl_random_normal(state, 3.5, 1.5),
    		  	                 hdrl_random_normal(state, 2.5, 1.0) };
    	e4[i]    = (hdrl_value){e_asc[i].data, e_asc[i].error};
    	e_des[i] = (hdrl_value){e_asc[i].data, e_asc[i].error};
    	e9[i]    = (hdrl_value){e_des[i].data, e_des[i].error};
    }
	hdrl_random_state_delete(state);

	/* Prepare arrays for ascending order */
	void *         arrs_asc[ 4] = { b_asc, c_asc, d_asc, e_asc};
	hdrl_sort_type types_asc[4] = { HDRL_SORT_INT, HDRL_SORT_DOUBLE, HDRL_SORT_CPL_SIZE, HDRL_SORT_HDRL_VALUE};

	/* Prepare arrays for descending order */
	void *         arrs_des[ 4] = { b_des, c_des, d_des, e_des};
	hdrl_sort_type types_des[4] = { HDRL_SORT_INT, HDRL_SORT_DOUBLE, HDRL_SORT_CPL_SIZE, HDRL_SORT_HDRL_VALUE};


	/* Test 1: Random array INT - ASCENDING */
	sort_array_index( a1, N, b1, HDRL_SORT_INT, CPL_SORT_ASCENDING);
	cpl_test_error(CPL_ERROR_NONE);

	/* Test 2: Random array DOUBLE - ASCENDING */
	sort_array_index( a2, N, c2, HDRL_SORT_DOUBLE, CPL_SORT_ASCENDING);
	cpl_test_error(CPL_ERROR_NONE);

	/* Test 3: Random array CPL_SIZE - ASCENDING */
	sort_array_index( a3, N, d3, HDRL_SORT_CPL_SIZE, CPL_SORT_ASCENDING);
	cpl_test_error(CPL_ERROR_NONE);

	/* Test 4: Random array HDRL_VALUE - ASCENDING */
	sort_array_index( a4, N, e4, HDRL_SORT_HDRL_VALUE, CPL_SORT_ASCENDING);
	cpl_test_error(CPL_ERROR_NONE);

	/* Test 5: Ramdom arrays --> order all with the same function - ASCENDING */
	sort_arrays_index(a_asc, N, arrs_asc, 4, types_asc, CPL_SORT_DESCENDING);
	cpl_test_error(CPL_ERROR_NONE);


	/* Test 6: Random array INT - DESCENDING */
	sort_array_index( a6, N, b6, HDRL_SORT_INT, CPL_SORT_DESCENDING);
	cpl_test_error(CPL_ERROR_NONE);

	/* Test 7: Random array DOUBLE - DESCENDING */
	sort_array_index( a7, N, c7, HDRL_SORT_DOUBLE, CPL_SORT_DESCENDING);
	cpl_test_error(CPL_ERROR_NONE);

	/* Test 8: Random array CPL_SIZE - DESCENDING */
	sort_array_index( a8, N, d8, HDRL_SORT_CPL_SIZE, CPL_SORT_DESCENDING);
	cpl_test_error(CPL_ERROR_NONE);

	/* Test 9: Random array HDRL_VALUE - DESCENDING */
	sort_array_index( a9, N, e9, HDRL_SORT_HDRL_VALUE, CPL_SORT_DESCENDING);
	cpl_test_error(CPL_ERROR_NONE);

	/* Test 10: Ramdom arrays --> order all with the same function - DESCENDING */
	sort_arrays_index(a_des, N, arrs_des, 4, types_des, CPL_SORT_ASCENDING);
	cpl_test_error(CPL_ERROR_NONE);

	cpl_free(a_asc);
	cpl_free(a_des);
	cpl_free(a1);
	cpl_free(a2);
	cpl_free(a3);
	cpl_free(a4);
	cpl_free(a6);
	cpl_free(a7);
	cpl_free(a8);
	cpl_free(a9);

	cpl_free(b_asc);
	cpl_free(b_des);
	cpl_free(b1);
	cpl_free(b6);


	cpl_free(c_asc);
	cpl_free(c_des);
	cpl_free(c2);
	cpl_free(c7);

	cpl_free(d_asc);
	cpl_free(d_des);
	cpl_free(d3);
	cpl_free(d8);

	cpl_free(e_asc);
	cpl_free(e_des);
	cpl_free(e4);
	cpl_free(e9);
}

void test_sort_arrays_fixed(void)
{
	/*** Testing the results fixed by hand ***/

	double     x_asc[10]      = {-0.5,  0.33, 2.66, -3.5,  5.1, 5.2, 5.3,  5.4,  5.5, 4.4};
	double     x_des[10]      = {-0.5,  0.33, 2.66, -3.5,  5.1, 5.2, 5.3,  5.4,  5.5, 4.4};
	double     x1[10]         = {-0.5,  0.33, 2.66, -3.5,  5.1, 5.2, 5.3,  5.4,  5.5, 4.4};
	double     x2[10]         = {-0.5,  0.33, 2.66, -3.5,  5.1, 5.2, 5.3,  5.4,  5.5, 4.4};
	double     x3[10]         = {-0.5,  0.33, 2.66, -3.5,  5.1, 5.2, 5.3,  5.4,  5.5, 4.4};
	double     x4[10]         = {-0.5,  0.33, 2.66, -3.5,  5.1, 5.2, 5.3,  5.4,  5.5, 4.4};
	double     x6[10]         = {-0.5,  0.33, 2.66, -3.5,  5.1, 5.2, 5.3,  5.4,  5.5, 4.4};
	double     x7[10]         = {-0.5,  0.33, 2.66, -3.5,  5.1, 5.2, 5.3,  5.4,  5.5, 4.4};
	double     x8[10]         = {-0.5,  0.33, 2.66, -3.5,  5.1, 5.2, 5.3,  5.4,  5.5, 4.4};
	double     x9[10]         = {-0.5,  0.33, 2.66, -3.5,  5.1, 5.2, 5.3,  5.4,  5.5, 4.4};
    double     x_corr_asc[10] = {-3.5, -0.5,  0.33,  2.66, 4.4, 5.1, 5.2,  5.3,  5.4, 5.5};
	double     x_corr_des[10] = { 5.5,  5.4,  5.3,   5.2,  5.1, 4.4, 2.66, 0.33,-0.5,-3.5};

	int        i_asc[10]      = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	int        i_des[10]      = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	int        i_1[10]        = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	int        i_6[10]        = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	int        i_corr_asc[10] = { 3, 0, 1, 2, 9, 4, 5, 6, 7, 8};
	int        i_corr_des[10] = { 8, 7, 6, 5, 4, 9, 2, 1, 0, 3};

	double     db_asc[10]      = { 0.,-1.,-2.,-3.,-4., 5., 6., 7., 8., 9.};
	double     db_des[10]      = { 0.,-1.,-2.,-3.,-4., 5., 6., 7., 8., 9.};
	double     db_2[10]        = { 0.,-1.,-2.,-3.,-4., 5., 6., 7., 8., 9.};
	double     db_7[10]        = { 0.,-1.,-2.,-3.,-4., 5., 6., 7., 8., 9.};
	double     db_corr_asc[10] = {-3., 0.,-1.,-2., 9.,-4., 5., 6., 7., 8.};
	double     db_corr_des[10] = { 8., 7., 6., 5.,-4., 9.,-2.,-1., 0.,-3.};

	cpl_size   cp_asc[10]      = { 0,-1,-2,-3,-4,-5,-6,-7,-8,-9};
	cpl_size   cp_des[10]      = { 0,-1,-2,-3,-4,-5,-6,-7,-8,-9};
	cpl_size   cp_3[10]        = { 0,-1,-2,-3,-4,-5,-6,-7,-8,-9};
	cpl_size   cp_8[10]        = { 0,-1,-2,-3,-4,-5,-6,-7,-8,-9};
	cpl_size   cp_corr_asc[10] = {-3, 0,-1,-2,-9,-4,-5,-6,-7,-8};
	cpl_size   cp_corr_des[10] = {-8,-7,-6,-5,-4,-9,-2,-1, 0,-3};

	hdrl_value hv_asc[10]      = {(hdrl_value){-3.5, 0.1}, (hdrl_value){-0.5,   0.2}, (hdrl_value){ 0.001, 0.3}, (hdrl_value){ 1.1,   0.4}, (hdrl_value){1.2, 0.5},
			                      (hdrl_value){ 5.0, 0.6}, (hdrl_value){ 6.0,   0.7}, (hdrl_value){ 7.0,   0.8}, (hdrl_value){ 8.0,   0.9}, (hdrl_value){9.0, 1.0}  };
	hdrl_value hv_des[10]      = {(hdrl_value){-3.5, 0.1}, (hdrl_value){-0.5,   0.2}, (hdrl_value){ 0.001, 0.3}, (hdrl_value){ 1.1,   0.4}, (hdrl_value){1.2, 0.5},
			                      (hdrl_value){ 5.0, 0.6}, (hdrl_value){ 6.0,   0.7}, (hdrl_value){ 7.0,   0.8}, (hdrl_value){ 8.0,   0.9}, (hdrl_value){9.0, 1.0}  };
	hdrl_value hv_4[10]        = {(hdrl_value){-3.5, 0.1}, (hdrl_value){-0.5,   0.2}, (hdrl_value){ 0.001, 0.3}, (hdrl_value){ 1.1,   0.4}, (hdrl_value){1.2, 0.5},
                                  (hdrl_value){ 5.0, 0.6}, (hdrl_value){ 6.0,   0.7}, (hdrl_value){ 7.0,   0.8}, (hdrl_value){ 8.0,   0.9}, (hdrl_value){9.0, 1.0}  };
	hdrl_value hv_9[10]        = {(hdrl_value){-3.5, 0.1}, (hdrl_value){-0.5,   0.2}, (hdrl_value){ 0.001, 0.3}, (hdrl_value){ 1.1,   0.4}, (hdrl_value){1.2, 0.5},
                                  (hdrl_value){ 5.0, 0.6}, (hdrl_value){ 6.0,   0.7}, (hdrl_value){ 7.0,   0.8}, (hdrl_value){ 8.0,   0.9}, (hdrl_value){9.0, 1.0}  };
	hdrl_value hv_corr_asc[10] = {(hdrl_value){ 1.1, 0.4}, (hdrl_value){-3.5,   0.1}, (hdrl_value){-0.5,   0.2}, (hdrl_value){ 0.001, 0.3}, (hdrl_value){9.0, 1.0},
			                      (hdrl_value){ 1.2, 0.5}, (hdrl_value){ 5.0,   0.6}, (hdrl_value){ 6.0,   0.7}, (hdrl_value){ 7.0,   0.8}, (hdrl_value){8.0, 0.9}  };
	hdrl_value hv_corr_des[10] = {(hdrl_value){ 8.0, 0.9}, (hdrl_value){ 7.0,   0.8}, (hdrl_value){ 6.0,   0.7}, (hdrl_value){ 5.0,   0.6}, (hdrl_value){1.2, 0.5},
			                      (hdrl_value){ 9.0, 1.0}, (hdrl_value){ 0.001, 0.3}, (hdrl_value){-0.5,   0.2}, (hdrl_value){-3.5,   0.1}, (hdrl_value){1.1, 0.4}  };

	/* Prepare arrays for ascending order */
	void *         arrs_asc[ 4] = { i_asc, db_asc, cp_asc, hv_asc};
	hdrl_sort_type types_asc[4] = { HDRL_SORT_INT, HDRL_SORT_DOUBLE, HDRL_SORT_CPL_SIZE, HDRL_SORT_HDRL_VALUE};

	/* Prepare arrays for descending order */
	void *         arrs_des[ 4] = { i_des, db_des, cp_des, hv_des};
	hdrl_sort_type types_des[4] = { HDRL_SORT_INT, HDRL_SORT_DOUBLE, HDRL_SORT_CPL_SIZE, HDRL_SORT_HDRL_VALUE};


	/* Test 1: Fixed array INT - ASCCENDING */
	sort_array_index(x1, 10, i_1, HDRL_SORT_INT, CPL_SORT_ASCENDING);
	cpl_test_error(CPL_ERROR_NONE);
	for (cpl_size ind = 0; ind < 10; ind++) {
		cpl_test_eq(x1[ ind], x_corr_asc[ind]);
		cpl_test_eq(i_1[ind], i_corr_asc[ind]);
	}

	/* Test 2: Fixed array DOUBLE - ASCCENDING */
	sort_array_index(x2, 10, db_2, HDRL_SORT_DOUBLE, CPL_SORT_ASCENDING);
	cpl_test_error(CPL_ERROR_NONE);
	for (cpl_size ind = 0; ind < 10; ind++) {
		cpl_test_eq(x2[  ind], x_corr_asc[ ind]);
		cpl_test_eq(db_2[ind], db_corr_asc[ind]);
	}

	/* Test 3: Fixed array CPL_SIZE - ASCCENDING */
	sort_array_index(x3, 10, cp_3, HDRL_SORT_CPL_SIZE, CPL_SORT_ASCENDING);
	cpl_test_error(CPL_ERROR_NONE);
	for (cpl_size ind = 0; ind < 10; ind++) {
		cpl_test_eq(x3[  ind], x_corr_asc[ ind]);
		cpl_test_eq(cp_3[ind], cp_corr_asc[ind]);
	}

	/* Test 4: Fixed array HDRL_VALUE - ASCCENDING */
	sort_array_index(x4, 10, hv_4, HDRL_SORT_HDRL_VALUE, CPL_SORT_ASCENDING);
	cpl_test_error(CPL_ERROR_NONE);
	for (cpl_size ind = 0; ind < 10; ind++) {
		cpl_test_eq(x4[  ind],       x_corr_asc[ ind]);
		cpl_test_eq(hv_4[ind].data,  hv_corr_asc[ind].data);
		cpl_test_eq(hv_4[ind].error, hv_corr_asc[ind].error);
	}

	/* Test 5: Fixed arrays --> order all with the same function - ASCCENDING */
	sort_arrays_index(x_asc, 10, arrs_asc, 4, types_asc, CPL_SORT_ASCENDING);
	cpl_test_error(CPL_ERROR_NONE);
	for (cpl_size ind = 0; ind < 10; ind++) {
		cpl_test_eq(x_asc[ ind],       x_corr_asc[ ind]);
		cpl_test_eq(i_asc[ ind],       i_corr_asc[ ind]);
		cpl_test_eq(db_asc[ind],       db_corr_asc[ind]);
		cpl_test_eq(cp_asc[ind],       cp_corr_asc[ind]);
		cpl_test_eq(hv_asc[ind].data,  hv_corr_asc[ind].data);
		cpl_test_eq(hv_asc[ind].error, hv_corr_asc[ind].error);
	}


	/* Test 6: Fixed array INT - DESCENDING */
	sort_array_index(x6, 10, i_6, HDRL_SORT_INT, CPL_SORT_DESCENDING);
	cpl_test_error(CPL_ERROR_NONE);
	for (cpl_size ind = 0; ind < 10; ind++) {
		cpl_test_eq(x6[ ind], x_corr_des[ind]);
		cpl_test_eq(i_6[ind], i_corr_des[ind]);
	}

	/* Test 7: Fixed array DOUBLE - DESCENDING */
	sort_array_index(x7, 10, db_7, HDRL_SORT_DOUBLE, CPL_SORT_DESCENDING);
	cpl_test_error(CPL_ERROR_NONE);
	for (cpl_size ind = 0; ind < 10; ind++) {
		cpl_test_eq(x7[  ind], x_corr_des[ ind]);
		cpl_test_eq(db_7[ind], db_corr_des[ind]);
	}

	/* Test 8: Fixed array CPL_SIZE - DESCENDING */
	sort_array_index(x8, 10, cp_8, HDRL_SORT_CPL_SIZE, CPL_SORT_DESCENDING);
	cpl_test_error(CPL_ERROR_NONE);
	for (cpl_size ind = 0; ind < 10; ind++) {
		cpl_test_eq(x8[  ind], x_corr_des[ ind]);
		cpl_test_eq(cp_8[ind], cp_corr_des[ind]);
	}

	/* Test 9: Fixed array HDRL_VALUE - DESCENDING */
	sort_array_index(x9, 10, hv_9, HDRL_SORT_HDRL_VALUE, CPL_SORT_DESCENDING);
	cpl_test_error(CPL_ERROR_NONE);
	for (cpl_size ind = 0; ind < 10; ind++) {
		cpl_test_eq(x9[  ind], x_corr_des[ ind]);
		cpl_test_eq(hv_9[ind].data,  hv_corr_des[ind].data);
		cpl_test_eq(hv_9[ind].error, hv_corr_des[ind].error);
	}

	/* Test 10: Fixed arrays --> order all with the same function - DESCENDING */
	sort_arrays_index(x_des, 10, arrs_des, 4, types_des, CPL_SORT_DESCENDING);
	cpl_test_error(CPL_ERROR_NONE);
	for (cpl_size ind = 0; ind < 10; ind++) {
		cpl_test_eq(x_des[ ind],       x_corr_des[ ind]);
		cpl_test_eq(i_des[ ind],       i_corr_des[ ind]);
		cpl_test_eq(db_des[ind],       db_corr_des[ind]);
		cpl_test_eq(cp_des[ind],       cp_corr_des[ind]);
		cpl_test_eq(hv_des[ind].data,  hv_corr_des[ind].data);
		cpl_test_eq(hv_des[ind].error, hv_corr_des[ind].error);
	}
}

int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT,CPL_MSG_WARNING);

    /* Test 3 diferent functions: qsort in C, hdrl_function and cpl_function
     * in ascending and descending order */
    test_cmp_sort_func(CPL_SORT_ASCENDING);
    test_cmp_sort_func(CPL_SORT_DESCENDING);

    /* Test sort arrays by index, the first is ordering and the nexts by index */
    test_sort_arrays_random();

    /* Test sort arrays by index, equal to previously but checking the results */
    test_sort_arrays_fixed();

    return cpl_test_end(0);
}
