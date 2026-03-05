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

#include "hdrl_cat_table.h"

#include "hdrl_cat_areals.h"
#include "hdrl_cat_radii.h"
#include "hdrl_cat_phopt.h"
#include "hdrl_cat_seeing.h"
#include "hdrl_cat_overlp.h"
#include "hdrl_cat_moments.h"
#include "hdrl_cat_extend.h"
#include "hdrl_cat_background.h"


#define INITROWS        2048   /* Allocation size for rows in the output table */

/* Number assigned of columns in the table */
#define COL_NUMBER       1
#define COL_FLUXISO      2
#define COL_X            3
#define COL_XERR         4
#define COL_Y            5
#define COL_YERR         6
#define COL_SIGMA        7
#define COL_ELLIPT       8
#define COL_PA           9
#define COL_AREAL1      10
#define COL_AREAL2      11
#define COL_AREAL3      12
#define COL_AREAL4      13
#define COL_AREAL5      14
#define COL_AREAL6      15
#define COL_AREAL7      16
#define COL_AREAL8      17
#define COL_PEAKHEIGHT  18
#define COL_PKHTERR     19
#define COL_APFLUX1     20
#define COL_APFLUX1ERR  21
#define COL_APFLUX2     22
#define COL_APFLUX2ERR  23
#define COL_APFLUX3     24
#define COL_APFLUX3ERR  25
#define COL_APFLUX4     26
#define COL_APFLUX4ERR  27
#define COL_APFLUX5     28
#define COL_APFLUX5ERR  29
#define COL_APFLUX6     30
#define COL_APFLUX6ERR  31
#define COL_APFLUX7     32
#define COL_APFLUX7ERR  33
#define COL_APFLUX8     34
#define COL_APFLUX8ERR  35
#define COL_APFLUX9     36
#define COL_APFLUX9ERR  37
#define COL_APFLUX10    38
#define COL_APFLUX10ERR 39
#define COL_APFLUX11    40
#define COL_APFLUX11ERR 41
#define COL_APFLUX12    42
#define COL_APFLUX12ERR 43
#define COL_APFLUX13    44
#define COL_APFLUX13ERR 45
#define COL_PETRAD      46
#define COL_KRONRAD     47
#define COL_HALFRAD     48
#define COL_PETFLUX     49
#define COL_PETFLUXERR  50
#define COL_KRONFLUX    51
#define COL_KRONFLUXERR 52
#define COL_HALFFLUX    53
#define COL_HALFFLUXERR 54
#define COL_ERRFLAG     55
#define COL_SKYLEVEL    56
#define COL_SKYSIGMA    57
#define COL_AVCONF      58
#define COL_RA          59
#define COL_DEC         60
#define COL_CLASS       61
#define COL_STAT        62
#define COL_FWHM        63

/* Name assigned of columns in the table */
static const char *ttype[NCOLS]={"Sequence_number","Isophotal_flux",
                                 "X_coordinate","X_coordinate_err",
                                 "Y_coordinate","Y_coordinate_err",
                                 "Gaussian_sigma","Ellipticity","Position_angle",
                                 "Areal_1_profile","Areal_2_profile","Areal_3_profile",
                                 "Areal_4_profile","Areal_5_profile","Areal_6_profile",
                                 "Areal_7_profile","Areal_8_profile",
                                 "Peak_height","Peak_height_err",
                                 "Aper_flux_1","Aper_flux_1_err",
                                 "Aper_flux_2","Aper_flux_2_err",
                                 "Aper_flux_3","Aper_flux_3_err",
                                 "Aper_flux_4","Aper_flux_4_err",
                                 "Aper_flux_5","Aper_flux_5_err",
                                 "Aper_flux_6","Aper_flux_6_err",
                                 "Aper_flux_7","Aper_flux_7_err",
                                 "Aper_flux_8","Aper_flux_8_err",
                                 "Aper_flux_9","Aper_flux_9_err",
                                 "Aper_flux_10","Aper_flux_10_err",
                                 "Aper_flux_11","Aper_flux_11_err",
                                 "Aper_flux_12","Aper_flux_12_err",
                                 "Aper_flux_13","Aper_flux_13_err",
                                 "Petr_radius","Kron_radius","Half_radius",
                                 "Petr_flux","Petr_flux_err",
                                 "Kron_flux","Kron_flux_err","Half_flux","Half_flux_err",
                                 "Error_bit_flag","Sky_level","Sky_rms",
                                 "Av_conf",
                                 "RA","DEC","Classification","Statistic",
                                 "FWHM"};

