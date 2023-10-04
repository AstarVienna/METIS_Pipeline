/*
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                    Includes
 -----------------------------------------------------------------------------*/

#include "igam.c"
#include "hdrl_bpm_fit.h"

#include <cpl.h>
#include <math.h>

/*----------------------------------------------------------------------------*/
/**
  @defgroup hdrl_bpm_fit_test    Testing of the HDRL bpm_fit
 */
/*----------------------------------------------------------------------------*/

void test_invalid_parameter(void)
{
    hdrl_parameter * p;
    /* invalid degree */
    p = hdrl_bpm_fit_parameter_create_pval(-1, 0.1);
    cpl_test_null(p);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    p = hdrl_bpm_fit_parameter_create_rel_chi(-1, 0.1, 0.1);
    cpl_test_null(p);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    p = hdrl_bpm_fit_parameter_create_rel_coef(-1, 0.1, 0.1);
    cpl_test_null(p);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    /* invalid threshold */
    p = hdrl_bpm_fit_parameter_create_pval(1, -0.1);
    cpl_test_null(p);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    p = hdrl_bpm_fit_parameter_create_pval(1, 100.1);
    cpl_test_null(p);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    p = hdrl_bpm_fit_parameter_create_rel_chi(1, -0.1, 0.1);
    cpl_test_null(p);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);
    p = hdrl_bpm_fit_parameter_create_rel_coef(1, -0.1, -0.1);
    cpl_test_null(p);
    cpl_test_error(CPL_ERROR_ILLEGAL_INPUT);

    /* NULL input */
    hdrl_bpm_fit_parameter_get_pval(NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    hdrl_bpm_fit_parameter_get_rel_chi_low(NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    hdrl_bpm_fit_parameter_get_rel_coef_high(NULL);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
}

void test_parameter(void)
{
    hdrl_parameter * p;

    p = hdrl_bpm_fit_parameter_create_pval(1, 10.);
    cpl_test(hdrl_bpm_fit_parameter_check(p));
    cpl_test_abs(hdrl_bpm_fit_parameter_get_pval(p), 10., 0);
    cpl_test_lt((hdrl_bpm_fit_parameter_get_rel_chi_low(p)), 0);
    cpl_test_lt((hdrl_bpm_fit_parameter_get_rel_coef_high(p)), 0);
    hdrl_parameter_delete(p);

    p = hdrl_bpm_fit_parameter_create_rel_chi(1, 10., 5.);
    cpl_test(hdrl_bpm_fit_parameter_check(p));
    cpl_test_eq(hdrl_bpm_fit_parameter_get_rel_chi_low(p), 10.);
    cpl_test_eq(hdrl_bpm_fit_parameter_get_rel_chi_high(p), 5.);
    cpl_test_lt(hdrl_bpm_fit_parameter_get_pval(p), 0);
    cpl_test_lt(hdrl_bpm_fit_parameter_get_rel_coef_low(p), 0);
    cpl_test_lt(hdrl_bpm_fit_parameter_get_rel_coef_high(p), 0);
    hdrl_parameter_delete(p);

    p = hdrl_bpm_fit_parameter_create_rel_coef(1, 10., 3.);
    cpl_test(hdrl_bpm_fit_parameter_check(p));
    cpl_test_eq(hdrl_bpm_fit_parameter_get_rel_coef_low(p), 10.);
    cpl_test_eq(hdrl_bpm_fit_parameter_get_rel_coef_high(p), 3.);
    cpl_test_lt(hdrl_bpm_fit_parameter_get_pval(p), 0);
    cpl_test_lt(hdrl_bpm_fit_parameter_get_rel_chi_low(p), 0);
    cpl_test_lt(hdrl_bpm_fit_parameter_get_rel_chi_high(p), 0);

    hdrl_parameter_delete(p);
}

void test_parameterlist(void)
{
    hdrl_parameter * p;
    hdrl_parameter * def;
    cpl_parameterlist * parlist;

    def = hdrl_bpm_fit_parameter_create_pval(-2, 0.1) ;
    parlist = hdrl_bpm_fit_parameter_create_parlist("RECIPE", "bpm_fit", def);
    hdrl_parameter_delete(def); 
    p = hdrl_bpm_fit_parameter_parse_parlist(parlist, "RECIPE.bpm_fit");
    cpl_test_null(p);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_parameterlist_delete(parlist);

    def = hdrl_bpm_fit_parameter_create_pval(2, -1) ;
    parlist = hdrl_bpm_fit_parameter_create_parlist("RECIPE", "bpm_fit", def);
    hdrl_parameter_delete(def); 
    p = hdrl_bpm_fit_parameter_parse_parlist(parlist, "RECIPE.bpm_fit");
    cpl_test_null(p);
    cpl_test_error(CPL_ERROR_NULL_INPUT);
    cpl_parameterlist_delete(parlist);

    def = hdrl_bpm_fit_parameter_create_pval(2, 0.1) ;
    parlist = hdrl_bpm_fit_parameter_create_parlist("RECIPE", "bpm_fit", def);
    hdrl_parameter_delete(def); 
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(cpl_parameterlist_get_size(parlist), 6);

    p = hdrl_bpm_fit_parameter_parse_parlist(parlist, "RECIPE.invalid");
    cpl_test_null(p);
    cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);

    p = hdrl_bpm_fit_parameter_parse_parlist(parlist, "RECIPE.bpm_fit");
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(cpl_parameterlist_get_size(parlist), 6);
    cpl_test_eq(hdrl_bpm_fit_parameter_get_degree(p), 2);
    cpl_test_abs(hdrl_bpm_fit_parameter_get_pval(p), 0.1, 0);
    hdrl_parameter_delete(p);
    cpl_parameterlist_delete(parlist);

    def = hdrl_bpm_fit_parameter_create_rel_chi(2, 3., 2.) ;
    parlist = hdrl_bpm_fit_parameter_create_parlist( "RECIPE", "bpm_fit", def);
    hdrl_parameter_delete(def); 
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(cpl_parameterlist_get_size(parlist), 6);
    p = hdrl_bpm_fit_parameter_parse_parlist(parlist, "RECIPE.bpm_fit");
    cpl_test_eq(hdrl_bpm_fit_parameter_get_degree(p), 2);
    cpl_test_eq(hdrl_bpm_fit_parameter_get_rel_chi_low(p), 3.);
    cpl_test_eq(hdrl_bpm_fit_parameter_get_rel_chi_high(p), 2.);
    hdrl_parameter_delete(p);
    cpl_parameterlist_delete(parlist);

    def = hdrl_bpm_fit_parameter_create_rel_coef(2, 2., 3.) ;
    parlist = hdrl_bpm_fit_parameter_create_parlist("RECIPE", "bpm_fit", def);
    hdrl_parameter_delete(def); 
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(cpl_parameterlist_get_size(parlist), 6);
    p = hdrl_bpm_fit_parameter_parse_parlist(parlist, "RECIPE.bpm_fit");
    cpl_test_eq(hdrl_bpm_fit_parameter_get_degree(p), 2);
    cpl_test_eq(hdrl_bpm_fit_parameter_get_rel_coef_low(p), 2.);
    cpl_test_eq(hdrl_bpm_fit_parameter_get_rel_coef_high(p), 3.);
    hdrl_parameter_delete(p);
    cpl_parameterlist_delete(parlist);
}

