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

#ifndef HDRL_RADII_H
#define HDRL_RADII_H


#include "hdrl_cat_def.h"


double hdrl_halflight(double rcores[], double cflux[], double halflight, double peak, cpl_size naper);
double hdrl_exprad(   double thresh, double peak, double areal0, double rcores[], cpl_size naper);
double hdrl_kronrad(  double areal0, double rcores[], double cflux[], cpl_size naper);
double hdrl_petrad(   double areal0, double rcores[], double cflux[], cpl_size naper);

void   hdrl_flux(     ap_t *ap, double parm[IMNUM][NPAR], cpl_size nbit, double apers[],
                        double fluxes[], cpl_size nr, double rcores[], double rfluxes[]);


#endif /* HDRL_RADII_H */
