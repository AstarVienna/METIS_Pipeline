/*
 * This file is part of the ESO Common Pipeline Library
 * Copyright (C) 2014 European Southern Observatory
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

#include <cpl_test.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <errno.h>
#include "irplib_sdp_spectrum.h"

/*-----------------------------------------------------------------------------
                                  Static functions
 -----------------------------------------------------------------------------*/

static void fill_keywords(irplib_sdp_spectrum *a);
static int test_get_set_functions(void);
static int test_copy_functions(void);
static int test_count_functions(void);
static int test_column_functions(void);
static int test_equal_function(void);
static int test_io_functions(void);
static int test_column_copy_update_functions(void);
static int test_generic_copy_functions(void);
static int test_append_provenance(void);

static cpl_boolean create_file_with_key(cpl_frameset* frames,
                                        const char *filename,
                                        const char *keyword,
                                        const char *value);

/*-----------------------------------------------------------------------------
                                  Main
 -----------------------------------------------------------------------------*/
int main(void)
{
  cpl_test_init(PACKAGE_BUGREPORT, CPL_MSG_WARNING);

  cpl_test_assert(test_get_set_functions() == EXIT_SUCCESS);
  cpl_test_assert(test_copy_functions() == EXIT_SUCCESS);
  cpl_test_assert(test_count_functions() == EXIT_SUCCESS);
  cpl_test_assert(test_column_functions() == EXIT_SUCCESS);
  cpl_test_assert(test_equal_function() == EXIT_SUCCESS);
  cpl_test_assert(test_io_functions() == EXIT_SUCCESS);
  cpl_test_assert(test_column_copy_update_functions() == EXIT_SUCCESS);
  cpl_test_assert(test_generic_copy_functions() == EXIT_SUCCESS);
  cpl_test_assert(test_append_provenance() == EXIT_SUCCESS);

  return cpl_test_end(0);
}


static void fill_keywords(irplib_sdp_spectrum *a)
{
  cpl_test_eq_error(irplib_sdp_spectrum_set_ra(a, 1.23), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_dec(a, 2.34), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_exptime(a, 3.45), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_texptime(a, 5.34), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_timesys(a, "gmt"), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_mjdobs(a, 4.56), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_mjdend(a, 5.67), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_prodlvl(a, 678), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_procsoft(a, "abc"), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_prodcatg(a, "bcd"), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_origin(a, "cde"), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_extobj(a, CPL_TRUE), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_dispelem(a, "def"), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_specsys(a, "efg"), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_progid(a, "fgh"), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_obid(a, 1, 789), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_mepoch(a, CPL_TRUE), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_obstech(a, "ghi"), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_fluxcal(a, "hij"), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_contnorm(a, CPL_TRUE), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_wavelmin(a, 8.90), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_wavelmax(a, 9.01), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_specbin(a, 10.12), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_totflux(a, CPL_TRUE), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_fluxerr(a, 432.19), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_referenc(a, "ijk"), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_specres(a, 23.45), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_specerr(a, 34.56), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_specsye(a, 45.67), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_lamnlin(a, 5678), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_lamrms(a, 67.89), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_gain(a, 78.90), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_detron(a, 89.01), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_effron(a, 90.12), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_snr(a, 93.75), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_ncombine(a, 12345), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_voclass(a, "jkl"), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_vopub(a, "klm"), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_title(a, "lmn"), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_object(a, "mno"), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_aperture(a, 234.56), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_telapse(a, 345.67), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_tmid(a, 456.78), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_specval(a, 567.89), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_specbw(a, 678.90), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_extname(a, "nop"), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_inherit(a, CPL_TRUE), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_nelem(a, 78901), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_tdmin(a, 890.12), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_tdmax(a, 901.23), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_prov(a, 1, "opq"), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_prov(a, 2, "pqr"), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_asson(a, 1, "qrs"), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_assoc(a, 1, "rst"), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_assom(a, 1, "stu"), CPL_ERROR_NONE);
}


