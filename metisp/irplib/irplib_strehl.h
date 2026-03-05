/* $Id: irplib_strehl.h,v 1.12 2009-06-29 14:32:53 kmirny Exp $
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
 * $Author: kmirny $
 * $Date: 2009-06-29 14:32:53 $
 * $Revision: 1.12 $
 * $Name: not supported by cvs2svn $
 */

#ifndef IRPLIB_STREHL_H
#define IRPLIB_STREHL_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>

/*-----------------------------------------------------------------------------
                                   Define
 -----------------------------------------------------------------------------*/

#ifndef IRPLIB_STREHL_BORDER
#define IRPLIB_STREHL_BORDER        5
#endif

/*----------------------------------------------------------------------------*/
/**
   @hideinitializer
   @ingroup irplib_strehl
   @brief  The diameter of the primary mirror, [m]
   @see irplib_strehl_compute()
 */
/*----------------------------------------------------------------------------*/
#define IRPLIB_STREHL_M1                       8.0
/*----------------------------------------------------------------------------*/
/**
   @hideinitializer
   @ingroup irplib_strehl
   @brief  The diameter of the secondary mirror, [m]
   @see irplib_strehl_compute()
 */
/*----------------------------------------------------------------------------*/
#define IRPLIB_STREHL_M2                       1.1
/*----------------------------------------------------------------------------*/
/**
   @hideinitializer
   @ingroup irplib_strehl
   @brief  The size of the internally used PSF-image, [pixel]
   @see irplib_strehl_compute()
 */
/*----------------------------------------------------------------------------*/
#define IRPLIB_STREHL_BOX_SIZE                 64

/*----------------------------------------------------------------------------*/
/**
   @hideinitializer
   @ingroup irplib_strehl
   @brief  The radius of the star, [Arcseconds]
   @see irplib_strehl_compute()
 */
/*----------------------------------------------------------------------------*/
#define IRPLIB_STREHL_STAR_RADIUS              2.0
/*----------------------------------------------------------------------------*/
/**
   @hideinitializer
   @ingroup irplib_strehl
   @brief  The inner radius of the noise-estimation region, [Arcseconds]
   @see irplib_strehl_compute()
 */
/*----------------------------------------------------------------------------*/
#define IRPLIB_STREHL_BACKGROUND_R1            2.0
/*----------------------------------------------------------------------------*/
/**
   @hideinitializer
   @ingroup irplib_strehl
   @brief  The outer radius of the noise-estimation region, [Arcseconds]
   @see irplib_strehl_compute()
 */
/*----------------------------------------------------------------------------*/
#define IRPLIB_STREHL_BACKGROUND_R2            3.0

typedef enum {
    IRPLIB_BG_METHOD_AVER_REJ,
    IRPLIB_BG_METHOD_MEDIAN
} irplib_strehl_bg_method;

/*-----------------------------------------------------------------------------
                               Function prototypes
 -----------------------------------------------------------------------------*/

cpl_error_code irplib_strehl_compute(const cpl_image *, double, double, double,
                                     double, double, int, double, double,
                                     double, double, double, int, int,
                                     double *, double *, double *, double *,
                                     double *, double *, double *, double *);

double irplib_strehl_disk_flux(const cpl_image *, double, double, double,
                               double);

double irplib_strehl_ring_background(const cpl_image *, double, double, double,
                                     double, irplib_strehl_bg_method);

cpl_image * irplib_strehl_generate_psf(double, double, double, double, double,
                                       int);
cpl_error_code irplib_strehl_disk_max(const cpl_image *, double, double,
                                             double, double *);
#endif
