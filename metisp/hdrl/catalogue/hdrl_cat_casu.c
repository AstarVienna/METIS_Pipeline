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

#include "hdrl_cat_conf.h"

#include "hdrl_cat_classify.h"


/*** Prototypes ***/

static void hdrl_casu_xytoradec(const cpl_wcs *wcs, double x, double y, double *ra, double *dec);


/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_casu  hdrl_casu
 * @ingroup  Catalogue
 *
 * @brief    Generic functions
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Wrap an table in a hdrl_casu_tfits wrapper
 *
 * @param    tab       The input cpl_table
 * @param    ehu       The input propertylist for the extension header for the new object.
 *
 * @return   The new hdrl_casu_tfits structure.
 *
 * Description:
 * 	    The input table is inserted into a hdrl_casu_tfits wrapper. A model
 *      hdrl_casu_tfits object may be provided to give the new object
 *      headers. If the ehu parameters are not null then they will
 *      be used as the propertylists for the new object. If not, then
 *      an attempt will be made to copy the propertylists from the model.
 */
/* ---------------------------------------------------------------------------*/
hdrl_casu_tfits * hdrl_casu_tfits_wrap(cpl_table *tab, cpl_propertylist *ehu)
{
    /* Check for nonsense input */
    if (tab == NULL) {
        return NULL;
    }

    /* Get the hdrl_casu_tfits structure */
    hdrl_casu_tfits *p = cpl_malloc(sizeof(hdrl_casu_tfits));

    /* Load stuff in */
    p->table  = tab;

    if (ehu != NULL) {
        p->ehu = ehu;
    } else {
        p->ehu = cpl_propertylist_new();
    }

    return p;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Get the propertylist for the extension header for a given
 *             hdrl_casu_tfits image.
 *
 * @param    p         The input hdrl_casu_tfits object
 *
 * @return   The propertylist represeting the extension header of the input table
 *             (NULL if there is an error).
 *
 * Description:
 * 	    Get the propertylist for the extension header for a given hdrl_casu_tfits
 *      image. This is the extension that is relevant of the image.
 *      This should only need to be read once and then can be used to add
 *      things to the primary header.
 */
/* ---------------------------------------------------------------------------*/
cpl_propertylist * hdrl_casu_tfits_get_ehu(hdrl_casu_tfits *p)
{
    /* Check for nonsense input */
    if (p == NULL) {
        return NULL;
    }

    return p->ehu;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Get the CPL table from the hdrl_casu_tfits object
 *
 * @param    p         The input hdrl_casu_tfits object
 *
 * @return   The cpl_image object. NULL if there was an error.
 *
 * Description:
 * 	    Return the CPL table from the input hdrl_casu_tfits object. This table is
 *      suitable for use in all cpl_table routines.
 */
/* ---------------------------------------------------------------------------*/
cpl_table * hdrl_casu_tfits_get_table(hdrl_casu_tfits *p)
{
    /* Check for nonsense input */
    if (p == NULL) {
        return NULL;
    }

    return p->table;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Free all the workspace associated with a hdrl_casu_fits object
 *
 * @param    p         The input hdrl_casu_tfits object
 *
 */
/* ---------------------------------------------------------------------------*/
void hdrl_casu_tfits_delete(hdrl_casu_tfits *p)
{
    /* Check for nonsense input */
    if (p != NULL) {

		/* Free up workspace if it's been used */
    	if (p->table   ) cpl_table_delete(p->table);
		if (p->ehu     ) cpl_propertylist_delete(p->ehu);

		cpl_free(p);
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Wrap an image in a hdrl_casu_fits wrapper
 *
 * @param    im        The input cpl_image
 *
 * @return   The new hdrl_casu_tfits structure.
 *
 * Description:
 * 	    The input image is inserted into a hdrl_casu_fits wrapper. A model
 *      hdrl_casu_fits object may be provided to give the new object header.
 */
/* ---------------------------------------------------------------------------*/
hdrl_casu_fits * hdrl_casu_fits_wrap(cpl_image *im)
{
    /* Check for nonsense input */
    if (im == NULL) {
        return NULL;
    }

    /* Get the hdrl_casu_fits structure */
    hdrl_casu_fits *p = cpl_malloc(sizeof(hdrl_casu_fits));

    /* Load stuff in */
    p->image = im;
    p->ehu   = cpl_propertylist_new();

    return p;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Copy a hdrl_casu_fits structure into another one.
 *
 * @param    in        The input hdrl_casu_fits object
 *
 * @return   The output hdrl_casu_fits object.
 *
 * Description:
 * 	    An input hdrl_casu_fits structure is duplcated and returned
 */
/* ---------------------------------------------------------------------------*/
hdrl_casu_fits * hdrl_casu_fits_duplicate(hdrl_casu_fits *in)
{
    /* Check for nonsense input */
    if (in == NULL) {
        return NULL;
    }

    /* Copy the hdrl_casu_fits structure */
    hdrl_casu_fits *p = cpl_malloc(sizeof(hdrl_casu_fits));
    p->image     = cpl_image_duplicate(in->image);
    p->ehu       = cpl_propertylist_duplicate(hdrl_casu_fits_get_ehu(in));

    return p;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Get the propertylist for the extension header for a given
 *             hdrl_casu_fits image.
 *
 * @param    p         The input hdrl_casu_fits object
 *
 * @return  The propertylist representing the extension header of the input
 *            image (NULL if there is an error).
 *
 * Description:
 * 	    Get the propertylist for the extension header for a given hdrl_casu_fits
 *      image. This is the extension that is relevant of the image.
 *      This should only need to be read once and then can be used to add
 *      things to the primary header.
 */
/* ---------------------------------------------------------------------------*/
cpl_propertylist * hdrl_casu_fits_get_ehu(hdrl_casu_fits *p)
{
    /* Check for nonsense input */
    if (p == NULL) {
        return NULL;
    }

    return p->ehu;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Get the CPL image from the hdrl_casu_fits object
 *
 * @param    p         The input hdrl_casu_tfits object
 *
 * @return   The cpl_image object. NULL if there was an error.
 *
 * Description:
 * 	    Return the CPL image from the input hdrl_casu_fits object. This image is
 *      suitable for use in all cpl_image routines.
 */
/* ---------------------------------------------------------------------------*/
cpl_image * hdrl_casu_fits_get_image(hdrl_casu_fits *p)
{
    /* Check for nonsense input */
    if (p == NULL) {
        return NULL;
    }
    
    return p->image;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Free all the workspace associated with a hdrl_casu_fits object
 *
 * @param    p         The input hdrl_casu_tfits object
 *
 */
/* ---------------------------------------------------------------------------*/
void hdrl_casu_fits_delete(hdrl_casu_fits *p)
{
    /* Check for nonsense input */
    if (p != NULL) {

		/* Free up workspace if it's been used */
    	if (p->image   ) cpl_image_delete(p->image);
		if (p->ehu     ) cpl_propertylist_delete(p->ehu);

		cpl_free(p);
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Generate object catalogues from input images
 *
 * @param    infile    The input frame with the image to be analysed
 * @param    conf      The input frame with the confidence map
 * @param    wcs       Input WCS structure
 * @param    ipix      The minimum allowable size of an object
 * @param    threshold The detection threshold in sigma above sky
 * @param    icrowd    If set then the deblending software will be used
 * @param    rcore     The core radius in pixels
 * @param    bkg_subtr The background subtraction switch
 * @param    nbsize    The smoothing box size for background map estimation
 * @param    cattype   The type of catalogue to be produced
 * @param    filtfwhm  The FWHM of the smoothing kernel in the detection algorithm
 * @param    gainloc   The detector gain in e-/ADU
 * @param    saturation The saturation level
 * @param    res
 *
 * @return   CPL_ERROR_NONE if everything is ok or the concret error in other case
 *
 * Description:
 * 	    A frame and its confidence map are given. Detection thresholds and
 *      various other parameters are also given. Output is a table with all
 *      the extracted objects with object classifications included.
 *
 * @note QC headers:
 *         The following values will go into the extension propertylist
 *         - \b SATURATION:    Saturation level in ADU
 *         - \b MEAN_SKY:      Mean sky brightness in ADU
 *         - \b SKY_NOISE:     Pixel noise at sky level in ADU
 *         - \b IMAGE_SIZE:    The average FWHM of stellar objects in pixels
 *         - \b ELLIPTICITY:   The average stellar ellipticity (1 - b/a)
 *         - \b APERTURE_CORR: The stellar aperture correction for 1x core flux
 *         - \b NOISE_OBJ:     The number of noise objects
 * @note Other headers:
 *         The following values will go into the extension propertylist
 *         - \b APCORxx:       A series of aperture correction values for each of the core radius apertures.
 *         - \b SYMBOLx:       A series of keywords to be used by GAIA for plotting ellipses
 */
/* ---------------------------------------------------------------------------*/
cpl_error_code hdrl_casu_catalogue(
		hdrl_casu_fits *infile, hdrl_casu_fits *conf,
		const cpl_wcs *wcs, cpl_size ipix,
		double threshold, cpl_size icrowd, double rcore,
		cpl_size bkg_subtr, cpl_size nbsize,
		hdrl_catalogue_options cattype,
		double filtfwhm, double gainloc, double saturation,
		hdrl_casu_result *res)
{
    /* Inherited status */
    res->catalogue = NULL;

    /* Copy the input, the background is subtracted in-place */
    hdrl_casu_fits *in = hdrl_casu_fits_duplicate(infile);

    /* Call the main processing routine and get the catalogue */
    if(hdrl_catalogue_conf(in, conf, ipix, threshold, icrowd, rcore, bkg_subtr,
	                nbsize, cattype, filtfwhm, gainloc, saturation, res) != CPL_ERROR_NONE) {
        hdrl_casu_fits_delete(in);
        return cpl_error_get_code();
    }

    if (cpl_table_get_nrow(hdrl_casu_tfits_get_table(res->catalogue)) == 0) {
        hdrl_casu_fits_delete(in);
        cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND, "hdrl_cat_casu_catalogue - No objects found in image");
        return CPL_ERROR_DATA_NOT_FOUND;
    }

    /* Do the classification */
    if (cattype & HDRL_CATALOGUE_CAT_COMPLETE) {

        if (hdrl_classify(res->catalogue, 16.) != CPL_ERROR_NONE) {
            hdrl_casu_fits_delete(in);
            return cpl_error_get_code();
        }


        /* Update the RA and DEC of the objects in the object catalogue */
        if (wcs) {

			/* Update the RA and DEC of the objects in the object catalogue */
			cpl_table* cat = hdrl_casu_tfits_get_table(res->catalogue);

			double* x   = cpl_table_get_data_double(cat, "X_coordinate");
			double* y   = cpl_table_get_data_double(cat, "Y_coordinate");
			double* ra  = cpl_table_get_data_double(cat, "RA");
			double* dec = cpl_table_get_data_double(cat, "DEC");

			cpl_size n = cpl_table_get_nrow(cat);
			for (cpl_size i = 0; i < n; i++) {
				hdrl_casu_xytoradec(wcs, x[i], y[i], &(ra[i]), &(dec[i]));
			}
        }

        cpl_propertylist_set_comment(hdrl_casu_tfits_get_ehu(res->catalogue),
        		"ESO QC IMAGE_SIZE", "[pixel] Average FWHM of stellar objects");

    } else {

        cpl_table_select_all(    hdrl_casu_tfits_get_table(res->catalogue));
        cpl_table_erase_selected(hdrl_casu_tfits_get_table(res->catalogue));
    }

    hdrl_casu_fits_delete(in);

    return CPL_ERROR_NONE;
}

/**@}*/

/* ---------------------------------------------------------------------------*/
/**
 * @brief    Convert x,y -> ra,dec
 *
 * @param    wcs       Input WCS structure
 * @param    x         Input X
 * @param    y         Input Y
 * @param    ra        Output RA
 * @param    dec       Output Dec
 *
 * Description:
 * 	    A WCS structure is used to convert input x,y coordinates
 *      to equatorial coordinates.
 */
/* ---------------------------------------------------------------------------*/
static void hdrl_casu_xytoradec(const cpl_wcs *wcs, double x, double y, double *ra, double *dec)
{
    /* Load up the information */
    cpl_matrix *from = cpl_matrix_new(1, 2);
    double     *xy   = cpl_matrix_get_data(from);
    xy[0] = x;
    xy[1] = y;

    /* Call the conversion routine */
    cpl_matrix *to     = NULL;
    cpl_array  *status = NULL;
    cpl_wcs_convert(wcs, from, &to, &status, CPL_WCS_PHYS2WORLD);

    /* Pass it back now */
    double *radec = cpl_matrix_get_data(to);
    *ra  = radec[0];
    *dec = radec[1];

    /* Tidy and exit */
    cpl_matrix_delete(from);
    cpl_matrix_delete(to);
    cpl_array_delete(status);
}