static int test_get_set_functions(void)
{
  /* Create a new spectrum structure and set all keywords with dummy values. */
  irplib_sdp_spectrum *a = irplib_sdp_spectrum_new();
  irplib_sdp_spectrum *b;
  cpl_test_assert(a != NULL);
  fill_keywords(a);

  /* Duplicate the spectrum and check if the new spectrum's keywords are the
   * same as for the first spectrum. */
  b = irplib_sdp_spectrum_duplicate(a);
  cpl_test_assert(b != NULL);
  cpl_test_abs(irplib_sdp_spectrum_get_ra(b), 1.23, DBL_EPSILON);
  cpl_test_abs(irplib_sdp_spectrum_get_dec(b), 2.34, DBL_EPSILON);
  cpl_test_abs(irplib_sdp_spectrum_get_exptime(b), 3.45, DBL_EPSILON);
  cpl_test_abs(irplib_sdp_spectrum_get_texptime(b), 5.34, DBL_EPSILON);
  cpl_test_eq_string(irplib_sdp_spectrum_get_timesys(b), "gmt");
  cpl_test_abs(irplib_sdp_spectrum_get_mjdobs(b), 4.56, DBL_EPSILON);
  cpl_test_abs(irplib_sdp_spectrum_get_mjdend(b), 5.67, DBL_EPSILON);
  cpl_test_eq(irplib_sdp_spectrum_get_prodlvl(b), 678);
  cpl_test_eq_string(irplib_sdp_spectrum_get_procsoft(b), "abc");
  cpl_test_eq_string(irplib_sdp_spectrum_get_prodcatg(b), "bcd");
  cpl_test_eq_string(irplib_sdp_spectrum_get_origin(b), "cde");
  cpl_test_eq(irplib_sdp_spectrum_get_extobj(b), CPL_TRUE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_dispelem(b), "def");
  cpl_test_eq_string(irplib_sdp_spectrum_get_specsys(b), "efg");
  cpl_test_eq_string(irplib_sdp_spectrum_get_progid(b), "fgh");
  cpl_test_eq(irplib_sdp_spectrum_get_obid(b, 1), 789);
  cpl_test_eq(irplib_sdp_spectrum_get_mepoch(b), CPL_TRUE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_obstech(b), "ghi");
  cpl_test_eq_string(irplib_sdp_spectrum_get_fluxcal(b), "hij");
  cpl_test_eq(irplib_sdp_spectrum_get_contnorm(b), CPL_TRUE);
  cpl_test_abs(irplib_sdp_spectrum_get_wavelmin(b), 8.90, DBL_EPSILON);
  cpl_test_abs(irplib_sdp_spectrum_get_wavelmax(b), 9.01, DBL_EPSILON);
  cpl_test_abs(irplib_sdp_spectrum_get_specbin(b), 10.12, DBL_EPSILON);
  cpl_test_eq(irplib_sdp_spectrum_get_totflux(b), CPL_TRUE);
  cpl_test_abs(irplib_sdp_spectrum_get_fluxerr(b), 432.19, DBL_EPSILON);
  cpl_test_eq_string(irplib_sdp_spectrum_get_referenc(b), "ijk");
  cpl_test_abs(irplib_sdp_spectrum_get_specres(b), 23.45, DBL_EPSILON);
  cpl_test_abs(irplib_sdp_spectrum_get_specerr(b), 34.56, DBL_EPSILON);
  cpl_test_abs(irplib_sdp_spectrum_get_specsye(b), 45.67, DBL_EPSILON);
  cpl_test_eq(irplib_sdp_spectrum_get_lamnlin(b), 5678);
  cpl_test_abs(irplib_sdp_spectrum_get_lamrms(b), 67.89, DBL_EPSILON);
  cpl_test_abs(irplib_sdp_spectrum_get_gain(b), 78.90, DBL_EPSILON);
  cpl_test_abs(irplib_sdp_spectrum_get_detron(b), 89.01, DBL_EPSILON);
  cpl_test_abs(irplib_sdp_spectrum_get_effron(b), 90.12, DBL_EPSILON);
  cpl_test_abs(irplib_sdp_spectrum_get_snr(b), 93.75, DBL_EPSILON);
  cpl_test_eq(irplib_sdp_spectrum_get_ncombine(b), 12345);
  cpl_test_eq_string(irplib_sdp_spectrum_get_voclass(b), "jkl");
  cpl_test_eq_string(irplib_sdp_spectrum_get_vopub(b), "klm");
  cpl_test_eq_string(irplib_sdp_spectrum_get_title(b), "lmn");
  cpl_test_eq_string(irplib_sdp_spectrum_get_object(b), "mno");
  cpl_test_abs(irplib_sdp_spectrum_get_aperture(b), 234.56, DBL_EPSILON);
  cpl_test_abs(irplib_sdp_spectrum_get_telapse(b), 345.67, DBL_EPSILON);
  cpl_test_abs(irplib_sdp_spectrum_get_tmid(b), 456.78, DBL_EPSILON);
  cpl_test_abs(irplib_sdp_spectrum_get_specval(b), 567.89, DBL_EPSILON);
  cpl_test_abs(irplib_sdp_spectrum_get_specbw(b), 678.90, DBL_EPSILON);
  cpl_test_eq_string(irplib_sdp_spectrum_get_extname(b), "nop");
  cpl_test_eq(irplib_sdp_spectrum_get_inherit(b), CPL_TRUE);
  cpl_test_eq(irplib_sdp_spectrum_get_nelem(b), 78901);
  cpl_test_abs(irplib_sdp_spectrum_get_tdmin(b), 890.12, DBL_EPSILON);
  cpl_test_abs(irplib_sdp_spectrum_get_tdmax(b), 901.23, DBL_EPSILON);
  cpl_test_eq_string(irplib_sdp_spectrum_get_prov(b, 1), "opq");
  cpl_test_eq_string(irplib_sdp_spectrum_get_prov(b, 2), "pqr");
  cpl_test_eq_string(irplib_sdp_spectrum_get_asson(b, 1), "qrs");
  cpl_test_eq_string(irplib_sdp_spectrum_get_assoc(b, 1), "rst");
  cpl_test_eq_string(irplib_sdp_spectrum_get_assom(b, 1), "stu");

  /* Remove the keywords from the second spectrum and check no errors occur. */
  cpl_test_eq_error(irplib_sdp_spectrum_reset_ra(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_dec(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_exptime(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_texptime(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_timesys(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_mjdobs(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_mjdend(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_prodlvl(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_procsoft(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_prodcatg(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_origin(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_extobj(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_dispelem(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_specsys(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_progid(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_obid(b, 1), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_mepoch(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_obstech(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_fluxcal(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_contnorm(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_wavelmin(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_wavelmax(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_specbin(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_totflux(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_fluxerr(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_referenc(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_specres(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_specerr(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_specsye(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_lamnlin(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_lamrms(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_gain(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_detron(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_effron(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_snr(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_ncombine(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_voclass(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_vopub(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_title(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_object(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_aperture(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_telapse(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_tmid(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_specval(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_specbw(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_extname(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_inherit(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_nelem(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_tdmin(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_tdmax(b), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_prov(b, 1), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_prov(b, 2), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_asson(b, 1), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_assoc(b, 1), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_assom(b, 1), CPL_ERROR_NONE);
  cpl_test_error(CPL_ERROR_NONE);

  /* Check that default values are returned for all keywords that were reset. */
  cpl_test(isnan(irplib_sdp_spectrum_get_ra(b)));
  cpl_test(isnan(irplib_sdp_spectrum_get_dec(b)));
  cpl_test(isnan(irplib_sdp_spectrum_get_exptime(b)));
  cpl_test(isnan(irplib_sdp_spectrum_get_texptime(b)));
  cpl_test_null(irplib_sdp_spectrum_get_timesys(b));
  cpl_test(isnan(irplib_sdp_spectrum_get_mjdobs(b)));
  cpl_test(isnan(irplib_sdp_spectrum_get_mjdend(b)));
  cpl_test_eq(irplib_sdp_spectrum_get_prodlvl(b), -1);
  cpl_test_null(irplib_sdp_spectrum_get_procsoft(b));
  cpl_test_null(irplib_sdp_spectrum_get_prodcatg(b));
  cpl_test_null(irplib_sdp_spectrum_get_origin(b));
  cpl_test_eq(irplib_sdp_spectrum_get_extobj(b), CPL_FALSE);
  cpl_test_null(irplib_sdp_spectrum_get_dispelem(b));
  cpl_test_null(irplib_sdp_spectrum_get_specsys(b));
  cpl_test_null(irplib_sdp_spectrum_get_progid(b));
  cpl_test_eq(irplib_sdp_spectrum_get_obid(b, 1), -1);
  cpl_test_eq(irplib_sdp_spectrum_get_mepoch(b), CPL_FALSE);
  cpl_test_null(irplib_sdp_spectrum_get_obstech(b));
  cpl_test_null(irplib_sdp_spectrum_get_fluxcal(b));
  cpl_test_eq(irplib_sdp_spectrum_get_contnorm(b), CPL_FALSE);
  cpl_test(isnan(irplib_sdp_spectrum_get_wavelmin(b)));
  cpl_test(isnan(irplib_sdp_spectrum_get_wavelmax(b)));
  cpl_test(isnan(irplib_sdp_spectrum_get_specbin(b)));
  cpl_test_eq(irplib_sdp_spectrum_get_totflux(b), CPL_FALSE);
  cpl_test(isnan(irplib_sdp_spectrum_get_fluxerr(b)));
  cpl_test_null(irplib_sdp_spectrum_get_referenc(b));
  cpl_test(isnan(irplib_sdp_spectrum_get_specres(b)));
  cpl_test(isnan(irplib_sdp_spectrum_get_specerr(b)));
  cpl_test(isnan(irplib_sdp_spectrum_get_specsye(b)));
  cpl_test_eq(irplib_sdp_spectrum_get_lamnlin(b), -1);
  cpl_test(isnan(irplib_sdp_spectrum_get_lamrms(b)));
  cpl_test(isnan(irplib_sdp_spectrum_get_gain(b)));
  cpl_test(isnan(irplib_sdp_spectrum_get_detron(b)));
  cpl_test(isnan(irplib_sdp_spectrum_get_effron(b)));
  cpl_test(isnan(irplib_sdp_spectrum_get_snr(b)));
  cpl_test_eq(irplib_sdp_spectrum_get_ncombine(b), -1);
  cpl_test_null(irplib_sdp_spectrum_get_voclass(b));
  cpl_test_null(irplib_sdp_spectrum_get_vopub(b));
  cpl_test_null(irplib_sdp_spectrum_get_title(b));
  cpl_test_null(irplib_sdp_spectrum_get_object(b));
  cpl_test(isnan(irplib_sdp_spectrum_get_aperture(b)));
  cpl_test(isnan(irplib_sdp_spectrum_get_telapse(b)));
  cpl_test(isnan(irplib_sdp_spectrum_get_tmid(b)));
  cpl_test(isnan(irplib_sdp_spectrum_get_specval(b)));
  cpl_test(isnan(irplib_sdp_spectrum_get_specbw(b)));
  cpl_test_null(irplib_sdp_spectrum_get_extname(b));
  cpl_test_eq(irplib_sdp_spectrum_get_inherit(b), CPL_FALSE);
  cpl_test_eq(irplib_sdp_spectrum_get_nelem(b), 0);
  cpl_test(isnan(irplib_sdp_spectrum_get_tdmin(b)));
  cpl_test(isnan(irplib_sdp_spectrum_get_tdmax(b)));
  cpl_test_null(irplib_sdp_spectrum_get_prov(b, 1));
  cpl_test_null(irplib_sdp_spectrum_get_prov(b, 2));
  cpl_test_null(irplib_sdp_spectrum_get_asson(b, 1));
  cpl_test_null(irplib_sdp_spectrum_get_assoc(b, 1));
  cpl_test_null(irplib_sdp_spectrum_get_assom(b, 1));

  irplib_sdp_spectrum_delete(b);
  irplib_sdp_spectrum_delete(a);

  return cpl_test_get_failed() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}


static int test_count_functions(void)
{
  irplib_sdp_spectrum *a = irplib_sdp_spectrum_new();
  cpl_test_assert(a != NULL);

  /* Test that the irplib_sdp_spectrum_count_* functions return the correct
   * values as we add keywords to the spectrum object. */
  cpl_test_eq(irplib_sdp_spectrum_count_obid(a), 0);
  irplib_sdp_spectrum_set_obid(a, 1, 1234);
  cpl_test_eq(irplib_sdp_spectrum_count_obid(a), 1);
  irplib_sdp_spectrum_set_obid(a, 2, 2345);
  cpl_test_eq(irplib_sdp_spectrum_count_obid(a), 2);
  irplib_sdp_spectrum_set_obid(a, 3, 3456);
  cpl_test_eq(irplib_sdp_spectrum_count_obid(a), 3);
  irplib_sdp_spectrum_set_obid(a, 901, 9012);
  cpl_test_eq(irplib_sdp_spectrum_count_obid(a), 4);

  cpl_test_eq(irplib_sdp_spectrum_count_prov(a), 0);
  irplib_sdp_spectrum_set_prov(a, 1, "a");
  cpl_test_eq(irplib_sdp_spectrum_count_prov(a), 1);
  irplib_sdp_spectrum_set_prov(a, 2, "b");
  cpl_test_eq(irplib_sdp_spectrum_count_prov(a), 2);
  irplib_sdp_spectrum_set_prov(a, 3, "c");
  cpl_test_eq(irplib_sdp_spectrum_count_prov(a), 3);
  irplib_sdp_spectrum_set_prov(a, 901, "d");
  cpl_test_eq(irplib_sdp_spectrum_count_prov(a), 4);

  cpl_test_eq(irplib_sdp_spectrum_count_asson(a), 0);
  irplib_sdp_spectrum_set_asson(a, 1, "a");
  cpl_test_eq(irplib_sdp_spectrum_count_asson(a), 1);
  irplib_sdp_spectrum_set_asson(a, 2, "b");
  cpl_test_eq(irplib_sdp_spectrum_count_asson(a), 2);
  irplib_sdp_spectrum_set_asson(a, 3, "c");
  cpl_test_eq(irplib_sdp_spectrum_count_asson(a), 3);
  irplib_sdp_spectrum_set_asson(a, 901, "d");
  cpl_test_eq(irplib_sdp_spectrum_count_asson(a), 4);

  cpl_test_eq(irplib_sdp_spectrum_count_assoc(a), 0);
  irplib_sdp_spectrum_set_assoc(a, 1, "a");
  cpl_test_eq(irplib_sdp_spectrum_count_assoc(a), 1);
  irplib_sdp_spectrum_set_assoc(a, 2, "b");
  cpl_test_eq(irplib_sdp_spectrum_count_assoc(a), 2);
  irplib_sdp_spectrum_set_assoc(a, 3, "c");
  cpl_test_eq(irplib_sdp_spectrum_count_assoc(a), 3);
  irplib_sdp_spectrum_set_assoc(a, 901, "d");
  cpl_test_eq(irplib_sdp_spectrum_count_assoc(a), 4);

  cpl_test_eq(irplib_sdp_spectrum_count_assom(a), 0);
  irplib_sdp_spectrum_set_assom(a, 1, "a");
  cpl_test_eq(irplib_sdp_spectrum_count_assom(a), 1);
  irplib_sdp_spectrum_set_assom(a, 2, "b");
  cpl_test_eq(irplib_sdp_spectrum_count_assom(a), 2);
  irplib_sdp_spectrum_set_assom(a, 3, "c");
  cpl_test_eq(irplib_sdp_spectrum_count_assom(a), 3);
  irplib_sdp_spectrum_set_assom(a, 901, "d");
  cpl_test_eq(irplib_sdp_spectrum_count_assom(a), 4);

  /* Now remove the keywords one by one and check if we still get the correct
   * counts. First remove the middle keywords to check how the count routine
   * deals with gaps in the keyword series. */
  irplib_sdp_spectrum_reset_obid(a, 901);
  cpl_test_eq(irplib_sdp_spectrum_count_obid(a), 3);
  irplib_sdp_spectrum_reset_obid(a, 2);
  cpl_test_eq(irplib_sdp_spectrum_count_obid(a), 2);
  irplib_sdp_spectrum_reset_obid(a, 1);
  cpl_test_eq(irplib_sdp_spectrum_count_obid(a), 1);
  irplib_sdp_spectrum_reset_obid(a, 3);
  cpl_test_eq(irplib_sdp_spectrum_count_obid(a), 0);

  irplib_sdp_spectrum_reset_prov(a, 901);
  cpl_test_eq(irplib_sdp_spectrum_count_prov(a), 3);
  irplib_sdp_spectrum_reset_prov(a, 2);
  cpl_test_eq(irplib_sdp_spectrum_count_prov(a), 2);
  irplib_sdp_spectrum_reset_prov(a, 1);
  cpl_test_eq(irplib_sdp_spectrum_count_prov(a), 1);
  irplib_sdp_spectrum_reset_prov(a, 3);
  cpl_test_eq(irplib_sdp_spectrum_count_prov(a), 0);

  irplib_sdp_spectrum_reset_asson(a, 901);
  cpl_test_eq(irplib_sdp_spectrum_count_asson(a), 3);
  irplib_sdp_spectrum_reset_asson(a, 2);
  cpl_test_eq(irplib_sdp_spectrum_count_asson(a), 2);
  irplib_sdp_spectrum_reset_asson(a, 1);
  cpl_test_eq(irplib_sdp_spectrum_count_asson(a), 1);
  irplib_sdp_spectrum_reset_asson(a, 3);
  cpl_test_eq(irplib_sdp_spectrum_count_asson(a), 0);

  irplib_sdp_spectrum_reset_assoc(a, 901);
  cpl_test_eq(irplib_sdp_spectrum_count_assoc(a), 3);
  irplib_sdp_spectrum_reset_assoc(a, 2);
  cpl_test_eq(irplib_sdp_spectrum_count_assoc(a), 2);
  irplib_sdp_spectrum_reset_assoc(a, 1);
  cpl_test_eq(irplib_sdp_spectrum_count_assoc(a), 1);
  irplib_sdp_spectrum_reset_assoc(a, 3);
  cpl_test_eq(irplib_sdp_spectrum_count_assoc(a), 0);

  irplib_sdp_spectrum_reset_assom(a, 901);
  cpl_test_eq(irplib_sdp_spectrum_count_assom(a), 3);
  irplib_sdp_spectrum_reset_assom(a, 2);
  cpl_test_eq(irplib_sdp_spectrum_count_assom(a), 2);
  irplib_sdp_spectrum_reset_assom(a, 1);
  cpl_test_eq(irplib_sdp_spectrum_count_assom(a), 1);
  irplib_sdp_spectrum_reset_assom(a, 3);
  cpl_test_eq(irplib_sdp_spectrum_count_assom(a), 0);

  irplib_sdp_spectrum_delete(a);
  return cpl_test_get_failed() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}


static int test_column_functions(void)
{
  const cpl_array *data;
  irplib_sdp_spectrum *a = irplib_sdp_spectrum_new();
  cpl_test_assert(a != NULL);

  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 0);

  /* Test simple column creation function. */
  cpl_test_eq_error(irplib_sdp_spectrum_new_column(a, "A", CPL_TYPE_INT),
                    CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 1);
  cpl_test_eq(irplib_sdp_spectrum_get_nelem(a), 0);
  cpl_test_eq(irplib_sdp_spectrum_get_column_type(a, "A"),
              CPL_TYPE_INT | CPL_TYPE_POINTER);
  cpl_test_null(irplib_sdp_spectrum_get_column_unit(a, "A"));
  cpl_test_nonnull(irplib_sdp_spectrum_get_column_format(a, "A"));
  cpl_test_null(irplib_sdp_spectrum_get_column_tutyp(a, "A"));
  cpl_test_null(irplib_sdp_spectrum_get_column_tucd(a, "A"));
  cpl_test_null(irplib_sdp_spectrum_get_column_tcomm(a, "A"));
  cpl_test_null(irplib_sdp_spectrum_get_column_data(a, "A"));
  cpl_test_error(CPL_ERROR_NONE);

  /* Test setter functions. First set to a known values. Check that its correct.
   * Then set the values to NULL and see that we get that again without error.
   */
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_tutyp(a, "A", "x"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "x");
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_tucd(a, "A", "y"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "y");
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_tcomm(a, "A", "z"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tcomm(a, "A"), "z");

  cpl_test_eq_error(irplib_sdp_spectrum_set_column_tutyp(a, "A", NULL),
                    CPL_ERROR_NONE);
  cpl_test_null(irplib_sdp_spectrum_get_column_tutyp(a, "A"));
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_tucd(a, "A", NULL),
                    CPL_ERROR_NONE);
  cpl_test_null(irplib_sdp_spectrum_get_column_tucd(a, "A"));
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_tcomm(a, "A", NULL),
                    CPL_ERROR_NONE);
  cpl_test_null(irplib_sdp_spectrum_get_column_tcomm(a, "A"));
  cpl_test_error(CPL_ERROR_NONE);

  /* Test column deletion. */
  cpl_test_eq_error(irplib_sdp_spectrum_delete_column(a, "A"), CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 0);
  cpl_test_null(irplib_sdp_spectrum_get_column_tutyp(a, "A"));
  cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
  cpl_test_null(irplib_sdp_spectrum_get_column_tucd(a, "A"));
  cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
  cpl_test_null(irplib_sdp_spectrum_get_column_tcomm(a, "A"));
  cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
  cpl_test_null(irplib_sdp_spectrum_get_column_data(a, "A"));
  cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);

  /* Test the add function with all extra parameters NULL. */
  cpl_test_eq_error(
      irplib_sdp_spectrum_add_column(a, "A", CPL_TYPE_FLOAT,
                                     NULL, NULL, NULL, NULL, NULL),
      CPL_ERROR_NONE
    );
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 1);
  cpl_test_eq(irplib_sdp_spectrum_get_nelem(a), 0);
  cpl_test_eq(irplib_sdp_spectrum_get_column_type(a, "A"),
              CPL_TYPE_FLOAT | CPL_TYPE_POINTER);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_unit(a, "A"), " ");
  cpl_test_nonnull(irplib_sdp_spectrum_get_column_format(a, "A"));
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "");
  cpl_test_null(irplib_sdp_spectrum_get_column_tcomm(a, "A"));
  cpl_test_nonnull(irplib_sdp_spectrum_get_column_data(a, "A"));
  cpl_test_error(CPL_ERROR_NONE);

  cpl_test_eq_error(irplib_sdp_spectrum_delete_column(a, "A"), CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 0);

  /* Add a column and check the properties are correct. */
  cpl_test_eq_error(
      irplib_sdp_spectrum_add_column(a, "A", CPL_TYPE_DOUBLE, "s", "1E",
                                     NULL, NULL, NULL),
      CPL_ERROR_NONE
    );
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 1);
  cpl_test_eq(irplib_sdp_spectrum_get_nelem(a), 0);
  cpl_test_eq(irplib_sdp_spectrum_get_column_type(a, "A"),
              CPL_TYPE_DOUBLE | CPL_TYPE_POINTER);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_unit(a, "A"), "s");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_format(a, "A"), "1E");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "");
  cpl_test_null(irplib_sdp_spectrum_get_column_tcomm(a, "A"));
  cpl_test_nonnull(irplib_sdp_spectrum_get_column_data(a, "A"));

  /* Check setting of the column keywords. */
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_unit(a, "A", "sec"),
                    CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_format(a, "A", "2E"),
                    CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_tutyp(a, "A", "t1"),
                    CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_tucd(a, "A", "u1"),
                    CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_tcomm(a, "A", "cmnt 1"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_unit(a, "A"), "sec");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_format(a, "A"), "2E");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "t1");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "u1");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tcomm(a, "A"), "cmnt 1");
  data = irplib_sdp_spectrum_get_column_data(a, "A");
  cpl_test_nonnull(data);
  cpl_test_eq(cpl_array_get_size(data), 0);

  /* Resize data arrays and check they were updated. */
  cpl_test_eq_error(irplib_sdp_spectrum_set_nelem(a, 10), CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_nelem(a), 10);
  data = irplib_sdp_spectrum_get_column_data(a, "A");
  cpl_test_nonnull(data);
  cpl_test_eq(cpl_array_get_size(data), 10);

  /* Check for correct error response when trying to use a missing column. */
  cpl_test_eq(irplib_sdp_spectrum_get_column_type(a, "C"), CPL_TYPE_INVALID);
  cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
  cpl_test_null(irplib_sdp_spectrum_get_column_unit(a, "C"));
  cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
  cpl_test_null(irplib_sdp_spectrum_get_column_format(a, "C"));
  cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
  cpl_test_null(irplib_sdp_spectrum_get_column_tutyp(a, "C"));
  cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
  cpl_test_null(irplib_sdp_spectrum_get_column_tucd(a, "C"));
  cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
  cpl_test_null(irplib_sdp_spectrum_get_column_tcomm(a, "C"));
  cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
  cpl_test_null(irplib_sdp_spectrum_get_column_data(a, "C"));
  cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);

  /* Add another column and check its properties are correct. */
  cpl_test_eq_error(
      irplib_sdp_spectrum_add_column(a, "B", CPL_TYPE_INT, "adu", "1J",
                                     NULL, NULL, NULL),
      CPL_ERROR_NONE
    );
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 2);
  cpl_test_eq(irplib_sdp_spectrum_get_column_type(a, "B"),
              CPL_TYPE_INT | CPL_TYPE_POINTER);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_unit(a, "B"), "adu");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_format(a, "B"), "1J");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "B"), "");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "B"), "");
  cpl_test_null(irplib_sdp_spectrum_get_column_tcomm(a, "B"));
  data = irplib_sdp_spectrum_get_column_data(a, "B");
  cpl_test_nonnull(data);
  cpl_test_eq(cpl_array_get_size(data), 10);

  /* Resize data arrays again and check they were updated. */
  cpl_test_eq_error(irplib_sdp_spectrum_set_nelem(a, 5), CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_nelem(a), 5);
  data = irplib_sdp_spectrum_get_column_data(a, "A");
  cpl_test_nonnull(data);
  cpl_test_eq(cpl_array_get_size(data), 5);
  data = irplib_sdp_spectrum_get_column_data(a, "B");
  cpl_test_nonnull(data);
  cpl_test_eq(cpl_array_get_size(data), 5);

  irplib_sdp_spectrum_delete(a);
  return cpl_test_get_failed() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}


static int test_equal_function(void)
{
  cpl_array *data;
  double datapoints1[5] = {1.2, 2.3, 3.4, 4.5, 5.6};
  double datapoints2[5] = {1.1, 2.4, 5.4, 4.6, 3.6};
  irplib_sdp_spectrum *a, *b;

  /* Setup two empty spectra, and compare.
   * Note: we compare both orders for the arguments, i.e. a, b and then b, a to
   * make sure the behaviour is symmetric as expected. */
  a = irplib_sdp_spectrum_new();
  cpl_test_assert(a != NULL);
  b = irplib_sdp_spectrum_new();
  cpl_test_assert(b != NULL);
  cpl_test_eq(irplib_sdp_spectrum_equal(a, b, CPL_FALSE), CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(b, a, CPL_FALSE), CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(a, b, CPL_TRUE), CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(b, a, CPL_TRUE), CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);
  irplib_sdp_spectrum_delete(b);
  irplib_sdp_spectrum_delete(a);

  /* Setup a spectrum, duplicated it and compare. */
  a = irplib_sdp_spectrum_new();
  cpl_test_assert(a != NULL);
  fill_keywords(a);
  cpl_test_eq_error(irplib_sdp_spectrum_set_nelem(a, 5), CPL_ERROR_NONE);
  cpl_test_eq_error(
      irplib_sdp_spectrum_add_column(a, "A", CPL_TYPE_DOUBLE, "s", "1E",
                                     NULL, NULL, NULL),
      CPL_ERROR_NONE
    );
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_tutyp(a, "A", "t1"),
                    CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_tucd(a, "A", "u1"),
                    CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_tcomm(a, "A", "c1"),
                    CPL_ERROR_NONE);
  data = cpl_array_wrap_double(datapoints1, 5);
  cpl_test_nonnull(data);
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_data(a, "A", data),
                    CPL_ERROR_NONE);
  cpl_array_unwrap(data);

  b = irplib_sdp_spectrum_duplicate(a);
  cpl_test_assert(b != NULL);
  cpl_test_eq(irplib_sdp_spectrum_equal(a, b, CPL_FALSE), CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(b, a, CPL_FALSE), CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(a, b, CPL_TRUE), CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(b, a, CPL_TRUE), CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);

  /* Make a change to b's column keyword values and see if we still get the
   * correct results. */
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_tutyp(b, "A", "T3"),
                    CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(a, b, CPL_FALSE), CPL_FALSE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(b, a, CPL_FALSE), CPL_FALSE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(a, b, CPL_TRUE), CPL_FALSE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(b, a, CPL_TRUE), CPL_FALSE);
  cpl_test_error(CPL_ERROR_NONE);

  /* Check result if a column is added to b. */
  irplib_sdp_spectrum_delete(b);
  b = irplib_sdp_spectrum_duplicate(a);
  cpl_test_eq_error(
      irplib_sdp_spectrum_add_column(b, "B", CPL_TYPE_DOUBLE, "adu", "1J",
                                     NULL, NULL, NULL),
      CPL_ERROR_NONE
    );
  cpl_test_eq(irplib_sdp_spectrum_equal(a, b, CPL_FALSE), CPL_FALSE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(b, a, CPL_FALSE), CPL_FALSE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(a, b, CPL_TRUE), CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(b, a, CPL_TRUE), CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);

  /* Check if changes in the column data points are picked up. */
  irplib_sdp_spectrum_delete(b);
  b = irplib_sdp_spectrum_duplicate(a);
  data = cpl_array_wrap_double(datapoints2, 5);
  cpl_test_nonnull(data);
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_data(b, "A", data),
                    CPL_ERROR_NONE);
  cpl_array_unwrap(data);
  cpl_test_eq(irplib_sdp_spectrum_equal(a, b, CPL_FALSE), CPL_FALSE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(b, a, CPL_FALSE), CPL_FALSE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(a, b, CPL_TRUE), CPL_FALSE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(b, a, CPL_TRUE), CPL_FALSE);
  cpl_test_error(CPL_ERROR_NONE);

  /* Check results if the NELEM values are different, but we still have the same
   * data point values for the overlapping part. */
  irplib_sdp_spectrum_delete(b);
  b = irplib_sdp_spectrum_duplicate(a);
  cpl_test_eq_error(irplib_sdp_spectrum_set_nelem(a, 4), CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(a, b, CPL_FALSE), CPL_FALSE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(b, a, CPL_FALSE), CPL_FALSE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(a, b, CPL_TRUE), CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(b, a, CPL_TRUE), CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);

  /* Check result if a primary keyword value is different. */
  irplib_sdp_spectrum_delete(b);
  b = irplib_sdp_spectrum_duplicate(a);
  cpl_test_eq_error(irplib_sdp_spectrum_set_dec(b, 999), CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(a, b, CPL_FALSE), CPL_FALSE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(b, a, CPL_FALSE), CPL_FALSE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(a, b, CPL_TRUE), CPL_FALSE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(b, a, CPL_TRUE), CPL_FALSE);
  cpl_test_error(CPL_ERROR_NONE);

  /* Check result if b has some keywords missing. */
  irplib_sdp_spectrum_delete(b);
  b = irplib_sdp_spectrum_duplicate(a);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_dec(a), CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(a, b, CPL_FALSE), CPL_FALSE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(b, a, CPL_FALSE), CPL_FALSE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(a, b, CPL_TRUE), CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(b, a, CPL_TRUE), CPL_TRUE);
  cpl_test_error(CPL_ERROR_NONE);

  irplib_sdp_spectrum_delete(b);
  irplib_sdp_spectrum_delete(a);
  return cpl_test_get_failed() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}


static int test_io_functions(void)
{
  cpl_frameset *emptyframes = cpl_frameset_new();
  cpl_frameset *allframes = cpl_frameset_new();
  cpl_propertylist *header = cpl_propertylist_new();
  cpl_parameterlist *parlist = cpl_parameterlist_new();
  cpl_frameset *usedframes = cpl_frameset_new();
  const cpl_frame *inherit = NULL;
  const char *recipe = "test_recipe";
  cpl_propertylist *applist = cpl_propertylist_new();
  cpl_propertylist *tablelist = cpl_propertylist_new();
  const char *remregexp = "^(CHECKSUM|DATASUM)$";
  const char *pipe_id = "iiinstrument";
  const char *dict_id = "TEST_DICT";
  cpl_error_code error;
  const char *filename1 = "dummy_raw_input1.fits";
  const char *filename2 = "dummy_test_sdp_spectrum1.fits";
  const char *filename3 = "dummy_test_sdp_spectrum2.fits";
  const char *filename4 = "dummy_test_sdp_spectrum3.fits";
  cpl_array *data = NULL;
  double datapoints[5] = {1.2, 2.3, 3.4, 4.5, 5.6};
  irplib_sdp_spectrum *b = NULL;
  irplib_sdp_spectrum *a = irplib_sdp_spectrum_new();
  const irplib_sdp_spectrum *spectrum = a;
  cpl_frame *frame = cpl_frame_new();
  cpl_propertylist *plist = cpl_propertylist_new();
  cpl_image *image = cpl_image_new(10, 10, CPL_TYPE_FLOAT);

  cpl_test_assert(allframes != NULL);
  cpl_test_assert(header != NULL);
  cpl_test_assert(parlist != NULL);
  cpl_test_assert(usedframes != NULL);
  cpl_test_assert(applist != NULL);
  cpl_test_assert(tablelist != NULL);
  cpl_test_assert(a != NULL);

  /* Make sure to delete dummy test files so no stale date gets used. */
  (void) remove(filename1);
  (void) remove(filename2);
  (void) remove(filename3);
  (void) remove(filename4);
  errno = 0;  /* In case of expected failure so that CPL tests done fail. */

  /* Save an empty spectrum to file and check we can load it back. */
  cpl_test_eq_error(irplib_sdp_spectrum_set_nelem(a, 2), CPL_ERROR_NONE);
  cpl_test_eq_error( /* Have to setup at least one column or load will fail. */
      irplib_sdp_spectrum_add_column(a, "A", CPL_TYPE_DOUBLE, "s", "5D",
                                     NULL, NULL, NULL),
      CPL_ERROR_NONE
    );
  cpl_test_eq_error(irplib_sdp_spectrum_save(a, filename2, NULL, NULL),
                    CPL_ERROR_NONE);
  b = irplib_sdp_spectrum_load(filename2);
  cpl_test_nonnull(b);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_origin(b), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_prodlvl(b), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_specsys(b), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_fluxerr(b), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_referenc(b), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_voclass(b), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_vopub(b), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_extname(b), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_reset_inherit(b), CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(a, b, CPL_FALSE), CPL_TRUE);

  /* Setup a dummy spectrum and save to file. */
  fill_keywords(a);
  cpl_test_eq_error(irplib_sdp_spectrum_set_nelem(a, 5), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_tutyp(a, "A", "t1"),
                    CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_tucd(a, "A", "u1"),
                    CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_tcomm(a, "A", "c1"),
                    CPL_ERROR_NONE);

  data = cpl_array_wrap_double(datapoints, 5);
  cpl_test_nonnull(data);
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_data(a, "A", data),
                    CPL_ERROR_NONE);
  cpl_array_unwrap(data);

  cpl_test_eq_error(irplib_sdp_spectrum_save(a, filename3, NULL, NULL),
                    CPL_ERROR_NONE);

  /* Now load back the spectrum and see that we have the same structure. */
  irplib_sdp_spectrum_delete(b);
  b = irplib_sdp_spectrum_load(filename3);
  cpl_test_nonnull(b);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(a, b, CPL_FALSE), CPL_TRUE);

  /* Save the spectrum with the DFS version and see if we get the same
   * value after loading it back. */
  cpl_test_eq_error(cpl_propertylist_append_string(applist, CPL_DFS_PRO_CATG,
                                                   "TEST_SPECTRUM"),
                    CPL_ERROR_NONE);

  cpl_test_eq_error(cpl_image_save(image, filename1, CPL_TYPE_FLOAT, plist, CPL_IO_CREATE),
                    CPL_ERROR_NONE);

  cpl_test_eq_error(cpl_frame_set_filename(frame, filename1),
                    CPL_ERROR_NONE);
  cpl_test_eq_error(cpl_frame_set_tag(frame, "RAW_IMAGE"),
                    CPL_ERROR_NONE);
  cpl_test_eq_error(cpl_frame_set_type(frame, CPL_FRAME_TYPE_IMAGE),
                    CPL_ERROR_NONE);
  cpl_test_eq_error(cpl_frame_set_group(frame, CPL_FRAME_GROUP_RAW),
                    CPL_ERROR_NONE);
  cpl_test_eq_error(cpl_frame_set_level(frame, CPL_FRAME_LEVEL_FINAL),
                    CPL_ERROR_NONE);
  cpl_test_eq_error(cpl_frameset_insert(usedframes, frame), CPL_ERROR_NONE);

  error = irplib_dfs_save_spectrum(allframes, header, parlist, usedframes,
                  inherit, spectrum, recipe, applist, tablelist, remregexp,
                  pipe_id, dict_id, filename4);
  cpl_test_eq_error(error, CPL_ERROR_NONE);

  irplib_sdp_spectrum_delete(b);
  b = irplib_sdp_spectrum_load(filename4);
  cpl_test_nonnull(b);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_equal(a, b, CPL_FALSE), CPL_TRUE);

  /* Check error handling for NULL input. */
  error = irplib_dfs_save_spectrum(NULL, header, parlist, usedframes,
                  inherit, spectrum, recipe, applist, tablelist, remregexp,
                  pipe_id, dict_id, filename4);
  cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

  error = irplib_dfs_save_spectrum(allframes, header, NULL, usedframes,
                  inherit, spectrum, recipe, applist, tablelist, remregexp,
                  pipe_id, dict_id, filename4);
  cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

  error = irplib_dfs_save_spectrum(allframes, header, parlist, NULL,
                  inherit, spectrum, recipe, applist, tablelist, remregexp,
                  pipe_id, dict_id, filename4);
  cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

  error = irplib_dfs_save_spectrum(allframes, header, parlist, usedframes,
                  inherit, NULL, recipe, applist, tablelist, remregexp,
                  pipe_id, dict_id, filename4);
  cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

  error = irplib_dfs_save_spectrum(allframes, header, parlist, usedframes,
                  inherit, spectrum, NULL, applist, tablelist, remregexp,
                  pipe_id, dict_id, filename4);
  cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

  error = irplib_dfs_save_spectrum(allframes, header, parlist, usedframes,
                  inherit, spectrum, recipe, NULL, tablelist, remregexp,
                  pipe_id, dict_id, filename4);
  cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

  error = irplib_dfs_save_spectrum(allframes, header, parlist, usedframes,
                  inherit, spectrum, recipe, applist, tablelist, remregexp,
                  NULL, dict_id, filename4);
  cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

  error = irplib_dfs_save_spectrum(allframes, header, parlist, usedframes,
                  inherit, spectrum, recipe, applist, tablelist, remregexp,
                  pipe_id, NULL, filename4);
  cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

  error = irplib_dfs_save_spectrum(allframes, header, parlist, usedframes,
                inherit, spectrum, recipe, applist, tablelist, remregexp,
                pipe_id, dict_id, NULL);
  cpl_test_eq_error(error, CPL_ERROR_NULL_INPUT);

  /* Check error handling of irplib_dfs_save_spectrum. */
  error = irplib_dfs_save_spectrum(allframes, header, parlist, usedframes,
                  inherit, spectrum, recipe, applist, tablelist, remregexp,
                  pipe_id, dict_id, "./invalid/");
  cpl_test_eq_error(error, CPL_ERROR_FILE_NOT_CREATED);

  error = irplib_dfs_save_spectrum(allframes, header, parlist, usedframes,
                  inherit, spectrum, recipe, applist, tablelist, "^^[[((",
                  pipe_id, dict_id, filename4);
  cpl_test_eq_error(error, CPL_ERROR_ILLEGAL_INPUT);

  error = irplib_dfs_save_spectrum(allframes, header, parlist, emptyframes,
                  inherit, spectrum, recipe, applist, tablelist, remregexp,
                  pipe_id, dict_id, filename4);
  cpl_test_eq_error(error, CPL_ERROR_DATA_NOT_FOUND);

  cpl_propertylist_empty(applist);
  error = irplib_dfs_save_spectrum(allframes, header, parlist, usedframes,
                  inherit, spectrum, recipe, applist, tablelist, remregexp,
                  pipe_id, dict_id, filename4);
  cpl_test_eq_error(error, CPL_ERROR_DATA_NOT_FOUND);

  /* Remove the FITS files if no errors were detected and clean up memory. */
  if (cpl_test_get_failed() == 0) {
    (void) remove(filename1);
    (void) remove(filename2);
    (void) remove(filename3);
    (void) remove(filename4);
  }
  irplib_sdp_spectrum_delete(b);
  irplib_sdp_spectrum_delete(a);
  cpl_image_delete(image);
  cpl_propertylist_delete(plist);
  cpl_frameset_delete(emptyframes);
  cpl_frameset_delete(allframes);
  cpl_propertylist_delete(header);
  cpl_parameterlist_delete(parlist);
  cpl_frameset_delete(usedframes);
  cpl_propertylist_delete(applist);
  cpl_propertylist_delete(tablelist);
  return cpl_test_get_failed() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}


static int test_copy_functions(void)
{
  /* Create a property list with some dummy test keywords. We then test that the
   * copy functions behave correctly when trying to copy from this list to an
   * SDP spectrum object. */
  cpl_propertylist *plist = cpl_propertylist_new();
  irplib_sdp_spectrum *a = irplib_sdp_spectrum_new();
  cpl_test_assert(plist != NULL);
  cpl_test_assert(a != NULL);

  cpl_test_assert(cpl_propertylist_append_bool(plist, "TEST_BOOL", CPL_TRUE)
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_propertylist_append_int(plist, "TEST_INT", 123)
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_propertylist_append_long_long(plist, "TEST_LONGLONG", 432)
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_propertylist_append_double(plist, "TEST_DOUBLE", 2.34)
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_propertylist_append_string(plist, "TEST_STRING", "abc")
                  == CPL_ERROR_NONE);

  cpl_test_eq_error(irplib_sdp_spectrum_copy_ra(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_ra(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_dec(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_dec(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_exptime(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_exptime(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_texptime(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_texptime(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_timesys(a, plist, "TEST_STRING"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_timesys(a), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_mjdobs(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_mjdobs(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_mjdend(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_mjdend(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_prodlvl(a, plist, "TEST_INT"),
                    CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_prodlvl(a), 123);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_procsoft(a, plist, "TEST_STRING"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_procsoft(a), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_prodcatg(a, plist, "TEST_STRING"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_prodcatg(a), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_origin(a, plist, "TEST_STRING"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_origin(a), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_extobj(a, plist, "TEST_BOOL"),
                    CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_extobj(a), CPL_TRUE);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_dispelem(a, plist, "TEST_STRING"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_dispelem(a), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_specsys(a, plist, "TEST_STRING"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_specsys(a), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_progid(a, plist, "TEST_STRING"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_progid(a), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_obid(a, 1, plist, "TEST_INT"),
                    CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_obid(a, 1), 123);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_mepoch(a, plist, "TEST_BOOL"),
                    CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_mepoch(a), CPL_TRUE);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_obstech(a, plist, "TEST_STRING"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_obstech(a), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_fluxcal(a, plist, "TEST_STRING"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_fluxcal(a), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_contnorm(a, plist, "TEST_BOOL"),
                    CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_contnorm(a), CPL_TRUE);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_wavelmin(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_wavelmin(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_wavelmax(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_wavelmax(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_specbin(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_specbin(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_totflux(a, plist, "TEST_BOOL"),
                    CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_totflux(a), CPL_TRUE);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_fluxerr(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_fluxerr(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_referenc(a, plist, "TEST_STRING"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_referenc(a), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_specres(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_specres(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_specerr(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_specerr(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_specsye(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_specsye(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_lamnlin(a, plist, "TEST_INT"),
                    CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_lamnlin(a), 123);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_lamrms(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_lamrms(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_gain(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_gain(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_detron(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_detron(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_effron(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_effron(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_snr(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_snr(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_ncombine(a, plist, "TEST_INT"),
                    CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_ncombine(a), 123);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_prov(a, 1, plist, "TEST_STRING"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_prov(a, 1), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_asson(a, 1, plist, "TEST_STRING"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_asson(a, 1), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_assoc(a, 1, plist, "TEST_STRING"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_assoc(a, 1), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_assom(a, 1, plist, "TEST_STRING"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_assom(a, 1), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_voclass(a, plist, "TEST_STRING"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_voclass(a), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_vopub(a, plist, "TEST_STRING"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_vopub(a), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_title(a, plist, "TEST_STRING"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_title(a), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_object(a, plist, "TEST_STRING"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_object(a), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_aperture(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_aperture(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_telapse(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_telapse(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_tmid(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_tmid(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_specval(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_specval(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_specbw(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_specbw(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_extname(a, plist, "TEST_STRING"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_extname(a), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_inherit(a, plist, "TEST_BOOL"),
                    CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_inherit(a), CPL_TRUE);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_nelem(a, plist, "TEST_LONGLONG"),
                    CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_nelem(a), 432);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_tdmin(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_tdmin(a), 2.34, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_tdmax(a, plist, "TEST_DOUBLE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_tdmax(a), 2.34, DBL_EPSILON);

  /* We check the error handling for a few examples. Dont need to check all
   * since most functions derive from the same template code. However, we do
   * check every function for successful operation above to make sure the
   * function is declared properly and links correctly. */
  cpl_test_eq_error(irplib_sdp_spectrum_copy_ra(a, plist, "TEST_STRING"),
                    CPL_ERROR_TYPE_MISMATCH);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_ra(a, plist, "SOME_KEY"),
                    CPL_ERROR_DATA_NOT_FOUND);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_prodlvl(a, plist, "TEST_STRING"),
                    CPL_ERROR_TYPE_MISMATCH);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_prodlvl(a, plist, "SOME_KEY"),
                    CPL_ERROR_DATA_NOT_FOUND);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_procsoft(a, plist, "TEST_INT"),
                    CPL_ERROR_TYPE_MISMATCH);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_procsoft(a, plist, "SOME_KEY"),
                    CPL_ERROR_DATA_NOT_FOUND);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_obid(a, 1, plist, "TEST_STRING"),
                    CPL_ERROR_TYPE_MISMATCH);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_obid(a, 1, plist, "SOME_KEY"),
                    CPL_ERROR_DATA_NOT_FOUND);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_mepoch(a, plist, "TEST_STRING"),
                    CPL_ERROR_TYPE_MISMATCH);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_mepoch(a, plist, "SOME_KEY"),
                    CPL_ERROR_DATA_NOT_FOUND);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_prov(a, 1, plist, "TEST_INT"),
                    CPL_ERROR_TYPE_MISMATCH);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_prov(a, 1, plist, "SOME_KEY"),
                    CPL_ERROR_DATA_NOT_FOUND);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_nelem(a, plist, "TEST_STRING"),
                    CPL_ERROR_TYPE_MISMATCH);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_nelem(a, plist, "SOME_KEY"),
                    CPL_ERROR_DATA_NOT_FOUND);

  irplib_sdp_spectrum_delete(a);
  cpl_propertylist_delete(plist);
  return cpl_test_get_failed() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}


static int test_column_copy_update_functions(void)
{
  const cpl_array *testdata;
  int valuesA[2] = {678, 890};
  int valuesD[2] = {345, 765};
  cpl_array *data;
  cpl_table *table = cpl_table_new(1);
  cpl_propertylist *plist = cpl_propertylist_new();
  irplib_sdp_spectrum *a = irplib_sdp_spectrum_new();
  cpl_test_assert(table != NULL);
  cpl_test_assert(a != NULL);

  /* Create a test table and test copying the columns using the column copy
   * functions. */
  cpl_test_assert(cpl_table_new_column_array(table, "A", CPL_TYPE_INT, 2)
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_table_set_column_unit(table, "A", "x")
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_table_set_column_format(table, "A", "%d")
                  == CPL_ERROR_NONE);
  data = cpl_array_wrap_int(valuesA, 2);
  cpl_test_assert(data != NULL);
  cpl_test_assert(cpl_table_set_array(table, "A", 0, data)
                  == CPL_ERROR_NONE);
  cpl_array_unwrap(data);
  cpl_test_assert(cpl_table_new_column(table, "B", CPL_TYPE_DOUBLE)
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_table_new_column(table, "C", CPL_TYPE_STRING)
                  == CPL_ERROR_NONE);

  cpl_test_eq_error(irplib_sdp_spectrum_copy_column(a, table, "A"),
                    CPL_ERROR_NONE);
  cpl_test(irplib_sdp_spectrum_has_column(a, "A"));
  cpl_test(! irplib_sdp_spectrum_has_column(a, "B"));
  cpl_test(! irplib_sdp_spectrum_has_column(a, "C"));
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 1);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "");
  cpl_test_eq_error(irplib_sdp_spectrum_delete_column(a, "A"), CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 0);
  cpl_test_null(irplib_sdp_spectrum_get_column_tutyp(a, "A"));
  cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);
  cpl_test_null(irplib_sdp_spectrum_get_column_tucd(a, "A"));
  cpl_test_error(CPL_ERROR_DATA_NOT_FOUND);

  cpl_test_eq_error(irplib_sdp_spectrum_copy_column_regexp(a, table, "A|B", 0),
                    CPL_ERROR_NONE);
  cpl_test(irplib_sdp_spectrum_has_column(a, "A"));
  cpl_test(irplib_sdp_spectrum_has_column(a, "B"));
  cpl_test(! irplib_sdp_spectrum_has_column(a, "C"));
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 2);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "B"), "");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "B"), "");
  cpl_test_eq_error(irplib_sdp_spectrum_delete_column(a, "A"), CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_delete_column(a, "B"), CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 0);

  cpl_test_eq_error(irplib_sdp_spectrum_copy_column_regexp(a, table, "A|B", 1),
                    CPL_ERROR_NONE);
  cpl_test(! irplib_sdp_spectrum_has_column(a, "A"));
  cpl_test(! irplib_sdp_spectrum_has_column(a, "B"));
  cpl_test(irplib_sdp_spectrum_has_column(a, "C"));
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 1);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "C"), "");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "C"), "");
  cpl_test_eq_error(irplib_sdp_spectrum_delete_column(a, "C"), CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 0);

  /* Fill the dummy property list and test the individual keyword copy
   * functions. */
  cpl_test_assert(cpl_propertylist_append_string(plist, "TS", "abc")
                   == CPL_ERROR_NONE);

  cpl_test_eq_error(irplib_sdp_spectrum_copy_column(a, table, "A"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_unit(a, "A"), "x");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_format(a, "A"), "%d");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "");
  cpl_test_null(irplib_sdp_spectrum_get_column_tcomm(a, "A"));
  cpl_test_eq_error(irplib_sdp_spectrum_copy_column_unit(a, "A", plist, "TS"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_unit(a, "A"), "abc");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_format(a, "A"), "%d");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "");
  cpl_test_null(irplib_sdp_spectrum_get_column_tcomm(a, "A"));
  cpl_test_eq_error(irplib_sdp_spectrum_delete_column(a, "A"), CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 0);

  cpl_test_eq_error(irplib_sdp_spectrum_copy_column(a, table, "A"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_unit(a, "A"), "x");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_format(a, "A"), "%d");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "");
  cpl_test_null(irplib_sdp_spectrum_get_column_tcomm(a, "A"));
  cpl_test_eq_error(irplib_sdp_spectrum_copy_column_tutyp(a, "A", plist, "TS"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_unit(a, "A"), "x");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_format(a, "A"), "%d");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "abc");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "");
  cpl_test_null(irplib_sdp_spectrum_get_column_tcomm(a, "A"));
  cpl_test_eq_error(irplib_sdp_spectrum_delete_column(a, "A"), CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 0);

  cpl_test_eq_error(irplib_sdp_spectrum_copy_column(a, table, "A"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_unit(a, "A"), "x");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_format(a, "A"), "%d");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "");
  cpl_test_null(irplib_sdp_spectrum_get_column_tcomm(a, "A"));
  cpl_test_eq_error(irplib_sdp_spectrum_copy_column_tucd(a, "A", plist, "TS"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_unit(a, "A"), "x");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_format(a, "A"), "%d");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "abc");
  cpl_test_null(irplib_sdp_spectrum_get_column_tcomm(a, "A"));
  cpl_test_eq_error(irplib_sdp_spectrum_delete_column(a, "A"), CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 0);

  cpl_test_eq_error(irplib_sdp_spectrum_copy_column(a, table, "A"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_unit(a, "A"), "x");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_format(a, "A"), "%d");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "");
  cpl_test_null(irplib_sdp_spectrum_get_column_tcomm(a, "A"));
  cpl_test_eq_error(irplib_sdp_spectrum_copy_column_tcomm(a, "A", plist, "TS"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_unit(a, "A"), "x");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_format(a, "A"), "%d");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tcomm(a, "A"), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_delete_column(a, "A"), CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 0);

  /* Add another column to the test table to check behaviour of the update
   * function. */
  cpl_test_assert(cpl_table_new_column_array(table, "D", CPL_TYPE_INT, 2)
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_table_set_column_unit(table, "D", "y")
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_table_set_column_format(table, "D", "%5d")
                  == CPL_ERROR_NONE);
  data = cpl_array_wrap_int(valuesD, 2);
  cpl_test_assert(data != NULL);
  cpl_test_assert(cpl_table_set_array(table, "D", 0, data)
                  == CPL_ERROR_NONE);
  cpl_array_unwrap(data);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_column(a, table, "A"),
                    CPL_ERROR_NONE);

  cpl_test_eq_error(irplib_sdp_spectrum_update_column(a, "A", table, "D",
                                                      IRPLIB_COLUMN_UNIT),
                    CPL_ERROR_NONE);
  cpl_test(irplib_sdp_spectrum_has_column(a, "A"));
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 1);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_unit(a, "A"), "y");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_format(a, "A"), "%d");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "");
  testdata = irplib_sdp_spectrum_get_column_data(a, "A");
  cpl_test_nonnull(data);
  cpl_test_eq(cpl_array_get_int(testdata, 0, NULL), valuesA[0]);
  cpl_test_eq(cpl_array_get_int(testdata, 1, NULL), valuesA[1]);
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_unit(a, "A", "x"),
                    CPL_ERROR_NONE);

  cpl_test_eq_error(irplib_sdp_spectrum_update_column(a, "A", table, "D",
                                                      IRPLIB_COLUMN_FORMAT),
                    CPL_ERROR_NONE);
  cpl_test(irplib_sdp_spectrum_has_column(a, "A"));
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 1);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_unit(a, "A"), "x");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_format(a, "A"), "%5d");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "");
  testdata = irplib_sdp_spectrum_get_column_data(a, "A");
  cpl_test_nonnull(data);
  cpl_test_eq(cpl_array_get_int(testdata, 0, NULL), valuesA[0]);
  cpl_test_eq(cpl_array_get_int(testdata, 1, NULL), valuesA[1]);
  cpl_test_eq_error(irplib_sdp_spectrum_set_column_format(a, "A", "%d"),
                    CPL_ERROR_NONE);

  cpl_test_eq_error(irplib_sdp_spectrum_update_column(a, "A", table, "D",
                                                      IRPLIB_COLUMN_DATA),
                    CPL_ERROR_NONE);
  cpl_test(irplib_sdp_spectrum_has_column(a, "A"));
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 1);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_unit(a, "A"), "x");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_format(a, "A"), "%d");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "");
  testdata = irplib_sdp_spectrum_get_column_data(a, "A");
  cpl_test_nonnull(data);
  cpl_test_eq(cpl_array_get_int(testdata, 0, NULL), valuesD[0]);
  cpl_test_eq(cpl_array_get_int(testdata, 1, NULL), valuesD[1]);

  /* Test update behaves like copy when column is missing. */
  cpl_test_eq_error(irplib_sdp_spectrum_delete_column(a, "A"), CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 0);
  cpl_test_eq_error(irplib_sdp_spectrum_update_column(a, "A", table, "D",
                                                      IRPLIB_COLUMN_DATA),
                    CPL_ERROR_NONE);
  cpl_test(irplib_sdp_spectrum_has_column(a, "A"));
  cpl_test_eq(irplib_sdp_spectrum_get_ncol(a), 1);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_unit(a, "A"), "y");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_format(a, "A"), "%5d");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "");
  testdata = irplib_sdp_spectrum_get_column_data(a, "A");
  cpl_test_nonnull(data);
  cpl_test_eq(cpl_array_get_int(testdata, 0, NULL), valuesD[0]);
  cpl_test_eq(cpl_array_get_int(testdata, 1, NULL), valuesD[1]);

  irplib_sdp_spectrum_delete(a);
  cpl_propertylist_delete(plist);
  cpl_table_delete(table);
  return cpl_test_get_failed() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}


static int test_generic_copy_functions(void)
{
  /* Create a property list with some test keywords and test copying of these
   * using the generic copy functions. */
  const cpl_property *p;
  cpl_propertylist *plist = cpl_propertylist_new();
  irplib_sdp_spectrum *a = irplib_sdp_spectrum_new();
  cpl_test_assert(plist != NULL);
  cpl_test_assert(a != NULL);

  cpl_test_assert(cpl_propertylist_append_bool(plist, "M_EPOCH", CPL_TRUE)
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_propertylist_append_int(plist, "PRODLVL", 123)
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_propertylist_append_int(plist, "OBID1", 234)
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_propertylist_append_int(plist, "OBID2", 345)
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_propertylist_append_long_long(plist, "NELEM", 456)
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_propertylist_append_double(plist, "EXPTIME", 1.35)
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_propertylist_append_string(plist, "PROCSOFT", "abc")
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_propertylist_append_string(plist, "PROV1", "bcd")
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_propertylist_append_string(plist, "PROV2", "cde")
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_propertylist_append_string(plist, "TUTYP1", "def")
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_propertylist_append_string(plist, "TUTYP2", "efg")
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_propertylist_append_string(plist, "TUCD1", "fgh")
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_propertylist_append_string(plist, "TUCD2", "ghi")
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_propertylist_append_double(plist, "APERTURE", 2.46)
                  == CPL_ERROR_NONE);
  cpl_test_assert(cpl_propertylist_append_double(plist, "WAVELMIN", 3.57)
                  == CPL_ERROR_NONE);

  cpl_test_eq_error(irplib_sdp_spectrum_copy_keyword(a, plist, "M_EPOCH"),
                    CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_mepoch(a), CPL_TRUE);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_keyword(a, plist, "PRODLVL"),
                    CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_prodlvl(a), 123);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_keyword(a, plist, "OBID1"),
                    CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_keyword(a, plist, "OBID2"),
                    CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_obid(a, 1), 234);
  cpl_test_eq(irplib_sdp_spectrum_get_obid(a, 2), 345);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_keyword(a, plist, "EXPTIME"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_exptime(a), 1.35, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_keyword(a, plist, "PROCSOFT"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_procsoft(a), "abc");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_keyword(a, plist, "PROV1"),
                    CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_keyword(a, plist, "PROV2"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_prov(a, 1), "bcd");
  cpl_test_eq_string(irplib_sdp_spectrum_get_prov(a, 2), "cde");

  cpl_test_eq_error(
      irplib_sdp_spectrum_add_column(a, "A", CPL_TYPE_INT, "s", "%2d", "", "",
                                     NULL),
      CPL_ERROR_NONE
    );
  cpl_test_eq_error(
      irplib_sdp_spectrum_add_column(a, "B", CPL_TYPE_INT, "m", "%5d", "", "",
                                     NULL),
      CPL_ERROR_NONE
    );
  cpl_test_eq_error(irplib_sdp_spectrum_copy_keyword(a, plist, "TUTYP1"),
                    CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_keyword(a, plist, "TUTYP2"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "def");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "B"), "efg");
  cpl_test_eq_error(irplib_sdp_spectrum_copy_keyword(a, plist, "TUCD1"),
                    CPL_ERROR_NONE);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_keyword(a, plist, "TUCD2"),
                    CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "fgh");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "B"), "ghi");

  cpl_test_eq_error(irplib_sdp_spectrum_copy_keyword(a, plist, "APERTURE"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_aperture(a), 2.46, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_keyword(a, plist, "WAVELMIN"),
                    CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_wavelmin(a), 3.57, DBL_EPSILON);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_keyword(a, plist, "NELEM"),
                    CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_nelem(a), 456);

  /* Test the usage of the irplib_sdp_spectrum_copy_property function. */
  irplib_sdp_spectrum_delete(a);
  a = irplib_sdp_spectrum_new();

  p = cpl_propertylist_get_property_const(plist, "M_EPOCH");
  cpl_test_assert(p != NULL);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_property(a, p), CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_mepoch(a), CPL_TRUE);
  p = cpl_propertylist_get_property_const(plist, "PRODLVL");
  cpl_test_assert(p != NULL);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_property(a, p), CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_prodlvl(a), 123);
  p = cpl_propertylist_get_property_const(plist, "OBID1");
  cpl_test_assert(p != NULL);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_property(a, p), CPL_ERROR_NONE);
  p = cpl_propertylist_get_property_const(plist, "OBID2");
  cpl_test_assert(p != NULL);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_property(a, p), CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_obid(a, 1), 234);
  cpl_test_eq(irplib_sdp_spectrum_get_obid(a, 2), 345);
  p = cpl_propertylist_get_property_const(plist, "EXPTIME");
  cpl_test_assert(p != NULL);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_property(a, p), CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_exptime(a), 1.35, DBL_EPSILON);
  p = cpl_propertylist_get_property_const(plist, "PROCSOFT");
  cpl_test_assert(p != NULL);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_property(a, p), CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_procsoft(a), "abc");
  p = cpl_propertylist_get_property_const(plist, "PROV1");
  cpl_test_assert(p != NULL);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_property(a, p), CPL_ERROR_NONE);
  p = cpl_propertylist_get_property_const(plist, "PROV2");
  cpl_test_assert(p != NULL);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_property(a, p), CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_prov(a, 1), "bcd");
  cpl_test_eq_string(irplib_sdp_spectrum_get_prov(a, 2), "cde");

  cpl_test_eq_error(
      irplib_sdp_spectrum_add_column(a, "A", CPL_TYPE_INT, "s", "%2d", "", "",
                                     NULL),
      CPL_ERROR_NONE
    );
  cpl_test_eq_error(
      irplib_sdp_spectrum_add_column(a, "B", CPL_TYPE_INT, "m", "%5d", "", "",
                                     NULL),
      CPL_ERROR_NONE
    );
  p = cpl_propertylist_get_property_const(plist, "TUTYP1");
  cpl_test_assert(p != NULL);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_property(a, p), CPL_ERROR_NONE);
  p = cpl_propertylist_get_property_const(plist, "TUTYP2");
  cpl_test_assert(p != NULL);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_property(a, p), CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "def");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "B"), "efg");
  p = cpl_propertylist_get_property_const(plist, "TUCD1");
  cpl_test_assert(p != NULL);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_property(a, p), CPL_ERROR_NONE);
  p = cpl_propertylist_get_property_const(plist, "TUCD2");
  cpl_test_assert(p != NULL);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_property(a, p), CPL_ERROR_NONE);
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "fgh");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "B"), "ghi");

  p = cpl_propertylist_get_property_const(plist, "APERTURE");
  cpl_test_assert(p != NULL);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_property(a, p), CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_aperture(a), 2.46, DBL_EPSILON);
  p = cpl_propertylist_get_property_const(plist, "WAVELMIN");
  cpl_test_assert(p != NULL);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_property(a, p), CPL_ERROR_NONE);
  cpl_test_abs(irplib_sdp_spectrum_get_wavelmin(a), 3.57, DBL_EPSILON);
  p = cpl_propertylist_get_property_const(plist, "NELEM");
  cpl_test_assert(p != NULL);
  cpl_test_eq_error(irplib_sdp_spectrum_copy_property(a, p), CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_nelem(a), 456);

  /* Test the regular expression copy function. */
  irplib_sdp_spectrum_delete(a);
  a = irplib_sdp_spectrum_new();
  cpl_test_eq_error(
      irplib_sdp_spectrum_add_column(a, "A", CPL_TYPE_INT, "s", "%2d", "", "",
                                     NULL),
      CPL_ERROR_NONE
    );
  cpl_test_eq_error(
      irplib_sdp_spectrum_add_column(a, "B", CPL_TYPE_INT, "m", "%5d", "", "",
                                     NULL),
      CPL_ERROR_NONE
    );

  cpl_test_eq_error(irplib_sdp_spectrum_copy_property_regexp(a, plist,
                                                             "PROV.*", 1),
                    CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_mepoch(a), CPL_TRUE);
  cpl_test_eq(irplib_sdp_spectrum_get_prodlvl(a), 123);
  cpl_test_eq(irplib_sdp_spectrum_get_obid(a, 1), 234);
  cpl_test_eq(irplib_sdp_spectrum_get_obid(a, 2), 345);
  cpl_test_abs(irplib_sdp_spectrum_get_exptime(a), 1.35, DBL_EPSILON);
  cpl_test_eq_string(irplib_sdp_spectrum_get_procsoft(a), "abc");
  cpl_test_null(irplib_sdp_spectrum_get_prov(a, 1));
  cpl_test_null(irplib_sdp_spectrum_get_prov(a, 2));
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "def");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "B"), "efg");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "fgh");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "B"), "ghi");
  cpl_test_abs(irplib_sdp_spectrum_get_aperture(a), 2.46, DBL_EPSILON);
  cpl_test_abs(irplib_sdp_spectrum_get_wavelmin(a), 3.57, DBL_EPSILON);
  cpl_test_eq(irplib_sdp_spectrum_get_nelem(a), 456);

  cpl_test_eq_error(irplib_sdp_spectrum_copy_property_regexp(a, plist,
                                                             ".*", 0),
                    CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_get_mepoch(a), CPL_TRUE);
  cpl_test_eq(irplib_sdp_spectrum_get_prodlvl(a), 123);
  cpl_test_eq(irplib_sdp_spectrum_get_obid(a, 1), 234);
  cpl_test_eq(irplib_sdp_spectrum_get_obid(a, 2), 345);
  cpl_test_abs(irplib_sdp_spectrum_get_exptime(a), 1.35, DBL_EPSILON);
  cpl_test_eq_string(irplib_sdp_spectrum_get_procsoft(a), "abc");
  cpl_test_eq_string(irplib_sdp_spectrum_get_prov(a, 1), "bcd");
  cpl_test_eq_string(irplib_sdp_spectrum_get_prov(a, 2), "cde");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "A"), "def");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tutyp(a, "B"), "efg");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "A"), "fgh");
  cpl_test_eq_string(irplib_sdp_spectrum_get_column_tucd(a, "B"), "ghi");
  cpl_test_abs(irplib_sdp_spectrum_get_aperture(a), 2.46, DBL_EPSILON);
  cpl_test_abs(irplib_sdp_spectrum_get_wavelmin(a), 3.57, DBL_EPSILON);
  cpl_test_eq(irplib_sdp_spectrum_get_nelem(a), 456);

  irplib_sdp_spectrum_delete(a);
  cpl_propertylist_delete(plist);
  return cpl_test_get_failed() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}


