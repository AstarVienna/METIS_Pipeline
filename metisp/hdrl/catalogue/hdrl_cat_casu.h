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

#ifndef HDRL_CASU_H
#define HDRL_CASU_H


#include "hdrl_cat_def.h"


hdrl_casu_fits *   hdrl_casu_fits_wrap(      cpl_image *im);
hdrl_casu_fits *   hdrl_casu_fits_duplicate( hdrl_casu_fits *in);
cpl_propertylist * hdrl_casu_fits_get_ehu(   hdrl_casu_fits *p);
cpl_image *        hdrl_casu_fits_get_image( hdrl_casu_fits *p);
void               hdrl_casu_fits_delete(    hdrl_casu_fits *p);

hdrl_casu_tfits *  hdrl_casu_tfits_wrap(     cpl_table *tab, cpl_propertylist *ehu);
cpl_propertylist * hdrl_casu_tfits_get_ehu(  hdrl_casu_tfits *p);
cpl_table *        hdrl_casu_tfits_get_table(hdrl_casu_tfits *p);
void               hdrl_casu_tfits_delete(   hdrl_casu_tfits *p);

cpl_error_code hdrl_casu_catalogue(
		hdrl_casu_fits *infile, hdrl_casu_fits *conf,
		const cpl_wcs *wcs, cpl_size ipix,
		double threshold, cpl_size icrowd, double rcore,
		cpl_size bkg_subtr, cpl_size nbsize,
		hdrl_catalogue_options cattype,
		double filtfwhm, double gainloc, double saturation,
		hdrl_casu_result *res);


#endif /* HDRL_CASU_H */
