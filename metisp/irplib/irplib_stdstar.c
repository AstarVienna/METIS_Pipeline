/* $Id: irplib_stdstar.c,v 1.45 2013-03-01 10:27:07 llundin Exp $
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
 * $Author: llundin $
 * $Date: 2013-03-01 10:27:07 $
 * $Revision: 1.45 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "irplib_stdstar.h"
#include "irplib_utils.h"
#include "irplib_wcs.h"
#include <cpl.h>

#include <string.h>
#include <math.h>
#include <float.h>

/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_stdstar     Functions for standard stars
 */
/*----------------------------------------------------------------------------*/
/**@{*/

/*-----------------------------------------------------------------------------
                                   Functions code
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Write the ASCII catalogs as FITS files
  @param    set_in          Input frameset where the product is registered
  @param    set_raw         Set of ASCII catalogs
  @param    recipe_name     Recipe name
  @param    pro_cat         PRO.CATG
  @param    pro_type        PRO.TYPE
  @param    package_name    Usually PACKAGE "/" PACKAGE_VERSION
  @param    ins_name        Instrument name
  @param    convert_ascii_table Conversion function
  @return   CPL_ERROR_NONE if ok or else the relevant CPL error code

  Every catalog will be written in a different extension, where EXTNAME
  is the name of the catalog ASCII file name.
 
  The conversion ascii -> cpl_table is done by convert_ascii_table() and
  should be defined in each instrument
 */
/*----------------------------------------------------------------------------*/
cpl_error_code
irplib_stdstar_write_catalogs(cpl_frameset        *   set_in,
                              const cpl_frameset  *   set_raw,
                              const char          *   recipe_name,
                              const char          *   pro_cat,
                              const char          *   pro_type,
                              const char          *   package_name,
                              const char          *   ins_name,
                              cpl_table * (*convert_ascii_table)(const char *))
{
    /* Number of catalogs */
    const cpl_size     nb_catalogs = cpl_frameset_get_size(set_raw);
    cpl_propertylist * plist_ext;
    char             * out_name;
    cpl_error_code     error = CPL_ERROR_NONE;
    cpl_size           i;

    /* Check entries */
    if (set_in == NULL) return cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
    if (set_raw == NULL) return cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
    if (recipe_name == NULL) return cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
    if (pro_cat == NULL) return cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
    if (ins_name == NULL) return cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
    if (convert_ascii_table == NULL) return
        cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);

    /* Define the file name */
    out_name = cpl_sprintf("%s" CPL_DFS_FITS, recipe_name);
    
    plist_ext = cpl_propertylist_new();
    
    /* Process the catalogs */
    for (i = 0; i < nb_catalogs; i++) {
        /* Get the catalog name */
        const cpl_frame * cur_frame = cpl_frameset_get_position_const(set_raw,
                                                                      i);
        const char      * cat_name = cpl_frame_get_filename(cur_frame);

        cpl_table       * out = convert_ascii_table(cat_name);
     
        /* Create the output table */
        if (out == NULL) {
            error = cpl_error_get_code() ? cpl_error_set_where(cpl_func)
                : cpl_error_set(cpl_func, CPL_ERROR_UNSPECIFIED);
            break;
        }

        if (cpl_table_get_nrow(out) == 0) {
            cpl_table_delete(out);
            error = cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                                          "Empty catalogue %d in '%s'",
                                          (int)i+1, cat_name);
            break;
        }

        cpl_propertylist_update_string(plist_ext, "EXTNAME", cat_name);

        /* Write the table */
        if (i == 0) {
            cpl_parameterlist * parlist = cpl_parameterlist_new();
            cpl_propertylist  * plist   = cpl_propertylist_new();

            /* Mandatory keywords */
            cpl_propertylist_append_string(plist, "INSTRUME", ins_name);
            cpl_propertylist_append_string(plist, CPL_DFS_PRO_CATG, pro_cat);
            if (pro_type != NULL) {
                cpl_propertylist_append_string(plist, CPL_DFS_PRO_TYPE,
                                               pro_type);
            }

            error = cpl_dfs_save_table(set_in, NULL, parlist, set_raw, NULL,
                                       out, plist_ext, recipe_name, plist,
                                       NULL, package_name, out_name);
            cpl_parameterlist_delete(parlist);
            cpl_propertylist_delete(plist);
        } else {
            error = cpl_table_save(out, NULL, plist_ext, out_name,
                                   CPL_IO_EXTEND);
        }

        cpl_table_delete(out);

        if (error) {
            (void)cpl_error_set_where(cpl_func);
            break;
        }
    }

    cpl_propertylist_delete(plist_ext);
    cpl_free(out_name);

    return error;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Load the FITS catalog in a table
  @param    filename    Name of the FITS catalog
  @param    ext_name    Name of the catalog or "all"
  @return   The newly allocated table on NULL in error case.

  The table is loaded from the specified extension.
  If "all" is specified, all extension with the same columns as the
  first one are loaded and put together in the returned table.
  
  The returned table must be de allocated with cpl_table_delete().
 */
