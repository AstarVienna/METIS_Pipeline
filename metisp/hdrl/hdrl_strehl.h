/*
 * This file is part of the HDRL
 * Copyright (C) 2014 European Southern Observatory
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

#ifndef HDRL_STREHL_H
#define HDRL_STREHL_H

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/
#include "hdrl_types.h"
#include "hdrl_image.h"
#include <cpl.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                            Functions prototypes
 -----------------------------------------------------------------------------*/

typedef struct {
    /* computed strehl value and its propagated error */
    hdrl_value strehl_value;
    /* computed x and y position of the star peak */
    double star_x, star_y;
    /* star peak and its propagated error */
    hdrl_value star_peak;
    /* star flux and its propagated error */
    hdrl_value star_flux;
    /* star background and its propagated error */
    hdrl_value star_background;
    /* star background error estimated from image
     * on normal data sqrt(pi / 2) larger that star_background error as it is
     * estimated via a median */
    double computed_background_error;
    /* number of pixels used for background estimation */
    size_t nbackground_pixels;
} hdrl_strehl_result;

hdrl_parameter * hdrl_strehl_parameter_create(double wavelength,
        double m1_radius, double m2_radius,
        double pixel_scale_x, double pixel_scale_y, double flux_radius,
        double bkg_radius_low, double bkg_radius_high);

cpl_parameterlist * hdrl_strehl_parameter_create_parlist(
        const char      *   base_context,
        const char      *   prefix,
        hdrl_parameter  *   par);

hdrl_parameter * hdrl_strehl_parameter_parse_parlist(
        const cpl_parameterlist *   parlist,
        const char              *   prefix);

hdrl_strehl_result
hdrl_strehl_compute(const hdrl_image * himg, hdrl_parameter* params);

#if defined HDRL_USE_EXPERIMENTAL || defined HDRL_USE_PRIVATE

cpl_boolean hdrl_strehl_parameter_check(const hdrl_parameter *);

double hdrl_strehl_parameter_get_wavelength(const hdrl_parameter * p);
double hdrl_strehl_parameter_get_m1(const hdrl_parameter * p);
double hdrl_strehl_parameter_get_m2(const hdrl_parameter * p);
double hdrl_strehl_parameter_get_pixel_scale_x(const hdrl_parameter * p);
double hdrl_strehl_parameter_get_pixel_scale_y(const hdrl_parameter * p);
double hdrl_strehl_parameter_get_flux_radius(const hdrl_parameter * p);
double hdrl_strehl_parameter_get_bkg_radius_low(const hdrl_parameter * p);
double hdrl_strehl_parameter_get_bkg_radius_high(const hdrl_parameter * p);

#endif

CPL_END_DECLS

#endif
