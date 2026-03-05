/* $Id: irplib_match_cats-test.c,v 1.2 2013-01-29 08:43:33 jtaylor Exp $
 *
 * This file is part of the ESO Common Pipeline Library
 * Copyright (C) 2001-2008 European Southern Observatory
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
 * $Author: jtaylor $
 * $Date: 2013-01-29 08:43:33 $
 * $Revision: 1.2 $
 * $Name: not supported by cvs2svn $
 */

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cpl_test.h>

#include "irplib_match_cats.h"

/*-----------------------------------------------------------------------------
                                   Static functions
 -----------------------------------------------------------------------------*/
static void irplib_match_cats_all_test(void);

/*-----------------------------------------------------------------------------
                                  Main
 -----------------------------------------------------------------------------*/
int main (void)
{

    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    irplib_match_cats_all_test();

    return cpl_test_end(0);
}

static void irplib_match_cats_all_test(void)
{
    cpl_table   **  catalogues;
    int             nsource_per_cat = 9;
    int             ntotal_sources = nsource_per_cat + 2 + 3;
    int             ncat = 5;
    int             mincat_match = 2;
    int             icat;
    int             iobj;
    cpl_table   *   matches;
    int             imatch;

    /* Create the catalogues */
    catalogues = cpl_malloc(ncat * sizeof(cpl_table *));
    for(icat = 0; icat < ncat; icat++)
    {
        catalogues[icat] = cpl_table_new(nsource_per_cat);
        cpl_table_new_column(catalogues[icat],"X_POS",CPL_TYPE_DOUBLE);
        cpl_table_new_column(catalogues[icat],"Y_POS",CPL_TYPE_DOUBLE);
    }
    for(iobj = 0 ; iobj < ntotal_sources; ++iobj)
    {
        double x,y;
        x = ((double)rand()/(double)RAND_MAX) * 1000;
        y = ((double)rand()/(double)RAND_MAX) * 1000;
        cpl_msg_warning(__func__,"obj %d x %f y %f", iobj, x, y);
        for(icat = 0; icat < ncat; icat++)
        {
            if(icat == 0 && iobj >= 2 && iobj <= nsource_per_cat +1)
            {
                cpl_table_set_double(catalogues[icat], "X_POS", iobj - 2 , x);
                cpl_table_set_double(catalogues[icat], "Y_POS", iobj - 2 , y);
            }
            if(icat == 1 && iobj >= 3 && iobj <= nsource_per_cat+2)
            {
                cpl_table_set_double(catalogues[icat], "X_POS", iobj - 3 , x);
                cpl_table_set_double(catalogues[icat], "Y_POS", iobj - 3 , y);
            }
            if(icat == 2 && iobj >= 1 && iobj <= nsource_per_cat)
            {
                cpl_table_set_double(catalogues[icat], "X_POS", iobj - 1 , x);
                cpl_table_set_double(catalogues[icat], "Y_POS", iobj - 1 , y);
            }
            if(icat == 3 && iobj >= 5 && iobj <= nsource_per_cat+4)
            {
                cpl_table_set_double(catalogues[icat], "X_POS", iobj - 5 , x);
                cpl_table_set_double(catalogues[icat], "Y_POS", iobj - 5 , y);
            }
            if(icat == 4 && iobj <= nsource_per_cat-1)
            {
                cpl_table_set_double(catalogues[icat], "X_POS", iobj    , x);
                cpl_table_set_double(catalogues[icat], "Y_POS", iobj    , y);
            }
            if(icat >= 5 && iobj <= nsource_per_cat-1)
            {
                cpl_table_set_double(catalogues[icat], "X_POS", iobj    , x);
                cpl_table_set_double(catalogues[icat], "Y_POS", iobj    , y);
            }
        }
    }


    /* Match the catalogues */
    matches = irplib_match_cats(catalogues, ncat, mincat_match, 
                                irplib_match_cats_match_condition);
    
    /* Output the matches */
    cpl_msg_warning(__func__,"Final matches:");
    for(imatch = 0; imatch < cpl_table_get_nrow(matches); ++imatch)
    {
        for(icat = 0; icat< ncat; ++icat)
        {
            printf(" %d ",cpl_array_get_int
                   (cpl_table_get_array
                    (matches, "MATCHING_SETS",imatch),icat, NULL));
        }
        printf("\n");
    }
    

    cpl_test_error(CPL_ERROR_NONE);
    //cpl_test_leq(ra1 - 0.00, DBL_EPSILON);


    /* Free */
    for(icat = 0; icat < ncat; icat++)
        cpl_table_delete(catalogues[icat]);
    cpl_free(catalogues);
    cpl_table_delete(matches);

}
