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

#include "../hdrl_cat_apio.h"
#include "../hdrl_cat_areals.h"
#include "../hdrl_cat_conf.h"
#include "../hdrl_cat_moments.h"
#include "../hdrl_cat_overlp.h"


#define NTEST        10
#define NT           117
#define SIZE_IMAGE   512
#define CENTER_IMAGE 256


void standard_test(void)
{
	/* Intitialize */
    double xpos[] = { 100., 200., 300., 400., 500., 600., 700., 800., 900., 1000.};
    double ypos[] = { 100., 200., 300., 400., 500., 600., 700., 800., 900., 1000.};
    double norm[] = {1000., 100., 200., 500., 550., 600., 650., 700., 750.,  800.};

    /* Create input data */
    cpl_image *im  = cpl_image_new(1024, 1024, CPL_TYPE_DOUBLE);
    cpl_image *bkg = cpl_image_new(1024, 1024, CPL_TYPE_DOUBLE);
    cpl_image *cnf = cpl_image_new(1024, 1024, CPL_TYPE_DOUBLE);

    cpl_image_fill_noise_uniform(bkg, -10.,   10. );
    cpl_image_fill_noise_uniform(cnf,  99.9, 100.1);

    hdrl_casu_fits *infErr = hdrl_casu_fits_wrap( im);
    hdrl_casu_fits *inconf = hdrl_casu_fits_wrap(cnf);
    hdrl_casu_fits *inf    = hdrl_casu_fits_wrap(bkg);

    double sigma = 2.;
    double norm2 = 2. * CPL_MATH_PI * sigma * sigma;

    double sky = 500.;
    cpl_image_add_scalar(bkg, sky);

    for (cpl_size i = 0; i < NTEST; i++) {
        cpl_image_fill_gaussian(im, xpos[i], ypos[i], norm[i] * norm2, sigma, sigma);
        cpl_image_add(bkg, im);
    }

    /*** Tests ***/
    hdrl_casu_result *res = cpl_calloc(1, sizeof(hdrl_casu_result));
    cpl_size ipix;
    cpl_size icrowd;

    /* Test 1: Error input null */
    ipix   = 5;
    icrowd = 0;
    cpl_test_eq(hdrl_catalogue_conf(infErr, NULL, ipix, 2.5, icrowd, 5., 1, 64, 6, 3., 1., INFINITY, res), CPL_ERROR_NONE);
    cpl_test_nonnull( res->catalogue);
    hdrl_casu_tfits_delete(res->catalogue);
    cpl_image_delete( res->segmentation_map);
    cpl_image_delete( res->background);

    /* Test 2 */
    ipix   = 5;
    icrowd = 0;
    cpl_test_eq(hdrl_catalogue_conf(infErr, inconf, ipix, 2.5, icrowd, 5., 1, 64, 6, 3., 1., INFINITY, res), CPL_ERROR_NONE);
    cpl_test_nonnull( res->catalogue);
    hdrl_casu_tfits_delete(res->catalogue);
    cpl_image_delete( res->segmentation_map);
    cpl_image_delete( res->background);

    /* Test 3 */
    ipix   = 10;
    icrowd = 10;
    cpl_test_eq(hdrl_catalogue_conf(infErr, NULL, ipix, 2.5, icrowd, 5., 1, 64, 6, 3., 1., INFINITY, res), CPL_ERROR_NONE);
    cpl_test_nonnull( res->catalogue);
    hdrl_casu_tfits_delete(res->catalogue);
    cpl_image_delete( res->segmentation_map);
    cpl_image_delete( res->background);

    /* Test 4 */
    ipix   = 10;
    icrowd = 10;
    cpl_test_eq(hdrl_catalogue_conf(infErr, inconf, ipix, 2.5, icrowd, 5., 1, 64, 6, 3., 1., INFINITY, res), CPL_ERROR_NONE);
    cpl_test_nonnull( res->catalogue);
    hdrl_casu_tfits_delete(res->catalogue);
    cpl_image_delete( res->segmentation_map);
    cpl_image_delete( res->background);

    /* Test 5 */
    ipix   = 0;
    icrowd = 5;
    cpl_test_eq(hdrl_catalogue_conf(inf,    NULL,   ipix, 1.5, icrowd, 5., 1, 64, 6, 3., 1., INFINITY, res), CPL_ERROR_NONE);
    cpl_test_nonnull( res->catalogue);
    hdrl_casu_tfits_delete(res->catalogue);
    cpl_image_delete( res->segmentation_map);
    cpl_image_delete( res->background);

    /* Test 6 */
    ipix   = 0;
    icrowd = 5;
    cpl_test_eq(hdrl_catalogue_conf(inf,   inconf,  ipix, 1.5, icrowd, 5., 1, 64, 6, 3., 1., INFINITY, res), CPL_ERROR_NONE);
    cpl_test_nonnull( res->catalogue);
    hdrl_casu_tfits_delete(res->catalogue);
    cpl_image_delete( res->segmentation_map);
    cpl_image_delete( res->background);

    /* Test 7 */
    ipix   = 10;
    icrowd = 10;
    cpl_test_eq(hdrl_catalogue_conf(inf,   NULL,   ipix, 1.5, icrowd, 5., 1, 64, 6, 3., 1., INFINITY, res), CPL_ERROR_NONE);
    cpl_test_nonnull( res->catalogue);
    hdrl_casu_tfits_delete(res->catalogue);
    cpl_image_delete( res->segmentation_map);
    cpl_image_delete( res->background);

    /* Test 8 */
    ipix   = 10;
    icrowd = 10;
    cpl_test_eq(hdrl_catalogue_conf(inf,   inconf, ipix, 1.5, icrowd, 5., 1, 64, 6, 3., 1., INFINITY, res), CPL_ERROR_NONE);
    cpl_test_nonnull( res->catalogue);
    hdrl_casu_tfits_delete(res->catalogue);
    cpl_image_delete( res->segmentation_map);
    cpl_image_delete( res->background);

    /* Clean up */
    hdrl_casu_fits_delete(infErr);
    hdrl_casu_fits_delete(inconf);
    hdrl_casu_fits_delete(inf);
    cpl_free(res);
}

