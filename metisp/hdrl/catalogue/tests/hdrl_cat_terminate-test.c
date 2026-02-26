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
#include "../hdrl_cat_terminate.h"


#define NT  117


int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

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
    ap.lsiz     = 2048;
    ap.csiz     = 2048;
    ap.thresh   = 11.0936;
    ap.inframe  = cpl_image_new(2048, 2048, CPL_TYPE_DOUBLE);
    ap.conframe = cpl_image_new(2048, 2048, CPL_TYPE_DOUBLE);

    /* Initialize */
    hdrl_apinit(&ap);
    ap.maxip        = 100;
    ap.ipnop        = 2;
    ap.areal_offset = 1.5;
    ap.multiply     = 1;

    ap.npl_pix = NT;
    ap.plarray = cpl_realloc(ap.plarray, NT * sizeof(*ap.plarray));
    for (cpl_size i = 0; i < NT; i++) {
        ap.plarray[i].x   = x[i];
        ap.plarray[i].y   = y[i];
        ap.plarray[i].z   = z[i];
        ap.plarray[i].zsm = z[i];
    }

    ap.xintmin      = 0.;
    ap.areal_offset = 3.56;
    ap.thresh       = 15.;
    ap.fconst       = 1.4;

    /* Run the test */
    cpl_size iareal[NAREAL];
    hdrl_areals(&ap, iareal);

    ap.indata   = cpl_image_get_data_double(ap.inframe);
    ap.confdata = cpl_image_get_data_double(ap.conframe);
    ap.mflag    = cpl_calloc(2048 * 2048, sizeof(*ap.mflag));

    /* Create a source */
	double tmax = 1000.;
	cpl_image_fill_gaussian(ap.inframe,  2048, 2048, tmax,  10.,  10.);
	cpl_image_fill_gaussian(ap.conframe, 2048, 2048, tmax, 100., 100.);

    /* Do a basic moments analysis and work out the areal profiles*/
    double momresults[8];
    hdrl_moments(&ap, momresults);

    /* Test 1 */
    ap.parent[1].pnop    = 0;
    ap.parent[1].first   = 0;
    ap.parent[1].last    = 0;
    ap.parent[1].growing = 0;
    ap.parent[1].touch   = 0;
    ap.parent[1].pnbp    = 0;
    hdrl_apfu(&ap);

    /* Clean up */
    hdrl_apclose(&ap);
    cpl_free(ap.mflag);
    cpl_image_delete(ap.inframe);
    cpl_image_delete(ap.conframe);

    return cpl_test_end(0);
}
