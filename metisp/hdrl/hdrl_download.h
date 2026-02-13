/*
 * hdrl_download.h
 *
 *  Created on: Jan 28, 2022
 *      Author: agabasch
 */

/*
 * This file is part of HDRL
 * Copyright (C) 2016 European Southern Observatory
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#ifndef HDRL_DOWNLOAD_H_
#define HDRL_DOWNLOAD_H_

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                                Functions prototypes
 -----------------------------------------------------------------------------*/

char *
hdrl_download_url_to_buffer(const char * url, size_t * data_length);

cpl_error_code
hdrl_download_url_to_file (const char * url, const char * filename);


CPL_END_DECLS


#endif /* HDRL_DOWNLOAD_H_ */