void test_bpm_fit(void)
{
    int rej;
    hdrl_imagelist * hl = hdrl_imagelist_new();
    cpl_vector * sample = cpl_vector_new(10);
    for (size_t i = 0; i < 10; i++) {
        hdrl_image * himg =  hdrl_image_new(13, 4);
        hdrl_image_add_scalar(himg, (hdrl_value){i + 1, sqrt(i + 1)});
        hdrl_image_set_pixel(himg, 1, 1, (hdrl_value){1.01, 1.});
        hdrl_imagelist_set(hl, himg, i);
        cpl_vector_set(sample, i, i);
    }

    hdrl_parameter * p;
    cpl_image * out_mask;

    p = hdrl_bpm_fit_parameter_create_pval(1, 0.1);
    hdrl_bpm_fit_compute(p, hl, sample, &out_mask);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(cpl_image_get_flux(out_mask), 0.);
    cpl_image_delete(out_mask);
    hdrl_parameter_delete(p);

    p = hdrl_bpm_fit_parameter_create_rel_coef(1, 1., 1.);
    hdrl_bpm_fit_compute(p, hl, sample, &out_mask);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(cpl_image_get(out_mask, 1, 1, &rej), 3);
    cpl_image_delete(out_mask);
    hdrl_parameter_delete(p);

    p = hdrl_bpm_fit_parameter_create_rel_chi(1, 1., 1.);
    hdrl_image_add_scalar(hdrl_imagelist_get(hl, 4),
                          (hdrl_value){5.1 , sqrt(5.1)});
    hdrl_bpm_fit_compute(p, hl, sample, &out_mask);
    cpl_test_error(CPL_ERROR_NONE);
    cpl_test_eq(cpl_image_get_flux(out_mask), 1.);
    cpl_test_eq(cpl_image_get(out_mask, 1, 1, &rej), 1.);
    cpl_image_delete(out_mask);
    hdrl_parameter_delete(p);

    hdrl_imagelist_delete(hl);
    cpl_vector_delete(sample);
}

void test_igam(void){

	cpl_test_error(CPL_ERROR_NONE);

	/* Fail test */
	cpl_test(isnan(igamc(-1, 1)));
	cpl_test(isnan(igamc(1, -1)));
	cpl_test(isnan(igamc(-1, -1)));

	cpl_test(!isnan(igamc(1, 1)));


	/* Normal test */
	double a = 1.;
	double x = 2.;

	double res1 = igamc(a, x);
	cpl_test_eq(res1, 0.);

	double res2 = igamc(x, a);
	cpl_test_eq(res2, 0.);

	double res3 = igam(a, x);
	cpl_test_eq(res3, 0.);

	double res4 = igam(x, a);
	cpl_test_eq(res4, 0.);


	double res11 = igamc(0., 0.);
	cpl_test(res11 != 0.);

	double res12 = igamc(0., 1.);
	cpl_test(res12 != 0.);

	double res13 = igamc(1., 0.);
	cpl_test(res13 != 0.);


	double res21 = igamc(0., 1e10);
	cpl_test(res21 != 0.);

	double res22 = igamc(1., 1e20);
	cpl_test_eq(res22, 0.);

	double res23 = igamc(1e10, 0.);
	cpl_test_eq(res23, 1.);

	double res24 = igamc(1e20, 1.);
	cpl_test_eq(res24, 1.);


	cpl_test_error(CPL_ERROR_NONE);
}


/*----------------------------------------------------------------------------*/
/**
  @brief   Unit tests of BPM module
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    test_invalid_parameter();
    test_parameter();
    test_parameterlist();
    test_bpm_fit();

    test_igam();


    return cpl_test_end(0);
}
