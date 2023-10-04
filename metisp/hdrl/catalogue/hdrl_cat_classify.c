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

#include "hdrl_cat_classify.h"

#include "hdrl_cat_casu.h"
#include "hdrl_cat_table.h"
#include "hdrl_cat_utils_sort.h"


/*** DEFINES ***/

#define STEP      0.05                     /*  */
#define NSAMPLE   150                      /*  */
#define MAXLOOP   5                        /*  */

#define NCOLFULL  15                       /*  */
#define FRAMECUT  0.05                     /*  */


/*** GLOBAL VARIABLES (INTO FILE) ***/

/* Make the data arrays and header values global */
static cpl_size   g_nrows;
static double     g_thresh;
static double     g_skylevel;
static double     g_skynoise;
static double     g_rcore;

/* Derived values */
static cpl_size   g_poor;
static double     g_sigell, g_fitell, g_elllim, g_sigellf, g_fitellf, g_sigpa, g_fitpa;
static double     g_blim, g_flim, g_cmin, g_cmax;
static double     g_fit1, g_fit2, g_fit3, g_fit4, g_fit5, g_fit6, g_fit7;
static double     g_fit_final, g_sigma_final;
static double     *g_lower1, *g_lower2, *g_lower3, *g_upper1, *g_upper2, *g_upper3, *g_uppere;
static double     g_avsig1, g_avsig2, g_avsig3, g_wt1, g_wt2, g_wt3;

/* Classification values */
static cpl_size   g_nstar, g_ngal, g_njunk, g_ncmp;

/* Values for the data quality and aperture corrections */
static double     g_avsat, g_corlim, g_cormin, g_apcpkht;
static double     g_apcor1, g_apcor2, g_apcor3, g_apcor4, g_apcor5;
static double     g_apcor6, g_apcor7;

/* Data arrays */
static double     *g_workspace = NULL;
static cpl_table  *g_catcopy = NULL;
static double     *g_areal[NAREAL];
static double     *g_core_flux,  *g_core1_flux, *g_core2_flux, *g_core3_flux;
static double     *g_core4_flux, *g_core5_flux, *g_core6_flux;
static double     *g_peak_height, *g_peak_mag, *g_ellipticity, *g_iso_flux;
static double     *g_total_flux, *g_cls, *g_sig, *g_xpos, *g_ypos, *g_pa, *g_skylev;

/* Column definitions */
static double     g_xmin, g_xmax, g_ymin, g_ymax;
static double     g_pixlim;
static const char *g_colsfull[NCOLFULL] = {
	"Aper_flux_3", "Aper_flux_1", "Aper_flux_4", "Aper_flux_5", "Aper_flux_6",
	"Peak_height", "Ellipticity", "Isophotal_flux", "Isophotal_flux",
	"Aper_flux_7", "X_coordinate","Y_coordinate", "Position_angle","Sky_level",
    "Aper_flux_2"};


/*** PROTOTYPES ***/

static void anhist(double *data, cpl_size n, double *medval, double *sigma);

static void boundaries(double *core1, double *core2, double *core3, double medval1,
		               double sigma1, double medval2, double sigma2, cpl_size small,
				       double area1, double area2,
				       double *wt, double *avsig, double *lower, double *upper);
static void boundpk(   double *core, double *pkht, double medval, double sigma,
		               double *wt, double *avsig, double *lower, double *upper);

static void classify_run(void);

static void classstats(      double *core1, double *core2, cpl_size small, double cutlev,
                             double *medval, double *sigma);
static void classstats_ap0(  double *medval, double *sigma);
static void classstats_ap67( double *mag1, double *mag2, double *medval, double *sigma);
static void classstats_el(   void);
static void classstats_pa(   void);
static void classstats_ellf( double);
static void classstats_final(void);

static void medstat(double *array, cpl_size n, double *medval, double *sigval);