/*----------------------------------------------------------------------------*/
cpl_table * irplib_stdstar_load_catalog(
        const char  *   filename, 
        const char  *   ext_name)
{
    int                     next;
    cpl_table           *   out;
    cpl_table           *   out_cur;
    cpl_frame           *   cur_frame;
    int                     i;

    /* Check entries */
    if (filename == NULL) return NULL;
    if (ext_name == NULL) return NULL;
    
    /* Initialise */
    out = NULL;
    
    /* Get the number of extensions in the catalog */
    cur_frame = cpl_frame_new();
    cpl_frame_set_filename(cur_frame, filename);
    next = cpl_frame_get_nextensions(cur_frame);
    cpl_frame_delete(cur_frame);

    /* Loop on the extentions */
    for (i=0; i<next; i++) {
        cpl_propertylist    *   plist;
        const char          *   cur_name;

        /* Check the name of the current extension */
        if ((plist = cpl_propertylist_load_regexp(filename, i+1, "EXTNAME", 
                        0)) == NULL) {
            cpl_msg_error(cpl_func, "Cannot load header of %d th extension",
                    i+1);
            return NULL;
        }
        cur_name = cpl_propertylist_get_string(plist, "EXTNAME");
        
        /* Check the current extension */
        if (!strcmp(cur_name, ext_name)) {
            /* Load the table */
            if (out == NULL) {
                out = cpl_table_load(filename, i+1, 1);
                cpl_table_new_column(out, IRPLIB_STDSTAR_CAT_COL, CPL_TYPE_STRING);
                cpl_table_fill_column_window_string(out, IRPLIB_STDSTAR_CAT_COL,
                                                    0, cpl_table_get_nrow(out),
                                                    cur_name);
                if (out == NULL) {
                    cpl_msg_error(cpl_func, "Cannot load extension %d", i+1);
                    cpl_propertylist_delete(plist);
                    return NULL;
                }
            }
        } else if (!strcmp(ext_name, "all")) {
            /* Load the table and append it */
            if (i==0) {
                /* Load the first table */
                out = cpl_table_load(filename, i+1, 1);
                cpl_table_new_column(out, IRPLIB_STDSTAR_CAT_COL, CPL_TYPE_STRING);
                cpl_table_fill_column_window_string(out, IRPLIB_STDSTAR_CAT_COL,
                                                    0, cpl_table_get_nrow(out),
                                                    cur_name);
                if (out == NULL) {
                    cpl_msg_error(cpl_func, "Cannot load extension %d", i+1);
                    cpl_propertylist_delete(plist); 
                    return NULL;
                }
            } else {
                /* Load the current table */
                out_cur = cpl_table_load(filename, i+1, 1);
                if (out_cur == NULL) {
                    cpl_msg_error(cpl_func, "Cannot load extension %d", i+1);
                    cpl_table_delete(out);
                    cpl_propertylist_delete(plist); 
                    return NULL;
                }
                cpl_table_new_column(out_cur, IRPLIB_STDSTAR_CAT_COL, CPL_TYPE_STRING);
                cpl_table_fill_column_window_string(out_cur, IRPLIB_STDSTAR_CAT_COL,
                                                    0, cpl_table_get_nrow(out_cur),
                                                    cur_name);
                /* Append the table */
                if (cpl_table_insert(out, out_cur, 
                            cpl_table_get_nrow(out)) != CPL_ERROR_NONE) {
                    cpl_msg_error(cpl_func, "Cannot merge table %d", i+1);
                    cpl_table_delete(out);
                    cpl_table_delete(out_cur);
                    cpl_propertylist_delete(plist); 
                    return NULL;
                }
                cpl_table_delete(out_cur);
            }
        }
        cpl_propertylist_delete(plist);
    }
    return out;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Check that the table has the relevant columns of a stdstar table
  @param    catal    Table with the catalogue
  @return   CPL_ERROR_NONE if the table has all the mandatory columns, 
            a proper error code otherwise 

  The table is checked for the presence of the mandatory keywords
 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_stdstar_check_columns_exist(
        const cpl_table  *   catal)
{
    /* Check for all the mandatory columns */
    if (!cpl_table_has_column(catal, IRPLIB_STDSTAR_STAR_COL)) {
        return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                "Missing column: %s",
                IRPLIB_STDSTAR_STAR_COL);
    }
    if (!cpl_table_has_column(catal, IRPLIB_STDSTAR_TYPE_COL)) {
        return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                "Missing column: %s",
                IRPLIB_STDSTAR_TYPE_COL);
    }
    if (!cpl_table_has_column(catal, IRPLIB_STDSTAR_CAT_COL)) {
        return cpl_error_set_message(cpl_func, 
                CPL_ERROR_ILLEGAL_INPUT,
                "Missing column: %s",
                IRPLIB_STDSTAR_CAT_COL);
    }
    if (!cpl_table_has_column(catal, IRPLIB_STDSTAR_RA_COL)) {
        return cpl_error_set_message(cpl_func, 
                CPL_ERROR_ILLEGAL_INPUT,
                "Missing column: %s",
                IRPLIB_STDSTAR_RA_COL);
    }
    if (!cpl_table_has_column(catal, IRPLIB_STDSTAR_DEC_COL)) {
        return cpl_error_set_message(cpl_func, 
                CPL_ERROR_ILLEGAL_INPUT,
                "Missing column: %s",
                IRPLIB_STDSTAR_DEC_COL);
    }
    return CPL_ERROR_NONE;
}

