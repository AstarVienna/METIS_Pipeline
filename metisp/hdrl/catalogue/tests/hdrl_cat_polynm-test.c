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

#include <cpl_test.h>

#include "../hdrl_cat_polynm.h"


int main(void)
{
	cpl_test_init(PACKAGE_BUGREPORT,CPL_MSG_WARNING);


	/* Initialize */
    double x[] = { 1. ,   3. ,   5. ,  -10.};
    double y[] = {-0.5, -27.5, -86.5, -424.};

    double coefs[3] = {1., 2.5, -4.};
    double poly[3];

    /* Do the test */
    hdrl_polynm(y, x, 4, poly, 3, 0);

    /* Check the results */
    cpl_test_rel(poly[0], coefs[0], 0.01);
    cpl_test_rel(poly[1], coefs[1], 0.01);
    cpl_test_rel(poly[2], coefs[2], 0.01);


    return cpl_test_end(0);
}