/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_classify         hdrl_classify
 * @ingroup  Catalogue
 *
 * @brief    Do star/galaxy classification
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Do star/galaxy classification
 *
 * @param    catalogue The input catalogue
 * @param    minsize   The minimum size in pixels of objects to be used in the analysis
 *
 * @return   CPL_ERROR_CODE if everything is OK, the concrete error in other case
 *
 * Description:
 * 	    The information in an catalogue is scanned and each object
 *      is classified based on a number of shape criteria.
 *
 * @note  QC headers:
 *      The following QC parameters are read from the catalogue extension.
 *      - \b MEAN_SKY:      The mean sky found by catalogue.
 *      - \b SKY_NOISE:     The sky noise found by catalogue.
 *      The following QC parameters are written to the catalogue extension.
 *      - \b IMAGE_SIZE: The average FWHM of stellar objects in the catalogue
 *      - \b ELLIPTICITY:   The average stellar ellipticity.
 *      - \b POSANG:        The average position angle on the image.
 *      - \b APERTURE_CORR: The aperture correction for an aperture with a radius of Rcore.
 *      - \b NOISE_OBJ:     The number of noise objects detected on the image
 * @note DRS headers:
 *      The following DRS parameters are read from the catalogue extension.
 *      - \b THRESHOL:      The detection threshold used by catalogue
 *      - \b RCORE:         The core radius used by catalogue
 *      - \b SEEING:        The averaged seeing found by catalogue
 *      - \b NXOUT:         The number of pixels in a row of the original image
 *      - \b NYOUT:         The number of pixels in a column of the original image
 *      The following DRS parameters are written to the catalogue extension
 *      - \b CLASSIFD:      Set if the catalogue has been classified
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_classify(hdrl_casu_tfits *catalogue, double minsize)
{
    /* Get the number of columns and decide which column labels to use */
    cpl_table *cat = hdrl_casu_tfits_get_table(catalogue);
    if (cpl_table_get_ncol(cat) != NCOLS) {
    	return CPL_ERROR_INCOMPATIBLE_INPUT;
    }

	/* Get some DQC info from the extra propertylist generated by catalogue */
    cpl_propertylist *extra = hdrl_casu_tfits_get_ehu(catalogue);

    double   fwhm  = cpl_propertylist_get_double(extra, "ESO DRS SEEING"  );
    cpl_size nxout = cpl_propertylist_get_int(   extra, "ESO DRS NXOUT"   );
    cpl_size nyout = cpl_propertylist_get_int(   extra, "ESO DRS NYOUT"   );

    g_thresh       = cpl_propertylist_get_double(extra, "ESO DRS THRESHOL");
    g_skylevel     = cpl_propertylist_get_double(extra, "ESO QC MEAN_SKY" );
    g_skynoise     = cpl_propertylist_get_double(extra, "ESO QC SKY_NOISE");
    g_rcore        = cpl_propertylist_get_double(extra, "ESO DRS RCORE"   );

    g_xmin         =       FRAMECUT  * (double)nxout;
    g_xmax         = (1. - FRAMECUT) * (double)nxout;
    g_ymin         =       FRAMECUT  * (double)nyout;
    g_ymax         = (1. - FRAMECUT) * (double)nyout;
    g_pixlim       = minsize;

    /* Make a copy of the table as you are going to muck about with the
     *  column values. Get the column data */
    g_catcopy = cpl_table_duplicate(cat);
    g_nrows   = cpl_table_get_nrow(cat);

    g_cls = cpl_table_get_data_double(cat, "Classification");
    g_sig = cpl_table_get_data_double(cat, "Statistic");

    const char *cols[ NCOLFULL];
    for (cpl_size i = 0; i < NCOLFULL; i++) {
        cols[ i] = g_colsfull[ i];
    }

    g_core_flux   = cpl_table_get_data_double(g_catcopy, cols[ 0]);
    g_core1_flux  = cpl_table_get_data_double(g_catcopy, cols[ 1]);
    g_core2_flux  = cpl_table_get_data_double(g_catcopy, cols[ 2]);
    g_core3_flux  = cpl_table_get_data_double(g_catcopy, cols[ 3]);
    g_core4_flux  = cpl_table_get_data_double(g_catcopy, cols[ 4]);
    g_peak_height = cpl_table_get_data_double(g_catcopy, cols[ 5]);
    g_ellipticity = cpl_table_get_data_double(g_catcopy, cols[ 6]);
    g_iso_flux    = cpl_table_get_data_double(g_catcopy, cols[ 7]);
    g_total_flux  = cpl_table_get_data_double(g_catcopy, cols[ 8]);
    g_core5_flux  = cpl_table_get_data_double(g_catcopy, cols[ 9]);
    g_xpos        = cpl_table_get_data_double(g_catcopy, cols[10]);
    g_ypos        = cpl_table_get_data_double(g_catcopy, cols[11]);
    g_pa          = cpl_table_get_data_double(g_catcopy, cols[12]);
    g_skylev      = cpl_table_get_data_double(g_catcopy, cols[13]);
    g_core6_flux  = cpl_table_get_data_double(g_catcopy, cols[14]);

    /* Get some workspace */
    g_workspace = cpl_malloc(2 * g_nrows * sizeof(*g_workspace));
    g_peak_mag = g_workspace;
    
    double *work = g_workspace + g_nrows;

    /* Convert fluxes to "magnitudes" */
    for (cpl_size i = 0; i < g_nrows; i++) {

        g_core_flux[i]  = 2.5 * log10(CPL_MAX(g_core_flux[i], 1.));
        g_core1_flux[i] = 2.5 * log10(CPL_MAX(g_core1_flux[i], 1.));
        g_core2_flux[i] = 2.5 * log10(CPL_MAX(g_core2_flux[i], 1.));
        g_core3_flux[i] = 2.5 * log10(CPL_MAX(g_core3_flux[i], 1.));
        g_core4_flux[i] = 2.5 * log10(CPL_MAX(g_core4_flux[i], 1.));
        g_core5_flux[i] = 2.5 * log10(CPL_MAX(g_core5_flux[i], 1.));

        double moff     = 1. / (1. - pow((g_thresh / CPL_MAX(g_peak_height[i] ,g_thresh)), 0.6));
        g_iso_flux[i]   = 2.5 * log10(CPL_MAX(moff *g_iso_flux[i], 1.));

        g_peak_mag[i]   = 2.5 * log10(CPL_MAX(g_peak_height[i] - g_skynoise, 0.1));
    }

    if (g_core6_flux != NULL) {
        for (cpl_size i = 0; i < g_nrows; i++) {
            g_core6_flux[i] =  2.5 * log10(CPL_MAX(g_core6_flux[i], 1.));
        }
    }

    /*  Now get the g_areal profile information. You'll need this in a sec */
    for (cpl_size i = 0; i < NAREAL; i++) {
        char colname[32];
        sprintf(colname, "Areal_%ld_profile", (long int)i + 1);
        g_areal[i] = cpl_table_get_data_double(g_catcopy, colname);
    }

    /* What is the seeing like? */
    g_poor = 0;
    if (fwhm > CPL_MAX(5., g_rcore * sqrt(2.))) g_poor = 1;

    /* Ok, now call the routine that does all the work */
    classify_run();

    /* Right, now get a better estimate of the seeing */
    cpl_size n = 0;
    for (cpl_size i = 0; i < g_nrows; i++) {

        double ell  = g_ellipticity[i];
        double core = g_core_flux[  i];
        double pkht = g_peak_height[i];

        if (g_cls[i] == -1. && ell < g_elllim && core < g_corlim && pkht > 10. * g_thresh) {

            double   ap    = log(0.5 * pkht / g_thresh) / log(2.) + 1.;
            cpl_size iap   = (cpl_size)ap;
            double   delap = ap - (double)iap;

            if (iap > 0 && iap < NAREAL && g_areal[1][i] > 0.) {

                double area = g_areal[iap - 1][i]*(1. - delap) + g_areal[iap][i] * delap;
                work[n++] = 2. * sqrt(area / CPL_MATH_PI);
            }
        }
    }

    if (n > 2) {

    	double junk;
        medstat(work, n, &fwhm, &junk);
       
        /* Allow for finite pixel size */
        double arg = (0.25 * CPL_MATH_PI * fwhm * fwhm) - 1;
        fwhm = 2. * sqrt(CPL_MAX(0., arg / CPL_MATH_PI));
       
    } else {
        fwhm = -1.;
    }

	if (g_catcopy != NULL) {
		cpl_table_delete(g_catcopy);
		g_catcopy = NULL;
	}


    /* Qc keywords */
    cpl_propertylist_update_double(extra, "ESO QC IMAGE_SIZE",    fwhm);
    cpl_propertylist_update_double(extra, "ESO QC ELLIPTICITY",   g_fitell);
    cpl_propertylist_update_double(extra, "ESO QC POSANG",        g_fitpa);
    cpl_propertylist_update_double(extra, "ESO QC APERTURE_CORR", g_apcor3);
    cpl_propertylist_update_int(   extra, "ESO QC NOISE_OBJ",     g_njunk);
    cpl_propertylist_update_double(extra, "ESO QC SATURATION",    g_avsat);

    cpl_propertylist_set_comment(  extra, "ESO QC IMAGE_SIZE",    "[pixels] Average FWHM of stellar objects");
    cpl_propertylist_set_comment(  extra, "ESO QC ELLIPTICITY",   "Average stellar ellipticity (1-b/a)");
    cpl_propertylist_set_comment(  extra, "ESO QC POSANG",        "[degrees] Median position angle");
    cpl_propertylist_set_comment(  extra, "ESO QC APERTURE_CORR", "Stellar ap-corr 1x core flux");
    cpl_propertylist_set_comment(  extra, "ESO QC NOISE_OBJ",     "Number of noise objects");

    
    /* DRS keywords */
    cpl_propertylist_update_bool(  extra, "ESO DRS CLASSIFD",     1);

    cpl_propertylist_set_comment(  extra, "ESO DRS CLASSIFD",     "Catalogue has been classified");


    /* Aperture correction keywords */
    cpl_propertylist_update_double(extra, "APCORPK",              g_apcpkht);
    cpl_propertylist_update_double(extra, "APCOR1",               g_apcor1);
    cpl_propertylist_update_double(extra, "APCOR2",               g_apcor2);
    cpl_propertylist_update_double(extra, "APCOR3",               g_apcor3);
    cpl_propertylist_update_double(extra, "APCOR4",               g_apcor4);
    cpl_propertylist_update_double(extra, "APCOR5",               g_apcor5);
    cpl_propertylist_update_double(extra, "APCOR6",               g_apcor6);
    cpl_propertylist_update_double(extra, "APCOR7",               g_apcor7);

    cpl_propertylist_set_comment(  extra, "APCORPK",              "Stellar aperture correction - peak height");
    cpl_propertylist_set_comment(  extra, "APCOR1",               "Stellar aperture correction - 1/2x core flux");
    cpl_propertylist_set_comment(  extra, "APCOR2",               "Stellar aperture correction - core/sqrt(2) flux");
    cpl_propertylist_set_comment(  extra, "APCOR3",               "Stellar aperture correction - 1x core flux");
    cpl_propertylist_set_comment(  extra, "APCOR4",               "Stellar aperture correction - sqrt(2)x core flux");
    cpl_propertylist_set_comment(  extra, "APCOR5",               "Stellar aperture correction - 2x core flux");
    cpl_propertylist_set_comment(  extra, "APCOR6",               "Stellar aperture correction - 2*sqrt(2)x core flux");
    cpl_propertylist_set_comment(  extra, "APCOR7",               "Stellar aperture correction - 4x core flux");


    /* Write header information to help GAIA */
    cpl_propertylist_update_string(extra, "SYMBOL1",              "{Ellipticity Position_angle Areal_1_profile Classification} {el");
    cpl_propertylist_update_string(extra, "SYMBOL2",              "lipse blue (1.0-$Ellipticity) $Position_angle+90 {} $Classific");
    cpl_propertylist_update_string(extra, "SYMBOL3",              "ation==1} {sqrt($Areal_1_profile*(1.0-$Ellipticity)/3.142)} : {");
    cpl_propertylist_update_string(extra, "SYMBOL4",              "Ellipticity Position_angle Areal_1_profile Classification} {el");
    cpl_propertylist_update_string(extra, "SYMBOL5",              "lipse red (1.0-$Ellipticity) $Position_angle+90 {} $Classific");
    cpl_propertylist_update_string(extra, "SYMBOL6",              "ation==-1} {sqrt($Areal_1_profile*(1.0-$Ellipticity)/3.142)} :");
    cpl_propertylist_update_string(extra, "SYMBOL7",              "{Ellipticity Position_angle Areal_1_profile Classification} {el");
    cpl_propertylist_update_string(extra, "SYMBOL8",              "lipse green (1.0-$Ellipticity) $Position_angle+90 {} $Classifi");
    cpl_propertylist_update_string(extra, "SYMBOL9",              "cation==0} {sqrt($Areal_1_profile*(1.0-$Ellipticity)/3.142)}");


    /* Clean up */
    if (g_workspace != NULL) {
    	cpl_free(g_workspace);
    	g_workspace = NULL;
    }

    return CPL_ERROR_NONE;
}

