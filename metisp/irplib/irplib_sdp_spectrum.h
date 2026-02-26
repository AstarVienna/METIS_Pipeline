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

#ifndef IRPLIB_SDP_SPECTRUM_H
#define IRPLIB_SDP_SPECTRUM_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>
#include <math.h>

CPL_BEGIN_DECLS

/*-----------------------------------------------------------------------------
                                   Defines
 -----------------------------------------------------------------------------*/

/* Define a default value for NAN if not already defined.
 * Note: one should add a test to configure.ac to check if the compiler can
 * handle NAN defined as (0.0/0.0) and define IRPLIB_USE_ZERO_BY_ZERO_FOR_NAN
 * appropriately when compiling this file. */
#ifndef NAN
#  ifdef IRPLIB_USE_ZERO_BY_ZERO_FOR_NAN

#    define NAN (0.0/0.0)

#  else /* IRPLIB_USE_ZERO_BY_ZERO_FOR_NAN */

#    include <assert.h>
#    include <stdint.h>
#    include <string.h>

     inline static float _irplib_nan_const(void)
     {
       /* Perform type-punning to convert binary representation to the floating
        * point format. We use memcpy since this should be the most portable.
        * Unions are OK only from C99, though in principle most older compilers
        * should anyway behave in the expected manner. The _irplib_nan_const is
        * actually a workaround for the better alternative of using 0.0/0.0
        * directly in the code, which should work with a modern compiler. Thus,
        * we just stick to memcpy to avoid strict aliasing rule violations.
        */
       uint32_t i = 0xFFC00000;
       float f;
       assert(sizeof(i) == sizeof(f));
       memcpy(&f, &i, sizeof(f));
       return f;
     }

#    define NAN _irplib_nan_const()

#  endif /* IRPLIB_USE_ZERO_BY_ZERO_FOR_NAN */
#endif /* NAN */

/*-----------------------------------------------------------------------------
                                   Type declarations
 -----------------------------------------------------------------------------*/

/**
 * @ingroup irplib_sdp_spectrum
 * @brief Data type for a Science Data Product 1D spectrum.
 */
typedef struct _irplib_sdp_spectrum_ irplib_sdp_spectrum;

/**
 * @ingroup irplib_sdp_spectrum
 * @brief Bitfield flags for the column update function.
 */
typedef enum _irplib_sdp_spectrum_update_flags_ {
  /* Indicates the units should be copied. */
  IRPLIB_COLUMN_UNIT    = 1 << 1,

  /* Indicates the format string should be copied. */
  IRPLIB_COLUMN_FORMAT  = 1 << 2,

  /* Indicates the data should be copied. */
  IRPLIB_COLUMN_DATA    = 1 << 3

} irplib_sdp_spectrum_update_flags;

/*-----------------------------------------------------------------------------
                                   Prototypes
 -----------------------------------------------------------------------------*/

/*
 * Create, copy and destroy operations.
 */

irplib_sdp_spectrum * irplib_sdp_spectrum_new(void) CPL_ATTR_ALLOC;

irplib_sdp_spectrum *
irplib_sdp_spectrum_duplicate(const irplib_sdp_spectrum *other) CPL_ATTR_ALLOC;

void irplib_sdp_spectrum_delete(irplib_sdp_spectrum *self);

/*
 * Method to compare two spectra.
 */

cpl_boolean irplib_sdp_spectrum_equal(const irplib_sdp_spectrum *a,
                                      const irplib_sdp_spectrum *b,
                                      cpl_boolean only_intersect);

/*
 * Methods to count number of elements for keyword series.
 */
cpl_size irplib_sdp_spectrum_count_obid(const irplib_sdp_spectrum *self);
cpl_size irplib_sdp_spectrum_count_prov(const irplib_sdp_spectrum *self);
cpl_size irplib_sdp_spectrum_count_asson(const irplib_sdp_spectrum *self);
cpl_size irplib_sdp_spectrum_count_assoc(const irplib_sdp_spectrum *self);
cpl_size irplib_sdp_spectrum_count_assom(const irplib_sdp_spectrum *self);

