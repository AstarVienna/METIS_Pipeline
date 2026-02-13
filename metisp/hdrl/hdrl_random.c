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
 *
 *
 *
 * uniform_double, poisson, normal partially based on numpy/random/mtrand
 * Copyright 2005 Robert Kern (robert.kern@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 *
 * PCG generator implementation is licensed under Apache License 2.0
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <hdrl_random.h>
#include <stdint.h>
#include <math.h>

/*-----------------------------------------------------------------------------
                                   Functions
 -----------------------------------------------------------------------------*/

struct hdrl_random_state_{
    uint64_t state;
    uint64_t inc;
    uint64_t has_normal;
    double normal;
};

/* ---------------------------------------------------------------------------*/
/**
 * @brief generate uniform 64 bit random numbers
 *
 * @param rng random number generator state structure
 *
 * @return uniformly distributed 32 bit unsigned integer
 */
/* ---------------------------------------------------------------------------*/
static uint32_t hdrl_random_uniform_uint32(hdrl_random_state * rng)
{
    /* from:
     * PCG: A Family of Simple Fast Space-Efficient Statistically Good
     * Algorithms for Random Number Generation */
    uint64_t oldstate = rng->state;
    // Advance internal state
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc|1);
    // Calculate output function (XSH RR), uses old state for max ILP
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief create random number generator state
 *
 * @param type type, currently needs to be 1 for pcg generator
 * @param seed seed array, length depends on used generator, if NULL uses
 *             rand() to seed
 *             for pcg: two integers state and streamid
 *
 * @return random number generator state structure, must be deleted with
 *         hdrl_random_state_delete
 */
/* ---------------------------------------------------------------------------*/
hdrl_random_state * hdrl_random_state_new(int type, uint64_t * seed)
{
    cpl_error_ensure(type == 1, CPL_ERROR_UNSUPPORTED_MODE,
                     return NULL, "type needs to be 1");
    hdrl_random_state * state = cpl_calloc(sizeof(*state), 1);
    uint64_t s1 = seed ? seed[0] : (uint64_t)rand();
    uint64_t s2 = seed ? seed[1] : (uint64_t)rand();
    state->state = 0;
    state->inc = s2;
    hdrl_random_uniform_uint32(state);
    state->state += s1;
    hdrl_random_uniform_uint32(state);
    return state;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief delete random number generator state structure
 *
 * @param state
 */
/* ---------------------------------------------------------------------------*/
void hdrl_random_state_delete(hdrl_random_state * state)
{
    cpl_free(state);
}

static uint64_t hdrl_random_uniform_uint64_full(hdrl_random_state * state)
{
    return ((uint64_t)hdrl_random_uniform_uint32(state) << 32) |
            (uint64_t)hdrl_random_uniform_uint32(state);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief generatore uniformly distributed 64 bit integers within range
 *
 * @param state   rng state
 * @param minval  minimum value, inclusive
 * @param maxval  maximum value, inclusive
 *
 * @return uniformly distributed 64 bit integer within range
 */
/* ---------------------------------------------------------------------------*/
int64_t
hdrl_random_uniform_int64(hdrl_random_state * state,
                          int64_t minval, int64_t maxval)
{
    uint64_t res, mask;
    if (maxval < minval) {
        cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                              "maximum value smaller than minimum value");
        return 0;
    }
    uint64_t umaxval = (uint64_t)maxval - (uint64_t)minval;
    if (umaxval == 0) {
        return 0;
    }

    /* create all 1 bitmask larger than maxval */
    mask = umaxval;
    mask |= mask >> 1;
    mask |= mask >> 2;
    mask |= mask >> 4;
    mask |= mask >> 8;
    mask |= mask >> 16;
    mask |= mask >> 32;

    /* draw numbers until the value is smaller than maxval */
    while (1) {
        res = hdrl_random_uniform_uint64_full(state) & mask;
        if (res <= umaxval) {
            break;
        }
    }
    return (int64_t)((uint64_t)minval + res);
}

static double hdrl_random_uniform_double_one(hdrl_random_state * state)
{
    /* shifts : 67108864 = 0x4000000, 9007199254740992 = 0x20000000000000 */
    uint64_t a = hdrl_random_uniform_uint32(state) >> 5;
    uint64_t b = hdrl_random_uniform_uint32(state) >> 6;
    return (a * 67108864.0 + b) / 9007199254740992.0;
}


/* ---------------------------------------------------------------------------*/
/**
 * @brief generatore uniformly distributed double within range
 *
 * @param state   rng state
 * @param minval  minimum value, inclusive
 * @param maxval  maximum value, inclusive
 *
 * @return uniformly distributed double within range
 */
/* ---------------------------------------------------------------------------*/
double hdrl_random_uniform_double(hdrl_random_state * state,
                                  double minval, double maxval)
{
    double scale = fabs(maxval - minval);
    return minval + scale * hdrl_random_uniform_double_one(state);
}

static uint64_t hdrl_random_poisson_low(hdrl_random_state * state, double lam)
{
    const double explam = exp(-lam);
    double prod = hdrl_random_uniform_double_one(state);
    uint64_t r = 0;

    while (prod > explam) {
        r++;
        prod *= hdrl_random_uniform_double_one(state);
    }

    return r;
}

/*
 * The transformed rejection method for generating Poisson random variables
 * W. Hoermann
 * Insurance: Mathematics and Economics 12, 39-45 (1993)
 */
static uint64_t hdrl_random_poisson_ptrs(hdrl_random_state * state, double lam)
{
    const double slam     =  sqrt(lam);
    const double loglam   =  log(lam);
    const double b        =  0.931  + 2.53    * slam;
    const double a        = -0.059  + 0.02483 * b;
    const double invalpha =  1.1239 + 1.1328  / (b - 3.4);
    const double vr       =  0.9277 - 3.6224  / (b - 2. );

    while (1) {

        double U  = hdrl_random_uniform_double_one(state) - 0.5;
        double V  = hdrl_random_uniform_double_one(state);

        double us = 0.5 - fabs(U);
        cpl_boolean eval = (us >= 0.07);

        double aux = 2 * a / us;
		int64_t k = (long)floor((aux + b) * U + lam + 0.43);

		if(eval && V <= vr) {
			return k;
		}

		if (k < 0 || (us < 0.013 && V > us) ) {
			continue;
		}

		double lgamk = lgamma(k + 1);

		if ((log(V) + log(invalpha) - log(a/(us*us)+b)) <=
			(-lam + k * loglam - lgamk)) {
			return k;
		}
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief generatore poisson distributed values
 *
 * @param state   rng state
 * @param lam     lambda/mean parameter of poisson distribution
 *
 * @return poisson distributed value
 */
/* ---------------------------------------------------------------------------*/
uint64_t hdrl_random_poisson(hdrl_random_state * state, double lam)
{
    if (lam >= 10.) {
        return hdrl_random_poisson_ptrs(state, lam);
    }
    else if (lam == 0.) {
        return 0;
    }
    else if (lam < 0.) {
        cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                              "lam must not be negative");
        return 0;
    }
    else {
        return hdrl_random_poisson_low(state, lam);
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief generatore normal distributed values
 *
 * @param state   rng state
 * @param mean    mean/location parameter of normal distribution
 * @param sigma   sigma/scale parameter of normal distribution
 *
 * @return normal distributed value
 */
/* ---------------------------------------------------------------------------*/
double hdrl_random_normal(hdrl_random_state * state, double mean, double sigma)
{
    if (sigma < 0.) {
        cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                              "sigma must not be negative");
        return 0;
    }
    if (state->has_normal) {
        state->has_normal = 0;
        return mean + sigma * state->normal;
    }
    else {
        double f, x1, x2, r;

        do {
            x1 = 2.0 * hdrl_random_uniform_double_one(state) - 1.0;
            x2 = 2.0 * hdrl_random_uniform_double_one(state) - 1.0;
            r = x1 * x1 + x2 * x2;
        }
        while (r >= 1.0 || r == 0.0);

        /* Box-Muller transform */
        f = sqrt(-2.0*log(r)/r);
        /* Keep second value for next call */
        state->normal = f * x1;
        state->has_normal = 1;
        return mean + sigma * f * x2;
    }
}
