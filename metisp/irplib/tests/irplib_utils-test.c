/*                                                                            *
 *   This file is part of the ESO IRPLIB package                              *
 *   Copyright (C) 2004,2005 European Southern Observatory                    *
 *                                                                            *
 *   This library is free software; you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License as published by     *
 *   the Free Software Foundation; either version 2 of the License, or        *
 *   (at your option) any later version.                                      *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program; if not, write to the Free Software              *
 *   Foundation, 51 Franklin St, Fifth Floor, Boston, MA  02111-1307  USA     *
 *                                                                            */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <irplib_utils.h>
#include <string.h>
#include <float.h>
#include <stdint.h>

/*-----------------------------------------------------------------------------
                                   Function prototypes
 -----------------------------------------------------------------------------*/

static IRPLIB_UTIL_SET_ROW(my_table_set_row);
static IRPLIB_UTIL_CHECK(my_table_check);

static void test_irplib_image_split(void);
static void test_irplib_dfs_table_convert(void);
static void test_irplib_table_read_from_frameset(void);
static void test_irplib_isnaninf(void);
static void bench_irplib_image_split(int, int);
static void frameset_sort_test(int sz);
static void test_irplib_aligned_alloc(void);

/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_utils_test Testing of the IRPLIB utilities
 */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/**
   @brief   Unit tests of utils module
**/
/*----------------------------------------------------------------------------*/

int main(void)
{
    /* Initialize CPL for unit testing */
    cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

    irplib_trace();

    test_irplib_isnaninf();

    test_irplib_dfs_table_convert();
    test_irplib_table_read_from_frameset();

    test_irplib_image_split();

    frameset_sort_test(122); /* test even */
    frameset_sort_test(127); /* test odd  */

    test_irplib_aligned_alloc();

    if (cpl_msg_get_level() <= CPL_MSG_INFO) {
        bench_irplib_image_split(1024, 100);
    } else {
        bench_irplib_image_split(64, 1);
    }

    return cpl_test_end(0);
}


/*----------------------------------------------------------------------------*/
/**
   @internal
   @brief   Test of irplib_isinf and irplib_isnan
**/
/*----------------------------------------------------------------------------*/
static void test_irplib_isnaninf(void)
{
    double infinity = DBL_MAX * DBL_MAX;
    double number[] = {17, 0};

    /* The computation  oo/oo  must result in NaN according to
       the IEEE 754 standard. However, some GCC 4.x versions erroneously
       optimize this to 1.

       Alternatively, a NaN could be produced using a IEEE 754 defined bit
       pattern. But that is likely to depend on the machine's word size.
       Therefore this test is disabled.

       double not_a_number = infinity / infinity;
    */

    cpl_test_zero(irplib_isnan(infinity) );
    /* cpl_test(  irplib_isnan(not_a_number) ); */
    cpl_test_zero(irplib_isnan(number[0]) );
    cpl_test_zero(irplib_isnan(number[1]) );

    cpl_test(  irplib_isinf(infinity) );
    /* cpl_test_zero(irplib_isinf(not_a_number) ); */
    cpl_test_zero(irplib_isinf(number[0]) );
    cpl_test_zero(irplib_isinf(number[1]) );

    return;
}


static void test_irplib_aligned_alloc(void)
{
    void * ptr = NULL;
    size_t alignment[] = {2, 4, 8, 16, 32, 64, 128, 4096};
    char zero[100] = {0};
    size_t i;

    for (i = 0; i < sizeof(alignment)/sizeof(alignment[0]); i++) {
        ptr = irplib_aligned_malloc(alignment[i], 100);
        cpl_test_nonnull(ptr);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_eq(((intptr_t)ptr % alignment[i]), 0);
        irplib_aligned_free(ptr);
        cpl_test_error(CPL_ERROR_NONE);
    }
    /* invalid alignment */
    ptr = irplib_aligned_malloc(5, 100);
    cpl_test_null(ptr);
    irplib_aligned_free(NULL);

    for (i = 0; i < sizeof(alignment)/sizeof(alignment[0]); i++) {
        ptr = irplib_aligned_calloc(alignment[i], 100, 1);
        cpl_test_nonnull(ptr);
        cpl_test_error(CPL_ERROR_NONE);
        cpl_test_eq(((intptr_t)ptr % alignment[i]), 0);
        cpl_test_eq(memcmp(ptr, zero, 100), 0);
        irplib_aligned_free(ptr);
        cpl_test_error(CPL_ERROR_NONE);
    }
    /* invalid alignment */
    ptr = irplib_aligned_calloc(5, 100, 1);
    cpl_test_null(ptr);
    irplib_aligned_free(NULL);
}