/*
 * Methods to copy keyword values from property objects.
 */
cpl_error_code irplib_sdp_spectrum_copy_keyword(irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *name);

cpl_error_code irplib_sdp_spectrum_copy_property(irplib_sdp_spectrum *self,
                                                 const cpl_property *prop);

cpl_error_code irplib_sdp_spectrum_copy_property_regexp(
                                                irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *regexp,
                                                int invert);

/*
 * Get/set methods for keywords.
 */

double irplib_sdp_spectrum_get_ra(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_ra(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_ra(irplib_sdp_spectrum *self,
                                          double value);
cpl_error_code irplib_sdp_spectrum_copy_ra(irplib_sdp_spectrum *self,
                                           const cpl_propertylist *plist,
                                           const char *name);

double irplib_sdp_spectrum_get_dec(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_dec(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_dec(irplib_sdp_spectrum *self,
                                           double value);
cpl_error_code irplib_sdp_spectrum_copy_dec(irplib_sdp_spectrum *self,
                                            const cpl_propertylist *plist,
                                            const char *name);

double irplib_sdp_spectrum_get_exptime(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_exptime(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_exptime(irplib_sdp_spectrum *self,
                                               double value);
cpl_error_code irplib_sdp_spectrum_copy_exptime(irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *name);

double irplib_sdp_spectrum_get_texptime(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_texptime(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_texptime(irplib_sdp_spectrum *self,
                                                double value);
cpl_error_code irplib_sdp_spectrum_copy_texptime(irplib_sdp_spectrum *self,
                                                 const cpl_propertylist *plist,
                                                 const char *name);

double irplib_sdp_spectrum_get_mjdobs(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_mjdobs(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_mjdobs(irplib_sdp_spectrum *self,
                                              double value);
cpl_error_code irplib_sdp_spectrum_copy_mjdobs(irplib_sdp_spectrum *self,
                                               const cpl_propertylist *plist,
                                               const char *name);

const char * irplib_sdp_spectrum_get_timesys(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_timesys(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_timesys(irplib_sdp_spectrum *self,
                                               const char *value);
cpl_error_code irplib_sdp_spectrum_copy_timesys(irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *name);

double irplib_sdp_spectrum_get_mjdend(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_mjdend(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_mjdend(irplib_sdp_spectrum *self,
                                              double value);
cpl_error_code irplib_sdp_spectrum_copy_mjdend(irplib_sdp_spectrum *self,
                                               const cpl_propertylist *plist,
                                               const char *name);

int irplib_sdp_spectrum_get_prodlvl(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_prodlvl(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_prodlvl(irplib_sdp_spectrum *self,
                                               int value);
cpl_error_code irplib_sdp_spectrum_copy_prodlvl(irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *name);

const char * irplib_sdp_spectrum_get_procsoft(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_procsoft(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_procsoft(irplib_sdp_spectrum *self,
                                                const char *value);
cpl_error_code irplib_sdp_spectrum_copy_procsoft(irplib_sdp_spectrum *self,
                                                 const cpl_propertylist *plist,
                                                 const char *name);

const char * irplib_sdp_spectrum_get_prodcatg(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_prodcatg(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_prodcatg(irplib_sdp_spectrum *self,
                                                const char *value);
cpl_error_code irplib_sdp_spectrum_copy_prodcatg(irplib_sdp_spectrum *self,
                                                 const cpl_propertylist *plist,
                                                 const char *name);

const char * irplib_sdp_spectrum_get_origin(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_origin(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_origin(irplib_sdp_spectrum *self,
                                              const char *value);
cpl_error_code irplib_sdp_spectrum_copy_origin(irplib_sdp_spectrum *self,
                                               const cpl_propertylist *plist,
                                               const char *name);

cpl_boolean irplib_sdp_spectrum_get_extobj(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_extobj(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_extobj(irplib_sdp_spectrum *self,
                                              cpl_boolean value);
cpl_error_code irplib_sdp_spectrum_copy_extobj(irplib_sdp_spectrum *self,
                                               const cpl_propertylist *plist,
                                               const char *name);

const char * irplib_sdp_spectrum_get_dispelem(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_dispelem(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_dispelem(irplib_sdp_spectrum *self,
                                                const char *value);
cpl_error_code irplib_sdp_spectrum_copy_dispelem(irplib_sdp_spectrum *self,
                                                 const cpl_propertylist *plist,
                                                 const char *name);

const char * irplib_sdp_spectrum_get_specsys(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_specsys(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_specsys(irplib_sdp_spectrum *self,
                                               const char *value);
cpl_error_code irplib_sdp_spectrum_copy_specsys(irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *name);

const char * irplib_sdp_spectrum_get_progid(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_progid(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_progid(irplib_sdp_spectrum *self,
                                              const char *value);
cpl_error_code irplib_sdp_spectrum_copy_progid(irplib_sdp_spectrum *self,
                                               const cpl_propertylist *plist,
                                               const char *name);

int irplib_sdp_spectrum_get_obid(const irplib_sdp_spectrum *self,
                                 cpl_size index);
cpl_error_code irplib_sdp_spectrum_reset_obid(irplib_sdp_spectrum *self,
                                              cpl_size index);
cpl_error_code irplib_sdp_spectrum_set_obid(irplib_sdp_spectrum *self,
                                            cpl_size index, int value);
cpl_error_code irplib_sdp_spectrum_copy_obid(irplib_sdp_spectrum *self,
                                             cpl_size index,
                                             const cpl_propertylist *plist,
                                             const char *name);

cpl_boolean irplib_sdp_spectrum_get_mepoch(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_mepoch(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_mepoch(irplib_sdp_spectrum *self,
                                              cpl_boolean value);
cpl_error_code irplib_sdp_spectrum_copy_mepoch(irplib_sdp_spectrum *self,
                                               const cpl_propertylist *plist,
                                               const char *name);

const char * irplib_sdp_spectrum_get_obstech(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_obstech(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_obstech(irplib_sdp_spectrum *self,
                                               const char *value);
cpl_error_code irplib_sdp_spectrum_copy_obstech(irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *name);

const char * irplib_sdp_spectrum_get_fluxcal(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_fluxcal(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_fluxcal(irplib_sdp_spectrum *self,
                                               const char *value);
cpl_error_code irplib_sdp_spectrum_copy_fluxcal(irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *name);

cpl_boolean irplib_sdp_spectrum_get_contnorm(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_contnorm(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_contnorm(irplib_sdp_spectrum *self,
                                                cpl_boolean value);
cpl_error_code irplib_sdp_spectrum_copy_contnorm(irplib_sdp_spectrum *self,
                                                 const cpl_propertylist *plist,
                                                 const char *name);

double irplib_sdp_spectrum_get_wavelmin(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_wavelmin(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_wavelmin(irplib_sdp_spectrum *self,
                                                double value);
cpl_error_code irplib_sdp_spectrum_copy_wavelmin(irplib_sdp_spectrum *self,
                                                 const cpl_propertylist *plist,
                                                 const char *name);

double irplib_sdp_spectrum_get_wavelmax(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_wavelmax(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_wavelmax(irplib_sdp_spectrum *self,
                                                double value);
cpl_error_code irplib_sdp_spectrum_copy_wavelmax(irplib_sdp_spectrum *self,
                                                 const cpl_propertylist *plist,
                                                 const char *name);

double irplib_sdp_spectrum_get_specbin(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_specbin(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_specbin(irplib_sdp_spectrum *self,
                                               double value);
cpl_error_code irplib_sdp_spectrum_copy_specbin(irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *name);

cpl_boolean irplib_sdp_spectrum_get_totflux(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_totflux(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_totflux(irplib_sdp_spectrum *self,
                                               cpl_boolean value);
cpl_error_code irplib_sdp_spectrum_copy_totflux(irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *name);

double irplib_sdp_spectrum_get_fluxerr(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_fluxerr(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_fluxerr(irplib_sdp_spectrum *self,
                                               double value);
cpl_error_code irplib_sdp_spectrum_copy_fluxerr(irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *name);

const char * irplib_sdp_spectrum_get_referenc(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_referenc(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_referenc(irplib_sdp_spectrum *self,
                                                const char *value);
cpl_error_code irplib_sdp_spectrum_copy_referenc(irplib_sdp_spectrum *self,
                                                 const cpl_propertylist *plist,
                                                 const char *name);

double irplib_sdp_spectrum_get_specres(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_specres(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_specres(irplib_sdp_spectrum *self,
                                               double value);
cpl_error_code irplib_sdp_spectrum_copy_specres(irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *name);

double irplib_sdp_spectrum_get_specerr(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_specerr(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_specerr(irplib_sdp_spectrum *self,
                                               double value);
cpl_error_code irplib_sdp_spectrum_copy_specerr(irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *name);

double irplib_sdp_spectrum_get_specsye(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_specsye(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_specsye(irplib_sdp_spectrum *self,
                                               double value);
cpl_error_code irplib_sdp_spectrum_copy_specsye(irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *name);

int irplib_sdp_spectrum_get_lamnlin(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_lamnlin(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_lamnlin(irplib_sdp_spectrum *self,
                                               int value);
cpl_error_code irplib_sdp_spectrum_copy_lamnlin(irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *name);

double irplib_sdp_spectrum_get_lamrms(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_lamrms(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_lamrms(irplib_sdp_spectrum *self,
                                              double value);
cpl_error_code irplib_sdp_spectrum_copy_lamrms(irplib_sdp_spectrum *self,
                                               const cpl_propertylist *plist,
                                               const char *name);

double irplib_sdp_spectrum_get_gain(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_gain(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_gain(irplib_sdp_spectrum *self,
                                            double value);
cpl_error_code irplib_sdp_spectrum_copy_gain(irplib_sdp_spectrum *self,
                                             const cpl_propertylist *plist,
                                             const char *name);

double irplib_sdp_spectrum_get_detron(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_detron(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_detron(irplib_sdp_spectrum *self,
                                              double value);
cpl_error_code irplib_sdp_spectrum_copy_detron(irplib_sdp_spectrum *self,
                                               const cpl_propertylist *plist,
                                               const char *name);

double irplib_sdp_spectrum_get_effron(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_effron(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_effron(irplib_sdp_spectrum *self,
                                              double value);
cpl_error_code irplib_sdp_spectrum_copy_effron(irplib_sdp_spectrum *self,
                                               const cpl_propertylist *plist,
                                               const char *name);

double irplib_sdp_spectrum_get_snr(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_snr(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_snr(irplib_sdp_spectrum *self,
                                           double value);
cpl_error_code irplib_sdp_spectrum_copy_snr(irplib_sdp_spectrum *self,
                                            const cpl_propertylist *plist,
                                            const char *name);

int irplib_sdp_spectrum_get_ncombine(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_ncombine(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_ncombine(irplib_sdp_spectrum *self,
                                                int value);
cpl_error_code irplib_sdp_spectrum_copy_ncombine(irplib_sdp_spectrum *self,
                                                 const cpl_propertylist *plist,
                                                 const char *name);

const char * irplib_sdp_spectrum_get_prov(const irplib_sdp_spectrum *self,
                                          cpl_size index);
cpl_error_code irplib_sdp_spectrum_reset_prov(irplib_sdp_spectrum *self,
                                              cpl_size index);
cpl_error_code irplib_sdp_spectrum_set_prov(irplib_sdp_spectrum *self,
                                            cpl_size index, const char *value);
cpl_error_code irplib_sdp_spectrum_copy_prov(irplib_sdp_spectrum *self,
                                             cpl_size index,
                                             const cpl_propertylist *plist,
                                             const char *name);
cpl_error_code irplib_sdp_spectrum_append_prov(irplib_sdp_spectrum *self,
                                               cpl_size firstindex,
                                               const cpl_frameset *frames);

const char * irplib_sdp_spectrum_get_asson(const irplib_sdp_spectrum *self,
                                           cpl_size index);
cpl_error_code irplib_sdp_spectrum_reset_asson(irplib_sdp_spectrum *self,
                                               cpl_size index);
cpl_error_code irplib_sdp_spectrum_set_asson(irplib_sdp_spectrum *self,
                                             cpl_size index, const char *value);
cpl_error_code irplib_sdp_spectrum_copy_asson(irplib_sdp_spectrum *self,
                                              cpl_size index,
                                              const cpl_propertylist *plist,
                                              const char *name);

const char * irplib_sdp_spectrum_get_assoc(const irplib_sdp_spectrum *self,
                                           cpl_size index);
cpl_error_code irplib_sdp_spectrum_reset_assoc(irplib_sdp_spectrum *self,
                                               cpl_size index);
cpl_error_code irplib_sdp_spectrum_set_assoc(irplib_sdp_spectrum *self,
                                             cpl_size index, const char *value);
cpl_error_code irplib_sdp_spectrum_copy_assoc(irplib_sdp_spectrum *self,
                                              cpl_size index,
                                              const cpl_propertylist *plist,
                                              const char *name);

const char * irplib_sdp_spectrum_get_assom(const irplib_sdp_spectrum *self,
                                           cpl_size index);
cpl_error_code irplib_sdp_spectrum_reset_assom(irplib_sdp_spectrum *self,
                                               cpl_size index);
cpl_error_code irplib_sdp_spectrum_set_assom(irplib_sdp_spectrum *self,
                                             cpl_size index, const char *value);
cpl_error_code irplib_sdp_spectrum_copy_assom(irplib_sdp_spectrum *self,
                                              cpl_size index,
                                              const cpl_propertylist *plist,
                                              const char *name);

const char * irplib_sdp_spectrum_get_voclass(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_voclass(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_voclass(irplib_sdp_spectrum *self,
                                               const char *value);
cpl_error_code irplib_sdp_spectrum_copy_voclass(irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *name);

const char * irplib_sdp_spectrum_get_vopub(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_vopub(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_vopub(irplib_sdp_spectrum *self,
                                             const char *value);
cpl_error_code irplib_sdp_spectrum_copy_vopub(irplib_sdp_spectrum *self,
                                              const cpl_propertylist *plist,
                                              const char *name);

const char * irplib_sdp_spectrum_get_title(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_title(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_title(irplib_sdp_spectrum *self,
                                             const char *value);
cpl_error_code irplib_sdp_spectrum_copy_title(irplib_sdp_spectrum *self,
                                              const cpl_propertylist *plist,
                                              const char *name);

const char * irplib_sdp_spectrum_get_object(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_object(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_object(irplib_sdp_spectrum *self,
                                              const char *value);
cpl_error_code irplib_sdp_spectrum_copy_object(irplib_sdp_spectrum *self,
                                               const cpl_propertylist *plist,
                                               const char *name);

double irplib_sdp_spectrum_get_aperture(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_aperture(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_aperture(irplib_sdp_spectrum *self,
                                                double value);
cpl_error_code irplib_sdp_spectrum_copy_aperture(irplib_sdp_spectrum *self,
                                                 const cpl_propertylist *plist,
                                                 const char *name);

double irplib_sdp_spectrum_get_telapse(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_telapse(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_telapse(irplib_sdp_spectrum *self,
                                               double value);
cpl_error_code irplib_sdp_spectrum_copy_telapse(irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *name);

double irplib_sdp_spectrum_get_tmid(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_tmid(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_tmid(irplib_sdp_spectrum *self,
                                            double value);
cpl_error_code irplib_sdp_spectrum_copy_tmid(irplib_sdp_spectrum *self,
                                             const cpl_propertylist *plist,
                                             const char *name);

double irplib_sdp_spectrum_get_specval(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_specval(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_specval(irplib_sdp_spectrum *self,
                                               double value);
cpl_error_code irplib_sdp_spectrum_copy_specval(irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *name);

double irplib_sdp_spectrum_get_specbw(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_specbw(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_specbw(irplib_sdp_spectrum *self,
                                              double value);
cpl_error_code irplib_sdp_spectrum_copy_specbw(irplib_sdp_spectrum *self,
                                               const cpl_propertylist *plist,
                                               const char *name);

const char * irplib_sdp_spectrum_get_extname(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_extname(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_extname(irplib_sdp_spectrum *self,
                                               const char *value);
cpl_error_code irplib_sdp_spectrum_copy_extname(irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *name);

cpl_boolean irplib_sdp_spectrum_get_inherit(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_inherit(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_inherit(irplib_sdp_spectrum *self,
                                               cpl_boolean value);
cpl_error_code irplib_sdp_spectrum_copy_inherit(irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *name);

cpl_size irplib_sdp_spectrum_get_nelem(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_nelem(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_nelem(irplib_sdp_spectrum *self,
                                             cpl_size value);
cpl_error_code irplib_sdp_spectrum_copy_nelem(irplib_sdp_spectrum *self,
                                              const cpl_propertylist *plist,
                                              const char *name);

double irplib_sdp_spectrum_get_tdmin(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_tdmin(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_tdmin(irplib_sdp_spectrum *self,
                                             double value);
cpl_error_code irplib_sdp_spectrum_copy_tdmin(irplib_sdp_spectrum *self,
                                              const cpl_propertylist *plist,
                                              const char *name);

double irplib_sdp_spectrum_get_tdmax(const irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_reset_tdmax(irplib_sdp_spectrum *self);
cpl_error_code irplib_sdp_spectrum_set_tdmax(irplib_sdp_spectrum *self,
                                             double value);
cpl_error_code irplib_sdp_spectrum_copy_tdmax(irplib_sdp_spectrum *self,
                                              const cpl_propertylist *plist,
                                              const char *name);

/*
 * Method to replace the default keyword comment with a custom comment
 */

cpl_error_code
irplib_sdp_spectrum_replace_column_comment(irplib_sdp_spectrum *self,
                                           const char *name,
                                           const char *keyword,
                                           const char *comment);

/*
 * Column manipulation functions.
 */

cpl_size irplib_sdp_spectrum_get_ncol(const irplib_sdp_spectrum *self);

cpl_boolean irplib_sdp_spectrum_has_column(const irplib_sdp_spectrum *self,
                                           const char* name);

cpl_array *
irplib_sdp_spectrum_get_column_names(const irplib_sdp_spectrum *self);

cpl_error_code
irplib_sdp_spectrum_new_column(irplib_sdp_spectrum *self, const char *name,
                               cpl_type type);

cpl_error_code
irplib_sdp_spectrum_add_column(irplib_sdp_spectrum *self, const char *name,
                               cpl_type type, const char *unit,
                               const char *format, const char *tutyp,
                               const char *tucd, const cpl_array *data);

cpl_error_code
irplib_sdp_spectrum_copy_column(irplib_sdp_spectrum *self,
                                const cpl_table* table, const char *name);

cpl_error_code
irplib_sdp_spectrum_copy_column_regexp(irplib_sdp_spectrum *self,
                                       const cpl_table* table,
                                       const char *regexp, int invert);

cpl_error_code
irplib_sdp_spectrum_update_column(irplib_sdp_spectrum *self, const char *name,
                                  const cpl_table* table, const char *colname,
                                  int flags);

cpl_error_code
irplib_sdp_spectrum_delete_column(irplib_sdp_spectrum *self, const char *name);

cpl_type irplib_sdp_spectrum_get_column_type(const irplib_sdp_spectrum *self,
                                             const char *name);

const char *
irplib_sdp_spectrum_get_column_unit(const irplib_sdp_spectrum *self,
                                    const char *name);

cpl_error_code
irplib_sdp_spectrum_set_column_unit(irplib_sdp_spectrum *self,
                                    const char *name, const char *unit);

cpl_error_code
irplib_sdp_spectrum_copy_column_unit(irplib_sdp_spectrum *self,
                                     const char *name,
                                     const cpl_propertylist *plist,
                                     const char *key);

const char *
irplib_sdp_spectrum_get_column_format(const irplib_sdp_spectrum *self,
                                      const char *name);

cpl_error_code
irplib_sdp_spectrum_set_column_format(irplib_sdp_spectrum *self,
                                      const char *name, const char *format);

const char *
irplib_sdp_spectrum_get_column_tutyp(const irplib_sdp_spectrum *self,
                                     const char *name);

cpl_error_code
irplib_sdp_spectrum_set_column_tutyp(irplib_sdp_spectrum *self,
                                     const char *name, const char *tutyp);

cpl_error_code
irplib_sdp_spectrum_copy_column_tutyp(irplib_sdp_spectrum *self,
                                      const char *name,
                                      const cpl_propertylist *plist,
                                      const char *key);

const char *
irplib_sdp_spectrum_get_column_tucd(const irplib_sdp_spectrum *self,
                                    const char *name);

cpl_error_code
irplib_sdp_spectrum_set_column_tucd(irplib_sdp_spectrum *self,
                                    const char *name, const char *tucd);

cpl_error_code
irplib_sdp_spectrum_copy_column_tucd(irplib_sdp_spectrum *self,
                                     const char *name,
                                     const cpl_propertylist *plist,
                                     const char *key);

const char *
irplib_sdp_spectrum_get_column_tcomm(const irplib_sdp_spectrum *self,
                                     const char *name);

cpl_error_code
irplib_sdp_spectrum_set_column_tcomm(irplib_sdp_spectrum *self,
                                     const char *name, const char *tcomm);

cpl_error_code
irplib_sdp_spectrum_copy_column_tcomm(irplib_sdp_spectrum *self,
                                      const char *name,
                                      const cpl_propertylist *plist,
                                      const char *key);

const cpl_array *
irplib_sdp_spectrum_get_column_data(const irplib_sdp_spectrum *self,
                                    const char *name);

cpl_error_code
irplib_sdp_spectrum_set_column_data(irplib_sdp_spectrum *self,
                                    const char *name, const cpl_array *array);

/*
 * Methods for loading and saving a spectrum object.
 */

irplib_sdp_spectrum * irplib_sdp_spectrum_load(const char *filename);

cpl_error_code irplib_sdp_spectrum_save(const irplib_sdp_spectrum *self,
                                        const char *filename,
                                        const cpl_propertylist *extra_pheader,
                                        const cpl_propertylist *extra_header);

cpl_error_code irplib_dfs_save_spectrum(cpl_frameset              * allframes,
                                        cpl_propertylist          * header,
                                        const cpl_parameterlist   * parlist,
                                        const cpl_frameset        * usedframes,
                                        const cpl_frame           * inherit,
                                        const irplib_sdp_spectrum * spectrum,
                                        const char                * recipe,
                                        const cpl_propertylist    * applist,
                                        const cpl_propertylist    * tablelist,
                                        const char                * remregexp,
                                        const char                * pipe_id,
                                        const char                * dict_id,
                                        const char                * filename);

/*
 * Dump function for debugging.
 */

void irplib_sdp_spectrum_dump(const irplib_sdp_spectrum *self, FILE *stream);

/*
 * Additional useful helper functions.
 */

#  ifdef IRPLIB_USE_FITS_UPDATE_CHECKSUM

/* NOTE: if one wants to use the following function the preprocessor macro
 * IRPLIB_USE_FITS_UPDATE_CHECKSUM must be defined when including this file and
 * when compiling the libirplib.a library. In addition, there will be a
 * dependency introduced to cfitsio in libirplib.a and any binary linking
 * against it. This will require adding the appropriate linker flags when
 * building such libraries.
 * The easiest way to add the IRPLIB_USE_FITS_UPDATE_CHECKSUM macro is to add
 * the following lines to either the acinclude.m4 or configure.ac files of the
 * parent pipeline project:
 *   IRPLIB_CPPFLAGS='-DIRPLIB_USE_FITS_UPDATE_CHECKSUM'
 *   AC_SUBST(IRPLIB_CPPFLAGS)
 */
cpl_error_code irplib_fits_update_checksums(const char* filename);

#  endif /* IRPLIB_USE_FITS_UPDATE_CHECKSUM */

CPL_END_DECLS

#endif /* IRPLIB_SDP_SPECTRUM_H */
