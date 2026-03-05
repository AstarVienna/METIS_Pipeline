/*
 * This file is part of the METIS Pipeline
 * Copyright (C) 2002-2017 European Southern Observatory
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

#include "metis_dfs.h"

/*----------------------------------------------------------------------------*/
/**
 * @defgroup metis_dfs_test  Unit test of metis_dfs
 *
 */
/*----------------------------------------------------------------------------*/

/**@{*/
/*----------------------------------------------------------------------------*/
/**
  @brief    Unit test of metis_dfs_set_groups
 */
/*----------------------------------------------------------------------------*/
static void test_set_groups(void)
{
    /* Simulate data */
    const char *const filename[] = {"raw1.fits",
                                    "raw2.fits",
                                    "calib.fits"};
    const char *const tag[] = {METIS_RAW,
                               METIS_RAW,
                               METIS_CALIB_FLAT};
    cpl_frame_group const expected_group[] = {CPL_FRAME_GROUP_RAW,
                                              CPL_FRAME_GROUP_RAW,
                                              CPL_FRAME_GROUP_CALIB};
    const size_t N = sizeof(filename) / sizeof(filename[0]);

    cpl_frameset *frames = cpl_frameset_new();
    cpl_error_code code;

    cpl_test_eq(sizeof(tag) / sizeof(tag[0]), N);
    cpl_test_eq(sizeof(expected_group) / sizeof(expected_group[0]), N);
    cpl_test_nonnull(frames);

    /* Test with invalid input */
    code = metis_dfs_set_groups(NULL);
    cpl_test_eq_error(code, CPL_ERROR_NULL_INPUT);

    /* Test with valid input */

    /* Call the function - first with an empty frameset */
    code = metis_dfs_set_groups(frames);
    cpl_test_eq_error(code, CPL_ERROR_NONE);

    for (size_t i = 0; i < N; i++) {
        cpl_frame *frame = cpl_frame_new();

        code = cpl_frame_set_filename(frame, filename[i]);
        cpl_test_eq_error(code, CPL_ERROR_NONE);
        code = cpl_frame_set_tag(frame, tag[i]);
        cpl_test_eq_error(code, CPL_ERROR_NONE);
        code = cpl_frameset_insert(frames, frame);
        cpl_test_eq_error(code, CPL_ERROR_NONE);
    }

    /* Call the function */
    code = metis_dfs_set_groups(frames);
    cpl_test_eq_error(code, CPL_ERROR_NONE);

    /* Verify results */
    for (size_t i = 0; i < N; i++) {
        const cpl_frame *frame = cpl_frameset_get_position_const(frames, i);

        cpl_test_nonnull(frame);

        cpl_test_eq(cpl_frame_get_group(frame), expected_group[i]);
    }

    cpl_frameset_delete(frames);

    return;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Unit tests of metis_dfs module
 */
/*----------------------------------------------------------------------------*/

int main(void)
{
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    test_set_groups();

    return cpl_test_end(0);
}

/**@}*/