static const char *tunit[NCOLS]={"","adu",
                                 "pixel","pixel",
                                 "pixel","pixel",
                                 "pixel","","deg",
                                 "pixel","pixel","pixel",
                                 "pixel","pixel","pixel",
                                 "pixel","pixel",
                                 "adu","adu",
                                 "adu","adu",
                                 "adu","adu",
                                 "adu","adu",
                                 "adu","adu",
                                 "adu","adu",
                                 "adu","adu",
                                 "adu","adu",
                                 "adu","adu",
                                 "adu","adu",
                                 "adu","adu",
                                 "adu","adu",
                                 "adu","adu",
                                 "adu","adu",
                                 "pixel","pixel","pixel",
                                 "adu","adu",
                                 "adu","adu","adu","adu",
                                 "","adu","adu","",
                                 "deg","deg","","",
                                 "pixel"};

static cpl_type g_tform[NCOLS]={CPL_TYPE_INT,    CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE,
                                CPL_TYPE_DOUBLE, CPL_TYPE_DOUBLE};

/* Apertures and areals */
static double g_apertures[NRADS];

static double g_rmults[] = {0.5,
		                    1. / CPL_MATH_SQRT2,
						    1.,
						    CPL_MATH_SQRT2,
                            2.,
						    2. * CPL_MATH_SQRT2,
                            4.,  5., 6., 7., 8., 10., 12.0};

static cpl_size g_nrcore  = 2;
static cpl_size g_n2rcore = 4;

static cpl_size g_areal_cols[NAREAL] = {COL_AREAL1, COL_AREAL2, COL_AREAL3,
                                        COL_AREAL4, COL_AREAL5, COL_AREAL6,
                                        COL_AREAL7, COL_AREAL8};


/*** Prototypes ***/

static cpl_error_code hdrl_do_seeing_gen(
		ap_t *ap, const char *col_ellipt, const char *col_pkht,
		const char *col_areals[NAREAL], cpl_size nobjects, cpl_table *tab);

static cpl_error_code hdrl_tabinit_gen(
		cpl_size ncols, cpl_type tform[], cpl_table **tab);


