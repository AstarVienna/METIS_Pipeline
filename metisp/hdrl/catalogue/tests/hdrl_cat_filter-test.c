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

#include "../hdrl_cat_filter.h"


#define NUM 100


int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    /* Test 1 */
    double x1[NUM];
    for (cpl_size i = 0; i < NUM; i++) {
        x1[i] = 0.;
    }
    padext(x1, NUM);

    /* Test 2 */
    double x2[NUM];
    for (cpl_size i = 0; i < NUM; i++) {
    	if (i < NUM / 2) {
    		x2[i] = -2000.;
    	} else {
    		x2[i] = 0.;
    	}
    }
    padext(x2, NUM);

    /* Test 3 */
    double x3[NUM];
    for (cpl_size i = 0; i < NUM; i++) {
    	if (i < NUM / 2) {
    		x3[i] = 0.;
    	} else {
    		x3[i] = -2000.;
    	}
    }
    padext(x3, NUM);

    /* Test 4 */
    double x4[NUM];
    for (cpl_size i = 0; i < NUM; i++) {
    	x4[i] = -2000.;
    }
    padext(x4, NUM);


    return cpl_test_end(0);
}