void advanced_test(void)
{
	cpl_size x[] = {398,399,400,397,398,399,400,401,402,403,396,397,398,399,400,
                    401,402,403,404,395,396,397,398,399,400,401,402,403,404,405,
                    395,396,397,398,399,400,401,402,403,404,405,395,396,397,398,
                    399,400,401,402,403,404,405,395,396,397,398,399,400,401,402,
                    403,404,405,406,395,396,397,398,399,400,401,402,403,404,405,
                    395,396,397,398,399,400,401,402,403,404,405,395,396,397,398,
                    399,400,401,402,403,404,405,396,397,398,399,400,401,402,403,
                    404,397,398,399,400,401,402,403,398,399,400,401};

	cpl_size y[] = {394,394,394,395,395,395,395,395,395,395,396,396,396,396,396,
                    396,396,396,396,397,397,397,397,397,397,397,397,397,397,397,
                    398,398,398,398,398,398,398,398,398,398,398,399,399,399,399,
                    399,399,399,399,399,399,399,400,400,400,400,400,400,400,400,
                    400,400,400,400,401,401,401,401,401,401,401,401,401,401,401,
                    402,402,402,402,402,402,402,402,402,402,402,403,403,403,403,
                    403,403,403,403,403,403,403,404,404,404,404,404,404,404,404,
                    404,405,405,405,405,405,405,405,406,406,406,406};

    double  z[] =  {8.87152,12.515,7.69699,10.8527,22.2509,21.7368,13.0388,
                    12.1853,17.1976,7.43948,15.2245,29.1964,37.9117,57.9371,
                    71.5542,57.1288,34.7726,15.5934,11.5374,15.995,21.3606,
                    60.4006,103.46,147.55,168.274,147.476,98.9157,51.7186,20.188,
                    3.04248,5.77832,49.3103,98.2057,187.557,268.353,310.638,
                    274.295,183.969,94.6933,47.9889,20.245,26.3758,59.1781,
                    152.389,275.916,395.107,450.251,397.53,272.322,147.053,54.767,
                    11.8971,13.3888,73.3689,165.899,298.455,449.707,493.25,441.585,
                    299.31,157.474,70.1224,15.5313,8.76074,20.7188,54.5798,141.249,
                    264.87,382.736,435.452,393.871,268.175,138.485,65.9307,28.7812,
                    19.379,36.6449,93.5458,186.823,270.95,305.093,260.879,183.683,
                    100.676,32.6281,16.6497,5.94965,17.8105,57.256,106.32,145.264,
                    164.271,137.093,88.9384,60.7841,31.8582,10.0435,4.69162,
                    15.2187,32.5385,61.0381,74.5399,67.3727,43.3964,25.0956,
                    16.7595,-0.37323,21.3832,19.2497,18.5883,9.37448,19.6048,
                    11.5006,13.0159,14.5852,13.66,-1.04889};


    /* Set up apm structure */
    ap_t ap;
    ap.lsiz     = SIZE_IMAGE;
    ap.csiz     = SIZE_IMAGE;
    ap.thresh   = 11.0936;
    ap.inframe  = cpl_image_new(SIZE_IMAGE, SIZE_IMAGE, CPL_TYPE_DOUBLE);
    ap.conframe = cpl_image_new(SIZE_IMAGE, SIZE_IMAGE, CPL_TYPE_DOUBLE);


    /* Initialize */
    hdrl_apinit(&ap);

    ap.npl_pix = NT;
    ap.plarray = cpl_realloc(ap.plarray, NT * sizeof(*ap.plarray));
    for (cpl_size i = 0; i < NT; i++) {
        ap.plarray[i].x   = x[i];
        ap.plarray[i].y   = y[i];
        ap.plarray[i].z   = z[i];
        ap.plarray[i].zsm = z[i];
    }

    ap.xintmin      = 0.;
    ap.areal_offset = 3.47165;
    ap.thresh       = 11.0936;
    ap.fconst       = 1.4427;

    /* Run the test */
    cpl_size iareal[NAREAL];
    hdrl_areals(&ap, iareal);

    ap.indata   = cpl_image_get_data_double(ap.inframe);
    ap.confdata = cpl_image_get_data_double(ap.conframe);
    ap.mflag    = cpl_calloc(2048 * 2048, sizeof(*ap.mflag));

    /* Create a source */
	double tmax = 1000.;
	cpl_image_fill_gaussian(ap.inframe,  CENTER_IMAGE, CENTER_IMAGE, tmax,  10.,  10.);
	cpl_image_fill_gaussian(ap.conframe, CENTER_IMAGE, CENTER_IMAGE, tmax, 100., 100.);

    /* Do a basic moments analysis and work out the areal profiles*/
    double momresults[8];
    hdrl_moments(&ap, momresults);

    double parmall[IMNUM][NPAR];
    for (cpl_size i = 0; i < IMNUM; i++) {
    	for (cpl_size j = 0; j < NPAR; j++) {
    		parmall[i][j] = 0.;
    	}
    }


    /*** TESTS ***/
    cpl_size       nbit;
    cpl_error_code e;

    /* NOTE: Only if (iareal[0] >= ap->mulpix && ap->icrowd) */

    /* Test 1 */
    nbit     = 1;
    ap.ipnop = 1;
    e = hdrl_overlp( &ap, parmall, &nbit,
        		       momresults[1], momresults[2], momresults[3],
				       iareal[0], momresults[7]);
    cpl_test_eq(e, CPL_ERROR_NONE);


    /* Test 2 */
    nbit     = 1;
    ap.ipnop = 1024;
    e = hdrl_overlp( &ap, parmall, &nbit,
        		           CENTER_IMAGE, CENTER_IMAGE, momresults[3],
    					   SIZE_IMAGE * SIZE_IMAGE, tmax);
    cpl_test_eq(e, CPL_ERROR_NONE);


    nbit = 2;
    ap.ipnop            = 2;
    ap.areal_offset     = 1.5;
    ap.thresh           = 15.;
    ap.fconst           = 0.5;
    hdrl_areals(&ap, iareal);
    for (cpl_size i = 0; i < IMNUM; i++) {
    	for (cpl_size j = 0; j < NPAR; j++) {
    		parmall[i][j] = tmax;
    	}
    }

    /* Test 3 */
    e = hdrl_overlp( &ap, parmall, &nbit,
    		           CENTER_IMAGE, CENTER_IMAGE, momresults[3],
					   SIZE_IMAGE * SIZE_IMAGE, tmax);
    cpl_test_eq(e, CPL_ERROR_NONE);

    /* Test 4 */
    e = hdrl_overlp_2orMore( &ap, parmall, &nbit,
    		                   CENTER_IMAGE, CENTER_IMAGE, momresults[3],
    						   SIZE_IMAGE * SIZE_IMAGE, 1, 1, 1);
    cpl_test_eq(e, CPL_ERROR_NONE);


    /* Clean up */
    hdrl_apclose(&ap);
    cpl_free(ap.mflag);
    cpl_image_delete(ap.inframe);
    cpl_image_delete(ap.conframe);
}


int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    standard_test();
    advanced_test();

    return cpl_test_end(0);
}