/**@}*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Analyse a histogram to give a median and sigma
 *
 * @param    data      The input data array
 * @param    n         The size of the input data array
 * @param    medval    The output median value
 * @param    sigma     The output sigma value
 *
 * Description:
 * 	    The entries in a data array are histogrammed. The histogram is
 *      analsyed to work out a median and a sigma. A certain amount of
 *      smoothing is done and extra searches are also done to make sure
 *      that they median position estimate is the best it can be.
 */
/* ---------------------------------------------------------------------------*/
static void anhist(double *data, cpl_size n, double *medval, double *sigma)
{
    #define MAXHIST   66536   /* maximum size of histogram array */

    /* Get some workspace for the histogram */
    cpl_size *histo = cpl_calloc(MAXHIST, sizeof(cpl_size));
    double   *sval  = cpl_calloc(MAXHIST, sizeof(double));

    /* Sort data into the histogram */
    for (cpl_size i = 0; i < n; i++) {

    	double   aux  = data[i] / STEP;
    	cpl_size ilev = (cpl_size)(aux + (aux < 0. ? -0.5 : 0.5));

        if (ilev >= -10 && ilev <= 100) {
            ilev += 10;
            histo[ilev] += 1;
        }
    }

    /* Now find the maximum of the histogram and its position ... */
    double   hmax = 0.;
    cpl_size imax = 0;
    for (cpl_size i = 0; i < MAXHIST; i++) {
        if (histo[i] > hmax) {
            hmax = (double)(histo[i]);
            imax = i;
        }
    }

    /* Trap for hmax == 0 */
    if (hmax == 0.) {

        if (n >= 10) {
            *medval = data[(n+1)/2-1];
            *sigma  = CPL_MATH_STD_MAD * 0.5 * (data[(3 * n + 3) / 4 - 1] - data[(n + 3) / 4 - 1]);
        } else {
            *medval = 0.;
            *sigma  = 1.;
        }

    } else {

		/* Now do three point running average to see if there are other local maxime */
		double   smax  = 0.;
		cpl_size ismax = 0;
		for (cpl_size i = 1; i < MAXHIST-1; i++) {
			sval[i] = (histo[i - 1] + histo[i] + histo[i + 1]) / 3.;
			if (sval[i] > smax) {
				smax  = sval[i];
				ismax = i;
			}
		}

		if (ismax < imax) {
			imax = ismax;
			hmax = (double)(histo[imax]);
		}

		/* Now check for lower local maxima */
		for (cpl_size i = imax-1; i > 0; i--) {
			if (sval[i] >= sval[i + 1] && sval[i] >= sval[i - 1]) {
				if (sval[i] > 0.5 * smax)
					ismax = i;
			}
		}

		if (ismax < imax) {
			imax = ismax;
			hmax = (double)(histo[imax]);
		}

		/* Now work out where the peak is */
		*medval = CPL_MIN((double)(imax - 10) * STEP, data[(n + 1) / 2 - 1]);

		double aux  = 0.5 * hmax;
		double hlim = (cpl_size)(aux + (aux < 0. ? -0.5 : 0.5));

		cpl_size i = 1;
		while (imax - i > 1 && histo[imax - i] > hlim) {
			i++;
		}

		if (imax - i >= 0) {
			double ratio = hmax / CPL_MAX(1., (double)(histo[imax - i]));
			*sigma = (double)i * STEP / (sqrt(2.) * CPL_MAX(1., log(ratio)));
			*sigma = CPL_MAX(*sigma, 0.5 * STEP);
		} else {
			*sigma = 1.;
		}
    }
    
	/* Clean up */
	if (histo) cpl_free(histo);
	if (sval)  cpl_free(sval);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Work out boundaries of the stellar locus
 *
 * @param    core1     The flux array for the primary aperture (in magnitudes)
 * @param    core2     The flux array for the secondary aperture in the case of good seeing
 * @param    core3     The flux array for the secondary aperture in the case of poor seeing
 * @param    medval1   The median of magnitude difference of core1 and core2
 * @param    sigma1    The sigma of magnitude difference of core1 and core2
 * @param    medval2   The median of magnitude difference of core1 and core3
 * @param    sigma2    The sigma of magnitude difference of core1 and core3
 * @param    small     This parameter changes sign in internal computation
 * @param    area1     The area of the larger of the apertures for core1,core2 comparison
 * @param    area2     The area of the larger of the apertures for core1,core3 comparison
 * @param    wt        An output weight for the estimate based on the scatter in the
 *                       magnitude differences
 * @param    avsig     An output average magnitude difference for the apertures used.
 * @param    lower     An array delimiting the lower boundary
 * @param    upper     An array delimiting the upper boundary
 *
 * Description:
 * 	    A number of flux estimates are given along with comparison statistics
 *      between those fluxes.
 */
/* ---------------------------------------------------------------------------*/
static void boundaries(double *core1, double *core2, double *core3, double medval1,
                       double sigma1, double medval2, double sigma2, cpl_size small,
                       double area1, double area2,
				       double *wt, double *avsig, double *lower, double *upper)
{
    /* Get a workspace */
    double *work = cpl_malloc(g_nrows * sizeof(double));

    /* Initialise the lower boundary */
    lower[0] = g_cmin;
    lower[1] = g_cmax;
    
    double asign = (small == 1 ? -1. : 1.);

    /* Now collect the data */
    cpl_size n = 0;
    for (cpl_size i = 0; i < g_nrows; i++) {

        double c1 = core1[i];

        if (!g_poor) {

            double c2 = core2[i];
            double dc = asign * (c2 - c1);

            if (dc > medval1 - 3. * sigma1 && c1 < g_blim - 3.) {
                work[n++] = dc - medval1;
            }

        } else {

            double c2 = core3[i];
            double dc = c2 - c1;

            if (dc > medval2 - 3. * sigma2 && c1 < g_blim - 3.) {
                work[n++] = dc - medval2;
            }
        }
    }
 
    /* Find the median */
    double junk;
    medstat(work, n, avsig, &junk);
    cpl_free(work);

    /* Work out sigma levels for both types of seeing */
    double xnoise;
    if (! g_poor) {
        *wt = CPL_MIN(5., CPL_MAX(1., * avsig / sigma1));
        xnoise = sqrt(area1) * g_skynoise;
    } else {
        *wt = CPL_MIN(2.5, CPL_MAX(1., * avsig / sigma2));
        xnoise = sqrt(area2) * g_skynoise;
    }

    /* Now work out the boundaries */
    /* The term pow(10.0,(double)(0.4*xmag)); comes from the magitude formula */
    for (cpl_size i = 0; i < NSAMPLE; i++) {

    	double xmag  = 5. + (double)(i + 1) * 0.1;
    	double xflux = pow(10., (double)(0.4 * xmag));
    	double ratio = 2.5 * log10(CPL_MAX(1. + xnoise / xflux, 0.));

        if (! g_poor) {
            lower[i] = medval1 - 3. * sqrt(sigma1 * sigma1 + ratio * ratio);
            upper[i] = medval1 + 3. * sqrt(sigma1 * sigma1 + 0.5 * ratio * ratio);
        } else {
            lower[i] = medval2 - 3. * sqrt(sigma2 * sigma2 + ratio * ratio);
            upper[i] = medval2 + 3. * sqrt(sigma2 * sigma2 + 0.5 * ratio * ratio);
        }
    }

    upper[0] = (g_poor == 0 ? medval1 : medval2);
    upper[1] = upper[0];
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Work out boundaries of the stellar locus using peak flux
 *
 * @param    core       The flux array for the primary aperture (in magnitudes)
 * @param    pkht       The array of magnitudes based on the peak height
 * @param    medval     The median of magnitude difference of core and pkht
 * @param    sigma      The sigma of magnitude difference of core and pkht
 * @param    wt         An output weight for the estimate based on the scatter in the
 *                        magnitude differences
 * @param    avsig      An output average magnitude difference for the two estimates
 * @param    lower      An array delimiting the lower boundary
 * @param    upper      An array delimiting the upper boundary
 *
 * Description:
 * 	    The boundaries of the stellar locus are located using the core
 *      flux and the peak flux.
 */
/* ---------------------------------------------------------------------------*/
static void boundpk(double *core, double *pkht, double medval, double sigma,
                    double *wt, double *avsig, double *lower, double *upper)
{
    /* Get the space for the boundry lines and a workspace */
    double *work = cpl_malloc(g_nrows * sizeof(double));

    /* Collect the data */
    cpl_size n = 0;
    for (cpl_size i = 0; i < g_nrows; i++) {

        double c = core[i];
        double p = pkht[i];

        if (c - p > medval - 3. * sigma && c < g_blim - 3.) {
            work[n++] = c - p - medval;
        }
    }

    /* Find the median */
    double junk;
    medstat(work, n, avsig, &junk);
    cpl_free(work);

    *wt = CPL_MIN(5., CPL_MAX(1., *avsig / sigma));

    /* Now work out boundaries */
    double xnoise = sqrt(CPL_MATH_PI * g_rcore * g_rcore) * g_skynoise;
    for (cpl_size i = 0; i < NSAMPLE; i++) {

        double xmag = 5.0 + (double)(i + 1) * 0.1;
        double pmag = xmag - medval;

        /* The term pow(10.0,(double)(0.4*xmag)) comes from the magnitude formula */
        double xflux = pow(10., (double)(0.4 * xmag));
        double pflux = pow(10., (double)(0.4 * pmag));

        /* The term 2.5*log10 comes from the magnitude formula */
        double ratio = 2.5 * log10((double)(1. + CPL_MAX(xnoise / xflux, g_skynoise / pflux)));

        lower[i] = medval - 3. * sqrt(sigma * sigma + ratio * ratio);
        upper[i] = medval + 3. * sqrt(sigma * sigma + 0.5 * ratio * ratio);
    }

    upper[0] = medval;
    upper[1] = upper[0];
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Main driver routine
 *
 * Description:
 * 	    This is the main driver routine for classify. It calls all the
 *      statistical routines and the boundary finding routines. It works
 *      out the aperture corrections and does the final classification for
 *      all objects in the catalogue.
 */
/* ---------------------------------------------------------------------------*/
static void classify_run(void)
{
	#define BLIMDEF   15.
	#define FLIMDEF   11.
    #define CMINDEF   7.5
    #define CMAXDEF   15.

	/* Update faint limit to cope with short exposures */
    g_blim = BLIMDEF;
    g_flim = FLIMDEF;

    /* the following formula comes from the magnitude computation */
    double fluxlim = 2.5 * log10((double)(5. * sqrt(CPL_MATH_PI * g_rcore * g_rcore) * g_skynoise));

    g_flim   = CPL_MIN(g_flim, CPL_MAX( 6.,  fluxlim + 3.));
    g_corlim = CPL_MIN(g_blim, CPL_MAX(12.5, fluxlim + 5.));
    g_cormin = CPL_MIN(g_blim, CPL_MAX(12.5, fluxlim + 5.));

    /* Work out min and max core flux */
    g_cmin = CMINDEF;
    g_cmax = CMAXDEF;
    for (cpl_size i = 0; i < g_nrows; i++) {
        double xflux = g_core_flux[i];
        g_cmin = CPL_MIN(g_cmin, xflux);
        g_cmax = CPL_MAX(g_cmax, xflux);
    }
    g_cmin  = CPL_MAX(fluxlim - 0.5, g_cmin);
    g_cmax += 0.1;
    g_cmax  = CPL_MIN(g_cmax, 20.);

    /* Work out g_ellipticity stats for likely stellar objects */
    classstats_el();

    /* Get the classification statistics for each of the tests */
    double sigma1, sigma2, sigma3, sigma4, sigma5, sigma6, sigma7;
    classstats(g_core_flux, g_core1_flux, 1, 0.2, &g_fit1, &sigma1);	/* Core flux vs         1/2 * core flux */
    classstats(g_core_flux, g_core3_flux, 0, 0.1, &g_fit2, &sigma2);	/* Core flux vs           2 * core flux */
    classstats(g_core_flux, g_core2_flux, 0, 0.0, &g_fit4, &sigma4);	/* Core flux vs     sqrt(2) * core flux */
    classstats(g_core_flux, g_core4_flux, 0, 0.1, &g_fit5, &sigma5);	/* Core flux vs 2 * sqrt(2) * core flux */
    classstats(g_core_flux, g_peak_mag,   1, 0.2, &g_fit3, &sigma3);	/* Core flux vs Peak height */

    /* Faint end g_ellipticity */
    classstats_ellf(fluxlim);

    /* Work out position angle stats for likely stellar objects */
    classstats_pa();

    /* Get workspace for the boundary arrays */
    g_lower1 = cpl_malloc(NSAMPLE * sizeof(*g_lower1));
    g_lower2 = cpl_malloc(NSAMPLE * sizeof(*g_lower2));
    g_lower3 = cpl_malloc(NSAMPLE * sizeof(*g_lower3));
    g_upper1 = cpl_malloc(NSAMPLE * sizeof(*g_upper1));
    g_upper2 = cpl_malloc(NSAMPLE * sizeof(*g_upper2));
    g_upper3 = cpl_malloc(NSAMPLE * sizeof(*g_upper3));

    /* Boundaries: (Core vs sqrt(2) * Core) or (Core vs           0.5 * Core) */
    boundaries(g_core_flux, g_core1_flux, g_core2_flux, g_fit1,
    		   sigma1, g_fit4, sigma4, 1,
			   CPL_MATH_PI * g_rcore * g_rcore, 2. * CPL_MATH_PI * g_rcore * g_rcore,
			   &g_wt1, &g_avsig1, g_lower1, g_upper1);

    /* Boundaries: (Core vs       2 * Core) or (Core vs   2 * sqrt(2) * Core) */
    boundaries(g_core_flux, g_core3_flux, g_core4_flux, g_fit2,
    		   sigma2, g_fit5, sigma5, 0,
			   4. * CPL_MATH_PI * g_rcore * g_rcore, 8. * CPL_MATH_PI * g_rcore * g_rcore,
			   &g_wt2, &g_avsig2, g_lower2, g_upper2);

    /* Boundaries: (Core vs peak height) */
    boundpk(g_core_flux, g_peak_mag, g_fit3, sigma3, &g_wt3, &g_avsig3, g_lower3, g_upper3);

    /* Do final classification statistics and find the saturation limit */
    classstats_final();

    /* Define final boundaries: The term pow(10.0,(0.4*fluxlim+1.5)); comes from the magitude formula */
    double *lower = cpl_malloc(NSAMPLE * sizeof(double));
    double *upper = cpl_malloc(NSAMPLE * sizeof(double));
    g_uppere      = cpl_malloc(NSAMPLE * sizeof(*g_uppere));

    double xnoise = sqrt(CPL_MATH_PI * g_rcore * g_rcore) * g_skynoise;

    double ratell;
    ratell = xnoise / pow(10., 0.4 * (fluxlim + 1.5));
    ratell = 2.5    * log10(CPL_MAX(1. + ratell, 0.));

    double ratscl;
    ratscl = (pow((g_fitellf + 2. * g_sigellf - g_fitell), 2.) - 4. * g_sigell * g_sigell) / (4. * ratell * ratell);
    ratscl = CPL_MAX(0.25, CPL_MIN(10., ratscl));

    for (cpl_size i = 0; i < NSAMPLE; i++) {

        double xmag  = 5. + 0.1 * (double)(i + 1);
        double xflux = pow(10., 0.4 * xmag);
        double ratio = 2.5 * log10(1. + xnoise / xflux);

        /* TODO: Check if it's correct the assign to upper[i] --> exist a factor multiply by zero */
        lower[i] = g_fit_final - 5. * sqrt(     g_sigma_final * g_sigma_final +      ratio * ratio);
        upper[i] = g_fit_final +      sqrt(9. * g_sigma_final * g_sigma_final + 0. * ratio * ratio);

        g_uppere[i] = g_fitell + 2. * sqrt(g_sigell * g_sigell + ratscl * ratio * ratio);
        g_uppere[i] = CPL_MIN(0.5, g_uppere[i]);
    }

    g_elllim = CPL_MIN(0.5, CPL_MAX(0.2, g_fitell + 2. * g_sigell));
    fluxlim  = 2.5 * log10((double)(2.5 * sqrt(CPL_MATH_PI * g_rcore * g_rcore) * g_skynoise));

    g_nstar = 0;
    g_ngal  = 0;
    g_njunk = 0;
    g_ncmp  = 0;

    for (cpl_size i = 0; i < g_nrows; i++) {

        double ell  = g_ellipticity[i];
        double core = g_core_flux[i];
        double pkht = g_peak_mag[i];

    	double aux1 = 10. * (core - 5.);

    	cpl_size iarg = CPL_MAX(1, CPL_MIN(NSAMPLE, (cpl_size)(aux1 + (aux1 < 0. ? -0.5 : 0.5)))) - 1;

        double sig1;
        double sig2;
        if (! g_poor) {
            sig1    = CPL_MAX(0.01, (g_fit1 - g_lower1[iarg]) / 3.);
            sig2    = CPL_MAX(0.01, (g_fit2 - g_lower2[iarg]) / 3.);
        } else {
            sig1    = CPL_MAX(0.01, (g_fit4 - g_lower1[iarg]) / 3.);
            sig2    = CPL_MAX(0.01, (g_fit5 - g_lower2[iarg]) / 3.);
        }
        double sig3 = CPL_MAX(0.01, (g_fit3 - g_lower3[iarg]) / 3.);

        double denom = (g_wt1 / sig1 + g_wt2 / sig2 + g_wt3 / sig3);

        double w1 = (g_wt1 / sig1) / denom;
        double w2 = (g_wt2 / sig2) / denom;
        double w3 = (g_wt3 / sig3) / denom;

        double statistic;
        if (! g_poor) {

        	double core_small = g_core1_flux[i];
        	double core_large = g_core3_flux[i];

            statistic =   (        core - core_small - g_fit1             ) * w1
                        + (CPL_MAX(core_large - core - g_fit2, -3. * sig2)) * w2
						+ (        core       - pkht - g_fit3             ) * w3;
        } else {

        	double core_midd  = g_core2_flux[i];
        	double core_large = g_core4_flux[i];

            statistic =   (        core_midd  - core - g_fit4             ) * w1
                        + (CPL_MAX(core_large - core - g_fit5, -3. * sig2)) * w2
                        + (        core       - pkht - g_fit3             ) * w3;
        }

        g_cls[i] = -1.;
        double aux2    = exp(CPL_MAX(0., core - g_corlim + 1.));
        double statcut = upper[iarg] + 3. * g_sigma_final * (aux2 - 1.);
        if (statistic >= statcut) {
            g_cls[i] = 1.;
        } else if (statistic <= lower[iarg]) {
            g_cls[i] = 0.;
        }

        /* Save distance from the stellar locus */
        g_sig[i] = (statistic - g_fit_final) / ((g_fit_final - lower[iarg]) / 5.);

        /* Right, now here are lots of overrides for special circumstances */

        /* Too spikey? -> junk */
        if (core - pkht - g_fit3 < -4. * sig3) g_cls[i] = 0.;

        /* Elliptical star? -> compact */
        double ellbound = CPL_MAX(g_elllim, g_uppere[iarg]);
        if (ell > ellbound && g_cls[i] == -1. && core < g_flim && g_sig[i] > -2.) g_cls[i] = -2.;

        /* Saturated? -> star */
        if (core > g_corlim && statistic >= lower[iarg]) g_cls[i] = -1.;

        /* Too elliptical? -> junk */
        if (ell > 0.9 && core < g_corlim) g_cls[i] = 0.;

        /* Too faint? -> junk */
        if (core < fluxlim) g_cls[i] = 0.;


        /* Now count how many you have of each */
        if (g_cls[i] == -1.) {
            g_nstar++;
        } else if (g_cls[i] == 1.) {
            g_ngal++;
        } else if (g_cls[i] == -2.) {
            g_ncmp++;
        } else {
            g_njunk++;
        }
    }
    cpl_free(lower);
    cpl_free(upper);

    /* Do stats to get the aperture corrections */
	classstats_ap67(g_core5_flux, g_core3_flux, &g_fit6, &sigma6);
	classstats_ap67(g_core_flux,  g_core6_flux, &g_fit7, &sigma7);
	g_fit6 += g_fit2;

    double fit0;
    double sigma0;
    classstats_ap0(&fit0, &sigma0);
    fit0 = CPL_MAX(g_fit6, fit0);

    /* pkht */
    g_apcpkht = fit0 + g_fit3;

	g_apcor1 = fit0 + g_fit1;  /*         0.5 * core */
	g_apcor2 = fit0 + g_fit7;  /* 1 / sqrt(2) * core */
	g_apcor3 = fit0;           /*               core */
	g_apcor4 = fit0 - g_fit4;  /*     sqrt(2) * core */
	g_apcor5 = fit0 - g_fit2;  /*           2 * core */
	g_apcor6 = fit0 - g_fit5;  /* 2 * sqrt(2) * core */
	g_apcor7 = fit0 - g_fit6;  /*           4 * core */

    /* Now do a better job on the saturation */
    double *work = cpl_malloc(g_nrows * sizeof(double));

    cpl_size ii = 0;
    for (cpl_size i = 0; i < g_nrows; i++) {

        double ell  = g_ellipticity[i];
        double core = g_core_flux[i];
        double pkht = CPL_MAX(g_thresh, g_peak_height[i]) + g_skylev[i];

        if (   (   (   ell            <  g_elllim
        		    && core           >  g_flim
        		    && g_cls[i]      == -1
				    && g_sig[i]      >=  5.
				    && g_areal[0][i] >=  g_pixlim
				   )
        	    || pkht >= 0.9 * g_avsat
			   )
        	&& g_xpos[i] >= g_xmin
			&& g_xpos[i] <= g_xmax
			&& g_ypos[i] >= g_ymin
			&& g_ypos[i] <= g_ymax
		){
            work[ii++] = pkht;
        }
    }

    double avsatnew;
    if (ii > 0) {
    	double junk;
        medstat(work,ii,&avsatnew,&junk);
        avsatnew = CPL_MAX(10000.0+g_skylevel,avsatnew);
    } else {
        avsatnew = 10000.0 + g_skylevel;
    }

    g_avsat = avsatnew;

    /* Clean up */
    cpl_free(work);

	if (g_lower1 != NULL) {cpl_free(g_lower1); g_lower1 = NULL;}
	if (g_lower2 != NULL) {cpl_free(g_lower2); g_lower2 = NULL;}
	if (g_lower3 != NULL) {cpl_free(g_lower3); g_lower3 = NULL;}

	if (g_upper1 != NULL) {cpl_free(g_upper1); g_upper1 = NULL;}
	if (g_upper2 != NULL) {cpl_free(g_upper2); g_upper2 = NULL;}
	if (g_upper3 != NULL) {cpl_free(g_upper3); g_upper3 = NULL;}

	if (g_uppere != NULL) {cpl_free(g_uppere); g_uppere = NULL;}
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Work out the median difference between two magnitude estimates
 *
 * @param    core1      The flux array for the first aperture (in magnitudes)
 * @param    core2      The flux array for the second aperture (in magnitudes)
 * @param    small      Set if the second aperture is smaller than the first
 * @param    cutlev     An upper limit to the allowed value of the magnitude difference
 * @param    medval     The output median magnitude difference
 * @param    sigma      The output sigma in magnitude difference
 *
 * Description:
 * 	    The difference between the magnitudes of two different apertures
 *      is calculated for all objects in a list.
 */
/* ---------------------------------------------------------------------------*/
static void classstats(double *core1, double *core2, cpl_size small, double cutlev,
                       double *medval, double *sigma) {

    /* Initialise the output values to something stupid */
    *medval = 0.0;
    *sigma  = 1.0e6;

    double amult = (small == 1 ? -1. : 1.);

    /* Get some workspace */
    double *work = cpl_malloc(g_nrows * sizeof(double));
    double *dc   = cpl_malloc(g_nrows * sizeof(double));

    /* Work out differences */
    for (cpl_size i = 0; i < g_nrows; i++) {
        dc[i] = amult * (core2[i] - core1[i]);
    }

    /* Do an iteration loop */
    for (cpl_size iloop = 0; iloop < MAXLOOP; iloop++) {

        double sigmaold = *sigma;

        /* Ok, gather up all the stats */
        cpl_size n = 0;
        for (cpl_size i = 0; i < g_nrows; i++) {
            
            /* Clipping criteria */
            if (   g_ellipticity[i] < g_elllim
            	&& core1[i] < g_blim
				&& core1[i] > g_flim
				&& fabs(dc[i] - *medval) < 3.*(*sigma)
				&& g_xpos[i] >= g_xmin
				&& g_xpos[i] <= g_xmax
				&& g_ypos[i] >= g_ymin
				&& g_ypos[i] <= g_ymax
				&& g_areal[0][i] >= g_pixlim)
            {
                if (iloop > 0 || (iloop == 0 && dc[i] >= cutlev)) {
                    work[n++] = dc[i];
                }
            }
        }

        /* Sort the work array and find the median and sigma */
        if (n > 0) {

       	    sort_array(work, n, sizeof(*work), HDRL_SORT_DOUBLE, CPL_SORT_ASCENDING);

            if (iloop == 0) {

                anhist( work, n, medval, sigma);

            } else {

                medstat(work, n, medval, sigma);

                *sigma = CPL_MIN(sigmaold, *sigma);
            }

        } else {

            *medval = 0.;
            *sigma  = 0.01;
        }

        /* Just in case ... */
        *sigma = CPL_MAX(*sigma, 0.01);
    }

    /* Clean up */
    cpl_free(work);
    cpl_free(dc);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Work out the median ellipticity of the sample
 *
 * Description:
 * 	    The median ellipticity of the sample is calculated interatively
 */
/* ---------------------------------------------------------------------------*/
static void classstats_el(void)
{
    /* Initialise the mean and sigma to something stupid */
    g_sigell = 1.e6;
    g_fitell = 0.;

    /* Get some workspace */
    double *work = cpl_malloc(g_nrows * sizeof(double));

    /* Do iteration loop */
    for (cpl_size iloop = 0; iloop < MAXLOOP; iloop++) {

    	cpl_size n = 0;
        for (cpl_size i = 0; i < g_nrows; i++) {
            if (   g_ellipticity[i] < 0.5
            	&& g_core_flux[i] < g_blim
				&& g_core_flux[i] > g_flim
				&& fabs(g_ellipticity[i] - g_fitell) < 2. * g_sigell
				&& g_xpos[i] >= g_xmin
				&& g_xpos[i] <= g_xmax
				&& g_ypos[i] >= g_ymin
				&& g_ypos[i] <= g_ymax
				&& g_areal[0][i] >= g_pixlim)
            {
                work[n++] = g_ellipticity[i];
            }
        }

        if (n > 2) {
            medstat(work, n, &g_fitell, &g_sigell);
        } else {
            g_fitell = 0.25;
            g_sigell = 0.05;
        }
    }

    g_elllim = CPL_MIN(0.5, CPL_MAX(0.2, g_fitell + 2. * g_sigell));

    /* Clean up */
    cpl_free(work);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Work out the median position angle of the sample
 *
 * Description:
 * 	    The median position angle of the sample is calculated interactively
 */
/* ---------------------------------------------------------------------------*/
static void classstats_pa(void)
{
    /* Initialise the mean and sigma to something stupid */
    g_sigpa = 1.e6;
    g_fitpa = 0.;

    /* Get some workspace */
    double *work = cpl_malloc(g_nrows * sizeof(double));

    /* Do iteration loop */
    for (cpl_size iloop = 0; iloop < MAXLOOP; iloop++) {

    	cpl_size n = 0;
        for (cpl_size i = 0; i < g_nrows; i++) {

            if (   g_core_flux[i] < g_blim
            	&& g_core_flux[i] > g_flim
				&& fabs(g_pa[i] - g_fitpa) < 2. * g_sigpa
				&& g_xpos[i] >= g_xmin
				&& g_xpos[i] <= g_xmax
				&& g_ypos[i] >= g_ymin
				&& g_ypos[i] <= g_ymax
				&& g_areal[0][i] >= g_pixlim)
            {
                work[n++] = g_pa[i];
            }
        }

        if (n > 2) {
            medstat(work, n, &g_fitpa, &g_sigpa);
        } else {
            g_fitpa = 0.;
            g_sigpa = 0.05;
        }
    }

    /* Get out of here */
    cpl_free(work);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Work out the median ellipticity for faint objects
 *
 * @param    fluxlim    The flux limit (in magnitudes)
 *
 * Description:
 * 	    The median ellipticity is calculated for objects that are fainter
 *      than a given flux limit.
 */
/* ---------------------------------------------------------------------------*/
static void classstats_ellf(double fluxlim)
{
    /* Initialise the mean and sigma to something stupid */
    g_sigellf = 1.e6;
    g_fitellf = 0.;

    /* Get some workspace */
    double *work = cpl_malloc(g_nrows * sizeof(double));

    /* Do iteration loop */
    for (cpl_size iloop = 0; iloop < MAXLOOP; iloop++) {

    	cpl_size n = 0;
        for (cpl_size i = 0; i < g_nrows; i++) {

            if (   g_ellipticity[i] < 0.75
            	&& g_core_flux[i] > fluxlim + 1.
				&& g_core_flux[i] < fluxlim + 2.
				&& fabs(g_ellipticity[i] - g_fitellf) < 2. * g_sigellf)
            {
                work[n++] = g_ellipticity[i];
            }
        }

        if (n > 2) {
            medstat(work, n, &g_fitellf, &g_sigellf);
        } else {
            g_fitellf = 0.25;
            g_sigellf = 0.05;
        }
    }

    /* Get out of here */
    cpl_free(work);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Work out the median magnitude difference for large apertures
 *
 * @param    medval     The output median magnitude difference
 * @param    sigma      The output sigma of the magnitude difference
 *
 * Description:
 * 	    The median magnitude difference for isophotal fluxes and a large
 *      aperture is calculated.
 */
/* ---------------------------------------------------------------------------*/
static void classstats_ap0(double *medval, double *sigma)
{
    /* Initialise the output values to something stupid */
    *medval  = 0.;
    *sigma   = 1.e6;
    g_elllim = CPL_MIN(0.5, CPL_MAX(0.2, g_fitell + 2. * g_sigell));

    /* Get some workspace */
    double *work = cpl_malloc(g_nrows * sizeof(double));
    double *dc   = cpl_malloc(g_nrows * sizeof(double));

    /* Work out differences */
    for (cpl_size i = 0; i < g_nrows; i++) {
        dc[i] = CPL_MAX(0., CPL_MAX(g_iso_flux[i], g_core5_flux[i])) - g_core_flux[i];
    }

    /* Do an iteration loop */
    for (cpl_size iloop = 0; iloop < MAXLOOP; iloop++) {

        /* Ok, gather up all the stats */
    	cpl_size n = 0;
        for (cpl_size i = 0; i < g_nrows; i++) {
            
            /* Clipping criteria */
            if (   g_ellipticity[i] < g_elllim
            	&& g_core_flux[i] < g_blim
				&& g_core_flux[i] > g_flim
				&& fabs(dc[i] - *medval) < 3. * (*sigma)
				&& g_cls[i] == -1.
				&& g_sig[i] < 5.
				&& g_xpos[i] >= g_xmin
				&& g_xpos[i] <= g_xmax
				&& g_ypos[i] >= g_ymin
				&& g_ypos[i] <= g_ymax
				&& g_areal[0][i] >= g_pixlim)
            {
                if (iloop > 0 || (iloop == 0 && dc[i] >= 0.)) {
                    work[n++] = dc[i];
                }
            }
        }

        /* Sort the work array and find the median and sigma */
        if (n > 0) {

            sort_array(work, n, sizeof(*work), HDRL_SORT_DOUBLE, CPL_SORT_ASCENDING);

            if (iloop == 0) {

                anhist(work, n, medval, sigma);

                *sigma = CPL_MATH_STD_MAD * (*medval - work[(cpl_size)(0.25 * (double)(n + 3)) - 1]);
                *sigma = CPL_MAX(0.025, *sigma);

            } else {

                double sigmanew;
                medstat(work, n, medval, &sigmanew);

                *sigma = CPL_MIN(*sigma, sigmanew);
                *sigma = CPL_MAX(0.01,   *sigma);
            }

        } else {

            *medval = 0.;
            *sigma  = 0.01;
        }

        /* Just in case ... */
        *sigma = CPL_MAX(*sigma, 0.01);
    }

    /* Clean up */
    cpl_free(work);
    cpl_free(dc);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief
 *
 * @param    mag1
 * @param    mag2
 * @param    medval     The output median magnitude difference
 * @param    sigma      The output sigma of the magnitude difference
 *
 * Description:
 *
 */
/* ---------------------------------------------------------------------------*/
static void classstats_ap67(double *mag1, double *mag2, double *medval, double *sigma)
{
    /* Initialise the output values to something stupid */
    *medval  = 0.;
    *sigma   = 1.e6;
    g_elllim = CPL_MIN(0.5, CPL_MAX(0.2, g_fitell + 2. * g_sigell));

    /* Get some workspace */
    double *work = cpl_malloc(g_nrows * sizeof(double));
    double *dc   = cpl_malloc(g_nrows * sizeof(double));

    /* Work out differences */
    for (cpl_size i = 0; i < g_nrows; i++) {
        dc[i] = mag1[i] - mag2[i];
    }

    /* Do an iteration loop */
    for (cpl_size iloop = 0; iloop < MAXLOOP; iloop++) {

        /* Ok, gather up all the stats */
    	cpl_size n = 0;
        for (cpl_size i = 0; i < g_nrows; i++) {
            
            /* Clipping criteria */
            if (   g_ellipticity[i] < g_elllim
            	&& g_core_flux[i] < g_blim
				&& g_core_flux[i] > g_flim
				&& fabs(dc[i] - *medval) < 3. * (*sigma)
				&& g_cls[i] == -1. && g_sig[i] < 5.
				&& g_xpos[i] >= g_xmin
				&& g_xpos[i] <= g_xmax
				&& g_ypos[i] >= g_ymin
				&& g_ypos[i] <= g_ymax
				&& g_areal[0][i] >= g_pixlim)
            {
                if (iloop > 0 || (iloop == 0 && dc[i] >= 0.)) {
                    work[n++] = dc[i];
                }
            }
        }

        /* Sort the work array and find the median and sigma */
        if (n > 0) {

            sort_array(work, n, sizeof(*work), HDRL_SORT_DOUBLE, CPL_SORT_ASCENDING);

            if (iloop == 0) {

                anhist(work, n, medval, sigma);

                *sigma = CPL_MATH_STD_MAD * (*medval - work[(cpl_size)(0.25 * (double)(n + 3)) - 1]);
                *sigma = CPL_MAX(0.025, *sigma);

            } else {

            	double sigmanew;
                medstat(work, n, medval, &sigmanew);

                *sigma = CPL_MIN(*sigma, sigmanew);
                *sigma = CPL_MAX(0.01,   *sigma);
            }

        } else {
            *medval = 0.;
            *sigma  = 0.01;
        }

        /* Just in case ... */
        *sigma = CPL_MAX(*sigma, 0.01);
    }

    /* Clean up */
    cpl_free(work);
    cpl_free(dc);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Routine to define the median classification statistic
 *
 * Description:
 * 	    The result of the previous boundary and statistical routines are
 *      combined to define the median classification statistic and its
 *      sigma. The curve is also investigated to see where saturation occurs
 */
/* ---------------------------------------------------------------------------*/
static void classstats_final(void)
{
    /* Initialise */
    g_sigma_final = 1.e6;
    g_fit_final   = 0.;

    cpl_size ncls = 0;

    /* Get some workspace */
    double *work      = cpl_malloc(g_nrows * sizeof(double));
    double *work1     = cpl_malloc(g_nrows * sizeof(double));
    double *statistic = cpl_malloc(g_nrows * sizeof(double));

    /* Calculate the statistic now */
    for (cpl_size i = 0; i < g_nrows; i++) {

        double pkht = g_peak_mag[i];
        double core = g_core_flux[i];

    	double aux = 10. * (core - 5.);
    	cpl_size iarg = CPL_MAX(1, CPL_MIN(NSAMPLE, (cpl_size)(aux + (aux < 0. ? -0.5 : 0.5)))) - 1;

        double sig1;
        double sig2;
        if (!g_poor) {
            sig1    = CPL_MAX(0.01, (g_fit1 - g_lower1[iarg]) / 3.);
            sig2    = CPL_MAX(0.01, (g_fit2 - g_lower2[iarg]) / 3.);
        } else {
            sig1    = CPL_MAX(0.01, (g_fit4 - g_lower1[iarg]) / 3.);
            sig2    = CPL_MAX(0.01, (g_fit5 - g_lower2[iarg]) / 3.);
        }
        double sig3 = CPL_MAX(0.01, (g_fit3 - g_lower3[iarg]) / 3.);

        double denom = (g_wt1 / sig1 + g_wt2 / sig2 + g_wt3 / sig3);

        double w1 = (g_wt1 / sig1) / denom;
        double w2 = (g_wt2 / sig2) / denom;
        double w3 = (g_wt3 / sig3) / denom;

        if (! g_poor) {

            double core_small = g_core1_flux[i];
            double core_large = g_core3_flux[i];

            statistic[i] =   (core - core_small - g_fit1) * w1
            		       + (core_large - core - g_fit2) * w2
						   + (core - pkht       - g_fit3) * w3;

        } else {

            double core_midd  = g_core2_flux[i];
            double core_large = g_core4_flux[i];

            statistic[i] =   (core_midd  - core - g_fit4) * w1
            		       + (core_large - core - g_fit5) * w2
						   + (core       - pkht - g_fit3) * w3;
        }
    }

    /* Iteration loop.  Use only lower g_ellipticity images and relevant peak height range */
    for (cpl_size iloop = 0; iloop < MAXLOOP; iloop++) {

        double sigmaold = g_sigma_final;

        cpl_size n = 0;
        for (cpl_size i = 0; i < g_nrows ; i++) {

            double ell  = g_ellipticity[i];
            double core = g_core_flux[i];

            if (   ell < g_elllim
            	&& core < g_blim
				&& core > g_flim
				&& fabs((double)(statistic[i] - g_fit_final)) < 3. * g_sigma_final
				&& g_areal[0][i] >= g_pixlim)
            {
                work[n++] = statistic[i];
            }

            /* This information is to be used later to find the curvature of saturated region */
            if (core > g_corlim && iloop == MAXLOOP - 2) {
                g_cls[ncls]   = statistic[i];
                g_sig[ncls++] = core;
            }
        }

        /* Median defines general fit */
        if (n > 2) {

            sort_array(work, n, sizeof(*work), HDRL_SORT_DOUBLE, CPL_SORT_ASCENDING);

            if (iloop == 0 && n > 10) {
                anhist( work, n, &g_fit_final, &g_sigma_final);
            } else {
                medstat(work, n, &g_fit_final, &g_sigma_final);
            }

            g_sigma_final = CPL_MAX(0.01, CPL_MIN(sigmaold, g_sigma_final));

        } else {

            g_fit_final   = 0.;
            g_sigma_final = 0.01;
        }
    }

    /* Now work out the curvature in the saturated region */
    sort_array_index(g_sig, ncls, g_cls, HDRL_SORT_DOUBLE, CPL_SORT_ASCENDING);

    cpl_size ii    =  0;
    cpl_size idx   = -1;
    cpl_size iend  =  0;

    double xcor    =  12.5;
    double corlim1 =  0.;
    double corlim2 =  0.;
    double corval1 =  0.;
    double corval2 =  0.;

    while (iend == 0 && idx < ncls-1) {

    	idx++;
        if (g_sig[idx] > xcor+0.25 && ii >= 3) {

            double cfit;
            double csig;
            medstat(work, ii, &cfit, &csig);

            for (cpl_size iloop = 0; iloop < 3; iloop++) {

                cpl_size kk = 0;
                for (cpl_size k = 0; k < ii; k++) {
                    if (work[k] <= cfit + 3. * csig)
                        work1[kk++] = work[k];
                }

                double junk;
                medstat(work1, kk, &cfit, &junk);
            }

            if (cfit <= g_fit_final + 3. * g_sigma_final) {

                corlim1 = xcor;
                corval1 = cfit;

            } else {

                corlim2 = xcor;
                corval2 = cfit;

                iend = 1;
            }

        } else {

            work[ii++] = g_cls[idx];
        }
    }

    /* Estimate where core measure and statistic become unreliable */
    if (iend == 1) {
        g_corlim = corlim2 - 0.5 * (corval2 - g_fit_final - 3. * g_sigma_final) / (corval2 - corval1);
    } else {
        g_corlim = corlim1;
    }
    g_corlim = CPL_MAX(g_cormin, g_corlim);

    cpl_size kk = 0;
    for (cpl_size i = 0; i < g_nrows; i++) {

        double core = g_core_flux[i];
        if (core >= g_corlim) {
            work[kk++] = g_peak_height[i] + g_skylevel;
        }
    }

    if (kk > 0) {

    	double junk;
        medstat(work, kk, &g_avsat, &junk);

        g_avsat = CPL_MAX(10000. + g_skylevel, g_avsat);

    } else {
        g_avsat = 10000. + g_skylevel;
    }

    /* Clean up */
    cpl_free(work);
    cpl_free(work1);
    cpl_free(statistic);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Work out the median and sigma of an array
 *
 * @param    array      The input data array
 * @param    n          The number of values in the data array
 * @param    medval     The output median value
 * @param    sigma      The output sigma value
 *
 * Description:
 * 	    The median of an array is calculated by sorting and choosing the
 *      middle value. The sigma is estimated by halving the space between
 *      the first and third quartiles and scaling by the appropriate factor
 */
/* ---------------------------------------------------------------------------*/
static void medstat(double *array, cpl_size n, double *medval, double *sigval)
{
    /* Sort the array first, then choose the median.  The sigma is defined
     *  as half the distance between the two quartile points multiplied by
     *  the appropriate scaling factor (1.48) */

    if (n == 0) {

        *medval = 0.;
        *sigval = 0.;

    } else {

        sort_array(array, n, sizeof(*array), HDRL_SORT_DOUBLE, CPL_SORT_ASCENDING);

		cpl_size lev1 = (    n + 1) / 2;
		cpl_size lev2 = (3 * n + 3) / 4;
		cpl_size lev3 = (    n + 3) / 4;

		*medval = array[lev1 - 1];
		*sigval = CPL_MATH_STD_MAD * 0.5 * (array[lev2 - 1] - array[lev3 - 1]);
    }
}