/**
 * @internal
 * @brief Create an input file for testing irplib_sdp_spectrum_append_prov.
 *
 * Generate an input file with only a property list with one keyword. An
 * appropriate frame is also created and added to the list of frames.
 *
 * @param[out] frames    The output list of frames to which a frame is appended.
 * @param[in]  filename  Name of the file to generate.
 * @param[in]  keyword   The name of the keyword to add. NULL if none should be
 *                       added.
 * @param[in]  value     The value of the keyword. If @c keyword is NULL then
 *                       this is ignored.
 * @return CPL_TRUE if the file was created and testing can continue, CPL_FALSE
 *         otherwise.
 */
static cpl_boolean create_file_with_key(cpl_frameset* frames,
                                        const char *filename,
                                        const char *keyword,
                                        const char *value)
{
  cpl_error_code error = CPL_ERROR_NONE;
  cpl_frame *frame = cpl_frame_new();
  cpl_propertylist *plist = cpl_propertylist_new();
  if (frame == NULL || plist == NULL) goto cleanup;
  if (keyword != NULL) {
    error |= cpl_propertylist_append_string(plist, keyword, value);
  }
  error |= cpl_propertylist_save(plist, filename, CPL_IO_CREATE);
  if (error) goto cleanup;
  cpl_propertylist_delete(plist);
  error |= cpl_frame_set_filename(frame, filename);
  error |= cpl_frame_set_tag(frame, "RAW");
  error |= cpl_frame_set_group(frame, CPL_FRAME_GROUP_RAW);
  error |= cpl_frame_set_level(frame, CPL_FRAME_LEVEL_FINAL);
  error |= cpl_frameset_insert(frames, frame);
  if (error) goto cleanup;
  return CPL_TRUE;

cleanup:
  cpl_frame_delete(frame);
  cpl_propertylist_delete(plist);
  return CPL_FALSE;
}


