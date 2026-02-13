/*
 * This file is part of the HDRL
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

#ifndef HDRL_RANDOM_H
#define HDRL_RANDOM_H

#include <cpl.h>
#include <stdint.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
  Experimental declarations - can be used, but no guarantees on api stability
 -----------------------------------------------------------------------------*/
#if defined HDRL_USE_EXPERIMENTAL || defined HDRL_USE_PRIVATE
/* currently intended for unit/regression testing purposes only */
typedef struct hdrl_random_state_ hdrl_random_state;

hdrl_random_state * hdrl_random_state_new(int type, uint64_t * seed);
void hdrl_random_state_delete(hdrl_random_state * state);

int64_t hdrl_random_uniform_int64(hdrl_random_state * state,
                                   int64_t minval, int64_t maxval);
double hdrl_random_uniform_double(hdrl_random_state * state,
                                  double minval, double maxval);
uint64_t hdrl_random_poisson(hdrl_random_state * state, double lam);
double hdrl_random_normal(hdrl_random_state * state, double mean, double sigma);

#endif

CPL_END_DECLS

#endif 
