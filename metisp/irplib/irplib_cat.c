/* $Id: irplib_cat.c,v 1.10 2009-12-01 12:34:25 cgarcia Exp $
 *
 * This file is part of the irplib package
 * Copyright (C) 2002,2003 European Southern Observatory
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02111-1307  USA
 */

/*
 * $Author: cgarcia $
 * $Date: 2009-12-01 12:34:25 $
 * $Revision: 1.10 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <libgen.h>
#include <math.h>
#include <cpl.h>

#include "irplib_cat.h"
#include "irplib_wcs.h"

#define FILENAME_SZBUF 1024

/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_cat   Functions for accessing catalogues
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*---------------------------------------------------------------------------*/
/**
 * @brief
 *       Find the name of the standard catalogue being used and its location
 * @param index
 *       The frame for the index FITS file
 * @param catpath
 *       The full path to the catalgoue FITS files
 * @param catname
 *       The name of the catalogue
 * @return
 *   CPL_ERROR_NONE if sucess
 *   CPL_ERROR_FILE_IO if the file does not exist or the header cannot be read.
 *
 *   Find the name of the standard catalogue being used and its location.
 *   The former should be in a header keyword in the specified FITS
 *   file. The latter is the full path of the FITS file. Both values
 *   need to be deallocated when you're finished with them.
*/
/*---------------------------------------------------------------------------*/

int irplib_2mass_get_catpars
(const cpl_frame *  master_index,
 char            ** catpath,
 char            ** catname)
{
    cpl_propertylist * p;
    char             * fname;
    int                status = CPL_ERROR_NONE;

    /* Initialise a few things */
    *catpath = NULL;
    *catname = NULL;

    /* First get the full path to the index file and make sure it exists */
    fname = cpl_strdup(cpl_frame_get_filename(master_index));
    if (access((const char *)fname,R_OK) != 0)
    {
         cpl_msg_error(__func__,"Can't access index file %s",fname);
         cpl_free(fname);
         return CPL_ERROR_FILE_IO;
    }
    *catpath = cpl_strdup(dirname(fname));

    /* Try to load the propertylist. If it is not possible signal a fatal
       error since this probably means the whole file is messed up */
    if ((p = cpl_propertylist_load(cpl_frame_get_filename(master_index),0)) == NULL)
    {
        cpl_msg_error(__func__,"Can't load index file header %s",fname);
        cpl_free(*catpath);
        cpl_free(fname);
        return CPL_ERROR_FILE_IO;
    }

    /* If there is a catalogue name in the header then send it back. If there
       isn't then give a default name and send a warning */
    if (cpl_propertylist_has(p,"CATNAME"))
    {
        *catname = cpl_strdup(cpl_propertylist_get_string(p,"CATNAME"));
        status = CPL_ERROR_NONE;
    } else {
        const char * unk = "unknown";
        *catname = cpl_strdup(unk);
        cpl_msg_warning(__func__,"Property CATNAME not in index file header %s",
                        fname);
    }

    /* Free and return */
    cpl_free(fname);
    cpl_propertylist_delete(p);
    return(status);
}



/*---------------------------------------------------------------------------*/
/**
 *  @brief
 *       Get coverage in ra, dec of a frame
 *  @param plist
 *      Input property list
 *  @param ext_search
 *      Factor for an extra box search.
 *  @param ra1
 *       Lower RA
 *  @param ra2
 *       Upper RA
 *  @param dec1
 *       Lower Dec
 *  @param dec2
 *       Upper Dec
 *  @return
 *      CPL_ERROR_NONE if sucess.
 *      CPL_ERROR_DATA_NOT_FOUND if wcs is not valid
 *
 *      Given a WCS solution this routine works out the min and max equatorial
 *      coordinates covered by the image.
 */
/*---------------------------------------------------------------------------*/

