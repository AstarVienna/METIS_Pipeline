/*
igam.c from torch-cephes which is a BSD licensed redistribution of cephes [0]
Minor modifications to use C99 lgamma and removed K&R syntax.

[0] https://github.com/jucor/torch-cephes/blob/master/LICENSE.txt

Copyright 1984, 1987, 1995, 2000 by Stephen L. Moshier

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the organization nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 *      Incomplete gamma integral  ->  igam.c
 *
 *
 *
 * SYNOPSIS:
 *
 * double a, x, y, igam();
 *
 * y = igam( a, x );
 *
 * DESCRIPTION:
 *
 * The function is defined by
 *
 *                           x
 *                            -
 *                   1       | |  -t  a-1
 *  igam(a,x)  =   -----     |   e   t   dt.
 *                  -      | |
 *                 | (a)    -
 *                           0
 *
 *
 * In this implementation both arguments must be positive.
 * The integral is evaluated by either a power series or
 * continued fraction expansion, depending on the relative
 * values of a and x.
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      0,30       200000       3.6e-14     2.9e-15
 *    IEEE      0,100      300000       9.9e-14     1.5e-14
 */

/*
 *      Complemented incomplete gamma integral --> igamc()
 *
 *
 *
 * SYNOPSIS:
 *
 * double a, x, y, igamc();
 *
 * y = igamc( a, x );
 *
 * DESCRIPTION:
 *
 * The function is defined by
 *
 *
 *  igamc(a,x)   =   1 - igam(a,x)
 *
 *                            inf.
 *                              -
 *                     1       | |  -t  a-1
 *               =   -----     |   e   t   dt.
 *                    -      | |
 *                   | (a)    -
 *                             x
 *
 *
 * In this implementation both arguments must be positive.
 * The integral is evaluated by either a power series or
 * continued fraction expansion, depending on the relative
 * values of a and x.
 *
 * ACCURACY:
 *
 * Tested at random a, x.
 *                a         x                      Relative error:
 * arithmetic   domain   domain     # trials      peak         rms
 *    IEEE     0.5,100   0,100      200000       1.9e-14     1.7e-15
 *    IEEE     0.01,0.5  0,100      200000       1.4e-13     1.6e-15
 */

/*
 * Cephes Math Library Release 2.8:  June, 2000
 * Copyright 1985, 1987, 2000 by Stephen L. Moshier
 */

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include <math.h>
#include <float.h>

double igam(double, double);
double igamc(double, double);

#define MACHEP DBL_EPSILON
#define MAXLOG log(FLT_MAX)
#define mtherr(x, y)

static double big    = 4.503599627370496e15;
static double biginv = 2.22044604925031308085e-16;

double igamc(double a, double x)
{
	double ans, ax, c, r, t, y, z;
	double pkm1, pkm2, qkm1, qkm2;

	if ((x < 0) || ( a <= 0)) {
		mtherr("igamc", DOMAIN);
		return(NAN);
	}

	if ((x < 1.0) || (x < a)) {
		return(1. - igam(a,x));
	}
	ax = a * log(x) - x - lgamma(a);

	if (ax < -MAXLOG) {
		mtherr("igamc", UNDERFLOW);
		return(0.);
	}
	ax = exp(ax);

	/* continued fraction */
	y    = 1. - a;
	z    = x + y + 1.;
	c    = 0.;
	pkm2 = 1.;
	qkm2 = x;
	pkm1 = x + 1.;
	qkm1 = z * x;
	ans  = pkm1/qkm1;

	do {
		c += 1.;
		y += 1.;
		z += 2.;

		double yc = y * c;
		double pk = pkm1 * z - pkm2 * yc;
		double qk = qkm1 * z - qkm2 * yc;

		if (qk != 0) {
			r = pk / qk;
			t = fabs((ans - r) / r);
			ans = r;
		} else {
			t = 1.;
		}

		pkm2 = pkm1;
		pkm1 = pk;
		qkm2 = qkm1;
		qkm1 = qk;

		if (fabs(pk) > big) {
			pkm2 *= biginv;
			pkm1 *= biginv;
			qkm2 *= biginv;
			qkm1 *= biginv;
		}

	} while(t > MACHEP);

	return( ans * ax );
}


/* left tail of incomplete gamma function:
*
*          inf.    k
*  a  -x   -       x
*  x  e    >   ----------
*          -     -
*          k=0   | (a+k+1)
*
*/

double igam(double a, double x)
{
	double ans, ax, c, r;

	/* Check zero integration limit first */
	if (x == 0) {
		return(0.);
	}

	if ((x < 0) || (a <= 0)) {
		mtherr("igam", DOMAIN);
		return(NAN);
	}

	if ((x > 1.) && (x > a)) {
		return(1. - igamc(a,x));
	}

	/* Compute  x**a * exp(-x) / gamma(a)  */
	ax = a * log(x) - x - lgamma(a);
	if (ax < -MAXLOG) {
		mtherr("igam", UNDERFLOW);
		return(0.);
	}
	ax = exp(ax);

	/* power series */
	r   = a;
	c   = 1.;
	ans = 1.;

	do {
		r   += 1.;
		c   *= x / r;
		ans += c;

	} while(c/ans > MACHEP);

	return(ans * ax / a);
}