/*----------------------------------------------------------------------------*/
/**
  @brief  Deselect the stars that are beyond a given distance
  @param   cat   The standard star catalog, with too distant stars to unselect
  @param   ra    Right ascension [degrees]
  @param   dec   Declination [degrees]
  @param   dist  The distance [degrees]
  @return  0 if ok, -1 in error case

  The stars that are further than dist from ra,dec are unselected in the table.
 */
/*----------------------------------------------------------------------------*/
int irplib_stdstar_select_stars_dist(
        cpl_table   *   cat, 
        double          ra, 
        double          dec, 
        double          dist)
{
    cpl_size            i, nrows;
    
    /* Check entries */
    if (cat == NULL) return -1;

    /* Get the number of selected rows */
    nrows = cpl_table_get_nrow(cat);

    /* Check if the columns are there */
    if (!cpl_table_has_column(cat, IRPLIB_STDSTAR_RA_COL)) {
        cpl_msg_error(cpl_func, "Missing column: " IRPLIB_STDSTAR_RA_COL);
        return -1;
    }
    if (!cpl_table_has_column(cat, IRPLIB_STDSTAR_DEC_COL)) {
        cpl_msg_error(cpl_func, "Missing column: " IRPLIB_STDSTAR_DEC_COL);
        return -1;
    }
    
    if (cpl_table_count_selected(cat) == 0) {
        cpl_msg_error(cpl_func, "All %d row(s) already deselected", (int)nrows);
        return -1;
    }
    
    /* Compute distances of the selected rows */
    for (i = 0; i < nrows; i++) {
        if (cpl_table_is_selected(cat, i)) {
            /* The row is selected - compute the distance */
            const double distance = irplib_wcs_great_circle_dist(ra, dec, 
                    cpl_table_get_double(cat, IRPLIB_STDSTAR_RA_COL, i, NULL),
                    cpl_table_get_double(cat, IRPLIB_STDSTAR_DEC_COL, i, NULL));            
            if (distance > dist) cpl_table_unselect_row(cat, i);
        }
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Select the stars that have a known magnitude
  @param    cat         the catalog
  @param    mag_colname the column name with the searched magnitude
  @return   0 if ok, -1 in error case

  The stars whose magnitude in the mag band is known are selected in the table.
 */
/*----------------------------------------------------------------------------*/
int irplib_stdstar_select_stars_mag(
        cpl_table   *   cat, 
        const char  *   mag_colname)
{
    /* Check entries */
    if (cat == NULL) return -1;
    if (mag_colname == NULL) return -1;

    /* Check that the table has the mag column */
    if (!cpl_table_has_column(cat, mag_colname)) {
        cpl_msg_error(cpl_func, "Column %s does not exist in the catalog",
                mag_colname);
        return -1;
    }

    /* Apply the selection */
    if (cpl_table_and_selected_double(cat, mag_colname, CPL_NOT_GREATER_THAN, 
                98.0) <= 0) {
        cpl_msg_error(cpl_func, "Column %s does not exist in the catalog",
                mag_colname);
        return -1;
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Find the closest star
  @param    cat     the catalog
  @param    ra      RA pos
  @param    dec     DEC pos
  @return   the index of the star in the table or -1 in error case
  
  Returns the index of the star that is closest to (ra,dec)
 */
/*----------------------------------------------------------------------------*/
int irplib_stdstar_find_closest(
        const cpl_table     *   cat, 
        double                  ra, 
        double                  dec)
{
    double              min_dist, distance;
    int                 nrows;
    int                 ind;
    int                 i;

    /* Check entries */
    if (cat == NULL) return -1;

    /* Initialize */
    min_dist = 1000.0;
    ind = -1;

    /* Get the number of selected rows */
    nrows = cpl_table_get_nrow(cat);

    /* Check if the columns are there */
    if (!cpl_table_has_column(cat, IRPLIB_STDSTAR_RA_COL)) {
        cpl_msg_error(cpl_func, "Missing %s column", IRPLIB_STDSTAR_RA_COL);
        return -1;
    }
    if (!cpl_table_has_column(cat, IRPLIB_STDSTAR_DEC_COL)) {
        cpl_msg_error(cpl_func, "Missing %s column", IRPLIB_STDSTAR_DEC_COL);
        return -1;
    }
    
    /* Compute distances of the selected rows */
    for (i=0; i<nrows; i++) {
        if (cpl_table_is_selected(cat, i)) {
            /* The row is selected - compute the distance */
            distance = irplib_wcs_great_circle_dist(ra, dec,
                    cpl_table_get_double(cat, IRPLIB_STDSTAR_RA_COL, i, NULL),
                    cpl_table_get_double(cat, IRPLIB_STDSTAR_DEC_COL, i, NULL));
            if (distance <= min_dist) {
                min_dist = distance;
                ind = i;
            }
        }
    }
    return ind;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Find the closest star to ra, dec in the catalog
  @param    catfile  the catalog file name
  @param    ra       RA pos where to search
  @param    dec      DEC pos where to search
  @param    band     the band name
  @param    catname  the searched catalog name
  @param    mag      the computed magnitude (output)
  @param    name     the star name (output)
  @param    type     the star type (output)
  @param    star_ra  the star RA from the catalog (output)
  @param    star_dec the star DEC from the catalog (output)
  @param    dist_am  the distance in arc minutes
  @return   CPL_ERROR_NONE if ok or else the relevant CPL error code

  The closest star to ra, dec with defined magnitude is search in all the 
  catalogues included in catfile.
  If catname is "all", all the catalogs are searched at once.
  The output pointers can be NULL if one is not interested in them.
 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_stdstar_find_star(
        const char          *   catfile,
        double                  ra, 
        double                  dec,
        const char          *   band,
        const char          *   catname,
        double              *   mag,
        char                **  name,
        char                **  type,
        char                **  usedcatname,
        double              *   star_ra,
        double              *   star_dec,
        double                  dist_am)
{
    cpl_errorstate prestate = cpl_errorstate_get();
    cpl_table   *   catal;
    const double    dist = dist_am / 60.0;
    int             ind;

    /* Check entries */
    if (catfile == NULL) return cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
    if (band    == NULL) return cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
    if (catname == NULL) return cpl_error_set(cpl_func, CPL_ERROR_NULL_INPUT);
    
    /* Load the catalog */
    if ((catal = irplib_stdstar_load_catalog(catfile, catname)) == NULL) {
        return cpl_error_set_message(cpl_func, CPL_ERROR_FILE_NOT_FOUND,
                                           "Cannot load the catalog %s from %s",
                                           catname, catfile);
    }
    
    /* Check the columns are present */
    if (irplib_stdstar_check_columns_exist(catal) != CPL_ERROR_NONE) {
        cpl_table_delete(catal);
        return cpl_error_set_where(cpl_func);
    }
    
    /* Select stars with known magnitude */
    if (irplib_stdstar_select_stars_mag(catal, band) == -1) {
        cpl_table_delete(catal);
        return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                                           "Cannot select stars in that band");
    }

    /* Select stars within a given distance */
    if (irplib_stdstar_select_stars_dist(catal, ra, dec, dist) == -1) {
        cpl_table_delete(catal);
        return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                                           "Cannot select close stars");
    }

    /* Take the closest */
    if ((ind=irplib_stdstar_find_closest(catal, ra, dec)) < 0) {
        cpl_table_delete(catal);
        return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                                           "Cannot get the closest star with "
                                           "known %s magnitude",band);
    }

    if(mag != NULL)
        *mag = cpl_table_get_double(catal, band, ind, NULL);

    if(name != NULL)
    {
        *name = cpl_strdup(cpl_table_get_string(catal,
                                                IRPLIB_STDSTAR_STAR_COL, ind));

    }
    if(type != NULL)
    {
        *type = cpl_strdup(cpl_table_get_string(catal, IRPLIB_STDSTAR_TYPE_COL,
                                                ind));
    }
    if(usedcatname != NULL)
    {
        if(strcmp(catname, "all"))
            *usedcatname = cpl_strdup(catname);
        else
        {
            *usedcatname = cpl_strdup(cpl_table_get_string
                                      (catal, IRPLIB_STDSTAR_CAT_COL, ind));
        }
    }
    if(star_ra != NULL)
        *star_ra  = cpl_table_get_double(catal, IRPLIB_STDSTAR_RA_COL, ind, NULL);
    if(star_dec != NULL)
        *star_dec = cpl_table_get_double(catal, IRPLIB_STDSTAR_DEC_COL, ind, NULL);
    
    /* Free and return */
    cpl_table_delete(catal);
    return cpl_errorstate_is_equal(prestate) ? CPL_ERROR_NONE
        : cpl_error_set_where(cpl_func);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Get the conversion
  @param    spec    the extracted spectrum 
  @param    dit     the DIT (in sec)
  @param    surface the surface of the miror (in sq cm)
  @param    gain    the gain of the instrument (ISAAC=4.5, SINFONI=2.42)
  @param    mag     the star magnitude
  @return   the newly allocated conversion or NULL in error case

  conversion(wave) = (spec * gain * 10^(mag/2.5) * h * c) / 
                     (dit * surface * dispersion * wave)
 */
/*----------------------------------------------------------------------------*/
cpl_vector * irplib_stdstar_get_conversion(
        const cpl_bivector  *   spec,
        double                  dit,
        double                  surface,
        double                  gain,
        double                  mag)
{
    double                      h = 6.62e-27;
    double                      c = 3e18;
    const cpl_vector    *       wave;
    const cpl_vector    *       extr;
    cpl_vector          *       out;
    double                      factor;

    /* Test entries */
    if (spec == NULL) return NULL;
    if (dit <= 0.0) return NULL;

    /* Get the extracted spectrum */
    wave = cpl_bivector_get_x_const(spec);
    extr = cpl_bivector_get_y_const(spec);

    /* Get the spectrum */
    out = cpl_vector_duplicate(extr);

    /* Divide by DIT */
    cpl_vector_divide_scalar(out, dit);

    /* Divide by the surface */
    cpl_vector_divide_scalar(out, surface);

    /* Multiply by the gain */
    cpl_vector_multiply_scalar(out, gain);

    /* Multiply by the difference magnitude */
    factor = pow(10, mag/2.5);
    cpl_vector_multiply_scalar(out, factor);

    /* Divide by the dispersion */
    factor = (cpl_vector_get(wave, cpl_vector_get_size(wave)-1) -
            cpl_vector_get(wave, 0)) / cpl_vector_get_size(wave);
    cpl_vector_divide_scalar(out, factor);

    /* Multiply by the energy of the photon */
    cpl_vector_multiply_scalar(out, h*c);
    cpl_vector_divide(out, wave);

    return out;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Get the 0 magnitude spectrum
  @param    sed     the SED in angstroms / ergs/s/cm^2/Angstrom
  @param    waves   the wavelengths in angstroms
  @param    cent_wl the central wavelength in microns
  @return   the newly allocated spectrum or NULL in error case
 */
/*----------------------------------------------------------------------------*/
cpl_vector * irplib_stdstar_get_mag_zero(
        const cpl_bivector  *   sed,
        const cpl_vector    *   waves,
        double                  cent_wl)
{
    double              wmin, wmax, wstep;
    int                 nb_sed;
    const double    *   sed_x;
    const double    *   sed_y;
    cpl_bivector    *   sed_loc;
    double          *   sed_loc_x;
    double          *   sed_loc_y;
    cpl_vector      *   out;
    cpl_bivector    *   out_biv;
    double              f0_jan, f0_erg, cent_val;
    int                 i;

    /* Test entries */
    if (sed == NULL) return NULL;
    if (waves == NULL) return NULL;

    /* Initialise */
    nb_sed = cpl_bivector_get_size(sed);
    sed_x = cpl_bivector_get_x_data_const(sed);
    sed_y = cpl_bivector_get_y_data_const(sed);
    wstep = sed_x[1] - sed_x[0];
    wmin = cpl_vector_get(waves, 0);
    wmax = cpl_vector_get(waves, cpl_vector_get_size(waves)-1);

    /* Expand sed with 0 to have it bigger than the required wavelengths */
    sed_loc = cpl_bivector_new(nb_sed + 4);
    sed_loc_x = cpl_bivector_get_x_data(sed_loc);
    sed_loc_y = cpl_bivector_get_y_data(sed_loc);
    for (i=0; i<nb_sed; i++) {
        sed_loc_x[i+2] = sed_x[i];
        sed_loc_y[i+2] = sed_y[i];
    }

    /* Low bound */
    sed_loc_x[1] = sed_loc_x[2] - wstep;
    if (sed_loc_x[2] < wmin) {
        sed_loc_x[0] = sed_loc_x[1] - wstep;
    } else {
        sed_loc_x[0] = wmin - wstep;
    }
    sed_loc_y[0] = 1e-20;
    sed_loc_y[1] = 1e-20;

    /* High bound */
    sed_loc_x[nb_sed+2] = sed_loc_x[nb_sed+1] + wstep;
    if (sed_loc_x[nb_sed+1] > wmax) {
        sed_loc_x[nb_sed+3] = sed_loc_x[nb_sed+2] + wstep;
    } else {
        sed_loc_x[nb_sed+3] = wmax + wstep;
    }
    sed_loc_y[nb_sed+2] = 1e-20;
    sed_loc_y[nb_sed+3] = 1e-20;

    /* Create the output bivector */
    out = cpl_vector_duplicate(waves);
    IRPLIB_DIAG_PRAGMA_PUSH_IGN(-Wcast-qual);
    /* the X entry (waves) is not modified by cpl_bivector_interpolate_linear */
    out_biv = cpl_bivector_wrap_vectors((cpl_vector*)waves, out);
    IRPLIB_DIAG_PRAGMA_POP;
    /* Interpolate */
    if (cpl_bivector_interpolate_linear(out_biv, sed_loc) != CPL_ERROR_NONE) {
        cpl_msg_error(cpl_func, "Cannot interpolate the wavelength");
        cpl_bivector_unwrap_vectors(out_biv);
        cpl_vector_delete(out);
        cpl_bivector_delete(sed_loc);
        return NULL;
    }
    cpl_bivector_unwrap_vectors(out_biv);
    cpl_bivector_delete(sed_loc);

    /* Compute f0_jan */
    f0_jan = 5513.15 / ( pow(cent_wl,3) * (exp(1.2848/cent_wl)-1) );

    /* Convert f0 Jansky -> ergs/s/cm^2/Angstrom */
    f0_erg = f0_jan * 1e-26 * 1e7 * 3e18 / (1e4 * cent_wl*cent_wl*1e4*1e4);

    /* Scale out so that the central value is f0 */
    cent_val = cpl_vector_get(out, cpl_vector_get_size(out)/2);
    if (cent_val <= 0.0) {
        cpl_msg_error(cpl_func, "Negative or 0 central value");
        cpl_vector_delete(out);
        return NULL;
    }
    cpl_vector_multiply_scalar(out, f0_erg/cent_val);

    /* Return */
    return out;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Get the SED
  @param    seds_file   the table file name
  @param    sptype      the requested spectral type
  @return   the newly allocated SED or NULL in error case

  wavelength in Angstroms
  SED in 
 */
/*----------------------------------------------------------------------------*/
cpl_bivector * irplib_stdstar_get_sed(
        const char  *   seds_file,
        const char  *   sptype)
{
    cpl_table           *   seds;
    cpl_bivector        *   out;
    cpl_vector          *   wave;
    cpl_vector          *   sed;
    cpl_bivector        *   tmp;
    int                     nlines;

    /* Test entries */
    if (seds_file == NULL) return NULL;
    if (sptype == NULL) return NULL;

    /* Load the table */
    if ((seds = cpl_table_load(seds_file, 1, 0)) == NULL) {
        cpl_msg_error(cpl_func, "Cannot load the table");
        return NULL;
    }

    /* Check if the column is there */
    if (!cpl_table_has_column(seds, sptype)) {
        cpl_msg_error(cpl_func, "SED of the requested star not available");
        cpl_table_delete(seds);
        return NULL;
    }

    /* Get the nb lines */
    nlines = cpl_table_get_nrow(seds);

    /* Get the wavelength as a vector */
    if ((wave = cpl_vector_wrap(nlines,
            cpl_table_get_data_double(seds, "Wavelength"))) == NULL) {
        cpl_msg_error(cpl_func, "Cannot get the Wavelength column");
        cpl_table_delete(seds);
        return NULL;
    }

    /* Get the SED as a vector */
    if ((sed = cpl_vector_wrap(nlines,
            cpl_table_get_data_double(seds, sptype))) == NULL) {
        cpl_msg_error(cpl_func, "Cannot get the SED column");
        cpl_table_delete(seds);
        cpl_vector_unwrap(wave);
        return NULL;
    }
    tmp = cpl_bivector_wrap_vectors(wave, sed);

    /* Create the output bivector */
    out = cpl_bivector_duplicate(tmp);

    /* Free */
    cpl_bivector_unwrap_vectors(tmp);
    cpl_vector_unwrap(wave);
    cpl_vector_unwrap(sed);
    cpl_table_delete(seds);

    /* Return  */
    return out;
}
/**@}*/