cpl_error_code irplib_cat_get_image_limits
(const cpl_wcs    * wcs,
 float              ext_search,
 double           * ra1,
 double           * ra2,
 double           * dec1,
 double           * dec2)
{
    double            min_4q;
    double            max_1q;
    int               first_quad;
    int               fourth_quad;
    const int       * naxes;
    long              j;
    const cpl_array * a;

    /* Initialise these in case of failure later*/
    *ra1 = 0.0;
    *ra2 = 0.0;
    *dec1 = 0.0;
    *dec2 = 0.0;

    /* Grab the WCS info from the property list */
    if (wcs == NULL)
        return CPL_ERROR_DATA_NOT_FOUND;

    /* Get the size of the data array */

    a = cpl_wcs_get_image_dims(wcs);
    if(a == NULL)
        return CPL_ERROR_ILLEGAL_INPUT;
    naxes = cpl_array_get_data_int_const(a);

    /* Find the RA and Dec limits of the image */

    *ra1 = 370.0;
    *ra2 = -370.0;
    *dec1 = 95.0;
    *dec2 = -95.0;
    first_quad = 0;
    fourth_quad = 0;
    min_4q = 370.0;
    max_1q = 0.0;
    for (j = 1; j < naxes[1]; j += 10) {
        long   i;
        double y = (double)j;
        for (i = 1; i < naxes[0]; i += 10) {
            double ra, dec;
            double x = (double)i;
            irplib_wcs_xytoradec(wcs,x,y,&ra,&dec);
            if (ra >= 0.0 && ra <= 90.0) {
                first_quad = 1;
                if(ra > max_1q)
                    max_1q = ra;
            } else if (ra >= 270.0 && ra <= 360.0) {
                fourth_quad = 1;
                if(ra - 360.0 < min_4q)
                    min_4q = ra - 360.0;
            }
            if(ra < *ra1)
                *ra1 = ra;
            if(ra > *ra2)
                *ra2 = ra;
            if(dec < *dec1)
                *dec1 = dec;
            if(dec > *dec2)
                *dec2 = dec;
        }
    }

    /* Now have a look to see if you had RA values in both the first and
       fourth quadrants.  If you have, then make the minimum RA a negative
       value.  This will be the signal to the caller that you have the
       wraparound... */

    if (first_quad && fourth_quad) {
        *ra1 = min_4q;
        *ra2 = max_1q;
    }

    /* Pad out search a bit */
    if (ext_search)
    {
        double dra, ddec;
        dra = 0.5*ext_search*(*ra2 - *ra1);
        *ra1 -= dra;
        *ra2 += dra;
        ddec = 0.5*ext_search*(*dec2 - *dec1);
        *dec1 -= ddec;
        *dec2 += ddec;
    }

    /* Exit */
    return CPL_ERROR_NONE;
}

/*---------------------------------------------------------------------------*/
/**
 *
 *  @brief
 *       Extract standards from the 2mass catalogue
 *  @param path
 *    The full path to the catalogue FITS files and index.
 *  @param ramin1
 *    The minimum RA, this can be negative in the case the area wraps around
 *    the equinox.
 *  @param ramax1
 *    The maximum RA
 *  @param decmin
 *    The minimum Declination
 *  @param decmax
 *    The maximum Declination
 *  @return
 *    A table structure with the extracted catalogue objects
 *
 *
 *       The FITS tables containing the 2mass psc catalogue are searched
 *       to find all of the objects within an input equatorial area. Deals
 *   with the sigularity at the equinox, but not at the poles.
 */
/*---------------------------------------------------------------------------*/