static cpl_boolean my_table_set_row(cpl_table * self,
                                    const char * line,
                                    int irow,
                                    const cpl_frame * rawframe,
                                    const cpl_parameterlist * parlist)
{
    char str[32] = "";
    double val = 0.0;

    cpl_ensure(self     != NULL, CPL_ERROR_NULL_INPUT, CPL_FALSE);
    cpl_ensure(line     != NULL, CPL_ERROR_NULL_INPUT, CPL_FALSE);
    cpl_ensure(irow     >= 0,    CPL_ERROR_ILLEGAL_INPUT, CPL_FALSE);
    cpl_ensure(rawframe != NULL, CPL_ERROR_NULL_INPUT, CPL_FALSE);
    cpl_ensure(parlist  != NULL, CPL_ERROR_NULL_INPUT, CPL_FALSE);

    cpl_test_assert(sscanf(line, "%31s %16lf", &str[0], &val) != EOF);

    cpl_test_assert(cpl_table_set_string(self, "MYLABEL1", (cpl_size)irow, str)
                    == CPL_ERROR_NONE);
    cpl_test_assert(cpl_table_set_double(self, "MYLABEL2", (cpl_size)irow, val)
                    == CPL_ERROR_NONE);

    return CPL_TRUE;

}

static cpl_error_code my_table_check(cpl_table * self,
                                     const cpl_frameset * useframes,
                                     const cpl_parameterlist * parlist)
{

    cpl_ensure_code(self      != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(useframes != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(parlist   != NULL, CPL_ERROR_NULL_INPUT);

    return CPL_ERROR_NONE;

}


/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief  Test irplib_dfs_table_convert()

**/
/*----------------------------------------------------------------------------*/
static void test_irplib_dfs_table_convert(void)
{

    /* FIXME: Room for improvement... */
    cpl_error_code error
        = irplib_dfs_table_convert(NULL, NULL, NULL, 1024, '#',
                                   NULL, NULL, NULL, NULL, NULL, NULL,
                                   NULL, NULL, NULL, my_table_set_row,
                                   my_table_check);

    cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);
}


/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief  Test irplib_table_read_from_frameset()

**/
/*----------------------------------------------------------------------------*/
static void test_irplib_table_read_from_frameset(void)
{
    cpl_size initial_failed = cpl_test_get_failed();
    cpl_error_code error;
    FILE * file;
    const char * filename1 = "dummy_input_file_for_irplib_utils_test_1.txt";
    const char * filename2 = "dummy_input_file_for_irplib_utils_test_2.txt";
    const int expected_rows = 5;
    cpl_table * self;
    cpl_parameterlist * parlist = cpl_parameterlist_new();
    cpl_frameset * useframes = cpl_frameset_new();
    cpl_frame * frame;

    cpl_test_nonnull(parlist);
    cpl_test_nonnull(useframes);

    error =
        irplib_table_read_from_frameset(NULL, NULL, 1024, '#', NULL,
                                        my_table_set_row);

    cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

    /* Test similar example as indicated in the documentation comments of
     * irplib_table_read_from_frameset.
     * First we need to generate some dummy input files and add their names to
     * the frameset. */
    file = fopen(filename1, "w");
    cpl_test_nonnull(file);
    cpl_test(fprintf(file, "abc 1.2\nde 4.3\nfhij 5.6\n") >= 0);
    (void) fclose(file);
    file = fopen(filename2, "w");
    cpl_test_nonnull(file);
    cpl_test(fprintf(file, "klm -7.8\nnopq 9\n") >= 0);
    (void) fclose(file);

    frame = cpl_frame_new();
    cpl_test_nonnull(frame);
    cpl_test_eq_error(cpl_frame_set_filename(frame, filename1), CPL_ERROR_NONE);
    cpl_test_eq_error(cpl_frame_set_tag(frame, "TEXT"), CPL_ERROR_NONE);
    cpl_test_eq_error(cpl_frame_set_type(frame, CPL_FRAME_TYPE_ANY),
                      CPL_ERROR_NONE);
    cpl_test_eq_error(cpl_frame_set_group(frame, CPL_FRAME_GROUP_RAW),
                      CPL_ERROR_NONE);
    cpl_test_eq_error(cpl_frame_set_level(frame, CPL_FRAME_LEVEL_TEMPORARY),
                      CPL_ERROR_NONE);
    cpl_test_eq_error(cpl_frameset_insert(useframes, frame), CPL_ERROR_NONE);
    frame = cpl_frame_new();
    cpl_test_nonnull(frame);
    cpl_test_eq_error(cpl_frame_set_filename(frame, filename2), CPL_ERROR_NONE);
    cpl_test_eq_error(cpl_frame_set_tag(frame, "TEXT"), CPL_ERROR_NONE);
    cpl_test_eq_error(cpl_frame_set_type(frame, CPL_FRAME_TYPE_ANY),
                      CPL_ERROR_NONE);
    cpl_test_eq_error(cpl_frame_set_group(frame, CPL_FRAME_GROUP_RAW),
                      CPL_ERROR_NONE);
    cpl_test_eq_error(cpl_frame_set_level(frame, CPL_FRAME_LEVEL_TEMPORARY),
                      CPL_ERROR_NONE);
    cpl_test_eq_error(cpl_frameset_insert(useframes, frame), CPL_ERROR_NONE);

    self = cpl_table_new(expected_rows);
    cpl_test_nonnull(self);
    cpl_table_new_column(self, "MYLABEL1", CPL_TYPE_STRING);
    cpl_table_new_column(self, "MYLABEL2", CPL_TYPE_DOUBLE);
    cpl_table_set_column_unit(self, "MYLABEL2", "Some_SI_Unit");

    error = irplib_table_read_from_frameset(self, useframes, 1024, '#', parlist,
                                            my_table_set_row);
    cpl_test_eq_error(error, CPL_ERROR_NONE);

    /* Check parsed table: */
    cpl_test_eq(cpl_table_get_nrow(self), expected_rows);
    cpl_test_eq_string(cpl_table_get_string(self, "MYLABEL1", 0), "abc");
    cpl_test_eq_string(cpl_table_get_string(self, "MYLABEL1", 1), "de");
    cpl_test_eq_string(cpl_table_get_string(self, "MYLABEL1", 2), "fhij");
    cpl_test_eq_string(cpl_table_get_string(self, "MYLABEL1", 3), "klm");
    cpl_test_eq_string(cpl_table_get_string(self, "MYLABEL1", 4), "nopq");
    cpl_test_abs(cpl_table_get_double(self, "MYLABEL2", 0, NULL),
                 1.2, DBL_EPSILON);
    cpl_test_abs(cpl_table_get_double(self, "MYLABEL2", 1, NULL),
                 4.3, DBL_EPSILON);
    cpl_test_abs(cpl_table_get_double(self, "MYLABEL2", 2, NULL),
                 5.6, DBL_EPSILON);
    cpl_test_abs(cpl_table_get_double(self, "MYLABEL2", 3, NULL),
                 -7.8, DBL_EPSILON);
    cpl_test_abs(cpl_table_get_double(self, "MYLABEL2", 4, NULL),
                 9.0, DBL_EPSILON);

    cpl_table_delete(self);
    cpl_parameterlist_delete(parlist);
    cpl_frameset_delete(useframes);

    /* Delete dummy input files if none of these unit tests failed. */
    if (cpl_test_get_failed() == initial_failed) {
        (void) remove(filename1);
        (void) remove(filename2);
    }
}


/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief  Test irplib_image_split()

**/
/*----------------------------------------------------------------------------*/
static void bench_irplib_image_split(int nxy, int nsplit) {

    const double th_low   =  0.0;
    const double th_high  = 50.0;
    const double alt_low  = th_low  - 1.0;
    const double alt_high = th_high + 1.0;
    cpl_image  * test     = cpl_image_new(nxy, nxy, CPL_TYPE_FLOAT);
    double       tsum = 0.0;
    int          i;

    for (i = 0; i < nsplit; i++) {
        double time1;
        const double time0 = cpl_test_get_cputime();
        const cpl_error_code error =
            irplib_image_split(test, NULL, test, NULL,
                               th_low,  CPL_TRUE, th_high, CPL_TRUE,
                               alt_low, alt_high,
                               CPL_TRUE, CPL_FALSE, CPL_TRUE);
        time1 = cpl_test_get_cputime();

        cpl_test_eq_error(error, CPL_ERROR_NONE);

        if (time1 > time0) tsum += time1 - time0;
    }

    cpl_msg_info(cpl_func,"Time to split with image size %d [ms]: %g", nxy,
                 1e3*tsum/nsplit);

    cpl_image_delete(test);

}


/*----------------------------------------------------------------------------*/
/**
  @internal
  @brief  Test error handling for irplib_image_split()

**/
/*----------------------------------------------------------------------------*/
static void test_irplib_image_split(void) {

    const double th_low   =  0.0;
    const double th_high  = 50.0;
    const double alt_low  = th_low  - 1.0;
    const double alt_high = th_high + 1.0;

    cpl_image * test   = cpl_image_new(100, 100, CPL_TYPE_DOUBLE);
    cpl_image * result = cpl_image_new(100, 100, CPL_TYPE_DOUBLE);

    /* Various error conditions */
    cpl_error_code error
        = irplib_image_split(NULL, test, result, test,
                             0.0, CPL_FALSE, 0.0, CPL_FALSE,
                             0.0, 0.0,
                             CPL_FALSE, CPL_FALSE, CPL_FALSE);
    cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);


    error = irplib_image_split(test, NULL, NULL, NULL,
                               th_low,  CPL_TRUE, th_high, CPL_TRUE,
                               alt_low, alt_high,
                               CPL_TRUE, CPL_FALSE, CPL_TRUE);
    cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

    error = irplib_image_split(test, NULL, result, NULL,
                               th_low,  CPL_TRUE, alt_low, CPL_TRUE,
                               alt_low, alt_high,
                               CPL_TRUE, CPL_FALSE, CPL_TRUE);

    cpl_test_eq_error(error, CPL_ERROR_ILLEGAL_INPUT);

    /* Verify against cpl_image_threshold() */
    error = cpl_image_fill_noise_uniform(test, -100.0, 100.0);
    cpl_test_eq_error(error, CPL_ERROR_NONE);

    error = irplib_image_split(test, NULL, result, NULL,
                               th_low,  CPL_TRUE, th_high, CPL_TRUE,
                               alt_low, alt_high,
                               CPL_TRUE, CPL_FALSE, CPL_TRUE);
    cpl_test_eq_error(error, CPL_ERROR_NONE);

    error = cpl_image_threshold(test, th_low, th_high, alt_low, alt_high);
    cpl_test_eq_error(error, CPL_ERROR_NONE);

    error = cpl_image_subtract(result, test);
    cpl_test_eq_error(error, CPL_ERROR_NONE);

    cpl_test_leq(cpl_image_get_absflux(result), DBL_EPSILON);

    cpl_image_delete(test);
    cpl_image_delete(result);

}

