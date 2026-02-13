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

#include "hdrl_cat_conf.h"

#include "hdrl_cat_apio.h"
#include "hdrl_cat_apline.h"
#include "hdrl_cat_table.h"
#include "hdrl_cat_background.h"
#include "hdrl_cat_terminate.h"


/*** Defines ***/
#define NW             5     /*  */
#define STUPID_VALUE  -1000  /* Minimum value of a pixel */


/*** Internal global variables **/

static double        *g_smoothed  = NULL;
static double        *g_smoothedc = NULL;
static unsigned char *g_mflag     = NULL;
static double        *g_indata    = NULL;
static double        *g_confdata  = NULL;
static double        *g_confsqrt  = NULL;

static ap_t          g_ap;
static cpl_size      g_freeconf   = 0;

static double        g_weights[NW*NW];
static cpl_size      g_nx;
static cpl_size      g_ny;


/*** Prototypes ***/

static void crweights(double    filtfwhm);
static void convolve( cpl_size  ir      );
static void clean_up( cpl_table *tab    );

/*---------------------------------------------------------------------------*/
/**
 * @defgroup Catalogue     Catalogue
 *
 * @brief    Main function of the catalogue
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Do source extraction
 *
 * @param    infile    The input image
 * @param    conf      The input confidence map
 * @param    ipix      The minimum allowable size of an object
 * @param    threshold The detection threshold in sigma above sky
 * @param    icrowd    If set then the deblending software will be used
 * @param    rcore     The core radius in pixels
 * @param    bkg_subtr The switch to do background subtraction
 * @param    nbsize    The smoothing box size for background map estimation
 * @param    cattype   The type of catalogue to be produced
 * @param    filtfwhm  The FWHM of the smoothing kernel in the detection algorithm
 * @param    gain      The header keyword with the gain in e-/ADU
 * @param    saturation The saturation level in ADU
 * @param    res        the result after source extraction
 *
 * @return   CPL_ERROR_NONE if everything is ok. The concrete error in other case.
 *
 * @note  QC headers:
 *      The following values will go into the image extension propertylist.
 *      - \b SATURATION:    Saturation level in ADU.
 *      - \b MEAN_SKY:      Mean sky brightness in ADU.
 *      - \b SKY_NOISE:     Pixel noise at sky level in ADU.
 * @note DRS headers:
 *      The following values will go into the image extension propertylist
 *      - \b SKYLEVEL:      Mean sky brightness in ADU
 *      - \b SKYNOISE:      Pixel noise at sky level in ADU
 *      The following values will go into the table extension propertylist
 *      - \b THRESHOL:      The detection threshold in ADU
 *      - \b MINPIX:        The minimum number of pixels per image
 *      - \b CROWDED:       Flag for crowded field analysis
 *      - \b RCORE:         The core radius for default profile fit in pixels
 *      - \b FILTFWHM:      The FWHM of the smoothing kernel in the detection algorithm
 *      - \b SEEING:        The average FWHM of stellar objects in pixels
 *      - \b XCOL:          The column containing the X position
 *      - \b YCOL:          The column containing the Y position
 *      - \b NXOUT:         The X dimension of the original image array
 *      - \b NYOUT:         The Y dimension of the original image array
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_catalogue_conf(
		hdrl_casu_fits *infile, hdrl_casu_fits *conf, cpl_size ipix,
		double threshold, cpl_size icrowd, double rcore,
		cpl_size bkg_subtr, cpl_size nbsize,
		cpl_size cattype, double filtfwhm, double gain,
		double saturation, hdrl_casu_result *res)
{
    /* Initialize output */
    res->catalogue = NULL;

    /* Useful constants */
    double   fconst   = CPL_MATH_LOG2E;
    cpl_size nobjects = 0;

    /* Open input image */
    cpl_table *tab = NULL;
    cpl_image *map = hdrl_casu_fits_get_image(infile);

    if ((g_indata = cpl_image_get_data_double(map)) == NULL) {
    	clean_up(tab);
    	cpl_error_set_message(cpl_func, CPL_ERROR_NULL_INPUT,
    			"hdrl_cat_catalogue_conf - Error getting image data");
    	return CPL_ERROR_NULL_INPUT;
    }

    g_nx = cpl_image_get_size_x(map);
    g_ny = cpl_image_get_size_y(map);

    cpl_size npts = g_nx * g_ny;
    cpl_size npix = g_nx * g_ny;

    /* Open the associated confidence map, if it exists */
    cpl_image *cmap;
    if (conf != NULL) {

        cmap = hdrl_casu_fits_get_image(conf);
        if ((g_confdata = cpl_image_get_data(cmap)) == NULL) {
        	clean_up(tab);
        	cpl_error_set_message(cpl_func, CPL_ERROR_NULL_INPUT,
        			"hdrl_cat_catalogue_conf - Error getting confidence map data");
        	return CPL_ERROR_NULL_INPUT;
        }

        cpl_size nxc = cpl_image_get_size_x(cmap);
        cpl_size nyc = cpl_image_get_size_y(cmap);
        if ((g_nx != nxc) || (g_ny != nyc)) {
        	clean_up(tab);
        	cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT,
        			"hdrl_cat_catalogue_conf - Input image and confidence dimensions don't match");
        	return CPL_ERROR_INCOMPATIBLE_INPUT;
        }

        g_freeconf = 0;
    } else {
        g_confdata = cpl_malloc(npts * sizeof(*g_confdata));
        for (cpl_size i = 0; i < npts; i++) {
            g_confdata[i] = 100;
        }
        g_freeconf = 1;
        cmap = NULL;
    }
    
    /* Get g_mflag array for flagging saturated pixels */
    g_mflag = cpl_calloc(npix, sizeof(*g_mflag));

    /* Open the g_ap structure and define some stuff in it */
    g_ap.lsiz     = g_nx;
    g_ap.csiz     = g_ny;
    g_ap.inframe  = map;
    g_ap.conframe = cmap;

    hdrl_apinit(&g_ap);

    g_ap.indata   = g_indata;
    g_ap.confdata = g_confdata;
    g_ap.multiply = 1;
    g_ap.ipnop    = ipix;
    g_ap.mflag    = g_mflag;
    g_ap.rcore    = rcore;
    g_ap.filtfwhm = filtfwhm;
    g_ap.icrowd   = icrowd;
    g_ap.fconst   = fconst;

    /* Open the output catalogue FITS table */
    cpl_size hdrl_xcol;
	cpl_size hdrl_ycol;
    hdrl_tabinit(&g_ap, &hdrl_xcol, &hdrl_ycol, cattype, &tab, res);

    /* Set up the data flags */
    for (cpl_size i = 0; i < npix; i++) {
        if (g_confdata[i] == 0) {
            g_mflag[i] = MF_ZEROCONF;
        } else if (g_indata[i] < STUPID_VALUE) {
            g_mflag[i] = MF_STUPID_VALUE;
        } else {
            g_mflag[i] = MF_CLEANPIX;
        }
    }

    /* Flag up regions where the value is above the saturation level*/
    for (cpl_size i = 0; i < npix ; i++) {
        if (g_mflag[i] == MF_CLEANPIX && g_indata[i] > saturation) {
            g_mflag[i] = MF_SATURATED;
        }
    }

    /* Compute the background variation and remove it from the data*/
    if (hdrl_background(&g_ap, nbsize, bkg_subtr, res) != CPL_ERROR_NONE) {
    	clean_up(tab);
    	return cpl_error_get_code();
    }

    /* Compute background statistics */
    double skymed;
    double skysig;
    if (hdrl_backstats(&g_ap, &skymed, &skysig) != CPL_ERROR_NONE) {
    	clean_up(tab);
    	return cpl_error_get_code();
    }

    /* Take mean sky level out of data. */
    if (bkg_subtr) {
        for (cpl_size i = 0; i < g_nx * g_ny; i++) {
            g_indata[i] -= skymed;
        }
    }

    /* Work out isophotal detection threshold levels */
    double thresh = threshold * skysig;
    if (!bkg_subtr && thresh < skymed) {
    	clean_up(tab);
    	cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT,
    			"Bad background corrected input. Background estimation disabled "
    			"but image median larger than threshold * sigma.");
    	return CPL_ERROR_INCOMPATIBLE_INPUT;
    }

    /* Minimum intensity for consideration */
    double xintmin = 1.5 * thresh * (double)ipix;

    /* Actual areal profile levels: T, 2T, 4T, 8T,...but written wrt T i.e. threshold as a power of 2 */
    double offset = log(thresh)*fconst;

    /* Get a bit of workspace for buffers */
    g_smoothed  = cpl_malloc(g_nx * sizeof(*g_smoothed));
    g_smoothedc = cpl_malloc(g_nx * sizeof(*g_smoothedc));

    /* Minimum size for considering multiple images */
    cpl_size mulpix = CPL_MAX(8, 2 * ipix);

    /* Define a few things more things in g_ap structure */
    g_ap.mulpix         = mulpix;
    g_ap.areal_offset   = offset;
    g_ap.thresh         = thresh;
    g_ap.xintmin        = xintmin;
    g_ap.sigma          = skysig;

    if (bkg_subtr) {
        g_ap.background = skymed;
        g_ap.saturation = saturation - skymed;
    } else {
        g_ap.background = 0.;
        g_ap.saturation = saturation;
    }

    /* Set the weights */
    crweights(filtfwhm);
    cpl_size nw2 = NW / 2;

    /* sqrt confdata, need as many rows as the convolution is wide */
    g_confsqrt = cpl_malloc(g_nx * NW * sizeof(*g_confsqrt));
    for (cpl_size j = 0; j < NW; j++) {
        for (cpl_size i = 0; i < g_nx; i++) {
            g_confsqrt[j * g_nx + i] = sqrt(0.01 * (double)(g_confdata[j * g_nx + i]));
        }
    }

    /* Right, now for the extraction loop.  Begin by defining a group of three rows of data and confidence */
    for (cpl_size j = nw2; j < g_ny-nw2; j++) {

    	double *current = g_indata + j * g_nx;

        if (j != nw2) {

            /* rotate buffer, could be a more efficient structure, but this sufficent for now */
            memmove(g_confsqrt, g_confsqrt + g_nx, g_nx * (NW - 1) * sizeof(*g_confsqrt));

            /* fill last row of buffer */
            for (cpl_size i = 0; i < g_nx; i++) {
                g_confsqrt[(NW - 1) * g_nx + i] = sqrt(0.01 * (double)(g_confdata[(j + nw2) * g_nx + i]));
            }
        }

        /* current row is the center of the buffer */
        double *currentc = g_confsqrt + nw2 * g_nx;
        convolve(j);
   
        /* Do the detection now */
        hdrl_apline(&g_ap, current, currentc, g_smoothed, g_smoothedc, j, NULL);

        /* Make sure we're not overruning the stacks */
        if (g_ap.ibstack > g_ap.maxbl - g_ap.lsiz) hdrl_apfu(&g_ap);
        if (g_ap.ipstack > g_ap.maxpa * 3 / 4    ) hdrl_apfu(&g_ap);

        /* See if there are any images to terminate */
        if (g_ap.ipstack > 1) {
            hdrl_terminate(&g_ap, gain, &nobjects, tab, res);
        }
    }

    /* Post process. First truncate the cpl_table to the correct size and then work out an estimate of the seeing */
    cpl_table_set_size(tab, nobjects);

    if (hdrl_do_seeing(&g_ap, nobjects, tab) != CPL_ERROR_NONE) {
    	clean_up(tab);
    	return cpl_error_get_code();
    }

    /* Create a property list with extra parameters. First QC parameters */
    cpl_propertylist *extra = cpl_propertylist_duplicate(hdrl_casu_fits_get_ehu(infile));


    /* QC parameters */
    cpl_propertylist_update_double(extra, "ESO QC SATURATION", g_ap.saturation);
    cpl_propertylist_update_double(extra, "ESO QC MEAN_SKY",   g_ap.background);
    cpl_propertylist_update_double(extra, "ESO QC SKY_NOISE",  g_ap.sigma);

    cpl_propertylist_set_comment(  extra, "ESO QC SATURATION", "[adu] Saturation level");
    cpl_propertylist_set_comment(  extra, "ESO QC MEAN_SKY",   "[adu] Median sky brightness");
    cpl_propertylist_set_comment(  extra, "ESO QC SKY_NOISE",  "[adu] Pixel noise at sky level");


    /* DRS parameters */
    cpl_propertylist_update_double(extra, "ESO DRS THRESHOL",  g_ap.thresh);
    cpl_propertylist_update_int(   extra, "ESO DRS MINPIX",    g_ap.ipnop);
    cpl_propertylist_update_int(   extra, "ESO DRS CROWDED",   g_ap.icrowd);
    cpl_propertylist_update_double(extra, "ESO DRS RCORE",     g_ap.rcore);
    cpl_propertylist_update_double(extra, "ESO DRS SEEING",    g_ap.fwhm);
    cpl_propertylist_update_double(extra, "ESO DRS FILTFWHM",  g_ap.filtfwhm);
    cpl_propertylist_update_int(   extra, "ESO DRS XCOL",      hdrl_xcol);
    cpl_propertylist_update_int(   extra, "ESO DRS YCOL",      hdrl_ycol);
    cpl_propertylist_update_int(   extra, "ESO DRS NXOUT",     g_nx);
    cpl_propertylist_update_int(   extra, "ESO DRS NYOUT",     g_ny);

    cpl_propertylist_set_comment(  extra, "ESO DRS THRESHOL",  "[adu] Isophotal analysis threshold");
    cpl_propertylist_set_comment(  extra, "ESO DRS MINPIX",    "[pixels] Minimum size for images");
    cpl_propertylist_set_comment(  extra, "ESO DRS CROWDED",   "Crowded field analysis flag");
    cpl_propertylist_set_comment(  extra, "ESO DRS RCORE",     "[pixels] Core radius for default profile fit");
    cpl_propertylist_set_comment(  extra, "ESO DRS SEEING",    "[pixels] Average FWHM");
    cpl_propertylist_set_comment(  extra, "ESO DRS FILTFWHM",  "[pixels] FWHM of smoothing kernel");
    cpl_propertylist_set_comment(  extra, "ESO DRS XCOL",      "Column for X position");
    cpl_propertylist_set_comment(  extra, "ESO DRS YCOL",      "Column for Y position");
    cpl_propertylist_set_comment(  extra, "ESO DRS NXOUT",     "X Dimension of input image");
    cpl_propertylist_set_comment(  extra, "ESO DRS NYOUT",     "Y Dimension of input image");

    res->catalogue = hdrl_casu_tfits_wrap(tab, extra);

    /* Clean up */
    clean_up(NULL);

    return CPL_ERROR_NONE;
}