cpl_table *  irplib_2mass_extract
(char *path,
 float ramin,
 float ramax,
 float decmin,
 float decmax)
{
    cpl_table *t,*s;
    cpl_table *out;
    int i,nrows,start,finish,first_index,last_index,irow,init,j;
    int wrap,iwrap;
    float dectest,ratest,ramin_wrap,ramax_wrap;
    char fullname[FILENAME_SZBUF];
    cpl_array *a;
    const char *deccol[] = {"Dec"};
    cpl_propertylist *p;

    /* Create an output table */

    out = cpl_table_new(0);
    init = 1;

    /* Create a cpl array */

    /* deccol will NOT be modified */
    a = cpl_array_wrap_string((char **)deccol,1);

    /* Is there a wrap around problem? */

    wrap = (ramin < 0.0 && ramax > 0.0) ? 2 : 1;

    /* Loop for each query. If there is a wrap around problem then we need 2
       queries. If not, then we only need 1 */

    for (iwrap = 0; iwrap < wrap; iwrap++) {
        int first_index_ra,last_index_ra;

        if (wrap == 2) {
            if (iwrap == 0) {
                ramin_wrap = ramin + 360.0;
                ramax_wrap = 360.0;
            } else {
                ramin_wrap = 0.000001;
                ramax_wrap = ramax;
            }
        } else {
            ramin_wrap = ramin;
            ramax_wrap = ramax;
        }

        /* Find out where in the index to look */

        first_index_ra = (int)ramin_wrap;
        last_index_ra = (int)ramax_wrap;
        if(last_index_ra > 359)
            last_index_ra = 359;

        /* Look at the min and max RA and decide which files need to be
           opened. */

        for (i = first_index_ra; i <= last_index_ra; i++)
        {

            /* Ok, we've found one that needs opening. Read the file with
               the relevant CPL call */

            (void)snprintf(fullname,FILENAME_SZBUF,"%s/npsc%03d.fits",path,i);

            /* Read the propertylist so that you know how many rows there
               are in the table */

            p = cpl_propertylist_load(fullname,1);
            if (p == NULL)
            {
                cpl_error_set_message_macro(__func__,CPL_ERROR_DATA_NOT_FOUND,
                        __FILE__, __LINE__, "2mass file %s missing",fullname);
                cpl_table_delete(out);
                cpl_array_unwrap(a);
                return(NULL);
            }
            nrows = cpl_propertylist_get_int(p, "NAXIS2");
            cpl_propertylist_delete(p);

            /* Load various rows until you find the Dec range that you
             have specified. First the minimum Dec */

            start = 0;
            finish = nrows;
            first_index = nrows/2;
            while (finish - start >= 2)
            {
                t = cpl_table_load_window(fullname, 1, 0, a, first_index, 1);
                dectest = cpl_table_get_float(t, "Dec", 0, NULL);
                cpl_table_delete(t);
                if (dectest < decmin)
                {
                    start = first_index;
                    first_index = (first_index + finish)/2;
                }
                else
                {
                    finish = first_index;
                    first_index = (first_index + start)/2;
                }
            }

            /* Load various rows until you find the Dec range that you
             have specified. Now the maximum Dec */

            start = first_index;
            finish = nrows;
            last_index = start + (finish - start)/2;
            while (finish - start >= 2)
            {
                t = cpl_table_load_window(fullname, 1, 0, a, last_index, 1);
                dectest = cpl_table_get_float(t, "Dec", 0, NULL);
                cpl_table_delete(t);
                if (dectest < decmax)
                {
                    start = last_index;
                    last_index = (last_index + finish)/2;
                }
                else
                {
                    finish = last_index;
                    last_index = (last_index + start)/2;
                }
            }
            if (last_index < first_index)
                last_index = first_index;

            /* Ok now now load all the rows in the relevant dec limits */

            nrows = last_index - first_index + 1;
            if ((t = cpl_table_load_window(fullname, 1, 0, NULL, first_index,
                                           nrows)) == NULL)
            {
                cpl_error_set_message_macro(__func__,CPL_ERROR_DATA_NOT_FOUND,
                        __FILE__, __LINE__, "Error in subset of 2mass file %s ",
                        fullname);
                cpl_table_delete(out);
                cpl_array_unwrap(a);
                return (NULL);
            }
            cpl_table_unselect_all(t);

            /* Right, we now know what range of rows to search. Go through
             these and pick the ones that are in the correct range of RA.
             If a row qualifies, then 'select' it. */

            for (j = 0; j < nrows; j++)
            {
                ratest = cpl_table_get_float(t, "RA", j, NULL);
                if (cpl_error_get_code() != CPL_ERROR_NONE)
                {
                    cpl_error_set_message_macro(__func__,CPL_ERROR_DATA_NOT_FOUND,
                            __FILE__, __LINE__, "No RA column in 2mass file %s",
                            fullname);
                    cpl_table_delete(t);
                    cpl_array_unwrap(a);
                    cpl_table_delete(out);
                    return (NULL);
                }
                if (ratest >= ramin_wrap && ratest <= ramax_wrap)
                    cpl_table_select_row(t, j);
            }

            /* Extract the rows that have been selected now and append them
             onto the output table */

            s = cpl_table_extract_selected(t);
            if (init == 1)
            {
                cpl_table_copy_structure(out, t);
                init = 0;
            }
            irow = cpl_table_get_nrow(out) + 1;
            cpl_table_insert(out, s, irow);

            /* Tidy up */

            cpl_table_delete(t);
            cpl_table_delete(s);
        }
    }

    /* Ok, now just return the table and get out of here */

    cpl_array_unwrap(a);
    return(out);
}
/**@}*/