/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_catalogue_table    hdrl_catalogue_table
 * @ingroup  Catalogue
 *
 * @brief    Table for the catalogue
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Initialize catalogues
 *
 * @param    ap        The current ap structure
 * @param    xcol
 * @param    ycol
 * @param    cattype   The type of catalogue to be produced
 * @param    tab
 * @param    res
 *
 * @return   CPL_ERROR_NONE if all went well and CPL_ERROR_ILLEGAL_INPUT if catalogue type is unrecognised.
 *
 * Description:
 * 	    Wrapper routine to call the relevant initialisation routine for
 *      each of the allowed types of catalogues.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_tabinit(
		ap_t *ap, cpl_size *xcol, cpl_size *ycol, hdrl_catalogue_options cattype,
		cpl_table **tab, hdrl_casu_result *res)
{
	/* Define RA and Dec columns */
	*xcol = COL_X;
	*ycol = COL_Y;

    /* Call the generic routine to open a new output table */
    cpl_error_code e = hdrl_tabinit_gen(NCOLS, g_tform, tab);
    if (e == CPL_ERROR_NONE){

		if (cattype & HDRL_CATALOGUE_SEGMAP) {
			res->segmentation_map = cpl_image_new(ap->lsiz, ap->csiz, CPL_TYPE_INT);
		} else {
			res->segmentation_map = NULL;
		}

		if (cattype & HDRL_CATALOGUE_BKG) {
			res->background = cpl_image_new(ap->lsiz, ap->csiz, CPL_TYPE_DOUBLE);
		} else {
			res->background = NULL;
		}
    }

    return e;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Do seeing estimate
 *
 * @param    ap        The current ap structure
 * @param    nobjects  Number of objects
 * @param    tab
 *
 * @return   CPL_ERROR_NONE if all went well (Currently it's the only value).
 *
 * Description:
 * 	    Wrapper routine to call the relevant routine to work out the seeing
 *      for each of the allowed types of catalogues.
 *      Areal profiles are analysed and a seeing estimate is extracted.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_do_seeing(
		ap_t *ap, cpl_size nobjects, cpl_table *tab)
{
	/* Sort out the areal profile column names */
	const char *areal_colnames[NAREAL];
	for (cpl_size i = 0; i < NAREAL; i++) {
		areal_colnames[i] = (const char *)ttype[g_areal_cols[i] - 1];
	}

	/* Just call the generic seeing routine */
	return hdrl_do_seeing_gen( ap,
			                     ttype[COL_ELLIPT     - 1],
 			                     ttype[COL_PEAKHEIGHT - 1],
                                 areal_colnames, nobjects, tab);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Process the results for each object and store them in the table
 *
 * @param    ap          The current ap structure
 * @param    gain        The header keyword with the gain in e-/ADU
 * @param    nobjects
 * @param    tab
 * @param    res
 *
 * @return   CPL_ERROR_NONE if all is well. CPL_ERROR_ILLEGAL_INPUT if peak flux < 0
 *
 * Description:
 * 	    Wrapper routine to call the relevant routine to work out the results
 *      for each of the allowed types of catalogues.
 *      The pixel processing is done for all the parameters wanted.
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_process_results(
		ap_t *ap, double gain, cpl_size *nobjects, cpl_table *tab, hdrl_casu_result *res)
{
    /* Do a basic moments analysis and work out the areal profiles*/
	double momresults[8];
    hdrl_moments(ap, momresults);
    if (momresults[0] < 0) {
        return CPL_ERROR_ILLEGAL_INPUT;
    }

    cpl_size iareal[NAREAL];
    hdrl_areals(ap, iareal);

    /* See if this object makes the cut in terms of its size.  If not, then just return with good status */
    if (iareal[0] < ap->ipnop || momresults[3] < ap->xintmin) {
        return CPL_ERROR_NONE;
    }

    /* Work out the total flux */
    double ttotal;
    hdrl_extend( ap, momresults[3], momresults[1], momresults[2],
                   momresults[4], momresults[5], momresults[6], (double)(iareal[0]),
                   momresults[7], &ttotal);

    /* Try and deblend the images if it is requested and justified */
    cpl_size nbit;
    double parmall[IMNUM][NPAR];
    if (iareal[0] >= ap->mulpix && ap->icrowd) {
        hdrl_overlp( ap,
        		       parmall,
					   &nbit,
        		       momresults[1],
					   momresults[2],
					   momresults[3],
					   iareal[0],
					   momresults[7]);
    } else {
        nbit = 1;
    }

    if (nbit == 1) {

        parmall[0][0] = momresults[3];
        parmall[0][1] = momresults[1];
        parmall[0][2] = momresults[2];
        parmall[0][3] = ap->thresh;

        for (cpl_size i = 4; i < 8; i++) {
            parmall[0][i] = momresults[i];
        }

        for (cpl_size i = 0; i < NAREAL; i++) {
            parmall[0][i + 8] = (double)(iareal[i]);
        }

    } else {

    	cpl_size mbit = 0;
        for (cpl_size i = 0; i < nbit; i++) {

            if (   parmall[i][1] > 1. && parmall[i][1] < ap->lsiz
				&& parmall[i][2] > 1. && parmall[i][2] < ap->csiz) {

                for (cpl_size j = 0; j < NPAR; j++) {
                    parmall[mbit][j] = parmall[i][j];
                }

                mbit++;
            }
        }
        nbit = mbit;

        if (nbit == 0) {
            return CPL_ERROR_NONE;
        }
    }

    /* Create a list of g_apertures */
    double badpix[IMNUM];
    double skyvar[IMNUM];
    double avconf[IMNUM];
    for (cpl_size i = 0; i < NRADS; i++) {
        g_apertures[i] = g_rmults[i] * (ap->rcore);
        skyvar[i]      = CPL_MATH_PI * g_apertures[i] * g_apertures[i];
    }

    double rcore_area = CPL_MATH_PI * pow(ap->rcore, 2.);

    /* Initialise the badpix accumulator */
    for (cpl_size i = 0; i < nbit; i++) {
        badpix[i] = 0.;
        avconf[i] = 0.;
    }

    /* Get the core fluxes in all g_apertures */
    double cflux[NRADS * IMNUM];
    hdrl_phopt(ap, parmall, nbit, NRADS, g_apertures, cflux, badpix, g_nrcore, avconf);
    for (cpl_size i = 0; i < nbit; i++) {
        avconf[i] /= rcore_area;
    }

    /* Get half-light radius for all images */
    double half_flux[IMNUM];
    double half_rad[ IMNUM];
    for (cpl_size k = 0; k < nbit; k++) {

        half_flux[k] = 0.5 * (CPL_MAX(parmall[k][0], cflux[k * NRADS + g_n2rcore]));

        half_rad[k]  = hdrl_halflight( g_apertures,
        		                         cflux+k*NRADS,
										 half_flux[k],
                                         parmall[k][7],
										 NRADS);
    }

    /* Get Kron radius for all images and get the flux */
    double areal;
    double kron_flux[IMNUM];
    double kron_rad[ IMNUM];
    for (cpl_size k = 0; k < nbit; k++) {
        areal = parmall[k][8];
        kron_rad[k] = hdrl_kronrad(areal, g_apertures, cflux + k * NRADS, NRADS);
    }
    hdrl_flux(ap, parmall, nbit, kron_rad, kron_flux, NRADS, g_apertures, cflux);

    /* Get Petrosian radius for all images and get the flux */
    double petr_flux[IMNUM];
    double petr_rad[ IMNUM];
    for (cpl_size k = 0; k < nbit; k++) {
        areal = parmall[k][8];
        petr_rad[k] = hdrl_petrad(areal, g_apertures, cflux + k * NRADS, NRADS);
    }
    hdrl_flux(ap, parmall, nbit, petr_rad, petr_flux, NRADS, g_apertures, cflux);

    /* Massage the results and write them to the fits table */
    double sigsq = pow(ap->sigma, 2.);
    double radeg = 180. / CPL_MATH_PI;

    for (cpl_size k = 0; k < nbit; k++) {

        double sxx = parmall[k][4];
        double sxy = parmall[k][5];
        double syy = parmall[k][6];

        double srr = CPL_MAX(0.5, sxx + syy);

        if (sxy > 0.) {
          sxy = CPL_MAX( 1.e-4, CPL_MIN(sxy,  sqrt(sxx * syy)));
        } else {
          sxy = CPL_MIN(-1.e-4, CPL_MAX(sxy, -sqrt(sxx * syy)));
        }

        double ecc = sqrt((syy - sxx) * (syy - sxx) + 4. * sxy * sxy) / srr;

        double temp = CPL_MAX((1. - ecc) / (1. + ecc), 0.);
        double ell  = CPL_MIN(0.99, CPL_MAX(0., 1. - sqrt(temp)));

        double xx = 0.5 * (1. + ecc) * srr - sxx;
        double yy;

        double theta;
        if (xx == 0.) {
            theta = 0.;
        } else {
            theta = 90. - radeg * atan(sxy / xx);
        }
        double theta_ra = theta/radeg;

        double cc = (1. + ecc) * pow(cos(theta_ra), 2.) + (1. - ecc) * pow(sin(theta_ra), 2.);
        double dd = (1. + ecc) * pow(sin(theta_ra), 2.) + (1. - ecc) * pow(cos(theta_ra), 2.);

        /* Create a list of values */
        cpl_size nrows = cpl_table_get_nrow(tab);

        (*nobjects)++;
        if (*nobjects > nrows) {
            cpl_table_set_size(tab, nrows + INITROWS);
        }
        cpl_size nr = *nobjects - 1;

        double iso_flux = parmall[k][0];

        double apflux1  = cflux[k * NRADS +  0];
        double apflux2  = cflux[k * NRADS +  1];
        double apflux3  = cflux[k * NRADS +  2];
        double apflux4  = cflux[k * NRADS +  3];
        double apflux5  = cflux[k * NRADS +  4];
        double apflux6  = cflux[k * NRADS +  5];
        double apflux7  = cflux[k * NRADS +  6];
        double apflux8  = cflux[k * NRADS +  7];
        double apflux9  = cflux[k * NRADS +  8];
        double apflux10 = cflux[k * NRADS +  9];
        double apflux11 = cflux[k * NRADS + 10];
        double apflux12 = cflux[k * NRADS + 11];
        double apflux13 = cflux[k * NRADS + 12];

        double peak = parmall[k][7];

        xx = parmall[k][1];
        yy = parmall[k][2];

        double xxe = sqrt(  (2. * sigsq / (CPL_MATH_PI * peak * peak))
        		          +  cc / (2.   *  CPL_MATH_PI * gain * peak)
					 	  + 0.0001);

        double yye = sqrt(  (2. * sigsq / (CPL_MATH_PI * peak * peak))
        		          +  dd / (2.   *  CPL_MATH_PI * gain * peak)
						  +  0.0001);

        double sigma = sqrt(srr);
        double fwhm  = sqrt(sigma * sigma / 2.) * CPL_MATH_FWHM_SIG;
        /* heuristic correction of moment based fwhm obtained via simulated 2d gaussians,
         * with gaussians 4.3 corrects slighltly better but it is not very
         * significant and 4.0 is the same factor sextractor uses */
        fwhm -= 1. / (4. * fwhm);

        double areal1 = parmall[k][8];
        double areal2 = parmall[k][9];
        double areal3 = parmall[k][10];
        double areal4 = parmall[k][11];
        double areal5 = parmall[k][12];
        double areal6 = parmall[k][13];
        double areal7 = parmall[k][14];

        double areal8;
        if (nbit > 1 && k == 0) {
            areal8 = 0.;
        } else {
            areal8 = parmall[k][15];
        }

        double skylev;
        double skyrms;
        hdrl_backest(ap, xx, yy, &skylev, &skyrms);

        double kron_fluxe = sqrt(kron_flux[k]              / gain + (sigsq + skyrms * skyrms) * CPL_MATH_PI * pow(kron_rad[k], 2.));
        double petr_fluxe = sqrt(petr_flux[k]              / gain + (sigsq + skyrms * skyrms) * CPL_MATH_PI * pow(petr_rad[k], 2.));
        double half_fluxe = sqrt(CPL_MAX(0., half_flux[k]) / gain + (sigsq + skyrms * skyrms) * CPL_MATH_PI * pow(half_rad[k], 2.));

        double apflux1e   = sqrt(CPL_MAX(0., apflux1       / gain) + skyvar[0 ] * (sigsq + skyrms * skyrms));
        double apflux2e   = sqrt(CPL_MAX(0., apflux2       / gain) + skyvar[1 ] * (sigsq + skyrms * skyrms));
        double apflux3e   = sqrt(CPL_MAX(0., apflux3       / gain) + skyvar[2 ] * (sigsq + skyrms * skyrms));
        double apflux4e   = sqrt(CPL_MAX(0., apflux4       / gain) + skyvar[3 ] * (sigsq + skyrms * skyrms));
        double apflux5e   = sqrt(CPL_MAX(0., apflux5       / gain) + skyvar[4 ] * (sigsq + skyrms * skyrms));
        double apflux6e   = sqrt(CPL_MAX(0., apflux6       / gain) + skyvar[5 ] * (sigsq + skyrms * skyrms));
        double apflux7e   = sqrt(CPL_MAX(0., apflux7       / gain) + skyvar[6 ] * (sigsq + skyrms * skyrms));
        double apflux8e   = sqrt(CPL_MAX(0., apflux8       / gain) + skyvar[7 ] * (sigsq + skyrms * skyrms));
        double apflux9e   = sqrt(CPL_MAX(0., apflux9       / gain) + skyvar[8 ] * (sigsq + skyrms * skyrms));
        double apflux10e  = sqrt(CPL_MAX(0., apflux10      / gain) + skyvar[9 ] * (sigsq + skyrms * skyrms));
        double apflux11e  = sqrt(CPL_MAX(0., apflux11      / gain) + skyvar[10] * (sigsq + skyrms * skyrms));
        double apflux12e  = sqrt(CPL_MAX(0., apflux12      / gain) + skyvar[11] * (sigsq + skyrms * skyrms));
        double apflux13e  = sqrt(CPL_MAX(0., apflux13      / gain) + skyvar[12] * (sigsq + skyrms * skyrms));

        double peake      = sqrt(peak                      / gain  + sigsq               + skyrms * skyrms) ;

        /* Store away the results for this object */
        cpl_table_set_int(   tab, ttype[COL_NUMBER      - 1], nr, *nobjects);
        cpl_table_set_double(tab, ttype[COL_FLUXISO     - 1], nr, iso_flux);
        cpl_table_set_double(tab, ttype[COL_X           - 1], nr, xx);
        cpl_table_set_double(tab, ttype[COL_XERR        - 1], nr, xxe);
        cpl_table_set_double(tab, ttype[COL_Y           - 1], nr, yy);
        cpl_table_set_double(tab, ttype[COL_YERR        - 1], nr, yye);
        cpl_table_set_double(tab, ttype[COL_SIGMA       - 1], nr, sigma);
        cpl_table_set_double(tab, ttype[COL_ELLIPT      - 1], nr, ell);
        cpl_table_set_double(tab, ttype[COL_PA          - 1], nr, theta);
        cpl_table_set_double(tab, ttype[COL_AREAL1      - 1], nr, areal1);
        cpl_table_set_double(tab, ttype[COL_AREAL2      - 1], nr, areal2);
        cpl_table_set_double(tab, ttype[COL_AREAL3      - 1], nr, areal3);
        cpl_table_set_double(tab, ttype[COL_AREAL4      - 1], nr, areal4);
        cpl_table_set_double(tab, ttype[COL_AREAL5      - 1], nr, areal5);
        cpl_table_set_double(tab, ttype[COL_AREAL6      - 1], nr, areal6);
        cpl_table_set_double(tab, ttype[COL_AREAL7      - 1], nr, areal7);
        cpl_table_set_double(tab, ttype[COL_AREAL8      - 1], nr, areal8);
        cpl_table_set_double(tab, ttype[COL_PEAKHEIGHT  - 1], nr, peak);
        cpl_table_set_double(tab, ttype[COL_PKHTERR     - 1], nr, peake);
        cpl_table_set_double(tab, ttype[COL_APFLUX1     - 1], nr, apflux1);
        cpl_table_set_double(tab, ttype[COL_APFLUX1ERR  - 1], nr, apflux1e);
        cpl_table_set_double(tab, ttype[COL_APFLUX2     - 1], nr, apflux2);
        cpl_table_set_double(tab, ttype[COL_APFLUX2ERR  - 1], nr, apflux2e);
        cpl_table_set_double(tab, ttype[COL_APFLUX3     - 1], nr, apflux3);
        cpl_table_set_double(tab, ttype[COL_APFLUX3ERR  - 1], nr, apflux3e);
        cpl_table_set_double(tab, ttype[COL_APFLUX4     - 1], nr, apflux4);
        cpl_table_set_double(tab, ttype[COL_APFLUX4ERR  - 1], nr, apflux4e);
        cpl_table_set_double(tab, ttype[COL_APFLUX5     - 1], nr, apflux5);
        cpl_table_set_double(tab, ttype[COL_APFLUX5ERR  - 1], nr, apflux5e);
        cpl_table_set_double(tab, ttype[COL_APFLUX6     - 1], nr, apflux6);
        cpl_table_set_double(tab, ttype[COL_APFLUX6ERR  - 1], nr, apflux6e);
        cpl_table_set_double(tab, ttype[COL_APFLUX7     - 1], nr, apflux7);
        cpl_table_set_double(tab, ttype[COL_APFLUX7ERR  - 1], nr, apflux7e);
        cpl_table_set_double(tab, ttype[COL_APFLUX8     - 1], nr, apflux8);
        cpl_table_set_double(tab, ttype[COL_APFLUX8ERR  - 1], nr, apflux8e);
        cpl_table_set_double(tab, ttype[COL_APFLUX9     - 1], nr, apflux9);
        cpl_table_set_double(tab, ttype[COL_APFLUX9ERR  - 1], nr, apflux9e);
        cpl_table_set_double(tab, ttype[COL_APFLUX10    - 1], nr, apflux10);
        cpl_table_set_double(tab, ttype[COL_APFLUX10ERR - 1], nr, apflux10e);
        cpl_table_set_double(tab, ttype[COL_APFLUX11    - 1], nr, apflux11);
        cpl_table_set_double(tab, ttype[COL_APFLUX11ERR - 1], nr, apflux11e);
        cpl_table_set_double(tab, ttype[COL_APFLUX12    - 1], nr, apflux12);
        cpl_table_set_double(tab, ttype[COL_APFLUX12ERR - 1], nr, apflux12e);
        cpl_table_set_double(tab, ttype[COL_APFLUX13    - 1], nr, apflux13);
        cpl_table_set_double(tab, ttype[COL_APFLUX13ERR - 1], nr, apflux13e);
        cpl_table_set_double(tab, ttype[COL_PETRAD      - 1], nr, 0.5 * petr_rad[k]);
        cpl_table_set_double(tab, ttype[COL_KRONRAD     - 1], nr, 0.5 * kron_rad[k]);
        cpl_table_set_double(tab, ttype[COL_HALFRAD     - 1], nr, half_rad[k]);
        cpl_table_set_double(tab, ttype[COL_PETFLUX     - 1], nr, petr_flux[k]);
        cpl_table_set_double(tab, ttype[COL_PETFLUXERR  - 1], nr, petr_fluxe);
        cpl_table_set_double(tab, ttype[COL_KRONFLUX    - 1], nr, kron_flux[k]);
        cpl_table_set_double(tab, ttype[COL_KRONFLUXERR - 1], nr, kron_fluxe);
        cpl_table_set_double(tab, ttype[COL_HALFFLUX    - 1], nr, half_flux[k]);
        cpl_table_set_double(tab, ttype[COL_HALFFLUXERR - 1], nr, half_fluxe);
        cpl_table_set_double(tab, ttype[COL_ERRFLAG     - 1], nr, badpix[k]);
        cpl_table_set_double(tab, ttype[COL_SKYLEVEL    - 1], nr, skylev);
        cpl_table_set_double(tab, ttype[COL_SKYSIGMA    - 1], nr, skyrms);
        cpl_table_set_double(tab, ttype[COL_AVCONF      - 1], nr, avconf[k]);
        cpl_table_set_double(tab, ttype[COL_FWHM        - 1], nr, fwhm);

        /* Store away some dummy values to avoid problems later on */
        double zero = 0.;
        cpl_table_set_double(tab, ttype[COL_RA          - 1], nr, zero);
        cpl_table_set_double(tab, ttype[COL_DEC         - 1], nr, zero);
        cpl_table_set_double(tab, ttype[COL_CLASS       - 1], nr, 100.);
        cpl_table_set_double(tab, ttype[COL_STAT        - 1], nr, zero);
    }

    /* Now that everything is okay - fill in the segmenataion map */
    cpl_msg_info(cpl_func, "Num objects found in catalogue: %lld", *nobjects);
    if (res->segmentation_map) {
        for (cpl_size index = 0; index < ap->npl_pix; index++) {
            cpl_image_set( res->segmentation_map,
            		       ap->plarray[index].x,
                           ap->plarray[index].y,
						   *nobjects);
        }
    }

    return CPL_ERROR_NONE;
}

/**@}*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Initialise tables (generic)
 *
 * @param    ncols     The number of columns in the table
 * @param    ttype     Array of column names for FITS table
 * @param    tunit     Array of units for each of the columns
 * @param    tform     Array of formats for each of the columns as defined in the FITS standard
 * @param    tab
 *
 * @return   CPL_ERROR_NONE if all is well, CPL_ERROR_ILLEGAL_INPUT in other case
 *
 * Description:
 * 	    Generic routine to create FITS tables for the output catalogues.
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code hdrl_tabinit_gen(
		cpl_size ncols, cpl_type tform[], cpl_table **tab)
{
	/* First, create the table with a default number of rows. */
    if ((*tab = cpl_table_new(0)) == NULL) {

    	cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
    			"hdrl_cat_tabinit_gen - Unable to open cpl table!");
    	return CPL_ERROR_ILLEGAL_INPUT;

    } else {

		/* Now define all of the columns */
		for (cpl_size i = 0; i < ncols; i++) {

			cpl_table_new_column(     *tab, ttype[i], tform[i]);
			cpl_table_set_column_unit(*tab, ttype[i], tunit[i]);
		}

		return CPL_ERROR_NONE;
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Do seeing estimate (generic)
 *
 * @param    ap          The current ap structure
 * @param    col_ellipt  The name of the column for ellipticity
 * @param    col_pkht    The name of the column for the peak height
 * @param    col_areals  The array of names of the areal profile columns
 * @param    nobjects
 * @param    tab
 *
 * @return   CPL_ERROR_NONE if all it's ok (This is currently the only value.)
 *
 * Description:
 * 	    Wrapper routine for doing the seeing estimate
 */
/* ---------------------------------------------------------------------------*/
static cpl_error_code hdrl_do_seeing_gen(
		ap_t *ap, const char *col_ellipt, const char *col_pkht,
		const char *col_areals[NAREAL], cpl_size nobjects, cpl_table *tab)
{
    /* Get some space and read the relevant columns */
    double fwhm;
    if (nobjects >= 3) {

        double *areal[NAREAL];
        for (cpl_size i = 0; i < NAREAL; i++) {
            areal[i]  = cpl_table_get_data_double(tab, col_areals[i]);
        }

    	double *ellipt = cpl_table_get_data_double(tab, col_ellipt);
    	double *pkht   = cpl_table_get_data_double(tab, col_pkht);

        /* Do the seeing calculation */
    	double *work = cpl_malloc(nobjects * sizeof(double));
        hdrl_seeing(ap, nobjects, ellipt, pkht, areal, work, &fwhm);
        cpl_free(work);

    } else {
        fwhm = 0.;
    }

    ap->fwhm = fwhm;

    return CPL_ERROR_NONE;
}