/**@}*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Create convolution kernel g_weights
 *
 * @param    filtfwhm  The FWHM of the Gaussian kernel
 *
 * Description:
 * 	    The convolution kernel for a Gaussian with a given FWHM is created.
 */
/* ---------------------------------------------------------------------------*/
static void crweights(double filtfwhm)
{
    /* Get the kernel size */
    cpl_size nw2 = NW / 2;
    
    /* Set the normalisation constants: 2, 2.35 one should use CPL_MATH_FWHM_SIG */
    double gsigsq = 1. / (2. * pow(CPL_MAX(1., (double)filtfwhm) / 2.35, 2.));
    double renorm = 0.;

    /* Now work out the weights */
    cpl_size n = -1;
    for (cpl_size i = -nw2; i <= nw2; i++) {

        double di = (double)i;
        di *= gsigsq * di;

        for (cpl_size j = -nw2; j <= nw2; j++) {

            double dj = (double)j;
            dj *= gsigsq * dj;

            n++;

            g_weights[n] = exp(-(di + dj));

            renorm += g_weights[n];
        }
    }

    /* Now normalise the weights */
    cpl_size nn = -1;
    for (cpl_size i = -nw2; i <= nw2; i++) {
        for (cpl_size j = -nw2; j <= nw2; j++) {

            nn++;

            g_weights[nn] /= renorm;
        }
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Smooth the original data and confidence
 *
 * @param    ir         The row number of the input image to smooth
 *
 * Description:
 * 	    Smooth a line of original data and confidence by convolving with a Gaussian kernel.
 */
/* ---------------------------------------------------------------------------*/
static void convolve(cpl_size ir)
{
    /* Zero the summations */
    for (cpl_size i = 0; i < g_nx; i++) {

        g_smoothed[i]  = 0.;
        g_smoothedc[i] = 0.;
    }

    /* Now big is the smoothing kernel? */
    cpl_size nw2 = NW / 2;

    /* Now loop for each column */
    for (cpl_size ix = nw2; ix < g_nx - nw2; ix++) {


    	double sum  = 0.;
    	double sumc = 0.;

    	cpl_size n = -1;
        for (cpl_size jy = ir - nw2; jy <= ir + nw2; jy++) {

            /* g_confsqrt [0..NW] rows */
        	double *cdata = g_confsqrt + (jy - ir + nw2) * g_nx;
        	double *idata = g_indata   +  jy             * g_nx;

            for (cpl_size jx = ix - nw2; jx <= ix + nw2; jx++) {

            	n++;

            	sum  += g_weights[n] * idata[jx];
                sumc += g_weights[n] * idata[jx] * cdata[jx];
            }
        }

        g_smoothed[ix]  = sum;
        g_smoothedc[ix] = sumc;
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Clean up the global variables
 *
 * Description:
 *
 */
/* ---------------------------------------------------------------------------*/
static void clean_up(cpl_table *tab) {

	if (tab) cpl_table_delete(tab);

    if (g_freeconf) {
        if (g_confdata) {cpl_free(g_confdata); g_confdata = NULL;}
    }

    if (g_confsqrt ) {cpl_free(g_confsqrt);  g_confsqrt  = NULL;}
    if (g_smoothed ) {cpl_free(g_smoothed);  g_smoothed  = NULL;}
    if (g_smoothedc) {cpl_free(g_smoothedc); g_smoothedc = NULL;}
    if (g_mflag    ) {cpl_free(g_mflag);     g_mflag     = NULL;}

    hdrl_apclose(&g_ap);
}
