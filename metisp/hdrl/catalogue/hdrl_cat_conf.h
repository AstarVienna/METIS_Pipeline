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

#ifndef HDRL_CATALOGUE_CONF_H
#define HDRL_CATALOGUE_CONF_H


#include "hdrl_cat_def.h"
#include "hdrl_cat_casu.h"


cpl_error_code hdrl_catalogue_conf(
		hdrl_casu_fits *infile, hdrl_casu_fits *conf, cpl_size ipix,
		double threshold, cpl_size icrowd, double rcore,
		cpl_size bkg_subtr, cpl_size nbsize,
		cpl_size cattype, double filtfwhm, double gain,
		double saturation, hdrl_casu_result *res);


#endif /* HDRL_CATALOGUE_CONF_H */
