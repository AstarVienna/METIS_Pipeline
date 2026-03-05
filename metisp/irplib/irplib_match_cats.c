/* $Id: irplib_match_cats.c,v 1.10 2009-12-18 10:44:48 cgarcia Exp $
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
 * $Date: 2009-12-18 10:44:48 $
 * $Revision: 1.10 $
 * $Name: not supported by cvs2svn $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "irplib_match_cats.h"

#include <math.h>

/*-----------------------------------------------------------------------------
                                   Defines
 -----------------------------------------------------------------------------*/

#define FILENAME_SZBUF 1024

/*----------------------------------------------------------------------------*/
/* Private functions
 */
/*----------------------------------------------------------------------------*/

cpl_error_code irplib_match_cats_get_all_matching_pairs
(cpl_table ** catalogues,
 int          ncats,
 cpl_table  * matching_sets,
 int (*binary_match_condition)
   (cpl_table * catalogue1,
    cpl_table * catalogue2,
    int         iobj1,
    int         iobj2)  );

cpl_error_code irplib_match_cats_get_all_matches_cresc
(cpl_table ** catalogues,
 cpl_array  * cat_index_begin,
 cpl_array  * cats_idx_set,
 int          mincat_match,
 cpl_table * matching_sets);

cpl_error_code irplib_match_cats_iterate_on_cat
(cpl_table ** catalogues,
 cpl_array  * cats_idx_set,
 int          icat_iterate,
 cpl_array  * valid_iobjs,
 int          mincat_match,
 cpl_table  * matching_sets,
 cpl_table  * less_minmatch_sets);

cpl_error_code irplib_match_cats_filter_obj_to_iter
(cpl_array * cats_idx_set,
 int         order_begin,
 cpl_table * matches_set,
 cpl_array * excluded_objs,
 int         itercat_nobj);

int irplib_match_cats_match_condition
(cpl_table ** catalogues,
 int       *  cats_idx_set_ptr,
 int          ncats);

int irplib_match_count_nonmatched
(int * cats_idx_set_ptr,
 int   ncats);

int irplib_nCombinations;
int irplib_nFilter;

/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_cat   Functions for matching of catalogues
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*---------------------------------------------------------------------------*/
/**
 * @brief
 *       Finds all the objects that appear at least in some of the catalogues
 * @param catalogues
 *       All the catalogues
 * @param ncats
 *       Number of catalogues
 * @return
 *   The matching table if sucess, NULL otherwise.
 *
 *
 *   Implementation notes:
 *
 *   cat_indexing_order tells you in which order the catalogues are being
 *   iterated. For example cat_indexing_order[2] gives the index of the
 *   catalogue that is being iterated in the 3rd postion.
 *
 *
*/
/*---------------------------------------------------------------------------*/
cpl_table * irplib_match_cat_pairs
(cpl_table ** catalogues,
 int          ncats,
 int (*binary_match_condition)
   (cpl_table * catalogue1,
    cpl_table * catalogue2,
    int         iobj1,
    int         iobj2)  )
{
    cpl_table *  matching_sets;

    //Initialize the solution
    matching_sets = cpl_table_new(0);
    cpl_table_new_column_array(matching_sets, "MATCHING_SETS",
                               CPL_TYPE_INT, ncats);

    irplib_match_cats_get_all_matching_pairs
        (catalogues, ncats, matching_sets, binary_match_condition);
    
    return matching_sets;
}

cpl_error_code irplib_match_cats_get_all_matching_pairs
(cpl_table ** catalogues,
 int          ncats,
 cpl_table  * matching_sets,
 int (*binary_match_condition)
   (cpl_table * catalogue1,
    cpl_table * catalogue2,
    int         iobj1,
    int         iobj2)  )
{
    int icat1;
    int icat2;
    
    irplib_nCombinations = 0;
    irplib_nFilter = 0;

    for(icat1 = 0; icat1 < ncats ; ++icat1)
        for(icat2 = icat1 + 1 ; icat2 < ncats ; ++icat2)
        {
            int iobj1;
            int iobj2;
            int nobj1;
            int nobj2;
            
            nobj1 = cpl_table_get_nrow(catalogues[icat1]);
            nobj2 = cpl_table_get_nrow(catalogues[icat2]);

            for(iobj1 = 0; iobj1 < nobj1 ; ++iobj1)
                for(iobj2 = 0 ; iobj2 < nobj2 ; ++iobj2)
                {
                    ++irplib_nCombinations;
                    if(binary_match_condition(catalogues[icat1],
                                              catalogues[icat2],
                                              iobj1, iobj2))
                    {
                        cpl_array  * cats_idx_set;
                        int          icat;
                        
                        ++irplib_nFilter;
                        cats_idx_set = cpl_array_new(ncats, CPL_TYPE_INT);
                        for(icat = 0; icat < ncats; ++icat)
                        {
                            if(icat == icat1)
                                cpl_array_set_int(cats_idx_set, icat, iobj1);
                            else if(icat == icat2)
                                cpl_array_set_int(cats_idx_set, icat, iobj2);
                            else
                                cpl_array_set_int(cats_idx_set, icat, -1);
                        }
                        
                        cpl_table_set_size(matching_sets,
                                           cpl_table_get_nrow(matching_sets)+1);
                        cpl_table_set_array(matching_sets,"MATCHING_SETS",
                                            cpl_table_get_nrow(matching_sets)-1,
                                            cats_idx_set);
                        cpl_array_delete(cats_idx_set);
                    }
                }
        }
    
    return CPL_ERROR_NONE;
}

/**@}*/