static int test_append_provenance(void)
{
  cpl_error_code error;
  const char *filename1 = "dummy_raw_input1_for_prov_test.fits";
  const char *filename2 = "dummy_raw_input2_for_prov_test.fits";
  const char *filename3 = "dummy_raw_input3_for_prov_test.fits";
  cpl_frameset *frames = cpl_frameset_new();
  irplib_sdp_spectrum *spec = irplib_sdp_spectrum_new();
  cpl_test_assert(frames != NULL);
  cpl_test_assert(spec != NULL);

  /* Test behaviour of irplib_sdp_spectrum_append_prov.
   * We first need to create a number of test input files and add them to the
   * frameset. One file should contain the ARCFILE keyword, another ORIGFILE and
   * the one should contain neither. */
  cpl_test_assert(create_file_with_key(frames, filename1, "ARCFILE", "fileA"));
  cpl_test_assert(create_file_with_key(frames, filename2, "ORIGFILE", "fileB"));
  cpl_test_assert(create_file_with_key(frames, filename3, NULL, NULL));
  cpl_test_eq(cpl_frameset_get_size(frames), 3);

  /* Now execute irplib_sdp_spectrum_append_prov and check that the PROVi
   * keywords are setup as expected. */
  error = irplib_sdp_spectrum_append_prov(spec, 2, frames);
  cpl_test_error(CPL_ERROR_NONE);
  cpl_test_eq_error(error, CPL_ERROR_NONE);
  cpl_test_eq(irplib_sdp_spectrum_count_prov(spec),
              cpl_frameset_get_size(frames));
  cpl_test_eq_string(irplib_sdp_spectrum_get_prov(spec, 2), "fileA");
  cpl_test_eq_string(irplib_sdp_spectrum_get_prov(spec, 3), "fileB");
  cpl_test_eq_string(irplib_sdp_spectrum_get_prov(spec, 4), filename3);

  /* Remove the FITS files if no errors were detected and clean up memory. */
  if (cpl_test_get_failed() == 0) {
    (void) remove(filename1);
    (void) remove(filename2);
    (void) remove(filename3);
  }
  irplib_sdp_spectrum_delete(spec);
  cpl_frameset_delete(frames);
  return cpl_test_get_failed() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
