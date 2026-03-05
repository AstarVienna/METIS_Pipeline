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

#ifndef HDRL_TABLE_H
#define HDRL_TABLE_H


#include "hdrl_cat_def.h"


#define NCOLS  63     /* Number of columns in the table */


cpl_error_code hdrl_tabinit(
		ap_t *ap, cpl_size *xcol, cpl_size *ycol, hdrl_catalogue_options cattype,
		cpl_table **tab, hdrl_casu_result *res);

cpl_error_code hdrl_do_seeing(
		ap_t *ap, cpl_size nobjects, cpl_table *tab);

cpl_error_code hdrl_process_results(
		ap_t *ap, double gain, cpl_size *nobjects, cpl_table *tab, hdrl_casu_result *res);


#endif /* HDRL_TABLE_H */