static void frameset_sort_test(int sz)
{
    /* 1. create a test frameset - each frame should contain EXPTIME property */
    cpl_frameset * pframeset = cpl_frameset_new();
    int          * idx       = cpl_malloc(sz * sizeof(*idx));
    double       * exptime   = cpl_malloc(sz * sizeof(*exptime));
    cpl_error_code error;
    int            i;

    cpl_test_nonnull(pframeset);

    for (i = 0; i < sz; i++) {
        cpl_frame        * pframe   = cpl_frame_new();
        cpl_propertylist * plist    = cpl_propertylist_new();
        char             * filename = cpl_sprintf("dummyon%d.fits", i);
        const double       value    = (i % 2) > 0 ? i : sz - i - 1;


        cpl_test_nonnull(pframe);
        /* assign exptime; */
        error = cpl_frame_set_filename(pframe, filename);
        cpl_test_eq_error(error, CPL_ERROR_NONE);
        error = cpl_frame_set_tag(pframe, "ON");
        cpl_test_eq_error(error, CPL_ERROR_NONE);
        error = cpl_frame_set_type(pframe, CPL_FRAME_TYPE_IMAGE);
        cpl_test_eq_error(error, CPL_ERROR_NONE);
        error = cpl_frame_set_group(pframe, CPL_FRAME_GROUP_RAW);
        cpl_test_eq_error(error, CPL_ERROR_NONE);

        error = cpl_frameset_insert(pframeset, pframe);
        cpl_test_eq_error(error, CPL_ERROR_NONE);
        error = cpl_propertylist_append_double(plist, "EXPTIME", value);
        cpl_test_eq_error(error, CPL_ERROR_NONE);
        error = cpl_propertylist_save(plist, filename, CPL_IO_CREATE);
        cpl_test_eq_error(error, CPL_ERROR_NONE);

        cpl_propertylist_delete(plist);
        cpl_free(filename);
    }

    error = irplib_frameset_sort(pframeset, idx, exptime);
    cpl_test_eq_error(error, CPL_ERROR_NONE);

    for (i = 0; i < sz; i++) {
        int k = i + 1 - (sz % 2);
        int j = sz -i - 1 ;
        cpl_test_eq(idx[i], (((i + (sz % 2)) % 2)  == 0 ? k : j));
    }

    cpl_free(idx);
    cpl_free(exptime);
    cpl_frameset_delete(pframeset);
    cpl_test_zero(system("rm dummyon*.fits"));
}
