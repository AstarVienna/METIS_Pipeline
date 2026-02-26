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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                    Includes
 -----------------------------------------------------------------------------*/

#include "hdrl_random.h"

#include <cpl.h>
#include <stdint.h>
#include <math.h>



/*----------------------------------------------------------------------------*/
/**
 * @defgroup hdrl_random_test   
            Testing of hdrl_random module
 */
/*----------------------------------------------------------------------------*/

cpl_error_code test_basic(void)
{
    hdrl_random_state * state = hdrl_random_state_new(1, NULL);
    const size_t N = 10000;

    for (size_t i = 0; i < N; i++) {
        int64_t r = hdrl_random_uniform_int64(state, 0, 100);
        cpl_test_lt(r, 101);
        r = hdrl_random_uniform_int64(state, 1000, 2000);
        cpl_test_lt(r, 2001);
        cpl_test_lt(999, r);

        r = hdrl_random_uniform_int64(state, -(1ll<<55ll), 1ll<<55ll);
        cpl_test_lt(r, (1ll<<55ll) + 1);
        cpl_test_lt(-(1ll<<55ll) + 1ll, r);

        r = hdrl_random_uniform_int64(state, -(1ll<<55ll), 0);
        cpl_test_lt(r, 1);
        cpl_test_lt(-(1ll<<55ll) + 1ll, r);

        double rd = hdrl_random_uniform_double(state, -5., 2.);
        cpl_test_lt(rd, nextafter(2., 1));
        cpl_test_lt(nextafter(-5., -1), rd);
    }

    int buf[N];
    double bufd[N];
    for (size_t i = 0; i < N; i++) {
        buf[i] = (int)hdrl_random_poisson(state, 300.);
        bufd[i] = hdrl_random_normal(state, 3.5, 1.5);
    }
    cpl_image * iimg = cpl_image_wrap_int(N, 1, buf);
    cpl_image * dimg = cpl_image_wrap_double(N, 1, bufd);
    cpl_test_abs(cpl_image_get_mean(iimg), 300, 1);
    cpl_test_abs(cpl_image_get_stdev(iimg), sqrt(300), 0.5);
    cpl_test_abs(cpl_image_get_mean(dimg), 3.5, 0.1);
    cpl_test_abs(cpl_image_get_stdev(dimg), 1.5, 0.1);

    cpl_image_unwrap(iimg);
    cpl_image_unwrap(dimg);
    hdrl_random_state_delete(state);

    uint64_t seed[2] = {1342, 232};
    state = hdrl_random_state_new(1, seed);
    hdrl_random_state_delete(state);

    return cpl_error_get_code();
}


/*----------------------------------------------------------------------------*/
/**
 @brief   Unit tests of hdrl_image
 **/
/*----------------------------------------------------------------------------*/
int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    test_basic();

    return cpl_test_end(0);
}
