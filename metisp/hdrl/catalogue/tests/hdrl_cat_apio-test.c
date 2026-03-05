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

#include "../hdrl_cat_apio.h"


int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT,CPL_MSG_WARNING);

    /* Create an input apm structure */
    ap_t ap;
    ap.lsiz     = 2048;
    ap.csiz     = 2048;
    ap.inframe  = NULL;
    ap.conframe = NULL;

    /* Initialize */
    hdrl_apinit(&ap);

    /* Test some of the various bits of the structure to see if they are the correct values */
    cpl_test_eq(      ap.maxpa,            1024);
    cpl_test_eq(      ap.maxip,            0);
    cpl_test_nonnull( ap.lastline);
    cpl_test_nonnull( ap.pstack);
    cpl_test_nonnull( ap.parent);
    cpl_test_eq(      ap.pstack[10],       10);
    cpl_test_eq(      ap.parent[10].pnop, -1);
    cpl_test_eq(      ap.parent[10].pnbp, -1);
    cpl_test_eq(      ap.ipstack,          1);
    cpl_test_nonnull( ap.bstack);
    cpl_test_nonnull( ap.blink);
    cpl_test_nonnull( ap.plessey);
    cpl_test_eq(      ap.bstack[10],       10);
    cpl_test_eq(      ap.ibstack,          2);
    cpl_test_eq(      ap.nimages,          0);
    cpl_test_eq(      ap.areal[1],         2 * ap.areal[0]);
    cpl_test_eq(      ap.npl,              ap.lsiz);
    cpl_test_eq(      ap.npl_pix,          0);
    cpl_test_nonnull( ap.plarray);
    cpl_test_eq(      ap.backmap.nby,     -1);
    cpl_test_null(    ap.backmap.bvals);
    cpl_test_null(    ap.indata);
    cpl_test_null(    ap.confdata);

    /* Clean up */
    hdrl_apclose(&ap);


    return cpl_test_end(0);
}
