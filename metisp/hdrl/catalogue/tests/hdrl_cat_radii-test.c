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
#include "../hdrl_cat_moments.h"
#include "../hdrl_cat_radii.h"

#define NT       117
#define SIZE_IMG 64


static double radii[]  = {2.5,3.53553,5.0,7.07107,10,14,20,25,30,35,40,50,60};

static double fluxes[] = {13670.3, 19834.4, 23923.2, 25124.0, 25332.3, 25488.9,
                          25648.7, 25842.8, 25950.9, 25893.9, 25982,1, 25297.6, 24919.1};


void flux_test(cpl_boolean execute_test)
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
    ap.lsiz     = SIZE_IMG;
    ap.csiz     = SIZE_IMG;
    ap.thresh   = 11.0936;
    ap.inframe  = cpl_image_new(SIZE_IMG, SIZE_IMG, CPL_TYPE_DOUBLE);
    ap.conframe = cpl_image_new(SIZE_IMG, SIZE_IMG, CPL_TYPE_DOUBLE);

    /* Initialize */
    hdrl_apinit(&ap);

    ap.npl_pix = NT;
    ap.plarray = cpl_realloc(ap.plarray, NT * sizeof(plstruct));
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
    ap.mflag    = cpl_calloc(SIZE_IMG * SIZE_IMG, sizeof(*ap.mflag));

    /* Create a background */
    cpl_image_fill_noise_uniform(ap.inframe, -10., 10.);
    cpl_image_add_scalar(        ap.inframe,  5000.);
    cpl_image_fill_noise_uniform(ap.conframe, 99, 101);

    /* Do a basic moments analysis and work out the areal profiles*/
    double momresults[8];
    hdrl_moments(&ap, momresults);

	/* Try and deblend the images if it is requested and justified */
	double parmall[IMNUM][NPAR];
	cpl_size nbit = 10;
	for (cpl_size i = 0; i < IMNUM; i++) {
		for (cpl_size j = 0; j < NPAR; j++) {
			parmall[i][j] = 0.;
		}
	}

	/* Get Kron radius for all images and get the flux */
	double areal;
	double kron_rad[IMNUM];
	for (cpl_size k = 0; k < nbit; k++) {
		areal = parmall[k][8];
		kron_rad[k] = hdrl_kronrad(areal, radii, fluxes, NRADS);
	}
	if (execute_test) {
		double kron_flux[IMNUM];
		hdrl_flux(&ap, parmall, nbit, kron_rad, kron_flux, NRADS, radii, fluxes);
	}

	/* Get Petrosian radius for all images and get the flux */
	double petr_rad[IMNUM];
	for (cpl_size k = 0; k < nbit; k++) {
		areal = parmall[k][8];
		petr_rad[k] = hdrl_petrad(areal, radii, fluxes, NRADS);
	}
	if (execute_test) {
		double petr_flux[IMNUM];
		hdrl_flux(&ap, parmall, nbit, petr_rad, petr_flux, NRADS, radii, fluxes);
	}

    /* Clean up */
    hdrl_apclose(&ap);
    cpl_free(ap.mflag);
    cpl_image_delete(ap.inframe);
    cpl_image_delete(ap.conframe);
}

int main(void) {

    /* Initialize */
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    double halfrad  = 2.35;
    double exprad   = 6.18;
    double kronrad  = 6.18;
    double petrrad  = 12.45;
    double peak     = 1007.07;
    double areal    = 120.;

    double halflight;
    double thresh;
    double rad;


    /* Test halflight */
    halflight = fluxes[4] / 2.;
    rad = hdrl_halflight(radii, fluxes, halflight, peak, NRADS);
    cpl_test_rel(rad, halfrad, 0.01);

    /* Test exprad */
    thresh = 4.;
    rad = hdrl_exprad(thresh, peak, areal, radii, NRADS);
    cpl_test_rel(rad, exprad, 0.01);

    /* Test Kron */
    rad = hdrl_kronrad(areal, radii, fluxes, NRADS);
    cpl_test_rel(rad, kronrad, 0.01);

    /* Test Petrosian */
    rad = hdrl_petrad(areal, radii, fluxes, NRADS);
    cpl_test_rel(rad, petrrad, 0.01);

    /* Flux_test. TODO: Fail in Debian 6.0 [debug - i686] and Debian 7.0 [debug x86_64]
     * Maybe for optiomization in the debug compiler mode for this machine.
     * Activate the test when this will be solve */
    flux_test(CPL_FALSE);

    return cpl_test_end(0);
}
