/* $Id: irplib_mkmaster.h,v 1.3 2011-11-02 13:17:25 amodigli Exp $
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
 * $Author: amodigli $
 * $Date: 2011-11-02 13:17:25 $
 * $Revision: 1.3 $
 * $Name: not supported by cvs2svn $
 */

#ifndef IRPLIB_MKMASTER_H
#define IRPLIB_MKMASTER_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>
#include <irplib_ksigma_clip.h>

cpl_image*
irplib_mkmaster_mean(cpl_imagelist* images,const double kappa, const int nclip, const double tolerance,const double klow,const double khigh,const int niter);

cpl_image* 
irplib_mkmaster_median(cpl_imagelist* images,const double kappa, const int nclip, const double tolerance);

cpl_image *
irplib_mdark_process_chip(const cpl_imagelist *raw_images,
    cpl_propertylist **raw_headers, const cpl_image *master_bias,
    cpl_propertylist *mdark_header, const cpl_parameterlist *parameters,
    const char* recipe_id, cpl_table* qclog, const int do_qc,
    const char* STACK_METHOD, const double STACK_KLOW, const double STACK_KHIGH,
    const int STACK_NITER,
    const int pr_num_x, const int pr_num_y,
    const int pr_box_sx, const int pr_box_sy);
#endif
